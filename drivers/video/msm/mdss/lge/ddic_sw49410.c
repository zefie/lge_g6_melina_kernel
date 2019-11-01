#define pr_fmt(fmt)	"[Display] %s: " fmt, __func__

#include <linux/delay.h>
#include "../mdss_dsi.h"

#define NUM_SHA_CTRL 		5
#define NUM_SC_CTRL 		4
#define OFFSET_SC_CTRL		1
#define NUM_SAT_CTRL 		6
#define OFFSET_SAT_CTRL 	8
#define NUM_HUE_CTRL 		5
#define OFFSET_HUE_CTRL 	2

#define BLMAP_HL_MODE_OFFSET 	4

#define NUM_DG_PRESET		10
#define STEP_GC_PRESET      5

#define LGE_SCREEN_TUNE_OFF 0
#define LGE_SCREEN_TUNE_ON  1
#define LGE_SCREEN_TUNE_GAM 2
#define LGE_SCREEN_TUNE_GAL 3
#define LGE_SAT_GAM_MODE    3
#define LGE_SAT_GAL_MODE    5

enum {
	PRESET_SETP0_OFFSET = 0,
	PRESET_SETP1_OFFSET = 2,
	PRESET_SETP2_OFFSET = 5
};

static char sha_ctrl_values[NUM_SHA_CTRL] = {0x00, 0x0D, 0x1A, 0x30, 0xD2};

static char sc_ctrl_values[NUM_SC_CTRL] = {0x00, 0x0F, 0x08, 0x0C};

static char sat_ctrl_values[NUM_SAT_CTRL][OFFSET_SAT_CTRL] = {
	{0x00, 0x38, 0x70, 0xA8, 0xE1, 0x00, 0x00, 0x00},
	{0x00, 0x3C, 0x78, 0xB4, 0xF1, 0x00, 0x00, 0x00},
	{0x00, 0x40, 0x80, 0xC0, 0x00, 0x01, 0x00, 0x00},
	{0x00, 0x43, 0x87, 0xCB, 0x00, 0x01, 0x00, 0x00},
	{0x00, 0x47, 0x8F, 0xD7, 0x00, 0x01, 0x00, 0x00},
	{0x00, 0x52, 0x8C, 0xD7, 0x00, 0x01, 0x90, 0x90},
};

static char hue_ctrl_values[NUM_HUE_CTRL][OFFSET_HUE_CTRL] = {
	{0xF7, 0x00},
	{0xF4, 0x00},
	{0xF0, 0x00},
	{0x74, 0x00},
	{0x77, 0x00},
};

static int rgb_preset[STEP_DG_PRESET][RGB_ALL] = {
	{PRESET_SETP2_OFFSET, PRESET_SETP0_OFFSET, PRESET_SETP2_OFFSET},
	{PRESET_SETP2_OFFSET, PRESET_SETP1_OFFSET, PRESET_SETP2_OFFSET},
	{PRESET_SETP0_OFFSET, PRESET_SETP0_OFFSET, PRESET_SETP0_OFFSET},
	{PRESET_SETP0_OFFSET, PRESET_SETP1_OFFSET, PRESET_SETP0_OFFSET},
	{PRESET_SETP0_OFFSET, PRESET_SETP2_OFFSET, PRESET_SETP0_OFFSET}
};

static int gc_preset[STEP_GC_PRESET][RGB_ALL] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x01, 0x05},
	{0x04, 0x01, 0x00},
	{0x02, 0x00, 0x00},
	{0x02, 0x00, 0x02},
};

