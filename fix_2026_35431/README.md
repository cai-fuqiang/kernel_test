# CVE-2026-31431 Runtime Mitigation Kmod

This package builds and installs the `cve_31431_mitigate` external kernel
module. The module places a kprobe on `af_alg_sendmsg()` and clears
`MSG_SPLICE_PAGES` before the function reads `msg->msg_flags`.

The first release is intentionally limited to:

- openEuler 24.03 LTS
- x86_64
- `6.6.0-jdcloud*` kernels

The implementation reads the second function argument from the x86_64
register state. Do not build or load it on another architecture.

## Prerequisites

Install RPM build tools and the development package that exactly matches the
target kernel. The build tree must exist at:

```text
/usr/src/kernels/<kverrel>
```

The target kernel must enable `CONFIG_KPROBES`, `CONFIG_KALLSYMS`,
`CONFIG_CRYPTO_USER_API`, and `CONFIG_CRYPTO_USER_API_AEAD`, and its headers
must define `MSG_SPLICE_PAGES`.

## Build

Supply the target kernel release explicitly. Do not select a kernel-devel
package implicitly on a build host that may contain multiple versions.

```bash
kverrel=6.6.0-jdcloud6.0.0ba1f317c7.x86_64

rpmbuild -ba cve-31431-mitigate.spec \
  --define "_sourcedir $PWD" \
  --define "kverrel $kverrel"
```

The source RPM can be rebuilt in another openEuler 24.03 build environment
with a different matching `kernel-devel` package and `kverrel`. The target
kernel EVR is included in the RPM `Release`, so packages built for different
kernels do not produce the same NEVRA.

Run the repository-level packaging check before building:

```bash
./tests/validate-packaging.sh
```

## Optional Module Signing

The default build is unsigned. To sign the module, keep the private key
outside the source tree and pass both signing paths at build time:

```bash
rpmbuild -ba cve-31431-mitigate.spec \
  --define "_sourcedir $PWD" \
  --define "kverrel $kverrel" \
  --with module_signing \
  --define "module_signing_key /secure/path/signing_key.pem" \
  --define "module_signing_cert /secure/path/signing_key.x509"
```

Signing material is not copied into the source RPM.
The RPM intentionally does not strip the module after Kbuild because stripping
an already signed kernel module invalidates its appended signature.

## Install

Install the binary RPM on a node where the mitigation should become active:

```bash
dnf install ./kmod-cve-31431-mitigate-*.rpm
```

The RPM runs `depmod`, registers the module with `weak-modules`, and loads it
with `modprobe` if it is not already loaded. It also installs vendor defaults
under `/usr/lib/modules-load.d` and `/usr/lib/modprobe.d`.

The RPM does not automatically reload an already running module during an
upgrade. Reload it during a maintenance window or reboot to activate updated
module code:

```bash
modprobe -r cve_31431_mitigate
modprobe cve_31431_mitigate
```

## Verify

Check package and module state:

```bash
rpm -q kmod-cve-31431-mitigate
lsmod | grep cve_31431_mitigate
cat /sys/module/cve_31431_mitigate/parameters/disable_splice
modinfo cve_31431_mitigate
dmesg | grep cve-31431-mitigate
```

The parameter should report `Y`, and the kernel log should report a registered
kprobe on `af_alg_sendmsg`.

The deployment is not complete until the CVE reproducer no longer triggers
the issue and normal AF_ALG AEAD operations still return correct results.
For a controlled test, the mitigation can be toggled at runtime:

```bash
echo 0 > /sys/module/cve_31431_mitigate/parameters/disable_splice
echo 1 > /sys/module/cve_31431_mitigate/parameters/disable_splice
```

## Weak Modules

The module is installed under:

```text
/lib/modules/<kverrel>/extra/cve_31431_mitigate/
```

openEuler's `weak-modules` tool may create links for later compatible kernels:

```bash
find /lib/modules -path '*/weak-updates/*cve_31431_mitigate*' -ls
modinfo -k <new-kernel-release> cve_31431_mitigate
```

`weak-modules` only reuses the module when symbol version checks pass. If no
link is created, or if the module reports an invalid format or unknown symbol,
build a new RPM against the new kernel.

## Rollback

Remove the package with:

```bash
dnf remove kmod-cve-31431-mitigate
```

Package removal first unloads the module. If unloading fails, the RPM erase is
stopped so package state does not claim that an active module was removed.
