#define pr_fmt(fmt)	"[Display] %s: " fmt, __func__

#include <linux/sysfs.h>
#include <linux/device.h>
#include "../mdss_fb.h"
#include "../mdss_mdp.h"
#include "../mdss_dsi.h"

#include "lge_mdss_sysfs.h"

/* "/sys/class/panel/aod/" */
struct device *lge_panel_sysfs_aod = NULL;

/* AoD Area */
static ssize_t area_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d %d %d %d\n",
						ctrl->lge_extra.aod_area.x,
						ctrl->lge_extra.aod_area.y,
						ctrl->lge_extra.aod_area.w,
						ctrl->lge_extra.aod_area.h);
}

static ssize_t area_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int tmp;
	struct lge_rect area;
	GET_DATA

	sscanf(buf, "%d %d %d %d %d", &tmp, &area.x, &area.y, &area.w, &area.h);
	ctrl->lge_extra.aod_area = area;
	if (mdss_dsi_is_panel_on_lp(&ctrl->panel_data))
		LGE_DDIC_OP(ctrl, send_u2_cmds);
	return ret;
}
static DEVICE_ATTR(area, S_IRUGO|S_IWUSR|S_IWGRP, area_get, area_set);

static ssize_t aod_interface_get(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	GET_DATA

	return sprintf(buf, "%d\n", ctrl->lge_extra.aod_interface);
}

static DEVICE_ATTR(aod_interface, S_IRUGO, aod_interface_get, NULL);


/* "/sys/class/panel/aod/" *" */
struct attribute *lge_mdss_aod_sysfs_list[] = {
	&dev_attr_area.attr,
	&dev_attr_aod_interface.attr,
	NULL,
};

static struct attribute_group lge_mdss_aod_sysfs_group = {
	.attrs = lge_mdss_aod_sysfs_list
};

int lge_mdss_aod_sysfs_init(struct class *panel, struct fb_info *fbi)
{
	int ret = 0;
	if(!lge_panel_sysfs_aod && panel && fbi) {
		lge_panel_sysfs_aod = device_create(panel, NULL, 0, fbi, "aod");
		if(IS_ERR(lge_panel_sysfs_aod)) {
			pr_err("Failed to create dev(lge_panel_sysfs_aod)!\n");
		}
		else {
			ret += sysfs_create_group(&lge_panel_sysfs_aod->kobj, &lge_mdss_aod_sysfs_group);
		}
	}

	return ret;
}

void lge_mdss_aod_sysfs_deinit(struct fb_info *fbi)
{
	sysfs_remove_group(&fbi->dev->kobj, &lge_mdss_aod_sysfs_group);
}

