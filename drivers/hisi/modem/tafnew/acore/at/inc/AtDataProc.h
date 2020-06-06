/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/


#ifndef __AT_DATA_PROC_H__
#define __AT_DATA_PROC_H__


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "mdrv.h"
#include "AtCtx.h"
#include "AtInputProc.h"
#include "AtNdisInterface.h"
#include "AtRnicInterface.h"
#include "AdsInterface.h"
#include "AtPppInterface.h"
#include "FcInterface.h"
#include "PppInterface.h"
#include "PsIfaceGlobalDef.h"
#include "TafPsApi.h"
#include "TafIfaceApi.h"
#include "AtCmdMsgProc.h"
#include "AtInternalMsg.h"
#include "acore_nv_stru_gucttf.h"
#include "dsm_ndis_pif.h"
#include "mdrv_eipf.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/* ASN�Ĳ��ֽṹ��δ���ֽڶ��뵼��LLT���̳���RUNTIME����, �·��ĺ�������ε�һ��������runtime���� */
#if defined(VOS_WIN32) && (VOS_OS_VER == VOS_WIN32) && (defined(__clang__) || defined(__GNUC__))
#define ATTRIBUTE_NO_SANITIZE_RUNTIME __attribute__((no_sanitize_undefined))
#else
#define ATTRIBUTE_NO_SANITIZE_RUNTIME
#endif


/*lint -e778 */
/*lint -e572 */
#ifndef VOS_NTOHL                   /* ��С�ֽ���ת��*/
#if VOS_BYTE_ORDER==VOS_BIG_ENDIAN
#define VOS_NTOHL(x)    (x)
#define VOS_HTONL(x)    (x)
#define VOS_NTOHS(x)    (x)
#define VOS_HTONS(x)    (x)
#else
#define VOS_NTOHL(x)    ((((x) & 0x000000ffU) << 24) | \
             (((x) & 0x0000ff00U) <<  8) | \
             (((x) & 0x00ff0000U) >>  8) | \
             (((x) & 0xff000000U) >> 24))

#define VOS_HTONL(x)    ((((x) & 0x000000ffU) << 24) | \
             (((x) & 0x0000ff00U) <<  8) | \
             (((x) & 0x00ff0000U) >>  8) | \
             (((x) & 0xff000000U) >> 24))

#define VOS_NTOHS(x)    ((((x) & 0x00ff) << 8) | \
             (((x) & 0xff00) >> 8))

#define VOS_HTONS(x)    ((((x) & 0x00ff) << 8) | \
             (((x) & 0xff00) >> 8))
#endif  /* _BYTE_ORDER==_LITTLE_ENDIAN */
#endif
/*lint -e572 */
/*lint -e778 */

#define AT_MAX_IPV6_DNS_LEN             16

#define AT_MAX_IPV6_STR_DOT_NUM         3
#define AT_MAX_IPV4V6_STR_COLON_NUM     6
#define AT_MAX_IPV6_STR_COLON_NUM       7

#define AT_IPV6_STR_MAX_TOKENS          16                            /* IPV6�ַ�����ʽʹ�õķָ������������ */
#define AT_IPV4_STR_DELIMITER           '.'                             /* RFCЭ��ʹ�õ�IPV4�ı���﷽ʽʹ�õķָ��� */
#define AT_IPV6_STR_DELIMITER           ':'                             /* RFC2373ʹ�õ�IPV6�ı���﷽ʽʹ�õķָ��� */

#define AT_GetIpv6Capability()          (AT_GetCommPsCtxAddr()->ucIpv6Capability)

#define AT_PS_GET_SHARE_PDP_FLG()       (AT_GetCommPsCtxAddr()->ucSharePdpFlag)

/* ��ȡAPP�ͻ���ID */
#define AT_APP_GET_CLIENT_ID()          (gastAtClientTab[AT_CLIENT_TAB_APP_INDEX].usClientId)

/*----------------------------------------------------------------------
   ^NDISSTAT: <stat>[,<err_code>[,<wx_stat>[,<PDP_type]]]
   <err_code> value defined as follows:
   0      - Unknown error/Unspecified error
   others - (E)SM Cause
            SM Cause is defined in TS 24.008 section 10.5.6.6
            ESM Cause is defined in TS 24.301 section 9.9.4.4
*---------------------------------------------------------------------*/
#define AT_NDISSTAT_ERR_UNKNOWN         0

/* ��ȡNDIS�ͻ���ID */
#define AT_NDIS_GET_CLIENT_ID()         (gastAtClientTab[AT_CLIENT_TAB_NDIS_INDEX].usClientId)

#define AT_APS_IP6_ADDR_PREFIX_BYTE_LEN 8

#define AT_MAC_LEN                      6
#define AT_IPV6_ADDR_PREFIX_BYTE_LEN    8
#define AT_IPV6_ADDR_PREFIX_BIT_LEN     64

#define AT_REG_FC                       1
#define AT_DEREG_FC                     0

/* QOS_TRAFFIC_CLASS */
#define AT_QOS_TRAFFIC_CLASS_SUBSCRIBE      0
#define AT_QOS_TRAFFIC_CLASS_CONVERSATIONAL 1
#define AT_QOS_TRAFFIC_CLASS_STREAMING      2
#define AT_QOS_TRAFFIC_CLASS_INTERACTIVE    3
#define AT_QOS_TRAFFIC_CLASS_BACKGROUND     4

#define AT_IPV6_STR_RFC2373_TOKENS      8                 /* �ָ�����ǵ������� */

#define AT_IPPROTO_UDP                  0x11              /* IPͷ����UDPЭ���*/
#define AT_IP_VERSION                   4                 /* IPͷ����IP V4�汾�� */
#define AT_IP_DEF_TTL                   0xFF              /* IPͷ����IP TTLȱʡֵ */
#define AT_IP_RAND_ID                   61468             /* IPͷ��IDֵ�����ȡ*/
#define AT_IP_HDR_LEN                   20                /* IP ͷ������ */
#define AT_UDP_HDR_LEN                  8                 /* UDP ͷ������ */

/* IPV6��ַ��8���ֽ�ȫ�㣬����Ϊ����Ч�� */
#define AT_PS_IS_IPV6_ADDR_IID_VALID(aucIpv6Addr)\
            !(((aucIpv6Addr[8]) == 0x00) && ((aucIpv6Addr[9]) == 0x00)\
             && ((aucIpv6Addr[10]) == 0x00) && ((aucIpv6Addr[11]) == 0x00)\
             && ((aucIpv6Addr[12]) == 0x00) && ((aucIpv6Addr[13]) == 0x00)\
             && ((aucIpv6Addr[14]) == 0x00) && ((aucIpv6Addr[15]) == 0x00))

#define AT_PS_INVALID_RMNET_ID          0xFFFF
#define AT_PS_INVALID_IFACE_ID          0xFF

#define AT_PS_IS_PDP_TYPE_SUPPORT(pdptype)  \
            ( (TAF_PDP_IPV4 == (pdptype))   \
             || (TAF_PDP_IPV6 == (pdptype)) \
             || (TAF_PDP_IPV4V6 == (pdptype)))

#define AT_PS_IS_RABID_VALID(ucRabId)\
    (((ucRabId) >= AT_PS_MIN_RABID) && ((ucRabId) <= AT_PS_MAX_RABID))

#define AT_PS_GET_CURR_CMD_OPT(index)   (gastAtClientTab[index].CmdCurrentOpt)
#define AT_PS_GET_CURR_DATA_MODE(index) (gastAtClientTab[index].DataMode)
#define AT_PS_GET_PPPID(index)          (gastAtClientTab[index].usPppId)

#define AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR()    (g_astAtChdataRnicRmNetIdTab)
#define AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE()   (AT_ARRAY_SIZE(g_astAtChdataRnicRmNetIdTab))

#define AT_PS_RMNET_IFACE_ID_TBL_PTR()              (g_astAtPsIfaceIdRmNetIdTab)
#define AT_PS_RMNET_IFACE_ID_TBL_SIZE()             (AT_ARRAY_SIZE(g_astAtPsIfaceIdRmNetIdTab))

#define AT_PS_GET_FC_IFACE_ID_TBL_PTR()             (g_astAtPsFcIfaceIdTab)
#define AT_PS_GET_FC_IFACE_ID_TBL_SIZE()            (AT_ARRAY_SIZE(g_astAtPsFcIfaceIdTab))

#define AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_PTR()     (g_astAtRptIfaceResultTab)
#define AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_SIZE()    (AT_ARRAY_SIZE(g_astAtRptIfaceResultTab))

#define AT_PS_DIAL_RAT_TYPE_NO_ASTRICT              0
#define AT_PS_DIAL_RAT_TYPE_1X_OR_HRPD              24
#define AT_PS_DIAL_RAT_TYPE_LTE_OR_EHRPD            36

#define IMS_PCSCF_ADDR_PORT_MAX         65535

#define AT_BUILD_EXRABID(i,j)           ((((i) << 6) & 0xC0) | ((j) & 0x3F))
#define AT_GET_RABID_FROM_EXRABID(i)    ((i) & 0x3F)

#define AT_IPV6_ADDR_MASK_FORMAT_STR_LEN        256
#define AT_IPV6_ADDR_DEC_FORMAT_STR_LEN         128
#define AT_IPV6_ADDR_COLON_FORMAT_STR_LEN       40
#define AT_IPV6_ADDR_DEC_TOKEN_NUM              16
#define AT_IPV6_ADDR_HEX_TOKEN_NUM              8
#define AT_IP_STR_DOT_DELIMITER                 '.'
#define AT_IP_STR_COLON_DELIMITER               ':'

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#define AT_GET_IFACE_FC_HEAD_BY_MODEMID(usModemId)  \
            ((usModemId == MODEM_ID_0) ? EIPF_MODEM0_ULFC : \
                ((usModemId == MODEM_ID_1) ? EIPF_MODEM1_ULFC : EIPF_MODEM2_ULFC))
#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum AT_PDP_STATUS_ENUM
{
    AT_PDP_STATUS_DEACT                    = 0,
    AT_PDP_STATUS_ACT                      = 1,
    AT_PDP_STATUS_BUTT
};
typedef VOS_UINT32 AT_PDP_STATUS_ENUM_UINT32;

/*****************************************************************************
 �ṹ����   : AT_IPV6_STR_TYPE_ENUM
 Э����   :
 ASN.1 ���� :
 �ṹ˵��   : IPV6 String��ʽö��
              HEXΪRFC2373Ҫ��ʹ��':'��Ϊ�ָ���
              DEXΪRFCҪ��ʹ��'.'��Ϊ�ָ���
*****************************************************************************/
enum AT_IPV6_STR_TYPE_ENUM
{
    AT_IPV6_STR_TYPE_HEX                = 0x01,
    AT_IPV6_STR_TYPE_DEC                = 0x02,

    AT_IPV6_STR_TYPE_BUTT               = 0xFF
};
typedef VOS_UINT8 AT_IPV6_STR_TYPE_ENUM_UINT8;

/*****************************************************************************
 �ṹ����   : AT_IP_ADDR_TYPE_ENUM
 Э����   :
 ASN.1 ���� :
 �ṹ˵��   : ������SOURCE����LOCAL��IP ADDR
*****************************************************************************/
enum AT_IP_ADDR_TYPE_ENUM
{
    AT_IP_ADDR_TYPE_SOURCE              = 0x01,
    AT_IP_ADDR_TYPE_LOCAL               = 0x02,

    AT_IP_ADDR_TYPE_BUTT                = 0xFF
};
typedef VOS_UINT8 AT_IP_ADDR_TYPE_ENUM_UINT8;

/*****************************************************************************
 ö������   : AT_HILINK_MODE_ENUM
 Э����   :
 ASN.1 ���� :
 ö��˵��   : HiLinkģʽ
*****************************************************************************/
enum AT_HILINK_MODE_ENUM
{
    AT_HILINK_NORMAL_MODE,
    AT_HILINK_GATEWAY_MODE,
    AT_HILINK_MODE_BUTT
};
typedef VOS_UINT8 AT_HILINK_MODE_ENUM_U8;

/* APP����״̬*/
/*****************************************************************************
 ö������   : AT_APP_CONN_STATE_ENUM
 Э����   :
 ASN.1 ���� :
 ö��˵��   : APP����״̬
*****************************************************************************/
enum AT_APP_CONN_STATE_ENUM
{
    AT_APP_DIALING,                     /*0: indicates is connecting*/
    AT_APP_DIALED,                      /*1: indicates connected*/
    AT_APP_UNDIALED,                    /*2: indicates disconnected*/
    AT_APP_BUTT
};
typedef VOS_UINT32 AT_APP_CONN_STATE_ENUM_U32;

/*****************************************************************************
 PPP���ź������������ʾ���������������:
 1.2Gģʽ�£�����ݵ�ǰפ����С��ģʽ��GSM/GPRS/EDGE�������������ݵ���ʾ,��Ӧ����:
     GSM          - 9600
     GPRS         - 85600
     GPRS Class33 - 107800
     EDGE         - 236800
     EDGE Class33 - 296000
     Ĭ�� -
 2.3gģʽ�£������RRC�汾��HSDPA��category��Ϣ�������������ݵ���ʾ����Ӧ����:
     RRC�汾ΪR99   - 384000
     RRC�汾Ϊ��R99 - �ж�HSDPA��category��Ϣ
                   6  - 3600000
                   8  - 7200000
                   9  - 10200000
                   10 - 14400000
                   12 - 1800000
                   14 - 21600000
                   18 - 28800000
                   24 - 43200000
                   26 - 57600000
                   28 - 86400000
     ����չ��category�Ļ���Ĭ�� - 21600000
     ����չ��category�Ļ���Ĭ�� - 7200000
*****************************************************************************/
enum PPP_RATE_DISPLAY_ENUM
{
    PPP_RATE_DISPLAY_GSM = 0,
    PPP_RATE_DISPLAY_GPRS,
    PPP_RATE_DISPLAY_GPRS_CALSS33,
    PPP_RATE_DISPLAY_EDGE,
    PPP_RATE_DISPLAY_EDGE_CALSS33,

    PPP_RATE_DISPLAY_R99,
    PPP_RATE_DISPLAY_DPA_CATEGORY_6,
    PPP_RATE_DISPLAY_DPA_CATEGORY_8,
    PPP_RATE_DISPLAY_DPA_CATEGORY_9,
    PPP_RATE_DISPLAY_DPA_CATEGORY_10,
    PPP_RATE_DISPLAY_DPA_CATEGORY_12,
    PPP_RATE_DISPLAY_DPA_CATEGORY_14,
    PPP_RATE_DISPLAY_DPA_CATEGORY_18,
    PPP_RATE_DISPLAY_DPA_CATEGORY_24,
    PPP_RATE_DISPLAY_DPA_CATEGORY_26,
    PPP_RATE_DISPLAY_DPA_CATEGORY_28,

    PPP_RATE_DISPLAY_BUTT
};
typedef VOS_UINT32 PPP_RATE_DISPLAY_ENUM_UINT32;

/*****************************************************************************
 ö����    : AT_CH_DATA_CHANNEL_ENUM
 �ṹ˵��  : AT^CHDATA�������õ�<datachannelid>��ȡֵ
*****************************************************************************/
enum AT_CH_DATA_CHANNEL_ENUM
{
    AT_CH_DATA_CHANNEL_ID_1             = 1,
    AT_CH_DATA_CHANNEL_ID_2,
    AT_CH_DATA_CHANNEL_ID_3,
    AT_CH_DATA_CHANNEL_ID_4,
    AT_CH_DATA_CHANNEL_ID_5,
    AT_CH_DATA_CHANNEL_ID_6,
    AT_CH_DATA_CHANNEL_ID_7,

    AT_CH_DATA_CHANNEL_BUTT
};
typedef VOS_UINT32 AT_CH_DATA_CHANNEL_ENUM_UINT32;

/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/

/*****************************************************************************
  6 STRUCT����
*****************************************************************************/

/* DHCP���ã�ȫ������*/
typedef struct
{
    VOS_UINT32                          ulIPAddr;                               /* IP ��ַ���������*/
    VOS_UINT32                          ulSubNetMask;                           /* ���������ַ������IP��ַ����*/
    VOS_UINT32                          ulGateWay;                              /* ���ص�ַ��Ҳ�Ǳ�DHCP Server�ĵ�ַ*/
    VOS_UINT32                          ulPrimDNS;                              /* �� DNS��ַ���������*/
    VOS_UINT32                          ulSndDNS;                               /* �� DNS��ַ���������*/
}AT_DHCP_CONFIG_STRU;

typedef struct AT_DHCP_PARA
{
    VOS_UINT16                          usClientID;                             /* Client ID*/
    VOS_UINT8                           ucRabID;                                /* Rab ID*/
    VOS_UINT8                           ucCid;                                  /* CID*/
    VOS_UINT32                          ulSpeed;                                /* Um Speed*/
    VOS_UINT32                          ulDLMaxRate;                            /* ���������������*/
    VOS_UINT32                          ulULMaxRate;                            /* ���������������*/
    AT_DHCP_CONFIG_STRU                 stDhcpCfg;
}AT_DHCP_PARA_STRU;

/*****************************************************************************
 �ṹ��    : AT_CLINTID_RABID_MAP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulUsed;   /* ָ��FCID��Ӧ�Ľ���Ƿ���Ч��VOS_TRUE:��Ч��VOS_FALSE:��Ч */
    VOS_UINT32                          ulRabIdMask;
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_PRI_ENUM_UINT8                   enFcPri;
    VOS_UINT8                           aucReserve[1];                          /* ���� */
} AT_FCID_MAP_STRU;

/*****************************************************************************
 �ṹ��     : AT_IPHDR_STRU
 Э����   :
 ASN.1����  :
 �ṹ˵��   : IPv4 packet header �ṹ
*****************************************************************************/
typedef struct
{
#if (VOS_LITTLE_ENDIAN == VOS_BYTE_ORDER)                   /* С���ֽ���*/
    VOS_UINT8                           ucIpHdrLen  :4;     /* IPͷ������ */
    VOS_UINT8                           ucIpVer     :4;     /* IP�汾��*/
#elif (VOS_BIG_ENDIAN == VOS_BYTE_ORDER)                    /* ����ֽ���*/
    VOS_UINT8                           ucIpVer     :4;     /* IP�汾��*/
    VOS_UINT8                           ucIpHdrLen  :4;     /* IPͷ������ */
#else
#error  "Please fix Macro VOS_BYTE_ORDER"                   /* VOS_BYTE_ORDERδ����*/
#endif
    VOS_UINT8                           ucServiceType;      /* IP TOS�ֶ�*/
    VOS_UINT16                          usTotalLen;         /* IP���ݰ��ܳ���*/
    VOS_UINT16                          usIdentification;   /* IP���ݰ�ID*/
    VOS_UINT16                          usOffset;           /* IP��Ƭƫ����*/
    VOS_UINT8                           ucTTL;              /* IPTTL*/
    VOS_UINT8                           ucProtocol;         /* IP�����غɲ���Э��*/
    VOS_UINT16                          usCheckSum;         /* IPͷ��У���*/
    VOS_UINT32                          ulSrcAddr;          /* ԴIP��ַ*/
    VOS_UINT32                          ulDstAddr;          /* Ŀ��IP��ַ*/
} AT_IPHDR_STRU;

/*****************************************************************************
 �ṹ��     : AT_UDP_HDR_STRU
 Э����   :
 ASN.1����  :
 �ṹ˵��   : UDP header �ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usSrcPort;          /* Դ�˿� */
    VOS_UINT16                          usDstPort;          /* Ŀ�Ķ˿� */
    VOS_UINT16                          usLen;              /* UDP������ */
    VOS_UINT16                          usChecksum;         /* UDPУ��� */
} AT_UDP_HDR_STRU;

/*****************************************************************************
 �ṹ��     : AT_UDP_PACKET_FORMAT_STRU
 Э����   :
 ASN.1����  :
 �ṹ˵��   : UDP packet �ṹ
*****************************************************************************/
typedef struct
{

    AT_IPHDR_STRU                       stIpHdr;            /* IPͷ */
    AT_UDP_HDR_STRU                     stUdpHdr;           /* UDPͷ */
    VOS_UINT32                          ulBody;
} AT_UDP_PACKET_FORMAT_STRU;

