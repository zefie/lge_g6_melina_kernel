#!/bin/bash
source .zefie/scripts/buildenv.sh
for m in ${SUPPORTED_MODELS[@]}; do
	.zefie/scripts/do_kernel_build.sh $m
	RC=$?
	if [ $RC -ne 0 ]; then
		exit $RC
	fi
done
