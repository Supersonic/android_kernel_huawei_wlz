/*
 * Device driver for regulators in Hisi IC
 *
 * Copyright (c) 2013 Linaro Ltd.
 * Copyright (c) 2011 Hisilicon.
 *
 * Guodong Xu <guodong.xu@linaro.org>
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
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/version.h>
#ifdef CONFIG_HISI_PMIC_DEBUG
#include <linux/debugfs.h>
#endif
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG HISI_SUB_PMIC_REGULATOR_TAG

#define BRAND_DEBUG(args...)	pr_debug(args)

struct hisi_sub_regulator_register_info {
	u32 chip_version;
	u32 version_1;
	u32 version_2;
	u32 ctrl_reg;
	u32 enable_mask;
	u32 vset_reg;
	u32 vset_mask;
};

struct hisi_sub_pmic_regulator {
	const char *name;
	struct hisi_sub_regulator_register_info register_info;
	struct timeval last_off_time;
	u32 off_on_delay;
	struct regulator_desc rdesc;
	int (*dt_parse)(struct hisi_sub_pmic_regulator *, struct spmi_device *,
		struct device_node *);
};
static DEFINE_MUTEX(enable_mutex);
struct timeval last_1_enabled;

static inline struct hisi_pmic *rdev_to_pmic(struct regulator_dev *dev)
{
	/* regulator_dev parent to->
	 * hisi regulator platform device_dev parent to->
	 * hisi pmic platform device_dev
	 */

	if ((rdev_get_dev(dev) == NULL) || (rdev_get_dev(dev)->parent == NULL))
		return NULL;

	return dev_get_drvdata(rdev_get_dev(dev)->parent->parent);
}

static void ensured_time_after(struct timeval since, u32 delay_us)
{
	struct timeval now;
	u64 elapsed_ns64, delay_ns64;
	u64 actual_us;

	delay_ns64 = delay_us * NSEC_PER_USEC;
	do_gettimeofday(&now);
	elapsed_ns64 = timeval_to_ns(&now) - timeval_to_ns(&since);
	if (delay_ns64 > elapsed_ns64) {
		actual_us =
			((delay_ns64 - elapsed_ns64) / NSEC_PER_USEC);
		if (actual_us >= 1000UL) {
			mdelay(actual_us / 1000UL);
			udelay(actual_us % 1000UL);
		} else if (actual_us > 0) {
			udelay(actual_us);
		}
	}
}

static int hisi_sub_pmic_regulator_is_enabled(struct regulator_dev *dev)
{
	u32 reg_val;
	struct hisi_sub_pmic_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);

	if (sreg == NULL)
		return 0;

	reg_val = hisi_sub_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x,enable_state=%d>\n", __func__,
		sreg->register_info.ctrl_reg,
		(reg_val & sreg->register_info.enable_mask));

	return ((reg_val & sreg->register_info.enable_mask) != 0);
}

static int hisi_sub_pmic_regulator_enable(struct regulator_dev *dev)
{
	struct hisi_sub_pmic_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val;

	if (sreg == NULL)
		return -EINVAL;
	ensured_time_after(sreg->last_off_time, sreg->off_on_delay);
	BRAND_DEBUG(
		"<[%s]: off_on_delay=%dus>\n", __func__, sreg->off_on_delay);
	mutex_lock(&enable_mutex);
	ensured_time_after(last_1_enabled, HISI_REGS_ENA_PROTECT_TIME);

	reg_val = hisi_sub_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x\n", __func__,
		sreg->register_info.ctrl_reg);
	/* set enable register */
	hisi_sub_pmic_write(pmic, sreg->register_info.ctrl_reg,
		(reg_val & (~sreg->register_info.enable_mask)) |
			sreg->register_info.enable_mask);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x,enable_mask=0x%x>\n", __func__,
		sreg->register_info.ctrl_reg, sreg->register_info.enable_mask);
	do_gettimeofday(&last_1_enabled);
	mutex_unlock(&enable_mutex);
	return 0;
}

