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
#include <soc/qcom/lge/board_lge.h>
#include <linux/input/lge_touch_notify.h>
#include <linux/lge_panel_notify.h>

extern int mdss_dsi_pinctrl_set_state(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		bool active);
extern void identify_revision_sw49410(struct mdss_dsi_ctrl_pdata *ctrl);

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_CTRL_SHUTDOWN)
void mdss_dsi_ctrl_shutdown(struct platform_device *pdev)
{
	int ret = 0;

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = platform_get_drvdata(pdev);

	if (!ctrl_pdata) {
		pr_err("no driver data\n");
		return;
	}

	usleep_range(5000, 5000);
	pr_info("t_rst L\n");
	lge_extra_gpio_set_value(ctrl_pdata, "t_rst", 0);
	pr_info("reset L\n");
	gpio_set_value((ctrl_pdata->rst_gpio), 0);

	usleep_range(5000, 5000);

	/* Exit from TTW mode before shutting down LABIBB */
	pr_info("Exit TTW mode\n");
	ret = msm_dss_set_vreg(ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg,
		REGULATOR_MODE_TTW_OFF);
	if (ret)
		pr_err("fail to disable TTW mode : %d\n", ret);

	usleep_range(1000, 1000);

	/* shut down LABIBB */
	pr_info("Shut down LABIBB\n");
	ret = msm_dss_set_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg,
		REGULATOR_MODE_SHUTDOWN);
	if (ret)
		pr_err("failed : %d\n", ret);

	usleep_range(5000, 5000);
	pr_info("vddio off\n");
	lge_extra_gpio_set_value(ctrl_pdata, "vddio", 0);
	usleep_range(1000, 1000);
}
#endif



#ifdef CONFIG_LGE_LCD_POWER_CTRL
extern int panel_not_connected;
extern int detect_factory_cable(void);

int lge_panel_power_off(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int ret = 0;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_info("+\n");

	if (!(panel_not_connected && detect_factory_cable() && !lge_get_mfts_mode())) {
		if (mdss_dsi_is_right_ctrl(ctrl_pdata)) {
			pr_err("%d, right ctrl configuration not needed\n", __LINE__);
			return ret;
		}
	}

	usleep_range(5000, 5000);

	pr_info("reset L\n");
	lge_panel_notifier_call_chain(LGE_PANEL_EVENT_RESET, 0, LGE_PANEL_RESET_LOW);
	gpio_set_value((ctrl_pdata->rst_gpio), 0);
	lge_extra_gpio_set_value(ctrl_pdata, "t_rst", 0);

	usleep_range(5000, 5000);

	/* Exit from TTW mode before shutting down LABIBB */
	pr_info("Exit TTW mode\n");
	ret = msm_dss_set_vreg(ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg,
		REGULATOR_MODE_TTW_OFF);
	if (ret)
		pr_err("fail to disable ttw mode : %d\n", ret);

	usleep_range(1000, 1000);

	pr_info("Enable Pull down\n");
	ret = msm_dss_set_vreg(
			ctrl_pdata->panel_power_data.vreg_config,
			ctrl_pdata->panel_power_data.num_vreg,
			REGULATOR_MODE_ENABLE_PULLDOWN);
	if (ret)
		pr_err("fail to enable pull down : %d\n", ret);

	usleep_range(1000, 1000);

	pr_info("Turn off LABIBB\n");
	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 0);
	if (ret)
		pr_err("failed to disable vregs for %d\n", ret);

	pr_info("SPARE ON\n");
	ret = msm_dss_set_vreg(ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg,
		REGULATOR_MODE_SPARE_ON);
	if (ret)
		pr_err("failed to spare on %d\n", ret);

	usleep_range(5000, 5000);

	pr_info("vddio off\n");
	lge_extra_gpio_set_value(ctrl_pdata, "vddio", 0);
	usleep_range(1000, 1000);

	pr_info("TTW ON\n");
	ret = msm_dss_set_vreg(ctrl_pdata->panel_power_data.vreg_config,
			ctrl_pdata->panel_power_data.num_vreg,
			REGULATOR_MODE_TTW_ON);
	if (ret)
		pr_err("failed to TTW on %d\n", ret);

	return ret;
}

int lge_panel_power_on(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	pr_info("+\n");

	if (!(panel_not_connected && detect_factory_cable() &&
						!lge_get_mfts_mode())) {
		if (mdss_dsi_is_right_ctrl(ctrl_pdata)) {
			pr_err("%d, right ctrl configuration not needed\n", __LINE__);
			return ret;
		}
	}

	pr_info("vddio on\n");
	lge_extra_gpio_set_value(ctrl_pdata, "vddio", 1);

	usleep_range(5000, 5000);

	pr_info("Enable LABIBB\n");
	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 1);

	pr_info("Disable Pulldown\n");
	ret = msm_dss_set_vreg(ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg,
		REGULATOR_MODE_DISABLE_PULLDOWN);
	if (ret)
		pr_err("failed to disable pulldown %d\n", ret);

	usleep_range(5000, 5000);

	pr_info("reset H\n");
	gpio_set_value((ctrl_pdata->rst_gpio), 1);
	lge_extra_gpio_set_value(ctrl_pdata, "t_rst", 1);

	usleep_range(5000, 5000);

	pr_info("-\n");

	return ret;
}
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_POWER_ON)
int mdss_dsi_panel_power_on(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_info("+\n");

	if (mdss_dsi_is_right_ctrl(ctrl_pdata) &&
		mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)) {
		pr_err("right ctrl gpio configuration not needed\n");
		return ret;
	}

#if 0 // need to check U0 near
	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 1);
	if (ret) {
		pr_err("failed to enable vregs for %s\n",
			__mdss_dsi_pm_name(DSI_PANEL_PM));
		return ret;
	}
#endif
	/*
	 * If continuous splash screen feature is enabled, then we need to
	 * request all the GPIOs that have already been configured in the
	 * bootloader. This needs to be done irresepective of whether
	 * the lp11_init flag is set or not.
	 */
	if (pdata->panel_info.cont_splash_enabled ||
		!pdata->panel_info.mipi.lp11_init) {
		if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
			pr_debug("reset enable: pinctrl not enabled\n");

		ret = mdss_dsi_panel_reset(pdata, 1);
		if (ret)
			pr_err("Panel reset failed. rc=%d\n", ret);
	}
	pr_info("-\n");

	return ret;
}
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_OVERRIDE_MDSS_DSI_PANEL_POWER_OFF)
int mdss_dsi_panel_power_off(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_info("+\n");

	if (mdss_dsi_is_right_ctrl(ctrl_pdata) &&
		mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)) {
		pr_debug("%d, right ctrl gpio configuration not needed\n", __LINE__);
		return ret;
	}

#if 0 // need to check U0 near
	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 0);
	if (ret)
		pr_err("failed to disable vregs for %s\n",
			__mdss_dsi_pm_name(DSI_PANEL_PM));
#endif
	pr_info("-\n");

	return ret;
}
#endif

void lge_mdss_post_dsi_event_handler(struct mdss_dsi_ctrl_pdata *ctrl, int event, void *arg)
{
	if (event == MDSS_EVENT_UNBLANK)
		identify_revision_sw49410(ctrl);

	return;
}
