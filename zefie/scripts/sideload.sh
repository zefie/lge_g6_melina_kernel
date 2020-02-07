#!/bin/bash
source .zefie/scripts/buildenv.sh

LATEST=$(ls -1t build/out/boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}*.zip | head -n1)
if [ -f "${LATEST}" ]; then
	adb sideload "${LATEST}"
else
	echo "Could not find a zip? Did you build the kernel and zip file?";
	echo ""
	echo "Try the following:"
	echo ".zefie/scripts/fullbuild.sh"
	echo ".zefie/scripts/sideload.sh"
	exit 1;
fi
