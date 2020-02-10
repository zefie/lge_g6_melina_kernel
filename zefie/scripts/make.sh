#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

errchk mkdir -p "${KERNEL_BUILD_DIR}"
if [ -z "${USE_CCACHE}" ]; then
	errchk make O="${KERNEL_BUILD_DIR}" CROSS_COMPILE="${TOOLCHAIN}" CROSS_COMPILE_ARM32="${TOOLCHAIN32}" CC="ccache gcc" "${@}"
else
	errchk make O="${KERNEL_BUILD_DIR}" CROSS_COMPILE="${TOOLCHAIN}" CROSS_COMPILE_ARM32="${TOOLCHAIN32}" "${@}"
fi
