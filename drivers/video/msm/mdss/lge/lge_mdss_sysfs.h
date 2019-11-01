#ifndef _H_LGE_MDSS_SYSFS_
#define _H_LGE_MDSS_SYSFS_

static int get_data(struct device *dev, struct fb_info **pfbi, struct msm_fb_data_type **pmfd, struct mdss_panel_data **ppdata, struct mdss_dsi_ctrl_pdata **pctrl)
{
	struct fb_info *fbi = NULL;
	struct msm_fb_data_type *mfd = NULL;
	struct mdss_panel_data *pdata = NULL;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	if (!pfbi)
		return 0;

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		pr_err("%pS : uninitialzed fb0\n", __builtin_return_address(0));
		return -EINVAL;
	}
	*pfbi = fbi;

	if (!pmfd)
		return 0;

	mfd = (struct msm_fb_data_type *)(fbi)->par;
	if (mfd == NULL) {
		pr_err("%pS : uninitialzed mfd\n", __builtin_return_address(0));
		return -EINVAL;
	}
	*pmfd = mfd;

	if (!ppdata)
		return 0;

	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (pdata == NULL) {
		pr_err("%pS: no panel connected!\n", __builtin_return_address(0));
		return -EINVAL;
	}
	*ppdata = pdata;

	if (!pctrl)
		return 0;

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata, panel_data);
	if (ctrl == NULL) {
		pr_err("%pS: ctrl is null\n", __builtin_return_address(0));
		return -EINVAL;
	}
	*pctrl = ctrl;

	return 0;
}
#define GET_DATA \
	struct fb_info *fbi = NULL; \
	struct msm_fb_data_type *mfd = NULL; \
	struct mdss_panel_data *pdata = NULL; \
	struct mdss_dsi_ctrl_pdata *ctrl = NULL; \
	if(get_data(dev, &fbi, &mfd, &pdata, &ctrl))  \
		return -EINVAL;  \

#endif //_H_LGE_MDSS_SYSFS_
