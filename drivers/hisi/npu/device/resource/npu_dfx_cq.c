/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "npu_dfx_cq.h"
#include "npu_dfx.h"
#include "npu_manager.h"
#include "npu_doorbell.h"
#include "npu_common.h"
#include "npu_pm.h"
#include "npu_shm.h"
#include "npu_cache.h"
#include "drv_log.h"
#include "npu_mailbox.h"
#include "npu_platform.h"
#include "devdrv_user_common.h"

static void devdrv_heart_beat_work(struct work_struct *work)
{
	struct devdrv_dfx_cq_report *report = NULL;
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cq_info *cq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	u32 sq_slot_index;
	u32 sq_index;
	u8 *sq_slot = NULL;
	u8 *cq_slot = NULL;
	u8 *cq_data = NULL;

	if (work == NULL)
	{
		devdrv_drv_err("work is null.\n");
		return;
	}

	cq_info = container_of(work, struct devdrv_dfx_cq_info, work);
	cqsq = (struct devdrv_dfx_cqsq *)cq_info->dfx_cqsq_ptr;
	if (cqsq == NULL) {
		devdrv_drv_err("dfx_cqsq_ptr is null.\n");
		return;
	}

	cq_slot =
	    cq_info->addr + (unsigned long)cq_info->tail * cq_info->slot_len;
	while (cq_slot[0] == cq_info->phase) {
		report = (struct devdrv_dfx_cq_report *)cq_slot;

		sq_index = report->sq_index;
		sq_info = &cqsq->sq_info[sq_index];
		sq_slot_index = (report->sq_head == 0) ?
		    (DEVDRV_MAX_DFX_SQ_DEPTH - 1) :
			(report->sq_head - 1);
		sq_slot = sq_info->addr +
		    (unsigned long)sq_slot_index * sq_info->slot_len;
		cq_data = cq_slot + DEVDRV_DFX_DETAILED_CQ_OFFSET;

		devdrv_drv_debug("call cq callback, cq id: %d.\n", cq_info->index);	// for test

		if (cq_info->callback != NULL)
			cq_info->callback(cq_data, sq_slot);
		sq_info->head = report->sq_head;

		if (cq_info->tail >= DEVDRV_MAX_DFX_CQ_DEPTH - 1) {
			cq_info->phase =
			    (cq_info->phase == DEVDRV_DFX_PHASE_ONE) ?
				DEVDRV_DFX_PHASE_ZERO : DEVDRV_DFX_PHASE_ONE;
			cq_info->tail = 0;
		} else {
			cq_info->tail++;
		}

		cq_slot = cq_info->addr +
		    (unsigned long)cq_info->tail * cq_info->slot_len;
	}

	cq_info->head = cq_info->tail;
	*cq_info->doorbell = (u32) cq_info->tail;
}

static void devdrv_get_dfx_cq_report(struct devdrv_dfx_int_context *int_context,
				struct devdrv_dfx_cq_info *cq_info)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (cq_info->function != DEVDRV_CQSQ_HEART_BEAT) {
		queue_work(int_context->wq, &cq_info->work);
		return;
	} else {
		cur_dev_ctx = get_dev_ctx_by_id(cq_info->devid);
		if(cur_dev_ctx == NULL){
			devdrv_drv_err("cur_dev_ctx %d is null\n",cq_info->devid);
			return;
		}

		if (cur_dev_ctx->heart_beat.hb_wq != NULL) {
			queue_work(cur_dev_ctx->heart_beat.hb_wq, &cq_info->work);
			return;
		}
	}

	return;
}

static int devdrv_get_dfx_cq_memory(struct devdrv_dfx_cq_info *cq_info,
				    u32 size)
{
	struct devdrv_platform_info *plat_info = NULL;
	unsigned int cnt = 0;
	unsigned int buf_size = 0;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return -EINVAL;
	}

	buf_size = DEVDRV_MAX_DFX_CQ_DEPTH * DEVDRV_DFX_MAX_CQ_SLOT_LEN;
	if (size > buf_size) {
		devdrv_drv_err("cq size=0x%x > 0x%x error\n", size, buf_size);
		return -ENOMEM;
	}
	// phy_addr:sq_calc_size + cq_calc_size + sq_dfx_size + off_size
	cq_info->phy_addr = (unsigned long long)(g_sq_desc->base +
						 DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM +
						 (DEVDRV_MAX_CQ_NUM * DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE) +
						 (DEVDRV_MAX_DFX_SQ_NUM * DEVDRV_MAX_DFX_SQ_DEPTH * DEVDRV_DFX_MAX_SQ_SLOT_LEN) +
						 (u64) cq_info->index * buf_size);	//lint !e647
	cq_info->addr = ioremap_nocache(cq_info->phy_addr, buf_size);
	for (cnt = 0; cnt < buf_size; cnt += 4) {
		writel(0, &cq_info->addr[cnt]);
	}

	devdrv_drv_debug("cur sq %d phy_addr = %pK base = %pK\n",
			 cq_info->index, (void *)(uintptr_t) cq_info->phy_addr,
			 (void *)(uintptr_t) (long)g_sq_desc->base);

	return 0;
}

int devdrv_dfx_cq_para_check(struct devdrv_dfx_create_cq_para *cq_para)
{
	if (cq_para->cq_type > DFX_DETAILED_CQ ||
	    cq_para->cq_index >= DEVDRV_MAX_DFX_CQ_NUM || cq_para->addr == NULL
	    || cq_para->slot_len <= 0
	    || cq_para->slot_len > DEVDRV_DFX_MAX_CQ_SLOT_LEN) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	return 0;
}

int devdrv_create_dfx_cq(struct devdrv_dev_ctx *cur_dev_ctx,
			 struct devdrv_dfx_create_cq_para *cq_para)
{
	struct devdrv_dfx_int_context *int_context = NULL;
	struct devdrv_dfx_cq_info *cq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	unsigned long flags;
	u32 cq_index = 0;
	int len;

	if (cur_dev_ctx == NULL || cq_para == NULL) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	if (devdrv_dfx_cq_para_check(cq_para)) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		return -ENOMEM;
	}

	if (cqsq->cq_num == 0) {
		devdrv_drv_err("no available cq num=%d.\n", cqsq->cq_num);
		return -ENOMEM;
	}

	cq_info = cqsq->cq_info;
	cq_index = cq_para->cq_index;

	mutex_lock(&cqsq->dfx_mutex);
	len = DEVDRV_MAX_DFX_CQ_DEPTH * cq_para->slot_len;
#ifdef PROFILING_USE_RESERVED_MEMORY
	if (devdrv_get_dfx_cq_memory(&cq_info[cq_index], len)) {
		mutex_unlock(&cqsq->dfx_mutex);
		devdrv_drv_err("devdrv_get_dfx_cq_memory failed.\n");
		return -ENOMEM;
	}
#else
	cq_info[cq_index].addr = kzalloc(len, GFP_KERNEL);
#endif
	if (cq_info[cq_index].addr == NULL) {
		mutex_unlock(&cqsq->dfx_mutex);
		devdrv_drv_err("kmalloc failed.\n");
		return -ENOMEM;
	}
	cq_info[cq_index].depth = DEVDRV_MAX_DFX_CQ_DEPTH;
	cq_info[cq_index].slot_len = cq_para->slot_len;
	cq_info[cq_index].type = cq_para->cq_type;
	cq_info[cq_index].phase = DEVDRV_DFX_PHASE_ONE;
	cq_info[cq_index].callback = cq_para->callback;
	cq_info[cq_index].function = cq_para->function;
	cqsq->cq_num--;
#ifdef PROFILING_USE_RESERVED_MEMORY
	*cq_para->addr = cq_info[cq_index].phy_addr;
#else
	*cq_para->addr =
	    (unsigned long)virt_to_phys((void *)cq_info[cq_index].addr);
	if (*cq_para->addr == NULL) {
		mutex_unlock(&cqsq->dfx_mutex);
		devdrv_drv_err("virt_to_phys failed.\n");
		return -ENOMEM;
	}
