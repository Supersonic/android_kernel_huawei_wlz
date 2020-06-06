/*
 * bq2560x_charger.c
 *
 * bq2560x driver
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
#include <linux/pm_wakeup.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>

#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/usb_short_circuit_protect.h>
#endif
#include "bq2560x_charger.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq2560x_charger
HWLOG_REGIST();

struct bq2560x_device_info *g_bq2560x_dev;

/* configured in dts based on the real value of the iin limit resistance */
/* static unsigned int rilim = BQ2560X_RILIM_220_OHM; */
/* configured in dts based on the real adc channel number */
/* static unsigned int adc_channel_iin = BQ2560X_ADC_CHANNEL_IIN_10; */

static bool g_hiz_mode = FALSE;
static int hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;
static int g_cv_flag;
static int g_cv_policy;
static int g_bq2560x_cv;
static bool g_shutdown_flag;
static int g_safe_time_cnt;
static time64_t g_safe_pre_time = SAFE_TIME_RESET;
static time64_t g_boot_time;
static int g_sgm41511_exist;

#define TERM_CURR                    840
#define LIMIT_CURR                   840

#define MSG_LEN                      2
#define BUF_LEN                      26

#define GPIO_LOW                     0
#define GPIO_HIGH                    1
#define DELAY_10MS                   10
#define DELAY_50MS                   50
#define DELAY_100MS                  100
#define DELAY_1500MS                 1500
#define RETRY_MAX3                   3
#define MIN_CV                       4350
#define RETRY_TIMES                  3

static int bq2560x_fcp_reset(void);

static int bq2560x_write_block(struct bq2560x_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
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
		return -EIO;
	}

	return 0;
}

static int bq2560x_read_block(struct bq2560x_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf;
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
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
		return -EIO;
	}

	return 0;
}

static int bq2560x_write_byte(u8 reg, u8 value)
{
	struct bq2560x_device_info *di = g_bq2560x_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return bq2560x_write_block(di, temp_buffer, reg, 1);
}

static int bq2560x_read_byte(u8 reg, u8 *value)
{
	struct bq2560x_device_info *di = g_bq2560x_dev;

	return bq2560x_read_block(di, value, reg, 1);
}

static int bq2560x_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = bq2560x_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return bq2560x_write_byte(reg, val);
}

static int bq2560x_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = bq2560x_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

#ifdef CONFIG_SYSFS
/*
 * There are a numerous options that are configurable on the bq2560x
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#define BQ2560x_SYSFS_FIELD(_name, r, f, m, store) \
{ \
	.attr = __ATTR(_name, m, bq2560x_sysfs_show, store), \
	.reg = BQ2560X_REG_##r, \
	.mask = BQ2560X_REG_##f##_MASK, \
	.shift = BQ2560X_REG_##f##_SHIFT, \
}

#define BQ2560x_SYSFS_FIELD_RW(_name, r, f) \
	BQ2560x_SYSFS_FIELD(_name, r, f, 0640, bq2560x_sysfs_store)

#define BQ2560x_SYSFS_FIELD_RO(_name, r, f) \
	BQ2560x_SYSFS_FIELD(_name, r, f, 0440, NULL)

static ssize_t bq2560x_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bq2560x_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct bq2560x_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
	u8 mask;
	u8 shift;
};

/* On i386 ptrace-abi.h defines SS that breaks the macro calls below. */
//#undef SS

static struct bq2560x_sysfs_field_info bq2560x_sysfs_field_tbl[] = {
	/* sysfs name reg field in reg */
	BQ2560x_SYSFS_FIELD_RW(reg_addr, NONE, NONE),
	BQ2560x_SYSFS_FIELD_RW(reg_value, NONE, NONE),
};

#define BQ2560X_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(bq2560x_sysfs_field_tbl) + 1)

static struct attribute *bq2560x_sysfs_attrs[BQ2560X_SYSFS_ATTRS_SIZE];

static const struct attribute_group bq2560x_sysfs_attr_group = {
	.attrs = bq2560x_sysfs_attrs,
};

static void bq2560x_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(bq2560x_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		bq2560x_sysfs_attrs[i] = &bq2560x_sysfs_field_tbl[i].attr.attr;

	bq2560x_sysfs_attrs[limit] = NULL;
}

static struct bq2560x_sysfs_field_info *bq2560x_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit = ARRAY_SIZE(bq2560x_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strcmp(name, bq2560x_sysfs_field_tbl[i].attr.attr.name))
			break;
	}

	if (i >= limit)
		return NULL;

	return &bq2560x_sysfs_field_tbl[i];
}

