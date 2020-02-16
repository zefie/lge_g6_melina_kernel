#!/bin/bash

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"


for f in $(grep -l -r "${1}" "${KERNEL_SOURCE_DIR}/${DEFCONFIG_DIR}"); do
	echo "Replacing in ${f} ..."
	sed -i "s|${1}|${2}|g" "${f}"
done

Z_VAR=$(echo "${1}" | cut -d'_' -f2- | cut -d'=' -f1)
Z_NEWVAR=$(echo "${2}" | cut -d'_' -f2- | cut -d'=' -f1)
for f in $(grep -lr "${Z_VAR}" $(find "${KERNEL_SOURCE_DIR}" | grep 'Kconfig$')); do
	echo "Replacing in ${f} ..."
	sed -i "s|${Z_VAR}|${Z_NEWVAR}|g" "${f}"
done