/*****************************************************************************
 �ṹ��    : AT_CHDATA_RNIC_RMNET_ID_STRU
 �ṹ˵��  : AT^CHDATA��RNIC����ӳ���ϵ�Ľṹ
*****************************************************************************/
typedef struct
{
    AT_CH_DATA_CHANNEL_ENUM_UINT32  enChdataValue;
    RNIC_DEV_ID_ENUM_UINT8          enRnicRmNetId;
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    VOS_UINT8                       aucReserved[2];

}AT_CHDATA_RNIC_RMNET_ID_STRU;

/*****************************************************************************
 �ṹ��    : AT_PS_RMNET_IFACE_ID_STRU
 �ṹ˵��  : IFACEID��RMNETIDӳ���ϵ�Ľṹ
*****************************************************************************/
typedef struct
{
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    RNIC_DEV_ID_ENUM_UINT8          enRmNetId;
    VOS_UINT8                       ucReserved1;
    VOS_UINT8                       ucReserved2;
}AT_PS_RMNET_IFACE_ID_STRU;

/*****************************************************************************
 �ṹ��    : AT_PS_FC_IFACE_ID_STRU
 �ṹ˵��  : FC��IFACE IDӳ���ϵ�Ľṹ
*****************************************************************************/
typedef struct
{
    FC_ID_ENUM_UINT8                enFcId;
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    VOS_UINT8                       ucReserved1;
    VOS_UINT8                       ucReserved2;
}AT_PS_FC_IFACE_ID_STRU;

/*****************************************************************************
 �ṹ��    : AT_DISPLAY_RATE_STRU
 �ṹ˵��  : ������ʾ��Ϊ���к���������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucDlSpeed[AT_AP_SPEED_STRLEN + 1];
    VOS_UINT8                           ucUlSpeed[AT_AP_SPEED_STRLEN + 1];
    VOS_UINT8                           aucReserved[2];
}AT_DISPLAY_RATE_STRU;

typedef struct
{
    VOS_UINT32                          ulCmdCurrentOpt;
    MN_PS_EVT_FUNC                      pEvtFunc;
} AT_PS_DYNAMIC_INFO_REPORT_FUNC_TBL_STRU;

/*****************************************************************************
  7 UNION����
*****************************************************************************/

/*****************************************************************************
  8 OTHERS����
*****************************************************************************/

/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/

/*��¼���翨�汾���ϵ絽���ųɹ�����ʱ�䣬��λ�� */
extern VOS_UINT32                       g_ulLcStartTime;

extern AT_HILINK_MODE_ENUM_U8           g_enHiLinkMode;

extern AT_FCID_MAP_STRU                 g_stFcIdMaptoFcPri[];

extern CONST AT_CHDATA_RNIC_RMNET_ID_STRU     g_astAtChdataRnicRmNetIdTab[];


/*****************************************************************************
  10 ��������
*****************************************************************************/

/*****************************************************************************
 �� �� ��  : AT_GetDhcpPara
 ��������  : ��ȡDHCP����(DHCP����Ϊ������)
 �������  : pstConfig                  - DHCP����(������)
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_VOID AT_GetDhcpPara(
    AT_DHCP_PARA_STRU                  *pstConfig,
    TAF_IFACE_IPV4_DHCP_PARAM_STRU     *pstIpv4Dhcp
);

/******************************************************************************
 �� �� ��  : AT_GetDisplayRate
 ��������  : ��ȡ�տ����۴�����NAS��ȡ���ҽ��ַ�����ʽתΪ����
 �������  : *pstSpeed     ----    ���������ʽṹ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :
******************************************************************************/
VOS_UINT32 AT_GetDisplayRate(
    VOS_UINT16                          usClientId,
    AT_DISPLAY_RATE_STRU               *pstSpeed
);

/******************************************************************************
 Function:      AT_CtrlGetPDPAuthType
 Description:    ��ȡPC���õ�PDP�������ж�Ӧ���͵�����
 Calls:
 Data Accessed:
 Data Updated:
 Input:
                 usTotalLen     PDP�������ڴ泤��
 Output:
 Return:        0   no auth
                1   pap
                2   chap
******************************************************************************/
PPP_AUTH_TYPE_ENUM_UINT8 AT_CtrlGetPDPAuthType(
    VOS_UINT32                          Value,
    VOS_UINT16                          usTotalLen
);

TAF_PDP_AUTH_TYPE_ENUM_UINT8 AT_ClGetPdpAuthType(
    VOS_UINT32                          Value,
    VOS_UINT16                          usTotalLen
);

/******************************************************************************
 ��������: AT_Ipv4AddrAtoi
 ��������: ��IPV4��ַ���ַ�����ʽת�������ָ�ʽ
******************************************************************************/
VOS_UINT32 AT_Ipv4AddrAtoi(
    VOS_CHAR *pcString, VOS_UINT8 *pucNumber, VOS_UINT32 ulNumBufLen);

/******************************************************************************
 ��������: AT_Ipv4AddrItoa
 ��������: ��IPV4��ַ�����ָ�ʽת�����ַ�����ʽ
******************************************************************************/
VOS_UINT32 AT_Ipv4AddrItoa(VOS_CHAR *pcString, VOS_UINT32 ulStrLen, VOS_UINT8 *pucNumber);

