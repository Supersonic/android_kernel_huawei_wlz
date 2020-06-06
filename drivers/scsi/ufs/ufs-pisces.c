/*
 * Copyright (c) 2019, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "ufshcd :" fmt
#include "dsm_ufs.h"
#include "ufs-hisi.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include <linux/hisi/hisi_idle_sleep.h>
#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>
#include <soc_actrl_interface.h>
#include <soc_hsdt_crg_interface.h>
#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>
extern void ufs_hisi_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params);

void ufs_kirin_regulator_init(struct ufs_hba *hba)
{
}

void ufs_clk_init(struct ufs_hba *hba)
{
}

void ufs_soc_init(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;
	u32 reg;
	int retry;
	u32 value = 0;

	dev_info(hba->dev, "%s ++\n", __func__);

	/* 1. Set SD and DS to 0*/
	writel(0x00030000, SOC_ACTRL_AO_MEM_CTRL2_ADDR(host->actrl));

	/* 2. release reset of UFS subsystem CRG */
	sys_ctrl_set_bits(
		host, BIT(IP_RST_UFS_SUBSYS_CRG), SOC_SCTRL_SCPERRSTDIS1);
	retry = 2000;
	while (retry--) {
		value = readl(SOC_SCTRL_SCPERRSTSTAT1_ADDR(host->sysctrl));

		if (!(value & IP_RST_UFS_SUBSYS_CRG))
			break;
		udelay(1);
	}
	if (retry < 0)
		pr_err("UFS Sub-sys CRG reset failed\n");

	/* 3. enable the clk of UFS side of NOC async bridge */
	hsdt_crg_set_bits(
		host, BIT(GT_CLK_UFS_NOC_ASYNCBRG), SOC_HSDT_CRG_PEREN1);
	/* release the reset of UFS side of NOC async bridge */
	hsdt_crg_set_bits(
		host, BIT(IP_RST_UFS_NOC_ASYNCBRG), SOC_HSDT_CRG_PERRSTDIS0);

	/* 5. enable ip_rst_ufs_subsys, reset ufs sub-system*/
	sys_ctrl_set_bits(
		host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTEN1);
	/* 6. setup ufs sub-system clk to 224MHz*/
	writel(0x003F0006, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));

	/*10. Release MPHY related ISO */
	writel(0x00210000, SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));

	/*11. Enable clk_ufs_subsys clock */
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
		SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));

	/* 12. handshake of UFS powerdomain */
	/* setup PMC registers, exit ldle mode */
	writel(0x00020000, SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(host->pmctrl));
	/* Polling the PMC idle ack0 register */
	retry = 2000;
	while (retry--) {
		value = readl(
			SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(host->pmctrl));

		if (!(value & NOC_UFS_POWER_IDLEACK_0))
			break;
		udelay(1);
	}
	if (retry < 0)
		pr_err("UFS Power idle exit failed\n");

	/* Polling the PMC idle status */
	retry = 2000;
	while (retry--) {
		value = readl(SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(host->pmctrl));

		if (!(value & NOC_UFS_POWER_IDLE_0))
			break;
		udelay(1);
	}
	if (retry < 0)
		pr_err("UFS Power idle exit status err\n");

	/* 15. release ufs mast-reset */
	sys_ctrl_set_bits(
		host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTDIS1);
	/* release internal resets */
	if (ufshcd_is_hisi_ufs_hc(hba)) {
		writel(0x01FF01FF, SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(
					   host->ufs_sys_ctrl));
	}

	/* device reset */
	ufs_kirin_device_hw_reset(hba);

	writel(0x00010001, SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(host->sysctrl));
	mdelay(1);

	if (!ufshcd_is_hisi_ufs_hc(hba)) {
		writel(1 << SOC_UFS_Sysctrl_UFS_UMECTRL_ufs_ies_en_mask_START,
			SOC_UFS_Sysctrl_UFS_UMECTRL_ADDR(host->ufs_sys_ctrl));
		writel(1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
			       16) |
				0,
			SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

		writel(1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START +
			       16) |
		1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_arst_ufs_START,
			SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));

		writel(1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START + 16),
			SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
		udelay(100);
		writel(1 << (SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START +
			       16) |
		1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START,
			SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
		reg = readl(
			SOC_UFS_Sysctrl_CRG_UFS_CFG_ADDR(host->ufs_sys_ctrl));
		if (reg & (1 << SOC_UFS_Sysctrl_CRG_UFS_CFG_ip_rst_ufs_START))
			mdelay(1);
	}

	/*set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when init*/
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 1);
	dev_info(hba->dev, "%s --\n", __func__);
}

int ufs_kirin_suspend_before_set_link_state(
	struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct ufs_kirin_host *host = hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	/*step1:store BUSTHRTL register*/
	host->busthrtl_backup = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
	/*enable PowerGating*/
	ufshcd_rmwl(hba, LP_PGE, LP_PGE, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

int ufs_kirin_resume_after_set_link_state(
	struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
#ifdef FEATURE_KIRIN_UFS_PSW
	struct ufs_kirin_host *host = hba->priv;

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	ufshcd_writel(hba, host->busthrtl_backup, UFS_REG_OCPTHRTL);
#endif
	return 0;
}

static void mphy_iso_enable(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;
	/* MPHY power off on FPGA */
	if (host->caps & USE_HISI_MPHY_TC)
		writel(0x00010001,
			SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
	else
		writel(0x00200020,
			SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
}

static void mphy_iso_disable(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	if (host->caps & USE_HISI_MPHY_TC)
		writel(0x00010000,
			SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
	else
		writel(0x00200000,
			SOC_ACTRL_ISO_EN_GROUP0_PERI_ADDR(host->actrl));
}

int ufs_kirin_suspend(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	struct ufs_kirin_host *host = hba->priv;
	uint32_t value;
	int retry;

	/*set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 0 when idle*/
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 0);

	if (ufshcd_is_runtime_pm(pm_op))
		return 0;

	if (host->in_suspend) {
		WARN_ON(1); /*lint !e730*/
		return 0;
	}
	udelay(10);

	/* Step 8 */
	init_completion(&hba->reg_ram_store_done);
	ufshcd_writel(hba, 0x1, UFS_CFG_RAM_CTRL);

	/*Step 13. check if the store is complete */
	if (!wait_for_completion_timeout(
		    &hba->reg_ram_store_done, msecs_to_jiffies(20))) {
		dev_err(hba->dev, "UFS registers saving is not finished\n");
		return -1;
	}

	/* Step 14. clear bit0 of ufs_cfg_ram_ctrl */
	value = ufshcd_readl(hba, UFS_CFG_RAM_CTRL);
	value &= (~0x01);
	ufshcd_writel(hba, value, UFS_CFG_RAM_CTRL);

	/* Step 15. enable MPHY ISO*/
	mphy_iso_enable(hba);

	/* Step 16. enable NOC powerdomain to low power mode */
	/* set bit1 to 1 with mask bit */
	writel(0x00020002, SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(host->pmctrl));
	/* Polling the PMC idle ack0 register */
	retry = 2000;
	while (retry--) {
		value = readl(
			SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(host->pmctrl));

		if (value & NOC_UFS_POWER_IDLEACK)
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle failed\n");

	/* Polling the PMC idle status */
	retry = 2000;
	while (retry--) {
		value = readl(SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(host->pmctrl));

		if (value & NOC_UFS_POWER_IDLE)
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle status err\n");

	/* Step 17. enable ip_rst_ufs_subsys, reset ufs sub-system */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTEN1);

	/* Step 18. disable input clk of UFS sub-system */
	sys_ctrl_set_bits(host, GT_CLK_UFS_SUBSYS, SOC_SCTRL_SCPERDIS4);

	/* Step 19. enable reset of UFS sub-system CRG */
	sys_ctrl_set_bits(
		host, BIT(IP_RST_UFS_SUBSYS_CRG), SOC_SCTRL_SCPERRSTEN1);

	/* Step 20. enable RET of dual-rail RAM, set DS =1 , SD = 0 */
	writel(0x00030002, SOC_ACTRL_AO_MEM_CTRL2_ADDR(host->actrl));

	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 0);

	host->in_suspend = true;

	return 0;
}

static void ufs_sub_sys_crg_reset_release(struct ufs_hba *hba)
{
	u32 value = 0;
	int retry = 2000;
	struct ufs_kirin_host *host = hba->priv;

	sys_ctrl_set_bits(
		host, BIT(IP_RST_UFS_SUBSYS_CRG), SOC_SCTRL_SCPERRSTDIS1);
	while (retry--) {
		value = readl(SOC_SCTRL_SCPERRSTSTAT1_ADDR(host->sysctrl));

		if (!(value & BIT(IP_RST_UFS_SUBSYS_CRG)))
			return;
		udelay(1);
	}
	dev_err(hba->dev, "UFS Sub-sys CRG reset failed\n");
}

static int wait_mphy_pll_lock(struct ufs_hba *hba)
{
	int retry = 40000;
	u32 value = 0;
	struct ufs_kirin_host *host = hba->priv;

	if (host->caps & USE_HISI_MPHY_TC)
		return 0;

	while (retry--) {
		value = readl(
			SOC_UFS_Sysctrl_HIUFS_DEBUG_ADDR(host->ufs_sys_ctrl));
		if (value & MPHY_PLL_LOCK)
			return 0;
		udelay(1);
	}
	dev_err(hba->dev, "MPHY PLL Lock err\n");
	return -1;
}

int ufs_kirin_resume(struct ufs_hba *hba, enum ufs_pm_op pm_op)
{
	int retry, ret;
	u32 value = 0;
	struct ufs_kirin_host *host = hba->priv;

	/*set SOC_SCTRL_SCBAKDATA11_ADDR ufs bit to 1 when busy*/
	if (!ufshcd_is_auto_hibern8_allowed(hba))
		hisi_idle_sleep_vote(ID_UFS, 1);

	if (!host->in_suspend)
		return 0;

	hisi_pmic_reg_write(PMIC_CLK_UFS_EN_ADDR(0), 1);
	/* 250us to ensure the clk stable */
	udelay(250);

	/* Step 1. release the reset of UFS subsys CRG */
	ufs_sub_sys_crg_reset_release(hba);

	/* Step 2. enable the clk of UFS side of NOC async bridge */
	hsdt_crg_set_bits(
		host, BIT(GT_CLK_UFS_NOC_ASYNCBRG), SOC_HSDT_CRG_PEREN1);
	/* release the reset of UFS side of NOC async bridge */
	hsdt_crg_set_bits(
		host, BIT(IP_RST_UFS_NOC_ASYNCBRG), SOC_HSDT_CRG_PERRSTDIS0);

	/* Step 4. disable RET of dual-rail RAM, DS = 0, SD = 0*/
	writel(0x00030000, SOC_ACTRL_AO_MEM_CTRL2_ADDR(host->actrl));

	/* Step 5. Reset UFS_SUBSYS */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTEN1);

	/* Step 6. setup ufs sub-system clk to 224MHz*/
	writel(0x003F0006, SOC_SCTRL_SCCLKDIV9_ADDR(host->sysctrl));
	udelay(1);

	/* Step 7. Release the reset of UFS_SUBSYS */
	sys_ctrl_set_bits(host, BIT(IP_RST_UFS_SUBSYS), SOC_SCTRL_SCPERRSTDIS1);

	/* Step 8. Enable clk_ufs_subsys clock */
	writel(BIT(SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START),
		SOC_SCTRL_SCPEREN4_ADDR(host->sysctrl));

	/* Step 9. handshake of UFS powerdomain */
	writel(NOC_UFS_POWER_IDLEREQ_MASK,
		SOC_PMCTRL_NOC_POWER_IDLEREQ_0_ADDR(host->pmctrl));

	/* Polling the PMC idle ack0 register */
	retry = 2000;
	while (retry--) {
		value = readl(
			SOC_PMCTRL_NOC_POWER_IDLEACK_0_ADDR(host->pmctrl));

		if (!(value & NOC_UFS_POWER_IDLEACK))
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle exit failed\n");

	/* Polling the PMC idle status */
	retry = 2000;
	while (retry--) {
		value = readl(SOC_PMCTRL_NOC_POWER_IDLE_0_ADDR(host->pmctrl));

		if (!(value & NOC_UFS_POWER_IDLE))
			break;
		udelay(1);
	}
	if (retry < 0)
		dev_err(hba->dev, "UFS Power idle exit status err\n");

	/* Step 10. enable UFS CFG clk */
	writel(0x00080008,
		SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(host->ufs_sys_ctrl));

	/* Step 11. release internal release of UFS Subsys */
	writel(0x01FF01FF,
		SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(host->ufs_sys_ctrl));

	/* Step 12. disable UFS CFG clk */
	writel(0x00080000,
		SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(host->ufs_sys_ctrl));

	/* Step 13. disable MPHY ISO */
	mphy_iso_disable(hba);

	/* Step 14. enable UFS CFG clk */
	writel(0x00080008,
		SOC_UFS_Sysctrl_CRG_UFS_CFG1_ADDR(host->ufs_sys_ctrl));

	/* Step 15. polling UFS MPHY PLL lock */
	ret = wait_mphy_pll_lock(hba);
	if (ret)
		return ret;

	/* Step 16 */
	udelay(500);

	/* Step 1.
	 * request UFSHCI and MPHY to restore register from dual-rail RAM
	 */
	init_completion(&hba->reg_ram_restore_done);
	ufshcd_writel(hba, 0x2, UFS_CFG_RAM_CTRL);

	/* Step 5. check if thr restore is complete */
	if (!wait_for_completion_timeout(
		    &hba->reg_ram_restore_done, msecs_to_jiffies(20))) {
		dev_err(hba->dev, "UFS registers restore is not finished\n");
		return -1;
	}

	/* Step 6. */
	value = ufshcd_readl(hba, UFS_CFG_RAM_CTRL);
	value &= (~0x02);
	ufshcd_writel(hba, value, UFS_CFG_RAM_CTRL);

	/* if we need to restore registers, put it here */

	/* Step 8.*/
	hisi_uic_write_reg(hba, HIBERNATE_EXIT_MODE, 0x1);

	host->in_suspend = false;
	return 0;
}

void ufs_kirin_device_hw_reset_hybrid(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	if (likely(!(host->caps & USE_HISI_MPHY_TC))) {
		ufs_sys_ctrl_writel(
			host, MASK_UFS_DEVICE_RESET | 0, UFS_DEVICE_RESET_CTRL);
		mdelay(1);
	} else if (!IS_V200_MPHY(hba)) {
		ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTDIS);
		mdelay(1);
	} else {
		ufs_i2c_writel(hba, 0x20000, SC_RSTEN_V200);
		mdelay(2);
	}

	if (likely(!(host->caps & USE_HISI_MPHY_TC)))
		ufs_sys_ctrl_writel(host,
			MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET,
			UFS_DEVICE_RESET_CTRL);
	else if (!IS_V200_MPHY(hba)) {
		ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTEN);
	} else {
		ufs_i2c_writel(hba, 0x20000, SC_RSTDIS_V200);
	}
	mdelay(10);
}

void ufs_kirin_device_hw_reset(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	if (!ufshcd_is_hisi_ufs_hc(hba))
		return ufs_kirin_device_hw_reset_hybrid(hba);

	if (likely(!(host->caps & USE_HISI_MPHY_TC))) {
		/* Enable device reset */
		writel(0x04000000,
			SOC_ACTRL_BITMSK_NONSEC_CTRL1_ADDR((host->actrl)));
		mdelay(1);
	} else {
		if (!IS_V200_MPHY(hba)) {
			ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTDIS);
			mdelay(1);
		} else {
			ufs_i2c_writel(hba, 0x20000, SC_RSTEN_V200);
			mdelay(2); /* wait 2 ms */
		}
	}

	if (likely(!(host->caps & USE_HISI_MPHY_TC)))
		/* Disable device reset */
		writel(0x04000400,
			SOC_ACTRL_BITMSK_NONSEC_CTRL1_ADDR((host->actrl)));
	else {
		if (!IS_V200_MPHY(hba))
			ufs_i2c_writel(hba, (unsigned int)BIT(6), SC_RSTEN);
		else
			ufs_i2c_writel(hba, 0x20000, SC_RSTDIS_V200);
	}
	mdelay(10);
}

int ufs_kirin_link_startup_pre_change(struct ufs_hba *hba)
{
	int err = 0;
	struct ufs_attr_cfg *cfg = NULL;
	struct ufs_kirin_host *host = hba->priv;

	dev_info(hba->dev, "%s ++\n", __func__);

	/*for FPGA hisi MPHY*/
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (IS_V200_MPHY(hba)) {
			cfg = hisi_mphy_v200_pre_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);
			mdelay(40);
		} else {
			cfg = hisi_mphy_v120_pre_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);
		}
	} else {
		/* for ASIC hisi MPHY and emu */
		cfg = hisi_mphy_V300_pre_link_attr;
		hisi_set_each_cfg_attr(hba, cfg);
		err = wait_mphy_init_done(hba);
		if (err)
			return err;
	}

	/*for hisi MPHY*/
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (IS_V200_MPHY(hba))
			hisi_mphy_V200_updata_vswing_fsm(hba, host);
		else
			hisi_mphy_updata_vswing_fsm_ocs5(hba, host);

		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0x15D5, 0x0), 0x1);

		pr_info("%s --\n", __func__);

		return err;
	}
	pr_info("%s --\n", __func__);
	return err;
}

