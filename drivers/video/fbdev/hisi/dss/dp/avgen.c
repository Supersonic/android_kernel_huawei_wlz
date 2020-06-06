/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
* Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
*/

#include "avgen.h"
#include "dp_aux.h"
#include "core.h"
#include "../hisi_fb.h"
#include "../hisi_fb_def.h"

#define OFFSET_FRACTIONAL_BITS 11

/*lint -save -e* */
static inline uint8_t dptx_bit_field(const uint16_t data, uint8_t shift, uint8_t width)
{
	return ((data >> shift) & ((((uint16_t)1) << width) - 1));
}

uint16_t dptx_concat_bits(uint8_t bhi, uint8_t ohi, uint8_t nhi, uint8_t blo, uint8_t olo, uint8_t nlo)
{
	return (dptx_bit_field(bhi, ohi, nhi) << nlo) |
		dptx_bit_field(blo, olo, nlo);
}

uint16_t dptx_byte_to_word(const uint8_t hi, const uint8_t lo)
{
	return dptx_concat_bits(hi, 0, 8, lo, 0, 8);
}

uint32_t dptx_byte_to_dword(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
	uint32_t retval = 0;

	retval = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	return retval;
}

int dptx_dtd_parse(struct dp_ctrl *dptx, struct dtd *mdtd, uint8_t data[18])
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	mdtd->pixel_repetition_input = 0;

	mdtd->pixel_clock = dptx_byte_to_word(data[1], data[0]);
	if (mdtd->pixel_clock < 0x01)
		return -EINVAL;

	mdtd->h_active = dptx_concat_bits(data[4], 4, 4, data[2], 0, 8);
	mdtd->h_blanking = dptx_concat_bits(data[4], 0, 4, data[3], 0, 8);
	mdtd->h_sync_offset = dptx_concat_bits(data[11], 6, 2, data[8], 0, 8);
	mdtd->h_sync_pulse_width = dptx_concat_bits(data[11], 4, 2, data[9],
							0, 8);
	mdtd->h_image_size = dptx_concat_bits(data[14], 4, 4, data[12], 0, 8);

	mdtd->v_active = dptx_concat_bits(data[7], 4, 4, data[5], 0, 8);
	mdtd->v_blanking = dptx_concat_bits(data[7], 0, 4, data[6], 0, 8);
	mdtd->v_sync_offset = dptx_concat_bits(data[11], 2, 2, data[10], 4, 4);
	mdtd->v_sync_pulse_width = dptx_concat_bits(data[11], 0, 2,
							data[10], 0, 4);
	mdtd->v_image_size = dptx_concat_bits(data[14], 0, 4, data[13], 0, 8);
	if (dptx_bit_field(data[17], 4, 1) != 1)
		return -EINVAL;
	if (dptx_bit_field(data[17], 3, 1) != 1)
		return -EINVAL;

	mdtd->interlaced = dptx_bit_field(data[17], 7, 1) == 1;
	mdtd->v_sync_polarity = dptx_bit_field(data[17], 2, 1) == 0;
	mdtd->h_sync_polarity = dptx_bit_field(data[17], 1, 1) == 0;
	if (mdtd->interlaced == 1)
		mdtd->v_active /= 2;
	HISI_FB_INFO("[DP] DTD pixel_clock: %llu interlaced: %d\n",
		 mdtd->pixel_clock, mdtd->interlaced);
	HISI_FB_INFO("[DP] h_active: %d h_blanking: %d h_sync_offset: %d\n",
		 mdtd->h_active, mdtd->h_blanking, mdtd->h_sync_offset);
	HISI_FB_INFO("[DP] h_sync_pulse_width: %d h_image_size: %d h_sync_polarity: %d\n",
		 mdtd->h_sync_pulse_width, mdtd->h_image_size,
		 mdtd->h_sync_polarity);
	HISI_FB_INFO("[DP] v_active: %d v_blanking: %d v_sync_offset: %d\n",
		 mdtd->v_active, mdtd->v_blanking, mdtd->v_sync_offset);
	HISI_FB_INFO("[DP] v_sync_pulse_width: %d v_image_size: %d v_sync_polarity: %d\n",
		 mdtd->v_sync_pulse_width, mdtd->v_image_size,
		 mdtd->v_sync_polarity);
	mdtd->pixel_clock *= 10;
	return 0;
}

void dptx_audio_sdp_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
	reg |= DPTX_EN_AUDIO_STREAM_SDP;
	dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL, reg);
}

void dptx_audio_timestamp_sdp_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_TIMESTAMP_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

#ifndef CONFIG_HISI_FB_V510
static uint8_t dptx_audio_get_sample_freq_cfg(struct audio_params *aparams)
{
	uint8_t iec_orig_samp_freq = 0;
	uint8_t iec_samp_freq = 0;
	uint8_t sample_freq_cfg = 0;

	iec_orig_samp_freq = aparams->iec_orig_samp_freq;
	iec_samp_freq = aparams->iec_samp_freq;

	if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_32K && iec_samp_freq == IEC_SAMP_FREQ_32K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_32K;
	} else if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_48K && iec_samp_freq == IEC_SAMP_FREQ_48K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_48K;
	} else if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_96K && iec_samp_freq == IEC_SAMP_FREQ_96K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_96K;
	} else if (iec_orig_samp_freq == IEC_ORIG_SAMP_FREQ_192K && iec_samp_freq == IEC_SAMP_FREQ_192K) {
		sample_freq_cfg = DPTX_AUDIO_SAMPLE_FREQ_192K;
	} else {
		sample_freq_cfg = DPTX_AUDIO_REFER_TO_STREAM_HEADER;
	}

	return sample_freq_cfg;
}

static uint8_t dptx_audio_get_data_width_cfg(struct audio_params *aparams)
{
	uint8_t data_width_cfg = 0;

	if (aparams->data_width == 16) {
		data_width_cfg = DPTX_AUDIO_SAMPLE_SIZE_16BIT;
	} else if (aparams->data_width == 24) {
		data_width_cfg = DPTX_AUDIO_SAMPLE_SIZE_24BIT;
	} else {
		data_width_cfg = DPTX_AUDIO_REFER_TO_STREAM_HEADER;
	}

	return data_width_cfg;
}

static uint8_t dptx_audio_get_num_channels_cfg(struct audio_params *aparams)
{
	uint8_t num_channels_cfg = 0;

	if (aparams->num_channels == 2) {
		num_channels_cfg = DPTX_AUDIO_CHANNEL_CNT_2CH;
	} else if (aparams->num_channels == 8) {
		num_channels_cfg = DPTX_AUDIO_CHANNEL_CNT_8CH;
	} else {
		num_channels_cfg = DPTX_AUDIO_REFER_TO_STREAM_HEADER;
	}

	return num_channels_cfg;
}

static uint8_t dptx_audio_get_speaker_map_cfg(struct audio_params *aparams)
{
	uint8_t speaker_map_cfg = 0;

	if (aparams->num_channels == 2) {
		speaker_map_cfg = DPTX_AUDIO_SPEAKER_MAPPING_2CH;
	} else {
		speaker_map_cfg = DPTX_AUDIO_SPEAKER_MAPPING_8CH;
	}

	return speaker_map_cfg;
}
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;
	uint8_t sample_freq_cfg = 0;
	uint8_t data_width_cfg = 0;
	uint8_t num_channels_cfg = 0;
	uint8_t speaker_map_cfg = 0;
	uint32_t audio_infoframe_header = AUDIO_INFOFREAME_HEADER;
	uint32_t audio_infoframe_data[3] = {0x0, 0x0, 0x0};

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	sample_freq_cfg = dptx_audio_get_sample_freq_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= sample_freq_cfg << DPTX_AUDIO_SAMPLE_FREQ_SHIFT;

	data_width_cfg = dptx_audio_get_data_width_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= data_width_cfg << DPTX_AUDIO_SAMPLE_SIZE_SHIFT;

	num_channels_cfg = dptx_audio_get_num_channels_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= num_channels_cfg << DPTX_AUDIO_CHANNEL_CNT_SHIFT;

	speaker_map_cfg = dptx_audio_get_speaker_map_cfg(&dptx->aparams);
	audio_infoframe_data[0] |= speaker_map_cfg << DPTX_AUDIO_SPEAKER_MAPPING_SHIFT;

	dptx->sdp_list[0].payload[0] = audio_infoframe_header;
	dptx_writel(dptx, DPTX_SDP_BANK, audio_infoframe_header);
	/* Synosys FAE luheng:
		set reg offset 0x604 to all zero. When infoframe is zero, sink just check stream head.
		Otherwire sink would checkout if inforame equal stream head info */
	dptx_writel(dptx, DPTX_SDP_BANK + 4, audio_infoframe_data[0]);
	dptx_writel(dptx, DPTX_SDP_BANK + 8, audio_infoframe_data[1]);
	dptx_writel(dptx, DPTX_SDP_BANK + 12, audio_infoframe_data[2]);

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_INFOFRAME_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}
#else
/*
This Function is used by audio team.
*/
void dptx_audio_infoframe_sdp_send(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint32_t audio_infoframe_header = AUDIO_INFOFREAME_HEADER;
	uint32_t audio_infoframe_data[3] = {0x00000710, 0x0, 0x0};
	uint8_t orig_sample_freq = 0;
	uint8_t sample_freq = 0;
	struct audio_params *aparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;
	sample_freq = aparams->iec_samp_freq;
	orig_sample_freq = aparams->iec_orig_samp_freq;

	if (orig_sample_freq == 12 && sample_freq == 3)
		audio_infoframe_data[0] = 0x00000710;
	else if (orig_sample_freq == 15 && sample_freq == 0)
		audio_infoframe_data[0] = 0x00000B10;
	else if (orig_sample_freq == 13 && sample_freq == 2)
		audio_infoframe_data[0] = 0x00000F10;
	else if (orig_sample_freq == 7 && sample_freq == 8)
		audio_infoframe_data[0] = 0x00001310;
	else if (orig_sample_freq == 5 && sample_freq == 10)
		audio_infoframe_data[0] = 0x00001710;
	else if (orig_sample_freq == 3 && sample_freq == 12)
		audio_infoframe_data[0] = 0x00001B10;
	else
		audio_infoframe_data[0] = 0x00001F10;

	audio_infoframe_data[0] |= (aparams->num_channels - 1);
	if (aparams->num_channels == 3)
		audio_infoframe_data[0] |= 0x02000000;
	else if (aparams->num_channels == 4)
		audio_infoframe_data[0] |= 0x03000000;
	else if (aparams->num_channels == 5)
		audio_infoframe_data[0] |= 0x07000000;
	else if (aparams->num_channels == 6)
		audio_infoframe_data[0] |= 0x0b000000;
	else if (aparams->num_channels == 7)
		audio_infoframe_data[0] |= 0x0f000000;
	else if (aparams->num_channels == 8)
		audio_infoframe_data[0] |= 0x13000000;

	HISI_FB_DEBUG("[DP] audio_infoframe_data[0] before = %x\n", audio_infoframe_data[0]);
	switch (aparams->data_width) {
		case 16:
			HISI_FB_DEBUG("[DP]%s: data_width = 16\n", __func__);
			audio_infoframe_data[0] &= ~GENMASK(9,8);
			audio_infoframe_data[0] |= 1 << 8;
			break;
		case 20:
			HISI_FB_DEBUG("[DP]%s: data_width = 20\n", __func__);
			audio_infoframe_data[0] &= ~GENMASK(9,8);
			audio_infoframe_data[0] |= 2 << 8;
			break;
		case 24:
			HISI_FB_DEBUG("[DP]%s: data_width = 24\n", __func__);
			audio_infoframe_data[0] &= ~GENMASK(9,8);
			audio_infoframe_data[0] |= 3 << 8;
			break;
		default:
			HISI_FB_DEBUG("[DP]%s: data_width not found\n", __func__);
			break;
	}

	HISI_FB_DEBUG("[DP] audio_infoframe_data[0] after = %x\n", audio_infoframe_data[0]);

	dptx->sdp_list[0].payload[0] = audio_infoframe_header;
	dptx_writel(dptx, DPTX_SDP_BANK, audio_infoframe_header);
	dptx_writel(dptx, DPTX_SDP_BANK + 4, audio_infoframe_data[0]);
	dptx_writel(dptx, DPTX_SDP_BANK + 8, audio_infoframe_data[1]);
	dptx_writel(dptx, DPTX_SDP_BANK + 12, audio_infoframe_data[2]);
	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_AUDIO_INFOFRAME_SDP;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}
#endif

static void dptx_hdr_infoframe_set_reg(struct dp_ctrl *dptx, uint8_t enable)
{
	int i, j;
	uint32_t reg;
	uint32_t hdr_infoframe_data = 0;
	struct sdp_full_data hdr_sdp_data;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("[DP] dptx has already off.\n");
		return;
	}

	memset(&hdr_sdp_data, 0, sizeof(hdr_sdp_data));
	hdr_sdp_data.en = enable;
	hdr_sdp_data.payload[0] = HDR_INFOFRAME_HEADER;
	hdr_sdp_data.payload[1] = (dptx->hdr_infoframe.data[1] << 24) | (dptx->hdr_infoframe.data[0] << 16) |
		(HDR_INFOFRAME_LENGTH << 8) |HDR_INFOFRAME_VERSION;

	for (i = 2; i < HDR_INFOFRAME_LENGTH; i++) {
		for (j = 0; j < DATA_NUM_PER_REG; j++) {
			hdr_infoframe_data |= (uint32_t)dptx->hdr_infoframe.data[i] << ((j % DATA_NUM_PER_REG) * INFOFRAME_DATA_SIZE);

			if (j < (DATA_NUM_PER_REG - 1))
				i++;

			if (i >= HDR_INFOFRAME_LENGTH)
				break;
		}

		hdr_sdp_data.payload[i / DATA_NUM_PER_REG + 1] = hdr_infoframe_data;
		hdr_infoframe_data = 0;
	}

	for (i = 0; i < DPTX_SDP_LEN; i++) {
		HISI_FB_DEBUG("[DP] hdr_sdp_data.payload[%d]: %x\n", i, hdr_sdp_data.payload[i]);
		dptx_writel(dptx, DPTX_SDP_BANK +  4 * i, hdr_sdp_data.payload[i]);
		reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_BANK + 4 * i);
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	reg |= DPTX_EN_HDR_INFOFRAME_SDP | DPTX_FIXED_PRIORITY_ARBITRATION;
	dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
}

int dptx_hdr_infoframe_sdp_send(struct dp_ctrl *dptx, const void __user *argp)
{
	int i;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] dptx is NULL\n");
		return -EINVAL;
	}

	if (argp == NULL) {
		HISI_FB_ERR("[DP] argp is NULL\n");
		return -EINVAL;
	}

	dptx->hdr_infoframe.type_code = INFOFRAME_PACKET_TYPE_HDR;
	dptx->hdr_infoframe.version_number = HDR_INFOFRAME_VERSION;
	dptx->hdr_infoframe.length = HDR_INFOFRAME_LENGTH;

	memset(dptx->hdr_infoframe.data, 0x00, HDR_INFOFRAME_LENGTH);

	if (copy_from_user(&(dptx->hdr_metadata), (void __user *)argp, sizeof(dptx->hdr_metadata))) {
		HISI_FB_ERR("[%s]: copy arg failed!\n", __func__);
		return -EFAULT;
	}

	dptx->hdr_infoframe.data[HDR_INFOFRAME_EOTF_BYTE_NUM]
		= dptx->hdr_metadata.electro_optical_transfer_function;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_METADATA_ID_BYTE_NUM]
		= dptx->hdr_metadata.static_metadata_descriptor_id;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_0_LSB]
		= dptx->hdr_metadata.red_primary_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_0_MSB]
		= (dptx->hdr_metadata.red_primary_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_0_LSB]
		= dptx->hdr_metadata.red_primary_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_0_MSB]
		= (dptx->hdr_metadata.red_primary_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_1_LSB]
		= dptx->hdr_metadata.green_primary_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_1_MSB]
		= (dptx->hdr_metadata.green_primary_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_1_LSB]
		= dptx->hdr_metadata.green_primary_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_1_MSB]
		= (dptx->hdr_metadata.green_primary_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_2_LSB]
		= dptx->hdr_metadata.blue_primary_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_2_MSB]
		= (dptx->hdr_metadata.blue_primary_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_2_LSB]
		= dptx->hdr_metadata.blue_primary_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_2_MSB]
		= (dptx->hdr_metadata.blue_primary_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_X_LSB]
		= dptx->hdr_metadata.white_point_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_X_MSB]
		= (dptx->hdr_metadata.white_point_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_Y_LSB]
		= dptx->hdr_metadata.white_point_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_Y_MSB]
		= (dptx->hdr_metadata.white_point_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LUMI_LSB]
		= dptx->hdr_metadata.max_display_mastering_luminance & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LUMI_MSB]
		= (dptx->hdr_metadata.max_display_mastering_luminance & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MIN_LUMI_LSB]
		= dptx->hdr_metadata.min_display_mastering_luminance & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MIN_LUMI_MSB]
		= (dptx->hdr_metadata.min_display_mastering_luminance & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LIGHT_LEVEL_LSB]
		= dptx->hdr_metadata.max_content_light_level & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LIGHT_LEVEL_MSB]
		= (dptx->hdr_metadata.max_content_light_level & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_AVERAGE_LEVEL_LSB]
		= dptx->hdr_metadata.max_frame_average_light_level & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_AVERAGE_LEVEL_MSB]
		= (dptx->hdr_metadata.max_frame_average_light_level & MSB_MASK) >> SHIFT_8BIT;

	for (i = 0; i < HDR_INFOFRAME_LENGTH; i++) {
		HISI_FB_DEBUG("[DP] hdr_infoframe->data[%d] = 0x%02x", i, dptx->hdr_infoframe.data[i]);
	}

	mutex_lock(&dptx->dptx_mutex);

	dptx_hdr_infoframe_set_reg(dptx, 1);

	mutex_unlock(&dptx->dptx_mutex);

	return 0;
}

void dptx_disable_sdp(struct dp_ctrl *dptx, uint32_t *payload)
{
	int i;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (payload == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	for (i = 0; i < DPTX_SDP_NUM; i++)
		if (!memcmp(dptx->sdp_list[i].payload, payload, sizeof(dptx->sdp_list[i].payload)))
			memset(dptx->sdp_list[i].payload, 0, sizeof(dptx->sdp_list[i].payload));
}

void dptx_enable_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	uint32_t i;
	uint32_t reg;
	int reg_num;
	uint32_t header;
	int sdp_offset;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (data == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg_num = 0;
	header = cpu_to_be32(data->payload[0]);
	for (i = 0; i < DPTX_SDP_NUM; i++)
		if (dptx->sdp_list[i].payload[0] == 0) {
			dptx->sdp_list[i].payload[0] = header;
			sdp_offset = i * DPTX_SDP_SIZE;
			reg_num = 0;
			while (reg_num < DPTX_SDP_LEN) {
				dptx_writel(dptx, DPTX_SDP_BANK + sdp_offset
					    + reg_num * 4,
					    cpu_to_be32(
							data->payload[reg_num])
					    );
				reg_num++;
			}
			switch (data->blanking) {
			case 0:
				reg = dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
				break;
			case 1:
				reg = dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL,
					    reg);
				break;
			case 2:
				reg = dptx_readl(dptx, DPTX_SDP_VERTICAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_VERTICAL_CTRL, reg);
				reg = dptx_readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
				reg |= (1 << (2 + i));
				dptx_writel(dptx, DPTX_SDP_HORIZONTAL_CTRL,
					    reg);
				break;
			}
			break;
		}
}

void dptx_fill_sdp(struct dp_ctrl *dptx, struct sdp_full_data *data)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (data == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (data->en == 1)
		dptx_enable_sdp(dptx, data);
	else
		dptx_disable_sdp(dptx, data->payload);
}
#ifdef CONFIG_HISI_FB_V510
void dptx_vsd_ycbcr420_send(struct dp_ctrl *dptx, uint8_t enable)
{
	struct sdp_full_data vsc_data;
	struct video_params *vparams;
	int i;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;

	vsc_data.en = enable;
	for (i = 0 ; i < 9 ; i++) {
		if (i == 0) {
			vsc_data.payload[i] = 0x00070513;
		} else if (i == 5) {
			switch(vparams->bpc) {
				case COLOR_DEPTH_8:
					vsc_data.payload[i] = 0x30010000;
					break;
				case COLOR_DEPTH_10:
					vsc_data.payload[i] = 0x30020000;
					break;
				case COLOR_DEPTH_12:
					vsc_data.payload[i] = 0x30030000;
					break;
				case COLOR_DEPTH_16:
					vsc_data.payload[i] = 0x30040000;
					break;
				default:
					break;
			}
		} else {
			vsc_data.payload[i] = 0x0;
		}
	}
	vsc_data.blanking = 0;
	vsc_data.cont = 1;

	dptx_fill_sdp(dptx, &vsc_data);
}
#endif

void dptx_en_audio_channel(struct dp_ctrl *dptx, int ch_num, int enable)
{
	uint32_t reg;
	uint32_t data_en = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_EN_IN_MASK;

	if (enable) {
		switch (ch_num) {
			case 1:
				data_en = DPTX_EN_AUDIO_CH_1;
				break;
			case 2:
				data_en = DPTX_EN_AUDIO_CH_2;
				break;
			case 3:
				data_en = DPTX_EN_AUDIO_CH_3;
				break;
			case 4:
				data_en = DPTX_EN_AUDIO_CH_4;
				break;
			case 5:
				data_en = DPTX_EN_AUDIO_CH_5;
				break;
			case 6:
				data_en = DPTX_EN_AUDIO_CH_6;
				break;
			case 7:
				data_en = DPTX_EN_AUDIO_CH_7;
				break;
			case 8:
				data_en = DPTX_EN_AUDIO_CH_8;
				break;
			default:
				break;
		}
		reg |= data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT;
	} else {
		switch (ch_num) {
			case 1:
				data_en = DPTX_EN_AUDIO_CH_1;
				break;
			case 2:
				data_en = DPTX_EN_AUDIO_CH_2;
				break;
			case 3:
				data_en = DPTX_EN_AUDIO_CH_3;
				break;
			case 4:
				data_en = DPTX_EN_AUDIO_CH_4;
				break;
			case 5:
				data_en = DPTX_EN_AUDIO_CH_5;
				break;
			case 6:
				data_en = DPTX_EN_AUDIO_CH_6;
				break;
			case 7:
				data_en = DPTX_EN_AUDIO_CH_7;
				break;
			case 8:
				data_en = DPTX_EN_AUDIO_CH_8;
				break;
			default:
				break;
		}
		reg &= ~(data_en << DPTX_AUD_CONFIG1_DATA_EN_IN_SHIFT);
	}
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_video_reset(struct dp_ctrl *dptx, int enable, int stream)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = (uint32_t)dptx_readl(dptx, DPTX_SRST_CTRL);
	if (enable)
		reg |= DPTX_SRST_VIDEO_RESET_N((uint32_t)stream);
	else
		reg &= ~DPTX_SRST_VIDEO_RESET_N((uint32_t)stream);
	dptx_writel(dptx, DPTX_SRST_CTRL, reg);
}

void dptx_audio_mute(struct dp_ctrl *dptx)
{
	uint32_t reg;
	struct audio_params *aparams = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;
	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);

	if (aparams->mute == 1)
		reg |= DPTX_AUDIO_MUTE;
	else
		reg &= ~DPTX_AUDIO_MUTE;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_config(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_audio_core_config(dptx);
	dptx_audio_sdp_en(dptx);
	dptx_audio_timestamp_sdp_en(dptx);

	dptx_audio_infoframe_sdp_send(dptx);
}

void dptx_audio_core_config(struct dp_ctrl *dptx)
{
	struct audio_params *aparams = NULL;
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	dptx_audio_inf_type_change(dptx);
	dptx_audio_num_ch_change(dptx);
	dptx_audio_data_width_change(dptx);

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_ATS_VER_MASK;
	reg |= aparams->ats_ver << (unsigned int)DPTX_AUD_CONFIG1_ATS_VER_SHFIT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);

	dptx_en_audio_channel(dptx, aparams->num_channels, 1);
}


void dptx_audio_inf_type_change(struct dp_ctrl *dptx)
{
	struct audio_params *aparams = NULL;
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_INF_TYPE_MASK;
	reg |= aparams->inf_type << (unsigned int)DPTX_AUD_CONFIG1_INF_TYPE_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_num_ch_change(struct dp_ctrl *dptx)
{
	uint32_t reg = 0;
	uint32_t num_ch_map = 0;
	struct audio_params *aparams = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_NCH_MASK;

	if (aparams->num_channels > 0 && aparams->num_channels <= 8) {
		num_ch_map = aparams->num_channels - 1;
	} else {
		num_ch_map = DPTX_AUD_CONFIG1_NCH_DEFAULT_VALUE;
	}

	reg |= num_ch_map << (unsigned int)DPTX_AUD_CONFIG1_NCH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

void dptx_audio_data_width_change(struct dp_ctrl *dptx)
{
	uint32_t reg;
	struct audio_params *aparams = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	aparams = &dptx->aparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_AUD_CONFIG1);
	reg &= ~DPTX_AUD_CONFIG1_DATA_WIDTH_MASK;
	reg |= aparams->data_width << (unsigned int)DPTX_AUD_CONFIG1_DATA_WIDTH_SHIFT;
	dptx_writel(dptx, DPTX_AUD_CONFIG1, reg);
}

bool dptx_check_low_temperature(struct dp_ctrl *dptx)
{
	uint32_t perictrl4;
	struct hisi_fb_data_type *hisifd = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return FALSE;
	}

	hisifd = dptx->hisifd;

	if (hisifd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return FALSE;
	}

	perictrl4 = inp32(hisifd->pmctrl_base + MIDIA_PERI_CTRL4);
	perictrl4 &= PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
	perictrl4 = (perictrl4 >> PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT);
	HISI_FB_INFO("[DP] Get current temperature: %d \n", perictrl4);

	if (perictrl4 != NORMAL_TEMPRATURE)
		return TRUE;
	else
		return FALSE;
}
/*
 * Video Generation
 */
void dptx_video_timing_change(struct dp_ctrl *dptx, int stream)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_disable_default_video_stream(dptx, stream);
#ifdef CONFIG_HISI_FB_V510
	if (dptx->dsc)
		dptx_dsc_enable(dptx);
#endif
	dptx_video_core_config(dptx, stream);
	dptx_video_ts_change(dptx, stream);
	dptx_enable_default_video_stream(dptx, stream);
}

int dptx_change_video_mode_tu_fail(struct dp_ctrl *dptx)
{
	struct dtd *mdtd;
	mdtd = &dptx->vparams.mdtd;

	if ((mdtd->pixel_clock > 500000) && (mdtd->h_active >= 3840)) {	/*4k 60HZ*/
		if (((dptx->link.lanes == 2) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR2)) ||
			((dptx->link.lanes == 4) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR)))
			return 95;	/*4k 30HZ*/
	}
	if(mdtd->h_active > 1920)
		return 16;/*1920*1080*/
	if(mdtd->h_active > 1280)
		return 4;/*1280*720*/
	if(mdtd->h_active > 640)
		return 1;/*640*480*/
	return 1;
}

