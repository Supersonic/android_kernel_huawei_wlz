

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 ͷ�ļ����� */
#include "oal_profiling.h"
#include "oal_net.h"
#include "frw_ext_if.h"
#include "hmac_tx_data.h"
#include "hmac_tx_amsdu.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_frag.h"
#include "hmac_11i.h"

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
#include "hmac_traffic_classify.h"
#endif

#include "hmac_crypto_tkip.h"
#include "hmac_device.h"
#include "hmac_resource.h"

#include "hmac_tcp_opt.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
#include "hmac_wds.h"
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
#include "hmac_car.h"
#endif
#include "mem_trace.h"

#include "hmac_auto_adjust_freq.h"  // Ϊ����ͳ������׼��

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_DATA_C
/*
 * definitions of king of games feature
 */
#ifdef CONFIG_NF_CONNTRACK_MARK
#define VIP_APP_VIQUE_TID WLAN_TIDNO_VIDEO
#define VIP_APP_MARK      0x5a
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#define PKTMARK(p)       (((struct sk_buff *)(p))->mark)
#define PKTSETMARK(p, m) ((struct sk_buff *)(p))->mark = (m)
#else /* !2.6.0 */
#define PKTMARK(p)       (((struct sk_buff *)(p))->nfmark)
#define PKTSETMARK(p, m) ((struct sk_buff *)(p))->nfmark = (m)
#endif /* 2.6.0 */
#else  /* CONFIG_NF_CONNTRACK_MARK */
#define PKTMARK(p) 0
#define PKTSETMARK(p, m)
#endif /* CONFIG_NF_CONNTRACK_MARK */

/* 2 ȫ�ֱ������� */
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
OAL_STATIC oal_uint8 g_uc_ac_new = 0;
#endif

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
hmac_tx_ampdu_hw_stru g_st_ampdu_hw = { 0 };
#endif

/* 3 ����ʵ�� */

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_is_dhcp(mac_ether_header_stru *pst_ether_hdr)
{
    mac_ip_header_stru *puc_ip_hdr;

    puc_ip_hdr = (mac_ip_header_stru *)(pst_ether_hdr + 1);

    return mac_is_dhcp_port_etc(puc_ip_hdr);
}


OAL_STATIC oal_void hmac_tx_report_dhcp_and_arp(mac_vap_stru *pst_mac_vap,
                                                mac_ether_header_stru *pst_ether_hdr,
                                                oal_uint16 us_ether_len)
{
    oal_bool_enum_uint8 en_flg = OAL_FALSE;

    switch (OAL_HOST2NET_SHORT(pst_ether_hdr->us_ether_type)) {
        case ETHER_TYPE_ARP:
            en_flg = OAL_TRUE;
            break;

        case ETHER_TYPE_IP:
            en_flg = hmac_tx_is_dhcp(pst_ether_hdr);
            break;

        default:
            en_flg = OAL_FALSE;
            break;
    }

    if (en_flg && oam_report_dhcp_arp_get_switch_etc()) {
        if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            oam_report_eth_frame_etc(pst_ether_hdr->auc_ether_dhost,
                                     (oal_uint8 *)pst_ether_hdr, us_ether_len,
                                     OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            oam_report_eth_frame_etc(pst_mac_vap->auc_bssid,
                                     (oal_uint8 *)pst_ether_hdr, us_ether_len,
                                     OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        } else {
        }
    }
}


oal_uint32 hmac_tx_report_eth_frame_etc(mac_vap_stru *pst_mac_vap,
                                        oal_netbuf_stru *pst_netbuf)
{
    oal_uint16 us_user_idx = 0;
    mac_ether_header_stru *pst_ether_hdr = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 auc_user_macaddr[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_switch_enum_uint8 en_eth_switch = 0;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_mac_vap, pst_netbuf))) {
        OAM_ERROR_LOG2(0, OAM_SF_TX,
                       "{hmac_tx_report_eth_frame_etc::input null %x %x}",
                       (uintptr_t)pst_mac_vap, (uintptr_t)pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_TX,
                       "{hmac_tx_report_eth_frame_etc::mac_res_get_hmac_vap fail. vap_id = %u}",
                       pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ͳ����̫�����������ݰ�ͳ�� */
    /* �޸����� ��1)����pst_hmac_vap ��֮ǰ����ifdef ����ġ�2)ul_rx_pkt_to_lan Ӧ����ul_rx_bytes_to_lan��ͳ���ֽ��� */
#ifdef _PRE_WLAN_DFT_STAT
    HMAC_VAP_DFT_STATS_PKT_INCR(pst_hmac_vap->st_query_stats.ul_rx_pkt_to_lan, 1);
    HMAC_VAP_DFT_STATS_PKT_INCR(pst_hmac_vap->st_query_stats.ul_rx_bytes_to_lan, OAL_NETBUF_LEN(pst_netbuf));
#endif
    OAM_STAT_VAP_INCR(pst_mac_vap->uc_vap_id, tx_pkt_num_from_lan, 1);
    OAM_STAT_VAP_INCR(pst_mac_vap->uc_vap_id, tx_bytes_from_lan, OAL_NETBUF_LEN(pst_netbuf));

    /* ��ȡĿ���û���Դ��id���û�MAC��ַ�����ڹ��� */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
        if (OAL_UNLIKELY(pst_ether_hdr == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_tx_report_eth_frame_etc::ether_hdr is null!\r\n");
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, pst_ether_hdr->auc_ether_dhost, &us_user_idx);
        if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_TX, "{hmac_tx_report_eth_frame_etc::find user return ptr null!!\r\n", ul_ret);
            return ul_ret;
        }

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
        if (ul_ret != OAL_SUCC) {
            ul_ret = hmac_find_valid_user_by_wds_sta(pst_hmac_vap, pst_ether_hdr->auc_ether_dhost, &us_user_idx);
        }
#endif

        if (ul_ret == OAL_FAIL) {
            /* ����Ҳ����û�����֡������dhcp����arp request����Ҫ�ϱ� */
            hmac_tx_report_dhcp_and_arp(pst_mac_vap, pst_ether_hdr, (oal_uint16)OAL_NETBUF_LEN(pst_netbuf));
            return OAL_SUCC;
        }

        oal_set_mac_addr(&auc_user_macaddr[0], pst_ether_hdr->auc_ether_dhost);
    }

    else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_mac_vap->us_user_nums == 0) {
            return OAL_SUCC;
        }
#if 1
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
        if (OAL_UNLIKELY(pst_ether_hdr == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_tx_report_eth_frame_etc::ether_hdr is null!\r\n");
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* ����Ҳ����û�����֡������dhcp����arp request����Ҫ�ϱ� */
        hmac_tx_report_dhcp_and_arp(pst_mac_vap, pst_ether_hdr, (oal_uint16)OAL_NETBUF_LEN(pst_netbuf));

#endif
        us_user_idx = pst_mac_vap->us_assoc_vap_id;
        oal_set_mac_addr(&auc_user_macaddr[0], pst_mac_vap->auc_bssid);
    }

    /* ����ӡ��̫��֡�Ŀ��� */
    ul_ret = oam_report_eth_frame_get_switch_etc(us_user_idx, OAM_OTA_FRAME_DIRECTION_TYPE_TX, &en_eth_switch);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{hmac_tx_report_eth_frame_etc::get tx eth frame switch fail!\r\n");
        return ul_ret;
    }

    if (en_eth_switch == OAL_SWITCH_ON) {
        /* ����̫��������֡�ϱ� */
        ul_ret = oam_report_eth_frame_etc(&auc_user_macaddr[0],
                                          oal_netbuf_data(pst_netbuf),
                                          (oal_uint16)OAL_NETBUF_LEN(pst_netbuf),
                                          OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, OAM_SF_TX,
                             "{hmac_tx_report_eth_frame_etc::oam_report_eth_frame_etc return err: 0x%x.}\r\n", ul_ret);
        }
    }

    return OAL_SUCC;
}


oal_uint16 hmac_free_netbuf_list_etc(oal_netbuf_stru *pst_buf)
{
    oal_netbuf_stru *pst_buf_tmp = OAL_PTR_NULL;
    mac_tx_ctl_stru *pst_tx_cb = OAL_PTR_NULL;
    oal_uint16 us_buf_num = 0;

    if (pst_buf != OAL_PTR_NULL) {
        pst_tx_cb = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_buf);

        while (pst_buf != OAL_PTR_NULL) {
            pst_buf_tmp = oal_netbuf_list_next(pst_buf);
            us_buf_num++;

            pst_tx_cb = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_buf);
            
            if ((oal_netbuf_headroom(pst_buf) < MAC_80211_QOS_HTC_4ADDR_FRAME_LEN) &&
                (MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb) != OAL_PTR_NULL)) {
                OAL_MEM_FREE(MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb), OAL_TRUE);
                MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb) = OAL_PTR_NULL;
            }

            oal_netbuf_free(pst_buf);

            pst_buf = pst_buf_tmp;
        }
    } else {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_free_netbuf_list_etc::pst_buf is null}");
    }

    return us_buf_num;
}

#ifdef _PRE_WLAN_FEATURE_HS20

oal_void hmac_tx_set_qos_map(oal_netbuf_stru *pst_buf, oal_uint8 *puc_tid)
{
    mac_ether_header_stru *pst_ether_header;
    mac_ip_header_stru *pst_ip;
    oal_uint8 uc_dscp;
    mac_tx_ctl_stru *pst_tx_ctl;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint8 uc_idx;

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctl));

    /* ��ȡ��̫��ͷ */
    pst_ether_header = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);

    /* �����Ϸ��Լ�� */
    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, pst_ether_header)) {
        OAM_ERROR_LOG0(0, OAM_SF_HS20,
            "{hmac_tx_set_qos_map::The input parameter of QoS_Map_Configure_frame_with_QoSMap_Set_element is NULL.}");
        return;
    }

    /* ��IP TOS�ֶ�Ѱ��DSCP���ȼ� */
    /* ---------------------------------
      tosλ����
      ---------------------------------
    |    bit7~bit2      | bit1~bit0 |
    |    DSCP���ȼ�     | ����      |
    --------------------------------- */
    /* ƫ��һ����̫��ͷ��ȡipͷ */
    pst_ip = (mac_ip_header_stru *)(pst_ether_header + 1);
    uc_dscp = pst_ip->uc_tos >> WLAN_DSCP_PRI_SHIFT;
    OAM_INFO_LOG2(0, OAM_SF_HS20, "{hmac_tx_set_qos_map::tos = %d, uc_dscp=%d.}", pst_ip->uc_tos, uc_dscp);

    if ((pst_hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except > 0) &&
        (pst_hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except <= MAX_DSCP_EXCEPT) &&
        (pst_hmac_vap->st_cfg_qos_map_param.uc_valid)) {
        for (uc_idx = 0; uc_idx < pst_hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except; uc_idx++) {
            if (uc_dscp == pst_hmac_vap->st_cfg_qos_map_param.auc_dscp_exception[uc_idx]) {
                *puc_tid = pst_hmac_vap->st_cfg_qos_map_param.auc_dscp_exception_up[uc_idx];
                MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_HS20;
                MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
                pst_hmac_vap->st_cfg_qos_map_param.uc_valid = 0;
                return;
            }
        }
    }

    for (uc_idx = 0; uc_idx < MAX_QOS_UP_RANGE; uc_idx++) {
        if ((uc_dscp < pst_hmac_vap->st_cfg_qos_map_param.auc_up_high[uc_idx]) &&
            (uc_dscp > pst_hmac_vap->st_cfg_qos_map_param.auc_up_low[uc_idx])) {
            *puc_tid = uc_idx;
            MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
            MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_HS20;
            MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
            pst_hmac_vap->st_cfg_qos_map_param.uc_valid = 0;
            return;
        } else {
            *puc_tid = 0;
        }
    }
    pst_hmac_vap->st_cfg_qos_map_param.uc_valid = 0;
    return;
}
#endif  // _PRE_WLAN_FEATURE_HS20

#ifdef _PRE_WLAN_FEATURE_CLASSIFY

OAL_STATIC OAL_INLINE oal_void hmac_tx_classify_lan_to_wlan(oal_netbuf_stru *pst_buf, oal_uint8 *puc_tid)
{
    mac_ether_header_stru *pst_ether_header = OAL_PTR_NULL;
    mac_ip_header_stru *pst_ip = OAL_PTR_NULL;
    oal_vlan_ethhdr_stru *pst_vlan_ethhdr = OAL_PTR_NULL;
    oal_uint32 ul_ipv6_hdr;
    oal_uint32 ul_pri;
    oal_uint16 us_vlan_tci;
    oal_uint8 uc_tid = 0;
    oal_uint8 uc_dscp;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
#endif
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    mac_tcp_header_stru *pst_tcp;
#endif
#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
    hmac_m2u_stru *pst_m2u;
#endif

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctl));
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_TX,
                         "{hmac_tx_classify_lan_to_wlan::mac_res_get_hmac_vap fail.vap_index[%u]}",
                         MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctl));
        return;
    }

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
    pst_m2u = (hmac_m2u_stru *)(pst_hmac_vap->pst_m2u);
    if ((pst_tx_ctl->bit_is_m2u_data) && (pst_m2u != OAL_PTR_NULL)) {
        *puc_tid = pst_m2u->en_tid_num;
        return;
    }
#endif
#ifdef CONFIG_NF_CONNTRACK_MARK
    /*
     * the king of game feature will mark packets
     * and we will use VI queue to send these packets.
     */
    if (PKTMARK(pst_buf) == VIP_APP_MARK) {
        *puc_tid = VIP_APP_VIQUE_TID;
        pst_tx_ctl->bit_is_needretry = OAL_TRUE;
        return;
    }
#endif

    /* ��ȡ��̫��ͷ */
    pst_ether_header = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);

    switch (pst_ether_header->us_ether_type) {
            /*lint -e778*/ /* ����Info-- Constant expression evaluates to 0 in operation '&' */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP) :
            OAM_INFO_LOG0(0, OAM_SF_TX, "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_IP.}");

#ifdef _PRE_WLAN_FEATURE_HS20
            if (pst_hmac_vap->st_cfg_qos_map_param.uc_valid) {
            hmac_tx_set_qos_map(pst_buf, &uc_tid);
            *puc_tid = uc_tid;
            return;
            }
#endif  // _PRE_WLAN_FEATURE_HS20

            /* ��IP TOS�ֶ�Ѱ�����ȼ� */
            /* ----------------------------------------------------------------------
                        tosλ����
               ----------------------------------------------------------------------
                    | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
                    | �����ȼ�  | ʱ�� | ������ | �ɿ��� | ����ɱ� | ���� |
               ---------------------------------------------------------------------- */
            pst_ip = (mac_ip_header_stru *)(pst_ether_header + 1); /* ƫ��һ����̫��ͷ��ȡipͷ */

            uc_dscp = pst_ip->uc_tos >> WLAN_DSCP_PRI_SHIFT;
            if (pst_hmac_vap->auc_dscp_tid_map[uc_dscp] != HMAC_DSCP_VALUE_INVALID) {
                uc_tid = pst_hmac_vap->auc_dscp_tid_map[uc_dscp];
                break;
            }

            uc_tid = pst_ip->uc_tos >> WLAN_IP_PRI_SHIFT;

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
            if (OAL_SWITCH_ON == mac_mib_get_TxTrafficClassifyFlag(&pst_hmac_vap->st_vap_base_info)) {
                if (uc_tid != 0) {
                    break;
                }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                /* RTP RTSP ����ֻ�� P2P�ϲſ���ʶ���� */
                if (!IS_LEGACY_VAP(&(pst_hmac_vap->st_vap_base_info)))
#endif
                {
                    hmac_tx_traffic_classify_etc(pst_tx_ctl, pst_ip, &uc_tid);
                }
            }
#endif /* _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN */

            OAM_INFO_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::tos = %d, uc_tid=%d.}",
                          pst_ip->uc_tos, uc_tid);
            /* �����DHCP֡�������VO���з��� */
            if (OAL_TRUE == mac_is_dhcp_port_etc(pst_ip)) {
                uc_tid = WLAN_DATA_VIP_TID;
                MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_DHCP;
                MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
            }
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
            /* ����chariot����Ľ������⴦������ֹ���� */
            else if (pst_ip->uc_protocol == MAC_TCP_PROTOCAL) {
                pst_tcp = (mac_tcp_header_stru *)(pst_ip + 1);

                if ((OAL_NTOH_16(pst_tcp->us_dport) == MAC_CHARIOT_NETIF_PORT) ||
                    (OAL_NTOH_16(pst_tcp->us_sport) == MAC_CHARIOT_NETIF_PORT)) {
                    OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                                  "{hmac_tx_classify_lan_to_wlan::chariot netif tcp pkt.}");
                    uc_tid = WLAN_DATA_VIP_TID;
                    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_CHARIOT_SIG;
                    MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
                }
            }
#endif

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
            pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
            if (OAL_UNLIKELY(pst_hmac_user == OAL_PTR_NULL)) {
                OAM_WARNING_LOG1(0, OAM_SF_CFG,
                                 "{hmac_edca_opt_rx_pkts_stat_etc::null param,pst_hmac_user[%d].}",
                                 MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
                break;
            }

            if ((pst_hmac_vap->uc_edca_opt_flag_ap == OAL_TRUE) &&
                (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
                /* mips�Ż�:�������ҵ��ͳ�����ܲ�10M���� */
                if (((pst_ip->uc_protocol == MAC_UDP_PROTOCAL) &&
                     (pst_hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(uc_tid)][WLAN_TX_UDP_DATA] <
                      (HMAC_EDCA_OPT_PKT_NUM + 10))) ||
                    ((pst_ip->uc_protocol == MAC_TCP_PROTOCAL) &&
                     (pst_hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(uc_tid)][WLAN_TX_TCP_DATA] <
                      (HMAC_EDCA_OPT_PKT_NUM + 10)))) {
                    hmac_edca_opt_tx_pkts_stat_etc(pst_tx_ctl, uc_tid, pst_ip);
                }
            }
#endif
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6):
            OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_IPV6.}");
            /* ��IPv6 traffic class�ֶλ�ȡ���ȼ� */
            /* ----------------------------------------------------------------------
                IPv6��ͷ ǰ32Ϊ����
             -----------------------------------------------------------------------
            | �汾�� | traffic class   | ������ʶ |
            | 4bit   | 8bit(ͬipv4 tos)|  20bit   |
            ----------------------------------------------------------------------- */
            ul_ipv6_hdr = *((oal_uint32 *)(pst_ether_header + 1)); /* ƫ��һ����̫��ͷ��ȡipͷ */

            ul_pri = (OAL_NET2HOST_LONG(ul_ipv6_hdr) & WLAN_IPV6_PRIORITY_MASK) >> WLAN_IPV6_PRIORITY_SHIFT;

            uc_tid = (oal_uint8)(ul_pri >> WLAN_IP_PRI_SHIFT);
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::uc_tid=%d.}", uc_tid);

            /* �����DHCPV6֡�������VO���з��� */
            if (OAL_TRUE == mac_is_dhcp6_etc((oal_ipv6hdr_stru *)(pst_ether_header + 1))) {
                OAM_INFO_LOG0(0, OAM_SF_TX, "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_DHCP6.}");
                uc_tid = WLAN_DATA_VIP_TID;
                MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_DHCPV6;
                MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
            }
            break;

        case OAL_HOST2NET_SHORT(ETHER_TYPE_PAE):
            /* �����EAPOL֡�������VO���з��� */
            OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{hmac_tx_classify_lan_to_wlan::trace_eapol::ETHER_TYPE_PAE.}");
            uc_tid = WLAN_DATA_VIP_TID;
            MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
            MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_EAPOL;
            MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;

            /* �����4 ���������õ�����Կ��������tx cb ��bit_is_eapol_key_ptk ��һ��dmac ���Ͳ����� */
            if (OAL_TRUE == mac_is_eapol_key_ptk_etc((mac_eapol_header_stru *)(pst_ether_header + 1))) {
                MAC_GET_CB_IS_EAPOL_KEY_PTK(pst_tx_ctl) = OAL_TRUE;
            }

            OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CONN,
                             "{hmac_tx_classify_lan_to_wlan:: EAPOL tx : uc_tid=%d,IS_EAPOL_KEY_PTK=%d.}",
                             uc_tid, MAC_GET_CB_IS_EAPOL_KEY_PTK(pst_tx_ctl));

            break;

        /* TDLS֡����������������������ȼ�TID���� */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_TDLS):
            OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_TDLS.}");
            uc_tid = WLAN_DATA_VIP_TID;
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::uc_tid=%d.}", uc_tid);
            break;

        /* PPPOE֡��������������(���ֽ׶�, �Ự�׶�)��������ȼ�TID���� */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PPP_DISC):
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PPP_SES):
            OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_PPP_DISC, ETHER_TYPE_PPP_SES.}");
            uc_tid = WLAN_DATA_VIP_TID;
            MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
            MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_PPPOE;
            MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;

            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::uc_tid=%d.}", uc_tid);
            break;

#ifdef _PRE_WLAN_FEATURE_WAPI
        case OAL_HOST2NET_SHORT(ETHER_TYPE_WAI):
            OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_WAI.}");
            uc_tid = WLAN_DATA_VIP_TID;
            MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
            MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = MAC_DATA_WAPI;
            MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctl) = OAL_TRUE;
            break;
#endif

        case OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN):
            OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::ETHER_TYPE_VLAN.}");
            /* ��ȡvlan tag�����ȼ� */
            pst_vlan_ethhdr = (oal_vlan_ethhdr_stru *)oal_netbuf_data(pst_buf);

            /* ------------------------------------------------------------------
                802.1Q(VLAN) TCI(tag control information)λ����
             -------------------------------------------------------------------
            |Priority | DEI  | Vlan Identifier |
            | 3bit    | 1bit |      12bit      |
             ------------------------------------------------------------------ */
            us_vlan_tci = OAL_NET2HOST_SHORT(pst_vlan_ethhdr->h_vlan_TCI);

            uc_tid = us_vlan_tci >> OAL_VLAN_PRIO_SHIFT; /* ����13λ����ȡ��3λ���ȼ� */
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::uc_tid=%d.}", uc_tid);

            break;
        /*lint +e778*/
        default:
            OAM_INFO_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                          "{hmac_tx_classify_lan_to_wlan::default us_ether_type[%d].}",
                          pst_ether_header->us_ether_type);
            break;
    }

    /* ���θ�ֵ */
    *puc_tid = uc_tid;
}


OAL_STATIC OAL_INLINE oal_void hmac_tx_update_tid(oal_bool_enum_uint8 en_wmm, oal_uint8 *puc_tid)
{
    if (OAL_LIKELY(en_wmm == OAL_TRUE)) { /* wmmʹ�� */
        *puc_tid = (*puc_tid < WLAN_TIDNO_BUTT) ? WLAN_TOS_TO_TID(*puc_tid) : WLAN_TIDNO_BCAST;
    } else { /* wmm��ʹ�� */
        *puc_tid = MAC_WMM_SWITCH_TID;
    }
}


oal_uint8 hmac_tx_wmm_acm_etc(oal_bool_enum_uint8 en_wmm, hmac_vap_stru *pst_hmac_vap, oal_uint8 *puc_tid)
{
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    oal_uint8 uc_ac;

    if (OAL_ANY_NULL_PTR2(pst_hmac_vap, puc_tid)) {
        return OAL_FALSE;
    }

    if (en_wmm == OAL_FALSE) {
        return OAL_FALSE;
    }

    uc_ac = WLAN_WME_TID_TO_AC(*puc_tid);
    g_uc_ac_new = uc_ac;
    while ((g_uc_ac_new != WLAN_WME_AC_BK) &&
           (OAL_TRUE == mac_mib_get_QAPEDCATableMandatory(&pst_hmac_vap->st_vap_base_info, g_uc_ac_new))) {
        switch (g_uc_ac_new) {
            case WLAN_WME_AC_VO:
                g_uc_ac_new = WLAN_WME_AC_VI;
                break;

            case WLAN_WME_AC_VI:
                g_uc_ac_new = WLAN_WME_AC_BE;
                break;

            default:
                g_uc_ac_new = WLAN_WME_AC_BK;
                break;
        }
    }

    if (g_uc_ac_new != uc_ac) {
        *puc_tid = WLAN_WME_AC_TO_TID(g_uc_ac_new);
    }
#endif /* defined(_PRE_PRODUCT_ID_HI110X_HOST) */

    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_void hmac_tx_classify(hmac_vap_stru *pst_hmac_vap,
                                                mac_user_stru *pst_user,
                                                oal_netbuf_stru *pst_buf)
{
    oal_uint8 uc_tid = 0;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    mac_device_stru *pst_mac_dev = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_11AX_BUGFIX
    mac_vap_rom_stru *pst_mac_vap_rom;
#endif

    /* ��qos��ͬ����Ҫ��EAPOL����ҵ��ʶ�� */
    hmac_tx_classify_lan_to_wlan(pst_buf, &uc_tid);

    /* ��QoSվ�㣬ֱ�ӷ��� */
    if (OAL_UNLIKELY(pst_user->st_cap_info.bit_qos != OAL_TRUE)) {
        OAM_INFO_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                      "{hmac_tx_classify::user is a none QoS station.}");
        return;
    }

    pst_mac_dev = mac_res_get_dev_etc(pst_user->uc_device_id);
    if (OAL_UNLIKELY(pst_mac_dev == OAL_PTR_NULL)) {
        OAM_WARNING_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_tx_classify::pst_mac_dev null.}");
        return;
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_WMMAC)
    if (g_en_wmmac_switch_etc) {
        oal_uint8 uc_ac_num;
        uc_ac_num = WLAN_WME_TID_TO_AC(uc_tid);
        /* ���ACMλΪ1���Ҷ�ӦAC��TSû�н����ɹ����򽫸�AC������ȫ���ŵ�BE���з��� */
        if ((OAL_TRUE == mac_mib_get_QAPEDCATableMandatory(&(pst_hmac_vap->st_vap_base_info), uc_ac_num)) &&
            (pst_user->st_ts_info[uc_ac_num].en_ts_status != MAC_TS_SUCCESS) &&
            (MAC_GET_CB_IS_VIPFRAME(pst_tx_ctl) == OAL_FALSE)) {
            uc_tid = WLAN_WME_AC_BE;
        }
    } else
#endif  // #if defined(_PRE_PRODUCT_ID_HI110X_HOST) && defined(_PRE_WLAN_FEATURE_WMMAC)
    {
        hmac_tx_wmm_acm_etc(pst_mac_dev->en_wmm, pst_hmac_vap, &uc_tid);

        
        /* 1102��������ֻʹ����4��tid:0 1 5 6 */
        if ((!MAC_GET_CB_IS_VIPFRAME(pst_tx_ctl)) || (pst_mac_dev->en_wmm == OAL_FALSE)) {
            hmac_tx_update_tid(pst_mac_dev->en_wmm, &uc_tid);
        }
    }

    /* ���ʹ����vap���ȼ�����������õ�vap���ȼ� */
    if (pst_mac_dev->en_vap_classify == OAL_TRUE) {
        uc_tid = mac_mib_get_VAPClassifyTidNo(&pst_hmac_vap->st_vap_base_info);
    }

    /* ���ap�ر���WMM�������б�����BE ���� */
    if (!pst_hmac_vap->st_vap_base_info.en_vap_wmm) {
        uc_tid = WLAN_TIDNO_BEST_EFFORT;
    }

    /* ����ac��tid��cb�ֶ� */
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = uc_tid;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_TID_TO_AC(uc_tid);

    return;
}
#endif


