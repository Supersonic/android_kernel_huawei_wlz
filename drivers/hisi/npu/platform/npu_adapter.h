/**
 * @file devdrv_adapt_init.h
 *
 * Copyright(C), 2017 - 2018, Huawei Tech. Co., Ltd. ALL RIGHTS RESERVED.
 *
 * @brief
 *
 * @version 1.0
 *
 */

#ifndef __DEVDRV_ADAPTER_INIT_H__
#define __DEVDRV_ADAPTER_INIT_H__

#include <linux/platform_device.h>
#include <linux/delay.h>
#ifndef BYPASS_SMMU_DEBUG
#include <linux/hisi/hisi_svm.h>
#endif

#include "drv_log.h"
#include "npu_common.h"

int devdrv_plat_pm_powerup(struct devdrv_dev_ctx *dev_ctx, u64 fw_load_addr, u32 is_secure);

int devdrv_plat_pm_open(void);

int devdrv_plat_pm_release(void);

int devdrv_plat_pm_powerdown(void *svm_dev,u32 is_secure, u32 *stage);

int devdrv_plat_res_fw_proc(const struct file *fp, loff_t fsize,
								loff_t pos, u64 fw_load_addr);

int devdrv_plat_res_mailbox_send(void *mailbox, int mailbox_len,
								const void *message, int message_len);

int devdrv_plat_send_ts_ctrl_core(uint32_t core_num);

int devdrv_plat_res_ctrl_core(struct devdrv_dev_ctx *dev_ctx, u32 core_num);

void __iomem* devdrv_plat_sram_remap(struct platform_device *pdev,
								resource_size_t sram_addr, resource_size_t sram_size);

void devdrv_plat_sram_unmap(struct platform_device *pdev, void* sram_addr);

int npu_log_init(void);
int npu_log_stop(void);

// common pm func
int devdrv_plat_powerup_till_npucpu(u64 is_secure);
int devdrv_plat_powerup_till_ts(u32 is_secure, u32 offset);
int devdrv_plat_powerdown_till_npucpu(u32 offset,u32 is_secure);
int devdrv_plat_powerdown_till_down(u32 expect_val, u32 offset);
int devdrv_plat_powerup_tbu(void);
int devdrv_plat_powerdown_tbu(void);

/************define ***********************/


typedef struct tmp_log_buf_header{
	volatile unsigned int buf_read;
	volatile unsigned int buf_len;/*malloc buffer length, head structure not included */
	volatile unsigned int rev[14];
	volatile unsigned int buf_write;
	volatile unsigned int rev2[15];
}tmp_log_buf_header_t;
#define TMP_CACHE_LINE_LEN (64)
#define TMP_LOG_DIR_LEN (128)
#define TMP_LOG_BUF_HEAD sizeof(tmp_log_buf_header_t)

#endif
