/*
 * fpf2283_aux.c
 *
 * fpf2283_aux driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/direct_charger.h>
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless_direct_charger.h>
#endif
#include "../dual_loadswitch.h"
#include "fpf2283.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG fpf2283_aux
HWLOG_REGIST();

static struct fpf2283_device_info *g_fpf2283_dev;

static int fpf2283_write_block(struct fpf2283_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	if (di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	*value = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = num_bytes + 1;

	ret = i2c_transfer(di->client->adapter, msg, 1);

	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		hwlog_err("write_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int fpf2283_read_block(struct fpf2283_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf;
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	if (di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	buf = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = num_bytes;

	ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);

	/* i2c_transfer returns number of messages transferred */
	if (ret != MSG_LEN) {
		hwlog_err("read_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int fpf2283_write_byte(u8 reg, u8 value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return fpf2283_write_block(di, temp_buffer, reg, 1);
}

static int fpf2283_read_byte(u8 reg, u8 *value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	return fpf2283_read_block(di, value, reg, 1);
}

static int fpf2283_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = fpf2283_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return fpf2283_write_byte(reg, val);
}

static void fpf2283_dump_register(void)
{
	int ret;
	u8 i;
	u8 val = 0;

	for (i = 0; i < FPF2283_MAX_REGS; ++i) {
		ret = fpf2283_read_byte(i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");

		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}

static int fpf2283_check_i2c(struct fpf2283_device_info *di)
{
	u8 val = 0;

	return fpf2283_read_block(di, &val, FPF2283_ID_REG, 1);
}

static int fpf2283_reg_init(void)
{
	int ret;
	int value = 0;

	ret = fpf2283_write_byte(FPF2283_ENABLE_REG, FPF2283_ENABLE_INIT);
	ret |= fpf2283_write_byte(FPF2283_INTERRUPT_MASK_REG,
		FPF2283_INTERRUPT_MASK_INIT);

	/* ovp level: offset 0mv, center value 11.5v */
	value |= ((FPF2283_OVP_OFFSET_0MV <<
		FPF2283_OVP_OFFSET_SHIFT) & FPF2283_OVP_OFFSET_MASK);
	value |= ((FPF2283_OVP_CENTER_VALUE_11500MV <<
		FPF2283_OVP_CENTER_VALUE_SHIFT) &
		FPF2283_OVP_CENTER_VALUE_MASK);
	ret |= fpf2283_write_byte(FPF2283_OVP_REG, value);

	if (ret)
		return -1;

	fpf2283_dump_register();

	return 0;
}

static int fpf2283_charge_enable(int enable)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;

	ret = fpf2283_write_mask(FPF2283_OVP_REG,
		FPF2283_OVP_OV_MODE_MASK, FPF2283_OVP_OV_MODE_SHIFT,
		value);
	if (ret)
		return -1;

	ret = fpf2283_read_byte(FPF2283_OVP_REG, &reg);
	if (ret)
		return -1;

	hwlog_info("ovp_reg [%x]=0x%x\n", FPF2283_OVP_REG, reg);
	return 0;
}

static int fpf2283_charge_init(void)
{
	int ret;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->chip_already_init = 1;

	ret = fpf2283_reg_init();
	if (ret)
		return -1;

	di->init_finish_flag = FPF2283_INIT_FINISH;
	return 0;
}

static int fpf2283_charge_exit(void)
{
	int ret;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fpf2283_charge_enable(FPF2283_OVP_OV_MODE_DISABLE);

	di->chip_already_init = 0;
	di->init_finish_flag = FPF2283_NOT_INIT;
	di->int_notify_enable_flag = FPF2283_DISABLE_INT_NOTIFY;

	usleep_range(10000, 11000); /* sleep 10ms */

	return ret;
}

static int fpf2283_discharge(int enable)
{
	return 0;
}

static int fpf2283_config_watchdog_ms(int time)
{
	return 0;
}

static int fpf2283_is_ls_close(void)
{
	u8 reg = 0;
	int ret;

	ret = fpf2283_read_byte(FPF2283_OVP_REG, &reg);
	if (ret)
		return 1;

	if (!(reg & FPF2283_OVP_OV_MODE_MASK))
		return 1;

	return 0;
}

static int fpf2283_ls_status(void)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("ls_status=%d\n", di->chip_already_init);

	if (di->chip_already_init == 1)
		return 0;

	return -1;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static void fpf2283_irq_work(struct work_struct *work)
{
	struct fpf2283_device_info *di = NULL;
	struct nty_data *data = NULL;
	struct atomic_notifier_head *fault_notifier_list = NULL;
	u8 detection_status = 0;
	u8 power_switch_flag = 0;
	u8 interrupt_mask = 0;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct fpf2283_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	data = &(di->nty_data);
	lvc_get_fault_notifier(&fault_notifier_list);

	ret = fpf2283_read_byte(FPF2283_DETECTION_STATUS_REG,
		&detection_status);
	ret |= fpf2283_read_byte(FPF2283_POWER_SWITCH_FLAG_REG,
		&power_switch_flag);
	ret |= fpf2283_read_byte(FPF2283_INTERRUPT_MASK_REG,
		&interrupt_mask);
	if (ret)
		hwlog_err("irq_work read fail\n");

	data->event1 = detection_status;
	data->event2 = power_switch_flag;
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag == FPF2283_ENABLE_INT_NOTIFY) {
		if (power_switch_flag & FPF2283_POWER_SWITCH_FLAG_OV_FLG_MASK) {
			hwlog_info("ovp happened\n");

			atomic_notifier_call_chain(fault_notifier_list,
				DC_FAULT_VBUS_OVP, data);
		} else if (power_switch_flag &
					FPF2283_POWER_SWITCH_FLAG_OC_FLG_MASK) {
			hwlog_info("ocp happened\n");

			atomic_notifier_call_chain(fault_notifier_list,
				DC_FAULT_INPUT_OCP, data);
		} else if (power_switch_flag &
					FPF2283_POWER_SWITCH_FLAG_OT_FLG_MASK) {
			hwlog_info("otp happened\n");

			atomic_notifier_call_chain(fault_notifier_list,
				DC_FAULT_OTP, data);
		}

		fpf2283_dump_register();
	}

	hwlog_info("detection_status_reg [%x]=0x%x\n",
		FPF2283_DETECTION_STATUS_REG, detection_status);
	hwlog_info("power_switch_flag_reg [%x]=0x%x\n",
		FPF2283_POWER_SWITCH_FLAG_REG, power_switch_flag);
	hwlog_info("interrupt_mask_reg [%x]=0x%x\n",
		FPF2283_INTERRUPT_MASK_REG, interrupt_mask);

	/* clear irq */
	enable_irq(di->irq_int);
}

