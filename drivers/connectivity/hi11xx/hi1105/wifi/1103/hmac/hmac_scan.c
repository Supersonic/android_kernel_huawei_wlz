

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 ͷ�ļ����� */
#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "dmac_ext_if.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_mgmt_sta.h"
#include "frw_ext_if.h"
#ifdef _PRE_SUPPORT_ACS
#include "dmac_acs.h"
#include "hmac_acs.h"
#endif
#include "hmac_chan_mgmt.h"
#include "hmac_p2p.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_ROAM
#include "hmac_roam_main.h"
#endif  // _PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
#include "hmac_11k.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SCAN_C

/* 2 ȫ�ֱ������� */
#if defined(_PRE_WLAN_CHIP_TEST_ALG) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_DEBUG_MODE)
struct kobject *g_scan_ct_sys_kobject = OAL_PTR_NULL;
#endif
hmac_scan_state_enum_uint8 g_en_bgscan_enable_flag_etc = HMAC_BGSCAN_ENABLE;
oal_uint32 g_pd_bss_expire_time_etc = 0;

/* 3 ����ʵ�� */
#if defined(_PRE_WLAN_CHIP_TEST_ALG) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_DEBUG_MODE)
OAL_STATIC oal_ssize_t hmac_scan_ct_proc_read(struct kobject *dev, struct kobj_attribute *attr, char *buf);
OAL_STATIC struct kobj_attribute dev_attr_alg_test_result =
    __ATTR(alg_test_result, OAL_S_IRUGO | OAL_S_IWUSR, hmac_scan_ct_proc_read, OAL_PTR_NULL);


OAL_STATIC oal_ssize_t hmac_scan_ct_proc_read(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    hmac_device_stru *pst_hmac_device;
    hmac_scan_record_stru *pst_record;
    oal_int32 l_len;

    /* ��ȡhmac device��ɨ�����м�¼ */
    pst_hmac_device = hmac_res_get_mac_dev_etc(0);
    pst_record = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    l_len = OAL_SIZEOF(wlan_scan_chan_stats_stru);

    if (EOK != memcpy_s(buf, l_len, &(pst_record->ast_chan_results[0]), l_len)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_scan_ct_proc_read::memcpy fail!");
    }

    return l_len;
}


oal_int32 hmac_scan_ct_init(oal_void)
{
    /* hi1102-cb add sys for 51/02 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_int32 l_ret = OAL_SUCC;

    if (g_scan_ct_sys_kobject == OAL_PTR_NULL) {
        g_scan_ct_sys_kobject = kobject_create_and_add("scan_ct", OAL_PTR_NULL);
        l_ret = sysfs_create_file(g_scan_ct_sys_kobject, &dev_attr_alg_test_result.attr);
    }

    return l_ret;
#else
    return OAL_SUCC;
#endif
}


oal_void hmac_scan_ct_exit(oal_void)
{
    if (g_scan_ct_sys_kobject != OAL_PTR_NULL) {
        sysfs_remove_file(g_scan_ct_sys_kobject, &dev_attr_alg_test_result.attr);
        kobject_del(g_scan_ct_sys_kobject);
        g_scan_ct_sys_kobject = OAL_PTR_NULL;
    }
}
#endif


oal_int32 hmac_snprintf_hex(oal_uint8 *puc_buf, oal_int32 l_buf_size, oal_uint8 *puc_data, oal_int32 l_len)
{
    oal_int32 l_loop;
    oal_uint8 *puc_pos = OAL_PTR_NULL;
    oal_uint8 *puc_end = OAL_PTR_NULL;
    oal_int32 l_ret;

    if (l_buf_size <= 0) {
        return 0;
    }

    puc_pos = puc_buf;
    puc_end = puc_buf + l_buf_size;
    for (l_loop = 0; l_loop < l_len; l_loop++) {
        l_ret = snprintf_s((oal_int8 *)puc_pos, (oal_uint16)(puc_end - puc_pos),
                           (oal_uint16)(puc_end - puc_pos) - 1, "%02x ", puc_data[l_loop]);
        if ((l_ret < 0) || (l_ret >= puc_end - puc_pos)) {
            puc_buf[l_buf_size - 1] = '\0';
            return puc_pos - puc_buf;
        }

        puc_pos += l_ret;
    }

    puc_buf[l_buf_size - 1] = '\0';
    return puc_pos - puc_buf;
}


OAL_STATIC oal_void hmac_scan_print_scan_params(mac_scan_req_stru *pst_scan_params, mac_vap_stru *pst_mac_vap)
{
    OAM_WARNING_LOG4(pst_scan_params->uc_vap_id, OAM_SF_SCAN,
        "hmac_scan_print_scan_params::Now Scan channel_num[%d] in [%d]ms with scan_func[0x%x], and ssid_num[%d]!",
        pst_scan_params->uc_channel_nums,
        pst_scan_params->us_scan_time,
        pst_scan_params->uc_scan_func,
        pst_scan_params->uc_ssid_num);

    OAM_WARNING_LOG3(pst_scan_params->uc_vap_id, OAM_SF_SCAN,
        "hmac_scan_print_scan_params::Scan param, p2p_scan:%d,max_scan_count_per_channel:%d,need back home_channel:%d!",
        pst_scan_params->bit_is_p2p0_scan,
        pst_scan_params->uc_max_scan_count_per_channel,
        pst_scan_params->en_need_switch_back_home_channel);
    return;
}

OAL_STATIC oal_void hmac_wifi_hide_ssid_etc(oal_uint8 *puc_frame_body, oal_uint16 us_mac_frame_len)
{
    oal_uint8 *puc_ssid_ie = OAL_PTR_NULL;
    oal_uint8 uc_ssid_len = 0;
    oal_uint8 uc_index = 0;

    if (puc_frame_body == OAL_PTR_NULL) {
        return;
    }

    puc_ssid_ie = mac_get_ssid_etc(puc_frame_body, us_mac_frame_len, &uc_ssid_len);

    /* ����4λ���������4λ�������� */
    if (puc_ssid_ie == OAL_PTR_NULL || uc_ssid_len < 4) {
        return;
    }

    for (uc_index = 2; uc_index < uc_ssid_len - 2; uc_index++)  { // ����ǰ��λ�ͺ���λ
        *(puc_ssid_ie + uc_index) = 0x78;  // 0x78ת��ΪASCII������ַ�'x'
    }
}


oal_void hmac_scan_print_scanned_bss_info_etc(oal_uint8 uc_device_id)
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = OAL_PTR_NULL;
    oal_uint8 auc_sdt_parse_hdr[MAC_80211_FRAME_LEN];
    oal_uint8 *puc_tmp_mac_body_addr = OAL_PTR_NULL;
    oal_uint8 uc_frame_sub_type;
    oal_int32 l_ret;

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{hmac_scan_print_scanned_bss_info_etc::pst_hmac_device null.}");
        return;
    }

    /* ��ȡָ��ɨ�����Ĺ����ṹ���ַ */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* ��ȡ�� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����ɨ�赽��bss��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

        /* ����ʾ�����뵽��BSS֡ */
        if (pst_scanned_bss->st_bss_dscr_info.en_new_scan_bss == OAL_TRUE) {
            pst_scanned_bss->st_bss_dscr_info.en_new_scan_bss = OAL_FALSE;
            /* �ϱ�beacon��probe֡ */
            pst_frame_hdr = (mac_ieee80211_frame_stru *)pst_bss_dscr->auc_mgmt_buff;

            /* ��beacon��duration�ֶ�(2�ֽ�)����Ϊrssi�Լ�channel,����SDT��ʾ */
            l_ret = memcpy_s((oal_uint8 *)auc_sdt_parse_hdr, MAC_80211_FRAME_LEN,
                             (oal_uint8 *)pst_frame_hdr, MAC_80211_FRAME_LEN);
            auc_sdt_parse_hdr[2] = (oal_uint8)pst_bss_dscr->c_rssi;
            auc_sdt_parse_hdr[3] = pst_bss_dscr->st_channel.uc_chan_number;

            puc_tmp_mac_body_addr = (oal_uint8 *)oal_memalloc(pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN);

            if (OAL_UNLIKELY(puc_tmp_mac_body_addr == OAL_PTR_NULL)) {
                OAM_WARNING_LOG0(0, OAM_SF_SCAN,
                    "{hmac_scan_print_scanned_bss_info::alloc memory failed for storing tmp mac_frame_body.}");
                continue;
            }

            l_ret += memcpy_s(puc_tmp_mac_body_addr, pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN,
                              (oal_uint8 *)(pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN),
                              pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN);
            if (l_ret != EOK) {
                OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_print_scanned_bss_info_etc::memcpy fail!");
                oal_free(puc_tmp_mac_body_addr);
                /* ����� */
                oal_spin_unlock(&(pst_bss_mgmt->st_lock));
                return;
            }

            uc_frame_sub_type = mac_get_frame_type_and_subtype((oal_uint8 *)pst_frame_hdr);
            if ((WLAN_FC0_SUBTYPE_BEACON == uc_frame_sub_type) ||
                (WLAN_FC0_SUBTYPE_PROBE_RSP == uc_frame_sub_type)) {
                hmac_wifi_hide_ssid_etc(puc_tmp_mac_body_addr, pst_bss_dscr->ul_mgmt_len - MAC_80211_FRAME_LEN);
            }

            /* �ϱ�beacon֡����probe rsp֡ */
            /*lint -e416*/
            oam_report_80211_frame_etc(BROADCAST_MACADDR,
                                       (oal_uint8 *)auc_sdt_parse_hdr,
                                       MAC_80211_FRAME_LEN,
                                       puc_tmp_mac_body_addr,
                                       (oal_uint16)pst_bss_dscr->ul_mgmt_len,
                                       OAM_OTA_FRAME_DIRECTION_TYPE_RX);
            oal_free(puc_tmp_mac_body_addr);
            /*lint +e416*/
#ifdef _PRE_WLAN_FEATURE_SNIFFER
            proc_sniffer_write_file(NULL, 0, pst_bss_dscr->auc_mgmt_buff, (oal_uint16)pst_bss_dscr->ul_mgmt_len, 0);
#endif
        }
    }

    /* ����� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return;
}


OAL_STATIC hmac_scanned_bss_info *hmac_scan_alloc_scanned_bss(oal_uint32 ul_mgmt_len)
{
    hmac_scanned_bss_info *pst_scanned_bss;

    /* �����ڴ棬�洢ɨ�赽��bss��Ϣ */
    pst_scanned_bss = oal_memalloc(OAL_SIZEOF(hmac_scanned_bss_info) + ul_mgmt_len -
                                   OAL_SIZEOF(pst_scanned_bss->st_bss_dscr_info.auc_mgmt_buff));
    if (OAL_UNLIKELY(pst_scanned_bss == OAL_PTR_NULL)) {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN,
                         "{hmac_scan_alloc_scanned_bss::alloc memory failed for storing scanned result.}");
        return OAL_PTR_NULL;
    }

    /* Ϊ������ڴ����� */
    memset_s(pst_scanned_bss,
             OAL_SIZEOF(hmac_scanned_bss_info) + ul_mgmt_len -
             OAL_SIZEOF(pst_scanned_bss->st_bss_dscr_info.auc_mgmt_buff), 0,
             OAL_SIZEOF(hmac_scanned_bss_info) + ul_mgmt_len -
             OAL_SIZEOF(pst_scanned_bss->st_bss_dscr_info.auc_mgmt_buff));

    /* ��ʼ������ͷ�ڵ�ָ�� */
    oal_dlist_init_head(&(pst_scanned_bss->st_dlist_head));

    return pst_scanned_bss;
}


OAL_STATIC oal_uint32 hmac_scan_add_bss_to_list(hmac_scanned_bss_info *pst_scanned_bss,
                                                hmac_device_stru *pst_hmac_device)
{
    hmac_bss_mgmt_stru *pst_bss_mgmt; /* ����ɨ�����Ľṹ�� */

    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    pst_scanned_bss->st_bss_dscr_info.en_new_scan_bss = OAL_TRUE;

    /* ������д����ǰ���� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����ɨ�����������У�������ɨ�赽��bss���� */
    oal_dlist_add_tail(&(pst_scanned_bss->st_dlist_head), &(pst_bss_mgmt->st_bss_list_head));

    pst_bss_mgmt->ul_bss_num++;
    /* ���� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_scan_del_bss_from_list_nolock(hmac_scanned_bss_info *pst_scanned_bss,
                                                         hmac_device_stru *pst_hmac_device)
{
    hmac_bss_mgmt_stru *pst_bss_mgmt; /* ����ɨ�����Ľṹ�� */

    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* ��������ɾ���ڵ㣬������ɨ�赽��bss���� */
    oal_dlist_delete_entry(&(pst_scanned_bss->st_dlist_head));

    pst_bss_mgmt->ul_bss_num--;

    return OAL_SUCC;
}


oal_void hmac_scan_clean_scan(hmac_scan_stru *pst_scan)
{
    hmac_scan_record_stru *pst_scan_record = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;

    /* �����Ϸ��Լ�� */
    if (pst_scan == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_clean_scan::pst_scan is null.}");
        return;
    }

    pst_scan_record = &pst_scan->st_scan_record_mgmt;

    /* 1.һ��Ҫ�����ɨ�赽��bss��Ϣ���ٽ������㴦�� */
    pst_bss_mgmt = &(pst_scan_record->st_bss_mgmt);

    /* ������д����ǰ���� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����������ɾ��ɨ�赽��bss��Ϣ */
    while (OAL_FALSE == oal_dlist_is_empty(&(pst_bss_mgmt->st_bss_list_head))) {
        pst_entry = oal_dlist_delete_head(&(pst_bss_mgmt->st_bss_list_head));
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);

        pst_bss_mgmt->ul_bss_num--;

        /* �ͷ�ɨ���������ڴ� */
        oal_free(pst_scanned_bss);
    }

    /* ������д����ǰ���� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    /* 2.������Ϣ���� */
    memset_s(pst_scan_record, OAL_SIZEOF(hmac_scan_record_stru), 0, OAL_SIZEOF(hmac_scan_record_stru));
    pst_scan_record->en_scan_rsp_status = MAC_SCAN_STATUS_BUTT; /* ��ʼ��ɨ�����ʱ״̬��Ϊ��Чֵ */
    pst_scan_record->en_vap_last_state = MAC_VAP_STATE_BUTT;    /* ������BUTT,����aputͣɨ���vap״̬�ָ��� */

    /* 3.���³�ʼ��bss��������������� */
    pst_bss_mgmt = &(pst_scan_record->st_bss_mgmt);
    oal_dlist_init_head(&(pst_bss_mgmt->st_bss_list_head));
    oal_spin_lock_init(&(pst_bss_mgmt->st_lock));

    /* 4.ɾ��ɨ�賬ʱ��ʱ�� */
    if (pst_scan->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_scan->st_scan_timeout));
    }

    OAM_INFO_LOG0(0, OAM_SF_SCAN, "{hmac_scan_clean_scan::cleaned scan record success.}");

    return;
}


OAL_STATIC oal_int32 hmac_is_connected_ap_bssid(oal_uint8 uc_device_id, oal_uint8 auc_bssid[WLAN_MAC_ADDR_LEN])
{
    oal_uint8 uc_vap_idx;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev_etc(uc_device_id);
    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_is_connected_ap_bssid::mac_res_get_dev_etc return null.}");
        return OAL_FALSE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(pst_mac_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(0, OAM_SF_P2P, "{hmac_is_connected_ap_bssid::mac_res_get_mac_vap fail! vap id is %d}",
                             pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (IS_LEGACY_VAP(pst_mac_vap) &&
            (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            if (0 == oal_memcmp(auc_bssid, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN)) {
                /* ���ϻ���ǰ������AP */
                OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                              "{hmac_is_connected_ap_bssid::connected AP bssid:%02X:XX:XX:XX:%02X:%02X}",
                              auc_bssid[0], auc_bssid[4], auc_bssid[5]);

                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}


OAL_STATIC oal_void hmac_scan_clean_expire_scanned_bss(hmac_vap_stru *pst_hmac_vap,
                                                       hmac_scan_record_stru *pst_scan_record)
{
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_entry_tmp = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    oal_uint32 ul_curr_time_stamp;

    /* �����Ϸ��Լ�� */
    if (pst_scan_record == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_clean_expire_scanned_bss::scan record is null.}");
        return;
    }

    /* ����ɨ���bss����Ľṹ�� */
    pst_bss_mgmt = &(pst_scan_record->st_bss_mgmt);

    ul_curr_time_stamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ������д����ǰ���� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����������ɾ����һ��ɨ�����е��ڵ�bss��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_entry_tmp, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

        if (oal_time_after32(ul_curr_time_stamp,
                             (pst_bss_dscr->ul_timestamp + HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE)) == FALSE) {
            OAM_INFO_LOG0(0, OAM_SF_SCAN,
                          "{hmac_scan_clean_expire_scanned_bss::do not remove the BSS, because it has not expired.}");
            continue;
        }
        /* �����ϻ�ʹ�� */
        if ((g_pd_bss_expire_time_etc != 0) &&
            (ul_curr_time_stamp - pst_bss_dscr->ul_timestamp < g_pd_bss_expire_time_etc * 1000)) {
            continue;
        }

        /* ���ϻ���ǰ���ڹ�����AP */
        if (hmac_is_connected_ap_bssid(pst_scan_record->uc_device_id, pst_bss_dscr->auc_bssid)) {
            pst_bss_dscr->c_rssi = pst_hmac_vap->station_info.signal;
            continue;
        }

        /* ��������ɾ���ڵ㣬������ɨ�赽��bss���� */
        oal_dlist_delete_entry(&(pst_scanned_bss->st_dlist_head));
        pst_bss_mgmt->ul_bss_num--;

        /* �ͷŶ�Ӧ�ڴ� */
        oal_free(pst_scanned_bss);
    }

    /* ������д����ǰ���� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));
    return;
}



mac_bss_dscr_stru *hmac_scan_find_scanned_bss_dscr_by_index_etc(oal_uint8 uc_device_id,
                                                                oal_uint32 ul_bss_index)
{
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;
    oal_uint8 ul_loop;

    /* ��ȡhmac device �ṹ */
    pst_hmac_device = hmac_res_get_mac_dev_etc(uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::pst_hmac_device is null.}");
        return OAL_PTR_NULL;
    }

    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* ������ɾ����ǰ���� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ������������ܹ�ɨ���bss�����������쳣 */
    if (ul_bss_index >= pst_bss_mgmt->ul_bss_num) {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::no such bss in bss list!}");

        /* ���� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));
        return OAL_PTR_NULL;
    }

    ul_loop = 0;
    /* �������������ض�Ӧindex��bss dscr��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);

        /* ��ͬ��bss index���� */
        if (ul_bss_index == ul_loop) {
            /* ���� */
            oal_spin_unlock(&(pst_bss_mgmt->st_lock));
            return &(pst_scanned_bss->st_bss_dscr_info);
        }

        ul_loop++;
    }
    /* ���� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return OAL_PTR_NULL;
}


hmac_scanned_bss_info *hmac_scan_find_scanned_bss_by_bssid_etc(hmac_bss_mgmt_stru *pst_bss_mgmt, oal_uint8 *puc_bssid)
{
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;

    /* ���������������������Ƿ��Ѿ�������ͬbssid��bss��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);

        /* ��ͬ��bssid��ַ */
        if (0 == oal_compare_mac_addr(pst_scanned_bss->st_bss_dscr_info.auc_bssid, puc_bssid)) {
            return pst_scanned_bss;
        }
    }

    return OAL_PTR_NULL;
}


oal_void *hmac_scan_get_scanned_bss_by_bssid(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mac_addr)
{
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL; /* ����ɨ���bss����Ľṹ�� */
    hmac_scanned_bss_info *pst_scanned_bss_info = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device;

    /* ��ȡhmac device �ṹ */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::pst_hmac_device is null, dev id[%d].}",
                         pst_mac_vap->uc_device_id);
        return OAL_PTR_NULL;
    }

    /* ��ȡ����ɨ���bss����Ľṹ�� */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    pst_scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid_etc(pst_bss_mgmt, puc_mac_addr);
    if (pst_scanned_bss_info == OAL_PTR_NULL) {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_get_scanned_bss_by_bssid::find the bss failed[%02X:XX:XX:%02X:%02X:%02X]}",
                         puc_mac_addr[0], puc_mac_addr[3], puc_mac_addr[4], puc_mac_addr[5]);

        /* ���� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));
        return OAL_PTR_NULL;
    }

    /* ���� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return &(pst_scanned_bss_info->st_bss_dscr_info);
}


OAL_STATIC OAL_INLINE oal_void hmac_scan_update_bss_list_wmm(mac_bss_dscr_stru *pst_bss_dscr,
                                                             oal_uint8 *puc_frame_body,
                                                             oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;

    pst_bss_dscr->uc_wmm_cap = OAL_FALSE;
    pst_bss_dscr->uc_uapsd_cap = OAL_FALSE;

    puc_ie = mac_get_wmm_ie_etc(puc_frame_body, us_frame_len);
    if (puc_ie != OAL_PTR_NULL) {
        pst_bss_dscr->uc_wmm_cap = OAL_TRUE;

        /* --------------------------------------------------------------------------------- */
        /* WMM Information/Parameter Element Format                                          */
        /* ---------------------------------------------------------------------------------- */
        /* EID | IE LEN | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
        /* --------------------------------------------------------------------------------- */
        /* 1   |   1    |  3  | 1       | 1          | 1       | 1       | ---------------- | */
        /* --------------------------------------------------------------------------------- */
        /* puc_ie[1] IE len ������EID��LEN�ֶ�,��ȡQoSInfo��uc_ie_len�������7�ֽڳ��� */
        /* Check if Bit 7 is set indicating U-APSD capability */
        if ((puc_ie[1] >= 7) && (puc_ie[8] & BIT7)) { /* wmm ie�ĵ�8���ֽ���QoS info�ֽ� */
            pst_bss_dscr->uc_uapsd_cap = OAL_TRUE;
        }
    } else {
        puc_ie = mac_find_ie_etc(MAC_EID_HT_CAP, puc_frame_body, us_frame_len);
        if (puc_ie != OAL_PTR_NULL) {
            
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            /* -------------------------------------------------------------- */
            /* HT capability Information/Parameter Element Format            */
            /* -------------------------------------------------------------- */
            /* EID | IE LEN |  HT capability Info |                 based   | */
            /* -------------------------------------------------------------- */
            /* 1   |   1    |         2           | ------------------------| */
            /* -------------------------------------------------------------- */
            /* puc_ie[1] IE len ������EID��LEN�ֶ�,��ȡHT cap Info��uc_ie_len�������2�ֽڳ��� */
            /* ht cap�ĵ� 2,3���ֽ���HT capability Info��Ϣ */
            /* Check if Bit 5 is set indicating short GI for 20M capability */
            if ((puc_ie[1] >= 2) && (puc_ie[2] & BIT5))
#endif
            {
                pst_bss_dscr->uc_wmm_cap = OAL_TRUE;
            }
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11D

OAL_STATIC oal_void hmac_scan_update_bss_list_country(mac_bss_dscr_stru *pst_bss_dscr,
                                                      oal_uint8 *puc_frame_body,
                                                      oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie;

    puc_ie = mac_find_ie_etc(MAC_EID_COUNTRY, puc_frame_body, us_frame_len);

    /* �����벻����, ȫ�����Ϊ0 */
    if (puc_ie == OAL_PTR_NULL) {
        pst_bss_dscr->ac_country[0] = 0;
        pst_bss_dscr->ac_country[1] = 0;
        pst_bss_dscr->ac_country[2] = 0;

        return;
    } else {
        pst_bss_dscr->puc_country_ie = puc_ie;
    }
    /* ���������2���ֽ�,IE LEN������ڵ���2 */
    if (puc_ie[1] >= 2) {
        pst_bss_dscr->ac_country[0] = (oal_int8)puc_ie[MAC_IE_HDR_LEN];
        pst_bss_dscr->ac_country[1] = (oal_int8)puc_ie[MAC_IE_HDR_LEN + 1];
        pst_bss_dscr->ac_country[2] = 0;
    }
}
#endif

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) ||  \
    defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)

