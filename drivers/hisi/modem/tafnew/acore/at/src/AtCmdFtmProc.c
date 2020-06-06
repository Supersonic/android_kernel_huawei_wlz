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
#include "mdrv.h"
#include "AtCmdFtmProc.h"
#include "ATCmdProc.h"
#include "AtMtaInterface.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "msp_diag.h"
#endif

#include "AtTestParaCmd.h"
#include "TafAppMma.h"

#include "AtDataProc.h"
#include "securec.h"


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_FTM_PROC_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

#define AT_LTEPWRCTRL_MAX_DELETE_CA_NUM                     (-8)
#define AT_LTEPWRCTRL_MAX_ADD_CA_NUM                        8

#define AT_LTEPWRCTRL_DISABLE_REDUCE_RI                     0
#define AT_LTEPWRCTRL_ENABLE_REDUCE_RI                      1

#define AT_LTEPWRCTRL_MIN_SUPPORT_BSR_NUM                   0
#define AT_LTEPWRCTRL_MAX_SUPPORT_BSR_NUM                   63

enum AT_LTEPWRCTRL_MODE_TYPE_ENUM
{
    AT_LTEPWRCTRL_MODE_CC_NUM_CTR      = 0,
    AT_LTEPWRCTRL_MODE_RI_NUM_CTR      = 1,
    AT_LTEPWRCTRL_MODE_BSR_NUM_CTR     = 2,
    AT_LTEPWRCTRL_MODE_BUTT            = 3,
};
typedef  VOS_UINT32  AT_LTEPWRCTRL_MODE_TYPE_ENUM_UINT32;

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

VOS_UINT32 At_SetLogPortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulOmLogPort;    // ׼���л���LogPort

    AT_PR_LOGI("Rcv Msg");


    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_ERROR;
    }

    /* ���������û�� */
    if ((gucAtParaIndex > 2) || (gucAtParaIndex == 0))
    {
        return AT_ERROR;
    }

    if (gastAtParaList[0].ulParaValue == AT_LOG_PORT_USB)
    {
        ulOmLogPort = CPM_OM_PORT_TYPE_USB;
    }
    else
    {
        ulOmLogPort = CPM_OM_PORT_TYPE_VCOM;
    }

    /* ����ֻ��һ����Ĭ��������Ч */
    if (gucAtParaIndex == 1)
    {
        gastAtParaList[1].ulParaValue = VOS_TRUE;
    }

    /* ����OM�Ľӿ� */
    ulRslt = DIAG_LogPortSwich(ulOmLogPort, gastAtParaList[1].ulParaValue);

    AT_PR_LOGI("Call interface success!");

    if (ulRslt == VOS_OK)
    {
        return AT_OK;
    }
    else if(ulRslt == ERR_MSP_AT_CHANNEL_BUSY)
    {
        return AT_CME_USB_TO_VCOM_IN_CONN_ERROR;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_QryLogPortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulOmLogPort;
    VOS_UINT32                          ulAtLogPort;
    VOS_UINT32                          ulRslt;

    usLength                            = 0;
    ulOmLogPort                         = AT_LOG_PORT_USB;

    AT_PR_LOGI("Rcv Msg");

    ulRslt = mdrv_PPM_QueryLogPort(&ulOmLogPort);

    AT_PR_LOGI("Call interface success!");

    if (ulRslt != VOS_OK)
    {
        return AT_ERROR;
    }

    if (ulOmLogPort == COMM_LOG_PORT_USB)
    {
        ulAtLogPort = AT_LOG_PORT_USB;
    }
    else
    {
        ulAtLogPort = AT_LOG_PORT_VCOM;
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
                                       ulAtLogPort);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}

VOS_UINT32 At_QryLogCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulAtLogCfg;

    usLength                            = 0;
    PS_PRINTF_INFO("at^logcfg?!\n");

    (void)mdrv_socp_get_cfg_ind_mode(&ulAtLogCfg);

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulAtLogCfg);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}
/*****************************************************************************
 �� �� ��  : At_QryLogCpsPara
 ��������  : ^LOGCPS�Ĳ�ѯ����
 �������  : VOS_UINT8 ucIndex
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :


*****************************************************************************/
VOS_UINT32 At_QryLogCpsPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulAtLogCps;
    usLength                            = 0;

    PS_PRINTF_INFO("at^logcps?!\n");

    (void)mdrv_socp_get_cps_ind_mode(&ulAtLogCps);

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulAtLogCps);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}

VOS_UINT32 At_SetDpdtTestFlagPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU   stAtCmd;
    VOS_UINT32                          ulRst;

    /* ������� */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT������MTA����Ϣ�ṹ��ֵ */
    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU));
    stAtCmd.enRatMode = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ucFlag    = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* ������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_DPDTTEST_FLAG_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_DPDTTEST_FLAG_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == AT_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDTTEST_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMtaSetDpdtTestFlagCnf(VOS_VOID *pMsg)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                    *pstMtaMsg         = VOS_NULL_PTR;
    MTA_AT_SET_DPDTTEST_FLAG_CNF_STRU  *pstSetDpdtFlagCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ����Ϣ���� */
    ucIndex   = 0;
    pstMtaMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstSetDpdtFlagCnf = (MTA_AT_SET_DPDTTEST_FLAG_CNF_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtTestFlagCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtTestFlagCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_DPDTTEST_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DPDTTEST_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtTestFlagCnf: WARNING:Not AT_CMD_DPDTTEST_SET!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstSetDpdtFlagCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult    = AT_OK;
    }
    else
    {
        ulResult    = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaSetDpdtValueCnf(VOS_VOID *pMsg)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                    *pstMtaMsg          = VOS_NULL_PTR;
    MTA_AT_SET_DPDT_VALUE_CNF_STRU     *pstSetDpdtValueCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ����Ϣ���� */
    ucIndex            = 0;
    pstMtaMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstSetDpdtValueCnf = (MTA_AT_SET_DPDT_VALUE_CNF_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtValueCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtValueCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_DPDT_SET��AT_CMD_TFDPDT_SET */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DPDT_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TFDPDT_SET))
    {
        AT_WARN_LOG("AT_RcvMtaSetDpdtValueCnf: WARNING:Not AT_CMD_DPDT_SET or AT_CMD_TFDPDT_SET!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstSetDpdtValueCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult    = AT_OK;
    }
    else
    {
        ulResult    = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaQryDpdtValueCnf(VOS_VOID *pMsg)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                    *pstMtaMsg          = VOS_NULL_PTR;
    MTA_AT_QRY_DPDT_VALUE_CNF_STRU     *pstQryDpdtValueCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ����Ϣ���� */
    ucIndex            = 0;
    pstMtaMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstQryDpdtValueCnf = (MTA_AT_QRY_DPDT_VALUE_CNF_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaQryDpdtValueCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryDpdtValueCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_DPDTQRY_SET��AT_CMD_TFDPDTQRY_SET */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DPDTQRY_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TFDPDTQRY_SET))
    {
        AT_WARN_LOG("AT_RcvMtaQryDpdtValueCnf: WARNING:Not AT_CMD_DPDTQRY_SET or AT_CMD_TFDPDTQRY_SET !");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstQryDpdtValueCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            "%s: %d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            pstQryDpdtValueCnf->ulDpdtValue);
    }
    else
    {
        ulResult = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvMtaSetRatFreqLockCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_FREQ_LOCK_CNF_STRU      *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ�� */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_FREQ_LOCK_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetRatFreqLockCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetRatFreqLockCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_RATFREQLOCK_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetRatFreqLockCnf : Current Option is not AT_CMD_RATFREQLOCK_SET.");
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

#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)

VOS_UINT32 AT_SetRatFreqLock(VOS_UINT8 ucIndex)
{
    TAF_NVIM_FREQ_LOCK_CFG_STRU         stAtCmd;
    VOS_UINT32                          ulRst;

    /* ����������� */
    if ((gucAtParaIndex < 1) || (gucAtParaIndex > 4))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(TAF_NVIM_FREQ_LOCK_CFG_STRU));
    stAtCmd.ucEnableFlg = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* �����Ƶ���ܹرգ�ֱ��дNv, ����AT_OK */
    if (stAtCmd.ucEnableFlg == VOS_FALSE)
    {
        if (TAF_ACORE_NV_WRITE(MODEM_ID_0, en_NV_Item_FREQ_LOCK_CFG, &stAtCmd, sizeof(stAtCmd)) != NV_OK)
        {
            AT_ERR_LOG("AT_SetRatFreqLock(): en_NV_Item_FREQ_LOCK_CFG NV Write Fail!");
            return AT_ERROR;
        }

        return AT_OK;
    }

    stAtCmd.ulLockedFreq = gastAtParaList[1].ulParaValue;

    /* ���������еĽ���ģʽ */
    if (gastAtParaList[2].usParaLen == 0)
    {
        stAtCmd.enRatMode = AT_MTA_FREQLOCK_RATMODE_WCDMA;
    }
    else
    {
        stAtCmd.enRatMode = (AT_MTA_FREQLOCK_RATMODE_ENUM_UINT8)gastAtParaList[2].ulParaValue;
    }

    /* ���������е�BAND��Ϣ */
    if (gastAtParaList[3].usParaLen == 0)
    {
        if (stAtCmd.enRatMode == AT_MTA_FREQLOCK_RATMODE_GSM)
        {
            return AT_ERROR;
        }
        stAtCmd.enBand = AT_MTA_GSM_BAND_BUTT;
    }
    else
    {
        stAtCmd.enBand = (AT_MTA_GSM_BAND_ENUM_UINT16)gastAtParaList[3].ulParaValue;
    }

    /* ������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_FREQ_LOCK_REQ,
                                   &stAtCmd,
                                   sizeof(TAF_NVIM_FREQ_LOCK_CFG_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_RATFREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryRatFreqLock(VOS_UINT8 ucIndex)
{
    TAF_NVIM_FREQ_LOCK_CFG_STRU         stNvFreqLockCfg;

    memset_s(&stNvFreqLockCfg, sizeof(stNvFreqLockCfg), 0x00, sizeof(stNvFreqLockCfg));

    /* ͨ����ȡNV����ȡFreq Lock��ǰ����ֵ */
    if (NV_OK != TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_FREQ_LOCK_CFG,
                           &stNvFreqLockCfg,
                           sizeof(TAF_NVIM_FREQ_LOCK_CFG_STRU)))
    {
        AT_WARN_LOG("At_QryRatFreqLock: TAF_ACORE_NV_READ en_NV_Item_FREQ_LOCK_CFG fail!");
        return AT_ERROR;
    }

    /* ��Ƶ���ܹر������ֻ�ϱ�����ֵ:0 */
    if (stNvFreqLockCfg.ucEnableFlg == VOS_FALSE)
    {
        /* ��ѯ����ϱ� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        stNvFreqLockCfg.ucEnableFlg);

        return AT_OK;
    }

    /* ��Ƶ����������Gģ�ϣ���ѯ��Ҫ�ϱ�BAND��Ϣ */
    if (stNvFreqLockCfg.enRatMode == TAF_NVIM_RAT_MODE_GSM)
    {
        /* ��ѯ����ϱ� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d,%u,\"0%d\",\"0%d\"",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        stNvFreqLockCfg.ucEnableFlg,
                                                        stNvFreqLockCfg.ulLockedFreq,
                                                        stNvFreqLockCfg.enRatMode,
                                                        stNvFreqLockCfg.enBand);
    }
    else
    {
        /* ��ѯ����ϱ� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d,%u,\"0%d\"",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        stNvFreqLockCfg.ucEnableFlg,
                                                        stNvFreqLockCfg.ulLockedFreq,
                                                        stNvFreqLockCfg.enRatMode);
    }

    return AT_OK;
}


VOS_UINT32 AT_SetGFreqLock(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_GSM_FREQLOCK_REQ_STRU    stGFreqLockInfo;
    VOS_UINT32                          ulRst;

    /* ����������� */
    if ((gucAtParaIndex != 1) && (gucAtParaIndex != 3))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ʼ�� */
    memset_s(&stGFreqLockInfo, sizeof(stGFreqLockInfo), 0x00, sizeof(AT_MTA_SET_GSM_FREQLOCK_REQ_STRU));

    /* ������Ч�Լ�� */
    /* ��һ����������� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stGFreqLockInfo.enableFlag = (PS_BOOL_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* ��������Ƶ���������ڶ��������͵��������� */
    if (stGFreqLockInfo.enableFlag == PS_TRUE)
    {
        if ( (gastAtParaList[1].usParaLen == 0)
          || (gastAtParaList[2].usParaLen == 0) )
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        else
        {
            stGFreqLockInfo.usFreq = (VOS_UINT16)gastAtParaList[1].ulParaValue;
            stGFreqLockInfo.enBand = (AT_MTA_GSM_BAND_ENUM_UINT16)gastAtParaList[2].ulParaValue;
        }
    }

    /* ������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_GSM_FREQLOCK_REQ,
                                   &stGFreqLockInfo,
                                   sizeof(AT_MTA_SET_GSM_FREQLOCK_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_GSM_FREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


TAF_UINT32 AT_QryGFreqLock(TAF_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    /* ������ϢID_AT_MTA_QRY_GSM_FREQLOCK_REQ��AT������ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_QRY_GSM_FREQLOCK_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_GSM_FREQLOCK_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}
#endif


VOS_UINT32 AT_RcvMtaSetGFreqLockCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_GSM_FREQLOCK_CNF_STRU   *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ�� */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_GSM_FREQLOCK_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetGFreqLockCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetGFreqLockCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_GSM_FREQLOCK_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetGFreqLockCnf : Current Option is not AT_CMD_GSM_FREQLOCK_SET.");
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



VOS_UINT32 AT_RcvMtaGFreqLockQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pRcvMsg           = VOS_NULL_PTR;
    MTA_AT_QRY_GSM_FREQLOCK_CNF_STRU   *pstQryGFreqlockCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstQryGFreqlockCnf   = (MTA_AT_QRY_GSM_FREQLOCK_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaGFreqLockQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaGFreqLockQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_GSM_FREQLOCK_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaGFreqLockQryCnf : Current Option is not AT_CMD_GSM_FREQLOCK_QRY.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ��AT^GFREQLOCK��ѯ����� */
    gstAtSendData.usBufLen = 0;
    if (pstQryGFreqlockCnf->ulResult != VOS_OK)
    {
        ulResult = AT_CME_UNKNOWN;
    }
    else
    {
        ulResult = AT_OK;

        if (pstQryGFreqlockCnf->enLockFlg == VOS_FALSE)
        {
            gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                                            (TAF_CHAR*)pgucAtSndCodeAddr,
                                                            "%s: %d",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            (TAF_INT32)pstQryGFreqlockCnf->enLockFlg);
        }
        else
        {
            gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                                            (TAF_CHAR*)pgucAtSndCodeAddr,
                                                            "%s: %d,%d,%d",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            (TAF_INT32)pstQryGFreqlockCnf->enLockFlg,
                                                            (TAF_INT32)pstQryGFreqlockCnf->usFreq,
                                                            (TAF_INT32)pstQryGFreqlockCnf->enBand);
        }
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_VOID AT_NetMonFmtPlmnId(
    VOS_UINT32                          ulMcc,
    VOS_UINT32                          ulMnc,
    VOS_CHAR                           *pstrPlmn,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT32                          ulMaxLength;
    VOS_UINT32                          ulLength;

    ulLength                          = 0;
    ulMaxLength                       = AT_NETMON_PLMN_STRING_MAX_LENGTH;

    /* ��ʽ���MCC MNC */
    if ((ulMnc & 0x0f0000) == 0x0f0000)
    {
        ulLength = (VOS_UINT32)VOS_nsprintf_s( (VOS_CHAR *)pstrPlmn,
                                  (VOS_UINT32)ulMaxLength,
                                  (VOS_UINT32)ulMaxLength,
                                  "%X%X%X,%X%X",
                                  (ulMcc & 0x0f),
                                  (ulMcc & 0x0f00)>>8,
                                  (ulMcc & 0x0f0000)>>16,
                                  (ulMnc & 0x0f),
                                  (ulMnc & 0x0f00)>>8);
    }
    else
    {
        ulLength = (VOS_UINT32)VOS_nsprintf_s( (VOS_CHAR *)pstrPlmn,
                                  (VOS_UINT32)ulMaxLength,
                                  (VOS_UINT32)ulMaxLength,
                                  "%X%X%X,%X%X%X",
                                  (ulMcc & 0x0f),
                                  (ulMcc & 0x0f00)>>8,
                                  (ulMcc & 0x0f0000)>>16,
                                  (ulMnc & 0x0f),
                                  (ulMnc & 0xf00)>>8,
                                  (ulMnc & 0x0f0000)>>16);
    }

    /* ���ȷ�ת���� */
    if (ulLength >= AT_NETMON_PLMN_STRING_MAX_LENGTH)
    {
        AT_ERR_LOG("AT_NetMonFmtPlmnId: MCC ulLength is error!");
        *(pstrPlmn + AT_NETMON_PLMN_STRING_MAX_LENGTH - 1) = 0;
        *pusLength = AT_NETMON_PLMN_STRING_MAX_LENGTH;
        return;
    }

    *pusLength = (VOS_UINT16)ulLength;

    return;
}


VOS_VOID AT_NetMonFmtGsmSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    memset_s(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* ��ʽ���PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: GSM,%s,%d,%u,%d,%X,%X",
                                        pucPlmnstr,
                                        pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.enBand,
                                        pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ulArfcn,
                                        pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ucBsic,
                                        pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ulCellID,
                                        pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.usLac );

    /* RSSI��Чֵ������ʾ */
    if (pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.sRssi == AT_NETMON_GSM_RSSI_INVALID_VALUE)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.sRssi );
    }

    /* ��Чֵ������ʾ */
    if (pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ucRxQuality == AT_NETMON_GSM_RX_QUALITY_INVALID_VALUE)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.ucRxQuality );
    }

    /*���TA*/
    if (pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.bitOpTa == PS_IE_PRESENT)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stGsmSCellInfo.usTa );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtUtranFddSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    memset_s(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* ��ʽ���PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: WCDMA,%s",
                                        pucPlmnstr );

    /*���Ƶ��*/
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        ",%u",
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulArfcn );

    /* PSC��Чֵ������ʾ */
    if ((pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSCP == AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE)
        && (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sECN0 == AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE))
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.usPSC );
    }

    /*���Cell ID*/
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.bitOpCellID == PS_IE_PRESENT)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%X",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulCellID );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    /*���LAC*/
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.bitOpLAC == PS_IE_PRESENT)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                            ",%X",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.usLAC );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    /* RSCP��Чֵ������ʾ */
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSCP == AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSCP );
    }

    /* RSSI��Чֵ������ʾ */
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSSI == AT_NETMON_UTRAN_FDD_RSSI_INVALID_VALUE)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sRSSI );
    }

    /* ECN0��Чֵ������ʾ */
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sECN0 == AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.sECN0 );
    }

    /*���DRX*/
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.bitOpDRX == PS_IE_PRESENT)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.ulDrx );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    /*���URA Id*/
    if (pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.bitOpURA == PS_IE_PRESENT)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsFDD.usURA );
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "," );
    }

    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtGsmNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->u.stNCellInfo.ucGsmNCellCnt > NETMON_MAX_GSM_NCELL_NUM)
    {
        pstNCellInfo->u.stNCellInfo.ucGsmNCellCnt = NETMON_MAX_GSM_NCELL_NUM;
    }

    /* GSM������ʾ */
    for (ulLoop = 0; ulLoop < pstNCellInfo->u.stNCellInfo.ucGsmNCellCnt; ulLoop++)
    {
        /* �������Ĳ��ǵ�һ���������ȴ�ӡ�س����� */
        if (usLength != 0)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf );
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: GSM,%d,%u",
                                            pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].enBand,
                                            pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].ulAfrcn );

        /*���Bsic*/
        if (pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].bitOpBsic == PS_IE_PRESENT)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",%d",
                                                pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].ucBsic );
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "," );
        }

        /*���Cell ID*/
        if (pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].bitOpCellID == PS_IE_PRESENT)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",%X",
                                                pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].ulCellID );
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                                "," );
        }

         /*���LAC*/
        if (pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].bitOpLAC == PS_IE_PRESENT)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",%X",
                                               pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].usLAC );
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "," );
        }

        /*���RSSI*/
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",%d",
                                            pstNCellInfo->u.stNCellInfo.astGsmNCellInfo[ulLoop].sRSSI );
    }

    *pusOutLen = usLength;

    return;
}

VOS_VOID AT_NetMonFmtUtranFddNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->u.stNCellInfo.ucUtranNCellCnt > NETMON_MAX_UTRAN_NCELL_NUM)
    {
        pstNCellInfo->u.stNCellInfo.ucUtranNCellCnt = NETMON_MAX_UTRAN_NCELL_NUM;
    }

    /*WCDMA ������ʾ*/
    for (ulLoop = 0; ulLoop < pstNCellInfo->u.stNCellInfo.ucUtranNCellCnt; ulLoop++)
    {
        /* ������ǵ�һ�δ�ӡ��������ӡ�س����� */
        if (usLength != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf);
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                            "^MONNC: WCDMA,%u,%d,%d,%d",
                                            pstNCellInfo->u.stNCellInfo.u.astFddNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->u.stNCellInfo.u.astFddNCellInfo[ulLoop].usPSC,
                                            pstNCellInfo->u.stNCellInfo.u.astFddNCellInfo[ulLoop].sRSCP,
                                            pstNCellInfo->u.stNCellInfo.u.astFddNCellInfo[ulLoop].sECN0 );
    }

    *pusOutLen = usLength;

    return;
}


VOS_UINT32 At_SetNetMonSCellPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = AT_FillAndSndAppReqMsg( gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_SET_NETMON_SCELL_REQ,
                                    VOS_NULL_PTR,
                                    0,
                                    I0_UEPS_PID_MTA );

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MONSC_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetNetMonNCellPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = AT_FillAndSndAppReqMsg( gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_SET_NETMON_NCELL_REQ,
                                    VOS_NULL_PTR,
                                    0,
                                    I0_UEPS_PID_MTA );

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MONNC_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_UINT32 At_SetNetMonSSCellPara(
    VOS_UINT8                           atIndex
)
{
    VOS_UINT32                          rslt;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(gastAtClientTab[atIndex].usClientId,
                                  gastAtClientTab[atIndex].opId,
                                  ID_AT_MTA_SET_NETMON_SSCELL_REQ,
                                  VOS_NULL_PTR,
                                  0,
                                  I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        gastAtClientTab[atIndex].CmdCurrentOpt = AT_CMD_MONSSC_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else {
        return AT_ERROR;
    }
}



VOS_UINT32 AT_RcvMtaSetNetMonSSCellCnf(
    VOS_VOID                           *msg
)
{
    AT_MTA_MSG_STRU                    *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *cellInfo = VOS_NULL_PTR;
    VOS_UINT8                           atIndex;
    VOS_UINT16                          length;
    VOS_UINT32                          msgLength;

    /* ��ʼ�� */
    rcvMsg    = (AT_MTA_MSG_STRU *)msg;
    cellInfo  = (MTA_AT_NETMON_CELL_INFO_STRU *)rcvMsg->aucContent;

    length    = 0;
    atIndex   = 0;
    msgLength = sizeof(AT_MTA_MSG_STRU) - 4 + sizeof(MTA_AT_NETMON_CELL_INFO_STRU) - VOS_MSG_HEAD_LENGTH;

    if (rcvMsg->ulLength != msgLength) {
        AT_WARN_LOG("AT_RcvMtaSetNetMonSSCellCnf: WARNING: AT Length Is Wrong");
        return VOS_ERR;
    }

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(rcvMsg->stAppCtrl.usClientId, &atIndex) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvMtaSetNetMonSSCellCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(atIndex)) {
        AT_WARN_LOG("AT_RcvMtaSetNetMonSSCellCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[atIndex].CmdCurrentOpt != AT_CMD_MONSSC_SET) {
        AT_WARN_LOG("AT_RcvMtaSetNetMonSSCellCnf : Current Option is not AT_CMD_MONSSC_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(atIndex);

    if ((cellInfo->enResult == MTA_AT_RESULT_ERROR) ||
        (cellInfo->enCellType != MTA_NETMON_SSCELL_TYPE)) {
        gstAtSendData.usBufLen = length;
        At_FormatResultData(atIndex, AT_ERROR);
        return VOS_OK;
    }

    /* ����յ�����ظ��룬��ô˵����ʱ����L�£��ظ�NONE */
    if (cellInfo->enResult == MTA_AT_RESULT_OPERATION_NOT_ALLOWED) {
        length = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + length,
                                            "^MONSSC: NONE");
        gstAtSendData.usBufLen = length;
        At_FormatResultData(atIndex, AT_OK);
        return VOS_OK;
    }

    switch (cellInfo->enRatType) {
        case MTA_AT_NETMON_CELL_INFO_NR: {
            AT_NetMonFmtNrSSCellData(cellInfo, &length);
            break;
        }

        default:  {
            length = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + length,
                                            "^MONSSC: NONE");
            break;
        }
    }


    /* ������ */
    gstAtSendData.usBufLen = length;
    At_FormatResultData(atIndex, AT_OK);

    return VOS_OK;
}

#endif


VOS_UINT32 AT_RcvMtaSetNetMonSCellCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* ��ʼ�� */
    pstRcvMsg                         = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf                         = (MTA_AT_NETMON_CELL_INFO_STRU *)pstRcvMsg->aucContent;

    usLength                          = 0;
    ucIndex                           = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitServingCellCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitServingCellCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MONSC_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitServingCellCnf : Current Option is not AT_CMD_JDETEX_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ( (pstSetCnf->enResult == MTA_AT_RESULT_ERROR)
      || (pstSetCnf->enCellType != MTA_NETMON_SCELL_TYPE) )
    {
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_OK;
    }

    switch(pstSetCnf->enRatType)
    {
        case MTA_AT_NETMON_CELL_INFO_GSM:
        {
            AT_NetMonFmtGsmSCellData(pstSetCnf, &usLength);
            break;
        }
        case MTA_AT_NETMON_CELL_INFO_UTRAN_FDD:
        {
            AT_NetMonFmtUtranFddSCellData(pstSetCnf, &usLength);
            break;
        }
#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
        case MTA_AT_NETMON_CELL_INFO_UTRAN_TDD:
        {
            AT_NetMonFmtUtranTddSCellData(pstSetCnf, &usLength);
            break;
        }
#endif
#if (FEATURE_LTE == FEATURE_ON)
        case MTA_AT_NETMON_CELL_INFO_LTE:
        {
            AT_NetMonFmtEutranSCellData(pstSetCnf, &usLength);
            break;
        }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case MTA_AT_NETMON_CELL_INFO_NR:
        {
            AT_NetMonFmtNrSCellData(pstSetCnf, &usLength);
            break;
        }
#endif

        default:
            usLength += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "^MONSC: NONE" );
            break;
    }


    /* ������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaSetNetMonNCellCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    VOS_UINT16                          usLengthTemp;

    /* ��ʼ�� */
    pstRcvMsg                         = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf                         = (MTA_AT_NETMON_CELL_INFO_STRU *)pstRcvMsg->aucContent;

    ucIndex                           = 0;
    usLength                          = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitNeighCellCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitNeighCellCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MONNC_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetMonitNeighCellCnf : Current Option is not AT_CMD_JDETEX_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ( (pstSetCnf->enResult == MTA_AT_RESULT_ERROR)
      || (pstSetCnf->enCellType != MTA_NETMON_NCELL_TYPE) )
    {
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, AT_ERROR);

        return VOS_OK;
    }

    usLengthTemp   = 0;
    usLength       = 0;

    /* GSM������ʾ */
    AT_NetMonFmtGsmNCellData(pstSetCnf, usLengthTemp, &usLength);

    /* UTRAN������ʾ */
    usLengthTemp = usLength;

    if (pstSetCnf->u.stNCellInfo.enCellMeasTypeChoice == MTA_NETMON_UTRAN_FDD_TYPE)
    {
        AT_NetMonFmtUtranFddNCellData(pstSetCnf, usLengthTemp, &usLength);
    }