static irqreturn_t fpf2283_interrupt(int irq, void *_di)
{
	struct fpf2283_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (di->init_finish_flag == FPF2283_INIT_FINISH)
		di->int_notify_enable_flag = FPF2283_ENABLE_INT_NOTIFY;

	hwlog_info("fpf2283 int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static struct loadswitch_ops fpf2283_sysinfo_ops = {
	.ls_init = fpf2283_charge_init,
	.ls_exit = fpf2283_charge_exit,
	.ls_enable = fpf2283_charge_enable,
	.ls_discharge = fpf2283_discharge,
	.is_ls_close = fpf2283_is_ls_close,
	.watchdog_config_ms = fpf2283_config_watchdog_ms,
	.ls_status = fpf2283_ls_status,
};

static int fpf2283_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct fpf2283_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (g_fpf2283_dev) {
		hwlog_err("aux driver already matched\n");
		return -ENODEV;
	}

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	hwlog_info("device is %s\n", (char *)of_device_get_match_data(di->dev));

	di->chip_already_init = 1;

	if (fpf2283_check_i2c(di)) {
		hwlog_info("i2c error, ignore this probe\n");
		devm_kfree(&client->dev, di);
		return -ENODEV;
	}
	g_fpf2283_dev = di;

#ifdef POWER_MODULE_DEBUG_FUNCTION
	INIT_WORK(&di->irq_work, fpf2283_irq_work);

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fpf2283_fail_1;
	}

	ret = gpio_request(di->gpio_int, "fpf2283_gpio_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fpf2283_fail_1;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		goto fpf2283_fail_2;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto fpf2283_fail_2;
	}

	ret = request_irq(di->irq_int, fpf2283_interrupt, IRQF_TRIGGER_FALLING,
		"fpf2283_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto fpf2283_fail_2;
	}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

	ret = loadswitch_aux_ops_register(&fpf2283_sysinfo_ops);
	if (ret) {
		hwlog_err("register loadswitch ops failed\n");
		goto fpf2283_fail_3;
	}

	hwlog_info("probe end\n");
	return 0;

fpf2283_fail_3:
#ifdef POWER_MODULE_DEBUG_FUNCTION
	free_irq(di->irq_int, di);
fpf2283_fail_2:
	gpio_free(di->gpio_int);
fpf2283_fail_1:
#endif /* POWER_MODULE_DEBUG_FUNCTION */
	devm_kfree(&client->dev, di);
	g_fpf2283_dev = NULL;

	return ret;
}

static int fpf2283_remove(struct i2c_client *client)
{
	struct fpf2283_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, fpf2283);
static const struct of_device_id fpf2283_of_match[] = {
	{
		.compatible = "huawei,fpf2283_aux",
		.data = "fpf2283",
	},
	{
		.compatible = "huawei,nxp6093a_aux",
		.data = "nxp6093a",
	},
	{},
};

static const struct i2c_device_id fpf2283_i2c_id[] = {
	{ "fpf2283_aux", 0 },
	{ "nxp6093a_aux", 1 },
};

static struct i2c_driver fpf2283_driver = {
	.probe = fpf2283_probe,
	.remove = fpf2283_remove,
	.id_table = fpf2283_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei,fpf2283_aux",
		.of_match_table = of_match_ptr(fpf2283_of_match),
	},
};

static int __init fpf2283_init(void)
{
	return i2c_add_driver(&fpf2283_driver);
}

static void __exit fpf2283_exit(void)
{
	i2c_del_driver(&fpf2283_driver);
}

module_init(fpf2283_init);
module_exit(fpf2283_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fpf2283 aux module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
