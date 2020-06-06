/*
 * Device driver for IP regulator vivobus
 *
 * Copyright (c) 2013 Linaro Ltd.
 * Copyright (c) 2018 Hisilicon.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/file.h>
#include <linux/types.h>
#include <linux/compat.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/pm_wakeup.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/uaccess.h>
#endif
#define REGULATOR_PR_ERR(fmt, args ...)	\
	({				\
		pr_err("[REGU_VIVOBUS]"fmt "\n", ##args); \
	})
#define REGULATOR_PR_INFO(fmt, args ...)	\
	({				\
		pr_info("[REGU_VIVOBUS]"fmt "\n", ##args); \
	})
#define REGULATOR_PR_DEBUG(fmt, args ...)	\
	({				\
		pr_debug("[REGU_VIVOBUS]"fmt "\n", ##args); \
	})

#define REGULATOR_VIVOBUS_ON	        _IOWR('v', 0x1A, unsigned int)
#define REGULATOR_VIVOBUS_OFF	        _IOWR('v', 0x1B, unsigned int)

extern struct list_head *get_regulator_list(void);

enum {
	REGULATOR_OPEN,
	REGULATOR_CLOSE,
	REGULATOR_CMD_MAX,
};

struct regulator_vivobus {
	int		vivobus_major_number;
	int		vivobus_cnt;
	atomic_t		ioctl_excl;
	struct device	*dev;
	struct regulator	*vivobus_ip;
	struct class	*vivobus_class;
	struct wakeup_source	wakelock;
};

struct regulator_vivobus *g_vivobus;

static inline int regulator_vivobus_lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void regulator_vivobus_unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

static long regulator_vivobus_ioctl(struct file *file, u_int cmd, unsigned long result)
{
	int ret = 0;
	int power_result = 0;

	if(result == 0) {
		REGULATOR_PR_ERR("result is NULL.\n");
		return (long)-EINVAL;
	}

	if (regulator_vivobus_lock(&(g_vivobus->ioctl_excl)))
		return -EBUSY;

	switch (cmd) {
	case REGULATOR_VIVOBUS_ON:
		if(g_vivobus->vivobus_cnt == 0) {
			power_result = regulator_enable(g_vivobus->vivobus_ip);
			if(power_result) {
				REGULATOR_PR_ERR("%s regulator enable fail!", __func__);
				break;
			}
			__pm_stay_awake(&g_vivobus->wakelock);
		}
		g_vivobus->vivobus_cnt++;/*lint !e456*/
		REGULATOR_PR_INFO("vivobus vote %d!", g_vivobus->vivobus_cnt);
		break;/*lint !e456*/
	case REGULATOR_VIVOBUS_OFF:
		if(g_vivobus->vivobus_cnt == 0) {
			power_result = -1;
			REGULATOR_PR_ERR("%s regulator vivobus vote fail!", __func__);
			break;/*lint !e456*/
		}
		if(g_vivobus->vivobus_cnt == 1) {
			power_result = regulator_disable(g_vivobus->vivobus_ip);
			if(power_result) {
				REGULATOR_PR_ERR("%s regulator disable fail!", __func__);
				break;/*lint !e456*/
			}
			__pm_relax(&g_vivobus->wakelock);/*lint !e455*/
		}
		g_vivobus->vivobus_cnt--;
		REGULATOR_PR_INFO("vivobus vote %d!", g_vivobus->vivobus_cnt);
		break;/*lint !e456*/
	default:
		REGULATOR_PR_INFO("%s cmd err", __func__);
		power_result = -1;
		break;/*lint !e456*/
	}

	ret = copy_to_user((void __user *)(uintptr_t)result, &power_result, sizeof(int));
	REGULATOR_PR_INFO("%s cmd result %d power: %d", __func__, ret, power_result);
	regulator_vivobus_unlock(&(g_vivobus->ioctl_excl));
	return (long)ret;/*lint !e454*/
}

static int regulator_vivobus_open(struct inode *ip, struct file *fp)
{
	REGULATOR_PR_INFO("%s success", __func__);
	//do nothing
	return 0;
}

static int regulator_vivobus_release(struct inode *ip, struct file *fp)
{
	REGULATOR_PR_INFO("%s", __func__);
	//do nothing
	return 0;
}

