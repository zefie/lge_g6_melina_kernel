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

If you find a config matching your fingerprint, report back to me with the fingerprint and kernel version, then wait patiently for me to attempt to build.
