#include <linux/debugfs.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "mdss.h"
#include "mdss_mdp.h"
#include "mdss_mdp_hwio.h"
#include "mdss_dsi.h"
#include "mdss_misr.h"

int vsync_count;
static struct mdss_mdp_misr_map {
	u32 ctrl_reg;
	u32 value_reg;
	u32 crc_op_mode;
	u32 crc_index;
	u32 last_misr;
	bool use_ping;
	bool is_ping_full;
	bool is_pong_full;
	struct mutex crc_lock;
	u32 crc_ping[MISR_CRC_BATCH_SIZE];
	u32 crc_pong[MISR_CRC_BATCH_SIZE];
} mdss_mdp_misr_table[DISPLAY_MISR_MAX] = {
	[DISPLAY_MISR_DSI0] = {
		.ctrl_reg = MDSS_MDP_LP_MISR_CTRL_DSI0,
		.value_reg = MDSS_MDP_LP_MISR_SIGN_DSI0,
		.crc_op_mode = 0,
		.crc_index = 0,
		.last_misr = 0,
		.use_ping = true,
		.is_ping_full = false,
		.is_pong_full = false,
	},
	[DISPLAY_MISR_DSI1] = {
		.ctrl_reg = MDSS_MDP_LP_MISR_CTRL_DSI1,
		.value_reg = MDSS_MDP_LP_MISR_SIGN_DSI1,
		.crc_op_mode = 0,
		.crc_index = 0,
		.last_misr = 0,
		.use_ping = true,
		.is_ping_full = false,
		.is_pong_full = false,
	},
	[DISPLAY_MISR_EDP] = {
		.ctrl_reg = MDSS_MDP_LP_MISR_CTRL_EDP,
		.value_reg = MDSS_MDP_LP_MISR_SIGN_EDP,
		.crc_op_mode = 0,
		.crc_index = 0,
		.last_misr = 0,
		.use_ping = true,
		.is_ping_full = false,
		.is_pong_full = false,
	},
	[DISPLAY_MISR_HDMI] = {
		.ctrl_reg = MDSS_MDP_LP_MISR_CTRL_HDMI,
		.value_reg = MDSS_MDP_LP_MISR_SIGN_HDMI,
		.crc_op_mode = 0,
		.crc_index = 0,
		.last_misr = 0,
		.use_ping = true,
		.is_ping_full = false,
		.is_pong_full = false,
	},
	[DISPLAY_MISR_MDP] = {
		.ctrl_reg = MDSS_MDP_LP_MISR_CTRL_MDP,
		.value_reg = MDSS_MDP_LP_MISR_SIGN_MDP,
		.crc_op_mode = 0,
		.crc_index = 0,
		.last_misr = 0,
		.use_ping = true,
		.is_ping_full = false,
		.is_pong_full = false,
	},
};

static inline struct mdss_mdp_misr_map *mdss_misr_get_map(u32 block_id,
		struct mdss_mdp_ctl *ctl, struct mdss_data_type *mdata,
		bool is_video_mode)
{
	struct mdss_mdp_misr_map *map;
	struct mdss_mdp_mixer *mixer;
	char *ctrl_reg = NULL, *value_reg = NULL;
	char *intf_base = NULL;

	if (block_id > DISPLAY_MISR_HDMI && block_id != DISPLAY_MISR_MDP) {
		pr_err("MISR Block id (%d) out of range\n", block_id);
		return NULL;
	}

	if (mdata->mdp_rev >= MDSS_MDP_HW_REV_105) {
		/* Use updated MDP Interface MISR Block address offset */
		if (block_id == DISPLAY_MISR_MDP) {
			if (ctl) {
				mixer = mdss_mdp_mixer_get(ctl,
					MDSS_MDP_MIXER_MUX_DEFAULT);

				if (mixer) {
					ctrl_reg = mixer->base +
						MDSS_MDP_LAYER_MIXER_MISR_CTRL;
					value_reg = mixer->base +
					MDSS_MDP_LAYER_MIXER_MISR_SIGNATURE;
				}
			}
		} else {
			if (block_id <= DISPLAY_MISR_HDMI) {
				intf_base = (char *)mdss_mdp_get_intf_base_addr(
						mdata, block_id);

				if ((block_id == DISPLAY_MISR_DSI0 ||
				     block_id == DISPLAY_MISR_DSI1) &&
				     !is_video_mode) {
					ctrl_reg = intf_base +
						MDSS_MDP_INTF_CMD_MISR_CTRL;
					value_reg = intf_base +
					    MDSS_MDP_INTF_CMD_MISR_SIGNATURE;

					/*
					 * extra offset required for
					 * cmd misr in 8996
					 */
					if (IS_MDSS_MAJOR_MINOR_SAME(
						  mdata->mdp_rev,
						  MDSS_MDP_HW_REV_107)) {
						ctrl_reg += 0x8;
						value_reg += 0x8;
					}

				} else {
					ctrl_reg = intf_base +
						MDSS_MDP_INTF_MISR_CTRL;
					value_reg = intf_base +
						MDSS_MDP_INTF_MISR_SIGNATURE;
				}
			}
			/*
			 * For msm8916/8939, additional offset of 0x10
			 * is required
			 */
			if ((mdata->mdp_rev == MDSS_MDP_HW_REV_106) ||
				(mdata->mdp_rev == MDSS_MDP_HW_REV_108) ||
				(mdata->mdp_rev == MDSS_MDP_HW_REV_112)) {
				ctrl_reg += 0x10;
				value_reg += 0x10;
			}
		}
		mdss_mdp_misr_table[block_id].ctrl_reg = (u32)(ctrl_reg -
							mdata->mdp_base);
		mdss_mdp_misr_table[block_id].value_reg = (u32)(value_reg -
							mdata->mdp_base);
	}

	map = mdss_mdp_misr_table + block_id;
	if ((map->ctrl_reg == 0) || (map->value_reg == 0)) {
		pr_err("MISR Block id (%d) config not found\n", block_id);
		return NULL;
	}

	pr_debug("MISR Module(%d) CTRL(0x%x) SIG(0x%x) intf_base(0x%pK)\n",
			block_id, map->ctrl_reg, map->value_reg, intf_base);
	return map;
}

/*
 * switch_mdp_misr_offset() - Update MDP MISR register offset for MDSS
 * Hardware Revision 103.
 * @map: mdss_mdp_misr_map
 * @mdp_rev: MDSS Hardware Revision
 * @block_id: Logical MISR Block ID
 *
 * Return: true when MDSS Revision is 103 else false.
 */
static bool switch_mdp_misr_offset(struct mdss_mdp_misr_map *map, u32 mdp_rev,
					u32 block_id)
{
	bool use_mdp_up_misr = false;

	if ((IS_MDSS_MAJOR_MINOR_SAME(mdp_rev, MDSS_MDP_HW_REV_103)) &&
		(block_id == DISPLAY_MISR_MDP)) {
		/* Use Upper pipe MISR for Layer Mixer CRC */
		map->ctrl_reg = MDSS_MDP_UP_MISR_CTRL_MDP;
		map->value_reg = MDSS_MDP_UP_MISR_SIGN_MDP;
		use_mdp_up_misr = true;
	}
	pr_debug("MISR Module(%d) Offset of MISR_CTRL = 0x%x MISR_SIG = 0x%x\n",
			block_id, map->ctrl_reg, map->value_reg);
	return use_mdp_up_misr;
}

void mdss_misr_disable(struct mdss_data_type *mdata,
			struct mdp_misr *req,
			struct mdss_mdp_ctl *ctl)
{
	struct mdss_mdp_misr_map *map;

	map = mdss_misr_get_map(req->block_id, ctl, mdata,
		ctl->is_video_mode);

	if (!map)
		return;

	/* clear the map data */
	memset(map->crc_ping, 0, sizeof(map->crc_ping));
	memset(map->crc_pong, 0, sizeof(map->crc_pong));
	map->crc_index = 0;
	map->use_ping = true;
	map->is_ping_full = false;
	map->is_pong_full = false;
	map->crc_op_mode = 0;
	map->last_misr = 0;

	/* disable MISR and clear the status */
	writel_relaxed(MDSS_MDP_MISR_CTRL_STATUS_CLEAR,
			mdata->mdp_base + map->ctrl_reg);

	/* make sure status is clear */
	wmb();
}

int mdss_misr_set(struct mdss_data_type *mdata,
			struct mdp_misr *req,
			struct mdss_mdp_ctl *ctl)
{
	struct mdss_mdp_misr_map *map;
	struct mdss_mdp_mixer *mixer;
	u32 config = 0, val = 0;
	u32 mixer_num = 0;
	bool is_valid_wb_mixer = true;
	bool use_mdp_up_misr = false;

	if (!mdata || !req || !ctl) {
		pr_err("Invalid input params: mdata = %pK req = %pK ctl = %pK",
			mdata, req, ctl);
		return -EINVAL;
	}
	pr_debug("req[block:%d frame:%d op_mode:%d]\n",
		req->block_id, req->frame_count, req->crc_op_mode);

	map = mdss_misr_get_map(req->block_id, ctl, mdata,
		ctl->is_video_mode);
	if (!map) {
		pr_err("Invalid MISR Block=%d\n", req->block_id);
		return -EINVAL;
	}
	use_mdp_up_misr = switch_mdp_misr_offset(map, mdata->mdp_rev,
				req->block_id);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
	if (req->block_id == DISPLAY_MISR_MDP) {
		mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_DEFAULT);
		if (!mixer) {
			pr_err("failed to get default mixer, Block=%d\n",
				req->block_id);
			return -EINVAL;
		}
		mixer_num = mixer->num;
		pr_debug("SET MDP MISR BLK to MDSS_MDP_LP_MISR_SEL_LMIX%d_GC\n",
			mixer_num);
		switch (mixer_num) {
		case MDSS_MDP_INTF_LAYERMIXER0:
			pr_debug("Use Layer Mixer 0 for WB CRC\n");
			val = MDSS_MDP_LP_MISR_SEL_LMIX0_GC;
			break;
		case MDSS_MDP_INTF_LAYERMIXER1:
			pr_debug("Use Layer Mixer 1 for WB CRC\n");
			val = MDSS_MDP_LP_MISR_SEL_LMIX1_GC;
			break;
		case MDSS_MDP_INTF_LAYERMIXER2:
			pr_debug("Use Layer Mixer 2 for WB CRC\n");
			val = MDSS_MDP_LP_MISR_SEL_LMIX2_GC;
			break;
		default:
			pr_err("Invalid Layer Mixer %d selected for WB CRC\n",
				mixer_num);
			is_valid_wb_mixer = false;
			break;
		}
		if ((is_valid_wb_mixer) &&
			(mdata->mdp_rev < MDSS_MDP_HW_REV_106)) {
			if (use_mdp_up_misr)
				writel_relaxed((val +
					MDSS_MDP_UP_MISR_LMIX_SEL_OFFSET),
					(mdata->mdp_base +
					 MDSS_MDP_UP_MISR_SEL));
			else
				writel_relaxed(val,
					(mdata->mdp_base +
					MDSS_MDP_LP_MISR_SEL));
		}
	}
	vsync_count = 0;
	map->crc_op_mode = req->crc_op_mode;
	config = (MDSS_MDP_MISR_CTRL_FRAME_COUNT_MASK & req->frame_count) |
			(MDSS_MDP_MISR_CTRL_ENABLE);

	writel_relaxed(MDSS_MDP_MISR_CTRL_STATUS_CLEAR,
			mdata->mdp_base + map->ctrl_reg);
	/* ensure clear is done */
	wmb();

	memset(map->crc_ping, 0, sizeof(map->crc_ping));
	memset(map->crc_pong, 0, sizeof(map->crc_pong));
	map->crc_index = 0;
	map->use_ping = true;
	map->is_ping_full = false;
	map->is_pong_full = false;

	if (MISR_OP_BM != map->crc_op_mode) {

		writel_relaxed(config,
				mdata->mdp_base + map->ctrl_reg);
		pr_debug("MISR_CTRL=0x%x [base:0x%pK reg:0x%x config:0x%x]\n",
				readl_relaxed(mdata->mdp_base + map->ctrl_reg),
				mdata->mdp_base, map->ctrl_reg, config);
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	return 0;
}

char *get_misr_block_name(int misr_block_id)
{
	switch (misr_block_id) {
	case DISPLAY_MISR_EDP: return "eDP";
	case DISPLAY_MISR_DSI0: return "DSI_0";
	case DISPLAY_MISR_DSI1: return "DSI_1";
	case DISPLAY_MISR_HDMI: return "HDMI";
	case DISPLAY_MISR_MDP: return "Writeback";
	case DISPLAY_MISR_DSI_CMD: return "DSI_CMD";
	default: return "???";
	}
}

int mdss_misr_get(struct mdss_data_type *mdata,
			struct mdp_misr *resp,
			struct mdss_mdp_ctl *ctl,
			bool is_video_mode)
{
	struct mdss_mdp_misr_map *map;
	struct mdss_mdp_mixer *mixer;
	u32 status;
	int ret = -1;
	int i;

	pr_debug("req[block:%d frame:%d op_mode:%d]\n",
		resp->block_id, resp->frame_count, resp->crc_op_mode);

	map = mdss_misr_get_map(resp->block_id, ctl, mdata,
		is_video_mode);
	if (!map) {
		pr_err("Invalid MISR Block=%d\n", resp->block_id);
		return -EINVAL;
	}
	switch_mdp_misr_offset(map, mdata->mdp_rev, resp->block_id);

	mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_DEFAULT);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
	switch (map->crc_op_mode) {
	case MISR_OP_SFM:
	case MISR_OP_MFM:
		ret = readl_poll_timeout(mdata->mdp_base + map->ctrl_reg,
				status, status & MDSS_MDP_MISR_CTRL_STATUS,
				MISR_POLL_SLEEP, MISR_POLL_TIMEOUT);
		if (ret == 0) {
			resp->crc_value[0] = readl_relaxed(mdata->mdp_base +
				map->value_reg);
			pr_debug("CRC %s=0x%x\n",
				get_misr_block_name(resp->block_id),
				resp->crc_value[0]);
			writel_relaxed(0, mdata->mdp_base + map->ctrl_reg);
		} else {
			pr_debug("Get MISR TimeOut %s\n",
				get_misr_block_name(resp->block_id));

			ret = readl_poll_timeout(mdata->mdp_base +
					map->ctrl_reg, status,
					status & MDSS_MDP_MISR_CTRL_STATUS,
					MISR_POLL_SLEEP, MISR_POLL_TIMEOUT);
			if (ret == 0) {
				resp->crc_value[0] =
					readl_relaxed(mdata->mdp_base +
					map->value_reg);
				pr_debug("Retry CRC %s=0x%x\n",
					get_misr_block_name(resp->block_id),
					resp->crc_value[0]);
			} else {
				pr_err("Get MISR TimeOut %s\n",
					get_misr_block_name(resp->block_id));
			}
			writel_relaxed(0, mdata->mdp_base + map->ctrl_reg);
		}
		break;
	case MISR_OP_BM:
		if (map->is_ping_full) {
			for (i = 0; i < MISR_CRC_BATCH_SIZE; i++)
				resp->crc_value[i] = map->crc_ping[i];
			memset(map->crc_ping, 0, sizeof(map->crc_ping));
			map->is_ping_full = false;
			ret = 0;
		} else if (map->is_pong_full) {
			for (i = 0; i < MISR_CRC_BATCH_SIZE; i++)
				resp->crc_value[i] = map->crc_pong[i];
			memset(map->crc_pong, 0, sizeof(map->crc_pong));
			map->is_pong_full = false;
			ret = 0;
		} else {
			pr_debug("mdss_mdp_misr_crc_get PING BUF %s\n",
				map->is_ping_full ? "FULL" : "EMPTRY");
			pr_debug("mdss_mdp_misr_crc_get PONG BUF %s\n",
				map->is_pong_full ? "FULL" : "EMPTRY");
		}
		resp->crc_op_mode = map->crc_op_mode;
		break;
	default:
		ret = -ENOSYS;
		break;
	}

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	return ret;
}

