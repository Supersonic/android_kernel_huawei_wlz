/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: system config
 * Author: zhangjianshun
 * Create: 2017-03-27
 */

/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWinjiDDUykL9e8pckESWBbMVmSWkBuyJO01cTiy3TdzKxGk0oBQa
mSMf7J4FkTpfvzHyMxSEsfcbL/G0fFswaAZ8tsS4we+PBWC6a/UNlzCWIaw+Ujkv9NAY+as0
fg7WZIRvw27AjvRqJbkRJvqFUORSa6KPQaSBMxCxJTGTTf//sQbjPOyYldN0OVR9ut4HFO4U
ZguGQVqcOAJQbE96v6175DqhuprKgQB8R+2fu7VD3qtX+ZJh/t0512oqv+e8YA==*/

#ifndef __VFMW_SYSCONFIG_HEADER__
#define __VFMW_SYSCONFIG_HEADER__

#include "vfmw.h"

/* valid vdh num */
#define MAX_VDH_NUM               2

#ifdef ENV_SOS_KERNEL
#define HEAP_SEC_DRM_BASE  0x60000000U
#define HEAP_SEC_DRM_SIZE  0x20000000U // (512*1024*1024)
#endif

/* register offset */
#define SCD_REG_OFFSET            0xc000
#define BPD_REG_OFFSET            0xd000

#define SOFTRST_REQ_OFFSET        0xcc0c // (0xf80c)
#define SOFTRST_OK_OFFSET         0xcc10 // (0xf810)

#define ALL_RESET_CTRL_BIT        0
#define MFDE_RESET_CTRL_BIT       1
#define SCD_RESET_CTRL_BIT        2
#define BPD_RESET_CTRL_BIT        3

#define ALL_RESET_OK_BIT          0
#define MFDE_RESET_OK_BIT         1
#define SCD_RESET_OK_BIT          2
#define BPD_RESET_OK_BIT          3

#define ALL_RESET_CTRL_MASK       (1 << ALL_RESET_CTRL_BIT)
#define MFDE_RESET_CTRL_MASK      (1 << MFDE_RESET_CTRL_BIT)
#define SCD_RESET_CTRL_MASK       (1 << SCD_RESET_CTRL_BIT)
#define BPD_RESET_CTRL_MASK       (1 << BPD_RESET_CTRL_BIT)

#define ALL_RESET_OK_MASK         (1 << ALL_RESET_OK_BIT)
#define MFDE_RESET_OK_MASK        (1 << MFDE_RESET_OK_BIT)
#define SCD_RESET_OK_MASK         (1 << SCD_RESET_OK_BIT)
#define BPD_RESET_OK_MASK         (1 << BPD_RESET_OK_BIT)

/* FPGA flag */
extern UINT32  g_is_fpga;

/* register base addr & range */
extern UINT32  g_vdh_reg_base_addr;
extern UINT32  g_scd_reg_base_addr;
extern UINT32  g_bpd_reg_base_addr;
extern UINT32  g_vdh_reg_range;
extern UINT32  g_soft_rst_req_addr;
extern UINT32  g_soft_rst_ok_addr;

/* smmu page table base addr */
extern UINT64  g_smmu_page_base;

/* peri crg base addr */
extern UINT32  g_pericrg_reg_base_addr;

/* irq num */
extern UINT32  g_vdec_irq_num_norm;
extern UINT32  g_vdec_irq_num_prot;
extern UINT32  g_vdec_irq_num_safe;

#endif