#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    else if (pstSetCnf->u.stNCellInfo.enCellMeasTypeChoice == MTA_NETMON_UTRAN_TDD_TYPE)
    {
        AT_NetMonFmtUtranTddNCellData(pstSetCnf, usLengthTemp, &usLength);
    }
#endif
    else
    {
        /*���Ͳ��ԣ��������κδ���*/
        ;
    }

#if (FEATURE_LTE == FEATURE_ON)
    /* LTE������ʾ */
    usLengthTemp = usLength;

    AT_NetMonFmtEutranNCellData(pstSetCnf, usLengthTemp, &usLength);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* NR������ʾ */
    usLengthTemp = usLength;

    AT_NetMonFmtNrNCellData(pstSetCnf, usLengthTemp, &usLength);
#endif

    /* ������������NONE */
    if ((pstSetCnf->u.stNCellInfo.ucGsmNCellCnt + pstSetCnf->u.stNCellInfo.ucUtranNCellCnt +
              pstSetCnf->u.stNCellInfo.ucLteNCellCnt + pstSetCnf->u.stNCellInfo.ucNrNCellCnt) == 0)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: NONE" );
    }

    /* ������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)

VOS_VOID AT_NetMonFmtUtranTddSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    memset_s(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* ��ʽ���PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: TD_SCDMA,%s",
                                        pucPlmnstr );

    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        ",%u,%d,%d,%X,%X,%d,%d,%d",
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulArfcn,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.usSyncID,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.usSC,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.ulCellID,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.usLAC,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.sRSCP,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.ulDrx,
                                        pstSCellInfo->u.unSCellInfo.stUtranSCellInfo.u.stCellMeasResultsTDD.usRac );

    *pusLength = usLength;

    return;
}

VOS_VOID AT_NetMonFmtUtranTddNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->u.stNCellInfo.ucUtranNCellCnt > NETMON_MAX_UTRAN_NCELL_NUM)
    {
        pstNCellInfo->u.stNCellInfo.ucUtranNCellCnt = NETMON_MAX_UTRAN_NCELL_NUM;
    }

    /*WCDMA ������ʾ*/
    for (ulLoop = 0; ulLoop < pstNCellInfo->u.stNCellInfo.ucUtranNCellCnt; ulLoop++)
    {
        /* ������ǵ�һ�δ�ӡ��������ӡ�س����� */
        if (usLength != 0)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                 "%s",
                                                 gaucAtCrLf );
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: TD_SCDMA,%u,%d,%d,%d",
                                            pstNCellInfo->u.stNCellInfo.u.astTddNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->u.stNCellInfo.u.astTddNCellInfo[ulLoop].usSyncID,
                                            pstNCellInfo->u.stNCellInfo.u.astTddNCellInfo[ulLoop].usSC,
                                            pstNCellInfo->u.stNCellInfo.u.astTddNCellInfo[ulLoop].sRSCP );
    }

    *pusOutLen = usLength;

    return;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)

VOS_VOID AT_NetMonFmtEutranSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    memset_s(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));

    ulMcc = pstSCellInfo->u.unSCellInfo.stLteSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->u.unSCellInfo.stLteSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    /* ��ʽ���PLMN */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^MONSC: LTE,%s",
                                        pucPlmnstr );

    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        ",%u,%X,%X,%X,%d,%d,%d",
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.ulArfcn,
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.ulCellID,
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.ulPID,
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.usTAC,
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.sRSRP,
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.sRSRQ,
                                        pstSCellInfo->u.unSCellInfo.stLteSCellInfo.sRSSI );
    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtEutranNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;

    usLength                          = usInLen;

    if (pstNCellInfo->u.stNCellInfo.ucLteNCellCnt > NETMON_MAX_LTE_NCELL_NUM)
    {
        pstNCellInfo->u.stNCellInfo.ucLteNCellCnt = NETMON_MAX_LTE_NCELL_NUM;
    }

     /* LTE������ʾ */
    for (ulLoop = 0; ulLoop < pstNCellInfo->u.stNCellInfo.ucLteNCellCnt; ulLoop++)
    {
        /* ������ǵ�һ�δ�ӡ��������ӡ�س����� */
        if (usLength != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf);
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: LTE,%u,%X,%d,%d,%d",
                                            pstNCellInfo->u.stNCellInfo.astLteNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->u.stNCellInfo.astLteNCellInfo[ulLoop].ulPID,
                                            pstNCellInfo->u.stNCellInfo.astLteNCellInfo[ulLoop].sRSRP,
                                            pstNCellInfo->u.stNCellInfo.astLteNCellInfo[ulLoop].sRSRQ,
                                            pstNCellInfo->u.stNCellInfo.astLteNCellInfo[ulLoop].sRSSI );
    }

    *pusOutLen = usLength;

    return;
}


VOS_VOID AT_FmtTimeStru(
    MTA_AT_TIME_STRU                   *pstTimeStru,
    VOS_UINT16                         *pusOutLen,
    VOS_UINT16                          usInLen
)
{
    VOS_UINT16                          usLength;

    usLength = usInLen;

    /* YY */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d/",
                                       pstTimeStru->usYear / 10,
                                       pstTimeStru->usYear % 10);
    /* MM */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d/",
                                       pstTimeStru->ucMonth / 10,
                                       pstTimeStru->ucMonth % 10);
    /* dd */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d,",
                                       pstTimeStru->ucDay / 10,
                                       pstTimeStru->ucDay % 10);

    /* hh */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d:",
                                       pstTimeStru->ucHour / 10,
                                       pstTimeStru->ucHour % 10);

    /* mm */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d:",
                                       pstTimeStru->ucMinute / 10,
                                       pstTimeStru->ucMinute % 10);

    /* ss */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d%d, ",
                                       pstTimeStru->ucSecond / 10,
                                       pstTimeStru->ucSecond % 10);

   *pusOutLen = usLength;

    return;
}

/*lint -save -e845 -specific(-e845)*/

VOS_VOID AT_FormatRsrp(
    VOS_UINT8                           ucIndex,
    MTA_AT_RS_INFO_QRY_CNF_STRU        *pstRsInfoQryCnf
)
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    usLength = 0;

    /* ��ʽ��AT+RSRP?��ѯ����� */
    gstAtSendData.usBufLen = 0;

    /* +RSRP: */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if ( (pstRsInfoQryCnf->enResult != MTA_AT_RESULT_NO_ERROR)
      || (pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum == 0) )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "NONE");

        gstAtSendData.usBufLen = usLength;
        return;
    }

    for (i = 0; i < pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].ulCellId);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].ulEarfcn);

        if (pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp >= 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d.%02d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp / AT_RS_INFO_MULTI,
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp % AT_RS_INFO_MULTI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",-%d.%02d",
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp) / AT_RS_INFO_MULTI,
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrpInfo[i].lRsrp) % AT_RS_INFO_MULTI);
        }


        if (i == pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum - 1)
        {
            break;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");
    }

    gstAtSendData.usBufLen = usLength;
    return;
}