OAL_STATIC oal_void hmac_scan_update_bss_list_rrm(mac_bss_dscr_stru *pst_bss_dscr,
                                                  oal_uint8 *puc_frame_body,
                                                  oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie;
    puc_ie = mac_find_ie_etc(MAC_EID_RRM, puc_frame_body, us_frame_len);
    if (puc_ie == OAL_PTR_NULL) {
        pst_bss_dscr->en_support_rrm = OAL_FALSE;
    } else {
        pst_bss_dscr->en_support_rrm = OAL_TRUE;
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_MBO

OAL_STATIC oal_void hmac_scan_update_bss_assoc_disallowed_attr(mac_bss_dscr_stru *pst_bss_dscr,
                                                               oal_uint8 *puc_frame_body,
                                                               oal_uint16 us_frame_len)
{
    oal_uint8 *puc_mbo_ie = OAL_PTR_NULL;
    oal_uint8 *puc_mbo_attribute = OAL_PTR_NULL;

    puc_mbo_ie = mac_find_vendor_ie_etc(MAC_MBO_VENDOR_IE, MAC_MBO_IE_OUI_TYPE, puc_frame_body, us_frame_len);
    if (puc_mbo_ie == OAL_PTR_NULL) {
        pst_bss_dscr->uc_bss_assoc_disallowed = OAL_FALSE;
        return;
    }

    puc_mbo_attribute = mac_find_mbo_attribute_etc(MBO_ATTR_ID_ASSOC_DISALLOW,
                                                   puc_mbo_ie + MBO_IE_HEADER,
                                                   MAC_MBO_ASSOC_DIS_ATTR_LEN);

    if (puc_mbo_attribute == OAL_PTR_NULL) {
        pst_bss_dscr->uc_bss_assoc_disallowed = OAL_FALSE; /* ������Association Disallowed Attribute���������� */
    } else {
        OAM_WARNING_LOG0(0, OAM_SF_ASSOC, "{hmac_scan_update_bss_assoc_disallowed_attr::Assoc Disallowed Attr found!}");
        pst_bss_dscr->uc_bss_assoc_disallowed = OAL_TRUE; /* ����Association Disallowed Attribute,AP ������������ */
    }

    return;
}
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM

OAL_STATIC oal_void hmac_scan_update_bss_list_1024qam(mac_bss_dscr_stru *pst_bss_dscr,
                                                      oal_uint8 *puc_frame_body,
                                                      oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie;
    puc_ie = mac_find_vendor_ie_etc(MAC_HUAWEI_VENDER_IE, MAC_HISI_1024QAM_IE, puc_frame_body, us_frame_len);
    if (puc_ie == OAL_PTR_NULL) {
        pst_bss_dscr->en_support_1024qam = OAL_FALSE;
    } else {
        pst_bss_dscr->en_support_1024qam = OAL_TRUE;
    }
}
#endif

#ifdef _PRE_WLAN_NARROW_BAND

OAL_STATIC oal_void hmac_scan_update_bss_list_nb(mac_bss_dscr_stru *pst_bss_dscr,
                                                 oal_uint8 *puc_frame_body,
                                                 oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie;

    puc_ie = mac_find_vendor_ie_etc(MAC_HUAWEI_VENDER_IE, MAC_HISI_NB_IE, puc_frame_body, us_frame_len);

    /* �ж��Ƿ�Я����IE */
    if (puc_ie == OAL_PTR_NULL) {
        pst_bss_dscr->en_nb_capable = OAL_FALSE;
    } else {
        pst_bss_dscr->en_nb_capable = OAL_TRUE;
    }
}
#endif


OAL_STATIC oal_void hmac_scan_update_11i(mac_bss_dscr_stru *pst_bss_dscr,
                                         oal_uint8 *puc_frame_body,
                                         oal_uint16 us_frame_len)
{
    pst_bss_dscr->puc_rsn_ie = mac_find_ie_etc(MAC_EID_RSN, puc_frame_body, (oal_int32)(us_frame_len));
    pst_bss_dscr->puc_wpa_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT,
                                                      MAC_OUITYPE_WPA,
                                                      puc_frame_body,
                                                      (oal_int32)(us_frame_len));
}


OAL_STATIC oal_void hmac_scan_update_bss_list_11n(mac_bss_dscr_stru *pst_bss_dscr,
                                                  oal_uint8 *puc_frame_body,
                                                  oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie;
    mac_ht_opern_stru *pst_ht_op = OAL_PTR_NULL;
    oal_uint8 uc_sec_chan_offset;
    wlan_bw_cap_enum_uint8 en_ht_cap_bw = WLAN_BW_CAP_20M;
    wlan_bw_cap_enum_uint8 en_ht_op_bw = WLAN_BW_CAP_20M;

    /* 11n */
    puc_ie = mac_find_ie_etc(MAC_EID_HT_CAP, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= 2)) {
        /* puc_ie[2]��HT Capabilities Info�ĵ�1���ֽ� */
        pst_bss_dscr->en_ht_capable = OAL_TRUE;        /* ֧��ht */
        pst_bss_dscr->en_ht_ldpc = (puc_ie[2] & BIT0); /* ֧��ldpc */
        en_ht_cap_bw = ((puc_ie[2] & BIT1) >> 1);      /* ȡ��֧�ֵĴ��� */
        pst_bss_dscr->en_ht_stbc = ((puc_ie[2] & BIT7) >> 7);
    }

    /* Ĭ��20M,���֡����δЯ��HT_OPERATION�����ֱ�Ӳ���Ĭ��ֵ */
    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_20M;

    puc_ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= 2)) {  // ����ie�����쳣���
        pst_ht_op = (mac_ht_opern_stru *)(puc_ie + MAC_IE_HDR_LEN);

        /* ��ȡ���ŵ�ƫ�� */
        uc_sec_chan_offset = pst_ht_op->bit_secondary_chan_offset;

        /* ��ֹap��channel width=0, ��channel offset = 1����3 ��ʱ��channel widthΪ�� */
        /* ht cap 20/40 enabled && ht operation 40 enabled */
        if ((pst_ht_op->bit_sta_chan_width != 0) && (en_ht_cap_bw > WLAN_BW_CAP_20M)) {  // cap > 20M��ȡchannel bw
            if (uc_sec_chan_offset == MAC_SCB) {
                pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                en_ht_op_bw = WLAN_BW_CAP_40M;
            } else if (uc_sec_chan_offset == MAC_SCA) {
                pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                en_ht_op_bw = WLAN_BW_CAP_40M;
            }
        }
    }

    /* ��AP��������ȡ������������Сֵ����ֹAP�쳣���ͳ��������������ݣ�������ݲ�ͨ */
    pst_bss_dscr->en_bw_cap = OAL_MIN(en_ht_cap_bw, en_ht_op_bw);

    puc_ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= 1)) {
        /* Extract 20/40 BSS Coexistence Management Support */
        pst_bss_dscr->uc_coex_mgmt_supp = (puc_ie[2] & BIT0);
#ifdef _PRE_WLAN_FEATURE_11AX
        /* �鿴 BSS Multi BSS֧����� */
        if (puc_ie[1] > 2) {
            pst_bss_dscr->st_mbssid_info.bit_ext_cap_multi_bssid_activated = ((puc_ie[4] & BIT6) >> 6);
        }
#endif
    }

#ifdef _PRE_WLAN_FEATURE_TXBF
    puc_ie = mac_find_vendor_ie_etc(MAC_HUAWEI_VENDER_IE, MAC_EID_11NTXBF, puc_frame_body, (oal_int32)us_frame_len);
    if (puc_ie != OAL_PTR_NULL) {
        pst_bss_dscr->en_11ntxbf =
            (((mac_11ntxbf_vendor_ie_stru *)puc_ie)->st_11ntxbf.bit_11ntxbf == 1) ? OAL_TRUE : OAL_FALSE;
    }
#endif
}

OAL_STATIC oal_void hmac_scan_update_bss_list_11ac_vht_cap(mac_bss_dscr_stru *pst_bss_dscr,
                                                   oal_uint8 *puc_frame_body,
                                                   oal_uint16 us_frame_len,
                                                   oal_uint en_is_vendor_ie)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_vht_capable = OAL_FALSE;
    oal_uint8 uc_supp_ch_width;

    puc_ie = mac_find_ie_etc(MAC_EID_VHT_CAP, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_VHT_CAP_IE_LEN)) {
        en_vht_capable = ((puc_ie[2] != 0) || (puc_ie[3] != 0) || (puc_ie[4] != 0) || (puc_ie[5] != 0) ||
                          (puc_ie[6] != 0xff) || (puc_ie[7] != 0xff));
        if (en_vht_capable == OAL_TRUE) {
            pst_bss_dscr->en_vht_capable = OAL_TRUE; /* ֧��vht */
        }

        /* ˵��vendor��Я��VHT ie�������ñ�־λ��assoc req��Ҳ��Я��vendor+vht ie */
        if (en_is_vendor_ie == OAL_TRUE) {
            pst_bss_dscr->en_vendor_vht_capable = OAL_TRUE;
        }

        /* ��ȡSupported Channel Width Set */
        uc_supp_ch_width = ((puc_ie[2] & (BIT3 | BIT2)) >> 2);

        if (uc_supp_ch_width == 0) {
            pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_80M; /* 80MHz */
        } else if (uc_supp_ch_width == 1) {
            pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_160M; /* 160MHz */
        } else if (uc_supp_ch_width == 2) {
            pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_80M; /* 80MHz */
        }
    } else {
        /* ˽��vendor�в�����vht ie������BCM 5g 20M ˽��Э�� */
        if (en_is_vendor_ie == OAL_TRUE) {
            pst_bss_dscr->en_vendor_novht_capable = OAL_TRUE;
        }
    }

}


OAL_STATIC oal_void hmac_scan_update_bss_list_11ac(mac_bss_dscr_stru *pst_bss_dscr,
                                                   oal_uint8 *puc_frame_body,
                                                   oal_uint16 us_frame_len,
                                                   oal_uint en_is_vendor_ie)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint8 uc_vht_chan_width;
    oal_uint8 uc_chan_center_freq;
    oal_uint8 uc_chan_center_freq_1;

    hmac_scan_update_bss_list_11ac_vht_cap(pst_bss_dscr, puc_frame_body, us_frame_len, en_is_vendor_ie);

    puc_ie = mac_find_ie_etc(MAC_EID_VHT_OPERN, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_VHT_OPERN_LEN)) {
        uc_vht_chan_width = puc_ie[2];
        uc_chan_center_freq = puc_ie[3];
        uc_chan_center_freq_1 = puc_ie[4];

        /* ���´�����Ϣ */
        if (uc_vht_chan_width == 0) { /* 40MHz */
            /* do nothing��en_channel_bandwidth�Ѿ���HT Operation IE�л�ȡ */
        } else if (uc_vht_chan_width == 1) { /* 80MHz */
#ifdef _PRE_WLAN_FEATURE_160M
            if ((uc_chan_center_freq_1 - uc_chan_center_freq == 8) ||
                (uc_chan_center_freq - uc_chan_center_freq_1 == 8)) {
                switch (uc_chan_center_freq_1 - pst_bss_dscr->st_channel.uc_chan_number) {
                    case 14:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSPLUS;
                        break;

                    case 10:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSPLUS;
                        break;

                    case 6:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSPLUS;
                        break;

                    case 2:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSPLUS;
                        break;

                    case -2:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
                        break;

                    case -6:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSMINUS;
                        break;

                    case -10:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSMINUS;
                        break;

                    case -14:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSMINUS;
                        break;

                    default:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_20M;
                        break;
                }
                return;
            }
#endif
            switch (uc_chan_center_freq - pst_bss_dscr->st_channel.uc_chan_number) {
                case 6:
                    /***********************************************************************
                    | ��20 | ��20 | ��40       |
                              |
                              |����Ƶ���������20ƫ6���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
                    break;

                case -2:
                    /***********************************************************************
                    | ��40        | ��20 | ��20 |
                              |
                              |����Ƶ���������20ƫ-2���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
                    break;

                case 2:
                    /***********************************************************************
                    | ��20 | ��20 | ��40       |
                              |
                              |����Ƶ���������20ƫ2���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
                    break;

                case -6:
                    /***********************************************************************
                    | ��40        | ��20 | ��20 |
                              |
                              |����Ƶ���������20ƫ-6���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
                    break;

                default:
                    break;
            }
        }
#ifdef _PRE_WLAN_FEATURE_160M
        else if (uc_vht_chan_width == 2) { /* 160MHz */
            switch (uc_chan_center_freq - pst_bss_dscr->st_channel.uc_chan_number) {
                case 14:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSPLUS;
                    break;
                case 10:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSPLUS;
                    break;

                case 6:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSPLUS;
                    break;
                case 2:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSPLUS;
                    break;
                case -2:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
                    break;
                case -6:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSMINUS;
                    break;
                case -10:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSMINUS;
                    break;
                case -14:
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSMINUS;
                    break;
                default:
                    break;
            }
        }
#endif
        else if (uc_vht_chan_width == 3) { /* 80+80MHz */
            switch (uc_chan_center_freq - pst_bss_dscr->st_channel.uc_chan_number) {
                case 6:
                    /***********************************************************************
                    | ��20 | ��20 | ��40       |
                              |
                              |����Ƶ���������20ƫ6���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
                    break;

                case -2:
                    /***********************************************************************
                    | ��40        | ��20 | ��20 |
                              |
                              |����Ƶ���������20ƫ-2���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
                    break;

                case 2:
                    /***********************************************************************
                    | ��20 | ��20 | ��40       |
                              |
                              |����Ƶ���������20ƫ2���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
                    break;

                case -6:
                    /***********************************************************************
                    | ��40        | ��20 | ��20 |
                              |
                              |����Ƶ���������20ƫ-6���ŵ�
                    ************************************************************************/
                    pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
                    break;

                default:
                    break;
            }
        } else {
            /* Unsupported Channel BandWidth */
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC oal_void hmac_scan_update_bss_list_11ax(mac_bss_dscr_stru *pst_bss_dscr,
                                                   oal_uint8 *puc_frame_body,
                                                   oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie;
    mac_frame_he_cap_ie_stru st_he_cap_value;
    mac_frame_he_oper_ie_stru st_he_oper_ie_value;
    oal_uint32 ul_ret;

    /* HE CAP */
    puc_ie = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_HE_CAP_MIN_LEN)) {
        memset_s(&st_he_cap_value, OAL_SIZEOF(st_he_cap_value), 0, OAL_SIZEOF(st_he_cap_value));
        /* �����̶����Ȳ���:MAC_Cap+PHY Cap + HE-MCS NSS(<=80MHz)  */
        ul_ret = mac_ie_parse_he_cap(puc_ie, &st_he_cap_value);
        if (ul_ret != OAL_SUCC) {
            return;
        }

        pst_bss_dscr->en_he_capable = OAL_TRUE; /* ֧��HE */

        if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_2G) {              /* 2G */
            if (st_he_cap_value.st_he_phy_cap.bit_channel_width_set & 0x1) { /* Bit0 2G �Ƿ�֧��40MHz */
                pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_40M;
            }
        } else {
            if (st_he_cap_value.st_he_phy_cap.bit_channel_width_set & 0x8) { /* B3-5G 80+80MHz */
                pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_80PLUS80;
            } else if (st_he_cap_value.st_he_phy_cap.bit_channel_width_set & 0x4) { /* B2-160MHz */
                pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_160M;
            } else if (st_he_cap_value.st_he_phy_cap.bit_channel_width_set & 0x2) { /* B2-5G֧��80MHz */
                pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_80M;
            } else {
                pst_bss_dscr->en_bw_cap = WLAN_BW_CAP_20M;
            }
        }
        /* �����û�֧�ַ��͵�txbf������ */
        pst_bss_dscr->uc_num_sounding_dim = (pst_bss_dscr->en_bw_cap <= WLAN_BW_CAP_80M) ?
            st_he_cap_value.st_he_phy_cap.bit_below_80mhz_sounding_dimensions_num :
            st_he_cap_value.st_he_phy_cap.bit_over_80mhz_sounding_dimensions_num;
    }

    /* HE Oper */
    puc_ie = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_OPERATION, puc_frame_body, us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_HE_OPERAION_MIN_LEN)) {
        memset_s(&st_he_oper_ie_value, OAL_SIZEOF(st_he_oper_ie_value), 0, OAL_SIZEOF(st_he_oper_ie_value));
        ul_ret = mac_ie_parse_he_oper(puc_ie, &st_he_oper_ie_value);
        if (ul_ret != OAL_SUCC) {
            return;
        }

        if (st_he_oper_ie_value.st_he_oper_param.bit_vht_operation_info_present == 1) {
            if (st_he_oper_ie_value.st_vht_operation_info.uc_channel_width == 1) { /* 80MHz */
                switch (st_he_oper_ie_value.st_vht_operation_info.uc_center_freq_seg0 -
                        pst_bss_dscr->st_channel.uc_chan_number) {
                    case 6:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
                        break;
                    case -2:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
                        break;
                    case 2:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
                        break;
                    case -6:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
                        break;
                    default:
                        break;
                }
            }
#ifdef _PRE_WLAN_FEATURE_160M
            else if (st_he_oper_ie_value.st_vht_operation_info.uc_channel_width == 2) { /* 160MHz */
                switch (st_he_oper_ie_value.st_vht_operation_info.uc_center_freq_seg0 -
                        pst_bss_dscr->st_channel.uc_chan_number) {
                    case 14:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSPLUS;
                        break;
                    case -2:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
                        break;
                    case 6:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSPLUS;
                        break;
                    case -10:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSMINUS;
                        break;
                    case 10:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSPLUS;
                        break;
                    case -6:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSMINUS;
                        break;
                    case 2:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSPLUS;
                        break;
                    case -14:
                        pst_bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSMINUS;
                        break;
                    default:
                        break;
                }
            }
#endif
        }
    }
}

#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX

oal_void hmac_scan_btcoex_backlist_check_by_oui_etc(mac_bss_dscr_stru *pst_bss_dscr,
                                                    oal_uint8 *puc_frame_body,
                                                    oal_uint16 us_frame_len)
{
    /* ��ʼ��Ϊ�Ǻ����� */
    pst_bss_dscr->en_btcoex_blacklist_chip_oui = 0;

    if (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_RALINK,
                                               MAC_WLAN_CHIP_OUI_TYPE_RALINK,
                                               puc_frame_body, us_frame_len) ||
        OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_RALINK,
                                               MAC_WLAN_CHIP_OUI_TYPE_RALINK1,
                                               puc_frame_body, us_frame_len) ||
        OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_SHENZHEN,
                                               MAC_WLAN_CHIP_OUI_TYPE_SHENZHEN,
                                               puc_frame_body, us_frame_len) ||
        OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_BROADCOM,
                                               MAC_WLAN_CHIP_OUI_TYPE_BROADCOM,
                                               puc_frame_body, us_frame_len)) {
        pst_bss_dscr->en_btcoex_blacklist_chip_oui |= MAC_BTCOEX_BLACKLIST_LEV0;
    }

    if (((OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_APPLE1,
                                                 MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_1,
                                                 puc_frame_body, us_frame_len)) ||
         (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_APPLE1,
                                                 MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_2,
                                                 puc_frame_body, us_frame_len)) ||
         (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_APPLE2,
                                                 MAC_WLAN_CHIP_OUI_TYPE_APPLE_2_1,
                                                 puc_frame_body, us_frame_len))) &&
        (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_BROADCOM,
                                                MAC_WLAN_CHIP_OUI_TYPE_BROADCOM,
                                                puc_frame_body, us_frame_len))) {
        pst_bss_dscr->en_btcoex_blacklist_chip_oui |= MAC_BTCOEX_BLACKLIST_LEV1;
    }
}
#endif


OAL_STATIC OAL_INLINE oal_void hmac_scan_update_bss_list_protocol(hmac_vap_stru *pst_hmac_vap,
                                                                  mac_bss_dscr_stru *pst_bss_dscr,
                                                                  oal_uint8 *puc_frame_body,
                                                                  oal_uint16 us_frame_len)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint16 us_offset_vendor_vht = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
#ifdef _PRE_WLAN_FEATURE_MBO
    mac_vap_rom_stru *pst_mac_vap_rom;

    pst_mac_vap_rom = (mac_vap_rom_stru *)((&pst_hmac_vap->st_vap_base_info)->_rom);
#endif

    puc_frame_body += MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    us_frame_len -= MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       | */
    /* ---------------------------------------------------------------------- */
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates| */
    /* ---------------------------------------------------------------------- */
    /* |3               |8    |4         |3   |4-255|26  | 3-257            | */
    /* ---------------------------------------------------------------------- */
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       | */
    /* ---------------------------------------------------------------------- */
    /* |7        |28              |24           |16                         | */
    /* ---------------------------------------------------------------------- */
    /* |Extended Capabilities |                                              */
    /* ---------------------------------------------------------------------- */
    /* |3-8                   |                                              */
    /*************************************************************************/
    /* wmm */
    hmac_scan_update_bss_list_wmm(pst_bss_dscr, puc_frame_body, us_frame_len);

    /* 11i */
    hmac_scan_update_11i(pst_bss_dscr, puc_frame_body, us_frame_len);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 11d */
    hmac_scan_update_bss_list_country(pst_bss_dscr, puc_frame_body, us_frame_len);
#endif

    /* 11n */
    hmac_scan_update_bss_list_11n(pst_bss_dscr, puc_frame_body, us_frame_len);

    /* rrm */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) ||  \
    defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
    hmac_scan_update_bss_list_rrm(pst_bss_dscr, puc_frame_body, us_frame_len);
#endif

    /* 11ac */
    hmac_scan_update_bss_list_11ac(pst_bss_dscr, puc_frame_body, us_frame_len, OAL_FALSE);

    /* 11ax */
#ifdef _PRE_WLAN_FEATURE_11AX
    if (IS_CUSTOM_OPEN_11AX_SWITCH(&pst_hmac_vap->st_vap_base_info)) {
        hmac_scan_update_bss_list_11ax(pst_bss_dscr, puc_frame_body, us_frame_len);
    }
#endif
    /* ����˽��vendor ie */
    puc_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_BROADCOM_EPIGRAM,
                                    MAC_WLAN_OUI_VENDOR_VHT_TYPE,
                                    puc_frame_body,
                                    us_frame_len);
    if ((puc_ie != OAL_PTR_NULL) && (puc_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        hmac_scan_update_bss_list_11ac(pst_bss_dscr, puc_ie + us_offset_vendor_vht,
                                       puc_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER, OAL_TRUE);
    }

    /* nb */
#ifdef _PRE_WLAN_NARROW_BAND
    hmac_scan_update_bss_list_nb(pst_bss_dscr, puc_frame_body, us_frame_len);
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_scan_update_bss_list_1024qam(pst_bss_dscr, puc_frame_body, us_frame_len);
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
    /* ���AP�Ƿ������assoc disallowed attr��MBO IE��MBO���ƻ����ش򿪲Ž��в��Ҷ�ӦMBO IE,������� */
    if (pst_mac_vap_rom->st_mbo_para_info.uc_mbo_enable == OAL_TRUE) {
        hmac_scan_update_bss_assoc_disallowed_attr(pst_bss_dscr, puc_frame_body, us_frame_len);
    }
#endif

    /* nb */
#ifdef _PRE_WLAN_NARROW_BAND
    hmac_scan_update_bss_list_nb(pst_bss_dscr, puc_frame_body, us_frame_len);
#endif

    if (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_ATHEROSC,
                                               MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC,
                                               puc_frame_body, us_frame_len)) {
        pst_bss_dscr->en_atheros_chip_oui = OAL_TRUE;
    } else {
        pst_bss_dscr->en_atheros_chip_oui = OAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hmac_scan_btcoex_backlist_check_by_oui_etc(pst_bss_dscr, puc_frame_body, us_frame_len);
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_BROADCOM,
                                               MAC_WLAN_CHIP_OUI_TYPE_BROADCOM,
                                               puc_frame_body, us_frame_len)) {
        pst_bss_dscr->en_roam_blacklist_chip_oui = OAL_TRUE;
    } else {
        pst_bss_dscr->en_roam_blacklist_chip_oui = OAL_FALSE;
    }
#endif

    if (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_MARVELL,
                                               MAC_WLAN_CHIP_OUI_TYPE_MARVELL,
                                               puc_frame_body, us_frame_len)) {
        pst_bss_dscr->en_txbf_blacklist_chip_oui = OAL_TRUE;
    } else {
        pst_bss_dscr->en_txbf_blacklist_chip_oui = OAL_FALSE;
    }

    if (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_RALINK,
                                               MAC_WLAN_CHIP_OUI_TYPE_RALINK,
                                               puc_frame_body, us_frame_len)) {
        pst_bss_dscr->en_is_tplink_oui = WLAN_AP_CHIP_OUI_RALINK;
    } else if (OAL_PTR_NULL != mac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_BROADCOM,
                                                      MAC_WLAN_CHIP_OUI_TYPE_BROADCOM,
                                                      puc_frame_body, us_frame_len)) {
        pst_bss_dscr->en_is_tplink_oui = WLAN_AP_CHIP_OUI_BCM;
    }
}

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151)

oal_uint8 hmac_scan_check_bss_supp_rates_etc(mac_device_stru *pst_mac_dev,
                                             oal_uint8 *puc_rate,
                                             oal_uint8 uc_bss_rate_num,
                                             oal_uint8 *puc_update_rate,
                                             oal_uint8 uc_rate_size)
{
    mac_data_rate_stru *pst_rates;
    oal_uint32 i, j, k;
    oal_uint8 uc_rate_num = 0;

    if (uc_rate_size > WLAN_USER_MAX_SUPP_RATES) {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::uc_rate_size err[%d].}", uc_rate_size);
        return uc_rate_num;
    }

    pst_rates = mac_device_get_all_rates_etc(pst_mac_dev);

    for (i = 0; i < uc_bss_rate_num; i++) {
        for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
            if ((IS_EQUAL_RATES(pst_rates[j].uc_mac_rate, puc_rate[i]))
                && (uc_rate_num < MAC_DATARATES_PHY_80211G_NUM)) {
                /* ȥ���ظ����� */
                for (k = 0; k < uc_rate_num; k++) {
                    if (IS_EQUAL_RATES(puc_update_rate[k], puc_rate[i])) {
                        break;
                    }
                }
                /* ���������ظ�����ʱ��k����uc_rate_num */
                if (k == uc_rate_num) {
                    puc_update_rate[uc_rate_num++] = puc_rate[i];
                }
                break;
            }
        }
    }

    return uc_rate_num;
}
#endif


void hmac_scan_rm_repeat_sup_exsup_rates(mac_bss_dscr_stru *pst_bss_dscr,
                                         oal_uint8 *puc_rates,
                                         oal_uint8 uc_exrate_num)
{
    int i, j;
    for (i = 0; i < uc_exrate_num; i++) {
        /* ȥ���ظ����� */
        for (j = 0; j < pst_bss_dscr->uc_num_supp_rates; j++) {
            if (IS_EQUAL_RATES(puc_rates[i], pst_bss_dscr->auc_supp_rates[j])) {
                break;
            }
        }

        /* ֻ�в������ظ�����ʱ��j����pst_bss_dscr->uc_num_supp_rates */
        if (j == pst_bss_dscr->uc_num_supp_rates && WLAN_USER_MAX_SUPP_RATES > pst_bss_dscr->uc_num_supp_rates) {
            pst_bss_dscr->auc_supp_rates[pst_bss_dscr->uc_num_supp_rates++] = puc_rates[i];
        }
    }
}


OAL_STATIC OAL_INLINE oal_void hmac_scan_remove_11b_rate(oal_uint8 *puc_avail_rate, oal_uint8 *puc_rate_num)
{
    /* �������ʴ�С��IE�б���Ϊԭ��ֵ��С��2��11b���ʴ�СΪ1, 2, 5.5, 11 */
    oal_uint8 auc_11b_rate[MAC_DATARATES_80211B_NUM] = { 2, 4, 11, 22 };
    oal_uint8 auc_target_rate[WLAN_USER_MAX_SUPP_RATES] = { 0 };
    oal_uint8 uc_target_rate_num = 0;
    oal_bool_enum_uint8 en_is_11b_rate;
    oal_uint8 i, j;

    for (i = 0; i < *puc_rate_num; i++) {
        en_is_11b_rate = OAL_FALSE;

        for (j = 0; j < MAC_DATARATES_80211B_NUM; j++) {
            if (IS_EQUAL_RATES(puc_avail_rate[i], auc_11b_rate[j])) {
                en_is_11b_rate = OAL_TRUE;
                break;
            }
        }

        if (!en_is_11b_rate) {
            auc_target_rate[uc_target_rate_num++] = puc_avail_rate[i];
        }
    }

    /* copyԭ���ʼ����ȣ�������պ������������� */
    if (EOK == memcpy_s(puc_avail_rate, WLAN_USER_MAX_SUPP_RATES, auc_target_rate, *puc_rate_num)) {
        *puc_rate_num = uc_target_rate_num;
        return;
    }
    OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_scan_remove_11b_rate::memcpy fail!");
}


