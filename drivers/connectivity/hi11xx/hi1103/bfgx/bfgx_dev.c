

/* Include Head file */
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/version.h>
#if ((defined CONFIG_LOG_EXCEPTION) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) && \
    (!defined PLATFORM_DEBUG_ENABLE))
#include <log/log_usertype.h>
#endif


#include "board.h"
#include "hw_bfg_ps.h"
#include "plat_type.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "plat_pm.h"
#include "bfgx_user_ctrl.h"
#include "bfgx_exception_rst.h"
#include "plat_firmware.h"
#include "plat_cali.h"
#include "securec.h"
#include "oal_ext_if.h"
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#include "wireless_patch.h"
#endif
#include "gps_refclk_src_3.h"

/*
 * This references is the per-PS platform device in the arch/arm/
 * board-xx.c file.
 */
struct platform_device *hw_ps_device_etc = NULL;
STATIC int debug_cnt = 0;
DUMP_CMD_QUEUE dump_cmd_queue_etc;

uint32 bfgx_open_cmd_etc[BFGX_BUTT] = {
    SYS_CFG_OPEN_BT,
    SYS_CFG_OPEN_FM,
    SYS_CFG_OPEN_GNSS,
    SYS_CFG_OPEN_IR,
    SYS_CFG_OPEN_NFC,
};

uint32 bfgx_close_cmd_etc[BFGX_BUTT] = {
    SYS_CFG_CLOSE_BT,
    SYS_CFG_CLOSE_FM,
    SYS_CFG_CLOSE_GNSS,
    SYS_CFG_CLOSE_IR,
    SYS_CFG_CLOSE_NFC,
};

uint32 bfgx_open_cmd_timeout_etc[BFGX_BUTT] = {
    WAIT_BT_OPEN_TIME,
    WAIT_FM_OPEN_TIME,
    WAIT_GNSS_OPEN_TIME,
    WAIT_IR_OPEN_TIME,
    WAIT_NFC_OPEN_TIME,
};

uint32 bfgx_close_cmd_timeout_etc[BFGX_BUTT] = {
    WAIT_BT_CLOSE_TIME,
    WAIT_FM_CLOSE_TIME,
    WAIT_GNSS_CLOSE_TIME,
    WAIT_IR_CLOSE_TIME,
    WAIT_NFC_CLOSE_TIME,
};

const uint8 *bfgx_subsys_name_etc[BFGX_BUTT] = {
    "BT",
    "FM",
    "GNSS",
    "IR",
    "NFC",
};

struct bt_data_combination bt_data_combination_etc = {0};

oal_int32 bfgx_open_ssi_dump = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(bfgx_open_ssi_dump, int, S_IRUGO | S_IWUSR);
#endif

uint32 gnss_me_thread_status = DEV_THREAD_EXIT;
uint32 gnss_lppe_thread_status = DEV_THREAD_EXIT;
volatile bool ir_only_mode = false;

static bool device_log_status = false;
void ps_set_device_log_status_etc(bool status)
{
    device_log_status = status;
}
bool ps_is_device_log_enable_etc(void)
{
#if ((defined CONFIG_LOG_EXCEPTION) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) && \
    (!defined PLATFORM_DEBUG_ENABLE))
    unsigned int log_usertype = get_log_usertype();
    bool status = ((log_usertype > COMMERCIAL_USER) && (log_usertype < OVERSEA_COMMERCIAL_USER));
    status = (status || device_log_status);
    return status;
#else
    return true;
#endif
}

/*
 * Prototype    : ps_get_plat_reference_etc
 * Description  : reference the plat's dat,This references the per-PS
 *                  platform device in the arch/arm/board-xx.c file.
 */
int32 ps_get_plat_reference_etc(struct ps_plat_s **plat_data)
{
    struct platform_device *pdev = NULL;
    struct ps_plat_s *ps_plat_d = NULL;

    pdev = hw_ps_device_etc;
    if (pdev == NULL) {
        *plat_data = NULL;
        PS_PRINT_ERR("%s pdev is NULL\n", __func__);
        return FAILURE;
    }

    ps_plat_d = dev_get_drvdata(&pdev->dev);
    *plat_data = ps_plat_d;

    return SUCCESS;
}

/*
 * Prototype    : ps_get_core_reference_etc
 * Description  : reference the core's data,This references the per-PS
 *                platform device in the arch/xx/board-xx.c file..
 */
int32 ps_get_core_reference_etc(struct ps_core_s **core_data)
{
    struct platform_device *pdev = NULL;
    struct ps_plat_s *ps_plat_d = NULL;

    pdev = hw_ps_device_etc;
    if (pdev == NULL) {
        *core_data = NULL;
        PS_PRINT_ERR("%s pdev is NULL\n", __func__);
        return FAILURE;
    }

    ps_plat_d = dev_get_drvdata(&pdev->dev);
    if (ps_plat_d == NULL) {
        PS_PRINT_ERR("ps_plat_d is NULL\n");
        return FAILURE;
    }

    *core_data = ps_plat_d->core_data;

    return SUCCESS;
}

/*
 * Prototype    : ps_chk_bfg_active_etc
 * Description  : to chk wether or not bfg active
 */
bool ps_chk_bfg_active_etc(struct ps_core_s *ps_core_d)
{
    int32 i = 0;
    for (i = 0; i < BFGX_BUTT; i++) {
        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) != POWER_STATE_SHUTDOWN) {
            return true;
        }
    }

    return false;
}

/* only gnss is open and it agree to sleep */
bool ps_chk_only_gnss_and_cldslp_etc(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if ((atomic_read(&ps_core_d->bfgx_info[BFGX_BT].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_FM].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_IR].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_NFC].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_GNSS].subsys_state) == POWER_STATE_OPEN) &&
        (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP) &&
        (pm_data->bfgx_dev_state == BFGX_ACTIVE)) {
        return true;
    }
    return false;
}

bool ps_chk_tx_queue_empty(struct ps_core_s *ps_core_d)
{
    PS_PRINT_FUNCTION_NAME;
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL");
        return true;
    }

    if (skb_queue_empty(&ps_core_d->tx_urgent_seq) &&
        skb_queue_empty(&ps_core_d->tx_high_seq) &&
        skb_queue_empty(&ps_core_d->tx_low_seq)) {
        return true;
    }
    return false;
}

/*
 * Prototype    : ps_alloc_skb_etc
 * Description  : allocate mem for new skb
 */
struct sk_buff *ps_alloc_skb_etc(uint16 len)
{
    struct sk_buff *skb = NULL;

    PS_PRINT_FUNCTION_NAME;

    skb = alloc_skb(len, GFP_KERNEL);
    if (skb == NULL) {
        PS_PRINT_WARNING("can't allocate mem for new skb, len=%d\n", len);
        return NULL;
    }

    skb_put(skb, len);

    return skb;
}

/*
 * Prototype    : ps_kfree_skb_etc
 * Description  : when close a function, kfree skb
 */
void ps_kfree_skb_etc(struct ps_core_s *ps_core_d, uint8 type)
{
    struct sk_buff *skb = NULL;

    PS_PRINT_FUNCTION_NAME;
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL");
        return;
    }

    while ((skb = ps_skb_dequeue_etc(ps_core_d, type))) {
        kfree_skb(skb);
    }

    switch (type) {
        case TX_URGENT_QUEUE:
            skb_queue_purge(&ps_core_d->tx_urgent_seq);
            break;
        case TX_HIGH_QUEUE:
            skb_queue_purge(&ps_core_d->tx_high_seq);
            break;
        case TX_LOW_QUEUE:
            skb_queue_purge(&ps_core_d->tx_low_seq);
            break;
        case RX_GNSS_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue);
            break;
        case RX_FM_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_FM].rx_queue);
            break;
        case RX_BT_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_BT].rx_queue);
            break;
        case RX_DBG_QUEUE:
            skb_queue_purge(&ps_core_d->rx_dbg_seq);
            break;
        case RX_NFC_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue);
            break;
        case RX_IR_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_IR].rx_queue);
            break;
        default:
            PS_PRINT_ERR("queue type is error, type=%d\n", type);
            break;
    }
    return;
}

/*
 * Prototype    : ps_restore_skbqueue_etc
 * Description  : when err and restore skb to seq function.
 */
int32 ps_restore_skbqueue_etc(struct ps_core_s *ps_core_d, struct sk_buff *skb, uint8 type)
{
    PS_PRINT_FUNCTION_NAME;

    if (unlikely((skb == NULL) || (ps_core_d == NULL))) {
        PS_PRINT_ERR(" skb or ps_core_d is NULL\n");
        return -EINVAL;
    }

    switch (type) {
        case RX_GNSS_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue, skb);
            break;
        case RX_FM_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_FM].rx_queue, skb);
            break;
        case RX_BT_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_BT].rx_queue, skb);
            break;
        case RX_IR_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_IR].rx_queue, skb);
            break;
        case RX_NFC_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue, skb);
            break;
        case RX_DBG_QUEUE:
            skb_queue_head(&ps_core_d->rx_dbg_seq, skb);
            break;

        default:
            PS_PRINT_ERR("queue type is error, type=%d\n", type);
            break;
    }

    return 0;
}

/* prepare to visit dev_node */
int32 prepare_to_visit_node_etc(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = NULL;
    uint8 uart_ready;
    uint64 flags;

    pm_data = pm_get_drvdata_etc();
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EFAULT;
    }

    /* lock wake_lock */
    bfg_wake_lock_etc();

    /* try to wake up device */
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    atomic_inc(&ps_core_d->node_visit_flag); /* mark someone is visiting dev node */
    uart_ready = ps_core_d->ps_pm->bfgx_uart_state_get();
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    if (uart_ready == UART_NOT_READY) {
        if (host_wkup_dev_etc() != 0) {
            PS_PRINT_ERR("wkup device FAILED!\n");
            atomic_dec(&ps_core_d->node_visit_flag);
            return -EIO;
        }
    }
    return 0;
}

/* we should do something before exit from visiting dev_node */
int32 post_to_visit_node_etc(struct ps_core_s *ps_core_d)
{
    atomic_dec(&ps_core_d->node_visit_flag);

    return 0;
}

int32 alloc_seperted_rx_buf_etc(uint8 subsys, uint32 len, uint8 alloctype)
{
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint8 *p_rx_buf = NULL;

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys out of range! subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_BT) {
        PS_PRINT_DBG("%s no sepreted buf\n", bfgx_subsys_name_etc[subsys]);
        return 0;
    }

    if (alloctype >= ALLOC_BUFF) {
        PS_PRINT_ERR("alloc type out of range! subsys=%d,alloctype=%d\n", subsys, alloctype);
        return -EINVAL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }
    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    if (alloctype == KZALLOC) {
        p_rx_buf = kzalloc(len, GFP_KERNEL);
    } else if (alloctype == VMALLOC) {
        p_rx_buf = vmalloc(len);
    }

    if (p_rx_buf == NULL) {
        PS_PRINT_ERR("alloc failed! subsys=%d, len=%d\n", subsys, len);
        return -ENOMEM;
    }

    spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    pst_sepreted_data->rx_prev_seq = RX_SEQ_NULL;
    pst_sepreted_data->rx_buf_all_len = 0;
    pst_sepreted_data->rx_buf_ptr = p_rx_buf;
    pst_sepreted_data->rx_buf_org_ptr = p_rx_buf;
    spin_unlock(&pst_sepreted_data->sepreted_rx_lock);

    return 0;
}

int32 free_seperted_rx_buf_etc(uint8 subsys, uint8 alloctype)
{
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint8 *buf_ptr = NULL;

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys out of range! subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_BT) {
        PS_PRINT_DBG("%s no sepreted buf\n", bfgx_subsys_name_etc[subsys]);
        return 0;
    }

    if (alloctype >= ALLOC_BUFF) {
        PS_PRINT_ERR("alloc type out of range! subsys=%d,alloctype=%d\n", subsys, alloctype);
        return -EINVAL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }
    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
    spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    pst_sepreted_data->rx_prev_seq = RX_SEQ_NULL;
    pst_sepreted_data->rx_buf_all_len = 0;
    pst_sepreted_data->rx_buf_ptr = NULL;
    pst_sepreted_data->rx_buf_org_ptr = NULL;
    spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
    if (buf_ptr != NULL) {
        if (alloctype == KZALLOC) {
            kfree(buf_ptr);
        } else if (alloctype == VMALLOC) {
            vfree(buf_ptr);
        }
    }

    return 0;
}

int32 bfgx_open_fail_process_etc(uint8 subsys, int32 error)
{
    struct ps_core_s *ps_core_d = NULL;

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys is error, subsys=[%d]\n", subsys);
        return BFGX_POWER_FAILED;
    }

    if (error >= BFGX_POWER_ENUM_BUTT) {
        PS_PRINT_ERR("error is error, error=[%d]\n", error);
        return BFGX_POWER_FAILED;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return BFGX_POWER_FAILED;
    }

    PS_PRINT_INFO("bfgx open fail, type=[%d]\n", error);

    if (bfgx_open_ssi_dump) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL);
    }

    switch (error) {
        case BFGX_POWER_PULL_POWER_GPIO_FAIL:
        case BFGX_POWER_TTY_OPEN_FAIL:
        case BFGX_POWER_TTY_FLOW_ENABLE_FAIL:
            break;

        case BFGX_POWER_WIFI_DERESET_BCPU_FAIL:
        case BFGX_POWER_WIFI_ON_BOOT_UP_FAIL:
            if (plat_power_fail_exception_info_set_etc(SUBSYS_BFGX, subsys, BFGX_POWERON_FAIL) == BFGX_POWER_SUCCESS) {
                bfgx_system_reset_etc();
                plat_power_fail_process_done_etc();
            } else {
                PS_PRINT_ERR("bfgx power fail, set exception info fail\n");
            }

            ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
            break;

        case BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL:
        case BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL:
            ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
            break;

        case BFGX_POWER_WAKEUP_FAIL:
        case BFGX_POWER_OPEN_CMD_FAIL:
            if (plat_power_fail_exception_info_set_etc(SUBSYS_BFGX, subsys, BFGX_POWERON_FAIL) == BFGX_POWER_SUCCESS) {
                if (bfgx_subsystem_reset_etc() != EXCEPTION_SUCCESS) {
                    PS_PRINT_ERR("bfgx_subsystem_reset_etc failed \n");
                }
                plat_power_fail_process_done_etc();
            } else {
                PS_PRINT_ERR("bfgx power fail, set exception info fail\n");
            }

            ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
            break;

        default:
            PS_PRINT_ERR("error is undefined, error=[%d]\n", error);
            break;
    }

    return BFGX_POWER_SUCCESS;
}

/*
 * Prototype    : uart_wifi_open_etc
 * Description  : functions called by wifi pm to open wifi throuhg bfgx system
 */
int32 uart_wifi_open_etc(void)
{
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft;
    int32 ret;

    PS_PRINT_INFO("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    /* ���BFGIN˯�ߣ�����֮ */
    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return ret;
    }

    PS_PRINT_INFO("uart open WCPU\n");
    INIT_COMPLETION(ps_core_d->wait_wifi_opened);
    /* tx sys bt open info */
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_OPEN_WIFI);

    timeleft = wait_for_completion_timeout(&ps_core_d->wait_wifi_opened, msecs_to_jiffies(WAIT_WIFI_OPEN_TIME));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait wifi open ack timeout\n");
        post_to_visit_node_etc(ps_core_d);
        return -ETIMEDOUT;
    }

    post_to_visit_node_etc(ps_core_d);

    msleep(20);

    return SUCCESS;
}

/*
 * Prototype    : uart_wifi_close_etc
 * Description  : functions called by wifi pm to close wifi throuhg bfgx system
 */
int32 uart_wifi_close_etc(void)
{
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft;
    int32 ret;

    PS_PRINT_INFO("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    /* ���BFGIN˯�ߣ�����֮ */
    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return ret;
    }

    PS_PRINT_INFO("uart close WCPU\n");

    INIT_COMPLETION(ps_core_d->wait_wifi_closed);
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_CLOSE_WIFI);

    timeleft = wait_for_completion_timeout(&ps_core_d->wait_wifi_closed, msecs_to_jiffies(WAIT_WIFI_CLOSE_TIME));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait wifi close ack timeout\n");
        post_to_visit_node_etc(ps_core_d);
        return -ETIMEDOUT;
    }

    PS_PRINT_WARNING("uart close WCPU done,gpio level[%d]\n", board_get_wlan_wkup_gpio_val_etc());

    post_to_visit_node_etc(ps_core_d);

    return SUCCESS;
}

/*
 * Prototype    : uart_bfgx_close_cmd_etc
 * Description  : functions called by bfgn pm to close bcpu throuhg bfgx system
 */
int32 uart_bfgx_close_cmd_etc(void)
{
#define wait_close_times 100
    struct ps_core_s *ps_core_d = NULL;
    int bwkup_gpio_val = 1;
    int32 ret;
    int i;

    PS_PRINT_INFO("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    /* ������dev������ϵͳ��Ϣ */
    if (ir_only_mode) {
        return SUCCESS;
    }

    /* ���BFGIN˯�ߣ�����֮ */
    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return ret;
    }

    /* �·�BFGIN shutdown���� */
    PS_PRINT_INFO("uart shutdown BCPU\n");

    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_SHUTDOWN_SLP);

    ret = FAILURE;
    for (i = 0; i < wait_close_times; i++) {
        bwkup_gpio_val = board_get_bwkup_gpio_val_etc();
        if (bwkup_gpio_val == 0) {
            ret = SUCCESS;
            break;
        }
        msleep(10);
    }
    PS_PRINT_INFO("bfg gpio level:%d, i=%d\n", bwkup_gpio_val, i);

    if (ret == FAILURE) {
        ps_uart_state_dump_etc(ps_core_d->tty);
    }

    post_to_visit_node_etc(ps_core_d);

    return ret;
}

int32 bfgx_open_cmd_send_etc(uint32 subsys)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return -EINVAL;
    }

    /* ������dev������ϵͳ��Ϣ */
    if (ir_only_mode) {
        return SUCCESS;
    }

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_IR) {
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, bfgx_open_cmd_etc[subsys]);
        msleep(20);
        return 0;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (is_dfr_test_en(BFGX_POWEON_FAULT)) {
        PS_PRINT_WARNING("[dfr test]:trigger powon fail\n");
        return -EINVAL;
    }
#endif

    if (subsys == BFGX_GNSS) {
        gnss_me_thread_status = DEV_THREAD_EXIT;
        gnss_lppe_thread_status = DEV_THREAD_EXIT;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    INIT_COMPLETION(pst_bfgx_data->wait_opened);
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, bfgx_open_cmd_etc[subsys]);
    timeleft = wait_for_completion_timeout(&pst_bfgx_data->wait_opened,
                                           msecs_to_jiffies(bfgx_open_cmd_timeout_etc[subsys]));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait %s open ack timeout\n", bfgx_subsys_name_etc[subsys]);
        if (subsys == BFGX_GNSS) {
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                                 CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_OPEN_THREAD);
        } else if (subsys == BFGX_BT) {
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_BT, CHR_LAYER_DRV,
                                 0, CHR_PLAT_DRV_ERROR_OPEN_THREAD);
        }
        return -ETIMEDOUT;
    }

    return 0;
}

int32 bfgx_close_cmd_send_etc(uint32 subsys)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    /* ������dev������ϵͳ��Ϣ */
    if (ir_only_mode) {
        return SUCCESS;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (is_dfr_test_en(BFGX_POWEOFF_FAULT)) {
        PS_PRINT_WARNING("[dfr test]:trigger power off fail\n");
        return -EINVAL;
    }
#endif

    if (subsys == BFGX_IR) {
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, bfgx_close_cmd_etc[subsys]);
        msleep(20);
        return 0;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    INIT_COMPLETION(pst_bfgx_data->wait_closed);
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, bfgx_close_cmd_etc[subsys]);
    timeleft = wait_for_completion_timeout(&pst_bfgx_data->wait_closed,
                                           msecs_to_jiffies(bfgx_close_cmd_timeout_etc[subsys]));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait %s close ack timeout\n", bfgx_subsys_name_etc[subsys]);
        if (subsys == BFGX_GNSS) {
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                                 CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_CLOSE_THREAD);
        } else if (subsys == BFGX_BT) {
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_BT, CHR_LAYER_DRV,
                                 0, CHR_PLAT_DRV_ERROR_CLOSE_THREAD);
        }
        return -ETIMEDOUT;
    }

    return 0;
}

/* ������ģʽ��������ϵͳʱ����,�رպ��� */
int32 hw_ir_only_open_other_subsys(void)
{
    int32 ret;

    ret = hw_bfgx_close(BFGX_IR);
    ir_only_mode = false;
    return ret;
}

int32 hw_bfgx_input_check(uint32 subsys)
{
    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }
    if (OAL_WARN_ON(is_bfgx_support() != OAL_TRUE)) {
        PS_PRINT_ERR("subsys is [%d], bfgx %s support\n", subsys,
                     (is_bfgx_support() == OAL_TRUE) ? "" : "don't");
        return -ENODEV;
    }

    return 0;
}

