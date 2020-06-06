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

#ifndef __ATCTXPACKET_H__
#define __ATCTXPACKET_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "VosPidDef.h"
#include "v_id.h"
#include "hi_list.h"
#include "AtTypeDef.h"
#include "MnClient.h"
#include "TafPsApi.h"
#include "acore_nv_stru_gucnas.h"
#include "nv_stru_gucnas.h"
#include "AtTimer.h"
#include "ImsaWifiInterface.h"
#include "TafIfaceApi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define AT_IPV6_CAPABILITY_IPV4_ONLY            (1)
#define AT_IPV6_CAPABILITY_IPV6_ONLY            (2)
#define AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP  (4)
#define AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP  (8)

#define AT_PS_RABID_OFFSET              (5)                 /* RABIDƫ�� */
#define AT_PS_RABID_MAX_NUM             (11)                /* RABID���� */
#define AT_PS_MIN_RABID                 (5)                 /* RABID��Сֵ */
#define AT_PS_MAX_RABID                 (15)                /* RABID���ֵ */
#define AT_PS_INVALID_RABID             (0xFF)              /* RABID��Чֵ */
#define AT_PS_RABID_MODEM_1_MASK        (0x40)

#define IPV6_ADDRESS_TEST_MODE_ENABLE                    (0x55aa55aa)           /* 0x55aa55aaֵIPV6��ַΪ����ģʽ */

#define AT_PS_APN_CUSTOM_CHAR_MAX_NUM   (16)            /* ��Ҫ��TAF_NVIM_APN_CUSTOM_CHAR_MAX_NUM����һ�� */
#define AT_PS_ASCII_UINT_SEPARATOR      (31)
#define AT_PS_ASCII_SLASH               (47)
#define AT_PS_ASCII_BACKSLASH           (92)
#define AT_PS_ASCII_DELETE              (127)
#define AT_PS_RAC_STRING_LENGTH         (3)
#define AT_PS_LAC_STRING_LENGTH         (3)
#define AT_PS_SGSN_STRING_LENGTH        (4)
#define AT_PS_RNC_STRING_LENGTH         (3)
#define AT_PS_GRPS_STRING_LENGTH        (4)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

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

/*��Ϣ������ָ��*/
typedef VOS_VOID (*AT_PS_RPT_IFACE_RSLT_FUNC)(TAF_IFACE_STATUS_IND_STRU *pstIfaceStatus);


/*lint -e958 -e959 �޸���:l60609;ԭ��:64bit*/
typedef struct
{
    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType;
    AT_PS_RPT_IFACE_RSLT_FUNC           pRptIfaceRsltFunc;
}AT_PS_REPORT_IFACE_RESULT_STRU;
/*lint +e958 +e959 �޸���:l60609;ԭ��:64bit*/


typedef struct
{
    VOS_UINT32                          ulUsed;                                 /* ָ��CID�Ƿ��Ѿ�ͨ��CHDATA����������ͨ����VOS_TRUE:�Ѿ����ã�VOS_FALSE:δ���� */
    VOS_UINT32                          ulRmNetId;                              /* ����ͨ��ID
                                                                                   VCOMͨ�� :RNIC_DEV_ID_RMNET0 ~ RNIC_DEV_ID_RMNET4
                                                                                   */
    VOS_UINT32                          ulIfaceId;
    VOS_UINT32                          ulIfaceActFlg;                          /* ָ��CID�Ƿ��Ѿ�PDP���VOS_TRUE:�Ѿ����VOS_FALSE:δ���� */
    AT_CLIENT_TAB_INDEX_UINT8           enPortIndex;
    VOS_UINT8                           aucReserved[7];
}AT_PS_DATA_CHANL_CFG_STRU;

