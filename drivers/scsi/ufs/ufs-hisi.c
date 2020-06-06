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

#include <linux/bootdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <soc_crgperiph_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>

#include "dsm_ufs.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include "ufshci.h"
#include "unipro.h"
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
extern bool ufshcd_support_inline_encrypt(struct ufs_hba *hba);
#endif
#ifdef CONFIG_HISI_DEBUG_FS
#define HISI_UFS_BUG() BUG()
#else
#define HISI_UFS_BUG()                                                         \
	do {                                                                   \
	} while (0)
#endif

int hisi_set_each_cfg_attr(struct ufs_hba *hba, struct ufs_attr_cfg *cfg)
{
	if (!cfg)
		return 0;

	while (cfg->addr != 0) {
		hisi_uic_write_reg(hba, cfg->addr, cfg->val);
		cfg++;
	}
	return 0;
}

static bool is_dme_reg(uint32_t reg_offset)
{
	if (((reg_offset >> 12) & 0xF) == 0xD)
		return true;
	else
		return false;
}

int hisi_uic_write_reg(struct ufs_hba *hba, uint32_t reg_offset, uint32_t value)
{
	int ret = 0;
	int h8_state;
	int retries = HISI_UIC_ACCESS_REG_RETRIES;
	bool auto_h8_disabled = 0;

	if ((hba->autoh8_disable_depth == 0) && !is_dme_reg(reg_offset)) {
		ret = ufshcd_hisi_disable_auto_hibern8(hba);
		if (ret) {
			dev_err(hba->dev,
				"disable autoh8 fail, skip uic reg access\n");
			return ret;
		}
		auto_h8_disabled = 1;
	}

	h8_state = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF);
	if ((h8_state & UFS_HC_AH8_STATE) != AH8_XFER)
		dev_err(hba->dev, "hisi ufs not in ah8_xfer status");

	do {
		writel(value, hba->ufs_unipro_base +
				      (reg_offset << HISI_UNIPRO_BIT_SHIFT));
		ret = (u32)readl(
			hba->ufs_unipro_base +
			(DME_LOCAL_OPC_STATE << HISI_UNIPRO_BIT_SHIFT));
	} while ((ret == LOCAL_ATTR_BUSY) && retries--);

	if (ret)
		dev_err(hba->dev, "hisi_uic_write [0x%x] error : 0x%x\n",
			reg_offset, ret);
	if (auto_h8_disabled)
		ufshcd_hisi_enable_auto_hibern8(hba);

	return ret;
}

int hisi_uic_read_reg(struct ufs_hba *hba, uint32_t reg_offset, u32 *value)
{
	int ret = 0;
	int h8_state;
	int retries = HISI_UIC_ACCESS_REG_RETRIES;
	bool auto_h8_disabled = 0;

	/* only when auto hibern8 is enabled, disable auto hibern8 is needed */
	if ((hba->autoh8_disable_depth == 0) && !is_dme_reg(reg_offset)) {
		ret = ufshcd_hisi_disable_auto_hibern8(hba);
		if (ret) {
			dev_err(hba->dev,
				"disable autoh8 fail, skip uic reg access\n");
			return ret;
		}
		auto_h8_disabled = 1;
	}

	h8_state = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF);
	if ((h8_state & UFS_HC_AH8_STATE) != AH8_XFER)
		dev_err(hba->dev, "hisi ufs not in ah8_xfer status");

	do {
		*value = (u32)readl(hba->ufs_unipro_base +
				    (reg_offset << HISI_UNIPRO_BIT_SHIFT));
		ret = (u32)readl(
			hba->ufs_unipro_base +
			(DME_LOCAL_OPC_STATE << HISI_UNIPRO_BIT_SHIFT));

	} while ((ret == LOCAL_ATTR_BUSY) && retries--);
	if (ret)
		dev_err(hba->dev, "hisi_uic_read [0x%x] error : 0x%x\n",
			reg_offset, ret);
	if (auto_h8_disabled)
		ufshcd_hisi_enable_auto_hibern8(hba);

	return ret;
}

int hisi_uic_peer_set(struct ufs_hba *hba, uint32_t offset, uint32_t value)
{
	uint32_t ctrl_dw = 0;

	hisi_uic_write_reg(hba, DME_PEER_OPC_WDATA, value);

	ctrl_dw = offset | UFS_BIT(28) | UFS_BIT(31);
	return hisi_uic_write_reg(hba, DME_PEER_OPC_CTRL, ctrl_dw);
}

int hisi_uic_peer_get(struct ufs_hba *hba, uint32_t offset, uint32_t *value)
{
	uint32_t ctrl_dw = 0;
	int ret = 0;

	ctrl_dw = offset | UFS_BIT(31);

	init_completion(&hba->uic_peer_get_done);
	hisi_uic_write_reg(hba, DME_PEER_OPC_CTRL, ctrl_dw);

	if (wait_for_completion_timeout(&hba->uic_peer_get_done,
		    msecs_to_jiffies(HISI_UIC_ACCESS_REG_TIMEOUT)))
		hisi_uic_read_reg(hba, DME_PEER_OPC_RDATA, value);
	else
		ret = -ETIMEDOUT;

	return ret;
}

/***************************************************************
 *
 * snps_to_hisi_addr
 * Description: the address transltation is different between
 *     HISI and SNPS UFS. For backward compatible, use this function
 *     to translate the address.
 *
 ***************************************************************/
/* SNPS addr coding : 0x REG_addr | select
 * example: TX 0x41 lane0 --> 0x00410000
 *      TX 0x41 lane1 --> 0x00410001
 *      RX 0xC1 lane0 --> 0x00C10004
 *      RX 0xC1 lane1 --> 0x00C10005
 *      HISI addr coding : 0x select | Reg_addr
 *      example: TX 0x41 lane0 --> 0x00000041
 *      TX 0x41 lane1 --> 0x00010041
 *      RX 0xC1 lane0 --> 0x000200C1
 *      RX 0xC1 lane1 --> 0x000300C1
 */
uint32_t snps_to_hisi_addr(uint32_t cmd, uint32_t arg1)
{
	uint32_t temp = 0;
	uint32_t rx0_sel = HISI_RX0_SEL;
	uint32_t rx1_sel = HISI_RX1_SEL;

	if (arg1 == 0xD0850000) {
		temp = 0x0000D014;
		return temp;
	}
	if ((cmd == UIC_CMD_DME_PEER_SET) || (cmd == UIC_CMD_DME_PEER_GET)) {
		/* If peer operation, we should use spec-defined SEL */
		rx0_sel = SPEC_RX0_SEL;
		rx1_sel = SPEC_RX1_SEL;
	}

	temp = ((arg1 & UIC_ADDR_MASK) >> UIC_SHIFT);
	temp &= UIC_SLE_MASK;
	if ((arg1 & 0xF) == SNPS_TX0_SEL)
		temp |= (HISI_TX0_SEL << UIC_SHIFT);
	else if ((arg1 & 0xF) == SNPS_TX1_SEL)
		temp |= (HISI_TX1_SEL << UIC_SHIFT);
	else if ((arg1 & 0xF) == SNPS_RX0_SEL)
		temp |= (rx0_sel << UIC_SHIFT);
	else if ((arg1 & 0xF) == SNPS_RX1_SEL)
		temp |= (rx1_sel << UIC_SHIFT);

	return temp;
}

int hisi_dme_link_startup(struct ufs_hba *hba)
{
	int ret = 0;
	uint32_t value = 0;
	uint32_t link_state;

	hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
	link_state = value & MASK_DEBUG_UNIPRO_STATE;
	if (link_state != LINK_DOWN) {
		dev_err(hba->dev, "unipro is not linkdown, error\n");
		return -1;
	}

	init_completion(&hba->uic_linkup_done);
	hisi_uic_write_reg(hba, DME_LINKSTARTUPREQ, 0x1);
	if (wait_for_completion_timeout(&hba->uic_linkup_done,
		    msecs_to_jiffies(HISI_UFS_DME_LINKUP_TIMEOUT)))
		return ret;

	hisi_ufs_dme_reg_dump(hba, HISI_UIC_LINKUP_FAIL);
	return -ETIMEDOUT;
}

int ufshcd_hisi_wait_for_unipro_register_poll(
	struct ufs_hba *hba, u32 reg, u32 mask, u32 val, int timeout_ms)
{
	int ret = 0;
	uint32_t reg_val;

	timeout_ms = timeout_ms * 10;
	while (timeout_ms-- > 0) {
		hisi_uic_read_reg(hba, reg, &reg_val);
		if ((reg_val & mask) == (val & mask))
			return ret;
		udelay(100);
	}
	ret = -ETIMEDOUT;
	return ret;
}

/**
 * ufshcd_host_memory_configure - configure local reference block with
 *				memory offsets
 * @hba: per adapter instance
 *
 * Configure Host memory space
 * 1. Update Corresponding UTRD.UCDBA and UTRD.UCDBAU with UCD DMA
 * address.
 * 2. Update each UTRD with Response UPIU offset, Response UPIU length
 * and PRDT offset.
 * 3. Save the corresponding addresses of UTRD, UCD.CMD, UCD.RSP and UCD.PRDT
 * into local reference block.
 */
