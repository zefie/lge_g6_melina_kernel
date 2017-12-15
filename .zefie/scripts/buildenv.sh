#!/bin/bash
export TOOLCHAIN="/home/zefie/dev/toolchains/uber/out/aarch64-linux-android-6.x/bin/aarch64-linux-android-"
export KERNEL_BUILDDIR="build"
export ORIG_DEFCONFIG=lucye_nao_us-perf_defconfig
export KERNEL_DEFCONFIG=lucye_us997_melina_defconfig
export KERNEL_NAME=melina
export ANDROID_TARGET="LGE-Stock"
export ARCH=arm64
export CROSS_COMPILE="${TOOLCHAIN}"

if [ "$(basename $0)" == "buildenv.sh" ]; then
	if [ ! -z "$1" ]; then
		$@
	fi
fi
