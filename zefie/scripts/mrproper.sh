#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"
errchk mkdir -p "${KERNEL_BUILD_DIR}"
errchk make O="${KERNEL_BUILD_DIR}" CROSS_COMPILE="${TOOLCHAIN}" CROSS_COMPILE_ARM32="${TOOLCHAIN32}gcc" mrproper
