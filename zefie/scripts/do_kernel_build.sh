#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

if [ ! -z "${1}" ]; then
	KERNEL_DEVMODEL="$(echo "${1}" | tr '[:lower:]' '[:upper:]')"
	KERNEL_DEVMODEL_LOWER="$(echo "${KERNEL_DEVMODEL}" | tr '[:upper:]' '[:lower:]')"
	export KERNEL_DEVMODEL KERNEL_DEVMODEL_LOWER

	SUPPORTED=0
	for m in "${SUPPORTED_MODELS[@]}"; do
		if [ "${KERNEL_DEVMODEL}" == "$m" ]; then
			SUPPORTED=1
			break;
		fi
	done

	if [ -z "${SUPPORTED}" ]; then
		echo "Error: Unknown model (${KERNEL_DEVMODEL})";
		echo "This script supports: ${SUPPORTED_MODELS[*]}";
		exit 1
	fi

	KERNLOG="${LG_OUT_DIRECTORY}/kernel-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOG="${LG_OUT_DIRECTORY}/buildzip-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOGD="${LG_OUT_DIRECTORY}/buildzip_details-${KERNEL_DEVMODEL_LOWER}.log"

	if [ -z "${WORKSPACE}" ]; then
		echo "* Cleaning old ${1} log files..."
		errchk rm -f "${LG_OUT_DIRECTORY}/"*"-${KERNEL_DEVMODEL_LOWER}.log"
	fi

	if [ -z "${WORKSPACE}" ]; then
		echo "* Building ${1} kernel (log in ${KERNLOG})"
		errchk "${SCRIPTDIR}/cleanbuild.sh" > "${KERNLOG}" 2>&1
	else
		echo "* Building ${1} kernel"
		errchk "${SCRIPTDIR}/cleanbuild.sh"
	fi

	if [ -z "${WORKSPACE}" ]; then
		echo "* Building ${1} zip (log in ${ZIPLOG})"
		errchk "${SCRIPTDIR}/buildzip.sh" > "${ZIPLOG}" 2>&1
	else
		echo "* Building ${1} zip"
		errchk "${SCRIPTDIR}/buildzip.sh"
	fi

	ZIPNAME=$(find build/out/ -name "boot_*.zip" | rev | cut -d'/' -f1 | rev)

	errchk cp build/out/buildzip.log "${ZIPLOGD}"
	errchk cp "build/out/${ZIPNAME}" "${LG_OUT_DIRECTORY}"/
else
	echo "Usage: ${0} MODEL [conf]"
	exit 1
fi