static void hisi_mphy_link_post_config(
	struct ufs_hba *hba, struct ufs_kirin_host *host)
{
	uint32_t tx_lane_num = 1;
	uint32_t rx_lane_num = 1;

	if (host->caps & USE_HISI_MPHY_TC) {
		/*set the PA_TActivate to 128. need to check in ASIC...*/
		/* H8's workaround */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x15a8, 0x0), 5);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x80da, 0x0), 0x2d);

		ufshcd_dme_get(hba, UIC_ARG_MIB(0x1561), &tx_lane_num);
		ufshcd_dme_get(hba, UIC_ARG_MIB(0x1581), &rx_lane_num);

		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x4),
			0x0); /*RX_MC_PRESENT*/
		if (tx_lane_num > 1 && rx_lane_num > 1) {
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5),
				0x0); /*RX_MC_PRESENT*/
		}
	}
}

void set_device_clk(struct ufs_hba *hba)
{
}

int ufs_kirin_link_startup_post_change(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;
	struct ufs_attr_cfg *cfg = NULL;
	uint32_t value, value_bak;

	pr_info("%s ++\n", __func__);

	/* for hisi asic mphy and emu, use USE_HISI_MPHY_ASIC on ASIC later */
	if (!(host->caps & USE_HISI_MPHY_TC) || hba->host->is_emulator) {
		cfg = hisi_mphy_V300_post_link_attr;
		hisi_set_each_cfg_attr(hba, cfg);
	}

	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (IS_V200_MPHY(hba)) {
			hisi_mphy_V200_link_post_config(hba, host);
			cfg = hisi_mphy_v200_post_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);

			ufshcd_dme_get(hba, UIC_ARG_MIB(0x1552), &value);
			if (value < 0x4B)
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x1552, 0x0), 0x4B);
			ufshcd_dme_get(hba, UIC_ARG_MIB(0x1554), &value);
			if (value < 0x4C)
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x1554, 0x0), 0x4C);
			ufshcd_dme_get(hba, UIC_ARG_MIB(0x1556), &value);
			if (value < 0x4D)
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x1556, 0x0), 0x4D);
			ufshcd_dme_get(hba, UIC_ARG_MIB(0x15D0), &value);
			if (value < 0x4E)
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x15D0, 0x0), 0x4E);

			ufshcd_dme_get(
				hba, UIC_ARG_MIB_SEL(0x00ba, 0x0), &value_bak);
			ufshcd_dme_get(
				hba, UIC_ARG_MIB_SEL(0x00ae, 0x0), &value);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ae, 0x0),
				value | BIT(7));
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL(0x00ba, 0x0), value_bak);

		} else {
			hisi_mphy_link_post_config(hba, host);
			cfg = hisi_mphy_v120_post_link_attr;
			hisi_set_each_cfg_attr(hba, cfg);
		}
	}

	if (host->caps & BROKEN_CLK_GATE_BYPASS) {
#ifndef CONFIG_HISI_UFS_HC
		/* not bypass ufs clk gate */
		ufs_sys_ctrl_clr_bits(
			host, MASK_UFS_CLK_GATE_BYPASS, CLOCK_GATE_BYPASS);
		ufs_sys_ctrl_clr_bits(
			host, MASK_UFS_SYSCRTL_BYPASS, UFS_SYSCTRL);
#endif
	}

	if (host->hba->caps & UFSHCD_CAP_AUTO_HIBERN8)
		/* disable power-gating in auto hibernate 8 */
		ufshcd_rmwl(hba, (LP_AH8_PGE | LP_PGE), 0, UFS_REG_OCPTHRTL);

	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09a),
		0x80000000); /* select received symbol cnt */
	ufshcd_dme_set(hba, UIC_ARG_MIB(0xd09c),
		0x00000005); /* reset counter0 and enable */

	pr_info("%s --\n", __func__);
	return 0;
}

