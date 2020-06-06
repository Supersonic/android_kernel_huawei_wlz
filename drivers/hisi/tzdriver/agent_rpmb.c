#include <linux/fs.h>
#include <linux/mmc/ioctl.h>	/* for struct mmc_ioc_rpmb */
#include <linux/mmc/card.h>	/* for struct mmc_card */
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/hisi/rpmb.h>
#include <securec.h>

#include "teek_client_constants.h"
#include "teek_ns_client.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "smc.h"
#include <linux/time.h>
#include <linux/delay.h>

typedef enum {
	sec_get_devinfo,
	sec_send_ioccmd,
	sec_rpmb_lock,
	sec_rpmb_unlock,
} rpmb_cmd_t;

#define RPMB_EMMC_CID_SIZE 32

struct rpmb_devinfo {
	uint8_t cid[RPMB_EMMC_CID_SIZE]; /* eMMC card ID */

	uint8_t rpmb_size_mult; /* EXT CSD-slice 168 "RPMB Size" */
	uint8_t rel_wr_sec_cnt; /* EXT CSD-slice 222 "Reliable Write Sector Count" */
	uint8_t tmp[2];
	uint32_t blk_size; /* RPMB blocksize */

	uint32_t max_blk_idx; /* The highest block index supported by current device */
	uint32_t access_start_blk_idx; /* The start block index SecureOS can access */

	uint32_t access_total_blk; /* The total blocks SecureOS can access */
	uint32_t tmp2;

	uint32_t mdt;	/* 1: EMMC 2: UFS */
	uint32_t support_bit_map;/* the device's support bit map, for example, if it support 1,2,32, then the value is 0x80000003 */
	uint32_t version;
	uint32_t tmp3;
};
struct rpmb_ioc {
	struct storage_blk_ioc_rpmb_data ioc_rpmb;  /* sizeof() = 72 */

	uint32_t buf_offset[STORAGE_IOC_MAX_RPMB_CMD];
	uint32_t tmp;
};

#define RPMB_CTRL_MAGIC	0x5A5A5A5A
struct rpmb_ctrl_t {
	uint32_t      magic;
	uint32_t      cmd_sn;
	uint8_t       lock_flag;
	uint8_t       tmp[3];
	enum rpmb_op_type op_type;
	union __args {
		struct rpmb_devinfo get_devinfo;
		struct rpmb_ioc send_ioccmd;
	} args;
	rpmb_cmd_t    cmd;
	uint32_t      reserved;
	uint32_t      buf_len;
	uint16_t      head_crc;
	uint16_t      buf_crc;
	int32_t       ret;
	uint32_t      reserved2;
	uint32_t      buf_start[0];
};/* sizeof() = 8 * 16 = 128 */

static struct rpmb_ctrl_t *m_rpmb_ctrl = NULL;
/*
 * the data_ptr from SecureOS is physical address,
 * so, we MUST update to the virtual address,
 * otherwise, segment default
 */
static void update_dataptr(struct rpmb_ctrl_t *trans_ctrl)
{
	uint32_t i, offset = 0;
	uint8_t *dst = NULL;

	if (trans_ctrl == NULL)
		return;

	for (i = 0; i < STORAGE_IOC_MAX_RPMB_CMD; i++) {
		offset = trans_ctrl->args.send_ioccmd.buf_offset[i];
		if (trans_ctrl->args.send_ioccmd.ioc_rpmb.data[i].buf) {
			dst = (uint8_t *) trans_ctrl->buf_start + offset;
			/* update the data_ptr */
			trans_ctrl->args.send_ioccmd.ioc_rpmb.data[i].buf = dst;
		}
	}
}

struct rpmb_agent_lock_info {
	unsigned int dev_id;
	bool lock_need_free;
};
static struct rpmb_agent_lock_info lock_info = { 0 };

static int process_rpmb_lock(struct tee_agent_kernel_ops *agent_instance)
{
	struct smc_event_data *event_data = NULL;

	if (agent_instance == NULL)
		return -1;

	mutex_lock(&rpmb_counter_lock);
	tlogd("obtain rpmb device lock\n");

	event_data = find_event_control(agent_instance->agent_id);
	if (event_data) {
		lock_info.dev_id = event_data->cmd.dev_file_id;
		lock_info.lock_need_free = true;
		tlogd("rpmb counter lock context: dev_id=%d\n",
		      lock_info.dev_id);
	}
	put_agent_event(event_data);
	return 0;
}

