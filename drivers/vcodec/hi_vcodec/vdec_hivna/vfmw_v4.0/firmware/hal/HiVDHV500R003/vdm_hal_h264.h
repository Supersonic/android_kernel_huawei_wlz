/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for h264 export header file.
 * Author: gaoyajun
 * Create: 2017-03-16
 */

#ifndef __VDM_HAL_H264_H__
#define __VDM_HAL_H264_H__

#include "basedef.h"
#include "mem_manage.h"
#include "memory.h"
#include "vfmw_intf.h"

#define H264_MSG_SLOT_RATIO        4

#ifdef MSG_POOL_ADDR_CHECK
SINT32 h264hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 h264hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif