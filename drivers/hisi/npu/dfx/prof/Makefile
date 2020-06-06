ifeq ($(TARGET_PRODUCT),cloud)
EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_1980
endif

ifeq ($(fpga),true)
EXTRA_CFLAGS += -DTEMP_FPGA_B100
endif

KERNEL_DIR :=$(srctree)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/resource
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/service
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/prof
EXTRA_CFLAGS += -DPROFILING_USE_RESERVED_MEMORY

ifeq ($(TARGET_PRODUCT),cloud)
	obj-m += drv_prof.o
else
	obj-$(CONFIG_NPU_PROF_DEVDRV)   += drv_prof.o
endif
drv_prof-objs := prof_drv_dev.o