static char dg_ctrl_values[NUM_DG_PRESET][OFFSET_DG_CTRL] = {
	{0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
	{0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F},
	{0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E},
	{0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D},
	{0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C},

	{0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B},
	{0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A},
	{0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39},
	{0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38},
	{0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37}
};

static int mplus_mode_to_dic_mp[LGE_MP_MAX][2] = {
	{LGE_DIC_MP_NOR, LGE_MODE_SET_1ST}, // LGE_MP_NOR
	{LGE_DIC_MP_PSM, LGE_MODE_SET_1ST}, // LGE_MP_PSM
	{LGE_DIC_MP_HBM, LGE_MODE_SET_1ST}, // LGE_MP_HBM
	{LGE_DIC_MP_GAL, LGE_MODE_SET_1ST}, // LGE_MP_GAL
	{LGE_DIC_MP_GAL, LGE_MODE_SET_2ND}, // LGE_MP_BRI not used
	{LGE_DIC_MP_PSM, LGE_MODE_SET_1ST}, // LGE_MP_PS2 not used
	{LGE_DIC_MP_GAL, LGE_MODE_SET_3RD}, // LGE_MP_HQC
	{LGE_DIC_MP_GAL, LGE_MODE_SET_1ST}, // LGE_MP_FHB
	{LGE_DIC_MP_NOR, LGE_MODE_SET_1ST}, // LGE_MP_OFF
};

static bool use_u2_vs = true;
module_param(use_u2_vs, bool, S_IRUGO|S_IWUSR|S_IWGRP);

#define SW49410_REG_TRIMMING 0xC7
#define SW49410_REVISION_NO_1 0xF1
#define REVISION_UNIDENTIFIED -1
static int revision = REVISION_UNIDENTIFIED;
module_param(revision, int,  S_IRUGO|S_IWUSR|S_IWGRP);

extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_panel_cmds *pcmds, u32 flags);
extern int lge_mdss_fb_get_bl_brightness(void);
extern int mdss_dsi_clk_ctrl(struct mdss_dsi_ctrl_pdata *ctrl, void *clk_handle,
		enum mdss_dsi_clk_type clk_type, enum mdss_dsi_clk_state clk_state);
extern int mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
		char cmd1, void (*fxn)(int), char *rbuf, int len);

void identify_revision_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (likely(revision != REVISION_UNIDENTIFIED)) {
		return;
	} else {
		char rx_buf = 0xFF;
		mdss_dsi_clk_ctrl(ctrl, ctrl->dsi_clk_handle, MDSS_DSI_ALL_CLKS, MDSS_DSI_CLK_ON);
		mdss_dsi_panel_cmd_read(ctrl, SW49410_REG_TRIMMING, 0, NULL, &rx_buf, sizeof(rx_buf));
		mdss_dsi_clk_ctrl(ctrl, ctrl->dsi_clk_handle, MDSS_DSI_ALL_CLKS, MDSS_DSI_CLK_OFF);
		pr_info("revision no. = 0x%X\n", rx_buf);
		revision = (rx_buf==SW49410_REVISION_NO_1)?1:0;
	}
}

static void mplus_change_blmap_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int brightness;
	int type;
	int bl_lvl;

	if (ctrl == NULL) {
		pr_err("%pS: ctrl == NULL\n", __builtin_return_address(0));
		return;
	}

	if (mdss_dsi_is_panel_off(&ctrl->panel_data) || mdss_dsi_is_panel_on_lp(&ctrl->panel_data)) {
		pr_info("unexpected panel power state while changing blmap\n");
		return;
	}

	if (ctrl->lge_extra.blmap == NULL || ctrl->lge_extra.blmap_size == 0) {
		pr_err("no blmap\n");
		return;
	}

	type = LGE_DDIC_OP(ctrl, get_blmap_type);
	if (type < 0 || type >= ctrl->lge_extra.blmap_list_size)
		type = 0;

	brightness = lge_mdss_fb_get_bl_brightness();
	if (brightness >= 0 && brightness < ctrl->lge_extra.blmap_size[type]) {
		bl_lvl = ctrl->lge_extra.blmap[type][brightness];
		ctrl->panel_data.set_backlight(&ctrl->panel_data, bl_lvl);
	} else {
		pr_err("brightness(%d) is out of range(%d), blmap type=%s\n", brightness, ctrl->lge_extra.blmap_size[type], ctrl->lge_extra.blmap_list[type]);
	}
}

static void lge_set_rgb_tune_send_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, enum lge_gamma_correction_mode gc_mode)
{
	int i = 0;
	int red_index = 0, green_index = 0, blue_index = 0, dg_status = 0;
	int red_sum = 0, green_sum = 0, blue_sum = 0;
	char *payload_ctrl[4] = {NULL, };

	struct dsi_panel_cmds *pcmds;

	if (ctrl == NULL) {
		pr_err("Invalid input\n");
		return;
	}

	pcmds = lge_get_extra_cmds_by_name(ctrl, "dg-dummy");
	if (pcmds) {
		for (i = 0; i < pcmds->cmd_cnt; i++)
		payload_ctrl[i] = pcmds->cmds[i].payload;
	} else {
		pr_err("no cmds: dg-dummy\n");
		return;
	}

	if (gc_mode == LGE_GC_MOD_NOR) {
		if (ctrl->lge_extra.cm_red_step > DG_MODE_MAX)
			ctrl->lge_extra.cm_red_step = DG_MODE_MAX;
		if (ctrl->lge_extra.cm_green_step > DG_MODE_MAX)
			ctrl->lge_extra.cm_green_step = DG_MODE_MAX;
		if (ctrl->lge_extra.cm_blue_step > DG_MODE_MAX)
			ctrl->lge_extra.cm_blue_step = DG_MODE_MAX;

		if ((ctrl->lge_extra.cm_preset_step == RGB_DEFAULT_PRESET) &&
			(ctrl->lge_extra.cm_red_step == RGB_DEFAULT_RED) &&
			(ctrl->lge_extra.cm_blue_step == RGB_DEFAULT_BLUE) &&
			(ctrl->lge_extra.cm_green_step == RGB_DEFAULT_GREEN)) {
			dg_status = DG_OFF;
		} else {
			dg_status = DG_ON;
		}

		if (dg_status == DG_ON) {
			red_index   = rgb_preset[ctrl->lge_extra.cm_preset_step][RED] + ctrl->lge_extra.cm_red_step;
			green_index = rgb_preset[ctrl->lge_extra.cm_preset_step][GREEN] + ctrl->lge_extra.cm_green_step;
			blue_index  = rgb_preset[ctrl->lge_extra.cm_preset_step][BLUE] + ctrl->lge_extra.cm_blue_step;
			pr_info("red_index=(%d) green_index=(%d) blue_index=(%d)\n", red_index, green_index, blue_index);

			for (i = 1; i < OFFSET_DG_CTRL+1; i++) {
				payload_ctrl[RED][i] = dg_ctrl_values[red_index][i-1];
				payload_ctrl[GREEN][i] = dg_ctrl_values[green_index][i-1];
				payload_ctrl[BLUE][i] = dg_ctrl_values[blue_index][i-1];
			}
		}
		payload_ctrl[RGB_ALL][1] = dg_status;
	} else {
		red_index   = gc_preset[gc_mode][RED];
		green_index = gc_preset[gc_mode][GREEN];
		blue_index  = gc_preset[gc_mode][BLUE];

		for (i = 1; i < OFFSET_DG_CTRL+1; i++) {
			payload_ctrl[RED][i] = dg_ctrl_values[red_index][i-1];
			payload_ctrl[GREEN][i] = dg_ctrl_values[green_index][i-1];
			payload_ctrl[BLUE][i] = dg_ctrl_values[blue_index][i-1];
		}
		dg_status = DG_ON;
		payload_ctrl[RGB_ALL][1] = dg_status;
	}
	mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);

	for (i = 1; i < NUM_DG_CTRL+1; i++) {
		red_sum += payload_ctrl[RED][i];
		green_sum += payload_ctrl[GREEN][i];
		blue_sum += payload_ctrl[BLUE][i];
	}

	pr_info("[0x%02x:0x%02x][0x%02x:0x%02x:%d][0x%02x:0x%02x:%d][0x%02x:0x%02x:%d]\n",
			payload_ctrl[RGB_ALL][0], payload_ctrl[RGB_ALL][1],
			payload_ctrl[RED][0], payload_ctrl[RED][1], red_sum,
			payload_ctrl[GREEN][0], payload_ctrl[GREEN][1], green_sum,
			payload_ctrl[BLUE][0], payload_ctrl[BLUE][1], blue_sum);

	return;
}

