/* Copyright (c) 2012-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_dpe_utils.h"
#if defined (CONFIG_HISI_PERIDVFS)
#include "peri_volt_poll.h"
#endif

DEFINE_SEMAPHORE(hisi_fb_dss_inner_clk_sem);

static int dss_inner_clk_refcount = 0;
static unsigned  int g_comform_value = 0;
static unsigned  int g_acm_State = 0;
static unsigned  int g_gmp_State = 0;
static unsigned int g_led_rg_csc_value[9];
static unsigned int g_is_led_rg_csc_set;
unsigned int g_led_rg_para1 = 7;
unsigned int g_led_rg_para2 = 30983;

#define OFFSET_FRACTIONAL_BITS	(11)
#define ROUND1(x,y)	((x) / (y) + ((x) % (y)  ? 1 : 0))
#define gmp_cnt_cofe (4913) //17*17*17
#define xcc_cnt_cofe (12)

#define PERI_VOLTAGE_LEVEL0_060V		(0) // 0.60v
#define PERI_VOLTAGE_LEVEL1_065V		(1) // 0.65v
#define PERI_VOLTAGE_LEVEL2_070V		(2) // 0.70v
#define PERI_VOLTAGE_LEVEL3_080V		(3) // 0.80v

#if CONFIG_SH_AOD_ENABLE
extern bool hisi_aod_get_aod_status(void);
#endif

/*lint -e647*/
static int get_lcd_frame_rate(struct hisi_panel_info *pinfo)
{
	return pinfo->pxl_clk_rate / (pinfo->xres + pinfo->pxl_clk_rate_div *
		(pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width))/(pinfo->yres +
		pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width);
}
/*lint +e647*/

static void hisifb_set_default_pri_clk_rate(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = &(hisifd->dss_vote_cmd);
	frame_rate = get_lcd_frame_rate(pinfo);

	if (g_fpga_flag == 1) {
		pdss_vote_cmd->dss_pri_clk_rate = 40 * 1000000UL;
	} else {
		if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
			if (frame_rate >= 110) {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
			} else {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
			}
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		} else {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		}
	}

	return;
}

bool is_vote_needed_for_low_temp(bool is_lowtemp, int volt_to_set)
{
	if (is_lowtemp && (volt_to_set == PERI_VOLTAGE_LEVEL3_080V)) {
		HISI_FB_INFO("is_lowtemp, vlotage cannot exceed 0.7v\n");
		return false;
	}

	return true;
}

int get_dss_clock_value(uint32_t voltage_level, dss_vote_cmd_t *dss_vote_cmd)
{
	if (dss_vote_cmd == NULL) {
		HISI_FB_ERR("dss_vote_cmd is NULL point! \n");
		return -EINVAL;
	}

	switch (voltage_level) {
	case PERI_VOLTAGE_LEVEL3:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		break;
	case PERI_VOLTAGE_LEVEL2:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL1:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L2;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL0:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		break;
	default:
		HISI_FB_ERR("wrong voltage value %d\n", voltage_level);
		return -EINVAL;
	}

	return 0;
}

static uint32_t get_clk_vote_level(dss_vote_cmd_t vote_cmd)
{
	uint32_t level = PERI_VOLTAGE_LEVEL0;

	switch(vote_cmd.dss_pri_clk_rate) {
	case DEFAULT_DSS_CORE_CLK_RATE_L4:
		level = PERI_VOLTAGE_LEVEL3;
		break;
	case DEFAULT_DSS_CORE_CLK_RATE_L3:
		level = PERI_VOLTAGE_LEVEL2;
		break;
	case DEFAULT_DSS_CORE_CLK_RATE_L2:
		level = PERI_VOLTAGE_LEVEL1;
		break;
	case DEFAULT_DSS_CORE_CLK_RATE_L1:
		level = PERI_VOLTAGE_LEVEL0;
		break;
	default:
		HISI_FB_ERR("wrong pri clk rate %llu\n", vote_cmd.dss_pri_clk_rate);
	}

	return level;
}

static void set_dss_perf_level_debug(struct hisi_fb_data_type *hisifd)
{
	struct dss_vote_cmd *pdss_vote_cmd = &(hisifd->dss_vote_cmd);

	if (g_dss_perf_debug <= 0)
		return;

	hisifd->core_clk_upt_support = 0;

	switch (g_dss_perf_debug - 1) {
	case PERI_VOLTAGE_LEVEL0:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		break;
	case PERI_VOLTAGE_LEVEL1:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L2;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL2:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL3:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		break;
	default:
		break;
	}

	HISI_FB_INFO("set edc %llu, mmbuf %llu",
		pdss_vote_cmd->dss_pri_clk_rate, pdss_vote_cmd->dss_mmbuf_rate);
}

struct dss_vote_cmd *get_dss_vote_cmd(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return pdss_vote_cmd;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = &(hisifd->dss_vote_cmd);
	frame_rate = get_lcd_frame_rate(pinfo);

	/* FIXME: TBD  */
	if (g_fpga_flag == 1) {
		if (pdss_vote_cmd->dss_pri_clk_rate == 0) {
			pdss_vote_cmd->dss_pri_clk_rate = 40 * 1000000UL;
		}
	} else {
		if (pdss_vote_cmd->dss_pri_clk_rate == 0) {
			if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
				hisifd->core_clk_upt_support = 0;
			} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
				if (frame_rate >= 110) {
					pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
					hisifd->core_clk_upt_support = 0;
				} else {
					pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
					hisifd->core_clk_upt_support = 1;
				}
			} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
				hisifd->core_clk_upt_support = 1;
			} else {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
				hisifd->core_clk_upt_support = 1;
			}
			pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
			pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
		}

		if (hisifd->index == EXTERNAL_PANEL_IDX) {
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		}
	}

	set_dss_perf_level_debug(hisifd);

	return pdss_vote_cmd;
}

static int get_mdc_clk_rate(dss_vote_cmd_t vote_cmd, uint64_t *clk_rate)
{
	switch (vote_cmd.dss_voltage_level) {
	case PERI_VOLTAGE_LEVEL0:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L1;
		break;
	case PERI_VOLTAGE_LEVEL1:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL2:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL3:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L4;
		break;
	default:
		HISI_FB_ERR("no support set dss_voltage_level(%d)! \n", vote_cmd.dss_voltage_level);
		return -1;
	}

	HISI_FB_DEBUG("get mdc clk rate: %llu \n", *clk_rate);
	return 0;
}

static int set_mdc_core_clk(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret;
	uint64_t clk_rate = 0;

	if (vote_cmd.dss_voltage_level == hisifd->dss_vote_cmd.dss_voltage_level) {
		return 0;
	}

	if (get_mdc_clk_rate(vote_cmd, &clk_rate)) {
		HISI_FB_ERR("get mdc clk rate failed! \n");
		return -1;
	}

	ret = clk_set_rate(hisifd->dss_clk_media_common_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("set dss_clk_media_common_clk(%llu) failed, error=%d!\n", clk_rate, ret);
		return -1;
	}
	hisifd->dss_vote_cmd.dss_voltage_level = vote_cmd.dss_voltage_level;

	HISI_FB_INFO("set dss_clk_media_common_clk = %llu.\n", clk_rate);

	return ret;
}

static int dss_core_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	if (hisifd->dss_pri_clk != NULL) {
		ret = clk_prepare(hisifd->dss_pri_clk);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pri_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(hisifd->dss_pri_clk);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pri_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	if (hisifd->dss_axi_clk != NULL) {
		ret = clk_prepare_enable(hisifd->dss_axi_clk);
		if (ret)
			HISI_FB_ERR("fb%d dss_core_clk_enable,dss_axi_clk,clk_prepare_enable failed, error=%d!\n",
				hisifd->index, ret);
	}

	if (hisifd->dss_mmbuf_clk != NULL) {
		ret = clk_prepare_enable(hisifd->dss_mmbuf_clk);
		if (ret)
			HISI_FB_ERR("fb%d dss_core_clk_enable,dss_mmbuf_clk,clk_prepare_enable failed, error=%d!\n",
				hisifd->index, ret);
	}

	return 0;
}

static int dss_core_clk_disable(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	if (hisifd->dss_pri_clk != NULL) {
		clk_disable(hisifd->dss_pri_clk);
		clk_unprepare(hisifd->dss_pri_clk);
	}

	if (hisifd->dss_axi_clk != NULL) {
		clk_disable_unprepare(hisifd->dss_axi_clk);
	}

	if (hisifd->dss_mmbuf_clk != NULL) {
		clk_disable_unprepare(hisifd->dss_mmbuf_clk);
	}

	return 0;
}

static void hisifb_get_other_fb_clk_vote(struct hisi_fb_data_type *hisifd,
	dss_vote_cmd_t *other_fb_clk_rate)
{
	struct hisi_fb_data_type *targetfd1 = NULL;
	struct hisi_fb_data_type *targetfd2 = NULL;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		targetfd1 = hisifd_list[EXTERNAL_PANEL_IDX];
		targetfd2 = hisifd_list[AUXILIARY_PANEL_IDX];
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		targetfd1 = hisifd_list[PRIMARY_PANEL_IDX];
		targetfd2 = hisifd_list[AUXILIARY_PANEL_IDX];
	} else {
		targetfd1 = hisifd_list[PRIMARY_PANEL_IDX];
		targetfd2 = hisifd_list[EXTERNAL_PANEL_IDX];
	}

	if ((targetfd1 == NULL) && (targetfd2 == NULL)) {
		return;
	} else if (targetfd1 == NULL) {
		other_fb_clk_rate->dss_pri_clk_rate = targetfd2->dss_vote_cmd.dss_pri_clk_rate;
		other_fb_clk_rate->dss_mmbuf_rate = targetfd2->dss_vote_cmd.dss_mmbuf_rate;
	} else if (targetfd2 == NULL) {
		other_fb_clk_rate->dss_pri_clk_rate = targetfd1->dss_vote_cmd.dss_pri_clk_rate;
		other_fb_clk_rate->dss_mmbuf_rate = targetfd1->dss_vote_cmd.dss_mmbuf_rate;
	} else {
		if (targetfd1->dss_vote_cmd.dss_pri_clk_rate > targetfd2->dss_vote_cmd.dss_pri_clk_rate) {
			other_fb_clk_rate->dss_pri_clk_rate = targetfd1->dss_vote_cmd.dss_pri_clk_rate;
			other_fb_clk_rate->dss_mmbuf_rate = targetfd1->dss_vote_cmd.dss_mmbuf_rate;
		} else {
			other_fb_clk_rate->dss_pri_clk_rate = targetfd2->dss_vote_cmd.dss_pri_clk_rate;
			other_fb_clk_rate->dss_mmbuf_rate = targetfd2->dss_vote_cmd.dss_mmbuf_rate;
		}
	}

	return;
}

static bool is_clk_voltage_matching(struct hisi_fb_data_type *hisifd,
	dss_vote_cmd_t target_dss_vote_cmd)
{
	int ret;
	int current_peri_voltage = 0;
	struct dss_vote_cmd supportable_clk_rate;
	struct hisi_fb_data_type *fb0 = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_fb_data_type *fb1 = hisifd_list[EXTERNAL_PANEL_IDX];
	struct hisi_fb_data_type *fb2 = hisifd_list[AUXILIARY_PANEL_IDX];

	memset(&supportable_clk_rate, 0, sizeof(supportable_clk_rate));

	(void)dss_get_peri_volt(&current_peri_voltage);
	ret = get_dss_clock_value(dpe_get_voltage_level(current_peri_voltage), &supportable_clk_rate);
	if (ret) {
		return false;
	}
	if ((target_dss_vote_cmd.dss_pri_clk_rate > supportable_clk_rate.dss_pri_clk_rate) ||
		(target_dss_vote_cmd.dss_mmbuf_rate > supportable_clk_rate.dss_mmbuf_rate)) {
		HISI_FB_ERR("fb%d, current voltage %d does not match clk rate %llu, %llu\n",
			hisifd->index, current_peri_voltage,
			target_dss_vote_cmd.dss_pri_clk_rate,
			target_dss_vote_cmd.dss_mmbuf_rate);

		if ((fb0 != NULL) && fb0->panel_power_on) {
			HISI_FB_INFO("CLKDIV2 0x%x, PERI_CTRL4 & CTRL5 = 0x%x & 0x%x\n",
				inp32(fb0->media_crg_base + MEDIA_CLKDIV1 + 0x4),
				inp32(fb0->pmctrl_base + PMCTRL_PERI_CTRL4),
				inp32(fb0->pmctrl_base + PMCTRL_PERI_CTRL5));
		}
		HISI_FB_INFO("volt: %d, %d, %d, edcClk: %llu, %llu, %llu\n",
			(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
			(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
			(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
			(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
			(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
			(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0);

		return false;
	}

	return true;
}

static int hisifb_set_edc_mmbuf_clk(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t target_dss_vote_cmd)
{
	int ret = 0;

	if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
		ret = dss_core_clk_enable(hisifd);
		if (ret < 0) {
			HISI_FB_ERR("dss_core_clk_enable failed, error=%d\n", ret);
			return -1;
		}
	}
	ret = clk_set_rate(hisifd->dss_pri_clk, target_dss_vote_cmd.dss_pri_clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("set dss_pri_clk_rate %llu failed, error=%d\n", target_dss_vote_cmd.dss_pri_clk_rate, ret);
		return -1;
	}

	ret = clk_set_rate(hisifd->dss_mmbuf_clk, target_dss_vote_cmd.dss_mmbuf_rate);
	if (ret < 0) {
		HISI_FB_ERR("set dss_mmbuf_rate %llu failed, error=%d\n", target_dss_vote_cmd.dss_mmbuf_rate, ret);
		return -1;
	}

	if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
		ret = dss_core_clk_disable(hisifd);
		if (ret < 0) {
			HISI_FB_ERR("dss_core_clk_disable, error=%d\n", ret);
			return -1;
		}
	}

	return ret;
}

static int hisifb_edc_mmbuf_clk_vote(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t dss_vote_cmd)
{
	dss_vote_cmd_t other_fb_clk_rate;
	dss_vote_cmd_t target_dss_vote_cmd;
	struct hisi_fb_data_type *fb0 = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_fb_data_type *fb1 = hisifd_list[EXTERNAL_PANEL_IDX];
	struct hisi_fb_data_type *fb2 = hisifd_list[AUXILIARY_PANEL_IDX];

	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));
	memset(&target_dss_vote_cmd, 0, sizeof(target_dss_vote_cmd));

	hisifb_get_other_fb_clk_vote(hisifd, &other_fb_clk_rate);
	if (dss_vote_cmd.dss_pri_clk_rate >= other_fb_clk_rate.dss_pri_clk_rate) {
		target_dss_vote_cmd.dss_pri_clk_rate = dss_vote_cmd.dss_pri_clk_rate;
		target_dss_vote_cmd.dss_mmbuf_rate = dss_vote_cmd.dss_mmbuf_rate;
	} else {
		target_dss_vote_cmd.dss_pri_clk_rate = other_fb_clk_rate.dss_pri_clk_rate;
		target_dss_vote_cmd.dss_mmbuf_rate = other_fb_clk_rate.dss_mmbuf_rate;
	}

	if (!is_clk_voltage_matching(hisifd, target_dss_vote_cmd))
		return -1;

	if (hisifb_set_edc_mmbuf_clk(hisifd, target_dss_vote_cmd))
		return -1;

	HISI_FB_DEBUG("fb%d, vote %llu, set %llu, get coreClk = %llu mmbufClk = %llu\n", hisifd->index,
		dss_vote_cmd.dss_pri_clk_rate, target_dss_vote_cmd.dss_pri_clk_rate,
		(uint64_t)clk_get_rate(hisifd->dss_pri_clk), (uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));

	HISI_FB_DEBUG("edc: %llu, %llu, %llu, mmbuf: %llu, %llu, %llu\n",
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_mmbuf_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_mmbuf_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_mmbuf_rate : 0);

	return 0;
}

int set_dss_vote_cmd(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		/* clk_media_common's voltage following the frequency */
		return set_mdc_core_clk(hisifd, vote_cmd);
	}

	if ((vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L1)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L2)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L3)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L4)) {
		HISI_FB_ERR("fb%d no support dss_pri_clk_rate %llu\n", hisifd->index, vote_cmd.dss_pri_clk_rate);
		return -EINVAL;
	}

	if ((vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L1)
		&& (vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L2)
		&& (vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L3)
		&& (vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L4)) {
		HISI_FB_ERR("fb%d no support dss_mmbuf_rate %llu\n", hisifd->index, vote_cmd.dss_pri_clk_rate);
		return -EINVAL;
	}

	if ((hisifd->index != AUXILIARY_PANEL_IDX) &&
		(vote_cmd.dss_pri_clk_rate == hisifd->dss_vote_cmd.dss_pri_clk_rate)) {
		return ret;
	}

	if ((hisifd->index == AUXILIARY_PANEL_IDX) && (!hisifd_list[PRIMARY_PANEL_IDX]->panel_power_on)) {
		HISI_FB_INFO("fb%d, primary_pane is power off\n", hisifd->index);
		return -EINVAL;
	}

	if (hisifd->index == AUXILIARY_PANEL_IDX) {
		hisifd->clk_vote_level = get_clk_vote_level(vote_cmd);
		HISI_FB_DEBUG("fb%d, get clk_vote_level = %d\n", hisifd->index, hisifd->clk_vote_level);
		return ret;
	}
	ret = hisifb_edc_mmbuf_clk_vote(hisifd, vote_cmd);

	if (ret == 0) {
		hisifd->dss_vote_cmd.dss_pri_clk_rate = vote_cmd.dss_pri_clk_rate;
		hisifd->dss_vote_cmd.dss_axi_clk_rate = vote_cmd.dss_axi_clk_rate;
		hisifd->dss_vote_cmd.dss_mmbuf_rate = vote_cmd.dss_mmbuf_rate;
	}

	return ret;
}

