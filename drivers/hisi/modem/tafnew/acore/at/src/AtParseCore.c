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

#include "AtCheckFunc.h"
#include "ATCmdProc.h"
#include "AtCmdMsgProc.h"
#include "AtInputProc.h"

#include "at_common.h"

#include "AcpuReset.h"

#include "AtInternalMsg.h"

#include "AtParseCore.h"

#include "AtDataProc.h"
#include "securec.h"



#define    THIS_FILE_ID        PS_FILE_ID_AT_PARSECORE_C


/* �����ͷ��� */
HI_LIST_S g_stCmdTblList = {0};

/* �������̲��� */
AT_PARSECMD_STRU g_stATParseCmd;

/* �����б� */
VOS_UINT8 gucAtParaIndex = 0;

AT_PARSE_PARA_TYPE_STRU gastAtParaList[AT_MAX_PARA_NUMBER];

/* ÿ���ͻ��˵Ľ�����Ϣ */
AT_PARSE_CONTEXT_STRU g_stParseContext[AT_MAX_CLIENT_NUM];

/* ����SMS��������ͨ�����ʽ��һ�£���������SMS�������Ӧ������ */
/* �����������֧�ֶ�ͨ����������������������
   ��g_stCmdElement=> g_stCmdElement[AT_MAX_CLIENT_NUM] */
AT_PAR_CMD_ELEMENT_STRU g_stCmdElement[AT_MAX_CLIENT_NUM];




VOS_VOID At_ParseInit(VOS_VOID)
{
    VOS_UINT32 i = 0;

    memset_s(&g_stATParseCmd, sizeof(g_stATParseCmd), 0x00, sizeof(g_stATParseCmd));
    memset_s(g_stParseContext, sizeof(g_stParseContext), 0x00, sizeof(g_stParseContext));
    memset_s(g_stCmdElement, sizeof(g_stCmdElement), 0x00, sizeof(g_stCmdElement));

    /*lint -e717*/

    HI_INIT_LIST_HEAD(&g_stCmdTblList);

    for(i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        HI_INIT_LIST_HEAD(&(g_stParseContext[i].stCombineCmdInfo.stCombineCmdList));
    }

    /*lint -e717*/

    return;
}
/*lint -save -e429*/

VOS_UINT32 AT_RegisterCmdTable(const AT_PAR_CMD_ELEMENT_STRU* pstCmdTblEntry, VOS_UINT16 usCmdNum)
{
    HI_LIST_S* pCmdTblHeader = NULL;
    AT_PAR_CMDTBL_LIST_STRU* pstCmdTblNode = NULL;/*lint !e830*/

    /* �����Ϊ�գ������������Ϊ0 */
    if((pstCmdTblEntry == NULL) || (usCmdNum == 0))
    {
        return ERR_MSP_FAILURE;
    }

    pCmdTblHeader = &g_stCmdTblList;

    /* ���������errcodetbl��ӵ������� */
    pstCmdTblNode = (AT_PAR_CMDTBL_LIST_STRU *)AT_MALLOC(sizeof(AT_PAR_CMDTBL_LIST_STRU));
    if(pstCmdTblNode == NULL)
    {
        return ERR_MSP_MALLOC_FAILUE;
    }

    memset_s(pstCmdTblNode, (VOS_SIZE_T)sizeof(AT_PAR_CMDTBL_LIST_STRU), 0x00, (VOS_SIZE_T)sizeof(AT_PAR_CMDTBL_LIST_STRU));
    pstCmdTblNode->usCmdNum = usCmdNum;

    pstCmdTblNode->pstCmdElement = (AT_PAR_CMD_ELEMENT_STRU*)pstCmdTblEntry;

    msp_list_add_tail((&pstCmdTblNode->stCmdTblList), pCmdTblHeader);

    return ERR_MSP_SUCCESS;
}
/*lint -restore */


VOS_VOID AT_ClacCmdProc(VOS_VOID)
{
    VOS_UINT16 i = 0;
    AT_PAR_CMD_ELEMENT_STRU *pstCmdElement    = VOS_NULL_PTR;
    VOS_UINT32               ulCmdNum;

    pstCmdElement       = At_GetBasicCmdTable();
    ulCmdNum            = At_GetBasicCmdNum();

    for(i = 0; i < ulCmdNum; i++)
    {
        /* �ж��Ƿ���Ҫ��ʾ */
        if((pstCmdElement[i].ulChkFlag & CMD_TBL_CLAC_IS_INVISIBLE) == 0)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                              "%s%c%c", pstCmdElement[i].pszCmdName, ucAtS3, ucAtS4);
        }
    }

    for(i = 0; i < gusAtSmsCmdNum; i++)
    {
        /* �ж��Ƿ���Ҫ��ʾ */
        if((gastAtSmsCmdTab[i].ulChkFlag & CMD_TBL_CLAC_IS_INVISIBLE) == 0)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                              "%s%c%c", gastAtSmsCmdTab[i].pszCmdName, ucAtS3, ucAtS4);
        }
    }

    pstCmdElement       = At_GetExtendCmdTable();
    ulCmdNum            = At_GetExtendCmdNum();

    for(i = 0; i < ulCmdNum; i++)
    {
        /* �ж��Ƿ���Ҫ��ʾ */
        if((pstCmdElement[i].ulChkFlag & CMD_TBL_CLAC_IS_INVISIBLE) == 0)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                              "%s%c%c", pstCmdElement[i].pszCmdName, ucAtS3, ucAtS4);
        }
    }

    return ;
}



VOS_UINT8 AT_BlockCmdCheck(VOS_VOID)
{
    VOS_UINT8 i = 0;
    VOS_UINT8 ucBlockid = AT_MAX_CLIENT_NUM;

    /* ��ѯ����ͨ��״̬ */
    for(i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        /* ���ĳͨ�����ڷ�ready̬����ֱ�ӷ��أ������� */
        if(g_stParseContext[i].ucClientStatus != AT_FW_CLIENT_STATUS_READY)
        {
            return AT_MAX_CLIENT_NUM;
        }

        if((g_stParseContext[i].usBlockCmdLen != 0) && (g_stParseContext[i].pBlockCmd != NULL))
        {
            ucBlockid = i;
        }
    }

    return ucBlockid;
}



VOS_VOID AT_ClearBlockCmdInfo(VOS_UINT8 ucIndex)
{
    if(g_stParseContext[ucIndex].pBlockCmd != NULL)
    {
        AT_FREE(g_stParseContext[ucIndex].pBlockCmd);
        g_stParseContext[ucIndex].pBlockCmd = NULL;
    }

    g_stParseContext[ucIndex].usBlockCmdLen = 0;

    return;
}



VOS_VOID AT_CheckProcBlockCmd(VOS_VOID)
{
    VOS_UINT8 ucIndex;
    VOS_UINT32 ulRet;


    /* �ж��Ƿ��л�������� */
    ucIndex = AT_BlockCmdCheck();
    if(ucIndex < AT_MAX_CLIENT_NUM)
    {
        /* ������Ϣ���������� */
        ulRet = At_SendCmdMsg(ucIndex, &ucIndex, sizeof(ucIndex), AT_COMBIN_BLOCK_MSG);
        if(ulRet != ERR_MSP_SUCCESS)
        {
            /* �ɶ�ʱ����ʱȥ���� */
        }
    }

    return ;
}



VOS_VOID AT_ParseCmdOver(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulRet = ERR_MSP_FAILURE;
    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;
    AT_FW_COMBINE_CMD_INFO_STRU* pstCombineCmdInfo = NULL;

    if(ucIndex >= AT_MAX_CLIENT_NUM)
    {
        return;
    }

    pClientContext = &(g_stParseContext[ucIndex]);

    pstCombineCmdInfo = &pClientContext->stCombineCmdInfo;

    if(pstCombineCmdInfo->usProcNum < pstCombineCmdInfo->usTotalNum)
    {
        pstCombineCmdInfo->usProcNum++;
    }
    else    /* ���ⱻ����� */
    {
        pstCombineCmdInfo->usProcNum = pstCombineCmdInfo->usTotalNum;
    }

    /* �ж���������Ƿ������ */
    if((pstCombineCmdInfo->usProcNum < pstCombineCmdInfo->usTotalNum))
    {
        /* ������Ϣ������һ������ */
        ulRet = At_SendCmdMsg(ucIndex, &ucIndex, sizeof(ucIndex), AT_COMBIN_BLOCK_MSG);
        if(ulRet != ERR_MSP_SUCCESS)
        {
            /* ��Ϣ����ʧ�ָܻ�������״̬ */
            pClientContext->ucClientStatus = AT_FW_CLIENT_STATUS_READY;
        }

        return ;
    }

    return;
}



VOS_VOID AT_BlockCmdTimeOutProc(
    REL_TIMER_MSG                      *pstMsg
)
{
    VOS_UINT8                           ucIndex;

    ucIndex = (VOS_UINT8)((pstMsg->ulName)>>12);

    gstAtSendData.usBufLen = 0;

    At_FormatResultData(ucIndex, AT_ERROR);

    AT_ClearBlockCmdInfo(ucIndex);

    return ;
}



VOS_VOID AT_PendClientProc(VOS_UINT8 ucIndex, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    if ((usLen == 4)
        && (At_UpChar(pData[0]) == 'S') && (At_UpChar(pData[1]) == 'T')
        && (At_UpChar(pData[2]) == 'O') && (At_UpChar(pData[3]) == 'P'))
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_ResetCombinParseInfo(ucIndex);
    }
    else if(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_A_SET)
    {
        if((At_UpChar(pData[0]) != 'A') || (At_UpChar(pData[1]) != 'T')
            || (At_UpChar(pData[2]) != 'H'))
        {
        }
        else
        {
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_ResetCombinParseInfo(ucIndex);
        }
    }

    else if ( AT_IsAbortCmdStr(ucIndex, pData, usLen) == VOS_TRUE )
    {
        /* AT��ϵ�ǰPEND״̬�Ĵ��� */
        AT_AbortCmdProc(ucIndex);
    }

    else
    {
    }

    return ;
}



VOS_VOID AT_HoldBlockCmd(VOS_UINT8 ucIndex, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulTimerName;
    VOS_UINT32                          ulTempIndex;
    AT_PARSE_CONTEXT_STRU*              pstClientContext = NULL;

    pstClientContext = &(g_stParseContext[ucIndex]);

    /* ֻ֧�ֻ���һ�����ݣ������µ�AT�����ַ����򸲸Ǳ�ͨ��֮ǰ�����������Ϣ */
    if(pstClientContext->pBlockCmd != NULL)
    {
        AT_FREE(pstClientContext->pBlockCmd);
    }

    pstClientContext->pBlockCmd = (VOS_UINT8*)AT_MALLOC(usLen);
    if(pstClientContext->pBlockCmd == NULL)
    {
        pstClientContext->usBlockCmdLen = 0;
        return ;
    }

    lMemResult = memcpy_s(pstClientContext->pBlockCmd, usLen, pData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen, usLen);
    pstClientContext->usBlockCmdLen = usLen;

    ulTempIndex  = (VOS_UINT32)ucIndex;
    ulTimerName  = AT_HOLD_CMD_TIMER;
    ulTimerName |= AT_INTERNAL_PROCESS_TYPE;
    ulTimerName |= (ulTempIndex<<12);

    /* ������ʱ�� */
    VOS_StopRelTimer(&pstClientContext->hTimer);

    AT_StartRelTimer(&pstClientContext->hTimer, AT_HOLD_CMD_TIMER_LEN, ulTimerName, 0, VOS_RELTIMER_NOLOOP);

    return ;
}



VOS_UINT32 AT_ParseCmdIsComb(VOS_UINT8 ucIndex, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    AT_PARSE_CONTEXT_STRU* pstClientContext = NULL;

    pstClientContext = &(g_stParseContext[ucIndex]);

    /* ��ʾ��ͨ��������������ڴ��� */
    if(pstClientContext->stCombineCmdInfo.usProcNum < pstClientContext->stCombineCmdInfo.usTotalNum)
    {
        CmdStringFormat(ucIndex, pData, &usLen);

        AT_PendClientProc(ucIndex, pData, usLen);

        return ERR_MSP_FAILURE;
    }

    return ERR_MSP_SUCCESS;

}



VOS_UINT32 AT_ParseCmdIsPend(VOS_UINT8 ucIndex, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    VOS_UINT32 i = 0;
    AT_PARSE_CONTEXT_STRU* pstClientContext = NULL;

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        pstClientContext = &(g_stParseContext[i]);

        if (i == ucIndex)
        {
            /* ��ͨ������PEND״̬ʱ��Ҫ�ж��Ƿ���STOP��ATH */
            if (pstClientContext->ucClientStatus == AT_FW_CLIENT_STATUS_PEND)
            {
                CmdStringFormat(ucIndex, pData, &usLen);

                AT_PendClientProc(ucIndex, pData, usLen);
                return ERR_MSP_FAILURE;
            }
        }
        /* ����ͨ������PEND״̬�����״̬ */
        else if ((pstClientContext->ucClientStatus == AT_FW_CLIENT_STATUS_PEND)
              || (pstClientContext->ucMode == AT_SMS_MODE))
        {
            /* �жϱ�ͨ���Լ���������ͨ���Ƿ���Բ��� */
            if (AT_IsConcurrentPorts(ucIndex, (VOS_UINT8)i) == VOS_TRUE)
            {
                /* ����ǣ����������ͨ�� */
                continue;
            }
            else
            {
                /* ������ǣ��򻺴������ */
                AT_HoldBlockCmd(ucIndex, pData, usLen);
                return ERR_MSP_FAILURE;
            }
        }
        else
        {
        }
    }

    return ERR_MSP_SUCCESS;
}



VOS_VOID AT_DiscardInvalidCharForSms(TAF_UINT8* pData, TAF_UINT16 *pusLen)
{
    TAF_CHAR                            aucCMGSCmd[] = "AT+CMGS=";
    TAF_CHAR                            aucCMGWCmd[] = "AT+CMGW=";
    TAF_CHAR                            aucCMGCCmd[] = "AT+CMGC=";
    TAF_CHAR                            aucTmpCmd[9];
    VOS_INT                             iCMGSResult;
    VOS_INT                             iCMGWResult;
    VOS_INT                             iCMGCResult;
    errno_t                             lMemResult;

    if (*pusLen < VOS_StrLen(aucCMGSCmd))
    {
        return;
    }

    lMemResult = memcpy_s(aucTmpCmd, sizeof(aucTmpCmd), pData, sizeof(aucTmpCmd) - 1);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucTmpCmd), sizeof(aucTmpCmd) - 1);
    At_UpString((TAF_UINT8 *)aucTmpCmd, sizeof(aucTmpCmd) - 1);
    aucTmpCmd[8] = '\0';

    iCMGSResult = VOS_StrCmp(aucCMGSCmd, aucTmpCmd);
    iCMGWResult = VOS_StrCmp(aucCMGWCmd, aucTmpCmd);
    iCMGCResult = VOS_StrCmp(aucCMGCCmd, aucTmpCmd);
    if ((iCMGSResult != 0) && (iCMGWResult != 0)&&(iCMGCResult != 0))
    {
        return;
    }

    /*  MACϵͳ�ϵ�MP��̨����:AT+CMGS=**<CR><^z><Z>(��AT+CMGW=**<CR><^z><Z>)
       Ϊ�˹�ܸ����⣬��Ҫ�ڽ��յ�������ʽ��������
       ��Ҫ����������Ч�ַ�<^z><Z>ɾȥ  */
    if ((ucAtS3 == pData[*pusLen - 3]) && (pData[*pusLen - 2] == '\x1a')
        && ((pData[*pusLen - 1] == 'z') || (pData[*pusLen - 1] == 'Z')))
    {
        /* ɾȥ��β��<^z><Z>�ַ� */
        *pusLen -= 2;
    }
    /* MACϵͳSFR��̨����: AT+CMGS=<LENGTH><CR><LF><CR>*/
    else if ((ucAtS3 == pData[*pusLen - 3])
          && (ucAtS4 == pData[*pusLen - 2])
          && (ucAtS3 == pData[*pusLen - 1]))
    {
        /* ɾȥ��β��<LF><CR>�ַ� */
        *pusLen -= 2;
    }
    /* �ж�pData�����Ľ�β�Ƿ�Ϊ<CR><LF>��ʽ */
    else if ((ucAtS3 == pData[*pusLen - 2]) && (ucAtS4 == pData[*pusLen - 1]))
    {
        /* ɾȥ��β��<LF>�ַ� */
        *pusLen -= 1;
    }
    else
    {
    }

    return;
}




VOS_VOID AT_ResetParseVariable(VOS_VOID)
{
    memset_s(&g_stATParseCmd, sizeof(g_stATParseCmd), 0x00 , sizeof(AT_PARSECMD_STRU));

    memset_s(gastAtParaList, sizeof(gastAtParaList), 0x00 , sizeof(gastAtParaList));

    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_BUTT;
    g_stATParseCmd.ucCmdFmtType = AT_CMD_TYPE_BUTT;

    gucAtParaIndex = 0;
    return;
}



VOS_UINT32 AT_ParseCmdType( VOS_UINT8 * pData, VOS_UINT16 usLen)
{
    VOS_UINT16 usDataTmpLen = usLen - 2;
    VOS_UINT8* pDataTmp = pData + 2;
    VOS_UINT32 ulResult = 0;

    /* ���õĵط���֤pData��Ϊ�� */

    if(usLen < 2)
    {
        return AT_ERROR;
    }

    if(At_CheckCharA(*pData) == AT_SUCCESS)   /* �����'A'/'a' */
    {
        if(At_CheckCharT(*(pData+1)) == AT_SUCCESS)   /* �����'T'/'t' */
        {
            if(usLen == 2)
            {
                return AT_OK;
            }

            if(usLen == 3)
            {
                if(At_CheckSemicolon(pData[2]) == AT_SUCCESS)
                {
                    return AT_OK;
                }
            }

            AT_ResetParseVariable();    /* ���ý��������õ�ȫ�ֱ��� */

            pDataTmp = pData + 2;
            usDataTmpLen = usLen - 2;

            /* ��D�����Ҫȥ�������β��';' */
            if((At_CheckCharD(*pDataTmp)) != AT_SUCCESS && (*(pDataTmp+usDataTmpLen-1) == ';'))
            {
                usDataTmpLen--;
            }

            switch(*pDataTmp)      /* ���ݵ������ַ����зַ� */
            {
            case '^':   /* ��չ���� */
            case '+':   /* ��չ���� */
            case '$':   /* ��չ���� */
                return atParseExtendCmd(pDataTmp, usDataTmpLen);

            case 'd':
            case 'D':   /* D���� */

                return ParseDCmdPreProc(pDataTmp, usDataTmpLen);

            case 's':
            case 'S':   /* S���� */
                return atParseSCmd(pDataTmp, usDataTmpLen);

            default:    /*�������� */
                {
                    ulResult = atParseBasicCmd(pDataTmp, usDataTmpLen);

                    if(ulResult == AT_FAILURE)
                    {
                        ulResult = AT_ERROR;
                    }

                    return ulResult;
                }
            }
        }
    }

    return AT_ERROR;
}



VOS_UINT32 At_MatchSmsCmdName(VOS_UINT8 ucIndex, VOS_CHAR *pszCmdName)
{
    VOS_UINT32                          i = 0;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    for (i = 0; i < gusAtSmsCmdNum; i++)
    {
        if ((TAF_CHAR*)gastAtSmsCmdTab[i].pszCmdName != VOS_NULL_PTR)
        {
            if (AT_STRCMP(pszCmdName, (TAF_CHAR*)gastAtSmsCmdTab[i].pszCmdName) == ERR_MSP_SUCCESS)
            {
                if ( pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT)
                {
                    g_stCmdElement[ucIndex].pszParam = gastAtSmsCmdTab[i].ParaText;
                }
                else
                {
                    g_stCmdElement[ucIndex].pszParam = gastAtSmsCmdTab[i].ParaPDU;
                }

                g_stCmdElement[ucIndex].ulCmdIndex      = gastAtSmsCmdTab[i].ulCmdIndex;
                g_stCmdElement[ucIndex].pfnSetProc      = gastAtSmsCmdTab[i].pfnSetProc;
                g_stCmdElement[ucIndex].ulSetTimeOut    = gastAtSmsCmdTab[i].ulSetTimeOut;
                g_stCmdElement[ucIndex].pfnQryProc      = NULL;
                g_stCmdElement[ucIndex].ulQryTimeOut    = 0;
                g_stCmdElement[ucIndex].pfnTestProc     = gastAtSmsCmdTab[i].pfnTestProc;
                g_stCmdElement[ucIndex].ulTestTimeOut   = gastAtSmsCmdTab[i].ulTestTimeOut;

                g_stCmdElement[ucIndex].pfnAbortProc    = gastAtSmsCmdTab[i].pfnAbortProc;
                g_stCmdElement[ucIndex].ulAbortTimeOut  = gastAtSmsCmdTab[i].ulAbortTimeOut;

                g_stCmdElement[ucIndex].ulParamErrCode  = gastAtSmsCmdTab[i].ulParamErrCode;
                g_stCmdElement[ucIndex].ulChkFlag       = gastAtSmsCmdTab[i].ulChkFlag;
                g_stCmdElement[ucIndex].pszCmdName      = gastAtSmsCmdTab[i].pszCmdName;

                g_stParseContext[ucIndex].pstCmdElement = &g_stCmdElement[ucIndex];

                return AT_SUCCESS;
            }
        }
    }

    g_stParseContext[ucIndex].pstCmdElement = NULL;

    return AT_FAILURE;
}



VOS_UINT32 atMatchCmdName (VOS_UINT8 ucClientId, VOS_UINT32 CmdType)
{
    VOS_UINT32 i = 0;
    HI_LIST_S* me = NULL;
    HI_LIST_S* pCmdTblList = NULL;
    AT_PAR_CMDTBL_LIST_STRU* pstCmdNode = NULL;
    VOS_CHAR *pszCmdName = NULL;

    /* ���õĵط���֤�����ָ�벻Ϊ�� */

    pCmdTblList = &(g_stCmdTblList);

    /* ȫ����ʽ��Ϊ��д�ַ� */
    if(At_UpString(g_stATParseCmd.stCmdName.aucCmdName,g_stATParseCmd.stCmdName.usCmdNameLen) == AT_FAILURE)
    {
        return AT_ERROR;
    }

    pszCmdName = (VOS_CHAR *)g_stATParseCmd.stCmdName.aucCmdName;

    if((CmdType == AT_EXTEND_CMD_TYPE) || (CmdType == AT_BASIC_CMD_TYPE))
    {
        /* ����������Ҫ���⴦�� */
        if(At_MatchSmsCmdName(ucClientId, pszCmdName) == ERR_MSP_SUCCESS)
        {
            return ERR_MSP_SUCCESS;
        }

        msp_list_for_each(me, pCmdTblList)
        {
            pstCmdNode = msp_list_entry(me, AT_PAR_CMDTBL_LIST_STRU, stCmdTblList);

            for(i = 0; i < pstCmdNode->usCmdNum; i++)
            {
                if((pstCmdNode->pstCmdElement == NULL) || (pstCmdNode->pstCmdElement[i].pszCmdName == NULL))
                {
                    continue;
                }

                if(AT_STRCMP((VOS_CHAR *)pszCmdName, (VOS_CHAR *)pstCmdNode->pstCmdElement[i].pszCmdName) == ERR_MSP_SUCCESS)
                {
                    g_stParseContext[ucClientId].pstCmdElement = &(pstCmdNode->pstCmdElement[i]);

                    return ERR_MSP_SUCCESS;
                }
            }
        }
    }

    return AT_CMD_NOT_SUPPORT;
}



VOS_UINT32 ParseParam(AT_PAR_CMD_ELEMENT_STRU* pstCmdElement)
{
    VOS_UINT32 ulParaLen = 0;

    if(g_stATParseCmd.ucCmdOptType == AT_CMD_OPT_SET_PARA_CMD)
    {
        if((pstCmdElement != NULL) && (pstCmdElement->pszParam != NULL))
        {
            ulParaLen = AT_STRLEN((VOS_CHAR *)pstCmdElement->pszParam);  /* ��ȡ�����ű����� */
            if(atParsePara((VOS_UINT8*)pstCmdElement->pszParam,(VOS_UINT16)ulParaLen) != AT_SUCCESS)  /* ����ƥ�� */
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }

    return ERR_MSP_SUCCESS;
}


AT_RRETURN_CODE_ENUM_UINT32 fwCmdTestProc(VOS_UINT8 ucIndex, AT_PAR_CMD_ELEMENT_STRU* pstCmdElement)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_FAILURE;

    /* ���õĵط���ָ֤�벻Ϊ�� */

    if(pstCmdElement->pfnTestProc != NULL)
    {
        ulResult = (AT_RRETURN_CODE_ENUM_UINT32)pstCmdElement->pfnTestProc(ucIndex);

        if(ulResult == AT_WAIT_ASYNC_RETURN)
        {
            g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_PEND;

            /* ����ʱ�� */
            if(At_StartTimer(pstCmdElement->ulTestTimeOut, ucIndex) != AT_SUCCESS)
            {
                return AT_ERROR;
            }

            At_SetAtCmdAbortTickInfo(ucIndex, VOS_GetTick());
        }

        return ulResult;
    }
    else if(pstCmdElement->pszParam != NULL)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
           (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
           "%s: %s", pstCmdElement->pszCmdName, pstCmdElement->pszParam);

        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}



AT_RRETURN_CODE_ENUM_UINT32 atCmdDispatch (VOS_UINT8 ucIndex)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult;
    PFN_AT_FW_CMD_PROC pfnCmdProc = NULL;
    VOS_UINT32 ulTimerLen = 0;
    AT_PAR_CMD_ELEMENT_STRU* pstCmdElement = g_stParseContext[ucIndex].pstCmdElement;

    switch (g_stATParseCmd.ucCmdOptType)
    {
    case AT_CMD_OPT_SET_PARA_CMD:
    case AT_CMD_OPT_SET_CMD_NO_PARA:
        pfnCmdProc = pstCmdElement->pfnSetProc;          /* �����޲������� */
        ulTimerLen = pstCmdElement->ulSetTimeOut;
        break;

    case AT_CMD_OPT_READ_CMD:
        pfnCmdProc = pstCmdElement->pfnQryProc;          /* ��ѯ���� */
        ulTimerLen = pstCmdElement->ulQryTimeOut;
        break;

    case AT_CMD_OPT_TEST_CMD:
        return fwCmdTestProc(ucIndex, pstCmdElement);         /* ���Բ��� */

    default:
        return AT_ERROR;                        /* ����������򷵻ش��� */
    }

    if(pfnCmdProc == NULL)
    {
        return AT_ERROR;
    }

    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)pfnCmdProc(ucIndex);

    if(ulResult == AT_WAIT_ASYNC_RETURN)
    {
        /* ����ʱ�� */
        if(At_StartTimer(ulTimerLen, ucIndex) != AT_SUCCESS)
        {
            return AT_ERROR;
        }

        At_SetAtCmdAbortTickInfo(ucIndex, VOS_GetTick());

        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_PEND;
    }

    return ulResult;
}



VOS_UINT32 LimitedCmdProc(VOS_UINT8 ucClientId, VOS_UINT8 *pData, VOS_UINT16 usLen, AT_PAR_CMD_ELEMENT_STRU* pstCmdElement)
{
    VOS_BOOL bE5CheckRight = VOS_TRUE;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_FAILURE;

    if(pstCmdElement == NULL)
    {
        return AT_ERROR;
    }



    /* ���E5���뱣�����ޣ�����ò�ѯE5���������ӿڣ����޵Ļ�����AT_ERROR�����򷵻�AT_OK */
    if((pstCmdElement->ulChkFlag & CMD_TBL_E5_IS_LOCKED) == 0)
    {
        /* ��ATͨ�� ��WIFIͨ��, ���������뱣�� */
        if ( (gastAtClientTab[ucClientId].UserType != AT_USBCOM_USER)
          && (gastAtClientTab[ucClientId].UserType != AT_SOCK_USER)
          && (gastAtClientTab[ucClientId].UserType != AT_APP_SOCK_USER))
        {
            /* ��������PIN�뱣���ļ�� */
        }
        else
        {
            bE5CheckRight = AT_E5CheckRight(ucClientId, pData, usLen);
            if(bE5CheckRight == VOS_TRUE)
            {
                /* E5�ѽ�������������PIN�뱣���ļ�� */
            }
            else
            {
                return AT_SUCCESS;  /* AT_E5CheckRight �ڲ��ѷ����ϱ���� */
            }
        }
    }

    /* �����E5 DOCK�����ֱ�ӵ���DOCK����ת���ӿڣ�����AT_SUCCESS */
    if(pstCmdElement->ulChkFlag & CMD_TBL_IS_E5_DOCK)
    {
        if (gastAtClientTab[ucClientId].UserType == AT_USBCOM_USER)
        {
            ulResult = (AT_RRETURN_CODE_ENUM_UINT32)AT_DockHandleCmd(ucClientId, pData, usLen);
            if(ulResult == AT_SUCCESS)
            {
                return AT_SUCCESS;  /* ����������ת����E5ͨ������ͨ���������κν�� */
            }
            else
            {
                return AT_ERROR;    /* ����ʧ�ܣ�����ERROR */
            }
        }
    }

    /* ���PIN�뱣�����ޣ�����ò�ѯPIN�����������ӿڣ����޵Ļ�����AT_ERROR�����򷵻�AT_OK */
    if((pstCmdElement->ulChkFlag & CMD_TBL_PIN_IS_LOCKED) == 0)
    {
        ulResult = (AT_RRETURN_CODE_ENUM_UINT32)At_CheckUsimStatus((VOS_UINT8*)pstCmdElement->pszCmdName, ucClientId);
        if(ulResult == AT_SUCCESS)
        {
            /* PIN���ѽ��������������ļ�鴦�� */
        }
        else
        {
            return ulResult;    /* ���ؾ���������ԭ��ֵ */
        }
    }


    return AT_OK;
}


VOS_UINT32 CmdParseProc(VOS_UINT8 ucClientId, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult;
    AT_PAR_CMD_ELEMENT_STRU* pstCmdElement = NULL;

    /* �ú������ô��ɱ�֤ucClientId�ĺϷ��ԣ�pDataIn��Ϊ�� */


    /* ƥ���������� */
    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)atMatchCmdName(ucClientId, g_stATParseCmd.ucCmdFmtType);
    if(ulResult != ERR_MSP_SUCCESS)
    {
        return ulResult;
    }

    pstCmdElement = g_stParseContext[ucClientId].pstCmdElement;

    if(pstCmdElement == NULL)
    {
        return AT_ERROR;
    }

    /* ���������жϺʹ���ӿڣ�����ʧ�ܱ�ʾ��������������ֱ�ӷ��ش����� */
    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)LimitedCmdProc(ucClientId, pData, usLen, pstCmdElement);
    if(ulResult != AT_OK)
    {
        return ulResult;
    }

    /* ƥ��������� */
    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)ParseParam(pstCmdElement);

    if(ulResult != ERR_MSP_SUCCESS)
    {
        if(pstCmdElement->ulCmdIndex == AT_CMD_SD)
        {
            return AT_SDParamErrCode();
        }

        if(pstCmdElement->ulParamErrCode == AT_RRETURN_CODE_BUTT)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        else
        {
            return (AT_RRETURN_CODE_ENUM_UINT32)(pstCmdElement->ulParamErrCode);
        }
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    /* �·���AT�����Ƿ���CLģʽ����Ҫ������AT������ƥ�䣬ƥ��Ļ���ֱ�ӷ��ز��������� */
    if (At_CheckCurrRatModeIsCL(ucClientId) == VOS_TRUE)
    {
        if (atCmdIsSupportedByCLMode(ucClientId) == VOS_TRUE)
        {
            return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
        }
    }
    else
    {
        if (atCmdIsSupportedByGULMode(ucClientId) == VOS_FALSE)
        {
            return AT_CME_OPERATION_NOT_ALLOWED;
        }
    }
#endif

    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)atCmdDispatch(ucClientId);

    return ulResult;
}



VOS_VOID RepeatCmdProc(AT_PARSE_CONTEXT_STRU* pstClientContext)
{
    VOS_UINT8*                          pData = pstClientContext->aucDataBuff;
    VOS_UINT32                          ulLen = pstClientContext->usDataLen;
    errno_t                             lMemResult;

    if(ucAtS4 == *pData)
    {
        pData++;
        ulLen--;
    }

    /* ����Ƿ���"A/" */
    if(ulLen == 3)
    {
        if(At_CheckCharA(*pData) == AT_SUCCESS)
        {
            if(atCheckChar0x2f(*(pData+1)) == AT_SUCCESS)
            {
                if(pstClientContext->usCmdLineLen == 0)
                {
                    pstClientContext->usDataLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pstClientContext->aucDataBuff,
                        (VOS_CHAR *)pstClientContext->aucDataBuff, "AT%c", ucAtS3);
                }
                else
                {
                   lMemResult = memcpy_s(pstClientContext->aucDataBuff, sizeof(pstClientContext->aucDataBuff), (VOS_UINT8*)pstClientContext->pucCmdLine,
                                         pstClientContext->usCmdLineLen);
                   TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstClientContext->aucDataBuff), pstClientContext->usCmdLineLen);

                   pstClientContext->usDataLen = pstClientContext->usCmdLineLen;
                }
            }
        }
    }

    return;
}



VOS_VOID SaveRepeatCmd(AT_PARSE_CONTEXT_STRU* pstClientContext, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    errno_t                             lMemResult;
    /* ����A/����� */
    if(pstClientContext->pucCmdLine != NULL)
    {
        AT_FREE(pstClientContext->pucCmdLine);
    }

    pstClientContext->pucCmdLine = (VOS_UINT8 *)AT_MALLOC(usLen);
    if(pstClientContext->pucCmdLine == NULL)
    {
        return;
    }

    lMemResult = memcpy_s(pstClientContext->pucCmdLine, usLen, pData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen, usLen);
    pstClientContext->usCmdLineLen = usLen;

    return;
}



VOS_UINT32 ScanDelChar( VOS_UINT8 *pData, VOS_UINT16 *pLen, VOS_UINT8 AtS5)
{
    VOS_UINT16 usChkLen  = 0;
    VOS_UINT16 usLen     = 0;
    VOS_UINT8  *pWrite   = pData;
    VOS_UINT8  *pRead    = pData;
    VOS_UINT16 usNum     = 0;    /* ��¼���ŵĸ��� */

    if(*pLen == 0)
    {
        return AT_FAILURE;
    }

    /* ���������� */
    while ( usChkLen++ < *pLen )
    {
        if(*pRead == '"')
        {
            usNum++;
            *pWrite++ = *pRead;
            usLen++;
        }
        else if((AtS5 == *pRead) && ((usNum%2) == 0))       /*ɾ����һ�ַ�,�����ڵ�ɾ����������*/
        {
            if( usLen > 0 )
            {
                pWrite--;
                usLen--;
            }
        }
        else                        /*ѡ����Ч�ַ�*/
        {
            *pWrite++ = *pRead;
            usLen++;
        }
        pRead++;
    }
    *pLen  =  usLen;
    return AT_SUCCESS;
}



VOS_UINT32 ScanCtlChar( VOS_UINT8 *pData, VOS_UINT16 *pLen)
{
    VOS_UINT8  *pWrite    = pData;
    VOS_UINT8  *pRead     = pData;
    VOS_UINT16 usChkLen   = 0;
    VOS_UINT16 usLen      = 0;
    VOS_UINT16 usNum      = 0;

    if(*pLen == 0)
    {
        return AT_FAILURE;
    }

    while( usChkLen++ < *pLen )
    {
        if( *pRead >= 0x20 )   /* ѡ����Ч�ַ� */
        {
            if(*pRead == '"')
            {
                usNum++;
            }
            *pWrite++ = *pRead;
            usLen++;
        }
        else
        {
            if(usNum%2)            /* ˫�����ڵ�С��0x20���ַ���ȥ�� */
            {
                *pWrite++ = *pRead;
                usLen++;
            }
        }
        pRead++;
    }

    *pLen  =  usLen;
    return AT_SUCCESS;
}



VOS_UINT32 ScanBlankChar( VOS_UINT8 *pData, VOS_UINT16 *pLen)
{
    VOS_UINT8  *pWrite        = pData;
    VOS_UINT8  *pRead         = pData;
    VOS_UINT16 usChkLen       = 0;
    VOS_UINT16 usLen          = 0;
    VOS_UINT16 usColonBankCnt = 0;
    VOS_BOOL bIsColonBack  = FALSE;
    VOS_BOOL bIsEqual      = FALSE;

    if(*pLen == 0)
    {
        return AT_FAILURE;
    }


    /* ��� */
    while( usChkLen++ < *pLen )
    {
        if(*pRead == '=')
        {
            bIsEqual = TRUE;
        }

        if((*pRead == ' ') && (bIsEqual != TRUE))
        {
            /* �Ⱥ�ǰ�Ŀո�ȥ�� */
        }
        else    /* rim����ǰ��Ŀո� */
        {
            if(*pRead != 0x20 && *pRead != ',')
            {
                /* rim','��ո� */
                if(bIsColonBack)
                {
                    pWrite -= usColonBankCnt;
                    usLen  -= usColonBankCnt;
                    usColonBankCnt = 0;
                    bIsColonBack   = FALSE;
                }
                else
                {
                    usColonBankCnt = 0;
                }
            }
            else if(*pRead == ',')
            {
                /* rim','ǰ�ո� */
                pWrite -= usColonBankCnt;
                usLen  -= usColonBankCnt;
                usColonBankCnt = 0;
                bIsColonBack   = TRUE;
            }
            else
            {
                usColonBankCnt++;
            }

            *pWrite++ = *pRead;
            usLen++;
        }

        pRead++;
    }

    usLen  -= usColonBankCnt;

    *pLen  =  usLen;
    return AT_SUCCESS;
}



PRIVATE VOS_UINT32 ScanBlankCharAfterEq(IN OUT VOS_UINT8 *pData,IN OUT VOS_UINT16 *pLen)
{
    VOS_UINT8  *pWrite        = pData;
    VOS_UINT8  *pRead         = pData;
    VOS_UINT16 usChkLen       = 0;
    VOS_UINT16 usLen          = 0;
    VOS_BOOL   bIsEqual      = FALSE;
    VOS_BOOL   bIsEqualNoSp  = FALSE;

    if(*pLen == 0)
    {
        return AT_FAILURE;
    }

    /* ��� */
    while( usChkLen++ < *pLen )
    {
        if(*pRead == '=')
        {
            bIsEqual = TRUE;
        }
        else
        {
            if((bIsEqual != TRUE) || (bIsEqualNoSp == TRUE))
            {
                /* �Ⱥ�ǰ���ַ�ȫ�����ƣ��Ⱥź�ķǿո��ַ�ȫ������ */
            }
            else
            {
                if(*pRead == ' ')
                {
                    pRead++;
                    continue;
                }
                else
                {
                    bIsEqualNoSp = TRUE;
                }
            }
        }

        *pWrite++ = *pRead;
        usLen++;

        pRead++;
    }

    *pLen  =  usLen;
    return AT_SUCCESS;
}


VOS_UINT32 FormatCmdStr (VOS_UINT8 *pData, VOS_UINT16 *pLen, VOS_UINT8 AtS3)
{
    VOS_UINT8  *pCheck    = pData;
    VOS_UINT32 ulChkS3Len = 0;
    VOS_UINT32 ulFlg      = 0;
    VOS_UINT16 usLen      = 0;

    /* �����н�����֮ǰ���ַ� */
    while ( ulChkS3Len++ < *pLen  )
    {
        if(AtS3 == *pCheck++)
        {
            ulFlg = 1;
            break;
        }
        else
        {
            usLen++;
        }
    }

    if( ulFlg != 1 )
    {
        return AT_FAILURE;
    }
    else
    {
        *pLen  =  usLen;
        return AT_SUCCESS;
    }
}


VOS_UINT32  AT_DiscardInvalidChar(VOS_UINT8* pucData, VOS_UINT16 *pusLen)
{
    VOS_UINT16                          i;
    VOS_UINT16                          usLen;

    if ((*pusLen == 0) || (*pusLen > AT_CMD_MAX_LEN))
    {
        PS_PRINTF_WARNING("<AT_DiscardInvalidChar> usLen > AT_CMD_MAX_LEN or usLen = 0. usLen :%d", *pusLen);
        return ERR_MSP_FAILURE;
    }
    for(i = 0; i < (*pusLen); i++)
    {
        if(At_CheckCharA(pucData[i]) != AT_SUCCESS)
        {
            continue;
        }
        else if(At_CheckCharT(pucData[i+1]) != AT_SUCCESS)
        {
            continue;
        }
        else
        {
            if (i != 0)
            {
                usLen = (*pusLen - i);
                VOS_MemMove_s(pucData, AT_CMD_MAX_LEN, (pucData+i), usLen);
                *pusLen = usLen;
            }

            return ERR_MSP_SUCCESS;
        }
    }

    return ERR_MSP_FAILURE;
}



VOS_UINT32 CmdStringFormat(VOS_UINT8 ucClientId, VOS_UINT8 *pData,VOS_UINT16* pusLen)
{
    VOS_UINT32 ulRet;

    ulRet = FormatCmdStr(pData,pusLen, ucAtS3);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return AT_ERROR;
    }

    /* ɨ���˸�� */
    ulRet = ScanDelChar(pData,pusLen, ucAtS5);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* ɨ����Ʒ� */
    ulRet = ScanCtlChar(pData,pusLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* ɨ��ո�� */
    ulRet = ScanBlankChar(pData,pusLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    /* ɨ������ŵȺź���Ŀո�� */
    ulRet = ScanBlankCharAfterEq(pData,pusLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING;
    }

    return ERR_MSP_SUCCESS;
}

VOS_BOOL CheckAnyCharCmdName(VOS_UINT8 *pData, VOS_UINT16* pusLen, VOS_UINT8 *plName, VOS_UINT8 *pbName)
{
    VOS_UINT16 i;
    VOS_UINT8  bFound   = 0;      /* check if found string ANY CHAR CMD */
    VOS_UINT8  ucOffset = 0;
    VOS_UINT8* pHeader = pData;   /* remember the header of the pointer */

    for(i=0; i < *pusLen; i++,pHeader++)
    {
        if (*pHeader == ' ')
        {
            continue;          /* skip space char */
        }
        else if ((plName[ucOffset] == *pHeader) || (pbName[ucOffset] == *pHeader))
        {
            ucOffset++;

            if (plName[ucOffset] == '?')
            {
                bFound = 1;    /* found string ANY CHAR CMD  */
            }
        }
        else
        {
            break;             /* not ANY CHAR CMD, go out */
        }
    }

    if (bFound)
    {
        if ((VOS_StrLen((VOS_CHAR *)plName) - 1) == ucOffset)
        {                          /* found string ANY CHAR CMD */
            *pusLen -= i;            /* remove string "AT^CPBW=" */
            TAF_MEM_MOVE_S(pData, *pusLen, pData+i, *pusLen);
            return TRUE;
        }
    }

    return FALSE;
}


VOS_UINT32 AnyCharCmdParse(VOS_UINT8* pData, VOS_UINT16 usLen, VOS_UINT8* pName)
{
    VOS_UINT16 i,j;
    VOS_UINT8  bInQoute = 0;

    VOS_UINT8  aucPara[AT_PARA_MAX_LEN + 1]  = {0};     /* �������ý������Ĳ����ַ��� */
    VOS_UINT16 usParaLen                     = 0;       /* ������ʶ�����ַ������� */
    errno_t    lMemResult;

    AT_ResetParseVariable();    /* ���ý��������õ�ȫ�ֱ��� */

    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;  /* ���������������*/
    g_stATParseCmd.ucCmdFmtType = AT_EXTEND_CMD_TYPE;       /* ������������*/

    g_stATParseCmd.stCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrLen((VOS_CHAR *)pName) - 2;    /* ������ ^CPBW ����*/

    lMemResult = memcpy_s(g_stATParseCmd.stCmdName.aucCmdName, sizeof(g_stATParseCmd.stCmdName.aucCmdName), pName, g_stATParseCmd.stCmdName.usCmdNameLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(g_stATParseCmd.stCmdName.aucCmdName), g_stATParseCmd.stCmdName.usCmdNameLen);

    /* ^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]] */
    /* ^SCPBW= (1-65535),(numa),(0-255),(numb),(0-255),(numc),(0-255),(numd),(0-255),(text),(0,1),(email)*/
    /* +CPBW=[<index>][,<number>[,<type>[,<text>]]] */

    /* seperate para by char ',' */
    for(i=0; i < usLen; i++,pData++)
    {
        if (usParaLen > AT_PARA_MAX_LEN) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ((*pData == ' ') && (bInQoute == 0))
        {
            continue;          /* skip space char not in quatation*/
        }
        else if (*pData == ',')
        {
            if (bInQoute)
            {
                if (bInQoute == 1)
                {
                    /* ������','���ǲ�����һ���֣����ǲ����ָ��� */
                    aucPara[usParaLen] = *pData;
                    usParaLen++;

                    continue;
                }

                bInQoute = 0;      /* ˫�����ڿ��ܻ���˫���ţ������ٸ��ݵڶ���˫������Ϊ�ַ������� , �Բ����ָ��','��Ϊ�ַ�����������*/

                /* ���������쳣 : ���ַ������棬','ǰ�����Ч�ո� */
                for(j=usParaLen-1; j>0; j--)
                {
                    if(aucPara[j] == ' ')
                    {
                        usParaLen --;
                    }
                    else if (aucPara[j] == '"')
                    {
                        break;
                    }
                    else
                    {
                        return AT_CME_INCORRECT_PARAMETERS;    /* ���һ��˫���ź����������ַ������ش��� */
                    }
                }
            }

            if(i < (usLen-1))
            {
                if(atfwParseSaveParam(aucPara, usParaLen) != ERR_MSP_SUCCESS)
                {
                    return AT_ERROR;
                }

                usParaLen = 0;

                /* too many para */
            }

            continue;
        }
        else
        {
            /* �洢������ȫ�ֱ����� */
            aucPara[usParaLen] = *pData;
            usParaLen++;

            if (*pData == '"')
            {
                bInQoute++;
            }
        }
    }

    if(atfwParseSaveParam(aucPara, usParaLen) != ERR_MSP_SUCCESS)
    {
        return AT_ERROR;
    }

    return AT_SUCCESS;
}



VOS_UINT32 AnyCharCmdProc(VOS_UINT8 *pData, VOS_UINT16* pusLen)
{
    VOS_UINT16 i;
    VOS_UINT32 ulRet = 0;
    VOS_UINT8  ucCmdName_lowercase[3][16]={"at^cpbw=?", "at+cpbw=?", "at^scpbw=?"};
    VOS_UINT8  ucCmdName_uppercase[3][16]={"AT^CPBW=?", "AT+CPBW=?", "AT^SCPBW=?"};

    for(i = 0; i < 3; i++)
    {
        if(CheckAnyCharCmdName(pData, pusLen, ucCmdName_lowercase[i], ucCmdName_uppercase[i]) == TRUE)
        {
            ulRet = FormatCmdStr(pData,pusLen, ucAtS3);
            if(ulRet != ERR_MSP_SUCCESS)
            {
                return AT_ERROR;
            }

            return AnyCharCmdParse(pData, *pusLen, &(ucCmdName_uppercase[i][2]));
        }
    }

    return AT_FAILURE;
}




VOS_VOID At_ReadyClientCmdProc(VOS_UINT8 ucIndex, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    VOS_UINT32 ulRet;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult;
    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;

    if(usLen < 3)
    {
        return;
    }

    pClientContext = &(g_stParseContext[ucIndex]);

    SaveRepeatCmd(pClientContext, pData, usLen);

    At_ResetCombinParseInfo(ucIndex);

    ulRet = AT_DiscardInvalidChar(pData, &usLen);
    if (ulRet != ERR_MSP_SUCCESS)
    {
        return ;
    }

    ulRet = AnyCharCmdProc(pData, &usLen);

    if(ulRet == AT_SUCCESS)     /* �������ַ����������� */
    {
        /* ����� */
        ulRet = CmdParseProc(ucIndex, pData, usLen);
        if((ulRet != AT_OK) && (ulRet != AT_WAIT_ASYNC_RETURN))
        {
            At_ResetCombinParseInfo(ucIndex);
        }

        At_FormatResultData(ucIndex, ulRet);

        return ;
    }
    else if(ulRet != AT_FAILURE)    /* �������ַ��������������������ʧ�� */
    {
        At_FormatResultData(ucIndex, ulRet);

        return;
    }
    else    /* ���������ַ����������� */
    {
        /* do nothing */
    }

    ulRet = CmdStringFormat(ucIndex, pData, &usLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        At_FormatResultData(ucIndex, ulRet);

        return;
    }

    /* ���Ϊ"AT"�����ַ� */
    if (usLen < 2)
    {
        return;
    }

    /* ����V3R1Ҫ��ǰ�����ַ�����"AT"���������κ���Ϣ */
    if(At_CheckCharA(*pData) != AT_SUCCESS)   /* �������'A'/'a' */
    {
        return;
    }

    if(At_CheckCharT(*(pData+1)) != AT_SUCCESS)   /* �������'T'/'t' */
    {
        return;
    }

    /* SIMLOCK���������61�����������⴦�� */
    if (At_ProcSimLockPara(ucIndex, pData, usLen) == AT_SUCCESS)
    {
        return;
    }

    /* ^DOCK������滹��"^"��"="�����⴦�� */
    if (AT_HandleDockSetCmd(ucIndex, pData, usLen) == AT_SUCCESS)
    {
        return;
    }

    /* AP-Modem��̬�£���������AT^FACAUTHPUBKEY��AT^SIMLOCKDATAWRITE��������������Ѿ������˽�������
       ������������512����Ҫ���⴦��*/
    if (At_HandleApModemSpecialCmd(ucIndex, pData, usLen) == AT_SUCCESS)
    {
        return;
    }

    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)At_CombineCmdChkProc(ucIndex, pData, usLen);

    /* ���ؽ�� */
    if((ulResult == AT_FAILURE) || (ulResult == AT_SUCCESS))
    {
        return ;
    }

    if(ulResult != AT_WAIT_ASYNC_RETURN)
    {
        At_FormatResultData(ucIndex, ulResult);
    }

    return;
}



