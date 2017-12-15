#!/bin/bash
rm -rf ./build/
.zefie/scripts/mrproper.sh
.zefie/scripts/defconfig.sh
.zefie/scripts/build.sh
