KERNEL_DIR := $(srctree)

ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),es)
		CHIP_VERSION=hi3690_es
	else ifeq ($(chip_type),cs2)
		CHIP_VERSION=kirin990_cs2
	else
		CHIP_VERSION=hi3690
	endif
	obj-$(CONFIG_PLATFORM_HI3690)   += drv_adapt.o
endif

ifeq ($(TARGET_BOARD_PLATFORM),orlando)
	CHIP_VERSION=hi6280
	obj-$(CONFIG_PLATFORM_HI6280)   += drv_adapt.o
endif

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/services/atf
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/manager
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)

obj-$(CONFIG_NPU_DEVDRV_DEVPLATFORM)   += drv_platform.o
drv_platform-objs := npu_platform.o npu_feature.o npu_irq.o npu_resmem.o npu_resmem.o npu_reg.o npu_dfx.o npu_gic.o npu_adapter_pm.o npu_debug_resource_count.o
drv_adapt-objs := $(CHIP_VERSION)/npu_adapter.o 
