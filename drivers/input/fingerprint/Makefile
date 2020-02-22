#
# Makefile for the fingerprint drivers.
#

# Each configuration option enables a list of files.
ccflags-y += -Idrivers/misc/

obj-$(CONFIG_INPUT_FINGERPRINT_FPC_BTP)		+= fpc_btp.o
obj-$(CONFIG_INPUT_FINGERPRINT_FPC1020)		+= fpc1020_tee.o

ifeq ($(CONFIG_FINGERPRINT_GOODIX_GF3208),y)
obj-$(CONFIG_FINGERPRINT_GOODIX_GF3208) += gf_spi.o platform.o netlink.o gf_common.o
endif
