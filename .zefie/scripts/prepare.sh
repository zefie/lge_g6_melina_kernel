#!/bin/bash
STDIFS=${IFS}
DEFCONFIG_DIR="arch/${ARCH}/configs"

cp -r .zefie/scripts/dtbTool scripts/

IFS=$'\n'
#patch_desc=file.patch
read -r -d '' CUSTOM_LOCAL << EOM
EOM

for c in ${CUSTOM_LOCAL}; do
	IFS=
	PATCH_INFO=$(echo "${c}" | cut -d'=' -f1)
	PATCH_FILE=$(echo "${c}" | cut -d'=' -f2)
	if [ -f ".zefie/patches/${PATCH_FILE}" ]; then
		echo "*** Applying local patch: ${PATCH_INFO}"
		patch -s -p1 < ".zefie/patches/${PATCH_FILE}";
		RES=$?
		if [ ${RES} -ne 0 ]; then
			exit ${RES};
		fi
	fi
	IFS=${STDIFS}
done;

# Melina Kernel Custom Config Options
read -r -d '' CONFIG << EOM
CONFIG_LOCALVERSION=""
EOM

if [ ! -z "${EXTRA_CONFIG}" ]; then
	CONFIG+="$(echo; echo "${EXTRA_CONFIG}";)"
fi

echo "*** Generating ${KERNEL_NAME} kernel defconfig..."
rm -f ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}
cp -f ${DEFCONFIG_DIR}/${ORIG_DEFCONFIG} ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}

if [ ! -z "${CONFIG}" ]; then
	echo "${CONFIG}" >> ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}
fi
