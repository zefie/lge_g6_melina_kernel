#!/bin/bash
# shellcheck disable=SC1090,SC2034

if [ -z "${SCRIPTDIR}" ]; then
	echo "This script is meant to be included by the buildenv script."
	exit 1;
fi

function z_setlog() {
	if [ -z "${1}" ]; then
		unset Z_ERROR_LOG
	else
		Z_ERROR_LOG="${1}"
		export Z_ERROR_LOG
	fi
}

function errout() {
	if { 2>/dev/null true >&3; }; then
		echo "$@" 1>&3;
	else
		echo "$@" 1>&2;
	fi
}

function errchk() {
        case "${1}" in
                "silent")
                        local silent=1
                        shift;
                        ;;
                "noout")
                        local noout=1
                        shift;
                        ;;
                "stayalive")
                        local stayalive=1
                        shift;
                        ;;
        esac
        if [ -z "${noout}" ]; then
                "${@}"
        else
                "${@}" 2>/dev/null >/dev/null
        fi
        local res=$?
        if [ ${res} -ne 0 ]; then
                if [ -z "${silent}" ]; then
                        errout "Error ${res} executing ${*}";
			if [ ! -z "${Z_ERROR_LOG}" ]; then
				errout "*** BUILD ERROR *** "
				errout "* Device: ${KERNEL_DEVMODEL}"
				errout "Build Log:"
				errout "$(cat "${Z_ERROR_LOG}")"
	                fi
		fi
		if [ -z "${stayalive}" ]; then
			exit "${res}"
		else
			return "${res}";
		fi
        fi
}

function kernel_setdevice() {
	local USER_DEV
	USER_DEV="$(echo "${1}" | tr '[:lower:]' '[:upper:]')"
	for m in "${SUPPORTED_MODELS[@]}"; do
		if [ "${USER_DEV}" == "${m}" ]; then
			KERNEL_DEVMODEL="${USER_DEV}";
			KERNEL_DEVMODEL_LOWER="$(echo "${KERNEL_DEVMODEL}" | tr '[:upper:]' '[:lower:]')"
			KERNEL_DEVPETNAME="$(kernel_get_device_defconfig "${m}" | cut -d'_' -f1)"
			case "${KERNEL_DEVPETNAME}" in
				"lucye")
					KERNEL_MODEL="G6"
					;;
				"elsa")
					KERNEL_MODEL="G5"
					;;
				*)
					KERNEL_MODEL="??"
					;;
			esac
			export KERNEL_DEVMODEL KERNEL_DEVMODEL_LOWER KERNEL_MODEL KERNEL_DEVPETNAME
			return 0;
		fi
	done

	echo "Error: Unknown model (${USER_DEV})";
	echo "This script supports: ${SUPPORTED_MODELS[*]}";
	exit 1
}

function kernel_make_gcc() {
	errchk mkdir -p "${KERNEL_BUILD_DIR}"
	errchk make O="${KERNEL_BUILD_DIR}" CROSS_COMPILE="${TOOLCHAIN}" CROSS_COMPILE_ARM32="${TOOLCHAIN32}" "${@}"
}

