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

/*****************************************************************************
   1 ͷ�ļ�����
*****************************************************************************/
#include "AtDataProc.h"
#include "ATCmdProc.h"
#include "AtCsdInterface.h"
#include "AtTafAgentInterface.h"
#include "AtCtxPacket.h"
#include "TafIfaceApi.h"
#include "AtCmdPacketProc.h"
#include "securec.h"



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_DATAPROC_C

/*****************************************************************************
   2 ȫ�ֱ�������
*****************************************************************************/

/* HiLinkģʽ: ����ģʽ������ģʽ */
AT_HILINK_MODE_ENUM_U8                  g_enHiLinkMode = AT_HILINK_NORMAL_MODE;

/* ����ָ����FC ID��Ӧ��FC Pri */
AT_FCID_MAP_STRU                        g_stFcIdMaptoFcPri[FC_ID_BUTT];

CONST AT_CHDATA_RNIC_RMNET_ID_STRU            g_astAtChdataRnicRmNetIdTab[] =
{
#if (MULTI_MODEM_NUMBER >= 2)
    {AT_CH_DATA_CHANNEL_ID_1, RNIC_DEV_ID_RMNET0, PS_IFACE_ID_RMNET0, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_2, RNIC_DEV_ID_RMNET1, PS_IFACE_ID_RMNET1, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_3, RNIC_DEV_ID_RMNET2, PS_IFACE_ID_RMNET2, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_4, RNIC_DEV_ID_RMNET3, PS_IFACE_ID_RMNET3, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_5, RNIC_DEV_ID_RMNET4, PS_IFACE_ID_RMNET4, {0, 0}}
#if (MULTI_MODEM_NUMBER == 3)
    ,
    {AT_CH_DATA_CHANNEL_ID_6, RNIC_DEV_ID_RMNET5, PS_IFACE_ID_RMNET5, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_7, RNIC_DEV_ID_RMNET6, PS_IFACE_ID_RMNET6, {0, 0}}
#endif
#else
    {AT_CH_DATA_CHANNEL_ID_1, RNIC_DEV_ID_RMNET0, PS_IFACE_ID_RMNET0, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_2, RNIC_DEV_ID_RMNET1, PS_IFACE_ID_RMNET1, {0, 0}},
    {AT_CH_DATA_CHANNEL_ID_3, RNIC_DEV_ID_RMNET2, PS_IFACE_ID_RMNET2, {0, 0}}
#endif
};

CONST AT_PS_RMNET_IFACE_ID_STRU               g_astAtPsIfaceIdRmNetIdTab[] =
{
    {PS_IFACE_ID_RMNET0,    RNIC_DEV_ID_RMNET0,     0,      0},
    {PS_IFACE_ID_RMNET1,    RNIC_DEV_ID_RMNET1,     0,      0},
    {PS_IFACE_ID_RMNET2,    RNIC_DEV_ID_RMNET2,     0,      0},
#if (MULTI_MODEM_NUMBER >= 2)
    {PS_IFACE_ID_RMNET3,    RNIC_DEV_ID_RMNET3,     0,      0},
    {PS_IFACE_ID_RMNET4,    RNIC_DEV_ID_RMNET4,     0,      0},
#if (MULTI_MODEM_NUMBER == 3)
    {PS_IFACE_ID_RMNET5,    RNIC_DEV_ID_RMNET5,     0,      0},
    {PS_IFACE_ID_RMNET6,    RNIC_DEV_ID_RMNET6,     0,      0},
#endif
#endif
    {PS_IFACE_ID_NDIS0,     RNIC_DEV_ID_BUTT,       0,      0},
};


AT_PS_FC_IFACE_ID_STRU                        g_astAtPsFcIfaceIdTab[] =
{
    /* APP��IFACE ID��� */
    {FC_ID_NIC_1,   PS_IFACE_ID_RMNET0, 0, 0},
    {FC_ID_NIC_2,   PS_IFACE_ID_RMNET1, 0, 0},
    {FC_ID_NIC_3,   PS_IFACE_ID_RMNET2, 0, 0},
#if (MULTI_MODEM_NUMBER >= 2)
    {FC_ID_NIC_4,   PS_IFACE_ID_RMNET3, 0, 0},
    {FC_ID_NIC_5,   PS_IFACE_ID_RMNET4, 0, 0},
#if (MULTI_MODEM_NUMBER == 3)
    {FC_ID_NIC_6,   PS_IFACE_ID_RMNET5, 0, 0},
    {FC_ID_NIC_7,   PS_IFACE_ID_RMNET6, 0, 0},
#endif
#endif
    /* NDIS��IFACE ID��� */
    {FC_ID_NIC_1,   PS_IFACE_ID_NDIS0,  0, 0},
};

CONST AT_PS_REPORT_IFACE_RESULT_STRU           g_astAtRptIfaceResultTab[] =
{
    /* ��ϢID */                            /* ��Ϣ������ */
    {TAF_IFACE_USER_TYPE_APP,               AT_PS_ProcAppIfaceStatus},
    {TAF_IFACE_USER_TYPE_NDIS,              AT_PS_ProcNdisIfaceStatus},
};

const AT_PS_EVT_FUNC_TBL_STRU           g_astAtIfaceEvtFuncTbl[] =
{
    {ID_EVT_TAF_IFACE_UP_CNF,                               AT_RcvTafIfaceEvtIfaceUpCnf             },
    {ID_EVT_TAF_IFACE_DOWN_CNF,                             AT_RcvTafIfaceEvtIfaceDownCnf           },
    {ID_EVT_TAF_IFACE_STATUS_IND,                           AT_RcvTafIfaceEvtIfaceStatusInd         },
    {ID_EVT_TAF_IFACE_DATA_CHANNEL_STATE_IND,               AT_RcvTafIfaceEvtDataChannelStateInd    },
    {ID_EVT_TAF_IFACE_USBNET_OPER_IND,                      AT_RcvTafIfaceEvtUsbNetOperInd          },
    {ID_EVT_TAF_IFACE_REG_FC_IND,                           AT_RcvTafIfaceEvtRegFcInd               },
    {ID_EVT_TAF_IFACE_DEREG_FC_IND,                         AT_RcvTafIfaceEvtDeRegFcInd             },
    {ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF,                 AT_RcvTafIfaceEvtDyamicParaCnf          },
    {ID_EVT_TAF_IFACE_RAB_INFO_IND,                         AT_RcvTafIfaceEvtRabInfoInd             },
};

const AT_PS_DYNAMIC_INFO_REPORT_FUNC_TBL_STRU           g_astAtDynamicInfoReportFuncTbl[] =
{
    {AT_CMD_IPV6TEMPADDR_SET,                               AT_PS_ReportSetIpv6TempAddrRst  },
    {AT_CMD_DHCP_SET,                                       AT_PS_ReportSetDhcpRst          },
    {AT_CMD_DHCP_QRY,                                       AT_PS_ReportQryDhcpRst          },
    {AT_CMD_DHCPV6_SET,                                     AT_PS_ReportSetDhcpv6Rst        },
    {AT_CMD_DHCPV6_QRY,                                     AT_PS_ReportQryDhcpv6Rst        },
    {AT_CMD_APRAINFO_SET,                                   AT_PS_ReportSetApRaInfoRst      },
    {AT_CMD_APRAINFO_QRY,                                   AT_PS_ReportQryApRaInfoRst      },
    {AT_CMD_APLANADDR_SET,                                  AT_PS_ReportSetApLanAddrRst     },
    {AT_CMD_APLANADDR_QRY,                                  AT_PS_ReportQryApLanAddrRst     },
    {AT_CMD_APCONNST_SET,                                   AT_PS_ReportSetApConnStRst      },
    {AT_CMD_APCONNST_QRY,                                   AT_PS_ReportQryApConnStRst      },
    {AT_CMD_DCONNSTAT_QRY,                                  AT_PS_ReportQryDconnStatRst     },
    {AT_CMD_DCONNSTAT_TEST,                                 AT_PS_ReportTestDconnStatRst    },
    {AT_CMD_NDISSTATQRY_QRY,                                AT_PS_ReportQryNdisStatRst      },
    {AT_CMD_CGMTU_SET,                                      AT_PS_ReportSetCgmtuRst         },
};

/*****************************************************************************
   3 ��������������
*****************************************************************************/

extern const TAF_CHAR                       *g_PppDialRateDisplay[];

#if (FEATURE_LTE == FEATURE_ON)
extern const AT_DISPLAY_RATE_PAIR_STRU      g_ucLTERateDisplay[AT_UE_LTE_CATEGORY_NUM_MAX];
#endif

extern const VOS_CHAR                       *g_ucDialRateDisplayNv[];

/*****************************************************************************
   4 ����ʵ��
*****************************************************************************/

VOS_UINT32 AT_Ipv4AddrAtoi(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber,
    VOS_UINT32                          ulNumBufLen
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulStrLen;
    VOS_UINT32                          ulNumLen = 0;
    VOS_UINT32                          ulDotNum = 0;
    VOS_UINT32                          ulValTmp = 0;
    VOS_UINT8                           aucAddr[4] = {0};


    if ( (pcString == VOS_NULL_PTR) || (pucNumber == VOS_NULL_PTR) )
    {
        return VOS_ERR;
    }

    if (ulNumBufLen < TAF_IPV4_ADDR_LEN)
    {
        return VOS_ERR;
    }

    ulStrLen = VOS_StrLen(pcString);

    if ( ulStrLen > VOS_StrLen("255.255.255.255") )
    {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: PCSCF IPV4 address length out of range");
        return VOS_ERR;
    }

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (pcString[i] >= '0') && (pcString[i] <= '9') )
        {
            ulValTmp = (ulValTmp * 10) + (pcString[i] - '0');

            ulNumLen++;
            continue;
        }
        else if ( pcString[i] == '.' )
        {
            if ( (ulNumLen == 0) || (ulNumLen > 3) )
            {
                AT_NORM_LOG("AT_Ipv4AddrAtoi: the number between dots is out of range");
                return VOS_ERR;
            }

            if ( ulValTmp > 255 )
            {
                AT_NORM_LOG("AT_Ipv4AddrAtoi: the number is larger than 255");
                return VOS_ERR;
            }

            aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

            ulValTmp = 0;
            ulNumLen = 0;

            /* ͳ��'.'�ĸ��� */
            ulDotNum++;

            continue;
        }
        else
        {
            AT_NORM_LOG("AT_Ipv4AddrAtoi: character not number nor dot, return ERROR");
            /* ����ֱֵ�ӷ���ʧ�� */
            return VOS_ERR;
        }
    }

    /* �������3��'.'�򷵻�ʧ�� */
    if ( ulDotNum != 3 )
    {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: dot number is not 3");
        return VOS_ERR;
    }

    /* �������ַ���ȡֵ */
    if (ulValTmp > 255)
    {
        AT_NORM_LOG("AT_Ipv4AddrAtoi: last number is larger than 255");
        return VOS_ERR;
    }

    aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

    lMemResult = memcpy_s(pucNumber, ulNumBufLen, aucAddr, sizeof(aucAddr));
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulNumBufLen, sizeof(aucAddr));

    return VOS_OK;
}


VOS_UINT32 AT_Ipv4AddrItoa(
    VOS_CHAR                           *pcString,
    VOS_UINT32                          ulStrLen,
    VOS_UINT8                          *pucNumber
)
{
    errno_t                             lMemResult;

    if ( (pcString == VOS_NULL_PTR) || (pucNumber == VOS_NULL_PTR) )
    {
        return VOS_ERR;
    }

    if (ulStrLen < TAF_MAX_IPV4_ADDR_STR_LEN)
    {
        return VOS_ERR;
    }

    lMemResult = memset_s(pcString, ulStrLen, 0x00, ulStrLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulStrLen, ulStrLen);

    VOS_sprintf_s(pcString, ulStrLen, "%d.%d.%d.%d", pucNumber[0], pucNumber[1], pucNumber[2], pucNumber[3]);

    return VOS_OK;
}


VOS_UINT32 AT_Ipv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT32                          ulStrBuflen,
    VOS_UINT8                          *pucNumber,
    VOS_UINT32                          ulAddrNumCnt
)
{
    errno_t                             lMemResult;

    if ( (pcString == VOS_NULL_PTR) || (pucNumber == VOS_NULL_PTR) )
    {
        return VOS_ERR;
    }

    if ((ulStrBuflen < TAF_MAX_IPV4_ADDR_STR_LEN) || (ulAddrNumCnt < TAF_IPV4_ADDR_LEN))
    {
        return VOS_ERR;
    }

    lMemResult = memset_s(pcString, ulStrBuflen, 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulStrBuflen, TAF_MAX_IPV4_ADDR_STR_LEN);

    /* ��������ַΪ�գ��򷵻ؿ��ַ��� */
    if ((pucNumber[0] == 0)
      &&(pucNumber[1] == 0)
      &&(pucNumber[2] == 0)
      &&(pucNumber[3] == 0))
    {
        return VOS_OK;
    }

    VOS_sprintf_s(pcString, ulStrBuflen, "%d.%d.%d.%d", pucNumber[0], pucNumber[1], pucNumber[2], pucNumber[3]);

    return VOS_OK;
}


VOS_VOID AT_PcscfIpv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT32                          ulStrBufLen,
    VOS_UINT8                          *pucNumber
)
{
    errno_t                             lMemResult;

    if (ulStrBufLen < TAF_MAX_IPV4_ADDR_STR_LEN)
    {
        return;
    }

    lMemResult = memset_s(pcString, ulStrBufLen, 0, ulStrBufLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulStrBufLen, ulStrBufLen);

    VOS_sprintf_s(pcString, ulStrBufLen, "%d.%d.%d.%d", pucNumber[0], pucNumber[1], pucNumber[2], pucNumber[3]);
}


VOS_UINT32 AT_Ipv6AddrAtoi(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber,
    VOS_UINT32                          ulIpv6BufLen
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulStrLen;
    VOS_UINT32                          ulNumLen = 0;
    VOS_UINT32                          ulDotNum = 0;
    VOS_UINT32                          ulValTmp = 0;
    VOS_UINT8                           aucAddr[TAF_IPV6_ADDR_LEN] = {0};


    if ( (pcString == VOS_NULL_PTR) || (pucNumber == VOS_NULL_PTR) )
    {
        return VOS_ERR;
    }

    if (ulIpv6BufLen < TAF_IPV6_ADDR_LEN)
    {
        return VOS_ERR;
    }

    ulStrLen = VOS_StrLen(pcString);

    if ( ulStrLen > VOS_StrLen("255.255.255.255.255.255.255.255.255.255.255.255.255.255.255.255") )
    {
        return VOS_ERR;
    }

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (pcString[i] >= '0') && (pcString[i] <= '9') )
        {
            ulValTmp = (ulValTmp * 10) + (pcString[i] - '0');

            ulNumLen++;
            continue;
        }
        else if ( pcString[i] == '.' )
        {
            if ( (ulNumLen == 0) || (ulNumLen > 3) )
            {
                return VOS_ERR;
            }

            if ( ulValTmp > 255 )
            {
                return VOS_ERR;
            }

            aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

            ulValTmp = 0;
            ulNumLen = 0;

            /* ͳ��'.'�ĸ��� */
            ulDotNum++;

            continue;
        }
        else
        {
            /* ����ֱֵ�ӷ���ʧ�� */
            return VOS_ERR;
        }
    }

    /* �������3��'.'�򷵻�ʧ�� */
    if ((TAF_IPV6_ADDR_LEN - 1) != ulDotNum )
    {
        return VOS_ERR;
    }

    /* �������ַ���ȡֵ */
    if (ulValTmp > 255)
    {
        return VOS_ERR;
    }

    aucAddr[ulDotNum] = (VOS_UINT8)ulValTmp;

    lMemResult = memcpy_s(pucNumber, ulIpv6BufLen, aucAddr, TAF_IPV6_ADDR_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulIpv6BufLen, TAF_IPV6_ADDR_LEN);

    return VOS_OK;
}


VOS_VOID  AT_Ipv6LenStrToAddrProcCompressed(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                           ucColonCount,
    VOS_UINT8                           ucDotCount,
    VOS_UINT8                           ucStrlen,
    VOS_UINT8                           ucIdxPos
)
{
    VOS_UINT8                           i;

    /* ���ַ�����ѹ��λ�ÿ�ʼ˳������ƶ� */
    for (i = ucStrlen; i >= ucIdxPos; i--)
    {
        if (ucDotCount != AT_MAX_IPV6_STR_DOT_NUM)
        {
            pucStr[i + AT_MAX_IPV6_STR_COLON_NUM - ucColonCount] =  pucStr[i];
        }
        else
        {
            pucStr[i + AT_MAX_IPV4V6_STR_COLON_NUM - ucColonCount] =  pucStr[i];
        }
    }

    /* ����ѹ����ð�� */
    if (ucDotCount != AT_MAX_IPV6_STR_DOT_NUM)
    {
        for (i = ucIdxPos; i < (ucIdxPos + AT_MAX_IPV6_STR_COLON_NUM - ucColonCount); i++)
        {
            pucStr[i + 1] =  ':';
        }
    }
    else
    {
        for (i = ucIdxPos; i < (ucIdxPos + AT_MAX_IPV4V6_STR_COLON_NUM - ucColonCount); i++)
        {
            pucStr[i + 1] =  ':';
        }
    }

    return;
}


VOS_UINT32 AT_Ipv6LenStrToAddrAccess(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucColonCount,
    VOS_UINT8                          *pucDotCount,
    VOS_UINT8                          *pucStrlen,
    VOS_UINT8                          *pucIdxPos
)
{
    VOS_UINT32                          i;

    for (i = 0; ((i < TAF_MAX_IPV6_ADDR_COLON_STR_LEN) && (pucStr[i] != '\0')); i++)
    {
        if ( (pucStr[i] < '0' || pucStr[i] > '9')
          && (pucStr[i] < 'A' || pucStr[i] > 'F')
          && (pucStr[i] != '.')
          && (pucStr[i] != ':') )
        {
            return VOS_ERR;
        }
        /* ȡ�ò���ð������λ�� */
        if ((i > 0)
         && (pucStr[i - 1] == ':')
         && (pucStr[i] == ':'))
        {
            *pucIdxPos = (VOS_UINT8)i;
        }

        /* ͳ��ð�Ÿ��� */
        if (pucStr[i] == ':')
        {
            (*pucColonCount)++;
        }

        /*ͳ�Ƶ�Ÿ���*/
        if (pucStr[i] == '.')
        {
            (*pucDotCount)++;
        }
    }

    *pucStrlen = (VOS_UINT8)i;

    return VOS_OK;
}