VOS_VOID AT_FormatRsrq(
    VOS_UINT8                           ucIndex,
    MTA_AT_RS_INFO_QRY_CNF_STRU        *pstRsInfoQryCnf
)
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    usLength = 0;

    /* ��ʽ��AT+RSRQ?��ѯ����� */
    gstAtSendData.usBufLen = 0;

    /* +RSRQ: */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if ( (pstRsInfoQryCnf->enResult != MTA_AT_RESULT_NO_ERROR)
      || (pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum == 0) )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "NONE");

        gstAtSendData.usBufLen = usLength;
        return;
    }

    for (i = 0; i < pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].ulCellId);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].ulEarfcn);

        if (pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq >= 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d.%02d",
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq / AT_RS_INFO_MULTI,
                                           pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq % AT_RS_INFO_MULTI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",-%d.%02d",
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq) / AT_RS_INFO_MULTI,
                                           (-pstRsInfoQryCnf->stRsInfoRslt.u.astRsrqInfo[i].lRsrq) % AT_RS_INFO_MULTI);
        }

        if (i == pstRsInfoQryCnf->stRsInfoRslt.ulRsInfoNum - 1)
        {
            break;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");
    }

    gstAtSendData.usBufLen = usLength;
    return;
}
/*lint -restore*/
#endif


VOS_VOID AT_FormatGasAtCmdRslt(
    MTA_AT_GAS_AUTOTEST_QRY_RSLT_STRU                      *pstAtCmdRslt)
{
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    VOS_UINT16                          usLength;

    i = 0;
    j = 0;
    usLength = 0;

    for (i = 0; i< (pstAtCmdRslt->ulRsltNum/3); i++)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr+usLength,
                                           "%d:%d,",
                                           (i+1),
                                           pstAtCmdRslt->aulRslt[j++]);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr+usLength,
                                           "0x%X,",
                                           pstAtCmdRslt->aulRslt[j++]&0xff);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr+usLength,
                                           "%d\r\n",
                                           pstAtCmdRslt->aulRslt[j++]);
    }

    gstAtSendData.usBufLen = usLength;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_VOID AT_NetMonFmtNrSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstSCellInfo,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           pucPlmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
    VOS_UINT16                          ulPlmnStrLen;
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucCellIdStr[AT_CELLID_STRING_MAX_LEN];

    usLength                          = 0;
    ulPlmnStrLen                      = 0;
    memset_s(pucPlmnstr, sizeof(pucPlmnstr), 0x00, sizeof(pucPlmnstr));
    memset_s(aucCellIdStr, sizeof(aucCellIdStr), 0x00, sizeof(aucCellIdStr));

    ulMcc = pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulMcc;
    ulMnc = pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulMnc;

    AT_NetMonFmtPlmnId(ulMcc, ulMnc, (VOS_CHAR *)pucPlmnstr, &ulPlmnStrLen);

    AT_ConvertCellIdToHexStrFormat(pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulCellIdentityLowBit,
                                   pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulCellIdentityHighBit,
                                   (VOS_CHAR *)aucCellIdStr);

    /* ��ʽ���PLMN */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^MONSC: NR,%s,%u,%d,%s,%X,%X,%d,%d,%d",
                                       pucPlmnstr,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulArfcn,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.enNrScsType,
                                       aucCellIdStr,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulPhyCellID,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.ulTac,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.sRSRP,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.sRSRQ,
                                       pstSCellInfo->u.unSCellInfo.stNrSCellInfo.sSINR);

    *pusLength = usLength;

    return;
}


VOS_VOID AT_NetMonFmtNrSSCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *cellInfo,
    VOS_UINT16                         *outLen
)
{
    VOS_UINT32                          loop;
    VOS_UINT16                          length;
    VOS_UINT32                          nrCcCnt;

    length = 0;
    nrCcCnt = cellInfo->u.secSrvCellInfo.componentCarrierNum;

    if (nrCcCnt > NETMON_MAX_NR_CC_NUM)
    {
        nrCcCnt = NETMON_MAX_NR_CC_NUM;
    }

     /* NR CC��ӡ */
    for (loop = 0; loop < nrCcCnt; loop++)
    {
        /* ������ǵ�һ�δ�ӡCC����ӡ�س����� */
        if (length != 0)
        {
            length += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + length,
                                             "%s",
                                             gaucAtCrLf);
        }

        length += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + length,
                                         "^MONSSC: NR,%u,%X,%d,%d,%d,%d",
                                         cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].arfcn,
                                         cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].phyCellId,
                                         cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].rsrp,
                                         cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].rsrq,
                                         cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].sinr,
                                         cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].measRsType);
    }

    *outLen = length;

    return;
}



VOS_VOID AT_NetMonFmtNrNCellData(
    MTA_AT_NETMON_CELL_INFO_STRU       *pstNCellInfo,
    VOS_UINT16                          usInLen,
    VOS_UINT16                         *pusOutLen
)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucNrNCellCnt;

    usLength     = usInLen;
    ucNrNCellCnt = pstNCellInfo->u.stNCellInfo.ucNrNCellCnt;

    if (ucNrNCellCnt > NETMON_MAX_NR_NCELL_NUM)
    {
        ucNrNCellCnt = NETMON_MAX_NR_NCELL_NUM;
    }

     /* NR������ʾ */
    for (ulLoop = 0; ulLoop < ucNrNCellCnt; ulLoop++)
    {
        /* ������ǵ�һ�δ�ӡ��������ӡ�س����� */
        if (usLength != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf);
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "^MONNC: NR,%u,%X,%d,%d,%d",
                                            pstNCellInfo->u.stNCellInfo.astNrNCellInfo[ulLoop].ulArfcn,
                                            pstNCellInfo->u.stNCellInfo.astNrNCellInfo[ulLoop].ulPhyCellID,
                                            pstNCellInfo->u.stNCellInfo.astNrNCellInfo[ulLoop].sRSRP,
                                            pstNCellInfo->u.stNCellInfo.astNrNCellInfo[ulLoop].sRSRQ,
                                            pstNCellInfo->u.stNCellInfo.astNrNCellInfo[ulLoop].sSINR);
    }

    *pusOutLen = usLength;

    return;
}


VOS_UINT32 AT_RcvMtaSetTrxTasCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg = VOS_NULL_PTR;
    MTA_AT_SET_TRX_TAS_CNF_STRU        *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    ucIndex            = 0;
    pstMtaMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf          = (MTA_AT_SET_TRX_TAS_CNF_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetTrxTasCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetTrxTasCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TRX_TAS_SET)
    {
        AT_WARN_LOG("AT_RcvMtaSetTrxTasCnf: WARNING:Not AT_CMD_TRX_TAS_SET!");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstSetCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult    = AT_OK;
    }
    else
    {
        ulResult    = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaQryTrxTasCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg = VOS_NULL_PTR;
    MTA_AT_QRY_TRX_TAS_CNF_STRU        *pstQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    ucIndex            = 0;
    pstMtaMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstQryCnf          = (MTA_AT_QRY_TRX_TAS_CNF_STRU *)pstMtaMsg->aucContent;

    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaQryTrxTasCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryTrxTasCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TRX_TAS_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaQryTrxTasCnf: WARNING:Not AT_CMD_TRX_TAS_QRY !");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstQryCnf->enRatMode,
                                                        pstQryCnf->ulTrxTasValue);
    }
    else
    {
        ulResult = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif


VOS_UINT32 AT_CheckJDCfgGsmPara(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* �ж�ƽ̨�Ƿ�֧��GSM */
    if (AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_GSM) != VOS_TRUE)
    {
        AT_ERR_LOG("AT_CheckJDCfgGsmPara: Not Support GSM.");
        return VOS_ERR;
    }

    /* ���������ʽ:AT^JDCFG=0,<rssi_thresh>,<rssi_num> */
    if (gucAtParaIndex != AT_JAM_DETECT_GSM_PARA_NUM)
    {
        return VOS_ERR;
    }

    /* ���ò�����Χ��� */
    if ((gastAtParaList[1].usParaLen == 0)
        || (gastAtParaList[1].ulParaValue > AT_JAM_DETECT_GSM_THRESHOLD_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[2].usParaLen == 0)
        || (gastAtParaList[2].ulParaValue > AT_JAM_DETECT_GSM_FREQNUM_MAX))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckJDCfgWcdmaPara(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* �ж�ƽ̨�Ƿ�֧��WCDMA */
    if (AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_WCDMA) != VOS_TRUE)
    {
        AT_ERR_LOG("AT_CheckJDCfgWcdmaPara: Not Support WCDMA.");
        return VOS_ERR;
    }

    /* ���������ʽ:AT^JDCFG=1,<rssi_thresh>,<rssi_percent>,<psch_thresh>,<psch_percent> */
    if (gucAtParaIndex != AT_JAM_DETECT_WL_PARA_NUM)
    {
        return VOS_ERR;
    }

    /* ���ò�����Χ��� */
    if ((gastAtParaList[1].usParaLen == 0) ||
        (gastAtParaList[1].ulParaValue > AT_JAM_DETECT_WCDMA_RSSI_THRESHOLD_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[2].usParaLen == 0) ||
        (gastAtParaList[2].ulParaValue > AT_JAM_DETECT_WCDMA_RSSI_PERCENT_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[3].usParaLen == 0) ||
        (gastAtParaList[3].ulParaValue > AT_JAM_DETECT_WCDMA_PSCH_THRESHOLD_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[4].usParaLen == 0) ||
        (gastAtParaList[4].ulParaValue > AT_JAM_DETECT_WCDMA_PSCH_PERCENT_MAX))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckJDCfgLtePara(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = MODEM_ID_0;

    /* �ж�ƽ̨�Ƿ�֧��LTE */
    if (AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_LTE) != VOS_TRUE)
    {
        AT_ERR_LOG("AT_CheckJDCfgLtePara: Not Support LTE.");
        return VOS_ERR;
    }

    /* ���������ʽ:AT^JDCFG=1,<rssi_thresh>,<rssi_percent>,<pssratio_thresh>,<pssratio_percent> */
    if (gucAtParaIndex != AT_JAM_DETECT_WL_PARA_NUM)
    {
        return VOS_ERR;
    }

    /* ���ò�����Χ��� */
    if ((gastAtParaList[1].usParaLen == 0) ||
        (gastAtParaList[1].ulParaValue > AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[2].usParaLen == 0) ||
        (gastAtParaList[2].ulParaValue > AT_JAM_DETECT_LTE_RSSI_PERCENT_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[3].usParaLen == 0) ||
        (gastAtParaList[3].ulParaValue > AT_JAM_DETECT_LTE_PSSRATIO_THRESHOLD_MAX))
    {
        return VOS_ERR;
    }

    if ((gastAtParaList[4].usParaLen == 0) ||
        (gastAtParaList[4].ulParaValue > AT_JAM_DETECT_LTE_PSSRATIO_PERCENT_MAX))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_CheckJDCfgPara(VOS_VOID)
{
    VOS_UINT32                          ulRst = VOS_ERR;

    /* ����������� */
    if ((gucAtParaIndex != AT_JAM_DETECT_GSM_PARA_NUM) && (gucAtParaIndex != AT_JAM_DETECT_WL_PARA_NUM))
    {
        return VOS_ERR;
    }

    /* ���GUL��ʽ�£�JD���ò�����Ч�� */
    switch ((AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue)
    {
        case AT_MTA_CMD_RATMODE_GSM:
            ulRst = AT_CheckJDCfgGsmPara();
            break;

        case AT_MTA_CMD_RATMODE_WCDMA:
            ulRst = AT_CheckJDCfgWcdmaPara();
            break;

        case AT_MTA_CMD_RATMODE_LTE:
            ulRst = AT_CheckJDCfgLtePara();
            break;

        default:
            break;
    }

    return ulRst;
}


VOS_UINT32 AT_ProcJDCfgPara(AT_MTA_SET_JAM_DETECT_REQ_STRU *pstAtCmd)
{
    /* ��ȡGUL��ʽ�£����ż�����ò��� */
    pstAtCmd->ucMode = AT_MTA_JAM_DETECT_MODE_UPDATE;
    pstAtCmd->ucRat  = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    switch ((AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue)
    {
        case AT_MTA_CMD_RATMODE_GSM:
            pstAtCmd->unJamPara.stGsmPara.ucThreshold   = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstAtCmd->unJamPara.stGsmPara.ucFreqNum     = (VOS_UINT8)gastAtParaList[2].ulParaValue;
            break;

        case AT_MTA_CMD_RATMODE_WCDMA:
            pstAtCmd->unJamPara.stWcdmaPara.ucRssiSrhThreshold    = (VOS_UINT8)gastAtParaList[1].ulParaValue;
            pstAtCmd->unJamPara.stWcdmaPara.ucRssiSrhFreqPercent  = (VOS_UINT8)gastAtParaList[2].ulParaValue;
            pstAtCmd->unJamPara.stWcdmaPara.usPschSrhThreshold    = (VOS_UINT16)gastAtParaList[3].ulParaValue;
            pstAtCmd->unJamPara.stWcdmaPara.ucPschSrhFreqPercent  = (VOS_UINT8)gastAtParaList[4].ulParaValue;
            break;

        case AT_MTA_CMD_RATMODE_LTE:
            pstAtCmd->unJamPara.stLtePara.sRssiThresh         = (VOS_INT16)gastAtParaList[1].ulParaValue - AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX;
            pstAtCmd->unJamPara.stLtePara.ucRssiPercent       = (VOS_UINT8)gastAtParaList[2].ulParaValue;
            pstAtCmd->unJamPara.stLtePara.usPssratioThresh    = (VOS_UINT16)gastAtParaList[3].ulParaValue;
            pstAtCmd->unJamPara.stLtePara.ucPssratioPercent   = (VOS_UINT8)gastAtParaList[4].ulParaValue;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetJDCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_JAM_DETECT_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ч�Լ�� */
    ulRst = AT_CheckJDCfgPara();

    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG("AT_SetJDCfgPara: AT_CheckJDCfgPara error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_JAM_DETECT_REQ_STRU));

    /* ��ȡ���ż�����ò��� */
    ulRst = AT_ProcJDCfgPara(&stAtCmd);

    if (ulRst != VOS_OK)
    {
        return AT_ERROR;
    }

    /* �·����ż���������������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SET_JAM_DETECT_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_JAM_DETECT_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDCFG_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryJDCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulRst;

    /* �·����ż�����ò�ѯ������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_QRY_JAM_DETECT_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDCFG_READ;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestJDCfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;

    /* ���GSM�Ĳ����б� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr+usLength,
                                       "%s: (0),(0-70),(0-255)%s",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       gaucAtCrLf);

    /* ���WCDMA�Ĳ����б� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr+usLength,
                                       "%s: (1),(0-70),(0-100),(0-65535),(0-100)%s",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       gaucAtCrLf);

    /* ���LTE�Ĳ����б� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr+usLength,
                                       "%s: (2),(0-70),(0-100),(0-1000),(0-100)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetJDSwitchPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_JAM_DETECT_REQ_STRU      stAtCmd = {0};
    VOS_UINT32                          ulRst;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    /* �·����ż�⿪������������Ϣ��C�˴��� */
    stAtCmd.ucMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ucRat  = AT_MTA_CMD_RATMODE_BUTT;

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SET_JAM_DETECT_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_JAM_DETECT_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDSWITCH_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryJDSwitchPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulRst;

    /* �·�JD��ѯ������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_QRY_JAM_DETECT_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_JDSWITCH_READ;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMtaSetJamDetectCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_SET_JAM_DETECT_CNF_STRU *pstSetCnf = VOS_NULL_PTR;
    VOS_UINT32                      ulResult  = AT_OK;
    VOS_UINT8                       ucIndex   = 0;

    /* ��ʼ�� */
    pstRcvMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf = (MTA_AT_SET_JAM_DETECT_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaSetJamDetectCfgCnf : WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetJamDetectCfgCnf : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_JDCFG_SET)
        && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_JDSWITCH_SET))
    {
        AT_WARN_LOG("AT_RcvMtaSetJamDetectCfgCnf : Current Option is not AT_CMD_JDCFG_SET or AT_CMD_JDSWITCH_SET.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    if (pstSetCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaQryJamDetectCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                *pstRcvMsg = VOS_NULL_PTR;
    MTA_AT_QRY_JAM_DETECT_CNF_STRU *pstQryCnf = VOS_NULL_PTR;
    VOS_UINT8                       ucIndex   = 0;
    VOS_UINT16                      usLength  = 0;

    /* ��ʼ�� */
    pstRcvMsg = (AT_MTA_MSG_STRU *)pMsg;
    pstQryCnf = (MTA_AT_QRY_JAM_DETECT_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaQryJamDetectCfgCnf : WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryJamDetectCfgCnf : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_JDCFG_READ:
            /* GSM��ʽ�����ż�����ò��� */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               AT_MTA_CMD_RATMODE_GSM,
                                               pstQryCnf->stGsmPara.ucThreshold,
                                               pstQryCnf->stGsmPara.ucFreqNum,
                                               gaucAtCrLf);

            /* WCDMA��ʽ�����ż�����ò��� */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d,%d,%d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               AT_MTA_CMD_RATMODE_WCDMA,
                                               pstQryCnf->stWcdmaPara.ucRssiSrhThreshold,
                                               pstQryCnf->stWcdmaPara.ucRssiSrhFreqPercent,
                                               pstQryCnf->stWcdmaPara.usPschSrhThreshold,
                                               pstQryCnf->stWcdmaPara.ucPschSrhFreqPercent,
                                               gaucAtCrLf);

            /* LTE��ʽ�����ż�����ò��� */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d,%d,%d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               AT_MTA_CMD_RATMODE_LTE,
                                               pstQryCnf->stLtePara.sRssiThresh + AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX,
                                               pstQryCnf->stLtePara.ucRssiPercent,
                                               pstQryCnf->stLtePara.usPssratioThresh,
                                               pstQryCnf->stLtePara.ucPssratioPercent);

            gstAtSendData.usBufLen = usLength;

            /* ��λAT״̬ */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, AT_OK);

            return VOS_OK;

        case AT_CMD_JDSWITCH_READ:
            /* GSM��ʽ�����ż�⿪��״̬ */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstQryCnf->ucGsmJamMode,
                                               AT_MTA_CMD_RATMODE_GSM,
                                               gaucAtCrLf);

            /* WCDMA��ʽ�����ż�⿪��״̬ */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d%s",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstQryCnf->ucWcdmaJamMode,
                                               AT_MTA_CMD_RATMODE_WCDMA,
                                               gaucAtCrLf);

            /* LTE��ʽ�����ż�⿪��״̬ */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+usLength,
                                               "%s: %d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstQryCnf->ucLteJamMode,
                                               AT_MTA_CMD_RATMODE_LTE);

            gstAtSendData.usBufLen = usLength;

            /* ��λAT״̬ */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, AT_OK);

            return VOS_OK;

        default:
            AT_WARN_LOG("AT_RcvMtaMbbQryJamDetectCnf : Current Option is not AT_CMD_JDCFG_READ or AT_CMD_JDSWITCH_READ.");

            return VOS_ERR;
    }
}