oal_void hmac_rx_dyn_bypass_extlna_switch(oal_uint32 ul_tx_throughput_mbps, oal_uint32 ul_rx_throughput_mbps)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32 ul_limit_throughput_high;
    oal_uint32 ul_limit_throughput_low;
    oal_uint32 ul_throughput_mbps = OAL_MAX(ul_tx_throughput_mbps, ul_rx_throughput_mbps);
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_is_pm_test;

    /* ������ƻ���֧�ָ�������bypass����LNA */
    if (g_st_rx_dyn_bypass_extlna_switch.uc_ini_en == OAL_FALSE) {
        return;
    }

    /* ÿ������������ */
    if ((g_st_rx_dyn_bypass_extlna_switch.us_throughput_high != 0) &&
        (g_st_rx_dyn_bypass_extlna_switch.us_throughput_low != 0)) {
        ul_limit_throughput_high = g_st_rx_dyn_bypass_extlna_switch.us_throughput_high;
        ul_limit_throughput_low = g_st_rx_dyn_bypass_extlna_switch.us_throughput_low;
    } else {
        ul_limit_throughput_high = WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_HIGH;
        ul_limit_throughput_low = WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_LOW;
    }
    if (ul_throughput_mbps > ul_limit_throughput_high) {
        /* ����100M,�ǵ͹��Ĳ��Գ��� */
        en_is_pm_test = OAL_FALSE;
    } else if (ul_throughput_mbps < ul_limit_throughput_low) {
        /* ����50M,�͹��Ĳ��Գ��� */
        en_is_pm_test = OAL_TRUE;
    } else {
        /* ����50M-100M֮��,�����л� */
        return;
    }

    /* ��Ҫ�л�ʱ������������֪ͨdevice���� */
    pst_mac_device = mac_res_get_dev_etc(0);

    /* ����ǵ�VAP,�򲻴��� */
    if (1 != mac_device_calc_up_vap_num_etc(pst_mac_device)) {
        return;
    }

    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap);
    if ((ul_ret != OAL_SUCC) || (pst_mac_vap == OAL_PTR_NULL)) {
        return;
    }

    /* ��ǰ��ʽ��ͬ,������ */
    if (g_st_rx_dyn_bypass_extlna_switch.uc_cur_status == en_is_pm_test) {
        return;
    }

    ul_ret = hmac_config_send_event_etc(pst_mac_vap, WLAN_CFGID_DYN_EXTLNA_BYPASS_SWITCH,
                                        OAL_SIZEOF (oal_uint8), (oal_uint8 *)(&en_is_pm_test));
    if (ul_ret == OAL_SUCC) {
        g_st_rx_dyn_bypass_extlna_switch.uc_cur_status = en_is_pm_test;
    }

    OAM_WARNING_LOG4(0, OAM_SF_ANY,
        "{hmac_rx_dyn_bypass_extlna_switch: limit_high:%d,limit_low:%d,throughput:%d,cur_status:%d(0:not pm, 1:pm))!}",
        ul_limit_throughput_high, ul_limit_throughput_low, ul_throughput_mbps, en_is_pm_test);
}

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU

OAL_STATIC oal_uint32 hmac_set_amsdu_num_based_protocol(mac_device_stru *pst_mac_device,
                                             wlan_tx_amsdu_enum_uint8 *pen_large_amsdu_ampdu)
{
    mac_vap_stru *pst_mac_vap;
    oal_uint32 ul_ret;

    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap);
    if ((ul_ret != OAL_SUCC) || (pst_mac_vap == OAL_PTR_NULL)) {
        return OAL_FAIL;
    }
    /* Э��涨HT mpdu���Ȳ��ó���4095�ֽ� */
    if ((pst_mac_vap->en_protocol == WLAN_HT_ONLY_MODE) ||
        (pst_mac_vap->en_protocol == WLAN_HT_11G_MODE) ||
        (pst_mac_vap->en_protocol == WLAN_HT_MODE)) {
        *pen_large_amsdu_ampdu = WLAN_TX_AMSDU_BY_2;
    } else if ((pst_mac_vap->en_protocol == WLAN_VHT_MODE) ||
               (pst_mac_vap->en_protocol == WLAN_VHT_ONLY_MODE)
#ifdef _PRE_WLAN_FEATURE_11AX
               || (pst_mac_vap->en_protocol == WLAN_HE_MODE)
#endif
            ) {
        *pen_large_amsdu_ampdu = WLAN_TX_AMSDU_BY_4;
    } else {
        *pen_large_amsdu_ampdu = WLAN_TX_AMSDU_NONE;
    }
    return OAL_SUCC;
}


oal_void hmac_tx_amsdu_ampdu_switch(oal_uint32 ul_tx_throughput_mbps)
{
    mac_device_stru *pst_mac_device;
    oal_uint32 ul_limit_throughput_high;
    oal_uint32 ul_limit_throughput_low;
    wlan_tx_amsdu_enum_uint8 en_large_amsdu_ampdu;
    oal_uint32 ul_ret;
    /* ������ƻ���֧��Ӳ���ۺ� */
    if (g_st_tx_large_amsdu.uc_host_large_amsdu_en == OAL_FALSE) {
        return;
    }

    /* ÿ������������ */
    if ((g_st_tx_large_amsdu.us_amsdu_throughput_high != 0) && (g_st_tx_large_amsdu.us_amsdu_throughput_low != 0)) {
        ul_limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high;
        ul_limit_throughput_low = g_st_tx_large_amsdu.us_amsdu_throughput_low;
    } else {
        ul_limit_throughput_high = WLAN_AMPDU_THROUGHPUT_THRESHOLD_HIGH;
        ul_limit_throughput_low = WLAN_AMPDU_THROUGHPUT_THRESHOLD_LOW;
    }

    pst_mac_device = mac_res_get_dev_etc(0);
    /* ����ǵ�VAP,�򲻿���amsdu */
    if (mac_device_calc_up_vap_num_etc(pst_mac_device) != 1) {
        en_large_amsdu_ampdu = WLAN_TX_AMSDU_NONE;
    } else {
        if (ul_tx_throughput_mbps > ul_limit_throughput_high) {
            /* ����300M,�л�amsdu����ۺ� */
            if (get_hi110x_subchip_type() == BOARD_VERSION_HI1105) {
                ul_ret = hmac_set_amsdu_num_based_protocol(pst_mac_device, &en_large_amsdu_ampdu);
                if (ul_ret != OAL_SUCC) {
                    return;
                }
            } else {
                en_large_amsdu_ampdu = WLAN_TX_AMSDU_BY_2;
            }
        } else if (ul_tx_throughput_mbps < ul_limit_throughput_low) {
            /* ����200M,�ر�amsdu����ۺ� */
            en_large_amsdu_ampdu = WLAN_TX_AMSDU_NONE;
        } else {
            /* ����200M-300M֮��,�����л� */
            return;
        }
    }
    /* ��ǰ�ۺϷ�ʽ��ͬ,������ */
    if (g_st_tx_large_amsdu.en_tx_amsdu_level == en_large_amsdu_ampdu) {
        return;
    }

    g_st_tx_large_amsdu.en_tx_amsdu_level = en_large_amsdu_ampdu;

    OAM_WARNING_LOG4(0, OAM_SF_ANY,
        "{hmac_tx_amsdu_ampdu_switch: amsdu level[%d],limit_high = [%d],limit_low = [%d],tx_throught= [%d]!}",
        en_large_amsdu_ampdu, ul_limit_throughput_high, ul_limit_throughput_low, ul_tx_throughput_mbps);
}
#endif
#ifdef _PRE_WLAN_TCP_OPT

oal_void hmac_tcp_ack_filter_switch(oal_uint32 ul_rx_throughput_mbps)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32 ul_limit_throughput_high;
    oal_uint32 ul_limit_throughput_low;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_tcp_ack_filter;
    /* ������ƻ���֧��tcp ack���˶�̬���� */
    if (g_st_tcp_ack_filter.uc_tcp_ack_filter_en == OAL_FALSE) {
        return;
    }

    /* ÿ������������ */
    if ((g_st_tcp_ack_filter.us_rx_filter_throughput_high != 0) &&
        (g_st_tcp_ack_filter.us_rx_filter_throughput_low != 0)) {
        ul_limit_throughput_high = g_st_tcp_ack_filter.us_rx_filter_throughput_high;
        ul_limit_throughput_low = g_st_tcp_ack_filter.us_rx_filter_throughput_low;
    } else {
        ul_limit_throughput_high = WLAN_TCP_ACK_FILTER_THROUGHPUT_TH_HIGH;
        ul_limit_throughput_low = WLAN_TCP_ACK_FILTER_THROUGHPUT_TH_LOW;
    }
    if (ul_rx_throughput_mbps > ul_limit_throughput_high) {
        /* ����60M, ��tcp ack���� */
        en_tcp_ack_filter = OAL_TRUE;
    } else if (ul_rx_throughput_mbps < ul_limit_throughput_low) {
        /* ����20M,�ر�tcp ack���� */
        en_tcp_ack_filter = OAL_FALSE;
    } else {
        /* ����20M-60M֮��,�����л� */
        return;
    }
    if (g_st_tcp_ack_filter.uc_cur_filter_status == en_tcp_ack_filter) {
        return;
    }

    pst_mac_device = mac_res_get_dev_etc(0);
    /* ����ǵ�VAP,���л� */
    if (1 != mac_device_calc_up_vap_num_etc(pst_mac_device)) {
        return;
    }

    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap);
    if ((ul_ret != OAL_SUCC) || (pst_mac_vap == OAL_PTR_NULL)) {
        return;
    }
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_mac_device->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        return;
    }
    pst_hmac_device->sys_tcp_tx_ack_opt_enable = en_tcp_ack_filter;
    g_st_tcp_ack_filter.uc_cur_filter_status = en_tcp_ack_filter;

    OAM_WARNING_LOG3(0, OAM_SF_ANY,
                     "{hmac_tcp_ack_filter_switch: limit_high = [%d],limit_low = [%d],rx_throught= [%d]!}",
                     ul_limit_throughput_high, ul_limit_throughput_low, ul_rx_throughput_mbps);
}
#endif

oal_void hmac_tx_small_amsdu_switch(oal_uint32 ul_rx_throughput_mbps, oal_uint32 ul_tx_pps)
{
    mac_device_stru *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_cfg_ampdu_tx_on_param_stru st_ampdu_tx_on = { 0 };
    oal_uint32 ul_limit_throughput_high;
    oal_uint32 ul_limit_throughput_low;
    oal_uint32 ul_limit_pps_high;
    oal_uint32 ul_limit_pps_low;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_small_amsdu;

    /* ������ƻ���֧��Ӳ���ۺ� */
    if (g_st_small_amsdu_switch.uc_ini_small_amsdu_en == OAL_FALSE) {
        return;
    }

    /* ÿ������������ */
    if ((g_st_small_amsdu_switch.us_small_amsdu_throughput_high != 0) &&
        (g_st_small_amsdu_switch.us_small_amsdu_throughput_low != 0)) {
        ul_limit_throughput_high = g_st_small_amsdu_switch.us_small_amsdu_throughput_high;
        ul_limit_throughput_low = g_st_small_amsdu_switch.us_small_amsdu_throughput_low;
    } else {
        ul_limit_throughput_high = WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_HIGH;
        ul_limit_throughput_low = WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_LOW;
    }
    /* ÿ��PPS���� */
    if ((g_st_small_amsdu_switch.us_small_amsdu_pps_high != 0) &&
        (g_st_small_amsdu_switch.us_small_amsdu_pps_low != 0)) {
        ul_limit_pps_high = g_st_small_amsdu_switch.us_small_amsdu_pps_high;
        ul_limit_pps_low = g_st_small_amsdu_switch.us_small_amsdu_pps_low;
    } else {
        ul_limit_pps_high = WLAN_SMALL_AMSDU_PPS_THRESHOLD_HIGH;
        ul_limit_pps_low = WLAN_SMALL_AMSDU_PPS_THRESHOLD_LOW;
    }

    if ((ul_rx_throughput_mbps > ul_limit_throughput_high) || (ul_tx_pps > ul_limit_pps_high)) {
        /* rx����������300M����tx pps����25000,��С��amsdu�ۺ� */
        en_small_amsdu = OAL_TRUE;
    } else if ((ul_rx_throughput_mbps < ul_limit_throughput_low) && (ul_tx_pps < ul_limit_pps_low)) {
        /* rx����������200M��tx ppsС��15000,�ر�С��amsdu�ۺϣ����������л� */
        en_small_amsdu = OAL_FALSE;
    } else {
        /* ����200M-300M֮��,�����л� */
        return;
    }

    /* ��ǰ�ۺϷ�ʽ��ͬ,������ */
    if (g_st_small_amsdu_switch.uc_cur_small_amsdu_en == en_small_amsdu) {
        return;
    }

    pst_mac_device = mac_res_get_dev_etc(0);
    /* ����ǵ�VAP,���л� */
    if (1 != mac_device_calc_up_vap_num_etc(pst_mac_device)) {
        return;
    }

    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap);
    if ((ul_ret != OAL_SUCC) || (pst_mac_vap == OAL_PTR_NULL)) {
        return;
    }

    OAM_WARNING_LOG3(0, OAM_SF_ANY,
                     "{hmac_tx_small_amsdu_switch: limit_high = [%d],limit_low = [%d],rx_throught= [%d]!}",
                     ul_limit_throughput_high, ul_limit_throughput_low, ul_rx_throughput_mbps);
    OAM_WARNING_LOG3(0, OAM_SF_ANY,
                     "{hmac_tx_small_amsdu_switch: PPS limit_high = [%d],PPS limit_low = [%d],tx_pps = %d!}",
                     ul_limit_pps_high, ul_limit_pps_low, ul_tx_pps);

    st_ampdu_tx_on.uc_aggr_tx_on = en_small_amsdu;

    g_st_small_amsdu_switch.uc_cur_small_amsdu_en = en_small_amsdu;

    hmac_config_set_amsdu_tx_on_etc(pst_mac_vap,
                                    OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru),
                                    (oal_uint8 *)&st_ampdu_tx_on);
}

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER

oal_void hmac_tx_tcp_ack_buf_switch(oal_uint32 ul_rx_throughput_mbps)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    mac_cfg_tcp_ack_buf_stru st_tcp_ack_param = { 0 };
    oal_uint32 ul_limit_throughput_high = 550;
    oal_uint32 ul_limit_throughput_low = 450;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_tcp_ack_buf = OAL_FALSE;

    /* ������ƻ����ز�֧���л���ֱ�ӷ��� */
    if (g_st_tcp_ack_buf_switch.uc_ini_tcp_ack_buf_en == OAL_FALSE) {
        return;
    }

    pst_mac_device = mac_res_get_dev_etc(0);
    /* ����ǵ�VAP,���л� */
    if (1 != mac_device_calc_up_vap_num_etc(pst_mac_device)) {
        return;
    }
    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap);
    if ((ul_ret != OAL_SUCC) || (pst_mac_vap == OAL_PTR_NULL)) {
        return;
    }
    if (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_20M) {
        /* ÿ������������ */
        if ((g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high != 0) &&
            (g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low != 0)) {
            ul_limit_throughput_high = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high;
            ul_limit_throughput_low = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low;
        } else {
            ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH;
            ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW;
        }
    } else if ((pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
               (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
        /* ÿ������������ */
        if ((g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_40M != 0) &&
            (g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_40M != 0)) {
            ul_limit_throughput_high = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_40M;
            ul_limit_throughput_low = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_40M;
        } else {
            ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M;
            ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_40M;
        }
    } else if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
               (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        /* ÿ������������ */
        if ((g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_80M != 0) &&
            (g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_80M != 0)) {
            ul_limit_throughput_high = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_80M;
            ul_limit_throughput_low = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_80M;
        } else {
            ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M;
            ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_80M;
        }
    }
#ifdef _PRE_WLAN_FEATURE_160M
    else if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
             (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
        /* ÿ������������ */
        if ((g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_160M != 0) &&
            (g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_160M != 0)) {
            ul_limit_throughput_high = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_high_160M;
            ul_limit_throughput_low = g_st_tcp_ack_buf_switch.us_tcp_ack_buf_throughput_low_160M;
        } else {
            ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_160M;
            ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_160M;
        }
    }
#endif
    else {
        return;
    }

    g_st_tcp_ack_buf_switch.us_tcp_ack_smooth_throughput >>= 1;
    g_st_tcp_ack_buf_switch.us_tcp_ack_smooth_throughput += (ul_rx_throughput_mbps >> 1);
    if (g_st_tcp_ack_buf_switch.us_tcp_ack_smooth_throughput > ul_limit_throughput_high) {
        /* ����������,��tcp ack buf */
        en_tcp_ack_buf = OAL_TRUE;
    } else if (g_st_tcp_ack_buf_switch.us_tcp_ack_smooth_throughput < ul_limit_throughput_low) {
        /* ����������,�ر�tcp ack buf */
        en_tcp_ack_buf = OAL_FALSE;
    } else {
        /* ������������֮�䣬 �����л� */
        return;
    }

    /* δ�����仯 ,������ */
    if (g_st_tcp_ack_buf_switch.uc_cur_tcp_ack_buf_en == en_tcp_ack_buf) {
        return;
    }

    OAM_WARNING_LOG4(0, OAM_SF_ANY,
        "{hmac_tx_tcp_ack_buf_switch: limit_high = [%d],limit_low = [%d],rx_throught= [%d]! en_tcp_ack_buf=%d}",
        ul_limit_throughput_high, ul_limit_throughput_low, ul_rx_throughput_mbps, en_tcp_ack_buf);

    g_st_tcp_ack_buf_switch.uc_cur_tcp_ack_buf_en = en_tcp_ack_buf;

    st_tcp_ack_param.en_cmd = MAC_TCP_ACK_BUF_ENABLE;
    st_tcp_ack_param.en_enable = en_tcp_ack_buf;

    hmac_config_tcp_ack_buf(pst_mac_vap, OAL_SIZEOF(mac_cfg_tcp_ack_buf_stru), (oal_uint8 *)&st_tcp_ack_param);
}
#endif

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

oal_void hmac_tx_ampdu_switch(oal_uint32 ul_tx_throughput_mbps)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru *pst_mac_vap;
    mac_cfg_ampdu_tx_on_param_stru st_ampdu_tx_on = { 0 };
    oal_uint32 ul_limit_throughput_high = 0;
    oal_uint32 ul_limit_throughput_low = 0;
    oal_uint32 ul_ret;
    oal_bool_enum_uint8 en_ampdu_hw;
    hmac_user_stru *pst_hmac_user;
    /* ������ƻ���֧��Ӳ���ۺ� */
    if (g_st_ampdu_hw.uc_ampdu_hw_en == OAL_FALSE) {
        return;
    }
    /* ÿ������������ */
    if ((g_st_ampdu_hw.us_throughput_high != 0) && (g_st_ampdu_hw.us_throughput_low != 0)) {
        ul_limit_throughput_high = g_st_ampdu_hw.us_throughput_high;
        ul_limit_throughput_low = g_st_ampdu_hw.us_throughput_low;
    } else {
        ul_limit_throughput_high = WLAN_AMPDU_THROUGHPUT_THRESHOLD_HIGH;
        ul_limit_throughput_low = WLAN_AMPDU_THROUGHPUT_THRESHOLD_LOW;
    }

    pst_mac_device = mac_res_get_dev_etc(0);
    /* ����ǵ�VAP,�򲻿���Ӳ���ۺ� */
    if (1 != mac_device_calc_up_vap_num_etc(pst_mac_device)) {
        en_ampdu_hw = OAL_FALSE;
    } else {
        if (ul_tx_throughput_mbps > ul_limit_throughput_high) {
            /* ����300M,�л�Ӳ���ۺ� */
            en_ampdu_hw = OAL_TRUE;
        } else if (ul_tx_throughput_mbps < ul_limit_throughput_low) {
            /* ����200M,�л�Ӳ���ۺ� */
            en_ampdu_hw = OAL_FALSE;
        } else {
            /* ����200M-300M֮��,�����л� */
            return;
        }
    }

    /* ��ǰ�ۺϷ�ʽ��ͬ,������ */
    if (g_st_ampdu_hw.uc_ampdu_hw_enable == en_ampdu_hw) {
        return;
    }

    /* �л���Ӳ���ۺ�ʱ,��Ҫ�ж��Ƿ�����л����� */
    if (en_ampdu_hw == OAL_TRUE) {
        g_st_ampdu_hw.us_remain_hw_cnt = 0;

        ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_mac_vap);
        if ((ul_ret != OAL_SUCC) || (pst_mac_vap == OAL_PTR_NULL)) {
            return;
        }

        /* VHT/HE��֧���л� */
        if ((pst_mac_vap->en_protocol != WLAN_VHT_MODE) &&
            (pst_mac_vap->en_protocol != WLAN_VHT_ONLY_MODE)
#ifdef _PRE_WLAN_FEATURE_11AX
            && (pst_mac_vap->en_protocol != WLAN_HE_MODE)
#endif
        ) {
            return;
        }

        /* ����:���鹤��Ƶ�ʵ���160Mhz(80M���´���)��֧���л� */
        if (WLAN_BAND_WIDTH_80PLUSPLUS > pst_mac_vap->st_channel.en_bandwidth) {
            return;
        }
        /* ��������AP������Ӳ���ۺ��л� */
        if (IS_LEGACY_STA(pst_mac_vap)) {
            pst_hmac_user = mac_res_get_hmac_user_etc(pst_mac_vap->us_assoc_vap_id);
            if (pst_hmac_user == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_tx_ampdu_switch: pst_hmac_user is null ptr.");
                return;
            }
            if (pst_hmac_user->en_user_ap_type & MAC_AP_TYPE_AGGR_BLACKLIST) {
                return;
            }
        }
    } else {
        /* ��������Ƶ���л�,��Ӳ���ۺ��л������ۺ�,�������ܽ������л���ֵ,
           ����Ҫ��Ӳ���ۺ�����������һ��ʱ�� */
        /* �ǵ�vap,ֱ���л� */
        if (1 == mac_device_calc_up_vap_num_etc(pst_mac_device)) {
            g_st_ampdu_hw.us_remain_hw_cnt++;
            if (g_st_ampdu_hw.us_remain_hw_cnt < WLAN_AMPDU_HW_SWITCH_PERIOD) {
                return;
            }
            g_st_ampdu_hw.us_remain_hw_cnt = 0;
        }
        pst_mac_vap = mac_res_get_mac_vap(0);
        if (pst_mac_vap == OAL_PTR_NULL) {
            return;
        }
    }

    g_st_ampdu_hw.uc_ampdu_hw_enable = en_ampdu_hw;

    OAM_WARNING_LOG3(0, OAM_SF_ANY, "{hmac_tx_ampdu_switch: limit_high = [%d],limit_low = [%d],tx_throught= [%d]!}",
                     ul_limit_throughput_high, ul_limit_throughput_low, ul_tx_throughput_mbps);

    if (en_ampdu_hw == OAL_TRUE) {
        st_ampdu_tx_on.uc_aggr_tx_on = 4;
    } else {
        st_ampdu_tx_on.uc_aggr_tx_on = 8;
    }

    st_ampdu_tx_on.uc_snd_type = 1;
    st_ampdu_tx_on.en_aggr_switch_mode = AMPDU_SWITCH_BY_BA_LUT;
    hmac_config_set_ampdu_tx_on_etc(pst_mac_vap,
                                    OAL_SIZEOF(mac_cfg_ampdu_tx_on_param_stru),
                                    (oal_uint8 *)&st_ampdu_tx_on);
}
#endif


OAL_STATIC OAL_INLINE oal_uint32 hmac_tx_filter_security(hmac_vap_stru *pst_hmac_vap,
                                                         oal_netbuf_stru *pst_buf,
                                                         hmac_user_stru *pst_hmac_user)
{
    mac_ether_header_stru *pst_ether_header = OAL_PTR_NULL;
    mac_user_stru *pst_mac_user = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret = OAL_SUCC;
    oal_uint16 us_ether_type;

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_hmac_user->st_user_base_info);

    if (OAL_TRUE == mac_mib_get_rsnaactivated(pst_mac_vap)) { /* �ж��Ƿ�ʹ��WPA/WPA2 */
        if (pst_mac_user->en_port_valid != OAL_TRUE) { /* �ж϶˿��Ƿ�� */
            /* ��ȡ��̫��ͷ */
            pst_ether_header = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);
            /* ��������ʱ����Է�EAPOL ������֡������ */
            if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_PAE) != pst_ether_header->us_ether_type) {
                us_ether_type = oal_byteorder_host_to_net_uint16(pst_ether_header->us_ether_type);
                OAM_WARNING_LOG2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                                 "{hmac_tx_filter_security::TYPE 0x%04x, 0x%04x.}",
                                 us_ether_type, ETHER_TYPE_PAE);
                ul_ret = OAL_FAIL;
            }
        }
    }

    return ul_ret;
}


oal_void hmac_tx_ba_setup_etc(hmac_vap_stru *pst_hmac_vap,
                              hmac_user_stru *pst_user,
                              oal_uint8 uc_tidno)
{
    mac_action_mgmt_args_stru st_action_args; /* ������дACTION֡�Ĳ��� */

    /*
    ����BA�Ựʱ��st_action_args�ṹ������Ա��������
    (1)uc_category:action�����
    (2)uc_action:BA action�µ����
    (3)ul_arg1:BA�Ự��Ӧ��TID
    (4)ul_arg2:BUFFER SIZE��С
    (5)ul_arg3:BA�Ự��ȷ�ϲ���
    (6)ul_arg4:TIMEOUTʱ��
    */
    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_ADDBA_REQ;
    st_action_args.ul_arg1 = uc_tidno; /* ������֡��Ӧ��TID�� */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* ADDBA_REQ�У�buffer_size��Ĭ�ϴ�С */
    st_action_args.ul_arg2 = (oal_uint32)wlan_customize_etc.ul_ampdu_tx_max_num;
    OAM_WARNING_LOG1(0, OAM_SF_TX, "hmac_tx_ba_setup_etc::[ba buffer size:%d]", st_action_args.ul_arg2);
#else
    st_action_args.ul_arg2 = WLAN_AMPDU_TX_MAX_BUF_SIZE; /* ADDBA_REQ�У�buffer_size��Ĭ�ϴ�С */
#endif

    st_action_args.ul_arg3 = MAC_BA_POLICY_IMMEDIATE; /* BA�Ự��ȷ�ϲ��� */
    st_action_args.ul_arg4 = 0;                       /* BA�Ự�ĳ�ʱʱ������Ϊ0 */

    /* ����BA�Ự */
    hmac_mgmt_tx_action_etc(pst_hmac_vap, pst_user, &st_action_args);
}