void ufshcd_hisi_host_memory_configure(struct ufs_hba *hba)
{
	struct utp_transfer_cmd_desc *cmd_descp;
	struct hisi_utp_transfer_req_desc *hisi_utrdlp;
	dma_addr_t cmd_desc_dma_addr;
	dma_addr_t cmd_desc_element_addr;
	u16 response_offset;
	u16 prdt_offset;
	int cmd_desc_size;
	int i;

	hisi_utrdlp = hba->hisi_utrdl_base_addr;
	cmd_descp = hba->ucdl_base_addr;

	response_offset =
		(u16)offsetof(struct utp_transfer_cmd_desc, response_upiu);
	prdt_offset = (u16)offsetof(struct utp_transfer_cmd_desc, prd_table);

	cmd_desc_size = sizeof(struct utp_transfer_cmd_desc);
	cmd_desc_dma_addr = hba->ucdl_dma_addr;

	for (i = 0; i < hba->nutrs; i++) {
		/* Configure UTRD with command descriptor base address */
		cmd_desc_element_addr =
			(cmd_desc_dma_addr + (cmd_desc_size * (unsigned int)i));
		hisi_utrdlp[i].command_desc_base_addr_lo =
			cpu_to_le32(lower_32_bits(cmd_desc_element_addr));
		hisi_utrdlp[i].command_desc_base_addr_hi =
			cpu_to_le32(upper_32_bits(cmd_desc_element_addr));

		/* Response upiu and prdt offset should be in double words */
		if (hba->quirks & UFSHCD_QUIRK_PRDT_BYTE_GRAN) {
			hisi_utrdlp[i].response_upiu_offset =
				cpu_to_le16(response_offset);
			hisi_utrdlp[i].prd_table_offset =
				cpu_to_le16(prdt_offset);
			hisi_utrdlp[i].response_upiu_length =
				cpu_to_le16(ALIGNED_UPIU_SIZE);
		} else {
			hisi_utrdlp[i].response_upiu_offset =
				cpu_to_le16((response_offset >> 2));
			hisi_utrdlp[i].prd_table_offset =
				cpu_to_le16((prdt_offset >> 2));
			hisi_utrdlp[i].response_upiu_length =
				cpu_to_le16(ALIGNED_UPIU_SIZE >> 2);
		}

		hba->lrb[i].hisi_utr_descriptor_ptr = (hisi_utrdlp + i);
		hba->lrb[i].utrd_dma_addr =
			hba->utrdl_dma_addr +
			(i * sizeof(struct hisi_utp_transfer_req_desc));
		hba->lrb[i].ucd_req_ptr =
			(struct utp_upiu_req *)(cmd_descp + i);
		hba->lrb[i].ucd_req_dma_addr = cmd_desc_element_addr;
		hba->lrb[i].ucd_rsp_ptr =
			(struct utp_upiu_rsp *)cmd_descp[i].response_upiu;
		hba->lrb[i].ucd_rsp_dma_addr =
			cmd_desc_element_addr + response_offset;
		hba->lrb[i].ucd_prdt_ptr =
			(struct ufshcd_sg_entry *)cmd_descp[i].prd_table;
		hba->lrb[i].ucd_prdt_dma_addr =
			cmd_desc_element_addr + prdt_offset;
	}
}

int ufshcd_hisi_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode)
{
	int ret = 0;
	u32 reg_val;
	int retries = HISI_UIC_ACCESS_REG_RETRIES;

	if (hba->quirks & UFSHCD_QUIRK_BROKEN_PA_RXHSUNTERMCAP) {
		ret = ufshcd_dme_set(
			hba, UIC_ARG_MIB_SEL(PA_RXHSUNTERMCAP, 0), 1);
		if (ret) {
			dev_err(hba->dev, "%s: failed to enable PA_RXHSUNTERMCAP ret %d\n",
				__func__, ret);
			return ret;
		}
	}
	init_completion(&hba->uic_pmc_done);

retry:
	ret = hisi_uic_write_reg(hba, PA_PWRMODE, mode);
	if (ret)
		return ret;
	if (wait_for_completion_timeout(&hba->uic_pmc_done,
		    msecs_to_jiffies(HISI_UIC_ACCESS_REG_TIMEOUT)))
		return ret;

	dev_err(hba->dev, "hisi ufs pmc timetout\n");
	hisi_uic_read_reg(hba, DME_POWERMODEIND, &reg_val);
	if ((reg_val & PWR_BUSY) && retries--)
		goto retry;

	hisi_ufs_dme_reg_dump(hba, HISI_UIC_PMC_FAIL);
	return -ETIMEDOUT;
}

void clear_hisi_h8_intr(struct ufs_hba *hba, bool h8_op)
{
	if (h8_op == UFS_HISI_H8_OP_ENTER) {
		clear_unipro_intr(hba,
			DME_HIBERN_ENTER_CNF_INTR | DME_HIBERN_ENTER_IND_INTR);
		ufshcd_writel(hba, UIC_HIBERNATE_ENTER, REG_INTERRUPT_STATUS);
	} else {
		clear_unipro_intr(hba,
			DME_HIBERN_EXIT_CNF_INTR | DME_HIBERN_EXIT_IND_INTR);
		ufshcd_writel(hba, UIC_HIBERNATE_EXIT, REG_INTERRUPT_STATUS);
	}
}

static int ufshcd_hisi_hibern8_enter_req(struct ufs_hba *hba)
{
	int ret;
	u32 value = 0;
	int retries = HISI_UIC_ACCESS_REG_RETRIES;
	int timeout_ms = HISI_UIC_ACCESS_REG_TIMEOUT;

retry:
	hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &value);
	hisi_uic_write_reg(hba, DME_HIBERNATE_ENTER, 0x1);

	ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
		DME_HIBERNATE_ENTER_STATE, DME_HIBERNATE_REQ_RECEIVED,
		DME_HIBERNATE_REQ_RECEIVED, timeout_ms);
	if (ret) {
		hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &value);

		if ((value == DME_HIBERNATE_REQ_DENIED) && retries--)
			goto retry;

		return ret;
	}

	ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
		DME_HIBERNATE_ENTER_IND, DME_HIBERNATE_ENTER_LOCAL_SUCC,
		DME_HIBERNATE_ENTER_LOCAL_SUCC, timeout_ms);
	if (ret) {
		hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_IND, &value);
		if ((value == DME_HIBERNATE_ENTER_BUSY) && retries--)
			goto retry;
		return ret;
	}
	dev_info(hba->dev, "Enter hibernate success\n");
	return ret;
}

static int ufshcd_hisi_hibern8_exit_req(struct ufs_hba *hba)
{
	int ret;
	u32 value = 0;
	int retries = HISI_UIC_ACCESS_REG_RETRIES;
	int timeout_ms = HISI_UIC_ACCESS_REG_TIMEOUT;

retry:
	hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &value);
	hisi_uic_write_reg(hba, DME_HIBERNATE_EXIT, 0x1);

	ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
		DME_HIBERNATE_EXIT_STATE, DME_HIBERNATE_REQ_RECEIVED,
		DME_HIBERNATE_REQ_RECEIVED, timeout_ms);
	if (ret) {
		hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &value);
		if (value == DME_HIBERNATE_REQ_DENIED && retries--)
			goto retry;

		return ret;
	}

	ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
		DME_HIBERNATE_EXIT_IND, DME_HIBERNATE_EXIT_LOCAL_SUCC,
		DME_HIBERNATE_EXIT_LOCAL_SUCC, timeout_ms);
	if (ret) {
		hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_IND, &value);
		if ((value == DME_HIBERNATE_EXIT_BUSY) && retries--)
			goto retry;
		return ret;
	}
	dev_info(hba->dev, "Exit hibernate success\n");
	return ret;
}

static int ufshcd_hisi_hibern8_req(struct ufs_hba *hba, bool h8_op)
{
	int ret;

	if (h8_op == UFS_HISI_H8_OP_ENTER)
		ret = ufshcd_hisi_hibern8_enter_req(hba);
	else
		ret = ufshcd_hisi_hibern8_exit_req(hba);
	return ret;
}

static int check_unipro_state_before_hibern8(struct ufs_hba *hba, bool h8_op)
{
	int ret = 0;
	u32 value = 0;

	hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
	if (h8_op == UFS_HISI_H8_OP_ENTER) {
		if ((value & MASK_DEBUG_UNIPRO_STATE) != LINK_UP) {
			dev_err(hba->dev, "unipro state is not up, can't do hibernate enter\n");
			ret = -EPERM;
		}

	} else {
		if ((value & MASK_DEBUG_UNIPRO_STATE) != LINK_HIBERN) {
			dev_err(hba->dev, "unipro state is not hibernate, can't do hibernate exit\n");
			ret = -EPERM;
		}
	}
	return ret;
}

int ufshcd_hisi_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op)
{
	u32 value = 0;
	u32 ie_value;
	u32 dme_intr_value;
	int ret = 0;

	/* disable auto h8 when we use software h8 */
	ret = ufshcd_hisi_disable_auto_hibern8(hba);
	if (ret) {
		dev_err(hba->dev, "disable autoh8 fail before enter h8\n");
		return ret;
	}
	hisi_uic_write_reg(hba, DME_CTRL1, UNIPRO_CLK_AUTO_GATE_WHEN_HIBERN);

	/* step 1: close interrupt and save interrupt value */
	ie_value = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	/* close DME hibernate interrupt */
	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &value);
	dme_intr_value = value;
	hisi_uic_write_reg(hba, DME_INTR_ENABLE, 0);

	ret = check_unipro_state_before_hibern8(hba, h8_op);
	if (ret)
		return ret;

	ret = ufshcd_hisi_hibern8_req(hba, h8_op);
	if (ret)
		goto out;

	clear_hisi_h8_intr(hba, h8_op);

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	if ((!h8_op) && hba->check_pwm_after_h8) {
		if (!hba->vops->get_pwr_by_debug_register) {
			dev_err(hba->dev, "no check pwm op\n");
			hba->check_pwm_after_h8 = 0;
		} else {
			value = hba->vops->get_pwr_by_debug_register(hba);
			if (value == SLOW) {
				dev_err(hba->dev, "ufs pwr = 0x%x after H8\n",
					value);
				hba->check_pwm_after_h8 = 0;
				ufshcd_init_pwr_info(hba);
				if (!work_busy(&hba->recover_hs_work))
					schedule_work(&hba->recover_hs_work);
				else
					dev_err(hba->dev, "%s:recover_hs_work is running\n",
						__func__);
			} else {
				hba->check_pwm_after_h8--;
			}
			dev_err(hba->dev,
				"check pwr after H8, %d times remain\n",
				hba->check_pwm_after_h8);
		}
	}
#endif
out:

	ufsdbg_error_inject_dispatcher(
		hba, ERR_INJECT_PWR_MODE_CHANGE_ERR, 0, &ret);

	if (ret) {
		hisi_ufs_dme_reg_dump(hba, h8_op);
		ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
		dsm_ufs_update_error_info(hba, DSM_UFS_ENTER_OR_EXIT_H8_ERR);
		schedule_ufs_dsm_work(hba);
	}

	ufshcd_writel(hba, ie_value, REG_INTERRUPT_ENABLE);
	hisi_uic_write_reg(hba, DME_INTR_ENABLE, dme_intr_value);
	return ret;
}

