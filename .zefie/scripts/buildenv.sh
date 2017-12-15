#!/bin/bash
export TOOLCHAIN="/home/zefie/dev/toolchains/uber/out/aarch64-linux-android-6.x/bin/aarch64-linux-android-"
export KERNEL_BUILDDIR="build"
export ORIG_DEFCONFIG=lucye_nao_us-perf_defconfig
export KERNEL_DEFCONFIG=lucye_us997_melina_defconfig
export KERNEL_NAME=melina
export ANDROID_TARGET="LGE-Stock"
export ARCH=arm64
export CROSS_COMPILE="${TOOLCHAIN}"
export Z_USE_CCACHE=1

# Do not edit below this line

if [ $(command -v ccache >/dev/null 2>&1) -eq $(true) ] && [ ${Z_USE_CCACHE} -eq 1 ]; then
	if [ -z "${Z_CCACHE_NOTICE}" ]; then
		export TOOLCHAIN="ccache ${TOOLCHAIN}"
		echo " * zefie scripts notice: ccache enabled"
		export Z_CCACHE_NOTICE=1
	fi
else
	export TOOLCHAIN="/home/zefie/dev/toolchains/uber/out/aarch64-linux-android-6.x/bin/aarch64-linux-android-"
	if [ -z "${Z_CCACHE_NOTICE}" ]; then
		echo " * zefie scripts notice: ccache NOT enabled"
		export Z_CCACHE_NOTICE=1
	fi
fi

if [ "$(basename $0)" == "buildenv.sh" ]; then
	if [ ! -z "$1" ]; then
		$@
	fi
fi

