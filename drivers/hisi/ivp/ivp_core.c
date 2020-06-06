/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description:  This file is IVP core driver related operations.
 * Create: 2017-2-9
 */
#include "ivp_core.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/dma-buf.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#include <linux/ion-iommu.h>
#endif
#include <linux/syscalls.h>
#if (KERNEL_VERSION(3, 13, 0) > LINUX_VERSION_CODE)
#include <linux/clk-private.h>
#endif
#include <linux/clk-provider.h>
#include <linux/bitops.h>
#include <linux/firmware.h>
#include "securec.h"
#include "ivp.h"
#include "ivp_log.h"
#include "ivp_smmu.h"
#include "ivp_reg.h"
//lint -save -e750 -e753 -specific(-e753) -e750 -e528 -specific(-e528) -e144 -e82 -e64 -e785 -e715 -e712 -e40
//lint -save -e63 -e732 -e42 -e550 -e438 -e834 -e648 -e747 -e778 -e50
//lint -save -e749 -e84 -e866 -e514 -e846 -e737 -e574

#define IVP_WDG_REG_BASE_OFFSET          0x1000
#define IVP_SMMU_REG_BASE_OFFSET         0x40000
#define IVP_IMAGE_SUFFIX                 ".bin"
#define IVP_INIT_SUCCESS                 0x1234ABCD
#define IVP_INIT_RESULT_CHECK_TIMES      0x0A
#define IVP_IMAGE_SUFFIX_LENGTH          (sizeof(IVP_IMAGE_SUFFIX) - 1)
#define IVP_HEAD_NAME                    "IVP:"
#define IVP_HEAD_NAME_LENGTH             (sizeof(IVP_HEAD_NAME) - 1)
#define SMMU_RESET_WAIT_TIME             1000
#define MAX_INDEX                        64
#define SECT_START_NUM                   3

static struct ivp_device ivp_dev;
struct mutex ivp_sec_mem_mutex;
static struct wakeup_source ivp_power_wakelock;
static struct mutex ivp_wake_lock_mutex;
static struct mutex ivp_load_image_mutex;
struct mutex ivp_power_up_off_mutex;
struct mutex ivp_open_release_mutex;

enum {
	IMAGE_SECTION_TYPE_EXEC = 0,
	IMAGE_SECTION_TYPE_DATA,
	IMAGE_SECTION_TYPE_BSS,
};

enum {
	IVP_BOOT_FROM_IRAM = 0,
	IVP_BOOT_FROM_DDR = 1,
};

enum {
	IVP_MEM_SLEEPMODE_NORMAL = 0,
	IVP_MEM_SLEEPMODE = 1,
	IVP_MEM_SLEEPMODE_DEEP = 2,
	IVP_MEM_SLEEPMODE_SHUTDOWN = 4,
	IVP_MEM_SLEEPMODE_MAX,
};

enum {
	IVP_DISABLE = 0,
	IVP_ENABLE  = 1,
	IVP_MODE_INVALID = 2,
};

static int ivp_check_image(const struct firmware *fw)
{
	errno_t ret;
	struct file_header mheader;

	if (sizeof(mheader) > fw->size) {
		ivp_err("(%s):image file mheader is err", __func__);
		return -EINVAL;
	}
	ret = memcpy_s(&mheader, sizeof(mheader), fw->data, sizeof(mheader));
	if (ret != EOK) {
		ivp_err("(%s):memcpy_s fail, ret [%d]", __func__, ret);
		return -EINVAL;
	}

	if (strncmp(mheader.name, IVP_HEAD_NAME, IVP_HEAD_NAME_LENGTH) != 0) {
		ivp_err("(%s):image file header is not for IVP", __func__);
		return -EINVAL;
	}

	if (fw->size != mheader.image_size) {
		ivp_err("image file request_firmware size 0x%zx is not equal to mheader size 0x%08x",
			fw->size, mheader.image_size);
		return -EINVAL;
	}
	return EOK;
}

static int ivp_get_validate_section_info(const struct firmware *fw,
	struct image_section_header *psect_header, unsigned int index)
{
	int offset;
	errno_t ret;
	if (index > MAX_INDEX) {
		ivp_err("%s, index = %d exceed max num!", __func__, index);
		return -EINVAL;
	}

	if (!psect_header) {
		ivp_err("%s,the input para section_header is NULL!", __func__);
		return -EINVAL;
	}

	offset = sizeof(struct file_header);
	offset += sizeof(struct image_section_header) * index;
	if ((offset + sizeof(struct image_section_header)) > fw->size) {
		ivp_err("(%s):image index  is err", __func__);
		return -EINVAL;
	}
	ret = memcpy_s(psect_header,
		sizeof(struct image_section_header),
		fw->data + offset,
		sizeof(struct image_section_header));
	if (ret != EOK) {
		ivp_err("(%s):memcpy_s fail, ret [%d]", __func__, ret);
		return -EINVAL;
	}

	if ((psect_header->offset + psect_header->size) > fw->size) {
		ivp_err("%s,get invalid offset 0x%x", __func__,
			psect_header->offset);
		return -EINVAL;
	}

	/* 0, 1,2,3 represent array index*/
	if ((psect_header->vaddr >= ivp_dev.sects[0].ivp_addr &&
		(psect_header->vaddr < (ivp_dev.sects[0].ivp_addr + ivp_dev.sects[0].len))) ||
		(psect_header->vaddr >= ivp_dev.sects[1].ivp_addr &&
		(psect_header->vaddr < (ivp_dev.sects[1].ivp_addr + ivp_dev.sects[1].len))) ||
		(psect_header->vaddr >= ivp_dev.sects[2].ivp_addr &&
		(psect_header->vaddr < (ivp_dev.sects[2].ivp_addr + ivp_dev.sects[2].len))) ||
		(psect_header->vaddr >= ivp_dev.sects[3].ivp_addr &&
		(psect_header->vaddr < (ivp_dev.sects[3].ivp_addr + ivp_dev.sects[3].len)))) {
		return EOK;
	}

	ivp_err("%s,get invalid addr", __func__);
	return -EINVAL;
}

void *ivp_vmap(phys_addr_t paddr, size_t size, unsigned int *offset)
{
	unsigned int i;
	void *vaddr = NULL;
	pgprot_t pgprot;
	unsigned int pages_count;
	struct page **pages = NULL;

	*offset = paddr & ~PAGE_MASK;
	paddr &= PAGE_MASK;
	pages_count = PAGE_ALIGN(size + *offset) / PAGE_SIZE;

	pages = kzalloc(sizeof(struct page *) * pages_count, GFP_KERNEL);
	if (!pages)
		return NULL;

	pgprot = pgprot_writecombine(PAGE_KERNEL);

	for (i = 0; i < pages_count; i++) {
		*(pages + i) = phys_to_page(
			(phys_addr_t)(paddr + PAGE_SIZE * i));
	}

	vaddr = vmap(pages, pages_count, VM_MAP, pgprot);
	kfree(pages);
	if (!vaddr)
		return NULL;

	return (char *)vaddr + *offset;
}

bool is_ivp_in_secmode(void)
{
	return ((ivp_dev.ivp_sec_support == 1) &&
		(ivp_dev.ivp_secmode == SECURE_MODE));
}

static int ivp_load_section(const struct firmware *fw,
	struct image_section_header image_sect)
{
	unsigned int i = 0;
	unsigned int offset = 0;
	unsigned int *source = NULL;
	unsigned char type = 0;
	unsigned long ivp_ddr_addr = 0;
	unsigned int *mem_addr = NULL;
	void *mem = NULL;
	bool ddr_flag = false;
	errno_t ret;

