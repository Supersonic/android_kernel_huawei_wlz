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
#include "TafAcoreLogPrivacy.h"
#include "TafLogPrivacyMatch.h"
#include "AtInternalMsg.h"
#include "AtParse.h"
#include "AtCtx.h"
#include "TafAppXsmsInterface.h"
#include "AtMtaInterface.h"
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
#include "AtXpdsInterface.h"
#endif
#include "TafDrvAgent.h"
#include "PsLogFilterInterface.h"
#include "MnMsgTs.h"
#include "TafCcmApi.h"
#include "AtRnicInterface.h"
#include "AtNdisInterface.h"
#include "RnicCdsInterface.h"
#include "dsm_rnic_pif.h"
#include "dsm_ndis_pif.h"
#include "AtParseCore.h"
#include "securec.h"


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/

#define THIS_FILE_ID                    PS_FILE_ID_TAF_ACORE_LOG_PRIVACY_C

#ifndef AT_SMS_MODE
#define    AT_SMS_MODE         (1)
#endif

#define    AT_XML_MODE         (2)

#define LOG_PRIVACY_AT_CMD_MAX_LEN         (50)                                 /* AT����,������󳤶� */

LOCAL VOS_UINT32 TAF_LogPrivacyGetModem0Pid(
    VOS_UINT32                          ulSrcPid
);

LOCAL VOS_UINT32 AT_PrivacyMatchIsWhiteListAtCmd(
    AT_MSG_STRU                        *pstAtMsg
);
LOCAL AT_MSG_STRU* AT_PrivacyFilterCimi(
    AT_MSG_STRU                        *pstMsg
);

LOCAL AT_MSG_STRU* AT_PrivacyFilterCgsn(
    AT_MSG_STRU                        *pstMsg
);

LOCAL AT_MSG_STRU* AT_PrivacyFilterMsid(
    AT_MSG_STRU                        *pstMsg
);

LOCAL AT_MSG_STRU* AT_PrivacyFilterHplmn(
    AT_MSG_STRU                        *pstMsg
);

LOCAL VOS_VOID* AT_PrivacyMatchNdisPdnInfoCfgReq(
    MsgBlock                                               *pstMsg
);

LOCAL VOS_VOID* AT_PrivacyMatchNdisIfaceUpConfigReq(
    MsgBlock                                               *pstMsg
);

/*****************************************************************************
  3 ȫ�ֱ�������
*****************************************************************************/

TAF_LOG_PRIVACY_MATCH_MODEM_PID_MAP_TBL_STRU                g_astTafPrivacyMatchModemPidMapTbl[] =
{
    {I0_WUEPS_PID_TAF,  I1_WUEPS_PID_TAF,   I2_WUEPS_PID_TAF},
    {I0_UEPS_PID_XSMS,  I1_UEPS_PID_XSMS,   I2_UEPS_PID_XSMS},
    {I0_WUEPS_PID_USIM, I1_WUEPS_PID_USIM,  I2_WUEPS_PID_USIM},
    {I0_MAPS_STK_PID,   I1_MAPS_STK_PID,    I2_MAPS_STK_PID},
    {I0_UEPS_PID_MTA,   I1_UEPS_PID_MTA,    I2_UEPS_PID_MTA},
    {I0_WUEPS_PID_DRV_AGENT, I1_WUEPS_PID_DRV_AGENT, I2_WUEPS_PID_DRV_AGENT},

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
    {I0_UEPS_PID_XPDS,  I1_UEPS_PID_XPDS,   I2_UEPS_PID_XPDS},
#endif
    {I0_WUEPS_PID_MMA,  I1_WUEPS_PID_MMA,   I2_WUEPS_PID_MMA},
    {I0_UEPS_PID_DSM,   I1_UEPS_PID_DSM,    I2_UEPS_PID_DSM},

#if (FEATURE_ON == FEATURE_IMS)
    {I0_PS_PID_IMSA,    I1_PS_PID_IMSA,     I0_PS_PID_IMSA},
#endif

    {I0_UEPS_PID_CCM,   I1_UEPS_PID_CCM,    I2_UEPS_PID_CCM},
};

/* ������������Ϣ��at�ڲ���Ϣ������ */
AT_INTER_MSG_ID_ENUM_UINT32                                 g_aenAtCmdWhiteListTbl[] =
{
    ID_AT_MNTN_INPUT_MSC,
    ID_AT_MNTN_START_FLOW_CTRL,
    ID_AT_MNTN_STOP_FLOW_CTRL,
    ID_AT_MNTN_REG_FC_POINT,
    ID_AT_MNTN_DEREG_FC_POINT,
    ID_AT_MNTN_PC_REPLAY_MSG,
    ID_AT_MNTN_PC_REPLAY_CLIENT_TAB,
    ID_AT_MNTN_RPT_PORT,
    ID_AT_MNTN_PS_CALL_ENTITY_RPT,
    ID_AT_COMM_CCPU_RESET_START,
    ID_AT_COMM_CCPU_RESET_END,
    ID_AT_COMM_HIFI_RESET_START,
    ID_AT_COMM_HIFI_RESET_END,
    ID_AT_NCM_CONN_STATUS_CMD,
    ID_AT_WATER_LOW_CMD,
    ID_AT_SWITCH_CMD_MODE,
};

AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU                    g_astPrivacyMatchAtCmdMapTbl[] =
{
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    /* �������AT���� */
    {"AT^CFSH"                  ,   "AT^CFSH"},
    {"AT^CBURSTDTMF"            ,   "AT^CBURSTDTMF"},
    {"AT^CCONTDTMF"             ,   "AT^CCONTDTMF"},
    {"AT^ECCALL"                ,   "AT^ECCALL"},
    {"AT^CUSTOMDIAL"            ,   "AT^CUSTOMDIAL"},
    {"AT^ECRANDOM"              ,   "AT^ECRANDOM"},
    {"AT^ECKMC"                 ,   "AT^ECKMC"},
    /* ������д���������AT���� */
    {"AT^CCMGS"                 ,   "AT^CCMGS"},
    {"AT^CCMGW"                 ,   "AT^CCMGW"},
    {"AT^CCMGD"                 ,   "AT^CCMGD"},
    {"AT^MEID"                  ,   "AT^MEID"},
    {"AT^CIMEI"                 ,   "AT^CIMEI"},

    /* XPDS��� */
    {"AT^CAGPSPRMINFO"          ,   "AT^CAGPSPRMINFO"},
    {"AT^CAGPSPOSINFO"          ,   "AT^CAGPSPOSINFO"},
    {"AT^CAGPSFORWARDDATA"      ,   "AT^CAGPSFORWARDDATA"},
    {"AT^CAGPSSTART"            ,   "AT^CAGPSSTART"},
#endif

    /* �����д���tmsi/ptmsi */
    { "AT^EFLOCIINFO", "AT^EFLOCIINFO" },
    { "AT^EFPSLOCIINFO", "AT^EFPSLOCIINFO" },
	
    {"AT^PHYNUM"                ,   "AT^PHYNUM"},
    {"AT^PHONEPHYNUM"           ,   "AT^PHONEPHYNUM"},
    {"AT^EPDU"                  ,   "AT^EPDU"},

    /* �绰�������� */
    {"AT^NVM"                   ,   "AT^NVM"},

    /* ��·��ҵ������ */
    {"ATD"                      ,   "ATD"},
    {"AT^APDS"                  ,   "AT^APDS"},
    {"AT+VTS"                   ,   "AT+VTS"},
    {"AT^DTMF"                  ,   "AT^DTMF"},
#if (FEATURE_ON == FEATURE_ECALL)
    {"AT^ECLSTART"              ,   "AT^ECLSTART"},
#endif
#if (FEATURE_ON == FEATURE_IMS)
    {"AT^CACMIMS"               ,   "AT^CACMIMS"},
    {"AT^ECONFDIAL"             ,   "AT^ECONFDIAL"},
#endif

    /* ����ҵ������ */
    {"AT+CCFC"                  ,   "AT+CCFC"},
    {"AT+CTFR"                  ,   "AT+CTFR"},
    {"AT^CHLD"                  ,   "AT^CHLD"},
    {"AT+CUSD"                  ,   "AT+CUSD"},
    /* SMS����ҵ������ */
    {"AT+CMGS"                  ,   "AT+CMGS"},
    {"AT+CMGW"                  ,   "AT+CMGW"},
    {"AT+CMGC"                  ,   "AT+CMGC"},
    {"AT+CMSS"                  ,   "AT+CMSS"},
    {"AT^RSTRIGGER"             ,   "AT^RSTRIGGER"},

    /* ��AP�Խ����� */
    {"AT+CPOS"                  ,   "AT+CPOS"},
    {"AT+CPBS"                  ,   "AT+CPBS"},
    {"AT+CPBR"                  ,   "AT+CPBR"},
    {"AT+CPBW"                  ,   "AT+CPBW"},
    {"AT+CPBF"                  ,   "AT+CPBF"},
    {"AT^EFLOCIINFO"            ,   "AT^EFLOCIINFO"},
    {"AT^EFPSLOCIINFO"          ,   "AT^EFPSLOCIINFO"},
    {"AT^AUTHDATA"              ,   "AT^AUTHDATA"},
    {"AT^CGDNS"                 ,   "AT^CGDNS"},
    {"AT+CGTFT"                 ,   "AT+CGTFT"},
    {"AT+CGDCONT"               ,   "AT+CGDCONT"},
    {"AT^NDISDUP"               ,   "AT^NDISDUP"},
};

AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU                    g_astPrivacyMatchRptAtCmdMapTbl[] =
{
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    /* �������AT���� */
    {"\r\n^CFSH"                ,   "\r\n^CFSH\r\n"},
    {"\r\n^CBURSTDTMF"          ,   "\r\n^CBURSTDTMF\r\n"},
    {"\r\n^CCONNNUM"            ,   "\r\n^CCONNNUM\r\n"},
    {"\r\n^CCALLEDNUM"          ,   "\r\n^CCALLEDNUM\r\n"},
    {"\r\n^CCALLINGNUM"         ,   "\r\n^CCALLINGNUM\r\n"},
    {"\r\n^CREDIRNUM"           ,   "\r\n^CREDIRNUM\r\n"},
    {"\r\n^CCWAC"               ,   "\r\n^CCWAC\r\n"},
    {"\r\n^CDISP"               ,   "\r\n^CDISP\r\n"},
    {"\r\n^CCONTDTMF"           ,   "\r\n^CCONTDTMF\r\n"},
    {"\r\n^ECCALL"              ,   "\r\n^ECCALL\r\n"},
    {"\r\n^ECRANDOM"            ,   "\r\n^ECRANDOM\r\n"},
    {"\r\n^ECKMC"               ,   "\r\n^ECKMC\r\n"},

    /* ������д���������AT���� */
    {"\r\n^CCMT"                ,   "\r\n^CCMT\r\n"},
    {"\r\n^CCMGW"               ,   "\r\n^CCMGW\r\n"},
    {"\r\n^MEID"                ,   "\r\n^MEID\r\n"},
    {"\r\n^GETESN"              ,   "\r\n^GETESN\r\n"},
    {"\r\n^GETMEID"             ,   "\r\n^GETMEID\r\n"},

    { "\r\n^EFLOCIINFO", "\r\n^EFLOCIINFO\r\n" },
    { "\r\n^EFPSLOCIINFO", "\r\n^EFPSLOCIINFO\r\n" },

    /* XPDS��� */
    {"\r\n^CAGPSPRMINFO"        ,   "\r\n^CAGPSPRMINFO\r\n"},
    {"\r\n^CAGPSPOSINFO"        ,   "\r\n^CAGPSPOSINFO\r\n"},
    {"\r\n^CAGPSFORWARDDATA"    ,   "\r\n^CAGPSFORWARDDATA\r\n"},
    {"\r\n^CAGPSREVERSEDATA"    ,   "\r\n^CAGPSREVERSEDATA\r\n"},
    {"\r\n^CAGPSREFLOCINFO"     ,   "\r\n^CAGPSREFLOCINFO\r\n"},
    {"\r\n^CAGPSPDEPOSINFO"     ,   "\r\n^CAGPSPDEPOSINFO\r\n"},
    {"\r\n^CAGPSACQASSISTINFO"  ,   "\r\n^CAGPSACQASSISTINFO\r\n"},
    {"\r\n^CAGPSIONINFO"        ,   "\r\n^CAGPSIONINFO\r\n"},
    {"\r\n^CAGPSEPHINFO"        ,   "\r\n^CAGPSEPHINFO\r\n"},
    {"\r\n^CAGPSALMINFO"        ,   "\r\n^CAGPSALMINFO\r\n"},
    {"\r\n^UTSGPSPOSINFO"       ,   "\r\n^UTSGPSPOSINFO\r\n"},
#endif

    {"\r\n^SCID"                ,   "\r\n^SCID\r\n"},
    {"\r\n^PHYNUM"              ,   "\r\n^PHYNUM\r\n"},
    {"\r\n^PHONEPHYNUM"         ,   "\r\n^PHONEPHYNUM\r\n"},
    {"\r\n^HVSCONT"             ,   "\r\n^HVSCONT\r\n"},
    {"\r\n^ICCID"               ,   "\r\n^ICCID\r\n"},
    {"\r\n^EPDUR"               ,   "\r\n^EPDUR\r\n"},

    /* �绰�������� */
    {"\r\n^XLEMA"               ,   "\r\n^XLEMA\r\n"},

#if (FEATURE_ON == FEATURE_IMS)
    /* ��·��ҵ������ */
    {"\r\n^CLCCECONF"           ,   "\r\n^CLCCECONF\r\n"},
    {"\r\n^CSSU"                ,   "\r\n^CSSU\r\n"},
    {"\r\n^CSSUEX"              ,   "\r\n^CSSUEX\r\n"},
    {"\r\n^ECONFDIAL"           ,   "\r\n^ECONFDIAL\r\n"},
    {"\r\n^ECONFERR"            ,   "\r\n^ECONFERR\r\n"},
    {"\r\n^VOLTEIMPU"           ,   "\r\n^VOLTEIMPU\r\n"},
    {"\r\n^IMSMTRPT"            ,   "\r\n^IMSMTRPT\r\n"},
    {"\r\n^VOLTEIMPI"           ,   "\r\n^VOLTEIMPI\r\n"},
    {"\r\n^DMUSER"              ,   "\r\n^DMUSER\r\n"},
#endif
    {"\r\n+CLCC"                ,   "\r\n+CLCC\r\n"},
    {"\r\n^CLCC"                ,   "\r\n^CLCC\r\n"},
    {"\r\n^CLPR"                ,   "\r\n^CLPR\r\n"},

    /* ����ҵ������ */
    {"\r\n+CLIP"                ,   "\r\n+CLIP\r\n"},
    {"\r\n+CCFC"                ,   "\r\n+CCFC\r\n"},
    {"\r\n+COLP"                ,   "\r\n+COLP\r\n"},
    {"\r\n+CUSS"                ,   "\r\n+CUSS\r\n"},
#if (FEATURE_ON == FEATURE_IMS)
    {"\r\n^CUSS"                ,   "\r\n^CUSS\r\n"},
    {"\r\n^CCWA"                ,   "\r\n^CCWA\r\n"},
    {"\r\n^CSSI"                ,   "\r\n^CSSI\r\n"},
#endif
    {"\r\n+CSSU"                ,   "\r\n+CSSU\r\n"},
    {"\r\n+CCWA"                ,   "\r\n+CCWA\r\n"},
    {"\r\n+CNAP"                ,   "\r\n+CNAP\r\n"},
    {"\r\n^CNAP"                ,   "\r\n^CNAP\r\n"},
    {"\r\n+CMOLRN"              ,   "\r\n+CMOLRN\r\n"},
    {"\r\n+CMOLRG"              ,   "\r\n+CMOLRG\r\n"},

    /* SMS����ҵ������ */
    {"\r\n+CMT"                 ,   "\r\n+CMT\r\n"},
    {"\r\n+CDS"                 ,   "\r\n+CDS\r\n"},
    {"\r\n+CMGR"                ,   "\r\n+CMGR\r\n"},
    {"\r\n+CMGL"                ,   "\r\n+CMGL\r\n"},
    {"\r\n^RSTRIGGER"           ,   "\r\n^RSTRIGGER\r\n"},

    /* ��AP�Խ����� */
    {"\r\n+CPOSR"               ,   "\r\n+CPOSR\r\n"},
    {"\r\n^DIALOGNTF"           ,   "\r\n^DIALOGNTF\r\n"},
    {"\r\n^NVRD"           ,        "\r\n^NVRD\r\n"},

    {"\r\n+CPBS"                ,   "\r\n+CPBS\r\n"},
    {"\r\n+CPBR"                ,   "\r\n+CPBR\r\n"},
    {"\r\n+CPBW"                ,   "\r\n+CPBW\r\n"},
    {"\r\n+CPBF"                ,   "\r\n+CPBF\r\n"},
    {"\r\n^EFLOCIINFO"          ,   "\r\n^EFLOCIINFO\r\n"},
    {"\r\n^EFPSLOCIINFO"        ,   "\r\n^EFPSLOCIINFO\r\n"},
    {"\r\n^LOCINFO"             ,   "\r\n^LOCINFO\r\n"},
    {"\r\n^CLOCINFO"            ,   "\r\n^CLOCINFO\r\n"},
    {"\r\n^LOCINFO"             ,   "\r\n^LOCINFO\r\n"},
    {"\r\n+CGDCONT"             ,   "\r\n+CGDCONT\r\n"},
    {"\r\n+CGPADDR"             ,   "\r\n+CGPADDR\r\n"},
    {"\r\n^CGCONTRDP"           ,   "\r\n^CGCONTRDP\r\n"},
    {"\r\n^CGTFTRDP"            ,   "\r\n^CGTFTRDP\r\n"},
    {"\r\n^AUTHDATA"            ,   "\r\n^AUTHDATA\r\n"},
    {"\r\n^CGDNS"               ,   "\r\n^CGDNS\r\n"},
    {"\r\n+CGTFT"               ,   "\r\n+CGTFT\r\n"},
    {"\r\n^DNSQUERY"            ,   "\r\n^DNSQUERY\r\n"},
    {"\r\n^SRCHEDPLMNINFO"      ,   "\r\n^SRCHEDPLMNINFO\r\n"},
    {"\r\n+CREG"                ,   "\r\n+CREG\r\n"},
    {"\r\n+CGREG"               ,   "\r\n+CGREG\r\n"},
    {"\r\n+CEREG"               ,   "\r\n+CEREG\r\n"},
    {"\r\n+ECID"                ,   "\r\n+ECID\r\n"},
    {"\r\n^REJINFO"             ,   "\r\n^REJINFO\r\n"},
    {"\r\n^NETSCAN"             ,   "\r\n^NETSCAN\r\n"},
    {"\r\n^MONSC"               ,   "\r\n^MONSC\r\n"},
    {"\r\n^MONNC"               ,   "\r\n^MONNC\r\n"},
    {"\r\n^PSEUCELL"            ,   "\r\n^PSEUCELL\r\n"},
    {"\r\n^SIMLOCKNWDATAWRITE"  ,   "\r\n^SIMLOCKNWDATAWRITE\r\n"},
    {"\r\n^IMSCTRLMSG"          ,   "\r\n^IMSCTRLMSG\r\n"},
    {"\r\n^IMSCTRLMSGU"         ,   "\r\n^IMSCTRLMSGU\r\n"},
    {"\r\n^NICKNAME"            ,   "\r\n^NICKNAME\r\n"},
};

