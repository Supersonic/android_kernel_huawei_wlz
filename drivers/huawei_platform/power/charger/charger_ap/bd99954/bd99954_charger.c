/*
 * bd99954_charger.c
 *
 * bd99954 driver
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
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
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
#include <linux/bitops.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#include <huawei_platform/power/series_batt_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include "bd99954_charger.h"

#define HWLOG_TAG bd99954_charger
HWLOG_REGIST();

#define OPER(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define BUF_LEN                26
#define RE_CHG_VOL             100
#define ITERM_DEFAULT          128
#define VTERM_ACC              30
#define READ_NUM               5
#define CHG_DONE_CNT           5

struct bd99954_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	int irq_active;
	int irq_int;
	int gpio_int;
	int gpio_otg;
	int term_curr;
	int iterm_pre;
	bool chg_done;
};

static struct bd99954_device_info *g_bd99954_dev;
static int g_chg_done_cnt;

static int bd99954_get_vbat_mv(void);
static int bd99954_get_terminal_voltage(void);

static int bd99954_read_word(u8 reg, u16 *value)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;
	u16 reg_value;

	if (!di || !di->client || !value)
		return -EINVAL;

	ret = i2c_smbus_read_word_swapped(di->client, reg);
	if (ret < 0) {
		hwlog_err("read reg:0x%x fail\n", reg);
		return ret;
	}

	reg_value = (u16)ret;
	*value = OPER(reg_value);
	return ret;
}

static int bd99954_write_word(u8 reg, u16 value)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;

	if (!di || !di->client)
		return -EINVAL;

	value = OPER(value);
	ret = i2c_smbus_write_word_swapped(di->client, reg, value);
	if (ret < 0)
		hwlog_err("write reg:0x%x fail\n", reg);

	return ret;
}

static int bd99954_write_mask(u8 reg, u16 mask, u16 shift, u16 value)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;
	u16 reg_value;

	if (!di || !di->client)
		return -EINVAL;

	ret = i2c_smbus_read_word_swapped(di->client, reg);
	if (ret < 0) {
		hwlog_err("read reg:0x%x fail\n", reg);
		return ret;
	}

	reg_value = (u16)ret;
	reg_value = OPER(reg_value);
	reg_value = (reg_value & (~mask)) | (value << shift);
	value = OPER(reg_value);
	ret = i2c_smbus_write_word_swapped(di->client, reg, value);
	if (ret < 0)
		hwlog_err("write reg:0x%x fail\n", reg);

	return ret;
}

#ifdef CONFIG_SYSFS
#define BD99954_SYSFS_FIELD(_name, m, store) \
{ \
	.attr = __ATTR(_name, m, bd99954_sysfs_show, store), \
	.reg = 0X00, \
}

#define BD99954_SYSFS_FIELD_RW(_name) \
	BD99954_SYSFS_FIELD(_name, 0644, bd99954_sysfs_store)

#define BD99954_SYSFS_FIELD_RO(_name) \
	BD99954_SYSFS_FIELD(_name, 0444, NULL)

static ssize_t bd99954_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bd99954_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct bd99954_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
};

static struct bd99954_sysfs_field_info bd99954_sysfs_field_tbl[] = {
	/* sysfs name reg field in reg */
	BD99954_SYSFS_FIELD_RW(reg_addr),
	BD99954_SYSFS_FIELD_RW(reg_value),
};

#define BD99954_SYSFS_ATTRS_SIZE  (ARRAY_SIZE(bd99954_sysfs_field_tbl) + 1)

static struct attribute *bd99954_sysfs_attrs[BD99954_SYSFS_ATTRS_SIZE];

static const struct attribute_group bd99954_sysfs_attr_group = {
	.attrs = bd99954_sysfs_attrs,
};

static void bd99954_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(bd99954_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		bd99954_sysfs_attrs[i] = &bd99954_sysfs_field_tbl[i].attr.attr;

	bd99954_sysfs_attrs[limit] = NULL;
}

static struct bd99954_sysfs_field_info *bd99954_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit = ARRAY_SIZE(bd99954_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strcmp(name, bd99954_sysfs_field_tbl[i].attr.attr.name))
			break;
	}

	if (i >= limit)
		return NULL;

	return &bd99954_sysfs_field_tbl[i];
}

