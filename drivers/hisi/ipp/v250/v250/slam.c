
#include <linux/printk.h>
#include <linux/string.h>
#include "ipp.h"
#include "slam_drv.h"
#include "slam_drv_priv.h"
#include "slam_reg_offset.h"
#include "slam_reg_offset_field.h"

#define LOG_TAG LOG_MODULE_SLAM_DRV

/**********************************************************
function name: slam_set_ctrl

description:
    set the contrl register of SLAM device.

input:
    dev  : the SLAM device
    ctrl : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_ctrl(slam_dev_t *dev, slam_ctrl_cfg_t *ctrl)
{
	U_SLAM_CFG temp;

	if (NULL == dev || NULL == ctrl) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	if ((1 == ctrl->brief_en) && (1 == ctrl->freak_en)) {
		E("brief and freak cannot be enabled at the same time.\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.pyramid_en = ctrl->pyramid_en;
	temp.bits.gsblur_en = ctrl->gsblur_en;
	temp.bits.fast_en = ctrl->fast_en;
	temp.bits.nms_en = ctrl->nms_en;
	temp.bits.orient_en = ctrl->orient_en;
	temp.bits.brief_en = ctrl->brief_en;
	temp.bits.freak_en = ctrl->freak_en;
	temp.bits.scoremap_en = ctrl->scoremap_en;
	temp.bits.gridstat_en = ctrl->gridstat_en;
	temp.bits.undistort_en = ctrl->undistort_en;
	CMDLST_SET_REG(dev->base_addr + SLAM_SLAM_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_size

description:
     set size for slam device.

input:
    dev  : the SLAM device
    size : the size value

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_size(slam_dev_t *dev, slam_size_cfg_t *size)
{
	U_IMAGE_SIZE temp;

	if (NULL == dev || NULL == size) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.width  = size->width;
	temp.bits.height = size->height;
	CMDLST_SET_REG(dev->base_addr + SLAM_IMAGE_SIZE_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_pyramid_scaler

description:
     set pyramid_scaler para for slam device.

input:
    dev  : the SLAM device
    pyramid_scaler : the pyramid_scaler value

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_pyramid_scaler(slam_dev_t *dev, slam_pyramid_scaler_cfg_t *pyramid_scaler)
{
	U_PYRAMID_INC_CFG temp_scl_inc;
	U_PYRAMID_VCROP_CFGB temp_crop_bottom;
	U_PYRAMID_VCROP_CFGT temp_crop_top;
	U_PYRAMID_HCROP_CFGR temp_crop_right;
	U_PYRAMID_HCROP_CFGL temp_crop_left;

	if (NULL == dev || NULL == pyramid_scaler) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_scl_inc.u32 = 0;
	temp_crop_bottom.u32 = 0;
	temp_crop_top.u32 = 0;
	temp_crop_right.u32 = 0;
	temp_crop_left.u32 = 0;
	temp_scl_inc.bits.scl_inc = pyramid_scaler->scl_inc;
	CMDLST_SET_REG(dev->base_addr + SLAM_PYRAMID_INC_CFG_REG, temp_scl_inc.u32);
	temp_crop_bottom.bits.scl_vbottom = pyramid_scaler->scl_vbottom;
	CMDLST_SET_REG(dev->base_addr + SLAM_PYRAMID_VCROP_CFGB_REG, temp_crop_bottom.u32);
	temp_crop_top.bits.scl_vtop = pyramid_scaler->scl_vtop;
	CMDLST_SET_REG(dev->base_addr + SLAM_PYRAMID_VCROP_CFGT_REG, temp_crop_top.u32);
	temp_crop_right.bits.scl_hright = pyramid_scaler->scl_hright;
	CMDLST_SET_REG(dev->base_addr + SLAM_PYRAMID_HCROP_CFGR_REG, temp_crop_right.u32);
	temp_crop_left.bits.scl_hleft = pyramid_scaler->scl_hleft;
	CMDLST_SET_REG(dev->base_addr + SLAM_PYRAMID_HCROP_CFGL_REG, temp_crop_left.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_gsblur_coef

description:
     set gsblur_coef for slam device.

input:
    dev  : the SLAM device
    gsblur_coef : the gsblur_coef

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_gsblur_coef(slam_dev_t *dev, slam_gsblur_coef_cfg_t *gsblur_coef)
{
	U_GSBLUR_COEF_01 temp_gsblur_01;
	U_GSBLUR_COEF_23 temp_gsblur_23;

	if (NULL == dev || NULL == gsblur_coef) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_gsblur_01.u32 = 0;
	temp_gsblur_23.u32 = 0;
	temp_gsblur_01.bits.coeff_gauss_0  = gsblur_coef->coeff_gauss_0;
	temp_gsblur_01.bits.coeff_gauss_1  = gsblur_coef->coeff_gauss_1;
	temp_gsblur_23.bits.coeff_gauss_2  = gsblur_coef->coeff_gauss_2;
	temp_gsblur_23.bits.coeff_gauss_3  = gsblur_coef->coeff_gauss_3;
	CMDLST_SET_REG(dev->base_addr + SLAM_GSBLUR_COEF_01_REG, temp_gsblur_01.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_GSBLUR_COEF_23_REG, temp_gsblur_23.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_fast

description:
     set fast coef for slam device.

input:
    dev  : the SLAM device
    fast_coef : the fast coef

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_fast(slam_dev_t *dev, slam_fast_coef_t *fast_coef)
{
	U_THRESHOLD_CFG temp;

	if (NULL == dev || NULL == fast_coef) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.ini_th = fast_coef->ini_th;
	temp.bits.min_th = fast_coef->min_th;
	CMDLST_SET_REG(dev->base_addr + SLAM_THRESHOLD_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_nms_win

description:
     set nms window for slam device.

input:
    dev  : the SLAM device
    nms_win : the nms window coef

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_nms_win(slam_dev_t *dev, slam_nms_win_t *nms_win)
{
	U_NMS_WIN_CFG temp;

	if (NULL == dev || NULL == nms_win) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.nmscell_v = nms_win->nmscell_v;
	temp.bits.nmscell_h = nms_win->nmscell_h;
	CMDLST_SET_REG(dev->base_addr + SLAM_NMS_WIN_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_gridstat

description:
     set grid status for slam device.

input:
    dev  : the SLAM device
    gridstat_cfg : the grid status coef

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_gridstat(slam_dev_t *dev, slam_gridstat_cfg_t *gridstat_cfg)
{
	U_BLOCK_NUM_CFG  temp_blk_num;
	U_BLOCK_SIZE_CFG temp_blk_size;

	if (NULL == dev || NULL == gridstat_cfg) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_blk_num.u32 = 0;
	temp_blk_size.u32 = 0;
	temp_blk_num.bits.blk_v_num = gridstat_cfg->block_num.blk_v_num;
	temp_blk_num.bits.blk_h_num = gridstat_cfg->block_num.blk_h_num;
	temp_blk_size.bits.blk_v_size = gridstat_cfg->block_size.blk_v_size;
	temp_blk_size.bits.blk_h_size = gridstat_cfg->block_size.blk_h_size;
	CMDLST_SET_REG(dev->base_addr + SLAM_BLOCK_NUM_CFG_REG, temp_blk_num.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_BLOCK_SIZE_CFG_REG, temp_blk_size.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: orb_set_octree

description:
     set octree for slam device.

input:
    dev  : the SLAM device
    nms_win : the octree coef

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_octree(slam_dev_t *dev, slam_octree_cfg_t *octree_cfg)
{
	unsigned int i = 0;
	U_OCTREE_CFG     temp_octree_cfg;
	U_SCORE_THESHOLD temp_score_theshold;
	U_INC_LUT_CFG    temp_inc_lut_cfg;

	if (NULL == dev || NULL == octree_cfg) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_octree_cfg.u32 = 0;
	temp_score_theshold.u32 = 0;
	temp_inc_lut_cfg.u32 = 0;
	temp_octree_cfg.bits.max_kpnum = octree_cfg->max_kpnum;
	temp_octree_cfg.bits.grid_max_kpnum = octree_cfg->grid_max_kpnum;
	temp_inc_lut_cfg.bits.inc_level = octree_cfg->inc_level;
	CMDLST_SET_REG(dev->base_addr + SLAM_OCTREE_CFG_REG, temp_octree_cfg.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_INC_LUT_CFG_REG, temp_inc_lut_cfg.u32);

	for (i = 0; i < SLAM_SCORE_TH_RANGE; i++) {
		temp_score_theshold.bits.score_th = octree_cfg->score_th[i];
		CMDLST_SET_REG(dev->base_addr + SLAM_SCORE_THESHOLD_0_REG + 4 * i, temp_score_theshold.u32);
	}

	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_undistort

description:
     set undistort coef for slam device.

input:
    dev  : the SLAM device
    undistort_cfg : the undistort coef

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_undistort(slam_dev_t *dev, slam_undistort_cfg_t *undistort_cfg)
{
	U_UNDISTORT_CX    temp_cx;
	U_UNDISTORT_CY    temp_cy;
	U_UNDISTORT_FX    temp_fx;
	U_UNDISTORT_FY    temp_fy;
	U_UNDISTORT_INVFX temp_invfx;
	U_UNDISTORT_INVFY temp_invfy;
	U_UNDISTORT_K1    temp_k1;
	U_UNDISTORT_K2    temp_k2;
	U_UNDISTORT_K3    temp_k3;
	U_UNDISTORT_P1    temp_p1;
	U_UNDISTORT_P2    temp_p2;

	if (NULL == dev || NULL == undistort_cfg) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_cx.u32 = 0;
	temp_cy.u32 = 0;
	temp_fx.u32 = 0;
	temp_fy.u32 = 0;
	temp_invfx.u32 = 0;
	temp_invfy.u32 = 0;
	temp_k1.u32 = 0;
	temp_k2.u32 = 0;
	temp_k3.u32 = 0;
	temp_p1.u32 = 0;
	temp_p2.u32 = 0;
	temp_cx.bits.cx = undistort_cfg->cx;
	temp_cy.bits.cy = undistort_cfg->cy;
	temp_fx.bits.fx = undistort_cfg->fx;
	temp_fy.bits.fy = undistort_cfg->fy;
	temp_invfx.bits.invfx = undistort_cfg->invfx;
	temp_invfy.bits.invfy = undistort_cfg->invfy;
	temp_k1.bits.k1 = undistort_cfg->k1;
	temp_k2.bits.k2 = undistort_cfg->k2;
	temp_k3.bits.k3 = undistort_cfg->k3;
	temp_p1.bits.p1 = undistort_cfg->p1;
	temp_p2.bits.p2 = undistort_cfg->p2;
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_CX_REG, temp_cx.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_CY_REG, temp_cy.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_FX_REG, temp_fx.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_FY_REG, temp_fy.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_INVFX_REG, temp_invfx.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_INVFY_REG, temp_invfy.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_K1_REG, temp_k1.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_K2_REG, temp_k2.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_K3_REG, temp_k3.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_P1_REG, temp_p1.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_UNDISTORT_P2_REG, temp_p2.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: orb_set_octree

description:
     set rBRIEF/FREAK pattern for slam device.

input:
    dev  : the SLAM device
    pattern_cfg : the rBRIEF/FREAK pattern coef

output:
    success : 0
    fail :  other
**********************************************************/

