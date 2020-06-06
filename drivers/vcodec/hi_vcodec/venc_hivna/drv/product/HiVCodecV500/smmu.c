/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2010-2019. All rights reserved.
 * Description: smmu description
 */

#include "smmu.h"
#include "smmu_regs.h"
#include "soc_venc_reg_interface.h"
#include "hi_drv_mem.h"
#include "drv_venc_efl.h"
#include "drv_venc_osal.h"

#define SMRx_ID_SIZE                 37
#define HIVENC_SMMU_COMMON_OFFSET    0x20000
#define HIVENC_SMMU_MASTER_OFFSET    0x1A000

#define MSTR_RIGHT_SHIFT  15
#define BIT_WEIGHT        21
#define START_BIT         0

typedef struct {
	HI_S32 *pSMMUCommonBaseVirAddr;
	HI_S32 *pSMMUMasterBaseVirAddr;
} SMMU_REG_VIR_S;

SMMU_REG_VIR_S gVencSmmuRegVir;

HI_S32 gVencSmmuInitFlag = 0;
extern venc_smmu_err_add_t g_smmu_err_mem;

#define RD_SMMU_COMMON_VREG(reg, dat)               \
do {                    \
	dat = *((volatile HI_S32*)((HI_S8 *)gVencSmmuRegVir.pSMMUCommonBaseVirAddr + reg)); \
} while(0)

#define WR_SMMU_COMMON_VREG(reg, dat)               \
do {                    \
	*((volatile HI_S32*)((HI_S8*)gVencSmmuRegVir.pSMMUCommonBaseVirAddr + reg)) = dat; \
} while(0)

#define RD_SMMU_MASTER_VREG(reg, dat)               \
do {                    \
	dat = *((volatile HI_S32*)((HI_S8*)gVencSmmuRegVir.pSMMUMasterBaseVirAddr + reg)); \
} while(0)

#define WR_SMMU_MASTER_VREG(reg, dat)               \
do {                    \
	*((volatile HI_S32*)((HI_S8*)gVencSmmuRegVir.pSMMUMasterBaseVirAddr + reg)) = dat; \
} while(0)

static HI_VOID set_common_reg(HI_S32 addr, HI_S32 val, HI_U32 bw, HI_U32 bs)
{
	HI_S32 mask = (1UL << bw) - 1UL;
	HI_S32 tmp = 0;

	RD_SMMU_COMMON_VREG(addr, tmp);
	tmp &= ~(mask << bs);/*lint !e502*/
	WR_SMMU_COMMON_VREG(addr, tmp | ((val & mask) << bs));/*lint !e665*/
}

static HI_VOID set_master_reg(HI_S32 addr, HI_S32 val, HI_U32 bw, HI_U32 bs)
{
	HI_S32 mask = (1UL << bw) - 1UL;
	HI_S32 tmp = 0;

	RD_SMMU_MASTER_VREG(addr, tmp);
	tmp &= ~(mask << bs);/*lint !e502*/
	WR_SMMU_MASTER_VREG(addr, tmp | ((val & mask) << bs));/*lint !e665*/
}