static ssize_t bq2560x_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	struct bq2560x_sysfs_field_info *info = NULL;
	struct bq2560x_sysfs_field_info *info2 = NULL;
	int ret;
	u8 v;

	info = bq2560x_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr")))
		return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", info->reg);

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = bq2560x_sysfs_field_lookup("reg_addr");
		if (!info2) {
			hwlog_err("get sysfs entries failed\n");
			return -EINVAL;
		}

		info->reg = info2->reg;
	}

	ret = bq2560x_read_mask(info->reg, info->mask, info->shift, &v);
	if (ret)
		return ret;

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", v);
}

static ssize_t bq2560x_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bq2560x_sysfs_field_info *info = NULL;
	struct bq2560x_sysfs_field_info *info2 = NULL;
	int ret;
	u8 v;

	info = bq2560x_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	ret = kstrtou8(buf, 0, &v);
	if (ret < 0) {
		hwlog_err("get kstrtou8 failed\n");
		return ret;
	}

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = bq2560x_sysfs_field_lookup("reg_addr");
		if (!info2) {
			hwlog_err("get sysfs entries failed\n");
			return -EINVAL;
		}

		info->reg = info2->reg;
	}

	if (!strncmp(("reg_addr"), attr->attr.name, strlen("reg_addr"))) {
		if (v < (u8) BQ2560X_REG_NUM) {
			info->reg = v;
			return count;
		} else {
			return -EINVAL;
		}
	}

	ret = bq2560x_write_mask(info->reg, info->mask, info->shift, v);
	if (ret)
		return ret;

	return count;
}

static void bq2560x_sysfs_create_group(struct device *dev)
{
	bq2560x_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "bq2560x",
		dev, &bq2560x_sysfs_attr_group);
}

static void bq2560x_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "bq2560x",
		dev, &bq2560x_sysfs_attr_group);
}
#else
static inline void bq2560x_sysfs_create_group(struct device *dev)
{
}

static inline void bq2560x_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int bq2560x_device_check(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_byte(BQ2560X_REG_VPRS, &reg);
	if (ret)
		return CHARGE_IC_BAD;

	hwlog_info("device_check [%x]=0x%x\n", BQ2560X_REG_VPRS, reg);

	if (((reg & BQ2560X_REG_VPRS_PN_MASK) >>
		BQ2560X_REG_VPRS_PN_SHIFT) == VENDOR_ID) {
		hwlog_info("bq2560x is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("bq2560x is bad\n");
	return CHARGE_IC_BAD;
}

static int sgm41511_device_check(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_byte(BQ2560X_REG_VPRS, &reg);
	if (ret)
		return 0;

	hwlog_info("device_check %x = 0x%x\n", BQ2560X_REG_VPRS, reg);

	if (((reg & BQ2560X_REG_VPRS_PART_MASK) >>
		BQ2560X_REG_VPRS_PART_SHIFT) == SGM41511H_VENDOR_ID) {
		hwlog_info("device exists\n");
		return 1;
	}

	hwlog_err("device not exists\n");
	return 0;
}

static int bq2560x_5v_chip_init(struct bq2560x_device_info *di)
{
	int ret;

	g_hiz_mode = FALSE;

	/* boost mode current limit = 1000mA */
	ret = bq2560x_write_byte(BQ2560X_REG_CCC, 0xa1);
	if (ret < 0)
		hwlog_err("init otg cur fail\n");

	/* I2C watchdog timer setting = 80s */
	/* fast charge timer setting = 12h */
	ret = bq2560x_write_byte(BQ2560X_REG_CTTC, 0xAF);
	if (ret < 0)
		hwlog_err("init wdt time fail\n");

	/* iprechg = 256ma,iterm current = 128ma */
	ret = bq2560x_write_byte(BQ2560X_REG_PCTCC, 0x42);
	if (ret < 0)
		hwlog_err("init pre_chg_cur fail\n");

	ret = bq2560x_write_mask(BQ2560X_REG_MOC,
		BQ2560X_REG_MOC_VDPM_BAT_TRACK_MASK,
		BQ2560X_REG_MOC_VDPM_BAT_TRACK_SHIFT,
		REG07_VDPM_BAT_TRACK_DISABLE);
	if (ret < 0)
		hwlog_err("disable auto vdpm fail\n");

	ret = bq2560x_write_mask(BQ2560X_REG_REG_BVTRC,
		BQ2560X_REG_REG_BVTRC_OVP_MASK,
		BQ2560X_REG_REG_BVTRC_OVP_SHIFT,
		REG06_OVP_10P5V);
	if (ret < 0)
		hwlog_err("init acov threshold fail\n");

	hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}
static int bq2560x_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bq2560x_device_info *di = g_bq2560x_dev;
	struct timespec64 ts = { 0 };

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	if (g_sgm41511_exist) {
		g_safe_time_cnt = 0;
		g_safe_pre_time = SAFE_TIME_RESET;
		get_monotonic_boottime64(&ts);
		g_boot_time = ts.tv_sec;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		/* reset fcp before chip init with vbus 5v */
		if (di->fcp_support)
			(void)bq2560x_fcp_reset();

		ret = bq2560x_5v_chip_init(di);
		break;
	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bq2560x_set_input_current(int value)
{
	int val;

	/*
	 * IC precision deviation by hw: +100mA(current >= 1700)
	 * max value protect: 3200mA
	 */
	if (value >= AC_IIN_CURRENT_THRESHOLD) {
		value += AC_IIN_CURRENT_OFFSET;
		if (value > AC_IIN_MAX_CURRENT)
			value = AC_IIN_MAX_CURRENT;
	}

	val = (value - REG00_IINLIM_BASE) / REG00_IINLIM_LSB;

	hwlog_info("set_input_current [%x]=0x%x\n", BQ2560X_REG_ISC, val);

	return bq2560x_write_mask(BQ2560X_REG_ISC,
		BQ2560X_REG_ISC_IINLIM_MASK,
		BQ2560X_REG_ISC_IINLIM_SHIFT,
		val);
}

static int bq2560x_set_charge_current(int value)
{
	int val;

	val = (value - REG02_ICHG_BASE) / REG02_ICHG_LSB;

	hwlog_info("set_charge_current [%x]=0x%x\n", BQ2560X_REG_CCC, val);

	return bq2560x_write_mask(BQ2560X_REG_CCC,
		BQ2560X_REG_CCC_ICHG_MASK,
		BQ2560X_REG_CCC_ICHG_SHIFT,
		val);
}

static int bq2560x_set_boost_current(int curr)
{
	int val;

	if (curr == BOOST_LIM_0P5A)
		val = REG02_BOOST_LIM_0P5A;
	else
		val = REG02_BOOST_LIM_1P2A;

	hwlog_info("set_boost_current [%x]=0x%x\n", BQ2560X_REG_REG_BVTRC, val);

	return bq2560x_write_mask(BQ2560X_REG_REG_BVTRC,
		BQ2560X_REG_REG_BVTRC_BOOSTV_MASK,
		BQ2560X_REG_REG_BVTRC_BOOSTV_SHIFT,
		val);
}

static int bq2560x_reused_set_cv(int value)
{
	int val;

	val = (value - REG04_VREG_BASE) / REG04_VREG_LSB;

	hwlog_info("reused_set_cv [%x]=0x%x\n", BQ2560X_REG_CVC, val);

	return bq2560x_write_mask(BQ2560X_REG_CVC,
		BQ2560X_REG_CVC_VREG_MASK,
		BQ2560X_REG_CVC_VREG_SHIFT,
		val);
}

static int bq2560x_set_terminal_voltage(int value)
{
	if ((g_bq2560x_cv > MIN_CV) && (value > g_bq2560x_cv)) {
		hwlog_info("set cv to custom_cv=%d\n", g_bq2560x_cv);
		value = g_bq2560x_cv;
	}

	g_cv_policy = value;

	if (g_cv_flag == 0)
		return bq2560x_reused_set_cv(value);
	else
		return 0;
}

static int bq2560x_set_dpm_voltage(int value)
{
	int val;

	val = (value - REG06_VINDPM_BASE) / REG06_VINDPM_LSB;

	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BQ2560X_REG_REG_BVTRC, val);

	return bq2560x_write_mask(BQ2560X_REG_REG_BVTRC,
		BQ2560X_REG_REG_BVTRC_VINDPM_MASK,
		BQ2560X_REG_REG_BVTRC_VINDPM_SHIFT,
		val);
}

static int bq2560x_reused_set_iterm(int value)
{
	int val;

	val = (value - REG03_ITERM_BASE) / REG03_ITERM_LSB;

	hwlog_info("reused_set_iterm [%x]=0x%x\n", BQ2560X_REG_PCTCC, val);

	return bq2560x_write_mask(BQ2560X_REG_PCTCC,
		BQ2560X_REG_PCTCC_ITERM_MASK,
		BQ2560X_REG_PCTCC_ITERM_SHIFT,
		val);
}

static int bq2560x_set_terminal_current(int value)
{
	int ret;
	int ichg = -hisi_battery_current();

	if (ichg > LIMIT_CURR) {
		ret = bq2560x_reused_set_cv(g_cv_policy + REG04_VREG_LSB);
		if (ret) {
			hwlog_err("set high v_term fail, reset to g_cv_policy\n");

			ret = bq2560x_reused_set_cv(g_cv_policy);
			if (ret)
				hwlog_err("set g_cv_policy error\n");

			return bq2560x_reused_set_iterm(value);
		}

		g_cv_flag = 1;
		return bq2560x_reused_set_iterm(TERM_CURR);
	}

	ret = bq2560x_reused_set_iterm(value);
	if (ret) {
		hwlog_err("set the iterm fail\n");
		return ret;
	}

	g_cv_flag = 0;
	return bq2560x_reused_set_cv(g_cv_policy);
}

static int bq2560x_set_charge_enable(int enable)
{
	return bq2560x_write_mask(BQ2560X_REG_POC,
		BQ2560X_REG_POC_CHG_CONFIG_MASK,
		BQ2560X_REG_POC_CHG_CONFIG_SHIFT,
		enable);
}

static int bq2560x_set_boost_voltage(int voltage)
{
	int val;

	if (voltage == BOOSTV_4850)
		val = REG06_BOOSTV_4P85V;
	else if (voltage == BOOSTV_5150)
		val = REG06_BOOSTV_5P15V;
	else if (voltage == BOOSTV_5300)
		val = REG06_BOOSTV_5P3V;
	else
		val = REG06_BOOSTV_5V;

	hwlog_info("set_boost_voltage [%x]=0x%x\n", BQ2560X_REG_REG_BVTRC, val);

	return bq2560x_write_mask(BQ2560X_REG_REG_BVTRC,
		BQ2560X_REG_REG_BVTRC_BOOSTV_MASK,
		BQ2560X_REG_REG_BVTRC_BOOSTV_SHIFT,
		val);
}

static int bq2560x_set_otg_enable(int enable)
{
	struct bq2560x_device_info *di = g_bq2560x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/*
	 * notice:
	 * why enable irq when entry to OTG mode only?
	 * because we care VBUS overloaded OCP or OVP's interrupt in boost mode
	 */
	if ((!di->irq_active) && (enable)) {
		di->irq_active = 1; /* active */
		enable_irq(di->irq_int);
	} else if ((di->irq_active) && (!enable)) {
		di->irq_active = 0; /* inactive */
		disable_irq(di->irq_int);
	}

	return bq2560x_write_mask(BQ2560X_REG_POC,
		BQ2560X_REG_POC_OTG_CONFIG_MASK,
		BQ2560X_REG_POC_OTG_CONFIG_SHIFT,
		enable);
}

static int bq2560x_set_term_enable(int enable)
{
	return bq2560x_write_mask(BQ2560X_REG_CTTC,
		BQ2560X_REG_CTTC_EN_TERM_MASK,
		BQ2560X_REG_CTTC_EN_TERM_SHIFT,
		enable);
}

static int bq2560x_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret;

	if (!state)
		return -1;

	ret = bq2560x_read_byte(BQ2560X_REG_SS, &reg);
	if (ret < 0) {
		hwlog_err("get_charge_state:reg_ss fail\n");
		return -1;
	}

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2560X_REG_SS, reg);

	if (!(reg & BQ2560X_REG_SS_PG_STAT_MASK))
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg & BQ2560X_REG_SS_CHRG_STAT_MASK) ==
		BQ2560X_REG_SS_CHRG_STAT_MASK)
		*state |= CHAGRE_STATE_CHRG_DONE;

	ret = bq2560x_read_byte(BQ2560X_REG_F, &reg);
	if (ret < 0) {
		hwlog_err("get_charge_state:reg_f fail\n");
		return -1;
	}

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2560X_REG_F, reg);

	if ((reg & BQ2560X_REG_F_FAULT_WDT_MASK) ==
		BQ2560X_REG_F_FAULT_WDT_MASK)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg & BQ2560X_REG_F_FAULT_BOOST_MASK) ==
		BQ2560X_REG_F_FAULT_BOOST_MASK)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if ((reg & BQ2560X_REG_F_FAULT_BAT_MASK) ==
		BQ2560X_REG_F_FAULT_BAT_MASK)
		*state |= CHAGRE_STATE_BATT_OVP;

	return ret;
}

