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

#ifndef __ATCMDCALLPROC_H__
#define __ATCMDCALLPROC_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
#include "AtMnInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

#define AT_CALL_D_GI_PARA_LEN                          2


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


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_SetCecallPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryCecallPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetEclstartPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_TestEclstartPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetEclstopPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetEclcfgPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryEclcfgPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetEclmsdPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryEclmsdPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_TestEclmsdPara(VOS_UINT8 ucIndex);
VOS_UINT8 AT_HaveEcallActive(VOS_UINT8 ucIndex, VOS_UINT8 ucCheckFlag);
VOS_UINT32 AT_SetEclpushPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetEclAbortPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryEclListPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetEclModePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryEclModePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_TestEclModePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvVcMsgEcallPushCnfProc(MN_AT_IND_EVT_STRU *pstData);
VOS_UINT32 AT_ProcVcEcallAbortCnf(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU              *pstVcEvtInfo
);
VOS_UINT32 AT_ProcVcReportEcallAlackEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU*             pstVcEvtInfo);
VOS_VOID AT_RcvTafEcallStatusErrorInd(VOS_VOID);
#endif

VOS_UINT32 At_RcvVcMsgDtmfDecoderIndProc(
    MN_AT_IND_EVT_STRU                 *pstData
);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmSndFlashRslt(
    VOS_VOID                           *pEvtInfo
);

extern VOS_UINT32 AT_RcvTafCcmSndBurstDTMFCnf(
    VOS_VOID                           *pEvtInfo
);

extern VOS_UINT32 AT_RcvTafCcmSndBurstDTMFRslt(
    VOS_VOID                           *pEvtInfo
);

extern VOS_UINT32 At_TestCBurstDTMFPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_SetCBurstDTMFPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_SetCfshPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_CheckCfshNumber(
    VOS_UINT8                          *pucAtPara,
    VOS_UINT16                          usLen
);

extern VOS_UINT32 AT_RcvTafCcmCalledNumInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmCallingNumInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmDispInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmExtDispInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmConnNumInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmRedirNumInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmSignalInfoInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmLineCtrlInfoInd(
    VOS_VOID                           *pEvtInfo
);

extern VOS_UINT32 AT_RcvTafCcmCCWACInd(
    VOS_VOID                           *pEvtInfo
);

extern VOS_UINT32 At_TestCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
);
extern VOS_UINT32 AT_SetCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
);
extern VOS_UINT32 AT_CheckCContDtmfKeyPara(VOS_VOID);
extern VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFCnf(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFRslt(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmRcvContinuousDtmfInd(
    VOS_VOID                           *pEvtInfo
);
extern VOS_UINT32 AT_RcvTafCcmRcvBurstDtmfInd(
    VOS_VOID                           *pEvtInfo
);
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryCimsErrPara(VOS_UINT8 ucIndex);
#endif
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_TestCclprPara( VOS_UINT8 ucIndex );
#endif
extern VOS_UINT32 AT_SetRejCallPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_TestRejCallPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_QryCsChannelInfoPara(VOS_UINT8 ucIndex);

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

#endif /* end of AtCmdCallProc.h */
