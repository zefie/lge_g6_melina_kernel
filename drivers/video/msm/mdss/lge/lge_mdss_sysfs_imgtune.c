#include "../mdss_fb.h"
#include "lge_mdss_display.h"

#include "lge_mdss_sysfs.h"

extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_panel_cmds *pcmds, u32 flags);
#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON)
extern int lge_mdss_mplus_sysfs_init(struct device *panel_sysfs_dev, struct fb_info *fbi);
extern void lge_mdss_mplus_sysfs_deinit(struct fb_info *fbi);
#else
extern int lge_mdss_lcd_sysfs_init(struct device *panel_sysfs_dev, struct fb_info *fbi);
extern void lge_mdss_lcd_sysfs_deinit(struct fb_info *fbi);
#endif

#if IS_ENABLED(CONFIG_LGE_ENHANCE_GALLERY_SHARPNESS)
static ssize_t sharpness_get(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	GET_DATA

#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON) ||  IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	ret = sprintf(buf, "%d\n", ctrl->lge_extra.sharpness);
#else
	ret = sprintf(buf, "%x\n", ctrl->sharpness_on_cmds.cmds[2].payload[3]);
#endif
	return ret;
}

static ssize_t sharpness_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore sharpness enhancement cmd\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &mode);
#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON) ||  IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	LGE_DDIC_OP_LOCKED(ctrl, sharpness_set, &mfd->mdss_sysfs_lock, mode);
#else
	ctrl->sharpness_on_cmds.cmds[2].payload[3] = mode;

	mdss_dsi_panel_cmds_send(ctrl, &ctrl->sharpness_on_cmds, CMD_REQ_COMMIT);

	pr_info("%s: sent sharpness enhancement cmd : 0x%02X\n",
			__func__, ctrl->sharpness_on_cmds.cmds[2].payload[3]);
#endif
	return ret;
}
static DEVICE_ATTR(sharpness, S_IWUSR|S_IRUGO, sharpness_get, sharpness_set);
#endif // CONFIG_LGE_ENHANCE_GALLERY_SHARPNESS

#if IS_ENABLED(CONFIG_LGE_LCD_DYNAMIC_CABC_MIE_CTRL)
static ssize_t image_enhance_get(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = 0;
	GET_DATA

#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON) || IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	ret = LGE_DDIC_OP(ctrl, image_enhance_get);
#else
	ret = ctrl->ie_on;
#endif
	return sprintf(buf, "%d\n", ret);
}

static ssize_t image_enhance_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore image enhancement cmd\n", __func__);
		return -EINVAL;
	}
	sscanf(buf, "%d", &mode);
	pr_info("%s: IE = %d \n", __func__, mode);

	ctrl->ie_on = mode;

#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON)
	LGE_DDIC_OP_LOCKED(ctrl, image_enhance_set, &mfd->mdss_sysfs_lock, mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
#elif IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	LGE_DDIC_OP_LOCKED(ctrl, image_enhance_set, &mfd->mdss_sysfs_lock, mode);
#else
	if(ctrl->ie_on == 1){
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->ie_on_cmds, CMD_REQ_COMMIT);
		pr_info("%s: set = %d, image enhance function on\n", __func__, ctrl->ie_on);
	}
	else if(ctrl->ie_on == 0){
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->ie_off_cmds, CMD_REQ_COMMIT);
		pr_info("%s: set = %d, image enhance funtion off\n", __func__, ctrl->ie_on);
	}
	else
		pr_info("%s: set = %d, wrong set value\n", __func__, ctrl->ie_on);
#endif

	return ret;
}
static DEVICE_ATTR(image_enhance_set, S_IWUSR|S_IRUGO, image_enhance_get, image_enhance_set);

static int cabc_on_off = 1;
static ssize_t cabc_get(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", cabc_on_off);
}

static ssize_t cabc_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
#if IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	struct dsi_panel_cmds *ie_pcmds;            //55h
	struct dsi_panel_cmds *cabc_pcmds;          //fbh
#endif
	GET_DATA

#if IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	ie_pcmds = lge_get_extra_cmds_by_name(ctrl, "ie-ctrl");
	if (!ie_pcmds)
	{
		pr_err("no cmds: ie-ctrl\n");
		return -EINVAL;
	}

	cabc_pcmds = lge_get_extra_cmds_by_name(ctrl, "cabc-on-off");
	if (!cabc_pcmds)
	{
		pr_err("no cmds: cabc-on-off\n");
		return -EINVAL;
	}
#endif

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore cabc set cmd\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &cabc_on_off);

#if IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	if (cabc_on_off == 0) {
		char mask = CABC_MASK;
		ie_pcmds->cmds[0].payload[1] &= (~mask);
		cabc_pcmds->cmds[0].payload[4] = CABC_OFF_VALUE;
	} else if (cabc_on_off == 1) {
		ie_pcmds->cmds[0].payload[1] |= CABC_MASK;
		cabc_pcmds->cmds[0].payload[4] |= CABC_ON_VALUE;
	} else {
		return -EINVAL;
	}

	pr_info("%s: CABC = %d, 55h = 0x%02x, fbh = 0x%02x\n",__func__,cabc_on_off,
		ie_pcmds->cmds[0].payload[1],cabc_pcmds->cmds[0].payload[1]);
	mdss_dsi_panel_cmds_send(ctrl, ie_pcmds, CMD_REQ_COMMIT);
	mdss_dsi_panel_cmds_send(ctrl, cabc_pcmds, CMD_REQ_COMMIT);
#endif
	return ret;
}
static DEVICE_ATTR(cabc, S_IWUSR|S_IRUGO, cabc_get, cabc_set);
#endif //CONFIG_LGE_LCD_DYNAMIC_CABC_MIE_CTRL

#if IS_ENABLED(CONFIG_LGE_DISPLAY_HT_LCD_TUNE_MODE)
static ssize_t get_ht_lcd_tune(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->lge_extra.ht_mode);
}
ssize_t set_ht_lcd_tune(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	int new_mode = 0;
	GET_DATA
	new_mode = simple_strtol(buf, NULL, 10);

	ctrl->lge_extra.ht_mode = new_mode;

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore ht_tune cmd\n", __func__);
		return -EINVAL;
	}

	ht_tune_mode_set(ctrl);
	return len;
}

static DEVICE_ATTR(ht_lcd_tune, S_IWUSR|S_IRUGO, get_ht_lcd_tune, set_ht_lcd_tune);
#endif

#if IS_ENABLED(CONFIG_LGE_DISPLAY_LINEAR_GAMMA)
static ssize_t linear_gamma_get(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%s\n", buf);
}

static ssize_t linear_gamma_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore color enhancement cmd\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &input);
	if (input == 0) {
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->linear_gamma_default_cmds, CMD_REQ_COMMIT);
	} else {
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->linear_gamma_tuning_cmds, CMD_REQ_COMMIT);
	}
	return ret;
}

static DEVICE_ATTR(linear_gamma, S_IWUSR|S_IRUGO, linear_gamma_get, linear_gamma_set);
#endif // CONFIG_LGE_DISPLAY_LINEAR_GAMMA

#if IS_ENABLED(CONFIG_LGE_DISPLAY_SRE_MODE)
static ssize_t sre_get(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->lge_extra.sre_mode);
}

static ssize_t sre_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore sre_set cmd\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &mode);

	LGE_DDIC_OP_LOCKED(ctrl, sre_set, &mfd->mdss_sysfs_lock, mode);
	return ret;
}

static DEVICE_ATTR(sre_mode, S_IWUSR|S_IRUGO, sre_get, sre_set);
static DEVICE_ATTR(daylight_mode, S_IWUSR|S_IRUGO, sre_get, sre_set);
#endif // CONFIG_LGE_DISPLAY_SRE_MODE

#if IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
static ssize_t SW49408_MUX_gate_voltage_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->mux_gate_voltage_status);
}

static ssize_t SW49408_MUX_gate_voltage_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int input;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore MUX gate voltage set\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &input);
	ctrl->mux_gate_voltage_status = input;

	if (input == 0) {
		pr_info("%s: Set MUX gate voltage to +8.7V/-8.8V\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->vgho_vglo_8p8v_cmd, CMD_REQ_COMMIT);
	} else if (input == 1) {
		pr_info("%s: Set MUX gate voltage to +11.6V/-11.6V\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->vgho_vglo_11p6v_cmd, CMD_REQ_COMMIT);
	} else {
		pr_info("%s: Invalid setting\n", __func__);
	}

	return ret;
}
static DEVICE_ATTR(mux_gate_voltage, S_IWUSR|S_IRUGO, SW49408_MUX_gate_voltage_get, SW49408_MUX_gate_voltage_set);
#endif //CONFIG_LGE_DISPLAY_LUCYE_COMMON

#if IS_ENABLED(CONFIG_LGE_DISPLAY_DOLBY_MODE)
static ssize_t dolby_mode_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->dolby_status);
}