/* This function is expected to be called from interrupt context */
void mdss_misr_crc_collect(struct mdss_data_type *mdata, int block_id,
	bool is_video_mode)
{
	struct mdss_mdp_misr_map *map;
	u32 status = 0;
	u32 crc = 0x0BAD0BAD;
	bool crc_stored = false;

	map = mdss_misr_get_map(block_id, NULL, mdata, is_video_mode);
	if (!map || (map->crc_op_mode != MISR_OP_BM))
		return;

	switch_mdp_misr_offset(map, mdata->mdp_rev, block_id);

	status = readl_relaxed(mdata->mdp_base + map->ctrl_reg);

	if (MDSS_MDP_MISR_CTRL_STATUS & status) {

		crc = readl_relaxed(mdata->mdp_base + map->value_reg);
		map->last_misr = crc; /* cache crc to get it from sysfs */

		if (map->use_ping) {
			if (map->is_ping_full) {
				pr_err_once("PING Buffer FULL\n");
			} else {
				map->crc_ping[map->crc_index] = crc;
				crc_stored = true;
			}
		} else {
			if (map->is_pong_full) {
				pr_err_once("PONG Buffer FULL\n");
			} else {
				map->crc_pong[map->crc_index] = crc;
				crc_stored = true;
			}
		}

		if (crc_stored) {
			map->crc_index = (map->crc_index + 1);
			if (map->crc_index == MISR_CRC_BATCH_SIZE) {
				map->crc_index = 0;
				if (true == map->use_ping) {
					map->is_ping_full = true;
					map->use_ping = false;
				} else {
					map->is_pong_full = true;
					map->use_ping = true;
				}
				pr_debug("USE BUFF %s\n", map->use_ping ?
					"PING" : "PONG");
				pr_debug("mdss_misr_crc_collect PING BUF %s\n",
					map->is_ping_full ? "FULL" : "EMPTRY");
				pr_debug("mdss_misr_crc_collect PONG BUF %s\n",
					map->is_pong_full ? "FULL" : "EMPTRY");
			}
		} else {
			pr_err_once("CRC(%d) Not saved\n", crc);
		}

		if (mdata->mdp_rev < MDSS_MDP_HW_REV_105) {
			writel_relaxed(MDSS_MDP_MISR_CTRL_STATUS_CLEAR,
					mdata->mdp_base + map->ctrl_reg);
			writel_relaxed(MISR_CRC_BATCH_CFG,
				mdata->mdp_base + map->ctrl_reg);
		}

	} else if (0 == status) {

		if (mdata->mdp_rev < MDSS_MDP_HW_REV_105)
			writel_relaxed(MISR_CRC_BATCH_CFG,
					mdata->mdp_base + map->ctrl_reg);
		else
			writel_relaxed(MISR_CRC_BATCH_CFG |
					MDSS_MDP_LP_MISR_CTRL_FREE_RUN_MASK,
					mdata->mdp_base + map->ctrl_reg);

		pr_debug("$$ Batch CRC Start $$\n");
	}

	pr_debug("$$ Vsync Count = %d, CRC=0x%x Indx = %d$$\n",
		vsync_count, crc, map->crc_index);

	if (MAX_VSYNC_COUNT == vsync_count) {
		pr_debug("RESET vsync_count(%d)\n", vsync_count);
		vsync_count = 0;
	} else {
		vsync_count += 1;
	}

}
