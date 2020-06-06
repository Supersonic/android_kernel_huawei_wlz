/*
 * Remote Processor - Histar ISP remoteproc platform data.
 * include/linux/platform_data/remoteproc-hisi.h
 *
 * Copyright (c) 2013-2014 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _PLAT_REMOTEPROC_HISI_ISP_H
#define _PLAT_REMOTEPROC_HISI_ISP_H

#include <linux/firmware.h>
#include <linux/remoteproc.h>
#include <linux/iommu.h>
#include <linux/gpio.h>
#include <asm/page.h>
#include <linux/version.h>

#define IOMMU_MASK (~(IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_NOEXEC | IOMMU_MMIO | IOMMU_DEVICE | IOMMU_EXEC))
#define ISP_FW_MEM_SHARED_SIZE   (0x1000)
struct rproc_ops;
struct platform_device;
struct rproc;
struct scatterlist;
struct virtio_device;
struct virtqueue;
struct dentry;

/**
 * ISP LOGIC TYPE
 */

enum hisi_isplogic_type_info_e {
    ISP_FPGA_EXCLUDE    = 0x0,
    ISP_FPGA,
    ISP_UDP,
    ISP_FPGA_EXC,
    ISP_MAXTYPE
};

/**
 * struct fw_rsc_trace - print version information
 * @magic: magic word
 * @module: module
 * @version: version info
 * @built_time: built time
 * @reserved: reserved (must be zero)
 */
struct fw_rsc_version {
	unsigned int magic;
	char module[8];
	char version[8];
	char build_time[32];
	char reserved[4];
} __packed;

/**
 * struct fw_rsc_carveout - physically non-contiguous memory request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested memory region
 */
struct fw_rsc_dynamic_memory {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
} __packed;

/**
 * struct fw_rsc_carveout - physically reserved memory request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested memory region
 */
struct fw_rsc_reserved_memory {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
} __packed;

/**
 * struct fw_rsc_trace - cda buffer declaration
 * @da: device address
 * @len: length (in bytes)
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the trace buffer
 */
struct fw_rsc_cda {
	u32 da;
	u32 len;
	u32 reserved;
	u8 name[32];
} __packed;

/**
 * struct rproc_cache_entry - memory cache entry
 * @va:	virtual address
 * @len: length, in bytes
 * @node: list node
 */
struct rproc_cache_entry {
	void *va;
	u32 len;
	struct list_head node;
};

/**
 * struct rproc_page - page memory
 * @va:	virtual address of pages
 * @num: number of pages
 * @node: list node
 */
struct rproc_page {
	struct sg_table *table;
	struct list_head node;
};

struct rproc_page_info {
	struct page *page;
	unsigned int order;
	struct list_head node;
};
/*
 * struct omap_rproc_pdata - omap remoteproc's platform data
 * @name: the remoteproc's name
 * @oh_name: omap hwmod device
 * @oh_name_opt: optional, secondary omap hwmod device
 * @firmware: name of firmware file to load
 * @mbox_name: name of omap mailbox device to use with this rproc
 * @ops: start/stop rproc handlers
 * @device_enable: omap-specific handler for enabling a device
 * @device_shutdown: omap-specific handler for shutting down a device
 * @set_bootaddr: omap-specific handler for setting the rproc boot address
 */

struct hisi_rproc_data {
    const char *name;
    const char *firmware;
	const char *bootware;
    const char *mbox_name;
    const struct rproc_ops *ops;
    unsigned int ipc_addr;
    int (*device_enable) (struct platform_device *pdev);
    int (*device_shutdown) (struct platform_device *pdev);
    void(*set_bootaddr)(u32);
};

