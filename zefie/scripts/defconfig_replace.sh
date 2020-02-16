#!/bin/bash

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"


for f in $(grep -l -r "${1}" "${KERNEL_SOURCE_DIR}/${DEFCONFIG_DIR}"); do
	echo "${f}"
	sed -i "s|${1}|${2}|g" "${f}"
done
