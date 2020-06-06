


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_profiling.h"
#include "oal_kernel_file.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_spec.h"
#include "wlan_types.h"

#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_ie.h"
#include "hmac_resource.h"
#include "hmac_scan.h"

#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"

#include "wal_main.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_scan.h"
#include "wal_linux_atcmdsrv.h"
#include "wal_linux_bridge.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_event.h"

#if ((defined(_PRE_PRODUCT_ID_HI110X_DEV)) || (defined(_PRE_PRODUCT_ID_HI110X_HOST)))
#include "plat_cali.h"
#include "oal_hcc_host_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/of.h>
#include <linux/of_gpio.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif
#include "board.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#endif
#endif
#include "securec.h"
#include "securectype.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_ATCMDSRV_C

/*****************************************************************************
  2 �ṹ�嶨��
*****************************************************************************/
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
typedef enum
{
    CHECK_LTE_GPIO_INIT            = 0,    /* ��ʼ�� */
    CHECK_LTE_GPIO_LOW             = 1,    /* ����Ϊ�͵�ƽ */
    CHECK_LTE_GPIO_HIGH            = 2,    /*����Ϊ�ߵ�ƽ */
    CHECK_LTE_GPIO_RESUME          = 3,    /*�ָ��Ĵ������� */
    CHECK_LTE_GPIO_DEV_LEVEL       = 4,    /*��ȡdevice GPIO�ܽŵ�ƽֵ*/
    CHECK_LTE_GPIO_BUTT
}check_lte_gpio_step;

typedef struct
{
    oal_uint8                     uc_mode;          /* ģʽ*/
    oal_uint8                     uc_band;          /* Ƶ�� */
}wal_atcmdsrv_mode_stru;

typedef struct
{
    oal_uint32                   ul_datarate;          /* at�������õ�����ֵ */
    oal_int8                    *puc_datarate;          /* �����ַ���*/
}wal_atcmdsrv_datarate_stru;

OAL_CONST wal_atcmdsrv_mode_stru g_ast_atcmdsrv_mode_table_etc[] =
{
    {WLAN_LEGACY_11A_MODE, WLAN_BAND_5G},    /* 11a, 5G, OFDM */
    {WLAN_LEGACY_11B_MODE, WLAN_BAND_2G},    /* 11b, 2.4G */
    {WLAN_LEGACY_11G_MODE, WLAN_BAND_2G},    /* �ɵ�11g only�ѷ���, 2.4G, OFDM */
    {WLAN_MIXED_ONE_11G_MODE, WLAN_BAND_2G},    /* 11bg, 2.4G */
    {WLAN_MIXED_TWO_11G_MODE, WLAN_BAND_2G},    /* 11g only, 2.4G */
    {WLAN_HT_MODE, WLAN_BAND_5G},    /* 11n(11bgn����11an������Ƶ���ж�) */
    {WLAN_VHT_MODE, WLAN_BAND_5G},    /* 11ac */
    {WLAN_HT_ONLY_MODE, WLAN_BAND_5G},    /* 11n only 5Gmode,ֻ�д�HT���豸�ſ��Խ��� */
    {WLAN_VHT_ONLY_MODE, WLAN_BAND_5G},    /* 11ac only mode ֻ�д�VHT���豸�ſ��Խ��� */
    {WLAN_HT_11G_MODE, WLAN_BAND_2G},    /* 11ng,������11b*/
    {WLAN_HT_ONLY_MODE_2G, WLAN_BAND_2G},/* 11nonlg 2Gmode*/
    {WLAN_VHT_ONLY_MODE_2G, WLAN_BAND_2G},    /* 11ac 2g mode ֻ�д�VHT���豸�ſ��Խ��� */
    {WLAN_PROTOCOL_BUTT,WLAN_BAND_2G},
};

OAL_STATIC OAL_CONST wal_atcmdsrv_datarate_stru   past_atcmdsrv_non_ht_rate_table[] =
{
    {0," 0 "},  /* mcs0, װ���·�65 */
    {1," 1 "},
    {2," 2 "},
    {5," 5.5 "},
    {6," 6 "},
    {7," 7 "},
    {8," 8 "},
    {9," 9 "},
    {11," 11 "},
    {12," 12 "},
    {18," 18 "},
    {24," 24 "},
    {36," 36 "},
    {48," 48 "},
    {54," 54 "},
};
oal_uint64                      ull_chipcheck_total_time_etc;
oal_uint16                      g_us_efuse_buffer_etc[WAL_ATCMDSRV_EFUSE_BUFF_LEN];

wal_efuse_bits                  *st_efuse_bits_etc = OAL_PTR_NULL;
oal_int32                       g_l_bandwidth_etc;
oal_int32                       g_l_mode_etc;

#define WAL_ATCMDSRV_WIFI_CHAN_NUM                2
#define WAL_ATCMDSRV_DYN_INTVAL_LEN               19
#define WAL_ATCMDSRV_DYN_INTVAL_LOCATION         (WAL_ATCMDSRV_DYN_INTVAL_LEN - 2)

extern OAL_CONST oal_int8   *pauc_bw_tbl[WLAN_BAND_ASSEMBLE_AUTO];

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern BOARD_INFO               board_info_etc;
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
wal_atcmdsrv_ant_info_stru g_st_atcmdsrv_ant_info;
#endif

extern oal_bool_enum_uint8 en_nv_dp_init_is_null;
extern oal_bool_enum_uint8 wlan_cal_intvl_enable;
extern oal_uint32  wal_hipriv_sta_pm_on_etc(oal_net_device_stru * pst_cfg_net_dev, oal_int8 * pc_param);
extern oal_bool_enum_uint8 wlan_cal_disable_switch;
extern oal_bool_enum_uint8 bfgx_is_shutdown_etc(void);


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

oal_int32  wal_atcmsrv_ioctl_get_rx_pckg_etc(oal_net_device_stru *pst_net_dev, oal_int32 *pl_rx_pckg_succ_num)
{
    oal_int32                   l_ret;
    mac_cfg_rx_fcs_info_stru   *pst_rx_fcs_info = OAL_PTR_NULL;
    wal_msg_write_stru          st_write_msg;
    mac_vap_stru               *pst_mac_vap;
    hmac_vap_stru              *pst_hmac_vap = OAL_PTR_NULL;
    oal_int32                   i_leftime;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_dbb_num::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_atcmsrv_ioctl_get_dbb_num::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag = OAL_FALSE;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RX_FCS_INFO, OAL_SIZEOF(mac_cfg_rx_fcs_info_stru));

    /* ��������������� */
    pst_rx_fcs_info = (mac_cfg_rx_fcs_info_stru *)(st_write_msg.auc_value);
    /*������������02�Ѿ�û������*/
    pst_rx_fcs_info->ul_data_op    = 1;
    pst_rx_fcs_info->ul_print_info = 0;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_rx_fcs_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag),WAL_ATCMDSRB_GET_RX_PCKT);

    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_pckg_etc::dbb_num wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_pckg_etc::dbb_num wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else
    {
        /* ��������  */
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_pckg_etc::dbb_num wait for %ld ms error!}",
                      ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        *pl_rx_pckg_succ_num = (oal_int)pst_hmac_vap->st_atcmdsrv_get_status.ul_rx_pkct_succ_num;
        return OAL_SUCC;
    }
}


oal_int32  wal_atcmsrv_ioctl_set_hw_addr_etc(oal_net_device_stru *pst_net_dev, oal_uint8 *pc_hw_addr)
{
    oal_int32                       l_ret;
    mac_cfg_staion_id_param_stru   *pst_mac_cfg_para = OAL_PTR_NULL;
    wal_msg_write_stru              st_write_msg;


    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_STATION_ID, OAL_SIZEOF(mac_cfg_staion_id_param_stru));

    /* ��������������� */
    pst_mac_cfg_para = (mac_cfg_staion_id_param_stru *)(st_write_msg.auc_value);
    /*������������02�Ѿ�û������*/
    pst_mac_cfg_para->en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    oal_set_mac_addr(pst_mac_cfg_para->auc_station_id, pc_hw_addr);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_staion_id_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_hw_addr_etc::return err code %d!}\r\n", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_freq(oal_net_device_stru *pst_net_dev, oal_int32 l_freq)
{
    wal_msg_write_stru          st_write_msg;

    oal_int32                   l_ret;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_freq:l_freq[%d]", l_freq);
    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CURRENT_CHANEL, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = l_freq;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_freq::return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_int32 wal_atcmsrv_ioctl_set_country(oal_net_device_stru *pst_net_dev, oal_int8 *puc_countrycode)
{
#ifdef _PRE_WLAN_FEATURE_11D
    oal_int32       l_ret;

    l_ret = wal_regdomain_update_for_dfs_etc(pst_net_dev, puc_countrycode);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_country::regdomain_update_for_dfs return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    l_ret = wal_regdomain_update_etc(pst_net_dev, puc_countrycode);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_country::regdomain_update return err code %d!}\r\n", l_ret);
        return l_ret;
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_txpower(oal_net_device_stru *pst_net_dev, oal_int32 l_txpower)
{
    wal_msg_write_stru          st_write_msg;

    oal_int32                   l_ret;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_txpower:l_txpower[%d]", l_txpower);

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TX_POWER, OAL_SIZEOF(oal_int32));
    *((oal_int32 *)(st_write_msg.auc_value)) = (oal_int32)(l_txpower );

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_txpower::return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_mode(oal_net_device_stru *pst_net_dev, oal_int32 l_mode)
{
    wal_msg_write_stru          st_write_msg;
    mac_cfg_mode_param_stru    *pst_mode_param;
    oal_uint8                   uc_prot_idx;
    mac_vap_stru               *pst_mac_vap;
    oal_uint8                   uc_arr_num = OAL_ARRAY_SIZE(g_ast_atcmdsrv_mode_table_etc);

    oal_int32                   l_ret = 0;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_mode::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    /*��ȡģʽ��Ӧ��band*/
    for (uc_prot_idx = 0; uc_prot_idx < uc_arr_num; uc_prot_idx++)
    {
        if (g_ast_atcmdsrv_mode_table_etc[uc_prot_idx].uc_mode == (oal_uint8)l_mode)
        {
            break;
        }
    }
    if (uc_prot_idx >= uc_arr_num)
    {
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"{wal_atcmsrv_ioctl_set_mode:err code[%u]}",uc_prot_idx);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_MODE, OAL_SIZEOF(mac_cfg_mode_param_stru));

    /*����ģʽ��������ģʽ��ʱ�򽫴���Ĭ�ϳ�20M*/
    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);
    if(WLAN_HT_ONLY_MODE_2G == l_mode)
    {
        pst_mode_param->en_protocol  = WLAN_HT_ONLY_MODE;
    }
    else if(WLAN_VHT_ONLY_MODE_2G == l_mode)
    {
        pst_mode_param->en_protocol  = WLAN_VHT_MODE;
    }
    else
    {
        pst_mode_param->en_protocol  = (oal_uint8)l_mode;
    }
    pst_mode_param->en_band      = (wlan_channel_band_enum_uint8)g_ast_atcmdsrv_mode_table_etc[uc_prot_idx].uc_band;
    pst_mode_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    /*δ��ʹ�ã�������ɾ��*/
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_atcmsrv_ioctl_set_mode::protocol[%d],band[%d],bandwidth[%d]!}\r\n",
                            pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_mode::return err code %d!}\r\n", l_ret);
        return l_ret;
    }
    g_l_mode_etc = pst_mode_param->en_protocol;
    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_atcmsrv_set_bw(oal_uint8 *pen_bw)
{
    oal_int8                    ac_bw_str[5] = {0};
    OAL_CONST oal_int8          *c_bw20 = "20";
    OAL_CONST oal_int8          *c_bw40 = "40";
    OAL_CONST oal_int8          *c_bw80 = "80";
    OAL_CONST oal_int8          *c_bw160 = "160";

    if ((WLAN_BAND_WIDTH_160PLUSPLUSPLUS <= g_l_bandwidth_etc)
            && (g_l_bandwidth_etc <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS))
    {
        strncpy_s(ac_bw_str, sizeof(ac_bw_str), c_bw160, OAL_MIN(OAL_ARRAY_SIZE(ac_bw_str), OAL_STRLEN(c_bw160)));
    }
    else if ((WLAN_BAND_WIDTH_80PLUSPLUS <= g_l_bandwidth_etc)
        && (g_l_bandwidth_etc <= WLAN_BAND_WIDTH_80MINUSMINUS))
    {
        strncpy_s(ac_bw_str, sizeof(ac_bw_str), c_bw80, OAL_MIN(OAL_ARRAY_SIZE(ac_bw_str), OAL_STRLEN(c_bw80)));
    }
    else if ((WLAN_BAND_WIDTH_40PLUS <= g_l_bandwidth_etc)
        && (g_l_bandwidth_etc <= WLAN_BAND_WIDTH_40MINUS))
    {
        strncpy_s(ac_bw_str, sizeof(ac_bw_str), c_bw40, OAL_MIN(OAL_ARRAY_SIZE(ac_bw_str), OAL_STRLEN(c_bw40)));
    }
    else
    {
        strncpy_s(ac_bw_str, sizeof(ac_bw_str), c_bw20, OAL_MIN(OAL_ARRAY_SIZE(ac_bw_str), OAL_STRLEN(c_bw20)));
    }

    for (*pen_bw = 0; *pen_bw < WLAN_BANDWITH_CAP_BUTT; (*pen_bw)++)
    {
        if(!oal_strcmp(pauc_bw_tbl[*pen_bw], ac_bw_str))
        {
            break;
        }
    }

    /* ��������Ƿ��� */
    if (*pen_bw >= WLAN_BANDWITH_CAP_BUTT)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{wal_hipriv_set_bw::not support this bandwidth!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}



OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_datarate(oal_net_device_stru *pst_net_dev, oal_int32 l_datarate)
{
    oal_uint8                   uc_prot_idx;
    oal_uint32                  ul_ret;
    mac_vap_stru                *pst_mac_vap;
    oal_uint8                   en_bw_index = 0;
    mac_cfg_tx_comp_stru        *pst_set_bw_param;
    wal_msg_write_stru          st_write_msg;
    oal_int32                   l_ret;
    oal_uint8                   uc_arr_num = OAL_ARRAY_SIZE(past_atcmdsrv_non_ht_rate_table);

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_datarate::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_datarate:l_datarate[%d]", l_datarate);

    /*��ȡ���ʶ�Ӧ���ַ�����������������ʵ���Ӧ�ӿ�*/
    for (uc_prot_idx = 0; uc_prot_idx < uc_arr_num; uc_prot_idx++)
    {
        if (past_atcmdsrv_non_ht_rate_table[uc_prot_idx].ul_datarate == (oal_uint32)l_datarate)
        {
            break;
        }
    }
    if (uc_prot_idx >= uc_arr_num)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY,"uc_prot_idx Overrunning!");
        return -OAL_EINVAL;
    }
    if(WLAN_HT_ONLY_MODE == g_l_mode_etc)/*����������Ϊ7ʱ��ʾMCS7*/
    {
        ul_ret = wal_hipriv_set_mcs_etc(pst_net_dev,(oal_int8 *)past_atcmdsrv_non_ht_rate_table[uc_prot_idx].puc_datarate);
    }
    else if(WLAN_VHT_MODE == g_l_mode_etc)
    {
        ul_ret = wal_hipriv_set_mcsac_etc(pst_net_dev,(oal_int8 *)past_atcmdsrv_non_ht_rate_table[uc_prot_idx].puc_datarate);
    }
    else
    {
        ul_ret = wal_hipriv_set_rate_etc(pst_net_dev,(oal_int8 *)past_atcmdsrv_non_ht_rate_table[uc_prot_idx].puc_datarate);
    }
    if (OAL_SUCC != ul_ret)
    {
        return -OAL_EFAIL;
    }
    /*���ó�������������*/
   /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_BW, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bw_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    l_ret = wal_atcmsrv_set_bw(&en_bw_index);
    if (OAL_SUCC != l_ret)
    {
        return -OAL_EFAIL;
    }
    pst_set_bw_param->uc_param = (oal_uint8)(en_bw_index);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_bandwidth(oal_net_device_stru *pst_net_dev, oal_int32 l_bandwidth)
{
    wal_msg_write_stru          st_write_msg;
    mac_cfg_mode_param_stru    *pst_mode_param;

    oal_int32                   l_ret;


    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_bandwidth:l_bandwidth[%d]", l_bandwidth);
    g_l_bandwidth_etc = l_bandwidth;

    /***************************************************************************
        ���¼���wal�㴦��
    ***************************************************************************/
    /* ��д��Ϣ */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BANDWIDTH, OAL_SIZEOF(oal_int32));

    /*���ô���ʱ��ģʽ�����޸ģ����ǰ���֮ǰ��ֵ����*/
    pst_mode_param = (mac_cfg_mode_param_stru *)(st_write_msg.auc_value);

    pst_mode_param->en_bandwidth = (oal_uint8)l_bandwidth;

    /* ������Ϣ */
    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_mode_param_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_mode::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    return OAL_SUCC;

}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_always_tx(oal_net_device_stru *pst_net_dev,oal_int32 l_always_tx)
{
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    mac_cfg_tx_comp_stru             *pst_set_bcast_param;
    oal_uint8                         auc_param[] = {"all"};
    oal_uint16                        us_len;
    oal_uint32                       *pul_num;


    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_always_tx:l_always_tx[%d]", l_always_tx);

    /* �رճ���ʱ����Ҫ�ظ�����num */
    if (l_always_tx != 0)
    {
        /***************************************************************************
                         ���¼���wal�㴦��,1103������̬У׼������num
        ***************************************************************************/
        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX_NUM, OAL_SIZEOF(oal_uint32));

        /* ��ȡ�������� */
        pul_num = (oal_uint32 *)(st_write_msg.auc_value);

        *pul_num = 0xffffffe; /*0xffffffff�����¶Ȳ��� */

        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint32),
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::return err code [%d]!}\r\n", l_ret);
        }
    }


    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_TX, OAL_SIZEOF(mac_cfg_tx_comp_stru));

    /* ��������������������� */
    pst_set_bcast_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* װ�����Ե������ֱ�ӽ������������ú� */
    pst_set_bcast_param->en_payload_flag = RF_PAYLOAD_RAND;
    pst_set_bcast_param->ul_payload_len = WAL_ATCMDSRB_IOCTL_AL_TX_LEN;
    pst_set_bcast_param->uc_param = (oal_uint8)l_always_tx;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_tx_comp_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_always_tx::return err code [%d]!}", l_ret);
        return l_ret;
    }


    /*��ӡά����Ϣ*/
    if (l_always_tx)
    {
        /*��ӡ���мĴ���*/
        /***************************************************************************
                                    ���¼���wal�㴦��
        ***************************************************************************/
        if (EOK != memcpy_s(st_write_msg.auc_value, OAL_STRLEN((oal_int8*)auc_param),
                             auc_param, OAL_STRLEN((oal_int8*)auc_param))) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_always_tx::memcpy fail!");
            return OAL_FAIL;
        }

        st_write_msg.auc_value[OAL_STRLEN((oal_int8*)auc_param)] = '\0';
        us_len = (oal_uint16)(OAL_STRLEN((oal_int8*)auc_param) + 1);

        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REG_INFO, us_len);

        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_always_tx::return err code [%d]!}\r\n", l_ret);
        }
    }
    return OAL_SUCC;
}



