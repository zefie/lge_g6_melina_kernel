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

#include <linux/of_platform.h>
#include "../mdss_dsi.h"
#include "lge_mdss_dsi_panel.h"

char *lge_blmap_name[] = {
	"lge,blmap",
	"lge,blmap-hl",
	"lge,blmap-ex",
	"lge,blmap-ex-dim",
	"lge,blmap-ex-hl",
	"lge,blmap-ex-dim-hl",
};

extern int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key);
extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_panel_cmds *pcmds, u32 flags);

#define	DDIC_NAME_LEN	15
static char lge_ddic_name[DDIC_NAME_LEN+1];

bool is_ddic_name(char *ddic_name)
{
	if (ddic_name == NULL) {
		pr_err("input ddic name is NULL\n");
		return false;
	}

	if(!strcmp(lge_ddic_name, ddic_name)) {
		return true;
	}

	pr_info("input ddic_name = %s, lge_ddic = %s\n", ddic_name, lge_ddic_name);
	return false;
}
EXPORT_SYMBOL(is_ddic_name);

void lge_mdss_panel_parse_ddic_name(struct device_node *np,
				   struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *ddic_name;

	memset(lge_ddic_name, 0x0, DDIC_NAME_LEN+1);
	ddic_name = of_get_property(np, "lge,ddic-name", NULL);
	if (ddic_name) {
		strncpy(lge_ddic_name, ddic_name, DDIC_NAME_LEN);
		pr_info("lge_ddic_name=%s\n", lge_ddic_name);
	} else {
		strncpy(lge_ddic_name, "undefined", DDIC_NAME_LEN);
		pr_info("ddic name is not set\n");
	}
}

#if IS_ENABLED(CONFIG_LGE_DISPLAY_HT_LCD_TUNE_MODE)
void ht_tune_mode_set(struct mdss_dsi_ctrl_pdata *ctrl)
{
	char ht_tune_name[10];
	snprintf(ht_tune_name, sizeof(ht_tune_name),"ht-tune-%d", ctrl->lge_extra.ht_mode);

	pr_info("ht_tune_name = %s\n",ht_tune_name);
	lge_send_extra_cmds_by_name(ctrl, ht_tune_name);

	return;
}
#endif


int lge_mdss_panel_parse_dt_extra_cmds(struct device_node *np,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc;
	int i;
	const char *name;
	char buf1[256];
	char buf2[256];

	rc = of_property_count_strings(np, "lge,mdss-dsi-extra-command-names");
	if (rc > 0) {
		ctrl_pdata->lge_extra.num_extra_cmds = rc;
		pr_info("num_extra_cmds=%d\n", ctrl_pdata->lge_extra.num_extra_cmds);
		ctrl_pdata->lge_extra.extra_cmds_array = kmalloc(sizeof(struct lge_dsi_cmds_entry)*ctrl_pdata->lge_extra.num_extra_cmds, GFP_KERNEL);
		if (NULL == ctrl_pdata->lge_extra.extra_cmds_array) {
			pr_err("no memory\n");
			ctrl_pdata->lge_extra.num_extra_cmds = 0;
			return -ENOMEM;
		}
		for (i = 0; i < ctrl_pdata->lge_extra.num_extra_cmds; i++) {
			of_property_read_string_index(np, "lge,mdss-dsi-extra-command-names", i, &name);
			pr_info("%s\n", name);
			strlcpy(ctrl_pdata->lge_extra.extra_cmds_array[i].name, name, sizeof(ctrl_pdata->lge_extra.extra_cmds_array[i].name));
			snprintf(buf1, sizeof(buf1), "lge,mdss-dsi-extra-command-%s", name);
			snprintf(buf2, sizeof(buf2), "lge,mdss-dsi-extra-command-state-%s", name);
			mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->lge_extra.extra_cmds_array[i].lge_dsi_cmds, buf1, buf2);
		}

	} else {
		ctrl_pdata->lge_extra.num_extra_cmds = 0;
	}

	return 0;
}

struct dsi_panel_cmds *lge_get_extra_cmds_by_name(struct mdss_dsi_ctrl_pdata *ctrl_pdata, char *name)
{
	int i;
	if (ctrl_pdata == NULL) {
		pr_err("ctrl_pdata is NULL\n");
		return NULL;
	}