int config_hisi_tr_qos(struct ufs_hba *hba)
{
	int ret;
	int slot = 0;
	int core = 0;
	unsigned int reg_val;
	int index = 0;
	unsigned long flags;
	int qos_outstanding_num[8];
	int qos_promote_num[7] = {0};
	int qos_increase_num[7] = {0};
	int spec_doorbell_qos[32] = {0};
	int core_doorbell_qos[64] = {0};

	spin_lock_irqsave(hba->host->host_lock, flags);
	ret = __ufshcd_wait_for_doorbell_clr(hba);
	if (ret) {
		dev_err(hba->dev,
			"wait doorbell clear timeout before config hisi qos\n");
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return ret;
	}

	for (index = 0; index < 8; index++)
		qos_outstanding_num[index] = 0x1F;
	reg_val = (qos_outstanding_num[0] |
		   (qos_outstanding_num[1] << MASK_QOS_1) |
		   (qos_outstanding_num[2] << MASK_QOS_2) |
		   (qos_outstanding_num[3] << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_OUTSTANDING);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_0_3_OUTSTANDING);
	dev_err(hba->dev, "test UFS_TR_QOS_0_3_OUTSTANDING ufs is 0x%x\n",
		reg_val);

	reg_val = (qos_outstanding_num[4] |
		   (qos_outstanding_num[5] << MASK_QOS_5) |
		   (qos_outstanding_num[6] << MASK_QOS_6) |
		   (qos_outstanding_num[7] << MASK_QOS_7));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_7_OUTSTANDING);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_4_7_OUTSTANDING);
	dev_err(hba->dev, "test UFS_TR_QOS_4_7_OUTSTANDING ufs is 0x%x\n",
		reg_val);

	reg_val = (qos_promote_num[0] | (qos_promote_num[1] << MASK_QOS_1) |
		   (qos_promote_num[2] << MASK_QOS_2) |
		   (qos_promote_num[3] << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_PROMOTE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_0_3_PROMOTE);
	dev_err(hba->dev, "test UFS_TR_QOS_0_3_PROMOTE ufs is 0x%x\n", reg_val);

	reg_val = (qos_promote_num[4] | (qos_promote_num[5] << MASK_QOS_5) |
		   (qos_promote_num[6] << MASK_QOS_6));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_6_PROMOTE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_4_6_PROMOTE);
	dev_err(hba->dev, "test UFS_TR_QOS_4_6_PROMOTE ufs is 0x%x\n", reg_val);

	reg_val = (qos_increase_num[0] | (qos_increase_num[1] << MASK_QOS_1) |
		   (qos_increase_num[2] << MASK_QOS_2) |
		   (qos_increase_num[3] << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_INCREASE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_0_3_INCREASE);
	dev_err(hba->dev, "test UFS_TR_QOS_0_3_INCREASE ufs is 0x%x\n",
		reg_val);

	reg_val = (qos_increase_num[4] | (qos_increase_num[5] << MASK_QOS_5) |
		   (qos_increase_num[6] << MASK_QOS_6));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_6_INCREASE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_4_6_INCREASE);
	dev_err(hba->dev, "test UFS_TR_QOS_4_6_INCREASE ufs is 0x%x\n",
		reg_val);

	ufshcd_writel(hba, OUTSTANDING_NUM, UFS_TR_OUTSTANDING_NUM);

	/* Enable Qos */
	reg_val = ufshcd_readl(hba, UFS_PROC_MODE_CFG);
	reg_val |= MASK_CFG_UTR_QOS_EN;
	ufshcd_writel(hba, reg_val, UFS_PROC_MODE_CFG);

	/* set UFSHCI doorbell Qos */
	reg_val = 0;
	for (slot = 0; slot <= 7; slot++)
		reg_val |= (spec_doorbell_qos[slot] << (4 * slot));

	ufshcd_writel(hba, reg_val, UFS_DOORBELL_0_7_QOS);
	reg_val = 0;
	for (slot = 8; slot <= 15; slot++) {
		index = slot - 8;
		reg_val |= (spec_doorbell_qos[slot] << (4 * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_8_15_QOS);
	reg_val = 0;
	for (slot = 16; slot <= 23; slot++) {
		index = slot - 16;
		reg_val |= (spec_doorbell_qos[slot] << (4 * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_16_23_QOS);
	reg_val = 0;
	for (slot = 24; slot <= 31; slot++) {
		index = slot - 24;
		reg_val |= (spec_doorbell_qos[slot] << (4 * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_24_31_QOS);

	/* set core 0~7 doorbell Qos */
	reg_val = 0;
	for (core = 0; core <= 7; core++) {
		for (slot = 0; slot <= 7; slot++)
			reg_val |= ((unsigned int)
					    core_doorbell_qos[8 * core +
								      slot]
				    << (4 * slot));

		ufshcd_writel(hba, reg_val, 0x2010 + core * 0x80);
	}

	spin_unlock_irqrestore(hba->host->host_lock, flags);
	return ret;
}

void ufshcd_hisi_enable_auto_hibern8(struct ufs_hba *hba)
{
	uint32_t value = 0;

	value = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
	if (value & UFS_AUTO_HIBERN_EN)
		return;

	value |= UFS_AUTO_HIBERN_EN;
	value &= ~UFS_HIBERNATE_EXIT_MODE;
	ufshcd_writel(hba, value, UFS_AHIT_CTRL_OFF);
}

int ufshcd_hisi_disable_auto_hibern8(struct ufs_hba *hba)
{
	uint32_t h8_reg;
	int retry = HISI_AUTO_H8_XFER_TIMEOUT;

	h8_reg = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
	if (h8_reg & UFS_AUTO_HIBERN_EN) {
		h8_reg &= (~UFS_AUTO_HIBERN_EN);
		ufshcd_writel(hba, h8_reg, UFS_AHIT_CTRL_OFF);
	} else {
		return 0;
	}

	do {
		h8_reg = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF);
		if ((h8_reg & UFS_HC_AH8_STATE) == 0x1)
			return 0;

		mdelay(1);
	} while (retry--);

	dev_err(hba->dev, "disable auto hibern fail\n");
	return -ETIMEDOUT;
}

void hisi_ufs_dme_reg_dump(
	struct ufs_hba *hba, enum hisi_uic_reg_dump_type dump_type)
{
	uint32_t reg_val = 0;
	int value = 0;

	hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &reg_val);
	dev_err(hba->dev, "DME_UNIPRO_STATE is 0x%x\n", reg_val);
	if (dump_type == HISI_UIC_HIBERNATE_ENTER) {
		hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATEENTER_STATE is 0x%x\n",
			reg_val);
		hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_IND, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATE_ENTER_IND is 0x%x\n", reg_val);
	} else if (dump_type == HISI_UIC_HIBERNATE_EXIT) {
		hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATEEXIT_STATE is 0x%x\n", reg_val);
		hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_IND, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATE_EXIT_IND is 0x%x\n", reg_val);
	} else if (dump_type == HISI_UIC_LINKUP_FAIL) {
		hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
		dev_err(hba->dev, "DME_UNIPRO_STATE is 0x%x\n", value);
		hisi_uic_read_reg(hba, PA_FSM_STATUS, &value);
		dev_err(hba->dev, "PA_FSM_STATUS is 0x%x\n", value);
		hisi_uic_read_reg(hba, PA_STATUS, &value);
		dev_err(hba->dev, "PA_STATUS is 0x%x\n", value);
	} else if (dump_type == HISI_UIC_PMC_FAIL) {
		hisi_uic_read_reg(hba, DME_POWERMODEIND, &reg_val);
		dev_err(hba->dev, "hisi ufs DME_POWERMODEIND is 0x%x\n",
			reg_val);
	}
}

void dbg_hisi_dme_dump(struct ufs_hba *hba)
{
	hisi_ufs_dme_reg_dump(hba, HISI_UIC_HIBERNATE_ENTER);
	hisi_ufs_dme_reg_dump(hba, HISI_UIC_HIBERNATE_EXIT);
	hisi_ufs_dme_reg_dump(hba, HISI_UIC_LINKUP_FAIL);
	hisi_ufs_dme_reg_dump(hba, HISI_UIC_PMC_FAIL);
}

void clear_unipro_intr(struct ufs_hba *hba, int dme_intr_clr)
{
	unsigned int value = 0;

	hisi_uic_read_reg(hba, DME_INTR_CLR, &value);
	value |= (unsigned int)dme_intr_clr;
	hisi_uic_write_reg(hba, DME_INTR_CLR, value);
}

void ufshcd_enable_vs_intr(struct ufs_hba *hba, u32 intrs)
{
	u32 set = ufshcd_readl(hba, UFS_VS_IE);

	set |= intrs;
	ufshcd_writel(hba, set, UFS_VS_IE);
}

void ufshcd_disable_vs_intr(struct ufs_hba *hba, u32 intrs)
{
	u32 set = ufshcd_readl(hba, UFS_VS_IE);

	set &= ~intrs;
	ufshcd_writel(hba, set, UFS_VS_IE);
}

/* enable clock gating when ufshcd is idle*/
void ufshcd_enable_clock_gating(struct ufs_hba *hba)
{
	ufshcd_writel(hba, 0xF, UFS_BLOCK_CG_CFG);
}

static void hisi_asic_mphy_pre_pmc_attr_config(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value = 0;
	struct ufs_attr_cfg *cfg = NULL;
	uint8_t deemp_20t4_en = 0;

	if (dev_req_params->pwr_rx == FAST_MODE ||
		dev_req_params->pwr_rx == FASTAUTO_MODE) {
		deemp_20t4_en = 1;
		switch (dev_req_params->gear_rx) {
		case UFS_HS_G4:
			cfg = hisi_mphy_V300_pre_pmc_fsg4_attr;
			break;
		case UFS_HS_G3:
			cfg = hisi_mphy_V300_pre_pmc_fsg3_attr;
			break;
		case UFS_HS_G2:
			cfg = hisi_mphy_V300_pre_pmc_fsg2_attr;
			break;
		case UFS_HS_G1:
			deemp_20t4_en = 0;
			cfg = hisi_mphy_V300_pre_pmc_fsg1_attr;
			break;
		default:
			dev_err(hba->dev, "unknown ufs gear\n");
			cfg = 0;
			break;
		}
	} else
		cfg = hisi_mphy_V300_pre_pmc_slow_attr;

	hisi_set_each_cfg_attr(hba, cfg);
	hisi_uic_read_reg(hba, 0x00000050, &value);
	if (deemp_20t4_en)
		value |= MPHY_DEEMPH_20T4_EN;
	else
		value &= (~MPHY_DEEMPH_20T4_EN);

	hisi_uic_write_reg(hba, 0x00000050, value);
	hisi_uic_write_reg(hba, 0x00010050, value);
}

static void hisi_mphy_tc_pre_pmc_attr_config(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	struct ufs_attr_cfg *cfg = NULL;

	if (IS_V200_MPHY(hba)) {
		if (dev_req_params->pwr_rx == FAST_MODE ||
			dev_req_params->pwr_rx == FASTAUTO_MODE) {
			switch (dev_req_params->gear_rx) {
			case UFS_HS_G4:
				cfg = hisi_mphy_V200_tc_pre_pmc_fsg4_attr;
				break;
			case UFS_HS_G3:
				cfg = hisi_mphy_V200_tc_pre_pmc_fsg3_attr;
				break;
			case UFS_HS_G2:
				cfg = hisi_mphy_V200_tc_pre_pmc_fsg2_attr;
				break;
			case UFS_HS_G1:
				cfg = hisi_mphy_V200_tc_pre_pmc_fsg1_attr;
				break;
			default:
				dev_err(hba->dev, "unknown ufs gear\n");
				cfg = 0;
				break;
			}
		} else {
			cfg = hisi_mphy_V200_tc_pre_pmc_slow_attr;
		}
		hisi_set_each_cfg_attr(hba, cfg);
	}
}

void ufs_hisi_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value = 0;
	struct ufs_kirin_host *host = hba->priv;

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif

	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1552), 0x4f); /* g1 sync length */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1554), 0x4f); /* g2 sync length */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1556), 0x4f); /* g3 sync length */
	ufshcd_dme_get(hba, UIC_ARG_MIB((u32)0x15a7), &value);
	if (value < 0xA)
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0x15a7),
			0xA);				    /* PA_Hibern8Time */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0x15a8), 0xA); /* PA_Tactivate */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xd014, 0x0), 0x01);

	ufshcd_dme_set(hba, UIC_ARG_MIB(0x155c), 0x0); /* PA_TxSkip */

	/*PA_PWRModeUserData0 = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b0), 8191);
	/*PA_PWRModeUserData1 = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b1), 65535);
	/*PA_PWRModeUserData2 = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b2), 32767);
	/*PA_PWRModeUserData3 = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b3), 8191);
	/*PA_PWRModeUserData4 = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b4), 65535);
	/*PA_PWRModeUserData5 = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b5), 32767);

	pr_info("%s --\n", __func__);
	/* for hisi asic mphy and emu, use USE_HISI_MPHY_ASIC on ASIC later */
	if (!(host->caps & USE_HISI_MPHY_TC) || hba->host->is_emulator)
		hisi_asic_mphy_pre_pmc_attr_config(hba, dev_req_params);

	if ((host->caps & USE_HISI_MPHY_TC))
		hisi_mphy_tc_pre_pmc_attr_config(hba, dev_req_params);
}
/*lint +e648 +e845*/

void ufshcd_hisi_enable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs)
{
	u32 set = 0;

	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &set);
	set |= unipro_intrs;
	hisi_uic_write_reg(hba, DME_INTR_ENABLE, set);
}

void ufshcd_hisi_disable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs)
{
	u32 set = 0;

	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &set);
	set &= ~unipro_intrs;
	hisi_uic_write_reg(hba, DME_INTR_ENABLE, set);
}

irqreturn_t ufshcd_hisi_unipro_intr(int unipro_irq, void *__hba)
{
	u32 unipro_intr_status, enabled_unipro_intr_status, result, reg_offset;
	u32 reg_value = 0;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);

	hisi_uic_read_reg(hba, DME_INTR_STATUS, &unipro_intr_status);
	if (unipro_intr_status)
		clear_unipro_intr(hba, unipro_intr_status);

	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &reg_value);
	enabled_unipro_intr_status = unipro_intr_status & reg_value;

	if (enabled_unipro_intr_status & LINKUP_CNF_INTR) {
		hisi_uic_read_reg(hba, DME_LINKSTARTUP_STATE, &reg_value);
		reg_value &= LINK_STARTUP_CNF;
		if (reg_value == LINK_STARTUP_SUCC)
			complete(&hba->uic_linkup_done);
	}
	if (enabled_unipro_intr_status & PMC_IND_INTR) {
		hisi_uic_read_reg(hba, DME_POWERMODEIND, &reg_value);
		if (reg_value & DME_POWER_MODE_LOCAL_SUCC)
			complete(&hba->uic_pmc_done);
	}
	if (enabled_unipro_intr_status & LOCAL_ATTR_FAIL_INTR) {
		hisi_uic_read_reg(hba, DME_LOCAL_OPC_DBG, &reg_value);
		if (reg_value) {
			reg_offset = reg_value & ATTR_LOCAL_ERR_ADDR;
			result = reg_value & ATTR_LOCAL_ERR_RES;
			dev_err(hba->dev, "local attr access fail, local_opc_dbg is 0x%x, reg_offset is 0x%x\n",
				result, reg_offset);
		}
	}
	if (enabled_unipro_intr_status & PEER_ATTR_COMPL_INTR) {
		hisi_uic_read_reg(hba, DME_PEER_OPC_STATE, &reg_value);
		if (reg_value & PEER_ATTR_RES)
			dev_err(hba->dev, "peer attr access fail\n");
		else
			complete(&hba->uic_peer_get_done);
	}
	if (enabled_unipro_intr_status & HSH8ENT_LR_INTR) {
		if (hba->ufshcd_state == UFSHCD_STATE_OPERATIONAL) {
			dev_err(hba->dev, "%s, unipro IS 0x%x\n", __func__,
				enabled_unipro_intr_status);
			__ufshcd_disable_pwm_cnt(hba);

			/* block commands from scsi mid-layer */
			scsi_block_requests(hba->host);

			hba->force_host_reset = true;
			hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;
			kthread_queue_work(&hba->eh_worker, &hba->eh_work);

		}
	}
	retval = IRQ_HANDLED;

	spin_unlock(hba->host->host_lock);
	return retval;
}