static ssize_t dolby_mode_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore color enhancement cmd\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &mode);
	LGE_DDIC_OP_LOCKED(ctrl, dolby_mode_set, &mfd->mdss_sysfs_lock, mode);

	return ret;
}
static DEVICE_ATTR(dolby_mode, S_IWUSR|S_IRUGO, dolby_mode_get, dolby_mode_set);
#endif // CONFIG_LGE_DISPLAY_DOLBY_MODE

#if IS_ENABLED(CONFIG_LGE_DISPLAY_HDR_MODE)
static ssize_t hdr_mode_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	GET_DATA

	ret = sprintf(buf, "%d\n", ctrl->lge_extra.hdr_mode);
	return ret;
}

static ssize_t hdr_mode_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int mode;
	GET_DATA

	if (pdata->panel_info.panel_power_state == 0) {
		pr_err("%s: Panel off state. Ignore color enhancement cmd\n", __func__);
		return -EINVAL;
	}

	sscanf(buf, "%d", &mode);

#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON)
	LGE_DDIC_OP_LOCKED(ctrl, hdr_mode_set, &mfd->mdss_sysfs_lock, mode);
	LGE_DDIC_OP_LOCKED(ctrl, mplus_change_blmap, &mfd->bl_lock);
#else
	LGE_DDIC_OP_LOCKED(ctrl, hdr_mode_set, &mfd->mdss_sysfs_lock, mode);
#endif
	return ret;
}
static DEVICE_ATTR(hdr_mode, S_IWUSR|S_IRUGO, hdr_mode_get, hdr_mode_set);
#endif // CONFIG_LGE_DISPLAY_HDR_MODE

static struct attribute *lge_mdss_imgtune_attrs[] = {
#if IS_ENABLED(CONFIG_LGE_ENHANCE_GALLERY_SHARPNESS)
	&dev_attr_sharpness.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_LCD_DYNAMIC_CABC_MIE_CTRL)
	&dev_attr_image_enhance_set.attr,
	&dev_attr_cabc.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_HT_LCD_TUNE_MODE)
	&dev_attr_ht_lcd_tune.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_LINEAR_GAMMA)
	&dev_attr_linear_gamma.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_SRE_MODE)
	&dev_attr_sre_mode.attr,
	&dev_attr_daylight_mode.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	&dev_attr_mux_gate_voltage.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_DOLBY_MODE)
	&dev_attr_dolby_mode.attr,
#endif
#if IS_ENABLED(CONFIG_LGE_DISPLAY_HDR_MODE)
	&dev_attr_hdr_mode.attr,
#endif
	NULL,
};

static struct attribute_group lge_mdss_imgtune_attr_group = {
	.attrs = lge_mdss_imgtune_attrs,
};

static struct device *lge_panel_sysfs_imgtune = NULL;


int lge_mdss_sysfs_imgtune_init(struct class *panel, struct fb_info *fbi)
{
	int rc = 0;

	if(!lge_panel_sysfs_imgtune){
		lge_panel_sysfs_imgtune = device_create(panel, NULL, 0, fbi, "img_tune");
		if (IS_ERR(lge_panel_sysfs_imgtune)) {
			pr_err("%s: Failed to create dev(lge_panel_sysfs_imgtune)!", __func__);
		}
		else {
			rc = sysfs_create_group(&lge_panel_sysfs_imgtune->kobj, &lge_mdss_imgtune_attr_group);
#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON)
			rc += lge_mdss_mplus_sysfs_init(lge_panel_sysfs_imgtune, fbi);
#else
			rc += lge_mdss_lcd_sysfs_init(lge_panel_sysfs_imgtune, fbi);
#endif
			if (rc)
				pr_err("lge sysfs group creation failed, rc=%d\n", rc);
		}
	}
	return rc;
}

void lge_mdss_sysfs_imgtune_deinit(struct fb_info *fbi)
{
#if IS_ENABLED(CONFIG_LGE_DISPLAY_FALCON_COMMON)
		lge_mdss_mplus_sysfs_deinit(fbi);
#else
		lge_mdss_lcd_sysfs_deinit(fbi);
#endif

	sysfs_remove_group(&fbi->dev->kobj, &lge_mdss_imgtune_attr_group);
}
