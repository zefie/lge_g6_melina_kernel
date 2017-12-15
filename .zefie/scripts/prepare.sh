#!/bin/bash
DEFCONFIG_DIR="arch/${ARCH}/configs"

cp -r .zefie/scripts/dtbTool scripts/

# Melina Kernel Custom Config Options
read -r -d '' CONFIG << EOM
CONFIG_LOCALVERSION="-${KERNEL_NAME}"
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