OAL_STATIC OAL_INLINE oal_uint32 hmac_scan_update_bss_list_rates(mac_bss_dscr_stru *pst_bss_dscr,
                                                                 oal_uint8 *puc_frame_body,
                                                                 oal_uint16 us_frame_len,
                                                                 mac_device_stru *pst_mac_dev)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    oal_uint8 uc_num_rates = 0;
    oal_uint8 uc_num_ex_rates = 0;
    oal_uint8 us_offset;
    oal_uint8 auc_rates[WLAN_USER_MAX_SUPP_RATES] = { 0 };

    /* ����Beacon֡��fieldƫ���� */
    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    puc_ie = mac_find_ie_etc(MAC_EID_RATES, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (puc_ie != OAL_PTR_NULL) {
        uc_num_rates = hmac_scan_check_bss_supp_rates_etc(pst_mac_dev, puc_ie + MAC_IE_HDR_LEN, puc_ie[1],
                                                          auc_rates, sizeof(auc_rates));

        
        if (uc_num_rates > WLAN_USER_MAX_SUPP_RATES) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rates::uc_num_rates=%d.}", uc_num_rates);
            uc_num_rates = WLAN_USER_MAX_SUPP_RATES;
        }

        if (EOK != memcpy_s(pst_bss_dscr->auc_supp_rates, WLAN_USER_MAX_SUPP_RATES, auc_rates, uc_num_rates)) {
            OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_update_bss_list_rates::memcpy fail!");
            return OAL_FAIL;
        }

        pst_bss_dscr->uc_num_supp_rates = uc_num_rates;
    }

    puc_ie = mac_find_ie_etc(MAC_EID_XRATES, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (puc_ie != OAL_PTR_NULL) {
        uc_num_ex_rates = hmac_scan_check_bss_supp_rates_etc(pst_mac_dev, puc_ie + MAC_IE_HDR_LEN, puc_ie[1],
                                                             auc_rates, sizeof(auc_rates));

        if (uc_num_rates + uc_num_ex_rates > WLAN_USER_MAX_SUPP_RATES) { /* ����֧�����ʸ��� */
            OAM_WARNING_LOG2(0, OAM_SF_SCAN,
                "{hmac_scan_update_bss_list_rates::number of rates too large, uc_num_rates=%d, uc_num_ex_rates=%d.}",
                uc_num_rates, uc_num_ex_rates);
        }

        if (uc_num_ex_rates > 0) {
            /* support_rates��extended_ratesȥ���ظ����ʣ�һ������ɨ���������ʼ��� */
            hmac_scan_rm_repeat_sup_exsup_rates(pst_bss_dscr, auc_rates, uc_num_ex_rates);
        }
    }

    if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        hmac_scan_remove_11b_rate(pst_bss_dscr->auc_supp_rates, &pst_bss_dscr->uc_num_supp_rates);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R

oal_uint32 mac_ie_get_rsn_akm_tail_pos(oal_uint8 *puc_ie,
                                       oal_int32 *pul_akm_tail_offset,
                                       oal_int32 *pul_akm_cnt_offset)
{
    oal_uint8 *puc_ie_bak;
    oal_uint16 us_suites_count;
    oal_uint16 us_idx;
    oal_uint32 ul_len;

    if (puc_ie == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /*  Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       |         4*m                |     2           |   4*n          */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    puc_ie_bak = puc_ie;

    puc_ie_bak += 1 + 1 + 2;

    /* Group Cipher Suite */
    puc_ie_bak += 4;

    us_suites_count = puc_ie_bak[0] + puc_ie_bak[1] * 8;
    puc_ie_bak += 2;

    /* Pairwise Cipher Suite ����2�� */
    for (us_idx = 0; us_idx < us_suites_count; us_idx++) {
        puc_ie_bak += 4;
    }

    ul_len = (oal_uint32)(puc_ie_bak - puc_ie);
    if (ul_len > puc_ie[1] + 2) {
        return OAL_FAIL;
    }

    *pul_akm_cnt_offset = (oal_int32)(puc_ie_bak - puc_ie);

    us_suites_count = puc_ie_bak[0] + puc_ie_bak[1] * 8;
    puc_ie_bak += 2;

    /* AKM Suite ����2�� */
    for (us_idx = 0; us_idx < us_suites_count; us_idx++) {
        puc_ie_bak += 4;
    }

    ul_len = (oal_uint32)(puc_ie_bak - puc_ie);
    if (ul_len > puc_ie[1] + 2) {
        return OAL_FAIL;
    }

    *pul_akm_tail_offset = (oal_int32)(puc_ie_bak - puc_ie);

    return OAL_SUCC;
}


oal_int32 hmac_scan_attach_akm_suite_to_rsn_ie(oal_uint8 *puc_mgmt_frame, oal_uint16 *pus_frame_len)
{
    oal_uint8 *puc_tagged_para;
    oal_uint32 ul_tagged_para_len;
    oal_uint8 *puc_rsn_ie;
    oal_uint8 *puc_cowork_ie;
    oal_uint8 *puc_md_ie;
    oal_uint32 ul_rsn_ie_len;
    oal_uint8 *puc_rsn_tail;
    oal_uint32 ul_rsn_tail_len;
    oal_uint32 ul_frm_len_before_rsn;
    oal_uint32 ul_frm_len_after_rsn;
    oal_uint32 ul_rsn_ie_akm_offset = 0;
    oal_uint32 ul_rsn_ie_akm_cnt_offset = 0;
    oal_uint32 ul_rsn_ie_end_pos;
    oal_uint8 *puc_rsn_ie_bak;
    oal_uint32 ul_idx;
    oal_uint32 ul_ret;
    oal_int32 l_ret;
    oal_sta_ap_cowork_ie_beacon *pst_cowork_ie;
    oal_cowork_md_ie st_md_ie;
    oal_sta_ap_cowork_akm_para_stru akm_suite_attatch[4] = {
        { MAC_RSN_AKM_FT_OVER_8021X_VAL,        MAC_RSN_AKM_FT_OVER_8021X },
        { MAC_RSN_AKM_FT_PSK_VAL,               MAC_RSN_AKM_FT_PSK },
        { MAC_RSN_AKM_FT_OVER_SAE_VAL,          MAC_RSN_AKM_FT_OVER_SAE },
        { MAC_RSN_AKM_FT_OVER_8021X_SHA384_VAL, MAC_RSN_AKM_FT_OVER_8021X_SHA384 }
    };

    if ((((mac_ieee80211_frame_stru *)puc_mgmt_frame)->st_frame_control.bit_sub_type != WLAN_BEACON) &&
        (((mac_ieee80211_frame_stru *)puc_mgmt_frame)->st_frame_control.bit_sub_type != WLAN_PROBE_RSP)) {
        return OAL_SUCC;
    }

    /* �ҵ� RSN IE ��λ�� puc_rsn_ie  */
    puc_tagged_para = puc_mgmt_frame + MAC_80211_FRAME_LEN +
        (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    ul_tagged_para_len = *pus_frame_len - MAC_80211_FRAME_LEN -
        (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    puc_rsn_ie = mac_find_ie_etc(MAC_EID_RSN, puc_tagged_para, ul_tagged_para_len);
    puc_cowork_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_HUAWEI,
                                           MAC_WLAN_OUI_TYPE_HAUWEI_COWORK, puc_tagged_para, ul_tagged_para_len);

    if ((puc_rsn_ie == OAL_PTR_NULL) || (puc_cowork_ie == OAL_PTR_NULL)) {
        return OAL_SUCC;
    }

    /* �� netbuf ��Ϊ����: ǰ��һ���� 80211֡ͷ��RSN IE(����), ����һ����RSN IE(��)�Ժ�� */
    ul_frm_len_before_rsn = (puc_rsn_ie - puc_mgmt_frame);
    ul_frm_len_after_rsn = *pus_frame_len - ul_frm_len_before_rsn;

    /* RSN IE(����ul_rsn_ie_len)�� RSN IE���������(����ul_rsn_tail_len)��ͬ��ɺ���һ�� */
    ul_rsn_ie_len = puc_rsn_ie[1] + 2;
    ul_rsn_tail_len = ul_frm_len_after_rsn - ul_rsn_ie_len;

    /* �ݴ�RSN IE���������(����ul_rsn_tail_len) */
    puc_rsn_ie_bak = puc_rsn_ie;
    ul_ret = mac_ie_get_rsn_akm_tail_pos(puc_rsn_ie_bak, &ul_rsn_ie_akm_offset, &ul_rsn_ie_akm_cnt_offset);

    if (ul_ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    puc_rsn_tail = (oal_int8 *)kmalloc(ul_rsn_tail_len + (ul_rsn_ie_len - ul_rsn_ie_akm_offset), GFP_KERNEL);
    if (OAL_UNLIKELY(puc_rsn_tail == NULL)) {
        return OAL_FAIL;
    }
    l_ret = memcpy_s(puc_rsn_tail, ul_rsn_tail_len + (ul_rsn_ie_len - ul_rsn_ie_akm_offset),
                     (oal_int8 *)(puc_rsn_ie + ul_rsn_ie_akm_offset),
                     ul_rsn_tail_len + (ul_rsn_ie_len - ul_rsn_ie_akm_offset));

    /* ���� AKM SUITE(FT) */
    pst_cowork_ie = (oal_sta_ap_cowork_ie_beacon *)puc_cowork_ie;
    for (ul_idx = 0; ul_idx < sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]); ul_idx++) {
        if ((pst_cowork_ie->capa_bitmap.akm_suite_val & 0xff) == akm_suite_attatch[ul_idx].real_akm_val) {
            l_ret += memcpy_s(&(puc_rsn_ie[ul_rsn_ie_akm_offset]), sizeof(akm_suite_attatch[ul_idx].akm_suite),
                              &(akm_suite_attatch[ul_idx].akm_suite), sizeof(akm_suite_attatch[ul_idx].akm_suite));
            break;
        } else if (ul_idx == sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]) - 1) {
            kfree(puc_rsn_tail);
            return OAL_FAIL;
        }
    }

    /* ���� AKM COUNT �ֶ�Ϊ 2 */
    puc_rsn_ie[ul_rsn_ie_akm_cnt_offset] = 0x02;

    /* ���� RSN IE �ĳ����ֶ� */
    puc_rsn_ie[1] += 4;

    puc_md_ie = mac_find_ie_etc(MAC_EID_MOBILITY_DOMAIN, puc_tagged_para, ul_tagged_para_len);
    if (puc_md_ie != OAL_PTR_NULL) {
        /* �Ѵ�ǰ���浽 puc_rsn_tail �е����ݿ��������ӵ� AKM SUITE ֮�� */
        l_ret += memcpy_s(&(puc_rsn_ie[ul_rsn_ie_akm_offset + 4]),
                          ul_rsn_tail_len + (ul_rsn_ie_len - ul_rsn_ie_akm_offset),
                          puc_rsn_tail, ul_rsn_tail_len + (ul_rsn_ie_len - ul_rsn_ie_akm_offset));

        /* ���´�����ά���Ĺ���֡�ĳ����ֶ� */
        (*pus_frame_len) += 4;

        kfree(puc_rsn_tail);
        return OAL_SUCC;
    }

    /* ��ǰ����֡��Я�� MDIE, ����ݻ�ͨIE ���� MDIE �����丽�� RSN IE֮�� */
    memset_s(&st_md_ie, OAL_SIZEOF(st_md_ie), 0, OAL_SIZEOF(st_md_ie));
    st_md_ie.tag_num = MAC_EID_MOBILITY_DOMAIN;
    st_md_ie.tag_len = 0x03;
    if (pst_cowork_ie->capa_bitmap.mdid_flag != 0) {
        st_md_ie.mdid = pst_cowork_ie->capa_bitmap.mdid;
    }
    st_md_ie.support_res_req_proto = pst_cowork_ie->capa_bitmap.support_res_req_proto;
    st_md_ie.ft_over_ds = pst_cowork_ie->capa_bitmap.ft_over_ds;

    ul_rsn_ie_end_pos = ul_rsn_ie_akm_offset + 4;
    l_ret += memcpy_s(&(puc_rsn_ie[ul_rsn_ie_end_pos]), (ul_rsn_ie_len - ul_rsn_ie_akm_offset),
                      puc_rsn_tail, (ul_rsn_ie_len - ul_rsn_ie_akm_offset));

    ul_rsn_ie_end_pos += (ul_rsn_ie_len - ul_rsn_ie_akm_offset);

    l_ret += memcpy_s(&puc_rsn_ie[ul_rsn_ie_end_pos], sizeof(st_md_ie), &st_md_ie, sizeof(st_md_ie));
    ul_rsn_ie_end_pos += sizeof(st_md_ie);

    l_ret += memcpy_s(&(puc_rsn_ie[ul_rsn_ie_end_pos]), ul_rsn_tail_len,
                      puc_rsn_tail + (ul_rsn_ie_len - ul_rsn_ie_akm_offset), ul_rsn_tail_len);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_scan_attach_akm_suite_to_rsn_ie::memcpy fail!");
        kfree(puc_rsn_tail);
        return OAL_FAIL;
    }

    (*pus_frame_len) += (4 + 5);
    kfree(puc_rsn_tail);

    return OAL_SUCC;
}


oal_uint8 hmac_scan_extend_mgmt_len_needed(oal_netbuf_stru *pst_netbuf, oal_uint16 us_netbuf_len)
{
    oal_uint8 *puc_mgmt_frame = (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf);
    oal_uint16 us_frame_len;
    oal_uint16 us_len_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    const oal_uint8 uc_akm_val = DEFAULT_AKM_VALUE;
    oal_uint8 *puc_tagged_para;
    oal_uint32 ul_tagged_para_len;
    oal_uint8 *puc_rsn_ie;
    oal_uint8 *puc_rsn_ie_bak;
    oal_uint8 *puc_md_ie;
    oal_uint8 *puc_cowork_ie;
    oal_uint32 ul_idx;
    oal_uint32 ul_rsn_ie_akm_offset = 0;
    oal_uint32 ul_rsn_ie_akm_cnt_offset = 0;
    oal_uint32 ul_ret;
    oal_sta_ap_cowork_ie_beacon *pst_cowork_ie;
    oal_sta_ap_cowork_akm_para_stru akm_suite_attatch[4] = {
        { MAC_RSN_AKM_FT_OVER_8021X_VAL,        MAC_RSN_AKM_FT_OVER_8021X },
        { MAC_RSN_AKM_FT_PSK_VAL,               MAC_RSN_AKM_FT_PSK },
        { MAC_RSN_AKM_FT_OVER_SAE_VAL,          MAC_RSN_AKM_FT_OVER_SAE },
        { MAC_RSN_AKM_FT_OVER_8021X_SHA384_VAL, MAC_RSN_AKM_FT_OVER_8021X_SHA384 }
    };

    if (us_netbuf_len < (us_len_offset + OAL_SIZEOF(mac_scanned_result_extend_info_stru))) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_extend_mgmt_len_needed:: netbuf len[%d] err.}", us_netbuf_len);
        return uc_akm_val;
    }

    us_frame_len = us_netbuf_len - OAL_SIZEOF(mac_scanned_result_extend_info_stru);
    puc_tagged_para = puc_mgmt_frame + us_len_offset;
    ul_tagged_para_len = us_frame_len - us_len_offset;

    puc_cowork_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_HUAWEI,
                                           MAC_WLAN_OUI_TYPE_HAUWEI_COWORK, puc_tagged_para, ul_tagged_para_len);
    puc_rsn_ie = mac_find_ie_etc(MAC_EID_RSN, puc_tagged_para, ul_tagged_para_len);
    puc_md_ie = mac_find_ie_etc(MAC_EID_MOBILITY_DOMAIN, puc_tagged_para, ul_tagged_para_len);

    /* ����֮һ: ����Я����ͨIE��RSN IE */
    if ((puc_cowork_ie == OAL_PTR_NULL) || (puc_rsn_ie == OAL_PTR_NULL)) {
        return uc_akm_val;
    }

    /* ����֮��: ��ͨIE�� Adaptive 11r�ֶα���ʹ�� */
    pst_cowork_ie = (oal_sta_ap_cowork_ie_beacon *)puc_cowork_ie;
    if (pst_cowork_ie->capa_bitmap.support_adaptive_11r == 0) {
        return uc_akm_val;
    }

    /* ����֮��: RSN IE�� AKM SUITE COUNT ����Ϊ 1 */
    puc_rsn_ie_bak = puc_rsn_ie;
    ul_ret = mac_ie_get_rsn_akm_tail_pos(puc_rsn_ie_bak, &ul_rsn_ie_akm_offset, &ul_rsn_ie_akm_cnt_offset);
    if ((ul_ret != OAL_SUCC) || (puc_rsn_ie[ul_rsn_ie_akm_cnt_offset] != 0x01) ||
        (puc_rsn_ie[ul_rsn_ie_akm_cnt_offset + 1] != 0x00)) {
        return uc_akm_val;
    }

    /* ����֮��: RSN IE��AKM SUITE���벻֧��11R */
    for (ul_idx = 0; ul_idx < sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]); ul_idx++) {
        if (akm_suite_attatch[ul_idx].akm_suite == CIPHER_SUITE_SELECTOR(puc_rsn_ie[ul_rsn_ie_akm_offset - 4],
            puc_rsn_ie[ul_rsn_ie_akm_offset - 3], puc_rsn_ie[ul_rsn_ie_akm_offset - 2],
            puc_rsn_ie[ul_rsn_ie_akm_offset - 1])) {
            return uc_akm_val;
        }
    }

    /* ����֮��: ��ͨIE����Я���� cowork_ie.capa_bitmap.akm_suite_val����֧��FT */
    for (ul_idx = 0; ul_idx < sizeof(akm_suite_attatch) / sizeof(akm_suite_attatch[0]); ul_idx++) {
        if ((pst_cowork_ie->capa_bitmap.akm_suite_val & 0xff) == akm_suite_attatch[ul_idx].real_akm_val) {
            /* �������֡Я�� MDIE, ��ֻ����չ AKM Suite �ĳ���; ��������չ (AKM Suite + MDIE) �ĳ��� */
            return ((puc_md_ie == OAL_PTR_NULL) ? (4 + OAL_SIZEOF(oal_cowork_md_ie)) : 4);
        }
    }

    return uc_akm_val;
}

#endif

oal_bool_enum_uint8 hmac_scan_is_hidden_ssid(oal_uint8 uc_vap_id,
                                             hmac_scanned_bss_info *pst_new_bss,
                                             hmac_scanned_bss_info *pst_old_bss)
{
    if ((pst_new_bss->st_bss_dscr_info.ac_ssid[0] == '\0') &&
        (pst_old_bss->st_bss_dscr_info.ac_ssid[0] != '\0')) {
        /*  ����SSID,���������AP��Ϣ,��ssid��Ϊ��,�˴�ͨ��BEACON֡ɨ�赽��AP��Ϣ,��SSIDΪ��,�򲻽��и��� */
        OAM_WARNING_LOG3(uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_is_hidden_ssid::find hide ssid:%.2x:%.2x:%.2x,ignore this update.}",
                         pst_new_bss->st_bss_dscr_info.auc_bssid[3],
                         pst_new_bss->st_bss_dscr_info.auc_bssid[4],
                         pst_new_bss->st_bss_dscr_info.auc_bssid[5]);
        return OAL_TRUE;
    }
    return OAL_FALSE;
}


oal_bool_enum_uint8 hmac_scan_need_update_old_scan_result_etc(oal_uint8 uc_vap_id,
                                                              hmac_scanned_bss_info *pst_new_bss,
                                                              hmac_scanned_bss_info *pst_old_bss)
{
    
    if ((((mac_ieee80211_frame_stru *)pst_old_bss->st_bss_dscr_info.auc_mgmt_buff)->st_frame_control.bit_sub_type == WLAN_PROBE_RSP) &&
        (((mac_ieee80211_frame_stru *)pst_new_bss->st_bss_dscr_info.auc_mgmt_buff)->st_frame_control.bit_sub_type == WLAN_BEACON) &&
        (pst_old_bss->st_bss_dscr_info.en_new_scan_bss == OAL_TRUE)) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}


oal_uint8 hmac_scan_check_chan(oal_netbuf_stru *pst_netbuf, hmac_scanned_bss_info *pst_scanned_bss)
{
    dmac_rx_ctl_stru *pst_rx_ctrl;
    oal_uint8 uc_curr_chan;
    oal_uint8 *puc_frame_body;
    oal_uint16 us_frame_body_len;
    oal_uint16 us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    oal_uint8 *puc_ie_start_addr;
    oal_uint8 uc_chan_num;

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    uc_curr_chan = pst_rx_ctrl->st_rx_info.uc_channel_number;
    puc_frame_body = pst_scanned_bss->st_bss_dscr_info.auc_mgmt_buff + MAC_80211_FRAME_LEN;
    us_frame_body_len = pst_scanned_bss->st_bss_dscr_info.ul_mgmt_len - MAC_80211_FRAME_LEN;

    /* ��DSSS Param set ie�н���chan num */
    puc_ie_start_addr = mac_find_ie_etc(MAC_EID_DSPARMS, puc_frame_body + us_offset, us_frame_body_len - us_offset);
    if ((puc_ie_start_addr != OAL_PTR_NULL) && (puc_ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        uc_chan_num = puc_ie_start_addr[2];
        if (OAL_SUCC != mac_is_channel_num_valid_etc(mac_get_band_by_channel_num(uc_chan_num), uc_chan_num)) {
            return OAL_FALSE;
        }
    }

    /* ��HT operation ie�н��� chan num */
    puc_ie_start_addr = mac_find_ie_etc(MAC_EID_HT_OPERATION, puc_frame_body + us_offset,
                                        us_frame_body_len - us_offset);
    if ((puc_ie_start_addr != OAL_PTR_NULL) && (puc_ie_start_addr[1] >= 1)) {
        uc_chan_num = puc_ie_start_addr[2];
        if (OAL_SUCC != mac_is_channel_num_valid_etc(mac_get_band_by_channel_num(uc_chan_num), uc_chan_num)) {
            return OAL_FALSE;
        }
    }

    uc_chan_num = pst_scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number;
    if (((uc_curr_chan > uc_chan_num) && (uc_curr_chan - uc_chan_num >= 3)) ||
        ((uc_curr_chan < uc_chan_num) && (uc_chan_num - uc_curr_chan >= 3))) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


OAL_STATIC oal_uint32 hmac_scan_update_bss_dscr(hmac_vap_stru *pst_hmac_vap,
                                                oal_netbuf_stru *pst_netbuf,
                                                oal_uint16 us_buf_len,
                                                hmac_scanned_bss_info *pst_scanned_bss,
                                                oal_uint8 uc_ext_len)
{
    mac_scanned_result_extend_info_stru *pst_scan_result_extend_info = OAL_PTR_NULL;
    mac_device_stru *pst_mac_device;
    mac_ieee80211_frame_stru *pst_frame_header = OAL_PTR_NULL;
    oal_uint8 *puc_frame_body = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    oal_uint8 *puc_ssid = OAL_PTR_NULL; /* ָ��beacon֡�е�ssid */
    oal_uint8 *puc_mgmt_frame = OAL_PTR_NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    oal_uint16 us_frame_len;
    oal_uint16 us_frame_body_len;
    oal_uint16 us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    oal_uint8 uc_ssid_len;
    oal_uint8 uc_frame_channel;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32 ul_ret;
#endif
    oal_int32 l_ret;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    struct timespec ts;
#else
    struct timeval tv;
#endif
#endif

    /* ��ȡmac device */
    pst_mac_device = mac_res_get_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_update_bss_dscr::pst_mac_device is null.}");
        return OAL_FAIL;
    }

    /* ��ȡdevice�ϱ���ɨ������Ϣ����������µ�bss�����ṹ���� */
    us_frame_len = us_buf_len - OAL_SIZEOF(mac_scanned_result_extend_info_stru);
    puc_mgmt_frame = (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf);
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* ָ��netbuf�е��ϱ���ɨ��������չ��Ϣ��λ�� */
    pst_scan_result_extend_info = (mac_scanned_result_extend_info_stru *)(puc_mgmt_frame + us_frame_len);

    /* ��������֡���� */
    l_ret = memcpy_s(pst_scanned_bss->st_bss_dscr_info.auc_mgmt_buff, (oal_uint32)us_frame_len,
                     puc_mgmt_frame, (oal_uint32)us_frame_len);
    puc_mgmt_frame = pst_scanned_bss->st_bss_dscr_info.auc_mgmt_buff;

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* ���ݻ�ͨIE���akm suite(FT)��RSN IE */
    if (uc_ext_len != DEFAULT_AKM_VALUE) {
        hmac_scan_attach_akm_suite_to_rsn_ie(puc_mgmt_frame, &us_frame_len);
    }
#endif

    /* ��ȡ����֡��֡ͷ��֡��ָ�� */
    pst_frame_header = (mac_ieee80211_frame_stru *)puc_mgmt_frame;
    puc_frame_body = puc_mgmt_frame + MAC_80211_FRAME_LEN;
    us_frame_body_len = us_frame_len - MAC_80211_FRAME_LEN;

    /* ��ȡ����֡�е��ŵ� */
    uc_frame_channel = mac_ie_get_chan_num_etc(puc_frame_body, us_frame_body_len, us_offset,
                                               pst_rx_ctrl->st_rx_info.uc_channel_number);

    /* ����bss��Ϣ */
    pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

    /*****************************************************************************
        ����beacon/probe rsp֡����¼��pst_bss_dscr
    *****************************************************************************/
    /* ����������ssid */
    puc_ssid = mac_get_ssid_etc(puc_frame_body, (oal_int32)us_frame_body_len, &uc_ssid_len);
    if ((puc_ssid != OAL_PTR_NULL) && (uc_ssid_len != 0)) {
        /* �����ҵ���ssid���浽bss�����ṹ���� */
        l_ret += memcpy_s(pst_bss_dscr->ac_ssid, WLAN_SSID_MAX_LEN, puc_ssid, uc_ssid_len);
        pst_bss_dscr->ac_ssid[uc_ssid_len] = '\0';
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_update_bss_dscr::memcpy fail!");
        return OAL_FAIL;
    }

    /* ����bssid */
    oal_set_mac_addr(pst_bss_dscr->auc_mac_addr, pst_frame_header->auc_address2);
    oal_set_mac_addr(pst_bss_dscr->auc_bssid, pst_frame_header->auc_address3);

    /* bss������Ϣ */
    pst_bss_dscr->en_bss_type = pst_scan_result_extend_info->en_bss_type;

    pst_bss_dscr->us_cap_info = *((oal_uint16 *)(puc_frame_body + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN));

    pst_bss_dscr->c_rssi = (oal_int8)pst_scan_result_extend_info->l_rssi;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    pst_bss_dscr->c_ant0_rssi = (oal_int8)pst_scan_result_extend_info->c_ant0_rssi;
    pst_bss_dscr->c_ant1_rssi = (oal_int8)pst_scan_result_extend_info->c_ant1_rssi;
#endif
    /* ����beacon������tim���� */
    pst_bss_dscr->us_beacon_period = mac_get_beacon_period_etc(puc_frame_body);
    pst_bss_dscr->uc_dtim_period = mac_get_dtim_period_etc(puc_frame_body, us_frame_body_len);
    pst_bss_dscr->uc_dtim_cnt = mac_get_dtim_cnt_etc(puc_frame_body, us_frame_body_len);

    /* �ŵ� */
    pst_bss_dscr->st_channel.uc_chan_number = uc_frame_channel;
    pst_bss_dscr->st_channel.en_band = mac_get_band_by_channel_num(uc_frame_channel);

    /* ��¼���ʼ� */
    hmac_scan_update_bss_list_rates(pst_bss_dscr, puc_frame_body, us_frame_body_len, pst_mac_device);

    /* 03��¼֧�ֵ����ռ��� */
#ifdef _PRE_WLAN_FEATURE_M2S
    pst_bss_dscr->en_support_max_nss = pst_scan_result_extend_info->en_support_max_nss;
    pst_bss_dscr->en_support_opmode = pst_scan_result_extend_info->en_support_opmode;
    pst_bss_dscr->uc_num_sounding_dim = pst_scan_result_extend_info->uc_num_sounding_dim;
#endif

    /* Э���������ϢԪ�صĻ�ȡ */
    hmac_scan_update_bss_list_protocol(pst_hmac_vap, pst_bss_dscr, puc_frame_body, us_frame_body_len);

    /* update st_channel.bandwidth in case hmac_sta_update_join_req_params_etc usage error */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_bss_dscr->st_channel.en_bandwidth = pst_bss_dscr->en_channel_bandwidth;
    ul_ret = mac_get_channel_idx_from_num_etc(pst_bss_dscr->st_channel.en_band,
                                              pst_bss_dscr->st_channel.uc_chan_number,
                                              &pst_bss_dscr->st_channel.uc_chan_idx);
    if (ul_ret == OAL_ERR_CODE_INVALID_CONFIG) {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{hmac_scan_update_bss_dscr::mac_get_channel_idx_from_num_etc fail.}");
    }
#endif

    /* ����ʱ��� */
    pst_bss_dscr->ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    get_monotonic_boottime(&ts);
    pst_bss_dscr->ul_wpa_rpt_time = ((u64)(ts.tv_sec) * 1000000) + ts.tv_nsec / 1000;
#else
    do_gettimeofday(&tv);
    pst_bss_dscr->ul_wpa_rpt_time = ((u64)tv.tv_sec * 1000000) + tv.tv_usec;
#endif
#endif

    pst_bss_dscr->ul_mgmt_len = us_frame_len;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    if (pst_hmac_vap->bit_11k_enable) {
        /* RSNI */
        pst_bss_dscr->ac_rsni[0] = pst_scan_result_extend_info->c_snr_ant0;
        pst_bss_dscr->ac_rsni[1] = pst_scan_result_extend_info->c_snr_ant1;
        /* phy type */
        if (pst_bss_dscr->st_channel.en_band == WLAN_BAND_2G) {
            pst_bss_dscr->uc_phy_type = PHY_TYPE_DSSS;
        } else {
            pst_bss_dscr->uc_phy_type = PHY_TYPE_OFDM;
        }
        /* Parent TSF */
        pst_bss_dscr->ul_parent_tsf = pst_scan_result_extend_info->ul_parent_tsf;
    }
#endif

    return OAL_SUCC;
}