VOS_UINT32 AT_PcscfIpv6StrToAddr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr,
    VOS_UINT8                           ucColonCount
)
{
    VOS_UINT8                           i;
    VOS_UINT8                           j;
    VOS_UINT16                          usValue;            /*Ipv6ʮ������ת����*/
    VOS_UINT8                           ucValue;            /*Ipv4ʮ������ת����*/
    VOS_UINT32                          ulNumLen;

    usValue                             = 0;
    ucValue                             = 0;
    j                                   = 0;
    ulNumLen                            = 0;

    if ((pucStr == VOS_NULL_PTR)
     || (pucIpAddr == VOS_NULL_PTR))
    {
        AT_NORM_LOG("AT_PcscfIpv6StrToAddr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    for (i = 0; ((i < TAF_MAX_IPV6_ADDR_COLON_STR_LEN) && (pucStr[i] != '\0')); i++)
    {
        /* ƥ���ַ� */
        if (pucStr[i] != ':')
        {
            usValue <<= 4;

            if ((pucStr[i] >= '0') &&(pucStr[i] <= '9'))
            {
                /* ʮ���Ƹ�ʽת�� */
                usValue += (pucStr[i] - '0');
                ucValue  = (VOS_UINT8)((ucValue * 10) + (pucStr[i] - 0x30));
            }
            else
            {
                /* ʮ�����Ƹ�ʽת�� */
                 usValue += ((pucStr[i] - 'A') + 10);
            }

            ulNumLen++;
        }
        /* ƥ�䵽ð�� */
        else
        {
            /* ð��֮����ַ�����4������Ϊ��ʽ���� */
            if (ulNumLen > 4)
            {
                AT_ERR_LOG("AT_PcscfIpv6StrToAddr: the number of char betwwen colons is more than 4, return ERROR");
                return VOS_ERR;
            }

            /* IPV6ʮ������ȡ�߰�λ���� */
            pucIpAddr[j] = (VOS_UINT8)((usValue >> 8) & 0x00FF);
            j++;
            /* IPV6ʮ������ȡ�Ͱ�λ���� */
            pucIpAddr[j] = (VOS_UINT8)(usValue & 0x00FF);
            j++;
            usValue      = 0;
            ulNumLen     = 0;
        }
    }

    /* ƥ�����һ��ת�� */
    if (ucColonCount == AT_MAX_IPV6_STR_COLON_NUM)
    {
        pucIpAddr[j] = (VOS_UINT8)((usValue >> 8) & 0x00FF);
        j++;
        pucIpAddr[j] = (VOS_UINT8)(usValue & 0x00FF);
        j++;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckPcscfIpv6Addr(
    VOS_CHAR                           *pucIpv6Str,
    VOS_UINT32                         *pulPortExistFlg
)
{
    VOS_CHAR                           *pucIpv6Start = VOS_NULL_PTR;
    VOS_CHAR                           *pucIpv6End   = VOS_NULL_PTR;

    if ((pucIpv6Str == VOS_NULL_PTR)
     || (pulPortExistFlg == VOS_NULL_PTR))
    {
        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    pucIpv6End   = VOS_StrStr(pucIpv6Str, "]");
    pucIpv6Start = VOS_StrStr(pucIpv6Str, "[");

    if ((pucIpv6End == VOS_NULL_PTR)
     && (pucIpv6Start == VOS_NULL_PTR))
    {
        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: NO [ ] symbol in IPV6 addr, port not exists");
        *pulPortExistFlg = VOS_FALSE;
        return VOS_OK;
    }

    if ((pucIpv6End != VOS_NULL_PTR)
     && (pucIpv6Start != VOS_NULL_PTR)
     && (pucIpv6End > pucIpv6Start))
    {
        if ((pucIpv6End - pucIpv6Start) > TAF_MAX_IPV6_ADDR_COLON_STR_LEN)
        {
            AT_ERR_LOG("AT_CheckPcscfIpv6Addr: length of IPV6 addr in [] is larger than 39, return ERROR");
            return VOS_ERR;
        }

        AT_NORM_LOG("AT_CheckPcscfIpv6Addr: Have both [ ] symbol in IPV6 addr");
        *pulPortExistFlg = VOS_TRUE;

        return VOS_OK;
    }

    AT_ERR_LOG("AT_CheckPcscfIpv6Addr: IPV6 addr format incorrect");
    return VOS_ERR;
}



VOS_UINT32  AT_ParsePortFromPcscfIpv6Addr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpv6Addr,
    VOS_UINT32                          ulIpv6BufLen,
    VOS_UINT32                         *pulPortExistFlg,
    VOS_UINT32                         *pulPortNum
)
{
    VOS_CHAR                           *pucIpv6Start = VOS_NULL_PTR;
    VOS_CHAR                           *pucIpv6End   = VOS_NULL_PTR;
    VOS_CHAR                           *pucIpv6Str   = VOS_NULL_PTR;
    VOS_CHAR                           *pcStrPort    = VOS_NULL_PTR;
    errno_t                             lMemResult;

    if ((pucStr == VOS_NULL_PTR)
     || (pucIpv6Addr == VOS_NULL_PTR)
     || (pulPortExistFlg == VOS_NULL_PTR)
     || (pulPortNum == VOS_NULL_PTR))
    {
        AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    if (AT_CheckPcscfIpv6Addr((VOS_CHAR*)pucStr, pulPortExistFlg) != VOS_OK)
    {
        AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: AT_CheckPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    /* [ipv6]:port��ʽ���ݴ�����ȡipv6�˿ں� */
    pucIpv6Str   = (VOS_CHAR*)pucStr;
    pucIpv6End   = VOS_StrStr(pucIpv6Str, "]");
    pucIpv6Start = VOS_StrStr(pucIpv6Str, "[");

    if (*pulPortExistFlg == VOS_TRUE)
    {
        if ( (pucIpv6End == VOS_NULL_PTR)
          || (pucIpv6Start == VOS_NULL_PTR))
        {
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: NO [ ] symbol in IPV6 addr, return ERROR");
            return VOS_ERR;
        }

        /* �����������ڵ�IPV6��ַ��Ϣ���Ƴ��˿ں� */
        lMemResult = memcpy_s(pucIpv6Addr, ulIpv6BufLen, pucIpv6Start + 1, (VOS_UINT32)((pucIpv6End - pucIpv6Start) - 1));
        TAF_MEM_CHK_RTN_VAL(lMemResult, ulIpv6BufLen, (VOS_UINT32)((pucIpv6End - pucIpv6Start) - 1));

        /* ��¼�ָ���ַ�Ͷ˿ڵ�ð�ŵ�ַ */
        pcStrPort = VOS_StrStr(pucIpv6End, ":");

        /* [ipv6]:port��ʽû��ð�ţ�����ERROR*/
        if (pcStrPort == VOS_NULL_PTR)
        {
            *pulPortExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port colon missed, return ERROR ");
            return VOS_ERR;
        }

        /* ð��ǰ�治��"]"������ERROR */
        if (pcStrPort != pucIpv6End + 1)
        {
            *pulPortExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port colon location incorrect, return ERROR ");
            return VOS_ERR;
        }

        /* [ipv6]:port��ʽ�˿ڽ������󣬷���ERROR*/
        if (AT_PortAtoI(pcStrPort+1, pulPortNum) != VOS_OK)
        {
            *pulPortExistFlg = VOS_FALSE;
            AT_ERR_LOG("AT_ParsePortFromPcscfIpv6Addr: IPV6 Port decode ERROR");
            return VOS_ERR;
        }

        *pulPortExistFlg = VOS_TRUE;
    }
    else
    {
        /* û�ж˿ںţ�������ַ���ֻ����IPV6��ַ */
        lMemResult = memcpy_s(pucIpv6Addr, ulIpv6BufLen, pucStr, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, ulIpv6BufLen, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    }

    return VOS_OK;
}


VOS_UINT32  AT_ParseAddrFromPcscfIpv6Addr(
    VOS_UINT8                          *pucStr,
    VOS_UINT32                          ulStrLen,
    VOS_UINT8                          *pucIpAddr
)
{
    VOS_UINT8                           ucColonCount;       /* �ַ�����ð�Ÿ��� */
    VOS_UINT8                           ucDotCount;         /* �ַ����е�Ÿ��� */
    VOS_UINT8                           ucStrlen;           /* �ַ������� */
    VOS_UINT8                           ucIdxPos;           /* ��Ҫ����ð�ŵ�λ�� */
    VOS_UINT32                          ulResult;

    ucColonCount = 0;
    ucDotCount   = 0;
    ucStrlen     = 0;
    ucIdxPos     = 0xFF;

    if ((pucStr == VOS_NULL_PTR)
     || (pucIpAddr == VOS_NULL_PTR))
    {
        AT_NORM_LOG("AT_ParseAddrFromPcscfIpv6Addr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    /* ����IPV6��ַ�ַ��� */
    if (AT_Ipv6LenStrToAddrAccess(pucStr, &ucColonCount, &ucDotCount, &ucStrlen, &ucIdxPos) != VOS_OK)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: AT_Ipv6LenStrToAddrAccess FAIL, return ERROR");
        return VOS_ERR;
    }

    /* ��֧��IPV4IPV6����͵ĸ�ʽ */
    if (ucDotCount != 0)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: There have dot symbol in address format, return ERROR");
        return VOS_ERR;
    }

    /* �ַ���Ϊ�շ���ʧ�� */
    if (ucStrlen == 0)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: IP address length is 0, return ERROR");
        return VOS_ERR;
    }

    /* ð�Ÿ�������7�򷵻�ʧ�� */
    if (ucColonCount > AT_MAX_IPV6_STR_COLON_NUM)
    {
        AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: IPV6 address Colon number is larger than 7, return ERROR");
        return VOS_ERR;
    }

    if (ucColonCount == AT_MAX_IPV6_STR_COLON_NUM)
    {
        /* ��ѹ����ʽ���� */
        ulResult = AT_PcscfIpv6StrToAddr(pucStr, pucIpAddr, ucColonCount);
    }
    else
    {
        if (ucIdxPos != 0xFF)
        {
            /* ѹ����ʽ���� */
            AT_Ipv6LenStrToAddrProcCompressed(pucStr, ucColonCount, ucDotCount, ucStrlen, ucIdxPos);
            /* ӳ��IPV6��ַ��ʽ */
            ulResult = AT_PcscfIpv6StrToAddr(pucStr, pucIpAddr, AT_MAX_IPV6_STR_COLON_NUM);
        }
        /* ѹ��IPV6��ַ���Ҳ����������ڵ�ð�ţ���ʽ���� */
        else
        {
            AT_ERR_LOG("AT_ParseAddrFromPcscfIpv6Addr: Can not find two consecutive colons in compressed IPV6 address , return ERROR");
            return VOS_ERR;
        }
    }

    return ulResult;
}


VOS_UINT32  AT_Ipv6PcscfDataToAddr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr,
    VOS_UINT32                         *pulPortExistFlg,
    VOS_UINT32                         *pulPortNum

)
{
    VOS_UINT8                           pucStrTmp[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];

    memset_s(pucStrTmp, TAF_MAX_IPV6_ADDR_COLON_STR_LEN, 0, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);

    if ((pucStr == VOS_NULL_PTR)
     || (pucIpAddr == VOS_NULL_PTR)
     || (pulPortExistFlg == VOS_NULL_PTR)
     || (pulPortNum == VOS_NULL_PTR))
    {
        AT_NORM_LOG("AT_Ipv6PcscfDataToAddr: input ptr is NULL, return ERROR");
        return VOS_ERR;
    }

    if (AT_ParsePortFromPcscfIpv6Addr(pucStr, pucStrTmp, sizeof(pucStrTmp), pulPortExistFlg, pulPortNum) != VOS_OK)
    {
        AT_ERR_LOG("AT_Ipv6PcscfDataToAddr: AT_ParsePortFromPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    /* ��IPV6��ַ��ʽת��Ϊ��д */
    VOS_StrToUpper((VOS_CHAR*)pucStrTmp);

    if (AT_ParseAddrFromPcscfIpv6Addr(pucStrTmp, sizeof(pucStrTmp), pucIpAddr) != VOS_OK)
    {
        AT_ERR_LOG("AT_Ipv6PcscfDataToAddr: AT_ParseAddrFromPcscfIpv6Addr FAIL, return ERROR");
        return VOS_ERR;
    }

    return VOS_OK;
}



VOS_UINT32 AT_Ipv6AddrToStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    AT_IPV6_STR_TYPE_ENUM_UINT8         enIpStrType
)
{
    VOS_UINT8                          *pucBuffer = VOS_NULL_PTR;
    VOS_UINT16                          ausAddrValue[AT_IPV6_STR_MAX_TOKENS];
    VOS_UINT16                          usAddrNum;
    VOS_UINT8                           ucDelimiter;
    VOS_UINT8                           ucTokensNum;
    VOS_UINT8                           ucRadix;
    VOS_UINT32                          i;

    pucBuffer                           = aucAddrStr;

    memset_s(ausAddrValue, sizeof(ausAddrValue), 0x00, sizeof(ausAddrValue));

    /* ����IP�ַ�����ʽ�������, ���ö�Ӧ��ת������ */
    switch (enIpStrType)
    {
        case AT_IPV6_STR_TYPE_HEX:
            ucDelimiter = AT_IPV6_STR_DELIMITER;
            ucTokensNum = 8;
            ucRadix     = 16;
            break;

        case AT_IPV6_STR_TYPE_DEC:
            ucDelimiter = AT_IPV4_STR_DELIMITER;
            ucTokensNum = 16;
            ucRadix     = 10;
            break;

        default:
            return VOS_ERR;
    }

    /* ����IP�ַ�����ʽ�������, ��ȡ�ֶε�IP��ַ��ֵ */
    for (i = 0; i < ucTokensNum; i++)
    {
        usAddrNum = *aucIpAddr++;

        if (enIpStrType == AT_IPV6_STR_TYPE_HEX)
        {
            usAddrNum <<= 8;
            usAddrNum  |= *aucIpAddr++;
        }

        ausAddrValue[i] = usAddrNum;
    }

    /* ����IP��ַ�ֶ�, �����зָ�����ǵ�IP��ַ�ַ��� */
    for (i=0; i < ucTokensNum; i++)
    {
        pucBuffer    = (VOS_UINT8*)AT_Itoa(ausAddrValue[i],
                                               (VOS_CHAR*)pucBuffer,
                                               ucRadix,
                                               (TAF_MAX_IPV6_ADDR_DOT_STR_LEN - (VOS_UINT32)(aucAddrStr - pucBuffer)));
        *pucBuffer++ = ucDelimiter;
    }

    /* ȡ�����һ���ָ���, �����ַ��������� */
    if (aucAddrStr != pucBuffer)
    {
        *(--pucBuffer) = '\0';
    }

    return VOS_OK;
}


VOS_UINT32 AT_Ipv6Addr2DecString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[]
)
{
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulLoop;
    VOS_INT                             iRslt;

    ulLength = 0;

    /* ѭ����ӡ10���Ƶ��IPv6��ַ */
    for (ulLoop = 0; ulLoop < AT_IPV6_ADDR_DEC_TOKEN_NUM; ulLoop++)
    {
        /* ��ӡ��ָ��� */
        if (ulLoop != 0)
        {
            *(pcIpv6FormatStr + ulLength) = AT_IP_STR_DOT_DELIMITER;
            ulLength ++;
        }

        iRslt = VOS_sprintf_s(pcIpv6FormatStr + ulLength,
                              AT_IPV6_ADDR_DEC_FORMAT_STR_LEN - ulLength,
                              "%d",
                              aucIpv6Addr[ulLoop]);

        if (iRslt <= 0)
        {
            AT_WARN_LOG("AT_Ipv6Addr2DecString: Print IPv6 Addr Failed!");
            return 0;
        }

        ulLength += iRslt;
    }

    return ulLength;
}


VOS_VOID AT_ConvertIpv6AddrToHexAddrAndGetMaxZeroCnt(
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT16                          ausIpv6HexAddr[],
    VOS_UINT32                         *pulZeroStartIndex,
    VOS_UINT32                         *pulZeroMaxCnt
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulZeroTmpIndex;
    VOS_UINT32                          ulZeroTmpCnt;

    ulZeroTmpIndex      = 0;
    ulZeroTmpCnt        = 0;

    /* ѭ��ת��IPv6��ʽ��ַ����ͳ���������� */
    for (ulLoop = 0; ulLoop < AT_IPV6_ADDR_HEX_TOKEN_NUM; ulLoop++)
    {
        /* �ϲ��ֽ� */
        ausIpv6HexAddr[ulLoop] = *(aucIpv6Addr + ulLoop + ulLoop);
        ausIpv6HexAddr[ulLoop] <<= 8;
        ausIpv6HexAddr[ulLoop] |= *(aucIpv6Addr + ulLoop + ulLoop + 1);

        if (ausIpv6HexAddr[ulLoop] == 0)
        {
            /* ���16�����ֶ�Ϊ0�����¼��ʱcnt��Index */
            if (ulZeroTmpCnt == 0)
            {
                ulZeroTmpIndex = ulLoop;
            }

            ulZeroTmpCnt++;
        }
        else
        {
            /* ���16�����ֶβ�Ϊ0�����ж��Ƿ������������¼ */
            if (ulZeroTmpCnt > *pulZeroMaxCnt)
            {
                *pulZeroStartIndex    = ulZeroTmpIndex;
                *pulZeroMaxCnt        = ulZeroTmpCnt;
            }

            ulZeroTmpCnt    = 0;
            ulZeroTmpIndex  = 0;
        }
    }


    /* �ж��Ƿ������������¼ */
    if (ulZeroTmpCnt > *pulZeroMaxCnt)
    {
        *pulZeroStartIndex    = ulZeroTmpIndex;
        *pulZeroMaxCnt        = ulZeroTmpCnt;
    }

    return;
}


VOS_UINT32 AT_Ipv6Addr2HexString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[]
)
{
    VOS_UINT32                          ulLength;
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;
    VOS_CHAR                           *pcFormatStr         = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          ausIpv6HexAddr[AT_IPV6_ADDR_HEX_TOKEN_NUM];
    VOS_UINT32                          ulZeroStartIndex;
    VOS_UINT32                          ulZeroMaxCnt;
    VOS_INT                             iRslt;

    /* �ֲ�������ʼ�� */
    ulLength            = 0;
    pstCommPsCtx        = AT_GetCommPsCtxAddr();
    ulZeroStartIndex    = 0;
    ulZeroMaxCnt        = 0;
    memset_s(ausIpv6HexAddr, sizeof(ausIpv6HexAddr), 0, sizeof(ausIpv6HexAddr));

    /* �����Ƿ���ǰ����ȷ�ϴ�ӡ��ʽ */
    pcFormatStr = (VOS_FALSE == pstCommPsCtx->bitOpIpv6LeadingZeros) ? "%04X" : "%X";

    /* ת��IPv6��ַΪ16Bit HEX���ͣ���ͳ�������������� */
    AT_ConvertIpv6AddrToHexAddrAndGetMaxZeroCnt(aucIpv6Addr, ausIpv6HexAddr, &ulZeroStartIndex, &ulZeroMaxCnt);

    /* ѭ����ӡ16���Ƶ��IPv6��ַ */
    for (ulLoop = 0; ulLoop < AT_IPV6_ADDR_HEX_TOKEN_NUM; ulLoop++)
    {
        /* ������ѹ�����ܣ��Ҵ�����������㣬��ѹ����ӡ */
        if ( (pstCommPsCtx->bitOpIpv6CompressZeros != VOS_FALSE)
          && (ulZeroMaxCnt > 0) )
        {
            /* ��һ��0����ӡð�� */
            if (ulZeroStartIndex == ulLoop)
            {
                *(pcIpv6FormatStr + ulLength) = AT_IP_STR_COLON_DELIMITER;
                ulLength ++;
                continue;
            }

            /* ����0����ӡ */
            if ( (ulLoop > ulZeroStartIndex)
              && (ulLoop < (ulZeroStartIndex + ulZeroMaxCnt)) )
            {
                /* ���һλΪ0����Ҫ���ӡһ��ð�� */
                if (ulLoop == (AT_IPV6_ADDR_HEX_TOKEN_NUM - 1))
                {
                    *(pcIpv6FormatStr + ulLength) = AT_IP_STR_COLON_DELIMITER;
                    ulLength ++;
                }

                continue;
            }
        }

        /* ��ӡð�ŷָ��� */
        if (ulLoop != 0)
        {
            *(pcIpv6FormatStr + ulLength) = AT_IP_STR_COLON_DELIMITER;
            ulLength ++;
        }

        iRslt = VOS_sprintf_s(pcIpv6FormatStr + ulLength,
                              AT_IPV6_ADDR_COLON_FORMAT_STR_LEN - ulLength,
                              pcFormatStr,
                              ausIpv6HexAddr[ulLoop]);

        if (iRslt <= 0)
        {
            AT_WARN_LOG("AT_Ipv6Addr2HexString: Print IPv6 Addr Failed!");
            return 0;
        }

        ulLength += iRslt;
    }

    return ulLength;
}


VOS_UINT32 AT_Ipv6AddrMask2FormatString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT8                           aucIpv6Mask[]
)
{
    VOS_UINT32                          ulLength;
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;
    VOS_INT                             iRslt;

    /* �ֲ�������ʼ�� */
    ulLength        = 0;
    pstCommPsCtx    = AT_GetCommPsCtxAddr();
    iRslt           = 0;

    /* ����Ҫ��IPv6��ַ����������ӡIPv6�������� */
    if (aucIpv6Addr == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_Ipv6AddrMask2FormatString: No IPv6 Address!");
        *pcIpv6FormatStr = '\0';
        return 0;
    }

    if (pstCommPsCtx->bitOpIpv6AddrFormat == VOS_FALSE)
    {
        /* 10���Ƶ�ָ�ʽ��ӡIPv6��ַ */
        ulLength = AT_Ipv6Addr2DecString(pcIpv6FormatStr, aucIpv6Addr);

        /* ��ӡ�������� */
        if (aucIpv6Mask != VOS_NULL_PTR)
        {
            /* ʹ�õ�ָ�IP��ַ���������� */
            *(pcIpv6FormatStr + ulLength) = AT_IP_STR_DOT_DELIMITER;
            ulLength ++;

            /* 10���Ƶ�ָ�ʽ��ӡIPv6�������� */
            ulLength += AT_Ipv6Addr2DecString(pcIpv6FormatStr + ulLength, aucIpv6Mask);
        }
    }
    else
    {
        /* 16����ð�ŷָ���ʽ��ӡIPv6��ַ */
        ulLength = AT_Ipv6Addr2HexString(pcIpv6FormatStr, aucIpv6Addr);

        /* ��ӡ�������� */
        if (aucIpv6Mask != VOS_NULL_PTR)
        {
            /* �ж����������ʽ */
            if (pstCommPsCtx->bitOpIpv6SubnetNotation == VOS_FALSE)
            {
                /* �������������ַ��IPv6��ַͨ���ո�ָ� */
                /* ʹ�õ�ָ�IP��ַ���������� */
                *(pcIpv6FormatStr + ulLength) = ' ';
                ulLength ++;

                /* 10���Ƶ�ָ�ʽ��ӡIPv6�������� */
                ulLength += AT_Ipv6Addr2HexString(pcIpv6FormatStr + ulLength, aucIpv6Mask);
            }
            else
            {
                /* б�߷ָ�����ǰ׺��������IPv6��ַ */
                /* ʹ��б�߷ָ�IP��ַ���������� */
                *(pcIpv6FormatStr + ulLength) = '/';
                ulLength ++;

                iRslt = VOS_sprintf_s(pcIpv6FormatStr + ulLength,
                                      AT_IPV6_ADDR_MASK_FORMAT_STR_LEN - ulLength,
                                      "%d",
                                      AT_CalcIpv6PrefixLength(aucIpv6Mask, TAF_IPV6_ADDR_LEN));
                if (iRslt <= 0)
                {
                    AT_WARN_LOG("AT_Ipv6AddrMask2FormatString: Print IPv6 Subnet Failed!");
                    *pcIpv6FormatStr = '\0';
                    return 0;
                }

                ulLength += iRslt;
            }
        }
    }

    /* ��󲹳��ַ��������� */
    *(pcIpv6FormatStr + ulLength) = '\0';

    return ulLength;
}


VOS_UINT8 AT_CalcIpv6PrefixLength(VOS_UINT8 *pucLocalIpv6Mask,
                                          VOS_UINT32 ulIpv6MaskLen)
{
    VOS_UINT32                          i                   = 0;
    VOS_UINT8                           j                   = 0;
    VOS_UINT8                           ucMaskLength        = 0;

    if ( pucLocalIpv6Mask == VOS_NULL_PTR )
    {
        return ucMaskLength;
    }

    for(i = 0; i < TAF_MIN(AT_IPV6_STR_MAX_TOKENS, ulIpv6MaskLen); i++)
    {
        if (*(pucLocalIpv6Mask + i) == 0xFF)
        {
            ucMaskLength = ucMaskLength + 8;
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                if (((*(pucLocalIpv6Mask + i)) & ((VOS_UINT32)1 << (7 - j))) != 0)
                {
                    ucMaskLength ++;
                }
                else
                {
                    break;
                }
            }
            break;
        }
    }

    return ucMaskLength;
}


VOS_VOID AT_GetIpv6MaskByPrefixLength(
    VOS_UINT8                       ucLocalIpv6Prefix,
    VOS_UINT8                      *pucLocalIpv6Mask
)
{
    VOS_UINT8                           ucNum;
    VOS_UINT8                           i                       = 0;

    ucNum = ucLocalIpv6Prefix/8;

    if ( pucLocalIpv6Mask == VOS_NULL_PTR )
    {
        return;
    }

    for (i = 0; i < ucNum; i ++)
    {
        *(pucLocalIpv6Mask + i) = 0xFF;
    }

    ucNum = ucLocalIpv6Prefix % 8;

    if (ucNum != 0)
    {
        *(pucLocalIpv6Mask + i) = 0xFF & ((VOS_UINT32)0xFF << (8 - ucNum));

    }

    return;
}


VOS_UINT32 AT_FindIpv6AddrZeroFieldsToBeCompressed(
    VOS_UINT8                          *pucZeroFieldStart,
    VOS_UINT8                          *pucZeroFieldCount,
    VOS_UINT16                          ausAddrValue[],
    VOS_UINT8                           ucTokensNum
)
{
    VOS_UINT8                           ucStart;
    VOS_UINT8                           ucCount;
    VOS_UINT8                           i;

    ucStart                             = 0xFF;
    ucCount                             = 0;

    for (i = 0; i < ucTokensNum - 1; i++)
    {
        if ( (ausAddrValue[i] == 0x0000)
          && (ausAddrValue[i + 1] == 0x0000) )
        {
            /* ��¼��ֵ����Ϊ0��IP��ַ����ʼλ�� */
            if (ucStart == 0xFF)
            {
                ucStart = i;
            }

            /* ������ֵ����Ϊ0��IP��ַ�θ��� */
            ucCount++;
        }
        else
        {
            /* ���´�ѹ����IP��ַ��λ��, �Լ�IP��ַ�θ��� */
            if (ucStart != 0xFF)
            {
                if (ucCount > *pucZeroFieldCount)
                {
                    *pucZeroFieldStart = ucStart;
                    *pucZeroFieldCount = ucCount;
                }

                ucStart = 0xFF;
                ucCount = 0;
            }
        }
    }

    /* ��ֵ����Ϊ0��IP��ַ���ڽ�βʱ, ��Ҫ����һ�δ�ѹ����IP��ַ��λ��,
       �Լ�IP��ַ�θ��� */
    if (ucStart != 0xFF)
    {
        if (ucCount > *pucZeroFieldCount)
        {
            *pucZeroFieldStart = ucStart;
            *pucZeroFieldCount = ucCount;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_ConvertIpv6AddrToCompressedStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    VOS_UINT8                           ucTokensNum
)
{
    VOS_UINT8                          *pucBuffer = VOS_NULL_PTR;
    VOS_UINT16                          ausAddrValue[8]; /*TAF_IPV6_STR_RFC2373_TOKENS]; */
    VOS_UINT16                          usAddrNum;
    VOS_UINT8                           ucDelimiter;
    VOS_UINT8                           ucRadix;
    VOS_UINT8                           ucZeroFieldStart;
    VOS_UINT8                           ucZeroFieldCount;
    VOS_UINT8                           i;

    memset_s(ausAddrValue, sizeof(ausAddrValue), 0x00, sizeof(ausAddrValue));

    pucBuffer                           = aucAddrStr;
    ucDelimiter                         = TAF_IPV6_STR_DELIMITER;
    ucRadix                             = 16;
    ucZeroFieldStart                    = 0xFF;
    ucZeroFieldCount                    = 0;

    /* ����IP�ַ�����ʽ�������, ��ȡ�ֶε�IP��ַ��ֵ */
    for (i = 0; i < ucTokensNum; i++)
    {
        usAddrNum = *aucIpAddr++;

        usAddrNum <<= 8;
        usAddrNum  |= *aucIpAddr++;

        ausAddrValue[i] = usAddrNum;
    }

    /* �ҳ���Ҫʹ��"::"��ʾ��IP��ַ�ε���ʼλ��  */
    AT_FindIpv6AddrZeroFieldsToBeCompressed(&ucZeroFieldStart,
                                            &ucZeroFieldCount,
                                            ausAddrValue,
                                            ucTokensNum);

    /* ����IP��ַ�ֶ�, �����зָ�����ǵ�IP��ַ�ַ��� */
    for (i=0; i < ucTokensNum; i++)
    {
        if (ucZeroFieldStart == i)
        {
            *pucBuffer++ = ucDelimiter;

            i += ucZeroFieldCount;

            /* ����ѵ�IP��ַ�ֶε����һ��, ��Ҫ����ָ��� */
            if ((ucTokensNum - 1) == i)
            {
                *pucBuffer++ = ucDelimiter;
            }
        }
        else
        {
            /* �����IP��ַ�ֶεĵ�һ��, ����Ҫ����ָ��� */
            if (i != 0)
            {
                *pucBuffer++ = ucDelimiter;
            }
            pucBuffer    = (VOS_UINT8*)AT_Itoa(ausAddrValue[i],
                                               (VOS_CHAR*)pucBuffer,
                                               ucRadix,
                                               (TAF_MAX_IPV6_ADDR_COLON_STR_LEN - (VOS_UINT32)(aucAddrStr - pucBuffer)));
        }
    }

    /* �����ַ��������� */
    if (aucAddrStr != pucBuffer)
    {
        *pucBuffer = '\0';
    }

    return VOS_OK;
}


VOS_UINT32  AT_PortAtoI(
    VOS_CHAR                           *pcString,
    VOS_UINT32                         *pulValue
)
{
    VOS_CHAR                           *pucTmp = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    if ((pcString == VOS_NULL_PTR)
     || (pulValue == VOS_NULL_PTR))
    {
        AT_ERR_LOG("AT_PortAtoI: pcString or pulValue is NULL, return ERROR");
        return VOS_ERR;
    }

    pucTmp  = pcString;
    ulRet   = 0;

    *pulValue = 0;

    for (pucTmp = pcString; *pucTmp != '\0'; pucTmp++)
    {
        /* ������, �򷵻�ʧ��*/
        if ((*pucTmp < '0') || (*pucTmp > '9'))
        {
            AT_ERR_LOG("AT_PortAtoI: Not all number type in pcString , return ERROR");
            return VOS_ERR;
        }

        ulRet = (ulRet * 10) + (*pucTmp - '0');

        if (ulRet > IMS_PCSCF_ADDR_PORT_MAX)
        {
            AT_ERR_LOG("AT_PortAtoI: Port number is larger than 65535, return ERROR");
            return VOS_ERR;
        }
    }

    if ((ulRet > 0)
     && (ulRet <= IMS_PCSCF_ADDR_PORT_MAX))
    {
        *pulValue = (VOS_UINT32)ulRet;

        return VOS_OK;
    }

    AT_ERR_LOG("AT_PortAtoI: return ERROR");
    return VOS_ERR;
}


VOS_UINT64  AT_AtoI(
    VOS_CHAR                           *pString
)
{
    VOS_CHAR    *pcTmp = VOS_NULL_PTR;
    VOS_UINT64   ullRet;

    pcTmp  = pString;
    ullRet = 0;

    for (pcTmp = pString ; *pcTmp != '\0' ; pcTmp++)
    {
        /* ������,�򲻴���*/
        if ((*pcTmp < '0') || (*pcTmp > '9'))
        {
            continue;
        }

        ullRet = (ullRet * 10) + (*pcTmp - '0');
    }

    return ullRet;
}


VOS_CHAR* AT_Itoa(
    VOS_UINT16                          usValue,
    VOS_CHAR                           *pcStr,
    VOS_UINT16                          usRadix,
    VOS_UINT32                          ulLength
)
{
    if (usRadix == 16)
    {
        pcStr += VOS_sprintf_s(pcStr, ulLength, "%x", usValue);
    }
    else if(usRadix == 10)
    {
        pcStr += VOS_sprintf_s(pcStr, ulLength, "%d", usValue);
    }
    else
    {
    }
    return pcStr;
}


VOS_INT32  AT_AtoInt(
    VOS_CHAR                           *pString,
    VOS_INT32                          *pOut
)
{
    VOS_CHAR                           *pcTmp = VOS_NULL_PTR;
    VOS_INT32                           lFlag;     /* negative flag */

    pcTmp = pString;
    lFlag = 0;

    if (*pcTmp == '-')
    {
        lFlag = VOS_TRUE;
        pcTmp ++;
    }

    for (; *pcTmp != '\0' ; pcTmp++)
    {
        /* ������, ֱ�ӷ��ش��� */
        if ((*pcTmp < '0') || (*pcTmp > '9'))
        {
            return VOS_ERR;
        }

        *pOut = (*pOut * 10) + (*pcTmp - '0');
    }

    if (lFlag == VOS_TRUE)
    {
        *pOut = (0 - (*pOut));
    }

    return VOS_OK;
}


VOS_VOID AT_GetDhcpPara(
    AT_DHCP_PARA_STRU                  *pstConfig,
    TAF_IFACE_IPV4_DHCP_PARAM_STRU     *pstIpv4Dhcp
)
{
    /* ��DHCP����ת��Ϊ������ */
    pstConfig->stDhcpCfg.ulIPAddr     = VOS_HTONL(pstIpv4Dhcp->ulAddr);
    pstConfig->stDhcpCfg.ulSubNetMask = VOS_HTONL(pstIpv4Dhcp->ulNetMask);
    pstConfig->stDhcpCfg.ulGateWay    = VOS_HTONL(pstIpv4Dhcp->ulGateWay);
    pstConfig->stDhcpCfg.ulPrimDNS    = VOS_HTONL(pstIpv4Dhcp->ulPrimDNS);
    pstConfig->stDhcpCfg.ulSndDNS     = VOS_HTONL(pstIpv4Dhcp->ulSecDNS);

    return;
}


VOS_UINT16 AT_CalcIpHdrCRC16(
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usSize
)
{
    VOS_UINT16                         *pusBuffer = VOS_NULL_PTR;
    VOS_UINT32                          ulCheckSum;

    pusBuffer    = (VOS_UINT16 *)pucData;
    ulCheckSum = 0;

    while (usSize > 1)
    {
        ulCheckSum += *pusBuffer++;
        usSize     -= sizeof(VOS_UINT16);
    }

    if (usSize)
    {
#if (VOS_LITTLE_ENDIAN == VOS_BYTE_ORDER)
        ulCheckSum += *(VOS_UINT8 *)pusBuffer;
#else
        ulCheckSum += 0 | ((*(VOS_UINT8 *)pusBuffer) << 8);
#endif
    }

    ulCheckSum  = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
    ulCheckSum += (ulCheckSum >> 16);

    return (VOS_UINT16)(~ulCheckSum);
}


ATTRIBUTE_NO_SANITIZE_RUNTIME VOS_UINT32 AT_BuildUdpHdr(
    AT_UDP_PACKET_FORMAT_STRU          *pstUdpPkt,
    VOS_UINT16                          usLen,
    VOS_UINT32                          ulSrcAddr,
    VOS_UINT32                          ulDstAddr,
    VOS_UINT16                          usSrcPort,
    VOS_UINT16                          usDstPort
)
{
    static VOS_UINT16                   usIdentification = 0;

    /* ���ָ��Ϸ��� */
    if (pstUdpPkt == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    /* ��дIPͷ */
    pstUdpPkt->stIpHdr.ucIpVer          = AT_IP_VERSION;
    pstUdpPkt->stIpHdr.ucIpHdrLen       = 5;
    pstUdpPkt->stIpHdr.ucServiceType    = 0x00;
    pstUdpPkt->stIpHdr.usTotalLen       = VOS_HTONS(usLen + AT_IP_HDR_LEN + AT_UDP_HDR_LEN);
    pstUdpPkt->stIpHdr.usIdentification = VOS_HTONS(usIdentification);
    pstUdpPkt->stIpHdr.usOffset         = 0;
    pstUdpPkt->stIpHdr.ucTTL            = AT_IP_DEF_TTL;
    pstUdpPkt->stIpHdr.ucProtocol       = AT_IPPROTO_UDP;
    pstUdpPkt->stIpHdr.ulSrcAddr        = VOS_HTONL(ulSrcAddr);
    pstUdpPkt->stIpHdr.ulDstAddr        = VOS_HTONL(ulDstAddr);
    pstUdpPkt->stIpHdr.usCheckSum       = AT_CalcIpHdrCRC16((VOS_UINT8 *)&pstUdpPkt->stIpHdr, AT_IP_HDR_LEN);

    /* ��дUDPͷ */
    pstUdpPkt->stUdpHdr.usSrcPort       = VOS_HTONS(usSrcPort);
    pstUdpPkt->stUdpHdr.usDstPort       = VOS_HTONS(usDstPort);
    pstUdpPkt->stUdpHdr.usLen           = VOS_HTONS(usLen + AT_UDP_HDR_LEN);
    pstUdpPkt->stUdpHdr.usChecksum      = 0;

    usIdentification++;

    return VOS_OK;
}


VOS_UINT32 AT_GetDisplayRate(
    VOS_UINT16                          usClientId,
    AT_DISPLAY_RATE_STRU               *pstSpeed
)
{
    PPP_RATE_DISPLAY_ENUM_UINT32        enRateDisplay;
    AT_DISPLAY_RATE_STRU                stDialRateTmp;
    TAF_AGENT_SYS_MODE_STRU             stSysMode;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulDlSpeedLen      = 0;
    VOS_UINT32                          ulUlSpeedLen      = 0;
    VOS_UINT32                          ulNvDialRateIndex;
    errno_t                             lMemResult;
    VOS_UINT8                           ucSubSysMode;
    VOS_UINT8                           ucDlCategoryIndex = 0;
    VOS_UINT8                           ucUlCategoryIndex = 0;

    /* ������ʼ�� */
    memset_s(&stSysMode, sizeof(stSysMode), 0x00, sizeof(stSysMode));
    memset_s(&stDialRateTmp, sizeof(AT_DISPLAY_RATE_STRU), 0x00, sizeof(AT_DISPLAY_RATE_STRU));

    /* ��C�˻�ȡucRatType��ucSysSubMode */
    ulRet = TAF_AGENT_GetSysMode(usClientId, &stSysMode);

    if (ulRet != VOS_OK)
    {
        stSysMode.enRatType     = TAF_PH_INFO_GSM_RAT;
        stSysMode.enSysSubMode  = TAF_SYS_SUBMODE_BUTT;
    }

    ucSubSysMode = stSysMode.enSysSubMode;
    switch(ucSubSysMode)
    {
        case TAF_SYS_SUBMODE_GSM:
        case TAF_SYS_SUBMODE_GPRS:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucGprsConnectRate;
            break;

        case TAF_SYS_SUBMODE_EDGE:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucEdgeConnectRate;
            break;

        case TAF_SYS_SUBMODE_WCDMA:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucWcdmaConnectRate;
            break;

        case TAF_SYS_SUBMODE_HSDPA:
        case TAF_SYS_SUBMODE_HSDPA_HSUPA:
            ulNvDialRateIndex =    g_stDialConnectDisplayRate.ucDpaConnectRate;
            break;

        default:
            ulNvDialRateIndex = 0;
            break;
    }

    if ((ulNvDialRateIndex == 0) || (ulNvDialRateIndex > 6))
    {
        if ( (stSysMode.enRatType == TAF_PH_INFO_WCDMA_RAT)
          || (stSysMode.enRatType == TAF_PH_INFO_TD_SCDMA_RAT) )
        {
            enRateDisplay = AT_GetRateDisplayIndexForWcdma(&g_stAtDlRateCategory);

            ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            lMemResult = memcpy_s(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_PppDialRateDisplay[enRateDisplay], ulDlSpeedLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), ulDlSpeedLen);
            lMemResult = memcpy_s(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_PppDialRateDisplay[enRateDisplay], ulUlSpeedLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), ulUlSpeedLen);
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (stSysMode.enRatType == TAF_PH_INFO_LTE_RAT)
        {
            ucDlCategoryIndex = AT_GetLteUeDlCategoryIndex();
            ucUlCategoryIndex = AT_GetLteUeUlCategoryIndex();

            ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)(g_ucLTERateDisplay[ucDlCategoryIndex].acStrDlSpeed));
            lMemResult = memcpy_s(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_ucLTERateDisplay[ucDlCategoryIndex].acStrDlSpeed, ulDlSpeedLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), ulDlSpeedLen);
            ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)(g_ucLTERateDisplay[ucUlCategoryIndex].acStrUlSpeed));
            lMemResult = memcpy_s(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_ucLTERateDisplay[ucUlCategoryIndex].acStrUlSpeed, ulUlSpeedLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), ulUlSpeedLen);
        }
#endif
        else
        {
            enRateDisplay = AT_GetRateDisplayIndexForGsm(&stSysMode);

            ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)g_PppDialRateDisplay[enRateDisplay]);
            lMemResult = memcpy_s(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_PppDialRateDisplay[enRateDisplay], ulDlSpeedLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), ulDlSpeedLen);
            lMemResult = memcpy_s(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_PppDialRateDisplay[enRateDisplay], ulUlSpeedLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), ulUlSpeedLen);
        }
    }
    else
    {
        ulDlSpeedLen = VOS_StrLen((TAF_CHAR *)g_ucDialRateDisplayNv[ulNvDialRateIndex - 1]);
        ulUlSpeedLen = VOS_StrLen((TAF_CHAR *)g_ucDialRateDisplayNv[ulNvDialRateIndex - 1]);
        lMemResult = memcpy_s(stDialRateTmp.ucDlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), g_ucDialRateDisplayNv[ulNvDialRateIndex - 1], ulDlSpeedLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucDlSpeed), ulDlSpeedLen);
        lMemResult = memcpy_s(stDialRateTmp.ucUlSpeed, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), g_ucDialRateDisplayNv[ulNvDialRateIndex - 1], ulUlSpeedLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stDialRateTmp.ucUlSpeed), ulUlSpeedLen);
    }

    /*��CONNECT����������Ϣ*/
    lMemResult = memcpy_s(pstSpeed->ucDlSpeed, AT_AP_SPEED_STRLEN + 1, stDialRateTmp.ucDlSpeed, (VOS_UINT16)ulDlSpeedLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_AP_SPEED_STRLEN + 1, (VOS_UINT16)ulDlSpeedLen);
    lMemResult = memcpy_s(pstSpeed->ucUlSpeed, AT_AP_SPEED_STRLEN + 1, stDialRateTmp.ucUlSpeed, (VOS_UINT16)ulUlSpeedLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_AP_SPEED_STRLEN + 1, (VOS_UINT16)ulUlSpeedLen);
    pstSpeed->ucDlSpeed[ulDlSpeedLen] = '\0';
    pstSpeed->ucUlSpeed[ulUlSpeedLen] = '\0';

    return VOS_OK;
}


VOS_UINT32 AT_Get3gppSmCauseByPsCause(
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT32                          ul3gppSmCause;

    if ( (enCause >= TAF_PS_CAUSE_SM_NW_SECTION_BEGIN)
      && (enCause <= TAF_PS_CAUSE_SM_NW_SECTION_END) )
    {
        ul3gppSmCause = enCause - TAF_PS_CAUSE_SM_NW_SECTION_BEGIN;
    }
    /* E5�����翨��PDP DEACTIVEʱ�ϱ������36��ԭ��ֵ */
    else if (enCause == TAF_PS_CAUSE_SUCCESS)
    {
        ul3gppSmCause = TAF_PS_CAUSE_SM_NW_REGULAR_DEACTIVATION - TAF_PS_CAUSE_SM_NW_SECTION_BEGIN;
    }
    else
    {
        ul3gppSmCause = AT_NDISSTAT_ERR_UNKNOWN;
    }

    return ul3gppSmCause;
}


VOS_VOID AT_NDIS_ConnStatusChgProc(NCM_IOCTL_CONNECT_STUS_E enStatus)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg          = VOS_NULL_PTR;
    TAF_IFACE_DOWN_STRU                 stIfaceDown;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          i;

    if (enStatus == NCM_IOCTL_STUS_BREAK)
    {
        for (i = 1; i <= TAF_MAX_CID; i++)
        {
            pstChanCfg = AT_PS_GetDataChanlCfg(AT_NDIS_GET_CLIENT_ID(), (VOS_UINT8)i);

            if ((pstChanCfg->ulUsed == VOS_FALSE)
             || (pstChanCfg->enPortIndex >= AT_CLIENT_BUTT))
            {
                continue;
            }

            if (pstChanCfg->ulIfaceId == PS_IFACE_ID_NDIS0)
            {
                memset_s(&stIfaceDown, sizeof(stIfaceDown), 0x00, sizeof(stIfaceDown));
                memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));

                stIfaceDown.ucIfaceId   = (VOS_UINT8)pstChanCfg->ulIfaceId;
                stIfaceDown.enCause     = TAF_PS_CALL_END_CAUSE_NORMAL;
                stIfaceDown.enUserType  = TAF_IFACE_USER_TYPE_NDIS;

                /* ������ƽṹ�� */
                (VOS_VOID)AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, gastAtClientTab[pstChanCfg->enPortIndex].usClientId, 0, &stCtrl);
                (VOS_VOID)TAF_IFACE_Down(&stCtrl, &stIfaceDown);
            }
        }
    }

    return;
}


VOS_UINT32 AT_ResetFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT8                           ucRabIdIndex;
    VOS_UINT32                          ulRabIdMask;
    VOS_UINT32                          ulRet;
    FC_ID_ENUM_UINT8                    enFcId;
    MODEM_ID_ENUM_UINT16                enModemId;

    enFcId      = (FC_ID_ENUM_UINT8)ulParam2;
    enModemId   = g_stFcIdMaptoFcPri[enFcId].enModemId;
    ulRabIdMask = g_stFcIdMaptoFcPri[enFcId].ulRabIdMask;

    if (g_stFcIdMaptoFcPri[enFcId].ulUsed == VOS_TRUE)
    {
        for (ucRabIdIndex = AT_PS_MIN_RABID; ucRabIdIndex <= AT_PS_MAX_RABID; ucRabIdIndex++)
        {
            if (((ulRabIdMask >> ucRabIdIndex) & 0x1) == 1)
            {
#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
                FC_ChannelMapDelete(ucRabIdIndex, enModemId);
#endif
            }
        }

        ulRet = FC_DeRegPoint(enFcId, enModemId);
        if (ulRet != VOS_OK)
        {
            AT_ERR_LOG("AT_ResetFlowCtl: ERROR: DeReg point failed.");
            return VOS_ERR;
        }

        g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_FALSE;
        g_stFcIdMaptoFcPri[enFcId].enFcPri      = FC_PRI_BUTT;
        g_stFcIdMaptoFcPri[enFcId].ulRabIdMask  = 0;
        g_stFcIdMaptoFcPri[enFcId].enModemId    = MODEM_ID_BUTT;

    }

    return VOS_OK;
}


FC_PRI_ENUM_UINT8 AT_GetFCPriFromQos(
    const TAF_UMTS_QOS_STRU                *pstUmtsQos
)
{

    FC_PRI_ENUM_UINT8                   enFCPri;
    VOS_UINT8                           ucTrafficClass;

    /* ��ʼ�� */
    enFCPri         = FC_PRI_FOR_PDN_NONGBR;

    ucTrafficClass              = pstUmtsQos->ucTrafficClass;

    /* ����QOS trafficClass������ȡQCI */
    if (ucTrafficClass == AT_QOS_TRAFFIC_CLASS_CONVERSATIONAL)
    {
        enFCPri = FC_PRI_FOR_PDN_GBR;
    }
    else if (ucTrafficClass == AT_QOS_TRAFFIC_CLASS_STREAMING)
    {
        enFCPri = FC_PRI_FOR_PDN_GBR;
    }
    else if (ucTrafficClass == AT_QOS_TRAFFIC_CLASS_INTERACTIVE)
    {
        enFCPri = FC_PRI_FOR_PDN_NONGBR;
    }
    else
    {
        enFCPri = FC_PRI_FOR_PDN_LOWEST;
    }

    return enFCPri;
}


VOS_VOID  AT_ModemPsRspPdpDeactivatedEvtProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    VOS_UINT32                          ulATHCmdFlg;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    pstLineCtrl = AT_GetUartLineCtrlInfo();
    usLength    = 0;
    ulResult    = AT_FAILURE;
    ulATHCmdFlg = (AT_CMD_H_PS_SET == AT_PS_GET_CURR_CMD_OPT(ucIndex)) ?
                  VOS_TRUE : VOS_FALSE;

    /* ȥע��Modem�˿ڵ����ص� */
    AT_DeRegModemPsDataFCPoint(ucIndex, pstEvent->ucRabId);

    if (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
    {
        /* �ͷ�PPPʵ�� & HDLCȥʹ�� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        if (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
        {
            /* �������ⵥAT2D13296���ڱ���ȥ�����龰�£���PPP������
               PPP_AT_CTRL_REL_PPP_REQ�󣬲�������������̬�����ǵ�
               ��PPP��ӦAT_PPP_PROTOCOL_REL_IND_MSG֮������������̬ */
            /* ��������ʱ�������ڵȴ�PPP��ӦAT_PPP_PROTOCOL_REL_IND_MSG */
            AT_STOP_TIMER_CMD_READY(ucIndex);

            if (At_StartTimer(AT_PPP_PROTOCOL_REL_TIME, ucIndex) != AT_SUCCESS)
            {
                AT_ERR_LOG("At_UsbModemStatusPreProc:ERROR:Start Timer fail");
            }

            /* ���õ�ǰ�������� */
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_WAIT_PPP_PROTOCOL_REL_SET;

            return;
        }
    }
    else if (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
    {
        /* �ͷ�PPPʵ�� & HDLCȥʹ�� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);
    }
    else
    {
        /* ��������ģʽ�����ô��� */
        AT_WARN_LOG("TAF_PS_EVT_PDP_DEACTIVATED OTHER MODE");
    }

    /* ��������ģʽ */
    At_SetMode(ucIndex, AT_CMD_MODE,AT_NORMAL_MODE);

    if (ulATHCmdFlg == VOS_TRUE)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_NO_CARRIER;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    /* ATH�Ͽ�PPP����ʱ, ���DCD�ź�ģʽΪCONNECT ON, ��Ҫ����DCD�ź� */
    if ( (ulResult == AT_OK)
      && (pstLineCtrl->enDcdMode == AT_UART_DCD_MODE_CONNECT_ON) )
    {
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

    return;
}


VOS_VOID  AT_ModemPsRspPdpActEvtRejProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    ulResult                            = AT_FAILURE;
    usLength                            = 0;

    if ( (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_D_PPP_CALL_SET)
      || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_PPP_ORG_SET) )
    {
        ulResult = AT_NO_CARRIER;
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);

        /* ��PPP����HDLCȥʹ�ܲ��� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        /* ��������ģʽ */
        At_SetMode(ucIndex,AT_CMD_MODE,AT_NORMAL_MODE);

    }
    else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_D_IP_CALL_SET)
    {
        ulResult = AT_ERROR;
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

        /* ��PPP����HDLCȥʹ�ܲ��� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);


    }
    else
    {
        ulResult = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    return;
}


VOS_VOID AT_FillPppIndConfigInfoPara(
    AT_PPP_IND_CONFIG_INFO_STRU        *pstPppIndConfigInfo,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    errno_t                             lMemResult;
    /* ��дIP��ַ */
    lMemResult = memcpy_s(pstPppIndConfigInfo->aucIpAddr,
                          sizeof(pstPppIndConfigInfo->aucIpAddr),
                          pstEvent->stPdpAddr.aucIpv4Addr,
                          TAF_IPV4_ADDR_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstPppIndConfigInfo->aucIpAddr), TAF_IPV4_ADDR_LEN);

    /* ��дDNS��ַ */
    if (pstEvent->stDns.bitOpPrimDnsAddr == VOS_TRUE)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpPriDns = VOS_TRUE;

        lMemResult = memcpy_s(pstPppIndConfigInfo->stPcoIpv4Item.aucPriDns,
                              sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucPriDns),
                              pstEvent->stDns.aucPrimDnsAddr,
                              TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucPriDns), TAF_IPV4_ADDR_LEN);
    }

    if (pstEvent->stDns.bitOpSecDnsAddr == VOS_TRUE)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpSecDns = VOS_TRUE;

        lMemResult = memcpy_s(pstPppIndConfigInfo->stPcoIpv4Item.aucSecDns,
                              sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucSecDns),
                              pstEvent->stDns.aucSecDnsAddr,
                              TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucSecDns), TAF_IPV4_ADDR_LEN);

    }

    /* ��дNBNS��ַ */
    if (pstEvent->stNbns.bitOpPrimNbnsAddr == VOS_TRUE)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpPriNbns = VOS_TRUE;

        lMemResult = memcpy_s(pstPppIndConfigInfo->stPcoIpv4Item.aucPriNbns,
                              sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucPriNbns),
                              pstEvent->stNbns.aucPrimNbnsAddr,
                              TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucPriNbns), TAF_IPV4_ADDR_LEN);
    }

    if (pstEvent->stNbns.bitOpSecNbnsAddr == VOS_TRUE)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpSecNbns = VOS_TRUE;

        lMemResult = memcpy_s(pstPppIndConfigInfo->stPcoIpv4Item.aucSecNbns,
                              sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucSecNbns),
                              pstEvent->stNbns.aucSecNbnsAddr,
                              TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucSecNbns), TAF_IPV4_ADDR_LEN);
    }

    /* ��дGATE WAY��ַ */
    if (pstEvent->stGateWay.bitOpGateWayAddr == VOS_TRUE)
    {
        pstPppIndConfigInfo->stPcoIpv4Item.bitOpGateWay = VOS_TRUE;

        lMemResult = memcpy_s(pstPppIndConfigInfo->stPcoIpv4Item.aucGateWay,
                              sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucGateWay),
                              pstEvent->stGateWay.aucGateWayAddr,
                              TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstPppIndConfigInfo->stPcoIpv4Item.aucGateWay), TAF_IPV4_ADDR_LEN);
    }


}


VOS_UINT32 AT_RegModemPsDataFCPoint(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    VOS_UINT32                          ulRet;
    FC_PRI_ENUM_UINT8                   enFcPri;
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, Begin */
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_UART_CTX_STRU                   *pstUartCtx = VOS_NULL_PTR;

    pstUartCtx = AT_GetUartCtxAddr();

    /* UART�˿����عر�ʱ��ע�����ص� */
    if ( (AT_CheckHsUartUser(ucIndex) == VOS_TRUE)
      && (pstUartCtx->stFlowCtrl.enDteByDce == AT_UART_FC_DTE_BY_DCE_NONE) )
    {
        return VOS_ERR;
    }

    enModemId = MODEM_ID_0;

    memset_s(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

    ulRet = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstEvent->stCtrl.usClientId, &enModemId);

    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemPsDataFCPoint: Get modem id fail.");
        return VOS_ERR;
    }

#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    /* ����ͨ����RABIDӳ���ϵ */
    FC_ChannelMapCreate(enFcId, pstEvent->ucRabId, enModemId);
#endif

    stRegFcPoint.enFcId             = enFcId;

    /* ����������������ȼ�RAB QoS���ȼ�������,���ȼ��ı�ʱ����Ҫ�ı����ȼ� */
    /*  FC_PRI_3        ��������ȼ��ĳ���
        FC_PRI_4        ��NONGBR����
        FC_PRI_5        ��GBR���� */
    if (pstEvent->bitOpUmtsQos == TAF_USED)
    {
        enFcPri = AT_GetFCPriFromQos(&pstEvent->stUmtsQos);
    }
    else
    {
        enFcPri = FC_PRI_FOR_PDN_NONGBR;
    }
    stRegFcPoint.enFcPri            = enFcPri;
    stRegFcPoint.enPolicyId         = FC_POLICY_ID_MEM;
#if (FEATURE_AT_HSUART == FEATURE_ON)
    stRegFcPoint.pClrFunc           = (AT_CLIENT_TAB_HSUART_INDEX == ucIndex) ?
                                      AT_HSUART_StopFlowCtrl : AT_MODEM_StopFlowCtrl;
    stRegFcPoint.pSetFunc           = (AT_CLIENT_TAB_HSUART_INDEX == ucIndex) ?
                                      AT_HSUART_StartFlowCtrl : AT_MODEM_StartFlowCtrl;
#else
    stRegFcPoint.pClrFunc           = AT_MODEM_StopFlowCtrl;
    stRegFcPoint.pSetFunc            = AT_MODEM_StartFlowCtrl;

#endif
    stRegFcPoint.ulParam1           = (VOS_UINT32)g_alAtUdiHandle[ucIndex];
    stRegFcPoint.enModemId          = enModemId;
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, End */
    stRegFcPoint.ulParam2           = enFcId;
    stRegFcPoint.pRstFunc           = AT_ResetFlowCtl;

    /* ע�����ص�,��Ҫ�ֱ�ע��MEM,CPU,CDS��GPRS�� */
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: FC RegPoint MEM Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CPU_A;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: FC RegPoint A CPU Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CDS;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_GPRS;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: FC RegPoint GPRS Failed.");
        return VOS_ERR;
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CDMA;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemFCPoint: ERROR: reg CDMA point Failed.");
        return VOS_ERR;
    }
