#define pr_fmt(fmt)	"[Display] %s: " fmt, __func__

#include <linux/delay.h>
#include "../mdss_dsi.h"

#define NUM_SHA_CTRL 		5
#define NUM_SAT_CTRL 		6
#define OFFSET_SAT_CTRL 	6
#define NUM_HUE_CTRL 		5
#define OFFSET_HUE_CTRL 	2

#define NUM_SC_CTRL 		4
#define NUM_DG_PRESET       16
#define STEP_GC_PRESET      4

#define LGE_SAT_GAM_MODE	5

enum {
	PRESET_SETP0_OFFSET = 0,  //40
	PRESET_SETP1_OFFSET = 1,  //3F
	PRESET_SETP2_OFFSET = 2,  //3E
	PRESET_SETP3_OFFSET = 5,  //3B
	PRESET_SETP4_OFFSET = 7,  //39
	PRESET_SETP5_OFFSET = 8,  //38
	PRESET_SETP6_OFFSET = 11  //35

};

static char sha_ctrl_values[NUM_SHA_CTRL] = {0x04, 0x0D, 0x1A, 0x30, 0xD2};  //sharpness

static char sat_ctrl_values[NUM_SAT_CTRL][OFFSET_SAT_CTRL] = {               //saturation
	{0x00, 0x37, 0x6E, 0xA5, 0xDB, 0x00},
	{0x00, 0x3B, 0x77, 0xB2, 0xED, 0x00},
	{0x00, 0x40, 0x80, 0xC0, 0x00, 0x01},
	{0x00, 0x41, 0x83, 0xC5, 0x00, 0x01},
	{0x00, 0x43, 0x87, 0xCB, 0x00, 0x01},
	{0x00, 0x43, 0x87, 0xCB, 0x00, 0x01}  //GAME
};

static char hue_ctrl_values[NUM_HUE_CTRL][OFFSET_HUE_CTRL] = {               //hue
	{0xF7, 0x00},
	{0xF4, 0x00},
	{0xF0, 0x00},
	{0x74, 0x00},
	{0x77, 0x00}
};

static int rgb_preset[STEP_DG_PRESET][RGB_ALL] = {                           //temperature
	{PRESET_SETP3_OFFSET, PRESET_SETP2_OFFSET, PRESET_SETP6_OFFSET},
	{PRESET_SETP1_OFFSET, PRESET_SETP0_OFFSET, PRESET_SETP5_OFFSET},
	{PRESET_SETP0_OFFSET, PRESET_SETP0_OFFSET, PRESET_SETP0_OFFSET},
	{PRESET_SETP0_OFFSET, PRESET_SETP3_OFFSET, PRESET_SETP4_OFFSET},
	{PRESET_SETP0_OFFSET, PRESET_SETP4_OFFSET, PRESET_SETP4_OFFSET}
};

static int gc_preset[STEP_GC_PRESET][RGB_ALL] = {                            //screen mode
	{0x00, 0x00, 0x00},  //NOR
	{0x00, 0x02, 0x04},  //CINEMA
	{0x02, 0x02, 0x00},  //SPORT
	{0x01, 0x01, 0x00},  //GAME
};

static char dg_ctrl_values[NUM_DG_PRESET][OFFSET_DG_CTRL] = {                //temperature value
	{0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
	{0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F},
	{0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E},
	{0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D, 0x3D},
	{0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C},

	{0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B},
	{0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A},
	{0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39},
	{0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38},
	{0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37},

	{0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36},
	{0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35},
	{0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34},
	{0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
	{0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32},

	{0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31}
};

extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_panel_cmds *pcmds, u32 flags);

static void lge_set_rgb_tune_send_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, enum lge_gamma_correction_mode gc_mode)
{
	int i = 0;
	int red_index = 0, green_index = 0, blue_index = 0, dg_status = 0;
	char *payload_ctrl[3] = {NULL, };

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
	}
	ctrl->lge_extra.dg_control = dg_status;
	mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);

	pr_info("[0x%02x:0x%02x][0x%02x:0x%02x][0x%02x:0x%02x]\n",
			payload_ctrl[RED][0], payload_ctrl[RED][1],
			payload_ctrl[GREEN][0], payload_ctrl[GREEN][1],
			payload_ctrl[BLUE][0], payload_ctrl[BLUE][1]);

	return;
}

static void lge_set_screen_tune_send_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int lge_screen_tune_status)
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

		payload_ctrl[0][3] = sha_ctrl_values[ctrl->lge_extra.sc_sha_step];             //sharpness
		ctrl->lge_extra.sharpness_control = true;

		for (i = 0; i < OFFSET_SAT_CTRL; i++)
			payload_ctrl[1][i+1] = sat_ctrl_values[ctrl->lge_extra.sc_sat_step][i];    //saturation
		for (i = 0; i < OFFSET_HUE_CTRL; i++)
			payload_ctrl[1][i+7] = hue_ctrl_values[ctrl->lge_extra.sc_hue_step][i];    //hue
	} else {
		if (ctrl->lge_extra.sharpness != 0x04) {
			payload_ctrl[0][3] = ctrl->lge_extra.sharpness;
		}
		ctrl->lge_extra.sc_sat_step = SC_MODE_DEFAULT;
		ctrl->lge_extra.sc_hue_step = SC_MODE_DEFAULT;
		ctrl->lge_extra.sc_sha_step = SHARP_DEFAULT;

		payload_ctrl[0][3] = sha_ctrl_values[ctrl->lge_extra.sc_sha_step];             //sharpness

		for (i = 0; i < OFFSET_SAT_CTRL; i++)
			payload_ctrl[1][i+1] = sat_ctrl_values[ctrl->lge_extra.sc_sat_step][i];    //saturation
		for (i = 0; i < OFFSET_HUE_CTRL; i++)
			payload_ctrl[1][i+7] = hue_ctrl_values[ctrl->lge_extra.sc_hue_step][i];    //hue

		if (lge_screen_tune_status == LGE_SCREEN_TUNE_GAM) {
			ctrl->lge_extra.sc_sat_step = LGE_SAT_GAM_MODE;
			for (i = 0; i < OFFSET_SAT_CTRL; i++)
				payload_ctrl[1][i+1] = sat_ctrl_values[ctrl->lge_extra.sc_sat_step][i];
		}
		ctrl->lge_extra.sharpness_control = false;
	}
	mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
	pr_info("sat : [0x%02x:0x%02x:0x%02x]  hue: [0x%02x] sharp : [0x%02x]\n",
			payload_ctrl[1][1], payload_ctrl[1][2], payload_ctrl[1][3],
			payload_ctrl[1][7], payload_ctrl[0][3]);
	return;
}


void sharpness_set_send_sw49408(struct mdss_dsi_ctrl_pdata *ctrl)
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
		pcmds->cmds[0].payload[3] = mode;
		pr_info("sharpness=%d\n", mode);
		mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
	} else {
		pr_err("no cmds: sharpness\n");
	}
}

void lge_set_image_quality_cmds(struct mdss_dsi_ctrl_pdata *ctrl)
{
	char mask = 0x00;
	struct dsi_panel_cmds *ie_pcmds;			//55h
	struct dsi_panel_cmds *mie_pcmds;			//f0h
	struct dsi_panel_cmds *cabc_pcmds;			//fbh

	ie_pcmds = lge_get_extra_cmds_by_name(ctrl, "ie-ctrl");
	if (!ie_pcmds)
	{
		pr_err("no cmds: ie-ctrl\n");
		return;
	}

	mie_pcmds = lge_get_extra_cmds_by_name(ctrl, "mie-ctrl");
	if (!mie_pcmds)
	{
		pr_err("no cmds: mie-ctrl\n");
		return;
	}

	cabc_pcmds = lge_get_extra_cmds_by_name(ctrl, "cabc-on-off");
	if (!cabc_pcmds)
	{
		pr_err("no cmds: cabc-on-off\n");
		return;
	}

	if(ctrl->lge_extra.sre_mode > 0) {
		if(ctrl->lge_extra.sre_mode == SRE_LOW) {
			ie_pcmds->cmds[0].payload[1] |= SRE_MASK_LOW;
			pr_info("%s: SRE LOW\n",__func__);
		}else if(ctrl->lge_extra.sre_mode == SRE_MID) {
			ie_pcmds->cmds[0].payload[1] |= SRE_MASK_MID;
			pr_info("%s: SRE MID\n",__func__);
		} else if(ctrl->lge_extra.sre_mode == SRE_HIGH) {
			ie_pcmds->cmds[0].payload[1] |= SRE_MASK_HIGH;
			pr_info("%s: SRE HIGH\n",__func__);
		} else {
			mask = SRE_MASK;
			ie_pcmds->cmds[0].payload[1] &= (~mask);
		}
	}

#if defined(CONFIG_LGE_LCD_DYNAMIC_CABC_MIE_CTRL)
	if (ctrl->lge_extra.ie_control == 0) {
		pr_info("%s: IE OFF\n",__func__);
		mask = IE_MASK;
		ie_pcmds->cmds[0].payload[1] &= (~mask);
		goto send;
	}
#endif
#if defined(CONFIG_LGE_DISPLAY_DOLBY_MODE)
	if(ctrl->lge_extra.dolby_mode > 0) {
		pr_info("%s: Dolby Mode ON\n", __func__);
		/* Dolby Setting : CABC OFF, SRE_OFF, IE OFF */
		mask = (IE_MASK | CABC_MASK | SRE_MASK);
		ie_pcmds->cmds[0].payload[1] &= (~mask);
		cabc_pcmds->cmds[0].payload[4] = CABC_OFF_VALUE;
		pr_info("%s: Dolby=%d 55h = 0x%02x, fbh = 0x%02x\n",__func__,ctrl->lge_extra.dolby_mode,
				ie_pcmds->cmds[0].payload[1],cabc_pcmds->cmds[0].payload[4]);
		goto send;
	}
#endif
#if defined(CONFIG_LGE_DISPLAY_HDR_MODE)
	if(ctrl->lge_extra.hdr_mode > 0) {
		pr_info("%s: HDR Mode ON\n", __func__);
		/* HDR Setting : CABC OFF, SRE OFF, IE OFF */
		mask = (IE_MASK | CABC_MASK | SRE_MASK);
		ie_pcmds->cmds[0].payload[1] &= (~mask);
		cabc_pcmds->cmds[0].payload[4] = CABC_OFF_VALUE;
		pr_info("%s: HDR=%d 55h = 0x%02x, fbh = 0x%02x\n",__func__,ctrl->lge_extra.hdr_mode,
				ie_pcmds->cmds[0].payload[1],cabc_pcmds->cmds[0].payload[4]);
		goto send;
	}
#endif

	//start mie_pcmds(F0h) setting
	mask = (SH_MASK | SAT_MASK | HUE_MASK | DG_MASK);
	mie_pcmds->cmds[0].payload[1] &= (~mask);  //F0h

	if(ctrl->lge_extra.sharpness_control == true)
		mie_pcmds->cmds[0].payload[1] |= SH_MASK;

	if(ctrl->lge_extra.dg_control == DG_ON)
		mie_pcmds->cmds[0].payload[1] |= DG_MASK;

    if(ctrl->lge_extra.screen_tune_status == LGE_SCREEN_TUNE_GAM)
		mie_pcmds->cmds[0].payload[1] |= SAT_MASK;

    if(ctrl->lge_extra.screen_tune_status == LGE_SCREEN_TUNE_ON)
		mie_pcmds->cmds[0].payload[1] |= SH_MASK | SAT_MASK | HUE_MASK;
	//end mie_pcmds(F0h) setting

send:
    mdss_dsi_panel_cmds_send(ctrl, ie_pcmds, CMD_REQ_COMMIT);
	mdss_dsi_panel_cmds_send(ctrl, mie_pcmds, CMD_REQ_COMMIT);
	mdss_dsi_panel_cmds_send(ctrl, cabc_pcmds, CMD_REQ_COMMIT);
	pr_info("%s : 55h:0x%02x, f0h:0x%02x, fbh(CABC):0x%02x \n",__func__,
			ie_pcmds->cmds[0].payload[1],  mie_pcmds->cmds[0].payload[1],
			cabc_pcmds->cmds[0].payload[4]);
}

