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
#include "ATCmdProc.h"
#include "product_config.h"
#include "AtMsgPrint.h"
#include "AtCheckFunc.h"
#include "AtCmdSmsProc.h"
#include "securec.h"



#define THIS_FILE_ID        PS_FILE_ID_AT_MSG_PRINT_C

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*****************************************************************************
  3���Ͷ���
*****************************************************************************/


/*****************************************************************************
  5 ����ʵ��
*****************************************************************************/

TAF_UINT32 AT_StubSendAutoReplyMsg(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo
)
{
    MN_MSG_SEND_PARM_STRU               stSendMsg;
    MN_MSG_TS_DATA_INFO_STRU            *pstTsSubmitInfo = VOS_NULL_PTR;
    MN_MSG_SUBMIT_STRU                  *pstSubmit = VOS_NULL_PTR;
    TAF_UINT32                          ulRet;
    errno_t                             lMemResult;

    /*1. Ϊ�Զ��ظ���ϢSUBMIT�����ڴ沢���*/
    pstTsSubmitInfo = (MN_MSG_TS_DATA_INFO_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(MN_MSG_TS_DATA_INFO_STRU));
    if (pstTsSubmitInfo == VOS_NULL_PTR)
    {
        return AT_ERROR;
    }
    memset_s(pstTsSubmitInfo, sizeof(MN_MSG_TS_DATA_INFO_STRU), 0x00, sizeof(MN_MSG_TS_DATA_INFO_STRU));

    /*2. Ϊ�Զ��ظ���ϢSUBMIT��дTPDU��������*/
    pstTsSubmitInfo->enTpduType = MN_MSG_TPDU_SUBMIT;
    pstSubmit = (MN_MSG_SUBMIT_STRU *)&pstTsSubmitInfo->u.stSubmit;
    lMemResult = memcpy_s(&pstSubmit->stDestAddr,
                          sizeof(pstSubmit->stDestAddr),
                          &pstTsDataInfo->u.stDeliver.stOrigAddr,
                          sizeof(pstSubmit->stDestAddr));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSubmit->stDestAddr), sizeof(pstSubmit->stDestAddr));
    lMemResult = memcpy_s(&pstSubmit->stDcs,
                          sizeof(pstSubmit->stDcs),
                          &pstTsDataInfo->u.stDeliver.stDcs,
                          sizeof(pstSubmit->stDcs));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSubmit->stDcs), sizeof(pstSubmit->stDcs));
    pstSubmit->stValidPeriod.enValidPeriod = MN_MSG_VALID_PERIOD_NONE;

    /*3. Ϊ�Զ��ظ���ϢSUBMIT����*/
    ulRet = MN_MSG_Encode(pstTsSubmitInfo, &stSendMsg.stMsgInfo.stTsRawData);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        /*lint -save -e830 */
        PS_MEM_FREE(WUEPS_PID_AT, pstTsSubmitInfo);
        /*lint -restore */
        return AT_ERROR;
    }

    /*4. ��д�ظ���Ϣ�Ķ�������, �洢�豸����Ϣ���ͺͷ�����*/
    stSendMsg.enMemStore                        = MN_MSG_MEM_STORE_NONE;
    stSendMsg.enClientType                      = MN_MSG_CLIENT_NORMAL;
    stSendMsg.stMsgInfo.stTsRawData.enTpduType  = MN_MSG_TPDU_SUBMIT;
    lMemResult = memcpy_s(&stSendMsg.stMsgInfo.stScAddr,
                          sizeof(stSendMsg.stMsgInfo.stScAddr),
                          &pstEvent->u.stDeliverInfo.stRcvMsgInfo.stScAddr,
                          sizeof(stSendMsg.stMsgInfo.stScAddr));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stSendMsg.stMsgInfo.stScAddr), sizeof(stSendMsg.stMsgInfo.stScAddr));

    /*5. ���ͻظ���Ϣ*/
    ulRet = MN_MSG_Send(ucIndex, 0, &stSendMsg);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        ulRet = AT_ERROR;
    }
    else
    {
        ulRet = AT_OK;
    }
    PS_MEM_FREE(WUEPS_PID_AT, pstTsSubmitInfo);

    return ulRet;
}


TAF_VOID AT_StubClearSpecificAutoRelyMsg(
    VOS_UINT8                           ucClientIndex,
    TAF_UINT32                          ulBufferIndex
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucClientIndex);

    if (pstSmsCtx->astSmsMtBuffer[ulBufferIndex].pstEvent != VOS_NULL_PTR)
    {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, pstSmsCtx->astSmsMtBuffer[ulBufferIndex].pstEvent);
        /*lint -restore */
    }

    if (pstSmsCtx->astSmsMtBuffer[ulBufferIndex].pstTsDataInfo != VOS_NULL_PTR)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pstSmsCtx->astSmsMtBuffer[ulBufferIndex].pstTsDataInfo);
    }

    return;
}


TAF_VOID AT_StubTriggerAutoReply(
    VOS_UINT8                           ucIndex,
    TAF_UINT8                           ucCfgValue
)
{
    TAF_UINT8                           ucLoop;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    pstSmsCtx->ucSmsAutoReply = ucCfgValue;

    /*���ر��Զ��ظ����ܣ��������ض�̬�ڴ�*/
    if (pstSmsCtx->ucSmsAutoReply == 0)
    {
        for (ucLoop = 0; ucLoop < AT_SMSMT_BUFFER_MAX; ucLoop++)
        {
            AT_StubClearSpecificAutoRelyMsg(ucIndex, ucLoop);
            pstSmsCtx->astSmsMtBuffer[ucLoop].bUsed = TAF_FALSE;
        }

        return;
    }

    /*���������Զ��ظ����ܣ���˳��ظ����յ��Ķ���*/
    for (ucLoop = 0; ucLoop < AT_SMSMT_BUFFER_MAX; ucLoop++)
    {
        if (pstSmsCtx->astSmsMtBuffer[ucLoop].bUsed != TAF_TRUE)
        {
            AT_StubClearSpecificAutoRelyMsg(ucIndex, ucLoop);
            continue;
        }

        if ((pstSmsCtx->astSmsMtBuffer[ucLoop].pstEvent == VOS_NULL_PTR)
         || (pstSmsCtx->astSmsMtBuffer[ucLoop].pstTsDataInfo == VOS_NULL_PTR))
        {

            AT_StubClearSpecificAutoRelyMsg(ucIndex, ucLoop);
            pstSmsCtx->astSmsMtBuffer[ucLoop].bUsed = TAF_FALSE;
            continue;
        }

        AT_StubSendAutoReplyMsg(ucIndex,
                                pstSmsCtx->astSmsMtBuffer[ucLoop].pstEvent,
                                pstSmsCtx->astSmsMtBuffer[ucLoop].pstTsDataInfo);
        AT_StubClearSpecificAutoRelyMsg(ucIndex, ucLoop);
        pstSmsCtx->astSmsMtBuffer[ucLoop].bUsed = TAF_FALSE;
        break;
    }

    return;

}


