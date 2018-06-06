# Compile Noise Source as Linux kernel module
# Remove the compiler flags if the make file is integrated with the kernel
# build system
#ccflags-y += -DCONFIG_CRYPTO_DRBG_HASH
KERNEL_DIR=$(OUT)/obj/KERNEL_OBJ/
CROSS_COMPILE=aarch64-linux-android-
# KERNEL_CFLAGS=-mno-android
KERNEL_CFLAGS := KCFLAGS=-mno-android
MAKE=make
ccflags-y += -DCONFIG_LGCRYPTO_FIPS_ENABLE 

ifeq ($(ARCH),arm64)
    ARMDIR := arm64
else
    ARMDIR := arm
endif

obj-m += lgecrypto.o
obj-m += ecb.o
obj-m += cbc.o
obj-m += ctr.o
obj-m += des_generic.o
obj-m += aes_generic.o
obj-m += sha1_generic.o
obj-m += sha256_generic.o
obj-m += sha512_generic.o
obj-m += drbg.o

obj-m += $(ARMDIR)/

all:
	$(MAKE) ARCH=arm64 CCFLAGS=$(ccflags-y) EXTRA_CFLAGS=-DCONFIG_COMPAT CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) $(KERNEL_CFLAGS) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean

