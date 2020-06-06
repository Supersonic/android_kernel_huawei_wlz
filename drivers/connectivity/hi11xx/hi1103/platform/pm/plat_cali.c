

/* ͷ�ļ����� */
#include "plat_firmware.h"
#include "plat_cali.h"
#include "plat_debug.h"
#include "plat_type.h"
#include "board.h"
#include "plat_pm.h"
#include "hisi_ini.h"
#include <linux/version.h>
#include "securec.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
#define HISI_NVRAM_SUPPORT
#endif

/* ����У׼���ݵ�buf */
oal_uint8 *CaliDataBuf_etc = NULL; /* 03 wifiУ׼���� */
oal_uint8 netdev_is_open_etc = OAL_FALSE;
oal_uint32 cali_update_channel_info = 0;

/* add for hi1103 bfgx */
struct completion cali_recv_done;
oal_uint8 *BfgxCaliDataBuf = NULL;

/* ���岻�ܳ���BFGX_BT_CUST_INI_SIZE/4 (128) */
bfgx_ini_cmd bfgx_ini_config_cmd[BFGX_BT_CUST_INI_SIZE / 4] = {
    { "bt_maxpower",                       0 },
    { "bt_edrpow_offset",                  0 },
    { "bt_blepow_offset",                  0 },
    { "bt_cali_txpwr_pa_ref_num",          0 },
    { "bt_cali_txpwr_pa_ref_band1",        0 },
    { "bt_cali_txpwr_pa_ref_band2",        0 },
    { "bt_cali_txpwr_pa_ref_band3",        0 },
    { "bt_cali_txpwr_pa_ref_band4",        0 },
    { "bt_cali_txpwr_pa_ref_band5",        0 },
    { "bt_cali_txpwr_pa_ref_band6",        0 },
    { "bt_cali_txpwr_pa_ref_band7",        0 },
    { "bt_cali_txpwr_pa_ref_band8",        0 },
    { "bt_cali_txpwr_pa_fre1",             0 },
    { "bt_cali_txpwr_pa_fre2",             0 },
    { "bt_cali_txpwr_pa_fre3",             0 },
    { "bt_cali_txpwr_pa_fre4",             0 },
    { "bt_cali_txpwr_pa_fre5",             0 },
    { "bt_cali_txpwr_pa_fre6",             0 },
    { "bt_cali_txpwr_pa_fre7",             0 },
    { "bt_cali_txpwr_pa_fre8",             0 },
    { "bt_cali_bt_tone_amp_grade",         0 },
    { "bt_rxdc_band",                      0 },
    { "bt_dbb_scaling_saturation",         0 },
    { "bt_productline_upccode_search_max", 0 },
    { "bt_productline_upccode_search_min", 0 },
    { "bt_dynamicsarctrl_bt",              0 },
    { "bt_powoffsbt",                      0 },
    { "bt_elna_2g_bt",                     0 },
    { "bt_rxisobtelnabyp",                 0 },
    { "bt_rxgainbtelna",                   0 },
    { "bt_rxbtextloss",                    0 },
    { "bt_elna_on2off_time_ns",            0 },
    { "bt_elna_off2on_time_ns",            0 },
    { "bt_hipower_mode",                   0 },
    { "bt_fem_control",                    0 },
    { "bt_feature_32k_clock",              0 },
    { "bt_feature_log",                    0 },
    { "bt_cali_swtich_all",                0 },
    { "bt_ant_num_bt",                     0 },
    { "bt_power_level_control",            0 },
    { "bt_country_code",                   0 },
    { "bt_reserved1",                      0 },
    { "bt_reserved2",                      0 },
    { "bt_reserved3",                      0 },
    { "bt_dedicated_antenna",              0 },
    { "bt_reserved5",                      0 },
    { "bt_reserved6",                      0 },
    { "bt_reserved7",                      0 },
    { "bt_reserved8",                      0 },
    { "bt_reserved9",                      0 },
    { "bt_reserved10",                     0 },

    { NULL, 0 }
};

/* ���岻�ܳ���BFGX_BT_CUST_INI_SIZE/4 (128) */
int32 bfgx_cust_ini_data[BFGX_BT_CUST_INI_SIZE / 4] = {0};

/*
 * �� �� ��  : get_cali_count_etc
 * ��������  : ����У׼�Ĵ������״ο���У׼ʱΪ0���Ժ����
 * �������  : uint32 *count:���ú�������У׼�����ĵ�ַ
 * �������  : count:�Կ����������Ѿ�У׼�Ĵ���
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
oal_int32 get_cali_count_etc(oal_uint32 *count)
{
    if (count == NULL) {
        PS_PRINT_ERR("count is NULL\n");
        return -EFAIL;
    }

    *count = cali_update_channel_info;

    PS_PRINT_WARNING("cali update info is [%d]\r\n", cali_update_channel_info);

    return SUCC;
}

#ifdef HISI_NVRAM_SUPPORT
/*
 * �� �� ��  : bfgx_nv_data_init
 * ��������  : bt У׼NV��ȡ
 */
oal_int32 bfgx_nv_data_init(void)
{
    int32 l_ret;
    int8 *pst_buf;
    uint32 ul_len;

    oal_uint8 bt_cal_nvram_tmp[OAL_BT_NVRAM_DATA_LENGTH];

    l_ret = read_conf_from_nvram_etc(bt_cal_nvram_tmp, OAL_BT_NVRAM_DATA_LENGTH,
                                     OAL_BT_NVRAM_NUMBER, OAL_BT_NVRAM_NAME);
    if (l_ret != INI_SUCC) {
        PS_PRINT_ERR("bfgx_nv_data_init::BT read NV error!");
        // last byte of NV ram is used to mark if NV ram is failed to read.
        bt_cal_nvram_tmp[OAL_BT_NVRAM_DATA_LENGTH - 1] = OAL_TRUE;
    } else {
        // last byte of NV ram is used to mark if NV ram is failed to read.
        bt_cal_nvram_tmp[OAL_BT_NVRAM_DATA_LENGTH - 1] = OAL_FALSE;
    }

    pst_buf = bfgx_get_nv_data_buf(&ul_len);
    if (pst_buf == NULL) {
        PS_PRINT_ERR("get bfgx nv buf fail!");
        return INI_FAILED;
    }
    if (ul_len < OAL_BT_NVRAM_DATA_LENGTH) {
        PS_PRINT_ERR("get bfgx nv buf size %d, NV data size is %d!", ul_len, OAL_BT_NVRAM_DATA_LENGTH);
        return INI_FAILED;
    }

    l_ret = memcpy_s(pst_buf, ul_len, bt_cal_nvram_tmp, OAL_BT_NVRAM_DATA_LENGTH);
    if (l_ret != EOK) {
        PS_PRINT_ERR("bfgx_nv_data_init FAILED!");
        return INI_FAILED;
    }
    PS_PRINT_INFO("bfgx_nv_data_init SUCCESS");
    return INI_SUCC;
}
#endif

/*
 * �� �� ��  : hi1103_get_bfgx_cali_data
 * ��������  : ���ر���bfgxУ׼���ݵ��ڴ��׵�ַ�Լ�����
 * �������  : uint8  *buf:���ú�������bfgxУ׼���ݵ��׵�ַ
 *             uint32 *len:���ú�������bfgxУ׼�����ڴ泤�ȵĵ�ַ
 *             uint32 buf_len:buf�ĳ���
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 hi1103_get_bfgx_cali_data(oal_uint8 *buf, oal_uint32 *len, oal_uint32 buf_len)
{
    oal_uint32 bfgx_cali_data_len;

    PS_PRINT_INFO("%s\n", __func__);

    if (unlikely(buf == NULL)) {
        PS_PRINT_ERR("buf is NULL\n");
        return -EFAIL;
    }

    if (unlikely(len == NULL)) {
        PS_PRINT_ERR("len is NULL\n");
        return -EFAIL;
    }

    if (unlikely(BfgxCaliDataBuf == NULL)) {
        PS_PRINT_ERR("BfgxCaliDataBuf is NULL\n");
        return -EFAIL;
    }

#ifdef HISI_NVRAM_SUPPORT
    if (bfgx_nv_data_init() != OAL_SUCC) {
        PS_PRINT_ERR("bfgx nv data init fail!\n");
    }
#endif

    bfgx_cali_data_len = sizeof(bfgx_cali_data_stru);
    if (buf_len < bfgx_cali_data_len) {
        PS_PRINT_ERR("bfgx cali buf len[%d] is smaller than struct size[%d]\n", buf_len, bfgx_cali_data_len);
        return -EFAIL;
    }

    memcpy_s(buf, buf_len, BfgxCaliDataBuf, bfgx_cali_data_len);
    *len = bfgx_cali_data_len;

    return SUCC;
}

/*
 * �� �� ��  : get_bfgx_cali_data_etc
 * ��������  : ���ر���bfgxУ׼���ݵ��ڴ��׵�ַ�Լ�����
 * �������  : uint8  *buf:���ú�������bfgxУ׼���ݵ��׵�ַ
 *             uint32 *len:���ú�������bfgxУ׼�����ڴ泤�ȵĵ�ַ
 *             uint32 buf_len:buf�ĳ���
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 get_bfgx_cali_data_etc(oal_uint8 *buf, oal_uint32 *len, oal_uint32 buf_len)
{
    oal_int32 l_subchip_type = get_hi110x_subchip_type();

    switch (l_subchip_type) {
        case BOARD_VERSION_HI1103:
        case BOARD_VERSION_HI1105:
            return hi1103_get_bfgx_cali_data(buf, len, buf_len);
        default:
            PS_PRINT_ERR("subchip type error! subchip=%d\n", l_subchip_type);
            return -EFAIL;
    }
}

/*
 * �� �� ��  : get_cali_data_buf_addr_etc
 * ��������  : ���ر���У׼���ݵ��ڴ��ַ
 */
