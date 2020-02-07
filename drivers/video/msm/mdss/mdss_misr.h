#ifndef MDSS_MISR_H
#define MDSS_MISR_H

#define MAX_VSYNC_COUNT		0xFFFFFFF
#define MISR_POLL_SLEEP		2000
#define MISR_POLL_TIMEOUT	32000
#define MISR_CRC_BATCH_CFG	0x101

int mdss_misr_set(struct mdss_data_type *mdata,
			struct mdp_misr *req,
			struct mdss_mdp_ctl *ctl);
int mdss_misr_get(struct mdss_data_type *mdata,
			struct mdp_misr *resp,
			struct mdss_mdp_ctl *ctl,
			bool is_video_mode);
void mdss_misr_disable(struct mdss_data_type *mdata,
			struct mdp_misr *req,
			struct mdss_mdp_ctl *ctl);
void mdss_misr_crc_collect(struct mdss_data_type *mdata, int block_id,
	bool is_video_mode);

#endif