int32 hw_bfgx_open(uint32 subsys)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    int32 error;

    ret = hw_bfgx_input_check(subsys);
    if (ret != 0) {
        return ret;
    }

    PS_PRINT_INFO("open %s\n", bfgx_subsys_name_etc[subsys]);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL) ||
                 (ps_core_d->ps_pm->bfg_power_set == NULL) ||
                 (ps_core_d->ps_pm->pm_priv_data == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        PS_PRINT_WARNING("%s has opened! It's Not necessary to send msg to device\n", bfgx_subsys_name_etc[subsys]);
        return BFGX_POWER_SUCCESS;
    }

    if (alloc_seperted_rx_buf_etc(subsys, bfgx_rx_max_frame_etc[subsys], VMALLOC) != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("no mem allocate to read!\n");
        return -ENOMEM;
    }

    /* ��������ģʽ��������ϵͳʱ����Ҫ�رյ��������������ϵͳ�����ϵ� */
    if (ir_only_mode && subsys != BFGX_IR) {
        if (hw_ir_only_open_other_subsys() != BFGX_POWER_SUCCESS) {
            PS_PRINT_ERR("ir only mode,but close ir only mode fail!\n");
            free_seperted_rx_buf_etc(subsys, VMALLOC);
            return -ENOMEM;
        }
    }

    error = ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_UP);
    if (error != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("set %s power on err! error = %d\n", bfgx_subsys_name_etc[subsys], error);
        goto bfgx_power_on_fail;
    }

    if (prepare_to_visit_node_etc(ps_core_d) != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("prepare work FAIL\n");
        error = BFGX_POWER_WAKEUP_FAIL;
        goto bfgx_wakeup_fail;
    }

    if (bfgx_open_cmd_send_etc(subsys) != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("bfgx open cmd fail\n");
        error = BFGX_POWER_OPEN_CMD_FAIL;
        goto bfgx_open_cmd_fail;
    }

    /* ������û�е͹��� */
    if (!ir_only_mode) {
        mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
        ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;
    }

    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_OPEN);
    post_to_visit_node_etc(ps_core_d);

    return BFGX_POWER_SUCCESS;

bfgx_open_cmd_fail:
    post_to_visit_node_etc(ps_core_d);
bfgx_wakeup_fail:
bfgx_power_on_fail:
    free_seperted_rx_buf_etc(subsys, VMALLOC);
    bfgx_open_fail_process_etc(subsys, error);
    return BFGX_POWER_FAILED;
}

int32 hw_bfgx_close(uint32 subsys)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    PS_PRINT_INFO("close %s\n", bfgx_subsys_name_etc[subsys]);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
        PS_PRINT_WARNING("%s has closed! It's Not necessary to send msg to device\n", bfgx_subsys_name_etc[subsys]);
        return BFGX_POWER_SUCCESS;
    }
    wake_up_interruptible(&pst_bfgx_data->rx_wait);

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        /* ����ʧ�ܣ�bfgx closeʱ�Ļ���ʧ�ܲ�����DFR�ָ� */
        PS_PRINT_ERR("prepare work FAIL\n");
    }

    ret = bfgx_close_cmd_send_etc(subsys);
    if (ret < 0) {
        /* ����close����ʧ�ܣ�������DFR�����������µ����̣�DFR�ָ��ӳٵ��´�openʱ��������ҵ������ʱ���� */
        PS_PRINT_ERR("bfgx close cmd fail\n");
    }

    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_SHUTDOWN);
    free_seperted_rx_buf_etc(subsys, VMALLOC);
    ps_kfree_skb_etc(ps_core_d, bfgx_rx_queue_etc[subsys]);

    ps_core_d->rx_pkt_num[subsys] = 0;
    ps_core_d->tx_pkt_num[subsys] = 0;

    if (bfgx_other_subsys_all_shutdown_etc(BFGX_GNSS)) {
        del_timer_sync(&pm_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;
    }

    ret = ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
    if (ret) {
        /* �µ�ʧ�ܣ�������DFR��DFR�ָ��ӳٵ��´�openʱ��������ҵ������ʱ���� */
        PS_PRINT_ERR("set %s power off err!ret = %d", bfgx_subsys_name_etc[subsys], ret);
    }

    post_to_visit_node_etc(ps_core_d);

    return 0;
}

/*
 * Prototype    : hw_bt_open
 * Description  : functions called from above bt hal,when open bt file
 * input        : "/dev/hwbt"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
STATIC int32 hw_bt_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_open(BFGX_BT);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_bt_read
 * Description  : functions called from above bt hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
STATIC ssize_t hw_bt_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue_etc(ps_core_d, RX_BT_QUEUE)) == NULL) {
        PS_PRINT_WARNING("bt read skb queue is null!\n");
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        PS_PRINT_ERR("copy_to_user is err!\n");
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_BT_QUEUE);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_BT_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_bt_write
 * Description  : functions called from above bt hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
STATIC ssize_t hw_bt_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 total_len;
    int32 ret = 0;
    uint8 __user *puser = (uint8 __user *)buf;
    uint8 type = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) || (ps_core_d->ps_pm == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        bt_data_combination_etc.len = 0;
        return -EINVAL;
    }

    if (count > BT_TX_MAX_FRAME) {
        PS_PRINT_ERR("bt skb len is too large!\n");
        bt_data_combination_etc.len = 0;
        return -EINVAL;
    }

    /* ����O��BT���ݷ������·����ȷ��������ͣ����ȹ̶�Ϊ1Byte��Ȼ�����ݣ���Ҫ�������������������device */
    if (count == BT_TYPE_DATA_LEN) {
        get_user(type, puser);
        bt_data_combination_etc.type = type;
        bt_data_combination_etc.len = count;

        return count;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        PS_PRINT_ERR("bt tx high seqlen large than MAXNUM\n");
        bt_data_combination_etc.len = 0;
        return 0;
    }

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work fail, bring to reset work\n");
        bt_data_combination_etc.len = 0;
        plat_exception_handler_etc(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->ps_pm->pm_priv_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    total_len = count + bt_data_combination_etc.len + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);

    skb = ps_alloc_skb_etc(total_len);
    if (skb == NULL) {
        PS_PRINT_ERR("ps alloc skb mem fail\n");
        post_to_visit_node_etc(ps_core_d);
        bt_data_combination_etc.len = 0;
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head) + bt_data_combination_etc.len], buf, count)) {
        PS_PRINT_ERR("copy_from_user from bt is err\n");
        kfree_skb(skb);
        post_to_visit_node_etc(ps_core_d);
        bt_data_combination_etc.len = 0;
        return -EFAULT;
    }

    if (bt_data_combination_etc.len == BT_TYPE_DATA_LEN) {
        skb->data[sizeof(struct ps_packet_head)] = bt_data_combination_etc.type;
    }

    bt_data_combination_etc.len = 0;

    ps_add_packet_head_etc(skb->data, BT_MSG, total_len);
    ps_skb_enqueue_etc(ps_core_d, skb, TX_HIGH_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    ps_core_d->tx_pkt_num[BFGX_BT]++;

    post_to_visit_node_etc(ps_core_d);

    return count;
}

/*
 * Prototype    : hw_bt_poll
 * Description  : called by bt func from hal;
 *                check whether or not allow read and write
 */
STATIC uint32 hw_bt_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32 mask = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_BT].rx_wait, wait);

    if (ps_core_d->bfgx_info[BFGX_BT].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_bt_ioctl
 * Description  : called by bt func from hal; default not use
 */
STATIC int64 hw_bt_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    PS_PRINT_FUNCTION_NAME;

    return 0;
}

/*
 * Prototype    : hw_bt_release
 * Description  : called by bt func from hal when close bt inode
 * input        : "/dev/hwbt"
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_bt_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_BT);

    oal_wake_unlock_force(&pm_data->bt_wake_lock);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

STATIC int32 hw_nfc_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_open(BFGX_NFC);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

STATIC ssize_t hw_nfc_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue_etc(ps_core_d, RX_NFC_QUEUE)) == NULL) {
        PS_PRINT_WARNING("nfc read skb queue is null!");
        return 0;
    }

    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        PS_PRINT_ERR("copy_to_user is err!\n");
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_NFC_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);
    if (skb->len == 0) {
        kfree_skb(skb);
    } else {
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_NFC_QUEUE);
    }

    return count1;
}

STATIC ssize_t hw_nfc_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);

    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) || (ps_core_d->ps_pm == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (count > NFC_TX_MAX_FRAME) {
        PS_PRINT_ERR("bt skb len is too large!\n");
        return -EINVAL;
    }

    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work fail, bring to reset work\n");
        plat_exception_handler_etc(SUBSYS_BFGX, THREAD_NFC, BFGX_WAKEUP_FAIL);
        return ret;
    }
    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    /* to divide up packet function and tx to tty work */
    if (ps_tx_nfcbuf_etc(ps_core_d, buf, count) < 0) {
        PS_PRINT_ERR("hw_nfc_write is err\n");
        post_to_visit_node_etc(ps_core_d);
        return -EFAULT;
    }

    ps_core_d->tx_pkt_num[BFGX_NFC]++;

    post_to_visit_node_etc(ps_core_d);

    PS_PRINT_DBG("NFC data write end\n");

    return count;
}

/*
 * Prototype    : hw_nfc_poll
 * Description  : called by nfc func from hal;
 *                check whether or not allow read and write
 */
STATIC uint32 hw_nfc_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32 mask = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_NFC].rx_wait, wait);

    if (ps_core_d->bfgx_info[BFGX_NFC].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

STATIC int32 hw_nfc_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_NFC);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_ir_open
 * Description  : open ir device
 */
STATIC int32 hw_ir_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    /* judge ir only mode */
    if ((wlan_is_shutdown_etc() == true) && (bfgx_is_shutdown_etc() == true)
        && (get_hi1103_asic_type() == HI1103_ASIC_PILOT)) {
        ir_only_mode = true;
    }

    ret = hw_bfgx_open(BFGX_IR);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_ir_read
 * Description  : read ir node data
 */
STATIC ssize_t hw_ir_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    uint16 ret_count;
    struct sk_buff *skb = NULL;
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue_etc(ps_core_d, RX_IR_QUEUE)) == NULL) {
        PS_PRINT_DBG("ir read skb queue is null!\n");
        return 0;
    }

    ret_count = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, ret_count)) {
        PS_PRINT_ERR("copy_to_user is err!\n");
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_IR_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, ret_count);

    if (skb->len == 0) {
        kfree_skb(skb);
    } else {
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_IR_QUEUE);
    }

    return ret_count;
}

/*
 * Prototype    : hw_ir_write
 * Description  : write data to ir node
 */
STATIC ssize_t hw_ir_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    int32 ret = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);

    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) || (ps_core_d->ps_pm == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[BFGX_IR];
    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
        PS_PRINT_WARNING("IR has closed! It's Not necessary to send msg to device\n");
        return 0;
    }

    if (count > IR_TX_MAX_FRAME) {
        PS_PRINT_ERR("IR skb len is too large!\n");
        return -EINVAL;
    }

    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work fail, bring to reset work\n");
        plat_exception_handler_etc(SUBSYS_BFGX, THREAD_IR, BFGX_WAKEUP_FAIL);
        return ret;
    }

    if (!ir_only_mode) {
        /* modify expire time of uart idle timer */
        mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
        ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;
    }

    /* to divide up packet function and tx to tty work */
    if (ps_tx_irbuf_etc(ps_core_d, buf, count) < 0) {
        PS_PRINT_ERR("hw_ir_write is err\n");
        post_to_visit_node_etc(ps_core_d);
        return -EFAULT;
    }

    ps_core_d->tx_pkt_num[BFGX_IR]++;

    post_to_visit_node_etc(ps_core_d);

    PS_PRINT_DBG("IR data write end\n");

    return count;
}

STATIC int32 hw_ir_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_IR);
    ir_only_mode = false;

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_fm_open
 * Description  : functions called from above fm hal,when open fm file
 * input        : "/dev/hwfm"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
STATIC int32 hw_fm_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_open(BFGX_FM);

    ps_core_d->fm_read_delay = FM_READ_DEFAULT_TIME;

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_fm_read
 * Description  : functions called from above fm hal,read count data to buf
 */
STATIC ssize_t hw_fm_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1;
    int64 timeout;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (ps_core_d->bfgx_info[BFGX_FM].rx_queue.qlen == 0) { /* if don,t data, and wait timeout function */
        if (filp->f_flags & O_NONBLOCK) {                   /* if O_NONBLOCK read and return */
            return -EAGAIN;
        }
        /* timeout function;when have data,can interrupt */
        timeout = wait_event_interruptible_timeout(ps_core_d->bfgx_info[BFGX_FM].rx_wait,
                                                   (ps_core_d->bfgx_info[BFGX_FM].rx_queue.qlen > 0),
                                                   msecs_to_jiffies(ps_core_d->fm_read_delay));
        if (!timeout) {
            PS_PRINT_DBG("fm read time out!\n");
            return -ETIMEDOUT;
        }
    }

    if ((skb = ps_skb_dequeue_etc(ps_core_d, RX_FM_QUEUE)) == NULL) {
        PS_PRINT_WARNING("fm read no data!\n");
        return -ETIMEDOUT;
    }

    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        PS_PRINT_ERR("copy_to_user is err!\n");
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_FM_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);

    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_FM_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_fm_write
 * Description  : functions called from above fm hal,write count data to buf
 */
STATIC ssize_t hw_fm_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) || (ps_core_d->ps_pm == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* if count is too large;and don,t tx */
    if (count > (FM_TX_MAX_FRAME - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end))) {
        PS_PRINT_ERR("err:fm packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work fail, bring to reset work\n");
        plat_exception_handler_etc(SUBSYS_BFGX, THREAD_FM, BFGX_WAKEUP_FAIL);
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    /* to divide up packet function and tx to tty work */
    if (ps_tx_fmbuf_etc(ps_core_d, buf, count) < 0) {
        PS_PRINT_ERR("hw_fm_write is err\n");
        post_to_visit_node_etc(ps_core_d);
        return -EFAULT;
    }
    ps_core_d->tx_pkt_num[BFGX_FM]++;

    post_to_visit_node_etc(ps_core_d);

    PS_PRINT_DBG("FM data write end\n");

    return count;
}

/*
 * Prototype    : hw_fm_ioctl
 * Description  : called by hw func from hal when open power gpio or close power gpio
 */
STATIC int64 hw_fm_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (file == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (cmd == FM_SET_READ_TIME) {
        if (arg < FM_MAX_READ_TIME) { /* set timeout for fm read function */
            ps_core_d->fm_read_delay = arg;
        } else {
            PS_PRINT_ERR("arg is too large!\n");
            return -EINVAL;
        }
    }

    return 0;
}

/*
 * Prototype    : hw_fm_release
 * Description  : called by fm func from hal when close fm inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_fm_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_FM);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/* device bfgx pm debug switch on/off */
void plat_pm_debug_switch(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is null\n");
        return;
    }

    PS_PRINT_INFO("%s", __func__);

    ps_get_core_reference_etc(&ps_core_d);

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return;
    }

    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, PL_PM_DEBUG);

    post_to_visit_node_etc(ps_core_d);

    return;
}

/*
 * Prototype    : hw_gnss_open
 * Description  : functions called from above gnss hal,when open gnss file
 * input        : "/dev/hwgnss"
 * output       : return 0 --> open is ok
 *                return !0--> open is false
 */
STATIC int32 hw_gnss_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);

    ret = hw_bfgx_open(BFGX_GNSS);

    ps_core_d->gnss_read_delay = GNSS_READ_DEFAULT_TIME;

    if (ret != BFGX_POWER_SUCCESS) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    }

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_gnss_poll
 * Description  : called by gnss func from hal;
 *                check whether or not allow read and write
 */
STATIC uint32 hw_gnss_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32 mask = 0;

    PS_PRINT_DBG("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_GNSS].rx_wait, wait);

    PS_PRINT_DBG("%s, recive gnss data\n", __func__);

    if (ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_gnss_read
 * Description  : functions called from above gnss hal,read count data to buf
 */
STATIC ssize_t hw_gnss_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    struct sk_buff_head read_queue;
    int32 count1;
    uint8 seperate_tag = GNSS_SEPER_TAG_INIT;
    int32 copy_cnt = 0;
    uint32 ret;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb_queue_head_init(&read_queue);

    spin_lock(&ps_core_d->gnss_rx_lock);
    do {
        if ((skb = ps_skb_dequeue_etc(ps_core_d, RX_GNSS_QUEUE)) == NULL) {
            spin_unlock(&ps_core_d->gnss_rx_lock);
            if (read_queue.qlen != 0) {
                // û���ҵ�last����skb queue�Ϳ���
                PS_PRINT_ERR("skb dequeue error, qlen=%x!\n", read_queue.qlen);
                goto skb_dequeue_error;
            } else {
                PS_PRINT_INFO("gnss read no data!\n");
                return 0;
            }
        }

        seperate_tag = skb->data[skb->len - 1];
        switch (seperate_tag) {
            case GNSS_SEPER_TAG_INIT:
            case GNSS_SEPER_TAG_LAST:
                break;
            default:
                PS_PRINT_ERR("seperate_tag=%x not support\n", seperate_tag);
                seperate_tag = GNSS_SEPER_TAG_LAST;
                break;
        }

        skb_queue_tail(&read_queue, skb);
    } while (seperate_tag == GNSS_SEPER_TAG_INIT);
    spin_unlock(&ps_core_d->gnss_rx_lock);

    copy_cnt = 0;
    do {
        skb = skb_dequeue(&read_queue);
        if (skb == NULL) {
            PS_PRINT_ERR("copy dequeue error, copy_cnt=%x\n", copy_cnt);
            goto skb_dequeue_error;
        }

        if (skb->len <= 1) {
            PS_PRINT_ERR("skb len error,skb->len=%x,copy_cnt=%x,count=%x\n", skb->len, copy_cnt, (uint32)count);
            goto copy_error;
        }

        count1 = skb->len - 1;
        if (count1 + copy_cnt > count) {
            PS_PRINT_ERR("copy total len error,skb->len=%x,tag=%x,copy_cnt=%x,read_cnt=%x\n",
                         skb->len, skb->data[skb->len - 1], copy_cnt, (uint32)count);
            goto copy_error;
        }

        ret = copy_to_user(buf + copy_cnt, skb->data, count1);
        if (ret != 0) {
            PS_PRINT_ERR("copy_to_user err,ret=%x,dest=%p,src=%p,tag:%x,count1=%x,copy_cnt=%x,read_cnt=%x\n",
                         ret, buf + copy_cnt, skb->data, skb->data[skb->len - 1], count1, copy_cnt, (uint32)count);
            goto copy_error;
        }

        copy_cnt += count1;
        kfree_skb(skb);
    } while (read_queue.qlen != 0);

    return copy_cnt;

copy_error:
    kfree_skb(skb);
skb_dequeue_error:
    while ((skb = skb_dequeue(&read_queue)) != NULL) {
        PS_PRINT_ERR("free skb: len=%x, tag=%x\n", skb->len, skb->data[skb->len - 1]);
        kfree_skb(skb);
    }

    return -EFAULT;
}

/*
 * Prototype    : hw_gnss_write
 * Description  : functions called from above gnss hal,write count data to buf
 */
STATIC ssize_t hw_gnss_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret = 0;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) ||
                 (ps_core_d->ps_pm == NULL) || (pm_data == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (count > GNSS_TX_MAX_FRAME) {
        PS_PRINT_ERR("err:gnss packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        PS_PRINT_ERR("prepare work fail, bring to reset work\n");
        plat_exception_handler_etc(SUBSYS_BFGX, THREAD_GNSS, BFGX_WAKEUP_FAIL);
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->ps_pm->pm_priv_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* to divide up packet function and tx to tty work */
    if (ps_tx_gnssbuf_etc(ps_core_d, buf, count) < 0) {
        PS_PRINT_ERR("hw_gnss_write is err\n");
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        count = -EFAULT;
    }

    ps_core_d->tx_pkt_num[BFGX_GNSS]++;

    post_to_visit_node_etc(ps_core_d);

    return count;
}

int32 plat_gnss_refclk_para_set_etc(uint64 arg)
{
    gps_refclk_param para = { 0 };
    uint32 __user *puser = (uint32 __user *)(uintptr_t)arg;
    if (copy_from_user(&para, puser, sizeof(gps_refclk_param))) {
        PS_PRINT_ERR("get gnss ref clk params error\n");
        return -EINVAL;
    }
    return set_gps_ref_clk_enable_hi110x_etc(para.enable, para.modem_id, para.rat);
}

/*
 * Prototype    : hw_gnss_ioctl
 * Description  : called by gnss func from hal when open power gpio or close power gpio
 */
STATIC int64 hw_gnss_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (file == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    switch (cmd) {
        case GNSS_SET_READ_TIME:
            if (arg < GNSS_MAX_READ_TIME) { /* set timeout for gnss read function */
                ps_core_d->gnss_read_delay = arg;
            } else {
                PS_PRINT_ERR("arg is too large!\n");
                return -EINVAL;
            }
            break;
        case PLAT_GNSS_REFCLK_PARA_CMD:
            return plat_gnss_refclk_para_set_etc(arg);
        default:
            PS_PRINT_WARNING("hw_gnss_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}

/*
 * Prototype    : hw_gnss_release
 * Description  : called by gnss func from hal when close gnss inode
 */
STATIC int32 hw_gnss_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_GNSS);

    oal_wake_unlock_force(&pm_data->gnss_wake_lock);

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

#ifndef HI110X_HAL_MEMDUMP_ENABLE
/*
 * Prototype    : plat_exception_dump_file_rotate_init_etc
 * Description  : driver init rotate resource
 */
void plat_exception_dump_file_rotate_init_etc(void)
{
    init_waitqueue_head(&dump_cmd_queue_etc.dump_type_wait);
    skb_queue_head_init(&dump_cmd_queue_etc.dump_type_queue);
    atomic_set(&dump_cmd_queue_etc.rotate_finish_state, ROTATE_FINISH);
    PS_PRINT_INFO("plat exception dump file rotate init success\n");
}

/*
 * Prototype    : plat_rotate_finish_set_etc
 * Description  : app set rotate state when rotate finish
 */
void plat_rotate_finish_set_etc(void)
{
    atomic_set(&dump_cmd_queue_etc.rotate_finish_state, ROTATE_FINISH);
}

/*
 * Prototype    : plat_wait_last_rotate_finish_etc
 * Description  : wait former app rotate finish
 */
void plat_wait_last_rotate_finish_etc(void)
{
    uint8 retry = 0;

#define RETRY_TIME 3

    while (atomic_read(&dump_cmd_queue_etc.rotate_finish_state) != ROTATE_FINISH) {
        /* maximum app rotate time is 0.1s */
        oal_udelay(100);
        if ((++retry) >= RETRY_TIME) {
            PS_PRINT_WARNING("retry wait last rotate fail:retry =%d", retry);
            break;
        }
    }

    atomic_set(&dump_cmd_queue_etc.rotate_finish_state, ROTATE_NOT_FINISH);
}

/*
 * Prototype    : plat_send_rotate_cmd_2_app_etc
 * Description  : driver send rotate cmd to app for rotate file
 */
int32 plat_send_rotate_cmd_2_app_etc(uint32 which_dump)
{
    struct sk_buff *skb = NULL;

    if (which_dump >= CMD_DUMP_BUFF) {
        PS_PRINT_WARNING("which dump:%d error\n", which_dump);
        return -EINVAL;
    }
    if (skb_queue_len(&dump_cmd_queue_etc.dump_type_queue) > MEMDUMP_ROTATE_QUEUE_MAX_LEN) {
        PS_PRINT_WARNING("too many dump type in queue,dispose type:%d", which_dump);
        return -EINVAL;
    }

    skb = alloc_skb(sizeof(which_dump), GFP_KERNEL);
    if (skb == NULL) {
        PS_PRINT_ERR("alloc errno skbuff failed! len=%d, errno=%x\n", (int32)sizeof(which_dump), which_dump);
        return -EINVAL;
    }
    skb_put(skb, sizeof(which_dump));
    *(uint32 *)skb->data = which_dump;
    skb_queue_tail(&dump_cmd_queue_etc.dump_type_queue, skb);
    PS_PRINT_INFO("save rotate cmd [%d] in queue\n", which_dump);

    wake_up_interruptible(&dump_cmd_queue_etc.dump_type_wait);

    return 0;
}

/*
 * Prototype    : plat_dump_rotate_cmd_read_etc
 * Description  : app read rotate cmd
 */
int32 plat_dump_rotate_cmd_read_etc(uint64 arg)
{
    uint32 __user *puser = (uint32 __user *)(uintptr_t)arg;
    struct sk_buff *skb = NULL;

    if (!access_ok(VERIFY_WRITE, (uintptr_t)puser, (int32)sizeof(uint32))) {
        PS_PRINT_ERR("address can not write\n");
        return -EINVAL;
    }

    if (wait_event_interruptible(dump_cmd_queue_etc.dump_type_wait,
                                 (skb_queue_len(&dump_cmd_queue_etc.dump_type_queue)) > 0)) {
        return -EINVAL;
    }

    skb = skb_dequeue(&dump_cmd_queue_etc.dump_type_queue);
    if (skb == NULL) {
        PS_PRINT_WARNING("skb is NULL\n");
        return -EINVAL;
    }

    if (copy_to_user(puser, skb->data, sizeof(uint32))) {
        PS_PRINT_WARNING("copy_to_user err!restore it, len=%d\n", (int32)sizeof(uint32));
        skb_queue_head(&dump_cmd_queue_etc.dump_type_queue, skb);
        return -EINVAL;
    }

    PS_PRINT_INFO("read rotate cmd [%d] from queue\n", *(uint32 *)skb->data);

    skb_pull(skb, skb->len);
    kfree_skb(skb);

    return 0;
}

/*
 * Prototype    : hw_debug_ioctl
 * Description  : called by ini_plat_dfr_set
 */
STATIC int64 hw_debug_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    if (file == NULL) {
        PS_PRINT_ERR("file is null\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PLAT_DFR_CFG_CMD:
            plat_dfr_cfg_set_etc(arg);
            break;
        case PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD:
            plat_beatTimer_timeOut_reset_cfg_set_etc(arg);
            break;
        case PLAT_BFGX_CALI_CMD:
            if (is_bfgx_support() == OAL_TRUE) {
                bfgx_cali_data_init();
            }
            break;
        case PLAT_DUMP_FILE_READ_CMD:
            plat_dump_rotate_cmd_read_etc(arg);
            break;
        case PLAT_DUMP_ROTATE_FINISH_CMD:
            plat_rotate_finish_set_etc();
            break;
        default:
            PS_PRINT_WARNING("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}
#else
int32 plat_excp_dump_rotate_cmd_read_etc(uint64 arg, memdump_info_t *memdump_info)
{
    uint32 __user *puser = (uint32 __user *)(uintptr_t)arg;
    struct sk_buff *skb = NULL;

    if (!access_ok(VERIFY_WRITE, (uintptr_t)puser, (int32)sizeof(uint32))) {
        PS_PRINT_ERR("address can not write\n");
        return -EINVAL;
    }

    if (wait_event_interruptible(memdump_info->dump_type_wait, (skb_queue_len(&memdump_info->dump_type_queue)) > 0)) {
        return -EINVAL;
    }

    skb = skb_dequeue(&memdump_info->dump_type_queue);
    if (skb == NULL) {
        PS_PRINT_WARNING("skb is NULL\n");
        return -EINVAL;
    }

    if (copy_to_user(puser, skb->data, sizeof(uint32))) {
        PS_PRINT_WARNING("copy_to_user err!restore it, len=%d,arg=%ld\n", (int32)sizeof(uint32), arg);
        skb_queue_head(&memdump_info->dump_type_queue, skb);
        return -EINVAL;
    }

    PS_PRINT_INFO("read rotate cmd [%d] from queue\n", *(uint32 *)skb->data);

    skb_pull(skb, skb->len);
    kfree_skb(skb);

    return 0;
}

int32 plat_bfgx_dump_rotate_cmd_read_etc(uint64 arg)
{
    return plat_excp_dump_rotate_cmd_read_etc(arg, &bcpu_memdump_cfg_etc);
}

int32 plat_wifi_dump_rotate_cmd_read_etc(uint64 arg)
{
    return plat_excp_dump_rotate_cmd_read_etc(arg, &wcpu_memdump_cfg_etc);
}

/*
 * Prototype    : hw_debug_ioctl
 * Description  : called by ini_plat_dfr_set
 */

STATIC int64 hw_debug_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    if (file == NULL) {
        PS_PRINT_ERR("file is null\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PLAT_DFR_CFG_CMD:
            plat_dfr_cfg_set_etc(arg);
            break;
        case PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD:
            plat_beatTimer_timeOut_reset_cfg_set_etc(arg);
            break;
        case PLAT_BFGX_CALI_CMD:
            bfgx_cali_data_init();
            break;
        default:
            PS_PRINT_WARNING("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}
#endif
int32 arm_timeout_submit(enum BFGX_THREAD_ENUM subs)
{
#define DFR_SUBMIT_LIMIT_TIME 300 /* second */
    static unsigned long long dfr_submit_last_time = 0;
    unsigned long long dfr_submit_current_time;
    unsigned long long dfr_submit_interval_time;
    struct timespec dfr_submit_time;

    if (subs >= BFGX_THREAD_BOTTOM) {
        return -EINVAL;
    }

    PS_PRINT_INFO("[subs id:%d]arm timeout trigger", subs);

    dfr_submit_time = current_kernel_time();
    dfr_submit_current_time = dfr_submit_time.tv_sec;
    dfr_submit_interval_time = dfr_submit_current_time - dfr_submit_last_time;

    /* 5��������ഥ��һ�� */
    if ((dfr_submit_interval_time > DFR_SUBMIT_LIMIT_TIME) || (dfr_submit_last_time == 0)) {
        dfr_submit_last_time = dfr_submit_current_time;
        plat_exception_handler_etc(SUBSYS_BFGX, subs, BFGX_ARP_TIMEOUT);
        return 0;
    } else {
        PS_PRINT_ERR("[subs id:%d]arm timeout cnt max than limit", subs);
        return -EAGAIN;
    }
}

#ifdef HI110X_HAL_MEMDUMP_ENABLE
STATIC int32 hw_excp_read(struct file *filp, int8 __user *buf,
                          size_t count, loff_t *f_pos, memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;
    uint16 count1;

    if (unlikely((buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }
    if ((skb = skb_dequeue(&memdump_t->quenue)) == NULL) {
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        PS_PRINT_ERR("copy_to_user is err!\n");
        skb_queue_head(&memdump_t->quenue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&memdump_t->quenue, skb);
    }

    return count1;
}

STATIC ssize_t hw_bfgexcp_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    return hw_excp_read(filp, buf, count, f_pos, &bcpu_memdump_cfg_etc);
}

STATIC int64 hw_bfgexcp_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    int32 ret = 0;
    if (file == NULL) {
        PS_PRINT_ERR("file is null\n");
        return -EINVAL;
    }
    switch (cmd) {
        case PLAT_BFGX_DUMP_FILE_READ_CMD:
            ret = plat_bfgx_dump_rotate_cmd_read_etc(arg);
            break;
        case DFR_HAL_GNSS_CFG_CMD:
            return arm_timeout_submit(THREAD_GNSS);
        case DFR_HAL_BT_CFG_CMD:
            return arm_timeout_submit(THREAD_BT);
        case DFR_HAL_FM_CFG_CMD:
            return arm_timeout_submit(THREAD_FM);
        default:
            PS_PRINT_WARNING("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return ret;
}

STATIC int64 hw_wifiexcp_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    int32 ret = 0;

    if (file == NULL) {
        PS_PRINT_ERR("file is null\n");
        return -EINVAL;
    }
    switch (cmd) {
        case PLAT_WIFI_DUMP_FILE_READ_CMD:
            ret = plat_wifi_dump_rotate_cmd_read_etc(arg);
            break;
        default:
            PS_PRINT_WARNING("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return ret;
}

STATIC ssize_t hw_wifiexcp_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    return hw_excp_read(filp, buf, count, f_pos, &wcpu_memdump_cfg_etc);
}
#endif

/*
 * Prototype    : hw_debug_open
 * Description  : functions called from above oam hal,when open debug file
 * input        : "/dev/hwbfgdbg"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
STATIC int32 hw_debug_open(struct inode *inode, struct file *filp)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_INFO("%s", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    debug_cnt++;
    PS_PRINT_INFO("%s debug_cnt=%d\n", __func__, debug_cnt);
    atomic_set(&ps_core_d->dbg_func_has_open, 1);

    ps_core_d->dbg_read_delay = DBG_READ_DEFAULT_TIME;

    return 0;
}

STATIC ssize_t hw_debug_read(struct file *filp, int8 __user *buf,
                             size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1 = 0;
    int64 timeout;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (ps_core_d->rx_dbg_seq.qlen == 0) { /* if no data, and wait timeout function */
        if (filp->f_flags & O_NONBLOCK) {  /* if O_NONBLOCK read and return */
            return -EAGAIN;
        }

        /* timeout function;when have data,can interrupt */
        timeout = wait_event_interruptible_timeout(ps_core_d->rx_dbg_wait,
                                                   (ps_core_d->rx_dbg_seq.qlen > 0),
                                                   msecs_to_jiffies(ps_core_d->dbg_read_delay));
        if (!timeout) {
            PS_PRINT_DBG("debug read time out!\n");
            return -ETIMEDOUT;
        }
    }

    /* pull skb data from skb queue */
    if ((skb = ps_skb_dequeue_etc(ps_core_d, RX_DBG_QUEUE)) == NULL) {
        PS_PRINT_DBG("dbg read no data!\n");
        return -ETIMEDOUT;
    }
    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        PS_PRINT_ERR("debug copy_to_user is err!\n");
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_DBG_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);

    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue_etc(ps_core_d, skb, RX_DBG_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_debug_write
 * Description  : functions called from above oam hal,write count data to buf
 */
#ifdef PLATFORM_DEBUG_ENABLE
STATIC ssize_t hw_debug_write(struct file *filp, const int8 __user *buf,
                              size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb;
    uint16 total_len;
    int32 ret = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (ps_core_d->tty_have_open == false) {
        PS_PRINT_ERR("err: uart not opened!\n");
        return -EFAULT;
    }

    if (count > (DBG_TX_MAX_FRAME - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end))) {
        PS_PRINT_ERR("err: dbg packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    if (ps_chk_bfg_active_etc(ps_core_d) == false) {
        PS_PRINT_ERR("bfg is closed, /dev/hwdebug cant't write!!!\n");
        return -EINVAL;
    }

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret < 0) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return ret;
    }
    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    total_len = count + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);

    skb = ps_alloc_skb_etc(total_len);
    if (skb == NULL) {
        PS_PRINT_ERR("ps alloc skb mem fail\n");
        post_to_visit_node_etc(ps_core_d);
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, count)) {
        PS_PRINT_ERR("copy_from_user from dbg is err\n");
        kfree_skb(skb);
        post_to_visit_node_etc(ps_core_d);
        return -EFAULT;
    }

    ps_add_packet_head_etc(skb->data, OML_MSG, total_len);
    ps_skb_enqueue_etc(ps_core_d, skb, TX_LOW_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    post_to_visit_node_etc(ps_core_d);

    return count;
}
#endif

/*
 * Prototype    : hw_debug_release
 * Description  : called by oam func from hal when close debug inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_debug_release(struct inode *inode, struct file *filp)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_INFO("%s", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    debug_cnt--;
    PS_PRINT_INFO("%s debug_cnt=%d", __func__, debug_cnt);
    if (debug_cnt == 0) {
        /* wake up bt dbg wait queue */
        wake_up_interruptible(&ps_core_d->rx_dbg_wait);
        atomic_set(&ps_core_d->dbg_func_has_open, 0);

        /* kfree have rx dbg skb */
        ps_kfree_skb_etc(ps_core_d, RX_DBG_QUEUE);
    }

    return 0;
}
#ifndef HI110X_HAL_MEMDUMP_ENABLE
STATIC int32 hw_excp_open(struct inode *inode, struct file *filp)
{
    PS_PRINT_INFO("%s", __func__);
    return 0;
}
STATIC int32 hw_excp_release(struct inode *inode, struct file *filp)
{
    PS_PRINT_INFO("%s", __func__);
    return 0;
}
STATIC int64 hw_excp_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    struct st_exception_info *pst_exception_data = NULL;
    if (file == NULL) {
        PS_PRINT_ERR("file is null\n");
        return -EINVAL;
    }
    PS_PRINT_INFO("%s", __func__);

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EINVAL;
    }

    switch (cmd) {
        case DFR_HAL_GNSS_CFG_CMD:
            return arm_timeout_submit(THREAD_GNSS);
            break;
        case DFR_HAL_BT_CFG_CMD:
            return arm_timeout_submit(THREAD_BT);
            break;
        case DFR_HAL_FM_CFG_CMD:
            return arm_timeout_submit(THREAD_FM);
            break;
        default:
            PS_PRINT_WARNING("hw_excp_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}
#endif
uart_loop_cfg uart_loop_test_cfg_etc = { 256, 60000, 0, 0, 0 };
uart_loop_test_struct *uart_loop_test_info_etc = NULL;

void uart_loop_test_tx_buf_init(uint8 *puc_data, uint16 us_data_len)
{
    get_random_bytes(puc_data, us_data_len);
}

int32 uart_loop_set_pkt_count_etc(uint32 count)
{
    PS_PRINT_INFO("uart loop test, set pkt count to [%d]\n", count);
    uart_loop_test_cfg_etc.loop_count = count;

    return 0;
}

int32 uart_loop_set_pkt_len_etc(uint32 pkt_len)
{
    PS_PRINT_INFO("uart loop test, set pkt len to [%d]\n", pkt_len);
    uart_loop_test_cfg_etc.pkt_len = pkt_len;

    return 0;
}

int32 alloc_uart_loop_test_etc(void)
{
    uint8 *uart_loop_tx_buf = NULL;
    uint8 *uart_loop_rx_buf = NULL;
    uint16 pkt_len = 0;

    if (uart_loop_test_info_etc == NULL) {
        uart_loop_test_info_etc = (uart_loop_test_struct *)kzalloc(sizeof(uart_loop_test_struct), GFP_KERNEL);
        if (uart_loop_test_info_etc == NULL) {
            PS_PRINT_ERR("malloc uart_loop_test_info_etc fail\n");
            goto malloc_test_info_fail;
        }

        pkt_len = uart_loop_test_cfg_etc.pkt_len;
        if (pkt_len == 0 || pkt_len > UART_LOOP_MAX_PKT_LEN) {
            pkt_len = UART_LOOP_MAX_PKT_LEN;
            uart_loop_test_cfg_etc.pkt_len = UART_LOOP_MAX_PKT_LEN;
        }

        uart_loop_tx_buf = (uint8 *)kzalloc(pkt_len, GFP_KERNEL);
        if (uart_loop_tx_buf == NULL) {
            PS_PRINT_ERR("malloc uart_loop_tx_buf fail\n");
            goto malloc_tx_buf_fail;
        }

        memset_s(uart_loop_tx_buf, pkt_len, 0xa5, pkt_len);

        uart_loop_rx_buf = (uint8 *)kzalloc(pkt_len, GFP_KERNEL);
        if (uart_loop_rx_buf == NULL) {
            PS_PRINT_ERR("malloc uart_loop_rx_buf fail\n");
            goto malloc_rx_buf_fail;
        }

        uart_loop_test_cfg_etc.uart_loop_enable = 1;
        uart_loop_test_cfg_etc.uart_loop_tx_random_enable = 1;

        init_completion(&uart_loop_test_info_etc->set_done);
        init_completion(&uart_loop_test_info_etc->loop_test_done);

        uart_loop_test_info_etc->test_cfg = &uart_loop_test_cfg_etc;
        uart_loop_test_info_etc->tx_buf = uart_loop_tx_buf;
        uart_loop_test_info_etc->rx_buf = uart_loop_rx_buf;
        uart_loop_test_info_etc->rx_pkt_len = 0;

        PS_PRINT_INFO("uart loop test, pkt len is [%d]\n", pkt_len);
        PS_PRINT_INFO("uart loop test, loop count is [%d]\n", uart_loop_test_cfg_etc.loop_count);
    }

    return 0;

malloc_rx_buf_fail:
    kfree(uart_loop_tx_buf);
malloc_tx_buf_fail:
    kfree(uart_loop_test_info_etc);
    uart_loop_test_info_etc = NULL;
malloc_test_info_fail:
    return -ENOMEM;
}

void free_uart_loop_test_etc(void)
{
    if (uart_loop_test_info_etc == NULL) {
        return;
    }
    PS_PRINT_ERR("free uart loop test buf\n");
    uart_loop_test_cfg_etc.uart_loop_enable = 0;
    kfree(uart_loop_test_info_etc->rx_buf);
    kfree(uart_loop_test_info_etc->tx_buf);
    kfree(uart_loop_test_info_etc);
    uart_loop_test_info_etc = NULL;

    return;
}

int32 uart_loop_test_open_etc(void)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 error;

    PS_PRINT_INFO("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL) || (ps_core_d->ps_pm->bfg_power_set == NULL))) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    if (ps_chk_bfg_active_etc(ps_core_d)) {
        PS_PRINT_ERR("bfgx subsys must all close\n");
        return -EINVAL;
    }

    if (alloc_uart_loop_test_etc() != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("alloc mem for uart loop test fail!\n");
        goto alloc_mem_fail;
    }

    error = ps_core_d->ps_pm->bfg_power_set(BFGX_GNSS, BFG_POWER_GPIO_UP);
    if (error != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("uart loop test, power on err! error = %d\n", error);
        goto power_on_fail;
    }

    if (prepare_to_visit_node_etc(ps_core_d) != BFGX_POWER_SUCCESS) {
        PS_PRINT_ERR("uart loop test, prepare work fail\n");
        error = BFGX_POWER_WAKEUP_FAIL;
        goto wakeup_fail;
    }

    post_to_visit_node_etc(ps_core_d);

    return BFGX_POWER_SUCCESS;

wakeup_fail:
    ps_core_d->ps_pm->bfg_power_set(BFGX_GNSS, BFG_POWER_GPIO_DOWN);
power_on_fail:
    free_uart_loop_test_etc();
alloc_mem_fail:
    return BFGX_POWER_FAILED;
}

int32 uart_loop_test_close_etc(void)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_INFO("%s", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (prepare_to_visit_node_etc(ps_core_d) != 0) {
        PS_PRINT_ERR("uart loop test, prepare work fail\n");
    }

    if (ps_core_d->ps_pm->bfg_power_set(BFGX_GNSS, BFG_POWER_GPIO_DOWN) != 0) {
        PS_PRINT_ERR("uart loop test, power off err!");
    }

    free_uart_loop_test_etc();

    post_to_visit_node_etc(ps_core_d);

    return 0;
}

int32 uart_loop_test_set_etc(uint8 flag)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    uint8 cmd;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return -EINVAL;
    }

    if (flag == UART_LOOP_SET_DEVICE_DATA_HANDLER) {
        cmd = SYS_CFG_SET_UART_LOOP_HANDLER;
    } else {
        cmd = SYS_CFG_SET_UART_LOOP_FINISH;
    }

    INIT_COMPLETION(uart_loop_test_info_etc->set_done);
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, cmd);
    timeleft = wait_for_completion_timeout(&uart_loop_test_info_etc->set_done, msecs_to_jiffies(5000));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait set uart loop ack timeout\n");
        return -ETIMEDOUT;
    }

    return 0;
}

int32 uart_loop_test_send_data_etc(struct ps_core_s *ps_core_d, uint8 *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    uint16 tx_gnss_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    while (count > 0) {
        if (count > GNSS_TX_PACKET_LIMIT) {
            tx_gnss_len = GNSS_TX_PACKET_LIMIT;
        } else {
            tx_gnss_len = count;
        }
        /* curr tx skb total lenth */
        tx_skb_len = tx_gnss_len + sizeof(struct ps_packet_head);
        tx_skb_len = tx_skb_len + sizeof(struct ps_packet_end);

        skb = ps_alloc_skb_etc(tx_skb_len);
        if (skb == NULL) {
            PS_PRINT_ERR("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > GNSS_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start gnss packet */
                ps_add_packet_head_etc(skb->data, GNSS_First_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head_etc(skb->data, GNSS_Common_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head_etc(skb->data, GNSS_Last_MSG, tx_skb_len);
        }

        if (memcpy_s(&skb->data[sizeof(struct ps_packet_head)], tx_skb_len - sizeof(struct ps_packet_head),
                     buf, tx_gnss_len) != EOK) {
            PS_PRINT_ERR("buf is not enough\n");
        }

        /* push the skb to skb queue */
        ps_skb_enqueue_etc(ps_core_d, skb, TX_LOW_QUEUE);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_gnss_len;
        count = count - tx_gnss_len;
    }

    return 0;
}

int32 uart_loop_test_send_pkt_etc(void)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (uart_loop_test_info_etc == NULL) ||
                 (uart_loop_test_info_etc->tx_buf == NULL))) {
        PS_PRINT_ERR("para is invalided\n");
        return -EFAULT;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        PS_PRINT_ERR("uart loop test, tx low seq is too large [%d]\n", ps_core_d->tx_low_seq.qlen);
        return 0;
    }

    if (prepare_to_visit_node_etc(ps_core_d) < 0) {
        PS_PRINT_ERR("prepare work fail\n");
        return -EFAULT;
    }

    INIT_COMPLETION(uart_loop_test_info_etc->loop_test_done);

    /* to divide up packet function and tx to tty work */
    if (uart_loop_test_send_data_etc(ps_core_d, uart_loop_test_info_etc->tx_buf,
                                     uart_loop_test_cfg_etc.pkt_len) < 0) {
        PS_PRINT_ERR("uart loop test pkt send is err\n");
        post_to_visit_node_etc(ps_core_d);
        return -EFAULT;
    }

    timeleft = wait_for_completion_timeout(&uart_loop_test_info_etc->loop_test_done, msecs_to_jiffies(5000));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait uart loop done timeout\n");
        post_to_visit_node_etc(ps_core_d);
        return -ETIMEDOUT;
    }

    post_to_visit_node_etc(ps_core_d);

    return 0;
}

int32 uart_loop_test_data_check(uint8 *puc_src, uint8 *puc_dest, uint16 us_data_len)
{
    uint16 us_index;

    for (us_index = 0; us_index < us_data_len; us_index++) {
        if (puc_src[us_index] != puc_dest[us_index]) {
            return false;
        }
    }

    return true;
}

int32 uart_loop_test_recv_pkt_etc(struct ps_core_s *ps_core_d, const uint8 *buf_ptr, uint16 pkt_len)
{
    uint16 expect_pkt_len;
    uint8 *rx_buf = NULL;
    uint16 recvd_len;

    if (unlikely((ps_core_d == NULL) || (uart_loop_test_info_etc == NULL))) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    expect_pkt_len = uart_loop_test_info_etc->test_cfg->pkt_len;
    rx_buf = uart_loop_test_info_etc->rx_buf;
    recvd_len = uart_loop_test_info_etc->rx_pkt_len;

    if ((uint32)recvd_len + (uint32)pkt_len <= expect_pkt_len) {
        if (memcpy_s(&rx_buf[recvd_len], expect_pkt_len - recvd_len, buf_ptr, pkt_len) != EOK) {
            PS_PRINT_ERR("memcpy_s error, destlen=%d, srclen=%d\n ", expect_pkt_len - recvd_len, pkt_len);
        }
        uart_loop_test_info_etc->rx_pkt_len += pkt_len;
    } else {
        PS_PRINT_ERR("pkt len err! pkt_len=[%d], recvd_len=[%d], max len=[%d]\n", pkt_len, recvd_len, expect_pkt_len);
    }

    if (expect_pkt_len == uart_loop_test_info_etc->rx_pkt_len) {
        if (uart_loop_test_data_check(rx_buf, uart_loop_test_info_etc->tx_buf, expect_pkt_len)) {
            PS_PRINT_INFO("uart loop recv pkt SUCC\n");
        }
        uart_loop_test_info_etc->rx_pkt_len = 0;
        complete(&uart_loop_test_info_etc->loop_test_done);
    }

    return 0;
}

int32 uart_loop_test_etc(void)
{
    uint32 i, count;
    uint16 pkt_len;
    unsigned long long tx_total_len;
    unsigned long long total_time = 0;
    unsigned long long throughout;
    unsigned long long effect;
    ktime_t start_time, end_time, trans_time;
    uint8 *puc_buf = NULL;

    if (uart_loop_test_open_etc() < 0) {
        goto open_fail;
    }

    if (uart_loop_test_set_etc(UART_LOOP_SET_DEVICE_DATA_HANDLER) < 0) {
        goto test_set_fail;
    }

    count = uart_loop_test_info_etc->test_cfg->loop_count;
    pkt_len = uart_loop_test_info_etc->test_cfg->pkt_len;
    tx_total_len = ((unsigned long long)count) * ((unsigned long long)pkt_len);
    puc_buf = uart_loop_test_info_etc->tx_buf;

    for (i = 0; i < count; i++) {
        if (uart_loop_test_info_etc->test_cfg->uart_loop_tx_random_enable) {
            uart_loop_test_tx_buf_init(puc_buf, pkt_len);  // ��ʼ��tx_bufΪ�����
        }

        start_time = ktime_get();

        if (uart_loop_test_send_pkt_etc() != SUCCESS) {
            PS_PRINT_ERR("uart loop test fail, i=[%d]\n", i);
            goto send_pkt_fail;
        }

        end_time = ktime_get();
        trans_time = ktime_sub(end_time, start_time);
        total_time += (unsigned long long)ktime_to_us(trans_time);
    }

    if (uart_loop_test_set_etc(UART_LOOP_RESUME_DEVICE_DATA_HANDLER) < 0) {
        PS_PRINT_ERR("uart loop test, resume device data handler failer\n");
    }

    uart_loop_test_close_etc();

    /*
     *  �������ֽ���ת����bit��(B->b)�Է�����ļ���
     *  1000000 :1M������
     *   10=8+2 :uartÿ����1�ֽ�������Ҫ���2bit�Ŀ�ʼλ�ͽ���λ
     *     2    :��Ϊuart�ǻ��صģ���1�ֽ����ݻ����1�ֽ�����
     */
    throughout = tx_total_len * 1000000 * 10 * 2;
    if (total_time == 0) {
        PS_PRINT_ERR("divisor can not be zero!\n");
        return -FAILURE;
    }
    do_div(throughout, total_time);
    effect = throughout;
    do_div(throughout, 8192); /* b->B->KB : x*8*1024 */
    do_div(effect, (default_baud_rate / 100)); /* ���Բ���������ٷֱ�Ч�� */

    PS_PRINT_INFO("[UART Test] pkt count      [%d] pkts sent\n", count);
    PS_PRINT_INFO("[UART Test] pkt len        [%d] is pkt len\n", pkt_len);
    PS_PRINT_INFO("[UART Test] data lenth     [%llu]\n",
                  tx_total_len * 2); /* uart�ǻ��صģ���1�ֽ����ݻ����1�ֽ����� */
    PS_PRINT_INFO("[UART Test] used time      [%llu] us\n", total_time);
    PS_PRINT_INFO("[UART Test] throughout     [%llu] KBps\n", throughout);
    PS_PRINT_INFO("[UART Test] effect         [%llu]%%\n", effect);

    return SUCCESS;

send_pkt_fail:
test_set_fail:
    uart_loop_test_close_etc();
open_fail:
    return -FAILURE;
}

int conn_test_uart_loop_etc(char *param)
{
    return uart_loop_test_etc();
}
EXPORT_SYMBOL(conn_test_uart_loop_etc);

STATIC const struct file_operations hw_bt_fops = {
    .owner = THIS_MODULE,
    .open = hw_bt_open,
    .write = hw_bt_write,
    .read = hw_bt_read,
    .poll = hw_bt_poll,
    .unlocked_ioctl = hw_bt_ioctl,
    .release = hw_bt_release,
};

STATIC const struct file_operations hw_fm_fops = {
    .owner = THIS_MODULE,
    .open = hw_fm_open,
    .write = hw_fm_write,
    .read = hw_fm_read,
    .unlocked_ioctl = hw_fm_ioctl,
    .release = hw_fm_release,
};

STATIC const struct file_operations hw_gnss_fops = {
    .owner = THIS_MODULE,
    .open = hw_gnss_open,
    .write = hw_gnss_write,
    .read = hw_gnss_read,
    .poll = hw_gnss_poll,
    .unlocked_ioctl = hw_gnss_ioctl,
    .release = hw_gnss_release,
};

static const struct file_operations hw_ir_fops = {
    .owner = THIS_MODULE,
    .open = hw_ir_open,
    .write = hw_ir_write,
    .read = hw_ir_read,
    .release = hw_ir_release,
};

static const struct file_operations hw_nfc_fops = {
    .owner = THIS_MODULE,
    .open = hw_nfc_open,
    .write = hw_nfc_write,
    .read = hw_nfc_read,
    .poll = hw_nfc_poll,
    .release = hw_nfc_release,
};

STATIC const struct file_operations hw_debug_fops = {
    .owner = THIS_MODULE,
    .open = hw_debug_open,
#ifdef PLATFORM_DEBUG_ENABLE
    .write = hw_debug_write,
#endif
    .read = hw_debug_read,
    .unlocked_ioctl = hw_debug_ioctl,
    .release = hw_debug_release,
};
#ifdef HI110X_HAL_MEMDUMP_ENABLE
STATIC const struct file_operations hw_bfgexcp_fops = {
    .owner = THIS_MODULE,
    .read = hw_bfgexcp_read,
    .unlocked_ioctl = hw_bfgexcp_ioctl,
};

STATIC const struct file_operations hw_wifiexcp_fops = {
    .owner = THIS_MODULE,
    .read = hw_wifiexcp_read,
    .unlocked_ioctl = hw_wifiexcp_ioctl,
};
#else
STATIC const struct file_operations hw_excp_fops = {
    .owner = THIS_MODULE,
    .open = hw_excp_open,
    .unlocked_ioctl = hw_excp_ioctl,
    .release = hw_excp_release,
};
#endif
#ifdef HAVE_HISI_BT
STATIC struct miscdevice hw_bt_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbt",
    .fops = &hw_bt_fops,
};
#endif

#ifdef HAVE_HISI_FM
STATIC struct miscdevice hw_fm_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwfm",
    .fops = &hw_fm_fops,
};
#endif

#ifdef HAVE_HISI_GNSS
STATIC struct miscdevice hw_gnss_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwgnss",
    .fops = &hw_gnss_fops,
};
#endif

#ifdef HAVE_HISI_IR
STATIC struct miscdevice hw_ir_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwir",
    .fops = &hw_ir_fops,
};
#endif

#ifdef HAVE_HISI_NFC
STATIC struct miscdevice hw_nfc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwnfc",
    .fops = &hw_nfc_fops,
};
#endif

STATIC struct miscdevice hw_debug_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbfgdbg",
    .fops = &hw_debug_fops,
};
#ifdef HI110X_HAL_MEMDUMP_ENABLE
STATIC struct miscdevice hw_bfgexcp_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbfgexcp",
    .fops = &hw_bfgexcp_fops,
};
STATIC struct miscdevice hw_wifiexcp_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwwifiexcp",
    .fops = &hw_wifiexcp_fops,
};
#else
STATIC struct miscdevice hw_excp_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwexcp",
    .fops = &hw_excp_fops,
};
#endif
static struct hw_ps_plat_data hisi_platform_data = {
    .dev_name = "/dev/ttyAMA4",
    .flow_cntrl = FLOW_CTRL_ENABLE,
    .baud_rate = DEFAULT_BAUD_RATE,
    .suspend = NULL,
    .resume = NULL,
    .set_bt_power = NULL,
    .set_fm_power = NULL,
    .set_gnss_power = NULL,
    .clear_bt_power = NULL,
    .clear_fm_power = NULL,
    .clear_gnss_power = NULL,
};
#ifdef HAVE_HISI_NFC
static int plat_poweroff_notify_sys(struct notifier_block *this, unsigned long code, void *unused)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_gnss_data = NULL;
    struct inode gnss_inode;
    struct file gnss_filp;
    int32 err;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("plat_poweroff_notify_sys get ps_core_d is NULL\n");
        return NOTIFY_BAD;
    }

    pst_gnss_data = &ps_core_d->bfgx_info[BFGX_GNSS];
    if (atomic_read(&pst_gnss_data->subsys_state) == POWER_STATE_OPEN) {
        err = hw_gnss_release(&gnss_inode, &gnss_filp);
        if (err != 0) {
            PS_PRINT_WARNING("plat_poweroff_notify_sys call hw_gnss_release failed (err=%d)\n", err);
        }
    }
    return NOTIFY_OK;
}

static struct notifier_block plat_poweroff_notifier = {
    .notifier_call = plat_poweroff_notify_sys,
};
#endif

#ifdef _PRE_CONFIG_USE_DTS
STATIC int32 ps_misc_dev_register(void)
{
    int32 err;

    if (is_bfgx_support() != OAL_TRUE) {
        /* don't support bfgx */
        PS_PRINT_INFO("bfgx disabled, misc dev register bypass\n ");
        return 0;
    }

#ifdef HAVE_HISI_BT
    err = misc_register(&hw_bt_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register bt inode\n ");
        goto err_register_bt;
    }
#endif

#ifdef HAVE_HISI_FM
    err = misc_register(&hw_fm_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register fm inode\n ");
        goto err_register_fm;
    }
#endif

#ifdef HAVE_HISI_GNSS
    err = misc_register(&hw_gnss_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register gnss inode\n ");
        goto err_register_gnss;
    }
#endif

#ifdef HAVE_HISI_IR
    err = misc_register(&hw_ir_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register ir inode\n");
        goto err_register_ir;
    }
#endif

#ifdef HAVE_HISI_NFC
    err = misc_register(&hw_nfc_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register nfc inode\n ");
        goto err_register_nfc;
    }

    err = register_reboot_notifier(&plat_poweroff_notifier);
    if (err) {
        PS_PRINT_WARNING("Failed to registe plat_poweroff_notifier (err=%d)\n", err);
    }
#endif

    return 0;
#ifdef HAVE_HISI_NFC
err_register_nfc:
#endif
#ifdef HAVE_HISI_IR
    misc_deregister(&hw_ir_device);
err_register_ir:
#endif
#ifdef HAVE_HISI_GNSS
    misc_deregister(&hw_gnss_device);
err_register_gnss:
#endif
#ifdef HAVE_HISI_FM
    misc_deregister(&hw_fm_device);
err_register_fm:
#endif
#ifdef HAVE_HISI_BT
    misc_deregister(&hw_bt_device);
err_register_bt:
#endif
    return -EFAULT;
}