#endif

    /* ����FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulUsed      = VOS_TRUE;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enFcPri     = enFcPri;
    /* ��һ�������϶��RABID�������������Ҫ�����RABID��¼���� */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulRabIdMask |= ((VOS_UINT32)1 << (pstEvent->ucRabId));
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enModemId   = enModemId;

    /* ��������Ϣ */
    AT_MNTN_TraceRegFcPoint(ucIndex, AT_FC_POINT_TYPE_MODEM_PS);

    return VOS_OK;
}


VOS_UINT32 AT_DeRegModemPsDataFCPoint(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucRabId
)
{
    VOS_UINT32                          ulRet;
    /* Modified by l60609 for DSDA Phase II, 2012-12-28, Begin */
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_UART_CTX_STRU                   *pstUartCtx = VOS_NULL_PTR;

    pstUartCtx = AT_GetUartCtxAddr();

    /* UART�˿����عر�ʱ��ע�����ص� */
    if ( (AT_CheckHsUartUser(ucIndex) == VOS_TRUE)
        && (pstUartCtx->stFlowCtrl.enDteByDce == AT_UART_FC_DTE_BY_DCE_NONE) )
    {
        return VOS_ERR;
    }

    enModemId = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_DeRegModemPsDataFCPoint: Get modem id fail.");
        return VOS_ERR;
    }
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, Begin */
#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    /* ɾ������ģ��ӳ���ϵ */
    FC_ChannelMapDelete(ucRabId, enModemId);
#endif

    ulRet = FC_DeRegPoint(FC_ID_MODEM, enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_DeRegModemPsDataFCPoint: ERROR: FC DeRegPoint Failed.");
        return VOS_ERR;
    }
    /* Modified by l60609 for DSDA Phase II, 2012-12-21, End */

    /* ���FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulUsed      = VOS_FALSE;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enFcPri     = FC_PRI_BUTT;
    /* ��һ�������϶��RABID�������������Ҫ����Ӧ��RABID��������� */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulRabIdMask &= ~((VOS_UINT32)1 << ucRabId);
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enModemId   = MODEM_ID_BUTT;

    /* ��������Ϣ */
    AT_MNTN_TraceDeregFcPoint(ucIndex, AT_FC_POINT_TYPE_MODEM_PS);

    return VOS_OK;
}


VOS_VOID AT_ModemPsRspPdpActEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_PPP_IND_CONFIG_INFO_STRU         stPppIndConfigInfo;

    /* ע��Modem�˿ڵ����ص� */
    AT_RegModemPsDataFCPoint(ucIndex, pstEvent, FC_ID_MODEM);

    /* ��ʼ�� */
    memset_s(&stPppIndConfigInfo, sizeof(stPppIndConfigInfo),
             0x00, sizeof(AT_PPP_IND_CONFIG_INFO_STRU));

    /* �������������� */
    AT_STOP_TIMER_CMD_READY(ucIndex);

#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    gastAtClientTab[ucIndex].ucIfaceId = PS_IFACE_ID_PPP0;
#endif

    if (pstEvent->stPdpAddr.enPdpType == TAF_PDP_PPP)
    {
        At_SetMode(ucIndex, AT_DATA_MODE, AT_IP_DATA_MODE);

        /* ע���������ݷ��ͺ��� */
        Ppp_RegDlDataCallback(gastAtClientTab[ucIndex].usPppId);

        At_FormatResultData(ucIndex, AT_CONNECT);
    }
    else if (pstEvent->stPdpAddr.enPdpType == TAF_PDP_IPV4)
    {
        /* ��дIP��ַ, DNS, NBNS */
        AT_FillPppIndConfigInfoPara(&stPppIndConfigInfo, pstEvent);

        /* ��AUTH��IPCP֡����PPP����: */
        Ppp_RcvConfigInfoInd(gastAtClientTab[ucIndex].usPppId, &stPppIndConfigInfo);
    }
    else
    {
        /* �������Ͳ������� */
    }

    return;
}


VOS_VOID  AT_ModemPsRspPdpDeactEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    VOS_UINT32                          ulModemUsrFlg;
    VOS_UINT32                          ulATHCmdFlg;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    pstLineCtrl     = AT_GetUartLineCtrlInfo();
    ulModemUsrFlg   = AT_CheckModemUser(ucIndex);
    usLength        = 0;
    ulResult        = AT_FAILURE;
    ulATHCmdFlg     = (AT_CMD_H_PS_SET == gastAtClientTab[ucIndex].CmdCurrentOpt) ?
                      VOS_TRUE : VOS_FALSE;

    if ( (gastAtClientTab[ucIndex].UserType != AT_MODEM_USER)
      && (gastAtClientTab[ucIndex].UserType != AT_HSUART_USER) )
    {
        return;
    }

    if (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
    {
        /* �ͷ�PPPʵ�� & HDLCȥʹ�� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);
    }
    else if (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
    {
        /* �ͷ�PPPʵ�� & HDLCȥʹ�� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

        /* ��PPP����HDLCȥʹ�ܲ��� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);
    }
    else
    {
        /* ��������ģʽ�����ô��� */
        AT_WARN_LOG("TAF_PS_EVT_PDP_DEACTIVE_CNF OTHER MODE");
    }

    /* ȥע��Modem�˿ڵ����ص� */
    AT_DeRegModemPsDataFCPoint(ucIndex, pstEvent->ucRabId);

    /* ��������ģʽ */
    At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

    /* Ϊ�˹��Linux��̨���β���ʧ�����⣬�������¹�ܷ���:
       PDP�����Ͽ����̽����󣬽��յ�TAF_PS_EVT_PDP_DEACTIVE_CNF�¼����жϲ���
       ϵͳ�Ƿ�ΪLinux����������ԭ�����̴������ǣ����ٷ���"NO CARRIER" */
    if ((DRV_GET_LINUXSYSTYPE() == VOS_OK) && (ulModemUsrFlg == VOS_TRUE))
    {
        /* ԭ�������У���At_FormatResultData�����ڲ���������"NO CARRIER"֮��
           ��Ҫ��DCD���ͣ��ڴ˹�ܷ����У�������"NO CARRIER"����DCD�źŵ�����
           �����Ծ���Ҫ����
        */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
        return;
    }

    if (ulATHCmdFlg == VOS_TRUE)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_NO_CARRIER;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    /* ATH�Ͽ�PPP����ʱ, ���DCD�ź�ģʽΪCONNECT ON, ��Ҫ����DCD�ź� */
    if ( (ulResult == AT_OK)
      && (pstLineCtrl->enDcdMode == AT_UART_DCD_MODE_CONNECT_ON) )
    {
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

    return;
}


VOS_VOID AT_MODEM_ProcCallEndCnfEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_END_CNF_STRU           *pstEvent
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    AT_DCE_MSC_STRU                     stDecMsc;
    VOS_UINT32                          ulModemUsrFlg;
    VOS_UINT32                          ulATHCmdFlg;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    pstLineCtrl     = AT_GetUartLineCtrlInfo();
    ulModemUsrFlg   = AT_CheckModemUser(ucIndex);
    ulResult        = AT_FAILURE;
    usLength        = 0;
    ulATHCmdFlg     = (AT_CMD_H_PS_SET == AT_PS_GET_CURR_CMD_OPT(ucIndex)) ?
                      VOS_TRUE : VOS_FALSE;

    memset_s(&stDecMsc, sizeof(stDecMsc), 0x00, sizeof(AT_DCE_MSC_STRU));

    /* ��鵱ǰ�û��Ĳ������� */
    if ( (AT_PS_GET_CURR_CMD_OPT(ucIndex) != AT_CMD_PS_DATA_CALL_END_SET)
      && (AT_PS_GET_CURR_CMD_OPT(ucIndex) != AT_CMD_H_PS_SET) )
    {
        return;
    }

    /* PS��������ڴ���Ͽ�����, ֱ�ӷ��� */
    if (pstEvent->enCause == TAF_ERR_NO_ERROR)
    {
        return;
    }

    /* �ͷ�PPPʵ�� & HDLCȥʹ�� */
    if (AT_PS_GET_CURR_DATA_MODE(ucIndex) == AT_PPP_DATA_MODE)
    {
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_REL_PPP_REQ);
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_HDLC_DISABLE);
    }
    else if (AT_PS_GET_CURR_DATA_MODE(ucIndex) == AT_IP_DATA_MODE)
    {
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_REL_PPP_RAW_REQ);
        PPP_RcvAtCtrlOperEvent(AT_PS_GET_PPPID(ucIndex), PPP_AT_CTRL_HDLC_DISABLE);
    }
    else
    {
        ;
    }

    /* ��������ģʽ */
    At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

    /* Ϊ�˹��Linux��̨���β���ʧ�����⣬�������¹�ܷ���:
       PDP�����Ͽ����̽����󣬽��յ�TAF_PS_EVT_PDP_DEACTIVE_CNF�¼����жϲ���
       ϵͳ�Ƿ�ΪLinux����������ԭ�����̴������ǣ����ٷ���"NO CARRIER" */
    if ((DRV_GET_LINUXSYSTYPE() == VOS_OK) && (ulModemUsrFlg == VOS_TRUE))
    {
        /* ԭ�������У���At_FormatResultData�����ڲ���������"NO CARRIER"֮��
           ��Ҫ��DCD���ͣ��ڴ˹�ܷ����У�������"NO CARRIER"����DCD�źŵ�����
           �����Ծ���Ҫ����
        */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
        return;
    }

    if (ulATHCmdFlg == VOS_TRUE)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_NO_CARRIER;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    /* ATH�Ͽ�PPP����ʱ, ���DCD�ź�ģʽΪCONNECT ON, ��Ҫ����DCD�ź� */
    if ( (ulResult == AT_OK)
      && (pstLineCtrl->enDcdMode == AT_UART_DCD_MODE_CONNECT_ON) )
    {
        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

    return;
}


VOS_VOID  AT_AnswerPdpActInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    VOS_UINT16                          usPppId;
    VOS_UINT32                          ulRslt;

    usPppId     = 0;

    if (pstEvent->stPdpAddr.enPdpType == TAF_PDP_PPP)
    {
        if (Ppp_CreateRawDataPppReq(&usPppId) != VOS_OK)
        {
            ulRslt = AT_ERROR;
        }
        else
        {
            /* ��¼PPP id��Index�Ķ�Ӧ��ϵ */
            gastAtPppIndexTab[usPppId]          = ucIndex;

            /* ����PPP id */
            gastAtClientTab[ucIndex].usPppId    = usPppId;


            /* ע��Modem�˿ڵ����ص� */
            AT_RegModemPsDataFCPoint(ucIndex, pstEvent, FC_ID_MODEM);

            /* �л�����ͨ�� */
            At_SetMode(ucIndex, AT_DATA_MODE, AT_IP_DATA_MODE);

            /* ע���������ݷ��ͺ��� */
            Ppp_RegDlDataCallback(usPppId);

            ulRslt = AT_CONNECT;
        }

        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    else
    {
        /* ��������....*/
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_CONNECT);
    }

    return;
}


VOS_UINT32 At_RcvTeConfigInfoReq(
    VOS_UINT16                          usPppId,
    AT_PPP_REQ_CONFIG_INFO_STRU        *pstPppReqConfigInfo
)
{
    if (usPppId >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_RcvTeConfigInfoReq usPppId Wrong");
        return AT_FAILURE;
    }

    if (pstPppReqConfigInfo == TAF_NULL_PTR)
    {
        AT_WARN_LOG("At_RcvTeConfigInfoReq pPppReqConfigInfo NULL");
        return AT_FAILURE;
    }

    if (gastAtClientTab[gastAtPppIndexTab[usPppId]].CmdCurrentOpt != AT_CMD_D_PPP_CALL_SET)
    {
        AT_WARN_LOG("At_RcvTeConfigInfoReq NOT AT_CMD_D_PPP_CALL_SET");
        return AT_FAILURE;
    }

    if ( TAF_PS_PppDialOrig(WUEPS_PID_AT,
                                      AT_PS_BuildExClientId(gastAtClientTab[gastAtPppIndexTab[usPppId]].usClientId),
                                      0,
                                      gastAtClientTab[gastAtPppIndexTab[usPppId]].ucCid,
                                      (TAF_PPP_REQ_CONFIG_INFO_STRU *)pstPppReqConfigInfo) == VOS_OK )
    {
        /* ����ʱ�� */
        if (At_StartTimer(AT_ACT_PDP_TIME, gastAtPppIndexTab[usPppId]) != AT_SUCCESS)
        {
            AT_ERR_LOG("At_RcvTeConfigInfoReq:ERROR:Start Timer");
            return AT_FAILURE;
        }

        /* ���õ�ǰ�������� */
        gastAtClientTab[gastAtPppIndexTab[usPppId]].CmdCurrentOpt = AT_CMD_PPP_ORG_SET;

        return AT_SUCCESS;
    }
    else
    {
        return AT_FAILURE;
    }
}


VOS_UINT32 At_RcvPppReleaseInd(
    VOS_UINT16                          usPppId
)
{
    TAF_UINT8                           aucEventInfo[4];
    AT_PPP_RELEASE_IND_MSG_STRU        *pMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    if (usPppId >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_RcvPppReleaseInd usPppId Wrong");
        return AT_FAILURE;
    }

    /* EVENT- At_RcvPppReleaseInd:usPppId / gastAtPppIndexTab[usPppId] */
    aucEventInfo[0] = (TAF_UINT8)(usPppId >> 8);
    aucEventInfo[1] = (TAF_UINT8)usPppId;
    aucEventInfo[2] = gastAtPppIndexTab[usPppId];
    AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_RELEASE_PPP,
                   aucEventInfo, (VOS_UINT32)sizeof(aucEventInfo));

    /* ��ATģ�鷢��AT_PPP_RELEASE_IND_MSG */
    ulLength       = sizeof(AT_PPP_RELEASE_IND_MSG_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    pMsg = (AT_PPP_RELEASE_IND_MSG_STRU *)PS_ALLOC_MSG(PS_PID_APP_PPP, ulLength);/*lint !e830*/
    /*lint -restore */
    if (pMsg == VOS_NULL_PTR)
    {
        /* ��ӡ������Ϣ---������Ϣ��ʧ�� */
        AT_ERR_LOG( "At_RcvPppReleaseInd:ERROR:Allocates a message packet for AT_PPP_RELEASE_IND_MSG_STRU msg FAIL!" );
        return AT_FAILURE;
    }
    /* ��д��Ϣͷ */
    pMsg->MsgHeader.ulSenderCpuId   = VOS_LOCAL_CPUID;
    pMsg->MsgHeader.ulSenderPid     = PS_PID_APP_PPP;
    pMsg->MsgHeader.ulReceiverCpuId = VOS_LOCAL_CPUID;
    pMsg->MsgHeader.ulReceiverPid   = WUEPS_PID_AT;
    pMsg->MsgHeader.ulLength        = ulLength;

    pMsg->MsgHeader.ulMsgName       = AT_PPP_RELEASE_IND_MSG;
    /* ��д��Ϣ�� */
    pMsg->ucIndex                   = gastAtPppIndexTab[usPppId];

    /* ���͸���Ϣ */
    if (PS_SEND_MSG(PS_PID_APP_PPP, pMsg) != VOS_OK)
    {
        /* ��ӡ������Ϣ---������Ϣʧ�� */
        AT_WARN_LOG( "At_RcvPppReleaseInd:WARNING:SEND AT_PPP_RELEASE_IND_MSG msg FAIL!" );
        return AT_FAILURE;
    }
    else
    {
        /* ��ӡ������Ϣ---��������Ϣ */
        AT_WARN_LOG( "At_RcvPppReleaseInd:NORMAL:SEND AT_PPP_RELEASE_IND_MSG Msg" );
        return AT_SUCCESS;
    }
}


VOS_VOID At_PppReleaseIndProc(
    VOS_UINT8                           ucIndex
)
{
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_PppReleaseIndProc:ERROR:ucIndex is abnormal!");
        return;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_WAIT_PPP_PROTOCOL_REL_SET)
    {
        /* ��ATͨ���л�Ϊ����ģʽ */
        At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

        /*ֹͣ��ʱ��*/
        AT_STOP_TIMER_CMD_READY(ucIndex);

        /*�ظ�NO CARRIER*/
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_NO_CARRIER);

        return;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_PS_DATA_CALL_END_SET)
    {
        return;
    }

    if ( TAF_PS_CallEnd(WUEPS_PID_AT,
                                  AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                  0,
                                  gastAtClientTab[ucIndex].ucCid) == VOS_OK )
    {
        /* ����ʱ�� */
        if (At_StartTimer(AT_DETACT_PDP_TIME, ucIndex) != AT_SUCCESS)
        {
            AT_ERR_LOG("At_PppReleaseIndProc:ERROR:Start Timer");
            return;
        }

        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
    }

    return;
}


TAF_UINT32 At_PsRab2PppId(
    TAF_UINT8                           ucExRabId,
    TAF_UINT16                         *pusPppId
)
{
    TAF_UINT16                          usPppId;
    TAF_UINT8                           ucIndex;

    if (pusPppId == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_PsRab2PppId, pusPppId NULL");
        return TAF_FAILURE;
    }

    /* ͨ��PPP ID������ң�Ч�ʻ�Ƚϸ� */
    for (usPppId = 1; usPppId <= PPP_MAX_ID_NUM; usPppId++)
    {
        ucIndex = gastAtPppIndexTab[usPppId];

        if (ucIndex >= AT_MAX_CLIENT_NUM)
        {
            continue;
        }

        if ( gastAtClientTab[ucIndex].ucUsed != AT_CLIENT_USED )
        {
            continue;
        }

        if (gastAtClientTab[ucIndex].Mode != AT_DATA_MODE)
        {
            continue;
        }

        if ( (gastAtClientTab[ucIndex].DataMode != AT_PPP_DATA_MODE)
          && (gastAtClientTab[ucIndex].DataMode != AT_IP_DATA_MODE) )
        {
            continue;
        }

        if (gastAtClientTab[ucIndex].ucExPsRabId == ucExRabId)
        {
            *pusPppId = usPppId;    /* ���ؽ�� */
            return TAF_SUCCESS;
        }

    }

    AT_LOG1("AT, At_PsRab2PppId, WARNING, Get PppId from Rab <1> Fail", ucExRabId);

    return TAF_FAILURE;
} /* At_PsRab2PppId */



TAF_UINT32 At_PppId2PsRab(
    TAF_UINT16                          usPppId,
    TAF_UINT8                          *pucExRabId
)
{
    TAF_UINT8                           ucIndex;

    if ((usPppId < 1) || (usPppId > PPP_MAX_ID_NUM))
    {
        TAF_LOG1(WUEPS_PID_AT, 0, PS_LOG_LEVEL_WARNING,
            "AT, At_PppId2PsRab, WARNING, PppId <1> Wrong", usPppId);
        return TAF_FAILURE;
    }

    if (pucExRabId == TAF_NULL_PTR)
    {
        AT_WARN_LOG("AT, At_PppId2PsRab, WARNING, pucRabId NULL");
        return TAF_FAILURE;
    }

    ucIndex = gastAtPppIndexTab[usPppId];

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, index <1> Wrong", ucIndex);
        return TAF_FAILURE;
    }

    if (gastAtClientTab[ucIndex].Mode != AT_DATA_MODE)
    {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, Mode <1> Wrong", gastAtClientTab[ucIndex].Mode);
        return TAF_FAILURE;
    }

    if ( (gastAtClientTab[ucIndex].DataMode != AT_PPP_DATA_MODE)
      && (gastAtClientTab[ucIndex].DataMode != AT_IP_DATA_MODE) )
    {
        AT_LOG1("AT, At_PppId2PsRab, WARNING, DataMode <1> Wrong", gastAtClientTab[ucIndex].DataMode);
        return TAF_FAILURE;
    }

    *pucExRabId = gastAtClientTab[ucIndex].ucExPsRabId;

    return TAF_SUCCESS;
} /* At_PppId2PsRab */

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)

VOS_UINT32 At_IfaceId2PppId(
    VOS_UINT8                           ucIfaceId,
    VOS_UINT16                          *pusPppId
)
{
    VOS_UINT16                          usPppId;
    VOS_UINT8                           ucIndex;

    if (pusPppId == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_IfaceId2PppId, pusPppId NULL");
        return VOS_ERR;
    }

    /* ͨ��PPP ID������ң�Ч�ʻ�Ƚϸ� */
    for (usPppId = 1; usPppId <= PPP_MAX_ID_NUM; usPppId++)
    {
        ucIndex = gastAtPppIndexTab[usPppId];

        if (ucIndex >= AT_MAX_CLIENT_NUM)
        {
            continue;
        }

        if (gastAtClientTab[ucIndex].ucUsed != AT_CLIENT_USED)
        {
            continue;
        }

        if (gastAtClientTab[ucIndex].Mode != AT_DATA_MODE)
        {
            continue;
        }

        if ( (gastAtClientTab[ucIndex].DataMode != AT_PPP_DATA_MODE)
          && (gastAtClientTab[ucIndex].DataMode != AT_IP_DATA_MODE) )
        {
            continue;
        }

        if (gastAtClientTab[ucIndex].ucIfaceId == ucIfaceId)
        {
            *pusPppId = usPppId;    /* ���ؽ�� */
            return VOS_OK;
        }

    }

    AT_LOG1("AT, At_IfaceId2PppId, WARNING, Get PppId from Rab <1> Fail",
                                                                   ucIfaceId);
    return VOS_ERR;
}


VOS_UINT32 At_PppId2IfaceId(
    VOS_UINT16                          usPppId,
    VOS_UINT8                          *pucIfaceId
)
{
    VOS_UINT8                           ucIndex;

    if ((usPppId < 1) || (usPppId > PPP_MAX_ID_NUM))
    {
        TAF_LOG1(WUEPS_PID_AT, 0, PS_LOG_LEVEL_WARNING,
            "AT, At_PppId2IfaceId, WARNING, PppId <1> Wrong", usPppId);
        return VOS_ERR;
    }

    if (pucIfaceId == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT, At_PppId2IfaceId, WARNING, pucIfaceId NULL");
        return VOS_ERR;
    }

    ucIndex = gastAtPppIndexTab[usPppId];
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_LOG1("AT, At_PppId2IfaceId, WARNING, index <1> Wrong", ucIndex);
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].Mode != AT_DATA_MODE)
    {
        AT_LOG1("AT, At_PppId2IfaceId, WARNING, Mode <1> Wrong",
                                                 gastAtClientTab[ucIndex].Mode);
        return VOS_ERR;
    }

    if ( (gastAtClientTab[ucIndex].DataMode != AT_PPP_DATA_MODE)
      && (gastAtClientTab[ucIndex].DataMode != AT_IP_DATA_MODE) )
    {
        AT_LOG1("AT, At_PppId2IfaceId, WARNING, DataMode <1> Wrong",
                                             gastAtClientTab[ucIndex].DataMode);
        return VOS_ERR;
    }

    *pucIfaceId = gastAtClientTab[ucIndex].ucIfaceId;

    return VOS_OK;
}
#endif

#if( FEATURE_ON == FEATURE_CSD )

VOS_UINT32 AT_VpResetFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT32                          ulRet;
    FC_ID_ENUM_UINT8                    enFcId;
    MODEM_ID_ENUM_UINT16                enModemId;

    enFcId    = ulParam2;
    enModemId = g_stFcIdMaptoFcPri[enFcId].enModemId;

    if (g_stFcIdMaptoFcPri[enFcId].ulUsed == VOS_TRUE)
    {
        ulRet = FC_DeRegPoint(enFcId, enModemId);
        if (ulRet != VOS_OK)
        {
            AT_ERR_LOG("AT_VpResetFlowCtl: ERROR: de reg point Failed.");
            return VOS_ERR;
        }

        g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_FALSE;
        g_stFcIdMaptoFcPri[enFcId].enFcPri      = FC_PRI_BUTT;
        g_stFcIdMaptoFcPri[enFcId].enModemId    = MODEM_ID_BUTT;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RegModemVideoPhoneFCPoint(
    VOS_UINT8                           ucIndex,
    FC_ID_ENUM_UINT8                    enFcId
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    VOS_UINT32                          ulRet;
    FC_PRI_ENUM_UINT8                   enFcPri;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    memset_s(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemVideoPhoneFCPoint: Get modem id fail.");
        return VOS_ERR;
    }

    stRegFcPoint.enFcId             = enFcId;

    /*  FC_PRI_3        ��������ȼ��ĳ���
        FC_PRI_4        ��NONGBR����
        FC_PRI_5        ��GBR���� */
    enFcPri                         = FC_PRI_FOR_PDN_LOWEST;
    stRegFcPoint.enFcPri            = enFcPri;
    stRegFcPoint.enPolicyId         = FC_POLICY_ID_MEM;
    stRegFcPoint.enModemId          = enModemId;
    stRegFcPoint.pClrFunc           = AT_MODEM_StopFlowCtrl;
    stRegFcPoint.pSetFunc           = AT_MODEM_StartFlowCtrl;
    stRegFcPoint.ulParam1           = (VOS_UINT32)g_alAtUdiHandle[ucIndex];
    stRegFcPoint.ulParam2           = enFcId;
    stRegFcPoint.pRstFunc           = AT_VpResetFlowCtl;

    /* ע�����ص�,��Ҫ�ֱ�ע��MEM,CPU,CST */
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemVideoPhoneFCPoint: ERROR: FC RegPoint CST Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CPU_A;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemVideoPhoneFCPoint: ERROR: FC RegPoint MEM Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CST;
    ulRet = FC_RegPoint(&stRegFcPoint);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_RegModemVideoPhoneFCPoint: ERROR: FC RegPoint A CPU Failed.");
        return VOS_ERR;
    }

    /* ����FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulUsed  = VOS_TRUE;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enFcPri = enFcPri;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enModemId = enModemId;

    /* ��������Ϣ */
    AT_MNTN_TraceRegFcPoint(ucIndex, AT_FC_POINT_TYPE_MODEM_VP);

    return VOS_OK;
}


