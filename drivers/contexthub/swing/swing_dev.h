/*
 * Copyright (C) Huawei Tech. Co. Ltd. 2017-2019. All rights reserved.
 * Description: dev drvier to communicate with sensorhub swing app
 * Author: Huawei
 * Create: 2017.12.05
 */

#ifndef __LINUX_CONTEXTHUB_SWING_H__
#define __LINUX_CONTEXTHUB_SWING_H__
#include <linux/types.h>

/* ioctl cmd define */
#define SWING_IO                         0xB1

#define SWING_IOCTL_SWING_OPEN         _IOW(SWING_IO, 0xD1, short)
#define SWING_IOCTL_SWING_CLOSE        _IOW(SWING_IO, 0xD2, short)
#define SWING_IOCTL_FUSION_EN          _IOW(SWING_IO, 0xD3, short)
#define SWING_IOCTL_ATOM_EN            _IOW(SWING_IO, 0xD4, short)
#define SWING_IOCTL_FUSION_SET         _IOW(SWING_IO, 0xD5, short)
#define SWING_IOCTL_ATOM_SET           _IOW(SWING_IO, 0xD6, short)

#define IOMCU_SYSTEMCACHE_HINT         8

typedef struct {
	u32 fusion_id;
	u32 en;
} swing_fusion_en_t;

typedef struct {
	u32 fusion_id;
	u32 ret_code;
} swing_fusion_en_resp_t;

typedef struct {
	swing_fusion_en_t en;
	swing_fusion_en_resp_t en_resp;
} swing_fusion_en_param_t;

typedef struct {
	u32 fusion_id;
	u32 data_type;
	u32 data_addr;
	u32 data_len;
} swing_fusion_set_t;

typedef struct {
	u32 fusion_id;
	u32 ret_code;
} swing_fusion_set_resp_t;

typedef struct {
	swing_fusion_set_t set;
	swing_fusion_set_resp_t set_resp;
} swing_fusion_set_param_t;

typedef struct {
	u32 fusion_id;
	u32 notify_type;
	u32 notify_len;
	u32 reserved;
} swing_upload_t;

#endif