	source = (unsigned int *)(fw->data + image_sect.offset);
	type = image_sect.type;
	if (image_sect.vaddr >= ivp_dev.sects[SECT_START_NUM].ivp_addr &&
		(image_sect.vaddr <= (ivp_dev.sects[SECT_START_NUM].ivp_addr +
			ivp_dev.sects[SECT_START_NUM].len))) {
		ddr_flag = true;
	} else {
		ddr_flag = false;
	}
	switch (type) {
	case IMAGE_SECTION_TYPE_EXEC:
	case IMAGE_SECTION_TYPE_DATA:
		if (ddr_flag == true) {
			ivp_ddr_addr = (ivp_dev.sects[SECT_START_NUM].acpu_addr << IVP_MMAP_SHIFT) +
				image_sect.vaddr - ivp_dev.sects[SECT_START_NUM].ivp_addr;
			mem = ivp_vmap(ivp_ddr_addr, image_sect.size, &offset);
		} else {
			mem = ioremap_nocache(image_sect.vaddr, image_sect.size);
		}

		if (!mem) {
			ivp_err("Can't map base address");
			return -EINVAL;
		}
		mem_addr = (unsigned int *)mem;

		if (ddr_flag == true) {
			ret = memcpy_s(mem_addr, image_sect.size, source, image_sect.size);
			if (ret != EOK) {
				ivp_err("(%s):memcpy_s fail, ret [%d]",
					__func__, ret);
				vunmap(mem - offset);
				return -EINVAL;
			}
		} else {
			for (i = 0; i < image_sect.size / 4; i++) /* 4 stand for byte wide*/
				*(mem_addr + i) = *(source + i);
		}
		break;

	case IMAGE_SECTION_TYPE_BSS:
		break;

	default:
		ivp_err("Unsupported section type %d", type);
		return -EINVAL;
	}

	if (mem) {
		if (ddr_flag == true)
			vunmap(mem - offset);
		else
			iounmap(mem);
	}
	return EOK;
}

static int ivp_load_firmware(const char *filename, const struct firmware *fw)
{
	int i;
	struct file_header mheader;
	errno_t ret;

	ret = memcpy_s(&mheader, sizeof(mheader), fw->data, sizeof(mheader));
	if (ret != EOK) {
		ivp_err("(%s):memcpy_s fail, ret [%d]", __func__, ret);
		return -EINVAL;
	}
	ivp_info("Begin to loading image %s, section counts 0x%x...",
				filename, mheader.sect_count);
	for (i = 0; i < mheader.sect_count; i++) {
		struct image_section_header sect;

		if (ivp_get_validate_section_info(fw, &sect, i)) {
			ivp_err("get section %d fails ...", i);
			return -EINVAL;
		}

		if (ivp_load_section(fw, sect)) {
			ivp_err("load section %d fails ...", i);
			return -EINVAL;
		}
	}
	return EOK;
}

static int ivp_load_image(const char *name)
{
	int ret;
	const struct firmware *firmware = NULL;
	struct device *dev = NULL;

	if (!name) {
		ivp_err("ivp image file name is invalid in function %s",
			__func__);
		return -EINVAL;
	}

	dev = ivp_dev.device.this_device;
	if (!dev) {
		ivp_err("ivp miscdevice element struce device is null");
		return -EINVAL;
	}
	ret = request_firmware(&firmware, name, dev);
	if (ret) {
		ivp_err("request_firmware return error value %d for file (%s)",
			ret, name);
		return ret;
	}

	ret = ivp_check_image(firmware);
	if (ret != 0) {
		release_firmware(firmware);
		ivp_err("check ivp image %s fail and return value 0x%x ",
			name, ret);
		return ret;
	}

	ret = ivp_load_firmware(name, firmware);
	release_firmware(firmware);
	return ret;
}

/*lint -save -e30 -e31 -e64 -e142 -e438 -e529
-e530 -e550 -e695 -e712 -e713 -e715 -e732 -e744
-e747 -e774 -e778 -e785 -e838 -e1058 -e1564*/

inline void ivp_reg_write(unsigned int off, u32 val)
{
	char __iomem *reg = ivp_dev.io_res.cfg_base_addr + off;

	writel(val, reg);
}

/* read ivp cfg reg */
inline u32 ivp_reg_read(unsigned int off)
{
	char __iomem *reg = ivp_dev.io_res.cfg_base_addr + off;
	u32 val = readl(reg);
	return val;
}

inline void ivp_pericrg_reg_write(unsigned int off, u32 val)
{
	char __iomem *reg = ivp_dev.io_res.pericrg_base_addr + off;

	writel(val, reg);
}

/* read ivp cfg reg */
inline u32 ivp_pericrg_reg_read(unsigned int off)
{
	char __iomem *reg = ivp_dev.io_res.pericrg_base_addr + off;
	u32 val = readl(reg);
	return val;
}

/******************************************************
 *     read/write watch dog reg need unlock first     *
 *****************************************************/
inline void ivp_wdg_reg_write(unsigned int off, u32 val)
{
	char __iomem *reg = ivp_dev.io_res.cfg_base_addr + IVP_WDG_REG_BASE_OFFSET + off;

	writel(val, reg);
}

inline u32 ivp_wdg_reg_read(unsigned int off)
{
	char __iomem *reg = ivp_dev.io_res.cfg_base_addr + IVP_WDG_REG_BASE_OFFSET + off;
	u32 val = readl(reg);
	return val;
}

inline void ivp_smmu_reg_write(unsigned int off, u32 val)
{
	char __iomem *reg = ivp_dev.io_res.cfg_base_addr + IVP_SMMU_REG_BASE_OFFSET + off;

	writel(val, reg);
}

inline u32 ivp_smmu_reg_read(unsigned int off)
{
	char __iomem *reg = ivp_dev.io_res.cfg_base_addr + IVP_SMMU_REG_BASE_OFFSET + off;
	u32 val = readl(reg);
	return val;
}

inline u32 ivp_pctrl_reg_read(unsigned int off)
{
	char __iomem *reg = ivp_dev.io_res.pctrl_base_addr + off;
	u32 val = readl(reg);
	return val;
}

inline void ivp_gic_reg_write(unsigned int off, u32 val)
{
	char __iomem *reg = ivp_dev.io_res.gic_base_addr;

	writel(val, reg);
}

inline u32 ivp_gic_reg_read(unsigned int off)
{
	char __iomem *reg = ivp_dev.io_res.gic_base_addr;
	u32 val = readl(reg);
	return val;
}

inline void ivp_hw_clr_wdg_irq(void)
{
	/* unlock reg */
	ivp_wdg_reg_write(WDG_REG_OFF_WDLOCK, WDG_REG_UNLOCK_KEY);

	/* clear irq */
	ivp_wdg_reg_write(WDG_REG_OFF_WDINTCLR, 1);

	/* disable irq */
	ivp_wdg_reg_write(WDG_REG_OFF_WDCONTROL, 0);

	/* lock reg */
	ivp_wdg_reg_write(WDG_REG_OFF_WDLOCK, WDG_REG_LOCK_KEY);
}

inline void ivp_hw_set_ocdhalt_on_reset(struct ivp_device *devp, unsigned int mode)
{
	ivp_reg_write(IVP_REG_OFF_OCDHALTONRESET, mode);
}

inline void ivp_hw_set_bootmode(struct ivp_device *devp, unsigned int mode)
{
	u32 val = mode;

	ivp_reg_write(IVP_REG_OFF_STATVECTORSEL, val & 0x01);
}

inline void ivp_hw_clockgate(struct ivp_device *devp, unsigned int state)
{
	u32 val = state;

	ivp_reg_write(IVP_REG_OFF_DSPCORE_GATE, val & 0x01);
}

inline void ivp_hw_disable_reset(struct ivp_device *devp)
{
	ivp_reg_write(IVP_REG_OFF_DSP_CORE_RESET_DIS, 0x04);
	ivp_reg_write(IVP_REG_OFF_DSP_CORE_RESET_DIS, 0x01);
	ivp_reg_write(IVP_REG_OFF_DSP_CORE_RESET_DIS, 0x02);
}

void ivp_hw_runstall(struct ivp_device *devp, unsigned int mode)
{
	u32 val = mode;

	ivp_reg_write(IVP_REG_OFF_RUNSTALL, val & 0x01);
}

static int ivp_hw_query_runstall(struct ivp_device *devp)
{
	return (int)ivp_reg_read(IVP_REG_OFF_RUNSTALL);
}

static void ivp_hw_trigger_NMI(struct ivp_device *devp)
{
	/* risedge triger.triger 0->1->0 */
	ivp_reg_write(IVP_REG_OFF_NMI, 0);
	ivp_reg_write(IVP_REG_OFF_NMI, 1);
	ivp_reg_write(IVP_REG_OFF_NMI, 0);
}

inline void ivp_hw_set_mem_sleepmode(unsigned int off, unsigned int mode, unsigned int mask)
{
	unsigned int old_mode = ivp_reg_read(off);
	u32 val = mask;

	unsigned int new_mode = (old_mode & (~val)) | mode;

	if (new_mode != old_mode) {
		ivp_dbg("mode changed.old:%#x, new:%#x", old_mode, new_mode);
		ivp_reg_write(off, new_mode);
	}
}

