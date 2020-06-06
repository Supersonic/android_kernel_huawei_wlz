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
#ifndef _TAF_IFACE_API_H_
#define _TAF_IFACE_API_H_


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "TafPsApi.h"
#include "ImsaWifiInterface.h"
#include "PsIfaceGlobalDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

#pragma pack(4)


/*****************************************************************************
  2 �궨��
*****************************************************************************/

#define TAF_IFACE_MAX_NUM                   (PS_IFACE_ID_BUTT)

/* 1~11 BIT CID��Ϣ */
#define TAF_IFACE_ALL_BIT_NV_CID            (0x0FFE)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum TAF_IFACE_MSG_ID_ENUM
{
    ID_MSG_TAF_IFACE_UP_REQ                                 = TAF_PS_IFACE_ID_BASE + 0x0001,
    ID_MSG_TAF_IFACE_DOWN_REQ                               = TAF_PS_IFACE_ID_BASE + 0x0002,
    ID_MSG_TAF_GET_IFACE_DYNAMIC_PARA_REQ                   = TAF_PS_IFACE_ID_BASE + 0x0003,

    ID_MSG_TAF_IFACE_BUTT
};
typedef VOS_UINT32 TAF_IFACE_MSG_ID_ENUM_UINT32;


enum TAF_IFACE_EVT_ID_ENUM
{
    ID_EVT_TAF_IFACE_UP_CNF                                 = TAF_PS_IFACE_ID_BASE + 0x0081,
    ID_EVT_TAF_IFACE_DOWN_CNF                               = TAF_PS_IFACE_ID_BASE + 0x0082,
    ID_EVT_TAF_IFACE_STATUS_IND                             = TAF_PS_IFACE_ID_BASE + 0x0083,
    ID_EVT_TAF_IFACE_DATA_CHANNEL_STATE_IND                 = TAF_PS_IFACE_ID_BASE + 0x0084,
    ID_EVT_TAF_IFACE_USBNET_OPER_IND                        = TAF_PS_IFACE_ID_BASE + 0x0085,
    ID_EVT_TAF_IFACE_REG_FC_IND                             = TAF_PS_IFACE_ID_BASE + 0x0086,
    ID_EVT_TAF_IFACE_DEREG_FC_IND                           = TAF_PS_IFACE_ID_BASE + 0x0087,
    ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF                   = TAF_PS_IFACE_ID_BASE + 0x0088,
    ID_EVT_TAF_IFACE_RAB_INFO_IND                           = TAF_PS_IFACE_ID_BASE + 0x0089,

    ID_EVT_TAF_IFACE_BUTT
};
typedef VOS_UINT32 TAF_IFACE_EVT_ID_ENUM_UINT32;


enum TAF_IFACE_STATUS_ENUM
{
    TAF_IFACE_STATUS_ACT                = 0x01,
    TAF_IFACE_STATUS_DEACT              = 0x02,

    TAF_IFACE_STATUS_BUTT
};
typedef VOS_UINT8 TAF_IFACE_STATUS_ENUM_UINT8;


enum TAF_IFACE_USER_TYPE_ENUM
{
    TAF_IFACE_USER_TYPE_APP             = 0,
    TAF_IFACE_USER_TYPE_NDIS            = 1,
    TAF_IFACE_USER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_IFACE_USER_TYPE_ENUM_U8;


enum TAF_IFACE_STATE_ENUM
{
    TAF_IFACE_STATE_IDLE                = 0,
    TAF_IFACE_STATE_ACTED               = 1,
    TAF_IFACE_STATE_ACTING              = 2,
    TAF_IFACE_STATE_DEACTING            = 3,
    TAF_IFACE_STATE_BUTT
};
typedef VOS_UINT8 TAF_IFACE_STATE_ENUM_U8;


enum TAF_IFACE_RAB_OPER_TYPE_ENUM
{
    TAF_IFACE_RAB_OPER_ADD              = 0,
    TAF_IFACE_RAB_OPER_DELETE           = 1,
    TAF_IFACE_RAB_OPER_CHANGE           = 2,
    TAF_IFACE_RAB_OPER_BUTT
};
typedef VOS_UINT8 TAF_IFACE_RAB_OPER_TYPE_ENUM_U8;

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/


typedef struct
{
    VOS_UINT32                          bitOpIPv4ValidFlag          : 1;
    VOS_UINT32                          bitOpSpare                  : 31;

    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType;                             /* �û��������� */
    VOS_UINT8                           ucCid;
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;
    VOS_UINT8                           ucApnLen;
    VOS_UINT8                           aucApn[TAF_MAX_APN_LEN + 1];
    VOS_UINT8                           ucUsernameLen;
    VOS_UINT8                           aucUsername[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];
    VOS_UINT8                           ucPasswordLen;
    VOS_UINT8                           aucPassword[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];
    VOS_UINT8                           ucAuthType;
    VOS_UINT8                           ucBitRatType;
    VOS_UINT8                           aucIPv4Addr[TAF_MAX_IPV4_ADDR_STR_LEN];
}TAF_IFACE_DIAL_PARAM_STRU;


typedef struct
{
    TAF_IFACE_DIAL_PARAM_STRU           stUsrDialParam;
    VOS_UINT8                           ucIfaceId;
    VOS_UINT8                           aucReserved[3];
}TAF_IFACE_UP_STRU;


typedef struct
{
    VOS_UINT8                           ucIfaceId;
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause;
    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType;                             /* �û��������� */
    VOS_UINT8                           aucReserved[1];
}TAF_IFACE_DOWN_STRU;


typedef struct
{
    VOS_UINT8                           ucPcscfAddrNum;                         /* IPV4��P-CSCF��ַ��������Ч��Χ[0,8] */
    VOS_UINT8                           aucRsv[3];                              /* ���� */
    VOS_UINT32                          aulPcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_IFACE_IPV4_PCSCF_LIST_STRU;


typedef struct
{
    VOS_UINT32                              bitOpPriDns   : 1;
    VOS_UINT32                              bitOpSecDns   : 1;
    VOS_UINT32                              bitOpSpare    : 30;

    VOS_UINT32                              ulAddr;                             /* IPV4��IP��ַ�������� */
    VOS_UINT32                              ulNetMask;                          /* IPV4�����룬������ */
    VOS_UINT32                              ulGateWay;                          /* IPV4�����ص�ַ�������� */
    VOS_UINT32                              ulPrimDNS;                          /* IPV4����DNS�������� */
    VOS_UINT32                              ulSecDNS;                           /* IPV4�ĸ�DNS�������� */
    TAF_IFACE_IPV4_PCSCF_LIST_STRU          stPcscfList;                        /* 8��Pcscf */
}TAF_IFACE_IPV4_DHCP_PARAM_STRU;


typedef struct
{
    TAF_IFACE_STATE_ENUM_U8                 enState;                            /* IPv4 ״̬ */
    VOS_UINT8                               ucRabId;                            /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    VOS_UINT8                               ucPduSessionId;                     /* ucPduSessionId��ʶ */
    VOS_UINT8                               aucRsv[3];
    VOS_UINT16                              usMtu;                              /* IPv4 MTU */
    TAF_IFACE_IPV4_DHCP_PARAM_STRU          stDhcpInfo;                         /* IPv4 DHCP��Ϣ */
} TAF_IFACE_IPV4_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucPcscfAddr[TAF_IPV6_ADDR_LEN];
} TAF_IFACE_IPV6_PCSCF_STRU;


typedef struct
{
    VOS_UINT8                           ucPcscfAddrNum;                         /* IPV6��P-CSCF��ַ��������Ч��Χ[0,8] */
    VOS_UINT8                           aucRsv[7];                              /* ���� */
    TAF_IFACE_IPV6_PCSCF_STRU           astPcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_IFACE_IPV6_PCSCF_LIST_STRU;


typedef struct
{
    VOS_UINT32                              bitOpPriDns   : 1;
    VOS_UINT32                              bitOpSecDns   : 1;
    VOS_UINT32                              bitOpSpare    : 30;

    VOS_UINT8                               aucTmpAddr[TAF_IPV6_ADDR_LEN];      /* �� PDP�����Ĵ�����IPV6��ַ���ȣ�������":" */
    VOS_UINT8                               aucAddr[TAF_IPV6_ADDR_LEN];         /* �� PDP�����Ĵ�����IPV6��ַ���ȣ�������":" */
    VOS_UINT8                               aucPrimDns[TAF_IPV6_ADDR_LEN];      /* �� PDP�����Ĵ�����IPV6��DNS���ȣ�������":" */
    VOS_UINT8                               aucSecDns[TAF_IPV6_ADDR_LEN];       /* �� PDP�����Ĵ�����IPV6��DNS���ȣ�������":" */
    TAF_IFACE_IPV6_PCSCF_LIST_STRU          stPcscfList;
}TAF_IFACE_IPV6_DHCP_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          bitOpLanAddr            : 1;
    VOS_UINT32                          bitOpPrefixAddr         : 1;
    VOS_UINT32                          bitOpMtuSize            : 1;
    VOS_UINT32                          bitOpPreferredLifetime  : 1;
    VOS_UINT32                          bitOpValidLifetime      : 1;
    VOS_UINT32                          bitOpSpare              : 27;

    VOS_UINT32                          ulBitCurHopLimit        :8;
    VOS_UINT32                          ulBitM                  :1;
    VOS_UINT32                          ulBitO                  :1;
    VOS_UINT32                          ulBitL                  :1;             /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          ulBitA                  :1;
    VOS_UINT32                          ulBitRsv                :20;

    VOS_UINT8                           aucLanAddr[TAF_IPV6_ADDR_LEN];          /* IPv6 ·����LAN�˿ڵ�ַ */
    VOS_UINT8                           aucPrefixAddr[TAF_IPV6_ADDR_LEN];       /* IPv6ǰ׺ */
    VOS_UINT32                          ulPrefixBitLen;                         /* IPv6ǰ׺���� */
    VOS_UINT32                          ulMtuSize;                              /* RA��Ϣ�й㲥��IPv6��MTU��ȡֵ */
    VOS_UINT32                          ulPreferredLifetime;                    /* IPv6ǰ׺��Preferred lifetime */
    VOS_UINT32                          ulValidLifetime;                        /* IPv6ǰ׺��Valid lifetime */
} TAF_IFACE_IPV6_RA_INFO_STRU;


typedef struct
{
    TAF_IFACE_STATE_ENUM_U8                 enState;                            /* IPv6 ״̬ */
    VOS_UINT8                               ucRabId;                            /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    VOS_UINT8                               ucPduSessionId;
    VOS_UINT8                               aucRsv[5];                          /* ����λ */
    TAF_IFACE_IPV6_RA_INFO_STRU             stRaInfo;                           /* IPv6 ·�ɹ�����Ϣ */
    TAF_IFACE_IPV6_DHCP_PARAM_STRU          stDhcpInfo;                         /* IPv6 DHCP��Ϣ */
} TAF_IFACE_IPV6_INFO_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_IFACE_UP_STRU                   stIfaceUp;
} TAF_IFACE_UP_REQ_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_IFACE_DOWN_STRU                 stIfaceDown;
} TAF_IFACE_DOWN_REQ_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_PS_CAUSE_ENUM_UINT32            enCause;
    VOS_UINT8                           ucCid;
    VOS_UINT8                           aucReserved[3];
} TAF_IFACE_UP_CNF_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_PS_CAUSE_ENUM_UINT32            enCause;
} TAF_IFACE_DOWN_CNF_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    TAF_IFACE_STATUS_ENUM_UINT8             enStatus;                           /* �ϱ�״̬���� */
    VOS_UINT8                               ucCid;                              /* Cid��Ϣ */
    TAF_PDP_TYPE_ENUM_UINT8                 enPdpType;                          /* PDP TYPE���� */
    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* �����û����� */
    TAF_PS_CAUSE_ENUM_UINT32                enCause;                            /* ȥ����ԭ��ֵ */
} TAF_IFACE_STATUS_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;

    VOS_UINT32                              bitOpActFlg         : 1;
    VOS_UINT32                              bitOpCleanFlg       : 1;
    VOS_UINT32                              bitOpSpare          : 30;

    VOS_UINT8                               ucCid;                              /* Cid��Ϣ */
    VOS_UINT8                               aucRsv[3];                          /* ����λ */
} TAF_IFACE_DATA_CHANNEL_STATE_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    VOS_UINT32                              bitOpActUsbNet      : 1;            /* ֪ͨ����USB NET��־λ */
    VOS_UINT32                              bitOpDeactUsbNet    : 1;            /* ֪ͨȥ����USB NET��־λ */
    VOS_UINT32                              bitOpSpare          : 30;
} TAF_IFACE_USBNET_OPER_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;

    VOS_UINT32                              bitOpUmtsQos        : 1;
    VOS_UINT32                              bitOpSpare          : 31;

    TAF_UMTS_QOS_STRU                       stUmtsQos;
    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* �û��������� */
    VOS_UINT8                               ucRabId;                            /* RABID */
    VOS_UINT8                               ucIfaceId;                          /* ȫ������ID */
    VOS_UINT8                               aucReserved[1];
} TAF_IFACE_REG_FC_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* �û��������� */
    VOS_UINT8                               ucRabId;                            /* RABID */
    VOS_UINT8                               ucIfaceId;                          /* ȫ������ID */
    VOS_UINT8                               aucReserved[1];
} TAF_IFACE_DEREG_FC_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    VOS_UINT32                              ulBitCid;                           /* Bit Cid��Ϣ */
} TAF_IFACE_GET_DYNAMIC_PARA_REQ_STRU;