int hisifb_restore_dss_voltage_clk_vote(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret = 0;
	int volt_to_set;
	int timeout = 10;
	int current_peri_voltage = 0;

	if (hisifd == NULL)
		return 0;
	if (hisifd->index != PRIMARY_PANEL_IDX)
		return 0;

	down(&g_hisifb_dss_clk_vote_sem);

	if ((hisifd->dss_vote_cmd.dss_voltage_level != 0) ||
		(hisifd->dss_vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L1)) {
		HISI_FB_ERR("break restore, expect: %d, %llu, now: %d, %llu\n",
			vote_cmd.dss_voltage_level, vote_cmd.dss_pri_clk_rate,
			hisifd->dss_vote_cmd.dss_voltage_level, hisifd->dss_vote_cmd.dss_pri_clk_rate);
	}

	volt_to_set = dpe_get_voltage_value(vote_cmd.dss_voltage_level);
	ret = hisifb_set_dss_vote_voltage(hisifd, vote_cmd.dss_voltage_level, &current_peri_voltage);
	if (ret < 0) {
		HISI_FB_ERR("set volt %d fail\n", vote_cmd.dss_voltage_level);
		goto vote_out;
	}

	while ((current_peri_voltage < volt_to_set) && (timeout > 0)) {
		msleep(1);
		dss_get_peri_volt(&current_peri_voltage);
		timeout--;
	}

	if (timeout > 0) {
		ret = set_dss_vote_cmd(hisifd, vote_cmd);
		if (ret < 0) {
			HISI_FB_ERR("set clk %llu, %llu fail\n", vote_cmd.dss_pri_clk_rate, vote_cmd.dss_mmbuf_rate);
			goto vote_out;
		}
	} else {
		HISI_FB_ERR("timeout, current_peri_voltage %d, less than expect %d\n", current_peri_voltage, volt_to_set);
		goto vote_out;
	}

	HISI_FB_INFO("restore vote: %d, %llu, %llu succ\n",
		hisifd->dss_vote_cmd.dss_voltage_level,
		hisifd->dss_vote_cmd.dss_pri_clk_rate,
		hisifd->dss_vote_cmd.dss_mmbuf_rate);

vote_out:
	up(&g_hisifb_dss_clk_vote_sem);
	return ret;
}

int hisifb_offline_vote_ctrl(struct hisi_fb_data_type *hisifd, bool offline_start)
{
	int ret = 0;
	struct dss_vote_cmd other_fb_clk_rate;
	struct dss_vote_cmd copybit_clk_rate;

	if (hisifd->index != AUXILIARY_PANEL_IDX) {
		return -1;
	}

	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));
	memset(&copybit_clk_rate, 0, sizeof(copybit_clk_rate));

	down(&g_hisifb_dss_clk_vote_sem);
	hisifb_get_other_fb_clk_vote(hisifd, &other_fb_clk_rate);

	if (offline_start) {
		ret = get_dss_clock_value(hisifd->clk_vote_level, &copybit_clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("get_dss_clock_value fail, level = %d\n", hisifd->clk_vote_level);
			goto offline_vote_out;
		}

		if (copybit_clk_rate.dss_pri_clk_rate > other_fb_clk_rate.dss_pri_clk_rate) {
			if (!is_clk_voltage_matching(hisifd, copybit_clk_rate)) {
				ret = -1;
				HISI_FB_ERR("offline play vote mismatch\n");
				goto offline_vote_out;
			}
			ret = hisifb_set_edc_mmbuf_clk(hisifd, copybit_clk_rate);
			if (ret < 0) {
				HISI_FB_ERR("fb%d offline start set dss_pri_clk_rate %llu fail\n", hisifd->index, copybit_clk_rate.dss_pri_clk_rate);
				ret = -1;
				goto offline_vote_out;
			}
		}

		hisifd->dss_vote_cmd.dss_pri_clk_rate = copybit_clk_rate.dss_pri_clk_rate;
		hisifd->dss_vote_cmd.dss_mmbuf_rate = copybit_clk_rate.dss_mmbuf_rate;
		HISI_FB_DEBUG("fb%d offline start set dss_pri_clk_rate %llu\n", hisifd->index, hisifd->dss_vote_cmd.dss_pri_clk_rate);
	} else {
		/* offline play end, recover fb2 voltage&CoreClk to max of other fb */
		if (hisifd->dss_vote_cmd.dss_pri_clk_rate > other_fb_clk_rate.dss_pri_clk_rate) {
			ret = hisifb_set_edc_mmbuf_clk(hisifd, other_fb_clk_rate);
			if (ret < 0) {
				HISI_FB_ERR("offline play end, set pri clk %llu fail\n", other_fb_clk_rate.dss_pri_clk_rate);
			} else {
				HISI_FB_DEBUG("fb%d offline end set dss_pri_clk_rate %llu\n", hisifd->index, other_fb_clk_rate.dss_pri_clk_rate);
			}
		}

		hisifd->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		hisifd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}

offline_vote_out:
	up(&g_hisifb_dss_clk_vote_sem);
	return ret;
}

int hisifb_set_dss_external_vote_pre(struct hisi_fb_data_type *hisifd, uint64_t pixel_clock)
{
	int ret = 0;
	unsigned int voltage_value = 0;
	int voltage_timeout = 15;
	uint64_t pixel_clock_ori;
	dss_vote_cmd_t vote_cmd;
	struct peri_volt_poll *pvp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	pixel_clock_ori = pixel_clock * 2; // double pixel
	memset(&vote_cmd, 0, sizeof(vote_cmd));

	if (EXTERNAL_PANEL_IDX == hisifd->index) {
		if (pixel_clock_ori > DEFAULT_DSS_PXL1_CLK_RATE_L3) {
			hisifd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL3;
			voltage_value = PERI_VOLTAGE_LEVEL3_080V;
			vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
			vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
			vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL3;
		} else if (pixel_clock_ori > DEFAULT_DSS_PXL1_CLK_RATE_L2) {
			hisifd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL2;
			voltage_value = PERI_VOLTAGE_LEVEL2_070V;
			vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
			vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
			vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL2;
		} else if (pixel_clock_ori > DEFAULT_DSS_PXL1_CLK_RATE_L1) {
			hisifd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL1;
			voltage_value = PERI_VOLTAGE_LEVEL1_065V;
			vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L2;
			vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
			vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL1;
		} else {
			hisifd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
			voltage_value = PERI_VOLTAGE_LEVEL0_060V;
			vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
			vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
			vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
		}

#if defined (CONFIG_HISI_PERIDVFS)
		pvp = peri_volt_poll_get(DEV_LDI1_VOLTAGE_ID, NULL);
		if (pvp == NULL) {
			HISI_FB_ERR("[DP] get pvp failed!\n");
			return -EINVAL;
		}

		ret = peri_set_volt(pvp, voltage_value);
		if (ret) {
			HISI_FB_ERR("[DP] set voltage_value failed!\n");
			return -EINVAL;
		}

		while (voltage_timeout) {
			mdelay(10);
			if (voltage_value <= peri_get_volt(pvp)) {
				break;
			} else {
				voltage_timeout--;
			}
		}

		if (voltage_value > peri_get_volt(pvp)) {
			HISI_FB_ERR("[DP] set voltage_value time out!! request volt(%d), cur volt(%d)\n", voltage_value, peri_get_volt(pvp));
			return  -EINVAL;
		}
#endif

		/* pixel clock/2 need more than DSS core clk. In double pixel mode, pixel clock is half of previous.
		So it only need pixel clock more than DSS core clk. */
		if ((voltage_value > PERI_VOLTAGE_LEVEL0_060V) && (pixel_clock > DEFAULT_DSS_CORE_CLK_RATE_L1)) {
			down(&hisifd_list[PRIMARY_PANEL_IDX]->dp_vote_sem);
			ret = set_dss_vote_cmd(hisifd, vote_cmd);
			if (ret < 0) {
				HISI_FB_ERR("[DP] DSS core clk set failed!!\n");
				up(&hisifd_list[PRIMARY_PANEL_IDX]->dp_vote_sem);
				return ret;
			}
			up(&hisifd_list[PRIMARY_PANEL_IDX]->dp_vote_sem);
		} else {
			hisifd->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
			hisifd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		}
	}
	return 0;
}

static void hisi_recovery_external_clk_rate(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX) {
		hisifd->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		hisifd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		hisifd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
		hisifb_edc_mmbuf_clk_vote(hisifd, hisifd->dss_vote_cmd);
	}
}

/*lint -e712 -e838*/
static int dpe_set_pxl_clk_rate(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo;
	int ret = 0;

	pinfo = &(hisifd->panel_info);

	if (is_dp_panel(hisifd)) {
		if (hisifd->dp_pxl_ppll7_init != NULL) {
			ret = hisifd->dp_pxl_ppll7_init(hisifd, pinfo->pxl_clk_rate);
		} else {
			ret = clk_set_rate(hisifd->dss_pxl1_clk, pinfo->pxl_clk_rate);
		}

		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, pinfo->pxl_clk_rate, ret);

			if (g_fpga_flag == 0) {
				return -EINVAL;
			}
		}
		HISI_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl1_clk));
	}

	return ret;
}

static void get_mmbuf_clk_rate(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	uint64_t pxl_clk_rate = 0;

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = &(hisifd->dss_vote_cmd);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		pxl_clk_rate = pinfo->pxl_clk_rate * 2;
		if (pxl_clk_rate > DEFAULT_DSS_PXL1_CLK_RATE_L3) {
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		} else if (pxl_clk_rate > DEFAULT_DSS_PXL1_CLK_RATE_L2) {
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		} else if (pxl_clk_rate > DEFAULT_DSS_PXL1_CLK_RATE_L1) {
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		} else {
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		}
	} else {
		if (pdss_vote_cmd->dss_mmbuf_rate == 0)
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}

	return;
}

int hisifb_set_mmbuf_clk_rate(struct hisi_fb_data_type *hisifd)
{
	int ret;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd other_fb_clk_rate;
	struct dss_vote_cmd vote_cmd_tmp;

	memset(&vote_cmd_tmp, 0, sizeof(vote_cmd_tmp));
	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));

	get_mmbuf_clk_rate(hisifd);

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = &(hisifd->dss_vote_cmd);
	vote_cmd_tmp.dss_mmbuf_rate = pdss_vote_cmd->dss_mmbuf_rate;

	if ((hisifd->index == AUXILIARY_PANEL_IDX) &&
		hisifd_list[PRIMARY_PANEL_IDX]->panel_power_on)
		return 0;

	hisifb_get_other_fb_clk_vote(hisifd, &other_fb_clk_rate);

	vote_cmd_tmp.dss_mmbuf_rate = vote_cmd_tmp.dss_mmbuf_rate > other_fb_clk_rate.dss_mmbuf_rate ?
		vote_cmd_tmp.dss_mmbuf_rate : other_fb_clk_rate.dss_mmbuf_rate;

	if (!is_clk_voltage_matching(hisifd, vote_cmd_tmp)) {
		HISI_FB_ERR("clk voltage does not match\n");
		vote_cmd_tmp.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}

	ret = clk_set_rate(hisifd->dss_mmbuf_clk, vote_cmd_tmp.dss_mmbuf_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, vote_cmd_tmp.dss_mmbuf_rate, ret);
		return -EINVAL;
	}

	if ((hisifd->index == PRIMARY_PANEL_IDX)
		|| (hisifd->index == EXTERNAL_PANEL_IDX)) {
		HISI_FB_INFO("fb%d mmbuf clk rate[%llu], set[%llu],get[%llu].\n", hisifd->index,
			pdss_vote_cmd->dss_mmbuf_rate, vote_cmd_tmp.dss_mmbuf_rate,
			(uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));
	}

	return 0;
}


