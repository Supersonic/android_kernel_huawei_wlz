/*
 * water_detect.h
 *
 * water intruded detect driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _WATER_DETECT_H_
#define _WATER_DETECT_H_

#include <linux/notifier.h>

enum water_detect_type {
	WD_TYPE_BEGIN = 0,
	WD_TYPE_USB_DP_DN = WD_TYPE_BEGIN, /* usb d+/d-pin */
	WD_TYPE_USB_ID, /* usb id pin */
	WD_TYPE_USB_GPIO, /* usb gpio */
	WD_TYPE_END,
};

enum water_detect_ne_list {
	WD_NE_REPORT_DMD,
	WD_NE_REPORT_UEVENT,
	WD_NE_DETECT_BY_USB_DP_DN,
	WD_NE_DETECT_BY_USB_ID,
	WD_NE_DETECT_BY_USB_GPIO,
};

struct water_detect_ops {
	const char *type_name;
	int (*is_water_intruded)(void);
};

struct water_detect_dev {
	struct notifier_block nb;
	unsigned int enabled;
	struct water_detect_ops *ops[WD_TYPE_END];
	unsigned int total_ops;
};

#ifdef CONFIG_HUAWEI_WATER_DETECT
int water_detect_ops_register(struct water_detect_ops *ops);
void water_detect_event_notify(unsigned long event, void *data);
#else
static inline int water_detect_ops_register(struct water_detect_ops *ops)
{
	return -1;
}

static inline void water_detect_event_notify(unsigned long event, void *data)
{
}
#endif /* CONFIG_HUAWEI_WATER_DETECT */

#endif /* _WATER_DETECT_H_ */
