/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2001-2019. All rights reserved.
 * Description: VDMV300 hardware abstraction export header file.
 * Author: zhuyonghao
 * Create: 2001-10-5
 */

#ifndef __VDM_HAL_VP9_H__
#define __VDM_HAL_VP9_H__
#include "memory.h"
#include "vfmw_intf.h"
#include "public.h"

#define VP9_MSG_SLOT_RATIO                           4

#ifdef MSG_POOL_ADDR_CHECK
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif // __VDM_HAL_AVS_H__
