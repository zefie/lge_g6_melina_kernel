#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

errchk "${SCRIPTDIR}/clean.sh"
errchk "${SCRIPTDIR}/build.sh" "${@}"
