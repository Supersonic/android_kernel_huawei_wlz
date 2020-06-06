/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
* Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
*/

#include "../hisi_dp.h"
#include "../hisi_fb.h"
#include "../hisi_fb_def.h"
#include "core.h"
#include "dp_aux.h"
#include "reg.h"
/*
 * Core Access Layer
 *
 * Provides low-level register access to the DPTX core.
 */

/**
 * dptx_intr_en() - Enables interrupts
 * @dptx: The dptx struct
 * @bits: The interrupts to enable
 *
 * This function enables (unmasks) all interrupts in the INTERRUPT
 * register specified by @bits.
 */
extern u16 usb31phy_cr_read(uint32_t addr);
extern u16 usb31phy_cr_write(uint32_t addr, u16 value);
/*lint -save -e* */
static void dptx_intr_en(struct dp_ctrl *dptx, uint32_t bits)
{
	uint32_t ien;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	HISI_FB_INFO("[DP] Enable DPTX Intr: %x \n", bits);
	ien = dptx_readl(dptx, DPTX_IEN);
	ien |= bits;
	dptx_writel(dptx, DPTX_IEN, ien);
}

/**
 * dptx_intr_dis() - Disables interrupts
 * @dptx: The dptx struct
 * @bits: The interrupts to disable
 *
 * This function disables (masks) all interrupts in the INTERRUPT
 * register specified by @bits.
 */
static void dptx_intr_dis(struct dp_ctrl *dptx, uint32_t bits)
{
	uint32_t ien;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	HISI_FB_INFO("[DP] Disable DPTX Intr: %x \n", bits);
	ien = dptx_readl(dptx, DPTX_IEN);
	ien &= ~bits;
	dptx_writel(dptx, DPTX_IEN, ien);
}

/**
 * dptx_global_intr_en() - Enables top-level interrupts
 * @dptx: The dptx struct
 *
 * Enables (unmasks) all top-level interrupts.
 */
void dptx_global_intr_en(struct dp_ctrl *dptx)
{
	uint32_t hpd_ien;
#ifdef CONFIG_DP_HDCP_ENABLE
	uint32_t reg = 0;
#endif

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

#ifdef CONFIG_HISI_FB_V510
	dptx_intr_en(dptx, DPTX_IEN_ALL_INTR  &
		     ~(DPTX_ISTS_AUX_REPLY | DPTX_ISTS_AUX_CMD_INVALID));
#else
	dptx_intr_en(dptx, DPTX_IEN_ALL_INTR);
#endif

	dptx_writel(dptx, DPTX_WRAP_REG_DPC_INT_MASK, 0x3);

	/* Enable all HPD interrupts */
	hpd_ien = dptx_readl(dptx, DPTX_HPD_IEN);
	hpd_ien |= (DPTX_HPD_IEN_IRQ_EN |
		    DPTX_HPD_IEN_HOT_PLUG_EN |
		    DPTX_HPD_IEN_HOT_UNPLUG_EN);
	dptx_writel(dptx, DPTX_HPD_IEN, hpd_ien);

#ifdef CONFIG_DP_HDCP_ENABLE
	if (dptx->hisifd->secure_ctrl.hdcp_dpc_sec_en)
		dptx->hisifd->secure_ctrl.hdcp_dpc_sec_en();

	/* enable interrupt include HDCP22 GPIO output status */
	if(dptx->hisifd->secure_ctrl.hdcp_reg_get)
		reg = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_API_INT_MSK);
	//reg |= DPTX_HDCP22_GPIOINT;

	if(dptx->hisifd->secure_ctrl.hdcp_int_mask)
		dptx->hisifd->secure_ctrl.hdcp_int_mask(reg);
#endif
}

/**
 * dptx_global_intr_dis() - Disables top-level interrupts
 * @dptx: The dptx struct
 *
 * Disables (masks) all top-level interrupts.
 */
