/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec tvp adapter
 * Author: zhangjianshun
 * Create: 2017-04-15
 */

#ifndef __TVP_ADAPTER_H__
#define __TVP_ADAPTER_H__

#include "public.h"

#ifdef ENV_ARMLINUX_KERNEL
hi_s32  tvp_vdec_secure_init(void);
hi_s32  tvp_vdec_secure_exit(void);
hi_s32  tvp_vdec_suspend(void);
hi_s32  tvp_vdec_resume(void);
#endif

#define TEEC_OPERATION_PARA_INDEX_FIRST  1
#define TEEC_OPERATION_PARA_INDEX_SECOND 2
#define TEEC_OPERATION_PARA_INDEX_THIRD  3

#define TVP_PACKAGE_NAME_MAX_LENGTH      70

#endif //end of __TVP_ADAPTER_H__