function kernel_make_clang() {
	local PATH="${Z_ANDROID}/prebuilts/build-tools/linux-x86/bin:${Z_ANDROID_CLANG}/bin:${PATH}"
	local LD_LIBRARY_PATH="${Z_ANDROID}/prebuilts/build-tools/linux-x86/lib:${Z_ANDROID_CLANG}/lib64:${LD_LIBRARY_PATH}"
	local PERL5LIB="${Z_ANDROID}/prebuilts/tools-extras/common/perl-base"
	local MAKE="${Z_ANDROID}/prebuilts/build-tools/linux-x86/bin/make"

	local CC=" clang"
	local HOSTCC="${Z_ANDROID_CLANG}/bin/clang"
	local HOSTCXX="${Z_ANDROID_CLANG}/bin/clang++"

	if [ ! -z "${USE_CCACHE}" ]; then
		CC="ccache ${CC}"
		HOSTCC="ccache ${HOSTCC}"
		HOSTCXX="ccache ${HOSTCXX}"
	fi
	local KENV=( \
		"CFLAGS_MODULE=-fno-pic" "HOSTCFLAGS=-I/usr/include -I/usr/include/x86_64-linux-gnu" \
		"O=${KERNEL_BUILD_DIR:?}" "ARCH=${ARCH}" "CC=${CC}" "CLANG_TRIPLE=aarch64-linux-gnu" "HOSTCC=${HOSTCC}" \
		"HOSTCXX=${HOSTCXX}" "HOSTAR=${Z_ANDROID_BINUTILS}/ar" "HOSTLD=${Z_ANDROID_BINUTILS}/ld" "BISON=$(which bison)" \
		"FLEX=$(which flex)" "PERL=$(which perl)" "CROSS_COMPILE=${TOOLCHAIN}" "CROSS_COMPILE_ARM32=${TOOLCHAIN32}" \
		"${@}" \
	)

	errchk mkdir -p "${KERNEL_BUILD_DIR}"
	errchk "${MAKE}" "${KENV[@]}"
}

function kernel_make() {
	MAKE_CMDS=("-j${CPUS}")
	if [ -z "${1}" ]; then
		MAKE_CMDS+=("${KERNEL_MAKE_TARGETS[@]}")
	else
		MAKE_CMDS+=("${@}")
	fi
	if [ -z "${USE_CLANG}" ]; then
		errchk kernel_make_gcc "${MAKE_CMDS[@]}"
	else
		errchk kernel_make_clang "${MAKE_CMDS[@]}"
	fi
}

function kernel_mrproper() {
	TOOLCHAIN32="${TOOLCHAIN32}gcc" kernel_make_gcc mrproper
}

function kernel_git_reset {
	echo "Resetting source tree..."
	rm -f .config .config.old
	rm -rf "${KERNEL_BUILD_DIR:?}/"
	git reset --hard
	exit $?
}

function kernel_git_clean {
	if [ -d ".git" ]; then
		if git diff-index --name-only HEAD | grep -qv "^scripts/package"; then
			if git diff-index --name-only HEAD | grep -qv "^zefie"; then
				kernel_git_reset
			fi
		fi
	fi
}

function kernel_clean() {
	rm -rf "${KERNEL_SOURCE_DIR:?}/${KERNEL_BUILD_DIR:?}"
	rm -rf "${KERNEL_BUILD_DIR:?}"
	rm -f "${DEFCONFIG_DIR}/${KERNEL_NAME_LOWER}_"*_defconfig
	errchk kernel_git_clean
	errchk kernel_mrproper
}


function kernel_get_device_defconfig() {
	local MODEL
	MODEL=$(echo "${1}" | tr '[:lower:]' '[:upper:]')

	# start model list
	case "${MODEL}" in
			"US997")
				echo lucye_nao_us-perf_defconfig
				;;
			"H870")
				echo lucye_global_com-perf_defconfig
				;;
			"H872")
				echo lucye_tmo_us-perf_defconfig
				;;
			"H990DS")
				echo elsa_global_com-perf_defconfig
				;;
			"H990TR")
				echo elsa_cno_cn-perf_defconfig
				;;
			"US996")
				echo elsa_nao_us-perf_defconfig
				;;
			"US996SANTA")
				echo elsa_nao_us_dirty-perf_defconfig
				;;
			"LS997")
				echo elsa_spr_us-perf_defconfig
				;;
			"VS995")
				echo elsa_vzw-perf_defconfig
				;;
			"H918")
				echo elsa_tmo_us-perf_defconfig
				;;
			"H910")
				echo elsa_att_us-perf_defconfig
				;;
			"H915")
				echo elsa_global_ca-perf_defconfig
				;;
			"F800K")
				echo elsa_kt_kr-perf_defconfig
				;;
			"F800L")
				echo elsa_lgu_kr-perf_defconfig
				;;
			"F800S")
				echo elsa_skt_kr-perf_defconfig
				;;
			*)
				return 1;
				;;
	esac
	# end model list
}

