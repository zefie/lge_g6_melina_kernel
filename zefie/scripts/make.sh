#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

errchk mkdir -p "${KERNEL_BUILDDIR}"
errchk make O="${KERNEL_BUILDDIR}" "${@}"
