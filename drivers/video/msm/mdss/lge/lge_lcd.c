#include "../mdss_fb.h"
#include "lge_mdss_display.h"
#include "lge_mdss_sysfs.h"

/* Advanced IE set */
static ssize_t screen_mode_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->lge_extra.screen_mode);
}

static ssize_t screen_mode_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	sscanf(buf, "%d", &mode);
	LGE_DDIC_OP_LOCKED(ctrl, screen_mode_set, &mfd->mdss_sysfs_lock, mode);
	return ret;
}
static DEVICE_ATTR(screen_mode, S_IRUGO|S_IWUSR|S_IWGRP, screen_mode_get, screen_mode_set);

static ssize_t rgb_tune_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d %d %d %d \n", ctrl->lge_extra.cm_preset_step,
					ctrl->lge_extra.cm_red_step,
					ctrl->lge_extra.cm_green_step,
					ctrl->lge_extra.cm_blue_step);

}

static ssize_t rgb_tune_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{

	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input_param[4];

	GET_DATA
	sscanf(buf, "%d %d %d %d", &input_param[0], &input_param[1], &input_param[2], &input_param[3]);

	ctrl->lge_extra.cm_preset_step = input_param[0];
	ctrl->lge_extra.cm_red_step    = abs(input_param[1]);
	ctrl->lge_extra.cm_green_step  = abs(input_param[2]);
	ctrl->lge_extra.cm_blue_step   = abs(input_param[3]);

	LGE_DDIC_OP_LOCKED(ctrl, rgb_tune_set, &mfd->mdss_sysfs_lock);
	return ret;
}
static DEVICE_ATTR(rgb_tune, S_IRUGO|S_IWUSR|S_IWGRP, rgb_tune_get, rgb_tune_set);

static ssize_t screen_tune_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d %d %d \n", ctrl->lge_extra.sc_sat_step,
					ctrl->lge_extra.sc_hue_step,
					ctrl->lge_extra.sc_sha_step);
}

static ssize_t screen_tune_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input_param[4];
	GET_DATA

	sscanf(buf, "%d %d %d", &input_param[0], &input_param[1], &input_param[2]);
	ctrl->lge_extra.sc_sat_step 	= abs(input_param[0]);
	ctrl->lge_extra.sc_hue_step 	= abs(input_param[1]);
	ctrl->lge_extra.sc_sha_step 	= abs(input_param[2]);

	LGE_DDIC_OP_LOCKED(ctrl, screen_tune_set, &mfd->mdss_sysfs_lock);
	return ret;
}
static DEVICE_ATTR(screen_tune, S_IRUGO|S_IWUSR|S_IWGRP, screen_tune_get, screen_tune_set);


static struct attribute *lge_mdss_imgtune_advancedIE_attrs[] = {
	&dev_attr_screen_mode.attr,
	&dev_attr_rgb_tune.attr,
	&dev_attr_screen_tune.attr,
	NULL,
};

static struct attribute_group lge_mdss_imgtune_advancedIE_attr_group = {
	.attrs = lge_mdss_imgtune_advancedIE_attrs,
};

int lge_mdss_lcd_sysfs_init(struct device *panel_sysfs_dev, struct fb_info *fbi)
{
	int ret = 0;
	struct msm_fb_data_type *mfd = NULL;
	struct mdss_panel_data *pdata = NULL;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	mfd = (struct msm_fb_data_type *)(fbi)->par;
	if (mfd == NULL) {
		pr_err("uninitialzed mfd\n");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (pdata == NULL) {
		pr_err("no panel connected!\n");
		return -EINVAL;
	}
	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata, panel_data);

	ret += sysfs_create_group(&panel_sysfs_dev->kobj, &lge_mdss_imgtune_advancedIE_attr_group);

	return ret;
}

void lge_mdss_lcd_sysfs_deinit(struct fb_info *fbi)
{
	struct msm_fb_data_type *mfd = NULL;
	struct mdss_panel_data *pdata = NULL;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	mfd = (struct msm_fb_data_type *)(fbi)->par;
	if (mfd == NULL) {
		pr_err("uninitialzed mfd\n");
		return;
	}
	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (pdata == NULL) {
		pr_err("no panel connected!\n");
		return;
	}
	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata, panel_data);
	sysfs_remove_group(&fbi->dev->kobj, &lge_mdss_imgtune_advancedIE_attr_group);
}
