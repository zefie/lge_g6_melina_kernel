#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

MAKE_CMDS=("-j${CPUS}" "${@}")

if [ -z "${USE_CLANG}" ]; then
	errchk "${SCRIPTDIR}/make-gcc.sh" "${MAKE_CMDS[@]}"
else
	errchk "${SCRIPTDIR}/make-clang.sh" "${MAKE_CMDS[@]}"
fi
