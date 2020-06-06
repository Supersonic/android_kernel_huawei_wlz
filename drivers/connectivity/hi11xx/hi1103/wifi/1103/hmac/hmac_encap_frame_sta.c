


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "wlan_spec.h"
#include "wlan_mib.h"
#include "mac_vap.h"
#include "mac_frame.h"
#include "hmac_encap_frame_sta.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_scan.h"
#include "mac_user.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ENCAP_FRAME_STA_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R

oal_void mac_add_cowork_ie_etc(oal_void * pst_hmac_sta, oal_uint8 *puc_buffer, oal_uint16 *pus_ie_len)
{
    mac_bss_dscr_stru               *curr_bss_dscr;
    oal_uint8                       *puc_cowork_ie;
    hmac_vap_stru                   *pst_hmac_vap;
    oal_uint8                       *puc_frame_body;
    oal_int32                        us_frame_body_len;
    oal_uint16                       us_offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    oal_uint32                       ul_ie_oui = MAC_WLAN_OUI_HUAWEI;
    oal_sta_ap_cowork_ie cowork_ie = { MAC_EID_VENDOR, 18,
                                       { (ul_ie_oui >> 16) & 0xFF, (ul_ie_oui >> 8) & 0xFF, ul_ie_oui & 0xFF },
                                       MAC_WLAN_OUI_TYPE_HAUWEI_COWORK,
                                       TYPE_TLV_CAPABILITY, 2, 1, 0, 1, 0, 0, 0, 0, 0,
                                       TYPE_TLV_DC_ROAM_INFO, 8, { 0, 0, 0, 0, 0, 0 }, 0, 0 };

    *pus_ie_len = 0;
    if (OAL_PTR_NULL == pst_hmac_sta)
    {
        return;
    }

    pst_hmac_vap = (hmac_vap_stru *) pst_hmac_sta;
    if (WLAN_VAP_MODE_BSS_STA != pst_hmac_vap->st_vap_base_info.en_vap_mode)
    {
        return;
    }

    curr_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(&pst_hmac_vap->st_vap_base_info,
        pst_hmac_vap->st_vap_base_info.auc_bssid);

    if (OAL_PTR_NULL == curr_bss_dscr)
    {
        return;
    }

    if (OAL_TRUE != pst_hmac_vap->bit_adaptive11r)
    {
        return;
    }

    puc_frame_body = curr_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN;
    us_frame_body_len = curr_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN;
    puc_cowork_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_HUAWEI, MAC_WLAN_OUI_TYPE_HAUWEI_COWORK,
        puc_frame_body + us_offset, us_frame_body_len - us_offset);

    /* ������յ�beacon֡�����ڶ˹�ЭͬIE���򷵻� */
    if (OAL_PTR_NULL == puc_cowork_ie)
    {
        return;
    }

    if (EOK != memcpy_s(puc_buffer, sizeof(oal_sta_ap_cowork_ie), &cowork_ie, sizeof(oal_sta_ap_cowork_ie))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_add_cowork_ie_etc::memcpy fail!");
        return;
    }
    *pus_ie_len = sizeof(oal_sta_ap_cowork_ie);

    return;
}
#endif


hmac_scanned_bss_info* hmac_vap_get_scan_bss_info(mac_vap_stru *pst_mac_vap)
{
    hmac_device_stru       *pst_hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru     *pst_bss_mgmt = OAL_PTR_NULL;
    hmac_scanned_bss_info  *pst_scaned_bss = OAL_PTR_NULL;

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_hmac_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{mac_vap_get_scan_bss_info::pst_mac_device null.}");
        return OAL_PTR_NULL;
    }
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    pst_scaned_bss = hmac_scan_find_scanned_bss_by_bssid_etc(pst_bss_mgmt, pst_mac_vap->auc_bssid);
    if(OAL_PTR_NULL == pst_scaned_bss)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_tx_qos_enhance_list_init::do not have scan result!!!}");
        return OAL_PTR_NULL;
    }
    return pst_scaned_bss;
}


