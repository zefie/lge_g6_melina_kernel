# Melina Kernel for LG G6 Stock ROMs (Oreo/Pie Edition)

Based on LGE OpenSource H871 v30a Kernel ([commit ccdcf7a](https://github.com/zefie/lge_g6_melina_kernel/tree/ccdcf7aca73df1f66fc3140d062856544a42b14f))

**Note**: I own the US997, therefore I cannot test any other releases made available,
only compile them and let you try them. Keep that in mind.

**Always retain a backup of your current kernel and system**.

## Information

 * For use on LG Oreo/Pie **Stock ROMs**
 * Developer tested on LGE Stock US997v20c
 * Uses [osm0sis's AnyKernel3](https://forum.xda-developers.com/showthread.php?t=2670512) system

## Features (rel_p1):

 * Significantly reduced debugging messages/routines for slight performance & battery enhancements
 * Optimize compiler flags with better CPU optimizations
 * Updated AnyKernel to v3, now retains Magisk [zefie/recovery_zip_template](zefie/recovery_zip_template)
 * Previous security updates from past Melina Kernels and new security updates
 * Optimize for performance rather than size
 * Binary releases built with ubertc aarch64-linux-android-6.x
 * DriveDroid CD-ROM Emulation Support
 * Qualcomm MDSS KCAL Support
 * Open source compatible replacement for Tuxera exFAT driver
 * zzmove and elementalx governors
 * Higher performance with lower battery usage
 * Various other improvements

## How to build:

 * [Prepare your toolchain](ubertc-guide.md)
 * Clone repo
 * Run `git submodule init && git submodule update```
 * Edit `zefie/scripts/buildenv.sh`, and set TOOLCHAIN, KERNEL_DEV and KERNEL_NAME path for your local setup
 * Run `zefie/scripts/build.sh device <yourdevice> build`, eg `zefie/scripts/build.sh device h870 build`

If all goes well, output will be in `build/out/`.
and you can also use `zefie/scripts/build.sh sideload` to automatically sideload the most recent zip to TWRP.

## Credits

 This is an incomplete list of sources used for cherry-picking some commits:

 * [mk2000 Kernel](https://github.com/LG-G6-DEV/android_kernel_lge_msm8996)
 * [nathanchance/android-kernel-clang](https://github.com/nathanchance/android-kernel-clang.git)
 * [nathanchance/marlin](https://github.com/nathanchance/marlin.git)
 * [SaberMod/android-kernel-lge-hammerhead](https://gitlab.com/SaberMod/android-kernel-lge-hammerhead)
 * [N00bKernel/stanlee](https://github.com/N00bKernel/stanlee.git)
 * [LineageOS/android_kernel_lge_msm8996](https://github.com/LineageOS/android_kernel_lge_msm8996)
 * [sonyexperiadev/caf-kernel](https://git.choose-a.name/sonyxperiadev/caf-kernel.git)
 * [LG-G6-DEV/android_kernel_lge_msm8996](https://github.com/LG-G6-DEV/android_kernel_lge_msm8996)
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
