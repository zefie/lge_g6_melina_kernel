#!/bin/bash
if [ ! -z "${CLEAN}" ]; then
	echo "Cleaning kernel and regenerating config..."
	.zefie/scripts/clean.sh
	RC=$?
	if [ $RC -ne 0 ]; then
		exit $RC
	fi
fi
.zefie/scripts/buildenv.sh .zefie/scripts/make.sh menuconfig
exit $?
