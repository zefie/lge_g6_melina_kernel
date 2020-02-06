# Melina Reborn Kernel for LG G6 modern AOSP ROMs (Oreo Edition)

Based on LGE OpenSource H870 v30a Kernel ([commit ed4ba0c](https://github.com/zefie/lge_g6_melina_kernel/tree/ed4ba0cace3e073a491e9ebfd509f031031170a0))

**Note**: I own the US997, therefore I cannot test any other releases made available,
only compile them and let you try them. Keep that in mind.

**Always retain a backup of your current kernel and system (we update kernel modules in /system/lib/modules)**.

## Information

 * For use on modern AOSP ROMs (Pie/Q) 
 * Developer tested on HavocOS-v3.2 (Android Q)
 * Uses [osm0sis's AnyKernel2](https://forum.xda-developers.com/showthread.php?t=2670512) system

## Features (rel_r1):

 * Previous security updates from past Melina Kernels
 * Optimize for performance rather than size
 * Binary releases built with ubertc aarch64-linux-android-6.x
 * DriveDroid CD-ROM Emulation Support
 * Open source compatible replacement for Tuxera exFAT driver
 * zzmove and elementalx governors
 * Higher performance with lower battery usage
 * Various other improvements

## How to build:

 * [Prepare your toolchain](ubertc-guide.md)
 * Clone repo
 * Run ```git submodule init && git submodule update```
 * Edit .zefie/scripts/buildenv.sh, and set TOOLCHAIN, KERNEL_DEV and KERNEL_NAME path for your local setup
 * Run .zefie/scripts/cleanbuild.sh

If all goes well, output will be in build/out/,
and you can also use .zefie/scripts/sideload.sh to automatically sideload the most recent zip to TWRP.

## Credits

 Commits have been cherry-picked from the following sources:

 * [android-7.0 branch of Melina Kernel](https://github.com/zefie/lge_g6_melina_kernel/tree/android-7.0)
 * [CrazyAquaKernel for LineageOS by CrazyGamerGR](https://github.com/CrazyGamerGR/CrazyAquaKernel-g5-g6-los-nougat)
 * [DX-Mi5 by pappschlumpf](https://github.com/pappschlumpf/DX-Mi5)
 * [leskal/f2fs-stable](https://github.com/leskal/f2fs-stable)
 * My old Noire Kernel (Samsung SM-T560NU code)

## XDA Credits

 * [osm0sis for AnyKernel2](https://forum.xda-developers.com/showthread.php?t=2670512)
 * [CrazyGamerGR for CrazyAquaKernel](https://forum.xda-developers.com/lg-g6/development/kernel-crazyaquakernel-t3661459)
 * [@nexusownerforlife](https://forum.xda-developers.com/member.php?u=6382322) for testing on Fulmics
 * [@Killua96](https://forum.xda-developers.com/member.php?u=4580019) for the data needed for H870 builds

