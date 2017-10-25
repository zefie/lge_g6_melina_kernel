/* This appears to be missing from FB_MSM_MDSS_COMMON */

static void msmfb_update(struct fb_info *info, uint32_t left, uint32_t top,
			 uint32_t eright, uint32_t ebottom);
static void msmfb_pan_update(struct fb_info *info, uint32_t left, uint32_t top,
			 uint32_t eright, uint32_t ebottom,
			 uint32_t yoffset, int pan_display);

/* from elsewhere in mdss: */
static int mdss_fb_pan_display(struct fb_var_screeninfo *var,
			       struct fb_info *info);

static void msmfb_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
	cfb_fillrect(p, rect);
	msmfb_update(p, rect->dx, rect->dy, rect->dx + rect->width,
		     rect->dy + rect->height);
}

static void msmfb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
	cfb_copyarea(p, area);
	msmfb_update(p, area->dx, area->dy, area->dx + area->width,
		     area->dy + area->height);
}

static void msmfb_imageblit(struct fb_info *p, const struct fb_image *image)
{
	cfb_imageblit(p, image);
	msmfb_update(p, image->dx, image->dy, image->dx + image->width,
		     image->dy + image->height);

}

static void msmfb_pan_update(struct fb_info *info, uint32_t left, uint32_t top,
			     uint32_t eright, uint32_t ebottom,
			     uint32_t yoffset, int pan_display)
{
	mdss_fb_pan_display(&info->var, info);
}

static void msmfb_update(struct fb_info *info, uint32_t left, uint32_t top,
			 uint32_t eright, uint32_t ebottom)
{
	msmfb_pan_update(info, left, top, eright, ebottom, 0, 0);
}