	for (i = 0; i < ctrl_pdata->lge_extra.num_extra_cmds; ++i) {
		if (!strcmp(ctrl_pdata->lge_extra.extra_cmds_array[i].name, name))
			return &ctrl_pdata->lge_extra.extra_cmds_array[i].lge_dsi_cmds;
	}
	return NULL;
}

void lge_send_extra_cmds_by_name(struct mdss_dsi_ctrl_pdata *ctrl_pdata, char *name)
{
	struct dsi_panel_cmds *pcmds = lge_get_extra_cmds_by_name(ctrl_pdata, name);
	if (pcmds) {
		mdss_dsi_panel_cmds_send(ctrl_pdata, pcmds, CMD_REQ_COMMIT);
	} else {
		pr_err("unsupported cmds: %s\n", name);
	}
}


char *lge_get_blmapname(enum lge_bl_map_type  blmaptype)
{
	if (blmaptype >= 0 && blmaptype < LGE_BLMAPMAX)
		return lge_blmap_name[blmaptype];
	else
		return lge_blmap_name[LGE_BLDFT];
}

void lge_mdss_panel_parse_dt_blmaps(struct device_node *np,
				   struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int i, j, rc;
	u32 *array;

	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	pinfo->blmap_size = 256;
	array = kzalloc(sizeof(u32) * pinfo->blmap_size, GFP_KERNEL);

	if (!array)
		return;

	for (i = 0; i < LGE_BLMAPMAX; i++) {
		/* check if property exists */
		if (!of_find_property(np, lge_blmap_name[i], NULL))
			continue;

		pr_info("%s: found %s\n", __func__, lge_blmap_name[i]);

		rc = of_property_read_u32_array(np, lge_blmap_name[i], array,
						pinfo->blmap_size);
		if (rc) {
			pr_err("%s:%d, unable to read %s\n",
					__func__, __LINE__, lge_blmap_name[i]);
			goto error;
		}

		pinfo->blmap[i] = kzalloc(sizeof(int) * pinfo->blmap_size,
				GFP_KERNEL);

		if (!pinfo->blmap[i]){
			goto error;
		}

		for (j = 0; j < pinfo->blmap_size; j++)
			pinfo->blmap[i][j] = array[j];

	}
	kfree(array);
	return;

error:
	for (i = 0; i < LGE_BLMAPMAX; i++)
		if (pinfo->blmap[i])
			kfree(pinfo->blmap[i]);
	kfree(array);
}

