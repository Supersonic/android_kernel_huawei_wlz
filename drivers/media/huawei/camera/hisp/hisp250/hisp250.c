 /*
 *Hisilicon K3 SOC camera driver source file
 *
 *Copyright (C) Huawei Technology Co., Ltd.
 *
 *Author:
 *Email:
 *Date:
 *
 *This program is free software; you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation; either version 2 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <linux/pm_qos.h>
#include <clocksource/arm_arch_timer.h>
#include <asm/arch_timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/pm_wakeup.h>
#include <linux/hisi-iommu.h>
#include <linux/platform_data/remoteproc-hisi.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <linux/mfd/hisi_pmic.h>

#include <media/huawei/camera.h>
#include <media/huawei/hisp250_cfg.h>
#include <hicam_buf.h>
#include "cam_log.h"
#include "hisp_intf.h"
#include "platform/sensor_commom.h"


#define HISP_MSG_LOG_MOD 100

#define ISP_TURBO_ISPFUNC_CLK_RATE 640000000
#define ISP_TURBO_ISPFUNC2_CLK_RATE 640000000
#define ISP_TURBO_ISPFUNC3_CLK_RATE 554000000
#define ISP_TURBO_ISPFUNC4_CLK_RATE 554000000

#define ISP_NORMAL_ISPFUNC_CLK_RATE 480000000
#define ISP_NORMAL_ISPFUNC2_CLK_RATE 480000000
#define ISP_NORMAL_ISPFUNC3_CLK_RATE 400000000
#define ISP_NORMAL_ISPFUNC4_CLK_RATE 400000000

#define ISP_LOWPOWER_ISPFUNC_CLK_RATE 332000000
#define ISP_LOWPOWER_ISPFUNC2_CLK_RATE 332000000
#define ISP_LOWPOWER_ISPFUNC3_CLK_RATE 300000000
#define ISP_LOWPOWER_ISPFUNC4_CLK_RATE 300000000

#define ISP_ULTRALOW_ISPFUNC_CLK_RATE 207500000
#define ISP_ULTRALOW_ISPFUNC2_CLK_RATE 207500000
#define ISP_ULTRALOW_ISPFUNC3_CLK_RATE 166000000
#define ISP_ULTRALOW_ISPFUNC4_CLK_RATE 185000000

#define R8_TURBO_ISPCPU_CLK_RATE 1200000000
#define R8_NORMAL_ISPCPU_CLK_RATE 960000000
#define R8_LOWPOWER_ISPCPU_CLK_RATE 960000000
#define R8_ULTRALOW_ISPCPU_CLK_RATE 960000000

#define TIMEOUT_IS_FPGA_BOARD 60000
#define TIMEOUT_IS_NOT_FPGA_BOARD 15000

DEFINE_MUTEX(hisi_rpmsg_service_mutex);

#ifdef CONFIG_HISI_DEBUG_FS
static struct pm_qos_request qos_request_ddr_down_record;
static int current_ddr_bandwidth = 0;
#endif
static struct wakeup_source hisp_power_wakelock;
static struct mutex hisp_wake_lock_mutex;
static struct mutex hisp_power_lock_mutex;
static struct mutex hisp_mem_lock_mutex;


extern void hisi_isp_boot_stat_dump(void);
extern int hisp_secmem_size_get(unsigned int *);
extern int hisp_secmem_size_get_from_type(unsigned int * size, unsigned int type);
extern unsigned int hisp_sec_mem_set(int sharefd, unsigned int type, unsigned int size, unsigned int prot);
extern int hisp_sec_mem_release(int sharefd, unsigned int type, unsigned int size, unsigned int da);
static void hisp250_deinit_isp_mem(void);
typedef enum _timestamp_state_t{
	TIMESTAMP_UNINTIAL = 0,
	TIMESTAMP_INTIAL,
} timestamp_state_t;

static timestamp_state_t s_timestamp_state;
static struct timeval s_timeval;
static u32 s_system_couter_rate;
static u64 s_system_counter;

enum hisp250_rpmsg_state {
	RPMSG_UNCONNECTED,
	RPMSG_CONNECTED,
	RPMSG_FAIL,
};

enum isp_clk_level {
	ISP_CLK_LEVEL_TURBO,
	ISP_CLK_LEVEL_NORMAL,
	ISP_CLK_LEVEL_LOWPOWER,
	ISP_CLK_LEVEL_ULTRALOW,
	ISP_CLK_LEVEL_MAX,
};

/*
 *These are used for distinguish the rpmsg_msg status
 *The process in hisp250_rpmsg_ept_cb are different
 *for the first receive and later.
 */
enum {
	HISP_SERV_FIRST_RECV,
	HISP_SERV_NOT_FIRST_RECV,
};

/**@brief the instance for rpmsg service
 *
 *When Histar ISP is probed, this sturcture will be initialized,
 *the object is used to send/recv rpmsg and store the rpmsg data
 *
 *@end
 */
struct rpmsg_hisp250_service {
	struct rpmsg_device *rpdev;
	struct mutex send_lock;
	struct mutex recv_lock;
	struct completion *comp;
	struct sk_buff_head queue;
	wait_queue_head_t readq;
	struct rpmsg_endpoint *ept;
	u32 dst;
	int state;
	char recv_count;
};


//#define SUPPORT_IRIS_SMP
enum hisp250_mem_pool_attr
{
	MEM_POOL_ATTR_READ_WRITE_CACHE = 0,
	MEM_POOL_ATTR_READ_WRITE_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY,
	MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE,
	MEM_POOL_ATTR_MAX,
};

struct hisp250_mem_pool {
	void *ap_va;
	unsigned int prot;
	unsigned int ion_iova;
	unsigned int r8_iova;
	size_t size;
	size_t align_size;
	int active;
	unsigned int security_isp_mode;
	struct sg_table *sgt;
	unsigned int sharedFd;
	unsigned int isApCached;
} ;

struct isp_mem {
	int active;
	struct dma_buf *dmabuf;
};
/**@brief the instance to talk to hisp driver
 *
 *When Histar ISP is probed, this sturcture will be initialized,
 *the object is used to notify hisp driver when needed.
 *
 *@end
 */
typedef struct _tag_hisp250 {
	hisp_intf_t intf;
	hisp_notify_intf_t *notify;
	char const *name;
	atomic_t opened;
	struct platform_device*  pdev; //by used to get dts node
	hisp_dt_data_t dt;
	struct hisp250_mem_pool mem_pool[MEM_POOL_ATTR_MAX];
	struct isp_mem mem;
} hisp250_t;

struct rpmsg_service_info {
	struct rpmsg_hisp250_service *hisi_isp_serv;
	struct completion isp_comp;
	int isp_minor;
};

extern void a7_mmu_unmap(unsigned int va, unsigned int size);

/*Store the only rpmsg_hisp250_service pointer to local static rpmsg_local*/
static struct rpmsg_service_info rpmsg_local;
static bool remote_processor_up = false;
static int g_hisp_ref = 0;

#define I2HI(i) container_of(i, hisp250_t, intf)

static void hisp250_notify_rpmsg_cb(void);
char const *hisp250_get_name(hisp_intf_t *i);
static int hisp250_config(hisp_intf_t *i, void *cfg);

static int hisp250_power_on(hisp_intf_t *i);
static int hisp250_power_off(hisp_intf_t *i);

static int hisp250_open(hisp_intf_t *i);
static int hisp250_close(hisp_intf_t *i);
static int hisp250_send_rpmsg(hisp_intf_t *i, hisp_msg_t *m, size_t len);
static int hisp250_recv_rpmsg(hisp_intf_t *i,
				  hisp_msg_t *user_addr, size_t len);

static int hisp250_set_sec_fw_buffer(struct hisp_cfg_data *cfg);
static int hisp250_release_sec_fw_buffer(void);

#ifdef CONFIG_HISI_DEBUG_FS
static void hisp250_set_ddrfreq(int ddr_bandwidth);
static void hisp250_release_ddrfreq(void);
static void hisp250_update_ddrfreq(unsigned int ddr_bandwidth);
#endif


