#!/bin/bash
source .zefie/scripts/buildenv.sh
KERNDIR="$(pwd)"
OUTDIR="${KERNDIR}/build/out"
TMPDIR="${OUTDIR}/build"
MODDIR="${TMPDIR}/modules"
LOGFIL="${OUTDIR}/buildzip.log"
MODULES=1

KERNEL_IMAGE="build/arch/${ARCH}/boot/Image.gz-dtb"

if [ ! -f "${KERNEL_IMAGE}" ]; then
	echo "Could not find binary kernel. Did you build it?";
        echo ""
        echo "Try all of the following:"
        echo ".zefie/scripts/resetgit.sh"
        echo ".zefie/scripts/cleanbuild.sh"
        echo ".zefie/scripts/buildzip.sh"
	exit 1;
fi

KVER=$(strings build/init/version.o | grep "Linux version" | cut -d' ' -f3 | cut -d'-' -f1-)
TCVER=$("${TOOLCHAIN}gcc" --version | awk '/gcc /{print $0;exit 0;}')
if [ ${MODULES} -eq 1 ]; then
## If you would like to add a custom module to your ROM
## add it's filename on its own line anywhere between the words INCLUDED.
## After building, try: find build -name "*.ko"
## to find all built modules.

read -r -d '' INCLUDED_MODULES << INCLUDED
ansi_cprng.ko
bluetooth-power.ko
br_netfilter.ko
dummy_hcd.ko
evbug.ko
g_laf.ko
gspca_main.ko
lcd.ko
mausb-core.ko
mausb-device.ko
mmc_block_test.ko
mmc_test.ko
rdbg.ko
spidev.ko
tcp_htcp.ko
tcp_westwood.ko
test-iosched.ko
texfat.ko=exfat.ko
ufs_test.ko
INCLUDED

	if [ ! -z "${EXTRA_MODULES}" ]; then
		INCLUDED_MODULES+="$(echo; echo "${EXTRA_MODULES}";)"
	fi
fi

mkdir -p "${OUTDIR}"
echo "*** Starting buildzip: $(date)" >> "${LOGFIL}"
echo "*** Starting buildzip: $(date)"
echo "*** Kernel Name: ${KERNEL_NAME}" >> "${LOGFIL}"
echo "*** Kernel Device: ${KERNEL_MANU} ${KERNEL_MODEL} (${KERNEL_DEVMODEL})" >> "${LOGFIL}"
echo "*** Kernel Version: ${KVER}" >> "${LOGFIL}"

rm -rf "${TMPDIR}"
mkdir -p "${TMPDIR}"
echo " * Copying template to build directory"
cp -rfv .zefie/recovery_zip_template/* "${TMPDIR}" >> "${LOGFIL}"
find "${TMPDIR}" -name "placeholder" -exec rm {} +
echo " * Patching template ..."

MODEL_WHITELIST="${KERNEL_DEVMODEL_LOWER}"

sed -i -e 's/\%MODEL_WHITELIST\%/'"${MODEL_WHITELIST}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%KERNELDEV\%/'"${KERNEL_DEVNAME}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%NAME\%/'"${KERNEL_NAME}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%MANU\%/'"${KERNEL_MANU}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%MODEL\%/'"${KERNEL_MODEL}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%DEVMODEL\%/'"${KERNEL_DEVMODEL}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%VERSION\%/'"${KVER}"'/' "${TMPDIR}/anykernel.sh"
sed -i -e 's/\%TOOLCHAIN_VERSION\%/'"${TCVER}"'/' "${TMPDIR}/anykernel.sh"

if [ ${MODULES} -eq 1 ]; then
	rm -rf "${TMPDIR}/_modtmp"
	mkdir -p "${TMPDIR}/_modtmp"
	.zefie/scripts/make.sh INSTALL_MOD_PATH="${TMPDIR}/_modtmp" modules_install 2>&1 >> "${LOGFIL}"

        # Rename exfat module for compatiblity (LG uses propritary Tuxera, we use open source)
	for m in ${INCLUDED_MODULES}; do
		if [ ! -z $(echo ${m} | grep '=') ]; then
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
			cp -f "${FILE}" "${MODDIR}/${mout}"
		else
			echo " * [MISSN] ${mout}" >> /dev/stderr
		fi
	done;
	chmod 644 "${MODDIR}/"*
	rm -rf "${TMPDIR}/_modtmp"
fi

cp "${KERNEL_IMAGE}" "${TMPDIR}/zImage"
#echo " * Generating QCDT..."
#build/scripts/dtbTool/dtbTool -o "${TMPDIR}/dtb" -d build/scripts/dtc/dtc build/arch/arm64/boot/dts/ 2>&1 >> "${LOGFIL}"

if [ ! -z "${TC_VER}" ]; then
	# zefie's layout enhanced naming
	OUTFILE="boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}_${KVER}_$(date --utc +%Y.%m.%d)_${TC_NAME}-${TC_VER}_${ANDROID_TARGET}_${KERNEL_DEVNAME}.zip"
else
	# Standard naming
	OUTFILE="boot_${KERNEL_MANU}-${KERNEL_DEVMODEL}_${KVER}_$(date --utc +%Y.%m.%d)_${ANDROID_TARGET}_${KERNEL_DEVNAME}.zip"
fi

if [ -f "${OUTDIR}/${OUTFILE}" ]; then
	rm -f "${OUTDIR}/${OUTFILE}"
fi

cd "${TMPDIR}"
echo " * Creating zip..."
zip -r9y "${OUTDIR}/${OUTFILE}" . 2>&1 >> "${LOGFIL}"
cd "${KERNDIR}"
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