typedef struct
{
    VOS_UINT32                              bitOpIpv4Valid      : 1;
    VOS_UINT32                              bitOpIpv6Valid      : 1;
    VOS_UINT32                              bitOpSpare          : 30;

    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* �û��������� */
    VOS_UINT8                               ucApnLen;                           /* APN���� */
    VOS_UINT8                               aucApn[TAF_MAX_APN_LEN + 1];        /* APN��Ϣ */
    VOS_UINT8                               aucRsv[6];                          /* ����λ */
    TAF_IFACE_IPV4_INFO_STRU                stIpv4Info;                         /* ����IPV4�����Ϣ��¼ */
    TAF_IFACE_IPV6_INFO_STRU                stIpv6Info;                         /* ����IPV6�����Ϣ��¼ */
} TAF_IFACE_DYNAMIC_INFO_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    VOS_UINT32                              ulBitCid;
    TAF_IFACE_DYNAMIC_INFO_STRU             astDynamicInfo[TAF_MAX_CID_NV + 1];     /* ��̬��ϸ��Ϣ */
} TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;

    VOS_UINT32                              bitOpPdpAddr        : 1;
    VOS_UINT32                              bitOpSpare          : 31;

    VOS_UINT8                               ucOldRabId;
    VOS_UINT8                               ucNewRabId;
    TAF_IFACE_RAB_OPER_TYPE_ENUM_U8         enOperType;
    PS_IFACE_ID_ENUM_UINT8                  enIfaceId;
    TAF_PDP_TYPE_ENUM_UINT8                 enPdpType;
    VOS_UINT8                               aucReserved[3];
    VOS_UINT32                              ulIpAddr;
} TAF_IFACE_RAB_INFO_IND_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
/*****************************************************************************
 �� �� ��  : TAF_IFACE_Up
 ��������  : ����DSM IFACE�ӿڷ��𼤻�IFACE����
 �������  : pstCtrl                    - DSM IFACE���ƽṹ
             pstIfaceUp                 - ����IFACE�����ṹ��
 �������  : ��
 �� �� ֵ  : VOS_OK                     - �ɹ�
             VOS_ERR                    - ʧ��
*****************************************************************************/
VOS_UINT32 TAF_IFACE_Up(
    const TAF_CTRL_STRU                *pstCtrl,
    const TAF_IFACE_UP_STRU            *pstIfaceUp
);

