/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "hisi_fb.h"
#include "hisi_spr_dsc.h"

#define SPR_LUT_LEN (65)
#define GEN_RGB_COEFF(a, b, c, d, e) ((a << 24) | (b << 18) | (c << 12) | (d << 6) | e)

static uint32_t g_spr_gama_degama_table[] = {
	/* gama_r */
	8, 296, 549, 738, 895, 1032, 1154, 1265, 1368, 1464, 1555, 1640, 1721,
	1799, 1874, 1945, 2014, 2081, 2146, 2208, 2269, 2328, 2386, 2442, 2497,
	2551, 2603, 2655, 2705, 2754, 2803, 2850, 2897, 2943, 2988, 3033, 3076,
	3120, 3162, 3204, 3245, 3286, 3326, 3366, 3405, 3443, 3482, 3519, 3557,
	3593, 3630, 3666, 3702, 3737, 3772, 3806, 3841, 3874, 3908, 3941, 3974,
	4007, 4039, 4071, 4095,
	/* gama_g */
	8, 296, 549, 738, 895, 1032, 1154, 1265, 1368, 1464, 1555, 1640, 1721,
	1799, 1874, 1945, 2014, 2081, 2146, 2208, 2269, 2328, 2386, 2442, 2497,
	2551, 2603, 2655, 2705, 2754, 2803, 2850, 2897, 2943, 2988, 3033, 3076,
	3120, 3162, 3204, 3245, 3286, 3326, 3366, 3405, 3443, 3482, 3519, 3557,
	3593, 3630, 3666, 3702, 3737, 3772, 3806, 3841, 3874, 3908, 3941, 3974,
	4007, 4039, 4071, 4095,
	/* gama_b */
	8, 296, 549, 738, 895, 1032, 1154, 1265, 1368, 1464, 1555, 1640, 1721,
	1799, 1874, 1945, 2014, 2081, 2146, 2208, 2269, 2328, 2386, 2442, 2497,
	2551, 2603, 2655, 2705, 2754, 2803, 2850, 2897, 2943, 2988, 3033, 3076,
	3120, 3162, 3204, 3245, 3286, 3326, 3366, 3405, 3443, 3482, 3519, 3557,
	3593, 3630, 3666, 3702, 3737, 3772, 3806, 3841, 3874, 3908, 3941, 3974,
	4007, 4039, 4071, 4095,

	/* degama_r */
	0, 14, 28, 43, 57, 71, 86, 102, 119, 139, 160, 182, 206, 232, 260, 289,
	320, 353, 387, 423, 462, 501, 543, 587, 632, 679, 728, 779, 832, 887,
	944, 1002, 1063, 1126, 1190, 1257, 1325, 1396, 1468, 1543, 1620, 1698,
	1779, 1862, 1947, 2034, 2123, 2214, 2308, 2403, 2501, 2600, 2702, 2806,
	2912, 3021, 3131, 3244, 3359, 3476, 3595, 3717, 3841, 3967, 4095,
	/* degama_g */
	0, 14, 28, 43, 57, 71, 86, 102, 119, 139, 160, 182, 206, 232, 260, 289,
	320, 353, 387, 423, 462, 501, 543, 587, 632, 679, 728, 779, 832, 887,
	944, 1002, 1063, 1126, 1190, 1257, 1325, 1396, 1468, 1543, 1620, 1698,
	1779, 1862, 1947, 2034, 2123, 2214, 2308, 2403, 2501, 2600, 2702, 2806,
	2912, 3021, 3131, 3244, 3359, 3476, 3595, 3717, 3841, 3967, 4095,
	/* degama_b */
	0, 14, 28, 43, 57, 71, 86, 102, 119, 139, 160, 182, 206, 232, 260, 289,
	320, 353, 387, 423, 462, 501, 543, 587, 632, 679, 728, 779, 832, 887,
	944, 1002, 1063, 1126, 1190, 1257, 1325, 1396, 1468, 1543, 1620, 1698,
	1779, 1862, 1947, 2034, 2123, 2214, 2308, 2403, 2501, 2600, 2702, 2806,
	2912, 3021, 3131, 3244, 3359, 3476, 3595, 3717, 3841, 3967, 4095,
};

static void spr_ctrl_config(struct spr_dsc_panel_para *spr, struct hisi_panel_info *pinfo)
{
	/* spr global init */
	spr->spr_rgbg2uyvy_8biten = 0;
	spr->spr_hpartial_mode = 0;
	spr->spr_partial_mode = 0;
	spr->spr_rgbg2uyvy_en = 0;
	spr->spr_horzborderdect = 0;
	spr->spr_linebuf_1dmode = 0;
	spr->spr_bordertb_dummymode = 0;
	spr->spr_borderlr_dummymode = 3;
	spr->spr_pattern_mode = 1;
	spr->spr_pattern_en = 0;
	spr->ck_gt_spr_en = 1;
	spr->spr_en = 1;
	spr->input_reso = ((pinfo->yres - 1) << 16) | (pinfo->xres - 1);
}

static void spr_rgb_coef_config(struct spr_dsc_panel_para *spr)
{
	/* config Red's v0h0 v0h1 v1h0 v1h1 coefficients */
	spr->spr_r_v0h0_coef[0] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_r_v0h0_coef[1] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_r_v0h0_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_r_v0h1_coef[0] = GEN_RGB_COEFF(0, 0, 8, 8, 0);
	spr->spr_r_v0h1_coef[1] = GEN_RGB_COEFF(0, 0, 8, 8, 0);
	spr->spr_r_v0h1_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_r_v1h0_coef[0] = GEN_RGB_COEFF(0, 0, 8, 8, 0);
	spr->spr_r_v1h0_coef[1] = GEN_RGB_COEFF(0, 0, 8, 8, 0);
	spr->spr_r_v1h0_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_r_v1h1_coef[0] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_r_v1h1_coef[1] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_r_v1h1_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	/* config Green's v0h0 v0h1 v1h0 v1h1 coefficients */
	spr->spr_g_v0h0_coef[0] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_g_v0h0_coef[1] = GEN_RGB_COEFF(0, 12, 12, 0, 0);
	spr->spr_g_v0h0_coef[2] = GEN_RGB_COEFF(0, 4, 4, 0, 0);

	spr->spr_g_v0h1_coef[0] = GEN_RGB_COEFF(0, 4, 4, 0, 0);
	spr->spr_g_v0h1_coef[1] = GEN_RGB_COEFF(0, 12, 12, 0, 0);
	spr->spr_g_v0h1_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_g_v1h0_coef[0] = GEN_RGB_COEFF(0, 4, 4, 0, 0);
	spr->spr_g_v1h0_coef[1] = GEN_RGB_COEFF(0, 12, 12, 0, 0);
	spr->spr_g_v1h0_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_g_v1h1_coef[0] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_g_v1h1_coef[1] = GEN_RGB_COEFF(0, 12, 12, 0, 0);
	spr->spr_g_v1h1_coef[2] = GEN_RGB_COEFF(0, 4, 4, 0, 0);

	/* config Blue's v0h0 v0h1 v1h0 v1h1 coefficients */
	spr->spr_b_v0h0_coef[0] = GEN_RGB_COEFF(0, 8, 8, 0, 0);
	spr->spr_b_v0h0_coef[1] = GEN_RGB_COEFF(0, 8, 8, 0, 0);
	spr->spr_b_v0h0_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_b_v0h1_coef[0] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_b_v0h1_coef[1] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_b_v0h1_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_b_v1h0_coef[0] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_b_v1h0_coef[1] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
	spr->spr_b_v1h0_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);

	spr->spr_b_v1h1_coef[0] = GEN_RGB_COEFF(0, 8, 8, 0, 0);
	spr->spr_b_v1h1_coef[1] = GEN_RGB_COEFF(0, 8, 8, 0, 0);
	spr->spr_b_v1h1_coef[2] = GEN_RGB_COEFF(0, 0, 0, 0, 0);
}

static void spr_rgb_border_config(struct spr_dsc_panel_para *spr, struct hisi_panel_info *pinfo)
{
	/* config RGB's border detect gain parameters */
	spr->spr_borderlr_detect_r = (0x0 << 24) | (0x80 << 16) | (0x0 << 8)
		| 0x80;
	spr->spr_bordertb_detect_r = (0x0 << 24) | (0x80 << 16) | (0x0 << 8)
		| 0x80;
	spr->spr_borderlr_detect_g = (0x0 << 24) | (0x20 << 16) | (0x0 << 8)
		| 0xC0;
	spr->spr_bordertb_detect_g = (0x0 << 24) | (0x80 << 16) | (0x0 << 8)
		| 0x80;
	spr->spr_borderlr_detect_b = (0x0 << 24) | (0x80 << 16) | (0x0 << 8)
		| 0x80;
	spr->spr_bordertb_detect_b = (0x0 << 24) | (0x80 << 16) | (0x0 << 8)
		| 0x80;

	/* config boarder region */
	spr->spr_borderl_position = (1 << 12) | 0x0;
	spr->spr_borderr_position = (pinfo->xres << 12) | (pinfo->xres - 1);
	spr->spr_bordert_position = 0x0;
	spr->spr_borderb_position = 0x0;
}

static void spr_rgb_diff_weight_gain_config(struct spr_dsc_panel_para *spr)
{
	/* config RGB's diff ,weight regs */
	spr->spr_r_diff = (64 << 20) | (64 << 12) | (32 << 4) | 4;
	spr->spr_r_weight = ((uint64_t)128 << (24 + 32))
		| ((uint64_t)64 << (16 + 32)) | ((uint64_t)128 << (8 + 32))
		| ((uint64_t)64 << 32) | ((uint64_t)1 << 16)
		| ((uint64_t)0 << 15) | ((uint64_t)1 << 14)
		| ((uint64_t)52 << 8) | (uint64_t)64;

	spr->spr_g_diff = ((uint64_t)64 << 20) | ((uint64_t)64 << 12)
		| ((uint64_t)32 << 4) | 4;
	spr->spr_g_weight = ((uint64_t)128 << (24 + 32))
		| ((uint64_t)64 << (16 + 32)) | ((uint64_t)128 << (8 + 32))
		| ((uint64_t)64 << 32) | ((uint64_t)1 << 16)
		| ((uint64_t)1 << 15) | ((uint64_t)0 << 14)
		| ((uint64_t)56 << 8) | (uint64_t)64;

	spr->spr_b_diff = ((uint64_t)64 << 20) | ((uint64_t)64 << 12)
		| ((uint64_t)32 << 4) | (uint64_t)4;
	spr->spr_b_weight = ((uint64_t)128 << (24 + 32))
		| ((uint64_t)64 << (16 + 32)) | ((uint64_t)128 << (8 + 32))
		| ((uint64_t)64 << 32) | ((uint64_t)1 << 18)
		| ((uint64_t)2 << 16) | ((uint64_t)2 << 14)
		| ((uint64_t)56 << 8) | 64;

	/* config RGB's final gain parameters */
	spr->spr_pixgain = ((uint64_t)0 << (24 + 32))
		| ((uint64_t)128 << (16 + 32)) | ((uint64_t)0 << (8 + 32))
		| ((uint64_t)128 << 32) | ((uint64_t)0x0 << 8) | 0x80;
}

static void spr_rgbg2uyvy_coeff_config(struct spr_dsc_panel_para *spr)
{
	/* config rgbg2uyvy filtering coeff */
	spr->spr_rgbg2uyvy_coeff[0] = (0x0 << 21) | (0x0 << 18) | (0x0 << 15)
		| (0x4 << 12) | (0x0 << 9) | (0x0 << 6) | (0x4 << 3) | 0x0;
	spr->spr_rgbg2uyvy_coeff[1] = (0x4 << 21) | (0x0 << 18) | (0x0 << 15)
		| (0x0 << 12) | (0x0 << 9) | (0x4 << 6) | (0x0 << 3) | 0x0;
	if (spr->spr_rgbg2uyvy_8biten) {
		spr->spr_rgbg2uyvy_coeff[2] = (0x80 << 12) | 0x0;
		spr->spr_rgbg2uyvy_coeff[3] = (0x0 << 12) | 0x80;
	} else {
		spr->spr_rgbg2uyvy_coeff[2] = (0x3ff << 12) | 0x3ff;
		spr->spr_rgbg2uyvy_coeff[3] = (0x3ff << 12) | 0x3ff;
	}
}

static void dsc_ctrl_config(struct spr_dsc_panel_para *spr, struct hisi_panel_info *pinfo)
{
	/* dsc global init */
	spr->dsc_enable = 1;
	spr->rcb_bits = 10908;
	spr->dsc_alg_ctrl = 0x2;
	spr->bits_per_component = 8;
	spr->dsc_sample = 0x1;

	spr->bpp_chk = (810 << 16) | 192;
	spr->pic_reso = (pinfo->yres << 16) | pinfo->xres;

	spr->initial_xmit_delay = 512;
	spr->initial_dec_delay = 397;
	spr->initial_scale_value = 32;
	spr->scale_interval = (2481 << 16) | 7;
	spr->first_bpg = (15 << 16) | 521;
	spr->second_bpg = (0 << 16) | 0;
	spr->second_adj = 0;
	spr->init_finl_ofs = (6144 << 16) | 2336;
	spr->slc_bpg = 443;
	spr->flat_range = (3 << 8) | 12;
	spr->rc_mode_edge = (8192 << 16) | 6;
	spr->rc_qua_tgt = (3 << 20) | (3 << 16) | (11 << 8) | 11;
}

static void dsc_ctrl_config_10bit(struct spr_dsc_panel_para *spr)
{
	/* dsc global init */
	spr->rcb_bits = 14288;
	spr->dsc_alg_ctrl = 0x2;
	spr->bits_per_component = 0xa;
	spr->dsc_sample = 0x1;

	spr->bpp_chk = (1080 << 16) | 256;
	spr->initial_xmit_delay = 341;
	spr->initial_dec_delay = 552;
	spr->initial_scale_value = 10;
	spr->scale_interval = (1198 << 16) | 90;
	spr->first_bpg = (15 << 16) | 521;
	spr->second_bpg = (0 << 16) | 0;
	spr->second_adj = 0;
	spr->init_finl_ofs = (2048 << 16) | 3072;
	spr->slc_bpg = 1229;
	spr->flat_range = (7 << 8) | 16;
	spr->rc_mode_edge = (8192 << 16) | 6;
	spr->rc_qua_tgt = (3 << 20) | (3 << 16) | (15 << 8) | 15;
}

static void dsc_rc_config(struct spr_dsc_panel_para *spr)
{
	/* Specify thresholds in the "RC model" for the 15 ranges */
	spr->rc_buf_thresh[0] = (56 << 24) | (42 << 16) | (28 << 8) | 14;
	spr->rc_buf_thresh[1] = (105 << 24) | (98 << 16) | (84 << 8) | 70;
	spr->rc_buf_thresh[2] = (123 << 24) | (121 << 16) | (119 << 8) | 112;
	spr->rc_buf_thresh[3] = (126 << 8) | 125;

	/* RC params for 15 registers */
	spr->rc_para[0] = (0 << 16) | (4 << 8) | 2;
	spr->rc_para[1] = (0 << 16) | (4 << 8) | 0;
	spr->rc_para[2] = (1 << 16) | (5 << 8) | 0;
	spr->rc_para[3] = (1 << 16) | (6 << 8) | 62;
	spr->rc_para[4] = (3 << 16) | (7 << 8) | 60;
	spr->rc_para[5] = (3 << 16) | (7 << 8) | 58;
	spr->rc_para[6] = (3 << 16) | (7 << 8) | 56;
	spr->rc_para[7] = (3 << 16) | (8 << 8) | 56;
	spr->rc_para[8] = (3 << 16) | (9 << 8) | 56;
	spr->rc_para[9] = (3 << 16) | (10 << 8) | 54;
	spr->rc_para[10] = (5 << 16) | (10 << 8) | 54;
	spr->rc_para[11] = (5 << 16) | (11 << 8) | 52;
	spr->rc_para[12] = (5 << 16) | (11 << 8) | 52;
	spr->rc_para[13] = (9 << 16) | (12 << 8) | 52;
	spr->rc_para[14] = (12 << 16) | (13 << 8) | 52;
}

static void dsc_rc_config_10bit(struct spr_dsc_panel_para *spr)
{
	/* Specify thresholds in the "RC model" for the 15 ranges */
	spr->rc_buf_thresh[0] = (56 << 24) | (42 << 16)
		| (28 << 8) | 14;
	spr->rc_buf_thresh[1] = (105 << 24) | (98 << 16)
		| (84 << 8) | 70;
	spr->rc_buf_thresh[2] = (123 << 24) | (121 << 16)
		| (119 << 8) | 112;
	spr->rc_buf_thresh[3] = (126 << 8) | 125;

	/* RC params for 15 registers */
	spr->rc_para[0] = (0 << 16) | (2 << 8) | 2;
	spr->rc_para[1] = (2 << 16) | (5 << 8) | 0;
	spr->rc_para[2] = (3 << 16) | (7 << 8) | 0;
	spr->rc_para[3] = (4 << 16) | (8 << 8) | 62;
	spr->rc_para[4] = (6 << 16) | (9 << 8) | 60;
	spr->rc_para[5] = (7 << 16) | (10 << 8) | 58;
	spr->rc_para[6] = (7 << 16) | (11 << 8) | 56;
	spr->rc_para[7] = (7 << 16) | (12 << 8) | 56;
	spr->rc_para[8] = (7 << 16) | (12 << 8) | 56;
	spr->rc_para[9] = (7 << 16) | (13 << 8) | 54;
	spr->rc_para[10] = (9 << 16) | (13 << 8) | 54;
	spr->rc_para[11] = (9 << 16) | (13 << 8) | 52;
	spr->rc_para[12] = (9 << 16) | (13 << 8) | 52;
	spr->rc_para[13] = (11 << 16) | (14 << 8) | 52;
	spr->rc_para[14] = (14 << 16) | (15 << 8) | 52;
}

static void spr_coef_offset_config(char __iomem *spr_coef_base,
	uint32_t offset, uint32_t coef[])
{
	/* set one group coefficients using 3 registers */
	outp32(spr_coef_base + offset, coef[0]);
	outp32(spr_coef_base + offset + 4, coef[1]);
	outp32(spr_coef_base + offset + 8, coef[2]);
}

static void spr_init_coeff(char __iomem *spr_base, struct spr_dsc_panel_para *spr)
{
	uint32_t offset = 0;

	/* set Red's v0h0 v0h1 v1h0 v1h1 coefficients */
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_r_v0h0_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_r_v0h1_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_r_v1h0_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_r_v1h1_coef);

	/* set Green's v0h0 v0h1 v1h0 v1h1 coefficients */
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_g_v0h0_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_g_v0h1_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_g_v1h0_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_g_v1h1_coef);

	/* set Blue's v0h0 v0h1 v1h0 v1h1 coefficients */
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_b_v0h0_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_b_v0h1_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_b_v1h0_coef);
	offset += 0xC;
	spr_coef_offset_config(spr_base + SPR_COEFF_OFFSET,
							offset, spr->spr_b_v1h1_coef);

}

static void spr_init(char __iomem *spr_base, struct spr_dsc_panel_para *spr)
{
	outp32(spr_base + SPR_PIX_EVEN_COEF_SEL, spr->spr_coeffsel_even);
	outp32(spr_base + SPR_PIX_ODD_COEF_SEL, spr->spr_coeffsel_odd);
	outp32(spr_base + SPR_PIX_PANEL_ARRANGE_SEL, spr->pix_panel_arrange_sel);

	spr_init_coeff(spr_base, spr);

	/* set RGB's border detect gain parameters */
	outp32(spr_base + SPR_BORDERLR_REG, spr->spr_borderlr_detect_r);
	outp32(spr_base + SPR_BORDERLR_REG + 0x4, spr->spr_bordertb_detect_r);
	outp32(spr_base + SPR_BORDERLR_REG + 0x8, spr->spr_borderlr_detect_g);
	outp32(spr_base + SPR_BORDERLR_REG + 0xC, spr->spr_bordertb_detect_g);
	outp32(spr_base + SPR_BORDERLR_REG + 0x10, spr->spr_borderlr_detect_b);
	outp32(spr_base + SPR_BORDERLR_REG + 0x14, spr->spr_bordertb_detect_b);

	/* set RGB's final gain parameters */
	outp32(spr_base + SPR_PIXGAIN_REG, (spr->spr_pixgain >> 32) & 0xFFFFFFFF);
	outp32(spr_base + SPR_PIXGAIN_REG1, spr->spr_pixgain & 0xFFFFFFFF);

	/* set boarder region */
	outp32(spr_base + SPR_BORDER_POSITION0, spr->spr_borderl_position);
	outp32(spr_base + SPR_BORDER_POSITION0 + 0x4, spr->spr_borderr_position);
	outp32(spr_base + SPR_BORDER_POSITION0 + 0x8, spr->spr_bordert_position);
	outp32(spr_base + SPR_BORDER_POSITION0 + 0xC, spr->spr_borderb_position);

	/* set RGB's diff ,weight regs */
	outp32(spr_base + SPR_R_DIFF_REG, spr->spr_r_diff);
	outp32(spr_base + SPR_R_WEIGHT_REG0, (spr->spr_r_weight >> 32) & 0xFFFFFFFF);
	outp32(spr_base + SPR_R_WEIGHT_REG1, spr->spr_r_weight & 0xFFFFFFFF);
	outp32(spr_base + SPR_R_DIFF_REG + 0xC, spr->spr_g_diff);
	outp32(spr_base + SPR_R_WEIGHT_REG0 + 0xC, (spr->spr_g_weight >> 32) & 0xFFFFFFFF);
	outp32(spr_base + SPR_R_WEIGHT_REG1 + 0xC, spr->spr_g_weight & 0xFFFFFFFF);
	outp32(spr_base + SPR_R_DIFF_REG + 0xC + 0xC, spr->spr_b_diff);
	outp32(spr_base + SPR_R_WEIGHT_REG0 + 0xC + 0xC, (spr->spr_b_weight >> 32) & 0xFFFFFFFF);
	outp32(spr_base + SPR_R_WEIGHT_REG1 + 0xC + 0xC, spr->spr_b_weight & 0xFFFFFFFF);

	outp32(spr_base + SPR_CSC_COEFF0, spr->spr_rgbg2uyvy_coeff[0]);
	outp32(spr_base + SPR_CSC_COEFF1, spr->spr_rgbg2uyvy_coeff[1]);
	outp32(spr_base + SPR_CSC_OFFSET0, spr->spr_rgbg2uyvy_coeff[2]);
	outp32(spr_base + SPR_CSC_OFFSET1, spr->spr_rgbg2uyvy_coeff[3]);
	outp32(spr_base + SPR_RESO, spr->input_reso);
}

static void dsc12_init(char __iomem *spr_base, struct spr_dsc_panel_para *spr)
{
	uint32_t offset;

	outp32(spr_base + DSC1_2_CTRL, (spr->slice1_ck_gt_en << 2) | (spr->slice0_ck_gt_en << 1) | spr->dsc_enable);
	outp32(spr_base + DSC_RC_BITS, spr->rcb_bits);
	outp32(spr_base + DSC_ALG_CTRL, spr->dsc_alg_ctrl);
	outp32(spr_base + DSC_BPC, spr->bits_per_component);
	outp32(spr_base + DSC_SAMPLE, spr->dsc_sample);
	outp32(spr_base + DSC_BPP_CHK, spr->bpp_chk);
	outp32(spr_base + DSC_PIC_RESO, spr->pic_reso);
	outp32(spr_base + DSC_SLC_RESO, spr->slc_reso);

	outp32(spr_base + DSC_INIT_XMIT_DLY, spr->initial_xmit_delay);
	outp32(spr_base + DSC_INIT_DEC_DLY, spr->initial_dec_delay);
	outp32(spr_base + DSC_INIT_SCALE, spr->initial_scale_value);
	outp32(spr_base + DSC_SCALE_INTVAL, spr->scale_interval);
	outp32(spr_base + DSC_FIRST_BPG, spr->first_bpg);
	outp32(spr_base + DSC_SECOND_BPG, spr->second_bpg);
	outp32(spr_base + DSC_SECOND_ADJ, spr->second_adj);
	outp32(spr_base + DSC_INIT_FINL_OFS, spr->init_finl_ofs);
	outp32(spr_base + DSC_SLC_BPG, spr->slc_bpg);
	outp32(spr_base + DSC_FLAT_RANGE, spr->flat_range);
	outp32(spr_base + DSC_RC_MOD_EDGE, spr->rc_mode_edge);
	outp32(spr_base + DSC_RC_QUA_TGT, spr->rc_qua_tgt);

	/* Specify thresholds in the "RC model" for the 15 ranges defined by 14 thresholds */
	for (offset = 0; offset < SPR_RC_BUF_THRESH_LEN; offset++)
		outp32(spr_base + DSC_RC_THRE + (offset << 2), spr->rc_buf_thresh[offset]);

	/* set RC params for 15 registers */
	for (offset = 0; offset < SPR_RC_PARA_LEN; offset++)
		outp32(spr_base + DSC_RC_PARAM + (offset << 2), spr->rc_para[offset]);
}

static void spr_lut_config(char __iomem *spr_base, struct spr_dsc_panel_para *pinfo)
{
	int idx;
	uint32_t lut;
	int gama_len = SPR_LUT_LEN * 3;

	set_reg(spr_base + SPR_CTRL, 0x1, 1, 12);
	for (idx = 0; idx < gama_len; idx++) {
		lut = (pinfo->spr_lut_table[idx] << 16)
			| pinfo->spr_lut_table[idx + gama_len]; //lint !e679
		outp32(spr_base + SPR_LUT_WR_DATA, lut);
	}
	set_reg(spr_base + SPR_CTRL, 0x0, 1, 12);

}

void spr_dsc_para_init(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct spr_dsc_panel_para *spr = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("null hisifd\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		HISI_FB_ERR("null pinfo\n");
		return;
	}

	if (pinfo->spr_dsc_mode == SPR_DSC_MODE_NONE)
		return;

	spr = &(pinfo->spr);

	spr->spr_lut_table = g_spr_gama_degama_table;

	spr_ctrl_config(spr, pinfo);
	spr_rgb_coef_config(spr);
	spr_rgb_border_config(spr, pinfo);
	spr_rgb_diff_weight_gain_config(spr);
	spr_rgbg2uyvy_coeff_config(spr);

	dsc_ctrl_config(spr, pinfo);
	dsc_rc_config(spr);
	if (spr->spr_rgbg2uyvy_8biten == 0) {
		dsc_ctrl_config_10bit(spr);
		dsc_rc_config_10bit(spr);
	}
}

uint32_t get_hsize_after_spr_dsc(struct hisi_fb_data_type *hisifd, uint32_t rect_width)
{
	struct hisi_panel_info *pinfo = NULL;
	struct spr_dsc_panel_para *spr = NULL;
	uint32_t hsize = rect_width;
	uint8_t cpnt_num;
	uint8_t bpc;
	uint8_t bpp;

	if (hisifd == NULL) {
		HISI_FB_ERR("null hisifd\n");
		return hsize;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		HISI_FB_ERR("null pinfo\n");
		return hsize;
	}

	if (pinfo->spr_dsc_mode == SPR_DSC_MODE_NONE)
		return hsize;

	spr = &(pinfo->spr);
	cpnt_num = (spr->dsc_sample & BIT(0)) ? 4 : 3;
	bpc = spr->bits_per_component;
	bpp = (uint8_t)((spr->bpp_chk & 0x3FF) >> 4);

	/*
	* compress_ratio = bpp / (bpc * cpnt_num)
	* bpp = (spr->bpp_chk & 0x3FF)
	* bpc = spr->bits_per_component
	* cpnt_num = 4 (YUV422), 3 (RGB)
	*/
	if (pinfo->spr_dsc_mode == SPR_DSC_MODE_SPR_AND_DSC) {
		if ((bpc != 0) && (cpnt_num  != 0))
			hsize = (rect_width * bpp * 2) / (bpc * cpnt_num * 3);
	} else if (pinfo->spr_dsc_mode == SPR_DSC_MODE_SPR_ONLY) {
		hsize = rect_width * 2 / 3;
	} else if (pinfo->spr_dsc_mode == SPR_DSC_MODE_DSC_ONLY) {
		if ((bpc != 0) && (cpnt_num  != 0))
			hsize = (rect_width * bpp) / (bpc * cpnt_num);
	}

	return hsize;
}

void spr_dsc_init(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *spr_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct spr_dsc_panel_para *spr = NULL;
	uint32_t spr_ctrl_value;

	if (hisifd == NULL)
		return;

	pinfo = &(hisifd->panel_info);
	if ((pinfo == NULL) || (pinfo->spr_dsc_mode == SPR_DSC_MODE_NONE))
		return;

	if (fastboot_enable)
		return;

	HISI_FB_INFO("+\n");
	spr = &(pinfo->spr);
	spr_base = hisifd->dss_base + SPR_OFFSET;

	/* init spr ctrl reg */
	spr_ctrl_value = (spr->spr_rgbg2uyvy_8biten << 17)
		| (spr->spr_hpartial_mode << 15)
		| (spr->spr_partial_mode << 13)
		| (spr->spr_rgbg2uyvy_en << 11)
		| (spr->spr_horzborderdect << 10)
		| (spr->spr_linebuf_1dmode << 9)
		| (spr->spr_bordertb_dummymode << 8)
		| (spr->spr_borderlr_dummymode << 6)
		| (spr->spr_pattern_mode << 4)
		| (spr->spr_pattern_en << 3)
		| (spr->spr_subpxl_layout << 2)
		| (spr->ck_gt_spr_en << 1)
		| spr->spr_en;

	HISI_FB_INFO("spr_ctrl_value = 0x%x\n", spr_ctrl_value);
	outp32(spr_base + SPR_CTRL, spr_ctrl_value);

	if (spr->spr_pattern_en) {
		/* spr pattern generate config */
		outp32(spr_base + SPR_PATTERNGEN_POSITION, (pinfo->xres << 12) | 0x0);
		outp32(spr_base + SPR_PATTERNGEN_POSITION1, (pinfo->yres << 12) | 0x0);
		outp32(spr_base + SPR_PATTERNGEN_PIX0, (0x3fc << 20) | (0x3fc << 10) | 0x3fc);
		outp32(spr_base + SPR_PATTERNGEN_PIX1, (0x200 << 20) | (0x200 << 10) | 0x200);
	}

	spr_init(spr_base, spr);
	if ((pinfo->spr_dsc_mode == SPR_DSC_MODE_SPR_AND_DSC) || (pinfo->spr_dsc_mode == SPR_DSC_MODE_DSC_ONLY))
		dsc12_init(spr_base, spr);
	spr_lut_config(spr_base, spr);

	HISI_FB_INFO("-\n");
}

void spr_dsc_partial_updt_config(struct hisi_fb_data_type *hisifd,
	struct dss_rect dirty, struct dss_rect dirty_overlap)
{
	uint32_t overlap_value;

	if (hisifd->panel_info.spr_dsc_mode == SPR_DSC_MODE_NONE)
		return;

	switch (dirty.h - dirty_overlap.h) {
	case 0:
		overlap_value = 0;
		break;
	case 1:
		/* if overlap dirty.y change ,up boarder overlap 1 line */
		if (dirty.y == dirty_overlap.y)
			overlap_value = 2;
		else
			overlap_value = 1;
		break;
	case 2:
		overlap_value = 3;
		break;
	default:
		HISI_FB_ERR("wrong overlap %d\n", dirty.h - dirty_overlap.h);
		return;
	}

	HISI_FB_INFO("overlap_value = %d\n", overlap_value);

	set_reg(hisifd->dss_base + SPR_OFFSET + SPR_CTRL,
		overlap_value, 2, 15);
	outp32(hisifd->dss_base + SPR_OFFSET + SPR_RESO,
		((dirty.h - 1) << 16) | (dirty.w - 1));
	outp32(hisifd->dss_base + SPR_OFFSET + DSC_PIC_RESO,
		((uint32_t)dirty_overlap.h << 16) | (uint32_t)dirty_overlap.w);
}

