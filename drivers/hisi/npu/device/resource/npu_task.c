/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include "npu_task.h"
#include "npu_common.h"
#include "drv_log.h"
#include "npu_pm.h"
#include "devdrv_user_common.h"

static u32 devdrv_get_available_task_num(struct devdrv_dev_ctx *dev_ctx)
{
	u32 task_id_num;

	spin_lock(&dev_ctx->task_spinlock);
	task_id_num = dev_ctx->task_id_num;
	spin_unlock(&dev_ctx->task_spinlock);

	return task_id_num;
}

static struct devdrv_task_info *devdrv_get_one_task_id(
						       struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_task_info *task_info = NULL;

	if (list_empty_careful(&dev_ctx->task_available_list))
		return NULL;

	task_info = list_first_entry(&dev_ctx->task_available_list,
				     struct devdrv_task_info, list);
	list_del(&task_info->list);
	dev_ctx->task_id_num--;
	return task_info;
}

static struct devdrv_task_info *devdrv_find_one_task(
						     struct devdrv_dev_ctx *dev_ctx,
						     u32 task_id)
{
	struct devdrv_task_info *task_info = NULL;
	task_info = (struct devdrv_task_info *)(dev_ctx->task_addr +
						(long)sizeof(struct devdrv_task_info) * task_id);

	return task_info->id != task_id ? NULL : task_info;
}

int devdrv_task_list_init(u8 dev_ctx_id)
{
	u32 i;
	u32 task_num = DEVDRV_MAX_TASK_ID;
	unsigned long size;
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_task_info *task_info = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		devdrv_drv_err("device id is illegal.\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		devdrv_drv_err("get device context by device id failed.\n");
		return -ENODATA;
	}

	INIT_LIST_HEAD(&dev_ctx->task_available_list);
	if (!list_empty_careful(&dev_ctx->task_available_list)) {
		devdrv_drv_err("available task list is not empty.\n");
		return -EEXIST;
	}

	size = (long)(unsigned)sizeof(struct devdrv_task_info) * task_num;
	task_info = vmalloc(size);
	if (task_info == NULL) {
		devdrv_drv_err("task_info vmalloc failed.\n");
		return -ENOMEM;
	}

	dev_ctx->task_id_num = 0;
	for (i = 0; i < task_num; i++) {
		task_info[i].id = i;
		task_info[i].devid = dev_ctx->devid;
		list_add_tail(&task_info[i].list,
			      &dev_ctx->task_available_list);
		dev_ctx->task_id_num++;
	}
	dev_ctx->task_addr = task_info;

	return 0;
}

EXPORT_SYMBOL(devdrv_task_list_init);

struct devdrv_task_info *devdrv_alloc_task(u8 dev_ctx_id)
{
	u32 task_num;

	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_task_info *task_info = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		devdrv_drv_err("device id is illegal.\n");
		return NULL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		devdrv_drv_err("get device context by device id failed.\n");
		return NULL;
	}

	task_num = devdrv_get_available_task_num(dev_ctx);
	if (task_num == 0) {
		devdrv_drv_err("no available task.\n");
		return NULL;
	}

	spin_lock(&dev_ctx->task_spinlock);
	task_info = devdrv_get_one_task_id(dev_ctx);
	if (task_info == NULL) {
		spin_unlock(&dev_ctx->task_spinlock);
		devdrv_drv_err("get one task info by dev_ctx failed.\n");
		return NULL;
	}
	spin_unlock(&dev_ctx->task_spinlock);

	return task_info;
}

EXPORT_SYMBOL(devdrv_alloc_task);

int devdrv_free_task_id(u8 dev_ctx_id, u32 task_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_task_info *task_info = NULL;

	if ((dev_ctx_id >= NPU_DEV_NUM)
	    || (task_id >= DEVDRV_MAX_TASK_ID)) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL) {
		devdrv_drv_err("get device context by device id failed.\n");
		return -ENODATA;
	}

	spin_lock(&dev_ctx->task_spinlock);
	task_info = devdrv_find_one_task(dev_ctx, task_id);
	if (task_info == NULL) {
		spin_unlock(&dev_ctx->task_spinlock);
		devdrv_drv_err("can not find task by device context.\n");
		return -ENODATA;
	}

	list_del(&task_info->list);
	list_add(&task_info->list, &dev_ctx->task_available_list);
	dev_ctx->task_id_num++;
	spin_unlock(&dev_ctx->task_spinlock);

	return 0;
}

int devdrv_task_list_destroy(u8 dev_ctx_id)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct devdrv_dev_ctx *dev_ctx = NULL;

	if (dev_ctx_id >= NPU_DEV_NUM) {
		devdrv_drv_err("device id is illegal.\n");
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	if (dev_ctx == NULL || dev_ctx->task_id_num == 0) {
		devdrv_drv_err("get device context by device id failed.\n");
		return -ENODATA;
	}

	if (!list_empty_careful(&dev_ctx->task_available_list)) {
		list_for_each_safe(pos, n, &dev_ctx->task_available_list) {
			dev_ctx->task_id_num--;
			list_del(pos);
		}
	}
	vfree(dev_ctx->task_addr);
	dev_ctx->task_addr = NULL;

	return 0;
}

EXPORT_SYMBOL(devdrv_task_list_destroy);