function kernel_generate_defconfigs() {
	local DEFCONFIG_DIR DEFCONFIG_OUTDIR ORIG_DEFCONFIG_US997 ORIG_DEFCONFIG_H870 \
	      ORIG_DEFCONFIG_H872 CUSTOM_CONFIG Z_DEVMODEL_LOWER Z_DEVMODEL_UPPER \
	      ORIG_DEFCONFIG TARGET_FILE EXTRA_CONFIG

	DEFCONFIG_DIR="${KERNEL_SOURCE_DIR}/arch/${ARCH}/configs"
	DEFCONFIG_OUTDIR="${DEFCONFIG_DIR}"

	CUSTOM_CONFIG=("${@}")

	if [ ! -z "${USE_DEBUG}" ]; then
		CUSTOM_CONFIG+=("CONFIG_MELINA_DEBUG_DISABLE=n" "CONFIG_MELINA_DEBUG_ENABLE=y")
	fi

	if [ ! -z "${KERNEL_RECOVERY}" ]; then
		EXTRA_CONFIG=( \
			"CONFIG_NTFS_FS=y" \
			"CONFIG_SECURITY_SELINUX_DEVELOP=y" \
		)
	fi

	for m in "${SUPPORTED_MODELS[@]}"; do
		if [ -z "${m}" ]; then continue; fi
		Z_DEVMODEL_LOWER="$(echo "$m" | tr '[:upper:]' '[:lower:]')"
		Z_DEVMODEL_UPPER="$(echo "$m" | tr '[:lower:]' '[:upper:]')"
		ORIG_DEFCONFIG="$(kernel_get_device_defconfig "${Z_DEVMODEL_UPPER}")"
		if [ -z "${ORIG_DEFCONFIG}" ]; then continue; fi

		TARGET_FILE="${DEFCONFIG_OUTDIR}/${KERNEL_NAME_LOWER}_${Z_DEVMODEL_LOWER}_defconfig"
		echo "*** Generating arch/${ARCH}/configs/${KERNEL_NAME_LOWER}_${Z_DEVMODEL_LOWER}_defconfig"
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
		if [ -f "${DEFCONFIG_DIR}/melina_${Z_DEVMODEL_LOWER}_subconfig" ]; then
			cat "${DEFCONFIG_DIR}/melina_${Z_DEVMODEL_LOWER}_subconfig" >> "${TARGET_FILE}"
		fi

		for l in "${EXTRA_CONFIG[@]}"; do
			echo "${l}" >> "${TARGET_FILE}"
		done

		for l in "${CUSTOM_CONFIG[@]}"; do
			echo "${l}" >> "${TARGET_FILE}"
		done

		# Force disable modules
		if [ ! -z "${KERNEL_RECOVERY}" ]; then
			sed -i -e '/CONFIG_MODULE/d' "${TARGET_FILE}"
			sed -i -e 's/\=m/\=y/g' "${TARGET_FILE}"
		fi
	done
}

function kernel_make_defconfig() {
	if [ ! -f "${DEFCONFIG_DIR}/${KERNEL_NAME_LOWER}_${KERNEL_DEVMODEL_LOWER}_defconfig" ]; then
		kernel_generate_defconfigs "${Z_CONFIG_OVERRIDE[@]}"
	fi
	local DEFCONFIG="${KERNEL_NAME_LOWER}_${KERNEL_DEVMODEL_LOWER}_defconfig"
	echo "Using ${DEFCONFIG}..."
	kernel_make "${DEFCONFIG}"
}

function sideload() {
	local LATEST
	LATEST=$(find "build/out/boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}"*.zip -printf "%T@ %Tc %p\n" | sort -nr | head -n1 | rev | cut -d' ' -f1 | rev)
	if [ -f "${LATEST}" ]; then
		adb sideload "${LATEST}"
	else
		errout "Could not find a zip? Did you build the kernel and zip file?";
		errout ""
		errout "Try the following:"
		errout "zefie/scripts/build.sh build"
		errour "zefie/scripts/build.sh sideload"
		exit 1;
	fi
}