OAL_STATIC oal_bool_enum hmac_sta_check_need_set_ext_cap_ie(mac_vap_stru *pst_mac_vap)
{
    oal_uint8       *puc_ext_cap_ie;
    oal_uint16       us_ext_cap_index;

    puc_ext_cap_ie = hmac_sta_find_ie_in_probe_rsp_etc(pst_mac_vap, MAC_EID_EXT_CAPS, &us_ext_cap_index);
    if (OAL_PTR_NULL == puc_ext_cap_ie)
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}



oal_void  hmac_set_supported_rates_ie_asoc_req_etc(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{

    oal_uint8         uc_nrates;
    oal_uint8         uc_idx;

    /**************************************************************************
                        ---------------------------------------
                        |Element ID | Length | Supported Rates|
                        ---------------------------------------
             Octets:    |1          | 1      | 1~8            |
                        ---------------------------------------
    The Information field is encoded as 1 to 8 octets, where each octet describes a single Supported
    Rate or BSS membership selector.
    **************************************************************************/
    puc_buffer[0] = MAC_EID_RATES;

    uc_nrates = mac_mib_get_SupportRateSetNums(&pst_hmac_sta->st_vap_base_info);

    if (uc_nrates > MAC_MAX_SUPRATES)
    {
        uc_nrates = MAC_MAX_SUPRATES;
    }

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++)
    {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_hmac_sta->auc_supp_rates[uc_idx];
    }

    puc_buffer[1] = uc_nrates;

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}

oal_void hmac_set_exsup_rates_ie_asoc_req_etc(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8         uc_nrates;
    oal_uint8         uc_idx;


    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    if (mac_mib_get_SupportRateSetNums(&pst_hmac_sta->st_vap_base_info) <= MAC_MAX_SUPRATES)
    {
        *puc_ie_len = 0;

        return;
    }

    puc_buffer[0] = MAC_EID_XRATES;
    uc_nrates     = mac_mib_get_SupportRateSetNums(&pst_hmac_sta->st_vap_base_info) - MAC_MAX_SUPRATES;
    puc_buffer[1] = uc_nrates;

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++)
    {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_hmac_sta->auc_supp_rates[uc_idx + MAC_MAX_SUPRATES];
    }

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}
#ifdef _PRE_WLAN_FEATURE_M2S

oal_void hmac_assoc_set_siso_mode(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_req_frame)
{
    hmac_user_stru         *pst_hmac_user;
    mac_frame_ht_cap_stru  *pst_ht_capinfo;

    /* ���������ֻ֧��siso�����ĺ���������������SISO��ʽ���� */
    pst_hmac_user = mac_res_get_hmac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_hmac_user)
    {
        return;
    }
    if ((pst_hmac_user->en_user_ap_type & MAC_AP_TYPE_MIMO_BLACKLIST)
      && (0 == oal_compare_mac_addr(pst_hmac_user->auc_mimo_blacklist_mac, pst_mac_vap->auc_bssid)))
    {
        puc_req_frame += MAC_IE_HDR_LEN;

        pst_ht_capinfo = (mac_frame_ht_cap_stru *)puc_req_frame;
        pst_ht_capinfo->bit_sm_power_save = MAC_SMPS_STATIC_MODE;
        pst_ht_capinfo->bit_tx_stbc       = OAL_FALSE;

        puc_req_frame -= MAC_IE_HDR_LEN;
        OAM_WARNING_LOG0(0, OAM_SF_ASSOC, "{hmac_assoc_set_siso_mode::set smps to STATIC.}");
    }
}

oal_void hmac_update_user_sounding_dim_num(mac_vap_stru *pst_mac_vap, hmac_scanned_bss_info *pst_scaned_bss)
{
    oal_uint8         uc_num_sounding_dim;
    mac_user_stru    *pst_mac_user;
#if defined(_PRE_WLAN_FEATURE_11AX)
    mac_he_hdl_stru  *pst_he_hdl;
#endif

    /* VHT�е��ֶ���Ҫ����AP��bf�������ã����ڴ���ǰ����MAC_USER��bf����*/
    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);

    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_update_user_sounding_dim_num::pst_mac_user null.}");
        return;
    }
    uc_num_sounding_dim = pst_scaned_bss->st_bss_dscr_info.uc_num_sounding_dim;

    pst_mac_user->st_vht_hdl.bit_num_sounding_dim = uc_num_sounding_dim;
