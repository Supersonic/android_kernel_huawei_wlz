/*
 * hisi_overlay_utils_dssv510.c
 *
 * The functions used for the process of displaying.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winteger-overflow"

#include "hisi_overlay_utils.h"
#include "hisi_mmbuf_manager.h"

static uint32_t vactive_timeout_count = 0;

uint32_t g_dss_module_base[DSS_CHN_MAX_DEFINE][MODULE_CHN_MAX] = {
	// D0
	{
	MIF_CH0_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH0_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH0_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH0,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH0_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH0_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH0_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD0_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_D0_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_D0_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_D0_CSC_OFFSET,  // MODULE_CSC
	},

	// D1
	{
	MIF_CH1_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH1_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH1_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH1,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH1_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH1_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH1_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD1_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_D1_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_D1_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  //MODULE_PCSC
	DSS_RCH_D1_CSC_OFFSET,  // MODULE_CSC
	},

	// V0
	{
	MIF_CH2_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH2_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH2_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH2,  //MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH2_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH2_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH2_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD2_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_VG0_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_VG0_DFC_OFFSET,  // MODULE_DFC
	DSS_RCH_VG0_SCL_OFFSET,  // MODULE_SCL
	DSS_RCH_VG0_SCL_LUT_OFFSET,  // MODULE_SCL_LUT
	DSS_RCH_VG0_ARSR_OFFSET,  // MODULE_ARSR2P
	DSS_RCH_VG0_ARSR_LUT_OFFSET,  //MODULE_ARSR2P_LUT
	DSS_RCH_VG0_POST_CLIP_OFFSET,  // MODULE_POST_CLIP
	DSS_RCH_VG0_PCSC_OFFSET,  // MODULE_PCSC
	DSS_RCH_VG0_CSC_OFFSET,  // MODULE_CSC
	},

	// G0
	{
	MIF_CH3_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH3_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH3_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH3,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH3_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH3_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH3_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD3_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_G0_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_G0_DFC_OFFSET,  // MODULE_DFC
	DSS_RCH_G0_SCL_OFFSET,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	DSS_RCH_G0_POST_CLIP_OFFSET,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_G0_CSC_OFFSET,  // MODULE_CSC
	},

	// V1
	{
	MIF_CH4_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH4_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH4_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH4,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH4_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH4_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH4_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD4_DBG,  //MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_VG1_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_VG1_DFC_OFFSET,  // MODULE_DFC
	DSS_RCH_VG1_SCL_OFFSET,  // MODULE_SCL
	DSS_RCH_VG1_SCL_LUT_OFFSET,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	DSS_RCH_VG1_POST_CLIP_OFFSET,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_VG1_CSC_OFFSET,  // MODULE_CSC
	},

	// G1
	{
	MIF_CH5_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH5_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH5_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH5,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH5_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH5_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH5_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD5_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_G1_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_G1_DFC_OFFSET,  // MODULE_DFC
	DSS_RCH_G1_SCL_OFFSET,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	DSS_RCH_G1_POST_CLIP_OFFSET,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_G1_CSC_OFFSET,  // MODULE_CSC
	},

	// D2
	{
	MIF_CH6_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH6_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH6_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH6,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH6_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH6_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH6_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD6_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_D2_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_D2_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_D2_CSC_OFFSET,  // MODULE_CSC
	},

	// D3
	{
	MIF_CH7_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH7_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH7_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH7,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH7_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH7_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH7_STARTY,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD7_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_D3_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_D3_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_D3_CSC_OFFSET,  // MODULE_CSC
	},

	// W0
	{
	MIF_CH8_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH8_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH8_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_WCH0,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_WCH0_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_WCH0_OV_IEN,  // MODULE_MCTL_CHN_OV_OEN
	0,  // MODULE_MCTL_CHN_STARTY
	0,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_WCH0_DMA_OFFSET,  // MODULE_DMA
	DSS_WCH0_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_WCH0_CSC_OFFSET,  // MODULE_CSC
	},

	// W1
	{
	MIF_CH9_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH9_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH9_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_WCH1,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_WCH1_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_WCH1_OV_IEN,  // MODULE_MCTL_CHN_OV_OEN
	0,  // MODULE_MCTL_CHN_STARTY
	0,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_WCH1_DMA_OFFSET,  // MODULE_DMA
	DSS_WCH1_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_WCH1_CSC_OFFSET,  //MODULE_CSC
	},

	// V2
	{
	MIF_CH10_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH11_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH11_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_RCH8,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH8_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	DSS_MCTRL_SYS_OFFSET + MCTL_RCH8_OV_OEN,  // MODULE_MCTL_CHN_OV_OEN
	0,  // MODULE_MCTL_CHN_STARTY
	DSS_MCTRL_SYS_OFFSET + MCTL_MOD8_DBG,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_RCH_VG2_DMA_OFFSET,  // MODULE_DMA
	DSS_RCH_VG2_DFC_OFFSET,  // MODULE_DFC
	DSS_RCH_VG2_SCL_OFFSET,  // MODULE_SCL
	DSS_RCH_VG2_SCL_LUT_OFFSET,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	DSS_RCH_VG2_POST_CLIP_OFFSET,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_RCH_VG2_CSC_OFFSET,  // MODULE_CSC
	},
	// W2
	{
	MIF_CH11_OFFSET,   // MODULE_MIF_CHN
	AIF0_CH12_OFFSET,  // MODULE_AIF0_CHN
	AIF1_CH12_OFFSET,  // MODULE_AIF1_CHN
	MCTL_CTL_MUTEX_WCH2,  // MODULE_MCTL_CHN_MUTEX
	DSS_MCTRL_SYS_OFFSET + MCTL_WCH2_FLUSH_EN,  // MODULE_MCTL_CHN_FLUSH_EN
	0,  // MODULE_MCTL_CHN_OV_OEN
	0,  // MODULE_MCTL_CHN_STARTY
	0,  // MODULE_MCTL_CHN_MOD_DBG
	DSS_WCH2_DMA_OFFSET,  // MODULE_DMA
	DSS_WCH2_DFC_OFFSET,  // MODULE_DFC
	0,  // MODULE_SCL
	0,  // MODULE_SCL_LUT
	0,  // MODULE_ARSR2P
	0,  // MODULE_ARSR2P_LUT
	0,  // MODULE_POST_CLIP
	0,  // MODULE_PCSC
	DSS_WCH2_CSC_OFFSET,  // MODULE_CSC
	},
};

uint32_t g_dss_module_ovl_base[DSS_MCTL_IDX_MAX][MODULE_OVL_MAX] = {
	{DSS_OVL0_OFFSET,
	DSS_MCTRL_CTL0_OFFSET},

	{DSS_OVL1_OFFSET,
	DSS_MCTRL_CTL1_OFFSET},

	{DSS_OVL2_OFFSET,
	DSS_MCTRL_CTL2_OFFSET},

	{DSS_OVL3_OFFSET,
	DSS_MCTRL_CTL3_OFFSET},

	{0,
	DSS_MCTRL_CTL4_OFFSET},

	{0,
	DSS_MCTRL_CTL5_OFFSET},
};

//SCF_LUT_CHN coef_idx
int g_scf_lut_chn_coef_idx[DSS_CHN_MAX_DEFINE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

uint32_t g_dss_module_cap[DSS_CHN_MAX_DEFINE][MODULE_CAP_MAX] = {
	/* D2 */
	{0,0,1,0,0,0,1,0,0,0,1},
	/* D3 */
	{0,0,1,0,0,0,0,0,0,0,1},
	/* V0 */
	{0,1,1,0,1,1,1,0,0,1,1},
	/* G0 */
	{0,1,0,0,0,0,1,0,0,0,0},
	/* V1 */
	{0,1,1,1,0,1,1,0,1,1,1},
	/* G1 */
	{0,1,0,0,0,0,1,0,0,0,0},
	/* D0 */
	{0,0,1,0,0,0,0,0,0,0,1},
	/* D1 */
	{0,0,1,0,0,0,0,0,0,0,1},

	/* W0 */
	{1,0,1,0,0,0,0,1,0,1,1},
	/* W1 */
	{1,0,1,0,0,0,0,1,0,1,1},

	/* V2 */
	{0,1,1,1,0,1,1,0,1,1,1},
	/* W2 */
	{1,0,1,0,0,0,0,1,0,1,1},
};

/* number of smrx idx for each channel */
uint32_t g_dss_chn_sid_num[DSS_CHN_MAX_DEFINE] = {
    4, 1, 4, 4, 4, 4, 1, 1, 3, 4, 3, 3
};

/* start idx of each channel */
/* smrx_idx = g_dss_smmu_smrx_idx[chn_idx] + (0 ~ g_dss_chn_sid_num[chn_idx]) */
uint32_t g_dss_smmu_smrx_idx[DSS_CHN_MAX_DEFINE] = {
    0, 4, 5, 9, 13, 17, 21, 22, 26, 29, 23, 36
};

uint32_t g_fpga_flag = 0;
//static int g_dss_module_resource_initialized = 0;
void *g_smmu_rwerraddr_virt = NULL;

static void aif_bw_sort(dss_aif_bw_t a[], int n)
{
	int i = 0;
	int j = 0;
	dss_aif_bw_t tmp;
	/*lint -e679*/
	for (; i < n; ++i) {
		for (j = i; j < n - 1; ++j) {
			if (a[j].bw > a[j+1].bw) {
				tmp = a[j];
				a[j] = a[j+1];
				a[j+1] = tmp;
			}
		}
	}
	/*lint +e679*/
}
/*lint -e574 -e647*/
int hisi_dss_aif_handler(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block)
{
	int i = 0;
	int j = 0;
	int k = 0;
	dss_layer_t *layer = NULL;
	dss_wb_layer_t *wb_layer = NULL;
	int chn_idx = 0;
	dss_aif_bw_t *aif_bw = NULL;
	uint32_t tmp = 0;
	uint32_t bw_sum = 0;

	int rch_cnt = 0;
	int axi0_cnt = 0;
	int axi1_cnt = 0;
	dss_aif_bw_t aif_bw_tmp[DSS_CHN_MAX_DEFINE];

	dss_aif_bw_t *aif1_bw = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if (NULL == pov_req) {
		HISI_FB_ERR("pov_req is NULL");
		return -EINVAL;
	}
	if (NULL == pov_h_block) {
		HISI_FB_ERR("pov_h_block is NULL");
		return -EINVAL;
	}

	memset(aif_bw_tmp, 0, sizeof(aif_bw_tmp));

	if (pov_req->wb_enable) {
		for (k = 0; k < pov_req->wb_layer_nums; k++) {
			wb_layer = &(pov_req->wb_layer_infos[k]);
			chn_idx = wb_layer->chn_idx;

			//
			aif_bw = &(hisifd->dss_module.aif_bw[chn_idx]);
			aif_bw->bw = (uint64_t)wb_layer->dst.buf_size *
				(wb_layer->src_rect.w * wb_layer->src_rect.h) / (wb_layer->dst.width * wb_layer->dst.height);
			aif_bw->chn_idx = chn_idx;
			if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON) {
				aif_bw->axi_sel = AXI_CHN0;
			} else {
				aif_bw->axi_sel = AXI_CHN1;
			}
			aif_bw->is_used = 1;
		}

		if ((pov_req->wb_compose_type == DSS_WB_COMPOSE_COPYBIT) ||
			(pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON)) {
			for (i = 0; i < pov_h_block->layer_nums; i++) {
				layer = &pov_h_block->layer_infos[i];
				chn_idx = layer->chn_idx;
				aif_bw_tmp[i].chn_idx = chn_idx;
				aif_bw_tmp[i].axi_sel = AXI_CHN0;
				aif_bw_tmp[i].is_used = 1;
				hisifd->dss_module.aif_bw[chn_idx] = aif_bw_tmp[i];
			}
		}
	}

	rch_cnt = 0;
	j = 0;
	//i is not chn_idx, is array idx
	for (i = 0; i < pov_h_block->layer_nums; i++) {
		layer = &pov_h_block->layer_infos[i];
		chn_idx = layer->chn_idx;

		if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
			continue;

		if ((layer->img.width == 0) || (layer->img.height == 0))
			continue;

		//MMBUF
		if ((layer->need_cap & CAP_AFBCD) || (layer->need_cap & CAP_HFBCD) ||
			(layer->need_cap & CAP_HEBCD)) {
			aif1_bw = &(hisifd->dss_module.aif1_bw[chn_idx]);
			aif1_bw->is_used = 1;
			aif1_bw->chn_idx = chn_idx;
			if ((pov_req->ovl_idx == DSS_OVL0) ||
				(pov_req->ovl_idx == DSS_OVL1)) {
				aif1_bw->axi_sel = (j % 2) ? AXI_CHN1 : AXI_CHN0;
			} else {
				aif1_bw->axi_sel = (j % 2) ? AXI_CHN0 : AXI_CHN1;
			}
			j++;
			if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer) {
				HISI_FB_INFO("fb%d, aif1, chn_idx=%d, axi_sel=%d.\n",
					hisifd->index, chn_idx, aif1_bw->axi_sel);
			}
		}

		aif_bw_tmp[i].bw = (uint64_t)layer->img.buf_size *
			(layer->src_rect.w * layer->src_rect.h) / (layer->img.width * layer->img.height);
		aif_bw_tmp[i].chn_idx = chn_idx;
		aif_bw_tmp[i].axi_sel = AXI_CHN0;
		aif_bw_tmp[i].is_used = 1;

		bw_sum += aif_bw_tmp[i].bw;
		rch_cnt++;
	}

	//sort
	aif_bw_sort(aif_bw_tmp, rch_cnt);

	//i is not chn_idx, is array idx
	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (aif_bw_tmp[i].is_used != 1)
			continue;

		tmp += aif_bw_tmp[i].bw;

		if ((pov_req->ovl_idx == DSS_OVL0) || (pov_req->ovl_idx == DSS_OVL1)) {
			if (tmp <= (bw_sum / 2)) {
				aif_bw_tmp[i].axi_sel = AXI_CHN0;
				if (axi0_cnt >= AXI0_MAX_DSS_CHN_THRESHOLD) {
					aif_bw_tmp[i - AXI0_MAX_DSS_CHN_THRESHOLD].axi_sel = AXI_CHN1;
					axi1_cnt++;
					axi0_cnt--;
				}
				axi0_cnt++;
			} else {
				aif_bw_tmp[i].axi_sel = AXI_CHN1;
				axi1_cnt++;
			}
		} else {
			if (tmp <= (bw_sum / 2)) {
				aif_bw_tmp[i].axi_sel = AXI_CHN1;
				if (axi1_cnt >= AXI1_MAX_DSS_CHN_THRESHOLD) {
					aif_bw_tmp[i - AXI1_MAX_DSS_CHN_THRESHOLD].axi_sel = AXI_CHN0;
					axi0_cnt++;
					axi1_cnt--;
				}
				axi1_cnt++;
			} else {
				aif_bw_tmp[i].axi_sel = AXI_CHN0;
				axi0_cnt++;
			}
		}

		chn_idx = aif_bw_tmp[i].chn_idx;
		hisifd->dss_module.aif_bw[chn_idx] = aif_bw_tmp[i];

		if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer) {
			HISI_FB_INFO("fb%d, aif0, chn_idx=%d, axi_sel=%d, bw=%llu.\n",
				hisifd->index, chn_idx, aif_bw_tmp[i].axi_sel, aif_bw_tmp[i].bw);
		}
	}

	return 0;
}
/*lint +e574 +e647*/
void hisi_dss_qos_on(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		return;
	}
}

/*******************************************************************************
** DSS AIF
*/
static int mid_array[DSS_CHN_MAX_DEFINE] = {0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x2, 0x1, 0x3, 0x0};

void hisi_dss_aif_init(const char __iomem *aif_ch_base,
	dss_aif_t *s_aif)
{
	if (NULL == aif_ch_base) {
		HISI_FB_ERR("aif_ch_base is NULL");
		return;
	}
	if (NULL == s_aif) {
		HISI_FB_ERR("s_aif is NULL");
		return;
	}

	//struct hisi_fb_data_type *hisifd_primary = hisifd_list[PRIMARY_PANEL_IDX];
	memset(s_aif, 0, sizeof(dss_aif_t));

	s_aif->aif_ch_ctl = inp32(aif_ch_base + AIF_CH_CTL);

	s_aif->aif_ch_hs = inp32(aif_ch_base + AIF_CH_HS);
	s_aif->aif_ch_ls = inp32(aif_ch_base + AIF_CH_LS);
	//set_reg(hisifd_primary->dss_base + DSS_VBIF0_AIF + AIF_CH_CTL_CMD, 0x1, 1, 0);
}

void hisi_dss_aif_ch_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *aif_ch_base, dss_aif_t *s_aif)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (aif_ch_base == NULL) {
		HISI_FB_ERR("aif_ch_base is null");
		return;
	}

	if (s_aif == NULL) {
		HISI_FB_ERR("s_aif is null");
		return;
	}

	hisifd->set_reg(hisifd, aif_ch_base + AIF_CH_CTL,
		s_aif->aif_ch_ctl, 32, 0);

	hisifd->set_reg(hisifd, aif_ch_base + AIF_CH_HS,
		s_aif->aif_ch_hs, 32, 0);
	hisifd->set_reg(hisifd, aif_ch_base + AIF_CH_LS,
		s_aif->aif_ch_ls, 32, 0);
}

int hisi_dss_aif_ch_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req,
	dss_layer_t *layer, dss_rect_t *wb_dst_rect, dss_wb_layer_t *wb_layer, int ovl_idx)
{
	dss_aif_t *aif = NULL;
	dss_aif_bw_t *aif_bw = NULL;
	int chn_idx = 0;
	int mid = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Point!");
		return -EINVAL;
	}
	if (pov_req == NULL){
		HISI_FB_ERR("pov_req is NULL Point!");
		return -EINVAL;
	}
	if ((layer == NULL) && (wb_layer == NULL)){
		HISI_FB_ERR("layer & wb_layer is NULL Point!");
		return -EINVAL;
	}
	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)){
		HISI_FB_ERR("ovl_idx(%d) is invalid!\n", ovl_idx);
		return -EINVAL;
	}
	/*lint -e613*/
	if (wb_layer != NULL) {
		chn_idx = wb_layer->chn_idx;
	} else {
		chn_idx = layer->chn_idx;
	}
	/*lint +e613*/
	aif = &(hisifd->dss_module.aif[chn_idx]);
	hisifd->dss_module.aif_ch_used[chn_idx] = 1;

	aif_bw = &(hisifd->dss_module.aif_bw[chn_idx]);
	if (aif_bw->is_used != 1) {
		HISI_FB_ERR("fb%d, aif_bw->is_used(%d) is invalid!", hisifd->index, aif_bw->is_used);
		return -EINVAL;
	}

	mid = mid_array[chn_idx];
	if (mid < 0 || mid > 0xb) {
		HISI_FB_ERR("fb%d, mid(%d) is invalid!", hisifd->index, mid);
		return -EINVAL;
	}

	aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, aif_bw->axi_sel, 1, 0);

	if (g_fpga_flag == 0) {
		if ((ovl_idx == DSS_OVL2) || (ovl_idx == DSS_OVL3)) {
			if (layer && ((layer->need_cap & CAP_AFBCD) != CAP_AFBCD)
				&& ((layer->need_cap & CAP_HFBCD) != CAP_HFBCD) &&
				((layer->need_cap & CAP_HEBCD) != CAP_HEBCD)) {
				aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, 0x1, 3, 8);
				aif->aif_ch_ls = set_bits32(aif->aif_ch_ls, 0x40061, 32, 0);
			}

			if (wb_layer && ((wb_layer->need_cap & CAP_AFBCE) != CAP_AFBCE)
				&& ((wb_layer->need_cap & CAP_HFBCE) != CAP_HFBCE) &&
				((wb_layer->need_cap & CAP_HEBCE) != CAP_HEBCE)) {
				aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, 0x1, 3, 8);
				aif->aif_ch_ls = set_bits32(aif->aif_ch_ls, 0x40061, 32, 0);
			}
		}
	}

	return 0;
}

int hisi_dss_aif1_ch_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer, int ovl_idx)
{
	dss_aif_t *aif1 = NULL;
	dss_aif_bw_t *aif1_bw = NULL;
	int chn_idx = 0;
	uint32_t need_cap = 0;
	int mid = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Point!");
		return -EINVAL;
	}
	if (pov_req == NULL){
		HISI_FB_ERR("pov_req is NULL Point!");
		return -EINVAL;
	}
	if ((layer == NULL) && (wb_layer == NULL)){
		HISI_FB_ERR("layer & wb_layer is NULL Point!");
		return -EINVAL;
	}
	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)){
		HISI_FB_ERR("ovl_idx(%d) is invalid!\n", ovl_idx);
		return -EINVAL;
	}
	/*lint -e613*/
	if (wb_layer != NULL) {
		chn_idx = wb_layer->chn_idx;
		need_cap = wb_layer->need_cap;
	} else {
		chn_idx = layer->chn_idx;
		need_cap = layer->need_cap;
	}
	/*lint +e613*/
	if (!(need_cap & (CAP_AFBCD | CAP_HFBCD | CAP_HEBCD))) {
		return 0;
	}

	aif1_bw = &(hisifd->dss_module.aif1_bw[chn_idx]);
	if (aif1_bw->is_used != 1) {
		HISI_FB_ERR("fb%d, aif1_bw->is_used=%d no equal to 1 is err!", hisifd->index, aif1_bw->is_used);
		return 0;
	}

	mid = mid_array[chn_idx];
	if (mid < 0 || mid > 0xb) {
		HISI_FB_ERR("fb%d, mid=%d is invalid!", hisifd->index, mid);
		return 0;
	}

	aif1 = &(hisifd->dss_module.aif1[chn_idx]);
	hisifd->dss_module.aif1_ch_used[chn_idx] = 1;

	aif1->aif_ch_ctl = set_bits32(aif1->aif_ch_ctl, aif1_bw->axi_sel, 1, 0);

	if (g_fpga_flag == 0) {
		if ((ovl_idx == DSS_OVL2) || (ovl_idx == DSS_OVL3)) {
			if (layer && ((layer->need_cap & CAP_AFBCD) ||
				(layer->need_cap & CAP_HFBCD) || (layer->need_cap & CAP_HEBCD))) {
				aif1->aif_ch_ctl = set_bits32(aif1->aif_ch_ctl, 0x1, 3, 8);
				aif1->aif_ch_ls = set_bits32(aif1->aif_ch_ls, 0x00040041, 32, 0);
			}
			if (wb_layer && ((wb_layer->need_cap & CAP_AFBCE)
				|| (wb_layer->need_cap & CAP_HFBCE) ||
				(wb_layer->need_cap & CAP_HEBCE))) {
				aif1->aif_ch_ctl = set_bits32(aif1->aif_ch_ctl, 0x1, 3, 8);
				aif1->aif_ch_ls = set_bits32(aif1->aif_ch_ls, 0x40041, 32, 0);
			}
		}
	}

	return 0;
}

/*******************************************************************************
** DSS SMMU
*/
void hisi_dss_smmu_init(char __iomem *smmu_base,
	dss_smmu_t *s_smmu)
{
	if (NULL == smmu_base) {
		HISI_FB_ERR("smmu_base is NULL");
		return;
	}
	if (NULL == s_smmu) {
		HISI_FB_ERR("s_smmu is NULL");
		return;
	}

	memset(s_smmu, 0, sizeof(dss_smmu_t));
}


void hisi_dss_smmu_on(struct hisi_fb_data_type *hisifd)
{
	char __iomem *smmu_base = NULL;
	int idx0 = 0;
	int idx1 = 0;
	int idx2 = 0;
	uint64_t smmu_rwerraddr_phys = 0;
	uint32_t fama_ptw_msb;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	smmu_base = hisifd->dss_base + DSS_SMMU_OFFSET;

	if (g_smmu_global_bypass) {
		set_reg(smmu_base + SMMU_SCR, 0x1, 1, 0);  //global bypass
	} else {
		set_reg(smmu_base + SMMU_SCR, 0x0, 1, 0);  //global bypass cancel
	}
	set_reg(smmu_base + SMMU_SCR, 0x1, 8, 20);   //ptw_mid

	set_reg(smmu_base + SMMU_SCR, g_dss_smmu_outstanding - 1, 4, 16);  //pwt_pf
	set_reg(smmu_base + SMMU_SCR, 0x7, 3, 3);  //interrupt cachel1 cach3l2 en
	set_reg(smmu_base + SMMU_LP_CTRL, 0x1, 1, 0);  //auto_clk_gt_en

	//Long Descriptor
	set_reg(smmu_base + SMMU_CB_TTBCR, 0x1, 1, 0);

	//RWERRADDR
	if (g_smmu_rwerraddr_virt != NULL) {
		smmu_rwerraddr_phys = virt_to_phys(g_smmu_rwerraddr_virt);
		set_reg(smmu_base + SMMU_ERR_RDADDR, (uint32_t)(smmu_rwerraddr_phys & 0xFFFFFFFF), 32, 0);
		set_reg(smmu_base + SMMU_ADDR_MSB, (uint32_t)((smmu_rwerraddr_phys >> 32) & 0xFFFF), 16, 0);
		set_reg(smmu_base + SMMU_ERR_WRADDR, (uint32_t)(smmu_rwerraddr_phys & 0xFFFFFFFF), 32, 0);
		set_reg(smmu_base + SMMU_ADDR_MSB, (uint32_t)((smmu_rwerraddr_phys >> 32) & 0xFFFF), 16, 16);
		HISI_FB_INFO("smmu_rwerraddr_phys_MSB = 0x%x\n",
			(uint32_t)((smmu_rwerraddr_phys >> 32) & 0xFFFF));
	} else {
		set_reg(smmu_base + SMMU_ERR_RDADDR, 0x7FF00000, 32, 0);
		//set_reg(smmu_base + SMMU_ADDR_MSB, 0x0, 2, 0);
		set_reg(smmu_base + SMMU_ERR_WRADDR, 0x7FFF0000, 32, 0);
		//set_reg(smmu_base + SMMU_ADDR_MSB, 0x0, 2, 2);
	}

	idx0 = 36; //debug stream id
	idx1 = 37; //cmd unsec stream id
	idx2 = 38; //cmd sec stream id
	//cmdlist stream bypass
	/*lint -e679*/
	set_reg(smmu_base + SMMU_SMRx_NS + idx0 * 0x4, 0x1d, 32, 0);
	set_reg(smmu_base + SMMU_SMRx_NS + idx1 * 0x4, 0x1d, 32, 0);
	set_reg(smmu_base + SMMU_SMRx_NS + idx2 * 0x4, 0x1d, 32, 0);
	/*lint +e679*/
	// disable cmdlist, dbg, reload
	set_reg(smmu_base + SMMU_RLD_EN0_NS, 0xffffffff, 32, 0);
	set_reg(smmu_base + SMMU_RLD_EN1_NS, 0xffffffcf, 32, 0);
	set_reg(smmu_base + SMMU_RLD_EN2_NS, 0x00ffffff, 32, 0);

	//TTBR0
	set_reg(smmu_base + SMMU_CB_TTBR0, (uint32_t)hisi_dss_domain_get_ttbr(), 32, 0);

#if defined (CONFIG_DRMDRIVER)
	configure_dss_service_security(DSS_SMMU_INIT, 0/*not used*/, 0/*not used*/);
#endif

	fama_ptw_msb = (hisi_dss_domain_get_ttbr() >> 32) & 0x7F;
	set_reg(smmu_base + SMMU_FAMA_CTRL0, 0x80, 14, 0);
	set_reg(smmu_base + SMMU_FAMA_CTRL1, fama_ptw_msb, 7, 0);
}

/*lint -e613 -e838 -e679*/
void hisi_mdc_smmu_on(struct hisi_fb_data_type *hisifd)
{
	char __iomem *smmu_base = NULL;
	int idx0 = 0;
	int idx1 = 0;
	int idx2 = 0;
	uint64_t smmu_rwerraddr_phys = 0;
	uint32_t fama_ptw_msb;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point.\n");
		return;
	}

	smmu_base = hisifd->media_common_base + VBIF0_SMMU_OFFSET;

	set_reg(smmu_base + SMMU_SCR, 0x0, 1, 0);  //global bypass cancel
	//set_reg(smmu_base + SMMU_SCR_S, 0x3, 2, 0);  //nscfg  using default value 0x3
	set_reg(smmu_base + SMMU_SCR, 0x1, 8, 20);   //ptw_mid
	set_reg(smmu_base + SMMU_SCR, g_dss_smmu_outstanding - 1, 4, 16);  //pwt_pf
	set_reg(smmu_base + SMMU_SCR, 0x7, 3, 3);  //interrupt cachel1 cach3l2 en
	set_reg(smmu_base + SMMU_LP_CTRL, 0x1, 1, 0);  //auto_clk_gt_en

	//Long Descriptor
	set_reg(smmu_base + SMMU_CB_TTBCR, 0x1, 1, 0);

	//RWERRADDR
	if (g_smmu_rwerraddr_virt != NULL) {
		smmu_rwerraddr_phys = virt_to_phys(g_smmu_rwerraddr_virt);

		set_reg(smmu_base + SMMU_ERR_RDADDR,
			(uint32_t)(smmu_rwerraddr_phys & 0xFFFFFFFF), 32, 0);
		//set_reg(smmu_base + SMMU_ADDR_MSB,
		//	(uint32_t)((smmu_rwerraddr_phys >> 32) & 0x3), 2, 0);
		set_reg(smmu_base + SMMU_ERR_WRADDR,
			(uint32_t)(smmu_rwerraddr_phys & 0xFFFFFFFF), 32, 0);
		//set_reg(smmu_base + SMMU_ADDR_MSB,
		//	(uint32_t)((smmu_rwerraddr_phys >> 32) & 0x3), 2, 2);
	} else {
		set_reg(smmu_base + SMMU_ERR_RDADDR, 0x7FF00000, 32, 0);
		//set_reg(smmu_base + SMMU_ADDR_MSB, 0x0, 2, 0);
		set_reg(smmu_base + SMMU_ERR_WRADDR, 0x7FFF0000, 32, 0);
		//set_reg(smmu_base + SMMU_ADDR_MSB, 0x0, 2, 2);
	}

	// disable cmdlist, dbg, reload
	set_reg(smmu_base + SMMU_RLD_EN0_NS, 0xffffffff, 32, 0);
	set_reg(smmu_base + SMMU_RLD_EN1_NS, 0xffffffcf, 32, 0);
	set_reg(smmu_base + SMMU_RLD_EN2_NS, 0x00ffffff, 32, 0);

	idx0 = 36; //debug stream id
	idx1 = 37; //cmd unsec stream id
	idx2 = 38; //cmd sec stream id

	//cmdlist stream bypass
	set_reg(smmu_base + SMMU_SMRx_NS + idx0 * 0x4, 0x1d, 32, 0);
	set_reg(smmu_base + SMMU_SMRx_NS + idx1 * 0x4, 0x1d, 32, 0);
	set_reg(smmu_base + SMMU_SMRx_NS + idx2 * 0x4, 0x1d, 32, 0);

	//TTBR0
	set_reg(smmu_base + SMMU_CB_TTBR0, (uint32_t)hisi_dss_domain_get_ttbr(), 32, 0);

	fama_ptw_msb = (hisi_dss_domain_get_ttbr() >> 32) & 0x7F;
	set_reg(smmu_base + SMMU_FAMA_CTRL0, 0x80, 14, 0);
	set_reg(smmu_base + SMMU_FAMA_CTRL1, fama_ptw_msb, 7, 0);
}
/*lint +e613 +e838 +e679*/

/*lint -e568 -e685 -e679 -e613 -e838*/
void hisi_dss_smmu_ch_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *smmu_base, dss_smmu_t *s_smmu, int chn_idx)
{
	uint32_t idx = 0;
	uint32_t i = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (smmu_base == NULL) {
		HISI_FB_ERR("smmu_base is null");
		return;
	}

	if (s_smmu == NULL) {
		HISI_FB_ERR("s_smmu is null");
		return;
	}

	if (s_smmu->smmu_smrx_ns_used[chn_idx] == 0)
		return;

	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			HISI_FB_ERR("idx is invalid");
			return;
		}
		hisifd->set_reg(hisifd, smmu_base + SMMU_SMRx_NS + idx * 0x4,
			s_smmu->smmu_smrx_ns[idx], 32, 0);
		if (hisifd->video_idle_ctrl.idle_frame_display)
			HISI_FB_DEBUG("idle display chn %d, smmu_smrx_ns[%d]=0x%x\n",
				chn_idx, idx, s_smmu->smmu_smrx_ns[idx]);

	}
}
int hisi_dss_smmu_ch_config(struct hisi_fb_data_type *hisifd,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer)
{
	dss_smmu_t *smmu = NULL;
	int chn_idx = 0;
	dss_img_t *img = NULL;
	uint32_t idx = 0;
	uint32_t i = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if ((layer == NULL) && (wb_layer == NULL)) {
		HISI_FB_ERR("layer or wb_layer is NULL");
		return -EINVAL;
	}

	if (wb_layer != NULL) {
		img = &(wb_layer->dst);
		chn_idx = wb_layer->chn_idx;
	} else {
		img = &(layer->img);
		chn_idx = layer->chn_idx;
	}

	smmu = &(hisifd->dss_module.smmu);
	hisifd->dss_module.smmu_used = 1;

	smmu->smmu_smrx_ns_used[chn_idx] = 1;

	for (i = 0; i < g_dss_chn_sid_num[chn_idx]; i++) {
		idx = g_dss_smmu_smrx_idx[chn_idx] + i;
		if (idx >= SMMU_SID_NUM) {
			HISI_FB_ERR("idx is invalid");
			return -EINVAL;
		}

		if (img->mmu_enable == 0) {
			smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 1, 0);
		} else {
			if (hisifd->video_idle_ctrl.idle_frame_display) {
				smmu->smmu_smrx_ns[idx] = inp32(hisifd->dss_base +
					DSS_SMMU_OFFSET + SMMU_SMRx_NS + idx * 0x4);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 1, 9);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 2, 7);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 2, 5);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x0, 1, 1);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 1, 4);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x3, 2, 2);
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x0, 1, 0);
				HISI_FB_DEBUG("idle display chn_%d, smmu_smrx_ns[%d]=0x%x\n",
					chn_idx, idx, smmu->smmu_smrx_ns[idx]);
			} else {
				/* stream config */
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x0, 1, 0);  //smr_bypass
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x3, 2, 2);  //smr_ptw_qos
				smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx], 0x1, 1, 4);  //smr_invld_en
				//smmu->smmu_smrx_ns[idx] = set_bits32(smmu->smmu_smrx_ns[idx],  , 20, 12);  //smr_offset_addr
			}
		}
	}
	return 0;
}
/*lint +e568 +e685 +e679 +e613 +e838*/
void hisifb_adjust_block_rect(int block_num, dss_rect_t *ov_block_rects[], dss_wb_layer_t *wb_layer)
{
	return ;
}

