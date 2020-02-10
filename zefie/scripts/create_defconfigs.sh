#!/bin/bash
# shellcheck disable=SC1090,SC2034

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

DEFCONFIG_DIR="${KERNEL_SOURCE_DIR}/arch/${ARCH}/configs"
DEFCONFIG_OUTDIR="${DEFCONFIG_DIR}"

ORIG_DEFCONFIG_US997="lucye_nao_us-perf_defconfig"
ORIG_DEFCONFIG_H870="lucye_global_com-perf_defconfig"
ORIG_DEFCONFIG_H872="lucye_tmo_us-perf_defconfig"

# If you would like to add custom config easily without breaking melina build system, add it here then
# run this script to get your custom defconfig with melina additions.
# This can also override melina additions by setting the config value to n

read -r -d '' CUSTOM_CONFIG << EOM
# Add custom config below here, above EOM line

"${@}"
EOM

### Do not edit below this line ###

if [ ! -z "${KERNEL_RECOVERY}" ]; then
read -r -d '' EXTRA_CONFIG << EOM
# NTFS read only support for recovery kernel
CONFIG_NTFS_FS=y

# Enable permissive selinux for recovery kernel
CONFIG_SECURITY_SELINUX_DEVELOP=y
EOM
fi

for m in "${SUPPORTED_MODELS[@]}"; do
	DEVMODEL_LOWER="$(echo "$m" | tr '[:upper:]' '[:lower:]')"
	DEVMODEL_UPPER="$(echo "$m" | tr '[:lower:]' '[:upper:]')"
	ORIG_DEFCONFIG=$(echo -n "ORIG_DEFCONFIG_${DEVMODEL_UPPER}")
	ORIG_DEFCONFIG="${!ORIG_DEFCONFIG}"

	TARGET_FILE="${DEFCONFIG_OUTDIR}/${KERNEL_NAME_LOWER}_${DEVMODEL_LOWER}_defconfig"
	echo "*** Generating ${KERNEL_NAME}_${DEVMODEL_LOWER} kernel defconfigs..."
	rm -f "${TARGET_FILE}"
	cp -f "${DEFCONFIG_DIR}/${ORIG_DEFCONFIG}" "${TARGET_FILE}"
	if [ -z "${KERNEL_RECOVERY}" ]; then
		echo "CONFIG_LOCALVERSION=\"-${KERNEL_NAME_LOWER}\"" >> "${TARGET_FILE}"
	else
		echo "CONFIG_LOCALVERSION=\"-${KERNEL_NAME_LOWER}-recovery\"" >> "${TARGET_FILE}"
	fi

	# you dont have to change this from melina, it won't show to the user.
        # if you do, rename the files in arch/arm64/configs accordingly.

	cat "${DEFCONFIG_DIR}/melina_common_subconfig" >> "${TARGET_FILE}"
	if [ -f "${DEFCONFIG_DIR}/melina_${DEVMODEL_LOWER}_subconfig" ]; then
		cat "${DEFCONFIG_DIR}/melina_${DEVMODEL_LOWER}_subconfig" >> "${TARGET_FILE}"
	fi

	if [ ! -z "${EXTRA_CONFIG}" ]; then
		echo "${EXTRA_CONFIG}" >> "${TARGET_FILE}"
	fi
	echo "${CUSTOM_CONFIG}" >> "${TARGET_FILE}"

	if [ ! -z "${KERNEL_RECOVERY}" ]; then
		sed -i -e '/CONFIG_MODULE/d' "${TARGET_FILE}"
		sed -i -e 's/\=m/\=y/g' "${TARGET_FILE}"
	fi
done