#define PMIC_REG_ADDR 0x96
static int lock_ref;
static int hisp250_lock_voltage(uint32_t lock)
{
	uint32_t read_val = 0;
	int write_val = lock == 0 ? 0x00 : 0x02;

	if (lock == 0 && lock_ref <= 0) {
		cam_err("%s: not locked", __func__);
		return 0;
	}

	if (lock == 0)
		--lock_ref;
	else
		++lock_ref;

	hisi_pmic_reg_write(PMIC_REG_ADDR, write_val);
	read_val = hisi_pmic_reg_read(PMIC_REG_ADDR);

	cam_info("%s: %s -> pmic_addr:%#x, write_val:%#x, read_val:%#x",
		__func__, lock == 0 ? "unlock" : "lock",
		PMIC_REG_ADDR, write_val, read_val);

	return 0;
}

void hisp250_init_timestamp(void);
void hisp250_destroy_timestamp(void);
void hisp250_set_timestamp(unsigned int *timestampH, unsigned int *timestampL);
void hisp250_handle_msg(hisp_msg_t *msg);

void hisp250_init_timestamp(void)
{
	s_timestamp_state  		= TIMESTAMP_INTIAL;
	s_system_counter  		= arch_counter_get_cntvct();
	s_system_couter_rate 	= arch_timer_get_rate();
	do_gettimeofday(&s_timeval);

	cam_debug("%s state=%u system_counter=%llu rate=%u"
		" time_second=%ld time_usecond=%ld size=%lu",
		__func__,
		(unsigned int)s_timestamp_state,
		s_system_counter,
		s_system_couter_rate,
		s_timeval.tv_sec,
		s_timeval.tv_usec,
		sizeof(s_timeval)/sizeof(u32));//FIXME
}

void hisp250_destroy_timestamp(void)
{
	s_timestamp_state		= TIMESTAMP_UNINTIAL;
	s_system_counter 		= 0;
	s_system_couter_rate	= 0;
	memset(&s_timeval, 0x00, sizeof(s_timeval));
}

/*Function declaration */
/**********************************************
 * |-----pow-on------->||<----  fw-SOF ---->|
 *  timeval(got)       ----------------->fw_timeval=?
 *  system_counter(got)----------------->fw_sys_counter(got)
 *
 *  fw_timeval = timeval + (fw_sys_counter - system_counter)
 *
 *With a base position(<timeval, system_counter>, we get it at same time),
 *we can calculate fw_timeval with fw syscounter
 *and deliver it to hal. Hal then gets second and microsecond
 *********************************************/
void hisp250_set_timestamp(unsigned int *timestampH, unsigned int *timestampL)
{
#define MICROSECOND_PER_SECOND 	(1000000)
	u64 fw_micro_second = 0;
	u64 fw_sys_counter = 0;
	u64 micro_second = 0;

	if (TIMESTAMP_UNINTIAL ==  s_timestamp_state) {
		cam_err("%s wouldn't enter this branch.", __func__);
		hisp250_init_timestamp();
	}

	if (timestampH == NULL || timestampL == NULL) {
		cam_err("%s timestampH or timestampL is null.", __func__);
		return;
	}

	cam_debug("%s ack_high:0x%x ack_low:0x%x", __func__,
		*timestampH, *timestampL);

	if (*timestampH == 0 && *timestampL == 0) {
		return;
	}

	fw_sys_counter = ((u64)(*timestampH) << 32) | // 32 for Bit operations
		(u64)(*timestampL);
	micro_second = (fw_sys_counter - s_system_counter) * MICROSECOND_PER_SECOND / s_system_couter_rate;

	//chang nano second to micro second
	fw_micro_second =
		(micro_second / MICROSECOND_PER_SECOND + s_timeval.tv_sec) * MICROSECOND_PER_SECOND
		+ ((micro_second % MICROSECOND_PER_SECOND) + s_timeval.tv_usec);

	*timestampH = (u32)(fw_micro_second >> 32 & 0xFFFFFFFF); // 32 for Bit operations
	*timestampL = (u32)(fw_micro_second & 0xFFFFFFFF);

	cam_debug("%s h:0x%x l:0x%x", __func__, *timestampH, *timestampL);
}

void hisp250_handle_msg(hisp_msg_t *msg)
{
	if (NULL == msg)
		return;
	switch (msg->api_name) {
	case BATCH_REQUEST_RESPONSE:
		msg->u.ack_batch_request.system_couter_rate = s_system_couter_rate;
		cam_info("%s batch h:0x%x l:0x%x, rate %d",
			__func__,
			msg->u.ack_batch_request.timestampH,
			msg->u.ack_batch_request.timestampL,
			msg->u.ack_batch_request.system_couter_rate);
		hisp250_set_timestamp(&(msg->u.ack_batch_request.timestampH), &(msg->u.ack_batch_request.timestampL));
		break;
	case REQUEST_RESPONSE:
		hisp250_set_timestamp(&(msg->u.ack_request.timestampH), &(msg->u.ack_request.timestampL));
		break;
	case MSG_EVENT_SENT:
		hisp250_set_timestamp(&(msg->u.event_sent.timestampH), &(msg->u.event_sent.timestampL));
		break;
	default:
		break;
	}
}

static hisp_vtbl_t s_vtbl_hisp250 = {
	.get_name = hisp250_get_name,
	.config = hisp250_config,
	.power_on = hisp250_power_on,
	.power_off = hisp250_power_off,
	.send_rpmsg = hisp250_send_rpmsg,
	.recv_rpmsg = hisp250_recv_rpmsg,
	.open = hisp250_open,
	.close = hisp250_close,
};

static hisp250_t s_hisp250 = {
	.intf = {.vtbl = &s_vtbl_hisp250,},
	.name = "hisp250",
};

static void hisp250_notify_rpmsg_cb(void)
{
	hisp_event_t isp_ev;
	isp_ev.kind = HISP_RPMSG_CB;
	hisp_notify_intf_rpmsg_cb(s_hisp250.notify, &isp_ev);
}


/*Function declaration */
/**********************************************
 *Save the rpmsg from isp to locally skb queue.
 *Only called by hisp250_rpmsg_ept_cb when api_name
 *is NOT POWER_REQ, will notify user space through HISP
 *********************************************/
static void hisp250_save_rpmsg_data(const void *data, int len)
{
	struct rpmsg_hisp250_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	struct sk_buff *skb = NULL;
	char *skbdata = NULL;

	if (NULL == hisi_serv) {
		cam_err("%s: hisi_serv is NULL",__func__);
		return;
	}
	hisp_assert(NULL != data);
	if (NULL == data) {
		return;
	}
	hisp_assert(len > 0);

	skb = alloc_skb(len, GFP_KERNEL);
	if (skb == NULL) {
		cam_err("%s() %d failed: alloc_skb len is %u!", __func__,
			__LINE__, len);
		return;
	}

	skbdata = skb_put(skb, len);
	memcpy(skbdata, data, len);

	/*add skb to skb queue */
	mutex_lock(&hisi_serv->recv_lock);
	skb_queue_tail(&hisi_serv->queue, skb);
	mutex_unlock(&hisi_serv->recv_lock);

	wake_up_interruptible(&hisi_serv->readq);
	hisp250_notify_rpmsg_cb();
}

/*Function declaration */
/**********************************************
 *Power up CSI/DPHY/sensor according to isp req
 *Only called by hisp250_rpmsg_ept_cb when api_name
 *is POWER_REQ, and will send a POWER_RSP to isp
 *after power request done.
 *********************************************/

static int
hisp250_rpmsg_ept_cb(struct rpmsg_device *rpdev,
			 void *data, int len, void *priv, u32 src)
{
	struct rpmsg_hisp250_service *hisi_serv = rpmsg_local.hisi_isp_serv;
	hisp_msg_t *msg = NULL;
	struct rpmsg_hdr *rpmsg_msg = NULL;

	if (NULL == hisi_serv) {
		cam_err("func %s: hisi_serv is NULL",__func__);
		return -EINVAL;
	}
	if (NULL == data) {
		cam_err("func %s: data is NULL",__func__);
		return -EINVAL;
	}

	hisp_assert(len > 0);

	if (RPMSG_CONNECTED != hisi_serv->state) {
		hisp_assert(RPMSG_UNCONNECTED == hisi_serv->state);
		rpmsg_msg = container_of(data, struct rpmsg_hdr, data);
		cam_info("msg src.%u, msg dst.%u", rpmsg_msg->src,
			  rpmsg_msg->dst);

		/*add instance dst and modify the instance state */
		hisi_serv->dst = rpmsg_msg->src;
		hisi_serv->state = RPMSG_CONNECTED;
	}

	msg = (hisp_msg_t *) (data);
	/* save the data and wait for hisp250_recv_rpmsg to get the data*/
	hisp_recvx(data);
	hisp250_save_rpmsg_data(data, len);
	return 0;
}

char const *hisp250_get_name(hisp_intf_t *i)
{
	hisp250_t *hi = NULL;
	hisp_assert(NULL != i);
	hi = I2HI(i);
	if (NULL == hi) {
		cam_err("func %s: hi is NULL",__func__);
		return NULL;
	}
	return hi->name;
}

static int buffer_is_invalid(int share_fd, unsigned int req_addr, unsigned int req_size)
{
	int ret;
	struct iommu_format fmt = { 0 };

	ret = hicam_buf_map_iommu(share_fd, &fmt);
	if (ret < 0) {
		cam_err("%s: fail to map iommu.", __func__);
		return ret;
	}

	if (req_addr != fmt.iova || req_size > fmt.size) {
		cam_err("%s: req_iova:%#x,  req_size:%u", __func__, req_addr, req_size);
		cam_err("%s:real_iova:%#llx, real_size:%llu.", __func__, fmt.iova, fmt.size);
		ret = -ERANGE;
	}
	hicam_buf_unmap_iommu(share_fd, &fmt);

	return ret;
}

static int find_suitable_mem_pool(struct hisp_cfg_data *pcfg)
{
	int ipool;
	if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
		ipool = MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE;
	} else if (pcfg->param.type == MAP_TYPE_STATIC_ISP_SEC) {
		ipool = MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY;
	} else {
		for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
			if (s_hisp250.mem_pool[ipool].prot == pcfg->param.prot) {
				break;
			}
		}

		if (ipool >= MEM_POOL_ATTR_MAX) {
			cam_err("func %s: no pool hit for prot:%d",
				__func__, pcfg->param.prot);
			return -EINVAL;
		}
	}
	return ipool;
}

static int hisp250_init_r8isp_memory_pool(void *cfg)
{
	int ipool;
	uint32_t r8va;
    uint32_t iova;
	struct hisp_cfg_data *pcfg = NULL;
	struct sg_table *sgt = NULL;
	enum mapType enMapType;

	if (NULL == cfg) {
		cam_err("%s: cfg is NULL", __func__);
		return -EINVAL;
	}

	pcfg = (struct hisp_cfg_data *)cfg;
	cam_info("%s: pool cfg vaddr=0x%pK, iova=0x%x, size=0x%x", __func__,
		pcfg->param.vaddr, pcfg->param.iova, pcfg->param.size);
	cam_info("%s: type=%d, prot=0x%x align=0x%zd sec=0x%x", __func__,
		pcfg->param.type, pcfg->param.prot,
		pcfg->param.pool_align_size, pcfg->param.security_isp_mode);

	if (!pcfg->param.security_isp_mode &&
		buffer_is_invalid(pcfg->param.sharedFd, pcfg->param.iova, pcfg->param.size)) {
		cam_err("check buffer fail!");
		return -EINVAL;
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0) {
		return ipool;
	}
	enMapType = pcfg->param.type;
	if (enMapType != MAP_TYPE_DYNAMIC_SEC) {
		if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE) {
			enMapType = MAP_TYPE_RAW2YUV;
		} else {
			enMapType = pcfg->param.security_isp_mode ?
				MAP_TYPE_STATIC_SEC : MAP_TYPE_DYNAMIC;
		}
	}

	/* take care of putting sgtable. */
	sgt = hicam_buf_get_sgtable(pcfg->param.sharedFd);
	if (IS_ERR(sgt)) {
		cam_err("%s: fail to get sgtable.", __func__);
		return -ENOENT;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_SEC) {
		iova = hisp_sec_mem_set(pcfg->param.sharedFd, HISP_DYNAMIC_SEC, pcfg->param.size, pcfg->param.prot);

		if (iova == 0) {
			cam_err("%s: hisp_sec_mem_set failed", __func__);
			mutex_unlock(&hisi_rpmsg_service_mutex);
			hicam_buf_put_sgtable(sgt);
			return -ENOMEM;
		}
	}

	r8va = hisp_mem_map_setup(sgt->sgl, pcfg->param.iova, pcfg->param.size,
			pcfg->param.prot, (unsigned int)ipool, enMapType,
			(unsigned int)(pcfg->param.pool_align_size));

	if (!r8va) {
		cam_err("%s: hisp_mem_map_setup failed", __func__);
		(void)hisp_sec_mem_release(pcfg->param.sharedFd, HISP_DYNAMIC_SEC, pcfg->param.size, pcfg->param.iova);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		hicam_buf_put_sgtable(sgt);
		return -ENOMEM;
	}

	/* hold sg_table things, release at deinit. */
	s_hisp250.mem_pool[ipool].sgt = sgt;
	s_hisp250.mem_pool[ipool].r8_iova = r8va;
	s_hisp250.mem_pool[ipool].ap_va = pcfg->param.vaddr;
	s_hisp250.mem_pool[ipool].ion_iova = pcfg->param.iova;
	s_hisp250.mem_pool[ipool].size = pcfg->param.size;
	s_hisp250.mem_pool[ipool].align_size = pcfg->param.pool_align_size;
	s_hisp250.mem_pool[ipool].security_isp_mode = pcfg->param.security_isp_mode;
	s_hisp250.mem_pool[ipool].sharedFd = pcfg->param.sharedFd;
	s_hisp250.mem_pool[ipool].isApCached = pcfg->param.isApCached;

	// ion iova isn't equal r8 iova, security or unsecurity, align etc
	// return r8 iova to daemon, and send to r8 later
	pcfg->param.iova = r8va;
	s_hisp250.mem_pool[ipool].active = 1;

	cam_info("%s: r8_iova_pool_base=0x%x",__func__, s_hisp250.mem_pool[ipool].r8_iova);
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return 0;
}

static int hisp250_deinit_r8isp_memory_pool(void *cfg)
{
	int ipool;
	struct hisp_cfg_data *pcfg = NULL;
	int ret;

	if (NULL == cfg) {
		cam_err("func %s: cfg is NULL",__func__);
		return -EINVAL;
	}

	ipool = find_suitable_mem_pool(cfg);
	if (ipool < 0) {
		return ipool;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);
	pcfg = (struct hisp_cfg_data *)cfg;
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_SEC) {
		ret = hisp_sec_mem_release(pcfg->param.sharedFd, HISP_DYNAMIC_SEC, pcfg->param.size, pcfg->param.iova);
		if (ret != 0) {
			cam_err("%s: hisp_sec_mem_release failed", __func__);
		}
	}

	if (s_hisp250.mem_pool[ipool].active) {
		s_hisp250.mem_pool[ipool].active = 0;
		hisp_mem_pool_destroy((unsigned int)ipool);
		/* release sg_table things. */
		hicam_buf_put_sgtable(s_hisp250.mem_pool[ipool].sgt);
	}
	memset(&(s_hisp250.mem_pool[ipool]), 0, sizeof(struct hisp250_mem_pool));
	mutex_unlock(&hisi_rpmsg_service_mutex);

	return 0;
}

// handle daemon carsh
// miss ispmanager poweroff
// miss memory pool deinit
static int hisp250_deinit_r8isp_memory_pool_force(void)
{
	int ipool = 0;

	cam_warn("func %s", __func__);

	mutex_lock(&hisi_rpmsg_service_mutex);
	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (s_hisp250.mem_pool[ipool].active) {
			cam_warn("%s: force deiniting pool:%d", __func__, ipool);
			s_hisp250.mem_pool[ipool].active = 0;
			hisp_mem_pool_destroy((unsigned int)ipool);
			hicam_buf_put_sgtable(s_hisp250.mem_pool[ipool].sgt);
		}
		memset(&(s_hisp250.mem_pool[ipool]), 0, sizeof(struct hisp250_mem_pool));
	}

	mutex_unlock(&hisi_rpmsg_service_mutex);
	return 0;
}