VOS_UINT32 AT_DeRegModemVideoPhoneFCPoint(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_DeRegModemVideoPhoneFCPoint: Get modem id fail.");
        return VOS_ERR;
    }

    ulRet = FC_DeRegPoint(FC_ID_MODEM, enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_DeRegModemVideoPhoneFCPoint: ERROR: FC DeRegPoint Failed.");
        return VOS_ERR;
    }

    /* ���FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[FC_ID_MODEM].ulUsed      = VOS_FALSE;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enFcPri     = FC_PRI_BUTT;
    g_stFcIdMaptoFcPri[FC_ID_MODEM].enModemId   = MODEM_ID_BUTT;

    /* ��������Ϣ */
    AT_MNTN_TraceDeregFcPoint(ucIndex, AT_FC_POINT_TYPE_MODEM_VP);

    return VOS_OK;
}
#endif


VOS_UINT32 AT_GetLanAddr32(
  VOS_UINT8                            *pucAddr
)
{
    VOS_UINT32                          ulAddr;

    ulAddr = *pucAddr++;
    ulAddr <<= 8;
    ulAddr |= *pucAddr++;
    ulAddr <<= 8;
    ulAddr |= *pucAddr++;
    ulAddr <<= 8;
    ulAddr |= *pucAddr;

    return ulAddr;
}


VOS_UINT32 AT_GetFcPriFromMap(
    const FC_ID_ENUM_UINT8             enFcId,
    AT_FCID_MAP_STRU                  *pstFcIdMap
)
{
    if (enFcId >= FC_ID_BUTT)
    {
        return VOS_ERR;
    }

    if ((enFcId == FC_ID_MODEM)
     || (enFcId == FC_ID_NIC_1)
     || (enFcId == FC_ID_NIC_2)
     || (enFcId == FC_ID_NIC_3)
     || (enFcId == FC_ID_NIC_4)
     || (enFcId == FC_ID_NIC_5)
     || (enFcId == FC_ID_NIC_6)
     || (enFcId == FC_ID_NIC_7)
     || (enFcId == FC_ID_DIPC_1)
     || (enFcId == FC_ID_DIPC_2)
     || (enFcId == FC_ID_DIPC_3))
    {
        pstFcIdMap->ulUsed  = g_stFcIdMaptoFcPri[enFcId].ulUsed;
        pstFcIdMap->enFcPri = g_stFcIdMaptoFcPri[enFcId].enFcPri;

        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_VOID AT_ChangeFCPoint(
    TAF_CTRL_STRU                       *pstCtrl,
    FC_PRI_ENUM_UINT8                    enFCPri,
    FC_ID_ENUM_UINT8                     enFcId
)
{
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstCtrl->usClientId, &enModemId);

    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_ChangeFCPoint: Get modem id fail.");
        return;
    }

    if (enFcId != FC_ID_BUTT)
    {
        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_MEM, enFCPri, enModemId);
        if (ulRet != VOS_OK)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }

        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_CPU_A , enFCPri, enModemId);
        if (ulRet != VOS_OK)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }

        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_CDS, enFCPri, enModemId);
        if (ulRet != VOS_OK)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }

        ulRet = FC_ChangePoint(enFcId, FC_POLICY_ID_GPRS, enFCPri, enModemId);
        if (ulRet != VOS_OK)
        {
            AT_ERR_LOG("AT_ChangeFCPoint: ERROR: Change FC Point Failed.");
        }
    }

    return;
}


VOS_VOID AT_NotifyFcWhenPdpModify(
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId
)
{
    FC_PRI_ENUM_UINT8                   enFCPriCurrent;
    AT_FCID_MAP_STRU                    stFCPriOrg;

    if (AT_GetFcPriFromMap(enFcId, &stFCPriOrg) == VOS_OK)
    {
        if (pstEvent->bitOpUmtsQos == TAF_USED)
        {
            enFCPriCurrent = AT_GetFCPriFromQos(&pstEvent->stUmtsQos);
        }
        else
        {
            enFCPriCurrent = FC_PRI_FOR_PDN_NONGBR;
        }

        if ( (stFCPriOrg.ulUsed == VOS_TRUE)
           && (enFCPriCurrent > stFCPriOrg.enFcPri))
        {
            /* ���ݷ���QOS���ı����ص�����ȼ�*/
            AT_ChangeFCPoint(&pstEvent->stCtrl,enFCPriCurrent,enFcId);
        }
    }

    return;
}


VOS_VOID AT_PS_SetPsCallErrCause(
    VOS_UINT16                          usClientId,
    TAF_PS_CAUSE_ENUM_UINT32            enPsErrCause
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    pstPsModemCtx->enPsErrCause = enPsErrCause;

    return;
}


TAF_PS_CAUSE_ENUM_UINT32 AT_PS_GetPsCallErrCause(
    VOS_UINT16                          usClientId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return pstPsModemCtx->enPsErrCause;
}


VOS_VOID AT_PS_AddIpAddrMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulIpAddr;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (pstEvent->enIfaceId >= PS_IFACE_ID_BUTT)
    {
        return;
    }
#else
    if (!AT_PS_IS_RABID_VALID(pstEvent->ucRabId))
    {
        return;
    }
#endif

    if (pstEvent->bitOpPdpAddr)
    {
        ulIpAddr = AT_GetLanAddr32(pstEvent->stPdpAddr.aucIpv4Addr);
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
        pstPsModemCtx->aulIpAddrIfaceIdMap[pstEvent->enIfaceId] = ulIpAddr;
#else
        pstPsModemCtx->aulIpAddrRabIdMap[pstEvent->ucRabId - AT_PS_RABID_OFFSET] = ulIpAddr;
#endif
    }

    return;
}


VOS_VOID AT_PS_DeleteIpAddrMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (pstEvent->enIfaceId >= PS_IFACE_ID_BUTT)
    {
        return;
    }

    if ((pstEvent->ucPrimFlag == VOS_TRUE) &&
        ((pstEvent->enPdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4))
    {
        pstPsModemCtx->aulIpAddrIfaceIdMap[pstEvent->enIfaceId] = 0;
    }
#else
    if (!AT_PS_IS_RABID_VALID(pstEvent->ucRabId))
    {
        return;
    }

    pstPsModemCtx->aulIpAddrRabIdMap[pstEvent->ucRabId - AT_PS_RABID_OFFSET] = 0;
#endif

    return;
}

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)

VOS_UINT32 AT_PS_GetIpAddrByIfaceId(
    const VOS_UINT16                          usClientId,
    const VOS_UINT8                           ucIfaceId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return pstPsModemCtx->aulIpAddrIfaceIdMap[ucIfaceId];
}
#else

VOS_UINT32 AT_PS_GetIpAddrByRabId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucRabId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulIpAddr;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(usClientId);
    ulIpAddr      = 0;

    if (AT_PS_IS_RABID_VALID(ucRabId))
    {
        ulIpAddr = pstPsModemCtx->aulIpAddrRabIdMap[ucRabId - AT_PS_RABID_OFFSET];
    }

    return ulIpAddr;
}
#endif


VOS_UINT32 AT_PS_GetChDataValueFromRnicRmNetId(
    RNIC_DEV_ID_ENUM_UINT8              enRnicRmNetId,
    AT_CH_DATA_CHANNEL_ENUM_UINT32     *penDataChannelId
)
{
    VOS_UINT32                          i;
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChdataRnicRmNetIdTab;

    pstChdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE() ; i++)
    {
        if (enRnicRmNetId == pstChdataRnicRmNetIdTab[i].enRnicRmNetId)
        {
            *penDataChannelId = pstChdataRnicRmNetIdTab[i].enChdataValue;
            break;
        }
    }

    if (i >= AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE())
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


CONST AT_CHDATA_RNIC_RMNET_ID_STRU *AT_PS_GetChDataCfgByChannelId(
    VOS_UINT8                           ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId
)
{
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChdataRnicRmNetIdTab = VOS_NULL_PTR;
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChDataConfig         = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          i;
    MODEM_ID_ENUM_UINT16                enModemId;

    pstChdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRslt != VOS_OK)
    {
        return VOS_NULL_PTR;
    }

    /**
     * MODEM0 RNIC ID Ϊ 0~2�� CHDATA��ֵΪ 1~3
     * MODEM1 RNIC ID Ϊ 3~4�� CHDATA��ֵΪ 4~5
     * MODEM2 RNIC ID Ϊ 6~7�� CHDATA��ֵΪ 6~7
     */

    if (enModemId == MODEM_ID_0)
    {
        if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_1)
         || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_3))
        {
            return VOS_NULL_PTR;
        }
    }
#if (MULTI_MODEM_NUMBER >= 2)
    else if (enModemId == MODEM_ID_1)
    {
        if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_4)
         || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_5))
        {
            return VOS_NULL_PTR;
        }
    }
#if (MULTI_MODEM_NUMBER == 3)
    else if (enModemId == MODEM_ID_2)
    {
        if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_6)
         || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_7))
        {
            return VOS_NULL_PTR;
        }
    }
#endif /* #if (MULTI_MODEM_NUMBER == 3) */
#endif
    else
    {
        return VOS_NULL_PTR;
    }

    /*  �����ж����ܱ�֤enDataChannelId����Ч�ԣ�����RM NET IDһ�����ڱ����ҵ� */
    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE() ; i++)
    {
        if (enDataChannelId == pstChdataRnicRmNetIdTab[i].enChdataValue)
        {
            pstChDataConfig = &pstChdataRnicRmNetIdTab[i];
            break;
        }
    }

    return pstChDataConfig;
}


RNIC_DEV_ID_ENUM_UINT8 AT_PS_GetRmnetIdFromIfaceId(
    const PS_IFACE_ID_ENUM_UINT8                enIfaceId
)
{
    VOS_UINT32                                  i;
    CONST AT_PS_RMNET_IFACE_ID_STRU            *pstRmnetIfaceIdTab;

    pstRmnetIfaceIdTab = AT_PS_RMNET_IFACE_ID_TBL_PTR();

    for (i = 0; i < AT_PS_RMNET_IFACE_ID_TBL_SIZE(); i++)
    {
        if (enIfaceId == pstRmnetIfaceIdTab[i].enIfaceId)
        {
            return pstRmnetIfaceIdTab[i].enRmNetId;
        }
    }

    AT_WARN_LOG("AT_PS_GetRmnetIdFromIfaceId: not find rmnet id");
    return RNIC_DEV_ID_BUTT;
}



FC_ID_ENUM_UINT8 AT_PS_GetFcIdByIfaceId(
    const VOS_UINT32                        ulIfaceId
)
{
    VOS_UINT32                                  i;
    AT_PS_FC_IFACE_ID_STRU                     *pstFcIfaceIdTab = VOS_NULL_PTR;

    pstFcIfaceIdTab = AT_PS_GET_FC_IFACE_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_FC_IFACE_ID_TBL_SIZE(); i++)
    {
        if (pstFcIfaceIdTab[i].enIfaceId == ulIfaceId)
        {
            return pstFcIfaceIdTab[i].enFcId;
        }
    }

    AT_WARN_LOG("AT_PS_GetFcIdFromRnidRmNetId: WARNING: data channel id is abnormal.");
    return FC_ID_BUTT;
}


AT_PS_DATA_CHANL_CFG_STRU* AT_PS_GetDataChanlCfg(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid
)
{
    AT_MODEM_PS_CTX_STRU                  *pstPsModemCtx        = VOS_NULL_PTR;

    pstPsModemCtx       = AT_GetModemPsCtxAddrFromClientId(usClientId);

    return &(pstPsModemCtx->astChannelCfg[ucCid]);
}


TAF_PS_CALL_END_CAUSE_ENUM_UINT8 AT_PS_TransCallEndCause(
    VOS_UINT8                           ucConnectType
)
{
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause;

    enCause = TAF_PS_CALL_END_CAUSE_NORMAL;

    if (ucConnectType == TAF_PS_CALL_TYPE_DOWN_FORCE)
    {
        enCause = TAF_PS_CALL_END_CAUSE_FORCE;
    }

    return enCause;
}


VOS_VOID AT_PS_ReportCustomPcoInfo(
    TAF_PS_CUSTOM_PCO_INFO_STRU        *pstPcoCustInfo,
    TAF_PS_PDN_OPERATE_TYPE_ENUM_UINT8  enOperateType,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_CLIENT_TAB_INDEX_UINT8           enPortIndex
)
{
    VOS_UINT8                          *pstrIpType      = VOS_NULL_PTR;
    VOS_UINT8                           aucStrIpv4[]    = "IPV4";
    VOS_UINT8                           aucStrIpv6[]    = "IPV6";
    VOS_UINT8                           aucStrIpv4v6[]  = "IPV4V6";
    VOS_UINT32                          i = 0;
    VOS_UINT16                          usLength = 0;

    if (enPdpType == TAF_PDP_IPV4)
    {
        pstrIpType = aucStrIpv4;
    }
    else if (enPdpType == TAF_PDP_IPV6)
    {
        pstrIpType = aucStrIpv6;
    }
    else if (enPdpType == TAF_PDP_IPV4V6)
    {
        pstrIpType = aucStrIpv4v6;
    }
    else
    {
        AT_ERR_LOG1("AT_PS_ReportCustomPcoInfo:ERROR: invalid pdp type ", enPdpType);
        return;
    }

    if (pstPcoCustInfo->ulContainerNum > TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM)
    {
        AT_ERR_LOG1("AT_PS_ReportCustomPcoInfo:ERROR: ulContainerNum abnormal ", pstPcoCustInfo->ulContainerNum);
        return;
    }

    for ( i = 0; i < pstPcoCustInfo->ulContainerNum; i++ )
    {
        usLength = 0;

        /* �ϱ���ʽΪ:^CUSTPCOINFO:CID,OPERATETYPE,IPTYPE,CONTAINERID,CONTAINER CONTENT */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s %d,%d,\"%s\",%x,\"",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CUST_PCO_INFO].pucText,
                                           ucCid,
                                           enOperateType,
                                           pstrIpType,
                                           pstPcoCustInfo->astContainerList[i].usContainerId);


        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                        pstPcoCustInfo->astContainerList[i].aucContents,
                                                        (VOS_UINT16)pstPcoCustInfo->astContainerList[i].ucContentLen);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s",
                                           gaucAtCrLf);

        /* ����At_SendResultData���������� */
        At_SendResultData(enPortIndex, pgucAtSndCodeAddr, usLength);
    }

    return;
}


VOS_UINT32 AT_PS_AppSetFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT32                          ulRslt;

    ulRslt = RNIC_StartFlowCtrl((VOS_UINT8)ulParam1);

    if (ulRslt != VOS_OK)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_AppClearFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT32                          ulRslt;

    ulRslt = RNIC_StopFlowCtrl((VOS_UINT8)ulParam1);

    if (ulRslt != VOS_OK)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_EnableNdisFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    /* ͨ��udi_ioctl����ʹ������ */
    VOS_UINT32  ulEnbflg = NCM_IOCTL_FLOW_CTRL_ENABLE;

    if (mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_FLOW_CTRL_NOTIF, (VOS_VOID*)(&ulEnbflg)) != 0)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_DisableNdisFlowCtl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    /* ͨ��udi_ioctl����ȥʹ������ */
    VOS_UINT32  ulEnbflg = NCM_IOCTL_FLOW_CTRL_DISABLE;

    if (mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_FLOW_CTRL_NOTIF, (VOS_VOID*)(&ulEnbflg)) != 0)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID AT_PS_ActiveUsbNet(VOS_VOID)
{
    VOS_UINT32                          ulLinkstus;
    VOS_INT32                           lRtn;
    NCM_IOCTL_CONNECTION_SPEED_S        stNcmConnectSpeed;
    AT_DISPLAY_RATE_STRU                stSpeed;

    memset_s(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));

    if (AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &stSpeed) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ActiveUsbNet : ERROR : AT_GetDisplayRate Error!");
    }
    /* ������ʳ���U32�ķ�Χ��ȡ���ֵ0xffffffff */
    stNcmConnectSpeed.u32DownBitRate   = (AT_AtoI((VOS_CHAR *)stSpeed.ucDlSpeed) >= 0xffffffff) ? 0xffffffff : (VOS_UINT32)AT_AtoI((VOS_CHAR *)stSpeed.ucDlSpeed);
    stNcmConnectSpeed.u32UpBitRate     = (AT_AtoI((VOS_CHAR *)stSpeed.ucUlSpeed) >= 0xffffffff) ? 0xffffffff : (VOS_UINT32)AT_AtoI((VOS_CHAR *)stSpeed.ucUlSpeed);

    lRtn        = mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF, (VOS_VOID *)(&stNcmConnectSpeed));
    if (lRtn != 0)
    {
        AT_ERR_LOG("AT_PS_ActiveUsbNet, Ctrl Speed Fail!" );
        return;
    }

    ulLinkstus  = NCM_IOCTL_CONNECTION_LINKUP;
    lRtn        = mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, &ulLinkstus);
    if (lRtn != 0)
    {
        AT_ERR_LOG("AT_PS_ActiveUsbNet, Active usb net Fail!" );
        return;
    }

    return;
}


VOS_VOID AT_PS_DeActiveUsbNet(VOS_VOID)
{
    VOS_UINT32  ulLinkstus;
    VOS_INT32   lRtn;

    /*ȥ����Ѻ�BSPȷ�ϣ����������ȥ�����ȥ���û��Ӱ��*/
    ulLinkstus = NCM_IOCTL_CONNECTION_LINKDOWN;

    lRtn  = mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, (VOS_VOID*)(&ulLinkstus));
    if(lRtn != 0)
    {
        AT_ERR_LOG("AT_PS_DeActiveUsbNet, Deactive usb net Fail!" );
        return;
    }

    return;
}


VOS_UINT32 AT_PS_IsIpv6CapabilityValid(VOS_UINT8 ucCapability)
{
    if ((ucCapability == AT_IPV6_CAPABILITY_IPV4_ONLY)
     || (ucCapability == AT_IPV6_CAPABILITY_IPV6_ONLY)
     || (ucCapability == AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 AT_PS_ProcCallModifyEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent
)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg          = VOS_NULL_PTR;
    FC_ID_ENUM_UINT8                    enDefaultFcId;

    pstChanCfg = AT_PS_GetDataChanlCfg(ucIndex, pstEvent->ucCid);

    if ( (pstChanCfg->ulUsed        != VOS_TRUE)
      || (pstChanCfg->ulIfaceActFlg != VOS_TRUE))
    {
        AT_ERR_LOG("AT_PS_ProcCallModifyEvent, ps call not activate!" );
        return VOS_ERR;
    }

    /* ��ȡ����ID��Ӧ��FC ID */
    enDefaultFcId = AT_PS_GetFcIdByIfaceId(pstChanCfg->ulIfaceId);

    if (enDefaultFcId >= FC_ID_BUTT)
    {
        return VOS_ERR;
    }

    AT_NotifyFcWhenPdpModify(pstEvent, enDefaultFcId);

    return VOS_OK;
}


TAF_IFACE_USER_TYPE_ENUM_U8 AT_PS_GetUserType(const VOS_UINT8 ucIndex)
{
    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    enUserType          = TAF_IFACE_USER_TYPE_BUTT;
    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    switch(gastAtClientTab[ucIndex].UserType)
    {
        case AT_NDIS_USER:
            enUserType = TAF_IFACE_USER_TYPE_NDIS;
            break;

        case AT_APP_USER:
            enUserType = TAF_IFACE_USER_TYPE_APP;
            break;

        case AT_USBCOM_USER:
            if ( (AT_GetPcuiPsCallFlag() == VOS_TRUE)
              || (*pucSystemAppConfig == SYSTEM_APP_WEBUI))
            {
                enUserType = TAF_IFACE_USER_TYPE_APP;
                break;
            }

            enUserType = TAF_IFACE_USER_TYPE_NDIS;
            break;

        case AT_CTR_USER:
            if (AT_GetCtrlPsCallFlag() == VOS_TRUE)
            {
                enUserType = TAF_IFACE_USER_TYPE_APP;
                break;
            }

            if (*pucSystemAppConfig != SYSTEM_APP_WEBUI)
            {
                enUserType = TAF_IFACE_USER_TYPE_NDIS;
                break;
            }
            break;

        case AT_PCUI2_USER:
            if (AT_GetPcui2PsCallFlag() == VOS_TRUE)
            {
                enUserType = TAF_IFACE_USER_TYPE_APP;
                break;
            }

            if (*pucSystemAppConfig != SYSTEM_APP_WEBUI)
            {
                enUserType = TAF_IFACE_USER_TYPE_NDIS;
                break;
            }
            break;

        default:
            AT_WARN_LOG("AT_PS_GetUserType: UserType Is Invalid!");
            enUserType = TAF_IFACE_USER_TYPE_BUTT;
            break;
    }

    return enUserType;
}


VOS_UINT32 AT_PS_CheckDialParamCnt(
    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType
)
{
    VOS_UINT8                       *pucSystemAppConfig   = VOS_NULL_PTR;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    /* ���������� */
    if (enUserType == TAF_IFACE_USER_TYPE_APP)
    {
        if (*pucSystemAppConfig != SYSTEM_APP_WEBUI)
        {
            if (gucAtParaIndex > 8)
            {
                AT_NORM_LOG1("AT_PS_CheckDialParamCnt: Phone Dial Parameter number is .\n", gucAtParaIndex);
                return AT_TOO_MANY_PARA;
            }
        }
        else
        {
            if (gucAtParaIndex > 7)
            {
                AT_NORM_LOG1("AT_PS_CheckDialParamCnt: WEBUI APP Dial Parameter number is .\n", gucAtParaIndex);
                return AT_TOO_MANY_PARA;
            }
        }

        return AT_SUCCESS;
    }

    if (enUserType == TAF_IFACE_USER_TYPE_NDIS)
    {
        /* ����MBB��Ʒ��Ҫ����չ��7�� */
        if (gucAtParaIndex > 7)
        {
            AT_NORM_LOG1("AT_PS_CheckDialParamCnt: NDIS Dial Parameter number is .\n", gucAtParaIndex);
            return AT_TOO_MANY_PARA;
        }
        else
        {
            return AT_SUCCESS;
        }
    }

    AT_WARN_LOG("AT_PS_ValidateDialParam: User Type is invalid.\n");

    return AT_ERROR;
}


VOS_UINT32 AT_PS_CheckDialParamApn(VOS_UINT16 usClientId)
{
    /* ��� APN */
    if (gastAtParaList[2].usParaLen != 0)
    {
        /* APN���ȼ�� */
        if (gastAtParaList[2].usParaLen > TAF_MAX_APN_LEN)
        {
            AT_NORM_LOG("AT_PS_CheckDialParamApn: APN is too long.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* APN��ʽ��� */
        if (AT_CheckApnFormat(gastAtParaList[2].aucPara,
                                        gastAtParaList[2].usParaLen,
                                        usClientId) != VOS_OK)
        {
            AT_NORM_LOG("AT_PS_CheckDialParamApn: Format of APN is wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_CheckDialParamIpAddr(VOS_VOID)
{
    VOS_UINT8                           aucIpv4Addr[TAF_IPV4_ADDR_LEN];

    memset_s(aucIpv4Addr, sizeof(aucIpv4Addr), 0x00, TAF_IPV4_ADDR_LEN);

    /* ip addr��� */
    if (gastAtParaList[6].usParaLen > (TAF_MAX_IPV4_ADDR_STR_LEN - 1))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[6].usParaLen > 0)
    {
        if (AT_Ipv4AddrAtoi((VOS_CHAR *)gastAtParaList[6].aucPara, aucIpv4Addr, sizeof(aucIpv4Addr)) != VOS_OK)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_GenIfaceId(
    const VOS_UINT8                     ucIndex,
    const PS_IFACE_ID_ENUM_UINT8        enBeginIfaceId,
    const PS_IFACE_ID_ENUM_UINT8        enEndIfaceId,
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg
)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfgTemp = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT32                          j;

    /* E5��̬APP���Ͳ��ſ��Բ���Ҫ^CHDATA���� */
    for (i = enBeginIfaceId; i <= enEndIfaceId; i++)
    {
        for (j = 1; j <= TAF_MAX_CID; j++)
        {
            pstChanCfgTemp = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)j);

            if ( (pstChanCfgTemp->ulUsed == VOS_TRUE)
              && (pstChanCfgTemp->ulIfaceId == i))
            {
                break;
            }
        }

        if (j > TAF_MAX_CID)
        {
            pstChanCfg->ulUsed          = VOS_TRUE;
            pstChanCfg->ulIfaceId       = i;
            pstChanCfg->ulRmNetId       = AT_PS_GetRmnetIdFromIfaceId((PS_IFACE_ID_ENUM_UINT8)pstChanCfg->ulIfaceId);
            return AT_SUCCESS;
        }
    }

    AT_NORM_LOG("AT_PS_GenIfaceId: IFACEID is all used!");
    return AT_ERROR;
}


VOS_UINT32 AT_PS_CheckDialParamDataChanl(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg          = VOS_NULL_PTR;
    VOS_UINT8                          *pucSystemAppConfig  = VOS_NULL_PTR;
    PS_IFACE_ID_ENUM_UINT8              enBeginIfaceId;
    PS_IFACE_ID_ENUM_UINT8              enEndIfaceId;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    pstChanCfg          = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);

    /* �ֻ���̬������ʹ��^CHDATA���� */
    if ( (enUserType == TAF_IFACE_USER_TYPE_APP)
      && (*pucSystemAppConfig != SYSTEM_APP_WEBUI))
    {
        /* ���ͨ��ӳ�� */
        if ( (pstChanCfg->ulUsed == VOS_FALSE)
          || (pstChanCfg->ulRmNetId == AT_PS_INVALID_RMNET_ID) )
        {
            AT_NORM_LOG2("AT_PS_CheckDialParamDataChanl: Used is .RmNetId is .\n", pstChanCfg->ulUsed, pstChanCfg->ulRmNetId);
            return AT_CME_INCORRECT_PARAMETERS;
        }

        return AT_SUCCESS;
    }

    /* ����ʱ�����ֻ���̬�Ĳ������ʹ����^CHDATA���ã���ֱ��ʹ��������Ϣ��������Ҫ�������IFACEID */
    if (gastAtParaList[1].ulParaValue == TAF_PS_CALL_TYPE_UP)
    {
        if (pstChanCfg->ulUsed == VOS_FALSE)
        {
            if (enUserType == TAF_IFACE_USER_TYPE_NDIS)
            {
                /* NDIS���͵Ĳ���Ĭ��ֱ��ʹ��PS_IFACE_ID_NDIS0 */
                enBeginIfaceId  = PS_IFACE_ID_NDIS0;
                enEndIfaceId    = PS_IFACE_ID_NDIS0;
            }
            else
            {
                /* E5��̬APP���Ͳ��ŷ�ΧΪPS_IFACE_ID_RMNET0~PS_IFACE_ID_RMNET2 */
                enBeginIfaceId  = PS_IFACE_ID_RMNET0;
                enEndIfaceId    = PS_IFACE_ID_RMNET2;
            }

            return AT_PS_GenIfaceId(ucIndex, enBeginIfaceId, enEndIfaceId, pstChanCfg);
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_ValidateDialParam(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
)
{
    VOS_UINT32                          ulRst;

    /* ����������� */
    if (g_stATParseCmd.ucCmdOptType == AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: No parameter input.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������� */
    ulRst = AT_PS_CheckDialParamCnt(enUserType);
    if (ulRst != AT_SUCCESS)
    {
        return ulRst;
    }

    /* ��� CID */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing CID.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���� CONN: �ò�������ʡ��, 1��ʾ��������, 0��ʾ�Ͽ��Ͽ����� */
    if (gastAtParaList[1].usParaLen == 0)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing connect state.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��� APN */
    ulRst = AT_PS_CheckDialParamApn(gastAtClientTab[ucIndex].usClientId);
    if (ulRst != AT_SUCCESS)
    {
        return ulRst;
    }

    /* ��� Username */
    if (gastAtParaList[3].usParaLen > TAF_MAX_AUTHDATA_USERNAME_LEN)
    {
        AT_NORM_LOG1("AT_PS_ValidateDialParam: Username length is.\n", gastAtParaList[3].usParaLen);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��� Password */
    if (gastAtParaList[4].usParaLen > TAF_MAX_AUTHDATA_PASSWORD_LEN)
    {
        AT_NORM_LOG1("AT_PS_ValidateDialParam: Password length is.\n", gastAtParaList[4].usParaLen);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ip addr��� */
    ulRst = AT_PS_CheckDialParamIpAddr();
    if (ulRst != AT_SUCCESS)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: ip addr is invalid.");
        return ulRst;
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (gastAtParaList[7].usParaLen > 0)
    {
        if (AT_PS_CheckDialRatType(ucIndex, (VOS_UINT8)gastAtParaList[7].ulParaValue) != VOS_TRUE)
        {
            AT_NORM_LOG1("AT_PS_ValidateDialParam: DialRatType is.\n", gastAtParaList[7].ulParaValue);
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
#endif

    /* ���ͨ��ӳ�� */
    ulRst = AT_PS_CheckDialParamDataChanl(ucIndex, enUserType);
    if (ulRst != AT_SUCCESS)
    {
        return ulRst;
    }

    return AT_SUCCESS;
}


VOS_UINT16 AT_PS_BuildExClientId(VOS_UINT16 usClientId)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(usClientId, &enModemId) != VOS_OK)
    {
        enModemId =  MODEM_ID_BUTT;
    }

    return TAF_PS_BUILD_EXCLIENTID(enModemId, usClientId);
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_PS_CheckDialRatType(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucBitRatType
)
{
    if (At_CheckCurrRatModeIsCL(ucIndex) == VOS_TRUE)
    {
        switch (ucBitRatType)
        {
            case AT_PS_DIAL_RAT_TYPE_NO_ASTRICT:
            case AT_PS_DIAL_RAT_TYPE_1X_OR_HRPD:
            case AT_PS_DIAL_RAT_TYPE_LTE_OR_EHRPD:
                return VOS_TRUE;

            default:
                AT_NORM_LOG1("AT_PS_CheckDialRatType: Rat Type Error.\n", ucBitRatType);
                return VOS_FALSE;
        }
    }
    else
    {
        AT_NORM_LOG("AT_PS_CheckDialRatType: Not CL mode.\n");
        return VOS_FALSE;
    }
}
#endif


VOS_VOID AT_PS_ReportImsCtrlMsgu(
    VOS_UINT8                                       ucIndex,
    AT_IMS_CTRL_MSG_RECEIVE_MODULE_ENUM_UINT8       enModule,
    VOS_UINT32                                      ulMsgLen,
    VOS_UINT8                                      *pucDst
)
{
    /* ����ֲ����� */
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^IMSCTRLMSGU: %d,%d,\"",
                                       gaucAtCrLf,
                                       enModule,
                                       ulMsgLen);

    usLength += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                    pucDst,
                                                    (VOS_UINT16)ulMsgLen);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"%s",
                                       gaucAtCrLf);

    /* ����At_SendResultData���������� */
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_UINT32 AT_PS_BuildIfaceCtrl(
    const VOS_UINT32                    ulModuleId,
    const VOS_UINT16                    usPortClientId,
    const VOS_UINT8                     ucOpId,
    TAF_CTRL_STRU                      *pstCtrl
)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* ��ȡclient id��Ӧ��Modem Id */
    if (AT_GetModemIdFromClient(usPortClientId, &enModemId) == VOS_ERR)
    {
        AT_ERR_LOG("AT_PS_BuildIfaceCtrl:AT_GetModemIdFromClient is error");
        return VOS_ERR;
    }

    pstCtrl->ulModuleId     = ulModuleId;
    pstCtrl->usClientId     = AT_PS_BuildExClientId(usPortClientId);
    pstCtrl->ucOpId         = ucOpId;

    return VOS_OK;
}


VOS_VOID AT_PS_GetUsrDialAuthType(
    VOS_UINT8                           ucIndex,
    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType,
    TAF_IFACE_DIAL_PARAM_STRU          *pstUsrDialParam
)
{
    VOS_UINT8                       *pucSystemAppConfig   = VOS_NULL_PTR;

    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

    /* AUTH TYPE */
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if ( (At_CheckCurrRatModeIsCL(ucIndex) == VOS_TRUE)
      && (*pucSystemAppConfig != SYSTEM_APP_WEBUI)
      && (enUserType == TAF_IFACE_USER_TYPE_APP))
    {
        pstUsrDialParam->ucAuthType = AT_ClGetPdpAuthType(gastAtParaList[5].ulParaValue,
                                                          gastAtParaList[5].usParaLen);
    }
    else
#endif
    {
        if (gastAtParaList[5].usParaLen > 0)
        {
            pstUsrDialParam->ucAuthType = AT_CtrlGetPDPAuthType(gastAtParaList[5].ulParaValue,
                                                                gastAtParaList[5].usParaLen);
        }
        else
        {
            pstUsrDialParam->ucAuthType = TAF_PDP_AUTH_TYPE_NONE;

            /* ����û��������볤�Ⱦ���Ϊ0, �Ҽ�Ȩ����δ����, ��Ĭ��ʹ��CHAP���� */
            if ( (gastAtParaList[3].usParaLen != 0)
              && (gastAtParaList[4].usParaLen != 0) )
            {
                pstUsrDialParam->ucAuthType = TAF_PDP_AUTH_TYPE_CHAP;
            }
        }
    }

    return;
}



VOS_UINT32 AT_PS_GetUsrDialParam(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType,
    TAF_IFACE_DIAL_PARAM_STRU          *pstUsrDialParam
)
{
    /* �ɵ����߱�֤��κͳ�����Ч�� */
    VOS_UINT8                          *pucSystemAppConfig  = VOS_NULL_PTR;
    TAF_PDP_PRIM_CONTEXT_STRU           stPdpCtxInfo;
    VOS_UINT32                          ulRslt;
    errno_t                             lMemResult;

    /* User Type */
    pstUsrDialParam->enUserType    = enUserType;

    /* CID */
    pstUsrDialParam->ucCid         = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* APN */
    pstUsrDialParam->ucApnLen      = (VOS_UINT8)gastAtParaList[2].usParaLen;
    lMemResult = memcpy_s(pstUsrDialParam->aucApn,
                          sizeof(pstUsrDialParam->aucApn),
                          gastAtParaList[2].aucPara,
                          gastAtParaList[2].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstUsrDialParam->aucApn), gastAtParaList[2].usParaLen);

    /* Username */
    pstUsrDialParam->ucUsernameLen = (VOS_UINT8)gastAtParaList[3].usParaLen;
    lMemResult = memcpy_s(pstUsrDialParam->aucUsername,
                          sizeof(pstUsrDialParam->aucUsername),
                          gastAtParaList[3].aucPara,
                          gastAtParaList[3].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstUsrDialParam->aucUsername), gastAtParaList[3].usParaLen);

    /* Password */
    pstUsrDialParam->ucPasswordLen = (VOS_UINT8)gastAtParaList[4].usParaLen;
    lMemResult = memcpy_s(pstUsrDialParam->aucPassword,
                          sizeof(pstUsrDialParam->aucPassword),
                          gastAtParaList[4].aucPara,
                          gastAtParaList[4].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstUsrDialParam->aucPassword), gastAtParaList[4].usParaLen);

    /* AUTH TYPE */
    AT_PS_GetUsrDialAuthType(ucIndex, enUserType, pstUsrDialParam);

    /* ADDR */
    /* ֻ��E5���ź�NDIS���Ų���Ҫ��� */
    pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
    if ( ( ( (enUserType == TAF_IFACE_USER_TYPE_APP) && (*pucSystemAppConfig == SYSTEM_APP_WEBUI))
        || (enUserType == TAF_IFACE_USER_TYPE_NDIS))
      && (gastAtParaList[6].usParaLen > 0))
    {
        pstUsrDialParam->bitOpIPv4ValidFlag = VOS_TRUE;
        lMemResult = memcpy_s((VOS_CHAR *)pstUsrDialParam->aucIPv4Addr,
                              sizeof(pstUsrDialParam->aucIPv4Addr),
                              (VOS_CHAR *)gastAtParaList[6].aucPara,
                              gastAtParaList[6].usParaLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstUsrDialParam->aucIPv4Addr), gastAtParaList[6].usParaLen);
    }

    /* PDN TYPE */
    memset_s(&stPdpCtxInfo, sizeof(stPdpCtxInfo), 0x00, sizeof(TAF_PDP_PRIM_CONTEXT_STRU));
    ulRslt = TAF_AGENT_GetPdpCidPara(&stPdpCtxInfo, ucIndex, pstUsrDialParam->ucCid);

    if ( (ulRslt == VOS_OK)
      && (AT_PS_IS_PDP_TYPE_SUPPORT(stPdpCtxInfo.stPdpAddr.enPdpType)) )
    {
        pstUsrDialParam->enPdpType = stPdpCtxInfo.stPdpAddr.enPdpType;
    }
    else
    {
        pstUsrDialParam->enPdpType = TAF_PDP_IPV4;
    }

#if(FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if (gastAtParaList[7].usParaLen > 0)
    {
        pstUsrDialParam->ucBitRatType  = (VOS_UINT8)gastAtParaList[7].ulParaValue;
    }

#endif

    if (AT_CheckIpv6Capability(pstUsrDialParam->enPdpType) != VOS_OK)
    {
        AT_INFO_LOG("AT_PS_GetUsrDialParam: PDP type is not supported.");

        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_PS_ProcIfaceUp(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg      = VOS_NULL_PTR;
    TAF_CTRL_STRU                       stCtrl;
    TAF_IFACE_UP_STRU                   stIfaceUp;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    /* ������ƽṹ�� */
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stCtrl) == VOS_ERR)
    {
        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);
        return AT_ERROR;
    }

    memset_s(&stIfaceUp, sizeof(stIfaceUp), 0x00, sizeof(TAF_IFACE_UP_STRU));

    /* ����������Ϣ��ֵ */
    if (AT_PS_GetUsrDialParam(ucIndex, enUserType, &(stIfaceUp.stUsrDialParam)) == VOS_ERR)
    {
        AT_ERR_LOG("AT_PS_ProcIfaceUp: AT_PS_GetUsrDialParam is error.");
        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);
        return AT_ERROR;
    }

    pstChanCfg            = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
    stIfaceUp.ucIfaceId   = (VOS_UINT8)pstChanCfg->ulIfaceId;

    /* ��������������� */
    if (TAF_IFACE_Up(&stCtrl, &stIfaceUp) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ProcIfaceUp: TAF_IFACE_Up is error.");
        AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);
        return AT_ERROR;
    }

    /* ���淢��IPFACE UP��PortIndex */
    pstChanCfg->enPortIndex = ucIndex;

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_ProcIfaceDown(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg      = VOS_NULL_PTR;
    TAF_CTRL_STRU                       stCtrl;
    TAF_IFACE_DOWN_STRU                 stIfaceDown;

    pstChanCfg      = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);

    if ( (pstChanCfg->ulUsed == VOS_FALSE)
      || (pstChanCfg->ulIfaceId == AT_PS_INVALID_IFACE_ID))
    {
        AT_NORM_LOG("AT_PS_ProcIfaceDown: Iface is not act.");
        return AT_ERROR;
    }

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    /* ������ƽṹ�� */
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stCtrl) == VOS_ERR)
    {
        return AT_ERROR;
    }

    /* ����ȥ������Ϣ��ֵ */
    memset_s(&stIfaceDown, sizeof(stIfaceDown), 0x00, sizeof(TAF_IFACE_DOWN_STRU));
    stIfaceDown.ucIfaceId   = (VOS_UINT8)pstChanCfg->ulIfaceId;
    stIfaceDown.enCause     = AT_PS_TransCallEndCause((VOS_UINT8)gastAtParaList[1].ulParaValue);
    stIfaceDown.enUserType  = enUserType;

    /* ��������ȥ������� */
    if (TAF_IFACE_Down(&stCtrl, &stIfaceDown) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ProcIfaceDown: TAF_IFACE_Down is error.");
        return AT_ERROR;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_PS_ProcIfaceCmd(
    const VOS_UINT8                     ucIndex,
    const TAF_IFACE_USER_TYPE_ENUM_U8   enUserType
)
{
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg         = VOS_NULL_PTR;
    VOS_UINT32                          ulRst;


    if (gastAtParaList[1].ulParaValue == TAF_PS_CALL_TYPE_UP)
    {
        pucSystemAppConfig  = AT_GetSystemAppConfigAddr();

        if ( (enUserType == TAF_IFACE_USER_TYPE_APP)
          && (*pucSystemAppConfig == SYSTEM_APP_WEBUI)
          && (g_enHiLinkMode == AT_HILINK_GATEWAY_MODE))
        {
            /* ��¼PS����д����� */
            AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_UNKNOWN);

            /* ���ͨ��ӳ�� */
            pstChanCfg = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
            AT_CleanDataChannelCfg(pstChanCfg);

            return AT_ERROR;
        }

        ulRst = AT_PS_ProcIfaceUp(ucIndex, enUserType);

        /* MBB��̬����Ҫ���ͨ��ӳ�� */
        if ( (ulRst != AT_SUCCESS)
          && (*pucSystemAppConfig != SYSTEM_APP_ANDROID))
        {
            /* ���ͨ��ӳ�� */
            pstChanCfg = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
            AT_CleanDataChannelCfg(pstChanCfg);
        }
    }
    else
    {
        ulRst = AT_PS_ProcIfaceDown(ucIndex, enUserType);
    }

    if (ulRst == AT_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NDISDUP_SET;

        /* ������������״̬ */
        ulRst = AT_WAIT_ASYNC_RETURN;
    }

    return ulRst;
}


VOS_UINT32 AT_PS_ProcMapconMsg(
    const VOS_UINT16                    usClientId,
    const AT_MAPCON_CTRL_MSG_STRU      *pstAtMapConMsgPara
)
{
    const VOS_UINT32                   *pulMsgType;
    TAF_CTRL_STRU                       stCtrl;
    TAF_PS_EPDG_CTRL_STRU               stEpdgCtrl;
    errno_t                             lMemResult;

    pulMsgType  = (VOS_UINT32 *)(pstAtMapConMsgPara->ucMsgContext);
    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    memset_s(&stEpdgCtrl, sizeof(stEpdgCtrl), 0x00, sizeof(TAF_PS_EPDG_CTRL_STRU));

    AT_NORM_LOG1("AT_PS_ProcMapconMsg: Msg Type is ", (*pulMsgType));

    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, usClientId, 0, &stCtrl) == VOS_ERR)
    {
        return VOS_ERR;
    }

    switch (*pulMsgType)
    {
        case ID_WIFI_IMSA_IMS_PDN_ACTIVATE_CNF:

            if (pstAtMapConMsgPara->ulMsgLen != sizeof(TAF_PS_EPDG_ACT_CNF_INFO_STRU))
            {
                AT_ERR_LOG("AT_PS_ProcMapconMsg: activate cnf length is error!");
                return VOS_ERR;
            }

            stEpdgCtrl.bitOpActCnf = VOS_TRUE;

            lMemResult = memcpy_s(&(stEpdgCtrl.stActCnfInfo),
                                  sizeof(stEpdgCtrl.stActCnfInfo),
                                  &pstAtMapConMsgPara->ucMsgContext[0],
                                  sizeof(TAF_PS_EPDG_ACT_CNF_INFO_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stEpdgCtrl.stActCnfInfo), sizeof(TAF_PS_EPDG_ACT_CNF_INFO_STRU));
            break;

        case ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_CNF:

            if (pstAtMapConMsgPara->ulMsgLen != sizeof(TAF_PS_EPDG_DEACT_CNF_INFO_STRU))
            {
                AT_ERR_LOG("AT_PS_ProcMapconMsg: deactivate cnf length is error!");
                return VOS_ERR;
            }

            stEpdgCtrl.bitOpDeActCnf = VOS_TRUE;

            lMemResult = memcpy_s(&(stEpdgCtrl.stDeActCnfInfo),
                                  sizeof(stEpdgCtrl.stDeActCnfInfo),
                                  &pstAtMapConMsgPara->ucMsgContext[0],
                                  sizeof(TAF_PS_EPDG_DEACT_CNF_INFO_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stEpdgCtrl.stDeActCnfInfo), sizeof(TAF_PS_EPDG_DEACT_CNF_INFO_STRU));
            break;

        case ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_IND:

            if (pstAtMapConMsgPara->ulMsgLen != sizeof(TAF_PS_EPDG_DEACT_IND_INFO_STRU))
            {
                AT_ERR_LOG("AT_PS_ProcMapconMsg: deactivate ind length is error!");
                return VOS_ERR;
            }

            stEpdgCtrl.bitOpDeActInd = VOS_TRUE;

            lMemResult = memcpy_s(&(stEpdgCtrl.stDeActIndInfo),
                                  sizeof(stEpdgCtrl.stDeActIndInfo),
                                  &pstAtMapConMsgPara->ucMsgContext[0],
                                  sizeof(TAF_PS_EPDG_DEACT_IND_INFO_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stEpdgCtrl.stDeActIndInfo), sizeof(TAF_PS_EPDG_DEACT_IND_INFO_STRU));
            break;

        default:
            AT_ERR_LOG("AT_PS_ProcMapconMsg: Msg Type is error!");
            return VOS_ERR;
    }

    return TAF_PS_EpdgCtrlMsg(&stCtrl, &stEpdgCtrl);
}