oal_uint32 hmac_scan_proc_scan_result_handle(hmac_vap_stru *pst_hmac_vap,
                                             oal_netbuf_stru *pst_netbuf,
                                             oal_uint16 us_buf_len,
                                             mac_multi_bssid_frame_info_stru *pst_mbss_frame_info)
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_new_scanned_bss = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_old_scanned_bss = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint16 us_mgmt_len;
    oal_uint8 uc_vap_id;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;
    oal_uint32 ul_curr_time_stamp;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_ieee80211_frame_stru *pst_frame_header;
#endif
    oal_uint8 uc_ext_len = DEFAULT_AKM_VALUE;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
    struct timespec ts;
#else
    struct timeval tv;
#endif
#endif

    /* ��ȡvap id */
    uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;

    /* ��ȡhmac device �ṹ */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_result_handle::pst_hmac_device null.}");

        /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��dmac�ϱ���netbuf���ݽ��н���������������ʾ */
    /***********************************************************************************************/
    /*            netbuf data����ϱ���ɨ�������ֶεķֲ�                                        */
    /* ------------------------------------------------------------------------------------------  */
    /* beacon/probe rsp body  |     ֡����渽���ֶ�(mac_scanned_result_extend_info_stru)          */
    /* -----------------------------------------------------------------------------------------   */
    /* �յ���beacon/rsp��body | rssi(4�ֽ�) | channel num(1�ֽ�)| band(1�ֽ�)|bss_tye(1�ֽ�)|���  */
    /* ------------------------------------------------------------------------------------------  */
    /*                                                                                             */
    /***********************************************************************************************/
    /* ����֡�ĳ��ȵ����ϱ���netbuf�ĳ��ȼ�ȥ�ϱ���ɨ��������չ�ֶεĳ��� */
    us_mgmt_len = us_buf_len - OAL_SIZEOF(mac_scanned_result_extend_info_stru);

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* adaptive 11Rģʽ�¼���Ƿ���Ҫ��RSN IE������akm suite */
    if (pst_hmac_vap->bit_adaptive11r == OAL_TRUE) {
        uc_ext_len = hmac_scan_extend_mgmt_len_needed(pst_netbuf, us_buf_len);
        if (uc_ext_len != DEFAULT_AKM_VALUE) {
            us_mgmt_len += uc_ext_len;
        }
    }
#endif

    /* ����洢ɨ�������ڴ� */
    pst_new_scanned_bss = hmac_scan_alloc_scanned_bss(us_mgmt_len);
    if (OAL_UNLIKELY(pst_new_scanned_bss == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_result_handle::alloc memory failed for storing scanned result.}");

        /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��������ɨ������bss dscr�ṹ�� */
    ul_ret = hmac_scan_update_bss_dscr(pst_hmac_vap, pst_netbuf, us_buf_len, pst_new_scanned_bss, uc_ext_len);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_result_handle::hmac_scan_update_bss_dscr failed[%d].}", ul_ret);

        /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
        oal_netbuf_free(pst_netbuf);

        /* �ͷ�����Ĵ洢bss��Ϣ���ڴ� */
        oal_free(pst_new_scanned_bss);
        return ul_ret;
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    /* ����mbss info */
    if (IS_CUSTOM_OPEN_MULTI_BSSID_SWITCH(&pst_hmac_vap->st_vap_base_info)) {
        memcpy_s(pst_new_scanned_bss->st_bss_dscr_info.st_mbssid_info.auc_transmitted_bssid, WLAN_MAC_ADDR_LEN,
                 pst_mbss_frame_info->auc_transmitted_bssid, WLAN_MAC_ADDR_LEN);
        pst_new_scanned_bss->st_bss_dscr_info.st_mbssid_info.bit_is_non_transimitted_bss =
            pst_mbss_frame_info->bit_is_non_transimitted_bss;
    }
#endif
    /* ��ȡ����ɨ���bss����Ľṹ�� */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    /* ������ɾ����ǰ���� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));
    /* �ж���ͬbssid��bss�Ƿ��Ѿ�ɨ�赽 */
    pst_old_scanned_bss = hmac_scan_find_scanned_bss_by_bssid_etc(pst_bss_mgmt,
                                                                  pst_new_scanned_bss->st_bss_dscr_info.auc_bssid);
    if (pst_old_scanned_bss == OAL_PTR_NULL) {
        /* ���� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));
        /*lint -e801*/
        goto add_bss;
        /*lint +e801*/
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* ֻ��probe rsp֡��ext cap ����֧��OPMODEʱ���Զ˲�ȷʵ֧��OPMODE��beacon֡��assoc rsp֡����Ϣ������ */
    pst_frame_header = (mac_ieee80211_frame_stru *)pst_new_scanned_bss->st_bss_dscr_info.auc_mgmt_buff;
    if (pst_frame_header->st_frame_control.bit_sub_type == WLAN_PROBE_RSP) {
        pst_old_scanned_bss->st_bss_dscr_info.en_support_opmode =
            pst_new_scanned_bss->st_bss_dscr_info.en_support_opmode;
    }
#endif

    /* ����ϵ�ɨ���bss���ź�ǿ�ȴ��ڵ�ǰɨ�赽��bss���ź�ǿ�ȣ����µ�ǰɨ�赽���ź�ǿ��Ϊ��ǿ���ź�ǿ�� */
    if (pst_old_scanned_bss->st_bss_dscr_info.c_rssi > pst_new_scanned_bss->st_bss_dscr_info.c_rssi) {
        /* 1s�����ھͲ���֮ǰ��BSS�����RSSI��Ϣ������Ͳ����µ�RSSI��Ϣ */
        ul_curr_time_stamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

        if (oal_time_after32((ul_curr_time_stamp),
            (pst_old_scanned_bss->st_bss_dscr_info.ul_timestamp + HMAC_SCAN_MAX_SCANNED_RSSI_EXPIRE)) == FALSE) {
            OAM_INFO_LOG0(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_result_handle::update signal strength value.}");
            pst_new_scanned_bss->st_bss_dscr_info.c_rssi = pst_old_scanned_bss->st_bss_dscr_info.c_rssi;
        }
    }

    if (OAL_TRUE == hmac_scan_is_hidden_ssid(uc_vap_id, pst_new_scanned_bss, pst_old_scanned_bss)) {
        /* ���� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));

        /* �ͷ�����Ĵ洢bss��Ϣ���ڴ� */
        oal_free(pst_new_scanned_bss);

        /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
        oal_netbuf_free(pst_netbuf);

        return OAL_SUCC;
    }

    if (OAL_FALSE == hmac_scan_need_update_old_scan_result_etc(uc_vap_id, pst_new_scanned_bss, pst_old_scanned_bss)
        || OAL_FALSE == hmac_scan_check_chan(pst_netbuf, pst_new_scanned_bss)) {
        pst_old_scanned_bss->st_bss_dscr_info.ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
        get_monotonic_boottime(&ts);
        pst_old_scanned_bss->st_bss_dscr_info.ul_wpa_rpt_time = ((u64)(ts.tv_sec) * 1000000)
                                                                + ts.tv_nsec / 1000;
#else
        do_gettimeofday(&tv);
        pst_old_scanned_bss->st_bss_dscr_info.ul_wpa_rpt_time = ((u64)tv.tv_sec * 1000000)
                                                                + tv.tv_usec;
#endif
#endif
        pst_old_scanned_bss->st_bss_dscr_info.c_rssi = pst_new_scanned_bss->st_bss_dscr_info.c_rssi;

        /* ���� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));

        /* �ͷ�����Ĵ洢bss��Ϣ���ڴ� */
        oal_free(pst_new_scanned_bss);

        /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
        oal_netbuf_free(pst_netbuf);

        return OAL_SUCC;
    }

    /* �������н�ԭ��ɨ�赽����ͬbssid��bss�ڵ�ɾ�� */
    hmac_scan_del_bss_from_list_nolock(pst_old_scanned_bss, pst_hmac_device);
    /* ���� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));
    /* �ͷ��ڴ� */
    oal_free(pst_old_scanned_bss);
add_bss:

    /* ��ɨ�������ӵ������� */
    hmac_scan_add_bss_to_list(pst_new_scanned_bss, pst_hmac_device);
    /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
    oal_netbuf_free(pst_netbuf);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC oal_uint32 hmac_scan_get_non_transmitted_bssid(oal_uint8 *puc_tx_bssid, oal_uint8 uc_tx_bssid_len,
                                                          oal_uint8 uc_max_indicator, oal_uint8 uc_non_bssid_index,
                                                          oal_uint8 *puc_non_tx_bssid, oal_uint8 uc_ntx_bssid_len)
{
#define MAC_ADDR_MASK       0xFFFFFFFFFFFF
#define MAX_BSSID_INDICATOR 46
    oal_uint8 uc_index;
    oal_uint8 auc_non_tx_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8 auc_bssid_tmp[WLAN_MAC_ADDR_LEN];
    oal_uint64 ull_bssid = 0;
    oal_uint64 ull_low_bssid;
    oal_uint64 ull_bssid_mask;
    oal_int32 l_ret;

    /* Э��涨������� */
    if (uc_max_indicator >= MAX_BSSID_INDICATOR) {
        return OAL_FAIL;
    }

    if ((uc_ntx_bssid_len != WLAN_MAC_ADDR_LEN) || (uc_tx_bssid_len != WLAN_MAC_ADDR_LEN)) {
        return OAL_FAIL;
    }

    ull_bssid_mask = ((MAC_ADDR_MASK << uc_max_indicator) ^ MAC_ADDR_MASK);

    /* �ֽڷ�ת */
    for (uc_index = 0; uc_index < 6; uc_index++) {
        auc_bssid_tmp[uc_index] = puc_tx_bssid[5 - uc_index];
    }
    l_ret = memcpy_s(&ull_bssid, sizeof(oal_uint64), auc_bssid_tmp, WLAN_MAC_ADDR_LEN);
    ull_low_bssid = ull_bssid & ull_bssid_mask;
    ull_bssid &= (~ull_bssid_mask);
    ull_low_bssid = ((ull_low_bssid + uc_non_bssid_index) % (1ULL << uc_max_indicator));
    ull_bssid = ((ull_bssid & (~ull_bssid_mask)) | (ull_low_bssid));
    l_ret += memcpy_s(auc_non_tx_bssid, WLAN_MAC_ADDR_LEN, (oal_uint8 *)&ull_bssid, 6);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_scan_get_non_transmitted_bssid::memcpy fail!");
        return OAL_FAIL;
    }

    for (uc_index = 0; uc_index < 6; uc_index++) {
        puc_non_tx_bssid[uc_index] = auc_non_tx_bssid[5 - uc_index];
    }

    return OAL_SUCC;
}


oal_netbuf_stru *hmac_scan_proc_non_transmitted_ssid_handle(oal_netbuf_stru *pst_netbuf,
                                                            oal_uint16 us_buf_len,
                                                            mac_multi_bssid_frame_info_stru *pst_mbss_frame_info,
                                                            oal_uint16 *pus_new_buf_len)
{
    oal_uint8 *puc_transmitted_mgmt_frame;
    dmac_rx_ctl_stru *pst_transmitted_rx_ctrl;
    oal_uint8 *puc_transmitted_mgmt_frame_body;
    oal_uint8 *puc_transmitted_ssid;
    oal_uint8 *puc_transmitted_ssid_ie = OAL_PTR_NULL;
    oal_uint8 *puc_after_transmitted_ssid_ie = OAL_PTR_NULL;
    oal_netbuf_stru *pst_new_netbuf = OAL_PTR_NULL;
    dmac_rx_ctl_stru *pst_new_rx_ctrl = OAL_PTR_NULL;
    oal_uint8 *puc_new_netbuf_data = OAL_PTR_NULL;
    oal_uint8 uc_transmitted_ssid_len;
    oal_uint16 us_transmitted_mgmt_frame_len;
    oal_uint16 us_transmitted_mgmt_frame_body_len;
    oal_uint16 us_before_ssid_ie_len;
    oal_uint16 us_use_buf_len;
    oal_uint16 us_after_ssid_ie_len;
    mac_ieee80211_frame_stru *pst_frame_header = OAL_PTR_NULL;
    oal_int32 l_ret;

    /* ��ȡ����֡֡���֡�� */
    puc_transmitted_mgmt_frame = (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf);
    pst_transmitted_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    us_transmitted_mgmt_frame_len = us_buf_len - OAL_SIZEOF(mac_scanned_result_extend_info_stru);

    puc_transmitted_mgmt_frame_body = puc_transmitted_mgmt_frame + MAC_80211_FRAME_LEN;
    us_transmitted_mgmt_frame_body_len =
        us_buf_len - OAL_SIZEOF(mac_scanned_result_extend_info_stru) - MAC_80211_FRAME_LEN;

    /* ��ȡtransimitted ֡��ssid ���� */
    puc_transmitted_ssid = mac_get_ssid_etc(puc_transmitted_mgmt_frame_body,
                                            (oal_int32)us_transmitted_mgmt_frame_body_len,
                                            &uc_transmitted_ssid_len);
    if ((puc_transmitted_ssid != OAL_PTR_NULL) && (uc_transmitted_ssid_len == 0)) {
        return OAL_PTR_NULL;
    }

    /* ����ssid ie ֮ǰ��֡�� */
    puc_transmitted_ssid_ie = puc_transmitted_ssid - MAC_IE_HDR_LEN;
    us_before_ssid_ie_len = puc_transmitted_ssid_ie - puc_transmitted_mgmt_frame;

    /* ����ssid ie ֮��֡�� */
    puc_after_transmitted_ssid_ie = puc_transmitted_ssid + uc_transmitted_ssid_len;
    us_after_ssid_ie_len = us_buf_len - (puc_after_transmitted_ssid_ie - puc_transmitted_mgmt_frame);

    /* ��Ҫ��������netbuf �ڴ� */
    *pus_new_buf_len = us_buf_len - uc_transmitted_ssid_len + pst_mbss_frame_info->uc_non_transmitted_ssid_len;
    pst_new_netbuf = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                             *pus_new_buf_len,
                                                             OAL_NETBUF_PRIORITY_MID);
    if (pst_new_netbuf == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN,
                       "{hmac_scan_proc_non_transmitted_ssid_handle::Allco netbuf_len=[%d] fail}",
                       *pus_new_buf_len);
        return OAL_PTR_NULL;
    }

    /* copy cb �ֶ� */
    pst_new_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_new_netbuf);
    l_ret = memcpy_s(pst_new_rx_ctrl, OAL_SIZEOF(dmac_rx_ctl_stru),
                     pst_transmitted_rx_ctrl, OAL_SIZEOF(dmac_rx_ctl_stru));

    /* copy  ssid ie ֮ǰ�Ĳ��� */
    puc_new_netbuf_data = (oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf);
    /* ��鳤���Ƿ��㹻 */
    us_use_buf_len = us_before_ssid_ie_len;
    if (*pus_new_buf_len < us_use_buf_len) {
        OAM_WARNING_LOG2(0, OAM_SF_SCAN,
                         "{hmac_scan_proc_non_transmitted_ssid_handle::us_use_buf_len=[%d] over netbuf_len=[%d]}",
                         us_use_buf_len, *pus_new_buf_len);

        oal_netbuf_free(pst_new_netbuf);
        return OAL_PTR_NULL;
    }
    l_ret += memcpy_s(puc_new_netbuf_data, *pus_new_buf_len, puc_transmitted_mgmt_frame, us_before_ssid_ie_len);
    // �滻mac header �е�bssid �� ta
    pst_frame_header = (mac_ieee80211_frame_stru *)puc_new_netbuf_data;
    l_ret += memcpy_s(pst_frame_header->auc_address2, WLAN_MAC_ADDR_LEN,
                      pst_mbss_frame_info->auc_non_transmitted_bssid, WLAN_MAC_ADDR_LEN);
    l_ret += memcpy_s(pst_frame_header->auc_address3, WLAN_MAC_ADDR_LEN,
                      pst_mbss_frame_info->auc_non_transmitted_bssid, WLAN_MAC_ADDR_LEN);

    puc_new_netbuf_data += us_before_ssid_ie_len;
    /* copy ssid ie */
    us_use_buf_len += (MAC_IE_HDR_LEN + pst_mbss_frame_info->uc_non_transmitted_ssid_len);
    if (*pus_new_buf_len < us_use_buf_len) {
        OAM_WARNING_LOG2(0, OAM_SF_SCAN,
                         "{hmac_scan_proc_non_transmitted_ssid_handle::us_use_buf_len=[%d] over netbuf_len=[%d]}",
                         us_use_buf_len, *pus_new_buf_len);
        oal_netbuf_free(pst_new_netbuf);
        return OAL_PTR_NULL;
    }
    puc_new_netbuf_data[0] = MAC_EID_SSID;
    puc_new_netbuf_data[1] = pst_mbss_frame_info->uc_non_transmitted_ssid_len;
    puc_new_netbuf_data += MAC_IE_HDR_LEN;
    l_ret += memcpy_s(puc_new_netbuf_data, pst_mbss_frame_info->uc_non_transmitted_ssid_len,
                      pst_mbss_frame_info->auc_non_transmitted_ssid, pst_mbss_frame_info->uc_non_transmitted_ssid_len);
    puc_new_netbuf_data += pst_mbss_frame_info->uc_non_transmitted_ssid_len;

    /* copy ssid ie ֮��Ĳ��� */
    us_use_buf_len += us_after_ssid_ie_len;

    if (*pus_new_buf_len < us_use_buf_len) {
        OAM_WARNING_LOG2(0, OAM_SF_SCAN,
                         "{hmac_scan_proc_non_transmitted_ssid_handle::us_use_buf_len=[%d] over netbuf_len=[%d]}",
                         us_use_buf_len, *pus_new_buf_len);
        oal_netbuf_free(pst_new_netbuf);
        return OAL_PTR_NULL;
    }
    l_ret += memcpy_s(puc_new_netbuf_data, us_after_ssid_ie_len, puc_after_transmitted_ssid_ie, us_after_ssid_ie_len);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_proc_non_transmitted_ssid_handle::memcpy fail!");
        oal_netbuf_free(pst_new_netbuf);
        return OAL_PTR_NULL;
    }

    return pst_new_netbuf;
}

#ifdef _PRE_DEBUG_CODE
void hmac_scan_mbssid_printf(mac_multi_bssid_frame_info_stru *pst_mbssid)
{
    OAM_ERROR_LOG4(0, OAM_SF_SCAN,
        "{>>>>>>>::bit_is_non_transimitted_bss=%d,maxbssid_indicator=%d,bssid_index=%d,non_transmitted_ssid_len=%d.}",
        pst_mbssid->bit_is_non_transimitted_bss, pst_mbssid->uc_maxbssid_indicator,
        pst_mbssid->uc_bssid_index, pst_mbssid->uc_non_transmitted_ssid_len);

    OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{>>>>>>>::us_non_tramsmitted_bssid_cap=[%d].}",
                   pst_mbssid->us_non_tramsmitted_bssid_cap);

    OAM_ERROR_LOG4(0, OAM_SF_UM, "{>>>>>>>::Transmitted BSSID:%02X:%02X:XX:XX:%02X:%02X.}",
                   pst_mbssid->auc_transmitted_bssid[0],
                   pst_mbssid->auc_transmitted_bssid[1],
                   pst_mbssid->auc_transmitted_bssid[4],
                   pst_mbssid->auc_transmitted_bssid[5]);
    OAM_ERROR_LOG4(0, OAM_SF_UM, "{>>>>>>>::Non_Transmitted BSSID:%02X:%02X:XX:XX:%02X:%02X.}",
                   pst_mbssid->auc_non_transmitted_bssid[0],
                   pst_mbssid->auc_non_transmitted_bssid[1],
                   pst_mbssid->auc_non_transmitted_bssid[4],
                   pst_mbssid->auc_non_transmitted_bssid[5]);
}
#endif

oal_uint8 hmac_scan_proc_non_transmit_bssid(hmac_vap_stru *pst_hmac_vap,
        oal_uint8 *puc_ie_data, mac_multi_bssid_frame_info_stru *pst_mbss_frame_info,
        oal_uint16 us_left_frame_body_len,
        mac_ieee80211_frame_stru *pst_frame_header)
{
    oal_uint8 uc_mbssid_ie_len;
    oal_uint8 auc_non_transmitted_bssid[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_uint32 ul_ret;
    oal_int32 l_ret;

    if (OAL_SUCC !=  mac_ie_parse_multi_bssid_opt_subie(puc_ie_data,
                            pst_mbss_frame_info, &uc_mbssid_ie_len)) {
        return 0;
    }

    /* �����ǰʣ��֡��С�ڽ�����mbssid ie len ����  */
    if (uc_mbssid_ie_len == 0 || uc_mbssid_ie_len > us_left_frame_body_len) {
        return 0;
    }

    /* ����transimitted bssid  */
    l_ret = memcpy_s(pst_mbss_frame_info->auc_transmitted_bssid, WLAN_MAC_ADDR_LEN,
                 pst_frame_header->auc_address2, WLAN_MAC_ADDR_LEN);

    /* ����non-transmitted bssid */
    ul_ret = hmac_scan_get_non_transmitted_bssid(pst_mbss_frame_info->auc_transmitted_bssid,
                                             sizeof(pst_mbss_frame_info->auc_transmitted_bssid),
                                             pst_mbss_frame_info->uc_maxbssid_indicator,
                                             pst_mbss_frame_info->uc_bssid_index,
                                             auc_non_transmitted_bssid,
                                             sizeof(auc_non_transmitted_bssid));
    if (ul_ret != OAL_SUCC) {
        return 0;
    }

    /* non_transmitted bssid */
    l_ret += memcpy_s(pst_mbss_frame_info->auc_non_transmitted_bssid, WLAN_MAC_ADDR_LEN,
                  auc_non_transmitted_bssid, WLAN_MAC_ADDR_LEN);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_proc_multi_bssid_scanned_bss::memcpy fail!");
        return 0;
    }

    pst_mbss_frame_info->bit_is_non_transimitted_bss = OAL_TRUE;

    return uc_mbssid_ie_len;
}


