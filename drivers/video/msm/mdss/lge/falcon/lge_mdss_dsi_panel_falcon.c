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

#define pr_fmt(fmt)	"[Display] %s: " fmt, __func__

#include <linux/delay.h>
#include "../../mdss_dsi.h"
#include "../lge_mdss_display.h"
#include "../../mdss_dba_utils.h"
#include <linux/input/lge_touch_notify.h>
#include <linux/lge_panel_notify.h>
#include <soc/qcom/lge/board_lge.h>

extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_panel_cmds *pcmds, u32 flags);

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_RESET)
static int mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	/* need nothing */
	return rc;
}

int mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i, rc = 0;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	if (mdss_dsi_is_right_ctrl(ctrl_pdata) &&
			mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)) {
		pr_debug("%d, right ctrl gpio configuration not needed\n", __LINE__);
		return rc;
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%d, reset line not configured\n", __LINE__);
		return rc;
	}

	pr_debug("enable = %d\n", enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		rc = mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}
		if (!pinfo->cont_splash_enabled) {
			lge_panel_notifier_call_chain(LGE_PANEL_EVENT_RESET, 0, LGE_PANEL_RESET_LOW); // PANEL RESET LOW
			for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
				gpio_set_value((ctrl_pdata->rst_gpio),
						pdata->panel_info.rst_seq[i]);
				if (pdata->panel_info.rst_seq[++i])
					usleep_range(pinfo->rst_seq[i] * 1000,
						pinfo->rst_seq[i] * 1000);
			}
			lge_panel_notifier_call_chain(LGE_PANEL_EVENT_RESET, 0, LGE_PANEL_RESET_HIGH); // PANEL RESET HIGH
		}
		if (pinfo->cont_splash_enabled) {
			usleep_range(5000,5000);
			rc = msm_dss_enable_vreg(
					ctrl_pdata->panel_power_data.vreg_config,
					ctrl_pdata->panel_power_data.num_vreg, 1);
			if (rc) {
				pr_err("failed to enable vregs for %s\n",
						__mdss_dsi_pm_name(DSI_PANEL_PM));
			} else {
				pr_info("enable vregs for %s\n",
						__mdss_dsi_pm_name(DSI_PANEL_PM));
			}
		}
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("Panel Not properly turned OFF\n");
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("Reset panel done\n");
		}
	} else {
		lge_panel_notifier_call_chain(LGE_PANEL_EVENT_RESET, 0, LGE_PANEL_RESET_LOW); // PANEL RESET LOW
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		gpio_free(ctrl_pdata->rst_gpio);
	}

	return rc;
}
#endif


#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_ON)
int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_info *pinfo;
	struct dsi_panel_cmds *on_cmds;
	int apo_idx = 0;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	pinfo = &pdata->panel_info;
	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	pr_info("+\n");

	if (pinfo->dcs_cmd_by_left) {
		if (ctrl->ndx != DSI_CTRL_LEFT)
			goto end;
	}

	if (pinfo->panel_dead) {
		for (apo_idx = 0; apo_idx < ctrl->lge_extra.num_extra_cmds; apo_idx++) {
			if (!strcmp(ctrl->lge_extra.extra_cmds_array[apo_idx].name, "on-with-apo"))
				break;
		}

		if (apo_idx >= ctrl->lge_extra.num_extra_cmds) {
			pr_info("no apo cmd, use default on cmd\n");
			on_cmds = &ctrl->on_cmds;
		} else {
			on_cmds = &ctrl->lge_extra.extra_cmds_array[apo_idx].lge_dsi_cmds;
			pr_info("set apo cmd\n");
		}
	} else {
		on_cmds = &ctrl->on_cmds;
	}

	if ((pinfo->mipi.dms_mode == DYNAMIC_MODE_SWITCH_IMMEDIATE) &&
			(pinfo->mipi.boot_mode != pinfo->mipi.mode))
		on_cmds = &ctrl->post_dms_on_cmds;

	pr_debug("ndx=%d cmd_cnt=%d\n",	ctrl->ndx, on_cmds->cmd_cnt);
	if (ctrl->on_cmds.cmd_cnt)
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds, CMD_REQ_COMMIT);

	LGE_DDIC_OP(ctrl, mplus_mode_set, ctrl->lge_extra.adv_mp_mode);

	if (pinfo->compression_mode == COMPRESSION_DSC)
		mdss_dsi_panel_dsc_pps_send(ctrl, pinfo);
	if (ctrl->ds_registered && pinfo->is_pluggable)
		mdss_dba_utils_video_on(pinfo->dba_data, pinfo);
	lge_panel_notifier_call_chain(LGE_PANEL_EVENT_BLANK, 0, LGE_PANEL_STATE_UNBLANK);

end:
	pr_info("-\n");
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_OFF)
int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_info *pinfo;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	pinfo = &pdata->panel_info;
	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	pr_info("+\n");

	if (pinfo->dcs_cmd_by_left) {
		if (ctrl->ndx != DSI_CTRL_LEFT)
			goto end;
	}

	if (ctrl->off_cmds.cmd_cnt)
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->off_cmds, CMD_REQ_COMMIT);

	if (ctrl->ds_registered && pinfo->is_pluggable) {
		mdss_dba_utils_video_off(pinfo->dba_data);
		mdss_dba_utils_hdcp_enable(pinfo->dba_data, false);
	}
	lge_panel_notifier_call_chain(LGE_PANEL_EVENT_BLANK, 0, LGE_PANEL_STATE_BLANK);

end:
	pr_info("-\n");
	return 0;
}
#endif

extern struct lge_ddic_ops *get_ddic_ops_sw49410(void);
int lge_ddic_ops_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;

	ctrl_pdata->lge_extra.ddic_ops = get_ddic_ops_sw49410();
	pr_info("set ddic_ops sw49410\n");

	return rc;
}
