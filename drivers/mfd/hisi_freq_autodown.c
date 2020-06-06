/*
 * hisi_hisi_powerkey.c - Hisilicon BUS automatic frequency reduction.
 *
 * Copyright (c) 2013 Linaro Ltd.
 * Copyright (c) 2011 Hisilicon.
 *
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
#include "hisi_freq_autodown.h"
#include <securec.h>
static FREQ_AUTODOWN_DESC *g_freq_autodown;

static unsigned int  hisi_crgperiph_regu_read(unsigned int reg)
{
	unsigned int read_value;
	if (NULL == g_freq_autodown->crgperi_base_addr)
		return -1;/*lint !e570*/
	read_value = readl(g_freq_autodown->crgperi_base_addr + reg);

	return read_value;
}/*lint !e715*/
static void hisi_crgperiph_regu_write(unsigned int val, unsigned int reg)
{
	if (NULL == g_freq_autodown->crgperi_base_addr)
		return ;
	writel(val, g_freq_autodown->crgperi_base_addr + reg);
    return ;
}/*lint !e715*/
static unsigned int  hisi_mediacrg_regu_read(unsigned int reg)
{
	unsigned int read_value;
	if (NULL == g_freq_autodown->media_crg_base_addr)
		return -1;/*lint !e570*/
	read_value = readl(g_freq_autodown->media_crg_base_addr + reg);

	return read_value;
}/*lint !e715*/
static void hisi_mediacrg_regu_write(unsigned int val, unsigned int reg)
{
	if (NULL == g_freq_autodown->media_crg_base_addr)
		return ;
	writel(val, g_freq_autodown->media_crg_base_addr + reg);
    return ;
}/*lint !e715*/
static unsigned int  hisi_media2crg_regu_read(unsigned int reg)
{
	unsigned int read_value;
	if (NULL == g_freq_autodown->media2_crg_base_addr)
		return -1;/*lint !e570*/
	read_value = readl(g_freq_autodown->media2_crg_base_addr + reg);
	return read_value;
}/*lint !e715*/
static void hisi_media2crg_regu_write(unsigned int val, unsigned int reg)
{
	if (NULL == g_freq_autodown->media2_crg_base_addr)
		return ;
	writel(val, g_freq_autodown->media2_crg_base_addr + reg);
    return ;
}/*lint !e715*/
int hisi_freq_autodown_clk_set(const char *freq_name, u32 control_flag)
{
	u32 bypass_reg_value = 0;
	unsigned int index = 0;
	unsigned int bypass_reg = 0, bypass_bit = 0, bypass_addr_flag = 0;
	unsigned int clock_reg = 0, clock_bit = 0, clock_addr_flag = 0;
	if (NULL == freq_name) {
		return -EINVAL;
	}
	if (NULL == g_freq_autodown) {
		return -ENODEV;
	}
	if (OPEN == g_freq_autodown->freq_autodown_state) {
		AUTOFS_DEBUG("freq_name = %s, control state = %d\n", freq_name, control_flag);
		spin_lock(&g_freq_autodown->lock);
		for (index = 0; index < g_freq_autodown->freq_autodown_num; index++) {
			if (!strcmp(freq_name, g_freq_autodown->freq_autodown_name[index])) {/*lint !e421*/
				bypass_reg = g_freq_autodown->freq_info[index].disable_bypass_reg;
				bypass_bit = g_freq_autodown->freq_info[index].disable_bypass_bit;
				bypass_addr_flag = g_freq_autodown->freq_info[index].disable_bypass_addr_flag;
				clock_reg = g_freq_autodown->freq_info[index].enable_clock_reg;
				clock_bit = g_freq_autodown->freq_info[index].enable_clock_bit;
				clock_addr_flag = g_freq_autodown->freq_info[index].enable_clock_addr_flag;
				AUTOFS_DEBUG("name[%s]:Info:bypass_reg=0x%x,bypass_bit=0x%x,flag0 =%d,clock_reg=0x%x,clock_bit=0x%x,flag1=%d\n",\
									freq_name, bypass_reg, bypass_bit, bypass_addr_flag, clock_reg, clock_bit, clock_addr_flag);
				if (ENABLE == control_flag) {
					if (INVALID_REG_ADDR != clock_reg) {/*for "vcodecbus","vdecbus","vencbus" etc.*/
						/*disable bypass*/
						if (MEDIA_CRG_BASE_ADDR == bypass_addr_flag) {
							AUTOFS_DEBUG("disable bypass addr flag = 0x%x\n", bypass_addr_flag);
							bypass_reg_value = hisi_mediacrg_regu_read(bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
							bypass_reg_value &= (~BIT(bypass_bit));
							hisi_mediacrg_regu_write(bypass_reg_value, bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_mediacrg_regu_read(bypass_reg));
						} else if (CRG_PERI_BASE_ADDR == bypass_addr_flag) {
							AUTOFS_DEBUG("disable bypass addr flag = 0x%x\n", bypass_addr_flag);
							bypass_reg_value = hisi_crgperiph_regu_read(bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
							bypass_reg_value &= (~BIT(bypass_bit));
							hisi_crgperiph_regu_write(bypass_reg_value, bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_crgperiph_regu_read(bypass_reg));
						} else if (MEDIA2_CRG_BASE_ADDR == bypass_addr_flag) {
							AUTOFS_DEBUG("disable bypass addr flag = 0x%x\n", bypass_addr_flag);
							bypass_reg_value = hisi_media2crg_regu_read(bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
							bypass_reg_value &= (~BIT(bypass_bit));
							hisi_media2crg_regu_write(bypass_reg_value, bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_media2crg_regu_read(bypass_reg));
						}
						/*enable clock*/
						if (MEDIA_CRG_BASE_ADDR == clock_addr_flag) {
							AUTOFS_DEBUG("enable clock addr flag = 0x%x\n", clock_addr_flag);
							hisi_mediacrg_regu_write(BIT(clock_bit), clock_reg);/*lint !e747*/
							AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",\
									BIT(clock_bit), hisi_mediacrg_regu_read(clock_reg + MEDIA_CRG_ADDR_STATE));
						} else if (CRG_PERI_BASE_ADDR == clock_addr_flag) {
							AUTOFS_DEBUG("enable clock addr flag = 0x%x\n", clock_addr_flag);
							hisi_crgperiph_regu_write(BIT(clock_bit), clock_reg);/*lint !e747*/
							AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",\
									BIT(clock_bit), hisi_crgperiph_regu_read(clock_reg + CRGPERI_ADDR_STATE));
						} else if (MEDIA2_CRG_BASE_ADDR == clock_addr_flag) {
							AUTOFS_DEBUG("enable clock addr flag = 0x%x\n", clock_addr_flag);
							hisi_media2crg_regu_write(BIT(clock_bit), clock_reg);/*lint !e747*/
							AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",\
									BIT(clock_bit), hisi_media2crg_regu_read(clock_reg + MEDIA2_CRG_ADDR_STATE));
						}
					} else {/*for "emmc1bus","emmc0bus","dbgbus","dmabus","cfgbus","sysbus" etc.*/
						/*disable bypass*/
						AUTOFS_DEBUG("reg[0x%lx], bypass_reg_value before = 0x%x\n",\
									(BIT(bypass_bit) | BITMSK(bypass_bit)), hisi_mediacrg_regu_read(bypass_reg));
						hisi_mediacrg_regu_write(BITMSK(bypass_bit), bypass_reg);/*lint !e747*/
						AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_mediacrg_regu_read(bypass_reg));
					}
				} else if (DISABLE == control_flag) {
					if (INVALID_REG_ADDR != clock_reg) {
						/*disable clock*/
						if (MEDIA_CRG_BASE_ADDR == clock_addr_flag) {
							AUTOFS_DEBUG("disable clock addr flag = 0x%x\n", clock_addr_flag);
							hisi_mediacrg_regu_write(BIT(clock_bit), clock_reg + MEDIA_CRG_ADDR_OFFSET);/*lint !e747*/
							AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",\
									BIT(clock_bit), hisi_mediacrg_regu_read(clock_reg + MEDIA_CRG_ADDR_STATE));
						} else if (CRG_PERI_BASE_ADDR == clock_addr_flag) {
							AUTOFS_DEBUG("disable clock addr flag = 0x%x\n", clock_addr_flag);
							hisi_crgperiph_regu_write(BIT(clock_bit), clock_reg + CRGPERI_ADDR_OFFSET);/*lint !e747*/
							AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",\
									BIT(clock_bit), hisi_crgperiph_regu_read(clock_reg + CRGPERI_ADDR_STATE));
						} else if (MEDIA2_CRG_BASE_ADDR == clock_addr_flag) {
							AUTOFS_DEBUG("disable clock addr flag = 0x%x\n", clock_addr_flag);
							hisi_media2crg_regu_write(BIT(clock_bit), clock_reg + MEDIA2_CRG_ADDR_OFFSET);/*lint !e747*/
							AUTOFS_DEBUG("clock_bit[0x%lx] clk_reg_value end = 0x%x\n",\
									BIT(clock_bit), hisi_media2crg_regu_read(clock_reg + MEDIA2_CRG_ADDR_STATE));
						}
						/*enable bypass*/
						if (MEDIA_CRG_BASE_ADDR == bypass_addr_flag) {
							AUTOFS_DEBUG("enable bypass addr flag = 0x%x\n", bypass_addr_flag);
							bypass_reg_value = hisi_mediacrg_regu_read(bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
							bypass_reg_value |= BIT(bypass_bit);/*lint !e712*/
							hisi_mediacrg_regu_write(bypass_reg_value, bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_mediacrg_regu_read(bypass_reg));
						} else if (CRG_PERI_BASE_ADDR == bypass_addr_flag) {
							AUTOFS_DEBUG("enable bypass addr flag = 0x%x\n", bypass_addr_flag);
							bypass_reg_value = hisi_crgperiph_regu_read(bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
							bypass_reg_value |= BIT(bypass_bit);/*lint !e712*/
							hisi_crgperiph_regu_write(bypass_reg_value, bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_crgperiph_regu_read(bypass_reg));
						} else if (MEDIA2_CRG_BASE_ADDR == bypass_addr_flag) {
							AUTOFS_DEBUG("enable bypass addr flag = 0x%x\n", bypass_addr_flag);
							bypass_reg_value = hisi_media2crg_regu_read(bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value before = 0x%x\n", bypass_reg_value);
							bypass_reg_value |= BIT(bypass_bit);/*lint !e712*/
							hisi_media2crg_regu_write(bypass_reg_value, bypass_reg);
							AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_media2crg_regu_read(bypass_reg));
						}
					} else {
						/*enable bypass*/
						AUTOFS_DEBUG("reg[0x%lx], bypass_reg_value before = 0x%x\n",\
								BITMSK(bypass_bit), hisi_mediacrg_regu_read(bypass_reg));
						hisi_mediacrg_regu_write((BIT(bypass_bit) | BITMSK(bypass_bit)), bypass_reg);/*lint !e747*/
						AUTOFS_DEBUG("bypass_reg_value end = 0x%x\n", hisi_mediacrg_regu_read(bypass_reg));
					}
				} else {
				/*not control*/
				}
			}
		}
		spin_unlock(&g_freq_autodown->lock);
	}
	return 0;
}
EXPORT_SYMBOL(hisi_freq_autodown_clk_set);

static int hisi_freq_autodown_initial(struct platform_device *pdev, FREQ_AUTODOWN_DESC *freq_autodown)
{
	struct device_node *root = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	unsigned int register_info[3] = {0};
	unsigned int index, bit;
	char compatible_string[FREQ_AUTODOWN_DTS_ATTR_LEN] = {0};
	s32 ret = 0;
	AUTOFS_DEBUG("Begin...\n");
	/*get freq autodown state*/
	ret = of_property_read_u32(np, "hisilicon,freq-autodown-state", &freq_autodown->freq_autodown_state);
	if (ret) {
		dev_err(dev, "[%s]get hisilicon,freq-autodown-state failed.\n", __func__);
		return -ENODEV;
	}
	AUTOFS_DEBUG("hisilicon,freq-autodown-state = %d\n", freq_autodown->freq_autodown_state);
	/*get freq autodown num*/
	ret = of_property_read_u32(np, "hisilicon,freq-autodown-num", &freq_autodown->freq_autodown_num);
	if (ret) {
	    dev_err(dev, "[%s]get hisilicon,freq-autodown-num failed.\n", __func__);
	    return -ENODEV;
	}
	AUTOFS_DEBUG("hisilicon,freq-autodown-num = %d\n", freq_autodown->freq_autodown_num);
	/*malloc freq autodown struct*/
	freq_autodown->freq_autodown_name = (char **)devm_kzalloc(dev, freq_autodown->freq_autodown_num*sizeof(char *), GFP_KERNEL);
	if (NULL == freq_autodown->freq_autodown_name) {
		dev_err(dev, "[%s]devm_kzalloc freq_autodown_name error.\n", __func__);
		return -ENOMEM;
	}

	freq_autodown->freq_info = (struct freq_autodown_info *)devm_kzalloc(dev, freq_autodown->freq_autodown_num*sizeof(struct freq_autodown_info), GFP_KERNEL);
	if (NULL == freq_autodown->freq_info) {
		dev_err(dev, "[%s]devm_kzalloc freq_autodown_name error.\n", __func__);
		return -ENOMEM;
	}
	/*get freq autodown name*/
	for (bit = 0; bit < freq_autodown->freq_autodown_num; bit++) {
		ret = of_property_read_string_index(np, "hisilicon,freq-autodown-names",
				bit, (const char **)&freq_autodown->freq_autodown_name[bit]);
		if (ret != 0) {
			pr_err("failed to get freq autodown names! ret=%d.\n", ret);
			return -EINVAL;
		}
		AUTOFS_DEBUG("hisilicon,freq-autodown-names[%d] = %s\n", bit, freq_autodown->freq_autodown_name[bit]);
	}
	/*get freq autodown info*/
	for (index = 0; index < freq_autodown->freq_autodown_num; index++) {
		ret = snprintf_s(compatible_string, FREQ_AUTODOWN_DTS_ATTR_LEN, FREQ_AUTODOWN_DTS_ATTR_LEN - 1,\
				"hisilicon,hisi-freq-autodown-%s", freq_autodown->freq_autodown_name[index]);
		if(ret == -1)
			pr_err("%s snprintf_s failed!\n", __func__);
		root = of_find_compatible_node(np, NULL, compatible_string);
		if (!root) {
			dev_err(dev, "[%s]no %s root node.\n", __func__, compatible_string);
			return -ENODEV;
		}
		/*get freq autodown disable bypass value*/
		ret = of_property_read_u32_array(root, "freq_autodown_disable_bypass",
				register_info, FREQ_AUTODOWN_DTS_REG_NUM);
		if (ret) {
			dev_err(dev, "no freq_autodown_disable_bypass property set\n");
			return -EINVAL;
		}
		freq_autodown->freq_info[index].disable_bypass_reg = register_info[0];
		freq_autodown->freq_info[index].disable_bypass_bit = register_info[1];
		freq_autodown->freq_info[index].disable_bypass_addr_flag = register_info[2];
		AUTOFS_DEBUG("index[%d] disable_bypass_reg = 0x%x\n", index, freq_autodown->freq_info[index].disable_bypass_reg);
		AUTOFS_DEBUG("index[%d] disable_bypass_bit = 0x%x\n", index, freq_autodown->freq_info[index].disable_bypass_bit);
		AUTOFS_DEBUG("index[%d] disable_bypass_addr_flag = %d\n", index, freq_autodown->freq_info[index].disable_bypass_addr_flag);
		/*get freq autodown enable clock value*/
		ret = of_property_read_u32_array(root, "freq_autodown_enable_clock",
				register_info, FREQ_AUTODOWN_DTS_REG_NUM);
		if (ret) {
			dev_err(dev, "no freq_autodown_enable_clock property set\n");
			return -EINVAL;
		}
		freq_autodown->freq_info[index].enable_clock_reg = register_info[0];
		freq_autodown->freq_info[index].enable_clock_bit = register_info[1];
		freq_autodown->freq_info[index].enable_clock_addr_flag = register_info[2];
		AUTOFS_DEBUG("index[%d] enable_clock_reg = 0x%x\n", index, freq_autodown->freq_info[index].enable_clock_reg);
		AUTOFS_DEBUG("index[%d] enable_clock_bit = 0x%x\n", index, freq_autodown->freq_info[index].enable_clock_bit);
		AUTOFS_DEBUG("index[%d] enable_clock_addr_flag = %d\n", index, freq_autodown->freq_info[index].enable_clock_addr_flag);
	}
	AUTOFS_DEBUG("End...\n");
	return 0;
}

static struct of_device_id of_hisi_freq_autodown_match_tbl[] = {
	{
		.compatible = "hisilicon,hisi-freq-autodown",
	},

	{ /* end */ }
};

static int hisi_freq_autodown_probe(struct platform_device *pdev)
{
	struct device *dev = NULL;
	FREQ_AUTODOWN_DESC *freq_autodown = NULL;
	struct device_node *np = NULL;
	int ret = 0;

	if (pdev == NULL) {
		pr_err("[%s] get  platform device para is err!\n", __func__);
		return -EINVAL;
	}

	dev = &pdev->dev;
	if (NULL == dev) {/*lint !e774*/
		pr_err("[%s] get dev is err!\n", __func__);
		return -EINVAL;
	}
	np = dev->of_node;
	if (NULL == np) {
		pr_err("[%s] get np is err!\n", __func__);
		return -EINVAL;
	}

	freq_autodown = (FREQ_AUTODOWN_DESC *)devm_kzalloc(dev, sizeof(*freq_autodown), GFP_KERNEL);
	if (NULL == freq_autodown) {
		dev_err(dev, "%s devm_kzalloc freq_autodown err\n", __func__);
		return -ENOMEM;
	}

	/*get freq autodown num*/
	ret = of_property_read_u32(np, "hisilicon,freq-autodown-baseaddress-num", &freq_autodown->freq_autodown_baseaddress_num);
	if (ret) {
	    dev_err(dev, "[%s]get hisilicon,freq-autodown-baseaddress-num failed.\n", __func__);
	    return -ENODEV;/*lint !e429*/
	}

	if (FREQ_AUTODOWN_BASEADDR_NUM == freq_autodown->freq_autodown_baseaddress_num) {
		/* get resources */
		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!freq_autodown->res) {
			dev_err(dev, "platform_get_resource err !\n");
			return -ENOENT;/*lint !e429*/
		}

		freq_autodown->media_crg_base_addr = ioremap(freq_autodown->res->start, resource_size(freq_autodown->res));
		if (!freq_autodown->media_crg_base_addr) {
			dev_err(dev, "cannot remap media_crg address memory\n");
			return -ENOMEM;/*lint !e429*/
		}

		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (!freq_autodown->res) {
			dev_err(dev, "platform_get_resource err !\n");
			return -ENOENT;/*lint !e429*/
		}

		freq_autodown->crgperi_base_addr = ioremap(freq_autodown->res->start, resource_size(freq_autodown->res));
		if (!freq_autodown->crgperi_base_addr) {
			dev_err(dev, "cannot remap crgperi address memory\n");
			return -ENOMEM;/*lint !e429*/
		}

		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
		if (!freq_autodown->res) {
			dev_err(dev, "platform_get_resource err !\n");
			return -ENOENT;/*lint !e429*/
		}

		freq_autodown->media2_crg_base_addr = ioremap(freq_autodown->res->start, resource_size(freq_autodown->res));
		if (!freq_autodown->media2_crg_base_addr) {
			dev_err(dev, "cannot remap media2_crg address memory\n");
			return -ENOMEM;/*lint !e429*/
		}

	} else {
		/* get resources */
		freq_autodown->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!freq_autodown->res) {
			dev_err(dev, "platform_get_resource err !\n");
			return -ENOENT;/*lint !e429*/
		}

		freq_autodown->media_crg_base_addr = ioremap(freq_autodown->res->start, resource_size(freq_autodown->res));
		if (!freq_autodown->media_crg_base_addr) {
			dev_err(dev, "cannot map register memory\n");
			return -ENOMEM;/*lint !e429*/
		}
	}

	freq_autodown->dev = &(pdev->dev);

	ret = hisi_freq_autodown_initial(pdev, freq_autodown);
	if (ret) {
		dev_err(dev, "%s of_get_freq_autodown_attr err!\n", __func__);
		goto freq_autodown_fail;
	}
	spin_lock_init(&freq_autodown->lock);
	g_freq_autodown = freq_autodown;
	platform_set_drvdata(pdev, freq_autodown);

	return ret;

freq_autodown_fail:
	if (FREQ_AUTODOWN_BASEADDR_NUM == freq_autodown->freq_autodown_baseaddress_num) {
		iounmap(freq_autodown->media_crg_base_addr);
		iounmap(freq_autodown->crgperi_base_addr);
		iounmap(freq_autodown->media2_crg_base_addr);
	} else {
		iounmap(freq_autodown->media_crg_base_addr);
	}
	return ret;
}

static int hisi_freq_autodown_remove(struct platform_device *pdev)
{
	FREQ_AUTODOWN_DESC *freq_autodown = platform_get_drvdata(pdev);

	if (FREQ_AUTODOWN_BASEADDR_NUM == freq_autodown->freq_autodown_baseaddress_num) {
		iounmap(freq_autodown->media_crg_base_addr);
		iounmap(freq_autodown->crgperi_base_addr);
		iounmap(freq_autodown->media2_crg_base_addr);
	} else {
		iounmap(freq_autodown->media_crg_base_addr);
	}

	return 0;
}

#ifdef CONFIG_PM
static int hisi_freq_autodown_suspend(struct platform_device *pdev, pm_message_t pm)
{
	return 0;
}

static int hisi_freq_autodown_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

static struct platform_driver hisi_freq_autodown_driver = {
	.driver = {
		.name = "hisi,freq-autodown",
		.owner = THIS_MODULE,
		.of_match_table = of_hisi_freq_autodown_match_tbl,
	},
	.probe  = hisi_freq_autodown_probe,
	.remove = hisi_freq_autodown_remove,
#ifdef CONFIG_PM
	.suspend    = hisi_freq_autodown_suspend,
	.resume     = hisi_freq_autodown_resume,
#endif
};

static int __init hisi_freq_autodown_init(void)
{
	int ret = 0;
	AUTOFS_DEBUG("\n================[%s begin ]=============\n", __func__);

	ret = platform_driver_register(&hisi_freq_autodown_driver);
	if (ret) {
		pr_err("%s: platform_driver_register failed!\n", __func__);
	}

	return ret;
}

static void __exit hisi_freq_autodown_exit(void)
{
	platform_driver_unregister(&hisi_freq_autodown_driver);
}

subsys_initcall(hisi_freq_autodown_init);
module_exit(hisi_freq_autodown_exit);

MODULE_DESCRIPTION("Hisi freq autodown driver");
MODULE_LICENSE("GPL V2");