static int hisp250_alloc_r8isp_addr(void *cfg)
{
	int ipool = 0;
	unsigned int r8_iova = 0;
	size_t  offset = 0;
	struct hisp_cfg_data *pcfg = NULL;
	int rc = 0;
	bool bSecureMode = false;

	if (NULL == cfg) {
		cam_err("func %s: cfg is NULL",__func__);
		return -1;
	}

	pcfg = (struct hisp_cfg_data *)cfg;
	//handle static memory, just return r8 reserved iova address == map only
	if (pcfg->param.type == MAP_TYPE_STATIC) {
#ifndef CONFIG_HISI_CAMERA_ISP_SECURE
		return -ENODEV;
#else
		cam_debug("func %s static", __func__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		pcfg->param.iova = a7_mmu_map(NULL, pcfg->param.size,
				pcfg->param.prot, pcfg->param.type);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
#endif
	}

	// handle dynamic carveout alloc
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_CARVEOUT) {
		cam_debug("func %s dynamic carveout", __func__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		pcfg->param.iova = hisp_mem_pool_alloc_carveout(pcfg->param.size, pcfg->param.type);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
	}

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (s_hisp250.mem_pool[ipool].security_isp_mode) {
			bSecureMode = true;
			break;
		}
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0) {
		return ipool;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);
	r8_iova = (unsigned int)hisp_mem_pool_alloc_iova(pcfg->param.size, (unsigned int)ipool);
	if (!r8_iova) {
		cam_err("func %s: hisp_mem_pool_alloc_iova error",__func__);
		rc = -ENOMEM;
		goto alloc_err;
	}

	// offset calculator
	// security mode, pool base is r8_iova, is security address, not align
	// normal mode, pool base is ion_iova, is normal address,  align by isp.
	if (pcfg->param.type == MAP_TYPE_RAW2YUV) {
		offset = r8_iova - s_hisp250.mem_pool[ipool].r8_iova;
	} else {
		if (bSecureMode) {
			offset = r8_iova - s_hisp250.mem_pool[ipool].r8_iova;
		} else {
			offset = r8_iova - s_hisp250.mem_pool[ipool].ion_iova;
		}
	}

	// FIXME: redundant check
	if (offset > s_hisp250.mem_pool[ipool].size) {
		cam_err("func %s: r8_iova invalid",__func__);
		rc = -EFAULT;
		goto check_err;
	}

	pcfg->param.vaddr = (void *)(((unsigned char *)s_hisp250.mem_pool[ipool].ap_va) + offset);
	pcfg->param.iova = r8_iova;
	pcfg->param.offset_in_pool = offset;
	pcfg->param.sharedFd = s_hisp250.mem_pool[ipool].sharedFd;
	pcfg->param.isApCached = s_hisp250.mem_pool[ipool].isApCached;
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return rc;

check_err:
	rc = (int)hisp_mem_pool_free_iova((unsigned int)ipool, r8_iova, pcfg->param.size);
	if (rc) {
		cam_err("func %s: hisp_mem_pool_free_iova error", __func__);
	}
alloc_err:
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return rc;
}

static int hisp250_free_r8isp_addr(void *cfg)
{
	int rc = 0;
	int ipool = 0;
	struct hisp_cfg_data *pcfg = NULL;

	if (NULL == cfg) {
		cam_err("func %s: cfg is NULL",__func__);
		return -EINVAL;
	}


	pcfg = (struct hisp_cfg_data *)cfg;

	//handle static memory, unmap only
	if (pcfg->param.type == MAP_TYPE_STATIC) {
#ifndef CONFIG_HISI_CAMERA_ISP_SECURE
		return -ENODEV;
#else
		cam_debug("func %s static", __func__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		a7_mmu_unmap(pcfg->param.iova, pcfg->param.size);
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return 0;
#endif
	}


	// handle dynamic carveout free
	if (pcfg->param.type == MAP_TYPE_DYNAMIC_CARVEOUT) {
		cam_debug("func %s dynamic carveout", __func__);
		mutex_lock(&hisi_rpmsg_service_mutex);
		rc = hisp_mem_pool_free_carveout(pcfg->param.iova, pcfg->param.size);
		if (rc) {
			cam_err("func %s: hisp_mem_pool_free_carveout error", __func__);
		}
		mutex_unlock(&hisi_rpmsg_service_mutex);
		return rc;
	}

	ipool = find_suitable_mem_pool(pcfg);
	if (ipool < 0) {
		return ipool;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);
	rc = (int)hisp_mem_pool_free_iova((unsigned int)ipool, pcfg->param.iova, pcfg->param.size);
	if (rc) {
		cam_err("func %s: hisp_mem_pool_free_iova error", __func__);
	}
	mutex_unlock(&hisi_rpmsg_service_mutex);
	return rc;
}

static int hisp250_mem_pool_pre_init(void)
{
	int ipool = 0;
	int prot = 0;

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		memset(&(s_hisp250.mem_pool[ipool]), 0, sizeof(struct hisp250_mem_pool));

		switch (ipool) {
		case MEM_POOL_ATTR_READ_WRITE_CACHE:
		case MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE:
			prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE;
			break;

		case MEM_POOL_ATTR_READ_WRITE_SECURITY:
		case MEM_POOL_ATTR_READ_WRITE_ISP_SECURITY:
			prot = IOMMU_READ | IOMMU_WRITE | IOMMU_CACHE | IOMMU_SEC;
			break;

		default:
			prot = -1;
			break;
		}

		cam_debug("%s  ipool %d prot 0x%x", __func__, ipool, prot);

		if (prot < 0) {
			cam_err("%s unkown ipool %d prot 0x%x", __func__, ipool, prot);
			return -EINVAL;
		}

		s_hisp250.mem_pool[ipool].prot = (unsigned int)prot;
	}

	return 0;
}

static int hisp250_mem_pool_later_deinit(void)
{
	int ipool = 0;
	cam_debug("%s", __func__);

	for (ipool = 0; ipool < MEM_POOL_ATTR_MAX; ipool++) {
		if (ipool == MEM_POOL_ATTR_READ_WRITE_CACHE_OFF_LINE) {
			continue;
		}
		if (s_hisp250.mem_pool[ipool].active) {
			cam_warn("%s: force deiniting pool:%d", __func__, ipool);
			s_hisp250.mem_pool[ipool].active = 0;
			hisp_mem_pool_destroy((unsigned int)ipool);
			hicam_buf_put_sgtable(s_hisp250.mem_pool[ipool].sgt);
		}
		memset(&s_hisp250.mem_pool[ipool], 0, sizeof(struct hisp250_mem_pool));
	}

	return 0;
}

static int hisp250_set_clk_rate(int clk_level)
{
	int rc = 0;
	int rc0 = 0;
	int rc1 = 0;
	int rc2 = 0;
	int rc3 = 0;

	switch (clk_level) {
	case ISP_CLK_LEVEL_TURBO:
		rc0 = hisp_set_clk_rate(ISPFUNC_CLK, ISP_TURBO_ISPFUNC_CLK_RATE);
		rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISP_TURBO_ISPFUNC2_CLK_RATE);
		rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISP_TURBO_ISPFUNC3_CLK_RATE);
		rc3 = hisp_set_clk_rate(ISPFUNC4_CLK, ISP_TURBO_ISPFUNC4_CLK_RATE);
		break;
	case ISP_CLK_LEVEL_NORMAL:
		rc0 = hisp_set_clk_rate(ISPFUNC_CLK, ISP_NORMAL_ISPFUNC_CLK_RATE);
		rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISP_NORMAL_ISPFUNC2_CLK_RATE);
		rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISP_NORMAL_ISPFUNC3_CLK_RATE);
		rc3 = hisp_set_clk_rate(ISPFUNC4_CLK, ISP_NORMAL_ISPFUNC4_CLK_RATE);
		break;
	case ISP_CLK_LEVEL_LOWPOWER:
		rc0 = hisp_set_clk_rate(ISPFUNC_CLK, ISP_LOWPOWER_ISPFUNC_CLK_RATE);
		rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISP_LOWPOWER_ISPFUNC2_CLK_RATE);
		rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISP_LOWPOWER_ISPFUNC3_CLK_RATE);
		rc3 = hisp_set_clk_rate(ISPFUNC4_CLK, ISP_LOWPOWER_ISPFUNC4_CLK_RATE);
		break;
	case ISP_CLK_LEVEL_ULTRALOW:
		rc0 = hisp_set_clk_rate(ISPFUNC_CLK, ISP_ULTRALOW_ISPFUNC_CLK_RATE);
		rc1 = hisp_set_clk_rate(ISPFUNC2_CLK, ISP_ULTRALOW_ISPFUNC2_CLK_RATE);
		rc2 = hisp_set_clk_rate(ISPFUNC3_CLK, ISP_ULTRALOW_ISPFUNC3_CLK_RATE);
		rc3 = hisp_set_clk_rate(ISPFUNC4_CLK, ISP_ULTRALOW_ISPFUNC4_CLK_RATE);
		break;
	}

	if (rc0 < 0 || rc1 < 0 || rc2 < 0 || rc3 < 0) {
		cam_err("%s: set clk fail, rc:%d, %d, %d, %d", __func__, rc0, rc1, rc2, rc3);
		rc = -EFAULT;
	}
	return rc;
}

static int hisp250_set_clk_rate_self_adapt(int clk_level)
{
	int rc;

	do {
		rc = hisp250_set_clk_rate(clk_level);
		if (rc == 0)
			break;
		cam_info("%s: set to clk level:%d fail, try level:%d", __func__,
			 clk_level, clk_level + 1);
		clk_level += 1; /* attention: plus one for a lower clk level. */
	} while (clk_level < ISP_CLK_LEVEL_MAX);

	return rc;
}

static int hisp250_phy_csi_connect(void *pdata)
{
	int rc = 0;
	struct msg_req_connect_camera_t *req_connect = NULL;

	if (pdata == NULL) {
		cam_err("%s: data is NULL", __func__);
		return -1;
	}
	req_connect = (struct msg_req_connect_camera_t *)(pdata);

	rc = hisp_phy_csi_connect((struct hisp_phy_info_t *)&(req_connect->phy_info), req_connect->csi_index);
	if (rc != 0) {
		cam_err("%s: phy csi connect fail:%d", __func__, rc);
	}
	return rc;
}

