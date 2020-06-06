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

#ifndef __TAFLOGPRIVACYMATCH_H__
#define __TAFLOGPRIVACYMATCH_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/

#include "vos.h"
#include "PsTypeDef.h"
#include "TafSsaApi.h"
#include "AtInternalMsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum TAF_LOG_PRIVACY_AT_CMD_ENUM
{
    TAF_LOG_PRIVACY_AT_CMD_BUTT                     = 0,

    TAF_LOG_PRIVACY_AT_CMD_CIMI                     = 1,
    TAF_LOG_PRIVACY_AT_CMD_CGSN                     = 2,
    TAF_LOG_PRIVACY_AT_CMD_MSID                     = 3,
    TAF_LOG_PRIVACY_AT_CMD_HPLMN                    = 4,
};
typedef VOS_UINT32 TAF_LOG_PRIVACY_AT_CMD_ENUM_UINT32;

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

typedef VOS_VOID* (*pTafLogPrivacyMatchFunc)(
    MsgBlock                           *pstMsg
);


typedef struct
{
    VOS_UINT32                          ulMsgName;                          /* ��Ϣ���� */
    pTafLogPrivacyMatchFunc             pFuncPrivacyMatch;                  /* ��Ϣ��Ӧ�Ĺ��˺��� */
}TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU;


typedef struct
{   VOS_UINT32                          ulReceiverPid;                      /* ��Ϣ����PID */
    VOS_UINT32                          ulTblSize;                          /* ��Ϣ���˺���ƥ�䴦����С */
    TAF_LOG_PRIVACY_MSG_MATCH_TBL_STRU *pstLogPrivacyMatchMsgTbl;           /* ��Ϣ���˺���ƥ�䴦��� */
}TAF_LOG_PRIVACY_RCV_PID_MATCH_TBL_STRU;


typedef struct
{
    VOS_UINT32                           ulModem0Pid;
    VOS_UINT32                           ulModem1Pid;
    VOS_UINT32                           ulModem2Pid;
}TAF_LOG_PRIVACY_MATCH_MODEM_PID_MAP_TBL_STRU;


typedef struct
{
    VOS_CHAR                           *pcOriginalAtCmd;
    VOS_CHAR                           *pcPrivacyAtCmd;
}AT_LOG_PRIVACY_MATCH_AT_CMD_MAP_TBL_STRU;

typedef AT_MSG_STRU* (*AT_PRIVACY_FILTER_AT_PROC_FUNC)(
    AT_MSG_STRU                        *pstAtMsg
);


typedef struct
{
    VOS_UINT32                          ulAtCmdType;
    AT_PRIVACY_FILTER_AT_PROC_FUNC      pcPrivacyAtCmd;
}AT_LOG_PRIVACY_MAP_CMD_TO_FUNC_STRU;


typedef VOS_VOID (*pTafIntraMsgPrivacyMatchFunc)(
    MsgBlock                           *pstMsg
);