static int hisi_sub_pmic_regulator_disable(struct regulator_dev *dev)
{
	struct hisi_sub_pmic_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val;

	if (sreg == NULL)
		return -EINVAL;
	reg_val = hisi_sub_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x\n", __func__,
		sreg->register_info.ctrl_reg);
	/* set enable register to 0 */
	hisi_sub_pmic_write(pmic, sreg->register_info.ctrl_reg,
		reg_val & (~sreg->register_info.enable_mask));
	BRAND_DEBUG("<[%s]: disable:ctrl_reg=0x%x,enable_mask=0x%x>\n",
		__func__, sreg->register_info.ctrl_reg,
		sreg->register_info.enable_mask);
	do_gettimeofday(&sreg->last_off_time);

	return 0;
}
static int hisi_sub_pmic_regulator_get_voltage(struct regulator_dev *dev)
{
	struct hisi_sub_pmic_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val, selector;

	if (sreg == NULL)
		return -EINVAL;

	/* get voltage selector */
	reg_val = hisi_sub_pmic_read(pmic, sreg->register_info.vset_reg);
	BRAND_DEBUG("<[%s]: vset_reg=0x%x>\n", __func__,
		sreg->register_info.vset_reg);

	selector = (reg_val & sreg->register_info.vset_mask) >>
		   ((unsigned int)ffs(sreg->register_info.vset_mask) - 1);

	return sreg->rdesc.ops->list_voltage(dev, selector);
}

static int hisi_sub_pmic_regulator_set_voltage(
	struct regulator_dev *dev, int min_uV, int max_uV, u32 *selector)
{
	struct hisi_sub_pmic_regulator *sreg = rdev_get_drvdata(dev);
	struct hisi_pmic *pmic = rdev_to_pmic(dev);
	u32 vsel;
	u32 reg_val, bits;
	int ret = 0;
	int uV = 0;

	if (sreg == NULL)
		return -EINVAL;

	for (vsel = 0; vsel < sreg->rdesc.n_voltages; vsel++) {
		uV = sreg->rdesc.volt_table[vsel];
		/* Break at the first in-range value */
		if (min_uV <= uV && uV <= max_uV)
			break;
	}
	/* unlikely to happen. sanity test done by regulator core */
	if (unlikely(vsel == sreg->rdesc.n_voltages))
		return -EINVAL;

	*selector = vsel;
	/* set voltage selector */
	reg_val = hisi_sub_pmic_read(pmic, sreg->register_info.vset_reg);
	BRAND_DEBUG("<[%s]: vset_reg=0x%x>\n", __func__,
		sreg->register_info.vset_reg);
	bits = vsel << ((unsigned int)ffs(sreg->register_info.vset_mask) - 1);
	hisi_sub_pmic_write(pmic, sreg->register_info.vset_reg,
		((sreg->register_info.vset_mask & bits) |
			(reg_val & ~sreg->register_info.vset_mask)));
	BRAND_DEBUG("<[%s]: vset_reg=0x%x, vset_mask=0x%x, value=0x%x>\n",
		__func__, sreg->register_info.vset_reg,
		sreg->register_info.vset_mask, bits);

	return ret;
}

static int hisi_sub_pmic_regulator_dt_common(
	struct hisi_sub_pmic_regulator *sreg, struct spmi_device *pdev,
	struct device_node *np)
{
	struct device *dev = &pdev->dev;
	struct regulator_desc *rdesc = &sreg->rdesc;
	unsigned int register_info[3] = {0};
	int ret;