void ufshcd_sl_fatal_intr(struct ufs_hba *hba, u32 intr_status)
{
	ufsdbg_error_inject_dispatcher(
		hba, ERR_INJECT_INTR, intr_status, &intr_status);
	hba->errors = INT_FATAL_ERRORS & intr_status;

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	if (ufshcd_support_inline_encrypt(hba))
		hba->errors |= CRYPTO_ENGINE_FATAL_ERROR & intr_status;
#endif

	if (hba->errors)
		ufshcd_check_errors(hba);
}

irqreturn_t ufshcd_hisi_fatal_err_intr(int fatal_err_irq, void *__hba)
{
	u32 fatal_intr_status, enabled_fatal_intr_status;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);
	fatal_intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	dev_err(hba->dev, "fatal_err intr status 0x%x\n", fatal_intr_status);

	enabled_fatal_intr_status =
		fatal_intr_status & ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	if (fatal_intr_status)
		ufshcd_writel(hba, fatal_intr_status, REG_INTERRUPT_STATUS);
	if (enabled_fatal_intr_status & INT_FATAL_ERRORS)
		ufshcd_sl_fatal_intr(hba, enabled_fatal_intr_status);

	retval = IRQ_HANDLED;
	spin_unlock(hba->host->host_lock);
	return retval;
}