#endif
	mutex_unlock(&cqsq->dfx_mutex);
	int_context = &cqsq->int_context;
	spin_lock_irqsave(&int_context->spinlock, flags);
	list_add(&cq_info[cq_index].int_list_node,
		 &int_context->int_list_header);
	spin_unlock_irqrestore(&int_context->spinlock, flags);

	devdrv_drv_debug
	    ("dev[%d] dfx cq is created, cq id: %d, cq addr: %llx %llx.\n",
	     cur_dev_ctx->devid, cq_index, *cq_para->addr,
	     (u64) (uintptr_t) cq_info[cq_index].addr);
	return 0;
}

EXPORT_SYMBOL(devdrv_create_dfx_cq);

static irqreturn_t devdrv_dfx_cq_irq_handler(int irq, void *data)
{
	struct devdrv_dfx_int_context *int_context = NULL;
	struct devdrv_dfx_cq_info *cq_info = NULL;
	struct list_head *pos = NULL, *n = NULL;
	unsigned long flags;
	u8 *addr = NULL;
	u32 get = 0;
	u32 len;

	int_context = (struct devdrv_dfx_int_context *)data;

	spin_lock_irqsave(&int_context->spinlock, flags);
	list_for_each_safe(pos, n, &int_context->int_list_header) {
		cq_info =
		    list_entry(pos, struct devdrv_dfx_cq_info, int_list_node);

		addr =
		    cq_info->addr +
		    (unsigned long)cq_info->tail * cq_info->slot_len;
		len = cq_info->slot_len * DEVDRV_MAX_DFX_CQ_DEPTH;

		if (addr[0] == cq_info->phase) {
			devdrv_get_dfx_cq_report(int_context, cq_info);
			devdrv_drv_debug("receive irq find one report, "
					 "cq id: %d, addr: %pK.\n",
					 cq_info->index, (void *)addr);
			get++;
		}
	}
	spin_unlock_irqrestore(&int_context->spinlock, flags);

	if (get == 0)
		devdrv_drv_err("receive irq but no report found.\n");

	return IRQ_HANDLED;
}

void devdrv_destroy_dfx_cq(struct devdrv_dev_ctx *cur_dev_ctx, u32 cq_index)
{
	struct devdrv_dfx_int_context *int_context = NULL;
	struct devdrv_dfx_cq_info *cq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	unsigned long flags;

	if (cur_dev_ctx == NULL || cq_index >= DEVDRV_MAX_DFX_CQ_NUM) {
		devdrv_drv_err("invalid input argument.\n");
		return;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		return;
	}

	cq_info = cqsq->cq_info;
	mutex_lock(&cq_info[cq_index].lock);
	if (cq_info[cq_index].addr != NULL) {
		int_context = &cqsq->int_context;

		spin_lock_irqsave(&int_context->spinlock, flags);
		list_del(&cq_info[cq_index].int_list_node);
		spin_unlock_irqrestore(&int_context->spinlock, flags);

#ifdef PROFILING_USE_RESERVED_MEMORY
		iounmap(cq_info[cq_index].addr);
#else
		kfree(cq_info[cq_index].addr);
#endif

		mutex_lock(&cqsq->dfx_mutex);
		cq_info[cq_index].head = 0;
		cq_info[cq_index].tail = 0;
		cq_info[cq_index].addr = NULL;
		cq_info[cq_index].depth = DEVDRV_MAX_DFX_CQ_DEPTH;
		cq_info[cq_index].slot_len = 0;
		cq_info[cq_index].type = 0;
		cq_info[cq_index].phase = DEVDRV_DFX_PHASE_ONE;
		cq_info[cq_index].callback = NULL;
		cq_info[cq_index].function = DEVDRV_MAX_CQSQ_FUNC;
		cqsq->cq_num++;
		mutex_unlock(&cqsq->dfx_mutex);
	}
	mutex_unlock(&cq_info[cq_index].lock);
}

EXPORT_SYMBOL(devdrv_destroy_dfx_cq);

static void devdrv_dfx_work(struct work_struct *work)
{
	struct devdrv_dfx_cq_report *report = NULL;
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cq_info *cq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	u32 sq_slot_index;
	u32 sq_index;
	u8 *sq_slot = NULL;
	u8 *cq_slot = NULL;
	u8 *cq_data = NULL;
	u32 len;

	cq_info = container_of(work, struct devdrv_dfx_cq_info, work);
	cqsq = (struct devdrv_dfx_cqsq *)cq_info->dfx_cqsq_ptr;
	if (cqsq == NULL) {
		devdrv_drv_err("dfx_cqsq_ptr is null.\n");
		return;
	}

	mutex_lock(&cq_info->lock);
	if (cq_info->addr == NULL) {
		mutex_unlock(&cq_info->lock);
		return;
	}
	cq_slot =
	    cq_info->addr + (unsigned long)cq_info->tail * cq_info->slot_len;
	len = cq_info->slot_len * DEVDRV_MAX_DFX_CQ_DEPTH;

	while (cq_slot[0] == cq_info->phase) {
		if (cq_info->type == DFX_DETAILED_CQ) {
			report = (struct devdrv_dfx_cq_report *)cq_slot;
			sq_index = report->sq_index;
			sq_info = &cqsq->sq_info[sq_index];
			sq_slot_index = (report->sq_head == 0) ?
			    (DEVDRV_MAX_DFX_SQ_DEPTH - 1) : 
				(report->sq_head - 1);
			sq_slot = sq_info->addr +
			    (unsigned long)sq_slot_index * sq_info->slot_len;
			cq_data = cq_slot + DEVDRV_DFX_DETAILED_CQ_OFFSET;

			devdrv_drv_debug("call cq callback1, cq id: %d.\n", cq_info->index);	// for test

			if (cq_info->callback != NULL) {
				cq_info->callback(cq_data, sq_slot);
			}
			sq_info->head = report->sq_head;
		} else {

			cq_data = cq_slot + DEVDRV_DFX_BRIEF_CQ_OFFSET;
			devdrv_drv_debug("call cq callback0, cq id: %d.\n", cq_info->index);	// for test

			if (cq_info->callback != NULL) {
				cq_info->callback(cq_data, NULL);
			}
		}

		if (cq_info->tail >= DEVDRV_MAX_DFX_CQ_DEPTH - 1) {
			cq_info->phase =
			    (cq_info->phase == DEVDRV_DFX_PHASE_ONE) ?
				DEVDRV_DFX_PHASE_ZERO :
			    DEVDRV_DFX_PHASE_ONE;
			cq_info->tail = 0;
		} else {
			cq_info->tail++;
		}
		cq_slot = cq_info->addr +
		    (unsigned long)cq_info->tail * cq_info->slot_len;
	}

	cq_info->head = cq_info->tail;
	*cq_info->doorbell = (u32) cq_info->tail;
	mutex_unlock(&cq_info->lock);
	return;
}

struct devdrv_dfx_cqsq *devdrv_get_dfx_cqsq_info(struct devdrv_dev_ctx
						 *cur_dev_ctx)
{
	if (cur_dev_ctx->dfx_cqsq_addr == NULL) {
		devdrv_drv_err("dfx_cqsq_addr is null.\n");
		return NULL;
	}

	return (struct devdrv_dfx_cqsq *)cur_dev_ctx->dfx_cqsq_addr;
}

int devdrv_dfx_cqsq_init(struct devdrv_dev_ctx *cur_dev_ctx)
{
	struct devdrv_dfx_sq_info *sq_info = NULL;
	struct devdrv_dfx_cq_info *cq_info = NULL;
	struct devdrv_dfx_cqsq *cqsq = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	u32 *doorbell_vaddr = NULL;
	int ret = 0;
	int i;

	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return -EINVAL;
	}

	cur_dev_ctx->dfx_cqsq_addr =
	    (void *)kzalloc(sizeof(struct devdrv_dfx_cqsq), GFP_KERNEL);
	if (cur_dev_ctx->dfx_cqsq_addr == NULL) {
		devdrv_drv_err("no mem to alloc dfx cqsq.\n");
		return -ENOMEM;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		ret = -ENOMEM;
		goto dfx_cqsq_addr_free;
	}

	INIT_LIST_HEAD(&cqsq->int_context.int_list_header);
	spin_lock_init(&cqsq->int_context.spinlock);
	cqsq->int_context.wq = create_workqueue("devdrv-cqsq-work");
	if (cqsq->int_context.wq == NULL) {
		devdrv_drv_err("create_workqueue error.\n");
		ret = -ENOMEM;
		goto dfx_cqsq_addr_free;
	}

	sq_info = kzalloc(sizeof(struct devdrv_dfx_sq_info) * DEVDRV_MAX_DFX_SQ_NUM,
		    GFP_KERNEL);
	if (sq_info == NULL) {
		devdrv_drv_err("kmalloc failed.\n");
		ret = -ENOMEM;
		goto workqueue_free;
	}

	cq_info = kzalloc(sizeof(struct devdrv_dfx_cq_info) * DEVDRV_MAX_DFX_CQ_NUM,
		    GFP_KERNEL);
	if (cq_info == NULL) {
		devdrv_drv_err("kmalloc failed.\n");
		ret = -ENOMEM;
		goto sq_info_free;
	}

	for (i = 0; i < DEVDRV_MAX_DFX_SQ_NUM; i++) {
		sq_info[i].devid = cur_dev_ctx->devid;
		sq_info[i].index = i;
		sq_info[i].depth = DEVDRV_MAX_DFX_SQ_DEPTH;
		sq_info[i].slot_len = 0;
		sq_info[i].addr = NULL;
		sq_info[i].head = 0;
		sq_info[i].tail = 0;
		sq_info[i].credit = DEVDRV_MAX_DFX_SQ_DEPTH;
		sq_info[i].phy_addr = 0;
		ret = devdrv_get_doorbell_vaddr(DOORBELL_RES_DFX_SQ, i,
					      &doorbell_vaddr);
		if (ret) {
			devdrv_drv_err
			    ("devdrv_get_doorbell_vaddr cq failed %d.\n", i);
			ret = -ENOMEM;
			goto cq_info_free;
		}
		sq_info[i].doorbell = doorbell_vaddr;
		sq_info[i].function = DEVDRV_MAX_CQSQ_FUNC;
	}

	for (i = 0; i < DEVDRV_MAX_DFX_CQ_NUM; i++) {
		cq_info[i].devid = cur_dev_ctx->devid;
		cq_info[i].index = i;
		cq_info[i].depth = DEVDRV_MAX_DFX_CQ_DEPTH;
		cq_info[i].slot_len = 0;
		cq_info[i].type = 0;
		cq_info[i].dfx_cqsq_ptr = (void *)cqsq;

	if (i != DFX_HEART_BEAT_REPORT_CQ) {
		INIT_WORK(&cq_info[i].work, devdrv_dfx_work);
	} else {
		INIT_WORK(&cq_info[i].work, devdrv_heart_beat_work);
	}
		mutex_init(&cq_info[i].lock);

		cq_info[i].addr = NULL;
		cq_info[i].head = 0;
		cq_info[i].tail = 0;
		cq_info[i].phase = DEVDRV_DFX_PHASE_ONE;
		ret = devdrv_get_doorbell_vaddr(DOORBELL_RES_DFX_CQ, i,
					      &doorbell_vaddr);
		if (ret) {
			devdrv_drv_err
			    ("devdrv_get_doorbell_vaddr cq failed %d.\n", i);
			ret = -ENOMEM;
			goto cq_info_free;
		}
		cq_info[i].doorbell = doorbell_vaddr;
		cq_info[i].callback = NULL;
		cq_info[i].function = DEVDRV_MAX_CQSQ_FUNC;
	}

	cqsq->sq_info = sq_info;
	cqsq->sq_num = DEVDRV_MAX_DFX_SQ_NUM;
	cqsq->cq_info = cq_info;
	cqsq->cq_num = DEVDRV_MAX_DFX_CQ_NUM;

	cqsq->int_context.irq_num = DEVDRV_PLAT_GET_DFX_CQ_IRQ(plat_info);
	ret = request_irq(cqsq->int_context.irq_num, devdrv_dfx_cq_irq_handler,
			  IRQF_TRIGGER_NONE, "devdrv-functional_cq",
			  &cqsq->int_context);
	if (ret) {
		devdrv_drv_err("request_irq failed\n");
		ret = -ENOMEM;
		goto request_irq_free;
	}

	mutex_init(&cqsq->dfx_mutex);

	return 0;

request_irq_free:
	cqsq->sq_info = NULL;
	cqsq->sq_num = 0;
	cqsq->cq_info = NULL;
	cqsq->cq_num = 0;
cq_info_free:
	kfree(cq_info);
	cq_info = NULL;
sq_info_free:
	kfree(sq_info);
	sq_info = NULL;
workqueue_free:
	destroy_workqueue(cqsq->int_context.wq);
dfx_cqsq_addr_free:
	kfree(cur_dev_ctx->dfx_cqsq_addr);
	cur_dev_ctx->dfx_cqsq_addr = NULL;

	return ret;
}

void devdrv_destroy_dfx_cqsq(struct devdrv_dev_ctx *cur_dev_ctx)
{
	struct devdrv_dfx_cqsq *cqsq = NULL;

	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("device not exist");
		return;
	}

	cqsq = devdrv_get_dfx_cqsq_info(cur_dev_ctx);
	if (cqsq == NULL) {
		devdrv_drv_err("cqsq is null.\n");
		return;
	}
	free_irq(cqsq->int_context.irq_num, &cqsq->int_context);

	destroy_workqueue(cqsq->int_context.wq);

	if (cqsq->sq_info != NULL) {
		kfree(cqsq->sq_info);
	}

	if (cqsq->cq_info != NULL) {
		kfree(cqsq->cq_info);
	}

	if (cur_dev_ctx->dfx_cqsq_addr != NULL) {
		kfree(cur_dev_ctx->dfx_cqsq_addr);
	}

	mutex_lock(&cqsq->dfx_mutex);
	cqsq->sq_info = NULL;
	cqsq->sq_num = 0;
	cqsq->cq_info = NULL;
	cqsq->cq_num = 0;
	mutex_unlock(&cqsq->dfx_mutex);
	cur_dev_ctx->dfx_cqsq_addr = NULL;
	return;
}

int devdrv_dfx_send_mailbox(const struct devdrv_dev_ctx *cur_dev_ctx,
			    struct devdrv_mailbox_cqsq *cqsq)
{
	struct devdrv_platform_info *plat_info = NULL;
	int result = 0;
	u32 len;
	int ret = 0;

	if (cur_dev_ctx == NULL || cqsq == NULL) {
		devdrv_drv_err("invalid input argument.\n");
		return -EINVAL;
	}
	// ts_work_status 0: power down
	if (cur_dev_ctx->ts_work_status == 0) {
		devdrv_drv_err("device is not working.\n");
		return DEVDRV_TS_DOWN;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return -EINVAL;
	}

	len = sizeof(struct devdrv_mailbox_cqsq);

	devdrv_drv_debug("send dfx cqsq to TS, cmdType: 0x%x, "
			"sq id: %d, sq addr: %pK,"
			"cq0 id: %d, cq0 addr: %pK,"
			"cq1 id: %d, cq1 addr: %pK,"
			"cq2 id: %d, cq2 addr: %pK.\n",
			cqsq->cmd_type,
			cqsq->sq_index, (void *)(uintptr_t) cqsq->sq_addr,
			cqsq->cq0_index, (void *)(uintptr_t) cqsq->cq0_addr,
			cqsq->cq1_index, (void *)(uintptr_t) cqsq->cq1_addr,
			cqsq->cq2_index, (void *)(uintptr_t) cqsq->cq2_addr);	// for test
	cqsq->plat_type = plat_info->plat_type;
	ret = devdrv_mailbox_message_send_for_res(cur_dev_ctx->devid,
						(u8 *) cqsq, len, &result);
	if (ret == 0)
		ret = result;
	return ret;
}

EXPORT_SYMBOL(devdrv_dfx_send_mailbox);
