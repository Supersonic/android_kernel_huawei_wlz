/*
 * power_event.h
 *
 * event for power module
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

#ifndef _POWER_EVENT_H_
#define _POWER_EVENT_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define POWER_EVENT_INVAID      (-1)
#define VBUS_CHECK_WORK_TIME    3000 /* 3s */
#define VBUS_ABSENT_CNTS        2

enum power_event_sysfs_type {
	POWER_EVENT_SYSFS_BEGIN = 0,
	POWER_EVENT_SYSFS_CONNECT_STATE = POWER_EVENT_SYSFS_BEGIN,
	POWER_EVENT_SYSFS_VBUS_STATE,
	POWER_EVENT_SYSFS_END,
};

enum power_event_connect_state {
	POWER_EVENT_DISCONNECT,
	POWER_EVENT_CONNECT,
};

enum power_event_vbus_state {
	POWER_EVENT_PRESENT,
	POWER_EVENT_ABSENT,
};

struct power_event_dev {
	struct device *dev;
	struct notifier_block nb;
	struct kobject *sysfs_ne;
	int connect_state;
	struct delayed_work vbus_check_work;
	int vbus_absent_cnt;
	int vbus_state;
};

#endif /* _POWER_EVENT_H_ */
