# Melina Kernel for LG G6 Stock ROMs

Based on LGE OpenSource US997U v15a kernel ([commit 3189c52](https://github.com/zefie/lge_g6_melina_kernel/tree/3189c52e67deebe6c466ab09e11c5a9d64781c20))

Previously based on LGE Opensource v10b kernel ([first commit (70c31dd)](https://github.com/zefie/lge_g6_melina_kernel/tree/70c31dde4f4575255d7aadf2f626d50e2c36d25a))

**Note**: I own the US997, therefore I cannot test any other releases made available,
only compile them and let you try them. Keep that in mind.

**Always retain a backup of your current kernel and system (we update kernel modules in /system)**.

## Information

 * For use on LG Nougat Stock ROMs
 * Developer tested on LGE Stock US997v16a
 * Uses [osm0sis's AnyKernel2](https://forum.xda-developers.com/showthread.php?t=2670512) system

## Features:

 * Meticulously updated from v3.18.31 to **v3.18.53** (Linux security, performance, bugfix, etc updates) ([rel4](https://github.com/zefie/lge_g6_melina_kernel/releases/tag/rel4) or newer)
 * Binary releases built with ubertc aarch64-linux-android-6.x
 * DriveDroid CD-ROM Emulation Support
 * Qualcomm MDSS KCAL Support
 * Flash Friendly FS (F2FS) Support ([rel4](https://github.com/zefie/lge_g6_melina_kernel/releases/tag/rel4) or newer)
 * Open source compatible replacement for Tuxera exFAT driver
 * zzmove and elementalx governors
 * Higher performance with lower battery usage ([due to dtb and power updates from US997Uv15a](https://github.com/zefie/lge_g6_melina_kernel/tree/3189c52e67deebe6c466ab09e11c5a9d64781c20)) ([rel2](https://github.com/zefie/lge_g6_melina_kernel/releases/tag/rel2) or newer)
 * Various other improvements

## Current Status:

 * Compiles: Yes ~ *Confirmed*: US997 H870
 * Boots: Yes ~ *Confirmed*: US997 H870
 * Bugs: **US997**: None ~ **H870**: no wifi

## How to build:

 * [Prepare your toolchain](ubertc-guide.md)
 * Clone repo
 * Run ```git submodule init && git submodule update```
 * Edit .zefie/scripts/buildenv.sh, and set TOOLCHAIN, KERNEL_DEV and KERNEL_NAME path for your local setup
 * Run .zefie/scripts/fullbuild.sh

If all goes well, output will be in build/out/,
and you can also use .zefie/scripts/sideload.sh to automatically sideload the most recent zip to TWRP.

## Credits

 Commits have been cherry-picked from the following sources:

 * [CrazyAquaKernel for LineageOS by CrazyGamerGR](https://github.com/CrazyGamerGR/CrazyAquaKernel-g5-g6-los-nougat)
 * [DX-Mi5 by pappschlumpf](https://github.com/pappschlumpf/DX-Mi5)
 * [Noire Kernel by me](https://git.zefie.net/zefie/android_kernel_samsung_msm8916)

## XDA Credits

 * [osm0sis for AnyKernel2](https://forum.xda-developers.com/showthread.php?t=2670512)
 * [CrazyGamerGR for CrazyAquaKernel](https://forum.xda-developers.com/lg-g6/development/kernel-crazyaquakernel-t3661459)
 * [@nexusownerforlife](https://forum.xda-developers.com/member.php?u=6382322) for testing on Fulmics
 * [@Killua96](https://forum.xda-developers.com/member.php?u=4580019) for the data needed for H870 builds
 * Kernel updates cherry-picked (then rebased) from [leskal/f2fs-stable](https://github.com/leskal/f2fs-stable)

## Benchmarks

Stock Kernel (US997v16a) | Melina Kernel ([rel2](https://github.com/zefie/lge_g6_melina_kernel/tree/rel2))
--- | ---
![Image](https://github.com/zefie/lge_g6_kernel_scripts/raw/master/benchmarks/stock_lge_16a_kernel.jpg) | ![Image](https://github.com/zefie/lge_g6_kernel_scripts/raw/master/benchmarks/melina_kernel_rel2.jpg)
[Details](http://browser.geekbench.com/v4/cpu/5472375) | [Details](http://browser.geekbench.com/v4/cpu/5490319)

### [GeekBench Comparison](http://browser.geekbench.com/v4/cpu/compare/5490319?baseline=5472375) (Left is Melina, Right is Stock)