AT_LOG_PRIVACY_MAP_CMD_TO_FUNC_STRU                         g_astPrivacyMapCmdToFuncTbl[] =
{
    {TAF_LOG_PRIVACY_AT_CMD_CIMI                ,           AT_PrivacyFilterCimi},
    {TAF_LOG_PRIVACY_AT_CMD_CGSN                ,           AT_PrivacyFilterCgsn},
    {TAF_LOG_PRIVACY_AT_CMD_MSID                ,           AT_PrivacyFilterMsid},
    {TAF_LOG_PRIVACY_AT_CMD_HPLMN               ,           AT_PrivacyFilterHplmn},
};

/**********************************************************************************************/
/***************************** WUEPS_PID_AT������Ϣ������������� *****************************/
/* AT���͸�GUC TAFģ����Ϣ������������ */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToTafMsgListTbl[] =
{
    {TAF_MSG_REGISTERSS_MSG,                                AT_PrivacyMatchRegisterSsMsg},
    {TAF_MSG_PROCESS_USS_MSG,                               AT_PrivacyMatchProcessUssMsg},
    {TAF_MSG_INTERROGATESS_MSG,                             AT_PrivacyMatchInterRogateMsg},
    {TAF_MSG_ERASESS_MSG,                                   AT_PrivacyMatchErasessMsg},
    {TAF_MSG_ACTIVATESS_MSG,                                AT_PrivacyMatchActivatessMsg},
    {TAF_MSG_DEACTIVATESS_MSG,                              AT_PrivacyMatchDeactivatessMsg},
    {TAF_MSG_REGPWD_MSG,                                    AT_PrivacyMatchRegPwdMsg},


    {MN_MSG_MSGTYPE_SEND_RPDATA_DIRECT,                     AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_SEND_RPDATA_FROMMEM,                    AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_WRITE,                                  AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_SEND_RPRPT,                             AT_PrivacyMatchSmsAppMsgReq},
    {MN_MSG_MSGTYPE_WRITE_SRV_PARA,                         AT_PrivacyMatchSmsAppMsgReq},
};

/* AT����XSMSģ����Ϣ�������������� */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToXsmsMsgListTbl[] =
{
    {TAF_XSMS_APP_MSG_TYPE_SEND_REQ,                        AT_PrivacyMatchAppMsgTypeSendReq},
    {TAF_XSMS_APP_MSG_TYPE_WRITE_REQ,                       AT_PrivacyMatchAppMsgTypeWriteReq},
    {TAF_XSMS_APP_MSG_TYPE_DELETE_REQ,                      AT_PrivacyMatchAppMsgTypeDeleteReq},
};
#endif

/* AT����MTAģ����Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToMtaMsgListTbl[] =
{
    {ID_AT_MTA_CPOS_SET_REQ,                                AT_PrivacyMatchCposSetReq},
    {ID_AT_MTA_MEID_SET_REQ,                                AT_PrivacyMatchMeidSetReq},
    {ID_AT_MTA_PSEUCELL_INFO_SET_REQ,                       AT_PrivacyMatchPseucellInfoSetReq},
};

/* AT����DRV_AGENTģ����Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToDrvAgentMsgListTbl[] =
{
    {DRV_AGENT_SIMLOCKWRITEEX_SET_REQ,                      AT_PrivacyMatchSimLockWriteExSetReq},
};

/* AT����NDISģ����Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToNdisMsgListTbl[] =
{
    {ID_AT_NDIS_PDNINFO_CFG_REQ,                            AT_PrivacyMatchNdisPdnInfoCfgReq},
    {ID_AT_NDIS_IFACE_UP_CONFIG_IND,                        AT_PrivacyMatchNdisIfaceUpConfigReq},
};

/* AT����IMSAģ����Ϣ�������������� */
#if (FEATURE_ON == FEATURE_IMS)
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToImsaMsgListTbl[] =
{
    {ID_AT_IMSA_IMS_CTRL_MSG,                               AT_PrivacyMatchImsaImsCtrlMsg},
    {ID_AT_IMSA_NICKNAME_SET_REQ,                           AT_PrivacyMatchImsaNickNameSetReq},
};
#endif

/**********************************************************************************************/
/***************************** WUEPS_PID_TAF������Ϣ������������� *****************************/
/* TAF(WUEPS_PID_TAF)����AT��Ϣ��������������, A��C��ʹ��ͬһ�ݴ��룬 */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToAtMsgListTbl[] =
{
    /* GUC A��C�˶��е��ã�������㴦�� */
    {MN_CALLBACK_CS_CALL,                                   TAF_PrivacyMatchAppMnCallBackCsCall},
    {MN_CALLBACK_SS,                                        TAF_PrivacyMatchAppMnCallBackSs},

    { MN_CALLBACK_QRY,                                      TAF_PrivacyMatchAtCallBackQryProc},

    { MN_CALLBACK_MSG,                                      TAF_PrivacyMatchAtCallBackSmsProc},

};

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/* AT���͸�XPDSģ����Ϣ������������ */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astAtAcorePrivacyMatchToXpdsMsgListTbl[] =
{
    {ID_AT_XPDS_GPS_POS_INFO_RSP,                           AT_PrivacyMatchCagpsPosInfoRsp},
    {ID_AT_XPDS_GPS_PRM_INFO_RSP,                           AT_PrivacyMatchCagpsPrmInfoRsp},
    {ID_AT_XPDS_AP_FORWARD_DATA_IND,                        AT_PrivacyMatchCagpsApForwardDataInd},

};
#endif
#endif

/**********************************************************************************************/
/***************************** UEPS_PID_XSMS������Ϣ������������� ****************************/
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/* XSMS����AT��Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafXsmsAcorePrivacyMatchToAtMsgListTbl[] =
{
    {TAF_XSMS_APP_MSG_TYPE_RCV_IND,                         TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd},
    {TAF_XSMS_APP_MSG_TYPE_WRITE_CNF,                       TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf},
};

/**********************************************************************************************/
/***************************** UEPS_PID_XPDS������Ϣ������������� *****************************/
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
/* XPDS����ATģ����Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafXpdsAcorePrivacyMatchToAtMsgListTbl[] =
{
    {ID_XPDS_AT_GPS_REFLOC_INFO_CNF,                        TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf},
    {ID_XPDS_AT_GPS_ION_INFO_IND,                           TAF_XPDS_PrivacyMatchAtGpsIonInfoInd},
    {ID_XPDS_AT_GPS_EPH_INFO_IND,                           TAF_XPDS_PrivacyMatchAtGpsEphInfoInd},
    {ID_XPDS_AT_GPS_ALM_INFO_IND,                           TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd},
    {ID_XPDS_AT_GPS_PDE_POSI_INFO_IND,                      TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd},
    {ID_XPDS_AT_GPS_ACQ_ASSIST_DATA_IND,                    TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd},
    {ID_XPDS_AT_AP_REVERSE_DATA_IND,                        TAF_XPDS_PrivacyMatchAtApReverseDataInd},
    {ID_XPDS_AT_UTS_GPS_POS_INFO_IND,                       TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd},
};

#endif
#endif

/* TAF����TAF��Ϣ�������������� */
/* (����hi6932��xģ�����¸����鶨���СΪ0������pclint�澯��gu��Ӵ����ɾ����cdma��) */