void dic_lcd_mode_set(struct mdss_dsi_ctrl_pdata *ctrl)
{
	enum lge_gamma_correction_mode gc_mode = LGE_GC_MOD_NOR;
	int lge_screen_tune_status = LGE_SCREEN_TUNE_OFF;

	if (ctrl == NULL) {
		pr_err("%pS: ctrl == NULL\n", __builtin_return_address(0));
		return;
	}

	if (mdss_dsi_is_panel_off(&ctrl->panel_data) || mdss_dsi_is_panel_on_lp(&ctrl->panel_data)) {
		pr_info("unexpected panel power state while setting lcd mode\n");
		return;
	}

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
		break;
	case LGE_COLOR_OPT:
		break;
	default:
		break;
    }
	ctrl->lge_extra.screen_tune_status = lge_screen_tune_status;

	pr_info("%s gc_mode=[%d] \n", __func__, gc_mode);

	lge_set_rgb_tune_send_sw49408(ctrl, gc_mode);
	lge_set_screen_tune_send_sw49408(ctrl, lge_screen_tune_status);
	sharpness_set_send_sw49408(ctrl);
	lge_set_image_quality_cmds(ctrl);
}

static int image_enhance_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	struct dsi_panel_cmds *ie_pcmds;

	if (ctrl == NULL)
		return -ENODEV;

	ie_pcmds = lge_get_extra_cmds_by_name(ctrl, "ie-ctrl");
	if (!ie_pcmds)
	{
		pr_err("no cmds: ie-ctrl\n");
		return -EINVAL;
	}

	ctrl->lge_extra.ie_control = mode;
	pr_info("ie_control : %d\n", mode);

	if (ctrl->lge_extra.ie_control == 1) {
		ie_pcmds->cmds[0].payload[1] |= IE_MASK;
	} else if (ctrl->lge_extra.ie_control == 0) {
		char mask = IE_MASK;
		ie_pcmds->cmds[0].payload[1] &= (~mask);
	} else {
		pr_info("%s: set = %d, wrong set value\n", __func__, ctrl->lge_extra.ie_control);
	}

	mdss_dsi_panel_cmds_send(ctrl, ie_pcmds, CMD_REQ_COMMIT);
	pr_info("%s : 55h:0x%02x\n",__func__,ie_pcmds->cmds[0].payload[1]);
	return 0;
}

static int image_enhance_get_sw49408(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;
	return ctrl->lge_extra.ie_control;
}

static int screen_mode_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;

	ctrl->lge_extra.screen_mode = mode;
	switch (ctrl->lge_extra.screen_mode) {
	case LGE_COLOR_GAM:
	case LGE_COLOR_CIN:
	case LGE_COLOR_SPO:
	case LGE_COLOR_MAN:
	case LGE_COLOR_OPT:
	case LGE_COLOR_MAX:
	default:
		break;
	}

	pr_info("screen_mode : %d\n", ctrl->lge_extra.screen_mode);

	dic_lcd_mode_set(ctrl);

	return 0;
}

static int rgb_tune_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;

	if (ctrl->lge_extra.cm_preset_step > 4)
        ctrl->lge_extra.cm_preset_step = 4;

	pr_info("preset : %d , red = %d , green = %d , blue = %d \n",
            ctrl->lge_extra.cm_preset_step, ctrl->lge_extra.cm_red_step,
            ctrl->lge_extra.cm_green_step, ctrl->lge_extra.cm_blue_step);

	dic_lcd_mode_set(ctrl);

	return 0;
}

static int screen_tune_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl == NULL)
		return -ENODEV;

	pr_info("sat : %d , hue = %d , sha = %d \n",
			ctrl->lge_extra.sc_sat_step, ctrl->lge_extra.sc_hue_step,
			ctrl->lge_extra.sc_sha_step);

	screen_mode_set_sw49408(ctrl, ctrl->lge_extra.screen_mode);
	return 0;
}