void lge_mdss_panel_parse_dt_bl_list_maps(struct device_node *np,
				   struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *name;
	int i, rc;

	rc = of_property_count_strings(np, "lge,blmap-list");
	if (rc > 0) {
		ctrl_pdata->lge_extra.blmap_list_size = rc;
		pr_info("blmap_list_size=%d\n", ctrl_pdata->lge_extra.blmap_list_size);
		ctrl_pdata->lge_extra.blmap_list = kzalloc(sizeof(char*) * ctrl_pdata->lge_extra.blmap_list_size, GFP_KERNEL);
		ctrl_pdata->lge_extra.blmap = kzalloc(sizeof(int*) * ctrl_pdata->lge_extra.blmap_list_size, GFP_KERNEL);
		ctrl_pdata->lge_extra.blmap_size = kzalloc(sizeof(int) * ctrl_pdata->lge_extra.blmap_list_size, GFP_KERNEL);
		if (NULL == ctrl_pdata->lge_extra.blmap_list || NULL == ctrl_pdata->lge_extra.blmap || NULL == ctrl_pdata->lge_extra.blmap_size) {
			pr_err("allocation failed\n");
			ctrl_pdata->lge_extra.blmap_list_size = 0;
			goto error;
		}
		for (i = 0; i < ctrl_pdata->lge_extra.blmap_list_size; i++) {
			of_property_read_string_index(np, "lge,blmap-list", i, &name);
			ctrl_pdata->lge_extra.blmap_list[i] = kzalloc(strlen(name)+1, GFP_KERNEL);
			if (NULL == ctrl_pdata->lge_extra.blmap_list[i]) {
				pr_err("allocation for blmap name %s failed\n", name);
				goto error;
			}
			strcpy(ctrl_pdata->lge_extra.blmap_list[i], name);
			pr_info("%s\n", ctrl_pdata->lge_extra.blmap_list[i]);
			if (of_find_property(np, name, &ctrl_pdata->lge_extra.blmap_size[i])) {
				ctrl_pdata->lge_extra.blmap_size[i] /= sizeof(u32);
				ctrl_pdata->lge_extra.blmap[i] = kzalloc(sizeof(int) * ctrl_pdata->lge_extra.blmap_size[i], GFP_KERNEL);
				pr_info("blmap_size for blmap %s = %d\n", name, ctrl_pdata->lge_extra.blmap_size[i]);
				if (NULL == ctrl_pdata->lge_extra.blmap[i]) {
					pr_err("allocation for blmap %s failed\n", name);
					goto error;
				}
				if (of_property_read_u32_array(np, name, ctrl_pdata->lge_extra.blmap[i], ctrl_pdata->lge_extra.blmap_size[i])) {
					pr_err("parsing %s failed\n", name);
					kfree(ctrl_pdata->lge_extra.blmap[i]);
					ctrl_pdata->lge_extra.blmap[i] = NULL;
				}
			} else {
				ctrl_pdata->lge_extra.blmap_size[i] = 0;
				ctrl_pdata->lge_extra.blmap[i] = NULL;
			}
		}
	} else {
		ctrl_pdata->lge_extra.blmap_list_size = 0;
	}
	return;

error:
	for (i = 0; i < ctrl_pdata->lge_extra.blmap_list_size; ++i) {
		if (ctrl_pdata->lge_extra.blmap_list[i]) {
			kfree(ctrl_pdata->lge_extra.blmap_list[i]);
			ctrl_pdata->lge_extra.blmap_list[i] = NULL;
		}
		if (ctrl_pdata->lge_extra.blmap[i]) {
			kfree(ctrl_pdata->lge_extra.blmap[i]);
			ctrl_pdata->lge_extra.blmap[i] = NULL;
		}
	}
	if (ctrl_pdata->lge_extra.blmap_list) {
		kfree(ctrl_pdata->lge_extra.blmap_list);
		ctrl_pdata->lge_extra.blmap_list = NULL;
	}
	if (ctrl_pdata->lge_extra.blmap) {
		kfree(ctrl_pdata->lge_extra.blmap);
		ctrl_pdata->lge_extra.blmap = NULL;
	}
	if (ctrl_pdata->lge_extra.blmap_size) {
		kfree(ctrl_pdata->lge_extra.blmap_size);
		ctrl_pdata->lge_extra.blmap_size = NULL;
	}
	ctrl_pdata->lge_extra.blmap_list_size = 0;
}

#ifdef CONFIG_LGE_DISPLAY_BL_EXTENDED
int mdss_panel_parse_blex_settings(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *data;

	ctrl_pdata->bkltex_ctrl = UNKNOWN_CTRL;
	data = of_get_property(np, "qcom,mdss-dsi-blex-pmic-control-type", NULL);
	if (data) {
		/* TODO: implement other types of backlight */
		if (!strcmp(data, "bl_ctrl_lge")) {
			ctrl_pdata->bkltex_ctrl = BL_OTHERS;
			pr_info("%s: Configured BL_OTHERS bkltex ctrl\n",
								__func__);
		} else {
			pr_info("%s: bkltex ctrl configuration fail\n",
								__func__);
		}
	}
	return 0;
}
#endif

static int find_blmap_index_by_name(struct mdss_dsi_ctrl_pdata *ctrl_pdata, const char *name)
{
	int index = -1;
	int i;

	for (i = 0; i < ctrl_pdata->lge_extra.blmap_list_size; ++i) {
		if (ctrl_pdata->lge_extra.blmap_list[i] && !strncmp(ctrl_pdata->lge_extra.blmap_list[i], name, strlen(name))) {
			index = i;
			break;
		}
	}
	if (index == -1)
		pr_err("index of blmap %s not found, blmap_list_size=%d\n", name, ctrl_pdata->lge_extra.blmap_list_size);
	return index;
}

