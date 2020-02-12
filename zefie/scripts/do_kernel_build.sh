#!/bin/bash
# shellcheck disable=SC1090
SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

function lerrchk() {
	local res="${1}"
	local log="${2}"
	if [ "${res}" -ne 0 ]; then
		echo "*** BUILD ERROR *** "
		echo "* Device: ${KERNEL_DEVMODEL}"
		echo "Build Log:"
		cat "${log}"
		exit "${res}"
	fi
}

if [ ! -z "${1}" ]; then
	KERNEL_DEVMODEL="$(echo "${1}" | tr '[:lower:]' '[:upper:]')"
	if [ -z "${USE_DEBUG}" ]; then
		KERNEL_DEVMODEL_LOWER="$(echo "${KERNEL_DEVMODEL}" | tr '[:upper:]' '[:lower:]')"
	else
		KERNEL_DEVMODEL_LOWER="debug-$(echo "${KERNEL_DEVMODEL}" | tr '[:upper:]' '[:lower:]')"
	fi
	shift
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

	mkdir -p "${LG_OUT_DIRECTORY}"

	KERNLOG="${LG_OUT_DIRECTORY}/${KERNEL_NAME_LOWER}-kernel-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOG="${LG_OUT_DIRECTORY}/${KERNEL_NAME_LOWER}-buildzip-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOGD="${LG_OUT_DIRECTORY}/${KERNEL_NAME_LOWER}-buildzip_details-${KERNEL_DEVMODEL_LOWER}.log"

	Z_BUILD_ARG=(clean)

	if [ -z "${USE_CLANG}" ]; then
		echo "* gcc build"
	else
		echo "* clang build"
		Z_BUILD_ARG+=(clang)
	fi

	if [ -z "${USE_DEBUG}" ]; then
		echo "* Production (non-debug) kernel"
	else
		echo "* DEBUG kernel"
		Z_BUILD_ARG+=(debug)
	fi

	if [ -z "${WORKSPACE}" ]; then
		echo "* Cleaning old ${KERNEL_DEVMODEL} log files..."
		errchk rm -f "${LG_OUT_DIRECTORY}/"*"-${KERNEL_DEVMODEL_LOWER}.log"
	fi

	Z_BUILD_ARG+=(build)

	echo "* Building ${KERNEL_DEVMODEL} kernel (log in ${KERNLOG})"
	"${SCRIPTDIR}/build.sh" "${Z_BUILD_ARG[@]}" > "${KERNLOG}" 2>&1
	lerrchk $? "${KERNLOG}"

	echo "* Building ${KERNEL_DEVMODEL} zip (log in ${ZIPLOG})"
	"${SCRIPTDIR}/buildzip.sh" > "${ZIPLOG}" 2>&1
	lerrchk $? "${ZIPLOG}"

	ZIPNAME=$(find build/out/ -name "boot_*.zip" | rev | cut -d'/' -f1 | rev)

	errchk cp build/out/buildzip.log "${ZIPLOGD}"
	errchk cp "build/out/${ZIPNAME}" "${LG_OUT_DIRECTORY}"/
else
	echo "Usage: ${0} MODEL [conf]"
	exit 1
fi