static void ivp_hw_set_all_bus_gate_clock(unsigned int mode)
{
	if (mode >= IVP_MODE_INVALID) {
		ivp_err("Invalid bus clk auto gate mode.");
		return;
	}

	if (mode == IVP_ENABLE) {
		ivp_reg_write(IVP_REG_OFF_BUS_GATE_CLOCK,
			IVP_BUS_GATE_CLK_ENABLE);
	} else {
		ivp_reg_write(IVP_REG_OFF_BUS_GATE_CLOCK,
			IVP_BUS_GATE_CLK_DISABLE);
	}
}

static void ivp_hw_set_all_smmu_awake_bypass(unsigned int mode)
{
	if (mode >= IVP_MODE_INVALID) {
		ivp_err("Invalid smmu awakebypass mode.");
		return;
	}

	if (mode == IVP_ENABLE) {
		ivp_reg_write(IVP_REG_OFF_SMMU_AWAKEBYPASS,
			IVP_SMMU_AWAKEBYPASS_ENABLE);
	} else {
		ivp_reg_write(IVP_REG_OFF_SMMU_AWAKEBYPASS,
			IVP_SMMU_AWAKEBYPASS_DISABLE);
	}
}

static void ivp_hw_set_peri_autodiv(unsigned int mode)
{
	unsigned int enable;

	if (mode >= IVP_MODE_INVALID) {
		ivp_err("Invalid set peri autodiv mode");
		return;
	}

	ivp_info("set mode to:%d", mode);
	enable = ivp_pericrg_reg_read(PERICRG_REG_OFF_PERI_AUTODIV0);
	if (mode == IVP_ENABLE) {
		ivp_pericrg_reg_write(PERICRG_REG_OFF_PEREN6, GATE_CLK_AUTODIV_IVP);
		ivp_pericrg_reg_write(PERICRG_REG_OFF_PERI_AUTODIV0,
				enable &
				~(IVP_DW_AXI_M2_ST_BYPASS | IVP_DW_AXI_M1_ST_BYPASS |
				IVP_PWAITMODE_BYPASS | IVP_DIV_AUTO_REDUCE_BYPASS));
	} else {
		ivp_pericrg_reg_write(PERICRG_REG_OFF_PERDIS6, GATE_CLK_AUTODIV_IVP);
	}
}

inline int ivp_hw_query_waitmode(struct ivp_device *devp)
{
	return (int)ivp_reg_read(IVP_REG_OFF_PWAITMODE);
}

static void ivp_dev_set_dynamic_clk(unsigned int mode)
{
	if (mode >= IVP_MODE_INVALID) {
		ivp_err("Invalid set dynamic clk mode");
		return;
	}

	if (mode == IVP_ENABLE) {
		/* bus gate clock enable. */
		ivp_hw_set_all_bus_gate_clock(IVP_ENABLE);
		/* pericrg. */
		ivp_hw_set_peri_autodiv(IVP_ENABLE);
		/* smmu bypass enable. */
		ivp_hw_set_all_smmu_awake_bypass(IVP_DISABLE);
	} else {
		/* smmu bypass disable. */
		ivp_hw_set_all_smmu_awake_bypass(IVP_ENABLE);
		/* pericrg. */
		ivp_hw_set_peri_autodiv(IVP_DISABLE);
		/* bus gate clock disable. */
		ivp_hw_set_all_bus_gate_clock(IVP_DISABLE);
	}
}

void ivp_dump_status(struct ivp_device *pdev)
{
	unsigned int cur_pc;
	unsigned int waiti;
	unsigned int runstall;
	unsigned int binterrupt;

	if (!pdev) {
		ivp_err("%s, pdev is NULL!", __func__);
		return;
	}

	if (atomic_read(&pdev->poweron_success) != 0) {
		ivp_err("%s,ivp has powered off!", __func__);
		return;
	}

	waiti = (u32)ivp_hw_query_waitmode(pdev);
	runstall = (u32)ivp_hw_query_runstall(pdev);
	binterrupt = ivp_reg_read(IVP_REG_OFF_BINTERRUPT);
	/* get pc, wfi, runstall. interrupt, or others status */
	ivp_reg_write(IVP_REG_OFF_OCDHALTONRESET, IVP_DEBUG_ENABLE);
	cur_pc = ivp_reg_read(IVP_REG_OFF_IVP_DEBUG_SIG_PC);

	ivp_err("Dsp curr pc = 0x%x, waiti = 0x%x, runstall = 0x%x, bint = 0x%x",
		cur_pc, waiti, runstall, binterrupt);
}

static int ivp_dev_poweron(struct ivp_device *devp)
{
	int ret = 0;

	mutex_lock(&ivp_wake_lock_mutex);
	if (!ivp_power_wakelock.active) {
		__pm_stay_awake(&ivp_power_wakelock);
		ivp_info("%s ivp power on enter, wake lock\n", __func__);
	}
	mutex_unlock(&ivp_wake_lock_mutex);/*lint !e456*/

	ret = ivp_poweron_pri(devp);
	if (ret) {
		ivp_err("power on pri setting failed [%d]!", ret);
		goto err_ivp_poweron_pri;
	}

	/* set auto gate clk etc. */
	if (devp->ivp_secmode == SECURE_MODE)
		ivp_dev_set_dynamic_clk(IVP_DISABLE);
	else
		ivp_dev_set_dynamic_clk(IVP_ENABLE);

	ret = ivp_poweron_remap(devp);
	if (ret) {
		ivp_err("power on remap setting failed [%d]!", ret);
		goto err_ivp_poweron_remap;
	}

	/* After reset, enter running mode */
	ivp_hw_set_ocdhalt_on_reset(devp, 0);

	/* Put ivp in stall mode */
	ivp_hw_runstall(devp, IVP_RUNSTALL_STALL);
	/* Reset ivp core */
	ivp_hw_enable_reset(devp);

	/* Boot from IRAM. */
	ivp_hw_set_bootmode(devp, IVP_BOOT_FROM_IRAM);

	/* Disable system reset, let ivp core leave from reset */
	ivp_hw_disable_reset(devp);

	return ret;/*lint !e454*/

err_ivp_poweron_remap:
	ivp_poweroff_pri(devp);

err_ivp_poweron_pri:
	mutex_lock(&ivp_wake_lock_mutex);
	if (ivp_power_wakelock.active) {
		__pm_relax(&ivp_power_wakelock);
		ivp_err("%s ivp power on failed, wake unlock\n", __func__);
	}
	mutex_unlock(&ivp_wake_lock_mutex);/*lint !e456*/

	return ret;/*lint !e454*/
}

static void ivp_dev_poweroff(struct ivp_device *devp)
{
	int ret = 0;

	ret = ivp_poweroff_pri(devp);
	if (ret)
		ivp_err("power on private setting failed [%d]!", ret);

	mutex_lock(&ivp_wake_lock_mutex);
	if (ivp_power_wakelock.active) {
		__pm_relax(&ivp_power_wakelock);
		ivp_info("%s ivp power off, wake unlock\n", __func__);
	}
	mutex_unlock(&ivp_wake_lock_mutex);/*lint !e456*/
}

static void ivp_dev_run(struct ivp_device *devp)
{
	int i = 0;

	if (ivp_hw_query_runstall(devp) == IVP_RUNSTALL_RUN)
		return;

	ivp_hw_runstall(devp, IVP_RUNSTALL_RUN);

	while (ivp_reg_read(IVP_REG_OFF_RESEVER_REG) != IVP_INIT_SUCCESS && i < IVP_INIT_RESULT_CHECK_TIMES) {
		udelay(100);
		i++;
		if (i >= IVP_INIT_RESULT_CHECK_TIMES) {
			ivp_err("%s ivp init fails value:0x%x", __func__, ivp_reg_read(IVP_REG_OFF_RESEVER_REG));
			return;
		}
	}

	ivp_info("%s ivp init success value:0x%x i:%d!", __func__, ivp_reg_read(IVP_REG_OFF_RESEVER_REG), i);
}

static void ivp_dev_stop(struct ivp_device *devp)
{
	ivp_dbg("enter");
	ivp_hw_runstall(devp, IVP_RUNSTALL_STALL);
}