/*****************************************************************************
 �ṹ����  : AT_IMS_EMC_IPV4_PDN_INFO_STRU
 �ṹ˵��  : IMS EMC IPV4 PDN��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucIpAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucDnsPrimAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucDnsSecAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT16                          usMtu;
    VOS_UINT16                          usReserved;

} AT_IMS_EMC_IPV4_PDN_INFO_STRU;

/*****************************************************************************
 �ṹ����  : AT_IMS_EMC_IPV6_PDN_INFO_STRU
 �ṹ˵��  : IMS EMC IPV6 PDN��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucIpAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                           aucDnsPrimAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                           aucDnsSecAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT16                          usMtu;
    VOS_UINT16                          ausReserved[3];

} AT_IMS_EMC_IPV6_PDN_INFO_STRU;

/*****************************************************************************
 �ṹ����  : AT_IMS_EMC_RDP_STRU
 �ṹ˵��  : IMS EMC ��̬�����ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitOpIPv4PdnInfo    : 1;
    VOS_UINT32                          bitOpIPv6PdnInfo    : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    AT_IMS_EMC_IPV4_PDN_INFO_STRU       stIPv4PdnInfo;
    AT_IMS_EMC_IPV6_PDN_INFO_STRU       stIPv6PdnInfo;

} AT_IMS_EMC_RDP_STRU;


typedef struct
{
    VOS_UINT8                                   ucCustomCharNum;
    VOS_UINT8                                   ucProtocolStringCheckFlag;
    VOS_UINT8                                   ucReserved1;
    VOS_UINT8                                   ucReserved2;
    VOS_UINT8                                   ucReserved3;
    VOS_UINT8                                   ucReserved4;
    VOS_UINT8                                   ucReserved5;
    VOS_UINT8                                   ucReserved6;
    VOS_UINT8                                   aucCustomChar[AT_PS_APN_CUSTOM_CHAR_MAX_NUM];
}AT_PS_APN_CUSTOM_FORMAT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIpv6Capability;
    VOS_UINT8                           bitOpIpv6AddrFormat     : 1;            /* IPv6��ַ��ʽ 0: ʮ���Ƶ�ָ�ʽ; 1: 16����ð�ŷָ���ʽ */
    VOS_UINT8                           bitOpIpv6SubnetNotation : 1;            /* IPv6���������ʽ 0: IP��ַ����������������ʾ��ͨ���ո�ָ�; 1: ͨ��б�߷ָ����� */
    VOS_UINT8                           bitOpIpv6LeadingZeros   : 1;            /* ����IPv6��ַ�Ƿ�ʡ��ǰ��0 0: ʡ��ǰ��0; 1: ��ʡ��ǰ��0 */
    VOS_UINT8                           bitOpIpv6CompressZeros  : 1;            /* ����IPv6��ַ�Ƿ�ѹ����ַ 0: ��ѹ��0; 1: ѹ��0 */
    VOS_UINT8                           bitOpSpare              : 4;
    VOS_UINT8                           aucReserved1[2];
    VOS_UINT32                          ulIpv6AddrTestModeCfg;

    VOS_UINT8                           ucSharePdpFlag;
    VOS_UINT8                           aucReserved2[7];

    VOS_INT32                           lSpePort;
    VOS_UINT32                          ulIpfPortFlg;

    AT_PS_APN_CUSTOM_FORMAT_CFG_STRU            astApnCustomFormatCfg[MODEM_ID_BUTT];
}AT_COMM_PS_CTX_STRU;


typedef struct
{
    /* CID������ͨ���Ķ�Ӧ��ϵ */
    AT_PS_DATA_CHANL_CFG_STRU           astChannelCfg[TAF_MAX_CID + 1];

    /* PS����д����� */
    TAF_PS_CAUSE_ENUM_UINT32            enPsErrCause;

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    /* IP��ַ��IFACEID��ӳ���, IP��ַΪ������ */
    VOS_UINT32                          aulIpAddrIfaceIdMap[PS_IFACE_ID_BUTT];
#else
    /* IP��ַ��RABID��ӳ���, IP��ַΪ������ */
    VOS_UINT32                          aulIpAddrRabIdMap[AT_PS_RABID_MAX_NUM];
    VOS_UINT32                          ulReserved;
#endif

#if (FEATURE_ON == FEATURE_IMS)
    /* IMS EMC PDN ��̬��Ϣ */
    AT_IMS_EMC_RDP_STRU                 stImsEmcRdp;
#endif

} AT_MODEM_PS_CTX_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/

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

#endif /* end of AtCtxPacket.h */