static int process_rpmb_unlock(struct tee_agent_kernel_ops *agent_instance)
{
	errno_t rc = EOK;

	if (agent_instance == NULL)
		return -1;

	rc = memset_s(&lock_info, sizeof(lock_info), 0,
		sizeof(lock_info));
	if (rc != EOK)
		return -1;

	lock_info.lock_need_free = false;
	mutex_unlock(&rpmb_counter_lock);
	tlogd("free rpmb device lock\n");
	return 0;
}

#define GET_RPMB_LOCK_MASK 0x01
#define FREE_RPMB_LOCK_MASK 0x02
static void send_ioccmd(struct tee_agent_kernel_ops *agent_instance)
{
	uint8_t lock_flag;
	int32_t ret;

	if (agent_instance == NULL || m_rpmb_ctrl == NULL) {
		tloge("bad parameters\n");
		return;
	}

	lock_flag = m_rpmb_ctrl->lock_flag;


	if (lock_flag & GET_RPMB_LOCK_MASK)
		process_rpmb_lock(agent_instance);

	ret = hisi_rpmb_ioctl_cmd(RPMB_FUNC_ID_SECURE_OS, m_rpmb_ctrl->op_type,
		&m_rpmb_ctrl->args.send_ioccmd.ioc_rpmb);
	if (ret)
		tloge("hisi_rpmb_ioctl_cmd failed: %d\n", ret);

	if (lock_flag & FREE_RPMB_LOCK_MASK)
		process_rpmb_unlock(agent_instance);
	m_rpmb_ctrl->ret = ret;
}

static void dump_memory(uint8_t *data, uint32_t count)
{
	uint32_t i;
	int j;
	uint32_t *p = NULL;
	uint8_t  buffer[256];

	if (data == NULL)
		return;

	p = (uint32_t *)data;
	for (i = 0; i < count / 16 ; i++) {
		j = snprintf_s((char *)buffer, sizeof(buffer), 64, "%x: ", (i * 16));
		if (j < 0)
			break;
		j = snprintf_s((char *)(buffer + j), sizeof(buffer) - j, 64,
			"%08x %08x %08x %08x ", *p, *(p + 1),
			*(p + 2), *(p + 3));
		if (j < 0)
			break;
		p += 4;
		tloge("%s\n", buffer);
	}

	if (count % 16) {
		j = snprintf_s((char *)buffer, sizeof(buffer), 64, "%x: ",
			((count / 16) * 16));
		for (i = 0; i < 4; i++) {
			if (j < 0)
				break;
			j += snprintf_s((char *)(buffer + j), sizeof(buffer) - j,
				64, "%08x ", *p++);
		}
		tloge("%s\n", (char *)buffer);
	}

}

static int rpmb_check_data(struct rpmb_ctrl_t *trans_ctrl)
{
	if (trans_ctrl == NULL)
		return 0;

	if (trans_ctrl->magic != RPMB_CTRL_MAGIC) {
		tloge("rpmb check magic error, now is 0x%x\n",
			trans_ctrl->magic);
		dump_memory((uint8_t *)trans_ctrl,
			(uint32_t)sizeof(*m_rpmb_ctrl));
		return -1;
	}

	return 0;
}
static uint32_t m_cmd_sn;
u64  g_ioctl_start_time = 0;
u64  g_ioctl_end_time = 0;
struct timeval tv;

static int rpmb_agent_work(struct tee_agent_kernel_ops *agent_instance)
{
	struct rpmb_ctrl_t *trans_ctrl = NULL;
	errno_t rc = EOK;
	uint32_t copy_len;

	if (agent_instance == NULL || agent_instance->agent_buffer == NULL ||
	    agent_instance->agent_buffer->kernel_addr == NULL) {
		return -1;
        }

	trans_ctrl =
		(struct rpmb_ctrl_t *)agent_instance->agent_buffer->kernel_addr;
	if (rpmb_check_data(trans_ctrl) == 0) {
		if (m_cmd_sn != trans_ctrl->cmd_sn) {
			if (m_cmd_sn) {
				tv = ns_to_timeval(g_ioctl_end_time -
					g_ioctl_start_time);
				tlogd("cmd_sn %x, total cost %d.%d s\n",
					trans_ctrl->cmd_sn, (uint32_t)tv.tv_sec,
					(uint32_t)tv.tv_usec);
			}
			g_ioctl_start_time = hisi_getcurtime();
			m_cmd_sn = trans_ctrl->cmd_sn;
		}

		if (m_rpmb_ctrl == NULL) {
			m_rpmb_ctrl = kzalloc(agent_instance->agent_buffer->len,
				GFP_KERNEL);
			if (m_rpmb_ctrl == NULL) {
				tloge("memory alloc failed\n");
				trans_ctrl->ret = TEEC_ERROR_OUT_OF_MEMORY;
				return -1;
			}

		}
		rc = memcpy_s((void *)m_rpmb_ctrl,
			agent_instance->agent_buffer->len, (void *)trans_ctrl,
			sizeof(*m_rpmb_ctrl) + trans_ctrl->buf_len);
		if (rc != EOK) {
			tloge("memcpy_s failed: 0x%x\n", rc);
			trans_ctrl->ret = TEEC_ERROR_SECURITY;
			kfree(m_rpmb_ctrl);
			m_rpmb_ctrl = NULL;
			return -1;
		}

		update_dataptr(m_rpmb_ctrl);
		switch (m_rpmb_ctrl->cmd) {
		case sec_get_devinfo:	/* stb used */
			tlogd("rpmb agent cmd is get_devinfo\n");
			// from Chicago this function not supported yet
			m_rpmb_ctrl->ret = -1;
			break;
		case sec_send_ioccmd:
			tlogd("rpmb agent cmd is send ioc\n");
			send_ioccmd(agent_instance);
			break;
		case sec_rpmb_lock:
			tlogd("rpmb agent cmd is lock\n");
			process_rpmb_lock(agent_instance);
			m_rpmb_ctrl->ret = 0;
			break;
		case sec_rpmb_unlock:
			tlogd("rpmb agent cmd is unlock\n");
			process_rpmb_unlock(agent_instance);
			m_rpmb_ctrl->ret = 0;
			break;
		default:
			tloge("rpmb agent cmd not supported 0x%x\n",
				m_rpmb_ctrl->cmd);
			break;
		}

		copy_len = agent_instance->agent_buffer->len -
			offsetof(struct rpmb_ctrl_t, buf_start);
		rc = memcpy_s((void *)trans_ctrl->buf_start, copy_len,
			(void *)m_rpmb_ctrl->buf_start, copy_len);
		if (rc != EOK) {
			tloge("memcpy_s 2 failed: 0x%x\n", rc);
			trans_ctrl->ret = TEEC_ERROR_SECURITY;
			kfree(m_rpmb_ctrl);
			m_rpmb_ctrl = NULL;
			return -1;
		}
		trans_ctrl->ret = m_rpmb_ctrl->ret;
	} else {
		trans_ctrl->ret = TEEC_ERROR_BAD_FORMAT;
		return -1;
	}

	g_ioctl_end_time = hisi_getcurtime();


	return 0;
}
/*lint +e613*/
static int rpmb_agent_exit(struct tee_agent_kernel_ops *agent_instance)
{
	tloge("rpmb agent is exit is being invoked\n");

	if (m_rpmb_ctrl != NULL) {
		kfree(m_rpmb_ctrl);
		m_rpmb_ctrl = NULL;
	}

	return 0;
}

static int rpmb_agent_crash_work(
	struct tee_agent_kernel_ops *agent_instance,
	tc_ns_client_context *context, unsigned int dev_file_id)
{
	tlogd("check free lock or not, dev_id=%d\n", dev_file_id);
	if (lock_info.lock_need_free && (lock_info.dev_id == dev_file_id)) {
		tloge("CA crash, need to free lock\n");
		process_rpmb_unlock(agent_instance);
	}
	return 0;
}

static struct tee_agent_kernel_ops rpmb_agent_ops = {
	.agent_name = "rpmb",
	.agent_id = TEE_RPMB_AGENT_ID,
	.tee_agent_init = NULL,
	.tee_agent_work = rpmb_agent_work,
	.tee_agent_exit = rpmb_agent_exit,
	.tee_agent_crash_work = rpmb_agent_crash_work,

	.list = LIST_HEAD_INIT(rpmb_agent_ops.list)
};

int rpmb_agent_register(void)
{
	tee_agent_kernel_register(&rpmb_agent_ops);
	return 0;
}

EXPORT_SYMBOL(rpmb_agent_register);
