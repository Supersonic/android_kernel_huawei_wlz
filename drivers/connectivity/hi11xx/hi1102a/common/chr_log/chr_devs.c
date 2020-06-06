

#ifdef CONFIG_HI1102_PLAT_HW_CHR
/* ͷ�ļ����� */
#include "chr_devs.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <asm/atomic.h>
#include <stdarg.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/un.h>

#ifdef CONFIG_HWCONNECTIVITY
#include "hisi_oneimage.h"
#endif
#include "oneimage.h"
#include "board.h"
#include "oal_schedule.h"
#include "chr_errno.h"
#include "oal_hcc_host_if.h"
#include "frw_ext_if.h"
#include "hal_commom_ops.h"
#include "frw_event_main.h"
#include "hw_bfg_ps.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "securec.h"

/* �������� */
static int32 chr_misc_open(struct inode *fd, struct file *fp);
static ssize_t chr_misc_read(struct file *fp, int8 __user *buff, size_t count, loff_t *loff);
static int64 chr_misc_ioctl(struct file *fp, uint32 cmd, uintptr_t arg);
static int32 chr_misc_release(struct inode *fd, struct file *fp);
void chr_rx_errno_to_dispatch(uint32 errno);
int32 chr_wifi_tx_handler(uint32 errno);
int32 chr_bfg_dev_tx_handler(uint32 ul_errno);
uint32 chr_rx_proc_test(uint32 errno);

/* ȫ�ֱ������� */
static CHR_EVENT chr_event;
chr_callback_stru gst_chr_get_wifi_info_callback;

/* ��ģ��debug����ȫ�ֱ��� */
static int32 log_enable = CHR_LOG_DISABLE;

static const struct file_operations chr_misc_fops = {
    .owner = THIS_MODULE,
    .open = chr_misc_open,
    .read = chr_misc_read,
    .release = chr_misc_release,
    .unlocked_ioctl = chr_misc_ioctl,
};

static struct miscdevice chr_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = CHR_DEV_KMSG_PLAT,
    .fops = &chr_misc_fops,
};

/*
 * �� �� ��  : chr_misc_open
 * ��������  : ���豸�ڵ�ӿ�
 */
static int32 chr_misc_open(struct inode *fd, struct file *fp)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s open fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }
    CHR_DBG("chr %s open success\n", chr_misc_dev.name);
    return CHR_SUCC;
}

/*
 * �� �� ��  : chr_misc_read
 * ��������  : ��ȡ�豸�ڵ�ӿ�
 */
static ssize_t chr_misc_read(struct file *fp, int8 __user *buff, size_t count, loff_t *loff)
{
    int32 ret;
    uint32 __user *puser = (uint32 __user *)buff;
    struct sk_buff *skb = NULL;
    uint16 data_len = 0;

    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s read fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }

    if (count < sizeof(CHR_DEV_EXCEPTION_STRU_PARA)) {
        CHR_ERR("The user space buff is too small\n");
        return -CHR_EFAIL;
    }

    if (buff == NULL) {
        CHR_ERR("chr %s read fail, user buff is NULL", chr_misc_dev.name);
        return -EAGAIN;
    }
    skb = skb_dequeue(&chr_event.errno_queue);
    if (skb == NULL) {
        if (fp->f_flags & O_NONBLOCK) {
            CHR_DBG("Thread read chr with NONBLOCK mode\n");
            /* for no data with O_NONBOCK mode return 0 */
            return 0;
        } else {
            if (wait_event_interruptible(chr_event.errno_wait,
                                         (skb = skb_dequeue(&chr_event.errno_queue)) != NULL)) {
                if (skb != NULL) {
                    skb_queue_head(&chr_event.errno_queue, skb);
                }
                CHR_WARNING("Thread interrupt with signel\n");
                return -ERESTARTSYS;
            }
        }
    }

    CHR_WARNING(">>>>>>>>>>>>>>>>>>[chr test]skb len=[%d], chr send event_id[%d] to user\n",
                skb->len, *(uint32 *)skb->data);
    /* event_id(4 bytes) + len(2 bytes) + type(1 byte) + resv(1 byte) + real_errid(16 bytes) = 24 bytes */
    if (skb->len == 24)
    {
        CHR_WARNING(">>>>>>>>>>>>>>>>>>[chr test]chr send real_errid[%d] to user\n",
            *(uint32 *)(skb->data + 8) + *(uint32 *)(skb->data + 12) + 
            (*(uint32 *)(skb->data + 16) << 8) + *(uint32 *)(skb->data + 20));
    }

    data_len = min_t(size_t, skb->len, count);
    ret = copy_to_user(puser, skb->data, data_len);

    if (ret) {
        CHR_WARNING("copy_to_user err!restore it, len=%d\n", data_len);
        skb_queue_head(&chr_event.errno_queue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, data_len);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&chr_event.errno_queue, skb);
    }

    return data_len;
}