VOS_UINT32 AT_RcvMtaJamDetectInd(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex         = 0;
    VOS_UINT16                          usLength        = 0;
    AT_MTA_MSG_STRU                    *pstMtaMsg       = VOS_NULL_PTR;
    MTA_AT_JAM_DETECT_IND_STRU         *pstJamDetectInd = VOS_NULL_PTR;

    /* ��ʼ����Ϣ���� */
    pstMtaMsg       = (AT_MTA_MSG_STRU *)pMsg;
    pstJamDetectInd = (MTA_AT_JAM_DETECT_IND_STRU *)pstMtaMsg->aucContent;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaJamDetectInd: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    /* �ϱ����ż���� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s^JDINFO: %d,%d%s",
                                       gaucAtCrLf,
                                       pstJamDetectInd->enJamResult,
                                       pstJamDetectInd->ucRat,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_SetEmRssiCfgPara(VOS_UINT8 ucIndex)
{
    TAF_MMA_EMRSSICFG_REQ_STRU          stEmRssiCfgPara;

    memset_s(&stEmRssiCfgPara, sizeof(stEmRssiCfgPara), 0x00, sizeof(TAF_MMA_EMRSSICFG_REQ_STRU));

    /* ����Ƿ��������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("At_SetEmRssiCfgPara: Not Set Command!");
        return AT_ERROR;
    }

    /* ���������� */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("At_SetEmRssiCfgPara: Input parameters go wrong!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����η�װ���ṹ�����ٷ������� */
    stEmRssiCfgPara.ucEmRssiCfgRat       = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stEmRssiCfgPara.ucEmRssiCfgThreshold = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* ������Ϣ��C�˴��� */
    if (TAF_MMA_SetEmRssiCfgReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId,
                                            &stEmRssiCfgPara) == VOS_TRUE)
    {

        /* ������������״̬ */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSICFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("At_SetEmRssiCfgPara: AT send ERROR!");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryEmRssiCfgPara(VOS_UINT8 ucIndex)
{
    /* ִ��������� */
    if (TAF_MMA_QryEmRssiCfgReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId) == VOS_TRUE)
    {

        /* ������������״̬ */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSICFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetEmRssiRptPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucEmRssiRptSwitch;

    /* ����Ƿ��������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("At_SetEmRssiRptPara: Not Set Command!");
        return AT_ERROR;
    }

    /* ���������� */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("At_SetEmRssiRptPara: Input parameters go wrong!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ucEmRssiRptSwitch = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* ������Ϣ��C�˴��� */
    if (TAF_MMA_SetEmRssiRptReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId,
                                            &ucEmRssiRptSwitch) == VOS_TRUE)
    {

        /* ������������״̬ */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSIRPT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryEmRssiRptPara(VOS_UINT8 ucIndex)
{
    /* ִ��������� */
    if (TAF_MMA_QryEmRssiRptReq(WUEPS_PID_AT,
                                            gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId) == VOS_TRUE)
    {

        /* ������������״̬ */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMRSSIRPT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_RcvMmaEmRssiCfgSetCnf(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_EMRSSICFG_SET_CNF_STRU     *pstCnfMsg = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex = 0;
    VOS_UINT32                          ulResult = AT_OK;

    pstCnfMsg = (TAF_MMA_EMRSSICFG_SET_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡucIndex */
    if (At_ClientIdToUserId(pstCnfMsg->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EMRSSICFG_SET)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: Current Option is not AT_CMD_EMRSSICFG_SET.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCnfMsg->enErrorCause != TAF_ERR_NO_ERROR)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgSetCnf: Set command go wrong!");
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaEmRssiCfgQryCnf(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex            = 0;
    VOS_UINT16                          usLength           = 0;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult           = AT_FAILURE;
    TAF_MMA_EMRSSICFG_QRY_CNF_STRU     *pstEmRssiCfgQryCnf = VOS_NULL_PTR;

    pstEmRssiCfgQryCnf = (TAF_MMA_EMRSSICFG_QRY_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEmRssiCfgQryCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EMRSSICFG_QRY)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: Current Option is not AT_CMD_EMRSSICFG_QRY.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstEmRssiCfgQryCnf->enErrorCause != TAF_ERR_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        /* GSM EMRSSICFG��� */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           TAF_MMA_RAT_GSM,
                                           pstEmRssiCfgQryCnf->stEmRssiCfgPara.ucEmRssiCfgGsmThreshold,
                                           gaucAtCrLf);

        /* WCDMA EMRSSICFG��� */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           TAF_MMA_RAT_WCDMA,
                                           pstEmRssiCfgQryCnf->stEmRssiCfgPara.ucEmRssiCfgWcdmaThreshold,
                                           gaucAtCrLf);

        /* LTE EMRSSICFG��� */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           TAF_MMA_RAT_LTE,
                                           pstEmRssiCfgQryCnf->stEmRssiCfgPara.ucEmRssiCfgLteThreshold);

        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaEmRssiRptSetCnf(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_EMRSSIRPT_SET_CNF_STRU     *pstCnfMsg = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex   = 0;
    VOS_UINT32                          ulResult  = AT_ERROR;

    pstCnfMsg = (TAF_MMA_EMRSSIRPT_SET_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡucIndex */
    if (At_ClientIdToUserId(pstCnfMsg->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EMRSSIRPT_SET)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: Current Option is not AT_CMD_EMRSSIRPT_SET.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCnfMsg->enErrorCause != TAF_ERR_NO_ERROR)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptSetCnf: Set command go wrong!");
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaEmRssiRptQryCnf(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex            = 0;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult           = AT_FAILURE;
    TAF_UINT16                          usLength           = 0;
    TAF_MMA_EMRSSIRPT_QRY_CNF_STRU     *pstEmRssiRptQryCnf = VOS_NULL_PTR;

    pstEmRssiRptQryCnf = (TAF_MMA_EMRSSIRPT_QRY_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEmRssiRptQryCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EMRSSIRPT_QRY)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiCfgQryCnf: Current Option is not AT_CMD_EMRSSIRPT_QRY.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstEmRssiRptQryCnf->enErrorCause != TAF_ERR_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstEmRssiRptQryCnf->ucEmRssiRptSwitch);

        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_VOID AT_ConvertRssiLevel(VOS_INT16 sRssiValue, VOS_UINT8 *pucRssiLevel)
{
    if(sRssiValue >= AT_HCSQ_VALUE_INVALID)
    {
        *pucRssiLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (sRssiValue >= AT_HCSQ_RSSI_VALUE_MAX)
    {
        *pucRssiLevel = AT_HCSQ_RSSI_LEVEL_MAX;
    }
    else if (sRssiValue < AT_HCSQ_RSSI_VALUE_MIN)
    {
        *pucRssiLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucRssiLevel = (VOS_UINT8)((sRssiValue - AT_HCSQ_RSSI_VALUE_MIN) + 1);
    }

    return;
}


VOS_UINT32 AT_RcvMmaEmRssiRptInd(VOS_VOID *pstMsg)
{
    TAF_MMA_RSSI_INFO_IND_STRU         *pstEmRssiRptInd = VOS_NULL_PTR;
    VOS_UINT8                           ucRssiValue     = 0;
    VOS_UINT8                           ucIndex         = 0;
    VOS_UINT16                          usLength        = 0;
    VOS_INT16                           sEmRssi         = 0;

    pstEmRssiRptInd = (TAF_MMA_RSSI_INFO_IND_STRU *)pstMsg;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstEmRssiRptInd->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaEmRssiRptInd:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    switch (pstEmRssiRptInd->stRssiInfo.enRatType)
    {
        case TAF_MMA_RAT_GSM:
        {
            sEmRssi = pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stGCellSignInfo.sRssiValue;
            break;
        }
        case TAF_MMA_RAT_WCDMA:
        {
            sEmRssi = pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sRscpValue
                      - pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sEcioValue;
            break;
        }
        case TAF_MMA_RAT_LTE:
        {
            sEmRssi = pstEmRssiRptInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRssi;
            break;
        }
        default:
        {
            return VOS_ERR;
        }
    }

    AT_ConvertRssiLevel(sEmRssi, &ucRssiValue);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d,%d%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_EMRSSIRPT].pucText,
                                       pstEmRssiRptInd->stRssiInfo.enRatType,
                                       ucRssiValue,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr,usLength);

    return VOS_OK;
}



LOCAL VOS_UINT32 AT_LtePwrDissParaCheck(AT_MTA_LTEPWRDISS_SET_REQ_STRU  *pstAtCmd)
{
    VOS_INT32                           lVal;
    VOS_UINT32                          ulRst;

    lVal = 0;
    if (AT_AtoInt((VOS_CHAR *)gastAtParaList[1].aucPara, &lVal) == VOS_ERR)
    {
        return VOS_ERR;
    }

    ulRst = VOS_ERR;
    switch ((AT_LTEPWRCTRL_MODE_TYPE_ENUM_UINT32)gastAtParaList[0].ulParaValue)
    {
        case AT_LTEPWRCTRL_MODE_CC_NUM_CTR:
        {
            if ((lVal >= AT_LTEPWRCTRL_MAX_DELETE_CA_NUM)
             && (lVal <= AT_LTEPWRCTRL_MAX_ADD_CA_NUM))
            {
                ulRst = VOS_OK;
            }
            break;
        }
        case AT_LTEPWRCTRL_MODE_RI_NUM_CTR:
        {
            if ((lVal == AT_LTEPWRCTRL_DISABLE_REDUCE_RI)
             || (lVal == AT_LTEPWRCTRL_ENABLE_REDUCE_RI))
            {
                ulRst = VOS_OK;
            }
            break;
        }
        case AT_LTEPWRCTRL_MODE_BSR_NUM_CTR:
        {
            if ((lVal >= AT_LTEPWRCTRL_MIN_SUPPORT_BSR_NUM)
             && (lVal <= AT_LTEPWRCTRL_MAX_SUPPORT_BSR_NUM))
            {
                ulRst = VOS_OK;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (ulRst == VOS_ERR)
    {
        return VOS_ERR;
    }

    pstAtCmd->ucMode = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    pstAtCmd->sPara  = (VOS_INT16)lVal;

    return VOS_OK;
}


VOS_UINT32 AT_SetLtePwrDissPara(VOS_UINT8 ucIndex)
{
    AT_MTA_LTEPWRDISS_SET_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����1�Ͳ���2�ĳ��Ȳ���Ϊ0*/
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����2���������ΧΪ(-8~63),���Ȳ��ܴ���2*/
    if (gastAtParaList[1].usParaLen > 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stAtCmd, sizeof(AT_MTA_LTEPWRDISS_SET_REQ_STRU), 0, sizeof(AT_MTA_LTEPWRDISS_SET_REQ_STRU));

    ulRst = AT_LtePwrDissParaCheck(&stAtCmd);

    if (ulRst == VOS_ERR)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*�·���������������Ϣ��C�˴��� */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_LTEPWRDISS_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_LTEPWRDISS_SET_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTEPWRDISS_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_TestLtePwrDissPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR*)pgucAtSndCodeAddr,
                                                    (TAF_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: (0-2),(-8-63)",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    return AT_OK;
}


VOS_UINT32 AT_RcvMtaAtLtePwrDissSetCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg                = VOS_NULL_PTR;
    AT_MTA_LTEPWRDISS_SET_CNF_STRU     *pstMtaAtLtePwrDissSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg                = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaAtLtePwrDissSetCnf = (AT_MTA_LTEPWRDISS_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaAtLtePwrDissSetCnf:WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaAtLtePwrDissSetCnf:AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTEPWRDISS_SET)
    {
        AT_WARN_LOG("AT_RcvMtaAtLtePwrDissSetCnf:Current Option is not AT_CMD_LTEPWRDISS_SET.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstMtaAtLtePwrDissSetCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}

#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)

VOS_UINT32 At_QryCcpuNidDisablePara(VOS_UINT8 ucIndex)
{
    VOS_INT32                           lRet;
    VOS_UINT32                          ulResult = AT_OK;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }


    /* ����DRV�ӿ� */
    lRet = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_GET_CCPUNIDEN,
                             0,
                             VOS_NULL_PTR,
                             0);

     /* С��0��ʾʧ�ܣ�0��ʾ����û��д����1��ʾ������д�� */
    if ((lRet == 0)
     || (lRet == 1))
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         "%s: %d",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                         lRet);
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;

}


VOS_UINT32 At_QryAcpuNidDisablePara(VOS_UINT8 ucIndex)
{
    VOS_INT32                           lRet;
    VOS_UINT32                          ulResult = AT_OK;

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_ERROR;
    }

    /* ����DRV�ӿ� */
    lRet = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_GET_ACPUNIDEN,
                             0,
                             VOS_NULL_PTR,
                             0);

     /* С��0��ʾʧ�ܣ�0��ʾ����û��д����1��ʾ������д�� */
    if ((lRet == 0)
     || (lRet == 1))
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         "%s: %d",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                         lRet);
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaLowPwrModeSetCnf(VOS_VOID *pMsg)
{
    /* ��ʼ�� */
    AT_MTA_MSG_STRU                    *pRcvMsg     = VOS_NULL_PTR;
    MTA_AT_LOW_PWR_MODE_CNF_STRU       *pstSetCnf   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    pRcvMsg     = (AT_MTA_MSG_STRU *)pMsg;
    ulResult    = AT_OK;
    pstSetCnf   = (MTA_AT_LOW_PWR_MODE_CNF_STRU *)pRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaLowPwrModeSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLowPwrModeSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /*�жϵ�ǰ���������Ƿ�ΪAT_CMD_SFEATURE_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LOWPWRMODE_SET)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;
    if (pstSetCnf->ulResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ConvertMtaResult(pstSetCnf->ulResult);
    }
    else
    {
        ulResult = AT_OK;
    }


    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_RcvMtaNrrcCapCfgCnf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg        = VOS_NULL_PTR;
    MTA_AT_NRRCCAP_CFG_SET_CNF_STRU    *pstNrrcCapCfgCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstNrrcCapCfgCnf    = (MTA_AT_NRRCCAP_CFG_SET_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;
    ulResult            = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNrrcCapCfgCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNrrcCapCfgCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�AT^NRRCCAPCFG����� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NRRCCAPCFG_SET)
    {
        AT_WARN_LOG("AT_RcvMtaNrrcCapCfgCnf : Current Option is not AT_CMD_NRRCCAPCFG_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ConvertMtaResult(pstNrrcCapCfgCnf->enResult);

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaNrrcCapQryCnf(
    VOS_VOID                           *pstMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg        = VOS_NULL_PTR;
    MTA_AT_NRRCCAP_QRY_CNF_STRU        *pstNrrcCapQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstNrrcCapQryCnf    = (MTA_AT_NRRCCAP_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;
    ulResult            = AT_ERROR;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNrrcCapQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNrrcCapQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�^ERRCCAPQRY����� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NRRCCAPQRY_SET)
    {
        AT_WARN_LOG("AT_RcvMtaNrrcCapQryCnf : Current Option is not AT_CMD_NRRCCAPQRY_SET.");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstNrrcCapQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        /* ^NRRCCAPQRY:  */
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                     "^NRRCCAPQRY: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
                                                     pstNrrcCapQryCnf->ulNrrcCfgNetMode,
                                                     pstNrrcCapQryCnf->aulPara[0],
                                                     pstNrrcCapQryCnf->aulPara[1],
                                                     pstNrrcCapQryCnf->aulPara[2],
                                                     pstNrrcCapQryCnf->aulPara[3],
                                                     pstNrrcCapQryCnf->aulPara[4],
                                                     pstNrrcCapQryCnf->aulPara[5],
                                                     pstNrrcCapQryCnf->aulPara[6],
                                                     pstNrrcCapQryCnf->aulPara[7],
                                                     pstNrrcCapQryCnf->aulPara[8],
                                                     pstNrrcCapQryCnf->aulPara[9]);

        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ConvertMtaResult(pstNrrcCapQryCnf->enResult);
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMtaNrPwrCtrlSetCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg                = VOS_NULL_PTR;
    MTA_AT_NRPWRCTRL_SET_CNF_STRU      *pstMtaAtNrPwrCtrlSetCnf  = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    pstRcvMsg                = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaAtNrPwrCtrlSetCnf  = (MTA_AT_NRPWRCTRL_SET_CNF_STRU *)pstRcvMsg->aucContent;
    ulResult                 = AT_ERROR;
    ucIndex                  = AT_CLIENT_ID_BUTT;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMtaNrPwrCtrlSetCnf:WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaNrPwrCtrlSetCnf:AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NRPWRCTRL_SET)
    {
        AT_WARN_LOG("AT_RcvMtaNrPwrCtrlSetCnf:Current Option is not AT_CMD_NRPWRCTRL_SET.");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ConvertMtaResult(pstMtaAtNrPwrCtrlSetCnf->enResult);

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif



