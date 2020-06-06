#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>

#include "npu_recycle.h"
#include "npu_event.h"
#include "npu_model.h"
#include "npu_task.h"
#include "npu_mailbox_msg.h"
#include "npu_proc_ctx.h"
#include "devdrv_user_common.h"
#include "npu_common.h"
#include "npu_calc_cq.h"
#include "npu_calc_sq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_pm.h"
#include "drv_log.h"
#include "npu_platform.h"

static int devdrv_inform_recycle_event_id(struct devdrv_proc_ctx *proc_ctx)
{
	int ret;
	int result = 0;
	struct devdrv_event_info *event_info = NULL;
	struct devdrv_recycle_event_msg recycle_event;
	struct list_head *pos = NULL, *n = NULL;

	if (list_empty_careful(&proc_ctx->event_list)) {
		return 0;
	}

	memset(&recycle_event, 0xFF, sizeof(struct devdrv_recycle_event_msg));

	recycle_event.count = 0;
	list_for_each_safe(pos, n, &proc_ctx->event_list) {
		event_info = list_entry(pos, struct devdrv_event_info, list);

		recycle_event.count++;
		ret = devdrv_create_recycle_event_msg(event_info,
											&recycle_event);
		if (ret != 0) {
			devdrv_drv_err("create recycle event msg failed.\n");
			return -1;
		}

		if (recycle_event.count >= DEVDRV_RECYCLE_MAX_EVENT_NUM) {
			ret = devdrv_mailbox_message_send_for_res(
											proc_ctx->devid,
											(u8 *)&recycle_event,
											sizeof(struct devdrv_recycle_event_msg),
											&result);
			if ((ret != 0) || (result != 0)) {
				devdrv_drv_err("send recycle event id message failed.\n");
				return -1;
			}
			memset(&recycle_event, 0xFF, sizeof(struct devdrv_recycle_event_msg));
			recycle_event.count = 0;
		}
	}

	if (recycle_event.count == 0) {
		return 0;
	}

	ret = devdrv_create_recycle_event_msg(event_info,
										&recycle_event);
	if (ret != 0) {
		devdrv_drv_err("create recycle event msg failed.\n");
		return -1;
	}

	ret = devdrv_mailbox_message_send_for_res(
										proc_ctx->devid,
										(u8 *)&recycle_event,
										sizeof(struct devdrv_recycle_event_msg),
										&result);
	if ((ret != 0) || (result != 0)) {
		devdrv_drv_err("send recycle less 25 event id message failed.\n");
		return -1;
	}

	devdrv_drv_debug("recycle event id inform ts succeed.\n");
	return 0;
}

int devdrv_recycle_event_id(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_event_info *event_info = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;
	struct list_head *pos = NULL, *n = NULL;
	int inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	int ret;

	if (list_empty_careful(&proc_ctx->event_list)) {
		devdrv_drv_warn("proc context event list is empty.\n");
		return 0;
	}

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n",proc_ctx->devid);
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("get plat_ops failed.\n");
		return -EFAULT;
	}

	mutex_lock(&proc_ctx->event_mutex);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	if (cur_dev_ctx->power_stage != DEVDRV_PM_UP
		||(cur_dev_ctx->will_powerdown == true)) {
		devdrv_drv_info("recyle event no need to inform ts.\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	} else if (cur_dev_ctx->secure_state == NPU_SEC) {
		devdrv_drv_warn("recyle event no need to inform ts in NPU_SEC state.\n");
		inform_ts = DEVDRV_NO_NEED_TO_INFORM;
	} else {
		devdrv_drv_info("recyle event inform ts.\n");
		inform_ts = DEVDRV_HAVE_TO_INFORM;
	}

	if (inform_ts == DEVDRV_HAVE_TO_INFORM) {
		ret = devdrv_inform_recycle_event_id(proc_ctx);
		if (ret != 0) {
			mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);
			mutex_unlock(&proc_ctx->event_mutex);
			devdrv_drv_err("inform recycle event id failed.\n");
			return -1;
		}
	}

	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	list_for_each_safe(pos, n, &proc_ctx->event_list) {
		event_info = list_entry(pos, struct devdrv_event_info, list);
		if (event_info != NULL) {
			(void)devdrv_proc_free_event(proc_ctx, event_info->id);
		}
	}
	mutex_unlock(&proc_ctx->event_mutex);

	devdrv_drv_debug("recycle %d event resource success,inform_ts = %d \n",
											proc_ctx->event_num,inform_ts);
	return 0;
}