int dpe_set_clk_rate(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	dss_vote_cmd_t pdss_vote_cmd_tmp;
	struct dss_vote_cmd other_fb_clk_rate;
	int ret = 0;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = get_dss_vote_cmd(hisifd);
	if (pdss_vote_cmd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}
	memset(&pdss_vote_cmd_tmp, 0, sizeof(pdss_vote_cmd_tmp));
	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));

	pdss_vote_cmd_tmp.dss_pri_clk_rate = pdss_vote_cmd->dss_pri_clk_rate;

	/*dss_pri_clk_rate*/
	hisifb_get_other_fb_clk_vote(hisifd, &other_fb_clk_rate);

	pdss_vote_cmd_tmp.dss_pri_clk_rate = pdss_vote_cmd_tmp.dss_pri_clk_rate > other_fb_clk_rate.dss_pri_clk_rate ?
		pdss_vote_cmd_tmp.dss_pri_clk_rate : other_fb_clk_rate.dss_pri_clk_rate;

	if (!is_clk_voltage_matching(hisifd, pdss_vote_cmd_tmp)) {
		HISI_FB_ERR("clk voltage does not match\n");
		pdss_vote_cmd_tmp.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
	}

	ret = clk_set_rate(hisifd->dss_pri_clk, pdss_vote_cmd_tmp.dss_pri_clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_pri_clk clk_set_rate(%llu) failed, error=%d!\n",
			hisifd->index, pdss_vote_cmd_tmp.dss_pri_clk_rate, ret);
		return -EINVAL;
	}

	/*pxl_clk_rate*/
	ret = dpe_set_pxl_clk_rate(hisifd);
	if (ret < 0) {
		HISI_FB_ERR("fb%d set pxl clk rate failed, error=%d!\n", hisifd->index, ret);
		return -EINVAL;
	}

	if ((hisifd->index == PRIMARY_PANEL_IDX) || (hisifd->index == EXTERNAL_PANEL_IDX)) {
		HISI_FB_INFO("fb%d, dss_pri_clk set %llu get %llu\n", hisifd->index,
			pdss_vote_cmd_tmp.dss_pri_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));
	}

	return ret;
}

int dpe_get_voltage_value(uint32_t dss_voltage_level)
{
	switch (dss_voltage_level) {
	case PERI_VOLTAGE_LEVEL0:
		return PERI_VOLTAGE_LEVEL0_060V;
	case PERI_VOLTAGE_LEVEL1:
		return PERI_VOLTAGE_LEVEL1_065V;
	case PERI_VOLTAGE_LEVEL2:
		return PERI_VOLTAGE_LEVEL2_070V;
	case PERI_VOLTAGE_LEVEL3:
		return PERI_VOLTAGE_LEVEL3_080V;
	default:
		HISI_FB_ERR("not support dss_voltage_level is %d \n", dss_voltage_level);
		return -1;
	}
}

int dpe_get_voltage_level(int votage_value)
{
	switch (votage_value) {
	case PERI_VOLTAGE_LEVEL0_060V:
		return PERI_VOLTAGE_LEVEL0;
	case PERI_VOLTAGE_LEVEL1_065V:
		return PERI_VOLTAGE_LEVEL1;
	case PERI_VOLTAGE_LEVEL2_070V:
		return PERI_VOLTAGE_LEVEL2;
	case PERI_VOLTAGE_LEVEL3_080V:
		return PERI_VOLTAGE_LEVEL3;
	default:
		HISI_FB_ERR("not support votage_value is %d \n", votage_value);
		return PERI_VOLTAGE_LEVEL0;
	}
}

int dpe_set_pixel_clk_rate_on_pll0(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	uint64_t clk_rate;
	struct peri_volt_poll *pvp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Pointer!\n");
		return -EINVAL;
	}

	if (is_dp_panel(hisifd)) {
		hisi_recovery_external_clk_rate(hisifd);

		clk_rate = DEFAULT_DSS_PXL1_CLK_RATE_POWER_OFF;
		ret = clk_set_rate(hisifd->dss_pxl1_clk, clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
		}
		HISI_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl1_clk));

#if defined (CONFIG_HISI_PERIDVFS)
		pvp = peri_volt_poll_get(DEV_LDI1_VOLTAGE_ID, NULL);
		if (pvp == NULL) {
			HISI_FB_ERR("get pvp failed!\n");
			return -EINVAL;
		}

		ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V);
		if (ret) {
			HISI_FB_ERR("set voltage_value=0 failed!\n");
			return -EINVAL;
		}
#endif
	}

	return ret;

}

int dpe_set_common_clk_rate_on_pll0(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint64_t clk_rate;
#if defined (CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
#endif

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Pointer!\n");
		return -EINVAL;
	}

	if (g_fpga_flag == 1) {
		return 0;
	}

	clk_rate = DEFAULT_DSS_MMBUF_CLK_RATE_POWER_OFF;
	ret = clk_set_rate(hisifd->dss_mmbuf_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
		return -EINVAL;
	}
	HISI_FB_INFO("dss_mmbuf_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));

	clk_rate = DEFAULT_DSS_CORE_CLK_RATE_POWER_OFF;
	ret = clk_set_rate(hisifd->dss_pri_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_pri_clk clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
		return -EINVAL;
	}
	HISI_FB_INFO("dss_pri_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));
	hisifb_set_default_pri_clk_rate(hisifd_list[PRIMARY_PANEL_IDX]);
	hisifd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
	hisifd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	hisifd_list[AUXILIARY_PANEL_IDX]->clk_vote_level = PERI_VOLTAGE_LEVEL0;

#if defined (CONFIG_HISI_PERIDVFS)
	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (pvp == NULL) {
		HISI_FB_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V);
	if (ret) {
		HISI_FB_ERR("set voltage_value=0 failed!\n");
		return -EINVAL;
	}
	hisifd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
	hisifd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;

	HISI_FB_INFO("set dss_voltage_level=0!\n");
#endif

	return ret;
}
/*lint +e712 +e838*/

#ifdef CONFIG_DSS_LP_USED
static void lp_first_level_clk_gate_ctrl(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;

	dss_base = hisifd->dss_base; //lint !e838
	outp32(dss_base + GLB_MODULE_CLK_SEL, 0x01800000);
	outp32(dss_base + DSS_DISP_GLB_OFFSET + MODULE_CORE_CLK_SEL, 0x00030000);

	outp32(dss_base + DSS_VBIF0_AIF + AIF_MODULE_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_MODULE_CLK_SEL, 0x00000000);

	//outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MODULE_CLK_SEL, 0x00000000);//
}

static void lp_second_level_clk_gate_ctrl(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;

	dss_base = hisifd->dss_base; //lint !e838
	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL1, 0x00000000);
	outp32(dss_base + DSS_SMMU_OFFSET + SMMU_LP_CTRL, 0x00000001);

	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL1, 0x00000000);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_HI_ACE_OFFSET + DPE_RAMCLK_FUNC, 0x00000000);
	outp32(dss_base + DSS_DPP_OFFSET + DPP_CLK_SEL, 0x00000400);

	/*notice: es, dsc no lp*/
	outp32(dss_base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000007);
	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_DISP_CH1_OFFSET + DISP_CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_DPP1_OFFSET + DPP_CLK_SEL, 0x00000000);

	//outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_CLK_SEL, 0x00000000);//
	//outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_CLK_SEL, 0x00000000);//

	outp32(dss_base + DSS_WB_OFFSET + WB_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_MIF_OFFSET + MIF_CLK_CTL,  0x00000001);
	outp32(dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL1_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL2_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL3_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL4_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL5_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MCTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MOD_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH0_FBCE_CREG_CTRL_GATE,  0x0000000C);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH1_FBCE_CREG_CTRL_GATE,  0x0000000C);

	outp32(dss_base + DSS_OVL0_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL2_OFFSET + OV8_CLK_SEL, 0x00000000);
	//outp32(dss_base + DSS_OVL1_OFFSET + OV8_CLK_SEL, 0x00000000);//
	outp32(dss_base + DSS_OVL3_OFFSET + OV2_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_PIPE_SW_DSI0_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
	//outp32(dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);//
	//outp32(dss_base + DSS_PIPE_SW3_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);//
	outp32(dss_base + DSS_PIPE_SW_WB_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);
}
#endif

/*lint -e838*/
static void no_memory_lp_ctrl(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;

	dss_base = hisifd->dss_base;

	outp32(dss_base + GLB_DSS_PM_CTRL, 0x0401A00F);

	outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);

	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + VPP_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);

	outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);
	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_WCH0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + AFBCE_MEM_CTRL, 0x00000888);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL, 0x88888888);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL_1, 0x00000088);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_LB_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_ARSR2P_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_ARSR2P_COEF_MEM_CTRL, 0x00000088);

	outp32(dss_base + DSS_HDR_OFFSET + HDR_MEM_CTRL, 0x08888888);

	outp32(dss_base + DSS_DISP_CH0_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DPP_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000007);

	outp32(dss_base + DSS_DPP1_GAMA_OFFSET + GAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP1_DEGAMMA_OFFSET + DEGAMA_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP1_GMP_OFFSET + GMP_MEM_CTRL, 0x00000008);
}

static void dss_memory_init(struct hisi_fb_data_type *hisifd)
{
	(void*)hisifd;
}

void dss_inner_clk_common_enable(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	int prev_refcount;
	bool is_in_aod = false;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point\n.");
		return;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		return;
	}

	down(&hisi_fb_dss_inner_clk_sem);

	prev_refcount = dss_inner_clk_refcount++;
	if (!prev_refcount && !fastboot_enable) {
		dss_memory_init(hisifd);

		if (g_fpga_flag == 1) {
			no_memory_lp_ctrl(hisifd);
		} else {
#ifdef CONFIG_DSS_LP_USED
	#if CONFIG_SH_AOD_ENABLE
			is_in_aod = hisi_aod_get_aod_status();
	#endif
			// if in aod, dss work in normal mode.
			if (!is_in_aod) {
				lp_first_level_clk_gate_ctrl(hisifd);
				lp_second_level_clk_gate_ctrl(hisifd);
			} else {
				no_memory_lp_ctrl(hisifd);
			}
#else
			no_memory_lp_ctrl(hisifd);
#endif
		}
	}

	HISI_FB_DEBUG("fb%d, dss_inner_clk_refcount=%d\n",
		hisifd->index, dss_inner_clk_refcount);

	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_common_disable(struct hisi_fb_data_type *hisifd)
{
	int new_refcount;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point\n.");
		return;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		return;
	}

	down(&hisi_fb_dss_inner_clk_sem);
	new_refcount = --dss_inner_clk_refcount;
	if (new_refcount < 0) {
		HISI_FB_ERR("dss new_refcount err");
	}

	if (!new_refcount) {
		;
	}

	HISI_FB_DEBUG("fb%d, dss_inner_clk_refcount=%d\n",
		hisifd->index, dss_inner_clk_refcount);
	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_pdp_enable(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (fastboot_enable) {
		return ;
	}
}

void dss_inner_clk_pdp_disable(struct hisi_fb_data_type *hisifd)
{
	(void*)hisifd;
}

void dss_inner_clk_sdp_enable(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	dss_base = hisifd->dss_base;

#ifdef CONFIG_DSS_LP_USED
	if (is_dp_panel(hisifd)) {
		outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MODULE_CLK_SEL, 0xE);//
		outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_MODULE_CLK_SEL, 0xE);//
		outp32(dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);//
		outp32(dss_base + DSS_PIPE_SW3_OFFSET + PIPE_SW_CLK_SEL, 0x00000000);//
	}
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_CLK_SEL, 0x00000000);//
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_CLK_SEL, 0x00000000);//
	outp32(dss_base + DSS_OVL1_OFFSET + OV8_CLK_SEL, 0x00000000);//

#else
	if (is_dp_panel(hisifd)) {
		outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_MEM_CTRL, 0x00000008);//
		outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_MEM_CTRL, 0x00000008);//
	}
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_MEM_CTRL, 0x00000008);//
	outp32(dss_base + DSS_DISP_CH2_OFFSET + DISP_CH_DITHER_MEM_CTRL, 0x00000008);//
#endif
}

void dss_inner_clk_sdp_disable(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint64_t dss_mmbuf_rate;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!");
		return ;
	}

	hisifd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;

	if (hisifd_list[PRIMARY_PANEL_IDX]) {
		dss_mmbuf_rate = hisifd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate;
		ret = clk_set_rate(hisifd->dss_mmbuf_clk, dss_mmbuf_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, dss_mmbuf_rate, ret);
			return ;
		}
	}

	return;
}

void init_dpp(struct hisi_fb_data_type *hisifd)
{
	char __iomem *disp_ch_base = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!");
		return ;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		disp_ch_base = hisifd->dss_base + DSS_DISP_CH0_OFFSET;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		disp_ch_base = hisifd->dss_base + DSS_DISP_CH2_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!\n", hisifd->index);
		return ;
	}
	HISI_FB_DEBUG("fb%d + \n", hisifd->index);

	outp32(disp_ch_base + IMG_SIZE_BEF_SR, (DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
	outp32(disp_ch_base + IMG_SIZE_AFT_SR, (DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
	outp32(disp_ch_base + IMG_SIZE_AFT_IFBCSW, (DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
}

static void init_dsc(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dsc_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dsc_panel_info *dsc = NULL;

	uint32_t dsc_en = 0;
	uint32_t pic_width = 0;
	uint32_t pic_height = 0;
	uint32_t chunk_size = 0;
	uint32_t groups_per_line = 0;
	uint32_t rbs_min = 0;
	uint32_t hrd_delay = 0;
	uint32_t target_bpp_x16 =0;
	uint32_t num_extra_mux_bits = 0;
	uint32_t slice_bits = 0;
	uint32_t final_offset = 0;
	uint32_t final_scale = 0;
	uint32_t nfl_bpg_offset = 0;
	uint32_t groups_total = 0;
	uint32_t slice_bpg_offset = 0;
	uint32_t scale_increment_interval = 0;
	uint32_t initial_scale_value = 0;
	uint32_t scale_decrement_interval = 0;
	uint32_t adjustment_bits =0;
	uint32_t adj_bits_per_grp = 0;
	uint32_t bits_per_grp = 0;
	uint32_t slices_per_line = 0;
	uint32_t pic_line_grp_num = 0;
	uint32_t dsc_insert_byte_num = 0;
	uint32_t dual_dsc_en = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->spr_dsc_mode != SPR_DSC_MODE_NONE) {
		HISI_FB_INFO("spr enable, dsc1.1 bypass\n");
		return;
	}

	dsc = &(pinfo->vesa_dsc);
	dsc_base = hisifd->dss_base + DSS_DSC_OFFSET;


	if ((pinfo->ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE)) {
		// dual_dsc_en = 0, dsc_if_bypass = 1, reset_ich_per_line = 0
		dsc_en = 0x5;
		pic_width = DSS_WIDTH(pinfo->xres);
	} else {
		// dual_dsc_en = 1, dsc_if_bypass = default value 1, reset_ich_per_line = 1//
		dsc_en = 0xF;
		dual_dsc_en = 1;
		pic_width = DSS_WIDTH(pinfo->xres);
	}

	pic_height =  DSS_HEIGHT(pinfo->yres);
	chunk_size = ROUND1((dsc->slice_width + 1) * dsc->bits_per_pixel, 8);

	groups_per_line = (dsc->slice_width + 3) / 3;
	rbs_min = dsc->rc_model_size - dsc->initial_offset + dsc->initial_xmit_delay * dsc->bits_per_pixel +
		groups_per_line * dsc->first_line_bpg_offset;
	hrd_delay = ROUND1(rbs_min, dsc->bits_per_pixel);

	target_bpp_x16 = dsc->bits_per_pixel * 16;
	slice_bits = 8 * chunk_size * (dsc->slice_height + 1);

	num_extra_mux_bits = 3 * (dsc->mux_word_size + (4 * dsc->bits_per_component + 4) - 2);
	while ((num_extra_mux_bits > 0) && ((slice_bits - num_extra_mux_bits) % dsc->mux_word_size))
		num_extra_mux_bits--;

	final_offset =	dsc->rc_model_size - ((dsc->initial_xmit_delay * target_bpp_x16 + 8) >> 4) + num_extra_mux_bits; //4336(0x10f0)
	final_scale = 8 * dsc->rc_model_size / (dsc->rc_model_size - final_offset);
	nfl_bpg_offset = ROUND1(dsc->first_line_bpg_offset << OFFSET_FRACTIONAL_BITS, dsc->slice_height); //793(0x319)
	groups_total = groups_per_line * (dsc->slice_height + 1);
	slice_bpg_offset = ROUND1((1 << OFFSET_FRACTIONAL_BITS) *
		(dsc->rc_model_size - dsc->initial_offset + num_extra_mux_bits), groups_total); // 611(0x263)
	scale_increment_interval = (1 << OFFSET_FRACTIONAL_BITS) * final_offset /
		((final_scale - 9) * (nfl_bpg_offset + slice_bpg_offset)); // 903(0x387)

	initial_scale_value = 8 * dsc->rc_model_size / (dsc->rc_model_size - dsc->initial_offset);
	if (groups_per_line < initial_scale_value - 8)	{
		initial_scale_value = groups_per_line + 8;
	}

	if (initial_scale_value > 8) {
		scale_decrement_interval = groups_per_line / (initial_scale_value - 8);
	} else {
		scale_decrement_interval = 4095;
	}

	adjustment_bits = (8 - (dsc->bits_per_pixel * (dsc->slice_width + 1)) % 8) % 8;
	adj_bits_per_grp = dsc->bits_per_pixel * 3 - 3;
	bits_per_grp = dsc->bits_per_pixel * 3;
	slices_per_line = ((pic_width + 1) / (dual_dsc_en + 1)) / (dsc->slice_width + 1) - 1;
	//slices_per_line = (pic_width > dsc->slice_width) ? 1 : 0;
	pic_line_grp_num = ((dsc->slice_width + 3) / 3) * (slices_per_line + 1) - 1;

	set_reg(dsc_base + DSC_REG_DEFAULT, 0x1, 1, 0);

	// dsc_en
	set_reg(dsc_base + DSC_EN, dsc_en, 4, 0);
	if (chunk_size % 3) {
		dsc_insert_byte_num = (3 - chunk_size % 3) & 0x3;
	}

	if (dsc_insert_byte_num != 0) {
		HISI_FB_INFO("dsc_insert_byte_num = %d\n", dsc_insert_byte_num);
		set_reg(dsc_base + DSC_CTRL, dsc->bits_per_component
			| (dsc->linebuf_depth << 4) | (dsc_insert_byte_num << 8) | (dsc->block_pred_enable << 10) |
			(0x1 << 11) | (dsc->bits_per_pixel << 16), 26, 0);
	} else {
		// bits_per_component, convert_rgb, bits_per_pixel
		set_reg(dsc_base + DSC_CTRL, dsc->bits_per_component | (dsc->linebuf_depth << 4) | (dsc->block_pred_enable << 10) |
			(0x1 << 11) | (dsc->bits_per_pixel << 16), 26, 0);
	}

	// pic_width, pic_height
	set_reg(dsc_base + DSC_PIC_SIZE, (pic_width << 16) | pic_height, 32, 0);

	// slice_width, slice_height
	set_reg(dsc_base + DSC_SLICE_SIZE, (dsc->slice_width << 16) | dsc->slice_height, 32, 0);

	// chunk_size
	set_reg(dsc_base + DSC_CHUNK_SIZE, chunk_size, 16, 0);

	// initial_xmit_delay, initial_dec_delay = hrd_delay -initial_xmit_delay
	set_reg(dsc_base + DSC_INITIAL_DELAY, dsc->initial_xmit_delay |
		((hrd_delay - dsc->initial_xmit_delay) << 16), 32, 0);

	// initial_scale_value, scale_increment_interval
	set_reg(dsc_base + DSC_RC_PARAM0, initial_scale_value | (scale_increment_interval << 16), 32, 0);

	// scale_decrement_interval, first_line_bpg_offset
	set_reg(dsc_base + DSC_RC_PARAM1, (dsc->first_line_bpg_offset << 16) | scale_decrement_interval, 21, 0);

	// nfl_bpg_offset, slice_bpg_offset
	set_reg(dsc_base + DSC_RC_PARAM2, nfl_bpg_offset | (slice_bpg_offset << 16), 32, 0);

	//DSC_RC_PARAM3
	set_reg(dsc_base + DSC_RC_PARAM3,
		((final_offset << 16) | dsc->initial_offset), 32, 0);

	//DSC_FLATNESS_QP_TH
	set_reg(dsc_base + DSC_FLATNESS_QP_TH,
		((dsc->flatness_max_qp << 16) | (dsc->flatness_min_qp << 0)), 24, 0);

	//DSC_RC_PARAM4
	set_reg(dsc_base + DSC_RC_PARAM4,
		((dsc->rc_edge_factor << 20) | (dsc->rc_model_size << 0)), 24, 0);
	//DSC_RC_PARAM5
	set_reg(dsc_base + DSC_RC_PARAM5,
		((dsc->rc_tgt_offset_lo << 20) |(dsc->rc_tgt_offset_hi << 16) |
		(dsc->rc_quant_incr_limit1 << 8) |(dsc->rc_quant_incr_limit0 << 0)), 24, 0);

	//DSC_RC_BUF_THRESH
	set_reg(dsc_base + DSC_RC_BUF_THRESH0,
		((dsc->rc_buf_thresh0 << 24) | (dsc->rc_buf_thresh1 << 16) |
		(dsc->rc_buf_thresh2 << 8) | (dsc->rc_buf_thresh3 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH1,
		((dsc->rc_buf_thresh4 << 24) | (dsc->rc_buf_thresh5 << 16) |
		(dsc->rc_buf_thresh6 << 8) | (dsc->rc_buf_thresh7 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH2,
		((dsc->rc_buf_thresh8 << 24) | (dsc->rc_buf_thresh9 << 16) |
		(dsc->rc_buf_thresh10 << 8) | (dsc->rc_buf_thresh11 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH3,
		((dsc->rc_buf_thresh12 << 24) | (dsc->rc_buf_thresh13 << 16)), 32, 0);

	//DSC_RC_RANGE_PARAM
	set_reg(dsc_base + DSC_RC_RANGE_PARAM0,
		((dsc->range_min_qp0 << 27) | (dsc->range_max_qp0 << 22) |
		(dsc->range_bpg_offset0 << 16) | (dsc->range_min_qp1 << 11) |
		(dsc->range_max_qp1 << 6) | (dsc->range_bpg_offset1 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM1,
		((dsc->range_min_qp2 << 27) | (dsc->range_max_qp2 << 22) |
		(dsc->range_bpg_offset2 << 16) | (dsc->range_min_qp3 << 11) |
		(dsc->range_max_qp3 << 6) | (dsc->range_bpg_offset3 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM2,
		((dsc->range_min_qp4 << 27) | (dsc->range_max_qp4 << 22) |
		(dsc->range_bpg_offset4 << 16) | (dsc->range_min_qp5 << 11) |
		(dsc->range_max_qp5 << 6) | (dsc->range_bpg_offset5 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM3,
		((dsc->range_min_qp6 << 27) | (dsc->range_max_qp6 << 22) |
		(dsc->range_bpg_offset6 << 16) | (dsc->range_min_qp7 << 11) |
		(dsc->range_max_qp7 << 6) | (dsc->range_bpg_offset7 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM4,
		((dsc->range_min_qp8 << 27) | (dsc->range_max_qp8 << 22) |
		(dsc->range_bpg_offset8 << 16) | (dsc->range_min_qp9 << 11) |
		(dsc->range_max_qp9 << 6) | (dsc->range_bpg_offset9 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM5,
		((dsc->range_min_qp10 << 27) | (dsc->range_max_qp10 << 22) |
		(dsc->range_bpg_offset10 << 16) | (dsc->range_min_qp11 << 11) |
		(dsc->range_max_qp11 << 6) | (dsc->range_bpg_offset11 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM6,
		((dsc->range_min_qp12 << 27) | (dsc->range_max_qp12 << 22) |
		(dsc->range_bpg_offset12 << 16) | (dsc->range_min_qp13 << 11) |
		(dsc->range_max_qp13 << 6) | (dsc->range_bpg_offset13 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM7,
		((dsc->range_min_qp14 << 27) | (dsc->range_max_qp14 << 22) |
		(dsc->range_bpg_offset14 << 16)), 32, 0);

	// adjustment_bits
	set_reg(dsc_base + DSC_ADJUSTMENT_BITS, adjustment_bits, 4, 0);

	// bits_per_grp, adj_bits_per_grp
	set_reg(dsc_base + DSC_BITS_PER_GRP, bits_per_grp | (adj_bits_per_grp << 8), 14, 0);

	//slices_per_line, pic_line_grp_num
	set_reg(dsc_base + DSC_MULTI_SLICE_CTL, slices_per_line |
		(pic_line_grp_num << 16), 32, 0);

	set_reg(dsc_base + DSC_CLK_SEL, 0x7, 32, 0);//no lp
	set_reg(dsc_base + DSC_CLK_EN, 0x7, 32, 0);
	set_reg(dsc_base + DSC_MEM_CTRL, 0x0, 32, 0);
	set_reg(dsc_base + DSC_ST_DATAIN, 0x0, 28, 0);
	set_reg(dsc_base + DSC_ST_DATAOUT, 0x0, 16, 0);
	set_reg(dsc_base + DSC0_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC1_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC0_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC1_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC_ST_ITFC, 0x0, 10, 0);
	set_reg(dsc_base + DSC_RD_SHADOW_SEL, 0x1, 1, 0);
	set_reg(dsc_base + DSC_REG_DEFAULT, 0x0, 1, 0);
}

void init_ifbc(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo->ifbc_type >= IFBC_TYPE_MAX) {
		HISI_FB_ERR("ifbc_type is larger than IFBC_TYPE_MAX.\n");
		return;
	}

	if (pinfo->ifbc_type == IFBC_TYPE_NONE)
		return ;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_IFBC))
		return;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (is_ifbc_vesa_panel(hisifd)) {
			init_dsc(hisifd);
		}
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}
}
/*lint +e838*/
/*lint -e438 -e550 -e838*/
void init_post_scf(struct hisi_fb_data_type *hisifd)
{
	char __iomem *scf_lut_base;
	char __iomem *scf_base;
	int ihright;
	int ihright1;
	int ivbottom;

	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	pinfo = &(hisifd->panel_info);

	scf_lut_base = hisifd->dss_base + DSS_POST_SCF_LUT_OFFSET;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF)) {
		return;
	}

	pinfo->post_scf_support = 1;
	hisi_dss_arsr_post_coef_on(hisifd);

	scf_base = hisifd->dss_base + DSS_POST_SCF_OFFSET;

	outp32(scf_base + ARSR_POST_SKIN_THRES_Y, 0x2585312C);
	outp32(scf_base + ARSR_POST_SKIN_THRES_U, 0x1C40A014);
	outp32(scf_base + ARSR_POST_SKIN_THRES_V, 0x2440C018);
	outp32(scf_base + ARSR_POST_SKIN_CFG0, 0x00018200);
	outp32(scf_base + ARSR_POST_SKIN_CFG1, 0x00000333);
	outp32(scf_base + ARSR_POST_SKIN_CFG2, 0x000002AA);
	outp32(scf_base + ARSR_POST_SHOOT_CFG1, 0x00140155);
	outp32(scf_base + ARSR_POST_SHOOT_CFG2, 0x001007B0);
	outp32(scf_base + ARSR_POST_SHOOT_CFG3, 0x00000014);
	outp32(scf_base + ARSR_POST_SHARP_CFG3, 0x00A00060);
	outp32(scf_base + ARSR_POST_SHARP_CFG4, 0x00600020);
	outp32(scf_base + ARSR_POST_SHARP_CFG5, 0);
	outp32(scf_base + ARSR_POST_SHARP_CFG6, 0x00040008);
	outp32(scf_base + ARSR_POST_SHARP_CFG7, 0x0000060A);
	outp32(scf_base + ARSR_POST_SHARP_CFG8, 0x00A00010);
	outp32(scf_base + ARSR_POST_SHARP_LEVEL, 0x20002);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_LOW, 0x3C0078);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_MID, 0x6400C8);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_HIGH, 0x5000A0);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_MF, 0x280);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_MF, 0x1400);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPH_HF, 0x140);
	outp32(scf_base + ARSR_POST_SHARP_GAINCTRLSLOPL_HF, 0xA00);
	outp32(scf_base + ARSR_POST_SHARP_MF_LMT, 0x40);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_MF, 0x12C012C);
	outp32(scf_base + ARSR_POST_SHARP_MF_B, 0);
	outp32(scf_base + ARSR_POST_SHARP_HF_LMT, 0x80);
	outp32(scf_base + ARSR_POST_SHARP_GAIN_HF, 0x104012C);
	outp32(scf_base + ARSR_POST_SHARP_HF_B, 0x1400);
	outp32(scf_base + ARSR_POST_SHARP_LF_CTRL, 0x100010);
	outp32(scf_base + ARSR_POST_SHARP_LF_VAR, 0x1800080);
	outp32(scf_base + ARSR_POST_SHARP_LF_CTRL_SLOP, 0);
	outp32(scf_base + ARSR_POST_SHARP_HF_SELECT, 0);
	outp32(scf_base + ARSR_POST_SHARP_CFG2_H, 0x10000C0);
	outp32(scf_base + ARSR_POST_SHARP_CFG2_L, 0x200010);
	outp32(scf_base + ARSR_POST_TEXTURE_ANALYSIS, 0x500040);
	outp32(scf_base + ARSR_POST_INTPLSHOOTCTRL, 0x8);

	ihright1 = ((int)pinfo->xres - 1) * ARSR1P_INC_FACTOR;
	ihright = ihright1 + 2 * ARSR1P_INC_FACTOR;
	if (ihright >= ((int)pinfo->xres) * ARSR1P_INC_FACTOR) {
		ihright = ((int)pinfo->xres) * ARSR1P_INC_FACTOR - 1;
	}

	ivbottom = ((int)pinfo->yres - 1) * ARSR1P_INC_FACTOR;
	if (ivbottom >= ((int)pinfo->yres) * ARSR1P_INC_FACTOR) {
		ivbottom = ((int)pinfo->yres) * ARSR1P_INC_FACTOR - 1;
	}

	outp32(scf_base + ARSR_POST_IHLEFT, 0x0);
	outp32(scf_base + ARSR_POST_IHRIGHT, ihright);
	outp32(scf_base + ARSR_POST_IHLEFT1, 0x0);
	outp32(scf_base + ARSR_POST_IHRIGHT1, ihright1);
	outp32(scf_base + ARSR_POST_IVTOP, 0x0);
	outp32(scf_base + ARSR_POST_IVBOTTOM, ivbottom);
	outp32(scf_base + ARSR_POST_IVBOTTOM1, ivbottom);
	outp32(scf_base + ARSR_POST_IHINC, ARSR1P_INC_FACTOR);
	outp32(scf_base + ARSR_POST_IVINC, ARSR1P_INC_FACTOR);

	outp32(scf_base + ARSR_POST_MODE, 0x1);

	return;
}
/*lint -e573 -e647 -e712*/
void init_dbuf(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dbuf_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int sram_valid_num = 0;

	uint32_t thd_rqos_in = 0;
	uint32_t thd_rqos_out = 0;
	uint32_t thd_wqos_in = 0;
	uint32_t thd_wqos_out = 0;
	uint32_t thd_cg_in = 0;
	uint32_t thd_cg_out = 0;
	uint32_t thd_wr_wait = 0;
	uint32_t thd_cg_hold = 0;
	uint32_t thd_flux_req_befdfs_in = 0;
	uint32_t thd_flux_req_befdfs_out = 0;
	uint32_t thd_flux_req_aftdfs_in = 0;
	uint32_t thd_flux_req_aftdfs_out = 0;
	uint32_t thd_dfs_ok = 0;
	uint32_t dfs_ok_mask = 0;
	uint32_t thd_flux_req_sw_en = 1;
	uint32_t htotal;
	uint32_t vtotal;
	uint32_t fps;
	uint32_t thd_rqos_idle;

	int depth = 0;
	int dfs_ram = 0;
	bool dbuf_burst = false;
	uint32_t rectwidth;
	uint32_t ifbc_type;
	uint32_t mipi_idx;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_DBUF)) {
		HISI_FB_ERR("HISI_DSS_SUPPORT_DPP_MODULE_BIT not support\n");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dbuf_base = hisifd->dss_base + DSS_DBUF0_OFFSET;
		sram_valid_num = 0;
		dfs_ram = 0xFFF0;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		dbuf_base = hisifd->dss_base + DSS_DBUF1_OFFSET;
		sram_valid_num = 2;
		dfs_ram = 0x111F;
	} else {
		HISI_FB_INFO("fb%d, not support!", hisifd->index);
		return;
	}

	if (is_dp_panel(hisifd)) {
		rectwidth = pinfo->xres;
	} else {
		mipi_idx = 0;
		ifbc_type = pinfo->ifbc_type;

		if ((hisifd->panel_info.mode_switch_to == MODE_10BIT_VIDEO_3X)
			&& (hisifd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_DUAL)) {
			/* data size of 10bit vesa3x is larger than 10bit vesa3.75x */
			rectwidth = pinfo->xres * 30 / 24 / g_mipi_ifbc_division[mipi_idx][ifbc_type].xres_div;
		} else {
			rectwidth = pinfo->xres / g_mipi_ifbc_division[mipi_idx][ifbc_type].xres_div;
		}

		rectwidth = get_hsize_after_spr_dsc(hisifd, rectwidth);
	}

	depth = (sram_valid_num + 1) * DBUF_DEPTH;

	if (pinfo->pxl_clk_rate_div <= 0) {
		pinfo->pxl_clk_rate_div = 1;
	}

	if (dbuf_burst) {
		if (sram_valid_num < 1) {
			HISI_FB_ERR("dbuf burst mode, sram_valid_num must >= 1\n");
		}
		thd_cg_in = depth - 1;
		htotal = pinfo->xres
			+ (pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch) * pinfo->pxl_clk_rate_div;
		vtotal = pinfo->yres + pinfo->ldi.v_pulse_width + pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch;
		fps = pinfo->pxl_clk_rate / htotal / vtotal;//need use lane_byte_clk
		thd_cg_out = thd_cg_in - 150 * (rectwidth / ((1000000 / fps / pinfo->yres) * 4));
	} else {
		thd_cg_in = depth - 1;
		thd_cg_out = thd_cg_in * 95 / 100;
	}
	thd_rqos_in = depth * 80 / 100;
	thd_rqos_out = depth * 90 / 100;
	thd_rqos_idle = depth;
	thd_flux_req_befdfs_in = depth * 70 / 100;
	thd_flux_req_befdfs_out = depth * 80 / 100;
	thd_flux_req_aftdfs_in = thd_flux_req_befdfs_in;
	thd_flux_req_aftdfs_out = thd_flux_req_befdfs_out;
	thd_dfs_ok = thd_flux_req_befdfs_in;
	dfs_ok_mask = 0;

	HISI_FB_INFO("sram_valid_num=%d,\n"
		"thd_rqos_in=0x%x\n"
		"thd_rqos_out=0x%x\n"
		"thd_cg_in=0x%x\n"
		"thd_cg_out=0x%x\n"
		"thd_flux_req_befdfs_in=0x%x\n"
		"thd_flux_req_befdfs_out=0x%x\n"
		"thd_flux_req_aftdfs_in=0x%x\n"
		"thd_flux_req_aftdfs_out=0x%x\n"
		"thd_dfs_ok=0x%x\n",
		sram_valid_num,
		thd_rqos_in,
		thd_rqos_out,
		thd_cg_in,
		thd_cg_out,
		thd_flux_req_befdfs_in,
		thd_flux_req_befdfs_out,
		thd_flux_req_aftdfs_in,
		thd_flux_req_aftdfs_out,
		thd_dfs_ok);

	outp32(dbuf_base + DBUF_FRM_SIZE, rectwidth * pinfo->yres);
	outp32(dbuf_base + DBUF_FRM_HSIZE, DSS_WIDTH(rectwidth));
	outp32(dbuf_base + DBUF_SRAM_VALID_NUM, sram_valid_num);

	outp32(dbuf_base + DBUF_THD_RQOS, (thd_rqos_out << 16) | thd_rqos_in);
	outp32(dbuf_base + DBUF_THD_WQOS, (thd_wqos_out << 16) | thd_wqos_in);
	outp32(dbuf_base + DBUF_THD_CG, (thd_cg_out << 16) | thd_cg_in);
	outp32(dbuf_base + DBUF_THD_OTHER, (thd_cg_hold << 16) | thd_wr_wait);
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_BEF, (thd_flux_req_befdfs_out << 16) | thd_flux_req_befdfs_in);
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_AFT, (thd_flux_req_aftdfs_out << 16) | thd_flux_req_aftdfs_in);
	outp32(dbuf_base + DBUF_THD_DFS_OK, thd_dfs_ok);
	outp32(dbuf_base + DBUF_FLUX_REQ_CTRL, (dfs_ok_mask << 1) | thd_flux_req_sw_en);

	outp32(dbuf_base + DBUF_DFS_LP_CTRL, 0x1);

	outp32(dbuf_base + DBUF_DFS_RAM_MANAGE, dfs_ram);

	if (dbuf_burst) {
		outp32(dbuf_base + DBUF_THD_RQOS_IDLE, (1 << 16) | thd_rqos_idle);
	} else {
		outp32(dbuf_base + DBUF_THD_RQOS_IDLE, thd_rqos_idle);
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x0, 1, 0);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x0, 1, 1);
	}
}
/*lint +e573 +e647 +e712*/
/*lint +e438 +e550 +e838*/
void deinit_dbuf(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x1, 1, 0);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DSS_DFS_OK_MASK, 0x1, 1, 1);
	}
}
/*lint -e438 -e550*/
static void init_dual_mipi_ctrl(struct hisi_fb_data_type *hisifd)
{
	dss_rect_t rect = { 0,0,0,0 };
	uint32_t dmipi_hsize = 0;

	rect.x = 0;
	rect.y = 0;
	rect.w = hisifd->panel_info.xres;//lint !e713 !e838
	rect.h = hisifd->panel_info.yres;//lint !e713

	mipi_ifbc_get_rect(hisifd, &rect);

	rect.w = get_hsize_after_spr_dsc(hisifd, rect.w);
	dmipi_hsize = (((uint32_t)rect.w * 2 - 1) << 16) | ((uint32_t)rect.w - 1); //lint !e838

	set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_HSIZE, dmipi_hsize, 32, 0);
	set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DUAL_MIPI_SWAP, 0x0, 1, 0);
	return;
}
/*lint +e438 +e550*/
static void init_pipe_sw(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI0_OFFSET + PIPE_SW_SIG_CTRL, 0x1, 8, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI0_OFFSET + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI0_OFFSET + PIPE_SW_DAT_CTRL, 0x1, 8, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI0_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
		if (is_dual_mipi_panel(hisifd)) {
			set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_SIG_CTRL, 0x1, 8, 0);
			set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_SIG_EN, 0x0, 1, 0);
			set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_DAT_CTRL, 0x1, 8, 0);
			set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
			init_dual_mipi_ctrl(hisifd);
		}
	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && is_dp_panel(hisifd)) {
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_SIG_CTRL, 0x2, 8, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DP_OFFSET + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DP_OFFSET + PIPE_SW_DAT_CTRL, 0x2, 8, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DP_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);

	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && is_mipi_panel(hisifd)) {
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_SIG_CTRL, 0x2, 8, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_SIG_EN, 0x1, 1, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + PIPE_SW_DAT_CTRL, 0x2, 8, 0);
		set_reg(hisifd->dss_base + DSS_PIPE_SW_DSI1_OFFSET + SW_POS_CTRL_DAT_EN, 0x1, 1, 0);
	}
	return;
}

static void init_dpp_ifbc_sw(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DPPSW_SIG_CTRL, 0x20101, 24, 0);
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DPPSW_DAT_CTRL, 0x20101, 24, 0);
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + IFBCSW_SIG_CTRL, 0x403, 16, 0);
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + IFBCSW_DAT_CTRL, 0x403, 16, 0);
		set_reg(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT, 0x0, 1, 0);
	}
	return;
}

void hisifb_display_post_process_chn_init(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null\n");
	}
	init_pipe_sw(hisifd);
	init_dpp_ifbc_sw(hisifd);
}

/*lint -e712, -e838*/
void init_ldi(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *ldi_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_rect_t rect;
	uint32_t te_source;
	int i = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	hisifb_display_post_process_chn_init(hisifd);

	if (is_dp_panel(hisifd)) {
		for(i = 0; i < hisifd->dp.streams; i++) {
			if (i == 0) {
				ldi_base = hisifd->dss_base + DSS_LDI_DP_OFFSET;
			} else {
				ldi_base = hisifd->dss_base + DSS_LDI_DP1_OFFSET;
			}

			HISI_FB_INFO("fb%d, fastboot_enable = %d!", hisifd->index, fastboot_enable);
			rect.x = 0;
			rect.y = 0;
			rect.w = hisifd->panel_info.xres;//lint !e713
			rect.h = hisifd->panel_info.yres;//lint !e713
			mipi_ifbc_get_rect(hisifd, &rect);

			if (is_mipi_video_panel(hisifd)) {
				outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
					pinfo->ldi.h_front_porch | ((pinfo->ldi.h_back_porch + DSS_WIDTH(pinfo->ldi.h_pulse_width)) << 16));
				outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
				outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(rect.w));
			} else {
				outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
					pinfo->ldi.h_front_porch | (pinfo->ldi.h_back_porch << 16));
				outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(pinfo->ldi.h_pulse_width));
				outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(rect.w));
			}
			outp32(ldi_base + LDI_VRT_CTRL0,
			pinfo->ldi.v_front_porch | (pinfo->ldi.v_back_porch << 16));
			outp32(ldi_base + LDI_VRT_CTRL1, DSS_HEIGHT(pinfo->ldi.v_pulse_width));
			outp32(ldi_base + LDI_VRT_CTRL2, DSS_HEIGHT(rect.h));

			outp32(ldi_base + LDI_PLR_CTRL,
				pinfo->ldi.vsync_plr | (pinfo->ldi.hsync_plr << 1) |
				(pinfo->ldi.pixelclk_plr << 2) | (pinfo->ldi.data_en_plr << 3));

			// bpp
			set_reg(ldi_base + LDI_CTRL, pinfo->bpp, 2, 3);
			// bgr
			set_reg(ldi_base + LDI_CTRL, pinfo->bgr_fmt, 1, 13);

			// for ddr pmqos
			outp32(ldi_base + LDI_VINACT_MSK_LEN,
			pinfo->ldi.v_front_porch);

			// cmd event sel
			outp32(ldi_base + LDI_CMD_EVENT_SEL, 0x1);

			// double pixel mode	1772
			outp32(ldi_base + LDI_MP_MODE, 0x1);

			// for 1Hz LCD and mipi command LCD
			if (is_mipi_cmd_panel(hisifd)) {
				set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x1, 2, 0);

				// DSI_TE_CTRL
				// te_source = 0, select te_pin
				// te_source = 1, select te_triger
				te_source = 0;

				// dsi_te_hard_en
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 0);
				// dsi_te0_pin_p , dsi_te1_pin_p
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 2, 1);
				// dsi_te_hard_sel
				set_reg(ldi_base + LDI_DSI_TE_CTRL, te_source, 1, 3);
				// select TE0 PIN
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x01, 2, 6);
				// dsi_te_mask_en
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 8);
				// dsi_te_mask_dis
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 9);
				// dsi_te_mask_und
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 13);
				// dsi_te_pin_en
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 17);

				set_reg(ldi_base + LDI_DSI_TE_HS_NUM, 0x0, 32, 0);
				set_reg(ldi_base + LDI_DSI_TE_HS_WD, 0x24024, 32, 0);

				// dsi_te0_vs_wd = lcd_te_width / T_pxl_clk, experience lcd_te_width = 2us
				if (pinfo->pxl_clk_rate_div == 0) {
					HISI_FB_ERR("pxl_clk_rate_div is NULL, not support !\n");
					pinfo->pxl_clk_rate_div = 1;
				}
				set_reg(ldi_base + LDI_DSI_TE_VS_WD,
					(0x3FC << 12) | (2 * pinfo->pxl_clk_rate / pinfo->pxl_clk_rate_div / 1000000), 32, 0);
			} else {
				// dsi_te_hard_en
				set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 0);
				set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x2, 2, 0);
			}

			// normal
			set_reg(ldi_base + LDI_WORK_MODE, 0x1, 1, 0);

			if (is_mipi_cmd_panel(hisifd))
				set_reg(ldi_base + LDI_FRM_MSK,
				(hisifd->frame_update_flag == 1) ? 0x0 : 0x1, 1, 0);

			if (hisifd->index == EXTERNAL_PANEL_IDX && (is_mipi_panel(hisifd)))
				set_reg(ldi_base + LDI_DP_DSI_SEL, 0x1, 1, 0);

			// mst mode
			if (hisifd->dp.mst)
				set_reg(ldi_base + LDI_MST_CONTROL, 0x1, 1, 0);

			if (i == 1)
				set_reg(ldi_base + LDI_WORK_MODE, 0x0, 1, 0);
		}

	} else {
		HISI_FB_INFO("fb%d is not dp panel\n", hisifd->index);
		return;
	}
	//init_wb_pkg_en(hisifd, fastboot_enable);
	//pipe_sw_ctrl(hisifd);

	if (is_hisync_mode(hisifd)) {
		hisifb_hisync_disp_sync_enable(hisifd);
	}

	// ldi disable
	if (!fastboot_enable) {
		disable_ldi(hisifd);
	}

	HISI_FB_DEBUG("-.!\n");
}
/*lint +e712, +e838*/
/*lint -e438, -e550*/
void deinit_ldi(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	disable_ldi(hisifd);
}
/*lint +e438, +e550*/
void enable_ldi(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (is_dp_panel(hisifd)) {
		ldi_base = hisifd->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);

		ldi_base = hisifd->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
	} else {
		if ((hisifd->index == PRIMARY_PANEL_IDX) && is_dual_mipi_panel(hisifd)) {
			set_reg(hisifd->mipi_dsi0_base + MIPI_LDI_CTRL, 0x1, 1, 5);
			return;
		}

		if (hisifd->index == PRIMARY_PANEL_IDX) {
			ldi_base = hisifd->mipi_dsi0_base;
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			ldi_base = hisifd->mipi_dsi1_base;
		} else {
			return;
		}
		set_reg(ldi_base + MIPI_LDI_CTRL, 0x1, 1, 0);
	}
}

void disable_ldi(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (is_dp_panel(hisifd)) {
		ldi_base = hisifd->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);

		ldi_base = hisifd->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
	} else {
		if ((hisifd->index == PRIMARY_PANEL_IDX) && is_dual_mipi_panel(hisifd)) {
			set_reg(hisifd->mipi_dsi0_base + MIPI_LDI_CTRL, 0x0, 1, 5);
			return;
		}

		if (hisifd->index == PRIMARY_PANEL_IDX) {
			ldi_base = hisifd->mipi_dsi0_base;
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			ldi_base = hisifd->mipi_dsi1_base;
		} else {
			return;
		}
		set_reg(ldi_base + MIPI_LDI_CTRL, 0x0, 1, 0);
	}
}

void ldi_frame_update(struct hisi_fb_data_type *hisifd, bool update)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (is_mipi_cmd_panel(hisifd)) {
			set_reg(hisifd->mipi_dsi0_base + MIPI_LDI_FRM_MSK, (update ? 0x0 : 0x1), 1, 0);
			if (is_dual_mipi_panel(hisifd)) {
				set_reg(hisifd->mipi_dsi1_base + MIPI_LDI_FRM_MSK, (update ? 0x0 : 0x1), 1, 0);
			}
			if (update) {
				enable_ldi(hisifd);
			}
		}
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
	}
}

void single_frame_update(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (is_dp_panel(hisifd)) {
		ldi_base = hisifd->dss_base + DSS_LDI_DP_OFFSET;
		set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);

		ldi_base = hisifd->dss_base + DSS_LDI_DP1_OFFSET;
		set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);
	} else if (is_mipi_cmd_panel(hisifd)) {
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			set_reg(hisifd->mipi_dsi0_base + MIPI_LDI_FRM_MSK_UP, 0x1, 1, 0);
			if (is_dual_mipi_panel(hisifd))
				set_reg(hisifd->mipi_dsi1_base + MIPI_LDI_FRM_MSK_UP, 0x1, 1, 0);
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			set_reg(hisifd->mipi_dsi1_base + MIPI_LDI_FRM_MSK_UP, 0x1, 1, 0);
		}
	}

	enable_ldi(hisifd);
	return;
}
/*lint -e838*/
void dpe_interrupt_clear(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;
	uint32_t clear;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_PDP_INTS, clear);
		outp32(hisifd->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INTS, clear);
		if (is_dual_mipi_panel(hisifd)) {
			outp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, clear);
		}
		outp32(dss_base + DSS_DPP_OFFSET + DPP_INTS, clear);

		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INTS, clear);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_SDP_INTS, clear);
		if (is_dp_panel(hisifd)) {
			outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INTS, clear);
			outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INTS, clear);
		} else {
			outp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INTS, clear);
		}
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_OFF_INTS, clear);
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		clear = ~0;
		outp32(hisifd->media_common_base + GLB_CPU_OFF_INTS, clear);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void dpe_interrupt_unmask(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t unmask = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	pinfo = &(hisifd->panel_info);
	dss_base = hisifd->dss_base;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		unmask = ~0;
		//unmask &= ~(BIT_DPP_INTS | BIT_ITF0_INTS | BIT_DSS_GLB_INTS | BIT_MMU_IRPT_NS);
		unmask &= ~(BIT_DPP_INTS | BIT_ITF0_INTS);
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, unmask);

		unmask = ~0;
		if (is_mipi_cmd_panel(hisifd)) {
			unmask &= ~(BIT_LCD_TE0_PIN | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		} else {
			unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		}

		outp32(hisifd->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK, unmask);
		if (is_dual_mipi_panel(hisifd)) {
			unmask = ~0;
			outp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, unmask);
		}

		unmask = ~0;
		//unmask &= ~(BIT_CE_END_IND | BIT_BACKLIGHT_INTP);
		if ((pinfo->acm_ce_support == 1) && !!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_ACE))
			unmask &= ~(BIT_CE_END_IND);

		if (pinfo->hiace_support == 1)
			unmask &= ~(BIT_HIACE_IND);

		outp32(dss_base + DSS_DISP_CH0_OFFSET + DPP_INT_MSK, unmask);

	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) &&
		is_mipi_panel(hisifd) && !is_dual_mipi_panel(hisifd)) {
		unmask = ~0;
		//unmask &= ~(BIT_SDP_ITF1_INTS  | BIT_SDP_DSS_GLB_INTS | BIT_SDP_MMU_IRPT_NS);
		unmask &= ~(BIT_SDP_ITF1_INTS | BIT_SDP_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, unmask);

		unmask = ~0;
		if (is_mipi_cmd_panel(hisifd)) {
			unmask &= ~(BIT_LCD_TE0_PIN | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		} else {
			unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		}
		outp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, unmask);
	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && is_dp_panel(hisifd)) {
		unmask = ~0;
		//unmask &= ~(BIT_DP_ITF1_INTS	| BIT_DP_DSS_GLB_INTS | BIT_DP_MMU_IRPT_NS);
		unmask &= ~(BIT_DP_ITF2_INTS | BIT_DP_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_DP_INT_MSK, unmask);

		unmask = ~0;
		unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		outp32(dss_base +  DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);

		outp32(dss_base +  DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_OFF_WCH0_INTS | BIT_OFF_WCH1_INTS | BIT_OFF_WCH0_WCH1_FRM_END_INT | BIT_OFF_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, unmask);
		unmask = ~0;
		unmask &= ~(BIT_OFF_CAM_WCH2_FRMEND_INTS);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, unmask);
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_OFF_WCH1_INTS);

		outp32(hisifd->media_common_base + GLB_CPU_OFF_INT_MSK, unmask);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void dpe_interrupt_mask(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t mask = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, mask);

		outp32(hisifd->mipi_dsi0_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
		if (is_dual_mipi_panel(hisifd)) {
			outp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
		}

		outp32(dss_base + DSS_DISP_CH0_OFFSET + DPP_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INT_MSK, mask);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, mask);
		if (is_dp_panel(hisifd)) {
			outp32(dss_base + DSS_LDI_DP_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
			outp32(dss_base + DSS_LDI_DP1_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
		} else {
			outp32(hisifd->mipi_dsi1_base + MIPI_LDI_CPU_ITF_INT_MSK, mask);
		}
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, mask);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, mask);
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		mask = ~0;
		outp32(hisifd->media_common_base + GLB_CPU_OFF_INT_MSK, mask);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}
/*lint +e838*/
static void mipi_ldi_data_gate_ctrl(char __iomem *ldi_base, bool enable)
{
	if (g_ldi_data_gate_en == 1) {
		set_reg(ldi_base + MIPI_LDI_FRM_VALID_DBG, (enable ? 0x0 : 0x1), 1, 29);
	} else {
		set_reg(ldi_base + MIPI_LDI_FRM_VALID_DBG, 0x1, 1, 29);
	}
}

void ldi_data_gate(struct hisi_fb_data_type *hisifd, bool enble)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (!is_mipi_cmd_panel(hisifd)) {
		hisifd->ldi_data_gate_en = (enble ? 1 : 0);
		return ;
	}

	if (is_dp_panel(hisifd)) {
		ldi_base = hisifd->dss_base + DSS_LDI_DP_OFFSET;
		if (g_ldi_data_gate_en == 1) {
			set_reg(ldi_base + LDI_CTRL, (enble ? 0x1 : 0x0), 1, 2);
		} else {
			set_reg(ldi_base + LDI_CTRL, 0x0, 1, 2);
		}

		ldi_base = hisifd->dss_base + DSS_LDI_DP1_OFFSET;
		if (g_ldi_data_gate_en == 1) {
			set_reg(ldi_base + LDI_CTRL, (enble ? 0x1 : 0x0), 1, 2);
		} else {
			set_reg(ldi_base + LDI_CTRL, 0x0, 1, 2);
		}
	} else {
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			mipi_ldi_data_gate_ctrl(hisifd->mipi_dsi0_base, enble);
			if (is_dual_mipi_panel(hisifd)) {
				mipi_ldi_data_gate_ctrl(hisifd->mipi_dsi1_base, enble);
			}
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			mipi_ldi_data_gate_ctrl(hisifd->mipi_dsi1_base, enble);
		} else {
			HISI_FB_ERR("fb%d, not support!", hisifd->index);
			return;
		}
	}

	if (g_ldi_data_gate_en == 1) {
		hisifd->ldi_data_gate_en = (enble ? 1 : 0);
	} else {
		hisifd->ldi_data_gate_en = 0;
	}

}

/* dpp csc config */
#define CSC_ROW	(3)
#define CSC_COL	(5)

/*
** Rec.601 for Computer
** [ p00 p01 p02 cscidc2 cscodc2 ]
** [ p10 p11 p12 cscidc1 cscodc1 ]
** [ p20 p21 p22 cscidc0 cscodc0 ]
*/
static int CSC10B_YUV2RGB709_WIDE[CSC_ROW][CSC_COL] = {
	{ 0x4000, 0x00000, 0x064ca, 0x000, 0x000 },
	{ 0x4000, 0x1f403, 0x1e20a, 0x600, 0x000 },
	{ 0x4000, 0x076c2, 0x00000, 0x600, 0x000 },
};

static int CSC10B_RGB2YUV709_WIDE[CSC_ROW][CSC_COL] = {
	{ 0x00d9b, 0x02dc6, 0x0049f, 0x000, 0x000 },
	{ 0x1f8ab, 0x1e755, 0x02000, 0x000, 0x200 },
	{ 0x02000, 0x1e2ef, 0x1fd11, 0x000, 0x200 },
};

static void init_csc10b(struct hisi_fb_data_type *hisifd, char __iomem * dpp_csc10b_base)
{
	int (*csc_coe)[CSC_COL];

	if (hisifd == NULL || dpp_csc10b_base == NULL) {
		HISI_FB_ERR("hisifd or dpp_csc10b_base is NULL!\n");
		return;
	}

	if (dpp_csc10b_base == (hisifd->dss_base + DSS_DPP_CSC_RGB2YUV10B_OFFSET)) {
		csc_coe = CSC10B_RGB2YUV709_WIDE;
		outp32(dpp_csc10b_base + CSC10B_MPREC, 0x2);
	} else if (dpp_csc10b_base == (hisifd->dss_base + DSS_DPP_CSC_YUV2RGB10B_OFFSET)) {
		csc_coe = CSC10B_YUV2RGB709_WIDE;
		outp32(dpp_csc10b_base + CSC10B_MPREC, 0x0);
	} else {
		return;
	}

	outp32(dpp_csc10b_base + CSC10B_IDC0, csc_coe[2][3]);
	outp32(dpp_csc10b_base + CSC10B_IDC1, csc_coe[1][3]);
	outp32(dpp_csc10b_base + CSC10B_IDC2, csc_coe[0][3]);
	outp32(dpp_csc10b_base + CSC10B_ODC0, csc_coe[2][4]);
	outp32(dpp_csc10b_base + CSC10B_ODC1, csc_coe[1][4]);
	outp32(dpp_csc10b_base + CSC10B_ODC2, csc_coe[0][4]);
	outp32(dpp_csc10b_base + CSC10B_P00, csc_coe[0][0]);
	outp32(dpp_csc10b_base + CSC10B_P01, csc_coe[0][1]);
	outp32(dpp_csc10b_base + CSC10B_P02, csc_coe[0][2]);
	outp32(dpp_csc10b_base + CSC10B_P10, csc_coe[1][0]);
	outp32(dpp_csc10b_base + CSC10B_P11, csc_coe[1][1]);
	outp32(dpp_csc10b_base + CSC10B_P12, csc_coe[1][2]);
	outp32(dpp_csc10b_base + CSC10B_P20, csc_coe[2][0]);
	outp32(dpp_csc10b_base + CSC10B_P21, csc_coe[2][1]);
	outp32(dpp_csc10b_base + CSC10B_P22, csc_coe[2][2]);

	outp32(dpp_csc10b_base + CSC10B_MODULE_EN, 0x1);
}
/*lint -e679, -e838*/
void init_dpp_csc(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("init_dpp_csc hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (pinfo->acm_support || pinfo->arsr1p_sharpness_support || pinfo->post_scf_support) {
		// init csc10b rgb2yuv
		init_csc10b(hisifd, hisifd->dss_base + DSS_DPP_CSC_RGB2YUV10B_OFFSET);
		// init csc10b yuv2rgb
		init_csc10b(hisifd, hisifd->dss_base + DSS_DPP_CSC_YUV2RGB10B_OFFSET);
	}
}

void acm_set_lut(char __iomem *address, uint32_t table[], uint32_t size)
{
	return;
}

void acm_set_lut_hue(char __iomem *address, uint32_t table[], uint32_t size)
{
	return;
}

void init_acm(struct hisi_fb_data_type *hisifd)
{
	return;
}
/*lint +e838*/

//lint -e838 -e550 -e438 -e737
static void dpe_init_degamma(struct hisi_fb_data_type *hisifd, uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *degamma_lut_base = NULL;
	char __iomem *degamma_base = NULL;
	struct degamma_info *degamma = NULL;
	struct dpp_buf_maneger *dpp_buff_mngr = NULL;
	uint32_t degamma_enable;
	uint32_t i;
	uint32_t index;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL!\n");
		return;
	}

	degamma_lut_base = hisifd->dss_base + DSS_DPP_DEGAMMA_LUT_OFFSET + config_ch * 0x40000;
	degamma_base = hisifd->dss_base + DSS_DPP_DEGAMMA_OFFSET + config_ch * 0x40000;

	pinfo = &(hisifd->panel_info);
	if(pinfo->gamma_support != true) {
		//degama memory shutdown
		outp32(degamma_base + DEGAMA_MEM_CTRL, 0x2); //only support deep sleep
		HISI_FB_INFO("[effect]degamma is not support!");
		return ;
	}

	dpp_buff_mngr = &(hisifd->effect_info.dpp_buf);
	if (dpp_buff_mngr->buf_info_list[buff_sel].degama_buf_save_status == DPP_BUF_INVALIED) {
		HISI_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	// choose default pinfo parameters
	if (param_choice == 0) {
		if (pinfo->igm_lut_table_len > 0
			&& pinfo->igm_lut_table_R
			&& pinfo->igm_lut_table_G
			&& pinfo->igm_lut_table_B) {
			for (i = 0; i < pinfo->igm_lut_table_len / 2; i++) {
				index = i * 2;
				outp32(degamma_lut_base + (U_DEGAMA_R_COEF +  i * 4), pinfo->igm_lut_table_R[index] | pinfo->igm_lut_table_R[index + 1] << 16);
				outp32(degamma_lut_base + (U_DEGAMA_G_COEF +  i * 4), pinfo->igm_lut_table_G[index] | pinfo->igm_lut_table_G[index + 1] << 16);
				outp32(degamma_lut_base + (U_DEGAMA_B_COEF +  i * 4), pinfo->igm_lut_table_B[index] | pinfo->igm_lut_table_B[index + 1] << 16);
			}
			outp32(degamma_lut_base + U_DEGAMA_R_LAST_COEF, pinfo->igm_lut_table_R[pinfo->igm_lut_table_len - 1]);
			outp32(degamma_lut_base + U_DEGAMA_G_LAST_COEF, pinfo->igm_lut_table_G[pinfo->igm_lut_table_len - 1]);
			outp32(degamma_lut_base + U_DEGAMA_B_LAST_COEF, pinfo->igm_lut_table_B[pinfo->igm_lut_table_len - 1]);
			HISI_FB_INFO("[effect]pinfo:R=%d,%d,%d,%d,%d,%d G=%d,%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d,%d\n", \
				pinfo->igm_lut_table_R[0], pinfo->igm_lut_table_R[1], pinfo->igm_lut_table_R[2], \
				pinfo->igm_lut_table_R[3], pinfo->igm_lut_table_R[4], pinfo->igm_lut_table_R[5], \
				pinfo->igm_lut_table_G[0], pinfo->igm_lut_table_G[1], pinfo->igm_lut_table_G[2], \
				pinfo->igm_lut_table_G[3], pinfo->igm_lut_table_G[4], pinfo->igm_lut_table_G[5], \
				pinfo->igm_lut_table_B[0], pinfo->igm_lut_table_B[1], pinfo->igm_lut_table_B[2], \
				pinfo->igm_lut_table_B[3], pinfo->igm_lut_table_B[4], pinfo->igm_lut_table_B[5]);
		}
		set_reg(degamma_base + DEGAMA_EN, 1, 1, 0);
	}

	// choose user parameters
	if (param_choice == 1) {
		degamma = &(dpp_buff_mngr->buf_info_list[buff_sel].degamma);
		degamma_enable = degamma->degamma_enable;
		if (degamma_enable) {
			for (i = 0; i < LCP_IGM_LUT_LENGTH; i = i + 2) {
				set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + i * 2), degamma->degamma_r_lut[i], 12, 0);
				if (i != LCP_IGM_LUT_LENGTH - 1)
					set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + i * 2), degamma->degamma_r_lut[i + 1], 12, 16);

				set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + i * 2), degamma->degamma_g_lut[i], 12, 0);
				if (i != LCP_IGM_LUT_LENGTH - 1)
					set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + i * 2), degamma->degamma_g_lut[i + 1], 12, 16);

				set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + i * 2),  degamma->degamma_b_lut[i], 12, 0);
				if (i != LCP_IGM_LUT_LENGTH - 1)
					set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + i * 2), degamma->degamma_b_lut[i + 1], 12, 16);
			}
			HISI_FB_INFO("[effect]user:R=%d,%d,%d,%d,%d,%d G=%d,%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d,%d\n", \
				degamma->degamma_r_lut[0], degamma->degamma_r_lut[1], degamma->degamma_r_lut[2], \
				degamma->degamma_r_lut[3], degamma->degamma_r_lut[4], degamma->degamma_r_lut[5], \
				degamma->degamma_g_lut[0], degamma->degamma_g_lut[1], degamma->degamma_g_lut[2], \
				degamma->degamma_g_lut[3], degamma->degamma_g_lut[4], degamma->degamma_g_lut[5], \
				degamma->degamma_b_lut[0], degamma->degamma_b_lut[1], degamma->degamma_b_lut[2], \
				degamma->degamma_b_lut[3], degamma->degamma_b_lut[4], degamma->degamma_b_lut[5]);
		}
		set_reg(degamma_base + DEGAMA_EN, degamma->degamma_enable, 1, 0);
	}

	return ;
}

static void dpe_init_gmp(struct hisi_fb_data_type *hisifd, uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	char __iomem *gmp_lut_base = NULL;
	char __iomem *gmp_base = NULL;
	uint32_t gmp_enable;
	struct gmp_info *gmp = NULL;
	struct dpp_buf_maneger *dpp_buff_mngr = NULL;
	struct hisi_panel_info *pinfo = NULL;

	uint32_t i;

	if (hisifd == NULL)	{
		HISI_FB_ERR("[effect] hisifd is NULL!\n");
		return;
	}

	gmp_lut_base = hisifd->dss_base + DSS_DPP_GMP_LUT_OFFSET + config_ch * 0x10000;
	gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET + config_ch * 0x40000;

	pinfo = &(hisifd->panel_info);
	if(pinfo->gamma_support != true) {
		//gmp memory shutdown
		outp32(gmp_base + GMP_MEM_CTRL, 0x2); //only support deep sleep
		HISI_FB_INFO("[effect]gmp is not support!");
		return ;
	}

	dpp_buff_mngr = &(hisifd->effect_info.dpp_buf);
	if (dpp_buff_mngr->buf_info_list[buff_sel].gmp_buf_save_status == DPP_BUF_INVALIED) {
		HISI_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	// choose default pinfo parameters
	if (param_choice == 0) {
		for (i = 0; i < LCP_GMP_LUT_LENGTH; i++) {
			outp32(gmp_lut_base + i * 2 * 4, g_gmp_lut_table_low32bit_init[i]);
			outp32(gmp_lut_base + i * 2 * 4 + 4, g_gmp_lut_table_high4bit_init[i]);
		}
		set_reg(gmp_base + GMP_EN, 0, 1, 0);

		HISI_FB_INFO("[effect]pinfo:low32=%d,%d,%d,%d,%d,%d,%d,%d,%d high4=%d,%d,%d,%d,%d,%d,%d,%d,%d\n", \
			g_gmp_lut_table_low32bit_init[0], g_gmp_lut_table_low32bit_init[500], g_gmp_lut_table_low32bit_init[1000], \
			g_gmp_lut_table_low32bit_init[1500], g_gmp_lut_table_low32bit_init[2000], g_gmp_lut_table_low32bit_init[2500], \
			g_gmp_lut_table_low32bit_init[3000], g_gmp_lut_table_low32bit_init[3500], g_gmp_lut_table_low32bit_init[4000], \
			g_gmp_lut_table_high4bit_init[0], g_gmp_lut_table_high4bit_init[500], g_gmp_lut_table_high4bit_init[1000], \
			g_gmp_lut_table_high4bit_init[1500], g_gmp_lut_table_high4bit_init[2000], g_gmp_lut_table_high4bit_init[2500], \
			g_gmp_lut_table_high4bit_init[3000], g_gmp_lut_table_high4bit_init[3500], g_gmp_lut_table_high4bit_init[4000]);
	}

	// choose user parameters
	if (param_choice == 1) {
		gmp = &(hisifd->effect_info.dpp_buf.buf_info_list[buff_sel].gmp);
		gmp_enable = gmp->gmp_enable;
		if (gmp_enable) {
			for (i = 0; i < LCP_GMP_LUT_LENGTH; i++) {
				// cppcheck-suppress *
				set_reg(gmp_lut_base + i * 2 * 4, gmp->gmp_lut_low32bit[i], 32, 0);
				// cppcheck-suppress *
				set_reg(gmp_lut_base + i * 2 * 4 + 4, gmp->gmp_lut_high4bit[i], 4, 0);
			}
			// cppcheck-suppress *
			HISI_FB_INFO("[effect]user:low32=%d,%d,%d,%d,%d,%d,%d,%d,%d high4=%d,%d,%d,%d,%d,%d,%d,%d,%d\n", \
				gmp->gmp_lut_low32bit[0], gmp->gmp_lut_low32bit[500], gmp->gmp_lut_low32bit[1000], \
				gmp->gmp_lut_low32bit[1500], gmp->gmp_lut_low32bit[2000], gmp->gmp_lut_low32bit[2500], \
				gmp->gmp_lut_low32bit[3000], gmp->gmp_lut_low32bit[3500], gmp->gmp_lut_low32bit[4000], \
				gmp->gmp_lut_high4bit[0], gmp->gmp_lut_high4bit[500], gmp->gmp_lut_high4bit[1000], \
				gmp->gmp_lut_high4bit[1500], gmp->gmp_lut_high4bit[2000], gmp->gmp_lut_high4bit[2500], \
				gmp->gmp_lut_high4bit[3000], gmp->gmp_lut_high4bit[3500], gmp->gmp_lut_high4bit[4000]);
		}
		set_reg(gmp_base + GMP_EN, gmp_enable, 1, 0);
	}
	return;
}

static uint32_t get_color_temp_rectify(struct hisi_panel_info *pinfo, uint32_t color_temp_rectify)
{
	if (pinfo->color_temp_rectify_support && color_temp_rectify && color_temp_rectify <= 32768) {
		return color_temp_rectify;
	}
	return 32768;
}

static void dpe_init_xcc(struct hisi_fb_data_type *hisifd, uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	char __iomem *xcc_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	uint32_t xcc_enable;
	struct xcc_info *xcc = NULL;
	struct dpp_buf_maneger *dpp_buff_mngr = NULL;
	uint32_t color_temp_rectify_R;
	uint32_t color_temp_rectify_G;
	uint32_t color_temp_rectify_B;
	uint32_t cnt;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	color_temp_rectify_R = get_color_temp_rectify(pinfo, pinfo->color_temp_rectify_R);
	color_temp_rectify_G = get_color_temp_rectify(pinfo, pinfo->color_temp_rectify_G);
	color_temp_rectify_B = get_color_temp_rectify(pinfo, pinfo->color_temp_rectify_B);

	if (hisifd->effect_ctl.lcp_xcc_support != true) {
		HISI_FB_INFO("[effect]xcc is not support!");
		return;
	}

	xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET + config_ch * 0x40000;
	dpp_buff_mngr = &(hisifd->effect_info.dpp_buf);
	if (dpp_buff_mngr->buf_info_list[buff_sel].xcc_buf_save_status == DPP_BUF_INVALIED) {
		HISI_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	if (param_choice == 0) {
		if (pinfo->xcc_table_len == xcc_cnt_cofe && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1] *
				g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6] *
				g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11] *
				g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX *
				color_temp_rectify_B / 32768);
		}
		set_reg(xcc_base + XCC_EN, 1, 1, 0);
	}

	if (param_choice == 1) {
		xcc = dpp_buff_mngr->buf_info_list[buff_sel].xcc;
		xcc_enable = xcc->xcc_enable;
		if (xcc_enable) {
			for (cnt = 0; cnt < XCC_COEF_LEN; cnt++)
					set_reg(xcc_base + XCC_COEF_00 + cnt * 4,  xcc->xcc_table[cnt], 17, 0);
		}
		set_reg(xcc_base + XCC_EN, xcc->xcc_enable, 1, 0);
	}

	return;
}

static void dpe_init_gamma(struct hisi_fb_data_type *hisifd, uint32_t config_ch, uint32_t buff_sel, int param_choice)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *gamma_lut_base = NULL;
	char __iomem *gama_base = NULL;
	struct gama_info *gama = NULL;
	struct dpp_buf_maneger *dpp_buff_mngr = NULL;
	uint32_t gama_enable;
	uint32_t i;
	uint32_t index;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL!\n");
		return;
	}

	gamma_lut_base = hisifd->dss_base + DSS_DPP_GAMA_LUT_OFFSET + config_ch * 0x40000;
	gama_base = hisifd->dss_base + DSS_DPP_GAMA_OFFSET + config_ch * 0x40000;

	pinfo = &(hisifd->panel_info);
	if(pinfo->gamma_support != true) {
		//degama memory shutdown
		outp32(gama_base + DEGAMA_MEM_CTRL, 0x2); //only support deep sleep
		HISI_FB_INFO("[effect]gamma is not support!");
		return ;
	}

	dpp_buff_mngr = &(hisifd->effect_info.dpp_buf);
	if (dpp_buff_mngr->buf_info_list[buff_sel].gama_buf_save_status == DPP_BUF_INVALIED) {
		HISI_FB_INFO("[effect]user param is valid,choose pinfo param!\n");
		param_choice = 0;
	}

	if (param_choice == 0) {
		if (pinfo->gamma_lut_table_len > 0 &&
			pinfo->gamma_lut_table_R &&
			pinfo->gamma_lut_table_G &&
			pinfo->gamma_lut_table_B) {
			for (i = 0; i < pinfo->gamma_lut_table_len / 2; i++) {
				index = i * 2;
				outp32(gamma_lut_base + (U_GAMA_R_COEF + i * 4), pinfo->gamma_lut_table_R[index] | pinfo->gamma_lut_table_R[index + 1] << 16);
				outp32(gamma_lut_base + (U_GAMA_G_COEF + i * 4), pinfo->gamma_lut_table_G[index] | pinfo->gamma_lut_table_G[index + 1] << 16);
				outp32(gamma_lut_base + (U_GAMA_B_COEF + i * 4), pinfo->gamma_lut_table_B[index] | pinfo->gamma_lut_table_B[index + 1] << 16);
			}
			outp32(gamma_lut_base + U_GAMA_R_LAST_COEF, pinfo->gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_lut_base + U_GAMA_G_LAST_COEF, pinfo->gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_lut_base + U_GAMA_B_LAST_COEF, pinfo->gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);
			HISI_FB_INFO("[effect]pinfo:R=%d,%d,%d,%d,%d,%d G=%d,%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d,%d\n", \
				pinfo->gamma_lut_table_R[0], pinfo->gamma_lut_table_R[1], pinfo->gamma_lut_table_R[2], \
				pinfo->gamma_lut_table_R[3], pinfo->gamma_lut_table_R[4], pinfo->gamma_lut_table_R[5], \
				pinfo->gamma_lut_table_G[0], pinfo->gamma_lut_table_G[1], pinfo->gamma_lut_table_G[2], \
				pinfo->gamma_lut_table_G[3], pinfo->gamma_lut_table_G[4], pinfo->gamma_lut_table_G[5], \
				pinfo->gamma_lut_table_B[0], pinfo->gamma_lut_table_B[1], pinfo->gamma_lut_table_B[2], \
				pinfo->gamma_lut_table_B[3], pinfo->gamma_lut_table_B[4], pinfo->gamma_lut_table_B[5]);
		}
		set_reg(gama_base + GAMA_EN, 1, 1, 0);
	}

	if (param_choice == 1) {
		gama = &(hisifd->effect_info.dpp_buf.buf_info_list[buff_sel].gama);
		gama_enable = gama->gama_enable;
		if (gama_enable) {
			for (i = 0; i < GAMA_LUT_LENGTH; i = i + 2) {
				set_reg(gamma_lut_base + (U_GAMA_R_COEF + i * 2), gama->gama_r_lut[i], 12, 0);
				if (i != GAMA_LUT_LENGTH - 1)
					set_reg(gamma_lut_base + (U_GAMA_R_COEF + i * 2), gama->gama_r_lut[i + 1], 12, 16);

				set_reg(gamma_lut_base + (U_GAMA_G_COEF + i * 2), gama->gama_g_lut[i], 12, 0);
				if (i != GAMA_LUT_LENGTH - 1)
					set_reg(gamma_lut_base + (U_GAMA_G_COEF + i * 2), gama->gama_g_lut[i + 1], 12, 16);

				set_reg(gamma_lut_base + (U_GAMA_B_COEF + i * 2), gama->gama_b_lut[i], 12, 0);
				if (i != GAMA_LUT_LENGTH - 1)
					set_reg(gamma_lut_base + (U_GAMA_B_COEF + i * 2), gama->gama_b_lut[i + 1], 12, 16);
			}

			HISI_FB_INFO("[effect]user:R=%d,%d,%d,%d,%d,%d G=%d,%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d,%d\n", \
				gama->gama_r_lut[0], gama->gama_r_lut[1], gama->gama_r_lut[2], \
				gama->gama_r_lut[3], gama->gama_r_lut[4], gama->gama_r_lut[5], \
				gama->gama_g_lut[0], gama->gama_g_lut[1], gama->gama_g_lut[2], \
				gama->gama_g_lut[3], gama->gama_g_lut[4], gama->gama_g_lut[5], \
				gama->gama_b_lut[0], gama->gama_b_lut[1], gama->gama_b_lut[2], \
				gama->gama_b_lut[3], gama->gama_b_lut[4], gama->gama_b_lut[5]);
		}
		set_reg(gama_base + GAMA_EN, gama->gama_enable, 1, 0);
	}
	return;
}

static void dpe_init_post_xcc(struct hisi_fb_data_type *hisifd)
{
	char __iomem *post_xcc_base = NULL;

	if (hisifd == NULL)	{
		HISI_FB_ERR("hisifd is NULL!\n");
		return;
	}

	post_xcc_base = hisifd->dss_base + DSS_DPP_POST_XCC_OFFSET;

	outp32(post_xcc_base + POST_XCC_COEF_00, g_post_xcc_table_temp[0]);
	outp32(post_xcc_base + POST_XCC_COEF_01, g_post_xcc_table_temp[1]);
	outp32(post_xcc_base + POST_XCC_COEF_02, g_post_xcc_table_temp[2]);
	outp32(post_xcc_base + POST_XCC_COEF_03, g_post_xcc_table_temp[3]);
	outp32(post_xcc_base + POST_XCC_COEF_10, g_post_xcc_table_temp[4]);
	outp32(post_xcc_base + POST_XCC_COEF_11, g_post_xcc_table_temp[5]);
	outp32(post_xcc_base + POST_XCC_COEF_12, g_post_xcc_table_temp[6]);
	outp32(post_xcc_base + POST_XCC_COEF_13, g_post_xcc_table_temp[7]);
	outp32(post_xcc_base + POST_XCC_COEF_20, g_post_xcc_table_temp[8]);
	outp32(post_xcc_base + POST_XCC_COEF_21, g_post_xcc_table_temp[9]);
	outp32(post_xcc_base + POST_XCC_COEF_22, g_post_xcc_table_temp[10]);
	outp32(post_xcc_base + POST_XCC_COEF_23, g_post_xcc_table_temp[11]);

	set_reg(post_xcc_base + POST_XCC_EN, 1, 1, 0);
	return;
}

void init_igm_gmp_xcc_gm(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dpp_buf_maneger *dpp_buff_mngr = NULL;
	uint32_t *ch_status = NULL;
	uint32_t buff_sel;
	uint32_t work_channel;
	static bool isInit = false;
	uint32_t dpp_ch_idx;

	if (hisifd == NULL)	{
		HISI_FB_ERR("hisifd is NULL!\n");
		return;
	}

	// avoid the partial update
	hisifd->display_effect_flag = 40;

	dpp_buff_mngr = &(hisifd->effect_info.dpp_buf);
	buff_sel = dpp_buff_mngr->dpp_latest_buf;
	if (buff_sel >= DPP_BUF_MAX_COUNT) {
		HISI_FB_INFO("[effect] select buffer %d\n", buff_sel);
		return;
	}

	// dump start status info  of software buffer and hardware channel
	work_channel = (uint32_t)inp32(hisifd->dss_base + DSS_DISP_GLB_OFFSET + DYN_SW_DEFAULT) & 0x1; //default read value is dpp0
	ch_status = hisifd->effect_info.dpp_chn_status;
	HISI_FB_INFO("[effect] inited=%d buf_sel=%d, buf_status:gmp=%d,gama=%d,degama=%d,xcc=%d, work_ch=%d, ch_status=%d,%d", \
		isInit,
		buff_sel,
		dpp_buff_mngr->buf_info_list[buff_sel].gmp_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[buff_sel].gama_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[buff_sel].degama_buf_cfg_status,
		dpp_buff_mngr->buf_info_list[buff_sel].xcc_buf_cfg_status,
		work_channel,
		*(ch_status + work_channel),
		*(ch_status + ((~work_channel) & 0x1)));

	if (isInit) {
		for (dpp_ch_idx = 0; dpp_ch_idx < DPP_CHN_MAX_COUNT; dpp_ch_idx++) {
			dpe_init_degamma(hisifd, dpp_ch_idx, buff_sel, 1);
			dpe_init_gmp(hisifd, dpp_ch_idx, buff_sel, 1);
			dpe_init_xcc(hisifd, dpp_ch_idx, buff_sel, 1);
			dpe_init_gamma(hisifd, dpp_ch_idx, buff_sel, 1);
		}
	} else {
		for (dpp_ch_idx = 0; dpp_ch_idx < DPP_CHN_MAX_COUNT; dpp_ch_idx++) {
			dpe_init_degamma(hisifd, dpp_ch_idx, buff_sel, 0);
			dpe_init_gmp(hisifd, dpp_ch_idx, buff_sel, 0);
			dpe_init_xcc(hisifd, dpp_ch_idx, buff_sel, 0);
			dpe_init_gamma(hisifd, dpp_ch_idx, buff_sel, 0);
		}

		// POST XCC pre matrix
		pinfo = &(hisifd->panel_info);
		if (pinfo->post_xcc_support == 1) {
			dpe_init_post_xcc(hisifd);
		}

		isInit = true;
	}

	//if dpp0&dpp1 gmp inited successfully, gmp_wkq_panel_state should be false
	dpp_buff_mngr->gmp_wkq_panel_state = false;

	return;
}

void init_dither(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *dither_base = NULL;

	if (hisifd == NULL)	{
		HISI_FB_ERR("hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (pinfo->dither_support != 1) {
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dither_base = hisifd->dss_base + DSS_DPP_DITHER_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	set_reg(dither_base + DITHER_CTL1, 0x00000005, 6, 0);
	set_reg(dither_base + DITHER_CTL0, 0x0000000B, 5, 0);
	set_reg(dither_base + DITHER_TRI_THD12_0, 0x00080080, 24, 0);
	set_reg(dither_base + DITHER_TRI_THD12_1, 0x00000080, 12, 0);
	set_reg(dither_base + DITHER_TRI_THD10, 0x02008020, 30, 0);
	set_reg(dither_base + DITHER_TRI_THD12_UNI_0, 0x00100100, 24, 0);
	set_reg(dither_base + DITHER_TRI_THD12_UNI_1, 0x00000000, 12, 0);
	set_reg(dither_base + DITHER_TRI_THD10_UNI, 0x00010040, 30, 0);
	set_reg(dither_base + DITHER_BAYER_CTL, 0x00000000, 28, 0);
	set_reg(dither_base + DITHER_BAYER_ALPHA_THD, 0x00000000, 30, 0);
	set_reg(dither_base + DITHER_MATRIX_PART1, 0x5D7F91B3, 32, 0);
	set_reg(dither_base + DITHER_MATRIX_PART0, 0x6E4CA280, 32, 0);

	set_reg(dither_base + DITHER_HIFREQ_REG_INI_CFG_EN, 0x00000001, 1, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_0, 0x6495FC13, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_1, 0x27E5DB75, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_2, 0x69036280, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_3, 0x7478D47C, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_0, 0x36F5325D, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_1, 0x90757906, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_2, 0xBBA85F01, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_3, 0x74B34461, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_0, 0x76435C64, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_1, 0x4989003F, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_2, 0xA2EA34C6, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_3, 0x4CAD42CB, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_POWER_CTRL, 0x00000009, 4, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_0, 0x00000421, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_1, 0x00000701, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_2, 0x00000421, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_R0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_R1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_G0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_G1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_B0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_B1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_ERRDIFF_CTL, 0x00000000, 3, 0);
	set_reg(dither_base + DITHER_ERRDIFF_WEIGHT, 0x01232134, 28, 0);

	set_reg(dither_base + DITHER_FRC_CTL, 0x00000001, 4, 0);
	set_reg(dither_base + DITHER_FRC_01_PART1, 0xFFFF0000, 32, 0);
	set_reg(dither_base + DITHER_FRC_01_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART0, 0xFFFF0000, 32, 0);
}
//lint +e838 +e550 +e438 +e737
/*lint -e838*/
void dpe_store_ct_cscValue(struct hisi_fb_data_type *hisifd, unsigned int csc_value[])
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL) {
		return;
	}

	pinfo->xcc_table[1] = csc_value[0];
	pinfo->xcc_table[2] = csc_value[1];
	pinfo->xcc_table[3] = csc_value[2];
	pinfo->xcc_table[5] = csc_value[3];
	pinfo->xcc_table[6] = csc_value[4];
	pinfo->xcc_table[7] = csc_value[5];
	pinfo->xcc_table[9] = csc_value[6];
	pinfo->xcc_table[10] = csc_value[7];
	pinfo->xcc_table[11] = csc_value[8];

	return;
}

void dpe_update_g_comform_discount(unsigned int value)
{
	g_comform_value = value;
	HISI_FB_INFO("g_comform_value = %d", g_comform_value);
}

int dpe_set_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_R = 32768;
	uint32_t color_temp_rectify_G = 32768;
	uint32_t color_temp_rectify_B = 32768;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -1;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R && pinfo->color_temp_rectify_R <= 32768) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B && pinfo->color_temp_rectify_B <= 32768) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}

	//XCC
	if (pinfo->xcc_support == 1) {
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
			hisifd->color_temperature_flag = 2;
		}
	}

	return 0;
}

ssize_t dpe_show_ct_cscValue(struct hisi_fb_data_type *hisifd, char *buf)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL) {
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		pinfo->xcc_table[1], pinfo->xcc_table[2], pinfo->xcc_table[3],
		pinfo->xcc_table[5], pinfo->xcc_table[6], pinfo->xcc_table[7],
		pinfo->xcc_table[9], pinfo->xcc_table[10], pinfo->xcc_table[11]);
}

int dpe_set_xcc_cscValue(struct hisi_fb_data_type *hisifd)
{
	return 0;
}

int dpe_set_comform_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -1;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R && pinfo->color_temp_rectify_R <= 32768) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B && pinfo->color_temp_rectify_B <= 32768) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}

	//XCC
	if (pinfo->xcc_support == 1) {
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
		}
	}

	return 0;
}

ssize_t dpe_show_comform_ct_cscValue(struct hisi_fb_data_type *hisifd, char *buf)
{
	struct hisi_panel_info *pinfo = NULL;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL) {
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,g_comform_value = %d\n",
		pinfo->xcc_table[1], pinfo->xcc_table[2], pinfo->xcc_table[3],
		pinfo->xcc_table[5], pinfo->xcc_table[6], pinfo->xcc_table[7],
		pinfo->xcc_table[9], pinfo->xcc_table[10], pinfo->xcc_table[11],
		g_comform_value);
}

void dpe_init_led_rg_ct_cscValue(void)
{
	g_led_rg_csc_value[0] = 32768;
	g_led_rg_csc_value[1] = 0;
	g_led_rg_csc_value[2] = 0;
	g_led_rg_csc_value[3] = 0;
	g_led_rg_csc_value[4] = 32768;
	g_led_rg_csc_value[5] = 0;
	g_led_rg_csc_value[6] = 0;
	g_led_rg_csc_value[7] = 0;
	g_led_rg_csc_value[8] = 32768;
	g_is_led_rg_csc_set = 0;

	return;
}

void dpe_store_led_rg_ct_cscValue(unsigned int csc_value[])
{
	g_led_rg_csc_value [0] = csc_value[0];
	g_led_rg_csc_value [1] = csc_value[1];
	g_led_rg_csc_value [2] = csc_value[2];
	g_led_rg_csc_value [3] = csc_value[3];
	g_led_rg_csc_value [4] = csc_value[4];
	g_led_rg_csc_value [5] = csc_value[5];
	g_led_rg_csc_value [6] = csc_value[6];
	g_led_rg_csc_value [7] = csc_value[7];
	g_led_rg_csc_value [8] = csc_value[8];
	g_is_led_rg_csc_set = 1;

	return;
}

int dpe_set_led_rg_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -1;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R && pinfo->color_temp_rectify_R <= 32768) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B && pinfo->color_temp_rectify_B <= 32768) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}

	//XCC
	if (g_is_led_rg_csc_set == 1 && pinfo->xcc_support == 1) {
		HISI_FB_DEBUG("real set color temperature: g_is_led_rg_csc_set = %d, R = 0x%x, G = 0x%x, B = 0x%x .\n",
				g_is_led_rg_csc_set, g_led_rg_csc_value[0], g_led_rg_csc_value[4], g_led_rg_csc_value[8]);
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
		}
	}

	return 0;
}
/*lint +e838*/
ssize_t dpe_show_led_rg_ct_cscValue(char *buf)
{
	if (buf == NULL) {
		HISI_FB_ERR("buf, NUll pointer warning\n");
		return 0;
	}
	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		g_led_rg_para1, g_led_rg_para2,
		g_led_rg_csc_value [0], g_led_rg_csc_value [1], g_led_rg_csc_value [2],
		g_led_rg_csc_value [3], g_led_rg_csc_value [4], g_led_rg_csc_value [5],
		g_led_rg_csc_value [6], g_led_rg_csc_value [7], g_led_rg_csc_value [8]);
}

ssize_t dpe_show_cinema_value(struct hisi_fb_data_type *hisifd, char *buf)
{
	if (buf == NULL) {
		HISI_FB_ERR("buf, NUll pointer warning\n");
		return 0;
	}
	return snprintf(buf, PAGE_SIZE, "gamma type is = %d\n", hisifd->panel_info.gamma_type);
}

int dpe_set_cinema(struct hisi_fb_data_type *hisifd, unsigned int value)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd, NUll pointer warning.\n");
		return -1;
	}

	if (hisifd->panel_info.gamma_type == value) {
		HISI_FB_DEBUG("fb%d, cinema mode is already in %d!\n", hisifd->index, value);
		return 0;
	}

	hisifd->panel_info.gamma_type = value;
	return 0;
}

void dpe_update_g_acm_state(unsigned int value)
{
	return;
}

void dpe_set_acm_state(struct hisi_fb_data_type *hisifd)
{
	return;
}
//lint -e838
ssize_t dpe_show_acm_state(char *buf)
{
	ssize_t ret = 0;

	if (buf == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_acm_State = %d\n", g_acm_State);

	return ret;
}

void dpe_update_g_gmp_state(unsigned int value)
{
	return;
}

void dpe_set_gmp_state(struct hisi_fb_data_type *hisifd)
{
	return;
}

ssize_t dpe_show_gmp_state(char *buf)
{
	ssize_t ret = 0;

	if (buf == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_gmp_State = %d\n", g_gmp_State);

	return ret;
}
//lint +e838
