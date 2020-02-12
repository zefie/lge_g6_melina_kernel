#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

errchk rm -rf "${KERNEL_BUILD_DIR:?}"
errchk "${SCRIPTDIR}/resetgit.sh"
errchk "${SCRIPTDIR}/mrproper.sh"
errchk rm -f "${DEFCONFIG_DIR}/${KERNEL_NAME_LOWER}_"*_defconfig

