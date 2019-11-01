#ifndef _LGE_MPLUS_H_
#define _LGE_MPLUS_H_

#define LGE_MPLUS_BR_DIM_CNT 10
#define LGE_MPLUS_BR_DIM_DELAY 10

enum lge_mplus_mode {
	LGE_MP_NOR = 0,
	LGE_MP_PSM,
	LGE_MP_HBM,
	LGE_MP_GAL,
	LGE_MP_BRI, /* not used */
	LGE_MP_PS2, /* not used */
	LGE_MP_HQC,
	LGE_MP_FHB,
	LGE_MP_OFF,
	LGE_MP_MAX,
};

enum lge_dic_mplus_mode {
	LGE_DIC_MP_HBM = 0,
	LGE_DIC_MP_NOR,
	LGE_DIC_MP_PSM,
	LGE_DIC_MP_GAL,
	LGE_DIC_MP_MAX,
};

enum lge_dic_mplus_mode_set {
	LGE_MODE_SET_1ST = 1,
	LGE_MODE_SET_2ND,
	LGE_MODE_SET_3RD,
	LGE_MODE_SET_MAX,
};

enum lge_screen_color_mplus_mode {
	LGE_COLOR_OPT = 0,
	LGE_COLOR_ECO,
	LGE_COLOR_CIN,
	LGE_COLOR_SPO,
	LGE_COLOR_GAM,
	LGE_COLOR_MAN = 10,
	LGE_COLOR_MAX,
};

enum lge_gamma_correction_mode {
	LGE_GC_MOD_NOR = 0,
	LGE_GC_MOD_CIN,
	LGE_GC_MOD_SPO,
	LGE_GC_MOD_GAM,
	LGE_GC_MOD_HQC,
	LGE_GC_MOD_MAX,
};

#endif // _LGE_MPLUS_H_
