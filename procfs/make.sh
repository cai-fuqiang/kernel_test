#!/bin/sh
:<<EOF
make -C /usr/src/kernels/4.18.0-305.7.1.el8_4.x86_64 M=/root/workspace/kernel_test/procfs modules;
make[1]: 进入目录“/usr/src/kernels/4.18.0-305.7.1.el8_4.x86_64”
test -e include/generated/autoconf.h -a -e include/config/auto.conf || (                \
echo >&2;                                                       \
echo >&2 "  ERROR: Kernel configuration is invalid.";           \
echo >&2 "         include/generated/autoconf.h or include/config/auto.conf are missing.";\
echo >&2 "         Run 'make oldconfig && make prepare' on kernel src to fix it.";      \
echo >&2 ;                                                      \
/bin/false)
mkdir -p /root/workspace/kernel_test/procfs/.tmp_versions ; rm -f /root/workspace/kernel_test/procfs/.tmp_versions/*
make -f ./scripts/Makefile.build obj=/root/workspace/kernel_test/procfs
(cat /dev/null;   echo kernel//root/workspace/kernel_test/procfs/proctest.ko;) > /root/workspace/kernel_test/procfs/modules.order
  gcc -Wp,-MD,/root/workspace/kernel_test/procfs/.proc-test.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/8/include -I./arch/x86/include -I./arch/x86/include/generated   -I./include/drm-backport -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -DCC_HAVE_ASM_GOTO -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_AVX512=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1 -DCONFIG_TPAUSE=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-int-in-bool-context -O2 --param=allow-store-data-races=0 -Wframe-larger-than=2048 -fstack-protector-strong -Wno-unused-but-set-variable -Wno-unused-const-variable -g -gdwarf-4 -pg -mrecord-mcount -mfentry -DCC_USING_FENTRY -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -Wno-stringop-truncation -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -fmacro-prefix-map=./= -Wno-packed-not-aligned  -DMODULE  -DKBUILD_BASENAME='"proc_test"' -DKBUILD_MODNAME='"proctest"' -c -o /root/workspace/kernel_test/procfs/.tmp_proc-test.o /root/workspace/kernel_test/procfs/proc-test.c
   ./tools/objtool/objtool orc generate  --module --no-fp  --retpoline "/root/workspace/kernel_test/procfs/.tmp_proc-test.o";
  ld -m elf_x86_64  -z max-page-size=0x200000   -r -o /root/workspace/kernel_test/procfs/proctest.o /root/workspace/kernel_test/procfs/proc-test.o ; scripts/mod/modpost /root/workspace/kernel_test/procfs/proctest.o
make -f ./scripts/Makefile.modpost
  find /root/workspace/kernel_test/procfs/.tmp_versions -name '*.mod' | xargs -r grep -h '\.ko$' | sort -u | sed 's/\.ko$/.o/' | scripts/mod/modpost -m -a -i ./Module.symvers -I /root/workspace/kernel_test/procfs/Module.symvers  -o /root/workspace/kernel_test/procfs/Module.symvers  -w  -s -T -
  gcc -Wp,-MD,/root/workspace/kernel_test/procfs/.proctest.mod.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/8/include -I./arch/x86/include -I./arch/x86/include/generated   -I./include/drm-backport -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -DCC_HAVE_ASM_GOTO -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_AVX512=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1 -DCONFIG_TPAUSE=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-int-in-bool-context -O2 --param=allow-store-data-races=0 -Wframe-larger-than=2048 -fstack-protector-strong -Wno-unused-but-set-variable -Wno-unused-const-variable -g -gdwarf-4 -pg -mrecord-mcount -mfentry -DCC_USING_FENTRY -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -Wno-stringop-truncation -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -fmacro-prefix-map=./= -Wno-packed-not-aligned  -DKBUILD_BASENAME='"proctest.mod"' -DKBUILD_MODNAME='"proctest"' -DMODULE  -c -o /root/workspace/kernel_test/procfs/proctest.mod.o /root/workspace/kernel_test/procfs/proctest.mod.c
  ld -r -m elf_x86_64  -z max-page-size=0x200000 -T ./scripts/module-common.lds  --build-id  -o /root/workspace/kernel_test/procfs/proctest.ko /root/workspace/kernel_test/procfs/proctest.o /root/workspace/kernel_test/procfs/proctest.mod.o ;  true
make[1]: 离开目录“/usr/src/kernels/4.18.0-305.7.1.el8_4.x86_64”
EOF
MY_PWD=$(pwd)
cd /usr/src/kernels/$(uname -r)
  gcc -Wp,-MD,/root/workspace/kernel_test/procfs/.proc-test.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/8/include -I./arch/x86/include -I./arch/x86/include/generated   -I./include/drm-backport -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -DCC_HAVE_ASM_GOTO -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_AVX512=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1 -DCONFIG_TPAUSE=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-int-in-bool-context -O2 --param=allow-store-data-races=0 -Wframe-larger-than=2048 -fstack-protector-strong -Wno-unused-but-set-variable -Wno-unused-const-variable -g -gdwarf-4 -pg -mrecord-mcount -mfentry -DCC_USING_FENTRY -fno-inline-functions-called-once -Wdeclaration-after-statement -Wno-pointer-sign -Wno-stringop-truncation -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -fmacro-prefix-map=./= -Wno-packed-not-aligned  -DMODULE  -DKBUILD_BASENAME='"proc_test"' -DKBUILD_MODNAME='"proctest"' -c -o /root/workspace/kernel_test/procfs/tmp_proc-test.o /root/workspace/kernel_test/procfs/proc-test.c
cp /root/workspace/kernel_test/procfs/tmp_proc-test.o /root/workspace/kernel_test/procfs/tmp_proc-test.o.bak
#./objtool orc generate  --module --no-fp  --retpoline "/root/workspace/kernel_test/procfs/tmp_proc-test.o"
cd --