STATIC void ps_misc_dev_unregister(void)
{
    if (is_bfgx_support() != OAL_TRUE) {
        /* don't support bfgx */
        PS_PRINT_INFO("bfgx disabled, misc dev unregister bypass\n ");
        return;
    }
#ifdef HAVE_HISI_BT
    misc_deregister(&hw_bt_device);
    PS_PRINT_INFO("misc bt device have removed\n");
#endif
#ifdef HAVE_HISI_FM
    misc_deregister(&hw_fm_device);
    PS_PRINT_INFO("misc fm device have removed\n");
#endif
#ifdef HAVE_HISI_GNSS
    misc_deregister(&hw_gnss_device);
    PS_PRINT_INFO("misc gnss device have removed\n");
#endif
#ifdef HAVE_HISI_IR
    misc_deregister(&hw_ir_device);
    PS_PRINT_INFO("misc ir have removed\n");
#endif
#ifdef HAVE_HISI_NFC
    misc_deregister(&hw_nfc_device);
    PS_PRINT_INFO("misc nfc have removed\n");
#endif
}

STATIC int32 ps_probe(struct platform_device *pdev)
{
    struct hw_ps_plat_data *pdata = NULL;
    struct ps_plat_s *ps_plat_d = NULL;
    int32 err;
    BOARD_INFO *bd_info = NULL;

    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        PS_PRINT_ERR("board info is err\n");
        return -FAILURE;
    }

    strncpy_s(hisi_platform_data.dev_name, sizeof(hisi_platform_data.dev_name),
              bd_info->uart_port, sizeof(hisi_platform_data.dev_name) - 1);
    hisi_platform_data.dev_name[sizeof(hisi_platform_data.dev_name) - 1] = '\0';

    /* FPGA�汾֧��2M����̬�޸� */
    if (!isAsic_etc()) {
        hisi_platform_data.baud_rate = UART_BAUD_RATE_2M;
        default_baud_rate = UART_BAUD_RATE_2M;
    }

    PS_PRINT_INFO("init baudrate=%d\n", default_baud_rate);

    pdev->dev.platform_data = &hisi_platform_data;
    pdata = &hisi_platform_data;

    hw_ps_device_etc = pdev;

    ps_plat_d = kzalloc(sizeof(struct ps_plat_s), GFP_KERNEL);
    if (ps_plat_d == NULL) {
        PS_PRINT_ERR("no mem to allocate\n");
        return -ENOMEM;
    }
    dev_set_drvdata(&pdev->dev, ps_plat_d);

    err = ps_core_init_etc(&ps_plat_d->core_data);
    if (err != 0) {
        PS_PRINT_ERR(" PS core init failed\n");
        goto err_core_init;
    }

    /* refer to itself */
    ps_plat_d->core_data->pm_data = ps_plat_d;
    /* get reference of pdev */
    ps_plat_d->pm_pdev = pdev;

    init_completion(&ps_plat_d->ldisc_uninstalled);
    init_completion(&ps_plat_d->ldisc_installed);
    init_completion(&ps_plat_d->ldisc_reconfiged);

    err = plat_bfgx_exception_rst_register_etc(ps_plat_d);
    if (err < 0) {
        PS_PRINT_ERR("bfgx_exception_rst_register failed\n");
        goto err_exception_rst_reg;
    }

    err = bfgx_user_ctrl_init_etc();
    if (err < 0) {
        PS_PRINT_ERR("bfgx_user_ctrl_init_etc failed\n");
        goto err_user_ctrl_init;
    }

    err = bfgx_customize_init();
    if (err < 0) {
        PS_PRINT_ERR("bfgx_customize_init failed\n");
        goto err_bfgx_custmoize_exit;
    }

    /* copying platform data */
    if (strncpy_s(ps_plat_d->dev_name, sizeof(ps_plat_d->dev_name),
                  pdata->dev_name, HISI_UART_DEV_NAME_LEN - 1) != EOK) {
        PS_PRINT_ERR("strncpy_s failed, please check!\n");
    }
    ps_plat_d->flow_cntrl = pdata->flow_cntrl;
    ps_plat_d->baud_rate = pdata->baud_rate;
    PS_PRINT_INFO("sysfs entries created\n");

    tty_recv_etc = ps_core_recv_etc;

    err = ps_misc_dev_register();
    if (err != 0) {
        goto err_misc_dev;
    }

    err = misc_register(&hw_debug_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register debug inode\n");
        goto err_register_debug;
    }
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    err = misc_register(&hw_bfgexcp_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register hw_bfgexcp_device inode\n");
        goto err_register_bfgexcp;
    }

    err = misc_register(&hw_wifiexcp_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register hw_wifiexcp_device inode\n");
        goto err_register_wifiexcp;
    }
#else
    err = misc_register(&hw_excp_device);
    if (err != 0) {
        PS_PRINT_ERR("Failed to register hw excp inode\n");
        goto err_register_excp;
    }
#endif

    PS_PRINT_SUC("%s is success!\n", __func__);

    return 0;
#ifdef HI110X_HAL_MEMDUMP_ENABLE
err_register_wifiexcp:
    misc_deregister(&hw_bfgexcp_device);
err_register_bfgexcp:
#else
err_register_excp:
#endif
    misc_deregister(&hw_debug_device);
err_register_debug:
    ps_misc_dev_unregister();
err_misc_dev:
err_bfgx_custmoize_exit:
    bfgx_user_ctrl_exit_etc();
err_user_ctrl_init:
err_exception_rst_reg:
    ps_core_exit_etc(ps_plat_d->core_data);
err_core_init:
    kfree(ps_plat_d);

    return -EFAULT;
}

/*
 * Prototype    : ps_suspend_etc
 * Description  : called by kernel when kernel goto suspend
 */
int32 ps_suspend_etc(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

/*
 * Prototype    : ps_resume_etc
 * Description  : called by kernel when kernel resume from suspend
 */
int32 ps_resume_etc(struct platform_device *pdev)
{
    return 0;
}

/*
 * Prototype    : ps_remove
 * Description  : called when user applation rmmod driver
 */
STATIC int32 ps_remove(struct platform_device *pdev)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct hw_ps_plat_data *pdata = NULL;

    PS_PRINT_FUNCTION_NAME;

    pdata = pdev->dev.platform_data;
    ps_plat_d = dev_get_drvdata(&pdev->dev);
    if (ps_plat_d == NULL) {
        PS_PRINT_ERR("ps_plat_d is null\n");
    }

    bfgx_user_ctrl_exit_etc();
    PS_PRINT_INFO("sysfs user ctrl removed\n");

    if (ps_plat_d != NULL) {
        ps_plat_d->pm_pdev = NULL;
        ps_core_exit_etc(ps_plat_d->core_data);
    }

    ps_misc_dev_unregister();
    misc_deregister(&hw_debug_device);
    PS_PRINT_INFO("misc debug device have removed\n");

    if (ps_plat_d != NULL) {
        kfree(ps_plat_d);
        ps_plat_d = NULL;
    }

    return 0;
}

static struct of_device_id hi110x_ps_match_table[] = {
    {
        .compatible = DTS_COMP_HI110X_PS_NAME,
        .data = NULL,
    },
    {},
};
#endif

/*  platform_driver struct for PS module */
STATIC struct platform_driver ps_platform_driver = {
#ifdef _PRE_CONFIG_USE_DTS
    .probe = ps_probe,
    .remove = ps_remove,
    .suspend = ps_suspend_etc,
    .resume = ps_resume_etc,
#endif
    .driver = {
        .name = "hisi_bfgx",
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hi110x_ps_match_table,
#endif
    },
};

int32 hw_ps_init_etc(void)
{
    int32 ret;

    PS_PRINT_FUNCTION_NAME;

    ret = platform_driver_register(&ps_platform_driver);
    if (ret) {
        PS_PRINT_ERR("Unable to register platform bfgx driver.\n");
    }
    return ret;
}

void hw_ps_exit_etc(void)
{
    platform_driver_unregister(&ps_platform_driver);
}

MODULE_DESCRIPTION("Public serial Driver for huawei BT/FM/GNSS chips");
MODULE_LICENSE("GPL");
