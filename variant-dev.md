## **Compiling for other G6 variants**

### ***Before we begin...***
Please keep in mind this is completely untested and may not work at all.

This serves as a guide to help, but not provide an end all solution.

### ***Setting up***

To get started you are going to need some information from your phone's STOCK ROM.
1. Build Fingerprint
   * Find your model's stock ROM "build fingerprint". This can be done in many ways, one such is using [Kernel Adiutor](https://play.google.com/store/apps/details?id=com.grarak.kerneladiutor), selecting *Device* from the side menu. For example, for the US997, it says **lge/lucye_nao_us/lucye**.
   * The 2nd part (in this example, "**lucye_nao_us"**) is the important part, now look at [this repo's available defconfigs](arch/arm64/configs) and see if your can find the codename you just found.
2. Kernel Version
   * I'm pretty sure all of LG's releases use *-perf* but just in case, check your kernel version string to see if it ends with "-perf". If so, use the defconfig with *-perf*.

If a file with your codename plus "_defconfig" exists, you can ***try*** to compile this kernel for yourself.

Check out the **ORIG_DEFCONFIG** variable while you are updating your **TOOLCHAIN** variable in [.zefie/scripts/buildenv.sh](.zefie/scripts/buildenv.sh).

In my example, we end up with *ORIG_DEFCONFIG=lucye_nao_us-perf_defconfig*

From there, follow the [existing instructions](README.md#how-to-build).

### ***Compile errors?***

If you are trying to compile for a variant and run into compile errors, I cannot help you. That is the nature of trying to cross-compile similar variant kernels, and you will have to fork the git and try to fix the errors yourself.
