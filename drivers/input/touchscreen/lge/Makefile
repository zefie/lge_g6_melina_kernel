#
# Makefile for lge touch core driver
#

# Each configuration option enables a list of files.

ifdef CONFIG_LGE_TOUCH_CORE_MTK
	include $(srctree)/drivers/misc/mediatek/Makefile.custom
# ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/synaptics_3320/-
# ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
endif

# Touch Core Driver Common Header Code..!!
ccflags-y += -I$(srctree)/drivers/input/touchscreen/lge/
ccflags-y += -Wno-missing-braces

obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_core.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_core_sysfs.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_common.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_i2c.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_spi.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_model.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= touch_notify.o

obj-$(CONFIG_LGE_TOUCH_CORE)		+= touch_hwif.o
obj-$(CONFIG_LGE_TOUCH_CORE_MTK)	+= touch_hwif_mtk.o

obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= lgsic/
obj-$(CONFIG_LGE_TOUCH_CORE_BASE)	+= synaptics/

obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_core_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_core_sysfs_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_common_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_i2c_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_spi_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_model_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += touch_notify_nos.o

obj-$(CONFIG_LGE_TOUCH_CORE_NOS)        += touch_hwif_nos.o
obj-$(CONFIG_LGE_TOUCH_CORE_MTK_NOS)    += touch_hwif_mtk_nos.o

obj-$(CONFIG_LGE_TOUCH_CORE_BASE_NOS)   += lgsic/