void *get_cali_data_buf_addr_etc(void)
{
    return CaliDataBuf_etc;
}

EXPORT_SYMBOL(get_cali_data_buf_addr_etc);
EXPORT_SYMBOL(netdev_is_open_etc);
EXPORT_SYMBOL(cali_update_channel_info);

/*
 * �� �� ��  : wifi_get_bfgx_rc_data_buf_addr
 * ��������  : ���ر���wifi rc codeУ׼���ݵ��ڴ��ַ
 */
void *wifi_get_bfgx_rc_data_buf_addr(uint32 *pul_len)
{
    bfgx_cali_data_stru *pst_bfgx_cali_buf = NULL;

    if (BfgxCaliDataBuf == NULL) {
        return NULL;
    }

    pst_bfgx_cali_buf = (bfgx_cali_data_stru *)BfgxCaliDataBuf;
    *pul_len = sizeof(pst_bfgx_cali_buf->auc_wifi_rc_code_data);

    PS_PRINT_INFO("wifi cali size is %d\n", *pul_len);

    return pst_bfgx_cali_buf->auc_wifi_rc_code_data;
}

EXPORT_SYMBOL(wifi_get_bfgx_rc_data_buf_addr);

/*
 * �� �� ��  : wifi_get_bt_cali_data_buf
 * ��������  : ���ر���wifi cali data for bt���ݵ��ڴ��ַ
 * �������  : wifi cali data for bt����buf�ĳ���
 * �� �� ֵ  : wifi cali data for bt����buf�ĵ�ַ��Ҳ������NULL
 */
void *wifi_get_bt_cali_data_buf(uint32 *pul_len)
{
    bfgx_cali_data_stru *pst_bfgx_cali_data_buf = NULL;

    if (BfgxCaliDataBuf == NULL) {
        return NULL;
    }

    pst_bfgx_cali_data_buf = (bfgx_cali_data_stru *)BfgxCaliDataBuf;

    *pul_len = sizeof(pst_bfgx_cali_data_buf->auc_wifi_cali_for_bt_data);

    PS_PRINT_INFO("bfgx wifi cali data for bt buf size is %d\n", *pul_len);

    return pst_bfgx_cali_data_buf->auc_wifi_cali_for_bt_data;
}

EXPORT_SYMBOL(wifi_get_bt_cali_data_buf);

/*
 * �� �� ��  : bfgx_get_cali_data_buf
 * ��������  : ���ر���bfgx btУ׼���ݵ��ڴ��ַ
 * �������  : bt У׼���� buf�ĳ���
 * �� �� ֵ  : bfgx btУ׼buf�ĵ�ַ��Ҳ������NULL
 */
void *bfgx_get_cali_data_buf(uint32 *pul_len)
{
    bfgx_cali_data_stru *pst_bfgx_cali_data_buf = NULL;

    if (BfgxCaliDataBuf == NULL) {
        return NULL;
    }

    pst_bfgx_cali_data_buf = (bfgx_cali_data_stru *)BfgxCaliDataBuf;

    *pul_len = sizeof(pst_bfgx_cali_data_buf->auc_bfgx_data);

    PS_PRINT_INFO("bfgx bt cali data buf size is %d\n", *pul_len);

    return pst_bfgx_cali_data_buf->auc_bfgx_data;
}

/*
 * �� �� ��  : bfgx_get_nv_data_buf
 * ��������  : ���ر���bfgx nv���ݵ��ڴ��ַ
 * �������  : nv buf�ĳ���
 * �� �� ֵ  : bfgx nv����buf�ĵ�ַ��Ҳ������NULL
 */
void *bfgx_get_nv_data_buf(uint32 *pul_len)
{
    bfgx_cali_data_stru *pst_bfgx_cali_data_buf = NULL;

    if (BfgxCaliDataBuf == NULL) {
        return NULL;
    }

    pst_bfgx_cali_data_buf = (bfgx_cali_data_stru *)BfgxCaliDataBuf;

    *pul_len = sizeof(pst_bfgx_cali_data_buf->auc_nv_data);

    PS_PRINT_INFO("bfgx nv buf size is %d\n", *pul_len);

    return pst_bfgx_cali_data_buf->auc_nv_data;
}

/*
 * �� �� ��  : bfgx_get_cust_ini_data_buf
 * ��������  : ���ر���bfgx ini���ƻ����ݵ��ڴ��ַ
 * �������  : bfgx ini���ƻ�����buf�ĳ���
 * �� �� ֵ  : bfgx ini����buf�ĵ�ַ��Ҳ������NULL
 */
void *bfgx_get_cust_ini_data_buf(uint32 *pul_len)
{
    bfgx_cali_data_stru *pst_bfgx_cali_data_buf = NULL;

    if (BfgxCaliDataBuf == NULL) {
        return NULL;
    }

    pst_bfgx_cali_data_buf = (bfgx_cali_data_stru *)BfgxCaliDataBuf;

    *pul_len = sizeof(pst_bfgx_cali_data_buf->auc_bt_cust_ini_data);

    PS_PRINT_INFO("bfgx cust ini buf size is %d\n", *pul_len);

    return pst_bfgx_cali_data_buf->auc_bt_cust_ini_data;
}

void plat_bfgx_cali_data_test_etc(void)
{
    bfgx_cali_data_stru *pst_cali_data = NULL;
    oal_uint32 *p_test = NULL;
    oal_uint32 count;
    oal_uint32 i;

    pst_cali_data = (bfgx_cali_data_stru *)bfgx_get_cali_data_buf(&count);
    if (pst_cali_data == NULL) {
        PS_PRINT_ERR("get_cali_data_buf_addr_etc failed\n");
        return;
    }

    p_test = (oal_uint32 *)pst_cali_data;
    count = count / sizeof(oal_uint32);

    for (i = 0; i < count; i++) {
        p_test[i] = i;
    }

    return;
}

/*
 * �� �� ��  : cali_data_buf_malloc_etc
 * ��������  : ���䱣��У׼���ݵ��ڴ�
 * �� �� ֵ  : 0��ʾ����ɹ���-1��ʾ����ʧ��
 */
oal_int32 cali_data_buf_malloc_etc(void)
{
    oal_uint8 *buffer = NULL;
    oal_uint32 ul_buffer_len;

    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1105) {
        ul_buffer_len = OAL_MIMO_CALI_DATA_STRU_LEN;
    } else {
        ul_buffer_len = OAL_DOUBLE_CALI_DATA_STRU_LEN;
    }

    buffer = (oal_uint8 *)OS_KZALLOC_GFP(ul_buffer_len);

    if (buffer == NULL) {
        PS_PRINT_ERR("malloc for CaliDataBuf_etc fail\n");
        return -EFAIL;
    }
    CaliDataBuf_etc = buffer;
    memset_s(CaliDataBuf_etc, ul_buffer_len, 0, ul_buffer_len);

    buffer = (oal_uint8 *)OS_KZALLOC_GFP(BFGX_CALI_DATA_BUF_LEN);
    if (buffer == NULL) {
        OS_MEM_KFREE(CaliDataBuf_etc);
        CaliDataBuf_etc = NULL;
        PS_PRINT_ERR("malloc for BfgxCaliDataBuf fail\n");
        return -EFAIL;
    }
    BfgxCaliDataBuf = buffer;

    init_completion(&cali_recv_done);

    return SUCC;
}

/*
 * �� �� ��  : cali_data_buf_free_etc
 * ��������  : �ͷű���У׼���ݵ��ڴ�
 */
void cali_data_buf_free_etc(void)
{
    if (CaliDataBuf_etc != NULL) {
        OS_MEM_KFREE(CaliDataBuf_etc);
    }
    CaliDataBuf_etc = NULL;

    if (BfgxCaliDataBuf != NULL) {
        OS_MEM_KFREE(BfgxCaliDataBuf);
    }
    BfgxCaliDataBuf = NULL;
}

/*
 * �� �� ��  : wait_bfgx_cali_data
 * ��������  : �ȴ�����device���͵�У׼����
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 wait_bfgx_cali_data(void)
{
#define WAIT_BFGX_CALI_DATA_TIME 2000
    uint64 timeleft;

    timeleft = wait_for_completion_timeout(&cali_recv_done, msecs_to_jiffies(WAIT_BFGX_CALI_DATA_TIME));
    if (!timeleft) {
        PS_PRINT_ERR("wait bfgx cali data timeout\n");
        return -ETIMEDOUT;
    }

    return 0;
}

/*
 * �� �� ��  : bfgx_cust_ini_init
 * ��������  : btУ׼���ƻ����ʼ��
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 bfgx_cust_ini_init(void)
{
    int32 i;
    int32 l_ret = INI_FAILED;
    int32 l_cfg_value;
    int32 l_ori_val;
    int8 *pst_buf = NULL;
    uint32 ul_len;

    for (i = 0; i < BFGX_CFG_INI_BUTT; i++) {
        l_ori_val = bfgx_ini_config_cmd[i].init_value;

        /* ��ȡini������ֵ */
        l_ret = get_cust_conf_int32_etc(INI_MODU_DEV_BT, bfgx_ini_config_cmd[i].name, &l_cfg_value);
        if (l_ret == INI_FAILED) {
            bfgx_cust_ini_data[i] = l_ori_val;
            PS_PRINT_DBG("bfgx read ini file failed cfg_id[%d],default value[%d]!", i, l_ori_val);
            continue;
        }

        bfgx_cust_ini_data[i] = l_cfg_value;

        PS_PRINT_INFO("bfgx ini init [id:%d] [%s] changed from [%d]to[%d]",
                      i, bfgx_ini_config_cmd[i].name, l_ori_val, l_cfg_value);
    }

    pst_buf = bfgx_get_cust_ini_data_buf(&ul_len);
    if (pst_buf == NULL) {
        PS_PRINT_ERR("get cust ini buf fail!");
        return INI_FAILED;
    }

    memcpy_s(pst_buf, ul_len, bfgx_cust_ini_data, sizeof(bfgx_cust_ini_data));

    return INI_SUCC;
}

/*
 * �� �� ��  : bfgx_customize_init
 * ��������  : bfgx���ƻ����ʼ������ȡini�����ļ�����ȡnv������
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 bfgx_customize_init(void)
{
    int32 ret;

    /* �������ڱ���У׼���ݵ�buffer */
    ret = cali_data_buf_malloc_etc();
    if (ret != OAL_SUCC) {
        PS_PRINT_ERR("alloc cali data buf fail\n");
        return INI_FAILED;
    }

    ret = bfgx_cust_ini_init();
    if (ret != OAL_SUCC) {
        PS_PRINT_ERR("bfgx ini init fail!\n");
        cali_data_buf_free_etc();
        return INI_FAILED;
    }

#ifdef HISI_NVRAM_SUPPORT
    ret = bfgx_nv_data_init();
    if (ret != OAL_SUCC) {
        PS_PRINT_ERR("bfgx nv data init fail!\n");
        cali_data_buf_free_etc();
        return INI_FAILED;
    }
#endif

    return INI_SUCC;
}

/*
 * �� �� ��  : bfgx_cali_data_init
 * ��������  : ������bt������btУ׼
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
oal_int32 bfgx_cali_data_init(void)
{
    int32 ret = 0;
    static uint32 cali_flag = 0;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    PS_PRINT_INFO("%s\n", __func__);

    /* У׼ֻ�ڿ���ʱִ��һ�Σ�OAM���ܱ�ɱ�����������Լӱ�־���� */
    if (cali_flag != 0) {
        PS_PRINT_INFO("bfgx cali data has inited\n");
        return 0;
    }

    cali_flag++;

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return OAL_FAIL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return OAL_FAIL;
    }

    mutex_lock(&pm_data->host_mutex);

    pst_bfgx_data = &ps_core_d->bfgx_info[BFGX_BT];
    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        PS_PRINT_WARNING("%s has opened! ignore bfgx cali!\n", bfgx_subsys_name_etc[BFGX_BT]);
        goto open_fail;
    }

    ret = hw_bfgx_open(BFGX_BT);
    if (ret != SUCC) {
        PS_PRINT_ERR("bfgx cali, open bt fail\n");
        goto open_fail;
    }

    ret = wait_bfgx_cali_data();
    if (ret != SUCC) {
        goto timeout;
    }

    ret = hw_bfgx_close(BFGX_BT);
    if (ret != SUCC) {
        PS_PRINT_ERR("bfgx cali, clsoe bt fail\n");
        goto close_fail;
    }

    mutex_unlock(&pm_data->host_mutex);

    return ret;

timeout:
    hw_bfgx_close(BFGX_BT);
close_fail:
open_fail:
    mutex_unlock(&pm_data->host_mutex);
    return ret;
}