void devdrv_recycle_model_id(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_model_info *model_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (list_empty_careful(&proc_ctx->model_list)) {
		devdrv_drv_err("proc context model list is empty.\n");
		return;
	}

	mutex_lock(&proc_ctx->model_mutex);
	list_for_each_safe(pos, n, &proc_ctx->model_list) {
		model_info = list_entry(pos, struct devdrv_model_info, list);
		if (model_info != NULL) {
			(void)devdrv_proc_free_model(proc_ctx, model_info->id);
		}
	}
	mutex_unlock(&proc_ctx->model_mutex);
}

void devdrv_recycle_task_id(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_task_info *task_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (list_empty_careful(&proc_ctx->task_list)) {
		devdrv_drv_err("proc context task list is empty.\n");
		return;
	}

	mutex_lock(&proc_ctx->task_mutex);
	list_for_each_safe(pos, n, &proc_ctx->task_list) {
		task_info = list_entry(pos, struct devdrv_task_info, list);
		if (task_info != NULL) {
			(void)devdrv_proc_free_task(proc_ctx, task_info->id);
		}
	}
	mutex_unlock(&proc_ctx->task_mutex);
}

bool devdrv_is_proc_resource_leaks(struct devdrv_proc_ctx *proc_ctx)
{
	bool result =false;

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null\n");
		return false;
	}

	if (!list_empty_careful(&proc_ctx->message_list_header)  ||
		atomic_read(&proc_ctx->mailbox_message_count)        ||
		!list_empty_careful(&proc_ctx->sink_stream_list)     ||
		!list_empty_careful(&proc_ctx->stream_list)          ||
		!list_empty_careful(&proc_ctx->event_list)           ||
		!list_empty_careful(&proc_ctx->model_list)           ||
		!list_empty_careful(&proc_ctx->task_list)) {

		result = true;
	}

	return result;
}

void devdrv_resource_leak_print(struct devdrv_proc_ctx *proc_ctx)
{

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null\n");
		return;
	}

	devdrv_drv_warn("some npu resource are not released. Process Name: %s "
		"PID: %d, TGID: %d\n", current->comm, current->pid, current->tgid);

	if (!list_empty_careful(&proc_ctx->message_list_header)) {
		devdrv_drv_warn("message_list_header is not empty\n");
	}

	if (atomic_read(&proc_ctx->mailbox_message_count)) {
		devdrv_drv_warn("leak mailbox_message_count is %d \n",
							proc_ctx->mailbox_message_count.counter);
	}

	if (!list_empty_careful(&proc_ctx->sink_stream_list)) {
		devdrv_drv_warn("some sink stream id are not released !!"
				" stream num = %d\n",proc_ctx->sink_stream_num);
	}

	if (!list_empty_careful(&proc_ctx->stream_list)) {
		devdrv_drv_warn("some non sink stream id are not released !!"
				" stream num = %d\n",proc_ctx->stream_num);
	}

	if (!list_empty_careful(&proc_ctx->event_list)) {
		devdrv_drv_warn("some event id are not released !! "
					"event num = %d\n",proc_ctx->event_num);
	}

	if (!list_empty_careful(&proc_ctx->model_list)) {
		devdrv_drv_warn("some model id are not released !!"
				" model num = %d\n",proc_ctx->model_num);
	}

	if (!list_empty_careful(&proc_ctx->task_list)) {
		devdrv_drv_warn("some task id are not released !!"
				" task num = %d\n",proc_ctx->task_num);
	}

}

//it makes sense only runtime ,driver and ts  work together and driver do not
//totally free cq(don not clear cq_head and cq_tail to zero value)
static void devdrv_update_cq_info_phase(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_cq_sub_info * cq_sub_info = NULL;
	struct devdrv_ts_cq_info *cq_info = NULL;
	u32 report_phase;

	cq_sub_info = list_first_entry(&proc_ctx->cq_list,
					struct devdrv_cq_sub_info, list);
	cq_info = devdrv_calc_cq_info(proc_ctx->devid, cq_sub_info->index);

	report_phase = devdrv_proc_get_cq_head_report_phase(proc_ctx);

	if (cq_info->head == cq_info->tail) {
		//overturn the cq phase when the end of a round
		if (cq_info->head == DEVDRV_MAX_CQ_DEPTH - 1) {
			cq_info->phase = ((report_phase == 1) ? 0 : 1);
			devdrv_drv_warn("cq %d phase overturned,cq head = %d report phase"
							" = %d, cq tail = %d, phase = %d\n",cq_info->index,cq_info->head,
							report_phase, cq_info->tail, cq_info->phase);
		}

	}
	else if (cq_info->head < cq_info->tail) {
		cq_info->phase = report_phase;
		devdrv_drv_warn("cq %d phase no overturned,cq head = %d ,"
						"cq tail = %d report phase = %d, phase = %d\n",
						cq_info->index,cq_info->head,
						cq_info->tail,report_phase, cq_info->phase);
	}
	else {
		cq_info->phase = ((report_phase == 1) ? 0 : 1);
		devdrv_drv_warn("cq %d phase overturned,cq head = %d report phase"
							" = %d, tail = %d, phase = %d\n",
							cq_info->index,cq_info->head,report_phase,
							cq_info->tail, cq_info->phase);
	}

}

static int devdrv_recycle_sink_stream_list(struct devdrv_proc_ctx *proc_ctx)
{
	int ret = 0;
	int error = 0;
	struct list_head *pos = NULL, *n = NULL;
	struct devdrv_stream_sub_info *stream_sub = NULL;

	list_for_each_safe(pos, n, &proc_ctx->sink_stream_list) {
		stream_sub = list_entry(pos, struct devdrv_stream_sub_info, list);

		mutex_lock(&proc_ctx->stream_mutex);
		//no need to inform ts
		ret = devdrv_proc_free_stream(proc_ctx, stream_sub->id);
		mutex_unlock(&proc_ctx->stream_mutex);

		if (ret != 0) {
			error++;
		}
	}

	return error;
}

static int devdrv_recycle_stream_list(struct devdrv_proc_ctx *proc_ctx)
{
	int ret = 0;
	int error = 0;
	struct list_head *pos = NULL, *n = NULL;
	struct devdrv_stream_sub_info *stream_sub = NULL;

	list_for_each_safe(pos, n, &proc_ctx->stream_list) {
		stream_sub = list_entry(pos, struct devdrv_stream_sub_info, list);

		mutex_lock(&proc_ctx->stream_mutex);
		ret = devdrv_proc_free_stream(proc_ctx, stream_sub->id);
		mutex_unlock(&proc_ctx->stream_mutex);

		if (ret != 0) {
			error++;
		}
	}

	return error;
}

static int devdrv_recycle_stream(struct devdrv_proc_ctx *proc_ctx)
{
	int error = 0;

	if (list_empty_careful(&proc_ctx->sink_stream_list) &&
		list_empty_careful(&proc_ctx->stream_list)) {
		devdrv_drv_debug("no stream leaks, no need to recycle \n");
		return 0;
	}

	devdrv_update_cq_info_phase(proc_ctx);

	if (!list_empty_careful(&proc_ctx->sink_stream_list)) {
		error += devdrv_recycle_sink_stream_list(proc_ctx);
	}
	if (!list_empty_careful(&proc_ctx->stream_list)) {
		error += devdrv_recycle_stream_list(proc_ctx);
	}

	if (error != 0) {
		error = -error;
		devdrv_drv_err("recycle %d sink stream %d non sink stream resource error happened ,"
						" error times = %d \n",
						proc_ctx->sink_stream_num,proc_ctx->stream_num,error);
		return -1;
	}

	devdrv_drv_debug("recycle %d stream resource success \n", proc_ctx->stream_num);
	return 0;
}

static void devdrv_recycle_cq(struct devdrv_proc_ctx *proc_ctx)
{

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null\n");
		return;
	}

	devdrv_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)devdrv_remove_proc_ctx(&proc_ctx->dev_ctx_list,proc_ctx->devid);
	(void)devdrv_proc_free_cq(proc_ctx);
}

void devdrv_recycle_npu_resources(struct devdrv_proc_ctx *proc_ctx)
{
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;
	int ret;

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null\n");
		return;
	}

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx is null,no leak resource get recycled\n");
		return;
	}

	/* recycle proc_ctx mailbox message */
	if (!list_empty_careful(&proc_ctx->message_list_header)) {

	}

	if (atomic_read(&proc_ctx->mailbox_message_count)) {

	}

	/* recycle stream */
	ret = devdrv_recycle_stream(proc_ctx);
	if (ret != 0) {
		devdrv_drv_err("devdrv_recycle_stream failed.\n");
		goto recycle_error;
	}

	/* recycle event */
	ret = devdrv_recycle_event_id(proc_ctx);
	if (ret != 0) {
		devdrv_drv_err("devdrv_recycle_event failed.\n");
		goto recycle_error;
	}

	/* recycle model */
	devdrv_recycle_model_id(proc_ctx);
	devdrv_recycle_task_id(proc_ctx);

	/* recycle cq */
	devdrv_recycle_cq(proc_ctx);

	//unbind
	devdrv_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)devdrv_remove_proc_ctx(&proc_ctx->dev_ctx_list,proc_ctx->devid);
	devdrv_drv_warn("recycle all sources success.\n");

	return;

recycle_error:
	devdrv_drv_warn("failed to recycle sources, some sources are unavailable.\n");
	devdrv_add_proc_ctx_to_rubbish_ctx_list(
							&proc_ctx->dev_ctx_list,
							proc_ctx->devid);
	return;
}