static ssize_t bd99954_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bd99954_sysfs_field_info *info = NULL;
	struct bd99954_sysfs_field_info *info2 = NULL;
	int ret;
	u16 reg_value;

	if (!g_bd99954_dev) {
		hwlog_err("g_bd99954_dev is null\n");
		return -EINVAL;
	}

	info = bd99954_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("%s info is null\n", __func__);
		return -EINVAL;
	}

	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr")))
		return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", info->reg);

	if (!strncmp(("reg_value"), attr->attr.name, strlen("reg_value"))) {
		info2 = bd99954_sysfs_field_lookup("reg_addr");
		if (!info2)
			return -EINVAL;
		info->reg = info2->reg;
	}

	ret = bd99954_read_word(info->reg, &reg_value);
	if (ret < 0)
		return ret;

	return scnprintf(buf, PAGE_SIZE, "0x%x\n", reg_value);
}

static ssize_t bd99954_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bd99954_sysfs_field_info *info = NULL;
	struct bd99954_sysfs_field_info *info2 = NULL;
	int ret;
	u16 v;

	info = bd99954_sysfs_field_lookup(attr->attr.name);
	if (!info) {
		hwlog_err("%s info is null\n", __func__);
		return -EINVAL;
	}

	ret = kstrtou16(buf, 0, &v);
	if (ret < 0) {
		hwlog_err("get kstrtou8 failed\n");
		return ret;
	}

	hwlog_info("%s v = 0x%x\n", __func__, v);
	if (!strncmp("reg_value", attr->attr.name, strlen("reg_value"))) {
		info2 = bd99954_sysfs_field_lookup("reg_addr");
		if (!info2)
			return -EINVAL;
		info->reg = info2->reg;
	}
	if (!strncmp(("reg_addr"), attr->attr.name, strlen("reg_addr"))) {
		if (v < (u8)BD99954_REG_TOTAL_NUM) {
			info->reg = v;
			return count;
		} else {
			return -EINVAL;
		}
	}

	ret = bd99954_write_word(info->reg, v);
	if (ret < 0)
		return ret;

	return count;
}

static void bd99954_sysfs_create_group(struct device *dev)
{
	bd99954_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "bd99954",
		dev, &bd99954_sysfs_attr_group);
}

static void bd99954_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "bd99954",
		dev, &bd99954_sysfs_attr_group);
}

#else

static inline void bd99954_sysfs_create_group(struct device *dev)
{
}

static inline void bd99954_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int init_charge_reg(void)
{
	int ret;

	/* Extended Commands, PROTECT_SET:16'h0000 MAP_SET:16'h0001 */
	ret = bd99954_write_word(BD99954_REG_PROTECT, 0);
	ret |= bd99954_write_word(BD99954_REG_MAP_SET_CHARGE, 0x01);

	ret |= bd99954_write_word(BD99954_REG_SMBREG, BD99954_REG_SMBREG_0H);
	/* enable int: vbus_rbuv_det,vbus_ov_det */
	ret |= bd99954_write_word(BD99954_REG_INT1_SET, 0x8020);
	/* enable int: tmp_out_det,ibat_short_det,vbat_ov_det */
	ret |= bd99954_write_word(BD99954_REG_INT3_SET, 0x0850);
	/* enable int: vsys_ov,vsys_sht_curr,vsys_uv */
	ret |= bd99954_write_word(BD99954_REG_INT4_SET, 0x25);

	ret |= bd99954_write_word(BD99954_REG_0X20, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X21, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X22, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X23, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X24, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X25, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X28, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X30, BD99954_REG_VALUE_0X00);
	return ret;
}

static int bd99954_5v_chip_init(struct bd99954_device_info *di)
{
	int ret = 0;

	/* Fast Charge Current Limit 960mA */
	ret |= bd99954_write_word(BD99954_REG_ICHG_SET,
		0xF << BD99954_ICHG_SHIFT);
	/* term curr 128ma */
	ret |= bd99954_write_word(BD99954_REG_ITERM_SET,
		0x2 << BD99954_ITERM_SHIFT);
	/* Battery Precharge to Fast Charge Threshold 7v */
	ret |= bd99954_write_word(BD99954_REG_VSYSREG_SET,
		0x6E << BD99954_VSYSREG_SHIFT);
	/* fast charge watchdog 12h */
	ret |= bd99954_write_word(BD99954_REG_CHGWDT_SET, 0xB410);
	/* vbat ovp 9488mv */
	ret |= bd99954_write_word(BD99954_REG_VBATOVP_SET, 0x2510);
	/* enabled input dpm */
	ret |= bd99954_write_mask(BD99954_REG_CHGOP_SET1,
		BD99954_DIS_AUTO_LIMIIN_MASK,
		BD99954_DIS_AUTO_LIMIIN_SHIFT,
		BD99954_EN_AUTO_LIMIIN);
	ret |= bd99954_write_word(BD99954_REG_ILIM_DECREASE_SET,
		BD99954_ILIM_DECREASE_VALUE);
	ret |= bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_ILIMIT_RESET_STEP_MASK,
		BD99954_ILIMIT_RESET_STEP_SHIFT,
		BD99954_ILIMIT_RESET_STEP_VALUE);
	ret |= bd99954_write_mask(BD99954_REG_VIN_CTRL_SET,
		BD99954_VCC_INPUT_EN_MASK,
		BD99954_VCC_INPUT_EN_SHIFT,
		BD99954_DISABLE_VCC_INPUT);
	/* disable top-off */
	ret |= bd99954_write_mask(BD99954_REG_CHGOP_SET1,
		BD99954_REG_AUTO_TOP_MASK,
		BD99954_REG_AUTO_TOP_SHIFT,
		BD99954_DISABLE_AUTO_TOP);
	return ret;
}

static int bd99954_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bd99954_device_info *di = g_bd99954_dev;

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -EINVAL;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bd99954_5v_chip_init(di);
		break;
	default:
		hwlog_err("invalid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bd99954_set_input_current(int value)
{
	int limit_current;
	u16 iin_limit;

	limit_current = value;
	if (limit_current < BD99954_IBUS_LIMIT_MIN)
		limit_current = BD99954_IBUS_LIMIT_MIN;
	else if (limit_current > BD99954_IBUS_LIMIT_MAX)
		limit_current = BD99954_IBUS_LIMIT_MAX;

	iin_limit = limit_current / BD99954_IBUS_LIMIT_STEP;
	hwlog_info("set_input_current [%x]=0x%x\n",
		BD99954_REG_IBUS_LIM_SET, iin_limit);

	return bd99954_write_word(BD99954_REG_IBUS_LIM_SET,
		iin_limit << BD99954_IBUS_LIMIT_SHIFT);
}

static int bd99954_set_input_voltage(int value)
{
	int vindpm_voltage;
	u16 vindpm;

	vindpm_voltage = value;
	if (vindpm_voltage > BD9995_VBUS_MAX)
		vindpm_voltage = BD9995_VBUS_MAX;
	else if (vindpm_voltage < BD9995_VBUS_MIN)
		vindpm_voltage = BD9995_VBUS_MIN;

	vindpm = vindpm_voltage / BD9995_VBUS_STEP;
	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BD99954_REG_VBUS_SET, vindpm);
	return bd99954_write_word(BD99954_REG_VBUS_SET,
		vindpm << BD9995_VBUS_SHIFT);
}

static int bd99954_check_input_dpm_state(void)
{
	u16 reg_value = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &reg_value);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n",
		BD99954_REG_VBUS_VCC_STATUS, reg_value);

	if (reg_value & BD99954_VBUS_CLPS_MASK)
		return TRUE;

	return FALSE;
}

static int bd99954_set_charge_current(int value)
{
	int currentma;
	u16 ichg;

	currentma = value;
	if (currentma < BD99954_ICHG_MIN)
		currentma = BD99954_ICHG_MIN;
	else if (currentma > BD99954_ICHG_MAX)
		currentma = BD99954_ICHG_MAX;

	ichg = currentma / BD99954_ICHG_STEP;
	hwlog_info("set_charge_current [%x]=0x%x\n",
		BD99954_REG_ICHG_SET, ichg);

	return bd99954_write_word(BD99954_REG_ICHG_SET,
		ichg << BD99954_ICHG_SHIFT);
}