static const struct file_operations regulator_vivobus_fops = {
	.unlocked_ioctl    = regulator_vivobus_ioctl,
	.open              = regulator_vivobus_open,
	.release           = regulator_vivobus_release,
};

static int regulator_vivobus_probe(struct platform_device *pdev)
{
	int error = 0;
	struct device *vdevice = NULL;

	g_vivobus = kzalloc(sizeof(*g_vivobus), GFP_KERNEL);
	if (!g_vivobus) {
		REGULATOR_PR_ERR("%s cannot allocate regulator_vivobus device info\n", __func__);
		error = -ENOMEM;
		return error;
	}

	g_vivobus->vivobus_ip = devm_regulator_get(&pdev->dev, "ip-vivobus");
	if(IS_ERR(g_vivobus->vivobus_ip)) {
		REGULATOR_PR_ERR("%s Couldn't get regulator vivobus!", __func__);
		error = -ENODEV;
		goto failed_register_vivobus;
	} else {
		REGULATOR_PR_INFO("%s Get regulator vivobus succuse!", __func__);
	}

	/*semaphore initial*/
	g_vivobus->vivobus_major_number = register_chrdev(0, "regulator-vivobus", &regulator_vivobus_fops);

	if (g_vivobus->vivobus_major_number < 0) {
		REGULATOR_PR_ERR("Failed to allocate major number for vivobus regulator.");
		error = -ENODEV;
		goto failed_register_vivobus;
	}
	atomic_set(&g_vivobus->ioctl_excl, 0);

	g_vivobus->vivobus_class = class_create(THIS_MODULE, "regulator-vivobus");

	if (IS_ERR(g_vivobus->vivobus_class)) {
		REGULATOR_PR_ERR("Error creating regulator_vivobus_class.");
		error = PTR_ERR(g_vivobus->vivobus_class);
		goto failed_chrdev;
	}

	vdevice = device_create(g_vivobus->vivobus_class, NULL, MKDEV((unsigned int)(g_vivobus->vivobus_major_number), 0), NULL, "regulator-vivobus");
	if (IS_ERR(vdevice)) {
		error = -EFAULT;
		REGULATOR_PR_ERR("regulator_vivobus: device_create error.");
		goto failed_class;
	}

	REGULATOR_PR_INFO("regulator_vivobus init ok!");

	wakeup_source_init(&g_vivobus->wakelock, "regulator-vivobus-wakelock");
	g_vivobus->vivobus_cnt = 0;

	platform_set_drvdata(pdev, g_vivobus);

	return 0;

failed_class:
	class_destroy(g_vivobus->vivobus_class);
failed_chrdev:
	unregister_chrdev(g_vivobus->vivobus_major_number, "regulator-vivobus");
failed_register_vivobus:
	kfree(g_vivobus);
	g_vivobus = NULL;
	return error;
}

static int regulator_vivobus_remove(struct platform_device *pdev)
{
	device_destroy(g_vivobus->vivobus_class, MKDEV((unsigned int)(g_vivobus->vivobus_major_number), 0));
	class_destroy(g_vivobus->vivobus_class);
	unregister_chrdev(g_vivobus->vivobus_major_number, "regulator-vivobus");
	kfree(g_vivobus);
	g_vivobus = NULL;
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct of_device_id of_regulator_vivobus_match_tbl[] = {
	{
		.compatible = "hisilicon,regulator-vivobus",
	},
	{ /* end */ }
};

static struct platform_driver regulator_vivobus_driver = {
	.driver = {
		.name	= "regulator_vivobus",
		.owner  = THIS_MODULE,
		.of_match_table = of_regulator_vivobus_match_tbl,
	},
	.probe	= regulator_vivobus_probe,
	.remove	= regulator_vivobus_remove,
};

static int __init regulator_vivobus_init(void)
{
	return platform_driver_register(&regulator_vivobus_driver);
}
static void __exit regulator_vivobus_exit(void)
{
	platform_driver_unregister(&regulator_vivobus_driver);
}

device_initcall(regulator_vivobus_init);
module_exit(regulator_vivobus_exit);
MODULE_DESCRIPTION("Hisi ip regulator vivobus driver");
MODULE_LICENSE("GPL v2");