/* TAF����MMA��Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToMmaMsgListTbl[] =
{
    {ID_TAF_MMA_EFLOCIINFO_SET_REQ,                         TAF_MMA_PrivacyMatchAtEfClocInfoSetReq},
    {ID_TAF_MMA_EFPSLOCIINFO_SET_REQ,                       TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq},
};

TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToCcmMsgListTbl[] =
{
    {ID_TAF_CCM_CALL_ORIG_REQ,                              TAF_CCM_PrivacyMatchCallOrigReq},
    {ID_TAF_CCM_CALL_SUPS_CMD_REQ,                          TAF_CCM_PrivacyMatchCallSupsCmdReq},
    {ID_TAF_CCM_CUSTOM_DIAL_REQ,                            TAF_CCM_PrivacyMatchCustomDialReqCmdReq},
#if (FEATURE_ON == FEATURE_IMS)
    {ID_TAF_CCM_ECONF_DIAL_REQ,                             TAF_CCM_PrivacyMatchCcmEconfDialReq},
#endif
    {ID_TAF_CCM_START_DTMF_REQ,                             TAF_CCM_PrivacyMatchCcmStartDtmfReq},
    {ID_TAF_CCM_STOP_DTMF_REQ,                              TAF_CCM_PrivacyMatchCcmStopDtmfReq},
    {ID_TAF_CCM_SET_UUSINFO_REQ,                            TAF_CCM_PrivacyMatchCcmSetUusInfoReq},
    {ID_TAF_CCM_CUSTOM_ECC_NUM_REQ,                         TAF_CCM_PrivacyMatchCcmCustomEccNumReq},

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {ID_TAF_CCM_SEND_FLASH_REQ,                             TAF_CCM_PrivacyMatchCcmSendFlashReq},
    {ID_TAF_CCM_SEND_BURST_DTMF_REQ,                        TAF_CCM_PrivacyMatchCcmSendBrustDtmfReq},
    {ID_TAF_CCM_SEND_CONT_DTMF_REQ,                         TAF_CCM_PrivacyMatchCcmSendContDtmfReq},
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT)
    {ID_TAF_CCM_ENCRYPT_VOICE_REQ,                          TAF_CCM_PrivacyMatchCcmEncryptVoiceReq},
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE)
    {ID_TAF_CCM_SET_EC_KMC_REQ,                             TAF_CCM_PrivacyMatchCcmSetEcKmcReq},
#endif
#endif
#endif
};

/* RNIC����CDS��Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astRnicAcorePrivacyMatchToCdsMsgListTbl[] =
{
    {ID_RNIC_CDS_IMS_DATA_REQ,                              RNIC_PrivacyMatchCdsImsDataReq},
};

/* TAF����AT��PS�¼���Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafDsmAcorePrivacyMatchPsEvtMsgListTbl[] =
{
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_CNF,                   TAF_DSM_PrivacyMatchPsCallPdpActCnf},
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_IND,                   TAF_DSM_PrivacyMatchPsCallPdpActInd},
    {ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND,                     TAF_DSM_PrivacyMatchPsCallPdpManageInd},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_CNF,                     TAF_DSM_PrivacyMatchPsCallPdpModCnf},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_IND,                     TAF_DSM_PrivacyMatchPsCallPdpModInd},
    {ID_EVT_TAF_PS_CALL_PDP_IPV6_INFO_IND,                  TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd},
    {ID_EVT_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF,           TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf},
    {ID_EVT_TAF_PS_GET_TFT_INFO_CNF,                        TAF_DSM_PrivacyMatchPsGetTftInfoCnf},
    {ID_EVT_TAF_PS_GET_PDP_IP_ADDR_INFO_CNF,                TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf},
    {ID_EVT_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF,   TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf},
    {ID_EVT_TAF_PS_GET_DYNAMIC_TFT_INFO_CNF,                TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf},
    {ID_EVT_TAF_PS_GET_AUTHDATA_INFO_CNF,                   TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf},
    {ID_EVT_TAF_PS_REPORT_PCO_INFO_IND,                     TAF_DSM_PrivacyMatchPsReportPcoInfoInd},
    {ID_EVT_TAF_PS_GET_PDP_DNS_INFO_CNF,                    TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf},
    {ID_EVT_TAF_PS_GET_NEGOTIATION_DNS_CNF,                 TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf},
    {ID_EVT_TAF_PS_EPDG_CTRLU_NTF,                          TAF_DSM_PrivacyMatchPsEpdgCtrluNtf},
};

/* TAF����AT��IFACE�¼���Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafDsmAcorePrivacyMatchIfaceEvtMsgListTbl[] =
{
    {ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF,                 TAF_DSM_PrivacyMatchTafIfaceGetDynamicParaCnf},
    {ID_EVT_TAF_IFACE_RAB_INFO_IND,                         TAF_DSM_PrivacyMatchTafIfaceRabInfoInd},
    {ID_DSM_NDIS_IFACE_UP_IND,                              TAF_DSM_PrivacyMatchNdisIfaceUpInd},
};


/* MTA����AT��Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                      g_astTafMtaAcorePrivacyMatchToAtMsgListTbl[] =
{
    {ID_MTA_AT_CPOSR_IND,                                   TAF_MTA_PrivacyMatchCposrInd},
    {ID_MTA_AT_MEID_QRY_CNF,                                TAF_MTA_PrivacyMatchAtMeidQryCnf},
    {ID_MTA_AT_CGSN_QRY_CNF,                                TAF_MTA_PrivacyMatchAtCgsnQryCnf},

    {ID_MTA_AT_ECID_SET_CNF,                                TAF_MTA_PrivacyMatchAtEcidSetCnf},

    {ID_MTA_AT_SET_NETMON_SCELL_CNF,                        TAF_MTA_PrivacyMatchAtSetNetMonScellCnf},
    {ID_MTA_AT_SET_NETMON_NCELL_CNF,                        TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf},
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
    {ID_MTA_AT_SET_NETMON_SSCELL_CNF,                       TAF_MTA_PrivacyMatchAtSetNetMonSScellCnf},
#endif
};

/* MMA����AT��Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafMmaAcorePrivacyMatchToAtMsgListTbl[] =
{
    {ID_TAF_MMA_USIM_STATUS_IND,                            TAF_MMA_PrivacyMatchAtUsimStatusInd},
    {ID_TAF_MMA_HOME_PLMN_QRY_CNF,                          TAF_MMA_PrivacyMatchAtHomePlmnQryCnf},
    {ID_TAF_MMA_LOCATION_INFO_QRY_CNF,                      TAF_MMA_PrivacyMatchAtLocationInfoQryCnf},
    {ID_TAF_MMA_REG_STATUS_IND,                             TAF_MMA_PrivacyMatchAtRegStatusInd},
    {ID_TAF_MMA_SRCHED_PLMN_INFO_IND,                       TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd},
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {ID_TAF_MMA_CDMA_LOCINFO_QRY_CNF,                       TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf},
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */
    {ID_TAF_MMA_NET_SCAN_CNF,                               TAF_MMA_PrivacyMatchAtNetScanCnf},
    {ID_TAF_MMA_REG_STATE_QRY_CNF,                          TAF_MMA_PrivacyMatchAtRegStateQryCnf},
    {ID_TAF_MMA_CLOCINFO_IND,                               TAF_MMA_PrivacyMatchAtClocInfoInd},
    {ID_TAF_MMA_REJINFO_QRY_CNF,                            TAF_MMA_PrivacyMatchAtRejInfoQryCnf},
    {ID_TAF_MMA_EFLOCIINFO_QRY_CNF,                         TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf},
    {ID_TAF_MMA_EFPSLOCIINFO_QRY_CNF,                       TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf},
};


/* TAF����DSM��Ϣ�������������� */
TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafAcorePrivacyMatchToDsmMsgListTbl[] =
{
    {ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ,                   TAF_DSM_PrivacyMatchTafSetAuthDataReq},
    {ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ,                    TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq},
    {ID_MSG_TAF_PS_CALL_ORIG_REQ,                           TAF_PrivacyMatchDsmPsCallOrigReq},
    {ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ,                       TAF_PrivacyMatchDsmPsPppDialOrigReq},
    {ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ,           TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq},
    {ID_MSG_TAF_PS_SET_TFT_INFO_REQ,                        TAF_PrivacyMatchDsmPsSetTftInfoReq},
    {ID_MSG_TAF_PS_EPDG_CTRL_NTF,                           TAF_PrivacyMatchDsmPsEpdgCtrlNtf},
    {ID_MSG_TAF_IFACE_UP_REQ,                               TAF_PrivacyMatchDsmIfaceUpReq},
};

TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafDrvAgentAcorePrivacyMatchToAtMsgListTbl[] =
{
    {DRV_AGENT_MSID_QRY_CNF,                                TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf},
};

TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafDsmAcorePrivacyMatchToRnicMsgListTbl[] =
{
    {ID_DSM_RNIC_IFACE_CFG_IND,                             TAF_DSM_PrivacyMatchRnicIfaceCfgInd},
};

TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                          g_astTafCcmAcorePrivacyMatchToAtMsgListTbl[] =
{
#if (FEATURE_ON == FEATURE_IMS)
    {ID_TAF_CCM_QRY_ECONF_CALLED_INFO_CNF,                  TAF_PrivacyMatchAppQryEconfCalledInfoCnf},
    {ID_TAF_CCM_ECONF_NOTIFY_IND,                           TAF_PrivacyMatchAtEconfNotifyInd},
#endif

    {ID_TAF_CCM_CALL_INCOMING_IND,                          TAF_PrivacyMatchAtCallIncomingInd},
    {ID_TAF_CCM_CALL_CONNECT_IND,                           TAF_PrivacyMatchAtCallConnectInd},
    {ID_TAF_CCM_QRY_CALL_INFO_CNF,                          TAF_PrivacyMatchAtQryCallInfoInd},
    {ID_TAF_CCM_CALL_SS_IND,                                TAF_PrivacyMatchAtCallSsInd},
    {ID_TAF_CCM_ECC_NUM_IND,                                TAF_PrivacyMatchAtEccNumInd},
    {ID_TAF_CCM_QRY_XLEMA_CNF,                              TAF_PrivacyMatchAtQryXlemaInd},
    {ID_TAF_CCM_CNAP_QRY_CNF,                               TAF_PrivacyMatchAtCnapQryCnf},
    {ID_TAF_CCM_CNAP_INFO_IND,                              TAF_PrivacyMatchAtCnapInfoInd},
    {ID_TAF_CCM_QRY_UUS1_INFO_CNF,                          TAF_PrivacyMatchAtQryUus1InfoCnf},
    {ID_TAF_CCM_UUS1_INFO_IND,                              TAF_PrivacyMatchAtUus1InfoInd},
    {ID_TAF_CCM_QRY_CLPR_CNF,                               TAF_PrivacyMatchAtQryClprCnf},

#if (FEATURE_ON == FEATURE_ECALL)
    {ID_TAF_CCM_QRY_ECALL_INFO_CNF,                         TAF_PrivacyMatchAtQryEcallInfoCnf},
#endif

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {ID_TAF_CCM_CALLED_NUM_INFO_IND,                        TAF_PrivacyMatchAtCalledNumInfoInd},
    {ID_TAF_CCM_CALLING_NUM_INFO_IND,                       TAF_PrivacyMatchAtCallingNumInfoInd},
    {ID_TAF_CCM_DISPLAY_INFO_IND,                           TAF_PrivacyMatchAtDisplayInfoInd},
    {ID_TAF_CCM_EXT_DISPLAY_INFO_IND,                       TAF_PrivacyMatchAtExtDisplayInfoInd},
    {ID_TAF_CCM_CONN_NUM_INFO_IND,                          TAF_PrivacyMatchAtConnNumInfoInd},
    {ID_TAF_CCM_REDIR_NUM_INFO_IND,                         TAF_PrivacyMatchAtRedirNumInfoInd},
    {ID_TAF_CCM_CCWAC_INFO_IND,                             TAF_PrivacyMatchAtCcwacInfoInfoInd},
    {ID_TAF_CCM_CONT_DTMF_IND,                              TAF_PrivacyMatchAtContDtmfInd},
    {ID_TAF_CCM_BURST_DTMF_IND,                             TAF_PrivacyMatchAtBurstDtmfInd},
    {ID_TAF_CCM_ENCRYPT_VOICE_IND,                          TAF_PrivacyMatchAtEncryptVoiceInd},
    {ID_TAF_CCM_GET_EC_RANDOM_CNF,                          TAF_PrivacyMatchAtGetEcRandomCnf},
    {ID_TAF_CCM_GET_EC_KMC_CNF,                             TAF_PrivacyMatchAtGetEcKmcCnf},
#endif
};

