/*
 * bat_heating.h
 *
 * battery heating of low temperature control driver
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

#ifndef _BAT_HEATING_H_
#define _BAT_HEATING_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/types.h>

#define DELAY_TIME_FOR_CHECK_WORK            5000 /* 5s */
#define DELAY_TIME_FOR_SLOW_MONITOR_WORK     20000 /* 20s */
#define DELAY_TIME_FOR_FAST_MONITOR_WORK     10000 /* 10s */
#define DEFAULT_LOW_TEMP_MIN_THLD            (-10)
#define DEFAULT_LOW_TEMP_MAX_THLD            9
#define DEFAULT_LOW_TEMP_HYSTERESIS          15
#define DEFAULT_LOW_TEMP_MIN_IBAT            (-150)

enum bat_heating_sysfs_type {
	BAT_HEATING_SYSFS_TYPE_BEGIN = 0,
	BAT_HEATING_SYSFS_MINTHLD,
	BAT_HEATING_SYSFS_MAXTHLD,
	BAT_HEATING_SYSFS_STATUS,
	BAT_HEATING_SYSFS_LOAD,
	BAT_HEATING_SYSFS_ICHG,
	BAT_HEATING_SYSFS_TYPE_END,
};

enum bat_heating_status_type {
	BAT_HEATING_STATUS_STOP,
	BAT_HEATING_STATUS_START,
};

enum bat_heating_load_type {
	BAT_HEATING_LOAD_NORMAL,
	BAT_HEATING_LOAD_OVER,
};

#define BAT_HEATING_TEMP_LEVEL    8

enum bat_heating_temp_info {
	BAT_HEATING_TEMP_MIN = 0,
	BAT_HEATING_TEMP_MAX,
	BAT_HEATING_TEMP_ICHG,
	BAT_HEATING_TEMP_TOTAL,
};

struct bat_heating_temp_para {
	int temp_min;
	int temp_max;
	int temp_ichg;
};

struct bat_heating_dev {
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work check_work;
	struct delayed_work monitor_work;
	int status;
	int load;
	int temp;
	int ibat;
	int ichg;
	int hysteresis;
	int low_temp_min_thld;
	int low_temp_max_thld;
	int low_temp_hysteresis;
	int low_temp_min_ibat;
	struct bat_heating_temp_para temp_para[BAT_HEATING_TEMP_LEVEL];
};

#endif /* _BAT_HEATING_H_ */