OAL_STATIC oal_void wal_atcmdsrv_ioctl_convert_dbb_num(oal_uint32 ul_dbb_num,oal_uint8 *pc_dbb_num)
{
    oal_uint8  uc_temp          = 0;

    /* MAC H/w version register format                  */
    /* ------------------------------------------------ */
    /* | 31 - 24 | 23 - 16 | 15 - 12 | 11 - 0 | */
    /* ------------------------------------------------ */
    /* | BN      | Y1      | Y2      |   Y3   | */
    /* ------------------------------------------------ */

    /* Format the version as BN.Y1.Y2.Y3 with all values in hex i.e. the  */
    /* version string would be XX.XX.X.XXX.                                 */
    /* For e.g. 0225020A saved in the version register would translate to */
    /* the configuration interface version number 02.25.0.20A             */

    uc_temp = (ul_dbb_num & 0xF0000000) >> 28;
    pc_dbb_num[0] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);
    uc_temp = (ul_dbb_num & 0x0F000000) >> 24;
    pc_dbb_num[1] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);

    pc_dbb_num[2] = '.';

    uc_temp = (ul_dbb_num & 0x00F00000) >> 20;
    pc_dbb_num[3] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);
    uc_temp = (ul_dbb_num & 0x000F0000) >> 16;
    pc_dbb_num[4] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);
    pc_dbb_num[5] = '.';

    uc_temp = (ul_dbb_num & 0x0000F000) >> 12;
    pc_dbb_num[6] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);
    pc_dbb_num[7] = '.';

    uc_temp = (ul_dbb_num & 0x00000F00) >> 8;
    pc_dbb_num[8] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);
    uc_temp = (ul_dbb_num & 0x000000F0) >> 4;
    pc_dbb_num[9] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);
    uc_temp = (ul_dbb_num & 0x0000000F) >> 0;
    pc_dbb_num[10] = WAL_ATCMDSRV_GET_HEX_CHAR(uc_temp);


    return ;
}


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_get_dbb_num(oal_net_device_stru *pst_net_dev, oal_int8 *pc_dbb_num)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_int32                       i_leftime;
    mac_vap_stru                   *pst_mac_vap;
    hmac_vap_stru                  *pst_hmac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_dbb_num::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_atcmsrv_ioctl_get_dbb_num::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    /***************************************************************************
                              ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_FALSE;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_VERSION, 0);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH,
                             (oal_uint8 *)&st_write_msg,
                             OAL_FALSE,
                             OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_dbb_num::wal_send_cfg_event_etc return err_code [%d]!}", l_ret);
        return l_ret;
    }
    /*�����ȴ�dmac�ϱ�*/
    /*lint -e730*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag),WAL_ATCMDSRB_DBB_NUM_TIME);
    /*lint +e730*/
    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_dbb_num::dbb_num wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_dbb_num::dbb_num wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else
    {
        /* ��������  */
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_dbb_num::dbb_num wait for %ld ms error!}",
                      ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));

        if(0x036c0907 != pst_hmac_vap->st_atcmdsrv_get_status.ul_dbb_num)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_get_dbb_num:ul_dbb_num[0x%x],not match 0x036c0907", pst_hmac_vap->st_atcmdsrv_get_status.ul_dbb_num);
            return -OAL_EINVAL;
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_get_dbb_num:ul_dbb_num[0x%x].", pst_hmac_vap->st_atcmdsrv_get_status.ul_dbb_num);
        }

        wal_atcmdsrv_ioctl_convert_dbb_num(pst_hmac_vap->st_atcmdsrv_get_status.ul_dbb_num,(oal_uint8 *)pc_dbb_num);
        return OAL_SUCC;
    }


}


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_lte_gpio_mode(oal_net_device_stru *pst_net_dev, oal_int32 l_check_lte_gpio_step)
{
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
    oal_int32                       i_leftime;
    mac_vap_stru                   *pst_mac_vap;
    hmac_vap_stru                  *pst_hmac_vap;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_mode::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_atcmsrv_ioctl_lte_gpio_mode::mac_res_get_hmac_vap failed!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap->st_atcmdsrv_get_status.uc_lte_gpio_check_flag = OAL_FALSE;

    /***************************************************************************
         ���¼���wal�㴦��
     ***************************************************************************/
     /* ��д��Ϣ */
     WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CHECK_LTE_GPIO, OAL_SIZEOF(oal_int32));

     /*����LTE�麸����ģʽ*/
     *(oal_int32 *)(st_write_msg.auc_value) = l_check_lte_gpio_step;

     /* ������Ϣ */
     l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);

     if (OAL_UNLIKELY(OAL_SUCC != l_ret))
     {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_mode::return err code %d!}\r\n", l_ret);
         return l_ret;
     }
    /*�����ȴ�dmac�ϱ�*/
    /*lint -e730*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_lte_gpio_check_flag),WAL_ATCMDSRB_DBB_NUM_TIME);
    /*lint +e730*/
    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_mode:: wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_mode:: wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else
    {
        return OAL_SUCC;
    }
}
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
OAL_STATIC oal_int32 wal_gpio_direction_output(oal_uint32 ul_gpio_num, oal_int32 l_gpio_level)
{
    oal_int32 l_ret = OAL_SUCC;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == ul_gpio_num)
    {
        OAM_WARNING_LOG1(0, 0, "wal_gpio_direction_output::no need to check gpio %d", ul_gpio_num);
        return OAL_SUCC;
    }

    OAM_WARNING_LOG2(0, 0, "wal_gpio_direction_output::gpio %d output set to %d", ul_gpio_num, l_gpio_level);
    l_ret = gpio_direction_output(ul_gpio_num, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET ISM PRIORITY FAIL!");
        return l_ret;
    }
#endif
    return l_ret;
}
#endif

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_lte_gpio_level_set(oal_int32 l_gpio_level)
{
    oal_int32 l_ret = OAL_SUCC;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_ism_priority, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET ISM PRIORITY FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_lte_rx, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET LTE RX FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_lte_tx, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET LTE TX FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_lte_inact, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET LTE INACT FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_ism_rx_act, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET ISM RX ACT FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_bant_pri, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET BANT PRI FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_bant_status, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET BANT STATUS FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_want_pri, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET WANT PRI FAIL!");
        return l_ret;
    }

    l_ret = wal_gpio_direction_output(wlan_customize_etc.ul_want_status, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET WANT STATUS FAIL!");
        return l_ret;
    }
#else
    l_ret = gpio_direction_output(WAL_ATCMDSRV_LTE_ISM_PRIORITY, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET LTE ISM PRIORITY FAIL!");
        return l_ret;
    }

    l_ret = gpio_direction_output(WAL_ATCMDSRV_LTE_RX_ACT, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET LTE RX ACT FAIL!");
        return l_ret;
    }

    l_ret = gpio_direction_output(WAL_ATCMDSRV_LTE_TX_ACT, l_gpio_level);
    if (OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_level_set:SET LTE TX ACT FAIL!");
        return l_ret;
    }
#endif
#endif

    return l_ret;
}


oal_uint8 g_uc_dev_lte_gpio_level_etc = 0x0;
OAL_STATIC oal_int32  wal_atcmsrv_ioctl_lte_gpio_level_check(oal_net_device_stru *pst_net_dev, oal_int32 l_gpio_level)
{
    oal_int32 l_ret;

    l_ret = wal_atcmsrv_ioctl_lte_gpio_mode(pst_net_dev, CHECK_LTE_GPIO_DEV_LEVEL);
    if (l_ret < 0)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_level_check::GET DEV LTE GPIO LEVEL FAIL!}");
        return -OAL_EINVAL;
    }

    l_ret = -OAL_EINVAL;

    if (0 == l_gpio_level)
    {
        if (0x0 == g_uc_dev_lte_gpio_level_etc)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_level_check::check gpio low mode SUCC!}");
            l_ret = OAL_SUCC;
        }
    }
    else if (1 == l_gpio_level)
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_HOST)
    {
        /*CHECK BIT_2 BIT_5 BIT_6*/
        if (0x64 == g_uc_dev_lte_gpio_level_etc)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_level_check::check gpio high mode SUCC!}");
            l_ret = OAL_SUCC;
        }
    }
#else
    {
        /*CHECK BIT_3 BIT_4*/
        if (0x18 == g_uc_dev_lte_gpio_level_etc)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_level_check::check gpio high mode SUCC!}");
            l_ret = OAL_SUCC;
        }
    }
#endif
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_level_check::unknown param l_gpio_level %d!}", l_gpio_level);
    }

    return l_ret;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
OAL_STATIC oal_int32 wal_gpio_request_one(oal_uint32 ul_gpio_num, OAL_CONST oal_int8* name)
{
    oal_int32       l_ret = OAL_SUCC;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (NULL == name)
    {
        OAM_ERROR_LOG1(0, 0, "wal_gpio_request_one::invalid input gpio %d", ul_gpio_num);
        return OAL_EFAIL;
    }

    if (0 == ul_gpio_num)
    {
        OAM_WARNING_LOG1(0, 0, "wal_gpio_request_one::no need to check gpio %d", ul_gpio_num);
        return OAL_SUCC;
    }
    OAM_WARNING_LOG1(0, 0, "wal_gpio_request_one::request gpio_num is %d", ul_gpio_num);
#ifdef GPIOF_OUT_INIT_LOW
    l_ret = gpio_request_one(ul_gpio_num, GPIOF_OUT_INIT_LOW, name);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_ISM_PRIORITY mode gpio fail");
#ifdef _PRE_CONFIG_USE_DTS
        if (board_info_etc.need_power_prepare)
        {
            l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_normal);
            if (l_ret)
            {
                OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set pinctrl_select_state fail");
            }
        }
