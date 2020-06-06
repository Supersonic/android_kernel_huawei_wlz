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
#include "AtCmdMiscProc.h"
#include "AtSndMsg.h"
#include "ATCmdProc.h"
#include "dms_core.h"
#include "AtDataProc.h"
#include "CssAtInterface.h"

#include "AtInit.h"

#include "TafDrvAgent.h"
#include "securec.h"


extern TAF_MMA_USER_SET_PRIO_RAT_ENUM_U8 AT_GetSysCfgPrioRat(
    TAF_MMA_SYS_CFG_PARA_STRU          *pstSysCfgExSetPara
);
LOCAL VOS_VOID AT_ProcDlMcsRsp(
    MTA_AT_MCS_QRY_CNF_STRU            *pstMtaAtQryMcsCnf,
    VOS_UINT8                           ucIndex,
    VOS_UINT16                         *pusLength
);

LOCAL VOS_VOID AT_ProcUlMcsRsp(
    MTA_AT_MCS_QRY_CNF_STRU            *pstMtaAtQryMcsCnf,
    VOS_UINT8                           ucIndex,
    VOS_UINT16                         *pusLength
);
/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_MISC_PROC_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#if(FEATURE_ON == FEATURE_UE_MODE_NR)
extern AT_MT_INFO_STRU                         g_stMtInfoCtx;
#else
extern AT_DEVICE_CMD_CTRL_STRU                 g_stAtDevCmdCtrl;
#endif

#define  MCS_RAT_LTE                    0
#define  MCS_RAT_NR                     1
#define  HFREQ_INFO_RAT_LTE             6
#define  HFREQ_INFO_RAT_NR              7

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


VOS_UINT32 AT_SetActiveModem(VOS_UINT8 ucIndex)
{
    TAF_NV_DSDS_ACTIVE_MODEM_MODE_STRU  stMode;
    AT_MTA_MODEM_CAP_UPDATE_REQ_STRU    stAtMtaModemCapUpdate;

    memset_s(&stAtMtaModemCapUpdate, (VOS_UINT32)sizeof(AT_MTA_MODEM_CAP_UPDATE_REQ_STRU), 0x00, (VOS_UINT32)sizeof(AT_MTA_MODEM_CAP_UPDATE_REQ_STRU));

    /* ����������� */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stMode, sizeof(stMode), 0x00, sizeof(stMode));
    stMode.enActiveModem = (TAF_NV_ACTIVE_MODEM_MODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* дNV, ����AT_OK */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, en_NV_Item_DSDS_Active_Modem_Mode, &stMode, sizeof(stMode)) != NV_OK)
    {
        AT_ERR_LOG("AT_SetActiveModem(): en_NV_Item_DSDS_Active_Modem_Mode NV Write Fail!");
        return AT_ERROR;
    }

    /* ������NV��֪ͨAT->MTA->RRM�����еײ�ƽ̨�������� */
    /* AT������MTA����Ϣ�ṹ��ֵ */
    stAtMtaModemCapUpdate.enModemCapUpdateType = AT_MTA_MODEM_CAP_UPDATE_TYPE_ACTIVE_MODEM;

    /* ������Ϣ��C�˴��� */
    if (AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                             0,
                                             ID_AT_MTA_MODEM_CAP_UPDATE_REQ,
                                             &stAtMtaModemCapUpdate,
                                             (VOS_UINT32)sizeof(AT_MTA_MODEM_CAP_UPDATE_REQ_STRU),
                                             I0_UEPS_PID_MTA) != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_SetActiveModem(): Snd MTA Req Failed!");

        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MODEM_CAP_UPDATE_SET;

    return AT_WAIT_ASYNC_RETURN;
}


#if(FEATURE_ON == FEATURE_LTE)
#if(FEATURE_ON == FEATURE_LTE_MBMS)

VOS_UINT32 AT_SetMBMSServiceOptPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_SERVICE_OPTION_SET_REQ_STRU         stMBMSServiceOption;
    VOS_UINT32                                      ulRst;

    memset_s(&stMBMSServiceOption, sizeof(stMBMSServiceOption), 0x00, sizeof(AT_MTA_MBMS_SERVICE_OPTION_SET_REQ_STRU));

    /* ����Ϊ�� */
    if(gastAtParaList[1].usParaLen != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = atAuc2ul(gastAtParaList[1].aucPara,
                     (VOS_UINT16)gastAtParaList[1].usParaLen,
                     &gastAtParaList[1].ulParaValue);

    if(ulRst != AT_SUCCESS)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[1].ulParaValue)
    {
        case 0:
            stMBMSServiceOption.enCfg = AT_MTA_CFG_DISABLE;
            break;

        case 1:
            stMBMSServiceOption.enCfg = AT_MTA_CFG_ENABLE;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϳ����Ϣ��C��, ����ʹ�ܻ���ȥʹ��MBMS�������� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_SERVICE_OPTION_SET_REQ,
                                   &stMBMSServiceOption,
                                   sizeof(stMBMSServiceOption),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMBMSServicePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_SERVICE_OPTION_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetMBMSServiceStatePara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_SERVICE_STATE_SET_REQ_STRU          stMBMSServiceState;
    VOS_UINT32                                      ulRst;

    memset_s(&stMBMSServiceState, sizeof(stMBMSServiceState), 0x00, sizeof(AT_MTA_MBMS_SERVICE_STATE_SET_REQ_STRU));

    switch(gastAtParaList[0].ulParaValue)
    {
        case AT_MBMS_ACTIVATE_TYPE:
            stMBMSServiceState.enStateSet   = AT_MTA_MBMS_SERVICE_STATE_SET_ACTIVE;
            break;

        case AT_MBMS_DEACTIVATE_TYPE:
            stMBMSServiceState.enStateSet   = AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE;
            break;

        case AT_MBMS_DEACTIVATE_ALL_TYPE:
            stMBMSServiceState.enStateSet   = AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE_ALL;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    if(stMBMSServiceState.enStateSet != AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE_ALL)
    {
        /* ����Ϊ�� */
        if(gastAtParaList[1].usParaLen == 0 || gastAtParaList[2].usParaLen == 0)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        ulRst = atAuc2ul(gastAtParaList[1].aucPara,
                         (VOS_UINT16)gastAtParaList[1].usParaLen,
                         &gastAtParaList[1].ulParaValue);

        if(ulRst != AT_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* �������Ȳ���ȷ */
        if(((AT_MBMS_TMGI_MAX_LENGTH - 1 ) != gastAtParaList[2].usParaLen)
        && (gastAtParaList[2].usParaLen != AT_MBMS_TMGI_MAX_LENGTH))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <AreaID> */
        stMBMSServiceState.ulAreaId                 = gastAtParaList[1].ulParaValue;

        /* <TMGI>:MBMS Service ID */
        if(At_Auc2ul(gastAtParaList[2].aucPara, AT_MBMS_SERVICE_ID_LENGTH, &stMBMSServiceState.stTMGI.ulMbmsSerId) == AT_FAILURE)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <TMGI>:Mcc */
        if(At_String2Hex(&gastAtParaList[2].aucPara[AT_MBMS_SERVICE_ID_LENGTH], AT_MBMS_MCC_LENGTH, &stMBMSServiceState.stTMGI.stPlmnId.ulMcc) == AT_FAILURE)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <TMGI>:Mnc */
        if(At_String2Hex(&gastAtParaList[2].aucPara[AT_MBMS_SERVICE_ID_AND_MCC_LENGTH], gastAtParaList[2].usParaLen - AT_MBMS_SERVICE_ID_AND_MCC_LENGTH, &stMBMSServiceState.stTMGI.stPlmnId.ulMnc) == AT_FAILURE)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ( (AT_MBMS_TMGI_MAX_LENGTH - 1 ) == gastAtParaList[2].usParaLen)
        {
            stMBMSServiceState.stTMGI.stPlmnId.ulMnc |= 0x0F00;
        }
    }

    /* ���Ϳ����Ϣ��C��, ����ʹ�ܻ���ȥʹ��MBMS�������� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_SERVICE_STATE_SET_REQ,
                                   &stMBMSServiceState,
                                   sizeof(stMBMSServiceState),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMBMSServiceStatePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_SERVICE_STATE_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetMBMSPreferencePara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_PREFERENCE_SET_REQ_STRU             stMBMSCastMode;
    VOS_UINT32                                      ulRst;

    memset_s(&stMBMSCastMode, sizeof(stMBMSCastMode), 0x00, sizeof(AT_MTA_MBMS_PREFERENCE_SET_REQ_STRU));

    /* ����Ϊ�� */
    if(gastAtParaList[1].usParaLen != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = atAuc2ul(gastAtParaList[1].aucPara, (VOS_UINT16)gastAtParaList[1].usParaLen, &gastAtParaList[1].ulParaValue);

    if(ulRst != AT_SUCCESS)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[1].ulParaValue)
    {
        case 0:
            stMBMSCastMode.enCastMode   = AT_MTA_MBMS_CAST_MODE_UNICAST;
            break;

        case 1:
            stMBMSCastMode.enCastMode   = AT_MTA_MBMS_CAST_MODE_MULTICAST;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϳ����Ϣ��C��, ����MBMS�㲥ģʽ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_PREFERENCE_SET_REQ,
                                   &stMBMSCastMode,
                                   sizeof(stMBMSCastMode),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMBMSPreferencePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_PREFERENCE_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetMBMSCMDPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 3)
    {
        return AT_TOO_MANY_PARA;
    }

    /* ����Ϊ�� */
    if(gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����<cmd>*/
    switch(gastAtParaList[0].ulParaValue)
    {
        /* <cmd> equal "MBMS_SERVICE_ENABLER" */
        case AT_MBMS_SERVICE_ENABLER_TYPE:
            ulRst = AT_SetMBMSServiceOptPara(ucIndex);
            break;
        /* <cmd> equal "ACTIVATE", "DEACTIVATE" or "DEACTIVATE_ALL" */
        case AT_MBMS_ACTIVATE_TYPE:
        case AT_MBMS_DEACTIVATE_TYPE:
        case AT_MBMS_DEACTIVATE_ALL_TYPE:
            ulRst = AT_SetMBMSServiceStatePara(ucIndex);
            break;
        /* <cmd> equal "MBMS_PREFERENCE" */
        case AT_MBMS_PREFERENCE_TYPE:
            ulRst = AT_SetMBMSPreferencePara(ucIndex);
            break;
        /* <cmd> equal "SIB16_GET_NETWORK_TIME" */
        case AT_MBMS_SIB16_GET_NETWORK_TIME_TYPE:
            ulRst = AT_QryMBMSSib16NetworkTimePara(ucIndex);
            break;
        /* <cmd> equal "BSSI_SIGNAL_LEVEL" */
        case AT_MBMS_BSSI_SIGNAL_LEVEL_TYPE:
            ulRst = AT_QryMBMSBssiSignalLevelPara(ucIndex);
            break;
        /* <cmd> equal "NETWORK_INFORMATION" */
        case AT_MBMS_NETWORK_INFORMATION_TYPE:
            ulRst = AT_QryMBMSNetworkInfoPara(ucIndex);
            break;
        /* <cmd> equal "MODEM_STATUS" */
        case AT_MBMS_MODEM_STATUS_TYPE:
            ulRst = AT_QryMBMSModemStatusPara(ucIndex);
            break;

        default:
            ulRst = AT_CME_INCORRECT_PARAMETERS;
            break;
    }

    return ulRst;
}


VOS_UINT32 AT_SetMBMSEVPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ_STRU        stMBMSUnsolicitedCfg;
    VOS_UINT32                                      ulRst;

    memset_s(&stMBMSUnsolicitedCfg, sizeof(stMBMSUnsolicitedCfg), 0x00, sizeof(AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if(gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[0].ulParaValue)
    {
        case 0:
            stMBMSUnsolicitedCfg.enCfg  = AT_MTA_CFG_DISABLE;
            break;

        case 1:
            stMBMSUnsolicitedCfg.enCfg  = AT_MTA_CFG_ENABLE;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϳ����Ϣ��C��, ����MBMS�����ϱ����� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ,
                                   &stMBMSUnsolicitedCfg,
                                   sizeof(stMBMSUnsolicitedCfg),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMBMSEVPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_UNSOLICITED_CFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetMBMSInterestListPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_INTERESTLIST_SET_REQ_STRU           stMBMSInterestList;
    VOS_UINT32                                      ulRst;
    VOS_UINT8                                       interestNum;

    memset_s(&stMBMSInterestList, sizeof(stMBMSInterestList), 0x00, sizeof(AT_MTA_MBMS_INTERESTLIST_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if (gucAtParaIndex != 6)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if(gastAtParaList[5].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[5].ulParaValue)
    {
        case 0:
            stMBMSInterestList.enMbmsPriority    = AT_MTA_MBMS_PRIORITY_UNICAST;
            break;

        case 1:
            stMBMSInterestList.enMbmsPriority    = AT_MTA_MBMS_PRIORITY_MBMS;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* Ƶ���б�ֵ */
    for(interestNum = 0; interestNum < AT_MTA_INTEREST_FREQ_MAX_NUM; interestNum++)
    {
        if (gastAtParaList[interestNum].usParaLen != 0)
        {
            ulRst = atAuc2ul(gastAtParaList[interestNum].aucPara,
                             (VOS_UINT16)gastAtParaList[interestNum].usParaLen,
                             &gastAtParaList[interestNum].ulParaValue);

            if(ulRst != AT_SUCCESS)
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            stMBMSInterestList.aulFreqList[interestNum]   =   gastAtParaList[interestNum].ulParaValue;
        }
        else
        {
            /* Ĭ��ֵΪ��Чֵ0xFFFFFFFF */
            stMBMSInterestList.aulFreqList[interestNum]   =   0xFFFFFFFF;
        }
    }

    /* ���Ϳ����Ϣ��C��, ����MBMS�����ϱ����� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_INTEREST_LIST_SET_REQ,
                                   &stMBMSInterestList,
                                   sizeof(stMBMSInterestList),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMBMSInterestListPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_INTERESTLIST_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryMBMSSib16NetworkTimePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* ���Ϳ����Ϣ��C��, ��ѯSIB16����ʱ������ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SIB16_NETWORK_TIME_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryMBMSSib16NetworkTimePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryMBMSBssiSignalLevelPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* ���Ϳ����Ϣ��C��, ��ѯBSSI�ź�ǿ������ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_BSSI_SIGNAL_LEVEL_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryMBMSBssiSignalLevelPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryMBMSNetworkInfoPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* ���Ϳ����Ϣ��C��, ��ѯ������Ϣ���� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_NETWORK_INFO_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryMBMSNetworkInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_NETWORK_INFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryMBMSModemStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* ���Ϳ����Ϣ��C��, ��ѯeMBMS����״̬���� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_EMBMS_STATUS_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryMBMSModemStatusPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMBMS_STATUS_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryMBMSCmdPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* AT ��MTA ���Ͳ�ѯ������Ϣ */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MBMS_AVL_SERVICE_LIST_QRY_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryMBMSCmdPara: send Msg fail.");
        return AT_ERROR;
    }

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_AVL_SERVICE_LIST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}



VOS_UINT32 At_TestMBMSCMDPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: (\"MBMS_SERVICE_ENABLER\",\"ACTIVATE\",\"DEACTIVATE\",\"DEACTIVATE_ALL\",\"MBMS_PREFERENCE\",\"SIB16_GET_NETWORK_TIME\",\"BSSI_SIGNAL_LEVEL\",\"NETWORK_INFORMATION\",\"MODEM_STATUS\")",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}



VOS_UINT32 AT_RcvMtaMBMSServiceOptSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceOptSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceOptSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_SERVICE_OPTION_SET)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceOptSetCnf : Current Option is not AT_CMD_MBMS_SERVICE_OPTION_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSServiceStateSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceStateSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceStateSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_SERVICE_STATE_SET)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceStateSetCnf : Current Option is not AT_CMD_MBMS_SERVICE_STATE_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSPreferenceSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSPreferenceSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSPreferenceSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_PREFERENCE_SET)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSPreferenceSetCnf : Current Option is not AT_CMD_MBMS_PREFERENCE_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSSib16NetworkTimeQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg   = VOS_NULL_PTR;
    MTA_AT_MBMS_SIB16_NETWORK_TIME_QRY_CNF_STRU    *pstMtaCnf = VOS_NULL_PTR;
    VOS_UINT64                                     *pullUTC   = VOS_NULL_PTR;
    VOS_UINT8                                       aucUTC[AT_MBMS_UTC_MAX_LENGTH + 1];
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_SIB16_NETWORK_TIME_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;
    memset_s(aucUTC, sizeof(aucUTC), 0x00, sizeof(aucUTC));

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSSib16NetworkTimeQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSSib16NetworkTimeQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSSib16NetworkTimeQryCnf : Current Option is not AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        pullUTC = (VOS_UINT64 *)pstMtaCnf->aulUTC;
        VOS_sprintf_s((VOS_CHAR *)aucUTC, AT_MBMS_UTC_MAX_LENGTH+1, "%llu", *pullUTC);
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    aucUTC);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSBssiSignalLevelQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_BSSI_SIGNAL_LEVEL_QRY_CNF_STRU     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_BSSI_SIGNAL_LEVEL_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSBssiSignalLevelQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSBssiSignalLevelQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSBssiSignalLevelQryCnf : Current Option is not AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (VOS_INT32)pstMtaCnf->ucBSSILevel);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSNetworkInfoQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_NETWORK_INFO_QRY_CNF_STRU          *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_NETWORK_INFO_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSNetworkInfoQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSNetworkInfoQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_NETWORK_INFO_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSNetworkInfoQryCnf : Current Option is not AT_CMD_MBMS_NETWORK_INFO_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (VOS_INT32)pstMtaCnf->ulCellId);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSModemStatusQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_EMBMS_STATUS_QRY_CNF_STRU               *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_EMBMS_STATUS_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSModemStatusQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSModemStatusQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EMBMS_STATUS_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSModemStatusQryCnf : Current Option is not AT_CMD_EMBMS_STATUS_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (VOS_INT32)pstMtaCnf->enStatus);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSEVSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSEVSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSEVSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_UNSOLICITED_CFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSEVSetCnf : Current Option is not AT_CMD_MBMS_UNSOLICITED_CFG_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSServiceEventInd(VOS_VOID *pstMsg)
{
    AT_MTA_MSG_STRU                        *pstRcvMsg      = VOS_NULL_PTR;
    MTA_AT_MBMS_SERVICE_EVENT_IND_STRU     *pstMtaAtInd    = VOS_NULL_PTR;
    VOS_UINT8                               ucIndex;
    VOS_UINT16                              usLength;

    /* ��ʼ�� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstMtaAtInd         = (MTA_AT_MBMS_SERVICE_EVENT_IND_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;
    usLength            = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceEventInd : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s %d%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_MBMSEV].pucText,
                                       pstMtaAtInd->enEvent,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaMBMSInterestListSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSInterestListSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSInterestListSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_INTERESTLIST_SET)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSInterestListSetCnf : Current Option is not AT_CMD_MBMS_INTERESTLIST_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_VOID AT_ReportMBMSCmdQryCnf(
    MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU      *pstMtaCnf,
    VOS_UINT8                                       ucIndex
)
{
    VOS_UINT32                          ulListNum;
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucServiceID[AT_MBMS_SERVICE_ID_LENGTH + 1];

    usLength = 0;

    for (ulListNum = 0; ulListNum < pstMtaCnf->ulAvlServiceNum; ulListNum++)
    {
        /* ���MBMS Service ID�ַ�����ʽ */
        memset_s(aucServiceID, sizeof(aucServiceID), 0x00, sizeof(aucServiceID));
        At_ul2Auc(pstMtaCnf->astAvlServices[ulListNum].stTMGI.ulMbmsSerId, AT_MBMS_SERVICE_ID_LENGTH, aucServiceID);

        /* ^MBMSCMD: <AreaID>,<TMGI>:MBMS Service ID */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: %d,%s",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          (VOS_INT32)pstMtaCnf->astAvlServices[ulListNum].ulAreaId,
                                          aucServiceID);

        /* <TMGI>:Mcc */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%x%x%x",
                                          (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMcc & 0x0f00) >> 8,
                                          (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMcc & 0xf0) >> 4,
                                          (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMcc & 0x0f));
        /* <TMGI>:Mnc */
        if ((pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f00) == 0x0f00)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%x%x",
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0xf0) >> 4,
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f));
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%x%x%x",
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f00) >> 8,
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0xf0) >> 4,
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f));
        }

        if (pstMtaCnf->astAvlServices[ulListNum].bitOpSessionId == VOS_TRUE)
        {
            /* <SessionID> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              ",%d",
                                              (VOS_INT32)pstMtaCnf->astAvlServices[ulListNum].ulSessionId);
        }

        if (ulListNum != (pstMtaCnf->ulAvlServiceNum - 1))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s",
                                              gaucAtCrLf);
        }
    }

    gstAtSendData.usBufLen = usLength;
    return;
}


VOS_UINT32 AT_RcvMtaMBMSCmdQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU      *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSCmdQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSCmdQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MBMS_AVL_SERVICE_LIST_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSCmdQryCnf : Current Option is not AT_CMD_MBMS_AVL_SERVICE_LIST_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        AT_ReportMBMSCmdQryCnf(pstMtaCnf, ucIndex);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif


VOS_UINT32 AT_SetLteLowPowerPara(VOS_UINT8 ucIndex)
{
    AT_MTA_LOW_POWER_CONSUMPTION_SET_REQ_STRU       stPowerConsumption;
    VOS_UINT32                                      ulRst;

    memset_s(&stPowerConsumption, sizeof(stPowerConsumption), 0x00, sizeof(AT_MTA_LOW_POWER_CONSUMPTION_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if(gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[0].ulParaValue)
    {
        case 0:
            stPowerConsumption.enLteLowPowerFlg = AT_MTA_LTE_LOW_POWER_NORMAL;
            break;

        case 1:
            stPowerConsumption.enLteLowPowerFlg = AT_MTA_LTE_LOW_POWER_LOW;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϳ����Ϣ��C��, ���õ͹��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_LOW_POWER_SET_REQ,
                                   &stPowerConsumption,
                                   sizeof(stPowerConsumption),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetLteLowPowerPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_INT32 AT_GetIsmCoexParaValue(VOS_UINT8 *pucBegain, VOS_UINT8 **ppEnd)
{
    VOS_UINT32 ulTotal      = 0;
    VOS_INT32  lRstTotal    = 0;
    VOS_UINT32 ulRst;
    VOS_UINT32 ulFlag       = 0;
    VOS_UINT8 *pucEnd       = VOS_NULL_PTR;

    pucEnd = pucBegain;

    while((*pucEnd != ' ') && (*pucEnd != '\0'))
    {
        pucEnd++;
    }

    if(*pucBegain == '-')
    {
        ulFlag = 1;
        pucBegain++;
    }

    ulRst = atAuc2ul(pucBegain, (VOS_UINT16)(pucEnd - pucBegain), &ulTotal);

    if(ulRst != AT_SUCCESS)
    {
        lRstTotal = AT_COEX_INVALID;
    }
    else
    {
        *ppEnd      = (pucEnd + 1);
        lRstTotal   = (VOS_INT32)(ulFlag ? (0 - ulTotal):ulTotal);
    }

    return lRstTotal;
}


VOS_UINT32 AT_CheckIsmCoexParaValue(VOS_INT32 ulVal, VOS_UINT32 ulParaNum)
{
    VOS_UINT32                          ulRst = AT_SUCCESS;

    switch(ulParaNum)
    {
        case AT_COEX_PARA_COEX_ENABLE:
            if((ulVal < AT_COEX_PARA_COEX_ENABLE_MIN)
            || (ulVal > AT_COEX_PARA_COEX_ENABLE_MAX))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_TX_BEGIN:
            if((ulVal < AT_COEX_PARA_TX_BEGIN_MIN)
            || (ulVal > AT_COEX_PARA_TX_BEGIN_MAX))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_TX_END:
            if((ulVal < AT_COEX_PARA_TX_END_MIN)
            || (ulVal > AT_COEX_PARA_TX_END_MAX))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_TX_POWER:
            if((ulVal < AT_COEX_PARA_TX_POWER_MIN)
            || (ulVal > AT_COEX_PARA_TX_POWER_MAX))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_RX_BEGIN:
            if((ulVal < AT_COEX_PARA_RX_BEGIN_MIN)
            || (ulVal > AT_COEX_PARA_RX_BEGIN_MAX))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_RX_END:
            if((ulVal < AT_COEX_PARA_RX_END_MIN)
            || (ulVal > AT_COEX_PARA_RX_END_MAX))
            {
                ulRst = AT_FAILURE;
            }
            break;
        default:
            ulRst = AT_FAILURE;
            break;

    }

    return ulRst;
}


VOS_VOID AT_SetL4AIsmCoexParaValue(AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU stIsmCoex, L4A_ISMCOEX_REQ_STRU *pstReqToL4A, VOS_UINT8 ucIndex)
{
    VOS_UINT32                          i;

    pstReqToL4A->stCtrl.ulClientId = gastAtClientTab[ucIndex].usClientId;;
    pstReqToL4A->stCtrl.ulOpId     = 0;
    pstReqToL4A->stCtrl.ulPid      = WUEPS_PID_AT;

    for(i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++)
    {
        pstReqToL4A->astCoex[i].ulFlag      = (VOS_UINT32)stIsmCoex.astCoexPara[i].enCfg;
        pstReqToL4A->astCoex[i].ulTXBegin   = (VOS_UINT32)stIsmCoex.astCoexPara[i].usTxBegin;
        pstReqToL4A->astCoex[i].ulTXEnd     = (VOS_UINT32)stIsmCoex.astCoexPara[i].usTxEnd;
        pstReqToL4A->astCoex[i].lTXPower    = (VOS_INT32)stIsmCoex.astCoexPara[i].sTxPower;
        pstReqToL4A->astCoex[i].ulRXBegin   = (VOS_UINT32)stIsmCoex.astCoexPara[i].usRxBegin;
        pstReqToL4A->astCoex[i].ulRXEnd     = (VOS_UINT32)stIsmCoex.astCoexPara[i].usRxEnd;
    }

    return;
}


VOS_UINT32 AT_SetIsmCoexPara(VOS_UINT8 ucIndex)
{
    AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU               stIsmCoex;
    L4A_ISMCOEX_REQ_STRU                            stReqToL4A = {0};
    VOS_UINT32                                      ulRst,ulRet;
    VOS_UINT32                                      i, j;
    VOS_INT32                                       ret;
    VOS_UINT16                                     *pusVal  = VOS_NULL_PTR; /* ��Ҫ�洢��ֵָ�� */
    VOS_UINT8                                      *pucCur  = VOS_NULL_PTR; /* �����ַ���ʱ�ĵ�ǰָ�� */
    VOS_UINT8                                      *pucPara = VOS_NULL_PTR; /* �����ַ���ͷָ�� */

    memset_s(&stIsmCoex, sizeof(stIsmCoex), 0x00, sizeof(AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex != AT_MTA_ISMCOEX_BANDWIDTH_NUM)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }


    for(i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++)
    {
        pucCur = gastAtParaList[i].aucPara;
        stIsmCoex.astCoexPara[i].enCoexBWType = (AT_MTA_COEX_BW_TYPE_ENUM_UINT16)i;
        pusVal = &(stIsmCoex.astCoexPara[i].enCfg);

        for(j = 0; j < sizeof(AT_MTA_COEX_PARA_STRU)/sizeof(VOS_UINT16) - 2; j++)
        {
            pucPara = pucCur;
            ret = AT_GetIsmCoexParaValue(pucPara, &pucCur);

            if(ret == AT_COEX_INVALID)
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            if (AT_CheckIsmCoexParaValue(ret, j) == AT_FAILURE)
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }

           *pusVal = (VOS_UINT16)ret;
            pusVal++;
        }
    }

    stIsmCoex.usCoexParaNum     = AT_MTA_ISMCOEX_BANDWIDTH_NUM;
    stIsmCoex.usCoexParaSize    = sizeof(stIsmCoex.astCoexPara);

    AT_SetL4AIsmCoexParaValue(stIsmCoex, &stReqToL4A, ucIndex);

    /* ������Ϣ��L4A */
    ulRet = atSendL4aDataMsg(gastAtClientTab[ucIndex].usClientId,
                             I0_MSP_L4_L4A_PID,
                             ID_MSG_L4A_ISMCOEXSET_REQ,
                             (VOS_VOID*)(&stReqToL4A),
                             sizeof(stReqToL4A));

    /* ���Ϳ����Ϣ��C�� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_WIFI_COEX_SET_REQ,
                                   &stIsmCoex,
                                   sizeof(stIsmCoex),
                                   I0_UEPS_PID_MTA);
    if (ulRet != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetIsmCoexPara: atSendDataMsg fail.");
    }

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetIsmCoexPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_WIFI_COEX_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryIsmCoexPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* ���Ϳ����Ϣ��C��, ��ѯISMCOEX�б����� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_WIFI_COEX_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryIsmCoexPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_WIFI_COEX_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaLteLowPowerSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaLteLowPowerSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLteLowPowerSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET)
    {
        AT_WARN_LOG("AT_RcvMtaLteLowPowerSetCnf : Current Option is not AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaIsmCoexSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTE_WIFI_COEX_SET)
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexSetCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvL4AIsmCoexSetCnf(
    VOS_VOID                           *pMsg
)
{
    return VOS_OK;
}



VOS_UINT32 AT_RcvMtaIsmCoexQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_LTE_WIFI_COEX_QRY_CNF_STRU          *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT32                                  i;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_LTE_WIFI_COEX_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTE_WIFI_COEX_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexQryCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    for(i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                        " %d %d %d %d %d %d,",
                                                        pstCnf->astCoexPara[i].enCfg,
                                                        pstCnf->astCoexPara[i].usTxBegin,
                                                        pstCnf->astCoexPara[i].usTxEnd,
                                                        pstCnf->astCoexPara[i].sTxPower,
                                                        pstCnf->astCoexPara[i].usRxBegin,
                                                        pstCnf->astCoexPara[i].usRxEnd);
    }

    gstAtSendData.usBufLen--;

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_ProLCaCfgPara(
    AT_MTA_CA_CFG_SET_REQ_STRU                     *pstCaCfgReq
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulParaIndex;
    VOS_UINT32                          i;

    /* �������Ϊʹ�ܣ�ֻ��Ҫһ������ */
    if (gastAtParaList[0].ulParaValue == 1)
    {
        pstCaCfgReq->ucCaActFlag = (VOS_UINT8)gastAtParaList[0].ulParaValue;
        return AT_OK;
    }
    /* �������Ϊȥʹ�ܣ�������Ҫ������������ */
    if ( (gucAtParaIndex < 3)
      || (gastAtParaList[1].usParaLen == 0)
      || (gastAtParaList[2].usParaLen == 0))
    {
        AT_WARN_LOG("AT_ProLCaCfgPara: para num is error or para len is 0.");
        return AT_ERROR;
    }

    /* band numֵ��band�����Բ��� */
    if ((gucAtParaIndex - AT_MTA_BAND_INFO_OFFSET) != gastAtParaList[2].ulParaValue)
    {
        AT_WARN_LOG("AT_ProLCaCfgPara: para num is error.");
        return AT_ERROR;
    }

    pstCaCfgReq->ucCaActFlag            = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    pstCaCfgReq->stCaInfo.ucCaA2Flg     = ((VOS_UINT8)gastAtParaList[1].ulParaValue) & (0x01);
    pstCaCfgReq->stCaInfo.ucCaA4Flg     = (((VOS_UINT8)gastAtParaList[1].ulParaValue) & (0x02)) >> 1;
    pstCaCfgReq->stCaInfo.ucCaCqiFlg    = (((VOS_UINT8)gastAtParaList[1].ulParaValue) & (0x04)) >> 2;
    pstCaCfgReq->stCaInfo.usBandNum     = (VOS_UINT16)gastAtParaList[2].ulParaValue;

    for (i = 0; i < pstCaCfgReq->stCaInfo.usBandNum; i++)
    {
        ulParaIndex = AT_MTA_BAND_INFO_OFFSET + i;

        if (At_AsciiNum2HexString(gastAtParaList[ulParaIndex].aucPara, &(gastAtParaList[ulParaIndex].usParaLen)) == AT_FAILURE)
        {
            return AT_ERROR;
        }

        if (sizeof(AT_MTA_BAND_INFO_STRU) != gastAtParaList[ulParaIndex].usParaLen)
        {
            AT_WARN_LOG("AT_ProLCaCfgPara: para len is error.");
            return AT_ERROR;
        }

        lMemResult = memcpy_s(&(pstCaCfgReq->stCaInfo.astBandInfo[i]),
                              (VOS_SIZE_T)sizeof(pstCaCfgReq->stCaInfo.astBandInfo[i]),
                              gastAtParaList[ulParaIndex].aucPara,
                              gastAtParaList[ulParaIndex].usParaLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(pstCaCfgReq->stCaInfo.astBandInfo[i]), gastAtParaList[ulParaIndex].usParaLen);
    }

    return AT_OK;
}


VOS_UINT32 AT_SetLCaCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_CA_CFG_SET_REQ_STRU                      stCaCfgReq;
    VOS_UINT32                                      ulRst;

    memset_s(&stCaCfgReq, sizeof(stCaCfgReq), 0x00, sizeof(AT_MTA_CA_CFG_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if ( (gucAtParaIndex < 1)
      || (gucAtParaIndex > 11))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ч�Լ�� */
    if (AT_ProLCaCfgPara(&stCaCfgReq) == AT_ERROR)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϳ����Ϣ��C��, ���õ͹��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_CA_CFG_SET_REQ,
                                   &stCaCfgReq,
                                   sizeof(stCaCfgReq),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetLCaCfgPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_CA_CFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaLteCaCfgSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                         *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCfgSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCfgSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTE_CA_CFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCfgSetCnf : Current Option is not AT_CMD_LTE_CA_CFG_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_QryLCaCellExPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* ���Ϳ����Ϣ��C��, ��ѯLCACELLEX�б����� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_CA_CELLEX_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryLCaCellExPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_CA_CELLEX_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaLteCaCellExQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_CA_CELL_INFO_CNF_STRU               *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT32                                  i;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_CA_CELL_INFO_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCellExQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCellExQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTE_CA_CELLEX_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCellExQryCnf : Current Option is not AT_CMD_LTE_CA_CELLEX_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;

        pstCnf->ulTotalCellNum = (pstCnf->ulTotalCellNum < MTA_AT_CA_MAX_CELL_NUM) ?
                                  pstCnf->ulTotalCellNum : MTA_AT_CA_MAX_CELL_NUM;

        for (i = 0; i < pstCnf->ulTotalCellNum; i++)
        {
            gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                            "%s: %d,%d,%d,%d,%d,%d,%d,%d",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            pstCnf->astCellInfo[i].ucCellIndex,
                                                            pstCnf->astCellInfo[i].ucUlConfigured,
                                                            pstCnf->astCellInfo[i].ucDlConfigured,
                                                            pstCnf->astCellInfo[i].ucActived,
                                                            pstCnf->astCellInfo[i].ucLaaScellFlg,
                                                            pstCnf->astCellInfo[i].usBandInd,
                                                            pstCnf->astCellInfo[i].enBandWidth,
                                                            pstCnf->astCellInfo[i].ulEarfcn);
            if (i != pstCnf->ulTotalCellNum - 1)
            {
                gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                            "%s",
                                                            gaucAtCrLf);
            }
        }
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaLteCaCellExInfoNtf(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex;
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_CA_CELL_INFO_NTF_STRU       *pstCaCellInfoNtf    = VOS_NULL_PTR;
    VOS_UINT32                          i;

    /* ��ʼ����Ϣ���� */
    ucIndex             = 0;
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstCaCellInfoNtf    = (MTA_AT_CA_CELL_INFO_NTF_STRU*)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaLteCaCellExInfoNtf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;

    for (i = 0; i < pstCaCellInfoNtf->ulTotalCellNum; i++)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                        "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                                        gaucAtCrLf,
                                                        gastAtStringTab[AT_STRING_LCACELLEX].pucText,
                                                        pstCaCellInfoNtf->ulTotalCellNum,
                                                        pstCaCellInfoNtf->astCellInfo[i].ucCellIndex,
                                                        pstCaCellInfoNtf->astCellInfo[i].ucUlConfigured,
                                                        pstCaCellInfoNtf->astCellInfo[i].ucDlConfigured,
                                                        pstCaCellInfoNtf->astCellInfo[i].ucActived,
                                                        pstCaCellInfoNtf->astCellInfo[i].ucLaaScellFlg,
                                                        pstCaCellInfoNtf->astCellInfo[i].usBandInd,
                                                        pstCaCellInfoNtf->astCellInfo[i].enBandWidth,
                                                        pstCaCellInfoNtf->astCellInfo[i].ulEarfcn);

        if (i == pstCaCellInfoNtf->ulTotalCellNum - 1)
        {
            gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                        "%s",
                                                        gaucAtCrLf);
        }
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}



VOS_UINT32 AT_SetLcaCellRptCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_CA_CELL_SET_REQ_STRU    stCACellType;
    VOS_UINT32                     ulRst;

    memset_s(&stCACellType, sizeof(stCACellType), 0x00, sizeof(AT_MTA_CA_CELL_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������������ȼ�� */
    if ( (gucAtParaIndex != 1)
      || (gastAtParaList[0].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stCACellType.blEnableType = gastAtParaList[0].ulParaValue;

    /* ���Ϳ����Ϣ��C�� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_CA_CELL_RPT_CFG_SET_REQ,
                                   &stCACellType,
                                   sizeof(stCACellType),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetLcaCellRptCfgPara: AT_SetLcaCellRptCfgPara fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LCACELLRPTCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaCACellSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_CA_CELL_SET_CNF_STRU                    *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_CA_CELL_SET_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaCACellSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaCACellSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LCACELLRPTCFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaCACellSetCnf : Current Option is not AT_CMD_LCACELLRPTCFG_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_QryLcaCellRptCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* ���Ϳ����Ϣ��C��, ��ѯLCACELLRPTCFG�б����� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_CA_CELL_RPT_CFG_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryLcaCellRptCfgPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LCACELLRPTCFG_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaCACellQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_CA_CELL_QRY_CNF_STRU                    *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_CA_CELL_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaCACellQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaCACellQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LCACELLRPTCFG_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaCACellQryCnf : Current Option is not AT_CMD_LCACELLRPTCFG_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstMtaCnf->blEnableType);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_SetFineTimeReqPara(VOS_UINT8 ucIndex)
{
    AT_MTA_FINE_TIME_SET_REQ_STRU       stFineTimeType;
    VOS_UINT32                          ulRst;

    memset_s(&stFineTimeType, sizeof(stFineTimeType), 0x00, sizeof(AT_MTA_FINE_TIME_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if ((gucAtParaIndex < 1) ||
        (gucAtParaIndex > 3)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[0].usParaLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stFineTimeType.ucFineTimeType = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    if (gastAtParaList[1].usParaLen == 0) {
        /* <num>�������·�ȡĬ��ֵ */
        stFineTimeType.usNum = AT_FINE_TIME_DEFAULT_NUM;
    } else {
        stFineTimeType.usNum = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    }

    if (gastAtParaList[2].usParaLen == 0) {
        /* <rat>�������·�ȡĬ��ֵLTE������ǰGPSоƬΪ��ƽ̨ */
        stFineTimeType.enRatMode = MTA_AT_FINE_TIME_RAT_LTE;
    } else {
        stFineTimeType.enRatMode = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    }

    if (stFineTimeType.ucFineTimeType == AT_MTA_FINETIME_PARA_0) {
        stFineTimeType.ucForceFlag = VOS_TRUE;
    } else {
        stFineTimeType.ucForceFlag = VOS_FALSE;
    }

    /* ���Ϳ����Ϣ��C�� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_FINE_TIME_SET_REQ,
                                   &stFineTimeType,
                                   sizeof(stFineTimeType),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetFineTimeReqPara: AT_SetFineTimeReqPara fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FINE_TIME_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaFineTimeSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_FINE_TIME_SET_CNF_STRU                  *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_FINE_TIME_SET_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaFineTimeSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaFineTimeSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FINE_TIME_SET)
    {
        AT_WARN_LOG("AT_RcvMtaFineTimeSetCnf : Current Option is not AT_CMD_LTE_FINE_TIME_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaSibFineTimeNtf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg          = VOS_NULL_PTR;
    MTA_AT_SIB_FINE_TIME_IND_STRU      *pstSibFineTime     = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;


    /* ��ʼ����Ϣ���� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstSibFineTime      = (MTA_AT_SIB_FINE_TIME_IND_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaSibFineTimeNtf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSibFineTimeNtf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;

    if (pstSibFineTime->enResult != VOS_OK)
    {
        /* "^FINETIMEFAIL: 1 */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                         "%s%s%d%s",
                                                         gaucAtCrLf,
                                                         gastAtStringTab[AT_STRING_FINETIMEFAIL].pucText,
                                                         1,
                                                         gaucAtCrLf);
        /* ������ */
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }
    else
    {
        /* "^FINETIMEINFO: */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                             "%s%s%d,%d,%d,%d,%s,%s",
                                                             gaucAtCrLf,
                                                             gastAtStringTab[AT_STRING_FINETIMEINFO].pucText,
                                                             pstSibFineTime->enRat,
                                                             pstSibFineTime->sTa,
                                                             pstSibFineTime->lSinr,
                                                             pstSibFineTime->enState,
                                                             pstSibFineTime->aucUtcTime,
                                                             pstSibFineTime->aucUtcTimeOffset);

        if(pstSibFineTime->ucLeapSecondValid == VOS_TRUE)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                             ",%d%s",
                                                             pstSibFineTime->sLeapSecond,
                                                             gaucAtCrLf);
        }
        else
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                            "%s",
                                                            gaucAtCrLf);
        }

        /* ������ */
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }

    return VOS_OK;

}


VOS_UINT32 AT_RcvMtaLppFineTimeNtf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg          = VOS_NULL_PTR;
    MTA_AT_LPP_FINE_TIME_IND_STRU      *pstLppFineTime     = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstLppFineTime      = (MTA_AT_LPP_FINE_TIME_IND_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaLppFineTimeNtf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLppFineTimeNtf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;

    if (pstLppFineTime->enResult != VOS_OK)
    {
        /* "^FINETIMEFAIL: 2 */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                         "%s%s%d%s",
                                                         gaucAtCrLf,
                                                         gastAtStringTab[AT_STRING_FINETIMEFAIL].pucText,
                                                         2,
                                                         gaucAtCrLf);
        /* ������ */
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }
    else
    {
        /* "^SFN: */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                           "%s%s%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_SFN].pucText,
                                           pstLppFineTime->usSysFn,
                                           gaucAtCrLf);
        /* ������ */
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }


    return VOS_OK;

}


VOS_UINT32 AT_CheckL2ComCfgPara(VOS_VOID)
{
    /* ������������2�������4����������Χ����ȷ */
    if ((gucAtParaIndex < 2)
     || (gucAtParaIndex > 4))
    {
        AT_WARN_LOG("AT_CheckL2ComCfgPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��һ����������Ϊ�գ����ش��� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_CheckL2ComCfgPara: First para length is 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_SetLL2ComCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_L2_COM_CFG_SET_REQ_STRU      stSetLmacComCfg;
    VOS_UINT32                          ulResult;

    memset_s(&stSetLmacComCfg, sizeof(stSetLmacComCfg), 0x00, sizeof(AT_MTA_L2_COM_CFG_SET_REQ_STRU));

    /* ������������ش��� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetLL2ComCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ч�Լ�� */
    ulResult = AT_CheckL2ComCfgPara();

    if (ulResult != AT_SUCCESS)
    {
        return ulResult;
    }

    /* ������� */
    stSetLmacComCfg.ulCmdType   = gastAtParaList[0].ulParaValue;
    stSetLmacComCfg.ulPara1     = gastAtParaList[1].ulParaValue;
    stSetLmacComCfg.ulPara2     = gastAtParaList[2].ulParaValue;
    stSetLmacComCfg.ulPara3     = gastAtParaList[3].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_LL2_COM_CFG_SET_REQ,
                                      (VOS_VOID*)&stSetLmacComCfg,
                                      sizeof(AT_MTA_L2_COM_CFG_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LL2COMCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetLL2ComCfgPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}


VOS_UINT32 AT_SetLL2ComQryPara(VOS_UINT8 ucIndex)
{
    AT_MTA_L2_COM_CFG_QRY_REQ_STRU      stL2ComQryReq;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stL2ComQryReq, sizeof(stL2ComQryReq), 0x00, sizeof(AT_MTA_L2_COM_CFG_QRY_REQ_STRU));

    /* ���������ͺϷ��Լ��,���Ϸ�ֱ�ӷ���ʧ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetLL2ComQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������ӦΪ1�������򷵻�AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetLL2ComQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��һ���������Ȳ���Ϊ0�����򷵻�AT_CME_INCORRECT_PARAMETERS */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_SetLL2ComQryPara: FIrst para length is 0.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ṹ�� */
    stL2ComQryReq.ulCmdType = gastAtParaList[0].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_LL2_COM_CFG_QRY_REQ,
                                      (VOS_VOID*)&stL2ComQryReq,
                                      sizeof(AT_MTA_L2_COM_CFG_QRY_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LL2COMCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetLL2ComQryPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}


VOS_UINT32 AT_RcvMtaLL2ComCfgSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg     = VOS_NULL_PTR;
    MTA_AT_L2_COM_CFG_SET_CNF_STRU     *pstMtaCnf   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_L2_COM_CFG_SET_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaLL2ComCfgSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLL2ComCfgSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LL2COMCFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaLL2ComCfgSetCnf : Current Option is not AT_CMD_LL2COMCFG_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    ulResult = AT_ConvertMtaResult(pstMtaCnf->enResult);

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaLL2ComCfgQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg           = VOS_NULL_PTR;
    MTA_AT_L2_COM_CFG_QRY_CNF_STRU     *pstLl2ComQryCnf     = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pMsg;
    pstLl2ComQryCnf     = (MTA_AT_L2_COM_CFG_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;
    ulResult            = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaLL2ComCfgQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLL2ComCfgQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�^LL2COMQRY����� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LL2COMCFG_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaLL2ComCfgQryCnf : Current Option is not AT_CMD_LL2COMCFG_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstLl2ComQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        /* ^LL2COMQRY:<cmd_type>,[<para1>,<para2>,<para3>]  */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                         "%s: %u,%u,%u,%u",
                                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                         pstLl2ComQryCnf->ulCmdType,
                                                         pstLl2ComQryCnf->ulPara1,
                                                         pstLl2ComQryCnf->ulPara2,
                                                         pstLl2ComQryCnf->ulPara3);
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ConvertMtaResult(pstLl2ComQryCnf->enResult);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_SetCenfsPara(VOS_UINT8 ucIndex)
{
    AT_MTA_UNSOLICITED_RPT_SET_REQ_STRU stAtCmd;
    VOS_UINT32                          ulResult;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(stAtCmd));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stAtCmd.enReqType       = AT_MTA_SET_CENFS_RPT_TYPE;
    stAtCmd.u.ucCenfsRptFlg = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* ��MTA���������ϱ������������� */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      0,
                                      ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                      &stAtCmd,
                                      sizeof(AT_MTA_UNSOLICITED_RPT_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetCenfsPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryCenfsPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��MTA���������ϱ������������� */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      0,
                                      ID_AT_MTA_EPS_NETWORK_QRY_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryCenfsPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CENFS_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT16 AT_PrintEpsNetworkInfo(
    MTA_AT_EPS_NETWORK_INFO_STRU       *pstEpsNetworkInfo,
    VOS_UINT8                           bCrlfFlg
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength = 0;

    /* ^CENFS: <plmn>,<IE> */
    if (bCrlfFlg == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s",
                                          gaucAtCrLf);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CENFS: %X%X%X",
                                       (0x0F00 & pstEpsNetworkInfo->ulMcc) >> 8,
                                       (0x00F0 & pstEpsNetworkInfo->ulMcc) >> 4,
                                       (0x000F & pstEpsNetworkInfo->ulMcc));

    if ( (0x000F & pstEpsNetworkInfo->ulMnc) == 0x000F )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%X%X,\"",
                                           (0x0F00 & pstEpsNetworkInfo->ulMnc) >> 8,
                                           (0x00F0 & pstEpsNetworkInfo->ulMnc) >> 4);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%X%X%X,\"",
                                           (0x0F00 & pstEpsNetworkInfo->ulMnc) >> 8,
                                           (0x00F0 & pstEpsNetworkInfo->ulMnc) >> 4,
                                           (0x000F & pstEpsNetworkInfo->ulMnc));
    }

    for (ulLoop = 0; ulLoop < pstEpsNetworkInfo->ucLength; ulLoop++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%02X",
                                           pstEpsNetworkInfo->aucEpsNetwork[ulLoop]);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"");

    if (bCrlfFlg == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           gaucAtCrLf);
    }

    return usLength;
}


VOS_UINT32 AT_RcvMtaEpsNetworkInfoInd(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg     = VOS_NULL_PTR;
    MTA_AT_EPS_NETWORK_INFO_STRU       *pstMtaInd   = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaInd           = (MTA_AT_EPS_NETWORK_INFO_STRU *)pRcvMsg->aucContent;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaEpsNetworkInfoInd: WARNING: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* ��ӡ��������ϱ���� */
    gstAtSendData.usBufLen = AT_PrintEpsNetworkInfo(pstMtaInd, VOS_TRUE);

    /* ������ */
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaEpsNetworkQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg     = VOS_NULL_PTR;
    MTA_AT_EPS_NETWORK_QRY_CNF_STRU    *pstMtaCnf   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_EPS_NETWORK_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaEpsNetworkQryCnf: WARNING: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* ��ѯ������ͨ���ǹ㲥 */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEpsNetworkQryCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CENFS_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaEpsNetworkQryCnf: Current Option is not AT_CMD_CENFS_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstMtaCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult                = AT_ConvertMtaResult(pstMtaCnf->enResult);
        gstAtSendData.usBufLen  = 0;
    }
    else
    {
        ulResult                = AT_OK;

        /* ��ӡ��������� */
        gstAtSendData.usBufLen  = AT_PrintEpsNetworkInfo(&pstMtaCnf->stEpsNetworkInfo, VOS_FALSE);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif


#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_SetNL2ComCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_L2_COM_CFG_SET_REQ_STRU      stSetNmacComCfg;
    VOS_UINT32                          ulResult;

    memset_s(&stSetNmacComCfg, sizeof(stSetNmacComCfg), 0x00, sizeof(AT_MTA_L2_COM_CFG_SET_REQ_STRU));

    /* ������������ش��� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetNL2ComCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ч�Լ�� */
    ulResult = AT_CheckL2ComCfgPara();

    if (ulResult != AT_SUCCESS)
    {
        return ulResult;
    }

    /* ������� */
    stSetNmacComCfg.ulCmdType   = gastAtParaList[0].ulParaValue;
    stSetNmacComCfg.ulPara1     = gastAtParaList[1].ulParaValue;
    stSetNmacComCfg.ulPara2     = gastAtParaList[2].ulParaValue;
    stSetNmacComCfg.ulPara3     = gastAtParaList[3].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_NL2_COM_CFG_SET_REQ,
                                      (VOS_VOID*)&stSetNmacComCfg,
                                      sizeof(AT_MTA_L2_COM_CFG_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NL2COMCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetNL2ComCfgPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}


VOS_UINT32 AT_RcvMtaNL2ComCfgSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg     = VOS_NULL_PTR;
    MTA_AT_L2_COM_CFG_SET_CNF_STRU     *pstMtaCnf   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_L2_COM_CFG_SET_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNL2ComCfgSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNL2ComCfgSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NL2COMCFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaNL2ComCfgSetCnf : Current Option is not AT_CMD_NL2COMCFG_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    ulResult = AT_ConvertMtaResult(pstMtaCnf->enResult);

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_SetNL2ComQryPara(VOS_UINT8 ucIndex)
{
    AT_MTA_L2_COM_CFG_QRY_REQ_STRU      stL2ComQryReq;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stL2ComQryReq, sizeof(stL2ComQryReq), 0x00, sizeof(AT_MTA_L2_COM_CFG_QRY_REQ_STRU));

    /* ���������ͺϷ��Լ��,���Ϸ�ֱ�ӷ���ʧ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetNL2ComQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������ӦΪ1�������򷵻�AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetNL2ComQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��һ���������Ȳ���Ϊ0�����򷵻�AT_CME_INCORRECT_PARAMETERS */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_SetNL2ComQryPara: First para length is 0.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ṹ�� */
    stL2ComQryReq.ulCmdType = gastAtParaList[0].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_NL2_COM_CFG_QRY_REQ,
                                      (VOS_VOID*)&stL2ComQryReq,
                                      sizeof(AT_MTA_L2_COM_CFG_QRY_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NL2COMCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }

    AT_WARN_LOG("AT_SetNL2ComQryPara: AT_FillAndSndAppReqMsg Failed!");
    return AT_ERROR;
}


VOS_UINT32 AT_RcvMtaNL2ComCfgQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg           = VOS_NULL_PTR;
    MTA_AT_L2_COM_CFG_QRY_CNF_STRU     *pstNl2ComQryCnf     = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pMsg;
    pstNl2ComQryCnf     = (MTA_AT_L2_COM_CFG_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;
    ulResult            = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNL2ComCfgQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNL2ComCfgQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�^NL2COMQRY����� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NL2COMCFG_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaNL2ComCfgQryCnf : Current Option is not AT_CMD_NL2COMCFG_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstNl2ComQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        /* ^NL2COMQRY:<cmd_type>,[<para1>,<para2>,<para3>]  */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                         "%s: %u,%u,%u,%u",
                                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                         pstNl2ComQryCnf->ulCmdType,
                                                         pstNl2ComQryCnf->ulPara1,
                                                         pstNl2ComQryCnf->ulPara2,
                                                         pstNl2ComQryCnf->ulPara3);
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ConvertMtaResult(pstNl2ComQryCnf->enResult);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif


VOS_UINT32 AT_SetLogEnablePara(VOS_UINT8 ucIndex)
{
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT����1��ʾ����ץȡmodemlog������0��ʾ������ץȡmodemlog */
    if (gastAtParaList[0].ulParaValue == 1)
    {
        /* ����ΪFALSE��ʾ����ץMODEM LOG */
        DMS_SET_PRINT_MODEM_LOG_TYPE(VOS_FALSE);
    }
    else
    {
        /* ����ΪTRUE��ʾ������ץMODEM LOG */
        DMS_SET_PRINT_MODEM_LOG_TYPE(VOS_TRUE);
    }

    return AT_OK;
}


VOS_UINT32 AT_QryLogEnable(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulEnableFlag;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usLength                            = 0;

    if (DMS_GET_PRINT_MODEM_LOG_TYPE() == VOS_FALSE)
    {
        /* DMS��ǰ����ץMODEM LOG������enableΪTRUE */
        ulEnableFlag = VOS_TRUE;
    }
    else
    {
        /* DMS��ǰ������ץMODEM LOG������enableΪFALSE */
        ulEnableFlag = VOS_FALSE;
    }

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulEnableFlag);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}




VOS_VOID AT_StopSimlockDataWriteTimer(VOS_UINT8  ucIndex)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimlockWriteExCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulTimerName;
    VOS_UINT32                          ulTempIndex;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    ulTempIndex  = (VOS_UINT32)ucIndex;
    ulTimerName  = AT_SIMLOCKWRITEEX_TIMER;
    ulTimerName |= AT_INTERNAL_PROCESS_TYPE;
    ulTimerName |= (ulTempIndex<<12);

    if (pstSimlockWriteExCtx != VOS_NULL_PTR)
    {
        (VOS_VOID)AT_StopRelTimer(ulTimerName, &(pstSimlockWriteExCtx->hSimLockWriteExProtectTimer));
    }

    return;
}

VOS_UINT32  AT_ProcSimlockWriteExData(
    VOS_UINT8                          *pucSimLockData,
    VOS_UINT16                          usParaLen
)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimlockWriteExCtx = VOS_NULL_PTR;
    VOS_UINT8                          *pTempData            = VOS_NULL_PTR;
    errno_t                             lMemResult;
    VOS_UINT16                          usTotalLen;

    if ((pucSimLockData == VOS_NULL_PTR)
     || (usParaLen == 0))
    {
        AT_ERR_LOG("AT_ProcSimlockWriteExData: NULL Pointer");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    /* ��ǰ��һ���µ����ù��̣��յ����ǵ�һ��AT���� */
    if (pstSimlockWriteExCtx->pucData == VOS_NULL_PTR)
    {
        /*lint -save -e830*/
        pstSimlockWriteExCtx->pucData = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, (VOS_UINT32)usParaLen);
        /*lint -restore */
        /* �����ڴ�ʧ�ܣ�ֱ�ӷ��� */
        if (pstSimlockWriteExCtx->pucData == VOS_NULL_PTR)
        {
            AT_ERR_LOG("AT_ProcSimlockWriteExData: first data, Alloc mem fail");

            return AT_CME_MEMORY_FAILURE;
        }

        lMemResult = memcpy_s(pstSimlockWriteExCtx->pucData, usParaLen, pucSimLockData, usParaLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, usParaLen, usParaLen);

        pstSimlockWriteExCtx->usSimlockDataLen = usParaLen;
    }
    else
    {
        /* ��ǰ�����յ���һ��AT�����Ҫƴ������ */
        usTotalLen = usParaLen + pstSimlockWriteExCtx->usSimlockDataLen;
        /*lint -save -e516*/
        pTempData  = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, usTotalLen);
        /*lint -restore */
        /* �����ڴ�ʧ�ܣ�ֱ�ӷ��� */
        if (pTempData == VOS_NULL_PTR)
        {
            AT_ERR_LOG("AT_ProcSimlockWriteExData: Non-first data, Alloc mem fail");

            return AT_CME_MEMORY_FAILURE;
        }

        lMemResult = memcpy_s(pTempData, usTotalLen, pstSimlockWriteExCtx->pucData, pstSimlockWriteExCtx->usSimlockDataLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, usTotalLen, pstSimlockWriteExCtx->usSimlockDataLen);
        lMemResult = memcpy_s((pTempData + pstSimlockWriteExCtx->usSimlockDataLen), (usTotalLen - pstSimlockWriteExCtx->usSimlockDataLen),
                              pucSimLockData, usParaLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, (usTotalLen - pstSimlockWriteExCtx->usSimlockDataLen), usParaLen);
        /*lint -save -e830*/
        PS_MEM_FREE(WUEPS_PID_AT, pstSimlockWriteExCtx->pucData);
        /*lint -restore */
        pstSimlockWriteExCtx->usSimlockDataLen = usTotalLen;
        pstSimlockWriteExCtx->pucData          = pTempData;
    }

    return AT_SUCCESS;


}