	/* parse .register_info.ctrl_reg */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-ctrl",
						register_info, 2);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-ctrl property set\n");
		return ret;
	}
	sreg->register_info.ctrl_reg = register_info[0];
	sreg->register_info.enable_mask = register_info[1];

	/* parse .register_info.vset_reg */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-vset",
						register_info, 2);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-vset property set\n");
		return ret;
	}
	sreg->register_info.vset_reg = register_info[0];
	sreg->register_info.vset_mask = register_info[1];

	/* parse .register_info.chip_version for  distinguishing chip version */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-select",
						register_info, 3);
	if (!ret) {
		sreg->register_info.chip_version = register_info[0];
		sreg->register_info.version_1 = register_info[1];
		sreg->register_info.version_2 = register_info[2];
		if (hisi_sub_pmic_reg_read(sreg->register_info.chip_version) != sreg->register_info.version_1) {
			if (hisi_sub_pmic_reg_read(sreg->register_info.chip_version) != sreg->register_info.version_2) {
				ret = -ENOMEM;
				dev_err(dev, "hisilicon,hisi-select property not Match with version\n");
				return ret;
			} else {
				/* sub pmu is hi6422v500 */
				ret = of_property_read_u32_array(np, "hisilicon,hisi-ctrl-2",
						register_info, 2);
				if (ret) {
					dev_err(dev, "no hisilicon,hisi-ctrl-2 property set\n");
					return ret;
				}
				sreg->register_info.ctrl_reg = register_info[0];
				sreg->register_info.enable_mask = register_info[1];
				ret = of_property_read_u32_array(np, "hisilicon,hisi-vset-2",
						register_info, 2);
				if (ret) {
					dev_err(dev, "no hisilicon,hisi-vset-2 property set\n");
					return ret;
				}
				sreg->register_info.vset_reg = register_info[0];
				sreg->register_info.vset_mask = register_info[1];
					}
		} else {
			/* sub pmu is hi6422v300 */
			ret = of_property_read_u32_array(np, "hisilicon,hisi-ctrl-1",
						register_info, 2);
			if (ret) {
				dev_err(dev, "no hisilicon,hisi-ctrl-1 property set\n");
				return ret;
			}
			sreg->register_info.ctrl_reg = register_info[0];
			sreg->register_info.enable_mask = register_info[1];
			ret = of_property_read_u32_array(np, "hisilicon,hisi-vset-1",
						register_info, 2);
			if (ret) {
				dev_err(dev, "no hisilicon,hisi-vset-1 property set\n");
				return ret;
			}
			sreg->register_info.vset_reg = register_info[0];
			sreg->register_info.vset_mask = register_info[1];
		}
	}

	ret = of_property_read_u32(
		np, "hisilicon,hisi-off-on-delay-us", &sreg->off_on_delay);
	if (ret) {
		dev_err(dev,
			"no hisilicon,hisi-off-on-delay-us property set\n");
		return ret;
	}

	/* parse .enable_time */
	ret = of_property_read_u32(
		np, "hisilicon,hisi-enable-time-us", &rdesc->enable_time);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-enable-time-us property set\n");
		return ret;
	}
	return ret;
}

static int hisi_sub_pmic_regulator_dt_parse(
	struct hisi_sub_pmic_regulator *sreg, struct spmi_device *pdev,
	struct device_node *np)
{
	struct device *dev = &pdev->dev;
	struct regulator_desc *rdesc = &sreg->rdesc;
	unsigned int *v_table = NULL;
	int ret = 0;

	if (!np) {
		dev_err(dev, "no hisilicon,no child root\n");
		ret = -ENOMEM;
		return ret;
	}
	ret = of_property_read_u32(
		np, "hisilicon,hisi-n-voltages", &rdesc->n_voltages);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-n-voltages property set\n");
		return ret;
	}

	/* alloc space for .volt_table */
	v_table = devm_kzalloc(
		dev, sizeof(unsigned int) * rdesc->n_voltages, GFP_KERNEL);
	if (!v_table) {
		ret = -ENOMEM;
		dev_err(dev, "no memory for .volt_table\n");
		return ret;
	}

	ret = of_property_read_u32_array(
		np, "hisilicon,hisi-vset-table", v_table, rdesc->n_voltages);
	if (ret) {
		dev_err(dev, "no hisilicon,hisi-vset-table property set\n");
		devm_kfree(dev, v_table);
		return ret;
	}
	rdesc->volt_table = v_table;

	/* parse hisi regulator's dt common part */
	ret = hisi_sub_pmic_regulator_dt_common(sreg, pdev, np);
	if (ret) {
		dev_err(dev, "failure in hisi_sub_pmic_regulator_dt_common\n");
		devm_kfree(dev, v_table);
		rdesc->volt_table = NULL;
		return ret;
	}
	return ret;
}

