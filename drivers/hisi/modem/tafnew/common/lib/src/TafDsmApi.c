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


#include "PsCommonDef.h"
#include "TafPsApi.h"
#include "securec.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "TafDsmMntn.h"
#include "TafDsmComFunc.h"
#endif



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_TAF_DSM_API_C

/* Log Print Module Define */
#ifndef THIS_MODU
#define THIS_MODU    ps_nas
#endif

/*****************************************************************************
   2 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
   3 �ⲿ��������
*****************************************************************************/

/*****************************************************************************
   4 ����ʵ��
*****************************************************************************/

VOS_UINT32 TAF_PS_SndDsmMsg(
    VOS_UINT16                          usModemId,
    VOS_UINT32                          ulMsgId,
    const VOS_VOID                     *pData,
    VOS_UINT32                          ulLength
)
{
    TAF_PS_MSG_STRU                    *pstMsg   = VOS_NULL_PTR;
    VOS_UINT32                          ulSendPid;
    VOS_UINT32                          ulRcvPid;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;

    switch (usModemId)
    {
        case MODEM_ID_0:
            ulSendPid = I0_WUEPS_PID_TAF;
            ulRcvPid  = I0_UEPS_PID_DSM;
            break;

        case MODEM_ID_1:
            ulSendPid = I1_WUEPS_PID_TAF;
            ulRcvPid  = I1_UEPS_PID_DSM;
            break;

        case MODEM_ID_2:
            ulSendPid = I2_WUEPS_PID_TAF;
            ulRcvPid  = I2_UEPS_PID_DSM;
            break;

        default:
            PS_PRINTF_WARNING("<TAF_PS_SndDsmMsg> ModemId is error!\n");
            return VOS_ERR;
    }

    /* ������Ϣ */
    pstMsg = (TAF_PS_MSG_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(
                                ulSendPid,
                                sizeof(MSG_HEADER_STRU) + ulLength);
    if (VOS_NULL_PTR == pstMsg)
    {
        return VOS_ERR;
    }

    pstMsg->stHeader.ulSenderPid        = ulSendPid;
    pstMsg->stHeader.ulReceiverPid      = ulRcvPid;
    pstMsg->stHeader.ulMsgName          = ulMsgId;

    /* ��д��Ϣ���� */
    lMemResult = memcpy_s(pstMsg->aucContent, ulLength, pData, ulLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, ulLength, ulLength);

    /* ������Ϣ */
    ulResult = PS_SEND_MSG(ulSendPid, pstMsg);

    if (VOS_OK != ulResult)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 TAF_PS_SetPrimPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PDP_PRIM_CONTEXT_EXT_STRU      *pstPdpContextInfo
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU   stSetPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetPdpCtxInfoReq, sizeof(stSetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ��Ϣ */
    stSetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetPdpCtxInfoReq.stPdpContextInfo  = *pstPdpContextInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ,
                                &stSetPdpCtxInfoReq,
                                sizeof(TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPrimPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ_STRU   stGetPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetPdpCtxInfoReq, sizeof(stGetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ,
                                &stGetPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_PDP_CONTEXT_INFO_REQ_STRU        stGetPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetPdpCtxInfoReq, sizeof(stGetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDPCONT_INFO_REQ,
                                &stGetPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetSecPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PDP_SEC_CONTEXT_EXT_STRU       *pstPdpContextInfo
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ_STRU    stSetPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetPdpCtxInfoReq, sizeof(stSetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ��Ϣ */
    stSetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetPdpCtxInfoReq.stPdpContextInfo  = *pstPdpContextInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ,
                                &stSetPdpCtxInfoReq,
                                sizeof(TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetSecPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ_STRU    stGetPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetPdpCtxInfoReq, sizeof(stGetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ,
                                &stGetPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetTftInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_TFT_EXT_STRU                   *pstTftInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_TFT_INFO_REQ_STRU        stSetTftInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetTftInfoReq, sizeof(stSetTftInfoReq), 0x00, sizeof(TAF_PS_SET_TFT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_TFT_INFO_REQ��Ϣ */
    stSetTftInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetTftInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetTftInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetTftInfoReq.stTftInfo         = *pstTftInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_TFT_INFO_REQ,
                                &stSetTftInfoReq,
                                sizeof(TAF_PS_SET_TFT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetTftInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_TFT_INFO_REQ_STRU        stGetTftInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetTftInfoReq, sizeof(stGetTftInfoReq), 0x00, sizeof(TAF_PS_GET_TFT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGetTftInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetTftInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetTftInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_TFT_INFO_REQ,
                                &stGetTftInfoReq,
                                sizeof(TAF_PS_GET_TFT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetUmtsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_UMTS_QOS_EXT_STRU              *pstUmtsQosInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_UMTS_QOS_INFO_REQ_STRU   stSetUmtsQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetUmtsQosInfoReq, sizeof(stSetUmtsQosInfoReq), 0x00, sizeof(TAF_PS_SET_UMTS_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_UMTS_QOS_INFO_REQ��Ϣ */
    stSetUmtsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetUmtsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetUmtsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetUmtsQosInfoReq.stUmtsQosInfo     = *pstUmtsQosInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_UMTS_QOS_INFO_REQ,
                                &stSetUmtsQosInfoReq,
                                sizeof(TAF_PS_SET_UMTS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetUmtsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_UMTS_QOS_INFO_REQ_STRU   stGetUmtsQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetUmtsQosInfoReq, sizeof(stGetUmtsQosInfoReq), 0x00, sizeof(TAF_PS_GET_UMTS_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_UMTS_QOS_INFO_REQ��Ϣ */
    stGetUmtsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetUmtsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetUmtsQosInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_UMTS_QOS_INFO_REQ,
                                &stGetUmtsQosInfoReq,
                                sizeof(TAF_PS_GET_UMTS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetUmtsQosMinInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_UMTS_QOS_EXT_STRU              *pstUmtsQosMinInfo
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ_STRU   stSetUmtsQosMinInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetUmtsQosMinInfoReq, sizeof(stSetUmtsQosMinInfoReq), 0x00, sizeof(TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ��Ϣ */
    stSetUmtsQosMinInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetUmtsQosMinInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetUmtsQosMinInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetUmtsQosMinInfoReq.stUmtsQosMinInfo  = *pstUmtsQosMinInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ,
                                &stSetUmtsQosMinInfoReq,
                                sizeof(TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetUmtsQosMinInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ_STRU   stGetUmtsQosMinInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetUmtsQosMinInfoReq, sizeof(stGetUmtsQosMinInfoReq), 0x00, sizeof(TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ��Ϣ */
    stGetUmtsQosMinInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetUmtsQosMinInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetUmtsQosMinInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ,
                                &stGetUmtsQosMinInfoReq,
                                sizeof(TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicUmtsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STRU                  *pstCidListInfo
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ_STRU   stGetDynamicUmtsQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetDynamicUmtsQosInfoReq, sizeof(stGetDynamicUmtsQosInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ��Ϣ */
    stGetDynamicUmtsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicUmtsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicUmtsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicUmtsQosInfoReq.stCidListInfo     = *pstCidListInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ,
                                &stGetDynamicUmtsQosInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetPdpContextState(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STATE_STRU            *pstCidListStateInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_PDP_STATE_REQ_STRU       stSetPdpStateReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetPdpStateReq, sizeof(stSetPdpStateReq), 0x00, sizeof(TAF_PS_SET_PDP_STATE_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_PDP_STATE_REQ��Ϣ */
    stSetPdpStateReq.stCtrl.ulModuleId  = ulModuleId;
    stSetPdpStateReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpStateReq.stCtrl.ucOpId      = ucOpId;
    stSetPdpStateReq.stCidListStateInfo = *pstCidListStateInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_PDP_CONTEXT_STATE_REQ,
                                &stSetPdpStateReq,
                                sizeof(TAF_PS_SET_PDP_STATE_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpContextState(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_PDP_STATE_REQ_STRU       stGetPdpStateReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetPdpStateReq, sizeof(stGetPdpStateReq), 0x00, sizeof(TAF_PS_GET_PDP_STATE_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_PDP_STATE_REQ��Ϣ */
    stGetPdpStateReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpStateReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpStateReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDP_CONTEXT_STATE_REQ,
                                &stGetPdpStateReq,
                                sizeof(TAF_PS_GET_PDP_STATE_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_CallModify(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STRU                  *pstCidListInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_MODIFY_REQ_STRU         stCallModifyReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stCallModifyReq, sizeof(stCallModifyReq), 0x00, sizeof(TAF_PS_CALL_MODIFY_REQ_STRU));

    /* ����ID_MSG_TAF_PS_CALL_MODIFY_REQ��Ϣ */
    stCallModifyReq.stCtrl.ulModuleId = ulModuleId;
    stCallModifyReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallModifyReq.stCtrl.ucOpId     = ucOpId;
    stCallModifyReq.stCidListInfo     = *pstCidListInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_MODIFY_REQ,
                                &stCallModifyReq,
                                sizeof(TAF_PS_CALL_MODIFY_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_CallAnswer(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_ANSWER_STRU                 *pstAnsInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_ANSWER_REQ_STRU         stCallAnswerReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stCallAnswerReq, sizeof(stCallAnswerReq), 0x00, sizeof(TAF_PS_CALL_ANSWER_REQ_STRU));

    /* ����ID_MSG_TAF_PS_CALL_ANSWER_REQ��Ϣ */
    stCallAnswerReq.stCtrl.ulModuleId = ulModuleId;
    stCallAnswerReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallAnswerReq.stCtrl.ucOpId     = ucOpId;
    stCallAnswerReq.stAnsInfo         = *pstAnsInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_ANSWER_REQ,
                                &stCallAnswerReq,
                                sizeof(TAF_PS_CALL_ANSWER_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_CallHangup(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_HANGUP_REQ_STRU         stCallHangupReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stCallHangupReq, sizeof(stCallHangupReq), 0x00, sizeof(TAF_PS_CALL_HANGUP_REQ_STRU));

    /* ����ID_MSG_TAF_PS_CALL_HANGUP_REQ��Ϣ */
    stCallHangupReq.stCtrl.ulModuleId = ulModuleId;
    stCallHangupReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallHangupReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_HANGUP_REQ,
                                &stCallHangupReq,
                                sizeof(TAF_PS_CALL_HANGUP_REQ_STRU));

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_CallOrig(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_DIAL_PARA_STRU              *pstDialParaInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_ORIG_REQ_STRU           stCallOrigReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stCallOrigReq, sizeof(stCallOrigReq), 0x00, sizeof(TAF_PS_CALL_ORIG_REQ_STRU));

    /* ����ID_MSG_TAF_PS_CALL_ORIG_REQ��Ϣ */
    stCallOrigReq.stCtrl.ulModuleId = ulModuleId;
    stCallOrigReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallOrigReq.stCtrl.ucOpId     = ucOpId;

    lMemResult = memcpy_s(&stCallOrigReq.stDialParaInfo,
                          sizeof(stCallOrigReq.stDialParaInfo),
                          pstDialParaInfo,
                          sizeof(TAF_PS_DIAL_PARA_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCallOrigReq.stDialParaInfo), sizeof(TAF_PS_DIAL_PARA_STRU));

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_ORIG_REQ,
                                &stCallOrigReq,
                                sizeof(TAF_PS_CALL_ORIG_REQ_STRU));

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_CallEnd(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    return TAF_PS_CallEndEx(ulModuleId, usExClientId, ucOpId, ucCid, TAF_PS_CALL_END_CAUSE_NORMAL);
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_CallEndEx(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_END_REQ_STRU            stCallEndReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stCallEndReq, sizeof(stCallEndReq), 0x00, sizeof(TAF_PS_CALL_END_REQ_STRU));

    /* ����ID_MSG_TAF_PS_CALL_END_REQ��Ϣ */
    stCallEndReq.stCtrl.ulModuleId = ulModuleId;
    stCallEndReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallEndReq.stCtrl.ucOpId     = ucOpId;
    stCallEndReq.ucCid             = ucCid;
    stCallEndReq.enCause           = enCause;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_END_REQ,
                                &stCallEndReq,
                                sizeof(TAF_PS_CALL_END_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpIpAddrInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STRU                  *pstCidListInfo
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_PDP_IP_ADDR_INFO_REQ_STRU    stGetPdpIpAddrReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetPdpIpAddrReq, sizeof(stGetPdpIpAddrReq), 0x00, sizeof(TAF_PS_GET_PDP_IP_ADDR_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_PDP_IP_ADDR_INFO_REQ��Ϣ */
    stGetPdpIpAddrReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpIpAddrReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpIpAddrReq.stCtrl.ucOpId     = ucOpId;
    stGetPdpIpAddrReq.stCidListInfo     = *pstCidListInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDP_IP_ADDR_INFO_REQ,
                                &stGetPdpIpAddrReq,
                                sizeof(TAF_PS_GET_PDP_IP_ADDR_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetAnsModeInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT32                          ulAnsMode
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_SET_ANSWER_MODE_INFO_REQ_STRU    stSetAnsModeReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetAnsModeReq, sizeof(stSetAnsModeReq), 0x00, sizeof(TAF_PS_SET_ANSWER_MODE_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_ANSWER_MODE_INFO_REQ��Ϣ */
    stSetAnsModeReq.stCtrl.ulModuleId = ulModuleId;
    stSetAnsModeReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetAnsModeReq.stCtrl.ucOpId     = ucOpId;
    stSetAnsModeReq.ulAnsMode         = ulAnsMode;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_ANSWER_MODE_INFO_REQ,
                                &stSetAnsModeReq,
                                sizeof(TAF_PS_SET_ANSWER_MODE_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetAnsModeInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_ANSWER_MODE_INFO_REQ_STRU    stGetAnsModeReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetAnsModeReq, sizeof(stGetAnsModeReq), 0x00, sizeof(TAF_PS_GET_ANSWER_MODE_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_ANSWER_MODE_INFO_REQ��Ϣ */
    stGetAnsModeReq.stCtrl.ulModuleId   = ulModuleId;
    stGetAnsModeReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetAnsModeReq.stCtrl.ucOpId       = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_ANSWER_MODE_INFO_REQ,
                                &stGetAnsModeReq,
                                sizeof(TAF_PS_GET_ANSWER_MODE_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicPrimPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                          ulResult;
    TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ_STRU   stGetDynamicPrimPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetDynamicPrimPdpCtxInfoReq, sizeof(stGetDynamicPrimPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGetDynamicPrimPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicPrimPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicPrimPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicPrimPdpCtxInfoReq.ucCid             = ucCid;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ,
                                &stGetDynamicPrimPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicSecPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                          ulResult;
    TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ_STRU    stGetDynamicSecPdpCtxInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetDynamicSecPdpCtxInfoReq, sizeof(stGetDynamicSecPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGetDynamicSecPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicSecPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicSecPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicSecPdpCtxInfoReq.ucCid             = ucCid;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ,
                                &stGetDynamicSecPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicTftInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_DYNAMIC_TFT_INFO_REQ_STRU    stGetDynamicTftInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetDynamicTftInfoReq, sizeof(stGetDynamicTftInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_TFT_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DYNAMIC_TFT_INFO_REQ��Ϣ */
    stGetDynamicTftInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicTftInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicTftInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicTftInfoReq.ucCid             = ucCid;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_TFT_INFO_REQ,
                                &stGetDynamicTftInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_TFT_INFO_REQ_STRU));

    return ulResult;
}

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_UINT32 TAF_PS_Set5QosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_5G_QOS_EXT_STRU                *pst5gQosInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_5G_QOS_INFO_REQ_STRU     stSet5QosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSet5QosInfoReq, sizeof(stSet5QosInfoReq), 0x00, sizeof(TAF_PS_SET_5G_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_5G_QOS_INFO_REQ��Ϣ */
    stSet5QosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSet5QosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSet5QosInfoReq.stCtrl.ucOpId     = ucOpId;
    stSet5QosInfoReq.st5QosInfo        = *pst5gQosInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_5G_QOS_INFO_REQ,
                                &stSet5QosInfoReq,
                                sizeof(TAF_PS_SET_5G_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_Get5gQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_5G_QOS_INFO_REQ_STRU     stGet5gQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGet5gQosInfoReq, sizeof(stGet5gQosInfoReq), 0x00, sizeof(TAF_PS_GET_5G_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ��Ϣ */
    stGet5gQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGet5gQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGet5gQosInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_5G_QOS_INFO_REQ,
                                &stGet5gQosInfoReq,
                                sizeof(TAF_PS_GET_5G_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamic5gQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                              ulResult;
    TAF_PS_GET_DYNAMIC_5G_QOS_INFO_REQ_STRU                 stGetDynamic5gQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetDynamic5gQosInfoReq, sizeof(stGetDynamic5gQosInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_5G_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ��Ϣ */
    stGetDynamic5gQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamic5gQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamic5gQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamic5gQosInfoReq.ucCid             = ucCid;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_5G_QOS_INFO_REQ,
                                &stGetDynamic5gQosInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_5G_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetUePolicyRpt(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucEnable
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_UE_POLICY_RPT_REQ_STRU   stSetUePolicyReq;

    memset_s(&stSetUePolicyReq, sizeof(stSetUePolicyReq), 0x00, sizeof(TAF_PS_SET_UE_POLICY_RPT_REQ_STRU));

    /* ��ʼ�� */
    ulResult = VOS_OK;

    /* ����ID_MSG_TAF_PS_GET_UE_POLICY_REQ��Ϣ */
    stSetUePolicyReq.stCtrl.ulModuleId = ulModuleId;
    stSetUePolicyReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetUePolicyReq.stCtrl.ucOpId     = ucOpId;
    stSetUePolicyReq.ucEnable          = ucEnable;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_UE_POLICY_RPT_REQ,
                                &stSetUePolicyReq,
                                sizeof(TAF_PS_SET_UE_POLICY_RPT_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetUePolicyInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_UE_POLICY_REQ_STRU       stGetUePolicyReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;

    memset_s(&stGetUePolicyReq, sizeof(stGetUePolicyReq), 0x00, sizeof(TAF_PS_GET_UE_POLICY_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_UE_POLICY_REQ��Ϣ */
    stGetUePolicyReq.stCtrl.ulModuleId = ulModuleId;
    stGetUePolicyReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetUePolicyReq.stCtrl.ucOpId     = ucOpId;
    stGetUePolicyReq.ucIndex           = ucIndex;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_UE_POLICY_REQ,
                                &stGetUePolicyReq,
                                sizeof(TAF_PS_GET_UE_POLICY_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetCsUePolicy(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_5G_UE_POLICY_INFO_STRU      *pst5gUePolicy
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_5G_UE_POLICY_RSP_STRU       *pst5gUePolicyRsp = VOS_NULL_PTR;
    VOS_UINT32                          ulAllocLen;
    errno_t                             lMemResult;

    /* ��ʼ�� */
    ulResult   = VOS_OK;
    ulAllocLen = sizeof(TAF_PS_5G_UE_POLICY_RSP_STRU) + pst5gUePolicy->usLength;

    pst5gUePolicyRsp = (TAF_PS_5G_UE_POLICY_RSP_STRU *)PS_MEM_ALLOC(ulModuleId, ulAllocLen);

    /* ��������ڴ�ʧ�� */
    if (pst5gUePolicyRsp == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    memset_s(pst5gUePolicyRsp, ulAllocLen, 0x00, ulAllocLen);

    /* ����ID_MSG_TAF_PS_5G_UE_POLICY_INFO_RSP��Ϣ */
    pst5gUePolicyRsp->stCtrl.ulModuleId = ulModuleId;
    pst5gUePolicyRsp->stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    pst5gUePolicyRsp->stCtrl.ucOpId     = ucOpId;

    /* ���MsgType */
    pst5gUePolicyRsp->stUePolicyInfo.enUePolicyMsgType = pst5gUePolicy->enUePolicyMsgType;

    /* ��ֵclass mark info */
    pst5gUePolicyRsp->stUePolicyInfo.ucClassMarkLen = pst5gUePolicy->ucClassMarkLen;

    lMemResult = memcpy_s(pst5gUePolicyRsp->stUePolicyInfo.aucClassMark,
                          sizeof(pst5gUePolicyRsp->stUePolicyInfo.aucClassMark),
                          pst5gUePolicy->aucClassMark,
                          sizeof(pst5gUePolicy->aucClassMark));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pst5gUePolicyRsp->stUePolicyInfo.aucClassMark), sizeof(pst5gUePolicy->aucClassMark));

    if (pst5gUePolicy->usLength != 0)
    {
        pst5gUePolicyRsp->stUePolicyInfo.usLength = pst5gUePolicy->usLength;

        lMemResult = memcpy_s(pst5gUePolicyRsp->stUePolicyInfo.aucContent,
                              pst5gUePolicy->usLength,
                              pst5gUePolicy->aucContent,
                              pst5gUePolicy->usLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, pst5gUePolicy->usLength, pst5gUePolicy->usLength);
    }

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_5G_UE_POLICY_INFO_RSP,
                                pst5gUePolicyRsp,
                                (sizeof(TAF_PS_5G_UE_POLICY_RSP_STRU) + pst5gUePolicyRsp->stUePolicyInfo.usLength));

    PS_MEM_FREE(ulModuleId, pst5gUePolicyRsp);

    return ulResult;
}

#endif


VOS_UINT32 TAF_PS_SetEpsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_EPS_QOS_EXT_STRU               *pstEpsQosInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_EPS_QOS_INFO_REQ_STRU    stSetEpsQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetEpsQosInfoReq, sizeof(stSetEpsQosInfoReq), 0x00, sizeof(TAF_PS_SET_EPS_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_EPS_QOS_INFO_REQ��Ϣ */
    stSetEpsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetEpsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetEpsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetEpsQosInfoReq.stEpsQosInfo      = *pstEpsQosInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_EPS_QOS_INFO_REQ,
                                &stSetEpsQosInfoReq,
                                sizeof(TAF_PS_SET_EPS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetEpsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_EPS_QOS_INFO_REQ_STRU    stGetEpsQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetEpsQosInfoReq, sizeof(stGetEpsQosInfoReq), 0x00, sizeof(TAF_PS_GET_EPS_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_EPS_QOS_INFO_REQ��Ϣ */
    stGetEpsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetEpsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetEpsQosInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_EPS_QOS_INFO_REQ,
                                &stGetEpsQosInfoReq,
                                sizeof(TAF_PS_GET_EPS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicEpsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ_STRU    stGetDynamicEpsQosInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetDynamicEpsQosInfoReq, sizeof(stGetDynamicEpsQosInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ��Ϣ */
    stGetDynamicEpsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicEpsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicEpsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicEpsQosInfoReq.ucCid             = ucCid;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ,
                                &stGetDynamicEpsQosInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetPdpDnsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PDP_DNS_EXT_STRU               *pstPdpDnsInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_PDP_DNS_INFO_REQ_STRU    stSetPdpDnsInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetPdpDnsInfoReq, sizeof(stSetPdpDnsInfoReq), 0x00, sizeof(TAF_PS_SET_PDP_DNS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ��Ϣ */
    stSetPdpDnsInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetPdpDnsInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpDnsInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetPdpDnsInfoReq.stPdpDnsInfo      = *pstPdpDnsInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ,
                                &stSetPdpDnsInfoReq,
                                sizeof(TAF_PS_SET_PDP_DNS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpDnsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_PDP_DNS_INFO_REQ_STRU    stGetPdpDnsInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetPdpDnsInfoReq, sizeof(stGetPdpDnsInfoReq), 0x00, sizeof(TAF_PS_GET_PDP_DNS_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_PDP_DNS_INFO_REQ��Ϣ */
    stGetPdpDnsInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpDnsInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpDnsInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDP_DNS_INFO_REQ,
                                &stGetPdpDnsInfoReq,
                                sizeof(TAF_PS_GET_PDP_DNS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ConfigNbnsFunction(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT32                          ulEnabled
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_CONFIG_NBNS_FUNCTION_REQ_STRU    stConfigNbnsFunReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stConfigNbnsFunReq, sizeof(stConfigNbnsFunReq), 0x00, sizeof(TAF_PS_CONFIG_NBNS_FUNCTION_REQ_STRU));

    /* ����ID_MSG_TAF_PS_CONFIG_NBNS_FUNCTION_REQ��Ϣ */
    stConfigNbnsFunReq.stCtrl.ulModuleId = ulModuleId;
    stConfigNbnsFunReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stConfigNbnsFunReq.stCtrl.ucOpId     = ucOpId;
    stConfigNbnsFunReq.ulEnabled         = ulEnabled;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CONFIG_NBNS_FUNCTION_REQ,
                                &stConfigNbnsFunReq,
                                sizeof(TAF_PS_CONFIG_NBNS_FUNCTION_REQ_STRU));

    return ulResult;
}



VOS_UINT32 TAF_PS_SetAuthDataInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_AUTHDATA_EXT_STRU              *pstAuthDataInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_AUTHDATA_INFO_REQ_STRU   stSetAuthDataInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetAuthDataInfoReq, sizeof(stSetAuthDataInfoReq), 0x00, sizeof(TAF_PS_SET_AUTHDATA_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ��Ϣ */
    stSetAuthDataInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetAuthDataInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetAuthDataInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetAuthDataInfoReq.stAuthDataInfo    = *pstAuthDataInfo;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ,
                                &stSetAuthDataInfoReq,
                                sizeof(TAF_PS_SET_AUTHDATA_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetAuthDataInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_AUTHDATA_INFO_REQ_STRU   stGetAuthDataInfoReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetAuthDataInfoReq, sizeof(stGetAuthDataInfoReq), 0x00, sizeof(TAF_PS_GET_AUTHDATA_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_AUTHDATA_INFO_REQ��Ϣ */
    stGetAuthDataInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetAuthDataInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetAuthDataInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_AUTHDATA_INFO_REQ,
                                &stGetAuthDataInfoReq,
                                sizeof(TAF_PS_GET_AUTHDATA_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetGprsActiveType(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_ATD_PARA_STRU                  *pstAtdPara
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ_STRU  stGetGprsActiveTypeReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stGetGprsActiveTypeReq, sizeof(stGetGprsActiveTypeReq), 0x00, sizeof(TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_AUTHDATA_INFO_REQ��Ϣ */
    stGetGprsActiveTypeReq.stCtrl.ulModuleId = ulModuleId;
    stGetGprsActiveTypeReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetGprsActiveTypeReq.stCtrl.ucOpId     = ucOpId;
    stGetGprsActiveTypeReq.stAtdPara         = *pstAtdPara;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ,
                                &stGetGprsActiveTypeReq,
                                sizeof(TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_PppDialOrig(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid,
    TAF_PPP_REQ_CONFIG_INFO_STRU       *pstPppReqConfigInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    TAF_PS_PPP_DIAL_ORIG_REQ_STRU       stPppDialOrigReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stPppDialOrigReq, sizeof(stPppDialOrigReq), 0x00, sizeof(TAF_PS_PPP_DIAL_ORIG_REQ_STRU));

    /* ����ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ��Ϣ */
    stPppDialOrigReq.stCtrl.ulModuleId          = ulModuleId;
    stPppDialOrigReq.stCtrl.usClientId          = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stPppDialOrigReq.stCtrl.ucOpId              = ucOpId;

    stPppDialOrigReq.stPppDialParaInfo.ucCid    = ucCid;

    stPppDialOrigReq.stPppDialParaInfo.bitOpPppConfigInfo = VOS_TRUE;
    lMemResult = memcpy_s(&(stPppDialOrigReq.stPppDialParaInfo.stPppReqConfigInfo),
                          sizeof(stPppDialOrigReq.stPppDialParaInfo.stPppReqConfigInfo),
                          pstPppReqConfigInfo,
                          sizeof(TAF_PPP_REQ_CONFIG_INFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stPppDialOrigReq.stPppDialParaInfo.stPppReqConfigInfo), sizeof(TAF_PPP_REQ_CONFIG_INFO_STRU));

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ,
                                &stPppDialOrigReq,
                                sizeof(TAF_PS_PPP_DIAL_ORIG_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicDnsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                          ulResult;

    TAF_PS_GET_NEGOTIATION_DNS_REQ_STRU stNegoDns;

    /* ��ʼ�� */
    memset_s(&stNegoDns, sizeof(stNegoDns), 0x00, sizeof(TAF_PS_GET_NEGOTIATION_DNS_REQ_STRU));

    /* �����Ϣ�ṹ */
    stNegoDns.stCtrl.ulModuleId = ulModuleId;
    stNegoDns.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stNegoDns.stCtrl.ucOpId     = ucOpId;
    stNegoDns.ucCid             = ucCid;

    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_NEGOTIATION_DNS_REQ,
                                &stNegoDns,
                                sizeof(TAF_PS_GET_NEGOTIATION_DNS_REQ_STRU));

    return ulResult;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_UINT32 TAF_PS_SetCtaInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucTimeLen
)
{
    TAF_PS_SET_CTA_INFO_REQ_STRU        stSetPktCdataInactivityTmrlenReq;
    VOS_UINT32                          ulResult;

    memset_s(&stSetPktCdataInactivityTmrlenReq, sizeof(TAF_PS_SET_CTA_INFO_REQ_STRU), 0x00, sizeof(TAF_PS_SET_CTA_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_DSFLOW_WRITE_NV_CFG_REQ��Ϣ */
    stSetPktCdataInactivityTmrlenReq.stCtrl.ulModuleId  = ulModuleId;
    stSetPktCdataInactivityTmrlenReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPktCdataInactivityTmrlenReq.stCtrl.ucOpId      = ucOpId;


    stSetPktCdataInactivityTmrlenReq.ucPktCdataInactivityTmrLen = ucTimeLen;


    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_CTA_INFO_REQ,
                                &stSetPktCdataInactivityTmrlenReq,
                                sizeof(stSetPktCdataInactivityTmrlenReq));

    return ulResult;
}

VOS_UINT32 TAF_PS_GetCtaInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_CTA_INFO_REQ_STRU        stGetPktCdataInactivityTmrlenReq;
    VOS_UINT32                          ulResult;

    memset_s(&stGetPktCdataInactivityTmrlenReq, sizeof(TAF_PS_GET_CTA_INFO_REQ_STRU), 0x00, sizeof(TAF_PS_GET_CTA_INFO_REQ_STRU));

    stGetPktCdataInactivityTmrlenReq.stCtrl.ulModuleId  = ulModuleId;
    stGetPktCdataInactivityTmrlenReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPktCdataInactivityTmrlenReq.stCtrl.ucOpId      = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_CTA_INFO_REQ,
                                &stGetPktCdataInactivityTmrlenReq,
                                sizeof(stGetPktCdataInactivityTmrlenReq));

    return ulResult;
}



#endif

#if (FEATURE_ON == FEATURE_IMS)

VOS_UINT32 TAF_PS_SetImsPdpCfg(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_IMS_PDP_CFG_STRU               *pstImsPdpCfg
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_IMS_PDP_CFG_REQ_STRU     stSetImsPdpCfgReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stSetImsPdpCfgReq, sizeof(TAF_PS_SET_IMS_PDP_CFG_REQ_STRU), 0x00, sizeof(TAF_PS_SET_IMS_PDP_CFG_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_IMS_PDP_CFG_REQ��Ϣ */
    stSetImsPdpCfgReq.stCtrl.ulModuleId = ulModuleId;
    stSetImsPdpCfgReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetImsPdpCfgReq.stCtrl.ucOpId     = ucOpId;
    stSetImsPdpCfgReq.stImsPdpCfg       = *pstImsPdpCfg;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_IMS_PDP_CFG_REQ,
                                &stSetImsPdpCfgReq,
                                sizeof(TAF_PS_SET_IMS_PDP_CFG_REQ_STRU));

    return ulResult;
}
#endif

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_UINT32 TAF_PS_SetCdmaDormantTimer(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucDormantTimer
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_1X_DORM_TIMER_REQ_STRU   st1xDormTimerReq;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&st1xDormTimerReq, sizeof(TAF_PS_SET_1X_DORM_TIMER_REQ_STRU), 0x00, sizeof(TAF_PS_SET_1X_DORM_TIMER_REQ_STRU));

    /* ����ID_MSG_TAF_PS_SET_1X_DORMANT_TIMER_REQ��Ϣ */
    st1xDormTimerReq.stCtrl.ulModuleId = ulModuleId;
    st1xDormTimerReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    st1xDormTimerReq.stCtrl.ucOpId     = ucOpId;
    st1xDormTimerReq.ucDormantTimer    = ucDormantTimer;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_1X_DORM_TIMER_REQ,
                                &st1xDormTimerReq,
                                sizeof(TAF_PS_SET_1X_DORM_TIMER_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ProcCdmaDormTimerQryReq(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_1X_DORM_TIMER_REQ_STRU   stGet1xDormTimerReq;
    VOS_UINT32                          ulResult;

    memset_s(&stGet1xDormTimerReq, sizeof(TAF_PS_GET_1X_DORM_TIMER_REQ_STRU), 0x00, sizeof(TAF_PS_GET_1X_DORM_TIMER_REQ_STRU));

    stGet1xDormTimerReq.stCtrl.ulModuleId = ulModuleId;
    stGet1xDormTimerReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGet1xDormTimerReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_1X_DORM_TIEMR_REQ,
                                &stGet1xDormTimerReq,
                                sizeof(stGet1xDormTimerReq));

    return ulResult;
}




VOS_UINT32 TAF_PS_SetMipMode(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_MIP_MODE_ENUM_UINT8          enMipMode
)
{
    TAF_PS_SET_MIP_MODE_REQ_STRU        stSetMipModeReq;
    VOS_UINT32                          ulResult;

    memset_s(&stSetMipModeReq, sizeof(TAF_PS_SET_MIP_MODE_REQ_STRU), 0x00, sizeof(TAF_PS_SET_MIP_MODE_REQ_STRU));

    stSetMipModeReq.stCtrl.ulModuleId   = ulModuleId;
    stSetMipModeReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetMipModeReq.stCtrl.ucOpId       = ucOpId;
    stSetMipModeReq.enMipMode           = enMipMode;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_MIP_MODE_REQ,
                                &stSetMipModeReq,
                                sizeof(stSetMipModeReq));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetMipMode(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_MIP_MODE_REQ_STRU        stGetMipModeReq;
    VOS_UINT32                          ulResult;

    memset_s(&stGetMipModeReq, sizeof(TAF_PS_GET_MIP_MODE_REQ_STRU), 0x00, sizeof(TAF_PS_GET_MIP_MODE_REQ_STRU));

    stGetMipModeReq.stCtrl.ulModuleId   = ulModuleId;
    stGetMipModeReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetMipModeReq.stCtrl.ucOpId       = ucOpId;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_MIP_MODE_REQ,
                                &stGetMipModeReq,
                                sizeof(stGetMipModeReq));

    return ulResult;
}

#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

VOS_UINT32 TAF_PS_GetModuleIdByCid(
    VOS_UINT8                           ucCid,
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    VOS_UINT8                           ucBearerInst    = 0;
    VOS_UINT8                           ucPdnInst       = 0;

    if (VOS_FALSE == TAF_DSM_GetPdnInstAndBearerInstByCid(ucCid, VOS_FALSE, &ucPdnInst, &ucBearerInst))
    {
        return 0;
    }

    if (VOS_TRUE == TAF_DSM_GetDsmCtxAddrByModemId(enModemId)->stDsmConnCtx.pastPdnEntityCtx[ucPdnInst]->stPdnClientType.bitImsaClient)
    {
        return PS_PID_IMSA;
    }

    if (VOS_TRUE == TAF_DSM_GetDsmCtxAddrByModemId(enModemId)->stDsmConnCtx.pastPdnEntityCtx[ucPdnInst]->stPdnClientType.bitAppClient)
    {
        return WUEPS_PID_AT;
    }

    return 0;

}


VOS_UINT8 TAF_PS_FindCidForDial(VOS_UINT32  ulAppPid)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(WUEPS_PID_TAF);

    return TAF_DSM_FindCidForDialByModemId(enModemId, ulAppPid);
}

#endif


MODULE_EXPORTED VOS_UINT32 TAF_PS_GetCidSdfParaInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_SDF_PARA_QUERY_INFO_STRU       *pstSdfQueryInfo
)
{
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    VOS_UINT8                           ucNum;
    VOS_UINT16                          usModemId;
#endif
    VOS_UINT32                          ulResult;
    TAF_PS_SDF_INFO_REQ_STRU            stSdfInfoReq;

    ulResult  = VOS_ERR;
    memset_s(&stSdfInfoReq, sizeof(stSdfInfoReq), 0x00, sizeof(TAF_PS_SDF_INFO_REQ_STRU));

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    usModemId = TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId);
#endif

    if (VOS_NULL_PTR == pstSdfQueryInfo)
    {
        /* Ŀǰ�ݲ�֧���첽��ʽ */
        return VOS_ERR;
    }

    memset_s(pstSdfQueryInfo, sizeof(TAF_SDF_PARA_QUERY_INFO_STRU), 0x00, sizeof(TAF_SDF_PARA_QUERY_INFO_STRU));

/* ͬ����ʽĿǰ��֧��C�� */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    for (ucNum = 0; ucNum <= TAF_MAX_CID_NV; ucNum++)
    {
        if (VOS_OK == TAF_DSM_GetSdfParaInfoByModemId(usModemId, ucNum,
                              &(pstSdfQueryInfo->astSdfPara[pstSdfQueryInfo->ulSdfNum])))
        {
            pstSdfQueryInfo->ulSdfNum ++;
        }
    }
    ulResult = VOS_OK;

    /* ͬ����Ϣ���� */
    TAF_DSM_TraceSyncMsgByModemId(usModemId,
                                  ID_MSG_TAF_PS_GET_CID_SDF_REQ,
                                  (VOS_UINT8 *)pstSdfQueryInfo,
                                  sizeof(TAF_SDF_PARA_QUERY_INFO_STRU));
#endif

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_GetUnusedCid(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                          *puCid
)
{
    VOS_UINT32                          ulResult;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId);

    if (enModemId >= MODEM_ID_BUTT)
    {
        PS_PRINTF_WARNING("<TAF_PS_GetUnusedCid> ModemId is error!");

        return VOS_ERR;
    }

    ulResult = VOS_OK;

    if (VOS_NULL_PTR == puCid)
    {
        /* Ŀǰ�ݲ�֧���첽��ʽ */
        return VOS_ERR;
    }

/* ͬ����ʽĿǰ��֧��C�� */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    /* ���ҿ����ڲ��ŵ�CID */
    *puCid = TAF_DSM_FindCidForDialByModemId(enModemId, ulModuleId);

    if ( TAF_INVALID_CID == *puCid )
    {
        ulResult = VOS_ERR;
    }

    /* ͬ����Ϣ���� */
    TAF_DSM_TraceSyncMsgByModemId(enModemId, ID_MSG_TAF_PS_GET_UNUSED_CID_REQ,
                                  puCid,
                                  sizeof(VOS_UINT8));
#endif

    return ulResult;
}


VOS_UINT32 TAF_PS_SetVzwApneInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucIsCustomCmd,
    TAF_PS_VZWAPNE_INFO_STRU           *pstVzwapneInfo
)
{
    TAF_PS_SET_VZWAPNE_REQ_STRU         stSetVzwApneReq;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;

    memset_s(&stSetVzwApneReq, sizeof(stSetVzwApneReq), 0x00, sizeof(stSetVzwApneReq));

    stSetVzwApneReq.stCtrl.ulModuleId   = ulModuleId;
    stSetVzwApneReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetVzwApneReq.stCtrl.ucOpId       = ucOpId;
    stSetVzwApneReq.ucIsCustomCmd       = ucIsCustomCmd;

    lMemResult = memcpy_s(&stSetVzwApneReq.stVzwApneInfo,
                          sizeof(TAF_PS_VZWAPNE_INFO_STRU),
                          pstVzwapneInfo,
                          sizeof(TAF_PS_VZWAPNE_INFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_PS_VZWAPNE_INFO_STRU), sizeof(TAF_PS_VZWAPNE_INFO_STRU));

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_VZWAPNE_REQ,
                                &stSetVzwApneReq,
                                sizeof(stSetVzwApneReq));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetVzwApneInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucIsCustomCmd
)
{
    TAF_PS_GET_VZWAPNE_REQ_STRU         stGetVzwApneReq;
    VOS_UINT32                          ulResult;

    memset_s(&stGetVzwApneReq, sizeof(stGetVzwApneReq), 0x00, sizeof(stGetVzwApneReq));

    stGetVzwApneReq.stCtrl.ulModuleId   = ulModuleId;
    stGetVzwApneReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetVzwApneReq.stCtrl.ucOpId       = ucOpId;
    stGetVzwApneReq.ucIsCustomCmd       = ucIsCustomCmd;

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_VZWAPNE_REQ,
                                &stGetVzwApneReq,
                                sizeof(stGetVzwApneReq));

    return ulResult;
}



VOS_UINT32 TAF_PS_SetApnThrotInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_APN_THROT_INFO_STRU         *pstApnThrotInfo
)
{
    TAF_PS_SET_APN_THROT_INFO_REQ_STRU  stSetApnThrotInfoReq;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;

    memset_s(&stSetApnThrotInfoReq, sizeof(stSetApnThrotInfoReq), 0x00, sizeof(TAF_PS_SET_APN_THROT_INFO_REQ_STRU));

    /* ��д��Ϣ���� */
    stSetApnThrotInfoReq.stCtrl.ulModuleId   = ulModuleId;
    stSetApnThrotInfoReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetApnThrotInfoReq.stCtrl.ucOpId       = ucOpId;

    lMemResult = memcpy_s(&(stSetApnThrotInfoReq.stApnThrotInfo),
                          sizeof(TAF_PS_APN_THROT_INFO_STRU),
                          pstApnThrotInfo,
                          sizeof(TAF_PS_APN_THROT_INFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_PS_APN_THROT_INFO_STRU), sizeof(TAF_PS_APN_THROT_INFO_STRU));

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_APN_THROT_INFO_REQ,
                                &stSetApnThrotInfoReq,
                                sizeof(stSetApnThrotInfoReq));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetLteAttchInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_LTE_ATTACH_INFO_REQ_STRU stGetLteAttchInfoReq;

    /* ��ʼ�� */
    memset_s(&stGetLteAttchInfoReq, sizeof(stGetLteAttchInfoReq), 0x00, sizeof(TAF_PS_GET_LTE_ATTACH_INFO_REQ_STRU));

    /* ����ID_MSG_TAF_PS_GET_LTE_ATTACH_ESM_INFO_REQ��Ϣ */
    stGetLteAttchInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetLteAttchInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetLteAttchInfoReq.stCtrl.ucOpId     = ucOpId;

    /* ������Ϣ */
    return TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                            ID_MSG_TAF_PS_GET_LTE_ATTACH_INFO_REQ,
                            &stGetLteAttchInfoReq,
                            sizeof(TAF_PS_GET_LTE_ATTACH_INFO_REQ_STRU));

}



VOS_UINT32 TAF_PS_EpdgCtrlMsg(
    const TAF_CTRL_STRU                    *pstCtrl,
    const TAF_PS_EPDG_CTRL_STRU            *pstEpdgCtrl
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    TAF_PS_EPDG_CTRL_NTF_STRU           stEpdgCtrlNtf;

    /* ��ʼ�� */
    ulResult = VOS_OK;
    memset_s(&stEpdgCtrlNtf, sizeof(stEpdgCtrlNtf), 0x00, sizeof(TAF_PS_EPDG_CTRL_NTF_STRU));

    /* ����ID_MSG_TAF_IFACE_UP_REQ��Ϣ */
    stEpdgCtrlNtf.stCtrl.ulModuleId = pstCtrl->ulModuleId;
    stEpdgCtrlNtf.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(pstCtrl->usClientId);
    stEpdgCtrlNtf.stCtrl.ucOpId     = pstCtrl->ucOpId;

    lMemResult = memcpy_s(&stEpdgCtrlNtf.stEpdgCtrl, sizeof(stEpdgCtrlNtf.stEpdgCtrl), pstEpdgCtrl, sizeof(TAF_PS_EPDG_CTRL_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stEpdgCtrlNtf.stEpdgCtrl), sizeof(TAF_PS_EPDG_CTRL_STRU));

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(pstCtrl->usClientId),
                                ID_MSG_TAF_PS_EPDG_CTRL_NTF,
                                &stEpdgCtrlNtf,
                                sizeof(TAF_PS_EPDG_CTRL_NTF_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetRoamPdpType(
    VOS_UINT32                                              ulModuleId,
    VOS_UINT16                                              usExClientId,
    VOS_UINT8                                               ucOpId,
    const TAF_PS_ROAMING_PDP_TYPE_INFO_STRU                *pstPdpTypeInfo
)
{
    TAF_PS_SET_ROAMING_PDP_TYPE_REQ_STRU                    stSetPdpType;
    VOS_UINT32                                              ulResult;
    errno_t                                                 lMemResult;

    memset_s(&stSetPdpType, sizeof(stSetPdpType), 0x00, sizeof(TAF_PS_SET_ROAMING_PDP_TYPE_REQ_STRU));

    /* ��д��Ϣ���� */
    stSetPdpType.stCtrl.ulModuleId   = ulModuleId;
    stSetPdpType.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpType.stCtrl.ucOpId       = ucOpId;

    lMemResult = memcpy_s(&(stSetPdpType.stPdpTypeInfo),
                          sizeof(TAF_PS_ROAMING_PDP_TYPE_INFO_STRU),
                          pstPdpTypeInfo,
                          sizeof(TAF_PS_ROAMING_PDP_TYPE_INFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_PS_ROAMING_PDP_TYPE_INFO_STRU), sizeof(TAF_PS_ROAMING_PDP_TYPE_INFO_STRU));

    /* ������Ϣ */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_ROAMING_PDP_TYPE_REQ,
                                &stSetPdpType,
                                sizeof(TAF_PS_SET_ROAMING_PDP_TYPE_REQ_STRU));

    return ulResult;
}

