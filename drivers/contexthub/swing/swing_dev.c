/*
 * Copyright (C) Huawei Tech. Co. Ltd. 2017-2019. All rights reserved.
 * Description: dev drvier to communicate with sensorhub swing app
 * Author: Huawei
 * Create: 2017.12.05
 */
#include "swing_dev.h"
#include <linux/version.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/syscalls.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <securec.h>
#include <protocol.h>
#include "hisi_lb.h"
#include "../inputhub_api.h"
#include "../common.h"
#include "../shmem.h"
#include "hisi_bbox_diaginfo.h"
#include "bbox_diaginfo_id_def.h"

#define swing_log_info(msg...) pr_info("[I/SWING]" msg)
#define swing_log_err(msg...) pr_err("[E/SWING]" msg)
#define swing_log_warn(msg...) pr_warn("[W/SWING]" msg)

#ifdef __LLT_UT__
#define STATIC
#else
#define STATIC static
#endif

typedef struct {
	unsigned int recv_len;
	void *p_recv;
} swing_read_data_t;

typedef struct {
	struct completion swing_wait;
	struct completion read_wait;
	struct mutex read_mutex;        // Used to protect ops on read
	struct mutex swing_mutex;       // Used to protect ops on ioctl & open
	struct kfifo read_kfifo;
	swing_fusion_en_resp_t en_resp;
	swing_fusion_set_resp_t set_resp;
	int ref_cnt;
	int sh_recover_flag;
} swing_priv_t;

typedef struct {
	unsigned int dmd_id;
	unsigned int dmd_para_num;
	const char *dmd_msg;
} swing_dmd_log_t;

#define SWING_READ_CACHE_COUNT	(5)

#define SWING_IOCTL_WAIT_TMOUT	5000 // ms

#define SWING_RESET_NOTIFY	(0xFFFF)

#define SWING_DMD_CASE_DETAIL			0xFF
#define SWING_DMD_INFO_NUM_MAX			5
#define SWING_DEV_MAX_UPLOAD_LEN		(0x1000)

static swing_dmd_log_t dmd_log[] = {
	{SWING_DMD_FDUL_PW_ON,          3, "FDUL PowerOn Failed: step %u, addr 0x%x, value 0x%x."},
	{SWING_DMD_FDUL_PW_OFF,         3, "FDUL PowerOff Failed: step %u, addr 0x%x, value 0x%x."},
	{SWING_DMD_HWTS_PW_ON,          4, "HWTS PowerOn Failed: step %u, addr 0x%x, value 0x%x 0x%x."},
	{SWING_DMD_HWTS_PW_OFF,         4, "HWTS PowerOff Failed: step %u, addr 0x%x, value 0x%x 0x%x."},
	{SWING_DMD_AIC_PW_ON,           3, "TinyCore PowerOn Failed: step %u, addr 0x%x, value 0x%x."},
	{SWING_DMD_AIC_PW_OFF,          3, "TinyCore PowerOff Failed: step %u, addr 0x%x, value 0x%x."},
	{SWING_DMD_HWI_CREATE,          3, "HWI Create Error: INT no.: %u, retCode: 0x%x, HwiPrio 0x%x."},
	{SWING_DMD_HWI_DELETE,          2, "HWI Delete Error: INT no.: %u, retCode: 0x%x."},
	{SWING_DMD_CAM_PW_ON,           2, "Swing cam pw on timeout: cam_state: %d, ao_cam_status: %d."},
	{SWING_DMD_CAM_PW_OFF,          2, "Swing cam pw off timeout: cam_state: %d, ao_cam_status: %d."},
	{SWING_DMD_CAM_IR_PW,           3, "Swing IR pw fail: onoff: %d, ir_status: %d, pw_err_type: %d."},
	{SWING_DMD_CAM_TIMEOUT,         0, "Camera timeout."},
	{SWING_DMD_INT1_AIC_ERR,        4, "FDUL INT1 aicError: uid %u, state 0x%x, ecode 0x %x %x."},
	{SWING_DMD_INT1_TIMEOUT,        4, "FDUL INT1 taskTimeout: uid %u, state 0x%x, ecode 0x %x %x."},
	{SWING_DMD_INT1_BUS_ERR,        4, "FDUL INT1 HwtsBusError: uid %u, state 0x%x, ecode 0x %x %x."},
	{SWING_DMD_INT7_BUS_ERR,        1, "FDUL INT7 BusError: state 0x%x."},
	{SWING_DMD_FDUL_NOC,            1, "FDUL NOC Error: modid 0x%x; 0x86000000/1: IOMCU; 0x86000002: FDUL."},
	{SWING_DMD_MODEL_LOAD,          4, "Model Load Faided: step %u, uid %u, dmd_code 0x%x, para1 0x%x."},
	{SWING_DMD_MODEL_UNLOAD,        2, "Model Unload Failed: uid %u, dmd_code 0x%x."},
	{SWING_DMD_MODEL_RUN,           4, "Model Run Failed: uid %u, err 0x%x, para1 0x%x, para2 0x%x."},
	{SWING_DMD_AIC_KICK,            5, "AIC Kick Failed: uid %u, step %u, state 0x%x, ecode 0x %x %x."},
	{SWING_DMD_CAMERA_INIT,         4, "Camera Init Failed: ret 0x%x, pro_info_e %u, type %u, mode %u."},
	{SWING_DMD_CAMERA_RELEASE,      4, "Camera Release Failed: ret 0x%x, pro_info_e %u, type %u, mode %u"},
	{SWING_DMD_SLEEP_FUSION,        3, "Fusions still open when system suspend: num %u, bitmap 0x%x %x."},
};

static swing_priv_t g_swing_priv = { {0} };

static void swing_dev_wait_init(struct completion *p_wait)
{
	if (p_wait == NULL) {
		swing_log_err("swing_dev_wait_init: wait NULL\n");
		return;
	}

	init_completion(p_wait);
}

STATIC int swing_dev_wait_completion(struct completion *p_wait, unsigned int tm_out)
{
	if (p_wait == NULL) {
		swing_log_err("swing_dev_wait_completion: wait NULL\n");
		return -EFAULT;
	}

	swing_log_info("swing_dev_wait_completion: waitting\n");
	if (tm_out != 0) {
		if (!wait_for_completion_interruptible_timeout(p_wait,
								msecs_to_jiffies(tm_out))) {
			swing_log_warn("swing_dev_wait_completion: wait timeout\n");
			return -ETIMEOUT;
		}
	} else {
		if (wait_for_completion_interruptible(p_wait)) {
			swing_log_warn("swing_dev_wait_completion: wait interrupted.\n");
			return -EFAULT;
		}
	}

	return 0;
}

static void swing_dev_complete(struct completion *p_wait)
{
	if (p_wait == NULL) {
		swing_log_err("swing_dev_complete: wait NULL\n");
		return;
	}

	complete(p_wait);
}

static int swing_dev_ioctl_fusion_en(struct file *file, unsigned int cmd, unsigned long arg)
{
	swing_fusion_en_param_t fe = { {0} };
	int ret = 0;

	if (arg == 0) {
		swing_log_err("[%s] arg NULL.\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user((void *)&fe, (void *)((uintptr_t)arg), sizeof(swing_fusion_en_param_t))) {
		swing_log_err("[%s]copy_from_user error\n", __func__);
		return -EFAULT;
	}

	if (send_cmd_from_kernel(TAG_SWING, CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_FUSION_EN,
				 (char *)(&fe.en), sizeof(swing_fusion_en_t))) {
		swing_log_err("[%s]send cmd error\n", __func__);
		return -EFAULT;
	}

	ret = swing_dev_wait_completion(&g_swing_priv.swing_wait, SWING_IOCTL_WAIT_TMOUT);
	if (ret)
		return ret;

	ret = memcpy_s((void *)(&fe.en_resp), sizeof(swing_fusion_en_resp_t),
		(void *)(&g_swing_priv.en_resp), sizeof(swing_fusion_en_resp_t));
	if(EOK != ret){
		pr_err("%s memcpy buffer fail, ret[%d]\n", __func__, ret);
	}

	if (copy_to_user((char *)((uintptr_t)arg), &fe, sizeof(swing_fusion_en_param_t))) {
		swing_log_err("%s failed to copy to user\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int swing_dev_ioctl_fusion_set(struct file *file, unsigned int cmd, unsigned long arg)
{
	swing_fusion_set_param_t fs = { {0} };
	int ret = 0;

	if (arg == 0) {
		swing_log_err("[%s] arg NULL.\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user((void *)&fs, (void *)((uintptr_t)arg), sizeof(swing_fusion_set_param_t))) {
		swing_log_err("[%s]copy_from_user error\n", __func__);
		return -EFAULT;
	}

	if (send_cmd_from_kernel(TAG_SWING, CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_FUSION_SET,
				 (char *)(&fs.set), sizeof(swing_fusion_set_t))) {
		swing_log_err("[%s]send cmd error\n", __func__);
		return -EFAULT;
	}

	ret = swing_dev_wait_completion(&g_swing_priv.swing_wait, SWING_IOCTL_WAIT_TMOUT);
	if (ret)
		return ret;

	ret = memcpy_s((void *)(&fs.set_resp), sizeof(swing_fusion_set_resp_t),
		(void *)(&g_swing_priv.set_resp), sizeof(swing_fusion_set_resp_t));
	if(EOK != ret){
		pr_err("%s memcpy buffer fail, ret[%d]\n", __func__, ret);
	}

	if (copy_to_user((void *)((uintptr_t)arg), &fs, sizeof(swing_fusion_set_param_t))) {
		swing_log_err("%s failed to copy to user\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_dev_ioctl_timeout_handler(void)
{
	swing_read_data_t read_data = {0};
	int ret = 0;

	swing_log_info("enter [%s]\n", __func__);
	mutex_lock(&g_swing_priv.read_mutex);

	if (kfifo_avail(&g_swing_priv.read_kfifo) < sizeof(swing_read_data_t)) {
		swing_log_err("%s read_kfifo is full, drop upload data.\n", __func__);
		ret = -EFAULT;
		goto ERR;
	}

	read_data.recv_len = sizeof(u32);
	read_data.p_recv = kzalloc(sizeof(u32), GFP_ATOMIC);
	if (read_data.p_recv == NULL) {
		swing_log_err("Failed to alloc memory for sensorhub reset message...\n");
		ret = -EFAULT;
		goto ERR;
	}

	*(u32 *)read_data.p_recv = SWING_RESET_NOTIFY;

	ret = kfifo_in(&g_swing_priv.read_kfifo, (unsigned char *)&read_data, sizeof(swing_read_data_t));
	if (ret <= 0) {
		swing_log_err("%s: kfifo_in failed \n", __func__);
		ret = -EFAULT;
		goto ERR;
	}

	mutex_unlock(&g_swing_priv.read_mutex);

	g_swing_priv.sh_recover_flag = 1;

	swing_dev_complete(&g_swing_priv.read_wait);

	return 0;

ERR:
	if (read_data.p_recv != NULL) {
		kfree(read_data.p_recv);
	}

	mutex_unlock(&g_swing_priv.read_mutex);

	return ret;
}

static long swing_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	swing_log_info("%s cmd: [0x%x]\n", __func__, cmd);

	if (g_swing_priv.sh_recover_flag == 1) {
		swing_log_info("%s sensorhub in recover mode\n", __func__);
		return -EBUSY;
	}

	mutex_lock(&g_swing_priv.swing_mutex);

	reinit_completion(&g_swing_priv.swing_wait);

	swing_log_info("%s reinit completion\n", __func__);
	switch (cmd) {
	case SWING_IOCTL_FUSION_EN:
		ret = swing_dev_ioctl_fusion_en(file, cmd, arg);
		break;

	case SWING_IOCTL_FUSION_SET:
		ret = swing_dev_ioctl_fusion_set(file, cmd, arg);
		break;

	default:
		mutex_unlock(&g_swing_priv.swing_mutex);
		swing_log_err("%s unknown cmd : %d\n", __func__, cmd);
		return -ENOTTY;
	}

	mutex_unlock(&g_swing_priv.swing_mutex);

	if (ret == -ETIMEOUT) {
		if (swing_dev_ioctl_timeout_handler())
			swing_log_err("%s swing_dev_ioctl_timeout_handler err\n", __func__);
	}

	return ret;
}

STATIC int swing_get_resp_para_check(const pkt_header_t *head)
{
	pkt_subcmd_resp_t *p_resp = NULL;
	swing_upload_t *p_upload = NULL;
	unsigned int valid_len = 0;

	p_resp = (pkt_subcmd_resp_t *)(head);

	if (p_resp == NULL) {
		swing_log_err("%s: p_resp is null\n", __func__);
		return -EFAULT;
	}

	if (p_resp->hd.tag != TAG_SWING) {
		swing_log_err("%s: invalid tag [0x%x]\n", __func__, p_resp->hd.tag);
		return -EFAULT;
	}

	switch (p_resp->subcmd) {
	case SUB_CMD_SWING_FUSION_EN:
		valid_len = sizeof(swing_fusion_en_resp_t);
		break;
	case SUB_CMD_SWING_FUSION_SET:
		valid_len = sizeof(swing_fusion_set_resp_t);
		break;

	case SUB_CMD_SWING_FUSION_UPLOAD:
        p_upload = (swing_upload_t *)(p_resp + 1);
		valid_len = sizeof(swing_upload_t) + p_upload->notify_len;
		break;

	default:
		swing_log_warn("%s: unhandled cmd, tag[%d], sub_cmd[%d]\n",
			__func__, p_resp->hd.tag, p_resp->subcmd);
		return -EFAULT;
	}

	if (head->length != (valid_len + 8)) {
		swing_log_err("%s: invalid payload length: tag[%d], sub_cmd[%d], length[0x%x]\n",
            __func__, p_resp->hd.tag, p_resp->subcmd, head->length);
		return -EFAULT;
	}

	return 0;
}

static int swing_get_resp_upload(pkt_subcmd_resp_t *p_resp)
{
	swing_upload_t *p_upload = NULL;
	swing_read_data_t read_data = {0};
	int ret = 0;

	mutex_lock(&g_swing_priv.read_mutex);

	if (kfifo_avail(&g_swing_priv.read_kfifo) < sizeof(swing_read_data_t)) {
		swing_log_err("%s read_kfifo is full, drop upload data.\n", __func__);
		ret = -EFAULT;
		goto RET_ERR;
	}

	p_upload = (swing_upload_t *)(p_resp + 1);

	if (p_upload->notify_len > SWING_DEV_MAX_UPLOAD_LEN) {
		swing_log_err("%s upload length err 0x%x.\n", __func__, p_upload->notify_len);
		ret = -EFAULT;
		goto RET_ERR;
	}

	read_data.recv_len = sizeof(swing_upload_t) + p_upload->notify_len;
	read_data.p_recv = kzalloc(read_data.recv_len, GFP_ATOMIC);
	if (read_data.p_recv == NULL) {
		swing_log_err("Failed to alloc memory to save upload resp...\n");
		ret = -EFAULT;
		goto RET_ERR;
	}

	ret = memcpy_s(read_data.p_recv, read_data.recv_len, p_upload, read_data.recv_len);
	if (ret != 0) {
		swing_log_err("%s memcpy_s failed...\n", __func__);
		ret = -EFAULT;
		goto RET_ERR;
	}

	ret = kfifo_in(&g_swing_priv.read_kfifo, (unsigned char *)&read_data, sizeof(swing_read_data_t));
	if (ret <= 0) {
		swing_log_err("%s: kfifo_in failed \n", __func__);
		ret = -EFAULT;
		goto RET_ERR;
	}

	mutex_unlock(&g_swing_priv.read_mutex);

	swing_log_info("Fusion Upload Received, [0x%x].\n", p_upload->fusion_id);
	swing_dev_complete(&g_swing_priv.read_wait);

	return 0;

 RET_ERR:
	if (read_data.p_recv != NULL) {
		kfree(read_data.p_recv);
	}

	mutex_unlock(&g_swing_priv.read_mutex);

	return ret;
}

static int swing_get_resp(const pkt_header_t *head)
{
	pkt_subcmd_resp_t *p_resp = NULL;
	swing_fusion_en_resp_t *p_en_resp = NULL;
	swing_fusion_set_resp_t *p_set_resp = NULL;
	int ret = 0;

	ret = swing_get_resp_para_check(head);
	if (ret != 0) {
		swing_log_warn("%s: para check failed.\n", __func__);
		return ret;
	}

	p_resp = (pkt_subcmd_resp_t *)(head);

	swing_log_info("%s: cmd[%d], length[%d], tag[%d], sub_cmd[%d]\n",
		__func__, p_resp->hd.cmd, p_resp->hd.length, p_resp->hd.tag, p_resp->subcmd);

	switch (p_resp->subcmd) {
	case SUB_CMD_SWING_FUSION_EN:
		p_en_resp = (swing_fusion_en_resp_t *)(p_resp + 1);
		ret = memcpy_s(&g_swing_priv.en_resp, sizeof(swing_fusion_en_resp_t), p_en_resp, sizeof(swing_fusion_en_resp_t));
		if(EOK != ret){
			pr_err("%s memcpy buffer fail, ret[%d]\n", __func__, ret);
		}

		swing_log_info("Fusion [0x%x] En Resp .\n", p_en_resp->fusion_id);
		swing_dev_complete(&g_swing_priv.swing_wait);
		break;
	case SUB_CMD_SWING_FUSION_SET:
		p_set_resp = (swing_fusion_set_resp_t *)(p_resp + 1);
		ret = memcpy_s(&g_swing_priv.set_resp, sizeof(swing_fusion_set_resp_t), p_set_resp, sizeof(swing_fusion_set_resp_t));
		if(EOK != ret){
			pr_err("%s memcpy buffer fail, ret[%d]\n", __func__, ret);
		}
		swing_log_info("Fusion [0x%x] Set Resp .\n", p_set_resp->fusion_id);
		swing_dev_complete(&g_swing_priv.swing_wait);
		break;

	case SUB_CMD_SWING_FUSION_UPLOAD:
		swing_log_info("Fusion Upload 0x%x.\n", p_resp->subcmd);

		ret = swing_get_resp_upload(p_resp);
		break;

	default:
		swing_log_warn("unhandled cmd: tag[%d], sub_cmd[%d]\n", p_resp->hd.tag, p_resp->subcmd);
		break;
	}

	return ret;
}

STATIC u32 swing_get_dmd_log_index(u32 dmd_id)
{
	u32 index = 0;
	size_t log_arry_size = sizeof(dmd_log) / sizeof(dmd_log[0]);
	for (index = 0; index < log_arry_size; index++) {
		if (dmd_log[index].dmd_id == dmd_id) {
			break;
		}
	}
	return index;
}

static int swing_get_shb_dmd_report(const pkt_header_t *head)
{
	int ret;
	u32 index;
	size_t hd_size = sizeof(pkt_header_t);
	size_t req_size = sizeof(pkt_dmd_log_report_req_t);
	size_t log_arry_size = sizeof(dmd_log) / sizeof(dmd_log[0]);

	pkt_dmd_log_report_req_t *req = (pkt_dmd_log_report_req_t *)(head);

	if (req == NULL) {
		swing_log_err("swing_get_shb_dmd_report: req is null!!\n");
		return -EFAULT;
	}

	if (req->hd.tag != TAG_SWING) {
		swing_log_err("swing_get_shb_dmd_report: tag[0x%x] not match TAG_SWING!!\n", req->hd.tag);
		return -EFAULT;
	}

	if (req->hd.cmd != CMD_LOG_REPORT_REQ) {
		swing_log_err("swing_get_shb_dmd_report: cmd[0x%x] not match CMD_LOG_REPORT_REQ!!\n", req->hd.cmd);
		return -EFAULT;
	}

	if (hd_size + req->hd.length != req_size) {
		swing_log_err("swing_get_shb_dmd_report: req length check failed!! hd_size[%u], payload size[%u], total[%u]!!\n",
					   (u32)hd_size, req->hd.length, (u32)req_size);
		return -EFAULT;
	}
	swing_log_info("swing_get_shb_dmd_report: dmd_id[%u], dmd_case[0x%x], length[%u], tranid[%u], para_num [%u]\n",
					req->dmd_id, req->dmd_case, req->hd.length, req->hd.tranid, req->resv1);

	if (req->dmd_case != SWING_DMD_CASE_DETAIL) {
		if (req->dmd_id == SWING_DMD_BRIEF_CASE) {
			ret = bbox_diaginfo_record(req->dmd_id, NULL, "swing dmd_case = %u", req->dmd_case);
			return ret;
		} else {
			swing_log_err("swing_get_shb_dmd_report: dmd_id[%u] err!! dmd_case [0x%x]\n", req->dmd_id, req->dmd_case);
			return -EFAULT;
		}
	}

	index = swing_get_dmd_log_index(req->dmd_id);
	if (index >= log_arry_size) {
		swing_log_err("swing_get_shb_dmd_report: dmd_id[%u] not found!! \n", req->dmd_id);
		return -EFAULT;
	}

	if (req->resv1 != dmd_log[index].dmd_para_num) {
		swing_log_err("swing_get_shb_dmd_report: resv1[%u] not equal to dmd_para_num[%u]!! req->dmd_id[%u].\n",
						req->resv1, dmd_log[index].dmd_para_num, req->dmd_id);
		return -EFAULT;
	}

	// req->resv1: info[] arry size
	switch (req->resv1) {
	case 0:
		ret = bbox_diaginfo_record(req->dmd_id, NULL, dmd_log[index].dmd_msg);
		break;

	case 1:
		ret = bbox_diaginfo_record(req->dmd_id, NULL, dmd_log[index].dmd_msg, req->info[0]);
		break;

	case 2:
		ret = bbox_diaginfo_record(req->dmd_id, NULL, dmd_log[index].dmd_msg, req->info[0], req->info[1]);
		break;

	case 3:
		ret = bbox_diaginfo_record(req->dmd_id, NULL, dmd_log[index].dmd_msg, req->info[0], req->info[1], req->info[2]);
		break;

	case 4:
		ret = bbox_diaginfo_record(req->dmd_id, NULL, dmd_log[index].dmd_msg, req->info[0], req->info[1], req->info[2], req->info[3]);
		break;

	case 5:
		ret = bbox_diaginfo_record(req->dmd_id, NULL, dmd_log[index].dmd_msg, req->info[0], req->info[1], req->info[2], req->info[3], req->info[4]);
		break;

	default:
		swing_log_err("swing_get_shb_dmd_report: info arry size[%u] error!! dmd_id[%u].\n", req->resv1, req->dmd_id);
		return -EFAULT;
	}

	return ret;
}

void swing_record_shb_dmd_noc(u32 modid)
{
	u32 index = 0;
	size_t log_arry_size = sizeof(dmd_log) / sizeof(dmd_log[0]);

	index = swing_get_dmd_log_index(SWING_DMD_FDUL_NOC);
	if (index >= log_arry_size) {
		swing_log_err("swing_record_shb_dmd_noc: dmd_id[%u] not found!! modid [0x%x]\n", SWING_DMD_FDUL_NOC, modid);
		return;
	}

	swing_log_info("swing_record_shb_dmd_noc: record success! dmd_id[%u]  modid [0x%x]\n", SWING_DMD_FDUL_NOC, modid);
	(void)bbox_diaginfo_record(SWING_DMD_FDUL_NOC, NULL, dmd_log[index].dmd_msg, modid);
}

static int swing_sensorhub_reset_handler(void)
{
	swing_read_data_t read_data = {0};
	int ret = 0;

	swing_log_info("enter [%s]\n", __func__);
	mutex_lock(&g_swing_priv.read_mutex);

	if (kfifo_avail(&g_swing_priv.read_kfifo) < sizeof(swing_read_data_t)) {
		swing_log_err("%s read_kfifo is full, drop upload data.\n", __func__);
		ret = -EFAULT;
		goto ERR;
	}

	read_data.recv_len = sizeof(u32);
	read_data.p_recv = kzalloc(sizeof(u32), GFP_ATOMIC);
	if (read_data.p_recv == NULL) {
		swing_log_err("Failed to alloc memory for sensorhub reset message...\n");
		ret = -EFAULT;
		goto ERR;
	}

	*(u32 *)read_data.p_recv = SWING_RESET_NOTIFY;

	ret = kfifo_in(&g_swing_priv.read_kfifo, (unsigned char *)&read_data, sizeof(swing_read_data_t));
	if (ret <= 0) {
		swing_log_err("%s: kfifo_in failed \n", __func__);
		ret = -EFAULT;
		goto ERR;
	}

	mutex_unlock(&g_swing_priv.read_mutex);

	g_swing_priv.sh_recover_flag = 1;

	swing_dev_complete(&g_swing_priv.read_wait);
	swing_dev_complete(&g_swing_priv.swing_wait);

	return 0;

ERR:
	if (read_data.p_recv != NULL) {
		kfree(read_data.p_recv);
	}

	mutex_unlock(&g_swing_priv.read_mutex);

	return ret;
}

static int swing_sensorhub_reset_notifier(struct notifier_block *nb, unsigned long action, void *data)
{
	int ret = 0;

	switch (action) {
	case IOM3_RECOVERY_IDLE:
		ret = swing_sensorhub_reset_handler();
		break;
	default:
		break;
	}

	return ret;
}

static struct notifier_block swing_reboot_notify = {
	.notifier_call = swing_sensorhub_reset_notifier,
	.priority = -1,
};

static ssize_t swing_dev_read(struct file *file,
			      char __user *buf, size_t count, loff_t *pos)
{
	swing_read_data_t read_data = {0};
	u32 error = 0;
	u32 length;
	int ret = 0;

	swing_log_info("[%s]\n", __func__);
	if (buf == NULL || count == 0)
		goto ERR;

	error = swing_dev_wait_completion(&g_swing_priv.read_wait, 0);
	if (error != 0) {
		error = 0;
		swing_log_err("wait_event_interruptible failed.\n");
		goto ERR;
	}

	mutex_lock(&g_swing_priv.read_mutex);
	if (kfifo_len(&g_swing_priv.read_kfifo) < sizeof(swing_read_data_t)) {
		swing_log_err("%s: read data failed.\n", __func__);
		mutex_unlock(&g_swing_priv.read_mutex);
		goto ERR;
	}

	ret = kfifo_out(&g_swing_priv.read_kfifo, (unsigned char *)&read_data, sizeof(swing_read_data_t));
	if (ret < 0) {
		swing_log_err("%s: kfifo out failed.\n", __func__);
		mutex_unlock(&g_swing_priv.read_mutex);
		goto ERR;
	}

	if (count < read_data.recv_len) {
		length = count;
		swing_log_err("%s user buffer is too small\n", __func__);
	} else {
		length = read_data.recv_len;
	}

	swing_log_info("[%s] copy len[0x%x], count[0x%x]\n", __func__, read_data.recv_len, (u32)count);

	error = length;
	/*copy to user*/
	if (copy_to_user(buf, read_data.p_recv, length)) {
		swing_log_err("%s failed to copy to user\n", __func__);
		error = 0;
	}

	mutex_unlock(&g_swing_priv.read_mutex);

 ERR:
	if (read_data.p_recv != NULL) {
		kfree(read_data.p_recv);
		read_data.p_recv = NULL;
		read_data.recv_len = 0;
	}

	return error;
}

static ssize_t swing_dev_write(struct file *file, const char __user *data,
			       size_t len, loff_t *ppos)
{
	swing_log_info("%s need to do...\n", __func__);
	return len;
}

static int swing_dev_open(struct inode *inode, struct file *file)
{
	swing_log_info("enter %s \n", __func__);

	mutex_lock(&g_swing_priv.swing_mutex);

	if (g_swing_priv.ref_cnt != 0) {
		swing_log_warn("%s duplicate open.\n", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -EFAULT;
	}

	if (g_swing_priv.ref_cnt == 0) {
		send_cmd_from_kernel(TAG_SWING, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
	}
	file->private_data = &g_swing_priv;

	g_swing_priv.ref_cnt++;
	g_swing_priv.sh_recover_flag = 0;

	mutex_unlock(&g_swing_priv.swing_mutex);
	return 0;
}

static int swing_dev_release(struct inode *inode, struct file *file)
{
	swing_log_info("enter %s \n", __func__);

	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_log_err("%s: ref cnt is 0.\n", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -EFAULT;
	}

	g_swing_priv.ref_cnt--;

	if (g_swing_priv.ref_cnt == 0) {
		send_cmd_from_kernel(TAG_SWING, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
	}

	mutex_unlock(&g_swing_priv.swing_mutex);

	return 0;
}

static int get_swing_dev_dts_status(void)
{
	struct device_node *node = NULL;

	node = of_find_compatible_node(NULL, NULL, "hisilicon,swing-dev");
	if (node == NULL) {
		pr_warn("[%s] : no swing dev..\n", __func__);
		return -ENODEV;
	}

	if (!of_device_is_available(node)) {
		pr_warn("[%s] swing disabled..\n", __func__);
		return -ENODATA;
	}

	pr_info("[%s][enabled]\n", __func__);

	return 0;
}

static const struct file_operations swing_dev_fops = {
	.owner             = THIS_MODULE,
	.llseek            = no_llseek,
	.unlocked_ioctl    = swing_dev_ioctl,
	.open              = swing_dev_open,
	.release           = swing_dev_release,
	.read              = swing_dev_read,
	.write             = swing_dev_write,
};

static struct miscdevice swing_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "swing_dev",
	.fops =     &swing_dev_fops,
};

static int __init swing_dev_init(void)
{
	int ret = 0;

	if (is_sensorhub_disabled()) {
		swing_log_err("sensorhub disabled....\n");
		return -EFAULT;
	}

	ret = get_swing_dev_dts_status();
	if (ret != 0) {
		swing_log_warn("%s swing is disabled\n", __func__);
		return ret;
	}

	ret = misc_register(&swing_miscdev);
	if (ret != 0) {
		swing_log_err("%s cannot register miscdev err=%d\n", __func__, ret);
		return ret;
	}

	mutex_init(&g_swing_priv.read_mutex);
	mutex_init(&g_swing_priv.swing_mutex);

	swing_dev_wait_init(&g_swing_priv.swing_wait);
	swing_dev_wait_init(&g_swing_priv.read_wait);

	ret = register_mcu_event_notifier(TAG_SWING, CMD_CMN_CONFIG_RESP, swing_get_resp);
	if (ret != 0) {
		swing_log_err("[%s]: register CMD_CMN_CONFIG_RESP notifier failed. [%d]\n", __func__, ret);
		goto ERR1;
	}

	ret = register_mcu_event_notifier(TAG_SWING, CMD_LOG_REPORT_REQ, swing_get_shb_dmd_report);
	if (ret != 0) {
		swing_log_err("[%s]: register DMD CMD_LOG_REPORT_REQ notifier failed. [%d]\n", __func__, ret);
		goto ERR2;
	}

	ret = register_iom3_recovery_notifier(&swing_reboot_notify);
	if (ret < 0) {
		swing_log_err("[%s]register_iom3_recovery_notifier fail\n", __func__);
		goto ERR3;
	}

	if (kfifo_alloc(&g_swing_priv.read_kfifo,
			sizeof(swing_read_data_t) * SWING_READ_CACHE_COUNT, GFP_KERNEL)) {
		swing_log_err("%s kfifo alloc failed.\n", __func__);
		ret = -ENOMEM;
		goto ERR4;
	}

    g_swing_priv.ref_cnt = 0;

	swing_log_info("%s : init sc....\n", __func__);
	return 0;

ERR4:
	//unregister_iom3_recovery_notifier
ERR3:
	unregister_mcu_event_notifier(TAG_SWING, CMD_CMN_CONFIG_RESP, swing_get_shb_dmd_report);
ERR2:
	unregister_mcu_event_notifier(TAG_SWING, CMD_LOG_REPORT_REQ, swing_get_resp);
ERR1:
	misc_deregister(&swing_miscdev);

	swing_log_err("%s : init failed....\n", __func__);

	return ret;
}

static void __exit swing_dev_exit(void)
{
	swing_log_info("%s : enter....\n", __func__);

	kfifo_free(&g_swing_priv.read_kfifo);

	unregister_mcu_event_notifier(TAG_SWING, CMD_CMN_CONFIG_RESP, swing_get_resp);
	unregister_mcu_event_notifier(TAG_SWING, CMD_LOG_REPORT_REQ, swing_get_shb_dmd_report);
	misc_deregister(&swing_miscdev);

	return;
}

late_initcall_sync(swing_dev_init);
module_exit(swing_dev_exit);