/*****************************************************************************
 �� �� ��  : AT_Ipv4Addr2Str
 ��������  : IPV4���͵ĵ�ַת��Ϊ�ַ�������
 �������  : pucNumber      - IPV4��ַ
 �������  : pcString       - �ַ���
 �� �� ֵ  : VOS_UINT32

*****************************************************************************/
VOS_UINT32 AT_Ipv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT32                          ulStrBuflen,
    VOS_UINT8                          *pucNumber,
    VOS_UINT32                          ulAddrNumCnt
);

VOS_UINT32 AT_Ipv6AddrAtoi(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber,
    VOS_UINT32                          ulIpv6BufLen
);

VOS_VOID AT_PcscfIpv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT32                          ulStrBufLen,
    VOS_UINT8                          *pucNumber
);

VOS_UINT32  AT_PortAtoI(
    VOS_CHAR                         *pcString,
    VOS_UINT32                       *pulValue
);

VOS_UINT32  AT_Ipv6PcscfDataToAddr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr,
    VOS_UINT32                         *pulPortExistFlg,
    VOS_UINT32                         *pulPortNum

);

/*****************************************************************************
 �� �� ��  : AT_Ipv6AddrToStr
 ��������  : ��IPV6��ַ��ʽת��Ϊ�ַ�����ʽ
 �������  : aucIpAddr[]    - IPV6��ַ(Э���ʽ)
             enIpStrType    - IPV6�ַ�����ʽ�������
 �������  : aucAddrStr[]   - IPV6��ַ(�ַ�����ʽ)
 �� �� ֵ  : VOS_OK         - ת���ɹ�
             VOS_ERR        - ת��ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_Ipv6AddrToStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    AT_IPV6_STR_TYPE_ENUM_UINT8         enIpStrType
);

/*****************************************************************************
 �� �� ��  : AT_Itoa
 ��������  : ����ת������(10��16), ������ת��ΪASCII��, �����������ַ���
 �������  : usValue    - ��ת��ΪASCII�������
             pcStr      - ���������ַ���
             usRadix    - ת������
 �������  : ��
 �� �� ֵ  : VOS_CHAR*
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_CHAR* AT_Itoa(
    VOS_UINT16                          usValue,
    VOS_CHAR                           *pcStr,
    VOS_UINT16                          usRadix,
    VOS_UINT32                          ulLength
);
/******************************************************************************
 �� �� ��  : AT_AtoI
 ��������  : �ַ���ת����
 �������  : pString �ַ���
 �������  : ��
 �� �� ֵ  : ���� IP��ַ
 ���ú���  :
 ��������  :
******************************************************************************/
VOS_UINT64  AT_AtoI(
    VOS_CHAR                           *pString
);


/******************************************************************************
 �� �� ��  : AT_AtoI
 ��������  : �ַ���ת����(���Դ�����ת��)
 �������  : pString �ַ���
 �������  : ��
 �� �� ֵ  : ���� IP��ַ
 ���ú���  :
 ��������  :
******************************************************************************/
VOS_INT32  AT_AtoInt(
    VOS_CHAR                           *pString,
    VOS_INT32                          *pOut
);

/*****************************************************************************
 �� �� ��  : AT_ConvertIpv6AddrToCompressedStr
 ��������  : ��IPV6��ַ��ʽת��Ϊ�ַ���ѹ����ʽ
 �������  : aucIpAddr[]    - IPV6��ַ(Э���ʽ)
             ucTokensNum    - ��ַ�θ���
 �������  : aucAddrStr[]   - IPV6��ַ(�ַ�����ʽ)
 �� �� ֵ  : VOS_OK         - ת���ɹ�
             VOS_ERR        - ת��ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_ConvertIpv6AddrToCompressedStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    VOS_UINT8                           ucTokensNum
);

/*****************************************************************************
 �� �� ��  : AT_BuildUdpHdr
 ��������  : ����IP��UDPͷ����Ϣ(���ڹ������ʹ�õ�UDP��)
 �������  : pstUdpPkt  - UDP����Ϣ
             usLen      - UDP������
             ulSrcAddr  - ԴIP��ַ
             ulDstAddr  - Ŀ��IP��ַ
             usSrcPort  - Դ�˿ں�
             usDstPort  - Ŀ��˿ں�
 �������  : ��
 �� �� ֵ  : VOS_OK     - ����ɹ�
             VOS_ERR    - ����ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_BuildUdpHdr(
    AT_UDP_PACKET_FORMAT_STRU          *pstUdpPkt,
    VOS_UINT16                          usLen,
    VOS_UINT32                          ulSrcAddr,
    VOS_UINT32                          ulDstAddr,
    VOS_UINT16                          usSrcPort,
    VOS_UINT16                          usDstPort
);


/*****************************************************************************
 �� �� ��  : AT_NidsGet3gppSmCauseByPsCause
 ��������  : ��PS����д�����ת����3GPPЭ�鶨���(E)SM Cause, ��3GPPЭ�鶨��
             �Ĵ�����ͳһת����0(Unknown error/Unspecified error)
 �������  : enCause - PS����д�����
 �������  : ��
 �� �� ֵ  : VOS_UINT16
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_Get3gppSmCauseByPsCause(
    TAF_PS_CAUSE_ENUM_UINT32            enCause
);

VOS_VOID AT_NDIS_ConnStatusChgProc(NCM_IOCTL_CONNECT_STUS_E enStatus);


VOS_VOID  AT_ModemPsRspPdpActEvtRejProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pEvent
);



VOS_VOID  AT_ModemPsRspPdpActEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pEvent
);


VOS_VOID  AT_ModemPsRspPdpDeactEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pEvent
);

/*****************************************************************************
 �� �� ��  : AT_MODEM_ProcCallEndedEvent
 ��������  : ����PS_CALL_END_CNF�¼�
 �������  : ucIndex  - �˿�����
             pstEvent - ID_EVT_TAF_PS_CALL_END_CNF�¼�ָ��
 �������  : ��
 �� �� ֵ  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_MODEM_ProcCallEndCnfEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_END_CNF_STRU           *pstEvent
);


/*****************************************************************************
 �� �� ��  : AT_MODEM_HangupCall
 ��������  : �Ҷ�PPP��������
 �������  : ucIndex - �˿�����
 �������  : ��
 �� �� ֵ  : AT_XXX  - ATC������
*****************************************************************************/
VOS_UINT32 AT_MODEM_HangupCall(VOS_UINT8 ucIndex);