TAF_VOID AT_StubSaveAutoReplyData(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent,
    MN_MSG_TS_DATA_INFO_STRU           *pstTsDataInfo
)
{
    errno_t                             lMemResult;
    TAF_UINT8                           ucLoop;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    /*�Զ��ظ�����δ����ֱ�ӷ���;*/
    if (pstSmsCtx->ucSmsAutoReply == 0)
    {
        return;
    }

    /*������Ϣ����DELIVER���Ż�TP-RPû����λֱ�ӷ���*/
    if ((pstTsDataInfo->enTpduType != MN_MSG_TPDU_DELIVER)
     || (pstTsDataInfo->u.stDeliver.bReplayPath != VOS_TRUE))
    {
        return;
    }

    /*���벢�����Զ��ظ���ز���������*/
    for (ucLoop = 0; ucLoop < AT_SMSMT_BUFFER_MAX; ucLoop++)
    {
        if (pstSmsCtx->astSmsMtBuffer[ucLoop].bUsed == TAF_TRUE)
        {
            continue;
        }

        AT_StubClearSpecificAutoRelyMsg(ucIndex, ucLoop);

        /*��¼���ն�����Ϣ��¼���ڴ棬���� GCF��������34��2��8*/
        pstSmsCtx->astSmsMtBuffer[ucLoop].pstEvent = (MN_MSG_EVENT_INFO_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                                                  sizeof(MN_MSG_EVENT_INFO_STRU));
        if (pstSmsCtx->astSmsMtBuffer[ucLoop].pstEvent == VOS_NULL_PTR)
        {
            AT_WARN_LOG("At_SmsDeliverProc: Fail to alloc memory.");
            return;
        }

        pstSmsCtx->astSmsMtBuffer[ucLoop].pstTsDataInfo = (MN_MSG_TS_DATA_INFO_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                                                     sizeof(MN_MSG_TS_DATA_INFO_STRU));
        if (pstSmsCtx->astSmsMtBuffer[ucLoop].pstTsDataInfo == VOS_NULL_PTR)
        {
            /*lint -save -e516 */
            PS_MEM_FREE(WUEPS_PID_AT, pstSmsCtx->astSmsMtBuffer[ucLoop].pstEvent);
            /*lint -restore */
            AT_WARN_LOG("At_SmsDeliverProc: Fail to alloc memory.");
            return;
        }

        lMemResult = memcpy_s(pstSmsCtx->astSmsMtBuffer[ucLoop].pstEvent, sizeof(MN_MSG_EVENT_INFO_STRU), pstEvent, sizeof(MN_MSG_EVENT_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(MN_MSG_EVENT_INFO_STRU), sizeof(MN_MSG_EVENT_INFO_STRU));
        lMemResult = memcpy_s(pstSmsCtx->astSmsMtBuffer[ucLoop].pstTsDataInfo,
                              sizeof(MN_MSG_TS_DATA_INFO_STRU),
                              pstTsDataInfo,
                              sizeof(MN_MSG_TS_DATA_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(MN_MSG_TS_DATA_INFO_STRU), sizeof(MN_MSG_TS_DATA_INFO_STRU));

        pstSmsCtx->astSmsMtBuffer[ucLoop].bUsed = TAF_TRUE;

        break;
    }

    return;
}



VOS_UINT32 At_ParseCsmpFo(
    VOS_UINT8                           *pucFo
)
{
    TAF_UINT32                          ulRet;

    /* ���<fo>,�������� */
    ulRet = At_CheckNumString(gastAtParaList[0].aucPara,gastAtParaList[0].usParaLen);
    if (ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* ע��: gastAtParaList[0].ulParaValue��ʱ��δת����������������������� */
    ulRet = At_Auc2ul(gastAtParaList[0].aucPara,gastAtParaList[0].usParaLen,
                      &gastAtParaList[0].ulParaValue);
    if (ulRet == AT_FAILURE)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* ���<fo>,һ���ֽ� */
    if (gastAtParaList[0].ulParaValue > 0xff)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (((0x03 & gastAtParaList[0].ulParaValue) != 0x01)
     && ((0x03 & gastAtParaList[0].ulParaValue) != 0x02))
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    *pucFo = (TAF_UINT8)gastAtParaList[0].ulParaValue;

    return AT_SUCCESS;
}


TAF_UINT32 At_GetAbsoluteTime(
    TAF_UINT8                           *pucTimeStr,
    TAF_UINT16                          usTimeStrLen,
    MN_MSG_TIMESTAMP_STRU               *pstAbsoluteTime
)
{
    TAF_UINT32                          ulRet;
    TAF_UINT8                           aucBcdNum[2];
    TAF_UINT32                          ulTmp               = 0;


    aucBcdNum[0] = 0;
    aucBcdNum[1] = 0;


    /* 6th of May 1994, 22:10:00 GMT+2 "94/05/06,22:10:00+08"
       ע��:��Ҫ�ж��м��ַ��Ƿ�Ϸ�
    */
    if ((pucTimeStr == TAF_NULL_PTR)
     || (pstAbsoluteTime == TAF_NULL_PTR))
    {
        AT_WARN_LOG("At_GetAbsoluteTime: parameter is NULL.");
        return AT_ERROR;
    }

    /* ���<vp>,�ַ������� */
    if ((usTimeStrLen != 22)
     || (pucTimeStr[0] != '"')
     || (pucTimeStr[usTimeStrLen - 1] != '"')/* '"' */
     || (pucTimeStr[3] != '/')/* '/' */
     || (pucTimeStr[6] != '/')/* '/' */
     || (pucTimeStr[9] != ',')/* ',' */
     || (pucTimeStr[12] != ':')/* ':' */
     || (pucTimeStr[15] != ':'))/* ':' */
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* Year */
    ulRet = At_AsciiNum2BcdNum(aucBcdNum, &pucTimeStr[1], 2);
    if (ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    pstAbsoluteTime->ucYear = AT_BCD_REVERSE(aucBcdNum[0]);


    /* Month */
    ulRet = At_AsciiNum2BcdNum(aucBcdNum, &pucTimeStr[4], 2);
    if (ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    pstAbsoluteTime->ucMonth = AT_BCD_REVERSE(aucBcdNum[0]);

    /* Day */
    ulRet = At_AsciiNum2BcdNum(aucBcdNum, &pucTimeStr[7], 2);
    if(ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    pstAbsoluteTime->ucDay = AT_BCD_REVERSE(aucBcdNum[0]);

    /* Hour */
    ulRet = At_AsciiNum2BcdNum(aucBcdNum, &pucTimeStr[10], 2);
    if(ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    pstAbsoluteTime->ucHour = AT_BCD_REVERSE(aucBcdNum[0]);

    /* Minute */
    ulRet = At_AsciiNum2BcdNum(aucBcdNum, &pucTimeStr[13], 2);
    if(ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    pstAbsoluteTime->ucMinute = AT_BCD_REVERSE(aucBcdNum[0]);

    /* Second */
    ulRet = At_AsciiNum2BcdNum(aucBcdNum, &pucTimeStr[16], 2);
    if(ulRet != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    pstAbsoluteTime->ucSecond = AT_BCD_REVERSE(aucBcdNum[0]);

    if(At_Auc2ul(&pucTimeStr[19],2,&ulTmp) == AT_FAILURE)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (((ulTmp % 4) != 0)
     || (ulTmp > AT_MAX_TIMEZONE_VALUE))
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* '+' ���� '-' */
    switch(pucTimeStr[18])
    {
    case '+':
        pstAbsoluteTime->cTimezone = (TAF_INT8)ulTmp;
        break;

    case '-':
        pstAbsoluteTime->cTimezone = (TAF_INT8)((-1) * ulTmp);
        break;

    default:
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    return AT_SUCCESS;

}


VOS_UINT32  AT_SetAbsoluteValidPeriod(
    VOS_UINT8                           ucIndex,
    TAF_UINT8                          *pucPara,
    TAF_UINT16                          usParaLen,
    MN_MSG_VALID_PERIOD_STRU           *pstValidPeriod
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulRet;
    TAF_UINT8                           ucDateInvalidType;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (usParaLen == 0)
    {
        if (pstSmsCtx->stCscaCsmpInfo.stVp.enValidPeriod == MN_MSG_VALID_PERIOD_ABSOLUTE)
        {
            lMemResult = memcpy_s(pstValidPeriod, sizeof(MN_MSG_VALID_PERIOD_STRU), &(pstSmsCtx->stCscaCsmpInfo.stVp), sizeof(MN_MSG_VALID_PERIOD_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(MN_MSG_VALID_PERIOD_STRU), sizeof(MN_MSG_VALID_PERIOD_STRU));
            return AT_SUCCESS;
        }
        else
        {
            return AT_ERROR;
        }
    }
    else
    {
        ulRet = At_GetAbsoluteTime(pucPara,
                                   usParaLen,
                                   &pstValidPeriod->u.stAbsoluteTime);
        if (ulRet != AT_SUCCESS)
        {
            return ulRet;
        }

        ulRet = MN_MSG_ChkDate(&pstValidPeriod->u.stAbsoluteTime, &ucDateInvalidType);
        if (ulRet != MN_ERR_NO_ERROR)
        {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        return AT_SUCCESS;
    }
}


VOS_UINT32  AT_SetRelativeValidPeriod(
    VOS_UINT8                           ucIndex,
    TAF_UINT8                          *pucPara,
    TAF_UINT16                          usParaLen,
    MN_MSG_VALID_PERIOD_STRU           *pstValidPeriod
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulRet;
    TAF_UINT32                          ulRelativeValidPeriod;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (usParaLen == 0)
    {
        if (pstSmsCtx->stCscaCsmpInfo.stVp.enValidPeriod == MN_MSG_VALID_PERIOD_RELATIVE)
        {
            lMemResult = memcpy_s(pstValidPeriod, sizeof(MN_MSG_VALID_PERIOD_STRU), &(pstSmsCtx->stCscaCsmpInfo.stVp), sizeof(MN_MSG_VALID_PERIOD_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(MN_MSG_VALID_PERIOD_STRU), sizeof(MN_MSG_VALID_PERIOD_STRU));
            return AT_SUCCESS;
        }
        else if (pstSmsCtx->stCscaCsmpInfo.stVp.enValidPeriod == MN_MSG_VALID_PERIOD_NONE)
        {
            pstValidPeriod->u.ucOtherTime = 167;
            return AT_SUCCESS;
        }
        else
        {
            return AT_ERROR;
        }
    }
    else
    {
        /* ���<vp>,�������� */
        ulRet = At_CheckNumString(pucPara, usParaLen);
        if (ulRet != AT_SUCCESS)
        {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        /* ע��: gastAtParaList[1].ulParaValue��ʱ��δת����������������������� */
        ulRet = At_Auc2ul(pucPara, usParaLen, &ulRelativeValidPeriod);
        if (ulRet == AT_FAILURE)
        {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (ulRelativeValidPeriod > 255)
        {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        pstValidPeriod->u.ucOtherTime = (TAF_UINT8)ulRelativeValidPeriod;
        return AT_SUCCESS;
    }
}


VOS_UINT32 At_ParseCsmpVp(
    VOS_UINT8                           ucIndex,
    MN_MSG_VALID_PERIOD_STRU           *pstVp
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulRet;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    /*��ȡ��ǰ���õ�TP-VPFֵ�����û�������TP-VPF��TP-VP������Ƶ�ǰ�ṹ����ʱ�ṹ���Ƴ�*/
    if (gastAtParaList[0].usParaLen != 0)
    {
        AT_GET_MSG_TP_VPF(pstVp->enValidPeriod, pstSmsCtx->stCscaCsmpInfo.ucTmpFo);
    }
    else if (gastAtParaList[1].usParaLen != 0)
    {
        pstVp->enValidPeriod = pstSmsCtx->stCscaCsmpInfo.stVp.enValidPeriod;
    }
    else
    {
        lMemResult = memcpy_s(pstVp, sizeof(MN_MSG_VALID_PERIOD_STRU), &(pstSmsCtx->stCscaCsmpInfo.stVp), sizeof(MN_MSG_VALID_PERIOD_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(MN_MSG_VALID_PERIOD_STRU), sizeof(MN_MSG_VALID_PERIOD_STRU));
        return AT_SUCCESS;
    }

    /*������Ч����������Ϊ��Ч��<VP>���������Ϊ��*/
    if (pstVp->enValidPeriod == MN_MSG_VALID_PERIOD_NONE)
    {
        if (gastAtParaList[1].usParaLen != 0)
        {
            return AT_ERROR;
        }
        memset_s(pstVp, sizeof(MN_MSG_VALID_PERIOD_STRU), 0x00, sizeof(MN_MSG_VALID_PERIOD_STRU));
        return AT_SUCCESS;
    }
    /*������Ч����������Ϊ�����Ч�ڣ�*/
    else if (pstVp->enValidPeriod == MN_MSG_VALID_PERIOD_RELATIVE)
    {
        ulRet = AT_SetRelativeValidPeriod(ucIndex,
                                          gastAtParaList[1].aucPara,
                                          gastAtParaList[1].usParaLen,
                                          pstVp);
        return ulRet;
    }
    else if(pstVp->enValidPeriod == MN_MSG_VALID_PERIOD_ABSOLUTE)
    {
        ulRet = AT_SetAbsoluteValidPeriod(ucIndex,
                                          gastAtParaList[1].aucPara,
                                          gastAtParaList[1].usParaLen,
                                          pstVp);
        return ulRet;
    }
    else
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

}


TAF_VOID At_MsgResultCodeFormat(
    TAF_UINT8                           ucIndex,
    TAF_UINT16                          usLength
)
{
    errno_t                             lMemResult;
    if(gucAtVType == AT_V_ENTIRE_TYPE)
    {
        lMemResult = memcpy_s((TAF_CHAR *)pgucAtSndCrLfAddr, AT_CMD_MAX_LEN + 20 - 1, (TAF_CHAR *)gaucAtCrLf,2);
        TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN + 20 - 1, 2);
        At_SendResultData(ucIndex,pgucAtSndCrLfAddr,usLength + 2);
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    }

    return;
}


TAF_VOID At_GetMsgFoValue(
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo,
    TAF_UINT8                           *pucFo
)
{
    TAF_UINT8                           ucFo                = 0;

    switch (pstTsDataInfo->enTpduType)
    {
        case MN_MSG_TPDU_DELIVER:
            /* TP-MTI, TP-MMS, TP-RP, TP_UDHI, TP-SRI:*/
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_DELIVER);
            AT_SET_MSG_TP_MMS(ucFo, pstTsDataInfo->u.stDeliver.bMoreMsg);
            AT_SET_MSG_TP_RP(ucFo, pstTsDataInfo->u.stDeliver.bReplayPath);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stDeliver.bUserDataHeaderInd);
            AT_SET_MSG_TP_SRI(ucFo, pstTsDataInfo->u.stDeliver.bStaRptInd);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            /*TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_DELIVER_REPORT);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stDeliverRptAck.bUserDataHeaderInd);
            break;
        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            /*TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_DELIVER_REPORT);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stDeliverRptErr.bUserDataHeaderInd);
            break;
        case MN_MSG_TPDU_STARPT:
            /*TP MTI TP UDHI TP MMS TP SRQ*/
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_STATUS_REPORT);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stStaRpt.bUserDataHeaderInd);
            AT_SET_MSG_TP_MMS(ucFo, pstTsDataInfo->u.stStaRpt.bMoreMsg);
            AT_SET_MSG_TP_SRQ(ucFo, pstTsDataInfo->u.stStaRpt.bStaRptQualCommand);/*??*/
            break;
        case MN_MSG_TPDU_SUBMIT:
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_SUBMIT);
            AT_SET_MSG_TP_RD(ucFo, pstTsDataInfo->u.stSubmit.bRejectDuplicates);
            AT_SET_MSG_TP_VPF(ucFo, pstTsDataInfo->u.stSubmit.stValidPeriod.enValidPeriod);
            AT_SET_MSG_TP_RP(ucFo, pstTsDataInfo->u.stSubmit.bReplayPath);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stSubmit.bUserDataHeaderInd);
            AT_SET_MSG_TP_SRR(ucFo, pstTsDataInfo->u.stSubmit.bStaRptReq);
            break;
        case MN_MSG_TPDU_COMMAND:
            /*TP MTI TP UDHI TP SRR */
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_COMMAND);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stCommand.bUserDataHeaderInd);
            AT_SET_MSG_TP_SRR(ucFo, pstTsDataInfo->u.stCommand.bStaRptReq);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            /*TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_SUBMIT_REPORT);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stSubmitRptAck.bUserDataHeaderInd);
            break;
        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            /*TP MTI  TP-UDHI  */
            AT_SET_MSG_TP_MTI(ucFo, AT_MSG_TP_MTI_SUBMIT_REPORT);
            AT_SET_MSG_TP_UDHI(ucFo, pstTsDataInfo->u.stSubmitRptErr.bUserDataHeaderInd);
            break;
        default:
            AT_NORM_LOG("At_GetMsgFoValue: invalid TPDU type.");
            break;
    }

    *pucFo = ucFo;
    return;
}


VOS_VOID At_SendMsgFoAttr(
    VOS_UINT8                           ucIndex,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo
)
{
    TAF_UINT8                           ucFo = 0;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CMGC_TEXT_SET)
    {
        ucFo = gastAtClientTab[ucIndex].AtSmsData.ucFo;
    }
    else
    {
        /*�ж�FO����Ч��*/
        if (pstSmsCtx->stCscaCsmpInfo.bFoUsed == TAF_TRUE)
        {
            ucFo = pstSmsCtx->stCscaCsmpInfo.ucFo;
        }
        else
        {
            if ((pstTsDataInfo->enTpduType == MN_MSG_TPDU_DELIVER)
             || (pstTsDataInfo->enTpduType == MN_MSG_TPDU_SUBMIT))
            {
                ucFo = AT_CSMP_FO_DEFAULT_VALUE1;
            }
            else if ((pstTsDataInfo->enTpduType == MN_MSG_TPDU_STARPT)
                  || (pstTsDataInfo->enTpduType == MN_MSG_TPDU_COMMAND))
            {
                ucFo = AT_CSMP_FO_DEFAULT_VALUE2;
            }
            else
            {
                AT_NORM_LOG("At_SendMsgFoAttr: invalid enTpduType.");
                return;
            }
        }

    }

    switch (pstTsDataInfo->enTpduType)
    {
        case MN_MSG_TPDU_COMMAND:
            /*TP MTI TP UDHI TP SRR */
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stCommand.bUserDataHeaderInd, ucFo);
            AT_GET_MSG_TP_SRR(pstTsDataInfo->u.stCommand.bStaRptReq, ucFo);
            break;

        case MN_MSG_TPDU_DELIVER:
            /* TP-MTI, TP-MMS, TP-RP, TP_UDHI, TP-SRI:*/
            /*decode fo:TP MTI TP MMS TP RP TP UDHI TP SRI*/
            AT_GET_MSG_TP_MMS(pstTsDataInfo->u.stDeliver.bMoreMsg, ucFo);
            AT_GET_MSG_TP_RP(pstTsDataInfo->u.stDeliver.bReplayPath, ucFo);
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stDeliver.bUserDataHeaderInd, ucFo);
            AT_GET_MSG_TP_SRI(pstTsDataInfo->u.stDeliver.bStaRptInd, ucFo);
            break;

        case MN_MSG_TPDU_STARPT:
            /*TP MTI ignore TP UDHI TP MMS TP SRQ*/
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stStaRpt.bUserDataHeaderInd, ucFo);
            AT_GET_MSG_TP_MMS(pstTsDataInfo->u.stStaRpt.bMoreMsg, ucFo);
            AT_GET_MSG_TP_SRQ(pstTsDataInfo->u.stStaRpt.bStaRptQualCommand, ucFo);
            break;

        case MN_MSG_TPDU_SUBMIT:
            AT_GET_MSG_TP_RD(pstTsDataInfo->u.stSubmit.bRejectDuplicates, ucFo);
            /*TP VPF 23040 9.2.3.3*/
            AT_GET_MSG_TP_VPF(pstTsDataInfo->u.stSubmit.stValidPeriod.enValidPeriod, ucFo);
            /*TP RP  23040 9.2.3.17*/
            AT_GET_MSG_TP_RP(pstTsDataInfo->u.stSubmit.bReplayPath, ucFo);
            /*TP UDHI23040 9.2.3.23*/
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stSubmit.bUserDataHeaderInd, ucFo);
            /*TP SRR 23040 9.2.3.5*/
            AT_GET_MSG_TP_SRR(pstTsDataInfo->u.stSubmit.bStaRptReq, ucFo);
            break;

        case MN_MSG_TPDU_DELIVER_RPT_ACK:
            /*TP MTI ignore  TP-UDHI  */
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stDeliverRptAck.bUserDataHeaderInd, ucFo);
            break;

        case MN_MSG_TPDU_DELIVER_RPT_ERR:
            /*TP MTI ignore  TP-UDHI  */
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stDeliverRptErr.bUserDataHeaderInd, ucFo);
            break;

        case MN_MSG_TPDU_SUBMIT_RPT_ACK:
            /*TP MTI ignore TP-UDHI  */
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stSubmitRptAck.bUserDataHeaderInd, ucFo);
            break;

        case MN_MSG_TPDU_SUBMIT_RPT_ERR:
            /*TP MTI ignore TP-UDHI  */
            AT_GET_MSG_TP_UDHI(pstTsDataInfo->u.stSubmitRptErr.bUserDataHeaderInd, ucFo);
            break;

        default:
            AT_NORM_LOG("At_SendMsgFoAttr: invalid pdu type.");
            break;
    }
    return;
}