oal_uint32 hmac_tx_ucast_process_etc(hmac_vap_stru *pst_hmac_vap,
                                     oal_netbuf_stru *pst_buf,
                                     hmac_user_stru *pst_user,
                                     mac_tx_ctl_stru *pst_tx_ctl)
{
    oal_uint32 ul_ret = HMAC_TX_PASS;
#ifdef _PRE_WLAN_FEATURE_CAR
    hmac_device_stru *pst_hmac_dev;
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    mac_qos_enhance_sta_stru *pst_qos_enhance_hash = OAL_PTR_NULL;
    oal_uint8 uc_old_tid;
    oal_uint8 uc_new_tid;
#endif

    /* ��ȫ���� */
#if defined(_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    if (OAL_UNLIKELY(OAL_SUCC != hmac_tx_filter_security(pst_hmac_vap, pst_buf, pst_user))) {
        OAM_STAT_VAP_INCR(pst_hmac_vap->st_vap_base_info.uc_vap_id, tx_security_check_faild, 1);
        return HMAC_TX_DROP_SECURITY_FILTER;
    }
#endif

    /* ��̫��ҵ��ʶ�� */
#ifdef _PRE_WLAN_FEATURE_CLASSIFY
    hmac_tx_classify(pst_hmac_vap, &(pst_user->st_user_base_info), pst_buf);
#endif

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    /* ���ap������WMM������qos_enhance�ж� */
    if ((pst_hmac_vap->st_vap_base_info.en_vap_wmm == OAL_TRUE) &&
        (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (pst_hmac_vap->st_vap_base_info.st_qos_enhance.en_qos_enhance_enable == OAL_TRUE)) {
        if (pst_hmac_vap->st_vap_base_info.st_qos_enhance.uc_qos_enhance_sta_count > 0) {
            uc_old_tid = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);
            uc_new_tid = uc_old_tid;
            pst_qos_enhance_hash = mac_tx_find_qos_enhance_list(&(pst_hmac_vap->st_vap_base_info),
                                                                pst_user->st_user_base_info.auc_user_mac_addr);

            if (pst_qos_enhance_hash != OAL_PTR_NULL && pst_qos_enhance_hash->uc_add_num >= MAC_QOS_ENHANCE_ADD_NUM) {
                if (uc_old_tid == WLAN_TIDNO_BEST_EFFORT ||
                    uc_old_tid == WLAN_TIDNO_BACKGROUND ||
                    uc_old_tid == WLAN_TIDNO_VIDEO) {
                    uc_new_tid = WLAN_TIDNO_VOICE;
                }
            } else {
                /* �����豸 */
                if (pst_user->st_user_base_info.en_qos_enhance_sta_state == MAC_USER_QOS_ENHANCE_NEAR) {
                    uc_new_tid = WLAN_TIDNO_VIDEO;
                }
                /* Զ���豸 */
                else if (pst_user->st_user_base_info.en_qos_enhance_sta_state == MAC_USER_QOS_ENHANCE_FAR) {
                    uc_new_tid = WLAN_TIDNO_BEST_EFFORT;
                }
            }

            if (uc_old_tid != uc_new_tid) {
                /* ����ac��tid��cb�ֶ� */
                MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = uc_new_tid;
                MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_TID_TO_AC(uc_new_tid);
            }
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_TRAFFIC_CTL
    /* TBD */
#endif

    OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_TRAFFIC_CLASSIFY);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TRAFFIC_CLASSIFY);

    /* �����EAPOL��DHCP֡����������������BA�Ự */
    if (MAC_GET_CB_IS_VIPFRAME(pst_tx_ctl)) {
        return HMAC_TX_PASS;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU
    if (OAL_TRUE == hmac_tid_need_ba_session(pst_hmac_vap, pst_user, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl), pst_buf)) {
        /* �Զ���������BA�Ự������AMPDU�ۺϲ�����Ϣ��DMACģ��Ĵ���addba rsp֡��ʱ�̺��� */
        hmac_tx_ba_setup_etc(pst_hmac_vap, pst_user, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl));
    }
#endif

    OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_SETUP_BA);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_SETUP_BA);

#ifdef _PRE_WLAN_FEATURE_CAR
    /* ������car���ж������� */
    pst_hmac_dev = hmac_res_get_mac_dev_etc(pst_hmac_vap->st_vap_base_info.uc_device_id);
    if (pst_hmac_dev->en_car_enable_flag == OAL_TRUE) {
        ul_ret = hmac_car_process(pst_hmac_dev, pst_hmac_vap, pst_user, pst_buf, HMAC_CAR_DOWNLINK);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(0, 0,
                "hmac_tx_ucast_process_etc:hmac_car_process: downlink car: DROP PACKET! ul_ret[%d]", ul_ret);
            return HMAC_TX_DROP_CAR_LIMIT;
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_AMSDU
    ul_ret = hmac_amsdu_notify_etc(pst_hmac_vap, pst_user, pst_buf);
    if (OAL_UNLIKELY(ul_ret != HMAC_TX_PASS)) {
        return ul_ret;
    }
#endif
    OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_AMSDU);

    return HMAC_TX_PASS;
}


OAL_STATIC oal_uint32 hmac_tx_need_frag(hmac_vap_stru *pst_hmac_vap,
                                        hmac_user_stru *pst_hmac_user,
                                        oal_netbuf_stru *pst_netbuf,
                                        mac_tx_ctl_stru *pst_tx_ctl)
{
    oal_uint32 ul_threshold;
    /* �жϱ����Ƿ���Ҫ���з�Ƭ */
    /* 1�����ȴ�������          */
    /* 2����legacЭ��ģʽ       */
    /* 3�����ǹ㲥֡            */
    /* 4�����Ǿۺ�֡            */
    /* 6��DHCP֡�����з�Ƭ      */
    if (MAC_GET_CB_IS_VIPFRAME(pst_tx_ctl)) {
        return OAL_FALSE;
    }

    ul_threshold = mac_mib_get_FragmentationThreshold(&pst_hmac_vap->st_vap_base_info);
    ul_threshold = (ul_threshold & (~(BIT0 | BIT1))) + 2;

    if ((ul_threshold < (OAL_NETBUF_LEN(pst_netbuf) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl))) &&
        (MAC_GET_CB_IS_MCAST(pst_tx_ctl) == OAL_FALSE) &&
        (OAL_FALSE == MAC_GET_CB_IS_AMSDU(pst_tx_ctl))
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        && (WLAN_TX_AMSDU_NONE == MAC_GET_CB_AMSDU_LEVEL(pst_tx_ctl))
#endif
        && (pst_hmac_user->st_user_base_info.en_cur_protocol_mode < WLAN_HT_MODE ||
                  pst_hmac_vap->st_vap_base_info.en_protocol < WLAN_HT_MODE)
        && (OAL_FALSE == hmac_tid_ba_is_setup(pst_hmac_user, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl)))) {
        return ul_threshold;
    }

    return 0;
}

#if defined _PRE_WLAN_FEATURE_WDS || defined _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_encap_wds_check(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user)
{
    return ((pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_REPEATER_STA) ||
            ((pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_ROOTAP) && (pst_user->uc_is_wds == OAL_TRUE)));
}
#endif


/*lint -e695*/
oal_uint32 hmac_tx_encap_etc(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, oal_netbuf_stru *pst_buf) /*lint !e695*/
{
    oal_uint8 *puc_80211_hdr = OAL_PTR_NULL; /* 802.11ͷ */
    oal_uint32 ul_qos = HMAC_TX_BSS_NOQOS;
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret = OAL_SUCC;
    oal_uint32 ul_threshold;
    mac_ieee80211_frame_stru *pst_head;
    oal_uint8 uc_buf_is_amsdu;
    oal_uint8 uc_ic_header = 0;
    oal_uint16 us_mpdu_len = 0;
    mac_ether_header_stru st_ether_hdr;

#ifdef _PRE_DEBUG_MODE
    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_vap, pst_buf))) {
        OAM_ERROR_LOG2(0, OAM_SF_TX, "{hmac_tx_encap_etc::param null,%x %x.}", (uintptr_t)pst_vap, (uintptr_t)pst_buf);
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    /* ��ȡCB */
    pst_tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(pst_buf));
    uc_buf_is_amsdu = MAC_GET_CB_IS_AMSDU(pst_tx_ctl);

#if defined _PRE_WLAN_FEATURE_WDS || defined _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    if (hmac_tx_encap_wds_check(pst_vap, pst_user)) {
        MAC_GET_CB_IS_4ADDRESS(pst_tx_ctl) = OAL_TRUE;
    }
#endif

    /* ��ȡ��̫��ͷ, ԭ��ַ��Ŀ�ĵ�ַ, ��̫������ */
    if (EOK != memcpy_s(&st_ether_hdr, sizeof(st_ether_hdr), oal_netbuf_data(pst_buf), ETHER_HDR_LEN)) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "hmac_tx_encap_etc::memcpy fail!");
        return OAL_FAIL;
    }

    /* ��amsdu֡ */
    if (uc_buf_is_amsdu == OAL_TRUE) {
        st_ether_hdr.us_ether_type = 0;
    } else {
        /* len = EHTER HEAD LEN + PAYLOAD LEN */
        us_mpdu_len = (oal_uint16)oal_netbuf_get_len(pst_buf);

        /* ����frame���ȣ�ָ��skb payload--LLC HEAD */
        MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = (us_mpdu_len - ETHER_HDR_LEN + SNAP_LLC_FRAME_LEN);

        MAC_GET_CB_ETHER_HEAD_PADDING(pst_tx_ctl) = 0;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        hmac_tx_encap_large_skb_amsdu(pst_vap, pst_user, pst_buf, pst_tx_ctl);

        if (MAC_GET_CB_AMSDU_LEVEL(pst_tx_ctl)) {
            /* �ָ�dataָ�뵽ETHER HEAD - LLC HEAD */
            oal_netbuf_pull(pst_buf, SNAP_LLC_FRAME_LEN);
        }
#endif
        /* ����LLC HEAD */
        mac_set_snap(pst_buf, st_ether_hdr.us_ether_type);

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        if (MAC_GET_CB_AMSDU_LEVEL(pst_tx_ctl)) {
            /* �ָ�dataָ�뵽ETHER HEAD */
            oal_netbuf_push(pst_buf, ETHER_HDR_LEN);

            /* ��֤4bytes���� */
            if ((oal_uint)(uintptr_t)oal_netbuf_data(pst_buf) !=
                OAL_ROUND_DOWN((oal_uint)(uintptr_t)oal_netbuf_data(pst_buf), 4)) {
                MAC_GET_CB_ETHER_HEAD_PADDING(pst_tx_ctl) = (oal_uint)(uintptr_t)oal_netbuf_data(pst_buf) -
                    OAL_ROUND_DOWN ((oal_uint)(uintptr_t)oal_netbuf_data(pst_buf), 4);
                oal_netbuf_push(pst_buf, MAC_GET_CB_ETHER_HEAD_PADDING(pst_tx_ctl));
            }
        }
#endif
    }

    /* ���skb��dataָ��ǰԤ���Ŀռ����802.11 mac head len������Ҫ���������ڴ���802.11ͷ */
    if (oal_netbuf_headroom(pst_buf) >= MAC_80211_QOS_HTC_4ADDR_FRAME_LEN) {
        puc_80211_hdr = (OAL_NETBUF_HEADER(pst_buf) - MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);

        MAC_GET_CB_80211_MAC_HEAD_TYPE(pst_tx_ctl) = 1; /* ָʾmacͷ����skb�� */
    } else {
        /* ��������80211ͷ */
        puc_80211_hdr = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_SHARED_DATA_PKT, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN, OAL_FALSE);
        if (OAL_UNLIKELY(puc_80211_hdr == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_tx_encap_etc::pst_hdr null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        MAC_GET_CB_80211_MAC_HEAD_TYPE(pst_tx_ctl) = 0; /* ָʾmacͷ������skb�У������˶����ڴ��ŵ� */
    }

    /* ���鲥֡����ȡ�û���QOS����λ��Ϣ */
    if (MAC_GET_CB_IS_MCAST(pst_tx_ctl) == OAL_FALSE) {
        /* �����û��ṹ���cap_info���ж��Ƿ���QOSվ�� */
        ul_qos = pst_user->st_user_base_info.st_cap_info.bit_qos;
        MAC_SET_CB_IS_QOS_DATA(pst_tx_ctl, ul_qos);
    }

    /* ����֡���� */
    hmac_tx_set_frame_ctrl(ul_qos, pst_tx_ctl, (mac_ieee80211_qos_htc_frame_addr4_stru *)puc_80211_hdr);
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_tx_set_frame_htc(pst_vap, ul_qos, pst_tx_ctl, pst_user,
                          (mac_ieee80211_qos_htc_frame_addr4_stru *)puc_80211_hdr);
#endif
    /* ���õ�ַ */
    hmac_tx_set_addresses(pst_vap, pst_user, pst_tx_ctl, &st_ether_hdr,
                          (mac_ieee80211_qos_htc_frame_addr4_stru *)puc_80211_hdr);

    /* �ҽ�802.11ͷ */
    pst_head = (mac_ieee80211_frame_stru *)puc_80211_hdr;
    MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl) = pst_head;

    /* ��Ƭ���� */
    ul_threshold = hmac_tx_need_frag(pst_vap, pst_user, pst_buf, pst_tx_ctl);
    if (ul_threshold != 0) {
        /* TBD���ü��ܽӿ���ʹ��TKIPʱ��MSDU���м��ܺ��ڽ��з�Ƭ */
        ul_ret = hmac_en_mic_etc(pst_vap, pst_user, pst_buf, &uc_ic_header);
        if (ul_ret != OAL_SUCC) {
            OAM_ERROR_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                           "{hmac_tx_encap_etc::hmac_en_mic_etc failed[%d].}", ul_ret);
            return ul_ret;
        }

        /* ���з�Ƭ���� */
        ul_ret = hmac_frag_process(pst_vap, pst_buf, pst_tx_ctl, (oal_uint32)uc_ic_header, ul_threshold);
    }

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file((const oal_uint8 *)pst_head, MAC_80211_QOS_FRAME_LEN,
                            (const oal_uint8 *)oal_netbuf_data(pst_buf), OAL_NETBUF_LEN(pst_buf), 1);
