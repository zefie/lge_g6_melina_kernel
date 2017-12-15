#!/bin/bash
export PATH=${PWD}/.zefie/lz4demo:${PATH}
export TOOLCHAIN="/home/zefie/dev/toolchains/uber/out/aarch64-linux-android-6.x/bin/aarch64-linux-android-"
export KERNEL_BUILDDIR="build"

export KERNEL_MANU="LG"
export KERNEL_MODEL="G6"
export KERNEL_DEVMODEL="US997"

export ORIG_DEFCONFIG=lucye_nao_us-perf_defconfig
export KERNEL_DEFCONFIG=lucye_us997_melina_defconfig

export KERNEL_NAME=melina
export ANDROID_TARGET="LGE-Stock"

# Do not edit below this line

export ARCH=arm64
export CROSS_COMPILE="${TOOLCHAIN}"
export KERNEL_COMPRESSION_SUFFIX=lz4

if [ "$(basename $0)" == "buildenv.sh" ]; then
	if [ ! -z "$1" ]; then
		$@
	fi
fi

