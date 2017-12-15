#!/bin/bash
source .zefie/scripts/buildenv.sh

LATEST=$(ls -1t build/out/${KERNEL_NAME}_kernel*.zip | head -n1)
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