TAF_VOID  At_PrintCsmsInfo(
    TAF_UINT8                           ucIndex
)
{
    AT_MSG_SERV_STRU                    stMsgServInfo       = {AT_MSG_SERV_STATE_SUPPORT,
                                                               AT_MSG_SERV_STATE_SUPPORT,
                                                               AT_MSG_SERV_STATE_SUPPORT};

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                     (TAF_CHAR *)pgucAtSndCodeAddr,
                                                     (TAF_CHAR *)(pgucAtSndCodeAddr + gstAtSendData.usBufLen),
                                                     "%d,%d,%d",
                                                     stMsgServInfo.enSmsMT,
                                                     stMsgServInfo.enSmsMO,
                                                     stMsgServInfo.enSmsBM);
    return;
}


TAF_UINT16 At_PrintAsciiAddr(
    MN_MSG_ASCII_ADDR_STRU              *pstAddr,
    TAF_UINT8                           *pDst
)
{
    TAF_UINT16                          usLength            = 0;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pDst + usLength,"\"");
    if ((pstAddr->ulLen > 0)
     && (pstAddr->ulLen <= MN_MAX_ASCII_ADDRESS_NUM))
    {
        pstAddr->aucAsciiNum[pstAddr->ulLen] = 0;

        if (pstAddr->enNumType == MN_MSG_TON_INTERNATIONAL)
        {
            usLength += (TAF_UINT16)At_ReadNumTypePara((pDst + usLength), (TAF_UINT8 *)"+");
        }

        usLength += (TAF_UINT16)At_ReadNumTypePara((pDst + usLength), pstAddr->aucAsciiNum);
    }
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pDst + usLength,"\"");

    return usLength;
}