HI_S32 venc_smmu_init(HI_U32 *pRegBase)
{
	HI_U32  i = 0;
	HI_U64 smmu_page_base_addr = venc_get_smmu_ttb();

	memset(&gVencSmmuRegVir, 0, sizeof(SMMU_REG_VIR_S));    /* unsafe_function_ignore: memset */
	gVencSmmuRegVir.pSMMUCommonBaseVirAddr   = (HI_U32 *)((HI_U64)(uintptr_t)pRegBase + HIVENC_SMMU_COMMON_OFFSET);
	if (NULL == gVencSmmuRegVir.pSMMUCommonBaseVirAddr) {
		HI_FATAL_VENC("pSMMUCommonBaseVirAddr is NULL\n");
		return SMMU_ERR;
	}

	gVencSmmuRegVir.pSMMUMasterBaseVirAddr   = (HI_U32 *)((HI_U64)(uintptr_t)pRegBase + HIVENC_SMMU_MASTER_OFFSET);
	if (NULL == gVencSmmuRegVir.pSMMUMasterBaseVirAddr) {
		HI_FATAL_VENC("pSMMUMasterBaseVirAddr is NULL \n");
		return SMMU_ERR;
	}

	/* smmu master */
	set_master_reg(SMMU_MSTR_GLB_BYPASS, 0x0, 32, 0);

	for (i = 0; i < SMRx_ID_SIZE; i++) {
		set_master_reg(SMMU_MSTR_SMRX_0 + i * 0x4, 0x1000, 32, 0); // len = 1; upwin = 0;
		set_common_reg(SMMU_SMRx_NS + i * 0x4, 0xbc,  32, 0);
	}

	/* mstr refld_luma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 0 * 0x4, 0x41, 8, 4); //upwin = 65
	set_master_reg(SMMU_MSTR_SMRX_0 + 0 * 0x4, 0x14, 8, 12); //len = 20

	/* mstr  refld_chroma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 1 * 0x4, 0x17, 8, 4); //upwin = 0x17
	set_master_reg(SMMU_MSTR_SMRX_0 + 1 * 0x4, 0xA, 8, 12); //len  = 0xA

	/* mst curld_y */
	set_master_reg(SMMU_MSTR_SMRX_0 + 13 * 0x4, 0, 8, 4); //upwin = 0
	set_master_reg(SMMU_MSTR_SMRX_0 + 13 * 0x4, 0x40, 8, 12); //len  = 0x40

	/* mst curld_u */
	set_master_reg(SMMU_MSTR_SMRX_0 + 14 * 0x4, 0, 8, 4); //upwin = 0
	set_master_reg(SMMU_MSTR_SMRX_0 + 14 * 0x4, 0x10, 8, 12); //len  = 0x10

	/* mst curld_v */
	set_master_reg(SMMU_MSTR_SMRX_0 + 15 * 0x4, 0, 8, 4); //upwin = 0
	set_master_reg(SMMU_MSTR_SMRX_0 + 15 * 0x4, 0x4, 8, 12); //len  = 0x4

	/* mst_pme_ld */
	set_master_reg(SMMU_MSTR_SMRX_0 + 21 * 0x4, 0x5, 8, 4); //upwin = 0x5
	set_master_reg(SMMU_MSTR_SMRX_0 + 21 * 0x4, 0x1, 8, 12); //len  = 0x1

	/* mst rec_st_luma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 22 * 0x4, 0, 8, 4); //upwin = 0
	set_master_reg(SMMU_MSTR_SMRX_0 + 22 * 0x4, 0x17, 8, 12); //len  = 0x17

	/* mst rec_st_chroma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 23 * 0x4, 0, 8, 4); //upwin = 0
	set_master_reg(SMMU_MSTR_SMRX_0 + 23 * 0x4, 0xD, 8, 12); //len  = 0xD

	/* mst rec_st_luma_h */
	set_master_reg(SMMU_MSTR_SMRX_0 + 35 * 0x4, 0x1, 8, 4); //upwin = 0x1
	set_master_reg(SMMU_MSTR_SMRX_0 + 35 * 0x4, 0x1, 8, 12); //len  = 0x1

	/* mst rec_st_chroma_h */
	set_master_reg(SMMU_MSTR_SMRX_0 + 36 * 0x4, 0x1, 8, 4); //upwin = 0x1
	set_master_reg(SMMU_MSTR_SMRX_0 + 36 * 0x4, 0x1, 8, 12); //len  = 0x1

	/* common scr */
	set_common_reg(SMMU_SCR, 0x0, 1, 0);
	set_common_reg(SMMU_INTCLR_NS, 0x3f, 32, 0);

	/* smmu context config */
	set_common_reg(SMMU_CB_TTBCR, 0x1, 1, 0);

	set_common_reg(SMMU_CB_TTBR0, smmu_page_base_addr & 0xFFFFFFFF, 32, 0);

	set_common_reg(SMMU_CB_TTBR_MSB, ((smmu_page_base_addr >> 32) & 0xFFFF), 16, 0);

	set_common_reg(SMMU_ERR_RDADDR_NS, g_smmu_err_mem.read_addr & 0xFFFFFFFF, 32, 0);
	set_common_reg(SMMU_ERR_WRADDR_NS, g_smmu_err_mem.write_addr & 0xFFFFFFFF, 32, 0);

	set_common_reg(SMMU_ERR_ADDR_MSB_NS, ((g_smmu_err_mem.read_addr >> 32) & 0xFFFF), 16, 0);
	set_common_reg(SMMU_ERR_ADDR_MSB_NS, ((g_smmu_err_mem.write_addr >> 32) & 0xFFFF), 16, 16);

	return SMMU_OK;
}

HI_S32 venc_smmu_cfg(struct encode_info *channelcfg, HI_U32 *reg_base)
{
	memset(&gVencSmmuRegVir, 0, sizeof(SMMU_REG_VIR_S));    /* unsafe_function_ignore: memset */

	gVencSmmuRegVir.pSMMUMasterBaseVirAddr   = (HI_U32 *)((HI_U64)(uintptr_t)reg_base + HIVENC_SMMU_MASTER_OFFSET);
	if (NULL == gVencSmmuRegVir.pSMMUMasterBaseVirAddr) {
		HI_FATAL_VENC("pSMMUMasterBaseVirAddr is NULL \n");
		return SMMU_ERR;
	}

	/* ref ld luma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, START_BIT);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref ld chroma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref ld luma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref ld chroma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* qpg ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + QPG_LD_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + QPG_LD_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* vlc ld ptr wr */
	set_master_reg(SMMU_MSTR_SMRX_1 + VLC_LD_PTR_WR * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + VLC_LD_PTR_WR * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* vlc ld ptr rd */
	set_master_reg(SMMU_MSTR_SMRX_1 + VLC_LD_PTR_RD * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + VLC_LD_PTR_RD * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* nbi ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + NBI_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + NBI_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme info ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + PMEINFO_LD_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PMEINFO_LD_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + PME_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PME_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st luma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st chroma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* qpg st */
	set_master_reg(SMMU_MSTR_SMRX_1 + QPG_ST_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + QPG_ST_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* vlc st ptr wr */
	set_master_reg(SMMU_MSTR_SMRX_1 + VLC_ST_PTR_WR * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + VLC_ST_PTR_WR * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* vlc st ptr rd */
	set_master_reg(SMMU_MSTR_SMRX_1 + VLC_ST_PTR_RD * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + VLC_ST_PTR_RD * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme info st */
	set_master_reg(SMMU_MSTR_SMRX_1 + PMEINFO_ST_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PMEINFO_ST_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* nbi st */
	set_master_reg(SMMU_MSTR_SMRX_1 + NBI_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + NBI_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme st */
	set_master_reg(SMMU_MSTR_SMRX_1 + PME_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PME_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st luma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st chroma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	return SMMU_OK;
}

#ifdef VENC_DEBUG_ENABLE
HI_VOID venc_smmu_debug(HI_U32 *reg_base, HI_BOOL first_cfg_flag)
{
	HI_U32 tmp = 0;

	if (first_cfg_flag)
		return;

	memset(&gVencSmmuRegVir, 0, sizeof(SMMU_REG_VIR_S));    /* unsafe_function_ignore: memset */
	gVencSmmuRegVir.pSMMUMasterBaseVirAddr = (HI_U32 *)((HI_U64)(uintptr_t)reg_base + HIVENC_SMMU_MASTER_OFFSET);
	if (NULL == gVencSmmuRegVir.pSMMUMasterBaseVirAddr) {
		HI_FATAL_VENC("smmu master base addr is NULL\n");
		return;
	}

	RD_SMMU_MASTER_VREG(0x0044, tmp);
	if (tmp & 0x1) {
		HI_ERR_VENC("enter read interrupt");
		RD_SMMU_MASTER_VREG(0x0050, tmp);
		HI_ERR_VENC("read sid 0x%x", tmp);
		RD_SMMU_MASTER_VREG(0x0054, tmp);
		HI_ERR_VENC("read addr 0x%x", tmp);
	}

	if (tmp & 0x4) {
		HI_ERR_VENC("enter write interrupt");
		RD_SMMU_MASTER_VREG(0x0058, tmp);
		HI_ERR_VENC("write sid 0x%x", tmp);
		RD_SMMU_MASTER_VREG(0x005C, tmp);
		HI_ERR_VENC("write addr 0x%x", tmp);
	}

	WR_SMMU_MASTER_VREG(0x004C, 0x1f);
}
#endif
