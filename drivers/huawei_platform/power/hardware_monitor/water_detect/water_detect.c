/*
 * water_detect.c
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

#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG water_detect
HWLOG_REGIST();

BLOCKING_NOTIFIER_HEAD(g_water_detect_nh);
static struct water_detect_dev *g_water_detect_dev;

static const char * const water_detect_type_table[] = {
	[WD_TYPE_USB_DP_DN] = "usb_dp_dn",
	[WD_TYPE_USB_ID] = "usb_id",
	[WD_TYPE_USB_GPIO] = "usb_gpio",
};

static int water_detect_check_type(int type)
{
	if ((type >= WD_TYPE_BEGIN) && (type < WD_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", type);
	return -1;
}

static int water_detect_get_type(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(water_detect_type_table); i++) {
		if (!strncmp(str, water_detect_type_table[i], strlen(str)))
			return i;
	}

	hwlog_err("invalid type_str=%s\n", str);
	return -1;
}

static struct water_detect_dev *water_detect_get_dev(void)
{
	if (!g_water_detect_dev) {
		hwlog_err("g_water_detect_dev is null\n");
		return NULL;
	}

	return g_water_detect_dev;
}

int water_detect_ops_register(struct water_detect_ops *ops)
{
	int type;

	if (!g_water_detect_dev || !ops || !ops->type_name) {
		hwlog_err("g_water_detect_dev or ops or type_name is null\n");
		return -1;
	}

	type = water_detect_get_type(ops->type_name);
	if (type < 0) {
		hwlog_err("%s ops register fail\n", ops->type_name);
		return -1;
	}

	g_water_detect_dev->ops[type] = ops;
	g_water_detect_dev->total_ops++;

	hwlog_info("total_ops=%d type=%d:%s ops register ok\n",
		g_water_detect_dev->total_ops, type, ops->type_name);
	return 0;
}

static bool water_detect_is_disabled(struct water_detect_dev *l_dev)
{
	if (!l_dev->enabled) {
		hwlog_info("water detect not support\n");
		return true;
	}

	if (power_cmdline_is_powerdown_charging_mode()) {
		hwlog_info("water detect disabled on pd charging mode\n");
		return true;
	}

	if (power_cmdline_is_factory_mode()) {
		hwlog_info("water detect disabled on factory mode\n");
		return true;
	}

	return false;
}

static void water_detect_report_dmd(void)
{
	power_dsm_dmd_report(POWER_DSM_BATTERY,
		ERROR_NO_WATER_CHECK_IN_USB,
		"water check is triggered");
}

static void water_detect_report_uevent(unsigned int flag)
{
	if (flag)
		set_sysfs_water_intruded_flag(1);
	else
		set_sysfs_water_intruded_flag(0);

	charge_send_uevent(VCHRG_STATE_WATER_INTRUSED);
}

static void water_detect_monitor(struct water_detect_dev *l_dev,
	enum water_detect_type detect_type)
{
	struct water_detect_ops *l_ops = NULL;
	enum huawei_usb_charger_type charger_type = charge_get_charger_type();

	if (water_detect_check_type(detect_type))
		return;

	if (water_detect_is_disabled(l_dev))
		return;

	if ((detect_type == WD_TYPE_USB_ID) &&
		(charger_type != CHARGER_TYPE_NON_STANDARD)) {
		hwlog_info("water detect with usbid must be non standard\n");
		return;
	}

	l_ops = l_dev->ops[detect_type];
	if (!l_ops || !l_ops->is_water_intruded) {
		hwlog_err("l_ops or is_water_intruded is null\n");
		return;
	}

	if (l_ops->is_water_intruded()) {
		water_detect_report_uevent(true);
		water_detect_report_dmd();
	}
}

static int water_detect_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct water_detect_dev *l_dev = water_detect_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case WD_NE_REPORT_DMD:
		water_detect_report_dmd();
		break;
	case WD_NE_REPORT_UEVENT:
		water_detect_report_uevent(*((unsigned int *)data));
		break;
	case WD_NE_DETECT_BY_USB_DP_DN:
		water_detect_monitor(l_dev, WD_TYPE_USB_DP_DN);
		break;
	case WD_NE_DETECT_BY_USB_ID:
		water_detect_monitor(l_dev, WD_TYPE_USB_ID);
		break;
	case WD_NE_DETECT_BY_USB_GPIO:
		water_detect_monitor(l_dev, WD_TYPE_USB_GPIO);
		break;
	default:
		hwlog_info("unknown notifier event\n");
		break;
	}

	return NOTIFY_OK;
}

static int water_detect_notifier_chain_register(struct notifier_block *nb)
{
	if (!nb)
		return NOTIFY_OK;

	return blocking_notifier_chain_register(&g_water_detect_nh, nb);
}

static int water_detect_notifier_chain_unregister(struct notifier_block *nb)
{
	if (!nb)
		return NOTIFY_OK;

	return blocking_notifier_chain_unregister(&g_water_detect_nh, nb);
}

void water_detect_event_notify(unsigned long event, void *data)
{
	blocking_notifier_call_chain(&g_water_detect_nh, event, data);
}
EXPORT_SYMBOL(water_detect_event_notify);

static void water_detect_parse_dts(void)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,charger", "water_check_enabled",
		&g_water_detect_dev->enabled, 0);
}

static int __init water_detect_init(void)
{
	int ret;
	struct water_detect_dev *l_dev = NULL;

	hwlog_info("probe begin\n");

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_water_detect_dev = l_dev;
	water_detect_parse_dts();

	l_dev->nb.notifier_call = water_detect_notifier_call;
	ret = water_detect_notifier_chain_register(&l_dev->nb);
	if (ret)
		goto fail_free_mem;

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_water_detect_dev = NULL;

	return ret;
}

static void __exit water_detect_exit(void)
{
	hwlog_info("remove begin\n");

	water_detect_notifier_chain_unregister(&g_water_detect_dev->nb);
	kfree(g_water_detect_dev);
	g_water_detect_dev = NULL;

	hwlog_info("remove end\n");
}

subsys_initcall_sync(water_detect_init);
module_exit(water_detect_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("water intruded detect driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
