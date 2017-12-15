## Melina Kernel (for LG G6 US997 Variant)

**Note**: This is for the US997, **NOT** the US997U (aka LG G6+)

Based on LGE Opensource v10b kernel (first commit)

Information

 * For use on LG Nougat Stock ROMs
 * Tested on LGE Stock US997v16a
 * Tested with ubertc aarch64-linux-android-6.x
 
Features:

 * TODO

# Current Status:

 * Compiles: Yes
 * Boots: Untested


# How to build:

 * Edit .zefie/scripts/buildenv.sh, and set TOOLCHAIN for your local setup
 * Run .zefie/scripts/fullbuild.sh

If all goes well, output will be in build/out/,
and you can also use .zefie/scripts/sideload.sh to automatically sideload the most recent zip to TWRP.