oal_uint32 hmac_scan_proc_multi_bssid_scanned_bss(hmac_vap_stru *pst_hmac_vap,
                                                  oal_netbuf_stru *pst_netbuf,
                                                  oal_uint16 us_buf_len)
{
    oal_uint16 us_mgmt_len;
    oal_uint8 *puc_mgmt_frame;
    mac_ieee80211_frame_stru *pst_frame_header;
    oal_uint8 *puc_ie_data = OAL_PTR_NULL;
    oal_uint8 *puc_frame_body;
    oal_uint16 us_frame_body_len;
    oal_uint16 us_left_frame_body_len;
    mac_multi_bssid_frame_info_stru st_mbss_frame_info;
    oal_uint8 *puc_mbssid_ie = OAL_PTR_NULL;
    oal_netbuf_stru *pst_new_netbuf = OAL_PTR_NULL;
    oal_uint8 uc_mbssid_ie_len;
    oal_uint8 uc_mgmt_filed_len;
    oal_uint16 us_new_buf_len = 0;
    oal_uint8 uc_multi_bssid_ie_len;

    /* ��dmac�ϱ���netbuf���ݽ��н���������������ʾ */
    /***********************************************************************************************/
    /*            netbuf data����ϱ���ɨ�������ֶεķֲ�                                        */
    /* ------------------------------------------------------------------------------------------  */
    /* beacon/probe rsp body  |     ֡����渽���ֶ�(mac_scanned_result_extend_info_stru)          */
    /* -----------------------------------------------------------------------------------------   */
    /* �յ���beacon/rsp��body | rssi(4�ֽ�) | channel num(1�ֽ�)| band(1�ֽ�)|bss_tye(1�ֽ�)|���  */
    /* ------------------------------------------------------------------------------------------  */
    /*                                                                                             */
    /***********************************************************************************************/
    /* ����֡�ĳ��ȵ����ϱ���netbuf�ĳ��ȼ�ȥ�ϱ���ɨ��������չ�ֶεĳ��� */
    us_mgmt_len    = us_buf_len - OAL_SIZEOF(mac_scanned_result_extend_info_stru);
    puc_mgmt_frame = (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf);

    /* ��ȡ����֡��֡ͷ��֡��ָ�� */
    pst_frame_header = (mac_ieee80211_frame_stru *)puc_mgmt_frame;
    puc_frame_body = puc_mgmt_frame + MAC_80211_FRAME_LEN;
    us_frame_body_len = us_mgmt_len - MAC_80211_FRAME_LEN;
    uc_mgmt_filed_len = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (us_frame_body_len <= uc_mgmt_filed_len) {
        return OAL_FAIL;
    }

    /* ie ��ʼλ�� */
    puc_ie_data = puc_frame_body + uc_mgmt_filed_len;
    us_left_frame_body_len = us_frame_body_len - uc_mgmt_filed_len;

    if (us_left_frame_body_len < MAC_MULTIPLE_BSSID_IE_MIN_LEN) {
        return OAL_FAIL;
    }

    /* ����multi-bssid ie */
    puc_mbssid_ie = mac_find_ie_etc(MAC_EID_MULTI_BSSID, puc_ie_data, us_left_frame_body_len);
    if (puc_mbssid_ie == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    memset_s(&st_mbss_frame_info, OAL_SIZEOF(mac_multi_bssid_frame_info_stru),
        0, OAL_SIZEOF(mac_multi_bssid_frame_info_stru));

    /* ����multi-bssid ie len */
    uc_multi_bssid_ie_len                    = (oal_uint8)*(puc_mbssid_ie + 1);
    puc_ie_data                              = puc_mbssid_ie + MAC_IE_HDR_LEN;

    /* ���� MaxBssid Indicator */
    st_mbss_frame_info.uc_maxbssid_indicator = (oal_uint8)*puc_ie_data;
    puc_ie_data                             += 1;
    us_left_frame_body_len                   = uc_multi_bssid_ie_len -1;

    /* �����Ƿ������Ч��Multi BSSID IE */
    while (us_left_frame_body_len > MAC_MULTIPLE_BSSID_IE_MIN_LEN) {
        uc_mbssid_ie_len = hmac_scan_proc_non_transmit_bssid(pst_hmac_vap,puc_ie_data,
            &st_mbss_frame_info, us_left_frame_body_len,pst_frame_header);
        if (uc_mbssid_ie_len == 0) {
            return OAL_FAIL;
        }
        pst_new_netbuf = hmac_scan_proc_non_transmitted_ssid_handle(pst_netbuf, us_buf_len,
                                                                    &st_mbss_frame_info, &us_new_buf_len);
        if (pst_new_netbuf == OAL_PTR_NULL) {
            return OAL_FAIL;
        }
        hmac_scan_proc_scan_result_handle(pst_hmac_vap, pst_new_netbuf, us_new_buf_len, &st_mbss_frame_info);

        us_left_frame_body_len  -= uc_mbssid_ie_len;
        puc_ie_data             += uc_mbssid_ie_len;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hmac_scan_proc_scanned_bss_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_netbuf_stru *pst_bss_mgmt_netbuf = OAL_PTR_NULL;
    dmac_tx_event_stru *pst_dtx_event = OAL_PTR_NULL;
    mac_multi_bssid_frame_info_stru st_mbss_frame_info;

    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_dtx_event = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_bss_mgmt_netbuf = pst_dtx_event->pst_netbuf;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss_etc::pst_hmac_vap null.}");

        /* �ͷ��ϱ���bss��Ϣ��beacon����probe rsp֡���ڴ� */
        oal_netbuf_free(pst_bss_mgmt_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (IS_CUSTOM_OPEN_MULTI_BSSID_SWITCH(&pst_hmac_vap->st_vap_base_info)) {
        hmac_scan_proc_multi_bssid_scanned_bss(pst_hmac_vap, pst_bss_mgmt_netbuf, pst_dtx_event->us_frame_len);
    }
#endif

    /* ��������Beacon ��Probe Rsp��bss */
    memset_s((oal_uint8 *)&st_mbss_frame_info, OAL_SIZEOF(st_mbss_frame_info), 0, OAL_SIZEOF(st_mbss_frame_info));
    hmac_scan_proc_scan_result_handle(pst_hmac_vap, pst_bss_mgmt_netbuf,
                                      pst_dtx_event->us_frame_len, &st_mbss_frame_info);

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_scan_print_channel_statistics_info(hmac_scan_record_stru *pst_scan_record)
{
    wlan_scan_chan_stats_stru *pst_chan_stats = pst_scan_record->ast_chan_results;
    oal_uint8 uc_vap_id = pst_scan_record->uc_vap_id;
    oal_uint8 uc_idx = 0;

    /* ��Ȿ��ɨ���Ƿ������ŵ����������û��ֱ�ӷ��� */
    if (0 == pst_chan_stats[0].uc_stats_valid) {
        OAM_INFO_LOG0(uc_vap_id, OAM_SF_SCAN,
                      "{hmac_scan_print_channel_statistics_info:: curr scan don't enable channel measure.\n}");
        return;
    }

    /* ��ӡ�ŵ�������� */
    OAM_INFO_LOG0(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info:: The chan measure result: \n}");

    for (uc_idx = 0; uc_idx < pst_scan_record->uc_chan_numbers; uc_idx++) {
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Chan num      : %d\n}",
                      pst_chan_stats[uc_idx].uc_channel_number);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Stats cnt     : %d\n}",
                      pst_chan_stats[uc_idx].uc_stats_cnt);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Stats valid   : %d\n}",
                      pst_chan_stats[uc_idx].uc_stats_valid);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Stats time us : %d\n}",
                      pst_chan_stats[uc_idx].ul_total_stats_time_us);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free time 20M : %d\n}",
                      pst_chan_stats[uc_idx].ul_total_free_time_20M_us);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free time 40M : %d\n}",
                      pst_chan_stats[uc_idx].ul_total_free_time_40M_us);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free time 80M : %d\n}",
                      pst_chan_stats[uc_idx].ul_total_free_time_80M_us);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Send time     : %d\n}",
                      pst_chan_stats[uc_idx].ul_total_send_time_us);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Recv time     : %d\n}",
                      pst_chan_stats[uc_idx].ul_total_recv_time_us);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free power cnt: %d\n}",
                      pst_chan_stats[uc_idx].uc_free_power_cnt);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free power 20M: %d\n}",
                      (oal_int32)pst_chan_stats[uc_idx].s_free_power_stats_20M);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free power 40M: %d\n}",
                      (oal_int32)pst_chan_stats[uc_idx].s_free_power_stats_40M);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Free power 80M: %d\n}",
                      (oal_int32)pst_chan_stats[uc_idx].s_free_power_stats_80M);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Radar detected: %d\n}",
                      pst_chan_stats[uc_idx].uc_radar_detected);
        OAM_INFO_LOG1(uc_vap_id, OAM_SF_SCAN, "{hmac_scan_print_channel_statistics_info::Radar bw      : %d\n}",
                      pst_chan_stats[uc_idx].uc_radar_bw);
    }

    return;
}


OAL_STATIC oal_void hmac_scan_print_scan_record_info(hmac_vap_stru *pst_hmac_vap,
                                                     hmac_scan_record_stru *pst_scan_record)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (defined(_PRE_PRODUCT_ID_HI110X_HOST))
    oal_time_t_stru st_timestamp_diff;

    /* ��ȡɨ����ʱ��� */
    st_timestamp_diff = oal_ktime_sub(oal_ktime_get(), pst_scan_record->st_scan_start_time);

    /* �����ں˽ӿڣ���ӡ�˴�ɨ���ʱ */
    OAM_WARNING_LOG4(pst_scan_record->uc_vap_id, OAM_SF_SCAN,
        "{hmac_scan_print_scan_record_info::scan comp,scan_status:%d,vap ch_num:%d,cookie:%x, duration time:%lums.}",
        pst_scan_record->en_scan_rsp_status,
        pst_hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
        pst_scan_record->ull_cookie,
        ktime_to_ms(st_timestamp_diff));
#endif

    /* ��ӡɨ�赽��bss��Ϣ */
    hmac_scan_print_scanned_bss_info_etc(pst_scan_record->uc_device_id);

    /* �ŵ�������� */
    hmac_scan_print_channel_statistics_info(pst_scan_record);

    return;
}


oal_uint32 hmac_scan_proc_scan_comp_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    mac_scan_rsp_stru *pst_d2h_scan_rsp_info = OAL_PTR_NULL;
    hmac_scan_stru *pst_scan_mgmt = OAL_PTR_NULL;
    oal_bool_enum_uint8 uc_scan_abort_sync_state = OAL_FALSE;

    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_event_hdr->uc_device_id);
    if (OAL_UNLIKELY(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event_etc::pst_hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_d2h_scan_rsp_info = (mac_scan_rsp_stru *)(pst_event->auc_event_data);
    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    if ((pst_event_hdr->uc_vap_id != pst_scan_mgmt->st_scan_record_mgmt.uc_vap_id) ||
        (pst_d2h_scan_rsp_info->ull_cookie != pst_scan_mgmt->st_scan_record_mgmt.ull_cookie)) {
        OAM_WARNING_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_comp_event_etc::Report vap:%d Scan_rsp(cookie %d), another vap:%d scan(cookie %d)!}",
            pst_event_hdr->uc_vap_id,
            pst_d2h_scan_rsp_info->ull_cookie,
            pst_scan_mgmt->st_scan_record_mgmt.uc_vap_id,
            pst_scan_mgmt->st_scan_record_mgmt.ull_cookie);
        return OAL_SUCC;
    }

    OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event_etc::scan status:%d !}",
                     pst_d2h_scan_rsp_info->en_scan_rsp_status);

    /* ɾ��ɨ�賬ʱ������ʱ�� */
    if ((pst_scan_mgmt->st_scan_timeout.en_is_registerd == OAL_TRUE) &&
        (pst_d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO)) {
        /* PNOû������ɨ�趨ʱ��,���ǵ�ȡ��PNOɨ��,�����·���ͨɨ��,PNOɨ������¼���������ͨɨ���Ӱ�� */
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_scan_mgmt->st_scan_timeout));
    }

    /* ��ȡhmac vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_comp_event_etc::pst_hmac_vap null.}");

        /* ���õ�ǰ���ڷ�ɨ��״̬ */
        pst_scan_mgmt->en_is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���ݵ�ǰɨ������ͺ͵�ǰvap��״̬�������л�vap��״̬�������ǰ��ɨ�裬����Ҫ�л�vap��״̬ */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (pst_d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO)) {
        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* �ı�vap״̬��SCAN_COMP */
            hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (pst_scan_mgmt->st_scan_record_mgmt.en_scan_rsp_status == MAC_SCAN_ABORT_SYNC)) {
        uc_scan_abort_sync_state = OAL_TRUE;
    }

    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (pst_scan_mgmt->st_scan_record_mgmt.en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        hmac_fsm_change_state_etc(pst_hmac_vap, pst_scan_mgmt->st_scan_record_mgmt.en_vap_last_state);
        pst_scan_mgmt->st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* ����device�ϱ���ɨ�������ϱ�sme */
    /* ��ɨ��ִ�����(ɨ��ִ�гɹ�������ʧ�ܵȷ��ؽ��)��¼��ɨ�����м�¼�ṹ���� */
    pst_scan_mgmt->st_scan_record_mgmt.en_scan_rsp_status = pst_d2h_scan_rsp_info->en_scan_rsp_status;
    pst_scan_mgmt->st_scan_record_mgmt.ull_cookie = pst_d2h_scan_rsp_info->ull_cookie;

    hmac_scan_print_scan_record_info(pst_hmac_vap, &(pst_scan_mgmt->st_scan_record_mgmt));

    if (pst_scan_mgmt->st_scan_record_mgmt.p_fn_cb != OAL_PTR_NULL) {
        /* ��ֹɨ��������ûص�,��ֹ��ֹɨ������������PNOɨ���Ӱ�� */
        pst_scan_mgmt->st_scan_record_mgmt.p_fn_cb(&(pst_scan_mgmt->st_scan_record_mgmt));
    } else {
        
        if (uc_scan_abort_sync_state == OAL_TRUE) {
            hmac_cfg80211_scan_comp(&(pst_scan_mgmt->st_scan_record_mgmt));
        }
    }

    /* ���õ�ǰ���ڷ�ɨ��״̬ */
    if (pst_d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO) {
        /* PNOɨ��û���ô�λΪOAL_TRUE,PNOɨ�����,����Ӱ�����ĳ���ɨ�� */
        pst_scan_mgmt->en_is_scanning = OAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        hmac_p2p_listen_timeout_etc(pst_hmac_vap, &pst_hmac_vap->st_vap_base_info);
    }

    if (pst_hmac_vap->en_wait_roc_end == OAL_TRUE) {
        
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_comp_event_etc::scan rsp status[%d]}",
                         pst_d2h_scan_rsp_info->en_scan_rsp_status);
        OAL_COMPLETE(&(pst_hmac_vap->st_roc_end_ready));
        pst_hmac_vap->en_wait_roc_end = OAL_FALSE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* STA����ɨ��ʱ����Ҫ��ǰʶ�����γ��� */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) {
        hmac_roam_check_bkscan_result_etc(pst_hmac_vap, &(pst_scan_mgmt->st_scan_record_mgmt));
    }
#endif  // _PRE_WLAN_FEATURE_ROAM

    return OAL_SUCC;
}

oal_uint32 hmac_scan_proc_scan_req_event_exception_etc(hmac_vap_stru *pst_hmac_vap, oal_void *p_params)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    hmac_scan_rsp_stru st_scan_rsp;
    hmac_scan_rsp_stru *pst_scan_rsp = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_hmac_vap, p_params))) {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_exception_etc::param null, %d %d.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)p_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��֧�ַ���ɨ���״̬������ɨ�� */
    OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                     "{hmac_scan_proc_scan_req_event_exception_etc::vap state is=%x.}",
                     pst_hmac_vap->st_vap_base_info.en_vap_state);

    memset_s(&st_scan_rsp, OAL_SIZEOF(hmac_scan_rsp_stru), 0, OAL_SIZEOF(hmac_scan_rsp_stru));

    /* ��ɨ������¼���WAL, ִ��SCAN_DONE , �ͷ�ɨ�������ڴ� */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(hmac_scan_rsp_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_req_event_exception_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_scan_rsp.uc_result_code = MAC_SCAN_REFUSED;
#ifdef _PRE_WLAN_FEATURE_ROAM
    /* When STA is roaming, scan req return success instead of failure,
       in case roaming failure which will cause UI scan list null  */
    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        st_scan_rsp.uc_result_code = MAC_SCAN_SUCCESS;
    }
#endif
    st_scan_rsp.uc_num_dscr = 0;

    /* ��д�¼� */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA,
                       OAL_SIZEOF(hmac_scan_rsp_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    pst_scan_rsp = (hmac_scan_rsp_stru *)pst_event->auc_event_data;

    if (EOK != memcpy_s(pst_scan_rsp, OAL_SIZEOF(hmac_scan_rsp_stru),
                        (oal_void *)(&st_scan_rsp), OAL_SIZEOF(hmac_scan_rsp_stru))) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_proc_scan_req_event_exception_etc::memcpy fail!");
        FRW_EVENT_FREE(pst_event_mem);
        return OAL_FAIL;
    }

    /* �ַ��¼� */
    frw_event_dispatch_event_etc(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


oal_void hmac_scan_set_sour_mac_addr_in_probe_req_etc(hmac_vap_stru *pst_hmac_vap,
                                                      oal_uint8 *puc_sour_mac_addr,
                                                      oal_bool_enum_uint8 en_is_rand_mac_addr_scan,
                                                      oal_bool_enum_uint8 en_is_p2p0_scan)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;

    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, puc_sour_mac_addr)) {
        OAM_ERROR_LOG2(0, OAM_SF_CFG,
            "{hmac_scan_set_sour_mac_addr_in_probe_req_etc::param null,pst_hmac_vap:%p,puc_sour_mac_addr:%p.}",
            (uintptr_t)pst_hmac_vap, (uintptr_t)puc_sour_mac_addr);
        return;
    }

    pst_mac_device = mac_res_get_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hmac_scan_set_sour_mac_addr_in_probe_req_etc::pst_mac_device is null.}");
        return;
    }

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_CFG,
                         "{hmac_scan_set_sour_mac_addr_in_probe_req_etc::pst_hmac_device is null. device_id %d}",
                         pst_hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* WLAN/P2P ��������£�p2p0 ��p2p-p2p0 cl ɨ��ʱ����Ҫʹ�ò�ͬ�豸 */
    if (en_is_p2p0_scan == OAL_TRUE) {
        oal_set_mac_addr(puc_sour_mac_addr, mac_mib_get_p2p0_dot11StationID(&pst_hmac_vap->st_vap_base_info));
    } else
#endif /* _PRE_WLAN_FEATURE_P2P */
    {
        /* ������mac addrɨ�����Կ����ҷ�P2P�������������mac addr��probe req֡�� */
        if ((en_is_rand_mac_addr_scan == OAL_TRUE) && (IS_LEGACY_VAP(&(pst_hmac_vap->st_vap_base_info))) &&
            ((pst_mac_device->auc_mac_oui[0] != 0) || (pst_mac_device->auc_mac_oui[1] != 0) ||
             (pst_mac_device->auc_mac_oui[2] != 0))) {
            /* �������mac ��ַ,ʹ���·����MAC OUI ���ɵ����mac ��ַ���µ�����ɨ�� */
            oal_set_mac_addr(puc_sour_mac_addr, pst_hmac_device->st_scan_mgmt.auc_random_mac);
        } else {
            /* ���õ�ַΪ�Լ���MAC��ַ */
            oal_set_mac_addr(puc_sour_mac_addr, mac_mib_get_StationID(&pst_hmac_vap->st_vap_base_info));
        }
    }

    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_scan_need_skip_channel(hmac_vap_stru *pst_hmac_vap, oal_uint8 uc_channel)
{
    wlan_channel_band_enum_uint8 en_band = mac_get_band_by_channel_num(uc_channel);

    if (OAL_TRUE != mac_chip_run_band(pst_hmac_vap->st_vap_base_info.uc_chip_id, en_band)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC oal_void hmac_scan_set_neighbor_report_scan_params(mac_scan_req_stru *pst_scan_params)
{
    if (pst_scan_params->uc_neighbor_report_process_flag == OAL_TRUE) {
        pst_scan_params->uc_scan_channel_interval = 2;
        pst_scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;
    }
    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_scan_get_random_mac_flag(hmac_vap_stru *pst_hmac_vap,
                                                             oal_bool_enum_uint8 en_customize_random_mac_scan)
{
    /* ��P2P�豸���������macɨ���־��device�����·���DMAC */
    return (IS_LEGACY_VAP(&(pst_hmac_vap->st_vap_base_info))) ? en_customize_random_mac_scan : OAL_FALSE;
}


OAL_STATIC oal_uint32 hmac_scan_update_scan_params(hmac_vap_stru *pst_hmac_vap,
                                                   mac_scan_req_stru *pst_scan_params,
                                                   oal_bool_enum_uint8 en_is_random_mac_addr_scan)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap_temp = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    wlan_vap_mode_enum_uint8 en_vap_mode;
    oal_uint8 uc_loop;
    oal_uint8 uc_chan_cnt = 0;

    /* ��ȡmac device */
    pst_mac_device = mac_res_get_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_mac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_device_id, OAM_SF_SCAN,
                         "{hmac_scan_update_scan_params::pst_mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    /* 1.��¼����ɨ���vap id��ɨ����� */
    pst_scan_params->uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;
    pst_scan_params->en_need_switch_back_home_channel = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (pst_scan_params->en_scan_mode != WLAN_SCAN_MODE_ROAM_SCAN)
#endif
    {
        pst_scan_params->en_scan_mode = WLAN_SCAN_MODE_FOREGROUND;
    }

    /* 2.�޸�ɨ��ģʽ���ŵ�ɨ�����: �����Ƿ����up״̬�µ�vap������ǣ����Ǳ���ɨ�裬������ǣ�����ǰ��ɨ�� */
    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap_temp);
    if ((ul_ret == OAL_SUCC) && (pst_mac_vap_temp != OAL_PTR_NULL)) {
        /* �ж�vap�����ͣ������sta��Ϊsta�ı���ɨ�裬�����ap������ap�ı���ɨ�裬�������͵�vap�ݲ�֧�ֱ���ɨ�� */
        en_vap_mode = pst_hmac_vap->st_vap_base_info.en_vap_mode;
        if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
#ifdef _PRE_WLAN_FEATURE_ROAM
            if (pst_scan_params->en_scan_mode != WLAN_SCAN_MODE_ROAM_SCAN)
#endif
            {
                /* �޸�ɨ�����Ϊsta�ı���ɨ�� */
                pst_scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;
            }
        } else if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* �޸�ɨ�����Ϊsta�ı���ɨ�� */
            pst_scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_AP;
        } else {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_scan_params::vap mode[%d], not support bg scan.}",
                           en_vap_mode);
            return OAL_FAIL;
        }
        pst_scan_params->en_need_switch_back_home_channel = OAL_TRUE;

        if (1 == mac_device_calc_up_vap_num_etc(pst_mac_device) &&
            !IS_LEGACY_VAP(pst_mac_vap_temp) &&
            IS_LEGACY_VAP(&pst_hmac_vap->st_vap_base_info)) {
            /*
             * �޸�ɨ���ŵ����(2)�ͻع����ŵ�����ʱ��(60ms):�������P2P���ڹ���״̬��
             * wlan����ȥ����״̬,wlan�����ɨ��
             */
            pst_scan_params->uc_scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE;
            pst_scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;

            if ((pst_scan_params->us_scan_time > WLAN_DEFAULT_ACTIVE_SCAN_TIME) &&
                (pst_scan_params->en_scan_type == WLAN_SCAN_TYPE_ACTIVE)) {
                /* ָ��SSIDɨ�賬��3��,���޸�ÿ��ɨ��ʱ��Ϊ40ms(Ĭ����20ms) */
                /* P2P������wlanδ��������,���ǵ�ɨ��ʱ�����Ӷ�p2p wfd������Ӱ��,����ÿ�ŵ�ɨ�����Ϊ1��(Ĭ��Ϊ2��) */
                pst_scan_params->uc_max_scan_count_per_channel = 1;
            }
        } else {
            /* Я������SSID�������ɨ3���ŵ���һ��home�ŵ����������Ĭ��ɨ��6���ŵ���home�ŵ�����100ms */
            pst_scan_params->uc_scan_channel_interval = (pst_scan_params->uc_ssid_num > 1) ?
                MAC_SCAN_CHANNEL_INTERVAL_HIDDEN_SSID : MAC_SCAN_CHANNEL_INTERVAL_DEFAULT;
            pst_scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
        }
        hmac_scan_set_neighbor_report_scan_params(pst_scan_params);
    }

    /* 3.���÷��͵�probe req֡��Դmac addr */
    pst_scan_params->en_is_random_mac_addr_scan =
        hmac_scan_get_random_mac_flag(pst_hmac_vap, en_is_random_mac_addr_scan);
    hmac_scan_set_sour_mac_addr_in_probe_req_etc(pst_hmac_vap, pst_scan_params->auc_sour_mac_addr,
                                                 en_is_random_mac_addr_scan, pst_scan_params->bit_is_p2p0_scan);

    for (uc_loop = 0; uc_loop < pst_scan_params->uc_channel_nums; uc_loop++) {
        if (!hmac_scan_need_skip_channel(pst_hmac_vap, pst_scan_params->ast_channel_list[uc_loop].uc_chan_number)) {
            if (uc_chan_cnt != uc_loop) {
                pst_scan_params->ast_channel_list[uc_chan_cnt] = pst_scan_params->ast_channel_list[uc_loop];
            }
            uc_chan_cnt++;
        }
    }

    if (!uc_chan_cnt) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_scan_update_scan_params::channels trimed to none!, ori cnt=%d}",
                         pst_scan_params->uc_channel_nums);

        return OAL_FAIL;
    }

    pst_scan_params->uc_channel_nums = uc_chan_cnt;
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_scan_check_can_enter_scan_state(mac_vap_stru *pst_mac_vap)
{
    /* p2p�п��ܽ��м������������Ǻ�scan req�����ȼ�һ������˵��ϲ㷢�����ɨ������ʱ��ͳһ��ʹ������Ľӿ��ж� */
    return hmac_p2p_check_can_enter_state_etc(pst_mac_vap, HMAC_FSM_INPUT_SCAN_REQ);
}


OAL_STATIC oal_uint32 hmac_scan_check_is_dispatch_scan_req(hmac_vap_stru *pst_hmac_vap,
                                                           hmac_device_stru *pst_hmac_device)
{
    oal_uint32 ul_ret;

    /* 1.�ȼ������vap��״̬�Ӷ��ж��Ƿ�ɽ���ɨ��״̬��ʹ��ɨ�辡��������������������� */
    ul_ret = hmac_scan_check_can_enter_scan_state(&(pst_hmac_vap->st_vap_base_info));
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_check_is_dispatch_scan_req::Because of err_code[%d], can't enter into scan state.}", ul_ret);
        return ul_ret;
    }

    /* 2.�жϵ�ǰɨ���Ƿ�����ִ�� */
    if (pst_hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_check_is_dispatch_scan_req::the scan request is rejected.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 3.�жϵ�ǰ�Ƿ�����ִ������ */
    if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_check_is_dispatch_scan_req:: roam reject new scan.}");
        return OAL_FAIL;
    }
#endif  // _PRE_WLAN_FEATURE_ROAM

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_scan_proc_last_scan_record(hmac_vap_stru *pst_hmac_vap,
                                                    hmac_device_stru *pst_hmac_device)
{
    OAM_INFO_LOG0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_etc:: start clean last scan record.}");

    /* ����ɨ��������ʱ�������һ��ɨ�����й��ڵ�bss��Ϣ */
    hmac_scan_clean_expire_scanned_bss(pst_hmac_vap, &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt));

    return;
}


OAL_STATIC oal_uint32 hmac_scan_proc_scan_timeout_fn(void *p_arg)
{
    hmac_device_stru *pst_hmac_device = (hmac_device_stru *)p_arg;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_scan_record_stru *pst_scan_record = OAL_PTR_NULL;
    oal_uint32 ul_pedding_data = 0;

    /* ��ȡɨ���¼��Ϣ */
    pst_scan_record = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    /* ��ȡhmac vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_scan_record->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_scan_record->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_timeout_fn::pst_hmac_vap null.}");

        /* ɨ��״̬�ָ�Ϊδ��ִ�е�״̬ */
        pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���ݵ�ǰɨ������ͺ͵�ǰvap��״̬�������л�vap��״̬�������ǰ��ɨ�裬����Ҫ�л�vap��״̬ */
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* �ı�vap״̬��SCAN_COMP */
            hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (pst_scan_record->en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        hmac_fsm_change_state_etc(pst_hmac_vap, pst_scan_record->en_vap_last_state);
        pst_scan_record->en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* ����ɨ����Ӧ״̬Ϊ��ʱ */
    pst_scan_record->en_scan_rsp_status = MAC_SCAN_TIMEOUT;
    OAM_WARNING_LOG1(pst_scan_record->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_scan_proc_scan_timeout_fn::scan time out cookie [%x].}",
                     pst_scan_record->ull_cookie);

    /* ���ɨ��ص�������Ϊ�գ�����ûص����� */
    if (pst_scan_record->p_fn_cb != OAL_PTR_NULL) {
        OAM_WARNING_LOG0(pst_scan_record->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_timeout_fn::scan callback func proc.}");
        pst_scan_record->p_fn_cb(pst_scan_record);
    }

    /* DMAC ��ʱδ�ϱ�ɨ����ɣ�HMAC �·�ɨ��������ֹͣDMAC ɨ�� */
    hmac_config_scan_abort_etc(&pst_hmac_vap->st_vap_base_info, OAL_SIZEOF(oal_uint32), (oal_uint8 *)&ul_pedding_data);

    /* ɨ��״̬�ָ�Ϊδ��ִ�е�״̬ */
    pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;

    CHR_EXCEPTION(CHR_WIFI_DRV(CHR_WIFI_DRV_EVENT_SCAN, CHR_WIFI_DRV_ERROR_SCAN_TIMEOUT));

    return OAL_SUCC;
}