#if defined(_PRE_WLAN_FEATURE_11AX)
    pst_he_hdl   = MAC_USER_HE_HDL_STRU(pst_mac_user);
    if(pst_scaned_bss->st_bss_dscr_info.en_bw_cap <= WLAN_BW_CAP_80M )
    {
        pst_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_below_80mhz_sounding_dimensions_num = uc_num_sounding_dim;
    }
    else
    {
        pst_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_over_80mhz_sounding_dimensions_num = uc_num_sounding_dim;
    }
#endif
}
#endif
#ifdef _PRE_WLAN_FEATURE_TXBF

oal_void hmac_sta_roam_and_assoc_update_txbf_etc(mac_vap_stru *pst_mac_vap, mac_bss_dscr_stru *pst_mac_bss_dscr)
{
    /* linksys 2G �������豸�ر�txbf���� */
    if(MAC_IS_LINKSYS(pst_mac_vap->auc_bssid)&&
        OAL_TRUE == pst_mac_bss_dscr->en_txbf_blacklist_chip_oui && WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "hmac_sta_roam_and_assoc_update_txbf_etc: txbf blacklist!");
        hmac_config_vap_close_txbf_cap_etc(pst_mac_vap);
    }
}
#endif

oal_uint32 hmac_mgmt_encap_asoc_req_sta_etc(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_req_frame, oal_uint8 *puc_curr_bssid)
{
    oal_uint16              us_fc;
    oal_uint8               uc_ie_len            = 0;
    oal_uint32              us_asoc_rsq_len      = 0;
    oal_uint8              *puc_req_frame_origin = OAL_PTR_NULL;
    mac_vap_stru           *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru        *pst_mac_device = OAL_PTR_NULL;
    oal_uint16              us_app_ie_len        = 0;
    en_app_ie_type_uint8    en_app_ie_type;
    hmac_scanned_bss_info  *pst_scaned_bss = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    hmac_user_stru         *pst_hmac_user;
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_txbf_cap_stru *pst_txbf_cap;
#endif

#ifdef _PRE_WLAN_FEATURE_11R
        wlan_wme_ac_type_enum_uint8 en_aci;
        wlan_wme_ac_type_enum_uint8 en_target_ac;
        oal_uint8                   uc_tid;
#endif
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) || defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
        oal_bool_enum_uint8         en_rrm_enable = OAL_TRUE;
#endif

    if (OAL_ANY_NULL_PTR2(pst_hmac_sta,puc_req_frame))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_req_sta_etc::null param, pst_hmac_sta=%x puc_req_frame=%x.}",
                       (uintptr_t)pst_hmac_sta, (uintptr_t)puc_req_frame);
        return us_asoc_rsq_len;
    }

    /* ������ʼ��ַ��������㳤��*/
    puc_req_frame_origin = puc_req_frame;

    pst_mac_vap = &(pst_hmac_sta->st_vap_base_info);

    /* ��ȡdevice */
    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    pst_scaned_bss = hmac_vap_get_scan_bss_info(pst_mac_vap);

    if (OAL_ANY_NULL_PTR2(pst_mac_device,pst_scaned_bss))
    {
        OAM_ERROR_LOG0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_req_sta_etc::pst_mac_device or pst_scaned_bss null.}");
        return us_asoc_rsq_len;
    }
#ifdef _PRE_WLAN_FEATURE_11K
    pst_mac_vap->bit_bss_include_rrm_ie  = pst_scaned_bss->st_bss_dscr_info.en_support_rrm;
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_update_user_sounding_dim_num(pst_mac_vap,pst_scaned_bss);
#endif
#ifdef _PRE_WLAN_FEATURE_TXBF
    hmac_sta_roam_and_assoc_update_txbf_etc(pst_mac_vap, &pst_scaned_bss->st_bss_dscr_info);
