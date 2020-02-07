#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

for m in "${SUPPORTED_MODELS[@]}"; do
	errchk "${SCRIPTDIR}/do_kernel_build.sh" "${m}"
done
