# AnyKernel2 Ramdisk Mod Script
# osm0sis @ xda-developers

## AnyKernel setup
# EDIFY properties
kernel.string="%NAME% kernel for %MANU% %MODEL% by zefie"
do.devicecheck=1
do.modules=0 # we will do it ourself.
do.cleanup=1
do.cleanuponabort=0
device.name1=us997
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
ui_print "Kernel Name: %NAME%"
ui_print "Kernel Device: %MANU% %MODEL% (%DEVMODEL%)"
ui_print "Kernel Version: %VERSION%"
ui_print "Toolchain: %TOOLCHAIN_VERSION%"
ui_print " "

## AnyKernel install
dump_boot;
write_boot;

# Manual module install
ui_print "Pushing modules...";
mount -o rw,remount -t auto /system;

rm -rf /system/lib/modules
mkdir -p /system/lib/modules

cp -rf /tmp/anykernel/modules/* /system/lib/modules/;
set_perm_recursive 0 0 0755 0644 /system/lib/modules;

mount -o ro,remount -t auto /system;
## end install
