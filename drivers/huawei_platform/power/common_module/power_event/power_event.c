/*
 * power_event.c
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

#include "power_event.h"
#include <huawei_platform/power/power_sysfs.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_event_ne.h>
#include <huawei_platform/power/huawei_charger_uevent.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/power/hisi/hisi_bci_battery.h>

#define HWLOG_TAG power_event
HWLOG_REGIST();

struct power_event_dev *g_power_event_dev;
static BLOCKING_NOTIFIER_HEAD(g_power_event_nh);

static struct power_event_dev *power_event_get_dev(void)
{
	if (!g_power_event_dev) {
		hwlog_err("g_power_event_dev is null\n");
		return NULL;
	}

	return g_power_event_dev;
}

static void power_event_vbus_check(struct power_event_dev *l_dev)
{
#ifdef CONFIG_DIRECT_CHARGER
	if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE)
		return;
#endif /* CONFIG_DIRECT_CHARGER */

	hwlog_info("vbus_state=%d, cnts=%d\n",
		l_dev->vbus_state, l_dev->vbus_absent_cnt);

	if (hisi_pmic_get_vbus_status() == 0) {
		if (l_dev->vbus_absent_cnt++ < VBUS_ABSENT_CNTS)
			return;

		l_dev->vbus_state = POWER_EVENT_ABSENT;
		charge_send_uevent(VCHRG_STOP_CHARGING_EVENT);
		sysfs_notify(l_dev->sysfs_ne, NULL, "vbus_state");
	} else {
		l_dev->vbus_state = POWER_EVENT_PRESENT;
		l_dev->vbus_absent_cnt = 0;
	}
}

static void power_event_vbus_check_work(struct work_struct *work)
{
	struct power_event_dev *l_dev = power_event_get_dev();

	if (!l_dev)
		return;

	if (!power_cmdline_is_powerdown_charging_mode())
		return;

	power_event_vbus_check(l_dev);
	schedule_delayed_work(&l_dev->vbus_check_work,
		msecs_to_jiffies(VBUS_CHECK_WORK_TIME));
}

static int power_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct power_event_dev *l_dev = power_event_get_dev();

	if (!l_dev || !l_dev->sysfs_ne)
		return NOTIFY_OK;

	hwlog_info("receive event %d\n", event);

	switch (event) {
	case POWER_EVENT_NE_USB_DISCONNECT:
	case POWER_EVENT_NE_WIRELESS_DISCONNECT:
		/* ignore repeat event */
		if (l_dev->connect_state == POWER_EVENT_DISCONNECT)
			break;
		l_dev->connect_state = POWER_EVENT_DISCONNECT;
		sysfs_notify(l_dev->sysfs_ne, NULL, "connect_state");
		break;
	case POWER_EVENT_NE_USB_CONNECT:
	case POWER_EVENT_NE_WIRELESS_CONNECT:
		/* ignore repeat event */
		if (l_dev->connect_state == POWER_EVENT_CONNECT)
			break;
		l_dev->connect_state = POWER_EVENT_CONNECT;
		sysfs_notify(l_dev->sysfs_ne, NULL, "connect_state");
		break;
	case POWER_EVENT_NE_VBUS_CHECK:
		schedule_delayed_work(&l_dev->vbus_check_work,
			msecs_to_jiffies(VBUS_CHECK_WORK_TIME));
		break;
	default:
		hwlog_info("unknown notifier event\n");
		break;
	}

	return NOTIFY_OK;
}

static int power_event_notifier_chain_register(struct notifier_block *nb)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_register(&g_power_event_nh, nb);
}

static int power_event_notifier_chain_unregister(struct notifier_block *nb)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_unregister(&g_power_event_nh, nb);
}

void power_event_notify(enum power_event_ne_list event)
{
	blocking_notifier_call_chain(&g_power_event_nh,
		(unsigned long)event, NULL);
}
EXPORT_SYMBOL_GPL(power_event_notify);

