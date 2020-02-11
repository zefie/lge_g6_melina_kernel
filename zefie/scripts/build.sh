#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"


REQ_MAKE_CMDS=("dtbs" "Image.gz-dtb")


MAKE_CMDS=("-j${CPUS}" "${@}")

# make sure we build required targets even if user specifies something
for c in "${REQ_MAKE_CMDS[@]}"; do
	if [ "$(echo "${MAKE_CMDS[*]}" | grep -c "${c}")" -eq 0 ]; then
		MAKE_CMDS+=("${c}")
	fi
done

if [ -z "${USE_CLANG}" ]; then
	errchk "${SCRIPTDIR}/make.sh" "${MAKE_CMDS[@]}"
else
	errchk "${SCRIPTDIR}/make-clang.sh" "${MAKE_CMDS[@]}"
fi