/*
 * �� �� ��  : chr_write_errno_to_queue
 * ��������  : ���쳣��д�����
 */
static int32 chr_write_errno_to_queue(uint32 ul_errno, chr_report_flags_enum_uint16 us_flag, uint8 *ptr_data, uint16 ul_len)
{
    struct sk_buff *skb = NULL;
    uint16 sk_len;
    int32 ret = 0;

    if (skb_queue_len(&chr_event.errno_queue) > CHR_ERRNO_QUEUE_MAX_LEN) {
        CHR_WARNING("chr errno queue is full, dispose errno=%x\n", ul_errno);
        return CHR_SUCC;
    }

    /* for code run in interrupt context */
    sk_len = sizeof(CHR_DEV_EXCEPTION_STRU_PARA) + ul_len;
    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        skb = alloc_skb(sk_len, GFP_ATOMIC);
    } else {
        skb = alloc_skb(sk_len, GFP_KERNEL);
    }
    if (skb == NULL) {
        CHR_ERR("chr errno alloc skbuff failed! len=%d, errno=%x\n", sk_len, ul_errno);
        return -ENOMEM;
    }

    skb_put(skb, sk_len);
    *(uint32 *)skb->data = ul_errno;
    *((uint16 *)(skb->data + 4)) = ul_len;  /* ƫ�ƴ��errno��ǰ4���ֽ� */
    *((uint16 *)(skb->data + 6)) = us_flag; /* ƫ�ƴ��errno�ӳ��ȵ�ǰ6���ֽ� */

    if ((ul_len > 0) && (ptr_data != NULL)) {
        ret = memcpy_s(((uint8 *)skb->data + OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA)),
                       sk_len - OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA),
                       ptr_data, ul_len);
        if (ret != EOK) {
            CHR_ERR("memcpy_s error, destlen=%lud, srclen=%d\n ",
                    sk_len - OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA),
                    ul_len);
        }
    }

    skb_queue_tail(&chr_event.errno_queue, skb);
    wake_up_interruptible(&chr_event.errno_wait);
    return CHR_SUCC;
}

/*
 * �� �� ��  : chr_misc_ioctl
 * ��������  : �����豸�ڵ�ӿ�
 */
static int64 chr_misc_ioctl(struct file *fp, uint32 cmd, uintptr_t arg)
{
    uint32 __user *puser = (uint32 __user *)arg;
    uint32 ret;
    uint32 value = 0;
    uint32 *pst_mem = NULL;
    CHR_HOST_EXCEPTION_STRU chr_rx_data;

    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s ioctl fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }

    if (_IOC_TYPE(cmd) != CHR_MAGIC) {
        CHR_ERR("chr %s ioctl fail, the type of cmd is error type is %d\n",
                chr_misc_dev.name, _IOC_TYPE(cmd));
        return -EINVAL;
    }

    if (_IOC_NR(cmd) > CHR_MAX_NR) {
        CHR_ERR("chr %s ioctl fail, the nr of cmd is error, nr is %d\n",
                chr_misc_dev.name, _IOC_NR(cmd));
        return -EINVAL;
    }

    switch (cmd) {
        case CHR_ERRNO_WRITE:
            ret = copy_from_user(&chr_rx_data, (struct CHR_HOST_EXCEPTION_STRU __user *)arg,
                                 OAL_SIZEOF(CHR_HOST_EXCEPTION_STRU));
            if (ret) {
                CHR_ERR("chr %s ioctl fail, get data from user fail", chr_misc_dev.name);
                return -EINVAL;
            }

            if (chr_rx_data.chr_len == 0) {
                chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, NULL, 0);
            } else {
                pst_mem = (uint32 *)oal_memalloc(chr_rx_data.chr_len);
                if (pst_mem == NULL) {
                    CHR_ERR("chr mem alloc failed len %u\n", chr_rx_data.chr_len);
                    return -EINVAL;
                }
                ret = copy_from_user(pst_mem, (uint32 __user *)(chr_rx_data.chr_ptr), chr_rx_data.chr_len);
                if (ret) {
                    CHR_ERR("chr %s ioctl fail, get data from user fail", chr_misc_dev.name);
                    oal_free(pst_mem);
                    return -EINVAL;
                }

                chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, (uint8 *)pst_mem, chr_rx_data.chr_len);
                oal_free(pst_mem);
            }
            break;
        case CHR_ERRNO_ASK:
            ret = get_user(value, puser);
            if (ret) {
                CHR_ERR("chr %s ioctl fail, get data from user fail", chr_misc_dev.name);
                return -EINVAL;
            }

            chr_rx_errno_to_dispatch(value);
            break;
        default:
            CHR_WARNING("chr ioctl not support cmd=0x%x\n", cmd);
            return -EINVAL;
    }
    return CHR_SUCC;
}

/*
 * �� �� ��  : chr_misc_release
 * ��������  : �ͷŽڵ��豸�ӿ�
 */
static int32 chr_misc_release(struct inode *fd, struct file *fp)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s release fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }
    CHR_DBG("chr %s release success\n", chr_misc_dev.name);
    return CHR_SUCC;
}

/*
 * �� �� ��  : __chr_printLog
 * ��������  : �ں���־��ӡ�ӿ�
 */
int32 __chr_printLog(CHR_LOGPRIORITY prio, CHR_DEV_INDEX dev_index, const int8 *fmt, ...)
{
    return CHR_SUCC;
}
EXPORT_SYMBOL(__chr_printLog);

/*
 * �� �� ��  : __chr_exception
 * ��������  : �ں˿ռ����쳣��ӿ�
 */
int32 __chr_exception(uint32 errno)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(errno, CHR_HOST, NULL, 0);
    return CHR_SUCC;
}

int32 __chr_exception_para(uint32 chr_errno, uint8 *chr_ptr, uint16 chr_len)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(chr_errno, CHR_HOST, chr_ptr, chr_len);
    return CHR_SUCC;
}

int32 __chr_exception_para_q(uint32 chr_errno, chr_report_flags_enum_uint16 chr_flag, uint8 *chr_ptr, uint16 chr_len)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }
    chr_write_errno_to_queue(chr_errno, chr_flag, chr_ptr, chr_len);
    return CHR_SUCC;
}

typedef struct {
    char ApErrCode;
    char arpTxErrCode;
    char macHardErrCode;
} dhcpFailChipInfo_STRU;

#define CHR_WIFI_TEST_ERRNO  909050004
#define CHR_BT_TEST_ERRNO    913050006

void chr_test(void)
{
    dhcpFailChipInfo_STRU aa = {0};
    aa.ApErrCode = 'j';
    aa.arpTxErrCode = 'p';
    aa.macHardErrCode = 'p';

    __chr_exception_para(CHR_WIFI_TEST_ERRNO, (oal_uint8 *)&aa, OAL_SIZEOF(dhcpFailChipInfo_STRU));

    chr_rx_errno_to_dispatch(CHR_BT_TEST_ERRNO);
    chr_rx_errno_to_dispatch(CHR_WIFI_TEST_ERRNO);

    OAL_IO_PRINT("{chr_test:: end !}\r\n");
}

uint32 chr_rx_proc_test(uint32 errno)
{
    OAL_IO_PRINT("{chr_rx_proc_test:: errno = %u !}", errno);
    CHR_EXCEPTION_P(errno, NULL, 0);
    return CHR_SUCC;
}

EXPORT_SYMBOL(__chr_exception);
EXPORT_SYMBOL(__chr_exception_para);
EXPORT_SYMBOL(__chr_exception_para_q);

/*
 * �� �� ��  : chr_get_exception_info
 * ��������  : ��ȡCHR ��Ϣ
 */
void chr_get_exception_info(CHR_DEV_EXCEPTION_STRU_PARA *pst_dmac_dev_exception_info,
    CHR_DEV_EXCEPTION_STRU_PARA *pst_hmac_dev_exception_info)
{
    pst_hmac_dev_exception_info->errno  = pst_dmac_dev_exception_info->errno;
    pst_hmac_dev_exception_info->errlen = OAL_SIZEOF(chr_hmac_info_stru) - OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA);
    pst_hmac_dev_exception_info->flag   = pst_dmac_dev_exception_info->flag;
}

/*
 * �� �� ��  : chr_get_common_info
 * ��������  : ��ȡоƬƽ̨��Ϣ
 */
void chr_get_common_info(chr_dmac_common_info_stru *pst_dmac_common_info,
    chr_hmac_common_info_stru *pst_hmac_common_info)
{
    /* ac_fw_ver device�����鳤��Ϊ20��host�೤��Ϊ50������ֻ��Ҫcopy dmac��ac_fw_ver����ĳ��� */
    oal_memcopy(pst_hmac_common_info->ac_fw_ver, pst_dmac_common_info->ac_fw_ver,
                OAL_MIN(OAL_SIZEOF(pst_dmac_common_info->ac_fw_ver), OAL_SIZEOF(pst_hmac_common_info->ac_fw_ver)));
    oal_memcopy(pst_hmac_common_info->ac_ko_ver, pst_dmac_common_info->ac_ko_ver,
                OAL_SIZEOF(pst_hmac_common_info->ac_ko_ver));
    oal_memcopy(pst_hmac_common_info->ac_dieid, pst_dmac_common_info->ac_dieid,
                OAL_SIZEOF(pst_hmac_common_info->ac_dieid));
    pst_hmac_common_info->s_cur_temp_state = pst_dmac_common_info->s_cur_temp_state;
}

