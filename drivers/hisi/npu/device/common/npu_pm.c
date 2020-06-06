/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include "npu_pm.h"
#include "drv_log.h"
#include "npu_platform.h"
#include "npu_firmware.h"
#include "npu_shm.h"

#include <linux/hisi/rdr_pub.h>
#include "bbox/npu_black_box.h"
#include "npu_heart_beat.h"

int npu_open(struct devdrv_dev_ctx *dev_ctx)
{
	int ret = 0;
	struct devdrv_platform_info *plat_info = NULL;

	devdrv_drv_warn("enter.\n");

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("get plat_ops failed.\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->open_access, 1, 0) == 0) {
		devdrv_drv_err("npu dev %d has already open!\n",dev_ctx->devid);
		return -EBUSY;
	}

	if (plat_info->dts_info.feature_switch[DEVDRV_FEATURE_KERNEL_LOAD_IMG]) {
		ret = devdrv_load_cpu_fw();
		if (ret != 0)
		{
			atomic_inc(&dev_ctx->open_access);
			devdrv_drv_err("load cpu fw failed ret %d.\n", ret);
			return ret;
		}
	} else {
		devdrv_drv_warn("no need to load firmware, for npu/aicpu firmware is loaded in fastboot.\n");
	}

	ret = DEVDRV_PLAT_GET_PM_OPEN(plat_info)();
	if (ret != 0) {
		atomic_inc(&dev_ctx->open_access);
		devdrv_drv_err("plat_ops npu_open failed.\n");
		return ret;
	}

	devdrv_drv_warn("npu dev %d open success!\n",dev_ctx->devid);
	atomic_dec(&dev_ctx->open_success);

	return 0;
}

int npu_release(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_platform_info *plat_info = NULL;

	devdrv_drv_debug("enter.\n");

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("get plat_ops failed.\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->open_success, 0, 1) == 1) {
		devdrv_drv_err("npu dev %d has already release!\n",dev_ctx->devid);
		return -EBUSY;
	}

	atomic_inc(&dev_ctx->open_access);
	devdrv_drv_warn("npu dev %d release success!\n",dev_ctx->devid);

	return 0;
}

int npu_powerup(struct devdrv_dev_ctx *dev_ctx,u32 secure_state)
{
	int ret = 0;
	u64 phy_fw_dst_addr = 0;
	struct devdrv_platform_info *plat_info = NULL;
	u32 *ts_status = NULL;
	unsigned long flags;

	devdrv_drv_warn("enter.\n");

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL)
	{
		devdrv_drv_err("get plat_ops failed.\n");
		return -ENODEV;
	}

	if (atomic_cmpxchg(&dev_ctx->power_access, 1, 0) == 0) {
		devdrv_drv_err("npu dev %d has already powerup!\n",dev_ctx->devid);
		return 0;
	}

	//unreset
	phy_fw_dst_addr = devdrv_get_firmware_phy_addr(DEVDRV_FW_TYPE_TS);
	devdrv_drv_info("firmware is loading");
	ret = DEVDRV_PLAT_GET_PM_POWER_UP(plat_info)(dev_ctx, phy_fw_dst_addr, secure_state);
	if (ret != 0)
	{
		atomic_inc(&dev_ctx->power_access);
		devdrv_drv_err("plat_ops npu_power_up failed.\n");
		/* bbox : npu power up falied */
		rdr_system_error((u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL, 0, 0);
		return ret;
	}
	if(dev_ctx->vma_l2 != NULL)
	{
		l2_mem_swapin(dev_ctx->vma_l2);
	}
	else
	{
		devdrv_drv_warn("dev_ctx->vma_l2 is NULL. No Need to do l2_mem_swapin\n");
	}
	//update inuse
	dev_ctx->inuse.devid = dev_ctx->devid;
	spin_lock_irqsave(&dev_ctx->ts_spinlock, flags);
	dev_ctx->inuse.ai_core_num = DEVDRV_PLAT_GET_AICORE_MAX(plat_info);
	dev_ctx->inuse.ai_core_error_bitmap = 0;
	dev_ctx->inuse.ai_cpu_num = DEVDRV_PLAT_GET_AICPU_MAX(plat_info);
	dev_ctx->inuse.ai_cpu_error_bitmap = 0;
	spin_unlock_irqrestore(&dev_ctx->ts_spinlock, flags);
	dev_ctx->ts_work_status = 1;
	ts_status = devdrv_get_ts_work_status(dev_ctx->devid, 0);
	if (ts_status != NULL)
	{
		*ts_status = (secure_state == NPU_NONSEC ? DEVDRV_TS_WORK : DEVDRV_TS_SEC_WORK);
	}

	devdrv_drv_warn("npu dev %u powerup success!\n", dev_ctx->devid);

	if (secure_state == NPU_NONSEC) {
		 /* bbox heart beat init in non_secure mode */
		ret = devdrv_heart_beat_init(dev_ctx);
		if (ret != 0)
		{
		    devdrv_drv_err("devdrv_heart_beat_init failed, ret = %d.\n", ret);
		}
	}
	atomic_dec(&dev_ctx->power_success);

	mutex_lock(&dev_ctx->npu_wake_lock_mutex);
	__pm_stay_awake(&dev_ctx->wakelock);
	mutex_unlock(&dev_ctx->npu_wake_lock_mutex);
	return 0; //lint !e454
}

int npu_powerdown(struct devdrv_dev_ctx *dev_ctx)
{
	int ret = 0;
	struct devdrv_platform_info *plat_info = NULL;
	u32 *ts_status = NULL;

	devdrv_drv_warn("enter.\n");
	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("get plat_ops failed.\n");
		return -1;
	}

	if (DEVDRV_PLAT_GET_FEAUTRE_SWITCH(plat_info,
			DEVDRV_FEATURE_AUTO_POWER_DOWN) == DEVDRV_FEATURE_OFF) {
		devdrv_drv_info("npu auto power down switch off.\n");
		return 0;
	}

	if (atomic_cmpxchg(&dev_ctx->power_success, 0, 1) == 1) {
		devdrv_drv_warn("npu dev %d has already powerdown!\n",dev_ctx->devid);
		return 0;
	}

	if (dev_ctx->secure_state == NPU_NONSEC) {
		/* bbox heart beat exit */
		devdrv_heart_beat_exit(dev_ctx);
	}

	if(dev_ctx->vma_l2 != NULL)
	{
		l2_mem_swapout(dev_ctx->vma_l2, dev_ctx->devid);
	}
	else
	{
		devdrv_drv_debug("dev_ctx->vma_l2 is NULL.No Need to do l2_mem_swapin\n");
	}

	ret = DEVDRV_PLAT_GET_PM_POWER_DOWN(plat_info)(dev_ctx->hisi_svm,
												dev_ctx->secure_state,
												&dev_ctx->power_stage);
	if (ret != 0) {
		devdrv_drv_err("plat_ops npu_power_down failed.\n");
		/* bbox : npu power down falied */
		rdr_system_error((u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL, 0, 0);
	}
	else
	{
		devdrv_drv_warn("npu dev %d powerdown success!\n",dev_ctx->devid);
	}

	dev_ctx->ts_work_status = 0;
	ts_status = devdrv_get_ts_work_status(dev_ctx->devid, 0);
	if (ts_status != NULL)
	{
		*ts_status = DEVDRV_TS_DOWN;
	}

	atomic_inc(&dev_ctx->power_access);

	mutex_lock(&dev_ctx->npu_wake_lock_mutex);
	__pm_relax(&dev_ctx->wakelock);
	mutex_unlock(&dev_ctx->npu_wake_lock_mutex); //lint !e455
	return ret;
}

//lint -e429
int devdrv_dev_software_register(struct devdrv_dev_ctx *dev_ctx,
				 software_ops_func init,
				 software_ops_func available,
				 software_ops_func recycle,
				 software_ops_func deinit)
{
	struct devdrv_dev_res_software_ops *software_ops = NULL;

	if (dev_ctx == NULL) {
		devdrv_drv_err("device context is null.\n");
		return -1;
	}

	software_ops = kzalloc(sizeof(struct devdrv_dev_res_software_ops),
		GFP_KERNEL);
	if (software_ops == NULL) {
		devdrv_drv_err("kzalloc software_ops failed.\n");
		return -1;
	}

	software_ops->devdrv_res_software_init = init;
	software_ops->devdrv_res_software_available = available;
	software_ops->devdrv_res_software_recycle = recycle;
	software_ops->devdrv_res_software_deinit = deinit;

	spin_lock_init(&dev_ctx->resource_software_spinlock);
	INIT_LIST_HEAD(&dev_ctx->resource_software_list);
	if (!list_empty_careful(&dev_ctx->resource_software_list)) {
		devdrv_drv_err("res_software_list is not empty.\n");
		kfree(software_ops);
		software_ops = NULL;
		return -1;
	}

	spin_lock(&dev_ctx->resource_software_spinlock);
	list_add_tail(&software_ops->list, &dev_ctx->resource_software_list);
	spin_unlock(&dev_ctx->resource_software_spinlock);

	return 0;
}

//lint +e429

int devdrv_dev_software_unregister(struct devdrv_dev_ctx *dev_ctx)
{
	struct list_head *pos = NULL, *n = NULL;
	struct devdrv_dev_res_software_ops *software_ops = NULL;

	if (!list_empty_careful(&dev_ctx->resource_software_list)) {
		list_for_each_safe(pos, n, &dev_ctx->resource_software_list) {
			software_ops = list_entry(pos,
						struct devdrv_dev_res_software_ops, list);
			kfree(software_ops);
			list_del(pos);
		}
	}

	return 0;
}

int devdrv_npu_ctrl_core(u32 dev_id, u32 core_num)
{
	int ret = 0;
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("get plat_ops failed.\n");
		return -EINVAL;
	}

	if(dev_id > NPU_DEV_NUM)
	{
		devdrv_drv_err("invalid device id %d.\n", dev_id);
		return -EINVAL;
	}

	if((core_num == 0) || (core_num > DEVDRV_PLAT_GET_AICORE_MAX(plat_info)))
	{
		devdrv_drv_err("invalid core num %d.\n", core_num);
		return -EINVAL;
	}

	if(NULL == DEVDRV_PLAT_GET_RES_CTRL_CORE(plat_info))
	{
		devdrv_drv_err("do not support ctrl core num %d.\n", core_num);
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if(dev_ctx == NULL)
	{
		devdrv_drv_err("get device %d ctx fail.\n", dev_id);
		return -EINVAL;
	}

	mutex_lock(&dev_ctx->npu_power_up_off_mutex);
	ret = DEVDRV_PLAT_GET_RES_CTRL_CORE(plat_info)(dev_ctx, core_num);
	mutex_unlock(&dev_ctx->npu_power_up_off_mutex);

	if(ret != 0)
	{
		devdrv_drv_err("ctrl device %d core num %d fail ret %d.\n", dev_id, core_num, ret);
	}
	else
	{
		devdrv_drv_warn("ctrl device %d core num %d success.\n", dev_id, core_num);
	}

	return ret;
}


EXPORT_SYMBOL(devdrv_npu_ctrl_core);

