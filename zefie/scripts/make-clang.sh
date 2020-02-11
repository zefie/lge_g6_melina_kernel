#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

# point to your android tree
Z_ANDROID="${Z_ANDROID:-/home/zefie/android}"
Z_ANDROID_CLANG="${Z_ANDROID}/prebuilts/clang/host/linux-x86/clang-r328903"
Z_ANDROID_BINUTILS="${Z_ANDROID}/prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.17-4.8/x86_64-linux/bin"

PATH="${Z_ANDROID}/prebuilts/build-tools/linux-x86/bin:${Z_ANDROID_CLANG}/bin:${PATH}"
LD_LIBRARY_PATH="${Z_ANDROID}/prebuilts/build-tools/linux-x86/lib:${Z_ANDROID_CLANG}/lib64:${LD_LIBRARY_PATH}"
PERL5LIB="${Z_ANDROID}/prebuilts/tools-extras/common/perl-base"
MAKE="${Z_ANDROID}/prebuilts/build-tools/linux-x86/bin/make"

export PATH LD_LIBRARY_PATH PERL5LIB

CC=" clang"
HOSTCC="${Z_ANDROID_CLANG}/bin/clang"
HOSTCXX="${Z_ANDROID_CLANG}/bin/clang++"

if [ ! -z "${USE_CCACHE}" ]; then
	CC="ccache ${CC}"
	HOSTCC="ccache ${HOSTCC}"
	HOSTCXX="ccache ${HOSTCXX}"
fi

KENV=( \
	"CFLAGS_MODULE=-fno-pic" "HOSTCFLAGS=-I/usr/include -I/usr/include/x86_64-linux-gnu" \
	"O=${KERNEL_BUILD_DIR:?}" "ARCH=${ARCH}" "CC=${CC}" "CLANG_TRIPLE=aarch64-linux-gnu" "HOSTCC=${HOSTCC}" \
	"HOSTCXX=${HOSTCXX}" "HOSTAR=${Z_ANDROID_BINUTILS}/ar" "HOSTLD=${Z_ANDROID_BINUTILS}/ld" "BISON=$(which bison)" \
	"FLEX=$(which flex)" "PERL=$(which perl)" "CROSS_COMPILE=${TOOLCHAIN}" "CROSS_COMPILE_ARM32=${TOOLCHAIN32}" \
	"${@}" \
)

errchk mkdir -p "${KERNEL_BUILD_DIR}"
errchk "${MAKE}" "${KENV[@]}"








