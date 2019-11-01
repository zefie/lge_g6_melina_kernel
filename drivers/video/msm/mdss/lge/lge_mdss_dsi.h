/*
 * Copyright(c) 2016, LG Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef LGE_MDSS_DSI_H
#define LGE_MDSS_DSI_H
#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON)
#include "lge_mplus.h"
#else
#include "lge_lcd.h"
#endif
#include "lge_color_manager.h"

#define DEFAULT_LGE_FSC_U3 20000
#define DEFAULT_LGE_FSC_U2 2500

struct lge_gpio_entry {
	char name[32];
	int gpio;
};

struct lge_dsi_cmds_entry {
	char name[128];
	struct dsi_panel_cmds lge_dsi_cmds;
};

struct lge_ddic_ops {
	/* blmap */
	int (*op_get_blmap_type)(struct mdss_dsi_ctrl_pdata *ctrl);
	void (*op_mplus_change_blmap)(struct mdss_dsi_ctrl_pdata *ctrl);

	/* image enhance mode */
	int (*op_image_enhance_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_image_enhance_get)(struct mdss_dsi_ctrl_pdata *ctrl);

	/* MPLUS */
	int (*op_mplus_mode_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_mplus_mode_get)(struct mdss_dsi_ctrl_pdata *ctrl);
	int (*op_mplus_hd_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_mplus_hd_get)(struct mdss_dsi_ctrl_pdata *ctrl);
	int (*op_mplus_max_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_mplus_max_get)(struct mdss_dsi_ctrl_pdata *ctrl);
	/* advanced IE */
	int (*op_screen_mode_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_rgb_tune_set)(struct mdss_dsi_ctrl_pdata *ctrl);
	int (*op_screen_tune_set)(struct mdss_dsi_ctrl_pdata *ctrl);
	int (*op_sharpness_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_sre_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_dolby_mode_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_hdr_mode_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);
	int (*op_hl_mode_set)(struct mdss_dsi_ctrl_pdata *ctrl, int mode);

	/* aod */
	int (*op_send_u2_cmds)(struct mdss_dsi_ctrl_pdata *ctrl);
};

struct lge_rect {
	int x;
	int y;
	int w;
	int h;
};

struct lge_mdss_dsi_ctrl_pdata {
	/* use full scale current support */
	bool use_u2_fsc;
	int fsc_old;
	int fsc_req;
	int fsc_u3;
	int fsc_u2;

	/* blank mode support */
	int blank_mode;

	/* gpio */
	int num_gpios;
	struct lge_gpio_entry *gpio_array;

	/* blmap */
	char **blmap_list;
	int blmap_list_size;
	int **blmap;
	int *blmap_size;

	/* cmds */
	int num_extra_cmds;
	struct lge_dsi_cmds_entry *extra_cmds_array;

	/* mplus */
	bool use_mplus;
	int mplus_dim_cnt;
	int mplus_dim_delay;
	enum lge_mplus_mode mplus_hd;
	enum lge_mplus_mode mp_max;
	enum lge_mplus_mode mp_mode;
	enum lge_mplus_mode adv_mp_mode;
	enum lge_mplus_mode cur_mp_mode;
	int *mp_to_blmap_tbl;
	int mp_to_blmap_tbl_size;

	/* advanced IE */
	int sharpness;
	bool sharpness_control;
	bool dg_control;
	bool ie_control;
	int sre_mode;
	int dolby_mode;
	int hdr_mode;
	int hl_mode;
#if IS_ENABLED(CONFIG_LGE_DISPLAY_HT_LCD_TUNE_MODE)
	int ht_mode;
#endif
	int screen_mode;
	int screen_tune_status;
	int sc_sat_step;
	int sc_hue_step;
	int sc_sha_step;
	int color_filter;

	/* For DISPLAY_COLOR_MANAGER */
	int cm_preset_step;
	int cm_red_step;
	int cm_green_step;
	int cm_blue_step;

	/* aod */
	struct lge_rect aod_area;
	u32 aod_interface;

	/* ddic ops */
	struct lge_ddic_ops *ddic_ops;
};

#define LGE_DDIC_OP_CHECK(c, op) (c && c->lge_extra.ddic_ops && c->lge_extra.ddic_ops->op_##op)
#define LGE_DDIC_OP(c, op, ...) (LGE_DDIC_OP_CHECK(c,op)?c->lge_extra.ddic_ops->op_##op(c, ##__VA_ARGS__):-ENODEV)
#define LGE_DDIC_OP_LOCKED(c, op, lock, ...) do { \
	mutex_lock(lock); \
	if (LGE_DDIC_OP_CHECK(c,op)) c->lge_extra.ddic_ops->op_##op(c, ##__VA_ARGS__); \
	mutex_unlock(lock); } while(0)

#define LGE_MDELAY(m) do { if ( m > 0) usleep_range((m)*1000,(m)*1000); } while(0)
#define LGE_OVERRIDE_VALUE(x, v) do { if ((v)) (x) = (v); } while(0)

#include "lge_mdss_dsi_panel.h"

int lge_mdss_dsi_parse_extra_params(
	struct platform_device *ctrl_pdev,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata);
void lge_extra_gpio_set_value(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		const char *name, int value);

int lge_panel_power_on(struct mdss_panel_data *pdata);
int lge_panel_power_off(struct mdss_panel_data *pdata);

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_CTRL_SHUTDOWN)
void mdss_dsi_ctrl_shutdown(struct platform_device *pdev);
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_ON)
int mdss_dsi_panel_on(struct mdss_panel_data *pdata);
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_OFF)
int mdss_dsi_panel_off(struct mdss_panel_data *pdata);
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_POWER_ON)
int mdss_dsi_panel_power_on(struct mdss_panel_data *pdata);
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_POWER_OFF)
int mdss_dsi_panel_power_off(struct mdss_panel_data *pdata);
#endif

#endif /* LGE_MDSS_DSI_H */