VOS_VOID atCmdMsgProc(VOS_UINT8 ucIndex, VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    VOS_UINT32                          ulRet;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_WAIT_ASYNC_RETURN;
    AT_PARSE_CONTEXT_STRU              *pClientContext = NULL;
    AT_MODEM_AGPS_CTX_STRU             *pstAgpsCtx = VOS_NULL_PTR;

    pstAgpsCtx = AT_GetModemAgpsCtxAddrFromClientId(ucIndex);

    if ((usLen == 0) || (usLen > AT_CMD_MAX_LEN))
    {
        return;
    }

    pClientContext = &(g_stParseContext[ucIndex]);

    if (pClientContext->ucMode == AT_SMS_MODE)
    {
        ulResult = (AT_RRETURN_CODE_ENUM_UINT32)At_SmsProc(ucIndex, pData, usLen);

        if ((ulResult == AT_SUCCESS)
         || (ulResult == AT_WAIT_SMS_INPUT))
        {
            At_FormatResultData(ucIndex,ulResult);
            return ;
        }

        gastAtClientTab[ucIndex].usSmsTxtLen = 0;   /* ����BUFFER��� */

        At_SetMode(ucIndex,AT_CMD_MODE,AT_NORMAL_MODE);         /* �û�����״̬ */

        if (ulResult == AT_WAIT_ASYNC_RETURN)
        {
            if (pClientContext->pstCmdElement != NULL)
            {
                if (At_StartTimer(pClientContext->pstCmdElement->ulSetTimeOut, ucIndex) != AT_SUCCESS)
                {
                    AT_ERR_LOG("atCmdMsgProc():ERROR:Start Timer Failed");
                }

                pClientContext->ucClientStatus = AT_FW_CLIENT_STATUS_PEND;
            }
            return;
        }

        At_FormatResultData(ucIndex,ulResult);
        return;
    }
    else if (pClientContext->ucMode == AT_XML_MODE)
    {
        /* ����XML���봦���� */
        ulResult = (AT_RRETURN_CODE_ENUM_UINT32)At_ProcXmlText(ucIndex, pData, usLen);

        /* ����ɹ��������ʾ��">" */
        if (ulResult == AT_WAIT_XML_INPUT)
        {
            At_FormatResultData(ucIndex,ulResult);
            return;
        }

        /* ��ջ����� */
        PS_MEM_FREE(WUEPS_PID_AT, pstAgpsCtx->stXml.pcXmlTextHead);                         /* XML BUFFER��� */
        pstAgpsCtx->stXml.pcXmlTextHead = VOS_NULL_PTR;
        pstAgpsCtx->stXml.pcXmlTextCur  = VOS_NULL_PTR;

        At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);                           /* �û�����״̬ */

        if (ulResult == AT_WAIT_ASYNC_RETURN)
        {
            if (pClientContext->pstCmdElement != NULL)
            {
                if (At_StartTimer(pClientContext->pstCmdElement->ulSetTimeOut, ucIndex) != AT_SUCCESS)
                {
                    AT_ERR_LOG("atCmdMsgProc():ERROR:Start Timer Failed");
                }

                pClientContext->ucClientStatus = AT_FW_CLIENT_STATUS_PEND;
            }
            return;
        }

        At_FormatResultData(ucIndex,ulResult);
        return;
    }
    else
    {
    }

    /* ���������������������ͨ������������ */
    ulRet = AT_ParseCmdIsComb(ucIndex, pData, usLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return;
    }

    /* �ж��Ƿ���PEND״̬��ͨ�� */
    ulRet = AT_ParseCmdIsPend(ucIndex, pData, usLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return;
    }

    At_ReadyClientCmdProc(ucIndex, pData, usLen);

    return ;
}



VOS_UINT32 At_CmdStreamRcv(VOS_UINT8 ucIndex, VOS_UINT8* pData, VOS_UINT16 usLen)
{
    VOS_UINT8*                          pHead = NULL;
    VOS_UINT16                          usCount = 0;  /* ��ǰָ����ַ�λ�� */
    VOS_UINT16                          usTotal = 0;  /* �Ѿ�������ַ����� */
    AT_PARSE_CONTEXT_STRU*              pstClientContext = NULL;
    errno_t                             lMemResult;

    /* ucIndexֻ���һ�Σ��������Ӻ������ټ�� */
    if((pData == NULL) || (ucIndex >= AT_MAX_CLIENT_NUM))
    {
        return ERR_MSP_INVALID_PARAMETER;
    }

    pstClientContext = &(g_stParseContext[ucIndex]);

    pHead = pData;

    /* �н�����(<CR>�����û�ָ��) */
    while(usCount++ < usLen)
    {
        if(At_CheckCmdSms(*(pData+usCount-1), pstClientContext->ucMode))
        {
            /* �������Ѿ������� */
            if(pstClientContext->usDataLen > 0)
            {
                if((pstClientContext->usDataLen + usCount) >= AT_CMD_MAX_LEN)
                {
                    pstClientContext->usDataLen = 0;
                    return ERR_MSP_INSUFFICIENT_BUFFER;
                }

                lMemResult = memcpy_s(&pstClientContext->aucDataBuff[pstClientContext->usDataLen],
                                      AT_CMD_MAX_LEN - pstClientContext->usDataLen,
                                      pHead,
                                      usCount);
                TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN - pstClientContext->usDataLen, usCount);
                pstClientContext->usDataLen += usCount;
            }
            else    /* ������������ */
            {
                lMemResult = memcpy_s(&pstClientContext->aucDataBuff[0], AT_CMD_MAX_LEN, pHead, (VOS_SIZE_T)(usCount-usTotal));
                TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN, (VOS_SIZE_T)(usCount-usTotal));
                pstClientContext->usDataLen = usCount-usTotal;
            }

            RepeatCmdProc(pstClientContext);

            atCmdMsgProc(ucIndex, pstClientContext->aucDataBuff, pstClientContext->usDataLen);

            pHead = pData+usCount;
            usTotal = usCount;
            pstClientContext->usDataLen = 0;
        }
    }

    /* �в����ַ�δ������Ҫ���� */
    if(usTotal < usLen)
    {
        if((pstClientContext->usDataLen + (usLen-usTotal)) >= AT_CMD_MAX_LEN)
        {
            pstClientContext->usDataLen = 0;
            return ERR_MSP_INSUFFICIENT_BUFFER;
        }

        lMemResult = memcpy_s(&pstClientContext->aucDataBuff[pstClientContext->usDataLen],
                              AT_CMD_MAX_LEN - pstClientContext->usDataLen,
                              pHead,
                              (VOS_UINT32)(usLen-usTotal));
        TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN - pstClientContext->usDataLen, (VOS_UINT32)(usLen-usTotal));

        pstClientContext->usDataLen += (VOS_UINT16)(usLen-usTotal); /* (pData-pHead+ulLen); */
    }

    return ERR_MSP_SUCCESS;
}



VOS_VOID At_CombineBlockCmdProc(VOS_UINT8 ucIndex)
{
    errno_t                             lMemResult;
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_WAIT_ASYNC_RETURN;
    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;
    AT_FW_COMBINE_CMD_INFO_STRU* pstCombineCmdInfo = NULL;
    VOS_UINT8* pucBlockCmd = NULL;
    VOS_UINT16 usBlockCmdLen = 0;

    if(ucIndex >= AT_MAX_CLIENT_NUM)
    {
        return;
    }

    pClientContext = &(g_stParseContext[ucIndex]);

    pstCombineCmdInfo = &pClientContext->stCombineCmdInfo;

    /* �ж���������Ƿ������ */
    if((pstCombineCmdInfo->usProcNum < pstCombineCmdInfo->usTotalNum))
    {
        /* ������һ������ */
        ulResult = (AT_RRETURN_CODE_ENUM_UINT32)At_CombineCmdProc(ucIndex);

        /* ���ؽ�� */
        if((ulResult == AT_FAILURE) || (ulResult == AT_SUCCESS))
        {
            return ;
        }

        if(ulResult != AT_WAIT_ASYNC_RETURN)
        {
            At_FormatResultData(ucIndex, ulResult);
        }

        return ;
    }

    /* �ж��Ƿ��л�������� */
    ucIndex = AT_BlockCmdCheck();
    if(ucIndex < AT_MAX_CLIENT_NUM)
    {
        pClientContext = &(g_stParseContext[ucIndex]);

        /* ֹͣ��������Ķ�ʱ�� */
        AT_StopRelTimer(ucIndex, &pClientContext->hTimer);

        usBlockCmdLen = pClientContext->usBlockCmdLen;
        pucBlockCmd = (VOS_UINT8*)AT_MALLOC(usBlockCmdLen);
        if(pucBlockCmd == NULL)
        {
            /* ������ʱ���ػ��������������󴥷� */
            return ;
        }

        lMemResult = memcpy_s(pucBlockCmd, usBlockCmdLen, pClientContext->pBlockCmd, usBlockCmdLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, usBlockCmdLen, usBlockCmdLen);

        AT_ClearBlockCmdInfo(ucIndex);

        /* ���������� */
        At_ReadyClientCmdProc(ucIndex, pucBlockCmd, usBlockCmdLen);

        AT_FREE(pucBlockCmd);

        return ;
    }

    return;
}


