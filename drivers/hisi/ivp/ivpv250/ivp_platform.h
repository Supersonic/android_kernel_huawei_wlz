#ifndef _IVP_PLATFORM_H_
#define _IVP_PLATFORM_H_

#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include "ivp.h"

#define LISTENTRY_SIZE         (0x00600000)

#define IVP_SEC_BUFF_SIZE      0x200000
#define IVP_SEC_SHARE_ADDR     0x11600000
#define IVP_SEC_LOG_ADDR       0x11680000

#define IVP_CLK_LEVEL_DEFAULT         0
#define IVP_CLK_LEVEL_ULTRA_LOW       1
#define IVP_CLK_LEVEL_LOW             2
#define IVP_CLK_LEVEL_MEDIUM          3
#define IVP_CLK_LEVEL_HIGH            4

#define GIC_IRQ_CLEAR_REG      (0xEA0001A4)//(0xe82b11a4)
#define IVP_CORE_RD_QOS_MODE   (0x000C)
#define IVP_CORE_WR_QOS_MODE   (0x008C)
#define IVP_IDMA_RD_QOS_MODE   (0x010C)
#define IVP_IDMA_WR_QOS_MODE   (0x018C)
#define IVP_SYS_QOS_CFG_VALUE  (0x30000)
#define NOC_IVP_QOS_CFG_VALUE  (0x02)
#define KIRIN990_CS2_ID        (0x36903100)
#define KIRIN990_CS1_ID        (0x36901100)

#define READ_BACK_IVP_SYS_QOS_CFG_ERROR        (0x1)
#define READ_BACK_IVP_CORE_RD_QOS_MODE_ERROR   (0x2)
#define READ_BACK_IVP_CORE_WR_QOS_MODE_ERROR   (0x3)
#define READ_BACK_IVP_IDMA_RD_QOS_MODE_ERROR   (0x4)
#define READ_BACK_IVP_IDMA_WR_QOS_MODE_ERROR   (0x5)

struct ivp_iomem_res {
	char __iomem *cfg_base_addr;
	char __iomem *pctrl_base_addr;
	char __iomem *pericrg_base_addr;
	char __iomem *gic_base_addr;
	char __iomem *noc_ivp_base_addr;
};

struct ivp_device {
	struct ivp_iomem_res io_res;
	struct clk *clk;
	unsigned int clk_rate;
	unsigned int middle_clk_rate;
	unsigned int low_clk_rate;
	unsigned int ultra_low_clk_rate;
	unsigned int clk_level;
	unsigned int clk_usrsetrate;
	unsigned int lowfrq_pd_clk_rate;
	unsigned int lowtemp_clk_rate;
	unsigned int wdg_irq;
	atomic_t wdg_sleep;
	unsigned int dwaxi_dlock_irq;
	struct semaphore wdg_sem;
	int sect_count;
	struct ivp_sect_info *sects;
	struct ivp_sect_info *sec_sects;
	struct dentry *debugfs;
	struct miscdevice device;
	struct regulator *regulator;
	struct regulator *ivp_media2_regulator;
	unsigned long smmu_pgd_base;
	struct ivp_smmu_dev *smmu_dev;
	atomic_t accessible;
	atomic_t poweron_access;
	atomic_t poweron_success;
	void *vaddr_memory;

	int ivp_meminddr_len;
	unsigned int dynamic_mem_size;
	unsigned int dynamic_mem_section_size;
	unsigned int ivp_sec_support;
	unsigned int ivp_secmode;
	int ivp_sec_buff_fd;
	struct platform_device *ivp_pdev;
};

int ivp_poweron_pri(struct ivp_device *ivp_devp);
int ivp_poweron_remap(struct ivp_device *ivp_devp);
int ivp_poweroff_pri(struct ivp_device *ivp_devp);
int ivp_init_pri(struct platform_device *pdev, struct ivp_device *ivp_devp);
void ivp_deinit_pri(struct ivp_device *ivp_devp);
int ivp_change_clk(struct ivp_device *ivp_devp, unsigned int level);
int ivp_init_resethandler(struct ivp_device *pdev);
void ivp_deinit_resethandler(struct ivp_device *pdev);
int ivp_check_resethandler(struct ivp_device *pdev);
int  ivp_sec_loadimage(struct ivp_device *pdev);
void ivp_dev_hwa_enable(void);
void ivp_hw_enable_reset(struct ivp_device *devp);
u32 noc_ivp_reg_read(struct ivp_device *ivp_dev, unsigned int off);
void noc_ivp_reg_write(struct ivp_device *ivp_dev, unsigned int off, u32 val);
void ivp_set_qos_cfg(struct ivp_device *dev);
int ivp_read_qos_cfg(struct ivp_device *dev);

#endif /* _IVP_PLATFORM_H_ */
