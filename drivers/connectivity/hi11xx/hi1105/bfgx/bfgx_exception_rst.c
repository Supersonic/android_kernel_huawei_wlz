

/* ͷ�ļ����� */
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/tty.h>

#include "plat_type.h"
#include "plat_debug.h"
#include "hw_bfg_ps.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "bfgx_exception_rst.h"
#include "plat_firmware.h"
#include "plat_uart.h"

#include "oal_sdio.h"
#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oal_ext_if.h"
#include <linux/ktime.h>
#include "chr_errno.h"
#include "oam_rdr.h"
#include "securec.h"

/* ȫ�ֱ������� */
struct st_exception_info *exception_info_etc = NULL;
struct sdio_dump_bcpu_buff st_bcpu_dump_buff_etc = { NULL, 0, 0 };
oal_netbuf_stru *st_bcpu_dump_netbuf_etc = NULL;

uint8 *bfgx_mem_file_name_etc[BFGX_MEM_DUMP_BLOCK_COUNT] = {
    "bcpu_itcm_mem",
    "bcpu_dtcm_mem",
};

struct st_exception_mem_info bfgx_mem_dump_etc[BFGX_MEM_DUMP_BLOCK_COUNT] = {{0}, {0}};

uint32 recvd_block_count_etc = 0;

#define WIFI_PUB_REG_BLOCKS  12
#define WIFI_PRIV_REG_BLOCKS 9
#define WIFI_MEM_BLOCKS      3

#define WIFI_PUB_GLB_CTL_LEN       (4 * 1024)
#define WIFI_PUB_PMU_CMU_CTL_LEN   (4 * 1024)
#define WIFI_PUB_RF_WB_CTL_LEN     (2 * 1024)
#define WIFI_PUB_RF_WB_TRX_REG_LEN (2 * 1024)
#define WIFI_PUB_RF_WB_PLL_REG_LEN (2 * 1024)
#define WIFI_PUB_RF_FG_CTL_LEN     (2 * 1024)
#define WIFI_PUB_RF_FG_TRX_REG_LEN (2 * 1024)
#define WIFI_PUB_RF_FG_PLL_REG_LEN (2 * 1024)
#define WIFI_PUB_COEX_CTL_LEN      (1 * 1024)
#define WIFI_PUB_DIAG_CTL_LEN      (1 * 1024)
#define WIFI_PUB_COM_CTL_LEN       (2 * 1024)
#define WIFI_PUB_AON_GPIO_RTC_LEN  (16 * 1024)

#define WIFI_PUB_REG_TOTAL_LEN (WIFI_PUB_GLB_CTL_LEN + WIFI_PUB_PMU_CMU_CTL_LEN + \
                                WIFI_PUB_RF_WB_CTL_LEN + WIFI_PUB_RF_WB_TRX_REG_LEN + \
                                WIFI_PUB_RF_WB_PLL_REG_LEN + WIFI_PUB_RF_FG_CTL_LEN + \
                                WIFI_PUB_RF_FG_TRX_REG_LEN + WIFI_PUB_RF_FG_PLL_REG_LEN + \
                                WIFI_PUB_COEX_CTL_LEN + WIFI_PUB_DIAG_CTL_LEN + \
                                WIFI_PUB_COM_CTL_LEN + WIFI_PUB_AON_GPIO_RTC_LEN)

#define WIFI_PRIV_W_CTL_LEN       (1 * 1024)
#define WIFI_PRIV_W_WDT_LEN       (1 * 1024)
#define WIFI_PRIV_W_TIMER_LEN     (1 * 1024)
#define WIFI_PRIV_W_CPU_CTL_LEN   (1 * 1024)
#define WIFI_PRIV_W_PHY_BANK1_LEN (1 * 1024)
#define WIFI_PRIV_W_PHY_BANK2_LEN (1 * 1024)
#define WIFI_PRIV_W_PHY_BANK3_LEN (1 * 1024)
#define WIFI_PRIV_W_PHY_BANK4_LEN (1 * 1024)
#define WIFI_PRIV_W_MAC_BANK_LEN  (2 * 1024 + 512)
#define WIFI_PRIV_W_DMA_LEN       (1 * 1024)

#define WIFI_PRIV_REG_TOTAL_LEN (WIFI_PRIV_W_CTL_LEN \
                                           + WIFI_PRIV_W_WDT_LEN \
                                           + WIFI_PRIV_W_TIMER_LEN \
                                           + WIFI_PRIV_W_CPU_CTL_LEN \
                                           + WIFI_PRIV_W_PHY_BANK1_LEN \
                                           + WIFI_PRIV_W_PHY_BANK2_LEN \
                                           + WIFI_PRIV_W_PHY_BANK3_LEN \
                                           + WIFI_PRIV_W_PHY_BANK4_LEN \
                                           + WIFI_PRIV_W_MAC_BANK_LEN \
                                           + WIFI_PRIV_W_DMA_LEN)

#define WIFI_MEM_W_TCM_WRAM_LEN      (512 * 1024)
#define WIFI_MEM_W_PKT_SHARE_RAM_LEN (256 * 1024)
#define WIFI_MEM_B_SHARE_RAM_LEN     (64 * 1024)

#define WIFI_MEM_TOTAL_LEN (WIFI_MEM_W_TCM_WRAM_LEN \
                                           + WIFI_MEM_W_PKT_SHARE_RAM_LEN \
                                           + WIFI_MEM_B_SHARE_RAM_LEN)

#define BFGX_PUB_REG_NUM   5
#define BFGX_PRIV_REG_NUM  2
#define BFGX_SHARE_RAM_NUM 2

#define BFGX_GLB_PMU_CMU_CTL_ADDR                0x50000000
#define BFGX_RF_WB_CTL_ADDR                      0x5000C000
#define BFGX_RF_FG_CTL_ADDR                      0x5000E000
#define BFGX_COEX_DIAG_COM_CTL_ADDR              0x50010000
#define BFGX_AON_WB_GPIO_RTC_ADDR                0x50006000
#define BFGX_PATCH_PLAT_NFC_BFGNI_SHARE_RAM_ADDR 0x300E4000
#define BFGX_W_SHARE_RAM_ADDR                    0x30178000
#define BFGX_GNSS_SUB_ADDR                       0x38000000
#define BFGX_B_CTL_WDT_TIMER_UART_ADDR           0x40000000
#define BFGX_IR_SUB_ADDR                         0x40007000
#define BFGX_B_DMA_CFG_ADDR                      0x40010000
#define BFGX_BT_FM_SUB_ADDR                      0x41040000
#define BFGX_NFC_APB_ADDR                        0x60000000

#define BFGX_GLB_PMU_CMU_CTL_LEN                (1024 * 12)
#define BFGX_RF_WB_CTL_LEN                      (1024 * 8)
#define BFGX_RF_FG_CTL_LEN                      (1024 * 8)
#define BFGX_COEX_DIAG_COM_CTL_LEN              (1024 * 20)
#define BFGX_AON_WB_GPIO_RTC_LEN                (1024 * 16)
#define BFGX_PATCH_PLAT_NFC_BFGNI_SHARE_RAM_LEN (1024 * 592)
#define BFGX_W_SHARE_RAM_LEN                    (1024 * 32)
#define BFGX_GNSS_SUB_LEN                       (1024 * 128)
#define BFGX_B_CTL_WDT_TIMER_UART_LEN           (1024 * 20)
#define BFGX_IR_SUB_LEN                         (1024 * 4)
#define BFGX_B_DMA_CFG_LEN                      (1024 * 4)
#define BFGX_BT_FM_SUB_LEN                      (1024 * 140)
#define BFGX_NFC_APB_LEN                        (1024 * 20)

#define GLB_PMU_CMU_CTL_FILE_NAME                "glb_pmu_cmu_ctl"
#define RF_WB_CTL_FILE_NAME                      "rf_wb_ctl"
#define RF_FG_CTL_FILE_NAME                      "rf_fg_ctl"
#define COEX_DIAG_COM_CTL_FILE_NAME              "coex_diag_com_ctl"
#define AON_WB_GPIO_RTC_FILE_NAME                "aon_wb_gpio_rtc"
#define PATCH_PLAT_NFC_BFGNI_SHARE_RAM_FILE_NAME "patch_plat_nfc_bfgni_share_ram"
#define W_SHARE_RAM_FILE_NAME                    "w_share_ram"
#define GNSS_SUB_FILE_NAME                       "gnss_sub"
#define B_CTL_WDT_TIMER_UART_FILE_NAME           "b_ctl_wdt_timer_uart"
#define IR_SUB_FILE_NAME                         "ir_sub"
#define B_DMA_CFG_FILE_NAME                      "b_dma_cfg"
#define BT_FM_SUB_FILE_NAME                      "bt_fm_sub"
#define NFC_APB_FILE_NAME                        "nfc_apb"

#define EXCEPTION_FILE_NAME_LEN 100

struct st_uart_dump_wifi_mem_info ast_wifi_pub_reg_info_etc[WIFI_PUB_REG_BLOCKS] = {
    { "glb_ctl",       WIFI_PUB_GLB_CTL_LEN },
    { "pmu_cmu_ctl",   WIFI_PUB_PMU_CMU_CTL_LEN },
    { "rf_wb_ctl",     WIFI_PUB_RF_WB_CTL_LEN },
    { "rf_wb_trx_reg", WIFI_PUB_RF_WB_TRX_REG_LEN },
    { "rf_wb_pll_reg", WIFI_PUB_RF_WB_PLL_REG_LEN },
    { "rf_fg_ctl",     WIFI_PUB_RF_FG_CTL_LEN },
    { "rf_fg_trx_reg", WIFI_PUB_RF_FG_TRX_REG_LEN },
    { "rf_fg_pll_reg", WIFI_PUB_RF_FG_PLL_REG_LEN },
    { "coex_ctl",      WIFI_PUB_COEX_CTL_LEN },
    { "diag_ctl",      WIFI_PUB_DIAG_CTL_LEN },
    { "com_ctl",       WIFI_PUB_COM_CTL_LEN },
    { "aon_gpio_rtc",  WIFI_PUB_AON_GPIO_RTC_LEN },
};

struct st_uart_dump_wifi_mem_info ast_wifi_priv_reg_info_etc[WIFI_PRIV_REG_BLOCKS] = {
    { "w_ctl",       WIFI_PRIV_W_CTL_LEN },
    { "w_wdt",       WIFI_PRIV_W_WDT_LEN },
    { "w_timer",     WIFI_PRIV_W_TIMER_LEN },
    { "w_cpu_ctl",   WIFI_PRIV_W_CPU_CTL_LEN },
    { "w_phy_bank1", WIFI_PRIV_W_PHY_BANK1_LEN },
    { "w_phy_bank2", WIFI_PRIV_W_PHY_BANK2_LEN },
    { "w_phy_bank3", WIFI_PRIV_W_PHY_BANK3_LEN },
    { "w_phy_bank4", WIFI_PRIV_W_PHY_BANK4_LEN },
    { "w_mac_bank",  WIFI_PRIV_W_MAC_BANK_LEN },
    /* {"w_dma",       WIFI_PRIV_W_DMA_LEN}, */
};

struct st_uart_dump_wifi_mem_info ast_wifi_mem_info_etc[WIFI_MEM_BLOCKS] = {
    { "w_tcm_wram",      WIFI_MEM_W_TCM_WRAM_LEN },
    { "w_pkt_share_ram", WIFI_MEM_W_PKT_SHARE_RAM_LEN },
    { "b_share_ram",     WIFI_MEM_B_SHARE_RAM_LEN },
};

struct st_uart_dump_wifi_info uart_read_wifi_mem_info_etc[UART_WIFI_MEM_DUMP_BOTTOM] = {
    { SYS_CFG_READ_WLAN_PUB_REG,  WIFI_PUB_REG_TOTAL_LEN,  WIFI_PUB_REG_BLOCKS,  ast_wifi_pub_reg_info_etc },
    { SYS_CFG_READ_WLAN_PRIV_REG, WIFI_PRIV_REG_TOTAL_LEN, WIFI_PRIV_REG_BLOCKS, ast_wifi_priv_reg_info_etc },
    { SYS_CFG_READ_WLAN_MEM,      WIFI_MEM_TOTAL_LEN,      WIFI_MEM_BLOCKS,      ast_wifi_mem_info_etc },
};

struct st_exception_mem_info uart_wifi_mem_dump_etc[UART_WIFI_MEM_DUMP_BOTTOM] = {{0}, {0}, {0}};
uint32 recvd_wifi_block_index_etc = UART_WIFI_MEM_DUMP_BOTTOM;

struct st_bfgx_reset_cmd ast_bfgx_reset_msg_etc[BFGX_BUTT] = {
    { BT_RESET_CMD_LEN,   { 0x04, 0xff, 0x01, 0xc7 }},
    { FM_RESET_CMD_LEN,   { 0xfb }},
    { GNSS_RESET_CMD_LEN, { 0xc7, 0x51, 0x0, 0x0, 0xa5, 0xa5, 0x0, 0x0, GNSS_SEPER_TAG_LAST }},
    { IR_RESET_CMD_LEN,   {0}},
    { NFC_RESET_CMD_LEN,  {0}},
};

exception_bcpu_dump_msg sdio_read_bcpu_pub_reg_info_etc[BFGX_PUB_REG_NUM] = {
    { GLB_PMU_CMU_CTL_FILE_NAME,   BFGX_GLB_PMU_CMU_CTL_ADDR,   ALIGN_2_BYTE, BFGX_GLB_PMU_CMU_CTL_LEN },
    { RF_WB_CTL_FILE_NAME,         BFGX_RF_WB_CTL_ADDR,         ALIGN_2_BYTE, BFGX_RF_WB_CTL_LEN },
    { RF_FG_CTL_FILE_NAME,         BFGX_RF_FG_CTL_ADDR,         ALIGN_2_BYTE, BFGX_RF_FG_CTL_LEN },
    { COEX_DIAG_COM_CTL_FILE_NAME, BFGX_COEX_DIAG_COM_CTL_ADDR, ALIGN_2_BYTE, BFGX_COEX_DIAG_COM_CTL_LEN },
    { AON_WB_GPIO_RTC_FILE_NAME,   BFGX_AON_WB_GPIO_RTC_ADDR,   ALIGN_2_BYTE, BFGX_AON_WB_GPIO_RTC_LEN },
};
exception_bcpu_dump_msg sdio_read_bcpu_mem_info_etc[BFGX_SHARE_RAM_NUM] = {
    {   PATCH_PLAT_NFC_BFGNI_SHARE_RAM_FILE_NAME, BFGX_PATCH_PLAT_NFC_BFGNI_SHARE_RAM_ADDR,
        ALIGN_1_BYTE, BFGX_PATCH_PLAT_NFC_BFGNI_SHARE_RAM_LEN
    },
    { W_SHARE_RAM_FILE_NAME, BFGX_W_SHARE_RAM_ADDR, ALIGN_1_BYTE, BFGX_W_SHARE_RAM_LEN },
};
exception_bcpu_dump_msg sdio_read_bcpu_priv_reg_info_etc[BFGX_PRIV_REG_NUM] = {
    /* ˽�мĴ���ֻ�ܿ���2�� */
    { B_CTL_WDT_TIMER_UART_FILE_NAME, BFGX_B_CTL_WDT_TIMER_UART_ADDR, ALIGN_2_BYTE, BFGX_B_CTL_WDT_TIMER_UART_LEN },
    { IR_SUB_FILE_NAME,               BFGX_IR_SUB_ADDR,               ALIGN_2_BYTE, BFGX_IR_SUB_LEN },
};

exception_bcpu_dump_msg sdio_read_all_etc[BFGX_PUB_REG_NUM + BFGX_SHARE_RAM_NUM + BFGX_PRIV_REG_NUM] = {
    { GLB_PMU_CMU_CTL_FILE_NAME,   BFGX_GLB_PMU_CMU_CTL_ADDR,   ALIGN_2_BYTE, BFGX_GLB_PMU_CMU_CTL_LEN },
    { RF_WB_CTL_FILE_NAME,         BFGX_RF_WB_CTL_ADDR,         ALIGN_2_BYTE, BFGX_RF_WB_CTL_LEN },
    { RF_FG_CTL_FILE_NAME,         BFGX_RF_FG_CTL_ADDR,         ALIGN_2_BYTE, BFGX_RF_FG_CTL_LEN },
    { COEX_DIAG_COM_CTL_FILE_NAME, BFGX_COEX_DIAG_COM_CTL_ADDR, ALIGN_2_BYTE, BFGX_COEX_DIAG_COM_CTL_LEN },
    { AON_WB_GPIO_RTC_FILE_NAME,   BFGX_AON_WB_GPIO_RTC_ADDR,   ALIGN_2_BYTE, BFGX_AON_WB_GPIO_RTC_LEN },
    {   PATCH_PLAT_NFC_BFGNI_SHARE_RAM_FILE_NAME, BFGX_PATCH_PLAT_NFC_BFGNI_SHARE_RAM_ADDR,
        ALIGN_1_BYTE, BFGX_PATCH_PLAT_NFC_BFGNI_SHARE_RAM_LEN
    },
    { W_SHARE_RAM_FILE_NAME,          BFGX_W_SHARE_RAM_ADDR,          ALIGN_1_BYTE, BFGX_W_SHARE_RAM_LEN },
    { B_CTL_WDT_TIMER_UART_FILE_NAME, BFGX_B_CTL_WDT_TIMER_UART_ADDR, ALIGN_2_BYTE, BFGX_B_CTL_WDT_TIMER_UART_LEN },
    { IR_SUB_FILE_NAME,               BFGX_IR_SUB_ADDR,               ALIGN_2_BYTE, BFGX_IR_SUB_LEN },
};

uint8 plat_beatTimer_timeOut_reset_cfg_etc = 0;
uint8 gst_excp_test_cfg[EXCP_TEST_CFG_BOTT] = { DFR_TEST_DISABLE, DFR_TEST_DISABLE,
                                                DFR_TEST_DISABLE, DFR_TEST_DISABLE };
#ifdef HI110X_HAL_MEMDUMP_ENABLE
memdump_info_t bcpu_memdump_cfg_etc;
memdump_info_t wcpu_memdump_cfg_etc;
#endif

void bfgx_beat_timer_expire_etc(uint64 data);
int32 get_exception_info_reference_etc(struct st_exception_info **exception_data);
int32 plat_exception_handler_etc(uint32 subsys_type, uint32 thread_type, uint32 exception_type);
void plat_exception_reset_work_etc(struct work_struct *work);
int32 wifi_exception_handler_etc(void);
int32 wifi_subsystem_reset_etc(void);
int32 wifi_system_reset_etc(void);
int32 wifi_status_recovery_etc(void);
int32 wifi_exception_mem_dump_etc(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count, oal_int32 excep_type);
int32 bfgx_exception_handler_etc(void);
int32 bfgx_subthread_reset_etc(void);
int32 bfgx_subsystem_reset_etc(void);
int32 bfgx_system_reset_etc(void);
int32 bfgx_recv_dev_mem_etc(uint8 *buf_ptr, uint16 count);
int32 bfgx_store_stack_mem_to_file_etc(void);
void bfgx_dump_stack_etc(void);
int32 bfgx_status_recovery_etc(void);
int32 plat_bfgx_exception_rst_register_etc(struct ps_plat_s *data);
int32 plat_exception_reset_init_etc(void);
int32 plat_exception_reset_exit_etc(void);

/*
 * �� �� ��  : plat_dfr_cfg_set_etc
 * ��������  : dfrȫ������
 */
void plat_dfr_cfg_set_etc(uint64 arg)
{
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }

    pst_exception_data->exception_reset_enable = arg ? (PLAT_EXCEPTION_ENABLE) : (PLAT_EXCEPTION_DISABLE);

    PS_PRINT_INFO("plat dfr cfg set value = %ld\n", arg);
}

/*
 * �� �� ��  : plat_beatTimer_timeOut_reset_cfg_set_etc
 * ��������  : beat_timerȫ������
 */
void plat_beatTimer_timeOut_reset_cfg_set_etc(uint64 arg)
{
    plat_beatTimer_timeOut_reset_cfg_etc = arg ? (PLAT_EXCEPTION_ENABLE) : (PLAT_EXCEPTION_DISABLE);
    PS_PRINT_INFO("plat beat timer timeOut reset cfg set value = %ld\n", arg);
}

int32 mod_beat_timer_etc(uint8 on)
{
    int ret;
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (on) {
        ret = mod_timer(&pst_exception_data->bfgx_beat_timer, jiffies + BFGX_BEAT_TIME * HZ);
        atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_NOT_RECV_BEAT_INFO);
        PS_PRINT_INFO("reset beat timer, ret=%d, jiffers=%lu, expires=%lu\n",
                      ret, jiffies, pst_exception_data->bfgx_beat_timer.expires);
    } else {
        PS_PRINT_INFO("delete beat timer\n");
        del_timer_sync(&pst_exception_data->bfgx_beat_timer);
    }

    return 0;
}

/*
 * �� �� ��  : bfgx_beat_timer_expire_etc
 * ��������  : bfgx������ʱ�����������ú������������ж��������У�����������˯�ߵĲ���
 * �������  : uint64 data������Ҫ��������Ϊ�������ں˵ĺ�������
 */
void bfgx_beat_timer_expire_etc(uint64 data)
{
    int ret;
    static uint32 timer_restart_cnt = 0;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    struct tty_buffer *thead;
    struct tty_bufhead *buf = NULL;
#endif

    if (bfgx_is_shutdown_etc()) {
        PS_PRINT_WARNING("bfgx is closed\n");
        return;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }

    if (unlikely(pst_exception_data->ps_plat_d == NULL)) {
        PS_PRINT_ERR("pst_exception_data->ps_plat_d is NULL\n");
        return;
    }

    ps_core_d = pst_exception_data->ps_plat_d->core_data;

    /* bfgx˯��ʱ��û��������Ϣ�ϱ� */
    if (ps_core_d->ps_pm->bfgx_dev_state_get() == BFGX_SLEEP) {
        PS_PRINT_INFO("bfgx has sleep!\n");
        return;
    }

    if (atomic_read(&pst_exception_data->bfgx_beat_flag) == BFGX_NOT_RECV_BEAT_INFO) {
        if (OAL_IS_ERR_OR_NULL(ps_core_d->tty)) {
            PS_PRINT_ERR("tty is NULL\n");
            return;
        }

        PS_PRINT_INFO("timeout!  HZ=%d, jiffers=%lu, expires=%lu\n",
                      HZ, jiffies, pst_exception_data->bfgx_beat_timer.expires);

        /* �쳣��ʱ��ʱ��δ����������ʱ������������5�� */
        if (time_after(pst_exception_data->bfgx_beat_timer.expires, jiffies) && (timer_restart_cnt < 5)) {
            timer_restart_cnt++;
            ret = mod_timer(&pst_exception_data->bfgx_beat_timer, jiffies + msecs_to_jiffies(BFGX_BEAT_TIME * 1000));
            PS_PRINT_ERR("error! timer is not expires and restart, ret=%d, jiffers=%lu, expires=%lu\n",
                          ret, jiffies, pst_exception_data->bfgx_beat_timer.expires);
            return;
        }

        timer_restart_cnt = 0;

        PS_PRINT_ERR("###########host can not recvive bfgx beat info@@@@@@@@@@@@@@!\n");
        ps_uart_state_pre_etc(ps_core_d->tty);
        ps_uart_state_dump_etc(ps_core_d->tty);

        if (ps_core_d->tty->port == NULL) {
            PS_PRINT_ERR("tty->port is null, bfgx download patch maybe failed!\n");
            return;
        }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
        buf = &(ps_core_d->tty->port->buf);
        thead = buf->head;
        while (thead != NULL) {
            PS_PRINT_INFO("tty rx buf:used=0x%x,size=0x%x,commit=0x%x,read=0x%x\n",
                          thead->used, thead->size, thead->commit, thead->read);
            thead = thead->next;
        }
#endif

        DECLARE_DFT_TRACE_KEY_INFO("bfgx beat timeout", OAL_DFT_TRACE_EXCEP);

        if (ssi_dump_en) {
            ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL | SSI_MODULE_MASK_WCTRL);
        }

        if (plat_beatTimer_timeOut_reset_cfg_etc == PLAT_EXCEPTION_ENABLE) {
            PS_PRINT_ERR("bfgx beat timer bring to reset work!\n");
            plat_exception_handler_etc(SUBSYS_BFGX, THREAD_IDLE, BFGX_BEATHEART_TIMEOUT);
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                                 CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BEAT_TIMEOUT);
            return;
        }
    }

    atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_NOT_RECV_BEAT_INFO);

    timer_restart_cnt = 0;
    ret = mod_timer(&pst_exception_data->bfgx_beat_timer, jiffies + BFGX_BEAT_TIME * HZ);
    PS_PRINT_INFO("reset beat timer, ret=%d, jiffers=%lu, expires=%lu\n",
                  ret, jiffies, pst_exception_data->bfgx_beat_timer.expires);

    bfg_check_timer_work();

    return;
}

/*
 * �� �� ��  : get_exception_info_reference_etc
 * ��������  : ��ñ����쳣��Ϣ��ȫ�ֱ���
 * �������  : st_exception_info **exception_data�ṹ��ָ�룬����ȫ�ֱ�����ַ��ָ��
 */
int32 get_exception_info_reference_etc(struct st_exception_info **exception_data)
{
    if (exception_data == NULL) {
        PS_PRINT_ERR("%s parm exception_data is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    if (exception_info_etc == NULL) {
        *exception_data = NULL;
        PS_PRINT_ERR("%s exception_info_etc is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    *exception_data = exception_info_etc;

    return EXCEPTION_SUCCESS;
}

int32 uart_reset_wcpu_etc(void)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EXCEPTION_FAIL;
    }

    if (pst_exception_data->subsys_type >= SUBSYS_BOTTOM) {
        PS_PRINT_ERR("subsys [%d] is error!\n", pst_exception_data->subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (prepare_to_visit_node_etc(ps_core_d) != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return -EXCEPTION_FAIL;
    }

    INIT_COMPLETION(ps_core_d->wait_wifi_opened);
    ps_uart_state_pre_etc(ps_core_d->tty);

    if (pst_exception_data->subsys_type == SUBSYS_WIFI) {
        PS_PRINT_INFO("uart reset WCPU\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_EXCEP_RESET_WCPU);
    } else {
        PS_PRINT_INFO("uart open WCPU\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_OPEN_WIFI);
    }

    timeleft = wait_for_completion_timeout(&ps_core_d->wait_wifi_opened, msecs_to_jiffies(WAIT_WIFI_OPEN_TIME));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait wifi open ack timeout\n");
        post_to_visit_node_etc(ps_core_d);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_OPEN_WCPU);
        return -EXCEPTION_FAIL;
    }

    post_to_visit_node_etc(ps_core_d);

    return EXCEPTION_SUCCESS;
}
void plat_dfr_sysrst_type_cnt_inc(enum DFR_RST_SYSTEM_TYPE_E rst_type, enum SUBSYSTEM_ENUM subs)
{
    struct st_exception_info *pst_exception_data = NULL;
    if ((rst_type >= DFR_SYSTEM_RST_TYPE_BOTT) || (subs >= SUBSYS_BOTTOM)) {
        goto exit;
    }
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        goto exit;
    }
    if (pst_exception_data->subsys_type == subs) {
        pst_exception_data->etype_info[pst_exception_data->excetion_type].rst_type_cnt[rst_type]++;
    }
    return;
exit:
    PS_PRINT_ERR("dfr rst type cnt inc fail\n");
}
excp_info_str_t excp_info_str_tab[] = {
    { .id = BFGX_BEATHEART_TIMEOUT, .name = "bfgx_beatheart_timeout" },
    { .id = BFGX_LASTWORD_PANIC,    .name = "bfgx_lastword_panic" },
    { .id = BFGX_TIMER_TIMEOUT,     .name = "bfgx_timer_timeout" },
    { .id = BFGX_ARP_TIMEOUT,       .name = "bfgx_arp_timeout" },
    { .id = BFGX_POWERON_FAIL,      .name = "bfgx_poweron_fail" },
    { .id = BFGX_WAKEUP_FAIL,       .name = "bfgx_wakeup_fail" },
    { .id = WIFI_WATCHDOG_TIMEOUT,  .name = "wifi_watchdog_timeout" },
    { .id = WIFI_POWERON_FAIL,      .name = "wifi_poweron_fail" },
    { .id = WIFI_WAKEUP_FAIL,       .name = "wifi_wakeup_fail" },
    { .id = WIFI_DEVICE_PANIC,      .name = "wifi_device_panic" },
    { .id = WIFI_TRANS_FAIL,        .name = "wifi_trans_fail" },
    { .id = SDIO_DUMPBCPU_FAIL,     .name = "sdio_dumpbcpu_fail" },
};
char *excp_info_str_get(int32 id)
{
    int32 i = 0;
    for (i = 0; i < sizeof(excp_info_str_tab) / sizeof(excp_info_str_t); i++) {
        if (id == excp_info_str_tab[i].id) {
            return excp_info_str_tab[i].name;
        }
    }
    return NULL;
}
int32 plat_get_dfr_sinfo(char *buf, int32 index)
{
    struct st_exception_info *pst_exception_data = NULL;
    int i = index;
    int etype;
    int ret;
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }
    ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1, "==========dfr info:=========\n");
    if (ret < 0) {
        return i;
    }
    i += ret;

    ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1, "total cnt:%-10d\n", plat_get_excp_total_cnt());
    if (ret < 0) {
        return i;
    }
    i += ret;

    for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
        ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1,
                         "id:%-30scnt:%-10dfail:%-10dsingle_sysrst:%-10dall_sysrst:%-10dmaxtime:%lldms\n",
                         excp_info_str_get(etype),
                         pst_exception_data->etype_info[etype].excp_cnt,
                         pst_exception_data->etype_info[etype].fail_cnt,
                         pst_exception_data->etype_info[etype].rst_type_cnt[DFR_SINGLE_SYS_RST],
                         pst_exception_data->etype_info[etype].rst_type_cnt[DFR_ALL_SYS_RST],
                         pst_exception_data->etype_info[etype].maxtime);
        if (ret < 0) {
            return i;
        }
        i += ret;
    }
    return i;
}
void plat_print_dfr_info(void)
{
    struct st_exception_info *pst_exception_data = NULL;
    int etype;
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }
    PS_PRINT_INFO("========== dfr info:+++++++++++++++++++++++++++++++\n");
    PS_PRINT_INFO("total cnt:%-10d\n", plat_get_excp_total_cnt());
    for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
        PS_PRINT_INFO("type:%-30scnt:%-10dfail:%-10dsingle_sysrst:%-10dall_sysrst:%-10dmaxtime:%lldms\n",
                      excp_info_str_get(etype),
                      pst_exception_data->etype_info[etype].excp_cnt,
                      pst_exception_data->etype_info[etype].fail_cnt,
                      pst_exception_data->etype_info[etype].rst_type_cnt[DFR_SINGLE_SYS_RST],
                      pst_exception_data->etype_info[etype].rst_type_cnt[DFR_ALL_SYS_RST],
                      pst_exception_data->etype_info[etype].maxtime);
    }
    PS_PRINT_INFO("========== dfr info:-----------------------------------\n");
}

int32 plat_get_excp_total_cnt()
{
    struct st_exception_info *pst_exception_data = NULL;
    int i;
    int total_cnt = 0;
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }
    for (i = 0; i < EXCEPTION_TYPE_BOTTOM; i++) {
        total_cnt += pst_exception_data->etype_info[i].excp_cnt;
    }
    return total_cnt;
}
/*
 * �� �� ��  : plat_power_fail_exception_info_set_etc
 * ��������  : bfgx openʧ��ʱ�����øú�������exception info
 * �������  : subsys_type:�����쳣��������ϵͳ��WIFI����BFGX
 *             thread_type:��ϵͳ�е����߳�
 *             exception_type:�쳣������
 */
int32 plat_power_fail_exception_info_set_etc(uint32 subsys_type, uint32 thread_type, uint32 exception_type)
{
    struct st_exception_info *pst_exception_data = NULL;
    uint64 flag;

    if (subsys_type >= SUBSYS_BOTTOM) {
        PS_PRINT_ERR("para subsys_type %u is error!\n", subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (((subsys_type == SUBSYS_WIFI) && (thread_type >= WIFI_THREAD_BOTTOM)) ||
        ((subsys_type == SUBSYS_BFGX) && (thread_type >= BFGX_THREAD_BOTTOM))) {
        PS_PRINT_ERR("para thread_type %u is error! subsys_type is %u\n", thread_type, subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (exception_type >= EXCEPTION_TYPE_BOTTOM) {
        PS_PRINT_ERR("para exception_type %u is error!\n", exception_type);
        return -EXCEPTION_FAIL;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
        PS_PRINT_WARNING("palt exception reset not enable!");
        return -EXCEPTION_FAIL;
    }

    spin_lock_irqsave(&pst_exception_data->exception_spin_lock, flag);

    if (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_IDLE) {
        pst_exception_data->subsys_type = subsys_type;
        pst_exception_data->thread_type = thread_type;
        pst_exception_data->excetion_type = exception_type;

        /* ��ǰ�쳣û�д������֮ǰ�������������µ��쳣 */
        atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_BUSY);
    } else {
        PS_PRINT_INFO("plat is processing exception! subsys=%d, exception type=%d\n",
                      pst_exception_data->subsys_type, pst_exception_data->excetion_type);
        spin_unlock_irqrestore(&pst_exception_data->exception_spin_lock, flag);
        return -EXCEPTION_FAIL;
    }

    /* ����ͳ����Ϣ */
    pst_exception_data->etype_info[exception_type].excp_cnt += 1;

    spin_unlock_irqrestore(&pst_exception_data->exception_spin_lock, flag);

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : plat_power_fail_process_done_etc
 * ��������  : bfgx openʧ��ʱ���쳣�������
 */
void plat_power_fail_process_done_etc(void)
{
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }

    atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);

    PS_PRINT_SUC("bfgx open fail process done\n");

    return;
}

static uint32 plat_is_dfr_enable(void)
{
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return OAL_FALSE;
    }

    if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
        PS_PRINT_INFO("plat exception reset not enable!");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * �� �� ��  : plat_exception_handler_etc
 * ��������  : ƽ̨host�쳣��������ں���������쳣��Ϣ���������쳣����work
 * �������  : subsys_type:�����쳣��������ϵͳ��WIFI����BFGX
 *             thread_type:��ϵͳ�е����߳�
 *             exception_type:�쳣������
 * �� �� ֵ  : �쳣�����ɹ�����0�����򷵻�1
 */
int32 plat_exception_handler_etc(uint32 subsys_type, uint32 thread_type, uint32 exception_type)
{
    struct st_exception_info *pst_exception_data = NULL;
    uint64 flag;
    int32 timeout;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    if (subsys_type >= SUBSYS_BOTTOM) {
        PS_PRINT_ERR("para subsys_type %u is error!\n", subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (((subsys_type == SUBSYS_WIFI) && (thread_type >= WIFI_THREAD_BOTTOM)) ||
        ((subsys_type == SUBSYS_BFGX) && (thread_type >= BFGX_THREAD_BOTTOM))) {
        PS_PRINT_ERR("para thread_type %u is error! subsys_type is %u\n", thread_type, subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (exception_type >= EXCEPTION_TYPE_BOTTOM) {
        PS_PRINT_ERR("para exception_type %u is error!\n", exception_type);
        return -EXCEPTION_FAIL;
    }

    if (plat_is_dfr_enable() == OAL_FALSE) {
        return EXCEPTION_SUCCESS;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }


    /* ����ֻ����spin lock����Ϊ�ú����ᱻ������ʱ�������ã�������ʱ�����������жϣ�������˯�� */
    spin_lock_irqsave(&pst_exception_data->exception_spin_lock, flag);
    if (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_IDLE) {
        pst_exception_data->subsys_type = subsys_type;
        pst_exception_data->thread_type = thread_type;
        pst_exception_data->excetion_type = exception_type;

        /* ��ǰ�쳣û�д������֮ǰ�������������µ��쳣 */
        atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_BUSY);
    } else {
        PS_PRINT_INFO("plat is processing exception! subsys=%d, exception type=%d\n",
                      pst_exception_data->subsys_type, pst_exception_data->excetion_type);
        spin_unlock_irqrestore(&pst_exception_data->exception_spin_lock, flag);
        return EXCEPTION_SUCCESS;
    }
    spin_unlock_irqrestore(&pst_exception_data->exception_spin_lock, flag);

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
    /* �ȴ�SSI������ɣ���ֹNOC */
    timeout = wait_for_ssi_idle_timeout(HI110X_DFR_WAIT_SSI_IDLE_MS);
    if (timeout <= 0) {
        PS_PRINT_ERR("[HI110X_DFR]wait for ssi idle failed\n");
        atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);
        return EXCEPTION_FAIL;
    }
    PS_PRINT_INFO("[HI110X_DFR]wait for ssi idle cost time:%dms\n", HI110X_DFR_WAIT_SSI_IDLE_MS - timeout);
#endif

    if (subsys_type == SUBSYS_BFGX) {
        /* timer time out �жϵ���ʱ������������ɾ�������� */
        if (exception_type != BFGX_BEATHEART_TIMEOUT) {
            pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
        }
    }

    pst_exception_data->etype_info[pst_exception_data->excetion_type].stime = ktime_get();
    PS_PRINT_WARNING("[HI110X_DFR]plat start doing exception! subsys=%d, exception type=%d\n",
                     pst_exception_data->subsys_type, pst_exception_data->excetion_type);
    plat_bbox_msg_hander(subsys_type, exception_type);

    oal_wake_lock_timeout(&pst_exception_data->plat_exception_rst_wlock, 10 * 1000); /* �����쳣������10�� */
    /* �����쳣����worker */
    queue_work(pst_exception_data->plat_exception_rst_workqueue, &pst_exception_data->plat_exception_rst_work);

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL(plat_exception_handler_etc);

/*
 * �� �� ��  : plat_exception_reset_work_etc
 * ��������  : ƽ̨host�쳣����work���жϲ����쳣��ϵͳ��������Ӧ�Ĵ���������
 *             ����������ȡmutex���Ա��Ⲣ��������
 * �� �� ֵ  : �쳣�����ɹ�����0�����򷵻�1
 */
void plat_exception_reset_work_etc(struct work_struct *work)
{
    int32 ret = -EXCEPTION_FAIL;
    struct st_exception_info *pst_exception_data = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    ktime_t trans_time;
    unsigned long long total_time;

    if (pm_data == NULL) {
        PS_PRINT_ERR("[HI110X_DFR]pm_data is NULL!\n");
        return;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("[HI110X_DFR]get exception info reference is error\n");
        return;
    }

    PS_PRINT_WARNING("[HI110X_DFR] enter plat_exception_reset_work_etc\n");

    mutex_lock(&pm_data->host_mutex);

    if (pst_exception_data->subsys_type == SUBSYS_WIFI) {
        if (wlan_is_shutdown_etc()) {
            PS_PRINT_WARNING("[HI110X_DFR]wifi is closed,stop wifi dfr\n");
            goto exit_exception;
        }
        ret = wifi_exception_handler_etc();
    } else {
        if (bfgx_is_shutdown_etc()) {
            PS_PRINT_WARNING("[HI110X_DFR]bfgx is closed,stop bfgx dfr\n");
            goto exit_exception;
        }
        ret = bfgx_exception_handler_etc();
    }

    pst_exception_data->etype_info[pst_exception_data->excetion_type].excp_cnt += 1;

    if (ret != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("[HI110X_DFR]plat execption recovery fail! subsys_type = [%u], exception_type = [%u]\n",
                     pst_exception_data->subsys_type, pst_exception_data->excetion_type);
        pst_exception_data->etype_info[pst_exception_data->excetion_type].fail_cnt++;
    } else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
        trans_time = ktime_sub(ktime_get(), pst_exception_data->etype_info[pst_exception_data->excetion_type].stime);
        total_time = (unsigned long long)ktime_to_ms(trans_time);
        if (pst_exception_data->etype_info[pst_exception_data->excetion_type].maxtime < total_time) {
            pst_exception_data->etype_info[pst_exception_data->excetion_type].maxtime = total_time;
            PS_PRINT_WARNING("[HI110X_DFR]time update:%llu,exception_type:%d \n",
                             total_time, pst_exception_data->excetion_type);
        }
#endif
        PS_PRINT_WARNING("[HI110X_DFR]plat execption recovery success, current time [%llu]ms, max time [%llu]ms\n",
                         total_time, pst_exception_data->etype_info[pst_exception_data->excetion_type].maxtime);
    }
    /* ÿ��dfr��ɣ���ʾ��ʷdfr������� */
    plat_print_dfr_info();

exit_exception:
    atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);
    mutex_unlock(&pm_data->host_mutex);
    return;
}

int32 is_subsystem_rst_enable(void)
{
#ifdef PLATFORM_DEBUG_ENABLE
    struct st_exception_info *pst_exception_data = NULL;
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return DFR_TEST_DISABLE;
    }
    PS_PRINT_INFO("#########is_subsystem_rst_enable:%d\n", pst_exception_data->subsystem_rst_en);
    return pst_exception_data->subsystem_rst_en;
#else
    return DFR_TEST_DISABLE;
#endif
}

/*
 * �� �� ��  : wifi_exception_handler_etc
 * ��������  : wifi�쳣������ں������ж�BFGN�Ƿ�򿪣����������õ�ϵͳ��λ���̣�
 *             BFGNû�������ȫϵͳ��λ����(��ϵͳ��λ��Ҫ�õ�uart)�������ϵͳ��λ
 *             ʧ�ܣ�������ȫϵͳ��λ��
 * �� �� ֵ  : �쳣�����ɹ�����0�����򷵻�1
 */
int32 wifi_exception_handler_etc(void)
{
    int32 ret = -EXCEPTION_FAIL;
    uint32 exception_type;
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    exception_type = pst_exception_data->excetion_type;

    /* ���bfgx�Ѿ��򿪣�ִ��wifi��ϵͳ��λ���̣�����ִ��wifiȫϵͳ��λ���� */
    if (!bfgx_is_shutdown_etc()) {
        PS_PRINT_INFO("bfgx is opened, start wifi subsystem reset!\n");
        bfgx_print_subsys_state();
        if (is_subsystem_rst_enable() != DFR_TEST_ENABLE) {
            ret = wifi_subsystem_reset_etc();
            if (ret != EXCEPTION_SUCCESS) {
                PS_PRINT_ERR("wifi subsystem reset failed, start wifi system reset!\n");
                ret = wifi_system_reset_etc();
            }
        } else {
            PS_PRINT_ERR("in debug mode,skip subsystem rst,do wifi system reset!\n");
            ret = wifi_system_reset_etc();
        }
    } else {
        PS_PRINT_INFO("bfgx is not opened, start wifi system reset!\n");
        ret = wifi_system_reset_etc();
    }

    if (ret != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("wifi execption recovery fail!\n");
        return ret;
    }

    if (pst_exception_data->wifi_callback->wifi_recovery_complete == NULL) {
        PS_PRINT_ERR("wifi recovery complete callback not regist\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->wifi_callback->wifi_recovery_complete();

    PS_PRINT_INFO("wifi recovery complete\n");

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : wifi_subsystem_reset_etc
 * ��������  : wifi��ϵͳ��λ��ͨ��uart��λ�⸴λWCPU,���¼���wifi firmware
 */
int32 wifi_subsystem_reset_etc(void)
{
    oal_int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    int32 l_subchip_type = get_hi110x_subchip_type();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    switch (l_subchip_type) {
        case BOARD_VERSION_HI1102:
            if (uart_reset_wcpu_etc() != EXCEPTION_SUCCESS) {
                PS_PRINT_ERR("uart reset wcpu failed\n");
                return -EXCEPTION_FAIL;
            }
            break;
        case BOARD_VERSION_HI1103:
        case BOARD_VERSION_HI1105:
            if (hi1103_wifi_subsys_reset() != EXCEPTION_SUCCESS) {
                PS_PRINT_ERR("wifi reset failed\n");
                return -EXCEPTION_FAIL;
            }
            break;
        default:
            PS_PRINT_ERR("wifi subsys reset error! subchip=%d\n", l_subchip_type);
            return -EXCEPTION_FAIL;
    }

    hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    if (firmware_download_function_etc(WIFI_CFG) != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("wifi firmware download failed\n");
        return -EXCEPTION_FAIL;
    }

    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        PS_PRINT_ERR("HCC_BUS_POWER_PATCH_LAUCH failed, ret=%d", ret);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WCPU_BOOTUP);

        return -EXCEPTION_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    // �·����ƻ�������deviceȥ
    hwifi_hcc_customize_h2d_data_cfg();
#endif

    plat_dfr_sysrst_type_cnt_inc(DFR_SINGLE_SYS_RST, SUBSYS_WIFI);

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WIFI_SUBSYS_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : wifi_system_reset_etc
 * ��������  : wifiȫϵͳ��λ����device�����ϵ磬bfgn��wifi firmware���¼��أ�
 *             �ָ�ȫϵͳ��λǰbfgn��ҵ��
 */
int32 wifi_system_reset_etc(void)
{
    int32 ret;
    uint64 timeleft;
    uint32 dfr_type = DFR_SINGLE_SYS_RST;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return BFGX_POWER_FAILED;
    }

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    /* �����ϵ磬firmware���¼��� */
    hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    if (!bfgx_is_shutdown_etc()) {
        dfr_type = DFR_ALL_SYS_RST;
        bfgx_print_subsys_state();
        del_timer_sync(&ps_core_d->ps_pm->pm_priv_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;

        ps_core_d->ps_pm->operate_beat_timer(BEAT_TIMER_DELETE);
        if (release_tty_drv_etc(ps_core_d->pm_data) != 0) {
            PS_PRINT_WARNING("wifi_system_reset_etc, release_tty_drv_etc fail, line = %d\n", __LINE__);
        }
    }

    PS_PRINT_INFO("wifi system reset, board power on\n");
    ret = board_power_reset(WLAN_POWER);
    if (ret) {
        PS_PRINT_ERR("board_power_reset wlan failed=%d\n", ret);
        return -EXCEPTION_FAIL;
    }

    if (firmware_download_function_etc(BFGX_AND_WIFI_CFG) != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("hi110x system power reset failed!\n");
        return -EXCEPTION_FAIL;
    }

    if (bfgx_is_shutdown_etc()) {
        hi1103_bfgx_disable();
    }

    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        PS_PRINT_ERR("HCC_BUS_POWER_PATCH_LAUCH failed ret=%d !!!!!!", ret);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WCPU_BOOTUP);

        return -EXCEPTION_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    // �·����ƻ�������deviceȥ
    hwifi_hcc_customize_h2d_data_cfg();
#endif
    if (!bfgx_is_shutdown_etc()) {
        INIT_COMPLETION(pm_data->dev_bootok_ack_comp);
        atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

        if (open_tty_drv_etc(ps_core_d->pm_data) != 0) {
            PS_PRINT_ERR("open_tty_drv_etc failed\n");
            return -EXCEPTION_FAIL;
        }

        if (wlan_pm_open_bcpu_etc() != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("wifi reset bcpu fail\n");
            if (release_tty_drv_etc(ps_core_d->pm_data) != 0) {
                PS_PRINT_WARNING("wifi_system_reset_etc, release_tty_drv_etc fail, line = %d\n", __LINE__);
            }
            atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_OPEN_BCPU);

            return -EXCEPTION_FAIL;
        }

        timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
        if (!timeleft) {
            PS_PRINT_ERR("wait bfgx boot ok timeout\n");
            atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP);

            return -FAILURE;
        }

        atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);

        bfgx_pm_feature_set_etc();

        /* �ָ�bfgxҵ��״̬ */
        if (bfgx_status_recovery_etc() != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("bfgx status revocery failed!\n");
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BFGX_RECOVERY);
            return -EXCEPTION_FAIL;
        }
    }

    plat_dfr_sysrst_type_cnt_inc(dfr_type, SUBSYS_WIFI);
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : wifi_status_recovery_etc
 * ��������  : ȫϵͳ��λ�Ժ󣬻ָ�wifiҵ����
 */
int32 wifi_status_recovery_etc(void)
{
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (wifi_subsystem_reset_etc() != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("wifi subsystem reset failed\n");
        return -EXCEPTION_FAIL;
    }

    if (pst_exception_data->wifi_callback->wifi_recovery_complete == NULL) {
        PS_PRINT_ERR("wifi recovery complete callback not regist\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->wifi_callback->wifi_recovery_complete();

    PS_PRINT_INFO("wifi recovery complete\n");

    return EXCEPTION_SUCCESS;
}

int32 wifi_open_bcpu_set_etc(uint8 enable)
{
    struct st_exception_info *pst_exception_data = NULL;
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EXCEPTION_FAIL;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (!enable) {
        PS_PRINT_INFO("wifi_open_bcpu_enable flag disable\n");
        pst_exception_data->wifi_open_bcpu_enable = false;
        return EXCEPTION_SUCCESS;
    }

    PS_PRINT_INFO("wifi_open_bcpu_enable flag enable\n");
    pst_exception_data->wifi_open_bcpu_enable = true;

    mutex_lock(&pm_data->host_mutex);

    if (release_tty_drv_etc(ps_core_d->pm_data) != SUCCESS) {
        PS_PRINT_ERR("close tty is err!");
        mutex_unlock(&pm_data->host_mutex);
        return -EXCEPTION_FAIL;
    }

    if (open_tty_drv_etc(ps_core_d->pm_data) != SUCCESS) {
        PS_PRINT_ERR("open tty is err!\n");
        mutex_unlock(&pm_data->host_mutex);
        return -EXCEPTION_FAIL;
    }

    if (wlan_pm_open_bcpu_etc() != OAL_SUCC) {
        mutex_unlock(&pm_data->host_mutex);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_OPEN_BCPU);

        return -EXCEPTION_FAIL;
    }

    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        PS_PRINT_ERR("wait BFGX boot ok timeout\n");
        mutex_unlock(&pm_data->host_mutex);
        return -EXCEPTION_FAIL;
    }

    mutex_unlock(&pm_data->host_mutex);

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL(wifi_open_bcpu_set_etc);

int32 wifi_device_reset(void)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    int32 l_subchip_type = get_hi110x_subchip_type();

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EXCEPTION_FAIL;
    }

    /* 1102ͨ��BCPU��λWCPU */
    if (l_subchip_type == BOARD_VERSION_HI1102) {
        PS_PRINT_INFO("reset wifi device by BCPU\n");
        /* If sdio transfer failed, reset wcpu first */
        if (!bfgx_is_shutdown_etc()) {
            if (prepare_to_visit_node_etc(ps_core_d) < 0) {
                PS_PRINT_ERR("prepare work FAIL\n");
                return -EXCEPTION_FAIL;
            }

            /* bcpu is power on, reset wcpu by bcpu */
            INIT_COMPLETION(ps_core_d->wait_wifi_opened);
            ps_uart_state_pre_etc(ps_core_d->tty);
            ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_DUMP_RESET_WCPU);
            timeleft = wait_for_completion_timeout(&ps_core_d->wait_wifi_opened, msecs_to_jiffies(WAIT_WIFI_OPEN_TIME));
            if (!timeleft) {
                ps_uart_state_dump_etc(ps_core_d->tty);
                PS_PRINT_ERR("wait wifi open ack timeout\n");
                post_to_visit_node_etc(ps_core_d);
                return -EXCEPTION_FAIL;
            } else {
                PS_PRINT_INFO("reset wifi by uart sucuess\n");
                post_to_visit_node_etc(ps_core_d);
                return EXCEPTION_SUCCESS;
            }
        } else {
            PS_PRINT_INFO("bfgx did't opened, repower wifi directly\n");
            return -EXCEPTION_FAIL;
        }
    } else { /* 1103 ͨ��w_en��λ */
        PS_PRINT_INFO("reset wifi device by w_en gpio\n");
        hi1103_wifi_disable();
        if (hi1103_wifi_enable()) {
            PS_PRINT_ERR("hi1103 wifi enable failed\n");
            return -EXCEPTION_FAIL;
        }
        return EXCEPTION_SUCCESS;
    }
}

/*
 * �� �� ��  : wifi_exception_mem_dump_etc
 * ��������  : ȫϵͳ��λ��firmware���¼��ص�ʱ�򣬵���deviceָ����ַ���ڴ�
 * �������  : pst_mem_dump_info  : ��Ҫ��ȡ���ڴ���Ϣ
 *             count              : ��Ҫ��ȡ���ڴ�����
 */
int32 wifi_exception_mem_dump_etc(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count, oal_int32 excep_type)
{
    int32 ret;
    int32 reset_flag = 0;
    hcc_bus_dev *pst_bus_dev = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    if (excep_type != WIFI_DEVICE_PANIC && excep_type != WIFI_TRANS_FAIL) {
        PS_PRINT_ERR("unsupport exception type :%d\n", excep_type);
        return -EXCEPTION_FAIL;
    }

    mutex_lock(&pm_data->host_mutex);

    if (wlan_is_shutdown_etc()) {
        PS_PRINT_ERR("[E]dfr ignored, wifi shutdown before memdump\n");
        mutex_unlock(&pm_data->host_mutex);
        return -EXCEPTION_FAIL;
    }

retry:
    if (excep_type != WIFI_DEVICE_PANIC) {
        if (wifi_device_reset() != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("wifi device reset fail, exception type :%d\n", excep_type);
            mutex_unlock(&pm_data->host_mutex);
            return -EXCEPTION_FAIL;
        }
        reset_flag = 1;
        hcc_change_state_exception_etc();
    }

    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);

    if (pst_bus_dev != NULL) {
        ret = hcc_bus_reinit(pst_bus_dev->cur_bus);
    } else {
        ret = -OAL_EIO;
    }

    if (ret != OAL_SUCC) {
        PS_PRINT_ERR("wifi mem dump:current bus reinit failed, ret=[%d]\n", ret);
        if (!reset_flag) {
            PS_PRINT_ERR("reinit failed, try to reset wifi\n");
            excep_type = WIFI_TRANS_FAIL;
            goto retry;
        }
        mutex_unlock(&pm_data->host_mutex);
        return -EXCEPTION_FAIL;
    }

    ret = wifi_device_mem_dump(pst_mem_dump_info, count);
    hcc_bus_disable_state(pm_data->pst_wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
    if (ret < 0) {
        PS_PRINT_ERR("wifi_device_mem_dump failed, ret=[%d]\n", ret);
        if (!reset_flag) {
            PS_PRINT_ERR("memdump failed, try to reset wifi\n");
            excep_type = WIFI_TRANS_FAIL;
            goto retry;
        }
        mutex_unlock(&pm_data->host_mutex);
        return -EXCEPTION_FAIL;
    }

    mutex_unlock(&pm_data->host_mutex);

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL(wifi_exception_mem_dump_etc);

/*
 * �� �� ��  : wifi_exception_work_submit_etc
 * ��������  : �쳣�ָ����������ӿ�
 */
int32 wifi_exception_work_submit_etc(uint32 wifi_excp_type)
{
    oal_int32 ret;
    struct st_exception_info *pst_exception_data = NULL;
    hcc_bus *hi_bus = hcc_get_current_110x_bus();

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (oal_work_is_busy(&exception_info_etc->wifi_excp_worker)) {
        PS_PRINT_WARNING("WIFI DFR %pF Worker is Processing:doing wifi excp_work...need't submit\n",
                         (void *)exception_info_etc->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else if (oal_work_is_busy(&exception_info_etc->wifi_excp_recovery_worker)) {
        PS_PRINT_WARNING(
            "WIFI DFR %pF Recovery_Worker is Processing:doing wifi wifi_excp_recovery_worker ...need't submit\n",
            (void *)exception_info_etc->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else if ((atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE) &&
               (pst_exception_data->subsys_type == SUBSYS_WIFI)) {
        PS_PRINT_WARNING("WIFI DFR %pF Recovery_Worker is Processing:doing  plat wifi recovery ...need't submit\n",
                         (void *)exception_info_etc->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else {
        ret = wifi_exception_mem_dump_etc(hi_bus->mem_info, hi_bus->mem_size, hi_bus->bus_excp_type);
        if (ret < 0) {
            PS_PRINT_ERR("Panic File Save Failed!");
        } else {
            PS_PRINT_INFO("Panic File Save OK!");
        }

        /*Ϊ����û�п���DFR  �������Ҳ��mem dump, ���ڴ˴����ж�*/
        if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
            PS_PRINT_INFO("palt exception reset not enable!");
            return EXCEPTION_SUCCESS;
        }

        PS_PRINT_ERR("WiFi DFR %pF Worker Submit, excp_type[%d]\n",
                     (void *)exception_info_etc->wifi_excp_worker.func, wifi_excp_type);
        exception_info_etc->wifi_excp_type = wifi_excp_type;
        hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_ALL);
        queue_work(exception_info_etc->wifi_exception_workqueue, &exception_info_etc->wifi_excp_worker);
    }
#else
    PS_PRINT_WARNING("Geting WIFI DFR, but _PRE_WLAN_FEATURE_DFR not open!");
#endif  // _PRE_WLAN_FEATURE_DFR
    return OAL_SUCC;
}
EXPORT_SYMBOL(exception_info_etc);
EXPORT_SYMBOL(wifi_exception_work_submit_etc);

oal_workqueue_stru *wifi_get_exception_workqueue_etc(oal_void)
{
    if (exception_info_etc == NULL) {
        return NULL;
    }
    return exception_info_etc->wifi_exception_workqueue;
}
EXPORT_SYMBOL(wifi_get_exception_workqueue_etc);

int32 prepare_to_recv_bfgx_stack_etc(uint32 len)
{
    if (recvd_block_count_etc > BFGX_MEM_DUMP_BLOCK_COUNT - 1) {
        PS_PRINT_ERR("bfgx mem dump have recvd [%d] blocks\n", recvd_block_count_etc);
        return -EXCEPTION_FAIL;
    }

    if (bfgx_mem_dump_etc[recvd_block_count_etc].exception_mem_addr == NULL) {
        bfgx_mem_dump_etc[recvd_block_count_etc].exception_mem_addr = (uint8 *)OS_VMALLOC_GFP(len);
        if (bfgx_mem_dump_etc[recvd_block_count_etc].exception_mem_addr == NULL) {
            PS_PRINT_ERR("prepare mem to recv bfgx stack failed\n");
            return -EXCEPTION_FAIL;
        } else {
            bfgx_mem_dump_etc[recvd_block_count_etc].recved_size = 0;
            bfgx_mem_dump_etc[recvd_block_count_etc].total_size = len;
            bfgx_mem_dump_etc[recvd_block_count_etc].file_name = bfgx_mem_file_name_etc[recvd_block_count_etc];
            PS_PRINT_INFO("prepare mem [%d] to recv bfgx stack\n", len);
        }
    }

    return EXCEPTION_SUCCESS;
}

int32 free_bfgx_stack_dump_mem_etc(void)
{
    uint32 i = 0;

    for (i = 0; i < BFGX_MEM_DUMP_BLOCK_COUNT; i++) {
        if (bfgx_mem_dump_etc[i].exception_mem_addr != NULL) {
            OS_MEM_VFREE(bfgx_mem_dump_etc[i].exception_mem_addr);
            bfgx_mem_dump_etc[i].recved_size = 0;
            bfgx_mem_dump_etc[i].total_size = 0;
            bfgx_mem_dump_etc[i].file_name = NULL;
            bfgx_mem_dump_etc[i].exception_mem_addr = NULL;
        }
    }

    recvd_block_count_etc = 0;

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : bfgx_exception_handler_etc
 * ��������  : bfgx�쳣������ں����������쳣���ͣ����ò�ͬ�Ĵ�������
 */
int32 bfgx_exception_handler_etc(void)
{
    int32 ret = -EXCEPTION_FAIL;
    uint32 exception_type;
    struct st_exception_info *pst_exception_data = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("pst_exception_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    cancel_work_sync(&pm_data->send_allow_sleep_work);
    del_timer_sync(&pm_data->bfg_timer);
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    if (pst_exception_data->excetion_type == BFGX_BEATHEART_TIMEOUT) {
        /* bfgx�쳣����ɾ��bfg timer������timer����ֹ�ظ�����bfgx�쳣 */
        pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
    }

    exception_type = pst_exception_data->excetion_type;

    /* ioctl�������쳣ִ���̸߳�λ����,��ǰִ��ϵͳȫ��λ */
    if (exception_type == BFGX_TIMER_TIMEOUT || exception_type == BFGX_ARP_TIMEOUT) {
        ret = bfgx_subthread_reset_etc();
    } else {
        /* �쳣�ָ�֮ǰ��������ƽ̨�����ջ�����ܱ�֤һ���ɹ�����Ϊ��ʱuart���ܲ�ͨ */
        bfgx_dump_stack_etc();

        ret = bfgx_subsystem_reset_etc();
    }

    if (ret != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("bfgx exception recovery fail!, exception_type = [%u]\n", exception_type);
        del_timer_sync(&pm_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;

        pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
        return ret;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : bfgx_subthread_reset_etc
 * ��������  : bfgx���̸߳�λ������ͨ��uart�·���λ���߳�������ȴ���λ
 *             �ɹ�ACK���յ�ACK���쳣������ɣ��������BFGX��ϵͳ��λ����
 */
int32 bfgx_subthread_reset_etc(void)
{
    /* ����ִ��оƬȫϵͳ��λ����֤оƬ�µ� */
    return bfgx_system_reset_etc();
}

int32 wifi_reset_bfgx_etc(void)
{
    uint64 timeleft;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    if (wlan_pm_shutdown_bcpu_cmd_etc() != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("wifi shutdown bcpu fail\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_CLOSE_BCPU);

        return -EXCEPTION_FAIL;
    }

    INIT_COMPLETION(pm_data->dev_bootok_ack_comp);

    if (wlan_pm_open_bcpu_etc() != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("wifi reset bcpu fail\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_OPEN_BCPU);

        return -EXCEPTION_FAIL;
    }

    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        PS_PRINT_ERR("wait bfgx boot ok timeout\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP);

        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : __bfgx_subsystem_reset_etc
 * ��������  : bfgx��ϵͳ��λ����������ú�����˵��wifi�ѿ���ͨ��sdio�·���λ
 *             �⸴λBCPU�������bfgx��ϵͳ��λ����ϵͳ��λ����Ժ���Ҫ�ָ�
 *             ��λǰbfgin��ҵ��
 * �� �� ֵ  : �ɹ�����0�����򷵻�1
 */
int32 __bfgx_subsystem_reset_etc(void)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    int32 l_subchip_type = get_hi110x_subchip_type();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EXCEPTION_FAIL;
    }

    if (ps_chk_bfg_active_etc(ps_core_d) == false) {
        PS_PRINT_ERR("bfgx no subsys is opened\n");
        return EXCEPTION_SUCCESS;
    }

    if (release_tty_drv_etc(ps_core_d->pm_data) != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("close tty is err!\n");
        if (!is_tty_open(ps_core_d->pm_data)) {
            return -EXCEPTION_FAIL;
        }
    }

    if (open_tty_drv_etc(ps_core_d->pm_data) != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("open tty is err!\n");
        return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

    switch (l_subchip_type) {
        case BOARD_VERSION_HI1102:
            if (wifi_reset_bfgx_etc() != EXCEPTION_SUCCESS) {
                PS_PRINT_ERR("wifi reset bfgx fail\n");
                atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
                return -EXCEPTION_FAIL;
            }
            break;
        case BOARD_VERSION_HI1103:
        case BOARD_VERSION_HI1105:
            INIT_COMPLETION(pm_data->dev_bootok_ack_comp);
            hi1103_bfgx_subsys_reset();
            timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp,
                                                   msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
            if (!timeleft) {
                PS_PRINT_ERR("wait bfgx boot ok timeout\n");
                atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
                CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                                     CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP);

                return -EXCEPTION_FAIL;
            }
            break;
        default:
            PS_PRINT_ERR("bfgx subsys reset error! subchip=%d\n", l_subchip_type);
            atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
            return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);

    bfgx_pm_feature_set_etc();

    if (bfgx_status_recovery_etc() != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("bfgx recovery status failed\n");

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BFGX_RECOVERY);
        return -EXCEPTION_FAIL;
    }

    plat_dfr_sysrst_type_cnt_inc(DFR_SINGLE_SYS_RST, SUBSYS_BFGX);

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                         CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BFGX_SUBSYS_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : bfgx_subsystem_reset_etc
 * ��������  : bfgx��ϵͳ��λ���������wifi�򿪣���ͨ��sdio�·���λ�⸴λBCPU���
 *             ����bfgx��ϵͳ��λ��wifiû��������bfgxȫϵͳ��λ���̡������ϵͳ
 *             ��λ���ܽ��쳣�ָ�Ҳ�����ȫϵͳ��λ���̡���ϵͳ��λ����Ժ���Ҫ
 *             �ָ���λǰbfgin��ҵ��
 * �� �� ֵ  : �ɹ�����0�����򷵻�1
 */
int32 bfgx_subsystem_reset_etc(void)
{
    if (!wlan_is_shutdown_etc()) {
        PS_PRINT_INFO("wifi is opened, start bfgx subsystem reset!\n");

        if (is_subsystem_rst_enable() != DFR_TEST_ENABLE) {
            if (__bfgx_subsystem_reset_etc() != EXCEPTION_SUCCESS) {
                PS_PRINT_ERR("bfgx subsystem reset failed, start bfgx system reset!\n");
                return bfgx_system_reset_etc();
            }
        } else {
            PS_PRINT_INFO("in debug mode,skip subsystem rst,do bfgx system reset!\n");
            return bfgx_system_reset_etc();
        }

        return EXCEPTION_SUCCESS;
    } else {
        PS_PRINT_INFO("wifi is not opened, start bfgx system reset!\n");
        return bfgx_system_reset_etc();
    }
}

int32 bfgx_power_reset_etc(void)
{
    int32 ret;
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    if (release_tty_drv_etc(ps_core_d->pm_data) != 0) {
        PS_PRINT_WARNING("bfgx_power_reset_etc, release_tty_drv_etc fail\n");
    }

    PS_PRINT_INFO("bfgx system reset, board power on\n");
    ret = board_power_reset(BFGX_POWER);
    if (ret) {
        PS_PRINT_ERR("board_power_reset bfgx failed=%d\n", ret);
        return -EXCEPTION_FAIL;
    }

    if (open_tty_drv_etc(ps_core_d->pm_data) != 0) {
        PS_PRINT_WARNING("bfgx_power_reset_etc, open_tty_drv_etc fail\n");
    }

    INIT_COMPLETION(pm_data->dev_bootok_ack_comp);
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

    if (firmware_download_function_etc(BFGX_CFG) != EXCEPTION_SUCCESS) {
        hcc_bus_disable_state(pm_data->pst_wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
        PS_PRINT_ERR("bfgx power reset failed!\n");
        atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
        return -EXCEPTION_FAIL;
    }
    hcc_bus_disable_state(pm_data->pst_wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);

    if (wlan_is_shutdown_etc()) {
        hi1103_wifi_disable();
    }

    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        PS_PRINT_ERR("wait bfgx boot ok timeout\n");
        atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP);

        return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);

    bfgx_pm_feature_set_etc();

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : bfgx_system_reset_etc
 * ��������  : bfginȫϵͳ��λ��������device�����ϵ磬wifi��bfgin firmware����
 *            ���أ��ָ�wifi���ָ�bfginҵ��
 * �� �� ֵ  : �ɹ�����0�����򷵻�-1
 */
int32 bfgx_system_reset_etc(void)
{
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (!wlan_is_shutdown_etc()) {
        if (pst_exception_data->wifi_callback->notify_wifi_to_recovery != NULL) {
            PS_PRINT_INFO("notify wifi bfgx start to power reset\n");
            pst_exception_data->wifi_callback->notify_wifi_to_recovery();
        }
    }

    /* �����ϵ磬firmware���¼��� */
    if (bfgx_power_reset_etc() != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("bfgx power reset failed!\n");
        return -EXCEPTION_FAIL;
    }

    if (bfgx_status_recovery_etc() != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("bfgx status revocery failed!\n");

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                             CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BFGX_RECOVERY);
        return -EXCEPTION_FAIL;
    }

    if (!wlan_is_shutdown_etc()) {
        if (wifi_status_recovery_etc() != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("wifi status revocery failed!\n");

            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WIFI_RECOVERY);

            return -EXCEPTION_FAIL;
        }
        plat_dfr_sysrst_type_cnt_inc(DFR_ALL_SYS_RST, SUBSYS_BFGX);
    } else {
        plat_dfr_sysrst_type_cnt_inc(DFR_SINGLE_SYS_RST, SUBSYS_BFGX);
    }

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_GNSS, CHR_LAYER_DRV,
                         CHR_GNSS_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : bfgx_recv_dev_mem_etc
 * ��������  : ����bfgx�쳣ʱ��device�ϱ���ջ�ڴ�
 * �������  : buf_ptr :uart���յ���ռ�ڴ�bufer�׵�ַ
 *             count   :buffer����
 */
int32 bfgx_recv_dev_mem_etc(uint8 *buf_ptr, uint16 count)
{
    struct st_exception_mem_info *pst_mem_info = NULL;
    uint32 offset;

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EFAIL;
    }

    if (recvd_block_count_etc >= BFGX_MEM_DUMP_BLOCK_COUNT) {
        PS_PRINT_ERR("recvd_block_count_etc=[%d]\n", recvd_block_count_etc);
        return -EFAIL;
    }

    pst_mem_info = &(bfgx_mem_dump_etc[recvd_block_count_etc]);

    if (pst_mem_info->exception_mem_addr == NULL) {
        PS_PRINT_ERR("mem addr is null, recvd_block_count_etc=[%d]\n", recvd_block_count_etc);
        return -EXCEPTION_FAIL;
    }

    offset = pst_mem_info->recved_size;
    if ((uint32)offset + (uint32)count > pst_mem_info->total_size) {
        PS_PRINT_ERR("outof buf total size, recved size is [%d], curr recved size is [%d], total size is [%d]\n",
                     offset, count, pst_mem_info->total_size);
        return -EXCEPTION_FAIL;
    } else {
        PS_PRINT_DBG("cpy stack size [%d] to exception mem\n", count);
        if (memcpy_s(pst_mem_info->exception_mem_addr + offset, pst_mem_info->total_size - offset,
                     buf_ptr, count) != EOK) {
            PS_PRINT_ERR("memcpy failed\n ");
            return -EXCEPTION_FAIL;
        }

        pst_mem_info->recved_size += count;
    }

    if (pst_mem_info->recved_size == pst_mem_info->total_size) {
        recvd_block_count_etc++;
        PS_PRINT_INFO("mem block [%d] recvd done\n", recvd_block_count_etc);
    }

    return EXCEPTION_SUCCESS;
}
#ifndef HI110X_HAL_MEMDUMP_ENABLE

int32 bfgx_store_stack_mem_to_file_etc(void)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    char filename[EXCEPTION_FILE_NAME_LEN] = {0};
    uint32 i;
    mm_segment_t fs;
    struct st_exception_mem_info *pst_mem_info = NULL;

    for (i = 0; i < BFGX_MEM_DUMP_BLOCK_COUNT; i++) {
        pst_mem_info = &(bfgx_mem_dump_etc[i]);
        if (pst_mem_info->exception_mem_addr == NULL) {
            PS_PRINT_ERR("mem addr is null, i=[%d]\n", i);
            continue;
        }
        memset_s(filename, sizeof(filename), 0, sizeof(filename));
        if (snprintf_s(filename, sizeof(filename), sizeof(filename) - 1, BFGX_DUMP_PATH "/%s_%s.bin",
                   UART_STORE_BFGX_STACK, pst_mem_info->file_name) < 0) {
            PS_PRINT_ERR("filename str format err.\n");
        }
        /* ���ļ���׼��������յ����ڴ� */
        fp = filp_open(filename, O_RDWR | O_CREAT, 0664);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_ERR("create file error,fp = 0x%p,filename:%s\n", fp, filename);
            continue;
        }

        /* �����յ����ڴ�д�뵽�ļ��� */
        fs = get_fs();
        set_fs(KERNEL_DS);
        PS_PRINT_INFO("pos = %d\n", (int)fp->f_pos);
        vfs_write(fp, pst_mem_info->exception_mem_addr, pst_mem_info->recved_size, &fp->f_pos);
        set_fs(fs);

        filp_close(fp, NULL);
    }

    /* send cmd to oam_hisi to rotate file */
    plat_send_rotate_cmd_2_app_etc(CMD_READM_BFGX_UART);

    free_bfgx_stack_dump_mem_etc();

    return EXCEPTION_SUCCESS;
}
#endif

/*
 * �� �� ��  : bfgx_dump_stack_etc
 * ��������  : ������ʱʱ������ͨ��uart��ջ������֤һ���ܳɹ�����Ϊ��ʱuart���ܲ�ͨ
 */
void bfgx_dump_stack_etc(void)
{
    uint64 timeleft;
    uint32 exception_type;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }

    exception_type = pst_exception_data->excetion_type;

    if (exception_type != BFGX_LASTWORD_PANIC) {
        return;
    }

    if (unlikely(pst_exception_data->ps_plat_d == NULL)) {
        PS_PRINT_ERR("pst_exception_data->ps_plat_d is NULL\n");
        return;
    }
    ps_core_d = pst_exception_data->ps_plat_d->core_data;

    INIT_COMPLETION(pst_exception_data->wait_read_bfgx_stack);

    /* �ȴ���ջ������� */
    timeleft = wait_for_completion_timeout(&pst_exception_data->wait_read_bfgx_stack,
                                           msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        atomic_set(&pst_exception_data->is_memdump_runing, 0);
        PS_PRINT_ERR("read bfgx stack failed!\n");
    } else {
        PS_PRINT_INFO("read bfgx stack success!\n");
    }
#ifndef HI110X_HAL_MEMDUMP_ENABLE
    plat_wait_last_rotate_finish_etc();

    bfgx_store_stack_mem_to_file_etc();
#endif
    return;
}

int32 prepare_to_recv_wifi_mem_etc(void)
{
    uint32 malloc_mem_len;
    uint32 index;

    PS_PRINT_INFO("%s\n", __func__);

    index = recvd_wifi_block_index_etc;
    if (index >= UART_WIFI_MEM_DUMP_BOTTOM) {
        PS_PRINT_ERR("recvd_wifi_block_index_etc is error [%d]\n", index);
        return -EXCEPTION_FAIL;
    }

    if (uart_wifi_mem_dump_etc[index].exception_mem_addr == NULL) {
        malloc_mem_len = uart_read_wifi_mem_info_etc[index].total_size;
        uart_wifi_mem_dump_etc[index].exception_mem_addr = (uint8 *)OS_VMALLOC_GFP(malloc_mem_len);
        if (uart_wifi_mem_dump_etc[index].exception_mem_addr == NULL) {
            PS_PRINT_ERR("prepare mem to recv wifi mem failed\n");
            return -EXCEPTION_FAIL;
        } else {
            uart_wifi_mem_dump_etc[index].recved_size = 0;
            uart_wifi_mem_dump_etc[index].total_size = malloc_mem_len;
            uart_wifi_mem_dump_etc[index].file_name = NULL;
            PS_PRINT_INFO("prepare mem [%d] to recv wifi mem, index = [%d]\n", malloc_mem_len, index);
        }
    }

    return EXCEPTION_SUCCESS;
}

int32 free_uart_read_wifi_mem_etc(void)
{
    uint32 index;

    PS_PRINT_INFO("%s\n", __func__);

    index = recvd_wifi_block_index_etc;
    if (index >= UART_WIFI_MEM_DUMP_BOTTOM) {
        PS_PRINT_ERR("recvd_wifi_block_index_etc is error [%d]\n", index);
        return -EXCEPTION_FAIL;
    }

    if (uart_wifi_mem_dump_etc[index].exception_mem_addr != NULL) {
        OS_MEM_VFREE(uart_wifi_mem_dump_etc[index].exception_mem_addr);
        uart_wifi_mem_dump_etc[index].recved_size = 0;
        uart_wifi_mem_dump_etc[index].total_size = 0;
        uart_wifi_mem_dump_etc[index].exception_mem_addr = NULL;
        PS_PRINT_INFO("vfree uart read wifi mem [%d] success\n", index);
    }

    recvd_wifi_block_index_etc = UART_WIFI_MEM_DUMP_BOTTOM;

    return EXCEPTION_SUCCESS;
}

int32 uart_recv_wifi_mem_etc(uint8 *buf_ptr, uint16 count)
{
    struct st_exception_mem_info *pst_mem_info = NULL;
    uint32 offset;
    uint32 index;

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EFAIL;
    }

    index = recvd_wifi_block_index_etc;
    if (index >= UART_WIFI_MEM_DUMP_BOTTOM) {
        PS_PRINT_ERR("recvd_wifi_block_index_etc [%d] is error\n", index);
        return -EXCEPTION_FAIL;
    }

    pst_mem_info = &(uart_wifi_mem_dump_etc[index]);

    if (pst_mem_info->exception_mem_addr == NULL) {
        PS_PRINT_ERR("mem addr is null, recvd_block_count_etc=[%d]\n", index);
        return -EXCEPTION_FAIL;
    }

    offset = pst_mem_info->recved_size;
    if (offset + count > pst_mem_info->total_size) {
        PS_PRINT_ERR(
            "outof buf total size, index=[%d], recved size is [%d], curr recved size is [%d], total size is [%d]\n",
            index, offset, count, pst_mem_info->total_size);
        return -EXCEPTION_FAIL;
    } else {
        PS_PRINT_INFO("cpy wifi mem size [%d] to recv buffer\n", count);
        if (memcpy_s(pst_mem_info->exception_mem_addr + offset, pst_mem_info->total_size - offset,
                     buf_ptr, count) != EOK) {
            PS_PRINT_ERR("memcopy error\n ");
        }
        pst_mem_info->recved_size += count;
        PS_PRINT_INFO("index [%d] have recved size is [%d], total size is [%d]\n",
                      index, pst_mem_info->recved_size, pst_mem_info->total_size);
    }

    return EXCEPTION_SUCCESS;
}

int32 __store_wifi_mem_to_file_etc(void)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    char filename[EXCEPTION_FILE_NAME_LEN] = {0};
    mm_segment_t fs;
    uint32 index;
    uint32 i;
    uint32 block_count;
    uint8 *block_file_name = NULL;
    uint32 block_size;
    uint32 offset = 0;
    struct st_exception_mem_info *pst_mem_info = NULL;

#ifdef PLATFORM_DEBUG_ENABLE
    struct timeval tv;
    struct rtc_time tm = {0};
    do_gettimeofday(&tv);
    rtc_time_to_tm(tv.tv_sec, &tm);
    PS_PRINT_INFO("%4d-%02d-%02d  %02d:%02d:%02d\n",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);  /* ת���ɵ�ǰʱ�� */
#endif
    PS_PRINT_INFO("%s\n", __func__);

    index = recvd_wifi_block_index_etc;
    if (index >= UART_WIFI_MEM_DUMP_BOTTOM) {
        PS_PRINT_ERR("recvd_wifi_block_index_etc [%d] is error\n", index);
        return -EXCEPTION_FAIL;
    }

    pst_mem_info = &(uart_wifi_mem_dump_etc[index]);
    if (pst_mem_info->exception_mem_addr == NULL) {
        PS_PRINT_ERR("mem addr is null, recvd_wifi_block_index_etc=[%d]", index);
        return -EXCEPTION_FAIL;
    }

    block_count = uart_read_wifi_mem_info_etc[index].block_count;

    fs = get_fs();
    set_fs(KERNEL_DS);
    for (i = 0; i < block_count; i++) {
        block_size = uart_read_wifi_mem_info_etc[index].block_info[i].size;
        block_file_name = uart_read_wifi_mem_info_etc[index].block_info[i].file_name;
        if (snprintf_s(filename, sizeof(filename), sizeof(filename) - 1, BFGX_DUMP_PATH "/%s_%s.bin",
                   UART_STORE_WIFI_MEM, block_file_name) < 0) {
            PS_PRINT_ERR("filename str format err.\n");
        }
        /* ���ļ���׼��������յ����ڴ� */
        fp = filp_open(filename, O_RDWR | O_CREAT, 0664);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_ERR("create file error,fp = 0x%p,filename:%s\n", fp, filename);
            set_fs(fs);
            return -EXCEPTION_FAIL;
        }

        /* �����յ����ڴ�д�뵽�ļ��� */
        PS_PRINT_INFO("pos = %d\n", (int)fp->f_pos);
        vfs_write(fp, pst_mem_info->exception_mem_addr + offset, block_size, &fp->f_pos);
        filp_close(fp, NULL);
        offset += block_size;
    }
    set_fs(fs);

    return EXCEPTION_SUCCESS;
}

void store_wifi_mem_to_file_work_etc(struct work_struct *work)
{
    struct st_exception_info *pst_exception_data = NULL;

    PS_PRINT_INFO("%s\n", __func__);

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }

    __store_wifi_mem_to_file_etc();

    free_uart_read_wifi_mem_etc();

    complete(&pst_exception_data->wait_uart_read_wifi_mem);

    return;
}

void store_wifi_mem_to_file_etc(void)
{
    struct st_exception_info *pst_exception_data = NULL;

    PS_PRINT_INFO("%s\n", __func__);

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }

    queue_work(pst_exception_data->plat_exception_rst_workqueue, &pst_exception_data->uart_store_wifi_mem_to_file_work);
    return;
}

int32 uart_halt_wcpu_etc(void)
{
    uint64 timeleft;
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *pst_bus_dev = NULL;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    PS_PRINT_INFO("%s\n", __func__);

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    INIT_COMPLETION(pst_exception_data->wait_uart_halt_wcpu);
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_HALT_WCPU);
    timeleft = wait_for_completion_timeout(&pst_exception_data->wait_uart_halt_wcpu,
                                           msecs_to_jiffies(UART_HALT_WCPU_TIMEOUT));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait uart halt wcpu ack timeout\n");
        return -ETIMEDOUT;
    }

    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev == NULL) {
        PS_PRINT_ERR("110x dev is null\n");
        return -ENODEV;
    }

    pst_bus = HDEV_TO_HBUS(pst_bus_dev);

    if (pst_bus == NULL) {
        PS_PRINT_ERR("110x bus is null\n");
        return -ENODEV;
    }

    hcc_bus_wakeup_request(pst_bus);
    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : exception_bcpu_dump_recv_etc
 * ��������  : �ص����ݴ�������
 */
int32 exception_bcpu_dump_recv_etc(uint8 *str, oal_netbuf_stru *netbuf)
{
    exception_bcpu_dump_header *cmd_header = {0};
    struct st_exception_info *pst_exception_data = NULL;
    int ret;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (st_bcpu_dump_buff_etc.mem_addr == NULL) {
        PS_PRINT_ERR("st_bcpu_dump_buff_etc addr is null\n");
        return -EXCEPTION_FAIL;
    }

    cmd_header = (exception_bcpu_dump_header *)(str);
    ret = memcpy_s(st_bcpu_dump_buff_etc.mem_addr + st_bcpu_dump_buff_etc.data_len,
                   st_bcpu_dump_buff_etc.data_limit - st_bcpu_dump_buff_etc.data_len,
                   str + sizeof(exception_bcpu_dump_header), cmd_header->men_len);
    if (ret != EOK) {
        PS_PRINT_ERR("memcopy_s error, destlen=%d, srclen=%d\n ",
                     st_bcpu_dump_buff_etc.data_limit - st_bcpu_dump_buff_etc.data_len, cmd_header->men_len);
        return -EXCEPTION_FAIL;
    }
    st_bcpu_dump_buff_etc.data_len += cmd_header->men_len;

    complete(&pst_exception_data->wait_sdio_d2h_dump_ack);

    return EXCEPTION_SUCCESS;
}

int32 free_buffer_and_netbuf_etc(void)
{
    if (st_bcpu_dump_buff_etc.mem_addr != NULL) {
        OS_MEM_VFREE(st_bcpu_dump_buff_etc.mem_addr);
        st_bcpu_dump_buff_etc.data_limit = 0;
        st_bcpu_dump_buff_etc.data_len = 0;
        st_bcpu_dump_buff_etc.mem_addr = NULL;
    }

    if (st_bcpu_dump_netbuf_etc != NULL) {
        st_bcpu_dump_netbuf_etc = NULL;
    }
    return EXCEPTION_SUCCESS;
}

int32 sdio_halt_bcpu_etc(void)
{
    int32 ret;
    uint64 timeleft;
    int i;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    // wake up wifi
    for (i = 0; i < WAKEUP_RETRY_TIMES; i++) {
        ret = wlan_pm_wakeup_dev_etc();
        if (ret == OAL_SUCC) {
            break;
        }
    }

    if (ret != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("wlan wake up fail!");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return -OAL_FAIL;
    }

    ret = hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_HALT_BCPU);
    if (ret == 0) {
        /* �ȴ�deviceִ������ */
        timeleft = wait_for_completion_timeout(&pst_wlan_pm->st_halt_bcpu_done,
                                               msecs_to_jiffies(WLAN_HALT_BCPU_TIMEOUT));
        if (timeleft == 0) {
            PS_PRINT_ERR("sdio halt bcpu failed!\n");
            hcc_tx_transfer_unlock(hcc_get_110x_handler());
            return -OAL_FAIL;
        }
    }
    PS_PRINT_INFO("halt bcpu sucess!");
    hcc_tx_transfer_unlock(hcc_get_110x_handler());
    return OAL_SUCC;
}

int32 allocate_data_save_buffer_etc(uint32 len)
{
    // ��ʱbuff����,���ڴ�������
    st_bcpu_dump_buff_etc.mem_addr = OS_VMALLOC_GFP(len);
    if (st_bcpu_dump_buff_etc.mem_addr == NULL) {
        PS_PRINT_ERR("st_bcpu_dump_buff_etc allocate fail!\n");
        return -EXCEPTION_FAIL;
    }
    st_bcpu_dump_buff_etc.data_limit = len;
    st_bcpu_dump_buff_etc.data_len = 0;
    return EXCEPTION_SUCCESS;
}

int32 allocate_send_netbuf_etc(uint32 len)
{
    st_bcpu_dump_netbuf_etc = hcc_netbuf_alloc(len);
    if (st_bcpu_dump_netbuf_etc == NULL) {
        PS_PRINT_ERR("st_bcpu_dump_netbuf_etc allocate fail !\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : dump_header_init_etc
 * ��������  : ��ʼ������CMD
 */
int32 dump_header_init_etc(exception_bcpu_dump_header *header, uint32 align_type, uint32 addr, uint32 send_len)
{
    /* cmd ��ʼ�� */
    header->align_type = align_type;
    header->start_addr = addr;
    header->men_len = send_len;
    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : init_hcc_head_and_send_etc
 * ��������  : ��ʼ��hcc����header
 */
int32 init_hcc_head_and_send_etc(struct hcc_transfer_param st_hcc_transfer_param,
                                 struct st_exception_info *pst_exception_data, uint32 wait_time)
{
    uint64 timeleft;
    int32 l_ret;
    // ����
    INIT_COMPLETION(pst_exception_data->wait_sdio_d2h_dump_ack);
    l_ret = hcc_tx_etc(hcc_get_110x_handler(), st_bcpu_dump_netbuf_etc, &st_hcc_transfer_param);
    if (l_ret != OAL_SUCC) {
        PS_PRINT_ERR("send tx  failed!\n");
        oal_netbuf_free(st_bcpu_dump_netbuf_etc);
        st_bcpu_dump_netbuf_etc = NULL;
        return -EXCEPTION_FAIL;
    }
    /* �ȴ�SDIO��������� */
    timeleft = wait_for_completion_timeout(&pst_exception_data->wait_sdio_d2h_dump_ack, msecs_to_jiffies(wait_time));
    if (!timeleft) {
        PS_PRINT_ERR("sdio read  failed!\n");
        st_bcpu_dump_netbuf_etc = NULL;
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

int32 sdio_get_data_pre(struct st_exception_info **exception_data)
{
    get_exception_info_reference_etc(exception_data);
    if (*exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (sdio_halt_bcpu_etc() != OAL_SUCC) {
        PS_PRINT_ERR("halt bcpu error!\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

int32 sdio_get_and_save_data_etc(exception_bcpu_dump_msg *sdio_read_info, uint32 count)
{
    uint32 header_len;
    uint32 netbuf_len;
    uint32 send_len;
    uint32 index;
    uint32 i = 0;
    uint32 buffer_len;
    uint32 send_total_len;
    uint32 align_type;
    uint32 max_buff_len;
    int32 error = EXCEPTION_SUCCESS;
    int8 filename[EXCEPTION_FILE_NAME_LEN] = {0};

    mm_segment_t fs = {0};
    OS_KERNEL_FILE_STRU *fp = {0};

    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct st_exception_info *pst_exception_data = NULL;
    exception_bcpu_dump_header dump_header = {0};

    PS_PRINT_INFO("%s\n", __func__);

    if (sdio_get_data_pre(&pst_exception_data) != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }

    header_len = sizeof(exception_bcpu_dump_header);

    fs = get_fs();
    set_fs(KERNEL_DS);
    for (i = 0; i < count; i++) {
        index = 0;
        if (snprintf_s(filename, sizeof(filename), sizeof(filename) - 1, BFGX_DUMP_PATH "/%s_%s.bin",
                   SDIO_STORE_BFGX_REGMEM, sdio_read_info[i].file_name) < 0) {
            PS_PRINT_ERR("filename str format err.\n");
        }
        /* ׼���ļ��ռ� */
        fp = filp_open(filename, O_RDWR | O_CREAT, 0664);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_ERR("create file error,fp = 0x%p\n", fp);
            set_fs(fs);
            return -EXCEPTION_FAIL;
        }
        PS_PRINT_INFO("%s is dumping...,pos = %d\n", sdio_read_info[i].file_name, (int)fp->f_pos);

        // prepare data buffer
        send_total_len = sdio_read_info[i].men_len;
        align_type = sdio_read_info[i].align_type;
        max_buff_len = (align_type == ALIGN_1_BYTE) ? DUMP_BCPU_MEM_BUFF_LEN : DUMP_BCPU_REG_BUFF_LEN;
        buffer_len = (send_total_len > max_buff_len) ? max_buff_len : sdio_read_info[i].men_len;

        if (allocate_data_save_buffer_etc(buffer_len) != EXCEPTION_SUCCESS) {
            error = -EXCEPTION_FAIL;
            goto exit;
        }

        // send cmd and save data
        while (index < send_total_len) {
            send_len = send_total_len - index;

            if (align_type == ALIGN_1_BYTE) {
                // dump mem set
                if (send_len > DUMP_BCPU_MEM_BUFF_LEN) {
                    send_len = DUMP_BCPU_MEM_BUFF_LEN;
                }
                hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_OAM, DUMP_MEM, 0, 0, DATA_HI_QUEUE);
            } else {  // dump reg set
                if (send_len > DUMP_BCPU_REG_BUFF_LEN) {
                    send_len = DUMP_BCPU_REG_BUFF_LEN;
                }
                hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_OAM, DUMP_REG, 0, 0, DATA_HI_QUEUE);
            }

            netbuf_len = header_len + send_len;
            if (allocate_send_netbuf_etc(netbuf_len) != EXCEPTION_SUCCESS) {
                error = -EXCEPTION_FAIL;
                goto exit;
            }

            dump_header_init_etc(&dump_header, sdio_read_info[i].align_type,
                                 sdio_read_info[i].start_addr + index, send_len);
            if (st_bcpu_dump_netbuf_etc == NULL) {
                goto exit;
            }

            if (memcpy_s(oal_netbuf_put(st_bcpu_dump_netbuf_etc, netbuf_len), netbuf_len,
                         &dump_header, sizeof(exception_bcpu_dump_header)) != EOK) {
                PS_PRINT_ERR("memcpy_s error, destlen=%ud, srclen=%lud\n ",
                             netbuf_len, sizeof(exception_bcpu_dump_header));
                goto exit;
            }

            // ����
            if (init_hcc_head_and_send_etc(st_hcc_transfer_param, pst_exception_data, WIFI_DUMP_BCPU_TIMEOUT) !=
                EXCEPTION_SUCCESS) {
                error = -EXCEPTION_FAIL;
                goto exit;
            }

            vfs_write(fp, st_bcpu_dump_buff_etc.mem_addr, st_bcpu_dump_buff_etc.data_len, &fp->f_pos);

            index += send_len;

            // prepare for next data
            st_bcpu_dump_buff_etc.data_len = 0;
            st_bcpu_dump_netbuf_etc = NULL;
        }
        filp_close(fp, NULL);
        free_buffer_and_netbuf_etc();
    }
    set_fs(fs);
    return EXCEPTION_SUCCESS;
exit:
    filp_close(fp, NULL);
    set_fs(fs);
    free_buffer_and_netbuf_etc();
    complete(&pst_exception_data->wait_sdio_d2h_dump_ack);

    return error;
}

int32 debug_sdio_read_bfgx_reg_and_mem_etc(uint32 which_mem)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);

    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error!\n");
        return -EXCEPTION_FAIL;
    }

    if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
        PS_PRINT_ERR("plat dfr is not enable ,can not dump info");
        return -EXCEPTION_FAIL;
    }

    if (bfgx_is_shutdown_etc()) {
        PS_PRINT_WARNING("bfgx is off can not dump bfgx msg !\n");
        return -EXCEPTION_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EXCEPTION_FAIL;
    }

    PS_PRINT_INFO("sdio dump bfgx msg begin\n");

    // prepare the wlan state
    if (wlan_is_shutdown_etc()) {
        PS_PRINT_WARNING("wifi is closed, can not dump bcpu info!");
        return -EXCEPTION_FAIL;
    } else {
        PS_PRINT_INFO("wifi is open!\n");
    }

    // ȥ��exception����,halt bcpu������DFR
    pst_exception_data->exception_reset_enable = PLAT_EXCEPTION_DISABLE;

    // dump data
    switch (which_mem) {
        case BFGX_PUB_REG:
            sdio_get_and_save_data_etc(sdio_read_bcpu_pub_reg_info_etc, BFGX_PUB_REG_NUM);
            break;
        case BFGX_PRIV_REG:
            sdio_get_and_save_data_etc(sdio_read_bcpu_priv_reg_info_etc, BFGX_PRIV_REG_NUM);
            break;
        case BFGX_MEM:
            sdio_get_and_save_data_etc(sdio_read_bcpu_mem_info_etc, BFGX_SHARE_RAM_NUM);
            break;
        case SDIO_BFGX_MEM_DUMP_BOTTOM:
            sdio_get_and_save_data_etc(sdio_read_all_etc,
                                       sizeof(sdio_read_all_etc) / sizeof(exception_bcpu_dump_msg));
            break;
        default:
            PS_PRINT_WARNING("input param error , which_mem is %d\n", which_mem);
            pst_exception_data->exception_reset_enable = PLAT_EXCEPTION_ENABLE;
            return -EXCEPTION_FAIL;
    }

    PS_PRINT_INFO("dump complete, recovery begin\n");

    // ʹ��DFR, recovery
    pst_exception_data->exception_reset_enable = PLAT_EXCEPTION_ENABLE;
    plat_exception_handler_etc(SUBSYS_BFGX, THREAD_IDLE, SDIO_DUMPBCPU_FAIL);

    return EXCEPTION_SUCCESS;
}

int32 uart_read_wifi_mem_etc(uint32 which_mem)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    PS_PRINT_INFO("%s\n", __func__);

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    if (which_mem >= UART_WIFI_MEM_DUMP_BOTTOM) {
        PS_PRINT_ERR("param which_mem [%d] is err\n", which_mem);
        return -EINVAL;
    }

    recvd_wifi_block_index_etc = which_mem;

    if (prepare_to_recv_wifi_mem_etc() < 0) {
        PS_PRINT_ERR("prepare mem to recv wifi mem fail, which_mem = [%d]\n", which_mem);
        recvd_wifi_block_index_etc = UART_WIFI_MEM_DUMP_BOTTOM;
        return -EINVAL;
    }

    INIT_COMPLETION(pst_exception_data->wait_uart_read_wifi_mem);
    ps_uart_state_pre_etc(ps_core_d->tty);
    ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, uart_read_wifi_mem_info_etc[which_mem].cmd);
    timeleft = wait_for_completion_timeout(&pst_exception_data->wait_uart_read_wifi_mem,
                                           msecs_to_jiffies(UART_READ_WIFI_MEM_TIMEOUT));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait uart read wifi mem [%d] timeout\n", which_mem);
        free_uart_read_wifi_mem_etc();
        return -ETIMEDOUT;
    }

    return EXCEPTION_SUCCESS;
}

int32 debug_uart_read_wifi_mem_etc(uint32 ul_lock)
{
    uint32 i;
    uint32 read_mem_succ_count = 0;
    struct ps_core_s *ps_core_d = NULL;
    int32 l_subchip_type = get_hi110x_subchip_type();
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    PS_PRINT_INFO("%s\n", __func__);

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EINVAL;
    }

    if (l_subchip_type == BOARD_VERSION_HI1102) {
        if (ul_lock) {
            mutex_lock(&pm_data->host_mutex);
        }

        if (prepare_to_visit_node_etc(ps_core_d) != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("prepare work FAIL\n");
            goto fail_return;
        }

        if (uart_halt_wcpu_etc() != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("uart halt wcpu fail!\n");
            post_to_visit_node_etc(ps_core_d);
            goto fail_return;
        }

        for (i = 0; i < UART_WIFI_MEM_DUMP_BOTTOM; i++) {
            if (uart_read_wifi_mem_etc(i) != EXCEPTION_SUCCESS) {
                PS_PRINT_ERR("uart read wifi mem [%d] fail!", i);
                break;
            }
            read_mem_succ_count++;
        }

        post_to_visit_node_etc(ps_core_d);

        if (ul_lock) {
            mutex_unlock(&pm_data->host_mutex);
        }

        return EXCEPTION_SUCCESS;
    } else {
        PS_PRINT_ERR("hi1103, ignore uart read wifi mem\n");
        return EXCEPTION_SUCCESS;
    }

fail_return:
    if (ul_lock) {
        mutex_unlock(&pm_data->host_mutex);
    }

    return -EXCEPTION_FAIL;
}

int32 bfgx_reset_cmd_send_etc(uint32 subsys)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    ret = ps_push_skb_queue_etc(ps_core_d, ast_bfgx_reset_msg_etc[subsys].cmd,
                                ast_bfgx_reset_msg_etc[subsys].len,
                                bfgx_rx_queue_etc[subsys]);
    if (ret != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("push %s reset cmd to skb fail\n", bfgx_subsys_name_etc[subsys]);
        return -EXCEPTION_FAIL;
    }

    wake_up_interruptible(&pst_bfgx_data->rx_wait);

    return EXCEPTION_SUCCESS;
}

/*
 * �� �� ��  : bfgx_status_recovery_etc
 * ��������  : ȫϵͳ��λ�Ժ󣬻ָ�bfginҵ����
 */
int32 bfgx_status_recovery_etc(void)
{
    uint32 i;
    struct st_exception_info *pst_exception_data = NULL;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return -EXCEPTION_FAIL;
    }

    if (prepare_to_visit_node_etc(ps_core_d) != EXCEPTION_SUCCESS) {
        PS_PRINT_ERR("prepare work FAIL\n");
        return -EXCEPTION_FAIL;
    }

    for (i = 0; i < BFGX_BUTT; i++) {
        pst_bfgx_data = &ps_core_d->bfgx_info[i];
        if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
            continue;
        }

        ps_kfree_skb_etc(ps_core_d, bfgx_rx_queue_etc[i]);

        if (bfgx_open_cmd_send_etc(i) != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("bfgx open cmd fail\n");
            post_to_visit_node_etc(ps_core_d);
            return -EXCEPTION_FAIL;
        }

        if (bfgx_reset_cmd_send_etc(i) != EXCEPTION_SUCCESS) {
            PS_PRINT_ERR("bfgx reset cmd send fail\n");
            post_to_visit_node_etc(ps_core_d);
            return -EXCEPTION_FAIL;
        }
    }

    post_to_visit_node_etc(ps_core_d);

    /* ������ʹ�� */
    PS_PRINT_INFO("exception: set debug beat flag to 1\n");
    pst_exception_data->debug_beat_flag = 1;

    return EXCEPTION_SUCCESS;
}

int32 is_bfgx_exception_etc(void)
{
    struct st_exception_info *pst_exception_data = NULL;
    int32 is_exception;
    uint64 flag;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return 0;
    }

    spin_lock_irqsave(&pst_exception_data->exception_spin_lock, flag);
    if (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_BUSY) {
        is_exception = PLAT_EXCEPTION_RESET_BUSY;
    } else {
        is_exception = PLAT_EXCEPTION_RESET_IDLE;
    }
    spin_unlock_irqrestore(&pst_exception_data->exception_spin_lock, flag);

    return is_exception;
}

int32 plat_bfgx_exception_rst_register_etc(struct ps_plat_s *data)
{
    struct st_exception_info *pst_exception_data = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (data == NULL) {
        PS_PRINT_ERR("para data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->ps_plat_d = data;

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL_GPL(plat_bfgx_exception_rst_register_etc);

int32 plat_wifi_exception_rst_register_etc(void *data)
{
    struct st_exception_info *pst_exception_data = NULL;
    struct st_wifi_dfr_callback *pst_wifi_callback = NULL;

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (data == NULL) {
        PS_PRINT_ERR("param data is null\n");
        return -EXCEPTION_FAIL;
    }

    /* wifi�쳣�ص�����ע�� */
    pst_wifi_callback = (struct st_wifi_dfr_callback *)data;
    pst_exception_data->wifi_callback = pst_wifi_callback;

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL_GPL(plat_wifi_exception_rst_register_etc);

/*
 * �� �� ��  : plat_exception_reset_init_etc
 * ��������  : ƽ̨�쳣����ģ���ʼ������
 */
int32 plat_exception_reset_init_etc(void)
{
    struct st_exception_info *p_exception_data = NULL;

    p_exception_data = (struct st_exception_info *)kzalloc(sizeof(struct st_exception_info), GFP_KERNEL);
    if (p_exception_data == NULL) {
        PS_PRINT_ERR("kzalloc p_exception_data is failed!\n");
        return -EXCEPTION_FAIL;
    }

    p_exception_data->wifi_callback = NULL;

    p_exception_data->subsys_type = SUBSYS_BOTTOM;
    p_exception_data->thread_type = BFGX_THREAD_BOTTOM;
    p_exception_data->excetion_type = EXCEPTION_TYPE_BOTTOM;

    p_exception_data->exception_reset_enable = PLAT_EXCEPTION_DISABLE;
    p_exception_data->subsystem_rst_en = DFR_TEST_DISABLE;

    p_exception_data->ps_plat_d = NULL;

    atomic_set(&p_exception_data->bfgx_beat_flag, BFGX_NOT_RECV_BEAT_INFO);
    atomic_set(&p_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);

    /* ��ʼ���쳣����workqueue��work */
    p_exception_data->plat_exception_rst_workqueue = create_singlethread_workqueue("plat_exception_reset_queue");
    INIT_WORK(&p_exception_data->plat_exception_rst_work, plat_exception_reset_work_etc);
    oal_wake_lock_init(&p_exception_data->plat_exception_rst_wlock, "hi11xx_excep_rst_wlock");
    INIT_WORK(&p_exception_data->uart_store_wifi_mem_to_file_work, store_wifi_mem_to_file_work_etc);

    /* ��ʼ������timer */
    init_timer(&p_exception_data->bfgx_beat_timer);
    p_exception_data->bfgx_beat_timer.function = bfgx_beat_timer_expire_etc;
    p_exception_data->bfgx_beat_timer.expires = jiffies + BFGX_BEAT_TIME * HZ;
    p_exception_data->bfgx_beat_timer.data = 0;

    /* ��ʼ���쳣���������� */
    spin_lock_init(&p_exception_data->exception_spin_lock);

    /* ��ʼ��bfgx��ջ����� */
    init_completion(&p_exception_data->wait_read_bfgx_stack);
    /* ��ʼ��sdio��ȡbcpu����� */
    init_completion(&p_exception_data->wait_sdio_d2h_dump_ack);

    /* ����ʹ�õı�����ʼ�� */
    p_exception_data->debug_beat_flag = 1;
    atomic_set(&p_exception_data->is_memdump_runing, 0);
    p_exception_data->wifi_open_bcpu_enable = false;

    init_completion(&p_exception_data->wait_uart_read_wifi_mem);
    init_completion(&p_exception_data->wait_uart_halt_wcpu);

    exception_info_etc = p_exception_data;

    /* ��ʼ��dump�ļ�����ģ�� */
    plat_exception_dump_file_rotate_init_etc();

    hisi_conn_rdr_init();

    PS_PRINT_SUC("plat exception reset init success\n");

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL_GPL(plat_exception_reset_init_etc);

/*
 * �� �� ��  : plat_exception_reset_exit_etc
 * ��������  : ƽ̨�쳣����ģ���˳���������
 */
int32 plat_exception_reset_exit_etc(void)
{
    struct st_exception_info *p_exception_data = NULL;

    p_exception_data = exception_info_etc;
    if (p_exception_data == NULL) {
        PS_PRINT_ERR("exception_info_etc is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    destroy_workqueue(p_exception_data->plat_exception_rst_workqueue);
    del_timer_sync(&p_exception_data->bfgx_beat_timer);

    oal_wake_lock_exit(&p_exception_data->plat_exception_rst_wlock);

    kfree(p_exception_data);
    exception_info_etc = NULL;

    PS_PRINT_SUC("plat exception reset exit success\n");

    hisi_conn_rdr_exit();

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL_GPL(plat_exception_reset_exit_etc);

#ifdef PLATFORM_DEBUG_ENABLE
int32 is_dfr_test_en(enum excp_test_cfg_em excp_cfg)
{
    if (excp_cfg >= EXCP_TEST_CFG_BOTT) {
        return DFR_TEST_DISABLE;
    }

    if (gst_excp_test_cfg[excp_cfg] == DFR_TEST_ENABLE) {
        gst_excp_test_cfg[excp_cfg] = DFR_TEST_DISABLE;
        return DFR_TEST_ENABLE;
    }
    return DFR_TEST_DISABLE;
}
EXPORT_SYMBOL_GPL(is_dfr_test_en);
void set_excp_test_en(enum excp_test_cfg_em excp_cfg)
{
    if (excp_cfg >= EXCP_TEST_CFG_BOTT) {
        return;
    }

    gst_excp_test_cfg[excp_cfg] = DFR_TEST_ENABLE;
}
#endif

#ifdef HI110X_HAL_MEMDUMP_ENABLE

void plat_exception_dump_file_rotate_init_etc(void)
{
    init_waitqueue_head(&bcpu_memdump_cfg_etc.dump_type_wait);
    skb_queue_head_init(&bcpu_memdump_cfg_etc.dump_type_queue);
    skb_queue_head_init(&bcpu_memdump_cfg_etc.quenue);
    init_waitqueue_head(&wcpu_memdump_cfg_etc.dump_type_wait);
    skb_queue_head_init(&wcpu_memdump_cfg_etc.dump_type_queue);
    skb_queue_head_init(&wcpu_memdump_cfg_etc.quenue);
    PS_PRINT_INFO("plat exception dump file rotate init success\n");
}

void excp_memdump_quenue_clear_etc(memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;
    while ((skb = skb_dequeue(&memdump_t->quenue)) != NULL) {
        kfree_skb(skb);
    }
}
int32 bfgx_memdump_quenue_clear_etc(void)
{
    PS_PRINT_DBG("bfgx_memdump_quenue_clear_etc\n");
    excp_memdump_quenue_clear_etc(&bcpu_memdump_cfg_etc);
    return 0;
}
void wifi_memdump_quenue_clear_etc(void)
{
    PS_PRINT_DBG("wifi_memdump_quenue_clear_etc\n");
    excp_memdump_quenue_clear_etc(&wcpu_memdump_cfg_etc);
}
void bfgx_memdump_finish_etc(void)
{
    bcpu_memdump_cfg_etc.is_working = 0;
}
void wifi_memdump_finish_etc(void)
{
    wcpu_memdump_cfg_etc.is_working = 0;
}
/*
 * Prototype    : plat_send_rotate_cmd_2_app_etc
 * Description  : driver send rotate cmd to app for rotate file
 */
int32 plat_excp_send_rotate_cmd_2_app_etc(uint32 which_dump, memdump_info_t *memdump_info)
{
    struct sk_buff *skb = NULL;

    if (which_dump >= CMD_DUMP_BUFF) {
        PS_PRINT_WARNING("which dump:%d error\n", which_dump);
        return -EINVAL;
    }
    if (skb_queue_len(&memdump_info->dump_type_queue) > MEMDUMP_ROTATE_QUEUE_MAX_LEN) {
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
    skb_queue_tail(&memdump_info->dump_type_queue, skb);
    PS_PRINT_INFO("save rotate cmd [%d] in queue\n", which_dump);

    wake_up_interruptible(&memdump_info->dump_type_wait);

    return 0;
}

int32 notice_hal_memdump_etc(memdump_info_t *memdump_t, uint32 which_dump)
{
    PS_PRINT_FUNCTION_NAME;
    if (memdump_t->is_working) {
        PS_PRINT_ERR("is doing memdump\n");
        return -1;
    }
    excp_memdump_quenue_clear_etc(memdump_t);
    plat_excp_send_rotate_cmd_2_app_etc(which_dump, memdump_t);
    memdump_t->is_working = 1;
    return 0;
}
int32 bfgx_notice_hal_memdump_etc(void)
{
    return notice_hal_memdump_etc(&bcpu_memdump_cfg_etc, CMD_READM_BFGX_UART);
}
int32 wifi_notice_hal_memdump_etc(void)
{
    return notice_hal_memdump_etc(&wcpu_memdump_cfg_etc, CMD_READM_WIFI_SDIO);
}
int32 excp_memdump_queue_etc(uint8 *buf_ptr, uint16 count, memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;

    PS_PRINT_DBG("[send] len:%d\n", count);
    if (!memdump_t->is_working) {
        PS_PRINT_ERR("excp_memdump_queue_etc not allow\n");
        return -EINVAL;
    }
    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }
    if (in_atomic() || in_softirq() || in_interrupt() || irqs_disabled()) {
        skb = alloc_skb(count, GFP_ATOMIC);
    } else {
        skb = alloc_skb(count, GFP_KERNEL);
    }

    if (skb == NULL) {
        PS_PRINT_ERR("can't allocate mem for new debug skb, len=%d\n", count);
        return -EINVAL;
    }

    if (memcpy_s(skb_tail_pointer(skb), count, buf_ptr, count) != EOK) {
        PS_PRINT_ERR("memcpy_s failed\n");
    }
    skb_put(skb, count);
    skb_queue_tail(&memdump_t->quenue, skb);
    PS_PRINT_DBG("[excp_memdump_queue_etc]qlen:%d,count:%d\n", memdump_t->quenue.qlen, count);
    return 0;
}
int32 bfgx_memdump_enquenue_etc(uint8 *buf_ptr, uint16 count)
{
    return excp_memdump_queue_etc(buf_ptr, count, &bcpu_memdump_cfg_etc);
}
int32 wifi_memdump_enquenue_etc(uint8 *buf_ptr, uint16 count)
{
    return excp_memdump_queue_etc(buf_ptr, count, &wcpu_memdump_cfg_etc);
}
#endif