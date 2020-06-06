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



#include "siappstk.h"
#include "si_stk.h"
#include "product_config.h"

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#include "si_pih.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "UsimPsInterface.h"
#include "UsimmInterface.h"
#include "SiCcoreApi.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID PS_FILE_ID_STK_API_C
#define    THIS_MODU    mod_pam_stk

/*Envelope ����ظ��б� */
VOS_UINT8 gucCallCtrlRsp[]      = {CAP_CFG_PARA_TAG,SUBADDRESS_TAG,ALPHA_IDENTIFIER_TAG,BC_REPEAT_INDICATOR_TAG,CAP_CFG_PARA_TAG};

VOS_UINT8 gucMOSMSCtrlRsp[]     = {ALPHA_IDENTIFIER_TAG};

SI_STK_TAGLIST_STRU gastEnvelopeDecodeList[2]
                                        = { {SI_STK_ENVELOPE_CALLCRTL,          sizeof(gucCallCtrlRsp),     gucCallCtrlRsp      },
                                            {SI_STK_ENVELOPE_SMSCRTL,           sizeof(gucMOSMSCtrlRsp),    gucMOSMSCtrlRsp     }
                                         };

#if  ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)))

VOS_UINT32 SI_STK_GetReceiverPid(
    MN_CLIENT_ID_T                      ClientId,
    VOS_UINT32                          *pulReceiverPid)
{
#if (MULTI_MODEM_NUMBER > 1)
    MODEM_ID_ENUM_UINT16                enModemID;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;

    /* ���ýӿڻ�ȡModem ID */
    if(VOS_OK != AT_GetModemIdFromClient(ClientId,&enModemID))
    {
        return VOS_ERR;
    }

    enSlotId = SI_GetSlotIdByModemId(enModemID);

    if (SI_PIH_CARD_SLOT_BUTT <= enSlotId)
    {
        return VOS_ERR;
    }

    if(SI_PIH_CARD_SLOT_0 == enSlotId)
    {
        *pulReceiverPid = I0_MAPS_STK_PID;
    }
    else if (SI_PIH_CARD_SLOT_1 == enSlotId)
    {
        *pulReceiverPid = I1_MAPS_STK_PID;
    }
    else
    {
        *pulReceiverPid = I2_MAPS_STK_PID;
    }
#else
    *pulReceiverPid = I0_MAPS_STK_PID;
#endif
    return VOS_OK;
}


VOS_UINT32 SI_STK_SendReqMsg(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT32                          MsgName,
    VOS_UINT32                          CmdType,
    VOS_UINT32                          DataLen,
    VOS_UINT8                           *pData)
{
    SI_STK_REQ_STRU                    *pstSTKReq = VOS_NULL_PTR;
    VOS_UINT32                          ulSendPid;
    VOS_UINT32                          ulReceiverPid;

    if (VOS_OK != SI_STK_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        STK_ERROR_LOG("SI_STK_SendReqMsg:Get ulReceiverPid Error.");
        return TAF_FAILURE;
    }

    ulSendPid = WUEPS_PID_AT;

    pstSTKReq = (SI_STK_REQ_STRU *)VOS_AllocMsg(ulSendPid, sizeof(SI_STK_REQ_STRU)- VOS_MSG_HEAD_LENGTH + DataLen);

    if(VOS_NULL_PTR == pstSTKReq)
    {
        STK_ERROR_LOG("SI_STK_SendReqMsg: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    pstSTKReq->MsgName       = MsgName;
    pstSTKReq->ulReceiverPid = ulReceiverPid;
    pstSTKReq->OpId          = OpId;
    pstSTKReq->ClientId      = ClientId;
    pstSTKReq->SatType       = CmdType;
    pstSTKReq->Datalen       = DataLen;

    if(DataLen != 0)
    {
        (VOS_VOID)memcpy_s(pstSTKReq->Data, DataLen, pData, DataLen);
    }

    if(VOS_OK != VOS_SendMsg(ulSendPid, pstSTKReq))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 SI_STK_GetMainMenu(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_GETMAINMNUE, SI_STK_NOCMDDATA,0,VOS_NULL_PTR);
}


VOS_UINT32 SI_STK_GetSTKCommand(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_STK_CMD_TYPE                     CmdType)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_GETCOMMAND, CmdType,0,VOS_NULL_PTR);
}


VOS_UINT32 SI_STK_QuerySTKCommand(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_SC == FEATURE_ON))
    return VOS_ERR;
#else
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_QUERYCOMMAND, SI_STK_NOCMDDATA,0,VOS_NULL_PTR);
#endif
}


VOS_UINT32 SI_STK_DataSendSimple(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_SEND_DATA_TYPE                   SendType,
    VOS_UINT32                          DataLen,
    VOS_UINT8                           *pData)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_SIMPLEDOWN,SendType,DataLen,pData);
}


VOS_UINT32 SI_STK_TerminalResponse(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_STK_TERMINAL_RSP_STRU            *pstTRStru)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_SC == FEATURE_ON))
    return VOS_ERR;
#else

    if(pstTRStru == VOS_NULL_PTR)
    {
        STK_ERROR_LOG("SI_STK_TerminalResponse: The input parameter is null.");

        return VOS_ERR;
    }

    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_TRDOWN, SI_STK_NOCMDDATA,sizeof(SI_STK_TERMINAL_RSP_STRU),(VOS_UINT8*)pstTRStru);
#endif
}



VOS_UINT32 SI_STKDualIMSIChangeReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_SC == FEATURE_ON))
    return TAF_FAILURE;
#else
    SI_STK_REQ_STRU                     *pstSTKReq = VOS_NULL_PTR;

    pstSTKReq = (SI_STK_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_STK_REQ_STRU)-VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR ==pstSTKReq)
    {
        STK_ERROR_LOG("SI_STKDualIMSIChangeReq: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    pstSTKReq->ClientId      = ClientId;
    pstSTKReq->OpId          = OpId;
    pstSTKReq->MsgName       = SI_STK_IMSICHG;
    pstSTKReq->ulReceiverPid = I0_MAPS_STK_PID;

    if(VOS_OK != VOS_SendMsg(I0_MAPS_STK_PID, pstSTKReq))
    {
        STK_ERROR_LOG("SI_STKDualIMSIChangeReq: VOS_SendMsg Return Error");
        return VOS_ERR;
    }

    return VOS_OK;
#endif
}


VOS_UINT32 SI_STKIsDualImsiSupport(VOS_VOID)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_SC == FEATURE_ON))
    return TAF_FAILURE;
#else
    VOS_UINT16 usDualIMSIFlag = 0;

    if ( NV_OK != mdrv_nv_read(en_NV_Item_NV_HUAWEI_DOUBLE_IMSI_CFG_I, &usDualIMSIFlag, sizeof(VOS_UINT16)) )
    {
        STK_WARNING_LOG("STK_InitGobal: Read en_NV_Item_NV_HUAWEI_DOUBLE_IMSI_CFG_I Fail");
    }

    /* ǰ�������Խھ�Ϊ1��Dual IMSI���ܲſ�������һ���ֽ�ΪNV�����־���ڶ���Ϊʹ��λ */
    if ( STK_NV_ENABLED == usDualIMSIFlag )
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
#endif
}



VOS_UINT32 SI_STK_MenuSelection(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_STK_ENVELOPE_STRU                *pstENStru)
{
#if ((1 < MULTI_MODEM_NUMBER) && (!defined(DMT)) && (FEATURE_PHONE_SC == FEATURE_ON))
    return TAF_FAILURE;
#else
    if((VOS_NULL_PTR == pstENStru)||(SI_STK_ENVELOPE_MENUSEL != pstENStru->enEnvelopeType))
    {
        STK_ERROR_LOG("SI_STK_MenuSelection: The Input Data is Error");

        return VOS_ERR;
    }

    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_MENUSELECTION, pstENStru->enEnvelopeType,sizeof(SI_STK_ENVELOPE_STRU),(SI_UINT8*)pstENStru);
#endif
}