static void sgm41511h_set_safe_time_enable(bool enable)
{
	int ret;
	int retry;

	for (retry = 0; retry < RETRY_TIMES; retry++) {
		ret = bq2560x_write_mask(BQ2560X_REG_CTTC,
			BQ2560X_REG_CTTC_EN_TIMER_MASK,
			BQ2560X_REG_CTTC_EN_TIMER_SHIFT, enable);
		if (ret == 0)
			break;
	}
	if (ret != 0)
		hwlog_err("set safe time enable fail\n");

	hwlog_info("set safe time en = %d, retry = %d\n", enable, retry);
}

static void sgm41511h_reset_for_safe_time(void)
{
	struct timespec64 ts = { 0 };

	if (g_safe_time_cnt < ENABLE_TIMES) {
		get_monotonic_boottime64(&ts);
		if ((ts.tv_sec - g_boot_time) > g_safe_pre_time) {
			g_safe_time_cnt++;
			g_safe_pre_time += SAFE_TIME_RESET;
			sgm41511h_set_safe_time_enable(false);
			msleep(5); /* delay 5ms for set reg */
			sgm41511h_set_safe_time_enable(true);
			hwlog_info("safety timer,cnt=%d, safe_time=%d\n",
				g_safe_time_cnt, g_safe_pre_time);
		}
	}
}

static int bq2560x_reset_watchdog_timer(void)
{
	if (g_sgm41511_exist)
		sgm41511h_reset_for_safe_time();

	return bq2560x_write_mask(BQ2560X_REG_POC,
		BQ2560X_REG_POC_WDT_RESET_MASK,
		BQ2560X_REG_POC_WDT_RESET_SHIFT,
		0x01);
}

static int bq2560x_get_ilim(void)
{
	return 0;
}

static int bq2560x_check_charger_plugged(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_mask(BQ2560X_REG_SS,
		BQ2560X_REG_SS_PG_STAT_MASK,
		BQ2560X_REG_SS_PG_STAT_SHIFT,
		&reg);
	if (ret < 0)
		return 0;

	hwlog_info("check_charger_plugged [%x]=0x%x\n", BQ2560X_REG_SS, reg);

	if (reg == BQ2560x_REG_SS_VBUS_PLUGGED)
		return REG08_POWER_GOOD;

	return 0;
}

static int bq2560x_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_byte(BQ2560X_REG_VINS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n", BQ2560X_REG_VINS, reg);

	if (reg & BQ2560X_REG_VINS_VINDPM_STAT_MASK)
		return TRUE;

	return FALSE;
}

static int bq2560x_check_input_vdpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_byte(BQ2560X_REG_VINS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_vdpm_state [%x]=0x%x\n", BQ2560X_REG_VINS, reg);

	if (reg & BQ2560X_REG_VINS_VINDPM_STAT_MASK)
		return TRUE;

	return FALSE;
}

static int bq2560x_check_input_idpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_byte(BQ2560X_REG_VINS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_idpm_state [%x]=0x%x\n", BQ2560X_REG_VINS, reg);

	if (reg & BQ2560X_REG_VINS_IINDPM_STAT_MASK)
		return TRUE;

	return FALSE;
}