static int hisp250_config(hisp_intf_t *i, void *cfg)
{
	int rc = 0;
	hisp250_t *hi = NULL;
	struct hisp_cfg_data *pcfg = NULL;

	hisp_assert(NULL != i);
	if (NULL == cfg) {
		cam_err("func %s: cfg is NULL",__func__);
		return -1;
	}
	pcfg = (struct hisp_cfg_data *)cfg;
	hi = I2HI(i);
	hisp_assert(NULL != hi);

	switch (pcfg->cfgtype) {
	case HISP_CONFIG_POWER_ON:
		mutex_lock(&hisp_power_lock_mutex);
		if (!remote_processor_up) {
			if (pcfg->isSecure == 0) {
				hisi_isp_rproc_case_set(NONSEC_CASE);
			} else if (pcfg->isSecure == 1) {
				hisi_isp_rproc_case_set(SEC_CASE);
			} else {
				cam_info("%s invalid mode", __func__);
			}
			cam_notice("%s power on the hisp250.", __func__);
			rc = hisp250_power_on(i);
		} else {
			cam_warn("%s hisp250 is still on power-on state, power off it.",
				__func__);

			rc = hisp250_power_off(i);
			if (0 != rc) {
				mutex_unlock(&hisp_power_lock_mutex);
				break;
			}

			hisp250_deinit_r8isp_memory_pool_force();

			cam_warn("%s begin to power on the hisp250.", __func__);
			rc = hisp250_power_on(i);
		}
		mutex_unlock(&hisp_power_lock_mutex);
		break;
	case HISP_CONFIG_POWER_OFF:
		mutex_lock(&hisp_power_lock_mutex);
		if (remote_processor_up) {
			cam_notice("%s power off the hisp250.", __func__);
			rc = hisp250_power_off(i);
		}
		mutex_unlock(&hisp_power_lock_mutex);
		break;

	case HISP_CONFIG_INIT_MEMORY_POOL:
		rc = hisp250_init_r8isp_memory_pool(cfg);
		break;

	case HISP_CONFIG_DEINIT_MEMORY_POOL:
		rc = hisp250_deinit_r8isp_memory_pool(cfg);
		break;

	case HISP_CONFIG_ALLOC_MEM:
		rc = hisp250_alloc_r8isp_addr(cfg);
		break;

	case HISP_CONFIG_FREE_MEM:
		rc = hisp250_free_r8isp_addr(cfg);
		break;
	//Func->FE, Func2->SRT, Func3->CRAW/CBE, func4->VRAW/VBE
	case HISP_CONFIG_ISP_TURBO:
		cam_debug("%s HISP_CONFIG_ISP_TURBO", __func__);
		rc = hisp250_set_clk_rate_self_adapt(ISP_CLK_LEVEL_TURBO);
		break;
	case HISP_CONFIG_ISP_NORMAL:
		cam_debug("%s HISP_CONFIG_ISP_NORMAL", __func__);
		rc = hisp250_set_clk_rate_self_adapt(ISP_CLK_LEVEL_NORMAL);
		break;
	case HISP_CONFIG_ISP_LOWPOWER:
		cam_debug("%s HISP_CONFIG_ISP_LOWPOWER", __func__);
		rc = hisp250_set_clk_rate_self_adapt(ISP_CLK_LEVEL_LOWPOWER);
		break;
	case HISP_CONFIG_ISP_ULTRALOW:
		cam_debug("%s HISP_CONFIG_ISP_ULTRALOW", __func__);
		rc = hisp250_set_clk_rate_self_adapt(ISP_CLK_LEVEL_ULTRALOW);
		break;
	case HISP_CONFIG_R8_TURBO:
		cam_debug("%s HISP_CONFIG_R8_TURBO", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_TURBO_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_NORMAL:
		cam_debug("%s HISP_CONFIG_R8_NORMAL", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_NORMAL_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_LOWPOWER:
		cam_debug("%s HISP_CONFIG_R8_LOWPOWER", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_LOWPOWER_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_R8_ULTRALOW:
		cam_debug("%s HISP_CONFIG_R8_ULTRALOW", __func__);
		rc = hisp_set_clk_rate(ISPCPU_CLK, R8_ULTRALOW_ISPCPU_CLK_RATE);
		break;
	case HISP_CONFIG_PROC_TIMEOUT:
		cam_info("%s message_id.0x%x",__func__, pcfg->cfgdata[0]);
		hisp_dump_rpmsg_with_id(pcfg->cfgdata[0]);
		break;
	case HISP_CONFIG_GET_SEC_ISPFW_SIZE:
		//rc = hisp_secmem_size_get_from_type(&pcfg->buf_size, pcfg->secMemType);
		rc = hisp_secmem_size_get(&pcfg->buf_size);
		break;
	case HISP_CONFIG_SET_SEC_ISPFW_BUFFER:
		rc = hisp250_set_sec_fw_buffer(cfg);
		break;
	case HISP_CONFIG_RELEASE_SEC_ISPFW_BUFFER:
		rc = hisp250_release_sec_fw_buffer();
		break;
	case HISP_CONFIG_SET_MDC_BUFFER:
		rc = hisp_set_mdc_buffer(cfg);
		break;
	case HISP_CONFIG_RELEASE_MDC_BUFFER:
		rc = hisp_release_mdc_buffer();
		break;
	case HISP_CONFIG_PHY_CSI_CONNECT:
		rc = hisp250_phy_csi_connect((void*)(pcfg->cfgdata));
		break;
	case HISP_CONFIG_LOCK_VOLTAGE:
		rc = hisp250_lock_voltage(pcfg->cfgdata[0]);
		break;
	default:
		cam_err("%s: unsupported cmd:%#x", __func__, pcfg->cfgtype);
		break;
	}

	if (rc < 0) {
		cam_err("%s: cmd:%#x fail, rc:%u", __func__, pcfg->cfgtype, rc);
	}
	return rc;
}

static int hisp250_open(hisp_intf_t *i)
{
	cam_info("%s hisp250 device open!",__func__);

	mutex_lock(&hisp_power_lock_mutex);
	g_hisp_ref++;
	mutex_unlock(&hisp_power_lock_mutex);
	return 0;
}

static int hisp250_close(hisp_intf_t *i)
{
	int rc = 0;
	cam_info("%s hisp250 device close!",__func__);
	mutex_lock(&hisp_power_lock_mutex);

	if (g_hisp_ref) {
		g_hisp_ref--;
	}

	if ((g_hisp_ref == 0) && remote_processor_up) {
		cam_warn("%s hisp250 is still on power-on state, power off it.",
			__func__);

		rc = hisp250_power_off(i);
		if (0 != rc) {
			cam_err("failed to hisp250 power off!");
		}
		hisp250_deinit_r8isp_memory_pool_force();
	}
	if (g_hisp_ref == 0){
		hisp250_lock_voltage(0);
		hisp250_deinit_isp_mem();
	}
	mutex_unlock(&hisp_power_lock_mutex);
	return rc;
}

static int hisp250_power_on(hisp_intf_t *i)
{
	int rc = 0;
	bool rproc_enabled = false;
	bool hi_opened = false;
	hisp250_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	uint32_t timeout = hw_is_fpga_board() ? TIMEOUT_IS_FPGA_BOARD : TIMEOUT_IS_NOT_FPGA_BOARD;

	struct rpmsg_hisp250_service *hisi_serv = NULL;
	struct rpmsg_channel_info chinfo = {
		.src = RPMSG_ADDR_ANY,
	};
	if (NULL == i) {
		return -1;
	}
	hi = I2HI(i);

	cam_info("%s enter ....", __func__);

	mutex_lock(&hisp_wake_lock_mutex);
	if (!hisp_power_wakelock.active) {
		__pm_stay_awake(&hisp_power_wakelock);
		cam_info("%s hisp power on enter, wake lock", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */

	mutex_lock(&hisi_rpmsg_service_mutex);
	if (!atomic_read((&hi->opened))) {
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_default)) {
				rc = pinctrl_select_state(hi->dt.pinctrl, hi->dt.pinctrl_default);
				if (0 != rc) {
					goto FAILED_RET;
				}
			}
		}

		hisp_rpmsgrefs_reset();
		rc = hisi_isp_rproc_enable();
		if (rc != 0) {
			goto FAILED_RET;
		}
		rproc_enabled = true;

		rc  = wait_for_completion_timeout(&channel_sync, msecs_to_jiffies(timeout));
		if (0 == rc ) {
			rc = -ETIME;
			hisi_isp_boot_stat_dump();
			goto FAILED_RET;
		} else {
			cam_info("%s() %d after wait completion, rc = %d!", __func__, __LINE__, rc);
			rc = 0;
		}

		atomic_inc(&hi->opened);
		hi_opened = true;
	} else {
		cam_notice("%s isp has been opened.", __func__);
	}
	remote_processor_up = true;
	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		rc = -ENODEV;
		goto FAILED_RET;
	}

	/*assign a new, unique, local address and associate instance with it */
#pragma GCC visibility push(default)
	hisi_serv->ept =
		rpmsg_create_ept(hisi_serv->rpdev, hisp250_rpmsg_ept_cb, hisi_serv,
				 chinfo);
#pragma GCC visibility pop
	if (hisi_serv->ept == NULL) {
		hisi_serv->state = RPMSG_FAIL;
		rc = -ENOMEM;
		goto FAILED_RET;
	}
	cam_info("%s() %d hisi_serv->rpdev:src.%d, dst.%d",
			__func__, __LINE__,
			hisi_serv->rpdev->src, hisi_serv->rpdev->dst);
	hisi_serv->state = RPMSG_CONNECTED;

	/*set the instance recv_count */
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	hisp250_init_timestamp();

	if (hisp250_mem_pool_pre_init() ) {
		cam_err("failed to pre init mem pool ");
		rc = -ENOMEM;
		goto FAILED_RET;
	}

	mutex_unlock(&hisi_rpmsg_service_mutex);
	cam_info("%s exit ,power on time:%d....", __func__,
		jiffies_to_msecs(jiffies - current_jiffies) );
	return rc; /*lint !e454 */

FAILED_RET:
	if (hi_opened) {
		atomic_dec(&hi->opened);
	}

	if (rproc_enabled) {
		hisi_isp_rproc_disable();
               rproc_set_sync_flag(true);
	}

	hisp250_mem_pool_later_deinit();
	remote_processor_up = false;

	mutex_unlock(&hisi_rpmsg_service_mutex);

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_err("%s hisp power on failed, wake unlock", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */
	return rc; /*lint !e454 */
} /*lint !e454 */

static int hisp250_power_off(hisp_intf_t *i)
{
	int rc = 0;
	hisp250_t *hi = NULL;
	unsigned long current_jiffies = jiffies;
	struct rpmsg_hisp250_service *hisi_serv = NULL;
	if (NULL == i) {
		return -1;
	}
	hi = I2HI(i);

	cam_info("%s enter ....", __func__);

	/*check the remote processor boot flow */
	if (false == remote_processor_up) {
		rc = -EPERM;
		goto RET;
	}

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		rc = -ENODEV;
		goto RET;
	}

	if (RPMSG_FAIL == hisi_serv->state) {
		rc = -EFAULT;
		goto RET;
	}

	mutex_lock(&hisi_rpmsg_service_mutex);

	if (hisi_serv->ept == NULL) {
		rc = -ENODEV;
		goto UNLOCK_RET;
	}
	rpmsg_destroy_ept(hisi_serv->ept);
	hisi_serv->ept = NULL;

	hisi_serv->state = RPMSG_UNCONNECTED;
	hisi_serv->recv_count = HISP_SERV_FIRST_RECV;

	if (atomic_read((&hi->opened))) {
		hisp_phy_csi_disconnect();
		hisi_isp_rproc_disable();
		if (!hw_is_fpga_board()) {
			if (!IS_ERR(hi->dt.pinctrl_idle)) {
				rc = pinctrl_select_state(hi->dt.pinctrl, hi->dt.pinctrl_idle);
				if (0 != rc) {
					//Empty.
				}
			}
		}

		remote_processor_up = false;
		atomic_dec(&hi->opened);
	} else {
		cam_notice("%s isp hasn't been opened.", __func__);
	}

	hisp250_destroy_timestamp();
UNLOCK_RET:
	hisp250_mem_pool_later_deinit();

	mutex_unlock(&hisi_rpmsg_service_mutex);
RET:
	cam_info("%s exit ,power 0ff time:%d....", __func__,
		jiffies_to_msecs(jiffies - current_jiffies) );

	mutex_lock(&hisp_wake_lock_mutex);
	if (hisp_power_wakelock.active) {
		__pm_relax(&hisp_power_wakelock);
		cam_info("%s hisp power off exit, wake unlock", __func__);
	}
	mutex_unlock(&hisp_wake_lock_mutex); /*lint !e456 */
	return rc;
}

static void hisp250_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_hisp250_service *hisi_serv = dev_get_drvdata(&rpdev->dev);

	cam_info("%s enter ....", __func__);

	if (hisi_serv == NULL) {
		cam_err("%s: hisi_serv == NULL!", __func__);
		return;
	}

	mutex_destroy(&hisi_serv->send_lock);
	mutex_destroy(&hisi_serv->recv_lock);

	kfree(hisi_serv);
	rpmsg_local.hisi_isp_serv = NULL;
	cam_notice("rpmsg hisi driver is removed");
}

static int
hisp250_rpmsg_driver_cb(struct rpmsg_device *rpdev,
			void *data, int len, void *priv, u32 src)
{
	cam_info("%s enter ....", __func__);
	cam_warn("%s() %d uhm, unexpected message!", __func__,
		__LINE__);

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1, // 16 for lenth
			   data, len, true);
	return 0;
}

static int
hisp250_send_rpmsg(hisp_intf_t *i, hisp_msg_t *from_user, size_t len)
{
	int rc = 0;
	hisp250_t *hi = NULL;
	struct rpmsg_hisp250_service *hisi_serv = NULL;
	hisp_msg_t *msg = from_user;
	hisp_assert(NULL != i);
	hisp_assert(NULL != from_user);
	hi = I2HI(i);

	cam_debug("%s enter. api_name(0x%x)", __func__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0) {
		cam_info("%s: api_name:%#x, message_id:%#x", __FUNCTION__,
			msg->api_name, msg->message_id);
	}

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		cam_err("%s() %d failed: hisi_serv does not exist!", __func__,
				__LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (hisi_serv->ept == NULL) {
		cam_err("%s() %d failed:hisi_serv->ept does not exist!", __func__,
			__LINE__);
		rc = -ENODEV;
		goto RET;
	}

	mutex_lock(&hisi_serv->send_lock);
	/*if the msg is the first msg, let's treat it special */
	if (RPMSG_CONNECTED != hisi_serv->state) {
		if (hisi_serv->rpdev == NULL) {
			cam_err("%s() %d failed:hisi_serv->rpdev does not exist!",
				__func__, __LINE__);
			rc = -ENODEV;
			goto UNLOCK_RET;
		}
		hisp_sendin(msg);
		rc = rpmsg_send_offchannel(hisi_serv->ept,
					   hisi_serv->ept->addr,
					   hisi_serv->rpdev->dst, (void *)msg,
					   len);
		if (rc) {
			cam_err("%s() %d failed: first rpmsg_send_offchannel ret is %d!",
				__func__, __LINE__, rc);
		}
		goto UNLOCK_RET;
	}
	hisp_sendin(msg);
	rc = rpmsg_send_offchannel(hisi_serv->ept, hisi_serv->ept->addr,
				   hisi_serv->dst, (void *)msg, len);

	if (rc) {
		cam_err("%s() %d failed: rpmsg_send_offchannel ret is %d!", __func__,
			__LINE__, rc);
		goto UNLOCK_RET;
	}
UNLOCK_RET:
	mutex_unlock(&hisi_serv->send_lock);
RET:
	return rc;
}

static int
hisp250_recv_rpmsg(hisp_intf_t *i, hisp_msg_t *user_addr, size_t len)
{
	int rc = len;
	hisp250_t *hi = NULL;
	struct rpmsg_hisp250_service *hisi_serv = NULL;
	struct sk_buff *skb = NULL;
	hisp_msg_t *msg = NULL;
	hisp_assert(NULL != i);
	if (NULL == user_addr) {
		cam_err("func %s: user_addr is NULL",__func__);
		return -1;
	}
	hi = I2HI(i);

	cam_debug("%s enter. ", __func__);

	hisi_serv = rpmsg_local.hisi_isp_serv;
	if (hisi_serv == NULL) {
		cam_err("%s() %d failed: hisi_serv does not exist!", __func__,
			__LINE__);
		rc = -ENODEV;
		goto RET;
	}

	if (HISP_SERV_FIRST_RECV == hisi_serv->recv_count) {
		hisi_serv->recv_count = HISP_SERV_NOT_FIRST_RECV;
	}

	if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
		cam_err("%s() %d failed: mutex_lock_interruptible!", __func__,
			__LINE__);
		rc = -ERESTARTSYS;
		goto RET;
	}

	if (RPMSG_CONNECTED != hisi_serv->state) {
		cam_err("%s() %d hisi_serv->state != RPMSG_CONNECTED!",
			__func__, __LINE__);
		rc = -ENOTCONN;
		goto UNLOCK_RET;
	}

	/*nothing to read ? */
	/*check if skb_queue is NULL ? */
	if (skb_queue_empty(&hisi_serv->queue)) {
		mutex_unlock(&hisi_serv->recv_lock); /*lint !e455 */
		cam_err("%s() %d skb_queue is empty!", __func__, __LINE__);

		/*otherwise block, and wait for data */
		if (wait_event_interruptible_timeout(hisi_serv->readq,
						 (!skb_queue_empty(&hisi_serv->queue)
						  || hisi_serv->state == RPMSG_FAIL),
						  msecs_to_jiffies(HISP_WAIT_TIMEOUT))) { //lint !e666
			cam_err("%s() %d hisi_serv->state = %d!", __func__,
				__LINE__, hisi_serv->state);
			rc = -ERESTARTSYS;
			goto RET;
		}

		if (mutex_lock_interruptible(&hisi_serv->recv_lock)) {
			cam_err("%s() %d failed: mutex_lock_interruptible!",
				__func__, __LINE__);
			rc = -ERESTARTSYS;
			goto RET;
		}
	}

	if (RPMSG_FAIL == hisi_serv->state) {
		cam_err("%s() %d state = RPMSG_FAIL!", __func__, __LINE__);
		rc = -ENXIO;
		goto UNLOCK_RET;
	}

	skb = skb_dequeue(&hisi_serv->queue);
	if (skb == NULL) {
		cam_err("%s() %d skb is NULL!", __func__, __LINE__);
		rc = -EIO;
		goto UNLOCK_RET;
	}

	rc = min((unsigned int)len, skb->len);
	msg = (hisp_msg_t *) (skb->data);
	hisp_recvdone((void *)msg);
	if (msg->api_name == ISP_CPU_POWER_OFF_RESPONSE) {
		hisp_rpmsgrefs_dump();
	}
	cam_debug("%s: api_name(0x%x)", __func__, msg->api_name);

	if (msg->message_id % HISP_MSG_LOG_MOD == 0) {
		cam_info("%s: api_name:%#x, message_id:%#x", __FUNCTION__,
			msg->api_name, msg->message_id);
	}

	hisp250_handle_msg(msg);
	if (!memcpy(user_addr, msg, rc)) {
		rc = -EFAULT;
		cam_err("Fail: %s()%d ret = %d", __func__, __LINE__, rc);
	}
	kfree_skb(skb);

UNLOCK_RET:
	mutex_unlock(&hisi_serv->recv_lock); /*lint !e455 */
RET:
	return rc;
}
int hisp250_set_sec_fw_buffer(struct hisp_cfg_data *cfg)
{
	int rc;

	mutex_lock(&hisp_mem_lock_mutex);
	rc = hisp_set_sec_fw_buffer(cfg);
	if(rc < 0)
		cam_err("%s: fail, rc:%d", __func__, rc);


	if (s_hisp250.mem.active) {
		s_hisp250.mem.active = 0;
		dma_buf_put(s_hisp250.mem.dmabuf);
	}

	s_hisp250.mem.dmabuf = dma_buf_get(cfg->share_fd);
	s_hisp250.mem.active = 1;
	mutex_unlock(&hisp_mem_lock_mutex);
	return rc;
}
int hisp250_release_sec_fw_buffer(void)
{
	mutex_lock(&hisp_mem_lock_mutex);
	int rc = hisp_release_sec_fw_buffer();

	if (rc < 0)
		cam_err("%s: fail, rc:%d", __func__, rc);

	if (s_hisp250.mem.active) {
		s_hisp250.mem.active = 0;
		dma_buf_put(s_hisp250.mem.dmabuf);
	}
	memset(&(s_hisp250.mem), 0, sizeof(struct isp_mem));
	mutex_unlock(&hisp_mem_lock_mutex);
	return rc;
}
static void hisp250_deinit_isp_mem(void)
{
	cam_info("func %s", __func__);
	mutex_lock(&hisp_mem_lock_mutex);
	if (s_hisp250.mem.active) {
		cam_err("sec isp ex,put dmabuf");
		s_hisp250.mem.active = 0;
		dma_buf_put(s_hisp250.mem.dmabuf);
	}

	memset(&(s_hisp250.mem), 0, sizeof(struct isp_mem));
	mutex_unlock(&hisp_mem_lock_mutex);

	return ;
}

