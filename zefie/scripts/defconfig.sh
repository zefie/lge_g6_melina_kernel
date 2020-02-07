#!/bin/bash
source .zefie/scripts/buildenv.sh

if [ ! -f "${DEFCONFIG_DIR}/${KERNEL_NAME_LOWER}_${KERNEL_DEVMODEL_LOWER}_defconfig" ]; then
	.zefie/scripts/create_defconfigs.sh
fi
DEFCONFIG=${KERNEL_NAME_LOWER}_${KERNEL_DEVMODEL_LOWER}_defconfig
echo "Using ${DEFCONFIG}..."
.zefie/scripts/make.sh ${DEFCONFIG}
exit $?
