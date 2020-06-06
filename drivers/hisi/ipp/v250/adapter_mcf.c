#include <linux/printk.h>
#include <linux/slab.h>
#include "mcf.h"
#include "cfg_table_mcf.h"
#include "cvdr_drv.h"
#include "mcf_drv.h"
#include "ipp_top_drv.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "sub_ctrl_reg_offset.h"

#define LOG_TAG LOG_MODULE_MCF

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
/*lint -e429  -e613  -e668*/
#define ALIGN_MCF_WIDTH  (16)

extern mcf_dev_t g_mcf_devs[];
static int mcf_dump_request(msg_req_mcf_request_t *req);
static int mcf_calc_stripe_info(unsigned int is_ygf_mode, cpe_stream_t *stream, isp_stripe_info_t *p_stripe_info,
								isp_stripe_info_t *p_stripe_info_DS4);

static int mcf_update_top_cfg_tab(msg_req_mcf_request_t *mcf_request, cpe_top_config_table_t *cfg_tab,
								  isp_stripe_info_t *stripe_info, isp_stripe_info_t *stripe_info_DS4, unsigned int stripe_index)
{
	cfg_tab->to_use = 1;
	cfg_tab->crop_vpwr.to_use = 1;

	if (mcf_request->streams[BO_RESULT_Y].buf) {
		cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft = stripe_info->overlap_left[stripe_index];
		cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright = stripe_info->stripe_width[stripe_index] -
				stripe_info->overlap_right[stripe_index];
	} else {
		cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft = 0;
		cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright = CROP_DEFAULT_VALUE;
	}

	if (mcf_request->streams[BO_RESULT_UV].buf) {
		if (mcf_request->mcf_cfg.mode == MCF_FULL_MODE || mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE) {
			cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = stripe_info_DS4->overlap_left[stripe_index];
			cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = stripe_info_DS4->stripe_width[stripe_index] -
					stripe_info_DS4->overlap_right[stripe_index];
		} else {
			cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = stripe_info->overlap_left[stripe_index];
			cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = stripe_info->stripe_width[stripe_index] -
					stripe_info->overlap_right[stripe_index];
		}
	} else {
		cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = 0;
		cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = CROP_DEFAULT_VALUE;
	}

	if (mcf_request->streams[BO_RESULT_C].buf) {
		cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft = stripe_info_DS4->overlap_left[stripe_index];
		cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright = stripe_info_DS4->stripe_width[stripe_index] -
				stripe_info_DS4->overlap_right[stripe_index];
	} else {
		cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft = 0;
		cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright = CROP_DEFAULT_VALUE;
	}

	D("crop0 = %d,%d,crop1 = %d,%d,crop2 = %d,%d", cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft,
	  cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright
	  , cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft, cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright
	  , cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft, cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright);
	return CPE_FW_OK;
}

static int mcf_update_cvdr_rd_cfg_tab(msg_req_mcf_request_t *mcf_request, cfg_tab_cvdr_t *cfg_tab,
									  isp_stripe_info_t *stripe_info, isp_stripe_info_t *stripe_info_DS4, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	loge_if(memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t)));
	pix_format_e format = PIXEL_FMT_CPE_MAX;
	cfmt.pix_fmt = DF_FMT_INVALID;
	unsigned int mcf_stride = 0;

	if (mcf_request->streams[BI_WARP_0_UVC].buf) {
		cfmt.id = CPE_RD0;
		cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.height = mcf_request->streams[BI_WARP_0_UVC].height;
		cfmt.addr = mcf_request->streams[BI_WARP_0_UVC].buf + stripe_info_DS4->stripe_start_point[stripe_index] * 3;
		cfmt.full_width = stripe_info_DS4->full_size.width;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_3PF8;
		mcf_stride = mcf_request->streams[BI_WARP_0_UVC].stride;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BI_WARP_1_UVC].buf) {
		cfmt.id = CPE_RD1;
		cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.height = mcf_request->streams[BI_WARP_1_UVC].height;
		cfmt.addr = mcf_request->streams[BI_WARP_1_UVC].buf + stripe_info_DS4->stripe_start_point[stripe_index] * 3;
		cfmt.full_width = stripe_info_DS4->full_size.width;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_3PF8;
		mcf_stride = mcf_request->streams[BI_WARP_1_UVC].stride;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BI_WARP_2_UVC].buf) {
		cfmt.id = CPE_RD2;
		cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.height = mcf_request->streams[BI_WARP_2_UVC].height;
		cfmt.addr = mcf_request->streams[BI_WARP_2_UVC].buf + stripe_info_DS4->stripe_start_point[stripe_index] * 3;
		cfmt.full_width = stripe_info_DS4->full_size.width;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_3PF8;
		mcf_stride = mcf_request->streams[BI_WARP_2_UVC].stride;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BI_MONO_DS4].buf) {
		cfmt.id = CPE_RD3;
		cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
		cfmt.height = mcf_request->streams[BI_MONO_DS4].height;
		cfmt.addr = mcf_request->streams[BI_MONO_DS4].buf + stripe_info_DS4->stripe_start_point[stripe_index];
		cfmt.full_width = stripe_info_DS4->full_size.width;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_Y8;
		mcf_stride = mcf_request->streams[BI_MONO_DS4].stride;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BI_MONO].buf) {
		cfmt.id = CPE_RD4;
		cfmt.width = stripe_info->stripe_width[stripe_index];
		cfmt.line_size = stripe_info->stripe_width[stripe_index];
		cfmt.height = mcf_request->streams[BI_MONO].height;
		cfmt.addr = mcf_request->streams[BI_MONO].buf + stripe_info->stripe_start_point[stripe_index];
		cfmt.full_width = stripe_info->full_size.width;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_Y8;
		mcf_stride = mcf_request->streams[BI_MONO].stride;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BI_WARP_COL_Y_DS4].buf) {
		cfmt.id = CPE_RD5;

		if (mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE) {
			cfmt.width = stripe_info->stripe_width[stripe_index];
			cfmt.line_size = stripe_info->stripe_width[stripe_index];
			cfmt.height = mcf_request->streams[BI_WARP_COL_Y_DS4].height;
			cfmt.addr = mcf_request->streams[BI_WARP_COL_Y_DS4].buf + stripe_info->stripe_start_point[stripe_index];
			cfmt.full_width = stripe_info->full_size.width;
			cfmt.expand = 0;
			format = PIXEL_FMT_CPE_Y8;
			mcf_stride = mcf_request->streams[BI_WARP_COL_Y_DS4].stride;
		} else {
			cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
			cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
			cfmt.height = mcf_request->streams[BI_WARP_COL_Y_DS4].height;
			cfmt.addr = mcf_request->streams[BI_WARP_COL_Y_DS4].buf + stripe_info_DS4->stripe_start_point[stripe_index];
			cfmt.full_width = stripe_info_DS4->full_size.width;
			cfmt.expand = 0;
			format = PIXEL_FMT_CPE_Y8;
			mcf_stride = mcf_request->streams[BI_WARP_COL_Y_DS4].stride;
		}

		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	return CPE_FW_OK;
}

static int mcf_update_cvdr_wr_cfg_tab(msg_req_mcf_request_t *mcf_request, cfg_tab_cvdr_t *cfg_tab,
									  isp_stripe_info_t *stripe_info, isp_stripe_info_t *stripe_info_DS4, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	loge_if(memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t)));
	pix_format_e format = PIXEL_FMT_CPE_MAX;
	cfmt.pix_fmt = DF_FMT_INVALID;
	unsigned int mcf_stride = 0;

	if (mcf_request->streams[BO_RESULT_Y].buf) {
		cfmt.id = CPE_WR1;
		cfmt.width = stripe_info->stripe_width[stripe_index] - stripe_info->overlap_left[stripe_index] -
					 stripe_info->overlap_right[stripe_index];
		cfmt.line_size = stripe_info->stripe_width[stripe_index] - stripe_info->overlap_left[stripe_index] -
						 stripe_info->overlap_right[stripe_index];
		cfmt.height = mcf_request->streams[BO_RESULT_Y].height;
		cfmt.full_width = stripe_info->full_size.width;
		cfmt.pix_fmt = DF_FMT_INVALID;
		cfmt.expand = 0;
		mcf_stride = mcf_request->streams[BO_RESULT_Y].stride;

		if (mcf_request->mcf_cfg.mode == MCF_FULL_MODE) {
			format = PIXEL_FMT_CPE_Y8;
			cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] +
						stripe_info->overlap_left[stripe_index]);
		} else if (mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE) {
			format = PIXEL_FMT_CPE_Y8;
			cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] +
						stripe_info->overlap_left[stripe_index]);
		} else if (mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE) {
			if (mcf_request->u.y_lf_gf_mode_param.output_mode == A_B) {
				format = PIXEL_FMT_CPE_2PF8;
				cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] +
							stripe_info->overlap_left[stripe_index]) * 2;
			} else {
				format = PIXEL_FMT_CPE_1PF8;
				cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] +
							stripe_info->overlap_left[stripe_index]);
			}
		}

		loge_if_ret(dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BO_RESULT_UV].buf) {
		cfmt.id = CPE_WR0;
		cfmt.height = mcf_request->streams[BO_RESULT_UV].height;
		cfmt.expand = 0;
		mcf_stride = mcf_request->streams[BO_RESULT_UV].stride;

		if (mcf_request->mcf_cfg.mode == MCF_FULL_MODE) {
			format = PIXEL_FMT_CPE_2PF8;
			cfmt.width = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_left[stripe_index] -
						 stripe_info_DS4->overlap_right[stripe_index];
			cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_left[stripe_index] -
							 stripe_info_DS4->overlap_right[stripe_index];
			cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info_DS4->stripe_start_point[stripe_index] +
						stripe_info_DS4->overlap_left[stripe_index]) * 2;
			cfmt.full_width = stripe_info_DS4->full_size.width;
		} else if (mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE) {
			format = PIXEL_FMT_CPE_2PF8;
			cfmt.width = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_left[stripe_index] -
						 stripe_info_DS4->overlap_right[stripe_index];
			cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_left[stripe_index] -
							 stripe_info_DS4->overlap_right[stripe_index];
			cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info_DS4->stripe_start_point[stripe_index] +
						stripe_info_DS4->overlap_left[stripe_index]) * 2;
			cfmt.full_width = stripe_info_DS4->full_size.width;
		} else if (mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE) {
			cfmt.width = stripe_info->stripe_width[stripe_index] - stripe_info->overlap_left[stripe_index] -
						 stripe_info->overlap_right[stripe_index];
			cfmt.full_width = stripe_info->full_size.width;
			cfmt.line_size = stripe_info->stripe_width[stripe_index] - stripe_info->overlap_left[stripe_index] -
							 stripe_info->overlap_right[stripe_index];

			if (mcf_request->u.y_lf_gf_mode_param.output_mode == A_B) {
				format = PIXEL_FMT_CPE_2PF8;
				cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info->stripe_start_point[stripe_index] +
							stripe_info->overlap_left[stripe_index]) * 2;
			} else {
				format = PIXEL_FMT_CPE_1PF8;
				cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info->stripe_start_point[stripe_index] +
							stripe_info->overlap_left[stripe_index]);
			}
		}

		loge_if_ret(dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	if (mcf_request->streams[BO_RESULT_C].buf) {
		cfmt.id = CPE_WR2;
		cfmt.width = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_left[stripe_index] -
					 stripe_info_DS4->overlap_right[stripe_index];
		cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_left[stripe_index] -
						 stripe_info_DS4->overlap_right[stripe_index];
		cfmt.height = mcf_request->streams[BO_RESULT_C].height;
		cfmt.addr = mcf_request->streams[BO_RESULT_C].buf + (stripe_info_DS4->stripe_start_point[stripe_index] +
					stripe_info_DS4->overlap_left[stripe_index]);
		cfmt.full_width = stripe_info_DS4->full_size.width;
		cfmt.pix_fmt = DF_FMT_INVALID;
		cfmt.expand = 0;
		mcf_stride = mcf_request->streams[BO_RESULT_C].stride;

		if (mcf_request->mcf_cfg.mode == MCF_FULL_MODE)
			format = PIXEL_FMT_CPE_Y8;

		loge_if_ret(dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, mcf_stride, CPE_CLK, format));
	}

	return CPE_FW_OK;
}

static int mcf_update_mcf_cfg_tab(msg_req_mcf_request_t *mcf_request, mcf_config_table_t *cfg_tab,
								  unsigned int stripe_width)
{

	switch (mcf_request->mcf_cfg.mode) {
	case MCF_Y_GF_MODE: {
		y_gf_mode_t *para = (y_gf_mode_t *) & (mcf_request->u.y_lf_gf_mode_param);
		cfg_tab->to_use = 1;
		cfg_tab->basic_cfg.uv_pack = mcf_request->mcf_cfg.uv_pack;
		cfg_tab->basic_cfg.op_mode = mcf_request->mcf_cfg.mode;
		cfg_tab->basic_cfg.y_gf_mode.to_use = 1;
		cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_in = para->input_mode;
		cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_out = para->output_mode;
		cfg_tab->size_cfg.width = stripe_width;
		cfg_tab->size_cfg.height = mcf_request->streams[BI_MONO].height;
		cfg_tab->adjust_y.to_use = 1;
		cfg_tab->adjust_y.adjust_col_gf.to_use = 1;
		cfg_tab->adjust_y.adjust_col_gf.y_gf_eps = para->adjust_y.adjust_col_gf.y_gf_eps;
		cfg_tab->adjust_y.adjust_col_gf.y_gf_radius = para->adjust_y.adjust_col_gf.y_gf_radius;
		cfg_tab->adjust_y.adjust_get_mask.to_use = 0;
		cfg_tab->adjust_y.adjust_get_ratio.to_use = 0;
		cfg_tab->mcf_uv.to_use = 0;
		cfg_tab->uv_fusion.to_use = 0;
		break;
	}

	default:
		break;
	}

	return CPE_FW_OK;
}

static int mcf_set_irq_mode(msg_req_mcf_request_t *mcf_request, cmdlst_para_t *cmdlst_para, unsigned int stripe_cnt)
{
	unsigned int irq_mode = 0;
	unsigned int i = 0;

	if (mcf_request->mcf_cfg.mode == MCF_FULL_MODE) {
		full_mode_t *para = (full_mode_t *) & (mcf_request->u.full_mode_param);

		if (para->uv_fusion.get_can.input_cnt == 0)
			irq_mode = 0xffff;
	} else if (mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE) {
		y_gf_mode_t *para = (y_gf_mode_t *) & (mcf_request->u.y_lf_gf_mode_param);

		if (para->input_mode == SINGLE_INPUT) {
			irq_mode = 0x03FFE37C;

			if (para->output_mode == LF_ONLY)
				irq_mode = 0x03FFEB7D;
		} else if (para->input_mode == DUAL_INPUT) {
			irq_mode = 0x03FFE27C;

			if (para->output_mode == LF_ONLY)
				irq_mode = 0x03FFEA7D;
		}
	}

	for (i = 0; i < stripe_cnt; i++)
		cmdlst_para->cmd_stripe_info[i].irq_mode = irq_mode;

	return 0;
}

int mcf_request_handler(msg_req_mcf_request_t *mcf_request)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = CPE_FW_OK;

	if (mcf_request == NULL) {
		E("[%s] failed : mcf_request is null", __func__);
		return -1;
	}

	mcf_dump_request(mcf_request);
	hispcpe_reg_set(CPE_TOP, SUB_CTRL_CPE_MODE_CFG_REG, 0);//SET TOP MODE = MCF
	unsigned int mode = (mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE) ? 1 : 0;
	isp_stripe_info_t *stripe_info = NULL;
	isp_stripe_info_t *stripe_info_DS4 = NULL;
	stripe_info = (isp_stripe_info_t *)vmalloc(sizeof(isp_stripe_info_t));

	if (NULL == stripe_info) {
		E(" fail to vmalloc stripe_info\n");
		return -1;
	}

	loge_if(memset_s(stripe_info, sizeof(isp_stripe_info_t), 0, sizeof(isp_stripe_info_t)));
	stripe_info_DS4 = (isp_stripe_info_t *)vmalloc(sizeof(isp_stripe_info_t));

	if (NULL == stripe_info_DS4) {
		E(" fail to vmalloc stripe_info_DS4\n");
		vfree(stripe_info);
		stripe_info = NULL;
		return -1;
	}

	loge_if(memset_s(stripe_info_DS4, sizeof(isp_stripe_info_t), 0, sizeof(isp_stripe_info_t)));
	mcf_calc_stripe_info(mode, &mcf_request->streams[BI_MONO], stripe_info, stripe_info_DS4);
	unsigned int stripe_cnt = stripe_info->stripe_cnt;

	if ((ret = cpe_mem_get(MEM_ID_MCF_CFG_TAB, &va, &da)) != 0) {
		E("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_MCF_CFG_TAB);
		goto fail_get_mem;
	}

	mcf_config_table_t *mcf_cfg_tab = (mcf_config_table_t *)(uintptr_t)va;

	if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_MM, &va, &da)) != 0) {
		E("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_MM);
		goto fail_get_mem;
	}

	cfg_tab_cvdr_t *cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)va;

	if ((ret = cpe_mem_get(MEM_ID_CPETOP_CFG_TAB_MM, &va, &da)) != 0) {
		E("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CPETOP_CFG_TAB_MM);
		goto fail_get_mem;
	}

	cpe_top_config_table_t *top_cfg_tab = (cpe_top_config_table_t *)(uintptr_t)va;

	if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_MM, &va, &da)) != 0) {
		E("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_MM);
		goto fail_get_mem;
	}

	cmdlst_para_t *cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	unsigned int i = 0;

	for (i = 0; i < stripe_cnt; i++) {
		mcf_update_mcf_cfg_tab(mcf_request, &mcf_cfg_tab[i], stripe_info->stripe_width[i]);
		mcf_update_cvdr_wr_cfg_tab(mcf_request, &cvdr_cfg_tab[i], stripe_info, stripe_info_DS4, i);
		mcf_update_cvdr_rd_cfg_tab(mcf_request, &cvdr_cfg_tab[i], stripe_info, stripe_info_DS4, i);
		mcf_update_top_cfg_tab(mcf_request, &top_cfg_tab[i], stripe_info, stripe_info_DS4, i);
	}

	vfree(stripe_info);
	stripe_info = NULL;
	vfree(stripe_info_DS4);
	stripe_info_DS4 = NULL;

	cmdlst_para->channel_id = CPE_CHANNEL;
	cmdlst_para->stripe_cnt = stripe_cnt;
	cpe_set_cmdlst_stripe(&cmdlst_para->cmd_stripe_info[0], cmdlst_para->stripe_cnt);
	df_sched_prepare(cmdlst_para);
	schedule_cmdlst_link_t *cmd_link_entry = cmdlst_para->cmd_entry;
	mcf_set_irq_mode(mcf_request, cmdlst_para, stripe_cnt);
	df_sched_set_buffer_header(cmdlst_para);

	for (i = 0; i < stripe_cnt; i++) {
		mcf_prepare_cmd(&g_mcf_devs[0], &cmd_link_entry[i].cmd_buf, &mcf_cfg_tab[i]);
		cpe_top_prepare_cmd(&g_cpe_top_devs[0], &cmd_link_entry[i].cmd_buf, &top_cfg_tab[i]);
		cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &cvdr_cfg_tab[i]);
		cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
	}

	ipp_update_cmdlst_cfg_tab(cmdlst_para);
	loge_if_ret(df_sched_start(cmdlst_para));
	cpe_mem_free(MEM_ID_MCF_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_MM);
	cpe_mem_free(MEM_ID_CPETOP_CFG_TAB_MM);
	return ret;
fail_get_mem:
	vfree(stripe_info);
	stripe_info = NULL;
	vfree(stripe_info_DS4);
	stripe_info_DS4 = NULL;
	return ret;
}

static int mcf_dump_request(msg_req_mcf_request_t *req)
{
#if FLAG_LOG_DEBUG
	mcf_uv_t *mcf_uv_tab = NULL;
	adjust_y_t *adjust_y_tab = NULL;
	int i = 0;
	I("size_of_mcf_req = %d\n", (int)(sizeof(msg_req_mcf_request_t)));
	I("frame_number = %d, uv_pack = %d\n", req->frame_number, req->mcf_cfg.uv_pack);

	for (i = 4; i < 6; i++) {
		I("streams[%d].width = %d,height = %d,stride = %d,buf = 0x%08x,format = %d\n",
		  i, req->streams[i].width, req->streams[i].height, req->streams[i].stride, req->streams[i].buf, req->streams[i].format);
	}

	for (i = 8; i < 9; i++) {
		I("streams[%d].width = %d,height = %d,stride = %d,buf = 0x%08x,format = %d\n",
		  i, req->streams[i].width, req->streams[i].height, req->streams[i].stride, req->streams[i].buf, req->streams[i].format);
	}

	if (req->mcf_cfg.mode == MCF_FULL_MODE) {
		D("MCF FULL MODE DUMP: \n");
		uv_fusion_t *uv_fusion_tab = &req->u.full_mode_param.uv_fusion;
		mcf_uv_tab = &req->u.full_mode_param.mcf_uv_cfg;
		adjust_y_tab = &req->u.full_mode_param.adjust_y;
		D("[uv_fusion][get_can_t]th_u = %d,th_v = %d, const_value = %d, input_cnt = %d\n",
		  uv_fusion_tab->get_can.th_u, uv_fusion_tab->get_can.th_v,
		  uv_fusion_tab->get_can.const_value, uv_fusion_tab->get_can.input_cnt);
		D("[uv_fusion][get_final_uv]ratio_dist_uv_to_conf = %d, ratio_dist_to_conf = %d, max_u_diff_const_inv = %d, max_v_diff_const_inv = %d\n",
		  uv_fusion_tab->get_final_uv.ratio_dist_uv_to_conf, uv_fusion_tab->get_final_uv.ratio_dist_to_conf,
		  uv_fusion_tab->get_final_uv.max_u_diff_const_inv, uv_fusion_tab->get_final_uv.max_v_diff_const_inv);
		D("[uv_fusion][get_con]hybrid_diff_hue_shift = %d, hybrid_diff_hue_coff = %d, hybrid_diff_hue_shift_ex = %d, max_diff = %d, max_diff_inv = %d\n",
		  uv_fusion_tab->get_con.hybrid_diff_hue_shift, uv_fusion_tab->get_con.hybrid_diff_hue_coff,
		  uv_fusion_tab->get_con.hybrid_diff_hue_shift_ex, uv_fusion_tab->get_con.max_diff,
		  uv_fusion_tab->get_con.max_diff_inv);
		D("[mcf_uv_cfg][fill_conflict_cfg]luma_th = %d, conflict_th = %d\n",
		  uv_fusion_tab->fill_conflict_cfg.luma_th, uv_fusion_tab->fill_conflict_cfg.conflict_th);
		D("[mcf_uv_cfg][fill_conflict_cfg]gauss_radius = %d, mask_blend = %d\n",
		  mcf_uv_tab->smooth_blend_cfg.gauss_radius, mcf_uv_tab->smooth_blend_cfg.mask_blend);
		D("[mcf_uv_cfg][smooth_blend_gf]uv_gf_radius = %d, uv_gf_eps = %d\n",
		  mcf_uv_tab->smooth_blend_gf.uv_gf_radius, mcf_uv_tab->smooth_blend_gf.uv_gf_eps);
		D("[adjust_y][adjust_col_gf]y_gf_radius = %d, y_gf_eps = %d\n",
		  adjust_y_tab->adjust_col_gf.y_gf_radius, adjust_y_tab->adjust_col_gf.y_gf_eps);
		D("[adjust_y][adjust_get_mask]mask_sub = %d, mask_mult = %d\n",
		  mcf_uv_tab->adjust_get_mask.mask_sub, mcf_uv_tab->adjust_get_mask.mask_mult);
		D("[adjust_y][adjust_get_ratio]ratio_par_down = %d, ratio_par_up = %d\n",
		  mcf_uv_tab->adjust_get_ratio.ratio_par_down, mcf_uv_tab->adjust_get_ratio.ratio_par_up);
	} else if (req->mcf_cfg.mode == MCF_PARTIAL_MODE) {
		D("MCF PARTIAL MODE DUMP: \n");
		mcf_uv_tab = &req->u.partial_mode_param.mcf_uv_cfg;
		adjust_y_tab = &req->u.partial_mode_param.adjust_y;
		D("[mcf_uv_cfg][fill_conflict_cfg]gauss_radius = %d, mask_blend = %d\n",
		  mcf_uv_tab->smooth_blend_cfg.gauss_radius, mcf_uv_tab->smooth_blend_cfg.mask_blend);
		D("[mcf_uv_cfg][smooth_blend_gf]uv_gf_radius = %d, uv_gf_eps = %d\n",
		  mcf_uv_tab->smooth_blend_gf.uv_gf_radius, mcf_uv_tab->smooth_blend_gf.uv_gf_eps);
		D("[adjust_y][adjust_col_gf]y_gf_radius = %d, y_gf_eps = %d\n",
		  adjust_y_tab->adjust_col_gf.y_gf_radius, adjust_y_tab->adjust_col_gf.y_gf_eps);
		D("[adjust_y][adjust_get_mask]mask_sub = %d, mask_mult = %d\n",
		  mcf_uv_tab->adjust_get_mask.mask_sub, mcf_uv_tab->adjust_get_mask.mask_mult);
		D("[adjust_y][adjust_get_ratio]ratio_par_down = %d, ratio_par_up = %d\n",
		  mcf_uv_tab->adjust_get_ratio.ratio_par_down, mcf_uv_tab->adjust_get_ratio.ratio_par_up);
	} else if (req->mcf_cfg.mode == MCF_Y_GF_MODE) {
		adjust_y_tab = &req->u.y_lf_gf_mode_param.adjust_y;
		D("MCF YGF MODE DUMP: \n");
		D("input mode = %d, out mode = %d\n",
		  req->u.y_lf_gf_mode_param.input_mode, req->u.y_lf_gf_mode_param.output_mode);
		D("[adjust_y][adjust_col_gf]y_gf_radius = %d, y_gf_eps = %d\n",
		  adjust_y_tab->adjust_col_gf.y_gf_radius, adjust_y_tab->adjust_col_gf.y_gf_eps);
	}
#else
	I("streams[4].width = %d,height = %d,stride = %d,format = %d\n",
		req->streams[4].width, req->streams[4].height, req->streams[4].stride, req->streams[4].format);

#endif

	return CPE_FW_OK;
}

static int mcf_calc_stripe_info(unsigned int is_ygf_mode, cpe_stream_t *stream, isp_stripe_info_t *p_stripe_info,
								isp_stripe_info_t *p_stripe_info_DS4)
{
	unsigned int i            = 0;
	unsigned int overlap      = is_ygf_mode ? 16 : 32;
	df_size_constrain_t  p_size_constrain;
	unsigned int align_width = 0;
	unsigned int constrain_cnt = 1;
	p_size_constrain.hinc      = 1 * 65536;
	p_size_constrain.pix_align = is_ygf_mode ? 16 : 32;
	p_size_constrain.out_width = 8912;
	align_width = ALIGN_UP(stream->width, ALIGN_MCF_WIDTH);
	df_size_split_stripe(constrain_cnt, &p_size_constrain, p_stripe_info, overlap, align_width);
	p_stripe_info->full_size.width = stream->stride;
	p_stripe_info->full_size.height = stream->height;
	df_size_dump_stripe_info(p_stripe_info, "org_stripe");

	if (0 == is_ygf_mode) {
		p_stripe_info_DS4->stripe_cnt = p_stripe_info->stripe_cnt;
		p_stripe_info_DS4->full_size.width = stream->stride / 2;
		p_stripe_info_DS4->full_size.height = stream->height / 2;

		for (i = 0; i < p_stripe_info->stripe_cnt; i++) {
			p_stripe_info_DS4->stripe_width[i]       = p_stripe_info->stripe_width[i] / 2;
			p_stripe_info_DS4->stripe_start_point[i] = p_stripe_info->stripe_start_point[i] / 2;
			p_stripe_info_DS4->stripe_end_point[i]   = p_stripe_info->stripe_end_point[i] / 2;
			p_stripe_info_DS4->overlap_left[i]       = p_stripe_info->overlap_left[i] / 2;
			p_stripe_info_DS4->overlap_right[i]      = p_stripe_info->overlap_right[i] / 2;
		}

		df_size_dump_stripe_info(p_stripe_info_DS4, "ds_4_stripe");
	}

	return CPE_FW_OK;
}

/*lint +e429  +e613  +e668*/
#pragma GCC diagnostic pop