function buildzip() {
	local KERNDIR OUTDIR TMPDIR MODDIR LOGFILE MODULES KERNEL_IMAGE \
	      KVER TC_VER GCC_VER INCLUDED_MODULES MODEL_WHITELIST MODEL_WHITELIST2="" \
	      EXTRA_CMDS EXTRA_CMDS_STR ANDROID_TARGET OUTFILE

	KERNDIR="$(pwd)"
	OUTDIR="${KERNDIR}/build/out"
	TMPDIR="${OUTDIR}/buildzip"
	LOGFILE="${OUTDIR}/buildzip.log"
	MODDIR="${TMPDIR}/modules"
	MODULES=1

	KERNEL_IMAGE="build/arch/${ARCH}/boot/Image.gz-dtb"

	if [ ! -f "${KERNEL_IMAGE}" ]; then
		errout "Could not find binary kernel. Did you build it?";
	        errout ""
	        errout "Try the following:"
	        errout "zefie/scripts/build.sh build"
		exit 1;
	fi

	KVER=$(strings build/init/version.o | grep "Linux version" | cut -d' ' -f3 | cut -d'-' -f1-)
	if [ -z "${USE_CLANG}" ]; then
		TC_VER=$("${TOOLCHAIN}gcc" --version | awk '/gcc /{print $0;exit 0;}')
	else
		TC_VER=$("${Z_ANDROID_CLANG}/bin/clang" --version | awk '/clang /{print $0;exit 0;}' | cut -d':' -f1 | rev | cut -d'(' -f2- | cut -d' ' -f2- | rev)
		GCC_VER=$("${TOOLCHAIN}gcc" --version | awk '/gcc /{print $0;exit 0;}')
	fi

	# Modules section was originally designed for stock LGE roms
	if [ ${MODULES} -eq 1 ]; then
		## If you would like to add a custom module to your ROM
		## add it's filename on its own line anywhere between the words INCLUDED.
		## After building, try: find build -name "*.ko"
		## to find all built modules.

		INCLUDED_MODULES=( \
			ansi_cprng.ko \
			bluetooth-power.ko \
			br_netfilter.ko \
			dummy_hcd.ko \
			evbug.ko \
			g_laf.ko \
			gspca_main.ko \
			lcd.ko \
			mausb-core.ko \
			mausb-device.ko \
			mmc_block_test.ko \
			mmc_test.ko \
			rdbg.ko \
			spidev.ko \
			tcp_htcp.ko \
			tcp_westwood.ko \
			test-iosched.ko \
			"texfat.ko=exfat.ko" \
			ufs_test.ko \
		)

		if [ ! -z "${EXTRA_MODULES}" ]; then
			INCLUDED_MODULES+=(${EXTRA_MODULES[@]});
		fi
	fi

	errchk mkdir -p "${OUTDIR}"
	echo "*** Starting buildzip: $(date)"

	{ echo "*** Starting buildzip: $(date)"; \
	  echo "*** Kernel Name: ${KERNEL_NAME}"; \
	  echo "*** Kernel Device: ${KERNEL_MANU} ${KERNEL_MODEL} (${KERNEL_DEVMODEL})"; \
	  echo "*** Kernel Version: ${KVER}"; } >> "${LOGFILE}"
	if [ ! -z "${GCC_VER}" ]; then
	  echo "*** Toolchain Version: ${TC_VER} + ${GCC_VER}" >> "${LOGFILE}"
	else
	  echo "*** Toolchain Version: ${TC_VER}" >> "${LOGFILE}"
	fi

	errchk rm -rf "${TMPDIR}"
	errchk mkdir -p "${TMPDIR}"
	errchk echo " * Copying template to build directory"
	errchk cp -rfv zefie/recovery_zip_template/* "${TMPDIR}" >> "${LOGFILE}"
	errchk find "${TMPDIR}" -name "placeholder" -exec rm {} +
	errchk echo " * Patching template ..."

	MODEL_WHITELIST="${KERNEL_DEVMODEL_LOWER}"
	if [ "${KERNEL_DEVMODEL_UPPER}" == "US996SANTA" ]; then
		MODEL_WHITELIST2="us996"
	fi

	errchk sed -i -e 's/\%MODEL_WHITELIST\%/'"${MODEL_WHITELIST}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%MODEL_WHITELIST2\%/'"${MODEL_WHITELIST2}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%KERNELDEV\%/'"${KERNEL_DEVNAME}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%NAME\%/'"${KERNEL_NAME}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%MANU\%/'"${KERNEL_MANU}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%MODEL\%/'"${KERNEL_MODEL}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%DEVMODEL\%/'"${KERNEL_DEVMODEL}"'/' "${TMPDIR}/anykernel.sh"
	errchk sed -i -e 's/\%VERSION\%/'"${KVER}"'/' "${TMPDIR}/anykernel.sh"
	EXTRA_CMDS=();
	if [ -z "${USE_CLANG}" ]; then
		errchk sed -i -e 's/\%TOOLCHAIN_VERSION\%/'"${TC_VER}"'/' "${TMPDIR}/anykernel.sh"
		errchk sed -i -e 's/\%EXTRA_CMDS\%//' "${TMPDIR}/anykernel.sh"
	else
		errchk sed -i -e 's/\%TOOLCHAIN_VERSION\%/'"${GCC_VER}"'/' "${TMPDIR}/anykernel.sh"
		EXTRA_CMDS+=("ui_print \"clang: ${TC_VER}\"")
	fi

	if [ -z "${USE_DEBUG}" ]; then
		EXTRA_CMDS+=("ui_print \"This is a release (non-debug) kernel.\"")

	else
		EXTRA_CMDS+=("ui_print \"***************************\"")
		EXTRA_CMDS+=("ui_print \"NOTICE NOTICE NOTICE NOTICE\"")
		EXTRA_CMDS+=("ui_print \"***************************\"")
		EXTRA_CMDS+=("ui_print \"\"")
		EXTRA_CMDS+=("ui_print \"This is a DEBUG kernel.\"")
		EXTRA_CMDS+=("ui_print \"If you are not a developer or trying to\"")
		EXTRA_CMDS+=("ui_print \"debug issues, you should use the \"")
		EXTRA_CMDS+=("ui_print \"non-debug version of this kernel.\"")
	fi

	EXTRA_CMDS_STR=""
	for c in "${EXTRA_CMDS[@]}"; do
		EXTRA_CMDS_STR+="${c}\n"
	done
	errchk sed -i -e 's/\%EXTRA_CMDS\%/'"${EXTRA_CMDS_STR}"'/' "${TMPDIR}/anykernel.sh"

	if [ ${MODULES} -eq 1 ]; then
		errchk rm -rf "${TMPDIR}/_modtmp"
		errchk mkdir -p "${TMPDIR}/_modtmp"
		errchk mkdir -p "${MODDIR}"
		errchk echo " * Building modules ..."
		errchk kernel_make modules >> "${LOGFILE}" 2>&1
		errchk echo " * Preparing modules ..."
		errchk kernel_make INSTALL_MOD_PATH="${TMPDIR}/_modtmp" modules_install >> "${LOGFILE}" 2>&1

	        # Rename exfat module for compatiblity (LG uses propritary Tuxera, we use open source)
		for m in "${INCLUDED_MODULES[@]}"; do
			if echo "${m}" | grep -q '='; then
				mout=$(echo "${m}" | cut -d'=' -f1)
				min=$(echo "${m}" | cut -d'=' -f2)
				echo " -*-  Found ${min}, placing as ${mout} ..." >> "${LOGFILE}"
			else
				min="${m}"
				mout="${m}"
			fi
			local FOUND=0
			FILE=$(find "${TMPDIR}/_modtmp" -name "${min}")
			if [ -f "${FILE}" ]; then
				FOUND=1
			fi

			if [ "${FOUND}" -eq "1" ]; then
				echo " * [FOUND] ${mout}"
				errchk cp -f "${FILE}" "${MODDIR}/${mout}"
				errchk chmod 644 "${MODDIR}/${mout}"
			else
				echo " * [MISSN] ${mout}" >> /dev/stderr
			fi
		done;
		errchk rm -rf "${TMPDIR}/_modtmp"
	fi

	errchk cp "${KERNEL_IMAGE}" "${TMPDIR}/zImage"

	# Debug Naming
	if [ ! -z "${USE_DEBUG}" ]; then
		ANDROID_TARGET="debug";
	else
		ANDROID_TARGET="nondebug";
	fi

	if [ ! -z "${USE_CLANG}" ]; then
		ANDROID_TARGET="clang_${ANDROID_TARGET}"
	fi

	# Standard naming
	OUTFILE="boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}_${KVER}_$(date --utc +%Y.%m.%d)_${ANDROID_TARGET}_${KERNEL_DEVNAME}.zip"

	if [ -f "${OUTDIR}/${OUTFILE}" ]; then
		rm -f "${OUTDIR}/${OUTFILE}"
	fi

	errchk cd "${TMPDIR}"
	echo " * Creating zip..."
	errchk zip -r9y "${OUTDIR}/${OUTFILE}" . >> "${LOGFILE}" 2>&1
	errchk cd "${KERNDIR}"
	echo ""
	if [ -f "${OUTDIR}/${OUTFILE}" ]; then
		echo "Flashable zip available at:"
		echo "${OUTDIR}/${OUTFILE}"
		echo ""
		echo "Generated ${OUTDIR}/${OUTFILE}" >> "${LOGFILE}"
	else
		echo "Could not create flashable zip. See above for possible error messages."
		echo "Error creating zip..." >> "${LOGFILE}"
		exit 1
	fi
	echo "*** Ending buildzip: $(date)" >> "${LOGFILE}"
	echo "" >> "${LOGFILE}"
	echo "*** Ending buildzip: $(date)"
}

function kernel_release_build() {
	local res=0;

	mkdir -p "${LG_OUT_DIRECTORY}"

	KERNLOG="${LG_OUT_DIRECTORY}/${KERNEL_NAME_LOWER}-kernel-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOG="${LG_OUT_DIRECTORY}/${KERNEL_NAME_LOWER}-buildzip-${KERNEL_DEVMODEL_LOWER}.log"
	ZIPLOGD="${LG_OUT_DIRECTORY}/${KERNEL_NAME_LOWER}-buildzip_details-${KERNEL_DEVMODEL_LOWER}.log"


	if [ -z "${USE_CLANG}" ]; then
		echo "* gcc build"
	else
		echo "* clang build"
	fi

	if [ -z "${USE_DEBUG}" ]; then
		echo "* Production (non-debug) kernel"
	else
		echo "* DEBUG kernel"
		Z_BUILD_ARG+=(debug)
	fi

	if [ -z "${WORKSPACE}" ]; then
		echo "* Cleaning old ${KERNEL_MANU} ${KERNEL_MODEL} ${KERNEL_DEVMODEL} log files..."
		errchk rm -f "${LG_OUT_DIRECTORY}/"*"-${KERNEL_DEVMODEL_LOWER}.log"
	fi

	Z_BUILD_CMDS=(kernel_clean kernel_generate_defconfigs kernel_make_defconfig kernel_make)
	z_setlog "${KERNLOG}"
	echo "* Building clean ${KERNEL_MANU} ${KERNEL_MODEL} ${KERNEL_DEVMODEL} kernel (log in ${KERNLOG})"
	for c in "${Z_BUILD_CMDS[@]}"; do
		3>&2 "${c}" > "${KERNLOG}" 2>&1
	done

	z_setlog "${ZIPLOG}"
	echo "* Building ${KERNEL_MANU} ${KERNEL_MODEL} ${KERNEL_DEVMODEL} zip (log in ${ZIPLOG})"
	3>&1 buildzip > "${ZIPLOG}" 2>&1
	z_setlog

	ZIPNAME=$(find build/out/ -name "boot_*.zip" | rev | cut -d'/' -f1 | rev)

	errchk cp build/out/buildzip.log "${ZIPLOGD}"
	errchk cp "build/out/${ZIPNAME}" "${LG_OUT_DIRECTORY}"/
}