VOS_UINT32 AT_BcdAddrToAscii(
    MN_MSG_BCD_ADDR_STRU                *pstBcdAddr,
    MN_MSG_ASCII_ADDR_STRU              *pstAsciiAddr
)
{
    VOS_UINT32                          ulRet;

    if ((pstBcdAddr == VOS_NULL_PTR)
     || (pstAsciiAddr == VOS_NULL_PTR))
    {
        AT_WARN_LOG("MN_MSG_BcdAddrToAscii: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (pstBcdAddr->ucBcdLen > MN_MSG_MAX_BCD_NUM_LEN)
    {
        AT_WARN_LOG("AT_BcdAddrToAscii: length of BcdAddr ucBcdLen is invalid.");

        return MN_ERR_INVALIDPARM;
    }

    pstAsciiAddr->enNumType = ((pstBcdAddr->addrType >> 4) & 0x07);
    pstAsciiAddr->enNumPlan = (pstBcdAddr->addrType & 0x0f);
    if((pstBcdAddr->aucBcdNum[pstBcdAddr->ucBcdLen - 1] & 0xF0) != 0xF0)
    {
        pstAsciiAddr->ulLen = pstBcdAddr->ucBcdLen * 2;
    }
    else
    {
        pstAsciiAddr->ulLen = (pstBcdAddr->ucBcdLen * 2) - 1;
    }

    if (pstAsciiAddr->ulLen > MN_MAX_ASCII_ADDRESS_NUM)
    {
        AT_WARN_LOG("MN_MSG_BcdAddrToAscii: length of number is invalid.");
        return MN_ERR_INVALIDPARM;
    }

    ulRet = AT_BcdNumberToAscii(pstBcdAddr->aucBcdNum, pstBcdAddr->ucBcdLen, (VOS_CHAR *)pstAsciiAddr->aucAsciiNum);
    if (ulRet == MN_ERR_NO_ERROR)
    {
        return MN_ERR_NO_ERROR;
    }
    else
    {
        return MN_ERR_INVALIDPARM;
    }
}


TAF_UINT16 At_PrintBcdAddr(
    MN_MSG_BCD_ADDR_STRU                *pstBcdAddr,
    TAF_UINT8                           *pDst
)
{
    TAF_UINT16                          usLength;
    TAF_UINT32                          ulRet;
    MN_MSG_ASCII_ADDR_STRU              stAsciiAddr;

    memset_s(&stAsciiAddr, sizeof(MN_MSG_ASCII_ADDR_STRU), 0x00, sizeof(stAsciiAddr));

    if (pstBcdAddr->ucBcdLen != 0 )
    {
        ulRet = AT_BcdAddrToAscii(pstBcdAddr, &stAsciiAddr);
        if (ulRet != MN_ERR_NO_ERROR)
        {
            AT_WARN_LOG("At_PrintBcdAddr: Fail to convert BCD to ASCII.");
            return 0;
        }
    }

    usLength = At_PrintAsciiAddr(&stAsciiAddr, pDst);
    return usLength;
}


TAF_UINT16  At_PrintAddrType(
    MN_MSG_ASCII_ADDR_STRU              *pstAddr,
    TAF_UINT8                           *pDst
)
{
    TAF_UINT8                           ucAddrType;
    TAF_UINT16                          usLength            = 0;

    ucAddrType  = 0x80;
    ucAddrType |= pstAddr->enNumPlan;
    ucAddrType |= ((pstAddr->enNumType << 4) & 0x70);
    usLength   += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pDst + usLength),
                                       "%d",
                                       ucAddrType);

    return usLength;
}


TAF_UINT16  At_PrintMsgFo(
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo,
    TAF_UINT8                           *pDst
)
{
    TAF_UINT16                          usLength            = 0;
    TAF_UINT8                           ucFo                = 0;

    At_GetMsgFoValue(pstTsDataInfo, &ucFo);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pDst + usLength,
                                       "%d",
                                       ucFo);

    return usLength;
}


VOS_UINT32 AT_ChkSmsNumType(
    MN_MSG_TON_ENUM_U8                  enNumType
)
{
    return MN_ERR_NO_ERROR;
}


VOS_UINT32 AT_ChkSmsNumPlan(
    MN_MSG_NPI_ENUM_U8                  enNumPlan
)
{
    return MN_ERR_NO_ERROR;
}


VOS_UINT32  AT_AsciiToBcdCode(
    VOS_CHAR                            cAsciiCode,
    VOS_UINT8                          *pucBcdCode
)
{
    if (pucBcdCode == VOS_NULL_PTR)
    {
        AT_NORM_LOG("AT_AsciiToBcdCode: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if ((cAsciiCode >= '0')
     && (cAsciiCode <= '9'))
    {
        *pucBcdCode = (VOS_UINT8)(cAsciiCode - '0');
    }
    else if (cAsciiCode == '*')
    {
        *pucBcdCode = 0x0a;
    }
    else if (cAsciiCode == '#')
    {
        *pucBcdCode = 0x0b;
    }
    else if ((cAsciiCode == 'a')
          || (cAsciiCode == 'b')
          || (cAsciiCode == 'c'))
    {
        *pucBcdCode = (VOS_UINT8)((cAsciiCode - 'a') + 0x0c);
    }
    else if ((cAsciiCode == 'A')
          || (cAsciiCode == 'B')
          || (cAsciiCode == 'C'))
    {
        *pucBcdCode = (VOS_UINT8)((cAsciiCode - 'A') + 0x0c);
    }
    else if (cAsciiCode == '+')
    {
        return MN_ERR_PLUS_SIGN;
    }
    else
    {
        AT_NORM_LOG("AT_AsciiToBcdCode: Parameter of the function is invalid.");
        return MN_ERR_INVALID_ASCII;
    }

    return MN_ERR_NO_ERROR;
}


VOS_UINT32  AT_AsciiNumberToBcd(
    const VOS_CHAR                      *pcAsciiNumber,
    VOS_UINT8                           *pucBcdNumber,
    VOS_UINT8                           *pucBcdLen
)
{
    VOS_UINT8                           ucInputLoop;
    VOS_UINT8                           ucOutputLoop;
    VOS_UINT8                           ucBcdCode;
    VOS_UINT32                          ulRet;

    if ((pcAsciiNumber == TAF_NULL_PTR)
     || (pucBcdNumber == TAF_NULL_PTR)
     || (pucBcdLen == TAF_NULL_PTR))
    {
        AT_NORM_LOG("AT_AsciiNumberToBcd: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    for (ucInputLoop = 0, ucOutputLoop = 0; pcAsciiNumber[ucInputLoop] != '\0'; ucInputLoop++)
    {
        ulRet = AT_AsciiToBcdCode(pcAsciiNumber[ucInputLoop], &ucBcdCode);

        /* AT_IsDCmdValidChar��ǰ�ǽ������е�+�˵� */
        /* �����������˵�����ΪBCD����û�ж�+�ı�ʾ */
        if ((ulRet == MN_ERR_INVALID_ASCII)
         || (ulRet == MN_ERR_NULLPTR))
        {
            return ulRet;
        }
        else if (ulRet == MN_ERR_PLUS_SIGN)
        {
            continue;
        }
        else
        {
            /* for lint */
        }

        /*����ǰ��Ҫ����Ŀռ���0*/
        pucBcdNumber[(ucOutputLoop / 2)] &= ((ucOutputLoop % 2) == 1) ? 0x0F : 0xF0;

        /*������������Ӧ�Ŀռ�*/
        pucBcdNumber[(ucOutputLoop / 2)] |= (((ucOutputLoop % 2) == 1) ? ((ucBcdCode << 4) & 0xF0) : (ucBcdCode & 0x0F));

        ucOutputLoop++;
    }

    /*�������Ϊ�����������һ���ַ���Ҫ�� F */
    if ((ucOutputLoop % 2) == 1)
    {
        pucBcdNumber[(ucOutputLoop / 2)] |= 0xF0;
    }

    *pucBcdLen = (ucOutputLoop + 1) / 2;

    return MN_ERR_NO_ERROR;
}


VOS_UINT32  AT_BcdToAsciiCode(
    VOS_UINT8                           ucBcdCode,
    VOS_CHAR                            *pcAsciiCode
)
{
    VOS_CHAR                            cAsciiCode;

    if (pcAsciiCode == TAF_NULL_PTR)
    {
        AT_NORM_LOG("AT_BcdToAsciiCode: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    if (ucBcdCode <= 0x09)
    {
        cAsciiCode = (VOS_CHAR)(ucBcdCode + 0x30);
    }
    else if (ucBcdCode == 0x0A)
    {
        cAsciiCode = (VOS_CHAR)(ucBcdCode + 0x20);    /*�ַ�'*'*/
    }
    else if (ucBcdCode == 0x0B)
    {
        cAsciiCode = (VOS_CHAR)(ucBcdCode + 0x18);    /*�ַ�'#'*/
    }
    else if ((ucBcdCode == 0x0C)
          || (ucBcdCode == 0x0D)
          || (ucBcdCode == 0x0E))
    {
        cAsciiCode = (VOS_CHAR)(ucBcdCode + 0x55);    /*�ַ�'a', 'b', 'c'*/
    }
    else
    {
        AT_NORM_LOG("AT_BcdToAsciiCode: Parameter of the function is invalid.");
        return MN_ERR_INVALID_BCD;
    }

    *pcAsciiCode = cAsciiCode;

    return MN_ERR_NO_ERROR;
}


VOS_UINT32  AT_BcdNumberToAscii(
    const VOS_UINT8                     *pucBcdNumber,
    VOS_UINT8                           ucBcdLen,
    VOS_CHAR                            *pcAsciiNumber
)
{
    VOS_UINT8                           ucLoop;
    VOS_UINT8                           ucLen;
    VOS_UINT8                           ucBcdCode;
    VOS_UINT32                          ulRet;

    if ((pucBcdNumber == TAF_NULL_PTR)
     || (pcAsciiNumber == TAF_NULL_PTR))
    {
        AT_NORM_LOG("AT_BcdNumberToAscii: Parameter of the function is null.");
        return MN_ERR_NULLPTR;
    }

    /*��������ַ�����ȥ����Ч��0XFF����*/
    while (ucBcdLen > 1)
    {
        if (pucBcdNumber[ucBcdLen - 1] == 0xFF)
        {
            ucBcdLen--;
        }
        else
        {
            break;
        }
    }

    /*�ж�pucBcdAddress��ָ����ַ��������һ���ֽڵĸ�λ�Ƿ�Ϊ1111��
    ����ǣ�˵������λ��Ϊ����������Ϊż��*/
    if ((pucBcdNumber[ucBcdLen - 1] & 0xF0) == 0xF0)
    {
        ucLen = (VOS_UINT8)((ucBcdLen * 2) - 1);
    }
    else
    {
        ucLen = (VOS_UINT8)(ucBcdLen * 2);
    }

    /*��������*/
    for (ucLoop = 0; ucLoop < ucLen; ucLoop++)
    {
        /*�жϵ�ǰ�����������λ���뻹��ż��λ���룬��0��ʼ����ż��*/
        if ((ucLoop % 2) == 1)
        {
            /*���������λ���룬��ȡ��4λ��ֵ*/
            ucBcdCode = ((pucBcdNumber[(ucLoop / 2)] >> 4) & 0x0F);
        }
        else
        {
            /*�����ż��λ���룬��ȡ��4λ��ֵ*/
            ucBcdCode = (pucBcdNumber[(ucLoop / 2)] & 0x0F);
        }

        /*������������ת����Ascii����ʽ*/
        ulRet = AT_BcdToAsciiCode(ucBcdCode, &(pcAsciiNumber[ucLoop]));
        if (ulRet != MN_ERR_NO_ERROR)
        {
            return ulRet;
        }
    }

    pcAsciiNumber[ucLoop] = '\0';      /*�ַ���ĩβΪ0*/

    return MN_ERR_NO_ERROR;
}



TAF_UINT32  At_GetAsciiOrBcdAddr(
    TAF_UINT8                           *pucAddr,
    TAF_UINT16                          usAddrLen,
    TAF_UINT8                           ucAddrType,
    TAF_UINT16                          usNumTypeLen,
    MN_MSG_ASCII_ADDR_STRU              *pstAsciiAddr,
    MN_MSG_BCD_ADDR_STRU                *pstBcdAddr
)
{
    TAF_UINT8                           aucAsciiNum[MN_MAX_ASCII_ADDRESS_NUM+2];   /*array  of ASCII Num*/
    TAF_UINT8                           *pucNum = VOS_NULL_PTR;                                /*ָ��ʵ�ʺ��루������+�ţ���ָ��*/
    TAF_UINT32                          ulAsciiAddrLen;
    TAF_UINT32                          ulRet;
    errno_t                             lMemResult;
    MN_MSG_TON_ENUM_U8                  enNumType;                              /*type of number*/
    MN_MSG_NPI_ENUM_U8                  enNumPlan;                              /*Numbering plan identification*/

    if ((pstAsciiAddr == TAF_NULL_PTR)
     && (pstBcdAddr == TAF_NULL_PTR))
    {
        AT_WARN_LOG("At_GetAsciiOrBcdAddr: output parameter is null. ");
        return AT_ERROR;
    }

    if (pstAsciiAddr != TAF_NULL_PTR)
    {
        memset_s(pstAsciiAddr, sizeof(MN_MSG_ASCII_ADDR_STRU), 0x00, sizeof(MN_MSG_ASCII_ADDR_STRU));
    }

    if (pstBcdAddr != TAF_NULL_PTR)
    {
        memset_s(pstBcdAddr, sizeof(MN_MSG_BCD_ADDR_STRU), 0x00, sizeof(MN_MSG_BCD_ADDR_STRU));
    }

    if(usAddrLen == 0)
    {
        return AT_OK;
    }

    if(At_CheckNumLen((MN_MAX_ASCII_ADDRESS_NUM + 1), usAddrLen) == AT_FAILURE)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    *(pucAddr + usAddrLen) = '\0';

    memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, (MN_MAX_ASCII_ADDRESS_NUM + 2));
    if(At_SetNumTypePara(aucAsciiNum, sizeof(aucAsciiNum), (TAF_UINT8 *)pucAddr, usAddrLen) != AT_SUCCESS)
    {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* ����<toda> */
    if ((At_GetCodeType(aucAsciiNum[0])) == AT_MSG_INTERNAL_ISDN_ADDR_TYPE)
    {
        pucNum = (TAF_UINT8 *)(aucAsciiNum + 1);
        if ((usNumTypeLen != 0)
         && (ucAddrType != AT_MSG_INTERNAL_ISDN_ADDR_TYPE))
        {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }
    else
    {
        pucNum = (TAF_UINT8 *)aucAsciiNum;
    }

    if (usNumTypeLen == 0)
    {
        ucAddrType = (TAF_UINT8)At_GetCodeType(aucAsciiNum[0]);
    }

    ulAsciiAddrLen = VOS_StrLen((TAF_CHAR *)pucNum);
    if (ulAsciiAddrLen > MN_MAX_ASCII_ADDRESS_NUM)
    {
        AT_NORM_LOG("At_GetAsciiOrBcdAddr: invalid address length.");
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    At_GetNumTypeFromAddrType(enNumType, ucAddrType);
    At_GetNumPlanFromAddrType(enNumPlan, ucAddrType);
    ulRet = AT_ChkSmsNumPlan(enNumPlan);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        AT_ERR_LOG("At_GetAsciiOrBcdAddr: Numbering plan is invalid");
        return AT_ERROR;
    }
    ulRet = AT_ChkSmsNumType(enNumType);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        AT_ERR_LOG("At_GetAsciiOrBcdAddr: Number type is invalid");
        return AT_ERROR;
    }

    if (pstAsciiAddr != TAF_NULL_PTR)
    {
        pstAsciiAddr->enNumType = enNumType;
        pstAsciiAddr->enNumPlan = enNumPlan;
        pstAsciiAddr->ulLen = ulAsciiAddrLen;
        lMemResult = memcpy_s(pstAsciiAddr->aucAsciiNum, sizeof(pstAsciiAddr->aucAsciiNum), pucNum, pstAsciiAddr->ulLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstAsciiAddr->aucAsciiNum), pstAsciiAddr->ulLen);
    }

    if (pstBcdAddr != TAF_NULL_PTR)
    {
        pstBcdAddr->addrType = ucAddrType;
        ulRet = AT_AsciiNumberToBcd((TAF_CHAR *)pucNum,
                                    pstBcdAddr->aucBcdNum,
                                    &pstBcdAddr->ucBcdLen);
        if (ulRet != MN_ERR_NO_ERROR)
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}


VOS_VOID AT_JudgeIsPlusSignInDialString(
    const VOS_CHAR                      *pcAsciiNumber,
    VOS_UINT8                           *ucIsExistPlusSign,
    VOS_UINT8                           *ucPlusSignLocation
)
{
    VOS_UINT8                           ucLoop;

    for (ucLoop = 0; pcAsciiNumber[ucLoop] != '\0'; ucLoop++)
    {
        if (pcAsciiNumber[ucLoop] == '+')
        {
            *ucIsExistPlusSign  = VOS_TRUE;
            *ucPlusSignLocation = ucLoop;
            break;
        }
    }

    return ;
}


TAF_UINT32  At_PrintListMsg(
    VOS_UINT8                            ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo,
    TAF_UINT8                           *pucDst
)
{
    TAF_UINT16                          usLength = 0;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    switch (pstTsDataInfo->enTpduType)
    {
        case MN_MSG_TPDU_DELIVER:
            /* +CMGL: <index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,<length>]<CR><LF><data>[<CR><LF>*/
            /* <oa> */
            usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stDeliver.stOrigAddr,
                                                      (pucDst + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");
            /* <alpha> ���� */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");

            /* <scts> */
            usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stDeliver.stTimeStamp,
                                                    (pucDst + usLength));
            if (pstSmsCtx->ucCsdhType == AT_CSDH_SHOW_TYPE)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pucDst + usLength),
                                                   ",");

                /* <tooa> */
                usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stDeliver.stOrigAddr,
                                                         (pucDst + usLength));


                /* <length> */
                usLength += AT_PrintSmsLength(pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                              pstTsDataInfo->u.stDeliver.stUserData.ulLen,
                                              (pucDst + usLength));
            }

            /* <data> �п��ܵõ���UCS2������ϸ����*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pucDst,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               "%s",
                                               gaucAtCrLf);

             usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN - (TAF_UINT32)(pucDst - pgucAtSndCodeAddr),
                                                       (TAF_INT8 *)pucDst,
                                                       pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                                       (pucDst + usLength),
                                                       pstTsDataInfo->u.stDeliver.stUserData.aucOrgData,
                                                       (TAF_UINT16)pstTsDataInfo->u.stDeliver.stUserData.ulLen);

            break;
        case MN_MSG_TPDU_SUBMIT:
            /* +CMGL: <index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,<length>]<CR><LF><data>[<CR><LF>*/
            /* <da> */
            usLength += At_PrintAsciiAddr(&pstTsDataInfo->u.stSubmit.stDestAddr,
                                          (TAF_UINT8 *)(pucDst + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");

            /* <alpha> ���� */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");

            if (pstSmsCtx->ucCsdhType == AT_CSDH_SHOW_TYPE)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pucDst + usLength),
                                                   ",");

                /* <toda> */
                usLength += At_PrintAddrType(&pstTsDataInfo->u.stSubmit.stDestAddr,
                                             (pucDst + usLength));

                /* <length> */
                usLength += AT_PrintSmsLength(pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                              pstTsDataInfo->u.stSubmit.stUserData.ulLen,
                                              (pucDst + usLength));

            }

            /* <data> �п��ܵõ���UCS2������ϸ����*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pucDst,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               "%s",
                                               gaucAtCrLf);

            usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN,
                                                       (TAF_INT8 *)pucDst,
                                                       pstTsDataInfo->u.stSubmit.stDcs.enMsgCoding,
                                                       (pucDst + usLength),
                                                       pstTsDataInfo->u.stSubmit.stUserData.aucOrgData,
                                                       (TAF_UINT16)pstTsDataInfo->u.stSubmit.stUserData.ulLen);

            break;
        case MN_MSG_TPDU_COMMAND:
            /*+CMGL: <index>,<stat>,<fo>,<ct>[<CR><LF>*/
            /*<fo>*/
            usLength += At_PrintMsgFo(pstTsDataInfo, (pucDst + usLength));
            /* <ct> */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pucDst,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",%d",
                                               pstTsDataInfo->u.stCommand.enCmdType);
            break;
        case MN_MSG_TPDU_STARPT:
            /*
               +CMGL: <index>,<stat>,<fo>,<mr>,
                      [<ra>],[<tora>],<scts>,<dt>,<st>
                      [<CR><LF>
            */
            /*<fo>*/
            usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pucDst + usLength));
            /*<mr>*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",%d,",
                                               pstTsDataInfo->u.stStaRpt.ucMr);

            /*<ra>*/
            usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stStaRpt.stRecipientAddr,
                                                      (pucDst + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");

            /*<tora>*/
            usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stStaRpt.stRecipientAddr,
                                                     (pucDst + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");

            /* <scts> */
            usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stStaRpt.stTimeStamp,
                                                    (pucDst + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pucDst + usLength),
                                               ",");

             /* <dt> */
             usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stStaRpt.stDischargeTime,
                                                     (pucDst + usLength));

             /*<st>*/
             usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)(pucDst + usLength),
                                                ",%d",
                                                pstTsDataInfo->u.stStaRpt.enStatus);
            break;
        default:
            break;
    }

    return usLength;
}


TAF_VOID At_GetCpmsMemStatus(
    VOS_UINT8                           ucIndex,
    MN_MSG_MEM_STORE_ENUM_U8            enMemType,
    TAF_UINT32                          *pulTotalRec,
    TAF_UINT32                          *pulUsedRec
)
{
    MN_MSG_STORAGE_LIST_EVT_INFO_STRU   *pstStorageList = VOS_NULL_PTR;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (enMemType == MN_MSG_MEM_STORE_SIM)
    {
        pstStorageList = &(pstSmsCtx->stCpmsInfo.stUsimStorage);
    }
    else if (enMemType == MN_MSG_MEM_STORE_ME)
    {
        pstStorageList = &(pstSmsCtx->stCpmsInfo.stNvimStorage);
    }
    else/*�޴洢�豸*/
    {
        *pulUsedRec = 0;
        *pulTotalRec = 0;
        return;
    }

    *pulTotalRec = pstStorageList->ulTotalRec;
    *pulUsedRec = pstStorageList->ulUsedRec;

    return;
}


TAF_VOID At_PrintSetCpmsRsp(
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT16                          usLength            = 0;
    TAF_UINT32                          ulTotalRec;                             /*sms capacity of NVIM or USIM*/
    TAF_UINT32                          ulUsedRec;                              /*used records including all status*/
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /*<used1>,<total1>,<used2>,<total2>,<used3>,<total3>*/
    At_GetCpmsMemStatus(ucIndex, pstSmsCtx->stCpmsInfo.enMemReadorDelete, &ulTotalRec, &ulUsedRec);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d,%d,",
                                       ulUsedRec,
                                       ulTotalRec);

    At_GetCpmsMemStatus(ucIndex, pstSmsCtx->stCpmsInfo.enMemSendorWrite, &ulTotalRec, &ulUsedRec);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d,%d,",
                                       ulUsedRec,
                                       ulTotalRec);

    At_GetCpmsMemStatus(ucIndex, pstSmsCtx->stCpmsInfo.stRcvPath.enSmMemStore, &ulTotalRec, &ulUsedRec);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d,%d",
                                       ulUsedRec,
                                       ulTotalRec);

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return;
}


