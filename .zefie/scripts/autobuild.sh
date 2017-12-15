#!/bin/bash
if [ -z "${NORESET}" ]; then
	./.zefie/scripts/resetgit.sh
	RES=$?
	if [ ${RES} -ne 0 ]; then
		echo "Error resetting kernel source (git error code: ${RES})"
		exit ${RES};
	fi
fi

./.zefie/scripts/prepare.sh
RES=$?
if [ ${RES} -ne 0 ]; then
	echo "Error preparing kernel (error code: ${RES})"
	exit ${RES};
fi
./.zefie/scripts/fullbuild.sh
