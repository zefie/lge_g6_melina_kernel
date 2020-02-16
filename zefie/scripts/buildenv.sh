#!/bin/bash
# shellcheck disable=SC2034,SC1090
# git clone https://github.com/zefie/binary_toolchains -b uber-6.x-x86_64-aarch64
export TOOLCHAIN="${CROSS_COMPILE:-/home/zefie/ubertc/out/aarch64-linux-android-6.x/bin/aarch64-linux-android-}"
export TOOLCHAIN32="${CROSS_COMPILE_ARM32:-/home/zefie/ubertc/out/arm-linux-androideabi-6.x/bin/arm-linux-androideabi-}"


SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/functions.sh"

KERNEL_SOURCE_DIR=$(realpath "${SCRIPTDIR}/../..")

if [ -z "${WORKSPACE}" ]; then
	# default to ../lg_out
	LG_OUT_DIRECTORY=$(realpath "${KERNEL_SOURCE_DIR}/../lg_out")
else
	# For Jenkins, default to ${WORKSPACE}/lg_out
	LG_OUT_DIRECTORY=$(realpath "${WORKSPACE:?}/lg_out")
fi

CPUS=$(nproc)
export KERNEL_SOURCE_DIR SCRIPTDIR CPUS LG_OUT_DIRECTORY

Z_ANDROID="${Z_ANDROID:-/home/zefie/android}"
Z_ANDROID_CLANG="${Z_ANDROID}/prebuilts/clang/host/linux-x86/clang-r349610"
Z_ANDROID_BINUTILS="${Z_ANDROID}/prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.17-4.8/x86_64-linux/bin"


export KERNEL_MAKE_TARGETS=("dtbs" "Image.gz-dtb")
# Jenkins
if [ ! -z "${WORKSPACE}" ]; then
	export TOOLCHAIN="${WORKSPACE}/ubertc/aarch64-linux-android-6.x/bin/aarch64-linux-android-"
	export TOOLCHAIN32="${WORKSPACE}/ubertc/arm-linux-androideabi-6.x/bin/arm-linux-androideabi-"
fi

export KERNEL_NAME="MelinaReborn" # please change from Melina for custom builds
export KERNEL_DEVNAME="${USER}" # can be normal name, defaults to linux username ;)

export ANDROID_TARGET="AOSP" # Could be Lineage-14.0, or whatever. Just for zip name.
export KERNEL_BUILD_DIR="build" # A subdirectory in this repo that is in .gitignore and doesn't already exist. Best just leave it be.

# These can be overridden by the env, using vars that replace DEFAULT_ with KERNEL_, ex: KERNEL_DEVMODEL
export DEFAULT_MANU="LG" # for zip filename
export DEFAULT_MODEL="G6" # for zip filename
export DEFAULT_DEVMODEL="US997" # for zip filename, and anykernel whitelist (converted to lowercase for whitelist)

# Do not edit below this line

#export SUPPORTED_MODELS=("US997" "H870" "H872")
SUPPORTED_MODELS=($(grep -A100 '# start model list' "${SCRIPTDIR}/functions.sh" | grep -B100 '# end model list' | grep ')$' |  grep -v '(' | grep -v "\*" | grep -v "\-\-" | cut -d'"' -f2))
export PATH="${PWD}/zefie/lz4demo:${PATH}"
export ARCH="arm64"
export DEFCONFIG_DIR="arch/${ARCH}/configs"
KERNEL_NAME_LOWER="$(echo "${KERNEL_NAME}" | tr '[:upper:]' '[:lower:]')"
TC_VER=$("${TOOLCHAIN}gcc" --version | awk '/gcc /{print $0;exit 0;}')
export KERNEL_NAME_LOWER TC_VER


# Allow env override
if [ -z "${TC_NAME}" ]; then export TC_NAME="${ZEFIE_TC_NAME}"; fi
if [ -z "${TC_TYPE}" ]; then export TC_TYPE="${ZEFIE_TC_TYPE}"; fi
if [ -z "${TC_VER}" ]; then export TC_VER="${ZEFIE_TC_VER}"; fi
if [ -z "${KERNEL_MANU}" ]; then export KERNEL_MANU="${DEFAULT_MANU}"; fi
if [ -z "${KERNEL_MODEL}" ]; then export KERNEL_MODEL="${DEFAULT_MODEL}"; fi
if [ -z "${KERNEL_DEVMODEL}" ]; then KERNEL_DEVMODEL="${DEFAULT_DEVMODEL}"; fi

kernel_setdevice "${KERNEL_DEVMODEL}"

if [ ! -z "${USE_CCACHE}" ]; then
	TOOLCHAIN="ccache ${TOOLCHAIN}"
	TOOLCHAIN32="ccache ${TOOLCHAIN32}"
	export TOOLCHAIN TOOLCHAIN32
fi

errchk cd "${KERNEL_SOURCE_DIR}"

if [ "$(basename "${0}")" == "buildenv.sh" ]; then
	if [ ! -z "$1" ]; then
		errchk "${@}"
	else
		export PS1="(melina-dev)${PS1}" Z_SHELL_ENV=1
		bash
	fi
fi