#endif
    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    /* ���� Frame Control field */
    /* �ж��Ƿ�Ϊreassoc���� */
    us_fc = (OAL_PTR_NULL != puc_curr_bssid) ? WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_REASSOC_REQ :
                                               WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ASSOC_REQ;
    mac_hdr_set_frame_control(puc_req_frame, us_fc);

    /* ���� DA address1: AP MAC��ַ (BSSID)*/
    oal_set_mac_addr(puc_req_frame + WLAN_HDR_ADDR1_OFFSET, pst_hmac_sta->st_vap_base_info.auc_bssid);

    /* ���� SA address2: dot11MACAddress */
    oal_set_mac_addr(puc_req_frame + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(&pst_hmac_sta->st_vap_base_info));

    /* ���� DA address3: AP MAC��ַ (BSSID)*/
    oal_set_mac_addr(puc_req_frame + WLAN_HDR_ADDR3_OFFSET, pst_hmac_sta->st_vap_base_info.auc_bssid);

    puc_req_frame += MAC_80211_FRAME_LEN;

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Association Request Frame - Frame Body                   */
    /* --------------------------------------------------------------------- */
    /* | Capability Information | Listen Interval | SSID | Supported Rates | */
    /* --------------------------------------------------------------------- */
    /* |2                       |2                |2-34  |3-10             | */
    /* --------------------------------------------------------------------- */

    /* --------------------------------------------------------------------- */
    /* |Externed Surpported rates| Power Capability | Supported Channels   | */
    /* --------------------------------------------------------------------- */
    /* |3-257                    |4                 |4-256                 | */
    /* --------------------------------------------------------------------- */

    /* --------------------------------------------------------------------- */
    /* | RSN   | QoS Capability | HT Capabilities | Extended Capabilities  | */
    /* --------------------------------------------------------------------- */
    /* |36-256 |3               |28               |3-8                     | */
    /* --------------------------------------------------------------------- */

    /* --------------------------------------------------------------------- */
    /* | WPS   | P2P |                                                       */
    /* --------------------------------------------------------------------- */
    /* |7-257  |X    |                                                       */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    mac_set_cap_info_sta_etc((oal_void *)pst_mac_vap, puc_req_frame);
    puc_req_frame += MAC_CAP_INFO_LEN;

    /* ���� Listen Interval IE */
    mac_set_listen_interval_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* Ressoc��֡����Current AP address */
    if (OAL_PTR_NULL != puc_curr_bssid)
    {
        oal_set_mac_addr(puc_req_frame, puc_curr_bssid);
        puc_req_frame += OAL_MAC_ADDR_LEN;
    }
    /* ���� SSID IE */
    mac_set_ssid_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len, WLAN_FC0_SUBTYPE_ASSOC_REQ);
    puc_req_frame += uc_ie_len;
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    /* ���� Supported Rates IE */
    mac_set_supported_rates_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* ���� Extended Supported Rates IE */
    mac_set_exsup_rates_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

#else
    /* ���� Supported Rates IE */
    hmac_set_supported_rates_ie_asoc_req_etc(pst_hmac_sta, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* ���� Extended Supported Rates IE */
    hmac_set_exsup_rates_ie_asoc_req_etc(pst_hmac_sta, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;
#endif
    /* ���� Power Capability IE */
    mac_set_power_cap_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

    /* ���� Supported channel IE */
    mac_set_supported_channel_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34)
    if (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_wpa2)
    {
        /* ���� RSN Capability IE */
        mac_set_rsn_ie_etc((oal_void *)pst_mac_vap, OAL_PTR_NULL, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }
    else if (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_wpa)
    {
        /* ���� WPA Capability IE */
        mac_set_wpa_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }
#endif
    /* ���WMM element */
    if (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_wmm_cap)
    {
        mac_set_wmm_ie_sta_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

    /* ���� HT Capability IE  */
    mac_set_ht_capabilities_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    if ((OAL_TRUE == pst_mac_vap->bit_ap_11ntxbf)
    && (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_11ntxbf)
    && (0 != uc_ie_len))
    {
        puc_req_frame += MAC_11N_TXBF_CAP_OFFSET;

        pst_txbf_cap  = (mac_txbf_cap_stru *)puc_req_frame;
        pst_txbf_cap->bit_rx_stagg_sounding             = OAL_TRUE;
        pst_txbf_cap->bit_explicit_compr_bf_fdbk        = 1;
        pst_txbf_cap->bit_compr_steering_num_bf_antssup = 1;
        pst_txbf_cap->bit_minimal_grouping              = 3;
        pst_txbf_cap->bit_chan_estimation               = 1;
        puc_req_frame -= MAC_11N_TXBF_CAP_OFFSET;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
        if( (OAL_TRUE == g_en_mimo_blacklist_etc)
          &&(0 != uc_ie_len))
        {
            hmac_assoc_set_siso_mode(pst_mac_vap, puc_req_frame);
        }
#endif

    puc_req_frame += uc_ie_len;

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) || defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
    if(OAL_PTR_NULL != pst_scaned_bss)
    {
    #ifndef _PRE_WLAN_FEATURE_11KV_INTERFACE
        en_rrm_enable = pst_hmac_sta->bit_11k_enable;
    #endif
        en_rrm_enable = en_rrm_enable && pst_scaned_bss->st_bss_dscr_info.en_support_rrm;
        if((OAL_TRUE == pst_hmac_sta->bit_11k_auth_flag) || (OAL_TRUE == en_rrm_enable))
        {
            mac_set_rrm_enabled_cap_field_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
            puc_req_frame += uc_ie_len;
        }
    }
#endif //_PRE_WLAN_FEATURE_11K

    /* ���� Extended Capability IE */
    if (hmac_sta_check_need_set_ext_cap_ie(pst_mac_vap) == OAL_TRUE)
    {
        mac_set_ext_capabilities_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

    /* ���� VHT Capability IE */
    if((OAL_PTR_NULL != pst_scaned_bss) && (OAL_TRUE == pst_scaned_bss->st_bss_dscr_info.en_vht_capable)
       && (OAL_FALSE== pst_scaned_bss->st_bss_dscr_info.en_vendor_vht_capable))
    {
        mac_set_vht_capabilities_ie_etc((oal_void *) pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }
#ifdef _PRE_WLAN_FEATURE_11AX
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mac_set_he_capabilities_ie((oal_void *)pst_mac_vap,puc_req_frame,&uc_ie_len);
    puc_req_frame += uc_ie_len;
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    pst_hmac_user = mac_res_get_hmac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if((OAL_PTR_NULL != pst_hmac_user) && (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_opmode))
    {
        mac_set_opmode_notify_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

    OAM_WARNING_LOG2(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_req_sta_etc::bit_opmode[%d]ap_chip_oui[%d].}\r\n",
        pst_mac_vap->st_cap_flag.bit_opmode, pst_mac_vap->bit_ap_chip_oui);
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
    if((OAL_PTR_NULL != pst_scaned_bss) && (OAL_TRUE == pst_scaned_bss->st_bss_dscr_info.en_support_1024qam))
    {
        mac_set_1024qam_vendor_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_set_11ntxbf_vendor_ie_etc(pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;
#endif

    /* ��� BCM Vendor VHT IE,�����BCM AP��˽��Э���ͨ���� */
    if ((OAL_PTR_NULL != pst_scaned_bss) && (OAL_TRUE == pst_scaned_bss->st_bss_dscr_info.en_vendor_vht_capable))
    {
        mac_set_vendor_vht_ie(pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }
    if((OAL_PTR_NULL != pst_scaned_bss) && (OAL_TRUE == pst_scaned_bss->st_bss_dscr_info.en_vendor_novht_capable))
    {
        mac_set_vendor_novht_ie(pst_mac_vap, puc_req_frame, &uc_ie_len);
        puc_req_frame += uc_ie_len;
    }

    en_app_ie_type = OAL_APP_ASSOC_REQ_IE;
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (OAL_PTR_NULL != puc_curr_bssid)
    {
        en_app_ie_type = OAL_APP_REASSOC_REQ_IE;
#ifdef _PRE_WLAN_FEATURE_11R
        if(OAL_TRUE == pst_hmac_sta->bit_11r_enable)
        {
            if (OAL_TRUE == mac_mib_get_ft_trainsistion(pst_mac_vap))
            {
                en_app_ie_type = OAL_APP_FT_IE;
            }
        }
#endif //_PRE_WLAN_FEATURE_11R
    }
#endif //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_NARROW_BAND
        if ((OAL_PTR_NULL != pst_scaned_bss) && (OAL_TRUE == pst_scaned_bss->st_bss_dscr_info.en_nb_capable)
            && (OAL_TRUE == pst_mac_vap->st_nb.en_open)
                && (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_nb))
        {
            mac_set_nb_ie(puc_req_frame, &uc_ie_len);
            puc_req_frame += uc_ie_len;
        }

#endif

    /* ���P2P/WPS IE ��Ϣ */
    mac_add_app_ie_etc(pst_mac_vap, puc_req_frame, &us_app_ie_len, en_app_ie_type);
    puc_req_frame += us_app_ie_len;

#ifdef _PRE_WLAN_FEATURE_11R
    if (OAL_TRUE == pst_hmac_sta->bit_11r_enable)
    {
        /* Q�汾����ʱwpa_s�·�IE��Я��MD IE, P�汾��Я�����˴����ж��Ƿ��Ѿ�Я���������ظ����MD IE */
        if ((OAL_FALSE == pst_hmac_sta->bit_reassoc_flag &&
            OAL_PTR_NULL == mac_find_ie_etc(MAC_EID_MOBILITY_DOMAIN, pst_mac_vap->ast_app_ie[en_app_ie_type].puc_ie, pst_mac_vap->ast_app_ie[en_app_ie_type].ul_ie_len))
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
            || 1 == pst_hmac_sta->bit_voe_11r_auth) /*voe 11r ��֤ʵ���һ�������Я������mdie�����޷���������*/
#else
        )
#endif
        {
            mac_set_md_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
            puc_req_frame += uc_ie_len;
        }
        else
        {/* Reasoc�а���RIC-Req */
            for (en_aci = WLAN_WME_AC_BE; en_aci < WLAN_WME_AC_BUTT; en_aci++)
            {
                if (mac_mib_get_QAPEDCATableMandatory(&pst_hmac_sta->st_vap_base_info, en_aci))
                {
                    en_target_ac = en_aci;
                    uc_tid = WLAN_WME_AC_TO_TID(en_target_ac);
                    mac_set_rde_ie_etc((oal_void *)pst_mac_vap,puc_req_frame, &uc_ie_len);
                    puc_req_frame += uc_ie_len;

                    mac_set_tspec_ie_etc((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len, uc_tid);
                    puc_req_frame += uc_ie_len;
                }
            }
        }
    }
#endif //_PRE_WLAN_FEATURE_11R

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* ���˹�ЭͬIE */
    mac_add_cowork_ie_etc((oal_void *)pst_hmac_sta, puc_req_frame, &us_app_ie_len);
    puc_req_frame += us_app_ie_len;
#endif

    /* multi-sta����������4��ַie */
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    mac_set_vender_4addr_ie((oal_void *)pst_mac_vap, puc_req_frame, &uc_ie_len);
    puc_req_frame += uc_ie_len;
#endif

#ifdef _PRE_DEBUG_MODE
    OAM_WARNING_LOG0(0, OAM_SF_TX, "{hmac_mgmt_encap_asoc_req_sta_etc::encap assoc req!}\r\n");
#endif

    us_asoc_rsq_len = (oal_uint32)(puc_req_frame - puc_req_frame_origin);

    return us_asoc_rsq_len;
}


oal_uint16  hmac_mgmt_encap_auth_req_etc(hmac_vap_stru *pst_hmac_sta, oal_uint8 *puc_mgmt_frame)
{
    oal_uint16      us_auth_req_len;
    hmac_user_stru *pst_user_ap = OAL_PTR_NULL;
    oal_uint16      us_auth_type;
    oal_uint32      ul_ret;
    oal_uint16      us_user_index;
#ifdef _PRE_WLAN_FEATURE_11R
    oal_uint16      us_app_ie_len;
#endif //_PRE_WLAN_FEATURE_11R

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    mac_hdr_set_frame_control(puc_mgmt_frame, WLAN_FC0_SUBTYPE_AUTH);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address1, pst_hmac_sta->st_vap_base_info.auc_bssid);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address2, mac_mib_get_StationID(&pst_hmac_sta->st_vap_base_info));

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address3, pst_hmac_sta->st_vap_base_info.auc_bssid);
    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Authentication Frame (Sequence 1) - Frame Body           */
    /* --------------------------------------------------------------------  */
    /* |Auth Algorithm Number|Auth Transaction Sequence Number|Status Code|  */
    /* --------------------------------------------------------------------  */
    /* | 2                   |2                               |2          |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    if(OAL_FALSE == mac_mib_get_privacyinvoked(&pst_hmac_sta->st_vap_base_info))
    {
        /* Open System */
        puc_mgmt_frame[MAC_80211_FRAME_LEN]     = 0x00;
        puc_mgmt_frame[MAC_80211_FRAME_LEN + 1] = 0x00;
    }
    else
    {
        us_auth_type = (oal_uint16)mac_mib_get_AuthenticationMode(&pst_hmac_sta->st_vap_base_info);

        if(WLAN_WITP_AUTH_SHARED_KEY == us_auth_type)
        {
            OAM_INFO_LOG0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_encap_auth_req_etc::WLAN_WITP_AUTH_SHARED_KEY.}");
            us_auth_type = WLAN_WITP_AUTH_SHARED_KEY;
        }
        else
        {
            OAM_INFO_LOG0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_encap_auth_req_etc::WLAN_WITP_AUTH_OPEN_SYSTEM.}");
            us_auth_type = WLAN_WITP_AUTH_OPEN_SYSTEM;
        }

        puc_mgmt_frame[MAC_80211_FRAME_LEN]     = (us_auth_type & 0xFF);
        puc_mgmt_frame[MAC_80211_FRAME_LEN + 1] = ((us_auth_type & 0xFF00) >> 8);
    }

    /* ���� Authentication Transaction Sequence Number Ϊ 1 */
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 2] = 0x01;
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 3] = 0x00;

    /* ���� Status Code Ϊ0. �����������ֶ�û�� . */
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 4] = 0x00;
    puc_mgmt_frame[MAC_80211_FRAME_LEN + 5] = 0x00;

    /* ���� ��֤֡�ĳ��� */
    us_auth_req_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                   MAC_STATUS_CODE_LEN;

