#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

LATEST=$(find "build/out/boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}"*.zip -printf "%T@ %Tc %p\n" | sort -nr | head -n1 | rev | cut -d' ' -f1 | rev)
if [ -f "${LATEST}" ]; then
	adb sideload "${LATEST}"
else
	echo "Could not find a zip? Did you build the kernel and zip file?";
	echo ""
	echo "Try the following:"
	echo "zefie/scripts/build.sh clean build zip"
	echo "zefie/scripts/sideload.sh"
	exit 1;
fi