VOS_UINT32 AT_SaveSimlockDataIntoCtx(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara,
    VOS_UINT8                      ucIndex,
    AT_SIMLOCK_TYPE_ENUM_UINT8     ucNetWorkFlg)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimlockWriteExCtx = VOS_NULL_PTR;
    VOS_UINT8                          *pucSimLockData       = VOS_NULL_PTR;
    VOS_UINT8                          *pucHmac              = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulLayer;
    VOS_UINT32                          ulTotal;
    VOS_UINT32                          ulCurIndex;
    errno_t                             lMemResult;
    VOS_UINT16                          usSimLockDataLen;
    VOS_UINT16                          usHmacLen;
    VOS_UINT8                           ucParaNum;

    /* ������ʼ�� */
    ulLayer           = pstSimlockWriteExPara->ulLayer;
    ulTotal           = pstSimlockWriteExPara->ulTotal;
    ulCurIndex        = pstSimlockWriteExPara->ulIndex;
    usSimLockDataLen  = pstSimlockWriteExPara->usSimLockDataLen;
    usHmacLen         = pstSimlockWriteExPara->usHmacLen;
    ucParaNum         = pstSimlockWriteExPara->ucParaNum;
    pucSimLockData    = pstSimlockWriteExPara->pucSimLockData;
    pucHmac           = pstSimlockWriteExPara->pucHmac;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    /* ��ǰ�������ù����У���һ���յ������� */
    if (pstSimlockWriteExCtx->ucSettingFlag == VOS_FALSE)
    {
        if (ulCurIndex != 1)
        {
            AT_WARN_LOG1("AT_SaveSimlockDataIntoCtx: Invalid ulCurrIndex", pstSimlockWriteExPara->ulIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* ���ַ�������ת��Ϊ������������ */
        ulResult = AT_ProcSimlockWriteExData(pucSimLockData, usSimLockDataLen);

        if (ulResult != AT_SUCCESS)
        {
            AT_WARN_LOG1("AT_SaveSimlockDataIntoCtx: AT_ProcSimlockWriteExData fail %d", ulResult);

            return ulResult;
        }

        pstSimlockWriteExCtx->ucNetWorkFlg  = ucNetWorkFlg;
        pstSimlockWriteExCtx->ucClientId    = ucIndex;
        pstSimlockWriteExCtx->ucTotalNum    = (VOS_UINT8)ulTotal;
        pstSimlockWriteExCtx->ucCurIdx      = (VOS_UINT8)ulCurIndex;
        pstSimlockWriteExCtx->ucLayer       = (VOS_UINT8)ulLayer;
        pstSimlockWriteExCtx->ucSettingFlag = VOS_TRUE;
    }
    else
    {
        /* ����ͬһ���û��·� */
        if (ucNetWorkFlg != pstSimlockWriteExCtx->ucNetWorkFlg)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: ucNetWorkFlg error, PreNetWorkFlg %d, CurNetWorkFlg %d", ucNetWorkFlg, pstSimlockWriteExCtx->ucNetWorkFlg);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* ������ͬһ��ͨ���·����� */
        if (ucIndex != pstSimlockWriteExCtx->ucClientId)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: port error, ucIndex %d, ucClientId %d", ucIndex, pstSimlockWriteExCtx->ucClientId);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* ��ǰ�Ѿ��������У���ǰ�·���Layer��֮ǰ֮ǰ�·���Layer��ͬ */
        if ((VOS_UINT8)ulLayer != pstSimlockWriteExCtx->ucLayer)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: Layer %d wrong, %d", ulLayer, pstSimlockWriteExCtx->ucLayer);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* ��ǰ�Ѿ��������У���ǰ�·���total��֮ǰ֮ǰ�·���total��ͬ */
        if ((VOS_UINT8)ulTotal != pstSimlockWriteExCtx->ucTotalNum)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: total %d wrong, %d", ulTotal, pstSimlockWriteExCtx->ucTotalNum);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* ��ǰ�·���Index����֮ǰ�·�Index+1 */
        if ((VOS_UINT8)ulCurIndex != (pstSimlockWriteExCtx->ucCurIdx + 1))
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: CurIndex %d wrong, %d", ulCurIndex, pstSimlockWriteExCtx->ucCurIdx);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* ���ַ�������ת��Ϊ���� */
        ulResult = AT_ProcSimlockWriteExData(pucSimLockData, usSimLockDataLen);

        if (ulResult != AT_SUCCESS)
        {
            AT_WARN_LOG1("AT_SaveSimlockDataIntoCtx: AT_ProcSimlockWriteExData fail %d", ulResult);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return ulResult;
        }

        /* ����CurrIndex */
        pstSimlockWriteExCtx->ucCurIdx      = (VOS_UINT8)ulCurIndex;
    }

    /* �����������Ϊ5������5������copy��ȫ�ֱ��������֮ǰ������HMAC������֮ǰ������ */
    if ((ucParaNum == 5) && (usHmacLen == AT_SET_SIMLOCK_DATA_HMAC_LEN))
    {
        lMemResult = memcpy_s(pstSimlockWriteExCtx->aucHmac, AT_SET_SIMLOCK_DATA_HMAC_LEN, pucHmac, usHmacLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, AT_SET_SIMLOCK_DATA_HMAC_LEN, usHmacLen);
        pstSimlockWriteExCtx->ucHmacLen = (VOS_UINT8)usHmacLen;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckSimlockDataWriteExPara(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara
)
{

    /* �涨��layerȡֵΪ0,1,2,3,4,255 */
    if ((pstSimlockWriteExPara->ulLayer > 4) && (pstSimlockWriteExPara->ulLayer != 255))
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: invalid layer value:", pstSimlockWriteExPara->ulLayer);
        return VOS_ERR;
    }

    /* �涨��ulIndexȡֵΪ1-255 */
    if ((pstSimlockWriteExPara->ulIndex == 0) || (pstSimlockWriteExPara->ulIndex > 255))
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: invalid ulIndex value:", pstSimlockWriteExPara->ulIndex);
        return VOS_ERR;
    }

    /* �涨��ulTotalȡֵΪ1-255 */
    if ((pstSimlockWriteExPara->ulTotal == 0) || (pstSimlockWriteExPara->ulTotal > 255))
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: invalid ulTotal value:", pstSimlockWriteExPara->ulTotal);
        return VOS_ERR;
    }

    /* IndexҪС��total */
    if (pstSimlockWriteExPara->ulIndex > pstSimlockWriteExPara->ulTotal)
    {
        AT_WARN_LOG2("AT_CheckSimlockDataWriteExPara: Index bigger than total", pstSimlockWriteExPara->ulIndex, pstSimlockWriteExPara->ulTotal);

        return VOS_ERR;
    }

    /* �涨һ��д���simlockdata���ݲ�����1400���ַ� */
    if (pstSimlockWriteExPara->usSimLockDataLen > AT_SIMLOCKDATA_PER_WRITE_MAX_LEN)
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: SimLockData is too long:", pstSimlockWriteExPara->usSimLockDataLen);
        return VOS_ERR;
    }

    return VOS_OK;

}


VOS_UINT32 AT_SetSimlockDataWriteExPara(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara,
    VOS_UINT8                      ucIndex,
    AT_SIMLOCK_TYPE_ENUM_UINT8     ucNetWorkFlg
)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX                     *pstSimlockWriteExCtx    = VOS_NULL_PTR;
    DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU                  *pstSimlockWriteExSetReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulTimerName;
    VOS_UINT32                                              ulTempIndex;
    VOS_UINT32                                              ulResult;
    VOS_UINT32                                              ulLength;
    errno_t                                                 lMemResult;
    VOS_UINT16                                              usHmacLen;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    if (AT_CheckSimlockDataWriteExPara(pstSimlockWriteExPara) != VOS_OK)
    {
        if (pstSimlockWriteExCtx->ucSettingFlag == VOS_FALSE)
        {
            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);
        }

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (pstSimlockWriteExCtx->ucTotalNum > AT_SIM_LOCK_DATA_WRITEEX_MAX_TOTAL)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������浽ȫ�ֱ���  */
    ulResult = AT_SaveSimlockDataIntoCtx(pstSimlockWriteExPara, ucIndex, ucNetWorkFlg);

    if (ulResult != AT_OK)
    {
        return ulResult;
    }

    /* �����δ�������ݣ���������ʱ�����ظ�OK */
    if (pstSimlockWriteExCtx->ucCurIdx < pstSimlockWriteExCtx->ucTotalNum)
    {
        /* ֹͣ��һ���ڵĶ�ʱ����������ʱ�� */
        AT_StopSimlockDataWriteTimer(ucIndex);

        ulTempIndex  = (VOS_UINT32)ucIndex;
        ulTimerName  = AT_SIMLOCKWRITEEX_TIMER;
        ulTimerName |= AT_INTERNAL_PROCESS_TYPE;
        ulTimerName |= (ulTempIndex<<12);

        (VOS_VOID)AT_StartRelTimer(&(pstSimlockWriteExCtx->hSimLockWriteExProtectTimer),
                                   AT_SIMLOCK_WRITE_EX_PROTECT_TIMER_LEN,
                                   ulTimerName,
                                   0, VOS_RELTIMER_NOLOOP);

        return AT_OK;
    }
    else
    {
        /* �Ѿ����������ݣ���Simlock_Dataת������ */
        ulResult = At_AsciiNum2HexString(pstSimlockWriteExCtx->pucData, &(pstSimlockWriteExCtx->usSimlockDataLen));
        if (ulResult != AT_SUCCESS)
        {
            AT_WARN_LOG2("AT_SetSimlockDataWriteExPara: At_AsciiNum2HexString fail ulResult: %d ulParaLen: %d",
                         ulResult,
                         pstSimlockWriteExCtx->usSimlockDataLen);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* �Ѿ����������ݣ���HMACת������ */
        usHmacLen = pstSimlockWriteExCtx->ucHmacLen;
        ulResult = At_AsciiNum2HexString(pstSimlockWriteExCtx->aucHmac, &usHmacLen);

        pstSimlockWriteExCtx->ucHmacLen = (VOS_UINT8)usHmacLen;

        if ((ulResult != AT_SUCCESS)
         || (pstSimlockWriteExCtx->ucHmacLen != DRV_AGENT_HMAC_DATA_LEN))
        {
            AT_WARN_LOG2("AT_SetSimlockDataWriteExPara: At_AsciiNum2HexString fail ulResult: %d ulParaLen: %d",
                         ulResult,
                         pstSimlockWriteExCtx->ucHmacLen);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        ulLength = sizeof(DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU) + pstSimlockWriteExCtx->usSimlockDataLen - 4;
        pstSimlockWriteExSetReq = (DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, ulLength);

        if (pstSimlockWriteExSetReq == VOS_NULL_PTR)
        {
            AT_WARN_LOG("AT_SetSimlockDataWriteExPara: alloc mem fail.");

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_MEMORY_FAILURE;
        }

        pstSimlockWriteExSetReq->ulHmacLen = pstSimlockWriteExCtx->ucHmacLen;
        lMemResult = memcpy_s(pstSimlockWriteExSetReq->aucHmac,
                              sizeof(pstSimlockWriteExSetReq->aucHmac),
                              pstSimlockWriteExCtx->aucHmac,
                              pstSimlockWriteExCtx->ucHmacLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSimlockWriteExSetReq->aucHmac), pstSimlockWriteExCtx->ucHmacLen);

        pstSimlockWriteExSetReq->ulSimlockDataLen = pstSimlockWriteExCtx->usSimlockDataLen;
        lMemResult = memcpy_s(pstSimlockWriteExSetReq->aucSimlockData,
                              pstSimlockWriteExCtx->usSimlockDataLen,
                              pstSimlockWriteExCtx->pucData,
                              pstSimlockWriteExCtx->usSimlockDataLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, pstSimlockWriteExCtx->usSimlockDataLen, pstSimlockWriteExCtx->usSimlockDataLen);

        /* ��¼�ܹ�д��Ĵ����������һ��д��ʱ��index */
        pstSimlockWriteExSetReq->ulTotal = pstSimlockWriteExCtx->ucTotalNum;

        /* ��¼�Ƿ��������·��ı�ʶ */
        pstSimlockWriteExSetReq->ucNetWorkFlg = pstSimlockWriteExCtx->ucNetWorkFlg;
        pstSimlockWriteExSetReq->ucLayer      = pstSimlockWriteExCtx->ucLayer;

        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        /* ת���ɹ�, ���Ϳ����Ϣ��C��, ���ò��߹�Կ */
        ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                          gastAtClientTab[ucIndex].opId,
                                          DRV_AGENT_SIMLOCKWRITEEX_SET_REQ,
                                          pstSimlockWriteExSetReq,
                                          ulLength,
                                          I0_WUEPS_PID_DRV_AGENT);
        /*lint -save -e516*/
        PS_MEM_FREE(WUEPS_PID_AT, pstSimlockWriteExSetReq);
        /*lint -restore */
        if (ulResult != TAF_SUCCESS)
        {
            AT_WARN_LOG("AT_SetSimlockDataWriteExPara: AT_FillAndSndAppReqMsg fail.");

            return AT_ERROR;
        }

        /* ����SIMLOCKDATAWRITEEX���⴦����Ҫ�ֶ�������ʱ��*/
        if (At_StartTimer(AT_SET_PARA_TIME, ucIndex) != AT_SUCCESS)
        {
            AT_WARN_LOG("AT_SetSimlockDataWriteExPara: At_StartTimer fail.");

            return AT_ERROR;
        }

        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_PEND;

        /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SIMLOCKDATAWRITEEX_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
}


VOS_UINT32 AT_ParseSimlockDataWriteExParaValue(
    VOS_UINT8                          *pucData,
    AT_SIMLOCK_WRITE_EX_PARA_STRU      *pstSimlockWriteExPara,
    VOS_UINT16                          usPos,
    VOS_UINT16                          usLen
)
{
    VOS_UINT16                          ausCommPos[4] = {0};
    VOS_UINT32                          ulFirstParaVal;
    VOS_UINT32                          ulSecParaVal;
    VOS_UINT32                          ulThirdParaVal;
    VOS_UINT16                          usLoop;
    VOS_UINT16                          usFirstParaLen;
    VOS_UINT16                          usSecondParaLen;
    VOS_UINT16                          usThirdParaLen;
    VOS_UINT16                          usFourthParaLen;
    VOS_UINT16                          usFifthParaLen;
    VOS_UINT16                          usCommaCnt;
    VOS_UINT8                           ucParaNum;

    usCommaCnt          = 0;
    usFourthParaLen     = 0;
    usFifthParaLen      = 0;
    ulFirstParaVal      = 0;
    ulSecParaVal        = 0;
    ulThirdParaVal      = 0;
    ucParaNum           = 0;

    /* ��ȡ�����еĶ���λ�ú͸��� */
    for ( usLoop = usPos; usLoop < usLen; usLoop++ )
    {
        if (*(pucData + usLoop) == ',')
        {
            /* ��¼�¶��ŵ�λ�� */
            if (usCommaCnt < 4)
            {
                ausCommPos[usCommaCnt] = usLoop + 1;
            }
            usCommaCnt++;
        }
    }

    /* �����Ÿ�������4����AT����������ʧ�� */
    if ((usCommaCnt != 4) && (usCommaCnt != 3))
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: Num of Para is  Invalid!");
        return VOS_ERR;
    }

    /* ��������ĳ��� */
    usFirstParaLen  = (ausCommPos[0] - usPos) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);
    usSecondParaLen = (ausCommPos[1] - ausCommPos[0]) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);
    usThirdParaLen  = (ausCommPos[2] - ausCommPos[1]) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);

    /* ������ŵĸ���Ϊ3����ô�����ĸ���Ϊ4��������ŵĸ���Ϊ4����ô�����ĸ���Ϊ5 */
    if (usCommaCnt == 3)
    {
        usFourthParaLen = usLen - ausCommPos[2];
        ucParaNum       = 4;
    }
    else
    {
        usFourthParaLen = (ausCommPos[3] - ausCommPos[2]) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);
        usFifthParaLen  = usLen - ausCommPos[3];
        ucParaNum       = 5;
    }

    /* ��ȡ��һ������ֵ */
    if (atAuc2ul(pucData + usPos, usFirstParaLen, &ulFirstParaVal) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: ulFirstParaVal value invalid");
        return VOS_ERR;
    }

    /* ��ȡ�ڶ�������ֵ */
    if (atAuc2ul(pucData + ausCommPos[0], usSecondParaLen, &ulSecParaVal) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: ulSecParaVal value invalid");
        return VOS_ERR;
    }

    /* ��ȡ����������ֵ */
    if (atAuc2ul(pucData + ausCommPos[1], usThirdParaLen, &ulThirdParaVal) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: ulThirdParaVal value invalid");
        return VOS_ERR;
    }

    pstSimlockWriteExPara->ucParaNum        = ucParaNum;
    pstSimlockWriteExPara->ulLayer          = ulFirstParaVal;
    pstSimlockWriteExPara->ulIndex          = ulSecParaVal;
    pstSimlockWriteExPara->ulTotal          = ulThirdParaVal;
    pstSimlockWriteExPara->usSimLockDataLen = usFourthParaLen;
    pstSimlockWriteExPara->pucSimLockData   = pucData + ausCommPos[2];

    /* ���������������5 */
    if (ucParaNum == 5)
    {
        pstSimlockWriteExPara->usHmacLen    = usFifthParaLen;
        pstSimlockWriteExPara->pucHmac      = pucData + ausCommPos[3];
    }

    return VOS_OK;
}
#if (FEATURE_SC_NETWORK_UPDATE == FEATURE_ON)

VOS_UINT32 AT_HandleSimLockNWDataWriteCmd(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucDataPara = VOS_NULL_PTR;
    AT_PARSE_CMD_NAME_TYPE_STRU         stAtCmdName;
    AT_SIMLOCK_WRITE_EX_PARA_STRU       stSimlockWriteExPara;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;
    VOS_UINT16                          usCmdlen;
    VOS_UINT16                          usPos;
    VOS_UINT16                          usLength;
    VOS_INT8                            cRet;

    /* ֻ����APPVCOM�˿�19�·������� */
    if ((gastAtClientTab[ucIndex].UserType != AT_APP_USER)
      ||(gastAtClientTab[ucIndex].ucPortNo != APP_VCOM_DEV_INDEX_19))
    {
        return AT_FAILURE;
    }

    /* �ֲ�������ʼ�� */
    memset_s(&stAtCmdName, sizeof(stAtCmdName), 0x00, sizeof(stAtCmdName));
    usCmdlen             = (VOS_UINT16)VOS_StrNLen("AT^SIMLOCKNWDATAWRITE=", AT_CONST_NUM_23);

    if (usLen < usCmdlen)
    {
        return AT_FAILURE;
    }

    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usCmdlen);
    if (pucDataPara == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_HandleSimLockNWDataWriteCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    /*�������������������Ƚ�ʹ��*/
    lMemResult = memcpy_s(pucDataPara, usCmdlen, pucData, usCmdlen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usCmdlen, usCmdlen);

    /* AT����ͷ�ַ�ת��д */
    At_UpString(pucDataPara, usCmdlen);

    /* ��������ַ���ͷ������"AT^SIMLOCKDATAWRITEEX="ֱ�ӷ���AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^SIMLOCKNWDATAWRITE=", usCmdlen);
    if (cRet != 0)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(ucIndex, (VOS_UINT8*)"^SIMLOCKNWDATAWRITE", AT_EXTEND_CMD_TYPE);

    /* ��ȡ����(����������ǰ׺AT)���Ƽ����� */
    usPos = (VOS_UINT16)VOS_StrNLen("AT", AT_CONST_NUM_3);

    stAtCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrNLen("^SIMLOCKNWDATAWRITE", AT_CONST_NUM_23);
    lMemResult = memcpy_s(stAtCmdName.aucCmdName,
                          sizeof(stAtCmdName.aucCmdName),
                          (pucData + usPos),
                          stAtCmdName.usCmdNameLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmdName.aucCmdName), stAtCmdName.usCmdNameLen);
    stAtCmdName.aucCmdName[stAtCmdName.usCmdNameLen] = '\0';
    usPos += stAtCmdName.usCmdNameLen;

    usPos += (VOS_UINT16)VOS_StrNLen("=", AT_CONST_NUM_2);

    /* �ֲ�������ʼ�� */
    memset_s(&stSimlockWriteExPara, sizeof(stSimlockWriteExPara), 0x00, sizeof(stSimlockWriteExPara));
    stSimlockWriteExPara.pucSimLockData   = VOS_NULL_PTR;
    stSimlockWriteExPara.pucHmac          = VOS_NULL_PTR;

    if (AT_ParseSimlockDataWriteExParaValue(pucData, &stSimlockWriteExPara,usPos, usLen) != VOS_OK)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_CME_INCORRECT_PARAMETERS);
        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        return AT_SUCCESS;
    }

     /* �����������ͣ��������ͺͲ������� */
    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    gucAtCmdFmtType = AT_EXTEND_CMD_TYPE;

    ulResult = AT_SetSimlockDataWriteExPara(&stSimlockWriteExPara, ucIndex, AT_SIMLOCK_TYPE_NW);

    /* ��Ӵ�ӡ ^SIMLOCKNWDATAWRITE:<index>���� */
    usLength = 0;

    if (ulResult != AT_WAIT_ASYNC_RETURN)
    {
        if (ulResult == AT_OK)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s:%d",
                                               "^SIMLOCKNWDATAWRITE",
                                               stSimlockWriteExPara.ulIndex);
        }

        gstAtSendData.usBufLen  = usLength;
        At_FormatResultData(ucIndex, ulResult);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
    return AT_SUCCESS;
}
#endif

VOS_UINT32 AT_HandleSimLockDataWriteExCmd(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucDataPara = VOS_NULL_PTR;
    AT_PARSE_CMD_NAME_TYPE_STRU         stAtCmdName;
    AT_SIMLOCK_WRITE_EX_PARA_STRU       stSimlockWriteExPara;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;
    VOS_UINT16                          usCmdlen;
    VOS_UINT16                          usPos;
    VOS_UINT16                          usLength;
    VOS_INT8                            cRet;

    memset_s(&stAtCmdName, sizeof(stAtCmdName), 0x00, sizeof(stAtCmdName));

    /* �ֲ�������ʼ�� */
    usCmdlen             = (VOS_UINT16)VOS_StrNLen("AT^SIMLOCKDATAWRITEEX=", AT_CONST_NUM_23);

    /* ͨ����� */
    if (AT_IsApPort(ucIndex) == VOS_FALSE)
    {
        return AT_FAILURE;
    }

    /* ���Ȳ����������������С���� AT^SIMLOCKDATAWRITEEX=1,1,1,  ���ںź�6���ַ� */
    if ((usCmdlen + 6) > usLen)
    {
        return AT_FAILURE;
    }
    /*lint -save -e516*/
    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usCmdlen);
    /*lint -restore */
    if (pucDataPara == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_HandleSimLockDataWriteExCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    /*�������������������Ƚ�ʹ��*/
    lMemResult = memcpy_s(pucDataPara, usCmdlen, pucData, usCmdlen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usCmdlen, usCmdlen);

    /* AT����ͷ�ַ�ת��д */
    At_UpString(pucDataPara, usCmdlen);

    /* ��������ַ���ͷ������"AT^SIMLOCKDATAWRITEEX="ֱ�ӷ���AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^SIMLOCKDATAWRITEEX=", usCmdlen);
    if (cRet != 0)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(ucIndex, (VOS_UINT8*)"^SIMLOCKDATAWRITEEX", AT_EXTEND_CMD_TYPE);

    /* ��ȡ����(����������ǰ׺AT)���Ƽ����� */
    usPos = (VOS_UINT16)VOS_StrNLen("AT", AT_CONST_NUM_3);

    stAtCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrNLen("^SIMLOCKDATAWRITEEX", AT_CONST_NUM_23);
    lMemResult = memcpy_s(stAtCmdName.aucCmdName,
                          sizeof(stAtCmdName.aucCmdName),
                          (pucData + usPos),
                          stAtCmdName.usCmdNameLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmdName.aucCmdName), stAtCmdName.usCmdNameLen);
    stAtCmdName.aucCmdName[stAtCmdName.usCmdNameLen] = '\0';
    usPos += stAtCmdName.usCmdNameLen;

    usPos += (VOS_UINT16)VOS_StrNLen("=", AT_CONST_NUM_2);

    /* �ֲ�������ʼ�� */
    memset_s(&stSimlockWriteExPara, sizeof(stSimlockWriteExPara), 0x00, sizeof(stSimlockWriteExPara));
    stSimlockWriteExPara.pucSimLockData   = VOS_NULL_PTR;
    stSimlockWriteExPara.pucHmac          = VOS_NULL_PTR;

    if (AT_ParseSimlockDataWriteExParaValue(pucData, &stSimlockWriteExPara,usPos, usLen) != VOS_OK)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_CME_INCORRECT_PARAMETERS);
        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        gstAtSendData.usBufLen  = 0;
        At_FormatResultData(ucIndex, AT_ERROR);

        return AT_SUCCESS;
    }

     /* �����������ͣ��������ͺͲ������� */
    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    gucAtCmdFmtType = AT_EXTEND_CMD_TYPE;

    AT_PR_LOGI("enter");

    ulResult = AT_SetSimlockDataWriteExPara(&stSimlockWriteExPara, ucIndex, AT_SIMLOCK_TYPE_FAC);

    /* ��Ӵ�ӡ ^SIMLOCKDATAWRITEEX:<index>���� */
    usLength = 0;

    if (ulResult != AT_WAIT_ASYNC_RETURN)
    {
        if (ulResult == AT_OK)
        {
            AT_PR_LOGI("return OK");

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s:%d",
                                               "^SIMLOCKDATAWRITEEX",
                                               stSimlockWriteExPara.ulIndex);
        }

        gstAtSendData.usBufLen  = usLength;
        At_FormatResultData(ucIndex, ulResult);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
    return AT_SUCCESS;
}


VOS_UINT32 AT_HandleSimLockOtaDataWriteCmd(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucDataPara = VOS_NULL_PTR;
    AT_PARSE_CMD_NAME_TYPE_STRU         stAtCmdName;
    AT_SIMLOCK_WRITE_EX_PARA_STRU       stSimlockWriteExPara;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;
    VOS_UINT16                          usCmdlen;
    VOS_UINT16                          usPos;
    VOS_UINT16                          usLength;
    VOS_INT8                            cRet;

    memset_s(&stAtCmdName, sizeof(stAtCmdName), 0x00, sizeof(stAtCmdName));

    /* �ֲ�������ʼ�� */
    usCmdlen             = (VOS_UINT16)VOS_StrNLen("AT^SIMLOCKOTADATAWRITE=", AT_CONST_NUM_24);

    /* ͨ����� */
    if (AT_IsApPort(ucIndex) == VOS_FALSE)
    {
        return AT_FAILURE;
    }

    /* ���Ȳ����������������С���� AT^SIMLOCKOTADATAWRITE=1,1,1,  ���ںź�6���ַ� */
    if ((usCmdlen + 6) > usLen)
    {
        return AT_FAILURE;
    }
    /*lint -save -e516*/
    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usCmdlen);
    /*lint -restore */
    if (pucDataPara == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_HandleSimLockOtaDataWriteCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    /*�������������������Ƚ�ʹ��*/
    lMemResult = memcpy_s(pucDataPara, usCmdlen, pucData, usCmdlen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usCmdlen, usCmdlen);

    /* AT����ͷ�ַ�ת��д */
    At_UpString(pucDataPara, usCmdlen);

    /* ��������ַ���ͷ������"AT^SIMLOCKOTADATAWRITE="ֱ�ӷ���AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^SIMLOCKOTADATAWRITE=", usCmdlen);
    if (cRet != 0)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(ucIndex, (VOS_UINT8*)"^SIMLOCKOTADATAWRITE", AT_EXTEND_CMD_TYPE);

    /* ��ȡ����(����������ǰ׺AT)���Ƽ����� */
    usPos = (VOS_UINT16)VOS_StrNLen("AT", AT_CONST_NUM_3);

    stAtCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrNLen("^SIMLOCKOTADATAWRITE", AT_CONST_NUM_24);
    lMemResult = memcpy_s(stAtCmdName.aucCmdName,
                          sizeof(stAtCmdName.aucCmdName),
                          (pucData + usPos),
                          stAtCmdName.usCmdNameLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmdName.aucCmdName), stAtCmdName.usCmdNameLen);
    stAtCmdName.aucCmdName[stAtCmdName.usCmdNameLen] = '\0';
    usPos += stAtCmdName.usCmdNameLen;

    usPos += (VOS_UINT16)VOS_StrNLen("=", AT_CONST_NUM_2);

    /* �ֲ�������ʼ�� */
    memset_s(&stSimlockWriteExPara, sizeof(stSimlockWriteExPara), 0x00, sizeof(stSimlockWriteExPara));
    stSimlockWriteExPara.pucSimLockData   = VOS_NULL_PTR;
    stSimlockWriteExPara.pucHmac          = VOS_NULL_PTR;

    if (AT_ParseSimlockDataWriteExParaValue(pucData, &stSimlockWriteExPara,usPos, usLen) != VOS_OK)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_CME_INCORRECT_PARAMETERS);
        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        gstAtSendData.usBufLen  = 0;
        At_FormatResultData(ucIndex, AT_ERROR);

        return AT_SUCCESS;
    }

     /* �����������ͣ��������ͺͲ������� */
    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    gucAtCmdFmtType = AT_EXTEND_CMD_TYPE;

    AT_PR_LOGI("enter");

    ulResult = AT_SetSimlockDataWriteExPara(&stSimlockWriteExPara, ucIndex, AT_SIMLOCK_TYPE_OTA);

    /* ��Ӵ�ӡ ^SIMLOCKDATAWRITEEX:<index>���� */
    usLength = 0;

    if (ulResult != AT_WAIT_ASYNC_RETURN)
    {
        if (ulResult == AT_OK)
        {
            AT_PR_LOGI("return OK");

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s:%d",
                                               "^SIMLOCKOTADATAWRITE",
                                               stSimlockWriteExPara.ulIndex);
        }

        gstAtSendData.usBufLen  = usLength;
        At_FormatResultData(ucIndex, ulResult);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
    return AT_SUCCESS;
}


