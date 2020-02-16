#!/bin/bash
# shellcheck disable=SC1090
if [ -z "${SCRIPTDIR}" ]; then
	SCRIPTDIR=$(realpath "$(dirname "${0}")")
	source "${SCRIPTDIR}/buildenv.sh"
fi

kernel_make "${@}"