/*******************************************************************************
** DSS CSC
*/
#define CSC_ROW	(3)
#define CSC_COL	(5)

/*
** [ p00 p01 p02 idc2 odc2 ]
** [ p10 p11 p12 idc1 odc1 ]
** [ p20 p21 p22 idc0 odc0 ]
*/
static int CSC_COE_YUV2RGB601_WIDE[CSC_ROW][CSC_COL] = {
	{0x4000, 0x00000, 0x059ba, 0x000, 0x000},
	{0x4000, 0x1e9fa, 0x1d24c, 0x600, 0x000},
	{0x4000, 0x07168, 0x00000, 0x600, 0x000},
};

static int CSC_COE_RGB2YUV601_WIDE[CSC_ROW][CSC_COL] = {
	{0x01323, 0x02591, 0x0074c, 0x000, 0x000},
	{0x1f533, 0x1eacd, 0x02000, 0x000, 0x200},
	{0x02000, 0x1e534, 0x1facc, 0x000, 0x200},
};

static int CSC_COE_YUV2RGB601_NARROW[CSC_ROW][CSC_COL] = {
	{0x4a85, 0x00000, 0x06625, 0x7c0, 0x000},
	{0x4a85, 0x1e6ed, 0x1cbf8, 0x600, 0x000},
	{0x4a85, 0x0811a, 0x00000, 0x600, 0x000},
};

static int CSC_COE_RGB2YUV601_NARROW[CSC_ROW][CSC_COL] = {
	{0x0106f, 0x02044, 0x00644, 0x000, 0x040},
	{0x1f684, 0x1ed60, 0x01c1c, 0x000, 0x200},
	{0x01c1c, 0x1e876, 0x1fb6e, 0x000, 0x200},
};

static int CSC_COE_YUV2RGB709_WIDE[CSC_ROW][CSC_COL] = {
	{0x4000, 0x00000, 0x064ca, 0x000, 0x000},
	{0x4000, 0x1f403, 0x1e20a, 0x600, 0x000},
	{0x4000, 0x076c2, 0x00000, 0x600, 0x000},
};

static int CSC_COE_RGB2YUV709_WIDE[CSC_ROW][CSC_COL] = {
	{0x00d9b, 0x02dc6, 0x0049f, 0x000, 0x000},
	{0x1f8ab, 0x1e755, 0x02000, 0x000, 0x200},
	{0x02000, 0x1e2ef, 0x1fd11, 0x000, 0x200},
};

static int CSC_COE_YUV2RGB709_NARROW[CSC_ROW][CSC_COL] = {
	{0x4a85, 0x00000, 0x072bc, 0x7c0, 0x000},
	{0x4a85, 0x1f25a, 0x1dde5, 0x600, 0x000},
	{0x4a85, 0x08732, 0x00000, 0x600, 0x000},
};

static int CSC_COE_RGB2YUV709_NARROW[CSC_ROW][CSC_COL] = {
	{0x00baf, 0x02750, 0x003f8, 0x000, 0x040},
	{0x1f98f, 0x1ea55, 0x01c1c, 0x000, 0x200},
	{0x01c1c, 0x1e678, 0x1fd6c, 0x000, 0x200},
};

static int CSC_COE_RGB2YUV2020[CSC_ROW][CSC_COL] = {
	{0x04000, 0x00000, 0x00000, 0x00000, 0x00000},
	{0x00000, 0x04000, 0x00000, 0x00600, 0x00000},
	{0x00000, 0x00000, 0x04000, 0x00600, 0x00000},
};

static int CSC_COE_YUV2RGB2020[CSC_ROW][CSC_COL] = {
	{0x04A85, 0x00000, 0x06B6F, 0x007C0, 0x00000},
	{0x04A85, 0x1F402, 0x1D65F, 0x00600, 0x00000},
	{0x04A85, 0x08912, 0x00000, 0x00600, 0x00000},
};
/*lint -e732*/
void hisi_dss_csc_init(const char __iomem *csc_base, dss_csc_t *s_csc)
{
	if (NULL == csc_base) {
		HISI_FB_ERR("csc_base is NULL");
		return;
	}
	if (NULL == s_csc) {
		HISI_FB_ERR("s_csc is NULL");
		return;
	}

	memset(s_csc, 0, sizeof(dss_csc_t));

	s_csc->idc0 = inp32(csc_base + CSC_IDC0);
	s_csc->idc2 = inp32(csc_base + CSC_IDC2);
	s_csc->odc0 = inp32(csc_base + CSC_ODC0);
	s_csc->odc2 = inp32(csc_base + CSC_ODC2);

	s_csc->p00 = inp32(csc_base + CSC_P00);
	s_csc->p01 = inp32(csc_base + CSC_P01);
	s_csc->p02 = inp32(csc_base + CSC_P02);
	s_csc->p10 = inp32(csc_base + CSC_P10);
	s_csc->p11 = inp32(csc_base + CSC_P11);
	s_csc->p12 = inp32(csc_base + CSC_P12);
	s_csc->p20 = inp32(csc_base + CSC_P20);
	s_csc->p21 = inp32(csc_base + CSC_P21);
	s_csc->p22 = inp32(csc_base + CSC_P22);
	s_csc->icg_module = inp32(csc_base + CSC_ICG_MODULE);
}
/*lint +e732*/
void hisi_dss_csc_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *csc_base, dss_csc_t *s_csc)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (csc_base == NULL) {
		HISI_FB_ERR("csc_base is null");
		return;
	}

	if (s_csc == NULL) {
		HISI_FB_ERR("s_csc is null");
		return;
	}

	hisifd->set_reg(hisifd, csc_base + CSC_IDC0, s_csc->idc0, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_IDC2, s_csc->idc2, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_ODC0, s_csc->odc0, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_ODC2, s_csc->odc2, 32, 0);

	hisifd->set_reg(hisifd, csc_base + CSC_P00, s_csc->p00, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P01, s_csc->p01, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P02, s_csc->p02, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P10, s_csc->p10, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P11, s_csc->p11, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P12, s_csc->p12, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P20, s_csc->p20, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P21, s_csc->p21, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_P22, s_csc->p22, 32, 0);
	hisifd->set_reg(hisifd, csc_base + CSC_ICG_MODULE, s_csc->icg_module, 32, 0);
}

bool is_pcsc_needed(dss_layer_t *layer)
{
	if (layer->chn_idx != DSS_RCHN_V0)
		return false;

	if (layer->need_cap & CAP_2D_SHARPNESS)
		return true;

	/*horizental shrink is not supported by arsr2p */
	if ((layer->dst_rect.h != layer->src_rect.h) || (layer->dst_rect.w > layer->src_rect.w))
		return true;

	return false;
}
/*lint -e701 -e732 */
int hisi_dss_csc_config(struct hisi_fb_data_type *hisifd,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer)
{
	dss_csc_t *csc = NULL;
	int chn_idx = 0;
	uint32_t format = 0;
	uint32_t csc_mode = 0;
	int (*csc_coe_yuv2rgb)[CSC_COL];
	int (*csc_coe_rgb2yuv)[CSC_COL];

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Point!");
		return -EINVAL;
	}

	if (wb_layer != NULL) {
		chn_idx = wb_layer->chn_idx;
		format = wb_layer->dst.format;
		csc_mode = wb_layer->dst.csc_mode;
	} else {
		if (layer != NULL) {
			chn_idx = layer->chn_idx;
			format = layer->img.format;
			csc_mode = layer->img.csc_mode;
		}
	}

	if ((chn_idx == DSS_RCHN_V0) && (hisifd->index == MEDIACOMMON_PANEL_IDX)) {
		return 0;
	}

	if (chn_idx != DSS_RCHN_V0) {
		if (!isYUV(format))
			return 0;
		hisifd->dss_module.csc_used[chn_idx] = 1;
	} else if ((chn_idx == DSS_RCHN_V0) && (!isYUV(format))){ //v0, rgb format
		if (layer != NULL) {
			if (!is_pcsc_needed(layer)) {
				return 0;
			}
		}

		hisifd->dss_module.csc_used[DSS_RCHN_V0] = 1;
		hisifd->dss_module.pcsc_used[DSS_RCHN_V0] = 1;
	} else {//v0, yuv format
		hisifd->dss_module.csc_used[chn_idx] = 1;
	}

	csc_coe_yuv2rgb = CSC_COE_YUV2RGB601_NARROW;
	csc_coe_rgb2yuv = CSC_COE_RGB2YUV601_NARROW;
	if (csc_mode == DSS_CSC_601_WIDE) {
		csc_coe_yuv2rgb = CSC_COE_YUV2RGB601_WIDE;
		csc_coe_rgb2yuv = CSC_COE_RGB2YUV601_WIDE;
	} else if (csc_mode == DSS_CSC_601_NARROW) {
		csc_coe_yuv2rgb = CSC_COE_YUV2RGB601_NARROW;
		csc_coe_rgb2yuv = CSC_COE_RGB2YUV601_NARROW;
	} else if (csc_mode == DSS_CSC_709_WIDE) {
		csc_coe_yuv2rgb = CSC_COE_YUV2RGB709_WIDE;
		csc_coe_rgb2yuv = CSC_COE_RGB2YUV709_WIDE;
	} else if (csc_mode == DSS_CSC_709_NARROW) {
		csc_coe_yuv2rgb = CSC_COE_YUV2RGB709_NARROW;
		csc_coe_rgb2yuv = CSC_COE_RGB2YUV709_NARROW;
	} else if (csc_mode == DSS_CSC_2020) {
		csc_coe_yuv2rgb = CSC_COE_YUV2RGB2020;
		csc_coe_rgb2yuv = CSC_COE_RGB2YUV2020;
	}

	/* config rch csc */
	if (layer != NULL && hisifd->dss_module.csc_used[chn_idx]) {
		csc = &(hisifd->dss_module.csc[chn_idx]);
		csc->icg_module = set_bits32(csc->icg_module, 0x1, 1, 0);

		csc->idc0 = set_bits32(csc->idc0, (uint32_t)csc_coe_yuv2rgb[2][3] | ((uint32_t)csc_coe_yuv2rgb[1][3] << 16), 27, 0);
		csc->idc2 = set_bits32(csc->idc2, csc_coe_yuv2rgb[0][3], 11, 0);

		csc->odc0 = set_bits32(csc->odc0, (uint32_t)csc_coe_yuv2rgb[2][4] | ((uint32_t)csc_coe_yuv2rgb[1][4] << 16), 27, 0);
		csc->odc2 = set_bits32(csc->odc2, csc_coe_yuv2rgb[0][4], 11, 0);

		csc->p00 = set_bits32(csc->p00, csc_coe_yuv2rgb[0][0], 17, 0);
		csc->p01 = set_bits32(csc->p01, csc_coe_yuv2rgb[0][1], 17, 0);
		csc->p02 = set_bits32(csc->p02, csc_coe_yuv2rgb[0][2], 17, 0);

		csc->p10 = set_bits32(csc->p10, csc_coe_yuv2rgb[1][0], 17, 0);
		csc->p11 = set_bits32(csc->p11, csc_coe_yuv2rgb[1][1], 17, 0);
		csc->p12 = set_bits32(csc->p12, csc_coe_yuv2rgb[1][2], 17, 0);

		csc->p20 = set_bits32(csc->p20, csc_coe_yuv2rgb[2][0], 17, 0);
		csc->p21 = set_bits32(csc->p21, csc_coe_yuv2rgb[2][1], 17, 0);
		csc->p22 = set_bits32(csc->p22, csc_coe_yuv2rgb[2][2], 17, 0);
	}

	/* config rch pcsc */
	if (layer != NULL && hisifd->dss_module.pcsc_used[chn_idx]) {
		csc = &(hisifd->dss_module.pcsc[chn_idx]);
		csc->icg_module = set_bits32(csc->icg_module, 0x1, 1, 0);

		csc->idc0 = set_bits32(csc->idc0, (uint32_t)csc_coe_rgb2yuv[2][3] | ((uint32_t)csc_coe_rgb2yuv[1][3] << 16), 27, 0);
		csc->idc2 = set_bits32(csc->idc2, csc_coe_rgb2yuv[0][3], 11, 0);

		csc->odc0 = set_bits32(csc->odc0, (uint32_t)csc_coe_rgb2yuv[2][4] | ((uint32_t)csc_coe_rgb2yuv[1][4] << 16), 27, 0);
		csc->odc2 = set_bits32(csc->odc2, csc_coe_rgb2yuv[0][4], 11, 0);

		csc->p00 = set_bits32(csc->p00, csc_coe_rgb2yuv[0][0], 17, 0);
		csc->p01 = set_bits32(csc->p01, csc_coe_rgb2yuv[0][1], 17, 0);
		csc->p02 = set_bits32(csc->p02, csc_coe_rgb2yuv[0][2], 17, 0);

		csc->p10 = set_bits32(csc->p10, csc_coe_rgb2yuv[1][0], 17, 0);
		csc->p11 = set_bits32(csc->p11, csc_coe_rgb2yuv[1][1], 17, 0);
		csc->p12 = set_bits32(csc->p12, csc_coe_rgb2yuv[1][2], 17, 0);

		csc->p20 = set_bits32(csc->p20, csc_coe_rgb2yuv[2][0], 17, 0);
		csc->p21 = set_bits32(csc->p21, csc_coe_rgb2yuv[2][1], 17, 0);
		csc->p22 = set_bits32(csc->p22, csc_coe_rgb2yuv[2][2], 17, 0);
	}

	/* config wch csc */
	if (wb_layer != NULL && hisifd->dss_module.csc_used[chn_idx]) {
		csc = &(hisifd->dss_module.csc[chn_idx]);
		csc->icg_module = set_bits32(csc->icg_module, 0x1, 1, 0);

		csc->idc0 = set_bits32(csc->idc0, (uint32_t)csc_coe_rgb2yuv[2][3] | ((uint32_t)csc_coe_rgb2yuv[1][3] << 16), 27, 0);
		csc->idc2 = set_bits32(csc->idc2, csc_coe_rgb2yuv[0][3], 11, 0);

		csc->odc0 = set_bits32(csc->odc0, (uint32_t)csc_coe_rgb2yuv[2][4] | ((uint32_t)csc_coe_rgb2yuv[1][4] << 16), 27, 0);
		csc->odc2 = set_bits32(csc->odc2, csc_coe_rgb2yuv[0][4], 11, 0);

		csc->p00 = set_bits32(csc->p00, csc_coe_rgb2yuv[0][0], 17, 0);
		csc->p01 = set_bits32(csc->p01, csc_coe_rgb2yuv[0][1], 17, 0);
		csc->p02 = set_bits32(csc->p02, csc_coe_rgb2yuv[0][2], 17, 0);

		csc->p10 = set_bits32(csc->p10, csc_coe_rgb2yuv[1][0], 17, 0);
		csc->p11 = set_bits32(csc->p11, csc_coe_rgb2yuv[1][1], 17, 0);
		csc->p12 = set_bits32(csc->p12, csc_coe_rgb2yuv[1][2], 17, 0);

		csc->p20 = set_bits32(csc->p20, csc_coe_rgb2yuv[2][0], 17, 0);
		csc->p21 = set_bits32(csc->p21, csc_coe_rgb2yuv[2][1], 17, 0);
		csc->p22 = set_bits32(csc->p22, csc_coe_rgb2yuv[2][2], 17, 0);
	}

	return 0;
}
/*lint +e701 +e732 */
/*lint -e679 -e730 -e732*/
void hisi_dss_ovl_init(const char __iomem *ovl_base, dss_ovl_t *s_ovl, int ovl_idx)
{
	int i = 0;

	if (NULL == ovl_base) {
		HISI_FB_ERR("ovl_base is NULL");
		return;
	}
	if (NULL == s_ovl) {
		HISI_FB_ERR("s_ovl is NULL");
		return;
	}

	memset(s_ovl, 0, sizeof(dss_ovl_t));

	s_ovl->ovl_size = inp32(ovl_base + OV_SIZE);
	s_ovl->ovl_bg_color = inp32(ovl_base + OV_BG_COLOR_RGB);
	s_ovl->ovl_bg_color_alpha = inp32(ovl_base + OV_BG_COLOR_A);
	s_ovl->ovl_dst_startpos = inp32(ovl_base + OV_DST_STARTPOS);
	s_ovl->ovl_dst_endpos = inp32(ovl_base + OV_DST_ENDPOS);
	s_ovl->ovl_gcfg = inp32(ovl_base + OV_GCFG);

	if (ovl_idx == DSS_OVL3) {
		for (i = 0; i < OVL_2LAYER_NUM; i++) {
			s_ovl->ovl_layer[i].layer_pos =
				inp32(ovl_base + OV_LAYER0_POS + i * 0x60);
			s_ovl->ovl_layer[i].layer_size =
				inp32(ovl_base + OV_LAYER0_SIZE + i * 0x60);
			s_ovl->ovl_layer[i].layer_pattern =
				inp32(ovl_base + OV_LAYER0_PATTERN_RGB + i * 0x60);
			s_ovl->ovl_layer[i].layer_pattern_alpha=
				inp32(ovl_base + OV_LAYER0_PATTERN_A + i * 0x60);
			s_ovl->ovl_layer[i].layer_alpha =
				inp32(ovl_base + OV_LAYER0_ALPHA_MODE + i * 0x60);
			s_ovl->ovl_layer[i].layer_alpha_a =
				inp32(ovl_base + OV_LAYER0_ALPHA_A + i * 0x60);
			s_ovl->ovl_layer[i].layer_cfg =
				inp32(ovl_base + OVL_LAYER0_CFG + i * 0x60);
			s_ovl->ovl_layer_pos[i].layer_pspos =
				inp32(ovl_base + OVL_LAYER0_PSPOS + i * 0x60);
			s_ovl->ovl_layer_pos[i].layer_pepos =
				inp32(ovl_base + OVL_LAYER0_PEPOS + i * 0x60);
		}

		s_ovl->ovl_block_size = inp32(ovl_base + OV2_BLOCK_SIZE);
	} else {
		for (i = 0; i < OV_8LAYER_NUM; i++) {
			s_ovl->ovl_layer[i].layer_pos =
				inp32(ovl_base + OV_LAYER0_POS + i * 0x60);
			s_ovl->ovl_layer[i].layer_size =
				inp32(ovl_base + OV_LAYER0_SIZE + i * 0x60);
			s_ovl->ovl_layer[i].layer_pattern =
				inp32(ovl_base + OV_LAYER0_PATTERN_RGB + i * 0x60);
			s_ovl->ovl_layer[i].layer_pattern_alpha=
				inp32(ovl_base + OV_LAYER0_PATTERN_A + i * 0x60);
			s_ovl->ovl_layer[i].layer_alpha =
				inp32(ovl_base + OV_LAYER0_ALPHA_MODE + i * 0x60);
			s_ovl->ovl_layer[i].layer_alpha_a =
				inp32(ovl_base + OV_LAYER0_ALPHA_A + i * 0x60);
			s_ovl->ovl_layer[i].layer_cfg =
				inp32(ovl_base + OV_LAYER0_CFG + i * 0x60);
			s_ovl->ovl_layer_pos[i].layer_pspos =
				inp32(ovl_base + OV_LAYER0_PSPOS + i * 0x60);
			s_ovl->ovl_layer_pos[i].layer_pepos =
				inp32(ovl_base + OV_LAYER0_PEPOS + i * 0x60);
		}

		s_ovl->ovl_block_size = inp32(ovl_base + OV8_BLOCK_SIZE);
	}
}

void hisi_dss_ovl_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *ovl_base, dss_ovl_t *s_ovl, int ovl_idx)
{
	int i = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (ovl_base == NULL) {
		HISI_FB_ERR("ovl_base is null");
		return;
	}

	if (s_ovl == NULL) {
		HISI_FB_ERR("s_ovl is null");
		return;
	}

	if (ovl_idx == DSS_OVL3) {
		hisifd->set_reg(hisifd, ovl_base + OV2_REG_DEFAULT, 0x1, 32, 0);
		hisifd->set_reg(hisifd, ovl_base + OV2_REG_DEFAULT, 0x0, 32, 0);
		hisifd->set_reg(hisifd, ovl_base + OV2_BLOCK_DBG, 0x4, 32, 0);
	} else {
		hisifd->set_reg(hisifd, ovl_base + OV8_REG_DEFAULT, 0x1, 32, 0);
		hisifd->set_reg(hisifd, ovl_base + OV8_REG_DEFAULT, 0x0, 32, 0);
		hisifd->set_reg(hisifd, ovl_base + OV8_BLOCK_DBG, 0x4, 32, 0);
	}

	hisifd->set_reg(hisifd, ovl_base + OV_SIZE, s_ovl->ovl_size, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_BG_COLOR_RGB, s_ovl->ovl_bg_color, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_BG_COLOR_A, s_ovl->ovl_bg_color_alpha, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_DST_STARTPOS, s_ovl->ovl_dst_startpos, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_DST_ENDPOS, s_ovl->ovl_dst_endpos, 32, 0);
	hisifd->set_reg(hisifd, ovl_base + OV_GCFG, s_ovl->ovl_gcfg, 32, 0);

	if (ovl_idx == DSS_OVL3) {
		for (i = 0; i < OVL_2LAYER_NUM; i++) {
			if (s_ovl->ovl_layer_used[i] == 1) {
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_POS + i * 0x60,
					s_ovl->ovl_layer[i].layer_pos, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_SIZE + i * 0x60,
					s_ovl->ovl_layer[i].layer_size, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PATTERN_RGB + i * 0x60,
					s_ovl->ovl_layer[i].layer_pattern, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PATTERN_A + i * 0x60,
					s_ovl->ovl_layer[i].layer_pattern_alpha, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_ALPHA_MODE + i * 0x60,
					s_ovl->ovl_layer[i].layer_alpha, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_ALPHA_A + i * 0x60,
					s_ovl->ovl_layer[i].layer_alpha_a, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_CFG + i * 0x60,
					s_ovl->ovl_layer[i].layer_cfg, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PSPOS + i * 0x60,
					s_ovl->ovl_layer_pos[i].layer_pspos, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PEPOS + i * 0x60,
					s_ovl->ovl_layer_pos[i].layer_pepos, 32, 0);
			} else {
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_POS + i * 0x60,
					s_ovl->ovl_layer[i].layer_pos, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_SIZE + i * 0x60,
					s_ovl->ovl_layer[i].layer_size, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_CFG + i * 0x60,
					s_ovl->ovl_layer[i].layer_cfg, 32, 0);
			}
		}

		hisifd->set_reg(hisifd, ovl_base + OV2_BLOCK_SIZE, s_ovl->ovl_block_size, 32, 0);
	} else {
		for (i = 0; i < OV_8LAYER_NUM; i++) {
			if (s_ovl->ovl_layer_used[i] == 1) {
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_POS + i * 0x60,
					s_ovl->ovl_layer[i].layer_pos, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_SIZE + i * 0x60,
					s_ovl->ovl_layer[i].layer_size, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PATTERN_RGB + i * 0x60,
					s_ovl->ovl_layer[i].layer_pattern, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PATTERN_A + i * 0x60,
					s_ovl->ovl_layer[i].layer_pattern_alpha, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_ALPHA_MODE + i * 0x60,
					s_ovl->ovl_layer[i].layer_alpha, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_ALPHA_A + i * 0x60,
					s_ovl->ovl_layer[i].layer_alpha_a, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_CFG + i * 0x60,
					s_ovl->ovl_layer[i].layer_cfg, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PSPOS + i * 0x60,
					s_ovl->ovl_layer_pos[i].layer_pspos, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_PEPOS + i * 0x60,
					s_ovl->ovl_layer_pos[i].layer_pepos, 32, 0);
			} else {
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_POS + i * 0x60,
					s_ovl->ovl_layer[i].layer_pos, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_SIZE + i * 0x60,
					s_ovl->ovl_layer[i].layer_size, 32, 0);
				hisifd->set_reg(hisifd, ovl_base + OV_LAYER0_CFG + i * 0x60,
					s_ovl->ovl_layer[i].layer_cfg, 32, 0);
			}
		}

		hisifd->set_reg(hisifd, ovl_base + OV8_BLOCK_SIZE, s_ovl->ovl_block_size, 32, 0);
	}
}
/*lint +e679 +e730 +e732*/

void hisi_dss_ov_set_reg_default_value(struct hisi_fb_data_type *hisifd,
	char __iomem *ovl_base, int ovl_idx)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (ovl_base == NULL) {
		HISI_FB_ERR("ovl_base is null");
		return;
	}

	if (ovl_idx == DSS_OVL3) {
		hisifd->set_reg(hisifd, ovl_base + OV2_REG_DEFAULT, 0x1, 32, 0);
		hisifd->set_reg(hisifd, ovl_base + OV2_REG_DEFAULT, 0x0, 32, 0);
	} else {
		hisifd->set_reg(hisifd, ovl_base + OV8_REG_DEFAULT, 0x1, 32, 0);
		hisifd->set_reg(hisifd, ovl_base + OV8_REG_DEFAULT, 0x0, 32, 0);
	}
}
/*lint -e838*/
uint32_t hisi_dss_mif_get_invalid_sel(dss_img_t *img, uint32_t transform, int v_scaling_factor,
	uint8_t is_tile, bool rdma_stretch_enable)
{
	uint32_t invalid_sel_val = 0;
	uint32_t tlb_tag_org = 0;

	if (img == NULL) {
		HISI_FB_ERR("img is null");
		return 0;
	}

	if ((transform == (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H))
		|| (transform == (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V))) {
		transform = HISI_FB_TRANSFORM_ROT_90;
	}

	tlb_tag_org =  (transform & 0x7) |
		((is_tile ? 1 : 0) << 3) | ((rdma_stretch_enable ? 1 : 0) << 4);

	switch (tlb_tag_org) {
		case MMU_TLB_TAG_ORG_0x0:
			invalid_sel_val = 1;
			break;
		case MMU_TLB_TAG_ORG_0x1:
			invalid_sel_val = 1;
			break;
		case MMU_TLB_TAG_ORG_0x2:
			invalid_sel_val = 2;
			break;
		case MMU_TLB_TAG_ORG_0x3:
			invalid_sel_val = 2;
			break;
		case MMU_TLB_TAG_ORG_0x4:
			invalid_sel_val = 0;
			break;
		case MMU_TLB_TAG_ORG_0x7:
			invalid_sel_val = 0;
			break;

		case MMU_TLB_TAG_ORG_0x8:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0x9:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0xA:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0xB:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0xC:
			invalid_sel_val = 0;
			break;
		case MMU_TLB_TAG_ORG_0xF:
			invalid_sel_val = 0;
			break;

		case MMU_TLB_TAG_ORG_0x10:
			invalid_sel_val = 1;
			break;
		case MMU_TLB_TAG_ORG_0x11:
			invalid_sel_val = 1;
			break;
		case MMU_TLB_TAG_ORG_0x12:
			invalid_sel_val = 2;
			break;
		case MMU_TLB_TAG_ORG_0x13:
			invalid_sel_val = 2;
			break;
		case MMU_TLB_TAG_ORG_0x14:
			invalid_sel_val = 0;
			break;
		case MMU_TLB_TAG_ORG_0x17:
			invalid_sel_val = 0;
			break;

		case MMU_TLB_TAG_ORG_0x18:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0x19:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0x1A:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0x1B:
			invalid_sel_val = 3;
			break;
		case MMU_TLB_TAG_ORG_0x1C:
			invalid_sel_val = 0;
			break;
		case MMU_TLB_TAG_ORG_0x1F:
			invalid_sel_val = 0;
			break;

		default:
			invalid_sel_val = 0;
			HISI_FB_ERR("not support this tlb_tag_org(0x%x)!\n", tlb_tag_org);
			break;
	}

	return invalid_sel_val;
}
/*lint +e838*/
/*******************************************************************************
** DSS ARSR2P
*/
#define ARSR2P_PHASE_NUM	(9)
#define ARSR2P_TAP4	(4)
#define ARSR2P_TAP6	(6)
#define ARSR2P_MIN_INPUT (16)
#define ARSR2P_MAX_WIDTH (2560)
#define ARSR2P_MAX_HEIGHT (8192)
#define ARSR2P_SCALE_MAX (60)


#define ARSR2P_SCL_UP_OFFSET (0x48)
#define ARSR2P_COEF_H0_OFFSET (0x100)
#define ARSR2P_COEF_H1_OFFSET (0x200)

#define ARSR1P_COEF_OFFSET (0x24)

#define LSC_ROW	(2)
#define LSC_COL (27)
//arsr1p lsc gain
static const uint32_t ARSR1P_LSC_GAIN_TABLE[LSC_ROW][LSC_COL] = {
	{1024, 1085, 1158, 1232, 1305, 1382, 1454, 1522, 1586, 1646, 1701, 1755, 1809, 1864, 1926, 1989, 2058, 2131, 2207, 2291, 2376, 2468, 2576, 2687, 2801, 2936, 3038}, //pgainlsc0
	{1052, 1122, 1192, 1268, 1345, 1418, 1488, 1554, 1616, 1674, 1728, 1783, 1838, 1895, 1957, 2023, 2089, 2165, 2245, 2331, 2424, 2523, 2629, 2744, 2866, 3006, 3038}  //pgainlsc1
};

//c0, c1, c2, c3
static const int COEF_AUV_SCL_UP_TAP4[ARSR2P_PHASE_NUM][ARSR2P_TAP4] = {
	{ -3, 254, 6, -1},
	{ -9, 255, 13, -3},
	{ -18, 254, 27, -7},
	{ -23, 245, 44, -10},
	{ -27, 233, 64, -14},
	{ -29, 218, 85, -18},
	{ -29, 198, 108, -21},
	{ -29, 177, 132, -24},
	{ -27, 155, 155, -27}
};

//c0, c1, c2, c3
static const int COEF_AUV_SCL_DOWN_TAP4[ARSR2P_PHASE_NUM][ARSR2P_TAP4] = {
	{ 31, 194, 31, 0},
	{ 23, 206, 44, -17},
	{ 14, 203, 57, -18},
	{ 6, 198, 70, -18},
	{ 0, 190, 85, -19},
	{ -5, 180, 99, -18},
	{ -10, 170, 114, -18},
	{ -13, 157, 129, -17},
	{ -15, 143, 143, -15}
};

//c0, c1, c2, c3, c4, c5
static const int COEF_Y_SCL_UP_TAP6[ARSR2P_PHASE_NUM][ARSR2P_TAP6] = {
	{ 0, -3, 254, 6, -1, 0},
	{ 4, -12, 252, 15, -5, 2},
	{ 7, -22, 245, 31, -9, 4},
	{ 10, -29, 234, 49, -14, 6},
	{ 12, -34, 221, 68, -19, 8},
	{ 13, -37, 206, 88, -24, 10},
	{ 14, -38, 189, 108, -29, 12},
	{ 14, -38, 170, 130, -33, 13},
	{ 14, -36, 150, 150, -36, 14}
};

static const int COEF_Y_SCL_DOWN_TAP6[ARSR2P_PHASE_NUM][ARSR2P_TAP6] = {
	{ -22, 43, 214, 43, -22, 0},
	{ -18, 29, 205, 53, -23, 10},
	{ -16, 18, 203, 67, -25, 9},
	{ -13, 9, 198, 80, -26, 8},
	{ -10, 0, 191, 95, -27, 7},
	{ -7, -7, 182, 109, -27, 6},
	{ -5, -14, 174, 124, -27, 4},
	{ -2, -18, 162, 137, -25, 2},
	{ 0, -22, 150, 150, -22, 0}
};

/*******************************************************************************
** DSS ARSR
*/

static int hisi_dss_arsr_post_write_coefs(struct hisi_fb_data_type *hisifd, char __iomem *addr, const int **p, int row, int col)
{
	int coef_value = 0;
	int coef_num;
	int i;
	int j;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if (NULL == addr) {
		HISI_FB_ERR("addr is NULL");
		return -EINVAL;
	}

	if ((row != ARSR2P_PHASE_NUM) || ((col != ARSR2P_TAP4) && (col != ARSR2P_TAP6))) {
		HISI_FB_ERR("arsr1p filter coefficients is err, arsr1p_phase_num = %d, arsr1p_tap_num = %d\n", row, col);
		return -EINVAL;
	}
	/*lint -e679 -e732 -e838*/
	coef_num = (col == ARSR2P_TAP4 ? 2 : 3);
	for (i = 0; i < row; i++) {
		for (j = 0; j < 2; j++) {
			if (coef_num == 2) {
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) | ((*((int *)p + i * col + j * coef_num + 1)	& 0x1FF) << 9);
			} else {
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) | ((*((int *)p + i * col + j * coef_num + 1)	& 0x1FF) << 9) | ((*((int *)p + i * col + j * coef_num + 2)  & 0x1FF) << 18);
			}
			set_reg(addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
		}
	}
	/*lint +e679 +e732 +e838*/
	return 0;
}

static void hisi_dss_arsr_post_write_config_coefs(struct hisi_fb_data_type *hisifd,
	char __iomem *addr, const int **scl_down, const int **scl_up, int row, int col)
{
	int ret;

	ret = hisi_dss_arsr_post_write_coefs(hisifd, addr, scl_down, row, col);
	if (ret < 0) {
		HISI_FB_ERR("Error to write arsr post COEF_SCL_DOWN coefficients.\n");
		return;
	}

	ret = hisi_dss_arsr_post_write_coefs(hisifd, addr + ARSR2P_SCL_UP_OFFSET, scl_up, row, col);
	if (ret < 0) {
		HISI_FB_ERR("Error to write arsr post COEF_SCL_UP coefficients.\n");
		return;
	}
}

void hisi_dss_arsr_post_coef_on(struct hisi_fb_data_type *hisifd)
{
	uint32_t arsr_post_lut_base;
	char __iomem *coefy_v;
	char __iomem *coefa_v;
	char __iomem *coefuv_v;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	arsr_post_lut_base = ARSR_POST_LUT_OFFSET;
	coefy_v = hisifd->dss_base + arsr_post_lut_base + ARSR2P_LUT_COEFY_V_OFFSET;
	coefa_v = hisifd->dss_base + arsr_post_lut_base + ARSR2P_LUT_COEFA_V_OFFSET;
	coefuv_v = hisifd->dss_base + arsr_post_lut_base + ARSR2P_LUT_COEFUV_V_OFFSET;

	/* COEFY_V COEFY_H */
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefy_v, (const int **)COEF_Y_SCL_DOWN_TAP6, (const int **)COEF_Y_SCL_UP_TAP6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefy_v + ARSR2P_COEF_H0_OFFSET, (const int **)COEF_Y_SCL_DOWN_TAP6, (const int **)COEF_Y_SCL_UP_TAP6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefy_v + ARSR2P_COEF_H1_OFFSET, (const int **)COEF_Y_SCL_DOWN_TAP6, (const int **)COEF_Y_SCL_UP_TAP6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);

	/* COEFA_V COEFA_H */
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefa_v, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefa_v + ARSR2P_COEF_H0_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefa_v + ARSR2P_COEF_H1_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);

	/* COEFUV_V COEFUV_H */
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefuv_v, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefuv_v + ARSR2P_COEF_H0_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr_post_write_config_coefs(hisifd,  coefuv_v + ARSR2P_COEF_H1_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
}

int hisi_dss_arsr2p_write_coefs(struct hisi_fb_data_type *hisifd, bool enable_cmdlist,
	char __iomem *addr, const int **p, int row, int col)
{
	int coef_value = 0;
	int coef_num = 0;
	int i = 0;
	int j = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if (NULL == addr) {
		HISI_FB_ERR("addr is NULL");
		return -EINVAL;
	}

	if ((row != ARSR2P_PHASE_NUM) || ((col != ARSR2P_TAP4) && (col != ARSR2P_TAP6))) {
		HISI_FB_ERR("arsr2p filter coefficients is err, arsr2p_phase_num = %d, arsr2p_tap_num = %d\n", row, col);
		return -EINVAL;
	}
	/*lint -e679 -e732 -e838*/
	coef_num = (col == ARSR2P_TAP4 ? 2 : 3);

	for (i = 0; i < row; i++) {
		for (j = 0; j < 2; j++) {
			if (coef_num == 2) {
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) | ((*((int *)p + i * col + j * coef_num + 1)  & 0x1FF) << 9);
			} else {
				coef_value = (*((int *)p + i * col + j * coef_num) & 0x1FF) | ((*((int *)p + i * col + j * coef_num + 1)  & 0x1FF) << 9) | ((*((int *)p + i * col + j * coef_num + 2)  & 0x1FF) << 18);
			}

			if (enable_cmdlist) {
				hisifd->set_reg(hisifd, addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
			} else {
				set_reg(addr + 0x8 * i + j * 0x4, coef_value, 32, 0);
			}
		}
	}
	/*lint +e679 +e732 +e838*/
	return 0;
}
/*lint -e838*/
void hisi_dss_arsr2p_write_config_coefs(struct hisi_fb_data_type *hisifd, bool enable_cmdlist,
	char __iomem *addr, const int **scl_down, const int **scl_up, int row, int col)
{
	int ret = 0;

	ret = hisi_dss_arsr2p_write_coefs(hisifd, enable_cmdlist, addr, scl_down, row, col);
	if (ret < 0) {
		HISI_FB_ERR("Error to write COEF_SCL_DOWN coefficients.\n");
		return;
	}

	ret = hisi_dss_arsr2p_write_coefs(hisifd, enable_cmdlist, addr + ARSR2P_SCL_UP_OFFSET, scl_up, row, col);
	if (ret < 0) {
		HISI_FB_ERR("Error to write COEF_SCL_UP coefficients.\n");
		return;
	}

}
/*lint +e838*/

#define HIGH16(val) (val << 16)
#define LOW16(val) (val)
static void arsr_pre_para_init(struct arsr2p_info *arsr_para)
{
    if (arsr_para == NULL) {
        return;
    }

    arsr_para->skin_thres_y = (600 << 20) | (332 << 10) | 300; // 0x2585312C
    arsr_para->skin_thres_u = (452 << 20) | (40 << 10) | 20; // 0x1C40A014
    arsr_para->skin_thres_v = (580 << 20) | (48 << 10) | 24; // 0x2440C018
    arsr_para->skin_cfg0 = (12 << 13) | 512; // 0x00018200
    arsr_para->skin_cfg1 = 819; // 0x00000333
    arsr_para->skin_cfg2 = 682; // 0x000002AA
    arsr_para->shoot_cfg1 = (20 << 16) | 341; // 0x00140155
    arsr_para->shoot_cfg2 = (-80 & 0x7ff) | (16 << 16); // 0x001007B0
    arsr_para->shoot_cfg3 = 20; // 0x00000014
    arsr_para->sharp_cfg3 = HIGH16(0xA0) | LOW16(0x60); // 0x00A00060
    arsr_para->sharp_cfg4 = HIGH16(0x60) | LOW16(0x20); // 0x00600020
    //arsr_para->sharp_cfg5 = 0;
    arsr_para->sharp_cfg6 = HIGH16(0x4) | LOW16(0x8); // 0x00040008
    arsr_para->sharp_cfg7 = (6 << 8) | 10; // 0x0000060A
    arsr_para->sharp_cfg8 = HIGH16(0xA0) | LOW16(0x10); // 0x00A00010

    arsr_para->sharp_level = 0x0020002;
    arsr_para->sharp_gain_low = 0x3C0078;
    arsr_para->sharp_gain_mid = 0x6400C8;
    arsr_para->sharp_gain_high = 0x5000A0;
    arsr_para->sharp_gainctrl_sloph_mf = 0x280;
    arsr_para->sharp_gainctrl_slopl_mf = 0x1400;
    arsr_para->sharp_gainctrl_sloph_hf = 0x140;
    arsr_para->sharp_gainctrl_slopl_hf = 0xA00;
    arsr_para->sharp_mf_lmt = 0x40;
    arsr_para->sharp_gain_mf = 0x12C012C;
    arsr_para->sharp_mf_b = 0;
    arsr_para->sharp_hf_lmt = 0x80;
    arsr_para->sharp_gain_hf = 0x104012C;
    arsr_para->sharp_hf_b = 0x1400;
    arsr_para->sharp_lf_ctrl = 0x100010;
    arsr_para->sharp_lf_var = 0x1800080;
    arsr_para->sharp_lf_ctrl_slop = 0;
    arsr_para->sharp_hf_select = 0;
    arsr_para->sharp_cfg2_h = 0x10000C0;
    arsr_para->sharp_cfg2_l = 0x200010;
    arsr_para->texture_analysis = 0x500040;
    arsr_para->intplshootctrl = 0x8;
}

/*lint -e570 -e732*/
void hisi_dss_arsr2p_init(const char __iomem *arsr2p_base, dss_arsr2p_t *s_arsr2p)
{
	if (NULL == arsr2p_base) {
		HISI_FB_ERR("arsr2p_base is NULL");
		return;
	}
	if (NULL == s_arsr2p) {
		HISI_FB_ERR("s_arsr2p is NULL");
		return;
	}

	memset(s_arsr2p, 0, sizeof(dss_arsr2p_t));

	s_arsr2p->arsr_input_width_height = inp32(arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT);
	s_arsr2p->arsr_output_width_height = inp32(arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT);
	s_arsr2p->ihleft = inp32(arsr2p_base + ARSR2P_IHLEFT);
	s_arsr2p->ihleft1 = inp32(arsr2p_base + ARSR2P_IHLEFT1);
	s_arsr2p->ihright = inp32(arsr2p_base + ARSR2P_IHRIGHT);
	s_arsr2p->ihright1 = inp32(arsr2p_base + ARSR2P_IHRIGHT1);
	s_arsr2p->ivtop = inp32(arsr2p_base + ARSR2P_IVTOP);
	s_arsr2p->ivbottom = inp32(arsr2p_base + ARSR2P_IVBOTTOM);
	s_arsr2p->ivbottom1 = inp32(arsr2p_base + ARSR2P_IVBOTTOM1);
	s_arsr2p->ihinc = inp32(arsr2p_base + ARSR2P_IHINC);
	s_arsr2p->ivinc = inp32(arsr2p_base + ARSR2P_IVINC);
	s_arsr2p->offset = inp32(arsr2p_base + ARSR2P_OFFSET);
	s_arsr2p->mode = inp32(arsr2p_base + ARSR2P_MODE);
	s_arsr2p->mode = ((s_arsr2p->mode) & 0x1FF) | 0x800;    //dbg_en=4 for memory low power

	arsr_pre_para_init(&s_arsr2p->arsr2p_effect);
	arsr_pre_para_init(&s_arsr2p->arsr2p_effect_scale_up);
	arsr_pre_para_init(&s_arsr2p->arsr2p_effect_scale_down);

}
/*lint +e570 +e732*/

void hisi_dss_arsr2p_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem * arsr2p_base, dss_arsr2p_t *s_arsr2p)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (arsr2p_base == NULL) {
		HISI_FB_ERR("arsr2p_base is null");
		return;
	}

	if (s_arsr2p == NULL) {
		HISI_FB_ERR("s_arsr2p is null");
		return;
	}

	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_INPUT_WIDTH_HEIGHT, s_arsr2p->arsr_input_width_height, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_OUTPUT_WIDTH_HEIGHT, s_arsr2p->arsr_output_width_height, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IHLEFT, s_arsr2p->ihleft, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IHLEFT1, s_arsr2p->ihleft1, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IHRIGHT, s_arsr2p->ihright, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IHRIGHT1, s_arsr2p->ihright1, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IVTOP, s_arsr2p->ivtop, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IVBOTTOM, s_arsr2p->ivbottom, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IVBOTTOM1, s_arsr2p->ivbottom1, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IHINC, s_arsr2p->ihinc, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_IVINC, s_arsr2p->ivinc, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_OFFSET, s_arsr2p->offset, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_MODE, s_arsr2p->mode, 32, 0);

	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SKIN_THRES_Y, s_arsr2p->arsr2p_effect.skin_thres_y, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SKIN_THRES_U, s_arsr2p->arsr2p_effect.skin_thres_u, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SKIN_THRES_V, s_arsr2p->arsr2p_effect.skin_thres_v, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SKIN_CFG0, s_arsr2p->arsr2p_effect.skin_cfg0, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SKIN_CFG1, s_arsr2p->arsr2p_effect.skin_cfg1, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SKIN_CFG2, s_arsr2p->arsr2p_effect.skin_cfg2, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHOOT_CFG1, s_arsr2p->arsr2p_effect.shoot_cfg1, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHOOT_CFG2, s_arsr2p->arsr2p_effect.shoot_cfg2, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHOOT_CFG3, s_arsr2p->arsr2p_effect.shoot_cfg3, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG1, s_arsr2p->arsr2p_effect.sharp_cfg1, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG2, s_arsr2p->arsr2p_effect.sharp_cfg2, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG3, s_arsr2p->arsr2p_effect.sharp_cfg3, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG4, s_arsr2p->arsr2p_effect.sharp_cfg4, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG5, s_arsr2p->arsr2p_effect.sharp_cfg5, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG6, s_arsr2p->arsr2p_effect.sharp_cfg6, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG7, s_arsr2p->arsr2p_effect.sharp_cfg7, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG8, s_arsr2p->arsr2p_effect.sharp_cfg8, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG9, s_arsr2p->arsr2p_effect.sharp_cfg9, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG10, s_arsr2p->arsr2p_effect.sharp_cfg10, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG11, s_arsr2p->arsr2p_effect.sharp_cfg11, 32, 0);
	//hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG12, s_arsr2p->arsr2p_effect.sharp_cfg12, 32, 0);

	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_LEVEL, s_arsr2p->arsr2p_effect.sharp_level, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAIN_LOW, s_arsr2p->arsr2p_effect.sharp_gain_low, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAIN_MID, s_arsr2p->arsr2p_effect.sharp_gain_mid, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAIN_HIGH, s_arsr2p->arsr2p_effect.sharp_gain_high, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPH_MF, s_arsr2p->arsr2p_effect.sharp_gainctrl_sloph_mf, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPL_MF, s_arsr2p->arsr2p_effect.sharp_gainctrl_slopl_mf, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPH_HF, s_arsr2p->arsr2p_effect.sharp_gainctrl_sloph_hf, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAINCTRLSLOPL_HF, s_arsr2p->arsr2p_effect.sharp_gainctrl_slopl_hf, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_MF_LMT, s_arsr2p->arsr2p_effect.sharp_mf_lmt, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAIN_MF, s_arsr2p->arsr2p_effect.sharp_gain_mf, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_MF_B, s_arsr2p->arsr2p_effect.sharp_mf_b, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_HF_LMT, s_arsr2p->arsr2p_effect.sharp_hf_lmt, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_GAIN_HF, s_arsr2p->arsr2p_effect.sharp_gain_hf, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_HF_B, s_arsr2p->arsr2p_effect.sharp_hf_b, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_LF_CTRL, s_arsr2p->arsr2p_effect.sharp_lf_ctrl, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_LF_VAR, s_arsr2p->arsr2p_effect.sharp_lf_var, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_LF_CTRL_SLOP, s_arsr2p->arsr2p_effect.sharp_lf_ctrl_slop, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_HF_SELECT, s_arsr2p->arsr2p_effect.sharp_hf_select, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG2_H, s_arsr2p->arsr2p_effect.sharp_cfg2_h, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_SHARP_CFG2_L, s_arsr2p->arsr2p_effect.sharp_cfg2_l, 32, 0);

	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_TEXTURW_ANALYSTS, s_arsr2p->arsr2p_effect.texture_analysis, 32, 0);
	hisifd->set_reg(hisifd, arsr2p_base + ARSR2P_INTPLSHOOTCTRL, s_arsr2p->arsr2p_effect.intplshootctrl, 32, 0);

}
/*lint -e838*/
void hisi_dss_arsr2p_coef_on(struct hisi_fb_data_type *hisifd, bool enable_cmdlist)
{
	uint32_t module_base = 0;
	char __iomem *arsr2p_base;
	char __iomem *coefy_v = NULL;
	char __iomem *coefa_v = NULL;
	char __iomem *coefuv_v = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	module_base = g_dss_module_base[DSS_RCHN_V0][MODULE_ARSR2P_LUT];
	coefy_v = hisifd->dss_base + module_base + ARSR2P_LUT_COEFY_V_OFFSET;
	coefa_v = hisifd->dss_base + module_base + ARSR2P_LUT_COEFA_V_OFFSET;
	coefuv_v = hisifd->dss_base + module_base + ARSR2P_LUT_COEFUV_V_OFFSET;
	arsr2p_base = hisifd->dss_base + g_dss_module_base[DSS_RCHN_V0][MODULE_ARSR2P];

	/* COEFY_V COEFY_H */
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefy_v, (const int **)COEF_Y_SCL_DOWN_TAP6, (const int **)COEF_Y_SCL_UP_TAP6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefy_v + ARSR2P_COEF_H0_OFFSET, (const int **)COEF_Y_SCL_DOWN_TAP6, (const int **)COEF_Y_SCL_UP_TAP6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefy_v + ARSR2P_COEF_H1_OFFSET, (const int **)COEF_Y_SCL_DOWN_TAP6, (const int **)COEF_Y_SCL_UP_TAP6, ARSR2P_PHASE_NUM, ARSR2P_TAP6);

	/* COEFA_V COEFA_H */
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefa_v, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefa_v + ARSR2P_COEF_H0_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefa_v + ARSR2P_COEF_H1_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);

	/* COEFUV_V COEFUV_H */
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefuv_v, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefuv_v + ARSR2P_COEF_H0_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
	hisi_dss_arsr2p_write_config_coefs(hisifd, enable_cmdlist, coefuv_v + ARSR2P_COEF_H1_OFFSET, (const int **)COEF_AUV_SCL_DOWN_TAP4, (const int **)COEF_AUV_SCL_UP_TAP4, ARSR2P_PHASE_NUM, ARSR2P_TAP4);
}
/*lint +e838*/
static int hisi_dss_arsr2p_config_check_width(dss_rect_t *dest_rect, int source_width, bool hscl_en, bool vscl_en, int source_height, uint32_t index)
{
	if (NULL == dest_rect) {
		HISI_FB_ERR("dest_rect is NULL");
		return -EINVAL;
	}

	/*check arsr2p input and output width*/
	if ((source_width < ARSR2P_MIN_INPUT) || (dest_rect->w < ARSR2P_MIN_INPUT) ||
		(source_width > ARSR2P_MAX_WIDTH) || (dest_rect->w > ARSR2P_MAX_WIDTH)) {
		if ((!hscl_en) && (!vscl_en)) {
			//sharpen_en = false;
			HISI_FB_DEBUG("fb%d, src_rect.w(%d) or dst_rect.w(%d) is smaller than 16 or larger than 2560, arsr2p bypass!\n",
				index, source_width, dest_rect->w);
			return 0;
		} else {
			HISI_FB_ERR("fb%d, src_rect(%d, %d) or dst_rect(%d, %d) is smaller than 16 or larger than 2560!\n",
				index, source_width, source_height, dest_rect->w, dest_rect->h);
			return -EINVAL;
		}
	}
	return 1;
}

static int hisi_dss_arsr2p_config_check_heigh(dss_rect_t *dest_rect, dss_rect_t *source_rect, dss_layer_t *layer, int source_width)
{
	if (NULL == dest_rect) {
		HISI_FB_ERR("dest_rect is NULL");
		return -EINVAL;
	}

	if (NULL == source_rect) {
		HISI_FB_ERR("source_rect is NULL");
		return -EINVAL;
	}

	if (NULL == layer) {
		HISI_FB_ERR("layer is NULL");
		return -EINVAL;
	}
	if ((dest_rect->w > (source_width * ARSR2P_SCALE_MAX))
		|| (source_width > (dest_rect->w * ARSR2P_SCALE_MAX))) {
		HISI_FB_ERR("width out of range, original_src_rec(%d, %d, %d, %d) "
			"src_rect(%d, %d, %d, %d), dst_rect(%d, %d, %d, %d)\n",
			layer->src_rect.x, layer->src_rect.y, source_width, layer->src_rect.h,
			source_rect->x, source_rect->y, source_width, source_rect->h,
			dest_rect->x, dest_rect->y, dest_rect->w, dest_rect->h);
		return -EINVAL;
	}

	/*check arsr2p input and output height*/
	if ((source_rect->h > ARSR2P_MAX_HEIGHT) || (dest_rect->h > ARSR2P_MAX_HEIGHT)) {
		HISI_FB_ERR("src_rect.h(%d) or dst_rect.h(%d) is smaller than 16 or larger than 8192!\n",
			source_rect->h, dest_rect->h);
		return -EINVAL;
	}

	if ((dest_rect->h > (source_rect->h * ARSR2P_SCALE_MAX))
		|| (source_rect->h > (dest_rect->h * ARSR2P_SCALE_MAX))) {
		HISI_FB_ERR("height out of range, original_src_rec(%d, %d, %d, %d) "
			"src_rect(%d, %d, %d, %d), dst_rect(%d, %d, %d, %d).\n",
			layer->src_rect.x, layer->src_rect.y, layer->src_rect.w, layer->src_rect.h,
			source_rect->x, source_rect->y, source_rect->w, source_rect->h,
			dest_rect->x, dest_rect->y, dest_rect->w, dest_rect->h);
		return -EINVAL;
	}
	return 0;
}

/*lint -e613 -e838*/
int hisi_dss_arsr2p_config(struct hisi_fb_data_type *hisifd, dss_layer_t *layer, dss_rect_t *aligned_rect, bool rdma_stretch_enable)
{
	dss_arsr2p_t *arsr2p = NULL;
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
	uint32_t need_cap = 0;
	int chn_idx = 0;
	dss_block_info_t *pblock_info = NULL;
	int extraw = 0;
	int extraw_left = 0;
	int extraw_right = 0;

	bool en_hscl = false;
	bool en_vscl = false;

	/* arsr mode */
	bool nointplen = false; //bit8
	bool prescaleren = false; //bit7
	bool nearest_en = false; //bit6
	bool diintpl_en = false; //bit5
	bool textureanalyhsisen_en = false; //bit4
	bool arsr2p_bypass = true; //bit0

	bool hscldown_flag = false;

	int ih_inc = 0;
	int iv_inc = 0;
	int ih_left = 0;  //input left acc
	int ih_right = 0; //input end position
	int iv_top = 0; //input top position
	int iv_bottom = 0; //input bottom position
	int uv_offset = 0;
	int src_width = 0;
	int src_height = 0;
	int dst_whole_width = 0;

	int outph_left = 0;  //output left acc
	int outph_right = 0; //output end position
	int outpv_bottom = 0; //output bottom position
	int ret = 0;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if (NULL == layer) {
		HISI_FB_ERR("layer is NULL");
		return -EINVAL;
	}

	chn_idx = layer->chn_idx;
	if (chn_idx != DSS_RCHN_V0) {
		return 0;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		return 0;
	}

	need_cap = layer->need_cap;
	if ((hisifd->index == PRIMARY_PANEL_IDX) && (need_cap & CAP_ROT) && aligned_rect) {
		src_rect = *aligned_rect;
	} else {
		src_rect = layer->src_rect;
	}
	dst_rect = layer->dst_rect;
	pblock_info = &(layer->block_info);

	//if (pblock_info && pblock_info->h_ratio_arsr2p && pblock_info->both_vscfh_arsr2p_used) { //new added
	if (pblock_info != NULL && pblock_info->h_ratio_arsr2p) { //new added
		src_rect = pblock_info->arsr2p_in_rect; //src_rect = arsr2p_in_rect when both arsr2p and vscfh are extended
	}

	/* if vertical ratio of scaling down is larger than or equal to 2, set src_rect height to aligned rect height */
	if (aligned_rect) {
		src_rect.h = aligned_rect->h;

		if (g_debug_ovl_offline_composer) {
			HISI_FB_INFO("aligned_rect = (%d, %d, %d, %d)\n", aligned_rect->x, aligned_rect->y, aligned_rect->w, aligned_rect->h);
			HISI_FB_INFO("layer->src_rect = (%d, %d, %d, %d)\n", layer->src_rect.x, layer->src_rect.y, layer->src_rect.w, layer->src_rect.h);
			HISI_FB_INFO("layer->dst rect = (%d, %d, %d, %d)\n", layer->dst_rect.x, layer->dst_rect.y, layer->dst_rect.w, layer->dst_rect.h);
			HISI_FB_INFO("arsr2p_in_rect rect = (%d, %d, %d, %d)\n", pblock_info->arsr2p_in_rect.x, pblock_info->arsr2p_in_rect.y,
				pblock_info->arsr2p_in_rect.w, pblock_info->arsr2p_in_rect.h);
		}
	}

	/* horizental scaler compute */
	do {
            //offline subblock
		if (pblock_info != NULL && pblock_info->h_ratio_arsr2p) {
			ih_inc = pblock_info->h_ratio_arsr2p;
			src_width = src_rect.w;
			src_height = src_rect.h;
			dst_whole_width = pblock_info->arsr2p_dst_w;
			src_rect.x = src_rect.x - pblock_info->arsr2p_src_x;
			src_rect.y = src_rect.y - pblock_info->arsr2p_src_y;
			dst_rect.x = dst_rect.x - pblock_info->arsr2p_dst_x;
			dst_rect.y = dst_rect.y - pblock_info->arsr2p_dst_y;

			if (pblock_info->both_vscfh_arsr2p_used) {
				hscldown_flag = true; //horizental scaling down
			}

			if (rdma_stretch_enable) {
				en_hscl = true;
			}

			if (ih_inc && ih_inc != ARSR2P_INC_FACTOR) {
				en_hscl = true;
			}
		} else {
			/* horizental scaling down is not supported by arsr2p, set src_rect.w = dst_rect.w */
			if (src_rect.w > dst_rect.w) {
				src_width = dst_rect.w;
				hscldown_flag = true; //horizental scaling down
			} else {
				src_width = src_rect.w;
			}
			dst_whole_width = dst_rect.w;

			src_rect.x = 0;  //set src rect to zero, in case
			src_rect.y = 0;
			dst_rect.x = 0;  //set dst rect to zero, in case
			dst_rect.y = 0;

			if (src_width != dst_rect.w)
				en_hscl = true;

			//ihinc=(arsr_input_width*65536+65536-ihleft)/(arsr_output_width+1)
			ih_inc = (DSS_WIDTH(src_width) * ARSR2P_INC_FACTOR + ARSR2P_INC_FACTOR - ih_left) / dst_rect.w;
		}

		//ihleft1 = starto*ihinc - (strati <<16)
		outph_left = dst_rect.x * ih_inc - (src_rect.x * ARSR2P_INC_FACTOR);
		if (outph_left < 0) {
			outph_left = 0;
        }
		/*lint -e414*/
		//ihleft = ihleft1 - even(8*65536/ihinc) * ihinc
		extraw = (8 * ARSR2P_INC_FACTOR) / ih_inc;
		extraw_left = (extraw % 2) ? (extraw + 1) : (extraw);
		ih_left = outph_left - extraw_left * ih_inc;
		if (ih_left < 0) {
			ih_left = 0;
        }

		//ihright1 = endo * ihinc - (strati <<16);
		outph_right = (dst_rect.x + dst_rect.w - 1) * ih_inc - (src_rect.x * ARSR2P_INC_FACTOR);

		if (dst_whole_width == dst_rect.w) {
			//ihright = ihright1 + even(2*65536/ihinc) * ihinc
			extraw = (2 * ARSR2P_INC_FACTOR) / ih_inc;
			extraw_right = (extraw % 2) ? (extraw + 1) : (extraw);
			ih_right = outph_right + extraw_right * ih_inc;

			/*if(ihright+(starti << 16)) >(width - 1)* ihinc);
			ihright = endo*ihinc-(starti<<16);*/
			extraw = (dst_whole_width - 1) * ih_inc - (src_rect.x * ARSR2P_INC_FACTOR);  //ihright is checked in every tile

			if (ih_right > extraw) {
				ih_right = extraw;
			}
		} else {
			//(endi-starti+1) << 16 - 1
			ih_right = src_width * ARSR2P_INC_FACTOR - 1;
		}
		/*lint +e414*/
	} while (0);

	/* vertical scaler compute */
	do {
		if (src_rect.h != dst_rect.h)
			en_vscl = true;

		if (src_rect.h > dst_rect.h) {
			//ivinc=(arsr_input_height*65536+65536/2-ivtop)/(arsr_output_height)
			iv_inc = (DSS_HEIGHT(src_rect.h) * ARSR2P_INC_FACTOR + ARSR2P_INC_FACTOR / 2 - iv_top) /
				DSS_HEIGHT(dst_rect.h);
		} else {
			//ivinc=(arsr_input_height*65536+65536-ivtop)/(arsr_output_height+1)
			iv_inc = (DSS_HEIGHT(src_rect.h) * ARSR2P_INC_FACTOR + ARSR2P_INC_FACTOR - iv_top) / dst_rect.h;
		}

		//ivbottom = arsr_output_height*ivinc + ivtop
		iv_bottom = DSS_HEIGHT(dst_rect.h) * iv_inc + iv_top;
		outpv_bottom = iv_bottom;

	} while (0);

	if ((!en_hscl) && (!en_vscl)) {
		if (!hscldown_flag) {
			/*if only sharpness is needed, disable image interplo, enable textureanalyhsis*/
			nointplen = true;
			textureanalyhsisen_en = true;
		}
	}

	arsr2p = &(hisifd->dss_module.arsr2p[chn_idx]);
	hisifd->dss_module.arsr2p_used[chn_idx] = 1;

	ret = hisi_dss_arsr2p_config_check_width(&dst_rect, src_width, en_hscl, en_vscl, src_height, hisifd->index);
	if (ret <= 0) {
		return ret;
	}
	ret = hisi_dss_arsr2p_config_check_heigh(&dst_rect, &src_rect, layer, src_width);
	if (ret < 0) {
		return ret;
	}

	/*if arsr2p is enabled, hbp+hfp+hsw > 20*/
	/*if (hisifd_primary && (hisifd_primary->panel_info.ldi.h_back_porch + hisifd_primary->panel_info.ldi.h_front_porch
		+ hisifd_primary->panel_info.ldi.h_pulse_width) <= 20) {
		HISI_FB_ERR("ldi hbp+hfp+hsw is not larger than 20, return!\n");
		return -EINVAL;
	}*/

	/*config arsr2p mode , start*/
	arsr2p_bypass = false;
	do {
		if (hscldown_flag) { //horizental scale down
			prescaleren = true;
			break;
		}

		if (!en_hscl && (iv_inc >= 2 * ARSR2P_INC_FACTOR) && !pblock_info->h_ratio_arsr2p) {
			//only vertical scale down, enable nearest scaling down, disable sharp in non-block scene
			nearest_en = true;
			break;
		}

		if ((!en_hscl) && (!en_vscl)) {
			break;
		}

		diintpl_en = true;
		//imageintpl_dis = true;
		textureanalyhsisen_en = true;
	} while (0);

	/*config arsr2p mode , end*/
	/*lint -e730 -e732*/
	/*config the effect parameters as long as arsr2p is used*/
	hisi_effect_arsr2p_config(&(arsr2p->arsr2p_effect), ih_inc, iv_inc);
	hisifd->dss_module.arsr2p_effect_used[chn_idx] = 1;

	arsr2p->arsr_input_width_height = set_bits32(arsr2p->arsr_input_width_height, DSS_HEIGHT(src_rect.h), 13, 0);
	arsr2p->arsr_input_width_height = set_bits32(arsr2p->arsr_input_width_height, DSS_WIDTH(src_width), 13, 16);
	arsr2p->arsr_output_width_height = set_bits32(arsr2p->arsr_output_width_height, DSS_HEIGHT(dst_rect.h), 13, 0);
	arsr2p->arsr_output_width_height = set_bits32(arsr2p->arsr_output_width_height, DSS_WIDTH(dst_rect.w), 13, 16);
	arsr2p->ihleft = set_bits32(arsr2p->ihleft, ih_left, 29, 0);
	arsr2p->ihright = set_bits32(arsr2p->ihright, ih_right, 29, 0);
	arsr2p->ivtop = set_bits32(arsr2p->ivtop, iv_top, 29, 0);
	arsr2p->ivbottom = set_bits32(arsr2p->ivbottom, iv_bottom, 29, 0);
	arsr2p->ihinc = set_bits32(arsr2p->ihinc, ih_inc, 22, 0);
	arsr2p->ivinc = set_bits32(arsr2p->ivinc, iv_inc, 22, 0);
	arsr2p->offset = set_bits32(arsr2p->offset, uv_offset, 22, 0);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p_bypass, 1, 0);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p->arsr2p_effect.sharp_enable, 1, 1);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p->arsr2p_effect.shoot_enable, 1, 2);
	arsr2p->mode = set_bits32(arsr2p->mode, arsr2p->arsr2p_effect.skin_enable, 1, 3);
	arsr2p->mode = set_bits32(arsr2p->mode, textureanalyhsisen_en, 1, 4);
	arsr2p->mode = set_bits32(arsr2p->mode, diintpl_en, 1, 5);
	arsr2p->mode = set_bits32(arsr2p->mode, nearest_en, 1, 6);
	arsr2p->mode = set_bits32(arsr2p->mode, prescaleren, 1, 7);
	arsr2p->mode = set_bits32(arsr2p->mode, nointplen, 1, 8);

	arsr2p->ihleft1 = set_bits32(arsr2p->ihleft1, outph_left, 29, 0);
	arsr2p->ihright1 = set_bits32(arsr2p->ihright1, outph_right, 29, 0);
	arsr2p->ivbottom1 = set_bits32(arsr2p->ivbottom1, outpv_bottom, 29, 0);
	/*lint +e730 +e732*/
	return 0;
}
/*lint +e613 +e838*/