#endif
        return -OAL_EFAIL;
    }
#else
    l_ret = gpio_request(ul_gpio_num, name);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_ISM_PRIORITY mode gpio fail");
#ifdef _PRE_CONFIG_USE_DTS
        if (board_info_etc.need_power_prepare)
        {
            l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_normal);
            if (l_ret)
            {
                OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set pinctrl_select_state fail");
            }
        }
#endif
        return -OAL_EFAIL;
    }

    gpio_direction_output(ul_gpio_num, 0);
#endif

#endif
    return l_ret;
}
#endif


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_lte_gpio_set(oal_void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_int32       l_ret = -OAL_EFAIL;
    /*�����ܽ����ó�gpioģʽ*/
#ifdef _PRE_CONFIG_USE_DTS
    if (board_info_etc.need_power_prepare)
    {
        /* set LowerPower mode */
        l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_idle);
        if (l_ret)
        {
            OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set mode gpio fail");
            return -OAL_EFAIL;
        }
    }
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_ism_priority, WAL_ATCMDSRV_ISM_PRIORITY_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_ISM_PRIORITY mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_lte_rx, WAL_ATCMDSRV_LTE_RX_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_RX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_lte_tx, WAL_ATCMDSRV_LTE_TX_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_lte_inact, WAL_ATCMDSRV_LTE_INACT_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_ism_rx_act, WAL_ATCMDSRV_ISM_RX_ACT_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_bant_pri, WAL_ATCMDSRV_BANT_PRI_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_bant_status, WAL_ATCMDSRV_BANT_STATUS_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_want_pri, WAL_ATCMDSRV_WANT_PRI_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }

    l_ret = wal_gpio_request_one(wlan_customize_etc.ul_want_status, WAL_ATCMDSRV_WANT_STATUS_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        return -OAL_EFAIL;
    }
#else
    l_ret = gpio_request_one(WAL_ATCMDSRV_LTE_ISM_PRIORITY, GPIOF_OUT_INIT_LOW, WAL_ATCMDSRV_LTE_ISM_PRIORITY_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_ISM_PRIORITY mode gpio fail");
        if (board_info_etc.need_power_prepare)
        {
            l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_normal);
            if (l_ret)
            {
                OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set pinctrl_select_state fail");
            }
        }
        return -OAL_EFAIL;
    }

    l_ret = gpio_request_one(WAL_ATCMDSRV_LTE_RX_ACT, GPIOF_OUT_INIT_LOW, WAL_ATCMDSRV_LTE_RX_ACT_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_RX_ACT mode gpio fail");
        gpio_free(WAL_ATCMDSRV_LTE_ISM_PRIORITY);
        if (board_info_etc.need_power_prepare)
        {
            l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_normal);
            if (l_ret)
            {
                OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set pinctrl_select_state fail");
            }
        }
        return -OAL_EFAIL;
    }

    l_ret = gpio_request_one(WAL_ATCMDSRV_LTE_TX_ACT, GPIOF_OUT_INIT_LOW, WAL_ATCMDSRV_LTE_TX_ACT_NAME);
    if (l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set LTE_TX_ACT mode gpio fail");
        gpio_free(WAL_ATCMDSRV_LTE_ISM_PRIORITY);
        gpio_free(WAL_ATCMDSRV_LTE_RX_ACT);
        if (board_info_etc.need_power_prepare)
        {
            l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_normal);
            if (l_ret)
            {
                OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_set:set pinctrl_select_state fail");
            }
        }
        return -OAL_EFAIL;
    }
#endif
#endif
    return OAL_SUCC;
}
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
OAL_STATIC oal_void  wal_gpio_free(oal_uint32 ul_gpio_num)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == ul_gpio_num)
        return;

    else
    {
        OAM_INFO_LOG1(0, 0, "wal_gpio_free:: free gpio %d", ul_gpio_num);
        gpio_free(ul_gpio_num);
    }
#endif
}
#endif

OAL_STATIC oal_void  wal_atcmsrv_ioctl_lte_gpio_free(oal_net_device_stru *pst_net_dev)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wal_msg_write_stru              st_write_msg;
    oal_int32                       l_ret;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    wal_gpio_free(wlan_customize_etc.ul_ism_priority);
    wal_gpio_free(wlan_customize_etc.ul_lte_rx);
    wal_gpio_free(wlan_customize_etc.ul_lte_tx);
    wal_gpio_free(wlan_customize_etc.ul_lte_inact);
    wal_gpio_free(wlan_customize_etc.ul_ism_rx_act);
    wal_gpio_free(wlan_customize_etc.ul_bant_pri);
    wal_gpio_free(wlan_customize_etc.ul_bant_status);
    wal_gpio_free(wlan_customize_etc.ul_want_pri);
    wal_gpio_free(wlan_customize_etc.ul_want_status);
#else
    gpio_free(WAL_ATCMDSRV_LTE_ISM_PRIORITY);

    gpio_free(WAL_ATCMDSRV_LTE_RX_ACT);

    gpio_free(WAL_ATCMDSRV_LTE_TX_ACT);
#endif
#ifdef _PRE_CONFIG_USE_DTS
    if (board_info_etc.need_power_prepare)
    {
        l_ret = pinctrl_select_state(board_info_etc.pctrl, board_info_etc.pins_normal);
        if (l_ret)
        {
            OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_free:set pinctrl_select_state fail");
        }
    }
#endif
    /***************************************************************************
         ���¼���wal�㴦��
     ***************************************************************************/
     /* ��д��Ϣ */
     WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_CHECK_LTE_GPIO, OAL_SIZEOF(oal_int32));

     /*����LTE�麸����ģʽ*/
     *(oal_int32 *)(st_write_msg.auc_value) = CHECK_LTE_GPIO_RESUME;

     /* ������Ϣ */
     l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                WAL_MSG_TYPE_WRITE,
                                WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                                (oal_uint8 *)&st_write_msg,
                                OAL_FALSE,
                                OAL_PTR_NULL);

     if (OAL_UNLIKELY(OAL_SUCC != l_ret))
     {
         OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_lte_gpio_mode::return err code %d!}\r\n", l_ret);
     }
#endif
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_lte_gpio_check(oal_net_device_stru *pst_net_dev)
{
    oal_int32 l_ret;

    /*********step1 ���ùܽ�Ϊgpioģʽ********/
    OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_check:enter lte gpio check!");
    /*��ʼ��host�ܽ�*/
    l_ret = wal_atcmsrv_ioctl_lte_gpio_set();
    if(OAL_SUCC != l_ret)
    {
        return l_ret;
    }

    /*��ʼ��device lte�������ż��*/
    l_ret = wal_atcmsrv_ioctl_lte_gpio_mode(pst_net_dev,CHECK_LTE_GPIO_INIT);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_check:CHECK_LTE_GPIO_INIT FAIL!");
        wal_atcmsrv_ioctl_lte_gpio_free(pst_net_dev);
        return l_ret;
    }

    /*********step2 ����host�ܽ�Ϊ�ͣ���ȡdevice���********/
    /*��gpioȫ������Ϊ��*/
    l_ret = wal_atcmsrv_ioctl_lte_gpio_level_set(0);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_check:SET LTE GPIO LOW FAIL!");
        wal_atcmsrv_ioctl_lte_gpio_free(pst_net_dev);
        return l_ret;
    }

    /*��ȡdevice GPIO�ܽŵ�ƽ*/
    l_ret = wal_atcmsrv_ioctl_lte_gpio_level_check(pst_net_dev, 0);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG1(0, 0, "wal_atcmsrv_ioctl_lte_gpio_check:check gpio low mode FAIL[%x]!", g_uc_dev_lte_gpio_level_etc);
        wal_atcmsrv_ioctl_lte_gpio_free(pst_net_dev);
        return l_ret;
    }

    /*********step3 ����host�ܽ�Ϊ�ߣ���ȡdevice���********/
    /*��gpioȫ������Ϊ��*/
    l_ret = wal_atcmsrv_ioctl_lte_gpio_level_set(1);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_lte_gpio_check:SET LTE GPIO HIGH FAIL!");
        wal_atcmsrv_ioctl_lte_gpio_free(pst_net_dev);
        return l_ret;
    }

    /*��ȡdevice GPIO�ܽŵ�ƽ*/
    l_ret = wal_atcmsrv_ioctl_lte_gpio_level_check(pst_net_dev, 1);
    if(0 != l_ret)
    {
        OAM_WARNING_LOG1(0, 0, "wal_atcmsrv_ioctl_lte_gpio_check:check gpio high mode FAIL[%x]!", g_uc_dev_lte_gpio_level_etc);
        wal_atcmsrv_ioctl_lte_gpio_free(pst_net_dev);
        return l_ret;
    }

    wal_atcmsrv_ioctl_lte_gpio_free(pst_net_dev);

    return OAL_SUCC;
 }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
OAL_STATIC oal_int32 wal_atcmsrv_ioctl_wipin_test(oal_net_device_stru *pst_net_dev, oal_int32 *pl_pin_status)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_int32                       l_ret = 0;
#endif
    oal_int32                       l_lte_status = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_wipin_test::wlan_customize_etc.ul_lte_gpio_check_switch is %d", wlan_customize_etc.ul_lte_gpio_check_switch);
    /* ���ƻ��Ƿ���lte����ܽ� */
    if(wlan_customize_etc.ul_lte_gpio_check_switch == 1)
    {
        /*��ȡlte����ܽŽ��*/
        l_ret = wal_atcmsrv_ioctl_lte_gpio_check(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            l_lte_status = 1;
        }
    }