void dptx_global_intr_dis(struct dp_ctrl *dptx)
{
	uint32_t hpd_ien;
#ifdef CONFIG_DP_HDCP_ENABLE
	uint32_t reg = 0;
#endif

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_intr_dis(dptx, DPTX_IEN_ALL_INTR);
	dptx_writel(dptx, DPTX_WRAP_REG_DPC_INT_MASK, 0x0);

	/*  all HPD interrupts */
	hpd_ien = dptx_readl(dptx, DPTX_HPD_IEN);
	hpd_ien &= ~(DPTX_HPD_IEN_IRQ_EN |
		    DPTX_HPD_IEN_HOT_PLUG_EN |
		    DPTX_HPD_IEN_HOT_UNPLUG_EN);
	dptx_writel(dptx, DPTX_HPD_IEN, hpd_ien);

#ifdef CONFIG_DP_HDCP_ENABLE
	if(dptx->hisifd->secure_ctrl.hdcp_dpc_sec_en)
		dptx->hisifd->secure_ctrl.hdcp_dpc_sec_en();

	/* Mask interrupt related to HDCP22 GPIO output status */
	if(dptx->hisifd->secure_ctrl.hdcp_reg_get)
		reg = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_API_INT_MSK);
	reg &= ~DPTX_HDCP22_GPIOINT;

	if(dptx->hisifd->secure_ctrl.hdcp_int_mask)
		dptx->hisifd->secure_ctrl.hdcp_int_mask(reg);
#endif
}

/**
 * dptx_clear_intr() - cleat top-level interrupts
 * @dptx: The dptx struct
 *
 * clear all top-level interrupts.
 */
void dptx_global_intr_clear(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_ALL_INTR);

	/*  all HPD interrupts */
	dptx_writel(dptx, DPTX_HPDSTS, (DPTX_HPDSTS_IRQ |DPTX_HPDSTS_HOT_PLUG | DPTX_HPDSTS_HOT_UNPLUG));
}

void dptx_aux_disreset(struct dp_ctrl *dptx, bool enable)
{
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}
#ifdef CONFIG_HISI_FB_V510
	/* Enable AUX Block */
	reg = (uint32_t)inp32(dptx->hisifd->dp_phy_base + DPTX_DP_AUX_CTRL);//lint !e838
#else
	reg = (uint32_t)dptx_readl(dptx, DPTX_WRAP_REG_DPC_AUX_CTRL);//lint !e838
#endif

	if (enable)
		reg |= DPTX_DPC_AUX_CTRL_BLOCK;//lint !e737 !e713
	else
		reg &= ~DPTX_DPC_AUX_CTRL_BLOCK;//lint !e737 !e713

	reg &= ~DPTX_TYPEC_AUX_VOD_TUNE_MASK;
	reg |= (3 << DPTX_TYPEC_AUX_VOD_TUNE_SHIFT);

#ifdef CONFIG_HISI_FB_V510
	outp32(dptx->hisifd->dp_phy_base + DPTX_DP_AUX_CTRL, reg);
#else
	dptx_writel(dptx, DPTX_WRAP_REG_DPC_AUX_CTRL, reg);
#endif

	mdelay(1);//lint !e778 !e747 !e774
}
/**
 * dptx_soft_reset() - Performs a core soft reset
 * @dptx: The dptx struct
 * @bits: The components to reset
 *
 * Resets specified parts of the core by writing @bits into the core
 * soft reset control register and clearing them 10-20 microseconds
 * later.
 */
void dptx_soft_reset(struct dp_ctrl *dptx, uint32_t bits)
{
	uint32_t rst;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	bits &= (DPTX_SRST_CTRL_ALL);

	/* Set reset bits */
	rst = dptx_readl(dptx, DPTX_SRST_CTRL);
	rst |= bits;
	dptx_writel(dptx, DPTX_SRST_CTRL, rst);

	usleep_range(10, 20);

	/* Clear reset bits */
	rst = dptx_readl(dptx, DPTX_SRST_CTRL);
	rst &= ~bits;
	dptx_writel(dptx, DPTX_SRST_CTRL, rst);
}

/**
 * dptx_soft_reset_all() - Reset all core modules
 * @dptx: The dptx struct
 */
void dptx_soft_reset_all(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_soft_reset(dptx, DPTX_SRST_CTRL_ALL);
}

void dptx_phy_soft_reset(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_soft_reset(dptx, DPTX_SRST_CTRL_PHY);
}

void dptx_typec_reset_ack(struct dp_ctrl *dptx)
{
	uint32_t dp_status;
	uint32_t reg;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	reg = dptx_readl(dptx, DPTX_TYPE_C_CTRL);
	dp_status = (reg & DPTX_TYPEC_DISABLE_STATUS) >> 1;
	reg &= ~DPTX_TYPEC_DISABLE_ACK;
	reg |= dp_status;//DPTX_TYPEC_DISABLE_ACK
	dptx_writel(dptx, DPTX_TYPE_C_CTRL, reg);
}

/**
 * dptx_core_init_phy() - Initializes the DP TX PHY module
 * @dptx: The dptx struct
 *
 * Initializes the PHY layer of the core. This needs to be called
 * whenever the PHY layer is reset.
 */
void dptx_core_init_phy(struct dp_ctrl *dptx)
{
	uint32_t phyifctrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	/* Set 40-bit PHY width */
	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	if (g_fpga_flag == 1) {
		phyifctrl |= DPTX_PHYIF_CTRL_WIDTH;	/*FPGA need 40 bit*/
	} else {
		phyifctrl &= ~DPTX_PHYIF_CTRL_WIDTH;	/*UDP need 20 bit*/
	}
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

/**
* dptx_sink_enabled_ssc() - Returns true, if sink is enabled ssc
* @dptx: The dptx struct
*
*/
bool dptx_sink_enabled_ssc(struct dp_ctrl *dptx)
{
	uint8_t byte = 0;
	int retval = 0;

	retval = dptx_read_dpcd(dptx, DP_MAX_DOWNSPREAD, &byte);
	if (retval)
		HISI_FB_ERR("[DP] dptx_read_dpcd fail\n");

	return byte & 1;
}
#ifdef CONFIG_HISI_FB_V510
/**
 * dptx_core_program_ssc() - Move phy to P3 state and programs SSC
 * @dptx: The dptx struct
 *
 * Enables SSC should be called during hot plug.
 *
 */
int dptx_core_program_ssc(struct dp_ctrl *dptx, bool sink_ssc)
{
	uint32_t phyifctrl;
	uint8_t  retval;

	/* Enable 4 lanes, before programming SSC */
	dptx_phy_set_lanes(dptx, 4);
	// Move PHY to P3 to program SSC
	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl |= (3 << DPTX_PHYIF_CTRL_LANE_PWRDOWN_SHIFT);// move phy to P3 state
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

	retval = dptx_phy_wait_busy(dptx, DPTX_MAX_LINK_LANES);
	if (retval) {
		HISI_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
		return retval;
	}

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	if(dptx->ssc_en && sink_ssc)
		phyifctrl &= ~DPTX_PHYIF_CTRL_SSC_DIS;
	else
		phyifctrl |= DPTX_PHYIF_CTRL_SSC_DIS;

	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

	retval = dptx_phy_wait_busy(dptx, DPTX_MAX_LINK_LANES);
	if (retval) {
		HISI_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
		return retval;
	}

	return 0;
}
#endif
/**
 * dptx_check_dptx_id() - Check value of DPTX_ID register
 * @dptx: The dptx struct
 *
 * Returns True if DPTX core correctly identifyed.
 */
bool dptx_check_dptx_id(struct dp_ctrl *dptx)
{
	uint32_t dptx_id;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return false;
	}

	dptx_id = dptx_readl(dptx, DPTX_ID);
	if (dptx_id != ((DPTX_ID_DEVICE_ID << DPTX_ID_DEVICE_ID_SHIFT) |
			DPTX_ID_VENDOR_ID))
		return false;

	return true;
}
#ifdef CONFIG_HISI_FB_V510
/**
* dptx_enable_ssc() - Enables SSC based on automation request,
*		      if DPTX controller enables ssc
* @dptx: The dptx struct
*
*/
void dptx_enable_ssc(struct dp_ctrl *dptx)
{
	bool sink_ssc = dptx_sink_enabled_ssc(dptx);
	if(sink_ssc)
		HISI_FB_DEBUG("[DP] %s: SSC enable on the sink side\n", __func__);
	else
		HISI_FB_DEBUG("[DP] %s: SSC disabled on the sink side\n", __func__);
	dptx_core_program_ssc(dptx, sink_ssc);
}

