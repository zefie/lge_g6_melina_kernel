obj-$(CONFIG_LGE_DISPLAY_HPLUS_COMMON) += hplus/
obj-$(CONFIG_LGE_DISPLAY_LUCYE_COMMON) += lucye/
obj-$(CONFIG_LGE_DISPLAY_FALCON_COMMON) += falcon/

obj-$(CONFIG_LGE_DISPLAY_AOD_SUPPORTED) += lge_mdss_aod.o
obj-$(CONFIG_LGE_DISPLAY_AOD_WITH_MIPI) += lge_mdss_watch.o
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_mdss_dsi.o
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_mdss_dsi_panel.o
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_mdss_fb.o
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_mdss_sysfs.o
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_mdss_sysfs_imgtune.o
ifeq ($(CONFIG_LGE_DISPLAY_FALCON_COMMON),y)
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_mplus.o
else
obj-$(CONFIG_LGE_DISPLAY_COMMON) += lge_lcd.o
endif
obj-$(CONFIG_LGE_DISPLAY_COMMON) += notifier/lge_panel_notifier.o
obj-$(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED) += ambient/lge_ambient.o
obj-$(CONFIG_LGE_DP_UNSUPPORT_NOTIFY) += lge_dp_notify.o