int dptx_change_video_mode_user(struct dp_ctrl *dptx)
{
	struct video_params *vparams = NULL;
	int retval;
	bool needchanged = false;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	if (!dptx->same_source) {
		if((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /*Siwtch to 1080p on PC mode*/
			needchanged = TRUE;
			HISI_FB_INFO("[DP] Video mode is changed by different source!\n");
		}
	}

	if(dptx->user_mode != 0) {
		vparams->video_format = dptx->user_mode_format;
		vparams->mode = dptx->user_mode; /*Siwtch to user setting*/
		needchanged = TRUE;
		HISI_FB_INFO("[DP] Video mode is changed by user setting!\n");
	}

	if (needchanged) {
		retval = dptx_video_mode_change(dptx, vparams->mode, 0);
		if (retval) {
			HISI_FB_ERR("[DP] Change mode error!\n");
			return retval;
		}
	}

	if (dptx_check_low_temperature(dptx)) {
		if ((vparams->mdtd.h_active > FHD_TIMING_H_ACTIVE) || (vparams->mdtd.v_active > FHD_TIMING_V_ACTIVE)) {
			vparams->video_format = VCEA;
			vparams->mode = 16; /*Siwtch to 1080p on PC mode*/
			HISI_FB_INFO("[DP] Video mode is changed by low temperature!\n");

			retval = dptx_video_mode_change(dptx, vparams->mode, 0);
			if (retval) {
				HISI_FB_ERR("[DP] Change mode error!\n");
				return retval;
			}
		}
	}

	dp_imonitor_set_param(DP_PARAM_SOURCE_MODE, &(dptx->same_source));
	dp_imonitor_set_param(DP_PARAM_USER_MODE,   &(vparams->mode));
	dp_imonitor_set_param(DP_PARAM_USER_FORMAT, &(vparams->video_format));
	return 0;
}

int dptx_video_mode_change(struct dp_ctrl *dptx, uint8_t vmode, int stream)
{
	int retval;
	struct video_params *vparams = NULL;
	struct dtd mdtd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	vparams = &dptx->vparams;
	vparams->mode = vmode;

	if (!dptx_dtd_fill(&mdtd, vparams->mode, vparams->refresh_rate,
			   vparams->video_format)) {
		HISI_FB_ERR("[DP] Invalid video mode value %d\n",
						vparams->mode);
		return -EINVAL;
	}
	vparams->mdtd = mdtd;
	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
					 dptx->link.rate, vparams->bpc,
					 vparams->pix_enc, mdtd.pixel_clock);

	HISI_FB_INFO("[DP] The mode is changed as [%d]\n", vparams->mode);

	return retval;
}

int dptx_video_config(struct dp_ctrl *dptx, int stream)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	if (!dptx_dtd_fill(mdtd, vparams->mode,
			   vparams->refresh_rate, vparams->video_format))
		return -EINVAL;

	dptx_video_core_config(dptx, stream);
	return 0;
}

#ifdef CONFIG_HISI_FB_V510
int dptx_calculate_hblank_interval(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	int pixel_clk;
	uint16_t h_blank;
	uint32_t link_clk;
	uint8_t rate;
	int hblank_interval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	pixel_clk = vparams->mdtd.pixel_clock;
	h_blank = vparams->mdtd.h_blanking;
	rate = dptx->link.rate;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_clk = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_clk = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_clk = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_clk = 202500;
		break;
	default:
		WARN(1, "Invalid rate 0x%x\n", rate);
		return -EINVAL;
	}

	hblank_interval = h_blank * link_clk / pixel_clk;

	return hblank_interval;
}

int dptx_sink_color_format_capabilities(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t dsc_color_format = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_read_dpcd(dptx, DP_DSC_DEC_COLOR_FORMAT_CAP, &dsc_color_format);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	if (dsc_color_format & DP_DSC_RGB)
		HISI_FB_DEBUG("Sink supports RGB color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_444)
		HISI_FB_DEBUG("Sink supports YCbCr 4:4:4 color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_422_SIMPLE)
		HISI_FB_DEBUG("Sink supports YCbCr 4:2:2 SIMPLE color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_422_NATIVE)
		HISI_FB_DEBUG("Sink supports YCbCr 4:2:2 NATIVE color format\n");

	if (dsc_color_format & DP_DSC_YCBCR_420_NATIVE)
		HISI_FB_DEBUG("Sink supports YCbCr 4:2:0 NATIVE color format\n");

	return 0;
}

int dptx_sink_color_depth_capabilities(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t dsc_color_depth = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_read_dpcd(dptx, DP_DSC_DEC_COLOR_DEPTH_CAP, &dsc_color_depth);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	if (dsc_color_depth & DP_DSC_8_BPC)
		HISI_FB_DEBUG("Sink supports 8 bpc\n");

	if (dsc_color_depth & DP_DSC_10_BPC)
		HISI_FB_DEBUG("Sink supports 10 bpc\n");

	if (dsc_color_depth & DP_DSC_12_BPC)
		HISI_FB_DEBUG("Sink supports 12 bpc\n");

	return 0;
}

int dptx_get_slice_count(int ppr)
{
	// Determine slice count by peak pixel throughput rate (ppr) (MP/s)
	if (ppr <= 340)
		return 1;

	if (ppr > 340 && ppr <= 680)
		return 2;

	if (ppr > 680 && ppr <= 1360)
		return 4;

	if (ppr > 1360 && ppr <= 3200)
		return 8;

	if (ppr > 3200 && ppr <= 4800)
		return 12;

	if (ppr > 4800 && ppr <= 6400)
		return 16;

	if (ppr > 6400 && ppr <= 8000)
		return 20;

	if (ppr > 8000 && ppr <= 9600)
		return 24;

	return -EINVAL;
}

bool dptx_slice_count_supported(struct dp_ctrl* dptx, uint8_t slice_count)
{
	uint8_t slice_cap1 = 0;
	uint8_t slice_cap2 = 0;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return false;
	}

	retval = dptx_read_dpcd(dptx, DP_DSC_SLICE_CAP_1, &slice_cap1);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	retval = dptx_read_dpcd(dptx, DP_DSC_SLICE_CAP_2, &slice_cap2);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	if ((slice_cap1 & DP_DSC_1_PER_DP_DSC_SINK) && (slice_count == 1))
		return true;

	if ((slice_cap1 & DP_DSC_2_PER_DP_DSC_SINK) && (slice_count == 2))
		return true;

	if ((slice_cap1 & DP_DSC_4_PER_DP_DSC_SINK) && (slice_count == 4))
		return true;

	if ((slice_cap1 & DP_DSC_6_PER_DP_DSC_SINK) && (slice_count == 6))
		return true;

	if ((slice_cap1 & DP_DSC_8_PER_DP_DSC_SINK) && (slice_count == 8))
		return true;

	if ((slice_cap1 & DP_DSC_10_PER_DP_DSC_SINK) && (slice_count == 10))
		return true;

	if ((slice_cap1 & DP_DSC_12_PER_DP_DSC_SINK) && (slice_count == 12))
		return true;

	if ((slice_cap2 & DP_DSC_16_PER_DP_DSC_SINK) && (slice_count == 16))
		return true;

	if ((slice_cap2 & DP_DSC_20_PER_DP_DSC_SINK) && (slice_count == 20))
		return true;

	if ((slice_cap2 & DP_DSC_24_PER_DP_DSC_SINK) && (slice_count == 24))
		return true;

	return false;
}

int  dptx_next_slice_count(struct dp_ctrl* dptx, uint8_t slice_count)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (slice_count == 24)
		return 1;

	if (slice_count == 1) {
		slice_count = 2;
	} else if (slice_count == 2) {
		slice_count = 4;
	} else {
		slice_count += 4;
	}

	return slice_count;
}

/*
 *  Get the slice width based on slice count, which is supported by the sink
 */
uint16_t dptx_get_slice_width(struct dp_ctrl* dptx, uint8_t* slice_count)
{
	struct video_params *vparams;
	uint16_t h_active;
	uint8_t max_slice_width = 0;
	uint16_t slice_width;
	int ppr;
	int retval;

	if ((dptx == NULL) || (slice_count == NULL)) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return 0;
	}

	vparams = &dptx->vparams;
	ppr = vparams->mdtd.pixel_clock;
	h_active = vparams->mdtd.h_active;

	retval = dptx_read_dpcd(dptx, DP_DSC_MAX_SLICE_WIDTH, &max_slice_width);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	// Initialize slice width
	slice_width = 4096;

	if (h_active <= 4096)
		slice_width = 2048;

	if (slice_width > max_slice_width)
		slice_width = max_slice_width;

	// Adjust slice count and recalculate again, if slice width > max slice width
	if ((*slice_count) != 0) {
		slice_width = (h_active / (*slice_count));
	} else {
		HISI_FB_ERR("[DP] slice_width is invalid\n");
		return 0;
	}

	return slice_width;
}

int dptx_dsc_encoders_count(struct dp_ctrl *dptx)
{
	struct video_params *vparams;
	int retval;
	int ppr;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	ppr = vparams->mdtd.pixel_clock;

	retval = dptx_sink_color_format_capabilities(dptx);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	retval = dptx_sink_color_depth_capabilities(dptx);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	// Get the initial slice count based on ppr
	HISI_FB_DEBUG("[DP] Initial slice count based on PPR is %d\n", vparams->encoders);

	vparams->slice_width = dptx_get_slice_width(dptx, &vparams->encoders);
	vparams->chunk_size = vparams->slice_width;

	if (vparams->slice_width == 0) {
		HISI_FB_ERR("[DP] DSC slice width error!!!\n");
		return -EINVAL;
	}

	HISI_FB_DEBUG("[DP] Slice count = %d, slice width = %d, vparams->chunk_size = %d\n",
		vparams->encoders, vparams->slice_width, vparams->chunk_size);

	return vparams->encoders;
}

void dptx_divide_pixel_clock(struct dp_ctrl* dptx)
{
	uint32_t reg;
	uint8_t pixels_per_pixelclk = 0;
	int encoders;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	encoders = dptx->vparams.encoders;
	// Determine sampled pixel count based on pixel clock
	switch (dptx->multipixel) {
	case DPTX_MP_SINGLE_PIXEL:
		pixels_per_pixelclk = 1;
		break;
	case DPTX_MP_DUAL_PIXEL:
		pixels_per_pixelclk = 2;
		break;
	case DPTX_MP_QUAD_PIXEL:
		pixels_per_pixelclk = 4;
		break;
	default:
		break;
	}

	// Program DSC_CTL.STREAMn_ENC_CLK_DIVIDED bit
	if (encoders > pixels_per_pixelclk) {
		// Divide pixel clock for DSC encoder
		reg = dptx_readl(dptx, DPTX_DSC_CTL);
		reg |= 1 << DPTX_DSC_STREAM0_ENC_CLK_DIV_SHIFT;
		dptx_writel(dptx, DPTX_DSC_CTL, reg);
	}
}

void dptx_pixel_mode_based_on_encoder_count(struct dp_ctrl *dptx)
{
	int encoders;
	uint32_t reg;
	uint8_t stream = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	encoders = dptx->vparams.encoders;
	// Change pixel mode based on encoders count
	switch (encoders) {
	case 8:
		dptx->multipixel = DPTX_MP_QUAD_PIXEL;
		break;
	case 4:
		dptx->multipixel = DPTX_MP_DUAL_PIXEL;
		break;
	default:
		break;
	}

	/* Single, dual, or quad pixel */
	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_MULTI_PIXEL_MASK;
	reg |= dptx->multipixel << DPTX_VSAMPLE_CTRL_MULTI_PIXEL_SHIFT;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);

	/* Divide pixel clock, if needed */
	dptx_divide_pixel_clock(dptx);
}

int dptx_mark_encoders(struct dp_ctrl *dptx)
{
	uint8_t available_encoders;
	uint8_t dsc_hwcfg;
	uint8_t encoders;
	uint32_t dsc_ctl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	encoders = dptx->vparams.encoders;
	dsc_hwcfg = dptx_readl(dptx, DPTX_DSC_HWCFG);
	dsc_ctl = dptx_readl(dptx, DPTX_DSC_CTL);
	available_encoders = dsc_hwcfg & DPTX_DSC_NUM_ENC_MSK;

	dsc_ctl = dptx_readl(dptx, DPTX_DSC_CTL);
	dsc_ctl |= (encoders / 2) << 22;
	dptx_writel(dptx, DPTX_DSC_CTL, dsc_ctl);

	HISI_FB_DEBUG("[DP] Calculated encoder count = %d\n", encoders);
	HISI_FB_DEBUG("[DP] Available encoders count = %d\n", available_encoders);

	if (encoders > available_encoders) {
		HISI_FB_INFO("[DP] Encoder count is greather than available encoders\n");
		return -EINVAL;
	}

	return 0;
}

void dptx_program_dsc_version(struct dp_ctrl* dptx)
{
	uint8_t dsc_rev = 0;
	uint8_t dsc_rev_min;
	uint8_t dsc_rev_maj;
	uint32_t pps;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	retval = dptx_read_dpcd(dptx, DP_DSC_REV, &dsc_rev);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	/* Hardcode, DPCD not present */
	dsc_rev_min = 2;
	dsc_rev_maj = 1 >> DP_DSC_MAJOR_SHIFT;

	HISI_FB_INFO("[DP] %s  PPS dsc_rev = %d\n", __func__, dsc_rev);
	HISI_FB_INFO("[DP] %s  PPS dsc_rev_min = %d  dsc_rev_maj = %d\n", __func__, dsc_rev_min, dsc_rev_maj);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0));
	pps |= dsc_rev_min;
	pps |= (dsc_rev_maj << DPTX_DSC_VER_MAJ_SHIFT);
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0), pps);
}

void dptx_program_dsc_buf_bit_depth(struct dp_ctrl* dptx)
{
	uint8_t buf_bit_depth = 0;
	uint8_t pps_buf_bit_depth;
	uint32_t pps;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	retval = dptx_read_dpcd(dptx, DP_DSC_LINE_BUF_BIT_DEPTH, &buf_bit_depth);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	pps_buf_bit_depth = (buf_bit_depth & DP_DSC_LINE_BUF_BIT_DEPTH_MASK);
	HISI_FB_INFO("[DP] %s PPS pps_buf_bit_depth = %d\n", __func__, pps_buf_bit_depth);

	/* Hardcode, DPCD not present */
	pps_buf_bit_depth = 0xE;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0));
	pps |= (pps_buf_bit_depth << DPTX_DSC_BUF_DEPTH_SHIFT);
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0), pps);
}

void dptx_program_dsc_block_prediction(struct dp_ctrl* dptx)
{
	uint8_t block_pred = 0;
	uint32_t pps;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	retval = dptx_read_dpcd(dptx, DP_DSC_BLK_PREDICTION_SUPPORT, &block_pred);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
	pps |=  block_pred << 5;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);

	HISI_FB_INFO("[DP] %s PPS block_pred = %d \n", __func__, block_pred);
}

static uint8_t dptx_calculate_bpc(struct dp_ctrl* dptx)
{
	uint8_t bpc;
	uint8_t port_cap[4] = {0};
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return 0;
	}

	retval = dptx_read_bytes_from_dpcd(dptx, DP_DOWNSTREAM_PORT_0,
		port_cap, sizeof(port_cap));
	if (retval)
		return 0;

	bpc = port_cap[2] & DP_DS_MAX_BPC_MASK;

	switch (bpc) {
	case DP_DS_8BPC:
		return 8;
	case DP_DS_10BPC:
		return 10;
	case DP_DS_12BPC:
		return 12;
	case DP_DS_16BPC:
		return 16;
	default:
		return 0;
	}
}

void dptx_program_dsc_bits_perpixel(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	enum pixel_enc_type;
	uint8_t bpp_high, bpp_low;
	uint16_t bpp;
	uint32_t pps;
	uint32_t bpp1;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	bpp1 = 128;

	bpp = DPTX_BITS_PER_PIXEL; // for Interop only
	dptx->vparams.dsc_bpp = bpp;

	/* Get high and low parts of bpp (10 bits) */
	bpp_high = (bpp & DPTX_DSC_BPP_HIGH_MASK) >> 8;
	bpp_low = bpp1 & DPTX_DSC_BPP_LOW_MASK;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
	pps |= bpp_high;
	pps |= bpp_low << 8 ;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);

	HISI_FB_INFO("[DP] %s PPS  bpp = %d\n", __func__, bpp);
}

void dptx_program_dsc_bpc_and_depth(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	enum pixel_enc_type pix_enc;
	uint8_t bpc;
	uint32_t pps;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	vparams->dsc_bpc = dptx_calculate_bpc(dptx);
	bpc = vparams->dsc_bpc;
	pix_enc = vparams->pix_enc;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0));
	switch (bpc) {
	case COLOR_DEPTH_8:
		pps |= bpc << DPTX_DSC_BPC_SHIFT;
		break;
	case COLOR_DEPTH_10:
		pps |= bpc << DPTX_DSC_BPC_SHIFT;
		break;
	case COLOR_DEPTH_12:
		pps |= bpc << DPTX_DSC_BPC_SHIFT;
		break;
	case COLOR_DEPTH_16:
		pps |= bpc << DPTX_DSC_BPC_SHIFT;
		break;
	default:
		HISI_FB_INFO("[DP] Unsupported Color depth by DSC spec\n");
		break;
	}

	HISI_FB_INFO("[DP] %s PPS bpc = %d\n", __func__, bpc);
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 0), pps);

	switch (pix_enc) {
	case RGB:
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
		pps |= 1 << 4;
		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);
		break;
	case YCBCR420:
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 23));
		pps |= 1 << 1;
		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 23), pps);
		break;
	default:
		break;
	}
}

void dptx_program_dsc_slice_width(struct dp_ctrl* dptx)
{
	uint32_t pps = 0;
	struct video_params *vparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;

	pps |= dptx->vparams.slice_width >> 8;
	pps |= (dptx->vparams.slice_width & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 3), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 3));
	pps |= (dptx->vparams.chunk_size >> 8) << 16;
	pps |= (dptx->vparams.chunk_size & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 3), pps);

	HISI_FB_DEBUG("[DP] %s PPS slice_width = %d\n", __func__, dptx->vparams.slice_width);
	HISI_FB_DEBUG("[DP] %s PPS chunk_size = %d\n", __func__, dptx->vparams.chunk_size);
	HISI_FB_DEBUG("[DP] %s PPS vbr_enable = %d\n", __func__, 0);
}

void dptx_program_dsc_pic_width_height(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	uint32_t pps;
	uint32_t pic_width;
	uint32_t pic_height;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	pic_width = vparams->mdtd.h_active;
	pic_height = vparams->mdtd.v_active;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2));
	pps = 0;
	pps |= (pic_width >> 8);
	pps |= (pic_width & GENMASK(7, 0)) << 8;

	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2), pps);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1));
	pps &= ~GENMASK(31, 16);
	pps |= (pic_height >> 8) << 16;
	pps |= (pic_height & GENMASK(7, 0)) << 24;

	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 1), pps);

	HISI_FB_DEBUG("[DP] %s PPS pic_width = %d\n", __func__, pic_width);
	HISI_FB_DEBUG("[DP] %s PPS pic_height = %d\n", __func__, pic_height);
}

void dptx_program_dsc_slice_height(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	uint16_t pic_height;
	uint16_t slice_height;
	uint16_t dsc_max_num_lines;
	uint32_t reg;
	uint8_t first_line_bpg_offset;
	uint8_t second_line_bpg_offset = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = dptx_readl(dptx, DPTX_CONFIG_REG2);
	dsc_max_num_lines = (reg & DSC_MAX_NUM_LINES_MASK) >> DSC_MAX_NUM_LINES_SHIFT;

	HISI_FB_DEBUG("[DP] %s dsc_max_num_lines = %d\n", __func__, dsc_max_num_lines);

	vparams = &dptx->vparams;
	pic_height = vparams->mdtd.v_active;

	if (pic_height < dsc_max_num_lines) {
		slice_height = pic_height;
	} else {
		slice_height = pic_height;
		while (!(slice_height < dsc_max_num_lines)) {
			slice_height = pic_height >> 1; // divide to 2
		}
	}

	dptx->vparams.slice_height = slice_height;
	HISI_FB_DEBUG("[DP] %s PPS slice_height = %d\n", __func__, slice_height);
	reg = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2));
	reg |= (slice_height >> 8) << 16;
	reg |= (slice_height & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 2), reg);

	// Calculate first_line_bpg_offset based on slice height
	if (slice_height >= 8) {
		first_line_bpg_offset = 12 +  ((9 * min(34, slice_height - 8)) / 100);
	} else {
		first_line_bpg_offset = 2 * (slice_height - 1);
	}

	reg = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6));
	reg |= first_line_bpg_offset << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6), reg);

	HISI_FB_DEBUG("[DP] %s PPS first_line_bpg_offset = %d\n", __func__, first_line_bpg_offset);

	// Calculate second_line_bpg_offset based on slice height only in YcBcR420
	if (dptx->vparams.pix_enc == YCBCR420) {
		if (slice_height < 8)
			second_line_bpg_offset = 2 * (slice_height - 1);
	} else {
		// Generic value for second_line_bpg_offset
		second_line_bpg_offset = 0;
	}

	reg = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 22));
	reg |= second_line_bpg_offset << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 22), reg);

	dptx->vparams.first_line_bpg_offset = first_line_bpg_offset;
}

uint16_t dptx_dsc_groups_per_line(struct dp_ctrl* dptx)
{
	uint16_t groups_per_line;
	uint16_t slice_width;
	enum pixel_enc_type pix_enc;
	struct video_params *vparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return 0;
	}

	vparams = &dptx->vparams;
	slice_width = dptx->vparams.slice_width;
	pix_enc = vparams->pix_enc;

	if (pix_enc == RGB) {
		groups_per_line = slice_width / PIXELS_PER_GROUP;
	} else {
		groups_per_line = (slice_width >> 1) / PIXELS_PER_GROUP;
	}

	return groups_per_line;
}

void dptx_program_dsc_min_rate_bufsize(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	enum pixel_enc_type pix_enc;
	uint32_t minRateBufferSize = 0;
	uint32_t pps;
	uint16_t group_pl;
	uint8_t rc_buf_size;
	uint8_t rc_buf_block_size;
	uint8_t first_line_bpg_offset;
	uint8_t bpc;
	uint16_t bpp;
	uint16_t rc_model_size = RC_MODEL_SIZE;
	uint16_t initial_offset = INITIAL_OFFSET;
	uint16_t initial_delay = INITIAL_DELAY;
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	bpc = vparams->dsc_bpc;
	bpp = vparams->dsc_bpp;
	pix_enc = vparams->pix_enc;
	first_line_bpg_offset = dptx->vparams.first_line_bpg_offset;
	group_pl = dptx_dsc_groups_per_line(dptx);

	retval = dptx_read_dpcd(dptx, DP_DSC_RC_BUF_SIZE, &rc_buf_size);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	retval = dptx_read_dpcd(dptx, DP_DSC_RC_BUF_BLK_SIZE, &rc_buf_block_size);
	if (retval)
		HISI_FB_ERR("[DP] %s : DPCD read failed\n", __func__);

	switch (pix_enc) {
	case RGB:
		minRateBufferSize = (rc_model_size - initial_offset) +
			(initial_delay * bpp) +
			(group_pl * first_line_bpg_offset);
		break;
	default:
		break;
	}

	HISI_FB_DEBUG("[DP] PPS DSC minRateBufferSize is %d\n", minRateBufferSize);
	HISI_FB_DEBUG("[DP] PPS DSC rc_model_size is %d\n", rc_model_size);
	HISI_FB_DEBUG("[DP] PPS DSC initial_offset is %d\n", initial_offset);
	HISI_FB_DEBUG("[DP] PPS DSC initial_delay is %d\n", initial_delay);

	// Program rc_model_size pps
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9));
	pps |= (rc_model_size >> 8) << 16;
	pps |= (rc_model_size & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9), pps);

	// Program initial_offset pps
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8));
	pps |= ((initial_offset & GENMASK(15, 8)) >> 8);
	pps |= (initial_offset & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8), pps);

	// Program initial_delay pps
	pr_err("initial_delay = %d\n", initial_delay);
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4));
	pps |= ((initial_delay & GENMASK(9, 8)) >> 8);
	pps |= (initial_delay & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4), pps);

	dptx->vparams.minRateBufferSize = minRateBufferSize;
}

uint8_t dptx_dsc_get_mux_word_size(struct dp_ctrl* dptx)
{
	uint8_t bpc;
	uint8_t muxWordSize = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return muxWordSize;
	}

	bpc = dptx->vparams.dsc_bpc;

	switch (bpc) {
	case 12:
	case 14:
	case 16:
		muxWordSize = 64;
		break;
	case 8:
	case 10:
		muxWordSize = 48;
		break;
	default:
		HISI_FB_ERR("[DP] Cant get muxWordSize based on bpc\n");
		break;
	}

	return muxWordSize;
}

void dptx_program_dsc_hrdelay(struct dp_ctrl* dptx)
{
	struct video_params *vparams;
	uint32_t minRateBufferSize;
	uint8_t muxWordSize, maxSeSize_Y, maxSeSize_C;
	uint16_t bpp;
	uint8_t bpc;
	uint16_t group_per_line;
	uint32_t pps;
	uint32_t hrddelay;
	uint32_t initial_dec_delay;
	uint32_t slice_bpg_offset;
	uint16_t initial_scale_value;
	uint16_t rc_model_size = RC_MODEL_SIZE;
	uint16_t initial_offset = INITIAL_OFFSET;
	uint32_t scale_decrement_interval;
	uint32_t scale_increment_interval;
	uint32_t numExtraMuxBits;
	int rcxformoffset;
	int final_offset;
	int final_scale_value;
	int nfl_bpg_offset;
	int groupsTotal;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	minRateBufferSize = dptx->vparams.minRateBufferSize;
	bpp = dptx->vparams.dsc_bpp;
	bpc = dptx->vparams.dsc_bpc;

	hrddelay = minRateBufferSize / bpp;

	HISI_FB_DEBUG("[DP] %s PPS hrddelay = %d\n", __func__, hrddelay);
	dptx->vparams.hrddelay = hrddelay;

	// Calculate initial_dec_delay and program PPS
	initial_dec_delay = hrddelay - INITIAL_DELAY;
	dptx->vparams.initial_dec_delay = initial_dec_delay;

	HISI_FB_DEBUG("[DP] %s PPS initial_dec_delay = %d\n", __func__, initial_dec_delay);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4));
	pps |= (initial_dec_delay >> 8) << 16;
	pps |= (initial_dec_delay & GENMASK(7, 0)) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 4), pps);

	// Calculate initial_scale_value and program PPS
	initial_scale_value = 8 * (rc_model_size / (rc_model_size - initial_offset));
	dptx->vparams.initial_scale_value = initial_scale_value;
	HISI_FB_DEBUG("[DP] %s PPS initial_scale_value = %d\n", __func__, initial_scale_value);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5));
	pps |= ((initial_scale_value >> 3) << INITIAL_SCALE_VALUE_SHIFT) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5), pps);

	// Calculate scale_decrement_interval and program PPS
	group_per_line = dptx_dsc_groups_per_line(dptx);

	scale_decrement_interval = group_per_line / (initial_scale_value - 8);
	HISI_FB_DEBUG("[DP] %s PPS scale_decrement_interval = %d\n", __func__, scale_decrement_interval);
	dptx->vparams.scale_decrement_interval = scale_decrement_interval;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6));
	pps |= (scale_decrement_interval & GENMASK(11,9)) >> 9;
	pps |= (scale_decrement_interval & GENMASK(7, 0)) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 6), pps);

	// Calculate scale_increment_interval and program PPS
	muxWordSize = dptx_dsc_get_mux_word_size(dptx);
	rcxformoffset = INITIAL_OFFSET - RC_MODEL_SIZE;

	nfl_bpg_offset = (dptx->vparams.first_line_bpg_offset << OFFSET_FRACTIONAL_BITS) / (dptx->vparams.slice_height - 1);
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7));
	pps |= ((nfl_bpg_offset & GENMASK(15, 8)) >> 8);
	pps |= ((nfl_bpg_offset & GENMASK(7, 0))) << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7), pps);

	if (bpc == 16) {
		maxSeSize_Y = maxSeSize_C = 64;
	} else {
		maxSeSize_Y = bpc * 4 + 4;
		maxSeSize_C = (bpc + 1) * 4; // 1 is convert_rgb
	}

	numExtraMuxBits = (muxWordSize + maxSeSize_Y - 2) + 2 * (muxWordSize + maxSeSize_C - 2);
	final_offset = RC_MODEL_SIZE - INITIAL_DELAY * bpp + numExtraMuxBits;

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8));
	pps |= ((final_offset & GENMASK(15, 8)) >> 8) << 16;
	pps |= ((final_offset & GENMASK(7, 0))) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 8), pps);

	final_scale_value = RC_MODEL_SIZE / (RC_MODEL_SIZE - final_offset);
	groupsTotal = group_per_line * dptx->vparams.slice_height;
	slice_bpg_offset = ((1 << OFFSET_FRACTIONAL_BITS) *(RC_MODEL_SIZE - INITIAL_OFFSET + numExtraMuxBits) / groupsTotal);

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7));
	pps |= ((slice_bpg_offset & GENMASK(15, 8)) >> 8) << 16;
	pps |= ((slice_bpg_offset & GENMASK(7, 0))) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 7), pps);

	scale_increment_interval = (1<< 11) * (final_offset / (((8 * final_scale_value) - 9) * (nfl_bpg_offset + slice_bpg_offset)));

	HISI_FB_DEBUG("[DP] %s PPS scale_increment_interval = %d\n", __func__, scale_increment_interval);

	if (scale_increment_interval > 65535)
		HISI_FB_INFO("[DP] Scale increment interval exceeds 65535\n");

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5));
	pps |= (scale_increment_interval >> 8) << 16;
	pps |= ((scale_increment_interval & GENMASK(7, 0))) << 24;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 5), pps);
}