enum a7mappint_type {
    A7BOOT = 0,
    A7TEXT,
    A7DATA,
    A7PGD,
    A7PMD,
    A7PTE,
    A7RDR,
    A7SHARED,
    A7VQ,
    A7VRING0,
    A7VRING1,
    A7HEAP,
    A7DYNAMIC,
    A7REGISP,
    A7REGIPCS,
    A7REGIPCNS,
    A7REGPCTRL,
    A7REGSCTRL,
    A7REGPCFG,
    A7REGGIC,
    A7REGSYSC,
    A7REGUART,
    A7REGGPIO,
    A7REGGPIO25,
    A7REGIOC,
    A7MDC,
    MAXA7MAPPING
};

enum hisi_isp_rproc_case_attr {
    DEFAULT_CASE = 0,
    SEC_CASE,
    NONSEC_CASE,
    INVAL_CASE,
};

enum HISP_CLK_TYPE {
    ISPCPU_CLK  = 0,
    ISPFUNC_CLK = 1,
    ISPI2C_CLK  = 2,
    VIVOBUS_CLK = 3,
    ISPFUNC2_CLK = 3,
    ISPFUNC3_CLK = 4,
    ISPFUNC4_CLK = 5,
    ISPFUNC5_CLK = 6,
    ISP_SYS_CLK = 7,
    ISP_CLK_MAX
};

enum HISP_CLKDOWN_TYPE {
    HISP_CLK_TURBO      = 0,
    HISP_CLK_NORMINAL   = 1,
    HISP_CLK_LOWSVS     = 2,
    HISP_CLK_SVS        = 3,
    HISP_CLK_DISDVFS    = 4,
    HISP_CLKDOWN_MAX
};

enum hisi_soc_reg_type {
    CRGPERI     = 0,
    ISPCORE     = 1,
    PMCTRL      = 2,
    PCTRL       = 3,
    SCTRL       = 4,
    VIVOBUS     = 5,
    CSSYS       = 6,
    SUBCTRL     = 7,
    WDT         = 8,
    MEDIA1      = 9,
    ACTRL       = 10,
    HISP_MAX_REGTYPE
};

enum isp_msg_rdr_e {
    RPMSG_RDR_LOW                   = 0,
    /* RECV RPMSG INFO */
    RPMSG_CAMERA_SEND_MSG           = 1,
    RPMSG_SEND_MSG_TO_MAILBOX       = 2,
    RPMSG_RECV_MAILBOX_FROM_ISPCPU  = 3,
    RPMSG_ISP_THREAD_RECVED         = 4,
    RPMSG_RECV_SINGLE_MSG           = 5,
    RPMSG_SINGLE_MSG_TO_CAMERA      = 6,
    RPMSG_CAMERA_MSG_RECVED         = 7,
    RPMSG_RDR_MAX,
};
enum isp_msg_err_e {
    RPMSG_SEND_ERR                  = 0,
    RPMSG_SEND_MAILBOX_LOST,
    RPMSG_RECV_MAILBOX_LOST,
    RPMSG_THREAD_RECV_LOST,
    RPMSG_RECV_SINGLE_LOST,
    RPMSG_SINGLE_MSG_TO_CAMERA_LOST,
    RPMSG_CAMERA_MSG_RECVED_LOST,
    RPMSG_RECV_THRED_TIMEOUT,
    RPMSG_CAMERA_GET_TIMEOUT,
    RPMSG_ERR_MAX,
};

enum hisp_phy_id_e {
	HISP_CDPHY_A = 0,
	HISP_CDPHY_B,
	HISP_CDPHY_C,
	HISP_CDPHY_D,
	HISP_CDPHY_MAX,
};

enum hisp_phy_mode_e {
	HISP_PHY_MODE_DPHY = 0,
	HISP_PHY_MODE_CPHY,
	HISP_PHY_MODE_MAX,
};

enum hisp_phy_freq_mode_e {
	HISP_PHY_AUTO_FREQ = 0,
	HISP_PHY_MANUAL_FREQ,
	HISP_PHY_FREQ_MODE_MAX,
};

enum hisp_phy_work_mode_e {
	HISP_PHY_SINGLE_MODE = 0,
	HISP_PHY_DUAL_MODE_SENSORA,//dphy use DL1&3,cphy use DL2
	HISP_PHY_DUAL_MODE_SENSORB,//dphy use DL0&2,cphy use DL0&1
	HISP_PHY_WORK_MODE_MAX,
};