VOS_VOID  AT_AnswerPdpActInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
);


VOS_VOID  AT_ModemPsRspPdpDeactivatedEvtProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU *pEvent
);

/*****************************************************************************
 �� �� ��  : AT_DeRegModemPsDataFCPoint
 ��������  : ȥע��Modem�˿����ص㡣
 �������  : MN_CLIENT_ID_T                      usClientID,
             VOS_UINT8                           ucRabId
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_DeRegModemPsDataFCPoint(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucRabId
);

#if( FEATURE_ON == FEATURE_CSD )
/*****************************************************************************
 �� �� ��  : AT_RegModemVideoPhoneFCPoint
 ��������  : ע��Modem�˿�CST���ص㡣
 �������  : FC_ID_ENUM_UINT8                    enFcId
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_RegModemVideoPhoneFCPoint(
    VOS_UINT8                           ucIndex,
    FC_ID_ENUM_UINT8                    enFcId
);

/*****************************************************************************
 �� �� ��  : AT_DeRegModemVideoPhoneFCPoint
 ��������  : ȥע��Modem�˿�CST���ص㡣
 �������  :
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_DeRegModemVideoPhoneFCPoint(VOS_UINT8 ucIndex);
#endif


VOS_VOID AT_NotifyFcWhenPdpModify(
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId
);

#if (FEATURE_LTE == FEATURE_ON)
/* ATģ���FTM ģ�鷢����Ϣ */
VOS_UINT32 atSendFtmDataMsg(VOS_UINT32 TaskId, VOS_UINT32 MsgId, VOS_UINT32 ulClientId, const VOS_VOID *pData, VOS_UINT32 uLen);
#endif

/*****************************************************************************
 �� �� ��  : AT_PS_AddIpAddrMap
 ��������  : ��ӳ���IP��RABID��ӳ��
 �������  : usClientId - �ͻ���ID
             pstEvent   - PS������¼�
 �������  : ��
 �� �� ֵ  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_PS_AddIpAddrMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
);


/*****************************************************************************
 �� �� ��  : AT_PS_DeleteIpAddrMap
 ��������  : ɾ������IP��RABID��ӳ��
 �������  : usClientId - �ͻ���ID
             pstEvent   - PS������¼�
 �������  : ��
 �� �� ֵ  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_PS_DeleteIpAddrMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
VOS_UINT32 AT_PS_GetIpAddrByIfaceId(
    const VOS_UINT16                          usClientId,
    const VOS_UINT8                           ucIfaceId
);
#else
/*****************************************************************************
 �� �� ��  : AT_PS_GetIpAddrByRabId
 ��������  : ����RABID��ȡ����IP��ַ
 �������  : usClientId - �ͻ���ID
             ucRabId    - RABID [5,15]
 �������  : ��
 �� �� ֵ  : ulIpAddr   - IP��ַ(������)
*****************************************************************************/
VOS_UINT32 AT_PS_GetIpAddrByRabId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucRabId
);
#endif

VOS_VOID AT_PS_SetPsCallErrCause(
    VOS_UINT16                          usClientId,
    TAF_PS_CAUSE_ENUM_UINT32            enPsErrCause
);

TAF_PS_CAUSE_ENUM_UINT32 AT_PS_GetPsCallErrCause(
    VOS_UINT16                          usClientId
);

VOS_UINT32 AT_PS_GetChDataValueFromRnicRmNetId(
    RNIC_DEV_ID_ENUM_UINT8              enRnicRmNetId,
    AT_CH_DATA_CHANNEL_ENUM_UINT32     *penDataChannelId
);

CONST AT_CHDATA_RNIC_RMNET_ID_STRU *AT_PS_GetChDataCfgByChannelId(
    VOS_UINT8                           ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId
);

TAF_IFACE_USER_TYPE_ENUM_U8 AT_PS_GetUserType(const VOS_UINT8 ucIndex);

VOS_UINT32 AT_PS_IsIpv6CapabilityValid(VOS_UINT8 ucCapability);