static int bd99954_set_terminal_voltage(int value)
{
	int voltagemv;
	u16 voreg, vrechg;
	int ret;

	voltagemv = value;
	if (voltagemv < BD99954_VCHARGE_MIN)
		voltagemv = BD99954_VCHARGE_MIN;
	else if (voltagemv > BD99954_VCHARGE_MAX)
		voltagemv = BD99954_VCHARGE_MAX;

	voreg = voltagemv / BD99954_VCHARGE_STEP;
	hwlog_info("set_terminal_voltage [%x]=0x%x\n",
		BD99954_REG_VFASTCHG_REG_SET1, voreg);

	vrechg = (voltagemv - BD99954_VRECHG_DEFAULT) / BD99954_VRECHG_STEP;
	ret = bd99954_write_word(BD99954_REG_VRECHG_SET,
		vrechg << BD99954_VRECHG_SHITF);
	ret |= bd99954_write_word(BD99954_REG_VFASTCHG_REG_SET1,
		voreg << BD99954_VFASTCHG_SHIFT);
	return ret;
}

static int bd99954_set_terminal_current(int value)
{
	int term_currentma;
	u16 iterm_reg;

	term_currentma = value;
	if (term_currentma < BD99954_ITERM_MIN)
		term_currentma = BD99954_ITERM_MIN;
	else if (term_currentma > BD99954_ITERM_MAX)
		term_currentma = BD99954_ITERM_MAX;

	g_bd99954_dev->term_curr = value;
	iterm_reg = term_currentma / BD99954_ITERM_STEP;
	hwlog_info("set_terminal_current [%x]=0x%x\n",
		BD99954_REG_ITERM_SET, iterm_reg);

	return bd99954_write_word(BD99954_REG_ITERM_SET,
		iterm_reg << BD99954_ITERM_SHIFT);
}

/*
 * because the precision of the charge termination current of bd99954 is low,
 * the coulometer is used to charging termination
 */
static void bd99954_charge_term_check(struct bd99954_device_info *di)
{
	int curr, curr_avg;
	int vterm, vterm_real;
	int vol_sum = 0;
	int vol_avg;
	int bat_max;
	int i;

	vterm = series_batt_get_vterm_single();
	vterm_real = vterm - VTERM_ACC;
	curr_avg = hisi_battery_current_avg();
	curr = -hisi_battery_current();

	for (i = 0; i < READ_NUM; i++)
		vol_sum += hw_battery_voltage(BAT_ID_MAX);

	vol_avg = vol_sum / READ_NUM;
	hwlog_info("vterm:%d bat_max:%d curr:%d curr_avg:%d iterm:%d\n",
		vterm, vol_avg, curr, curr_avg, di->term_curr);

	/* if term curr changes, continue charging */
	if (di->term_curr != di->iterm_pre) {
		hwlog_info("term curr change\n");
		di->chg_done = FALSE;
		return;
	}

	if ((vol_avg > vterm_real) &&
		(curr_avg < di->term_curr) &&
		(curr < di->term_curr)) {
		bat_max = hw_battery_voltage(BAT_ID_MAX);
		g_chg_done_cnt++;
		hwlog_info("g_chg_done_cnt:%d\n", g_chg_done_cnt);
	} else {
		g_chg_done_cnt = 0;
	}

	/* re-charge */
	if (di->chg_done) {
		if (vol_avg < (vterm - RE_CHG_VOL)) {
			di->chg_done = FALSE;
			hwlog_info("re-charge\n");
		}
	}

	if (g_chg_done_cnt >= CHG_DONE_CNT) {
		g_chg_done_cnt = CHG_DONE_CNT;
		di->chg_done = TRUE;
		hwlog_info("charge done\n");
	}
}

static int bd99954_set_charge_enable(int enable)
{
	struct bd99954_device_info *di = g_bd99954_dev;

	hwlog_info("charge enable:%d\n", enable);
	if (enable) {
		bd99954_charge_term_check(di);
	} else {
		g_chg_done_cnt = 0;
		di->chg_done = FALSE;
	}

	if (di->chg_done)
		enable = 0;

	di->iterm_pre = di->term_curr;
	return bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_REG_CHG_EN_MASK, BD99954_REG_CHG_EN_SHIFT,
		enable);
}

static int bd99954_set_otg_enable(int enable)
{
	if (enable) {
		wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_BOOST_GPIO_OTG);
		gpio_set_value(g_bd99954_dev->gpio_otg, enable);
	} else {
		gpio_set_value(g_bd99954_dev->gpio_otg, enable);
		boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_BOOST_GPIO_OTG);
		wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
	}

	return 0;
}

static int bd99954_get_charge_state(unsigned int *state)
{
	u16 chg_status = 0;
	u16 vbatt_status = 0;
	u16 vbus_status = 0;
	u16 wdt_status = 0;
	int vbat_max;
	int vterm;

	if (!state)
		return -EINVAL;

	bd99954_read_word(BD99954_REG_CHGSTM_STATUS, &chg_status);
	bd99954_read_word(BD99954_REG_VBAT_VSYS_STATUS, &vbatt_status);
	bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &vbus_status);
	bd99954_read_word(BD99954_REG_WDT_STATUS, &wdt_status);
	vterm = series_batt_get_vterm_single();
	vbat_max = hw_battery_voltage(BAT_ID_MAX);

	hwlog_info("0x0:%0x, 0x1:%0x, 0x2:%0x\n",
		chg_status, vbatt_status, vbus_status);

	if (vbatt_status & BD99954_VBAT_OVP_MASK)
		*state |= CHAGRE_STATE_BATT_OVP;
	if (vbus_status & BD99954_VBUS_OVP_MASK)
		*state |= CHAGRE_STATE_VBUS_OVP;
	if (((chg_status & BD99954_CURR_STATE_MASK) ==
		BD99954_FAST_CHARGE_MODE) && (vbat_max > (vterm - RE_CHG_VOL)))
		*state |= CHAGRE_STATE_CV_MODE;
	if (g_bd99954_dev->chg_done)
		*state |= CHAGRE_STATE_CHRG_DONE;

	return 0;
}

static int bd99954_get_ibus_ma(void)
{
	u16 ibus = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_IBUS_AVE_VAL, &ibus);
	if (ret < 0)
		return -1;
	return (int)ibus * BD99954_IBUS_VAL_STEP;
}

static int bd99954_get_vbus_mv(unsigned int *vbus_mv)
{
	u16 vbus = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBUS_AVE_VAL, &vbus);
	if (ret < 0)
		return ret;

	*vbus_mv = (unsigned int)vbus * BD99954_VBUS_VAL_STEP;
	return 0;
}

static int bd99954_get_vbat_mv(void)
{
	u16 vbat = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBAT_AVE_VAL, &vbat);
	if (ret < 0)
		return -1;

	vbat = (unsigned int)vbat * BD99954_REG_VBAT_VAL_STEP;
	return vbat;
}

static int bd99954_set_otg_current(int value)
{
	return 0;
}

static int bd99954_set_charger_hiz(int enable)
{
	return bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_REG_USB_SUS_MASK, BD99954_REG_USB_SUS_SHIFT,
		enable);
}

static int bd99954_get_terminal_voltage(void)
{
	u16 vterm_reg = 0;
	int vterm;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VFASTCHG_REG_SET1, &vterm_reg);
	if (ret < 0)
		return -1;

	vterm_reg = vterm_reg >> BD99954_VFASTCHG_SHIFT;
	vterm = (int)vterm_reg * BD99954_VCHARGE_STEP;
	return vterm;
}

