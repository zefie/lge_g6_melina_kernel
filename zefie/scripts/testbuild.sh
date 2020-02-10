#!/bin/bash
# shellcheck disable=SC1090

export USE_CCACHE=1

if [ ! -z "${1}" ]; then
	export KERNEL_DEVMODEL="$(echo "${1}" | tr [:lower:] [:upper:])"
	shift
fi

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

errchk cd "${KERNEL_SOURCE_DIR}"



if [ "${1}" == "clean" ]; then
	shift
	errchk rm -rf "${KERNEL_SOURCE_DIR:?}/${KERNEL_BUILD_DIR:?}"
	errchk "${SCRIPTDIR}/clean.sh"
elif [ "${1}" == "prodtest" ]; then
	shift
	export FORCE_RESET=1
	errchk "${SCRIPTDIR}/clean.sh"
elif [ -z "${1}" ]; then
	"${SCRIPTDIR}/make.sh" "${@}"
	exit $?
fi


errchk "${SCRIPTDIR}/create_defconfigs.sh" CONFIG_MELINA_WARN_AS_ERR=y
errchk "${SCRIPTDIR}/defconfig.sh"
errchk "${SCRIPTDIR}/build.sh"
errchk "${SCRIPTDIR}/buildzip.sh"