VOS_UINT32 AT_CheckIpv6Capability(
    VOS_UINT8                           ucPdpType
);

VOS_UINT8 AT_CalcIpv6PrefixLength(
    VOS_UINT8                           *pucLocalIpv6Mask,
    VOS_UINT32                           ulIpv6MaskLen
);

VOS_VOID AT_GetIpv6MaskByPrefixLength(
    VOS_UINT8                            ucLocalIpv6Prefix,
    VOS_UINT8                           *pucLocalIpv6Mask
);

VOS_UINT32 AT_PS_ProcCallModifyEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent
);

VOS_UINT32 AT_PS_ValidateDialParam(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
);

VOS_UINT32 AT_CheckApnFormat(
    VOS_UINT8                          *pucApn,
    VOS_UINT16                          usApnLen,
    VOS_UINT16                          usClientId
);

VOS_VOID AT_PS_ReportCustomPcoInfo(
    TAF_PS_CUSTOM_PCO_INFO_STRU        *pstPcoCustInfo,
    TAF_PS_PDN_OPERATE_TYPE_ENUM_UINT8  enOperateType,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_CLIENT_TAB_INDEX_UINT8           enPortIndex
);

/*****************************************************************************
 �� �� ��  : AT_PS_BuildExClientId
 ��������  : ������չ��ClientId(ModemId + ClientId)
 �������  : usClientId                 - ClientId
 �������  : ��
 �� �� ֵ  : VOS_UINT16                 - ���ɵ���չClientId
*****************************************************************************/
VOS_UINT16 AT_PS_BuildExClientId(VOS_UINT16 usClientId);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_PS_CheckDialRatType(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucBitRatType
);
#endif

VOS_UINT32 AT_Ipv6AddrMask2FormatString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT8                           aucIpv6Mask[]
);

VOS_VOID AT_PS_ReportImsCtrlMsgu(
    VOS_UINT8                                       ucIndex,
    AT_IMS_CTRL_MSG_RECEIVE_MODULE_ENUM_UINT8       enModule,
    VOS_UINT32                                      ulMsgLen,
    VOS_UINT8                                      *pucDst
);

FC_ID_ENUM_UINT8 AT_PS_GetFcIdByIfaceId(
    const VOS_UINT32                        ulIfaceId
);

AT_PS_DATA_CHANL_CFG_STRU* AT_PS_GetDataChanlCfg(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid
);

VOS_UINT32 AT_PS_BuildIfaceCtrl(
    const VOS_UINT32                    ulModuleId,
    const VOS_UINT16                    usPortClientId,
    const VOS_UINT8                     ucOpId,
    TAF_CTRL_STRU                      *pstCtrl
);

VOS_UINT32 AT_PS_ProcIfaceCmd(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
);

VOS_UINT32 AT_PS_ProcMapconMsg(
    const VOS_UINT16                    usClientId,
    const AT_MAPCON_CTRL_MSG_STRU      *pstAtMapConMsgPara
);

VOS_VOID AT_PS_ProcAppIfaceStatus(
    TAF_IFACE_STATUS_IND_STRU          *pstIfaceStatus
);

VOS_VOID AT_PS_ProcNdisIfaceStatus(
    TAF_IFACE_STATUS_IND_STRU          *pstIfaceStatus
);

VOS_VOID AT_PS_ProcNdisIfaceUpCfg(
    const DSM_NDIS_IFACE_UP_IND_STRU   *pstRcvMsg
);

VOS_VOID AT_PS_ProcNdisIfaceDownCfg(
    const DSM_NDIS_IFACE_DOWN_IND_STRU *pstRcvMsg
);

VOS_VOID AT_PS_ProcNdisConfigIpv6Dns(
    const DSM_NDIS_CONFIG_IPV6_DNS_IND_STRU    *pstRcvMsg
);

VOS_VOID AT_RcvTafIfaceEvt(
    TAF_PS_EVT_STRU                     *pstEvt
);

VOS_UINT32 AT_RcvTafIfaceEvtIfaceUpCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtIfaceDownCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtIfaceStatusInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtDataChannelStateInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtUsbNetOperInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtRegFcInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtDeRegFcInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafPsEvtEpdgCtrluNtf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);

VOS_UINT32 AT_RcvTafIfaceEvtDyamicParaCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_RcvTafIfaceEvtRabInfoInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
);
VOS_UINT32 AT_PS_ReportSetIpv6TempAddrRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportSetDhcpRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryDhcpRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportSetDhcpv6Rst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryDhcpv6Rst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportSetApRaInfoRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryApRaInfoRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportSetApLanAddrRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryApLanAddrRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportSetApConnStRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryApConnStRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryDconnStatRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportQryNdisStatRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_UINT32 AT_PS_ReportTestDconnStatRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
);
VOS_UINT32 AT_PS_ReportSetCgmtuRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
);
VOS_VOID AT_PS_SendNdisIPv4IfaceUpCfgInd(
    const DSM_NDIS_IFACE_UP_IND_STRU   *pstRcvMsg
);

VOS_VOID AT_PS_SendNdisIPv6IfaceUpCfgInd(
    const DSM_NDIS_IFACE_UP_IND_STRU   *pstRcvMsg
);

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __AT_DATA_PROC_H__ */

