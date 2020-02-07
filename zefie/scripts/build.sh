#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"
errchk "${SCRIPTDIR}/make.sh" -j"${CPUS}"
