

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 ͷ�ļ����� */
#include "oal_profiling.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oal_net.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_rx_data.h"
#include "dmac_ext_if.h"
#include "hmac_vap.h"
#include "hmac_ext_if.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "mac_vap.h"
#ifdef _PRE_WLAN_FEATURE_ISOLATION
#include "hmac_isolation.h"
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif
#include "hmac_blockack.h"
#include "hmac_tcp_opt.h"

#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
#include "hmac_wds.h"
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
#include "hmac_car.h"
#endif

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
#include <linux/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/ieee80211.h>
#include <linux/ipv6.h>
#endif

#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING
#include <linux/inetdevice.h>
#endif

#include <hmac_auto_adjust_freq.h>  // Ϊhmac_auto_adjust_freq.cͳ���հ���׼��

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif

#ifdef CONFIG_HUAWEI_DUBAI
#include <chipset_common/dubai/dubai.h>
#endif

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_DATA_C

/* 2 ȫ�ֱ������� */
/* 3 ����ʵ�� */

#ifdef _PRE_WLAN_DFT_DUMP_FRAME
oal_void hmac_rx_report_eth_frame_etc(mac_vap_stru *pst_mac_vap,
                                      oal_netbuf_stru *pst_netbuf)
{
    oal_uint16 us_user_idx = 0;
    mac_ether_header_stru *pst_ether_hdr = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 auc_user_macaddr[WLAN_MAC_ADDR_LEN] = { 0 };
    oal_switch_enum_uint8 en_eth_switch = 0;

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        return;
    }

    /* ��skb��dataָ��ָ����̫����֡ͷ */
    oal_netbuf_push(pst_netbuf, ETHER_HDR_LEN);

    /* ��ȡĿ���û���Դ��id */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
        if (OAL_UNLIKELY(pst_ether_hdr == OAL_PTR_NULL)) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{hmac_rx_report_eth_frame_etc::pst_ether_hdr null.}");
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            return;
        }

        ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap, pst_ether_hdr->auc_ether_shost, &us_user_idx);
        if (ul_ret == OAL_ERR_CODE_PTR_NULL) {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{hmac_rx_report_eth_frame_etc::ul_ret null.}");
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            return;
        }

        if (ul_ret == OAL_FAIL) {
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            return;
        }

        oal_set_mac_addr(auc_user_macaddr, pst_ether_hdr->auc_ether_shost);
    } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_mac_vap->us_user_nums == 0) {
            oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
            /* SUCC , return */
            return;
        }

        us_user_idx = pst_mac_vap->us_assoc_vap_id;
        oal_set_mac_addr(auc_user_macaddr, pst_mac_vap->auc_bssid);
    }

    ul_ret = oam_report_eth_frame_get_switch_etc(us_user_idx, OAM_OTA_FRAME_DIRECTION_TYPE_RX, &en_eth_switch);
    if (ul_ret != OAL_SUCC) {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                         "{hmac_rx_report_eth_frame_etc::oam_report_eth_frame_get_switch_etc failed[%d].}", ul_ret);
        oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
        return;
    }

    if (en_eth_switch == OAL_SWITCH_ON) {
        /* ��Ҫ������̫����֡�ϱ� */
        ul_ret = oam_report_eth_frame_etc(auc_user_macaddr,
                                          oal_netbuf_data(pst_netbuf),
                                          (oal_uint16)OAL_NETBUF_LEN(pst_netbuf),
                                          OAM_OTA_FRAME_DIRECTION_TYPE_RX);
        if (ul_ret != OAL_SUCC) {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                             "{hmac_rx_report_eth_frame_etc::oam_report_eth_frame_etc return err: 0x%x.}\r\n", ul_ret);
        }
    }

    oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
}
#endif


OAL_STATIC OAL_INLINE oal_void hmac_rx_frame_80211_to_eth(oal_netbuf_stru *pst_netbuf,
                                                          const unsigned char *puc_da,
                                                          const unsigned char *puc_sa)
{
    mac_ether_header_stru *pst_ether_hdr = OAL_PTR_NULL;
    mac_llc_snap_stru *pst_snap;
    oal_uint8 ul_offset_len;

    /* ע��:Ĭ��ʹ��protocol��ʽ��snapͷ��length��ʽsnapͷ��ʽ��ǰ�����ֽڸ�ʽ��protocol��ʽ��ͬ */
    pst_snap = (mac_llc_snap_stru *)oal_netbuf_data(pst_netbuf);

    if (OAL_LIKELY(mac_snap_is_protocol_type(MAC_GET_SNAP_TYPE(pst_snap)))) {
        ul_offset_len = HMAC_RX_DATA_ETHER_OFFSET_LENGTH;
    } else {
        ul_offset_len = MAC_SUBMSDU_HEADER_LEN;
    }

    /*
     * ��payload��ǰ����һ��������������̫��ͷ��14�ֽڿռ䡣protocl��ʽ����̫ͷ�Ḳ��ԭ����snapͷ��
     * length��ʽ�ĻὫsnapͷ��Ϊ���ݲ���
     */
    oal_netbuf_push(pst_netbuf, ul_offset_len);
    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

    pst_ether_hdr->us_ether_type =
        (ul_offset_len == HMAC_RX_DATA_ETHER_OFFSET_LENGTH) ? pst_snap->us_ether_type : pst_netbuf->len;
    oal_set_mac_addr(pst_ether_hdr->auc_ether_shost, puc_sa);
    oal_set_mac_addr(pst_ether_hdr->auc_ether_dhost, puc_da);
}


oal_void hmac_rx_free_netbuf_etc(oal_netbuf_stru *pst_netbuf, oal_uint16 us_nums)
{
    oal_netbuf_stru *pst_netbuf_temp = OAL_PTR_NULL;
    oal_uint16 us_netbuf_num;

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_etc::pst_netbuf null.}\r\n");
        return;
    }

    for (us_netbuf_num = us_nums; us_netbuf_num > 0; us_netbuf_num--) {
        pst_netbuf_temp = OAL_NETBUF_NEXT(pst_netbuf);

        /* ����netbuf��Ӧ��user���ü��� */
        oal_netbuf_free(pst_netbuf);

        pst_netbuf = pst_netbuf_temp;

        if (pst_netbuf == OAL_PTR_NULL) {
            if (OAL_UNLIKELY(us_netbuf_num != 1)) {
                OAM_ERROR_LOG2(0, OAM_SF_RX,
                               "{hmac_rx_free_netbuf_etc::pst_netbuf list broken, us_netbuf_num[%d]us_nums[%d].}",
                               us_netbuf_num, us_nums);
                return;
            }
            break;
        }
    }
}


oal_void hmac_rx_free_netbuf_list_etc(oal_netbuf_head_stru *pst_netbuf_hdr, oal_uint16 uc_num_buf)
{
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    oal_uint16 us_idx;

    if (OAL_UNLIKELY(pst_netbuf_hdr == OAL_PTR_NULL)) {
        OAM_INFO_LOG0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list_etc::pst_netbuf null.}");
        return;
    }

    OAM_INFO_LOG1(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list_etc::free [%d].}", uc_num_buf);

    for (us_idx = uc_num_buf; us_idx > 0; us_idx--) {
        pst_netbuf = oal_netbuf_delist(pst_netbuf_hdr);
        if (pst_netbuf != OAL_PTR_NULL) {
            OAM_INFO_LOG0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list_etc::pst_netbuf null.}");
            oal_netbuf_free(pst_netbuf);
        }
    }
}


OAL_STATIC oal_uint32 hmac_rx_transmit_to_wlan(frw_event_hdr_stru *pst_event_hdr,
                                               oal_netbuf_head_stru *pst_netbuf_head)
{
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL; /* ��netbuf����ȡ������ָ��netbuf��ָ�� */
    oal_uint32 ul_netbuf_num;
    oal_uint32 ul_ret;
    oal_netbuf_stru *pst_buf_tmp = OAL_PTR_NULL; /* �ݴ�netbufָ�룬����whileѭ�� */
    mac_tx_ctl_stru *pst_tx_ctl = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR2(pst_event_hdr, pst_netbuf_head))) {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_transmit_to_wlan::param null, %d %d.}",
                       (uintptr_t)pst_event_hdr, (uintptr_t)pst_netbuf_head);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡ��ͷ��net buffer */
    pst_netbuf = oal_netbuf_peek(pst_netbuf_head);

    /* ��ȡmac vap �ṹ */
    ul_ret = hmac_tx_get_mac_vap_etc(pst_event_hdr->uc_vap_id, &pst_mac_vap);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        ul_netbuf_num = oal_netbuf_list_len(pst_netbuf_head);
        hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)ul_netbuf_num);
        OAM_WARNING_LOG3(pst_event_hdr->uc_vap_id, OAM_SF_RX,
                         "{hmac_rx_transmit_to_wlan::find vap [%d] failed[%d], free [%d] netbuffer.}",
                         pst_event_hdr->uc_vap_id, ul_ret, ul_netbuf_num);
        return ul_ret;
    }

    /* ѭ������ÿһ��netbuf��������̫��֡�ķ�ʽ���� */
    while (pst_netbuf != OAL_PTR_NULL) {
        pst_buf_tmp = OAL_NETBUF_NEXT(pst_netbuf);

        OAL_NETBUF_NEXT(pst_netbuf) = OAL_PTR_NULL;
        OAL_NETBUF_PREV(pst_netbuf) = OAL_PTR_NULL;

        pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);
        memset_s(pst_tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

        MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) = FRW_EVENT_TYPE_WLAN_DTX;
        MAC_GET_CB_EVENT_SUBTYPE(pst_tx_ctl) = DMAC_TX_WLAN_DTX;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* set the queue map id when wlan to wlan */
        oal_skb_set_queue_mapping(pst_netbuf, WLAN_NORMAL_QUEUE);
#endif

        ul_ret = hmac_tx_lan_to_wlan_etc(pst_mac_vap, pst_netbuf);

        /* ����ʧ�ܣ��Լ������Լ��ͷ�netbuff�ڴ� */
        if (ul_ret != OAL_SUCC) {
            hmac_free_netbuf_list_etc(pst_netbuf);
        }

        pst_netbuf = pst_buf_tmp;
    }

    return OAL_SUCC;
}


oal_void hmac_rx_free_amsdu_netbuf(oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru *pst_netbuf_next = OAL_PTR_NULL;
    while (pst_netbuf != OAL_PTR_NULL) {
        pst_netbuf_next = oal_get_netbuf_next(pst_netbuf);
        oal_netbuf_free(pst_netbuf);
        pst_netbuf = pst_netbuf_next;
    }
}


oal_void hmac_rx_clear_amsdu_last_netbuf_pointer(oal_netbuf_stru *pst_netbuf, oal_uint16 uc_num_buf)
{
    while (pst_netbuf != OAL_PTR_NULL) {
        uc_num_buf--;
        if (uc_num_buf == 0) {
            pst_netbuf->next = OAL_PTR_NULL;
            break;
        }
        pst_netbuf = oal_get_netbuf_next(pst_netbuf);
    }
}


OAL_STATIC oal_uint32 hmac_rx_init_amsdu_state(oal_netbuf_stru *pst_netbuf, dmac_msdu_proc_state_stru *pst_msdu_state)
{
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL; /* MPDU�Ŀ�����Ϣ */
    oal_uint32       ul_need_pull_len;

    /* �״ν���ú�������AMSDU */
    if (OAL_ALL_ZERO_VALUE2(pst_msdu_state->uc_procd_netbuf_nums, pst_msdu_state->uc_procd_msdu_in_netbuf)) {
        pst_msdu_state->pst_curr_netbuf      = pst_netbuf;

        /* AMSDUʱ���׸�netbuf���а���802.11ͷ����Ӧ��payload��Ҫƫ�� */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_msdu_state->pst_curr_netbuf);

        pst_msdu_state->puc_curr_netbuf_data   = (oal_uint8*)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl) +
                                                 pst_rx_ctrl->uc_mac_header_len;
        pst_msdu_state->uc_netbuf_nums_in_mpdu = pst_rx_ctrl->bit_buff_nums;
        pst_msdu_state->uc_msdu_nums_in_netbuf = pst_rx_ctrl->uc_msdu_in_buffer;
        pst_msdu_state->us_submsdu_offset      = 0;

        /* ʹnetbuf ָ��amsdu ֡ͷ */
        ul_need_pull_len = (oal_uint32)(pst_msdu_state->puc_curr_netbuf_data - oal_netbuf_payload(pst_netbuf));
        oal_netbuf_pull(pst_msdu_state->pst_curr_netbuf, ul_need_pull_len);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_rx_amsdu_check_frame_len(oal_netbuf_stru *pst_netbuf,
                                    dmac_msdu_proc_state_stru *pst_msdu_state,
                                    oal_uint16 us_submsdu_len,
                                    oal_uint8 uc_submsdu_pad_len)
{
    if (us_submsdu_len > WLAN_MAX_NETBUF_SIZE ||
        (pst_msdu_state->us_submsdu_offset + MAC_SUBMSDU_HEADER_LEN + us_submsdu_len >
            OAL_NETBUF_LEN(pst_netbuf))) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_rx_parse_amsdu_etc(oal_netbuf_stru *pst_netbuf,
                                   dmac_msdu_stru *pst_msdu,
                                   dmac_msdu_proc_state_stru *pst_msdu_state,
                                   mac_msdu_proc_status_enum_uint8 *pen_proc_state)
{
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;    /* MPDU�Ŀ�����Ϣ */
    oal_uint8 *puc_buffer_data_addr = OAL_PTR_NULL; /* ָ��netbuf�������ָ�� */
    oal_uint16 us_offset;                           /* submsdu�����dataָ���ƫ�� */
    oal_uint16 us_submsdu_len = 0;                  /* submsdu�ĳ��� */
    oal_uint8 uc_submsdu_pad_len = 0;               /* submsdu����䳤�� */
    oal_uint8 *puc_submsdu_hdr = OAL_PTR_NULL;      /* ָ��submsduͷ����ָ�� */
    oal_netbuf_stru *pst_netbuf_prev = OAL_PTR_NULL;
    oal_bool_enum_uint8 b_need_free_netbuf = OAL_FALSE;

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu_etc::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �״ν���ú�������AMSDU */
    hmac_rx_init_amsdu_state(pst_netbuf, pst_msdu_state);

    /* ��ȡsubmsdu��ͷָ�� */
    puc_buffer_data_addr = pst_msdu_state->puc_curr_netbuf_data;
    us_offset = pst_msdu_state->us_submsdu_offset;
    puc_submsdu_hdr = puc_buffer_data_addr + us_offset;

    /* 1��netbuf ֻ����һ��msdu */
    if (pst_msdu_state->uc_msdu_nums_in_netbuf == 1) {
        mac_get_submsdu_len(puc_submsdu_hdr, &us_submsdu_len);
        oal_set_mac_addr(pst_msdu->auc_sa, (puc_submsdu_hdr + MAC_SUBMSDU_SA_OFFSET));
        oal_set_mac_addr(pst_msdu->auc_da, (puc_submsdu_hdr + MAC_SUBMSDU_DA_OFFSET));

        /* ָ��amsdu֡�� */
        oal_netbuf_pull(pst_msdu_state->pst_curr_netbuf, MAC_SUBMSDU_HEADER_LEN);

        if (us_submsdu_len > OAL_NETBUF_LEN(pst_msdu_state->pst_curr_netbuf)) {
            *pen_proc_state = MAC_PROC_ERROR;
            OAM_WARNING_LOG2(0, OAM_SF_RX,
                             "{hmac_rx_parse_amsdu_etc::us_submsdu_len %d is not valid netbuf len=%d.}",
                             us_submsdu_len,
                             OAL_NETBUF_LEN(pst_msdu_state->pst_curr_netbuf));
            hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
            return OAL_FAIL;
        }

        oal_netbuf_trim(pst_msdu_state->pst_curr_netbuf, OAL_NETBUF_LEN(pst_msdu_state->pst_curr_netbuf));

        oal_netbuf_put(pst_msdu_state->pst_curr_netbuf, us_submsdu_len);

        /* ֱ��ʹ�ø�netbuf�ϱ����ں� ��ȥһ��netbuf����Ϳ��� */
        b_need_free_netbuf = OAL_FALSE;
        pst_msdu->pst_netbuf = pst_msdu_state->pst_curr_netbuf;

    } else {
        /* ��ȡsubmsdu�������Ϣ */
        mac_get_submsdu_len(puc_submsdu_hdr, &us_submsdu_len);
        mac_get_submsdu_pad_len(MAC_SUBMSDU_HEADER_LEN + us_submsdu_len, &uc_submsdu_pad_len);
        oal_set_mac_addr(pst_msdu->auc_sa, (puc_submsdu_hdr + MAC_SUBMSDU_SA_OFFSET));
        oal_set_mac_addr(pst_msdu->auc_da, (puc_submsdu_hdr + MAC_SUBMSDU_DA_OFFSET));

        if (hmac_rx_amsdu_check_frame_len(pst_netbuf, pst_msdu_state, us_submsdu_len, uc_submsdu_pad_len) != OAL_SUCC) {
            OAM_WARNING_LOG4(0, OAM_SF_RX,
                "hmac_rx_parse_amsdu_etc::submsdu not valid. submsdu_len %d, offset %d. msdu_nums %d, procd_msdu %d.",
                us_submsdu_len,
                pst_msdu_state->us_submsdu_offset,
                pst_msdu_state->uc_msdu_nums_in_netbuf,
                pst_msdu_state->uc_procd_msdu_in_netbuf);
            OAM_STAT_VAP_INCR(0, rx_no_buff_dropped, 1);
            hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
            return OAL_FAIL;
        }

        /* ��Ե�ǰ��netbuf�������µ�subnetbuf�������ö�Ӧ��netbuf����Ϣ����ֵ����Ӧ��msdu */
        pst_msdu->pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
                                                    (MAC_SUBMSDU_HEADER_LEN + us_submsdu_len + uc_submsdu_pad_len),
                                                    OAL_NETBUF_PRIORITY_MID);
        if (pst_msdu->pst_netbuf == OAL_PTR_NULL) {
            OAM_ERROR_LOG3(0, OAM_SF_RX,
                "{hmac_rx_parse_amsdu_etc::alloc netbuf fail. submsdu len %d, pad_len %d, total_len %d}",
                us_submsdu_len,
                uc_submsdu_pad_len,
                (MAC_SUBMSDU_HEADER_LEN + us_submsdu_len + uc_submsdu_pad_len));
            OAM_STAT_VAP_INCR(0, rx_no_buff_dropped, 1);
            hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
            return OAL_FAIL;
        }

        OAL_MEM_NETBUF_TRACE(pst_msdu->pst_netbuf, OAL_TRUE);

        /* ���ÿһ����msdu���޸�netbuf��end��data��tail��lenָ�� */
        oal_netbuf_put(pst_msdu->pst_netbuf, us_submsdu_len + MAC_SUBMSDU_HEADER_LEN);
        oal_netbuf_pull(pst_msdu->pst_netbuf, MAC_SUBMSDU_HEADER_LEN);
        if (EOK != memcpy_s(pst_msdu->pst_netbuf->data, us_submsdu_len + uc_submsdu_pad_len,
                            (puc_submsdu_hdr + MAC_SUBMSDU_HEADER_LEN), us_submsdu_len)) {
            OAM_WARNING_LOG4(0, OAM_SF_RX,
                "{hmac_rx_parse_amsdu_etc::memcpy fail. submsdu len %d, pad_len %d. msdu_nums %d, procd_msdu %d.}",
                us_submsdu_len,
                uc_submsdu_pad_len,
                pst_msdu_state->uc_msdu_nums_in_netbuf,
                pst_msdu_state->uc_procd_msdu_in_netbuf);
        }

        b_need_free_netbuf = OAL_TRUE;
    }

    /* ���ӵ�ǰ�Ѵ�����msdu�ĸ��� */
    pst_msdu_state->uc_procd_msdu_in_netbuf++;

    /* ��ȡ��ǰ��netbuf�е���һ��msdu���д��� */
    if (pst_msdu_state->uc_procd_msdu_in_netbuf < pst_msdu_state->uc_msdu_nums_in_netbuf) {
        pst_msdu_state->us_submsdu_offset += us_submsdu_len + uc_submsdu_pad_len + MAC_SUBMSDU_HEADER_LEN;
    } else if (pst_msdu_state->uc_procd_msdu_in_netbuf == pst_msdu_state->uc_msdu_nums_in_netbuf) {
        pst_msdu_state->uc_procd_netbuf_nums++;

        pst_netbuf_prev = pst_msdu_state->pst_curr_netbuf;

        /* ��ȡ��MPDU��Ӧ����һ��netbuf������ */
        if (pst_msdu_state->uc_procd_netbuf_nums < pst_msdu_state->uc_netbuf_nums_in_mpdu) {
            pst_msdu_state->pst_curr_netbuf = OAL_NETBUF_NEXT(pst_msdu_state->pst_curr_netbuf);
            pst_msdu_state->puc_curr_netbuf_data = oal_netbuf_data(pst_msdu_state->pst_curr_netbuf);

            pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_msdu_state->pst_curr_netbuf);

            pst_msdu_state->uc_msdu_nums_in_netbuf = pst_rx_ctrl->uc_msdu_in_buffer;
            pst_msdu_state->us_submsdu_offset = 0;
            pst_msdu_state->uc_procd_msdu_in_netbuf = 0;

            /* amsdu �ڶ���netbuf len��0, ��put�����size */
            oal_netbuf_put(pst_msdu_state->pst_curr_netbuf, WLAN_MEM_NETBUF_SIZE2);
        } else if (pst_msdu_state->uc_procd_netbuf_nums == pst_msdu_state->uc_netbuf_nums_in_mpdu) {
            *pen_proc_state = MAC_PROC_LAST_MSDU;
            if (b_need_free_netbuf) {
                oal_netbuf_free(pst_netbuf_prev);
            }
            return OAL_SUCC;
        } else {
            *pen_proc_state = MAC_PROC_ERROR;
            OAM_WARNING_LOG0(0, OAM_SF_RX,
                "{hmac_rx_parse_amsdu_etc::pen_proc_state is err for uc_procd_netbuf_nums > uc_netbuf_nums_in_mpdul.}");
            hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
            return OAL_FAIL;
        }
        if (b_need_free_netbuf) {
            oal_netbuf_free(pst_netbuf_prev);
        }
    } else {
        *pen_proc_state = MAC_PROC_ERROR;
        OAM_WARNING_LOG0(0, OAM_SF_RX,
            "{hmac_rx_parse_amsdu_etc::pen_proc_state is err for uc_procd_netbuf_nums > uc_netbuf_nums_in_mpdul.}");
        hmac_rx_free_amsdu_netbuf(pst_msdu_state->pst_curr_netbuf);
        return OAL_FAIL;
    }

    *pen_proc_state = MAC_PROC_MORE_MSDU;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 hmac_rx_prepare_msdu_list_to_wlan(hmac_vap_stru *pst_vap,
                                                        oal_netbuf_head_stru *pst_netbuf_header,
                                                        oal_netbuf_stru *pst_netbuf,
                                                        mac_ieee80211_frame_stru *pst_frame_hdr)
{
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;                      /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    dmac_msdu_stru st_msdu;                                           /* �������������ÿһ��MSDU */
    mac_msdu_proc_status_enum_uint8 en_process_state = MAC_PROC_BUTT; /* ����AMSDU��״̬ */
    dmac_msdu_proc_state_stru st_msdu_state = { 0 };                  /* ��¼MPDU�Ĵ�����Ϣ */
    oal_uint8 *puc_addr = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    oal_uint8 auc_sa[WLAN_MAC_ADDR_LEN];
    oal_uint8 auc_da[WLAN_MAC_ADDR_LEN];
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_prepare_msdu_list_to_wlan::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ����MPDU-->MSDU */ /* ��MSDU���netbuf�� */
    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);

    /* ��ȡ��MPDU�Ŀ�����Ϣ */
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    memset_s(&st_msdu, OAL_SIZEOF(dmac_msdu_stru), 0, OAL_SIZEOF(dmac_msdu_stru));

    /* ���һ:����AMSDU�ۺϣ����MPDU��Ӧһ��MSDU��ͬʱ��Ӧһ��NETBUF,��MSDU��ԭ
       ����̫����ʽ֡�Ժ�ֱ�Ӽ��뵽netbuf�������
    */
    if (pst_rx_ctrl->bit_amsdu_enable == OAL_FALSE) {
        pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl));

        if (OAL_UNLIKELY(pst_hmac_user == OAL_PTR_NULL)) {
            /* ��ӡ��net buf�����Ϣ */
            mac_rx_report_80211_frame_etc((oal_uint8 *)&(pst_vap->st_vap_base_info),
                                          (oal_uint8 *)pst_rx_ctrl,
                                          pst_netbuf,
                                          OAM_OTA_TYPE_RX_HMAC_CB);
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_netbuf = hmac_defrag_process_etc(pst_hmac_user, pst_netbuf, pst_rx_ctrl->uc_mac_header_len);
        if (pst_netbuf == OAL_PTR_NULL) {
            return OAL_SUCC;
        }

        /* ���»�ȡ��MPDU�Ŀ�����Ϣ */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

        pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl);

        /* ��MACͷ�л�ȡԴ��ַ��Ŀ�ĵ�ַ */
        mac_rx_get_sa(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(&auc_sa[0], puc_addr);

        mac_rx_get_da(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(&auc_da[0], puc_addr);

        /* ��netbuf��dataָ��ָ��mac frame��payload����Ҳ����ָ����8�ֽڵ�snapͷ */
        oal_netbuf_pull(pst_netbuf, pst_rx_ctrl->uc_mac_header_len);

        /* ��MSDUת��Ϊ��̫����ʽ��֡ */
        hmac_rx_frame_80211_to_eth(pst_netbuf, auc_da, auc_sa);

#ifdef _PRE_WLAN_PKT_TIME_STAT
        memset_s(OAL_NETBUF_CB(pst_netbuf), OAL_NETBUF_CB_ORIGIN, 0, OAL_NETBUF_CB_ORIGIN);
#else
        memset_s(OAL_NETBUF_CB(pst_netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
#endif
        /* ��MSDU���뵽netbuf������� */
        oal_netbuf_add_to_list_tail(pst_netbuf, pst_netbuf_header);

    }

    else { /* �����:AMSDU�ۺ� */
        st_msdu_state.uc_procd_netbuf_nums = 0;
        st_msdu_state.uc_procd_msdu_in_netbuf = 0;

        /* amsdu ���һ��netbuf nextָ����Ϊ NULL ����ʱ�����ͷ�amsdu netbuf */
        hmac_rx_clear_amsdu_last_netbuf_pointer(pst_netbuf, pst_rx_ctrl->bit_buff_nums);

        do {
            /* ��ȡ��һ��Ҫת����msdu */
            ul_ret = hmac_rx_parse_amsdu_etc(pst_netbuf, &st_msdu, &st_msdu_state, &en_process_state);
            if (ul_ret != OAL_SUCC) {
                OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "{hmac_rx_prepare_msdu_list_to_wlan::hmac_rx_parse_amsdu_etc failed[%d].}", ul_ret);
                return ul_ret;
            }

            /* ��MSDUת��Ϊ��̫����ʽ��֡ */
            hmac_rx_frame_80211_to_eth(st_msdu.pst_netbuf, st_msdu.auc_da, st_msdu.auc_sa);

            /* ��MSDU���뵽netbuf������� */
            oal_netbuf_add_to_list_tail(st_msdu.pst_netbuf, pst_netbuf_header);

        } while (en_process_state != MAC_PROC_LAST_MSDU);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
OAL_STATIC oal_void hmac_pkt_mem_opt_stat_reset(hmac_device_stru *pst_hmac_device,
                                                oal_bool_enum_uint8 en_dscr_opt_state)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    hmac_rx_dscr_opt_stru *pst_dscr_opt = &pst_hmac_device->st_rx_dscr_opt;

    if (OAL_UNLIKELY(pst_hmac_device->pst_device_base_info == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_pkt_mem_opt_stat_reset::pst_device_base_info null!}");
        return;
    }

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_stat_reset::new_state[%d], pkt_num[%d]}",
                     en_dscr_opt_state, pst_dscr_opt->ul_rx_pkt_num);
    pst_dscr_opt->en_dscr_opt_state = en_dscr_opt_state;
    pst_dscr_opt->ul_rx_pkt_num = 0;

    /***************************************************************************
        ���¼���dmacģ��,��ͳ����Ϣ����dmac
    ***************************************************************************/
    pst_event_mem = FRW_EVENT_ALLOC(0);
    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_timeout_fn::pst_event_mem null.}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* ��д�¼�ͷ */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_DSCR_OPT,
                       0,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hmac_device->pst_device_base_info->uc_chip_id,
                       pst_hmac_device->pst_device_base_info->uc_device_id,
                       0);

    /* �������� */
    pst_event->auc_event_data[0] = pst_dscr_opt->en_dscr_opt_state;

    /* �ַ��¼� */
    frw_event_dispatch_event_etc(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);
}


oal_void hmac_pkt_mem_opt_cfg_etc(oal_uint32 ul_cfg_tpye, oal_uint32 ul_cfg_value)
{
    hmac_device_stru *pst_hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(0);
    hmac_rx_dscr_opt_stru *pst_dscr_opt;

    if (ul_cfg_tpye > 2) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_cfg::invalid cfg tpye.}");
        return;
    }
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_cfg::hmac device is null.}");
        return;
    }

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_cfg::cfg type[%d], cfg value[%d].}", ul_cfg_tpye, ul_cfg_value);
    pst_dscr_opt = &pst_hmac_device->st_rx_dscr_opt;
    if (ul_cfg_tpye == 0) {
        pst_dscr_opt->en_dscr_opt_enable = (oal_uint8)ul_cfg_value;
        if (pst_dscr_opt->en_dscr_opt_enable == OAL_FALSE && pst_dscr_opt->en_dscr_opt_state == OAL_TRUE) {
            hmac_pkt_mem_opt_stat_reset(pst_hmac_device, OAL_FALSE);
        }
    }
    if (ul_cfg_tpye == 1) {
        pst_dscr_opt->ul_rx_pkt_opt_limit = ul_cfg_value;
    }
    if (ul_cfg_tpye == 2) {
        pst_dscr_opt->ul_rx_pkt_reset_limit = ul_cfg_value;
    }
}

oal_uint32 hmac_pkt_mem_opt_timeout_fn_etc(oal_void *p_arg)
{
    hmac_device_stru *pst_hmac_device;
    hmac_rx_dscr_opt_stru *pst_dscr_opt;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_device = (hmac_device_stru *)p_arg;
    pst_dscr_opt = &pst_hmac_device->st_rx_dscr_opt;

    if (pst_dscr_opt->en_dscr_opt_enable != OAL_TRUE) {
        return OAL_SUCC;
    }

    OAM_INFO_LOG2(0, OAM_SF_ANY, "{hmac_rx_dscr_opt_timeout_fn::state[%d], pkt_num[%d]}",
                  pst_dscr_opt->en_dscr_opt_state, pst_dscr_opt->ul_rx_pkt_num);

    /* rx_dscrδ����״̬ʱ, ��⵽RXҵ��,���������� */
    if (pst_dscr_opt->en_dscr_opt_state == OAL_FALSE &&
        pst_dscr_opt->ul_rx_pkt_num > pst_dscr_opt->ul_rx_pkt_opt_limit) {
        hmac_pkt_mem_opt_stat_reset(pst_hmac_device, OAL_TRUE);
    }
    /* rx_dscr�ѵ���״̬ʱ, δ��⵽RXҵ��,������������,��֤TX���� */
    else if (pst_dscr_opt->en_dscr_opt_state == OAL_TRUE &&
             pst_dscr_opt->ul_rx_pkt_num < pst_dscr_opt->ul_rx_pkt_reset_limit) {
        hmac_pkt_mem_opt_stat_reset(pst_hmac_device, OAL_FALSE);
    } else {
        pst_dscr_opt->ul_rx_pkt_num = 0;
    }

    return OAL_SUCC;
}

oal_void hmac_pkt_mem_opt_init_etc(hmac_device_stru *pst_hmac_device)
{
    if (pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer.en_is_registerd == OAL_TRUE) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_pkt_mem_opt_init_etc::rx_dscr_timer is_registerd.}");
        return;
    }

    pst_hmac_device->st_rx_dscr_opt.en_dscr_opt_state = OAL_FALSE;
    pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_num = 0;
    pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_opt_limit = WLAN_PKT_MEM_PKT_OPT_LIMIT;
    pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_reset_limit = WLAN_PKT_MEM_PKT_RESET_LIMIT;
    pst_hmac_device->st_rx_dscr_opt.en_dscr_opt_enable = OAL_TRUE;

    FRW_TIMER_CREATE_TIMER(&(pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer),
                           hmac_pkt_mem_opt_timeout_fn_etc, WLAN_PKT_MEM_OPT_TIME_MS,
                           pst_hmac_device,
                           OAL_TRUE,
                           OAM_MODULE_ID_HMAC,
                           pst_hmac_device->pst_device_base_info->ul_core_id);
}

oal_void hmac_pkt_mem_opt_exit_etc(hmac_device_stru *pst_hmac_device)
{
    if (pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer.en_is_registerd == OAL_TRUE) {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hmac_device->st_rx_dscr_opt.st_rx_dscr_opt_timer));
    }
}

OAL_STATIC oal_void hmac_pkt_mem_opt_rx_pkts_stat(hmac_vap_stru *pst_vap, oal_ip_header_stru *pst_ip)
{
    hmac_device_stru *pst_hmac_device =
        (hmac_device_stru *)hmac_res_get_mac_dev_etc(pst_vap->st_vap_base_info.uc_device_id);

    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_pkt_mem_opt_rx_pkts_stat::hmac_res_get_mac_dev_etc fail.device_id :%d}",
                         pst_vap->st_vap_base_info.uc_device_id);
        return;
    }
    /* ����IP_LEN С�� HMAC_RX_DSCR_OPT_MIN_PKT_LEN�ı��� */
    if (OAL_NET2HOST_SHORT(pst_ip->us_tot_len) < WLAN_PKT_MEM_OPT_MIN_PKT_LEN) {
        return;
    }

    if ((pst_ip->uc_protocol == MAC_UDP_PROTOCAL) || (pst_ip->uc_protocol == MAC_TCP_PROTOCAL)) {
        pst_hmac_device->st_rx_dscr_opt.ul_rx_pkt_num++;
    } else {
        OAM_INFO_LOG0(0, OAM_SF_RX, "{hmac_rx_dscr_opt_rx_pkts_stat: neither UDP nor TCP ");
    }
}
#endif

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_is_tcp_udp_frame(oal_netbuf_stru *pst_netbuf)
{
    oal_ip_header_stru *pst_ip_header = OAL_PTR_NULL;
    oal_ipv6hdr_stru *pst_ipv6_header = OAL_PTR_NULL;
    mac_ether_header_stru *pst_ether_header = (mac_ether_header_stru *)(oal_netbuf_data(pst_netbuf) - ETHER_HDR_LEN);

    /*lint -e778*/
    if (OAL_HOST2NET_SHORT(ETHER_TYPE_IP) == pst_ether_header->us_ether_type) {
        /*lint +e778*/
        pst_ip_header = (oal_ip_header_stru *)(pst_ether_header + 1);

        if ((pst_ip_header->uc_protocol == OAL_IPPROTO_TCP) ||
            ((pst_ip_header->uc_protocol == OAL_IPPROTO_UDP) &&
             (OAL_FALSE == mac_is_dhcp_port_etc((mac_ip_header_stru *)pst_ip_header)))) { // DHCPֱ֡�Ӹ��ں�
            return OAL_TRUE;
        }
    } else if (OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6) == pst_ether_header->us_ether_type) {
        pst_ipv6_header = (oal_ipv6hdr_stru *)(pst_ether_header + 1);
        if ((pst_ipv6_header->nexthdr == OAL_IPPROTO_TCP) ||
            ((pst_ipv6_header->nexthdr == OAL_IPPROTO_UDP) &&
            (OAL_FALSE == mac_is_dhcp6_etc(pst_ipv6_header)))) { // DHCPֱ֡�Ӹ��ں�
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE

OAL_STATIC oal_void hmac_parse_ipv4_packet(oal_void *pst_eth)
{
    const struct iphdr *iph = OAL_PTR_NULL;
    oal_uint32 iphdr_len;
    struct tcphdr *th = OAL_PTR_NULL;
    struct udphdr *uh = OAL_PTR_NULL;
    struct icmphdr *icmph = OAL_PTR_NULL;

    OAL_IO_PRINT(WIFI_WAKESRC_TAG "ipv4 packet.\n");
    iph = (struct iphdr *)((mac_ether_header_stru *)pst_eth + 1);
    iphdr_len = iph->ihl * 4;

    OAL_IO_PRINT(WIFI_WAKESRC_TAG "src ip:%d.x.x.%d, dst ip:%d.x.x.%d\n", IPADDR(iph->saddr), IPADDR(iph->daddr));
    if (iph->protocol == IPPROTO_UDP) {
        uh = (struct udphdr *)((oal_uint8 *)iph + iphdr_len);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "UDP packet, src port:%d, dst port:%d.\n",
                     OAL_NTOH_16(uh->source), OAL_NTOH_16(uh->dest));
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP_UDP_V4", "port", OAL_NTOH_16(uh->dest));
#endif
    } else if (iph->protocol == IPPROTO_TCP) {
        th = (struct tcphdr *)((oal_uint8 *)iph + iphdr_len);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "TCP packet, src port:%d, dst port:%d.\n",
                     OAL_NTOH_16(th->source), OAL_NTOH_16(th->dest));
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP_TCP_V4", "port", OAL_NTOH_16(th->dest));
#endif
    } else if (iph->protocol == IPPROTO_ICMP) {
        icmph = (struct icmphdr *)((oal_uint8 *)iph + iphdr_len);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "ICMP packet, type(%d):%s, code:%d.\n", icmph->type,
            ((icmph->type == 0) ? "ping reply" : ((icmph->type == 8) ?
            "ping request" : "other icmp pkt")), icmph->code);
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", (int32_t)iph->protocol);
#endif
    } else if (iph->protocol == IPPROTO_IGMP) {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "IGMP packet.\n");
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", (int32_t)iph->protocol);
#endif
    } else {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "other IPv4 packet.\n");
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", (int32_t)iph->protocol);
#endif
    }

    return;
}



OAL_STATIC oal_void hmac_parse_ipv6_packet(oal_void *pst_eth)
{
    struct ipv6hdr *ipv6h = OAL_PTR_NULL;

    OAL_IO_PRINT(WIFI_WAKESRC_TAG "ipv6 packet.\n");
    ipv6h = (struct ipv6hdr *)((mac_ether_header_stru *)pst_eth + 1);
    OAL_IO_PRINT(WIFI_WAKESRC_TAG "version: %d, payload length: %d, nh->nexthdr: %d. \n",
                 ipv6h->version, OAL_NTOH_16(ipv6h->payload_len), ipv6h->nexthdr);
    OAL_IO_PRINT(WIFI_WAKESRC_TAG "ipv6 src addr:%04x:x:x:x:x:x:x:%04x \n", IPADDR6(ipv6h->saddr));
    OAL_IO_PRINT(WIFI_WAKESRC_TAG "ipv6 dst addr:%04x:x:x:x:x:x:x:%04x \n", IPADDR6(ipv6h->daddr));
    if (OAL_IPPROTO_ICMPV6 == ipv6h->nexthdr) {
        oal_nd_msg_stru *pst_rx_nd_hdr;
        pst_rx_nd_hdr = (oal_nd_msg_stru *)(ipv6h + 1);
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "ipv6 nd type: %d. \n", pst_rx_nd_hdr->icmph.icmp6_type);
    }
#ifdef CONFIG_HUAWEI_DUBAI
    dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", IPPROTO_IPV6);
#endif

    return;
}


OAL_STATIC oal_void hmac_parse_arp_packet(oal_void *pst_eth)
{
    const struct iphdr *iph = OAL_PTR_NULL;
    int iphdr_len;
    struct arphdr *arp;

    iph = (struct iphdr *)((mac_ether_header_stru *)pst_eth + 1);
    iphdr_len = iph->ihl * 4;
    arp = (struct arphdr *)((oal_uint8 *)iph + iphdr_len);
    OAL_IO_PRINT(WIFI_WAKESRC_TAG "ARP packet, hardware type:%d, protocol type:%d, opcode:%d.\n",
                 OAL_NTOH_16(arp->ar_hrd), OAL_NTOH_16(arp->ar_pro), OAL_NTOH_16(arp->ar_op));

    return;
}


OAL_STATIC oal_void hmac_parse_8021x_packet(oal_void *pst_eth)
{
    struct ieee8021x_hdr *hdr = (struct ieee8021x_hdr *)((mac_ether_header_stru *)pst_eth + 1);

    OAL_IO_PRINT(WIFI_WAKESRC_TAG "802.1x frame: version:%d, type:%d, length:%d\n",
                 hdr->version, hdr->type, OAL_NTOH_16(hdr->length));

    return;
}


oal_void hmac_parse_packet_etc(oal_netbuf_stru *pst_netbuf_eth)
{
    oal_uint16 us_type;
    mac_ether_header_stru *pst_ether_hdr;

    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf_eth);
    if (OAL_UNLIKELY(pst_ether_hdr == OAL_PTR_NULL)) {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "ether header is null.\n");
        return;
    }

    us_type = pst_ether_hdr->us_ether_type;
    OAL_IO_PRINT(WIFI_WAKESRC_TAG "protocol type:0x%04x\n", OAL_NTOH_16(us_type));

    if (us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IP)) {
        hmac_parse_ipv4_packet((oal_void *)pst_ether_hdr);
    } else if (us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6)) {
        hmac_parse_ipv6_packet((oal_void *)pst_ether_hdr);
    } else if (us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_ARP)) {
        hmac_parse_arp_packet((oal_void *)pst_ether_hdr);
    } else if (us_type == OAL_HOST2NET_SHORT(ETHER_TYPE_PAE)) {
        hmac_parse_8021x_packet((oal_void *)pst_ether_hdr);
    } else {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "receive other packet.\n");
    }

    return;
}

#endif

#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING

oal_bool_enum_uint8 hmac_rx_check_arp_spoofing(oal_net_device_stru *pst_device, oal_netbuf_stru *pst_netbuf)
{
    mac_ether_header_stru *pst_ether_header = OAL_PTR_NULL;
    oal_eth_arphdr_stru *pst_arp_hdr = OAL_PTR_NULL;
    oal_uint16 us_ether_type;
    oal_uint8 *puc_dev_addr = OAL_PTR_NULL;
    struct net_bridge_port *pst_br_port = OAL_PTR_NULL;
    oal_uint32 ul_addr_ret = 0;

    oal_netbuf_push(pst_netbuf, ETHER_HDR_LEN);
    pst_ether_header = (mac_ether_header_stru *)OAL_NETBUF_HEADER(pst_netbuf);
    /* ��ԭskb��dataָ�� */
    oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
    us_ether_type = pst_ether_header->us_ether_type;
    if (OAL_HOST2NET_SHORT(ETHER_TYPE_ARP) == us_ether_type) {
        /* ARP ����ַת�� */
        pst_arp_hdr = (oal_eth_arphdr_stru *)(pst_ether_header + 1);
        /*lint -e778*/
        if (OAL_HOST2NET_SHORT(ETHER_TYPE_IP) == pst_arp_hdr->us_ar_pro) {
            /*lint +e778*/
            /* LAN��ARP������ */
            pst_br_port = br_port_get_rcu(pst_device);
            if (pst_br_port == OAL_PTR_NULL) {
                return OAL_FALSE;
            }
            /* ��ȡ��AP��Ӳ��IP��ַ ��ַ�����ΪARP��ƭ���� */
            ul_addr_ret = (oal_uint32)inet_select_addr(pst_br_port->br->dev, 0, RT_SCOPE_LINK);
            puc_dev_addr = (oal_uint8 *)(&ul_addr_ret);
#ifdef _PRE_DEBUG_MODE
            OAM_WARNING_LOG4(0, OAM_SF_RX, "hmac_rx_check_arp_spoofing: find br addr: %d:%d:%d:%d",
                             puc_dev_addr[0], puc_dev_addr[1], puc_dev_addr[2], puc_dev_addr[3]);
#endif
            if (!OAL_MEMCMP(puc_dev_addr, pst_arp_hdr->auc_ar_sip, pst_arp_hdr->uc_ar_hln)) {
                return OAL_TRUE;
            }
        }
    }
    return OAL_FALSE;
}

#endif  // _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING

#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
extern int fastip_forward_wifi(unsigned int idx, struct sk_buff *skb);
extern unsigned int fastip_run_flag;
#endif


OAL_STATIC oal_void hmac_rx_transmit_msdu_to_lan(hmac_vap_stru *pst_vap,
                                                 hmac_user_stru *pst_hmac_user,
                                                 dmac_msdu_stru *pst_msdu)
{
    oal_net_device_stru *pst_device = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf;
    mac_ether_header_stru *pst_ether_hdr = OAL_PTR_NULL;
    oal_uint8 *puc_mac_addr;
    mac_vap_stru *pst_mac_vap = &(pst_vap->st_vap_base_info);
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    hmac_user_stru *pst_hmac_user_st = OAL_PTR_NULL;
    mac_ip_header_stru *pst_ip = OAL_PTR_NULL;
    oal_uint16 us_assoc_id = 0xffff;
#endif
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    hmac_wds_stas_stru *pst_sta;
    oal_netbuf_stru *pst_temp_netbuf = OAL_PTR_NULL;
#endif

#if (defined(CONFIG_BALONG_SPE) && defined(_PRE_WLAN_SPE_SUPPORT))
    dma_addr_t ul_dma_addr;
    oal_int32 l_ret = 0;
#endif
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8 en_trace_pkt_type;
#endif

#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT ||  \
     _PRE_TARGET_PRODUCT_TYPE_CPE == _PRE_CONFIG_TARGET_PRODUCT)
    oal_uint8 uc_data_type;
#endif

    /* ��ȡnetbuf����netbuf��dataָ���Ѿ�ָ��payload�� */
    pst_netbuf = pst_msdu->pst_netbuf;
    puc_mac_addr = pst_msdu->auc_ta;

#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT ||  \
     _PRE_TARGET_PRODUCT_TYPE_CPE == _PRE_CONFIG_TARGET_PRODUCT)
    uc_data_type = mac_get_data_type_etc(pst_netbuf);
#endif

    OAL_NETBUF_PREV(pst_netbuf) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(pst_netbuf) = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    /* STA���е���ת�鲥���� */
    if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (OAL_LIKELY(!ETHER_IS_MULTICAST(pst_msdu->auc_da))) {
            hmac_m2u_unicast_convert_multicast(pst_vap, pst_netbuf, pst_msdu);
        }
    }
#endif

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    if (OAL_SUCC == hmac_wds_find_sta(pst_vap, pst_msdu->auc_sa, &pst_sta)) {
        pst_temp_netbuf = oal_netbuf_copy(pst_netbuf, GFP_ATOMIC);
        hmac_rx_frame_80211_to_eth(pst_temp_netbuf, pst_msdu->auc_da, pst_msdu->auc_ta);
        hmac_rx_frame_80211_to_eth(pst_netbuf, pst_msdu->auc_da, pst_msdu->auc_sa);
    } else {
        hmac_rx_frame_80211_to_eth(pst_netbuf, pst_msdu->auc_da, pst_msdu->auc_sa);
    }
#else
    {
        hmac_rx_frame_80211_to_eth(pst_netbuf, pst_msdu->auc_da, pst_msdu->auc_sa);
    }
#endif

    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
    if (OAL_UNLIKELY(pst_ether_hdr == OAL_PTR_NULL)) {
        oal_netbuf_free(pst_netbuf);
        OAM_ERROR_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_transmit_msdu_to_lan::pst_ether_hdr null.}");
        return;
    }
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (OAL_TRUE == wlan_pm_wkup_src_debug_get()) {
        OAL_IO_PRINT(WIFI_WAKESRC_TAG "rx: hmac_parse_packet_etc!\n");
        hmac_parse_packet_etc(pst_netbuf);
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
    hmac_parse_special_ipv4_packet(oal_netbuf_data(pst_netbuf), oal_netbuf_get_len(pst_netbuf), HMAC_PKT_DIRECTION_RX);
#endif
#ifdef _PRE_DEBUG_MODE
    // ���ӹؼ�֡��ӡ�������ж��Ƿ�����Ҫ��ӡ��֡���ͣ�Ȼ���ӡ��֡����
    en_trace_pkt_type = mac_pkt_should_trace((oal_uint8 *)pst_ether_hdr, MAC_NETBUFF_PAYLOAD_ETH);
    if (en_trace_pkt_type != PKT_TRACE_BUTT) {
        oam_report_eth_frame_etc(puc_mac_addr,
                                 oal_netbuf_data(pst_netbuf),
                                 (oal_uint16)OAL_NETBUF_LEN(pst_netbuf),
                                 OAM_OTA_FRAME_DIRECTION_TYPE_RX);
        OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_rx_transmit_msdu_to_lan::type%d to eth[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n",
            en_trace_pkt_type);
    }
#endif

    if (OAL_SUCC != hmac_11i_ether_type_filter_etc(pst_vap, pst_hmac_user, pst_ether_hdr->us_ether_type)) {
        /* ���հ�ȫ���ݹ��� */
        oam_report_eth_frame_etc(puc_mac_addr, (oal_uint8 *)pst_ether_hdr,
                                 (oal_uint16)OAL_NETBUF_LEN(pst_netbuf), OAM_OTA_FRAME_DIRECTION_TYPE_RX);

        oal_netbuf_free(pst_netbuf);
        OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_portvalid_check_fail_dropped, 1);
        return;
    }

#if defined(_PRE_WLAN_FEATURE_MCAST) || defined(_PRE_WLAN_FEATURE_HERA_MCAST)
    if (pst_vap->pst_m2u != OAL_PTR_NULL) {
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
        if (OAL_SUCC == hmac_wds_find_sta(pst_vap, pst_msdu->auc_sa, &pst_sta)) {
            hmac_m2u_snoop_inspecting(pst_vap, pst_hmac_user, pst_temp_netbuf);
        } else {
            hmac_m2u_snoop_inspecting(pst_vap, pst_hmac_user, pst_netbuf);
        }

        if (pst_temp_netbuf != OAL_PTR_NULL) {
            oal_netbuf_free(pst_temp_netbuf);
        }
#else
        {
            hmac_m2u_snoop_inspecting(pst_vap, pst_hmac_user, pst_netbuf);
        }
#endif
    }
#endif

    /* ��ȡnet device hmac������ʱ����Ҫ��¼netdeviceָ�� */
    pst_device = pst_vap->pst_net_device;

    /* ��protocolģʽ��ֵ */
    OAL_NETBUF_PROTOCOL(pst_netbuf) = oal_eth_type_trans(pst_netbuf, pst_device);

    /* APģʽ��֧�ַ�arp��ƭ���� */
#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING
    if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if (hmac_rx_check_arp_spoofing(pst_device, pst_netbuf)) {
            OAM_WARNING_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                             "{hmac_rx_transmit_msdu_to_lan::detect ARP spoofing data .}");
            oal_netbuf_free(pst_netbuf);
            return;
        }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if (OAL_FALSE == ETHER_IS_MULTICAST(pst_msdu->auc_da)) {
        oal_atomic_inc(&(pst_hmac_user->st_hmac_user_btcoex.st_hmac_btcoex_arp_req_process.ul_rx_unicast_pkt_to_lan));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    if (OAL_FALSE == ETHER_IS_MULTICAST(pst_msdu->auc_da)) {
        oal_atomic_inc(&(pst_vap->st_hmac_vap_m2s.ul_rx_unicast_pkt_to_lan));
    }
#endif

    /* ��Ϣͳ����֡�ϱ����� */
    /* ����ͳ����Ϣ */
    HMAC_VAP_DFT_STATS_PKT_INCR(pst_vap->st_query_stats.ul_rx_pkt_to_lan, 1);
    HMAC_VAP_DFT_STATS_PKT_INCR(pst_vap->st_query_stats.ul_rx_bytes_to_lan, OAL_NETBUF_LEN(pst_netbuf));
    OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_pkt_to_lan, 1); /* ���ӷ���LAN��֡����Ŀ */
    /* ���ӷ���LAN���ֽ��� */
    OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_bytes_to_lan, OAL_NETBUF_LEN(pst_netbuf));

#ifdef _PRE_WLAN_DFT_DUMP_FRAME
    hmac_rx_report_eth_frame_etc(&pst_vap->st_vap_base_info, pst_netbuf);
#endif

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) && (pst_vap->uc_edca_opt_flag_ap == OAL_TRUE)) {
        /*lint -e778*/
        if (OAL_HOST2NET_SHORT(ETHER_TYPE_IP) == pst_ether_hdr->us_ether_type) {
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
            if ((OAL_SUCC != mac_vap_find_user_by_macaddr_etc(pst_mac_vap,
                                                              pst_ether_hdr->auc_ether_shost, &us_assoc_id)) &&
                (OAL_SUCC != hmac_find_valid_user_by_wds_sta(pst_vap, pst_ether_hdr->auc_ether_shost, &us_assoc_id)))
#else
            if (OAL_SUCC != mac_vap_find_user_by_macaddr_etc(pst_mac_vap, pst_ether_hdr->auc_ether_shost, &us_assoc_id))
#endif
            {
                OAM_WARNING_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_M2U,
                    "{hmac_rx_transmit_msdu_to_lan::find_user_by_macaddr[%02x:XX:XX:%02x:%02x:%02x]failed}",
                    (oal_uint32)(pst_ether_hdr->auc_ether_shost[0]),
                    (oal_uint32)(pst_ether_hdr->auc_ether_shost[3]),
                    (oal_uint32)(pst_ether_hdr->auc_ether_shost[4]),
                    (oal_uint32)(pst_ether_hdr->auc_ether_shost[5]));
                oal_netbuf_free(pst_netbuf);
                return;
            }
            pst_hmac_user_st = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_assoc_id);
            if (pst_hmac_user_st == OAL_PTR_NULL) {
                OAM_ERROR_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                    "{hmac_rx_transmit_msdu_to_lan::mac_res_get_hmac_user_etc fail. assoc_id: %u}", us_assoc_id);
                oal_netbuf_free(pst_netbuf);
                return;
            }

            pst_ip = (mac_ip_header_stru *)(pst_ether_hdr + 1);

            /* mips�Ż�:�������ҵ��ͳ�����ܲ�10M���� */
            if (((pst_ip->uc_protocol == MAC_UDP_PROTOCAL) &&
                 (pst_hmac_user_st->aaul_txrx_data_stat[WLAN_WME_AC_BE][WLAN_RX_UDP_DATA] <
                  (HMAC_EDCA_OPT_PKT_NUM + 10))) ||
                ((pst_ip->uc_protocol == MAC_TCP_PROTOCAL) &&
                 (pst_hmac_user_st->aaul_txrx_data_stat[WLAN_WME_AC_BE][WLAN_RX_TCP_DATA] <
                  (HMAC_EDCA_OPT_PKT_NUM + 10)))) {
                hmac_edca_opt_rx_pkts_stat_etc(us_assoc_id, WLAN_TIDNO_BEST_EFFORT, pst_ip);
            }
        }
        /*lint +e778*/
    }
#endif

    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
#ifdef _PRE_WLAN_PKT_TIME_STAT
    memset_s(OAL_NETBUF_CB(pst_netbuf), OAL_NETBUF_CB_ORIGIN, 0, OAL_NETBUF_CB_ORIGIN);
#else
    memset_s(OAL_NETBUF_CB(pst_netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
#endif

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
    hmac_pkt_mem_opt_rx_pkts_stat (pst_vap, (oal_ip_header_stru *)(pst_ether_hdr + 1));
#endif
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_NETBUF_FOR_KERNEL);

    /* ��skbת������ */
    if (OAL_TRUE == hmac_get_rxthread_enable_etc()) {
        hmac_rxdata_netbuf_enqueue_etc(pst_netbuf);
    } else {
        oal_notice_netif_rx(pst_netbuf);
        oal_netif_rx_ni(pst_netbuf);
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
    /* 4.11�����ں˰汾net_device�ṹ����û��last_rx�ֶ� */
#else
    /* ��λnet_dev->jiffies���� */
    OAL_NETDEVICE_LAST_RX(pst_device) = OAL_TIME_JIFFY;
#endif
}


oal_void hmac_rx_lan_frame_classify_etc(hmac_vap_stru *pst_vap,
                                        oal_netbuf_stru *pst_netbuf,
                                        mac_ieee80211_frame_stru *pst_frame_hdr)
{
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;                      /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    dmac_msdu_stru st_msdu;                                           /* �������������ÿһ��MSDU */
    mac_msdu_proc_status_enum_uint8 en_process_state = MAC_PROC_BUTT; /* ����AMSDU��״̬ */
    dmac_msdu_proc_state_stru st_msdu_state = { 0 };                  /* ��¼MPDU�Ĵ�����Ϣ */
    oal_uint8 *puc_addr = OAL_PTR_NULL;
    oal_uint32 ul_ret;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    oal_uint8 uc_datatype = MAC_DATA_BUTT;
#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_stru *pst_wapi;
    oal_bool_enum_uint8 en_is_mcast = OAL_FALSE;
#endif
#if defined _PRE_WLAN_FEATURE_WDS || defined _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    oal_uint8 uc_is_first = OAL_TRUE;
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_netbuf_head_stru st_w2w_netbuf_hdr;
    frw_event_hdr_stru st_event_hdr;
    oal_uint16 us_user_dix = MAC_INVALID_USER_ID;
#endif

    if (OAL_UNLIKELY(OAL_ANY_NULL_PTR3(pst_vap, pst_netbuf, pst_frame_hdr))) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_lan_frame_classify_etc::params null.}");
        return;
    }

    memset_s(&st_msdu, OAL_SIZEOF(dmac_msdu_stru), 0, OAL_SIZEOF(dmac_msdu_stru));
    mac_get_transmit_addr(pst_frame_hdr, &puc_addr);
    oal_set_mac_addr(st_msdu.auc_ta, puc_addr);

    /* ��ȡ��MPDU�Ŀ�����Ϣ */
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl));
    if (OAL_UNLIKELY(pst_hmac_user == OAL_PTR_NULL)) {
        /* ��ӡ��net buf�����Ϣ */
        OAM_ERROR_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_rx_lan_frame_classify_etc::cb info,vap id=%d mac_hdr_len=%d,frame_len=%d mac_hdr_start_addr=0x%08x}",
            pst_rx_ctrl->bit_vap_id,
            pst_rx_ctrl->uc_mac_header_len,
            pst_rx_ctrl->us_frame_len,
            (uintptr_t)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl));
        OAM_ERROR_LOG3(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                       "{hmac_rx_lan_frame_classify_etc::user_idx=%d,net_buf ptr addr=0x%08x, cb ptr addr=0x%08x.}",
                       MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl),
                       (uintptr_t)pst_netbuf, (uintptr_t)pst_rx_ctrl);
#ifdef _PRE_WLAN_DFT_DUMP_FRAME
        mac_rx_report_80211_frame_etc((oal_uint8 *)&(pst_vap->st_vap_base_info),
                                      (oal_uint8 *)pst_rx_ctrl,
                                      pst_netbuf,
                                      OAM_OTA_TYPE_RX_HMAC_CB);
#endif

        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_netbuf_list_head_init(&st_w2w_netbuf_hdr);
    st_event_hdr.uc_chip_id = pst_vap->st_vap_base_info.uc_chip_id;
    st_event_hdr.uc_device_id = pst_vap->st_vap_base_info.uc_device_id;
    st_event_hdr.uc_vap_id = pst_vap->st_vap_base_info.uc_vap_id;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* offload ��amsdu֡���ֳܷɶ��seq��ͬ��netbuf�ϱ� ֻ����last amsdu buffer������λ
        �������׵��������򻺳��������ź���ǿ���ƴ����յ�ba start֮ǰseq��amsdu֡��֡
     */
    if ((pst_rx_ctrl->bit_amsdu_enable == OAL_FALSE) || (pst_rx_ctrl->bit_is_last_buffer == OAL_TRUE)) {
        hmac_ba_update_rx_bitmap(pst_hmac_user, pst_frame_hdr);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    proc_sniffer_write_file(NULL, 0, (oal_uint8 *)oal_netbuf_payload(pst_netbuf), pst_rx_ctrl->us_frame_len, 0);
#endif

    /* ���һ:����AMSDU�ۺϣ����MPDU��Ӧһ��MSDU��ͬʱ��Ӧһ��NETBUF */
    if (pst_rx_ctrl->bit_amsdu_enable == OAL_FALSE) {
#ifdef _PRE_WLAN_FEATURE_WAPI
        en_is_mcast = ETHER_IS_MULTICAST(pst_frame_hdr->auc_address1);
        /*lint -e730*/
        pst_wapi = hmac_user_get_wapi_ptr_etc(&pst_vap->st_vap_base_info, !en_is_mcast,
                                              pst_hmac_user->st_user_base_info.us_assoc_id);
        /*lint +e730*/
        if (pst_wapi == OAL_PTR_NULL) {
            OAM_WARNING_LOG0(0, OAM_SF_WPA, "{hmac_rx_lan_frame_classify_etc:: get pst_wapi Err!.}");
            HMAC_USER_STATS_PKT_INCR(pst_hmac_user->ul_rx_pkt_drop, 1);
            return;
        }

        if ((OAL_TRUE == WAPI_PORT_FLAG(pst_wapi)) && (pst_wapi->wapi_netbuff_rxhandle != OAL_PTR_NULL)) {
            pst_netbuf = pst_wapi->wapi_netbuff_rxhandle(pst_wapi, pst_netbuf);
            if (pst_netbuf == OAL_PTR_NULL) {
                OAM_WARNING_LOG0(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "{hmac_rx_lan_frame_classify_etc:: wapi decrypt FAIL!}");
                HMAC_USER_STATS_PKT_INCR(pst_hmac_user->ul_rx_pkt_drop, 1);
                return;
            }

            /* ���»�ȡ��MPDU�Ŀ�����Ϣ */
            pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

        pst_netbuf = hmac_defrag_process_etc(pst_hmac_user, pst_netbuf, pst_rx_ctrl->uc_mac_header_len);
        if (pst_netbuf == OAL_PTR_NULL) {
            return;
        }

        /* ���»�ȡ��MPDU�Ŀ�����Ϣ */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl);

        /* ��ӡ���ؼ�֡(dhcp)��Ϣ */
        uc_datatype = mac_get_data_type_from_80211_etc(pst_netbuf, pst_rx_ctrl->uc_mac_header_len);
        if (uc_datatype <= MAC_DATA_ARP_RSP) {
            OAM_WARNING_LOG4(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                "{hmac_rx_lan_frame_classify_etc::::user[%d], datatype==%u, len==%u, rx_drop_cnt==%u}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
                MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl), uc_datatype,
                pst_rx_ctrl->us_frame_len, pst_hmac_user->ul_rx_pkt_drop);
        }

        /* �Ե�ǰ��msdu���и�ֵ */
        st_msdu.pst_netbuf = pst_netbuf;

        /* ��ȡԴ��ַ��Ŀ�ĵ�ַ */
        mac_rx_get_sa(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(st_msdu.auc_sa, puc_addr);

        mac_rx_get_da(pst_frame_hdr, &puc_addr);
        oal_set_mac_addr(st_msdu.auc_da, puc_addr);

        /* ��netbuf��dataָ��ָ��mac frame��payload�� */
        oal_netbuf_pull(pst_netbuf, pst_rx_ctrl->uc_mac_header_len);

        if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            ul_ret = mac_vap_find_user_by_macaddr_etc(&pst_vap->st_vap_base_info, st_msdu.auc_da, &us_user_dix);
            if (ul_ret == OAL_SUCC) {
                /* ��MSDUת��Ϊ��̫����ʽ��֡ */
                hmac_rx_frame_80211_to_eth(st_msdu.pst_netbuf, st_msdu.auc_da, st_msdu.auc_sa);
                /* ��MSDU���뵽netbuf������� */
                oal_netbuf_add_to_list_tail(st_msdu.pst_netbuf, &st_w2w_netbuf_hdr);
            } else
#endif
            {
                OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_PREPARE_MSDU_INFO);
                /* ��MSDUת����LAN */
                hmac_rx_transmit_msdu_to_lan(pst_vap, pst_hmac_user, &st_msdu);
            }
        } else {
            OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_PREPARE_MSDU_INFO);
            /* ��MSDUת����LAN */
            hmac_rx_transmit_msdu_to_lan(pst_vap, pst_hmac_user, &st_msdu);
        }
    }
    /* �����:AMSDU�ۺ� */
    else {
        st_msdu_state.uc_procd_netbuf_nums = 0;
        st_msdu_state.uc_procd_msdu_in_netbuf = 0;

        /* amsdu ���һ��netbuf nextָ����Ϊ NULL ����ʱ�����ͷ�amsdu netbuf */
        hmac_rx_clear_amsdu_last_netbuf_pointer(pst_netbuf, pst_rx_ctrl->bit_buff_nums);

        do {
            /* ��ȡ��һ��Ҫת����msdu */
            ul_ret = hmac_rx_parse_amsdu_etc(pst_netbuf, &st_msdu, &st_msdu_state, &en_process_state);
            if (ul_ret != OAL_SUCC) {
                OAM_WARNING_LOG1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                 "{hmac_rx_lan_frame_classify_etc::hmac_rx_parse_amsdu_etc failed[%d].}", ul_ret);
                return;
            }

            OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_PREPARE_MSDU_INFO);

#ifdef _PRE_WLAN_FEATURE_WDS
            /* �����4��ַwds���ݣ�����wds�� */
            if ((uc_is_first == OAL_TRUE) &&
                (OAL_TRUE == mac_is_4addr((oal_uint8 *)pst_frame_hdr)) &&
                (pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_ROOTAP)) {
                hmac_wds_update_table(pst_vap, pst_frame_hdr->auc_address2, (&st_msdu)->auc_sa, WDS_TABLE_ADD_ENTRY);
                hmac_vap_set_wds_user(pst_vap, pst_frame_hdr->auc_address2);
                uc_is_first = OAL_FALSE;
            }
#endif
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
            /* Virtual Multi-STA���Լ�������λ��ʽ����ͨ��4��ַ��ʶ�� */
            if ((uc_is_first == OAL_TRUE) &&
                (pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_ROOTAP) &&
                hmac_vmsta_get_user_a4_support(pst_vap, pst_frame_hdr->auc_address2)) {
                hmac_wds_update_table(pst_vap, pst_frame_hdr->auc_address2, (&st_msdu)->auc_sa, WDS_TABLE_ADD_ENTRY);
                uc_is_first = OAL_FALSE;
            }
#endif

            if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                ul_ret = mac_vap_find_user_by_macaddr_etc(&pst_vap->st_vap_base_info, st_msdu.auc_da, &us_user_dix);
                if (ul_ret == OAL_SUCC) {
                    /* ��MSDUת��Ϊ��̫����ʽ��֡ */
                    hmac_rx_frame_80211_to_eth(st_msdu.pst_netbuf, st_msdu.auc_da, st_msdu.auc_sa);
                    /* ��MSDU���뵽netbuf������� */
                    oal_netbuf_add_to_list_tail(st_msdu.pst_netbuf, &st_w2w_netbuf_hdr);
                } else
#endif
                {
                    /* ��ÿһ��MSDUת����LAN */
                    hmac_rx_transmit_msdu_to_lan(pst_vap, pst_hmac_user, &st_msdu);
                }
            } else {
                /* ��ÿһ��MSDUת����LAN */
                hmac_rx_transmit_msdu_to_lan(pst_vap, pst_hmac_user, &st_msdu);
            }
        } while (en_process_state != MAC_PROC_LAST_MSDU);
    }

    if (pst_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_TO_LAN);
        /*  ��MSDU���������������̴��� */
        if (OAL_FALSE == oal_netbuf_list_empty(&st_w2w_netbuf_hdr) &&
            OAL_PTR_NULL != oal_netbuf_tail(&st_w2w_netbuf_hdr) &&
            OAL_PTR_NULL != oal_netbuf_peek(&st_w2w_netbuf_hdr)) {
            OAL_NETBUF_NEXT((oal_netbuf_tail(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;
            OAL_NETBUF_PREV((oal_netbuf_peek(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;

            hmac_rx_transmit_to_wlan(&st_event_hdr, &st_w2w_netbuf_hdr);
        }
        OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_END);
#endif
    }
}


oal_uint32 hmac_rx_copy_netbuff_etc(oal_netbuf_stru **ppst_dest_netbuf,
                                    oal_netbuf_stru *pst_src_netbuf, oal_uint8 uc_vap_id,
                                    mac_ieee80211_frame_stru **ppul_mac_hdr_start_addr)
{
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    oal_int32 l_ret;

    if (pst_src_netbuf == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *ppst_dest_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (OAL_UNLIKELY(*ppst_dest_netbuf == OAL_PTR_NULL)) {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_RX, "{hmac_rx_copy_netbuff_etc::pst_netbuf_copy null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* ��Ϣ���� */
    l_ret = memcpy_s(oal_netbuf_cb(*ppst_dest_netbuf), OAL_SIZEOF(mac_rx_ctl_stru),
                     oal_netbuf_cb(pst_src_netbuf), OAL_SIZEOF(mac_rx_ctl_stru));  // modify src bug
    l_ret += memcpy_s(oal_netbuf_data(*ppst_dest_netbuf), OAL_NETBUF_LEN(pst_src_netbuf),
                      oal_netbuf_data(pst_src_netbuf), OAL_NETBUF_LEN(pst_src_netbuf));
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "hmac_rx_copy_netbuff_etc::memcpy fail!");
        return OAL_FAIL;
    }

    /* ����netbuf���ȡ�TAILָ�� */
    oal_netbuf_put(*ppst_dest_netbuf, oal_netbuf_get_len(pst_src_netbuf));

    /* ����MAC֡ͷ��ָ��copy�󣬶�Ӧ��mac header��ͷ�Ѿ������仯) */
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(*ppst_dest_netbuf);
    MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl) = (oal_uint32 *)oal_netbuf_data(*ppst_dest_netbuf);
    *ppul_mac_hdr_start_addr = (mac_ieee80211_frame_stru *)oal_netbuf_data(*ppst_dest_netbuf);

    return OAL_SUCC;
}


oal_void hmac_rx_process_data_filter_etc(oal_netbuf_head_stru *pst_netbuf_header,
                                         oal_netbuf_stru *pst_temp_netbuf,
                                         oal_uint16 us_netbuf_num)
{
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_AMPDU
    hmac_user_stru *pst_hmac_user;
    oal_uint32 ul_ret = OAL_SUCC;
#endif
    oal_uint8 uc_buf_nums;
    oal_uint8 uc_vap_id;
    oal_bool_enum_uint8 en_is_ba_buf;
    oal_uint8 uc_netbuf_num;

    while (us_netbuf_num != 0) {
        en_is_ba_buf = OAL_FALSE;
        pst_netbuf = pst_temp_netbuf;
        if (pst_netbuf == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_filter_etc::us_netbuf_num = %d}", us_netbuf_num);
            break;
        }

        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        uc_buf_nums = pst_rx_ctrl->bit_buff_nums;

        /* ��ȡ��һ��Ҫ������MPDU */
        oal_netbuf_get_appointed_netbuf(pst_netbuf, uc_buf_nums, &pst_temp_netbuf);
        us_netbuf_num = OAL_SUB(us_netbuf_num, uc_buf_nums);

        uc_vap_id = pst_rx_ctrl->uc_mac_vap_id;
        // if (0 == pst_vap->uc_vap_id || WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT < pst_vap->uc_vap_id)
        /* ˫оƬ�£�0��1��������vap id�����������Ҫ����ҵ��vap ��ʵid���������vap mac numֵ�����ж� */
        if (oal_board_get_service_vap_start_id() > uc_vap_id || uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
            OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_filter_etc::Invalid vap_id.vap_id[%u]}", uc_vap_id);
            hmac_rx_free_netbuf_list_etc(pst_netbuf_header, uc_buf_nums);
            continue;
        }
#ifdef _PRE_WLAN_FEATURE_AMPDU
        // make sure ta user idx is exist
        pst_hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl));
        if (pst_hmac_user == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(0, OAM_SF_ROAM,
                             "{hmac_rx_process_data_filter_etc::pst_hmac_user[%d] null.}",
                             MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl));
            hmac_rx_free_netbuf_list_etc(pst_netbuf_header, uc_buf_nums);
            continue;
        }

        ul_ret = hmac_ba_filter_serv_etc(pst_hmac_user, pst_rx_ctrl, pst_netbuf_header, &en_is_ba_buf);
        if (ul_ret != OAL_SUCC) {
            hmac_rx_free_netbuf_list_etc(pst_netbuf_header, uc_buf_nums);
            continue;
        }
        OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_REORDER_FILTER);
#endif

        if (en_is_ba_buf == OAL_TRUE) {
            continue;
        }

        /* �����buff��reorder���У������¹ҵ�����β������ */
        for (uc_netbuf_num = 0; uc_netbuf_num < uc_buf_nums; uc_netbuf_num++) {
            pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header);

            if (OAL_LIKELY(pst_netbuf != OAL_PTR_NULL)) {
                oal_netbuf_list_tail_nolock(pst_netbuf_header, pst_netbuf);
            } else {
                OAM_WARNING_LOG0(pst_rx_ctrl->bit_vap_id, OAM_SF_RX,
                                 "{hmac_rx_process_data_filter_etc::no buff error.}");
            }
        }
        OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_NON_REORDER_BACK);
    }
}

#ifdef _PRE_WLAN_TCP_OPT
OAL_STATIC oal_bool_enum_uint8 hmac_transfer_rx_tcp_ack_handler(hmac_device_stru *pst_hmac_device,
                                                                hmac_vap_stru *hmac_vap,
                                                                oal_netbuf_stru *netbuf)
{
#ifndef WIN32
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    mac_rx_ctl_stru *pst_rx_ctrl; /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    oal_netbuf_stru *pst_mac_llc_snap_netbuf;

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    pst_mac_llc_snap_netbuf = (oal_netbuf_stru *)(netbuf->data + pst_rx_ctrl->uc_mac_header_len);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    OAM_WARNING_LOG1(0, OAM_SF_TX,
                     "{hmac_transfer_rx_handler::uc_mac_header_len = %d}\r\n", pst_rx_ctrl->uc_mac_header_len);
#endif
    if (OAL_TRUE == hmac_judge_rx_netbuf_classify_etc(pst_mac_llc_snap_netbuf)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        OAM_WARNING_LOG0(0, OAM_SF_TX,
                         "{hmac_transfer_rx_handler::netbuf is tcp ack.}\r\n");
#endif
        oal_spin_lock_bh(&hmac_vap->st_hamc_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        oal_netbuf_list_tail(&hmac_vap->st_hamc_tcp_ack[HCC_RX].data_queue[HMAC_TCP_ACK_QUEUE],
                             netbuf);

        oal_spin_unlock_bh(&hmac_vap->st_hamc_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        hmac_sched_transfer_etc();
        return OAL_TRUE;
    }
#endif
#endif
    return OAL_FALSE;
}


oal_void hmac_transfer_rx_handle(hmac_device_stru *pst_hmac_device,
                                 hmac_vap_stru *pst_hmac_vap,
                                 oal_netbuf_head_stru *pst_netbuf_header)
{
    oal_netbuf_head_stru st_temp_header;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;

    oal_netbuf_head_init(&st_temp_header);
    while (!!(pst_netbuf = oal_netbuf_delist_nolock(pst_netbuf_header))) {
        if (OAL_FALSE == hmac_transfer_rx_tcp_ack_handler(pst_hmac_device, pst_hmac_vap, pst_netbuf)) {
            oal_netbuf_list_tail_nolock(&st_temp_header, pst_netbuf);
        }
    }
    /*lint -e522*/
    OAL_WARN_ON(!oal_netbuf_list_empty(pst_netbuf_header));
    /*lint +e522*/
    oal_netbuf_splice_init(&st_temp_header, pst_netbuf_header);
}
#endif


oal_uint32 hmac_rx_lan_frame_etc(oal_netbuf_head_stru *pst_netbuf_header)
{
    oal_uint32 ul_netbuf_num;
    oal_netbuf_stru *pst_temp_netbuf = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;
    oal_uint8 uc_buf_nums;
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = OAL_PTR_NULL;
    hmac_vap_stru *pst_vap = OAL_PTR_NULL;
#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
    oal_uint8 *puc_mac_addr;
#endif

    ul_netbuf_num = oal_netbuf_get_buf_num(pst_netbuf_header);
    pst_temp_netbuf = oal_netbuf_peek(pst_netbuf_header);

    while (ul_netbuf_num != 0) {
        pst_netbuf = pst_temp_netbuf;
        if (pst_netbuf == NULL) {
            break;
        }

        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl);
        uc_buf_nums = pst_rx_ctrl->bit_buff_nums;

        oal_netbuf_get_appointed_netbuf(pst_netbuf, uc_buf_nums, &pst_temp_netbuf);
        ul_netbuf_num = OAL_SUB(ul_netbuf_num, uc_buf_nums);

        pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_rx_ctrl->uc_mac_vap_id);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_rx_lan_frame_etc::mac_res_get_hmac_vap null. vap_id:%u}",
                           pst_rx_ctrl->uc_mac_vap_id);
            continue;
        }

        MAC_GET_RX_CB_DA_USER_IDX(pst_rx_ctrl) = pst_vap->st_vap_base_info.us_assoc_vap_id;

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
        mac_rx_get_sa(pst_frame_hdr, &puc_mac_addr);
        /* Also we should drop multicast pkt whose sa belond to lan side(like LanCpe/br0) */
        if ((pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_REPEATER_STA) &&
            (OAL_SUCC == hmac_wds_neigh_not_expired(pst_vap, puc_mac_addr))) {
            OAM_WARNING_LOG1(pst_rx_ctrl->uc_mac_vap_id, OAM_SF_RX,
                             "{hmac_rx_lan_frame_etc::free %d netbuf up to netdevice.}",
                             uc_buf_nums);
            hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
            continue;
        }
#endif

        hmac_rx_lan_frame_classify_etc(pst_vap, pst_netbuf, pst_frame_hdr);
    }

    if (OAL_TRUE == hmac_get_rxthread_enable_etc()) {
        hmac_rxdata_sched_etc();
    }

    return OAL_SUCC;
}


oal_void hmac_rx_process_data_ap_tcp_ack_opt_etc(hmac_vap_stru *pst_vap, oal_netbuf_head_stru *pst_netbuf_header)
{
    frw_event_hdr_stru st_event_hdr;
    mac_ieee80211_frame_stru *pst_frame_hdr = OAL_PTR_NULL;      /* ����mac֡��ָ�� */
    mac_ieee80211_frame_stru *pst_copy_frame_hdr = OAL_PTR_NULL; /* ����mac֡��ָ�� */
    oal_uint8 *puc_da = OAL_PTR_NULL;                            /* �����û�Ŀ�ĵ�ַ��ָ�� */
    hmac_user_stru *pst_hmac_da_user = OAL_PTR_NULL;
    oal_uint32 ul_rslt;
    oal_uint16 us_user_dix;
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;     /* ÿһ��MPDU�Ŀ�����Ϣ */
    oal_uint16 us_netbuf_num;                        /* netbuf�����ĸ��� */
    oal_uint8 uc_buf_nums;                           /* ÿ��mpduռ��buf�ĸ��� */
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL;      /* ���ڱ��浱ǰ������MPDU�ĵ�һ��netbufָ�� */
    oal_netbuf_stru *pst_temp_netbuf = OAL_PTR_NULL; /* ������ʱ������һ����Ҫ������netbufָ�� */
    oal_netbuf_stru *pst_netbuf_copy = OAL_PTR_NULL; /* ���ڱ����鲥֡copy */
    oal_netbuf_head_stru st_w2w_netbuf_hdr;          /* ����wlan to wlan��netbuf������ͷ */
#ifdef _PRE_WLAN_FEATURE_ISOLATION
    cs_isolation_forward_enum en_forward;
#endif

    /* ѭ���յ���ÿһ��MPDU�����������:
        1���鲥֡ʱ������WLAN TO WLAN��WLAN TO LAN�ӿ�
        2������������ʵ�����������WLAN TO LAN�ӿڻ���WLAN TO WLAN�ӿ� */
    oal_netbuf_list_head_init(&st_w2w_netbuf_hdr);
    pst_temp_netbuf = oal_netbuf_peek(pst_netbuf_header);
    us_netbuf_num = (oal_uint16)oal_netbuf_get_buf_num(pst_netbuf_header);
    st_event_hdr.uc_chip_id = pst_vap->st_vap_base_info.uc_chip_id;
    st_event_hdr.uc_device_id = pst_vap->st_vap_base_info.uc_device_id;
    st_event_hdr.uc_vap_id = pst_vap->st_vap_base_info.uc_vap_id;

    while (us_netbuf_num != 0) {
        pst_netbuf = pst_temp_netbuf;
        if (pst_netbuf == OAL_PTR_NULL) {
            break;
        }

        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

        /* ��ȡ֡ͷ��Ϣ */
        pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl);

        /* ��ȡ��ǰMPDUռ�õ�netbuf��Ŀ */
        uc_buf_nums = pst_rx_ctrl->bit_buff_nums;

        /* ��ȡ��һ��Ҫ������MPDU */
        oal_netbuf_get_appointed_netbuf(pst_netbuf, uc_buf_nums, &pst_temp_netbuf);
        us_netbuf_num = OAL_SUB(us_netbuf_num, uc_buf_nums);

        pst_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_rx_ctrl->uc_mac_vap_id);
        if (OAL_UNLIKELY(pst_vap == OAL_PTR_NULL)) {
            OAM_WARNING_LOG0(pst_rx_ctrl->bit_vap_id, OAM_SF_RX, "{hmac_rx_process_data_ap::pst_vap null.}");
            hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
            continue;
        }

        /* ��ȡ���ն˵�ַ  */
        mac_rx_get_da(pst_frame_hdr, &puc_da);

#ifdef _PRE_WLAN_FEATURE_WDS
        /* �����4��ַwds���ݣ�����wds�� */
        if ((pst_rx_ctrl->bit_amsdu_enable == OAL_FALSE) &&
            (OAL_TRUE == mac_is_4addr((oal_uint8 *)pst_frame_hdr)) &&
            (pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_ROOTAP)) {
            hmac_wds_update_table(pst_vap, pst_frame_hdr->auc_address2,
                                  ((mac_ieee80211_qos_htc_frame_addr4_stru *)pst_frame_hdr)->auc_address4,
                                  WDS_TABLE_ADD_ENTRY);
            hmac_vap_set_wds_user(pst_vap, pst_frame_hdr->auc_address2);
        }
#endif

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        /* Virtual Multi-STA���Լ�������λ��ʽ����ͨ��4��ַ��ʶ�� */
        if ((pst_rx_ctrl->bit_amsdu_enable == OAL_FALSE) &&
            (pst_vap->st_wds_table.en_wds_vap_mode == WDS_MODE_ROOTAP) &&
            hmac_vmsta_get_user_a4_support(pst_vap, pst_frame_hdr->auc_address2)) {
            hmac_wds_update_table(pst_vap, pst_frame_hdr->auc_address2,
                                  ((mac_ieee80211_qos_htc_frame_addr4_stru *)pst_frame_hdr)->auc_address4,
                                  WDS_TABLE_ADD_ENTRY);
        }
#endif

        /* Ŀ�ĵ�ַΪ�鲥��ַʱ������WLAN_TO_WLAN��WLAN_TO_LAN��ת�� */
        if (ETHER_IS_MULTICAST(puc_da)) {
            OAM_INFO_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX,
                          "{hmac_rx_lan_frame_classify_etc::the frame is a group frame.}");
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_mcast_cnt, 1);

            if (OAL_SUCC != hmac_rx_copy_netbuff_etc(&pst_netbuf_copy, pst_netbuf,
                                                     pst_rx_ctrl->uc_mac_vap_id, &pst_copy_frame_hdr)) {
                OAM_WARNING_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX,
                                 "{hmac_rx_process_data_ap::send mcast pkt to air fail.}");

                OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_no_buff_dropped, 1);
                continue;
            }

            hmac_rx_lan_frame_classify_etc(pst_vap, pst_netbuf, pst_frame_hdr);  // �ϱ������

#ifdef _PRE_WLAN_FEATURE_ISOLATION
            pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf_copy);

            /* ��ȡ֡ͷ��Ϣ */
            pst_frame_hdr = (mac_ieee80211_frame_stru *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_ctrl);
            mac_rx_get_da(pst_frame_hdr, &puc_da);

            en_forward = hmac_isolation_filter(&pst_vap->st_vap_base_info, puc_da);
            if (en_forward == CS_ISOLATION_FORWORD_DROP) {
                /* �ͷŵ�ǰ������MPDUռ�õ�netbuf. 2014.7.29 cause memory leak bug fixed */
                /* OAL_IO_PRINT("isolation drop %d-%d\n",uc_netbuf_num,uc_buf_nums);1-1 */
                hmac_rx_free_netbuf_etc(pst_netbuf_copy, (oal_uint16)uc_buf_nums);
                continue;
            }
#endif

            /* ��MPDU�����ɵ���MSDU�������е�MSDU���һ��netbuf�� */
            hmac_rx_prepare_msdu_list_to_wlan(pst_vap, &st_w2w_netbuf_hdr, pst_netbuf_copy, pst_copy_frame_hdr);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_ISOLATION
        en_forward = hmac_isolation_filter(&pst_vap->st_vap_base_info, puc_da);
        if (en_forward == CS_ISOLATION_FORWORD_DROP) {
            /* �ͷŵ�ǰ������MPDUռ�õ�netbuf. 2014.7.29 cause memory leak bug fixed */
            /* OAL_IO_PRINT("isolation drop %d-%d\n",uc_netbuf_num,uc_buf_nums);1-1 */
            hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
            /* return OAL_SUCC; bug fixed */
            continue;
        }
#endif

#ifdef _PRE_WLAN_FEATURE_CAR
        /* ���������� car �������� */
        ul_rslt = hmac_car_process_uplink(MAC_GET_RX_CB_TA_USER_IDX(pst_rx_ctrl),
                                          (oal_void *)pst_vap, pst_netbuf, HMAC_CAR_UPLINK);
        if (ul_rslt != OAL_SUCC) {
            OAM_WARNING_LOG1(0, 0,
                             "hmac_rx_process_data_ap_tcp_ack_opt_etc: uplink car: DROP PACKET! ret[%d]", ul_rslt);
            /* �ͷŵ�ǰ������MPDUռ�õ�netbuf */
            hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);
            continue;
        }
#endif

        ul_rslt = OAL_FAIL;
        us_user_dix = MAC_INVALID_USER_ID;
        if ((OAL_FALSE == mac_is_4addr((oal_uint8 *)pst_frame_hdr)) ||
            ((OAL_TRUE == mac_is_4addr((oal_uint8 *)pst_frame_hdr)) && (pst_rx_ctrl->bit_amsdu_enable == OAL_FALSE))) {
            /* ��ȡĿ�ĵ�ַ��Ӧ���û�ָ�� */
            ul_rslt = mac_vap_find_user_by_macaddr_etc(&pst_vap->st_vap_base_info, puc_da, &us_user_dix);
            if (ul_rslt == OAL_ERR_CODE_PTR_NULL) { /* �����û�ʧ�� */
                /* �ͷŵ�ǰ������MPDUռ�õ�netbuf */
                hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
                OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);
                continue;
            }

#if defined(_PRE_WLAN_FEATURE_WDS) || defined(_PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA)
            if (ul_rslt != OAL_SUCC) {
                ul_rslt = hmac_find_valid_user_by_wds_sta(pst_vap, puc_da, &us_user_dix);
            }
#endif
        }

        /* û���ҵ���Ӧ���û� */
        if (ul_rslt != OAL_SUCC) {
            OAM_INFO_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX,
                          "{hmac_rx_lan_frame_classify_etc::the frame is a unique frame.}");
            /* Ŀ���û�����AP���û����У�����wlan_to_lanת���ӿ� */
            hmac_rx_lan_frame_classify_etc(pst_vap, pst_netbuf, pst_frame_hdr);
            continue;
        }

        /* Ŀ���û�����AP���û����У�����WLAN_TO_WLANת�� */
        pst_hmac_da_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(us_user_dix);

        if (pst_hmac_da_user == OAL_PTR_NULL) {
            OAM_WARNING_LOG1(st_event_hdr.uc_vap_id, OAM_SF_RX,
                             "{hmac_rx_lan_frame_classify_etc::pst_hmac_da_user[%d] null.}", us_user_dix);
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);

            hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
            continue;
        }

        if (pst_hmac_da_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
            OAM_WARNING_LOG0(st_event_hdr.uc_vap_id, OAM_SF_RX,
                             "{hmac_rx_lan_frame_classify_etc::the station is not associated with ap.}");
            OAM_STAT_VAP_INCR(pst_vap->st_vap_base_info.uc_vap_id, rx_da_check_dropped, 1);

            hmac_rx_free_netbuf_etc(pst_netbuf, (oal_uint16)uc_buf_nums);
            hmac_mgmt_send_deauth_frame_etc(&pst_vap->st_vap_base_info, puc_da, MAC_NOT_AUTHED, OAL_FALSE);

            continue;
        }

        /* ��Ŀ�ĵ�ַ����Դ������ֵ�ŵ�cb�ֶ��У�user��asoc id���ڹ�����ʱ�򱻸�ֵ */
        MAC_GET_RX_CB_DA_USER_IDX(pst_rx_ctrl) = pst_hmac_da_user->st_user_base_info.us_assoc_id;

        /* ��MPDU�����ɵ���MSDU�������е�MSDU���һ��netbuf�� */
        hmac_rx_prepare_msdu_list_to_wlan(pst_vap, &st_w2w_netbuf_hdr, pst_netbuf, pst_frame_hdr);
    }

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_TO_LAN);

    /*  ��MSDU���������������̴��� */
    if (OAL_FALSE == oal_netbuf_list_empty(&st_w2w_netbuf_hdr) && OAL_PTR_NULL != oal_netbuf_tail(&st_w2w_netbuf_hdr) &&
        OAL_PTR_NULL != oal_netbuf_peek(&st_w2w_netbuf_hdr)) {
        OAL_NETBUF_NEXT((oal_netbuf_tail(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;
        OAL_NETBUF_PREV((oal_netbuf_peek(&st_w2w_netbuf_hdr))) = OAL_PTR_NULL;

        hmac_rx_transmit_to_wlan(&st_event_hdr, &st_w2w_netbuf_hdr);
    }

    if (OAL_TRUE == hmac_get_rxthread_enable_etc()) {
        hmac_rxdata_sched_etc();
    }

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_END);
}


oal_uint32 hmac_rx_process_data_sta_tcp_ack_opt_etc(hmac_vap_stru *pst_vap, oal_netbuf_head_stru *pst_netbuf_header)
{
    /* ����Ҫ�ϱ���֡��һ���Ӵ��� */
    hmac_rx_lan_frame_etc(pst_netbuf_header);

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_END);
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_HMAC_END);
#ifdef _PRE_WLAN_PROFLING_MIPS
    oal_profiling_stop_rx_save();
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MONITOR

oal_uint32 hmac_sniffer_save_data(oal_netbuf_stru *pst_netbuf, oal_uint16 us_netbuf_num)
{
    oal_netbuf_stru *pst_temp_netbuf = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL; /* ָ��MPDU���ƿ���Ϣ��ָ�� */
    oal_uint8 *puc_mac_payload_addr = OAL_PTR_NULL;
    hal_sniffer_rx_status_stru *pst_rx_status;
    hal_sniffer_rx_statistic_stru *pst_sniffer_rx_statistic;
    ieee80211_radiotap_stru st_ieee80211_radiotap;
    hal_statistic_stru *pst_per_rate;
    oal_uint32 *pul_rate_kbps;
    oal_int32 iPacketLength = 0;

    /******************************************************************************************************************/
    /*                   netbuf                                                                                  */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |frame body |rx status |rx statistic | rate_kbps |  rate info                                                  */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |variable   |   6      |     8       | 4         |      4                                                      */
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/

    while (us_netbuf_num != 0) {
        pst_temp_netbuf = pst_netbuf;
        if (pst_temp_netbuf == OAL_PTR_NULL) {
            break;
        }

        pst_netbuf = OAL_NETBUF_NEXT(pst_temp_netbuf);

        /* ��ȡ��MPDU�Ŀ�����Ϣ */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_temp_netbuf);
        puc_mac_payload_addr = oal_netbuf_payload(pst_temp_netbuf);
        pst_rx_status = (hal_sniffer_rx_status_stru *)(puc_mac_payload_addr + pst_rx_ctrl->us_frame_len -
            OAL_SIZEOF(hal_sniffer_rx_status_stru) - OAL_SIZEOF(hal_sniffer_rx_statistic_stru) -
            OAL_SIZEOF(oal_uint32) - OAL_SIZEOF(hal_statistic_stru));
        pst_sniffer_rx_statistic = (hal_sniffer_rx_statistic_stru *)(puc_mac_payload_addr + pst_rx_ctrl->us_frame_len -
            OAL_SIZEOF(hal_sniffer_rx_statistic_stru) - OAL_SIZEOF(oal_uint32) - OAL_SIZEOF(hal_statistic_stru));
        pul_rate_kbps = (oal_uint32 *)(puc_mac_payload_addr + pst_rx_ctrl->us_frame_len -
            OAL_SIZEOF(oal_uint32) - OAL_SIZEOF(hal_statistic_stru));
        pst_per_rate =
           (hal_statistic_stru *)(puc_mac_payload_addr + pst_rx_ctrl->us_frame_len - OAL_SIZEOF(hal_statistic_stru));

        iPacketLength = pst_rx_ctrl->us_frame_len - SNIFFER_RX_INFO_SIZE;
        if (iPacketLength < 0) {
            OAM_WARNING_LOG1(0, OAM_SF_RX, "iPacketLength(%d) < 0", iPacketLength);
            continue;
        }
#ifdef _PRE_WLAN_FEATURE_MONITOR_DEBUG
        OAM_WARNING_LOG4(0, OAM_SF_RX,
                         "{hmac_sniffer_save_data:: chn[%d], frame len[%d], pul_rate_kbps[%d], uc_bandwidth[%d].}",
                         pst_rx_ctrl->uc_channel_number,
                         pst_rx_ctrl->us_frame_len,
                         (*pul_rate_kbps),
                         pst_per_rate->uc_bandwidth);

        OAM_WARNING_LOG4(0, OAM_SF_RX,
                         "{hmac_sniffer_save_data::c_rssi_dbm[%d], c_snr_ant0[%d], c_snr_ant1[%d], c_ant0_rssi[%d].}",
                         pst_sniffer_rx_statistic->c_rssi_dbm,
                         pst_sniffer_rx_statistic->c_snr_ant0,
                         pst_sniffer_rx_statistic->c_snr_ant1,
                         pst_sniffer_rx_statistic->c_ant0_rssi);

        OAM_WARNING_LOG4(0, OAM_SF_RX,
            "{hmac_sniffer_save_data::status[%d],freq_bandwidth_mode[%d], ext_spatial_streams[%d], protocol_mode[%d].}",
            pst_rx_status->bit_dscr_status,
            pst_rx_status->bit_freq_bandwidth_mode,
            pst_rx_status->bit_ext_spatial_streams,
            pst_rx_status->un_nss_rate.st_rate.bit_protocol_mode);
#endif
        hmac_sniffer_fill_radiotap(&st_ieee80211_radiotap, pst_rx_ctrl, pst_rx_status,
                                   pst_sniffer_rx_statistic, puc_mac_payload_addr, pul_rate_kbps, pst_per_rate);

#ifdef _PRE_WLAN_FEATURE_SNIFFER
        /* �����ն��ṩ�Ľӿ�д�뵽�ļ� */
        proc_sniffer_write_file((oal_uint8 *)&st_ieee80211_radiotap, OAL_SIZEOF(ieee80211_radiotap_stru),
                                (oal_uint8 *)oal_netbuf_payload(pst_temp_netbuf),
                                iPacketLength,
                                DIRECTION_MONITOR_MODE);
#endif

        us_netbuf_num--;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 hmac_rx_process_data_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = OAL_PTR_NULL;
    frw_event_hdr_stru *pst_event_hdr = OAL_PTR_NULL;
    dmac_wlan_drx_event_stru *pst_wlan_rx_event = OAL_PTR_NULL;
    oal_netbuf_stru *pst_netbuf = OAL_PTR_NULL; /* ������ʱ������һ����Ҫ������netbufָ�� */
    oal_uint16 us_netbuf_num;                   /* netbuf�����ĸ��� */
    oal_netbuf_head_stru st_netbuf_header;      /* �洢�ϱ������������� */
    oal_netbuf_stru *pst_temp_netbuf = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
#ifdef _PRE_WLAN_TCP_OPT
    hmac_device_stru *pst_hmac_device = OAL_PTR_NULL;
#endif

    if (OAL_UNLIKELY(pst_event_mem == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_data_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_START);
    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_DATA_START);

    /* ��ȡ�¼�ͷ���¼��ṹ��ָ�� */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_wlan_rx_event = (dmac_wlan_drx_event_stru *)(pst_event->auc_event_data);
    pst_netbuf = pst_wlan_rx_event->pst_netbuf;
    us_netbuf_num = pst_wlan_rx_event->us_netbuf_num;

    OAM_PROFILING_RX_STATISTIC(OAM_PROFILING_FUNC_RX_HMAC_BASE_INFO);

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        OAM_ERROR_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_event::us_netbuf_num = %d.}", us_netbuf_num);
        return OAL_SUCC; /* ������¼�����������Ϊ�˷�ֹ51��UT�ҵ� ���� true */
    }

#if defined(_PRE_WLAN_TCP_OPT) || defined(_PRE_WLAN_FEATURE_MONITOR)
    pst_hmac_device = hmac_res_get_mac_dev_etc(pst_event_hdr->uc_device_id);
    if (pst_hmac_device == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_process_data_event::pst_hmac_device null.}");
        hmac_rx_free_netbuf_etc(pst_netbuf, us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_MONITOR
    if (pst_hmac_device->en_monitor_mode == OAL_TRUE) {
        pst_hmac_device->ul_data_frames_cnt += us_netbuf_num;

        hmac_sniffer_save_data(pst_netbuf, us_netbuf_num);
        hmac_rx_free_netbuf_etc(pst_netbuf, us_netbuf_num);
        return OAL_SUCC;
    }
#endif

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (pst_hmac_vap == OAL_PTR_NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{hmac_rx_process_data_event::pst_hmac_vap null.}");
        hmac_rx_free_netbuf_etc(pst_netbuf, us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* If mib info is null ptr,release the netbuf */
    if (pst_hmac_vap->st_vap_base_info.pst_mib_info == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_rx_process_data_event::pst_mib_info null.}");
        hmac_rx_free_netbuf_etc(pst_netbuf, us_netbuf_num);
        return OAL_SUCC;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    /* ͳ���հ�������������ȫ�ֱ��� */
    hmac_auto_freq_wifi_rx_stat(us_netbuf_num);
#endif

    /* ������netbuffȫ�������� */
    oal_netbuf_list_head_init(&st_netbuf_header);
    while (us_netbuf_num != 0) {
        pst_temp_netbuf = pst_netbuf;
        if (pst_temp_netbuf == OAL_PTR_NULL) {
            break;
        }

        pst_netbuf = OAL_NETBUF_NEXT(pst_temp_netbuf);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        hmac_auto_freq_wifi_rx_bytes_stat(OAL_NETBUF_LEN(pst_temp_netbuf));
#endif
        oal_netbuf_list_tail_nolock(&st_netbuf_header, pst_temp_netbuf);
        us_netbuf_num--;
    }

    if (us_netbuf_num != 0) {
        OAM_ERROR_LOG2(0, OAM_SF_RX, "{hmac_rx_process_data_event::us_netbuf_num[%d], event_buf_num[%d].}",
                       us_netbuf_num, pst_wlan_rx_event->us_netbuf_num);
    }

    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_GET_NETBUF_LIST);

    hmac_rx_process_data_filter_etc(&st_netbuf_header, pst_wlan_rx_event->pst_netbuf, pst_wlan_rx_event->us_netbuf_num);

#ifdef _PRE_WLAN_TCP_OPT
    if (pst_hmac_device->sys_tcp_rx_ack_opt_enable == OAL_TRUE) {
        hmac_transfer_rx_handle(pst_hmac_device, pst_hmac_vap, &st_netbuf_header);
    }
#endif

    OAL_MIPS_RX_STATISTIC(HMAC_PROFILING_FUNC_RX_TCP_ACK_OPT);

    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_rx_process_data_sta_tcp_ack_opt_etc(pst_hmac_vap, &st_netbuf_header);
    } else if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_rx_process_data_ap_tcp_ack_opt_etc(pst_hmac_vap, &st_netbuf_header);
    } else {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{hmac_rx_process_data_event::RX FAIL! VAP MODE[%d]!.}",
                         pst_hmac_vap->st_vap_base_info.en_vap_mode);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG

OAL_STATIC const oal_uint8 *hmac_dhcp_get_option(const oal_uint8 *puc_opt_buff,
                                                 oal_uint32 ul_opt_total_len,
                                                 oal_uint8 uc_opt,
                                                 oal_uint32 *pul_option_len,
                                                 oal_uint32 ul_expect_len)
{
    const oal_uint8 *puc_buff = puc_opt_buff;
    const oal_uint8 *puc_buff_end = puc_buff + ul_opt_total_len;
    oal_uint8 uc_opt_len = 0;
    oal_uint8 uc_len = 0;
    oal_uint8 uc_opt_type = 0;
    const oal_uint8 *puc_opt_val = OAL_PTR_NULL;

    /* DHCP Options are in TLV format with T and L each being a single
     * byte.We must make sure there is enough room to read both T and L.
     */
    while (puc_buff + 1 < puc_buff_end) {
        uc_opt_type = *puc_buff;
        puc_buff++;  // point to length

        if (uc_opt_type == uc_opt) {
            puc_opt_val = puc_buff + 1;
            uc_opt_len += OAL_MIN(*puc_buff, puc_buff_end - puc_opt_val);
        }
        switch (uc_opt_type) {
            case DHO_PAD:
                continue;
            case DHO_END:
                break;
        }

        uc_len = *puc_buff++;
        puc_buff += uc_len;  // skip value, now point to type
    }

    if (ul_expect_len > 0 && uc_opt_len != ul_expect_len) {
        return OAL_PTR_NULL;  // unexpect length of value
    }

    if (pul_option_len) {
        *pul_option_len = uc_opt_len;
    }

    return puc_opt_val;
}

static oal_int32 hmac_dhcp_get_option_uint32(oal_uint32 *pul_option_val, const oal_uint8 *puc_opt_buff,
                                             oal_uint32 ul_opt_len, oal_uint8 uc_option)
{
    const oal_uint8 *puc_val = hmac_dhcp_get_option(puc_opt_buff, ul_opt_len, uc_option,
                                                    OAL_PTR_NULL, OAL_SIZEOF(oal_uint32));
    oal_uint32 ul_val;

    if (puc_val == OAL_PTR_NULL) {
        return OAL_FAIL;
    }
    if (EOK != memcpy_s(&ul_val, OAL_SIZEOF(ul_val), puc_val, OAL_SIZEOF(ul_val))) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_dhcp_get_option_uint32::memcpy fail!");
        return OAL_FAIL;
    }
    if (pul_option_val) {
        *pul_option_val = OAL_NTOH_32(ul_val);
    }
    return OAL_SUCC;
}

static oal_int32 hmac_dhcp_get_option_uint8(oal_uint8 *puc_option_val, const oal_uint8 *puc_opt_buff,
                                            oal_uint32 ul_opt_len, oal_uint8 uc_option)
{
    const oal_uint8 *puc_val = hmac_dhcp_get_option(puc_opt_buff, ul_opt_len, uc_option,
                                                    OAL_PTR_NULL, OAL_SIZEOF(oal_uint8));

    if (puc_val == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    if (puc_option_val) {
        *puc_option_val = *(puc_val);
    }
    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_parse_special_dhcp_packet(oal_uint8 *puc_buff, oal_uint32 ul_buflen, oal_uint8 *puc_dst)
{
    oal_uint8 uc_type = 0;
    oal_uint32 ul_req_ip = 0;
    oal_uint32 ul_req_srv = 0;
    oal_uint32 ul_len;
    oal_dhcp_packet_stru *pst_msg;
    if (ul_buflen <= OAL_SIZEOF(oal_dhcp_packet_stru)) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "invalid dhcp packet\n");
        return;
    }
    pst_msg = (oal_dhcp_packet_stru *)puc_buff;
    ul_len = ul_buflen - OAL_SIZEOF(oal_dhcp_packet_stru);

    if (hmac_dhcp_get_option_uint8(&uc_type, &pst_msg->options[4], ul_len, DHO_MESSAGETYPE) == OAL_FAIL) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "get message type failed\n");
        return;
    }

    if (uc_type == DHCP_DISCOVER) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "type: DHCP_DISCOVER\n");
    } else if (uc_type == DHCP_OFFER) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "type: DHCP_OFFER, ip:%d.x.x.%d srvip:%d.x.x.%d MAC:" HWMACSTR "\n",
                     IPADDR(pst_msg->yiaddr), IPADDR(pst_msg->siaddr), HWMAC2STR(puc_dst));
    } else if (uc_type == DHCP_REQUEST) {
        hmac_dhcp_get_option_uint32(&ul_req_ip, &pst_msg->options[4], ul_len, DHO_IPADDRESS);
        hmac_dhcp_get_option_uint32(&ul_req_srv, &pst_msg->options[4], ul_len, DHO_SERVERID);
        ul_req_ip = OAL_NTOH_32(ul_req_ip);
        ul_req_srv = OAL_NTOH_32(ul_req_srv);
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "type: DHCP_REQUEST, ip:%d.x.x.%d srvip:%d.x.x.%d\n",
                     IPADDR(pst_msg->yiaddr), IPADDR(pst_msg->siaddr));
    } else if (uc_type == DHCP_ACK) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "type: DHCP_ACK MAC:" HWMACSTR "\n", HWMAC2STR(puc_dst));
    } else if (uc_type == DHCP_NAK) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "type: DHCP_NAK MAC:" HWMACSTR "\n", HWMAC2STR(puc_dst));
    }
}


OAL_STATIC oal_void hmac_parse_dns_query(oal_uint8 *puc_msg, oal_uint32 ul_msg_len)
{
    oal_uint8 *puc_buff = puc_msg;
    oal_uint8 *puc_end = puc_msg + ul_msg_len;
    oal_uint8 uc_qlen = 0;
    oal_uint8 auc_domain[DNS_MAX_DOMAIN_LEN + 2] = { 0 };
    oal_uint8 *puc_domain = auc_domain;
    oal_uint8 *puc_domain_end = auc_domain + DNS_MAX_DOMAIN_LEN;

    while (puc_buff < puc_end) {
        uc_qlen = *puc_buff++;
        if (uc_qlen > 0) {
            if ((puc_buff + uc_qlen < puc_end) && (puc_domain + uc_qlen <= puc_domain_end)) {
                if (EOK != memcpy_s(puc_domain, uc_qlen, puc_buff, uc_qlen)) {
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_parse_dns_query::memcpy fail!");
                    return;
                }
                puc_domain += uc_qlen;
                *puc_domain = '.';
                *(puc_domain + 1) = '\0';
                puc_domain += 1;
            }
            puc_buff += uc_qlen;
        } else {
            // only printf one record of query item
            break;
        }
    }
    OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "domain name: %s \n", auc_domain);
}


OAL_STATIC oal_void hmac_parse_dns_answer(oal_uint8 *puc_msg, oal_uint16 us_msg_len, oal_uint16 us_qdcount)
{
    oal_uint8 *puc_buff = puc_msg;
    oal_uint8 *puc_end = puc_msg + us_msg_len;
    oal_uint8 uc_qlen = 0;
    oal_uint16 us_type = 0;
    oal_uint16 us_rdlen = 0;
    oal_uint32 ul_ipv4 = 0;
    oal_uint8 auc_domain[DNS_MAX_DOMAIN_LEN + 2] = { 0 };
    oal_uint8 auc_domain_s[DNS_MAX_DOMAIN_LEN + 2] = { 0 };
    oal_uint8 *puc_domain = auc_domain;
    oal_uint8 *puc_domain_end = auc_domain + DNS_MAX_DOMAIN_LEN;
    oal_int32 l_ret = EOK;
    // skip Questions
    while (us_qdcount > 0 && puc_buff < puc_end) {
        uc_qlen = *puc_buff++;
        if (uc_qlen > 0) {
            if ((puc_buff + uc_qlen < puc_end) && (puc_domain + uc_qlen <= puc_domain_end)) {
                l_ret += memcpy_s(puc_domain, uc_qlen, puc_buff, uc_qlen);
                puc_domain += uc_qlen;
                *puc_domain = '.';
                *(puc_domain + 1) = '\0';
                puc_domain += 1;
            }
            puc_buff += uc_qlen;
        } else {
            puc_buff += 4;  // class: 2 bytes, type: 2 bytes
            us_qdcount--;
            l_ret += memcpy_s(auc_domain_s, DNS_MAX_DOMAIN_LEN + 2, auc_domain, DNS_MAX_DOMAIN_LEN + 2);
            puc_domain = auc_domain;
            puc_domain_end = auc_domain + DNS_MAX_DOMAIN_LEN;
            memset_s(auc_domain, OAL_SIZEOF(auc_domain), 0, OAL_SIZEOF(auc_domain));
        }
    }

    // parse Answers
    while (puc_buff + 12 < puc_end) {
        puc_buff += 2;  // name: 2 bytes;
        l_ret += memcpy_s(&us_type, OAL_SIZEOF(us_type), puc_buff, OAL_SIZEOF(us_type));
        puc_buff += 8;  // type, class: 2bytes, ttl: 4bytes;
        l_ret += memcpy_s(&us_rdlen, OAL_SIZEOF(us_rdlen), puc_buff, OAL_SIZEOF(us_rdlen));
        puc_buff += 2;
        us_type = OAL_NTOH_16(us_type);
        us_rdlen = OAL_NTOH_16(us_rdlen);
        if (us_type == OAL_NS_T_A && us_rdlen == 4) {
            l_ret += memcpy_s(&ul_ipv4, us_rdlen, puc_buff, us_rdlen);
            OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "domain name: %s %d.x.x.%d\n", auc_domain_s, IPADDR(ul_ipv4));
            return;
        }
        puc_buff += us_rdlen;
    }
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "hmac_parse_dns_answer::memcpy fail!");
        return;
    }

    OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "domain name: %s\n", auc_domain_s);
}


OAL_STATIC oal_void hmac_parse_special_dns_packet(oal_uint8 *puc_msg, oal_uint32 ul_msg_len)
{
    oal_uint8 uc_qr, uc_opcode, uc_rcode;
    oal_uint16 us_flag, us_qdcount;
    oal_uint32 ul_dns_hdr_len = OAL_SIZEOF(oal_dns_hdr_stru);
    oal_dns_hdr_stru *pst_dns_hdr = NULL;

    if (ul_dns_hdr_len >= ul_msg_len) {
        OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "invalid dns packet\n");
        return;
    }

    pst_dns_hdr = (oal_dns_hdr_stru *)puc_msg;
    us_flag = OAL_NTOH_16(pst_dns_hdr->flags);
    us_qdcount = OAL_NTOH_16(pst_dns_hdr->qdcount);
    uc_qr = DNS_GET_QR_FROM_FLAG(us_flag);
    uc_opcode = DNS_GET_OPCODE_FROM_FLAG(us_flag);
    uc_rcode = DNS_GET_RCODE_FROM_FLAG(us_flag);

    if (uc_qr == OAL_NS_Q_REQUEST) {
        if (uc_opcode == OAL_NS_O_QUERY && us_qdcount > 0) {
            hmac_parse_dns_query(puc_msg + ul_dns_hdr_len, ul_msg_len - ul_dns_hdr_len);
        } else {
            OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "dont parse dns request pkt, opcode: %u, qd: %u\n",
                         uc_opcode, us_qdcount);
        }
    } else if (uc_qr == OAL_NS_Q_RESPONSE) {
        if (uc_opcode == OAL_NS_O_QUERY && uc_rcode == OAL_NS_R_NOERROR) {
            hmac_parse_dns_answer(puc_msg + ul_dns_hdr_len, ul_msg_len - ul_dns_hdr_len, us_qdcount);
        } else {
            OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "dont parse dns response pkt, opcode: %u, rcode: %u\n",
                         uc_opcode, uc_rcode);
        }
    }
}


oal_void hmac_parse_special_ipv4_packet(oal_uint8 *puc_pktdata, oal_uint32 ul_datalen,
                                        hmac_pkt_direction_enum en_pkt_direction)
{
    oal_uint32 ul_iphdr_len;
    const struct iphdr *pst_iph;
    struct udphdr *pst_uh;
    oal_uint16 us_src_port;
    oal_uint16 us_dst_port;
    oal_uint16 us_udp_len;
    oal_uint16 us_tot_len;
    oal_uint16 us_udphdr_len;
    oal_uint8 *puc_uplayer_data;

    // invalid ipv4 packet
    if ((puc_pktdata == NULL) || (ul_datalen <= ETH_HLEN + 20)) {
        return;
    }
    pst_iph = (struct iphdr *)(puc_pktdata + ETH_HLEN);
    ul_iphdr_len = pst_iph->ihl * 4;
    us_tot_len = OAL_NTOH_16(pst_iph->tot_len);

    // invalid ipv4 packet
    if (ul_datalen < (us_tot_len + ETH_HLEN)) {
        return;
    }

    if (pst_iph->protocol == IPPROTO_UDP) {
        pst_uh = (struct udphdr *)(puc_pktdata + ETH_HLEN + ul_iphdr_len);
        us_src_port = OAL_NTOH_16(pst_uh->source);
        us_dst_port = OAL_NTOH_16(pst_uh->dest);
        us_udp_len = OAL_NTOH_16(pst_uh->len);
        us_udphdr_len = (oal_uint16)OAL_SIZEOF(struct udphdr);

        // invalid udp packet
        if (us_udp_len <= us_udphdr_len) {
            return;
        }

        us_udp_len -= us_udphdr_len;                                                              // skip udp header
        puc_uplayer_data = (oal_uint8 *)(puc_pktdata + ETH_HLEN + ul_iphdr_len + us_udphdr_len);  // skip udp header
        if (us_src_port == DNS_SERVER_PORT || us_dst_port == DNS_SERVER_PORT) {
            OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "%s parse dns packet\n", GET_PKT_DIRECTION_STR(en_pkt_direction));
            hmac_parse_special_dns_packet(puc_uplayer_data, us_udp_len);
        } else if ((us_src_port == DHCP_SERVER_PORT && us_dst_port == DHCP_CLIENT_PORT) ||
                   (us_dst_port == DHCP_SERVER_PORT && us_src_port == DHCP_CLIENT_PORT)) {
            OAL_IO_PRINT(WIFI_SEPCIAL_IPV4_PKT_TAG "%s parse dhcp packet\n", GET_PKT_DIRECTION_STR(en_pkt_direction));
            hmac_parse_special_dhcp_packet(puc_uplayer_data, us_udp_len, puc_pktdata + ETH_ALEN);
        }
    }
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