static void lge_set_screen_tune_send_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int lge_screen_tune_status)
{
	int i = 0;
	char *payload_ctrl[2] = {NULL, };
	struct dsi_panel_cmds *pcmds;

	pcmds = lge_get_extra_cmds_by_name(ctrl, "color-dummy");
	if (pcmds) {
		for (i = 0; i < pcmds->cmd_cnt; i++)
			payload_ctrl[i] = pcmds->cmds[i].payload;
	} else {
		pr_err("no cmds: color-dummy\n");
		return;
	}

	if (lge_screen_tune_status == LGE_SCREEN_TUNE_ON) {
		if (ctrl->lge_extra.sc_sat_step > SC_MODE_MAX)
			ctrl->lge_extra.sc_sat_step = SC_MODE_MAX;
		if (ctrl->lge_extra.sc_hue_step > SC_MODE_MAX)
			ctrl->lge_extra.sc_hue_step = SC_MODE_MAX;
		if (ctrl->lge_extra.sc_sha_step > SC_MODE_MAX)
			ctrl->lge_extra.sc_sha_step = SC_MODE_MAX;

		payload_ctrl[0][1] = SHA_ON;
		payload_ctrl[0][4] = sha_ctrl_values[ctrl->lge_extra.sc_sha_step];
		ctrl->lge_extra.sharpness_control = true;

		payload_ctrl[1][1] = sc_ctrl_values[lge_screen_tune_status];
		for (i = 0; i < OFFSET_SAT_CTRL; i++)
			payload_ctrl[1][i+2] = sat_ctrl_values[ctrl->lge_extra.sc_sat_step][i];
		for (i = 0; i < OFFSET_HUE_CTRL; i++)
			payload_ctrl[1][i+8] = hue_ctrl_values[ctrl->lge_extra.sc_hue_step][i];
	} else {
		if (ctrl->lge_extra.sharpness == SHA_OFF) {
			payload_ctrl[0][1] = SHA_OFF;
		} else {
			payload_ctrl[0][1] = SHA_ON;
			payload_ctrl[0][4] = ctrl->lge_extra.sharpness;
		}

		if (lge_screen_tune_status == LGE_SCREEN_TUNE_GAM) {
			ctrl->lge_extra.sc_sat_step = LGE_SAT_GAM_MODE;
			payload_ctrl[1][1] = sc_ctrl_values[lge_screen_tune_status];
			for (i = 0; i < OFFSET_SAT_CTRL; i++)
				payload_ctrl[1][i+2] = sat_ctrl_values[ctrl->lge_extra.sc_sat_step][i];
		} else if (lge_screen_tune_status == LGE_SCREEN_TUNE_GAL) {
			ctrl->lge_extra.sc_sat_step = LGE_SAT_GAL_MODE;
			payload_ctrl[1][1] = sc_ctrl_values[lge_screen_tune_status];
			for (i = 0; i < OFFSET_SAT_CTRL; i++)
				payload_ctrl[1][i+2] = sat_ctrl_values[ctrl->lge_extra.sc_sat_step][i];
		} else {
			payload_ctrl[1][1] = sc_ctrl_values[lge_screen_tune_status];
		}
		ctrl->lge_extra.sharpness_control = false;
	}
	mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
	pr_info("[0x%02x:0x%02x:0x%02x][0x%02x:0x%02x:0x%02x:0x%02x]\n",
			payload_ctrl[0][0], payload_ctrl[0][1], payload_ctrl[0][4],
			payload_ctrl[1][0], payload_ctrl[1][1], payload_ctrl[1][3],
			payload_ctrl[1][8]);

	return;
}

void sharpness_set_send_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int mode = 0;
	struct dsi_panel_cmds *pcmds;

	if (ctrl->lge_extra.sharpness_control == true ) {
		pr_info("skip sharpness control! \n");
		return;
	}

	pcmds = lge_get_extra_cmds_by_name(ctrl, "sharpness");
	mode = ctrl->lge_extra.sharpness;

	if (pcmds) {
		if (mode == 0) {
			pcmds->cmds[0].payload[1] = SHA_OFF;
		} else {
			pcmds->cmds[0].payload[1] = SHA_ON;
			pcmds->cmds[0].payload[4] = mode;
		}
		pr_info("sharpness=%d\n", mode);
		mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
	} else {
		pr_err("no cmds: sharpness\n");
	}
}

static void dic_mplus_mode_set(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	struct dsi_panel_cmds *pcmds;
	enum lge_mplus_mode req_mp_mode;
	enum lge_mplus_mode old_mp_mode;
	enum lge_gamma_correction_mode gc_mode = LGE_GC_MOD_NOR;
	int lge_screen_tune_status = LGE_SCREEN_TUNE_OFF;

	int mpl_mplus_mode, mplwr_no;
	char buf[128];

	if (ctrl == NULL) {
		pr_err("%pS: ctrl == NULL\n", __builtin_return_address(0));
		return;
	}

	if (mdss_dsi_is_panel_off(&ctrl->panel_data) || mdss_dsi_is_panel_on_lp(&ctrl->panel_data)) {
		pr_info("unexpected panel power state while setting mplus mode\n");
		return;
	}

	req_mp_mode = mode;
	old_mp_mode = ctrl->lge_extra.cur_mp_mode;

	if (req_mp_mode < 0 || req_mp_mode >= LGE_MP_MAX) {
		pr_err("%pS: unsupproted mode:%d, mplus_hd:%d, mp_max:%d, mp_mode:%d\n", __builtin_return_address(0),
			req_mp_mode, ctrl->lge_extra.mplus_hd, ctrl->lge_extra.mp_max, ctrl->lge_extra.mp_mode);
		req_mp_mode = LGE_MP_NOR;
	}

	pr_info("%pS: req mplus mode: %d\n", __builtin_return_address(0), req_mp_mode);

	mpl_mplus_mode = mplus_mode_to_dic_mp[req_mp_mode][0];
	mplwr_no = mplus_mode_to_dic_mp[req_mp_mode][1];

	switch (ctrl->lge_extra.screen_mode) {
	case LGE_COLOR_CIN:
		gc_mode = LGE_GC_MOD_CIN;
		break;
	case LGE_COLOR_SPO:
		gc_mode = LGE_GC_MOD_SPO;
		break;
	case LGE_COLOR_GAM:
		gc_mode = LGE_GC_MOD_GAM;
		lge_screen_tune_status = LGE_SCREEN_TUNE_GAM;
		break;
	case LGE_COLOR_MAN:
		lge_screen_tune_status = LGE_SCREEN_TUNE_ON;
		if(ctrl->lge_extra.color_filter)
		    gc_mode = LGE_GC_MOD_HQC;
		break;
	case LGE_COLOR_OPT:
	default:
		break;
    }
	if ((ctrl->lge_extra.mp_mode == LGE_MP_GAL) &&
			(lge_screen_tune_status == LGE_SCREEN_TUNE_OFF)) {
		lge_screen_tune_status = LGE_SCREEN_TUNE_GAL;
	}
	pr_info("gc_mode = %d, lge_screen_tune_status = %d\n", gc_mode, lge_screen_tune_status);

	pcmds = lge_get_extra_cmds_by_name(ctrl, "mplus-ctrl");
	if (pcmds) {
		pcmds->cmds[0].payload[1] &= 0xCF;
		pcmds->cmds[0].payload[1] |= (mpl_mplus_mode & 0x3) << 4;
		mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
		pr_info("mpl_mplus_mode = %d, mplwr_no = %d mp mode : %x\n", mpl_mplus_mode, mplwr_no, pcmds->cmds[0].payload[1]);
	} else {
		pr_err("no cmds: mplus-ctrl\n");
	}
	snprintf(buf, sizeof(buf), "mplus-wr-start%d", mplwr_no);

	/*
	 * can't re-write to mplwr(D4h) register
	 * this issue is fixed on Rev1
	 */
	if (revision == 1)
		lge_send_extra_cmds_by_name(ctrl, buf);

	lge_set_rgb_tune_send_sw49410(ctrl, gc_mode);
	lge_set_screen_tune_send_sw49410(ctrl, lge_screen_tune_status);
	sharpness_set_send_sw49410(ctrl);

	if ((old_mp_mode == LGE_MP_HBM) || (req_mp_mode == LGE_MP_HBM) ||
		(old_mp_mode == LGE_MP_FHB) || (req_mp_mode == LGE_MP_FHB)) {
		pr_debug("skip brightness dimming\n");
	} else {
		pr_debug("brightness dimming\n");
		// TBD
	}

	ctrl->lge_extra.cur_mp_mode = req_mp_mode;

	pr_info("[mp_hd:%d][hdr:%d][hl:%d][max:%d][mp_adv:%d][mp:%d][gc:%d][sc_tun:%d][shap:%d]\n",
			ctrl->lge_extra.mplus_hd, ctrl->lge_extra.hdr_mode, ctrl->lge_extra.hl_mode,
			ctrl->lge_extra.mp_max, ctrl->lge_extra.adv_mp_mode, ctrl->lge_extra.mp_mode,
			gc_mode, lge_screen_tune_status, ctrl->lge_extra.sharpness);
}

