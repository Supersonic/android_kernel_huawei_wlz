

#define HISI_LOG_TAG "[HCC_TEST]"
#include "oal_hcc_host_if.h"
#include "oal_ext_if.h"
#include "plat_pm_wlan.h"
#include "plat_pm.h"
#include "securec.h"
#include "hisi_ini.h"
#include "plat_cali.h"
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "oal_pcie_host.h"
#include "oal_pcie_linux.h"
#endif
#if defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
#include "board.h"
#endif
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/host.h>

typedef struct _hcc_test_data_ {
    oal_int32 mode_idx;
    oal_int32 pkt_rcvd;      /* packet received */
    oal_int32 pkt_sent;      /* packet sent */
    oal_int32 pkt_gen;       /* packet generate */
    oal_int32 pkt_len;       /* packet  length */
    oal_ulong trans_timeout; /* msec */
    oal_uint64 total_rcvd_bytes;
    oal_uint64 total_sent_bytes;
    oal_uint64 trans_time;
    oal_uint64 trans_time_us;
    oal_uint64 throughput;
    hsdio_trans_test_info trans_info;
} hcc_test_data;

struct hcc_test_event {
    oal_mutex_stru mutex_lock; /* sdio test task lock */
    oal_int32 errorno;

    struct workqueue_struct *test_workqueue;
    struct work_struct test_work;
    hcc_test_data test_data;

    /* hcc perf started,for data transfer */
    oal_int32 started;

    oal_int32 rx_stop;

    /* hcc perf statistic */
    ktime_t start_time;
    /* The last update time */
    ktime_t last_time;

    /* To hcc test sync */
    oal_completion test_done;
    oal_completion test_trans_done;

    hcc_queue_type hcc_test_queue;

    oal_uint8 pad_payload;

    oal_uint8 test_value;
    oal_uint8 verified;

    oal_ulong start_tick;
    oal_ulong tick_timeout;

    /* sdio test thread and seam */
};

struct hcc_test_stru {
    const char *mode;
    oal_uint16 start_cmd;
    const char *mode_desc;
};

/* 全局变量定义 */
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
oal_completion pcie_test_trans_done;
hcc_pcie_test_request_stru pcie_test_request_ack;
#endif
OAL_STATIC oal_int32 test_force_stop = 0;
struct hcc_test_event *hcc_test_event_etc = NULL;

struct hcc_test_stru hcc_test_stru_etc[HCC_TEST_CASE_COUNT] = {
    [HCC_TEST_CASE_TX] = { "tx",   HCC_TEST_CMD_START_TX,   "HCC_TX_MODE" },
    [HCC_TEST_CASE_RX] = { "rx",   HCC_TEST_CMD_START_RX,   "HCC_RX_MODE" },
    [HCC_TEST_CASE_LOOP] = { "loop", HCC_TEST_CMD_START_LOOP, "HCC_LOOP_MODE" },
};

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
oal_uint32 wifi_panic_debug_etc = 0;
oal_debug_module_param(wifi_panic_debug_etc, int, S_IRUGO | S_IWUSR);
OAL_STATIC LIST_HEAD(wifi_panic_log_head);
#endif

oal_int32 ft_pcie_test_wifi_runtime = 5000; /* 5 seconds */
oal_int32 ft_sdio_test_wifi_runtime = 5000; /* 5 seconds */
oal_int32 ft_pcie_test_min_throught = 1613; /* 1600Mbps */
oal_int32 ft_pcie_test_retry_cnt = 3;       /* retry 3 times */
oal_int32 ft_ip_test_cpu_max_freq = 1;      /* retry 3 times */
oal_int32 hcc_test_performance_mode = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(ft_pcie_test_wifi_runtime, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(ft_sdio_test_wifi_runtime, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(ft_pcie_test_min_throught, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(ft_pcie_test_retry_cnt, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(ft_ip_test_cpu_max_freq, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(hcc_test_performance_mode, int, S_IRUGO | S_IWUSR);
#endif
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
oal_int32 ft_pcie_loopback_rw_test_count = 500;
oal_int32 ft_pcie_loopback_linkup_timeout = 600000; /* us */
oal_int32 ft_pcie_loopback_bias_cldo3 = 900;        /* 0.9v */
oal_int32 ft_pcie_loopback_bias_vptx = 900;         /* 0.9v */
oal_int32 ft_pcie_loopback_bias_vph = 1800;         /* 1.8v */
module_param(ft_pcie_loopback_rw_test_count, int, S_IRUGO | S_IWUSR);
module_param(ft_pcie_loopback_linkup_timeout, int, S_IRUGO | S_IWUSR);
module_param(ft_pcie_loopback_bias_cldo3, int, S_IRUGO | S_IWUSR);
module_param(ft_pcie_loopback_bias_vptx, int, S_IRUGO | S_IWUSR);
module_param(ft_pcie_loopback_bias_vph, int, S_IRUGO | S_IWUSR);
#endif

oal_int32 conn_test_hcc_chann_switch(const char *new_dev);
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
oal_int32 hcc_assem_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len);
oal_int32 hcc_flowctrl_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len);
oal_int32 hcc_queues_len_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len);
oal_int32 hcc_queues_pkts_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len);
oal_int32 hsdio_sysfs_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len);
OAL_STATIC DECLARE_WIFI_PANIC_STRU(hcc_panic_assem_info, hcc_assem_info_print_etc);
OAL_STATIC DECLARE_WIFI_PANIC_STRU(hcc_panic_flowctrl, hcc_flowctrl_info_print_etc);
OAL_STATIC DECLARE_WIFI_PANIC_STRU(hcc_panic_queues_len, hcc_queues_len_info_print_etc);
OAL_STATIC DECLARE_WIFI_PANIC_STRU(hcc_panic_queues_pkts, hcc_queues_pkts_info_print_etc);
OAL_STATIC DECLARE_WIFI_PANIC_STRU(sdio_panic, hsdio_sysfs_info_print_etc);
#endif

OAL_STATIC oal_kobject *conn_syfs_hcc_object = NULL;
OAL_STATIC oal_int32 hcc_test_normal_start(oal_uint16 test_type);
OAL_STATIC oal_int32 hcc_send_test_cmd(oal_uint8 *cmd, oal_int32 hdr_len, oal_int32 cmd_len);
OAL_STATIC oal_int32 hcc_test_start(oal_uint16 start_cmd);
int hcc_test_set_case_etc(hcc_test_data *data);

/* 函数声明 */
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
oal_void hwifi_panic_log_register_etc(hwifi_panic_log *pst_log, oal_void *data)
{
    if (OAL_UNLIKELY(!pst_log)) {
        OAL_IO_PRINT("%s error: pst_log is null", __FUNCTION__);
        return;
    };

    pst_log->data = data;
    list_add_tail(&pst_log->list, &wifi_panic_log_head);
    OAL_IO_PRINT("Log module %s added![%pF]\n", pst_log->name, (void *)_RET_IP_);
}
oal_module_symbol(hwifi_panic_log_register_etc);

/* should't use dynamic mem when panic */
OAL_STATIC char panic_mem[PAGE_SIZE];
void hwifi_panic_log_dump_etc(char *print_level)
{
    oal_uint32 buf_size = PAGE_SIZE;
    char *pst_buf = NULL;
    hwifi_panic_log *pst_log = NULL;
    struct list_head *head = &wifi_panic_log_head;

    if (print_level == NULL) {
        print_level = "";
    }

    pst_buf = panic_mem;

    printk("%sdump wifi info when panic\n", print_level);
    list_for_each_entry(pst_log, head, list)
    {
        if (pst_log == NULL) {
            printk(KERN_ERR "hwifi_panic_log_dump_etc:pst_log is null\n");
            return;
        }
        pst_buf[0] = '\0';
        printk("%s[%s]:\n", print_level, pst_log->name);
        pst_log->cb(pst_log->data, pst_buf, buf_size);
        printk("%s%s\n", print_level, pst_buf);
        printk("%s\n", print_level);
    }
    printk("%s\n", print_level);
}
oal_module_symbol(hwifi_panic_log_dump_etc);
#endif

oal_int32 hcc_flowctrl_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 count = 0;
    struct oal_sdio *hi_sdio = NULL;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }

#ifdef CONFIG_MMC
    hi_sdio = oal_get_sdio_default_handler();
    if ((hi_sdio != NULL) && (hi_sdio->func->card->host->claimer != NULL)) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "claim host name:%s\n",
                         hi_sdio->func->card->host->claimer->comm);
    }
    if (ret < 0) {
        return count;
    }
    count += ret;
#else
    OAL_REFERENCE(hi_sdio);
#endif
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc state:%s[%d]\n",
                     (oal_atomic_read(&hcc->state) == HCC_OFF) ? "off" : "on",
                     oal_atomic_read(&hcc->state));
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl flag:%s\n",
                     (D2H_MSG_FLOWCTRL_OFF == hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_flag) ?
                     "off" : "on");
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl on:%d\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_on_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl off:%d\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_off_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flowctrl reset count:%d\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_reset_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "flowctrl hi update count:%d,hi credit value:%u\n",
                     hcc->hcc_transer_info.tx_flow_ctrl.flowctrl_hipri_update_count,
                     hcc->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}

oal_int32 hcc_assem_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 i;
    oal_int32 total = 0;
    oal_int32 count = 0;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }
    for (i = 0; i < HCC_TX_ASSEM_INFO_MAX_NUM; i++) {
        if (hcc->hcc_transer_info.tx_assem_info.info[i]) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "[tx][%2d]:%-20u pkts\n",
                             i, hcc->hcc_transer_info.tx_assem_info.info[i]);
            if (ret < 0) {
                return count;
            }
            count += ret;
            total += (oal_int32)hcc->hcc_transer_info.tx_assem_info.info[i] * (i == 0 ? 1 : i);
        }
    }
    if (total) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc tx total:%d!\n", total);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }

    total = 0;

    for (i = 0; i < HCC_RX_ASSEM_INFO_MAX_NUM; i++) {
        if (hcc->hcc_transer_info.rx_assem_info.info[i]) {
            total += (oal_int32)hcc->hcc_transer_info.rx_assem_info.info[i] * (i == 0 ? 1 : i);
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "[rx][%2d]:%-20u pkts\n",
                             i, hcc->hcc_transer_info.rx_assem_info.info[i]);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }

    if (total) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc rx total:%d!\n", total);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
    return count;
}

