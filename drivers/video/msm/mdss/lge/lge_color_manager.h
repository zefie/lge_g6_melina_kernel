/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
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
#ifndef LGE_COLOR_MANAGER_H
#define LGE_COLOR_MANAGER_H

#define HDR_OFF				0
#define HDR_ON 				1

#define DOLBY_OFF			0
#define DOLBY_ON			1

#define SHA_OFF				0
#define SHA_ON				1

#define HL_MODE_OFF 		0
#define HL_MODE_ON 			1

#define IE_OFF				0
#define IE_ON				1

#define SRE_OFF				0
#define SRE_ON				1

#define SC_MODE_DEFAULT		2
#define SHARP_DEFAULT		0
#define SC_MODE_MAX			4

#define RGB_DEFAULT_PRESET	2
#define RGB_DEFAULT_RED		4
#define RGB_DEFAULT_BLUE	4
#define RGB_DEFAULT_GREEN	4

#define DG_MODE_MAX			4
#define DG_OFF				0
#define DG_ON				1
#define STEP_DG_PRESET		5
#define OFFSET_DG_CTRL		8
#define NUM_DG_CTRL			0x10

#define LGE_SCREEN_TUNE_OFF    0
#define LGE_SCREEN_TUNE_ON     1
#define LGE_SCREEN_TUNE_GAM    2
#define LGE_SCREEN_TUNE_GAL    3
#define LGE_SAT_GAL_MODE       5

enum {
	RED      = 0,
	GREEN    = 1,
	BLUE     = 2,
	RGB_ALL  = 3
};

enum {
	PRESET_SETP0_INDEX = 0,
	PRESET_SETP1_INDEX = 6,
	PRESET_SETP2_INDEX = 12,
};

#endif /* LGE_COLOR_MANAGER_H */