static struct regulator_ops hisi_regulator_sub_pmic_rops = {
	.is_enabled = hisi_sub_pmic_regulator_is_enabled,
	.enable = hisi_sub_pmic_regulator_enable,
	.disable = hisi_sub_pmic_regulator_disable,
	.list_voltage = regulator_list_voltage_table,
	.get_voltage = hisi_sub_pmic_regulator_get_voltage,
	.set_voltage = hisi_sub_pmic_regulator_set_voltage,
	.get_current_limit = NULL,
	.set_current_limit = NULL,
	.get_mode = NULL,
	.set_mode = NULL,
};
static const struct hisi_sub_pmic_regulator hisi_sub_pmic_regulator = {
	.rdesc = {
			.ops = &hisi_regulator_sub_pmic_rops,
			.type = REGULATOR_VOLTAGE,
			.owner = THIS_MODULE,
		},
	.dt_parse = hisi_sub_pmic_regulator_dt_parse,
};

static const struct of_device_id of_hisi_sub_regulator_match_tbl[] = {
	{
		.compatible = "hisilicon-hisi-sub-pmic-ldo",
		.data = &hisi_sub_pmic_regulator,
	},
	{} };

static int fake_of_get_regulator_constraint(
	struct regulation_constraints *constraints, struct device_node *np)
{
	const __be32 *min_uV = NULL;
	const __be32 *max_uV = NULL;
	unsigned int *valid_ops_mask = NULL;
	unsigned int *valid_modes_mask = NULL;

	if ((!np) || (!constraints))
		return -1;
	(constraints)->name = of_get_property(np, "regulator-name", NULL);

	min_uV = of_get_property(np, "regulator-min-microvolt", NULL);
	if (min_uV) {
		(constraints)->min_uV = be32_to_cpu(*min_uV);
		(constraints)->min_uA = be32_to_cpu(*min_uV);
	}

	max_uV = of_get_property(np, "regulator-max-microvolt", NULL);
	if (max_uV) {
		(constraints)->max_uV = be32_to_cpu(*max_uV);
		(constraints)->max_uA = be32_to_cpu(*max_uV);
	}
	valid_ops_mask = (unsigned int *)of_get_property(
		np, "hisilicon,valid-ops-mask", NULL);
	if (valid_ops_mask)
		(constraints)->valid_ops_mask = be32_to_cpu(*valid_ops_mask);
	valid_modes_mask = (unsigned int *)of_get_property(
		np, "hisilicon,valid-modes-mask", NULL);
	if (valid_modes_mask) {
		(constraints)->valid_modes_mask =
			be32_to_cpu(*valid_modes_mask);
	}
	return 0;
}
static int hisi_sub_regulator_probe(struct spmi_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_dev *rdev = NULL;
	struct regulator_desc *rdesc = NULL;
	struct hisi_sub_pmic_regulator *sreg = NULL;
	struct regulator_init_data *initdata = NULL;
	const struct of_device_id *match = NULL;
	const char *supplyname = NULL;
	const struct hisi_sub_pmic_regulator *template = NULL;
	struct regulator_config config = {};

	match = of_match_device(of_hisi_sub_regulator_match_tbl, &pdev->dev);
	if (match == NULL) {
		pr_err("get hisi regulator fail!\n\r");
		return -EINVAL;
	}

	template = (struct hisi_sub_pmic_regulator *)(match->data);
	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (initdata == NULL) {
		pr_err("[%s]:get regulator init data error !\n", __func__);
		return -EINVAL;
	}

	ret = fake_of_get_regulator_constraint(&initdata->constraints, np);
	if (!!ret) {
		pr_err("[%s]:get regulator constraint error !\n", __func__);
		return -EINVAL;
	}

	/* TODO:scharger regulator supports two modes */
	if (!template) {
		pr_err("[%s]no hisilicon-hisi-sub-pmic-ldo template data node.\n", __func__);
		return -ENODEV;
	}
	sreg = kmemdup(template, sizeof(*sreg), GFP_KERNEL);
	if (!sreg)
		return -ENOMEM;

	sreg->name = initdata->constraints.name;
	rdesc = &sreg->rdesc;
	rdesc->name = sreg->name;
	rdesc->min_uV = initdata->constraints.min_uV;
	supplyname = of_get_property(np, "hisilicon,supply_name", NULL);
	if (supplyname != NULL)
		initdata->supply_regulator = supplyname;
	/* to parse device tree data for regulator specific */
	ret = sreg->dt_parse(sreg, pdev, np);
	if (ret) {
		dev_err(dev, "device tree parameter parse error!\n");
		goto hisi_sub_regulator_end;
	}
	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = sreg;
	config.of_node = pdev->dev.of_node;

	/* register regulator */
	rdev = regulator_register(rdesc, &config);
	if (IS_ERR(rdev)) {
		pr_err("[%s]:regulator failed to register %s\n", __func__,
			rdesc->name);
		ret = PTR_ERR(rdev);
		/*goto hisi_charger_probe_end;*/
		goto hisi_sub_regulator_end;
	}
	dev_set_drvdata(dev, rdev);
	return ret;
hisi_sub_regulator_end:
		kfree(sreg);
	return ret;
}

