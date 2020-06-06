

/* 1 ͷ�ļ����� */
#include "oal_mem.h"
#include "oal_net.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_data.h"
#include "dmac_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DATA_ROM_C

/* 2 ȫ�ֱ������� */
mac_data_cb g_st_mac_data_rom_cb = { OAL_PTR_NULL };

/* 3 ����ʵ�� */

oal_bool_enum_uint8 mac_is_dhcp_port_etc(mac_ip_header_stru *pst_ip_hdr)
{
    udp_hdr_stru *pst_udp_hdr = OAL_PTR_NULL;
    /* DHCP�жϱ�׼: udpЭ�飬ipͷ��fragment offset�ֶ�Ϊ0��Ŀ�Ķ˿ں�Ϊ67��68 */
    if (pst_ip_hdr->uc_protocol == MAC_UDP_PROTOCAL && ((pst_ip_hdr->us_frag_off & 0xFF1F) == 0)) {
        pst_udp_hdr = (udp_hdr_stru *)(pst_ip_hdr + 1);

        if (OAL_NET2HOST_SHORT(pst_udp_hdr->us_des_port) == 67 ||
            OAL_NET2HOST_SHORT(pst_udp_hdr->us_des_port) == 68) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_is_nd_etc(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    oal_icmp6hdr_stru *pst_icmp6hdr = OAL_PTR_NULL;

    if (pst_ipv6hdr->nexthdr == OAL_IPPROTO_ICMPV6) {
        pst_icmp6hdr = (oal_icmp6hdr_stru *)(pst_ipv6hdr + 1);

        if ((pst_icmp6hdr->icmp6_type == MAC_ND_RSOL) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_RADVT) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_NSOL) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_NADVT) ||
            (pst_icmp6hdr->icmp6_type == MAC_ND_RMES)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_is_dhcp6_etc(oal_ipv6hdr_stru *pst_ipv6hdr)
{
    udp_hdr_stru *pst_udp_hdr = OAL_PTR_NULL;

    if (pst_ipv6hdr->nexthdr == MAC_UDP_PROTOCAL) {
        pst_udp_hdr = (udp_hdr_stru *)(pst_ipv6hdr + 1);

        if (pst_udp_hdr->us_des_port == OAL_HOST2NET_SHORT(MAC_IPV6_UDP_DES_PORT) ||
            pst_udp_hdr->us_des_port == OAL_HOST2NET_SHORT(MAC_IPV6_UDP_SRC_PORT)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

mac_data_type_enum_uint8 mac_get_arp_type_by_arphdr(oal_eth_arphdr_stru *pst_rx_arp_hdr)
{
    if (MAC_ARP_REQUEST == OAL_NET2HOST_SHORT(pst_rx_arp_hdr->us_ar_op)) {
        return MAC_DATA_ARP_REQ;
    } else if (MAC_ARP_RESPONSE == OAL_NET2HOST_SHORT(pst_rx_arp_hdr->us_ar_op)) {
        return MAC_DATA_ARP_RSP;
    }

    return MAC_DATA_BUTT;
}


oal_uint8 mac_get_data_type_from_8023_etc(oal_uint8 *puc_frame_hdr, mac_netbuff_payload_type uc_hdr_type)
{
    mac_ip_header_stru  *pst_ip = OAL_PTR_NULL;
    oal_uint8           *puc_frame_body = OAL_PTR_NULL;
    oal_uint16           us_ether_type;
    oal_uint8            uc_datatype = MAC_DATA_BUTT;

    if (puc_frame_hdr == OAL_PTR_NULL) {
        return uc_datatype;
    }

    if (uc_hdr_type == MAC_NETBUFF_PAYLOAD_ETH) {
        us_ether_type = ((mac_ether_header_stru *)puc_frame_hdr)->us_ether_type;
        puc_frame_body = puc_frame_hdr + (oal_uint16)OAL_SIZEOF(mac_ether_header_stru);
    } else if (uc_hdr_type == MAC_NETBUFF_PAYLOAD_SNAP) {
        us_ether_type = ((mac_llc_snap_stru *)puc_frame_hdr)->us_ether_type;
        puc_frame_body = puc_frame_hdr + (oal_uint16)OAL_SIZEOF(mac_llc_snap_stru);
    } else {
        return uc_datatype;
    }

    switch (us_ether_type) {
            /* lint -e778 */ /* ����Info-- Constant expression evaluates to 0 in operation '&' */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IP):
            /* ��IP TOS�ֶ�Ѱ�����ȼ� */
            /* ----------------------------------------------------------------------
                tosλ����
             ----------------------------------------------------------------------
            | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
            | �����ȼ�  | ʱ�� | ������ | �ɿ��� | ����ɱ� | ���� |
             ---------------------------------------------------------------------- */
            pst_ip = (mac_ip_header_stru *)puc_frame_body; /* ƫ��һ����̫��ͷ��ȡipͷ */

            if (mac_is_dhcp_port_etc(pst_ip) == OAL_TRUE) {
                uc_datatype = MAC_DATA_DHCP;
            }
            break;
        case OAL_HOST2NET_SHORT(ETHER_TYPE_IPV6):
            /* ��IPv6 traffic class�ֶλ�ȡ���ȼ� */
            /* ----------------------------------------------------------------------
                IPv6��ͷ ǰ32Ϊ����
             -----------------------------------------------------------------------
            | �汾�� | traffic class   | ������ʶ |
            | 4bit   | 8bit(ͬipv4 tos)|  20bit   |
            ----------------------------------------------------------------------- */
            /* �����ND֡�������VO���з��� */
            if (mac_is_nd_etc((oal_ipv6hdr_stru *)puc_frame_body) == OAL_TRUE) {
                uc_datatype = MAC_DATA_ND;
            }

            /* �����DHCPV6֡ */
            else if (mac_is_dhcp6_etc((oal_ipv6hdr_stru *)puc_frame_body) == OAL_TRUE) {
                uc_datatype = MAC_DATA_DHCPV6;
            }
            break;
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PAE):
            /* �����EAPOL֡�������VO���з��� */
            uc_datatype = MAC_DATA_EAPOL; /* eapol */
            break;
        /* TDLS֡��������������������ȼ�TID���� */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_TDLS):
            uc_datatype = MAC_DATA_TDLS;
            break;
        /* PPPOE֡������������(���ֽ׶�, �Ự�׶�)��������ȼ�TID���� */
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PPP_DISC):
        case OAL_HOST2NET_SHORT(ETHER_TYPE_PPP_SES):
            uc_datatype = MAC_DATA_PPPOE;
            break;
        case OAL_HOST2NET_SHORT(ETHER_TYPE_WAI):
            uc_datatype = MAC_DATA_WAPI;
            break;
        case OAL_HOST2NET_SHORT(ETHER_TYPE_VLAN):
            uc_datatype = MAC_DATA_VLAN;
            break;
        case OAL_HOST2NET_SHORT(ETHER_TYPE_ARP):
            /* �����ARP֡�������VO���з��� */
            uc_datatype = (oal_uint8)mac_get_arp_type_by_arphdr((oal_eth_arphdr_stru *)puc_frame_body);
            break;
        /* lint +e778 */
        default:
            uc_datatype = MAC_DATA_BUTT;
            break;
    }

    if (g_st_mac_data_rom_cb.data_type_from_8023_cb != OAL_PTR_NULL) {
        g_st_mac_data_rom_cb.data_type_from_8023_cb(puc_frame_hdr, uc_hdr_type, &uc_datatype);
    }

    return uc_datatype;
}

oal_uint8 mac_get_data_type_etc(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8          uc_datatype;
    mac_llc_snap_stru *pst_snap = OAL_PTR_NULL;

    if (pst_netbuff == OAL_PTR_NULL) {
        return MAC_DATA_BUTT;
    }

    pst_snap = (mac_llc_snap_stru *)oal_netbuf_payload(pst_netbuff);
    if (pst_snap == OAL_PTR_NULL) {
        return MAC_DATA_BUTT;
    }

    uc_datatype = mac_get_data_type_from_8023_etc((oal_uint8 *)pst_snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return uc_datatype;
}

oal_uint16 mac_get_eapol_keyinfo_etc(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8  uc_datatype;
    oal_uint8 *puc_payload = OAL_PTR_NULL;

    uc_datatype = mac_get_data_type_etc(pst_netbuff);

    if (uc_datatype != MAC_DATA_EAPOL) {
        return 0;
    }

    puc_payload = oal_netbuf_payload(pst_netbuff);
    if (puc_payload == OAL_PTR_NULL) {
        return 0;
    }

    return *(oal_uint16 *)(puc_payload + OAL_EAPOL_INFO_POS);
}


oal_uint8 mac_get_eapol_type_etc(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8  uc_datatype;
    oal_uint8 *puc_payload = OAL_PTR_NULL;

    uc_datatype = mac_get_data_type_etc(pst_netbuff);

    if (uc_datatype != MAC_DATA_EAPOL) {
        return 0;
    }

    puc_payload = oal_netbuf_payload(pst_netbuff);
    if (puc_payload == OAL_PTR_NULL) {
        return 0;
    }

    return *(puc_payload + OAL_EAPOL_TYPE_POS);
}
#ifdef _PRE_DEBUG_MODE

pkt_trace_type_enum_uint8 mac_pkt_should_trace(oal_uint8 *puc_frame_hdr, mac_netbuff_payload_type uc_hdr_type)
{
    oal_uint8                    uc_data_type;
    pkt_trace_type_enum_uint8    en_trace_data_type = PKT_TRACE_BUTT;
    oal_uint16                   us_ether_type;
    oal_uint8                   *puc_frame_body = OAL_PTR_NULL;
    oal_ip_header_stru          *pst_ip = OAL_PTR_NULL;
    oal_uint8                   *puc_icmp_body = OAL_PTR_NULL;

    // ʶ���DHCP/ECHO/EAPOL/ARP
    uc_data_type = mac_get_data_type_from_8023_etc(puc_frame_hdr, uc_hdr_type);

    if (uc_data_type == MAC_DATA_DHCP) {
        en_trace_data_type = PKT_TRACE_DATA_DHCP;
    } else if (uc_data_type == MAC_DATA_ARP_REQ) {
        en_trace_data_type = PKT_TRACE_DATA_ARP_REQ;
    } else if (uc_data_type == MAC_DATA_ARP_RSP) {
        en_trace_data_type = PKT_TRACE_DATA_ARP_RSP;
    } else if (uc_data_type == MAC_DATA_EAPOL) {
        en_trace_data_type = PKT_TRACE_DATA_EAPOL;
    } else {
        if (uc_hdr_type == MAC_NETBUFF_PAYLOAD_ETH) {
            us_ether_type = ((mac_ether_header_stru *)puc_frame_hdr)->us_ether_type;
            puc_frame_body = puc_frame_hdr + (oal_uint16)OAL_SIZEOF(mac_ether_header_stru);
        } else if (uc_hdr_type == MAC_NETBUFF_PAYLOAD_SNAP) {
            us_ether_type = ((mac_llc_snap_stru *)puc_frame_hdr)->us_ether_type;
            puc_frame_body = puc_frame_hdr + (oal_uint16)OAL_SIZEOF(mac_llc_snap_stru);
        } else {
            return en_trace_data_type;
        }

        /* lint -e778 */
        if (us_ether_type == OAL_HOST2NET_SHORT(ETHER_TYPE_IP)) {
            pst_ip = (oal_ip_header_stru *)puc_frame_body; /* ƫ��һ����̫��ͷ��ȡipͷ */

            if (pst_ip->uc_protocol == MAC_ICMP_PROTOCAL) {  // �ж�ΪICMP����֮�󣬽���ɸѡ��ICMP REQ��ICMP REPLY
                puc_icmp_body = puc_frame_body + (oal_uint16)OAL_SIZEOF(oal_ip_header_stru);
                if (*puc_icmp_body == 0 || *puc_icmp_body == 8) {
                    en_trace_data_type = PKT_TRACE_DATA_ICMP;
                }
            }
        }
    }

    return en_trace_data_type;
}


pkt_trace_type_enum_uint8 wifi_pkt_should_trace(oal_netbuf_stru *pst_netbuff, oal_uint16 us_mac_hdr_len)
{
    pkt_trace_type_enum_uint8    en_trace_data_type = PKT_TRACE_BUTT;
    mac_llc_snap_stru           *pst_snap = OAL_PTR_NULL;
    mac_ieee80211_frame_stru    *pst_mac_header;

    pst_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuff);
    if (pst_mac_header->st_frame_control.bit_type == WLAN_MANAGEMENT) {
        if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_ASSOC_REQ) {
            en_trace_data_type = PKT_TRACE_MGMT_ASSOC_REQ;
        } else if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_ASSOC_RSP) {
            en_trace_data_type = PKT_TRACE_MGMT_ASSOC_RSP;
        } else if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_REASSOC_REQ) {
            en_trace_data_type = PKT_TRACE_MGMT_REASSOC_REQ;
        } else if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_REASSOC_RSP) {
            en_trace_data_type = PKT_TRACE_MGMT_REASSOC_RSP;
        } else if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_DISASOC) {
            en_trace_data_type = PKT_TRACE_MGMT_DISASOC;
        } else if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_AUTH) {
            en_trace_data_type = PKT_TRACE_MGMT_AUTH;
        } else if (pst_mac_header->st_frame_control.bit_sub_type == WLAN_DEAUTH) {
            en_trace_data_type = PKT_TRACE_MGMT_DEAUTH;
        }
    } else if (pst_mac_header->st_frame_control.bit_type == WLAN_DATA_BASICTYPE &&
               pst_mac_header->st_frame_control.bit_sub_type != WLAN_NULL_FRAME) {
        pst_snap = (mac_llc_snap_stru *)(OAL_NETBUF_DATA(pst_netbuff) + us_mac_hdr_len);

        en_trace_data_type = mac_pkt_should_trace((oal_uint8 *)pst_snap, MAC_NETBUFF_PAYLOAD_SNAP);
    }

    return en_trace_data_type;
}
#endif

/*lint -e19*/
oal_module_symbol(mac_is_dhcp_port_etc);
oal_module_symbol(mac_is_dhcp6_etc);
oal_module_symbol(mac_is_nd_etc);
oal_module_symbol(mac_get_data_type_from_8023_etc);
oal_module_symbol(mac_get_data_type_etc);
#ifdef _PRE_DEBUG_MODE
oal_module_symbol(mac_pkt_should_trace);
oal_module_symbol(wifi_pkt_should_trace);
#endif /*lint +e19*/