void dptx_program_flatness_qp(struct dp_ctrl* dptx)
{
	uint32_t pps;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9));
	pps |= FLATNESS_MIN_QP;
	pps |= FLATNESS_MAX_QP << 8;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 9), pps);
}

/*
 *  Returns 2's complement representation of negative number
 *  Number should be 5 bits in this function
 *  Number passed as integer, for being able have negative values
 */
uint8_t dptx_get_twos_compl_rep(int number)
{
	uint8_t res;

	if (number >= 0)
		return (uint8_t)number;

	number = -number;

	// Calculate 2's complement representation
	res = ((uint8_t)number ^ 0x1f) + 1;

	// Set the sign bit before returning
	return (res | (1 << 5));
}

static struct rc_range_param g_rc_range_params[15] = {
	{0, 4, 2}, {0, 4, 0}, {1, 5, 0}, {1, 6, -2}, {3, 7, -4},
	{3, 7, -6}, {3, 7, -8}, {3, 8, -8}, {3, 9, -8}, {3, 10, -10},
	{5, 10, -10}, {5, 11, -12}, {5, 11, -12}, {9, 12, -12}, {12, 13, -12},
};

void dptx_program_rc_range_parameters(struct dp_ctrl* dptx)
{
	uint32_t pps;
	int i;
	int pps_index;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, (14)));
	pps |= ((g_rc_range_params[0].maxQP & GENMASK(4, 2)) >> 2) << 16;
	pps |= g_rc_range_params[0].minQP << 19;
	pps |= dptx_get_twos_compl_rep(g_rc_range_params[0].offset) << 24;
	pps |= (g_rc_range_params[0].maxQP & GENMASK(1, 0)) << 29;
	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 14), pps);

	// Read once 4 byte and program in there 2  rc_range_parameters
	for (i = 1, pps_index = 60; i < 15; i += 2, pps_index += 4) {
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, (pps_index / 4)));
		pps |= ((g_rc_range_params[i].maxQP & GENMASK(4, 2)) >> 2);
		pps |= (g_rc_range_params[i].minQP) << 3;
		pps |= dptx_get_twos_compl_rep(g_rc_range_params[i].offset) << 8;
		pps |= (g_rc_range_params[i].maxQP & GENMASK(1, 0)) << 14 ;

		pps |= ((g_rc_range_params[i + 1].maxQP & GENMASK(4, 2)) >> 2) << 16;
		pps |= (g_rc_range_params[i + 1].minQP) << 19;
		pps |= dptx_get_twos_compl_rep(g_rc_range_params[i + 1].offset) << 24;
		pps |= (g_rc_range_params[i + 1].maxQP & GENMASK(1, 0)) << 30;

		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, pps_index / 4), pps);
	}
}

static uint32_t g_rc_buf_threshold[14] = {
	896, 1792, 2688, 3584, 4480, 5376, 6272, 6720, 7168, 7616, 7744, 7872, 8000, 8064,
};


void dptx_program_rc_parameter_set(struct dp_ctrl* dptx)
{
	uint32_t pps;
	int i;
	int pps_index;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	// rc_edge_factor
	pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 10));
	pps |= RC_EDGE_FACTOR;

	// rc_quant_incr_limit0
	pps |= RC_QUANT_INCR_LIMIT0 << 8;

	// rc_quant_incr_limit1
	pps |= RC_QUANT_INCR_LIMIT1 << 16;

	// rc_tgt_offset_hi
	pps |= RC_TGT_OFFSET_HIGH << 28;

	// rc_tgt_offset_lo
	pps |= RC_TGT_OFFSET_LOW << 24;

	dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, 10), pps);

	// PPS44 - PPS57 rc_buf_threshold values
	for (i = 0, pps_index = 44; i < 14; ++i, ++pps_index) {
		pps = dptx_readl(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, (pps_index / 4)));
		pps |= (g_rc_buf_threshold[i] >> 6) << (pps_index % 4) * 8;
		dptx_writel(dptx, DPTX_DSC_PPS(DPTX_SST_MODE, pps_index / 4), pps);
	}

	// RC_RANGE_PARAMETERS from DSC 1.2 spec
	dptx_program_rc_range_parameters(dptx);
}

void dptx_program_pps_sdps(struct dp_ctrl* dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_program_dsc_version(dptx);
	dptx_program_dsc_buf_bit_depth(dptx);
	dptx_program_dsc_block_prediction(dptx);
	dptx_program_dsc_bits_perpixel(dptx);
	dptx_program_dsc_bpc_and_depth(dptx);
	dptx_program_dsc_slice_width(dptx);
	dptx_program_dsc_pic_width_height(dptx);
	dptx_program_dsc_slice_height(dptx);
	dptx_program_dsc_min_rate_bufsize(dptx);
	dptx_program_dsc_hrdelay(dptx);
	dptx_program_flatness_qp(dptx);
	dptx_program_rc_parameter_set(dptx);
}

void dptx_calc_dsc_lsteer_xmit_delay(struct dp_ctrl* dptx)
{
	int encoder_delay;
	uint32_t mux_word_size;
	uint32_t muxer_initial_delay;
	uint32_t reg;
	uint16_t h_active;
	uint16_t h_blanking;
	uint32_t lsteer_xmit_delay;
	uint8_t horizontal_slices;
	uint8_t vertical_slices;
	uint8_t bpc;
	uint8_t multipixel;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	bpc = dptx->vparams.dsc_bpc;
	mux_word_size = (bpc < 12) ? 48 : 64;
	muxer_initial_delay = (mux_word_size + (4 * bpc + 4) - 3 + 32) * 3;
	horizontal_slices = dptx->vparams.encoders;
	multipixel = dptx->multipixel;
	h_active = dptx->vparams.mdtd.h_active;
	h_blanking = dptx->vparams.mdtd.h_blanking;
	vertical_slices= dptx->vparams.mdtd.v_active / dptx->vparams.slice_height;

	encoder_delay = (((PIXEL_HOLD_DELAY + PIXEL_FLATNESSBUF_DELAY +
		PIXEL_GROUP_DELAY + muxer_initial_delay +
		MUXER_INITIAL_BUFFERING_DELAY) * horizontal_slices)) /
		(1 << multipixel);

	lsteer_xmit_delay = encoder_delay + (((h_blanking + h_active) * vertical_slices) +
		h_blanking + h_active +  (INITIAL_DELAY * horizontal_slices)) / (1 << multipixel);

	HISI_FB_DEBUG("[DP] muxer inital delay  = %d\n", muxer_initial_delay);
	HISI_FB_DEBUG("[DP] DSC encoder delay = %d\n", encoder_delay);
	HISI_FB_DEBUG("[DP] DSC XMIT delay = %d\n", lsteer_xmit_delay);

	reg = dptx_readl(dptx, DPTX_VIDEO_DSCCFG);
	reg &= ~DPTX_DSC_LSTEER_XMIT_DELAY_MASK;
	reg |= lsteer_xmit_delay << DPTX_DSC_LSTEER_XMIT_DELAY_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_DSCCFG, reg);
}

void dptx_dsc_compress_video(struct dp_ctrl* dptx)
{
	uint8_t encoders;
	uint16_t bpp;
	uint32_t reg;
	uint32_t wait_cnt_int;
	uint32_t wait_cnt_frac;
	s64 fixp;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	bpp = dptx->vparams.dsc_bpp;
	encoders = dptx->vparams.encoders;

	// Get the integer part
	fixp = drm_fixp_from_fraction(128, (bpp * encoders));
	wait_cnt_int = drm_fixp2int(fixp);

	// Get the fractional part
	fixp &= DRM_FIXED_DECIMAL_MASK;
	fixp *= 64;
	wait_cnt_frac = drm_fixp2int(fixp);

	HISI_FB_INFO("[DP] wait_cnt_int = %u, wait_cnt_frac = %u\n", wait_cnt_int, wait_cnt_frac);

	reg = dptx_readl(dptx, DPTX_VIDEO_DSCCFG);
	reg &= ~DPTX_DSC_LSTEER_INT_SHIFT_MASK;
	reg &= ~DPTX_DSC_LSTEER_FRAC_SHIFT_MASK;
	reg |= wait_cnt_int << DPTX_DSC_LSTEER_INT_SHIFT;
	reg |= wait_cnt_frac << DPTX_DSC_LSTEER_FRAC_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_DSCCFG, reg);

	dptx_calc_dsc_lsteer_xmit_delay(dptx);
}

void dptx_dsc_enable(struct dp_ctrl* dptx)
{
	int retval;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx->vparams.encoders = dptx->dsc_decoders;
	retval = dptx_dsc_encoders_count(dptx);
	if (retval)
		HISI_FB_ERR("[DP] Calc DSC encoder count error!\n");

	retval = dptx_mark_encoders(dptx);
	if (retval)
		HISI_FB_ERR("[DP] mark_encoders error!\n");

	retval = dptx_write_dpcd(dptx, DP_DSC_ENABLE, 0x1);
	if (retval)
		HISI_FB_ERR("[DP] Fail to write DPCD\n");

	// Apply soft reset - stream 0 for SST mode
	dptx_soft_reset(dptx, DPTX_SRST_VIDEO_RESET_N(0));
	dptx_pixel_mode_based_on_encoder_count(dptx);

	// Program PPS table
	dptx_program_pps_sdps(dptx);
	dptx_dsc_compress_video(dptx);
}
#endif

void dptx_video_core_config(struct dp_ctrl *dptx, int stream)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;
	uint32_t reg;
	uint8_t vmode;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	vmode = vparams->mode;

	dptx_video_set_core_bpc(dptx, stream);
#ifdef CONFIG_HISI_FB_V510
	/* Single, dual, or quad pixel */
	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_MULTI_PIXEL_MASK;
	reg |= dptx->multipixel << DPTX_VSAMPLE_CTRL_MULTI_PIXEL_SHIFT;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);
#endif
	/* Configure DPTX_VSAMPLE_POLARITY_CTRL register */
	reg = 0;

	if (mdtd->h_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_H_SYNC_POL_EN;
	if (mdtd->v_sync_polarity == 1)
		reg |= DPTX_POL_CTRL_V_SYNC_POL_EN;

	dptx_writel(dptx, DPTX_VSAMPLE_POLARITY_CTRL_N(stream), reg);

	reg = 0;

	/* Configure video_config1 register */
	if (vparams->video_format == VCEA) {
		if (vmode == 5 || vmode == 6 || vmode == 7 ||
		    vmode == 10 || vmode == 11 || vmode == 20 ||
		    vmode == 21 || vmode == 22 || vmode == 39 ||
		    vmode == 25 || vmode == 26 || vmode == 40 ||
		    vmode == 44 || vmode == 45 || vmode == 46 ||
		    vmode == 50 || vmode == 51 || vmode == 54 ||
		    vmode == 55 || vmode == 58 || vmode  == 59)
			reg |= DPTX_VIDEO_CONFIG1_IN_OSC_EN;
	}

	if (mdtd->interlaced == 1)
		reg |= DPTX_VIDEO_CONFIG1_O_IP_EN;

	reg |= mdtd->h_active << DPTX_VIDEO_H_ACTIVE_SHIFT;
	reg |= mdtd->h_blanking << DPTX_VIDEO_H_BLANK_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG1_N(stream), reg);

	/* Configure video_config2 register */
	reg = 0;
	reg |= mdtd->v_active << DPTX_VIDEO_V_ACTIVE_SHIFT;
	reg |= mdtd->v_blanking << DPTX_VIDEO_V_BLANK_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG2_N(stream), reg);

	/* Configure video_config3 register */
	reg = 0;
	reg |= mdtd->h_sync_offset << DPTX_VIDEO_H_FRONT_PORCH;
	reg |= mdtd->h_sync_pulse_width << DPTX_VIDEO_H_SYNC_WIDTH;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG3_N(stream), reg);

	/* Configure video_config4 register */
	reg = 0;
	reg |= mdtd->v_sync_offset << DPTX_VIDEO_V_FRONT_PORCH;
	reg |= mdtd->v_sync_pulse_width << DPTX_VIDEO_V_SYNC_WIDTH;
	dptx_writel(dptx, DPTX_VIDEO_CONFIG4_N(stream), reg);

	/* Configure video_config5 register */
	dptx_video_ts_change(dptx, stream);

	/* Configure video_msa1 register */
	reg = 0;
	reg |= (mdtd->h_blanking - mdtd->h_sync_offset)
		<< DPTX_VIDEO_MSA1_H_START_SHIFT;
	reg |= (mdtd->v_blanking - mdtd->v_sync_offset)
		<< DPTX_VIDEO_MSA1_V_START_SHIFT;
	dptx_writel(dptx, DPTX_VIDEO_MSA1_N(stream), reg);

	dptx_video_set_sink_bpc(dptx, stream);
