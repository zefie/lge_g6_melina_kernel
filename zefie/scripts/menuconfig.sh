#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

if [ ! -z "${CLEAN}" ]; then
	echo "Cleaning kernel and regenerating config..."
	errchk "${SCRIPTDIR}/clean.sh"
fi
errchk "${SCRIPTDIR}/make.sh" menuconfig