VOS_VOID AT_PS_ReportAppIfaceUp(
    const TAF_IFACE_STATUS_IND_STRU        *pstIfaceStatus
)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s^DCONN:%d,\"IPV4\"%s",
                                           gaucAtCrLf,
                                           pstIfaceStatus->ucCid,
                                           gaucAtCrLf);
    }

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s^DCONN:%d,\"IPV6\"%s",
                                           gaucAtCrLf,
                                           pstIfaceStatus->ucCid,
                                           gaucAtCrLf);
    }

    At_SendResultData((VOS_UINT8)pstIfaceStatus->stCtrl.usClientId, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportAppIfaceDown(
    const TAF_IFACE_STATUS_IND_STRU        *pstIfaceStatus
)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s^DEND:%d,%d,\"IPV4\"%s",
                                           gaucAtCrLf,
                                           pstIfaceStatus->ucCid,
                                           pstIfaceStatus->enCause,
                                           gaucAtCrLf);
    }

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s^DEND:%d,%d,\"IPV6\"%s",
                                           gaucAtCrLf,
                                           pstIfaceStatus->ucCid,
                                           pstIfaceStatus->enCause,
                                           gaucAtCrLf);
    }

    At_SendResultData((VOS_UINT8)pstIfaceStatus->stCtrl.usClientId, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportNdisIfaceStat(
    const TAF_IFACE_STATUS_IND_STRU        *pstIfaceStatus
)
{
    VOS_UINT32                          ul3gppSmCause;
    VOS_UINT16                          usLength;

    usLength      = 0;
    ul3gppSmCause = AT_Get3gppSmCauseByPsCause(pstIfaceStatus->enCause);

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4)
    {
        if (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:0,%d,,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                               ul3gppSmCause,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:1,,,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                               gaucAtCrLf);
        }
    }

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6)
    {
        if (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:0,%d,,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                               ul3gppSmCause,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:1,,,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTAT].pucText,
                                               gaucAtCrLf);
        }
    }

    At_SendResultData((VOS_UINT8)pstIfaceStatus->stCtrl.usClientId, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportNdisIfaceStatEx(
    const TAF_IFACE_STATUS_IND_STRU        *pstIfaceStatus
)
{
    VOS_UINT32                          ul3gppSmCause;
    VOS_UINT16                          usLength;

    usLength      = 0;
    ul3gppSmCause = AT_Get3gppSmCauseByPsCause(pstIfaceStatus->enCause);

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4)
    {
        if (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:%d,0,%d,,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                               pstIfaceStatus->ucCid,
                                               ul3gppSmCause,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:%d,1,,,\"IPV4\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                               pstIfaceStatus->ucCid,
                                               gaucAtCrLf);
        }
    }

    if ((pstIfaceStatus->enPdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6)
    {
        if (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:%d,0,%d,,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                               pstIfaceStatus->ucCid,
                                               ul3gppSmCause,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s:%d,1,,,\"IPV6\"%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_NDISSTATEX].pucText,
                                               pstIfaceStatus->ucCid,
                                               gaucAtCrLf);
        }
    }

    At_SendResultData((VOS_UINT8)pstIfaceStatus->stCtrl.usClientId, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ProcAppIfaceStatus(
    TAF_IFACE_STATUS_IND_STRU          *pstIfaceStatus
)
{
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    pucSystemAppConfig = AT_GetSystemAppConfigAddr();

    /* ����E5�����翨��E355����̬�Ĳ��� */
    /* PCUI���·��ϱ�^NDISSTAT��APP���·��ϱ�^NDISSTATEX */
    if ((*pucSystemAppConfig) == SYSTEM_APP_WEBUI)
    {
        if ( (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_ACT)
          || (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT))
        {
            if (AT_CheckAppUser((VOS_UINT8)pstIfaceStatus->stCtrl.usClientId) == VOS_TRUE)
            {
                AT_PS_ReportNdisIfaceStatEx(pstIfaceStatus);
                return;
            }

            AT_PS_ReportNdisIfaceStat(pstIfaceStatus);
        }

        return;
    }

    if (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_ACT)
    {
        AT_PS_ReportAppIfaceUp(pstIfaceStatus);
    }

    if (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT)
    {
        AT_PS_ReportAppIfaceDown(pstIfaceStatus);
    }

    return;
}


VOS_VOID AT_PS_ProcNdisIfaceStatus(
    TAF_IFACE_STATUS_IND_STRU          *pstIfaceStatus
)
{
    if ( (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_ACT)
      || (pstIfaceStatus->enStatus == TAF_IFACE_STATUS_DEACT))
    {
        AT_PS_ReportNdisIfaceStat(pstIfaceStatus);
    }

    return;
}



AT_PS_RPT_IFACE_RSLT_FUNC AT_PS_GetRptIfaceResultFunc(
    const TAF_IFACE_USER_TYPE_ENUM_U8      enUserType
)
{
    const AT_PS_REPORT_IFACE_RESULT_STRU   *pstRptIfaceRsltFuncTblPtr = VOS_NULL_PTR;
    AT_PS_RPT_IFACE_RSLT_FUNC               pRptIfaceRsltFunc         = VOS_NULL_PTR;
    VOS_UINT32                              ulCnt;

    pstRptIfaceRsltFuncTblPtr = AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_PTR();

    /* �û�����ƥ�� */
    for (ulCnt = 0; ulCnt < AT_PS_GET_RPT_IFACE_RSLT_FUNC_TBL_SIZE(); ulCnt++)
    {
        if (enUserType == pstRptIfaceRsltFuncTblPtr[ulCnt].enUserType)
        {
            pRptIfaceRsltFunc = pstRptIfaceRsltFuncTblPtr[ulCnt].pRptIfaceRsltFunc;
            break;
        }
    }

    return pRptIfaceRsltFunc;
}


VOS_VOID AT_PS_ChangeFcPoint(
    const MODEM_ID_ENUM_UINT16           enModemId,
    const FC_PRI_ENUM_UINT8              enFCPri,
    const FC_ID_ENUM_UINT8               enFcId
)
{
    if (enFcId != FC_ID_BUTT)
    {
        if (FC_ChangePoint(enFcId, FC_POLICY_ID_MEM, enFCPri, enModemId) != VOS_OK)
        {
            AT_ERR_LOG("AT_PS_ChangeFcPoint: ERROR: Change FC Point Failed.");
        }

        if (FC_ChangePoint(enFcId, FC_POLICY_ID_CPU_A , enFCPri, enModemId) != VOS_OK)
        {
            AT_ERR_LOG("AT_PS_ChangeFcPoint: ERROR: Change FC Point Failed.");
        }

        if (FC_ChangePoint(enFcId, FC_POLICY_ID_CDS, enFCPri, enModemId) != VOS_OK)
        {
            AT_ERR_LOG("AT_PS_ChangeFcPoint: ERROR: Change FC Point Failed.");
        }

        if (FC_ChangePoint(enFcId, FC_POLICY_ID_GPRS, enFCPri, enModemId) != VOS_OK)
        {
            AT_ERR_LOG("AT_PS_ChangeFcPoint: ERROR: Change FC Point Failed.");
        }
    }

    return;
}


VOS_VOID AT_PS_RegAppFcPoint(
    const FC_ID_ENUM_UINT8              enFcId,
    const MODEM_ID_ENUM_UINT16          enModemId,
    const TAF_IFACE_REG_FC_IND_STRU    *pstRegFcInd
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    FC_PRI_ENUM_UINT8                   enDefaultFcPri;

    enDefaultFcPri = FC_PRI_FOR_PDN_LOWEST;
    memset_s(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    /* ����ͨ����RABIDӳ���ϵ */
    FC_ChannelMapCreate(enFcId, pstRegFcInd->ucRabId, enModemId);
#endif

    /* ����������������ȼ�RAB QoS���ȼ�������,���ȼ��ı�ʱ����Ҫ�ı����ȼ� */
    /*  FC_PRI_3        ��������ȼ��ĳ���
        FC_PRI_4        ��NONGBR����
        FC_PRI_5        ��GBR���� */
    stRegFcPoint.enFcId     = enFcId;
    stRegFcPoint.enFcPri    = enDefaultFcPri;

    stRegFcPoint.enModemId  = enModemId;
    stRegFcPoint.pClrFunc   = AT_PS_AppClearFlowCtrl;
    stRegFcPoint.pSetFunc   = AT_PS_AppSetFlowCtrl;

    /* Paramter1����ΪRmNetId, Paramter2����ΪFCID */
    stRegFcPoint.ulParam1   = AT_PS_GetRmnetIdFromIfaceId(pstRegFcInd->ucIfaceId);
    stRegFcPoint.ulParam2   = enFcId;
    stRegFcPoint.pRstFunc   = AT_ResetFlowCtl;

    /* ע�����ص�, ��Ҫ�ֱ�ע��MEM��CDS */
    stRegFcPoint.enPolicyId = FC_POLICY_ID_MEM;
    if (FC_RegPoint(&stRegFcPoint) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_RegAppFcPoint: ERROR: reg mem point Failed.");
        return;
    }


    stRegFcPoint.enPolicyId = FC_POLICY_ID_CDS;
    if (FC_RegPoint(&stRegFcPoint) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_RegAppFcPoint: ERROR: reg CDS point Failed.");
        return;
    }

    /* ����FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_TRUE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri      = enDefaultFcPri;
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask  |= ((VOS_UINT32)1 << (pstRegFcInd->ucRabId));
    g_stFcIdMaptoFcPri[enFcId].enModemId    = enModemId;

    /* ��������Ϣ */
    AT_MNTN_TraceRegFcPoint((VOS_UINT8)pstRegFcInd->stCtrl.usClientId, AT_FC_POINT_TYPE_RMNET);

    return;
}


VOS_UINT32 AT_PS_DeRegAppFcPoint(
    const FC_ID_ENUM_UINT8              enFcId,
    const MODEM_ID_ENUM_UINT16          enModemId,
    const TAF_IFACE_DEREG_FC_IND_STRU  *pstDeRegFcInd
)
{
    if ((g_stFcIdMaptoFcPri[enFcId].ulRabIdMask & (0x01UL << pstDeRegFcInd->ucRabId)) == 0)
    {
        AT_ERR_LOG1("AT_PS_DeRegAppFcPoint: RabId err, RabIdMask :", g_stFcIdMaptoFcPri[enFcId].ulRabIdMask);
        return VOS_ERR;
    }

#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    /* �ڵ���FC_DeRegPointǰ,�ȵ���FC_ChannelMapDelete */
    FC_ChannelMapDelete(pstDeRegFcInd->ucRabId, enModemId);
#endif

    if (FC_DeRegPoint(enFcId, enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_DeRegAppFcPoint: ERROR: de reg point Failed.");
        return VOS_ERR;
    }

    /* ���FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask  &= ~((VOS_UINT32)1 << (pstDeRegFcInd->ucRabId));

    if (g_stFcIdMaptoFcPri[enFcId].ulRabIdMask == 0)
    {
        g_stFcIdMaptoFcPri[enFcId].ulUsed       = VOS_FALSE;
        g_stFcIdMaptoFcPri[enFcId].enFcPri      = FC_PRI_BUTT;
        g_stFcIdMaptoFcPri[enFcId].enModemId    = MODEM_ID_BUTT;
    }

    /* ��������Ϣ */
    AT_MNTN_TraceDeregFcPoint((VOS_UINT8)pstDeRegFcInd->stCtrl.usClientId, AT_FC_POINT_TYPE_RMNET);

    return VOS_OK;
}


VOS_UINT32 AT_PS_RegNdisFcPoint(
    const TAF_IFACE_REG_FC_IND_STRU    *pstRegFcInd,
    const FC_ID_ENUM_UINT8              enFcId,
    const MODEM_ID_ENUM_UINT16          enModemId
)
{
    FC_REG_POINT_STRU                   stRegFcPoint;
    FC_PRI_ENUM_UINT8                   enFCPri;

    enFCPri = FC_PRI_FOR_PDN_NONGBR;
    memset_s(&stRegFcPoint, sizeof(stRegFcPoint), 0x00, sizeof(FC_REG_POINT_STRU));

#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    /* ����ͨ����RABIDӳ���ϵ */
    FC_ChannelMapCreate(enFcId, pstRegFcInd->ucRabId, enModemId);
#endif

    stRegFcPoint.enFcId             = enFcId;

    /* ����������������ȼ�RAB QoS���ȼ�������,���ȼ��ı�ʱ����Ҫ�ı����ȼ� */
    /*  FC_PRI_3        ��������ȼ��ĳ���
        FC_PRI_4        ��NONGBR����
        FC_PRI_5        ��GBR���� */

    if (pstRegFcInd->bitOpUmtsQos == TAF_USED)
    {
        enFCPri = AT_GetFCPriFromQos(&pstRegFcInd->stUmtsQos);
    }

    stRegFcPoint.enFcPri            = enFCPri;
    stRegFcPoint.enPolicyId         = FC_POLICY_ID_MEM;
    stRegFcPoint.enModemId          = enModemId;
    stRegFcPoint.pClrFunc           = AT_PS_DisableNdisFlowCtl;
    stRegFcPoint.pSetFunc           = AT_PS_EnableNdisFlowCtl;
    stRegFcPoint.pRstFunc           = AT_ResetFlowCtl;
    stRegFcPoint.ulParam2           = enFcId;
    stRegFcPoint.ulParam1           = (VOS_UINT32)pstRegFcInd->stCtrl.usClientId;

    /* ע�����ص�,��Ҫ�ֱ�ע��MEM,CPU,CDS��GPRS�� */
    if (FC_RegPoint(&stRegFcPoint) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_RegNdisFcPoint: ERROR: reg mem point Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CPU_A;
    if (FC_RegPoint(&stRegFcPoint) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_RegNdisFcPoint: ERROR: reg a cpu point Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_CDS;
    if (FC_RegPoint(&stRegFcPoint) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_RegNdisFcPoint: ERROR: reg cds point Failed.");
        return VOS_ERR;
    }

    stRegFcPoint.enPolicyId         = FC_POLICY_ID_GPRS;
    if (FC_RegPoint(&stRegFcPoint) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_RegNdisFcPoint: ERROR: reg gprs point Failed.");
        return VOS_ERR;
    }

    /* ����FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[enFcId].ulUsed      = VOS_TRUE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri     = enFCPri;
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask |= ((VOS_UINT32)1 << (pstRegFcInd->ucRabId));
    g_stFcIdMaptoFcPri[enFcId].enModemId   = enModemId;

    /* ��������Ϣ */
    AT_MNTN_TraceRegFcPoint((VOS_UINT8)pstRegFcInd->stCtrl.usClientId, AT_FC_POINT_TYPE_NDIS);

    return VOS_OK;
}


VOS_UINT32 AT_PS_DeRegNdisFcPoint(
    const FC_ID_ENUM_UINT8               enFcId,
    const MODEM_ID_ENUM_UINT16           enModemId,
    const TAF_IFACE_DEREG_FC_IND_STRU   *pstDeRegFcInd
)
{
    if ((g_stFcIdMaptoFcPri[enFcId].ulRabIdMask & (0x01UL << pstDeRegFcInd->ucRabId)) == 0)
    {
        AT_ERR_LOG1("AT_PS_DeRegNdisFcPoint: RabId err, RabIdMask :", g_stFcIdMaptoFcPri[enFcId].ulRabIdMask);
        return VOS_ERR;
    }

#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    /* �ڵ���FC_DeRegPointǰ,�ȵ���FC_ChannelMapDelete */
    FC_ChannelMapDelete(pstDeRegFcInd->ucRabId, enModemId);
#endif

    if (FC_DeRegPoint(enFcId, enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_DeRegNdisFcPoint: ERROR: de reg point Failed.");
        return VOS_ERR;
    }

    /* ���FCID��FC Pri��ӳ���ϵ */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask &= ~((VOS_UINT32)1 << (pstDeRegFcInd->ucRabId));

    if (g_stFcIdMaptoFcPri[enFcId].ulRabIdMask == 0)
    {
        g_stFcIdMaptoFcPri[enFcId].ulUsed      = VOS_FALSE;
        g_stFcIdMaptoFcPri[enFcId].enFcPri     = FC_PRI_BUTT;
        g_stFcIdMaptoFcPri[enFcId].enModemId   = MODEM_ID_BUTT;
    }

    /* ��������Ϣ */
    AT_MNTN_TraceDeregFcPoint((VOS_UINT8)pstDeRegFcInd->stCtrl.usClientId, AT_FC_POINT_TYPE_NDIS);

    return VOS_OK;
}


VOS_VOID  AT_PS_ProcAppRegFcPoint(
    const TAF_IFACE_REG_FC_IND_STRU        *pstRegFcInd
)
{
    AT_FCID_MAP_STRU                    stFCPriOrg;
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_ID_ENUM_UINT8                    enDefaultFcId;

    enModemId       = MODEM_ID_0;

    if (AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstRegFcInd->stCtrl.usClientId, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ProcAppRegFcPoint: Get modem id fail.");
        return;
    }

    memset_s(&stFCPriOrg, sizeof(stFCPriOrg), 0x00, sizeof(AT_FCID_MAP_STRU));
    enDefaultFcId = AT_PS_GetFcIdByIfaceId(pstRegFcInd->ucIfaceId);

    if (AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg) == VOS_OK)
    {
        /* ���FC IDδע�ᣬ��ôע������ص㡣Ŀǰֻ֧��һ������.*/
        if ((stFCPriOrg.ulUsed != VOS_TRUE)
         || ((g_stFcIdMaptoFcPri[enDefaultFcId].ulRabIdMask & (0x01UL << pstRegFcInd->ucRabId)) == 0))
        {
            /* ע��APP����ʹ�õ����ص�(Ĭ��ʹ������1) */
            AT_PS_RegAppFcPoint(enDefaultFcId, enModemId, pstRegFcInd);
        }
        else
        {
            /* APP����ֻʹ����͵�����QOS���ȼ�FC_PRI_FOR_PDN_LOWEST */
            AT_NORM_LOG("AT_PS_ProcAppRegFcPoint: No need to change the default QOS priority.");
        }
    }

    return;
}


VOS_VOID AT_PS_ProcAppDeRegFcPoint(
    const TAF_IFACE_DEREG_FC_IND_STRU      *pstDeRegFcInd
)
{
    FC_ID_ENUM_UINT8                    enDefaultFcId;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)pstDeRegFcInd->stCtrl.usClientId, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ProcAppDeRegFcPoint: Get modem id fail.");
        return;
    }

    enDefaultFcId = AT_PS_GetFcIdByIfaceId(pstDeRegFcInd->ucIfaceId);

    if (enDefaultFcId < FC_ID_BUTT)
    {
        /* ȥע��APP����ʹ�õ����ص� */
        AT_PS_DeRegAppFcPoint(enDefaultFcId, enModemId, pstDeRegFcInd);
    }

    return;
}


VOS_VOID  AT_PS_ProcNdisRegFcPoint(
    const TAF_IFACE_REG_FC_IND_STRU        *pstRegFcInd
)
{
    AT_FCID_MAP_STRU                    stFCPriOrg;
    FC_PRI_ENUM_UINT8                   enFCPriCurrent;
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_ID_ENUM_UINT8                    enDefaultFcId;

    enModemId       = MODEM_ID_0;

    if (AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)(pstRegFcInd->stCtrl.usClientId), &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ProcNdisRegFcPoint:Get Modem Id fail");
        return;
    }

    enDefaultFcId = AT_PS_GetFcIdByIfaceId(pstRegFcInd->ucIfaceId);
    memset_s(&stFCPriOrg, sizeof(stFCPriOrg), 0x00, sizeof(AT_FCID_MAP_STRU));

    if (AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg) == VOS_OK)
    {
        /* ���FC IDδע�ᣬ��ôע������ص㡣Ŀǰֻ֧��һ������.*/
        if ((stFCPriOrg.ulUsed != VOS_TRUE)
         || ((g_stFcIdMaptoFcPri[enDefaultFcId].ulRabIdMask & (0x01UL << pstRegFcInd->ucRabId)) == 0))
        {
            /* ע��NDIS�˿ڵ����ص� */
            AT_PS_RegNdisFcPoint(pstRegFcInd, enDefaultFcId, enModemId);
        }
        else
        {
            AT_NORM_LOG("AT_PS_ProcNdisRegFcPoint: has already reg");
            enFCPriCurrent  = FC_PRI_FOR_PDN_NONGBR;

            if (pstRegFcInd->bitOpUmtsQos == TAF_USED)
            {
                enFCPriCurrent = AT_GetFCPriFromQos(&pstRegFcInd->stUmtsQos);
            }

            /* �����ǰFC���ȼ���֮ǰ���ص�FC���ȼ��ߣ���ô�������ȼ���*/
            if(enFCPriCurrent > stFCPriOrg.enFcPri)
            {
                AT_PS_ChangeFcPoint(enModemId, enFCPriCurrent, enDefaultFcId);
            }
        }
    }

    return;
}


VOS_VOID  AT_PS_ProcNdisDeRegFcPoint(
    TAF_IFACE_DEREG_FC_IND_STRU        *pstDeRegFcInd
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_ID_ENUM_UINT8                    enDefaultFcId;

    enModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)(pstDeRegFcInd->stCtrl.usClientId), &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_PS_ProcNdisDeRegFcPoint:Get Modem Id fail");
        return;
    }

    enDefaultFcId = AT_PS_GetFcIdByIfaceId(pstDeRegFcInd->ucIfaceId);

    if (enDefaultFcId < FC_ID_BUTT)
    {
        /* ȥע��NDIS�˿ڵ����ص� */
        AT_PS_DeRegNdisFcPoint(enDefaultFcId, enModemId, pstDeRegFcInd);
    }

    return;
}


VOS_VOID AT_PS_SendNdisIPv4IfaceUpCfgInd(
    const DSM_NDIS_IFACE_UP_IND_STRU   *pstRcvMsg
)
{
    errno_t                             lMemResult;
#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    AT_NDIS_IFACE_UP_CONFIG_IND_STRU   *pstNdisCfgUp        = VOS_NULL_PTR;

    pstNdisCfgUp = (AT_NDIS_IFACE_UP_CONFIG_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_IFACE_UP_CONFIG_IND_STRU));
#else
    AT_NDIS_PDNINFO_CFG_REQ_STRU       *pstNdisCfgUp        = VOS_NULL_PTR;

    pstNdisCfgUp = (AT_NDIS_PDNINFO_CFG_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));
#endif

    if (pstNdisCfgUp == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_PS_SendNdisIPv4IfaceUpCfgInd: alloc msg fail!");
        return;
    }

    /* ��ʼ����Ϣ */
   lMemResult = memset_s(AT_GET_MSG_ENTITY(pstNdisCfgUp), AT_GET_MSG_LENGTH(pstNdisCfgUp),
                         0x00, AT_GET_MSG_LENGTH(pstNdisCfgUp));
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_GET_MSG_LENGTH(pstNdisCfgUp), AT_GET_MSG_LENGTH(pstNdisCfgUp));

#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    /* ��д��Ϣͷ */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgUp, ID_AT_NDIS_IFACE_UP_CONFIG_IND);

    /* ��д��Ϣ�� */
    pstNdisCfgUp->ucIfaceId                         = pstRcvMsg->ucIfaceId;
    pstNdisCfgUp->stIpv4IfaceInfo.ucIfaceId         = pstRcvMsg->ucIfaceId;
    pstNdisCfgUp->stIpv4IfaceInfo.ucPduSessionId    = pstRcvMsg->stIpv4PdnInfo.ucPduSessionId;
    pstNdisCfgUp->stIpv4IfaceInfo.ucFcHead          = AT_GET_IFACE_FC_HEAD_BY_MODEMID(pstRcvMsg->enModemId);
    pstNdisCfgUp->stIpv4IfaceInfo.ucModemId         = (VOS_UINT8)pstRcvMsg->enModemId;
#else
    /* ��д��Ϣͷ */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgUp, ID_AT_NDIS_PDNINFO_CFG_REQ);

    /* ��д��Ϣ�� */
    pstNdisCfgUp->ucRabId           = pstRcvMsg->ucRabId;
    pstNdisCfgUp->enModemId         = pstRcvMsg->enModemId;
    pstNdisCfgUp->lSpePort          = AT_GetCommPsCtxAddr()->lSpePort;
    pstNdisCfgUp->ulIpfFlag         = AT_GetCommPsCtxAddr()->ulIpfPortFlg;
#endif
    pstNdisCfgUp->bitOpIpv4PdnInfo  = VOS_TRUE;
    pstNdisCfgUp->ulHandle          = g_ulAtUdiNdisHdl;

    /* ��дIPv4��ַ */
    if ( pstRcvMsg->stIpv4PdnInfo.bitOpPdnAddr == VOS_TRUE)
    {
        pstNdisCfgUp->stIpv4PdnInfo.bitOpPdnAddr = VOS_TRUE;
        lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr,
                              sizeof(pstNdisCfgUp->stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr),
                              pstRcvMsg->stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr,
                              DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    /* ��д�����ַ */
    lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stSubnetMask.aucIpV4Addr,
                          sizeof(pstNdisCfgUp->stIpv4PdnInfo.stSubnetMask.aucIpV4Addr),
                          pstRcvMsg->stIpv4PdnInfo.stSubnetMask.aucIpV4Addr,
                          DSM_NDIS_IPV4_ADDR_LENGTH);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stSubnetMask.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);

    /* ��д���ص�ַ */
    lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr,
                          sizeof(pstNdisCfgUp->stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr),
                          pstRcvMsg->stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr,
                          DSM_NDIS_IPV4_ADDR_LENGTH);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);

    /* ��д��DNS��ַ */
    if (pstRcvMsg->stIpv4PdnInfo.bitOpDnsPrim != 0)
    {
        pstNdisCfgUp->stIpv4PdnInfo.bitOpDnsPrim = VOS_TRUE;
        lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr,
                              sizeof(pstNdisCfgUp->stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr),
                              pstRcvMsg->stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr,
                              DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    /* ��д��DNS��ַ */
    if (pstRcvMsg->stIpv4PdnInfo.bitOpDnsSec != 0)
    {
        pstNdisCfgUp->stIpv4PdnInfo.bitOpDnsSec = VOS_TRUE;
        lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr,
                              sizeof(pstNdisCfgUp->stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr),
                              pstRcvMsg->stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr,
                              DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    if (pstRcvMsg->stIpv4PdnInfo.bitOpPcscfPrim == VOS_TRUE)
    {
        pstNdisCfgUp->stIpv4PdnInfo.bitOpPcscfPrim = VOS_TRUE;
        lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr,
                              sizeof(pstNdisCfgUp->stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr),
                              pstRcvMsg->stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr,
                              DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    if (pstRcvMsg->stIpv4PdnInfo.bitOpPcscfSec == VOS_TRUE)
    {
        pstNdisCfgUp->stIpv4PdnInfo.bitOpPcscfSec = VOS_TRUE;
        lMemResult = memcpy_s(pstNdisCfgUp->stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr,
                              sizeof(pstNdisCfgUp->stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr),
                              pstRcvMsg->stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr,
                              DSM_NDIS_IPV4_ADDR_LENGTH);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr), DSM_NDIS_IPV4_ADDR_LENGTH);
    }

    /* ������Ϣ */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT,pstNdisCfgUp);

    return;
}


VOS_VOID AT_PS_SendNdisIPv6IfaceUpCfgInd(
    const DSM_NDIS_IFACE_UP_IND_STRU   *pstRcvMsg
)
{
    errno_t                             lMemResult;
#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    AT_NDIS_IFACE_UP_CONFIG_IND_STRU    *pstNdisCfgUp     = VOS_NULL_PTR;

    pstNdisCfgUp = (AT_NDIS_IFACE_UP_CONFIG_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_IFACE_UP_CONFIG_IND_STRU));
#else
    AT_NDIS_PDNINFO_CFG_REQ_STRU        *pstNdisCfgUp     = VOS_NULL_PTR;

    pstNdisCfgUp = (AT_NDIS_PDNINFO_CFG_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));
#endif

    if (pstNdisCfgUp == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_PS_SendNdisIPv6IfaceUpCfgInd: alloc msg fail!");
        return;
    }

    /* ��ʼ����Ϣ */
    lMemResult = memset_s(AT_GET_MSG_ENTITY(pstNdisCfgUp), AT_GET_MSG_LENGTH(pstNdisCfgUp),
                          0x00, AT_GET_MSG_LENGTH(pstNdisCfgUp));
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_GET_MSG_LENGTH(pstNdisCfgUp), AT_GET_MSG_LENGTH(pstNdisCfgUp));

#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    /* ��д��Ϣͷ */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgUp, ID_AT_NDIS_IFACE_UP_CONFIG_IND);

    /* ��д��Ϣ�� */
    pstNdisCfgUp->ucIfaceId                         = pstRcvMsg->ucIfaceId;
    pstNdisCfgUp->stIpv6IfaceInfo.ucIfaceId         = pstRcvMsg->ucIfaceId;
    pstNdisCfgUp->stIpv6IfaceInfo.ucPduSessionId    = pstRcvMsg->stIpv6PdnInfo.ucPduSessionId;
    pstNdisCfgUp->stIpv6IfaceInfo.ucFcHead          = AT_GET_IFACE_FC_HEAD_BY_MODEMID(pstRcvMsg->enModemId);
    pstNdisCfgUp->stIpv6IfaceInfo.ucModemId         = (VOS_UINT8)pstRcvMsg->enModemId;
#else
    /* ��д��Ϣͷ */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgUp, ID_AT_NDIS_PDNINFO_CFG_REQ);

    /* ��д��Ϣ�� */
    pstNdisCfgUp->ucRabId               = pstRcvMsg->ucRabId;
    pstNdisCfgUp->enModemId             = pstRcvMsg->enModemId;
    pstNdisCfgUp->lSpePort              = AT_GetCommPsCtxAddr()->lSpePort;
    pstNdisCfgUp->ulIpfFlag             = AT_GetCommPsCtxAddr()->ulIpfPortFlg;
#endif
    pstNdisCfgUp->bitOpIpv6PdnInfo      = VOS_TRUE;
    pstNdisCfgUp->ulHandle              = g_ulAtUdiNdisHdl;

    /* �������DNS */
    pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.ucSerNum    = 0;
    if (pstRcvMsg->stIpv6PdnInfo.stDnsSer.ucSerNum >= 1)
    {
        lMemResult = memcpy_s((VOS_VOID *)pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.aucPriServer,
                              sizeof(pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.aucPriServer),
                              pstRcvMsg->stIpv6PdnInfo.stDnsSer.aucPriServer,
                              TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.aucPriServer), TAF_IPV6_ADDR_LEN);
        pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    if (pstRcvMsg->stIpv6PdnInfo.stDnsSer.ucSerNum >= 2)
    {
        lMemResult = memcpy_s((VOS_VOID *)pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.aucSecServer,
                              sizeof(pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.aucSecServer),
                              pstRcvMsg->stIpv6PdnInfo.stDnsSer.aucSecServer,
                              TAF_IPV6_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.aucSecServer), TAF_IPV6_ADDR_LEN);
        pstNdisCfgUp->stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    /* ���MTU */
    if (pstRcvMsg->stIpv6PdnInfo.ulBitOpMtu == VOS_TRUE)
    {
        pstNdisCfgUp->stIpv6PdnInfo.ulBitOpMtu   = VOS_TRUE;
        pstNdisCfgUp->stIpv6PdnInfo.ulMtu        = pstRcvMsg->stIpv6PdnInfo.ulMtu;
    }

    pstNdisCfgUp->stIpv6PdnInfo.ulBitCurHopLimit = pstRcvMsg->stIpv6PdnInfo.ulBitCurHopLimit;
    pstNdisCfgUp->stIpv6PdnInfo.ulBitM           = pstRcvMsg->stIpv6PdnInfo.ulBitM;
    pstNdisCfgUp->stIpv6PdnInfo.ulBitO           = pstRcvMsg->stIpv6PdnInfo.ulBitO;
    pstNdisCfgUp->stIpv6PdnInfo.ulPrefixNum      = pstRcvMsg->stIpv6PdnInfo.ulPrefixNum;
    lMemResult = memcpy_s((VOS_VOID *)pstNdisCfgUp->stIpv6PdnInfo.astPrefixList,
                          sizeof(pstNdisCfgUp->stIpv6PdnInfo.astPrefixList),
                          (VOS_VOID *)pstRcvMsg->stIpv6PdnInfo.astPrefixList,
                          sizeof(TAF_PDP_IPV6_PREFIX_STRU)*TAF_MAX_PREFIX_NUM_IN_RA);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv6PdnInfo.astPrefixList), sizeof(TAF_PDP_IPV6_PREFIX_STRU)*TAF_MAX_PREFIX_NUM_IN_RA);

    /* ��дINTERFACE��ȡIPV6��ַ�ĺ�8�ֽ�����дINTERFACE */
    lMemResult = memcpy_s((VOS_VOID*)pstNdisCfgUp->stIpv6PdnInfo.aucInterfaceId,
                          sizeof(pstNdisCfgUp->stIpv6PdnInfo.aucInterfaceId),
                          (VOS_VOID*)pstRcvMsg->stIpv6PdnInfo.aucInterfaceId,
                          sizeof(VOS_UINT8)*AT_NDIS_IPV6_IFID_LENGTH);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv6PdnInfo.aucInterfaceId), sizeof(VOS_UINT8)*AT_NDIS_IPV6_IFID_LENGTH);

    /* �������PCSCF��ַ  */
    pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.ucSerNum      = 0;
    if (pstRcvMsg->stIpv6PdnInfo.stPcscfSer.ucSerNum > 0)
    {
        pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.ucSerNum++;

        lMemResult = memcpy_s((VOS_VOID *)pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.aucPriServer,
                              sizeof(pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.aucPriServer),
                              pstRcvMsg->stIpv6PdnInfo.stPcscfSer.aucPriServer,
                              sizeof(pstRcvMsg->stIpv6PdnInfo.stPcscfSer.aucPriServer));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.aucPriServer), sizeof(pstRcvMsg->stIpv6PdnInfo.stPcscfSer.aucPriServer));
    }

    if (pstRcvMsg->stIpv6PdnInfo.stPcscfSer.ucSerNum > 1)
    {
        pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.ucSerNum++;

        lMemResult = memcpy_s((VOS_VOID *)pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.aucSecServer,
                              sizeof(pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.aucSecServer),
                              pstRcvMsg->stIpv6PdnInfo.stPcscfSer.aucSecServer,
                              sizeof(pstRcvMsg->stIpv6PdnInfo.stPcscfSer.aucSecServer));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstNdisCfgUp->stIpv6PdnInfo.stPcscfSer.aucSecServer), sizeof(pstRcvMsg->stIpv6PdnInfo.stPcscfSer.aucSecServer));
    }

    /* ������Ϣ */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstNdisCfgUp);

    return;
}


VOS_VOID AT_PS_ProcNdisIfaceUpCfg(
    const DSM_NDIS_IFACE_UP_IND_STRU   *pstRcvMsg
)
{
    if (pstRcvMsg->bitOpIpv4PdnInfo == VOS_TRUE)
    {
        AT_PS_SendNdisIPv4IfaceUpCfgInd(pstRcvMsg);
    }

    if (pstRcvMsg->bitOpIpv6PdnInfo == VOS_TRUE)
    {
        AT_PS_SendNdisIPv6IfaceUpCfgInd(pstRcvMsg);
    }

    return;
}


VOS_VOID AT_PS_ProcNdisIfaceDownCfg(
    const DSM_NDIS_IFACE_DOWN_IND_STRU *pstRcvMsg
)
{
    errno_t                             lMemResult;
#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    AT_NDIS_IFACE_DOWN_CONFIG_IND_STRU  *pstNdisCfgDown     = VOS_NULL_PTR;

    pstNdisCfgDown = (AT_NDIS_IFACE_DOWN_CONFIG_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_IFACE_DOWN_CONFIG_IND_STRU));
#else
    AT_NDIS_PDNINFO_REL_REQ_STRU        *pstNdisCfgDown     = VOS_NULL_PTR;

    pstNdisCfgDown = (AT_NDIS_PDNINFO_REL_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(
                            sizeof(AT_NDIS_PDNINFO_REL_REQ_STRU));
#endif

    if (pstNdisCfgDown == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_PS_ProcNdisIfaceDownCfg: alloc msg fail!");
        return;
    }

    /* ��ʼ����Ϣ */
    lMemResult = memset_s(AT_GET_MSG_ENTITY(pstNdisCfgDown), AT_GET_MSG_LENGTH(pstNdisCfgDown),
                          0x00, AT_GET_MSG_LENGTH(pstNdisCfgDown));
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_GET_MSG_LENGTH(pstNdisCfgDown), AT_GET_MSG_LENGTH(pstNdisCfgDown));

#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    /* ��д��Ϣͷ */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgDown, ID_AT_NDIS_IFACE_DOWN_CONFIG_IND);

    /* ��д��Ϣ�� */
    pstNdisCfgDown->bitOpIpv4PdnInfo    = pstRcvMsg->bitOpIpv4PdnInfo;
    pstNdisCfgDown->bitOpIpv6PdnInfo    = pstRcvMsg->bitOpIpv6PdnInfo;
    pstNdisCfgDown->ucIfaceId           = pstRcvMsg->ucIfaceId;
#else
    /* ��д��Ϣͷ */
    AT_CFG_NDIS_MSG_HDR(pstNdisCfgDown, ID_AT_NDIS_PDNINFO_REL_REQ);

    /* ��д��Ϣ�� */
    pstNdisCfgDown->enModemId           = pstRcvMsg->enModemId;
    pstNdisCfgDown->ucRabId             = pstRcvMsg->ucRabId;
#endif

    /* ������Ϣ */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstNdisCfgDown);

    return;
}


VOS_VOID AT_PS_ProcNdisConfigIpv6Dns(
    const DSM_NDIS_CONFIG_IPV6_DNS_IND_STRU    *pstRcvMsg
)
{
    NCM_IPV6DNS_S                       stIPv6Dns;
    VOS_INT32                           lRslt;
    errno_t                             lMemResult;

    stIPv6Dns.pu8Ipv6DnsInfo = (unsigned char*)PS_MEM_ALLOC(
                                    WUEPS_PID_AT,
                                    BSP_NCM_IPV6_DNS_LEN);

    if (stIPv6Dns.pu8Ipv6DnsInfo == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_PS_ProcNdisConfigIpv6Dns:Invalid stIPv6Dns.pu8Ipv6DnsInfo");
        return;
    }

    memset_s(stIPv6Dns.pu8Ipv6DnsInfo, BSP_NCM_IPV6_DNS_LEN, 0x00, BSP_NCM_IPV6_DNS_LEN);

    /* �ϱ��������DNS���ȹ̶�Ϊ32(Primary DNS LEN + Secondary DNS LEN) */
    stIPv6Dns.u32Length = BSP_NCM_IPV6_DNS_LEN;

    /*�����DNS����Ҫ����DRV�Ľӿ��ϱ�DNS��PC*/
    if (pstRcvMsg->bitOpIpv6PriDns == VOS_TRUE)
    {
        lMemResult = memcpy_s(stIPv6Dns.pu8Ipv6DnsInfo,
                              BSP_NCM_IPV6_DNS_LEN,
                              pstRcvMsg->aucIpv6PrimDns,
                              AT_MAX_IPV6_DNS_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, BSP_NCM_IPV6_DNS_LEN, AT_MAX_IPV6_DNS_LEN);
    }

    if (pstRcvMsg->bitOpIpv6SecDns == VOS_TRUE)
    {
        lMemResult = memcpy_s(stIPv6Dns.pu8Ipv6DnsInfo + AT_MAX_IPV6_DNS_LEN,
                              BSP_NCM_IPV6_DNS_LEN - AT_MAX_IPV6_DNS_LEN,
                              pstRcvMsg->aucIpv6SecDns,
                              AT_MAX_IPV6_DNS_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, BSP_NCM_IPV6_DNS_LEN - AT_MAX_IPV6_DNS_LEN, AT_MAX_IPV6_DNS_LEN);
    }

    /* ���õ�������DNS��Ϣ */
    lRslt = mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_SET_IPV6_DNS, &stIPv6Dns);
    if (lRslt != 0)
    {
        AT_ERR_LOG("AT_PS_ProcNdisConfigIpv6Dns, DRV_UDI_IOCTL Fail!" );
    }

    /* �ͷ�������ڴ� */
    PS_MEM_FREE(WUEPS_PID_AT, stIPv6Dns.pu8Ipv6DnsInfo);
    return;
}


VOS_VOID AT_RcvTafIfaceEvt(
    TAF_PS_EVT_STRU                     *pstEvt
)
{
    MN_PS_EVT_FUNC                      pEvtFunc    = VOS_NULL_PTR;
    TAF_CTRL_STRU                      *pstCtrl     = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucPortIndex;

    /* ��ʼ�� */
    pstCtrl     = (TAF_CTRL_STRU*)(pstEvt->aucContent);
    ulResult    = VOS_ERR;
    ucPortIndex = 0;

    if (At_ClientIdToUserId(pstCtrl->usClientId, &ucPortIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafIfaceEvt: usPortClientId At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucPortIndex))
    {
        /* �㲥IDNEX��������Ϊ�����±�ʹ�ã���Ҫ���¼�����������ϸ�˶ԣ���������Խ�硣
           Ŀǰû�й㲥�¼�������ϸ�˶ԣ� */
        AT_WARN_LOG("AT_RcvTafIfaceEvt: AT_BROADCAST_INDEX,but not Broadcast Event.");
        return;
    }

    /* ���¼�������в��Ҵ����� */
    for (i = 0; i < AT_ARRAY_SIZE(g_astAtIfaceEvtFuncTbl); i++)
    {
        if ( pstEvt->ulEvtId == g_astAtIfaceEvtFuncTbl[i].ulEvtId )
        {
            /* �¼�IDƥ�� */
            pEvtFunc = g_astAtIfaceEvtFuncTbl[i].pEvtFunc;
            break;
        }
    }

    /* ������������������ */
    if (pEvtFunc != VOS_NULL_PTR)
    {
        ulResult = pEvtFunc(ucPortIndex, pstEvt->aucContent);
    }

    if (ulResult != VOS_OK)
    {
        AT_ERR_LOG1("AT_RcvTafIfaceEvt: Can not handle this message! <MsgId>", pstEvt->ulEvtId);
    }

    return;
}


VOS_UINT32 AT_RcvTafIfaceEvtIfaceUpCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_UP_CNF_STRU              *pstIfaceUpCnf   = VOS_NULL_PTR;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NDISDUP_SET)
    {
        AT_WARN_LOG("AT_RcvTafIfaceEvtIfaceUpCnf : Current Option is not AT_CMD_NDISDUP_SET.");
        return VOS_ERR;
    }

    ulResult        = AT_OK;
    pstIfaceUpCnf   = (TAF_IFACE_UP_CNF_STRU*)pEvtInfo;

    if (pstIfaceUpCnf->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        AT_PS_SetPsCallErrCause(ucIndex, pstIfaceUpCnf->enCause);

        ulResult = AT_ERROR;

        if (pstIfaceUpCnf->enCause == TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT)
        {
            ulResult = AT_CME_PDP_ACT_LIMIT;
        }

        if (pstIfaceUpCnf->ucCid <= TAF_MAX_CID)
        {
            pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(ucIndex);
            pstPsModemCtx->astChannelCfg[pstIfaceUpCnf->ucCid].enPortIndex = AT_CLIENT_BUTT;
        }

        AT_WARN_LOG1("AT_RcvTafIfaceEvtIfaceUpCnf: <enCause> is ", pstIfaceUpCnf->enCause);
    }

    /* ��������״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafIfaceEvtIfaceDownCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_DOWN_CNF_STRU            *pstIfaceDownCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NDISDUP_SET)
    {
        AT_WARN_LOG("AT_RcvTafIfaceEvtIfaceDownCnf : Current Option is not AT_CMD_NDISDUP_SET.");
        return VOS_ERR;
    }

    ulResult        = AT_OK;
    pstIfaceDownCnf = (TAF_IFACE_DOWN_CNF_STRU*)pEvtInfo;

    if (pstIfaceDownCnf->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        AT_WARN_LOG1("AT_RcvTafIfaceEvtIfaceDownCnf: <enCause> is ", pstIfaceDownCnf->enCause);
        ulResult = AT_ERROR;
    }

    /* ��������״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafIfaceEvtIfaceStatusInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_STATUS_IND_STRU          *pstIfaceStatusInd   = VOS_NULL_PTR;
    AT_PS_RPT_IFACE_RSLT_FUNC           pRptIfaceRsltFunc;

    pstIfaceStatusInd = (TAF_IFACE_STATUS_IND_STRU*)pEvtInfo;

    if (pstIfaceStatusInd->enStatus == TAF_IFACE_STATUS_ACT)
    {
        AT_PS_SetPsCallErrCause(pstIfaceStatusInd->stCtrl.usClientId, TAF_PS_CAUSE_SUCCESS);
    }

    if (pstIfaceStatusInd->enStatus == TAF_IFACE_STATUS_DEACT)
    {
        AT_PS_SetPsCallErrCause(pstIfaceStatusInd->stCtrl.usClientId, pstIfaceStatusInd->enCause);
    }

    /* ���¼�������в��Ҵ����� */
    pRptIfaceRsltFunc = AT_PS_GetRptIfaceResultFunc(pstIfaceStatusInd->enUserType);

    /* ������������������ */
    if (pRptIfaceRsltFunc != VOS_NULL_PTR)
    {
        pRptIfaceRsltFunc(pstIfaceStatusInd);
    }
    else
    {
        AT_ERR_LOG("AT_RcvTafIfaceEvtIfaceStatusInd:ERROR: User type or enStatus is invalid!");
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafIfaceEvtDataChannelStateInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_DATA_CHANNEL_STATE_IND_STRU  *pstDataChannelStateInd  = VOS_NULL_PTR;
    AT_PS_DATA_CHANL_CFG_STRU              *pstChanCfg              = VOS_NULL_PTR;
    VOS_UINT8                               ucCid;

    pstDataChannelStateInd  = (TAF_IFACE_DATA_CHANNEL_STATE_IND_STRU*)pEvtInfo;
    ucCid                   = pstDataChannelStateInd->ucCid;

    /* ���CID�Ϸ��� */
    if ( ucCid > TAF_MAX_CID)
    {
        AT_ERR_LOG1("AT_RcvTafIfaceEvtDataChannelStateInd, WARNING, CID error:%d", ucCid);
        return VOS_ERR;
    }

    pstChanCfg = AT_PS_GetDataChanlCfg(pstDataChannelStateInd->stCtrl.usClientId, ucCid);

    /* ��������δʹ�� */
    if (pstChanCfg->ulUsed == VOS_FALSE)
    {
        AT_WARN_LOG("AT_RcvTafIfaceEvtDataChannelStateInd: Channel is not config!");
        AT_CleanDataChannelCfg(pstChanCfg);
        return VOS_ERR;
    }

    if (pstDataChannelStateInd->bitOpActFlg == VOS_TRUE)
    {
        /* ��IFACE�����־���� */
        pstChanCfg->ulIfaceActFlg = VOS_TRUE;
        return VOS_OK;
    }

    if (pstDataChannelStateInd->bitOpCleanFlg == VOS_TRUE)
    {
        /* ���CID������ͨ����ӳ���ϵ */
        AT_CleanDataChannelCfg(pstChanCfg);
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafIfaceEvtDataChannelStateInd: state is not act or clean!");
    return VOS_ERR;
}


VOS_UINT32 AT_RcvTafIfaceEvtUsbNetOperInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_USBNET_OPER_IND_STRU          *pstUsbNetOperInd = VOS_NULL_PTR;

    pstUsbNetOperInd   = (TAF_IFACE_USBNET_OPER_IND_STRU*)pEvtInfo;

    if (pstUsbNetOperInd->bitOpActUsbNet == VOS_TRUE)
    {
        AT_PS_ActiveUsbNet();
        return VOS_OK;
    }

    if (pstUsbNetOperInd->bitOpDeactUsbNet == VOS_TRUE)
    {
        AT_PS_DeActiveUsbNet();
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafIfaceEvtUsbNetOperInd: oper is not act or deact usb net!");
    return VOS_ERR;
}


VOS_UINT32 AT_RcvTafIfaceEvtRegFcInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_REG_FC_IND_STRU          *pstRegFcInd         = VOS_NULL_PTR;

    pstRegFcInd         = (TAF_IFACE_REG_FC_IND_STRU*)pEvtInfo;

    if (!AT_PS_IS_RABID_VALID(pstRegFcInd->ucRabId))
    {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRegFcInd:ERROR: RABID is invalid!");
        return VOS_ERR;
    }

    /* APP PS CALL���� */
    if (pstRegFcInd->enUserType == TAF_IFACE_USER_TYPE_APP)
    {
        AT_PS_ProcAppRegFcPoint(pstRegFcInd);
        return VOS_OK;
    }

    /* NDIS PS CALL���� */
    if (pstRegFcInd->enUserType == TAF_IFACE_USER_TYPE_NDIS)
    {
        AT_PS_ProcNdisRegFcPoint(pstRegFcInd);
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafIfaceEvtRegFcInd: User Type is not APP or NDIS!");
    return VOS_ERR;
}


VOS_UINT32 AT_RcvTafIfaceEvtDeRegFcInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_DEREG_FC_IND_STRU        *pstDeRegFcInd = VOS_NULL_PTR;

    pstDeRegFcInd         = (TAF_IFACE_DEREG_FC_IND_STRU*)pEvtInfo;

    if (!AT_PS_IS_RABID_VALID(pstDeRegFcInd->ucRabId))
    {
        AT_ERR_LOG("AT_RcvTafIfaceEvtDeRegFcInd:ERROR: RABID is invalid!");
        return VOS_ERR;
    }

    /* APP PS CALL���� */
    if (pstDeRegFcInd->enUserType == TAF_IFACE_USER_TYPE_APP)
    {
        AT_PS_ProcAppDeRegFcPoint(pstDeRegFcInd);
        return VOS_OK;
    }

    /* NDIS PS CALL���� */
    if (pstDeRegFcInd->enUserType == TAF_IFACE_USER_TYPE_NDIS)
    {
        AT_PS_ProcNdisDeRegFcPoint(pstDeRegFcInd);
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafIfaceEvtDeRegFcInd: User Type is not APP or NDIS!");
    return VOS_ERR;
}


VOS_UINT32 AT_RcvTafIfaceEvtRabInfoInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_IFACE_RAB_INFO_IND_STRU    *pstRabInfoInd = VOS_NULL_PTR;
    AT_MODEM_PS_CTX_STRU           *pstPsModemCtx = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16            usModemId;

    usModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(ucIndex, &usModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: Get modem id fail.");
        return VOS_ERR;
    }

    pstRabInfoInd = (TAF_IFACE_RAB_INFO_IND_STRU*)pEvtInfo;

    if (!AT_PS_IS_RABID_VALID(pstRabInfoInd->ucNewRabId))
    {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: New RabId is invalid.");
        return VOS_ERR;
    }

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (pstRabInfoInd->enIfaceId >= PS_IFACE_ID_BUTT)
    {
        AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: enIfaceId is invalid.");
        return VOS_ERR;
    }
#endif

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(ucIndex);

    switch(pstRabInfoInd->enOperType)
    {
        case TAF_IFACE_RAB_OPER_ADD:
            /* ����Ϊ��չRABID = modemId + rabId */
            gastAtClientTab[ucIndex].ucExPsRabId  = AT_BUILD_EXRABID(usModemId, pstRabInfoInd->ucNewRabId);

            if (pstRabInfoInd->bitOpPdpAddr)
            {
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
                pstPsModemCtx->aulIpAddrIfaceIdMap[pstRabInfoInd->enIfaceId] = pstRabInfoInd->ulIpAddr;
#else
                pstPsModemCtx->aulIpAddrRabIdMap[pstRabInfoInd->ucNewRabId - AT_PS_RABID_OFFSET] = pstRabInfoInd->ulIpAddr;
#endif
            }
            break;

        case TAF_IFACE_RAB_OPER_DELETE:
            gastAtClientTab[ucIndex].ucExPsRabId = 0;
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
            if ((pstRabInfoInd->enPdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4)
            {
                pstPsModemCtx->aulIpAddrIfaceIdMap[pstRabInfoInd->enIfaceId] = 0;
            }
#else
            pstPsModemCtx->aulIpAddrRabIdMap[pstRabInfoInd->ucNewRabId - AT_PS_RABID_OFFSET] = 0;
#endif
            break;

        case TAF_IFACE_RAB_OPER_CHANGE:
            if (!AT_PS_IS_RABID_VALID(pstRabInfoInd->ucOldRabId))
            {
                AT_ERR_LOG("AT_RcvTafIfaceEvtRabInfoInd: Old RabId is invalid.");
                return VOS_ERR;
            }

            /* ����Ϊ��չRABID = modemId + rabId */
            gastAtClientTab[ucIndex].ucExPsRabId  = AT_BUILD_EXRABID(usModemId, pstRabInfoInd->ucNewRabId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM != FEATURE_ON)
            pstPsModemCtx->aulIpAddrRabIdMap[pstRabInfoInd->ucNewRabId - AT_PS_RABID_OFFSET] = pstPsModemCtx->aulIpAddrRabIdMap[pstRabInfoInd->ucOldRabId - AT_PS_RABID_OFFSET];
            pstPsModemCtx->aulIpAddrRabIdMap[pstRabInfoInd->ucOldRabId - AT_PS_RABID_OFFSET] = 0;
#endif

            break;

        default:
            AT_WARN_LOG("AT_RcvTafIfaceEvtRabInfoInd: enOperType is invalid!");
            break;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtEpdgCtrluNtf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_EPDG_CTRLU_NTF_STRU         *pstEpdgCtrluNtf = VOS_NULL_PTR;
    VOS_UINT8                           ucBroadCastIndex;

    pstEpdgCtrluNtf  = (TAF_PS_EPDG_CTRLU_NTF_STRU*)pEvtInfo;

    if (At_ClientIdToUserBroadCastId(pstEpdgCtrluNtf->stCtrl.usClientId, &ucBroadCastIndex) != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_RcvTafPsEvtEpdgCtrluNtf: At_ClientIdToUserBroadCastId is err!");
        return VOS_ERR;
    }

    if (pstEpdgCtrluNtf->stEpdgCtrlu.bitOpActReq == VOS_TRUE)
    {
        AT_PS_ReportImsCtrlMsgu(ucBroadCastIndex,
                                AT_IMS_CTRL_MSG_RECEIVE_MODULE_NON_IMSA,
                                (VOS_UINT32)(sizeof(TAF_PS_EPDG_ACT_REQ_INFO_STRU)),
                                (VOS_UINT8 *)(&(pstEpdgCtrluNtf->stEpdgCtrlu.stActReqInfo)));
        return VOS_OK;
    }

    if (pstEpdgCtrluNtf->stEpdgCtrlu.bitOpDeActReq == VOS_TRUE)
    {
        AT_PS_ReportImsCtrlMsgu(ucBroadCastIndex,
                                AT_IMS_CTRL_MSG_RECEIVE_MODULE_NON_IMSA,
                                (VOS_UINT32)(sizeof(TAF_PS_EPDG_DEACT_REQ_INFO_STRU)),
                                (VOS_UINT8 *)(&(pstEpdgCtrluNtf->stEpdgCtrlu.stDeActReqInfo)));
        return VOS_OK;
    }

    AT_WARN_LOG("AT_RcvTafPsEvtEpdgCtrluNtf: not ACT or DEACT REQ!");
    return VOS_ERR;
}


VOS_UINT32 AT_RcvTafIfaceEvtDyamicParaCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvt
)
{
    MN_PS_EVT_FUNC                      pEvtFunc    = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT32                          ulResult;

    /* ���¼�������в��Ҵ����� */
    for (i = 0; i < AT_ARRAY_SIZE(g_astAtDynamicInfoReportFuncTbl); i++)
    {
        if ( gastAtClientTab[ucIndex].CmdCurrentOpt == g_astAtDynamicInfoReportFuncTbl[i].ulCmdCurrentOpt )
        {
            /* �¼�IDƥ�� */
            pEvtFunc = g_astAtDynamicInfoReportFuncTbl[i].pEvtFunc;
            break;
        }
    }

    /* ������������������ */
    ulResult = VOS_ERR;
    if (pEvtFunc != VOS_NULL_PTR)
    {
        ulResult = pEvtFunc(ucIndex, pstEvt);
    }

    if (ulResult != VOS_OK)
    {
        AT_ERR_LOG1("AT_RcvTafIfaceEvtDyamicInfoCnf: Can not handle this message! <CmdCurrentOpt>", gastAtClientTab[ucIndex].CmdCurrentOpt);
    }

    return ulResult;
}