VOS_UINT32 AT_RcvDrvAgentSimlockWriteExSetCnf(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                         *pRcvMsg = VOS_NULL_PTR;
    DRV_AGENT_SIMLOCKWRITEEX_SET_CNF_STRU      *pstEvent = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT16                                  usLength;
    VOS_UINT8                                   ucIndex;

    AT_PR_LOGI("enter");

    /* ��ʼ����Ϣ���� */
    pRcvMsg         = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent        = (DRV_AGENT_SIMLOCKWRITEEX_SET_CNF_STRU *)pRcvMsg->aucContent;
    ucIndex         = 0;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockWriteExSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockWriteExSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_SIMLOCKWRITEEX_SET */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SIMLOCKDATAWRITEEX_SET )
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockWriteExSetCnf: CmdCurrentOpt ERR.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength = 0;

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if ( pstEvent->enResult == DRV_AGENT_PERSONALIZATION_NO_ERROR )
    {
        /* ������ý�� */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           "%s:%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstEvent->ulTotal);
        ulResult    = AT_OK;
    }
    else
    {
        /* �쳣���, ת�������� */
        ulResult    = AT_PERSONALIZATION_ERR_BEGIN + pstEvent->enResult;
    }

    gstAtSendData.usBufLen  = usLength;

    /* ����AT_FormATResultDATa���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#if (FEATURE_PHONE_SC == FEATURE_ON)

VOS_UINT32 AT_SimLockDataReadExPara(VOS_UINT8 ucIndex)
{
    DRV_AGENT_SIMLOCKDATAREADEX_READ_REQ_STRU stSimLockDataReadExReq;
    VOS_UINT32                                ulResult;

    AT_PR_LOGI("enter");

    /* �ֲ�������ʼ�� */
    memset_s(&stSimLockDataReadExReq, sizeof(stSimLockDataReadExReq), 0x00, sizeof(stSimLockDataReadExReq));

    /* ͨ����� */
    if (AT_IsApPort(ucIndex) == VOS_FALSE)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: It Is not Ap Port.");
        return AT_ERROR;
    }

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: ucCmdOptType is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: gucAtParaIndex ERR.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����ĳ��Ȳ���Ϊ0 */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: gastAtParaList[0].usParaLen err.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSimLockDataReadExReq.ucLayer = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stSimLockDataReadExReq.ucIndex = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      DRV_AGENT_SIMLOCKDATAREADEX_READ_REQ,
                                      &stSimLockDataReadExReq,
                                      (VOS_UINT32)sizeof(stSimLockDataReadExReq),
                                      I0_WUEPS_PID_DRV_AGENT);
    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SIMLOCKDATAREADEX_READ_SET;

    return AT_WAIT_ASYNC_RETURN;

}
#endif


VOS_UINT32 AT_RcvDrvAgentSimlockDataReadExReadCnf(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                         *pRcvMsg     = VOS_NULL_PTR;
    DRV_AGENT_SIMLOCKDATAREADEX_READ_CNF_STRU  *pstEvent    = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT32                                  ulLoop;
    VOS_UINT16                                  usLength;
    VOS_UINT8                                   ucIndex;

    AT_PR_LOGI("enter");

    /* ��ʼ����Ϣ���� */
    usLength = 0;
    ucIndex  = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    pRcvMsg  = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent = (DRV_AGENT_SIMLOCKDATAREADEX_READ_CNF_STRU *)pRcvMsg->aucContent;

    ulResult = pstEvent->ulResult;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId,&ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_SIMLOCKDATAREADEX_READ_SET */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SIMLOCKDATAREADEX_READ_SET)
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: CmdCurrentOpt ERR.");
        return VOS_ERR;
    }

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if ( pstEvent->ulResult == DRV_AGENT_PERSONALIZATION_NO_ERROR)
    {
        /* ������ý�� */
        ulResult    = AT_OK;

        /* ���<layer>,<index>,<total>��ӡ */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                            "%s:%d,%d,%d,",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            pstEvent->ucLayer,
                                            pstEvent->ucIndex,
                                            pstEvent->ucTotal);

        /* ���<simlock_data>��ӡ */
        for (ulLoop = 0; ulLoop < pstEvent->ulDataLen; ulLoop++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%02x",
                                               pstEvent->aucData[ulLoop]);
        }
    }
    else
    {
        /* �쳣���, ת�������� */
        ulResult    = AT_PERSONALIZATION_ERR_BEGIN + pstEvent->ulResult;
    }

    gstAtSendData.usBufLen = usLength;

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ����AT_FormATResultDATa���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;

}


VOS_UINT32 AT_SetActPdpStubPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucFlag;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if((gastAtParaList[0].usParaLen == 0)
    || (gastAtParaList[1].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ȡ���õı�־ */
    ucFlag = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* ����MODEM ID���ò�ͬ��׮���� */
    if (gastAtParaList[0].ulParaValue == 0)
    {
        AT_SetPcuiPsCallFlag(ucFlag, AT_CLIENT_TAB_APP_INDEX);
    }
    else if (gastAtParaList[0].ulParaValue == 1)
    {
        AT_SetCtrlPsCallFlag(ucFlag, AT_CLIENT_TAB_APP_INDEX);
    }
    else if (gastAtParaList[0].ulParaValue == 2)
    {
        AT_SetPcui2PsCallFlag(ucFlag, AT_CLIENT_TAB_APP_INDEX);
    }
    else
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}


VOS_UINT32 AT_SetNVCHKPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8           ucLoopIndex;

    AT_PR_LOGI("Rcv Msg");

    /* ������� */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if ( gucAtParaIndex != 1 )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* δ�������ж� */
    if(gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����ȫ����飬��ѭ�����ȫ��CRC */
    if ( gastAtParaList[0].ulParaValue == 0)
    {
        for( ucLoopIndex = 0; ucLoopIndex < 3; ucLoopIndex++ )
        {
            if ( mdrv_nv_check_factorynv( ucLoopIndex ) != 0 )
            {
                AT_PR_LOGI("Call interface success!");
                return AT_ERROR;
            }
        }

        return AT_OK;
    }

    /* �������0��Ϊ��������0Ϊ�쳣 */
    if ( mdrv_nv_check_factorynv( gastAtParaList[0].ulParaValue - 1 ) == 0)
    {
        AT_PR_LOGI("Call interface success!");
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }

}




VOS_UINT32 AT_RcvMtaAfcClkInfoCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pstMtaMsg    = VOS_NULL_PTR;
    MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF_STRU    *pstAfcCnf    = VOS_NULL_PTR;
    VOS_UINT32                                  ulRet;
    VOS_UINT8                                   ucIndex;

    pstMtaMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstAfcCnf = (MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF_STRU *)pstMtaMsg->aucContent;

    /* ��ʼ����Ϣ���� */
    ucIndex = 0;
    ulRet   = AT_OK;

     /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvMtaAfcClkInfoCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex)) {
        AT_WARN_LOG("AT_RcvMtaAfcClkInfoCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_AFCCLKINFO_QRY) {
        AT_WARN_LOG("AT_RcvMtaAfcClkInfoCnf : Current Option is not AT_CMD_AFCCLKINFO_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstAfcCnf->enResult != MTA_AT_RESULT_NO_ERROR) {
        ulRet = AT_ERROR;
    } else {
        /* ^AFCCLKINFO: <status>,<deviation>,<sTemp>,<eTemp>,<a0_m>,<a0_e>,
                        <a1_m>,<a1_e>,<a2_m>,<a2_e>,<a3_m>,<a3_e>,
                        <rat>,<ModemId> */
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %u,%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstAfcCnf->enStatus,
                                                        pstAfcCnf->lDeviation,
                                                        pstAfcCnf->sCoeffStartTemp,
                                                        pstAfcCnf->sCoeffEndTemp,
                                                        pstAfcCnf->aulCoeffMantissa[0],
                                                        pstAfcCnf->aulCoeffMantissa[1],
                                                        pstAfcCnf->aulCoeffMantissa[2],
                                                        pstAfcCnf->aulCoeffMantissa[3],
                                                        pstAfcCnf->ausCoeffExponent[0],
                                                        pstAfcCnf->ausCoeffExponent[1],
                                                        pstAfcCnf->ausCoeffExponent[2],
                                                        pstAfcCnf->ausCoeffExponent[3],
                                                        pstAfcCnf->enRatMode,
                                                        pstAfcCnf->enModemId);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulRet);

    return VOS_OK;

}




VOS_UINT32 AT_SetSecureStatePara(VOS_UINT8 ucIndex)
{
    VOS_INT                                 iRst;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����Ϸ��Լ�� */
    if ( (gucAtParaIndex != 1)
      || (gastAtParaList[0].usParaLen == 0) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���õ����ṩ�Ľӿ�ʵ�����ò��� */
    iRst = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_SET_SECURESTATE,
                            (VOS_INT)gastAtParaList[0].ulParaValue,
                            VOS_NULL_PTR,
                            0);

    /* ���ݵ���ӿڷ��صĽ���������ý�� */
    /* ������1ʱ�����ظ�����,����0ʱ���óɹ�,�����������ʧ�� */
    if (iRst == AT_EFUSE_REPEAT)
    {
        return AT_ERROR;
    }
    else if (iRst == AT_EFUSE_OK)
    {
        return AT_OK;
    }
    else
    {
        AT_WARN_LOG("AT_SetSecureStatePara : Set Secure state req failed.");
    }

    return AT_CME_UNKNOWN;
}


VOS_UINT32 AT_QrySecureStatePara(VOS_UINT8 ucIndex)
{
    VOS_INT                             iResult;
    VOS_UINT16                          usLength;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usLength = 0;

    /* ���õ����ṩ�Ľ�ڲ�ѯ */
    iResult = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_GET_SECURESTATE,
                               0,
                               VOS_NULL_PTR,
                               0);

    /* �����쳣��ѯ��� */
    /* ��ѯʧ�� */
    if (iResult < AT_SECURE_STATE_NOT_SET)
    {
        return AT_ERROR;
    }

    /* ��ѯ����쳣 */
    if (iResult > AT_SECURE_STATE_RMA)
    {
        return AT_CME_UNKNOWN;
    }

    /* ��ӡ��� */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: %d",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                     iResult);

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}

#if (FEATURE_ON == FEATURE_PHONE_ENG_AT_CMD)

VOS_UINT32 AT_SetKcePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                              ulResult;
    VOS_INT                                 iRst;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����Ϸ��Լ�� */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* Asciiת�ֽ��� */
    ulResult = At_AsciiNum2HexString(gastAtParaList[0].aucPara, &gastAtParaList[0].usParaLen);

    if (ulResult != AT_SUCCESS)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���õ����ṩ�Ľӿ�ʵ�����ò��� */
    iRst = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_SET_KCE,
                            0,
                            gastAtParaList[0].aucPara,
                            (VOS_INT)(gastAtParaList[0].usParaLen / sizeof(VOS_UINT32)));

    /* ���ݵ���ӿڷ��صĽ���������ý�� */
    /* ������1ʱ�����ظ�����,����0ʱ���óɹ�,�����������ʧ�� */
    if (iRst == AT_EFUSE_REPEAT)
    {
        return AT_ERROR;
    }
    else if (iRst == AT_EFUSE_OK)
    {
        return AT_OK;
    }
    else
    {
        AT_WARN_LOG("AT_SetSecureStatePara : Set KCE req failed.");
    }

    return AT_CME_UNKNOWN;
}
#endif


VOS_UINT32 AT_QrySocidPara(VOS_UINT8 ucIndex)
{
    VOS_INT                                 iResult;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucSocid[AT_DRV_SOCID_LEN];
    VOS_UINT32                              i;

    /* �ֲ�������ʼ�� */
    memset_s(aucSocid, sizeof(aucSocid), 0x00, AT_DRV_SOCID_LEN);
    usLength = 0;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }



    /* ���õ����ṩ�Ľ�ڲ�ѯ */

    iResult = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_GET_SOCID,
                               0,
                               aucSocid,
                               (VOS_INT)(AT_DRV_SOCID_LEN / sizeof(VOS_UINT32)));

    /* �����쳣��ѯ��� */
    if (iResult != AT_EFUSE_OK)
    {
        return AT_ERROR;
    }

    /* ������ */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: ",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* ��16����������� */
    for (i = 0; i < AT_DRV_SOCID_LEN; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%02X",
                                          aucSocid[i]);
    }

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}



VOS_UINT32 AT_SetCtzuPara(VOS_UINT8 ucIndex)
{
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(gastAtParaList[0].usParaLen == 0)
    {
        g_ulCtzuFlag = 0;
    }
    else
    {
        g_ulCtzuFlag = gastAtParaList[0].ulParaValue;
    }

    return AT_OK;
}


VOS_UINT32 AT_QryCtzuPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                           usLength;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: %d",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                     g_ulCtzuFlag);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_SetEvdoSysEvent(VOS_UINT8 ucIndex)
{
    AT_MTA_EVDO_SYS_EVENT_SET_REQ_STRU  stSysEvent;
    VOS_UINT32                          ulRslt;

    memset_s(&stSysEvent, sizeof(stSysEvent), 0x00, sizeof(AT_MTA_EVDO_SYS_EVENT_SET_REQ_STRU));

    /* ����״̬��� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������Ϊ1�����ַ������ȴ���10, 4294967295 = 0xffffffff*/
    if ((gucAtParaIndex != 1)
     || (gastAtParaList[0].usParaLen > 10))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSysEvent.ulDoSysEvent = gastAtParaList[0].ulParaValue;

    ulRslt = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_EVDO_SYS_EVENT_SET_REQ,
                                    &stSysEvent,
                                    sizeof(stSysEvent),
                                    I0_UEPS_PID_MTA);

    if (ulRslt == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EVDO_SYS_EVENT_SET;

        /* ������������״̬ */
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetEvdoSysEvent: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

}


VOS_UINT32 AT_SetDoSigMask(VOS_UINT8 ucIndex)
{
    AT_MTA_EVDO_SIG_MASK_SET_REQ_STRU   stSigMask;
    VOS_UINT32                          ulRslt;

    memset_s(&stSigMask, sizeof(stSigMask), 0x00, sizeof(AT_MTA_EVDO_SIG_MASK_SET_REQ_STRU));

    /* ����״̬��� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������Ϊ1�����ַ������ȴ���10, 4294967295 = 0xffffffff*/
    if ((gucAtParaIndex != 1)
     || (gastAtParaList[0].usParaLen > 10))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSigMask.ulDoSigMask = gastAtParaList[0].ulParaValue;

    ulRslt = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_EVDO_SIG_MASK_SET_REQ,
                                    &stSigMask,
                                    sizeof(stSigMask),
                                    I0_UEPS_PID_MTA);

    if (ulRslt == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EVDO_SIG_MASK_SET;

        /* ������������״̬ */
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetDoSigMask: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

}


VOS_UINT32 AT_RcvMtaEvdoSysEventSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSysEventSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSysEventSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EVDO_SYS_EVENT_SET)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSysEventSetCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ��AT^MEID����� */
    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaEvdoSigMaskSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigMaskSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigMaskSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EVDO_SIG_MASK_SET)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigMaskSetCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ��AT^MEID����� */
    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;

}


VOS_UINT32 AT_RcvMtaEvdoRevARLinkInfoInd(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                                               ucIndex;
    VOS_UINT32                                              ulDataLen;
    VOS_UINT32                                              ulRslt;
    AT_MTA_MSG_STRU                                        *pstMtaMsg    = VOS_NULL_PTR;
    MTA_AT_EVDO_REVA_RLINK_INFO_IND_STRU                   *pstRlinkInfo = VOS_NULL_PTR;
    VOS_UINT8                                              *pucData      = VOS_NULL_PTR;

    /* ��ʼ����Ϣ���� */
    ucIndex             = 0;
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstRlinkInfo        = (MTA_AT_EVDO_REVA_RLINK_INFO_IND_STRU*)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    ulDataLen               = pstRlinkInfo->ulParaLen * sizeof(VOS_UINT8) * 2 + 3;
    /*lint -save -e516 */
    pucData                 = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, ulDataLen);
    /*lint -restore */
    if (pucData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd(): mem alloc Fail!");
        return VOS_ERR;
    }

    memset_s(pucData, ulDataLen, 0x00, ulDataLen);
    pucData[ulDataLen - 1]  = '\0';

    ulRslt = AT_HexToAsciiString(&pucData[1], (ulDataLen - 3), pstRlinkInfo->aucContent, pstRlinkInfo->ulParaLen);

    pucData[0]              = '"';
    pucData[ulDataLen - 2]  = '"';

    if (ulRslt != AT_OK)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd: WARNING: Hex to Ascii trans fail!");

        PS_MEM_FREE(WUEPS_PID_AT, pucData);

        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DOREVARLINK: %d,%s%s",
                                                    gaucAtCrLf,
                                                    pstRlinkInfo->ulParaLen * 2,
                                                    pucData,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    PS_MEM_FREE(WUEPS_PID_AT, pucData);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaEvdoSigExEventInd(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex;
    AT_MTA_MSG_STRU                    *pstMtaMsg       = VOS_NULL_PTR;
    MTA_AT_EVDO_SIG_EXEVENT_IND_STRU   *pstSigExEvent   = VOS_NULL_PTR;
    VOS_UINT8                          *pucData         = VOS_NULL_PTR;
    VOS_UINT32                          ulDataLen;
    VOS_UINT32                          ulRslt;

    /* ��ʼ����Ϣ���� */
    ucIndex             = 0;
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstSigExEvent       = (MTA_AT_EVDO_SIG_EXEVENT_IND_STRU*)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    ulDataLen               = pstSigExEvent->ulParaLen * sizeof(VOS_UINT8) * 2 + 3;
    /*lint -save -e516 */
    pucData                 = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, ulDataLen);
    /*lint -restore */
    if (pucData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd(): mem alloc Fail!");
        return VOS_ERR;
    }

    memset_s(pucData, ulDataLen, 0x00, ulDataLen);
    pucData[ulDataLen - 1]  = '\0';

    ulRslt = AT_HexToAsciiString(&pucData[1], (ulDataLen - 3), pstSigExEvent->aucContent, pstSigExEvent->ulParaLen);

    pucData[0]              = '"';
    pucData[ulDataLen - 2]  = '"';

    if (ulRslt != AT_OK)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd: WARNING: Hex to Ascii trans fail!");

        PS_MEM_FREE(WUEPS_PID_AT, pucData);

        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DOSIGEXEVENT: %d,%s%s",
                                                    gaucAtCrLf,
                                                    pstSigExEvent->ulParaLen * 2,
                                                    pucData,
                                                    gaucAtCrLf);

    PS_MEM_FREE(WUEPS_PID_AT, pucData);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaNoCardModeSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU             *pstCnf  = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNoCardModeSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNoCardModeSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NOCARDMODE_SET)
    {
        AT_WARN_LOG("AT_RcvMtaNoCardModeSetCnf : Current Option is not AT_CMD_NOCARDMODE_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ��AT^CNOCARDMODE����� */
    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaNoCardModeQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_NO_CARD_MODE_QRY_CNF_STRU   *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_NO_CARD_MODE_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNoCardModeQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNoCardModeQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NOCARDMODE_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaNoCardModeQryCnf : Current Option is not AT_CMD_NOCARDMODE_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     "%s: %d",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     pstMtaCnf->ulEnableFlag);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_SetCdmaModemCapPara(
    PLATAFORM_RAT_CAPABILITY_STRU      *pstSourceModemPlatform,
    PLATAFORM_RAT_CAPABILITY_STRU      *pstDestinationModemPlatform,
    MODEM_ID_ENUM_UINT16                enSourceModemId,
    MODEM_ID_ENUM_UINT16                enDestinationModemId,
    VOS_UINT8                           ucIndex
)
{
    AT_MTA_CDMA_MODEM_CAP_SET_REQ_STRU  stAtCmd;
    errno_t                             lMemResult;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(stAtCmd));

    AT_WARN_LOG2("AT_SetCdmaModemCapPara: Entry enSourceModemId and enDestinationModemId !",
                        enDestinationModemId, enDestinationModemId);

    stAtCmd.enSourceModemId      = enSourceModemId;
    stAtCmd.enDestinationModemId = enDestinationModemId;
    lMemResult = memcpy_s(&(stAtCmd.stSourceModemPlatform), sizeof(stAtCmd.stSourceModemPlatform),
                          pstSourceModemPlatform, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmd.stSourceModemPlatform), sizeof(PLATAFORM_RAT_CAPABILITY_STRU));

    lMemResult = memcpy_s(&(stAtCmd.stDestinationModemPlatform), sizeof(stAtCmd.stDestinationModemPlatform),
                          pstDestinationModemPlatform, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmd.stDestinationModemPlatform), sizeof(PLATAFORM_RAT_CAPABILITY_STRU));

    if (AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                           0,
                           ID_AT_MTA_CDMA_MODEM_CAP_SET_REQ,
                           &stAtCmd,
                           sizeof(AT_MTA_CDMA_MODEM_CAP_SET_REQ_STRU),
                           I0_UEPS_PID_MTA) == TAF_SUCCESS)
    {
        AT_ERR_LOG("AT_SetCdmaModemCapPara,AT_FillAndSndAppReqMsg return SUCCESS");
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CDMAMODEMSWITCH_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        AT_ERR_LOG("AT_SetCdmaModemCapPara,AT_FillAndSndAppReqMsg return ERROR");
        return AT_ERROR;
    }
}


LOCAL VOS_UINT32 AT_CdmaModemIdSet(
    VOS_UINT32                          ulModem0SupportCMode,
    VOS_UINT32                          ulModem1SupportCMode,
    AT_MTA_CDMA_CAP_RESUME_SET_REQ_STRU *pstAtCmd
)
{
    if ((ulModem0SupportCMode == VOS_TRUE) && (ulModem1SupportCMode == VOS_FALSE))
    {
        pstAtCmd->enCdmaModemId = MODEM_ID_0;
        return AT_SUCCESS;
    }
    else if ((ulModem0SupportCMode == VOS_FALSE) && (ulModem1SupportCMode == VOS_TRUE))
    {
        pstAtCmd->enCdmaModemId = MODEM_ID_1;
        return AT_SUCCESS;
    }
    else if ((ulModem0SupportCMode == VOS_FALSE) && (ulModem1SupportCMode == VOS_FALSE))
    {
        /*  ����modem��û��cdma����������Ҫ�ָ� */
        AT_NORM_LOG("At_SetCdmaCapResume:Modem0 and modem1 not have cdma capa!");
        return AT_OK;
    }
    else /* ��ǰ��֧��˫cdma������������ã�����ʧ�ܣ�������ʼ��ʧ�� */
    {
        AT_ERR_LOG("At_SetCdmaCapResume:Modem0 and modem1 both have cdma capa!");
        return AT_ERROR;
    }
}


VOS_UINT32 At_SetCdmaCapResume(VOS_UINT8 ucIndex)
{
    PLATAFORM_RAT_CAPABILITY_STRU       stModem0PlatRat;
    PLATAFORM_RAT_CAPABILITY_STRU       stModem1PlatRat;
    AT_MTA_CDMA_CAP_RESUME_SET_REQ_STRU stAtCmd;
    VOS_UINT32                          ulModem0Support1XFlg;
    VOS_UINT32                          ulModem0SupportHrpdFlg;
    VOS_UINT32                          ulModem1Support1XFlg;
    VOS_UINT32                          ulModem1SupportHrpdFlg;
    VOS_UINT32                          ulModem0SupportCMode;
    VOS_UINT32                          ulModem1SupportCMode;
    VOS_UINT32                          ulRslt;
    errno_t                             lMemResult;
    MODEM_ID_ENUM_UINT16                enModemId;


    memset_s(&stModem0PlatRat, sizeof(stModem0PlatRat), 0x00, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    memset_s(&stModem1PlatRat, sizeof(stModem1PlatRat), 0x00, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_CDMA_CAP_RESUME_SET_REQ_STRU));

    AT_WARN_LOG("At_SetCdmaCapResume Entry: ");

    /* �������֧����Modem0�Ϸ��� */
    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("At_SetCdmaCapResume: Get modem id fail!");

        return AT_ERROR;
    }

    if (enModemId != MODEM_ID_0)
    {
        AT_ERR_LOG1("At_SetCdmaCapResume:  modem is not modem0!", enModemId);
        return AT_ERROR;
    }

    /* ��ȡnv���������� */
    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_0, en_NV_Item_Platform_RAT_CAP, &stModem0PlatRat,
                                            sizeof(PLATAFORM_RAT_CAPABILITY_STRU)) != NV_OK)
    {
        AT_ERR_LOG("At_SetCdmaCapResume: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem0!");
        return AT_ERROR;
    }
    else
    {
        AT_WARN_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, stModem0PlatRat);
    }

    /* ��ȡnv���������� */
    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_1, en_NV_Item_Platform_RAT_CAP, &stModem1PlatRat,
                                            sizeof(PLATAFORM_RAT_CAPABILITY_STRU)) != NV_OK)
    {
        AT_ERR_LOG("At_SetCdmaCapResume: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem1!");
        return AT_ERROR;
    }
    else
    {
        AT_WARN_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, stModem1PlatRat);
    }
    if ((stModem0PlatRat.usRatNum == 0)
     || (stModem1PlatRat.usRatNum == 0))
    {
        AT_ERR_LOG("At_SetCdmaCapResume: Factory PlatForm RatNum is 0");
        return AT_ERROR;
    }

    lMemResult = memcpy_s(&(stAtCmd.stModem0Platform), sizeof(stAtCmd.stModem0Platform),
                          &stModem0PlatRat, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmd.stModem0Platform), sizeof(PLATAFORM_RAT_CAPABILITY_STRU));

    lMemResult = memcpy_s(&(stAtCmd.stModem1Platform), sizeof(stAtCmd.stModem1Platform),
                          &stModem1PlatRat, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAtCmd.stModem1Platform), sizeof(PLATAFORM_RAT_CAPABILITY_STRU));

    /* ���cdma�������ĸ�modem�ϣ�Ϊ�˷�ֹ��������modem������cdma����(�Ḵλ)��
       �ָ�ʱ���Ȼָ���cdma������modem  */
    ulModem0Support1XFlg   = AT_IsPlatformSupport1XMode(&stModem0PlatRat);
    ulModem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&stModem0PlatRat);
    ulModem1Support1XFlg   = AT_IsPlatformSupport1XMode(&stModem1PlatRat);
    ulModem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&stModem1PlatRat);

    ulModem0SupportCMode = (ulModem0Support1XFlg | ulModem0SupportHrpdFlg);
    ulModem1SupportCMode = (ulModem1Support1XFlg | ulModem1SupportHrpdFlg);

    ulRslt = AT_CdmaModemIdSet(ulModem0SupportCMode, ulModem1SupportCMode, &stAtCmd);
    if (ulRslt != AT_SUCCESS)
    {
        return ulRslt;
    }

    if (AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                           0,
                           ID_AT_MTA_CDMA_CAP_RESUME_SET_REQ,
                           &stAtCmd,
                           sizeof(AT_MTA_CDMA_CAP_RESUME_SET_REQ_STRU),
                           I0_UEPS_PID_MTA) == TAF_SUCCESS)
    {
        AT_ERR_LOG("At_SetCdmaCapResume, AT_FillAndSndAppReqMsg CDMA_CAP_RESUME_SET_REQ return SUCCESS");
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CDMACAPRESUME_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        AT_ERR_LOG("At_SetCdmaCapResume,AT_FillAndSndAppReqMsg return ERROR");
        return AT_ERROR;
    }
}


VOS_UINT32 At_QryFactoryCdmaCap(VOS_UINT8 ucIndex)
{

    PLATAFORM_RAT_CAPABILITY_STRU       stModem0PlatRat;
    PLATAFORM_RAT_CAPABILITY_STRU       stModem1PlatRat;
    VOS_UINT32                          ulModem0Support1XFlg;
    VOS_UINT32                          ulModem0SupportHrpdFlg;
    VOS_UINT32                          ulModem1Support1XFlg;
    VOS_UINT32                          ulModem1SupportHrpdFlg;
    VOS_UINT32                          ulModem0SupportCMode;
    VOS_UINT32                          ulModem1SupportCMode;
    VOS_UINT32                          ulCheckRlt;
    VOS_UINT32                          ulAllModemNotSupportCMode;

    VOS_UINT16                          usLength;

    memset_s(&stModem0PlatRat, sizeof(stModem0PlatRat), 0x00, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    memset_s(&stModem1PlatRat, sizeof(stModem1PlatRat), 0x00, sizeof(PLATAFORM_RAT_CAPABILITY_STRU));
    ulCheckRlt                = AT_SUCCESS;
    ulAllModemNotSupportCMode = VOS_FALSE;

    AT_NORM_LOG("At_QryFactoryCdmaCap Entry: ");

    /*  ��ѯ��ȡnv���޸�Ϊ�ӱ�������ȡ�����ӹ�������ȡ */
    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_0, en_NV_Item_Platform_RAT_CAP, &stModem0PlatRat,
                              sizeof(PLATAFORM_RAT_CAPABILITY_STRU)) != NV_OK)
    {
        AT_WARN_LOG("Read PlatForm from Factory fail: MODEM_ID_0");
        return AT_ERROR;
    }
    else
    {
        AT_NORM_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, stModem0PlatRat);
    }

    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_1, en_NV_Item_Platform_RAT_CAP, &stModem1PlatRat,
                              sizeof(PLATAFORM_RAT_CAPABILITY_STRU)) != NV_OK)
    {
        AT_WARN_LOG("Read PlatForm from Factory fail: MODEM_ID_1");
        return AT_ERROR;
    }
    else
    {
        AT_NORM_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_1, stModem1PlatRat);
    }

    ulModem0Support1XFlg   = AT_IsPlatformSupport1XMode(&stModem0PlatRat);
    ulModem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&stModem0PlatRat);
    ulModem1Support1XFlg   = AT_IsPlatformSupport1XMode(&stModem1PlatRat);
    ulModem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&stModem1PlatRat);

    AT_NORM_LOG2("At_QryFactoryCdmaCap modem0 cdma cpa: ", ulModem0Support1XFlg, ulModem0SupportHrpdFlg);
    AT_NORM_LOG2("At_QryFactoryCdmaCap modem1 cdma cpa: ", ulModem1Support1XFlg, ulModem1SupportHrpdFlg);

    ulModem0SupportCMode = (ulModem0Support1XFlg | ulModem0SupportHrpdFlg);
    ulModem1SupportCMode = (ulModem1Support1XFlg | ulModem1SupportHrpdFlg);

    /* ���modem0��modem1ͬʱ����֧�֣�����Ϊ����ʧ�� */
    if ( (ulModem0SupportCMode == VOS_TRUE)
      && (ulModem1SupportCMode == VOS_TRUE) )
    {
        ulCheckRlt = AT_ERROR;
    }

    if ( (ulModem0SupportCMode == VOS_FALSE)
      && (ulModem1SupportCMode == VOS_FALSE))
    {
        ulCheckRlt                = AT_ERROR;
        ulAllModemNotSupportCMode = VOS_TRUE;
    }

    if (ulCheckRlt == AT_ERROR)
    {
        if (ulAllModemNotSupportCMode == VOS_TRUE)
        {
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: -1",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

            gstAtSendData.usBufLen = usLength;

            return AT_OK;
        }

        return AT_ERROR;
    }

    /* 1X,DO��Modem0 */
    if (ulModem0SupportCMode == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         "%s: 0",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        gstAtSendData.usBufLen = usLength;

        return AT_OK;
    }

    /* 1X,DO��Modem1 */
    if (ulModem1SupportCMode == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         "%s: 1",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        gstAtSendData.usBufLen = usLength;
    }

    return AT_OK;
}

