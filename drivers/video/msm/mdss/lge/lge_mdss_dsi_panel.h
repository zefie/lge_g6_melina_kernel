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

#ifndef _LGE_MDSS_DSI_PANEL_H
#define _LGE_MDSS_DSI_PANEL_H

int lge_mdss_dsi_panel_init(struct device_node *node,
					struct mdss_dsi_ctrl_pdata *ctrl_pdata);
struct dsi_panel_cmds *lge_get_extra_cmds_by_name(struct mdss_dsi_ctrl_pdata *ctrl_pdata, char *name);
void lge_send_extra_cmds_by_name(struct mdss_dsi_ctrl_pdata *ctrl_pdata, char *name);
int lge_panel_br_to_bl(struct mdss_dsi_ctrl_pdata *ctrl, int br_lvl);

char *lge_get_blmapname(enum lge_bl_map_type  blmaptype);

/* This function should be defined under each model directory */
extern int lge_ddic_ops_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata);

#ifdef CONFIG_LGE_DISPLAY_BL_EXTENDED
int mdss_panel_parse_blex_settings(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata);
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_HT_LCD_TUNE_MODE)
void ht_tune_mode_set(struct mdss_dsi_ctrl_pdata *ctrl);
#endif
#endif