static void mplus_mode_set_sub_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl->lge_extra.mplus_hd != LGE_MP_OFF)
		dic_mplus_mode_set(ctrl, ctrl->lge_extra.mplus_hd);
	else if (ctrl->lge_extra.hdr_mode)
		dic_mplus_mode_set(ctrl, LGE_MP_NOR);
	else if (ctrl->lge_extra.hl_mode)
		dic_mplus_mode_set(ctrl, LGE_MP_HBM);
	else if (ctrl->lge_extra.mp_max != LGE_MP_OFF)
		dic_mplus_mode_set(ctrl, LGE_MP_FHB);
	else if (ctrl->lge_extra.adv_mp_mode != LGE_MP_OFF)
		dic_mplus_mode_set(ctrl, ctrl->lge_extra.adv_mp_mode);
	else
		dic_mplus_mode_set(ctrl, ctrl->lge_extra.mp_mode);
}

static int image_enhance_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;
	ctrl->lge_extra.mp_mode = mode;
	pr_info("mp_mode : %d\n", mode);
	mplus_mode_set_sub_sw49410(ctrl);
	return 0;
}

static int image_enhance_get_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;
	return ctrl->lge_extra.mp_mode;
}

static int mplus_mode_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;
	ctrl->lge_extra.adv_mp_mode = mode;
	pr_info("adv_mp_mode : %d\n", mode);
	mplus_mode_set_sub_sw49410(ctrl);
	return 0;
}

static int mplus_mode_get_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;
	return ctrl->lge_extra.adv_mp_mode;
}

static int mplus_hd_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;
	ctrl->lge_extra.mplus_hd = mode;
	pr_info("mplus_hd : %d\n", mode);
	mplus_mode_set_sub_sw49410(ctrl);
	return 0;
}

static int mplus_hd_get_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;
	return ctrl->lge_extra.mplus_hd;
}

static int mplus_max_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;
	ctrl->lge_extra.mp_max = mode;
	pr_info("mp_max : %d\n", mode);
	mplus_mode_set_sub_sw49410(ctrl);
	return 0;
}

static int mplus_max_get_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;
	return ctrl->lge_extra.mp_max;
}

static int get_blmap_type_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int mode;
	int type = 0;
	if (ctrl == NULL)
		return 0;

	if (ctrl->lge_extra.use_mplus) {
		if (ctrl->lge_extra.mplus_hd != LGE_MP_OFF)
			mode = ctrl->lge_extra.mplus_hd;
		else if (ctrl->lge_extra.mp_max != LGE_MP_OFF)
			mode = LGE_MP_FHB;
		else if (ctrl->lge_extra.hdr_mode)
			mode = LGE_MP_NOR;
		else if (ctrl->lge_extra.adv_mp_mode != LGE_MP_OFF)
			mode = ctrl->lge_extra.adv_mp_mode;
		else
			mode = ctrl->lge_extra.mp_mode;

		if (ctrl->lge_extra.mp_to_blmap_tbl_size > 0 && mode >= ctrl->lge_extra.mp_to_blmap_tbl_size) {
			pr_err("no blmap for mplus mode %d\n", mode);
			mode = LGE_MP_NOR;
		}
		type = ctrl->lge_extra.mp_to_blmap_tbl[mode];

		if (ctrl->lge_extra.hl_mode == 1 && ctrl->lge_extra.mplus_hd == LGE_MP_OFF && type < BLMAP_HL_MODE_OFFSET) {
			pr_info("adding hl mode offset to blmap type\n");
			type += BLMAP_HL_MODE_OFFSET;
		}
		pr_debug("searching mp mode=%d, type=%d\n", mode, type);
	}

	return type;
}

static int screen_mode_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;

	ctrl->lge_extra.screen_mode = mode;
	switch (ctrl->lge_extra.screen_mode) {
	case LGE_COLOR_ECO:
	case LGE_COLOR_GAM:
		ctrl->lge_extra.adv_mp_mode = LGE_MP_PSM;
		break;
	case LGE_COLOR_CIN:
	case LGE_COLOR_SPO:
		ctrl->lge_extra.adv_mp_mode = LGE_MP_NOR;
		break;
	case LGE_COLOR_MAN:
		if (ctrl->lge_extra.color_filter)
			ctrl->lge_extra.adv_mp_mode = LGE_MP_HQC;
		else
			ctrl->lge_extra.adv_mp_mode = LGE_MP_NOR;
		break;
	case LGE_COLOR_OPT:
	case LGE_COLOR_MAX:
	default:
		ctrl->lge_extra.adv_mp_mode = LGE_MP_OFF;
		break;
	}

	pr_info("screen_mode : %d, adv_mp_mode : %d\n", ctrl->lge_extra.screen_mode, ctrl->lge_extra.adv_mp_mode);

	mplus_mode_set_sub_sw49410(ctrl);

	return 0;
}

static int rgb_tune_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;

	if (ctrl->lge_extra.cm_preset_step > 4)
        ctrl->lge_extra.cm_preset_step = 4;

	pr_info("preset : %d , red = %d , green = %d , blue = %d \n",
            ctrl->lge_extra.cm_preset_step, ctrl->lge_extra.cm_red_step,
            ctrl->lge_extra.cm_green_step, ctrl->lge_extra.cm_blue_step);

	mplus_mode_set_sub_sw49410(ctrl);
	return 0;
}

static int screen_tune_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;

	pr_info("sat : %d , hue = %d , sha = %d , CF = %d \n",
			ctrl->lge_extra.sc_sat_step, ctrl->lge_extra.sc_hue_step,
			ctrl->lge_extra.sc_sha_step, ctrl->lge_extra.color_filter);

	screen_mode_set_sw49410(ctrl, ctrl->lge_extra.screen_mode);
	return 0;
}

static int sharpness_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;
	ctrl->lge_extra.sharpness = mode;
	sharpness_set_send_sw49410(ctrl);
	return 0;
}

static int hdr_mode_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	struct dsi_panel_cmds *pcmds;

	if (ctrl == NULL)
		return -ENODEV;

	pcmds = lge_get_extra_cmds_by_name(ctrl, "ie-ctrl");
	if (pcmds) {
		if (mode == 0) {
			pcmds->cmds[0].payload[1] = 0x81; /* IE on */
			pcmds->cmds[1].payload[4] = 0x26; /* CABC on */
		} else {
			pcmds->cmds[0].payload[1] = 0x00; /* IE off */
			pcmds->cmds[1].payload[4] = 0x02; /* CABC off */
		}
		pr_info("hdr_mode=%d\n", mode);
		ctrl->lge_extra.hdr_mode = mode;
		mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
	} else {
		pr_err("no cmds: ie-ctrl\n");
	}
	mplus_mode_set_sub_sw49410(ctrl);

	return 0;
}

static int hl_mode_set_sw49410(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;
	ctrl->lge_extra.hl_mode = mode;
	mplus_mode_set_sub_sw49410(ctrl);

	return 0;
}

static int find_cmd_index(struct dsi_panel_cmds *pcmds, int cmd)
{
	int i;
	if (pcmds == NULL)
		return -1;

	for (i = 0; i < pcmds->cmd_cnt; ++i) {
		if (pcmds->cmds[i].payload[0] == cmd)
			return i;
	}
	return -2;
}

#define PARTIAL_AREA_CMD 0x30
#define U2_CTRL_CMD 0xCD
#define ENABLE_SCROLL 0x09
#define DISABLE_SCROLL 0x00
#define SR_UPPER 1
#define SR_LOWER 2
#define ER_UPPER 3
#define ER_LOWER 4
#define U2_CTRL_VSPOSFIX 1
#define U2_CTRL_VSUB_LOWER 13
#define U2_CTRL_VSUB_UPPER 14
static int send_u2_cmds_sw49410(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct dsi_panel_cmds *pcmds;

	pr_info("+\n");
	pcmds = lge_get_extra_cmds_by_name(ctrl, "u2");
	if (pcmds) {
		int sr, er, vsub, h;
		int partial_area_cmd_idx = find_cmd_index(pcmds, PARTIAL_AREA_CMD);
		int u2_ctrl_cmd_idx = find_cmd_index(pcmds, U2_CTRL_CMD);
		if (partial_area_cmd_idx >= 0 && u2_ctrl_cmd_idx >= 0) {
			h = ctrl->lge_extra.aod_area.h;
			h = h?h:ctrl->panel_data.panel_info.yres;
			if (use_u2_vs) {
				sr = 0;
				er = h - 1;
				vsub = ctrl->lge_extra.aod_area.y;
			} else {
				sr = ctrl->lge_extra.aod_area.y;
				er = ctrl->lge_extra.aod_area.y + h - 1;
				vsub = 0;
			}
			pr_info("SR=%d, ER=%d, VSUB=%d\n", sr, er, vsub);
			pcmds->cmds[partial_area_cmd_idx].payload[SR_UPPER] = (sr & 0xff00) >> 8;
			pcmds->cmds[partial_area_cmd_idx].payload[SR_LOWER] = sr & 0xff;
			pcmds->cmds[partial_area_cmd_idx].payload[ER_UPPER] = (er & 0xff00) >> 8;
			pcmds->cmds[partial_area_cmd_idx].payload[ER_LOWER] = er & 0xff;
			pcmds->cmds[u2_ctrl_cmd_idx].payload[U2_CTRL_VSUB_LOWER] = (vsub & 0xf) << 4;
			pcmds->cmds[u2_ctrl_cmd_idx].payload[U2_CTRL_VSUB_UPPER] = (vsub & 0xff0) >> 4;
			pcmds->cmds[u2_ctrl_cmd_idx].payload[U2_CTRL_VSPOSFIX] = use_u2_vs?ENABLE_SCROLL:DISABLE_SCROLL;
		} else {
			pr_err("there is no partial area cmd or u2 ctrl cmd\n");
		}
		mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
	} else {
		pr_err("no cmds: u2\n");
	}
	pr_info("-\n");
	return 0;
}

static struct lge_ddic_ops sw49410_ops = {
	.op_get_blmap_type = get_blmap_type_sw49410,
	.op_mplus_change_blmap = mplus_change_blmap_sw49410,

	.op_image_enhance_set = image_enhance_set_sw49410,
	.op_image_enhance_get = image_enhance_get_sw49410,
	.op_mplus_mode_set = mplus_mode_set_sw49410,
	.op_mplus_mode_get = mplus_mode_get_sw49410,
	.op_mplus_hd_set = mplus_hd_set_sw49410,
	.op_mplus_hd_get = mplus_hd_get_sw49410,
	.op_mplus_max_set = mplus_max_set_sw49410,
	.op_mplus_max_get = mplus_max_get_sw49410,

	.op_screen_mode_set = screen_mode_set_sw49410,
	.op_rgb_tune_set = rgb_tune_set_sw49410,
	.op_screen_tune_set  = screen_tune_set_sw49410,
	.op_sharpness_set  = sharpness_set_sw49410,
	.op_hdr_mode_set  = hdr_mode_set_sw49410,
	.op_hl_mode_set  = hl_mode_set_sw49410,

	.op_send_u2_cmds = send_u2_cmds_sw49410,
};

struct lge_ddic_ops *get_ddic_ops_sw49410(void)
{
	return &sw49410_ops;
}