#ifdef CONFIG_SYSFS
#define POWER_EVENT_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, power_event_sysfs_show, store), \
	.name = POWER_EVENT_SYSFS_##n, \
}

#define POWER_EVENT_SYSFS_FIELD_RO(_name, n) \
	POWER_EVENT_SYSFS_FIELD(_name, n, 0440, NULL)

struct power_event_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t power_event_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_event_sysfs_field_info power_event_sysfs_field_tbl[] = {
	POWER_EVENT_SYSFS_FIELD_RO(connect_state, CONNECT_STATE),
	POWER_EVENT_SYSFS_FIELD_RO(vbus_state, VBUS_STATE),
};

#define POWER_EVENT_SYSFS_ATTRS_SIZE (ARRAY_SIZE(power_event_sysfs_field_tbl) + 1)

static struct attribute *power_event_sysfs_attrs[POWER_EVENT_SYSFS_ATTRS_SIZE];

static const struct attribute_group power_event_sysfs_attr_group = {
	.attrs = power_event_sysfs_attrs,
};

static void power_event_sysfs_init_attrs(void)
{
	int s;
	int e = ARRAY_SIZE(power_event_sysfs_field_tbl);

	for (s = 0; s < e; s++)
		power_event_sysfs_attrs[s] = &power_event_sysfs_field_tbl[s].attr.attr;

	power_event_sysfs_attrs[e] = NULL;
}

static struct power_event_sysfs_field_info *power_event_sysfs_field_lookup(
	const char *name)
{
	int s;
	int e = ARRAY_SIZE(power_event_sysfs_field_tbl);

	for (s = 0; s < e; s++) {
		if (!strncmp(name,
			power_event_sysfs_field_tbl[s].attr.attr.name,
			strlen(name)))
			break;
	}

	if (s >= e)
		return NULL;

	return &power_event_sysfs_field_tbl[s];
}

static ssize_t power_event_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_event_sysfs_field_info *info = NULL;
	struct power_event_dev *l_dev = power_event_get_dev();
	int len;

	if (!l_dev)
		return -EINVAL;

	info = power_event_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case POWER_EVENT_SYSFS_CONNECT_STATE:
		len = scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->connect_state);
		break;
	case POWER_EVENT_SYSFS_VBUS_STATE:
		len = scnprintf(buf, PAGE_SIZE, "%d\n", l_dev->vbus_state);
		break;
	default:
		hwlog_err("invalid sysfs_name\n");
		len = 0;
		break;
	}

	return len;
}

static struct device *power_event_sysfs_create_group(void)
{
	power_event_sysfs_init_attrs();
	return power_sysfs_create_group("hw_power", "power_event",
		&power_event_sysfs_attr_group);
}

static void power_event_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_event_sysfs_attr_group);
}
#else
static inline struct device *power_event_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_event_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_event_init(void)
{
	int ret;
	struct power_event_dev *l_dev = NULL;

	hwlog_info("probe begin\n");

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_event_dev = l_dev;
	l_dev->nb.notifier_call = power_event_notifier_call;
	ret = power_event_notifier_chain_register(&l_dev->nb);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->vbus_check_work,
		power_event_vbus_check_work);
	l_dev->dev = power_event_sysfs_create_group();
	if (l_dev->dev)
		l_dev->sysfs_ne = &l_dev->dev->kobj;
	l_dev->connect_state = POWER_EVENT_INVAID;
	l_dev->vbus_state = POWER_EVENT_INVAID;

	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_power_event_dev = NULL;

	return ret;
}

static void __exit power_event_exit(void)
{
	struct power_event_dev *l_dev = g_power_event_dev;

	hwlog_info("remove begin\n");

	if (!l_dev)
		return;

	cancel_delayed_work(&l_dev->vbus_check_work);
	power_event_notifier_chain_unregister(&l_dev->nb);
	power_event_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_power_event_dev = NULL;

	hwlog_info("remove ok\n");
}

fs_initcall_sync(power_event_init);
module_exit(power_event_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power event module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