static int sharpness_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	if (ctrl == NULL)
		return -ENODEV;

	ctrl->lge_extra.sharpness = mode;
	sharpness_set_send_sw49408(ctrl);
	return 0;
}


static int sre_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	char mask = SRE_MASK;
	struct dsi_panel_cmds *ie_pcmds;            //55h

	if (ctrl == NULL)
		return -ENODEV;

	ie_pcmds = lge_get_extra_cmds_by_name(ctrl, "ie-ctrl");
	if (!ie_pcmds)
	{
		pr_err("no cmds: ie-ctrl\n");
		return -EINVAL;
	}

	pr_info("sre_mode=%d\n", mode);
	ctrl->lge_extra.sre_mode = mode;

	if(ctrl->lge_extra.hdr_mode > 0 || ctrl->lge_extra.dolby_mode > 0) {
		pr_info("%s : HDR or Dolby on, so disable SRE \n", __func__);
		return -EINVAL;
	}

	ie_pcmds->cmds[0].payload[1] &= (~mask);
	if (mode == 0) {
		ctrl->lge_extra.sre_mode = 0;
		pr_info("%s : SRE OFF \n",__func__);
	} else {
		if (mode == SRE_LOW) {
			ctrl->lge_extra.sre_mode = SRE_LOW;
			pr_info("%s : SRE LOW \n",__func__);
			ie_pcmds->cmds[0].payload[1] |= SRE_MASK_LOW;
		} else if (mode == SRE_MID) {
			ctrl->lge_extra.sre_mode = SRE_MID;
			pr_info("%s : SRE MID \n",__func__);
			ie_pcmds->cmds[0].payload[1] |= SRE_MASK_MID;
		} else if (mode == SRE_HIGH) {
			ctrl->lge_extra.sre_mode = SRE_HIGH;
			pr_info("%s : SRE HIGH \n",__func__);
			ie_pcmds->cmds[0].payload[1] |= SRE_MASK_HIGH;
		} else {
			return -EINVAL;
		}
	}
	mdss_dsi_panel_cmds_send(ctrl, ie_pcmds, CMD_REQ_COMMIT);
	pr_info("%s : 55h:0x%02x\n",__func__, ie_pcmds->cmds[0].payload[1]);

	return 0;
}

static int dolby_mode_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	char mask = 0x00;
	struct dsi_panel_cmds *ie_pcmds;            //55h
	struct dsi_panel_cmds *cabc_pcmds;			//fbh

	if (ctrl == NULL)
		return -ENODEV;

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

	pr_info("dolby_mode=%d\n", mode);
	ctrl->lge_extra.dolby_mode = mode;

	if (mode == 0) {
		pr_info("%s: Dolby Mode OFF\n", __func__);
		/* Retore 55h, FBh Reg */
		mask = (IE_MASK | CABC_MASK);
		ie_pcmds->cmds[0].payload[1] |= mask;
		cabc_pcmds->cmds[0].payload[4] = CABC_ON_VALUE;

		if(ctrl->lge_extra.sre_mode > 0) {
			if(ctrl->lge_extra.sre_mode == SRE_LOW) {
				ie_pcmds->cmds[0].payload[1] |= SRE_MASK_LOW;
				pr_info("%s: SRE LOW\n",__func__);
			}else if(ctrl->lge_extra.sre_mode == SRE_MID) {
				ie_pcmds->cmds[0].payload[1] |= SRE_MASK_MID;
				pr_info("%s: SRE MID\n",__func__);
			} else if(ctrl->lge_extra.sre_mode == SRE_HIGH) {
				ie_pcmds->cmds[0].payload[1] |= SRE_MASK_HIGH;
				pr_info("%s: SRE HIGH\n",__func__);
			} else {
				mask = SRE_MASK;
				ie_pcmds->cmds[0].payload[1] &= (~mask);
			}
		}
	} else {
		pr_info("%s: Dolby Mode ON\n", __func__);
		/* Dolby Setting : IE_OFF, CABC OFF, SRE OFF*/
		mask = (IE_MASK | CABC_MASK | SRE_MASK);
		ie_pcmds->cmds[0].payload[1] &= (~mask);
		cabc_pcmds->cmds[0].payload[4] = CABC_OFF_VALUE;
	}
	mdss_dsi_panel_cmds_send(ctrl, ie_pcmds, CMD_REQ_COMMIT);
	mdss_dsi_panel_cmds_send(ctrl, cabc_pcmds, CMD_REQ_COMMIT);

	pr_info("%s : 55h:0x%02x, fbh(CABC):0x%02x \n",__func__,
			ie_pcmds->cmds[0].payload[1], cabc_pcmds->cmds[0].payload[4]);
	return 0;
}