static int lge_mdss_panel_parse_mplus_dt(struct device_node *np,
						struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *name;
	int i, rc;

	ctrl_pdata->lge_extra.mp_to_blmap_tbl = NULL;
	ctrl_pdata->lge_extra.mp_to_blmap_tbl_size = 0;

	ctrl_pdata->lge_extra.use_mplus =
		LGE_DDIC_OP_CHECK(ctrl_pdata, mplus_mode_set)
		&& LGE_DDIC_OP_CHECK(ctrl_pdata, mplus_mode_get)
		&& LGE_DDIC_OP_CHECK(ctrl_pdata, mplus_max_set)
		&& LGE_DDIC_OP_CHECK(ctrl_pdata, mplus_max_get)
		&& LGE_DDIC_OP_CHECK(ctrl_pdata, mplus_hd_set)
		&& LGE_DDIC_OP_CHECK(ctrl_pdata, mplus_hd_get);

	if (!ctrl_pdata->lge_extra.use_mplus)
		return 0;

	rc = of_property_count_strings(np, "lge,blmap-for-mplus-mode");
	if (rc > 0) {
		ctrl_pdata->lge_extra.mp_to_blmap_tbl_size = rc;
		pr_info("mp_to_blmap_tbl_size=%d\n", ctrl_pdata->lge_extra.mp_to_blmap_tbl_size);
		ctrl_pdata->lge_extra.mp_to_blmap_tbl = kzalloc(sizeof(int) * ctrl_pdata->lge_extra.blmap_list_size, GFP_KERNEL);
		if (NULL == ctrl_pdata->lge_extra.mp_to_blmap_tbl) {
			pr_err("allocation failed\n");
			ctrl_pdata->lge_extra.use_mplus = false;
			ctrl_pdata->lge_extra.mp_to_blmap_tbl_size = 0;
			return -ENOMEM;
		}
		for (i = 0; i < ctrl_pdata->lge_extra.mp_to_blmap_tbl_size; i++) {
			of_property_read_string_index(np, "lge,blmap-for-mplus-mode", i, &name);
			pr_info("%s\n", name);
			ctrl_pdata->lge_extra.mp_to_blmap_tbl[i] = find_blmap_index_by_name(ctrl_pdata, name);
		}
	} else {
		pr_err("lge,blmap-for-mplus-mode not exist");
		ctrl_pdata->lge_extra.use_mplus = false;
	}
	return 0;
}

static int lge_mdss_panel_parse_feature_dt(struct device_node *np,
						struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct lge_mdss_dsi_ctrl_pdata *lge_extra;
	int rc = 0;
	u32 tmp = 0;

	if (np == NULL || ctrl_pdata == NULL) {
		pr_err("Invalid input\n");
		return -EINVAL;
	}

	lge_extra = &(ctrl_pdata->lge_extra);

	lge_extra->use_u2_fsc = of_property_read_bool(np, "lge,use-u2-fsc");
	if (lge_extra->use_u2_fsc) {
		rc = of_property_read_u32(np, "lge,fsc-curr-ua-u3", &tmp);
		if (rc) {
			lge_extra->fsc_u3 = DEFAULT_LGE_FSC_U3;
			pr_err("fail to parse lge,fsc-curr-ua-u3 Set to Default %d\n", lge_extra->fsc_u3);
		} else {
			lge_extra->fsc_u3 = tmp;
			pr_info("fsc_u3 %d\n", lge_extra->fsc_u3);
		}
		rc = of_property_read_u32(np, "lge,fsc-curr-ua-u2", &tmp);
		if (rc) {
			lge_extra->fsc_u2 = DEFAULT_LGE_FSC_U2;
			pr_err("fail to parse lge,fsc-curr-ua-u2 Set to Default %d\n", lge_extra->fsc_u2);
		} else {
			lge_extra->fsc_u2 = tmp;
			pr_info("fsc_u2 %d\n", lge_extra->fsc_u2);
		}
	} else {
		pr_err("lge,use-u2-fsc not exist");
	}

	return 0;
}

static int lge_mdss_panel_parse_dt(struct device_node *np,
						struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	lge_mdss_panel_parse_ddic_name(np, ctrl_pdata);

	lge_mdss_panel_parse_dt_bl_list_maps(np, ctrl_pdata);
	if (ctrl_pdata->lge_extra.blmap_list_size == 0)
		lge_mdss_panel_parse_dt_blmaps(np, ctrl_pdata);

	lge_mdss_panel_parse_mplus_dt(np, ctrl_pdata);
	lge_mdss_panel_parse_dt_extra_cmds(np, ctrl_pdata);
	lge_mdss_panel_parse_feature_dt(np, ctrl_pdata);

	return 0;
}

int lge_ddic_feature_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	if (ctrl_pdata == NULL)
		return -ENODEV;

	ctrl_pdata->lge_extra.hdr_mode = HDR_OFF;
	ctrl_pdata->lge_extra.dolby_mode = DOLBY_OFF;
	ctrl_pdata->lge_extra.sre_mode = SRE_OFF;
	ctrl_pdata->lge_extra.hl_mode = HL_MODE_OFF;
	ctrl_pdata->lge_extra.aod_interface = 0x144;
	ctrl_pdata->lge_extra.cm_preset_step = RGB_DEFAULT_PRESET;
	ctrl_pdata->lge_extra.cm_red_step = RGB_DEFAULT_RED;
	ctrl_pdata->lge_extra.cm_green_step = RGB_DEFAULT_GREEN;
	ctrl_pdata->lge_extra.cm_blue_step = RGB_DEFAULT_BLUE;
	ctrl_pdata->lge_extra.sharpness = SHA_OFF;
	ctrl_pdata->lge_extra.ie_control = IE_OFF;

	ctrl_pdata->lge_extra.sc_sat_step = SC_MODE_DEFAULT;
	ctrl_pdata->lge_extra.sc_hue_step = SC_MODE_DEFAULT;
	ctrl_pdata->lge_extra.sc_sha_step = SC_MODE_DEFAULT;
	ctrl_pdata->lge_extra.color_filter = SC_MODE_DEFAULT;

	if (ctrl_pdata->lge_extra.use_mplus) {
		ctrl_pdata->lge_extra.mplus_hd = LGE_MP_OFF;
		ctrl_pdata->lge_extra.mp_max = LGE_MP_OFF;
		ctrl_pdata->lge_extra.mp_mode = LGE_MP_OFF;
		ctrl_pdata->lge_extra.adv_mp_mode = LGE_MP_OFF;
		ctrl_pdata->lge_extra.cur_mp_mode = LGE_MP_NOR;
		ctrl_pdata->lge_extra.mplus_dim_cnt = LGE_MPLUS_BR_DIM_CNT;
		ctrl_pdata->lge_extra.mplus_dim_delay = LGE_MPLUS_BR_DIM_DELAY;
	}
	return 0;
}

int lge_mdss_dsi_panel_init(struct device_node *node, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	lge_ddic_ops_init(ctrl_pdata);

	lge_mdss_panel_parse_dt(node, ctrl_pdata);

	lge_ddic_feature_init(ctrl_pdata);
	return 0;
}

static int lge_panel_get_blmap_type(struct mdss_dsi_ctrl_pdata *ctrl)
{
#if defined(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED)
	if (mdss_dsi_is_panel_on_lp(&ctrl->panel_data))
		return find_blmap_index_by_name(ctrl, "lge,blmap-ex");
#endif

	if (!LGE_DDIC_OP_CHECK(ctrl, get_blmap_type))
		return 0;

	return LGE_DDIC_OP(ctrl, get_blmap_type);
}

int lge_panel_br_to_bl(struct mdss_dsi_ctrl_pdata *ctrl, int br_lvl)
{
	int type = 0;
	int *blmap = NULL;
	int blmap_size = 0;

	if (ctrl) {
		type = lge_panel_get_blmap_type(ctrl);
		if (ctrl->lge_extra.blmap_list_size && type >= 0 && type < ctrl->lge_extra.blmap_list_size) {
			blmap = ctrl->lge_extra.blmap[type];
			blmap_size = ctrl->lge_extra.blmap_size[type];
			pr_debug("blmap type = %s\n", ctrl->lge_extra.blmap_list[type]);
		}
	} else {
		pr_err("ctrl is NULL\n");
	}

	if (blmap == NULL || blmap_size == 0) {
		pr_err("there is no blmap\n");
		return 100;
	}
	if (br_lvl < 0)
		br_lvl = 0;
	if (br_lvl >= blmap_size)
		br_lvl = blmap_size-1;

	return blmap[br_lvl];
}