#ifdef CONFIG_HISI_DEBUG_FS
static void hisp250_set_ddrfreq(int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%d",__func__,ddr_bandwidth);
}

static void hisp250_release_ddrfreq(void)
{
	cam_info("%s enter",__func__);
	if (current_ddr_bandwidth == 0)
		return;
	pm_qos_remove_request(&qos_request_ddr_down_record);
	current_ddr_bandwidth = 0;
}

static void hisp250_update_ddrfreq(unsigned int ddr_bandwidth)
{
	cam_info("%s enter,ddr_bandwidth:%u",__func__,ddr_bandwidth);
	if (!atomic_read(&s_hisp250.opened)) {
		cam_info("%s ,cam is not opened,so u can not set ddr bandwidth",__func__);
		return;
	}

	if (current_ddr_bandwidth == 0) {
		hisp250_set_ddrfreq(ddr_bandwidth);
	} else if (current_ddr_bandwidth > 0) {
		pm_qos_update_request(&qos_request_ddr_down_record, ddr_bandwidth);
		current_ddr_bandwidth = ddr_bandwidth;
	} else {
		cam_err("%s,current_ddr_bandwidth is invalid",__func__);
	}
}

static ssize_t hisp_ddr_freq_ctrl_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
	cam_info("enter %s,current_ddr_bandwidth:%d", __func__, current_ddr_bandwidth);

	return scnprintf(buf, PAGE_SIZE, "%d", current_ddr_bandwidth);
}

static ssize_t hisp_ddr_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ddr_bandwidth = 0;
	if (buf == NULL) {
		cam_err("%s,input buffer is invalid",__func__);
		return -EINVAL;
	}

	ddr_bandwidth = simple_strtol(buf, NULL, 10); // 10 for simple_strtol base
	cam_info("%s enter,ddr_bandwidth:%d", __func__, ddr_bandwidth);

	if (ddr_bandwidth < 0) {
		cam_err("%s,ddr_bandwidth is invalid",__func__);
		return -EINVAL;
	} else if (ddr_bandwidth == 0) {
		hisp250_release_ddrfreq();
	} else if (ddr_bandwidth > 0) {
		hisp250_update_ddrfreq(ddr_bandwidth);
	}

	return count;
}
#endif


static int32_t hisp250_rpmsg_probe(struct rpmsg_device *rpdev)
{
	int32_t ret = 0;
	struct rpmsg_hisp250_service *hisi_serv = NULL;
	cam_info("%s enter", __func__);

	if (NULL != rpmsg_local.hisi_isp_serv) {
		cam_notice("%s hisi_serv is already up!", __func__);
		goto SERVER_UP;
	}

	hisi_serv = kzalloc(sizeof(*hisi_serv), GFP_KERNEL);
	if (hisi_serv == NULL) {
		cam_err("%s() %d kzalloc failed!", __func__, __LINE__);
		ret = -ENOMEM;
		goto ERROR_RET;
	}
	mutex_init(&hisi_serv->send_lock);
	mutex_init(&hisi_serv->recv_lock);
	skb_queue_head_init(&hisi_serv->queue);
	init_waitqueue_head(&hisi_serv->readq);
	hisi_serv->ept = NULL;
	hisi_serv->comp = &rpmsg_local.isp_comp;

	rpmsg_local.hisi_isp_serv = hisi_serv;
SERVER_UP:
	if (NULL == hisi_serv) {
		cam_err("func %s: hisi_serv is NULL",__func__);
		return -1;
	}
	hisi_serv->rpdev = rpdev;
	hisi_serv->state = RPMSG_UNCONNECTED;
	dev_set_drvdata(&rpdev->dev, hisi_serv);

	complete(hisi_serv->comp);

	cam_info("new HISI connection srv channel: %u -> %u!",
		rpdev->src, rpdev->dst);
ERROR_RET:
	return ret;
}

static struct rpmsg_device_id rpmsg_hisp250_id_table[] = {
	{.name = "rpmsg-hisi"},
	{},
};

MODULE_DEVICE_TABLE(platform, rpmsg_hisp250_id_table);

static const struct of_device_id s_hisp250_dt_match[] = {
	{
	 .compatible = "huawei,hisi_isp250",
	 .data = &s_hisp250.intf,
	 },
	{
	 },
};

MODULE_DEVICE_TABLE(of, s_hisp250_dt_match);
#pragma GCC visibility push(default)
static struct rpmsg_driver rpmsg_hisp250_driver = {
	.drv.name   = KBUILD_MODNAME, //lint !e485
	.drv.owner  = THIS_MODULE, //lint !e485
	.id_table = rpmsg_hisp250_id_table,
	.probe = hisp250_rpmsg_probe,
	.callback = hisp250_rpmsg_driver_cb,
	.remove = hisp250_rpmsg_remove,
};
#pragma GCC visibility pop

#ifdef CONFIG_HISI_DEBUG_FS
static struct device_attribute hisp_ddr_freq_ctrl_attr =
    __ATTR(ddr_freq_ctrl, 0660, hisp_ddr_freq_ctrl_show, hisp_ddr_freq_store); // 0660 for ATTR mode
#endif /* CONFIG_HISI_DEBUG_FS */

static int32_t
hisp250_platform_probe(
	struct platform_device* pdev)
{
	int32_t ret = 0;

	cam_info("%s: enter", __func__);
	wakeup_source_init(&hisp_power_wakelock, "hisp_power_wakelock");
	mutex_init(&hisp_wake_lock_mutex);
	mutex_init(&hisp_power_lock_mutex);
	mutex_init(&hisp_mem_lock_mutex);
	ret = hisp_get_dt_data(pdev, &s_hisp250.dt);
	if (ret < 0) {
		cam_err("%s: get dt failed.", __func__);
		goto error;
	}

	init_completion(&rpmsg_local.isp_comp);
	ret = hisp_register(pdev, &s_hisp250.intf, &s_hisp250.notify);
	if (0 == ret) {
		atomic_set(&s_hisp250.opened, 0);
	} else {
		cam_err("%s() %d hisp_register failed with ret %d!",
			__func__, __LINE__, ret);
		goto error;
	}

	rpmsg_local.hisi_isp_serv = NULL;

	ret = register_rpmsg_driver(&rpmsg_hisp250_driver);
	if (0 != ret) {
		cam_err("%s() %d register_rpmsg_driver failed with ret %d!",
			__func__, __LINE__, ret);
		goto error;
	}

	s_hisp250.pdev = pdev;

	memset(&(s_hisp250.mem), 0, sizeof(struct isp_mem));
#ifdef CONFIG_HISI_DEBUG_FS
	ret = device_create_file(&pdev->dev, &hisp_ddr_freq_ctrl_attr);
	if (ret < 0) {
		cam_err("%s failed to creat hisp ddr freq ctrl attribute.", __func__);
		unregister_rpmsg_driver(&rpmsg_hisp250_driver);
		hisp_unregister(s_hisp250.pdev);
		goto error;
	}
#endif
	return 0;

error:
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
	mutex_destroy(&hisp_power_lock_mutex);
	mutex_destroy(&hisp_mem_lock_mutex);
	cam_notice("%s exit with ret = %d", __func__, ret);
	return ret;
}

static struct platform_driver
s_hisp250_driver =
{
	.probe = hisp250_platform_probe,
	.driver =
	{
		.name = "huawei,hisi_isp250",
		.owner = THIS_MODULE,
		.of_match_table = s_hisp250_dt_match,
	},
};

static int __init
hisp250_init_module(void)
{
	cam_notice("%s enter", __func__);
	return platform_driver_register(&s_hisp250_driver);
}

static void __exit
hisp250_exit_module(void)
{
	cam_notice("%s enter", __func__);
	hisp_unregister(s_hisp250.pdev);
	platform_driver_unregister(&s_hisp250_driver);
	wakeup_source_trash(&hisp_power_wakelock);
	mutex_destroy(&hisp_wake_lock_mutex);
}

module_init(hisp250_init_module);
module_exit(hisp250_exit_module);
MODULE_DESCRIPTION("hisp250 driver");
MODULE_LICENSE("GPL v2");
