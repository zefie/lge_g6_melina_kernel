#!/bin/bash
source .zefie/scripts/buildenv.sh
mkdir -p "${KERNEL_BUILDDIR}"
make O="${KERNEL_BUILDDIR}" "${@}"
exit $?