/*******************************************************************************
** DSS remove mctl ch&ov mutex for offline
*/
/*lint -e838 -e679*/
void hisi_remove_mctl_mutex(struct hisi_fb_data_type *hisifd, int mctl_idx, uint32_t cmdlist_idxs)
{
	dss_module_reg_t *dss_module = NULL;
	int i = 0;
	char __iomem *chn_mutex_base = NULL;
	char __iomem *cmdlist_base = NULL;
	uint32_t offset = 0;
	uint32_t cmdlist_idxs_temp = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_module = &(hisifd->dss_module);
	cmdlist_base = hisifd->dss_base + DSS_CMDLIST_OFFSET;

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (dss_module->mctl_ch_used[i] == 1) {
			chn_mutex_base = dss_module->mctl_ch_base[i].chn_mutex_base +
				g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
			if (NULL == chn_mutex_base) {
				HISI_FB_ERR("chn_mutex_base is NULL");
				return;
			}

			set_reg(chn_mutex_base, 0, 32, 0);
		}
	}

	set_reg(dss_module->mctl_base[mctl_idx] + MCTL_CTL_MUTEX_OV, 0, 32, 0);

	offset = 0x40;
	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			set_reg(cmdlist_base + CMDLIST_CH0_CTRL + i * offset, 0x6, 3, 2); //start sel
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

}
/*lint +e838 +e679*/
/*lint -e838 -e774 -e655 -e573 -e737  -e732 -e730 -e613*/
int hisi_dss_wb_scl_config(struct hisi_fb_data_type *hisifd, dss_wb_layer_t *wb_layer)
{
	dss_scl_t *scl = NULL;
	dss_rect_t src_rect;
	dss_rect_t dst_rect;
	dss_rect_t temp;
	int chn_idx;
	dss_wb_block_info_t *pblock_info = NULL;

	bool has_pixel_alpha = false;
	bool en_hscl = false;
	bool en_vscl = false;
	bool en_mmp = false;
	uint32_t h_ratio = 0;
	uint32_t v_ratio = 0;
	uint32_t h_v_order = 0;
	uint32_t acc_hscl = 0;
	uint32_t acc_vscl = 0;
	uint32_t scf_en_vscl = 0;

	if ((hisifd == NULL) || (wb_layer == NULL)) {
		return -EINVAL;
	}

	chn_idx = wb_layer->chn_idx;
	if (chn_idx != DSS_WCHN_W1) {
		return 0;
	}

	pblock_info = &(wb_layer->wb_block_info);
	if (pblock_info && (pblock_info->src_rect.w != 0) && (pblock_info->dst_rect.w != 0)) {
		src_rect = pblock_info->src_rect;
		dst_rect = pblock_info->dst_rect;
	} else {
		src_rect = wb_layer->src_rect;
		dst_rect = wb_layer->dst_rect;

		if (wb_layer->transform == (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V)) {
			temp.w = dst_rect.w;
			dst_rect.w = dst_rect.h;
			dst_rect.h = temp.w;
		}
	}

	do {
		if (pblock_info && (pblock_info->h_ratio != 0) && (pblock_info->h_ratio != SCF_INC_FACTOR)) {
			h_ratio = pblock_info->h_ratio;
			en_hscl = true;
			break;
		}

		if (src_rect.w == dst_rect.w)
			break;

		en_hscl = true;

		if ((src_rect.w < SCF_MIN_INPUT) || (dst_rect.w < SCF_MIN_OUTPUT)) {
			HISI_FB_ERR("src_rect.w(%d) small than 16, or dst_rect.w(%d) small than 16\n",
				src_rect.w, dst_rect.w);
			return -EINVAL;
		}

		h_ratio = (DSS_HEIGHT(src_rect.w) * SCF_INC_FACTOR + SCF_INC_FACTOR / 2 - acc_hscl) /
			DSS_HEIGHT(dst_rect.w);

		if ((dst_rect.w > (src_rect.w * SCF_UPSCALE_MAX))
			|| (src_rect.w > (dst_rect.w * SCF_DOWNSCALE_MAX))) {
			HISI_FB_ERR("width out of range, original_src_rec(%d, %d, %d, %d) "
				"src_rect(%d, %d, %d, %d), dst_rect(%d, %d, %d, %d)\n",
				wb_layer->src_rect.x, wb_layer->src_rect.y, wb_layer->src_rect.w, wb_layer->src_rect.h,
				src_rect.x, src_rect.y, src_rect.w, src_rect.h,
				dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);

			return -EINVAL;
		}
	} while (0);

	do {
		if (src_rect.h == dst_rect.h)
			break;

		en_vscl = true;
		scf_en_vscl = 1;

		v_ratio = (DSS_HEIGHT(src_rect.h) * SCF_INC_FACTOR + SCF_INC_FACTOR / 2 - acc_vscl) /
			DSS_HEIGHT(dst_rect.h);

		if ((dst_rect.h > (src_rect.h * SCF_UPSCALE_MAX))
			|| (src_rect.h > (dst_rect.h * SCF_DOWNSCALE_MAX))) {
			HISI_FB_ERR("height out of range, original_src_rec(%d, %d, %d, %d) "
				"src_rect(%d, %d, %d, %d), dst_rect(%d, %d, %d, %d).\n",
				wb_layer->src_rect.x, wb_layer->src_rect.y, wb_layer->src_rect.w, wb_layer->src_rect.h,
				src_rect.x, src_rect.y, src_rect.w, src_rect.h,
				dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
			return -EINVAL;
		}
	} while (0);

	if (!en_hscl && !en_vscl) {
		return 0;
	}

	/* scale down, do hscl first; scale up, do vscl first*/
	h_v_order = (src_rect.w > dst_rect.w) ? 0 : 1;

	if (pblock_info && (pblock_info->acc_hscl != 0)) {
		acc_hscl = pblock_info->acc_hscl;
	}

	scl = &(hisifd->dss_module.scl[chn_idx]);
	hisifd->dss_module.scl_used[chn_idx] = 1;

	has_pixel_alpha = hal_format_has_alpha(wb_layer->dst.format);

	scl->en_hscl_str = set_bits32(scl->en_hscl_str, 0x0, 1, 0);

	//if (DSS_HEIGHT(src_rect.h) * 2 >= DSS_HEIGHT(dst_rect.h)) {
	if (v_ratio >= 2 * SCF_INC_FACTOR) {
		if (has_pixel_alpha)
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x3, 2, 0);
		else
			scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x1, 2, 0);
	} else {
		scl->en_vscl_str = set_bits32(scl->en_vscl_str, 0x0, 1, 0);
	}

	if (src_rect.h > dst_rect.h) {
		scf_en_vscl = 0x3;
	}
	en_mmp = 0x1;

	scl->h_v_order = set_bits32(scl->h_v_order, h_v_order, 1, 0);
	scl->input_width_height = set_bits32(scl->input_width_height,
		DSS_HEIGHT(src_rect.h), 13, 0);
	scl->input_width_height = set_bits32(scl->input_width_height,
		DSS_WIDTH(src_rect.w), 13, 16);
	scl->output_width_height = set_bits32(scl->output_width_height,
		DSS_HEIGHT(dst_rect.h), 13, 0);
	scl->output_width_height = set_bits32(scl->output_width_height,
		DSS_WIDTH(dst_rect.w), 13, 16);
	scl->en_hscl = set_bits32(scl->en_hscl, (en_hscl ? 0x1 : 0x0), 1, 0);
	scl->en_vscl = set_bits32(scl->en_vscl, scf_en_vscl, 2, 0);
	scl->acc_hscl = set_bits32(scl->acc_hscl, acc_hscl, 31, 0);
	scl->inc_hscl = set_bits32(scl->inc_hscl, h_ratio, 24, 0);
	scl->inc_vscl = set_bits32(scl->inc_vscl, v_ratio, 24, 0);
	scl->en_mmp = set_bits32(scl->en_mmp, en_mmp, 1, 0);

	if (g_debug_ovl_mediacommon_composer) {
		HISI_FB_INFO("ch%d, wb_layer_src[%d %d], wb_layer_dst[%d %d], acc_hscl=%d, h_ratio=%d, transform=%d, w=%d, h=%d!"
			"pblock_info->src_rect[%d:%d:%d:%d], pblock_info->dst_rect[%d:%d:%d:%d].\n",
			wb_layer->chn_idx, wb_layer->src_rect.w, wb_layer->src_rect.h,
			wb_layer->dst_rect.w, wb_layer->dst_rect.h, wb_layer->wb_block_info.acc_hscl, pblock_info->h_ratio,
			wb_layer->transform, dst_rect.w, dst_rect.h,
			pblock_info->src_rect.x, pblock_info->src_rect.y, pblock_info->src_rect.w, pblock_info->src_rect.h,
			pblock_info->dst_rect.x, pblock_info->dst_rect.y, pblock_info->dst_rect.w, pblock_info->dst_rect.h);
	}

	return 0;
}
/*lint +e838 +e774 +e655 +e573 +e737 +e732 +e730 +e613*/

void hisi_dss_mctl_ov_set_ctl_dbg_reg(struct hisi_fb_data_type *hisifd, char __iomem *mctl_base, bool enable_cmdlist)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null");
		return;
	}

	if (mctl_base == NULL) {
		HISI_FB_ERR("mctl_base is null!\n");
		return;
	}

	if (enable_cmdlist) {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB03A20, 32, 0);
		set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
	} else {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB13A00, 32, 0);
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			set_reg(mctl_base + MCTL_CTL_TOP, 0x2, 32, 0);
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			set_reg(mctl_base + MCTL_CTL_TOP, 0x3, 32, 0);
		} else {
			;
		}
	}
}

void hisi_dss_post_clip_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *post_clip_base, dss_post_clip_t *s_post_clip, int chn_idx)
{
	if (NULL == hisifd || NULL == post_clip_base || NULL == s_post_clip) {
		HISI_FB_ERR("hisifd post_clip_base or s_post_clip is null\n");
		return;
	}

	hisifd->set_reg(hisifd, post_clip_base + POST_CLIP_DISP_SIZE, s_post_clip->disp_size, 32, 0);
	hisifd->set_reg(hisifd, post_clip_base + POST_CLIP_CTL_HRZ, s_post_clip->clip_ctl_hrz, 32, 0);
	hisifd->set_reg(hisifd, post_clip_base + POST_CLIP_CTL_VRZ, s_post_clip->clip_ctl_vrz, 32, 0);
	hisifd->set_reg(hisifd, post_clip_base + POST_CLIP_EN, s_post_clip->ctl_clip_en, 32, 0);
}


/*******************************************************************************
**
*/
static int hisi_dss_check_wblayer_buff(struct hisi_fb_data_type *hisifd, dss_wb_layer_t *wb_layer)
{
	if (hisi_dss_check_addr_validate(&wb_layer->dst)) {
		return 0;
	}

	if (wb_layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR)) {
		return 0;
	}

	return -EINVAL;
}

static int hisi_dss_check_wblayer_rect(struct hisi_fb_data_type *hisifd, dss_wb_layer_t *wb_layer)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("invalid hisifd!");
		return -EINVAL;
	}

	if (wb_layer == NULL) {
		HISI_FB_ERR("fb%d, wb_layer is NULL, return!", hisifd->index);
		return -EINVAL;
	}

	if (wb_layer->src_rect.x < 0 || wb_layer->src_rect.y < 0 ||
		wb_layer->src_rect.w <= 0 || wb_layer->src_rect.h <= 0) {
		HISI_FB_ERR("fb%d, src_rect(%d, %d, %d, %d) is out of range!\n",
			hisifd->index, wb_layer->src_rect.x, wb_layer->src_rect.y,
			wb_layer->src_rect.w, wb_layer->src_rect.h);
		return -EINVAL;
	}
	/*lint -e574 -e737*/
	if (wb_layer->dst_rect.x < 0 || wb_layer->dst_rect.y < 0 ||
		wb_layer->dst_rect.w <= 0 || wb_layer->dst_rect.h <= 0 ||
		wb_layer->dst_rect.w > wb_layer->dst.width || wb_layer->dst_rect.h > wb_layer->dst.height) {
		HISI_FB_ERR("fb%d, dst_rect(%d, %d, %d, %d), dst(%d, %d) is out of range!\n",
			hisifd->index, wb_layer->dst_rect.x, wb_layer->dst_rect.y, wb_layer->dst_rect.w,
			wb_layer->dst_rect.h, wb_layer->dst.width, wb_layer->dst.height);
		return -EINVAL;
	}
	/*lint +e574 +e737*/

	return 0;
}

static int hisi_dss_check_userdata_base(struct hisi_fb_data_type *hisifd,
	dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block_infos)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("invalid hisifd!");
		return -EINVAL;
	}

	if (pov_req == NULL) {
		HISI_FB_ERR("fb%d, invalid pov_req!", hisifd->index);
		return -EINVAL;
	}

	if (pov_h_block_infos == NULL) {
		HISI_FB_ERR("fb%d, invalid pov_h_block_infos!", hisifd->index);
		return -EINVAL;
	}

	if ((pov_req->ov_block_nums <= 0) ||
		(pov_req->ov_block_nums > HISI_DSS_OV_BLOCK_NUMS)) {
		HISI_FB_ERR("fb%d, invalid ov_block_nums=%d!",
			hisifd->index, pov_req->ov_block_nums);
		return -EINVAL;
	}

	if ((pov_h_block_infos->layer_nums <= 0)
		|| (pov_h_block_infos->layer_nums > OVL_LAYER_NUM_MAX)) {
		HISI_FB_ERR("fb%d, invalid layer_nums=%d!",
			hisifd->index, pov_h_block_infos->layer_nums);
		return -EINVAL;
	}

	if ((pov_req->ovl_idx < 0) ||
		(pov_req->ovl_idx >= DSS_OVL_IDX_MAX)) {
		HISI_FB_ERR("fb%d, invalid ovl_idx=%d!",
			hisifd->index, pov_req->ovl_idx);
		return -EINVAL;
	}
	return 0;
}

static int hisi_dss_check_userdata_dst(dss_wb_layer_t *wb_layer, uint32_t index)
{
	if (wb_layer == NULL) {
		HISI_FB_ERR("fb%d, invalid wb_layer!", index);
		return -EINVAL;
	}

	if (wb_layer->need_cap & CAP_AFBCE) {
		if ((wb_layer->dst.afbc_header_stride == 0) || (wb_layer->dst.afbc_payload_stride == 0)) {
			HISI_FB_ERR("fb%d, afbc_header_stride=%d, afbc_payload_stride=%d is invalid!",
				index, wb_layer->dst.afbc_header_stride, wb_layer->dst.afbc_payload_stride);
			return -EINVAL;
		}
	}

	if (wb_layer->need_cap & CAP_HFBCE) {
		if ((wb_layer->dst.hfbc_header_stride0 == 0) || (wb_layer->dst.hfbc_payload_stride0 == 0) ||
			(wb_layer->dst.hfbc_header_stride1 == 0) || (wb_layer->dst.hfbc_payload_stride1 == 0) ||
			(wb_layer->chn_idx != DSS_WCHN_W1)) {
				HISI_FB_ERR("fb%d, hfbc_header_stride0=%d, hfbc_payload_stride0=%d,"
					"hfbc_header_stride1=%d, hfbc_payload_stride1=%d is invalid or wchn_idx=%d no support hfbce!\n",
					index, wb_layer->dst.hfbc_header_stride0, wb_layer->dst.hfbc_payload_stride0,
					wb_layer->dst.hfbc_header_stride1, wb_layer->dst.hfbc_payload_stride1, wb_layer->chn_idx);
				return -EINVAL;
		}
	}
	return 0;
}

/*lint -e574 -e737*/
int hisi_dss_check_userdata(struct hisi_fb_data_type *hisifd,
	dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block_infos)
{
	int i = 0;
	dss_wb_layer_t *wb_layer = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("invalid hisifd!");
		return -EINVAL;
	}

	if (pov_req == NULL) {
		HISI_FB_ERR("fb%d, invalid pov_req!", hisifd->index);
		return -EINVAL;
	}

	if (pov_h_block_infos == NULL) {
		HISI_FB_ERR("fb%d, invalid pov_h_block_infos!", hisifd->index);
		return -EINVAL;
	}

	if (hisi_dss_check_userdata_base(hisifd, pov_req, pov_h_block_infos) < 0) {
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->panel_info.dirty_region_updt_support) {
			if (pov_req->dirty_rect.x < 0 || pov_req->dirty_rect.y < 0 ||
				pov_req->dirty_rect.w < 0 || pov_req->dirty_rect.h < 0) {
				HISI_FB_ERR("dirty_rect(%d, %d, %d, %d) is out of range!\n",
					pov_req->dirty_rect.x, pov_req->dirty_rect.y,
					pov_req->dirty_rect.w, pov_req->dirty_rect.h);
				return -EINVAL;
			}
		}
	}

	if (hisifd->index == AUXILIARY_PANEL_IDX) {
		if (pov_req->wb_enable != 1) {
			HISI_FB_ERR("pov_req->wb_enable=%u is invalid!\n", pov_req->wb_enable);
			return -EINVAL;
		}

		if ((pov_req->wb_layer_nums <= 0) ||
			(pov_req->wb_layer_nums > MAX_DSS_DST_NUM)) {
			HISI_FB_ERR("fb%d, invalid wb_layer_nums=%d!",
				hisifd->index, pov_req->wb_layer_nums);
			return -EINVAL;
		}

		if (pov_req->wb_ov_rect.x < 0 || pov_req->wb_ov_rect.y < 0) {
			HISI_FB_ERR("wb_ov_rect(%d, %d) is out of range!\n",
				pov_req->wb_ov_rect.x, pov_req->wb_ov_rect.y);
			return -EINVAL;
		}

		if (pov_req->wb_compose_type >= DSS_WB_COMPOSE_TYPE_MAX) {
			HISI_FB_ERR("wb_compose_type=%u is invalid!\n", pov_req->wb_compose_type);
			return -EINVAL;
		}

		for (i = 0; i < pov_req->wb_layer_nums; i++) {
			wb_layer = &(pov_req->wb_layer_infos[i]);

			if (wb_layer->chn_idx != DSS_WCHN_W2) {
				if (wb_layer->chn_idx < DSS_WCHN_W0 || wb_layer->chn_idx > DSS_WCHN_W1) {
					HISI_FB_ERR("fb%d, wchn_idx=%d is invalid!", hisifd->index, wb_layer->chn_idx);
					return -EINVAL;
				}
			}

			if (wb_layer->dst.format >= HISI_FB_PIXEL_FORMAT_MAX) {
				HISI_FB_ERR("fb%d, format=%d is invalid!", hisifd->index, wb_layer->dst.format);
				return -EINVAL;
			}

			if ((wb_layer->dst.bpp == 0) || (wb_layer->dst.width == 0) || (wb_layer->dst.height == 0)
				|| ((wb_layer->dst.stride == 0) && (!(wb_layer->need_cap & (CAP_HFBCE | CAP_HEBCE))))) {
				HISI_FB_ERR("fb%d, bpp=%d, width=%d, height=%d, stride=%d is invalid!",
					hisifd->index, wb_layer->dst.bpp, wb_layer->dst.width, wb_layer->dst.height,
					wb_layer->dst.stride);
				return -EINVAL;
			}

			if (hisi_dss_check_wblayer_buff(hisifd, wb_layer)) {
				HISI_FB_ERR("fb%d, failed to check_wblayer_buff!", hisifd->index);
				return -EINVAL;
			}

			if (!(wb_layer->need_cap & (CAP_HFBCE | CAP_HEBCE))) {
				if (isYUVSemiPlanar(wb_layer->dst.format) || isYUVPlanar(wb_layer->dst.format)) {
					if ((wb_layer->dst.stride_plane1 == 0) || (wb_layer->dst.offset_plane1 == 0)) {
						HISI_FB_ERR("fb%d, stride_plane1=%d, offset_plane1=%d is invalid!",
							hisifd->index, wb_layer->dst.stride_plane1, wb_layer->dst.offset_plane1);
						return -EINVAL;
					}
				}

				if (isYUVPlanar(wb_layer->dst.format)) {
					if ((wb_layer->dst.stride_plane2 == 0) || (wb_layer->dst.offset_plane2 == 0)) {
						HISI_FB_ERR("fb%d, stride_plane2=%d, offset_plane2=%d is invalid!",
							hisifd->index, wb_layer->dst.stride_plane2, wb_layer->dst.offset_plane2);
						return -EINVAL;
					}
				}
			}

			if (hisi_dss_check_userdata_dst(wb_layer, hisifd->index) < 0) {
				return -EINVAL;
			}

			if (wb_layer->dst.csc_mode >= DSS_CSC_MOD_MAX) {
				HISI_FB_ERR("fb%d, csc_mode=%d is invalid!", hisifd->index, wb_layer->dst.csc_mode);
				return -EINVAL;
			}

			if (wb_layer->dst.afbc_scramble_mode >= DSS_AFBC_SCRAMBLE_MODE_MAX) {
				HISI_FB_ERR("fb%d, afbc_scramble_mode=%d is invalid!", hisifd->index, wb_layer->dst.afbc_scramble_mode);
				return -EINVAL;
			}

			if (hisi_dss_check_wblayer_rect(hisifd, wb_layer)) {
				HISI_FB_ERR("fb%d, failed to check_wblayer_rect!", hisifd->index);
				return -EINVAL;
			}
		}
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		for (i = 0; i < pov_req->wb_layer_nums; i++) {
			wb_layer = &(pov_req->wb_layer_infos[i]);
			if (hisi_dss_check_wblayer_buff(hisifd, wb_layer)) {
				HISI_FB_ERR("fb%d, failed to check_wblayer_buff!", hisifd->index);
				return -EINVAL;
			}
		}
	}

	return 0;
}
/*lint +e574 +e737*/

static int hisi_dss_check_layer_buff(struct hisi_fb_data_type *hisifd, dss_layer_t *layer)
{
	if (hisifd->video_idle_ctrl.idle_frame_display)
		return 0;

	if (hisi_dss_check_addr_validate(&layer->img)) {
		return 0;
	}

	if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR)) {
		return 0;
	}

	return -EINVAL;
}

static int hisi_dss_check_layer_rect(dss_layer_t *layer)
{
	if (layer == NULL) {
		HISI_FB_ERR("layer is NULL, return!");
		return -EINVAL;
	}

	if (layer->src_rect.x < 0 || layer->src_rect.y < 0 ||
		layer->src_rect.w <= 0 || layer->src_rect.h <= 0) {
		HISI_FB_ERR("src_rect(%d, %d, %d, %d) is out of range!\n",
			layer->src_rect.x, layer->src_rect.y,
			layer->src_rect.w, layer->src_rect.h);
		return -EINVAL;
	}

	if (layer->src_rect_mask.x < 0 || layer->src_rect_mask.y < 0 ||
		layer->src_rect_mask.w < 0 || layer->src_rect_mask.h < 0) {
		HISI_FB_ERR("src_rect_mask(%d, %d, %d, %d) is out of range!\n",
			layer->src_rect_mask.x, layer->src_rect_mask.y,
			layer->src_rect_mask.w, layer->src_rect_mask.h);
		return -EINVAL;
	}

	if (layer->dst_rect.x < 0 || layer->dst_rect.y < 0 ||
		layer->dst_rect.w <= 0 || layer->dst_rect.h <= 0) {
		HISI_FB_ERR("dst_rect(%d, %d, %d, %d) is out of range!\n",
			layer->dst_rect.x, layer->dst_rect.y,
			layer->dst_rect.w, layer->dst_rect.h);
		return -EINVAL;
	}

	return 0;
}

static int hisi_dss_check_layer_par_need_cap(dss_layer_t *layer, uint32_t index)
{
	if (layer == NULL) {
		HISI_FB_ERR("layer is NULL, return!");
		return -EINVAL;
	}

	if (layer->need_cap & CAP_AFBCD) {
		if ((layer->img.afbc_header_stride == 0) || (layer->img.afbc_payload_stride == 0)
			|| (layer->img.mmbuf_size == 0)) {
			HISI_FB_ERR("fb%d, afbc_header_stride=%d, afbc_payload_stride=%d, mmbuf_size=%d is invalid!",
				index, layer->img.afbc_header_stride,
				layer->img.afbc_payload_stride, layer->img.mmbuf_size);
			return -EINVAL;
		}
	}

	if (layer->need_cap & CAP_HFBCD) {
		if ((layer->img.hfbc_header_stride0 == 0) || (layer->img.hfbc_payload_stride0 == 0) ||
			(layer->img.hfbc_header_stride1 == 0) || (layer->img.hfbc_payload_stride1 == 0) ||
			((layer->chn_idx != DSS_RCHN_V0) && (layer->chn_idx != DSS_RCHN_V1))) {
			HISI_FB_ERR("fb%d, hfbc_header_stride0=%d, hfbc_payload_stride0=%d,"
				"hfbc_header_stride1=%d, hfbc_payload_stride1=%d is invalid or chn_idx=%d no support hfbcd!\n",
				index, layer->img.hfbc_header_stride0, layer->img.hfbc_payload_stride0,
				layer->img.hfbc_header_stride1, layer->img.hfbc_payload_stride1, layer->chn_idx);
			return -EINVAL;
		}
	}
	return 0;
}

int hisi_dss_check_layer_par(struct hisi_fb_data_type *hisifd, dss_layer_t *layer)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL, return!");
		return -EINVAL;
	}

	if (layer == NULL) {
		HISI_FB_ERR("layer is NULL, return!");
		return -EINVAL;
	}

	if (layer->layer_idx < 0 || layer->layer_idx >= OVL_LAYER_NUM_MAX) {
		HISI_FB_ERR("fb%d, layer_idx=%d is invalid!", hisifd->index, layer->layer_idx);
		return -EINVAL;
	}

	if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
		return 0;

	if (hisifd->index == AUXILIARY_PANEL_IDX) {
		if (layer->chn_idx != DSS_RCHN_V2) {
			if (layer->chn_idx < 0 ||layer->chn_idx >= DSS_WCHN_W0) {
				HISI_FB_ERR("fb%d, rchn_idx=%d is invalid!", hisifd->index, layer->chn_idx);
				return -EINVAL;
			}
		}

		if (layer->chn_idx == DSS_RCHN_D2) {
			HISI_FB_ERR("fb%d, chn_idx[%d] does not used by offline play!", hisifd->index, layer->chn_idx);
			return -EINVAL;
		}
	} else {
		if (layer->chn_idx < 0 || layer->chn_idx >= DSS_COPYBIT_MAX) {
			HISI_FB_ERR("fb%d, rchn_idx=%d is invalid!", hisifd->index, layer->chn_idx);
			return -EINVAL;
		}
	}

	if (layer->blending < 0 || layer->blending >= HISI_FB_BLENDING_MAX) {
		HISI_FB_ERR("fb%d, blending=%d is invalid!", hisifd->index, layer->blending);
		return -EINVAL;
	}

	if (layer->img.format >= HISI_FB_PIXEL_FORMAT_MAX) {
		HISI_FB_ERR("fb%d, format=%d is invalid!", hisifd->index, layer->img.format);
		return -EINVAL;
	}

	if ((layer->img.bpp == 0) || (layer->img.width == 0) || (layer->img.height == 0)
		|| ((layer->img.stride == 0) && (!(layer->need_cap & (CAP_HFBCD | CAP_HEBCD))))) {
		HISI_FB_ERR("fb%d, bpp=%d, width=%d, height=%d, stride=%d is invalid!",
			hisifd->index, layer->img.bpp, layer->img.width, layer->img.height,
			layer->img.stride);
		return -EINVAL;
	}

	if (hisi_dss_check_layer_buff(hisifd, layer)) {
		HISI_FB_ERR("fb%d, failed to check_layer_buff!", hisifd->index);
		return -EINVAL;
	}

	if (!(layer->need_cap & (CAP_HFBCD | CAP_HEBCD))) {
		if (isYUVSemiPlanar(layer->img.format) || isYUVPlanar(layer->img.format)) {
			if ((layer->img.stride_plane1 == 0) || (layer->img.offset_plane1 == 0)) {
				HISI_FB_ERR("fb%d, stride_plane1=%d, offset_plane1=%d is invalid!",
					hisifd->index, layer->img.stride_plane1, layer->img.offset_plane1);
				return -EINVAL;
			}
		}

		if (isYUVPlanar(layer->img.format)) {
			if ((layer->img.stride_plane2 == 0) || (layer->img.offset_plane2 == 0)) {
				HISI_FB_ERR("fb%d, stride_plane2=%d, offset_plane2=%d is invalid!",
					hisifd->index, layer->img.stride_plane2, layer->img.offset_plane2);
				return -EINVAL;
			}
		}
	}

	if (hisi_dss_check_layer_par_need_cap(layer, hisifd->index) < 0) {
		return -EINVAL;
	}

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if (((layer->chn_idx == DSS_RCHN_V0) || (layer->chn_idx == DSS_RCHN_V1)) && (layer->need_cap & CAP_HFBCD)) {
			HISI_FB_INFO("fb%d, ch%d,need_cap=%d, need rot!\n!", hisifd->index, layer->chn_idx, layer->need_cap);
		} else {
			HISI_FB_ERR("fb%d, ch%d is not support need_cap=%d, transform=%d!\n",
				hisifd->index, layer->chn_idx, layer->need_cap, layer->transform);
			return -EINVAL;
		}
	}

	if (layer->img.csc_mode >= DSS_CSC_MOD_MAX) {
		HISI_FB_ERR("fb%d, csc_mode=%d is invalid!", hisifd->index, layer->img.csc_mode);
		return -EINVAL;
	}

	if (layer->img.afbc_scramble_mode >= DSS_AFBC_SCRAMBLE_MODE_MAX) {
		HISI_FB_ERR("fb%d, afbc_scramble_mode=%d is invalid!", hisifd->index, layer->img.afbc_scramble_mode);
		return -EINVAL;
	}

	if ((layer->layer_idx != 0) && (layer->need_cap & CAP_BASE)) {
		HISI_FB_ERR("fb%d, layer%d is not base!", hisifd->index, layer->layer_idx);
		return -EINVAL;
	}

	if (hisi_dss_check_layer_rect(layer)) {
		HISI_FB_ERR("fb%d, failed to check_layer_rect!", hisifd->index);
		return -EINVAL;
	}

	return 0;
}

/*******************************************************************************
** DSS disreset
*/
/*lint -e747 -e778 -e774 -e732 -e838*/
void hisifb_dss_disreset(struct hisi_fb_data_type *hisifd)
{
	(void)hisifd;
}
/*lint -e438 -e550 -e573 -e647 -e701 -e712 -e713 -e737 -e834 -e838 -e845*/

static int hisi_dss_hfbcd_mmbuf_config(struct hisi_fb_data_type *hisifd, int ovl_idx,
	dss_layer_t *layer, uint32_t hfbcd_block_type, bool is_pixel_10bit)
{
	int chn_idx;
	dss_rect_t src_rect;
	bool mm_alloc_needed = false;

	dss_rect_ltrb_t hfbcd_rect;
	uint32_t mmbuf_line_num = 0;

	chn_idx = layer->chn_idx;
	src_rect = layer->src_rect;

	if (ovl_idx <= DSS_OVL1) {
		mm_alloc_needed = true;
	} else {
		if (hisifd->mmbuf_info->mm_used[chn_idx] == 1) {
			mm_alloc_needed = false;
		} else {
			mm_alloc_needed = true;
		}
	}

	if (mm_alloc_needed) {
		hfbcd_rect.left = ALIGN_DOWN((uint32_t)src_rect.x, MMBUF_ADDR_ALIGN);
		hfbcd_rect.right = ALIGN_UP((uint32_t)(src_rect.x - hfbcd_rect.left + src_rect.w), MMBUF_ADDR_ALIGN);

		if (hfbcd_block_type == 0) {
			if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
				mmbuf_line_num = MMBUF_BLOCK0_ROT_LINE_NUM;
				hfbcd_rect.left = ALIGN_DOWN((uint32_t)src_rect.y, MMBUF_ADDR_ALIGN);
				hfbcd_rect.right = ALIGN_UP((uint32_t)(src_rect.y - hfbcd_rect.left + src_rect.h), MMBUF_ADDR_ALIGN);
			} else {
				mmbuf_line_num = MMBUF_BLOCK0_LINE_NUM;
			}
		} else if (hfbcd_block_type == 1) {
			mmbuf_line_num = MMBUF_BLOCK1_LINE_NUM;
		} else {
			HISI_FB_ERR("hfbcd_block_type=%d no support!\n", layer->img.hfbcd_block_type);
			return -EINVAL;
		}

		hisifd->mmbuf_info->mm_size0_y8[chn_idx] = hfbcd_rect.right * mmbuf_line_num;
		hisifd->mmbuf_info->mm_size1_c8[chn_idx] = hisifd->mmbuf_info->mm_size0_y8[chn_idx] / 2;
		if (is_pixel_10bit) {
			if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
				hfbcd_rect.left = ALIGN_DOWN((uint32_t)src_rect.y, MMBUF_ADDR_ALIGN);
				hfbcd_rect.right = ALIGN_UP(ALIGN_UP((uint32_t)(src_rect.y - hfbcd_rect.left + src_rect.h), MMBUF_ADDR_ALIGN) / 4,
					MMBUF_ADDR_ALIGN);
			} else {
				hfbcd_rect.left = ALIGN_DOWN((uint32_t)src_rect.x, MMBUF_ADDR_ALIGN);
				hfbcd_rect.right = ALIGN_UP(ALIGN_UP((uint32_t)(src_rect.x - hfbcd_rect.left + src_rect.w), MMBUF_ADDR_ALIGN) / 4,
					MMBUF_ADDR_ALIGN);
			}

			hisifd->mmbuf_info->mm_size2_y2[chn_idx] = hfbcd_rect.right * mmbuf_line_num;
			hisifd->mmbuf_info->mm_size3_c2[chn_idx] = hisifd->mmbuf_info->mm_size2_y2[chn_idx] / 2;
		}

		hisifd->mmbuf_info->mm_size[chn_idx] = hisifd->mmbuf_info->mm_size0_y8[chn_idx] +
			hisifd->mmbuf_info->mm_size1_c8[chn_idx] + hisifd->mmbuf_info->mm_size2_y2[chn_idx] +
			hisifd->mmbuf_info->mm_size3_c2[chn_idx];

		hisifd->mmbuf_info->mm_base[chn_idx] = hisi_dss_mmbuf_alloc(g_mmbuf_gen_pool,
			hisifd->mmbuf_info->mm_size[chn_idx]);
		hisifd->mmbuf_info->mm_base0_y8[chn_idx] = hisifd->mmbuf_info->mm_base[chn_idx];
		hisifd->mmbuf_info->mm_base1_c8[chn_idx] = hisifd->mmbuf_info->mm_base0_y8[chn_idx] + hisifd->mmbuf_info->mm_size0_y8[chn_idx];
		hisifd->mmbuf_info->mm_base2_y2[chn_idx] = hisifd->mmbuf_info->mm_base1_c8[chn_idx] + hisifd->mmbuf_info->mm_size1_c8[chn_idx];
		hisifd->mmbuf_info->mm_base3_c2[chn_idx] = hisifd->mmbuf_info->mm_base2_y2[chn_idx] + hisifd->mmbuf_info->mm_size2_y2[chn_idx];

		if ((hisifd->mmbuf_info->mm_base0_y8[chn_idx] < MMBUF_BASE) ||
			(hisifd->mmbuf_info->mm_base1_c8[chn_idx] < MMBUF_BASE)) {
			HISI_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base0_y8=0x%x, mm_base1_c8=0x%x.\n",
				hisifd->index, chn_idx, hisifd->mmbuf_info->mm_base0_y8[chn_idx], hisifd->mmbuf_info->mm_base1_c8[chn_idx]);
				return -EINVAL;
		}

		if (is_pixel_10bit) {
			if ((hisifd->mmbuf_info->mm_base2_y2[chn_idx] < MMBUF_BASE) ||
				(hisifd->mmbuf_info->mm_base3_c2[chn_idx] < MMBUF_BASE)) {
				HISI_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base2_y2=0x%x, mm_base3_c2=0x%x.\n",
					hisifd->index, chn_idx, hisifd->mmbuf_info->mm_base2_y2[chn_idx], hisifd->mmbuf_info->mm_base3_c2[chn_idx]);
				return -EINVAL;
			}
		}
	}

	hisifd->mmbuf_info->mm_used[chn_idx] = 1;
	return 0;
}

static int hisi_dss_hfbcd_config_check_mm_base0(struct hisi_fb_data_type *hisifd, uint32_t mm_base0_y8,
	uint32_t mm_base1_c8, int chn_idx, int32_t layer_idx)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL, return!");
		return -EINVAL;
	}

	if ((mm_base0_y8 & (MMBUF_ADDR_ALIGN - 1)) || (hisifd->mmbuf_info->mm_size0_y8[chn_idx] & (MMBUF_ADDR_ALIGN - 1)) ||
		(mm_base1_c8 & (MMBUF_ADDR_ALIGN - 1)) || (hisifd->mmbuf_info->mm_size1_c8[chn_idx] & (MMBUF_ADDR_ALIGN - 1))) {
		HISI_FB_ERR("layer%d mm_base0_y8(0x%x) or mm_size0_y8(0x%x) or"
			"mm_base1_c8(0x%x) or mm_size1_c8(0x%x) is not %d bytes aligned!\n",
			layer_idx, mm_base0_y8, hisifd->mmbuf_info->mm_size0_y8[chn_idx],
			mm_base1_c8, hisifd->mmbuf_info->mm_size1_c8[chn_idx], MMBUF_ADDR_ALIGN);
		return -EINVAL;
	}
	return 0;
}

static int hisi_dss_hfbcd_config_check_mm_base2(struct hisi_fb_data_type *hisifd, uint32_t mm_base2_y2,
	uint32_t mm_base3_c2, int chn_idx, int32_t layer_idx)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL, return!");
		return -EINVAL;
	}

	if ((mm_base2_y2 & (MMBUF_ADDR_ALIGN - 1)) || (hisifd->mmbuf_info->mm_size2_y2[chn_idx] & (MMBUF_ADDR_ALIGN - 1)) ||
		(mm_base3_c2 & (MMBUF_ADDR_ALIGN - 1)) || (hisifd->mmbuf_info->mm_size3_c2[chn_idx] & (MMBUF_ADDR_ALIGN - 1))) {
		HISI_FB_ERR("layer%d mm_base2_y2(0x%x) or mm_size2_y2(0x%x) or"
			"mm_base3_c2(0x%x) or mm_size3_c2(0x%x) is not %d bytes aligned!\n",
			layer_idx, mm_base2_y2, hisifd->mmbuf_info->mm_size2_y2[chn_idx],
			mm_base3_c2, hisifd->mmbuf_info->mm_size3_c2[chn_idx], MMBUF_ADDR_ALIGN);
		return -EINVAL;
	}
	return 0;
}

int hisi_dss_hfbcd_config(struct hisi_fb_data_type *hisifd, int ovl_idx,
	dss_layer_t *layer, dss_rect_ltrb_t *clip_rect,
	dss_rect_t *out_aligned_rect, bool *rdma_stretch_enable)
{
	dss_rdma_t *dma = NULL;
	bool mmu_enable = false;
	bool is_yuv_semi_planar = false;
	bool is_yuv_planar = false;
	int rdma_format;
	int rdma_transform;
	uint32_t stretch_size_vrt = 0;
	uint32_t stretched_line_num;

	int bpp;
	int aligned_pixel;
	int rdma_oft_x0;
	int rdma_oft_x1;
	int chn_idx;
	int ret;
	dss_rect_ltrb_t aligned_rect = {0, 0, 0, 0};
	dss_rect_t src_rect;
	uint32_t src_bottom;
	uint32_t src_right;

	bool is_pixel_10bit = false;
	uint32_t hfbcd_header_addr0 = 0;
	uint32_t hfbcd_header_stride0 = 0;
	uint32_t hfbcd_header_addr1 = 0;
	uint32_t hfbcd_header_stride1 = 0;
	uint32_t hfbcd_header_pointer_offset = 0;
	uint32_t hfbcd_top_crop_num = 0;
	uint32_t hfbcd_bottom_crop_num = 0;
	uint32_t hfbcd_crop_num_max = 0;
	uint32_t hfbcd_block_width_align = 0;
	uint32_t hfbcd_block_height_align = 0;
	uint32_t hfbcd_payload_addr0 = 0;
	uint32_t hfbcd_payload_stride0 = 0;
	uint32_t hfbcd_payload0_align = 0;
	uint32_t hfbcd_payload_addr1 = 0;
	uint32_t hfbcd_payload_stride1 = 0;
	uint32_t hfbcd_payload1_align = 0;
	uint32_t hfbcd_hreg_pic_width;
	uint32_t hfbcd_hreg_pic_height;

	uint32_t mm_base0_y8 = 0;
	uint32_t mm_base1_c8 = 0;
	uint32_t mm_base2_y2 = 0;
	uint32_t mm_base3_c2 = 0;
	uint32_t hfbcd_block_type;

	chn_idx = layer->chn_idx;
	src_rect = layer->src_rect;
	src_bottom = (uint32_t)src_rect.y + src_rect.h;
	src_right = (uint32_t)src_rect.x + src_rect.w;

	stretched_line_num = isNeedRdmaStretchBlt(hisifd, layer);
	*rdma_stretch_enable = (stretched_line_num > 0) ? true : false;

	mmu_enable = (layer->img.mmu_enable == 1) ? true : false;
	is_yuv_semi_planar = isYUVSemiPlanar(layer->img.format);
	is_yuv_planar = isYUVPlanar(layer->img.format);

	rdma_format = hisi_pixel_format_hal2dma(layer->img.format);
	if (rdma_format < 0) {
		HISI_FB_ERR("layer format(%d) not support !\n", layer->img.format);
		return -EINVAL;
	}

	rdma_transform = hisi_transform_hal2dma(layer->transform, chn_idx);
	if (rdma_transform < 0) {
		HISI_FB_ERR("layer transform(%d) not support!\n", layer->transform);
		return -EINVAL;
	}

	is_pixel_10bit = isPixel10Bit2dma(rdma_format);
	bpp = (is_yuv_semi_planar || is_yuv_planar) ? 1 : layer->img.bpp;
	if (is_pixel_10bit) {
		bpp = layer->img.bpp;
	}

	aligned_pixel = DMA_ALIGN_BYTES / bpp;

	dma = &(hisifd->dss_module.rdma[chn_idx]);
	hisifd->dss_module.dma_used[chn_idx] = 1;

	hfbcd_block_type = layer->img.hfbcd_block_type;
	if ((layer->img.hfbc_mmbuf_base0_y8 > 0) && (layer->img.hfbc_mmbuf_base1_c8 > 0)) {
		mm_base0_y8 = layer->img.hfbc_mmbuf_base0_y8;
		mm_base1_c8 = layer->img.hfbc_mmbuf_base1_c8;
		mm_base2_y2 = layer->img.hfbc_mmbuf_base2_y2;
		mm_base3_c2 = layer->img.hfbc_mmbuf_base3_c2;
	} else {
		ret = hisi_dss_hfbcd_mmbuf_config(hisifd, ovl_idx, layer, hfbcd_block_type, is_pixel_10bit);
		if (ret < 0) {
			HISI_FB_ERR("hisi_dss_hfbcd_mmbuf_config fail!\n");
			return -EINVAL;
		}

		mm_base0_y8 = hisifd->mmbuf_info->mm_base0_y8[chn_idx];
		mm_base1_c8 = hisifd->mmbuf_info->mm_base1_c8[chn_idx];
		if (is_pixel_10bit) {
			mm_base2_y2 = hisifd->mmbuf_info->mm_base2_y2[chn_idx];
			mm_base3_c2 = hisifd->mmbuf_info->mm_base3_c2[chn_idx];
		}
	}

	mm_base0_y8 -= MMBUF_BASE;
	mm_base1_c8 -= MMBUF_BASE;

	ret = hisi_dss_hfbcd_config_check_mm_base0(hisifd, mm_base0_y8, mm_base1_c8, chn_idx, layer->layer_idx);
	if (ret == -EINVAL) {
		return ret;
	}

	if (is_pixel_10bit) {
		mm_base2_y2 -= MMBUF_BASE;
		mm_base3_c2 -= MMBUF_BASE;

		ret = hisi_dss_hfbcd_config_check_mm_base2(hisifd, mm_base2_y2, mm_base3_c2, chn_idx, layer->layer_idx);
		if (ret == -EINVAL) {
			return ret;
		}
	}

	if (hfbcd_block_type == 0) {
		hfbcd_block_width_align = HFBC_BLOCK0_WIDTH_ALIGN;
		hfbcd_block_height_align = HFBC_BLOCK0_HEIGHT_ALIGN;
	} else if (hfbcd_block_type == 1) {
		hfbcd_block_width_align = HFBC_BLOCK1_WIDTH_ALIGN;
		hfbcd_block_height_align = HFBC_BLOCK1_HEIGHT_ALIGN;
	} else {
		HISI_FB_ERR("hfbcd_block_type=%d no support!\n", layer->img.hfbcd_block_type);
		return -EINVAL;
	}

	//aligned rect
	aligned_rect.left = ALIGN_DOWN((uint32_t)src_rect.x, hfbcd_block_width_align);
	aligned_rect.right = ALIGN_UP(src_right, hfbcd_block_width_align) - 1; //lint !e502
	aligned_rect.top = ALIGN_DOWN((uint32_t)src_rect.y, hfbcd_block_height_align);
	aligned_rect.bottom = ALIGN_UP(src_bottom, hfbcd_block_height_align) - 1; //lint !e502

	hfbcd_hreg_pic_width = aligned_rect.right - aligned_rect.left;
	hfbcd_hreg_pic_height = aligned_rect.bottom - aligned_rect.top;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if ((hfbcd_hreg_pic_width > HFBC_PIC_WIDTH_ROT_MAX) || (hfbcd_hreg_pic_height > HFBC_PIC_HEIGHT_ROT_MAX)) {
			HISI_FB_ERR("layer%d hfbcd_hreg_pic_width(%d), hfbcd_hreg_pic_height(%d) is too large(4096*2160)!\n",
				layer->layer_idx, hfbcd_hreg_pic_width, hfbcd_hreg_pic_height);
			return -EINVAL;
		}
	}

	if ((layer->img.width & (hfbcd_block_width_align - 1)) ||
		(layer->img.height & (hfbcd_block_height_align - 1))) {
		HISI_FB_ERR("layer%d img width(%d) is not %d bytes aligned, or "
			"img heigh(%d) is not %d bytes aligned!\n",
			layer->layer_idx, layer->img.width, hfbcd_block_width_align,
			layer->img.height, hfbcd_block_height_align);
		return -EINVAL;
	}

	if (hfbcd_block_type == 0) {
		if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
			hfbcd_crop_num_max = HFBCD_BLOCK0_ROT_CROP_MAX;
		} else {
			hfbcd_crop_num_max = HFBCD_BLOCK0_CROP_MAX;
		}
	} else {
		hfbcd_crop_num_max = HFBCD_BLOCK1_CROP_MAX;
	}

	rdma_oft_x0 = aligned_rect.left / aligned_pixel;
	rdma_oft_x1 = aligned_rect.right / aligned_pixel;

	hfbcd_header_stride0 = layer->img.hfbc_header_stride0;
	hfbcd_header_pointer_offset = (aligned_rect.top / hfbcd_block_height_align) * hfbcd_header_stride0 +
		(aligned_rect.left / hfbcd_block_width_align) * HFBC_HEADER_STRIDE_BLOCK;

	hfbcd_header_addr0 = layer->img.hfbc_header_addr0 + hfbcd_header_pointer_offset;
	hfbcd_header_addr1 = layer->img.hfbc_header_addr1 + hfbcd_header_pointer_offset;
	hfbcd_header_stride1 = hfbcd_header_stride0;
	if ((hfbcd_header_addr0 & (HFBC_HEADER_ADDR_ALIGN - 1)) ||(hfbcd_header_stride0 & (HFBC_HEADER_STRIDE_ALIGN - 1)) ||
		(hfbcd_header_addr1 & (HFBC_HEADER_ADDR_ALIGN - 1))) {
		HISI_FB_ERR("layer%d hfbcd_header_addr0(0x%x) or hfbcd_header_addr1(0x%x) is not %d bytes aligned or "
			"hfbcd_header_stride0(0x%x) is not %d bytes aligned!\n",
			layer->layer_idx, hfbcd_header_addr0, hfbcd_header_addr1, HFBC_HEADER_ADDR_ALIGN,
			hfbcd_header_stride0, HFBC_HEADER_STRIDE_ALIGN);
		return -EINVAL;
	}

	//hfbcd payload
	if (is_pixel_10bit) {
		hfbcd_payload0_align = HFBC_PAYLOAD_ALIGN_10BIT;
		hfbcd_payload1_align = HFBC_PAYLOAD_ALIGN_10BIT;
	} else {
		hfbcd_payload0_align = HFBC_PAYLOAD0_ALIGN_8BIT;
		hfbcd_payload1_align = HFBC_PAYLOAD1_ALIGN_8BIT;
	}

	hfbcd_payload_stride0 = layer->img.hfbc_payload_stride0;
	hfbcd_payload_stride1 = layer->img.hfbc_payload_stride1;

	hfbcd_payload_addr0 = layer->img.hfbc_payload_addr0 +
		(aligned_rect.top / hfbcd_block_height_align) * hfbcd_payload_stride0 +
		(aligned_rect.left / hfbcd_block_width_align) * hfbcd_payload0_align;

	hfbcd_payload_addr1 = layer->img.hfbc_payload_addr1 +
		(aligned_rect.top / hfbcd_block_height_align) * hfbcd_payload_stride1 +
		(aligned_rect.left / hfbcd_block_width_align) * hfbcd_payload1_align;

	if (is_pixel_10bit) {
		if ((hfbcd_payload_addr0 != hfbcd_payload_addr1) || (hfbcd_payload_stride0 != hfbcd_payload_stride1)) {
			HISI_FB_ERR("layer%d 10bit hfbcd_payload_addr0(0x%x) is not equal to hfbcd_payload_addr1(0x%x) or "
				"hfbcd_payload_stride0(0x%x) is not equal to hfbcd_payload_stride1(0x%x)!\n",
				layer->layer_idx, hfbcd_payload_addr0, hfbcd_payload_addr1,
				hfbcd_payload_stride0, hfbcd_payload_stride1);
			return -EINVAL;
		}
	}

	if ((hfbcd_payload_addr0 & (hfbcd_payload0_align - 1)) || (hfbcd_payload_stride0 & (hfbcd_payload0_align - 1)) ||
		(hfbcd_payload_addr1 & (hfbcd_payload1_align - 1)) || (hfbcd_payload_stride1 & (hfbcd_payload1_align - 1))) {
		HISI_FB_ERR("layer%d hfbc_payload_addr0(0x%x) or hfbc_payload_stride0(0x%x) is not %d bytes aligned, or "
			"hfbc_payload_addr1(0x%x) or hfbc_payload_stride1(0x%x) is not %d bytes aligned!\n",
			layer->layer_idx, hfbcd_payload_addr0, hfbcd_payload_stride0, hfbcd_payload0_align,
			hfbcd_payload_addr1, hfbcd_payload_stride1, hfbcd_payload1_align);
		return -EINVAL;
	}

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		aligned_rect.left = ALIGN_DOWN((uint32_t)src_rect.y, hfbcd_block_height_align);
		aligned_rect.right = ALIGN_UP(src_bottom, hfbcd_block_height_align) - 1; //lint !e502
		aligned_rect.top = ALIGN_DOWN((uint32_t)src_rect.x, hfbcd_block_width_align);
		aligned_rect.bottom = ALIGN_UP(src_right, hfbcd_block_width_align) - 1; //lint !e502
	}

	//out_aligned_rect
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = aligned_rect.right - aligned_rect.left + 1;
	out_aligned_rect->h = aligned_rect.bottom - aligned_rect.top + 1;

	// rdfc clip_rect
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		clip_rect->left = src_rect.y - aligned_rect.left;
		clip_rect->right = aligned_rect.right - DSS_WIDTH(src_bottom);
		clip_rect->top = src_rect.x - aligned_rect.top;
		clip_rect->bottom = aligned_rect.bottom - DSS_HEIGHT(src_right);
	} else {
		clip_rect->left = src_rect.x - aligned_rect.left;
		clip_rect->right = aligned_rect.right - DSS_WIDTH(src_right);
		clip_rect->top = src_rect.y - aligned_rect.top;
		clip_rect->bottom = aligned_rect.bottom - DSS_HEIGHT(src_bottom);
	}
	if (hisi_adjust_clip_rect(layer, clip_rect) < 0) {
		HISI_FB_ERR("clip rect invalid => layer_idx=%d, chn_idx=%d, clip_rect(%d, %d, %d, %d).\n",
			layer->layer_idx, chn_idx, clip_rect->left, clip_rect->right,
			clip_rect->top, clip_rect->bottom);
		return -EINVAL;
	}
	/*lint -e574*/
	//hfbcd crop
	hfbcd_top_crop_num = (clip_rect->top > hfbcd_crop_num_max) ? hfbcd_crop_num_max : clip_rect->top;
	hfbcd_bottom_crop_num = (clip_rect->bottom > hfbcd_crop_num_max) ? hfbcd_crop_num_max : clip_rect->bottom;
	/*lint +e574*/
	clip_rect->top -= hfbcd_top_crop_num;
	clip_rect->bottom -= hfbcd_bottom_crop_num;

	//adjust out_aligned_rect
	out_aligned_rect->h -= (hfbcd_top_crop_num + hfbcd_bottom_crop_num);
	stretch_size_vrt = DSS_HEIGHT(out_aligned_rect->h);
	stretched_line_num = 0;

	dma->hfbcd_used = 1;
	dma->oft_x0 = set_bits32(dma->oft_x0, rdma_oft_x0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, rdma_oft_x1, 16, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt,
		(stretch_size_vrt | (stretched_line_num << 13)), 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, rdma_transform, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, (*rdma_stretch_enable ? 1 : 0), 1, 12);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 2);

	dma->hfbcd_hreg_hdr_ptr_l0 = set_bits32(dma->hfbcd_hreg_hdr_ptr_l0,
		hfbcd_header_addr0, 32, 0);
	dma->hfbcd_hreg_pic_width = set_bits32(dma->hfbcd_hreg_pic_width,
		hfbcd_hreg_pic_width, 16, 0);
	dma->hfbcd_hreg_pic_height = set_bits32(dma->hfbcd_hreg_pic_height,
		hfbcd_hreg_pic_height, 16, 0);
	dma->hfbcd_line_crop = set_bits32(dma->hfbcd_line_crop,
		(hfbcd_top_crop_num << 8 | hfbcd_bottom_crop_num), 16, 0);
	dma->hfbcd_input_header_stride0 = set_bits32(dma->hfbcd_input_header_stride0, hfbcd_header_stride0, 14, 0);
	dma->hfbcd_hreg_hdr_ptr_l1 = set_bits32(dma->hfbcd_hreg_hdr_ptr_l1, hfbcd_header_addr1, 32, 0);
	dma->hfbcd_header_stride1 = set_bits32(dma->hfbcd_header_stride1, hfbcd_header_stride1, 14, 0);
	dma->hfbcd_mm_base0_y8 = set_bits32(dma->hfbcd_mm_base0_y8, mm_base0_y8, 32, 0);
	dma->hfbcd_mm_base1_c8 = set_bits32(dma->hfbcd_mm_base1_c8, mm_base1_c8, 32, 0);
	dma->hfbcd_mm_base2_y2 = set_bits32(dma->hfbcd_mm_base2_y2, mm_base2_y2, 32, 0);
	dma->hfbcd_mm_base3_c2 = set_bits32(dma->hfbcd_mm_base3_c2, mm_base3_c2, 32, 0);
	dma->hfbcd_payload_pointer = set_bits32(dma->hfbcd_payload_pointer,
		hfbcd_payload_addr0, 32, 0);
	dma->hfbcd_payload_stride0 = set_bits32(dma->hfbcd_payload_stride0,
		hfbcd_payload_stride0, 20, 0);
	dma->hfbcd_hreg_pld_ptr_l1 = set_bits32(dma->hfbcd_hreg_pld_ptr_l1,
		hfbcd_payload_addr1, 32, 0);
	dma->hfbcd_payload_stride1 = set_bits32(dma->hfbcd_payload_stride1,
		hfbcd_payload_stride1, 20, 0);
	dma->hfbcd_creg_fbcd_ctrl_mode = set_bits32(dma->hfbcd_creg_fbcd_ctrl_mode, 1, 2, 0);
	//hfbcd_scramble_mode
	dma->hfbcd_scramble_mode = set_bits32(dma->hfbcd_scramble_mode,
		layer->img.hfbc_scramble_mode, 4, 2);
	dma->hfbcd_block_type = set_bits32(dma->hfbcd_block_type, hfbcd_block_type, 2, 0);

	if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer) {
		HISI_FB_INFO("fb%d, mm_base0_y8=0x%x, mm_base2_y2=0x%x, mm_base1_c8=0x%x, mm_base3_c2=0x%x,"
			"mm_base0_y8_size=%d,mm_base2_y2_size=%d,mm_base1_c8_size=%d,mm_base3_c2_size=%d,"
			"aligned_rect(%d,%d,%d,%d), hfbcd_block_type=%d!\n",
			hisifd->index, mm_base0_y8, mm_base2_y2, mm_base1_c8, mm_base3_c2, layer->img.hfbc_mmbuf_size0_y8,
			layer->img.hfbc_mmbuf_size2_y2, layer->img.hfbc_mmbuf_size1_c8, layer->img.hfbc_mmbuf_size3_c2,
			aligned_rect.left, aligned_rect.top, aligned_rect.right, aligned_rect.bottom, hfbcd_block_type);
	}

	return 0;

}

int hisi_dss_hfbce_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req,
	dss_wb_layer_t *layer, dss_rect_t aligned_rect, dss_rect_t *ov_block_rect, bool last_block)
{
	dss_wdma_t *wdma = NULL;
	int chn_idx = 0;
	int wdma_format = 0;
	int wdma_transform = 0;

	uint32_t wdma_addr = 0;
	dss_rect_t in_rect;
	int aligned_pixel = 0;
	bool mmu_enable = false;

	bool is_pixel_10bit = false;
	dss_rect_ltrb_t hfbce_header_rect = {0};
	dss_rect_ltrb_t hfbce_payload_rect = {0};
	uint32_t hfbce_hreg_pic_blks = 0;
	uint32_t hfbce_header_addr0 = 0;
	uint32_t hfbce_header_stride0 = 0;
	uint32_t hfbce_header_pointer_offset = 0;
	uint32_t hfbce_block_width_align = 0;
	uint32_t hfbce_block_height_align = 0;
	uint32_t hfbce_payload_addr0 = 0;
	uint32_t hfbce_payload_stride0 = 0;
	uint32_t hfbce_header_addr1 = 0;
	uint32_t hfbce_header_stride1 = 0;
	uint32_t hfbce_payload_addr1 = 0;
	uint32_t hfbce_payload_stride1 = 0;
	uint32_t hfbce_payload0_align = 0;
	uint32_t hfbce_payload1_align = 0;

	if (NULL == hisifd || NULL == pov_req || NULL == layer) {
		HISI_FB_ERR("NULL ptr\n");
		return -EINVAL;
	}

	chn_idx = layer->chn_idx;

	wdma = &(hisifd->dss_module.wdma[chn_idx]);
	hisifd->dss_module.dma_used[chn_idx] = 1;

	wdma_format = hisi_pixel_format_hal2dma(layer->dst.format);
	if (wdma_format < 0) {
		HISI_FB_ERR("hisi_pixel_format_hal2dma failed!\n");
		return -EINVAL;
	}

	in_rect = aligned_rect;
	aligned_pixel = DMA_ALIGN_BYTES / layer->dst.bpp;

	wdma_transform = hisi_transform_hal2dma(layer->transform, chn_idx);
	if (wdma_transform < 0) {
		HISI_FB_ERR("hisi_transform_hal2dma failed!\n");
		return -EINVAL;
	}

	is_pixel_10bit = isPixel10Bit2dma(wdma_format);
	mmu_enable = (layer->dst.mmu_enable == 1) ? true : false;
	wdma_addr = mmu_enable ? layer->dst.vir_addr : layer->dst.phy_addr;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		hfbce_block_width_align = HFBC_BLOCK1_WIDTH_ALIGN;
		hfbce_block_height_align = HFBC_BLOCK1_HEIGHT_ALIGN;
	} else {
		hfbce_block_width_align = HFBC_BLOCK0_WIDTH_ALIGN;
		hfbce_block_height_align = HFBC_BLOCK0_HEIGHT_ALIGN;
	}

	if ((layer->dst.width & (hfbce_block_width_align - 1)) ||
		(layer->dst.height & (hfbce_block_height_align - 1))) {
		HISI_FB_ERR("wb_layer dst width(%d) is not %d bytes aligned, or img heigh(%d) is not %d bytes aligned!\n",
			layer->dst.width, hfbce_block_width_align, layer->dst.height, hfbce_block_height_align);
		return -EINVAL;
	}

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if ((in_rect.w < HFBC_PIC_WIDTH_ROT_MIN) || (in_rect.w > HFBC_PIC_WIDTH_MAX) ||
			(in_rect.h < HFBC_PIC_HEIGHT_ROT_MIN) || (in_rect.h > HFBC_PIC_HEIGHT_MAX) ||
			((uint32_t)in_rect.w & (HFBC_BLOCK1_HEIGHT_ALIGN - 1)) || ((uint32_t)in_rect.h & (HFBC_BLOCK1_WIDTH_ALIGN - 1))) {
			HISI_FB_ERR("hfbce in_rect(%d,%d, %d,%d) is out of range!",
				in_rect.x, in_rect.y, in_rect.w, in_rect.h);
			return -EINVAL;
		}
	} else {
		if ((in_rect.w < HFBC_PIC_WIDTH_MIN) || (in_rect.w > HFBC_PIC_WIDTH_MAX) ||
			(in_rect.h < HFBC_PIC_HEIGHT_MIN) || (in_rect.h > HFBC_PIC_HEIGHT_MAX) ||
			((uint32_t)in_rect.w & (HFBC_BLOCK0_WIDTH_ALIGN - 1)) || ((uint32_t)in_rect.h & (HFBC_BLOCK0_HEIGHT_ALIGN - 1))) {
			HISI_FB_ERR("hfbce in_rect(%d,%d, %d,%d) is out of range!",
				in_rect.x, in_rect.y, in_rect.w, in_rect.h);
			return -EINVAL;
		}
	}
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		hfbce_header_rect.left = ALIGN_DOWN((uint32_t)layer->dst_rect.x, hfbce_block_width_align);
		hfbce_header_rect.top = ALIGN_DOWN((uint32_t)(layer->dst_rect.y + (ov_block_rect->x - layer->dst_rect.x)),
			hfbce_block_height_align);

		hfbce_payload_rect.left = ALIGN_DOWN((uint32_t)layer->dst_rect.x, hfbce_block_width_align);
		hfbce_payload_rect.top = hfbce_header_rect.top;

		hfbce_hreg_pic_blks = (in_rect.h / hfbce_block_width_align) * (in_rect.w / hfbce_block_height_align) - 1;
	} else {
		hfbce_header_rect.left = ALIGN_DOWN((uint32_t)in_rect.x, hfbce_block_width_align);
		hfbce_header_rect.top = ALIGN_DOWN((uint32_t)in_rect.y, hfbce_block_height_align);

		hfbce_payload_rect.left = ALIGN_DOWN((uint32_t)in_rect.x, hfbce_block_width_align);
		hfbce_payload_rect.top = hfbce_header_rect.top;

		hfbce_hreg_pic_blks = (in_rect.w / hfbce_block_width_align) * (in_rect.h / hfbce_block_height_align) - 1;
	}
	//hfbc header
	hfbce_header_stride0 = layer->dst.hfbc_header_stride0;
	hfbce_header_pointer_offset = (hfbce_header_rect.top / hfbce_block_height_align) * hfbce_header_stride0 +
		(hfbce_header_rect.left / hfbce_block_width_align) * HFBC_HEADER_STRIDE_BLOCK;

	hfbce_header_addr0 = layer->dst.hfbc_header_addr0 + hfbce_header_pointer_offset;
	hfbce_header_addr1 = layer->dst.hfbc_header_addr1 + hfbce_header_pointer_offset;
	hfbce_header_stride1 = hfbce_header_stride0;
	if ((hfbce_header_addr0 & (HFBC_HEADER_ADDR_ALIGN - 1)) || (hfbce_header_stride0 & (HFBC_HEADER_STRIDE_ALIGN - 1))
		|| (hfbce_header_addr1 & (HFBC_HEADER_ADDR_ALIGN - 1))) {
		HISI_FB_ERR("wb_layer hfbce_header_addr0(0x%x) or hfbcd_header_addr1(0x%x) is not %d bytes aligned, or "
			"hfbce_header_stride0(0x%x) is not %d bytes aligned!\n",
			hfbce_header_addr0, hfbce_header_addr1, HFBC_HEADER_ADDR_ALIGN,
			hfbce_header_stride0, HFBC_HEADER_STRIDE_ALIGN);
		return -EINVAL;
	}

	//hfbc payload
	if (is_pixel_10bit) {
		hfbce_payload0_align = HFBC_PAYLOAD_ALIGN_10BIT;
		hfbce_payload1_align = HFBC_PAYLOAD_ALIGN_10BIT;
	} else {
		hfbce_payload0_align = HFBC_PAYLOAD0_ALIGN_8BIT;
		hfbce_payload1_align = HFBC_PAYLOAD1_ALIGN_8BIT;
	}
	hfbce_payload_stride0 = layer->dst.hfbc_payload_stride0;
	hfbce_payload_stride1 = layer->dst.hfbc_payload_stride1;

	hfbce_payload_addr0 = layer->dst.hfbc_payload_addr0 +
		(hfbce_payload_rect.top / hfbce_block_height_align) * hfbce_payload_stride0 +
		(hfbce_payload_rect.left / hfbce_block_width_align) * hfbce_payload0_align;

	hfbce_payload_addr1 = layer->dst.hfbc_payload_addr1 +
		(hfbce_payload_rect.top / hfbce_block_height_align) * hfbce_payload_stride1 +
		(hfbce_payload_rect.left / hfbce_block_width_align) * hfbce_payload1_align;

	if (is_pixel_10bit) {
		if ((hfbce_payload_addr0 != hfbce_payload_addr1) || (hfbce_payload_stride0 != hfbce_payload_stride1)) {
			HISI_FB_ERR("ch%d 10bit hfbce_payload_addr0(0x%x) is not equal to hfbce_payload_addr1(0x%x) or "
				"hfbce_payload_stride0(0x%x) is not equal to hfbce_payload_stride1(0x%x)!\n",
				layer->chn_idx, hfbce_payload_addr0, hfbce_payload_addr1,
				hfbce_payload_stride0, hfbce_payload_stride1);
			return -EINVAL;
		}
	}

	if ((hfbce_payload_addr0 & (hfbce_payload0_align - 1)) || (hfbce_payload_stride0 & (hfbce_payload0_align - 1)) ||
		(hfbce_payload_addr1 & (hfbce_payload1_align - 1)) || (hfbce_payload_stride1 & (hfbce_payload1_align - 1))) {
		HISI_FB_ERR("ch%d hfbce_payload_addr0(0x%x) or hfbce_payload_stride0(0x%x) is not %d bytes aligned, or "
			"hfbce_payload_addr1(0x%x) or hfbce_payload_stride1(0x%x) is not %d bytes aligned!\n",
			layer->chn_idx, hfbce_payload_addr0, hfbce_payload_stride0, hfbce_payload0_align,
			hfbce_payload_addr1, hfbce_payload_stride1, hfbce_payload1_align);
		return -EINVAL;
	}

	wdma->hfbce_used = 1;
	wdma->ctrl = set_bits32(wdma->ctrl, wdma_format, 5, 3);
	wdma->ctrl = set_bits32(wdma->ctrl, (mmu_enable ? 0x1 : 0x0), 1, 8);
	wdma->ctrl = set_bits32(wdma->ctrl, wdma_transform, 3, 9);
	if (last_block) {
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0x1d, 5, 0);
	} else {
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0xd, 5, 0);
	}

	wdma->rot_size = set_bits32(wdma->rot_size,
		(DSS_WIDTH((uint32_t)in_rect.w) | (DSS_HEIGHT((uint32_t)in_rect.h) << 16)), 32, 0);

	wdma->hfbce_hreg_pic_blks = set_bits32(wdma->hfbce_hreg_pic_blks, hfbce_hreg_pic_blks, 24, 0);
	wdma->hfbce_hreg_hdr_ptr_l0 = set_bits32(wdma->hfbce_hreg_hdr_ptr_l0, hfbce_header_addr0, 32, 0);
	wdma->hfbce_hreg_pld_ptr_l0 = set_bits32(wdma->hfbce_hreg_pld_ptr_l0, hfbce_payload_addr0, 32, 0);
	wdma->hfbce_picture_size = set_bits32(wdma->hfbce_picture_size,
		((DSS_WIDTH((uint32_t)in_rect.w) << 16) | DSS_HEIGHT((uint32_t)in_rect.h)), 32, 0);
	wdma->hfbce_header_stride0 = set_bits32(wdma->hfbce_header_stride0, hfbce_header_stride0, 14, 0);
	wdma->hfbce_payload_stride0 = set_bits32(wdma->hfbce_payload_stride0, hfbce_payload_stride0, 20, 0);
	wdma->hfbce_scramble_mode = set_bits32(wdma->hfbce_scramble_mode,
		layer->dst.hfbc_scramble_mode, 4, 2);
	wdma->hfbce_header_pointer_offset = set_bits32(wdma->hfbce_header_pointer_offset,
		hfbce_header_pointer_offset, 32, 0);
	wdma->fbce_creg_fbce_ctrl_mode = set_bits32(wdma->fbce_creg_fbce_ctrl_mode, 1, 32, 0);
	wdma->hfbce_hreg_hdr_ptr_l1 = set_bits32(wdma->hfbce_hreg_hdr_ptr_l1, hfbce_header_addr1, 32, 0);
	wdma->hfbce_hreg_pld_ptr_l1 = set_bits32(wdma->hfbce_hreg_pld_ptr_l1, hfbce_payload_addr1, 32, 0);
	wdma->hfbce_header_stride1 = set_bits32(wdma->hfbce_header_stride1, hfbce_header_stride1, 14, 0);
	wdma->hfbce_payload_stride1 = set_bits32(wdma->hfbce_payload_stride1, hfbce_payload_stride1, 20, 0);

	if (g_debug_ovl_offline_composer) {
		HISI_FB_INFO("aligned_rect(%d,%d,%d,%d), hfbce_rect(%d,%d,%d,%d)!\n",
			in_rect.x, in_rect.y, DSS_WIDTH(in_rect.x + in_rect.w), DSS_WIDTH(in_rect.y + in_rect.h),
			hfbce_payload_rect.left, hfbce_payload_rect.top, hfbce_payload_rect.right, hfbce_payload_rect.bottom);
	}

	return 0;
}

void hisi_dss_hfbcd_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	if ((hisifd == NULL) || (dma_base == NULL) || (s_dma == NULL)) {
		HISI_FB_DEBUG("hisifd or dma_base, s_dma is NULL!\n");
		return;
	}

	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_HDR_PTR_LO, s_dma->hfbcd_hreg_hdr_ptr_l0, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_PIC_WIDTH, s_dma->hfbcd_hreg_pic_width, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_PIC_HEIGHT, s_dma->hfbcd_hreg_pic_height, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_LINE_CROP, s_dma->hfbcd_line_crop, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_INPUT_HEADER_STRIDE, s_dma->hfbcd_input_header_stride0, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_PAYLOAD_STRIDE, s_dma->hfbcd_payload_stride0, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_AFBCD_PAYLOAD_POINTER, s_dma->hfbcd_payload_pointer, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_SCRAMBLE_MODE, s_dma->hfbcd_scramble_mode, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_CREG_FBCD_CTRL_MODE, s_dma->hfbcd_creg_fbcd_ctrl_mode, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_HDR_PTR_L1, s_dma->hfbcd_hreg_hdr_ptr_l1, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_PLD_PTR_L1, s_dma->hfbcd_hreg_pld_ptr_l1, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HEADER_SRTIDE_1, s_dma->hfbcd_header_stride1, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_PAYLOAD_SRTIDE_1, s_dma->hfbcd_payload_stride1, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_BLOCK_TYPE, s_dma->hfbcd_block_type, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_MM_BASE_0, s_dma->hfbcd_mm_base0_y8, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_MM_BASE_1, s_dma->hfbcd_mm_base1_c8, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_MM_BASE_2, s_dma->hfbcd_mm_base2_y2, 32 , 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_MM_BASE_3, s_dma->hfbcd_mm_base3_c2, 32 , 0);
}

void hisi_dss_hfbce_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *wdma_base, dss_wdma_t *s_wdma)
{
	if ((hisifd == NULL) || (wdma_base == NULL) || (s_wdma == NULL)) {
		HISI_FB_DEBUG("hisifd or wdma_base, s_wdma is NULL!\n");
		return;
	}

	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_PIC_BLKS, s_wdma->hfbce_hreg_pic_blks, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_HDR_PTR_L0, s_wdma->hfbce_hreg_hdr_ptr_l0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_PLD_PTR_L0, s_wdma->hfbce_hreg_pld_ptr_l0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_PICTURE_SIZE, s_wdma->hfbce_picture_size, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HEADER_SRTIDE, s_wdma->hfbce_header_stride0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_PAYLOAD_STRIDE, s_wdma->hfbce_payload_stride0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_SCRAMBLE_MODE, s_wdma->hfbce_scramble_mode, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HEADER_POINTER_OFFSET, s_wdma->hfbce_header_pointer_offset, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_CREG_FBCE_CTRL_MODE, s_wdma->fbce_creg_fbce_ctrl_mode, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_HDR_PTR_L1, s_wdma->hfbce_hreg_hdr_ptr_l1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_PLD_PTR_L1, s_wdma->hfbce_hreg_pld_ptr_l1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HEADER_SRTIDE_1, s_wdma->hfbce_header_stride1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_PAYLOAD_SRTIDE_1, s_wdma->hfbce_payload_stride1, 32, 0);
}

void hisi_dss_hebcd_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *dma_base, dss_rdma_t *s_dma)
{
	if ((hisifd == NULL) || (dma_base == NULL) || (s_dma == NULL)) {
		HISI_FB_DEBUG("hisifd or dma_base, s_dma is NULL!\n");
		return;
	}

	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_HDR_PTR_LO, s_dma->hebcd_hreg_hdr_ptr_l0, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_PIC_WIDTH, s_dma->hebcd_hreg_pic_width, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_PIC_HEIGHT, s_dma->hebcd_hreg_pic_height, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_FORMAT, s_dma->hebcd_hreg_format, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_STR, s_dma->hebcd_str, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_LINE_CROP, s_dma->hebcd_line_crop, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_INPUT_HEADER_STRIDE, s_dma->hebcd_input_header_stride0, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_PAYLOAD_STRIDE, s_dma->hebcd_payload_stride0, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_MM_BASE_0, s_dma->hebcd_mm_base0_y8, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_AFBCD_PAYLOAD_POINTER, s_dma->hebcd_payload_pointer, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HEIGHT_BF_STR, s_dma->hebcd_height_bf_str, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_SCRAMBLE_MODE, s_dma->hebcd_scramble_mode, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_CREG_FBCD_CTRL_MODE, s_dma->hebcd_creg_fbcd_ctrl_mode, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_HDR_PTR_L1, s_dma->hebcd_hreg_hdr_ptr_l1, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HREG_PLD_PTR_L1, s_dma->hebcd_hreg_pld_ptr_l1, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_HEADER_SRTIDE_1, s_dma->hebcd_header_stride1, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_PAYLOAD_SRTIDE_1, s_dma->hebcd_payload_stride1, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_BLOCK_TYPE, s_dma->hebcd_block_type, 32, 0);
	hisifd->set_reg(hisifd, dma_base + AFBCD_MM_BASE_1, s_dma->hebcd_mm_base1_c8, 32, 0);
}

static int hisi_dss_hebcd_config_check_mm_base0(struct hisi_fb_data_type *hisifd,
	uint32_t mm_base0_y8, uint32_t mm_base1_c8, int chn_idx, int32_t layer_idx, bool is_yuv_semi_planar)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL, return!");
		return -EINVAL;
	}

	if ((mm_base0_y8 & (HEBC_MMBUF_ADDR_ALIGN - 1)) || (hisifd->mmbuf_info->mm_size0_y8[chn_idx] &
		(HEBC_MMBUF_ADDR_ALIGN - 1))) {
		HISI_FB_ERR("layer%d mm_base0_y8 0x%x or mm_size0_y80x%x or "
			"mm_base1_c8 0x%x or mm_size1_c8 0x%x is not %d bytes aligned!\n",
			layer_idx, mm_base0_y8, hisifd->mmbuf_info->mm_size0_y8[chn_idx],
			mm_base1_c8, hisifd->mmbuf_info->mm_size1_c8[chn_idx], HEBC_MMBUF_ADDR_ALIGN);
		return -EINVAL;
	}

	if (is_yuv_semi_planar) {
		if ((mm_base1_c8 & (HEBC_MMBUF_ADDR_ALIGN - 1)) || (hisifd->mmbuf_info->mm_size1_c8[chn_idx] &
			(HEBC_MMBUF_ADDR_ALIGN - 1))) {
			HISI_FB_ERR("layer%d mm_base0_y8 0x%x or mm_size0_y8 0x%x or "
				"mm_base1_c8 0x%x or mm_size1_c8 0x%x is not %d bytes aligned!\n",
				layer_idx, mm_base0_y8, hisifd->mmbuf_info->mm_size0_y8[chn_idx],
				mm_base1_c8, hisifd->mmbuf_info->mm_size1_c8[chn_idx], HEBC_MMBUF_ADDR_ALIGN);
			return -EINVAL;
		}
	}

	return 0;
}

/*lint -save -e834 -e737 -e713 -e838 -e537 -e647 -e712 -e573 -e574 -e845 -e701*/
static int hisi_dss_hebcd_mmbuf_config(struct hisi_fb_data_type *hisifd, int ovl_idx,
	dss_layer_t *layer, uint32_t hebcd_block_type, bool is_yuv_semi_planar)
{
	int chn_idx;
	dss_rect_t src_rect;
	bool mm_alloc_needed = false;

	dss_rect_ltrb_t hebcd_rect;
	uint32_t mmbuf_y8_line_num = 0;
	uint32_t mmbuf_c8_line_num = 0;

	if (hisifd == NULL || layer == NULL)
		return -EINVAL;

	chn_idx = layer->chn_idx;
	src_rect = layer->src_rect;

	if (ovl_idx <= DSS_OVL1) {
		mm_alloc_needed = true;
	} else {
		if (hisifd->mmbuf_info->mm_used[chn_idx] == 1)
			mm_alloc_needed = false;
		else
			mm_alloc_needed = true;
	}

	if (mm_alloc_needed) {
		hebcd_rect.left = ALIGN_DOWN(src_rect.x, HEBC_MMBUF_ADDR_ALIGN);
		hebcd_rect.right = ALIGN_UP((src_rect.x - hebcd_rect.left + src_rect.w),
			HEBC_MMBUF_ADDR_ALIGN);

		if (hebcd_block_type == 0) {
			if (layer->transform & HISI_FB_TRANSFORM_ROT_90) { // read rotation
				if (is_yuv_semi_planar) {
					mmbuf_y8_line_num = 64; // Y8: 64 Y10: 32
					mmbuf_c8_line_num = 32; // C8: 32 C10: 16
				} else {
					mmbuf_y8_line_num = 16;
				}
			} else {
				if (is_yuv_semi_planar) {
					mmbuf_y8_line_num = HEBC_MMBUF_BLOCK0_Y8_LINE_NUM;
					mmbuf_c8_line_num = HEBC_MMBUF_BLOCK0_C8_LINE_NUM;
				} else {
					mmbuf_y8_line_num = HEBC_MMBUF_BLOCK0_RGB_LINE_NUM;
				}
			}
		} else if (hebcd_block_type == 1) {
			if (is_yuv_semi_planar) {
				mmbuf_y8_line_num = HEBC_MMBUF_BLOCK1_Y8_LINE_NUM;
				mmbuf_c8_line_num = HEBC_MMBUF_BLOCK1_C8_LINE_NUM;
			} else {
				mmbuf_y8_line_num = HEBC_MMBUF_BLOCK1_RGB_LINE_NUM;
			}
		} else {
			HISI_FB_ERR("hebcd_block_type=%d no support!\n", layer->img.hebcd_block_type);
			return -EINVAL;
		}

		// Y8: W*1*line_num, Y10: W*2*line_num, RGB: W*4*line_num
		if (is_yuv_semi_planar)
			hebcd_rect.right *= 1;
		else
			hebcd_rect.right *= 4;

		hisifd->mmbuf_info->mm_size0_y8[chn_idx] = hebcd_rect.right * mmbuf_y8_line_num;
		if (is_yuv_semi_planar)
			hisifd->mmbuf_info->mm_size1_c8[chn_idx] = hebcd_rect.right * mmbuf_c8_line_num;
		else
			hisifd->mmbuf_info->mm_size1_c8[chn_idx] = 0;

		hisifd->mmbuf_info->mm_size[chn_idx] = hisifd->mmbuf_info->mm_size0_y8[chn_idx] +
			hisifd->mmbuf_info->mm_size1_c8[chn_idx];

		hisifd->mmbuf_info->mm_base[chn_idx] = hisi_dss_mmbuf_alloc(g_mmbuf_gen_pool,
			hisifd->mmbuf_info->mm_size[chn_idx]);
		hisifd->mmbuf_info->mm_base0_y8[chn_idx] = hisifd->mmbuf_info->mm_base[chn_idx];
		if (hisifd->mmbuf_info->mm_base0_y8[chn_idx] < MMBUF_BASE) {
			HISI_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base0_y8=0x%x\n",
				hisifd->index, chn_idx, hisifd->mmbuf_info->mm_base0_y8[chn_idx]);
			return -EINVAL;
		}

		if (is_yuv_semi_planar) {
			hisifd->mmbuf_info->mm_base1_c8[chn_idx] = hisifd->mmbuf_info->mm_base0_y8[chn_idx] +
				hisifd->mmbuf_info->mm_size0_y8[chn_idx];
			if (hisifd->mmbuf_info->mm_base1_c8[chn_idx] < MMBUF_BASE) {
				HISI_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base1_c8=0x%x\n",
					hisifd->index, chn_idx, hisifd->mmbuf_info->mm_base1_c8[chn_idx]);
				return -EINVAL;
			}
		}
	}

	hisifd->mmbuf_info->mm_used[chn_idx] = 1;

	return 0;
}

int hisi_dss_hebcd_config(struct hisi_fb_data_type *hisifd, int ovl_idx,
	dss_layer_t *layer, dss_rect_ltrb_t *clip_rect,
	dss_rect_t *out_aligned_rect, bool *rdma_stretch_enable)
{
	dss_rdma_t *dma = NULL;
	bool mmu_enable = false;
	bool is_yuv_semi_planar = false;
	int rdma_format;
	int rdma_transform;
	uint32_t stretch_size_vrt = 0;
	uint32_t stretched_line_num;

	int bpp;
	int aligned_pixel;
	int rdma_oft_x0;
	int rdma_oft_x1;
	int chn_idx;
	int ret;
	dss_rect_ltrb_t aligned_rect = {0, 0, 0, 0};
	dss_rect_t src_rect;
	uint32_t src_bottom;
	uint32_t src_right;

	bool is_pixel_10bit = false;
	uint32_t hebcd_header_addr0 = 0;
	uint32_t hebcd_header_stride0 = 0;
	uint32_t hebcd_header_addr1 = 0;
	uint32_t hebcd_header_stride1 = 0;
	uint32_t hebcd_header_pointer_offset = 0;
	uint32_t hebcd_top_crop_num = 0;
	uint32_t hebcd_bottom_crop_num = 0;
	uint32_t hebcd_crop_num_max = 0;
	uint32_t hebcd_block_width_align = 0;
	uint32_t hebcd_block_height_align = 0;
	uint32_t hebcd_payload_addr0 = 0;
	uint32_t hebcd_payload_stride0 = 0;
	uint32_t hebcd_payload0_align = 0;
	uint32_t hebcd_payload_addr1 = 0;
	uint32_t hebcd_payload_stride1 = 0;
	uint32_t hebcd_payload1_align = 0;
	uint32_t hebcd_hreg_pic_width = 0;
	uint32_t hebcd_hreg_pic_height = 0;

	uint32_t mm_base0_y8 = 0;
	uint32_t mm_base1_c8 = 0;
	uint32_t hebcd_block_type = 0;
	uint32_t color_transform = 1;
	uint32_t hebcd_height_bf_str = 0;

	if (hisifd ==NULL || layer == NULL || clip_rect == NULL ||
		out_aligned_rect == NULL || rdma_stretch_enable == NULL)
		return -EINVAL;

	chn_idx = layer->chn_idx;
	src_rect = layer->src_rect;
	src_bottom = (uint32_t)src_rect.y + src_rect.h;
	src_right = (uint32_t)src_rect.x + src_rect.w;

	stretched_line_num = isNeedRdmaStretchBlt(hisifd, layer);
	*rdma_stretch_enable = (stretched_line_num > 0) ? true : false;

	mmu_enable = (layer->img.mmu_enable == 1) ? true : false;
	is_yuv_semi_planar = isYUVSemiPlanar(layer->img.format);

	rdma_format = hisi_pixel_format_hal2dma(layer->img.format);
	if (rdma_format < 0) {
		HISI_FB_ERR("layer format(%d) not support !\n", layer->img.format);
		return -EINVAL;
	}

	rdma_transform = hisi_transform_hal2dma(layer->transform, chn_idx);
	if (rdma_transform < 0) {
		HISI_FB_ERR("layer transform(%d) not support!\n", layer->transform);
		return -EINVAL;
	}

	if (is_yuv_semi_planar)
		color_transform = 0;

	is_pixel_10bit = isPixel10Bit2dma(rdma_format);
	bpp = is_yuv_semi_planar ? 1 : layer->img.bpp;
	if (is_pixel_10bit)
		bpp = layer->img.bpp;

	aligned_pixel = DMA_ALIGN_BYTES / bpp;

	dma = &(hisifd->dss_module.rdma[chn_idx]);
	hisifd->dss_module.dma_used[chn_idx] = 1;

	hebcd_block_type = layer->img.hebcd_block_type;

	if (layer->img.hebc_mmbuf_base0_y8 > 0) {
		mm_base0_y8 = layer->img.hebc_mmbuf_base0_y8;
	} else {
		ret = hisi_dss_hebcd_mmbuf_config(hisifd, ovl_idx, layer, hebcd_block_type,
			is_yuv_semi_planar);
		if (ret < 0) {
			HISI_FB_ERR("hisi_dss_hebcd_mmbuf_config fail!\n");
			return -EINVAL;
		}
		mm_base0_y8 = hisifd->mmbuf_info->mm_base0_y8[chn_idx];
	}

	mm_base0_y8 -= MMBUF_BASE;

	if (is_yuv_semi_planar) {
		if (layer->img.hebc_mmbuf_base1_c8 > 0)
			mm_base1_c8 = layer->img.hebc_mmbuf_base1_c8;
		else
			mm_base1_c8 = hisifd->mmbuf_info->mm_base1_c8[chn_idx];
		mm_base1_c8 -= MMBUF_BASE;
	}

	ret = hisi_dss_hebcd_config_check_mm_base0(hisifd, mm_base0_y8, mm_base1_c8,
		chn_idx, layer->layer_idx, is_yuv_semi_planar);
	if (ret == -EINVAL)
		return ret;

	if (hebcd_block_type == 0) {
		if (is_yuv_semi_planar) {
			if (rdma_format == DMA_PIXEL_FORMAT_YUV420_SP_10BIT) {
				hebcd_block_width_align = 32;
				hebcd_block_height_align = 16;
			} else {
				hebcd_block_width_align = HEBC_BLOCK0_YUV_WIDTH_ALIGN;
				hebcd_block_height_align = HEBC_BLOCK0_YUV_HEIGHT_ALIGN;
			}
		} else {
			hebcd_block_width_align = HEBC_BLOCK0_RGB_WIDTH_ALIGN;
			hebcd_block_height_align = HEBC_BLOCK0_RGB_HEIGHT_ALIGN;
		}
	} else if (hebcd_block_type == 1) {
		if (is_yuv_semi_planar) {
			hebcd_block_width_align = HEBC_BLOCK1_YUV_WIDTH_ALIGN;
			hebcd_block_height_align = HEBC_BLOCK1_YUV_HEIGHT_ALIGN;
		} else {
			hebcd_block_width_align = HEBC_BLOCK1_RGB_WIDTH_ALIGN;
			hebcd_block_height_align = HEBC_BLOCK1_RGB_HEIGHT_ALIGN;
		}
	} else {
		HISI_FB_ERR("hebcd_block_type=%d no support!\n", layer->img.hebcd_block_type);
		return -EINVAL;
	}

	//aligned rect
	aligned_rect.left = ALIGN_DOWN(src_rect.x, hebcd_block_width_align);
	aligned_rect.right = ALIGN_UP(src_right, hebcd_block_width_align) - 1;
	aligned_rect.top = ALIGN_DOWN(src_rect.y, hebcd_block_height_align);
	aligned_rect.bottom = ALIGN_UP(src_bottom, hebcd_block_height_align) - 1;

	hebcd_hreg_pic_width = aligned_rect.right - aligned_rect.left;
	hebcd_hreg_pic_height = aligned_rect.bottom - aligned_rect.top;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if ((hebcd_hreg_pic_width > HFBC_PIC_WIDTH_ROT_MAX) ||
			(hebcd_hreg_pic_height > HFBC_PIC_HEIGHT_ROT_MAX)) {
			HISI_FB_ERR("layer%d hfbcd_hreg_pic_width %d, hfbcd_hreg_pic_height %d "
				"is too large(4096*2160)!\n", layer->layer_idx, hebcd_hreg_pic_width,
				hebcd_hreg_pic_height);
			return -EINVAL;
		}
	}
	if ((layer->img.width & (hebcd_block_width_align - 1)) ||
		(layer->img.height & (hebcd_block_height_align - 1))) {
		HISI_FB_ERR("layer%d img width %d is not %d bytes aligned, or "
			"img heigh %d is not %d bytes aligned!\n",
			layer->layer_idx, layer->img.width, hebcd_block_width_align,
			layer->img.height, hebcd_block_height_align);
		return -EINVAL;
	}

	if (hebcd_block_type == 0) {
		if (is_yuv_semi_planar)
			hebcd_crop_num_max = HEBC_BLOCK0_YUV_CROP_MAX;
		else
			hebcd_crop_num_max = HEBC_BLOCK0_RGB_CROP_MAX;
	} else if (hebcd_block_type == 1) {
		if (is_yuv_semi_planar)
			hebcd_crop_num_max = HEBC_BLOCK1_YUV_CROP_MAX;
		else
			hebcd_crop_num_max = HEBC_BLOCK1_RGB_CROP_MAX;
	} else {
		HISI_FB_ERR("hebcd_block_type=%d no support!\n", layer->img.hebcd_block_type);
		return -EINVAL;
	}

	rdma_oft_x0 = aligned_rect.left / aligned_pixel;
	rdma_oft_x1 = aligned_rect.right / aligned_pixel;

	hebcd_header_stride0 = layer->img.hebc_header_stride0;
	hebcd_header_pointer_offset = (aligned_rect.top / hebcd_block_height_align) *
		hebcd_header_stride0 + (aligned_rect.left / hebcd_block_width_align) *
		HEBC_HEADER_STRIDE_BLOCK;

	hebcd_header_addr0 = layer->img.hebc_header_addr0 + hebcd_header_pointer_offset;
	if ((hebcd_header_addr0 & (HEBC_HEADER_ADDR_ALIGN - 1)) ||
		(hebcd_header_stride0 & (HEBC_HEADER_STRIDE_ALIGN - 1))) {
		HISI_FB_ERR("layer%d hebcd_header_addr0 0x%x is not %d bytes aligned or "
			"hebcd_header_stride0 0x%x is not %d bytes aligned!\n",
			layer->layer_idx, hebcd_header_addr0, HEBC_HEADER_ADDR_ALIGN,
			hebcd_header_stride0, HEBC_HEADER_STRIDE_ALIGN);
		return -EINVAL;
	}

	if (is_yuv_semi_planar) {
		hebcd_header_addr1 = layer->img.hebc_header_addr1 + hebcd_header_pointer_offset;
		hebcd_header_stride1 = hebcd_header_stride0;
		if (hebcd_header_addr1 & (HEBC_HEADER_ADDR_ALIGN - 1)) {
			HISI_FB_ERR("layer%d hebcd_header_addr1 0x%x is not %d bytes aligned!\n",
				layer->layer_idx, hebcd_header_addr1, HFBC_HEADER_ADDR_ALIGN);
			return -EINVAL;
		}
	}

	hebcd_payload0_align = HEBC_PAYLOAD_512B_ALIGN;
	hebcd_payload_stride0 = layer->img.hebc_payload_stride0;
	hebcd_payload_addr0 = layer->img.hebc_payload_addr0 +
		(aligned_rect.top / hebcd_block_height_align) * hebcd_payload_stride0 +
		(aligned_rect.left / hebcd_block_width_align) * hebcd_payload0_align;
	if ((hebcd_payload_addr0 & (hebcd_payload0_align - 1)) ||
		(hebcd_payload_stride0 & (hebcd_payload0_align - 1))) {
		HISI_FB_ERR("layer%d hebcd_payload_addr0 0x%x or hebcd_payload_stride0 0x%x"
			"is not %d bytes aligned!\n", layer->layer_idx, hebcd_payload_addr0,
			hebcd_payload_stride0, hebcd_payload0_align);
		return -EINVAL;
	}

	if (is_yuv_semi_planar) {
		hebcd_payload1_align = HEBC_PAYLOAD_512B_ALIGN;
		hebcd_payload_stride1 = layer->img.hebc_payload_stride1;
		hebcd_payload_addr1 = layer->img.hebc_payload_addr1 +
			(aligned_rect.top / hebcd_block_height_align) * hebcd_payload_stride1 +
			(aligned_rect.left / hebcd_block_width_align) * hebcd_payload1_align;
		if ((hebcd_payload_addr1 & (hebcd_payload1_align - 1)) ||
			(hebcd_payload_stride1 & (hebcd_payload1_align - 1))) {
			HISI_FB_ERR("layer%d hebcd_payload_addr1 0x%x or hebcd_payload_stride1"
				"0x%x is not %d bytes aligned!\n", layer->layer_idx,
				hebcd_payload_addr1, hebcd_payload_stride1, hebcd_payload1_align);
			return -EINVAL;
		}
	}

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		aligned_rect.left = ALIGN_DOWN((uint32_t)src_rect.y, hebcd_block_height_align);
		aligned_rect.right = ALIGN_UP(src_bottom, hebcd_block_height_align) - 1; //lint !e502
		aligned_rect.top = ALIGN_DOWN((uint32_t)src_rect.x, hebcd_block_width_align);
		aligned_rect.bottom = ALIGN_UP(src_right, hebcd_block_width_align) - 1; //lint !e502
	}

	//out_aligned_rect
	out_aligned_rect->x = 0;
	out_aligned_rect->y = 0;
	out_aligned_rect->w = aligned_rect.right - aligned_rect.left + 1;
	out_aligned_rect->h = aligned_rect.bottom - aligned_rect.top + 1;
	hebcd_height_bf_str = aligned_rect.bottom - aligned_rect.top + 1;

	// rdfc clip_rect
	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		clip_rect->left = src_rect.y - aligned_rect.left;
		clip_rect->right = aligned_rect.right - DSS_WIDTH(src_bottom);
		clip_rect->top = src_rect.x - aligned_rect.top;
		clip_rect->bottom = aligned_rect.bottom - DSS_HEIGHT(src_right);
	} else {
		clip_rect->left = src_rect.x - aligned_rect.left;
		clip_rect->right = aligned_rect.right - DSS_WIDTH(src_right);
		clip_rect->top = src_rect.y - aligned_rect.top;
		clip_rect->bottom = aligned_rect.bottom - DSS_HEIGHT(src_bottom);
	}

	if (hisi_adjust_clip_rect(layer, clip_rect) < 0) {
		HISI_FB_ERR("clip rect invalid => layer_idx=%d, chn_idx=%d, clip_rect(%d, %d, %d, %d).\n",
			layer->layer_idx, chn_idx, clip_rect->left, clip_rect->right,
			clip_rect->top, clip_rect->bottom);
		return -EINVAL;
	}

	hebcd_top_crop_num = (clip_rect->top > hebcd_crop_num_max) ? hebcd_crop_num_max : clip_rect->top;
	hebcd_bottom_crop_num = (clip_rect->bottom > hebcd_crop_num_max) ? hebcd_crop_num_max : clip_rect->bottom;

	clip_rect->top -= hebcd_top_crop_num;
	clip_rect->bottom -= hebcd_bottom_crop_num;

	//adjust out_aligned_rect
	out_aligned_rect->h -= (hebcd_top_crop_num + hebcd_bottom_crop_num);
	stretch_size_vrt = DSS_HEIGHT(out_aligned_rect->h);
	stretched_line_num = 0;

	dma->hebcd_used = 1;
	dma->oft_x0 = set_bits32(dma->oft_x0, rdma_oft_x0, 16, 0);
	dma->oft_x1 = set_bits32(dma->oft_x1, rdma_oft_x1, 16, 0);
	dma->stretch_size_vrt = set_bits32(dma->stretch_size_vrt, (stretch_size_vrt | (stretched_line_num << 13)), 19, 0);
	dma->ctrl = set_bits32(dma->ctrl, rdma_format, 5, 3);
	dma->ctrl = set_bits32(dma->ctrl, (mmu_enable ? 0x1 : 0x0), 1, 8);
	dma->ctrl = set_bits32(dma->ctrl, rdma_transform, 3, 9);
	dma->ctrl = set_bits32(dma->ctrl, (*rdma_stretch_enable ? 1 : 0), 1, 12);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 0);
	dma->ch_ctl = set_bits32(dma->ch_ctl, 0x1, 1, 2);

	dma->hebcd_hreg_hdr_ptr_l0 = set_bits32(dma->hebcd_hreg_hdr_ptr_l0, hebcd_header_addr0, 32, 0);
	dma->hebcd_hreg_pic_width = set_bits32(dma->hebcd_hreg_pic_width, hebcd_hreg_pic_width, 16, 0);
	dma->hebcd_hreg_pic_height = set_bits32(dma->hebcd_hreg_pic_height, hebcd_hreg_pic_height, 16, 0);
	dma->hebcd_hreg_format = set_bits32(dma->hebcd_hreg_format, color_transform, 1, 21);
	dma->hebcd_line_crop = set_bits32(dma->hebcd_line_crop, (hebcd_top_crop_num << 8 | hebcd_bottom_crop_num), 16, 0);
	dma->hebcd_input_header_stride0 = set_bits32(dma->hebcd_input_header_stride0, hebcd_header_stride0, 14, 0);
	dma->hebcd_payload_stride0 = set_bits32(dma->hebcd_payload_stride0, hebcd_payload_stride0, 20, 0);
	dma->hebcd_mm_base0_y8 = set_bits32(dma->hebcd_mm_base0_y8, mm_base0_y8, 32, 0);
	dma->hebcd_payload_pointer = set_bits32(dma->hebcd_payload_pointer, hebcd_payload_addr0, 32, 0);
	dma->hebcd_height_bf_str = set_bits32(dma->hebcd_height_bf_str, DSS_HEIGHT(hebcd_height_bf_str), 16, 0);
	dma->hebcd_scramble_mode = set_bits32(dma->hebcd_scramble_mode, layer->img.hebc_scramble_mode, 4, 8);
	dma->hebcd_creg_fbcd_ctrl_mode = set_bits32(dma->hebcd_creg_fbcd_ctrl_mode, 0x10, 32, 0);
	dma->hebcd_hreg_hdr_ptr_l1 = set_bits32(dma->hebcd_hreg_hdr_ptr_l1, hebcd_header_addr1, 32, 0);
	dma->hebcd_hreg_pld_ptr_l1 = set_bits32(dma->hebcd_hreg_pld_ptr_l1, hebcd_payload_addr1, 32, 0);
	dma->hebcd_header_stride1 = set_bits32(dma->hebcd_header_stride1, hebcd_header_stride1, 14, 0);
	dma->hebcd_payload_stride1 = set_bits32(dma->hebcd_payload_stride1, hebcd_payload_stride1, 20, 0);
	dma->hebcd_block_type = set_bits32(dma->hebcd_block_type, hebcd_block_type, 2, 0);
	dma->hebcd_mm_base1_c8 = set_bits32(dma->hebcd_mm_base1_c8, mm_base1_c8, 32, 0);

	if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer) {
		HISI_FB_INFO("fb%d, mm_base0_y8=0x%x, mm_base1_c8=0x%x,"
			"mm_base0_y8_size=%d,mm_base1_c8_size=%d,"
			"aligned_rect(%d,%d,%d,%d), hebcd_block_type=%d!\n",
			hisifd->index, mm_base0_y8, mm_base1_c8, layer->img.hebc_mmbuf_size0_y8,
			layer->img.hebc_mmbuf_size1_c8, aligned_rect.left, aligned_rect.top,
			aligned_rect.right, aligned_rect.bottom, hebcd_block_type);
	}

	return 0;
}

void hisi_dss_hebce_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *wdma_base, dss_wdma_t *s_wdma)
{
	if ((hisifd == NULL) || (wdma_base == NULL) || (s_wdma == NULL)) {
		HISI_FB_DEBUG("hisifd or wdma_base, s_wdma is NULL!\n");
		return;
	}

	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_PIC_BLKS, s_wdma->hebce_hreg_pic_blks, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_FORMAT, s_wdma->hebce_hreg_format, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_HDR_PTR_L0, s_wdma->hebce_hreg_hdr_ptr_l0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_PLD_PTR_L0, s_wdma->hebce_hreg_pld_ptr_l0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_PICTURE_SIZE, s_wdma->hebce_picture_size, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HEADER_SRTIDE, s_wdma->hebce_header_stride0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_PAYLOAD_STRIDE, s_wdma->hebce_payload_stride0, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_SCRAMBLE_MODE, s_wdma->hebce_scramble_mode, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_CREG_FBCE_CTRL_MODE, s_wdma->fbce_creg_fbce_ctrl_mode, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_HDR_PTR_L1, s_wdma->hebce_hreg_hdr_ptr_l1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HREG_PLD_PTR_L1, s_wdma->hebce_hreg_pld_ptr_l1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_HEADER_SRTIDE_1, s_wdma->hebce_header_stride1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + AFBCE_PAYLOAD_SRTIDE_1, s_wdma->hebce_payload_stride1, 32, 0);
	hisifd->set_reg(hisifd, wdma_base + HEBCE_SPBLOCK_MODE, s_wdma->hebce_spblock_mode, 32, 0);
}

int hisi_dss_hebce_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req,
	dss_wb_layer_t *layer, dss_rect_t aligned_rect, dss_rect_t *ov_block_rect, bool last_block)
{
	dss_wdma_t *wdma = NULL;
	int chn_idx = 0;
	int wdma_format = 0;
	int wdma_transform = 0;

	dss_rect_t in_rect;
	bool mmu_enable = false;
	bool is_yuv_format = false;

	uint32_t color_transform = 1;
	dss_rect_ltrb_t hebce_header_rect = {0};
	dss_rect_ltrb_t hebce_payload_rect = {0};

	uint32_t hebce_header_addr0 = 0;
	uint32_t hebce_header_stride0 = 0;
	uint32_t hebce_header_pointer_offset = 0;
	uint32_t hebce_block_width_align = 0;
	uint32_t hebce_block_height_align = 0;
	uint32_t hebce_payload_addr0 = 0;
	uint32_t hebce_payload_stride0 = 0;
	uint32_t hebce_header_addr1 = 0;
	uint32_t hebce_header_stride1 = 0;
	uint32_t hebce_payload_addr1 = 0;
	uint32_t hebce_payload_stride1 = 0;
	uint32_t hebce_payload0_align = 0;
	uint32_t hebce_payload1_align = 0;

	if (hisifd == NULL || pov_req == NULL || layer == NULL || ov_block_rect == NULL)
		return -EINVAL;

	chn_idx = layer->chn_idx;

	wdma = &(hisifd->dss_module.wdma[chn_idx]);
	hisifd->dss_module.dma_used[chn_idx] = 1;

	wdma_format = hisi_pixel_format_hal2dma(layer->dst.format);
	if (wdma_format < 0) {
		HISI_FB_ERR("hisi_pixel_format_hal2dma failed!\n");
		return -EINVAL;
	}

	if ((wdma_format == DMA_PIXEL_FORMAT_YUV_420_SP_HP)
		|| (wdma_format == DMA_PIXEL_FORMAT_YUV420_SP_10BIT)) {
		color_transform = 0;
		is_yuv_format = true;
	}

	in_rect = aligned_rect;

	wdma_transform = hisi_transform_hal2dma(layer->transform, chn_idx);
	if (wdma_transform < 0) {
		HISI_FB_ERR("hisi_transform_hal2dma failed!\n");
		return -EINVAL;
	}

	mmu_enable = (layer->dst.mmu_enable == 1) ? true : false;

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if (is_yuv_format) {
			if (wdma_format == DMA_PIXEL_FORMAT_YUV420_SP_10BIT) {
				hebce_block_width_align = HEBC_BLOCK1_YUV_WIDTH_ALIGN;
				hebce_block_height_align = HEBC_BLOCK1_YUV_HEIGHT_ALIGN;
			} else {
				hebce_block_width_align = HEBC_BLOCK1_YUV_WIDTH_ALIGN;
				hebce_block_height_align = HEBC_BLOCK1_YUV_HEIGHT_ALIGN;
			}
		} else {
			hebce_block_width_align = HEBC_BLOCK1_RGB_WIDTH_ALIGN;
			hebce_block_height_align = HEBC_BLOCK1_RGB_HEIGHT_ALIGN;
		}
	} else {
		if (is_yuv_format) {
			if (wdma_format == DMA_PIXEL_FORMAT_YUV420_SP_10BIT) {
				hebce_block_width_align = 32;
				hebce_block_height_align = 16;
			} else {
				hebce_block_width_align = HEBC_BLOCK0_YUV_WIDTH_ALIGN;
				hebce_block_height_align = HEBC_BLOCK0_YUV_HEIGHT_ALIGN;
			}
		} else {
			hebce_block_width_align = HEBC_BLOCK0_RGB_WIDTH_ALIGN;
			hebce_block_height_align = HEBC_BLOCK0_RGB_HEIGHT_ALIGN;
		}
	}

	if ((layer->dst.width & (hebce_block_width_align - 1)) ||
		(layer->dst.height & (hebce_block_height_align - 1))) {
		HISI_FB_ERR("wb_layer dst width %d is not %d bytes aligned, or img heigh%d is not %d bytes aligned!\n",
			layer->dst.width, hebce_block_width_align, layer->dst.height, hebce_block_height_align);
		return -EINVAL;
	}

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		if (is_yuv_format) {
			if ((in_rect.w < HEBC_YUV_WIDTH_ROT_MIN) || (in_rect.w > HEBC_YUV_WIDTH_ROT_MAX) ||
				(in_rect.h < HEBC_YUV_HEIGHT_ROT_MIN) || (in_rect.h > HEBC_YUV_HEIGHT_ROT_MAX) ||
				(in_rect.w & (HEBC_BLOCK1_YUV_HEIGHT_ALIGN - 1)) || (in_rect.h & (HEBC_BLOCK1_YUV_WIDTH_ALIGN - 1))) {
				HISI_FB_ERR("hebce in_rect %d,%d, %d,%d is out of range!",
					in_rect.x, in_rect.y, in_rect.w, in_rect.h);
				return -EINVAL;
			}
		} else {
			if ((in_rect.w < HEBC_RGB_WIDTH_ROT_MIN) || (in_rect.w > HEBC_RGB_WIDTH_ROT_MAX) ||
				(in_rect.h < HEBC_RGB_HEIGHT_ROT_MIN) || (in_rect.h > HEBC_RGB_HEIGHT_ROT_MAX) ||
				(in_rect.w & (HEBC_BLOCK1_RGB_HEIGHT_ALIGN - 1)) || (in_rect.h & (HEBC_BLOCK1_RGB_WIDTH_ALIGN - 1))) {
				HISI_FB_ERR("hebce in_rect %d,%d, %d,%d is out of range!",
					in_rect.x, in_rect.y, in_rect.w, in_rect.h);
				return -EINVAL;
			}
		}
	} else {
		if (is_yuv_format) {
			if ((in_rect.w < HEBC_YUV_WIDTH_MIN) || (in_rect.w > HEBC_YUV_WIDTH_MAX) ||
				(in_rect.h < HEBC_YUV_HEIGHT_MIN) || (in_rect.h > HEBC_YUV_HEIGHT_MAX) ||
				(in_rect.w & (HEBC_BLOCK0_YUV_WIDTH_ALIGN - 1)) || (in_rect.h & (HEBC_BLOCK0_YUV_HEIGHT_ALIGN - 1))) {
				HISI_FB_ERR("hebce in_rect %d,%d, %d,%d is out of range!",
					in_rect.x, in_rect.y, in_rect.w, in_rect.h);
				return -EINVAL;
			}
		} else {
			if ((in_rect.w < HEBC_RGB_WIDTH_MIN) || (in_rect.w > HEBC_RGB_WIDTH_MAX) ||
				(in_rect.h < HEBC_RGB_HEIGHT_MIN) || (in_rect.h > HEBC_RGB_HEIGHT_MAX) ||
				(in_rect.w & (HEBC_BLOCK0_RGB_WIDTH_ALIGN - 1)) || (in_rect.h & (HEBC_BLOCK0_RGB_HEIGHT_ALIGN - 1))) {
				HISI_FB_ERR("hebce in_rect %d,%d, %d,%d is out of range!",
					in_rect.x, in_rect.y, in_rect.w, in_rect.h);
				return -EINVAL;
			}
		}
	}

	if (layer->transform & HISI_FB_TRANSFORM_ROT_90) {
		hebce_header_rect.left = ALIGN_DOWN(layer->dst_rect.x, hebce_block_width_align);
		hebce_header_rect.top = ALIGN_DOWN(layer->dst_rect.y + (ov_block_rect->x - layer->dst_rect.x),
			hebce_block_height_align);

		hebce_payload_rect.left = ALIGN_DOWN(layer->dst_rect.x, hebce_block_width_align);
		hebce_payload_rect.top = hebce_header_rect.top;
	} else {
		hebce_header_rect.left = ALIGN_DOWN(in_rect.x, hebce_block_width_align);
		hebce_header_rect.top = ALIGN_DOWN(in_rect.y, hebce_block_height_align);

		hebce_payload_rect.left = ALIGN_DOWN(in_rect.x, hebce_block_width_align);
		hebce_payload_rect.top = hebce_header_rect.top;
	}

	// hebc payload
	if (is_yuv_format) {
		hebce_payload0_align = HEBC_PAYLOAD_512B_ALIGN;
		hebce_payload1_align = HEBC_PAYLOAD_512B_ALIGN;
	} else {
		if (layer->dst.bpp == 4)
			hebce_payload0_align = HEBC_PAYLOAD_512B_ALIGN;
		else if (layer->dst.bpp == 2)
			hebce_payload0_align = HEBC_PAYLOAD_256B_ALIGN;
	}

	//hebc header
	hebce_header_stride0 = layer->dst.hebc_header_stride0;
	if (is_yuv_format) {
		hebce_header_pointer_offset = (hebce_header_rect.top / hebce_block_height_align) * hebce_header_stride0 +
			(hebce_header_rect.left / hebce_block_width_align) * HEBC_HEADER_STRIDE_BLOCK;
		hebce_header_addr1 = layer->dst.hebc_header_addr1 + hebce_header_pointer_offset;
		hebce_header_stride1 = hebce_header_stride0;
		if (hebce_header_addr1 & (HEBC_HEADER_ADDR_ALIGN - 1)) {
			HISI_FB_ERR("wb_layer hebcd_header_addr1 0x%x is not %d bytes aligned!\n",
				hebce_header_addr1, HEBC_HEADER_ADDR_ALIGN);
			return -EINVAL;
		}

		hebce_payload_stride1 = layer->dst.hebc_payload_stride1;
		hebce_payload_addr1 = layer->dst.hebc_payload_addr1 +
			(hebce_payload_rect.top / hebce_block_height_align) * hebce_payload_stride1 +
			(hebce_payload_rect.left / hebce_block_width_align) * hebce_payload1_align;

		if ((hebce_payload_addr1 & (hebce_payload1_align - 1)) || (hebce_payload_stride1 & (hebce_payload1_align - 1))) {
			HISI_FB_ERR("ch%d hebce_payload_addr1 0x%x or hebce_payload_stride1 0x%x is not %d bytes aligned!\n",
				layer->chn_idx, hebce_payload_addr1, hebce_payload_stride1, hebce_payload1_align);
			return -EINVAL;
		}

		hebce_block_height_align /= 2;
	}

	hebce_header_pointer_offset = (hebce_header_rect.top / hebce_block_height_align) * hebce_header_stride0 +
		(hebce_header_rect.left / hebce_block_width_align) * HEBC_HEADER_STRIDE_BLOCK;
	hebce_header_addr0 = layer->dst.hebc_header_addr0 + hebce_header_pointer_offset;
	if ((hebce_header_addr0 & (HEBC_HEADER_ADDR_ALIGN - 1)) || (hebce_header_stride0 & (HEBC_HEADER_STRIDE_ALIGN - 1))) {
		HISI_FB_ERR("wb_layer hebce_header_addr0 0x%x is not %d bytes aligned, or "
			"hebce_header_stride0 0x%x is not %d bytes aligned!\n",
			hebce_header_addr0, HEBC_HEADER_ADDR_ALIGN, hebce_header_stride0, HEBC_HEADER_STRIDE_ALIGN);
		return -EINVAL;
	}

	hebce_payload_stride0 = layer->dst.hebc_payload_stride0;
	hebce_payload_addr0 = layer->dst.hebc_payload_addr0 +
		(hebce_payload_rect.top / hebce_block_height_align) * hebce_payload_stride0 +
		(hebce_payload_rect.left / hebce_block_width_align) * hebce_payload0_align;

	if ((hebce_payload_addr0 & (hebce_payload0_align - 1)) || (hebce_payload_stride0 & (hebce_payload0_align - 1))) {
		HISI_FB_ERR("ch%d hebce_payload_addr0 0x%x or hebce_payload0_align 0x%x is not %d bytes aligned!\n",
			layer->chn_idx, hebce_payload_addr0, hebce_payload_stride0, hebce_payload0_align);
		return -EINVAL;
	}

	wdma->hebce_used = 1;
	wdma->ctrl = set_bits32(wdma->ctrl, wdma_format, 5, 3);
	wdma->ctrl = set_bits32(wdma->ctrl, (mmu_enable ? 0x1 : 0x0), 1, 8);
	wdma->ctrl = set_bits32(wdma->ctrl, wdma_transform, 3, 9);
	if (last_block)
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0x1d, 5, 0);
	else
		wdma->ch_ctl = set_bits32(wdma->ch_ctl, 0xd, 5, 0);

	wdma->rot_size = set_bits32(wdma->rot_size,
		(DSS_WIDTH(in_rect.w) | (DSS_HEIGHT(in_rect.h) << 16)), 32, 0);

	wdma->hebce_hreg_pic_blks = set_bits32(wdma->hebce_hreg_pic_blks, 0, 24, 0);
	//color transform
	wdma->hebce_hreg_format = set_bits32(wdma->hebce_hreg_format, color_transform, 1, 21);
	wdma->hebce_hreg_hdr_ptr_l0 = set_bits32(wdma->hebce_hreg_hdr_ptr_l0, hebce_header_addr0, 32, 0);
	wdma->hebce_hreg_pld_ptr_l0 = set_bits32(wdma->hebce_hreg_pld_ptr_l0, hebce_payload_addr0, 32, 0);
	wdma->hebce_picture_size = set_bits32(wdma->hebce_picture_size,
		((DSS_WIDTH(in_rect.w) << 16) | DSS_HEIGHT(in_rect.h)), 32, 0);
	wdma->hebce_header_stride0 = set_bits32(wdma->hebce_header_stride0, hebce_header_stride0, 14, 0);
	wdma->hebce_payload_stride0 = set_bits32(wdma->hebce_payload_stride0, hebce_payload_stride0, 20, 0);
	wdma->hebce_scramble_mode = set_bits32(wdma->hebce_scramble_mode, layer->dst.hebc_scramble_mode, 4, 6);
	//HEBCE enable
	wdma->fbce_creg_fbce_ctrl_mode = set_bits32(wdma->fbce_creg_fbce_ctrl_mode, 0x20, 32, 0);
	wdma->hebce_hreg_hdr_ptr_l1 = set_bits32(wdma->hebce_hreg_hdr_ptr_l1, hebce_header_addr1, 32, 0);
	wdma->hebce_hreg_pld_ptr_l1 = set_bits32(wdma->hebce_hreg_pld_ptr_l1, hebce_payload_addr1, 32, 0);
	wdma->hebce_header_stride1 = set_bits32(wdma->hebce_header_stride1, hebce_header_stride1, 14, 0);
	wdma->hebce_payload_stride1 = set_bits32(wdma->hebce_payload_stride1, hebce_payload_stride1, 20, 0);
	wdma->hebce_spblock_mode = set_bits32(wdma->hebce_spblock_mode, 0x1, 32, 0);

	if (g_debug_ovl_offline_composer) {
		HISI_FB_INFO("aligned_rect %d,%d,%d,%d, hebce_payload_rect %d,%d,%d,%d, hebce_header_pointer_offset=0x%x!\n",
			in_rect.x, in_rect.y, DSS_WIDTH(in_rect.x + in_rect.w), DSS_WIDTH(in_rect.y + in_rect.h),
			hebce_payload_rect.left, hebce_payload_rect.top, hebce_payload_rect.right, hebce_payload_rect.bottom,
			hebce_header_pointer_offset);
	}

	return 0;
}
/*lint -restore*/

void hisi_dss_mctl_sys_init(const char __iomem *mctl_sys_base, dss_mctl_sys_t *s_mctl_sys)
{
	int i;

	if (NULL == mctl_sys_base || NULL == s_mctl_sys) {
		HISI_FB_ERR("NULL ptr\n");
		return;
	}

	memset(s_mctl_sys, 0, sizeof(dss_mctl_sys_t));

	for (i= 0; i < DSS_OVL_IDX_MAX; i++) {
		s_mctl_sys->chn_ov_sel[i] = inp32(mctl_sys_base + MCTL_RCH_OV0_SEL + i * 0x4);
		if (i != DSS_OVL3) {
			s_mctl_sys->chn_ov_sel1[i] = inp32(mctl_sys_base + MCTL_RCH_OV0_SEL1 + i * 0x4);
		}
	}

	for (i= 0; i < DSS_WCH_MAX; i++) {
		s_mctl_sys->wchn_ov_sel[i] = inp32(mctl_sys_base + MCTL_WCH_OV2_SEL + i * 0x4);
	}
}

void hisi_dss_dpp_acm_gm_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *xcc_base = NULL;
	//char __iomem *acm_base = NULL;
	char __iomem *gamma_base = NULL;
	char __iomem *gamma_lut_base = NULL;
	//char __iomem *acm_lut_base = NULL;
	static uint8_t last_gamma_type = 0;
	static uint32_t gamma_config_flag = 0;
	uint32_t index = 0;
	uint32_t i;
	uint32_t gama_lut_sel;
	//uint32_t acm_lut_sel;

	uint32_t *local_gamma_lut_table_R = NULL;
	uint32_t *local_gamma_lut_table_G = NULL;
	uint32_t *local_gamma_lut_table_B = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd, NUll pointer warning.\n");
		goto func_exit;
	}

	pinfo = &(hisifd->panel_info);

	if (0 == pinfo->gamma_support ) {
		goto func_exit;
	}

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_GAMA)) {//lint !e845 !e774
		HISI_FB_DEBUG("gamma is not suppportted in this platform.\n");
		goto func_exit;
	}

	if (PRIMARY_PANEL_IDX == hisifd->index) {
		gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET;
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
		//acm_base = hisifd->dss_base + DSS_DPP_ACM_OFFSET;
		gamma_base = hisifd->dss_base + DSS_DPP_GAMA_OFFSET;
		gamma_lut_base = hisifd->dss_base + DSS_DPP_GAMA_LUT_OFFSET;
		//acm_lut_base = hisifd->dss_base + DSS_DPP_ACM_LUT_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!\n", hisifd->index);
		goto func_exit;
	}

	if (0 == gamma_config_flag) {
		if (last_gamma_type != hisifd->panel_info.gamma_type) {
			//disable acm
			//set_reg(acm_base + ACM_EN, 0x0, 1, 0);
			//disable gamma
			set_reg(gamma_base + GAMA_EN, 0x0, 1, 0);
			//disable gmp
			set_reg(gmp_base + GMP_EN, 0x0, 1, 0);
			//disable xcc
			set_reg(xcc_base + XCC_EN, 0x1, 1, 0);
			gamma_config_flag = 1;
			last_gamma_type = hisifd->panel_info.gamma_type;
		}
		goto func_exit;
	}

	if (1 == gamma_config_flag) {

		if (1 == last_gamma_type) {
			//set gamma cinema parameter
			if (pinfo->cinema_gamma_lut_table_len > 0 && pinfo->cinema_gamma_lut_table_R
				&& pinfo->cinema_gamma_lut_table_G && pinfo->cinema_gamma_lut_table_B) {
					local_gamma_lut_table_R = pinfo->cinema_gamma_lut_table_R;
					local_gamma_lut_table_G = pinfo->cinema_gamma_lut_table_G;
					local_gamma_lut_table_B = pinfo->cinema_gamma_lut_table_B;
			} else {
				HISI_FB_ERR("can't get gamma cinema paramter from pinfo.\n");
				goto func_exit;
			}
		} else {
			if (pinfo->gamma_lut_table_len > 0 && pinfo->gamma_lut_table_R
				&& pinfo->gamma_lut_table_G && pinfo->gamma_lut_table_B) {
				local_gamma_lut_table_R = pinfo->gamma_lut_table_R;
				local_gamma_lut_table_G = pinfo->gamma_lut_table_G;
				local_gamma_lut_table_B = pinfo->gamma_lut_table_B;
			} else {
				HISI_FB_ERR("can't get gamma normal parameter from pinfo.\n");
				goto func_exit;
			}
		}

		//config regsiter use default or cinema parameter
		for (index = 0; index < pinfo->gamma_lut_table_len / 2; index++) {
			i = index << 1;
			outp32(gamma_lut_base + (U_GAMA_R_COEF + index * 4), (local_gamma_lut_table_R[i] | (local_gamma_lut_table_R[i+1] << 16)));
			outp32(gamma_lut_base + (U_GAMA_G_COEF + index * 4), (local_gamma_lut_table_G[i] | (local_gamma_lut_table_G[i+1] << 16)));
			outp32(gamma_lut_base + (U_GAMA_B_COEF + index * 4), (local_gamma_lut_table_B[i] | (local_gamma_lut_table_B[i+1] << 16)));
			//GAMA  PRE LUT
			outp32(gamma_lut_base + (U_GAMA_PRE_R_COEF + index * 4), (local_gamma_lut_table_R[i] | (local_gamma_lut_table_R[i+1] << 16)));
			outp32(gamma_lut_base + (U_GAMA_PRE_G_COEF + index * 4), (local_gamma_lut_table_G[i] | (local_gamma_lut_table_G[i+1] << 16)));
			outp32(gamma_lut_base + (U_GAMA_PRE_B_COEF + index * 4), (local_gamma_lut_table_B[i] | (local_gamma_lut_table_B[i+1] << 16)));
        }
		outp32(gamma_lut_base + U_GAMA_R_LAST_COEF, local_gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
		outp32(gamma_lut_base + U_GAMA_G_LAST_COEF, local_gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
		outp32(gamma_lut_base + U_GAMA_B_LAST_COEF, local_gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);
		//GAMA  PRE LUT
		outp32(gamma_lut_base + U_GAMA_PRE_R_LAST_COEF, local_gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
		outp32(gamma_lut_base + U_GAMA_PRE_G_LAST_COEF, local_gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
		outp32(gamma_lut_base + U_GAMA_PRE_B_LAST_COEF, local_gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);

		gama_lut_sel = (uint32_t)inp32(gamma_base + GAMA_LUT_SEL);
		set_reg(gamma_base + GAMA_LUT_SEL, (~(gama_lut_sel & 0x1)) & 0x1, 1, 0);

	}

	//enable gamma
	set_reg(gamma_base + GAMA_EN, 0x1, 1, 0);
	//enable gamma pre
	//set_reg(gamma_base + GAMA_EN, 0x1, 1, 1);

	//enable gmp
	set_reg(gmp_base + GMP_EN, 0x1, 1, 0);
	//enable xcc
	set_reg(xcc_base + XCC_EN, 0x1, 1, 0);
	//enable xcc pre
	//set_reg(xcc_base + XCC_EN, 0x1, 1, 1);

	//enable acm
	//set_reg(acm_base + ACM_EN, 0x1, 1, 0);
	gamma_config_flag = 0;
func_exit:
	return;//lint !e438
}//lint !e550
void hisi_dss_post_scf_init(const char __iomem *dss_base, const char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	if (NULL == post_scf_base) {
		HISI_FB_ERR("post_scf_base is NULL");
		return;
	}
	if (NULL == s_post_scf) {
		HISI_FB_ERR("s_post_scf is NULL");
		return;
	}
	if (NULL == dss_base) {
		HISI_FB_ERR("dss_base is NULL");
		return;
	}

	memset(s_post_scf, 0, sizeof(dss_arsr1p_t));

	s_post_scf->dpp_img_size_bef_sr = (uint32_t)inp32(dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR);
	s_post_scf->dpp_img_size_aft_sr = (uint32_t)inp32(dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR);

	s_post_scf->ihleft = (uint32_t)inp32(post_scf_base + ARSR_POST_IHLEFT);
	s_post_scf->ihleft1 = (uint32_t)inp32(post_scf_base + ARSR_POST_IHLEFT1);
	s_post_scf->ihright = (uint32_t)inp32(post_scf_base + ARSR_POST_IHRIGHT);
	s_post_scf->ihright1 = (uint32_t)inp32(post_scf_base + ARSR_POST_IHRIGHT1);
	s_post_scf->ivtop = (uint32_t)inp32(post_scf_base + ARSR_POST_IVTOP);
	s_post_scf->ivbottom = (uint32_t)inp32(post_scf_base + ARSR_POST_IVBOTTOM);
	s_post_scf->ivbottom1 = (uint32_t)inp32(post_scf_base + ARSR_POST_IVBOTTOM1);
	s_post_scf->ihinc = (uint32_t)inp32(post_scf_base + ARSR_POST_IHINC);
	s_post_scf->ivinc = (uint32_t)inp32(post_scf_base + ARSR_POST_IVINC);
	s_post_scf->uv_offset = (uint32_t)inp32(post_scf_base + ARSR_POST_UV_OFFSET);
	s_post_scf->mode = (uint32_t)inp32(post_scf_base + ARSR_POST_MODE);


	s_post_scf->skin_thres_y = (600 << 20) | (332 << 10) | 300; // 0x2585312C
	s_post_scf->skin_thres_u = (452 << 20) | (40 << 10) | 20; // 0x1C40A014
	s_post_scf->skin_thres_v = (580 << 20) | (48 << 10) | 24; // 0x2440C018
	s_post_scf->skin_cfg0 = (12 << 13) | 512; // 0x00018200
	s_post_scf->skin_cfg1 = 819; // 0x00000333
	s_post_scf->skin_cfg2 = 682; // 0x000002AA
	s_post_scf->shoot_cfg1 = (20 << 16) | 341; // 0x00140155
	s_post_scf->shoot_cfg2 = (-80 & 0x7ff) | (16 << 16); // 0x001007B0
	s_post_scf->shoot_cfg3 = 20; // 0x00000014
	s_post_scf->sharp_cfg3 = (0xA0 << 16) | 0x60; // 0x00A00060
	s_post_scf->sharp_cfg4 = (0x60 << 16) | 0x20; // 0x00600020
	s_post_scf->sharp_cfg5 = 0;
	s_post_scf->sharp_cfg6 = (0x4 << 16) | 0x8; // 0x00040008
	s_post_scf->sharp_cfg7 = (6 << 8) | 10; // 0x0000060A
	s_post_scf->sharp_cfg8 = (0xA0 << 16) | 0x10; // 0x00A00010

	s_post_scf->sharp_level = 0x0020002;
	s_post_scf->sharp_gain_low = 0x3C0078;
	s_post_scf->sharp_gain_mid = 0x6400C8;
	s_post_scf->sharp_gain_high = 0x5000A0;
	s_post_scf->sharp_gainctrl_sloph_mf = 0x280;
	s_post_scf->sharp_gainctrl_slopl_mf = 0x1400;
	s_post_scf->sharp_gainctrl_sloph_hf = 0x140;
	s_post_scf->sharp_gainctrl_slopl_hf = 0xA00;
	s_post_scf->sharp_mf_lmt = 0x40;
	s_post_scf->sharp_gain_mf = 0x12C012C;
	s_post_scf->sharp_mf_b = 0;
	s_post_scf->sharp_hf_lmt = 0x80;
	s_post_scf->sharp_gain_hf = 0x104012C;
	s_post_scf->sharp_hf_b = 0x1400;
	s_post_scf->sharp_lf_ctrl = 0x100010;
	s_post_scf->sharp_lf_var = 0x1800080;
	s_post_scf->sharp_lf_ctrl_slop = 0;
	s_post_scf->sharp_hf_select = 0;
	s_post_scf->sharp_cfg2_h = 0x10000C0;
	s_post_scf->sharp_cfg2_l = 0x200010;
	s_post_scf->texture_analysis = 0x500040;
	s_post_scf->intplshootctrl = 0x8;
}

int hisi_dss_post_scf_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_rect_t src_rect = {0};
	dss_rect_t dst_rect = {0};
	dss_arsr1p_t *post_scf = NULL;

	int32_t ihinc = 0;
	int32_t ivinc = 0;
	int32_t ihleft = 0;
	int32_t ihright = 0;
	int32_t ihleft1 = 0;
	int32_t ihright1 = 0;
	int32_t ivtop = 0;
	int32_t ivbottom = 0;
	int32_t extraw = 0;
	int32_t extraw_left = 0;
	int32_t extraw_right = 0;
	/*lint -e713 -e730 -e838*/
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF)) {
		return 0;
	}

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		return 0;
	}

	if (pov_req != NULL) {
		if ((pov_req->res_updt_rect.w <= 0) || (pov_req->res_updt_rect.h <= 0)) {
			HISI_FB_DEBUG("fb%d, res_updt_rect[%d,%d, %d,%d] is invalid!\n", hisifd->index,
				pov_req->res_updt_rect.x, pov_req->res_updt_rect.y,
				pov_req->res_updt_rect.w, pov_req->res_updt_rect.h);
			return  0;
		}

		if ((pov_req->res_updt_rect.w == hisifd->ov_req_prev.res_updt_rect.w)
			&& (pov_req->res_updt_rect.h == hisifd->ov_req_prev.res_updt_rect.h)) {
			return 0;
		}
		src_rect = pov_req->res_updt_rect;

		HISI_FB_DEBUG("fb%d, post scf res_updt_rect[%d, %d]->lcd_rect[%d, %d]\n",
			hisifd->index,
			pov_req->res_updt_rect.w, pov_req->res_updt_rect.h,
			pinfo->xres, pinfo->yres);
	} else {
		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.w = pinfo->xres;
		src_rect.h = pinfo->yres;
	}

	if (pinfo->cascadeic_support && pov_req &&
		(pov_req->rog_width > 0 && pov_req->rog_height > 0)) {
		// for foldable display ROG partial update
		dst_rect.x = pov_req->res_updt_rect.x;
		dst_rect.y = pov_req->res_updt_rect.y;
		dst_rect.w = pov_req->res_updt_rect.w;
		dst_rect.h = pov_req->res_updt_rect.h;
	} else {
		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.w = pinfo->xres;
		dst_rect.h = pinfo->yres;
	}

	post_scf = &(hisifd->dss_module.post_scf);
	hisifd->dss_module.post_scf_used = 1;

	post_scf->dpp_img_size_bef_sr = set_bits32(post_scf->dpp_img_size_bef_sr,
		(DSS_HEIGHT((uint32_t)src_rect.h) << 16) | DSS_WIDTH((uint32_t)src_rect.w), 32, 0);
	post_scf->dpp_img_size_aft_sr = set_bits32(post_scf->dpp_img_size_aft_sr,
		(DSS_HEIGHT((uint32_t)dst_rect.h) << 16) | DSS_WIDTH((uint32_t)dst_rect.w), 32, 0);
	post_scf->dpp_used = 1;

	if ((src_rect.w < 16) || (src_rect.h < 16)
		|| (src_rect.w > 3840) || (src_rect.h > 8192)
		|| (dst_rect.w > 8192) || (dst_rect.h > 8192)) {
		HISI_FB_ERR("invalid input size: src_rect(%d,%d,%d,%d) should be larger than 16*16, less than 3840*8192!\n"
			"invalid output size: dst_rect(%d,%d,%d,%d) should be less than 8192*8192!\n",
			src_rect.x, src_rect.y, src_rect.w, src_rect.h,
			dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
		//ARSR1P bypass
		post_scf->mode = 0x1;
		return 0;
	}

	ihinc = ARSR1P_INC_FACTOR * src_rect.w / dst_rect.w;
	ivinc = ARSR1P_INC_FACTOR * src_rect.h / dst_rect.h;

	if ((ihinc == ARSR1P_INC_FACTOR)
		&& (ivinc == ARSR1P_INC_FACTOR)
		&& (pinfo->arsr1p_sharpness_support != 1)) {
		//ARSR1P bypass
		post_scf->mode = 0x1;
		return 0;
	}

	/* 0x2000<=ihinc<=0x80000; 0x2000<=ivinc<=0x80000; */
	if ((ihinc < 0x2000) || (ihinc > ARSR1P_INC_FACTOR)
		|| (ivinc < 0x2000) || (ivinc > ARSR1P_INC_FACTOR)) {
		HISI_FB_ERR("invalid ihinc(0x%x), ivinc(0x%x)!\n", ihinc, ivinc);
		//ARSR1P bypass
		post_scf->mode = 0x1;
		return -1;
	}

	if ((ihinc > ARSR1P_INC_FACTOR) ||  (ivinc > ARSR1P_INC_FACTOR)) {
		//scaler down, not supported
		HISI_FB_ERR("scaling down is not supported by ARSR1P, ihinc = 0x%x, ivinc = 0x%x\n", ihinc, ivinc);
		//ARSR1P bypass
		post_scf->mode = 0x1;
		return -1;
	}

	//enable arsr1p
	post_scf->mode = 0x0;

	if (pinfo->arsr1p_sharpness_support) {
		//enable sharp  skinctrl, shootdetect
		post_scf->mode |= 0xe;
	}

	//enable direction
	post_scf->mode |= 0x10;

	if ((ihinc < ARSR1P_INC_FACTOR) ||  (ivinc < ARSR1P_INC_FACTOR)) {
		//enable diintplen
		post_scf->mode |= 0x20;
	} else {
		//only sharp, enable nointplen
		post_scf->mode |= 0x100;
	}

	extraw = (8 * ARSR1P_INC_FACTOR) / ihinc;
	extraw_left = (extraw % 2) ? (extraw + 1) : (extraw);
	extraw = (2 * ARSR1P_INC_FACTOR) / ihinc;
	extraw_right = (extraw % 2) ? (extraw + 1) : (extraw);

	//ihleft1 = (startX_o * ihinc) - (ov_startX0 << 16)
	ihleft1 = dst_rect.x * ihinc - src_rect.x * ARSR1P_INC_FACTOR;
	if (ihleft1 < 0) {
		ihleft1 = 0;
	}
	//ihleft = ihleft1 - even(8 * 65536 / ihinc) * ihinc;
	ihleft = ihleft1 - extraw_left * ihinc;
	if (ihleft < 0) {
		ihleft = 0;
	}
	//ihright1 = ihleft1 + (oww-1) * ihinc
	ihright1 = ihleft1 + (dst_rect.w - 1) * ihinc;
	//ihright = ihright1 + even(2 * 65536/ihinc) * ihinc
	ihright = ihright1 + extraw_right * ihinc;
	//ihright >= img_width * ihinc
	if (ihright >= src_rect.w * ARSR1P_INC_FACTOR) {
		ihright = src_rect.w * ARSR1P_INC_FACTOR - 1;
	}
	//ivtop = (startY_o * ivinc) - (ov_startY0<<16)
	ivtop = dst_rect.y * ivinc - src_rect.y * ARSR1P_INC_FACTOR;
	if (ivtop < 0) {
		ivtop = 0;
	}
	//ivbottom = ivtop + (ohh - 1) * ivinc
	ivbottom = ivtop + (dst_rect.h - 1) * ivinc;
	/*lint +e713 +e730 +e838*/
	//ivbottom >= img_height * ivinc
	if (ivbottom >= src_rect.h * ARSR1P_INC_FACTOR) {
		ivbottom = src_rect.h * ARSR1P_INC_FACTOR - 1;
	}
	//(ihleft1 - ihleft) % (ihinc) == 0;
	if ((ihleft1 - ihleft) % (ihinc)) {
		HISI_FB_ERR("(ihleft1(%d)-ihleft(%d))  ihinc(%d) != 0, invalid!\n",
			ihleft1, ihleft, ihinc);
		post_scf->mode = 0x1;
		return -1;
	}

	//(ihright1 - ihleft1) % ihinc == 0;
	if ((ihright1 - ihleft1) % ihinc) {
		HISI_FB_ERR("(ihright1(%d)-ihleft1(%d))  ihinc(%d) != 0, invalid!\n",
			ihright1, ihleft1, ihinc);
		post_scf->mode = 0x1;
		return -1;
	}

	post_scf->ihleft = set_bits32(post_scf->ihleft, ihleft, 32, 0);
	post_scf->ihright = set_bits32(post_scf->ihright, ihright, 32, 0);
	post_scf->ihleft1 = set_bits32(post_scf->ihleft1, ihleft1, 32, 0);
	post_scf->ihright1 = set_bits32(post_scf->ihright1, ihright1, 32, 0);
	post_scf->ivtop = set_bits32(post_scf->ivtop, ivtop, 32, 0);
	post_scf->ivbottom = set_bits32(post_scf->ivbottom, ivbottom, 32, 0);
	post_scf->ivbottom1 = set_bits32(post_scf->ivbottom1, ivbottom, 32, 0);
	//post_scf->uv_offset = set_bits32(post_scf->uv_offset, , 32, 0);
	post_scf->ihinc = set_bits32(post_scf->ihinc, ihinc, 32, 0);
	post_scf->ivinc = set_bits32(post_scf->ivinc, ivinc, 32, 0);

	return 0;
}

void hisi_dss_post_scf_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *post_scf_base, dss_arsr1p_t *s_post_scf)
{
	if (NULL == hisifd || NULL == post_scf_base || NULL == s_post_scf) {
		HISI_FB_ERR("NULL ptr.\n");
		return;
	}

	if (s_post_scf->dpp_used == 1) {
		hisifd->set_reg(hisifd, hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR,
			s_post_scf->dpp_img_size_bef_sr, 32, 0);
		hisifd->set_reg(hisifd, hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR,
			s_post_scf->dpp_img_size_aft_sr, 32, 0);
	}

	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IHLEFT, s_post_scf->ihleft, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IHRIGHT, s_post_scf->ihright, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IHLEFT1, s_post_scf->ihleft1, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IHRIGHT1, s_post_scf->ihright1, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IVTOP, s_post_scf->ivtop, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IVBOTTOM, s_post_scf->ivbottom, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IVBOTTOM1, s_post_scf->ivbottom1, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_UV_OFFSET, s_post_scf->uv_offset, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IHINC, s_post_scf->ihinc, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_IVINC, s_post_scf->ivinc, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_MODE, s_post_scf->mode, 32, 0);

	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SKIN_THRES_Y, s_post_scf->skin_thres_y, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SKIN_THRES_U, s_post_scf->skin_thres_u, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SKIN_THRES_V, s_post_scf->skin_thres_v, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SKIN_CFG0, s_post_scf->skin_cfg0, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SKIN_CFG1, s_post_scf->skin_cfg1, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SKIN_CFG2, s_post_scf->skin_cfg2, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHOOT_CFG1, s_post_scf->shoot_cfg1, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHOOT_CFG2, s_post_scf->shoot_cfg2, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHOOT_CFG3, s_post_scf->shoot_cfg3, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG3, s_post_scf->sharp_cfg3, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG4, s_post_scf->sharp_cfg4, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG5, s_post_scf->sharp_cfg5, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG6, s_post_scf->sharp_cfg6, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG7, s_post_scf->sharp_cfg7, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG8, s_post_scf->sharp_cfg8, 32, 0);

	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_LEVEL, s_post_scf->sharp_level, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAIN_LOW, s_post_scf->sharp_gain_low, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAIN_MID, s_post_scf->sharp_gain_mid, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAIN_HIGH, s_post_scf->sharp_gain_high, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_MF, s_post_scf->sharp_gainctrl_sloph_mf, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_MF, s_post_scf->sharp_gainctrl_slopl_mf, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_HF, s_post_scf->sharp_gainctrl_sloph_hf, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_HF, s_post_scf->sharp_gainctrl_slopl_hf, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_MF_LMT, s_post_scf->sharp_mf_lmt, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAIN_MF, s_post_scf->sharp_gain_mf, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_MF_B, s_post_scf->sharp_mf_b, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_HF_LMT, s_post_scf->sharp_hf_lmt, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_GAIN_HF, s_post_scf->sharp_gain_hf, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_HF_B, s_post_scf->sharp_hf_b, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_LF_CTRL, s_post_scf->sharp_lf_ctrl, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_LF_VAR, s_post_scf->sharp_lf_var, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_LF_CTRL_SLOP, s_post_scf->sharp_lf_ctrl_slop, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_HF_SELECT, s_post_scf->sharp_hf_select, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG2_H, s_post_scf->sharp_cfg2_h, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_SHARP_CFG2_L, s_post_scf->sharp_cfg2_l, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_TEXTURE_ANALYSIS, s_post_scf->texture_analysis, 32, 0);
	hisifd->set_reg(hisifd, post_scf_base + ARSR_POST_INTPLSHOOTCTRL, s_post_scf->intplshootctrl, 32, 0);
}

int hisi_dss_mdc_module_default(struct hisi_fb_data_type *hisifd)
{
	dss_module_reg_t *dss_mdc_module = NULL;
	char __iomem *mdc_base;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point\n");
		return -1;
	}

	mdc_base = hisifd->media_common_base;
	if (mdc_base == NULL) {
		HISI_FB_ERR("mdc_base is NULL point\n");
		return -1;
	}

	dss_mdc_module = &(hisifd->dss_mdc_module_default); //lint !e838
	memset(dss_mdc_module, 0, sizeof(dss_module_reg_t));

	HISI_FB_DEBUG("fb%d +!\n", hisifd->index);

	//RCH
	dss_mdc_module->aif_ch_base[DSS_RCHN_V0] = mdc_base + AIF0_CH2_OFFSET;
	hisi_dss_aif_init(dss_mdc_module->aif_ch_base[DSS_RCHN_V0], &(dss_mdc_module->aif[DSS_RCHN_V0]));

	dss_mdc_module->mif_ch_base[DSS_RCHN_V0] = mdc_base + MIF_CH2_OFFSET;
	hisi_dss_mif_init(dss_mdc_module->mif_ch_base[DSS_RCHN_V0], &(dss_mdc_module->mif[DSS_RCHN_V0]), DSS_RCHN_V0);

	dss_mdc_module->mctl_ch_base[DSS_RCHN_V0].chn_mutex_base = mdc_base + MCTL_MUTEX_OFFSET + MCTL_CTL_MUTEX_RCH2;
	dss_mdc_module->mctl_ch_base[DSS_RCHN_V0].chn_ov_en_base = mdc_base + MCTL_SYS_OFFSET + MCTL_SYS_RCH_OEN;
	dss_mdc_module->mctl_ch_base[DSS_RCHN_V0].chn_flush_en_base = mdc_base + MCTL_SYS_OFFSET + MCTL_RCH2_FLUSH_EN;

	dss_mdc_module->mctl_ch_base[DSS_RCHN_V0].chn_mod_dbg_base = mdc_base + MCTL_SYS_OFFSET + MCTL_MOD2_DBG;
	hisi_dss_mctl_ch_mod_dbg_init(dss_mdc_module->mctl_ch_base[DSS_RCHN_V0].chn_mod_dbg_base,
		&(dss_mdc_module->mctl_ch[DSS_RCHN_V0]));

	dss_mdc_module->dma_base[DSS_RCHN_V0] = mdc_base + RCH_OFFSET;
	hisi_dss_rdma_init(dss_mdc_module->dma_base[DSS_RCHN_V0], &(dss_mdc_module->rdma[DSS_RCHN_V0]));
	hisi_dss_rdma_u_init(dss_mdc_module->dma_base[DSS_RCHN_V0], &(dss_mdc_module->rdma[DSS_RCHN_V0]));
	hisi_dss_rdma_v_init(dss_mdc_module->dma_base[DSS_RCHN_V0], &(dss_mdc_module->rdma[DSS_RCHN_V0]));

	dss_mdc_module->dfc_base[DSS_RCHN_V0] = mdc_base + DSS_RCH_VG0_DFC_OFFSET;
	hisi_dss_dfc_init(dss_mdc_module->dfc_base[DSS_RCHN_V0], &(dss_mdc_module->dfc[DSS_RCHN_V0]));

	//WCH
	dss_mdc_module->aif_ch_base[DSS_WCHN_W1] = mdc_base + AIF0_CH9_OFFSET;
	hisi_dss_aif_init(dss_mdc_module->aif_ch_base[DSS_WCHN_W1], &(dss_mdc_module->aif[DSS_WCHN_W1]));

	dss_mdc_module->mif_ch_base[DSS_WCHN_W1] = mdc_base + MIF_CH9_OFFSET;
	hisi_dss_mif_init(dss_mdc_module->mif_ch_base[DSS_WCHN_W1], &(dss_mdc_module->mif[DSS_WCHN_W1]), DSS_WCHN_W1);

	dss_mdc_module->mctl_ch_base[DSS_WCHN_W1].chn_mutex_base = mdc_base + MCTL_MUTEX_OFFSET + MCTL_CTL_MUTEX_WCH1;
	dss_mdc_module->mctl_ch_base[DSS_WCHN_W1].chn_ov_en_base = mdc_base + MCTL_SYS_OFFSET + MCTL_SYS_WCH_IEN;
	dss_mdc_module->mctl_ch_base[DSS_WCHN_W1].chn_flush_en_base = mdc_base + MCTL_SYS_OFFSET + MCTL_WCH1_FLUSH_EN;

	dss_mdc_module->mctl_ch_base[DSS_WCHN_W1].chn_mod_dbg_base = mdc_base + MCTL_SYS_OFFSET + MCTL_MOD9_DBG;
	hisi_dss_mctl_ch_mod_dbg_init(dss_mdc_module->mctl_ch_base[DSS_WCHN_W1].chn_mod_dbg_base,
		&(dss_mdc_module->mctl_ch[DSS_WCHN_W1]));

	dss_mdc_module->scl_base[DSS_WCHN_W1] = mdc_base + WCH_OFFSET + WCH_SCF_OFFSET;
	hisi_dss_scl_init(dss_mdc_module->scl_base[DSS_WCHN_W1], &(dss_mdc_module->scl[DSS_WCHN_W1]));

	dss_mdc_module->dma_base[DSS_WCHN_W1] = mdc_base + WCH_OFFSET;
	hisi_dss_wdma_init(dss_mdc_module->dma_base[DSS_WCHN_W1], &(dss_mdc_module->wdma[DSS_WCHN_W1]));

	dss_mdc_module->dfc_base[DSS_WCHN_W1] = mdc_base + WCH_OFFSET + WCH_DFC_OFFSET;
	hisi_dss_dfc_init(dss_mdc_module->dfc_base[DSS_WCHN_W1], &(dss_mdc_module->dfc[DSS_WCHN_W1]));

	dss_mdc_module->pcsc_base[DSS_WCHN_W1] = mdc_base + WCH_OFFSET + WCH_POST_CSC_OFFSET;
	hisi_dss_csc_init(dss_mdc_module->pcsc_base[DSS_WCHN_W1], &(dss_mdc_module->pcsc[DSS_WCHN_W1]));

	dss_mdc_module->post_clip_base[DSS_WCHN_W1] = mdc_base + WCH_OFFSET + WCH_POST_CLIP_OFFSET;
	hisi_dss_post_clip_init(dss_mdc_module->post_clip_base[DSS_WCHN_W1], &(dss_mdc_module->post_clip[DSS_WCHN_W1]));

	dss_mdc_module->csc_base[DSS_WCHN_W1] = mdc_base + WCH_OFFSET + WCH_CSC_OFFSET;
	hisi_dss_csc_init(dss_mdc_module->csc_base[DSS_WCHN_W1], &(dss_mdc_module->csc[DSS_WCHN_W1]));

	dss_mdc_module->mctl_sys_base = mdc_base + MCTL_SYS_OFFSET;
	hisi_dss_mctl_sys_init(dss_mdc_module->mctl_sys_base, &(dss_mdc_module->mctl_sys));

	dss_mdc_module->mctl_base[0] = mdc_base + MCTL_MUTEX_OFFSET;
	hisi_dss_mctl_init(dss_mdc_module->mctl_base[0], &(dss_mdc_module->mctl[0]));

	dss_mdc_module->smmu_base = mdc_base + VBIF0_SMMU_OFFSET;
	hisi_dss_smmu_init(dss_mdc_module->smmu_base, &(dss_mdc_module->smmu));

	HISI_FB_DEBUG("fb%d -!\n", hisifd->index);

	return 0;
}

static void print_dsi_error_info(struct hisi_fb_data_type *hisifd, uint32_t* read_value)
{
	HISI_FB_ERR("fb%d, "
		"Number of the Errors on DSI : 0x05 = 0x%x\n"
		"Display Power Mode : 0x0A = 0x%x\n"
		"Display Signal Mode : 0x0E = 0x%x\n"
		"Display Self-Diagnostic Result : 0x0F = 0x%x\n"
		"LDI vstate : 0x%x, LDI dpi0_hstate : 0x%x\n",
		hisifd->index,
		read_value[0], read_value[1], read_value[2], read_value[3],
		inp32(hisifd->mipi_dsi0_base + MIPI_LDI_VSTATE),
		inp32(hisifd->mipi_dsi0_base + MIPI_LDI_DPI0_HSTATE));
}

static void hisifb_te_state_handle(struct hisi_fb_data_type *hisifd, uint32_t ldi_vstate,
	uint32_t phy_status, bool panel_check, uint32_t timeout)
{
	//waitting te0
	if ((LDI_VSTATE_V_WAIT_TE0 == ldi_vstate) || (!panel_check && (phy_status & BIT(1)))) {
		vactive_timeout_count++;
		if ((vactive_timeout_count >= timeout) && hisifd->panel_info.esd_enable) {
			hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
			if (hisifd->esd_ctrl.esd_check_wq) {
				queue_work(hisifd->esd_ctrl.esd_check_wq, &(hisifd->esd_ctrl.esd_check_work));
			}
	    }
	}
}

static void print_ldi_int_state(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req_dump)
{
	uint32_t isr_s1 = 0;
	uint32_t isr_s2 = 0;
	uint32_t isr_s2_mask = 0;
	uint32_t ldi_ctrl = 0;
	uint32_t ldi_frm_msk = 0;

	if (pov_req_dump == NULL) {
		return;
	}

	if (pov_req_dump->ovl_idx == DSS_OVL0) {
		isr_s1 = inp32(hisifd->dss_base + GLB_CPU_PDP_INTS);
		isr_s2_mask = inp32(hisifd->mipi_dsi0_base + 0x024C);
		isr_s2 = inp32(hisifd->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INTS);
		ldi_ctrl = inp32(hisifd->mipi_dsi0_base + MIPI_LDI_CTRL);
		ldi_frm_msk = inp32(hisifd->mipi_dsi0_base + MIPI_LDI_FRM_MSK);

		HISI_FB_ERR("fb%d, isr_s1=0x%x, isr_s2_mask=0x%x, isr_s2=0x%x, "
					"LDI_CTRL(0x%x), LDI_FRM_MSK(0x%x).\n",
					hisifd->index, isr_s1, isr_s2_mask, isr_s2, ldi_ctrl, ldi_frm_msk);
	} else if (pov_req_dump->ovl_idx == DSS_OVL1) {
		isr_s1 = inp32(hisifd->dss_base + GLB_CPU_SDP_INTS);
		if (is_dp_panel(hisifd)) {
			isr_s2_mask = inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK);
			isr_s2 = inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS);
			ldi_ctrl = inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_CTRL);
			ldi_frm_msk = inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_FRM_MSK);
		} else {
			isr_s2_mask = inp32(hisifd->mipi_dsi1_base + 0x024C);
			isr_s2 = inp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS);
			ldi_ctrl = inp32(hisifd->mipi_dsi1_base + MIPI_LDI_CTRL);
			ldi_frm_msk = inp32(hisifd->mipi_dsi1_base + MIPI_LDI_FRM_MSK);
		}
		HISI_FB_ERR("fb%d, isr_s1=0x%x, isr_s2_mask=0x%x, isr_s2=0x%x, "
			"LDI_CTRL(0x%x), LDI_FRM_MSK(0x%x).\n",
			hisifd->index, isr_s1, isr_s2_mask, isr_s2, ldi_ctrl, ldi_frm_msk);
	}
}
/*lint -e666 -e570 -e648 -e713*/
static void hisi_dump_vote_info(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_data_type *fb0 = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_fb_data_type *fb1 = hisifd_list[EXTERNAL_PANEL_IDX];
	struct hisi_fb_data_type *fb2 = hisifd_list[AUXILIARY_PANEL_IDX];
	struct hisi_fb_data_type *fb3 = hisifd_list[MEDIACOMMON_PANEL_IDX];

	HISI_FB_INFO("fb%d, voltage[%d, %d, %d, %d], edc[%llu, %llu, %llu, %llu]\n", hisifd->index,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_voltage_level : 0,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_pri_clk_rate : 0);
}

void hisi_dump_current_info(struct hisi_fb_data_type *hisifd)
{
	int i;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null!\n");
		return;
	}
	if ((hisifd->index != PRIMARY_PANEL_IDX) && (hisifd->index != EXTERNAL_PANEL_IDX))
		return;

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (g_dss_module_base[i][MODULE_DMA] == 0)
			continue;
		HISI_FB_INFO("chn%d DMA_BUF_DBG0=0x%x, DMA_BUF_DBG1=0x%x\n", i,
			inp32(hisifd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG0),
			inp32(hisifd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG1));
	}

	HISI_FB_INFO("AIF0 status: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		inp32(hisifd->dss_base + AIF0_MONITOR_OS_R0),
		inp32(hisifd->dss_base + AIF0_MONITOR_OS_R1),
		inp32(hisifd->dss_base + AIF0_MONITOR_OS_R2),
		inp32(hisifd->dss_base + AIF0_MONITOR_OS_R3),
		inp32(hisifd->dss_base + AIF0_MONITOR_OS_R4));

	HISI_FB_INFO("PERI_STAT: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		inp32(hisifd->pctrl_base + PERI_STAT0),
		inp32(hisifd->pctrl_base + PERI_STAT53),
		inp32(hisifd->pctrl_base + PERI_STAT55),
		inp32(hisifd->pmctrl_base + NOC_POWER_IDLEREQ),
		inp32(hisifd->pmctrl_base + NOC_POWER_IDLEACK),
		inp32(hisifd->pmctrl_base + PERI_INT1_MASK),
		inp32(hisifd->pmctrl_base + PERI_INT1_STAT));

	hisi_dump_vote_info(hisifd);

	HISI_FB_INFO("arsr_post: 0x%x, 0x%x,0x%x, 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_MODE),
		inp32(hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR),
		inp32(hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IHLEFT),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IHLEFT1),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IHRIGHT),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IHRIGHT1),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IVTOP),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IVBOTTOM),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IVBOTTOM1),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IHINC),
		inp32(hisifd->dss_module.post_scf_base + ARSR_POST_IVINC));

	HISI_FB_INFO("dpp: 0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x,0x%x, 0x%x, 0x%x,0x%x,0x%x, 0x%x\n",
		inp32(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT),
		inp32(hisifd->dss_base + DSS_DPP_CH0_GAMA_OFFSET + GAMA_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH1_GAMA_OFFSET + GAMA_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH0_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH1_DEGAMMA_OFFSET + DEGAMA_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH0_GMP_OFFSET + GMP_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH1_GMP_OFFSET + GMP_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH0_XCC_OFFSET + XCC_EN),
		inp32(hisifd->dss_base + DSS_DPP_CH1_XCC_OFFSET + XCC_EN),
		inp32(hisifd->dss_base + DSS_DPP_POST_XCC_OFFSET + POST_XCC_EN),
		inp32(hisifd->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_ACE),
		inp32(hisifd->dss_base + DSS_HI_ACE_OFFSET + DPE_BYPASS_NR),
		inp32(hisifd->dss_base + DSS_HI_ACE_OFFSET + DPE_IMAGE_INFO),
		inp32(hisifd->dss_base + DSS_DPP_DITHER_OFFSET + DITHER_CTL0));

	HISI_FB_INFO("ov:0x%x,dbuf:0x%x,0x%x,disp_ch:0x%x,0x%x,0x%x,dsc:0x%x\n",
		inp32(hisifd->dss_base +  DSS_OVL0_OFFSET + OV_SIZE),
		inp32(hisifd->dss_base +  DSS_DBUF0_OFFSET + DBUF_FRM_SIZE),
		inp32(hisifd->dss_base +  DSS_DBUF0_OFFSET + DBUF_FRM_HSIZE),
		inp32(hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_BEF_SR),
		inp32(hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_SR),
		inp32(hisifd->dss_base + DSS_DISP_CH0_OFFSET + IMG_SIZE_AFT_IFBCSW),
		inp32(hisifd->dss_base + DSS_DSC_OFFSET + DSC_PIC_SIZE));

	if (is_dp_panel(hisifd)) {
		HISI_FB_INFO("itf_int_mask:0x%x,itf_ints:0x%x,vstate:0x%x,dpi0_hstate:0x%x\n",
			inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK),
			inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS),
			inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_VSTATE),
			inp32(hisifd->dss_base + DSS_LDI_DP_OFFSET + LDI_DPI0_HSTATE));
	} else {
		HISI_FB_INFO("ldi-dsi:0x%x,0x%x,0x%x,0x%x\n",
			inp32(hisifd->mipi_dsi0_base + MIPI_LDI_DPI0_HRZ_CTRL2),
			inp32(hisifd->mipi_dsi0_base + MIPI_LDI_VRT_CTRL2),
			inp32(hisifd->mipi_dsi0_base + MIPIDSI_EDPI_CMD_SIZE_OFFSET),
			inp32(hisifd->mipi_dsi0_base + MIPIDSI_VID_VACTIVE_LINES_OFFSET));
	}


	HISI_FB_INFO("Mediacrg CLKDIV: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 0 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 1 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 2 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 3 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 4 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 5 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 6 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 7 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 8 * 4),
		inp32(hisifd->media_crg_base + MEDIA_CLKDIV1 + 9 * 4));


	HISI_FB_INFO("PMCTRL PERI_CTRL4, CTRL5 = 0x%x, 0x%x \n",
		inp32(hisifd->pmctrl_base + PMCTRL_PERI_CTRL4),
		inp32(hisifd->pmctrl_base + PMCTRL_PERI_CTRL5));
}

static int hisi_vactive0_start_config_br1(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req)
{
	int ret = 0;
	int ret1 = 0;
	int times = 0;
	bool panel_check = true;
	struct timeval tv0;
	struct timeval tv1;
	uint32_t timeout_interval = 0;
	dss_overlay_t *pov_req_dump = NULL;
	dss_overlay_t *pov_req_prev = NULL;
	dss_overlay_t *pov_req_prev_prev = NULL;
	uint32_t cmdlist_idxs = 0;
	uint32_t cmdlist_idxs_prev = 0;
	uint32_t cmdlist_idxs_prev_prev = 0;
	uint32_t read_value[4] = {0};
	char __iomem *mipi_dsi_base;
	uint32_t phy_status;
	uint32_t ldi_vstate;

	pov_req_prev = &(hisifd->ov_req_prev); //lint !e838
	pov_req_prev_prev = &(hisifd->ov_req_prev_prev); //lint !e838

	if (g_fpga_flag == 0) {
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC;
	} else {
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX) {
		mipi_dsi_base = hisifd->mipi_dsi1_base;
	} else {
		mipi_dsi_base = hisifd->mipi_dsi0_base;
	}
	//pov_req_dump = &(hisifd->ov_req_prev);
	pov_req_dump = &(hisifd->ov_req_prev_prev); //lint !e838
	if ((hisifd->vactive0_start_flag == 1)
		&& (hisifd->secure_ctrl.secure_event == DSS_SEC_ENABLE)
		&& (pov_req_prev->sec_enable_status == DSS_SEC_ENABLE)) {
		hisifd->vactive0_start_flag = 0;
		single_frame_update(hisifd);
	}

	if (hisifd->vactive0_start_flag == 0) {
		hisifb_get_timestamp(&tv0);

		times = 0;
		while (1) {
			ret = wait_event_interruptible_timeout(hisifd->vactive0_start_wq, hisifd->vactive0_start_flag, msecs_to_jiffies(timeout_interval));
			if ((ret == -ERESTARTSYS) && (times++ < 50)) {
				mdelay(10);
			} else {
				break;
			}
		}

		if (ret <= 0) {
			hisifb_get_timestamp(&tv1);

			ret1 = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmdlist_idxs_prev, NULL);
			if (ret1 != 0) {
				HISI_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev failed! ret = %d\n", hisifd->index, ret1);
			}

			ret1 = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev_prev, &cmdlist_idxs_prev_prev, NULL);
			if (ret1 != 0) {
				HISI_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev_prev failed! ret = %d\n", hisifd->index, ret1);
			}

			cmdlist_idxs = cmdlist_idxs_prev | cmdlist_idxs_prev_prev;
			// cppcheck-suppress *
			HISI_FB_ERR("fb%d, 1st timeout!ret=%d, "
				"flag=%d, pre_pre_frm_no=%u, frm_no=%u, timediff= %u us, "
				"cmdlist_idxs_prev=0x%x, cmdlist_idxs_prev_prev=0x%x, cmdlist_idxs=0x%x.\n",
				hisifd->index, ret, hisifd->vactive0_start_flag, pov_req_dump->frame_no, pov_req->frame_no,
				hisifb_timestamp_diff(&tv0, &tv1),
				cmdlist_idxs_prev, cmdlist_idxs_prev_prev, cmdlist_idxs);

			ldi_vstate = inp32(mipi_dsi_base + MIPI_LDI_VSTATE);
			phy_status = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
			HISI_FB_ERR("itf_ints=0x%x, dpp_dbg=0x%x, online_fill_lvl=0x%x, ldi_vstate=0x%x, phy_status=0x%x.\n",
				inp32(mipi_dsi_base + MIPI_LDI_CPU_ITF_INTS),
				inp32(hisifd->dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DBG_CNT),
				inp32(hisifd->dss_base + DSS_DBUF0_OFFSET + DBUF_ONLINE_FILL_LEVEL),
				ldi_vstate, phy_status);

			hisi_dump_current_info(hisifd);
			if (g_debug_ldi_underflow || g_debug_ovl_online_composer_hold)
				hisi_dss_underflow_dump_cmdlist(hisifd, pov_req_prev, pov_req_prev_prev);

			if (g_debug_ovl_online_composer_hold) {
				mdelay(HISI_DSS_COMPOSER_HOLD_TIME);
			}

			if (g_debug_ldi_underflow_clear && g_ldi_data_gate_en) {
				hisi_cmdlist_config_reset(hisifd, pov_req_dump, cmdlist_idxs);
				ldi_data_gate(hisifd, false);
				hisifd->underflow_flag = 1;

				panel_check = mipi_panel_check_reg(hisifd, read_value);
				print_dsi_error_info(hisifd, read_value);

				memset(&(hisifd->ov_block_infos_prev), 0,
					HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
				hisifb_dss_overlay_info_init(&hisifd->ov_req_prev);
				hisifb_te_state_handle(hisifd, ldi_vstate, phy_status, panel_check, 1);
				return 0;
			}

			ldi_data_gate(hisifd, false);
			panel_check = mipi_panel_check_reg(hisifd, read_value);
			print_dsi_error_info(hisifd, read_value);

			times = 0;
			while (1) {
				ret = wait_event_interruptible_timeout(hisifd->vactive0_start_wq, hisifd->vactive0_start_flag, msecs_to_jiffies(timeout_interval));
				if ((ret == -ERESTARTSYS) && (times++ < 50)) {
						mdelay(10);
				} else {
					break;
				}
			}

			if (ret <= 0) {
				HISI_FB_ERR("fb%d, 2end timeout!ret=%d, "
					"vactive0_start_flag=%d, frame_no=%u.\n",
					hisifd->index, ret, hisifd->vactive0_start_flag, pov_req_dump->frame_no);

				ldi_data_gate(hisifd, false);
				ret = -ETIMEDOUT;
				print_ldi_int_state(hisifd, pov_req_dump);

				ldi_vstate = inp32(mipi_dsi_base + MIPI_LDI_VSTATE);
				phy_status = inp32(mipi_dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
				hisifb_te_state_handle(hisifd, ldi_vstate, phy_status, panel_check, 1);
			} else {
				ldi_data_gate(hisifd, true);
				ret = 0;
			}
		} else {
			ldi_data_gate(hisifd, true);
			ret = 0;
		}
	}

	//ldi_data_gate(hisifd, true);
	hisifd->vactive0_start_flag = 0;
	hisifd->vactive0_end_flag = 0;
	if (ret >= 0) {
		vactive_timeout_count = 0;
	}

	return ret;
}

int hisi_vactive0_start_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req)
{
	int ret = 0;
	int times = 0;
	uint32_t prev_vactive0_start = 0;
	struct timeval tv0;
	struct timeval tv1;
	uint32_t timeout_interval = 0;
	dss_overlay_t *pov_req_dump = NULL;
	dss_overlay_t *pov_req_prev = NULL;
	dss_overlay_t *pov_req_prev_prev = NULL;
	uint32_t cmdlist_idxs = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	if (NULL == pov_req) {
		HISI_FB_ERR("pov_req is NULL");
		return -EINVAL;
	}

	pov_req_prev = &(hisifd->ov_req_prev); //lint !e838
	pov_req_prev_prev = &(hisifd->ov_req_prev_prev); //lint !e838

	if (g_fpga_flag == 0) {
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC;
	} else {
		timeout_interval = DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;
	}

	if (is_mipi_cmd_panel(hisifd) && (hisifd->frame_update_flag == 0)) {
		ret = hisi_vactive0_start_config_br1(hisifd, pov_req);
	} else {
		pov_req_dump = &(hisifd->ov_req_prev);

		hisifb_get_timestamp(&tv0);
		ldi_data_gate(hisifd, false);
		prev_vactive0_start = hisifd->vactive0_start_flag;

		while (1) {
			ret = wait_event_interruptible_timeout(hisifd->vactive0_start_wq, (prev_vactive0_start != hisifd->vactive0_start_flag),
				msecs_to_jiffies(timeout_interval));
			if ((ret == -ERESTARTSYS) && (times++ < 50)) {
					mdelay(10);
			} else {
				break;
			}
		}

		if (g_fastboot_enable_flag && g_enable_ovl_cmdlist_online)
			set_reg(hisifd->dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_TOP, 0x1, 32, 0);

		if (ret <= 0) {
			hisifb_get_timestamp(&tv1);

			ret = hisi_cmdlist_get_cmdlist_idxs(pov_req_dump, &cmdlist_idxs, NULL);
			if (ret != 0) {
				HISI_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev failed! ret = %d\n", hisifd->index, ret);
			}

			HISI_FB_ERR("fb%d, 1wait_for vactive0_start_flag timeout!ret=%d, "
				"vactive0_start_flag=%d, pre frame_no=%u, TIMESTAMP_DIFF is %u us,"
				"cmdlist_idxs=0x%x!\n",
				hisifd->index, ret, hisifd->vactive0_start_flag, pov_req_dump->frame_no,
				hisifb_timestamp_diff(&tv0, &tv1), cmdlist_idxs);

			hisi_dump_current_info(hisifd);
			if (g_debug_ldi_underflow || g_debug_ovl_online_composer_hold)
				hisi_dss_underflow_dump_cmdlist(hisifd, pov_req_dump, NULL);

			if (g_debug_ovl_online_composer_hold) {
				mdelay(HISI_DSS_COMPOSER_HOLD_TIME);
			}
			ret = 0;
		} else {
			ret = 0;
		}
	}
	return ret;
}
/*lint +e666 +e570 +e648 +e713*/
/*lint +e438 +e550 +e573 +e647 +e701 +e712 +e713 +e737 +e834 +e838 +e845*/

#pragma GCC diagnostic pop