oal_int32 hcc_queues_pkts_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len)
{
    oal_int32 ret;
    oal_int32 count = 0;
    int i, j;
    oal_uint64 total;
    hcc_trans_queue *pst_hcc_queue = NULL;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "queues_pkts_info_show [tx_seq:%d]\n",
                     atomic_read(&hcc->tx_seq));
    if (ret < 0) {
        return count;
    }
    count += ret;

    for (i = 0; i < HCC_DIR_COUNT; i++) {
        total = 0;
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                         "transfer dir:%s\n", HCC_GET_CHAN_STRING(i));
        if (ret < 0) {
            return count;
        }
        count += ret;

        for (j = 0; j < HCC_QUEUE_COUNT; j++) {
            pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[i].queues[j];
            if (pst_hcc_queue->total_pkts || pst_hcc_queue->loss_pkts) {
                ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                                 "queue:%4d,pkts num:%10u,loss num:%10u\n", j,
                                 pst_hcc_queue->total_pkts,
                                 pst_hcc_queue->loss_pkts);
                if (ret < 0) {
                    return count;
                }
                count += ret;
            }
            total += pst_hcc_queue->total_pkts;
        }
        if (total) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "total:%llu\n", total);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "flow ctrl info show\n");
    if (ret < 0) {
        return count;
    }
    count += ret;

    for (j = 0; j < HCC_QUEUE_COUNT; j++) {
        pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_TX].queues[j];
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "tx queue:%4d,%s,low wl:%u, high wl:%u\n",
                         j,
                         (pst_hcc_queue->flow_ctrl.is_stopped == OAL_FALSE) ? "run " : "stop",
                         pst_hcc_queue->flow_ctrl.low_waterline,
                         pst_hcc_queue->flow_ctrl.high_waterline);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }

    pst_hcc_queue = &hcc->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE];
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "rx queue:%4d,low wl:%u,high wl:%u, enabled:%d\n",
                     DATA_LO_QUEUE,
                     pst_hcc_queue->flow_ctrl.low_waterline,
                     pst_hcc_queue->flow_ctrl.high_waterline, pst_hcc_queue->flow_ctrl.enable);
    if (ret < 0) {
        return count;
    }
    count += ret;

#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hcc thread stat info:\n");
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "condition true:%llu,false:%llu\n",
                     hcc->hcc_transer_info.thread_stat.wait_event_run_count,
                     hcc->hcc_transer_info.thread_stat.wait_event_block_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "thread loop do:%llu,empty:%llu\n",
                     hcc->hcc_transer_info.thread_stat.loop_have_data_count,
                     hcc->hcc_transer_info.thread_stat.loop_no_data_count);
    if (ret < 0) {
        return count;
    }
    count += ret;
#endif
    return count;
}

oal_int32 hcc_queues_len_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 count = 0;
    oal_int32 i, j;
    struct hcc_handler *hcc = (struct hcc_handler *)data;
    if (hcc == NULL) {
        return 0;
    }
    for (i = 0; i < HCC_DIR_COUNT; i++) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "dir:%s\n", HCC_GET_CHAN_STRING(i));
        if (ret < 0) {
            return count;
        }
        count += ret;

        for (j = 0; j < HCC_QUEUE_COUNT; j++) {
            if (oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[i].queues[j].data_queue)) {
                ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "queue:%d, len:%d\n", j,
                                 oal_netbuf_list_len(&hcc->hcc_transer_info.hcc_queues[i].queues[j].data_queue));
                if (ret < 0) {
                    return count;
                }
                count += ret;
            }
        }
    }
    return count;
}

oal_int32 hcc_print_current_trans_info(oal_uint32 print_device_info)
{
    int ret = 0;
    char *buf = NULL;
    oal_uint32 buf_size = PAGE_SIZE;
    hcc_bus *pst_bus = NULL;
    struct hcc_handler *hcc;
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return -OAL_EFAIL;
    }

    pst_bus = hcc_get_current_110x_bus();

    buf = oal_memalloc(buf_size);
    if (buf != NULL) {
        ret = 0;
        memset_s(buf, buf_size, 0, buf_size);

        if ((pst_bus != NULL) && (pst_bus->bus_type == HCC_BUS_SDIO)) {
            ret += hcc_flowctrl_info_print_etc(hcc, buf, buf_size - ret);
        }

        ret += hcc_queues_len_info_print_etc(hcc, buf, buf_size - ret);

        ret += hcc_queues_pkts_info_print_etc(hcc, buf + ret, buf_size - ret);

        OAL_IO_PRINT("%s\n", buf);

        oal_free(buf);
    } else {
        OAL_IO_PRINT("alloc buf size %u failed\n", buf_size);
    }

    if (pst_bus != NULL) {
        /* 打印device信息要保证打印过程中 不会进入深睡 */
        hcc_bus_print_trans_info(pst_bus, print_device_info ?
                                 (HCC_PRINT_TRANS_FLAG_DEVICE_STAT | HCC_PRINT_TRANS_FLAG_DEVICE_REGS) : 0x0);
    }

    return OAL_SUCC;
}

OAL_STATIC ssize_t hcc_get_assem_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_assem_info_print_etc(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_set_assem_info(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return count;
    }
    hcc_transfer_lock(hcc);
    hcc_tx_assem_info_reset_etc(hcc);
    oal_sdio_rx_assem_info_reset(hcc);
    hcc_transfer_unlock(hcc);

    OAL_IO_PRINT("reset done!\n");

    return count;
}

OAL_STATIC ssize_t hcc_get_queues_pkts_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_queues_pkts_info_print_etc(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_get_queues_len_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_queues_len_info_print_etc(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_get_flowctrl_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_flowctrl_info_print_etc(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

oal_int32 hcc_wakelock_info_print_etc(struct hcc_handler *hcc, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 count = 0;

    if (hcc == NULL) {
        return 0;
    }
#ifdef CONFIG_PRINTK
    if (hcc->tx_wake_lock.locked_addr) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakelocked by:%pf\n",
                         (oal_void *)hcc->tx_wake_lock.locked_addr);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
#endif

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hold %lu locks\n",
                     hcc->tx_wake_lock.lock_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

OAL_STATIC ssize_t hcc_get_wakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += hcc_wakelock_info_print_etc(hcc, buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC ssize_t hcc_get_allwakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return ret;
    }

    ret += oal_print_all_wakelock_buff(buf, PAGE_SIZE);

    return ret;
}

OAL_STATIC ssize_t hcc_set_allwakelock_info(struct kobject *dev, struct kobj_attribute *attr,
                                            const char *buf, size_t count)
{
    oal_uint32 level;
    char lockname[200]; /* 因编译原因，暂不支持用const变量定义数组大小 */

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= OAL_SIZEOF(lockname)) || (buf[count] != '\0')) {
        OAL_IO_PRINT("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    memset_s(lockname, OAL_SIZEOF(lockname), 0, OAL_SIZEOF(lockname));

    if ((sscanf(buf, "%s %u", lockname, &level) < 2)) {
        OAL_IO_PRINT("error input,must input more than 2 arguments!\n");
        return count;
    }

    oal_set_wakelock_debuglevel(lockname, !!level);

    return count;
}

OAL_STATIC struct kobj_attribute dev_attr_flowctrl =
    __ATTR(flowctrl, S_IRUGO, hcc_get_flowctrl_info, NULL);
OAL_STATIC struct kobj_attribute dev_attr_assem_info =
    __ATTR(assem_info, S_IRUGO | S_IWUSR, hcc_get_assem_info, hcc_set_assem_info);
OAL_STATIC struct kobj_attribute dev_attr_queues_pkts =
    __ATTR(queues_pkts, S_IRUGO, hcc_get_queues_pkts_info, NULL);
OAL_STATIC struct kobj_attribute dev_attr_queues_len =
    __ATTR(queues_len, S_IRUGO, hcc_get_queues_len_info, NULL);
OAL_STATIC struct kobj_attribute dev_attr_wakelock =
    __ATTR(wakelock, S_IRUGO, hcc_get_wakelock_info, NULL);
OAL_STATIC struct kobj_attribute dev_attr_wakelockall =
    __ATTR(wakelockall, S_IRUGO | S_IWUSR, hcc_get_allwakelock_info, hcc_set_allwakelock_info);

/*
 * Prototype    : hcc_test_get_trans_time_etc
 * Description  : get hcc test trans time
 * Return Value : succ or fail
 */
oal_uint64 hcc_test_get_trans_time_etc(ktime_t start_time, ktime_t stop_time)
{
    ktime_t trans_time;
    oal_uint64 trans_us;

    trans_time = ktime_sub(stop_time, start_time);
#ifdef _PRE_COMMENT_CODE_
    OAL_IO_PRINT("start time:%llu, stop time:%llu, trans_time:%llu\n",
                 ktime_to_us(start_time), ktime_to_us(stop_time), ktime_to_us(trans_time));
#endif
    trans_us = (oal_uint64)ktime_to_us(trans_time);

    if (trans_us == 0) {
        trans_us = 1;
    }

    return trans_us;
}

oal_void hcc_test_throughput_cac_etc(oal_uint64 trans_bytes, ktime_t start_time, ktime_t stop_time)
{
    oal_uint64 trans_us;
    oal_uint64 temp;
    oal_uint64 us_to_s; /* converted  usecond to second */

    trans_us = hcc_test_get_trans_time_etc(start_time, stop_time);

    temp = (trans_bytes);

    temp = temp * 1000u;

    temp = temp * 1000u;

    /* 17是推算出的，无实际意义 */
    temp = (temp >> 17);

    temp = div_u64(temp, trans_us);

    us_to_s = trans_us;
    hcc_test_event_etc->test_data.trans_time_us = trans_us;
    do_div(us_to_s, 1000000u);
    hcc_test_event_etc->test_data.throughput = temp;
    hcc_test_event_etc->test_data.trans_time = us_to_s;
}

OAL_STATIC oal_void hcc_test_throughput_gen(oal_void)
{
    if (hcc_test_event_etc->test_data.mode_idx == HCC_TEST_CASE_TX) {
        hcc_test_throughput_cac_etc(hcc_test_event_etc->test_data.total_sent_bytes,
                                    hcc_test_event_etc->start_time,
                                    hcc_test_event_etc->last_time);
    } else if (hcc_test_event_etc->test_data.mode_idx == HCC_TEST_CASE_RX) {
        hcc_test_throughput_cac_etc(hcc_test_event_etc->test_data.total_rcvd_bytes,
                                    hcc_test_event_etc->start_time,
                                    hcc_test_event_etc->last_time);
    } else if (hcc_test_event_etc->test_data.mode_idx == HCC_TEST_CASE_LOOP) {
        hcc_test_throughput_cac_etc(hcc_test_event_etc->test_data.total_rcvd_bytes +
                                    hcc_test_event_etc->test_data.total_sent_bytes,
                                    hcc_test_event_etc->start_time,
                                    hcc_test_event_etc->last_time);
    }
}

OAL_STATIC oal_uint64 hcc_test_utilization_ratio_gen(oal_uint64 payload_size, oal_uint64 transfer_size)
{
    oal_uint64 ret;
    payload_size = payload_size * 1000; /* 计算占有率 */
    if (transfer_size) {
        ret = div_u64(payload_size, transfer_size);
    } else {
        ret = 0;
    }
    return ret;
}

/* 统计发送方向的丢包率，接收方向默认不丢包 */
OAL_STATIC oal_uint32 hcc_test_tx_pkt_loss_gen(oal_uint32 tx_pkts, oal_uint32 actual_tx_pkts)
{
    oal_uint32 ul_loss;

    if (tx_pkts == actual_tx_pkts || !tx_pkts) {
        return 0;
    }
    if (tx_pkts < actual_tx_pkts) {
        return 0;
    }

    ul_loss = tx_pkts - actual_tx_pkts;
    return ul_loss * 1000 / tx_pkts; /* 计算占有率 */
}

OAL_STATIC void hcc_test_rcvd_data(oal_netbuf_stru *pst_netbuf)
{
    oal_int32 ret;
    oal_int32 filter_flag = 0;

    /* 计算总共数据包长度 */
    if (OAL_UNLIKELY(hcc_test_event_etc->test_data.pkt_len != OAL_NETBUF_LEN(pst_netbuf))) {
        if (printk_ratelimit()) {
            OAL_IO_PRINT("[E]recvd netbuf pkt len:%d,but request len:%d\n",
                         OAL_NETBUF_LEN(pst_netbuf),
                         hcc_test_event_etc->test_data.pkt_len);
        }
        filter_flag = 1;
    }

    if (hcc_test_event_etc->verified) {
        oal_int32 i;
        oal_int32 flag = 0;
        oal_uint8 *data = OAL_NETBUF_DATA(pst_netbuf);
        for (i = 0; i < OAL_NETBUF_LEN(pst_netbuf); i++) {
            if (*(data + i) != hcc_test_event_etc->test_value) {
                flag = 1;
                OAL_IO_PRINT("[E]data wrong, [i:%d] value:%x should be %x\n",
                             i, *(data + i), hcc_test_event_etc->test_value);
                break;
            }
        }

        if (flag) {
            oal_print_hex_dump(data, OAL_NETBUF_LEN(pst_netbuf), HEX_DUMP_GROUP_SIZE, "hcc rx verified ");
            filter_flag = 1;
        }
    }

    if (!filter_flag) {
        /* filter_flag=1 时接收的数据包不符合要求，则过滤掉 */
        hcc_test_event_etc->test_data.pkt_rcvd++;
        hcc_test_event_etc->test_data.total_rcvd_bytes += OAL_NETBUF_LEN(pst_netbuf);
        hcc_test_event_etc->last_time = ktime_get();
    }

    if (hcc_test_event_etc->test_data.mode_idx == HCC_TEST_CASE_RX) {
        if (time_after(jiffies, (hcc_test_event_etc->start_tick + hcc_test_event_etc->tick_timeout))) {
            if (!hcc_test_event_etc->rx_stop) {
                OAL_IO_PRINT("RxTestTimeIsUp\n");
                ret = hcc_send_message(hcc_get_110x_handler(), H2D_MSG_STOP_SDIO_TEST);
                if (ret) {
                    OAL_IO_PRINT("send message failed, ret=%d", ret);
                }
                hcc_test_event_etc->rx_stop = 1;
            }
        }
    }
}

OAL_STATIC void hcc_test_rcvd_cmd(oal_netbuf_stru *pst_netbuf)
{
    oal_int32 ret;
    hcc_test_cmd_stru cmd;
    ret = memcpy_s(&cmd, OAL_SIZEOF(hcc_test_cmd_stru), OAL_NETBUF_DATA(pst_netbuf), OAL_SIZEOF(hcc_test_cmd_stru));
    if (ret != EOK) {
        OAL_IO_PRINT("memcpy_s err!\n");
    }
    if (cmd.cmd_type == HCC_TEST_CMD_STOP_TEST) {
        ret = memcpy_s(&hcc_test_event_etc->test_data.trans_info,
                       OAL_SIZEOF(hsdio_trans_test_info),
                       hcc_get_test_cmd_data(OAL_NETBUF_DATA(pst_netbuf)),
                       OAL_SIZEOF(hsdio_trans_test_info));
        if (ret != EOK) {
            OAL_IO_PRINT("memcpy_s err!\n");
        }
        hcc_test_event_etc->last_time = ktime_get();
        OAL_IO_PRINT("hcc_test_rcvd:cmd %d recvd!\n", cmd.cmd_type);
        OAL_COMPLETE(&hcc_test_event_etc->test_trans_done);
    }
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    else if ((cmd.cmd_type == HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST) ||
             (cmd.cmd_type == HCC_TEST_CMD_PCIE_PHY_LOOPBACK_TST) ||
             (cmd.cmd_type == HCC_TEST_CMD_PCIE_REMOTE_PHY_LOOPBACK_TST)) {
        ret = memcpy_s(&pcie_test_request_ack, OAL_SIZEOF(pcie_test_request_ack),
                       (oal_void *)hcc_get_test_cmd_data(OAL_NETBUF_DATA(pst_netbuf)),
                       OAL_SIZEOF(pcie_test_request_ack));
        if (ret != EOK) {
            OAL_IO_PRINT("memcpy_s err!\n");
        }
        OAL_COMPLETE(&pcie_test_trans_done);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie test done type=%s",
                             (cmd.cmd_type == HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST) ? "mac" : "phy");
    }
#endif
}

/*
 * Prototype    : hcc_test_rcvd
 * Description  : test pkt rcvd
 * Input        : main_type   uint8
 *                test_type   uint8
 *                skb         sk_buff handle
 *                context     context handle
 * Return Value : succ or fail
 */
OAL_STATIC oal_int32 hcc_test_rcvd(struct hcc_handler *hcc, oal_uint8 stype,
                                   hcc_netbuf_stru *pst_hcc_netbuf,
                                   oal_uint8 *pst_context)
{
    oal_netbuf_stru *pst_netbuf = pst_hcc_netbuf->pst_netbuf;
    OAL_REFERENCE(pst_context);
    OAL_REFERENCE(hcc);

    if (OAL_LIKELY(stype == HCC_TEST_SUBTYPE_DATA)) {
        hcc_test_rcvd_data(pst_netbuf);
    } else if (stype == HCC_TEST_SUBTYPE_CMD) {
        hcc_test_rcvd_cmd(pst_netbuf);
    } else {
        /* unkown subtype */
        OAL_IO_PRINT("receive unkown stype:%d\n", stype);
    }

    oal_netbuf_free(pst_netbuf);

    return OAL_SUCC;
}

/*
 * Prototype    : hcc_test_sent
 * Description  : test pkt sent
 * Input        : uint8  test_type, uint8 gen_type
 * Return Value : succ or fail
 */
OAL_STATIC oal_int32 hcc_test_sent(struct hcc_handler *hcc, struct hcc_transfer_param *param, oal_uint16 start_cmd)
{
    oal_uint8 pad_payload = hcc_test_event_etc->pad_payload;
    oal_netbuf_stru *pst_netbuf;
    /*
    * 1) alloc memory for skb,
    * 2) skb free when send after dequeue from tx queue
    * */
    pst_netbuf = hcc_netbuf_alloc(hcc_test_event_etc->test_data.pkt_len + pad_payload);
    if (pst_netbuf == NULL) {
        OAL_IO_PRINT("hwifi alloc skb fail.\n");
        return -OAL_EFAIL;
    }

    if (pad_payload) {
        oal_netbuf_reserve(pst_netbuf, pad_payload);
    }

    if (hcc_test_event_etc->test_value) {
        memset_s(oal_netbuf_put(pst_netbuf, hcc_test_event_etc->test_data.pkt_len),
                 hcc_test_event_etc->test_data.pkt_len,
                 hcc_test_event_etc->test_value, hcc_test_event_etc->test_data.pkt_len);
    } else {
        oal_netbuf_put(pst_netbuf, hcc_test_event_etc->test_data.pkt_len);
    }

    if (start_cmd == HCC_TEST_SUBTYPE_DATA) {
        hcc_test_event_etc->test_data.total_sent_bytes += OAL_NETBUF_LEN(pst_netbuf);
    }

    return hcc_tx_etc(hcc, pst_netbuf, param);
}

OAL_STATIC oal_int32 hcc_send_test_cmd(oal_uint8 *cmd, oal_int32 hdr_len, oal_int32 cmd_len)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    pst_netbuf = hcc_netbuf_alloc(cmd_len);
    if (pst_netbuf == NULL) {
        OAL_IO_PRINT("hwifi alloc skb fail.\n");
        return -OAL_EFAIL;
    }

    if (hdr_len > cmd_len) {
        hdr_len = cmd_len;
    }

    if (memcpy_s(oal_netbuf_put(pst_netbuf, cmd_len), cmd_len, cmd, hdr_len) != EOK) {
        OAL_IO_PRINT("memcpy_s error, destlen=%d, srclen=%d\n ", cmd_len, hdr_len);
        return -OAL_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_TEST,
                       HCC_TEST_SUBTYPE_CMD,
                       0,
                       HCC_FC_WAIT,
                       hcc_test_event_etc->hcc_test_queue);
    return hcc_tx_etc(hcc, pst_netbuf, &st_hcc_transfer_param);
}

OAL_STATIC oal_int32 hcc_test_rx_start(oal_uint16 start_cmd)
{
    oal_uint32 cmd_len;
    oal_int32 ret;

    hcc_test_cmd_stru *pst_cmd = NULL;
    hcc_trans_test_rx_info *pst_rx_info = NULL;

    struct hcc_handler *hcc;
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }
    cmd_len = OAL_SIZEOF(hcc_test_cmd_stru) + OAL_SIZEOF(hcc_trans_test_rx_info);
    pst_cmd = (hcc_test_cmd_stru *)oal_memalloc(cmd_len);
    if (pst_cmd == NULL) {
        return -OAL_EFAIL;
    }

    OAL_INIT_COMPLETION(&hcc_test_event_etc->test_trans_done);
    hcc_test_event_etc->test_data.pkt_rcvd = 0;
    hcc_test_event_etc->test_data.pkt_sent = 0;
    hcc_test_event_etc->test_data.total_rcvd_bytes = 0;
    hcc_test_event_etc->test_data.total_sent_bytes = 0;
    hcc_test_event_etc->test_data.throughput = 0;
    hcc_test_event_etc->test_data.trans_time = 0;
    hcc_test_event_etc->start_tick = jiffies;
    hcc_test_event_etc->last_time = hcc_test_event_etc->start_time = ktime_get();

    memset_s((oal_void *)pst_cmd, cmd_len, 0, cmd_len);
    pst_cmd->cmd_type = start_cmd;
    pst_cmd->cmd_len = cmd_len;

    pst_rx_info = (hcc_trans_test_rx_info *)hcc_get_test_cmd_data(pst_cmd);

    pst_rx_info->total_trans_pkts = hcc_test_event_etc->test_data.pkt_gen;
    pst_rx_info->pkt_len = hcc_test_event_etc->test_data.pkt_len;
    pst_rx_info->pkt_value = hcc_test_event_etc->test_value;

    if (hcc_send_test_cmd((oal_uint8 *)pst_cmd, pst_cmd->cmd_len, pst_cmd->cmd_len) != OAL_SUCC) {
        oal_free(pst_cmd);
        return -OAL_EFAIL;
    }

    oal_free(pst_cmd);

    hcc_test_event_etc->last_time = ktime_get();

    /* 等待回来的CMD命令 */
    ret = wait_for_completion_interruptible(&hcc_test_event_etc->test_trans_done);
    if (ret < 0) {
        OAL_IO_PRINT("Test Event  terminated ret=%d\n", ret);
        ret = -OAL_EFAIL;
        OAL_IO_PRINT("H2D_MSG_STOP_SDIO_TEST send\n");
        hcc_send_message(hcc, H2D_MSG_STOP_SDIO_TEST);
    }

    if (test_force_stop) {
        hcc_send_message(hcc, H2D_MSG_STOP_SDIO_TEST);
        test_force_stop = 0;
        oal_msleep(100);
    }

    OAL_COMPLETE(&hcc_test_event_etc->test_done);
    return ret;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
static oal_uint16 map_cmd_type(oal_int32 is_phy_loopback)
{
    oal_uint16 cmd_type;

    if (is_phy_loopback == OAL_PHY_LOOPBACK_IS_MAC) {
        cmd_type = HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST;
    } else if (is_phy_loopback == OAL_PHY_LOOPBACK_IS_INTERNALPHY) {
        cmd_type = HCC_TEST_CMD_PCIE_PHY_LOOPBACK_TST;
    } else if (is_phy_loopback == OAL_PHY_LOOPBACK_IS_REMOTEPHY) {
        cmd_type = HCC_TEST_CMD_PCIE_REMOTE_PHY_LOOPBACK_TST;
    } else {
        cmd_type = 0;
    }
    return cmd_type;
}

oal_int32 hcc_test_pcie_loopback(oal_int32 is_phy_loopback)
{
    oal_int32 ret;
    oal_uint32 vol = 0;
    oal_netbuf_stru *pst_netbuf = NULL;
    hcc_test_cmd_stru cmd = {0};
    hcc_pcie_test_request_stru pcie_test_rq = {0};

    struct hcc_handler *hcc = NULL;

    struct hcc_transfer_param st_hcc_transfer_param = {0};

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    OAL_INIT_COMPLETION(&pcie_test_trans_done);
    memset_s(&pcie_test_request_ack, OAL_SIZEOF(pcie_test_request_ack), 0, OAL_SIZEOF(pcie_test_request_ack));
    pcie_test_request_ack.response_ret = HCC_PCIE_RESPONSE_INITIAL_VALUE;

    cmd.cmd_type = map_cmd_type(is_phy_loopback);
    cmd.cmd_len = OAL_SIZEOF(hcc_test_cmd_stru);

    pst_netbuf = hcc_netbuf_alloc(cmd.cmd_len + sizeof(pcie_test_rq));
    if (pst_netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc %u mem failed!", cmd.cmd_len);
        return -OAL_ENOMEM;
    }

    oal_netbuf_put(pst_netbuf, cmd.cmd_len + sizeof(pcie_test_rq));

    ret = memcpy_s((oal_void *)OAL_NETBUF_DATA(pst_netbuf), cmd.cmd_len + sizeof(pcie_test_rq),
                   (oal_void *)&cmd, cmd.cmd_len);
    if (ret != EOK) {
        oal_netbuf_free(pst_netbuf);
        OAL_IO_PRINT("memcpy_s error, destlen=%u, srclen=%u\n ",
                     (oal_uint32)(cmd.cmd_len + sizeof(pcie_test_rq)), (oal_uint32)cmd.cmd_len);
        return -OAL_EFAIL;
    }

    pcie_test_rq.request_flag = (uintptr_t)&pcie_test_rq;
    pcie_test_rq.linkup_timeout = (oal_uint32)ft_pcie_loopback_linkup_timeout;
    pcie_test_rq.rw_test_count = (oal_uint32)ft_pcie_loopback_rw_test_count;
    ret = oal_pcie_get_vol_reg_1v8_value(ft_pcie_loopback_bias_vph, &vol);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        return -OAL_EFAUL;
    }
    pcie_test_rq.bias_vph = (oal_uint32)vol;
    ret = oal_pcie_get_vol_reg_0v9_value(ft_pcie_loopback_bias_cldo3, &vol);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        return -OAL_EFAUL;
    }
    pcie_test_rq.bias_cldo3 = (oal_uint32)vol;
    ret = oal_pcie_get_vol_reg_0v9_value(ft_pcie_loopback_bias_vptx, &vol);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        return -OAL_EFAUL;
    }
    pcie_test_rq.bias_vptx = (oal_uint32)vol;
    ret = memcpy_s((oal_void *)hcc_get_test_cmd_data(OAL_NETBUF_DATA(pst_netbuf)),
                   OAL_SIZEOF(pcie_test_rq), (oal_void *)&pcie_test_rq, OAL_SIZEOF(pcie_test_rq));
    if (ret != EOK) {
        oal_netbuf_free(pst_netbuf);
        OAL_IO_PRINT("memcpy_s error!");
        return -OAL_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_TEST,
                       HCC_TEST_SUBTYPE_CMD,
                       0,
                       HCC_FC_WAIT,
                       DATA_HI_QUEUE);

    ret = hcc_tx_etc(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie test send failed ret=%d", ret);
        oal_netbuf_free(pst_netbuf);
        return ret;
    }
    ret = wait_for_completion_interruptible_timeout(&pcie_test_trans_done, OAL_MSECS_TO_JIFFIES(15000));
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie test Event  terminated ret=%d\n", ret);
        oal_dft_print_all_key_info_etc();
        return ret;
    } else if (ret == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie test timeout");
        oal_dft_print_all_key_info_etc();
        return -OAL_ETIMEDOUT;
    }
    if (pcie_test_rq.request_flag != pcie_test_request_ack.request_flag) {
        ret = -OAL_EFAIL;
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pcie test request mismatch request 0x%llx , return 0x%llx",
                             pcie_test_rq.request_flag,
                             pcie_test_request_ack.request_flag);
    } else {
        if (pcie_test_request_ack.response_ret == OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_test_ok linkup cost %u us , rw_test cost %u us , total %u us",
                                 pcie_test_request_ack.link_up_cost, pcie_test_request_ack.rw_test_cost,
                                 pcie_test_request_ack.total_test_cost);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                 "pcie_test_failed linkup cost %u us , rw_test cost %u us , total %u us, ret=%d",
                                 pcie_test_request_ack.link_up_cost, pcie_test_request_ack.rw_test_cost,
                                 pcie_test_request_ack.total_test_cost, pcie_test_request_ack.response_ret);
        }
        ret = pcie_test_request_ack.response_ret;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie %s test %s", is_phy_loopback ? "phy" : "mac",
                         (ret == OAL_SUCC) ? "succ" : "failed");
    return ret;
}
OAL_STATIC ssize_t hcc_test_pcie_loopback_get_para(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    oal_int32 ret;
    oal_int32 count = 0;
    oal_uint32 buf_len = PAGE_SIZE - 1;
    if (dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev is null");
        return 0;
    }
    if (attr == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "attr is null");
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TestResult: %s\n",
                     (pcie_test_request_ack.response_ret == OAL_SUCC) ? "PASS" : "FAIL");
    if (ret < 0) {
        return count;
    }
    count += ret;
    if (pcie_test_request_ack.response_ret != OAL_SUCC) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "ReturnFailValue: %d\n",
                         pcie_test_request_ack.response_ret);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Bias: CLDO3 %u mv VPTX %u mv VPH %u mv\n",
                     ft_pcie_loopback_bias_cldo3, ft_pcie_loopback_bias_vptx, ft_pcie_loopback_bias_vph);
    if (ret < 0) {
        return count;
    }
    count += ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TimeCost linkup %u us  %u ms\n",
                     pcie_test_request_ack.link_up_cost, pcie_test_request_ack.link_up_cost / 1000);
    if (ret < 0) {
        return count;
    }
    count += ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TimeCost rw_test_cost %u us  %u ms\n",
                     pcie_test_request_ack.rw_test_cost, pcie_test_request_ack.rw_test_cost / 1000);
    if (ret < 0) {
        return count;
    }
    count += ret;
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "TimeCost total_test_cost %u us  %u ms\n",
                     pcie_test_request_ack.total_test_cost, pcie_test_request_ack.total_test_cost / 1000);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}
OAL_STATIC ssize_t hcc_test_pcie_loopback_set_para(struct kobject *dev, struct kobj_attribute *attr,
                                                   const char *buf, size_t count)
{
    oal_int32 ret;
    oal_int32 test_rw_count = 0;
    oal_uint32 cldo3 = 0;
    oal_uint32 vptx = 0;
    oal_uint32 vph = 0;
    oal_int32 is_phy_loopback = 0;
    oal_int32 test_linkup_timeout = 0;
    const oal_uint32 test_case_size = 128;
    char test_case[test_case_size];

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }
    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }
    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= OAL_SIZEOF(test_case)) || (buf[count] != '\0')) {
        OAL_IO_PRINT("input illegal!%s\n", __FUNCTION__);
        return 0;
    }

    if (!oal_strncmp("setparam", buf, OAL_STRLEN("setparam"))) {
        oal_uint32 vol;
        buf += OAL_STRLEN("setparam");
        for (; *buf == ' '; buf++)
            ;
        if ((sscanf(buf, "%u %u %u", &cldo3, &vptx, &vph) != 3)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "error input:%s", buf);
            return count;
        }
        ret = oal_pcie_get_vol_reg_1v8_value(vph, &vol);
        if (ret != OAL_SUCC) {
            return count;
        }
        ret = oal_pcie_get_vol_reg_0v9_value(cldo3, &vol);
        if (ret != OAL_SUCC) {
            return count;
        }
        ret = oal_pcie_get_vol_reg_0v9_value(vptx, &vol);
        if (ret != OAL_SUCC) {
            return count;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "setbias vph:%umv cldo3:%umv vptx:%umv", vph, cldo3, vptx);
        ft_pcie_loopback_bias_vph = vph;
        ft_pcie_loopback_bias_cldo3 = cldo3;
        ft_pcie_loopback_bias_vptx = vptx;
    } else if (!oal_strncmp("runloopback", buf, OAL_STRLEN("runloopback"))) {
        buf += OAL_STRLEN("runloopback");
        for (; *buf == ' '; buf++)
            ;
        if ((sscanf(buf, "%s %d %d", test_case, &test_linkup_timeout, &test_rw_count) != 3)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "error input:%s", buf);
            return count;
        }
        ft_pcie_loopback_rw_test_count = test_rw_count;
        ft_pcie_loopback_linkup_timeout = test_linkup_timeout;
        if (!oal_strcmp("mac", test_case)) {
            is_phy_loopback = OAL_PHY_LOOPBACK_IS_MAC;
        } else if (!oal_strcmp("internalPhy", test_case)) {
            is_phy_loopback = OAL_PHY_LOOPBACK_IS_INTERNALPHY;
        } else if (!oal_strcmp("remotePhy", test_case)) {
            is_phy_loopback = OAL_PHY_LOOPBACK_IS_REMOTEPHY;
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "error input:%s", buf);
            return count;
        }
        pcie_test_request_ack.response_ret = HCC_PCIE_RESPONSE_INITIAL_VALUE;
        hcc_test_pcie_loopback(is_phy_loopback);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unkown:%s\n", buf);
    }
    return count;
}
OAL_STATIC struct kobj_attribute dev_attr_pcie_loopback =
    __ATTR(pcie_loopback, S_IRUGO | S_IWUSR, hcc_test_pcie_loopback_get_para,
           hcc_test_pcie_loopback_set_para);
#endif

oal_int32 hcc_test_fix_wcpu_freq(oal_void)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    hcc_test_cmd_stru cmd = {0};
    oal_int32 ret;

    struct hcc_handler *hcc = NULL;

    struct hcc_transfer_param st_hcc_transfer_param = {0};

    if (ft_ip_test_cpu_max_freq == 0) {
        return OAL_SUCC;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    cmd.cmd_type = HCC_TEST_CMD_CFG_FIX_FREQ;
    cmd.cmd_len = OAL_SIZEOF(hcc_test_cmd_stru);

    pst_netbuf = hcc_netbuf_alloc(cmd.cmd_len);
    if (pst_netbuf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc %u mem failed!", cmd.cmd_len);
        return -OAL_ENOMEM;
    }

    ret = memcpy_s(oal_netbuf_put(pst_netbuf, cmd.cmd_len), cmd.cmd_len, &cmd, cmd.cmd_len);
    if (ret != EOK) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "copy cmd to netbuf failed!");
        return -OAL_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_TEST,
                       HCC_TEST_SUBTYPE_CMD,
                       0,
                       HCC_FC_WAIT,
                       DATA_HI_QUEUE);

    return hcc_tx_etc(hcc, pst_netbuf, &st_hcc_transfer_param);
}

OAL_STATIC oal_int32 hcc_test_normal_start(oal_uint16 start_cmd)
{
    oal_int32 ret = OAL_SUCC;
    oal_int32 retry_count = 0;
    oal_int32 i;
    hcc_test_cmd_stru cmd = {0};

    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = NULL;
    hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_TEST, HCC_TEST_SUBTYPE_DATA, 0, HCC_FC_WAIT,
                       hcc_test_event_etc->hcc_test_queue);
    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wifi is closed!");
        return -OAL_EFAIL;
    }

    OAL_INIT_COMPLETION(&hcc_test_event_etc->test_trans_done);
    hcc_test_event_etc->test_data.pkt_rcvd = 0;
    hcc_test_event_etc->test_data.pkt_sent = 0;
    hcc_test_event_etc->test_data.total_rcvd_bytes = 0;
    hcc_test_event_etc->test_data.total_sent_bytes = 0;
    hcc_test_event_etc->test_data.throughput = 0;
    hcc_test_event_etc->test_data.trans_time = 0;

    cmd.cmd_type = start_cmd;
    cmd.cmd_len = OAL_SIZEOF(hcc_test_cmd_stru) + OAL_SIZEOF(hsdio_trans_test_info);
    if (hcc_send_test_cmd((oal_uint8 *)&cmd, OAL_SIZEOF(cmd), cmd.cmd_len) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    /* wait the device recv the cmd,change the test mode! */
    oal_msleep(50);

    hcc_test_event_etc->last_time = hcc_test_event_etc->start_time = ktime_get();
    hcc_test_event_etc->start_tick = jiffies;

    for (i = 0; i < hcc_test_event_etc->test_data.pkt_gen; i++) {
        ret = hcc_test_sent(hcc, &st_hcc_transfer_param, HCC_TEST_SUBTYPE_DATA);
        if (ret < 0) {
            OAL_IO_PRINT("hcc test gen pkt send fail.\n");
            break;
        }

        hcc_test_event_etc->test_data.pkt_sent++;
        hcc_test_event_etc->last_time = ktime_get();

        if (time_after(jiffies, (hcc_test_event_etc->start_tick + hcc_test_event_etc->tick_timeout))) {
            OAL_IO_PRINT("TestTimeIsUp\n");
            break;
        }

        if (OAL_UNLIKELY(hcc_test_event_etc->started == OAL_FALSE)) {
            ret = -OAL_EFAIL;
            break;
        }
    }

    cmd.cmd_type = HCC_TEST_CMD_STOP_TEST;
    hcc_send_test_cmd((oal_uint8 *)&cmd, OAL_SIZEOF(cmd), cmd.cmd_len);
    hcc_test_event_etc->last_time = ktime_get();

retry:
    /* 等待回来的CMD命令 */
    ret = wait_for_completion_interruptible_timeout(&hcc_test_event_etc->test_trans_done, OAL_MSECS_TO_JIFFIES(5000));
    if (ret < 0) {
        OAL_IO_PRINT("Test Event  terminated ret=%d\n", ret);
        ret = -OAL_EFAIL;
        hcc_print_current_trans_info(0);
        hcc_send_test_cmd((oal_uint8 *)&cmd, OAL_SIZEOF(cmd), cmd.cmd_len);
    } else if (ret == 0) {
        /* cmd response timeout */
        if (retry_count++ < 1) {
            oal_msleep(100);
            hcc_send_test_cmd((oal_uint8 *)&cmd, OAL_SIZEOF(cmd), cmd.cmd_len);
            hcc_test_event_etc->last_time = ktime_get();
            OAL_IO_PRINT("resend the stop cmd!retry count:%d\n", retry_count);
            goto retry;
        } else {
            OAL_IO_PRINT("resend the stop cmd timeout!retry count:%d\n", retry_count);
            ret = -OAL_EFAIL;
        }
    } else {
        if (test_force_stop) {
            hcc_send_test_cmd((oal_uint8 *)&cmd, OAL_SIZEOF(cmd), cmd.cmd_len);
            hcc_test_event_etc->last_time = ktime_get();
            test_force_stop = 0;
            OAL_IO_PRINT("normal start force stop\n");
            oal_msleep(100);
        }
        ret = OAL_SUCC;
    }

    OAL_COMPLETE(&hcc_test_event_etc->test_done);
    return ret;
}

OAL_STATIC oal_int32 hcc_test_start(oal_uint16 start_cmd)
{
    OAL_IO_PRINT("%s Test start.\n", hcc_test_stru_etc[hcc_test_event_etc->test_data.mode_idx].mode);
    if (hcc_test_event_etc->test_data.mode_idx == HCC_TEST_CASE_RX) {
        return hcc_test_rx_start(start_cmd);
    } else {
        return hcc_test_normal_start(start_cmd);
    }
}

/*
 * Prototype    :hcc_test_work_etc
 * Description  :sdio test work_struct function
 * Input        :main_type char
 * Return Value : succ or fail
 */
oal_void hcc_test_work_etc(struct work_struct *work)
{
    oal_uint16 start_cmd;
    oal_int32 ret;

#if defined(CONFIG_ARCH_HISI)
    struct cpumask fast_cpus;
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param = {0};
#endif

    start_cmd = hcc_test_stru_etc[hcc_test_event_etc->test_data.mode_idx].start_cmd;

    if (hcc_test_performance_mode) {
#if defined(CONFIG_ARCH_HISI)
        hisi_get_fast_cpus(&fast_cpus);
#endif

#if defined(CONFIG_ARCH_HISI)
        OAL_IO_PRINT("hcc test bind to fast cpus\n");
        set_cpus_allowed_ptr(current, &fast_cpus);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        param.sched_priority = OAL_BUS_TEST_WORK_PRIORITY;
        oal_set_thread_property_etc(current,
                                    SCHED_RR,
                                    &param,
                                    OAL_BUS_TEST_WORK_NICE);
        oal_msleep(OAL_JIFFIES_TO_MSECS(1));
#endif
    }

    /* hcc test start */
    ret = hcc_test_start(start_cmd);
    if (ret == -OAL_EFAIL) {
        hcc_test_event_etc->errorno = ret;
        OAL_COMPLETE(&hcc_test_event_etc->test_done);
        OAL_IO_PRINT("hcc test work start test pkt send fail. ret = %d\n", ret);
        return;
    }
}

ssize_t hcc_test_print_thoughput(char *buf, oal_uint32 buf_len)
{
    int ret;
    oal_int32 count = 0;
    const char *mode_str = NULL;
    oal_int32 tmp_mode_idx;

    if (buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "buf is null");
        return 0;
    }

    tmp_mode_idx = hcc_test_event_etc->test_data.mode_idx;

    mode_str = "unknown";

    if ((tmp_mode_idx >= 0) && (tmp_mode_idx < OAL_ARRAY_SIZE(hcc_test_stru_etc))) {
        mode_str = hcc_test_stru_etc[tmp_mode_idx].mode;
    }

    hcc_test_throughput_gen();

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Test_Mode: %s\n", mode_str);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Actual sent %d pkts, request %llu bytes\n",
                     hcc_test_event_etc->test_data.pkt_sent,
                     ((oal_uint64)hcc_test_event_etc->test_data.pkt_sent) *
                     (oal_uint64)hcc_test_event_etc->test_data.pkt_len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Actual rcvd %d pkts, request %llu bytes\n",
                     hcc_test_event_etc->test_data.pkt_rcvd,
                     ((oal_uint64)hcc_test_event_etc->test_data.pkt_rcvd) *
                     (oal_uint64)hcc_test_event_etc->test_data.pkt_len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "PayloadSend %llu bytes, ActualSend  %llu bytes\n",
                     hcc_test_event_etc->test_data.total_sent_bytes,
                     hcc_test_event_etc->test_data.trans_info.total_h2d_trans_bytes);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "PayloadRcvd %llu bytes, ActualRecv  %llu bytes\n",
                     hcc_test_event_etc->test_data.total_rcvd_bytes,
                     hcc_test_event_etc->test_data.trans_info.total_d2h_trans_bytes);
    if (ret < 0) {
        return count;
    }
    count += ret;

    /* SDIO通道利用率 */
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Hcc Utilization Ratio %llu\n",
                     hcc_test_utilization_ratio_gen(hcc_test_event_etc->test_data.total_sent_bytes +
                                                    hcc_test_event_etc->test_data.total_rcvd_bytes,
                                                    hcc_test_event_etc->test_data.trans_info.total_h2d_trans_bytes +
                                                    hcc_test_event_etc->test_data.trans_info.total_d2h_trans_bytes));
    if (ret < 0) {
        return count;
    }
    count += ret;

    /* 发送方向的丢包率 */
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "TxPackageLoss %u, pkt_sent: %d actual_tx_pkts: %u\n",
                     hcc_test_tx_pkt_loss_gen(hcc_test_event_etc->test_data.pkt_sent,
                                              hcc_test_event_etc->test_data.trans_info.actual_tx_pkts),
                     hcc_test_event_etc->test_data.pkt_sent,
                     hcc_test_event_etc->test_data.trans_info.actual_tx_pkts);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Requet Generate %d pkts\n",
                     hcc_test_event_etc->test_data.pkt_gen);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Per-package Length %d\n",
                     hcc_test_event_etc->test_data.pkt_len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "TranserTimeCost %llu Seconds, %llu microsecond\n",
                     hcc_test_event_etc->test_data.trans_time, hcc_test_event_etc->test_data.trans_time_us);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "Throughput %u Mbps\n",
                     (oal_int32)hcc_test_event_etc->test_data.throughput);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

/*
 * Prototype    : hcc_test_get_para
 * Description  : get test param
 * Input        : dev   device_handle
 *                attr  device_attribute handle
 *                buf   data buf handle
 * Return Value : get test param
 */
OAL_STATIC ssize_t hcc_test_get_para(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    if (dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev is null");
        return 0;
    }

    if (attr == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "attr is null");
        return 0;
    }

    return hcc_test_print_thoughput(buf, PAGE_SIZE - 1);
}

/*
 * Prototype    : hcc_test_set_para
 * Description  : set test param
 */
OAL_STATIC ssize_t hcc_test_set_para(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    hcc_test_data data = {0};
    oal_int32 tmp_pkt_len;
    oal_int32 tmp_pkt_gen;
    char mode[OAL_SAVE_MODE_BUFF_SIZE] = {0};
    oal_int32 i;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= OAL_SIZEOF(mode)) || (buf[count] != '\0')) {
        OAL_IO_PRINT("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf(buf, "%15s %d %d", mode, &tmp_pkt_len, &tmp_pkt_gen) < 1)) {
        OAL_IO_PRINT("error input,must input more than 1 arguments!\n");
        return -OAL_EINVAL;
    }

    for (i = 0; i < OAL_ARRAY_SIZE(hcc_test_stru_etc); i++) {
        /* find mode if match */
        if (sysfs_streq(hcc_test_stru_etc[i].mode, mode)) {
            break;
        }
    }

    if (i == OAL_ARRAY_SIZE(hcc_test_stru_etc)) {
        OAL_IO_PRINT("unknown test mode.%s\n", mode);

        return -OAL_EINVAL;
    }

    data.pkt_len = tmp_pkt_len;
    data.pkt_gen = tmp_pkt_gen;
    data.mode_idx = i;
    data.trans_timeout = ~0UL;

    if (hcc_test_set_case_etc(&data)) {
        return count;
    }

    return count;
}

/*
 * Prototype    : hcc_test_set_rt_para
 * Description  : set test param
 */
OAL_STATIC ssize_t hcc_test_set_rt_para(struct kobject *dev, struct kobj_attribute *attr,
                                        const char *buf, size_t count)
{
    hcc_test_data data = {0};
    oal_int32 tmp_pkt_len;
    oal_ulong tmp_runtime;
    char mode[OAL_SAVE_MODE_BUFF_SIZE] = {0};
    oal_int32 i;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= OAL_SIZEOF(mode)) || (buf[count] != '\0')) {
        OAL_IO_PRINT("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf(buf, "%15s %d %lu", mode, &tmp_pkt_len, &tmp_runtime) < 1)) {
        OAL_IO_PRINT("error input,must input more than 1 arguments!\n");
        return -OAL_EINVAL;
    }

    for (i = 0; i < OAL_ARRAY_SIZE(hcc_test_stru_etc); i++) {
        /* find mode if match */
        if (sysfs_streq(hcc_test_stru_etc[i].mode, mode)) {
            break;
        }
    }

    if (i == OAL_ARRAY_SIZE(hcc_test_stru_etc)) {
        OAL_IO_PRINT("unknown test mode.%s\n", mode);

        return -OAL_EINVAL;
    }

    data.pkt_len = tmp_pkt_len;
    data.pkt_gen = 0x7fffffff;
    data.mode_idx = i;
    data.trans_timeout = tmp_runtime;

    if (hcc_test_set_case_etc(&data)) {
        return count;
    }

    return count;
}

OAL_STATIC struct kobj_attribute dev_attr_test =
    __ATTR(test, S_IRUGO | S_IWUSR, hcc_test_get_para, hcc_test_set_para);
OAL_STATIC struct kobj_attribute dev_attr_test_rt =
    __ATTR(test_rt, S_IRUGO | S_IWUSR, hcc_test_get_para, hcc_test_set_rt_para);

OAL_STATIC ssize_t hcc_dev_panic_set_para(struct kobject *dev, struct kobj_attribute *attr,
                                          const char *buf, size_t count)
{
    char mode[OAL_SAVE_MODE_BUFF_SIZE] = {0};

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= OAL_SIZEOF(mode)) || (buf[count] != '\0')) {
        OAL_IO_PRINT("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf(buf, "%s", mode) < 1)) {
        OAL_IO_PRINT("error input,must input more than 1 arguments!\n");
        return count;
    }

    if (oal_strncmp(mode, "panic", OAL_STRLEN("panic"))) {
        OAL_IO_PRINT("invalid input:%s\n", mode);
        return count;
    }

    wlan_pm_disable_etc();
    hcc_bus_send_message(hcc_get_current_110x_bus(), H2D_MSG_TEST);
    wlan_pm_enable_etc();
    return count;
}
OAL_STATIC struct kobj_attribute dev_attr_dev_panic =
    __ATTR(dev_panic, S_IWUSR, NULL, hcc_dev_panic_set_para);

OAL_STATIC ssize_t hcc_test_set_abort_test(struct kobject *dev, struct kobj_attribute *attr,
                                           const char *buf, size_t count)
{
    oal_int32 ret;
    struct hcc_handler *hcc;
    hcc = hcc_get_110x_handler();
    ret = hcc_send_message(hcc, H2D_MSG_STOP_SDIO_TEST);
    OAL_IO_PRINT("hcc_test_set_abort_test msg send ret=%d\n", ret);
    oal_msleep(500);
    return count;
}

OAL_STATIC struct kobj_attribute dev_attr_abort_test =
    __ATTR(abort_test, S_IWUSR, NULL, hcc_test_set_abort_test);

OAL_STATIC ssize_t hcc_test_set_value(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    oal_uint32 value;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf(buf, "0x%2x", &value) != 1)) {
        OAL_IO_PRINT("set value one char!\n");
        return -OAL_EINVAL;
    }

    hcc_test_event_etc->test_value = value;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_value(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "0x%2x\n", hcc_test_event_etc->test_value);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_queue_id(struct kobject *dev, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    oal_uint32 queue_id;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf(buf, "%u", &queue_id) != 1)) {
        OAL_IO_PRINT("set value one char!\n");
        return -OAL_EINVAL;
    }
    if (queue_id >= HCC_QUEUE_COUNT) {
        OAL_IO_PRINT("wrong queue id:%u\n", queue_id);
        return count;
    }
    hcc_test_event_etc->hcc_test_queue = (hcc_queue_type)queue_id;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_queue_id(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", hcc_test_event_etc->hcc_test_queue);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_pad_payload(struct kobject *dev, struct kobj_attribute *attr,
                                            const char *buf, size_t count)
{
    oal_uint32 pad_payload;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf(buf, "%u", &pad_payload) != 1)) {
        OAL_IO_PRINT("set value one char!\n");
        return -OAL_EINVAL;
    }

    hcc_test_event_etc->pad_payload = (oal_uint8)pad_payload;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_pad_payload(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }
    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", hcc_test_event_etc->pad_payload);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_verified(struct kobject *dev, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    oal_uint32 verified;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf(buf, "%u", &verified) != 1)) {
        OAL_IO_PRINT("set value one char!\n");
        return -OAL_EINVAL;
    }
    verified = !!verified;
    hcc_test_event_etc->verified = (oal_uint8)verified;

    return count;
}

OAL_STATIC ssize_t hcc_test_get_verified(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", hcc_test_event_etc->verified);
    return ret;
}

OAL_STATIC ssize_t hcc_test_set_switch(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    oal_int32 ret;
    oal_uint8 input[200]; /* 因编译原因，暂不支持用const变量定义数组大小 */

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((count == 0) || (count >= OAL_SIZEOF(input)) || (buf[count] != '\0')) {
        OAL_IO_PRINT("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    memset_s((oal_void *)input, OAL_SIZEOF(input), 0, OAL_SIZEOF(input));

    if ((sscanf(buf, "%s", input) != 1)) {
        OAL_IO_PRINT("set value one char!\n");
        return -OAL_EINVAL;
    }

    ret = conn_test_hcc_chann_switch(input);
    if (ret == OAL_SUCC) {
        OAL_IO_PRINT("swtich to %s succ\n", input);
    } else {
        OAL_IO_PRINT("swtich to %s failed\n", input);
    }

    return count;
}

OAL_STATIC ssize_t hcc_test_get_switch(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    char *current_bus = NULL;
    hcc_bus *pst_bus = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    pst_bus = hcc_get_current_110x_bus();

    if (pst_bus == NULL) {
        current_bus = "none";
    } else {
        if (pst_bus->bus_type == HCC_BUS_SDIO) {
            current_bus = "sdio";
        } else if (pst_bus->bus_type == HCC_BUS_PCIE) {
            current_bus = "pcie";
        } else if (pst_bus->bus_type == HCC_BUS_USB) {
            current_bus = "usb";
        } else {
            current_bus = "unkown type";
        }
    }

    ret += snprintf_s(buf + ret, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", current_bus);
    return ret;
}

OAL_STATIC struct kobj_attribute dev_attr_value =
    __ATTR(value, S_IRUGO | S_IWUSR, hcc_test_get_value, hcc_test_set_value);
OAL_STATIC struct kobj_attribute dev_attr_queue_id =
    __ATTR(queue_id, S_IRUGO | S_IWUSR, hcc_test_get_queue_id, hcc_test_set_queue_id);
OAL_STATIC struct kobj_attribute dev_attr_pad_payload =
    __ATTR(pad_payload, S_IRUGO | S_IWUSR, hcc_test_get_pad_payload, hcc_test_set_pad_payload);
OAL_STATIC struct kobj_attribute dev_attr_verified =
    __ATTR(verified, S_IRUGO | S_IWUSR, hcc_test_get_verified, hcc_test_set_verified);
OAL_STATIC struct kobj_attribute dev_attr_switch_bus =
    __ATTR(switch_bus, S_IRUGO | S_IWUSR, hcc_test_get_switch, hcc_test_set_switch);

OAL_STATIC struct attribute *hcc_test_sysfs_entries[] = {
    &dev_attr_test.attr,
    &dev_attr_test_rt.attr,
    &dev_attr_abort_test.attr,
    &dev_attr_value.attr,
    &dev_attr_verified.attr,
    &dev_attr_queue_id.attr,
    &dev_attr_pad_payload.attr,
    &dev_attr_switch_bus.attr,
    &dev_attr_dev_panic.attr,
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    &dev_attr_pcie_loopback.attr,
#endif
    NULL
};

OAL_STATIC struct attribute_group hcc_test_attribute_group = {
    .name = "test",
    .attrs = hcc_test_sysfs_entries,
};

OAL_STATIC struct attribute *hcc_sysfs_entries[] = {
    &dev_attr_flowctrl.attr,
    &dev_attr_assem_info.attr,
    &dev_attr_queues_pkts.attr,
    &dev_attr_queues_len.attr,
    &dev_attr_wakelock.attr,
    &dev_attr_wakelockall.attr,
    NULL
};
OAL_STATIC struct attribute_group hcc_attribute_group = {
    .attrs = hcc_sysfs_entries,
};

oal_int32 hsdio_sysfs_info_print_etc(oal_void *data, char *buf, oal_int32 buf_len)
{
    oal_int32 ret;
    oal_int32 bit;
    oal_int32 count = 0;
    struct oal_sdio *hi_sdio = (struct oal_sdio *)data;
    if (hi_sdio == NULL) {
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "sdio info, state:0x%4x\n gpio_int_count:%llu \n wakeup_int_count:%llu \n",
                     hi_sdio->state, hi_sdio->pst_bus->gpio_int_count,
                     hi_sdio->pst_bus->wakeup_int_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "func1_err_reg_info:%u \n func1_err_int_count:%u \n func1_ack_int_acount:%u \n",
                     hi_sdio->func1_stat.func1_err_reg_info,
                     hi_sdio->func1_stat.func1_err_int_count,
                     hi_sdio->func1_stat.func1_ack_int_acount);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "func1_msg_int_count:%u \n func1_data_int_count:%u \n func1_no_int_count:%u \n",
                     hi_sdio->func1_stat.func1_msg_int_count,
                     hi_sdio->func1_stat.func1_data_int_count,
                     hi_sdio->func1_stat.func1_no_int_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "\nsdio error stat:\n rx_scatt_info_not_match:%d\n",
                     hi_sdio->error_stat.rx_scatt_info_not_match);
    if (ret < 0) {
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "msg count info:\n tx scatt buf len:%u\n rx scatt buf len:%u\n",
                     hi_sdio->tx_scatt_buff.len, hi_sdio->rx_scatt_buff.len);
    if (ret < 0) {
        return count;
    }
    count += ret;

    for (bit = 0; bit < D2H_MSG_COUNT; bit++) {
        if (hi_sdio->pst_bus->msg[bit].count) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                             "msg [%d] count:%u:,last update time:%llu\n",
                             bit, hi_sdio->pst_bus->msg[bit].count,
                             hi_sdio->pst_bus->msg[bit].cpu_time);
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "last msg:%u\n", hi_sdio->pst_bus->last_msg);
    if (ret < 0) {
        return count;
    }
    count += ret;
    return count;
}

OAL_STATIC ssize_t hsdio_get_sdio_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (oal_get_sdio_default_handler()) {
        ret += hsdio_sysfs_info_print_etc(oal_get_sdio_default_handler(), buf, PAGE_SIZE - ret);
    }

    return ret;
}

oal_int32 hsdio_wakelock_info_print_etc(struct oal_sdio *hi_sdio, char *buf, oal_int32 buf_len)
{
    oal_int32 ret;
    oal_int32 count = 0;

#ifdef CONFIG_PRINTK
    if (hi_sdio->pst_bus->st_bus_wakelock.locked_addr) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakelocked by:%pf\n",
                         (oal_void *)hi_sdio->pst_bus->st_bus_wakelock.locked_addr);
        if (ret < 0) {
            return count;
        }
        count += ret;
    }
#endif

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "hold %lu locks\n",
                     hi_sdio->pst_bus->st_bus_wakelock.lock_count);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

OAL_STATIC ssize_t hsdio_get_wakelock_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    struct hcc_handler *hcc = NULL;

    if (buf == NULL) {
        OAL_IO_PRINT("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        OAL_IO_PRINT("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        OAL_IO_PRINT("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        OAL_IO_PRINT("get hcc handler failed!%s\n", __FUNCTION__);
        return ret;
    }

    if (oal_get_sdio_default_handler()) {
        ret += hsdio_wakelock_info_print_etc(oal_get_sdio_default_handler(), buf, PAGE_SIZE - ret);
    }

    return ret;
}

OAL_STATIC struct kobj_attribute dev_attr_sdio_info = __ATTR(sdio_info, S_IRUGO, hsdio_get_sdio_info, NULL);
OAL_STATIC struct kobj_attribute dev_attr_sdio_wakelock = __ATTR(wakelock, S_IRUGO, hsdio_get_wakelock_info, NULL);
OAL_STATIC struct attribute *hsdio_sysfs_entries[] = {
    &dev_attr_sdio_info.attr,
    &dev_attr_sdio_wakelock.attr,
    NULL
};

OAL_STATIC struct attribute_group hsdio_attribute_group = {
    .name = "sdio",
    .attrs = hsdio_sysfs_entries,
};

int hcc_test_set_case_etc(hcc_test_data *data)
{
    int ret;
    int errorno;
    if (OAL_WARN_ON(data == NULL)) {
        return -OAL_EINVAL;
    }

    if (OAL_UNLIKELY(hcc_test_event_etc->test_workqueue == NULL)) {
        OAL_IO_PRINT("wifi probe failed, please retry.\n");
        return -OAL_EBUSY;
    }

    mutex_lock(&hcc_test_event_etc->mutex_lock);
    if (hcc_test_event_etc->started == OAL_TRUE) {
        OAL_IO_PRINT("sdio test task is processing, wait for end and reinput.\n");
        mutex_unlock(&hcc_test_event_etc->mutex_lock);
        return -OAL_EINVAL;
    }

    OAL_IO_PRINT("%s Test Start,test pkts:%d,pkt len:%d\n",
                 hcc_test_stru_etc[data->mode_idx].mode, data->pkt_gen, data->pkt_len);

    hcc_test_event_etc->started = OAL_TRUE;
    hcc_test_event_etc->rx_stop = 0;
    hcc_test_event_etc->errorno = OAL_SUCC;
    hcc_test_event_etc->tick_timeout = OAL_MSECS_TO_JIFFIES(data->trans_timeout);

    memcpy_s(&hcc_test_event_etc->test_data, sizeof(hcc_test_data), data, sizeof(hcc_test_data));

    test_force_stop = 0;
    INIT_COMPLETION(hcc_test_event_etc->test_done);

    queue_work(hcc_test_event_etc->test_workqueue, &hcc_test_event_etc->test_work);
    ret = wait_for_completion_interruptible(&hcc_test_event_etc->test_done);
    if (ret < 0) {
        OAL_IO_PRINT("Test Event  terminated ret=%d\n", ret);
        hcc_test_event_etc->started = OAL_FALSE;
        OAL_COMPLETE(&hcc_test_event_etc->test_trans_done);
        test_force_stop = 1;
        cancel_work_sync(&hcc_test_event_etc->test_work);
    }

    OAL_IO_PRINT("Test Done.ret=%d\n", hcc_test_event_etc->errorno);

    hcc_test_throughput_gen();

    memcpy_s(data, sizeof(hcc_test_data), &hcc_test_event_etc->test_data, sizeof(hcc_test_data));
    hcc_test_event_etc->started = OAL_FALSE;
    errorno = hcc_test_event_etc->errorno;
    mutex_unlock(&hcc_test_event_etc->mutex_lock);
    return errorno;
}

oal_int32 hcc_test_current_bus_chan(oal_int32 pkt_len,
                                    oal_int32 mode_idx,
                                    oal_ulong trans_timeout,
                                    oal_uint32 min_throught,
                                    oal_int32 retry_cnt)
{
    oal_int32 ret;
    oal_int32 retry_times = 0;

    oal_void *pst_buff = NULL;

    hcc_test_data data = {0};

    data.pkt_len = pkt_len;
    data.pkt_gen = 0x7fffffff;
    data.trans_timeout = trans_timeout;
    data.mode_idx = mode_idx;

    netdev_is_open_etc = OAL_TRUE;
    ret = wlan_pm_open_etc();
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 下发固定频率的命令 */
    ret = hcc_test_fix_wcpu_freq();
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "set  wcpu fix freq failed! ret=%d", ret);
        wlan_pm_close_etc();
        return ret;
    }

    /* 电压拉偏 */
    hcc_bus_voltage_bias_init(hcc_get_current_110x_bus());

retry:

    ret = hcc_test_set_case_etc(&data);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "conn_test_wifi_chan_loop test failed!");
        wlan_pm_close_etc();
        return ret;
    }

    pst_buff = oal_memalloc(PAGE_SIZE);
    if (pst_buff != NULL) {
        memset_s(pst_buff, PAGE_SIZE, 0, 1);
        hcc_test_print_thoughput(pst_buff, PAGE_SIZE - 1);
        OAL_IO_PRINT("%s", (char *)pst_buff);
        oal_free(pst_buff);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc mem failed!");
    }

    if (min_throught > 0) {
        /* 吞吐率门限，当小于最小值时，认为性能失败 */
        if (hcc_test_event_etc->test_data.throughput < (oal_uint64)min_throught) {
            retry_times++;
            if (retry_times <= retry_cnt) {
                /* retry */
                oal_print_hi11xx_log(HI11XX_LOG_WARN,
                    "loop test warning, throught too low, just %llu Mbps, min_limit is %u Mbps, retry_cnt:%d",
                    hcc_test_event_etc->test_data.throughput,
                    min_throught,
                    retry_cnt);
                goto retry;
            } else {
                /* touch retry limit, failed */
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                    "loop test failed, throught too low, just %llu Mbps, min_limit is %u Mbps, retry_cnt:%d",
                    hcc_test_event_etc->test_data.throughput,
                    min_throught,
                    retry_cnt);
                wlan_pm_close_etc();
                return -OAL_EFAIL;
            }
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "throught %llu Mbps over limit %u Mbps, retry times %d, test pass!",
                                 hcc_test_event_etc->test_data.throughput,
                                 min_throught,
                                 retry_times);
        }
    }

    hcc_bus_chip_info(hcc_get_current_110x_bus(), OAL_TRUE, OAL_TRUE);

    ret = wlan_pm_close_etc();
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "conn_test_wifi_chan_loop test failed! ret=%d", ret);
    }

    return OAL_SUCC;
}

/* 测试WIFI通道是否连通 */
oal_int32 conn_test_wifi_chan_loop(const char *param)
{
    oal_int32 ret;
    oal_int32 run_flag = 0;
    hcc_bus *pst_bus = NULL;
    hcc_bus *old_bus = NULL;
#if defined(_PRE_PLAT_FEATURE_HI110X_PCIE) || defined(CONFIG_MMC)
    declare_time_cost_stru(cost);
    oal_int32 pkt_len = 1500;
#endif
    OAL_REFERENCE(param);

    if (!wlan_is_shutdown_etc()) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan is open, test abort!");
        return -OAL_EBUSY;
    }

    old_bus = hcc_get_current_110x_bus();

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    if (oal_pcie_110x_working_check() == OAL_TRUE) {
        oal_int32 pcie_min_throught = 0;
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "PCIe Chan Test!");
        oal_get_time_cost_start(cost);

        pst_bus = hcc_get_current_110x_bus();
        if (pst_bus == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
            return -OAL_EFAIL;
        }

        if (pst_bus->bus_type != HCC_BUS_PCIE) {
            /* 尝试切换到PCIE */
            ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_PCIE);
            if (ret) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
                return -OAL_ENODEV;
            }
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "switch to PCIe ok.");
        }

        /* Test Start */
        /* check pcie loop first */
        ret = hcc_test_current_bus_chan(pkt_len, HCC_TEST_CMD_START_LOOP,
                                        ft_pcie_test_wifi_runtime, 0, 0);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "test PCIe failed, ret=%d", ret);
            return ret;
        }

        if (get_cust_conf_int32_etc(INI_MODU_PLAT, "pcie_min_throught", &pcie_min_throught) == INI_SUCC) {
            ft_pcie_test_min_throught = pcie_min_throught;
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie min throught %d Mbps from ini", pcie_min_throught);
        }

        /* check the pcie throught */
        if (ft_pcie_test_min_throught) {
            ret = hcc_test_current_bus_chan(pkt_len, HCC_TEST_CMD_START_RX,
                                            ft_pcie_test_wifi_runtime,
                                            (oal_uint32)ft_pcie_test_min_throught,
                                            ft_pcie_test_retry_cnt);
            if (ret != OAL_SUCC) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "test PCIe failed, ret=%d", ret);
                return ret;
            }
        }

        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie loop cost %llu us", time_cost_var_sub(cost));

        run_flag++;
    }