static int hdr_mode_set_sw49408(struct mdss_dsi_ctrl_pdata *ctrl, int mode)
{
	char mask = 0x00;
	struct dsi_panel_cmds *ie_pcmds;            //55h
	struct dsi_panel_cmds *cabc_pcmds;			//fbh

	if (ctrl == NULL)
		return -ENODEV;

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

	pr_info("hdr_mode=%d\n", mode);
	ctrl->lge_extra.hdr_mode = mode;

	if (mode == 0) {
		pr_info("%s: HDR Mode OFF\n", __func__);
		/* Retore 55h & FBh Reg */
		mask = (IE_MASK | CABC_MASK);
		ie_pcmds->cmds[0].payload[1] |= mask;
		cabc_pcmds->cmds[0].payload[4] = CABC_ON_VALUE;

		if(ctrl->lge_extra.sre_mode > 0) {
			if(ctrl->lge_extra.sre_mode == SRE_LOW) {
				ie_pcmds->cmds[0].payload[1] |= SRE_MASK_LOW;
				pr_info("%s: SRE LOW\n",__func__);
			}else if(ctrl->lge_extra.sre_mode == SRE_MID) {
				ie_pcmds->cmds[0].payload[1] |= SRE_MASK_MID;
				pr_info("%s: SRE MID\n",__func__);
			} else if(ctrl->lge_extra.sre_mode == SRE_HIGH) {
				ie_pcmds->cmds[0].payload[1] |= SRE_MASK_HIGH;
				pr_info("%s: SRE HIGH\n",__func__);
			} else {
				mask = SRE_MASK;
				ie_pcmds->cmds[0].payload[1] &= (~mask);
			}
		}
	} else {
		pr_info("%s: HDR Mode ON\n", __func__);
		/* Dolby Setting : CABC OFF, SRE OFF, IE OFF */
		mask = (IE_MASK | CABC_MASK | SRE_MASK);
		ie_pcmds->cmds[0].payload[1] &= (~mask);
		cabc_pcmds->cmds[0].payload[4] = CABC_OFF_VALUE;
	}
	mdss_dsi_panel_cmds_send(ctrl, ie_pcmds, CMD_REQ_COMMIT);
	mdss_dsi_panel_cmds_send(ctrl, cabc_pcmds, CMD_REQ_COMMIT);

	pr_info("%s : 55h:0x%02x, fbh(CABC):0x%02x \n",__func__,
			ie_pcmds->cmds[0].payload[1], cabc_pcmds->cmds[0].payload[4]);

	return 0;
}

static struct lge_ddic_ops sw49408_ops = {
	.op_image_enhance_set = image_enhance_set_sw49408,
	.op_image_enhance_get = image_enhance_get_sw49408,

	.op_screen_mode_set = screen_mode_set_sw49408,
	.op_rgb_tune_set = rgb_tune_set_sw49408,
	.op_screen_tune_set  = screen_tune_set_sw49408,
	.op_sharpness_set  = sharpness_set_sw49408,
	.op_sre_set   = sre_set_sw49408,
	.op_dolby_mode_set = dolby_mode_set_sw49408,
	.op_hdr_mode_set  = hdr_mode_set_sw49408,
};

struct lge_ddic_ops *get_ddic_ops_sw49408(void)
{
	return &sw49408_ops;
}
