#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

if [ ! -f "${DEFCONFIG_DIR}/${KERNEL_NAME_LOWER}_${KERNEL_DEVMODEL_LOWER}_defconfig" ]; then
	errchk "${SCRIPTDIR}/create_defconfigs.sh" "${Z_CONFIG_OVERRIDE[@]}"
fi
DEFCONFIG="${KERNEL_NAME_LOWER}_${KERNEL_DEVMODEL_LOWER}_defconfig"
echo "Using ${DEFCONFIG}..."
errchk "${SCRIPTDIR}/make.sh" "${DEFCONFIG}"
