
#include "../mdss_fb.h"
#include "lge_mdss_display.h"
#include "lge_mdss_sysfs.h"

/* Advanced Mplus Set */
static ssize_t mplus_mode_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	GET_DATA

	ret = LGE_DDIC_OP(ctrl, mplus_mode_get);
	return sprintf(buf, "%d\n", ret);
}

static ssize_t mplus_mode_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	sscanf(buf, "%d", &mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_mode_set, &mfd->mdss_sysfs_lock, mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
	return ret;
}
static DEVICE_ATTR(mplus_mode, S_IRUGO|S_IWUSR|S_IWGRP, mplus_mode_get, mplus_mode_set);

/* Hidden Menu Mplus Set */
static ssize_t mplus_hd_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	GET_DATA

	ret = LGE_DDIC_OP(ctrl, mplus_hd_get);
	return sprintf(buf, "%d\n", ret);
}

static ssize_t mplus_hd_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	sscanf(buf, "%d", &mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_hd_set, &mfd->mdss_sysfs_lock, mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
	return ret;
}
static DEVICE_ATTR(mplus_hd, S_IRUGO|S_IWUSR|S_IWGRP, mplus_hd_get, mplus_hd_set);

/* Mplus Max Set */
static ssize_t mplus_max_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	GET_DATA

	ret = LGE_DDIC_OP(ctrl, mplus_max_get);
	return sprintf(buf, "%d\n", ret);
}

static ssize_t mplus_max_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	sscanf(buf, "%d", &mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_max_set, &mfd->mdss_sysfs_lock, mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
	return ret;
}
static DEVICE_ATTR(mplus_max, S_IRUGO|S_IWUSR|S_IWGRP, mplus_max_get, mplus_max_set);

static ssize_t mplus_dim_cnt_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->lge_extra.mplus_dim_cnt);
}

static ssize_t mplus_dim_cnt_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input;
	GET_DATA

	sscanf(buf, "%d", &input);
	ctrl->lge_extra.mplus_dim_cnt = input;
	pr_info("mplus dim cnt : %d\n", ctrl->lge_extra.mplus_dim_cnt);

	return ret;
}
static DEVICE_ATTR(mplus_dim_cnt, S_IRUGO|S_IWUSR|S_IWGRP, mplus_dim_cnt_get, mplus_dim_cnt_set);

static ssize_t mplus_dim_delay_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->lge_extra.mplus_dim_delay);
}

static ssize_t mplus_dim_delay_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input;
	GET_DATA

	sscanf(buf, "%d", &input);
	ctrl->lge_extra.mplus_dim_delay = input;
	pr_info("mplus dim delay : %d\n", ctrl->lge_extra.mplus_dim_delay);

	return ret;
}
static DEVICE_ATTR(mplus_dim_delay, S_IRUGO|S_IWUSR|S_IWGRP, mplus_dim_delay_get, mplus_dim_delay_set);

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
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
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
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
	return ret;
}
static DEVICE_ATTR(rgb_tune, S_IRUGO|S_IWUSR|S_IWGRP, rgb_tune_get, rgb_tune_set);

static ssize_t screen_tune_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d %d %d %d \n", ctrl->lge_extra.sc_sat_step,
					ctrl->lge_extra.sc_hue_step,
					ctrl->lge_extra.sc_sha_step,
					ctrl->lge_extra.color_filter);

}

static ssize_t screen_tune_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input_param[4];
	GET_DATA

	sscanf(buf, "%d %d %d %d", &input_param[0], &input_param[1], &input_param[2], &input_param[3]);
	ctrl->lge_extra.sc_sat_step 	= abs(input_param[0]);
	ctrl->lge_extra.sc_hue_step 	= abs(input_param[1]);
	ctrl->lge_extra.sc_sha_step 	= abs(input_param[2]);
	ctrl->lge_extra.color_filter	= abs(input_param[3]);

	LGE_DDIC_OP_LOCKED(ctrl, screen_tune_set, &mfd->mdss_sysfs_lock);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
	return ret;
}
static DEVICE_ATTR(screen_tune, S_IRUGO|S_IWUSR|S_IWGRP, screen_tune_get, screen_tune_set);


/* "/sys/class/panel/img_tune/mplus_*" */
static struct attribute *lge_mdss_imgtune_mplus_attrs[] = {
	&dev_attr_mplus_mode.attr,
	&dev_attr_mplus_hd.attr,
	&dev_attr_mplus_max.attr,
	&dev_attr_mplus_dim_cnt.attr,
	&dev_attr_mplus_dim_delay.attr,
	NULL,
};

static struct attribute *lge_mdss_imgtune_advancedIE_attrs[] = {
	&dev_attr_screen_mode.attr,
	&dev_attr_rgb_tune.attr,
	&dev_attr_screen_tune.attr,
	NULL,
};

static struct attribute_group lge_mdss_imgtune_mplus_attr_group = {
	.attrs = lge_mdss_imgtune_mplus_attrs,
};

static struct attribute_group lge_mdss_imgtune_advancedIE_attr_group = {
	.attrs = lge_mdss_imgtune_advancedIE_attrs,
};

int lge_mdss_mplus_sysfs_init(struct device *panel_sysfs_dev, struct fb_info *fbi)
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
	if (ctrl->lge_extra.use_mplus) {
		ret = sysfs_create_group(&panel_sysfs_dev->kobj, &lge_mdss_imgtune_mplus_attr_group);
		ret += sysfs_create_group(&panel_sysfs_dev->kobj, &lge_mdss_imgtune_advancedIE_attr_group);
	}
	return ret;
}

void lge_mdss_mplus_sysfs_deinit(struct fb_info *fbi)
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
	if (ctrl->lge_extra.use_mplus) {
		sysfs_remove_group(&fbi->dev->kobj, &lge_mdss_imgtune_mplus_attr_group);
		sysfs_remove_group(&fbi->dev->kobj, &lge_mdss_imgtune_advancedIE_attr_group);
	}
}