static int bd99954_dump_register(char *reg_value, int size)
{
	u16 reg[BD99954_REG_TOTAL_NUM] = { 0 };
	char buff[BUF_LEN] = { 0 };
	int i;
	int ret;

	if (!reg_value) {
		hwlog_err("reg_value is null\n");
		return -EINVAL;
	}

	memset(reg_value, 0, size);

	for (i = 0; i < BD99954_REG_TOTAL_NUM; i++) {
		ret = bd99954_read_word(i, &reg[i]);
		if (ret < 0)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-7.2x   ", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bd99954_get_register_head(char *reg_head, int size)
{
	char buff[BUF_LEN] = { 0 };
	int i;

	if (!reg_head)
		return -EINVAL;

	memset(reg_head, 0, size);

	for (i = 0; i < BD99954_REG_TOTAL_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[0x%-2.2x]   ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bd99954_get_input_current_set(void)
{
	u16 iin_reg = 0;
	int iin_set;
	int ret;

	ret = bd99954_read_word(BD99954_REG_CUR_IBUS_LIM_SET, &iin_reg);
	if (ret < 0)
		return -1;

	iin_set = (int)iin_reg * BD99954_CUR_IBUS_LIM_SET_STEP;
	return iin_set;
}

static int bd99954_device_check(void)
{
	return CHARGE_IC_GOOD;
}

static int bd99954_set_covn_start(int enable)
{
	return 0;
}

static int bd99954_set_batfet_disable(int disable)
{
	return 0;
}

static int bd99954_set_watchdog_timer(int value)
{
	return 0;
}

static int bd99954_set_term_enable(int enable)
{
	return 0;
}

static int bd99954_reset_watchdog_timer(void)
{
	return 0;
}

static struct charge_device_ops bd99954_chg_ops = {
	.chip_init = bd99954_chip_init,
	.dev_check = bd99954_device_check,
	.set_input_current = bd99954_set_input_current,
	.set_dpm_voltage = bd99954_set_input_voltage,
	.set_charge_current = bd99954_set_charge_current,
	.set_terminal_voltage = bd99954_set_terminal_voltage,
	.set_terminal_current = bd99954_set_terminal_current,
	.set_charge_enable = bd99954_set_charge_enable,
	.set_otg_enable = bd99954_set_otg_enable,
	.set_term_enable = bd99954_set_term_enable,
	.get_charge_state = bd99954_get_charge_state,
	.reset_watchdog_timer = bd99954_reset_watchdog_timer,
	.dump_register = bd99954_dump_register,
	.get_register_head = bd99954_get_register_head,
	.set_watchdog_timer = bd99954_set_watchdog_timer,
	.set_batfet_disable = bd99954_set_batfet_disable,
	.get_ibus = bd99954_get_ibus_ma,
	.get_vbus = bd99954_get_vbus_mv,
	.set_covn_start = bd99954_set_covn_start,
	.set_otg_current = bd99954_set_otg_current,
	.set_charger_hiz = bd99954_set_charger_hiz,
	.get_iin_set = bd99954_get_input_current_set,
	.get_terminal_voltage = bd99954_get_terminal_voltage,
	.check_input_dpm_state = bd99954_check_input_dpm_state,
	.soft_vbatt_ovp_protect = NULL,
	.rboost_buck_limit = NULL,
	.get_charge_current = NULL,
	.turn_on_ico = NULL,
	.set_force_term_enable = NULL,
};

static struct charger_otg_device_ops bd99954_otg_ops = {
	.chip_name = "bd99954",
	.otg_set_charger_enable = bd99954_set_charge_enable,
	.otg_set_enable = bd99954_set_otg_enable,
	.otg_set_current = bd99954_set_otg_current,
	.otg_set_watchdog_timer = bd99954_set_watchdog_timer,
	.otg_reset_watchdog_timer = bd99954_reset_watchdog_timer,
};

struct hw_batt_vol_ops bd99954_vbatt_ops = {
	.get_batt_vol = bd99954_get_vbat_mv,
};

static void bd99954_irq_work(struct work_struct *work)
{
	struct bd99954_device_info *di = NULL;
	u16 chg_status = 0;
	u16 vbatt_status = 0;
	u16 vbus_status = 0;
	u16 chg_op_status = 0;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct bd99954_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(100); /* sleep 100ms */

	bd99954_read_word(BD99954_REG_CHGSTM_STATUS, &chg_status);
	bd99954_read_word(BD99954_REG_VBAT_VSYS_STATUS, &vbatt_status);
	bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &vbus_status);
	bd99954_read_word(BD99954_REG_CHGOP_STATUS, &chg_op_status);
	hwlog_info("status reg 0x01-0x03:%u,%u,%u,%u\n",
		chg_status, vbatt_status, vbus_status, chg_op_status);

	if (vbus_status & BD99954_VBUS_OVP_MASK) {
		hwlog_info("vbus ovp happened\n");
		atomic_notifier_call_chain(&fault_notifier_list,
			CHARGE_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bd99954_interrupt(int irq, void *_di)
{
	struct bd99954_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bd99954 int happened %d\n", di->irq_active);

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bd99954_gpio_init(struct bd99954_device_info *di,
	struct device_node *np)
{
	int ret = -1;

	di->gpio_otg = of_get_named_gpio(np, "gpio_otg", 0);
	hwlog_info("gpio_otg:%d\n", di->gpio_otg);

	if (!gpio_is_valid(di->gpio_otg)) {
		hwlog_err("gpio is not valid\n");
		return ret;
	}

	ret = gpio_request(di->gpio_otg, "gpio_otg");
	if (ret) {
		hwlog_err("gpio request fail\n");
		return ret;
	}

	ret = gpio_direction_output(di->gpio_otg, 0);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		gpio_free(di->gpio_otg);
		return ret;
	}

	return 0;
}

static int bd99954_irq_init(struct bd99954_device_info *di,
	struct device_node *np)
{
	int ret;

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		return ret;
	}

	ret = gpio_request(di->gpio_int, "charger_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		return ret;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		return ret;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto irq_init_err;
	}

	ret = request_irq(di->irq_int, bd99954_interrupt,
		IRQF_TRIGGER_FALLING, "charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto irq_init_err;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;
	return 0;
irq_init_err:
	gpio_free(di->gpio_int);
	return ret;
}

static void bd99954_init_charge_para(struct bd99954_device_info *di)
{
	di->chg_done = FALSE;
	di->term_curr = ITERM_DEFAULT;
	di->iterm_pre = ITERM_DEFAULT;
}

static int bd99954_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bd99954_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");
	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bd99954_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bd99954_irq_work);
	ret = init_charge_reg();
	if (ret < 0) {
		hwlog_info("init_charge_reg fail\n");
		return ret;
	}

	ret = bd99954_gpio_init(di, np);
	if (ret) {
		hwlog_err("bd99954_gpio_init failed\n");
		return ret;
	}

	ret = bd99954_irq_init(di, np);
	if (ret) {
		hwlog_err("bd99954_irq_init failed\n");
		goto gpio_fail;
	}

	bd99954_sysfs_create_group(di->dev);
	ret = hw_battery_voltage_ops_register(&bd99954_vbatt_ops, "bq25882");
	if (ret)
		hwlog_err("bd99954 hw_battery_voltage ops register failed\n");

	ret = series_batt_ops_register(&bd99954_chg_ops);
	if (ret) {
		hwlog_err("bd99954 series_batt ops register fail\n");
		goto sysfs_fail;
	}

	ret = charger_otg_ops_register(&bd99954_otg_ops);
	if (ret) {
		hwlog_err("bd99954 charger otg ops register fail\n");
		goto sysfs_fail;
	}

	bd99954_init_charge_para(di);
	hwlog_info("probe end\n");
	return 0;

sysfs_fail:
	bd99954_sysfs_remove_group(di->dev);
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
gpio_fail:
	gpio_free(di->gpio_otg);
	return ret;
}

static int bd99954_remove(struct i2c_client *client)
{
	struct bd99954_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_otg)
		gpio_free(di->gpio_otg);

	bd99954_sysfs_remove_group(di->dev);
	g_bd99954_dev = NULL;
	return 0;
}

static void bd99954_shutdown(struct i2c_client *client)
{
	bd99954_write_word(BD99954_REG_SMBREG, BD99954_REG_SMBREG_5H);
}

MODULE_DEVICE_TABLE(i2c, bd99954);
static const struct of_device_id bd99954_of_match[] = {
	{
		.compatible = "rohm,bd99954_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bd99954_i2c_id[] = {
	{ "bd99954_charger", 0 }, {}
};

static struct i2c_driver bd99954_driver = {
	.probe = bd99954_probe,
	.remove = bd99954_remove,
	.shutdown = bd99954_shutdown,
	.id_table = bd99954_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bd99954_charger",
		.of_match_table = of_match_ptr(bd99954_of_match),
	},
};

static int __init bd99954_init(void)
{
	return i2c_add_driver(&bd99954_driver);
}

static void __exit bd99954_exit(void)
{
	i2c_del_driver(&bd99954_driver);
}

module_init(bd99954_init);
module_exit(bd99954_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bd99954 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