TAF_UINT8 *At_GetCpmsMemTypeStr(
    MN_MSG_MEM_STORE_ENUM_U8            enMemType
)
{
    TAF_UINT32                          ulMemType;

    if (enMemType == MN_MSG_MEM_STORE_SIM)
    {
        ulMemType = AT_STRING_SM;
    }
    else if (enMemType == MN_MSG_MEM_STORE_ME)
    {
        ulMemType = AT_STRING_ME;
    }
    else
    {
        ulMemType = AT_STRING_BUTT;
    }
    return gastAtStringTab[ulMemType].pucText;
}


VOS_VOID At_PrintGetCpmsRsp(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT16                          usLength            = 0;
    VOS_UINT32                          ulTotalRec;                             /*sms capacity of NVIM or USIM*/
    VOS_UINT32                          ulUsedRec;                              /*used records including all status*/
    VOS_UINT8                          *pucMemTypeStr       = VOS_NULL_PTR;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx           = VOS_NULL_PTR;

    ulTotalRec = 0;
    ulUsedRec  = 0;
    pstSmsCtx  = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /*<used1>,<total1>,<used2>,<total2>,<used3>,<total3>*/
    pucMemTypeStr = At_GetCpmsMemTypeStr(pstSmsCtx->stCpmsInfo.enMemReadorDelete);
    At_GetCpmsMemStatus(ucIndex, pstSmsCtx->stCpmsInfo.enMemReadorDelete, &ulTotalRec, &ulUsedRec);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s,%d,%d,",
                                       pucMemTypeStr,
                                       ulUsedRec,
                                       ulTotalRec);

    pucMemTypeStr = At_GetCpmsMemTypeStr(pstSmsCtx->stCpmsInfo.enMemSendorWrite);
    At_GetCpmsMemStatus(ucIndex, pstSmsCtx->stCpmsInfo.enMemSendorWrite, &ulTotalRec, &ulUsedRec);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s,%d,%d,",
                                       pucMemTypeStr,
                                       ulUsedRec,
                                       ulTotalRec);

    pucMemTypeStr = At_GetCpmsMemTypeStr(pstSmsCtx->stCpmsInfo.stRcvPath.enSmMemStore);
    At_GetCpmsMemStatus(ucIndex, pstSmsCtx->stCpmsInfo.stRcvPath.enSmMemStore, &ulTotalRec, &ulUsedRec);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s,%d,%d",
                                       pucMemTypeStr,
                                       ulUsedRec,
                                       ulTotalRec);

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return;
}


TAF_VOID AT_PrintTimeZone(
    TAF_INT8                            cTimezone,
    TAF_UINT8                           *pucDst,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTimeZone;
    TAF_UINT16                          usLength;

    if (cTimezone < 0)
    {
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pucDst,
                                           "-");
        ucTimeZone  = (TAF_UINT8)(cTimezone * (-1));
    }
    else
    {
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pucDst,
                                           "+");
        ucTimeZone  = (TAF_UINT8)cTimezone;
    }

    if (ucTimeZone > MN_MSG_MAX_TIMEZONE_VALUE)
    {
        AT_WARN_LOG("AT_PrintTimeZone: Time zone is invalid.");
        ucTimeZone = 0;
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pucDst + usLength),
                                       "%d%d\"",
                                       (0x0f & (ucTimeZone / 10)),
                                       (ucTimeZone % 10));

    *pusLength = usLength;

    return;
}


TAF_UINT32 At_SmsPrintScts(
    const MN_MSG_TIMESTAMP_STRU         *pstTimeStamp,
    TAF_UINT8                           *pDst
)
{
    TAF_UINT16                          usLength;
    TAF_UINT16                          usTimeZoneLength;
    MN_MSG_DATE_INVALID_TYPE_ENUM_UINT8 ucDateInvalidType;
    TAF_UINT32                          ulRet;

    ulRet = MN_MSG_ChkDate(pstTimeStamp, &ucDateInvalidType);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        AT_WARN_LOG("At_SmsPrintScts: Date is invalid.");
    }

    /* "yy/MM/dd,hh:mm:ss��zz" */
    if ((MN_MSG_DATE_INVALID_YEAR & ucDateInvalidType) == 0)
    {
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst,
                                           "\"%d%d/",
                                           ((pstTimeStamp->ucYear >> 4) & 0x0f),
                                           (pstTimeStamp->ucYear & 0x0f));
    }
    else
    {
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst,
                                           "\"00/");
    }

    /* MM */
    if ((MN_MSG_DATE_INVALID_MONTH & ucDateInvalidType) == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "%d%d/",
                                           ((pstTimeStamp->ucMonth>> 4) & 0x0f),
                                           (pstTimeStamp->ucMonth & 0x0f));
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "01/");
    }

    /* dd */
    if ((MN_MSG_DATE_INVALID_DAY & ucDateInvalidType) == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "%d%d,",
                                           ((pstTimeStamp->ucDay >> 4) & 0x0f),
                                           (pstTimeStamp->ucDay & 0x0f));
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "01,");
    }

    /* hh */
    if ((MN_MSG_DATE_INVALID_HOUR & ucDateInvalidType) == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "%d%d:",
                                           ((pstTimeStamp->ucHour >> 4) & 0x0f),
                                           (pstTimeStamp->ucHour & 0x0f));
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "00:");
    }

    /* mm */
    if ((MN_MSG_DATE_INVALID_MINUTE & ucDateInvalidType) == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "%d%d:",
                                           ((pstTimeStamp->ucMinute >> 4) & 0x0f),
                                           (pstTimeStamp->ucMinute & 0x0f));
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "00:");
    }

    /* ss */
    if ((MN_MSG_DATE_INVALID_SECOND & ucDateInvalidType) == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "%d%d",
                                           ((pstTimeStamp->ucSecond >> 4) & 0x0f),
                                           (pstTimeStamp->ucSecond & 0x0f));
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pDst + usLength,
                                           "00");
    }

    /* ��zz */
    AT_PrintTimeZone(pstTimeStamp->cTimezone,
                     (pDst + usLength),
                     &usTimeZoneLength);
    usLength += usTimeZoneLength;

    return usLength;
}


TAF_UINT16 At_MsgPrintVp(
    MN_MSG_VALID_PERIOD_STRU            *pstValidPeriod,
    TAF_UINT8 *pDst
)
{
    TAF_UINT16 usLength = 0;

    switch (pstValidPeriod->enValidPeriod)
    {
        case MN_MSG_VALID_PERIOD_RELATIVE:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pDst + usLength,
                                               "%d",
                                               pstValidPeriod->u.ucOtherTime);
            break;
        case MN_MSG_VALID_PERIOD_ABSOLUTE:
            usLength += (TAF_UINT16)At_SmsPrintScts(&pstValidPeriod->u.stAbsoluteTime, pDst);
            break;
        default:
            break;
    }
    return usLength;
}


TAF_UINT32 At_SmsPrintState(
    AT_CMGF_MSG_FORMAT_ENUM_U8          enSmsFormat,
    MN_MSG_STATUS_TYPE_ENUM_U8          enStatus,
    TAF_UINT8                           *pDst
)
{
    TAF_UINT16 usLength = 0;

    if(enSmsFormat == AT_CMGF_MSG_FORMAT_TEXT)    /* TEXT */
    {
        switch(enStatus)
        {
        case MN_MSG_STATUS_MT_NOT_READ:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_REC_UNREAD_TEXT].pucText);
            break;

        case MN_MSG_STATUS_MT_READ:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_REC_READ_TEXT].pucText);
            break;

        case MN_MSG_STATUS_MO_NOT_SENT:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_STO_UNSENT_TEXT].pucText);
            break;

        case MN_MSG_STATUS_MO_SENT:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_STO_SENT_TEXT].pucText);
            break;

        case MN_MSG_STATUS_NONE:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_ALL_TEXT].pucText);
            break;

        default:
            return 0;
        }
    }
    else        /* PDU */
    {
        switch(enStatus)
        {
        case MN_MSG_STATUS_MT_NOT_READ:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_REC_UNREAD_PDU].pucText);
            break;

        case MN_MSG_STATUS_MT_READ:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_REC_READ_PDU].pucText);
            break;

        case MN_MSG_STATUS_MO_NOT_SENT:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_STO_UNSENT_PDU].pucText);
            break;

        case MN_MSG_STATUS_MO_SENT:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_STO_SENT_PDU].pucText);
            break;

        case MN_MSG_STATUS_NONE:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,"%s",gastAtStringTab[AT_STRING_ALL_PDU].pucText);
            break;

        default:
            return 0;
        }
    }

    return usLength;
}


TAF_UINT32  At_GetScaFromInputStr(
    const TAF_UINT8                     *pucAddr,
    MN_MSG_BCD_ADDR_STRU                *pstBcdAddr,
    TAF_UINT32                          *pulLen
)
{
    TAF_UINT32                          ulRet;
    MN_MSG_ASCII_ADDR_STRU              stAsciiAddr;

    ulRet = MN_MSG_DecodeAddress(pucAddr, TAF_TRUE, &stAsciiAddr, pulLen);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        return ulRet;
    }

    pstBcdAddr->addrType = 0x80;
    pstBcdAddr->addrType |= stAsciiAddr.enNumPlan;
    pstBcdAddr->addrType |= ((stAsciiAddr.enNumType << 4) & 0x70);

    ulRet = AT_AsciiNumberToBcd((TAF_CHAR *)stAsciiAddr.aucAsciiNum, pstBcdAddr->aucBcdNum, &pstBcdAddr->ucBcdLen);

    return ulRet;
}


TAF_UINT32 At_MsgDeleteCmdProc(
    TAF_UINT8                           ucIndex,
    MN_OPERATION_ID_T                   opId,
    MN_MSG_DELETE_PARAM_STRU            stDelete,
    TAF_UINT32                          ulDeleteTypes
)
{
    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes & AT_MSG_DELETE_SINGLE)
    {
        stDelete.enDeleteType = MN_MSG_DELETE_SINGLE;
        if (MN_MSG_Delete(gastAtClientTab[ucIndex].usClientId, opId, &stDelete) != MN_ERR_NO_ERROR)
        {
            return AT_ERROR;
        }
        else
        {
            return AT_OK;
        }
    }

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes & AT_MSG_DELETE_READ)
    {
        stDelete.enDeleteType = MN_MSG_DELETE_READ;
        if (MN_MSG_Delete(gastAtClientTab[ucIndex].usClientId, opId, &stDelete) != MN_ERR_NO_ERROR)
        {
            return AT_ERROR;
        }
        else
        {
            return AT_OK;
        }
    }

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes & AT_MSG_DELETE_SENT)
    {
        stDelete.enDeleteType = MN_MSG_DELETE_SENT;
        if (MN_MSG_Delete(gastAtClientTab[ucIndex].usClientId, opId, &stDelete) != MN_ERR_NO_ERROR)
        {
            return AT_ERROR;
        }
        else
        {
            return AT_OK;
        }
    }

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes & AT_MSG_DELETE_UNSENT)
    {
        stDelete.enDeleteType = MN_MSG_DELETE_NOT_SENT;
        if (MN_MSG_Delete(gastAtClientTab[ucIndex].usClientId, opId, &stDelete) != MN_ERR_NO_ERROR)
        {
            return AT_ERROR;
        }
        else
        {
            return AT_OK;
        }
    }

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes & AT_MSG_DELETE_ALL)
    {
        stDelete.enDeleteType = MN_MSG_DELETE_ALL;
        if (MN_MSG_Delete(gastAtClientTab[ucIndex].usClientId, opId, &stDelete) != MN_ERR_NO_ERROR)
        {
            return AT_ERROR;
        }
        else
        {
            return AT_OK;
        }
    }

    return AT_ERROR;
}



