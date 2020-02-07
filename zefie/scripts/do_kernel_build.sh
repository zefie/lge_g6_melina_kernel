LG_OUT_DIRECTORY="../lg_out"

if [ ! -z "${1}" ]; then
	export KERNEL_DEVMODEL="$(echo "${1}" | tr '[:lower:]' '[:upper:]')"
	export KERNEL_DEVMODEL_LOWER="$(echo "${KERNEL_DEVMODEL}" | tr '[:upper:]' '[:lower:]')"

	# TODO: An list/array of models

	for m in ${SUPPORTED_MODELS[@]}; do
		if [ "${KERNEL_DEVMODEL}" == "$m" ]; then
			SUPPORTED=1
			break;
		fi
	done

	if [ ! -z "${SUPPORTED}" ]; then
		echo "Error: Unknown model (${1})";
		echo "This script supports: ${SUPPORTED_MODELS[*]}";
		exit 1
	fi

	KERNLOG="${LG_OUT_DIRECTORY}/kernel-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOG="${LG_OUT_DIRECTORY}/buildzip-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOGD="${LG_OUT_DIRECTORY}/buildzip_details-${KERNEL_DEVMODEL_LOWER}.log"

	if [ -z "${WORKSPACE}" ]; then
		echo "* Cleaning old ${1} log files..."
		rm -f ${LG_OUT_DIRECTORY}/*-${KERNEL_DEVMODEL_LOWER}.log
	fi

	if [ -z "${WORKSPACE}" ]; then
		echo "* Building ${1} kernel (log in ${KERNLOG})"
		.zefie/scripts/cleanbuild.sh > "${KERNLOG}" 2>&1
	else
		echo "* Building ${1} kernel"
		.zefie/scripts/cleanbuild.sh
	fi
	RC=$?
	if [ $RC -ne 0 ]; then
		echo "Error. Please check log. Code: $RC"
		exit $RC
	fi

	if [ -z "${WORKSPACE}" ]; then
		echo "* Building ${1} zip (log in ${ZIPLOG})"
		.zefie/scripts/buildzip.sh > "${ZIPLOG}" 2>&1
	else
		echo "* Building ${1} zip"
		.zefie/scripts/buildzip.sh
	fi
	RC=$?
	if [ $RC -ne 0 ]; then
		echo "Error. Please check log. Code: $RC"
		exit $RC
	fi

	ZIPNAME=$(find build/out/ -name "boot_*.zip" | rev | cut -d'/' -f1 | rev)

	cp build/out/buildzip.log "${ZIPLOGD}"
	cp "build/out/${ZIPNAME}" "${LG_OUT_DIRECTORY}"/
else
	echo "Usage: ${0} MODEL [conf]"
	exit 1
fi