VOS_UINT32 SI_STK_SetUpCallConfirm(
    MN_CLIENT_ID_T                      ClientId,
    SI_STK_SETUPCALLCONFIRM_ENUM_UINT32 enAction)
{
    /* ������� */
    if (SI_STK_SETUPCALL_BUTT <= enAction)
    {
        STK_ERROR_LOG("SI_STK_SetUpCallConfirm: The Input Para is Error");

        return VOS_ERR;
    }

    return SI_STK_SendReqMsg(ClientId, 0, SI_STK_SETUPCALL_CONFIRM, SI_STK_SETUPCALL, sizeof(VOS_UINT32), (VOS_UINT8*)&enAction);
}
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (defined(DMT)))

MODULE_EXPORTED VOS_VOID SI_STKGetCurImsiSign(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT16                         *pusDualIMSIEnable,
    VOS_UINT32                         *pulCurImsiSign)
{
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    SI_SAT_SetUpMenu_DATA_STRU         *pstSetUpMenuData = VOS_NULL_PTR;
    SI_STK_IMSICHG_MATCH_STRU           stIMSIMatch = {0};
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;

    if (enModemId >= MODEM_ID_BUTT)
    {
        return;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    *pulCurImsiSign = SI_STK_GetIMSIChgCtrlAddr(enSlotId)->ulCurImsiSign;

    *pusDualIMSIEnable = VOS_FALSE;

    pstSetUpMenuData = SI_STK_GetSetUpMenuDataAddr(enSlotId);

    /* �ж��Ƿ�������˵� */
    if (SI_STK_MENU_EXIST != pstSetUpMenuData->ucUsatTag)
    {
        STK_WARNING_LOG("SI_STKGetCurImsiSign: There is any Main Menu Content");

        return;
    }

    if (NV_OK != SI_NVReadexBySlotId(enSlotId,
                                     en_NV_Item_Stk_DualImsi_Ctrl,
                                     &stIMSIMatch,
                                     sizeof(SI_STK_IMSICHG_MATCH_STRU)))
    {
        STK_WARNING_LOG("SI_STKGetCurImsiSign: Read en_NV_Item_Stk_DualImsi_Ctrl Fail");

        return;
    }

    if (VOS_FALSE == SI_STK_CheckDualSimCtrlNvDataValid(enSlotId, &stIMSIMatch))
    {
        STK_WARNING_LOG("SI_STKGetCurImsiSign: Read en_NV_Item_Stk_DualImsi_Ctrl Success, data invalid");

        return;
    }

    /* �����˵���ƥ�䣬���ƥ��ʧ��pusDualIMSIEnable����ΪDisable��*/
    for (i = 0; i < pstSetUpMenuData->stSetUpMenu.ulItemNum; i++)
    {
        for (j = 0; j < stIMSIMatch.usMatchStrCnt; j++)
        {
            if (VOS_TRUE == SI_STK_StrStr(pstSetUpMenuData->stSetUpMenu.Item[i].pucItemText,
                                          stIMSIMatch.astMatchStr[j].aucMatchStr,
                                          pstSetUpMenuData->stSetUpMenu.Item[i].ucLen,
                                          stIMSIMatch.astMatchStr[j].usStrLen))
            {
                *pusDualIMSIEnable = VOS_TRUE;

                return;
            }
        }
    }

    return;
}


MODULE_EXPORTED VOS_VOID SI_STK_CCResultInd(
    MODEM_ID_ENUM_UINT16                enModemId,
    SI_STK_ENVELOPE_RSP_STRU           *pstRspData)
{
/*lint -e813*/
    SI_STK_EVENT_INFO_STRU              stEvent;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;
    errno_t                             ret;

    if (enModemId >= MODEM_ID_BUTT)
    {
        return;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    (VOS_VOID)memset_s(&stEvent, sizeof(stEvent), 0, sizeof(stEvent));

    stEvent.STKCBEvent                                  = SI_STK_CC_RESULT_IND_EVENT;
    stEvent.STKErrorNo                                  = VOS_OK;
    stEvent.STKCmdStru.CmdStru.STKCcIndInfo.ucResult    = (SI_UINT8)pstRspData->Result;

    if (USSD_STRING_TAG == (pstRspData->uResp.CallCtrlRsp.SpecialData.ucTag & 0x7F))
    {
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.ucType = SI_STK_USSD_CALL_CTRL;
    }
    else if (SS_STRING_TAG == (pstRspData->uResp.CallCtrlRsp.SpecialData.ucTag & 0x7F))
    {
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.ucType = SI_STK_SS_CALL_CTRL;
    }
    else if (ADDRESS_TAG == (pstRspData->uResp.CallCtrlRsp.SpecialData.ucTag & 0x7F))
    {
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.ucType = SI_STK_MO_CALL_CTRL;
    }
    else
    {
        return;
    }

    /* COPY aplhaid �ֶ� */
    if (VOS_TRUE == pstRspData->uResp.CallCtrlRsp.OP_Alaph)
    {
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.ulAlphaLen
                = (pstRspData->uResp.CallCtrlRsp.AlphaId.ulLen > SI_STK_DATA_MAX_LEN) ? SI_STK_DATA_MAX_LEN : pstRspData->uResp.CallCtrlRsp.AlphaId.ulLen;

        ret = memcpy_s(stEvent.STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.aucAlphaId,
                     SI_STK_DATA_MAX_LEN,
                     pstRspData->uResp.CallCtrlRsp.AlphaId.pucAlphabet,
                     stEvent.STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.ulAlphaLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    if (VOS_TRUE == pstRspData->uResp.CallCtrlRsp.OP_SepcialData)
    {
        /* ���������ֶ� */
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.ucDataType
                                                       = pstRspData->uResp.CallCtrlRsp.SpecialData.pValue[0];

        if (pstRspData->uResp.CallCtrlRsp.SpecialData.ucLen > 1)
        {
            /* �����ֶ�copy */
            stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.usDataLen
                = pstRspData->uResp.CallCtrlRsp.SpecialData.ucLen - 1;

            ret = memcpy_s(stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.aucData,
                           SI_STK_DATA_MAX_LEN,
                           &pstRspData->uResp.CallCtrlRsp.SpecialData.pValue[1],
                           pstRspData->uResp.CallCtrlRsp.SpecialData.ucLen - 1);

            PAM_PRINT_SECFUN_RESULT(ret);
        }
    }

    SI_STKCallBack_BroadCast(enSlotId, &stEvent);
/*lint +e813*/
    return;
}


MODULE_EXPORTED VOS_VOID SI_STK_SMSCtrlResultInd(
    MODEM_ID_ENUM_UINT16                enModemId,
    SI_STK_ENVELOPE_RSP_STRU           *pstRspData)
{
    //��Ҫȷ��addr1��Ŀ�ĵ�ַ����addr2
/*lint -e813*/
    SI_STK_EVENT_INFO_STRU              stEvent;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;
    errno_t                             ret;

    if (enModemId >= MODEM_ID_BUTT)
    {
        return;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    (VOS_VOID)memset_s(&stEvent, sizeof(stEvent), 0, sizeof(stEvent));

    stEvent.STKCBEvent                                  = SI_STK_SMSCTRL_RESULT_IND_EVENT;
    stEvent.STKErrorNo                                  = VOS_OK;
    stEvent.STKCmdStru.CmdStru.STKCcIndInfo.ucResult    = (SI_UINT8)pstRspData->Result;
    stEvent.STKCmdStru.CmdStru.STKCcIndInfo.ucType      = SI_STK_SMS_CTRL;


    /* COPY aplhaid �ֶ� */
    if (VOS_TRUE == pstRspData->uResp.MoSmsCtrlRsp.OP_Alaph)
    {
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.ulAlphaLen
               = (pstRspData->uResp.MoSmsCtrlRsp.AlphaId.ulLen > SI_STK_DATA_MAX_LEN) ? SI_STK_DATA_MAX_LEN : pstRspData->uResp.MoSmsCtrlRsp.AlphaId.ulLen;

        ret = memcpy_s(stEvent.STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.aucAlphaId,
                     SI_STK_DATA_MAX_LEN,
                     pstRspData->uResp.MoSmsCtrlRsp.AlphaId.pucAlphabet,
                     stEvent.STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.ulAlphaLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    if (VOS_TRUE == pstRspData->uResp.MoSmsCtrlRsp.OP_Addr1)
    {
        /* ����Ŀ�ĵ�ַ�����ֶ� */
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stDstAddrInfo.ucNumType
                                                            = pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucNumType;
        /* copyĿ�ĵ�ַ */
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stDstAddrInfo.ucAddrLen
                                                            = pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen;

        ret = memcpy_s(stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stDstAddrInfo.aucAddr,
                     SI_STK_ADDR_MAX_LEN,
                     pstRspData->uResp.MoSmsCtrlRsp.Addr1.pucAddr,
                     pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    if (VOS_TRUE == pstRspData->uResp.MoSmsCtrlRsp.OP_Addr2)
    {
        /* �����������ĵ�ַ�����ֶ� */
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stSerCenterAddrInfo.ucNumType
                                                          = pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucNumType;

        /* copy�������ĺ��� */
        stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stSerCenterAddrInfo.ucAddrLen
                                                            =pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen;

        ret = memcpy_s(stEvent.STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stSerCenterAddrInfo.aucAddr,
                     SI_STK_ADDR_MAX_LEN,
                     pstRspData->uResp.MoSmsCtrlRsp.Addr2.pucAddr,
                     pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen);

        PAM_PRINT_SECFUN_RESULT(ret);
    }

    SI_STKCallBack_BroadCast(enSlotId, &stEvent);
/*lint +e813*/
    return;
}


VOS_VOID SI_STK_DecodeCCRspSpecial(
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId,
    VOS_UINT32                          ulDataLen,
    VOS_UINT8                           *pucCmdData,
    VOS_UINT32                          ulSpareBuffSize,
    SI_STK_ENVELOPE_RSP_STRU            *pstRspData)
{
    VOS_UINT8   aucTagList[]={ADDRESS_TAG,SS_STRING_TAG,USSD_STRING_TAG,PDP_CONTEXT_ACTIVATION_PAR_TAG,EPSPDN_ACTIVE_PARA_TAG};
    VOS_UINT32  i;
    VOS_UINT32  ulCCTagOffset;
    VOS_PID     ulStkPid;
    errno_t     ret;

    ulStkPid = USIMM_GetPidBySlotId(I0_MAPS_STK_PID, enSlotId);

    for(i=0; i<sizeof(aucTagList); i++)
    {
        ulCCTagOffset = SI_STKFindTag(aucTagList[i], pucCmdData, ulSpareBuffSize, ulDataLen, 1);

        if(ulCCTagOffset != SI_TAGNOTFOUND)
        {
            pstRspData->uResp.CallCtrlRsp.SpecialData.ucTag = pucCmdData[ulCCTagOffset-1] & 0x7F;
            pstRspData->uResp.CallCtrlRsp.SpecialData.ucLen = pucCmdData[ulCCTagOffset];

            if (pstRspData->uResp.CallCtrlRsp.SpecialData.ucLen > 0)
            {
                pstRspData->uResp.CallCtrlRsp.SpecialData.pValue
                    = (SI_UINT8 *)VOS_MemAlloc(ulStkPid, DYNAMIC_MEM_PT, pucCmdData[ulCCTagOffset]);

                if(VOS_NULL_PTR != pstRspData->uResp.CallCtrlRsp.SpecialData.pValue)
                {
                    ret = memcpy_s(pstRspData->uResp.CallCtrlRsp.SpecialData.pValue,
                                   pucCmdData[ulCCTagOffset],
                                   &pucCmdData[ulCCTagOffset + 1],
                                   pucCmdData[ulCCTagOffset]);

                    PAM_PRINT_SECFUN_RESULT(ret);

                    pstRspData->uResp.CallCtrlRsp.OP_SepcialData = 1;
                }
            }

            return;
        }
    }

    return;
}


VOS_VOID SI_STK_DecodeMoSmsRspSpecial(
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId,
    VOS_UINT32                          ulDataLen,
    VOS_UINT8                           *pucCmdData,
    VOS_UINT32                          ulSpareBuffSize,
    SI_STK_ENVELOPE_RSP_STRU            *pstRspData)
{
    VOS_UINT32  ulTagOffset;
    VOS_PID     ulStkPid;

    ulStkPid = USIMM_GetPidBySlotId(I0_MAPS_STK_PID, enSlotId);

    pstRspData->uResp.MoSmsCtrlRsp.OP_Addr1 = 0;
    pstRspData->uResp.MoSmsCtrlRsp.OP_Addr2 = 0;

    ulTagOffset = SI_STKFindTag(ADDRESS_TAG, pucCmdData, ulSpareBuffSize, ulDataLen, 1);

    if(ulTagOffset != SI_TAGNOTFOUND)
    {
        pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen = 0;
        pstRspData->uResp.MoSmsCtrlRsp.Addr1.pucAddr = VOS_NULL_PTR;

        if (pucCmdData[ulTagOffset] > 1)
        {
            pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen = pucCmdData[ulTagOffset] - 1;
            pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucNumType = pucCmdData[ulTagOffset+1];
            pstRspData->uResp.MoSmsCtrlRsp.Addr1.pucAddr
                = (SI_UINT8*)VOS_MemAlloc(ulStkPid, DYNAMIC_MEM_PT, pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen);

            if(VOS_NULL_PTR != pstRspData->uResp.MoSmsCtrlRsp.Addr1.pucAddr)
            {
                (VOS_VOID)memcpy_s(pstRspData->uResp.MoSmsCtrlRsp.Addr1.pucAddr,
                             pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen,
                             &pucCmdData[ulTagOffset+2],
                             pstRspData->uResp.MoSmsCtrlRsp.Addr1.ucLen);

                pstRspData->uResp.MoSmsCtrlRsp.OP_Addr1 = 1;
            }
        }
    }

    ulTagOffset = SI_STKFindTag(ADDRESS_TAG, pucCmdData, ulSpareBuffSize, ulDataLen, 2);

    if(ulTagOffset != SI_TAGNOTFOUND)
    {
        pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen = 0;
        pstRspData->uResp.MoSmsCtrlRsp.Addr2.pucAddr = VOS_NULL_PTR;

        if (pucCmdData[ulTagOffset] > 1)
        {
            pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen = pucCmdData[ulTagOffset] - 1;
            pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucNumType = pucCmdData[ulTagOffset+1];
            pstRspData->uResp.MoSmsCtrlRsp.Addr2.pucAddr
                = (SI_UINT8*)VOS_MemAlloc(ulStkPid, DYNAMIC_MEM_PT, pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen);

            if(VOS_NULL_PTR != pstRspData->uResp.MoSmsCtrlRsp.Addr2.pucAddr)
            {
                (VOS_VOID)memcpy_s(pstRspData->uResp.MoSmsCtrlRsp.Addr2.pucAddr,
                             pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen,
                             &pucCmdData[ulTagOffset+2],
                             pstRspData->uResp.MoSmsCtrlRsp.Addr2.ucLen);

                pstRspData->uResp.MoSmsCtrlRsp.OP_Addr2 = 1;
            }
        }
    }

    return;
}



MODULE_EXPORTED VOS_UINT32 SI_STK_EnvelopeRsp_Decode(
    MODEM_ID_ENUM_UINT16                enModemId,
    SI_STK_ENVELOPE_TYPE_UINT32         enDataType,
    VOS_UINT32                          ulDataLen,
    VOS_UINT8                           *pucCmdData,
    SI_STK_ENVELOPE_RSP_STRU            *pstRspData)
{
    VOS_UINT32  i;
    VOS_UINT32  ulOffset;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;

    if (enModemId >= MODEM_ID_BUTT)
    {
        return VOS_ERR;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    if((pucCmdData == VOS_NULL_PTR)||(pstRspData == VOS_NULL_PTR))
    {
        STK_ERROR_LOG("SI_STKCommCodeData: The Input Parameter is Error");

        return VOS_ERR;
    }

    pstRspData->EnvelopeType    = enDataType;
    pstRspData->Result          = pucCmdData[0];

    if(pucCmdData[0] > 0x80)/*ȷ����������ĳ����ֽ�*/
    {
        ulOffset = 0x02;
    }
    else
    {
        ulOffset = 0x01;
    }

    if ((ulOffset + 1) >= ulDataLen)
    {
        return VOS_ERR;
    }

    for(i=0; i<ARRAYSIZE(gastEnvelopeDecodeList); i++)
    {
        if(enDataType == gastEnvelopeDecodeList[i].ulCmdType)
        {
            SI_STKDecodeTagList(enSlotId,
                                &pucCmdData[ulOffset],
                                ulDataLen - ulOffset,
                                gastEnvelopeDecodeList[i].pucTagList,
                                gastEnvelopeDecodeList[i].ulTagLen,
                                (SI_SAT_COMMDATA_STRU *)pstRspData);

            break;
        }
    }

    if(i >= ARRAYSIZE(gastEnvelopeDecodeList))
    {
        STK_ERROR_LOG("SI_STKCommCodeData: The Input enDataType is Error");

        return VOS_ERR;
    }

    if(enDataType == SI_STK_ENVELOPE_CALLCRTL)
    {
        SI_STK_DecodeCCRspSpecial(enSlotId, (VOS_UINT32)pucCmdData[ulOffset], &pucCmdData[ulOffset+1], ulDataLen-(ulOffset+1), pstRspData);
    }

    if(enDataType == SI_STK_ENVELOPE_SMSCRTL)
    {
        SI_STK_DecodeMoSmsRspSpecial(enSlotId, (VOS_UINT32)pucCmdData[ulOffset], &pucCmdData[ulOffset+1], ulDataLen-(ulOffset+1), pstRspData);
    }

    return VOS_OK;
}


VOS_UINT32 SI_STK_EnvelopeData_Code_ParaCheck(
    MODEM_ID_ENUM_UINT16                enModemId,
    SI_STK_ENVELOPE_STRU               *pstENStru,
    VOS_UINT32                         *pulDataLen,
    USIMM_U8_LVDATA_STRU               *eventData,
    VOS_UINT32                         *pulDataOffset)
{
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;

    if (enModemId >= MODEM_ID_BUTT)
    {
        return VOS_ERR;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    if((pstENStru               == VOS_NULL_PTR)
     ||(pulDataLen              == VOS_NULL_PTR)
     ||(eventData               == VOS_NULL_PTR)
     ||(eventData->pucData      == VOS_NULL_PTR)
     ||(eventData->ulDataLen    <= 2)
     ||(pulDataOffset           == VOS_NULL_PTR))
    {
        STK_ERROR_LOG("SI_STK_EnvelopeData_Code: The Input Data is Error");

        return VOS_ERR;
    }

    return VOS_OK;
}


MODULE_EXPORTED VOS_UINT32 SI_STK_EnvelopeData_Code(
    MODEM_ID_ENUM_UINT16                enModemId,
    SI_STK_ENVELOPE_STRU               *pstENStru,
    VOS_UINT32                         *pulDataLen,
    USIMM_U8_LVDATA_STRU               *eventData,
    VOS_UINT32                         *pulDataOffset)
{
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;
    USIMM_U8_LVDATA_STRU                cmdData;
    errno_t                             ret;
    VOS_UINT32                          totalLen;

    if (SI_STK_EnvelopeData_Code_ParaCheck(enModemId,
                                           pstENStru,
                                           pulDataLen,
                                           eventData,
                                           pulDataOffset) != VOS_OK)
    {
        return VOS_ERR;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    SI_STK_InitEnvelope(eventData->pucData, eventData->ulDataLen, &pstENStru->DeviceId, (VOS_UINT8)pstENStru->enEnvelopeType);

    if (pstENStru->enEnvelopeType == SI_STK_ENVELOPE_CALLCRTL)
    {
        if(pstENStru->uEnvelope.CallCtrl.OP_SepcialData != 0)
        {
            if (eventData->ulDataLen <= 9)
            {
                STK_ERROR_LOG1("SI_STK_EnvelopeData_Code: ulDataLen  err", eventData->ulDataLen);
                return VOS_ERR;
            }

            totalLen = ((VOS_UINT32)pstENStru->uEnvelope.CallCtrl.SpecialData.ucLen + eventData->pucData[2] + 2);

            if (totalLen > USIMM_APDU_DATA_MAXLEN)
            {
                STK_ERROR_LOG1("SI_STK_EnvelopeData_Code: totalLen  err", totalLen);

                return VOS_ERR;
            }

            eventData->pucData[2] += pstENStru->uEnvelope.CallCtrl.SpecialData.ucLen+2;
            eventData->pucData[7] = pstENStru->uEnvelope.CallCtrl.SpecialData.ucTag;
            eventData->pucData[8] = pstENStru->uEnvelope.CallCtrl.SpecialData.ucLen;

            /* ���ø�API�ĺ����������pucData����Ϊ256 */
            ret = memcpy_s(&eventData->pucData[9],
                           (eventData->ulDataLen - 9),
                           pstENStru->uEnvelope.CallCtrl.SpecialData.pValue,
                           pstENStru->uEnvelope.CallCtrl.SpecialData.ucLen);

            if (ret != EOK)
            {
                STK_ERROR_LOG1("SI_STK_EnvelopeData_Code: memcpy_s  errno", (VOS_UINT32)ret);

                return VOS_ERR;
            }
        }
    }

    cmdData.ulDataLen = eventData->ulDataLen - 1;
    cmdData.pucData = &eventData->pucData[1];

    if(SI_STKCommCodeData(enSlotId,
                         &cmdData,
                          SI_CODE_ENVELOPE_DATA,
                          pstENStru->enEnvelopeType,
                          (SI_SAT_COMMDATA_STRU*)pstENStru) != VOS_OK)
    {
        STK_ERROR_LOG("SI_STK_EnvelopeData_Code: The Code Data is Error");

        return VOS_ERR;
    }

    if(eventData->pucData[2] > 0x7F)
    {
        eventData->pucData[1] = 0x81;

        *pulDataLen = eventData->pucData[2] + 3;

        *pulDataOffset = 0;
    }
    else
    {
        *pulDataLen   = eventData->pucData[2] + 2;

        *pulDataOffset = 1;
    }

    return VOS_OK;
}


MODULE_EXPORTED VOS_VOID SI_STK_EnvelopeRspDataFree(
    MODEM_ID_ENUM_UINT16                     enModemId,
    SI_STK_ENVELOPE_RSP_STRU                *pstData)
{
    VOS_UINT32 i;
    VOS_UINT32 j;
    VOS_UINT32 ulOffset = 0;
    VOS_UINT8                          *pData = VOS_NULL_PTR;
    VOS_PID    ulStkPid;
    SI_PIH_CARD_SLOT_ENUM_UINT32        enSlotId;

    if (enModemId >= MODEM_ID_BUTT)
    {
        return;
    }

    enSlotId = USIMM_GetSlotIdByModemId(enModemId);

    ulStkPid = USIMM_GetPidBySlotId(I0_MAPS_STK_PID, enSlotId);

    if(VOS_NULL_PTR == pstData)
    {
        STK_ERROR_LOG("SI_STK_EnvelopeRspDataFree: The Input Parameter is Error");

        return ;
    }

    pData = (VOS_UINT8*)&(pstData->uResp)+sizeof(VOS_UINT32);

    for(i=0; i<STK_ARRAYSIZE(gastEnvelopeDecodeList); i++)
    {
        if(pstData->EnvelopeType == gastEnvelopeDecodeList[i].ulCmdType)
        {
            for(j=0; j<gastEnvelopeDecodeList[i].ulTagLen; j++)
            {
                ulOffset = SI_STKTagDataFree(enSlotId, *(gastEnvelopeDecodeList[i].pucTagList+j), pData);

                pData += ulOffset;
            }

            break;
        }
    }

    if((pstData->EnvelopeType == SI_STK_ENVELOPE_CALLCRTL)
      &&(pstData->uResp.CallCtrlRsp.OP_SepcialData != 0))
    {
        (VOS_VOID)VOS_MemFree(ulStkPid, pstData->uResp.CallCtrlRsp.SpecialData.pValue);
    }

    if(pstData->EnvelopeType == SI_STK_ENVELOPE_SMSCRTL)
    {
        if(pstData->uResp.MoSmsCtrlRsp.OP_Addr1 != 0)
        {
            (VOS_VOID)VOS_MemFree(ulStkPid, pstData->uResp.MoSmsCtrlRsp.Addr1.pucAddr);
        }

        if(pstData->uResp.MoSmsCtrlRsp.OP_Addr2 != 0)
        {
            (VOS_VOID)VOS_MemFree(ulStkPid, pstData->uResp.MoSmsCtrlRsp.Addr2.pucAddr);
        }
    }

    return;
}

#endif