VOS_VOID At_CmdMsgDistr(VOS_VOID *pMsg)
{
    AT_CMD_MSG_NUM_CTRL_STRU           *pstMsgNumCtrlCtx = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstMsg           = VOS_NULL_PTR;
    VOS_UINT16                          usLen;
    VOS_UINT8                          *pData = NULL;
    VOS_UINT8                           ucIndex;
    VOS_ULONG                           ulLockLevel;

    pstMsg = (AT_MSG_STRU*)pMsg;

    if (pstMsg->ucType == AT_NORMAL_TYPE_MSG)
    {
        pstMsgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();

        /* ���м��� */
        /*lint -e571*/
        VOS_SpinLockIntLock(&(pstMsgNumCtrlCtx->stSpinLock), ulLockLevel);
        /*lint +e571*/

        if ( pstMsgNumCtrlCtx->ulMsgCount > 0)
        {
            pstMsgNumCtrlCtx->ulMsgCount--;
        }

        VOS_SpinUnlockIntUnlock(&(pstMsgNumCtrlCtx->stSpinLock), ulLockLevel);
    }

    usLen   = pstMsg->usLen;
    pData   = pstMsg->aucValue;
    ucIndex = pstMsg->ucIndex;

    /* C��/HIFI��λʱ��ģ�����AT��AT������Ϣ */
    switch(pstMsg->ucType)
    {
        case ID_CCPU_AT_RESET_START_IND:
            AT_RcvCcpuResetStartInd(pstMsg);
            return;

        case ID_CCPU_AT_RESET_END_IND:
            AT_RcvCcpuResetEndInd(pstMsg);
            return;

        case ID_HIFI_AT_RESET_START_IND:
            AT_RcvHifiResetStartInd(pstMsg);
            return;

        case ID_HIFI_AT_RESET_END_IND:
             AT_RcvHifiResetEndInd(pstMsg);
            return;

        default:
            break;
    }

    AT_AddUsedClientId2Tab(ucIndex);

    /* �������C�˸�λǰ�򲻴����κ�AT���� */
    if (AT_GetResetFlag() == VOS_TRUE)
    {
        return;
    }

    if (pstMsg->ucType == AT_SWITCH_CMD_MODE_MSG)
    {
        AT_RcvSwitchCmdModeMsg(pstMsg->ucIndex);
        return;
    }

    if (pstMsg->ucType == AT_WATER_LOW_MSG)
    {
        AT_RcvWaterLowMsg(pstMsg->ucIndex);
        return;
    }

    if (pstMsg->ucType == AT_NCM_CONN_STATUS_MSG)
    {
        AT_NDIS_ConnStatusChgProc(NCM_IOCTL_STUS_BREAK);
        return;
    }


    if(pstMsg->ucType == AT_COMBIN_BLOCK_MSG)
    {
        At_CombineBlockCmdProc(ucIndex);
        return;
    }

    At_CmdStreamRcv(ucIndex, pData, usLen);

    return;
}



VOS_UINT32 atfwParseSaveParam(VOS_UINT8* pStringBuf, VOS_UINT16 usLen)
{
    errno_t                             lMemResult;
    if ((usLen > AT_PARA_MAX_LEN) || (gucAtParaIndex >= AT_MAX_PARA_NUMBER))
    {
        return ERR_MSP_FAILURE;
    }

    /* �������Ϊ0���������ַ���ָ��Ϊ�գ����򷵻�ʧ�� */
    if ((pStringBuf == NULL) && (usLen != 0))
    {
        return ERR_MSP_FAILURE;
    }

    /* ����ַ�������Ϊ0������Ҫ���� */
    if((usLen != 0) && (pStringBuf != NULL))
    {
        lMemResult = memcpy_s(gastAtParaList[gucAtParaIndex].aucPara,
                              sizeof(gastAtParaList[gucAtParaIndex].aucPara),
                              pStringBuf,
                              usLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(gastAtParaList[gucAtParaIndex].aucPara), usLen);
    }

    gastAtParaList[gucAtParaIndex].usParaLen = usLen;

    gucAtParaIndex++;

    return ERR_MSP_SUCCESS;
}


VOS_UINT32 AT_SaveCdataDialParam(
    VOS_UINT8                          *pStringBuf,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucCurrPtr = pStringBuf;
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;

    if ((usLen > AT_PARA_MAX_LEN) || (pStringBuf == NULL))
    {
        return ERR_MSP_FAILURE;
    }
    /* Ϊ�������"*99#", "*98#",
       ��"#777"������Ϊ:
       #,
       #777,
       777
    */

    if (usLen > AT_CDATA_DIAL_777_LEN)
    {
        /* ָ������'D' */
        pucCurrPtr++;

        /* ����'#' */
        gastAtParaList[gucAtParaIndex].aucPara[0] = *pucCurrPtr;
        gastAtParaList[gucAtParaIndex].usParaLen  = 1;

        /* ����'#777' */
        gucAtParaIndex++;
        usLength = usLen - 1;
        lMemResult = memcpy_s(gastAtParaList[gucAtParaIndex].aucPara,
                              sizeof(gastAtParaList[gucAtParaIndex].aucPara),
                              pucCurrPtr,
                              usLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(gastAtParaList[gucAtParaIndex].aucPara), usLength);
        gastAtParaList[gucAtParaIndex].usParaLen = usLength;

        /* ����'777' */
        pucCurrPtr++;
        gucAtParaIndex++;
        usLength--;
        lMemResult = memcpy_s(gastAtParaList[gucAtParaIndex].aucPara,
                              sizeof(gastAtParaList[gucAtParaIndex].aucPara),
                              pucCurrPtr,
                              usLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(gastAtParaList[gucAtParaIndex].aucPara), usLength);
        gastAtParaList[gucAtParaIndex].usParaLen = usLength;

    }

    gucAtParaIndex++;

    return ERR_MSP_SUCCESS;
}



VOS_UINT32 At_CmdTestProcOK(VOS_UINT8 ucIndex)
{
    return AT_OK;
}



VOS_UINT32 At_CmdTestProcERROR(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}



VOS_UINT32 AT_IsAbortCmdStr(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    AT_ABORT_CMD_PARA_STRU             *pstAbortCmdPara = VOS_NULL_PTR;
    VOS_UINT32                          ulAbortCmdStrLen;
    VOS_UINT32                          ulAtCurrSetTick;
    AT_CMD_ABORT_TICK_INFO             *pstCmdAbortTick     = VOS_NULL_PTR;
    VOS_UINT32                          ulTimeInternal;

    ulAtCurrSetTick = VOS_GetTick();
    pstCmdAbortTick = At_GetAtCmdAbortTickInfo();

    /* ������ַ��Ѿ�ȥ���˲��ɼ��ַ�(<0x20��ASCII�ַ�),�ո�, S3, S5���ַ�. ��û���ַ��������� */

    ulAbortCmdStrLen = 0;

    /* ��ȡ�������Ĳ��� */
    pstAbortCmdPara  = AT_GetAbortCmdPara();

    if (pstAbortCmdPara == VOS_NULL_PTR)
    {
        return VOS_FALSE;
    }

    /* �ж��Ƿ�ʹ�ܴ������� */
    if ( pstAbortCmdPara->ucAbortEnableFlg != VOS_TRUE )
    {
        return VOS_FALSE;
    }



    /* ��⵱ǰ���tickֵ�Ƿ��������125ms����,��������㣬��ֱ�ӷ��ز���� */
    if  ( ulAtCurrSetTick >= pstCmdAbortTick->ulAtSetTick[ucIndex] )
    {

        ulTimeInternal = ulAtCurrSetTick - pstCmdAbortTick->ulAtSetTick[ucIndex];
    }
    else
    {
        ulTimeInternal = ulAtCurrSetTick + (AT_MAX_TICK_TIME_VALUE - pstCmdAbortTick->ulAtSetTick[ucIndex]);
    }

    if ( ulTimeInternal < AT_MIN_ABORT_TIME_INTERNAL)
    {
        return VOS_FALSE;
    }

    /* ��⵱ǰ�Ƿ��������ַ���ϣ�����ǣ���ֱ�ӷ��ش�� */
    if  ( At_GetAtCmdAnyAbortFlg() == VOS_TRUE )
    {
        return VOS_TRUE;
    }

    /* �����ж� */
    ulAbortCmdStrLen = VOS_StrLen((VOS_CHAR *)pstAbortCmdPara->aucAbortAtCmdStr);
    if ( (usLen == 0) || (usLen != ulAbortCmdStrLen) )
    {
        return VOS_FALSE;
    }

    /* ��ǰ���������ַ���ϣ�����Ҫ���AT������ݣ��Ǵ������򷵻ش�� */
    if ( (VOS_StrNiCmp((VOS_CHAR *)pucData, (VOS_CHAR *)pstAbortCmdPara->aucAbortAtCmdStr, usLen) == 0) )
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_VOID AT_AbortCmdProc(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulTimerLen;
    VOS_UINT32                          ulResult;

    /* ��ָ���� */
    if ( g_stParseContext[ucIndex].pstCmdElement == VOS_NULL_PTR )
    {
        AT_WARN_LOG("AT_AbortCmdProc: pstCmdElement NULL.");
        return;
    }

    if ( g_stParseContext[ucIndex].pstCmdElement->pfnAbortProc == VOS_NULL_PTR )
    {
        return;
    }

    /* ���õ�ǰ���ڴ����AT�����Abort���� */
    ulResult = g_stParseContext[ucIndex].pstCmdElement->pfnAbortProc(ucIndex);
    if ( ulResult == AT_WAIT_ASYNC_RETURN )
    {
        /* ���ڵ�ǰAT�˿ڵ�״̬�Ѿ�����PEND״̬, �˴�����Ҫ�ٸ��¶˿�״̬ */

        /* ������ϱ�����ʱ��, ��ʱ����Ҫ��AT�˿��ϱ�ABORT */
        ulTimerLen = g_stParseContext[ucIndex].pstCmdElement->ulAbortTimeOut;
        if ( ulTimerLen == 0 )
        {
            AT_WARN_LOG("AT_AbortCmdProc: TimerLen 0.");
            return;
        }

        /* �𱣻���ʱ�� */
        if ( At_StartTimer(ulTimerLen, ucIndex) != AT_SUCCESS )
        {
            AT_WARN_LOG("AT_AbortCmdProc: StartTimer Err.");
            return;
        }
    }
    /* ��ʾATֱ�Ӵ�� ����Ҫ���첽��Ϣ */
    else if ( (ulResult == AT_ABORT)
           || (ulResult == AT_OK) )
    {
        ulResult               = AT_ABORT;
        gstAtSendData.usBufLen = 0;

        AT_STOP_TIMER_CMD_READY(ucIndex);

        At_FormatResultData(ucIndex,ulResult);
    }
    /* �����쳣������Ϣ����ʧ�� �ڴ�����ʧ�ܣ���ǰ����������Ͳ�֧�ִ��
       (����ָ���ѹ����յ��������, ����ǰָ���Ѳ�֧�ִ��)�ȣ����� ������ϴ��� */
    else
    {
        AT_WARN_LOG1("AT_AbortCmdProc, WARNING, Return Code %d!", ulResult);
    }

}


AT_PAR_CMD_ELEMENT_STRU* AT_GetCmdElementInfo(
    VOS_UINT8                          *pucCmdName,
    VOS_UINT32                          ulCmdType
)
{
    HI_LIST_S                          *pstCurList    = VOS_NULL_PTR;
    HI_LIST_S                          *pstCmdTblList = VOS_NULL_PTR;
    AT_PAR_CMDTBL_LIST_STRU            *pstCurCmdNode = VOS_NULL_PTR;
    VOS_UINT32                          i;


    /* ���õĵط���֤�����ָ�벻Ϊ�� */

    pstCmdTblList   = &(g_stCmdTblList);

    /* ����������Ͳ�����չ����Ҳ���ǻ������ֱ�ӷ��ؿ�ָ�� */
    if ((ulCmdType != AT_EXTEND_CMD_TYPE) && (ulCmdType != AT_BASIC_CMD_TYPE))
    {
        return VOS_NULL_PTR;
    }

    /* �ڸ�������в���ָ������ */
    msp_list_for_each(pstCurList, pstCmdTblList)
    {
        pstCurCmdNode = msp_list_entry(pstCurList, AT_PAR_CMDTBL_LIST_STRU, stCmdTblList);

        for (i = 0; i < pstCurCmdNode->usCmdNum; i++)
        {
            /* û�ҵ�ʱ����������в�����һ������ */
            if ((pstCurCmdNode->pstCmdElement == NULL) || (pstCurCmdNode->pstCmdElement[i].pszCmdName == NULL))
            {
                continue;
            }
            /* �ҵ�ʱ��ָ��ָ����Ӧ������ */
            if (AT_STRCMP((VOS_CHAR*)pucCmdName, (VOS_CHAR*)pstCurCmdNode->pstCmdElement[i].pszCmdName) == ERR_MSP_SUCCESS)
            {
                return &(pstCurCmdNode->pstCmdElement[i]);
            }
        }
    }

    return VOS_NULL_PTR;
}


VOS_VOID AT_SaveCmdElementInfo(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucCmdName,
    VOS_UINT32                          ulCmdType
)
{
    AT_PAR_CMD_ELEMENT_STRU            *pstGetCmdElemnet = VOS_NULL_PTR;

    if (pucCmdName == VOS_NULL_PTR)
    {
        return;
    }

    /* û���ҵ���Ӧ��ָ������ʱ��ֱ�ӷ��� */
    pstGetCmdElemnet = AT_GetCmdElementInfo(pucCmdName, ulCmdType);
    if (pstGetCmdElemnet == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SaveCmdElementInfo: GetCmdElementInfo Failed.");
        return;
    }

    /* �ҵ�ʱ�����浽ȫ�ֱ��� */
    g_stParseContext[ucIndex].pstCmdElement = pstGetCmdElemnet;

    return;
}



VOS_UINT32 AT_IsAnyParseClientPend(VOS_VOID)
{
    AT_PORT_BUFF_CFG_STRU              *pstPortBuffCfg = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT8                           ucClientIndex;

    pstPortBuffCfg = AT_GetPortBuffCfgInfo();

    if (pstPortBuffCfg->ucNum > AT_MAX_CLIENT_NUM)
    {
        pstPortBuffCfg->ucNum = AT_MAX_CLIENT_NUM;
    }

    /* ���ݵ�ǰ��¼��ͨ����ѯͨ��״̬ */
    for (i = 0; i < pstPortBuffCfg->ucNum; i++)
    {
        ucClientIndex = (VOS_UINT8)pstPortBuffCfg->ulUsedClientID[i];

        if (ucClientIndex >= AT_CLIENT_ID_BUTT)
        {
            continue;
        }

        if (g_stParseContext[ucClientIndex].ucClientStatus == AT_FW_CLIENT_STATUS_PEND)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_UINT32 AT_IsAllClientDataMode(VOS_VOID)
{
    AT_PORT_BUFF_CFG_STRU              *pstPortBuffCfg = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT8                           ucClientIndex;
    VOS_UINT32                          ulDataModeNum;

    ulDataModeNum = 0;
    pstPortBuffCfg = AT_GetPortBuffCfgInfo();

    if (pstPortBuffCfg->ucNum > AT_MAX_CLIENT_NUM)
    {
        pstPortBuffCfg->ucNum = AT_MAX_CLIENT_NUM;
    }

    /* �鿴�ж��ٸ�ͨ������dataģʽ */
    for (i = 0; i < pstPortBuffCfg->ucNum; i++)
    {
        ucClientIndex = (VOS_UINT8)pstPortBuffCfg->ulUsedClientID[i];
        if (ucClientIndex >= AT_CLIENT_ID_BUTT)
        {
            continue;
        }

        if (gastAtClientTab[ucClientIndex].Mode == AT_DATA_MODE)
        {
            ulDataModeNum++;
        }
    }

    /* ����ͨ���д���dataģʽ */
    if (ulDataModeNum == pstPortBuffCfg->ucNum)
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;

}


#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT8 atCmdIsSupportedByCLMode(VOS_UINT8 ucIndex)
{
    AT_PAR_CMD_ELEMENT_STRU            *pstCmdElement = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulCmdArraySize;
    VOS_UINT32                          aulCmdIndex[]  = {
                                                             /*AT_CMD_CLIP,
                                                                    AT_CMD_DTMF,*/
                                                             AT_CMD_CARDMODE,

                                                             #if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
                                                             AT_CMD_CSCB,
                                                             #endif
                                                             /*AT_CMD_CNMI,*/

                                                             /*AT_CMD_CSASM,*/
                                                             AT_CMD_CUSD,
                                                             /*AT_CMD_CMGW,
                                                                    AT_CMD_CMGD,*/
                                                             AT_CMD_CCWA,
                                                             AT_CMD_CCFC,
                                                             /*AT_CMD_CLIR,*/

                                                             AT_CMD_CELLINFO,
                                                             AT_CMD_CPMS,
                                                             AT_CMD_RELEASERRC,
                                                             AT_CMD_EOPLMN,
                                                             AT_CMD_CNMR
                                                             /*  ����Volte�������ӣ�at�������ƽ�� */
                                                             };

    ulCmdArraySize = sizeof((aulCmdIndex)) / sizeof((aulCmdIndex[0]));

    pstCmdElement = g_stParseContext[ucIndex].pstCmdElement;

    /* CLCK�Ĳ�������ΪSC��FD����Ҫ������������Ҫ���� */
    if (pstCmdElement->ulCmdIndex == AT_CMD_CLCK)
    {
        if ((gastAtParaList[0].ulParaValue == AT_CLCK_SC_TYPE)
         || (gastAtParaList[0].ulParaValue == AT_CLCK_FD_TYPE))
        {
            return VOS_FALSE;
        }
        else
        {
            return VOS_TRUE;
        }
    }

    /* CPWD�Ĳ�������ΪSC��FD����Ҫ������������Ҫ���� */
    if (pstCmdElement->ulCmdIndex == AT_CMD_CPWD)
    {
        if ((gastAtParaList[0].ulParaValue == AT_CLCK_SC_TYPE)
         || (gastAtParaList[0].ulParaValue == AT_CLCK_P2_TYPE))
        {
            return VOS_FALSE;
        }
        else
        {
            return VOS_TRUE;
        }
    }

    /* �ж��·���AT�����cmdindex�Ƿ��������е���CLģʽ�´�������AT�����index */
    for (ulLoop = 0; ulLoop < ulCmdArraySize; ulLoop++)
    {
        if (pstCmdElement->ulCmdIndex == aulCmdIndex[ulLoop])
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_UINT8 atCmdIsSupportedByGULMode(VOS_UINT8 ucIndex)
{
    AT_PAR_CMD_ELEMENT_STRU            *pstCmdElement = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulCmdArraySize;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          aulCmdIndex[]  = {
                                                            AT_CMD_ECCALL,
                                                            AT_CMD_ECCTRL,
                                                            AT_CMD_ECCAP,
                                                            AT_CMD_ECRANDOM,
                                                            AT_CMD_ECKMC,
                                                            AT_CMD_ECCTEST
                                                         };

    enModemId = MODEM_ID_0;

    /* ��ȡmodemidʧ�ܣ�Ĭ�ϲ�������Ӧ��AT���� */
    if (AT_GetModemIdFromClient((VOS_UINT16)ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("atCmdIsSupportedByGULMode:Get Modem Id fail!");
        return VOS_TRUE;
    }

    AT_NORM_LOG1("atCmdIsSupportedByGULMode: enModemId:", (VOS_INT32)enModemId);

    ulCmdArraySize = sizeof((aulCmdIndex)) / sizeof((aulCmdIndex[0]));

    pstCmdElement = g_stParseContext[ucIndex].pstCmdElement;

    /* �ж��·���AT�����cmdindex�Ƿ��������е���GULģʽ�²����д����AT�����index */
    for (ulLoop = 0; ulLoop < ulCmdArraySize; ulLoop++)
    {
        if (pstCmdElement->ulCmdIndex == aulCmdIndex[ulLoop])
        {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}
#endif



