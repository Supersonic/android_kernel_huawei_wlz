

#ifndef __HMAC_CONFIG_H__
#define __HMAC_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 ����ͷ�ļ����� */
#include "oal_ext_if.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "hmac_device.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_H

/* 2 �궨�� */
#define HMAC_MAX_MCS_NUM               8 /* ������˫��֧�ֵ�mac������ */
#define HAMC_QUERY_INFO_FROM_DMAC_TIME (5 * OAL_TIME_HZ)
#ifdef _PRE_SUPPORT_ACS
#define HMAC_SET_RESCAN_TIMEOUT_CHECK(_pst_hmac_device, _ul_bak) \
    do {                                                         \
        if (!(_pst_hmac_device)->ul_rescan_timeout) {            \
            (_pst_hmac_device)->ul_rescan_timeout = (_ul_bak);   \
        }                                                        \
    } while (0)
#endif
/* 3 ö�ٶ��� */
/* APUT OWE group definition, hipriv.sh BIT format transit to pst_hmac_vap->owe_group */
#define WAL_HIPRIV_OWE_19 BIT(0)
#define WAL_HIPRIV_OWE_20 BIT(1)
#define WAL_HIPRIV_OWE_21 BIT(2)
/* 4 ȫ�ֱ������� */
/* 5 ��Ϣͷ���� */
/* 6 ��Ϣ���� */
/* 7 STRUCT���� */
/* hmac_vap�ṹ�У�һ���ֳ�Ա�Ĵ�С����Щ��Ա��linux��windows�µĶ�����ܲ�ͬ */
typedef struct {
    oal_uint32 ul_hmac_vap_cfg_priv_stru_size;
    oal_uint32 ul_frw_timeout_stru_size;
    oal_uint32 ul_oal_spin_lock_stru_size;
    oal_uint32 ul_mac_key_mgmt_stru_size;
    oal_uint32 ul_mac_pmkid_cache_stru_size;
    oal_uint32 ul_mac_curr_rateset_stru_size;
    oal_uint32 ul_hmac_vap_stru_size;
} hmac_vap_member_size_stru;

/* 8 UNION���� */
/* 9 OTHERS���� */
/* 10 �������� */
extern oal_uint32 hmac_config_start_vap_event_etc(mac_vap_stru *pst_mac_vap,
                                                  oal_bool_enum_uint8 en_mgmt_rate_init_flag);
extern oal_uint32 hmac_set_mode_event_etc(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_config_sta_update_rates_etc(mac_vap_stru *pst_mac_vap,
                                                   mac_cfg_mode_param_stru *pst_cfg_mode,
                                                   mac_bss_dscr_stru *pst_bss_dscr);
extern oal_uint32 hmac_event_config_syn_etc(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 hmac_event_log_syn(frw_event_mem_stru *pst_event_mem);

extern oal_uint32 hmac_protection_update_from_user(mac_vap_stru *pst_mac_vap,
                                                   oal_uint16 us_len,
                                                   oal_uint8 *puc_param);
extern oal_uint32 hmac_40M_intol_sync_event(mac_vap_stru *pst_mac_vap,
                                            oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_tlv_cmd(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                          oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_str_cmd(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                          oal_uint8 *puc_param);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32 hmac_get_thruput_info_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len,
                                            oal_uint8 *puc_param);
extern oal_void hcc_msg_slave_thruput_bypass_etc(oal_void);
oal_uint32 hmac_config_set_tx_ampdu_type(mac_vap_stru *pst_mac_vap, oal_uint16 us_len,
                                         oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_STA_PM
extern oal_uint32 hmac_config_sta_pm_on_syn(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_set_ipaddr_timeout_etc(void *puc_para);
#endif
extern oal_uint32 hmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap,
                                              oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
extern oal_uint32 hmac_config_enable_arp_offload(mac_vap_stru *pst_mac_vap,
                                                 oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_ip_addr_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_show_arpoffload_info(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
oal_uint32 hmac_config_roam_enable_etc(mac_vap_stru *pst_mac_vap,
                                       oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_org_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_band_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_start_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_roam_info_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_11R
oal_uint32 hmac_config_set_ft_ies_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif  // _PRE_WLAN_FEATURE_11R

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
oal_uint32 hmac_config_enable_2040bss_etc(mac_vap_stru *pst_mac_vap,
                                          oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_get_2040bss_sw(mac_vap_stru *pst_mac_vap,
                                      oal_uint16 *pus_len, oal_uint8 *puc_param);
#endif
oal_uint32 hmac_config_get_dieid_rsp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_get_dieid(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
oal_uint32 hmac_config_auto_cali(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_get_cali_status(mac_vap_stru *pst_mac_vap,
                                       oal_uint16 *pus_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cali_vref(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
oal_uint32 hmac_config_set_auto_freq_enable_etc(mac_vap_stru *pst_mac_vap,
                                                oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_FEATURE_WAVEAPP_CLASSIFY
oal_uint32 hmac_config_get_waveapp_flag_rsp(mac_vap_stru *pst_mac_vap,
                                            oal_uint8 uc_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_get_waveapp_flag(mac_vap_stru *pst_mac_vap,
                                        oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
oal_uint32 hmac_config_set_sta_pm_on_etc(mac_vap_stru *pst_mac_vap,
                                         oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_sta_pm_mode_etc(mac_vap_stru *pst_mac_vap,
                                           oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_fast_sleep_para_etc(mac_vap_stru *pst_mac_vap,
                                               oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
oal_uint32 hmac_config_load_ini_power_gain(mac_vap_stru *pst_mac_vap,
                                           oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_all_log_level_etc(mac_vap_stru *pst_mac_vap,
                                             oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_rf_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_dts_cali_etc(mac_vap_stru *pst_mac_vap,
                                            oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_nvram_params_etc(mac_vap_stru *pst_mac_vap,
                                                oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_dev_customize_info_etc(mac_vap_stru *pst_mac_vap,
                                              oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_set_cus_dyn_cali(mac_vap_stru *pst_mac_vap,
                                        oal_uint16 us_len, oal_uint8 *puc_param);
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
oal_uint32 hmac_scan_rrm_proc_save_bss_etc(mac_vap_stru *pst_mac_vap,
                                           oal_uint8 uc_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern oal_uint32 hmac_config_vowifi_report_etc(mac_vap_stru *pst_mac_vap,
                                                oal_uint8 uc_len, oal_uint8 *puc_param);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
extern oal_uint32 hmac_config_stop_altx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_WDS
oal_uint32 hmac_config_wds_vap_mode(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_wds_get_vap_mode(mac_vap_stru *pst_mac_vap,
                                        oal_uint16 *us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_wds_vap_show(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_wds_sta_add(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_wds_sta_del(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_wds_sta_age(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
oal_uint32 hmac_config_wds_get_sta_num(mac_vap_stru *pst_mac_vap,
                                       oal_uint16 *us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_11K_STAT
extern oal_uint32 hmac_config_query_stat_info(mac_vap_stru *pst_mac_vap,
                                              oal_uint16 us_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
extern oal_uint32 hmac_config_send_radio_meas_req(mac_vap_stru *pst_mac_vap,
                                                  oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_set_11k_switch(mac_vap_stru *pst_mac_vap,
                                             oal_uint16 us_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
extern oal_uint32  hmac_config_ftm_dbg(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32  hmac_ftm_rx_gas_init_resp(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf);

#endif
#ifdef _PRE_WLAN_FEATURE_APF
extern oal_uint32 hmac_config_apf_filter_cmd(mac_vap_stru *pst_mac_vap,
                                             oal_uint16 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 hmac_config_remove_app_ie(mac_vap_stru *pst_mac_vap,
                                            oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_FORCE_STOP_FILTER
extern oal_uint32 hmac_config_force_stop_filter(mac_vap_stru *pst_mac_vap,
                                                oal_uint16 us_len, oal_uint8 *puc_param);
#endif

extern oal_uint32 hmac_config_fem_lp_flag(mac_vap_stru *pst_mac_vap,
                                          oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_softap_mimo_mode(mac_vap_stru *pst_mac_vap,
                                               oal_uint16 us_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
extern oal_uint32 hmac_config_assigned_filter_etc(mac_vap_stru *pst_mac_vap,
                                                  oal_uint16 us_len, oal_uint8 *puc_param);
#endif

extern oal_uint32 hmac_config_set_owe_etc(mac_vap_stru *pst_mac_vap,
                                          oal_uint16 us_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_TWT
extern oal_uint32 hmac_get_chip_vap_num(mac_chip_stru *pst_chip);
extern oal_uint32 hmac_config_twt_setup_req_auto(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_config_twt_teardown_req_auto(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_config_twt_setup_req_etc(mac_vap_stru *pst_mac_vap,
                                                oal_uint16 us_len, oal_uint8 *puc_param);
extern oal_uint32 hmac_config_twt_teardown_req_etc(mac_vap_stru *pst_mac_vap,
                                                   oal_uint16 us_len,
                                                   oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
extern oal_uint32 hmac_mbo_check_is_assoc_or_re_assoc_allowed(mac_vap_stru *pst_mac_vap,
                                                              mac_conn_param_stru *pst_connect_param,
                                                              mac_bss_dscr_stru *pst_bss_dscr);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */