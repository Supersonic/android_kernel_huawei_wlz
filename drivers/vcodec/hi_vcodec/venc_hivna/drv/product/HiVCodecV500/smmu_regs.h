/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2010-2019. All rights reserved.
 * Description: smmu_regs description
 */

#ifndef __HIVDEC_SMMU_REGS_H__
#define __HIVDEC_SMMU_REGS_H__

/* SMMU COMMON regs offset */
#define SMMU_SCR                     (0x0000)

#define SMMU_INTMASK_NS              (0x0010)

#define SMMU_INTSTAT_NS              (0x0018)

#define SMMU_INTCLR_NS               (0x001C)

// (0x0020+n*0x4)
#define SMMU_SMRx_NS                 (0x0020)

#define SMMU_RLD_EN0_NS              (0x01F0)

#define SMMU_RLD_EN1_NS              (0x01F4)

#define SMMU_CB_SCTRL                (0x0200)

#define SMMU_CB_TTBR0                (0x0204)

#define SMMU_CB_TTBR1                (0x0208)

#define SMMU_CB_TTBCR                (0x020C)

#define SMMU_CB_TTBR_MSB             (0x0224)

#define SMMU_ERR_ADDR_MSB_NS         (0x0300)

#define SMMU_ERR_RDADDR_NS           (0x0304)

#define SMMU_ERR_WRADDR_NS           (0x0308)

/* SMMU MSTR regs offset */
#define SMMU_MSTR_GLB_BYPASS         (0x0000)

// m*0x4+0x100
#define SMMU_MSTR_SMRX_0             (0x0100)

// m*0x4+0x260
#define SMMU_MSTR_SMRX_1             (0x0260)

// m*0x4+0x3C0
#define SMMU_MSTR_SMRX_2             (0x03C0)
#endif
