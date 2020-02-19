# Melina Reborn Kernel for LG G6 modern AOSP ROMs

Based off of [LG-G6-DEV/android_kernel_lge_msm8996 commit 757e457](https://github.com/LG-G6-DEV/android_kernel_lge_msm8996/commit/757e45767a0725a93ae01f2cf93cdda9f598aea7))

**Note**: I own the US997, therefore I cannot test any other releases made available,
only compile them and let you try them. Keep that in mind.

**Always retain a backup of your current kernel and system image**.

## Information

 * For use on modern AOSP ROMs (10, Pie may be unstable) 
 * Developer tested on HavocOS-v3.2 (Android 10)
 * Uses [osm0sis's AnyKernel3](https://forum.xda-developers.com/showthread.php?t=2670512) system

## Features:
 * Release builds have significantly reduced debugging messages/routines for slight performance & battery enhancements
 * No random reboots compared to V30A kernels (including Melina Reborn rel_r4 and older)
 * Optimize compiler flags with better CPU optimizations
 * Updated AnyKernel to v3, now retains Magisk [zefie/recovery_zip_template](zefie/recovery_zip_template)
 * Previous security updates from past Melina Kernels and new security updates
 * Optimize for performance rather than size
 * Binary releases built with ubertc aarch64-linux-android-6.x
 * DriveDroid CD-ROM Emulation Support
 * zzmove and elementalx governors
 * Higher performance with lower battery usage
 * Various other improvements

## Features for Developers:
 * [zefie/scripts](zefie/scripts) build helper scripts
 * BE DAI Name Table (sound soc HAL) backported from msm 4.x
 * Moduleless Kernel, don't need to work about modules and paths
 * Most Melina Reborn tweaks are enabled via a custom Kconfig menu, so you can choose which features are enabled
 * Re-enable debug via 2 kernel config options (`CONFIG_MELINA_DEBUG_DISABLE=n` `CONFIG_MELINA_DEBUG_ENABLE=y`)
 * Kernel is ready to compile in-tree or out of tree with clang or gcc

## How to build:

 * [Prepare your toolchain](ubertc-guide.md)
 * Clone this git repository and enter the directory
 * Edit `.zefie/scripts/buildenv.sh`, and set `TOOLCHAIN`, `KERNEL_DEV` and `KERNEL_NAME` path for your local setup
 * To build, run (for example `h870`) `.zefie/scripts/build.sh setdevice h870 clean build`
 * To build all 6 kernels (3 devices, debug and non-debug), run `./zefie/scripts/build.sh release-build`
 * To build all 6 kernels with clang, run `./zefie/scripts/build.sh clang release-build`
 * To build all **12** kernels with (clang/nonclang debug/nondebug), run `./zefie/scripts/build.sh release-build clang release-build`

If all goes well, output will be in `build/out/`,
and you can also use `.zefie/scripts/sideload.sh` to automatically sideload the most recent zip to TWRP.

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