/**********************************************************************************************/
/*************************************** PIDӳ�䴦��� ****************************************/
/* WUEPS_PID_AT���͸���ͬpid����Ϣ��Ӧ����������� */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astAtAcorePrivacyMatchRcvPidListTbl[] =
{
    /* AT���͸�XPDS����Ϣ���� */
#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
    {UEPS_PID_XPDS,      sizeof(g_astAtAcorePrivacyMatchToXpdsMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),        g_astAtAcorePrivacyMatchToXpdsMsgListTbl},
#endif

    /* AT���͸�TAF����Ϣ���� */
    {WUEPS_PID_TAF,     sizeof(g_astAtAcorePrivacyMatchToTafMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToTafMsgListTbl},
    /* AT���͸�XSMS����Ϣ */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    {UEPS_PID_XSMS,     sizeof(g_astAtAcorePrivacyMatchToXsmsMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astAtAcorePrivacyMatchToXsmsMsgListTbl},
#endif
    /* AT���͸�MTA����Ϣ */
    {UEPS_PID_MTA,      sizeof(g_astAtAcorePrivacyMatchToMtaMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToMtaMsgListTbl},

    {PS_PID_APP_NDIS,    sizeof(g_astAtAcorePrivacyMatchToNdisMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),        g_astAtAcorePrivacyMatchToNdisMsgListTbl},

    {WUEPS_PID_DRV_AGENT,      sizeof(g_astAtAcorePrivacyMatchToDrvAgentMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToDrvAgentMsgListTbl},

#if (FEATURE_ON == FEATURE_IMS)
    {PS_PID_IMSA,      sizeof(g_astAtAcorePrivacyMatchToImsaMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astAtAcorePrivacyMatchToImsaMsgListTbl},
#endif

};

/* TAF(WUEPS_PID_TAF)���͸���ͬpid����Ϣ��Ӧ����������� */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astTafAcorePrivacyMatchRcvPidListTbl[] =
{
    /* GUC A��C�˶��е��ã�������㴦�� */
    {WUEPS_PID_AT,      sizeof(g_astTafAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astTafAcorePrivacyMatchToAtMsgListTbl},

    {WUEPS_PID_MMA,     sizeof(g_astTafAcorePrivacyMatchToMmaMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astTafAcorePrivacyMatchToMmaMsgListTbl},

    {UEPS_PID_DSM,      sizeof(g_astTafAcorePrivacyMatchToDsmMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astTafAcorePrivacyMatchToDsmMsgListTbl},
    {UEPS_PID_CCM,     sizeof(g_astTafAcorePrivacyMatchToCcmMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),          g_astTafAcorePrivacyMatchToCcmMsgListTbl},
};

/* UEPS_PID_XSMS���͸���ͬpid����Ϣ��Ӧ����������� */
#if(FEATURE_ON == FEATURE_UE_MODE_CDMA)
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astXsmsAcorePrivacyMatchRcvPidListTbl[] =
{
    /* XSMS���͸�AT����Ϣ���� */
    {WUEPS_PID_AT,      sizeof(g_astTafXsmsAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),      g_astTafXsmsAcorePrivacyMatchToAtMsgListTbl},
};

/* UEPS_PID_XPDS���͸���ͬpid����Ϣ��Ӧ����������� */
#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astXpdsAcorePrivacyMatchRcvPidListTbl[] =
{
    /* XPDS���͸�AT����Ϣ���� */
    {WUEPS_PID_AT,      sizeof(g_astTafXpdsAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),      g_astTafXpdsAcorePrivacyMatchToAtMsgListTbl},
};
#endif
#endif

/* MTA���͸���ͬPID����Ϣ��Ӧ����������� */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                  g_astTafMtaAcorePrivacyMatchRcvPidListTbl[] =
{
    /* MTA����AT����Ϣ */
    {WUEPS_PID_AT,     sizeof(g_astTafMtaAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),  g_astTafMtaAcorePrivacyMatchToAtMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                  g_astTafMmaAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,     sizeof(g_astTafMmaAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),  g_astTafMmaAcorePrivacyMatchToAtMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                  g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,     sizeof(g_astTafDrvAgentAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),  g_astTafDrvAgentAcorePrivacyMatchToAtMsgListTbl},
};

/* RNIC���͸���ͬpid����Ϣ��Ӧ����������� */
TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astRnicAcorePrivacyMatchRcvPidListTbl[] =
{
    {UEPS_PID_CDS,     sizeof(g_astRnicAcorePrivacyMatchToCdsMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),         g_astRnicAcorePrivacyMatchToCdsMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astTafDsmAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,       sizeof(g_astTafDsmAcorePrivacyMatchPsEvtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),     g_astTafDsmAcorePrivacyMatchPsEvtMsgListTbl},
    {WUEPS_PID_AT,       sizeof(g_astTafDsmAcorePrivacyMatchIfaceEvtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),     g_astTafDsmAcorePrivacyMatchIfaceEvtMsgListTbl},
    {ACPU_PID_RNIC,      sizeof(g_astTafDsmAcorePrivacyMatchToRnicMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),    g_astTafDsmAcorePrivacyMatchToRnicMsgListTbl},
};

TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU                      g_astTafCcmAcorePrivacyMatchRcvPidListTbl[] =
{
    {WUEPS_PID_AT,       sizeof(g_astTafCcmAcorePrivacyMatchToAtMsgListTbl)/sizeof(TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU),     g_astTafCcmAcorePrivacyMatchToAtMsgListTbl},
};


LOCAL VOS_UINT32 TAF_LogPrivacyGetModem0Pid(
    VOS_UINT32                          ulSrcPid
)
{
    VOS_UINT32                          ulLoop;

    for (ulLoop = 0; ulLoop < sizeof(g_astTafPrivacyMatchModemPidMapTbl)/sizeof(TAF_LOG_PRIVACY_MATCH_MODEM_PID_MAP_TBL_STRU); ulLoop++)
    {
        if ( (ulSrcPid == g_astTafPrivacyMatchModemPidMapTbl[ulLoop].ulModem1Pid)
          || (ulSrcPid == g_astTafPrivacyMatchModemPidMapTbl[ulLoop].ulModem2Pid) )
        {
            return g_astTafPrivacyMatchModemPidMapTbl[ulLoop].ulModem0Pid;
        }
    }

    return ulSrcPid;
}


VOS_UINT32 AT_PrivacyFilterMode(
    AT_MSG_STRU                        *pstAtMsg
)
{
    if (pstAtMsg->ucIndex < AT_MAX_CLIENT_NUM)
    {
        if (AT_XML_MODE == g_stParseContext[pstAtMsg->ucIndex].ucMode)
        {
            /* xmlģʽֱ�ӽ��й��� */
            MN_NORM_LOG1("AT_PrivacyFilterTypeAndMode: TRUE,XML MODE, ulMsgName ", pstAtMsg->enMsgId);
            return VOS_FALSE;
        }

        if (AT_SMS_MODE == g_stParseContext[pstAtMsg->ucIndex].ucMode)
        {
           /* ����ģʽֱ�ӽ��й��� */
           MN_NORM_LOG1("AT_PrivacyFilterTypeAndMode: TRUE,SMS MODE ulMsgName ", pstAtMsg->enMsgId);
           return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}


VOS_VOID* AT_PrivacyFilterType(
    AT_MSG_STRU                        *pstAtMsg
)
{
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;

    for (ulLoop = 0; ulLoop < (sizeof(g_astPrivacyMapCmdToFuncTbl) / sizeof(AT_LOG_PRIVACY_MAP_CMD_TO_FUNC_STRU)); ulLoop++)
    {
        if (pstAtMsg->ucFilterAtType == g_astPrivacyMapCmdToFuncTbl[ulLoop].ulAtCmdType)
        {
            pstPrivacyMatchAtMsg = g_astPrivacyMapCmdToFuncTbl[ulLoop].pcPrivacyAtCmd(pstAtMsg);

            if (VOS_NULL_PTR == pstPrivacyMatchAtMsg)
            {
                break;
            }
            else
            {
                return (VOS_VOID*)pstPrivacyMatchAtMsg;
            }
        }
    }
    return VOS_NULL_PTR;
}


VOS_VOID* AT_PrivacyFilterMatchCmdTbl(
    AT_MSG_STRU                        *pstAtMsg,
    VOS_CHAR                           *pcPrivacyAtCmd
)
{
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    errno_t                             lMemResult;
    VOS_UINT16                          usPrivacyAtMsgLen;
    VOS_UINT16                          usAtMsgHeaderLen;
    VOS_UINT16                          usPrivacyAtCmdLen;

    usPrivacyAtCmdLen       = (VOS_UINT16)(VOS_StrLen(pcPrivacyAtCmd));

    /* ��Ϣ�ṹ�峤�� + at�����ַ������� - ��Ϣ�ṹ����aucValue���鳤�� */
    usPrivacyAtMsgLen       = sizeof(AT_MSG_STRU) + usPrivacyAtCmdLen - 4;
    /* A�˲����ö�ʵ���ӿ������ڴ� */
    pstPrivacyMatchAtMsg    = (AT_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                          DYNAMIC_MEM_PT,
                                                          usPrivacyAtMsgLen);

    if (VOS_NULL_PTR == pstPrivacyMatchAtMsg)
    {
        return VOS_NULL_PTR;
    }

    usAtMsgHeaderLen  = sizeof(AT_MSG_STRU) - 4;

    /* ����ԭʼ��Ϣͷ�� */
    lMemResult = memcpy_s(pstPrivacyMatchAtMsg,
                          usPrivacyAtMsgLen,
                          pstAtMsg,
                          usAtMsgHeaderLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usPrivacyAtMsgLen, usAtMsgHeaderLen);

    /* �����µ�at cmd�ַ������� */
    pstPrivacyMatchAtMsg->ulLength = usPrivacyAtMsgLen - VOS_MSG_HEAD_LENGTH;
    pstPrivacyMatchAtMsg->usLen    = usPrivacyAtCmdLen;

    /* ����������at�����ַ��� */
    lMemResult = memcpy_s(pstPrivacyMatchAtMsg->aucValue,
                          usPrivacyAtCmdLen,
                          pcPrivacyAtCmd,
                          usPrivacyAtCmdLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usPrivacyAtCmdLen, usPrivacyAtCmdLen);

    return (VOS_VOID *)pstPrivacyMatchAtMsg;

}


LOCAL VOS_VOID* AT_PrivacyMatchNdisPdnInfoCfgReq(
    MsgBlock                                               *pstMsg
)
{
    AT_NDIS_PDNINFO_CFG_REQ_STRU       *pstPdnInfoCfgReq = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* ������Ϣ,�����ڴ����ͳһ�ɵײ��ͷ� */
    pstPdnInfoCfgReq = (AT_NDIS_PDNINFO_CFG_REQ_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                    DYNAMIC_MEM_PT,
                                                                    ulLength);

    if (VOS_NULL_PTR == pstPdnInfoCfgReq)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstPdnInfoCfgReq,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* ��������Ϣ����Ϊȫ0 */
    memset_s(&(pstPdnInfoCfgReq->stIpv4PdnInfo),
             sizeof(pstPdnInfoCfgReq->stIpv4PdnInfo),
             0,
             sizeof(AT_NDIS_IPV4_PDN_INFO_STRU));

    memset_s(&(pstPdnInfoCfgReq->stIpv6PdnInfo),
             sizeof(pstPdnInfoCfgReq->stIpv6PdnInfo),
             0,
             sizeof(AT_NDIS_IPV6_PDN_INFO_STRU));

    return (VOS_VOID *)pstPdnInfoCfgReq;
}


LOCAL VOS_VOID* AT_PrivacyMatchNdisIfaceUpConfigReq(
    MsgBlock                                               *pstMsg
)
{
    AT_NDIS_IFACE_UP_CONFIG_IND_STRU   *pstIfaceUpConfigInd = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;

    ulLength  = pstMsg->ulLength + VOS_MSG_HEAD_LENGTH;

    /* ������Ϣ,�����ڴ����ͳһ�ɵײ��ͷ� */
    pstIfaceUpConfigInd = (AT_NDIS_IFACE_UP_CONFIG_IND_STRU *)VOS_MemAlloc(pstMsg->ulSenderPid,
                                                                           DYNAMIC_MEM_PT,
                                                                           ulLength);

    if (VOS_NULL_PTR == pstIfaceUpConfigInd)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pstIfaceUpConfigInd,
                          ulLength,
                          pstMsg,
                          ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* ��������Ϣ����Ϊȫ0 */
    memset_s(&(pstIfaceUpConfigInd->stIpv4PdnInfo),
             sizeof(pstIfaceUpConfigInd->stIpv4PdnInfo),
             0,
             sizeof(AT_NDIS_IPV4_PDN_INFO_STRU));

    memset_s(&(pstIfaceUpConfigInd->stIpv6PdnInfo),
             sizeof(pstIfaceUpConfigInd->stIpv6PdnInfo),
             0,
             sizeof(AT_NDIS_IPV6_PDN_INFO_STRU));

    return (VOS_VOID *)pstIfaceUpConfigInd;
}


LOCAL VOS_UINT32 AT_PrivacyMatchIsWhiteListAtCmd(
    AT_MSG_STRU                        *pstAtMsg
)
{
    VOS_UINT32                          ulLoop;

    for (ulLoop = 0; ulLoop < sizeof(g_aenAtCmdWhiteListTbl)/sizeof(AT_INTER_MSG_ID_ENUM_UINT32); ulLoop++)
    {
        if (g_aenAtCmdWhiteListTbl[ulLoop] == pstAtMsg->enMsgId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_VOID* AT_PrivacyMatchAtCmd(
    MsgBlock                           *pstMsg
)
{
    VOS_UINT8                          *pucAtCmdData            = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstAtMsg                = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    errno_t                             lMemResult;
    VOS_UINT16                          usTempAtCmdLen;
    VOS_UINT16                          usSetAtTmpLen;

    pstAtMsg        = (AT_MSG_STRU *)pstMsg;

    /* �������е���Ϣ������, ֱ�ӷ���ԭ��Ϣ, ���ڰ������е���Ϣ�ٽ���������� */
    if (VOS_TRUE == AT_PrivacyMatchIsWhiteListAtCmd(pstAtMsg))
    {
        return (VOS_VOID *)pstMsg;
    }
    else
    {
        if(VOS_FALSE == AT_PrivacyFilterMode(pstAtMsg))
        {
            return VOS_NULL_PTR;
        }

        pstPrivacyMatchAtMsg = (AT_MSG_STRU*)AT_PrivacyFilterType(pstAtMsg);

        if(VOS_NULL_PTR != pstPrivacyMatchAtMsg)
        {
            return (VOS_VOID*)pstPrivacyMatchAtMsg;
        }

        /* �����㹻����ڴ�, ��ʱ���AT Cmd, �����ж��Ƿ���Ҫ����, ʹ�ú��ͷ� */
        usTempAtCmdLen  = (pstAtMsg->usLen > LOG_PRIVACY_AT_CMD_MAX_LEN) ? pstAtMsg->usLen : LOG_PRIVACY_AT_CMD_MAX_LEN;
        pucAtCmdData    = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, usTempAtCmdLen);

        if (VOS_NULL_PTR == pucAtCmdData)
        {
            return VOS_NULL_PTR;
        }

        memset_s(pucAtCmdData, usTempAtCmdLen, 0x00, usTempAtCmdLen);
        lMemResult = memcpy_s(pucAtCmdData, usTempAtCmdLen, pstAtMsg->aucValue, pstAtMsg->usLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, usTempAtCmdLen, pstAtMsg->usLen);

        (VOS_VOID)At_UpString(pucAtCmdData, pstAtMsg->usLen);

        /* �ϱ���ظ�����Ĵ��� */
        for (ulLoop = 0; ulLoop < (sizeof(g_astPrivacyMatchRptAtCmdMapTbl)/sizeof(AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU)); ulLoop++)
        {
            if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)g_astPrivacyMatchRptAtCmdMapTbl[ulLoop].pcOriginalAtCmd, pucAtCmdData, VOS_StrLen(g_astPrivacyMatchRptAtCmdMapTbl[ulLoop].pcOriginalAtCmd)))
            {
                pstPrivacyMatchAtMsg = AT_PrivacyFilterMatchCmdTbl(pstAtMsg, g_astPrivacyMatchRptAtCmdMapTbl[ulLoop].pcPrivacyAtCmd);

                PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

                return (VOS_VOID *)pstPrivacyMatchAtMsg;
            }
        }

        usSetAtTmpLen = pstAtMsg->usLen;
        AT_DiscardInvalidChar(pucAtCmdData, &usSetAtTmpLen);

        /* �·�����Ĵ��� */
        for (ulLoop = 0; ulLoop < (sizeof(g_astPrivacyMatchAtCmdMapTbl)/sizeof(AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU)); ulLoop++)
        {
            if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcOriginalAtCmd, pucAtCmdData, VOS_StrLen(g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcOriginalAtCmd)))
            {
                pstPrivacyMatchAtMsg = AT_PrivacyFilterMatchCmdTbl(pstAtMsg, g_astPrivacyMatchAtCmdMapTbl[ulLoop].pcPrivacyAtCmd);

                PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

                return (VOS_VOID *)pstPrivacyMatchAtMsg;
            }
        }

        /* δƥ��, ������������Ϣ, ����ԭʼ��Ϣ */
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

        return (VOS_VOID *)pstMsg;
    }
}

LOCAL AT_MSG_STRU* AT_PrivacyFilterCnfCommProc(
    AT_MSG_STRU                        *pstMsg,
    VOS_UINT32                          ulStartIndex,
    VOS_CHAR                            ucEndChar
)
{
    AT_MSG_STRU                        *pstAtMsg                = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT8                          *pucPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT8                          *pucValue                = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;
    errno_t                             lMemResult;
    VOS_UINT16                          usPrivacyAtMsgLen;

    pstAtMsg        = (AT_MSG_STRU *)pstMsg;

    ulIndex         = 0;

    /* �ж��ǲ�ѯ�����ǻظ����,����ǲ�ѯ��������Ҫ���� */
    if ('\r' != pstAtMsg->aucValue[0])
    {
        return VOS_NULL_PTR;
    }

    usPrivacyAtMsgLen    = sizeof(AT_MSG_STRU) + pstAtMsg->usLen - 4;

    /* A�˲����ö�ʵ���ӿ������ڴ� */
    pucPrivacyMatchAtMsg = (VOS_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT,
                                         DYNAMIC_MEM_PT,
                                         usPrivacyAtMsgLen);

    if (VOS_NULL_PTR == pucPrivacyMatchAtMsg)
    {
        return VOS_NULL_PTR;
    }

    pstPrivacyMatchAtMsg = (AT_MSG_STRU *)pucPrivacyMatchAtMsg;

    /* ����ԭʼ��Ϣ */
    lMemResult = memcpy_s(pstPrivacyMatchAtMsg,
                          usPrivacyAtMsgLen,
                          pstAtMsg,
                          usPrivacyAtMsgLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usPrivacyAtMsgLen, usPrivacyAtMsgLen);

    pucValue = pucPrivacyMatchAtMsg + sizeof(AT_MSG_STRU) - 4;

    if (AT_CMD_MAX_LEN < pstPrivacyMatchAtMsg->usLen)
    {
        pstPrivacyMatchAtMsg->usLen = AT_CMD_MAX_LEN;
    }

    for (ulIndex = ulStartIndex; ulIndex < pstPrivacyMatchAtMsg->usLen; ulIndex++)
    {
        if (ucEndChar == pucValue[ulIndex])
        {
            break;
        }

        pucValue[ulIndex] = '*';
    }

    return pstPrivacyMatchAtMsg;
}


LOCAL AT_MSG_STRU* AT_PrivacyFilterCimi(
    AT_MSG_STRU                        *pstMsg
)
{
    /****************************************
    �ӵ�7λ��ʼ�滻Ϊ*,��0λ����6λ����Ҫ�滻
    0:\r     1:\n     2-4:MCC      5-6:MNC
    ****************************************/
    return AT_PrivacyFilterCnfCommProc(pstMsg, 7, '\r');
}

LOCAL AT_MSG_STRU* AT_PrivacyFilterCgsn(
    AT_MSG_STRU                        *pstMsg
)
{
    /****************************************
    �ӵ�2λ��ʼ�滻Ϊ*,��0λ�͵�1λ����Ҫ�滻
    0:\r     1:\n
    ****************************************/
    return AT_PrivacyFilterCnfCommProc(pstMsg, 2, '\r');
}

LOCAL AT_MSG_STRU* AT_PrivacyFilterMsid(
    AT_MSG_STRU                        *pstMsg
)
{
    VOS_UINT8                          *pucAtCmdData            = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstAtMsg                = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstPrivacyMatchAtMsg    = VOS_NULL_PTR;
    VOS_UINT16                          usPrivacyAtMsgLen;
    VOS_CHAR                           *pcAtCmd         = "ATI";
    VOS_CHAR                           *pcFilterField   = "IMEI: ";
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulIndex2;
    errno_t                             lMemResult;
    VOS_UINT16                          usFilterFieldLen;

    pstAtMsg        = (AT_MSG_STRU *)pstMsg;

    /* �����ڴ�, ��ʱ���AT Cmd, �����ж��Ƿ���Ҫ����, ʹ�ú��ͷ� */
    pucAtCmdData    = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, pstAtMsg->usLen);

    if (VOS_NULL_PTR == pucAtCmdData)
    {
        return VOS_NULL_PTR;
    }

    lMemResult = memcpy_s(pucAtCmdData, pstAtMsg->usLen, pstAtMsg->aucValue, pstAtMsg->usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, pstAtMsg->usLen, pstAtMsg->usLen);

    (VOS_VOID)At_UpString(pucAtCmdData, pstAtMsg->usLen);

    /* �ж���ATI�Ĳ�ѯ�����ǻظ����,����ǲ�ѯ��������Ҫ���� */
    if ( VOS_OK == PS_MEM_CMP((VOS_UINT8 *)pcAtCmd, pucAtCmdData, VOS_StrLen(pcAtCmd)))
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

        return VOS_NULL_PTR;
    }

    usPrivacyAtMsgLen    = sizeof(AT_MSG_STRU) + pstAtMsg->usLen - 4;

    /* A�˲����ö�ʵ���ӿ������ڴ� */
    pstPrivacyMatchAtMsg = (AT_MSG_STRU *)VOS_MemAlloc(WUEPS_PID_AT,
                                                       DYNAMIC_MEM_PT,
                                                       usPrivacyAtMsgLen);
    if (VOS_NULL_PTR == pstPrivacyMatchAtMsg)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

        return VOS_NULL_PTR;
    }

    /* ����ԭʼ��Ϣ */
    lMemResult = memcpy_s(pstPrivacyMatchAtMsg,
                          usPrivacyAtMsgLen,
                          pstAtMsg,
                          usPrivacyAtMsgLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usPrivacyAtMsgLen, usPrivacyAtMsgLen);

    /****************************************
    ֻ�滻IMEI�ֶκ����Ϣ
    ****************************************/
    usFilterFieldLen = (VOS_UINT16)VOS_StrLen(pcFilterField);

    if (usFilterFieldLen > pstPrivacyMatchAtMsg->usLen)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);
        /*lint -save -e516*/
        PS_MEM_FREE(WUEPS_PID_AT, pstPrivacyMatchAtMsg);
        /*lint -restore*/

        return VOS_NULL_PTR;
    }

    if (AT_CMD_MAX_LEN < pstPrivacyMatchAtMsg->usLen)
    {
        pstPrivacyMatchAtMsg->usLen = AT_CMD_MAX_LEN;
    }

    for (ulIndex = 0; ulIndex < (VOS_UINT32)(pstPrivacyMatchAtMsg->usLen - usFilterFieldLen); ulIndex++)
    {
        /* �ҳ�IMEI�ֶ� */
        if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)pcFilterField, &(pstPrivacyMatchAtMsg->aucValue[ulIndex]), usFilterFieldLen))
        {
            /* ��IMEI����ֵ�滻��* */
            for (ulIndex2 = (ulIndex + usFilterFieldLen); ulIndex2 < pstPrivacyMatchAtMsg->usLen; ulIndex2++)
            {
                if ('\r' == pstPrivacyMatchAtMsg->aucValue[ulIndex2])
                {
                    break;
                }

                pstPrivacyMatchAtMsg->aucValue[ulIndex2] = '*';
            }

            break;
        }
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucAtCmdData);

    return pstPrivacyMatchAtMsg;
}