#endif
    *pl_pin_status = l_lte_status;

    if(0 != *pl_pin_status)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_wipin_test::coe_lte_pin status is %d", *pl_pin_status);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}
#endif


oal_int32 wal_atcmsrv_ioctl_get_hw_status_etc(oal_net_device_stru *pst_net_dev, oal_int32 *pl_fem_pa_status)
{
    oal_int32                       l_ret = 0;
    oal_uint32                      ul_lte_status = 0;
    hi1103_cali_param_stru          *pst_cali_data_c0;
    hi1103_cali_param_stru          *pst_cali_data_c1;

    if(wlan_customize_etc.ul_lte_gpio_check_switch == 1)
    {
        /*��ȡlte����ܽŽ��*/
        l_ret = wal_atcmsrv_ioctl_lte_gpio_check(pst_net_dev);
        if(OAL_SUCC != l_ret)
        {
            ul_lte_status = 1;
        }
    }

    pst_cali_data_c0 = (hi1103_cali_param_stru *)get_cali_data_buf_addr_etc();
    pst_cali_data_c1 = pst_cali_data_c0 + 1;
    if ((NULL == pst_cali_data_c0) || (NULL == pst_cali_data_c1))
    {
        OAM_ERROR_LOG0(0, 0, "wal_atcmsrv_ioctl_get_hw_status_etc::null ptr happened");
        return -OAL_EINVAL;
    }

    *pl_fem_pa_status = (oal_int32)((pst_cali_data_c0->ul_check_hw_status)|(pst_cali_data_c1->ul_check_hw_status << 4)|(ul_lte_status << 8));
    if(0 != *pl_fem_pa_status)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_hw_status_etc::fem_pa_sta_c0[bit0-bit1 2g,bit2-bit3 5g],fem_pa_sta_c1[bit4-bit5 2g,bit6-7 5g],lte_gpio[bit8],ul_check_hw_status[0x%x]}", *pl_fem_pa_status);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}


oal_void wal_atcmsrv_ioctl_get_fem_pa_status_etc(oal_net_device_stru *pst_net_dev, oal_int32 *pl_fem_pa_status)
{
    hi1103_cali_param_stru            *pst_cali_data_c0;
    hi1103_cali_param_stru            *pst_cali_data_c1;

    pst_cali_data_c0 = (hi1103_cali_param_stru *)get_cali_data_buf_addr_etc();
    pst_cali_data_c1 = pst_cali_data_c0 + 1;
    if ((NULL == pst_cali_data_c0) || (NULL == pst_cali_data_c1))
    {
        OAM_ERROR_LOG0(0, 0, "wal_atcmsrv_ioctl_get_hw_status_etc::null ptr happened");
        return;
    }

    *pl_fem_pa_status = (oal_int32)((pst_cali_data_c0->ul_check_hw_status)|(pst_cali_data_c1->ul_check_hw_status << 4));
    if(0 != *pl_fem_pa_status)
    {
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV, CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_FEM_FAIL);
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_fem_pa_status_etc::fem_pa_sta_c0[bit0-bit1 2g,bit2-bit3 5g],fem_pa_sta_c1[bit4-bit5 2g,bit6-7 5g];ul_check_hw_status[0x%x]}", *pl_fem_pa_status);
#ifdef CONFIG_HUAWEI_DSM
        hw_1103_dsm_client_notify(SYSTEM_TYPE_WIFI, DSM_WIFI_FEMERROR, "%s: fem error",  __FUNCTION__);
#endif
    }
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_always_rx(oal_net_device_stru *pst_net_dev, oal_int32 l_always_rx)
{
    wal_msg_write_stru               st_write_msg;
    oal_int32                        l_ret;
    oal_uint8                        auc_param[] = {"all"};
    oal_uint16                       us_len;

     /*��״̬��ֵ*/
     *(oal_uint8 *)(st_write_msg.auc_value) = (oal_uint8)l_always_rx;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_ALWAYS_RX, OAL_SIZEOF(oal_uint8));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_uint8),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }

    if (l_always_rx)
    {
         /*��ӡ���мĴ���*/
        /***************************************************************************
                                    ���¼���wal�㴦��
        ***************************************************************************/
        if (EOK != memcpy_s(st_write_msg.auc_value, OAL_STRLEN((oal_int8*)auc_param),
                             auc_param, OAL_STRLEN((oal_int8*)auc_param))) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_always_rx::memcpy fail!");
            return OAL_FAIL;
        }

        st_write_msg.auc_value[OAL_STRLEN((oal_int8*)auc_param)] = '\0';
        us_len = (oal_uint16)(OAL_STRLEN((oal_int8*)auc_param) + 1);

        WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REG_INFO, us_len);

        l_ret = wal_send_cfg_event_etc(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (oal_uint8 *)&st_write_msg,
                                   OAL_FALSE,
                                   OAL_PTR_NULL);

        if (OAL_UNLIKELY(OAL_SUCC != l_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_always_rx::return err code [%d]!}\r\n", l_ret);
        }
    }
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_pm_switch(oal_net_device_stru *pst_net_dev, oal_int32 l_pm_switch)
{
    wal_msg_write_stru          st_write_msg;

    oal_int32                   l_ret;
    oal_uint8                   sta_pm_on[5] = " 0 ";

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_pm_switch:l_pm_switch[%d]", l_pm_switch);

    *(oal_uint8 *)(st_write_msg.auc_value) = (oal_uint8)l_pm_switch;

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_PM_SWITCH, OAL_SIZEOF(oal_int32));

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(oal_int32),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_pm_switch::return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }
#ifdef _PRE_WLAN_FEATURE_STA_PM
    l_ret = wal_hipriv_sta_pm_on_etc(pst_net_dev, sta_pm_on);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_set_pm_switch::CMD_SET_STA_PM_ON return err code [%d]!}\r\n", l_ret);
        return l_ret;
    }

#endif

    return OAL_SUCC;

}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_get_rx_rssi(oal_net_device_stru *pst_net_dev, oal_int32 *pl_rx_rssi)
{
    oal_int32                   l_ret;
    mac_cfg_rx_fcs_info_stru   *pst_rx_fcs_info;
    wal_msg_write_stru          st_write_msg;
    mac_vap_stru               *pst_mac_vap;
    hmac_vap_stru              *pst_hmac_vap;
    oal_int32                   i_leftime;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_rssi::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_atcmsrv_ioctl_get_rx_rssi::mac_res_get_hmac_vap failed!}");
        return OAL_FAIL;
    }

    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag = OAL_FALSE;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RX_FCS_INFO, OAL_SIZEOF(mac_cfg_rx_fcs_info_stru));

    /* ��������������� */
    pst_rx_fcs_info = (mac_cfg_rx_fcs_info_stru *)(st_write_msg.auc_value);
    /*������������02�Ѿ�û������*/
    pst_rx_fcs_info->ul_data_op    = 0;
    pst_rx_fcs_info->ul_print_info = 0;

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + OAL_SIZEOF(mac_cfg_rx_fcs_info_stru),
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_rssi::return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag),WAL_ATCMDSRB_GET_RX_PCKT);

    if ( 0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_rssi::get_rssi wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_rssi::get_rssi wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else
    {
        /* ��������  */
        *pl_rx_rssi = (oal_int)pst_hmac_vap->st_atcmdsrv_get_status.s_rx_rssi;
        /*lint -e571*/
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmsrv_ioctl_get_rx_rssi::get rssi [%d]!}", *pl_rx_rssi);
        /*lint -e571*/
        return OAL_SUCC;
    }
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_chipcheck(oal_net_device_stru *pst_net_dev, oal_int32 l_runing_test_mode)
{
    oal_int32                ul_ret;
    ul_ret = wlan_device_mem_check_etc(l_runing_test_mode);

    return ul_ret;
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_get_chipcheck_result(oal_net_device_stru *pst_net_dev, oal_int32 *l_chipcheck_result)
{

    *l_chipcheck_result = wlan_device_mem_check_result_etc(&ull_chipcheck_total_time_etc);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_get_chipcheck_result:result[%d]",*l_chipcheck_result);
    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_get_chipcheck_time(oal_net_device_stru *pst_net_dev, oal_uint64 *ull_chipcheck_time)
{
    *ull_chipcheck_time = ull_chipcheck_total_time_etc;

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_get_chipcheck_time:[%d]",ull_chipcheck_total_time_etc);
    return OAL_SUCC;
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_uart_loop(oal_net_device_stru *pst_net_dev, oal_int32 *l_uart_loop_set)
{
    return conn_test_uart_loop_etc((oal_int8 *)&l_uart_loop_set);
}

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_wifi_chan_loop(oal_net_device_stru *pst_net_dev, oal_int32 *l_wifi_chan_loop_set)
{
    return conn_test_wifi_chan_loop((oal_int8 *)&l_wifi_chan_loop_set);
}

OAL_STATIC oal_int32 wal_atcmdsrv_efuse_regs_read(oal_net_device_stru *pst_net_dev)
{
    oal_int32                   l_ret;
    wal_msg_write_stru          st_write_msg;
    mac_vap_stru               *pst_mac_vap;
    hmac_vap_stru              *pst_hmac_vap;
    oal_int32                   i_leftime;
    oal_uint8                   auc_param[] = {"efuse"};
    oal_uint16                  us_len;

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{wal_atcmdsrv_efuse_regs_read::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,"{wal_atcmdsrv_efuse_regs_read::mac_res_get_hmac_vap failed!}");
        return -OAL_EINVAL;
    }

    pst_hmac_vap->st_atcmdsrv_get_status.uc_report_efuse_reg_flag = OAL_FALSE;

     /*��ӡ���мĴ���*/
    /***************************************************************************
                                ���¼���wal�㴦��
    ***************************************************************************/
    if (EOK != memcpy_s(st_write_msg.auc_value, OAL_STRLEN((oal_int8*)auc_param),
                         auc_param, OAL_STRLEN((oal_int8*)auc_param))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "wal_atcmdsrv_efuse_regs_read::memcpy fail!");
        return -OAL_EINVAL;
    }
    st_write_msg.auc_value[OAL_STRLEN((oal_int8*)auc_param)] = '\0';

    us_len = (oal_uint16)(OAL_STRLEN((oal_int8*)auc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_REG_INFO, us_len);

    l_ret = wal_send_cfg_event_etc(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (oal_uint8 *)&st_write_msg,
                               OAL_FALSE,
                               OAL_PTR_NULL);

    if (OAL_UNLIKELY(OAL_SUCC != l_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{wal_atcmdsrv_efuse_regs_read::return err code [%d]!}\r\n", l_ret);
        return -OAL_EINVAL;
    }

    /*�����ȴ�dmac�ϱ�*/
    i_leftime = OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(pst_hmac_vap->query_wait_q,(oal_uint32)(OAL_TRUE == pst_hmac_vap->st_atcmdsrv_get_status.uc_report_efuse_reg_flag),WAL_ATCMDSRB_DBB_NUM_TIME);

    if (0 == i_leftime)
    {
        /* ��ʱ��û���ϱ�ɨ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmdsrv_efuse_regs_read::efuse_regs wait for %ld ms timeout!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else if (i_leftime < 0)
    {
        /* ��ʱ���ڲ����� */
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_atcmdsrv_efuse_regs_read::efuse_regs wait for %ld ms error!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000)/OAL_TIME_HZ));
        return -OAL_EINVAL;
    }
    else
    {
        return OAL_SUCC;
    }

}

OAL_STATIC void wal_atcmdsrv_efuse_info_print(void)
{
    oal_uint32 loop         = 0;
    oal_uint32 high_bit     = WAL_ATCMDSRV_EFUSE_REG_WIDTH - 1;
    oal_uint32 low_bit      = 0;
    for (loop = 0; loop < WAL_ATCMDSRV_EFUSE_BUFF_LEN; loop++)
    {
        OAM_WARNING_LOG3(0,0,"HI1103_DIE_ID: ATE bits:[%d:%d] value[0x%x]",high_bit,low_bit,g_us_efuse_buffer_etc[loop]);
        high_bit += WAL_ATCMDSRV_EFUSE_REG_WIDTH;
        low_bit  += WAL_ATCMDSRV_EFUSE_REG_WIDTH;
    }
}

OAL_STATIC oal_int32 wal_atcmdsrv_ioctl_efuse_bits_check(void)
{
    oal_int32 result     = OAL_SUCC;

    st_efuse_bits_etc = (wal_efuse_bits*)g_us_efuse_buffer_etc;

    /*��ӡ����efuse�ֶ�*/
    wal_atcmdsrv_efuse_info_print();

    /*******************************************
      efuse rd_data10ӳ��λ��Ϊ
        IR 15:15
        GNSS 14:14
        FM 13:13
        BT 12:12
        WIIF 11:11
        CHIP_VER 10:8
        CHID_ID 7:0
    *******************************************/
    if (WAL_ATCMDSRV_EFUSE_CHIP_ID != st_efuse_bits_etc->efuse_rd_10_11.chip_id)
    {
        OAM_ERROR_LOG1(0,0,"wal_atcmdsrv_ioctl_efuse_bits_check::chip ver expect value[0x3] error value[0x%x]\n",st_efuse_bits_etc->efuse_rd_10_11.chip_id);
        result = -OAL_EINVAL;
    }

    return result;
}


OAL_STATIC oal_int32 wal_atcmsrv_ioctl_dieid_inform(oal_net_device_stru *pst_net_dev, oal_uint16 *pl_die_id)
{
    oal_int32    l_ret;
    oal_uint16                               ul_loop = 0;

    /*��ȡefuse�ֶ�*/
    l_ret = wal_atcmdsrv_efuse_regs_read(pst_net_dev);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_efuse_check:get efuse reg fail");
        return l_ret;
    }
    /*�ϱ�efuse�ֶ�*/
    for(ul_loop = 0;ul_loop < 16;ul_loop++)
    {
        pl_die_id[ul_loop] = g_us_efuse_buffer_etc[ul_loop];
    }
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_atcmsrv_ioctl_efuse_check(oal_net_device_stru *pst_net_dev, oal_int32 *pl_efuse_check_result)
{
    oal_int32    l_ret = OAL_SUCC;

    /*��ȡefuse�ֶ�*/
    l_ret = wal_atcmdsrv_efuse_regs_read(pst_net_dev);
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_efuse_check:get efuse reg fail");
        *pl_efuse_check_result = 1;
        return l_ret;
    }
    /*���efuse�ֶ�*/
    l_ret = wal_atcmdsrv_ioctl_efuse_bits_check();
    if(OAL_SUCC != l_ret)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_efuse_check:check efuse reg fail");
        *pl_efuse_check_result = 1;
    }

    return l_ret;
}

#ifdef _PRE_WLAN_FEATURE_M2S

OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_mimo_ch(oal_net_device_stru *pst_net_dev, oal_int32 l_mimo_channel)
{
    oal_int32                        l_ret = OAL_SUCC;
    oal_int8                         pc_param[6] = "3 2 0";

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_set_mimo_ch::mimo_channel[%d]", l_mimo_channel);

    if (l_mimo_channel == 1)
    {
        pc_param[2] = '0';
    }
    else
    {
        pc_param[2] = '1';

    }

    /* "3 0/1/2 0" for mimo/siso switch */

    l_ret = wal_hipriv_set_m2s_switch(pst_net_dev, pc_param);
    return l_ret;
}
#endif


OAL_STATIC oal_int32 wal_atcmsrv_ioctl_get_dp_init(oal_net_device_stru *pst_net_dev, oal_int8 *c_dp_init)
{
    oal_int32                        l_ret = OAL_SUCC;
    oal_uint8                        uc_i;
    hi1103_cali_param_stru           *pst_cali_data_c0;
    hi1103_cali_param_stru           *pst_cali_data_c1;

    /* 1.��ȡNV��dp_init�Ƿ�յı�־λ */
    c_dp_init[0] = en_nv_dp_init_is_null;
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "wal_atcmsrv_ioctl_get_dp_init::dp_init_status[%d]", c_dp_init[0]);

    /* 2.��Ϊ�գ���device�������ϱ�,AT����д��NV */
    if (c_dp_init[0] == OAL_TRUE)
    {
        /* ά�� */
        hmac_dump_cali_result_etc();

        pst_cali_data_c0 = (hi1103_cali_param_stru *)get_cali_data_buf_addr_etc();
        pst_cali_data_c1 = pst_cali_data_c0 + 1;

        if ((NULL == pst_cali_data_c0) || (NULL == pst_cali_data_c1))
        {
            OAM_WARNING_LOG0(0, 0, "{wal_atcmsrv_ioctl_get_dp_init::null ptr!}");
            l_ret = OAL_FAIL;
            return l_ret;
        }

        for (uc_i = 0; uc_i < DP_INIT_EACH_CORE_NUM; uc_i++)
        {
            /* ���ڹ���У׼��13���ŵ��޸�Ϊ3���ŵ��������ϲ�ӿڻ���13��������ʱ����dpinit��ȫ������Ϊ0���ӿ�Ԥ�� */
            c_dp_init[uc_i+DP_INIT_FLAG_NUM] = 0; //pst_cali_data_c0->ast_2Gcali_param.st_cali_tx_power_cmp_2G[uc_i].c_dp_init;      //core 0 ��dp_init
            c_dp_init[uc_i+DP_INIT_FLAG_NUM+DP_INIT_EACH_CORE_NUM] = 0; //pst_cali_data_c1->ast_2Gcali_param.st_cali_tx_power_cmp_2G[uc_i].c_dp_init;    // core 1��dp_init
        }
    }

    return l_ret;
}


OAL_STATIC oal_int32 wal_atcmsrv_ioctl_get_pd_tran_param(oal_net_device_stru *pst_net_dev, oal_int8 *c_pd_param)
{
    oal_int32                        l_ret = OAL_SUCC;
    oal_uint32                       ul_param_num = (oal_uint32)c_pd_param[WAL_ATCMDSRV_NV_WINVRAM_LENGTH-1];
    oal_int8                         *pc_pd_info;

    OAM_WARNING_LOG1(0, 0, "wal_atcmsrv_ioctl_get_pd_tran_param::s_pd_param end element is %d", c_pd_param[WAL_ATCMDSRV_NV_WINVRAM_LENGTH-1]);

    if (ul_param_num >= WLAN_CFG_DTS_NVRAM_END)
    {
        OAM_ERROR_LOG1(0, 0, "wal_atcmsrv_ioctl_get_pd_tran_param::num[%d] larger than array_row", ul_param_num);
        return OAL_FAIL;
    }

    pc_pd_info = (oal_int8 *)hwifi_get_nvram_param(ul_param_num);
    if (NULL == pc_pd_info)
    {
        OAM_WARNING_LOG0(0, 0, "wal_atcmsrv_ioctl_get_pd_tran_param::null ptr!");
        return OAL_FAIL;
    }

    if (EOK != memcpy_s(c_pd_param, OAL_MIN(WAL_ATCMDSRV_NV_WINVRAM_LENGTH, CUS_PARAMS_LEN_MAX),
                        pc_pd_info, OAL_MIN(WAL_ATCMDSRV_NV_WINVRAM_LENGTH, CUS_PARAMS_LEN_MAX))) {
        OAM_ERROR_LOG0(0, 0, "wal_atcmsrv_ioctl_get_pd_tran_param::memcpy fail!");
        return OAL_FAIL;
    }

    OAL_IO_PRINT("wal_atcmsrv_ioctl_get_pd_tran_param::pd_param is %s", c_pd_param);
    return l_ret;
}

OAL_STATIC oal_int32 wal_atcmsrv_ioctl_io_test(oal_net_device_stru *pst_net_dev, oal_int16 *s_wkup_io_status)
{
    oal_int32     l_ret;
    OAM_WARNING_LOG0(0, 0, "{wal_atcmsrv_ioctl_io_test::func entered}");
    l_ret = hi1103_dev_io_test();
    if (l_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG1(0, 0, "{wal_atcmsrv_ioctl_io_test::io test ret is %d}", l_ret);
        *s_wkup_io_status = 1;
    }

    return l_ret;
}

OAL_STATIC oal_int32 wal_atcmdsrv_ioctl_pcie_ip_test(oal_net_device_stru *pst_net_dev, oal_int16 *s_pcie_status)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    oal_int32     l_ret;
    oal_int32     l_test_count = 1;

    OAM_WARNING_LOG0(0, 0, "{wal_atcmdsrv_ioctl_pcie_ip_test::func entered}");

    l_ret = hi1103_pcie_ip_test(l_test_count);
    if (l_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG1(0, 0, "{wal_atcmdsrv_ioctl_pcie_ip_test::pcie_ip test ret is %d}", l_ret);
        *s_pcie_status = 1;
    }

    return l_ret;
#endif
}

OAL_STATIC oal_int32 wal_atcmdsrv_ioctl_dyn_intervel_set(oal_net_device_stru *pst_net_dev, oal_int8 *c_dyn_intv)
{
    oal_int32     l_ret = -1;
    oal_int8      c_dyn_cmd[WAL_ATCMDSRV_WIFI_CHAN_NUM][WAL_ATCMDSRV_DYN_INTVAL_LEN] = {"2g_dscr_interval  ", "5g_dscr_interval  "};
    oal_int8      c_i = 0;

    for (c_i = 0; c_i < WAL_ATCMDSRV_WIFI_CHAN_NUM; c_i++)
    {
        c_dyn_cmd[c_i][WAL_ATCMDSRV_DYN_INTVAL_LOCATION] = c_dyn_intv[c_i];
        l_ret = (oal_int32)wal_hipriv_dyn_cali_cfg(pst_net_dev, &c_dyn_cmd[c_i][0]);
        if (l_ret != OAL_SUCC)
        {
            OAM_WARNING_LOG1(0, 0, "{wal_atcmdsrv_ioctl_dyn_intervel_set::ret is %d}", l_ret);
        }
    }

    return l_ret;
}

OAL_STATIC oal_int32 wal_atcmdsrv_ioctl_selfcali_intervel_set(oal_net_device_stru *pst_net_dev, oal_int32 l_selfcali_en)
{
    /* selcali_en: 1 for sel_cali every time, 0 for never sel_cal */
    if (!l_selfcali_en)
    {
        wlan_cal_disable_switch = OAL_TRUE;
    }
    if (l_selfcali_en)
    {
        wlan_cal_intvl_enable = OAL_FALSE;     /* ʹ��ÿ�����µ���У׼ */
        wlan_cal_disable_switch = OAL_FALSE;
    }

    OAM_WARNING_LOG1(0, 0, "wal_atcmdsrv_ioctl_selfcali_intervel_set::set selfcali_interval to [%d]!", l_selfcali_en);
    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_atcmdsrv_ioctl_pt_set(oal_net_device_stru *pst_net_dev, oal_int32 l_pt_set)
{
    if (l_pt_set)
    {
        OAM_WARNING_LOG0(0, 0, "pt_test pull up power_on_enable gpio!\n");
        hi1103_chip_power_on();
    }
    else
    {
        OAM_WARNING_LOG0(0, 0, "pt_test pull down power_on_enable gpio!\n");
        hi1103_chip_power_off();
    }

    return OAL_SUCC;
}

OAL_STATIC oal_int32 wal_atcmdsrv_ioctl_tas_ant_set(oal_net_device_stru *pst_net_dev, oal_int32 l_tas_ant_set)
{

    board_wifi_tas_set(l_tas_ant_set);

    return OAL_SUCC;

}


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_set_bss_expire_age(oal_net_device_stru *pst_net_dev, oal_uint32 ul_expire_age)
{
    g_pd_bss_expire_time_etc= (ul_expire_age < WAL_ATCMSRV_MIN_BSS_EXPIRATION_AGE)?WAL_ATCMSRV_MIN_BSS_EXPIRATION_AGE:ul_expire_age;
    g_pd_bss_expire_time_etc= (g_pd_bss_expire_time_etc> WAL_ATCMSRV_MAX_BSS_EXPIRATION_AGE)?WAL_ATCMSRV_MAX_BSS_EXPIRATION_AGE:g_pd_bss_expire_time_etc;

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "wal_atcmsrv_ioctl_set_bss_expire_age::pd_bss_expire_time %d, input expire time %d",
                        g_pd_bss_expire_time_etc, ul_expire_age);

    return OAL_SUCC;
}


OAL_STATIC oal_int32  wal_atcmsrv_ioctl_get_wifi_connect_info(oal_net_device_stru *pst_net_dev,struct wal_atcmdsrv_wifi_connect_info *pst_connect_info)
{
    mac_vap_stru                *pst_mac_vap;
    hmac_vap_stru               *pst_hmac_vap;
    hmac_device_stru            *pst_hmac_device;
    hmac_bss_mgmt_stru          *pst_bss_mgmt;          /*����ɨ���bss����Ľṹ�� */
    hmac_scanned_bss_info       *pst_scanned_bss_info   = OAL_PTR_NULL;

    if (OAL_ANY_NULL_PTR2(pst_net_dev,pst_connect_info))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "wal_atcmsrv_ioctl_get_wifi_connect_info::null pointer. net_dev %p, connect_info %p",
                            (uintptr_t)pst_net_dev, (uintptr_t)pst_connect_info);
        return -OAL_EFAIL;
    }

    memset_s(pst_connect_info, OAL_SIZEOF(*pst_connect_info), 0, OAL_SIZEOF(*pst_connect_info));

    pst_mac_vap = OAL_NET_DEV_PRIV(pst_net_dev);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0,OAM_SF_ANY,"{wal_atcmsrv_ioctl_get_wifi_connect_info::vap is null.}");
        return -OAL_EINVAL;
    }

    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)
    {
        OAM_ERROR_LOG1(0,OAM_SF_ANY,"{wal_atcmsrv_ioctl_get_wifi_connect_info::invalid vap mode.vap_mode [%d]}", pst_mac_vap->en_vap_mode);
        return -OAL_EINVAL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP)
    {

        pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_PTR_NULL == pst_hmac_vap)
        {
            OAM_ERROR_LOG1(0,OAM_SF_ANY,"{wal_atcmsrv_ioctl_get_wifi_connect_info::mac_res_get_hmac_vap fail.vap_id[%u]}",pst_mac_vap->uc_vap_id);
            return -OAL_EINVAL;
        }

        /* ��ȡhmac device �ṹ */
        pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
        if (OAL_PTR_NULL == pst_hmac_device)
        {
            OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{wal_atcmsrv_ioctl_get_wifi_connect_info::hmac_device is null.}");
            return -OAL_EINVAL;
        }

        pst_connect_info->en_status = ATCMDSRV_WIFI_CONNECTED;
        pst_connect_info->c_rssi    = pst_hmac_vap->station_info.signal;
        memcpy_s(pst_connect_info->auc_bssid, WLAN_MAC_ADDR_LEN, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);

        /* ��ȡ����ɨ���bss����Ľṹ�� */
        pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
        /* ������ɾ����ǰ����*/
        oal_spin_lock(&(pst_bss_mgmt->st_lock));
        pst_scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid_etc(pst_bss_mgmt, pst_connect_info->auc_bssid);
        if (OAL_PTR_NULL == pst_scanned_bss_info)
        {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{wal_atcmsrv_ioctl_get_wifi_connect_info::find the bss failed by bssid:%02X:XX:XX:%02X:%02X:%02X}",
                             pst_connect_info->auc_bssid[0],
                             pst_connect_info->auc_bssid[3],
                             pst_connect_info->auc_bssid[4],
                             pst_connect_info->auc_bssid[5]);

            /* ���� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));
        return -OAL_EINVAL;
        }
        /* ����*/
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));

        if (EOK != memcpy_s(pst_connect_info->auc_ssid, WLAN_SSID_MAX_LEN, pst_scanned_bss_info->st_bss_dscr_info.ac_ssid, WLAN_SSID_MAX_LEN)) {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "wal_atcmsrv_ioctl_get_wifi_connect_info::memcpy fail!");
            return -OAL_EINVAL;
        }
    }
    else
    {
        pst_connect_info->en_status = ATCMDSRV_WIFI_DISCONNECT;
    }

        OAM_WARNING_LOG4(0, OAM_SF_CFG, "wal_atcmsrv_ioctl_get_wifi_connect_info::state %d, rssi %d, BSSID[XX:XX:XX:XX:%02X:%02X]",
                        pst_connect_info->en_status, pst_connect_info->c_rssi,
                        pst_connect_info->auc_bssid[4], pst_connect_info->auc_bssid[5]);

    return OAL_SUCC;
}


OAL_STATIC oal_int32 wal_atcmsrv_ioctl_pcie_sdio_set(oal_net_device_stru *pst_net_dev, oal_int16 s_pcie_sdio_set)
{
    oal_int32 ret;
    oal_uint8 input_1[200]="sdio";
    oal_uint8 input_2[200]="pcie";


    if(1 == s_pcie_sdio_set)
    {
        ret = conn_test_hcc_chann_switch(input_1);
        if(OAL_SUCC != ret)
        {
            return -OAL_EINVAL;
        }
    }
    else if(2 == s_pcie_sdio_set)
    {
        ret = conn_test_hcc_chann_switch(input_2);
        if(OAL_SUCC != ret)
        {
            return -OAL_EINVAL;
        }
    }
    else
    {
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}

oal_int32 wal_atcmdsrv_wifi_priv_cmd_etc(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, oal_int32 ul_cmd)
{
    wal_atcmdsrv_wifi_priv_cmd_stru  st_priv_cmd;
    oal_int32    l_ret              = OAL_SUCC;


    if (OAL_ANY_NULL_PTR2(pst_ifr->ifr_data,pst_net_dev))
    {
        l_ret = -OAL_EINVAL;
        return l_ret;
    }
    /*���û�̬���ݿ������ں�̬*/
    if (oal_copy_from_user(&st_priv_cmd, pst_ifr->ifr_data, sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
    {
        l_ret = -OAL_EINVAL;
        return l_ret;
    }

    switch(st_priv_cmd.ul_cmd)
    {
        case WAL_ATCMDSRV_IOCTL_CMD_WI_FREQ_SET:
             l_ret = wal_atcmsrv_ioctl_set_freq(pst_net_dev,st_priv_cmd.pri_data.l_freq);
             break;
        case WAL_ATCMDSRV_IOCTL_CMD_WI_POWER_SET:
             l_ret = wal_atcmsrv_ioctl_set_txpower(pst_net_dev,st_priv_cmd.pri_data.l_pow);
             break;
        case WAL_ATCMDSRV_IOCTL_CMD_MODE_SET:
             l_ret = wal_atcmsrv_ioctl_set_mode(pst_net_dev,st_priv_cmd.pri_data.l_mode);
             break;
        case WAL_ATCMDSRV_IOCTL_CMD_DATARATE_SET:
             l_ret = wal_atcmsrv_ioctl_set_datarate(pst_net_dev,st_priv_cmd.pri_data.l_datarate);
             break;
        case WAL_ATCMDSRV_IOCTL_CMD_BAND_SET:
             l_ret = wal_atcmsrv_ioctl_set_bandwidth(pst_net_dev,st_priv_cmd.pri_data.l_bandwidth);
             break;
        case WAL_ATCMDSRV_IOCTL_CMD_ALWAYS_TX_SET:
             l_ret = wal_atcmsrv_ioctl_set_always_tx(pst_net_dev,st_priv_cmd.pri_data.l_awalys_tx);
             break;
        case WAL_ATCMDSRV_IOCTL_CMD_DBB_GET:
             l_ret = wal_atcmsrv_ioctl_get_dbb_num(pst_net_dev,st_priv_cmd.pri_data.auc_dbb);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_HW_STATUS_GET:
            l_ret = wal_atcmsrv_ioctl_get_hw_status_etc(pst_net_dev,&st_priv_cmd.pri_data.l_fem_pa_status);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_ALWAYS_RX_SET:
            l_ret = wal_atcmsrv_ioctl_set_always_rx(pst_net_dev,st_priv_cmd.pri_data.l_awalys_rx);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_HW_ADDR_SET:
            l_ret = wal_atcmsrv_ioctl_set_hw_addr_etc(pst_net_dev,st_priv_cmd.pri_data.auc_mac_addr);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET:
            l_ret = wal_atcmsrv_ioctl_get_rx_pckg_etc(pst_net_dev,&st_priv_cmd.pri_data.l_rx_pkcg);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_PM_SWITCH:
            l_ret = wal_atcmsrv_ioctl_set_pm_switch(pst_net_dev,st_priv_cmd.pri_data.l_pm_switch);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_RX_RSSI:
            l_ret = wal_atcmsrv_ioctl_get_rx_rssi(pst_net_dev,&st_priv_cmd.pri_data.l_rx_rssi);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET:
            l_ret = wal_atcmsrv_ioctl_set_chipcheck(pst_net_dev,st_priv_cmd.pri_data.l_runingtest_mode);
            break;
         case WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_RESULT:
            l_ret = wal_atcmsrv_ioctl_get_chipcheck_result(pst_net_dev,&st_priv_cmd.pri_data.l_chipcheck_result);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
         case WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_TIME:
            l_ret = wal_atcmsrv_ioctl_get_chipcheck_time(pst_net_dev,&st_priv_cmd.pri_data.ull_chipcheck_time);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
         case WAL_ATCMDSRV_IOCTL_CMD_UART_LOOP_SET:
            l_ret = wal_atcmsrv_ioctl_set_uart_loop(pst_net_dev,&st_priv_cmd.pri_data.l_uart_loop_set);
            break;
         case WAL_ATCMDSRV_IOCTL_CMD_SDIO_LOOP_SET:
            l_ret = wal_atcmsrv_ioctl_set_wifi_chan_loop(pst_net_dev,&st_priv_cmd.pri_data.l_wifi_chan_loop_set);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_EFUSE_CHECK:
            l_ret = wal_atcmsrv_ioctl_efuse_check(pst_net_dev,&st_priv_cmd.pri_data.l_efuse_check_result);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_DIEID_INFORM:
             l_ret = wal_atcmsrv_ioctl_dieid_inform(pst_net_dev,(oal_uint16*)st_priv_cmd.pri_data.die_id);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_SET_COUNTRY:
            l_ret = wal_atcmsrv_ioctl_set_country(pst_net_dev,st_priv_cmd.pri_data.auc_country_code);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_WIPIN_TEST:
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
            l_ret = wal_atcmsrv_ioctl_wipin_test(pst_net_dev,&st_priv_cmd.pri_data.l_pin_status);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
#endif
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_SET_MIMO_CHANNEL:
    #ifdef _PRE_WLAN_FEATURE_M2S
            l_ret = wal_atcmsrv_ioctl_set_mimo_ch(pst_net_dev,st_priv_cmd.pri_data.l_mimo_channel);
    #endif
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_GET_DP_INIT:
            l_ret = wal_atcmsrv_ioctl_get_dp_init(pst_net_dev,st_priv_cmd.pri_data.ac_dp_init);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_GET_PDET_PARAM:
            l_ret = wal_atcmsrv_ioctl_get_pd_tran_param(pst_net_dev,st_priv_cmd.pri_data.ac_pd_tran_param);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;

        case WAL_ATCMDSRV_IOCTL_CMD_IO_TEST:
            l_ret = wal_atcmsrv_ioctl_io_test(pst_net_dev,&st_priv_cmd.pri_data.s_wkup_io_status);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;

        case WAL_ATCMDSRV_IOCTL_CMD_PCIE_TEST:
            l_ret = wal_atcmdsrv_ioctl_pcie_ip_test(pst_net_dev,&st_priv_cmd.pri_data.s_pcie_status);
            if(oal_copy_to_user(pst_ifr->ifr_data,&st_priv_cmd,sizeof(wal_atcmdsrv_wifi_priv_cmd_stru)))
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY,"wal_atcmdsrv_wifi_priv_cmd_etc:Failed to copy ioctl_data to user !");
                l_ret = -OAL_EINVAL;
            }
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_PCIE_SDIO_SET:
            l_ret = wal_atcmsrv_ioctl_pcie_sdio_set(pst_net_dev,st_priv_cmd.pri_data.s_pcie_sdio_set);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_DYN_INTERVAL:
            l_ret = wal_atcmdsrv_ioctl_dyn_intervel_set(pst_net_dev, st_priv_cmd.pri_data.c_dyn_interval);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_PT_STE:
            l_ret = wal_atcmdsrv_ioctl_pt_set(pst_net_dev, st_priv_cmd.pri_data.l_pt_set);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_TAS_ANT_SET:
            l_ret = wal_atcmdsrv_ioctl_tas_ant_set(pst_net_dev, st_priv_cmd.pri_data.l_tas_ant_set);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_SELFCALI_INTERVAL:
            l_ret = wal_atcmdsrv_ioctl_selfcali_intervel_set(pst_net_dev, st_priv_cmd.pri_data.l_selfcali_interval_set);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_SET_BSS_EXPIRE_AGE:
            l_ret = wal_atcmsrv_ioctl_set_bss_expire_age(pst_net_dev, st_priv_cmd.pri_data.ul_bss_expire_age);
            break;
        case WAL_ATCMDSRV_IOCTL_CMD_GET_CONN_INFO:
            l_ret = wal_atcmsrv_ioctl_get_wifi_connect_info(pst_net_dev, &st_priv_cmd.pri_data.st_connect_info);

            OAL_IO_PRINT("wal_atcmsrv_ioctl_get_wifi_connect_info::status %d, %.32s,%02x:%02x:xx:xx:%02x:%02x,%d",
                         st_priv_cmd.pri_data.st_connect_info.en_status,
                         st_priv_cmd.pri_data.st_connect_info.auc_ssid,
                         st_priv_cmd.pri_data.st_connect_info.auc_bssid[0], st_priv_cmd.pri_data.st_connect_info.auc_bssid[1],
                         st_priv_cmd.pri_data.st_connect_info.auc_bssid[4], st_priv_cmd.pri_data.st_connect_info.auc_bssid[5],
                         st_priv_cmd.pri_data.st_connect_info.c_rssi);

            oal_copy_to_user(pst_ifr->ifr_data, &st_priv_cmd, sizeof(wal_atcmdsrv_wifi_priv_cmd_stru));
            break;
        default:
             break;
    }
    return l_ret;
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