#endif

#ifdef CONFIG_MMC
    if (oal_sdio_110x_working_check() == OAL_TRUE) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "SDIO Chan Test!");
        oal_get_time_cost_start(cost);
        pst_bus = hcc_get_current_110x_bus();
        if (pst_bus == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
            return -OAL_EFAIL;
        }

        if (pst_bus->bus_type != HCC_BUS_SDIO) {
            /* 尝试切换到SDIO */
            ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_SDIO);
            if (ret) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
                return -OAL_ENODEV;
            }
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "switch to SDIO ok.");
        }

        /* Test Start */
        ret = hcc_test_current_bus_chan(pkt_len,
                                        HCC_TEST_CMD_START_RX,
                                        ft_sdio_test_wifi_runtime,
                                        0, 0);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "test PCIe failed, ret=%d", ret);
            return ret;
        }

        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sdio loop cost %llu us", time_cost_var_sub(cost));

        run_flag++;
    }
#endif

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
        return -OAL_EFAIL;
    }

    if (old_bus->bus_type != pst_bus->bus_type) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, old_bus->bus_type);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "restore to %u failed, ret=%d", old_bus->bus_type, ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "restore to %d ok.", old_bus->bus_type);
    }

    if (run_flag) {
        return 0;
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "no wifi chan is runing...");
        return -1;
    }
}
EXPORT_SYMBOL(conn_test_wifi_chan_loop);

/* Input, "sdio" or "pcie" */
oal_int32 conn_test_hcc_chann_switch(const char *new_dev)
{
    oal_int32 ret = -OAL_EFAIL;
    if (OAL_WARN_ON(new_dev == NULL)) {
        return -OAL_EINVAL;
    }
    if (!oal_strcmp("sdio", new_dev)) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_SDIO);
    } else if (!oal_strcmp("pcie", new_dev)) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_PCIE);
    } else {
        ret = -OAL_ENODEV;
    }

    return ret;
}
EXPORT_SYMBOL(conn_test_hcc_chann_switch);

oal_void hcc_test_get_case_etc(hcc_test_data *data)
{
    if (OAL_WARN_ON(data == NULL)) {
        return;
    }
    hcc_test_throughput_gen();
    memcpy_s((oal_void *)data, sizeof(hcc_test_data),
             (oal_void *)&hcc_test_event_etc->test_data, sizeof(hcc_test_data));
}

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
OAL_STATIC oal_int32 hwifi_panic_handler(struct notifier_block *this,
                                         oal_ulong event, oal_void *unused)
{
    if (wifi_panic_debug_etc) {
        hwifi_panic_log_dump_etc(KERN_ERR);
    } else {
        printk(KERN_WARNING "wifi panic debug off\n");
    }

    return NOTIFY_OK;
}

OAL_STATIC struct notifier_block hwifi_panic_notifier = {
    .notifier_call = hwifi_panic_handler,
};
#endif

oal_void hcc_test_kirin_noc_handler(oal_void)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    hcc_bus *pst_bus = NULL;

    hi11xx_kernel_crash = 1;

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus != NULL) {
        if (pst_bus->bus_type == HCC_BUS_PCIE) {
#if defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
            oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
            printk(KERN_ERR "hisi wifi noc dump\n");
            if ((pst_pci_lres != NULL) && (pst_pci_lres->pst_pci_res != NULL)) {
                printk(KERN_ERR "pcie link state:%s\n",
                       oal_pcie_get_link_state_str(pst_pci_lres->pst_pci_res->link_state));
            }

            printk(KERN_ERR "host wakeup dev gpio:%d\n", board_get_host_wakeup_dev_stat());
            if (hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_USER) {
                ssi_dump_device_regs(SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT | SSI_MODULE_MASK_PCIE_CUT);
            } else {
                ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
            }
#else
            printk(KERN_ERR "ssi not support\n");
#endif
        }
    }
#endif
}

#if (defined(CONFIG_KIRIN_PCIE_NOC_NOTIFY))
typedef void (*func_pcie_noc_notify)(void);
extern func_pcie_noc_notify g_pcie_noc_notify;
#elif (defined(CONFIG_KIRIN_PCIE_NOC_DBG))
typedef void (*WIFI_DUMP_FUNC)(void);
void register_wifi_dump_func(WIFI_DUMP_FUNC func);
#else
#endif

oal_int32 hcc_test_init_module_etc(struct hcc_handler *hcc)
{
    oal_int32 ret = OAL_SUCC;
    oal_kobject *pst_root_object = NULL;

    if (OAL_UNLIKELY(hcc == NULL)) {
        OAL_WARN_ON(1);
        return OAL_FAIL;
    }

#if (defined(CONFIG_KIRIN_PCIE_NOC_NOTIFY))
    g_pcie_noc_notify = hcc_test_kirin_noc_handler;
#elif (defined(CONFIG_KIRIN_PCIE_NOC_DBG))
    register_wifi_dump_func(hcc_test_kirin_noc_handler);
#else
#endif

    pst_root_object = oal_get_sysfs_root_object_etc();
    if (pst_root_object == NULL) {
        OAL_IO_PRINT("[E]get root sysfs object failed!\n");
        return -OAL_EFAIL;
    }

    conn_syfs_hcc_object = kobject_create_and_add("hcc", pst_root_object);
    if (conn_syfs_hcc_object == NULL) {
        goto fail_conn_syfs_hcc_object;
    }

    ret = oal_debug_sysfs_create_group(conn_syfs_hcc_object, &hsdio_attribute_group);
    if (ret) {
        ret = -OAL_ENOMEM;
        OAL_IO_PRINT("sysfs create sdio_sysfs_entries group fail.ret=%d\n", ret);
        goto fail_create_sdio_group;
    }

    /* create the files associated with this kobject */
    ret = oal_debug_sysfs_create_group(conn_syfs_hcc_object, &hcc_test_attribute_group);
    if (ret) {
        ret = -OAL_ENOMEM;
        OAL_IO_PRINT("sysfs create test group fail.ret=%d\n", ret);
        goto fail_create_hcc_test_group;
    }

    ret = oal_debug_sysfs_create_group(conn_syfs_hcc_object, &hcc_attribute_group);
    if (ret) {
        OAL_IO_PRINT("sysfs create hcc group fail.ret=%d\n", ret);
        ret = -OAL_ENOMEM;
        goto fail_create_hcc_group;
    }

    /* alloc memory for perf_action pointer */
    hcc_test_event_etc = kzalloc(sizeof(*hcc_test_event_etc), GFP_KERNEL);
    if (hcc_test_event_etc == NULL) {
        OAL_IO_PRINT("error kzalloc hcc_test_event_etc mem.\n");
        ret = -OAL_ENOMEM;
        goto fail_g_hcc_test_event;
    }

    hcc_test_event_etc->hcc_test_queue = DATA_LO_QUEUE;
    hcc_test_event_etc->test_value = 0x5a;

    /* register callback for rx */
    ret = hcc_rx_register_etc(hcc, HCC_ACTION_TYPE_TEST, hcc_test_rcvd, NULL);
    if (ret != OAL_SUCC) {
        OAL_IO_PRINT("error %d register callback for rx.\n", ret);
        ret = -OAL_EFAIL;
        goto fail_rx_cb_register;
    }

    /* mutex lock init */
    mutex_init(&hcc_test_event_etc->mutex_lock);

    OAL_INIT_COMPLETION(&hcc_test_event_etc->test_done);
    OAL_INIT_COMPLETION(&hcc_test_event_etc->test_trans_done);

    /* init hcc_test param */
    hcc_test_event_etc->test_data.mode_idx = -1;
    hcc_test_event_etc->test_data.pkt_len = 0;
    hcc_test_event_etc->test_data.pkt_sent = 0;
    hcc_test_event_etc->test_data.pkt_gen = 0;
    hcc_test_event_etc->started = OAL_FALSE;

    /* create workqueue */
    hcc_test_event_etc->test_workqueue = create_singlethread_workqueue("sdio_test");
    if (hcc_test_event_etc == NULL) {
        OAL_IO_PRINT("work queue create fail.\n");
        ret = -OAL_ENOMEM;
        goto fail_sdio_test_workqueue;
    }
    INIT_WORK(&hcc_test_event_etc->test_work, hcc_test_work_etc);

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    atomic_notifier_chain_register(&panic_notifier_list,
                                   &hwifi_panic_notifier);
#endif

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    if (oal_get_sdio_default_handler()) {
        hwifi_panic_log_register_etc(&sdio_panic, oal_get_sdio_default_handler());
    }
    hwifi_panic_log_register_etc(&hcc_panic_assem_info, hcc);
    hwifi_panic_log_register_etc(&hcc_panic_flowctrl, hcc);

    hwifi_panic_log_register_etc(&hcc_panic_queues_len, hcc);
    hwifi_panic_log_register_etc(&hcc_panic_queues_pkts, hcc);
#endif
    return ret;
fail_sdio_test_workqueue:
    hcc_rx_unregister_etc(hcc, HCC_ACTION_TYPE_TEST);
fail_rx_cb_register:
    kfree(hcc_test_event_etc);
    hcc_test_event_etc = NULL;
fail_g_hcc_test_event:
    oal_debug_sysfs_remove_group(conn_syfs_hcc_object, &hcc_attribute_group);
fail_create_hcc_group:
    oal_debug_sysfs_remove_group(conn_syfs_hcc_object, &hcc_test_attribute_group);
fail_create_hcc_test_group:
    oal_debug_sysfs_remove_group(conn_syfs_hcc_object, &hsdio_attribute_group);
fail_create_sdio_group:
    kobject_put(conn_syfs_hcc_object);
fail_conn_syfs_hcc_object:
    return ret;
}

oal_void hcc_test_exit_module_etc(struct hcc_handler *hcc)
{
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    atomic_notifier_chain_unregister(&panic_notifier_list,
                                     &hwifi_panic_notifier);
#endif

    if (hcc_test_event_etc->test_workqueue != NULL) {
        destroy_workqueue(hcc_test_event_etc->test_workqueue);
        hcc_test_event_etc->test_workqueue = NULL;
    }
    oal_debug_sysfs_remove_group(conn_syfs_hcc_object, &hcc_attribute_group);
    oal_debug_sysfs_remove_group(conn_syfs_hcc_object, &hcc_test_attribute_group);
    oal_debug_sysfs_remove_group(conn_syfs_hcc_object, &hsdio_attribute_group);
    kobject_put(conn_syfs_hcc_object);
    hcc_rx_unregister_etc(hcc, HCC_ACTION_TYPE_TEST);

#if (defined(CONFIG_KIRIN_PCIE_NOC_NOTIFY))
    g_pcie_noc_notify = NULL;
#elif (defined(CONFIG_KIRIN_PCIE_NOC_DBG))
    register_wifi_dump_func(NULL);
#else
#endif

    kfree(hcc_test_event_etc);
    hcc_test_event_etc = NULL;
}
#endif
