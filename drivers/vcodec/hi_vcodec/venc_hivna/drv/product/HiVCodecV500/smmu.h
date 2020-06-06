/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2010-2019. All rights reserved.
 * Description: smmu description
 */

#ifndef __HIVDEC_SMMU_H__
#define __HIVDEC_SMMU_H__

#include "hi_type.h"
#include "drv_venc_ioctl.h"
#include "hi_drv_mem.h"

#define SMMU_OK     0
#define SMMU_ERR    -1

#define SECURE_ON    1
#define SECURE_OFF   0
#define SMMU_ON      1
#define SMMU_OFF     0

enum {
	REFLD_LUMA = 0,
	REFLD_CHROMA,
	REFLD_H_LUMA,
	REFLD_H_CHROMA,
	VCPI_LD,
	QPG_LD_QP,
	QPG_LD_CELL,
	VLC_LD_PTR_WR,
	VLC_LD_PTR_RD,
	NBI_LD,
	PMV_LD_DATA,
	PMV_LD_CELL,
	LOW_DELAY,
	CURLD_Y,
	CURLD_U,
	CURLD_V,
	CURLD_YH,
	CURLD_CH,
	PMEINFO_LD_FLAG,
	PMEINFO_LD_QPG,
	PMEINFO_LD_SKIP_WGT,
	PME_LD,
	REC_ST_LUMA,
	REC_ST_CHROMA,
	REC_ST_CELL,
	PMV_ST_DATA,
	PMV_ST_CELL,
	QPG_ST_QP,
	QPG_ST_CELL,
	VLC_ST_PTR_WR,
	VLC_ST_PTR_RD,
	VLC_ST_STEAM,
	PMEINFO_ST_FLAG,
	NBI_ST,
	PME_ST,
	REC_ST_H_LUMA,
	REC_ST_H_CHROMA,
};

HI_S32 venc_smmu_init(HI_U32 *pRegBase);
HI_S32 venc_smmu_cfg(struct encode_info *channelcfg, HI_U32 *reg_base);
HI_VOID venc_smmu_debug(HI_U32 *reg_base, HI_BOOL first_cfg_flag);
#endif