/*
 * �� �� ��  : chr_get_rate_info
 * ��������  : ��ȡ����������Ϣ
 */
void chr_get_rate_info(chr_dmac_rate_info_stru *pst_dmac_rate_info, chr_hmac_rate_info_stru *pst_hmac_rate_info)
{
    oal_memcopy(pst_hmac_rate_info, pst_dmac_rate_info, OAL_SIZEOF(chr_hmac_rate_info_stru));
}

/*
 * �� �� ��  : chr_get_radio_link_quality_and_btcoex_info
 * ��������  : ��ȡ�տ���·�����͹��������Ϣ
 */
void chr_get_radio_link_quality_and_btcoex_info(chr_dmac_radio_link_quality_info_stru *pst_dmac_radio_link_quality_info,
                                                chr_hmac_radio_link_quality_info_stru *pst_hmac_radio_link_quality_info,
                                                chr_hmac_btcoex_status_stru *pst_hmac_btcoex_status)
{
    int32                           l_ret;
    hmac_get_wifi_info_ext_stru     st_get_wifi_info_ext;

    /* ʹ�ù��Ӻ�����ȡhmac�����,����ͨ��deviceͨ���ϱ� */
    if (gst_chr_get_wifi_info_callback.chr_get_wifi_ext_info_from_host == OAL_PTR_NULL) {
        return;
    }
    l_ret = gst_chr_get_wifi_info_callback.chr_get_wifi_ext_info_from_host(&st_get_wifi_info_ext);
    if (l_ret != CHR_SUCC) {
        return;
    }

    /* ��ȡ���Ų�����оƬ���Ͳ��� */
    pst_hmac_radio_link_quality_info->en_alg_distance_stat = st_get_wifi_info_ext.uc_device_distance;
    pst_hmac_radio_link_quality_info->en_adj_intf_state = st_get_wifi_info_ext.uc_intf_state_cca;
    pst_hmac_radio_link_quality_info->en_co_intf_state = st_get_wifi_info_ext.uc_intf_state_co;
    pst_hmac_radio_link_quality_info->c_chip_type = st_get_wifi_info_ext.uc_chip_type;

    /* ��ȡ������� */
    oal_memcopy(&pst_hmac_btcoex_status->st_bt_status, &st_get_wifi_info_ext.st_bt_status,
                OAL_SIZEOF(bt_status_stru));
    oal_memcopy(&pst_hmac_btcoex_status->st_ble_status,&st_get_wifi_info_ext.st_ble_status,
                OAL_SIZEOF(btcoex_ble_status_stru));

    pst_hmac_radio_link_quality_info->s_ant0_rssi = pst_dmac_radio_link_quality_info->s_ant0_rssi;

}


/*
 * �� �� ��  : chr_get_chip_count_info
 * ��������  : ��ȡоƬͳ����Ϣ
 */
void chr_get_chip_count_info(chr_dmac_chip_count_info_stru *pst_dmac_chip_count_info,
                             chr_hmac_chip_count_info_stru *pst_hmac_chip_count_info)
{
    oal_memcopy(pst_hmac_chip_count_info, pst_dmac_chip_count_info, OAL_SIZEOF(chr_hmac_chip_count_info_stru));
}

/*
 * �� �� ��  : chr_get_queue_info
 * ��������  : ��ȡ����״̬��Ϣ
 */
void chr_get_queue_info(chr_dmac_queue_info_stru *pst_dmac_queue_info, chr_hmac_queue_info_stru *pst_hmac_queue_info)
{

    oal_memcopy(pst_hmac_queue_info->aus_tx_mpdu_num, pst_dmac_queue_info->aus_tx_mpdu_num,
                OAL_SIZEOF(uint16)*CHR_WLAN_TID_MAX_NUM);
    oal_memcopy(pst_hmac_queue_info->auc_tx_ppdu_num, pst_dmac_queue_info->auc_tx_ppdu_num,
                OAL_SIZEOF(uint8)*CHR_HAL_TX_QUEUE_NUM);
    oal_memcopy(pst_hmac_queue_info->aus_rx_ppdu_num, pst_dmac_queue_info->aus_rx_ppdu_num,
                OAL_SIZEOF(uint8)*CHR_HAL_RX_QUEUE_NUM);

    pst_hmac_queue_info->us_tx_free_ppdu_cnt = pst_dmac_queue_info->us_tx_free_ppdu_cnt;
}

/*
 * �� �� ��  : chr_get_send_mode_info
 * ��������  : ��ȡ����ģʽ��Ϣ
 */
void chr_get_send_mode_info(chr_dmac_send_mode_info_stru *pst_dmac_send_mode_info,
    chr_hmac_send_mode_info_stru *pst_hmac_send_mode_info)
{
    /* host����device��ṹ�����һ������auc_tx_ant��vht��һ����Ա���������Էֶ�copy */
    oal_memcopy(pst_hmac_send_mode_info, pst_dmac_send_mode_info,
                OAL_OFFSET_OF(chr_hmac_send_mode_info_stru, auc_tx_ant));
    /* ������ֵ����4������ */
    oal_memcopy(pst_hmac_send_mode_info->auc_tx_chain, pst_dmac_send_mode_info->auc_tx_chain,
                OAL_SIZEOF(uint8)*CHR_HAL_TX_RATE_MAX_NUM);
    oal_memcopy(pst_hmac_send_mode_info->auc_phy_mode, pst_dmac_send_mode_info->auc_phy_mode,
                OAL_SIZEOF(uint8)*CHR_HAL_TX_RATE_MAX_NUM);
    oal_memcopy(pst_hmac_send_mode_info->auc_edca_cwmax, pst_dmac_send_mode_info->auc_edca_cwmax,
                OAL_SIZEOF(uint8)*CHR_HAL_TX_RATE_MAX_NUM);
    oal_memcopy(pst_hmac_send_mode_info->auc_edca_cwmin, pst_dmac_send_mode_info->auc_edca_cwmin,
                OAL_SIZEOF(uint8)*CHR_HAL_TX_RATE_MAX_NUM);

    oal_memcopy(pst_hmac_send_mode_info->ac_acc_limit, pst_dmac_send_mode_info->ac_acc_limit,
                OAL_SIZEOF(int8)*CHR_WLAN_WME_AC_BUTT);

    pst_hmac_send_mode_info->en_bandwidth        = pst_dmac_send_mode_info->en_bandwidth;
    pst_hmac_send_mode_info->uc_ldpc_implement   = pst_dmac_send_mode_info->uc_ldpc_implement;
    pst_hmac_send_mode_info->uc_ldpc_activate    = pst_dmac_send_mode_info->uc_ldpc_activate;
}

/*
 * �� �� ��  : chr_get_txrx_packets
 * ��������  : ��ȡ�շ���ͳ����Ϣ
 */
void chr_get_txrx_packets(chr_dmac_txrx_pkts_stru *pst_dmac_txrx_pkts, chr_hmac_txrx_pkts_stru *pst_hmac_txrx_pkts)
{
    /* host���device������һ����Ա����rssi����Ҫ�ֶ�copy */
    oal_memcopy(pst_hmac_txrx_pkts, pst_dmac_txrx_pkts, OAL_OFFSET_OF(chr_dmac_txrx_pkts_stru, l_rssi));

    oal_memcopy(&pst_hmac_txrx_pkts->ul_rx_total_pkts, &pst_dmac_txrx_pkts->ul_rx_total_pkts,
                OAL_SIZEOF(chr_dmac_txrx_pkts_stru)-OAL_OFFSET_OF(chr_dmac_txrx_pkts_stru, ul_rx_total_pkts));

}

/*
 * �� �� ��  : chr_get_pm_info
 * ��������  : ��ȡоƬ�͹�����Ϣ
 */
void chr_get_pm_info(chr_dmac_pm_info_stru *pst_dmac_pm_info, chr_hmac_pm_info_stru *pst_hmac_pm_info)
{
    /* ������ֵǰ�������� */
    oal_memcopy(pst_hmac_pm_info, pst_dmac_pm_info, OAL_OFFSET_OF(chr_hmac_pm_info_stru, ul_duty_ratio));

    pst_hmac_pm_info->ul_cpu_freq_level = pst_dmac_pm_info->ul_cpu_freq_level;

    /* host����device��ṹ���8bit��ͬ,�����ܶ�λ��ȡ��ַ,���Բ���ֱ��memcopy */
    pst_hmac_pm_info->bit_pm_more_data_expected = pst_dmac_pm_info->bit_pm_more_data_expected;
    pst_hmac_pm_info->bit_11k_enable            = pst_dmac_pm_info->bit_11k_enable;
    pst_hmac_pm_info->bit_11v_enable            = pst_dmac_pm_info->bit_11v_enable;
    pst_hmac_pm_info->bit_11r_enable            = pst_dmac_pm_info->bit_11r_enable;
    pst_hmac_pm_info->bit_ap_11k_enable         = pst_dmac_pm_info->bit_ap_11k_enable;
    pst_hmac_pm_info->bit_ap_11v_enable         = pst_dmac_pm_info->bit_ap_11v_enable;
    pst_hmac_pm_info->bit_ap_11r_enable         = pst_dmac_pm_info->bit_ap_11r_enable;

}

/*
 * �� �� ��  : chr_dmac_transition_to_hmac
 * ��������  : ��DMAC�ϱ���CHR����ת��HMAC��Ӧ�����ݽṹ
 * �����ϲ�����������
 */
void chr_dmac_transition_to_hmac(chr_dmac_info_stru *buff, uint16 len)
{
    chr_dmac_info_stru *pst_chr_dmac_info = NULL;
    chr_hmac_info_stru *pst_chr_hmac_info = NULL;
    oal_uint8 *chr_data = NULL;

    if (buff == NULL) {
        return;
    }

    if (len != OAL_SIZEOF(chr_dmac_info_stru)) {
        CHR_WARNING("chr recv device data fail, len %d is unavailable, chr_len %d\n",
                    (int32)len, (int32)OAL_SIZEOF(chr_dmac_info_stru));
        return;
    }

    pst_chr_dmac_info = (chr_dmac_info_stru *)buff;
    pst_chr_hmac_info = (chr_hmac_info_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                            OAL_SIZEOF(chr_hmac_info_stru), OAL_TRUE);
    if (pst_chr_hmac_info == OAL_PTR_NULL) {
        return;
    }

    oal_memset(pst_chr_hmac_info, 0, OAL_SIZEOF(chr_hmac_info_stru));

    chr_get_exception_info(&pst_chr_dmac_info->st_dev_exception_info, &pst_chr_hmac_info->st_dev_exception_info);
    chr_get_common_info(&pst_chr_dmac_info->st_common_info, &pst_chr_hmac_info->st_common_info);
    chr_get_rate_info(&pst_chr_dmac_info->st_chr_rate_info, &pst_chr_hmac_info->st_chr_rate_info);
    chr_get_radio_link_quality_and_btcoex_info(&pst_chr_dmac_info->st_radio_link_quality_info,
        &pst_chr_hmac_info->st_radio_link_quality_info, &pst_chr_hmac_info->st_btcoex_status);
    chr_get_chip_count_info(&pst_chr_dmac_info->st_chip_count_info, &pst_chr_hmac_info->st_chip_count_info);
    chr_get_queue_info(&pst_chr_dmac_info->st_queue_info, &pst_chr_hmac_info->st_queue_info);
    chr_get_send_mode_info(&pst_chr_dmac_info->st_send_mode_info, &pst_chr_hmac_info->st_send_mode_info);
    chr_get_txrx_packets(&pst_chr_dmac_info->st_txrx_pkts, &pst_chr_hmac_info->st_txrx_pkts);
    chr_get_pm_info(&pst_chr_dmac_info->st_pm_info, &pst_chr_hmac_info->st_pm_info);

    chr_data = (oal_uint8 *)pst_chr_hmac_info + OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA);
    chr_write_errno_to_queue(pst_chr_hmac_info->st_dev_exception_info.errno,
        pst_chr_hmac_info->st_dev_exception_info.flag, chr_data, pst_chr_hmac_info->st_dev_exception_info.errlen);

    /* �����ϲ�,add chr�ϱ�������־ */
    CHR_EXCEPTION_Q(pst_chr_hmac_info->st_dev_exception_info.errno, CHR_REPORT_FINISH, NULL, 0);

    OAL_MEM_FREE(pst_chr_hmac_info, OAL_TRUE);

}

/*
 * �� �� ��  : chr_dev_exception_callback
 * ��������  : device�쳣�ص��ӿ�
 */
void chr_dev_exception_callback(void *buff, uint16 len)
{
    CHR_DEV_EXCEPTION_STRU_PARA *chr_dev_exception_p = NULL;
    CHR_DEV_EXCEPTION_STRU *chr_dev_exception = NULL;
    oal_uint32 chr_len = 0;
    oal_uint8 *chr_data = NULL;

    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return;
    }

    if (buff == NULL) {
        CHR_WARNING("chr recv device errno fail, buff is NULL\n");
        return;
    }

    chr_dev_exception = (CHR_DEV_EXCEPTION_STRU *)buff;

    /* mode select */
    if (chr_dev_exception->framehead == 0x7e && chr_dev_exception->frametail == 0x7e) {
        /* old interface: chr upload has only errno */
        chr_len = sizeof(CHR_DEV_EXCEPTION_STRU);

        if (len != chr_len) {
            CHR_WARNING("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32)len, chr_len);
            return;
        }

        chr_write_errno_to_queue(chr_dev_exception->error, CHR_DEVICE, NULL, 0);
    } else {
        /* new interface:chr upload eigher has data or not */
        chr_dev_exception_p = (CHR_DEV_EXCEPTION_STRU_PARA *)buff;
        chr_len = sizeof(CHR_DEV_EXCEPTION_STRU_PARA) + chr_dev_exception_p->errlen;

        if (len != chr_len) {
            CHR_WARNING("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32)len, chr_len);
            return;
        }

        if (chr_dev_exception_p->errlen == 0) {
            chr_write_errno_to_queue(chr_dev_exception_p->errno, chr_dev_exception_p->flag, NULL, 0);

        } else {
            if (chr_dev_exception_p->errno <= CHR_WIFI_WEB_SLOW_EVENTID &&
                chr_dev_exception_p->errno >= CHR_WIFI_DISCONNECT_EVENTID) {
                chr_dmac_transition_to_hmac(buff, len);
            } else {
                chr_data = (oal_uint8 *)buff + OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA);
                chr_write_errno_to_queue(chr_dev_exception_p->errno, chr_dev_exception_p->flag,
                    chr_data, chr_dev_exception_p->errlen);
            }
        }
    }
}
EXPORT_SYMBOL(chr_dev_exception_callback);