void dptx_init_hwparams(struct dp_ctrl *dptx)
{
	uint32_t reg;

	reg = dptx_readl(dptx, DPTX_CONFIG1);

	/* Num MST streams */
	dptx->streams = (reg & DPTX_CONFIG1_NUM_STREAMS_MASK) >>
		DPTX_CONFIG1_NUM_STREAMS_SHIFT;

	/* Combo PHY */
	dptx->hwparams.gen2phy = !!(reg & DPTX_CONFIG1_GEN2_PHY);

	/* DSC */
	dptx->hwparams.dsc = !!(reg & DPTX_CONFIG1_DSC_EN);

	/* Multi pixel mode */
	switch ((reg & DPTX_CONFIG1_MP_MODE_MASK) >> DPTX_CONFIG1_MP_MODE_SHIFT) {
	default:
	case DPTX_CONFIG1_MP_MODE_SINGLE:
		dptx->hwparams.multipixel = DPTX_MP_SINGLE_PIXEL;
		break;
	case DPTX_CONFIG1_MP_MODE_DUAL:
		dptx->hwparams.multipixel = DPTX_MP_DUAL_PIXEL;
		break;
	case DPTX_CONFIG1_MP_MODE_QUAD:
		dptx->hwparams.multipixel = DPTX_MP_QUAD_PIXEL;
		break;
	}
}
#endif
/**
 * dptx_core_init() - Initializes the DP TX core
 * @dptx: The dptx struct
 *
 * Initialize the DP TX core and put it in a known state.
 */
int dptx_core_init(struct dp_ctrl *dptx)
{
	char str[11];
	uint32_t version;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	/* Reset the core */
	dptx_soft_reset_all(dptx);
	dptx_typec_reset_ack(dptx);
	dptx_phy_set_lanes_status(dptx, false);

#ifdef CONFIG_HISI_FB_V510
	dptx_writel(dptx, DPTX_CCTL,
	    DPTX_CCTL_ENH_FRAME_EN
	    | (dptx->mst ? DPTX_CCTL_ENABLE_MST_MODE : 0)
	);
#else
	dptx_writel(dptx, DPTX_CCTL, 0x0);
#endif

	/* Check the core version */
	memset(str, 0, sizeof(str));
	version = dptx_readl(dptx, DPTX_VER_NUMBER);
	str[0] = (char)(version >> 24) & 0xff;
	str[1] = '.';
	str[2] = (char)(version >> 16) & 0xff;
	str[3] = (char)(version >> 8) & 0xff;
	str[4] = (char)(version & 0xff);

	version = dptx_readl(dptx, DPTX_VER_TYPE);
	str[5] = '-';
	str[6] = (char)(version >> 24) & 0xff;
	str[7] = (char)(version >> 16) & 0xff;
	str[8] = (char)(version >> 8) & 0xff;
	str[9] = (char)(version & 0xff);
	str[10] = '\0';

	HISI_FB_DEBUG("Core version: %s\n", str);
	dptx->version = version;

	dptx_core_init_phy(dptx);

	return 0;
}

/**
 * dptx_core_deinit() - Deinitialize the core
 * @dptx: The dptx struct
 *
 * Disable the core in preparation for module shutdown.
 */
int dptx_core_deinit(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	dptx_global_intr_dis(dptx);
	dptx_soft_reset_all(dptx);
	return 0;
}

/*
 * PHYIF core access functions
 */
void dptx_phy_set_lanes_status(struct dp_ctrl *dptx, bool bopen)
{
	uint32_t phyifctrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	if (bopen) {
		phyifctrl &= ~DPTX_PHYIF_CTRL_PWRDOWN_MASK;
	} else {
		//phyifctrl |= DPTX_PHYIF_CTRL_PWRDOWN_MASK;
		phyifctrl |= (3 << 17);
	}
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

int dptx_phy_get_lanes(struct dp_ctrl *dptx)
{
	uint32_t phyifctrl;
	uint32_t val;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return  -EINVAL;
	}

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	val = (phyifctrl & DPTX_PHYIF_CTRL_LANES_MASK) >>
		DPTX_PHYIF_CTRL_LANES_SHIFT;

	return (1 << val);
}

void dptx_phy_set_lanes(struct dp_ctrl *dptx, uint32_t lanes)
{
	uint32_t phyifctrl;
	uint32_t val;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	HISI_FB_INFO("[DP] DPTX set lanes =%d\n", lanes);

	switch (lanes) {
	case 1:
		val = 0;
		break;
	case 2:
		val = 1;
		break;
	case 4:
		val = 2;
		break;
	default:
		HISI_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		return;
	}

	phyifctrl = 0;
	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_LANES_MASK;
	phyifctrl |= (val << DPTX_PHYIF_CTRL_LANES_SHIFT);
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

void dptx_phy_set_rate(struct dp_ctrl *dptx, uint32_t rate)
{
	uint32_t phyifctrl;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	HISI_FB_INFO("[DP] DPTX set rate=%d\n", rate);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
	case DPTX_PHYIF_CTRL_RATE_HBR2:
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		break;
	default:
		HISI_FB_ERR("[DP] Invalid PHY rate %d\n", rate);
		break;
	}

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_RATE_MASK;
	phyifctrl |= rate << DPTX_PHYIF_CTRL_RATE_SHIFT;
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

int dwc_phy_get_rate(struct dp_ctrl *dptx)
{
	uint32_t phyifctrl;
	uint32_t rate;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return  -EINVAL;
	}

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	rate = (phyifctrl & DPTX_PHYIF_CTRL_RATE_MASK) >>
		DPTX_PHYIF_CTRL_RATE_SHIFT;

	return rate;
}

int dptx_phy_wait_busy(struct dp_ctrl *dptx, uint32_t lanes)
{
	unsigned long dw_jiffies = 0;
	uint32_t phyifctrl;
	uint32_t mask = 0;
	HISI_FB_INFO("[DP] lanes=%d\n", lanes);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return  -EINVAL;
	}

	/*FPGA don't need config phy*/
	if (g_fpga_flag == 1) {
		return 0;
	}
	switch (lanes) {
	/* This case (value 4) is not terminated by a 'break' statement */
	case 4:
		mask |= DPTX_PHYIF_CTRL_BUSY(3);
		mask |= DPTX_PHYIF_CTRL_BUSY(2);
	/* This case (value 2) is not terminated by a 'break' statement */
	case 2:
		mask |= DPTX_PHYIF_CTRL_BUSY(1);
	case 1:
		mask |= DPTX_PHYIF_CTRL_BUSY(0);
		break;
	default:
		HISI_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		break;
	}

	dw_jiffies = jiffies + HZ / 10;

	do {
		phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);

		if (!(phyifctrl & mask))
			return 0;

		udelay(1);
	} while (time_after(dw_jiffies, jiffies));

	HISI_FB_ERR("PHY BUSY timed out\n");
	return -EBUSY;
}

void dptx_phy_set_pre_emphasis(struct dp_ctrl *dptx,
			       uint32_t lane,
			       uint32_t level)
{
	uint32_t phytxeq;
	HISI_FB_INFO("[DP] lane=%d, level=0x%x\n", lane, level);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (lane > 3){
		HISI_FB_ERR("[DP] Invalid lane %d \n", lane);
		return;
	}

	if (level > 3) {
		HISI_FB_ERR("[DP] Invalid pre-emphasis level %d, using 3", level);
		level = 3;
	}

	phytxeq = dptx_readl(dptx, DPTX_PHY_TX_EQ);
	phytxeq &= ~DPTX_PHY_TX_EQ_PREEMP_MASK(lane);
	phytxeq |= (level << DPTX_PHY_TX_EQ_PREEMP_SHIFT(lane)) &
		DPTX_PHY_TX_EQ_PREEMP_MASK(lane);
	dptx_writel(dptx, DPTX_PHY_TX_EQ, phytxeq);
}

void dptx_phy_set_vswing(struct dp_ctrl *dptx,
			 uint32_t lane,
			 uint32_t level)
{
	uint32_t phytxeq;
	HISI_FB_INFO("[DP] lane=%d, level=0x%x\n", lane, level);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if (lane > 3){
		HISI_FB_ERR("[DP] Invalid lane %d \n", lane);
		return;
	}

	if (level > 3) {
		HISI_FB_ERR("[DP] Invalid vswing level %d, using 3", level);
		level = 3;
	}

	phytxeq = dptx_readl(dptx, DPTX_PHY_TX_EQ);
	phytxeq &= ~DPTX_PHY_TX_EQ_VSWING_MASK(lane);
	phytxeq |= (level << DPTX_PHY_TX_EQ_VSWING_SHIFT(lane)) &
		DPTX_PHY_TX_EQ_VSWING_MASK(lane);

	dptx_writel(dptx, DPTX_PHY_TX_EQ, phytxeq);
}

void dptx_phy_set_pattern(struct dp_ctrl *dptx,
			  uint32_t pattern)
{
	uint32_t phyifctrl = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	HISI_FB_INFO("[DP] Setting PHY pattern=0x%x\n", pattern);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_TPS_SEL_MASK;
	phyifctrl |= ((pattern << DPTX_PHYIF_CTRL_TPS_SEL_SHIFT) &
		      DPTX_PHYIF_CTRL_TPS_SEL_MASK);
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

void dptx_phy_enable_xmit(struct dp_ctrl *dptx, uint32_t lanes, bool enable)
{
	uint32_t phyifctrl;
	uint32_t mask = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	HISI_FB_INFO("[DP] lanes=%d, enable=%d\n", lanes, enable);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);

	switch (lanes) {
	/* This case (value 4) is not terminated by a 'break' statement */
	case 4:
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(3);
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(2);
	/* This case (value 2) is not terminated by a 'break' statement */
	case 2:
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(1);
	case 1:
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	default:
		HISI_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		break;
	}

	if (enable)
		phyifctrl |= mask;
	else
		phyifctrl &= ~mask;

	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

int dptx_phy_rate_to_bw(uint8_t rate)
{
	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		return DP_LINK_BW_1_62;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		return DP_LINK_BW_2_7;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		return DP_LINK_BW_5_4;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		return DP_LINK_BW_8_1;
	default:
		HISI_FB_ERR("[DP] Invalid rate 0x%x\n", rate);
		return -EINVAL;
	}
}

int dptx_bw_to_phy_rate(uint8_t bw)
{
	switch (bw) {
	case DP_LINK_BW_1_62:
		return DPTX_PHYIF_CTRL_RATE_RBR;
	case DP_LINK_BW_2_7:
		return DPTX_PHYIF_CTRL_RATE_HBR;
	case DP_LINK_BW_5_4:
		return DPTX_PHYIF_CTRL_RATE_HBR2;
	case DP_LINK_BW_8_1:
		return DPTX_PHYIF_CTRL_RATE_HBR3;
	default:
		HISI_FB_ERR("[DP] Invalid bw 0x%x\n", bw);
		return -EINVAL;
	}
}
/*lint -restore*/
