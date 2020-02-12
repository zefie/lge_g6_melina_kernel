#!/bin/bash
# shellcheck disable=SC1090

SCRIPTDIR=$(realpath "$(dirname "${0}")")
source "${SCRIPTDIR}/buildenv.sh"

KERNDIR="$(pwd)"
OUTDIR="${KERNDIR}/build/out"
TMPDIR="${OUTDIR}/build"
MODDIR="${TMPDIR}/modules"
LOGFIL="${OUTDIR}/buildzip.log"
MODULES=0

KERNEL_IMAGE="build/arch/${ARCH}/boot/Image.gz-dtb"

if [ ! -f "${KERNEL_IMAGE}" ]; then
	echo "Could not find binary kernel. Did you build it?";
        echo ""
        echo "Try the following:"
        echo "zefie/scripts/build.sh clean build zip"
	exit 1;
fi

KVER=$(strings build/init/version.o | grep "Linux version" | cut -d' ' -f3 | cut -d'-' -f1-)
if [ -z "${USE_CLANG}" ]; then
	TC_VER=$("${TOOLCHAIN}gcc" --version | awk '/gcc /{print $0;exit 0;}')
else
	TC_VER=$("${Z_ANDROID_CLANG}/bin/clang" --version | awk '/clang /{print $0;exit 0;}' | cut -d':' -f1 | rev | cut -d'(' -f2- | cut -d' ' -f2- | rev)
	GCC_VER=$("${TOOLCHAIN}gcc" --version | awk '/gcc /{print $0;exit 0;}')
fi
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
  echo "*** Kernel Version: ${KVER}"; } >> "${LOGFIL}"
if [ ! -z "${GCC_VER}" ]; then
  echo "*** Toolchain Version: ${TC_VER} + ${GCC_VER}" >> "${LOGFIL}"
else
  echo "*** Toolchain Version: ${TC_VER}" >> "${LOGFIL}"
fi

errchk rm -rf "${TMPDIR}"
errchk mkdir -p "${TMPDIR}"
errchk echo " * Copying template to build directory"
errchk cp -rfv zefie/recovery_zip_template/* "${TMPDIR}" >> "${LOGFIL}"
errchk find "${TMPDIR}" -name "placeholder" -exec rm {} +
errchk echo " * Patching template ..."

MODEL_WHITELIST="${KERNEL_DEVMODEL_LOWER}"

errchk sed -i -e 's/\%MODEL_WHITELIST\%/'"${MODEL_WHITELIST}"'/' "${TMPDIR}/anykernel.sh"
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
	errchk "${SCRIPTDIR}/make.sh" INSTALL_MOD_PATH="${TMPDIR}/_modtmp" modules_install >> "${LOGFIL}" 2>&1

        # Rename exfat module for compatiblity (LG uses propritary Tuxera, we use open source)
	for m in "${INCLUDED_MODULES[@]}"; do
		if echo "${m}" | grep -q '='; then
			mout=$(echo "${m}" | cut -d'=' -f1)
			min=$(echo "${m}" | cut -d'=' -f2)
			echo " -*-  Found ${min}, placing as ${mout} ..." >> "${LOGFIL}"
		else
			min="${m}"
			mout="${m}"
		fi
		FOUND=0
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
	ANDROID_TARGET="debug-${ANDROID_TARGET}"
fi

# Standard naming
if [ -z "${USE_CLANG}" ]; then
	OUTFILE="boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}_${KVER}_$(date --utc +%Y.%m.%d)_${ANDROID_TARGET}_${KERNEL_DEVNAME}.zip"
else
	OUTFILE="boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}_${KVER}_$(date --utc +%Y.%m.%d)_clang_${ANDROID_TARGET}_${KERNEL_DEVNAME}.zip"
fi

if [ -f "${OUTDIR}/${OUTFILE}" ]; then
	rm -f "${OUTDIR}/${OUTFILE}"
fi

errchk cd "${TMPDIR}"
echo " * Creating zip..."
errchk zip -r9y "${OUTDIR}/${OUTFILE}" . >> "${LOGFIL}" 2>&1
errchk cd "${KERNDIR}"
echo ""
if [ -f "${OUTDIR}/${OUTFILE}" ]; then
	echo "Flashable zip available at:"
	echo "${OUTDIR}/${OUTFILE}"
	echo ""
	echo "Generated ${OUTDIR}/${OUTFILE}" >> "${LOGFIL}"
else
	echo "Could not create flashable zip. See above for possible error messages."
	echo "Error creating zip..." >> "${LOGFIL}"
	exit 1
fi
echo "*** Ending buildzip: $(date)" >> "${LOGFIL}"
echo "" >> "${LOGFIL}"
echo "*** Ending buildzip: $(date)"