irqreturn_t ufshcd_hisi_core0_intr(int fatal_err_irq, void *__hba)
{
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);

	retval = IRQ_HANDLED;
	spin_unlock(hba->host->host_lock);
	return retval;
}

void ufshcd_sl_vs_intr(struct ufs_hba *hba, u32 intr_status)
{
	ufsdbg_error_inject_dispatcher(hba,
		ERR_INJECT_INTR,
		intr_status,
		&intr_status);

	/* Device Timeout handler */
	if (intr_status & UFS_RX_CPORT_TIMEOUT_INTR) {
		hba->errors |= UFS_RX_CPORT_TIMEOUT_INTR;
		if (hba->ufshcd_state == UFSHCD_STATE_OPERATIONAL) {
			dev_err(hba->dev, "%s, UFS_VS_IS 0x%x\n", __func__,
				intr_status);
			__ufshcd_disable_dev_tmt_cnt(hba);

			/* block commands from scsi mid-layer */
			scsi_block_requests(hba->host);

			hba->force_host_reset = true;
			hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;
			kthread_queue_work(&hba->eh_worker, &hba->eh_work);
		}
	}

	/* Idle Timeout handler */
	if (likely(hba->ufs_idle_intr_en)) {
		if (!hba->idle_intr_disabled) {
			if (intr_status & IDLE_PREJUDGE_INTR)
				ufshcd_idle_handler(hba);
		}
	}

	if (intr_status & SAVE_REG_CMPL_INTR)
		complete(&hba->reg_ram_store_done);

	if (intr_status & RESTORE_REG_CMPL_INTR)
		complete(&hba->reg_ram_restore_done);
}

int wait_mphy_init_done(struct ufs_hba *hba)
{
	int ret = 0;
	u32 timeout_ms = MPHY_INIT_TIMEOUT;

	if (unlikely(hba->host->is_emulator))
		return 0;

	ret = ufshcd_hisi_wait_for_unipro_register_poll(
		hba, MPHY_INIT, MASK_MPHY_INIT, MPHY_INIT_DONE, timeout_ms);

	if (ret < 0)
		dev_err(hba->dev, "Wait_mphy_init_done failed %d\n", ret);

	return ret;
}

void ufshcd_hisi_enable_dev_tmt_intr(struct ufs_hba *hba)
{
	ufshcd_enable_vs_intr(hba, UFS_RX_CPORT_TIMEOUT_INTR);
}

void ufshcd_hisi_enable_pwm_intr(struct ufs_hba *hba)
{
	ufshcd_hisi_enable_unipro_intr(hba, HSH8ENT_LR_INTR);
}

void ufshcd_hisi_enable_idle_tmt_cnt(struct ufs_hba *hba)
{
	u32 reg_val;

	reg_val = ufshcd_readl(hba, UFS_CFG_IDLE_ENABLE);
	reg_val |= IDLE_PREJUDGE_TIMTER_EN;
	ufshcd_writel(hba, reg_val, UFS_CFG_IDLE_ENABLE);
}

void ufshcd_hisi_disable_idle_tmt_cnt(struct ufs_hba *hba)
{
	u32 reg_val;

	reg_val = ufshcd_readl(hba, UFS_CFG_IDLE_ENABLE);
	reg_val &= ~IDLE_PREJUDGE_TIMTER_EN;
	ufshcd_writel(hba, reg_val, UFS_CFG_IDLE_ENABLE);
}