/*****************************************************************************
 �� �� ��  : TAF_IFACE_Down
 ��������  : ����DSM IFACE�ӿڶϿ�IFACE����
 �������  : pstCtrl                    - DSM IFACE���ƽṹ
             pstIfaceDown               - �Ͽ�IFACE�����ṹ��
 �������  : ��
 �� �� ֵ  : VOS_OK                     - �ɹ�
             VOS_ERR                    - ʧ��
*****************************************************************************/
VOS_UINT32 TAF_IFACE_Down(
    const TAF_CTRL_STRU                *pstCtrl,
    const TAF_IFACE_DOWN_STRU          *pstIfaceDown
);

/*****************************************************************************
 �� �� ��  : TAF_IFACE_GetDynamicPara
 ��������  : ����DSM IFACE�ӿڷ����ѯ��ӦBit Cid�Ķ�̬��Ϣ
 �������  : const TAF_CTRL_STRU                *pstCtrl,
             const VOS_UINT32                    ulBitCid
 �������  : ��
 �� �� ֵ  : VOS_OK                     - �ɹ�
             VOS_ERR                    - ʧ��
*****************************************************************************/
VOS_UINT32 TAF_IFACE_GetDynamicPara(
    const TAF_CTRL_STRU                *pstCtrl,
    const VOS_UINT32                    ulBitCid
);


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif

