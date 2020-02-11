#!/bin/bash
# shellcheck disable=SC1090

export USE_CCACHE=1

KERNEL_DEVMODEL="${KERNEL_DEVMODEL:-US997}"
export KERNEL_DEVMODEL

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"


errchk cd "${KERNEL_SOURCE_DIR}"

Z_BUILD_SCRIPT="${SCRIPTDIR}/build.sh"
Z_BUILD_SCRIPT_PARAM=()


if [ "${1}" == "clean" ]; then
	shift
	errchk rm -rf "${KERNEL_SOURCE_DIR:?}/${KERNEL_BUILD_DIR:?}"
	errchk "${SCRIPTDIR}/clean.sh"
elif [ "${1}" == "prodtest" ]; then
	shift
	export FORCE_RESET=1
	errchk "${SCRIPTDIR}/clean.sh"
	Z_BUILD_SCRIPT="${SCRIPTDIR}/do_kernel_build.sh"
	Z_BUILD_SCRIPT_PARAM=("${KERNEL_DEVMODEL}")
elif [ -z "${1}" ]; then
	Z_BUILD_SCRIPT="${SCRIPTDIR}/build.sh"
	Z_BUILD_SCRIPT_PARAM=("${@}")
fi


errchk "${SCRIPTDIR}/create_defconfigs.sh" CONFIG_MELINA_WARN_AS_ERR=y
errchk "${SCRIPTDIR}/defconfig.sh"
errchk "${Z_BUILD_SCRIPT}" "${Z_BUILD_SCRIPT_PARAM[@]}"
