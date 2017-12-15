#!/bin/bash
source .zefie/scripts/buildenv.sh

DEFCONFIG_DIR="arch/${ARCH}/configs"

cp -r .zefie/scripts/dtbTool scripts/

read -r -d '' CONFIG << EOM
# Melina Kernel Custom Config Options
CONFIG_LOCALVERSION="-${KERNEL_NAME}"
CONFIG_DRIVEDROID_CDROM=y
CONFIG_EXFAT_FS=m
CONFIG_EXFAT_DISCARD=y
CONFIG_EXFAT_DELAYED_SYNC=y
CONFIG_EXFAT_KERNEL_DEBUG=n
CONFIG_EXFAT_DEBUG_MSG=n
CONFIG_EXFAT_DEFAULT_CODEPAGE=437
CONFIG_EXFAT_DEFAULT_IOCHARSET="utf8
CONFIG_FB_MSM_MDSS_KCAL_CTRL=y
CONFIG_DYNAMIC_FSYNC=y
CONFIG_CPU_INPUT_BOOST=y
EOM

if [ ! -z "${EXTRA_CONFIG}" ]; then
	CONFIG+="$(echo; echo "${EXTRA_CONFIG}";)"
fi

echo "*** Generating ${KERNEL_NAME} kernel defconfig..."
rm -f ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}
cp -f ${DEFCONFIG_DIR}/${ORIG_DEFCONFIG} ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}

if [ ! -z "${CONFIG}" ]; then
	echo "${CONFIG}" >> ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}
fi

make -C .zefie/lz4demo clean > /dev/null 2>&1
make -C .zefie/lz4demo