#endif

    return ul_ret;
}
/*lint +e695*/

OAL_STATIC oal_uint32 hmac_tx_lan_mpdu_process_sta(hmac_vap_stru *pst_vap,
                                                   oal_netbuf_stru *pst_buf,
                                                   mac_tx_ctl_stru *pst_tx_ctl)
{
    hmac_user_stru *pst_user = OAL_PTR_NULL; /* Ŀ��STA�ṹ�� */
    mac_ether_header_stru *pst_ether_hdr;    /* ��̫��ͷ */
    oal_uint32 ul_ret;
    oal_uint16 us_user_idx;
    oal_uint8 *puc_ether_payload = OAL_PTR_NULL;

    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctl) = pst_vap->st_vap_base_info.uc_vap_id;

    us_user_idx = pst_vap->st_vap_base_info.us_assoc_vap_id;

    pst_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_idx);
    if (pst_user == OAL_PTR_NULL) {
        OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_USER_NULL;
    }
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    if (pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_REPEATER_STA) {
        hmac_wds_update_neigh(pst_vap, pst_ether_hdr->auc_ether_shost);
    } else if ((pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_NONE) &&
               (oal_byteorder_host_to_net_uint16(ETHER_TYPE_ARP) == pst_ether_hdr->us_ether_type))
#else
    if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_ARP) == pst_ether_hdr->us_ether_type)
#endif
    {
        pst_ether_hdr++;
        puc_ether_payload = (oal_uint8 *)pst_ether_hdr;
        /* The source MAC address is modified only if the packet is an   */
        /* ARP Request or a Response. The appropriate bytes are checked. */
        /* Type field (2 bytes): ARP Request (1) or an ARP Response (2)  */
        if ((puc_ether_payload[6] == 0x00) &&
            (puc_ether_payload[7] == 0x02 || puc_ether_payload[7] == 0x01)) {
            /* Set Address2 field in the WLAN Header with source address */
            oal_set_mac_addr(puc_ether_payload + 8, mac_mib_get_StationID(&pst_vap->st_vap_base_info));
        }
    }

    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;

    ul_ret = hmac_tx_ucast_process_etc(pst_vap, pst_buf, pst_user, pst_tx_ctl);
    if (OAL_UNLIKELY(ul_ret != HMAC_TX_PASS)) {
        return ul_ret;
    }

    /* ��װ802.11ͷ */
    ul_ret = hmac_tx_encap_etc(pst_vap, pst_user, pst_buf);
    if (OAL_UNLIKELY((ul_ret != OAL_SUCC))) {
        OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_mpdu_process_sta::hmac_tx_encap_etc failed[%d].}", ul_ret);
        OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }

    OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_ENCAP_HEAD);
    return HMAC_TX_PASS;
}

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_proxyarp_is_en(hmac_vap_stru *pst_vap)
{
    return (oal_bool_enum_uint8)((pst_vap->st_vap_base_info.pst_vap_proxyarp != OAL_PTR_NULL) &&
                                 (pst_vap->st_vap_base_info.pst_vap_proxyarp->en_is_proxyarp == OAL_TRUE));
}
#endif


OAL_STATIC OAL_INLINE oal_uint32 hmac_tx_lan_mpdu_process_ap(hmac_vap_stru *pst_vap,
                                                             oal_netbuf_stru *pst_buf,
                                                             mac_tx_ctl_stru *pst_tx_ctl)
{
    hmac_user_stru *pst_user = OAL_PTR_NULL; /* Ŀ��STA�ṹ�� */
    mac_ether_header_stru *pst_ether_hdr;    /* ��̫��ͷ */
    oal_uint8 *puc_addr;                     /* Ŀ�ĵ�ַ */
    oal_uint32 ul_ret;
    oal_uint16 us_user_idx = MAC_INVALID_USER_ID;
#ifdef _PRE_WLAN_FEATURE_CAR
    hmac_device_stru *pst_hmac_dev;
#endif
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    oal_uint8 *src_addr; /* Դ��ַ */
#endif

    /* �ж����鲥�򵥲�,����lan to wlan�ĵ���֡��������̫����ַ */
    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);
    puc_addr = pst_ether_hdr->auc_ether_dhost;
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    src_addr = pst_ether_hdr->auc_ether_shost;
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    /* ȷ��proxy arp �Ƿ�ʹ�� */
    if (OAL_TRUE == hmac_tx_proxyarp_is_en(pst_vap)) {
        if (OAL_TRUE == hmac_proxy_arp_proc(pst_vap, pst_buf)) {
            return HMAC_TX_DROP_PROXY_ARP;
        }
    }
#endif

    /* ��������֡ */
#ifdef _PRE_WLAN_CHIP_TEST
    if (OAL_LIKELY(!ETHER_IS_MULTICAST(puc_addr)) && pst_vap->st_vap_base_info.bit_al_tx_flag != OAL_SWITCH_ON)
#else
    if (OAL_LIKELY(!ETHER_IS_MULTICAST(puc_addr)))
#endif
    {
        ul_ret = mac_vap_find_user_by_macaddr_etc(&(pst_vap->st_vap_base_info), puc_addr, &us_user_idx);
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
        if (ul_ret != OAL_SUCC) {
            ul_ret = hmac_find_valid_user_by_wds_sta(pst_vap, puc_addr, &us_user_idx);
        }
#endif

        if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{hmac_tx_lan_mpdu_process_ap::hmac_tx_find_user failed %2x:%2x:%2x:%2x}",
                             puc_addr[2], puc_addr[3], puc_addr[4], puc_addr[5]);
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_USER_UNKNOWN;
        }

        /* ת��HMAC��USER�ṹ�� */
        pst_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_idx);
        if (OAL_UNLIKELY(pst_user == OAL_PTR_NULL)) {
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_USER_NULL;
        }

        /* �û�״̬�ж� */
        if (OAL_UNLIKELY(pst_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC)) {
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_USER_INACTIVE;
        }

        /* Ŀ��userָ�� */
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;

        ul_ret = hmac_tx_ucast_process_etc(pst_vap, pst_buf, pst_user, pst_tx_ctl);
        if (OAL_UNLIKELY(ul_ret != HMAC_TX_PASS)) {
            return ul_ret;
        }
    } else { /* �鲥 or �㲥 */
        /* �����鲥��ʶλ */
        MAC_GET_CB_IS_MCAST(pst_tx_ctl) = OAL_TRUE;

        /* ����ACK���� */
        MAC_GET_CB_ACK_POLACY(pst_tx_ctl) = WLAN_TX_NO_ACK;

        /* ��ȡ�鲥�û� */
        pst_user = mac_res_get_hmac_user_etc(pst_vap->st_vap_base_info.us_multi_user_idx);

        if (OAL_UNLIKELY(pst_user == OAL_PTR_NULL)) {
            OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{hmac_tx_lan_mpdu_process_ap::get multi user failed[%d].}",
                             pst_vap->st_vap_base_info.us_multi_user_idx);
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_MUSER_NULL;
        }

        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = pst_vap->st_vap_base_info.us_multi_user_idx;
        MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = WLAN_TIDNO_BCAST;
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_TID_TO_AC(WLAN_TIDNO_BCAST);

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST) /* �鲥ת���� */
        if ((!ETHER_IS_BROADCAST(puc_addr)) && (pst_vap->pst_m2u != OAL_PTR_NULL)) {
            if ((ETHER_IS_IPV4_MULTICAST(puc_addr)) || (ETHER_IS_IPV6_MULTICAST(puc_addr))) {
                ul_ret = hmac_m2u_snoop_convert(pst_vap, pst_buf);
                if (ul_ret != HMAC_TX_PASS) {
                    return ul_ret;
                }
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
                else { /* �鲥�������� */
                    /* ����STA�������� */
                    hmac_m2u_adaptive_inspecting(pst_vap, pst_buf);
                    ul_ret = hmac_m2u_multicast_drop(pst_vap, pst_buf);
                    if (ul_ret != HMAC_TX_PASS) {
                        return ul_ret;
                    }
                }
#endif
            }
        }
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
        /* δ֪�鲥��car���ж������� */
        pst_hmac_dev = hmac_res_get_mac_dev_etc(pst_vap->st_vap_base_info.uc_device_id);
        if ((pst_hmac_dev->en_car_enable_flag == OAL_TRUE) &&
            (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            ul_ret = hmac_car_multicast_process(pst_hmac_dev, pst_buf);
            if (ul_ret != OAL_SUCC) {
                OAM_WARNING_LOG1(0, 0,
                    "hmac_tx_lan_mpdu_process_ap: device downlink car multicast : DROP PACKET! ul_ret[%d]", ul_ret);
                return HMAC_TX_DROP_CAR_LIMIT;
            }
        }
#endif

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
        
        if ((pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_ROOTAP) && ETHER_IS_BROADCAST(puc_addr)) {
            hmac_wds_node_ergodic(pst_vap, src_addr, hmac_wds_tx_broadcast_pkt, (void *)pst_buf);
        }
#endif
    }

    /* ��װ802.11ͷ */
    ul_ret = hmac_tx_encap_etc(pst_vap, pst_user, pst_buf);
    if (OAL_UNLIKELY((ul_ret != OAL_SUCC))) {
        OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_mpdu_process_ap::hmac_tx_encap_etc failed[%d].}", ul_ret);
        OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }

    return HMAC_TX_PASS;
}


oal_void hmac_tx_lan_mpdu_info(mac_vap_stru *pst_vap, oal_uint8 uc_data_type,
                               oal_netbuf_stru *pst_buf, mac_tx_ctl_stru *pst_tx_ctl)
{
#ifndef WIN32
    mac_eapol_type_enum_uint8 en_eapol_type = MAC_EAPOL_PTK_BUTT;
    oal_uint8 uc_dhcp_type;
    mac_ieee80211_qos_htc_frame_addr4_stru *pst_mac_header = OAL_PTR_NULL;
    mac_llc_snap_stru *pst_llc = OAL_PTR_NULL;
    oal_ip_header_stru *pst_rx_ip_hdr = OAL_PTR_NULL;
    oal_eth_arphdr_stru *puc_arp_head = OAL_PTR_NULL;
    oal_int32 l_ret = EOK;

    oal_uint8 auc_ar_sip[ETH_SENDER_IP_ADDR_LEN]; /* sender IP address */
    oal_uint8 auc_ar_dip[ETH_SENDER_IP_ADDR_LEN]; /* sender IP address */

    /* ����skb�Ѿ���װ80211ͷ */
    /* ��ȡLLC SNAP */
    pst_llc = (mac_llc_snap_stru *)oal_netbuf_data(pst_buf);

    if (uc_data_type == MAC_DATA_EAPOL) {
        en_eapol_type = mac_get_eapol_key_type_etc((oal_uint8 *)(pst_llc + 1));
        OAM_WARNING_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_tx_lan_mpdu_info::EAPOL type=%u, len==%u}[1:1/4 2:2/4 3:3/4 4:4/4]",
                         en_eapol_type, OAL_NETBUF_LEN(pst_buf));
    } else if (uc_data_type == MAC_DATA_DHCP) {
        pst_rx_ip_hdr = (oal_ip_header_stru *)(pst_llc + 1);

        l_ret += memcpy_s((oal_uint8 *)auc_ar_sip, ETH_SENDER_IP_ADDR_LEN,
                          (oal_uint8 *)&pst_rx_ip_hdr->ul_saddr, OAL_SIZEOF(oal_uint32));
        l_ret += memcpy_s((oal_uint8 *)auc_ar_dip, ETH_SENDER_IP_ADDR_LEN,
                          (oal_uint8 *)&pst_rx_ip_hdr->ul_daddr, OAL_SIZEOF(oal_uint32));
        if (l_ret != EOK) {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_tx_lan_mpdu_info::memcpy fail!");
            return;
        }

        uc_dhcp_type = mac_get_dhcp_frame_type_etc(pst_rx_ip_hdr);
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_tx_lan_mpdu_info::DHCP type=%d.[1:discovery 2:offer 3:request 4:decline 5:ack 6:nack 7:release 8:inform.]",
            uc_dhcp_type);
        OAM_WARNING_LOG4(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_lan_mpdu_info:: DHCP sip: %d.%d, dip: %d.%d.",
                         auc_ar_sip[2], auc_ar_sip[3], auc_ar_dip[2], auc_ar_dip[3]);
    } else {
        puc_arp_head = (oal_eth_arphdr_stru *)(pst_llc + 1);
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_lan_mpdu_info:: ARP type=%d.[2:arp resp 3:arp req.]",
                         uc_data_type);
        OAM_WARNING_LOG4(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_lan_mpdu_info:: ARP sip: %d.%d, dip: %d.%d",
                         puc_arp_head->auc_ar_sip[2], puc_arp_head->auc_ar_sip[3],
                         puc_arp_head->auc_ar_tip[2], puc_arp_head->auc_ar_tip[3]);
    }

    pst_mac_header = (mac_ieee80211_qos_htc_frame_addr4_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
    if (pst_mac_header != OAL_PTR_NULL) {
        OAM_WARNING_LOG4(pst_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_tx_lan_mpdu_info::send to wlan smac: %x:%x, dmac: %x:%x]",
                         pst_mac_header->auc_address2[4], pst_mac_header->auc_address2[5],
                         pst_mac_header->auc_address1[4], pst_mac_header->auc_address1[5]);
    }
#endif
}


/*lint -e695*/
oal_uint32 hmac_tx_lan_to_wlan_no_tcp_opt_etc(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf)
{
    frw_event_stru *pst_event = OAL_PTR_NULL; /* �¼��ṹ�� */
    frw_event_mem_stru *pst_event_mem = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap;                /* VAP�ṹ�� */
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL; /* SKB CB */
    oal_uint32 ul_ret = HMAC_TX_PASS;
    dmac_tx_event_stru *pst_dtx_stru = OAL_PTR_NULL;
    oal_uint8 uc_data_type;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_stru *pst_wapi;
    mac_ieee80211_frame_stru *pst_mac_hdr;
    oal_bool_enum_uint8 en_is_mcast = OAL_FALSE;
#endif

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);

    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_TX, "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::pst_hmac_vap null.}");
        OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAPģʽ�ж� */
    if (OAL_UNLIKELY(pst_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP && pst_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::en_vap_mode=%d.}",
                         pst_vap->en_vap_mode);
        OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* ��������û�����Ϊ0���������� */
    if (OAL_UNLIKELY(pst_hmac_vap->st_vap_base_info.us_user_nums == 0)) {
        OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_FAIL;
    }

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_NO_TCP_OPT);

    /* �˴����ݿ��ܴ��ں˶�����Ҳ�п�����dev��������ͨ���տ�ת��ȥ��ע��һ�� */
    uc_data_type = mac_get_data_type_from_8023_etc((oal_uint8 *)oal_netbuf_data(pst_buf), MAC_NETBUFF_PAYLOAD_ETH);
    /* ��ʼ��CB tx rx�ֶ� , CB�ֶ���ǰ���Ѿ������㣬 �����ﲻ��Ҫ�ظ���ĳЩ�ֶθ���ֵ */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    MAC_GET_CB_MPDU_NUM(pst_tx_ctl) = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctl) = 1;
    MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctl) = WLAN_DATA_BASICTYPE;
    MAC_GET_CB_ACK_POLACY(pst_tx_ctl) = WLAN_TX_NORMAL_ACK;
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctl) = pst_vap->uc_vap_id;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = MAC_INVALID_USER_ID;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_BE; /* ��ʼ����BE���� */
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) = uc_data_type;

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
    hmac_parse_special_ipv4_packet(oal_netbuf_data(pst_buf), oal_netbuf_get_len(pst_buf), HMAC_PKT_DIRECTION_TX);
#endif

    /* ����LAN TO WLAN��WLAN TO WLAN��netbuf�������������Ϊ�����֣���Ҫ���ж�
       ��������������netbufȻ���ٶ�CB���¼������ֶθ�ֵ */
    if (MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) != FRW_EVENT_TYPE_WLAN_DTX) {
        MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) = FRW_EVENT_TYPE_HOST_DRX;
        MAC_GET_CB_EVENT_SUBTYPE(pst_tx_ctl) = DMAC_TX_HOST_DRX;
    }

#if ((_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT ||  \
         _PRE_TARGET_PRODUCT_TYPE_CPE == _PRE_CONFIG_TARGET_PRODUCT) && \
     (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    if ((uc_data_type == MAC_DATA_ARP_REQ || uc_data_type == MAC_DATA_ARP_RSP) &&
        (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        if (OAL_TRUE == wlan_check_arp_spoofing((struct net_device *)(hmac_vap_get_net_device_etc(pst_vap->uc_vap_id)),
                                                (struct sk_buff *)pst_buf)) {
            OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY,
                             "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::ARP spoofing, type is %d", uc_data_type);
            oam_report_eth_frame_etc(pst_vap->auc_bssid,
                                     oal_netbuf_data(pst_buf),
                                     (oal_uint16)OAL_NETBUF_LEN(pst_buf),
                                     OAM_OTA_FRAME_DIRECTION_TYPE_TX);
            return OAL_FAIL;
        }
    }
#endif

    oal_spin_lock_bh(&pst_hmac_vap->st_lock_state);
    /* ����host�ඪ������ڴ���ͳ�� */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    HMAC_TX_PKTS_STAT(1);
    hmac_auto_freq_wifi_tx_bytes_stat(OAL_NETBUF_LEN(pst_buf));
#endif

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    /* �����ݰ�����ͳ�� */
    hmac_auto_freq_wifi_tx_stat(1);
#endif

    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /*  ������ǰ MPDU */
        if (OAL_FALSE == mac_mib_get_dot11QosOptionImplemented(&pst_hmac_vap->st_vap_base_info)) {
            MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_VO; /* APģʽ ��WMM ��VO���� */
            MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = WLAN_WME_AC_TO_TID(MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl));
        }

        ul_ret = hmac_tx_lan_mpdu_process_ap(pst_hmac_vap, pst_buf, pst_tx_ctl);
    } else if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* ������ǰMPDU */
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_VO; /* STAģʽ ��qos֡��VO���� */
        MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = WLAN_WME_AC_TO_TID(MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl));

        OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_CB_INIT);

        ul_ret = hmac_tx_lan_mpdu_process_sta(pst_hmac_vap, pst_buf, pst_tx_ctl);
#ifdef _PRE_WLAN_FEATURE_WAPI
        if (ul_ret == HMAC_TX_PASS) {
            // && OAL_UNLIKELY(WAPI_IS_WORK(pst_hmac_vap)))
            /* ��ȡwapi���� �鲥/���� */
            pst_mac_hdr = MAC_GET_CB_FRAME_HEADER_ADDR((mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf));
            en_is_mcast = ETHER_IS_MULTICAST(pst_mac_hdr->auc_address1);
            /*lint -e730*/
            pst_wapi = hmac_user_get_wapi_ptr_etc(pst_vap, !en_is_mcast, pst_vap->us_assoc_vap_id);
            if (pst_wapi == OAL_PTR_NULL) {
                OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
                OAM_WARNING_LOG1(0, OAM_SF_ANY,
                    "hmac_tx_lan_to_wlan_no_tcp_opt_etc::hmac_user_get_wapi_ptr_etc fail! us_assoc_id[%u]}",
                    pst_vap->us_assoc_vap_id);
                oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
                return OAL_ERR_CODE_PTR_NULL;
            }

            /*lint +e730*/
            if ((OAL_TRUE == WAPI_PORT_FLAG(pst_wapi)) &&
                (pst_wapi->wapi_netbuff_txhandle != OAL_PTR_NULL)) {
                pst_buf = pst_wapi->wapi_netbuff_txhandle(pst_wapi, pst_buf);
                if (pst_buf == OAL_PTR_NULL) {
                    OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
                    OAM_WARNING_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY,
                                     "{hmac_tx_lan_to_wlan_no_tcp_opt_etc:: wapi_netbuff_txhandle fail!}");
                    oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
                    return OAL_ERR_CODE_PTR_NULL;
                }
                /*  ����wapi�����޸�netbuff���˴���Ҫ���»�ȡһ��cb */
                pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
            }
        }

#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */
    }

    oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_EVENT_TO_DMAC);

    if (OAL_LIKELY(ul_ret == HMAC_TX_PASS)) {
        /* ά�⣬���һ���ؼ�֡��ӡ */
        if (uc_data_type <= MAC_DATA_ARP_REQ) {
            hmac_tx_lan_mpdu_info(pst_vap, uc_data_type, pst_buf, pst_tx_ctl);
        }

#ifdef _PRE_WLAN_PKT_TIME_STAT
        if (DELAY_STATISTIC_SWITCH_ON && IS_NEED_RECORD_DELAY(pst_buf, TP_SKB_HMAC_XMIT)) {
            skbprobe_record_time(pst_buf, TP_SKB_HMAC_TX);
        }
#endif

        /* ���¼�������DMAC */
        pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
        if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_TX, "{hmac_tx_lan_to_wlan_etc::FRW_EVENT_ALLOC failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        pst_event = frw_get_event_stru(pst_event_mem);

        /* ��д�¼�ͷ */
        FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                           FRW_EVENT_TYPE_HOST_DRX,
                           DMAC_TX_HOST_DRX,
                           OAL_SIZEOF(dmac_tx_event_stru),
                           FRW_EVENT_PIPELINE_STAGE_1,
                           pst_vap->uc_chip_id,
                           pst_vap->uc_device_id,
                           pst_vap->uc_vap_id);

        pst_dtx_stru = (dmac_tx_event_stru *)pst_event->auc_event_data;
        pst_dtx_stru->pst_netbuf = pst_buf;
        pst_dtx_stru->us_frame_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl);

        /* �����¼� */
        ul_ret = frw_event_dispatch_event_etc(pst_event_mem);
        if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
            OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                             "{hmac_tx_lan_to_wlan_etc::frw_event_dispatch_event_etc failed[%d].}", ul_ret);
            OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        }

        /* �ͷ��¼� */
        FRW_EVENT_FREE(pst_event_mem);

        OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_TX_EVENT_TO_DMAC);
#ifdef _PRE_WLAN_PROFLING_MIPS
        oal_profiling_stop_tx_save();
#endif
    } else if (OAL_UNLIKELY(ul_ret == HMAC_TX_BUFF)) {
        ul_ret = OAL_SUCC;
    } else if (ul_ret == HMAC_TX_DONE) {
        ul_ret = OAL_SUCC;
    }
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    else if (ul_ret == HMAC_TX_DROP_NOSMART) {
        /* �鲥���Ĳ������鲥ת������(�����ܼҾ��豸)�����Զ�������������Ϊ */
        OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                      "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::HMAC_TX_DROP.reason[%d]!}", ul_ret);
    } else if (ul_ret == HMAC_TX_DROP_NOADAP) {
        /* �鲥���Ĳ������鲥ת������(������ģʽ)�����Զ�������������Ϊ */
        OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                      "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::HMAC_TX_DROP.reason[%d]!}", ul_ret);
    }
#endif

    else {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::HMAC_TX_DROP.reason[%d]!}", ul_ret);
    }

    return ul_ret;
}
/*lint +e695*/
#ifdef _PRE_WLAN_TCP_OPT
OAL_STATIC oal_uint32 hmac_transfer_tx_handler(hmac_device_stru *hmac_device,
                                               hmac_vap_stru *hmac_vap,
                                               oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *pst_tx_ctl;
    oal_uint32 ul_ret = OAL_SUCC;

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    if (WLAN_TCP_ACK_QUEUE == oal_netbuf_select_queue_etc(netbuf)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAM_WARNING_LOG0(0, OAM_SF_TX,
                         "{hmac_transfer_tx_handler::netbuf is tcp ack.}\r\n");
#endif
        oal_spin_lock_bh(&hmac_vap->st_hamc_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        oal_netbuf_list_tail(&hmac_vap->st_hamc_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE], netbuf);

        /* ����TCP ACK�ȴ�����, ���ⱨ�����Ϸ��� */
        if (hmac_judge_tx_netbuf_is_tcp_ack_etc((oal_ether_header_stru *)oal_netbuf_data(netbuf))) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
            MAC_GET_CB_TCP_ACK(pst_tx_ctl) = OAL_TRUE;
#endif
            oal_spin_unlock_bh(&hmac_vap->st_hamc_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
            hmac_sched_transfer_etc();
        } else {
            oal_spin_unlock_bh(&hmac_vap->st_hamc_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
            hmac_tcp_ack_process_etc();
        }
    } else {
        ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(&(hmac_vap->st_vap_base_info), netbuf);
    }
#endif
    return ul_ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_DHCP_REQ_DISABLE
OAL_STATIC oal_uint8 mac_get_dhcp_type(oal_uint8 *puc_pos, oal_uint8 *puc_packet_end)
{
    oal_uint8 *puc_opt;
    while ((puc_pos < puc_packet_end) && (*puc_pos != 0xFF)) {
        puc_opt = puc_pos++;
        if (*puc_opt == 0) {
            continue; /* Padding */
        }
        puc_pos += *puc_pos + 1;
        if (puc_pos >= puc_packet_end) {
            break;
        }
        if ((*puc_opt == 53) && (puc_opt[1] != 0)) { /* Message type */
            return puc_opt[2];
        }
    }
    return 0xFF;  // unknow type
}

oal_bool_enum_uint8 mac_dhcp_frame_should_drop(oal_uint8 *puc_frame_hdr, wlan_vap_mode_enum_uint8 mode)
{
    // oal_uint8                       uc_data_type         = MAC_DATA_BUTT;
    oal_uint8 *puc_pos = puc_frame_hdr;
    oal_ip_header_stru *pst_rx_ip_hdr;
    oal_dhcp_packet_stru *pst_rx_dhcp_hdr;
    oal_uint8 *puc_packet_end;
    oal_uint8 uc_type;

    if (OAL_UNLIKELY(puc_frame_hdr == OAL_PTR_NULL)) {
        return OAL_TRUE;
    }

    puc_pos += ETHER_HDR_LEN; /* ָ��IP Header */
    pst_rx_ip_hdr = (oal_ip_header_stru *)puc_pos;
    puc_pos += (puc_pos[0] & 0x0F) << 2; /* point udp header */
    pst_rx_dhcp_hdr = (oal_dhcp_packet_stru *)(puc_pos + 8);

    puc_packet_end = (oal_uint8 *)pst_rx_ip_hdr + OAL_NET2HOST_SHORT(pst_rx_ip_hdr->us_tot_len);
    puc_pos = &(pst_rx_dhcp_hdr->options[4]);

    uc_type = mac_get_dhcp_type(puc_pos, puc_packet_end);

    /* ��ֹ��AP������ӿڷ���DHCP Request��discovery���ģ���ֹ��STA������ӿڷ���DHCP offer��DHCP ACK���� */
    if ((mode == WLAN_VAP_MODE_BSS_STA && (uc_type == MAC_DHCP_OFFER || uc_type == MAC_DHCP_ACK)) ||
        (mode == WLAN_VAP_MODE_BSS_AP && (uc_type == MAC_DHCP_DISCOVER || uc_type == MAC_DHCP_REQUEST))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
#endif


oal_uint32 hmac_tx_wlan_to_wlan_ap_etc(oal_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL; /* �¼��ṹ�� */
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;
    oal_netbuf_stru *pst_buf = OAL_PTR_NULL;     /* ��netbuf����ȡ������ָ��netbuf��ָ�� */
    oal_netbuf_stru *pst_buf_tmp = OAL_PTR_NULL; /* �ݴ�netbufָ�룬����whileѭ�� */
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    /* ����ж� */
    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_tx_wlan_to_wlan_ap_etc::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* ��ȡ�¼� */
    pst_event = frw_get_event_stru(pst_event_mem);
    if (OAL_UNLIKELY(pst_event == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_tx_wlan_to_wlan_ap_etc::pst_event null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡPAYLOAD�е�netbuf�� */
    pst_buf = (oal_netbuf_stru *)(uintptr_t)(*((oal_uint *)(pst_event->auc_event_data)));

    ul_ret = hmac_tx_get_mac_vap_etc(pst_event->st_event_hdr.uc_vap_id, &pst_mac_vap);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        OAM_ERROR_LOG1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_TX,
                       "{hmac_tx_wlan_to_wlan_ap_etc::hmac_tx_get_mac_vap_etc failed[%d].}", ul_ret);
#ifdef _PRE_DEBUG_MODE
        hmac_free_netbuf_list_etc(pst_buf);
#else
        hmac_free_netbuf_list_etc(pst_buf);
#endif
        return ul_ret;
    }

    /* ѭ������ÿһ��netbuf��������̫��֡�ķ�ʽ���� */
    while (pst_buf != OAL_PTR_NULL) {
        pst_buf_tmp = OAL_NETBUF_NEXT(pst_buf);

        OAL_NETBUF_NEXT(pst_buf) = OAL_PTR_NULL;
        OAL_NETBUF_PREV(pst_buf) = OAL_PTR_NULL;

        
        pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_buf);
        memset_s(pst_tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

        MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) = FRW_EVENT_TYPE_WLAN_DTX;
        MAC_GET_CB_EVENT_SUBTYPE(pst_tx_ctl) = DMAC_TX_WLAN_DTX;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* set the queue map id when wlan to wlan */
        oal_skb_set_queue_mapping(pst_buf, WLAN_NORMAL_QUEUE);
#endif

        ul_ret = hmac_tx_lan_to_wlan_etc(pst_mac_vap, pst_buf);

        /* ����ʧ�ܣ��Լ������Լ��ͷ�netbuff�ڴ� */
        if (ul_ret != OAL_SUCC) {
            hmac_free_netbuf_list_etc(pst_buf);
        }

        pst_buf = pst_buf_tmp;
    }

    return OAL_SUCC;
}


/*lint -e695*/
oal_uint32 hmac_tx_lan_to_wlan_etc(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf)
{
    oal_uint32 ul_ret = HMAC_TX_PASS;
#ifdef _PRE_WLAN_TCP_OPT
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap; /* VAP�ṹ�� */
#endif

#ifdef _PRE_WLAN_TCP_OPT

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (OAL_UNLIKELY(pst_hmac_vap == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_tx_lan_to_wlan_tcp_opt::pst_dmac_vap null.}\r\n");
        return OAL_FAIL;
    }
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_vap->uc_device_id);
    if (OAL_UNLIKELY(pst_hmac_device == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                       "{hmac_tx_lan_to_wlan_tcp_opt::pst_hmac_device null.}\r\n");
        return OAL_FAIL;
    }
    if (pst_hmac_device->sys_tcp_tx_ack_opt_enable == OAL_TRUE) {
        ul_ret = hmac_transfer_tx_handler(pst_hmac_device, pst_hmac_vap, pst_buf);
    } else
#endif
    {
        ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(pst_vap, pst_buf);
    }
    return ul_ret;
}
/*lint +e695*/

oal_net_dev_tx_enum hmac_bridge_vap_xmit_etc(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint32 ul_ret;
#ifdef _PRE_WLAN_FEATURE_ROAM
    oal_uint8 uc_data_type;
#endif
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    hmac_user_stru *pst_hmac_user;
#endif

#if defined(_PRE_WLAN_FEATURE_ALWAYS_TX)
    hmac_device_stru *pst_hmac_device;
#endif
    oal_bool_enum_uint8 en_drop_frame = OAL_FALSE;

    if (OAL_UNLIKELY(pst_buf == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_bridge_vap_xmit_etc::pst_buf = OAL_PTR_NULL!}\r\n");
        return OAL_NETDEV_TX_OK;
    }

#ifdef _PRE_SKB_TRACE
    mem_trace_add_node((oal_ulong)pst_buf);
#endif

    if (OAL_UNLIKELY(pst_dev == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{hmac_bridge_vap_xmit_etc::pst_dev = OAL_PTR_NULL!}\r\n");
        oal_netbuf_free(pst_buf);
        OAM_STAT_VAP_INCR(0, tx_abnormal_msdu_dropped, 1);
        return OAL_NETDEV_TX_OK;
    }

    /* ��ȡVAP�ṹ�� */
    pst_vap = (mac_vap_stru *)OAL_NET_DEV_PRIV(pst_dev);

    /* ���VAP�ṹ�岻���ڣ��������� */
    if (OAL_UNLIKELY(pst_vap == OAL_PTR_NULL)) {
        /* will find vap fail when receive a pkt from
         * kernel while vap is deleted, return OAL_NETDEV_TX_OK is so. */
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{hmac_bridge_vap_xmit_etc::pst_vap = OAL_PTR_NULL!}\r\n");
        oal_netbuf_free(pst_buf);
        OAM_STAT_VAP_INCR(0, tx_abnormal_msdu_dropped, 1);
        return OAL_NETDEV_TX_OK;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_bridge_vap_xmit_etc::pst_hmac_vap null.}");
        oal_netbuf_free(pst_buf);
        return OAL_NETDEV_TX_OK;
    }

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_vap->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_PROXYSTA, "{hmac_bridge_vap_xmit_etc::pst_hmac_device is null!}");
        oal_netbuf_free(pst_buf);

        return OAL_NETDEV_TX_OK;
    }

    /* ��������ģ�pst_device_struָ���л�δ״̬, ���������л�δ����״̬ */
    if (pst_vap->bit_al_tx_flag == OAL_SWITCH_ON) {
        // || (pst_mac_device->pst_device_stru->bit_al_rx_flag != HAL_ALWAYS_RX_DISABLE))
        OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_TX, "{hmac_bridge_vap_xmit_etc::the vap alway tx/rx!}\r\n");
        oal_netbuf_free(pst_buf);
        return OAL_NETDEV_TX_OK;
    }
#endif

    pst_buf = oal_netbuf_unshare(pst_buf, GFP_ATOMIC);
    if (OAL_UNLIKELY(pst_buf == OAL_PTR_NULL)) {
        OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_TX,
                      "{hmac_bridge_vap_xmit_etc::the unshare netbuf = OAL_PTR_NULL!}\r\n");
        return OAL_NETDEV_TX_OK;
    }
#ifdef _PRE_WLAN_PKT_TIME_STAT
    if (DELAY_STATISTIC_SWITCH_ON && IS_NEED_RECORD_DELAY(pst_buf, TP_SKB_IP)) {
        skbprobe_record_time(pst_buf, TP_SKB_HMAC_XMIT);
    }
#endif

    /* ����̫��������֡�ϱ�SDT */
    hmac_tx_report_eth_frame_etc(pst_vap, pst_buf);

    if (OAL_GET_THRUPUT_BYPASS_ENABLE(OAL_TX_LINUX_BYPASS)) {
        oal_netbuf_free(pst_buf);
        return OAL_NETDEV_TX_OK;
    }

    /* ����VAP״̬������滥�⣬��Ҫ�������� */
    oal_spin_lock_bh(&pst_hmac_vap->st_lock_state);

    /* �ж�VAP��״̬�����ROAM���������� MAC_DATA_DHCP/MAC_DATA_ARP */
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (pst_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        uc_data_type = mac_get_data_type_from_8023_etc((oal_uint8 *)oal_netbuf_payload(pst_buf),
                                                       MAC_NETBUFF_PAYLOAD_ETH);
        if (uc_data_type != MAC_DATA_EAPOL) {
            oal_netbuf_free(pst_buf);
            oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
            return OAL_NETDEV_TX_OK;
        }
    } else {
#endif  // _PRE_WLAN_FEATURE_ROAM
        /* �ж�VAP��״̬�����û��UP/PAUSE����������. */
        if (OAL_UNLIKELY(!((pst_vap->en_vap_state == MAC_VAP_STATE_UP) ||
                           (pst_vap->en_vap_state == MAC_VAP_STATE_PAUSE)))) {
            /* ���������д���p2pɨ�費��dhcp��eapol֡����ֹ����ʧ�� */
            if (pst_vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
                en_drop_frame = OAL_TRUE;
            } else {
                uc_data_type = mac_get_data_type_from_8023_etc((oal_uint8 *)oal_netbuf_payload(pst_buf),
                                                               MAC_NETBUFF_PAYLOAD_ETH);
                if ((uc_data_type != MAC_DATA_EAPOL) && (uc_data_type != MAC_DATA_DHCP)) {
                    en_drop_frame = OAL_TRUE;
                } else {
                    OAM_WARNING_LOG2(pst_vap->uc_vap_id, OAM_SF_TX,
                                     "{hmac_bridge_vap_xmit_etc::donot drop [%d]frame[EAPOL:1,DHCP:0]. vap state[%d].}",
                                     uc_data_type, pst_vap->en_vap_state);
                }
            }

            if (en_drop_frame == OAL_TRUE) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                /* filter the tx xmit pkts print */
                if (pst_vap->en_vap_state == MAC_VAP_STATE_INIT || pst_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
                    OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                                     "{hmac_bridge_vap_xmit_etc::vap state[%d] != MAC_VAP_STATE_{UP|PAUSE}!}\r\n",
                                     pst_vap->en_vap_state);
                } else {
                    OAM_INFO_LOG1(pst_vap->uc_vap_id, OAM_SF_TX,
                                  "{hmac_bridge_vap_xmit_etc::vap state[%d] != MAC_VAP_STATE_{UP|PAUSE}!}\r\n",
                                  pst_vap->en_vap_state);
                }
#endif
                oal_netbuf_free(pst_buf);
                OAM_STAT_VAP_INCR(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);

                oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
                return OAL_NETDEV_TX_OK;
            }
        }
#ifdef _PRE_WLAN_FEATURE_ROAM
    }
#endif

    OAL_NETBUF_NEXT(pst_buf) = OAL_PTR_NULL;
    OAL_NETBUF_PREV(pst_buf) = OAL_PTR_NULL;

#ifdef _PRE_WLAN_PKT_TIME_STAT
    memset_s(OAL_NETBUF_CB(pst_buf), OAL_NETBUF_CB_ORIGIN, 0, OAL_NETBUF_CB_ORIGIN);
#else
    memset_s(OAL_NETBUF_CB(pst_buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
#endif

    if (IS_STA(&(pst_hmac_vap->st_vap_base_info))) {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* ���ͷ����arp_req ͳ�ƺ�ɾba�Ĵ��� */
        hmac_btcoex_arp_fail_delba_process_etc(pst_buf, &(pst_hmac_vap->st_vap_base_info));
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
        /* ���ͷ����arp_req ͳ�ƺ��ع����Ĵ��� */
        hmac_m2s_arp_fail_process(pst_buf, pst_hmac_vap);
#endif
    }

    oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
    OAL_MIPS_TX_STATISTIC(HMAC_PROFILING_FUNC_BRIDGE_VAP_XMIT);

    ul_ret = hmac_tx_lan_to_wlan_etc(pst_vap, pst_buf);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        /* ����ʧ�ܣ�Ҫ�ͷ��ں������netbuff�ڴ�� */
        oal_netbuf_free(pst_buf);
    }

    return OAL_NETDEV_TX_OK;
}

/* ���ڲ����������� */
oal_uint8 g_uc_tx_ba_policy_select = OAL_TRUE;


oal_void hmac_tx_ba_cnt_vary_etc(hmac_vap_stru *pst_hmac_vap,
                                 hmac_user_stru *pst_hmac_user,
                                 oal_uint8 uc_tidno,
                                 oal_netbuf_stru *pst_buf)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32 ul_current_timestamp;

    if ((pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (g_uc_tx_ba_policy_select == OAL_TRUE)) {
        
        pst_hmac_user->auc_ba_flag[uc_tidno]++;
    } else {
        ul_current_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

        /* ��һ����ֱ�Ӽ�����
           ��ʱ����������ʱ����ʼ����BA;
           TCP ACK�ظ�����������ʱ�����ơ� */
        if ((pst_hmac_user->auc_ba_flag[uc_tidno] == 0) ||
            (oal_netbuf_is_tcp_ack_etc((oal_ip_header_stru *)(oal_netbuf_data(pst_buf) + ETHER_HDR_LEN))) ||
            ((oal_uint32)OAL_TIME_GET_RUNTIME(pst_hmac_user->aul_last_timestamp[uc_tidno], ul_current_timestamp) <
             WLAN_BA_CNT_INTERVAL)) {
            pst_hmac_user->auc_ba_flag[uc_tidno]++;
        } else {
            pst_hmac_user->auc_ba_flag[uc_tidno] = 0;
        }

        pst_hmac_user->aul_last_timestamp[uc_tidno] = ul_current_timestamp;
    }
#endif
}

/*lint -e19*/
oal_module_symbol(hmac_tx_wlan_to_wlan_ap_etc);
oal_module_symbol(hmac_tx_lan_to_wlan_etc);
oal_module_symbol(hmac_free_netbuf_list_etc);

oal_module_symbol(hmac_tx_report_eth_frame_etc);
oal_module_symbol(hmac_bridge_vap_xmit_etc);

/*lint +e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