#ifdef CONFIG_HISI_FB_V510
	reg = dptx_calculate_hblank_interval(dptx);
	reg |= (DPTX_VIDEO_HBLANK_INTERVAL_ENABLE << DPTX_VIDEO_HBLANK_INTERVAL_SHIFT);
	dptx_writel(dptx, DPTX_VIDEO_HBLANK_INTERVAL, reg);
#endif
}

#ifdef CONFIG_HISI_FB_V510
uint8_t dptx_calculate_dsc_init_threshold(struct dp_ctrl* dptx)
{
	uint32_t link_pixel_clock_ratio;
	uint16_t pixle_push_rate;
	uint8_t lanes;
	uint8_t tu;
	uint16_t slot_count;
	uint8_t fec_slot_count;
	uint32_t link_clk;
	uint64_t pixel_clk;
	uint16_t dsc_bpp;
	uint8_t rate;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	tu = dptx->vparams.aver_bytes_per_tu;
	lanes = dptx->link.lanes;
	dsc_bpp = dptx->vparams.dsc_bpp;


	if (dptx->fec) {
		if (lanes == 1) {
			fec_slot_count = 13;
		} else {
			fec_slot_count = 7;
		}
	} else {
		fec_slot_count = 0;
	}

	pixle_push_rate = (8 / dsc_bpp) * lanes;

	if (tu > 0) {
		slot_count = tu + 1 + fec_slot_count;
	} else {
		slot_count = tu + fec_slot_count;
	}

	slot_count = ROUND_UP_TO_NEAREST(slot_count, 4);
	pixel_clk = dptx->vparams.mdtd.pixel_clock;

	rate = dptx->link.rate;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_clk = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_clk = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_clk = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_clk = 202500;
		break;
	default:
		WARN(1, "Invalid rate 0x%x\n", rate);
		return -EINVAL;
	}

	link_pixel_clock_ratio = link_clk / pixel_clk;

	return (uint8_t)(pixle_push_rate * link_pixel_clock_ratio * slot_count);
}

int dptx_calculate_mst_init_threshold(struct dp_ctrl *dptx, int lane_num, int bpc,
	int encoding, int pixel_clock, int link_rate, int link_clk)
{
	struct video_params *vparams;
	struct dtd *mdtd;
	uint32_t tu_mst;
	uint32_t tu_frac_mst;
	int numerator;
	int denominator;
	s64 fixp;
	int lane_count;
	int fec_slot_count;
	int slot_count;
	int num_lanes_divisor;
	int slot_count_adjust;
	int mst_T1 = 0;
	int mst_T2;
	int mst_T3;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	numerator = mdtd->pixel_clock * 3 * 10;  //synopsys set value;
	denominator = (link_rate / 10) * lane_num * 100 * 1000;

	HISI_FB_INFO("[DP] MST: pixel_clock=%llu, MST: numerator=%d, denominator=%d\n",
		mdtd->pixel_clock, numerator, denominator);

	fixp = drm_fixp_from_fraction(numerator * 64, denominator);
	tu_mst = drm_fixp2int(fixp);

	lane_count = dptx->link.lanes;

	if(dptx->fec) {
		if(lane_count == 1)
			fec_slot_count = 13;
		else
			fec_slot_count = 7;

	} else {
		fec_slot_count = 0;
	}

	fixp &= DRM_FIXED_DECIMAL_MASK;
	fixp *= 64;
	tu_frac_mst = drm_fixp2int_ceil(fixp);

	HISI_FB_INFO("[DP] MST: tu = %d, tu_frac = %d\n", tu_mst, tu_frac_mst);
	vparams->aver_bytes_per_tu = tu_mst;
	vparams->aver_bytes_per_tu_frac = tu_frac_mst;

	if(tu_mst > 0)
		slot_count = tu_mst + 1 + fec_slot_count;
	else
		slot_count = tu_mst + fec_slot_count;

	switch (bpc) {
	case COLOR_DEPTH_6:
		if (encoding == RGB || encoding == YCBCR420)
			mst_T1 = 72000 / 36;
		break;
	case COLOR_DEPTH_8:
		if (encoding == RGB || encoding == YCBCR420) {
			mst_T1 = 16000 / 12;
		} else if (encoding == YCBCR422) {
			mst_T1 = 8000 / 4;
		}
		break;
	case COLOR_DEPTH_10:
		if (encoding == RGB || encoding == YCBCR420) {
			mst_T1 = 64000 / 60;
		} else if (encoding == YCBCR422) {
			mst_T1 = 32000 / 20;
		}
		break;
	case COLOR_DEPTH_12:
		if (encoding == RGB || encoding == YCBCR420) {
			mst_T1 = 32000 / 36;
		} else if (encoding == YCBCR422) {
			mst_T1 = 16000 / 12;
		}
		break;
	case COLOR_DEPTH_16:
		if (encoding == RGB || encoding == YCBCR420) {
			mst_T1 = 8000 / 12;
		} else if (encoding == YCBCR422) {
			mst_T1 = 4000 / 4;
		}
		break;
	default:
		HISI_FB_DEBUG("Invalid param BPC = %d\n" , bpc);
		return -EINVAL;
		break;
	}

	if (encoding == YCBCR420)
		pixel_clock = pixel_clock / 2;

	mst_T2 = (link_clk * 1000 / pixel_clock);

	if(lane_count == 1) {
		num_lanes_divisor = 4;
		slot_count_adjust = 3;
	} else if (lane_count == 2) {
		num_lanes_divisor = 2;
		slot_count_adjust = 1;
	} else {
		num_lanes_divisor = 1;
		slot_count_adjust = 0;
	}

	mst_T3 = ((slot_count + slot_count_adjust) / num_lanes_divisor) + 8;

	vparams->init_threshold = mst_T1 * mst_T2 * mst_T3 / (1000 * 1000);

	HISI_FB_INFO("[DP] T1 = %d, T2 =%d, tu = %d, vparams->init_threshold = %d \n", mst_T1, mst_T2, mst_T3, vparams->init_threshold);

	return 0;
}

int dptx_calculate_init_threshold(struct dp_ctrl *dptx, int lane_num, int bpc,
	int encoding, int pixel_clock, int tu, int link_clk)
{
	struct video_params *vparams;
	struct dtd *mdtd;
	int T1 = 0;
	int T2 = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	// Single Pixel Mode
	if (dptx->multipixel == DPTX_MP_SINGLE_PIXEL) {
		if (tu < 6) {
			vparams->init_threshold = 32;
		} else if (mdtd->h_blanking <= 40 && encoding == YCBCR420) {
			vparams->init_threshold = 3;
		} else if (mdtd->h_blanking <= 80  && encoding != YCBCR420) {
			vparams->init_threshold = 12;
		} else {
			vparams->init_threshold = 16;
		}
	} else {
	//Multiple Pixel Mode
		switch (bpc) {
			case COLOR_DEPTH_6:
				T1 = (4 * 1000 / 9) * lane_num;
				break;
			case COLOR_DEPTH_8:
				if (encoding == YCBCR422) {
					T1 = (1 * 1000 / 2) * lane_num;
				} else if (encoding == YONLY) {
					T1 = lane_num * 1000;
				} else {
					if(dptx->multipixel == DPTX_MP_DUAL_PIXEL)
						T1 = (1 * 1000 / 3) * lane_num;
					else
						T1 = (3 * 1000 / 16) * lane_num;
				}
				break;
			case COLOR_DEPTH_10:
				if (encoding == YCBCR422) {
					T1 = (2 * 1000 / 5) * lane_num;
				} else if (encoding == YONLY) {
					T1 = (4 * 1000 / 5) * lane_num;
				} else {
					T1 = (4 * 1000 / 15) * lane_num;
				}
				break;
			case COLOR_DEPTH_12:
				if (encoding == YCBCR422) {
					if(dptx->multipixel == DPTX_MP_DUAL_PIXEL)
						T1 = (1 * 1000 / 6) * lane_num;
					else
						T1 = (1 * 1000 / 3) * lane_num;
				} else if (encoding == YONLY) {
					T1 = (2 * 1000 / 3) * lane_num;
				} else {
					T1 = (2 * 1000 / 9) * lane_num;
				}
				break;
			case COLOR_DEPTH_16:
				if (encoding == YONLY)
					T1 = (1 * 1000 / 2) * lane_num;

				if ((encoding != YONLY) && (encoding != YCBCR422) &&
					(dptx->multipixel == DPTX_MP_DUAL_PIXEL)) {
					T1 = (1 * 1000 / 6) * lane_num;
				} else {
					T1 = (1 * 1000 / 4) * lane_num;
				}
				break;
			default:
				HISI_FB_DEBUG("Invalid param BPC = %d\n" , bpc);
				return -EINVAL;
			break;
		}

		if (encoding == YCBCR420)
			pixel_clock = pixel_clock / 2;

		T2 = (link_clk * 1000 / pixel_clock);

		vparams->init_threshold = T1 * T2 * tu / (1000 * 1000);
	}
	return 0;
}

int dptx_video_ts_calculate(struct dp_ctrl *dptx, int lane_num, int rate,
			    int bpc, int encoding, int pixel_clock)
{
	struct video_params *vparams;
	struct dtd *mdtd;
	int link_rate;
	int link_clk;
	int retval = 0;
	int ts;
	int tu;
	int tu_frac;
	int color_dep;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_rate = 162;
		link_clk = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_rate = 270;
		link_clk = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_rate = 540;
		link_clk = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_rate = 810;
		link_clk = 202500;
		break;
	default:
		HISI_FB_DEBUG("Invalid rate param = %d\n" , rate);
		return -EINVAL;
	}

	switch (bpc) {
	case COLOR_DEPTH_6:
		color_dep = 18;
		break;
	case COLOR_DEPTH_8:
		if (encoding == YCBCR420) {
			color_dep = 12;
		} else if (encoding == YCBCR422) {
			color_dep = 16;
		} else if (encoding == YONLY) {
			color_dep = 8;
		} else {
			color_dep = 24;
		}
		break;
	case COLOR_DEPTH_10:
		if (encoding == YCBCR420) {
			color_dep = 15;
		} else if (encoding == YCBCR422) {
			color_dep = 20;
		} else if (encoding  == YONLY) {
			color_dep = 10;
		} else {
			color_dep = 30;
		}
		break;
	case COLOR_DEPTH_12:
		if (encoding == YCBCR420) {
			color_dep = 18;
		} else if (encoding == YCBCR422) {
			color_dep = 24;
		} else if (encoding == YONLY) {
			color_dep = 12;
		} else {
			color_dep = 36;
		}
		break;
	case COLOR_DEPTH_16:
		if (encoding == YCBCR420) {
			color_dep = 24;
		} else if (encoding == YCBCR422) {
			color_dep = 32;
		} else if (encoding == YONLY) {
			color_dep = 16;
		} else {
			color_dep = 48;
		}
		break;
	default:
		color_dep = 18;
		break;
	}

	if (lane_num * link_rate == 0) {
		HISI_FB_ERR("[DP] lane_num = %d, link_rate = %d", lane_num, link_rate);
		return -EINVAL;
	}

	if (dptx->dsc)
		color_dep = dptx->vparams.dsc_bpp;

	ts = (8 * color_dep * pixel_clock) / (lane_num * link_rate);
	tu = ts / 1000;
	dp_imonitor_set_param(DP_PARAM_TU, &tu);

	if (tu >= 65) {
		HISI_FB_ERR("[DP] tu(%d) > 65", tu);
		return -EINVAL;
	}

	tu_frac = ts / 100 - tu * 10;

	// Calculate init_threshold for DSC mode
	if (dptx->dsc) {
		vparams->init_threshold = dptx_calculate_dsc_init_threshold(dptx);
		HISI_FB_DEBUG("calculated init_threshold for dsc = %d\n", vparams->init_threshold);
		if (vparams->init_threshold < 32) {
			vparams->init_threshold = 32;
			HISI_FB_DEBUG("Set init_threshold for dsc to %d\n", vparams->init_threshold);
		}
		// Calculate init_threshold for non DSC mode
	} else {
		retval = dptx_calculate_init_threshold(dptx, lane_num, bpc,
			encoding, pixel_clock, tu, link_clk);

		if (retval) {
			HISI_FB_ERR("[DP] NULL Pointer\n");
			return -EINVAL;
		}
	}

	HISI_FB_INFO("[DP] color_dep = %d, tu = %d, init_threshold = %d",color_dep, tu, vparams->init_threshold);

	vparams->aver_bytes_per_tu = (uint8_t)tu;

	vparams->aver_bytes_per_tu_frac = (uint8_t)tu_frac;

	if (dptx->mst) {
		retval = dptx_calculate_mst_init_threshold(dptx, lane_num, bpc,
			encoding, pixel_clock, link_rate, link_clk);
		if (retval) {
			HISI_FB_ERR("[DP] NULL Pointer\n");
			return -EINVAL;
		}
	}

	return retval;
}

void dptx_video_ts_change(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	struct video_params *vparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_VIDEO_CONFIG5_N(stream));
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_MASK);
	reg = reg | (vparams->aver_bytes_per_tu <<
			DPTX_VIDEO_CONFIG5_TU_SHIFT);
	if (dptx->mst) {
		reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_MST);
		reg = reg | (vparams->aver_bytes_per_tu_frac <<
			     DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_MST);
	} else {
		reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_SST);
		reg = reg | (vparams->aver_bytes_per_tu_frac <<
			     DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_SST);
	}

	reg = reg & (~DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_MASK);
	reg = reg | (vparams->init_threshold <<
			DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_CONFIG5_N(stream), reg);
}
#else

int dptx_video_ts_calculate(struct dp_ctrl *dptx, int lane_num, int rate,
			    int bpc, int encoding, int pixel_clock)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;
	int link_rate;
	int retval = 0;
	int ts;
	int tu;
	int tu_frac;
	int color_dep;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		link_rate = 162;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		link_rate = 270;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		link_rate = 540;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		link_rate = 810;
		break;
	default:
		link_rate = 162;
		break;
	}

	switch (bpc) {
	case COLOR_DEPTH_6:
		color_dep = 18;
		break;
	case COLOR_DEPTH_8:
		if (encoding == YCBCR420) {
			color_dep  = 12;
		} else if (encoding == YCBCR422) {
			color_dep = 16;
		} else if (encoding == YONLY) {
			color_dep = 8;
		} else {
			color_dep = 24;
		}
		break;
	case COLOR_DEPTH_10:
		if (encoding == YCBCR420) {
			color_dep = 15;
		} else if (encoding == YCBCR422) {
			color_dep = 20;
		} else if (encoding  == YONLY) {
			color_dep = 10;
		} else {
			color_dep = 30;
		}
		break;

	case COLOR_DEPTH_12:
		if (encoding == YCBCR420) {
			color_dep = 18;
		} else if (encoding == YCBCR422) {
			color_dep = 24;
		} else if (encoding == YONLY) {
			color_dep = 12;
		} else {
			color_dep = 36;
		}
		break;

	case COLOR_DEPTH_16:
		if (encoding == YCBCR420) {
			color_dep = 24;
		} else if (encoding == YCBCR422) {
			color_dep = 32;
		} else if (encoding == YONLY) {
			color_dep = 16;
		} else {
			color_dep = 48;
		}
		break;
	default:
		color_dep = 18;
		break;
	}

	if (lane_num * link_rate == 0) {
		HISI_FB_ERR("[DP] lane_num = %d, link_rate = %d", lane_num, link_rate);
		return -EINVAL;
	}

	ts = (8 * color_dep * pixel_clock) / (lane_num * link_rate);
	tu  = ts / 1000;
	dp_imonitor_set_param(DP_PARAM_TU, &tu);

	if (tu >= 65) {
		HISI_FB_ERR("[DP] tu(%d) > 65", tu);
		return -EINVAL;
	}

	tu_frac = ts / 100 - tu * 10;

	if (tu < 6)
		vparams->init_threshold = 32;
	else if ((encoding == RGB || encoding == YCBCR444) &&
		 mdtd->h_blanking <= 80)
		vparams->init_threshold = 12;
	else
		vparams->init_threshold = 15;

	HISI_FB_INFO("[DP] tu = %d\n", tu);

	vparams->aver_bytes_per_tu = (uint8_t)tu;

	vparams->aver_bytes_per_tu_frac = (uint8_t)tu_frac;

	return retval;
}

void dptx_video_ts_change(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	struct video_params *vparams = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;

	reg = (uint32_t)dptx_readl(dptx, DPTX_VIDEO_CONFIG5_N(stream));
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_MASK);
	reg = reg | (vparams->aver_bytes_per_tu <<
			DPTX_VIDEO_CONFIG5_TU_SHIFT);
	reg = reg & (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_SST);
	reg = reg | (vparams->aver_bytes_per_tu_frac <<
		       DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_SST);

	reg = reg & (~DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_MASK);
	reg = reg | (vparams->init_threshold <<
			DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_CONFIG5_N(stream), reg);

	return;
}

#endif

void dptx_video_bpc_change(struct dp_ctrl *dptx, int stream)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_video_set_core_bpc(dptx, stream);
	dptx_video_set_sink_bpc(dptx, stream);
}

void dptx_video_set_core_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg;
	uint8_t bpc_mapping = 0, bpc = 0;
	enum pixel_enc_type pix_enc;
	struct video_params *vparams = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	bpc = vparams->bpc;
	pix_enc = vparams->pix_enc;

	reg = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	reg &= ~DPTX_VSAMPLE_CTRL_VMAP_BPC_MASK;

	switch (pix_enc) {
	case RGB:
		if (bpc == COLOR_DEPTH_6)
			bpc_mapping = 0;
		else if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR444:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 5;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 6;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 7;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 8;
		break;
	case YCBCR422:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 9;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 10;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 11;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 12;
		break;
	case YCBCR420:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 13;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 14;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 15;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 16;
		break;
	case YONLY:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 17;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 18;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 19;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 20;
		break;
	case RAW:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 23;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 24;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 25;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 27;
		break;
	}

	if (dptx->dsc) {
		reg |= (1 << DPTX_VG_CONFIG1_BPC_SHIFT);
	} else {
		reg |= (bpc_mapping << DPTX_VSAMPLE_CTRL_VMAP_BPC_SHIFT);
	}

	reg |= (1 << DPTX_VSAMPLE_CTRL_VMAP_BPC_SHIFT); // this code is used to set 8 bit.

	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), reg);
}
void dptx_video_set_sink_col(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg_msa2;
	uint8_t col_mapping;
	uint8_t colorimetry;
	uint8_t dynamic_range;
	struct video_params *vparams = NULL;
	enum pixel_enc_type pix_enc;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	pix_enc = vparams->pix_enc;
	colorimetry = vparams->colorimetry;
	dynamic_range = vparams->dynamic_range;

	reg_msa2 = dptx_readl(dptx, DPTX_VIDEO_MSA2_N(stream));
	reg_msa2 &= ~DPTX_VIDEO_VMSA2_COL_MASK;

	col_mapping = 0;

	/* According to Table 2-94 of DisplayPort spec 1.3 */
	switch (pix_enc) {
	case RGB:
		if (dynamic_range == CEA)
			col_mapping = 4;
		else if (dynamic_range == VESA)
			col_mapping = 0;
		break;
	case YCBCR422:
		if (colorimetry == ITU601)
			col_mapping = 5;
		else if (colorimetry == ITU709)
			col_mapping = 13;
		break;
	case YCBCR444:
		if (colorimetry == ITU601)
			col_mapping = 6;
		else if (colorimetry == ITU709)
			col_mapping = 14;
		break;
	case RAW:
		col_mapping = 1;
		break;
	case YCBCR420:
	case YONLY:
		break;
	}

	reg_msa2 |= (col_mapping << DPTX_VIDEO_VMSA2_COL_SHIFT);
	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
}

void dptx_video_set_sink_bpc(struct dp_ctrl *dptx, int stream)
{
	uint32_t reg_msa2, reg_msa3;
	uint8_t bpc_mapping = 0, bpc = 0;
	struct video_params *vparams = NULL;
	enum pixel_enc_type pix_enc;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vparams = &dptx->vparams;
	pix_enc = vparams->pix_enc;
	bpc = vparams->bpc;

	reg_msa2 = dptx_readl(dptx, DPTX_VIDEO_MSA2_N(stream));
	reg_msa3 = dptx_readl(dptx, DPTX_VIDEO_MSA3_N(stream));

	reg_msa2 &= ~DPTX_VIDEO_VMSA2_BPC_MASK;
	reg_msa3 &= ~DPTX_VIDEO_VMSA3_PIX_ENC_MASK;

	switch (pix_enc) {
	case RGB:
		if (bpc == COLOR_DEPTH_6)
			bpc_mapping = 0;
		else if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR444:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR422:
		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case YCBCR420:
		reg_msa3 |= 1 << DPTX_VIDEO_VMSA3_PIX_ENC_YCBCR420_SHIFT;
		break;
	case YONLY:
		/* According to Table 2-94 of DisplayPort spec 1.3 */
		reg_msa3 |= 1 << DPTX_VIDEO_VMSA3_PIX_ENC_SHIFT;

		if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 2;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 3;
		if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 4;
		break;
	case RAW:
		 /* According to Table 2-94 of DisplayPort spec 1.3 */
		reg_msa3 |= (1 << DPTX_VIDEO_VMSA3_PIX_ENC_SHIFT);

		if (bpc == COLOR_DEPTH_6)
			bpc_mapping = 1;
		else if (bpc == COLOR_DEPTH_8)
			bpc_mapping = 3;
		else if (bpc == COLOR_DEPTH_10)
			bpc_mapping = 4;
		else if (bpc == COLOR_DEPTH_12)
			bpc_mapping = 5;
		else if (bpc == COLOR_DEPTH_16)
			bpc_mapping = 7;
		break;
	default:
		break;
	}

	reg_msa2 |= (bpc_mapping << DPTX_VIDEO_VMSA2_BPC_SHIFT);

	dptx_writel(dptx, DPTX_VIDEO_MSA2_N(stream), reg_msa2);
	dptx_writel(dptx, DPTX_VIDEO_MSA3_N(stream), reg_msa3);
	dptx_video_set_sink_col(dptx, stream);
}


void dptx_disable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	uint32_t vsamplectrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl &= ~DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dptx->dptx_vr) && (dptx->dptx_detect_inited)) {
		HISI_FB_INFO("[DP] Cancel dptx detect err count when disable video stream.\n");
		hrtimer_cancel(&dptx->dptx_hrtimer);
	}
}

void dptx_enable_default_video_stream(struct dp_ctrl *dptx, int stream)
{
	uint32_t vsamplectrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	vsamplectrl = dptx_readl(dptx, DPTX_VSAMPLE_CTRL_N(stream));
	vsamplectrl |= DPTX_VSAMPLE_CTRL_STREAM_EN;
	dptx_writel(dptx, DPTX_VSAMPLE_CTRL_N(stream), vsamplectrl);

	if ((dptx->dptx_vr) && (dptx->dptx_detect_inited)) {
		HISI_FB_INFO("[DP] restart dptx detect err count when enable video stream.\n");
		hrtimer_restart(&dptx->dptx_hrtimer);
	}
}

/*
 * Audio/Video Parameters
 */

void dptx_audio_params_reset(struct audio_params *params)
{
	if (params == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	memset(params, 0x0, sizeof(struct audio_params));
	params->iec_channel_numcl0 = 8;
	params->iec_channel_numcr0 = 4;
	params->use_lut = 1;
	params->iec_samp_freq = 3;
	params->iec_word_length = 11;
	params->iec_orig_samp_freq = 12;
	params->data_width = 16;
	params->num_channels = 2;
	params->inf_type = 1;
	params->ats_ver = 17;
	params->mute = 0;
}

void dptx_video_params_reset(struct video_params *params)
{
	if (params == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	memset(params, 0x0, sizeof(struct video_params));

	/* 6 bpc should be default - use 8 bpc for MST calculation */
	params->bpc = COLOR_DEPTH_8;
	params->dsc_bpp = DPTX_BITS_PER_PIXEL;
	params->pix_enc = RGB;
	params->mode = 1;
	params->colorimetry = ITU601;
	params->dynamic_range = CEA;
	params->aver_bytes_per_tu = 30;
	params->aver_bytes_per_tu_frac = 0;
	params->init_threshold = 15;
	params->pattern_mode = RAMP;
	params->refresh_rate = 60000;
	params->video_format = VCEA;
}

/*
 * DTD
 */

void dwc_dptx_dtd_reset(struct dtd *mdtd)
{
	if (mdtd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	mdtd->pixel_repetition_input = 0;
	mdtd->pixel_clock  = 0;
	mdtd->h_active = 0;
	mdtd->h_blanking = 0;
	mdtd->h_sync_offset = 0;
	mdtd->h_sync_pulse_width = 0;
	mdtd->h_image_size = 0;
	mdtd->v_active = 0;
	mdtd->v_blanking = 0;
	mdtd->v_sync_offset = 0;
	mdtd->v_sync_pulse_width = 0;
	mdtd->v_image_size = 0;
	mdtd->interlaced = 0;
	mdtd->v_sync_polarity = 0;
	mdtd->h_sync_polarity = 0;
}

bool dptx_dtd_fill(struct dtd *mdtd, uint8_t code, uint32_t refresh_rate,
		  uint8_t video_format)
{
	if (mdtd == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return false;
	}

	dwc_dptx_dtd_reset(mdtd);

	mdtd->h_image_size = 16;
	mdtd->v_image_size = 9;

	if (video_format == VCEA) {
		switch (code) {
		case 1: /* 640x480p @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 640;
			mdtd->v_active = 480;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 10;
			mdtd->h_sync_pulse_width = 96;
			mdtd->v_sync_pulse_width = 2;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 25175;
			break;
		case 2: /* 720x480p @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 3: /* 720x480p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 480;
			mdtd->h_blanking = 138;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 62;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			break;
		case 69:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 4: /* 1280x720p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 370;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 110;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 5: /* 1920x1080i @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 74250;
			break;
		case 6: /* 720(1440)x480i @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 7: /* 720(1440)x480i @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 27000;
			break;
		case 8: /* 720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 9: /* 720(1440)x240p @59.826/60.054/59.886/60.115Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = (refresh_rate == 59940) ? 22 : 23;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = (refresh_rate == 59940) ? 4 : 5;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			/*  else 60.115/59.886 Hz */
			break;
		case 10: /* 2880x480i @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 11: /* 2880x480i @ 59.94/60Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 240;
			mdtd->h_blanking = 552;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 76;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 248;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 12: /* 2880x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 13: /* 2880x240p @ 59.826/60.054/59.886/60.115Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 240;
			mdtd->h_blanking = 552;
			mdtd->v_blanking = (refresh_rate == 60054) ? 22 : 23;
			mdtd->h_sync_offset = 76;
			mdtd->v_sync_offset = (refresh_rate == 60054) ? 4 : 5;
			mdtd->h_sync_pulse_width = 248;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 14: /* 1440x480p @ 59.94/60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 15: /* 1440x480p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 480;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 32;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 76:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 16: /* 1920x1080p @ 59.94/60Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 17: /* 720x576p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 18: /* 720x576p @ 50Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 576;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 12;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 64;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			break;
		case 68:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 19: /* 1280x720p @ 50Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 700;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 440;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 20: /* 1920x1080i @ 50Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 74250;
			break;
		case 21: /* 720(1440)x576i @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 22: /* 720(1440)x576i @ 50Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 27000;
			break;
		case 23: /* 720(1440)x288p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 24: /* 720(1440)x288p @ 50Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = (refresh_rate == 50080) ? 24
				: ((refresh_rate == 49920) ? 25 : 26);
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = (refresh_rate == 50080) ? 2
				: ((refresh_rate == 49920) ? 3 : 4);
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 27000;
			break;
		case 25: /* 2880x576i @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 26: /* 2880x576i @ 50Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 288;
			mdtd->h_blanking = 576;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 252;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 27: /* 2880x288p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 28: /* 2880x288p @ 50Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 288;
			mdtd->h_blanking = 576;
			mdtd->v_blanking = (refresh_rate == 50080) ? 24
				: ((refresh_rate == 49920) ? 25 : 26);
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = (refresh_rate == 50080) ? 2
				: ((refresh_rate == 49920) ? 3 : 4);
			mdtd->h_sync_pulse_width = 252;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 29: /* 1440x576p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 30: /* 1440x576p @ 50Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 576;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 75:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 31: /* 1920x1080p @ 50Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 72:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 32: /* 1920x1080p @ 23.976/24Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 830;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 638;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 73:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 33: /* 1920x1080p @ 25Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 74:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 34: /* 1920x1080p @ 29.97/30Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 35: /* 2880x480p @ 60Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 36: /* 2880x480p @ 60Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 480;
			mdtd->h_blanking = 552;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 248;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 37: /* 2880x576p @ 50Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 38: /* 2880x576p @ 50Hz 16:9 */
			mdtd->h_active = 2880;
			mdtd->v_active = 576;
			mdtd->h_blanking = 576;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 256;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 39: /* 1920x1080i (1250 total) @ 50Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 85;
			mdtd->h_sync_offset = 32;
			mdtd->v_sync_offset = 23;
			mdtd->h_sync_pulse_width = 168;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 72000;
			break;
		case 40: /* 1920x1080i @ 100Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 148500;
			break;
		case 70:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 41: /* 1280x720p @ 100Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 700;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 440;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 42: /* 720x576p @ 100Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 43: /* 720x576p @ 100Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 576;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 12;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 64;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 44: /* 720(1440)x576i @ 100Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 45: /* 720(1440)x576i @ 100Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 46: /* 1920x1080i @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 540;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 148500;
			break;
		case 71:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 47: /* 1280x720p @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 370;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 110;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 148500;
			break;
		case 48: /* 720x480p @ 119.88/120Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 49: /* 720x480p @ 119.88/120Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 480;
			mdtd->h_blanking = 138;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 62;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 54000;
			break;
		case 50: /* 720(1440)x480i @ 119.88/120Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 51: /* 720(1440)x480i @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 54000;
			break;
		case 52: /* 720X576p @ 200Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 53: /* 720X576p @ 200Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 576;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 49;
			mdtd->h_sync_offset = 12;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 64;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 54: /* 720(1440)x576i @ 200Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 55: /* 720(1440)x576i @ 200Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 288;
			mdtd->h_blanking = 288;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 126;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 108000;
			break;
		case 56: /* 720x480p @ 239.76/240Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 57: /* 720x480p @ 239.76/240Hz 16:9 */
			mdtd->h_active = 720;
			mdtd->v_active = 480;
			mdtd->h_blanking = 138;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 9;
			mdtd->h_sync_pulse_width = 62;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 108000;
			break;
		case 58: /* 720(1440)x480i @ 239.76/240Hz 4:3 */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 59: /* 720(1440)x480i @ 239.76/240Hz 16:9 */
			mdtd->h_active = 1440;
			mdtd->v_active = 240;
			mdtd->h_blanking = 276;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 38;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 124;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 1;
			mdtd->pixel_clock = 108000;
			break;
		case 65:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 60: /* 1280x720p @ 23.97/24Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 2020;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1760;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 59400;
			break;
		case 66:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 61: /* 1280x720p @ 25Hz 16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 2680;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 2420;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 67:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 62: /* 1280x720p @ 29.97/30Hz  16:9 */
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 2020;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1760;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 74250;
			break;
		case 78:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 63: /* 1920x1080p @ 119.88/120Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 77:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 64: /* 1920x1080p @ 100Hz 16:9 */
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 720;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 528;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 79:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 1620;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1360;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 80:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 1488;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 1228;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 81:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 960;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 700;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 82:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 520;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 260;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 82500;
			break;
		case 83:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 520;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 260;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 99000;
			break;
		case 84:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 320;
			mdtd->v_blanking = 105;
			mdtd->h_sync_offset = 60;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 165000;
			break;
		case 85:
			mdtd->h_active = 1680;
			mdtd->v_active = 720;
			mdtd->h_blanking = 320;
			mdtd->v_blanking = 105;
			mdtd->h_sync_offset = 60;
			mdtd->v_sync_offset = 5;
			mdtd->h_sync_pulse_width = 40;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 198000;
			break;
		case 86:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 1190;
			mdtd->v_blanking = 20;
			mdtd->h_sync_offset = 998;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 99000;
			break;
		case 87:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 640;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 448;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 90000;
			break;
		case 88:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 960;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 768;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 118800;
			break;
		case 89:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 740;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 548;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 185625;
			break;
		case 90:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 440;
			mdtd->v_blanking = 20;
			mdtd->h_sync_offset = 248;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 198000;
			break;
		case 91:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 410;
			mdtd->v_blanking = 170;
			mdtd->h_sync_offset = 218;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 371250;
			break;
		case 92:
			mdtd->h_active = 2560;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 740;
			mdtd->v_blanking = 170;
			mdtd->h_sync_offset = 548;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 495000;
			break;
		case 99:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1184;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 968;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 100:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 304;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 101:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1184;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 968;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 102:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 304;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 103:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 93:		/* 4k x 2k, 30Hz */
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1660;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1276;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 104:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 94:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1440;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1056;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 105:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 95:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 176;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 106:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 96:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1440;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1056;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		case 107:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
		case 97:
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 176;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 594000;
			break;
		case 98:
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 1404;
			mdtd->v_blanking = 90;
			mdtd->h_sync_offset = 1020;
			mdtd->v_sync_offset = 8;
			mdtd->h_sync_pulse_width = 88;
			mdtd->v_sync_pulse_width = 10;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;
			mdtd->pixel_clock = 297000;
			break;
		default:
			return false;
		}
	} else if (video_format == CVT) {
		switch (code) {
		case 1:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 640;
			mdtd->v_active = 480;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 20;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 23750;
			break;
		case 2:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 224;
			mdtd->v_blanking = 24;
			mdtd->h_sync_offset = 31;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 81;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 38250;
			break;
		case 3:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1024;
			mdtd->v_active = 768;
			mdtd->h_blanking = 304;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 104;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 63500;
			break;
		case 4:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 960;
			mdtd->h_blanking = 416;
			mdtd->v_blanking = 36;
			mdtd->h_sync_offset = 80;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 101250;
			break;
		case 5:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1400;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 464;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 144;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 121750;
			break;
		case 6:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 112;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 68;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 161000;
			break;
		case 12:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 1024;
			mdtd->h_blanking = 432;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 80;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 136;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 109000;
			break;
		case 13:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 768;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 79500;
			break;
		case 16:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 720;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 28;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 74500;
			break;
		case 17:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1360;
			mdtd->v_active = 768;
			mdtd->h_blanking = 416;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 72;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 136;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 84750;
			break;
		case 20:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 656;
			mdtd->v_blanking = 40;
			mdtd->h_sync_offset = 128;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 173000;
			break;
		case 22:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2560;
			mdtd->v_active = 1440;
			mdtd->h_blanking = 928;
			mdtd->v_blanking = 53;
			mdtd->h_sync_offset = 192;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 272;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 312250;
			break;
		case 28:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 800;
			mdtd->h_blanking = 400;
			mdtd->v_blanking = 31;
			mdtd->h_sync_offset = 72;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 83500;
			break;
		case 34:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 672;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 136;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 38:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 3840;
			mdtd->v_active = 2400;
			mdtd->h_blanking = 80;
			mdtd->v_blanking = 69;
			mdtd->h_sync_offset = 320;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 424;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 580128;
			break;
		case 40:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 35;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 124076;
			break;
		case 41:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2048;
			mdtd->v_active = 1536;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 44;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 208000;
			break;
		default:
			return false;
		}
	} else if (video_format == DMT) {
		switch (code) {
		case 1: // HISilicon timing
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 3600;
			mdtd->v_active = 1800;
			mdtd->h_blanking = 120;
			mdtd->v_blanking = 128;
			mdtd->h_sync_offset = 20;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 20;
			mdtd->v_sync_pulse_width = 2;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 645500;
			break;
		case 2:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 3840;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 62;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 533000;
			break;
		case 4:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 640;
			mdtd->v_active = 480;
			mdtd->h_blanking = 144;
			mdtd->v_blanking = 29;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 2;
			mdtd->h_sync_pulse_width = 96;
			mdtd->v_sync_pulse_width = 2;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 25175;
			break;
		case 11:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 256;
			mdtd->v_blanking = 25;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 80;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 49500;
			break;
		case 13:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 36;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 73250;
			break;
		case 14: /* 848x480p@60Hz */
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 848;
			mdtd->v_active = 480;
			mdtd->h_blanking = 240;
			mdtd->v_blanking = 37;
			mdtd->h_sync_offset = 16;
			mdtd->v_sync_offset = 6;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI)  */;
			mdtd->pixel_clock = 33750;
			break;
		case 22:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 768;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 22;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 68250;
			break;
		case 35:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 1024;
			mdtd->h_blanking = 408;
			mdtd->v_blanking = 42;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 108000;
			break;
		case 39:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1360;
			mdtd->v_active = 768;
			mdtd->h_blanking = 432;
			mdtd->v_blanking = 27;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 85500;
			break;
		case 40:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1360;
			mdtd->v_active = 768;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 148250;
			break;
		case 81:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1366;
			mdtd->v_active = 768;
			mdtd->h_blanking = 426;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 70;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 142;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 85500;
			break;
		case 86:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1366;
			mdtd->v_active = 768;
			mdtd->h_blanking = 134;
			mdtd->v_blanking = 32;
			mdtd->h_sync_offset = 14;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 56;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 72000;
			break;
		case 87:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 80;
			mdtd->v_blanking = 62;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 48;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 556744;
			break;
		case 88:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 4096;
			mdtd->v_active = 2160;
			mdtd->h_blanking = 80;
			mdtd->v_blanking = 62;
			mdtd->h_sync_offset = 8;
			mdtd->v_sync_offset = 48;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 8;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 556188;
			break;
		case 41:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1400;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 101000;
			break;
		case 42:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1400;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 464;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 144;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 121750;
			break;
		case 46:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1440;
			mdtd->v_active = 900;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 26;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 88750;
			break;
		case 47:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1440;
			mdtd->v_active = 900;
			mdtd->h_blanking = 464;
			mdtd->v_blanking = 34;
			mdtd->h_sync_offset = 80;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 152;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 106500;
			break;
		case 51:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 50;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 192;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 162000;
			break;
		case 57:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1680;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 119000;
			break;
		case 58:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1680;
			mdtd->v_active = 1050;
			mdtd->h_blanking = 560;
			mdtd->v_blanking = 39;
			mdtd->h_sync_offset = 104;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 176;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 146250;
			break;
		case 68:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 35;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 154000;
			break;
		case 69:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1200;
			mdtd->h_blanking = 672;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 136;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 76:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2560;
			mdtd->v_active = 1600;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 46;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 77:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 2560;
			mdtd->v_active = 1600;
			mdtd->h_blanking = 940;
			mdtd->v_blanking = 58;
			mdtd->h_sync_offset = 192;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 280;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 193250;
			break;
		case 82:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1080;
			mdtd->h_blanking = 280;
			mdtd->v_blanking = 45;
			mdtd->h_sync_offset = 88;
			mdtd->v_sync_offset = 4;
			mdtd->h_sync_pulse_width = 44;
			mdtd->v_sync_pulse_width = 5;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock  = 148500;
			break;
		case 83:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1600;
			mdtd->v_active = 900;
			mdtd->h_blanking = 200;
			mdtd->v_blanking = 100;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 80;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 108000;
			break;
		case 9:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 800;
			mdtd->v_active = 600;
			mdtd->h_blanking = 256;
			mdtd->v_blanking = 28;
			mdtd->h_sync_offset = 40;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 4;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 40000;
			break;
		case 16:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1024;
			mdtd->v_active = 768;
			mdtd->h_blanking = 320;
			mdtd->v_blanking = 38;
			mdtd->h_sync_offset = 24;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 136;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 65000;
			break;
		case 23:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 768;
			mdtd->h_blanking = 384;
			mdtd->v_blanking = 30;
			mdtd->h_sync_offset = 64;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 128;
			mdtd->v_sync_pulse_width = 7;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 79500;
			break;
		case 62:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active =  1792;
			mdtd->v_active = 1344;
			mdtd->h_blanking = 656;
			mdtd->v_blanking =  50;
			mdtd->h_sync_offset = 128;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 200;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0; /* (progressive_nI) */
			mdtd->pixel_clock = 204750;
			break;
		case 32:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 960;
			mdtd->h_blanking = 520;
			mdtd->v_blanking = 40;
			mdtd->h_sync_offset = 96;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 112;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;  /* (progressive_nI) */
			mdtd->pixel_clock = 108000;
			break;
		case 73:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1920;
			mdtd->v_active = 1440;
			mdtd->h_blanking = 680;
			mdtd->v_blanking = 60;
			mdtd->h_sync_offset = 128;
			mdtd->v_sync_offset = 1;
			mdtd->h_sync_pulse_width = 208;
			mdtd->v_sync_pulse_width = 3;
			mdtd->h_sync_polarity = 0;
			mdtd->v_sync_polarity = 1;
			mdtd->interlaced = 0;  /* (progressive_nI) */
			mdtd->pixel_clock = 234000;
			break;
		case 27:
			mdtd->h_image_size = 4;
			mdtd->v_image_size = 3;
			mdtd->h_active = 1280;
			mdtd->v_active = 800;
			mdtd->h_blanking = 160;
			mdtd->v_blanking = 23;
			mdtd->h_sync_offset = 48;
			mdtd->v_sync_offset = 3;
			mdtd->h_sync_pulse_width = 32;
			mdtd->v_sync_pulse_width = 6;
			mdtd->h_sync_polarity = 1;
			mdtd->v_sync_polarity = 0;
			mdtd->interlaced = 0;  /* (progressive_nI) */
			mdtd->pixel_clock = 71000;
			break;
		default:
			return false;
		}
	} else {
		HISI_FB_ERR("[DP] Video Format is ERROR\n");
		return false;
	}

	return true;
}
