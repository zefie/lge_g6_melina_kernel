#!/bin/bash
# shellcheck disable=SC1090

if [ -z "${SCRIPTDIR}" ]; then
	SCRIPTDIR=$(realpath "$(dirname "${0}")")
	source "${SCRIPTDIR}/buildenv.sh"
fi

for m in "${SUPPORTED_MODELS[@]}"; do
	# Pproduction Kernel
	errchk "${SCRIPTDIR}/do_kernel_build.sh" "${m}"

	# Debug Kernel
	export USE_DEBUG=1
	errchk "${SCRIPTDIR}/do_kernel_build.sh" "${m}"
	export USE_DEBUG=

done