/**
 * Soc init will reset host controller, all register value will lost
 * including memory address, doorbell and AH8 AGGR
 */
void ufs_kirin_full_reset(struct ufs_hba *hba)
{
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_disable_volt_irq(hba);
#endif
	disable_irq(hba->irq);

	/*
	 * Cancer platform need a full reset when error handler occurs.
	 * If a request sending in ufshcd_queuecommand passed through
	 * ufshcd_state check. And eh may reset host controller, a NOC
	 * error happens. 1000ms sleep is enough for waiting those requests.
	 **/
	msleep(1000);

	ufs_soc_init(hba);

	enable_irq(hba->irq);
#ifdef CONFIG_HUAWEI_UFS_DSM
	dsm_ufs_enable_volt_irq(hba);
#endif
}

/*lint -restore*/

/* compatible with old platform */
void ufs_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value;
	struct ufs_kirin_host *host = hba->priv;

	if (ufshcd_is_hisi_ufs_hc(hba))
		return ufs_hisi_kirin_pwr_change_pre_change(
			hba, dev_req_params);

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif
	/*
	 * ARIES platform need to set SaveConfigTime to 0x13, and change sync
	 * length to maximum value
	 */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xD0A0),
		0x13); /* VS_DebugSaveConfigTime */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1552), 0x4f); /* g1 sync length */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1554), 0x4f); /* g2 sync length */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1556), 0x4f); /* g3 sync length */

	ufshcd_dme_get(hba, UIC_ARG_MIB(0x15A7), &value);
	if (value < 0xA)
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0x15a7),
			0xA);				    /* PA_Hibern8Time */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0x15a8), 0xA); /* PA_Tactivate */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xd085, 0x0), 0x01);

	ufshcd_dme_set(hba, UIC_ARG_MIB(0x155c), 0x0); /* PA_TxSkip */

	/*PA_PWRModeUserData0 = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b0), 8191);
	/*PA_PWRModeUserData1 = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b1), 65535);
	/*PA_PWRModeUserData2 = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b2), 32767);
	/*DME_FC0ProtectionTimeOutVal = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd041), 8191);
	/*DME_TC0ReplayTimeOutVal = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd042), 65535);
	/*DME_AFC0ReqTimeOutVal = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd043), 32767);
	/*PA_PWRModeUserData3 = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b3), 8191);
	/*PA_PWRModeUserData4 = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b4), 65535);
	/*PA_PWRModeUserData5 = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b5), 32767);
	/*DME_FC1ProtectionTimeOutVal = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd044), 8191);
	/*DME_TC1ReplayTimeOutVal = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd045), 65535);
	/*DME_AFC1ReqTimeOutVal = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd046), 32767);

	pr_info("%s --\n", __func__);
	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (IS_V200_MPHY(hba))
			hisi_mphy_V200_pwr_change_pre_config(hba, host);
	}
}
