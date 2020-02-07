#!/bin/bash
CPUS=$(grep -c ^processor /proc/cpuinfo)
.zefie/scripts/make.sh -j"${CPUS}"
exit $?