static int bq2560x_dump_register(char *reg_value, int size)
{
	u8 reg[BQ2560X_REG_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i;
	int ret;

	if (!reg_value)
		return 0;

	memset(reg_value, 0, size);

	for (i = 0; i < BQ2560X_REG_NUM; i++) {
		ret = bq2560x_read_byte(i, &reg[i]);
		if (ret)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-8.2x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq2560x_get_register_head(char *reg_head, int size)
{
	char buff[BUF_LEN] = {0};
	int i;

	if (!reg_head)
		return 0;

	memset(reg_head, 0, size);

	for (i = 0; i < BQ2560X_REG_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[0x%x]  ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq2560x_set_batfet_disable(int disable)
{
	return bq2560x_write_mask(BQ2560X_REG_MOC,
		BQ2560X_REG_MOC_BATFET_DISABLE_MASK,
		BQ2560X_REG_MOC_BATFET_DISABLE_SHIFT,
		disable);
}

static int bq2560x_set_watchdog_timer(int value)
{
	int ret;
	int val;

	if (value == WDT_BASE || g_hiz_mode == TRUE)
		val = REG05_WDT_DISABLE;
	else if (value <= WDT_40S)
		val = REG05_WDT_40S;
	else if (value <= WDT_80S)
		val = REG05_WDT_80S;
	else
		val = REG05_WDT_160S;

	ret = bq2560x_write_mask(BQ2560X_REG_CTTC,
		BQ2560X_REG_CTTC_WDT_MASK,
		BQ2560X_REG_CTTC_WDT_SHIFT,
		val);
	if (ret)
		hwlog_err("set_watchdog_timer write fail\n");

	hwlog_info("set_watchdog_timer [%x]=0x%x\n", BQ2560X_REG_CTTC, val);

	if (value > 0)
		ret = bq2560x_reset_watchdog_timer();

	return ret;
}

static int bq2560x_set_charger_hiz(int enable)
{
	int ret;
	int ret2;
	static int first_in = 1;
	struct bq2560x_device_info *di = g_bq2560x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (g_shutdown_flag)
		enable = 0;

	if (enable > 0) {
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		if (di->hiz_iin_limit == 1 && is_uscp_hiz_mode() &&
			!is_in_rt_uscp_mode()) {
			hiz_iin_limit_flag = HIZ_IIN_FLAG_TRUE;

			if (first_in) {
				hwlog_info("uscp_hiz_mode HIZ, enable:%d, set 100mA\n",
					enable);
				first_in = 0;
				/* set inputcurrent to 100mA */
				return bq2560x_set_input_current(IINLIM_100);
			} else {
				return 0;
			}
		} else {
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */
			ret = bq2560x_write_mask(BQ2560X_REG_ISC,
				BQ2560X_REG_ISC_EN_HIZ_MASK,
				BQ2560X_REG_ISC_EN_HIZ_SHIFT,
				TRUE);

			g_hiz_mode = TRUE;

			ret2 = bq2560x_set_watchdog_timer(
				WATCHDOG_TIMER_DISABLE);
			if (ret2)
				hwlog_err("set_charger_hiz write fail\n");
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		}
#endif /* CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT */
	} else {
		hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;
		first_in = 1;

		ret = bq2560x_write_mask(BQ2560X_REG_ISC,
			BQ2560X_REG_ISC_EN_HIZ_MASK,
			BQ2560X_REG_ISC_EN_HIZ_SHIFT,
			FALSE);
		g_hiz_mode = FALSE;
	}

	return ret;
}

static struct charge_device_ops bq2560x_ops = {
	.chip_init = bq2560x_chip_init,
	.dev_check = bq2560x_device_check,
	.set_input_current = bq2560x_set_input_current,
	.set_charge_current = bq2560x_set_charge_current,
	.set_terminal_voltage = bq2560x_set_terminal_voltage,
	.set_dpm_voltage = bq2560x_set_dpm_voltage,
	.set_terminal_current = bq2560x_set_terminal_current,
	.set_charge_enable = bq2560x_set_charge_enable,
	.set_otg_enable = bq2560x_set_otg_enable,
	.set_otg_current = bq2560x_set_boost_current,
	.set_term_enable = bq2560x_set_term_enable,
	.get_charge_state = bq2560x_get_charge_state,
	.reset_watchdog_timer = bq2560x_reset_watchdog_timer,
	.dump_register = bq2560x_dump_register,
	.get_register_head = bq2560x_get_register_head,
	.set_watchdog_timer = bq2560x_set_watchdog_timer,
	.set_batfet_disable = bq2560x_set_batfet_disable,
	.get_ibus = bq2560x_get_ilim,
	.check_charger_plugged = bq2560x_check_charger_plugged,
	.check_input_dpm_state = bq2560x_check_input_dpm_state,
	.check_input_vdpm_state = bq2560x_check_input_vdpm_state,
	.check_input_idpm_state = bq2560x_check_input_idpm_state,
	.set_charger_hiz = bq2560x_set_charger_hiz,
	.get_charge_current = NULL,
	.set_boost_voltage = bq2560x_set_boost_voltage,
};

static struct charger_otg_device_ops bq2560x_otg_ops = {
	.chip_name = "bq2560x",
	.otg_set_charger_enable = bq2560x_set_charge_enable,
	.otg_set_enable = bq2560x_set_otg_enable,
	.otg_set_current = bq2560x_set_boost_current,
	.otg_set_watchdog_timer = bq2560x_set_watchdog_timer,
	.otg_reset_watchdog_timer = bq2560x_reset_watchdog_timer,
};

static int bq2560x_fcp_init(struct bq2560x_device_info *info)
{
	int ret;
	struct device_node *np = NULL;

	if (!info || !info->dev || !info->dev->of_node) {
		hwlog_err("info is null\n");
		goto fail_init_fcp;
	}

	np = info->dev->of_node;
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fcp_support", &(info->fcp_support), 0);

	if (!info->fcp_support)
		goto fail_init_fcp;

	info->gpio_fcp = of_get_named_gpio(np, "gpio_fcp", 0);
	hwlog_info("gpio_fcp=%d\n", info->gpio_fcp);

	if (!gpio_is_valid(info->gpio_fcp)) {
		hwlog_err("gpio is not valid\n");
		goto fail_init_fcp;
	}

	ret = gpio_request(info->gpio_fcp, "gpio_fcp");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_init_fcp;
	}

	/* gpio_fcp init to input mode */
	ret = gpio_direction_input(info->gpio_fcp);
	if (ret < 0) {
		gpio_free(info->gpio_fcp);
		hwlog_err("gpio set input fail\n");
		goto fail_init_fcp;
	}

	return 0;

fail_init_fcp:
	if (info)
		info->fcp_support = 0;
	return -1;
}

static int bq2560x_set_ovpthreshold(int threshld)
{
	int ret;
	int try_cnt = 0;

	while (try_cnt < RETRY_MAX3) {
		try_cnt++;
		ret = bq2560x_write_mask(BQ2560X_REG_REG_BVTRC,
			BQ2560X_REG_REG_BVTRC_OVP_MASK,
			BQ2560X_REG_REG_BVTRC_OVP_SHIFT,
			threshld);
		if (!ret)
			return ret;

		msleep(DELAY_10MS);
	}

	hwlog_err("set ovp %d fail\n", threshld);
	return -1;
}

static int bq2560x_is_ovpstate(bool *is_ovp)
{
	u8 reg = 0;
	int ret;

	ret = bq2560x_read_byte(BQ2560X_REG_VINS, &reg);
	if (ret) {
		*is_ovp = false;
		hwlog_err("get ovp state fail\n");
		return ret;
	}

	hwlog_info("fcp ovp read reg [%x]=0x%x\n", BQ2560X_REG_VINS, reg);
	if ((reg & BQ2560X_REG_VINS_ACOV_STAT_MASK) ==
		BQ2560X_REG_VINS_ACOV_STAT_MASK)
		*is_ovp = true;
	else
		*is_ovp = false;

	return ret;
}

static int bq2560x_fcp_reset(void)
{
	struct bq2560x_device_info *di = g_bq2560x_dev;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	/* reset gpio to input status */
	gpio_set_value(di->gpio_fcp, GPIO_LOW);
	ret = gpio_direction_input(di->gpio_fcp);

	hwlog_info("fcp reset gpio_fcp\n");
	return ret;
}

static int bq2560x_fcp_adapter_detect(void)
{
	int ret;
	int try_cnt = 0;
	bool is_ovp = false;
	struct bq2560x_device_info *di = g_bq2560x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* first: set ovp 6p2 to detect vbus 9v */
	ret = bq2560x_set_ovpthreshold(REG06_OVP_6P2V);
	if (ret)
		return -1;

	/*
	 * second:
	 * 1. wait 1.5s after DCP detect
	 * 2. set gpio_fcp to output high
	 * 3. wait 100ms up to 9v
	 */
	msleep(DELAY_1500MS);
	gpio_direction_output(di->gpio_fcp, GPIO_LOW);
	gpio_set_value(di->gpio_fcp, GPIO_HIGH);
	msleep(DELAY_100MS);
	hwlog_info("fcp detect set gpio_fcp = %d\n",
		gpio_get_value(di->gpio_fcp));

	/* third: detect ovp to judge 9v */
	while (try_cnt < RETRY_MAX3) {
		try_cnt++;
		ret = bq2560x_is_ovpstate(&is_ovp);
		if (!ret && is_ovp == true) {
			/* up 9v succ must reset ovp to 14p3 */
			(void)bq2560x_set_ovpthreshold(REG06_OVP_14P3V);
			hwlog_info("fcp detect upto9v succ\n");
			return 0;
		}
		msleep(DELAY_50MS);
	}

	/* fourth: reset when detect 9v failed */
	(void)bq2560x_set_ovpthreshold(REG06_OVP_14P3V);
	(void)bq2560x_fcp_reset();

	hwlog_err("fcp detect upto9v fail\n");
	return -1;
}

static int bq2560x_fcp_reg_read_block(int reg, int *val, int num)
{
	int i;

	if (!val) {
		hwlog_err("val is null\n");
		return -1;
	}

	for (i = 0; i < num; i++)
		val[i] = 0;

	return 0;
}

static int bq2560x_fcp_reg_write_block(int reg, const int *val, int num)
{
	return 0;
}

static struct fcp_protocol_ops bq2560x_fcp_protocol_ops = {
	.chip_name = "bq2560x",
	.reg_read = bq2560x_fcp_reg_read_block,
	.reg_write = bq2560x_fcp_reg_write_block,
	.detect_adapter = bq2560x_fcp_adapter_detect,
	.soft_reset_master = bq2560x_fcp_reset,
	.soft_reset_slave = bq2560x_fcp_reset,
	.get_master_status = NULL,
	.stop_charging_config = bq2560x_fcp_reset,
	.is_accp_charger_type = NULL,
};

static void bq2560x_irq_work(struct work_struct *work)
{
	struct bq2560x_device_info *di = NULL;
	u8 reg = 0;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct bq2560x_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(100); /* sleep 100ms */

	ret = bq2560x_read_byte(BQ2560X_REG_F, &reg);
	if (ret)
		hwlog_err("irq_work read fail\n");

	hwlog_err("boost_ovp_reg [%x]=0x%x\n", BQ2560X_REG_F, reg);

	if (reg & BQ2560X_REG_F_FAULT_BOOST_MASK) {
		hwlog_info("CHARGE_FAULT_BOOST_OCP happened\n");

		atomic_notifier_call_chain(&fault_notifier_list,
			CHARGE_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq2560x_interrupt(int irq, void *_di)
{
	struct bq2560x_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	hwlog_info("bq2560x int happened\n");

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bq2560x_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bq2560x_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bq2560x_dev = di;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq2560x_irq_work);

	/* check if bq2560x exist */
	if (bq2560x_device_check() == CHARGE_IC_BAD) {
		hwlog_err("bq2560x not exists\n");
		ret = -EINVAL;
		goto bq2560x_fail_0;
	}

	if (sgm41511_device_check())
		g_sgm41511_exist = 1;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hiz_iin_limit", &(di->hiz_iin_limit), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"custom_cv", &g_bq2560x_cv, 0);

	di->gpio_cd = of_get_named_gpio(np, "gpio_cd", 0);
	hwlog_info("gpio_cd=%d\n", di->gpio_cd);

	if (!gpio_is_valid(di->gpio_cd)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto bq2560x_fail_0;
	}

	ret = gpio_request(di->gpio_cd, "charger_cd");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto bq2560x_fail_0;
	}

	/* set gpio to control CD pin to disable/enable bq2560x IC */
	ret = gpio_direction_output(di->gpio_cd, 0);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto bq2560x_fail_1;
	}

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto bq2560x_fail_1;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto bq2560x_fail_1;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		goto bq2560x_fail_2;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto bq2560x_fail_2;
	}

	ret = request_irq(di->irq_int, bq2560x_interrupt, IRQF_TRIGGER_FALLING,
		"charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto bq2560x_fail_2;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;

	ret = charge_ops_register(&bq2560x_ops);
	if (ret) {
		hwlog_err("bq2560x charge ops register fail\n");
		goto bq2560x_fail_3;
	}

	ret = charger_otg_ops_register(&bq2560x_otg_ops);
	if (ret) {
		hwlog_err("bq2560x charger_otg ops register fail\n");
		goto bq2560x_fail_3;
	}

	 bq2560x_sysfs_create_group(di->dev);

	/* set bq2560x boost voltage */
	ret = bq2560x_set_boost_voltage(BOOSTV_5000);
	if (ret < 0)
		hwlog_err("set bq2560x boost voltage fail\n");

	if (!bq2560x_fcp_init(di))
		fcp_protocol_ops_register(&bq2560x_fcp_protocol_ops);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("probe end\n");
	return 0;

bq2560x_fail_3:
	free_irq(di->irq_int, di);
bq2560x_fail_2:
	gpio_free(di->gpio_int);
bq2560x_fail_1:
	gpio_free(di->gpio_cd);
bq2560x_fail_0:
	kfree(di);
	g_bq2560x_dev = NULL;

	return ret;
}

static void bq2560x_shutdown(struct i2c_client *client)
{
	int ret;
	u8 part;
	u8 reg = 0;

	ret = bq2560x_read_byte(BQ2560X_REG_VPRS, &reg);
	if (ret)
		return;

	part = reg & BQ2560X_REG_VPRS_PART_MASK;
	if (part) {
		g_shutdown_flag = TRUE;
		/* 0:close hiz mode */
		ret = bq2560x_set_charger_hiz(0);
		if (ret)
			hwlog_err("set bq2560x set hiz fail\n");
	}
}

static int bq2560x_remove(struct i2c_client *client)
{
	struct bq2560x_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	bq2560x_sysfs_remove_group(di->dev);

	gpio_set_value(di->gpio_cd, 1);

	if (di->gpio_cd)
		gpio_free(di->gpio_cd);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_fcp)
		gpio_free(di->gpio_fcp);

	kfree(di);
	g_bq2560x_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq25601);
static const struct of_device_id bq2560x_of_match[] = {
	{
		.compatible = "huawei,bq2560x_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq2560x_i2c_id[] = {
	{ "bq2560x_charger", 0 }, {}
};

static struct i2c_driver bq2560x_driver = {
	.probe = bq2560x_probe,
	.remove = bq2560x_remove,
	.shutdown = bq2560x_shutdown,
	.id_table = bq2560x_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq2560x_charger",
		.of_match_table = of_match_ptr(bq2560x_of_match),
	},
};

static int __init bq2560x_init(void)
{
	return i2c_add_driver(&bq2560x_driver);
}

static void __exit bq2560x_exit(void)
{
	i2c_del_driver(&bq2560x_driver);
}

module_init(bq2560x_init);
module_exit(bq2560x_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq2560x charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