static int slam_set_pattern(slam_dev_t *dev, slam_pattern_cfg_t *pattern_cfg)
{
	unsigned int i = 0;
	U_BRIEF_PATTERN temp_pattern;

	if (NULL == dev || NULL == pattern_cfg) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_pattern.u32 = 0;

	for (i = 0; i < SLAM_PATTERN_RANGE; i++) {
		temp_pattern.bits.pattern_x0 = pattern_cfg->pattern_x0[i];
		temp_pattern.bits.pattern_y0 = pattern_cfg->pattern_y0[i];
		temp_pattern.bits.pattern_x1 = pattern_cfg->pattern_x1[i];
		temp_pattern.bits.pattern_y1 = pattern_cfg->pattern_y1[i];
		CMDLST_SET_REG(dev->base_addr + SLAM_BRIEF_PATTERN_0_REG + 4 * i, temp_pattern.u32);
	}

	return CPE_FW_OK;
}

/**********************************************************
function name: slam_set_cvdr

description:
     set cvdr for slam device.

input:
    dev  : the SLAM device
    size : the cvdr setting

output:
    success : 0
    fail :  other
**********************************************************/
static int slam_set_cvdr(slam_dev_t *dev, slam_cvdr_cfg_t *cvdr_cfg)
{
	U_CVDR_CFG_0 temp_cvdr_cfg0;
	U_CVDR_CFG_1 temp_cvdr_cfg1;

	if (NULL == dev || NULL == cvdr_cfg) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp_cvdr_cfg0.u32 = 0;
	temp_cvdr_cfg1.u32 = 0;
	temp_cvdr_cfg0.bits.vprd_horizontal_blanking  = cvdr_cfg->vprd_horizontal_blanking;
	temp_cvdr_cfg1.bits.vprd_line_wrap  = cvdr_cfg->vprd_line_wrap;
	CMDLST_SET_REG(dev->base_addr + SLAM_CVDR_CFG_0_REG, temp_cvdr_cfg0.u32);
	CMDLST_SET_REG(dev->base_addr + SLAM_CVDR_CFG_1_REG, temp_cvdr_cfg1.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: slam_do_config

description:
     do_config interface of SLAM device.

input:
    dev    : the SLAM device
    table  : the configure table

output:
    success : 0
    fail : other
**********************************************************/
static int slam_do_config(slam_dev_t *dev, cfg_tab_slam_t *tab_slam)
{
	if (NULL == dev || NULL == tab_slam) {
		E("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	if (tab_slam->ctrl.to_use) {
		loge_if_ret(slam_set_ctrl(dev, &(tab_slam->ctrl)));
		tab_slam->ctrl.to_use = 0;
	}

	if (tab_slam->size_cfg.to_use) {
		loge_if_ret(slam_set_size(dev, &(tab_slam->size_cfg)));
		tab_slam->size_cfg.to_use = 0;
	}

	if (tab_slam->pyramid_scaler_cfg.to_use) {
		loge_if_ret(slam_set_pyramid_scaler(dev, &(tab_slam->pyramid_scaler_cfg)));
		tab_slam->pyramid_scaler_cfg.to_use = 0;
	}

	if (tab_slam->gsblur_coef_cfg.to_use) {
		loge_if_ret(slam_set_gsblur_coef(dev, &(tab_slam->gsblur_coef_cfg)));
		tab_slam->gsblur_coef_cfg.to_use = 0;
	}

	if ((tab_slam->fast_coef.to_use) || (tab_slam->nms_win.to_use)) {
		loge_if_ret(slam_set_fast(dev, &(tab_slam->fast_coef)));
		tab_slam->fast_coef.to_use = 0;
	}

	if (tab_slam->nms_win.to_use) {
		loge_if_ret(slam_set_nms_win(dev, &(tab_slam->nms_win)));
		tab_slam->nms_win.to_use = 0;
	}

	if (tab_slam->gridstat_cfg.to_use) {
		loge_if_ret(slam_set_gridstat(dev, &(tab_slam->gridstat_cfg)));
		tab_slam->gridstat_cfg.to_use = 0;
	}

	if (tab_slam->octree_cfg.to_use) {
		loge_if_ret(slam_set_octree(dev, &(tab_slam->octree_cfg)));
		tab_slam->octree_cfg.to_use = 0;
	}

	if (tab_slam->undistort_cfg.to_use) {
		loge_if_ret(slam_set_undistort(dev, &(tab_slam->undistort_cfg)));
		tab_slam->undistort_cfg.to_use = 0;
	}

	if ((1 == tab_slam->ctrl.brief_en) || (1 == tab_slam->ctrl.freak_en)) {
		loge_if_ret(slam_set_pattern(dev, &(tab_slam->pattern_cfg)));
	}

	if (tab_slam->cvdr_cfg.to_use) {
		loge_if_ret(slam_set_cvdr(dev, &(tab_slam->cvdr_cfg)));
		tab_slam->cvdr_cfg.to_use = 0;
	}

	return CPE_FW_OK;
}


int slam_prepare_cmd(slam_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_slam_t *table)
{
	dev->cmd_buf = cmd_buf;
	loge_if_ret(slam_do_config(dev, table));
	return 0;
}

static slam_ops_t slam_ops = {
	.prepare_cmd   = slam_prepare_cmd,
};

slam_dev_t g_slam_devs[] = {
	[0] =
	{
		.base_addr = JPG_SLAM_ADDR,
		.ops = &slam_ops,
	},
};



/********************************** END **********************************/



