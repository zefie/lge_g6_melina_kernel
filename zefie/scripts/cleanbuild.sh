#!/bin/bash
.zefie/scripts/clean.sh
RC=$?
if [ $RC -ne 0 ]; then
	echo "Error while cleaning."
	exit $RC
fi
.zefie/scripts/build.sh
RC=$?
if [ $RC -ne 0 ]; then
	echo "Error while building."
	exit $RC
fi