enum hisp_sec_mem_type {
	HISP_SEC_PAGE       = 0,
	HISP_SEC_TEXT       = 1,
	HISP_SEC_DATA       = 2,
	HISP_ISP_SEC_POOL   = 3,
	HISP_SEC_POOL       = 4,
	HISP_DYNAMIC_SEC    = 5,
	HISP_SEC_IPP        = 6,
	HISP_MEM_SET_SEC    = 7,
	HISP_SEC_MAX_TYPE
};

enum hisp_sec_staic_mem_type {
	HISP_SEC_VR0       = 0,
	HISP_SEC_VR1       = 1,
	HISP_SEC_VQ        = 2,
	HISP_SEC_SHARE     = 3,
	HISP_SEC_RDR       = 4,
	HISP_SEC_STATIC_MAX_TYPE
};

struct hisp_phy_info_t {
	unsigned int is_master_sensor;
	enum hisp_phy_id_e phy_id;
	enum hisp_phy_mode_e phy_mode;
	enum hisp_phy_freq_mode_e phy_freq_mode;
	unsigned int phy_freq;
	enum hisp_phy_work_mode_e phy_work_mode;
};

int rpmsg_sensor_ioctl(unsigned int cmd, int index, char* name);

/**
 * enum rpmsg_client_choice- choose which rpmsg client driver for debug
 *
 * @ISP_DEBUG_RPMSG_CLIENT: use isp debug rpmsg client
 * @INVALID_CLIENT: use hisp(new camera arch) rpmsg client
 */
enum rpmsg_client_choice {
	ISP_DEBUG_RPMSG_CLIENT  = 0x1111,
	INVALID_CLIENT          = 0xFFFF,
};
/*
 * hisi mem alloc struct
 */
struct hisi_isp_ion_s {
    struct ion_handle *ion_handle;
    struct ion_client *ion_client;
    u64 paddr;
    void *virt_addr;
};
/*
 * hisi viring alloc struct
 */
#define CONFIG_HISI_REMOTEPROC_DMAALLOC_DEBUG
#ifdef CONFIG_HISI_REMOTEPROC_DMAALLOC_DEBUG
#define HISI_ISP_VRING_NUM      3
#define HISI_ISP_VRING_SIEZ     (0x3000)
#define HISI_ISP_VQUEUE_SIEZ    (0x40000)

enum hisi_isp_vring_e {
    VRING0          = 0x0,
    VRING1,
    VIRTQUEUE,
};
struct hisi_isp_vring_s {
    u64 paddr;
    void *virt_addr;
    size_t size;
};
#endif

struct hisp_mdc_device {
    int isp_mdc_flag;
    int isp_mdc_count;
    dma_addr_t mdc_dma_addr;
	unsigned long iova;
	size_t size;
};
/*
 * hisi nesc cpucfg dump
 */
#define CORE_DUMP_EXC_CUR       (1)
#define CORE_DUMP_ION           (1 << 1)
#define CORE_DUMP_RTOS_FINISH   (1 << 2)
#define CORE_DUMP_ALL_FINISH    (1 << 3)
#define ISP_CPU_POWER_DOWN      (1 << 7)
#define CORE_DUMP_EXCEPTION     (1 << 5)

#define DUMP_ISPCPU_PC_TIMES    (3)
#define MAX_RESULT_LENGTH       (8)
#define DUMP_ISP_BOOT_SIZE      (64)
#define ISPCPU_STONE_OFFSET     (0xF00)
struct hisi_nsec_cpu_dump_s {
	unsigned int reg_addr;
	unsigned int actual_value;
	unsigned int expected_value;
	unsigned int care_bits;
	unsigned int compare_result;
	char result[MAX_RESULT_LENGTH];
};

struct hisi_isp_clk_dump_s {
	bool enable;
	unsigned int ispcpu_stat;
	unsigned int ispcpu_pc[DUMP_ISPCPU_PC_TIMES];
	unsigned long freq[ISP_CLK_MAX];
	unsigned int freqmask;
};

struct hisi_ispcpu_stone_s {
	unsigned int type; /**< EXCPTION TYPE                      */
	unsigned int uw_cpsr; /**< Current program status register (CPSR)     */
	unsigned int uw_r0; /**< Register R0                        */
	unsigned int uw_r1; /**< Register R1                        */
	unsigned int uw_r2; /**< Register R2                        */
	unsigned int uw_r3; /**< Register R3                        */
	unsigned int uw_r4; /**< Register R4                        */
	unsigned int uw_r5; /**< Register R5                        */
	unsigned int uw_r6; /**< Register R6                        */
	unsigned int uw_r7; /**< Register R7                        */
	unsigned int uw_r8; /**< Register R8                        */
	unsigned int uw_r9; /**< Register R9                        */
	unsigned int uw_r10; /**< Register R10                      */
	unsigned int uw_r11; /**< Register R11                      */
	unsigned int uw_r12; /**< Register R12                      */
	unsigned int uw_sp; /**< Stack pointer                      */
	unsigned int uw_lr; /**< Program returning address          */
	unsigned int uw_pc; /**< PC pointer of the exceptional function    */
	unsigned int dfsr; /**< DataAbort STATE      -- DFSR       */
	unsigned int dfar; /**< DataAbort ADDR       -- DFAR       */
	unsigned int ifsr; /**< PrefetchAbort STATE  -- IFSR       */
	unsigned int ifar; /**< PrefetchAbort ADDR   -- IFAR       */
	unsigned int smmu_far_low;  /**< SMMU500 ADDR -- FAR       */
	unsigned int smmu_far_high; /**< SMMU500 ADDR -- FAR       */
	unsigned int smmu_fsr; /**< SMMU500 STATE    -- FSR        */
	unsigned int smmu_fsynr0; /**< SMMU500 STATE -- FSYNR      */
};
enum hisi_isp_clk_info_e {
	ISPCPU          = 0x0,
	ISPFUNC,
	INFO_MAX,
};

extern int rpmsg_client_debug;
extern struct completion channel_sync;
extern const struct rproc_fw_ops rproc_elf_fw_ops;
extern const struct rproc_fw_ops rproc_bin_fw_ops;
extern struct rproc_shared_para *isp_share_para;

void hisi_ispsec_share_para_set(void);
int hisi_isp_rproc_case_set(enum hisi_isp_rproc_case_attr);
enum hisi_isp_rproc_case_attr hisi_isp_rproc_case_get(void);
int hisp_powerup(void);
int hisp_powerdn(void);
void *hisi_fstcma_alloc(dma_addr_t *dma_handle, size_t size, gfp_t flag);
void hisi_fstcma_free(void *va, dma_addr_t dma_handle, size_t size);

void atfisp_set_nonsec(void);
int atfisp_isptop_power_up(void);
int atfisp_isptop_power_down(void);
int atfisp_disreset_ispcpu(void);
int use_nonsec_isp(void);
int ispcpu_qos_cfg(void);
int hisp_qos_dtget(struct device_node *np);
void hisp_qos_free(void);
int use_sec_isp(void);
u64 get_a7sharedmem_addr(void);
u64 get_a7remap_addr(void);
void *getsec_a7sharedmem_addr(void);
unsigned long long get_nonsec_pgd(void);
void *get_a7remap_va(void);
void *get_a7sharedmem_va(void);
void set_a7mem_pa(u64 addr);
void set_a7mem_va(void *addr);
void set_a7sharedmem_addr(unsigned int addr);
int hisi_isp_nsec_probe(struct platform_device *pdev);
int hisi_isp_nsec_remove(struct platform_device *pdev);
int hisi_isp_rproc_pgd_set(struct rproc *rproc);
int nonsec_isp_device_enable(void);
int nonsec_isp_device_disable(void);
int hisp_nsec_jpeg_powerup(void);
int hisp_nsec_jpeg_powerdn(void);
void set_rpmsg_status(int status);
int is_ispcpu_powerup(void);

int hisp_rpmsg_rdr_init(void);
int hisp_rpmsg_rdr_deinit(void);
void hisp_sendin(void *data);
void hisp_sendx(void);
void hisp_recvtask(void);
void hisp_recvthread(void);
void hisp_recvin(void *data);
void hisp_recvx(void *data);
void hisp_recvdone(void *data);
void hisp_rpmsgrefs_dump(void);
void hisp_rpmsgrefs_reset(void);
void hisp_dump_rpmsg_with_id(const unsigned int message_id);

#ifdef DEBUG_HISI_ISP
extern unsigned int hispdbg_get_test_message_id(void);
extern void hispdbg_set_test_index(unsigned int dex, unsigned int type);
ssize_t isprdr_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t isprdr_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t ispclk_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t ispclk_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t isppower_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t isppower_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t ispmsg_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t ispmsg_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t atfisp_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t atfisp_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t regs_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t dump_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t atfisp_test_show(struct device *pdev, struct device_attribute *attr, char *buf);
ssize_t atfisp_test_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t security_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t security_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t ispbinupdate_show(struct device *pdev, struct device_attribute *attr, char *buf);
extern struct device *get_isp_device(void);
#endif
int hisp_sec_jpeg_powerup(void);
int hisp_sec_jpeg_powerdn(void);
int hisp_jpeg_powerdn(void);
int hisp_jpeg_powerup(void);
int check_dvfs_valid(void);
int hisp_set_clk_rate(unsigned int type, unsigned int rate);
int secnsec_setclkrate(unsigned int type, unsigned int rate);
unsigned long hisp_get_clk_rate(unsigned int type);
unsigned long secnsec_getclkrate(unsigned int type);
int nsec_setclkrate(unsigned int type, unsigned int rate);
int sec_setclkrate(unsigned int type, unsigned int rate);
int bypass_power_updn(void);
int set_power_updn(int bypass);
void __iomem *get_regaddr_by_pa(unsigned int type);
extern void hisi_isp_boot_stat_dump(void);
void dump_media1_regs(void);
void dump_smmu500_regs(void);
extern u64 hisi_getcurtime(void);
extern size_t print_time(u64 ts, char *buf);
extern unsigned int get_slice_time(void);
unsigned int a7_mmu_map(struct scatterlist *sgl, unsigned int size, unsigned int prot, unsigned int flag);
void a7_mmu_unmap(unsigned int va, unsigned int size);
#ifdef CONFIG_HISI_ISP_RDR
int rdr_isp_init(void);
void rdr_isp_exit(void);
extern u64 get_isprdr_addr(void);
void ispperfctrl_update(void);
void isploglevel_update(void);
void ispperf_stop_record(void);
void wait_firmware_coredump(void);
void ispcoresight_update(void);
void ispmonitor_update(void);
int sync_isplogcat(void);
int start_isplogcat(void);
void stop_isplogcat(void);
void clear_isplog_info(void);
extern void ap_send_fiq2ispcpu(void);
unsigned int secisp_ta_map(int sharefd, unsigned int type, unsigned int size, unsigned int prot);
int secisp_ta_unmap(int sharefd, unsigned int type, unsigned int size, unsigned int da);

#else
static inline int rdr_isp_init(void){return 0;};
static inline void rdr_isp_exit(void){return;};
static inline u64 get_isprdr_addr(void){return 0;}
static inline void ispperfctrl_update(void){return;}
static inline void isploglevel_update(void){return;}
static inline void ispperf_stop_record(void){return;}
static inline void wait_firmware_coredump(void){return;}
static inline void ispcoresight_update(void){return;}
static inline void ispmonitor_update(void){return;}
static inline int sync_isplogcat(void){return -1;}
static inline int start_isplogcat(void){return -1;}
static inline void stop_isplogcat(void){return;}
static inline void clear_isplog_info(void){return;}
static inline void ap_send_fiq2ispcpu(void){return;}
static inline unsigned int secisp_ta_map(int sharefd, unsigned int type, unsigned int size, unsigned int prot){return 0;};
static inline int secisp_ta_unmap(int sharefd, unsigned int type, unsigned int size, unsigned int da){return -1;};

#endif
void virtqueue_sg_init(struct scatterlist *sg, void *va, dma_addr_t dma, int size);
int rpmsg_vdev_map_resource(struct virtio_device *vdev, dma_addr_t dma, int total_space);
extern int rproc_add_virtio_devices(struct rproc *rproc);
int rproc_trigger_auto_boot(struct rproc *rproc);
extern int rproc_bootware_attach(struct rproc *rproc, const char *bootware);
extern struct rproc_shared_para *rproc_get_share_para(void);
extern int rproc_handle_version(struct rproc *rproc, struct fw_rsc_hdr *hdr, int offset, int avail);
extern int rproc_handle_cda(struct rproc *rproc, struct fw_rsc_hdr *hdr, int offset, int avail);
extern void rproc_memory_cache_flush(struct rproc *rproc);
extern int rproc_handle_dynamic_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr, int offset, int avail);
extern int rproc_handle_reserved_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr, int offset, int avail);
extern int rproc_handle_rdr_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr, int offset, int avail);
extern int rproc_handle_shared_memory(struct rproc *rproc, struct fw_rsc_hdr *hdr, int offset, int avail);
extern void *get_rsctable(int *tablesz);

struct dentry *rproc_create_cda_file(const char *name, struct rproc *rproc,
					struct rproc_mem_entry *cda);
extern int hisi_atfisp_probe(struct platform_device *pdev);
extern int hisi_atfisp_remove(struct platform_device *pdev);
extern int secisp_device_enable(void);
extern int secisp_device_disable(void);
extern void rproc_resource_cleanup(struct rproc *rproc);
extern void hisp_dynamic_mem_pool_clean(void);
extern void rproc_fw_config_virtio(const struct firmware *fw, void *context);
void rproc_auto_boot_callback(const struct firmware *fw, void *context);
extern int hisp_mem_type_pa_init(unsigned int etype, unsigned long paddr);
extern int sec_rproc_boot(struct rproc *rproc);
int nonsec_rproc_boot(struct rproc *rproc);
extern int hisp_rsctable_init(void);
extern int hisi_firmware_load_func(struct rproc *rproc);
extern void hisi_secisp_dump(void);
extern void wakeup_secisp_kthread(void);
extern int hw_is_fpga_board(void);
extern unsigned long hisp_mem_pool_alloc_iova(unsigned int size, unsigned int pool_num);
extern unsigned int hisp_mem_pool_free_iova(unsigned int pool_num, unsigned int va, unsigned int size);
extern unsigned int hisp_mem_map_setup(struct scatterlist *sgl, unsigned int iova, unsigned int size,
                        unsigned int prot, unsigned int pool_num, unsigned int flag,unsigned int align);
extern void hisp_mem_pool_destroy(unsigned int pool_num);
extern void hisp_mem_pool_init(void);
extern void hisp_mem_pool_exit(void);
extern void *hisp_rproc_da_to_va(struct rproc *rproc, u64 da, int len);
extern int hisp_rproc_enable_iommu(struct rproc *rproc, iommu_fault_handler_t handler);
extern void hisp_rproc_disable_iommu(struct rproc *rproc);
extern void hisp_rproc_init(struct rproc *rproc);
extern void hisp_rproc_resource_cleanup(struct rproc *rproc);
extern void hisp_virtio_boot_complete(struct rproc *rproc, int flag);
extern int hisp_rproc_boot(struct rproc *rproc);
extern unsigned long hisp_sg2virtio(struct virtqueue *vq, struct scatterlist *sg);
extern int rproc_fw_boot(struct rproc *rproc, const struct firmware *fw);
extern int rproc_probe_subdevices(struct rproc *rproc);
extern void rproc_remove_subdevices(struct rproc *rproc);

extern int hisi_isp_rproc_device_mclk_config(unsigned int id, unsigned int clk, int on);
extern int hisi_rproc_device_init(void);
extern int hisi_rproc_select_def(void);
extern int hisi_rproc_select_idle(void);
extern int hisi_isp_dependent_clock_enable(void);
extern int hisi_isp_dependent_clock_disable(void);
extern int get_rproc_enable_status(void);
extern int hisi_isp_rproc_enable(void);
extern int hisi_isp_rproc_disable(void);
extern void rproc_set_sync_flag(bool);
extern int set_isp_remap_addr(u64 remap_addr);
extern int hisp_jpeg_powerup(void);
extern int hisp_jpeg_powerdn(void);
extern int hisp_apisp_map(unsigned int *a7addr, unsigned int *ispaddr, unsigned int size);

unsigned int hisp_mem_pool_alloc_carveout(size_t size, unsigned int type);
int hisp_mem_pool_free_carveout(unsigned int  iova, size_t size);
extern struct hisi_isp_ion_s *get_nesc_addr_ion(size_t size, \
        size_t align, unsigned int heap_id_mask, unsigned int flags);
extern void free_nesc_addr_ion(struct hisi_isp_ion_s *hisi_nescaddr_ion);
extern int get_isp_mdc_flag(void);
extern int get_ispcpu_cfg_info(void);
int hisp_mntn_dumpregs(void);
u32 get_share_exc_flag(void);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
void *get_mdc_addr_va(void);
int mdc_addr_pa_init(void);
void mdc_addr_release(unsigned int size);
#else
int hispmdc_map_fw(unsigned long iova, phys_addr_t paddr, size_t size, int prot);
int hispmdc_unmap_fw(unsigned long iova, size_t size);
void hisp_mdc_dev_deinit(void);
#endif
void hisp_mdc_dev_init(void);
extern int set_ispcpu_reset(void);
extern u64 get_mdc_addr_pa(void);
extern void set_shared_mdc_pa_addr(u64 mdc_pa_addr);
extern unsigned int dynamic_memory_map(struct scatterlist *sgl,size_t addr,size_t size,unsigned int prot);
extern int dynamic_memory_unmap(size_t addr, size_t size);
int hisp_bsp_read_bin(const char *partion_name, unsigned int offset, unsigned int length, char *buffer);
int hisp_bin_load_segments(struct rproc *rproc);
void *hisp_get_rsctable(int *tablesz);

#ifdef CONFIG_HISI_REMOTEPROC_DMAALLOC_DEBUG
void *get_vring_dma_addr(u64 *dma_handle, size_t size, unsigned int index);
#endif
int get_media1_subsys_power_state(void);
int get_isptop_power_state(void);
extern unsigned int get_debug_isp_clk_enable(void);
extern int set_debug_isp_clk_enable(int state);
extern int set_debug_isp_clk_freq(unsigned int type, unsigned long value);
extern unsigned long get_debug_isp_clk_freq(unsigned int type);
extern struct hisi_nsec_cpu_dump_s* get_debug_ispcpu_param(void);
extern int last_boot_state;
extern int get_ispcpu_idle_stat(unsigned int isppd_adb_flag);
extern void dump_hisi_isp_boot(struct rproc *rproc,unsigned int size);
unsigned int wait_share_excflag_timeout(unsigned int flag, unsigned int time);
int hisp_ion_phys(struct ion_client *client, struct ion_handle *handle, dma_addr_t *addr);
extern void free_secmem_rsctable(void);
extern int hisp_phy_csi_connect(struct hisp_phy_info_t * phy_info, unsigned int csi_index);
extern int hisp_phy_csi_disconnect(void);
extern int sec_process_use_ca_ta(void);
#endif /* _PLAT_REMOTEPROC_HISI_ISP_H */

