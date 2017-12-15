#!/bin/bash
source .zefie/scripts/buildenv.sh
KERNDIR="$(pwd)"
OUTDIR="${KERNDIR}/build/out"
TMPDIR="${OUTDIR}/build"
MODDIR="${TMPDIR}/modules"
MODULES=0

KERNEL_IMAGE="build/arch/${ARCH}/boot/Image.${KERNEL_COMPRESSION_SUFFIX}-dtb"

if [ ! -f "${KERNEL_IMAGE}" ]; then
	echo "Could not find binary kernel. Did you build it?";
        echo ""
        echo "Try the following:"
        echo ".zefie/scripts/cleanbuild.sh"
        echo ".zefie/scripts/buildzip.sh"
	exit 1;
fi

#KVER=$(strings build/init/version.o | grep "Linux version" | cut -d' ' -f3 |  cut -d'-' -f3-)
KVER=$(strings build/init/version.o | grep "Linux version" | cut -d' ' -f3 | cut -d'-' -f1-)

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
texfat.ko
ufs_test.ko
INCLUDED

	#if [ ! -z "${ANDROID_MARSHMALLOW}" ]; then
	#read -r -d '' EXTRA_MODULES << INCLUDED
	#wlan.ko
	#INCLUDED

	if [ ! -z "${EXTRA_MODULES}" ]; then
		INCLUDED_MODULES+="$(echo; echo "${EXTRA_MODULES}";)"
	fi
fi

mkdir -p "${OUTDIR}"
echo "*** Starting buildzip: $(date)" >> "${OUTDIR}/buildzip.log"
echo "*** Starting buildzip: $(date)"

rm -rf "${TMPDIR}"
mkdir -p "${TMPDIR}"
echo " * Copying template to build directory"
cp -rfv .zefie/scripts/recovery_zip_template/* "${TMPDIR}" >> "${OUTDIR}/buildzip.log"
find "${TMPDIR}" -name "placeholder" -exec rm {} +
echo " * Patching template version: ${KVER}"
sed -i -e 's/\%VERSION\%/'"${KVER}"'/' "${TMPDIR}/anykernel.sh"

if [ ${MODULES} -eq 1 ]; then
	rm -rf "${TMPDIR}/_modtmp"
	mkdir -p "${TMPDIR}/_modtmp"
	.zefie/scripts/make.sh INSTALL_MOD_PATH="${TMPDIR}/_modtmp" modules_install 2>&1 >> "${OUTDIR}/buildzip.log"

	for m in ${INCLUDED_MODULES}; do
		FOUND=0
		FILE=$(find "${TMPDIR}/_modtmp" -name "${m}")
		if [ -f "${FILE}" ]; then
			FOUND=1
		fi

		if [ "${FOUND}" -eq "1" ]; then
			echo " * [FOUND] ${m}"
			cp -f "${FILE}" "${MODDIR}/${m}"
		else
			echo " * [MISSN] ${m}" > /dev/stderr
		fi
	done;
	chmod 644 "${MODDIR}/"*

	# Rename wifi module for compatiblity
	if [ -f "${MODDIR}/wlan.ko" ]; then
		mkdir "${MODDIR}/pronto"
		mv "${MODDIR}/wlan.ko" "${MODDIR}/pronto/pronto_wlan.ko"
	fi

	rm -rf "${TMPDIR}/_modtmp"
fi

cp "${KERNEL_IMAGE}" "${TMPDIR}/zImage"
#echo " * Generating QCDT..."
#build/scripts/dtbTool/dtbTool -o "${TMPDIR}/dtb" -d build/scripts/dtc/dtc build/arch/arm64/boot/dts/ 2>&1 >> "${OUTDIR}/buildzip.log"

OUTFILE="${KERNEL_NAME}_kernel_$(date --utc +%Y.%m.%d)_${KVER}_us997_${ANDROID_TARGET}_${USER}.zip"

if [ -f "${OUTDIR}/${OUTFILE}" ]; then
	rm -f "${OUTDIR}/${OUTFILE}"
fi

cd "${TMPDIR}"
echo " * Creating zip..."
zip -r9y "${OUTDIR}/${OUTFILE}" . 2>&1 >> "${OUTDIR}/buildzip.log"
cd "${KERNDIR}"
echo ""
echo "Flashable zip available at:"
echo "${OUTDIR}/${OUTFILE}"
echo ""
echo "Generated ${OUTDIR}/${OUTFILE}" >> "${OUTDIR}/buildzip.log"
echo "*** Ending buildzip: $(date)" >> "${OUTDIR}/buildzip.log"
echo "*** Ending buildzip: $(date)"

