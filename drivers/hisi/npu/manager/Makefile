ifeq ($(TARGET_BOARD_PLATFORM),kirin990)
	ifeq ($(chip_type),es)
		CHIP_VERSION=hi3690_es
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990_ES
		EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	else ifeq ($(chip_type),cs2)
		CHIP_VERSION=kirin990_cs2
		EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	else
		CHIP_VERSION=hi3690
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_KIRIN990
		EXTRA_CFLAGS += -DCONFIG_NPU_NOC
	endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),orlando)
	CHIP_VERSION=hi6280
endif

KERNEL_DIR := $(srctree)

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc/comm
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/inc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/common
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/resource
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/device/service
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/dfx/logdrv
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/bbox
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/npu/platform/$(CHIP_VERSION)

obj-$(CONFIG_NPU_DEVDRV_DEVMANAGER) += npu_manager_module.o

npu_manager_module-objs := npu_proc_ctx.o npu_manager.o npu_ioctl_services.o
npu_manager_module-objs += npu_firmware.o npu_secure.o
npu_manager_module-objs += npu_recycle.o npu_manager_ioctl_services.o
npu_manager_module-objs += npu_heart_beat.o
npu_manager_module-objs += npu_manager_common.o
npu_manager_module-objs += bbox/npu_black_box.o
