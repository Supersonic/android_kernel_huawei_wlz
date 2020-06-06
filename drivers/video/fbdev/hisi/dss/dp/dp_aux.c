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

#include "dp_aux.h"
#include "core.h"
#include "../hisi_fb_def.h"

#define DPTX_NO_DEBUG_REG
#ifdef CONFIG_HISI_FB_V510
static int dptx_handle_aux_reply(struct dp_ctrl *dptx)
{
	uint32_t auxsts;
	uint32_t status;
	uint32_t auxm;
	uint32_t br;
	int count;

	count = 0;
	while(1) {
		auxsts = dptx_readl(dptx, DPTX_AUX_STS);

		if (!(auxsts & DPTX_AUX_STS_REPLY_RECEIVED))
			break;

		count ++;
		if (count > 5000)
			return -ETIMEDOUT;

		udelay(1);
	};

	/*lint -e648*/
	auxsts = dptx_readl(dptx, DPTX_AUX_STS);

	status = (auxsts & DPTX_AUX_STS_STATUS_MASK) >>
		DPTX_AUX_STS_STATUS_SHIFT;

	auxm = (auxsts & DPTX_AUX_STS_AUXM_MASK) >>
		DPTX_AUX_STS_AUXM_SHIFT;
	br = (auxsts & DPTX_AUX_STS_BYTES_READ_MASK) >>
		DPTX_AUX_STS_BYTES_READ_SHIFT;
	/*lint -e648*/
	dptx_dbg_aux(dptx, "%s: 0x%08x: sts=%d, auxm=%d, br=%d, replyrcvd=%d, ",
		     "replyerr=%d, timeout=%d, disconn=%d\n",
		     __func__, auxsts, status, auxm, br,
		     !!(auxsts & DPTX_AUX_STS_REPLY_RECEIVED),
		     !!(auxsts & DPTX_AUX_STS_REPLY_ERR),
		     !!(auxsts & DPTX_AUX_STS_TIMEOUT),
		     !!(auxsts & DPTX_AUX_STS_SINK_DWA));

	switch (status) {
	case DPTX_AUX_STS_STATUS_ACK:
		dptx_dbg_aux(dptx, "%s: DPTX_AUX_STS_STATUS_ACK\n", __func__);
		break;
	case DPTX_AUX_STS_STATUS_NACK:
		dptx_dbg_aux(dptx, "%s: DPTX_AUX_STS_STATUS_NACK\n", __func__);
		break;
	case DPTX_AUX_STS_STATUS_DEFER:
		dptx_dbg_aux(dptx, "%s: DPTX_AUX_STS_STATUS_DEFER\n", __func__);
		break;
	case DPTX_AUX_STS_STATUS_I2C_NACK:
		dptx_dbg_aux(dptx, "%s: DPTX_AUX_STS_STATUS_I2C_NACK\n",
			     __func__);
		break;
	case DPTX_AUX_STS_STATUS_I2C_DEFER:
		dptx_dbg_aux(dptx, "%s: DPTX_AUX_STS_STATUS_I2C_DEFER\n",
			     __func__);
		break;
	default:
		HISI_FB_ERR("[DP] Invalid AUX status 0x%x\n", status);
		break;
	}

	dptx->aux.data[0] = dptx_readl(dptx, DPTX_AUX_DATA0);
	dptx->aux.data[1] = dptx_readl(dptx, DPTX_AUX_DATA1);
	dptx->aux.data[2] = dptx_readl(dptx, DPTX_AUX_DATA2);
	dptx->aux.data[3] = dptx_readl(dptx, DPTX_AUX_DATA3);
	dptx->aux.sts = auxsts;

	return 0;
}
#endif
/*lint -save -e* */
static void dptx_aux_clear_data(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	dptx_writel(dptx, DPTX_AUX_DATA0, 0);
	dptx_writel(dptx, DPTX_AUX_DATA1, 0);
	dptx_writel(dptx, DPTX_AUX_DATA2, 0);
	dptx_writel(dptx, DPTX_AUX_DATA3, 0);
}

static int dptx_aux_read_data(struct dp_ctrl *dptx, uint8_t *bytes, uint32_t len)
{
	uint32_t i;
	uint32_t *data = NULL;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	data = dptx->aux.data;
	for (i = 0; i < len; i++) {
		if ((i/4) > 3)
			return -EINVAL;

		bytes[i] = (data[i / 4] >> ((i % 4) * 8)) & 0xff;
	}

	return len;
}

static int dptx_aux_write_data(struct dp_ctrl *dptx, uint8_t const *bytes,
			       uint32_t len)
{
	uint32_t i;
	uint32_t data[4];
	memset(data, 0, sizeof(uint32_t) * 4);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		if ((i/4) > 3)
			return -EINVAL;

		data[i / 4] |= (bytes[i] << ((i % 4) * 8));
	}

	dptx_writel(dptx, DPTX_AUX_DATA0, data[0]);
	dptx_writel(dptx, DPTX_AUX_DATA1, data[1]);
	dptx_writel(dptx, DPTX_AUX_DATA2, data[2]);
	dptx_writel(dptx, DPTX_AUX_DATA3, data[3]);

	return len;
}

static int dptx_aux_rw(struct dp_ctrl *dptx,
		       bool rw,
		       bool i2c,
		       bool mot,
		       bool addr_only,
		       uint32_t addr,
		       uint8_t *bytes,
		       uint32_t len)
{
	int retval;
	int tries = 0;
	uint32_t auxcmd;
	uint32_t type;
	uint32_t status;
	uint32_t br;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

again:
	mdelay(1);
	tries++;
	if (tries > 100) {
		HISI_FB_ERR("[DP] AUX exceeded retries\n");
		return -EAGAIN;
	}

	HISI_FB_DEBUG("[DP] device addr=0x%08x, len=%d, try=%d\n",
					addr, len, tries);

	if ((len > 16) || (len == 0)) {
		 HISI_FB_ERR("[DP] AUX read/write len must be 1-15, len=%d\n", len);
		return -EINVAL;
	}

	type = rw ? DPTX_AUX_CMD_TYPE_READ : DPTX_AUX_CMD_TYPE_WRITE;

	if (!i2c)
		type |= DPTX_AUX_CMD_TYPE_NATIVE;

	if (i2c && mot)
		type |= DPTX_AUX_CMD_TYPE_MOT;

	dptx_aux_clear_data(dptx);

	if (!rw)
		dptx_aux_write_data(dptx, bytes, len);

	auxcmd = ((type << DPTX_AUX_CMD_TYPE_SHIFT) |
		  (addr << DPTX_AUX_CMD_ADDR_SHIFT) |
		  ((len - 1) << DPTX_AUX_CMD_REQ_LEN_SHIFT));

	if (addr_only)
		auxcmd |= DPTX_AUX_CMD_I2C_ADDR_ONLY;

	dptx_writel(dptx, DPTX_AUX_CMD, auxcmd);

#ifdef CONFIG_HISI_FB_V510
	retval = dptx_handle_aux_reply(dptx);
	if (retval)
		return retval;
#else
	retval = dptx_wait(dptx,
			   (atomic_read(&dptx->aux.event) ||
			    atomic_read(&dptx->aux.abort)),
			   DPTX_AUX_TIMEOUT);

	atomic_set(&dptx->aux.event, 0);
#endif

	if (retval == -ETIMEDOUT) {
		HISI_FB_ERR("AUX timed out\n");
		return retval;
	}

	if (retval == -ESHUTDOWN) {
		HISI_FB_INFO("[DP] AUX aborted on driver shutdown\n");
		return retval;
	}

	if (atomic_read(&dptx->aux.abort)) {
		HISI_FB_INFO("[DP] AUX aborted\n");
		return -ETIMEDOUT;
	}

	status = (dptx->aux.sts & DPTX_AUX_STS_STATUS_MASK) >>
		DPTX_AUX_STS_STATUS_SHIFT;

	br = (dptx->aux.sts & DPTX_AUX_STS_BYTES_READ_MASK) >>
		DPTX_AUX_STS_BYTES_READ_SHIFT;

	switch (status) {
	case DPTX_AUX_STS_STATUS_ACK:
		HISI_FB_DEBUG("[DP] AUX Success\n");
		if (br == 0) {
			HISI_FB_ERR("[DP] BR=0, Retry\n");
			dptx_soft_reset(dptx, DPTX_SRST_CTRL_AUX);
			goto again;
		}
		break;
	case DPTX_AUX_STS_STATUS_NACK:
	case DPTX_AUX_STS_STATUS_I2C_NACK:
		HISI_FB_INFO("[DP] AUX Nack\n");
		return -ECONNREFUSED;
	case DPTX_AUX_STS_STATUS_I2C_DEFER:
	case DPTX_AUX_STS_STATUS_DEFER:
		HISI_FB_INFO("[DP] AUX Defer\n");
		goto again;
	default:
		HISI_FB_ERR("[DP] AUX Status Invalid\n");
		dptx_soft_reset(dptx, DPTX_SRST_CTRL_AUX);
		goto again;
	}

	if (rw)
		dptx_aux_read_data(dptx, bytes, len);

	return 0;
}

int dptx_aux_rw_bytes(struct dp_ctrl *dptx,
		      bool rw,
		      bool i2c,
		      uint32_t addr,
		      uint8_t *bytes,
		      uint32_t len)
{
	int retval;
	uint32_t i;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	for (i = 0; i < len; ) {
		uint32_t curlen;

		curlen = min_t(uint32_t, len - i, 16);

		if (!i2c)
			retval = dptx_aux_rw(dptx, rw, i2c, true, false,
					addr + i, &bytes[i], curlen);
		else
			retval = dptx_aux_rw(dptx, rw, i2c, true, false,
					addr, &bytes[i], curlen);

		if (retval) {
			dp_imonitor_set_param_aux_rw(rw, i2c, addr, len, retval);
			return retval;
		}

		i += curlen;
	}

	return 0;
}

int dptx_read_bytes_from_i2c(struct dp_ctrl *dptx,
			     uint32_t device_addr,
			     uint8_t *bytes,
			     uint32_t len)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	return dptx_aux_rw_bytes(dptx, true, true,
				 device_addr, bytes, len);
}

int dptx_i2c_address_only(struct dp_ctrl *dptx, uint32_t device_addr)
{
	uint8_t bytes[1];
	return dptx_aux_rw(dptx, 0, true, false, true,
		device_addr, &bytes[0], 1);
}

int dptx_write_bytes_to_i2c(struct dp_ctrl *dptx,
			    uint32_t device_addr,
			    uint8_t *bytes,
			    uint32_t len)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	return dptx_aux_rw_bytes(dptx, false, true,
				 device_addr, bytes, len);
}

int __dptx_read_bytes_from_dpcd(struct dp_ctrl *dptx,
				uint32_t reg_addr,
				uint8_t *bytes,
				uint32_t len)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	return dptx_aux_rw_bytes(dptx, true, false,
				 reg_addr, bytes, len);
}

int __dptx_write_bytes_to_dpcd(struct dp_ctrl *dptx,
			       uint32_t reg_addr,
			       uint8_t *bytes,
			       uint32_t len)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (bytes == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	return dptx_aux_rw_bytes(dptx, false, false,
				 reg_addr, bytes, len);
}

int __dptx_read_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t *byte)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (byte == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	return __dptx_read_bytes_from_dpcd(dptx, addr, byte, 1);
}

int __dptx_write_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t byte)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	return __dptx_write_bytes_to_dpcd(dptx, addr, &byte, 1);
}

int dptx_read_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t *byte)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (byte == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}
	return __dptx_read_dpcd(dptx, addr, byte);
}

int dptx_write_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t byte)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	return __dptx_write_dpcd(dptx, addr, byte);
}
/*lint -restore*/