#endif


VOS_VOID AT_ReadPlatFormPrint(
    MODEM_ID_ENUM_UINT16                enModemId,
    PLATAFORM_RAT_CAPABILITY_STRU       stPlatRat
)
{
    VOS_UINT32                          i;

    AT_WARN_LOG1("Read PlatForm ModemId: ", enModemId);
    AT_WARN_LOG1("Read PlatForm RatNum: ", stPlatRat.usRatNum);

    for (i = 0; (i < stPlatRat.usRatNum) && (i < PLATFORM_MAX_RAT_NUM); i++)
    {
        AT_WARN_LOG1("Read PlatForm RatType: ", stPlatRat.aenRatList[i]);
    }
}


VOS_UINT32 AT_SetFratIgnitionPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    AT_MTA_FRAT_IGNITION_ENUM_UNIT8     enFratIgnitionSta;

    /* ������� */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ŀ��� */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ucFratIgnitionSta ȡֵ���� */
    if ((gastAtParaList[0].ulParaValue >= AT_MTA_FRAT_IGNITION_STATT_BUTT)
     || (gastAtParaList[0].usParaLen == 0))
    {
        AT_WARN_LOG1("AT_SetFratIgnitionPara: para err", gastAtParaList[0].ulParaValue);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    enFratIgnitionSta = (AT_MTA_FRAT_IGNITION_ENUM_UNIT8)gastAtParaList[0].ulParaValue;

    /* ������Ϣ ID_AT_MTA_FRAT_IGNITION_SET_REQ ��MTA��������Ϣ������(VOS_UINT8)gastAtParaList[0].ulParaValue */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_FRAT_IGNITION_SET_REQ,
                                   (VOS_VOID *)&enFratIgnitionSta,
                                   sizeof(enFratIgnitionSta),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FRATIGNITION_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetFratIgnitionPara: send ReqMsg fail");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_CheckLineIndexListBsPara(VOS_VOID)
{
    VOS_UINT32                          ulLoop;

    /* ��ȡЯ��BS1��BS2��BS3�����ܳ���,BS1�ǵ�4����������gastAtParaList[3]������BS1�����Ϣ */
    for (ulLoop = 3; ulLoop < gucAtParaIndex; ulLoop++)
    {
        /* ��������BS1������BS2������BS3���ڿն����߳��ȴ���500��ֱ�ӷ��ش���
           ����AT^LINEINDEXlIST=255,"00.00.001",0,,BS2 (����BS1���ڿն�,����Ϊ0),
           AT^LINEINDEXlIST=255,"00.00.001",0,BS1,     (����BS2���ڿն�,����Ϊ0) */
        if ((gastAtParaList[ulLoop].usParaLen == 0)
         || (gastAtParaList[ulLoop].usParaLen > AT_CMD_LINE_INDEX_LIST_BS_MAX_LENGTH))
        {
            AT_ERR_LOG("AT_CheckLineIndexListBsPara: input BS format is error");

            return AT_CME_INCORRECT_PARAMETERS;
        }

    }
    return AT_SUCCESS;
}


VOS_UINT32 AT_CheckLineIndexListPara(VOS_VOID)
{
    /* ������������ȷ,3~6������ */
    if((gucAtParaIndex > 6)
    || (gucAtParaIndex < 3))
    {
       AT_ERR_LOG("AT_CheckLineIndexListPara:number of parameter error.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������Ȳ��� */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen != AT_CSS_CLOUD_LINE_VERSION_LEN)
     || (gastAtParaList[2].usParaLen == 0))
    {
        AT_ERR_LOG("AT_CheckLineIndexListPara:para len error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������������·������Ϣ����������С�ڵ���3�� */
    if ((gastAtParaList[2].ulParaValue == AT_CSS_LINE_INDEX_LIST_ADD)
     && (gucAtParaIndex <= 3))
    {
       AT_ERR_LOG("AT_CheckLineIndexListPara:too little para when add line index list.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������������·������Ϣ��BS1����С��0 */
    if ((gastAtParaList[2].ulParaValue == AT_CSS_LINE_INDEX_LIST_ADD)
     && (gastAtParaList[3].usParaLen == 0))
    {
       AT_ERR_LOG("AT_CheckLineIndexListPara:BS1 length is 0 when add line index list.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����ɾ�����е���ͨ�Ÿ���������Ϣ������ҪЯ��BS1��BS2��BS3����length����Ϊ0 */
    if ((gastAtParaList[2].ulParaValue == AT_CSS_LINE_INDEX_LIST_DELETE_ALL)
     && (gucAtParaIndex > 3))
    {
       AT_ERR_LOG("AT_CheckLineIndexListPara:too mang para when delete all line index list.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����BS��ʽ����ȷ���������ڿն���BS�ܳ��Ȳ�����length���� */
    if (AT_CheckLineIndexListBsPara() != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_CheckLineIndexListPara:input BS para is error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_CheckLineDetailBsPara(VOS_VOID)
{
    VOS_UINT32                          ulLoop;

    /* ��ȡЯ��BS1��BS2��BS3�����ܳ���,BS1�ǵ�4����������gastAtParaList[3]������BS1�����Ϣ */
    for (ulLoop = 3; ulLoop < gucAtParaIndex; ulLoop++)
    {
        /* ��������BS1������BS2������BS3���ڿն����߳��ȴ���500��ֱ�ӷ��ش���
           ����AT^LINEINDEXlIST=255,"00.00.001",0,,BS2 (����BS1���ڿն�,����Ϊ0),
           AT^LINEINDEXlIST=255,"00.00.001",0,BS1,     (����BS2���ڿն�,����Ϊ0) */
        if ((gastAtParaList[ulLoop].usParaLen == 0)
         || (gastAtParaList[ulLoop].usParaLen > AT_CMD_LINE_DETAIL_BS_MAX_LENGTH))
        {
            AT_ERR_LOG("AT_CheckLineIndexListBsPara: input BS format is error");

            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    return AT_SUCCESS;
}


VOS_UINT32 AT_CheckLineDetailPara(VOS_VOID)
{
    /* ������������ȷ,3~6������ */
    if((gucAtParaIndex > 6)
    || (gucAtParaIndex < 3))
    {
       AT_ERR_LOG("AT_CheckLineDetailPara:number of parameter error.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������Ȳ��� */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen != AT_CSS_CLOUD_LINE_VERSION_LEN)
     || (gastAtParaList[2].usParaLen == 0))
    {
        AT_ERR_LOG("AT_CheckLineDetailPara:para len error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������������·������Ϣ����������С��3�� */
    if ((gastAtParaList[2].ulParaValue == AT_CSS_LINE_DETAIL_ADD)
     && (gucAtParaIndex <= 3))
    {
       AT_ERR_LOG("AT_CheckLineDetailPara:too little para when add line index list.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������������·������Ϣ��BS1����С��0 */
    if ((gastAtParaList[2].ulParaValue == AT_CSS_LINE_DETAIL_ADD)
     && (gastAtParaList[3].usParaLen == 0))
    {
       AT_ERR_LOG("AT_CheckLineDetailPara:BS1 length is 0 when add line index list.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����ɾ�����е���ͨ�Ÿ���������Ϣ������ҪЯ��BS1��BS2��BS3����length����Ϊ0 */
    if ((gastAtParaList[2].ulParaValue == AT_CSS_LINE_DETAIL_DELETE_ALL)
     && (gucAtParaIndex > 3))
    {
       AT_ERR_LOG("AT_CheckLineDetailPara:too mang para when delete all line index list.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �����BS��ʽ����ȷ���������ڿն���BS�ܳ��Ȳ�����length���� */
    if (AT_CheckLineDetailBsPara() != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_CheckLineDetailPara:input BS para is error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_SetLineIndexListPara(
    VOS_UINT8                           ucIndex
)
{
    AT_CSS_LINE_INDEX_LIST_SET_REQ_STRU                    *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                              ulBufLen;
    VOS_UINT32                                              ulBsLen;
    VOS_UINT32                                              ulResult;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulDeltaLen;
    errno_t                                                 lMemResult;
    MODEM_ID_ENUM_UINT16                                    enModemId;
    VOS_UINT32                                              ulRet;

    enModemId = MODEM_ID_0;
    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_SetMccFreqPara: Get modem id fail.");
        return AT_ERROR;
    }

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_ERR_LOG("AT_SetLineIndexListPara:Cmd Opt Type is wrong.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    ulResult = AT_CheckLineIndexListPara();
    if (ulResult != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_SetLineIndexListPara:check line index list para error.");

        return ulResult;
    }

    ulBsLen  = gastAtParaList[3].usParaLen + gastAtParaList[4].usParaLen + gastAtParaList[5].usParaLen;
    ulBufLen = sizeof(AT_CSS_LINE_INDEX_LIST_SET_REQ_STRU);

    if (ulBsLen > 4)
    {
        ulBufLen += ulBsLen - 4;
    }

    /* ������Ϣ��AT_CSS_LINE_INDEX_LIST_SET_REQ_STRU */
    pstMsg = (AT_CSS_LINE_INDEX_LIST_SET_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(ulBufLen);

    /* �ڴ�����ʧ�ܣ����� */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SetLineIndexListPara:memory alloc fail.");

        return AT_ERROR;
    }

    TAF_MEM_SET_S((VOS_INT8 *)pstMsg + VOS_MSG_HEAD_LENGTH,
                  (VOS_SIZE_T)ulBufLen - VOS_MSG_HEAD_LENGTH,
                  0x00,
                  (VOS_SIZE_T)ulBufLen - VOS_MSG_HEAD_LENGTH);

    /* ��д��Ϣͷ */
    AT_CFG_MSG_HDR(pstMsg, PS_PID_CSS, ID_AT_CSS_LINE_INDEX_LIST_SET_REQ);

    /* ��д��Ϣ���� */
    pstMsg->usClientId                  = gastAtClientTab[ucIndex].usClientId;
    pstMsg->ucSeq                       = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    pstMsg->usModemId                   = enModemId;

    lMemResult = memcpy_s(pstMsg->aucVersionId,
                          sizeof(pstMsg->aucVersionId),
                          gastAtParaList[1].aucPara,
                          gastAtParaList[1].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstMsg->aucVersionId), gastAtParaList[1].usParaLen);

    pstMsg->enOperateType               = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    pstMsg->ulLineIndexListBuffLen      = ulBsLen;

    ulDeltaLen = 0;
    for (ulLoop = 3; ulLoop < 6; ulLoop++)
    {
        if (gastAtParaList[ulLoop].usParaLen > 0)
        {
            /* BS��󳤶���AT_CMD_LINE_INDEX_LIST_BS_MAX_LENGTH */
            lMemResult = memcpy_s(pstMsg->aucLineIndexListBuff + ulDeltaLen,
                                  (ulBsLen - ulDeltaLen),
                                  gastAtParaList[ulLoop].aucPara,
                                  gastAtParaList[ulLoop].usParaLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (ulBsLen - ulDeltaLen), gastAtParaList[ulLoop].usParaLen);
            ulDeltaLen =  ulDeltaLen + gastAtParaList[ulLoop].usParaLen;
        }
        else
        {
            break;
        }
    }

    /* ������Ϣ��������������״̬ */
    AT_SEND_MSG(pstMsg);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LINEINDEXLIST_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetLineDetailPara(
    VOS_UINT8                           ucIndex
)
{
    errno_t                             lMemResult;
    AT_CSS_LINE_DETAIL_SET_REQ_STRU    *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulBufLen;
    VOS_UINT32                          ulBsLen;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulDeltaLen;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRet;

    enModemId = MODEM_ID_0;
    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_SetMccFreqPara: Get modem id fail.");
        return AT_ERROR;
    }

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_ERR_LOG("AT_SetLineDetailPara:Cmd Opt Type is wrong.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    ulResult = AT_CheckLineDetailPara();
    if (ulResult != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_SetLineDetailPara:check line index list para error.");

        return ulResult;
    }

    ulBsLen  = gastAtParaList[3].usParaLen + gastAtParaList[4].usParaLen + gastAtParaList[5].usParaLen;
    ulBufLen = sizeof(AT_CSS_LINE_DETAIL_SET_REQ_STRU);

    if (ulBsLen > 4)
    {
        ulBufLen += ulBsLen - 4;
    }

    /* ������Ϣ��AT_CSS_LINE_DETAIL_SET_REQ_STRU */
    pstMsg = (AT_CSS_LINE_DETAIL_SET_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(ulBufLen);

    /* �ڴ�����ʧ�ܣ����� */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SetLineDetailPara:memory alloc fail.");

        return AT_ERROR;
    }

    TAF_MEM_SET_S((VOS_INT8 *)pstMsg + VOS_MSG_HEAD_LENGTH,
                  (VOS_SIZE_T)ulBufLen - VOS_MSG_HEAD_LENGTH,
                  0x00,
                  (VOS_SIZE_T)ulBufLen - VOS_MSG_HEAD_LENGTH);

    /* ��д��Ϣͷ */
    AT_CFG_MSG_HDR(pstMsg, PS_PID_CSS, ID_AT_CSS_LINE_DETAIL_SET_REQ);

    /* ��д��Ϣ���� */
    pstMsg->usModemId                   = enModemId;
    pstMsg->usClientId                  = gastAtClientTab[ucIndex].usClientId;
    pstMsg->ucSeq                       = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    lMemResult = memcpy_s(pstMsg->aucVersionId,
                  sizeof(pstMsg->aucVersionId),
                  gastAtParaList[1].aucPara,
                  gastAtParaList[1].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstMsg->aucVersionId), gastAtParaList[1].usParaLen);

    pstMsg->enOperateType            = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    pstMsg->ulLineDetailBuffLen      = ulBsLen;

    ulDeltaLen = 0;
    for (ulLoop = 3; ulLoop < 6; ulLoop++)
    {
        if (gastAtParaList[ulLoop].usParaLen > 0)
        {
            /* BS��󳤶���AT_CMD_LINE_INDEX_LIST_BS_MAX_LENGTH */
            lMemResult = memcpy_s(pstMsg->aucLineDetailBuff + ulDeltaLen,
                          (ulBsLen - ulDeltaLen),
                          gastAtParaList[ulLoop].aucPara,
                          gastAtParaList[ulLoop].usParaLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (ulBsLen - ulDeltaLen), gastAtParaList[ulLoop].usParaLen);
            ulDeltaLen =  ulDeltaLen + gastAtParaList[ulLoop].usParaLen;
        }
        else
        {
            break;
        }
    }

    /* ������Ϣ��������������״̬ */
    AT_SEND_MSG(pstMsg);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LINEDETAIL_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryLineIndexListPara(
    VOS_UINT8                           ucIndex
)
{
    AT_CSS_LINE_INDEX_LIST_QUERY_REQ_STRU                  *pstMsg = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                                    enModemId;
    VOS_UINT32                                              ulRet;

    enModemId = MODEM_ID_0;

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_SetMccFreqPara: Get modem id fail.");
        return AT_ERROR;
    }

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        AT_ERR_LOG("AT_QryLineIndexListPara: Invalid Cmd Type");

        return AT_ERROR;
    }

    /* Allocating memory for message */
    /*lint -save -e516 */
    pstMsg = (AT_CSS_LINE_INDEX_LIST_QUERY_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_CSS_LINE_INDEX_LIST_QUERY_REQ_STRU));
    /*lint -restore */

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_QryLineIndexListPara: Mem Alloc failed");

        return AT_ERROR;
    }

    TAF_MEM_SET_S(((VOS_UINT8 *)pstMsg + VOS_MSG_HEAD_LENGTH),
                  (VOS_SIZE_T)(sizeof(AT_CSS_LINE_INDEX_LIST_QUERY_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                  0x00,
                  (VOS_SIZE_T)(sizeof(AT_CSS_LINE_INDEX_LIST_QUERY_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��д��Ϣͷ */
    AT_CFG_MSG_HDR(pstMsg, PS_PID_CSS, ID_AT_CSS_LINE_INDEX_LIST_QUERY_REQ);
    pstMsg->usModemId           = enModemId;
    pstMsg->usClientId          = gastAtClientTab[ucIndex].usClientId;

    AT_SEND_MSG(pstMsg);

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LINEINDEXLIST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TimeParaYTDCheck(AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];
    errno_t                             lMemResult;

    /* ������������VER���Ȳ���ȷ */
    if (gastAtParaList[0].usParaLen != AT_MODEM_YTD_LEN)
    {
       AT_ERR_LOG("AT_TimeParaYTDCheck: length of YTD parameter is error.");
       return VOS_ERR;
    }

    /* ���ո�ʽ YYYY/MM/DD ���������գ����жϸ�ʽ����Χ */
    if ((gastAtParaList[0].aucPara[4] != '/')
     || (gastAtParaList[0].aucPara[7] != '/'))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The date formats parameter is error.");
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[0].aucPara, AT_MODEM_YEAR_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_YEAR_LEN);
    pstAtMtaModemTime->lYear = (VOS_INT32)AT_AtoI((VOS_CHAR *)aucBuffer);

    if ((pstAtMtaModemTime->lYear > AT_MODEM_YEAR_MAX)
     || (pstAtMtaModemTime->lYear < AT_MODEM_YEAR_MIN))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The parameter of year is out of range");
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[5], AT_MODEM_MONTH_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_MONTH_LEN);
    pstAtMtaModemTime->lMonth = (VOS_INT32)AT_AtoI((VOS_CHAR *)aucBuffer);

    if ((pstAtMtaModemTime->lMonth > AT_MODEM_MONTH_MAX)
     || (pstAtMtaModemTime->lMonth < AT_MODEM_MONTH_MIN))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The parameter of month is out of range");
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[8], AT_MODEM_DATE_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_DATE_LEN);
    pstAtMtaModemTime->lDay = (VOS_INT32)AT_AtoI((VOS_CHAR *)aucBuffer);

    if ((pstAtMtaModemTime->lDay > AT_MODEM_DAY_MAX)
     || (pstAtMtaModemTime->lDay < AT_MODEM_DAY_MIN))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The parameter of day is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_TimeParaTimeCheck(AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];
    errno_t                             lMemResult;

    if (gastAtParaList[1].usParaLen != AT_MODEM_TIME_LEN)
    {
       AT_ERR_LOG("AT_TimeParaTimeCheck: length of time parameter is error.");
       return VOS_ERR;
    }

    /* ���ո�ʽ HH:MM:SS ����ʱ�䣬���жϸ�ʽ����Χ */
    if ((gastAtParaList[1].aucPara[2] != ':')
     || (gastAtParaList[1].aucPara[5] != ':'))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The time formats parameter is error.");
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[1].aucPara, AT_MODEM_HOUR_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_HOUR_LEN);
    pstAtMtaModemTime->lHour = (VOS_INT32)AT_AtoI((VOS_CHAR *)aucBuffer);

    if ((pstAtMtaModemTime->lHour > AT_MODEM_HOUR_MAX)
     || (pstAtMtaModemTime->lHour < AT_MODEM_HOUR_MIN))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The parameter of hour is out of range");
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[1].aucPara[3], AT_MODEM_MIN_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_MIN_LEN);
    pstAtMtaModemTime->lMin = (VOS_INT32)AT_AtoI((VOS_CHAR *)aucBuffer);

    if ((pstAtMtaModemTime->lMin > AT_MODEM_MIN_MAX)
     || (pstAtMtaModemTime->lMin < AT_MODEM_MIN_MIN))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The parameter of min is out of range");
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[1].aucPara[6], AT_MODEM_SEC_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_SEC_LEN);
    pstAtMtaModemTime->lSec = (VOS_INT32)AT_AtoI((VOS_CHAR *)aucBuffer);

    if ((pstAtMtaModemTime->lSec > AT_MODEM_SEC_MAX)
     || (pstAtMtaModemTime->lSec < AT_MODEM_SEC_MIN))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The parameter of second is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_TimeParaZoneCheck(AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];
    errno_t                             lMemResult;

    if ( (gastAtParaList[2].usParaLen == 0)
      || (gastAtParaList[2].usParaLen >= AT_GET_MODEM_TIME_BUFF_LEN) )
    {
        AT_ERR_LOG1("AT_TimeParaZoneCheck: length of zone parameter is wrong.", gastAtParaList[2].usParaLen);
        return VOS_ERR;
    }

    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[2].aucPara, gastAtParaList[2].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[2].usParaLen);

    /* ʱ����Χ��[-12, 12] */
    if (AT_AtoInt((VOS_CHAR *)aucBuffer, &pstAtMtaModemTime->lZone) == VOS_ERR)
    {
        return VOS_ERR;
    }

    if ((pstAtMtaModemTime->lZone > AT_MODEM_ZONE_MAX)
    ||  (pstAtMtaModemTime->lZone < AT_MODEM_ZONE_MIN))
    {
        AT_ERR_LOG("AT_TimeParaZoneCheck: The parameter of zone is out of range.");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetModemTimePara(VOS_UINT8 ucIndex)
{
    AT_MTA_MODEM_TIME_STRU              stAtMtaModemTime;
    VOS_UINT32                          ulRst;

    /* ������� */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ŀ��� */
    if (gucAtParaIndex != 3)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������ʽ��� */
    memset_s(&stAtMtaModemTime, (VOS_SIZE_T)sizeof(stAtMtaModemTime), 0x00, (VOS_SIZE_T)sizeof(stAtMtaModemTime));

    /* ��������� */
    if (AT_TimeParaYTDCheck(&stAtMtaModemTime) == VOS_ERR)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ʱ��*/
    if (AT_TimeParaTimeCheck(&stAtMtaModemTime) == VOS_ERR)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ʱ�� */
    if (AT_TimeParaZoneCheck(&stAtMtaModemTime) == VOS_ERR)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϣ ID_AT_MTA_MODEM_TIME_SET_REQ ��MTA��������Ϣ������ stAtMtaModemTime */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_MODEM_TIME_SET_REQ,
                                   (VOS_VOID *)&stAtMtaModemTime,
                                   (VOS_SIZE_T)sizeof(stAtMtaModemTime),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MODEM_TIME_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetModemTimePara: send ReqMsg fail");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetPhyComCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_PHY_COM_CFG_SET_REQ_STRU     stPhyComCfg;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stPhyComCfg, (VOS_SIZE_T)sizeof(stPhyComCfg), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_PHY_COM_CFG_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetPhyComCfg : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �жϲ������� */
    if ((gucAtParaIndex < 3) || (gucAtParaIndex > 5))
    {
        /* ������������ */
        AT_WARN_LOG("AT_SetPhyComCfg : Current Number wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��Ϣ��ֵ */
    stPhyComCfg.usCmdType               = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stPhyComCfg.usRatBitmap             = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stPhyComCfg.ulPara1                 = gastAtParaList[2].ulParaValue;

    if (gucAtParaIndex == 4)
    {
        stPhyComCfg.ulPara2             = gastAtParaList[3].ulParaValue;
    }
    else
    {
        stPhyComCfg.ulPara2             = gastAtParaList[3].ulParaValue;
        stPhyComCfg.ulPara3             = gastAtParaList[4].ulParaValue;
    }

    /* ������Ϣ��MTA */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_PHY_COM_CFG_SET_REQ,
                                      &stPhyComCfg,
                                      (VOS_SIZE_T)sizeof(stPhyComCfg),
                                      I0_UEPS_PID_MTA);

    /* ����ʧ�� */
    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetPhyComCfg: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ���ͳɹ������õ�ǰ����ģʽ */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PHY_COM_CFG_SET;

    /* �ȴ��첽����ʱ�䷵�� */
    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)

VOS_UINT32 At_SetLowPowerModePara(VOS_UINT8 ucIndex)
{
    AT_MTA_LOW_PWR_MODE_REQ_STRU        stLowPwrModeReq;

    /* ��ʼ�� */
    memset_s(&stLowPwrModeReq, sizeof(stLowPwrModeReq), 0x0, sizeof(AT_MTA_LOW_PWR_MODE_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("At_SetLowPowerModePara:Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �жϲ������� */
    if ((gucAtParaIndex != 1)
     || (gastAtParaList[0].usParaLen == 0))
    {
        /* ������������ */
        AT_WARN_LOG("At_SetLowPowerModePara : Para wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stLowPwrModeReq.enRat = (AT_MTA_LOW_PWR_MODE_RAT_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* ��MTA����Ϣ */
    if (AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                               gastAtClientTab[ucIndex].opId,
                                               ID_AT_MTA_LOW_PWR_MODE_SET_REQ,
                                               &stLowPwrModeReq,
                                               sizeof(stLowPwrModeReq),
                                               I0_UEPS_PID_MTA) == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LOWPWRMODE_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}


VOS_UINT32 At_SetCcpuNidDisablePara(VOS_UINT8 ucIndex)
{
    VOS_INT32                           lRet;
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("At_SetCcpuNidDisablePara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �жϲ������� */
    if ((gucAtParaIndex != 1)
     || (gastAtParaList[0].usParaLen == 0))
    {
        /* ������������ */
        AT_WARN_LOG("At_SetCcpuNidDisablePara : Para wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����DRV�ӿ� */
    lRet = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_SET_CCPUNIDEN,
                            (int)gastAtParaList[0].ulParaValue,
                            VOS_NULL_PTR,
                            0);

    /* �ж��Ƿ�ִ�гɹ� */
    if ((lRet == AT_EFUSE_OK)
     || (lRet == AT_EFUSE_REPEAT))
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }

}


VOS_UINT32 At_SetAcpuNidDisablePara(VOS_UINT8 ucIndex)
{
    VOS_INT32                           lRet;
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("At_SetAcpuNidDisablePara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �жϲ������� */
    if ((gucAtParaIndex != 1)
     || (gastAtParaList[0].usParaLen == 0))
    {
        /* ������������ */
        AT_WARN_LOG("At_SetAcpuNidDisablePara : Para wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����DRV�ӿ� */
    lRet = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_SET_ACPUNIDEN,
                            (int)gastAtParaList[0].ulParaValue,
                            VOS_NULL_PTR,
                            0);

    /* �ж��Ƿ�ִ�гɹ� */
    if ((lRet == AT_EFUSE_OK)
     || (lRet == AT_EFUSE_REPEAT))
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}
#endif


VOS_UINT32 AT_RcvMtaPhyComCfgSetCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg       = VOS_NULL_PTR;
    MTA_AT_PHY_COM_CFG_SET_CNF_STRU    *pstPhyComCfgCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /*��ʼ���ֲ�����*/
    pstRcvMsg                           = (AT_MTA_MSG_STRU *)pMsg;
    pstPhyComCfgCnf                     = (MTA_AT_PHY_COM_CFG_SET_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex                             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaPhyComCfgSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaPhyComCfgSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_PHY_COM_CFG_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PHY_COM_CFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaPhyComCfgSetCnf: NOT CURRENT CMD OPTION!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    /* ��ʽ���ϱ����� */
    if (pstPhyComCfgCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        /* ������ *AT_ERROR */
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        /* ������ *AT_OK */
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}



VOS_UINT32 AT_GetValidSamplePara(
    AT_MTA_SET_SAMPLE_REQ_STRU          *stAtCmd
)
{
    VOS_INT32                          lTemp;

    /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������4��������AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex > 4)
    {
        AT_WARN_LOG("AT_GetValidNetScanPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��1����������Ϊ0������AT_CME_INCORR ECT_PARAMETERS */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_GetValidNetScanPara: para0 Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stAtCmd->usReqType = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    if (gastAtParaList[1].usParaLen == 0)
    {
        stAtCmd->usTempRange = AT_TEMP_ZONE_DEFAULT;
    }
    else
    {
        stAtCmd->usTempRange = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    }

    if (gastAtParaList[2].usParaLen == 0)
    {
        stAtCmd->sPpmOffset = 0;
    }
    else
    {
        lTemp      = 0;

        if (AT_AtoInt((VOS_CHAR *)gastAtParaList[2].aucPara, &lTemp) == VOS_ERR)
        {
            AT_ERR_LOG("AT_GetValidNetScanPara: para2 err");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stAtCmd->sPpmOffset = (VOS_INT16)lTemp;
    }

    if (gastAtParaList[3].usParaLen == 0)
    {
        stAtCmd->sTimeOffset = 0;
    }
    else
    {
        lTemp      = 0;

        if (AT_AtoInt((VOS_CHAR *)gastAtParaList[3].aucPara, &lTemp) == VOS_ERR)
        {
            AT_ERR_LOG("AT_GetValidNetScanPara: para3 err");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stAtCmd->sTimeOffset = (VOS_INT16)lTemp;
    }

    return AT_OK;

}




VOS_UINT32 AT_SetSamplePara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_SAMPLE_REQ_STRU          stAtCmd;
    VOS_UINT32                          ulResult;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRet;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_SAMPLE_REQ_STRU));

    enModemId = MODEM_ID_0;
    ulRet     = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_SetSamplePara: Get modem id fail.");
        return AT_ERROR;
    }

    if (enModemId != MODEM_ID_0)
    {
        AT_ERR_LOG("AT_SetSamplePara: MODEM_ID ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet    = AT_GetValidSamplePara(&stAtCmd);

    if (ulRet != AT_OK)
    {
        AT_ERR_LOG("AT_SetSamplePara: AT_GetValidNetScanPara Failed");
        return ulRet;
    }

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      0,
                                      ID_AT_MTA_SET_SAMPLE_REQ,
                                      &stAtCmd,
                                      sizeof(AT_MTA_SET_SAMPLE_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_SetSamplePara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SAMPLE_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaSetSampleCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                        *pstRcvMsg               = VOS_NULL_PTR;
    MTA_AT_SET_SAMPLE_CNF_STRU             *pstSetCnf               = VOS_NULL_PTR;
    VOS_UINT32                              ulResult;
    VOS_UINT8                               ucIndex;

    /* ��ʼ�� */
    pstRcvMsg                 = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf                 = (MTA_AT_SET_SAMPLE_CNF_STRU *)pstRcvMsg->aucContent;
    ulResult                  = AT_OK;
    ucIndex                   = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetSampleCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �ж��Ƿ�Ϊ�㲥 */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetSampleCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SAMPLE_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetSampleCnf : Current Option is not AT_CMD_SAMPLE_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = 0;

    if (pstSetCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}




VOS_UINT32 AT_SetRxTestModePara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_RXTESTMODE_REQ_STRU      stRxTestModeCfg;
    VOS_UINT32                          ulRst;

    memset_s(&stRxTestModeCfg, (VOS_SIZE_T)sizeof(stRxTestModeCfg), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_SET_RXTESTMODE_REQ_STRU));

    /* ������� */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        AT_WARN_LOG("AT_SetRxTestModePara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if ( gucAtParaIndex != 1 )
    {
        AT_WARN_LOG("AT_SetRxTestModePara : The number of input parameters is error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if( gastAtParaList[0].usParaLen == 0 )
    {
        AT_WARN_LOG("AT_SetRxTestModePara : The number of input parameters is zero.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stRxTestModeCfg.enCfg  = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* ���Ϳ����Ϣ��C��, ������������ģʽ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_RX_TEST_MODE_SET_REQ,
                                   &stRxTestModeCfg,
                                   (VOS_SIZE_T)sizeof(stRxTestModeCfg),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetRxTestModePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_RXTESTMODE_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaSetRxTestModeCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_RXTESTMODE_CNF_STRU     *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ�� */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_RXTESTMODE_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetRxTestModeCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetRxTestModeCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_RXTESTMODE_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetRxTestModeCnf : Current Option is not AT_CMD_RXTESTMODE_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    if (pstSetCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}


VOS_UINT32 AT_SetMipiWrParaEx(VOS_UINT8 ucIndex)
{
    AT_MTA_MIPI_WREX_REQ_STRU           stMipiWrEx;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stMipiWrEx, (VOS_SIZE_T)sizeof(stMipiWrEx), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_MIPI_WREX_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ */
    if (gucAtParaIndex != 6)
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx : Current Number wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* extend_flag Ϊ 0 */
    if (gastAtParaList[0].ulParaValue == 0)
    {
        /* byte_cnt �����Ϊ1 ���� reg_addr ����0-31֮��,��ֵ����0XFF,ֱ�ӷ��ش��� */
        if ((gastAtParaList[4].ulParaValue != 1)
         || (gastAtParaList[3].ulParaValue > 31)
         || (gastAtParaList[5].ulParaValue > 0xFF))
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 0, byte_cnt or reg_addr or value wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stMipiWrEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
        stMipiWrEx.ulMipiId             = gastAtParaList[1].ulParaValue;
        stMipiWrEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
        stMipiWrEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
        stMipiWrEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
        stMipiWrEx.ulValue              = gastAtParaList[5].ulParaValue;
    }
    /* extend_flag Ϊ 1 */
    else if (gastAtParaList[0].ulParaValue == 1)
    {
        /* reg_addr ����0-255֮��, ֱ�ӷ��ش��� */
        if ((gastAtParaList[4].ulParaValue == 1)
         && (gastAtParaList[5].ulParaValue > 0xFF))
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 1, byte_cnt is 1, value wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ((gastAtParaList[4].ulParaValue == 2)
         && (gastAtParaList[5].ulParaValue > 0xFFFF))
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 1, byte_cnt is 2, value wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (gastAtParaList[3].ulParaValue > 255)
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 1, reg_addr wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stMipiWrEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
        stMipiWrEx.ulMipiId             = gastAtParaList[1].ulParaValue;
        stMipiWrEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
        stMipiWrEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
        stMipiWrEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
        stMipiWrEx.ulValue              = gastAtParaList[5].ulParaValue;
    }
    else
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϣ��MTA */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MIPI_WREX_REQ,
                                      &stMipiWrEx,
                                      (VOS_SIZE_T)sizeof(stMipiWrEx),
                                      I0_UEPS_PID_MTA);

    /* ����ʧ�� */
    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ���ͳɹ������õ�ǰ����ģʽ */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPI_WREX;

    /* �ȴ��첽����ʱ�䷵�� */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaMipiWrEXCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg      = VOS_NULL_PTR;
    MTA_AT_MIPI_WREX_CNF_STRU          *pstMipiWrEXCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ���ֲ����� */
    pstRcvMsg                           = (AT_MTA_MSG_STRU *)pMsg;
    pstMipiWrEXCnf                      = (MTA_AT_MIPI_WREX_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex                             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMiPiWrEXCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMiPiWrEXCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_MIPI_WREX */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MIPI_WREX)
    {
        AT_WARN_LOG("AT_RcvMtaMiPiWrEXCnf: NOT CURRENT CMD OPTION!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ���ϱ����� */
    if (pstMipiWrEXCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        /* ������ *AT_ERROR */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        /* ������ *AT_OK */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetMipiRdParaEx(VOS_UINT8 ucIndex)
{
    AT_MTA_MIPI_RDEX_REQ_STRU           stMipiRdEx;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stMipiRdEx, (VOS_SIZE_T)sizeof(stMipiRdEx), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_MIPI_RDEX_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ */
    if ( (gucAtParaIndex != 5)
      && (gucAtParaIndex != 6))
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx : Current Numbers Wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* extend_flag Ϊ 0 */
    if (gastAtParaList[0].ulParaValue == 0)
    {
        /* byte_cnt �����Ϊ1 ���� reg_addr ����0-31֮��, ֱ�ӷ��ش��� */
        if ((gastAtParaList[4].ulParaValue != 1)
         || (gastAtParaList[3].ulParaValue > 31))
        {
            AT_WARN_LOG("AT_SetMipiRdParaEx : extend_flag is 0, byte_cnt or reg_addr wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    /* extend_flag Ϊ 1 */
    else if (gastAtParaList[0].ulParaValue == 1)
    {
        /* reg_addr ����0-255֮��, ֱ�ӷ��ش��� */
        if (gastAtParaList[3].ulParaValue > 255)
        {
            AT_WARN_LOG("AT_SetMipiRdParaEx : extend_flag is 1, reg_addr wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx : extend_flag wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stMipiRdEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
    stMipiRdEx.ulMipiId             = gastAtParaList[1].ulParaValue;
    stMipiRdEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
    stMipiRdEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
    stMipiRdEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
    if (gucAtParaIndex == 6)
    {
        stMipiRdEx.ulSpeedType      = gastAtParaList[5].ulParaValue;
    }
    else
    {
        /* Ϊ�˼���֮ǰ�İ汾���ò���Ĭ�Ͽ��Բ������ȫ�ٽ���READ���� */
        stMipiRdEx.ulSpeedType      = 1;
    }

    /* ������Ϣ��MTA */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MIPI_RDEX_REQ,
                                      &stMipiRdEx,
                                      (VOS_SIZE_T)sizeof(stMipiRdEx),
                                      I0_UEPS_PID_MTA);

    /* ����ʧ�� */
    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ���ͳɹ������õ�ǰ����ģʽ */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPI_RDEX;

    /* �ȴ��첽����ʱ�䷵�� */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaMipiRdEXCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg      = VOS_NULL_PTR;
    MTA_AT_MIPI_RDEX_CNF_STRU          *pstMiPiRdEXCnf = VOS_NULL_PTR;
    VOS_UINT16                          ulLength;
    VOS_UINT8                           ucIndex;

    /* ��ʼ���ֲ����� */
    pstRcvMsg                           = (AT_MTA_MSG_STRU *)pMsg;
    pstMiPiRdEXCnf                      = (MTA_AT_MIPI_RDEX_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex                             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMiPiRdEXCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMiPiRdEXCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_MIPI_WREX */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MIPI_RDEX)
    {
        AT_WARN_LOG("AT_RcvMtaMiPiRdEXCnf: NOT CURRENT CMD OPTION!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ���ϱ����� */
    if (pstMiPiRdEXCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        /* ������ *AT_ERROR */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        /* ������ *AT_OK */
        ulLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s:%d",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstMiPiRdEXCnf->ulValue);
        gstAtSendData.usBufLen = ulLength;
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetCrrconnPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_CRRCONN_REQ_STRU         stSetCrrconn;
    VOS_UINT32                          ulRst;

    memset_s(&stSetCrrconn, sizeof(stSetCrrconn), 0x00, sizeof(AT_MTA_SET_CRRCONN_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������ֵ */
    stSetCrrconn.enEnable = (AT_MTA_CFG_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* ���Ϳ����Ϣ��C�ˣ�����CRRCONN�����ϱ����� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_CRRCONN_SET_REQ,
                                   &stSetCrrconn,
                                   sizeof(stSetCrrconn),
                                   I0_UEPS_PID_MTA);
    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetCrrconnPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CRRCONN_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryCrrconnPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* AT ��MTA ���Ͳ�ѯ������Ϣ */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_CRRCONN_QRY_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryCrrconnPara: send Msg fail.");
        return AT_ERROR;
    }

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CRRCONN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaSetCrrconnCnf(
    VOS_VOID                        *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_CRRCONN_CNF_STRU        *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_CRRCONN_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetCrrconnCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetCrrconnCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CRRCONN_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetCrrconnCnf : Current Option is not AT_CMD_CRRCONN_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    if (pstSetCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaQryCrrconnCnf(
    VOS_VOID                        *pMsg
)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                  *pstMtaMsg         = VOS_NULL_PTR;
    MTA_AT_QRY_CRRCONN_CNF_STRU      *pstQryCrrconnCnf  = VOS_NULL_PTR;
    VOS_UINT32                        ulResult;
    VOS_UINT8                         ucIndex;

    /* ��ʼ����Ϣ���� */
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstQryCrrconnCnf    = (MTA_AT_QRY_CRRCONN_CNF_STRU*)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaQryCrrconnCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryCrrconnCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CRRCONN_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CRRCONN_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaQryCrrconnCnf: WARNING:Not AT_CMD_CRRCONN_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstQryCrrconnCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult                = AT_OK;
        gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s: %d,%d,%d,%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            pstQryCrrconnCnf->enEnable,
                                            pstQryCrrconnCnf->ucStatus0,
                                            pstQryCrrconnCnf->ucStatus1,
                                            pstQryCrrconnCnf->ucStatus2);
    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    /* ����AT_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaCrrconnStatusInd(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_CRRCONN_STATUS_IND_STRU     *pstCrrconnStatusInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    ucIndex                = 0;
    pstMtaMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstCrrconnStatusInd    = (MTA_AT_CRRCONN_STATUS_IND_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaCrrconnStatusInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s^CRRCONN: %d,%d,%d%s",
                                            gaucAtCrLf,
                                            pstCrrconnStatusInd->ucStatus0,
                                            pstCrrconnStatusInd->ucStatus1,
                                            pstCrrconnStatusInd->ucStatus2,
                                            gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_SetVtrlqualrptPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_VTRLQUALRPT_REQ_STRU     stSetVtrlqualrpt;
    VOS_UINT32                          ulRst;

    memset_s(&stSetVtrlqualrpt, sizeof(stSetVtrlqualrpt), 0x00, sizeof(AT_MTA_SET_VTRLQUALRPT_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������ȷ */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������ֵ */
    stSetVtrlqualrpt.ulEnable       = gastAtParaList[0].ulParaValue;
    stSetVtrlqualrpt.ulThreshold    = gastAtParaList[1].ulParaValue;

    /* ���Ϳ����Ϣ��C�� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_VTRLQUALRPT_SET_REQ,
                                   &stSetVtrlqualrpt,
                                   (VOS_SIZE_T)sizeof(stSetVtrlqualrpt),
                                   I0_UEPS_PID_MTA);
    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetVtrlqualrptPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VTRLQUALRPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaSetVtrlqualrptCnf(
    VOS_VOID                        *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_VTRLQUALRPT_CNF_STRU    *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_VTRLQUALRPT_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetVtrlqualrptCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetVtrlqualrptCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_VTRLQUALRPT_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetVtrlqualrptCnf : Current Option is not AT_CMD_VTRLQUALRPT_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    if (pstSetCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaRlQualityInfoInd(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_RL_QUALITY_INFO_IND_STRU    *pstRlQualityInfoInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    ucIndex                = 0;
    pstMtaMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstRlQualityInfoInd    = (MTA_AT_RL_QUALITY_INFO_IND_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaRlQualityInfoInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s^LTERLQUALINFO: %d,%d,%d,%d%s",
                                            gaucAtCrLf,
                                            pstRlQualityInfoInd->sRsrp,
                                            pstRlQualityInfoInd->sRsrq,
                                            pstRlQualityInfoInd->sRssi,
                                            pstRlQualityInfoInd->usBler,
                                            gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaVideoDiagInfoRpt(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_VIDEO_DIAG_INFO_RPT_STRU    *pstVideoDiagInfoRpt = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    ucIndex                = 0;
    pstMtaMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstVideoDiagInfoRpt    = (MTA_AT_VIDEO_DIAG_INFO_RPT_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaVideoDiagInfoRpt: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s^LPDCPINFORPT: %u,%u,%u,%u%s",
                                            gaucAtCrLf,
                                            pstVideoDiagInfoRpt->ulCurrBuffTime,
                                            pstVideoDiagInfoRpt->ulCurrBuffPktNum,
                                            pstVideoDiagInfoRpt->ulMacUlThrput,
                                            pstVideoDiagInfoRpt->ulMaxBuffTime,
                                            gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}



VOS_UINT32 AT_SetEccCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_ECC_CFG_REQ_STRU         stSetEccCfgReq;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stSetEccCfgReq, sizeof(stSetEccCfgReq), 0x00, sizeof(stSetEccCfgReq));

    /* ������Ч�Լ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������ӦΪ2�������򷵻�AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��1���������Ȳ�Ϊ0�����򷵻�AT_CME_INCORR ECT_PARAMETERS */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ṹ�� */
    stSetEccCfgReq.ulEccEnable = (PS_BOOL_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    stSetEccCfgReq.ulRptPeriod = gastAtParaList[1].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_ECC_CFG_SET_REQ,
                                      (VOS_VOID*)&stSetEccCfgReq,
                                      sizeof(AT_MTA_SET_ECC_CFG_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECCCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)
TAF_UINT32 At_SetNrFreqLockPara(TAF_UINT8 ucIndex)
{
    AT_MTA_SET_NRFREQLOCK_REQ_STRU      stNrFreqLock;
    VOS_UINT32                          ulRst;

    memset_s(&stNrFreqLock, sizeof(stNrFreqLock), 0x00, sizeof(AT_MTA_SET_NRFREQLOCK_REQ_STRU));

    ulRst = AT_GetNrFreqLockPara(ucIndex, &stNrFreqLock);

    if (ulRst != AT_SUCCESS)
    {
        return ulRst;
    }

    /* ������Ϣ ID_AT_MTA_NR_FREQLOCK_SET_REQ ��C�� AT ������ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_NR_FREQLOCK_SET_REQ,
                                   &stNrFreqLock,
                                   sizeof(AT_MTA_SET_NRFREQLOCK_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NRFREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_GetNrFreqLockPara(
    VOS_UINT8                           ucClientId,
    AT_MTA_SET_NRFREQLOCK_REQ_STRU     *pstNrFreqLockInfo
)
{
    /* ������Ч�Լ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("GetNrFreqLockPara: At Cmd Opt Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    pstNrFreqLockInfo->enFreqType = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    switch (pstNrFreqLockInfo->enFreqType)
    {
        case MTA_AT_FREQLOCK_TYPE_LOCK_NONE:
            if (gastAtParaList[0].usParaLen != 1)
            {
               return AT_CME_INCORRECT_PARAMETERS;
            }

            break;
        case MTA_AT_FREQLOCK_TYPE_LOCK_FREQ:
            if (gucAtParaIndex != 4)
            {
               return AT_CME_INCORRECT_PARAMETERS;
            }

            pstNrFreqLockInfo->enFreqType  = (VOS_UINT8)gastAtParaList[0].ulParaValue;
            pstNrFreqLockInfo->enScsType   = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstNrFreqLockInfo->usBand      = (VOS_UINT16)gastAtParaList[2].ulParaValue;
            pstNrFreqLockInfo->ulNrArfcn   = (VOS_UINT32)gastAtParaList[3].ulParaValue;

            break;
        case MTA_AT_FREQLOCK_TYPE_LOCK_CELL:
            if (gucAtParaIndex != 5)
            {
               return AT_CME_INCORRECT_PARAMETERS;
            }

            pstNrFreqLockInfo->enFreqType  = (VOS_UINT8)gastAtParaList[0].ulParaValue;
            pstNrFreqLockInfo->enScsType   = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstNrFreqLockInfo->usBand      = (VOS_UINT16)gastAtParaList[2].ulParaValue;
            pstNrFreqLockInfo->ulNrArfcn   = (VOS_UINT32)gastAtParaList[3].ulParaValue;
            pstNrFreqLockInfo->usPhyCellId = (VOS_UINT16)gastAtParaList[4].ulParaValue;

            break;
        case MTA_AT_FREQLOCK_TYPE_LOCK_BAND:
            if (gucAtParaIndex != 3)
            {
               return AT_CME_INCORRECT_PARAMETERS;
            }

            pstNrFreqLockInfo->enFreqType  = (VOS_UINT8)gastAtParaList[0].ulParaValue;
            pstNrFreqLockInfo->enScsType   = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstNrFreqLockInfo->usBand      = (VOS_UINT16)gastAtParaList[2].ulParaValue;
            break;
        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}
#endif


VOS_UINT32 AT_SetLendcPara(VOS_UINT8 ucIndex)
{
    AT_MTA_UNSOLICITED_RPT_SET_REQ_STRU     stAtCmd;
    VOS_UINT32                              ulResult;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(stAtCmd));

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stAtCmd.u.ucLendcRptFlg  = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.enReqType        =  AT_MTA_SET_LENDC_RPT_TYPE;

    /* ��MTA����^LENDC�������� */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      0,
                                      ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                      &stAtCmd,
                                      sizeof(AT_MTA_UNSOLICITED_RPT_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif


VOS_UINT32 AT_ProcCclkTimeParaYMDAuxMode(
    VOS_UINT8                           ucYearLen,
    VOS_INT32                          *plYear,
    VOS_INT32                          *plMonth,
    VOS_INT32                          *plDay
)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];
    errno_t                             lMemResult;

    /* ���ո�ʽ "(yy/yyyy)/mm/dd,hh:mm:ss(+/-)zz"�����жϷ�Χ */

    /* ���(yy/yyyy) */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[0].aucPara, ucYearLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), ucYearLen);

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plYear) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of year is err");
        return VOS_ERR;
    }

    if (ucYearLen == AT_MODEM_DEFALUT_AUX_MODE_YEAR_LEN)
    {
        if ((*plYear > AT_MODEM_DEFALUT_AUX_MODE_YEAR_MAX)
         || (*plYear < AT_MODEM_DEFALUT_AUX_MODE_YEAR_MIN))
        {
            AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of year is out of range");
            return VOS_ERR;
        }
    }
    else
    {
        if ((*plYear > AT_MODEM_YEAR_MAX)
         || (*plYear < AT_MODEM_YEAR_MIN))
        {
            AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of year is out of range");
            return VOS_ERR;
        }
    }

    /* ���mm */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 1], AT_MODEM_MONTH_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_MONTH_LEN);

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plMonth) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of month is err");
        return VOS_ERR;
    }

    if ((*plMonth > AT_MODEM_MONTH_MAX)
     || (*plMonth < AT_MODEM_MONTH_MIN))
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of month is out of range");
        return VOS_ERR;
    }

    /* ���dd */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 4], AT_MODEM_DATE_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_DATE_LEN);

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plDay) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of day is err");
        return VOS_ERR;
    }

    if ((*plDay > AT_MODEM_DAY_MAX)
     || (*plDay < AT_MODEM_DAY_MIN))
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaYMDAuxMode: The parameter of day is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_ProcCclkTimeParaHMSAuxMode(
    VOS_UINT8                           ucYearLen,
    VOS_INT32                          *plHour,
    VOS_INT32                          *plMin,
    VOS_INT32                          *plSec
)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];
    errno_t                             lMemResult;

    /* ���ո�ʽ "(yy/yyyy)/mm/dd,hh:mm:ss(+/-)zz"�����жϷ�Χ */

    /* ���hh */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 7], AT_MODEM_HOUR_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_HOUR_LEN);

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plHour) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaHMSAuxMode: The parameter of hour is err");
        return VOS_ERR;
    }

    if ((*plHour > AT_MODEM_HOUR_MAX)
     || (*plHour < AT_MODEM_HOUR_MIN))
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaHMSAuxMode: The parameter of hour is out of range");
        return VOS_ERR;
    }

    /* ���mm */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 10], AT_MODEM_MIN_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_MIN_LEN);

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plMin) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaHMSAuxMode: The parameter of min is err");
        return VOS_ERR;
    }

    if ((*plMin > AT_MODEM_MIN_MAX)
     || (*plMin < AT_MODEM_MIN_MIN))
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaHMSAuxMode: The parameter of min is out of range");
        return VOS_ERR;
    }

    /* ���ss */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 13], AT_MODEM_SEC_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_SEC_LEN);

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plSec) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaHMSAuxMode: The parameter of sec is err");
        return VOS_ERR;
    }

    if ((*plSec > AT_MODEM_SEC_MAX)
     || (*plSec < AT_MODEM_SEC_MIN))
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaHMSAuxMode: The parameter of sec is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_ProcCclkTimeParaZoneAuxMode(
    VOS_UINT8                           ucYearLen,
    VOS_INT32                          *plZone
)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];
    errno_t                             lMemResult;

    /* ���ո�ʽ "(yy/yyyy)/mm/dd,hh:mm:ss(+/-)zz"�����жϷ�Χ */

    /* ���(+/-)zz */
    memset_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));

    if (gastAtParaList[0].aucPara[ucYearLen + 15] == '-')
    {
        lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 15], AT_MODEM_ZONE_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_ZONE_LEN);
    }
    else
    {
        lMemResult = memcpy_s(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[ucYearLen + 16], AT_MODEM_ZONE_LEN - 1);
        TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(aucBuffer), AT_MODEM_ZONE_LEN - 1);
    }

    if (AT_AtoInt((VOS_CHAR *)aucBuffer, plZone) == VOS_ERR)
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaZoneAuxMode: The parameter of zone is err");
        return VOS_ERR;
    }

    if ((*plZone > AT_MODEM_TIME_ZONE_MAX)
     || (*plZone < AT_MODEM_TIME_ZONE_MIN))
    {
        AT_ERR_LOG("AT_ProcCclkTimeParaZoneAuxMode: The parameter of zone is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}



VOS_UINT32 AT_ProcCclkTimeParaAuxMode(
    VOS_UINT8                       ucIndex,
    VOS_UINT8                       ucYearLen
)
{
    VOS_INT32                           lSec;
    VOS_INT32                           lMin;
    VOS_INT32                           lHour;
    VOS_INT32                           lDay;
    VOS_INT32                           lMonth;
    VOS_INT32                           lYear;
    VOS_INT32                           lZone;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    lSec    = 0;
    lMin    = 0;
    lHour   = 0;
    lDay    = 0;
    lMonth  = 0;
    lYear   = 0;
    lZone   = 0;

    /* ���ո�ʽ "(yy/yyyy)/mm/dd,hh:mm:ss(+/-)zz"�����жϷ�Χ */

    /* ���(yy/yyyy)/mm/dd */
    if (AT_ProcCclkTimeParaYMDAuxMode(ucYearLen, &lYear, &lMonth, &lDay) == VOS_ERR)
    {
        return VOS_ERR;
    }

    /* ���hh:mm:ss */
    if (AT_ProcCclkTimeParaHMSAuxMode(ucYearLen, &lHour, &lMin, &lSec) == VOS_ERR)
    {
        return VOS_ERR;
    }

    /* ���(+/-)zz */
    if (AT_ProcCclkTimeParaZoneAuxMode(ucYearLen, &lZone) == VOS_ERR)
    {
        return VOS_ERR;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    if (ucYearLen == AT_MODEM_DEFALUT_AUX_MODE_YEAR_LEN)
    {
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucYear = (VOS_UINT8)lYear;
    }
    else
    {
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucYear = (VOS_UINT8)(lYear % 100);
    }

    pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth   = (VOS_UINT8)lMonth;
    pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucDay     = (VOS_UINT8)lDay;
    pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucHour    = (VOS_UINT8)lHour;
    pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute  = (VOS_UINT8)lMin;
    pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond  = (VOS_UINT8)lSec;
    pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = (VOS_INT8)lZone;
    pstNetCtx->stTimeInfo.cLocalTimeZone                            = (VOS_INT8)lZone;

    pstNetCtx->stTimeInfo.ucIeFlg = pstNetCtx->stTimeInfo.ucIeFlg | NAS_MM_INFO_IE_UTLTZ;

    return VOS_OK;
}



VOS_UINT32 AT_SetCclkPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_NET_CTX_STRU                  *pstNetCtx = VOS_NULL_PTR;
    VOS_UINT8                               ucYearLen;

    /* ������� */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ŀ��� */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    if (pstNetCtx->stCsdfCfg.ucAuxMode == 1)
    {
        /* "yy/mm/dd,hh:mm:ss(+/-)zz" */
        ucYearLen = AT_MODEM_DEFALUT_AUX_MODE_YEAR_LEN;
    }
    else
    {
        /* "yyyy/mm/dd,hh:mm:ss(+/-)zz" */
        ucYearLen = AT_MODEM_OTHER_AUX_MODE_YEAR_LEN;
    }

    /* �������Ȳ���ȷ */
    if ((AT_MODEM_AUX_MODE_EXCEPT_YEAR_TIME_LEN + ucYearLen) != gastAtParaList[0].usParaLen)
    {
       AT_ERR_LOG("AT_SetCclkPara: length of parameter is error.");
       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ո�ʽ "(yy/yyyy)/mm/dd,hh:mm:ss(+/-)zz"�����жϸ�ʽ */
    if ( (gastAtParaList[0].aucPara[ucYearLen] != '/')
      || (gastAtParaList[0].aucPara[ucYearLen + 3] != '/')
      || (gastAtParaList[0].aucPara[ucYearLen + 6] != ',')
      || (gastAtParaList[0].aucPara[ucYearLen + 9] != ':')
      || (gastAtParaList[0].aucPara[ucYearLen + 12] != ':')
      || ( (gastAtParaList[0].aucPara[ucYearLen + 15] != '+')
        && (gastAtParaList[0].aucPara[ucYearLen + 15] != '-')))
    {
        AT_ERR_LOG("AT_SetCclkPara: The date formats parameter is error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_ProcCclkTimeParaAuxMode(ucIndex, ucYearLen) == VOS_ERR)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

#if (FEATURE_DSDS == FEATURE_ON)

VOS_UINT32 AT_SetDsdsStatePara(
    VOS_UINT8                           ucIndex
)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetDsdsStatePara: Get modem id fail.");
        return AT_ERROR;
    }

    if (enModemId != MODEM_ID_0)
    {
        return AT_ERROR;
    }

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_MMA_SetDsdsStateReq(WUEPS_PID_AT,
                            gastAtClientTab[ucIndex].usClientId,
                            gastAtParaList[0].ulParaValue) == VOS_TRUE)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DSDS_STATE_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMmaDsdsStateSetCnf(
    VOS_VOID                           *pstMsg
)
{
    TAF_MMA_DSDS_STATE_SET_CNF_STRU    *pstDsdsStateSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex = 0;
    VOS_UINT32                          ulResult;

    pstDsdsStateSetCnf = (TAF_MMA_DSDS_STATE_SET_CNF_STRU *)pstMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstDsdsStateSetCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaDsdsStateSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaDsdsStateSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DSDS_STATE_SET)
    {
        AT_WARN_LOG("AT_RcvMmaDsdsStateSetCnf : Current Option is not AT_CMD_CIND_SET.");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstDsdsStateSetCnf->ulRslt == VOS_TRUE)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaDsdsStateNotify(
    VOS_VOID                           *pstMsg
)
{
    TAF_MMA_DSDS_STATE_NOTIFY_STRU     *pstDsdsStateNotify = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* ��ʼ����Ϣ���� */
    ucIndex             = 0;
    usLength            = 0;
    pstDsdsStateNotify  = (TAF_MMA_DSDS_STATE_NOTIFY_STRU *)pstMsg;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstDsdsStateNotify->usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMmaDsdsStateNotify: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvMmaDsdsStateNotify: Get modem id fail.");
        return VOS_ERR;
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_DSDSSTATE].pucText,
                                       pstDsdsStateNotify->ucSupportDsds3,
                                       gaucAtCrLf);

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}
#endif


VOS_UINT32 AT_SetGameModePara(
    VOS_UINT8                           ucIndex
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_MTA_COMM_GAME_MODE_SET_REQ_STRU  stAtCmd;

    enModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetGameModePara: Get modem id fail.");
        return AT_ERROR;
    }

    if (enModemId != MODEM_ID_0)
    {
        AT_ERR_LOG("enModemId isn't MODEM 0");
        return AT_ERROR;
    }

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(stAtCmd));
    stAtCmd.ucGameMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    if (AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                           0,
                           ID_AT_MTA_GAME_MODE_SET_REQ,
                           &stAtCmd,
                           sizeof(AT_MTA_COMM_GAME_MODE_SET_REQ_STRU),
                           I0_UEPS_PID_MTA) == TAF_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_GAME_MODE_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
}



VOS_UINT32 AT_SetSmsDomainPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SMS_DOMAIN_SET_REQ_STRU  stSmsDomain;
    VOS_UINT32                      ulRst;

    memset_s(&stSmsDomain, sizeof(stSmsDomain), 0x00, sizeof(AT_MTA_SMS_DOMAIN_SET_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������������ȼ�� */
    if ( (gucAtParaIndex != 1)
      || (gastAtParaList[0].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSmsDomain.enSmsDomain = (AT_MTA_SMS_DOMAIN_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* ���Ϳ����Ϣ��C�� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SMS_DOMAIN_SET_REQ,
                                   &stSmsDomain,
                                   sizeof(stSmsDomain),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetSmsDomainPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SMSDOMAIN_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaSmsDomainSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_SMS_DOMAIN_SET_CNF_STRU                 *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_SMS_DOMAIN_SET_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSmsDomainSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSmsDomainSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SMSDOMAIN_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSmsDomainSetCnf : Current Option is not AT_CMD_SMSDOMAIN_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enResult != VOS_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_QrySmsDomainPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* AT ��MTA ���Ͳ�ѯ������Ϣ */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_SMS_DOMAIN_QRY_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_SetSmsDomainPara: AT_QrySmsDomainPara fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SMSDOMAIN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaSmsDomainQryCnf(
    VOS_VOID                        *pMsg
)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                  *pstMtaMsg         = VOS_NULL_PTR;
    MTA_AT_SMS_DOMAIN_QRY_CNF_STRU   *pstQryCnf         = VOS_NULL_PTR;
    VOS_UINT32                        ulResult;
    VOS_UINT8                         ucIndex;

    /* ��ʼ����Ϣ���� */
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstQryCnf           = (MTA_AT_SMS_DOMAIN_QRY_CNF_STRU*)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSmsDomainQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSmsDomainQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_SMSDOMAIN_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SMSDOMAIN_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaSmsDomainQryCnf: WARNING:Not AT_CMD_SMSDOMAIN_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult                = AT_OK;
        gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s: %d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            pstQryCnf->enSmsDomain);
    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    /* ����AT_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_ConvertWs46RatOrderPara(
    VOS_UINT8                           ucClientId,
    TAF_MMA_SYS_CFG_PARA_STRU          *pstSysCfgSetPara
)
{
    TAF_MMA_MULTIMODE_RAT_CFG_STRU     *pstRatOrder = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRst;

    enModemId = MODEM_ID_0;
    pstRatOrder = &(pstSysCfgSetPara->stMultiModeRatCfg);

    ulRst = AT_GetModemIdFromClient(ucClientId, &enModemId);
    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG1("AT_ConvertWs46RatOrderPara:Get ModemID From ClientID fail,ClientID:", ucClientId);
        return AT_ERROR;
    }

    AT_SetDefaultRatPrioList(enModemId, pstRatOrder, VOS_TRUE, VOS_TRUE);

    pstSysCfgSetPara->enUserPrio = AT_GetSysCfgPrioRat(pstSysCfgSetPara);

    return AT_OK;
}



VOS_UINT32 AT_SetWs46Para(VOS_UINT8 ucIndex)
{
    TAF_MMA_SYS_CFG_PARA_STRU           stSysCfgSetPara;
    VOS_UINT32                          ulRst;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    memset_s(&stSysCfgSetPara, sizeof(stSysCfgSetPara), 0x00, sizeof(TAF_MMA_SYS_CFG_PARA_STRU));

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    ulRst = AT_OK;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = AT_ConvertWs46RatOrderPara(ucIndex,
                                       &stSysCfgSetPara);

    if (ulRst != AT_OK)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* stSysCfgSetPara����������ֵ */
    if (pstNetCtx->ucRoamFeature == AT_ROAM_FEATURE_ON)
    {
        stSysCfgSetPara.enRoam = TAF_MMA_ROAM_UNCHANGE;
    }
    else
    {
        stSysCfgSetPara.enRoam = AT_ROAM_FEATURE_OFF_NOCHANGE;
    }

    stSysCfgSetPara.enSrvDomain         = TAF_MMA_SERVICE_DOMAIN_NOCHANGE;
    stSysCfgSetPara.stGuBand.ulBandLow  = TAF_PH_BAND_NO_CHANGE;
    stSysCfgSetPara.stGuBand.ulBandHigh = 0;

    memset_s(&(stSysCfgSetPara.stLBand),
             sizeof(stSysCfgSetPara.stLBand),
             0x00,
             sizeof(TAF_USER_SET_LTE_PREF_BAND_INFO_STRU));
    stSysCfgSetPara.stLBand.aulBandInfo[0] = TAF_PH_BAND_NO_CHANGE;

    /* ִ��������� */
    if (TAF_MMA_SetSysCfgReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stSysCfgSetPara) == VOS_TRUE)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_WS46_SET;

        /* ������������״̬ */
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryWs46Para(VOS_UINT8 ucIndex)
{
    if (TAF_MMA_QrySyscfgReq(WUEPS_PID_AT,
                             gastAtClientTab[ucIndex].usClientId,
                             0) == VOS_TRUE)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_WS46_QRY;             /*���õ�ǰ����ģʽ */
        return AT_WAIT_ASYNC_RETURN;                                            /* �ȴ��첽�¼����� */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestWs46Para( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "(12,22,25,28,29,30,31)"
                             );

    return AT_OK;
}



VOS_UINT32 AT_SetGpsLocSetPara(VOS_UINT8 ucIndex)
{
    AT_MTA_GPSLOCSET_SET_REQ_STRU       stAtCmd;
    VOS_UINT32                          ulResult;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_GPSLOCSET_SET_REQ_STRU));

    /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������������1��������AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetGpsLocSetPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��1����������Ϊ0������AT_CME_INCORRECT_PARAMETERS */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_SetGpsLocSetPara: para0 Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stAtCmd.ulLocationPermitFlag = gastAtParaList[0].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      0,
                                      ID_AT_MTA_GPS_LOCSET_SET_REQ,
                                      &stAtCmd,
                                      sizeof(AT_MTA_GPSLOCSET_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_SetGpsLocSetPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_GPSLOCSET_SET;

    return AT_WAIT_ASYNC_RETURN;
}



VOS_UINT32 AT_RcvMtaGpsLocSetCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                        *pstRcvMsg               = VOS_NULL_PTR;
    MTA_AT_GPSLOCSET_SET_CNF_STRU          *pstSetCnf               = VOS_NULL_PTR;
    VOS_UINT32                              ulResult;
    VOS_UINT8                               ucIndex;

    /* ��ʼ�� */
    pstRcvMsg                 = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf                 = (MTA_AT_GPSLOCSET_SET_CNF_STRU *)pstRcvMsg->aucContent;
    ulResult                  = AT_OK;
    ucIndex                   = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaGpsLocSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �ж��Ƿ�Ϊ�㲥 */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaGpsLocSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_GPSLOCSET_SET)
    {
        AT_WARN_LOG("AT_RcvMtaGpsLocSetCnf : Current Option is not AT_CMD_GPSLOCSET_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = 0;

    ulResult = AT_ConvertMtaResult(pstSetCnf->enResult);

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}





VOS_UINT32 AT_SetVzwMruC(
    VOS_UINT8                           ucIndex
)
{
    AT_CSS_VZWMRUC_SET_REQ_STRU        *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;
    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_SetVzwMruC: Get modem id fail.");
        return AT_ERROR;
    }

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        AT_ERR_LOG("AT_SetVzwMruC:Cmd Opt Type is wrong.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϣ��AT_CSS_VZWMRUC_SET_REQ_STRU */
    /* Allocating memory for message */
    /*lint -save -e516 */
    pstMsg = (AT_CSS_VZWMRUC_SET_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_CSS_VZWMRUC_SET_REQ_STRU));
    /*lint -restore */

    /* �ڴ�����ʧ�ܣ����� */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SetVzwMruC:memory alloc fail.");

        return AT_ERROR;
    }

    TAF_MEM_SET_S(((VOS_UINT8 *)pstMsg + VOS_MSG_HEAD_LENGTH),
                  (VOS_SIZE_T)(sizeof(AT_CSS_VZWMRUC_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                  0x00,
                  (VOS_SIZE_T)(sizeof(AT_CSS_VZWMRUC_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��д��Ϣͷ */
    AT_CFG_MSG_HDR(pstMsg, PS_PID_CSS, ID_AT_CSS_VZWMRUC_SET_REQ);

    /* ��д��Ϣ���� */
    pstMsg->usClientId = gastAtClientTab[ucIndex].usClientId;
    pstMsg->usModemId  = enModemId;

    /* ������Ϣ��������������״̬ */
    AT_SEND_MSG(pstMsg);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VZWMRUC_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_CheckVzwMruEPara(VOS_VOID)
{
    /* ������������ȷ,����ֻ��4������ */
    if(gucAtParaIndex != 4)
    {
       AT_ERR_LOG("AT_CheckVzwMruEPara:number of parameter error.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������Ȳ��� */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0)
     || (gastAtParaList[2].usParaLen == 0)
     || (gastAtParaList[3].usParaLen == 0))
    {
        AT_ERR_LOG("AT_CheckVzwMruEPara:para len error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��һ������entry����Ϊ0 */
    if (gastAtParaList[0].ulParaValue == 0)
    {
       AT_ERR_LOG("AT_CheckVzwMruEPara:entry can not be 0.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �ڶ�������ֻ��ΪGSM/UMTS/LTE ��һֵ*/
    if ((VOS_StrCmp((TAF_CHAR*)gastAtParaList[1].aucPara,"GSM") != 0)
     && (VOS_StrCmp((TAF_CHAR*)gastAtParaList[1].aucPara,"UMTS") != 0)
     && (VOS_StrCmp((TAF_CHAR*)gastAtParaList[1].aucPara,"LTE") != 0))
    {
       AT_ERR_LOG("AT_CheckVzwMruEPara:rat can only be one of GSM/UMTS/LTE");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_ConvertDecFormatPlmnId2HexFormat(
    CSS_AT_PLMN_ID_STRU                *pstPlmnId,
    VOS_UINT32                          ulPlmnId
)
{
    if (pstPlmnId == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SetVzwMruE:plmn id should not be null.");
        return VOS_FALSE;
    }

    /* Ĭ��ulPlmnIdֵ��Ӧ�ô���999999��*/
    if (ulPlmnId >= AT_CONST_MILLION)
    {
        AT_ERR_LOG("AT_SetVzwMruE:plmn id must be less than 999999.");
        return VOS_FALSE;
    }

    /*  Ĭ��ulPlmnIdֵ����99999ʱ��MNCΪ��λ������MNCΪ��λ��
        ��: ulPlmnId����123456ʱ������ulMcc����0x030201,ulMnc����0x060504;
            ulPlmnId����12345ʱ������ulMcc����0x030201,ulMnc����0x0f0504;  */
    if (ulPlmnId / AT_CONST_HUNDRED_THOUSAND == 0)
    {
        pstPlmnId->ulMcc = (VOS_UINT32)(((ulPlmnId % AT_CONST_HUNDRED_THOUSAND) / AT_CONST_TEN_THOUSAND)
                            | (((ulPlmnId % AT_CONST_TEN_THOUSAND) / AT_CONST_THOUSAND) << 8)
                            | (((ulPlmnId % AT_CONST_THOUSAND) / AT_CONST_HUNDRED) << 16));
        pstPlmnId->ulMnc = (VOS_UINT32)((0x0f << 16)
                            | ((ulPlmnId % AT_CONST_HUNDRED) / AT_CONST_TEN)
                            | (((ulPlmnId % AT_CONST_TEN) / AT_CONST_ONE) << 8));
    }
    else
    {
        pstPlmnId->ulMcc = (VOS_UINT32)(((ulPlmnId % AT_CONST_MILLION) / AT_CONST_HUNDRED_THOUSAND)
                            | (((ulPlmnId % AT_CONST_HUNDRED_THOUSAND) / AT_CONST_TEN_THOUSAND) << 8)
                            | (((ulPlmnId % AT_CONST_TEN_THOUSAND) / AT_CONST_THOUSAND) << 16));
        pstPlmnId->ulMnc = (VOS_UINT32)(((ulPlmnId % AT_CONST_THOUSAND) / AT_CONST_HUNDRED)
                            | (((ulPlmnId % AT_CONST_HUNDRED) / AT_CONST_TEN) << 8)
                            | (((ulPlmnId % AT_CONST_TEN) / AT_CONST_ONE) << 16));
    }

    return VOS_TRUE;
}



VOS_UINT32 AT_SetVzwMruE(
    VOS_UINT8                           ucIndex
)
{
    AT_CSS_VZWMRUE_SET_REQ_STRU        *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_CSS_RAT_ENUM_UINT8               enRat;

    enModemId = MODEM_ID_0;
    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_SetVzwMruE: Get modem id fail.");
        return AT_ERROR;
    }

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_ERR_LOG("AT_SetVzwMruE:Cmd Opt Type is wrong.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    ulResult = AT_CheckVzwMruEPara();
    if (ulResult != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_SetVzwMruE:Check VzwMrue para error.");

        return ulResult;
    }

    /* rat��client���ַ�������ת��ΪCSS�����ʽ */
    if (VOS_StrCmp((TAF_CHAR*)gastAtParaList[1].aucPara,"GSM") == 0)
    {
       enRat = AT_CSS_RAT_TYPE_GSM;
    }
    else if (VOS_StrCmp((TAF_CHAR*)gastAtParaList[1].aucPara,"UMTS") == 0)
    {
        enRat = AT_CSS_RAT_TYPE_WCDMA;
    }
    else if (VOS_StrCmp((TAF_CHAR*)gastAtParaList[1].aucPara,"LTE") == 0)
    {
        enRat = AT_CSS_RAT_TYPE_LTE;
    }
    else
    {
        /* ��ֹ�Ժ������麯�������仯 */
        AT_ERR_LOG("AT_SetVzwMruE:Check VzwMruE para error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϣ��AT_CSS_VZWMRUC_SET_REQ_STRU */
    /* Allocating memory for message */
    /*lint -save -e516 */
    pstMsg = (AT_CSS_VZWMRUE_SET_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_CSS_VZWMRUE_SET_REQ_STRU));
    /*lint -restore */

    /* �ڴ�����ʧ�ܣ����� */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SetVzwMruE:memory alloc fail.");

        return AT_ERROR;
    }

    TAF_MEM_SET_S((VOS_INT8 *)pstMsg + VOS_MSG_HEAD_LENGTH,
                  sizeof(AT_CSS_VZWMRUE_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH,
                  0x00,
                  sizeof(AT_CSS_VZWMRUE_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    /* ��д��Ϣͷ */
    AT_CFG_MSG_HDR(pstMsg, PS_PID_CSS, ID_AT_CSS_VZWMRUE_SET_REQ);

    /* ��д��Ϣ���� */
    pstMsg->usClientId           = gastAtClientTab[ucIndex].usClientId;
    pstMsg->usModemId            = enModemId;
    pstMsg->stMru.ucEntry        = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    pstMsg->stMru.enRat          = enRat;
    pstMsg->stMru.usBandId       = (VOS_UINT16)gastAtParaList[2].ulParaValue;

    /* Client��PLMN-Id��ʽת��ΪCSS�����ʽ */
    (VOS_VOID)AT_ConvertDecFormatPlmnId2HexFormat(&(pstMsg->stMru.stPlmnId), gastAtParaList[3].ulParaValue);

    /* ������Ϣ��������������״̬ */
    AT_SEND_MSG(pstMsg);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VZWMRUE_SET;

    return AT_WAIT_ASYNC_RETURN;
}



VOS_UINT32 AT_QryVzwMruE(
    VOS_UINT8                           ucIndex
)
{
    AT_CSS_VZWMRUE_QUERY_REQ_STRU      *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;
    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRet != VOS_OK)
    {
        AT_ERR_LOG("AT_QryVzwMruE: Get modem id fail.");
        return AT_ERROR;
    }

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϣ��AT_CSS_VZWMRUC_SET_REQ_STRU */
    /* Allocating memory for message */
    /*lint -save -e516 */
    pstMsg = (AT_CSS_VZWMRUE_QUERY_REQ_STRU*)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_CSS_VZWMRUE_QUERY_REQ_STRU));
    /*lint -restore */

    /* �ڴ�����ʧ�ܣ����� */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_QryVzwMruE:memory alloc fail.");

        return AT_ERROR;
    }

    TAF_MEM_SET_S(((VOS_UINT8 *)pstMsg + VOS_MSG_HEAD_LENGTH),
                  (VOS_SIZE_T)(sizeof(AT_CSS_VZWMRUE_QUERY_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                  0x00,
                  (VOS_SIZE_T)(sizeof(AT_CSS_VZWMRUE_QUERY_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��д��Ϣͷ */
    AT_CFG_MSG_HDR(pstMsg, PS_PID_CSS, ID_AT_CSS_VZWMRUE_QUERY_REQ);

    /* ��д��Ϣ���� */
    pstMsg->usClientId               = gastAtClientTab[ucIndex].usClientId;
    pstMsg->usModemId                = enModemId;

    /* ������Ϣ��������������״̬ */
    AT_SEND_MSG(pstMsg);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VZWMRUE_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestVzwMruE(
    VOS_UINT8                           ucIndex
)
{
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_TEST_CMD)
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: (1-10),(\"GSM\",\"UTMS\",\"LTE\"),(0-65535),(0-999999)",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_VOID AT_PrintCclkTime(
    VOS_UINT8                           ucIndex,
    TIME_ZONE_TIME_STRU                *pstTimeZoneTime,
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    TIME_ZONE_TIME_STRU                *pstTimeZone = VOS_NULL_PTR;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulYear;

    pstTimeZone = pstTimeZoneTime;

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    if (pstNetCtx->stCsdfCfg.ucAuxMode == 1)
    {
        /* "yy/mm/dd,hh:mm:ss(+/-)zz" */
        ulYear = pstTimeZone->ucYear;
    }
    else
    {
        /* "yyyy/mm/dd,hh:mm:ss(+/-)zz" */
        if (pstTimeZone->ucYear > AT_GMT_TIME_DEFAULT)
        {
            ulYear = 1900 + pstTimeZone->ucYear;
        }
        else
        {
            ulYear = 2000 + pstTimeZone->ucYear;
        }
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        "%s: \"%02d/%02d/%02d,%02d:%02d:%02d",
                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                        ulYear,
                                        pstTimeZone->ucMonth,
                                        pstTimeZone->ucDay,
                                        pstTimeZone->ucHour,
                                        pstTimeZone->ucMinute,
                                        pstTimeZone->ucSecond);

    if (pstTimeZone->cTimeZone >= 0)
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,
            (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
            "+%02d\"",
            pstTimeZone->cTimeZone);

    }
    else
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,
            (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
            "-%02d\"",
            -(pstTimeZone->cTimeZone));

    }

    return;
}


VOS_UINT32 AT_RcvMtaCclkQryCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                        *pstRcvMsg               = VOS_NULL_PTR;
    MTA_AT_CCLK_QRY_CNF_STRU               *pstQryCnf               = VOS_NULL_PTR;
    AT_MODEM_NET_CTX_STRU                  *pstNetCtx               = VOS_NULL_PTR;
    VOS_UINT32                              ulResult;
    VOS_UINT8                               ucIndex;
    MODEM_ID_ENUM_UINT16                    enModemId;
    TIME_ZONE_TIME_STRU                     stTime;;

    /* ��ʼ�� */
    memset_s(&stTime, sizeof(stTime), 0x00, sizeof(stTime));
    pstRcvMsg                 = (AT_MTA_MSG_STRU *)pMsg;
    pstQryCnf                 = (MTA_AT_CCLK_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    ulResult                  = AT_OK;
    ucIndex                   = 0;

    ulResult = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulResult != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvMtaCclkQryCnf: Get modem id fail.");
        return VOS_ERR;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaCclkQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �ж��Ƿ�Ϊ�㲥 */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaCclkQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCLK_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaCclkQryCnf : Current Option is not AT_CMD_CCLK_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = 0;

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult                = AT_OK;
        stTime                  = pstQryCnf->stTime;

        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucYear    = pstQryCnf->stTime.ucYear;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth   = pstQryCnf->stTime.ucMonth;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucDay     = pstQryCnf->stTime.ucDay;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucHour    = pstQryCnf->stTime.ucHour;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute  = pstQryCnf->stTime.ucMinute;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond  = pstQryCnf->stTime.ucSecond;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = pstQryCnf->stTime.cTimeZone;
        pstNetCtx->stTimeInfo.ucIeFlg                                  |= NAS_MM_INFO_IE_UTLTZ;

        AT_PrintCclkTime(ucIndex, &stTime, enModemId);
    }
    else
    {
        ulResult                = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}




VOS_UINT32 AT_RcvMtaTempProtectInd(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                  *pstMtaMsg         = VOS_NULL_PTR;
    MTA_AT_TEMP_PROTECT_IND_STRU     *pstTempInd        = VOS_NULL_PTR;
    VOS_UINT8                         ucIndex;

    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstTempInd          = (MTA_AT_TEMP_PROTECT_IND_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaTempProtectInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   "%s%s%d%s",
                                                   gaucAtCrLf,
                                                   gastAtStringTab[AT_STRING_TEMPPROTECT].pucText,
                                                   pstTempInd->lTempResult,
                                                   gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_SetNvRefreshPara( VOS_UINT8 ucIndex )
{
    AT_MTA_NV_REFRESH_SET_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulResult;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Phone�濨NV */
    AT_ReadIpv6AddrTestModeCfgNV();
    AT_ReadPrivacyFilterCfgNv();
    AT_InitStk();
    AT_ReadWasCapabilityNV();
    AT_ReadAgpsNv();

    /* ����MBB�濨NV */
    AT_ReadSmsNV();
    AT_ReadRoamCapaNV();
    AT_ReadIpv6CapabilityNV();
    AT_ReadGasCapabilityNV();
    AT_ReadApnCustomFormatCfgNV();

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_NV_REFRESH_SET_REQ_STRU));

    stAtCmd.enReason               = AT_MTA_NV_REFRESH_USIM_DEPENDENT;

    /* ��MTA����NV_REFRESH�������� */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      0,
                                      ID_AT_MTA_NV_REFRESH_SET_REQ,
                                      &stAtCmd,
                                      sizeof(AT_MTA_NV_REFRESH_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);


    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NVREFRESH_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_ERR_LOG("AT_SetNvRefreshPara:  AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMtaNvRefreshSetCnf(
    VOS_VOID                           *pMsg
)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                    *pstMtaMsg = VOS_NULL_PTR;
    MTA_AT_NV_REFRESH_SET_CNF_STRU     *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstSetCnf           = (MTA_AT_NV_REFRESH_SET_CNF_STRU*)pstMtaMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNvRefreshSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNvRefreshSetCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_NVREFRESH_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NVREFRESH_SET)
    {
        AT_WARN_LOG("AT_RcvMtaNvRefreshSetCnf: WARNING:Not AT_CMD_NVREFRESH_SET!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    /* �ж����ò����Ƿ�ɹ� */
    if (pstSetCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult                = AT_OK;
    }
    else
    {
        ulResult                = AT_ERROR;
    }

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}




VOS_UINT32 AT_SetPseudBtsPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_PSEUDBTS_REQ_STRU        stAtCmd;
    VOS_UINT32                          ulRet;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_PSEUDBTS_REQ_STRU));

     /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������������2��������AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("AT_SetPseudBtsPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��1����������Ϊ0������AT_CME_INCORR ECT_PARAMETERS */
    if ( (gastAtParaList[0].usParaLen == 0)
      || (gastAtParaList[1].usParaLen == 0))
    {
        AT_WARN_LOG("AT_SetPseudBtsPara: para Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stAtCmd.ucPseudRat         = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ucPseudBtsQryType  = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    ulRet = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                  0,
                                  ID_AT_MTA_PSEUDBTS_SET_REQ,
                                  &stAtCmd,
                                  sizeof(AT_MTA_SET_PSEUDBTS_REQ_STRU),
                                  I0_UEPS_PID_MTA);

    if (ulRet != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_SetPseudBtsPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PSEUDBTS_SET;

    return AT_WAIT_ASYNC_RETURN;

}


VOS_UINT32 AT_CheckSubClfsParamPara(VOS_VOID)
{
    /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������������9����С��6��������AT_CME_INCORRECT_PARAMETERS */
    if ((gucAtParaIndex < 6) || (gucAtParaIndex > 9))
    {
        AT_WARN_LOG("AT_CheckSubClfsParamPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[1].usParaLen > 10)
    {
        AT_WARN_LOG("AT_CheckSubClfsParamPara: At Para1 Length Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϊ���ܹر������������Ϊ6�� */
    if ( (gastAtParaList[3].ulParaValue == 0) && (gucAtParaIndex > 6))
    {
        AT_WARN_LOG("AT_CheckSubClfsParamPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_SetSubClfsParamPara(VOS_UINT8 ucIndex)
{

    AT_MTA_SET_SUBCLFSPARAM_REQ_STRU    *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                           ulBufLen;
    VOS_UINT32                           ulBsLen;
    VOS_UINT32                           ulLoop;
    VOS_UINT32                           ulDeltaLen;
    VOS_UINT32                           ulRet;
    errno_t                             lMemResult;

    /* ������� */
    ulRet = AT_CheckSubClfsParamPara();
    if (ulRet != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_SetSubClfsParamPara: AT_CheckSubClfsParamPara Failed");
        return ulRet;
    }

    ulBsLen  = gastAtParaList[6].usParaLen + gastAtParaList[7].usParaLen + gastAtParaList[8].usParaLen;
    ulBufLen = sizeof(AT_MTA_SET_SUBCLFSPARAM_REQ_STRU);

    if (ulBsLen > 4)
    {
        ulBufLen += ulBsLen - 4;
    }

    /*lint -save -e830*/
    pstMsg = (AT_MTA_SET_SUBCLFSPARAM_REQ_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, ulBufLen);
    /*lint -restore */

    /* �ڴ�����ʧ�ܣ�����AT_ERROR */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SetSubClfsParamPara:memory alloc fail.");
        return AT_ERROR;
    }

    memset_s((VOS_INT8 *)pstMsg, ulBufLen, 0x00, ulBufLen);

    /* ��д��Ϣ���� */
    pstMsg->ucSeq                       = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    pstMsg->ucType                      = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    pstMsg->ucActiveFlg                 = (VOS_UINT8)gastAtParaList[3].ulParaValue;
    pstMsg->ulProbaRate                 = gastAtParaList[4].ulParaValue;
    pstMsg->stParaInfo.usClfsGroupNum   = (VOS_UINT16)gastAtParaList[5].ulParaValue;
    pstMsg->stParaInfo.usDataLength     = (VOS_UINT16)ulBsLen;

    lMemResult = memcpy_s(pstMsg->aucVersionId,
                          sizeof(pstMsg->aucVersionId),
                          gastAtParaList[1].aucPara,
                          gastAtParaList[1].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstMsg->aucVersionId), gastAtParaList[1].usParaLen);

    ulDeltaLen = 0;
    for (ulLoop = 6; ulLoop < 9; ulLoop++)
    {
        if (gastAtParaList[ulLoop].usParaLen > 0)
        {
            lMemResult = memcpy_s(pstMsg->stParaInfo.aucClfsData+ ulDeltaLen,
                                  (ulBsLen - ulDeltaLen),
                                  gastAtParaList[ulLoop].aucPara,
                                  gastAtParaList[ulLoop].usParaLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (ulBsLen - ulDeltaLen), gastAtParaList[ulLoop].usParaLen);
            ulDeltaLen =  ulDeltaLen + gastAtParaList[ulLoop].usParaLen;
        }
        else
        {
            break;
        }
    }

    ulRet = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                  gastAtClientTab[ucIndex].opId,
                                  ID_AT_MTA_SUBCLFSPARAM_SET_REQ,
                                  pstMsg,
                                  ulBufLen,
                                  I0_UEPS_PID_MTA);

    /*lint -save -e516*/
    PS_MEM_FREE(WUEPS_PID_AT, pstMsg);
    /*lint -restore */

    if (ulRet != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_SetPseudBtsPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SUBCLFSPARAM_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QrySubClfsParamPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* ���Ϳ����Ϣ��C��, ��ѯ��ͨ�Ű汾���� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SUBCLFSPARAM_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QrySubClfsParamPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SUBCLFSPARAM_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_SetClDbDomainStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucMode;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������������1������AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetClDbDomainStatusPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��д��Ϣ���� */
    ucMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    if (TAF_MMA_SetClDbDomainStatus(WUEPS_PID_AT,
                                                gastAtClientTab[ucIndex].usClientId,
                                                0,
                                                ucMode) != VOS_TRUE)
    {
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CLDBDOMAINSTATUS_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif


VOS_UINT32 AT_SetUlFreqRptPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucMode;

    /* ָ�����ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetUlFreqPara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������򳤶Ȳ���ȷ */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetUlFreqPara : The number of input parameters is error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }


    /* ��д��Ϣ */
    ucMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;

      /* ������Ϣ��C�˴��� */
    if (TAF_MMA_SetUlFreqRptReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId,
                                            ucMode) == VOS_TRUE)
    {

        /* ������������״̬ */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ULFREQRPT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryUlFreqPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRslt;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* ��MMA������Ϣ��ѯ����С������Ƶ�� */
    ulRslt = TAF_MMA_QryUlFreqRptReq(WUEPS_PID_AT,
                                     gastAtClientTab[ucIndex].usClientId,
                                     gastAtClientTab[ucIndex].opId);

    if (ulRslt == VOS_TRUE)
    {
        /* ������������״̬ */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ULFREQRPT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMmaUlFreqRptSetCnf(VOS_VOID *pMsg)
{
    TAF_MMA_ULFREQRPT_SET_CNF_STRU      *pstUlfreqSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ�� */
    ucIndex      = 0;
    ulResult     = AT_ERROR;
    pstUlfreqSetCnf = (TAF_MMA_ULFREQRPT_SET_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstUlfreqSetCnf->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqRptSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqRptSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ULFREQRPT_SET)
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqRptSetCnf : Current Option is not AT_CMD_EFPSLOCIINFO_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    if (pstUlfreqSetCnf->enResult == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaUlFreqChangeInd(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex;
    TAF_MMA_UL_FREQ_IND_STRU           *pstUlFreqInfoInd = VOS_NULL_PTR;

    /* ��ʼ����Ϣ���� */
    ucIndex            = 0;
    pstUlFreqInfoInd   = (TAF_MMA_UL_FREQ_IND_STRU *)pMsg;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstUlFreqInfoInd->usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqChangeInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s: %d,%d,%d%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_ULFREQRPT].pucText,
                                                    pstUlFreqInfoInd->enRat,
                                                    pstUlFreqInfoInd->ulFreq,
                                                    pstUlFreqInfoInd->usBandWidth,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaUlFreqRptQryCnf(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_ULFREQRPT_QRY_CNF_STRU     *pstUlFreqInfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    ucIndex            = 0;
    pstUlFreqInfoInd   = (TAF_MMA_ULFREQRPT_QRY_CNF_STRU *)pMsg;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstUlFreqInfoInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqRptQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqRptQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ULFREQRPT_QRY)
    {
        AT_WARN_LOG("AT_RcvMmaUlFreqRptQryCnf : Current Option is not AT_CMD_EFPSLOCIINFO_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstUlFreqInfoInd->enErrorCause == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                "^ULFREQRPT: %d,%d,%d,%d",
                                                pstUlFreqInfoInd->enMode,
                                                pstUlFreqInfoInd->enRat,
                                                pstUlFreqInfoInd->ulUlFreq,
                                                pstUlFreqInfoInd->usBandwidth);
    }
    else
    {
        ulResult = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaPseudBtsIdentInd(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                     *pstMtaMsg                  = VOS_NULL_PTR;
    MTA_AT_PSEUD_BTS_IDENT_IND_STRU     *pstPseudBtsIdentInd        = VOS_NULL_PTR;
    VOS_UINT8                            ucIndex;

    pstMtaMsg                    = (AT_MTA_MSG_STRU*)pMsg;
    pstPseudBtsIdentInd          = (MTA_AT_PSEUD_BTS_IDENT_IND_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMtaPseudBtsIdentInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   "%s%s: %d%s",
                                                   gaucAtCrLf,
                                                   gastAtStringTab[AT_STRING_PSEUDBTS].pucText,
                                                   pstPseudBtsIdentInd->ulPseudBtsType,
                                                   gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 At_SetPsScenePara(TAF_UINT8 ucIndex)
{
#if  ( FEATURE_MULTI_MODEM == FEATURE_ON )
    TAF_MMA_PS_SCENE_PARA_STRU          stPsSceneStru;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_BUTT;

    /* ͨ��clientID���ModemID */
    ulRslt    = AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId);

    /* ���ModemID��ȡʧ�ܻ���Modem1������ʧ�� */
    if ((ulRslt != VOS_OK)
     || (enModemId != MODEM_ID_1))
    {
        AT_WARN_LOG1("At_SetPsScenePara: AT_GetModemIdFromClient failed or not modem1!modem id is,",enModemId);
        return AT_ERROR;
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (At_CheckCurrRatModeIsCL((VOS_UINT8)(gastAtClientTab[ucIndex].usClientId)) == VOS_TRUE)
    {
        AT_WARN_LOG("At_SetPsScenePara: operation not allowed in CL mode!");
        return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
    }
#endif

    /* ������� */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* PSҵ��ʼʱ��ps_detach��Ǳ���Ϊfalse */
    if ((gastAtParaList[0].ulParaValue == 1)
     && (gastAtParaList[1].ulParaValue != 0))
    {
        AT_WARN_LOG2("At_SetPsScenePara: paravalue is wrong,para0,para1,",gastAtParaList[0].ulParaValue, gastAtParaList[1].ulParaValue);
        return AT_ERROR;
    }

    memset_s(&stPsSceneStru, sizeof(stPsSceneStru), 0x00, sizeof(TAF_MMA_PS_SCENE_PARA_STRU));
    stPsSceneStru.ucPsServiceState  = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stPsSceneStru.ucPsDetachFlag    = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    if (TAF_MMA_SetPsSceneReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stPsSceneStru) == VOS_TRUE)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PSSCENE_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
#else
    return AT_ERROR;
#endif

}


VOS_UINT32 AT_QryPsScenePara(TAF_UINT8 ucIndex)
{
#if  ( FEATURE_MULTI_MODEM == FEATURE_ON )
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_BUTT;

    /* ͨ��clientID���ModemID */
    ulRslt    = AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId);

    /* ���ModemID��ȡʧ�ܻ���Modem1������ʧ�� */
    if ((ulRslt != VOS_OK)
     || (enModemId != MODEM_ID_1))
    {
        AT_WARN_LOG("AT_QryPsScenePara: AT_GetModemIdFromClient failed or not modem1!");
        return AT_ERROR;
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (At_CheckCurrRatModeIsCL((VOS_UINT8)(gastAtClientTab[ucIndex].usClientId)) == VOS_TRUE)
    {
        AT_WARN_LOG("AT_QryPsScenePara: operation not allowed in CL mode!");
        return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
    }
#endif

    if (TAF_MMA_QryPsSceneReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0) == VOS_TRUE)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PSSCENE_QRY;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
#else
    return AT_ERROR;
#endif
}


VOS_UINT32 AT_RcvMmaPsSceneSetCnf(VOS_VOID *pstMsg)
{
    TAF_MMA_PS_SCENE_SET_CNF_STRU      *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    ucIndex                             = 0;
    pstSetCnf                     = (TAF_MMA_PS_SCENE_SET_CNF_STRU*)pstMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstSetCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaPsSceneSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaPsSceneSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PSSCENE_SET)
    {
        AT_WARN_LOG("AT_RcvMmaPsSceneSetCnf : Current Option is not AT_CMD_PSSCENE_SET.");
        return VOS_ERR;
    }
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstSetCnf->enResult == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = At_ChgTafErrorCode(ucIndex, pstSetCnf->enResult);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaPsSceneQryCnf(VOS_VOID *pstMsg)
{
    TAF_MMA_PS_SCENE_QRY_CNF_STRU      *pstPsSceneQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ����Ϣ���� */
    ucIndex                             = 0;
    ulResult                            = 0;
    pstPsSceneQryCnf                    = (TAF_MMA_PS_SCENE_QRY_CNF_STRU *)pstMsg;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstPsSceneQryCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMmaPsSceneQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaPsSceneQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PSSCENE_QRY)
    {
        AT_WARN_LOG("AT_RcvMmaPsSceneQryCnf : Current Option is not AT_CMD_CEMODE_READ.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstPsSceneQryCnf->enResult == TAF_ERR_NO_ERROR)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    pstPsSceneQryCnf->ulPsSceneState);
        ulResult               = AT_OK;
    }
    else
    {
        ulResult               = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_SetForceSyncPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_FORCESYNC_REQ_STRU       stAtCmd;
    VOS_UINT32                          ulRet;

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_FORCESYNC_REQ_STRU));

    /* ���������ڷ�����ģʽ��ʹ�� */
#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
#else
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
#endif
    {
        return AT_ERROR;
    }

     /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������������1��������AT_CME_INCORRECT_PARAMETERS */
    if ((gucAtParaIndex != 1)
        || (gastAtParaList[0].usParaLen == 0))
    {
        AT_WARN_LOG("AT_SetForceSyncPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stAtCmd.usNum = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    ulRet = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                  0,
                                  ID_AT_MTA_FORCESYNC_SET_REQ,
                                  &stAtCmd,
                                  sizeof(AT_MTA_SET_FORCESYNC_REQ_STRU),
                                  I0_UEPS_PID_MTA);

    if (ulRet != AT_SUCCESS)
    {
        AT_WARN_LOG("AT_SetForceSyncPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FORCESYNC_SET;

    return AT_WAIT_ASYNC_RETURN;

}


VOS_UINT32 AT_RcvMtaForceSyncSetCnf (VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pRcvMsg             = VOS_NULL_PTR;
    MTA_AT_SET_FORCESYNC_CNF_STRU      *pstSetForceSyncCnf  = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    ucIndex                 = 0;
    ulResult                = 0;
    pRcvMsg                 = (AT_MTA_MSG_STRU *)pMsg;
    pstSetForceSyncCnf      = (MTA_AT_SET_FORCESYNC_CNF_STRU *)(pRcvMsg->aucContent);

     /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaForceSyncSetCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �ж��Ƿ�Ϊ�㲥 */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaForceSyncSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FORCESYNC_SET)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstSetForceSyncCnf->ulResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }
    gstAtSendData.usBufLen = 0;

    /* ���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_UINT32 At_SetGnssNtyPara(VOS_UINT8 ucIndex)
{
    AT_MTA_GNSSNTY_STRU                 stAtMtaGnssnty;
    VOS_UINT32                          ulRst;

    /* ������� */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if(gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������ȼ�� */
    if(gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*Hi110XоƬ��GNSS�����¸�MODEM��AT^GNSSNTY=1,�˳�GNSSNTY������AT^GNSSNTY=0*/
    stAtMtaGnssnty.ulValue = gastAtParaList[0].ulParaValue;


    /* ������Ϣ ID_AT_MTA_GNSS_NTY ��MTA��������Ϣ������ stAtMtaGnssnty */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_GNSS_NTY,
                                   (VOS_VOID *)&stAtMtaGnssnty,
                                   (VOS_SIZE_T)sizeof(stAtMtaGnssnty),
                                   I0_UEPS_PID_MTA);

    if (ulRst != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetGnssNtyPara: send ReqMsg fail");
        return AT_ERROR;
    }

    return AT_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

LOCAL VOS_UINT32 AT_CheckNrrcCapCfgPara(VOS_VOID)
{
    /* ������Ч�Լ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��1������Ϊ0��1�����򷵻�AT_CME_INCORR ECT_PARAMETERS */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara: First Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������ӦΪ9��11�������򷵻�AT_CME_INCORRECT_PARAMETERS */
    if ((gastAtParaList[0].ulParaValue == 0)
     && (gucAtParaIndex != 9))
    {
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara: First Para Is 0, At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((gastAtParaList[0].ulParaValue == 1)
     && (gucAtParaIndex != 11))
    {
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara: First Para Is 1, At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_SetNrrcCapCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_NRRCCAP_CFG_SET_REQ_STRU     stSetNrrcCapCfgReq;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          i;

    /* �ֲ�������ʼ�� */
    memset_s(&stSetNrrcCapCfgReq, sizeof(stSetNrrcCapCfgReq), 0x00, sizeof(AT_MTA_NRRCCAP_CFG_SET_REQ_STRU));

    /* ���������ͺϷ��Լ��,���Ϸ�ֱ�ӷ���ʧ�� */
    ulResult = AT_CheckNrrcCapCfgPara();
    if (ulResult != AT_SUCCESS)
    {
        return ulResult;
    }

    /* ���ṹ�� */
    stSetNrrcCapCfgReq.ulNrrcCfgNetMode = gastAtParaList[0].ulParaValue;

    for (i = 0; i < AT_MTA_NRRCCAP_PARA_MAX_NUM; i++)
    {
        stSetNrrcCapCfgReq.aulPara[i] = (gastAtParaList + i + 1)->ulParaValue;
    }

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_NRRCCAP_CFG_SET_REQ,
                                      (VOS_VOID*)&stSetNrrcCapCfgReq,
                                      sizeof(AT_MTA_NRRCCAP_CFG_SET_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NRRCCAPCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}


VOS_UINT32 AT_SetNrrcCapQryPara(VOS_UINT8 ucIndex)
{
    AT_MTA_NRRCCAP_QRY_REQ_STRU         stSetNrrcCapQryReq;
    VOS_UINT32                          ulResult;

    /* �ֲ�������ʼ�� */
    memset_s(&stSetNrrcCapQryReq, sizeof(stSetNrrcCapQryReq), 0x00, sizeof(AT_MTA_NRRCCAP_QRY_REQ_STRU));

    /* ���������ͺϷ��Լ��,���Ϸ�ֱ�ӷ���ʧ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetNrrcCapQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������ӦΪ1�������򷵻�AT_CME_INCORRECT_PARAMETERS */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetNrrcCapQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��һ���������Ȳ���Ϊ0�����򷵻�AT_CME_INCORR ECT_PARAMETERS */
    if (gastAtParaList[0].usParaLen == 0)
    {
        AT_WARN_LOG("AT_SetNrrcCapQryPara: First Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ṹ�� */
    stSetNrrcCapQryReq.ulNrrcCfgNetMode = gastAtParaList[0].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_NRRCCAP_QRY_REQ,
                                      (VOS_VOID *)&stSetNrrcCapQryReq,
                                      sizeof(AT_MTA_NRRCCAP_QRY_REQ_STRU),
                                      I0_UEPS_PID_MTA);

    if (ulResult == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NRRCCAPQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


LOCAL VOS_UINT32 AT_NrPwrCtrlParaCheck(AT_MTA_NRPWRCTRL_SET_REQ_STRU  *pstAtCmd)
{
    VOS_INT32                           lVal;
    VOS_UINT32                          ulRst;

    lVal = 0;
    if (AT_AtoInt((VOS_CHAR *)gastAtParaList[1].aucPara, &lVal) == VOS_ERR)
    {
        AT_WARN_LOG("AT_NrPwrCtrlParaCheck: string -> num error.");
        return VOS_ERR;
    }

    ulRst = VOS_ERR;
    switch ((AT_NRPWRCTRL_MODE_TYPE_ENUM_UINT32)gastAtParaList[0].ulParaValue)
    {
        /* 0��2��case��Χ�ȱ����ƣ�����㿪����ʱ�����޸� */
        case AT_NRPWRCTRL_MODE_RI_NUM_CTR:
            if ((lVal == AT_NRPWRCTRL_DISABLE_REDUCE_RI)
             || (lVal == AT_NRPWRCTRL_ENABLE_REDUCE_RI))
            {
                ulRst = VOS_OK;
            }
            break;

        default:
            AT_WARN_LOG("AT_NrPwrCtrlParaCheck: unexpected mode type.");
            break;
    }

    if (ulRst == VOS_ERR)
    {
        return VOS_ERR;
    }

    pstAtCmd->ucMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    pstAtCmd->sPara  = (VOS_INT16)lVal;

    return VOS_OK;
}



VOS_UINT32 AT_SetNrPwrCtrlPara(VOS_UINT8 ucIndex)
{
    AT_MTA_NRPWRCTRL_SET_REQ_STRU       stAtCmd;
    VOS_UINT32                          ulRst;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetNrPwrCtrlPara: ucCmdOptType is not SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("AT_SetNrPwrCtrlPara: Para num is not correct.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����1�Ͳ���2�ĳ��Ȳ���Ϊ0*/
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        AT_WARN_LOG("AT_SetNrPwrCtrlPara: Length of para is 0.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����2���������ΧΪ(-8~63),���Ȳ��ܴ���2*/
    if (gastAtParaList[1].usParaLen > 2)
    {
        AT_WARN_LOG("AT_SetNrPwrCtrlPara: Length of para 2 is greater than 2.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stAtCmd, sizeof(AT_MTA_NRPWRCTRL_SET_REQ_STRU), 0, sizeof(AT_MTA_NRPWRCTRL_SET_REQ_STRU));

    ulRst = AT_NrPwrCtrlParaCheck(&stAtCmd);

    if (ulRst == VOS_ERR)
    {
        AT_WARN_LOG("AT_SetNrPwrCtrlPara: Check para fail.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*�·���������������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_NRPWRCTRL_SET_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_NRPWRCTRL_SET_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NRPWRCTRL_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_TestNrPwrCtrlPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR*)pgucAtSndCodeAddr,
                                                    (TAF_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: (0-2),(0-1)",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    return AT_OK;
}
#endif


VOS_UINT32 AT_RcvMtaTxPowerQryCnf(
    VOS_VOID                        *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg        = VOS_NULL_PTR;
    MTA_AT_TXPOWER_QRY_CNF_STRU        *pstTxpwrQryCnf   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    pstRcvMsg        = (AT_MTA_MSG_STRU *)pstMsg;
    pstTxpwrQryCnf   = (MTA_AT_TXPOWER_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    usLength         = 0;
    ucIndex          = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaTxPowerQryCnf: WARNING: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �㲥��Ϣ������ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaTxPowerQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TXPOWER_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaTxPowerQryCnf: WARNING: CmdCurrentOpt != AT_CMD_TXPOWER_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ConvertMtaResult(pstTxpwrQryCnf->enResult);

    if (pstTxpwrQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: %d,%d,%d,%d,%d",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstTxpwrQryCnf->stTxpwrInfo.sGuTxPwr,
                                          pstTxpwrQryCnf->stTxpwrInfo.sPuschPwr,
                                          pstTxpwrQryCnf->stTxpwrInfo.sPucchPwr,
                                          pstTxpwrQryCnf->stTxpwrInfo.sSrsPwr,
                                          pstTxpwrQryCnf->stTxpwrInfo.sPrachPwr);
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_RcvMtaNtxPowerQryCnf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg        = VOS_NULL_PTR;
    MTA_AT_NTXPOWER_QRY_CNF_STRU       *pstTxpwrQryCnf   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    pstRcvMsg        = (AT_MTA_MSG_STRU *)pstMsg;
    pstTxpwrQryCnf   = (MTA_AT_NTXPOWER_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    usLength         = 0;
    ucIndex          = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNtxPowerQryCnf: WARNING: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �㲥��Ϣ������ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNtxPowerQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NTXPOWER_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaNtxPowerQryCnf: WARNING: CmdCurrentOpt != AT_CMD_NTXPOWER_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ConvertMtaResult(pstTxpwrQryCnf->enResult);

    if (pstTxpwrQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: ",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        for (ulLoop = 0; ulLoop < pstTxpwrQryCnf->usNrCellNum; ulLoop++)
        {
            if (pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_SUL_ONLY)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  "%d,%d,%d,%d,%d,",
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sSulPuschPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sSulPucchPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sSulSrsPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sSulPrachPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].ulSulFreq);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  "%d,%d,%d,%d,%d,",
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sUlPuschPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sUlPucchPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sUlSrsPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].sUlPrachPwr,
                                                  pstTxpwrQryCnf->astNrTxPwrInfo[ulLoop].ulUlFreq);
            }
        }
        /* ɾ��ѭ������,�ַ� */
        gstAtSendData.usBufLen = usLength - 1;
    }
    else
    {
        gstAtSendData.usBufLen = usLength;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvMtaMcsSetCnf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MCS_QRY_CNF_STRU            *pstMtaAtQryMcsCnf = VOS_NULL_PTR;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx         = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    pstRcvMsg         = (AT_MTA_MSG_STRU *)pstMsg;
    pstMtaAtQryMcsCnf = (MTA_AT_MCS_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    ucIndex  = 0;
    usLength = 0;

    /* ͨ�� Clientid ��ȡ index */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaMcsSetCnf: WARNING: AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMcsSetCnf: WARNING:AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

     /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MCS_SET)
    {
        return VOS_ERR;
    }

    pstNetCtx         = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ConvertMtaResult(pstMtaAtQryMcsCnf->enResult);

    if (pstMtaAtQryMcsCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        if (pstNetCtx->ucMcsDirection == AT_MCS_DIRECTION_UL)
        {
            AT_ProcUlMcsRsp(pstMtaAtQryMcsCnf, ucIndex, &usLength);
        }
        else
        {
            AT_ProcDlMcsRsp(pstMtaAtQryMcsCnf, ucIndex, &usLength);
        }
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaHfreqinfoQryCnf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg           = VOS_NULL_PTR;
    MTA_AT_HFREQINFO_QRY_CNF_STRU      *pstMtaAtQryHfreqCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32                          ulLoop;
#endif
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstMtaAtQryHfreqCnf = (MTA_AT_HFREQINFO_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    ucIndex  = 0;
    usLength = 0;

    /* ͨ�� Clientid ��ȡ index */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaHfreqinfoQryCnf: WARNING: AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaHfreqinfoQryCnf: WARNING:AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

     /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_HFREQINFO_QRY)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ConvertMtaResult(pstMtaAtQryHfreqCnf->enResult);

    if (pstMtaAtQryHfreqCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        if (pstMtaAtQryHfreqCnf->enResultType != MTA_AT_RESULT_TYPE_NR)
        {
            /* LTE */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s: %d,%d,%d,%d,%d,%d,%d,%d,%d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              pstMtaAtQryHfreqCnf->ucIsreportFlg,
                                              HFREQ_INFO_RAT_LTE,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.usBand,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.ulDlEarfcn,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.ulDlFreq,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.ulDlBandWidth,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.ulUlEarfcn,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.ulUlFreq,
                                              pstMtaAtQryHfreqCnf->stLteHfreqInfo.ulUlBandWidth);
        }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (pstMtaAtQryHfreqCnf->enResultType == MTA_AT_RESULT_TYPE_DC)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s",
                                              gaucAtCrLf);
        }

        if (pstMtaAtQryHfreqCnf->enResultType != MTA_AT_RESULT_TYPE_LTE)
        {
            /* NR�����ݲ�֧�ֲ��� */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s: %d,%d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              pstMtaAtQryHfreqCnf->ucIsreportFlg,
                                              HFREQ_INFO_RAT_NR);

            for (ulLoop = 0; ulLoop < (VOS_UINT32)AT_MIN(pstMtaAtQryHfreqCnf->usNrCellNum, MTA_AT_MAX_CC_NUMBER); ulLoop++)
            {
                if (pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_SUL_ONLY)
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d",
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].usSulBand);
                }
                else
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d",
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].usBand);
                }

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  ",%d,%d,%d",
                                                  pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulDlNarfcn,
                                                  pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulDlFreq,
                                                  pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulDlBandWidth);

                if (pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_SUL_ONLY)
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d,%d,%d",
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulSulNarfcn,
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulSulFreq,
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulSulBandWidth);
                }
                else
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d,%d,%d",
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulUlNarfcn,
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulUlFreq,
                                                      pstMtaAtQryHfreqCnf->astNrHfreqInfo[ulLoop].ulUlBandWidth);

                }
            }
        }
#endif

    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaHfreqinfoInd(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg           = VOS_NULL_PTR;
    MTA_AT_HFREQINFO_IND_STRU          *pstMtaAtHfreqInd    = VOS_NULL_PTR;
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32                          ulLoop;
#endif
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstMtaAtHfreqInd    = (MTA_AT_HFREQINFO_IND_STRU *)pstRcvMsg->aucContent;

    ucIndex  = 0;
    usLength = 0;

    /* ͨ�� Clientid ��ȡ index */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaHfreqinfoInd: WARNING: AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (pstMtaAtHfreqInd->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        if (pstMtaAtHfreqInd->enResultType == MTA_AT_RESULT_TYPE_LTE)
        {
            /* LTE */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s^HFREQINFO: %d,%d,%d,%d,%d,%d,%d,%d%s",
                                              gaucAtCrLf,
                                              HFREQ_INFO_RAT_LTE,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.usBand,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.ulDlEarfcn,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.ulDlFreq,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.ulDlBandWidth,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.ulUlEarfcn,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.ulUlFreq,
                                              pstMtaAtHfreqInd->stLteHfreqInfo.ulUlBandWidth,
                                              gaucAtCrLf);
        }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (pstMtaAtHfreqInd->enResultType == MTA_AT_RESULT_TYPE_NR)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s^HFREQINFO: %d",
                                              gaucAtCrLf,
                                              HFREQ_INFO_RAT_NR);
            for (ulLoop = 0; ulLoop < (VOS_UINT32)AT_MIN(pstMtaAtHfreqInd->usNrCellNum, MTA_AT_MAX_CC_NUMBER); ulLoop++)
            {
                if (pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_SUL_ONLY)
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d",
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].usSulBand);
                }
                else
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d",
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].usBand);
                }
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  ",%d,%d,%d",
                                                  pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulDlNarfcn,
                                                  pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulDlFreq,
                                                  pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulDlBandWidth);

                if (pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_SUL_ONLY)
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d,%d,%d",
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulSulNarfcn,
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulSulFreq,
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulSulBandWidth);
                }
                else
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d,%d,%d",
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulUlNarfcn,
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulUlFreq,
                                                      pstMtaAtHfreqInd->astNrHfreqInfo[ulLoop].ulUlBandWidth);

                }
            }
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s",
                                              gaucAtCrLf);
        }
#endif

    }

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaRrcStatQryCnf(
    VOS_VOID                           *pstMsg
)
{
    TAF_MMA_RRCSTAT_QRY_CNF_STRU       *pstRrcStatQryCnf = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    pstRrcStatQryCnf = (TAF_MMA_RRCSTAT_QRY_CNF_STRU *)pstMsg;
    usLength         = 0;
    ucIndex          = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRrcStatQryCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaRrcStatQryCnf: WARNING: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �㲥��Ϣ������ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaRrcStatQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_RRCSTAT_QRY)
    {
        AT_WARN_LOG("AT_RcvMmaRrcStatQryCnf: WARNING: CmdCurrentOpt != AT_CMD_RRCSTAT_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstRrcStatQryCnf->ucReportFlg,
                                      pstRrcStatQryCnf->enRrcStat);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaRrcStatInd(
    VOS_VOID                           *pstMsg
)
{
    VOS_UINT8                           ucIndex;
    TAF_MMA_RRCSTAT_IND_STRU           *pstRrcStat = VOS_NULL_PTR;

    /* ��ʼ����Ϣ���� */
    ucIndex        = 0;
    pstRrcStat     = (TAF_MMA_RRCSTAT_IND_STRU *)pstMsg;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstRrcStat->stCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvMmaRrcStatInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   "%s^RRCSTAT: %d%s",
                                                   gaucAtCrLf,
                                                   pstRrcStat->enRrcStat,
                                                   gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


LOCAL VOS_VOID AT_ProcUlMcsRsp(
    MTA_AT_MCS_QRY_CNF_STRU            *pstMtaAtQryMcsCnf,
    VOS_UINT8                           ucIndex,
    VOS_UINT16                         *pusLength
)
{
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32                          ulLoop;
#endif

    if (pstMtaAtQryMcsCnf->enResultType != MTA_AT_RESULT_TYPE_NR)
    {
        /* LTE */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%s: %d,%d,%d,%d,%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            AT_MCS_DIRECTION_UL,
                                            MCS_RAT_LTE,
                                            MTA_AT_INDEX_TABLE_ONE,
                                            pstMtaAtQryMcsCnf->stLteMcsInfo.usUlMcs[0],
                                            pstMtaAtQryMcsCnf->stLteMcsInfo.usUlMcs[1]);
    }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (pstMtaAtQryMcsCnf->enResultType == MTA_AT_RESULT_TYPE_DC)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%s",
                                            gaucAtCrLf);
    }

    if (pstMtaAtQryMcsCnf->enResultType != MTA_AT_RESULT_TYPE_LTE)
    {
        /* NR */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%s: %d,%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            AT_MCS_DIRECTION_UL,
                                            MCS_RAT_NR);
        for (ulLoop = 0; ulLoop < (VOS_UINT32)AT_MIN(pstMtaAtQryMcsCnf->usNrCellNum, MTA_AT_MAX_CC_NUMBER); ulLoop ++)
        {
            /* NR�����ݲ�֧�ֲ��� */
            if (pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_UL_ONLY)
            {
                *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                    ",%d,%d,%d",
                                                    pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].enUlMcsTable,
                                                    pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].usUlMcs[0],
                                                    pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].usUlMcs[1]);
            }
            else if (pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].enUlMode == MTA_AT_UL_MODE_SUL_ONLY)
            {
                *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                    ",%d,%d,%d",
                                                    pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].enSulMcsTable,
                                                    pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].usSulMcs[0],
                                                    pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].usSulMcs[1]);
            }
            else
            {
                *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                    ",%d,%d,%d",
                                                    MTA_AT_INDEX_TABLE_BUTT,
                                                    MTA_AT_INVALID_MCS_VALUE,
                                                    MTA_AT_INVALID_MCS_VALUE);
            }
        }
    }
#endif
}


LOCAL VOS_VOID AT_ProcDlMcsRsp(
    MTA_AT_MCS_QRY_CNF_STRU            *pstMtaAtQryMcsCnf,
    VOS_UINT8                           ucIndex,
    VOS_UINT16                         *pusLength
)
{
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32                          ulLoop;
#endif

    if (pstMtaAtQryMcsCnf->enResultType != MTA_AT_RESULT_TYPE_NR)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%s: %d,%d,%d,%d,%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            AT_MCS_DIRECTION_DL,
                                            MCS_RAT_LTE,
                                            MTA_AT_INDEX_TABLE_ONE,
                                            pstMtaAtQryMcsCnf->stLteMcsInfo.usDlMcs[0],
                                            pstMtaAtQryMcsCnf->stLteMcsInfo.usDlMcs[1]);
    }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (pstMtaAtQryMcsCnf->enResultType == MTA_AT_RESULT_TYPE_DC)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%s",
                                            gaucAtCrLf);
    }

    if (pstMtaAtQryMcsCnf->enResultType != MTA_AT_RESULT_TYPE_LTE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%s: %d,%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            AT_MCS_DIRECTION_DL,
                                            MCS_RAT_NR);
        for (ulLoop = 0; ulLoop < (VOS_UINT32)AT_MIN(pstMtaAtQryMcsCnf->usNrCellNum, MTA_AT_MAX_CC_NUMBER); ulLoop ++)
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                ",%d,%d,%d",
                                                pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].enDlMcsTable,
                                                pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].usDlMcs[0],
                                                pstMtaAtQryMcsCnf->astNrMcsInfo[ulLoop].usDlMcs[1]);
        }
    }
#endif
}

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_UINT32 AT_RcvMta5gOptionSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_SET_5G_OPTION_CNF_STRU                  *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_SET_5G_OPTION_CNF_STRU *)pRcvMsg->aucContent;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMta5gOptionSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMta5gOptionSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_5G_OPTION_SET)
    {
        AT_WARN_LOG("AT_RcvMta5gOptionSetCnf : Current Option is not AT_CMD_5G_OPTION_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enRslt != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMta5gOptionQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_QRY_5G_OPTION_CNF_STRU                  *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_QRY_5G_OPTION_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMta5gOptionQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMta5gOptionQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_5G_OPTION_QRY)
    {
        AT_WARN_LOG("AT_RcvMta5gOptionQryCnf : Current Option is not AT_CMD_5G_OPTION_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstMtaCnf->enRslt != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d,%d,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstMtaCnf->ucNrSaSupportFlag,
                                                        pstMtaCnf->enNrDcMode,
                                                        pstMtaCnf->en5gcAccessMode);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif



