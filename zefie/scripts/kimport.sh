#!/bin/bash
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

OLDKERNEL="$(realpath "${KERNEL_SOURCE_DIR}/../lge_g6_melina_kernel")"
NEWKERNEL="${KERNEL_SOURCE_DIR}"

if [ ! -z "${1}" ]; then
	for r in $(find "${OLDKERNEL}" | sed "s|${OLDKERNEL}/||" | grep ${@}); do
		if [ -d "${OLDKERNEL}/${r}" ]; then
			mkdir -p "${NEWKERNEL}/${r}"
		else
			cp -v "${OLDKERNEL}/${r}" "${NEWKERNEL}/${r}"
		fi
	done
fi