static long ivp_dev_suspend(struct ivp_device *devp)
{
	unsigned long irq_status = 0;
	u32 wfi = 0;
	u32 binterrupt = 0;
	u32 wdg_enable = 0;

	ivp_dbg("enter %s", __func__);

	local_irq_save(irq_status);
	wfi = ivp_reg_read(IVP_REG_OFF_PWAITMODE);
	binterrupt = ivp_reg_read(IVP_REG_OFF_BINTERRUPT);
	wdg_enable = ivp_wdg_reg_read(WDG_REG_OFF_WDCONTROL);
	if (wfi == 1 && binterrupt == 0 && wdg_enable == 0) { // ivp reg read type is 1
		ivp_hw_runstall(devp, IVP_RUNSTALL_STALL);
		local_irq_restore(irq_status);
		return EOK;
	}
	local_irq_restore(irq_status);

	if (wfi == 1 && (binterrupt != 0 || wdg_enable != 0))
		ivp_warn("Suspend on wrong status, binterrupt=%u wdgenable=%u",
			binterrupt, wdg_enable);
	return -EINVAL;
}

static void ivp_dev_resume(struct ivp_device *devp)
{
	ivp_dbg("enter");

	if (ivp_hw_query_runstall(devp) == IVP_RUNSTALL_RUN)
		return;

	ivp_hw_runstall(devp, IVP_RUNSTALL_RUN);
}

static int ivp_dev_keep_on_wdg(struct ivp_device *devp)
{
	if (down_interruptible(&ivp_dev.wdg_sem)) {
		ivp_info("interrupt");
		return -EINTR;
	}

	if (atomic_read(&devp->wdg_sleep)) {
		ivp_info("watchdog sleeped");
		return -EAGAIN;
	}

	return EOK;
}

static void ivp_dev_sleep_wdg(struct ivp_device *devp)
{
	atomic_set(&devp->wdg_sleep, 1);
	up(&devp->wdg_sem);
}

static int ivp_dev_smmu_reset(void)
{
	unsigned int status = 0;
	int ret = 0;
	int count = SMMU_RESET_WAIT_TIME;

	ivp_info("enter");
	ivp_reg_write(IVP_REG_OFF_SMMU_RST_EN, BIT_SMMU_CRST_EN | BIT_SMMU_BRST_EN);
	udelay(10);
	ivp_reg_write(IVP_REG_OFF_SMMU_RST_DIS, BIT_SMMU_CRST_DIS | BIT_SMMU_BRST_DIS);

	while (count--) {
		status = ivp_reg_read(IVP_REG_OFF_SMMU_RST_ST);
		if ((status & BIT_SMMU_CRST_DIS) && (status & BIT_SMMU_BRST_DIS))
			break;
		udelay(1);
	}

	if (count <= 0) {
		ret = -EINVAL;
		ivp_err("Reset smmu fail.");
	}

	return ret;
}

static int ivp_dev_smmu_init(struct ivp_device *pdev)
{
	struct ivp_smmu_dev *smmu_dev = pdev->smmu_dev;
	int ret;

	if (!smmu_dev) {
		ivp_err("Ivp smmu dev is NULL.");
		return -ENODEV;
	}

	/* reset smmu */
	ret = ivp_dev_smmu_reset();
	if (ret)
		ivp_warn("Reset ret [%d]", ret);

	/* enable smmu */
	ret = ivp_smmu_trans_enable(smmu_dev);
	if (ret)
		ivp_warn("Enable trans ret [%d]", ret);

	return ret;
}

static int ivp_dev_smmu_deinit(struct ivp_device *pdev)
{
	struct ivp_smmu_dev *smmu_dev = pdev->smmu_dev;
	int ret = 0;

	ivp_info("enter");
	if (!smmu_dev) {
		ivp_err("Ivp smmu dev is NULL.");
		return -ENODEV;
	}

	ret = ivp_smmu_trans_disable(smmu_dev);
	if (ret)
		ivp_err("Enable trans failed.[%d]", ret);

	return ret;
}

static int ivp_dev_smmu_invalid_tlb(void)
{
	struct ivp_smmu_dev *smmu_dev = ivp_dev.smmu_dev;
	int ret = 0;

	ivp_info("enter");
	if (!smmu_dev) {
		ivp_err("Ivp smmu dev is NULL.");
		return -ENODEV;
	}

	ret = ivp_smmu_invalid_tlb(smmu_dev, IVP_SMMU_CB_VMID_NS);
	if (ret)
		ivp_err("invalid tbl fail.[%d]", ret);

	return ret;
}

static irqreturn_t ivp_wdg_irq_handler(int irq, void *dev_id)
{
	struct ivp_device *pdev = (struct ivp_device *)dev_id;

	ivp_warn("=========WDG IRQ Trigger=============");
	//Clear IRQ
	ivp_hw_clr_wdg_irq();

	up(&pdev->wdg_sem);
	ivp_warn("=========WDG IRQ LEAVE===============");

	ivp_dsm_error_notify(DSM_IVP_WATCH_ERROR_NO);

	return IRQ_HANDLED;
}

static void ivp_parse_dwaxi_info(void)
{
	u32 val, bits_val;

	val = ivp_pctrl_reg_read(PCTRL_REG_OFF_PERI_STAT4);

	ivp_dbg("pctrl reg:%#x = %#x", PCTRL_REG_OFF_PERI_STAT4, val);

	bits_val = (val & BIT(18)) >> 18; // right shift 18 bit
	ivp_warn("BIT[18] : %#x", bits_val);

	bits_val = (val & BIT(17)) >> 17; // right shift 17 bit
	if (bits_val == 0) {
		ivp_warn("BIT[17] : %#x, dlock write", bits_val);
	} else {
		ivp_warn("BIT[17] : %#x, dlock read", bits_val);
	}

	bits_val = (val & 0x1e000) >> 13; /* 0x1e000 and 13 is mask*/
	ivp_warn("ivp32 dlock id[%#x]", bits_val);

	bits_val = (val & 0x1c00) >> 10; /* 0x1c00 and 10 is mask*/
	ivp_warn("ivp32 dlock slv[%#x]", bits_val);

	val = ivp_reg_read(IVP_REG_OFF_SMMU_RST_ST);
	ivp_warn("ivp reg:%#x = %#x", IVP_REG_OFF_SMMU_RST_ST, val);

	val = ivp_reg_read(IVP_REG_OFF_TIMER_WDG_RST_STATUS);
	ivp_warn("ivp:%#x = %#x", IVP_REG_OFF_TIMER_WDG_RST_STATUS, val);

	val = ivp_reg_read(IVP_REG_OFF_DSP_CORE_RESET_STATUS);
	ivp_warn("ivp:%#x = %#x", IVP_REG_OFF_DSP_CORE_RESET_STATUS, val);
}

static irqreturn_t ivp_dwaxi_irq_handler(int irq, void *dev_id __attribute__((unused)))
{
	ivp_warn("==========DWAXI IRQ Trigger===============");
	ivp_warn("dwaxi triggled!SMMU maybe in reset status");
	/* clear dwaxi irq */
	ivp_gic_reg_write(GIC_REG_OFF_GICD_ICENABLERN,
		GIC_REG_GICD_ICENABLERN_VAL);
	ivp_parse_dwaxi_info();
	ivp_warn("==========DWAXI IRQ LEAVE=================");

	ivp_dsm_error_notify(DSM_IVP_DWAXI_ERROR_NO);

	return IRQ_HANDLED;
}

static int ivp_open(struct inode *inode __attribute__((unused)),
		struct file *fd __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;

	mutex_lock(&ivp_open_release_mutex);

	if (atomic_read(&pdev->accessible) == 0) {
		mutex_unlock(&ivp_open_release_mutex);
		ivp_err("maybe ivp dev has been opened!");
		return -EBUSY;
	}

	atomic_dec(&pdev->accessible);

	mutex_unlock(&ivp_open_release_mutex);
	return EOK;
}

static int ivp_poweron(
	struct ivp_device *pdev, const struct ivp_power_up_info *pu_info)
{
	int ret = 0;

	if (atomic_read(&pdev->poweron_access) == 0) {
		ivp_err("maybe ivp dev has power on!");
		return -EBUSY;
	}
	atomic_dec(&pdev->poweron_access);
	atomic_set(&pdev->wdg_sleep, 0);
	sema_init(&pdev->wdg_sem, 0);

	if (pu_info->sec_mode == SECURE_MODE && pdev->ivp_sec_support == 0) {
		ivp_err("ivp don't support secure mode");
		goto err_ivp_dev_poweron;
	}
	pdev->ivp_secmode = pu_info->sec_mode;
	pdev->ivp_sec_buff_fd = pu_info->sec_buff_fd;
	ret = ivp_dev_poweron(pdev);
	if (ret < 0) {
		ivp_err("Failed to power on ivp.");
		goto err_ivp_dev_poweron;
	}

	if (pdev->ivp_secmode == NOSEC_MODE) {
		ret = ivp_dev_smmu_init(pdev);
		if (ret) {
			ivp_err("Failed to init smmu.");
			goto err_ivp_dev_smmu_init;
		}
		ret = request_irq(pdev->dwaxi_dlock_irq, ivp_dwaxi_irq_handler, 0, "ivp_dwaxi_irq", (void *)pdev);
		if (ret) {
			ivp_err("Failed to request dwaxi irq.%d", ret);
			goto err_request_irq_dwaxi;
		}
	}

	ret = request_irq(pdev->wdg_irq, ivp_wdg_irq_handler, 0, "ivp_wdg_irq", (void *)pdev);
	if (ret) {
		ivp_err("Failed to request wdg irq.%d", ret);
		goto err_request_irq_wdg;
	}

	ret = ivp_init_resethandler(pdev);
	if (ret) {
		ivp_err("Failed to init reset handler.");
		goto err_ivp_init_resethandler;
	}

	ivp_info("open ivp device success!");
	atomic_dec(&pdev->poweron_success);

	return ret;

err_ivp_init_resethandler:
	free_irq(pdev->wdg_irq, pdev);
err_request_irq_wdg:
	if (pdev->ivp_secmode == NOSEC_MODE)
		free_irq(pdev->dwaxi_dlock_irq, pdev);
err_request_irq_dwaxi:
	if (pdev->ivp_secmode == NOSEC_MODE)
		ivp_dev_smmu_deinit(pdev);
err_ivp_dev_smmu_init:
	ivp_dev_poweroff(pdev);
err_ivp_dev_poweron:
	pdev->ivp_secmode = NOSEC_MODE;
	atomic_inc(&pdev->poweron_access);
	ivp_dsm_error_notify(DSM_IVP_OPEN_ERROR_NO);

	ivp_info("poweron ivp device fail!");
	return ret;
}

static void ivp_poweroff(struct ivp_device *pdev)
{
	if (atomic_read(&pdev->poweron_success) != 0) {
		ivp_err("maybe ivp dev not poweron success!");
		return;
	}

	ivp_deinit_resethandler(pdev);

	ivp_hw_runstall(pdev, IVP_RUNSTALL_STALL);
	if (ivp_hw_query_runstall(pdev) != IVP_RUNSTALL_STALL)
		ivp_err("Failed to stall ivp.");
	ivp_hw_clr_wdg_irq();

	disable_irq(pdev->wdg_irq);
	free_irq(pdev->wdg_irq, pdev);

	if (pdev->ivp_secmode == NOSEC_MODE) {
		disable_irq(pdev->dwaxi_dlock_irq);
		free_irq(pdev->dwaxi_dlock_irq, pdev);
		ivp_dev_smmu_deinit(pdev);
	}

	ivp_dev_poweroff(pdev);

	pdev->ivp_secmode = NOSEC_MODE;
	atomic_inc(&pdev->poweron_access);
	atomic_inc(&pdev->poweron_success);
}

static int ivp_release(struct inode *inode, struct file *fd)
{
	struct ivp_device *pdev = &ivp_dev;

	mutex_lock(&ivp_open_release_mutex);
	if (atomic_read(&pdev->accessible) != 0) {
		mutex_unlock(&ivp_open_release_mutex);
		ivp_err("maybe ivp dev not opened!");
		return -EINVAL;
	}

	mutex_lock(&ivp_power_up_off_mutex);
	ivp_poweroff(pdev);
	mutex_unlock(&ivp_power_up_off_mutex);

	ivp_info("ivp device closed.");

	atomic_inc(&pdev->accessible);

	mutex_unlock(&ivp_open_release_mutex);
	return EOK;
}

static long ioctl_power_up(unsigned long args, struct file *fd)
{
	long ret = 0;
	struct ivp_power_up_info info = {0};

	if (!args || fd == NULL) {
		ivp_err("Invalid input args 0 or fd is null");
		return -EINVAL;
	}

	mutex_lock(&ivp_power_up_off_mutex);
	if (copy_from_user(&info, (void *)(uintptr_t)args, sizeof(info)) != 0) {
		ivp_err("Invalid input param size.");
		mutex_unlock(&ivp_power_up_off_mutex);
		return -EINVAL;
	}

	if (info.sec_mode != SECURE_MODE && info.sec_mode != NOSEC_MODE) {
		ivp_err("Invalid input secMode value:%d", info.sec_mode);
		mutex_unlock(&ivp_power_up_off_mutex);
		return -EINVAL;
	}
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	if (info.sec_buff_fd != -1) {
		ivp_err("Invalid sec buffer fd value:%d for kernel 4.9", info.sec_buff_fd);
		mutex_unlock(&ivp_power_up_off_mutex);
		return -EINVAL;
	}
#else
	if (info.sec_mode == SECURE_MODE && info.sec_buff_fd < 0) {
		ivp_err("Invalid sec buffer fd value:%d for kernel 4.14", info.sec_buff_fd);
		mutex_unlock(&ivp_power_up_off_mutex);
		return -EINVAL;
	}
#endif
	ret = ivp_poweron(&ivp_dev, &info);
	if (!ret) {
		fd->private_data = (void *)&ivp_dev;
	}
	mutex_unlock(&ivp_power_up_off_mutex);

	return ret;
}

static long ioctl_sect_count(unsigned long args)
{
	long ret = 0;
	unsigned int sect_count = ivp_dev.sect_count;

	ivp_info("IOCTL:get sect count:%#x.", sect_count);
	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}
	ret = copy_to_user((void *)(uintptr_t)args, &sect_count, sizeof(sect_count));

	return ret;
}

static long ioctl_sect_info(unsigned long args)
{
	long ret = 0;
	struct ivp_sect_info info;

	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}
	if (copy_from_user(&info, (void __user *)(uintptr_t)args, sizeof(info)) != 0) {
		ivp_err("Invalid input param size.");
		return -EINVAL;
	}

	if (info.index >= ivp_dev.sect_count) {
		ivp_err("index is out of range.index:%u, sec_count:%u",
		info.index, ivp_dev.sect_count);
		return -EINVAL;
	}

	if (ivp_dev.ivp_secmode == SECURE_MODE) {
		ret = copy_to_user((void *)(uintptr_t)args,
			&ivp_dev.sec_sects[info.index],
			sizeof(struct ivp_sect_info));
		ivp_dbg("name:%s, ivp_addr:0x%x, acpu_addr:0x%lx ",
			ivp_dev.sec_sects[info.index].name,
			ivp_dev.sec_sects[info.index].ivp_addr,
			ivp_dev.sec_sects[info.index].acpu_addr);
	} else {
		ret = copy_to_user((void *)(uintptr_t)args,
			&ivp_dev.sects[info.index],
			sizeof(struct ivp_sect_info));
	}

	return ret;
}

static long ioctl_load_firmware(unsigned long args)
{
	long ret = 0;
	struct ivp_image_info info;
	errno_t rc = 0;
	struct ivp_device *pdev = &ivp_dev;

	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}

	if (ivp_hw_query_runstall(pdev) == IVP_RUNSTALL_RUN) {
		ivp_err("Invalid ivp status: ivp alredy run ");
		return -EINVAL;
	}
	mutex_lock(&ivp_load_image_mutex);
	rc = memset_s((char *)&info, sizeof(info), 0, sizeof(info));
	if (rc != EOK) {
		ivp_err("(%s):memcpy_s fail, ret [%d]", __func__, rc);
		mutex_unlock(&ivp_load_image_mutex);
		return -EINVAL;
	}
	if (copy_from_user(&info, (void __user *)(uintptr_t)args, sizeof(info)) != 0) {
		ivp_err("invalid input param size.");
		mutex_unlock(&ivp_load_image_mutex);
		return -EINVAL;
	}
	info.name[sizeof(info.name) - 1] = '\0';
	if ((info.length > (sizeof(info.name) - 1)) ||
		info.length <= IVP_IMAGE_SUFFIX_LENGTH ||
		info.length != strlen(info.name)) {
		ivp_err("image file:but pass param length:%d", info.length);
		mutex_unlock(&ivp_load_image_mutex);
		return -EINVAL;
	}
	if (strcmp((const char *)&info.name[info.length - IVP_IMAGE_SUFFIX_LENGTH],
		IVP_IMAGE_SUFFIX)) {
		ivp_err("image is not bin file");
		mutex_unlock(&ivp_load_image_mutex);
		return -EINVAL;
	}
	if (pdev->ivp_secmode == SECURE_MODE)
		ret = ivp_sec_loadimage(pdev);
	else
		ret = ivp_load_image(info.name);

	mutex_unlock(&ivp_load_image_mutex);
	return ret;
}

static long ioctl_dsp_run(unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;

	if (ivp_check_resethandler(pdev) == 1) {
		ivp_dev_run(pdev);
	} else {
		ivp_err("ivp image not upload.");
		return -EINVAL;
	}

	return EOK;
}

static long ioctl_dsp_suspend(unsigned long args __attribute__((unused)))
{
	long ret;
	struct ivp_device *pdev = &ivp_dev;
	ret = ivp_dev_suspend(pdev);
	if (ret)
		ivp_warn("ivp dev suspend fail");

	return ret;
}

static long ioctl_dsp_resume(unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;

	if (ivp_check_resethandler(pdev) == 1) {
		ivp_dev_resume(pdev);
	} else {
		ivp_err("ivp image not upload.");
		return -EINVAL;
	}

	return EOK;
}

static long ioctl_dsp_stop(unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;
	ivp_dev_stop(pdev);

	return EOK;
}

static long ioctl_query_runstall(unsigned long args)
{
	long ret = 0;
	unsigned int runstall = 0;
	struct ivp_device *pdev = &ivp_dev;

	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}

	runstall = (u32)ivp_hw_query_runstall(pdev);
	ret = copy_to_user((void *)(uintptr_t)args, &runstall, sizeof(runstall));

	return ret;
}

static long ioctl_query_waiti(unsigned long args)
{
	long ret = 0;
	unsigned int waiti = 0;
	struct ivp_device *pdev = &ivp_dev;

	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}

	waiti = (u32)ivp_hw_query_waitmode(pdev);
	ret = copy_to_user((void *)(uintptr_t)args, &waiti, sizeof(waiti));

	return ret;
}

static long ioctl_trigger_nmi(unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;
	ivp_hw_trigger_NMI(pdev);

	return EOK;
}

static long ioctl_watchdog(unsigned long args __attribute__((unused)))
{
	long ret = 0;
	struct ivp_device *pdev = &ivp_dev;
	ret = ivp_dev_keep_on_wdg(pdev);

	return ret;
}

static long ioctl_watchdog_sleep(unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;
	ivp_dev_sleep_wdg(pdev);

	return EOK;
}

static long ioctl_smmu_invalidate_tlb(unsigned long args __attribute__((unused)))
{
	long ret = 0;
	ret = ivp_dev_smmu_invalid_tlb();

	return ret;
}

static long ioctl_bm_init(unsigned long args __attribute__((unused)))
{
	ivp_dev_hwa_enable();
	return EOK;
}

static long ioctl_clk_level(unsigned long args)
{
	long ret;
	unsigned int level = 0;
	struct ivp_device *pdev = &ivp_dev;

	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}

	if (copy_from_user(&level, (void __user *)(uintptr_t)args, sizeof(unsigned int)) != 0) {
		ivp_err("Invalid input param size.");
		return -EINVAL;
	}

	ret = ivp_change_clk(pdev, level);
	if (ret)
		ivp_err("ivp change clk fail");
	return ret;
}

static long ioctl_dump_dsp_status(unsigned long args __attribute__((unused)))
{
	struct ivp_device *pdev = &ivp_dev;
	ivp_dump_status(pdev);

	return EOK;
}

#ifdef IVP_CHIPTYPE_SUPPORT
inline void ivp_write_chip_type_value(unsigned int chip_type)
{
	ivp_reg_write(IVP_REG_OFF_RESEVER_REG, chip_type);
}

static long ioctl_query_chip_type(unsigned long args)
{
	unsigned int chip_type = 0;

	if (!args) {
		ivp_err("Invalid input args 0");
		return -EINVAL;
	}

	if (copy_from_user(&chip_type, (void __user *)(uintptr_t)args, sizeof(unsigned int)) != 0) {
		ivp_err("Invalid input param size.");
		return -EINVAL;
	}

	if ((chip_type == KIRIN990_CS2_ID) || (chip_type == KIRIN990_CS1_ID)) {
		ivp_write_chip_type_value(chip_type);
	} else {
		ivp_err("invalid chiptype value");
		return -EINVAL;
	}

	return EOK;
}
#endif

typedef long (*ivp_ioctl_func_t)(unsigned long args);

typedef long (*ivp_ioctl_func_fd_t)(unsigned long args, struct file *fd);

struct ivp_ioctl_ops {
	unsigned int cmd;
	unsigned char is_need_fd;
	ivp_ioctl_func_t func;
	ivp_ioctl_func_fd_t func_fd;
};

#define IVP_IOCTL_OPS(_cmd, _func) {.cmd = _cmd, \
	.is_need_fd = 0, .func = _func, .func_fd = NULL}

#define IVP_IOCTL_OPS_FD(_cmd, _func) {.cmd = _cmd, \
	.is_need_fd = 1, .func = NULL, .func_fd = _func}


static struct ivp_ioctl_ops ioctl_ops[] = {
	IVP_IOCTL_OPS_FD(IVP_IOCTL_POWER_UP, ioctl_power_up),
	IVP_IOCTL_OPS(IVP_IOCTL_SECTCOUNT, ioctl_sect_count),
	IVP_IOCTL_OPS(IVP_IOCTL_SECTINFO, ioctl_sect_info),
	IVP_IOCTL_OPS(IVP_IOCTL_LOAD_FIRMWARE, ioctl_load_firmware),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_RUN, ioctl_dsp_run),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_SUSPEND, ioctl_dsp_suspend),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_RESUME, ioctl_dsp_resume),
	IVP_IOCTL_OPS(IVP_IOCTL_DSP_STOP, ioctl_dsp_stop),
	IVP_IOCTL_OPS(IVP_IOCTL_QUERY_RUNSTALL, ioctl_query_runstall),
	IVP_IOCTL_OPS(IVP_IOCTL_QUERY_WAITI, ioctl_query_waiti),
	IVP_IOCTL_OPS(IVP_IOCTL_TRIGGER_NMI, ioctl_trigger_nmi),
	IVP_IOCTL_OPS(IVP_IOCTL_WATCHDOG, ioctl_watchdog),
	IVP_IOCTL_OPS(IVP_IOCTL_WATCHDOG_SLEEP, ioctl_watchdog_sleep),
	IVP_IOCTL_OPS(IVP_IOCTL_SMMU_INVALIDATE_TLB, ioctl_smmu_invalidate_tlb),
	IVP_IOCTL_OPS(IVP_IOCTL_BM_INIT, ioctl_bm_init),
	IVP_IOCTL_OPS(IVP_IOCTL_CLK_LEVEL, ioctl_clk_level),
	IVP_IOCTL_OPS(IVP_IOCTL_DUMP_DSP_STATUS, ioctl_dump_dsp_status),
#ifdef IVP_CHIPTYPE_SUPPORT
	IVP_IOCTL_OPS(IVP_IOCTL_QUERY_CHIP_TYPE, ioctl_query_chip_type),
#endif
};

static long ioctl_cmd_check(unsigned int cmd)
{
	struct ivp_device *pdev = &ivp_dev;

	if (atomic_read(&pdev->poweron_success) != 0 && cmd != IVP_IOCTL_POWER_UP) {
		ivp_err("ivp dev has not powered up, ioctl cmd is error %d", cmd);
		return -EINVAL;
	}

	return EOK;
}

static long ivp_ioctl(struct file *fd, unsigned int cmd, unsigned long args)
{
	uint32_t idx;
	ivp_ioctl_func_t func = NULL;
	ivp_ioctl_func_fd_t func_fd = NULL;

	if(ioctl_cmd_check(cmd) != EOK) {
		return -EINVAL;
	}

	for (idx = 0; idx < sizeof(ioctl_ops)/sizeof(ioctl_ops[0]); idx++) {
		if (cmd == ioctl_ops[idx].cmd) {
			if (ioctl_ops[idx].is_need_fd == 1) {
				if (ioctl_ops[idx].func_fd == NULL) {
					ivp_err("ivp ioctl: invalid func\n");
					return -EINVAL;
				}
				func_fd = ioctl_ops[idx].func_fd;
				return func_fd(args, fd);
			} else {
				if (ioctl_ops[idx].func == NULL) {
					ivp_err("ivp ioctl: invalid func\n");
					return -EINVAL;
				}
				func = ioctl_ops[idx].func;
				return func(args);
			}
		}
	}

	ivp_err("Invalid ioctl command(0x%08x) received!", cmd);
	return -EINVAL;
}

#ifdef CONFIG_COMPAT
static long ivp_ioctl32(struct file *fd, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	void *ptr_user = compat_ptr(arg);

	ret = ivp_ioctl(fd, cmd, (uintptr_t)ptr_user);
	return ret;
}
#endif

static int ivp_mmap(struct file *fd, struct vm_area_struct *vma)
{
	int i = 0;
	int ret = 0;
	unsigned int size = 0;
	unsigned long mm_pgoff = (vma->vm_pgoff << IVP_MMAP_SHIFT);
	unsigned long phy_addr = vma->vm_pgoff << (PAGE_SHIFT + IVP_MMAP_SHIFT);
	struct ivp_sect_info *psects = NULL;

	if (atomic_read(&ivp_dev.poweron_success) != 0) {
		ivp_err("Invalid mmap, not powerup!");
		return -EINVAL;
	}
	size = vma->vm_end - vma->vm_start;
	if (size > LISTENTRY_SIZE) {
		ivp_err("Invalid size = 0x%08x.", size);
		return -EINVAL;
	}

	if (ivp_dev.ivp_secmode == SECURE_MODE)
		psects = ivp_dev.sec_sects;
	else
		psects = ivp_dev.sects;
	for (; i < ivp_dev.sect_count; i++) {
		if (phy_addr >= (psects[i].acpu_addr << IVP_MMAP_SHIFT) &&
			(phy_addr <= ((psects[i].acpu_addr << IVP_MMAP_SHIFT) + psects[i].len)) &&
		    (phy_addr + size) <= ((psects[i].acpu_addr << IVP_MMAP_SHIFT) + psects[i].len)) {
			ivp_info("Valid section %d for target.", i);
			break;
		}
	}

	if (i == ivp_dev.sect_count) {
		ivp_err("Invalid mapping address or size.");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start, mm_pgoff, size, vma->vm_page_prot);
	if (ret < 0) {
		ivp_err("Failed to map address space. Error code is %d.", ret);
		return ret;
	}
	ivp_dbg("nocached success, ret:%#x", ret);

	return EOK;
}

static const struct file_operations ivp_fops = {
	.owner = THIS_MODULE,
	.open = ivp_open,
	.release = ivp_release,
	.unlocked_ioctl = ivp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ivp_ioctl32,
#endif
	.mmap = ivp_mmap,
};

static struct ivp_device ivp_dev = {
	.device = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = IVP_MODULE_NAME,
		.fops = &ivp_fops,
	}
};

#ifdef CONFIG_OF
static inline int ivp_setup_one_onchipmem_section(
	struct ivp_sect_info *sect, struct device_node *np)
{
	const char *name = of_node_full_name(np);
	unsigned int settings[3] = {0};
	unsigned int len = strlen(name);
	errno_t ret;

	if (of_property_read_u32_array(np, OF_IVP_SECTION_NAME, settings, ARRAY_SIZE(settings))) {/*lint -save -e30 -e84 -e514 -e846 -e866*/
		ivp_err("read reg fail.");
		return -EINVAL;
	}

	len = (len >= sizeof(sect->name)) ? (sizeof(sect->name) - 1) : len;
	ret = strncpy_s(sect->name, (sizeof(sect->name) - 1), name, len);
	if (ret != EOK) {
		ivp_err("(%s):strncpy_s fail, ret [%d]", __func__, ret);
		return -EINVAL;
	}
	sect->name[len] = '\0';
	sect->ivp_addr  = settings[0];
	sect->acpu_addr = settings[1];
	sect->len       = settings[2];

	return EOK;
}

static inline int ivp_setup_onchipmem_sections(
	struct platform_device *plat_devp, struct ivp_device *ivp_devp)
{
	struct device_node *parent = NULL;
	struct device_node *child = NULL;
	size_t i = 0;
	size_t sects_size = 0;
	errno_t ret;

	if (!plat_devp || !ivp_devp) {
		ivp_err("pointer is NULL.");
		return -EINVAL;
	}

	parent = of_get_child_by_name(
		plat_devp->dev.of_node, OF_IVP_SECTION_NODE_NAME);
	if (!parent) {
		ivp_err("Failed to get mem parent node.");
		return -ENODEV;
	}

	/*lint -save -e737 -e838*/
	ivp_devp->sect_count = of_get_child_count(parent);
	sects_size = sizeof(struct ivp_sect_info) * ivp_devp->sect_count;
	ivp_devp->sects = (struct ivp_sect_info *)kzalloc(sects_size, GFP_KERNEL);
	if (!ivp_devp->sects) {
		ivp_err("kmalloc sects fail.");
		return -ENOMEM;
	}

	ivp_devp->sec_sects = (struct ivp_sect_info *)kzalloc(sects_size, GFP_KERNEL);
	if (!ivp_devp->sec_sects) {
		ivp_err("kmalloc sec_sects fail.");
		goto err_sects;
	}

	ivp_info("section count:%d.", ivp_devp->sect_count);

	for_each_child_of_node(parent, child) {
		if (ivp_setup_one_onchipmem_section(&ivp_devp->sects[i], child)) {
			ivp_err("setup %lu section fail", i);
			goto err_out;
		}

		if (!strncmp(ivp_devp->sects[i].name, OF_IVP_DDR_MEM_NAME, sizeof(OF_IVP_DDR_MEM_NAME)) ||
		    !strncmp(ivp_devp->sects[i].name, OF_IVP_SHARE_MEM_NAME, sizeof(OF_IVP_SHARE_MEM_NAME)) ||
		    !strncmp(ivp_devp->sects[i].name, OF_IVP_LOG_MEM_NAME, sizeof(OF_IVP_LOG_MEM_NAME))) {
			ivp_devp->ivp_meminddr_len += ivp_devp->sects[i].len;
		}

		i++;
	}
	ret = memcpy_s(ivp_devp->sec_sects, sects_size, ivp_devp->sects, sects_size);
	if (ret != EOK) {
		ivp_err("(%s):memcpy_s fail, ret [%d]", __func__, ret);
		goto err_out;
	}
	/*lint -restore*/
	return EOK;

err_out:
	if (ivp_devp->sec_sects)
		kfree(ivp_devp->sec_sects);
	ivp_devp->sec_sects = NULL;
err_sects:
	if (ivp_devp->sects)
		kfree(ivp_devp->sects);
	ivp_devp->sects = NULL;
	ivp_devp->sect_count = 0;
	return -EFAULT;
}
#endif

static int ivp_setup_wdg_irq(
	struct platform_device *plat_devp, struct ivp_device *ivp_devp)
{
	int irq = platform_get_irq(plat_devp, 0);

	if (irq < 0) {
		ivp_err("Get irq fail!");
		return -EINVAL;
	}

	ivp_devp->wdg_irq = (unsigned int)irq;
	ivp_info("Get irq: %d.", irq);

	return EOK;
}

static int ivp_setup_dwaxi_irq(
	struct platform_device *plat_devp, struct ivp_device *ivp_devp)
{
	int irq = platform_get_irq(plat_devp, 1);

	if (irq < 0) {
		ivp_err("Get irq fail!");
		return -EINVAL;
	}

	ivp_devp->dwaxi_dlock_irq = (unsigned int)irq;
	ivp_info("Get irq: %d.", irq);

	return EOK;
}

