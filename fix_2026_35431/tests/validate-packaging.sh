#!/usr/bin/env bash
set -euo pipefail

root_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root_dir"

fail()
{
	echo "FAIL: $*" >&2
	exit 1
}

require_file()
{
	[[ -f "$1" ]] || fail "missing file: $1"
}

require_text()
{
	local file=$1
	local text=$2

	grep -Fq "$text" "$file" ||
		fail "$file does not contain: $text"
}

require_file cve-31431-mitigate.c
require_file Makefile
require_file cve_31431_mitigate.modules-load.conf
require_file cve_31431_mitigate.modprobe.conf
require_file README.md
require_file cve-31431-mitigate.spec

require_text cve-31431-mitigate.c "#ifndef CONFIG_X86_64"
require_text cve-31431-mitigate.c 'request_module("af_alg")'
require_text cve-31431-mitigate.c 'MODULE_VERSION("1.0.0")'

require_text Makefile "obj-m += cve_31431_mitigate.o"
require_text Makefile "cve_31431_mitigate-y := cve-31431-mitigate.o"
require_text cve_31431_mitigate.modules-load.conf "cve_31431_mitigate"
require_text cve_31431_mitigate.modprobe.conf \
	"options cve_31431_mitigate disable_splice=1"

require_text cve-31431-mitigate.spec "%{!?kverrel:"
require_text cve-31431-mitigate.spec "%global debug_package %{nil}"
require_text cve-31431-mitigate.spec "%global __strip /bin/true"
require_text cve-31431-mitigate.spec "%global kernel_evr "
require_text cve-31431-mitigate.spec "Release:        1.%{kernel_evr_tag}%{?dist}"
require_text cve-31431-mitigate.spec "ExclusiveArch:  x86_64"
require_text cve-31431-mitigate.spec "BuildRequires:  kernel-devel = %{kernel_evr}"
require_text cve-31431-mitigate.spec "Requires:       kernel = %{kernel_evr}"
require_text cve-31431-mitigate.spec "6.6.0-jdcloud*.x86_64|6.11.4-jdcloud*.x86_64"
require_text cve-31431-mitigate.spec "weak-modules --add-modules --no-initramfs"
require_text cve-31431-mitigate.spec "weak-modules --remove-modules --no-initramfs"
require_text cve-31431-mitigate.spec "/usr/lib/modules-load.d/"
require_text cve-31431-mitigate.spec "/usr/lib/modprobe.d/"

if command -v rpmspec >/dev/null 2>&1; then
	if rpmspec -P cve-31431-mitigate.spec >/dev/null 2>&1; then
		fail "spec unexpectedly parses without kverrel"
	fi

	rpmspec -P \
		--define "kverrel 6.6.0-jdcloud6.0.0ba1f317c7.x86_64" \
		cve-31431-mitigate.spec >/dev/null
fi

echo "PASS: packaging definition is internally consistent"
