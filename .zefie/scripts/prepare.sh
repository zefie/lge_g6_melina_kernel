#!/bin/bash
STDIFS=${IFS}
DEFCONFIG_DIR="arch/${ARCH}/configs"

cp -r .zefie/scripts/dtbTool scripts/

IFS=$'\n'
#CPU Specific Optimizations=cpu_opts.patch
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

IFS=$'\n'
read -r -d '' COMMITS << EOM
EOM

for c in ${COMMITS}; do
	if [ ! -z "${c}" ]; then
		IFS=
		if [ -f ".zefie/patches/git/${c}.patch" ]; then
			COMMIT_DATA=$(cat ".zefie/patches/git/${c}.patch")
		else
			COMMIT_DATA=$(curl -s -k "https://git.zefie.net/zefie/android_kernel_samsung_msm8916/commit/${c}.patch")
			REMOTE=1
		fi
		PATCH_DATA=$(echo ${COMMIT_DATA} | sed -n '/^diff/,$p')
		PATCH_INFO=$(echo ${COMMIT_DATA} | grep "^Subject" | cut -d' ' -f3-)
		if [ -z "${REMOTE}" ]; then
			echo "*** Applying local patch: ${PATCH_INFO}"
		else
			echo "*** Applying remote patch: ${PATCH_INFO}"
		fi
		echo ${PATCH_DATA} | patch -s -p1
		RES=$?
		if [ ${RES} -ne 0 ]; then
			exit ${RES};
		fi
		IFS=${STDIFS}
	fi
done

# Melina Kernel Custom Config Options
#read -r -d '' CONFIG << EOM
#EOM

if [ ! -z "${EXTRA_CONFIG}" ]; then
	CONFIG+="$(echo; echo "${EXTRA_CONFIG}";)"
fi

echo "*** Generating ${KERNEL_NAME} kernel defconfig..."
rm -f ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}
cp -f ${DEFCONFIG_DIR}/${ORIG_DEFCONFIG} ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}

if [ ! -z "${CONFIG}" ]; then
	echo "${CONFIG}" >> ${DEFCONFIG_DIR}/${KERNEL_DEFCONFIG}
fi