#ifdef _PRE_WLAN_FEATURE_11R
    if(OAL_TRUE == pst_hmac_sta->bit_11r_enable)
    {
        if ((OAL_TRUE == mac_mib_get_ft_trainsistion(&pst_hmac_sta->st_vap_base_info))
           && (MAC_VAP_STATE_ROAMING == pst_hmac_sta->st_vap_base_info.en_vap_state))
        {
            /* FT System */
            puc_mgmt_frame[MAC_80211_FRAME_LEN]     = 0x02;
            puc_mgmt_frame[MAC_80211_FRAME_LEN + 1] = 0x00;
            puc_mgmt_frame       += us_auth_req_len;

            mac_add_app_ie_etc((oal_void *)&pst_hmac_sta->st_vap_base_info, puc_mgmt_frame, &us_app_ie_len, OAL_APP_FT_IE);
            us_auth_req_len      += us_app_ie_len;
            puc_mgmt_frame       += us_app_ie_len;
        }
    }
#endif //_PRE_WLAN_FEATURE_11R

    pst_user_ap = (hmac_user_stru *)mac_res_get_hmac_user_etc(pst_hmac_sta->st_vap_base_info.us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_user_ap)
    {
        OAM_INFO_LOG0(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_mgmt_encap_auth_req_etc::no present ap, alloc new ap.}");
        ul_ret = hmac_user_add_etc(&pst_hmac_sta->st_vap_base_info, pst_hmac_sta->st_vap_base_info.auc_bssid, &us_user_index);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(pst_hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_mgmt_encap_auth_req_etc::hmac_user_add_etc failed[%d].}", ul_ret);
            us_auth_req_len = 0;
        }
    }

