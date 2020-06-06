ifeq ($(TARGET_PRODUCT),cloud)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_1980
endif

ifeq ($(fpga),true)
EXTRA_CFLAGS += -DTEMP_FPGA_B100
endif

KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/driver

ifeq ($(TARGET_PRODUCT),cloud)
	obj-m += drv_dbg.o
else
	obj-$(CONFIG_NPU_DBG_DEVDRV)   += drv_dbg.o
endif
drv_dbg-objs := drv_mailbox.o debug_drv_dev.o