static int hisi_sub_regulator_remove(struct spmi_device *pdev)
{
	struct regulator_dev *rdev = dev_get_drvdata(&pdev->dev);
	struct hisi_sub_pmic_regulator *sreg = rdev_get_drvdata(rdev);

	if (sreg == NULL)
		return 0;

	regulator_unregister(rdev);

	/* TODO: should i worry about that? devm_kzalloc */
	if (sreg->rdesc.volt_table)
		devm_kfree(&pdev->dev, (unsigned int *)sreg->rdesc.volt_table);

	kfree(sreg);
	return 0;
}
static int hisi_sub_regulator_suspend(struct device *dev, pm_message_t state)
{
	struct hisi_sub_pmic_regulator *hisi_regulator = dev_get_drvdata(dev);

	if (hisi_regulator == NULL) {
		pr_err("%s:regulator is NULL\n", __func__);
		return -ENOMEM;
	}

	pr_info("%s:+\n", __func__);
	pr_info("%s:-\n", __func__);

	return 0;
}

static int hisi_sub_regulator_resume(struct device *dev)
{
	struct hisi_sub_pmic_regulator *hisi_regulator = dev_get_drvdata(dev);

	if (hisi_regulator == NULL) {
		pr_err("%s:regulator is NULL\n", __func__);
		return -ENOMEM;
	}

	pr_info("%s:+\n", __func__);
	pr_info("%s:-\n", __func__);

	return 0;
}

static const struct spmi_device_id regulator_spmi_id[] = {
	{"hisilicon-hisi-sub-pmic-ldo", 0},
	{} };

MODULE_DEVICE_TABLE(spmi, pmic_spmi_id);
static struct spmi_driver hisi_sub_regulator_driver = {
	.driver = {
			.name = "hisi-sub-pmic",
			.owner = THIS_MODULE,
			.of_match_table = of_hisi_sub_regulator_match_tbl,
			.suspend = hisi_sub_regulator_suspend,
			.resume = hisi_sub_regulator_resume,
		},
	.id_table = regulator_spmi_id,
	.probe = hisi_sub_regulator_probe,
	.remove = hisi_sub_regulator_remove,
};

static int __init hisi_sub_regulator_init(void)
{
	return spmi_driver_register(&hisi_sub_regulator_driver);
}

static void __exit hisi_sub_regulator_exit(void)
{
	spmi_driver_unregister(&hisi_sub_regulator_driver);
}

fs_initcall(hisi_sub_regulator_init);
module_exit(hisi_sub_regulator_exit);


MODULE_DESCRIPTION("Hisi sub regulator driver");
MODULE_LICENSE("GPL v2");
