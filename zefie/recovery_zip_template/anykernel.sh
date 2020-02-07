# AnyKernel2 Ramdisk Mod Script
# osm0sis @ xda-developers

## AnyKernel setup
# EDIFY properties
kernel.string="%NAME% kernel for %MANU% %MODEL% by %KERNELDEV%"
do.devicecheck=1
do.modules=0 # we will do it ourself.
do.cleanup=1
do.cleanuponabort=0
device.name1=%MODEL_WHITELIST%
device.name2=
device.name3=
device.name4=
device.name5=

# shell variables
block=/dev/block/bootdevice/by-name/boot;
is_slot_device=0;
force_seandroid=1;

## AnyKernel methods (DO NOT CHANGE)
# import patching functions/variables - see for reference
. /tmp/anykernel/tools/ak2-core.sh;

## zefie kernel info

ui_print " "
ui_print "Kernel Device: %MANU% %MODEL% (%DEVMODEL%)"
ui_print "Kernel Name: %NAME%"
ui_print "Kernel Maintainer: %KERNELDEV%"
ui_print "Kernel Version: %VERSION%"
ui_print "Toolchain: %TOOLCHAIN_VERSION%"
ui_print " "

## AnyKernel install

# Dump initrd
dump_boot;

# remove /sbin/rctd if it exists
if [ -f "$ramdisk/sbin/rctd" ]; then
	PATCHED=1
	ui_print "Removing /sbin/rctd..."
	rm -f "$ramdisk/sbin/rctd"
fi

if [ -f "$ramdisk/init.lge.rc" ]; then
	if [ "$(grep /sbin/rctd $ramdisk/init.lge.rc | wc -l)" -gt "0" ]; then
		PATCHED=1
		ui_print "Removing rctd service from init.lge.rc..."
		sed -ie '/\/sbin\/rctd/,+4d' "$ramdisk/init.lge.rc"
	fi
fi

if [ -f "$ramdisk/fstab.lucye" ]; then
	if [ "$(grep forceencrypt $ramdisk/fstab.lucye | wc -l)" -gt "0" ]; then
		PATCHED=1
		ui_print "Disabling forceencrypt..."
		sed -ie 's/forceencrypt/encryptable/' "$ramdisk/fstab.lucye"
	fi
fi

if [ -f "$ramdisk/init.rc" ]; then
	if [ "$(grep /system/bin/install-recovery $ramdisk/init.rc | wc -l)" -gt "0" ]; then
		PATCHED=1
		ui_print "Removing custom recovery killer..."
		sed -ie '/\/system\/bin\/install-recovery/,+2d' "$ramdisk/init.rc"
	fi
fi

# write new boot
write_boot;

# Manual module install
ui_print "Installing modules...";
if `mount -o rw,remount -t auto /vendor`; then
	ui_print "Detected Android Vendor Partition"
	rm -rf /vendor/lib/modules
	mkdir -p /vendor/lib/modules
elif `mount -o rw,remount -t auto /system`; then
	if [ -d "/system/vendor/lib/modules" ]; then
		ui_print "Detected Android System-Root with Vendor-on-System"
		rm -rf /system/vendor/lib/modules
		mkdir -p /system/vendor/lib/modules
	elif [ -d "/system/system/lib/modules" ]; then
		ui_print "Detected Android System-Root without Vendor"
		rm -rf /system/system/lib/modules
		mkdir -p /system/system/lib/modules
	elif [ -d "/system/lib/modules" ]; then
		ui_print "Detected Classic Android System"
		rm -rf /system/lib/modules
		mkdir -p /system/lib/modules
	fi
else
	ui_print "Could not install modules."
	exit 1
fi
cp -rf /tmp/anykernel/modules/* /system/lib/modules/;
set_perm_recursive 0 0 0755 0644 /system/lib/modules;

mount -o ro,remount -t auto /system;
## end install