LOCAL AT_MSG_STRU* AT_PrivacyFilterHplmn(
    AT_MSG_STRU                        *pstMsg
)
{
    /****************************************
    �ӵ�15λ��ʼ�滻Ϊ*,��0λ����14λ����Ҫ�滻
    0:\r     1:\n     2-9:^HPLMN:(�˴�����һ���ո�)
    10-12:MCC   13-14:MNC
    ****************************************/
    return AT_PrivacyFilterCnfCommProc(pstMsg, 15, ',');
}


VOS_VOID* AT_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstAtMsgPrivacyMatchMsg  = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;
    ulRcvPid        = pstRcvMsgHeader->ulReceiverPid;

    if (WUEPS_PID_AT == pstRcvMsgHeader->ulReceiverPid)
    {
        pstAtMsgPrivacyMatchMsg = AT_PrivacyMatchAtCmd((MsgBlock *)pstMsg);

        return (VOS_VOID *)pstAtMsgPrivacyMatchMsg;
    }

    /* A�˵�����, ��Ҫ��I1/I2��PIDת��ΪI0��PID */
    ulRcvPid        = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    ulTblSize       = 0;

    /* ���������Ϣ��Ӧ��rcvpid�� */
    for (ulLoop = 0; ulLoop < (sizeof(g_astAtAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulLoop++)
    {
        if (ulRcvPid == g_astAtAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astAtAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astAtAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    ulMsgName = (VOS_UINT16)(pstRcvMsgHeader->ulMsgName);

    /* ���������Ϣ��Ӧ���������� */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        for (ulLoop = 0; ulLoop < ulTblSize; ulLoop++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulLoop].ulMsgName)
            {
                pstAtMsgPrivacyMatchMsg = pstPrivacyMatchMsgTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstAtMsgPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl    = VOS_NULL_PTR;
    VOS_VOID                                               *pstTafLogPrivacyMatchMsg    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader             = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* A�˵�����, ��Ҫ��I1/I2��PIDת��ΪI0��PID */
    ulRcvPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* ����rcv pid����pidӳ��� */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (ulRcvPid == g_astTafAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* ������rcv pid�ҵ�pidӳ�����������ƥ�����Ҵ����� */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)(pstRcvMsgHeader->ulMsgName);

        /* ����msg name���ҹ��˺���ӳ��� */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstTafLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstTafLogPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID* TAF_XSMS_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstXsmsPrivacyMatchMsg      = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl       = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstMsgHeader                = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulIndex;
    VOS_UINT32                                              ulMsgName;

    pstMsgHeader    = (MSG_HEADER_STRU *)pstMsg;
    ulTblSize       = 0;

    /* ���������Ϣ��Ӧ��rcvpid�� */
    for (ulIndex = 0; ulIndex < (sizeof(g_astXsmsAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulIndex++)
    {
        if (pstMsgHeader->ulReceiverPid == g_astXsmsAcorePrivacyMatchRcvPidListTbl[ulIndex].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astXsmsAcorePrivacyMatchRcvPidListTbl[ulIndex].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astXsmsAcorePrivacyMatchRcvPidListTbl[ulIndex].ulTblSize;

            break;
        }
    }

    /* ���������Ϣ��Ӧ���������� */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        ulMsgName = (VOS_UINT16)(pstMsgHeader->ulMsgName);

        for (ulIndex = 0; ulIndex < ulTblSize; ulIndex++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulIndex].ulMsgName)
            {
                pstXsmsPrivacyMatchMsg = pstPrivacyMatchMsgTbl[ulIndex].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstXsmsPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}
#endif

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))

VOS_VOID* TAF_XPDS_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstXpdsPrivacyMatchMsg    = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl     = VOS_NULL_PTR;
    PS_MSG_HEADER_STRU                                     *pstMsgHeader              = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulMsgName;

    pstMsgHeader    = (PS_MSG_HEADER_STRU *)pstMsg;
    ulTblSize       = 0;

    /* ���������Ϣ��Ӧ��rcvpid�� */
    for (ulLoop = 0; ulLoop < (sizeof(g_astXpdsAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulLoop++)
    {
        if (pstMsgHeader->ulReceiverPid == g_astXpdsAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astXpdsAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astXpdsAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* ���������Ϣ��Ӧ���������� */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        ulMsgName = pstMsgHeader->ulMsgName;

        for (ulLoop = 0; ulLoop < ulTblSize; ulLoop++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulLoop].ulMsgName)
            {
                pstXpdsPrivacyMatchMsg = pstPrivacyMatchMsgTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstXpdsPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}
#endif


VOS_VOID* TAF_MTA_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                                               *pstMtaLogPrivacyMatchMsg  = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafMtaAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* ����ulReceiverPid����PIDӳ��� */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (pstRcvMsgHeader->ulReceiverPid == g_astTafMtaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafMtaAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafMtaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* ������ulReceiverPid�ҵ�PIDӳ�����������ƥ��� */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)pstRcvMsgHeader->ulMsgName;

        /* ����ulMsgName���ҹ��˺���ӳ��� */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstMtaLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstMtaLogPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_MMA_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl = VOS_NULL_PTR;
    VOS_VOID                                               *pstMmaLogPrivacyMatchMsg  = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafMmaAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* ����ulReceiverPid����PIDӳ��� */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (pstRcvMsgHeader->ulReceiverPid == g_astTafMmaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafMmaAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafMmaAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* ������ulReceiverPid�ҵ�PIDӳ�����������ƥ��� */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)pstRcvMsgHeader->ulMsgName;

        /* ����ulMsgName���ҹ��˺���ӳ��� */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstMmaLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstMmaLogPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl       = VOS_NULL_PTR;
    VOS_VOID                                               *pstDrvAgentLogPrivacyMatchMsg  = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader                = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* ����ulReceiverPid����PIDӳ��� */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (pstRcvMsgHeader->ulReceiverPid == g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafDrvAgentAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* ������ulReceiverPid�ҵ�PIDӳ�����������ƥ��� */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (VOS_UINT16)pstRcvMsgHeader->ulMsgName;

        /* ����ulMsgName���ҹ��˺���ӳ��� */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstDrvAgentLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstDrvAgentLogPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID TAF_OM_LayerMsgLogPrivacyMatchRegAcore(VOS_VOID)
{
     /*  1��AT��PID initʱ�ȶ�ȡNV, Ȼ����ע�������ص�����, ���Դ˴�ֱ��ʹ��NVֵ,
       *     log����NV��, ��ע��ص�����, ����ע��
       *  2��A�˵�����, ֻ�ܷ���I0�ӿ�, ����I1/I2����Ϣͬ��ע��I0�Ĺ��˽ӿ�, �ڹ��˺���������PIDת��
       *  3����Ʒ��ȷ��NV������MODEM_0Ϊ׼
       */

    if (VOS_TRUE == AT_GetPrivacyFilterEnableFlg())
    {
        PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT,     AT_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(ACPU_PID_RNIC,    RNIC_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_DSM,  TAF_DSM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_CCM,  TAF_CCM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_MTA,  TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I0_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif

#if (MULTI_MODEM_NUMBER >= 2)
        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_CCM,  TAF_CCM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_DSM,  TAF_DSM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif

        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_MTA,  TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I1_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif

#if (3 == MULTI_MODEM_NUMBER)

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_TAF, TAF_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_CCM,  TAF_CCM_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_DSM,  TAF_DSM_AcoreMsgLogPrivacyMatchProc);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_XSMS, TAF_XSMS_AcoreMsgLogPrivacyMatchProc);
#endif

        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_MTA,  TAF_MTA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_MMA, TAF_MMA_AcoreMsgLogPrivacyMatchProc);

        PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_DRV_AGENT, TAF_DRVAGENT_AcoreMsgLogPrivacyMatchProc);

#if ((FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
        PS_OM_LayerMsgReplaceCBReg(I2_UEPS_PID_XPDS, TAF_XPDS_AcoreMsgLogPrivacyMatchProc);
#endif
#endif
#endif
    }

    return;
}


VOS_VOID* RNIC_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    VOS_VOID                                               *pstPrivacyMsg            = VOS_NULL_PTR;
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstPrivacyMatchMsgTbl    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader          = VOS_NULL_PTR;
    VOS_UINT32                                              ulTblSize;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulTblSize       = 0;

    ulRcvPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* ���������Ϣ��Ӧ��rcvpid�� */
    for (ulLoop = 0; ulLoop < (sizeof(g_astRnicAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU)); ulLoop++)
    {
        if (ulRcvPid == g_astRnicAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstPrivacyMatchMsgTbl   = g_astRnicAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulTblSize               = g_astRnicAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    ulMsgName = (VOS_UINT16)(pstRcvMsgHeader->ulMsgName);

    /* ���������Ϣ��Ӧ���������� */
    if (VOS_NULL_PTR != pstPrivacyMatchMsgTbl)
    {
        for (ulLoop = 0; ulLoop < ulTblSize; ulLoop++)
        {
            if (ulMsgName == pstPrivacyMatchMsgTbl[ulLoop].ulMsgName)
            {
                pstPrivacyMsg = pstPrivacyMatchMsgTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstPrivacyMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_DSM_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl    = VOS_NULL_PTR;
    VOS_VOID                                               *pstTafLogPrivacyMatchMsg    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader             = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              i;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulReceiverPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafDsmAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    ulReceiverPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* ����rcv pid����pidӳ��� */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        pstLogPrivacyMsgMatchTbl = g_astTafDsmAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

        if ((pstLogPrivacyMsgMatchTbl == VOS_NULL_PTR)
         || (g_astTafDsmAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid != ulReceiverPid))
        {
            continue;
        }

        ulMsgMatchTblSize        = g_astTafDsmAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

        ulMsgName = pstRcvMsgHeader->ulMsgName;

        if (WUEPS_PID_AT == ulReceiverPid)
        {
            if ( (ulMsgName == MN_CALLBACK_PS_CALL)
              || (ulMsgName == MN_CALLBACK_IFACE))
            {
                ulMsgName = ((TAF_PS_EVT_STRU *)pstRcvMsgHeader)->ulEvtId;
            }
        }

        /* ����msg name���ҹ��˺���ӳ��� */
        for (i = 0; i < ulMsgMatchTblSize; i++)
        {
            if (pstLogPrivacyMsgMatchTbl[i].ulMsgName != ulMsgName)
            {
                continue;
            }

            if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl[i].pFuncPrivacyMatch)
            {
                pstTafLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[i].pFuncPrivacyMatch((MsgBlock *)pstMsg);
                return pstTafLogPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}


VOS_VOID* TAF_CCM_AcoreMsgLogPrivacyMatchProc(
    MsgBlock                           *pstMsg
)
{
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU                     *pstLogPrivacyMsgMatchTbl    = VOS_NULL_PTR;
    VOS_VOID                                               *pstTafLogPrivacyMatchMsg    = VOS_NULL_PTR;
    MSG_HEADER_STRU                                        *pstRcvMsgHeader             = VOS_NULL_PTR;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulRcvPidMatchTblSize;
    VOS_UINT32                                              ulMsgMatchTblSize;
    VOS_UINT32                                              ulMsgName;
    VOS_UINT32                                              ulRcvPid;

    pstRcvMsgHeader = (MSG_HEADER_STRU *)pstMsg;

    ulRcvPidMatchTblSize = sizeof(g_astTafCcmAcorePrivacyMatchRcvPidListTbl)/sizeof(TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU);
    ulMsgMatchTblSize    = 0;

    /* A�˵�����, ��Ҫ��I1/I2��PIDת��ΪI0��PID */
    ulRcvPid = TAF_LogPrivacyGetModem0Pid(pstRcvMsgHeader->ulReceiverPid);

    /* ����rcv pid����pidӳ��� */
    for (ulLoop = 0; ulLoop < ulRcvPidMatchTblSize; ulLoop++)
    {
        if (ulRcvPid == g_astTafCcmAcorePrivacyMatchRcvPidListTbl[ulLoop].ulReceiverPid)
        {
            pstLogPrivacyMsgMatchTbl = g_astTafCcmAcorePrivacyMatchRcvPidListTbl[ulLoop].pstLogPrivacyMatchMsgTbl;

            ulMsgMatchTblSize        = g_astTafCcmAcorePrivacyMatchRcvPidListTbl[ulLoop].ulTblSize;

            break;
        }
    }

    /* ������rcv pid�ҵ�pidӳ�����������ƥ�����Ҵ����� */
    if (VOS_NULL_PTR != pstLogPrivacyMsgMatchTbl)
    {
        ulMsgName = (pstRcvMsgHeader->ulMsgName);

        /* ����msg name���ҹ��˺���ӳ��� */
        for (ulLoop = 0; ulLoop < ulMsgMatchTblSize; ulLoop++)
        {
            if (ulMsgName == pstLogPrivacyMsgMatchTbl[ulLoop].ulMsgName)
            {
                pstTafLogPrivacyMatchMsg = pstLogPrivacyMsgMatchTbl[ulLoop].pFuncPrivacyMatch((MsgBlock *)pstMsg);

                return pstTafLogPrivacyMatchMsg;
            }
        }
    }

    /* û�ҵ���������ֱ�ӷ���ԭ��Ϣ */
    return (VOS_VOID *)pstMsg;
}



