/*
 * dual_sc.c
 *
 * dual sc driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/power_devices_info.h>

#include "../stwlc68/stwlc68.h"
#include "dual_sc.h"

#define HWLOG_TAG wireless_dual_sc
HWLOG_REGIST();

static struct dual_sc_info *g_dual_sc_di;
static struct wireless_cp_ops *g_main_ops;
static struct wireless_cp_ops *g_aux_ops;

int dual_sc_main_wlchip_ops_register(struct wireless_cp_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_main_ops = ops;
		hwlog_info("dual sc main ops register ok\n");
	} else {
		hwlog_info("dual sc main ops has registered\n");
		ret = -1;
	}

	return ret;
}

int dual_sc_aux_wlchip_ops_register(struct wireless_cp_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_aux_ops = ops;
		hwlog_info("dual sc aux ops register ok\n");
	} else {
		hwlog_info("dual sc aux ops has registered\n");
		ret = -1;
	}

	return ret;
}

static int dual_sc_ops_register(struct wireless_cp_ops *ops)
{
	return stwlc68_cp_ops_register(ops);
}

static int dual_sc_chip_init(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->chip_init)
		ret |= g_main_ops->chip_init();

	if (g_aux_ops && g_aux_ops->chip_init)
		ret |= g_aux_ops->chip_init();

	return ret;
}

static int dual_sc_set_bp_mode(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_bp_mode)
		ret |= g_main_ops->set_bp_mode();

	if (g_aux_ops && g_aux_ops->set_bp_mode)
		ret |= g_aux_ops->set_bp_mode();

	return ret;
}

static int dual_sc_set_cp_mode(void)
{
	int ret = 0;

	if (g_main_ops && g_main_ops->set_cp_mode)
		ret |= g_main_ops->set_cp_mode();

	if (g_aux_ops && g_aux_ops->set_cp_mode)
		ret |= g_aux_ops->set_cp_mode();

	return ret;
}

static bool dual_sc_is_cp_open(void)
{
	bool ret1 = false;
	bool ret2 = false;

	if (g_main_ops && g_main_ops->is_cp_open)
		ret1 = g_main_ops->is_cp_open();

	if (g_aux_ops && g_aux_ops->is_cp_open)
		ret2 = g_aux_ops->is_cp_open();

	if (ret1 && ret2) {
		hwlog_info("[%s] sucess\n", __func__);
		return true;
	}
	hwlog_err("%s: fail\n", __func__);

	return false;
}

static bool dual_sc_is_bp_open(void)
{
	bool ret1 = false;
	bool ret2 = false;

	if (g_main_ops && g_main_ops->is_bp_open)
		ret1 = g_main_ops->is_bp_open();

	if (g_aux_ops && g_aux_ops->is_bp_open)
		ret2 = g_aux_ops->is_bp_open();

	if (ret1 && ret2) {
		hwlog_info("[%s] sucess\n", __func__);
		return true;
	}
	hwlog_err("%s: fail\n", __func__);

	return false;
}

struct wireless_cp_ops dual_sc_ops = {
	.chip_init = dual_sc_chip_init,
	.set_bp_mode = dual_sc_set_bp_mode,
	.set_cp_mode = dual_sc_set_cp_mode,
	.is_cp_open = dual_sc_is_cp_open,
	.is_bp_open = dual_sc_is_bp_open,
};

static int dual_sc_probe(struct platform_device *pdev)
{
	int ret;
	struct dual_sc_info *info = NULL;

	hwlog_info("[%s] begin\n", __func__);
	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	g_dual_sc_di = info;
	info->pdev = pdev;
	info->dev = &pdev->dev;

	ret = dual_sc_ops_register(&dual_sc_ops);
	if (ret) {
		hwlog_err("register dual_sc ops failed\n");
		goto dual_sc_fail_0;
	}
	platform_set_drvdata(pdev, info);
	hwlog_info("[%s] end\n", __func__);
	return 0;

dual_sc_fail_0:
	devm_kfree(&pdev->dev, info);
	g_dual_sc_di = NULL;
	return ret;
}

static int dual_sc_remove(struct platform_device *pdev)
{
	struct dual_sc_info *info = platform_get_drvdata(pdev);

	hwlog_info("[%s] begin\n", __func__);
	if (!info)
		return -ENODEV;

	g_aux_ops = NULL;
	g_main_ops = NULL;
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_dual_sc_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id dual_sc_match_table[] = {
	{
		.compatible = "wireless,dual_sc",
		.data = NULL,
	},
	{},
};

static struct platform_driver dual_sc_driver = {
	.probe = dual_sc_probe,
	.remove = dual_sc_remove,
	.driver = {
		.name = "dual_sc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dual_sc_match_table),
	},
};

static int __init dual_sc_init(void)
{
	return platform_driver_register(&dual_sc_driver);
}

static void __exit dual_sc_exit(void)
{
	platform_driver_unregister(&dual_sc_driver);
}

rootfs_initcall(dual_sc_init);
module_exit(dual_sc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dual sc module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