/*
 * �� �� ��  : chr_host_callback_register
 * ��������  : host�ص�ע�ắ��
 */
void chr_host_callback_register(chr_get_wifi_info pfunc)
{
    if (pfunc == NULL) {
        CHR_ERR("chr_host_callback_register::pfunc is null !");
        return;
    }
    gst_chr_get_wifi_info_callback.chr_get_wifi_info = pfunc;

    return;
}

void chr_host_callback_unregister(void)
{
    gst_chr_get_wifi_info_callback.chr_get_wifi_info = OAL_PTR_NULL;

    return;
}

/*
 * �� �� ��  : chr_get_wifi_ext_info_callback_register
 * ��������  : host�ص�ע�ắ��:���ڻ�ȡdevice�ϱ��Ķ�����Ϣ
 */
void chr_get_wifi_ext_info_callback_register(chr_get_wifi_info_ext pfunc)
{
    if (pfunc == NULL) {
        CHR_ERR("chr_get_wifi_info_callback_register::pfunc is null !");
        return;
    }
    gst_chr_get_wifi_info_callback.chr_get_wifi_ext_info_from_host = pfunc;

    return;
}
void chr_get_wifi_ext_info_callback_unregister(void)
{
    gst_chr_get_wifi_info_callback.chr_get_wifi_ext_info_from_host = OAL_PTR_NULL;

    return;
}


EXPORT_SYMBOL(chr_host_callback_register);
EXPORT_SYMBOL(chr_host_callback_unregister);
EXPORT_SYMBOL(chr_get_wifi_ext_info_callback_register);
EXPORT_SYMBOL(chr_get_wifi_ext_info_callback_unregister);

/*
 * �� �� ��  : chr_rx_errno_to_dispatch
 * ��������  : �����յ���errno���н���������
 */
void chr_rx_errno_to_dispatch(uint32 errno)
{
    uint32 chr_num;
    chr_num = errno / CHR_ID_MSK;
    switch (chr_num) {
        case CHR_WIFI:

            if (chr_wifi_tx_handler(errno) != CHR_SUCC) {
                CHR_ERR("chr_rx_errno_to_dispatch::wifi tx failed,0x%x", errno);
            }
            break;

        case CHR_BT:
        case CHR_GNSS:

            if (chr_bfg_dev_tx_handler(errno) != CHR_SUCC) {
                CHR_ERR("chr_rx_errno_to_dispatch::bt/gnss tx failed,0x%x", errno);
            }
            break;

        default:
            CHR_ERR("chr_rx_errno_to_dispatch::rcv error num 0x%x", errno);
    }
}

/*
 * �� �� ��  : chr_wifi_dev_tx_handler
 * ��������  : ͨ��hccͨ����errno�·���wifi device
 */
int32 chr_wifi_dev_tx_handler(uint32 errno)
{
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    oal_netbuf_stru *pst_netbuf = NULL;
    int32 l_ret;

    if (hcc == NULL) {
        OAL_IO_PRINT("chr_wifi_dev_tx_handler::hcc is null\n");
        return -CHR_EFAIL;
    }

    pst_netbuf = hcc_netbuf_alloc(OAL_SIZEOF(uint32));
    if (pst_netbuf == NULL) {
        OAL_IO_PRINT("hwifi alloc skb fail.\n");
        return -CHR_EFAIL;
    }

    l_ret = memcpy_s(oal_netbuf_put(pst_netbuf, OAL_SIZEOF(uint32)), OAL_SIZEOF(uint32), &errno, OAL_SIZEOF(uint32));
    if (l_ret != EOK) {
        OAL_IO_PRINT("chr_wifi errno copy failed\n");
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_CHR,
                       0,
                       0,
                       HCC_FC_NONE,
                       DATA_HI_QUEUE);
    l_ret = hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (l_ret != CHR_SUCC) {
        OAL_IO_PRINT("chr_wifi_dev_tx_handler::hcc tx is fail,ret=%d\n", l_ret);
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    return CHR_SUCC;
}

/*
 * �� �� ��  : chr_host_tx_handler
 * ��������  : ���ûص��ӿڽ�errno����hmac
 */
int32 chr_host_tx_handler(uint32 errno)
{
    if (gst_chr_get_wifi_info_callback.chr_get_wifi_info == OAL_PTR_NULL) {
        OAL_IO_PRINT("{chr_host_tx_handler:: callback is null!}");
        return CHR_EFAIL;
    }
    if (gst_chr_get_wifi_info_callback.chr_get_wifi_info(errno) != CHR_SUCC) {
        OAL_IO_PRINT("{chr_host_tx_handler:: tx faild, errno = %u !}", errno);
        return CHR_EFAIL;
    }

    return CHR_SUCC;
}

int32 chr_wifi_tx_handler(uint32 errno)
{
    int32 ret1;
    uint32 ret2;

    ret1 = chr_host_tx_handler(errno);

    ret2 = chr_wifi_dev_tx_handler(errno);

    if (ret1 != CHR_SUCC || ret2 != CHR_SUCC) {
        CHR_ERR("chr_wifi_tx_handler::wifi tx failed,errno[0x%x],host tx ret1[%u],device tx ret2[%u]",
                errno, ret1, ret2);
        return -CHR_EFAIL;
    }

    CHR_INFO("chr_wifi_tx_handler::tx is succ,errno %u\n", errno);

    return CHR_SUCC;
}

/*
 * �� �� ��  : chr_bfg_dev_tx_handler
 * ��������  : ����uartͨ����errno����bfg
 */
int32 chr_bfg_dev_tx_handler(uint32 ul_errno)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 sk_len;
    int32 ret;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        CHR_ERR("chr_bfg_dev_tx_handler::ps_core_d is NULL\n");
        return CHR_EFAIL;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        CHR_ERR("bt tx high seqlen large than MAXNUM\n");
        return CHR_EFAIL;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret != CHR_SUCC) {
        CHR_ERR("chr_bfg_dev_tx_handler::prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    /* alloc skb buf */
    sk_len = sizeof(uint32) + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);
    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        skb = alloc_skb(sk_len, GFP_ATOMIC);
    } else {
        skb = alloc_skb(sk_len, GFP_KERNEL);
    }
    if (skb == NULL) {
        CHR_ERR("chr_bfg_dev_tx_handler::alloc skbuff failed! len=%d, errno=0x%x\n", sk_len, ul_errno);
        post_to_visit_node(ps_core_d);
        return -CHR_EFAIL;
    }

    skb_put(skb, sk_len);

    /* skb data init,reuse the type of mem_dump to prevent the change of rom */
    ps_add_packet_head(skb->data, MEM_DUMP, sk_len);

    /* put errno into skb_data */
    *(uint32 *)(skb->data + sizeof(struct ps_packet_head)) = ul_errno;

    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    post_to_visit_node(ps_core_d);

    CHR_WARNING("chr_bfg_dev_tx_handler::tx is succ,errno %u\n", ul_errno);

    return CHR_SUCC;
}

int32 chr_miscdevs_init(void)
{
    int32 ret;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (!is_my_chip()) {
        CHR_INFO("cfg chr log chip type is not match, skip driver init");
        log_enable = CHR_LOG_DISABLE;
        return -EINVAL;
    } else {
        CHR_INFO("cfg chr log is matched with hi110x, continue");
    }
#endif
    init_waitqueue_head(&chr_event.errno_wait);
    skb_queue_head_init(&chr_event.errno_queue);

    ret = misc_register(&chr_misc_dev);
    if (ret != CHR_SUCC) {
        CHR_ERR("chr module init fail\n");
        return -CHR_EFAIL;
    }
    log_enable = CHR_LOG_ENABLE;
    CHR_INFO("chr module init succ\n");

    return CHR_SUCC;
}

void chr_miscdevs_exit(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (!is_my_chip()) {
        CHR_INFO("cfg chr log chip type is not match, skip driver init");
        return;
    } else {
        CHR_INFO("cfg chr log is matched with hi110x, continue");
    }
#endif
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_INFO("chr module is diabled\n");
        return;
    }

    misc_deregister(&chr_misc_dev);
    log_enable = CHR_LOG_DISABLE;
    CHR_INFO("chr module exit succ\n");
}

MODULE_AUTHOR("Hisilicon platform Driver Group");
MODULE_DESCRIPTION("hi110x chr log driver");
MODULE_LICENSE("GPL");
#endif