#ifdef _PRE_DEBUG_MODE
    OAM_WARNING_LOG0(0, OAM_SF_TX, "{hmac_mgmt_encap_auth_req_etc::encap auth req!}\r\n");
#endif

    return us_auth_req_len;
}


oal_uint16  hmac_mgmt_encap_auth_req_seq3_etc(hmac_vap_stru *pst_sta, oal_uint8 *puc_mgmt_frame, oal_uint8 *puc_mac_hrd)
{
    oal_uint8  *puc_data       = OAL_PTR_NULL;
    oal_uint16  us_index        = 0;
    oal_uint16  us_auth_req_len = 0;
    oal_uint8  *puc_ch_text     = OAL_PTR_NULL;
    oal_uint8   uc_ch_text_len  = 0;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    mac_hdr_set_frame_control(puc_mgmt_frame, WLAN_FC0_SUBTYPE_AUTH);

    /* ��֡�����ֶ���1 */
    mac_set_wep(puc_mgmt_frame, 1);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address1, pst_sta->st_vap_base_info.auc_bssid);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address2, mac_mib_get_StationID(&pst_sta->st_vap_base_info));

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_mgmt_frame)->auc_address3, pst_sta->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Authentication Frame (Sequence 3) - Frame Body           */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* ��ȡ��֤֡payload */
    us_index = MAC_80211_FRAME_LEN;
    puc_data = (oal_uint8 *)(puc_mgmt_frame + us_index);

    /* ���� ��֤֡�ĳ��� */
    us_auth_req_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                      MAC_STATUS_CODE_LEN;

    /* In case of no failure, the frame must be WEP encrypted. 4 bytes must  */
    /* be   left for the  IV  in  that  case. These   fields will  then  be  */
    /* reinitialized, using the correct index, with offset for IV field.     */
    puc_data[0] = WLAN_WITP_AUTH_SHARED_KEY;    /* Authentication Algorithm Number               */
    puc_data[1] = 0x00;

    puc_data[2] = 0x03;                    /* Authentication Transaction Sequence Number    */
    puc_data[3] = 0x00;

    /* If WEP subfield in the  incoming  authentication frame is 1,  respond */
    /* with  'challenge text failure' status,  since the STA does not expect */
    /* an encrypted frame in this state.                                     */
    if(1 == mac_is_protectedframe(puc_mac_hrd))
    {
        puc_data[4] = MAC_CHLNG_FAIL;
        puc_data[5] = 0x00;
    }

    /* If the STA does not support WEP, respond with 'unsupported algo'      */
    /* status, since WEP is necessary for Shared Key Authentication.         */
    else if(OAL_FALSE == mac_is_wep_enabled(&(pst_sta->st_vap_base_info)))
    {
        puc_data[4] = MAC_UNSUPT_ALG;
        puc_data[5] = 0x00;
    }

    /* If the default WEP key is NULL, respond with 'challenge text failure' */
    /* status, since a NULL key value cannot be used for WEP operations.     */
    else if(mac_get_wep_default_keysize(&pst_sta->st_vap_base_info) == 0)
    {
        puc_data[4] = MAC_CHLNG_FAIL;
        puc_data[5] = 0x00;
    }

    /* If there is a mapping in dot11WEPKeyMappings matching the address of  */
    /* the AP, and the corresponding key is NULL respond with 'challenge     */
    /* text failure' status. This is currently not being used.               */

    /* No error condition detected */
    else
    {

        /* Set Status Code to 'success' */
        puc_data[4] = MAC_SUCCESSFUL_STATUSCODE;
        puc_data[5] = 0x00;

        /* Extract 'Challenge Text' and its 'length' from the incoming       */
        /* authentication frame                                              */
        uc_ch_text_len = puc_mac_hrd[MAC_80211_FRAME_LEN + 7];
        puc_ch_text     = (oal_uint8 *)(&puc_mac_hrd[MAC_80211_FRAME_LEN + 8]);

        /* Challenge Text Element                  */
        /* --------------------------------------- */
        /* |Element ID | Length | Challenge Text | */
        /* --------------------------------------- */
        /* | 1         |1       |1 - 253         | */
        /* --------------------------------------- */

        puc_mgmt_frame[us_index + 6]   = MAC_EID_CHALLENGE;
        puc_mgmt_frame[us_index + 7]   = uc_ch_text_len;
        memcpy_s(&puc_mgmt_frame[us_index + 8], uc_ch_text_len, puc_ch_text, uc_ch_text_len);

        /* Add the challenge text element length to the authentication       */
        /* request frame length. The IV, ICV element lengths will be added   */
        /* after encryption.                                                 */
        us_auth_req_len += (uc_ch_text_len + MAC_IE_HDR_LEN);
    }

#ifdef _PRE_DEBUG_MODE
    OAM_WARNING_LOG0(0, OAM_SF_TX, "{hmac_mgmt_encap_auth_req_seq3_etc::encap auth req seq3!}\r\n");
#endif

    return us_auth_req_len;
}


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

