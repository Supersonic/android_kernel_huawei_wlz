/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include "npu_mailbox_msg.h"
#include "npu_shm.h"
#include "npu_stream.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "npu_common.h"
#include "npu_mailbox.h"
#include "npu_platform.h"
#include "npu_calc_cq.h"
#include <linux/hisi/hisi_svm.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
#include <linux/sched/mm.h>
#endif

static int hisi_svm_get_ttbr(u64 *ttbr, u64 *tcr)
{
	unsigned long asid = 0;
	struct mm_struct *mm = NULL;
	if((ttbr == NULL) || (tcr == NULL))
	{
		devdrv_drv_err("invalid ttbr or tcr\n");
		return -EINVAL;
	}
	mm = get_task_mm(current);
	if(mm == NULL)
	{
		devdrv_drv_err("get mm is null.\n");
		return -EINVAL;
	}
	//flush cache? ion_flush_all_cpus_caches
	asid = ASID(mm);
	*ttbr = virt_to_phys(mm->pgd)|(asid << 48);
	*tcr  = read_sysreg(tcr_el1);

	devdrv_drv_debug("pgdaddr:0x:%pK, context:0x%pK, pa:0x%pK\n",
					mm->pgd, (u64 *)(mm->pgd),
					(void *)(uintptr_t)virt_to_phys(mm->pgd));

	mmput(mm);

	devdrv_drv_debug("asid:%lu ,ttbr:0x%pK, tcr:0x%pK.\n",
								asid, (void *)(uintptr_t)*ttbr,
								(void *)(uintptr_t)*tcr);

	return 0;
}

static int __devdrv_create_alloc_stream_msg(u8 dev_id, u32 stream_id,
					    struct devdrv_stream_msg *stream_msg)
{
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	struct devdrv_dev_ctx* dev_ctx = NULL;
	phys_addr_t sq_phy_addr = 0;
	phys_addr_t cq_phy_addr = 0;
	u32 sq_index = 0;
	u32 cq_index = 0;
	u32 cq_stream_num = 0;
	int ret  = 0;

	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed dev id %d\n",
			       dev_id);
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (stream_id >= DEVDRV_MAX_STREAM_ID) {
		devdrv_drv_err("illegal npu stream id %d\n", stream_id);
		return -1;
	}

	if (stream_msg == NULL) {
		devdrv_drv_err("illegal para ,stream_msg is null \n");
		return -1;
	}

	stream_info = devdrv_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		devdrv_drv_err("npu dev id %d stream_id = %d `s "
			       "stream_info ,stream_info is null \n",
				   dev_id, stream_id);
		return -1;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		devdrv_drv_err("get_dev_ctx_by_id error, dev_id = %d \n", dev_id);
		return -1;
	}

	ret = hisi_svm_get_ssid((struct hisi_svm*)dev_ctx->hisi_svm,&(stream_msg->SSID),
							&(stream_msg->TTBR), &(stream_msg->TCR));
	if (ret != 0) {
		devdrv_drv_err("fail to get ssid, ret = %d\n", ret);
		return ret;
	}

	ret = hisi_svm_get_ttbr(&(stream_msg->TTBR), &(stream_msg->TCR));
	if (ret != 0) {
		devdrv_drv_err("fail to get process info, ret = %d\n", ret);
		return ret;
	}

	sq_index = stream_info->sq_index;
	cq_index = stream_info->cq_index;
	sq_info = devdrv_calc_sq_info(dev_id, sq_index);
	cq_info = devdrv_calc_cq_info(dev_id, cq_index);

	// get sq and cq phy addr of cur stream
	(void)devdrv_get_sq_phy_addr(dev_id, sq_index, &sq_phy_addr);
	(void)devdrv_get_cq_phy_addr(dev_id, cq_index, &cq_phy_addr);

	/* add message header */
	stream_msg->valid = DEVDRV_MAILBOX_MESSAGE_VALID;
	stream_msg->cmd_type = DEVDRV_MAILBOX_CREATE_CQSQ_CALC;
	stream_msg->result = 0;

	/* add payload */
	stream_msg->sq_index = sq_index;
	stream_msg->sq_addr = sq_phy_addr;
	if (sq_info->stream_num > 1) {
		stream_msg->sq_addr = 0;
	}
	stream_msg->cq0_addr = cq_phy_addr;

	cq_stream_num = devdrv_get_cq_ref_by_stream(dev_id, cq_index);
	if (cq_stream_num > 1) {
		stream_msg->cq0_addr = 0;
		devdrv_drv_debug("cur cq %d has been reference by %d "
				 "streams and cq_addr should be zero to "
				 "inform ts when alloc stream id %d\n",
				 cq_info->index, cq_stream_num,
				 stream_id);
	}

	devdrv_drv_debug("cur cq %d has been reference by %d "
			 "streams inform ts stream_id = %d \n",
			 cq_info->index, cq_stream_num,
			 stream_info->id);

	stream_msg->cq0_index = cq_index;

	stream_msg->stream_id = stream_id;
	stream_msg->plat_type = DEVDRV_PLAT_GET_TYPE(plat_info);
	stream_msg->cq_slot_size = cq_info->slot_size;


	devdrv_drv_debug("create alloc stream msg :"
			 "stream_msg->valid = %d  "
			 "stream_msg->cmd_type = %d  "
			 "stream_msg->result = %d  "
			 "stream_msg->sq_index = %d \n"
			 "stream_msg->sq_addr = %pK  "
			 "stream_msg->cq0_index = %d  "
			 "stream_msg->cq0_addr = %pK  "
			 "stream_msg->stream_id = %d  \n"
			 "stream_msg->plat_type = %d  "
			 "stream_msg->cq_slot_size = %d \n",
			 stream_msg->valid,
			 stream_msg->cmd_type,
			 stream_msg->result,
			 stream_msg->sq_index,
			 (void *)(uintptr_t) stream_msg->sq_addr,
			 stream_msg->cq0_index,
			 (void *)(uintptr_t) stream_msg->cq0_addr,
			 stream_msg->stream_id,
			 stream_msg->plat_type,
			 stream_msg->cq_slot_size);

	return 0;
}

static int __devdrv_create_free_stream_msg(u8 dev_id, u32 stream_id,
					   struct devdrv_stream_msg *stream_msg)
{
	struct devdrv_stream_info *stream_info = NULL;
	struct devdrv_ts_sq_info *sq_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	u32 sq_index = 0;
	u32 cq_index = 0;
	u32 cq_stream_num = 0;

	struct devdrv_platform_info *plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed dev id %d\n",
			       dev_id);
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (stream_id >= DEVDRV_MAX_STREAM_ID) {
		devdrv_drv_err("illegal npu stream id %d\n", stream_id);
		return -1;
	}

	if (stream_msg == NULL) {
		devdrv_drv_err("illegal para ,stream_msg is null \n");
		return -1;
	}

	stream_info = devdrv_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		devdrv_drv_err("npu dev id %d stream_id = %d `s "
			       "stream_info ,stream_info is null \n",
				   dev_id, stream_id);
		return -1;
	}

	sq_index = stream_info->sq_index;
	cq_index = stream_info->cq_index;
	sq_info = devdrv_calc_sq_info(dev_id, sq_index);
	cq_info = devdrv_calc_cq_info(dev_id, cq_index);

	/* add message header */
	stream_msg->valid = DEVDRV_MAILBOX_MESSAGE_VALID;
	stream_msg->cmd_type = DEVDRV_MAILBOX_RELEASE_CQSQ_CALC;
	stream_msg->result = 0;

	/* add payload */
	// no need carry address info when free stream
	stream_msg->sq_addr = 0;
	stream_msg->cq0_addr = 0;
	stream_msg->sq_index = sq_index;
	stream_msg->cq0_index = cq_index;

	cq_stream_num = devdrv_get_cq_ref_by_stream(dev_id, cq_index);
	if (cq_stream_num > 1) {
		stream_msg->cq0_index = DEVDRV_MAILBOX_INVALID_INDEX;
		devdrv_drv_debug("no need free stream`s cq now because "
				 "it is not the last stream reference of cq %d \n",
				 cq_index);
	} else {
		devdrv_drv_debug("should free stream`s cq now because "
				 "it is the last stream reference of cq %d "
				 "cq_info->stream_num = %d\n", cq_index,
				 cq_stream_num);
	}

	stream_msg->stream_id = stream_id;
	stream_msg->plat_type = DEVDRV_PLAT_GET_TYPE(plat_info);
	stream_msg->cq_slot_size = cq_info->slot_size;

	devdrv_drv_debug("create free stream msg :"
			 "stream_msg->valid = %d  "
			 "stream_msg->cmd_type = %d  "
			 "stream_msg->result = %d  "
			 "stream_msg->sq_index = %d \n"
			 "stream_msg->sq_addr = %pK  "
			 "stream_msg->cq0_index = %d  "
			 "stream_msg->cq0_addr = %pK  "
			 "stream_msg->stream_id = %d  \n"
			 "stream_msg->plat_type = %d  "
			 "stream_msg->cq_slot_size = %d \n",
			 stream_msg->valid,
			 stream_msg->cmd_type,
			 stream_msg->result,
			 stream_msg->sq_index,
			 (void *)(uintptr_t) stream_msg->sq_addr,
			 stream_msg->cq0_index,
			 (void *)(uintptr_t) stream_msg->cq0_addr,
			 stream_msg->stream_id,
			 stream_msg->plat_type,
			 stream_msg->cq_slot_size);

	return 0;
}

// build stream_msg  about stream to inform ts
// stream_msg is output para
// called after bind stream with sq and cq
int devdrv_create_alloc_stream_msg(u8 dev_id, u32 stream_id,
				   struct devdrv_stream_msg *stream_msg)
{
	int ret = -1;

	ret = __devdrv_create_alloc_stream_msg(dev_id, stream_id, stream_msg);

	return ret;
}

int devdrv_create_free_stream_msg(u8 dev_id, u32 stream_id,
				  struct devdrv_stream_msg *stream_msg)
{
	int ret = -1;

	ret = __devdrv_create_free_stream_msg(dev_id, stream_id, stream_msg);

	return ret;
}

int devdrv_create_recycle_event_msg(const struct devdrv_event_info *event_info,
				    struct devdrv_recycle_event_msg *recycle_event)
{
	struct devdrv_platform_info *plat_info = NULL;

	if ((event_info == NULL) || (recycle_event == NULL)) {
		devdrv_drv_err("event info or recycle event is null.\n");
		return -1;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed.\n");
		return -1;
	}

	recycle_event->event[recycle_event->count] = event_info->id;
	recycle_event->header.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
	recycle_event->header.cmd_type = DEVDRV_MAILBOX_RECYCLE_EVENT_ID;
	recycle_event->header.result = 0;
	recycle_event->plat_type = DEVDRV_PLAT_GET_TYPE(plat_info);

	return 0;
}