oal_uint32 hmac_scan_proc_scan_req_event_etc(hmac_vap_stru *pst_hmac_vap, oal_void *p_params)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    mac_scan_req_h2d_stru *pst_h2d_scan_req_params = OAL_PTR_NULL; /* hmac���͵�dmac��ɨ��������� */
    mac_scan_req_stru *pst_scan_params = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_scan_record_stru *pst_scan_record = OAL_PTR_NULL;
    oal_uint32 ul_scan_timeout;
    oal_uint32 ul_ret;
    oal_netbuf_stru *pst_netbuf_scan_req = OAL_PTR_NULL;
    dmac_tx_event_stru *pst_scan_req_event = OAL_PTR_NULL;
    oal_uint8 *puc_netbuf_data = OAL_PTR_NULL;
    oal_int32 l_ret;

    /* �����Ϸ��Լ�� */
    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_hmac_vap, p_params))) {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_etc::param null, %p %p.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)p_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ɨ��ֹͣģ����� */
    if (((g_en_bgscan_enable_flag_etc == HMAC_BGSCAN_DISABLE) &&
         (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) ||
        (g_en_bgscan_enable_flag_etc == HMAC_SCAN_DISABLE)) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "hmac_scan_proc_scan_req_event_etc: g_en_bgscan_enable_flag_etc= %d.",
                         g_en_bgscan_enable_flag_etc);
        return OAL_FAIL;
    }

    pst_h2d_scan_req_params = (mac_scan_req_h2d_stru *)p_params;
    pst_scan_params = &(pst_h2d_scan_req_params->st_scan_params);

    /* �쳣�ж�: ɨ����ŵ�����Ϊ0 */
    if (pst_scan_params->uc_channel_nums == 0) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event_etc::channel_nums=0.}");
        return OAL_FAIL;
    }

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event_etc::pst_hmac_device[%d] null.}",
                         pst_hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* ���´˴�ɨ�������ɨ����� */
    if (pst_scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
        ul_ret = hmac_scan_update_scan_params(pst_hmac_vap, pst_scan_params, OAL_FALSE);
    } else {
        /* ���´˴�ɨ�������ɨ����� */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        ul_ret = hmac_scan_update_scan_params(pst_hmac_vap, pst_scan_params,
                                              wlan_customize_etc.uc_random_mac_addr_scan);
#else
        ul_ret = hmac_scan_update_scan_params(pst_hmac_vap, pst_scan_params,
                                              pst_hmac_device->st_scan_mgmt.en_is_random_mac_addr_scan);
#endif
    }

    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event_etc::update scan mode failed, error[%d].}", ul_ret);
        return ul_ret;
    }

    /* ����Ƿ���Ϸ���ɨ���������������������ϣ�ֱ�ӷ��� */
    ul_ret = hmac_scan_check_is_dispatch_scan_req(pst_hmac_vap, pst_hmac_device);
    if (ul_ret != OAL_SUCC) {
        if (pst_scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
            hmac_vap_state_restore_etc(pst_hmac_vap);
        }

        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event_etc::Because of error[%d], can't dispatch scan req.}", ul_ret);
        return ul_ret;
    }

    /* ����ɨ��ģ�鴦��ɨ��״̬������ɨ�����󽫶��� */
    pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_TRUE;

    /* ������һ��ɨ���¼��Ŀǰֱ�������һ�ν��������������Ҫ�ϻ�ʱ�䴦�� */
    hmac_scan_proc_last_scan_record(pst_hmac_vap, pst_hmac_device);

    /* ��¼ɨ�跢���ߵ���Ϣ��ĳЩģ��ص�����ʹ�� */
    pst_scan_record = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    pst_scan_record->uc_chip_id = pst_hmac_device->pst_device_base_info->uc_chip_id;
    pst_scan_record->uc_device_id = pst_hmac_device->pst_device_base_info->uc_device_id;
    pst_scan_record->uc_vap_id = pst_scan_params->uc_vap_id;
    pst_scan_record->uc_chan_numbers = pst_scan_params->uc_channel_nums;
    pst_scan_record->p_fn_cb = pst_scan_params->p_fn_cb;

    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event_etc::save last en_vap_state:%d}",
                         pst_hmac_vap->st_vap_base_info.en_vap_state);

        pst_scan_record->en_vap_last_state = pst_hmac_vap->st_vap_base_info.en_vap_state;
    }

    pst_scan_record->ull_cookie = pst_scan_params->ull_cookie;

    /* ��¼ɨ�迪ʼʱ�� */
    pst_scan_record->st_scan_start_time = oal_ktime_get();

    /* ����netbuf�ڴ�  */
    pst_netbuf_scan_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                               (OAL_SIZEOF(mac_scan_req_h2d_stru)),
                                               OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf_scan_req == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_req_event_etc::pst_netbuf_app_ie(%u) alloc failed.}",
                       OAL_SIZEOF(mac_scan_req_h2d_stru));
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* ��ɨ�������¼���DMAC, ����event�¼��ڴ� */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (pst_event_mem == OAL_PTR_NULL) {
        oal_netbuf_free(pst_netbuf_scan_req);
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_req_event_etc::alloc memory failed.}");
        /* �ָ�ɨ��״̬Ϊ������״̬ */
        pst_hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �������ɨ���vap��ģʽΪsta�����ң������״̬Ϊ��up״̬���ҷ�p2p����״̬�����л���ɨ��״̬ */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (MAC_SCAN_FUNC_P2P_LISTEN != pst_scan_params->uc_scan_func)) {
        if (pst_hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP) {
            /* �л�vap��״̬ΪWAIT_SCAN״̬ */
            hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_STA_WAIT_SCAN);
        }
    }

    /* AP������ɨ�������⴦������hostapd�·�ɨ������ʱ��VAP������INIT״̬ */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_INIT)) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event_etc::ap startup scan.}");
        hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_AP_WAIT_START);
    }

    /* ��д�¼� */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************** copy data **************************/
    memset_s(oal_netbuf_cb(pst_netbuf_scan_req), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);
    puc_netbuf_data = (oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf_scan_req));
    /* ����ɨ�����������netbuf data���� */
    l_ret = memcpy_s(puc_netbuf_data, OAL_SIZEOF(mac_scan_req_h2d_stru),
                     (oal_uint8 *)p_params, OAL_SIZEOF(mac_scan_req_stru));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "mac_set_rsn_ie_etc::memcpy fail!");
        oal_netbuf_free(pst_netbuf_scan_req);
        FRW_EVENT_FREE(pst_event_mem);
        return OAL_FAIL;
    }

    /* ����netbuf ���¼��������� */
    pst_scan_req_event = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_scan_req_event->pst_netbuf = pst_netbuf_scan_req;
    pst_scan_req_event->us_frame_len = OAL_SIZEOF(mac_scan_req_h2d_stru);
    pst_scan_req_event->us_remain = 0;

    /******************************************************************/
    /* ��ӡɨ�����������ʹ�� */
    /* �����P2P ���������������HMAC ɨ�賬ʱʱ��ΪP2P ����ʱ�� */
    ul_scan_timeout = (pst_scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) ?
        (pst_scan_params->us_scan_time * 2) : WLAN_DEFAULT_MAX_TIME_PER_SCAN;
    hmac_scan_print_scan_params(pst_scan_params, &pst_hmac_vap->st_vap_base_info);

    /* ����ɨ�豣����ʱ������ֹ���¼����˼�ͨ��ʧ�ܵ�����µ��쳣��������ʱ�������ĳ�ʱʱ��Ϊ4.5�� */
    FRW_TIMER_CREATE_TIMER(&(pst_hmac_device->st_scan_mgmt.st_scan_timeout),
                           hmac_scan_proc_scan_timeout_fn,
                           ul_scan_timeout,
                           pst_hmac_device,
                           OAL_FALSE,
                           OAM_MODULE_ID_HMAC,
                           pst_hmac_device->pst_device_base_info->ul_core_id);

    /* �ַ��¼� */
    frw_event_dispatch_event_etc(pst_event_mem);

    oal_netbuf_free(pst_netbuf_scan_req);
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 hmac_scan_proc_sched_scan_req_event_etc(hmac_vap_stru *pst_hmac_vap, oal_void *p_params)
{
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    frw_event_stru *pst_event = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_scan_record_stru *pst_scan_record = OAL_PTR_NULL;
    mac_pno_scan_stru *pst_pno_scan_params = OAL_PTR_NULL;
    oal_uint32 ul_ret;

    /* �����Ϸ��Լ�� */
    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_hmac_vap, p_params))) {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{hmac_scan_proc_sched_scan_req_event_etc::param null, %p %p.}",
                       (uintptr_t)pst_hmac_vap, (uintptr_t)p_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_pno_scan_params = (mac_pno_scan_stru *)p_params;

    /* �ж�PNO����ɨ���·��Ĺ��˵�ssid����С�ڵ���0 */
    if (pst_pno_scan_params->l_ssid_count <= 0) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_sched_scan_req_event_etc::ssid_count <=0.}");
        return OAL_FAIL;
    }

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_sched_scan_req_event_etc::pst_hmac_device[%d] null.}",
                         pst_hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* ����Ƿ���Ϸ���ɨ���������������������ϣ�ֱ�ӷ��� */
    ul_ret = hmac_scan_check_is_dispatch_scan_req(pst_hmac_vap, pst_hmac_device);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_sched_scan_req_event_etc::Because of error[%d], can't dispatch scan req.}", ul_ret);
        return ul_ret;
    }

    /* �����һ�ε�ɨ���� */
    hmac_scan_proc_last_scan_record(pst_hmac_vap, pst_hmac_device);

    /* ��¼ɨ�跢���ߵ���Ϣ��ĳЩģ��ص�����ʹ�� */
    pst_scan_record = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    pst_scan_record->uc_chip_id = pst_hmac_device->pst_device_base_info->uc_chip_id;
    pst_scan_record->uc_device_id = pst_hmac_device->pst_device_base_info->uc_device_id;
    pst_scan_record->uc_vap_id = pst_hmac_vap->st_vap_base_info.uc_vap_id;
    pst_scan_record->p_fn_cb = pst_pno_scan_params->p_fn_cb;

    /* ��ɨ�������¼���DMAC, �����¼��ڴ� */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(mac_pno_scan_stru *));
    if (pst_event_mem == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_sched_scan_req_event_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��д�¼� */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ,
                       OAL_SIZEOF(mac_pno_scan_stru *),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_vap->st_vap_base_info.uc_chip_id,
                       pst_hmac_vap->st_vap_base_info.uc_device_id,
                       pst_hmac_vap->st_vap_base_info.uc_vap_id);

    /* �¼�data����Я��PNOɨ��������� */
    if (EOK != memcpy_s(frw_get_event_payload(pst_event_mem), OAL_SIZEOF(mac_pno_scan_stru *),
                        (oal_uint8 *)&pst_pno_scan_params, OAL_SIZEOF(mac_pno_scan_stru *))) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_scan_proc_sched_scan_req_event_etc::memcpy fail!");
        FRW_EVENT_FREE(pst_event_mem);
        return OAL_FAIL;
    }

    /* �ַ��¼� */
    frw_event_dispatch_event_etc(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 hmac_scan_process_chan_result_event_etc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    dmac_crx_chan_result_stru *pst_chan_result_param = OAL_PTR_NULL;
    hmac_scan_record_stru *pst_scan_record = OAL_PTR_NULL;
    oal_uint8 uc_scan_idx;

    /* ��ȡ�¼���Ϣ */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_chan_result_param = (dmac_crx_chan_result_stru *)(pst_event->auc_event_data);
    uc_scan_idx = pst_chan_result_param->uc_scan_idx;

    /* ��ȡhmac device */
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_event_hdr->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{hmac_scan_process_chan_result_event_etc::pst_hmac_device is null.}");
        return OAL_FAIL;
    }

    pst_scan_record = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    /* ����ϱ��������Ƿ�Ϸ� */
    if (uc_scan_idx >= pst_scan_record->uc_chan_numbers) {
        /* dmac�ϱ���ɨ����������Ҫɨ����ŵ����� */
        OAM_WARNING_LOG2(0, OAM_SF_SCAN,
            "{hmac_scan_process_chan_result_event_etc::result from dmac error! scan_idx[%d], chan_numbers[%d].}",
            uc_scan_idx, pst_scan_record->uc_chan_numbers);

        return OAL_FAIL;
    }

    pst_scan_record->ast_chan_results[uc_scan_idx] = pst_chan_result_param->st_chan_result;

    return OAL_SUCC;
}
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)

oal_uint32 hmac_scan_rrm_proc_save_bss_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_netbuf_stru *pst_action_table_bcn_rpt;
    oal_uint8 *puc_data;
    mac_user_stru *pst_mac_user;
    oal_uint16 us_index;
    mac_meas_rpt_ie_stru *pst_meas_rpt_ie;
    mac_bcn_rpt_stru *pst_bcn_rpt;
    mac_tx_ctl_stru *pst_tx_ctl;
    mac_vap_rrm_trans_req_info_stru *pst_trans_req_info;
    oal_uint32 ul_ret;
    hmac_device_stru *pst_hmac_device;
    hmac_bss_mgmt_stru *pst_bss_mgmt;
    oal_dlist_head_stru *pst_entry;
    hmac_scanned_bss_info *pst_scanned_bss;
    oal_uint8 uc_bss_idx = 0;
    hmac_vap_stru *pst_hmac_vap;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    oal_uint8 uc_snr_ant0;
    oal_uint8 uc_snr_ant1;
    oal_uint8 uc_snr;
#endif
    oal_uint16 us_len;

    if (puc_param == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_scan_rrm_proc_save_bss_etc::puc_param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡhmac vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_rrm_proc_save_bss_etc::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_rrm_proc_save_bss_etc::pst_hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if (pst_mac_user == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{hmac_scan_rrm_proc_save_bss_etc::pst_mac_user[%d] null.",
                         pst_mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_trans_req_info = (mac_vap_rrm_trans_req_info_stru *)puc_param;

    pst_action_table_bcn_rpt = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                                       WLAN_MEM_NETBUF_SIZE2,
                                                                       OAL_NETBUF_PRIORITY_MID);
    if (pst_action_table_bcn_rpt == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_scan_rrm_proc_save_bss_etc::pst_action_table_bcn_rpt null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(pst_action_table_bcn_rpt), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    puc_data = (oal_uint8 *)OAL_NETBUF_HEADER(pst_action_table_bcn_rpt);

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    if (pst_hmac_vap->pst_rrm_info == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_rrm_proc_save_bss_etc::pst_rrm_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
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
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(puc_data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    puc_data[2] = 0;
    puc_data[3] = 0;

    /* DA is address of STA requesting association */
    oal_set_mac_addr(puc_data + 4, pst_mac_user->auc_user_mac_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(puc_data + 10, mac_mib_get_StationID(pst_mac_vap));

    oal_set_mac_addr(puc_data + 16, pst_mac_vap->auc_bssid);

    /* seq control */
    puc_data[22] = 0;
    puc_data[23] = 0;

    /*************************************************************************/
    /*                    Radio Measurement Report Frame - Frame Body        */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Measurement Report Elements         | */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          |  var                                 */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* Initialize index and the frame data pointer */
    us_index = MAC_80211_FRAME_LEN;

    /* Category */
    puc_data[us_index++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    puc_data[us_index++] = MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT;

    /* Dialog Token  */
    puc_data[us_index++] = pst_trans_req_info->uc_action_dialog_token;

    us_len = us_index;
    /* ��ȡ����ɨ���bss����Ľṹ�� */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    if ((WLAN_MEM_NETBUF_SIZE2 - us_len) < (MAC_MEASUREMENT_RPT_FIX_LEN + MAC_BEACON_RPT_FIX_LEN)) {
        /* �ͷ� */
        oal_netbuf_free(pst_action_table_bcn_rpt);
        return OAL_SUCC;
    }
    pst_meas_rpt_ie = (mac_meas_rpt_ie_stru *)(puc_data + us_index);

    /* ������ɾ����ǰ���� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);

        /* BSSID���� */
        if (!ETHER_IS_BROADCAST(pst_trans_req_info->auc_bssid) &&
            OAL_MEMCMP(pst_scanned_bss->st_bss_dscr_info.auc_bssid, pst_trans_req_info->auc_bssid, WLAN_MAC_ADDR_LEN)) {
            continue;
        }

        /* SSID���ˣ���������ssid����Ϊ0���򲻹��� */
        if (pst_trans_req_info->us_ssid_len != 0 &&
            OAL_MEMCMP(pst_scanned_bss->st_bss_dscr_info.ac_ssid,
                       pst_trans_req_info->auc_ssid, pst_trans_req_info->us_ssid_len)) {
            continue;
        }

        /*************************************************************************/
        /*                   Measurement Report IE - Frame Body                  */
        /* --------------------------------------------------------------------- */
        /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
        /* --------------------------------------------------------------------- */
        /* |1          |1      | 1        |  1            | 1         | var      */
        /* --------------------------------------------------------------------- */
        /*                                                                       */
        /*************************************************************************/
        pst_meas_rpt_ie->uc_eid = MAC_EID_MEASREP;
        pst_meas_rpt_ie->uc_len = MAC_MEASUREMENT_RPT_FIX_LEN - MAC_IE_HDR_LEN + MAC_BEACON_RPT_FIX_LEN;
        pst_meas_rpt_ie->uc_token = pst_trans_req_info->uc_meas_token;
        memset_s(&(pst_meas_rpt_ie->st_rptmode),
                 OAL_SIZEOF(mac_meas_rpt_mode_stru), 0,
                 OAL_SIZEOF(mac_meas_rpt_mode_stru));
        pst_meas_rpt_ie->uc_rpttype = RM_RADIO_MEAS_BCN;

        pst_bcn_rpt = (mac_bcn_rpt_stru *)pst_meas_rpt_ie->auc_meas_rpt;
        memset_s(pst_bcn_rpt, OAL_SIZEOF(mac_bcn_rpt_stru), 0, OAL_SIZEOF(mac_bcn_rpt_stru));
        memcpy_s(pst_bcn_rpt->auc_bssid, WLAN_MAC_ADDR_LEN,
                 pst_scanned_bss->st_bss_dscr_info.auc_bssid, WLAN_MAC_ADDR_LEN);
        pst_bcn_rpt->uc_channum = pst_scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number;
        if (pst_hmac_vap->bit_11k_auth_oper_class != 0) {
            pst_bcn_rpt->uc_optclass = pst_hmac_vap->bit_11k_auth_oper_class;
        } else {
            pst_bcn_rpt->uc_optclass = pst_trans_req_info->uc_oper_class;
        }

        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_rrm_proc_save_bss_etc::In Channel [%d] Find BSS %02X:XX:XX:XX:%02X:%02X.}",
                         pst_scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number,
                         pst_scanned_bss->st_bss_dscr_info.auc_bssid[0],
                         pst_scanned_bss->st_bss_dscr_info.auc_bssid[4],
                         pst_scanned_bss->st_bss_dscr_info.auc_bssid[5]);

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
        memcpy_s(pst_bcn_rpt->aul_act_meas_start_time,
                 OAL_SIZEOF(pst_bcn_rpt->aul_act_meas_start_time),
                 pst_hmac_vap->pst_rrm_info->aul_act_meas_start_time,
                 OAL_SIZEOF(pst_bcn_rpt->aul_act_meas_start_time));
        /* Meas Duration,�ο������豸, bcn report��req duration��дһ�� */
        pst_bcn_rpt->us_duration = pst_trans_req_info->us_duration;

        pst_bcn_rpt->bit_condensed_phy_type = pst_scanned_bss->st_bss_dscr_info.uc_phy_type;
        pst_bcn_rpt->bit_rpt_frm_type = 0; /* Beacon/Probe rsp */
        pst_bcn_rpt->uc_rcpi = HMAC_RRM_CAL_RCPI(pst_scanned_bss->st_bss_dscr_info.c_rssi);
        pst_bcn_rpt->uc_rsni = (oal_uint8)((pst_scanned_bss->st_bss_dscr_info.ac_rsni[0] +
            pst_scanned_bss->st_bss_dscr_info.ac_rsni[1]) / 2);
        pst_bcn_rpt->uc_antenna_id = 0;  // unknown
        pst_bcn_rpt->ul_parent_tsf = pst_scanned_bss->st_bss_dscr_info.ul_parent_tsf;
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
        pst_bcn_rpt->uc_rcpi = (oal_uint8)((oal_uint32)(pst_scanned_bss->st_bss_dscr_info.c_rssi + 110) << 1);

        /* ��ȡ�����  RSNI=(����� + 10) * 2 */
        uc_snr_ant0 = (oal_uint8)pst_scanned_bss->st_bss_dscr_info.c_ant0_rssi;
        uc_snr_ant1 = (oal_uint8)pst_scanned_bss->st_bss_dscr_info.c_ant0_rssi;
        if (uc_snr_ant0 == OAL_SNR_INIT_VALUE && uc_snr_ant1 != OAL_SNR_INIT_VALUE) {
            uc_snr = ((uc_snr_ant1 >> 1) + 10) << 1;
        } else if (uc_snr_ant0 != OAL_SNR_INIT_VALUE && uc_snr_ant1 == OAL_SNR_INIT_VALUE) {
            uc_snr = ((uc_snr_ant0 >> 1) + 10) << 1;
        } else if (uc_snr_ant0 != OAL_SNR_INIT_VALUE && uc_snr_ant1 != OAL_SNR_INIT_VALUE) {
            uc_snr = OAL_MAX(uc_snr_ant0, uc_snr_ant1);
            uc_snr = ((uc_snr >> 1) + 10) << 1;
        } else {
            uc_snr = 0xFF; /* ��ЧֵΪ0xFF */
        }

        pst_bcn_rpt->uc_rsni = uc_snr;
#endif

        us_len += (MAC_MEASUREMENT_RPT_FIX_LEN + MAC_BEACON_RPT_FIX_LEN);
        uc_bss_idx++;
        if ((WLAN_MEM_NETBUF_SIZE2 - us_len) < (MAC_MEASUREMENT_RPT_FIX_LEN + MAC_BEACON_RPT_FIX_LEN)) {
            break;
        }

        pst_meas_rpt_ie = (mac_meas_rpt_ie_stru *)pst_bcn_rpt->auc_subelm;
    }
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                     "{hmac_scan_rrm_proc_save_bss_etc::Find BssNum=[%d],us_len=[%d].buf_size=[%d].}",
                     uc_bss_idx, us_len, WLAN_MEM_NETBUF_SIZE2);

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_action_table_bcn_rpt);
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_len;
    ul_ret = mac_vap_set_cb_tx_user_idx(pst_mac_vap, pst_tx_ctl, pst_mac_user->auc_user_mac_addr);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "(hmac_scan_rrm_proc_save_bss_etc::fail to find user by xx:xx:xx:0x:0x:0x.}",
                         pst_mac_user->auc_user_mac_addr[3],
                         pst_mac_user->auc_user_mac_addr[4],
                         pst_mac_user->auc_user_mac_addr[5]);
    }

    if (MAC_GET_CB_MPDU_LEN(pst_tx_ctl) > WLAN_MEM_NETBUF_SIZE2) {
        oal_netbuf_free(pst_action_table_bcn_rpt);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_scan_rrm_proc_save_bss_etc::invalid us_len=[%d].}",
                         MAC_GET_CB_MPDU_LEN(pst_tx_ctl));
        return OAL_SUCC;
    }
    oal_netbuf_put(pst_action_table_bcn_rpt, MAC_GET_CB_MPDU_LEN(pst_tx_ctl));

    ul_ret = hmac_tx_mgmt_send_event_etc(pst_mac_vap, pst_action_table_bcn_rpt, MAC_GET_CB_MPDU_LEN(pst_tx_ctl));
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(pst_action_table_bcn_rpt);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_scan_rrm_proc_save_bss_etc::hmac_tx_mgmt_send_event_etc failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}
#endif

oal_void hmac_scan_init_etc(hmac_device_stru *pst_hmac_device)
{
    hmac_scan_stru *pst_scan_mgmt = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;

    /* ��ʼ��ɨ������ṹ����Ϣ */
    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);
    memset_s(pst_scan_mgmt, OAL_SIZEOF(hmac_scan_stru), 0, OAL_SIZEOF(hmac_scan_stru));
    pst_scan_mgmt->en_is_scanning = OAL_FALSE;
    pst_scan_mgmt->st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;

    /* ��ʼ��bss��������������� */
    pst_bss_mgmt = &(pst_scan_mgmt->st_scan_record_mgmt.st_bss_mgmt);
    oal_dlist_init_head(&(pst_bss_mgmt->st_bss_list_head));
    oal_spin_lock_init(&(pst_bss_mgmt->st_lock));

    /* ��ʼ���ں��·�ɨ��request��Դ�� */
    oal_spin_lock_init(&(pst_scan_mgmt->st_scan_request_spinlock));

    /* ��ʼ�� st_wiphy_mgmt �ṹ */
    OAL_WAIT_QUEUE_INIT_HEAD(&(pst_scan_mgmt->st_wait_queue));

    /* ��ʼ��ɨ���������MAC ��ַ */
    oal_random_ether_addr(pst_hmac_device->st_scan_mgmt.auc_random_mac);

#if defined(_PRE_WLAN_CHIP_TEST_ALG) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_DEBUG_MODE)
    hmac_scan_ct_init();
#endif

    return;
}


oal_void hmac_scan_exit_etc(hmac_device_stru *pst_hmac_device)
{
    hmac_scan_stru *pst_scan_mgmt = OAL_PTR_NULL;

    /* ���ɨ���¼��Ϣ */
    hmac_scan_clean_scan(&(pst_hmac_device->st_scan_mgmt));

#if defined(_PRE_SUPPORT_ACS) || defined(_PRE_WLAN_FEATURE_DFS) || defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)
    if (pst_hmac_device->st_scan_mgmt.st_init_scan_timeout.en_is_registerd) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_hmac_device->st_scan_mgmt.st_init_scan_timeout);
    }
#endif

    /* ���ɨ������ṹ����Ϣ */
    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);
    memset_s(pst_scan_mgmt, OAL_SIZEOF(hmac_scan_stru), 0, OAL_SIZEOF(hmac_scan_stru));
    pst_scan_mgmt->en_is_scanning = OAL_FALSE;

#if defined(_PRE_WLAN_CHIP_TEST_ALG) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_DEBUG_MODE)
    hmac_scan_ct_exit();
#endif

    return;
}

#ifdef _PRE_DEBUG_MODE

oal_void hmac_scan_test_cb(void *p_scan_record)
{
    hmac_scan_record_stru *pst_scan_record = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = OAL_PTR_NULL;
    mac_bss_dscr_stru *pst_bss_dscr = OAL_PTR_NULL;
    hmac_scanned_bss_info *pst_scanned_bss = OAL_PTR_NULL;
    oal_dlist_head_stru *pst_entry = OAL_PTR_NULL;
    oal_time_us_stru st_curr_time;
    oal_uint8 uc_idx;

    if (p_scan_record == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "pst_scan_record is null ptr.");
        return;
    }

    pst_scan_record = (hmac_scan_record_stru *)p_scan_record;

    /* ��ӡ�ŵ�������� */
    OAL_IO_PRINT("The chan measure result: \n");
    for (uc_idx = 0; uc_idx < pst_scan_record->uc_chan_numbers; uc_idx++) {
        OAL_IO_PRINT("[channel_result]Chan num      : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].uc_channel_number);
        OAL_IO_PRINT("[channel_result]Stats cnt     : %d\n", pst_scan_record->ast_chan_results[uc_idx].uc_stats_cnt);
        OAL_IO_PRINT("[channel_result]Stats valid   : %d\n", pst_scan_record->ast_chan_results[uc_idx].uc_stats_valid);
        OAL_IO_PRINT("[channel_result]Stats time us : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].ul_total_stats_time_us);
        OAL_IO_PRINT("[channel_result]Free time 20M : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].ul_total_free_time_20M_us);
        OAL_IO_PRINT("[channel_result]Free time 40M : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].ul_total_free_time_40M_us);
        OAL_IO_PRINT("[channel_result]Free time 80M : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].ul_total_free_time_80M_us);
        OAL_IO_PRINT("[channel_result]Send time     : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].ul_total_send_time_us);
        OAL_IO_PRINT("[channel_result]Recv time     : %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].ul_total_recv_time_us);
        OAL_IO_PRINT("[channel_result]Free power cnt: %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].uc_free_power_cnt);
        OAL_IO_PRINT("[channel_result]Free power 20M: %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].s_free_power_stats_20M);
        OAL_IO_PRINT("[channel_result]Free power 40M: %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].s_free_power_stats_40M);
        OAL_IO_PRINT("[channel_result]Free power 80M: %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].s_free_power_stats_80M);
        OAL_IO_PRINT("[channel_result]Radar detected: %d\n",
                     pst_scan_record->ast_chan_results[uc_idx].uc_radar_detected);
        OAL_IO_PRINT("[channel_result]Radar bw      : %d\n\n", pst_scan_record->ast_chan_results[uc_idx].uc_radar_bw);
    }

    /* ��ӡBSS��� */
    OAL_IO_PRINT("The bss result: \n");

    /* ��ȡɨ�����Ĺ����ṹ��ַ */
    pst_bss_mgmt = &(pst_scan_record->st_bss_mgmt);

    /* ��ȡ�� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����ɨ�赽��bss��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);
        if (pst_bss_dscr == OAL_PTR_NULL) {
            continue;
        }

        OAL_IO_PRINT("Chan num      : %d\n", pst_bss_dscr->st_channel.uc_chan_number);
        OAL_IO_PRINT("BSSID         : 0x%x:0x0x:0x0x:0x0x:0x%x:0x%x\n\n", pst_bss_dscr->auc_bssid[0],
                     pst_bss_dscr->auc_bssid[4],
                     pst_bss_dscr->auc_bssid[5]);
    }

    /* �ͷ��� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    /* ��ӡ�˴�ɨ���ʱ */
    oal_time_get_stamp_us(&st_curr_time);
    OAL_IO_PRINT("Scan start time: %d %d\n",
                 (oal_uint32)pst_scan_record->st_scan_start_timestamp.i_sec,
                 (oal_uint32)pst_scan_record->st_scan_start_timestamp.i_usec);
    OAL_IO_PRINT("Scan end   time: %d %d\n\n",
                 (oal_uint32)st_curr_time.i_sec,
                 (oal_uint32)st_curr_time.i_usec);

    return;
}


oal_uint32 hmac_scan_test(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_scan_req_stru st_scan_req;
    oal_uint8 uc_chan_idx;
    oal_uint32 ul_ret;
    oal_uint8 uc_2g_chan_num = 0;
    oal_uint8 uc_5g_chan_num = 0;
    oal_uint8 uc_chan_number = 0;
    oal_int8 ac_param[15] = { 0 };
    oal_bool_enum_uint8 en_2g_all = OAL_FALSE;
    oal_bool_enum_uint8 en_5g_all = OAL_FALSE;
    mac_ioctl_scan_test_config_stru *pst_scan_test = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    pst_scan_test = (mac_ioctl_scan_test_config_stru *)puc_param;

    memcpy_s(ac_param, sizeof(ac_param), pst_scan_test->ac_scan_type, sizeof(ac_param));
    memset_s(&st_scan_req, OAL_SIZEOF(st_scan_req), 0, OAL_SIZEOF(st_scan_req));

    if (0 == oal_strcmp("2g", ac_param)) {
        en_2g_all = OAL_TRUE;
    } else if (0 == oal_strcmp("5g", ac_param)) {
        en_5g_all = OAL_TRUE;
    } else if (0 == oal_strcmp("all", ac_param)) {
        en_2g_all = OAL_TRUE;
        en_5g_all = OAL_TRUE;
    } else {
        /* ָ�������ŵ� */
        uc_chan_number = (oal_uint8)oal_atoi(ac_param);
    }

    if (en_2g_all) {
        /* 2Gȫ�ŵ� */
        for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++) {
            ul_ret = mac_is_channel_idx_valid_etc(WLAN_BAND_2G, uc_chan_idx);
            if (ul_ret == OAL_SUCC) {
                mac_get_channel_num_from_idx_etc(WLAN_BAND_2G, uc_chan_idx, &uc_chan_number);

                st_scan_req.ast_channel_list[uc_2g_chan_num].uc_chan_number = uc_chan_number;
                st_scan_req.ast_channel_list[uc_2g_chan_num].en_band = WLAN_BAND_2G;
                st_scan_req.ast_channel_list[uc_2g_chan_num].uc_chan_idx = uc_chan_idx;
                st_scan_req.ast_channel_list[uc_2g_chan_num].en_bandwidth = pst_scan_test->en_bandwidth;
                st_scan_req.uc_channel_nums++;
                uc_2g_chan_num++;
            }
        }
    }

    if (en_5g_all) {
        /* 5Gȫ�ŵ� */
        for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++) {
            ul_ret = mac_is_channel_idx_valid_etc(WLAN_BAND_5G, uc_chan_idx);
            if (ul_ret == OAL_SUCC) {
                mac_get_channel_num_from_idx_etc(WLAN_BAND_5G, uc_chan_idx, &uc_chan_number);

                st_scan_req.ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].uc_chan_number = uc_chan_number;
                st_scan_req.ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].en_band = WLAN_BAND_5G;
                st_scan_req.ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].uc_chan_idx = uc_chan_idx;
                st_scan_req.ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].en_bandwidth =
                    pst_scan_test->en_bandwidth;
                st_scan_req.uc_channel_nums++;
                uc_5g_chan_num++;
            }
        }
    }

    if (en_2g_all == OAL_FALSE && en_5g_all == OAL_FALSE) {
        if (uc_chan_number < 15) {
            ul_ret = mac_is_channel_num_valid_etc(WLAN_BAND_2G, uc_chan_number);
            if (ul_ret != OAL_SUCC) {
                OAL_IO_PRINT("Invalid chan num: %d. return!\n", uc_chan_number);
                return OAL_FAIL;
            }
            st_scan_req.ast_channel_list[0].en_band = WLAN_BAND_2G;
        } else {
            ul_ret = mac_is_channel_num_valid_etc(WLAN_BAND_5G, uc_chan_number);
            if (ul_ret != OAL_SUCC) {
                OAL_IO_PRINT("Invalid chan num: %d. return!\n", uc_chan_number);
                return OAL_FAIL;
            }
            st_scan_req.ast_channel_list[0].en_band = WLAN_BAND_5G;
        }

        st_scan_req.ast_channel_list[0].uc_chan_number = uc_chan_number;
        st_scan_req.ast_channel_list[0].en_bandwidth = pst_scan_test->en_bandwidth;

        ul_ret = mac_get_channel_idx_from_num_etc(st_scan_req.ast_channel_list[0].en_band,
                                                  st_scan_req.ast_channel_list[0].uc_chan_number,
                                                  &(st_scan_req.ast_channel_list[0].uc_chan_idx));
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "hmac_scan_test::mac_get_channel_idx_from_num_etc fail=%d", ul_ret);
            return OAL_FAIL;
        }
        st_scan_req.uc_channel_nums = 1;
    }

    st_scan_req.en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    st_scan_req.en_scan_type = WLAN_SCAN_TYPE_ACTIVE;
    st_scan_req.uc_bssid_num = 0;
    st_scan_req.uc_ssid_num = 0;

    oal_set_mac_addr(st_scan_req.auc_bssid[0], BROADCAST_MACADDR);
    st_scan_req.uc_bssid_num = 1;

    st_scan_req.us_scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    st_scan_req.en_scan_mode = WLAN_SCAN_MODE_FOREGROUND;
    st_scan_req.uc_scan_func = MAC_SCAN_FUNC_ALL;

    st_scan_req.p_fn_cb = hmac_scan_test_cb;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    return hmac_scan_proc_scan_req_event_etc(pst_hmac_vap, &st_scan_req);
}
#endif


oal_uint32 hmac_bgscan_enable_etc(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    hmac_scan_state_enum_uint8 pen_bgscan_enable_flag;

    pen_bgscan_enable_flag = *puc_param; /* ����ɨ��ֹͣʹ��λ */

    /* ����ɨ��ֹͣ���� */
    switch (pen_bgscan_enable_flag) {
        case 0:
            g_en_bgscan_enable_flag_etc = HMAC_BGSCAN_DISABLE;
            break;
        case 1:
            g_en_bgscan_enable_flag_etc = HMAC_BGSCAN_ENABLE;
            break;
        case 2:
            g_en_bgscan_enable_flag_etc = HMAC_SCAN_DISABLE;
            break;
        default:
            g_en_bgscan_enable_flag_etc = HMAC_BGSCAN_ENABLE;
            break;
    }

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "hmac_bgscan_enable_etc: g_en_bgscan_enable_flag_etc=%d.",
                     g_en_bgscan_enable_flag_etc);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DBAC

oal_uint32 hmac_scan_start_dbac_etc(mac_device_stru *pst_dev)
{
    oal_uint8 auc_cmd[32];
    oal_uint16 us_len;
    oal_uint32 ul_ret = OAL_FAIL;
    oal_uint8 uc_idx;
#define DBAC_START_STR     " dbac start"
#define DBAC_START_STR_LEN 11
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    mac_ioctl_alg_config_stru *pst_alg_config = (mac_ioctl_alg_config_stru *)auc_cmd;

    if (EOK != memcpy_s(auc_cmd + OAL_SIZEOF(mac_ioctl_alg_config_stru),
                        sizeof(auc_cmd) - OAL_SIZEOF(mac_ioctl_alg_config_stru),
                        (const oal_int8 *)DBAC_START_STR, 11)) {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "hmac_scan_start_dbac_etc::memcpy fail!");
        return OAL_FAIL;
    }
    auc_cmd[OAL_SIZEOF(mac_ioctl_alg_config_stru) + DBAC_START_STR_LEN] = 0;

    pst_alg_config->uc_argc = 2;
    pst_alg_config->auc_argv_offset[0] = 1;
    pst_alg_config->auc_argv_offset[1] = 6;

    for (uc_idx = 0; uc_idx < pst_dev->uc_vap_num; uc_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_dev->auc_vap_id[uc_idx]);
        if (pst_mac_vap != OAL_PTR_NULL &&
            pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            break;
        }
    }
    if (pst_mac_vap) {
        us_len = OAL_SIZEOF(mac_ioctl_alg_config_stru) + DBAC_START_STR_LEN + 1;
        ul_ret = hmac_config_send_event_etc(pst_mac_vap, WLAN_CFGID_ALG, us_len, auc_cmd);
        if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_alg_etc::hmac_config_send_event_etc failed[%d].}", ul_ret);
        }
        OAL_IO_PRINT("start dbac\n");
    } else {
        OAL_IO_PRINT("no vap found to start dbac\n");
    }

    return ul_ret;
}
#endif

oal_uint32 hmac_start_all_bss_of_device_etc(hmac_device_stru *pst_hmac_dev)
{
    oal_uint8 uc_idx;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    mac_device_stru *pst_dev = pst_hmac_dev->pst_device_base_info;

    OAM_WARNING_LOG1(0, OAM_SF_ACS, "{hmac_start_all_bss_of_device_etc:device id=%d}",
                     pst_hmac_dev->pst_device_base_info->uc_device_id);
    if (OAL_UNLIKELY(pst_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_start_all_bss_of_device_etc::pst_device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_idx = 0; uc_idx < pst_dev->uc_vap_num; uc_idx++) {
        pst_hmac_vap = mac_res_get_hmac_vap(pst_dev->auc_vap_id[uc_idx]);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG2(0, OAM_SF_ACS, "hmac_start_all_bss_of_device_etc:null ap, idx=%d id=%d",
                             uc_idx, pst_dev->auc_vap_id[uc_idx]);
            continue;
        }

        if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_AP_WAIT_START
#ifdef _PRE_WLAN_FEATURE_DBAC
             || (mac_is_dbac_enabled(pst_dev) && pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_PAUSE)
#endif
             || pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) {
            hmac_chan_start_bss_etc(pst_hmac_vap, OAL_PTR_NULL, WLAN_PROTOCOL_BUTT);
        } else {
            OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ACS, "vap %d not started\n",
                             pst_dev->auc_vap_id[uc_idx]);
            continue;
        }
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_enabled(pst_dev)) {
        return hmac_scan_start_dbac_etc(pst_dev);
    }
#endif

    return OAL_SUCC;
}

/* TBD �˺�������scanģ�����ƴ�׮�ڴˣ���ҪACS��д�˽ӿ� �ŵ�hmac_acs�ļ��� */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

OAL_STATIC oal_uint32 hmac_get_pri_sec_chan(mac_bss_dscr_stru *pst_bss_dscr,
                                            oal_uint32 *pul_pri_chan,
                                            oal_uint32 *pul_sec_chan)
{
    *pul_pri_chan = *pul_sec_chan = 0;

    *pul_pri_chan = pst_bss_dscr->st_channel.uc_chan_number;

    OAM_INFO_LOG1(0, OAM_SF_2040, "hmac_get_pri_sec_chan:pst_bss_dscr->st_channel.en_bandwidth = %d\n",
                  pst_bss_dscr->en_channel_bandwidth);

    if (WLAN_BAND_WIDTH_40PLUS == pst_bss_dscr->en_channel_bandwidth) {
        *pul_sec_chan = *pul_pri_chan + 4;
    } else if (WLAN_BAND_WIDTH_40MINUS == pst_bss_dscr->en_channel_bandwidth) {
        *pul_sec_chan = *pul_pri_chan - 4;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_2040,
                         "hmac_get_pri_sec_chan: pst_bss_dscr is not support 40Mhz, *pul_sec_chan = %d\n",
                         *pul_sec_chan);
    }

    OAM_INFO_LOG2(0, OAM_SF_2040, "*pul_pri_chan = %d, *pul_sec_chan = %d\n\n", *pul_pri_chan, *pul_sec_chan);

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_switch_pri_sec(mac_channel_stru *pst_channel)
{
    if (pst_channel->en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        pst_channel->uc_chan_number = pst_channel->uc_chan_number + 4;
        pst_channel->en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
    } else if (pst_channel->en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        pst_channel->uc_chan_number = pst_channel->uc_chan_number - 4;
        pst_channel->en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_2040, "hmac_switch_pri_sec:en_bandwidth = %d\n not need obss scan\n",
                         pst_channel->en_bandwidth);
    }
}


OAL_STATIC oal_bool_enum_uint8 hmac_obss_check_40mhz_2g(mac_vap_stru *pst_mac_vap,
                                                        hmac_scan_record_stru *pst_scan_record,
                                                        mac_channel_stru *pst_dst_channel)
{
    oal_uint32 ul_pri_freq;
    oal_uint32 ul_sec_freq;
    oal_uint32 ul_affected_start;
    oal_uint32 ul_affected_end;

    oal_uint32 ul_pri;
    oal_uint32 ul_sec;
    oal_uint32 ul_sec_chan, ul_pri_chan;
    oal_int8 c_obss_rssi_th;

    hmac_bss_mgmt_stru *pst_bss_mgmt;
    mac_bss_dscr_stru *pst_bss_dscr;
    hmac_scanned_bss_info *pst_scanned_bss;
    oal_dlist_head_stru *pst_entry;

    *pst_dst_channel = pst_mac_vap->st_channel;
    c_obss_rssi_th = HMAC_OBSS_RSSI_TH;
    /* ��ȡ���ŵ������ŵ�����Ƶ�� */
    // 2412 + (pst_mac_vap->st_channel.uc_chan_number - 1) * 5;
    ul_pri_freq = (oal_int32)g_ast_freq_map_2g_etc[pst_mac_vap->st_channel.uc_chan_number - 1].us_freq;

    if (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        ul_sec_freq = ul_pri_freq + 20;
    } else if (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        ul_sec_freq = ul_pri_freq - 20;
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "hmac_obss_check_40mhz_2g:en_bandwidth = %d not need obss\n",
                         pst_mac_vap->st_channel.en_bandwidth);
        return OAL_TRUE;
    }

    /* 2.4G������,��ⷶΧ��40MHz��������Ƶ��Ϊ����,���Ҹ�25MHZ */
    ul_affected_start = ((ul_pri_freq + ul_sec_freq) >> 1) - 25;
    ul_affected_end = ((ul_pri_freq + ul_sec_freq) >> 1) + 25;

    OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                  "hmac_obss_check_40mhz_2g:40 MHz affected channel range: [%d, %d] MHz",
                  ul_affected_start, ul_affected_end);

    /* ��ȡɨ�����Ĺ����ṹ��ַ */
    pst_bss_mgmt = &(pst_scan_record->st_bss_mgmt);

    /* ��ȡ�� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ����ɨ�赽��bss��Ϣ */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);
        if (pst_bss_dscr == OAL_PTR_NULL) {
            OAM_WARNING_LOG0(0, OAM_SF_2040, "{hmac_obss_check_40mhz_2g::pst_bss_dscr is NULL}");
            continue;
        }

        if (pst_bss_dscr->c_rssi < c_obss_rssi_th) {
            continue;
        }
        // 2412 + (st_chan_result.uc_channel_number - 1) * 5;
        ul_pri = (oal_int32)g_ast_freq_map_2g_etc[pst_bss_dscr->st_channel.uc_chan_number - 1].us_freq;
        ul_sec = ul_pri;

        OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                      "pst_bss_dscr->st_channel.uc_chan_number = %d, ul_pri = %d\n",
                      pst_bss_dscr->st_channel.uc_chan_number, ul_pri);

        /* ��ȡɨ�赽��BSS���ŵ���Ƶ����Ϣ */
        hmac_get_pri_sec_chan(pst_bss_dscr, &ul_pri_chan, &ul_sec_chan);

        /* ��BSSΪ40MHz����,������ŵ�Ƶ�� */
        if (ul_sec_chan) {
            if (ul_sec_chan < ul_pri_chan) {
                ul_sec = ul_pri - 20;
            } else {
                ul_sec = ul_pri + 20;
            }
        }

        if ((ul_pri < ul_affected_start || ul_pri > ul_affected_end) &&
            (ul_sec < ul_affected_start || ul_sec > ul_affected_end)) {
            continue; /* not within affected channel range */
        }

        if (ul_pri_freq != ul_pri) {  // �е�Ƶ���Ҳ���ͬһ���ŵ�����ǰAP����ʹ��40M
            OAM_WARNING_LOG4(0, OAM_SF_2040,
                             "hmac_obss_check_40mhz_2g:40 MHz pri/sec <%d, %d >mismatch with BSS <%d, %d>\n",
                             ul_pri_freq, ul_sec_freq, ul_pri, ul_sec);
            /* ����� */
            oal_spin_unlock(&(pst_bss_mgmt->st_lock));

            // just trim bandwidth to 2G
            pst_dst_channel->en_bandwidth = WLAN_BAND_WIDTH_20M;

            return OAL_FALSE;
        }
    }

    /* ����� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return OAL_TRUE;
}


OAL_STATIC oal_bool_enum_uint8 hmac_obss_check_40mhz_5g(mac_vap_stru *pst_mac_vap,
                                                        hmac_scan_record_stru *pst_scan_record,
                                                        mac_channel_stru *pst_dst_channel)
{
    oal_uint32 ul_pri_chan;
    oal_uint32 ul_sec_chan;
    oal_uint32 ul_pri_bss;
    oal_uint32 ul_sec_bss;
    oal_uint32 ul_bss_pri_chan;
    oal_uint32 ul_bss_sec_chan;
    oal_uint8 uc_match;
    oal_uint8 uc_inverse;
    oal_uint8 uc_pri_20_bss;
    oal_uint8 uc_sec_20_bss;
    hmac_bss_mgmt_stru *pst_bss_mgmt;
    mac_bss_dscr_stru *pst_bss_dscr;
    hmac_scanned_bss_info *pst_scanned_bss;
    oal_dlist_head_stru *pst_entry;
    oal_int8 c_obss_rssi_th;

    *pst_dst_channel = pst_mac_vap->st_channel;

    /* ��ȡ���ŵ��ʹ��ŵ� */
    ul_pri_chan = pst_mac_vap->st_channel.uc_chan_number;
    if (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        ul_sec_chan = ul_pri_chan + 4;
    } else if (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        ul_sec_chan = ul_pri_chan - 4;
    } else {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "hmac_obss_check_40mhz_5g: en_bandwidth = %d not need obss scan",
                         pst_mac_vap->st_channel.en_bandwidth);
        return OAL_TRUE;
    }

    /* ��ȡɨ�����Ĺ����ṹ��ַ */
    pst_bss_mgmt = &(pst_scan_record->st_bss_mgmt);

    /* ��ȡ�� */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* ���ڴ��ŵ���⵽Beacon, �������ŵ���û��, ����Ҫ���������ŵ� */
    ul_pri_bss = ul_sec_bss = 0;
    c_obss_rssi_th = HMAC_OBSS_RSSI_TH;

    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);

        if (pst_bss_dscr == OAL_PTR_NULL) {
            OAM_WARNING_LOG0(0, OAM_SF_2040, "{hmac_obss_check_40mhz_5g::pst_bss_dscr is NULL}");
            continue;
        }
        if (pst_bss_dscr->c_rssi < c_obss_rssi_th) {
            continue;
        }

        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "hmac_obss_check_40mhz_5g:bss uc_channel_number = %d\n",
                      pst_bss_dscr->st_channel.uc_chan_number);
        if (pst_bss_dscr->st_channel.uc_chan_number == ul_pri_chan) {
            ul_pri_bss++;
        } else if (pst_bss_dscr->st_channel.uc_chan_number == ul_sec_chan) {
            ul_sec_bss++;
        }
    }

    if (ul_sec_bss && !ul_pri_bss) {
        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_2040,
            "hmac_obss_check_40mhz_5g:Switch own primary and sec chan to get sec chan with no Beacons from other BSS");

        hmac_switch_pri_sec(pst_dst_channel);

        /* �˴����ν������ֱ�ӷ��ؼ���, ��hostapd-2.4.0�汾�޸� */
        /* �ͷ��� */
        oal_spin_unlock(&(pst_bss_mgmt->st_lock));
        // ���ﲻ���ж��Ƿ���Ҫ�л���20M
        return OAL_TRUE;
    }

    /*
     * Match PRI/SEC channel with any existing HT40 BSS on the same
     * channels that we are about to use (if already mixed order in
     * existing BSSes, use own preference).
     */
    // �Ƿ����뵱ǰAP�����ŵ���ȫһ�»����෴��40M BSS������Ҳ��ȷ���Ƿ��������ŵ����ߴ��ŵ���20M BSS
    uc_match = OAL_FALSE;
    uc_inverse = OAL_FALSE;
    uc_pri_20_bss = OAL_FALSE;
    uc_sec_20_bss = OAL_FALSE;
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bss_mgmt->st_bss_list_head))
    {
        pst_scanned_bss = OAL_DLIST_GET_ENTRY(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr = &(pst_scanned_bss->st_bss_dscr_info);
        if (pst_bss_dscr == OAL_PTR_NULL) {
            OAM_WARNING_LOG0(0, OAM_SF_2040, "{hmac_obss_check_40mhz_5g::pst_bss_dscr is NULL}");
            continue;
        }
        if (pst_bss_dscr->c_rssi < c_obss_rssi_th) {
            continue;
        }

        hmac_get_pri_sec_chan(pst_bss_dscr, &ul_bss_pri_chan, &ul_bss_sec_chan);
        if (ul_pri_chan == ul_bss_pri_chan && 0 == ul_bss_sec_chan) {
            uc_pri_20_bss = OAL_TRUE;  // �����ڵ�ǰAP���ŵ���20M BSS
        }
        if (ul_sec_chan == ul_bss_pri_chan && 0 == ul_bss_sec_chan) {
            uc_sec_20_bss = OAL_TRUE;  // �����ڵ�ǰAP���ŵ���20M BSS
        }
        if (ul_pri_chan == ul_bss_sec_chan &&
            ul_sec_chan == ul_bss_pri_chan) {
            uc_inverse = OAL_TRUE;  // �����뵱ǰAP�����ŵ��෴��
        }
        if (ul_pri_chan == ul_bss_pri_chan &&
            ul_sec_chan == ul_bss_sec_chan) {
            uc_match = OAL_TRUE;  // �����뵱ǰAP�����ŵ�һ�µ�
        }
    }
    // ���෴��40M,û��һ�µģ�����20û��20M BSS,���������ŵ�
    if (uc_match == OAL_FALSE && uc_inverse == OAL_TRUE && uc_pri_20_bss == OAL_FALSE) {
        OAM_WARNING_LOG0(0, OAM_SF_2040,
                         "hmac_obss_check_40mhz_5g:switch own primary and secondary channel due to BSS overlap with\n");

        hmac_switch_pri_sec(pst_dst_channel);
    } else if (uc_match == OAL_FALSE && uc_inverse == OAL_TRUE &&
               uc_pri_20_bss == OAL_TRUE && uc_sec_20_bss == OAL_FALSE) {
        // ���෴��40M,û��һ�µģ�����20��20M BSS,��20û��20M BSS,��Ҫ���������ŵ������л���20M
        OAM_WARNING_LOG0(0, OAM_SF_2040,
            "hmac_obss_check_40mhz_5g:switch own primary and secondary channel due to BSS overlap and to 20M\n");

        hmac_switch_pri_sec(pst_dst_channel);
        pst_dst_channel->en_bandwidth = WLAN_BAND_WIDTH_20M;
    } else if (uc_sec_20_bss == OAL_TRUE) {  // ���ŵ���20M BSS����Ҫ�л���20M
        OAM_WARNING_LOG2(0, OAM_SF_2040, "hmac_obss_check_40mhz_5g:40 MHz pri/sec <%d, %d > to 20M\n",
                         ul_pri_chan, ul_sec_chan);
        pst_dst_channel->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    /* ����� */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_obss_check_40mhz(mac_vap_stru *pst_mac_vap,
                                                                hmac_scan_record_stru *pst_scan_record,
                                                                mac_channel_stru *pst_dst_channel)
{
    return (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) ?
            hmac_obss_check_40mhz_2g(pst_mac_vap, pst_scan_record, pst_dst_channel) :
            hmac_obss_check_40mhz_5g(pst_mac_vap, pst_scan_record, pst_dst_channel);
}


oal_uint32 hmac_obss_init_scan_hook_etc(hmac_scan_record_stru *pst_scan_record,
                                        hmac_device_stru *pst_dev)
{
    /*
    1����ǰVAP���ŵ���ΪĿ���ŵ�
    2���ж�֮������Ҫ����VAP���ŵ��ʹ���
    3����ǰʵ���У������Ǹ����ŵ��ţ�ֻ���Ĵ���
    4���ŵ������浽device�£�vap���ŵ��������£���init scan hook�м��д���
    */
    oal_uint8 uc_idx;
    mac_vap_stru *pst_mac_vap;
    mac_vap_stru *pst_vap[WLAN_BAND_BUTT] = { OAL_PTR_NULL, OAL_PTR_NULL };

    OAM_WARNING_LOG0(0, OAM_SF_ACS, "hmac_obss_init_scan_hook_etc run\n");

    /* 2G/5G vap ����һ�� */
    for (uc_idx = 0; uc_idx < pst_dev->pst_device_base_info->uc_vap_num; uc_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_dev->pst_device_base_info->auc_vap_id[uc_idx]);

        if (pst_mac_vap != OAL_PTR_NULL && pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            if ((pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) && !pst_vap[WLAN_BAND_2G]) {
                pst_vap[WLAN_BAND_2G] = pst_mac_vap;
            } else if ((pst_mac_vap->st_channel.en_band == WLAN_BAND_5G) && !pst_vap[WLAN_BAND_5G]) {
                pst_vap[WLAN_BAND_5G] = pst_mac_vap;
            }
        }
    }

    for (uc_idx = 0; uc_idx < WLAN_BAND_BUTT; uc_idx++) {
        if (pst_vap[uc_idx]) {
            hmac_obss_check_40mhz(pst_vap[uc_idx], pst_scan_record, pst_dev->ast_best_channel + uc_idx);
            pst_vap[uc_idx]->st_ch_switch_info.en_user_pref_bandwidth = pst_dev->ast_best_channel[uc_idx].en_bandwidth;

            OAM_WARNING_LOG4(pst_vap[uc_idx]->uc_vap_id, OAM_SF_2040,
                             "hmac_obss_init_scan_hook_etc::before:ch=%d bw=%d, after :ch=%d bw=%d",
                             pst_vap[uc_idx]->st_channel.uc_chan_number,
                             pst_vap[uc_idx]->st_channel.en_bandwidth,
                             pst_dev->ast_best_channel[uc_idx].uc_chan_number,
                             pst_dev->ast_best_channel[uc_idx].en_bandwidth);
        }
    }

    return OAL_SUCC;
}
#endif

#if defined(_PRE_SUPPORT_ACS) || defined(_PRE_WLAN_FEATURE_DFS) || defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST)

oal_bool_enum_uint8 hmac_device_in_init_scan_etc(mac_device_stru *pst_mac_device)
{
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_device->uc_device_id);

    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_res_get_mac_dev_etc:pst_hmac_device is null\n");
        return OAL_FALSE;
    } else {
        return pst_hmac_device->en_in_init_scan;
    }
}


mac_need_init_scan_res_enum_uint8 hmac_need_init_scan_etc(hmac_device_stru *pst_hmac_device,
                                                          mac_vap_stru *pst_in_mac_vap,
                                                          mac_try_init_scan_type en_type)
{
    mac_need_init_scan_res_enum_uint8 en_result = MAC_INIT_SCAN_NOT_NEED;
    mac_device_stru *pst_mac_device = pst_hmac_device->pst_device_base_info;

    if (OAL_UNLIKELY(pst_mac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_need_init_scan_etc::pst_device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_in_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP && pst_in_mac_vap->en_vap_mode != WLAN_VAP_MODE_CONFIG) {
        return MAC_INIT_SCAN_NOT_NEED;
    }

    if (pst_hmac_device->en_in_init_scan) {
        return en_type == MAC_TRY_INIT_SCAN_RESCAN ? MAC_INIT_SCAN_NOT_NEED : MAC_INIT_SCAN_IN_SCAN;
    }

#ifdef _PRE_SUPPORT_ACS
    en_result |= ((mac_get_acs_switch(pst_mac_device) == MAC_ACS_SW_INIT ||
                  mac_get_acs_switch(pst_mac_device) == MAC_ACS_SW_BOTH ? OAL_TRUE : OAL_FALSE));
    if (en_type == MAC_TRY_INIT_SCAN_RESCAN) {
        return en_result;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    /*lint -e514*/
    en_result |= (mac_vap_get_dfs_enable(pst_in_mac_vap) &&
                  mac_dfs_get_cac_enable(pst_hmac_device->pst_device_base_info) &&
                  hmac_dfs_need_for_cac(pst_mac_device, pst_in_mac_vap)) ? OAL_TRUE : OAL_FALSE;
    /*lint +e514*/
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if (!pst_in_mac_vap->bit_bw_fixed) {
        en_result |= (mac_get_2040bss_switch(pst_mac_device) &&
                      (WLAN_BAND_WIDTH_20M != pst_in_mac_vap->st_channel.en_bandwidth)) ? OAL_TRUE : OAL_FALSE;
    }
#endif

    return en_result;
}


oal_uint32 hmac_init_scan_sync_channel(hmac_scan_record_stru *pst_scan_record,
                                       hmac_device_stru *pst_hmac_dev)
{
    mac_device_stru *pst_mac_device;
    mac_channel_stru *pst_channel;
    oal_uint8 uc_idx;
    mac_vap_stru *pst_mac_vap;
    oal_bool_enum_uint8 en_acs_en = OAL_FALSE;
    oal_bool_enum_uint8 en_obss_en = OAL_FALSE;
#if (defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_WLAN_FEATRUE_DBAC_DOUBLE_AP_MODE))
    hmac_vap_stru *pst_hmac_vap;
#endif
    if (!pst_scan_record || !pst_hmac_dev || !pst_hmac_dev->pst_device_base_info) {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "hmac_init_scan_sync_channel:null ptr\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = pst_hmac_dev->pst_device_base_info;
    pst_channel = pst_hmac_dev->ast_best_channel;

#ifdef _PRE_SUPPORT_ACS
    if (mac_get_acs_switch(pst_mac_device)) {
        en_acs_en = OAL_TRUE;
    }
#endif

    // just in case someone missing
    for (uc_idx = 0; uc_idx < WLAN_BAND_BUTT; uc_idx++) {
        mac_get_channel_idx_from_num_etc(pst_channel[uc_idx].en_band,
                                         pst_channel[uc_idx].uc_chan_number,
                                         &pst_channel[uc_idx].uc_chan_idx);
    }

    /*
     * ACSδʹ�ܡ�OBSSʹ��ʱ��ʹ��dev�����õ��ŵ�
     * ACSʹ�ܣ�  δomit acsʱ��ʹ��dev�����õ��ŵ�
     *              omit acsʱ��ʹ��ԭvap�ŵ�������OBSS���
     */
    for (uc_idx = 0; uc_idx < pst_mac_device->uc_vap_num; uc_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_idx]);
        if (OAL_UNLIKELY(pst_mac_vap == OAL_PTR_NULL)) {
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        if (mac_get_2040bss_switch(pst_mac_device) && (!pst_mac_vap->bit_bw_fixed)) {
            en_obss_en = OAL_TRUE;
        }
#endif

        if (!en_acs_en && !en_obss_en) {
            continue;
        }

        if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)
            && (pst_mac_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START)) {
#if (defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_WLAN_FEATRUE_DBAC_DOUBLE_AP_MODE))
            pst_hmac_vap = OAL_DLIST_GET_ENTRY(pst_mac_vap, hmac_vap_stru, st_vap_base_info);

            if (pst_hmac_vap->en_omit_acs_chan && en_acs_en) {
                // ����ACS�ŵ�����VAPԭ�ŵ�����ACS�ŵ�
                pst_channel[pst_mac_vap->st_channel.en_band] = pst_mac_vap->st_channel;
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                                 "hmac_init_scan_sync_channel:override acs channel\n");

                if (en_obss_en &&
                    ((WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth) ||
                     (WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth))) {
                    if (!hmac_obss_check_40mhz(pst_mac_vap, pst_scan_record,
                                               &pst_channel[pst_mac_vap->st_channel.en_band])) {
                        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                                         "hmac_init_scan_sync_channel:channel trimed to 20MHz for obss\n");
                    }
                    pst_mac_vap->st_ch_switch_info.en_user_pref_bandwidth =
                        pst_channel[pst_mac_vap->st_channel.en_band].en_bandwidth;
                }
            }
#endif

            // ��dev��ѡ����ŵ��Ϸ������串�ǵ�vap�ṹ��
            if (pst_channel[pst_mac_vap->st_channel.en_band].uc_chan_number) {
                pst_mac_vap->st_channel = pst_channel[pst_mac_vap->st_channel.en_band];
                OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                                 "hmac_init_scan_sync_channel:vap ch info(band=%d,bw=%d,ch=%d) sync from acs\n",
                                 pst_mac_vap->st_channel.en_band,
                                 pst_mac_vap->st_channel.en_bandwidth,
                                 pst_mac_vap->st_channel.uc_chan_number);
            } else {
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                                 "hmac_init_scan_sync_channel:no acs channel found for band %d\n",
                                 pst_mac_vap->st_channel.en_band);
            }
        }
    }

    return OAL_SUCC;
}


oal_uint32 hmac_init_scan_timeout_etc(void *p_arg)
{
    hmac_device_stru *pst_dev = (hmac_device_stru *)p_arg;

    if (!pst_dev->en_init_scan) {
        return OAL_SUCC;
    }

    pst_dev->en_init_scan = OAL_FALSE;
    pst_dev->en_in_init_scan = OAL_FALSE;
    hmac_start_all_bss_of_device_etc(pst_dev);

    return OAL_SUCC;
}


oal_uint32 hmac_init_scan_cancel_timer_etc(hmac_device_stru *pst_hmac_dev)
{
    if (pst_hmac_dev && pst_hmac_dev->st_scan_mgmt.st_init_scan_timeout.en_is_registerd) {
        /* �رճ�ʱ��ʱ�� */
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_hmac_dev->st_scan_mgmt.st_init_scan_timeout);
    }

    return OAL_SUCC;
}


oal_void hmac_init_scan_cb_etc(void *p_scan_record)
{
    hmac_scan_record_stru *pst_scan_record = (hmac_scan_record_stru *)p_scan_record;
    oal_uint8 uc_device_id = pst_scan_record->uc_device_id;
    hmac_device_stru *pst_hmac_dev = hmac_res_get_mac_dev_etc(uc_device_id);
    mac_device_stru *pst_mac_dev;
    mac_vap_stru *pst_mac_vap;

    OAM_WARNING_LOG0(0, OAM_SF_ACS, "{hmac_init_scan_cb_etc called}");

    if (pst_hmac_dev == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ACS, "{hmac_init_scan_cb_etc:pst_hmac_dev=NULL, device_id=%d}", uc_device_id);
        return;
    }

    pst_mac_dev = pst_hmac_dev->pst_device_base_info;
    if (OAL_ANY_NULL_PTR2(pst_hmac_dev, pst_mac_dev)) {
        OAM_ERROR_LOG1(0, OAM_SF_ACS, "{hmac_init_scan_cb_etc:pst_mac_dev=NULL, device_id=%d}", uc_device_id);
        return;
    }

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_scan_record->uc_vap_id);
    if (pst_mac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ACS, "{hmac_init_scan_cb_etc:pst_mac_vap=NULL, vap_id=%d}",
                       pst_scan_record->uc_vap_id);
        return;
    }

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_init_scan_cb_etc::scan report ch=%d bss=%d init=%d}\n",
                     pst_scan_record->uc_chan_numbers,
                     pst_scan_record->st_bss_mgmt.ul_bss_num, pst_hmac_dev->en_init_scan);

#ifdef _PRE_SUPPORT_ACS
    if (MAC_ACS_SW_INIT == mac_get_acs_switch(pst_mac_dev) || MAC_ACS_SW_BOTH == mac_get_acs_switch(pst_mac_dev)) {
        OAM_WARNING_LOG0(0, OAM_SF_ACS, "acs  enable, post and return\n");
        /* ��ACS�������У���ȴ�ACS APP������Ӧ��� */
        if (OAL_SUCC == hmac_acs_init_scan_hook(pst_scan_record, pst_hmac_dev)) {
            return;
        }
    }
#endif

    OAM_WARNING_LOG0(0, OAM_SF_ACS, "acs not enable, cancel timer\n");
    /* ACSδִ�У���ʱ�������˽��� */
    hmac_init_scan_cancel_timer_etc(pst_hmac_dev);

    /* ��ACSδִ�л���ִ��ʧ�ܣ�����ִ�к������� */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if (mac_get_2040bss_switch(pst_mac_dev) && (!pst_mac_vap->bit_bw_fixed)) {
        hmac_obss_init_scan_hook_etc(pst_scan_record, pst_hmac_dev);
    }
#endif

    /*
     * ���ˣ��ŵ�����ȷ�ϣ���������device����δͬ����vap���ŵ��ṹ��
     * �˴�ͬ������¼ԭʼ�ŵ���ͬʱ��֤CAC���������ŵ���ִ��
     * �����������ŵ���Ӳ�����ɺ����������
     */
    hmac_init_scan_sync_channel(pst_scan_record, pst_hmac_dev);

#ifdef _PRE_WLAN_FEATURE_DFS
    if (mac_vap_get_dfs_enable(pst_mac_vap)) {
        /* ���ɹ���ʼ��CAC�����Ѿ���ʼCAC�����أ���CAC��ʱ��������VAP START */
        if (OAL_SUCC == hmac_dfs_init_scan_hook_etc(pst_scan_record, pst_hmac_dev)) {
            return;
        }
    }
#endif

    /* ACSδ���С�DFSδ���У�ֱ������BSS */
    hmac_init_scan_timeout_etc(pst_hmac_dev);
}


oal_uint32 hmac_init_scan_do_etc(hmac_device_stru *pst_hmac_dev, mac_vap_stru *pst_mac_vap,
                                 mac_init_scan_req_stru *pst_cmd, hmac_acs_cfg_stru *pst_acs_cfg)
{
    oal_uint8 uc_idx, uc_cnt;
    mac_scan_req_stru st_scan_req;
    wlan_channel_band_enum_uint8 en_band;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    oal_uint32 ul_ret = OAL_FAIL;
    hmac_vap_stru *pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, pst_acs_cfg)) {
        OAM_ERROR_LOG1(0, OAM_SF_ACS, "{hmac_init_scan_do_etc:get hmac vap=NULL or acs_cfg is NULL, vapid=%d}",
                       pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    memset_s(&st_scan_req, OAL_SIZEOF(st_scan_req), 0, OAL_SIZEOF(st_scan_req));
    st_scan_req.en_scan_mode = pst_cmd->auc_arg[0];
    st_scan_req.en_scan_type = pst_cmd->auc_arg[1];
    st_scan_req.uc_scan_func = pst_cmd->auc_arg[2];
    st_scan_req.en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    st_scan_req.uc_bssid_num = 0;
    st_scan_req.uc_ssid_num = 0;

    st_scan_req.uc_max_scan_count_per_channel = 1;
    st_scan_req.uc_max_send_probe_req_count_per_channel = st_scan_req.en_scan_mode == WLAN_SCAN_MODE_FOREGROUND ? 2 : 1;

    st_scan_req.us_scan_time = st_scan_req.en_scan_mode == WLAN_SCAN_MODE_FOREGROUND ? 120 : 30;
    st_scan_req.uc_probe_delay = 0;
    st_scan_req.uc_vap_id = pst_mac_vap->uc_vap_id; /* ��ʵ�Ǹ�device�µ�vap_id[0] */
    st_scan_req.p_fn_cb = hmac_init_scan_cb_etc;

    st_scan_req.uc_channel_nums = 0; /* �ŵ��б����ŵ��ĸ��� */
#ifdef _PRE_SUPPORT_ACS
    st_scan_req.uc_acs_type = pst_acs_cfg->uc_acs_type;
    st_scan_req.en_switch_chan = pst_acs_cfg->en_switch_chan;
#endif
    uc_cnt = 0;
    for (uc_idx = 0; uc_idx < pst_cmd->ul_cmd_len; uc_idx += 2) {
        en_band = pst_cmd->auc_data[uc_idx] & 0X0F;
        en_bandwidth = (pst_cmd->auc_data[uc_idx] >> 4) & 0x0F;
        if (OAL_SUCC != mac_is_channel_num_valid_etc(en_band, pst_cmd->auc_data[uc_idx + 1])) {
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ACS,
                             "{hmac_init_scan_do_etc:invalid channel number, ch=%d, band=%d bw=%d}",
                             pst_cmd->auc_data[uc_idx + 1], en_band, en_bandwidth);
            OAL_IO_PRINT("{hmac_init_scan_do_etc:invalid channel number, ch=%d, band=%d bw=%d}\n",
                         pst_cmd->auc_data[uc_idx + 1], en_band, en_bandwidth);

            continue;
        }

        st_scan_req.ast_channel_list[uc_cnt].uc_chan_number = pst_cmd->auc_data[uc_idx + 1];
        st_scan_req.ast_channel_list[uc_cnt].en_band = en_band;
        st_scan_req.ast_channel_list[uc_cnt].en_bandwidth = en_bandwidth;
        ul_ret = mac_get_channel_idx_from_num_etc(st_scan_req.ast_channel_list[uc_cnt].en_band,
                                                  st_scan_req.ast_channel_list[uc_cnt].uc_chan_number,
                                                  &st_scan_req.ast_channel_list[uc_cnt].uc_chan_idx);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ACS,
                "{hmac_init_scan_do_etc:mac_get_channel_idx_from_num_etc failed en_band:[%d],chan_number:[%d]}",
                st_scan_req.ast_channel_list[uc_cnt].en_band, st_scan_req.ast_channel_list[uc_cnt].uc_chan_number);
        }
        uc_cnt++;
    }

    OAL_IO_PRINT("hmac_init_scan_do_etc::got=5:do scan mode=%d func=0x%x type=%d ch_cnt=%d\n",
                 st_scan_req.en_scan_mode,
                 st_scan_req.uc_scan_func, st_scan_req.en_scan_type, uc_cnt);

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "hmac_init_scan_do_etc::got=5:do scan mode=%d func=0x%x type=%d ch_cnt=%d\n",
                     st_scan_req.en_scan_mode,
                     st_scan_req.uc_scan_func, st_scan_req.en_scan_type, uc_cnt);

    if (uc_cnt != 0) {
        st_scan_req.uc_channel_nums = uc_cnt;

        /* ֱ�ӵ���ɨ��ģ��ɨ������������ */
        ul_ret = hmac_scan_proc_scan_req_event_etc(pst_hmac_vap, &st_scan_req);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                             "hmac_init_scan_do_etc:hmac_scan_add_req failed, ret=%d", ul_ret);
        }
    } else {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "hmac_init_scan_do_etc:no valid channel found, not scan");
    }

    return ul_ret;
}


oal_bool_enum_uint8 hmac_init_scan_skip_channel_etc(hmac_device_stru *pst_hmac_dev,
                                                    wlan_channel_band_enum_uint8 en_band,
                                                    oal_uint8 uc_idx)
{
    /* skip any illegal channel */
    if (OAL_SUCC != mac_is_channel_idx_valid_etc(en_band, uc_idx)) {
        return OAL_TRUE;
    }
    if (OAL_UNLIKELY(pst_hmac_dev->pst_device_base_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_init_scan_skip_channel_etc::pst_device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hmac_dev->pst_device_base_info
        && pst_hmac_dev->pst_device_base_info->en_max_band != en_band) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_uint32 hmac_init_scan_process_etc(hmac_device_stru *pst_hmac_dev,
                                      mac_vap_stru *pst_mac_vap,
                                      hmac_acs_cfg_stru *pst_acs_cfg)
{
    oal_uint8 ast_buf[OAL_SIZEOF(mac_acs_cmd_stru) - 4 + WLAN_MAX_CHANNEL_NUM * 2];
    oal_uint8 uc_idx;
    oal_uint8 *puc_dat, uc_tot, uc_chan_num;
    oal_uint32 ul_ret;

    mac_init_scan_req_stru *pst_cmd = (mac_init_scan_req_stru *)ast_buf;

    puc_dat = pst_cmd->auc_data;

    pst_hmac_dev->en_init_scan = (pst_acs_cfg->en_scan_op == MAC_SCAN_OP_INIT_SCAN) ? OAL_TRUE : OAL_FALSE;
    pst_hmac_dev->en_in_init_scan = OAL_TRUE;

    pst_cmd->uc_cmd = 5;  // DMAC_ACS_CMD_DO_SCAN;
    pst_cmd->auc_arg[0] = (pst_acs_cfg->en_scan_op == MAC_SCAN_OP_FG_SCAN_ONLY ||
        pst_acs_cfg->en_scan_op == MAC_SCAN_OP_INIT_SCAN) ? WLAN_SCAN_MODE_FOREGROUND : WLAN_SCAN_MODE_BACKGROUND_AP;

    pst_cmd->auc_arg[1] = WLAN_SCAN_TYPE_ACTIVE;
    pst_cmd->auc_arg[2] = MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS | MAC_SCAN_FUNC_BSS;
    pst_cmd->ul_cmd_cnt = 0;

    uc_tot = 0;
    for (uc_idx = 0; uc_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_idx++) {
        if (!hmac_init_scan_skip_channel_etc(pst_hmac_dev, WLAN_BAND_2G, uc_idx)) {
            mac_get_channel_num_from_idx_etc(WLAN_BAND_2G, uc_idx, &uc_chan_num);
            *puc_dat++ = ((WLAN_BAND_WIDTH_20M) << 4) | WLAN_BAND_2G;
            *puc_dat++ = uc_chan_num;
            uc_tot++;
        }
    }
    for (uc_idx = 0; uc_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_idx++) {
        if (!hmac_init_scan_skip_channel_etc(pst_hmac_dev, WLAN_BAND_5G, uc_idx)) {
            mac_get_channel_num_from_idx_etc(WLAN_BAND_5G, uc_idx, &uc_chan_num);
            *puc_dat++ = ((WLAN_BAND_WIDTH_20M) << 4) | WLAN_BAND_5G;
            *puc_dat++ = uc_chan_num;
            uc_tot++;
        }
    }

    pst_cmd->ul_cmd_len = uc_tot * 2;

    /* best�ŵ����Ϊ0����start bss��Ϊ�Ϸ����ж����� */
    if (pst_hmac_dev->en_init_scan) {
        memset_s(pst_hmac_dev->ast_best_channel, OAL_SIZEOF(pst_hmac_dev->ast_best_channel),
                 0, OAL_SIZEOF(pst_hmac_dev->ast_best_channel));
    }

    /* ������ʱ��, ��ʱ��ǿ������BSS: ����ʱ�����ɨ��ʱ����APP����ʱ�� */
    {
        FRW_TIMER_CREATE_TIMER(&pst_hmac_dev->st_scan_mgmt.st_init_scan_timeout,
                               hmac_init_scan_timeout_etc,
                               HMAC_INIT_SCAN_TIMEOUT_MS,
                               pst_hmac_dev,
                               OAL_FALSE,
                               OAM_MODULE_ID_HMAC,
                               pst_hmac_dev->pst_device_base_info->ul_core_id);
    }

    ul_ret = hmac_init_scan_do_etc(pst_hmac_dev, pst_mac_vap, pst_cmd, pst_acs_cfg);
    if (ul_ret != OAL_SUCC) {
        pst_hmac_dev->en_init_scan = OAL_FALSE;
        pst_hmac_dev->en_in_init_scan = OAL_FALSE;
    }

    return ul_ret;
}


oal_uint32 hmac_init_scan_try_etc(mac_device_stru *pst_mac_device, mac_vap_stru *pst_in_mac_vap,
                                  mac_try_init_scan_type en_type, hmac_acs_cfg_stru *pst_acs_cfg)
{
    oal_uint8 uc_idx;
    mac_vap_stru *pst_mac_vap;
    mac_vap_stru *pst_mac_vap_scan = NULL;
    hmac_vap_stru *pst_hmac_vap;
    oal_uint8 en_scan_type;
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_device->uc_device_id);
    oal_bool_enum_uint8 en_need_do_init_scan = OAL_FALSE;

    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_init_scan_try_etc:mac_res_get_hmac_vap failed. device_id:%d.}",
                         pst_mac_device->uc_device_id);
        return OAL_FAIL;
    }
    en_scan_type = hmac_need_init_scan_etc(pst_hmac_device, pst_in_mac_vap, en_type);
    if (en_scan_type == MAC_INIT_SCAN_NOT_NEED) {
        return OAL_FAIL;
    } else if (en_scan_type == MAC_INIT_SCAN_IN_SCAN) {
        OAL_IO_PRINT("just in init scan\n");

        mac_vap_init_rates_etc(pst_in_mac_vap);
        pst_hmac_vap = mac_res_get_hmac_vap(pst_in_mac_vap->uc_vap_id);
        if (pst_hmac_vap == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{hmac_init_scan_try_etc:mac_res_get_hmac_vap failed vap_id:%d.}",
                             pst_in_mac_vap->uc_vap_id);
            return OAL_FAIL;
        }
        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) {
            hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_AP_WAIT_START);
        }

        return OAL_SUCC;
    } else {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "hmac_init_scan_try_etc: need init scan");
    }

    for (uc_idx = 0; uc_idx < pst_hmac_device->pst_device_base_info->uc_vap_num; uc_idx++) {
        pst_mac_vap = mac_res_get_mac_vap(pst_hmac_device->pst_device_base_info->auc_vap_id[uc_idx]);
        if (!pst_mac_vap) {
            continue;
        }

        if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            mac_vap_init_rates_etc(pst_mac_vap);

            /* ǿ������AP��״̬��Ϊ WAIT_START����Ϊ��Ҫִ�г�ʼ�ŵ���� */
            pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
            if (pst_hmac_vap == OAL_PTR_NULL) {
                OAM_WARNING_LOG1(0, OAM_SF_SCAN,
                                 "{hmac_init_scan_try_etc::need init scan fail.vap_id = %u}",
                                 pst_mac_vap->uc_vap_id);
                continue;
            }
            switch (pst_hmac_vap->st_vap_base_info.en_vap_state) {
                case MAC_VAP_STATE_UP:
                case MAC_VAP_STATE_PAUSE:  // dbac
                    hmac_fsm_change_state_etc(pst_hmac_vap, MAC_VAP_STATE_AP_WAIT_START);
                // no break here!
                case MAC_VAP_STATE_AP_WAIT_START:
                    en_need_do_init_scan = OAL_TRUE;
                    pst_mac_vap_scan = pst_mac_vap;
                    break;
                default:
                    break;
            }
        }
    }

    if (en_need_do_init_scan) {
        return hmac_init_scan_process_etc(pst_hmac_device, pst_mac_vap_scan, pst_acs_cfg);
    }

    return OAL_SUCC;
}

#endif
/*lint -e578*/ /*lint -e19*/
oal_module_symbol(hmac_scan_find_scanned_bss_dscr_by_index_etc);
oal_module_symbol(hmac_scan_find_scanned_bss_by_bssid_etc);
/*lint -e578*/ /*lint -e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