typedef struct
{
    VOS_UINT32                          ulMsgName;                          /* ��Ϣ���� */
    pTafIntraMsgPrivacyMatchFunc        pTafIntraMsgPrivacyMatchFunc;       /* ��Ϣ��Ӧ�Ĺ��˺��� */
}TAF_LOG_PRIVACY_INTRA_MSG_MATCH_TBL_STRU;


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
VOS_UINT32 TAF_MnCallBackSsLcsEvtIsNeedLogPrivacy(
    TAF_SSA_EVT_ID_ENUM_UINT32          enEvtId
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* AT_PrivacyMatchAppMsgTypeSendReq(
    MsgBlock                           *pstMsg
);
#endif

VOS_VOID*  AT_PrivacyMatchCposSetReq(
    MsgBlock                           *pstMsg
);

VOS_VOID*  AT_PrivacyMatchSimLockWriteExSetReq(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_IMS)
VOS_VOID* AT_PrivacyMatchImsaImsCtrlMsg(
    MsgBlock                                               *pstMsg
);
VOS_VOID* AT_PrivacyMatchImsaNickNameSetReq(
    MsgBlock                                               *pstMsg
);
#endif

VOS_VOID*  AT_PrivacyMatchMeidSetReq(
    MsgBlock                           *pstMsg
);
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* AT_PrivacyMatchAppMsgTypeWriteReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchAppMsgTypeDeleteReq(
    MsgBlock                           *pstMsg
);
#endif

VOS_VOID* TAF_PrivacyMatchAppMnCallBackCsCall(
    MsgBlock                           *pstMsg
);
VOS_VOID* AT_PrivacyMatchRegisterSsMsg(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchProcessUssMsg(
    MsgBlock                           *pstMsg
);
VOS_VOID* AT_PrivacyMatchInterRogateMsg(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchErasessMsg(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchActivatessMsg(
    MsgBlock                           *pstMsg
);
VOS_VOID* AT_PrivacyMatchDeactivatessMsg(
    MsgBlock                           *pstMsg
);
VOS_VOID* AT_PrivacyMatchRegPwdMsg(
    MsgBlock                           *pstMsg
);


VOS_VOID* AT_PrivacyMatchCallAppCustomEccNumReq(
    MsgBlock                           *pstMsg
);


VOS_VOID* TAF_PrivacyMatchMnCallBackSsLcsEvt(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchMnCallBackSsAtIndEvt(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchAppMnCallBackSs(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* TAF_XSMS_PrivacyMatchAppMsgTypeRcvInd(
    MsgBlock                           *pstMsg
);

VOS_VOID*  TAF_XSMS_PrivacyMatchAppMsgTypeWriteCnf(
    MsgBlock                           *pstMsg
);

#if ((FEATURE_ON == FEATURE_AGPS) && (FEATURE_ON == FEATURE_XPDS))
VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsRefLocInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsIonInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsEphInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAlmInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsPdePosiInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtGpsAcqAssistDataInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtApReverseDataInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_XPDS_PrivacyMatchAtUtsGpsPosInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCagpsPosInfoRsp(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCagpsPrmInfoRsp(
    MsgBlock                           *pstMsg
);

VOS_VOID* AT_PrivacyMatchCagpsApForwardDataInd(
    MsgBlock                           *pstMsg
);
#endif
#endif

VOS_VOID* TAF_MTA_PrivacyMatchCposrInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtMeidQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtCgsnQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchAtCallBackQryProc(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtUsimStatusInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtHomePlmnQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DRVAGENT_PrivacyMatchAtMsidQryCnf(
    MsgBlock                           *pstMsg
);


VOS_VOID* AT_PrivacyMatchSmsAppMsgReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchAtCallBackSmsProc(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtEcidSetCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoSetReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfClocInfoQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoSetReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtEfPsClocInfoQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_CCM_PrivacyMatchCallOrigReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCallSupsCmdReq(
    MsgBlock                                               *pstMsg
);

VOS_VOID* TAF_CCM_PrivacyMatchCustomDialReqCmdReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmStartDtmfReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmStopDtmfReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmSetUusInfoReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmCustomEccNumReq(
    MsgBlock                                               *pstMsg
);
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* TAF_CCM_PrivacyMatchCcmSendFlashReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmSendBrustDtmfReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmSendContDtmfReq(
    MsgBlock                                               *pstMsg
);
VOS_VOID* TAF_CCM_PrivacyMatchCcmEncryptVoiceReq(
    MsgBlock                                               *pstMsg
);
#if (FEATURE_ON == FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE)
VOS_VOID* TAF_CCM_PrivacyMatchCcmSetEcKmcReq(
    MsgBlock                                               *pstMsg
);
#endif
#endif

#if (FEATURE_ON == FEATURE_IMS)
VOS_VOID* TAF_PrivacyMatchAppQryEconfCalledInfoCnf(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtEconfNotifyInd(
    MsgBlock                           *pstMsg
);
#endif

VOS_VOID* TAF_PrivacyMatchAtCallIncomingInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtCallConnectInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtQryCallInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtCallSsInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtEccNumInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtQryXlemaInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtCnapQryCnf(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtCnapInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtQryUus1InfoCnf(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtUus1InfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtQryEcallInfoCnf(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtQryClprCnf(
    MsgBlock                           *pstMsg
);
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* TAF_PrivacyMatchAtCalledNumInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtCallingNumInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtDisplayInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtExtDisplayInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtConnNumInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtRedirNumInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtCcwacInfoInfoInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtContDtmfInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtBurstDtmfInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtEncryptVoiceInd(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtGetEcRandomCnf(
    MsgBlock                           *pstMsg
);
VOS_VOID* TAF_PrivacyMatchAtGetEcKmcCnf(
    MsgBlock                           *pstMsg
);
#endif

#if (FEATURE_ON == FEATURE_IMS)
VOS_VOID* TAF_CCM_PrivacyMatchCcmEconfDialReq(
    MsgBlock                                               *pstMsg
);
#endif
VOS_VOID* TAF_DSM_PrivacyMatchTafSetAuthDataReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchTafSetSetPdpDnsInfoReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchTafSetGetPdpDnsInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchTafGetNegotiationDnsCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonScellCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonNcellCnf(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_UE_MODE_NR)
VOS_VOID* TAF_MTA_PrivacyMatchAtSetNetMonSScellCnf(
    MsgBlock                           *msg
);
#endif

VOS_VOID* AT_PrivacyMatchPseucellInfoSetReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtLocationInfoQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtRegStatusInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtSrchedPlmnInfoInd(
    MsgBlock                           *pstMsg
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_VOID* TAF_MMA_PrivacyMatchAtCdmaLocInfoQryCnf(
    MsgBlock                           *pstMsg
);
#endif /* (FEATURE_ON == FEATURE_UE_MODE_CDMA) */

VOS_VOID* TAF_MMA_PrivacyMatchAtNetScanCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtRegStateQryCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtClocInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_MMA_PrivacyMatchAtRejInfoQryCnf(
    MsgBlock                           *pstMsg
);


VOS_VOID* RNIC_PrivacyMatchCdsImsDataReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpActInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpManageInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpModInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallPdpIpv6InfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetPrimPdpCtxInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetTftInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetPdpIpAddrInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetDynamicTftInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsGetAuthdataInfoCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsReportPcoInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchDsmPsCallOrigReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchDsmPsPppDialOrigReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchDsmPsSetPrimPdpCtxInfoReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchDsmPsSetTftInfoReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsEpdgCtrluNtf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchPsCallHandoverRstNtf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchDsmPsEpdgCtrlNtf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_PrivacyMatchDsmIfaceUpReq(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceGetDynamicParaCnf(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchTafIfaceRabInfoInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchRnicIfaceCfgInd(
    MsgBlock                           *pstMsg
);

VOS_VOID* TAF_DSM_PrivacyMatchNdisIfaceUpInd(
    MsgBlock                           *pstMsg
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

#endif /* end of TafLogPrivacyMatch.h */