static int ivp_setup_smmu_dev(struct ivp_device *ivp_devp)
{
	struct ivp_smmu_dev *smmu_dev = ivp_smmu_get_device(0UL);

	if (!smmu_dev) {
		ivp_err("Failed to get ivp smmu dev!");
		return -ENODEV;
	}
	ivp_devp->smmu_dev = smmu_dev;
	ivp_devp->smmu_pgd_base = smmu_dev->pgd_base;
	return EOK;
}

static void ivp_release_iores(struct platform_device *plat_devp)
{
	struct ivp_device *pdev =
	     (struct ivp_device *)platform_get_drvdata(plat_devp);

	ivp_info("enter");
	if (!pdev) {
		ivp_err("%s: pdev is null", __func__);
		return;
	}

#ifdef IVP_QOS_SUPPORT
	if (pdev->io_res.noc_ivp_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.noc_ivp_base_addr);
		pdev->io_res.noc_ivp_base_addr = NULL;
	}
#endif

	if (pdev->io_res.gic_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.gic_base_addr);
		pdev->io_res.gic_base_addr = NULL;
	}

	if (pdev->io_res.pericrg_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.pericrg_base_addr);
		pdev->io_res.pericrg_base_addr = NULL;
	}

	if (pdev->io_res.pctrl_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.pctrl_base_addr);
		pdev->io_res.pctrl_base_addr = NULL;
	}

	if (pdev->io_res.cfg_base_addr) {
		devm_iounmap(&plat_devp->dev, pdev->io_res.cfg_base_addr);
		pdev->io_res.cfg_base_addr = NULL;
	}
}

static int ivp_init_reg_res(
	struct platform_device *pdev, struct ivp_device *ivp_devp)
{
	struct resource *mem_res = NULL;

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0); /*lint -save -e838*/
	if (!mem_res) {
		ivp_err("Get cfg res fail!");
		goto err_exit;
	}

	ivp_devp->io_res.cfg_base_addr = devm_ioremap(
		&pdev->dev, mem_res->start, resource_size(mem_res));
	if (!ivp_devp->io_res.cfg_base_addr) {
		ivp_err("Map cfg reg failed!");
		goto err_exit;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem_res) {
		ivp_err("Get pctrl res failed!");
		goto err_exit;
	}

	ivp_devp->io_res.pctrl_base_addr = devm_ioremap(
		&pdev->dev, mem_res->start, resource_size(mem_res));
	if (!ivp_devp->io_res.pctrl_base_addr) {
		ivp_err("Map pctrl reg failed!");
		goto err_exit;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (!mem_res) {
		ivp_err("Get preicrg res failed!");
		goto err_exit;
	}
	ivp_devp->io_res.pericrg_base_addr = devm_ioremap(
		&pdev->dev, mem_res->start, resource_size(mem_res));
	if (!ivp_devp->io_res.pericrg_base_addr) {
		ivp_err("Map pericrg res failed!");
		goto err_exit;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if (!mem_res) {
		ivp_err("Get gic res failed");
		goto err_exit;
	}

	ivp_devp->io_res.gic_base_addr = devm_ioremap(&pdev->dev, GIC_IRQ_CLEAR_REG, 4); /*lint -save -e747*/
	if (!ivp_devp->io_res.gic_base_addr) {
		ivp_err("Map gic res failed!");
		goto err_exit;
	}
#ifdef IVP_QOS_SUPPORT
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	if (!mem_res) {
		ivp_err("Get noc_ivp res failed");
		goto err_exit;
	}

	ivp_devp->io_res.noc_ivp_base_addr = devm_ioremap(
		&pdev->dev, mem_res->start, resource_size(mem_res)); /*lint -save -e747*/
	if (!ivp_devp->io_res.noc_ivp_base_addr) {
		ivp_err("Map NOC_IVP32_Service_Target res failed!");
		goto err_exit;
	}
#endif
	return EOK;
err_exit:
	ivp_release_iores(pdev);
	return -EFAULT ;
}

static int ivp_remove(struct platform_device *plat_devp)
{
	struct ivp_device *pdev =
		(struct ivp_device *)platform_get_drvdata(plat_devp);

	if (pdev == NULL || pdev->sects == NULL) {
		ivp_err("ivp dev is NULL.This should not happen");
		return -ENODEV;
	}

	misc_deregister(&pdev->device);

	ivp_deinit_pri(pdev);

	kfree(pdev->sects);
	kfree(pdev->sec_sects);
	pdev->sects = NULL;
	pdev->sec_sects = NULL;
	pdev->sect_count = 0;
	ivp_release_iores(plat_devp);

	if (pdev->smmu_dev)
		pdev->smmu_dev = NULL;
	wakeup_source_trash(&ivp_power_wakelock);
	mutex_destroy(&ivp_wake_lock_mutex);
	mutex_destroy(&ivp_sec_mem_mutex);
	mutex_destroy(&ivp_load_image_mutex);
	mutex_destroy(&ivp_power_up_off_mutex);
	mutex_destroy(&ivp_open_release_mutex);

	return EOK;
}

static void ivp_probe_param_init(void)
{
	mutex_init(&ivp_wake_lock_mutex);
	mutex_init(&ivp_sec_mem_mutex);
	mutex_init(&ivp_load_image_mutex);
	mutex_init(&ivp_power_up_off_mutex);
	mutex_init(&ivp_open_release_mutex);
}

static void ivp_probe_param_deinit(void)
{
	mutex_destroy(&ivp_wake_lock_mutex);
	mutex_destroy(&ivp_sec_mem_mutex);
	mutex_destroy(&ivp_load_image_mutex);
	mutex_destroy(&ivp_power_up_off_mutex);
	mutex_destroy(&ivp_open_release_mutex);
}

static int ivp_probe(struct platform_device *pdev)
{
	int ret = 0;

	ivp_dev.ivp_pdev = pdev;
	platform_set_drvdata(pdev, &ivp_dev);
	atomic_set(&ivp_dev.accessible, 1);
	atomic_set(&ivp_dev.poweron_access, 1);
	atomic_set(&ivp_dev.poweron_success, 1);

	wakeup_source_init(&ivp_power_wakelock, "ivp_power_wakelock");
	ivp_probe_param_init();

	/*lint -save -e838*/
	ret = misc_register(&ivp_dev.device);
	if (ret)
		goto err_out_misc;

	ret = ivp_setup_smmu_dev(&ivp_dev);
	if (ret)
		goto err_out_smmu;

	ret = ivp_setup_onchipmem_sections(pdev, &ivp_dev);
	if (ret)
		goto err_out_onchipmem;

	ret = ivp_setup_wdg_irq(pdev, &ivp_dev);
	if (ret)
		goto err_out_wdg;

	ret = ivp_setup_dwaxi_irq(pdev, &ivp_dev);
	if (ret)
		goto err_out_dwaxi_irq;

	ret = ivp_init_reg_res(pdev, &ivp_dev);
	if (ret)
		goto err_out_reg_res;

	ret = ivp_init_pri(pdev, &ivp_dev);
	if (ret)
		goto err_out_init_pri;

	ivp_info("Success!");
	return ret;
	/*lint -restore*/

err_out_init_pri:
	ivp_release_iores(pdev);
err_out_reg_res:
err_out_dwaxi_irq:
err_out_wdg:
	if (ivp_dev.sects)
		kfree(ivp_dev.sects);
	if (ivp_dev.sec_sects)
		kfree(ivp_dev.sec_sects);
	ivp_dev.sects = NULL;
	ivp_dev.sec_sects = NULL;
	ivp_dev.sect_count = 0;
err_out_onchipmem:
	ivp_dev.smmu_dev = NULL;
err_out_smmu:
	misc_deregister(&ivp_dev.device);
err_out_misc:
	ivp_probe_param_deinit();
	wakeup_source_trash(&ivp_power_wakelock);

	return ret;
}

/*lint -save -e785 -e64*/
#ifdef CONFIG_OF
static const struct of_device_id ivp_of_id[] = {
	{ .compatible = "hisilicon,hisi-ivp", },
	{}
};
#endif

static struct platform_driver ivp_platform_driver = {
	.probe = ivp_probe,
	.remove = ivp_remove,
	.driver = {
		.name = IVP_MODULE_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ivp_of_id),
#endif
	},
};

module_platform_driver(ivp_platform_driver);
/*lint -restore*/

MODULE_DESCRIPTION("hisilicon_ivp driver");
MODULE_LICENSE("GPL");
//lint -restore
