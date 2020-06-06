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

#include "PppInterface.h"
#include "AtUsbComItf.h"
#include "AtInputProc.h"
#include "AtCsdInterface.h"
#include "AtTafAgentInterface.h"
#include "TafAgentInterface.h"
#include "AtCmdMsgProc.h"
#include "AtDataProc.h"
#include "ImmInterface.h"
#include "mdrv.h"
#include "AtMntn.h"
#include "AcpuReset.h"

#include "AtInternalMsg.h"
#include "securec.h"


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_INPUTPROC_C

/*****************************************************************************
   2 ȫ�ֱ�������
*****************************************************************************/
VOS_UINT32                              g_ulAtUsbDebugFlag = VOS_FALSE;

extern VOS_UINT32 CBTCPM_NotifyChangePort(AT_PHY_PORT_ENUM_UINT32 enPhyPort);

/* AT/DIAGͨ������·���� */
VOS_UINT8                               gucOmDiagIndex    = AT_MAX_CLIENT_NUM;

/* USB NCM��UDI��� */
UDI_HANDLE                              g_ulAtUdiNdisHdl  = UDI_INVALID_HANDLE;

/* �ñ���Ŀǰֻ����USB-MODEM, HSIC-MODEM��HS-UART�ľ����������Ҫ�����Ҫ��PL���� */
UDI_HANDLE                              g_alAtUdiHandle[AT_CLIENT_BUTT] = {UDI_INVALID_HANDLE};

/* AT֡�ṹ��DRV ֵ֮��Ķ�Ӧ��ϵ */
AT_UART_FORMAT_PARAM_STRU               g_astAtUartFormatTab[] =
{
    /* auto detect (not support) */

    /* 8 data 2 stop */
    {AT_UART_FORMAT_8DATA_2STOP,            AT_UART_DATA_LEN_8_BIT,
     AT_UART_STOP_LEN_2_BIT,                AT_UART_PARITY_LEN_0_BIT},

    /* 8 data 1 parity 1 stop*/
    {AT_UART_FORMAT_8DATA_1PARITY_1STOP,    AT_UART_DATA_LEN_8_BIT,
     AT_UART_STOP_LEN_1_BIT,                AT_UART_PARITY_LEN_1_BIT},

    /* 8 data 1 stop */
    {AT_UART_FORMAT_8DATA_1STOP,            AT_UART_DATA_LEN_8_BIT,
     AT_UART_STOP_LEN_1_BIT,                AT_UART_PARITY_LEN_0_BIT},

    /* 7 data 2 stop */
    {AT_UART_FORMAT_7DATA_2STOP,            AT_UART_DATA_LEN_7_BIT,
     AT_UART_STOP_LEN_2_BIT,                AT_UART_PARITY_LEN_0_BIT},

    /* 7 data 1 parity 1 stop */
    {AT_UART_FORMAT_7DATA_1PARITY_1STOP,    AT_UART_DATA_LEN_7_BIT,
     AT_UART_STOP_LEN_1_BIT,                AT_UART_PARITY_LEN_1_BIT},

    /* 7 data 1 stop */
    {AT_UART_FORMAT_7DATA_1STOP,            AT_UART_DATA_LEN_7_BIT,
     AT_UART_STOP_LEN_1_BIT,                AT_UART_PARITY_LEN_0_BIT}
};


/*****************************************************************************
   3 ��������������
*****************************************************************************/

/*****************************************************************************
   4 ����ʵ��
*****************************************************************************/


VOS_VOID AT_GetAtMsgStruMsgLength(
    VOS_UINT32                          ulInfoLength,
    VOS_UINT32                         *pulMsgLength
)
{
    if (ulInfoLength > 4)
    {
        *pulMsgLength = (sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH)
                      + (ulInfoLength - 4);
    }
    else
    {
        *pulMsgLength = sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH;
    }

    return;
}


VOS_VOID AT_GetUserTypeFromIndex(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucUserType
)
{
    if (ucIndex < AT_MAX_CLIENT_NUM)
    {
        *pucUserType    = gastAtClientTab[ucIndex].UserType;
    }
    else
    {
        *pucUserType    = AT_BUTT_USER;
    }

    return;
}


VOS_VOID AT_VcomCmdStreamEcho(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    /* E5��̬������� */
    /* AGPSͨ��������� */
    if ( (*pucSystemAppConfig != SYSTEM_APP_WEBUI)
#if (FEATURE_VCOM_EXT == FEATURE_ON)
      && (ucIndex != AT_CLIENT_TAB_APP9_INDEX)
      && (ucIndex != AT_CLIENT_TAB_APP12_INDEX)
      && (ucIndex != AT_CLIENT_TAB_APP24_INDEX)
#endif
    )
    {
        AT_MNTN_IncSndStreamCnt(ucIndex);
        APP_VCOM_SEND(gastAtClientTab[ucIndex].ucPortNo, pData, usLen);
    }

    return;
}


VOS_VOID AT_SetAts3Value(VOS_UINT8 ucValue)
{
    if (g_ulAtUsbDebugFlag == VOS_TRUE)
    {
        ucAtS3 = ucValue;
    }

    return;
}


VOS_VOID AT_CmdStreamEcho(
    VOS_UINT8                           ucIndex,
    VOS_UINT8*                          pData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT16                          usEchoLen;

    /* �ж�pData�����Ľ�β�Ƿ�Ϊ<CR><LF>��ʽ��������2Ϊ�س����������ַ����� */
    if ((usLen > 2) && (ucAtS3 == pData[usLen - 2]) && (ucAtS4 == pData[usLen - 1]))
    {
        /* ɾȥ��β��<LF>�ַ� */
        usEchoLen = usLen - 1;
    }
    else
    {
        usEchoLen = usLen;
    }

    if(gastAtClientTab[ucIndex].UserType == AT_USBCOM_USER)
    {
        /*��USB COM�ڷ�������*/
        AT_MNTN_IncSndStreamCnt(ucIndex);
        DMS_COM_SEND(AT_USB_COM_PORT_NO, pData, usEchoLen);
        AT_MNTN_TraceCmdResult(ucIndex, pData, usEchoLen);
    }
    else if (gastAtClientTab[ucIndex].UserType == AT_CTR_USER)
    {
        AT_MNTN_IncSndStreamCnt(ucIndex);
        DMS_COM_SEND(AT_CTR_PORT_NO, pData, usEchoLen);
        AT_MNTN_TraceCmdResult(ucIndex, pData, usEchoLen);
    }
    else if(gastAtClientTab[ucIndex].UserType == AT_PCUI2_USER)
    {
        /*��PCUI2�ڷ�������*/
        DMS_COM_SEND(AT_PCUI2_PORT_NO, pData, usEchoLen);
        AT_MNTN_TraceCmdResult(ucIndex, pData, usEchoLen);
    }
    else if (gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
    {
        AT_SendDataToModem(ucIndex, pData, usEchoLen);
    }
    else if (gastAtClientTab[ucIndex].UserType == AT_APP_USER)
    {
        /* VCOM AT�ڵĻ��Դ��� */
        AT_VcomCmdStreamEcho(ucIndex, pData, usEchoLen);
    }
    else if (gastAtClientTab[ucIndex].UserType == AT_SOCK_USER)
    {
        if ( mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT )
        {
            mdrv_CPM_ComSend(CPM_AT_COMM, pData, VOS_NULL_PTR, usEchoLen);
        }
    }
    else if (gastAtClientTab[ucIndex].UserType == AT_NDIS_USER)
    {
        /* NDIS AT��������� */
        AT_WARN_LOG("AT_CmdStreamEcho:WARNING: NDIS AT");
    }
    else
    {
        AT_LOG1("AT_CmdStreamEcho:WARNING: Abnormal UserType,ucIndex:",ucIndex);
    }

    return;
}


VOS_UINT32 At_CmdStreamPreProc(VOS_UINT8 ucIndex, VOS_UINT8* pData, VOS_UINT16 usLen)
{
    VOS_UINT8                          *pHead = TAF_NULL_PTR;
    errno_t                             lMemResult;
    VOS_UINT16                          usCount = 0;
    VOS_UINT16                          usTotal = 0;

    pHead = pData;

    if (g_ulAtUsbDebugFlag == VOS_TRUE)
    {
        PS_PRINTF_INFO("<At_CmdStreamPreProc> AtEType = %d, UserType = %d, ucAtS3 = %d\n",
                   gucAtEType, gastAtClientTab[ucIndex].UserType, ucAtS3);
    }

    /* ����ͨ������ */
    if( gucAtEType == AT_E_ECHO_CMD )
    {
        AT_CmdStreamEcho(ucIndex, pData, usLen);
    }

    /* MACϵͳ�ϵ�MP��̨����:AT+CMGS=**<CR><^z><Z>(��AT+CMGW=**<CR><^z><Z>)
       Ϊ�˹�ܸ����⣬��Ҫ�ڽ��յ�������ʽ��������
       ��Ҫ����������Ч�ַ�<^z><Z>ɾȥ */
    AT_DiscardInvalidCharForSms(pData, &usLen);

    /* �����������ַ��Ž���������Ϣ��ʽ���͵�AT����Ϣ������: <CR>/<ctrl-z>/<ESC> */
    while(usCount++ < usLen)
    {
        if (At_CheckSplitChar((*((pData + usCount) - 1))))
        {
            if(g_aucAtDataBuff[ucIndex].ulBuffLen > 0)
            {
                if((g_aucAtDataBuff[ucIndex].ulBuffLen + usCount) >= AT_COM_BUFF_LEN)
                {
                    g_aucAtDataBuff[ucIndex].ulBuffLen = 0;
                    AT_WARN_LOG("At_CmdStreamPreProc: buff is full at split char!");
                    return AT_FAILURE;
                }
                lMemResult = memcpy_s(&g_aucAtDataBuff[ucIndex].aucDataBuff[g_aucAtDataBuff[ucIndex].ulBuffLen],
                                      AT_COM_BUFF_LEN - g_aucAtDataBuff[ucIndex].ulBuffLen,
                                      pHead,
                                      usCount);
                TAF_MEM_CHK_RTN_VAL(lMemResult, AT_COM_BUFF_LEN - g_aucAtDataBuff[ucIndex].ulBuffLen, usCount);
                At_SendCmdMsg(ucIndex,g_aucAtDataBuff[ucIndex].aucDataBuff, (TAF_UINT16)(g_aucAtDataBuff[ucIndex].ulBuffLen + usCount), AT_NORMAL_TYPE_MSG);
                pHead   = pData + usCount;
                usTotal = usCount;
                g_aucAtDataBuff[ucIndex].ulBuffLen = 0;
            }
            else
            {
                At_SendCmdMsg(ucIndex, pHead, usCount - usTotal, AT_NORMAL_TYPE_MSG);
                pHead   = pData + usCount;
                usTotal = usCount;
            }
        }
    }

    if(usTotal < usLen)
    {
        if((g_aucAtDataBuff[ucIndex].ulBuffLen + (usLen - usTotal)) >= AT_COM_BUFF_LEN)
        {
            g_aucAtDataBuff[ucIndex].ulBuffLen = 0;
            AT_WARN_LOG("At_CmdStreamPreProc: buff is full after split char!");
            return AT_FAILURE;
        }
        lMemResult = memcpy_s(&g_aucAtDataBuff[ucIndex].aucDataBuff[g_aucAtDataBuff[ucIndex].ulBuffLen],
                              AT_COM_BUFF_LEN - g_aucAtDataBuff[ucIndex].ulBuffLen,
                              pHead,
                              (TAF_UINT32)(usLen - usTotal));
        TAF_MEM_CHK_RTN_VAL(lMemResult, AT_COM_BUFF_LEN - g_aucAtDataBuff[ucIndex].ulBuffLen, (TAF_UINT32)(usLen - usTotal));
        g_aucAtDataBuff[ucIndex].ulBuffLen += (VOS_UINT16)((pData - pHead) + usLen);
    }

    return AT_SUCCESS;
}


VOS_VOID AT_StopFlowCtrl(VOS_UINT8 ucIndex)
{
    switch (gastAtClientTab[ucIndex].UserType)
    {
        case AT_MODEM_USER:
            AT_MNTN_TraceStopFlowCtrl(ucIndex, AT_FC_DEVICE_TYPE_MODEM);
            AT_CtrlCTS(ucIndex, AT_IO_LEVEL_HIGH);
            break;

        case AT_HSUART_USER:
            AT_MNTN_TraceStopFlowCtrl(ucIndex, AT_FC_DEVICE_TYPE_HSUART);
            AT_CtrlCTS(ucIndex, AT_IO_LEVEL_HIGH);
            break;

        default:
            break;
    }

    return;
}


LOCAL VOS_UINT32 At_Om2AtSwitch(
    VOS_UINT8                          *pData,
    VOS_UINT16                          usLen
)
{
    if ((usLen == 3)
     && (pData[0] == '+')
     && (pData[1] == '+')
     && (pData[2] == '+'))
    {
        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_UINT32 At_OmDataProc (
    VOS_UINT8                           ucPortNo,
    VOS_UINT8                          *pData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT32                          ulRst;

    /*OMֻ����UART PCUI CTRL�ڵ�����*/
    switch(ucPortNo)
    {
        case AT_UART_PORT_NO:
            if (g_apAtPortDataRcvFuncTab[AT_UART_PORT] == VOS_NULL_PTR)
            {
                AT_ERR_LOG("At_OmDataProc: Uart port proc func is NULL!");
                return VOS_ERR;
            }

            ulRst = g_apAtPortDataRcvFuncTab[AT_UART_PORT](pData, (VOS_UINT32)usLen);
            break;

        case AT_USB_COM_PORT_NO:
            /* ����+++�������󣬽�OM���л���AT�� */
            if (At_Om2AtSwitch(pData, usLen) == VOS_OK)
            {
               ulRst = AT_ResumeOm2AtMode();
               return ulRst;
            }

            if (g_apAtPortDataRcvFuncTab[AT_PCUI_PORT] == VOS_NULL_PTR)
            {
                AT_ERR_LOG("At_OmDataProc: PCUI proc func is NULL!");
                return VOS_ERR;
            }

            ulRst = g_apAtPortDataRcvFuncTab[AT_PCUI_PORT](pData, (VOS_UINT32)usLen);
            break;

        case AT_CTR_PORT_NO:
            if (g_apAtPortDataRcvFuncTab[AT_CTRL_PORT] == VOS_NULL_PTR)
            {
                AT_ERR_LOG("At_OmDataProc: CTRL proc func is NULL!");
                return VOS_ERR;
            }

            ulRst = g_apAtPortDataRcvFuncTab[AT_CTRL_PORT](pData, (VOS_UINT32)usLen);
            break;

        case AT_PCUI2_PORT_NO:
            if (g_apAtPortDataRcvFuncTab[AT_PCUI2_PORT] == VOS_NULL_PTR)
            {
                AT_ERR_LOG("At_OmDataProc: PCUI2 proc func is NULL!");
                return VOS_ERR;
            }

            ulRst = g_apAtPortDataRcvFuncTab[AT_PCUI2_PORT](pData, (VOS_UINT32)usLen);
            break;

        case AT_HSUART_PORT_NO:
            if (g_apAtPortDataRcvFuncTab[AT_HSUART_PORT] == VOS_NULL_PTR)
            {
                AT_ERR_LOG("At_OmDataProc: HSUART proc func is NULL!");
                return VOS_ERR;
            }

            ulRst = g_apAtPortDataRcvFuncTab[AT_HSUART_PORT](pData, (VOS_UINT32)usLen);
            break;

        default:
            AT_WARN_LOG("At_OmDataProc: don't proc data of this port!");
            return VOS_ERR;
    }

    return ulRst;
}


TAF_UINT32 At_DataStreamPreProc (TAF_UINT8 ucIndex,TAF_UINT8 DataMode,TAF_UINT8* pData, TAF_UINT16 usLen)
{

    AT_LOG1("At_DataStreamPreProc ucIndex:",ucIndex);
    AT_LOG1("At_DataStreamPreProc usLen:",usLen);
    AT_LOG1("At_DataStreamPreProc DataMode:",DataMode);

    switch(DataMode)    /* ��ǰ�û����������� */
    {
        case AT_CSD_DATA_MODE:
            break;

        /*����OM�ṩ�ĸ��˿ڽ������ݺ����������ݣ�����Ҫ����������ģʽ*/
        case AT_DIAG_DATA_MODE:
        case AT_OM_DATA_MODE:
            At_OmDataProc(gastAtClientTab[ucIndex].ucPortNo, pData,usLen);
            break;

        default:
            AT_WARN_LOG("At_DataStreamPreProc DataMode Wrong!");
            break;
    }
    return AT_SUCCESS;
}


VOS_UINT32  AT_CsdDataModeRcvModemMsc(
    VOS_UINT8                           ucIndex
)
{
    TAFAGENT_CALL_INFO_PARAM_STRU       astCallInfos[MN_CALL_MAX_NUM];
    TAF_CTRL_STRU                       stCtrl;
    TAF_CCM_CALL_END_INFO_STRU          stEndInfo;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          i;
    VOS_UINT8                           ucNumOfCalls;
    VOS_UINT32                          ulRlst;

    memset_s(astCallInfos, sizeof(astCallInfos), 0x00, sizeof(astCallInfos));
    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stEndInfo, sizeof(stEndInfo), 0x00, sizeof(stEndInfo));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }


    /* ��ѯ��ǰ�ĺ���״̬��Ϣ�������VIDEO���͵ĺ��У���Ҷϸú��У�Ŀǰ���ڲ�����ڶ��VIDEO���У�
       ����ҵ�һ��VIDEO���͵ĺ���ִ����Ϻ󣬼����˳� */

    ulRlst = TAF_AGENT_GetCallInfoReq(ucIndex, &ucNumOfCalls, astCallInfos);

    if(ulRlst == VOS_OK)
    {
        for (i = 0; i < ucNumOfCalls; i++)
        {
            if (astCallInfos[i].enCallType == MN_CALL_TYPE_VIDEO)
            {
                /* δ�ҶϹ��绰 */
                if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_END_SET)
                {
                    TAF_LOG1(WUEPS_PID_AT, 0, PS_LOG_LEVEL_INFO, "At_SetHPara: ulNumOfCalls is ",(TAF_INT32)ucNumOfCalls);

                    stEndInfo.callId = astCallInfos[i].callId;
                    /* AT��CCM���ͺ��н�����Ϣ */
                    if (TAF_CCM_CallCommonReq(&stCtrl,
                                                        &stEndInfo,
                                                        ID_TAF_CCM_CALL_END_REQ,
                                                        sizeof(stEndInfo),
                                                        enModemId) == VOS_OK)
                    {
                        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_END_SET;

                        return AT_SUCCESS;
                    }
                    else
                    {
                        return AT_ERROR;
                    }
                }
                else
                {
                    /* ֮ǰ�������Ҷϵ绰���������յ�����DTR�ܽ��ź���Ϣ���������� */
                    return AT_SUCCESS;
                }
            }
        }
    }


    return AT_CME_UNKNOWN;
}


VOS_UINT32  AT_PppDataModeRcvModemMsc(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                     *pMscStru
)
{
    /* 1.�ж�(AT_CMD_PS_DATA_CALL_END_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
         ��Ŀ��:�������Ͽ�����֮ǰ�Ѿ�ִ����PDP DEACTIVE���̣���ʱ��������DTR����
         ������ִ�и��쳣����
        2.�÷�֧�Ĵ�����:����������״̬��,�û�����PPP�Ͽ�����ʱPPP�����޷�������
          ֻ�����������DTR�źŵ�ʱ��ִ��PPP�Ͽ�����
    */
    if (pMscStru->OP_Dtr && (pMscStru->ucDtr == 0))
    {
        if ( (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_PS_DATA_CALL_END_SET)
          || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_WAIT_PPP_PROTOCOL_REL_SET) )
        {
            return AT_SUCCESS;
        }

        /* ����������״̬�£���PPP�Ͽ���Э�̱������޷�����UE��ģ�
           UEֻ����DTR�ź����͵�ʱ��,ִ��PPP���ŶϿ�����*/
        if ((gastAtClientTab[ucIndex].ModemStatus & IO_CTRL_CTS) == 0)
        {
            AT_StopFlowCtrl((TAF_UINT8)ucIndex);
        }

        /*��PPP�����ͷ�PPP����*/
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);

        /*��PPP����HDLCȥʹ�ܲ���*/
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        /* ֹͣModem�ڵ�AT��ʱ���Լ�AT��·�ĵ�ǰ����ָʾ */
        AT_STOP_TIMER_CMD_READY(ucIndex);

        /*EVENT - RCV Down DTR to Disconnect PPP in Abnormal procedure(PDP type:IP) ;index*/
        AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_DOWN_DTR_RELEASE_PPP_IP_TYPE,
                        &ucIndex, sizeof(ucIndex));

        if ( TAF_PS_CallEnd(WUEPS_PID_AT,
                                      AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                      0,
                                      gastAtClientTab[ucIndex].ucCid) == VOS_OK )
        {
            /* ����ʱ�� */
            if (At_StartTimer(AT_SET_PARA_TIME, ucIndex) != AT_SUCCESS)
            {
                AT_ERR_LOG("At_UsbModemStatusPreProc:ERROR:Start Timer");
                return AT_FAILURE;
            }

            /* ���õ�ǰ�������� */
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
        }
        else
        {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32  AT_IpDataModeRcvModemMsc(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                     *pMscStru
)
{
    if (pMscStru->OP_Dtr && (pMscStru->ucDtr == 0))
    {
        /*��ԭ�ȿ��������أ�����ֹͣ����*/
        if ((gastAtClientTab[ucIndex].ModemStatus & IO_CTRL_CTS) == 0)
        {
            AT_StopFlowCtrl((TAF_UINT8)ucIndex);
        }

        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

        /*��PPP����HDLCȥʹ�ܲ���*/
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        /* ֹͣModem�ڵ�AT��ʱ���Լ�AT��·�ĵ�ǰ����ָʾ */
        AT_STOP_TIMER_CMD_READY(ucIndex);;

        /*EVENT - RCV Down DTR to Disconnect PPP in Abnormal procedure(PDP type:PPP) ;index*/
        AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_DOWN_DTR_RELEASE_PPP_PPP_TYPE,
                        &ucIndex, sizeof(ucIndex));

        if ( TAF_PS_CallEnd(WUEPS_PID_AT,
                                      AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                      0,
                                      gastAtClientTab[ucIndex].ucCid) == VOS_OK )
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
            return AT_SUCCESS;
        }
        else
        {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}


VOS_VOID AT_MODEM_ProcDtrChange(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstDceMsc
)
{
    if (pstDceMsc->ucDtr == 1)
    {
        /*����DSR��CTS�ź�*/
        AT_CtrlDSR(ucIndex, AT_IO_LEVEL_HIGH);
        AT_StopFlowCtrl(ucIndex);
    }
    else
    {
        /* �ο�Qʵ�֣�DSR�ź����ϵ��һֱ��������״̬����ʹ�յ�DTRҲ������DSR��
           ͬʱ��PC������������һ�㲻������DTR�źţ����쳣�����лὫ֮���ͣ�
           ����UE���յ�DTR���� ��ʱ����Ҫ��DCD���� */
        if ( (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
          && (gastAtClientTab[ucIndex].DataMode == AT_CSD_DATA_MODE) )
        {
            g_ucDtrDownFlag = VOS_TRUE;
        }

        AT_CtrlDCD(ucIndex, AT_IO_LEVEL_LOW);
    }

}


VOS_UINT32 AT_MODEM_WriteMscCmd(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstDceMsc
)
{
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_MODEM_WriteMscCmd: Invalid UDI handle!");
        return AT_FAILURE;
    }

    /* д�ܽ��ź� */
    lResult = mdrv_udi_ioctl(lUdiHandle, ACM_MODEM_IOCTL_MSC_WRITE_CMD, pstDceMsc);
    if (lResult != VOS_OK)
    {
        AT_ERR_LOG("AT_MODEM_WriteMscCmd: Write failed!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_MODEM_StartFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT8                           ucIndex;

    for (ucIndex = 0; ucIndex < AT_MAX_CLIENT_NUM; ucIndex++)
    {
        if ( (gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
          && (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE) )
        {
            if ( (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
              || (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
              || (gastAtClientTab[ucIndex].DataMode == AT_CSD_DATA_MODE) )
            {
                AT_MNTN_TraceStartFlowCtrl(ucIndex, AT_FC_DEVICE_TYPE_MODEM);
                AT_CtrlCTS(ucIndex, AT_IO_LEVEL_LOW);
            }
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_MODEM_StopFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT8                           ucIndex;

    for(ucIndex = 0; ucIndex < AT_MAX_CLIENT_NUM; ucIndex++)
    {
        if ( (gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
          && (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE) )
        {
            if ( (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
                || (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
                || (gastAtClientTab[ucIndex].DataMode == AT_CSD_DATA_MODE) )
            {
                AT_MNTN_TraceStopFlowCtrl(ucIndex, AT_FC_DEVICE_TYPE_MODEM);
                AT_CtrlCTS(ucIndex, AT_IO_LEVEL_HIGH);
            }
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_ModemStatusPreProc(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pMscStru
)
{

    NAS_OM_EVENT_ID_ENUM_UINT16         enEventId;

#if (FEATURE_AT_HSUART == FEATURE_ON)
    VOS_UINT32                          ulRet;
    ulRet = VOS_TRUE;
#endif



    if (pMscStru == VOS_NULL_PTR)
    {
        return AT_FAILURE;
    }

    if (pMscStru->OP_Dtr)
    {
        enEventId = (0 != pMscStru->ucDtr) ?
                    NAS_OM_EVENT_DTE_UP_DTR : NAS_OM_EVENT_DTE_DOWN_DTR;

        AT_EventReport(WUEPS_PID_AT, enEventId, &ucIndex, sizeof(VOS_UINT8));

        if (AT_CheckModemUser(ucIndex) == VOS_TRUE)
        {
            AT_MODEM_ProcDtrChange(ucIndex, pMscStru);
        }

#if (FEATURE_AT_HSUART == FEATURE_ON)
        if (AT_CheckHsUartUser(ucIndex) == VOS_TRUE)
        {
            ulRet = AT_HSUART_ProcDtrChange(ucIndex, pMscStru);
            if (ulRet == VOS_FALSE)
            {
                return AT_SUCCESS;
            }
        }
#endif
    }

    /* ����ģʽ��ӦMSC���� */
    if ( (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
      || (gastAtClientTab[ucIndex].Mode == AT_ONLINE_CMD_MODE) )
    {
        switch (gastAtClientTab[ucIndex].DataMode)
        {
        case AT_CSD_DATA_MODE:
            if ((pMscStru->OP_Dtr) && (pMscStru->ucDtr == 0))
            {
                AT_CsdDataModeRcvModemMsc(ucIndex);
            }
            return AT_SUCCESS;

        case AT_PPP_DATA_MODE:
            return AT_PppDataModeRcvModemMsc(ucIndex, pMscStru);

        case AT_IP_DATA_MODE:
            return AT_IpDataModeRcvModemMsc(ucIndex, pMscStru);

        default:
            AT_WARN_LOG("At_UsbModemStatusPreProc: DataMode Wrong!");
            break;
        }
    }
    else
    {
         /* �п����ڻ�û������ʱ�����͹ܽ��źţ���ʱ����������״̬��
            Ŀǰ��˵ֻ��CSDģʽ�»������������ */
         if ((pMscStru->OP_Dtr) && (pMscStru->ucDtr == 0))
         {
             AT_CsdDataModeRcvModemMsc(ucIndex);
         }
    }

    return AT_SUCCESS;
}


VOS_VOID AT_ModemSetCtlStatus(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pMscStru
)
{
    if (pMscStru == TAF_NULL_PTR)
    {
        return;
    }

    /*����dsr�ź�*/
    if ( pMscStru->OP_Dsr )
    {
        if ( pMscStru->ucDsr == 1 )
        {
            gastAtClientTab[ucIndex].ModemStatus |= IO_CTRL_DSR;
        }
        else
        {
            gastAtClientTab[ucIndex].ModemStatus &= ~IO_CTRL_DSR;
        }
    }

    /*����CTS�ź�*/
    if ( pMscStru->OP_Cts )
    {
        if ( pMscStru->ucCts == 1 )
        {
            gastAtClientTab[ucIndex].ModemStatus |= IO_CTRL_CTS;
        }
        else
        {
            gastAtClientTab[ucIndex].ModemStatus &= ~IO_CTRL_CTS;
        }
    }

    /*����RI�ź�*/
    if ( pMscStru->OP_Ri )
    {
        if ( pMscStru->ucRi == 1 )
        {
            gastAtClientTab[ucIndex].ModemStatus |= IO_CTRL_RI;
        }
        else
        {
            gastAtClientTab[ucIndex].ModemStatus &= ~IO_CTRL_RI;
        }
    }

    /*����DCD�ź�*/
    if ( pMscStru->OP_Dcd )
    {
        if ( pMscStru->ucDcd == 1 )
        {
            gastAtClientTab[ucIndex].ModemStatus |= IO_CTRL_DCD;
        }
        else
        {
            gastAtClientTab[ucIndex].ModemStatus &= ~IO_CTRL_DCD;
        }
    }

    /*����FC�ź�*/
    if ( pMscStru->OP_Fc )
    {
        if ( pMscStru->ucFc == 1 )
        {
            gastAtClientTab[ucIndex].ModemStatus |= IO_CTRL_FC;
        }
        else
        {
            gastAtClientTab[ucIndex].ModemStatus &= ~IO_CTRL_FC;
        }
    }

}


VOS_UINT32 AT_SetModemStatus(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstMsc
)
{
    VOS_UINT32                          ulResult;

    if (pstMsc == VOS_NULL_PTR)
    {
        return AT_FAILURE;
    }

    if (ucIndex >= AT_CLIENT_BUTT)
    {
        return AT_FAILURE;
    }

    /* ���±��عܽ��ź�*/
    AT_ModemSetCtlStatus(ucIndex, pstMsc);

    /* ����ܽ��źſ�ά�ɲ� */
    AT_MNTN_TraceOutputMsc(ucIndex, pstMsc);

    /* д��ܽ��źŲ��� */
    switch (gastAtClientTab[ucIndex].UserType)
    {
        case AT_MODEM_USER:
            ulResult = AT_MODEM_WriteMscCmd(ucIndex, pstMsc);
            break;

#if (FEATURE_AT_HSUART == FEATURE_ON)
        case AT_HSUART_USER:
            ulResult = AT_HSUART_WriteMscCmd(ucIndex, pstMsc);
            break;
#endif

        default:
            ulResult = AT_SUCCESS;
            break;
    }

    return ulResult;
}

/* ɾ��At_SetModemStatusForFC����, ���ܺ�At_SetModemStatus�ظ� */


VOS_UINT32 At_ModemEst (
    VOS_UINT8                           ucIndex,
    AT_CLIENT_ID_ENUM_UINT16            usClientId,
    VOS_UINT8                           ucPortNo
)
{

    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    /* ��д�û����� */
    gastAtClientTab[ucIndex].usClientId      = usClientId;
    gastAtClientTab[ucIndex].ucPortType      = ucPortNo;
    gastAtClientTab[ucIndex].ucDlci          = AT_MODEM_USER_DLCI;
    gastAtClientTab[ucIndex].ucPortNo        = ucPortNo;
    gastAtClientTab[ucIndex].UserType        = AT_MODEM_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;

    /* ���¿��Բ�����д��ǰ��PS_MEMSET�Ѿ���ʼ����ֻΪ�ɿ���� */
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    AT_LOG1("At_ModemEst ucIndex:",ucIndex);

    return AT_SUCCESS;
}


VOS_UINT32 At_ModemMscInd (
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucDlci,
    AT_DCE_MSC_STRU                    *pMscStru
)
{
    AT_PPP_MODEM_MSC_IND_MSG_STRU      *pMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    errno_t                             lMemResult;
    VOS_UINT_PTR                        ulTmpAddr;

    ulLength = (sizeof(AT_PPP_MODEM_MSC_IND_MSG_STRU) - VOS_MSG_HEAD_LENGTH)
               + (sizeof(AT_DCE_MSC_STRU) - 4);
    /*lint -save -e830*/
    pMsg = ( AT_PPP_MODEM_MSC_IND_MSG_STRU * )PS_ALLOC_MSG( PS_PID_APP_PPP, ulLength );
    /*lint -restore */
    if ( pMsg == VOS_NULL_PTR )
    {
        /*��ӡ������Ϣ---������Ϣ��ʧ��:*/
        AT_WARN_LOG("At_ModemMscInd: Alloc AT_PPP_MODEM_MSC_IND_MSG_STRU msg fail!");
        return AT_FAILURE;
    }

    /*��д��Ϣͷ:*/
    pMsg->MsgHeader.ulSenderCpuId   = VOS_LOCAL_CPUID;
    pMsg->MsgHeader.ulSenderPid     = PS_PID_APP_PPP;
    pMsg->MsgHeader.ulReceiverCpuId = VOS_LOCAL_CPUID;
    pMsg->MsgHeader.ulReceiverPid   = WUEPS_PID_AT;
    pMsg->MsgHeader.ulLength        = ulLength;
    pMsg->MsgHeader.ulMsgName       = AT_PPP_MODEM_MSC_IND_MSG;

    /*��д��Ϣ��*/
    pMsg->ucIndex                   = ucIndex;
    pMsg->ucDlci                    = ucDlci;

    /* ��д�ܽ����� */
    ulTmpAddr = (VOS_UINT_PTR)(pMsg->aucMscInd);

    lMemResult = memcpy_s((VOS_VOID *)ulTmpAddr, sizeof(AT_DCE_MSC_STRU), (VOS_UINT8 *)pMscStru, sizeof(AT_DCE_MSC_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(AT_DCE_MSC_STRU), sizeof(AT_DCE_MSC_STRU));

    /* ������Ϣ */
    if ( PS_SEND_MSG( PS_PID_APP_PPP, pMsg ) != VOS_OK )
    {
        /*��ӡ������Ϣ---������Ϣʧ��:*/
        AT_WARN_LOG( "At_ModemMscInd:WARNING:SEND AT_PPP_MODEM_MSC_IND_MSG_STRU msg FAIL!" );
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_ModemGetUlDataBuf(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                       **ppstBuf
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    VOS_INT32                           ulResult;


    memset_s(&stCtlParam, sizeof(stCtlParam), 0x00, sizeof(stCtlParam));



    /* ��ȡ������������buffer */
    ulResult = mdrv_udi_ioctl(g_alAtUdiHandle[ucIndex], ACM_IOCTL_GET_RD_BUFF, &stCtlParam);

    if ( ulResult != VOS_OK )
    {
        AT_ERR_LOG1("AT_ModemGetUlDataBuf, WARNING, Get modem buffer failed code %d!",
                  ulResult);
        AT_MODEM_DBG_UL_GET_RD_FAIL_NUM(1);
        return AT_FAILURE;
    }

    if (stCtlParam.pVirAddr == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_ModemGetUlDataBuf, WARNING, Data buffer error");
        AT_MODEM_DBG_UL_INVALID_RD_NUM(1);
        return AT_FAILURE;
    }

    AT_MODEM_DBG_UL_GET_RD_SUCC_NUM(1);

    *ppstBuf = (IMM_ZC_STRU *)stCtlParam.pVirAddr;

    return AT_SUCCESS;
}


VOS_UINT32 At_ModemDataInd(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucDlci,
    IMM_ZC_STRU                        *pstData
)
{
    AT_DCE_MSC_STRU                     stMscStru;
    VOS_UINT32                          ulRet;
    /* pDataΪ��������ָ�� */
    VOS_UINT8                          *pData = VOS_NULL_PTR;
    /* usLenΪ�������ݵĳ��� */
    VOS_UINT16                          usLen;

    /* ���index��Dlci�Ƿ���ȷ */
    if (ucIndex != AT_CLIENT_TAB_MODEM_INDEX)
    {
        /*�ͷ��ڴ�*/
        AT_ModemFreeUlDataBuf(ucIndex, pstData);
        return AT_FAILURE;
    }

    /* ��pstData(IMM_ZC_STRU����)��ȡ���������ݺͳ��ȣ��ֱ𱣴���pData��usLen�� */
    pData = pstData->data;
    usLen = (VOS_UINT16)pstData->len;

    if ( gastAtClientTab[ucIndex].Mode == AT_CMD_MODE )
    {
        /*��Modemͨ���Ѿ���������̬������ʱ��Ȼ�յ�PPP֡����ֱ�Ӷ���*/
        if ((usLen > 0) && (pData[0] == 0x7e) && (pData[usLen - 1] == 0x7e))
        {
            /*�ͷ�BSP�ڴ�*/
            AT_ModemFreeUlDataBuf(ucIndex, pstData);
            return AT_SUCCESS;
        }

        ulRet = At_CmdStreamPreProc(ucIndex,pData,usLen);

        /*�ͷ�BSP�ڴ�*/
        AT_ModemFreeUlDataBuf(ucIndex, pstData);
        return ulRet;
    }

    /* ����modem�ڵ�״̬���зַ�*/
    switch ( gastAtClientTab[ucIndex].DataMode )
    {
        case AT_PPP_DATA_MODE:

            /* (AT2D17549)���MAC 10.6.2ϵͳ���ŹҶ�ʧ������.���ձ�˵ķ�ʽ��
               ����������Ϊ"+++"����ģ������DTR�źŵĴ���ʽ
            */
            if (usLen == 3)
            {
                if ((pData[0] == '+') && (pData[1] == '+') && (pData[2] == '+'))
                {
                    /*ģ������DTR�ź�*/
                    memset_s(&stMscStru, (VOS_SIZE_T)sizeof(stMscStru), 0x00, (VOS_SIZE_T)sizeof(stMscStru));
                    stMscStru.OP_Dtr = 1;
                    stMscStru.ucDtr  = 0;
                    At_ModemMscInd(ucIndex, ucDlci, &stMscStru);
                    break;
                }
            }
            /* PPP�����ͷ������ڴ� */
            PPP_PullPacketEvent(gastAtClientTab[ucIndex].usPppId, pstData);
            return AT_SUCCESS;

        case AT_IP_DATA_MODE:
            if (usLen == 3)
            {
                if ((pData[0] == '+') && (pData[1] == '+') && (pData[2] == '+'))
                {
                    /*ģ������DTR�ź�*/
                    memset_s(&stMscStru, (VOS_SIZE_T)sizeof(stMscStru), 0x00, (VOS_SIZE_T)sizeof(stMscStru));
                    stMscStru.OP_Dtr = 1;
                    stMscStru.ucDtr  = 0;
                    At_ModemMscInd(ucIndex, ucDlci, &stMscStru);
                    break;
                }
            }
            /* PPP�����ͷ������ڴ� */
            PPP_PullRawDataEvent(gastAtClientTab[ucIndex].usPppId, pstData);
            return AT_SUCCESS;

        /* Modified by s62952 for AT Project��2011-10-17,  Begin*/
        case AT_CSD_DATA_MODE:
#if(FEATURE_ON == FEATURE_CSD)
            CSD_UL_SendData(pstData);
            return AT_SUCCESS;
#endif
         /* Modified by s62952 for AT Project��2011-10-17,  end*/

        default:
            AT_WARN_LOG("At_ModemDataInd: DataMode Wrong!");
            break;
    }

    /*�ͷ��ڴ�*/
    AT_ModemFreeUlDataBuf(ucIndex, pstData);
    return AT_SUCCESS;
}


VOS_UINT32 AT_ModemInitUlDataBuf(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulEachBuffSize,
    VOS_UINT32                          ulTotalBuffNum
)
{
    ACM_READ_BUFF_INFO                  stReadBuffInfo;
    VOS_INT32                           ulResult;


    /* ��д��Ҫ�ͷŵ��ڴ�ָ�� */
    stReadBuffInfo.u32BuffSize = ulEachBuffSize;
    stReadBuffInfo.u32BuffNum  = ulTotalBuffNum;

    ulResult= mdrv_udi_ioctl(g_alAtUdiHandle[ucIndex], ACM_IOCTL_RELLOC_READ_BUFF, &stReadBuffInfo);

    if ( ulResult != VOS_OK )
    {
        AT_ERR_LOG1("AT_ModemInitUlDataBuf, WARNING, Initialize data buffer failed code %d!\r\n",
                  ulResult);

        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_ModemFreeUlDataBuf(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstBuf
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    VOS_INT32                           ulResult;

    /* ��д��Ҫ�ͷŵ��ڴ�ָ�� */
    stCtlParam.pVirAddr = (VOS_CHAR*)pstBuf;
    stCtlParam.pPhyAddr = VOS_NULL_PTR;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = VOS_NULL_PTR;

    ulResult = mdrv_udi_ioctl(g_alAtUdiHandle[ucIndex], ACM_IOCTL_RETURN_BUFF, &stCtlParam);

    if ( ulResult != VOS_OK )
    {
        AT_ERR_LOG1("AT_ModemFreeUlDataBuf, ERROR, Return modem buffer failed, code %d!\r\n",
                  ulResult);
        AT_MODEM_DBG_UL_RETURN_BUFF_FAIL_NUM(1);
        return AT_FAILURE;
    }

    AT_MODEM_DBG_UL_RETURN_BUFF_SUCC_NUM(1);

    return AT_SUCCESS;
}


VOS_VOID AT_ModemFreeDlDataBuf(
    VOS_CHAR                           *pstBuf
)
{
    AT_MODEM_DBG_DL_FREE_BUFF_NUM(1);

    /* �ͷ�pstBuf */
    IMM_ZcFree((IMM_ZC_STRU*)pstBuf);
    return;
}


VOS_UINT32 AT_ModemWriteData(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstBuf
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    VOS_INT32                           ulResult;

    /* ��д�������ڴ��ַ */
    stCtlParam.pVirAddr                 = (VOS_CHAR*)pstBuf;
    stCtlParam.pPhyAddr                 = VOS_NULL_PTR;
    stCtlParam.u32Size                  = 0;
    stCtlParam.pDrvPriv                 = VOS_NULL_PTR;

    if (g_alAtUdiHandle[ucIndex] == UDI_INVALID_HANDLE)
    {
        AT_ModemFreeDlDataBuf((VOS_CHAR*)pstBuf);
        return AT_FAILURE;
    }

    /* �첽��ʽд����*/
    ulResult = mdrv_udi_ioctl(g_alAtUdiHandle[ucIndex], ACM_IOCTL_WRITE_ASYNC, &stCtlParam);

    if (ulResult != VOS_OK)
    {
        AT_WARN_LOG("AT_ModemWriteData: Write data failed with code!\r\n");
        AT_MODEM_DBG_DL_WRITE_ASYNC_FAIL_NUM(1);
        AT_ModemFreeDlDataBuf((VOS_CHAR*)pstBuf);
        return AT_FAILURE;
    }

    AT_MODEM_DBG_DL_WRITE_ASYNC_SUCC_NUM(1);

    return AT_SUCCESS;
}


VOS_UINT32 AT_SendDataToModem(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucDataBuf,
    VOS_UINT16                          usLen
)
{
    IMM_ZC_STRU                        *pstData = VOS_NULL_PTR;
    VOS_CHAR                           *pstZcPutData = VOS_NULL_PTR;
    errno_t                             lMemResult;

    pstData = VOS_NULL_PTR;

    pstData = IMM_ZcStaticAlloc((VOS_UINT16)usLen);

    if (pstData == VOS_NULL_PTR)
    {
        return AT_FAILURE;
    }

    /*�˲��費���٣�����ƫ������βָ��*/
    pstZcPutData = (VOS_CHAR *)IMM_ZcPut(pstData, usLen);

    lMemResult = memcpy_s(pstZcPutData, usLen, pucDataBuf, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen, usLen);

    /*������д��MODEM�豸��д�ɹ����ڴ��ɵ������ͷ�*/
    if (AT_ModemWriteData(ucIndex, pstData) != AT_SUCCESS)
    {
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_SendZcDataToModem(
    VOS_UINT16                          usPppId,
    IMM_ZC_STRU                        *pstDataBuf
)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    ucIndex = gastAtPppIndexTab[usPppId];

    if ( (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
      || (gastAtClientTab[ucIndex].Mode == AT_ONLINE_CMD_MODE) )
    {
        IMM_ZcFree(pstDataBuf);
        return AT_FAILURE;
    }

    switch (gastAtClientTab[ucIndex].UserType)
    {
        case AT_MODEM_USER:
            ulResult = AT_ModemWriteData(ucIndex, pstDataBuf);
            break;

#if (FEATURE_AT_HSUART == FEATURE_ON)
        case AT_HSUART_USER:
            ulResult = AT_HSUART_WriteDataAsync(ucIndex, pstDataBuf);
            break;
#endif

        default:
            IMM_ZcFree(pstDataBuf);
            ulResult = AT_FAILURE;
            break;
    }

    return ulResult;
}


VOS_UINT32 AT_SendCsdZcDataToModem(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstDataBuf
)
{
    /*������д��MODEM�豸��д�ɹ����ڴ��ɵ������ͷ�*/
    if (AT_ModemWriteData(ucIndex, pstDataBuf) != AT_SUCCESS)
    {
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_VOID AT_UsbModemEnableCB(VOS_UINT32 ulEnable)
{
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_MODEM_INDEX;

    AT_ModemeEnableCB(ucIndex, ulEnable);

    return;
}


VOS_VOID AT_UsbModemReadDataCB( VOS_VOID )
{
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucDlci;
    IMM_ZC_STRU                        *pstBuf = VOS_NULL_PTR;

    pstBuf          = VOS_NULL_PTR;

    /* HSIC MODEM������ */
    ucIndex     = AT_CLIENT_TAB_MODEM_INDEX;

    AT_MODEM_DBG_UL_DATA_READ_CB_NUM(1);

    if (AT_ModemGetUlDataBuf(ucIndex, &pstBuf) == AT_SUCCESS)
    {

        /*MODEM��·�� */
        ucDlci      = AT_MODEM_USER_DLCI;

        /* �����豸��ǰģʽ���ַ��������� */
        At_ModemDataInd(ucIndex, ucDlci, pstBuf);
    }

    return;
}


VOS_VOID AT_UsbModemReadMscCB(AT_DCE_MSC_STRU *pstRcvedMsc)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucDlci;

    if (pstRcvedMsc == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_UsbModemReadMscCB, WARNING, Receive NULL pointer MSC info!");

        return;
    }

    /* MODEM������ */
    ucIndex     = AT_CLIENT_TAB_MODEM_INDEX;

    /*MODEM��·�� */
    ucDlci      = AT_MODEM_USER_DLCI;

     /* ����ܽ��źſ�ά�ɲ� */
    AT_MNTN_TraceInputMsc(ucIndex, pstRcvedMsc);

    At_ModemMscInd(ucIndex, ucDlci, pstRcvedMsc);

    return;
}


VOS_VOID AT_UsbModemInit( VOS_VOID )
{
    UDI_OPEN_PARAM_S                    stParam;
    VOS_UINT8                           ucIndex;

    ucIndex         = AT_CLIENT_TAB_MODEM_INDEX;
    stParam.devid   = UDI_ACM_MODEM_ID;

    /* ��Device�����ID */
    g_alAtUdiHandle[ucIndex] = mdrv_udi_open(&stParam);

    if (g_alAtUdiHandle[ucIndex] == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_UsbModemInit, ERROR, Open usb modem device failed!");

        return;
    }

    /* ע��MODEM�豸�������ݽ��ջص� */
    if (mdrv_udi_ioctl (g_alAtUdiHandle[ucIndex], ACM_IOCTL_SET_READ_CB, AT_UsbModemReadDataCB) != VOS_OK)
    {
        AT_ERR_LOG("AT_UsbModemInit, ERROR, Set data read callback for modem failed!");

        return;
    }

    /* ע��MODEM���������ڴ��ͷŽӿ� */
    if (mdrv_udi_ioctl (g_alAtUdiHandle[ucIndex], ACM_IOCTL_SET_FREE_CB, AT_ModemFreeDlDataBuf) != VOS_OK)
    {
        AT_ERR_LOG("AT_UsbModemInit, ERROR, Set memory free callback for modem failed!");

        return;
    }

    /* ע��MODEM�������ݲ���Ҫ���� */
    if (mdrv_udi_ioctl (g_alAtUdiHandle[ucIndex], ACM_IOCTL_WRITE_DO_COPY, (void *)0) != VOS_OK)
    {
        AT_ERR_LOG("AT_UsbModemInit, ERROR, Set not do copy for modem failed!");

        return;
    }

    /* ע��ܽ��ź�֪ͨ�ص� */
    if (mdrv_udi_ioctl (g_alAtUdiHandle[ucIndex], ACM_MODEM_IOCTL_SET_MSC_READ_CB, AT_UsbModemReadMscCB) != VOS_OK)
    {
        AT_ERR_LOG("AT_UsbModemInit, ERROR, Set msc read callback for modem failed!");

        return;
    }

    /* ע��MODEM�豸ʹ�ܡ�ȥʹ��֪ͨ�ص� */
    if (mdrv_udi_ioctl (g_alAtUdiHandle[ucIndex], ACM_MODEM_IOCTL_SET_REL_IND_CB, AT_UsbModemEnableCB) != VOS_OK)
    {
        AT_ERR_LOG("AT_UsbModemInit, ERROR, Set enable callback for modem failed!");

        return;
    }

    /* ����MODEM�豸��������buffer��� */
    AT_ModemInitUlDataBuf(ucIndex, AT_MODEM_UL_DATA_BUFF_SIZE, AT_MODEM_UL_DATA_BUFF_NUM);

    /* ��ʼ��MODMEͳ����Ϣ */
    AT_InitModemStats();

    /*ע��client id*/
    At_ModemEst(ucIndex, AT_CLIENT_ID_MODEM, AT_USB_MODEM_PORT_NO);

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_MODEM, ID_AT_MNTN_RESULT_MODEM);

    return;
}


VOS_VOID AT_UsbModemClose(VOS_VOID)
{
    AT_CLIENT_TAB_INDEX_UINT8           ucIndex;

    ucIndex = AT_CLIENT_TAB_MODEM_INDEX;

    /* ȥע��MODEM���ص�(��TTFȷ��δע�����ص�Ҳ����ȥע�����ص�)�� */

    AT_DeRegModemPsDataFCPoint(ucIndex, AT_GET_RABID_FROM_EXRABID(gastAtClientTab[ucIndex].ucExPsRabId));

    if (g_alAtUdiHandle[ucIndex] != UDI_INVALID_HANDLE)
    {
        mdrv_udi_close(g_alAtUdiHandle[ucIndex]);

        g_alAtUdiHandle[ucIndex] = UDI_INVALID_HANDLE;

        PS_PRINTF_INFO("<AT_UsbModemClose>\n");
    }

    return;
}


VOS_VOID AT_SetUsbDebugFlag(VOS_UINT32 ulFlag)
{
    g_ulAtUsbDebugFlag = ulFlag;
}



VOS_INT At_RcvFromUsbCom(
    VOS_UINT8                           ucPortNo,
    VOS_UINT8                          *pData,
    VOS_UINT16                          uslength
)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulRet;

    if (g_ulAtUsbDebugFlag == VOS_TRUE)
    {
        PS_PRINTF_INFO("<At_RcvFromUsbCom> PortNo = %d, length = %d, data = %s\n", ucPortNo, uslength, pData);
    }

    if (pData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_RcvFromUsbCom: pData is NULL PTR!");
        return AT_DRV_FAILURE;
    }

    if (uslength == 0)
    {
        AT_WARN_LOG("At_RcvFromUsbCom: uslength is 0!");
        return AT_DRV_FAILURE;
    }

    /*PCUI��CTRL����*/
    for (ucIndex = 0; ucIndex < AT_MAX_CLIENT_NUM; ucIndex++)
    {
        if ((gastAtClientTab[ucIndex].UserType == AT_USBCOM_USER)
         || (gastAtClientTab[ucIndex].UserType == AT_CTR_USER)
         || (gastAtClientTab[ucIndex].UserType == AT_PCUI2_USER)
         || (gastAtClientTab[ucIndex].UserType == AT_UART_USER))
        {
            if (gastAtClientTab[ucIndex].ucUsed != AT_CLIENT_NULL)
            {
                if (gastAtClientTab[ucIndex].ucPortNo == ucPortNo)
                {
                    break;
                }
            }
        }
    }

    if (g_ulAtUsbDebugFlag == VOS_TRUE)
    {
        PS_PRINTF_INFO("<At_RcvFromUsbCom> ucIndex = %d\n", ucIndex);
    }

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_RcvFromUsbCom (ucIndex >= AT_MAX_CLIENT_NUM)");
        return AT_DRV_FAILURE;
    }

    AT_MNTN_IncRcvStreamCnt(ucIndex);

    if (g_ulAtUsbDebugFlag == VOS_TRUE)
    {
        PS_PRINTF_INFO("<At_RcvFromUsbCom> CmdMode = %d\n", gastAtClientTab[ucIndex].Mode);
    }

    if (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
    {
        ulRet = At_CmdStreamPreProc(ucIndex,pData,uslength);
    }
    else
    {
        ulRet = At_DataStreamPreProc(ucIndex,gastAtClientTab[ucIndex].DataMode,pData,uslength);
    }

    if ( ulRet == AT_SUCCESS )
    {
        return AT_DRV_SUCCESS;
    }
    else
    {
        return AT_DRV_FAILURE;
    }
}



VOS_UINT32 At_UsbPcuiEst(VOS_UINT8 ucPortNo)
{
    VOS_UINT8                           ucIndex;

    if (ucPortNo != AT_USB_COM_PORT_NO)
    {
        AT_WARN_LOG("At_UsbPcuiEst the PortNo is error)");
        return VOS_ERR;
    }

    ucIndex = AT_CLIENT_TAB_PCUI_INDEX;

    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_PCUI, ID_AT_MNTN_RESULT_PCUI);

    /* ��д�û����� */
    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_PCUI;
    gastAtClientTab[ucIndex].ucPortNo        = ucPortNo;
    gastAtClientTab[ucIndex].UserType        = AT_USBCOM_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;

    /* ���¿��Բ�����д��ǰ��PS_MEMSET�Ѿ���ʼ����ֻΪ�ɿ���� */
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    #if (VOS_WIN32 == VOS_OS_VER)
    Sock_RecvCallbackRegister(ucPortNo, (pComRecv)At_RcvFromUsbCom);
    #else
    /*��DMSע��Ӵ����л�ȡ���ݵĻص�����*/
    (VOS_VOID)DMS_COM_RCV_CALLBACK_REGI(ucPortNo, (pComRecv)At_RcvFromUsbCom);
    #endif


    AT_LOG1("At_UsbPcuiEst ucIndex:",ucIndex);
    return VOS_OK;
}



VOS_UINT32 At_UsbCtrEst(VOS_UINT8 ucPortNo)
{
    VOS_UINT8                           ucIndex;


    if (ucPortNo != AT_CTR_PORT_NO)
    {
        AT_WARN_LOG("At_UsbCtrEst the PortNo is error)");
        return VOS_ERR;
    }

    ucIndex = AT_CLIENT_TAB_CTRL_INDEX;

    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_CTRL, ID_AT_MNTN_RESULT_CTRL);

    /* ��д�û����� */
    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_CTRL;
    gastAtClientTab[ucIndex].ucPortNo        = ucPortNo;
    gastAtClientTab[ucIndex].UserType        = AT_CTR_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;

    /* ���¿��Բ�����д��ǰ��PS_MEMSET�Ѿ���ʼ����ֻΪ�ɿ���� */
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;


    /*�����DMS�Ӵ����л�ȡ���ݵĻص�����*/
    (VOS_VOID)DMS_COM_RCV_CALLBACK_REGI(ucPortNo, (pComRecv)At_RcvFromUsbCom);

    return VOS_OK;
}


VOS_UINT32 At_UsbPcui2Est(VOS_UINT8 ucPortNo)
{
    VOS_UINT8                           ucIndex;

    if (ucPortNo != AT_PCUI2_PORT_NO)
    {
        AT_WARN_LOG("At_UsbPcui2Est the PortNo is error)");
        return VOS_ERR;
    }

    ucIndex = AT_CLIENT_TAB_PCUI2_INDEX;

    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_PCUI2, ID_AT_MNTN_RESULT_PCUI2);

    /* ��д�û����� */
    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_PCUI2;
    gastAtClientTab[ucIndex].ucPortNo        = ucPortNo;
    gastAtClientTab[ucIndex].UserType        = AT_PCUI2_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;

    /* ���¿��Բ�����д��ǰ��PS_MEMSET�Ѿ���ʼ����ֻΪ�ɿ���� */
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    /*�����DMS�Ӵ����л�ȡ���ݵĻص�����*/
    (VOS_VOID)DMS_COM_RCV_CALLBACK_REGI(ucPortNo, (pComRecv)At_RcvFromUsbCom);

    return VOS_OK;
}


VOS_UINT32 AT_UART_GetUlDataBuff(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                         **ppucData,
    VOS_UINT32                         *pulLen
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_UART_GetUlDataBuff: Invalid UDI handle!\r\n");
        return AT_FAILURE;
    }

    /* ��ȡ������������BUFFER */
    stCtlParam.pVirAddr = VOS_NULL_PTR;
    stCtlParam.pPhyAddr = VOS_NULL_PTR;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = VOS_NULL_PTR;

    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_GET_RD_BUFF, &stCtlParam);
    if (lResult != VOS_OK)
    {
        AT_ERR_LOG("AT_UART_GetUlDataBuff: Get buffer failed!\r\n");
        return AT_FAILURE;
    }

    if ( (stCtlParam.pVirAddr == VOS_NULL_PTR)
      || (stCtlParam.u32Size == AT_INIT_DATA_LEN) )
    {
        AT_ERR_LOG("AT_UART_GetUlDataBuff: Data buffer error!\r\n");
        return AT_FAILURE;
    }

    *ppucData = (VOS_UINT8 *)stCtlParam.pVirAddr;
    *pulLen   = stCtlParam.u32Size;

    return AT_SUCCESS;
}


VOS_UINT32 AT_UART_WriteDataSync(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT32                          ulLen
)
{
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_UART_WriteDataSync: Invalid UDI handle!\r\n");
        return AT_FAILURE;
    }

    /* ���������Ч�� */
    if ((pucData == VOS_NULL_PTR) || (ulLen == 0))
    {
        AT_ERR_LOG("AT_UART_WriteDataSync: DATA is invalid!\r\n");
        return AT_FAILURE;
    }

    lResult = mdrv_udi_write(lUdiHandle, pucData, ulLen);
    if (lResult != VOS_OK)
    {
        AT_HSUART_DBG_DL_WRITE_SYNC_FAIL_LEN(ulLen);
        AT_ERR_LOG("AT_UART_WriteDataSync: Write buffer failed!\r\n");
        return AT_FAILURE;
    }

    AT_HSUART_DBG_DL_WRITE_SYNC_SUCC_LEN(ulLen);

    return AT_SUCCESS;
}


VOS_UINT32 AT_UART_SendDlData(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    /* ͬ��дUART�豸, ���������ͷ� */
    return AT_UART_WriteDataSync(ucIndex, pucData, usLen);
}


VOS_UINT32 AT_UART_SendRawDataFromOm(
    VOS_UINT8                          *pucVirAddr,
    VOS_UINT8                          *pucPhyAddr,
    VOS_UINT32                          ulLen
)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    ucIndex  = AT_CLIENT_TAB_UART_INDEX;

    ulResult = AT_UART_WriteDataSync(ucIndex, pucVirAddr, (VOS_UINT16)ulLen);
    if (ulResult != AT_SUCCESS)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID AT_UART_UlDataReadCB(VOS_VOID)
{
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    VOS_UINT32                          ulLen;
    VOS_UINT8                           ucIndex;

    ulLen   = 0;
    ucIndex = AT_CLIENT_TAB_UART_INDEX;

    if (AT_UART_GetUlDataBuff(ucIndex, &pucData, &ulLen) == AT_SUCCESS)
    {
        /* �����豸��ǰģʽ���ַ��������� */
        At_RcvFromUsbCom(AT_UART_PORT_NO, pucData, (VOS_UINT16)ulLen);
    }

    return;
}


VOS_VOID AT_UART_InitLink(VOS_UINT8 ucIndex)
{
    TAF_AT_NVIM_DEFAULT_LINK_OF_UART_STRU    stDefaultLinkType;


    stDefaultLinkType.enUartLinkType = AT_UART_LINK_TYPE_BUTT;


    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    gastAtClientTab[ucIndex].ucPortNo  = AT_UART_PORT_NO;
    gastAtClientTab[ucIndex].UserType  = AT_UART_USER;
    gastAtClientTab[ucIndex].ucUsed    = AT_CLIENT_USED;


    /* ��ȡUART�˿�Ĭ�Ϲ���ģʽNV�� */
    if (TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_DEFAULT_LINK_OF_UART,
                                   &stDefaultLinkType.enUartLinkType,
                                   sizeof(stDefaultLinkType.enUartLinkType)) != NV_OK)
    {
        /* NV���ȡʧ�ܣ���UART�˿ڵĹ���ģʽ����ΪOMģʽ */
        AT_ERR_LOG("AT_UART_InitLink:Read NV failed!");

        /*��¼AT/OMͨ������Ӧ��������*/
        gucAtOmIndex = ucIndex;

        /* �л���OM����ģʽ */
        At_SetMode(ucIndex, AT_DATA_MODE, AT_OM_DATA_MODE);
        gastAtClientTab[ucIndex].DataState = AT_DATA_START_STATE;

        AT_AddUsedClientId2Tab(AT_CLIENT_TAB_UART_INDEX);

        /* ֪ͨOAM�л�UART��OMģʽ */
        CBTCPM_NotifyChangePort(AT_UART_PORT);
    }
    else
    {
        /* NV��ȡ�ɹ������UART�˿ڵ�Ĭ�Ϲ���ģʽ */
        if (stDefaultLinkType.enUartLinkType != AT_UART_LINK_TYPE_AT)
        {
            AT_NORM_LOG("AT_UART_InitLink:DEFAULT UART LINK TYPE is OM!");

            /*��¼AT/OMͨ������Ӧ��������*/
            gucAtOmIndex = ucIndex;

            /* �л���OM����ģʽ */
            At_SetMode(ucIndex, AT_DATA_MODE, AT_OM_DATA_MODE);
            gastAtClientTab[ucIndex].DataState = AT_DATA_START_STATE;

            AT_AddUsedClientId2Tab(AT_CLIENT_TAB_UART_INDEX);

            /* ֪ͨOAM�л�UART��OMģʽ */
            CBTCPM_NotifyChangePort(AT_UART_PORT);
        }
        else
        {
            /* ��д�û����� */
            gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_UART;

            /* ���¿��Բ�����д��ǰ��PS_MEMSET�Ѿ���ʼ����ֻΪ�ɿ���� */
            gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
            gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
            gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
            gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
            gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
            g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;
        }
    }

    return;
}


VOS_VOID AT_UART_InitPort(VOS_VOID)
{
    UDI_OPEN_PARAM_S                    stParam;
    UDI_HANDLE                          lUdiHandle;
    VOS_UINT8                           ucIndex;

    stParam.devid = UDI_UART_0_ID;
    ucIndex       = AT_CLIENT_TAB_UART_INDEX;

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_UART, ID_AT_MNTN_RESULT_UART);

    lUdiHandle = mdrv_udi_open(&stParam);
    if (lUdiHandle != UDI_INVALID_HANDLE)
    {
        /* ע��UART�豸�������ݽ��ջص� */
        if (mdrv_udi_ioctl (lUdiHandle, UART_IOCTL_SET_READ_CB, AT_UART_UlDataReadCB) != VOS_OK)
        {
            AT_ERR_LOG("AT_UART_InitPort: Reg data read callback failed!\r\n");
        }

        /* ��ʼ��UART��· */
        AT_UART_InitLink(ucIndex);
        g_alAtUdiHandle[ucIndex] = lUdiHandle;
    }
    else
    {
        AT_ERR_LOG("AT_UART_InitPort: Open UART device failed!\r\n");
        g_alAtUdiHandle[ucIndex] = UDI_INVALID_HANDLE;
    }

    return;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_UINT32 AT_HSUART_IsBaudRateValid(AT_UART_BAUDRATE_ENUM_UINT32 enBaudRate)
{
    VOS_UINT32                          ulRet = VOS_FALSE;

    /*
     * ARM   --- 0,300,600,1200,2400,4800,9600,19200,38400,57600,115200,
     *           230400,460800,921600,2764800,4000000
     */
    switch (enBaudRate)
    {
        case AT_UART_BAUDRATE_0:
        case AT_UART_BAUDRATE_300:
        case AT_UART_BAUDRATE_600:
        case AT_UART_BAUDRATE_1200:
        case AT_UART_BAUDRATE_2400:
        case AT_UART_BAUDRATE_4800:
        case AT_UART_BAUDRATE_9600:
        case AT_UART_BAUDRATE_19200:
        case AT_UART_BAUDRATE_38400:
        case AT_UART_BAUDRATE_57600:
        case AT_UART_BAUDRATE_115200:
        case AT_UART_BAUDRATE_230400:
        case AT_UART_BAUDRATE_460800:
        case AT_UART_BAUDRATE_921600:
        case AT_UART_BAUDRATE_2764800:
        case AT_UART_BAUDRATE_4000000:
            ulRet = VOS_TRUE;
            break;

        default:
            ulRet = VOS_FALSE;
            break;
    }

    return ulRet;
}


VOS_UINT32 AT_HSUART_IsFormatValid(AT_UART_FORMAT_ENUM_UINT8 enFormat)
{
    VOS_UINT32                          ulRet = VOS_FALSE;

    switch (enFormat)
    {
        case AT_UART_FORMAT_8DATA_2STOP:
        case AT_UART_FORMAT_8DATA_1PARITY_1STOP:
        case AT_UART_FORMAT_8DATA_1STOP:
        case AT_UART_FORMAT_7DATA_2STOP:
        case AT_UART_FORMAT_7DATA_1PARITY_1STOP:
        case AT_UART_FORMAT_7DATA_1STOP:
            ulRet = VOS_TRUE;
            break;

        default:
            ulRet = VOS_FALSE;
            break;
    }

    return ulRet;
}


VOS_UINT32 AT_HSUART_IsParityValid(AT_UART_PARITY_ENUM_UINT8 enParity)
{
    VOS_UINT32                          ulRet = VOS_FALSE;

    /*
     * ע: ��ͬ�汾ѡ�õ�UART IP��ͬ, У�����������
     *
     * V3R3            --- ODD, EVEN, MARK, SPACE
     *
     * V7R11(or later) --- ODD, EVEN
     *
     */
    switch (enParity)
    {
        case AT_UART_PARITY_ODD:
        case AT_UART_PARITY_EVEN:
            ulRet = VOS_TRUE;
            break;

        default:
            ulRet = VOS_FALSE;
            break;
    }

    return ulRet;
}


VOS_UINT32 AT_HSUART_ValidateFlowCtrlParam(
    AT_UART_FC_DCE_BY_DTE_ENUM_UINT8    enFcDceByDte,
    AT_UART_FC_DTE_BY_DCE_ENUM_UINT8    enFcDteByDce
)
{
    /*
     * ע: ��ͬ�汾ѡ�õ�UART IP��ͬ, ���ع���������
     *
     * V3R3            --- Ӳ������֧�������е���������ر�
     *
     * V7R11(or later) --- Ӳ������֧��������ͬʱ������ر�
     *
     */
    if (enFcDceByDte != enFcDteByDce)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32 AT_HSUART_ValidateCharFrameParam(
    AT_UART_FORMAT_ENUM_UINT8           enFormat,
    AT_UART_PARITY_ENUM_UINT8           enParity
)
{
    /* ����ʽ�����Ƿ�֧�� */
    if (AT_HSUART_IsFormatValid(enFormat) == VOS_FALSE)
    {
        return VOS_FALSE;
    }

    /* �����鷽ʽ�Ƿ�֧�� */
    if (AT_HSUART_IsParityValid(enParity) == VOS_FALSE)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


AT_UART_FORMAT_PARAM_STRU *AT_HSUART_GetFormatParam(
    AT_UART_FORMAT_ENUM_UINT8           enFormat
)
{
    AT_UART_FORMAT_PARAM_STRU          *pstFormatTblPtr = VOS_NULL_PTR;
    AT_UART_FORMAT_PARAM_STRU          *pstFormatParam  = VOS_NULL_PTR;
    VOS_UINT32                          ulCnt;

    pstFormatTblPtr = AT_UART_GET_FORMAT_TBL_PTR();

    for (ulCnt = 0; ulCnt < AT_UART_GET_FORMAT_TBL_SIZE(); ulCnt++)
    {
        if (enFormat == pstFormatTblPtr[ulCnt].enFormat)
        {
            pstFormatParam = &pstFormatTblPtr[ulCnt];
        }
    }

    return pstFormatParam;
}


VOS_UINT32 AT_HSUART_GetUdiValueByDataLen(
    AT_UART_DATA_LEN_ENUM_UINT8         enDataLen,
    VOS_UINT32                         *pulUdiValue
)
{
    /* ָ��ǿռ�� */
    if (pulUdiValue == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    /* ӳ�����UDI VALUE */
    switch (enDataLen)
    {
        case AT_UART_DATA_LEN_5_BIT:
            *pulUdiValue = WLEN_5_BITS;
            break;

        case AT_UART_DATA_LEN_6_BIT:
            *pulUdiValue = WLEN_6_BITS;
            break;

        case AT_UART_DATA_LEN_7_BIT:
            *pulUdiValue = WLEN_7_BITS;
            break;

        case AT_UART_DATA_LEN_8_BIT:
            *pulUdiValue = WLEN_8_BITS;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_HSUART_GetUdiValueByStopLen(
    AT_UART_STOP_LEN_ENUM_UINT8         enStopLen,
    VOS_UINT32                         *pulUdiValue
)
{
    /* ָ��ǿռ�� */
    if (pulUdiValue == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    /* ӳ�����UDI VALUE */
    switch (enStopLen)
    {
        case AT_UART_STOP_LEN_1_BIT:
            *pulUdiValue = STP2_OFF;
            break;

        case AT_UART_STOP_LEN_2_BIT:
            *pulUdiValue = STP2_ON;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_HSUART_GetUdiValueByParity(
    AT_UART_PARITY_ENUM_UINT8           enParity,
    VOS_UINT32                         *pulUdiValue
)
{
    /* ָ��ǿռ�� */
    if (pulUdiValue == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    /* ӳ�����UDI VALUE */
    switch (enParity)
    {
        case AT_UART_PARITY_ODD:
            *pulUdiValue = PARITY_CHECK_ODD;
            break;

        case AT_UART_PARITY_EVEN:
            *pulUdiValue = PARITY_CHECK_EVEN;
            break;

        case AT_UART_PARITY_MARK:
            *pulUdiValue = PARITY_CHECK_MARK;
            break;

        case AT_UART_PARITY_SPACE:
            *pulUdiValue = PARITY_CHECK_SPACE;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_HSUART_WriteMscCmd(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstDceMsc
)
{
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_WriteMscCmd: Invalid UDI handle!\r\n");
        return AT_FAILURE;
    }

    /* д�ܽ��ź� */
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_MSC_WRITE_CMD, pstDceMsc);
    if (lResult != VOS_OK)
    {
        AT_ERR_LOG("AT_HSUART_WriteMscCmd: Write failed!\r\n");
        AT_HSUART_DBG_IOCTL_MSC_WRITE_FAIL_NUM(1);
        return AT_FAILURE;
    }

    AT_HSUART_DBG_IOCTL_MSC_WRITE_SUCC_NUM(1);
    return AT_SUCCESS;
}


VOS_UINT32 AT_HSUART_ConfigFlowCtrl(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulRtsFlag,
    VOS_UINT32                          ulCtsFlag
)
{
    UDI_HANDLE                          lUdiHandle;
    uart_flow_ctrl_union                unFlowCtrlValue;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_ConfigFlowCtrl: Invalid UDI handle!");
        return VOS_ERR;
    }

    memset_s(&unFlowCtrlValue, sizeof(unFlowCtrlValue), 0x00, sizeof(unFlowCtrlValue));

    unFlowCtrlValue.reg.rtsen = ulRtsFlag;
    unFlowCtrlValue.reg.ctsen = ulCtsFlag;

    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_FLOW_CONTROL, &unFlowCtrlValue) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_CFG_FC_FAIL_NUM(1);
        return VOS_ERR;
    }

    AT_HSUART_DBG_IOCTL_CFG_FC_SUCC_NUM(1);
    return VOS_OK;
}


VOS_UINT32 AT_HSUART_ConfigCharFrame(
    VOS_UINT8                           ucIndex,
    AT_UART_FORMAT_ENUM_UINT8           enFormat,
    AT_UART_PARITY_ENUM_UINT8           enParity
)
{
    AT_UART_FORMAT_PARAM_STRU          *pstFormatParam = VOS_NULL_PTR;
    UDI_HANDLE                          lUdiHandle;
    VOS_UINT32                          ulUdiDataLenth;
    VOS_UINT32                          ulUdiStpLenth;
    VOS_UINT32                          ulUdiParity;
    VOS_UINT32                          ulResult;

    /* ������ʼ�� */
    ulUdiDataLenth = WLEN_8_BITS;
    ulUdiStpLenth  = STP2_OFF;
    ulUdiParity    = PARITY_NO_CHECK;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Invalid UDI handle!");
        return VOS_ERR;
    }

    /* �����õ�֡��ʽ��У�鷽��ת��ΪDRV�ӿڸ�ʽ */
    pstFormatParam = AT_HSUART_GetFormatParam(enFormat);
    if (pstFormatParam == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Format is invalid!");
        return VOS_ERR;
    }

    ulResult = AT_HSUART_GetUdiValueByDataLen(pstFormatParam->enDataBitLength, &ulUdiDataLenth);
    if (ulResult != VOS_OK)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Data bit length is invalid!");
        return VOS_ERR;
    }

    ulResult = AT_HSUART_GetUdiValueByStopLen(pstFormatParam->enStopBitLength, &ulUdiStpLenth);
    if (ulResult != VOS_OK)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Stop bit length is invalid!");
        return VOS_ERR;
    }

    if (pstFormatParam->enParityBitLength == AT_UART_PARITY_LEN_1_BIT)
    {
        ulResult = AT_HSUART_GetUdiValueByParity(enParity, &ulUdiParity);
        if (ulResult != VOS_OK)
        {
            AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Parity bit length is invalid!");
            return VOS_ERR;
        }
    }

    /* ����DRV�������ô�������λ���� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_WLEN, (VOS_VOID *)&ulUdiDataLenth) != MDRV_OK)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Set WLEN failed!");
        AT_HSUART_DBG_IOCTL_SET_WLEN_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* ����DRV�������ô���ֹͣλ���� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_STP2, (VOS_VOID *)&ulUdiStpLenth) != MDRV_OK)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Set STP2 failed!");
        AT_HSUART_DBG_IOCTL_SET_STP_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* ����DRV�������ô���У��λ */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_EPS, (VOS_VOID *)&ulUdiParity) != MDRV_OK)
    {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Set Parity failed!");
        AT_HSUART_DBG_IOCTL_SET_PARITY_FAIL_NUM(1);
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_HSUART_ConfigBaudRate(
    VOS_UINT8                           ucIndex,
    AT_UART_BAUDRATE_ENUM_UINT32        enBaudRate
)
{
    UDI_HANDLE                          lUdiHandle;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_ConfigBaudRate: Invalid UDI handle!");
        return VOS_ERR;
    }

    /* ����DRV�������ô��ڵĲ����� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_BAUD, (VOS_VOID *)&enBaudRate) != MDRV_OK)
    {
        AT_ERR_LOG("AT_HSUART_ConfigBaudRate: Set Baud failed!");
        AT_HSUART_DBG_IOCTL_SET_BAUD_FAIL_NUM(1);
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_HSUART_FreeUlDataBuff(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstImmZc
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_FreeUlDataBuff: Invalid UDI handle!");
        return AT_FAILURE;
    }

    /* ��д���ͷŵ��ڴ��ַ */
    stCtlParam.pVirAddr = (VOS_CHAR *)pstImmZc;
    stCtlParam.pPhyAddr = VOS_NULL_PTR;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = VOS_NULL_PTR;

    /* ����ִ���ͷ��ڴ���� */
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_RETURN_BUFF, &stCtlParam);
    if (lResult != VOS_OK)
    {
        AT_ERR_LOG1("AT_HSUART_FreeUlDataBuff: Return buffer failed! <code>\r\n", lResult);
        AT_HSUART_DBG_UL_RETURN_BUFF_FAIL_NUM(1);
        IMM_ZcFree(pstImmZc);
        return AT_FAILURE;
    }

    AT_HSUART_DBG_UL_RETURN_BUFF_SUCC_NUM(1);
    return AT_SUCCESS;
}


VOS_VOID AT_HSUART_FreeDlDataBuff(IMM_ZC_STRU *pstImmZc)
{
    if (pstImmZc != VOS_NULL_PTR)
    {
        AT_HSUART_DBG_DL_FREE_BUFF_NUM(1);
        IMM_ZcFree(pstImmZc);
    }

    return;
}


VOS_UINT32 AT_HSUART_ClearDataBuff(VOS_UINT8 ucIndex)
{
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_ClearDataBuff: Invalid UDI handle!");
        return VOS_ERR;
    }

    /* ����ִ������������ */
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_RELEASE_BUFF, VOS_NULL_PTR);
    if (lResult != MDRV_OK)
    {
        AT_ERR_LOG1("AT_HSUART_ClearDataBuff: Release buffer failed! <code>\r\n", lResult);
        AT_HSUART_DBG_IOCTL_CLEAR_BUFF_FAIL_NUM(1);
        return VOS_ERR;
    }

    AT_HSUART_DBG_IOCTL_CLEAR_BUFF_SUCC_NUM(1);
    return VOS_OK;
}


VOS_UINT32 AT_HSUART_GetUlDataBuff(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                       **pstImmZc,
    VOS_UINT32                         *pulLen
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_GetUlDataBuff: Invalid UDI handle!\r\n");
        return AT_FAILURE;
    }

    /* ��ȡ������������BUFFER */
    stCtlParam.pVirAddr = VOS_NULL_PTR;
    stCtlParam.pPhyAddr = VOS_NULL_PTR;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = VOS_NULL_PTR;

    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_GET_RD_BUFF, &stCtlParam);
    if (lResult != VOS_OK)
    {
        AT_ERR_LOG1("AT_HSUART_GetUlDataBuff: Get buffer failed! <code>", lResult);
        AT_HSUART_DBG_UL_GET_RD_FAIL_NUM(1);
        return AT_FAILURE;
    }

    /* ������Ч�Լ�� */
    if ( (stCtlParam.pVirAddr == VOS_NULL_PTR)
      || (stCtlParam.u32Size == AT_INIT_DATA_LEN) )
    {
        AT_ERR_LOG("AT_HSUART_GetUlDataBuff: Data buffer error");
        AT_HSUART_DBG_UL_INVALID_RD_NUM(1);
        return AT_FAILURE;
    }

    AT_HSUART_DBG_UL_GET_RD_SUCC_NUM(1);

    *pstImmZc = (IMM_ZC_STRU *)stCtlParam.pVirAddr;
    *pulLen   = stCtlParam.u32Size;

    return AT_SUCCESS;
}


VOS_UINT32 AT_HSUART_WriteDataAsync(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstImmZc
)
{
    ACM_WR_ASYNC_INFO                   stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    VOS_INT32                           ulResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_WriteDataAsync: Invalid UDI handle!\r\n");
        AT_HSUART_FreeDlDataBuff(pstImmZc);
        return AT_FAILURE;
    }

    /* ��д�������ڴ��ַ */
    stCtlParam.pVirAddr = (VOS_CHAR *)pstImmZc;
    stCtlParam.pPhyAddr = VOS_NULL_PTR;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = VOS_NULL_PTR;

    /* �첽��ʽд�� */
    ulResult = mdrv_udi_ioctl(g_alAtUdiHandle[ucIndex], UART_IOCTL_WRITE_ASYNC, &stCtlParam);
    if (ulResult != VOS_OK)
    {
        AT_ERR_LOG("AT_HSUART_WriteDataAsync: Write failed!\r\n");
        AT_HSUART_DBG_DL_WRITE_ASYNC_FAIL_NUM(1);
        AT_HSUART_FreeDlDataBuff(pstImmZc);
        return AT_FAILURE;
    }

    AT_HSUART_DBG_DL_WRITE_ASYNC_SUCC_NUM(1);
    return AT_SUCCESS;
}


VOS_UINT32 AT_HSUART_SendDlData(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    IMM_ZC_STRU                        *pstImmZc  = VOS_NULL_PTR;
    VOS_CHAR                           *pcPutData = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;

    /* ��A�������ڴ��з���ռ� */
    pstImmZc = IMM_ZcStaticAlloc((VOS_UINT16)usLen);
    if (pstImmZc == VOS_NULL_PTR)
    {
        return AT_FAILURE;
    }

    /* ƫ������βָ�� */
    pcPutData = (VOS_CHAR *)IMM_ZcPut(pstImmZc, usLen);

    /* �������� */
    lMemResult = memcpy_s(pcPutData, usLen, pucData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen, usLen);

    /* �첽дHSUART�豸, д�ɹ����ڴ��ɵ������ͷ� */
    ulResult = AT_HSUART_WriteDataAsync(ucIndex, pstImmZc);

    return ulResult;
}


VOS_VOID AT_HSUART_ProcUlData(
    VOS_UINT8                           ucIndex,
    IMM_ZC_STRU                        *pstImmZc
)
{
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    VOS_UINT16                          usLen;

    /* ��pstData(IMM_ZC_STRU����)��ȡ���������ݺͳ��ȣ��ֱ𱣴���pData��usLen�� */
    pucData = pstImmZc->data;
    usLen   = (VOS_UINT16)pstImmZc->len;

    /* �����ǰ��������ģʽ ������ online_commandģʽ */
    if ( (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
      || (gastAtClientTab[ucIndex].Mode == AT_ONLINE_CMD_MODE) )
    {
        /* ��UARTͨ���Ѿ���������̬������ʱ��Ȼ�յ�PPP֡����OM���ݣ���ֱ�Ӷ��� */
        if ((usLen > 0) && (pucData[0] == 0x7e) && (pucData[usLen - 1] == 0x7e))
        {
            AT_HSUART_DBG_UL_INVALID_CMD_DATA_NUM(1);
            AT_HSUART_FreeUlDataBuff(ucIndex, pstImmZc);
            return;
        }

        AT_HSUART_DBG_UL_VALID_CMD_DATA_NUM(1);

        /* ����AT�����ַ��͸� AT */
        if (At_CmdStreamPreProc(ucIndex, pucData, usLen) != AT_SUCCESS)
        {
            AT_WARN_LOG("AT_HSUART_ProcUlData: At_CmdStreamPreProc fail!");
        }

        AT_HSUART_FreeUlDataBuff(ucIndex, pstImmZc);
        return;
    }

    /* ����UART�ڵ�״̬���зַ�*/
    switch (gastAtClientTab[ucIndex].DataMode)
    {
        /* ����PPP data���� */
        case AT_PPP_DATA_MODE:

            /* PPP�����ͷ������ڴ� */
            PPP_PullPacketEvent(gastAtClientTab[ucIndex].usPppId, pstImmZc);

            /* ��¼��ά�ɲ� */
            AT_HSUART_DBG_UL_IP_DATA_NUM(1);
            return;

        /* ����IP data ���� */
        case AT_IP_DATA_MODE:

            /* PPP�����ͷ������ڴ� */
            PPP_PullRawDataEvent(gastAtClientTab[ucIndex].usPppId, pstImmZc);

            /* ��¼��ά�ɲ� */
            AT_HSUART_DBG_UL_PPP_DATA_NUM(1);
            return;

        /* ����OM���� */
        case AT_DIAG_DATA_MODE:
        case AT_OM_DATA_MODE:
            At_OmDataProc(gastAtClientTab[ucIndex].ucPortNo, pucData, usLen);

            /* ��¼��ά�ɲ� */
            AT_HSUART_DBG_UL_OM_DATA_NUM(1);
            break;

#if(FEATURE_ON == FEATURE_CSD)
        /* ����CSD���� �ݲ�ʵ��*/
        case AT_CSD_DATA_MODE:
#endif
        default:
            AT_WARN_LOG("AT_HSUART_ProcUlData: DataMode Wrong!");
            AT_HSUART_DBG_UL_INVALID_DATA_NUM(1);
            break;
    }

    AT_HSUART_FreeUlDataBuff(ucIndex, pstImmZc);
    return;
}


VOS_UINT32 AT_HSUART_ProcDtrChange(
    VOS_UINT8                           ucIndex,
    AT_DCE_MSC_STRU                    *pstDceMsc
)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstLineCtrl = AT_GetUartLineCtrlInfo();
    ulRet       = VOS_TRUE;

    if (pstDceMsc->ucDtr == 1)
    {
        /* �ж�&S[<value>] */
        if (pstLineCtrl->enDsrMode == AT_UART_DSR_MODE_ALWAYS_ON)
        {
            AT_CtrlDSR(ucIndex, AT_IO_LEVEL_HIGH);
        }

        /* �ж�&C[<value>] */
        if (pstLineCtrl->enDcdMode == AT_UART_DCD_MODE_ALWAYS_ON)
        {
            AT_CtrlDCD(ucIndex, AT_IO_LEVEL_HIGH);
        }

        /* ֹͣ���� */
        AT_StopFlowCtrl(ucIndex);
    }
    else
    {
        ulRet = AT_HSUART_ProcDtrCtrlMode();
    }

    return ulRet;
}


VOS_UINT32 AT_HSUART_ProcDtrCtrlMode(VOS_VOID)
{
    AT_UART_LINE_CTRL_STRU             *pstLineCtrl = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;

    pstLineCtrl = AT_GetUartLineCtrlInfo();
    ucIndex     = AT_CLIENT_TAB_HSUART_INDEX;

    switch (pstLineCtrl->enDtrMode)
    {
        case AT_UART_DTR_MODE_IGNORE:
            ulRet = VOS_FALSE;
            break;

        case AT_UART_DTR_MODE_SWITCH_CMD_MODE:
            /* Ŀǰֻ֧��PPP��IPģʽ���л�ΪONLINE-COMMANDģʽ */
            if ( (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
              && ( (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
                || (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE) ) )
            {
                if (pstLineCtrl->enDsrMode == AT_UART_DSR_MODE_CONNECT_ON)
                {
                    AT_CtrlDSR(ucIndex, AT_IO_LEVEL_LOW);
                }

                At_SetMode(ucIndex, AT_ONLINE_CMD_MODE, AT_NORMAL_MODE);
                At_FormatResultData(ucIndex, AT_OK);
            }
            ulRet = VOS_FALSE;
            break;

        case AT_UART_DTR_MODE_HANGUP_CALL:
            ulRet = VOS_TRUE;
            break;

        default:
            ulRet = VOS_FALSE;
            break;
    }

    return ulRet;
}


VOS_UINT32 AT_HSUART_StartFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_HSUART_INDEX;

    if (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
    {
        if ( (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
          || (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
          || (gastAtClientTab[ucIndex].DataMode == AT_CSD_DATA_MODE) )
        {
            AT_MNTN_TraceStartFlowCtrl(ucIndex, AT_FC_DEVICE_TYPE_HSUART);
            AT_CtrlCTS(ucIndex, AT_IO_LEVEL_LOW);
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_HSUART_StopFlowCtrl(
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2
)
{
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_HSUART_INDEX;

    if (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
    {
        if ( (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
          || (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
          || (gastAtClientTab[ucIndex].DataMode == AT_CSD_DATA_MODE) )
        {
            AT_MNTN_TraceStopFlowCtrl(ucIndex, AT_FC_DEVICE_TYPE_HSUART);
            AT_CtrlCTS(ucIndex, AT_IO_LEVEL_HIGH);
        }
    }

    return VOS_OK;
}



VOS_VOID AT_HSUART_UlDataReadCB(VOS_VOID)
{
    IMM_ZC_STRU                        *pstImmZc = VOS_NULL_PTR;
    VOS_UINT32                          ulLen;
    VOS_UINT8                           ucIndex;

    ulLen   = 0;
    ucIndex = AT_CLIENT_TAB_HSUART_INDEX;

    AT_HSUART_DBG_UL_DATA_READ_CB_NUM(1);

    if (AT_HSUART_GetUlDataBuff(ucIndex, &pstImmZc, &ulLen) == AT_SUCCESS)
    {
        /* �����豸��ǰģʽ���ַ��������� */
        AT_HSUART_ProcUlData(ucIndex, pstImmZc);
    }

    return;
}


VOS_UINT32 AT_HSUART_SendRawDataFromOm(
    VOS_UINT8                          *pucVirAddr,
    VOS_UINT8                          *pucPhyAddr,
    VOS_UINT32                          ulLen
)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_HSUART_INDEX;

    ulResult = AT_UART_WriteDataSync(ucIndex, pucVirAddr, ulLen);
    if (ulResult != AT_SUCCESS)
    {
        AT_ERR_LOG("AT_HSUART_SendRawDataFromOm: Send data failed!\r\n");
        AT_HSUART_DBG_DL_WRITE_SYNC_FAIL_NUM(1);
        return VOS_ERR;
    }

    AT_HSUART_DBG_DL_WRITE_SYNC_SUCC_NUM(1);

    return VOS_OK;
}


VOS_VOID AT_HSUART_MscReadCB(AT_DCE_MSC_STRU *pstDceMsc)
{
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_HSUART_INDEX;

    /* ��μ�� */
    if (pstDceMsc == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_HSUART_MscReadCB: pstDceMsc is NULL!");
        return;
    }

    /* ����ܽ��źſ�ά�ɲ� */
    AT_HSUART_DBG_IOCTL_MSC_READ_CB_NUM(1);

    /* ���ܽ���Ϣ���ͳ�ȥ */
    At_ModemMscInd(ucIndex, AT_MODEM_USER_DLCI, pstDceMsc);

    return;
}


VOS_VOID AT_HSUART_SwitchCmdDetectCB(VOS_VOID)
{
    AT_MSG_STRU                        *pstMsg = VOS_NULL_PTR;

    /* ��¼��ά�ɲ� */
    AT_HSUART_DBG_IOCTL_SWITCH_CB_NUM(1);

    /* ������Ϣ */
    pstMsg = (AT_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(
                                WUEPS_PID_AT,
                                sizeof(AT_MSG_STRU));

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_HSUART_SwitchCmdDetectCB: Alloc message failed!");
        return;
    }

    /* ��ʼ����Ϣ */
    TAF_MEM_SET_S((VOS_CHAR *)pstMsg + VOS_MSG_HEAD_LENGTH,
               sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH,
               0x00,
               sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH);

    /* ��д��Ϣͷ */
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = WUEPS_PID_AT;
    pstMsg->enMsgId         = ID_AT_SWITCH_CMD_MODE;

    /* ��д��Ϣ���� */
    pstMsg->ucType          = AT_SWITCH_CMD_MODE_MSG;
    pstMsg->ucIndex         = AT_CLIENT_TAB_HSUART_INDEX;

    /* ������Ϣ */
    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("AT_HSUART_SwitchCmdDetectCB: Send message failed!");
    }

    return;
}


VOS_VOID AT_HSUART_WaterDetectCB(water_level enLevel)
{
    AT_UART_CTX_STRU                   *pstUartCtx = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstMsg     = VOS_NULL_PTR;

    pstUartCtx = AT_GetUartCtxAddr();

    /*
     * (1) ����TX��ˮ�߱�ʶ
     * (2) ���TX�ﵽ��ˮ��, ���͵�ˮ���ڲ���Ϣ, �����������
     */

    pstUartCtx->ulTxWmHighFlg = (HIGH_LEVEL == enLevel) ? VOS_TRUE : VOS_FALSE;

    if (enLevel == LOW_LEVEL)
    {
        /* ����OSA��Ϣ */
        pstMsg = (AT_MSG_STRU *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_MSG_STRU));
        if (pstMsg == VOS_NULL_PTR)
        {
            AT_ERR_LOG("AT_HSUART_SwitchCmdDetectCB: Alloc message failed!");
            return;
        }

        /* �����Ϣ���� */
        TAF_MEM_SET_S(AT_GET_MSG_ENTITY(pstMsg), AT_GET_MSG_LENGTH(pstMsg), 0x00, AT_GET_MSG_LENGTH(pstMsg));

        /* ��д��Ϣͷ */
        AT_CFG_INTRA_MSG_HDR(pstMsg, ID_AT_WATER_LOW_CMD);

        /* ��д��Ϣ���� */
        pstMsg->ucType  = AT_WATER_LOW_MSG;
        pstMsg->ucIndex = AT_CLIENT_TAB_HSUART_INDEX;

        /* ������Ϣ */
        AT_SEND_MSG(pstMsg);
    }

    return;
}


VOS_VOID AT_HSUART_RegCallbackFunc(VOS_UINT8 ucIndex)
{
    ACM_READ_BUFF_INFO                  stReadBuffInfo;
    UDI_HANDLE                          lUdiHandle;

    stReadBuffInfo.u32BuffSize = AT_UART_UL_DATA_BUFF_SIZE;
    stReadBuffInfo.u32BuffNum  = AT_UART_UL_DATA_BUFF_NUM;

    /* ���UDI�����Ч�� */
    lUdiHandle = g_alAtUdiHandle[ucIndex];
    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_RegCallbackFunc: Invalid UDI handle!");
        return;
    }

    /* ע��UART�豸�������ݽ��ջص� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_READ_CB, AT_HSUART_UlDataReadCB) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_SET_READ_CB_ERR(1);
    }

    /* ����UART�豸�������ݻ����� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_RELLOC_READ_BUFF, &stReadBuffInfo) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_RELLOC_READ_BUFF_ERR(1);
    }

    /* ע��UART���������ڴ��ͷŽӿ� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_FREE_CB, AT_HSUART_FreeDlDataBuff) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_SET_FREE_CB_ERR(1);
    }

    /* ע��ܽ��ź�֪ͨ�ص� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_MSC_READ_CB, AT_HSUART_MscReadCB) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_SET_MSC_READ_CB_ERR(1);
    }

    /* ע��"+++"������ص� */
    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SWITCH_MODE_CB, AT_HSUART_SwitchCmdDetectCB) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_SET_SWITCH_CB_ERR(1);
    }

    if (mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SET_WATER_CB, AT_HSUART_WaterDetectCB) != MDRV_OK)
    {
        AT_HSUART_DBG_IOCTL_SET_WATER_CB_ERR(1);
    }

    return;
}


VOS_VOID AT_HSUART_InitLink(VOS_UINT8 ucIndex)
{
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_HSUART;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;
    gastAtClientTab[ucIndex].UserType        = AT_HSUART_USER;
    gastAtClientTab[ucIndex].ucPortNo        = AT_HSUART_PORT_NO;
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    return;
}


VOS_UINT32 AT_HSUART_InitPort(VOS_VOID)
{
    AT_UART_PHY_CFG_STRU               *pstPhyCfg = VOS_NULL_PTR;
    UDI_OPEN_PARAM_S                    stParam;
    UDI_HANDLE                          lUdiHandle;
    VOS_UINT8                           ucIndex;

    pstPhyCfg     = AT_GetUartPhyCfgInfo();
    stParam.devid = UDI_HSUART_0_ID;
    ucIndex       = AT_CLIENT_TAB_HSUART_INDEX;

    /* ����HSUART�˿���Ϣӳ�� */
    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_HSUART, ID_AT_MNTN_RESULT_HSUART);

    /* ��Device�����ID */
    lUdiHandle = mdrv_udi_open(&stParam);

    if (lUdiHandle == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_HSUART_InitPort, ERROR, Open UART device failed!");
        g_alAtUdiHandle[ucIndex] = UDI_INVALID_HANDLE;
        return VOS_ERR;
    }

    /* ����UDI��� */
    g_alAtUdiHandle[ucIndex] = lUdiHandle;

    /* ��ʼ��UART��· */
    AT_HSUART_InitLink(ucIndex);

    /* ע��UART�˿���ػص����� */
    AT_HSUART_RegCallbackFunc(ucIndex);

    /* ����UART������ */
    AT_HSUART_ConfigBaudRate(ucIndex, pstPhyCfg->enBaudRate);

    /* ����UART֡��ʽ */
    AT_HSUART_ConfigCharFrame(ucIndex, pstPhyCfg->stFrame.enFormat, pstPhyCfg->stFrame.enParity);

    return VOS_OK;
}
#endif


VOS_VOID AT_CtrlDCD(
    VOS_UINT8                           ucIndex,
    AT_IO_LEVEL_ENUM_UINT8              enIoLevel
)
{
    AT_DCE_MSC_STRU                     stDceMsc;
    NAS_OM_EVENT_ID_ENUM_UINT16         enEventId;

    memset_s(&stDceMsc, sizeof(stDceMsc), 0x00, sizeof(AT_DCE_MSC_STRU));

    stDceMsc.OP_Dcd = VOS_TRUE;
    stDceMsc.ucDcd  = enIoLevel;

    AT_SetModemStatus(ucIndex, &stDceMsc);

    enEventId = (AT_IO_LEVEL_HIGH == enIoLevel) ?
                NAS_OM_EVENT_DCE_UP_DCD : NAS_OM_EVENT_DCE_DOWN_DCD;

    AT_EventReport(WUEPS_PID_AT, enEventId, &ucIndex, sizeof(VOS_UINT8));

    return;
}


VOS_VOID AT_CtrlDSR(
    VOS_UINT8                           ucIndex,
    AT_IO_LEVEL_ENUM_UINT8              enIoLevel
)
{
    AT_DCE_MSC_STRU                     stDceMsc;
    NAS_OM_EVENT_ID_ENUM_UINT16         enEventId;

    memset_s(&stDceMsc, sizeof(stDceMsc), 0x00, sizeof(AT_DCE_MSC_STRU));

    stDceMsc.OP_Dsr = VOS_TRUE;
    stDceMsc.ucDsr  = enIoLevel;

    AT_SetModemStatus(ucIndex, &stDceMsc);

    enEventId = (AT_IO_LEVEL_HIGH == enIoLevel) ?
                NAS_OM_EVENT_DCE_UP_DSR: NAS_OM_EVENT_DCE_DOWN_DSR;

    AT_EventReport(WUEPS_PID_AT, enEventId, &ucIndex, sizeof(VOS_UINT8));

    return;
}


VOS_VOID AT_CtrlCTS(
    VOS_UINT8                           ucIndex,
    AT_IO_LEVEL_ENUM_UINT8              enIoLevel
)
{
    AT_DCE_MSC_STRU                     stDceMsc;
    NAS_OM_EVENT_ID_ENUM_UINT16         enEventId;

    memset_s(&stDceMsc, sizeof(stDceMsc), 0x00, sizeof(AT_DCE_MSC_STRU));

    stDceMsc.OP_Cts = VOS_TRUE;
    stDceMsc.ucCts  = enIoLevel;

    AT_SetModemStatus(ucIndex, &stDceMsc);

    enEventId = (AT_IO_LEVEL_HIGH == enIoLevel) ?
                NAS_OM_EVENT_DCE_UP_CTS: NAS_OM_EVENT_DCE_DOWN_CTS;

    AT_EventReport(WUEPS_PID_AT, enEventId, &ucIndex, sizeof(VOS_UINT8));

    return;
}


VOS_VOID AT_CtrlRI(
    VOS_UINT8                           ucIndex,
    AT_IO_LEVEL_ENUM_UINT8              enIoLevel
)
{
    AT_DCE_MSC_STRU                     stDceMsc;
    NAS_OM_EVENT_ID_ENUM_UINT16         enEventId;

    memset_s(&stDceMsc, sizeof(stDceMsc), 0x00, sizeof(AT_DCE_MSC_STRU));

    stDceMsc.OP_Ri = VOS_TRUE;
    stDceMsc.ucRi  = enIoLevel;

    AT_SetModemStatus(ucIndex, &stDceMsc);

    enEventId = (AT_IO_LEVEL_HIGH == enIoLevel) ?
                NAS_OM_EVENT_DCE_UP_RI: NAS_OM_EVENT_DCE_DOWN_RI;

    AT_EventReport(WUEPS_PID_AT, enEventId, &ucIndex, sizeof(VOS_UINT8));

    return;
}


AT_IO_LEVEL_ENUM_UINT8 AT_GetIoLevel(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucIoCtrl
)
{
    if ((gastAtClientTab[ucIndex].ModemStatus & ucIoCtrl) == 0)
    {
        return AT_IO_LEVEL_LOW;
    }

    return AT_IO_LEVEL_HIGH;
}


int  App_VcomRecvCallbackRegister(unsigned char  uPortNo, pComRecv pCallback)
{
    return VOS_OK;
}


VOS_INT AT_RcvFromAppCom(
    VOS_UINT8                           ucVcomId,
    VOS_UINT8                          *pData,
    VOS_UINT32                          ullength
)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulRet;

    /* AT����ȴ�����ͳһ�õ�VOS_UINT16������֧�ַ�Χ�ĳ��ȴ�ӡ�쳣���� */
    if (ullength > 0xffff)
    {
        AT_WARN_LOG("AT_RcvFromAppCom: ullength is more than 0xffff!");
        return VOS_ERR;
    }

    if (ucVcomId >= APP_VCOM_DEV_INDEX_BUTT)
    {
        AT_WARN_LOG("AT_RcvFromAppCom: Port No ERR!");
        return VOS_ERR;
    }
    /* APPVCOM��󼸸�����ATͨ�� */
    if (ucVcomId >= AT_VCOM_AT_CHANNEL_MAX)
    {
        AT_WARN_LOG("AT_RcvFromAppCom: Port No ERR!");
        return VOS_ERR;
    }
    if (pData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_RcvFromAppCom: pData is NULL PTR!");
        return VOS_ERR;
    }

    if (ullength == 0)
    {
        AT_WARN_LOG("AT_RcvFromAppCom: uslength is 0!");
        return VOS_ERR;
    }

    /* ���ݶ˿ں�ȷ��Index��ֵ */
    ucIndex = AT_CLIENT_TAB_APP_INDEX + ucVcomId;

    /* �ж��Ƿ���APPͨ�� */
    if ((gastAtClientTab[ucIndex].UserType != AT_APP_USER)
     || (gastAtClientTab[ucIndex].ucUsed == AT_CLIENT_NULL))
    {
        AT_WARN_LOG("AT_RcvFromAppCom: APP client is unused!");
        return VOS_ERR;
    }

    AT_MNTN_IncRcvStreamCnt(ucIndex);

    if (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
    {
        ulRet = At_CmdStreamPreProc(ucIndex, pData, (VOS_UINT16)ullength);
    }
    else
    {
        ulRet = At_DataStreamPreProc(ucIndex, gastAtClientTab[ucIndex].DataMode, pData, (VOS_UINT16)ullength);
    }

    if ( ulRet == AT_SUCCESS )
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_INT32 AT_AppComEst(VOS_VOID)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucLoop;

    for (ucLoop = 0; ucLoop < AT_VCOM_AT_CHANNEL_MAX; ucLoop++)
    {
        ucIndex = AT_CLIENT_TAB_APP_INDEX + ucLoop;

        /* ��ն�Ӧ���� */
        memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

        AT_ConfigTraceMsg(ucIndex, (ID_AT_CMD_APP + ucLoop), (ID_AT_MNTN_RESULT_APP + ucLoop));

        gastAtClientTab[ucIndex].usClientId     = AT_CLIENT_ID_APP + ucLoop;

        /* ��д�û����� */
        gastAtClientTab[ucIndex].ucPortNo        = APP_VCOM_DEV_INDEX_0 + ucLoop;
        gastAtClientTab[ucIndex].UserType        = AT_APP_USER;
        gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;
        gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
        gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
        gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
        gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
        gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

        /* ע��ص����� */
        APP_VCOM_REG_DATA_CALLBACK(gastAtClientTab[ucIndex].ucPortNo, (SEND_UL_AT_FUNC)AT_RcvFromAppCom);
    }

    return VOS_OK;
}


VOS_INT AT_RcvFromSock(
    VOS_UINT8                          *pData,
    VOS_UINT32                         uslength
)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulRet;

    ucIndex = AT_CLIENT_TAB_SOCK_INDEX;

    if (pData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_RcvFromSock: pData is NULL PTR!");
        return VOS_ERR;
    }
    if (uslength == 0)
    {
        AT_WARN_LOG("AT_RcvFromSock: uslength is 0!");
        return VOS_ERR;
    }

    if ((gastAtClientTab[ucIndex].UserType != AT_SOCK_USER)
        ||(gastAtClientTab[ucIndex].ucUsed == AT_CLIENT_NULL))
    {
        AT_WARN_LOG("AT_RcvFromSock: SOCK client is unused!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
    {
        ulRet = At_CmdStreamPreProc(ucIndex,pData,(VOS_UINT16)uslength);
    }
    else
    {
        ulRet = At_DataStreamPreProc(ucIndex,gastAtClientTab[ucIndex].DataMode,pData,(VOS_UINT16)uslength);
    }
    if ( ulRet == AT_SUCCESS )
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_INT32 AT_SockComEst(VOS_UINT8 ucPortNo)
{

    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_SOCK_INDEX;

    if (ucPortNo != AT_SOCK_PORT_NO)
    {
        AT_WARN_LOG("At_SockComEst the PortNo is error)");
        return VOS_ERR;
    }

    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_SOCK, ID_AT_MNTN_RESULT_SOCK);

    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_SOCK;
    gastAtClientTab[ucIndex].ucPortNo        = ucPortNo;
    gastAtClientTab[ucIndex].UserType        = AT_SOCK_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    /* Modified by s62952 for AT Project��2011-10-17,  Begin*/
    /*��DMSע��Ӵ����л�ȡ���ݵĻص�����*/
    (VOS_VOID)mdrv_CPM_LogicRcvReg(CPM_AT_COMM,(CBTCPM_RCV_FUNC)AT_RcvFromSock);
    /* Modified by s62952 for AT Project��2011-10-17,  end*/

    return VOS_OK;
}


VOS_INT AT_RcvFromAppSock(
    VOS_UINT8                           ucPortNo,
    VOS_UINT8                          *pData,
    VOS_UINT16                          uslength
)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulRet;

    ucIndex = AT_CLIENT_TAB_APPSOCK_INDEX;

    if (pData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_RcvFromAppSock: pData is NULL PTR!");
        return VOS_ERR;
    }

    if (uslength == 0)
    {
        AT_WARN_LOG("AT_RcvFromAppSock: uslength is 0!");
        return VOS_ERR;
    }

    if (ucPortNo != AT_APP_SOCK_PORT_NO)
    {
        AT_WARN_LOG("AT_RcvFromAppSock: Port No ERR!");
        return VOS_ERR;
    }

    if ( (gastAtClientTab[ucIndex].UserType != AT_APP_SOCK_USER)
       ||(gastAtClientTab[ucIndex].ucUsed == AT_CLIENT_NULL))
    {
        AT_WARN_LOG("AT_RcvFromAppSock: SOCK client is unused!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
    {
        ulRet = At_CmdStreamPreProc(ucIndex,pData,uslength);
    }
    else
    {
        ulRet = At_DataStreamPreProc(ucIndex,gastAtClientTab[ucIndex].DataMode,pData,uslength);
    }

    if ( ulRet == AT_SUCCESS )
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_INT32 AT_AppSockComEst(VOS_UINT8 ucPortNo)
{
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_APPSOCK_INDEX;

    if (ucPortNo != AT_APP_SOCK_PORT_NO)
    {
         AT_WARN_LOG("AT_E5SockComEst the PortNo is error)");
        return VOS_ERR;
    }

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_APPSOCK, ID_AT_MNTN_RESULT_APPSOCK);

    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    /* ��д�û����� */
    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_APPSOCK;
    gastAtClientTab[ucIndex].ucPortNo        = ucPortNo;
    gastAtClientTab[ucIndex].UserType        = AT_APP_SOCK_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    /*��DMSע��Ӵ����л�ȡ���ݵĻص�����*/
    (VOS_VOID)App_VcomRecvCallbackRegister(ucPortNo, (pComRecv)AT_RcvFromAppSock);


    return VOS_OK;
}


VOS_UINT32 At_UsbGetWwanMode(VOS_VOID)
{
    return WWAN_WCDMA;
}


VOS_VOID AT_UsbNcmConnStatusChgCB(NCM_IOCTL_CONNECT_STUS_E enStatus, VOS_VOID *pBuffer)
{
    AT_MSG_STRU                        *pstMsg = VOS_NULL_PTR;
    errno_t                             lMemResult;

    /*
     * ���������Ͽ��ڲ���Ϣ, �����������
     */

    if (enStatus == NCM_IOCTL_STUS_BREAK)
    {
        /* ����OSA��Ϣ */
        /*lint -save -e516 */
        pstMsg = (AT_MSG_STRU *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_MSG_STRU));
        /*lint -restore */
        if (pstMsg == VOS_NULL_PTR)
        {
            AT_ERR_LOG("AT_UsbNcmConnStatusChgCB: Alloc message failed!");
            return;
        }

        /* �����Ϣ���� */
        lMemResult = memset_s(AT_GET_MSG_ENTITY(pstMsg), AT_GET_MSG_LENGTH(pstMsg), 0x00, AT_GET_MSG_LENGTH(pstMsg));
        TAF_MEM_CHK_RTN_VAL(lMemResult, AT_GET_MSG_LENGTH(pstMsg), AT_GET_MSG_LENGTH(pstMsg));

        /* ��д��Ϣͷ */
        AT_CFG_INTRA_MSG_HDR(pstMsg, ID_AT_NCM_CONN_STATUS_CMD);

        /* ��д��Ϣ���� */
        pstMsg->ucType  = AT_NCM_CONN_STATUS_MSG;
        pstMsg->ucIndex = AT_CLIENT_TAB_NDIS_INDEX;

        /* ������Ϣ */
        AT_SEND_MSG(pstMsg);
    }

    return;
}


VOS_INT AT_RcvFromNdisCom(
    VOS_UINT8                           *pucData,
    VOS_UINT16                          uslength
)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulRet;

    /* ������� */
    if (pucData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_RcvFromNdisCom: pData is NULL PTR!");
        return VOS_ERR;
    }

    /* ������� */
    if (uslength == 0)
    {
        AT_WARN_LOG("At_RcvFromNdisCom: uslength is 0!");
        return VOS_ERR;
    }

    ucIndex = AT_CLIENT_TAB_NDIS_INDEX;

    /* NDIS��·û�н��� */
    if ( (gastAtClientTab[ucIndex].UserType != AT_NDIS_USER)
       ||(gastAtClientTab[ucIndex].ucUsed == AT_CLIENT_NULL))
    {
        AT_WARN_LOG("At_RcvFromNdisCom: NDIS is unused");
        return VOS_ERR;
    }

    /*����NDISͨ��״̬Ϊ���ϱ�����*/
    DMS_SetNdisChanStatus(ACM_EVT_DEV_READY);

    if (gastAtClientTab[ucIndex].Mode == AT_CMD_MODE)
    {
        ulRet = At_CmdStreamPreProc(ucIndex,pucData,uslength);
    }
    else
    {
        ulRet = At_DataStreamPreProc(ucIndex,gastAtClientTab[ucIndex].DataMode,pucData,uslength);
    }

    if ( ulRet == AT_SUCCESS )
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_UINT32 AT_UsbNdisEst(VOS_VOID)
{
    VOS_UINT8                           ucIndex;

    ucIndex = AT_CLIENT_TAB_NDIS_INDEX;

    /* ��ն�Ӧ���� */
    memset_s(&gastAtClientTab[ucIndex], sizeof(AT_CLIENT_MANAGE_STRU), 0x00, sizeof(AT_CLIENT_MANAGE_STRU));

    AT_ConfigTraceMsg(ucIndex, ID_AT_CMD_NDIS, ID_AT_MNTN_RESULT_NDIS);

    /* ��д�û����� */
    gastAtClientTab[ucIndex].usClientId      = AT_CLIENT_ID_NDIS;
    gastAtClientTab[ucIndex].ucPortNo        = AT_NDIS_PORT_NO;
    gastAtClientTab[ucIndex].UserType        = AT_NDIS_USER;
    gastAtClientTab[ucIndex].ucUsed          = AT_CLIENT_USED;

    /* ���¿��Բ�����д��ǰ��PS_MEMSET�Ѿ���ʼ����ֻΪ�ɿ���� */
    gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
    gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
    gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
    gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;

    return VOS_OK;
}


VOS_VOID AT_OpenUsbNdis(VOS_VOID)
{
    UDI_OPEN_PARAM_S                    stParam;
    VOS_UINT32                          ulRst;

    stParam.devid   = UDI_NCM_NDIS_ID;

    /* ��Device�����ID */
    g_ulAtUdiNdisHdl = mdrv_udi_open(&stParam);

    if (g_ulAtUdiNdisHdl == UDI_INVALID_HANDLE)
    {
        AT_ERR_LOG("AT_OpenUsbNdis, ERROR, Open usb ndis device failed!");

        return;
    }

    /* ע��DMS�ص�����ָ�� */
    /*lint -e732   ���Ͳ�ͳһ����ʱע����ȷ�Ͻӿ��ٽ��*/
    ulRst =  DMS_USB_NAS_REGFUNC((USBNdisStusChgFunc)AT_UsbNcmConnStatusChgCB,
                                 (USB_NAS_AT_CMD_RECV)AT_RcvFromNdisCom,
                                 (USB_NAS_GET_WWAN_MODE)At_UsbGetWwanMode);
    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG("AT_OpenUsbNdis, ERROR, Reg NCM failed!");

        return;
    }
    /*lint +e732*/

    return;
}


VOS_VOID AT_CloseUsbNdis(VOS_VOID)
{
    /* �Ͽ�NDIS�������� */
    AT_UsbNcmConnStatusChgCB(NCM_IOCTL_STUS_BREAK, VOS_NULL_PTR);

    if (g_ulAtUdiNdisHdl != UDI_INVALID_HANDLE)
    {
        mdrv_udi_close(g_ulAtUdiNdisHdl);

        g_ulAtUdiNdisHdl = UDI_INVALID_HANDLE;
    }

    return;
}


VOS_UINT32 AT_CheckPcuiCtrlConcurrent(
    VOS_UINT8                           ucIndexOne,
    VOS_UINT8                           ucIndexTwo
)
{
    VOS_UINT32                          ulUserFlg1;
    VOS_UINT32                          ulUserFlg2;

    ulUserFlg1 = AT_CheckUserType(ucIndexOne, AT_USBCOM_USER);
    ulUserFlg1 |= AT_CheckUserType(ucIndexOne, AT_CTR_USER);
    ulUserFlg1 |= AT_CheckUserType(ucIndexOne, AT_PCUI2_USER);

    if (ulUserFlg1 != VOS_TRUE)
    {
        return VOS_FALSE;
    }

    ulUserFlg2 = AT_CheckUserType(ucIndexTwo, AT_USBCOM_USER);
    ulUserFlg2 |= AT_CheckUserType(ucIndexTwo, AT_CTR_USER);
    ulUserFlg2 |= AT_CheckUserType(ucIndexTwo, AT_PCUI2_USER);

    if (ulUserFlg2 != VOS_TRUE)
    {
        return VOS_FALSE;
    }

    if (gastAtClientTab[ucIndexOne].UserType != gastAtClientTab[ucIndexTwo].UserType)
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 AT_IsConcurrentPorts(
    VOS_UINT8                           ucIndexOne,
    VOS_UINT8                           ucIndexTwo
)
{
    VOS_UINT32                          ulAppUserFlg1;
    VOS_UINT32                          ulAppUserFlg2;

    /* ͬһ��ͨ����֧�ֲ���������㺯����֤ */
    ulAppUserFlg1  = AT_CheckAppUser(ucIndexOne);
    ulAppUserFlg2  = AT_CheckAppUser(ucIndexTwo);

    /* ͨ��1��APPͨ��,ͨ��2Ҳ��APPͨ�� */
    if (ulAppUserFlg1 == VOS_TRUE)
    {
        if (ulAppUserFlg2 == VOS_TRUE)
        {
            return VOS_TRUE;
        }
    }

    /* PCUI��CTRL�ڲ����жϣ����������� */
    if (AT_GetPcuiCtrlConcurrentFlag() == VOS_TRUE)
    {
        if (AT_CheckPcuiCtrlConcurrent(ucIndexOne, ucIndexTwo) == VOS_TRUE)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

/* Added by l60609 for AP������Ŀ ��2012-09-10 Begin */

VOS_UINT32 AT_CheckAppUser(VOS_UINT8 ucIndex)
{
    if (gastAtClientTab[ucIndex].UserType != AT_APP_USER)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;

}


VOS_UINT32 AT_CheckNdisUser(VOS_UINT8 ucIndex)
{
    if (gastAtClientTab[ucIndex].UserType != AT_NDIS_USER)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;

}


VOS_UINT32 AT_CheckHsUartUser(VOS_UINT8 ucIndex)
{
#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (gastAtClientTab[ucIndex].UserType == AT_HSUART_USER)
    {
        return VOS_TRUE;
    }
#endif

    return VOS_FALSE;
}


VOS_UINT32 AT_CheckModemUser(VOS_UINT8 ucIndex)
{
    if (gastAtClientTab[ucIndex].UserType != AT_MODEM_USER)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32 AT_CheckUserType(
    VOS_UINT8                               ucIndex,
    AT_USER_TYPE                            enUserType
)
{
    if (enUserType != gastAtClientTab[ucIndex].UserType)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_VOID AT_InitFcMap(VOS_VOID)
{
    VOS_UINT8       ucLoop;

    /* ��ʼ��g_stFcIdMaptoFcPri */
    for (ucLoop = 0; ucLoop < FC_ID_BUTT; ucLoop++)
    {
        g_stFcIdMaptoFcPri[ucLoop].ulUsed  = VOS_FALSE;
        g_stFcIdMaptoFcPri[ucLoop].enFcPri = FC_PRI_BUTT;
        g_stFcIdMaptoFcPri[ucLoop].ulRabIdMask  = 0;
        g_stFcIdMaptoFcPri[ucLoop].enModemId    = MODEM_ID_BUTT;
    }
}


VOS_UINT32 AT_SendDiagCmdFromOm(
    VOS_UINT8                           ucPortNo,
    VOS_UINT8                           ucType,
    VOS_UINT8                          *pData,
    VOS_UINT16                          uslength
)
{
    return VOS_OK;
}


VOS_UINT32 AT_SendPcuiDataFromOm(
    VOS_UINT8                          *pucVirAddr,
    VOS_UINT8                          *pucPhyAddr,
    VOS_UINT32                          ulLength
)
{
    if (At_SendData(AT_CLIENT_TAB_PCUI_INDEX,
                                  gastAtClientTab[AT_CLIENT_TAB_PCUI_INDEX].DataMode,
                                  pucVirAddr,
                                  (VOS_UINT16)ulLength) != AT_SUCCESS)
    {
        return VOS_ERR;
    }
    else
    {
        return VOS_OK;
    }
}


VOS_UINT32 AT_SendCtrlDataFromOm(
    VOS_UINT8                          *pucVirAddr,
    VOS_UINT8                          *pucPhyAddr,
    VOS_UINT32                          ulLength
)
{
    if (At_SendData(AT_CLIENT_TAB_CTRL_INDEX,
                                  gastAtClientTab[AT_CLIENT_TAB_CTRL_INDEX].DataMode,
                                  pucVirAddr,
                                  (VOS_UINT16)ulLength) != AT_SUCCESS)
    {
        return VOS_ERR;
    }
    else
    {
        return VOS_OK;
    }
}


VOS_UINT32 AT_SendPcui2DataFromOm(
    VOS_UINT8                          *pucVirAddr,
    VOS_UINT8                          *pucPhyAddr,
    VOS_UINT32                          ulLength
)
{
    if (At_SendData(AT_CLIENT_TAB_PCUI2_INDEX,
                                  gastAtClientTab[AT_CLIENT_TAB_PCUI2_INDEX].DataMode,
                                  pucVirAddr,
                                  (VOS_UINT16)ulLength) != AT_SUCCESS)
    {
        return VOS_ERR;
    }
    else
    {
        return VOS_OK;
    }
}



CBTCPM_SEND_FUNC AT_QuerySndFunc(AT_PHY_PORT_ENUM_UINT32 ulPhyPort)
{
    switch(ulPhyPort)
    {
        case AT_UART_PORT:
            return AT_UART_SendRawDataFromOm;

        case AT_PCUI_PORT:
            return AT_SendPcuiDataFromOm;

        case AT_CTRL_PORT:
            return AT_SendCtrlDataFromOm;

#if (FEATURE_AT_HSUART == FEATURE_ON)
        case AT_HSUART_PORT:
            return AT_HSUART_SendRawDataFromOm;
#endif

        default:
            AT_WARN_LOG("AT_QuerySndFunc: don't proc data of this port!");
            return VOS_NULL_PTR;
    }
}


TAF_UINT32 At_SendCmdMsg (TAF_UINT8 ucIndex,TAF_UINT8* pData, TAF_UINT16 usLen,TAF_UINT8 ucType)
{
    AT_MSG_STRU                        *pMsg = TAF_NULL_PTR;
    AT_CMD_MSG_NUM_CTRL_STRU           *pstMsgNumCtrlCtx = VOS_NULL_PTR;
    VOS_UINT_PTR                        ulTmpAddr;
    VOS_UINT32                          ulLength;
    VOS_ULONG                           ulLockLevel;
    errno_t                             lMemResult;
    MODEM_ID_ENUM_UINT16                enModemId;
    pstMsgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();

    if (pData == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendCmdMsg :pData is null ptr!");
        return AT_FAILURE;
    }

    if (usLen == 0)
    {
        AT_WARN_LOG("At_SendCmdMsg ulLength = 0");
        return AT_FAILURE;
    }

    if (usLen > AT_COM_BUFF_LEN)
    {
        AT_WARN_LOG("At_SendCmdMsg ulLength > AT_COM_BUFF_LEN");
        return AT_FAILURE;
    }

    /* �����Զ����ITEM����4���ֽ� */
    AT_GetAtMsgStruMsgLength(usLen, &ulLength);
    pMsg = (AT_MSG_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    if ( pMsg == TAF_NULL_PTR )
    {
        AT_ERR_LOG("At_SendCmdMsg:ERROR:Alloc Msg");
        return AT_FAILURE;
    }

    if (ucType == AT_NORMAL_TYPE_MSG)
    {
        if (pstMsgNumCtrlCtx->ulMsgCount > AT_MAX_MSG_NUM)
        {
            /*lint -save -e516 */
            /* �ͷŷ�����ڴ�ռ� */
            PS_FREE_MSG(WUEPS_PID_AT, pMsg);
            /*lint -restore */

            return AT_FAILURE;
        }

        /*lint -e571*/
        VOS_SpinLockIntLock(&(pstMsgNumCtrlCtx->stSpinLock), ulLockLevel);
        /*lint +e571*/

        pstMsgNumCtrlCtx->ulMsgCount++;

        VOS_SpinUnlockIntUnlock(&(pstMsgNumCtrlCtx->stSpinLock), ulLockLevel);
    }

    /* �������ػ����ʵ��������pMsg->aucValue;*/
    pMsg->ulReceiverCpuId   = VOS_LOCAL_CPUID;
    pMsg->ulSenderPid       = WUEPS_PID_AT;
    pMsg->ulReceiverPid     = WUEPS_PID_AT;

    if (ucType == AT_COMBIN_BLOCK_MSG)
    {
        pMsg->enMsgId = ID_AT_COMBIN_BLOCK_CMD;
    }
    else
    {
        pMsg->enMsgId = AT_GetCmdMsgID(ucIndex);
    }

    pMsg->ucType            = ucType;     /* ���� */
    pMsg->ucIndex           = ucIndex;    /* ���� */
    pMsg->usLen             = usLen;    /* ���� */

    enModemId               = MODEM_ID_0;
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        enModemId = MODEM_ID_0;
    }

    pMsg->enModemId     = (VOS_UINT8)enModemId;
    /* �汾��Ϣ*/
    pMsg->enVersionId   = 0xAA;
    pMsg->ucFilterAtType  = (VOS_UINT8)g_enLogPrivacyAtCmd;

    memset_s(pMsg->aucValue, sizeof(pMsg->aucValue), 0x00, sizeof(pMsg->aucValue));
    AT_GetUserTypeFromIndex(ucIndex, &pMsg->ucUserType);


    /* ��д����Ϣ���� */
    ulTmpAddr = (VOS_UINT_PTR)(pMsg->aucValue);
    lMemResult = memcpy_s((VOS_VOID*)ulTmpAddr, usLen, pData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen, usLen);

    /*������Ϣ��AT_PID;*/
    if ( PS_SEND_MSG( WUEPS_PID_AT, pMsg ) != 0 )
    {
        AT_ERR_LOG("At_SendCmdMsg:ERROR:VOS_SendMsg");

        /* ������Ϣ����ʧ��ʱ���ᴥ��������λ���ʴ˴�����ulMsgCount--���� */

        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32 AT_IsApPort(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulVcomUserFlg;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    if (g_stAtDebugInfo.ucUnCheckApPortFlg == 0)
    {
        /* ��ʼ�� */
        pucSystemAppConfig  = AT_GetSystemAppConfigAddr();
        ulVcomUserFlg       = AT_CheckAppUser(ucIndex);

        if (*pucSystemAppConfig == SYSTEM_APP_ANDROID)
        {
            /* ������ֻ���̬����Ҫ�ж�HSIC�˿ڣ�MUX�˿ڣ�VCOM�˿� */
            if (ulVcomUserFlg == VOS_FALSE)
            {
                return VOS_FALSE;
            }
        }
        else
        {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}


VOS_INT AT_ProcCCpuResetBefore(VOS_VOID)
{
    AT_MSG_STRU                        *pstMsg = VOS_NULL_PTR;

    AT_PR_LOGI("enter, %u", VOS_GetSlice());

    /* ���ô��ڸ�λǰ�ı�־ */
    AT_SetResetFlag(VOS_TRUE);

    DMS_InitModemStatus();

    /* ���TAFAGENT���е��ź��� */
    TAF_AGENT_ClearAllSem();

    /* ������Ϣ */
    /*lint -save -e516 */
    pstMsg = (AT_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                                         sizeof(AT_MSG_STRU));
    /*lint -restore */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_PR_LOGE("alloc msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* ��ʼ����Ϣ */
    TAF_MEM_SET_S((VOS_CHAR *)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
               0x00,
               (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��д��Ϣͷ */
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = WUEPS_PID_AT;
    pstMsg->ucType                      = ID_CCPU_AT_RESET_START_IND;

    pstMsg->enMsgId                     = ID_AT_COMM_CCPU_RESET_START;

    /* ����Ϣ */
    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_PR_LOGE("send msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* �ȴ��ظ��ź�����ʼΪ��״̬���ȴ���Ϣ��������ź��������� */
    if (VOS_SmP(AT_GetResetSem(), AT_RESET_TIMEOUT_LEN) != VOS_OK)
    {
        AT_PR_LOGE("VOS_SmP fail, %u", VOS_GetSlice());
        AT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        return VOS_ERROR;
    }

    /* ��¼��λǰ�Ĵ��� */
    AT_DBG_SAVE_CCPU_RESET_BEFORE_NUM(1);

    AT_PR_LOGI("succ, %u", VOS_GetSlice());

    return VOS_OK;
}


VOS_INT AT_ProcCCpuResetAfter(VOS_VOID)
{
    AT_MSG_STRU                        *pstMsg = VOS_NULL_PTR;

    AT_PR_LOGI("enter, %u", VOS_GetSlice());

    /* ������Ϣ */
    /*lint -save -e516 */
    pstMsg = (AT_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                                         sizeof(AT_MSG_STRU));
    /*lint -restore */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_PR_LOGE("alloc msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* ��ʼ����Ϣ */
    TAF_MEM_SET_S((VOS_CHAR *)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
               0x00,
               (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��д��Ϣͷ */
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = WUEPS_PID_AT;
    pstMsg->ucType                      = ID_CCPU_AT_RESET_END_IND;

    pstMsg->enMsgId                     = ID_AT_COMM_CCPU_RESET_END;

    /* ����Ϣ */
    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_PR_LOGE("send msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* ��¼��λ��Ĵ��� */
    AT_DBG_SAVE_CCPU_RESET_AFTER_NUM(1);

    AT_PR_LOGI("succ, %u", VOS_GetSlice());

    return VOS_OK;
}


VOS_INT AT_CCpuResetCallback(
    DRV_RESET_CB_MOMENT_E               enParam,
    VOS_INT                             iUserData
)
{
    /* ��λǰ */
    if (enParam == MDRV_RESET_CB_BEFORE)
    {
        return AT_ProcCCpuResetBefore();
    }
    /* ��λ�� */
    else if (enParam == MDRV_RESET_CB_AFTER)
    {
        return AT_ProcCCpuResetAfter();
    }
    else
    {
        return VOS_ERROR;
    }
}


VOS_INT AT_HifiResetCallback(
    enum DRV_RESET_CALLCBFUN_MOMENT     enParam,
    VOS_INT                             iUserData
)
{
    AT_MSG_STRU                        *pstMsg = VOS_NULL_PTR;

    /* ����Ϊ0��ʾ��λǰ���� */
    if (enParam == DRV_RESET_CALLCBFUN_RESET_BEFORE)
    {
        AT_PR_LOGI("before reset enter, %u", VOS_GetSlice());
        /* ������Ϣ */
        /*lint -save -e516 */
        pstMsg = (AT_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                                             sizeof(AT_MSG_STRU));
        /*lint -restore */
        if (pstMsg == VOS_NULL_PTR)
        {
            AT_PR_LOGE("before reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        /* ��ʼ����Ϣ */
        TAF_MEM_SET_S((VOS_CHAR *)pstMsg + VOS_MSG_HEAD_LENGTH,
                   (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
                   0x00,
                   (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

        /* ��д��Ϣͷ */
        pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
        pstMsg->ulReceiverPid               = WUEPS_PID_AT;
        pstMsg->ucType                      = ID_HIFI_AT_RESET_START_IND;

        pstMsg->enMsgId                     = ID_AT_COMM_HIFI_RESET_START;

        /* ����Ϣ */
        if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
        {
            AT_PR_LOGE("after reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        return VOS_OK;
    }
    /* ��λ�� */
    else if (enParam == DRV_RESET_CALLCBFUN_RESET_AFTER)
    {
        AT_PR_LOGI("after reset enter, %u", VOS_GetSlice());
        /* ������Ϣ */
        /*lint -save -e516 */
        pstMsg = (AT_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                                             sizeof(AT_MSG_STRU));
        /*lint -restore */
        if (pstMsg == VOS_NULL_PTR)
        {
            AT_PR_LOGE("after reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        /* ��ʼ����Ϣ */
        TAF_MEM_SET_S((VOS_CHAR *)pstMsg + VOS_MSG_HEAD_LENGTH,
                   (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
                   0x00,
                   (VOS_SIZE_T)(sizeof(AT_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

        /* ��д��Ϣͷ */
        pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
        pstMsg->ulReceiverPid               = WUEPS_PID_AT;
        pstMsg->ucType                      = ID_HIFI_AT_RESET_END_IND;

        pstMsg->enMsgId                     = ID_AT_COMM_HIFI_RESET_END;

        /* ����Ϣ */
        if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
        {
            AT_PR_LOGE("after reset send msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }
        return VOS_OK;
    }
    else
    {
        return VOS_ERROR;
    }
}



VOS_VOID AT_ModemeEnableCB(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulEnable
)
{
    /* �豸Ĭ�ϴ�����Ч״̬�������ݾ�ͨ�����ص����գ�
    �� �豸ʧЧʱ�����ݵ�ǰ״̬��֪ͨPPP���紦������̬��
       ��֪ͨATȥ����PDP.
    */
    if (ulEnable == PS_FALSE)
    {
        if (gastAtClientTab[ucIndex].DataMode == AT_PPP_DATA_MODE)
        {
            PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId,
                                   PPP_AT_CTRL_REL_PPP_REQ);

            /* ��ԭ�ȿ��������أ�����ֹͣ���� */
            if ((gastAtClientTab[ucIndex].ModemStatus & IO_CTRL_CTS) == 0)
            {
                AT_StopFlowCtrl(ucIndex);
            }

            /* �Ͽ����� */
            if (TAF_PS_CallEnd(WUEPS_PID_AT,
                                         AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                         0,
                                         gastAtClientTab[ucIndex].ucCid) != VOS_OK)
            {
                AT_ERR_LOG("AT_ModemeEnableCB: TAF_PS_CallEnd failed in <AT_PPP_DATA_MODE>!");
                return;
            }
        }
        else if (gastAtClientTab[ucIndex].DataMode == AT_IP_DATA_MODE)
        {
            PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId,
                                   PPP_AT_CTRL_REL_PPP_RAW_REQ);

            /* ��ԭ�ȿ��������أ�����ֹͣ���� */
            if ((gastAtClientTab[ucIndex].ModemStatus & IO_CTRL_CTS) == 0)
            {
                AT_StopFlowCtrl(ucIndex);
            }

            /* �Ͽ����� */
            if ( TAF_PS_CallEnd(WUEPS_PID_AT,
                                          AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                          0,
                                          gastAtClientTab[ucIndex].ucCid) != VOS_OK )
            {
                AT_ERR_LOG("AT_ModemeEnableCB: TAF_PS_CallEnd failed in <AT_IP_DATA_MODE>!");
                return;
            }
        }
        else
        {
            /* �յ�else��֧������PCLINT���� */
        }

        /* ��PPP����HDLCȥʹ�ܲ��� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId,
                               PPP_AT_CTRL_HDLC_DISABLE);

        /* ֹͣ��ʱ�� */
        AT_StopRelTimer(ucIndex, &gastAtClientTab[ucIndex].hTimer);

        /* �ܽ��ź��޸ĺ�At_ModemRelInd����ֻ������USB���γ���ʱ����ã�
           Ϊ�˴ﵽModem��always-on��Ŀ�ģ���ʱ��Ҫ����AT��·��״̬Ǩ��
           ������������״̬:
        */
        gastAtClientTab[ucIndex].Mode            = AT_CMD_MODE;
        gastAtClientTab[ucIndex].IndMode         = AT_IND_MODE;
        gastAtClientTab[ucIndex].DataMode        = AT_DATA_BUTT_MODE;
        gastAtClientTab[ucIndex].DataState       = AT_DATA_STOP_STATE;
        gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;
    }
}





