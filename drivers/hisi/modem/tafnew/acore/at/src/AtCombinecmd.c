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

#include "hi_list.h"

#include "at_common.h"
#include "AtCmdCallProc.h"

#include "AtCombinecmd.h"
#include "securec.h"


#define    THIS_FILE_ID        PS_FILE_ID_AT_COMBINEDCMD_C

VOS_UINT32 At_GetSecondAddr(VOS_UINT8 *pData,VOS_UINT16 usLen, VOS_UINT8** ppDataOut)
{
    errno_t    lMemResult;
    VOS_UINT8* pucFirAddr = NULL;
    VOS_UINT8* pucSecAddr = NULL;
    VOS_UINT32 ulBasicCmdLen = 0;
    VOS_UINT8* pcTmp = NULL;

    /* �Ƚ��ַ�����Ҫ��֤�ַ���������\0 */
    pcTmp = (VOS_UINT8*)AT_MALLOC(usLen+1);
    if(pcTmp == NULL)
    {
        return ERR_MSP_FAILURE;
    }

    lMemResult = memcpy_s(pcTmp, usLen+1, pData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen+1, usLen);
    pcTmp[usLen] = '\0';

    pucFirAddr = At_GetFirstBasicCmdAddr(pcTmp, &ulBasicCmdLen);
    if(pucFirAddr == NULL)
    {
        AT_FREE(pcTmp);
        return ERR_MSP_FAILURE;
    }

    pucSecAddr = At_GetFirstBasicCmdAddr((pucFirAddr + ulBasicCmdLen), &ulBasicCmdLen);

    if(pucSecAddr == NULL)
    {
        *ppDataOut = pData + usLen;
    }
    else
    {
        *ppDataOut = pData + (pucSecAddr - pcTmp);
    }

    AT_FREE(pcTmp);
    return ERR_MSP_SUCCESS;
}



VOS_UINT32 At_GetFirstCmdLen( VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    VOS_UINT8* pucBegin  = pData;
    VOS_UINT8* pucEnd    = pData;
    VOS_UINT16 usLenFir  = 0;

    if(At_GetSecondAddr(pucBegin, usLen, &pucEnd) == ERR_MSP_SUCCESS)
    {
        usLenFir = (VOS_UINT16)(pucEnd - pucBegin);
    }
    else
    {
        /* ������ҵ�һ������ʧ�ܣ��򷵻������ַ����ĳ��� */
        usLenFir = usLen;
    }

    return usLenFir;
}



VOS_VOID At_ResetCombinCmdInfo(HI_LIST_S* pstCombList)
{
    AT_FW_COMBINE_CMD_INFO_STRU        *pstCombineCmdInfo = VOS_NULL_PTR;
    AT_FW_COMBINE_CMD_NODE_STRU        *pstNode = VOS_NULL_PTR;
    HI_LIST_S                          *pstTmp = VOS_NULL_PTR;
    HI_LIST_S                          *me = VOS_NULL_PTR;

    if ((pstCombList == VOS_NULL_PTR) || (pstCombList->next == VOS_NULL_PTR))
    {
        return ;
    }

    /* �������в���ÿ���������� */
    msp_list_for_each_safe(me, pstTmp, pstCombList)
    {
        if (me == NULL)
        {
            pstCombineCmdInfo = msp_list_entry(pstCombList, AT_FW_COMBINE_CMD_INFO_STRU, stCombineCmdList);
            pstCombineCmdInfo->usTotalNum = 0;
            pstCombineCmdInfo->usProcNum  = 0;
            /*lint -e717*/
            HI_INIT_LIST_HEAD(pstCombList);
            /*lint +e717*/
            return;
        }

        pstNode = msp_list_entry(me, AT_FW_COMBINE_CMD_NODE_STRU, stCombCmdList);
        /*lint -e774*/
        if (pstNode == VOS_NULL_PTR)
        {
            pstCombineCmdInfo = msp_list_entry(pstCombList, AT_FW_COMBINE_CMD_INFO_STRU, stCombineCmdList);
            pstCombineCmdInfo->usTotalNum = 0;
            pstCombineCmdInfo->usProcNum  = 0;
            /*lint -e717*/
            HI_INIT_LIST_HEAD(pstCombList);
            /*lint +e717*/
            return;
        }
        /*lint +e774*/

        msp_list_del(&(pstNode->stCombCmdList));
        AT_FREE(pstNode->pstCombCmd);
        AT_FREE(pstNode);
    }

    pstCombineCmdInfo = msp_list_entry(pstCombList, AT_FW_COMBINE_CMD_INFO_STRU, stCombineCmdList);
    pstCombineCmdInfo->usTotalNum = 0;
    pstCombineCmdInfo->usProcNum  = 0;

    return ;
}



VOS_VOID At_ResetCombinParseInfo(VOS_UINT8 ucIndex)
{
    HI_LIST_S* pstCombList = NULL;
    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        return;
    }

    pClientContext = &(g_stParseContext[ucIndex]);

    pstCombList = &pClientContext->stCombineCmdInfo.stCombineCmdList;

    if(pstCombList->next == NULL)
    {
        /*lint -e717*/
        HI_INIT_LIST_HEAD(&(pClientContext->stCombineCmdInfo.stCombineCmdList));
        /*lint -e717*/
    }

    At_ResetCombinCmdInfo(pstCombList);

    /*������buffer*/
    gstAtCombineSendData[ucIndex].usBufLen = 0;

    return ;
}


/* ��ӡ�����������������Ϣ�������� */
VOS_VOID At_PrintCombinCmd(VOS_VOID)
{
    VOS_UINT32 i = 0, j = 0;
    HI_LIST_S* me = NULL;
    HI_LIST_S* pstCombList = NULL;
    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;
    AT_FW_COMBINE_CMD_STRU* pstCombCmd = NULL;
    AT_FW_COMBINE_CMD_NODE_STRU* pstCombCmdNode = NULL;

    for(j = 0; j < AT_MAX_CLIENT_NUM; j++)
    {
        pClientContext = &(g_stParseContext[j]);

        pstCombList = &pClientContext->stCombineCmdInfo.stCombineCmdList;

        /* ��client�����в���ָ����ClientId */
        msp_list_for_each(me, pstCombList)
        {
            pstCombCmdNode = msp_list_entry(me, AT_FW_COMBINE_CMD_NODE_STRU, stCombCmdList);
            pstCombCmd = pstCombCmdNode->pstCombCmd;

            if(pstCombCmd == NULL)
            {
                continue;
            }

            PS_PRINTF_INFO("-%d-:", pstCombCmd->ulLen);

            for(i = 0; i < pstCombCmd->ulLen; i++)
            {
                PS_PRINTF_INFO("%c", pstCombCmd->ucData[i]);
            }
        }
    }

    return ;
}

/*lint -save -e429*/

LOCAL VOS_UINT32 At_StoreSubCombCmd(HI_LIST_S* pstCombList, VOS_UINT8 *pDataIn, VOS_UINT16 usLenIn)
{
    AT_FW_COMBINE_CMD_STRU* pstCombCmd = NULL;
    AT_FW_COMBINE_CMD_NODE_STRU* pstCombCmdNode = NULL;/*lint !e830*/
    AT_FW_COMBINE_CMD_INFO_STRU* pstCombineCmdInfo = NULL;
    errno_t                      lMemResult;

    /* ���õĵط���ָ֤�벻Ϊ�� */

    pstCombineCmdInfo = msp_list_entry(pstCombList, AT_FW_COMBINE_CMD_INFO_STRU, stCombineCmdList);
    if(pstCombineCmdInfo->usTotalNum >= AT_MAX_NUM_COMBINE_CMD)
    {
        PS_PRINTF_WARNING("usTotalNum %d\n", pstCombineCmdInfo->usTotalNum);
        return ERR_MSP_FAILURE;
    }

    pstCombCmdNode = (AT_FW_COMBINE_CMD_NODE_STRU*)AT_MALLOC(sizeof(AT_FW_COMBINE_CMD_NODE_STRU));
    if(pstCombCmdNode == NULL)
    {
        return ERR_MSP_MALLOC_FAILUE;
    }

    pstCombCmd = (AT_FW_COMBINE_CMD_STRU*)AT_MALLOC(sizeof(AT_FW_COMBINE_CMD_STRU) + usLenIn + AT_FW_LEN_AT);
    if(pstCombCmd == NULL)
    {
        AT_FREE(pstCombCmdNode);

        return ERR_MSP_MALLOC_FAILUE;
    }
    memset_s(pstCombCmd,
             (VOS_SIZE_T)sizeof(AT_FW_COMBINE_CMD_STRU) + usLenIn + AT_FW_LEN_AT,
             0x00,
             (VOS_SIZE_T)sizeof(AT_FW_COMBINE_CMD_STRU) + usLenIn + AT_FW_LEN_AT);

    pstCombCmdNode->pstCombCmd = pstCombCmd;
    pstCombCmd->ulLen = usLenIn + AT_FW_LEN_AT;
    lMemResult = memcpy_s(pstCombCmd->ucData, usLenIn + AT_FW_LEN_AT, "AT", AT_FW_LEN_AT);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLenIn + AT_FW_LEN_AT, AT_FW_LEN_AT);
    lMemResult = memcpy_s(&(pstCombCmd->ucData[AT_FW_LEN_AT]), usLenIn, pDataIn, usLenIn);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLenIn, usLenIn);

    msp_list_add_tail(&pstCombCmdNode->stCombCmdList, pstCombList);

    pstCombineCmdInfo->usTotalNum++;

    return ERR_MSP_SUCCESS;
}
/*lint -restore */

/* �������������� */
static VOS_UINT32 At_BasicCombineCmdParse(HI_LIST_S* pstCombList, VOS_UINT8 *pDataIn, VOS_UINT16 usLenIn)
{
    VOS_UINT32 ulRet = ERR_MSP_FAILURE;
    VOS_UINT16 usLenFir = 0, usChkLen = 0;
    VOS_UINT8* pData = pDataIn;

    while(usChkLen < usLenIn)
    {
        usLenFir = (VOS_UINT16)At_GetFirstCmdLen(pData, (usLenIn - usChkLen));

        ulRet = At_StoreSubCombCmd(pstCombList, pData, usLenFir);
        if(ulRet != ERR_MSP_SUCCESS)
        {
            At_ResetCombinCmdInfo(pstCombList);
            return ulRet;
        }

        usChkLen = usChkLen + usLenFir;
        pData = pDataIn + usChkLen;
    }

    return ERR_MSP_SUCCESS;
}


/* �����������չ������Ͻ��� */
VOS_UINT32 At_BasicExCombineCmdParse(HI_LIST_S* pstCombList, VOS_UINT8 *pDataIn, VOS_UINT16 usLenIn, VOS_UINT16 usFirIndex)
{
    VOS_UINT32 ulRet = ERR_MSP_FAILURE;
    VOS_UINT8* pData = pDataIn;

    /* ���õĵط���֤pstCombList��pDataIn��Ϊ�� */

    if(usFirIndex >= usLenIn)
    {
        return ERR_MSP_FAILURE;
    }

    /* ��0���ַ�����"+^$"����֮ǰ���ǻ������� */
    if(usFirIndex != 0)
    {
        ulRet = At_BasicCombineCmdParse(pstCombList, pData, usFirIndex);
        if(ulRet != ERR_MSP_SUCCESS)
        {
            At_ResetCombinCmdInfo(pstCombList);
            return ulRet;
        }
    }

    ulRet = At_StoreSubCombCmd(pstCombList, (pData+usFirIndex), (usLenIn-usFirIndex));
    if(ulRet != ERR_MSP_SUCCESS)
    {
        At_ResetCombinCmdInfo(pstCombList);
    }

    return ulRet;
}



static VOS_VOID At_UpStringCmdName(VOS_UINT8 *pData, VOS_UINT16 usLen)
{
    TAF_UINT8  *pTmp  = pData;                 /* current Char */
    TAF_UINT16 ChkLen = 0;

    if(usLen == 0)
    {
        return ;
    }

    while((ChkLen++ < usLen) && ((*pTmp) != '='))
    {
        if ( (*pTmp >= 'a') && (*pTmp <= 'z'))
        {
            *pTmp = *pTmp - 0x20;
        }
        pTmp++;
    }

    return ;
}


VOS_UINT32 At_SemicolonCmdParse(HI_LIST_S* pstCombList, VOS_UINT8 *pDataIn, VOS_UINT16 usLenIn)
{
    VOS_UINT32 ulRet = ERR_MSP_FAILURE;
    VOS_UINT16 usNumQuota = 0;
    VOS_UINT16 usNumSymbol = 0;
    VOS_UINT16 usFirIndex = 0;
    VOS_UINT16 i = 0;

    if((pstCombList == NULL) || (pDataIn == NULL))
    {
        return ERR_MSP_FAILURE;
    }

    if(At_CheckCharD(*pDataIn) != AT_SUCCESS)
    {
        At_UpStringCmdName(pDataIn, usLenIn);
    }
    else
    {
        At_UpStringCmdName(pDataIn, 1);
    }

    /* ';'����������Ľ�β������D���ֱ�Ӵ���Ϊ������������ */
    if(usLenIn > 2)
    {
        if(At_CheckCharD(*pDataIn) == AT_SUCCESS)
        {
            if(At_CheckSemicolon(*(pDataIn+usLenIn-1)) == AT_SUCCESS)
            {
                ulRet = At_StoreSubCombCmd(pstCombList, pDataIn, usLenIn);
                if(ulRet != ERR_MSP_SUCCESS)
                {
                    At_ResetCombinCmdInfo(pstCombList);
                }

                return ulRet;
            }
        }
    }

    /* ATD+117���ͣ�ֱ�Ӵ���Ϊ���ݺ������� */
    if(usLenIn > 2)
    {
        if((At_CheckCharD(*pDataIn) == AT_SUCCESS) && (pDataIn[1] == '+'))
        {
            ulRet = At_StoreSubCombCmd(pstCombList, pDataIn, usLenIn);
            if(ulRet != ERR_MSP_SUCCESS)
            {
                At_ResetCombinCmdInfo(pstCombList);
            }

            return ulRet;
        }
    }

    /* ͳ�������������չ�����ʶ������ */
    while(i < usLenIn)
    {
        if (pDataIn[i] == '"')
        {
            usNumQuota++;
        }
        else if (((pDataIn[i] == '+') || (pDataIn[i] == '^') || (pDataIn[i] == '$'))
             && ((usNumQuota % 2) == 0))
        {
            usNumSymbol++;
            if (usNumSymbol == 1)
            {
                usFirIndex = i; /* ��¼��һ��"+^$"��λ�� */
            }
        }
        else
        {
        }

        i++;
    }

    /* ��չ�����ʶ������Ϊ0���������������Ϊ���ɻ���AT�������� */
    if(usNumSymbol == 0)
    {
        ulRet = At_BasicCombineCmdParse(pstCombList, pDataIn, usLenIn);
    }
    /* ��չ�����ʶ������Ϊ1�������ǵ���һ����չ���Ҳ���������ɻ���������Ϻ�һ����չ����  */
    else if(usNumSymbol == 1)
    {
        ulRet = At_BasicExCombineCmdParse(pstCombList, pDataIn, usLenIn, usFirIndex);
    }
    /* ��������ÿ�������У���չ�����ʶ��('+'��'^'��'$')���ֻ�ܳ���һ�� */
    else
    {
        ulRet = ERR_MSP_FAILURE;
    }

    return ulRet;
}



PRIVATE VOS_UINT32 At_CombineCmdPreProc(HI_LIST_S* pstCombList, VOS_UINT8 *pDataIn, VOS_UINT16 usLenIn)
{
    VOS_UINT16 i = 0;
    VOS_UINT32 ulRet = ERR_MSP_FAILURE;
    VOS_UINT16 usNumQuota = 0;
    VOS_UINT8* pData = pDataIn;
    VOS_UINT8* pDataHead = pDataIn;

    if((pstCombList == NULL) || (pDataIn == NULL))
    {
        return ERR_MSP_FAILURE;
    }

    while( i++ < usLenIn )
    {
        /* �ֺ��������� */
        if((*pData == ';') && ((usNumQuota%2) == 0))
        {
            ulRet = At_SemicolonCmdParse(pstCombList, pDataHead, (VOS_UINT16)(pData- pDataHead + 1));
            if(ulRet != ERR_MSP_SUCCESS)
            {
                return ulRet;
            }

            pDataHead = (pData + 1);
        }
        else if(*pData == '"')
        {
            usNumQuota++;
        }
        else
        {
        }

        pData++;
    }

    /* ���һ���ַ����Ƿֺ�ʱҪ�����洢���һ��������� */
    if(pDataHead != (pDataIn + usLenIn))
    {
        ulRet = At_SemicolonCmdParse(pstCombList, pDataHead, (VOS_UINT16)(pData- pDataHead));
        if(ulRet != ERR_MSP_SUCCESS)
        {
            return ulRet;
        }
    }

    return ERR_MSP_SUCCESS;
}


VOS_UINT32 AT_IsDCmdValidChar(
    VOS_UINT8                           ucPara
)
{
    /* ֧��dialing digits: 0-9,*,#,A-D */
    if ((ucPara >= '0') && (ucPara <= '9'))
    {
        return VOS_TRUE;
    }

    if ((ucPara >= 'a') && (ucPara <= 'c'))
    {
        return VOS_TRUE;
    }

    if ((ucPara >= 'A') && (ucPara <= 'C'))
    {
        return VOS_TRUE;
    }

    if ((ucPara == '*') || (ucPara == '#'))
    {
        return VOS_TRUE;
    }

    /* ֧��GSM/UMTS modifier characters: > i g I G */
    if (ucPara == '>')
    {
        return VOS_TRUE;
    }

    if ((ucPara == 'i') || (ucPara == 'I'))
    {
        return VOS_TRUE;
    }

    if ((ucPara == 'g') || (ucPara == 'G'))
    {
        return VOS_TRUE;
    }

    if (ucPara == '+')
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_VOID AT_InsertDCmdGIPara(
    VOS_UINT32                  *pulSrcStrLen,
    VOS_UINT8                   *pucSrcStr,
    VOS_UINT32                   ulMaxMemLength,
    VOS_UINT32                   ulInsertStrLen,
    VOS_UINT8                   *pucInsertStr
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulSrcStrLen;

    ulSrcStrLen = *pulSrcStrLen;

    if (pucSrcStr[ulSrcStrLen - 1] == ';')
    {
        lMemResult = memcpy_s(&(pucSrcStr[ulSrcStrLen - 1]), ulMaxMemLength - (ulSrcStrLen - 1), pucInsertStr, ulInsertStrLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxMemLength - (ulSrcStrLen - 1), ulInsertStrLen);

        ulSrcStrLen += ulInsertStrLen;

        pucSrcStr[ulSrcStrLen - 1] = ';';
    }
    else
    {
        lMemResult = memcpy_s(&(pucSrcStr[ulSrcStrLen]), ulMaxMemLength - ulSrcStrLen, pucInsertStr, ulInsertStrLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxMemLength - ulSrcStrLen, ulInsertStrLen);

        ulSrcStrLen += ulInsertStrLen;
    }

    *pulSrcStrLen = ulSrcStrLen;

    return;
}


VOS_VOID AT_ProcDCmdGIPara(
    VOS_UINT32                         *pulSrcStrLen,
    VOS_UINT8                          *pucSrcStr
)
{
    VOS_UINT32                          ulLoop;
    VOS_BOOL                            bIExist;
    VOS_BOOL                            biExist;
    VOS_BOOL                            bGExist;
    VOS_BOOL                            bgExist;
    VOS_UINT8                           aucInsertStr[AT_CALL_D_GI_PARA_LEN];
    VOS_UINT32                          ulInsertStrLen;
    VOS_UINT32                          ulPos;
    VOS_UINT32                          ulDstStrLen;
    VOS_UINT8                          *pucDstPara = VOS_NULL_PTR;
    VOS_UINT32                          ulMaxMemLength;

    bIExist = VOS_FALSE;
    biExist = VOS_FALSE;
    bGExist = VOS_FALSE;
    bgExist = VOS_FALSE;

    ulDstStrLen = *pulSrcStrLen;
    ulMaxMemLength = *pulSrcStrLen;
    pucDstPara  = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, ulDstStrLen);
    if (pucDstPara == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_ProcDCmdGIPara : fail to alloc memory . ");
        return;
    }

    /*
    ��ȡD�����GI�������ԣ�������ڳ�ͻ��I��i�ַ������һ���ַ�����Ϊ׼��
    ɾ���ַ����е������ַ�GgIi
    */
    for (ulLoop = 0, ulPos   = 0; ulLoop < ulDstStrLen; ulLoop++)
    {
        if (pucSrcStr[ulLoop] == 'I')
        {
            bIExist = VOS_TRUE;
            biExist = VOS_FALSE;
            continue;
        }

        if (pucSrcStr[ulLoop] == 'i')
        {
            biExist = VOS_TRUE;
            bIExist = VOS_FALSE;
            continue;
        }

        if (pucSrcStr[ulLoop] == 'G')
        {
            bGExist = VOS_TRUE;
            continue;
        }

        if (pucSrcStr[ulLoop] == 'g')
        {
            bgExist = VOS_TRUE;
            continue;
        }

        *(pucDstPara + ulPos) = *(pucSrcStr + ulLoop);

        ulPos++;
    }

    ulDstStrLen         = ulPos;

    /* �����޸ĵ��û������ַ��� */
    *pulSrcStrLen       = ulDstStrLen;
    TAF_MEM_MOVE_S(pucSrcStr, ulDstStrLen, pucDstPara, ulDstStrLen);

    PS_MEM_FREE(WUEPS_PID_AT, pucDstPara);

    /* ����GI�����ַ��� */
    memset_s(aucInsertStr, sizeof(aucInsertStr), 0x00, sizeof(aucInsertStr));

    ulInsertStrLen = 0;
    if (bIExist == VOS_TRUE)
    {
        aucInsertStr[ulInsertStrLen] = 'I';
        ulInsertStrLen++;
    }
    else
    {
        if (biExist == VOS_TRUE)
        {
            aucInsertStr[ulInsertStrLen] = 'i';
            ulInsertStrLen++;
        }
    }

    if ((bGExist == VOS_TRUE)
     || (bgExist == VOS_TRUE))
    {
        aucInsertStr[ulInsertStrLen] = 'G';
        ulInsertStrLen++;
    }

    /* ��GI�����ַ������뵽���к���ͷֺ��ַ�֮�� */
    AT_InsertDCmdGIPara(pulSrcStrLen, pucSrcStr, ulMaxMemLength, ulInsertStrLen, aucInsertStr);

    return;
}



VOS_VOID At_FilterDCmdSpecCharacter(
    VOS_UINT32                  *pulParaLen,
    VOS_UINT8                   *pucPara
)
{
    VOS_UINT16                  i;
    VOS_UINT16                  usBeginPos;
    VOS_UINT32                  ulRet;

    if (*pulParaLen <= 3)
    {
        return;
    }

    /* �������ATD����������� */
    /* ��Ϊ'a'����'A'ʱ���� */
    if(At_CheckCharA(pucPara[0]) != AT_SUCCESS)
    {
        return;
    }
    /* ��Ϊ't'����'T'ʱ���� */
    if(At_CheckCharT(pucPara[1]) != AT_SUCCESS)
    {
        return;
    }
    /* ��Ϊ'D'����'d'ʱ���� */
    if (At_CheckCharD(pucPara[2]) != AT_SUCCESS)
    {
        return ;
    }

    usBeginPos = 3;
    /* ISDN���ŵ�ATDI<STRING>��ATD<STRING>��ͬ��ɾ��I�ַ����ִ�������һ���� */
    if (pucPara[usBeginPos] == 'I')
    {
        *pulParaLen = *pulParaLen - 1;
        TAF_MEM_MOVE_S(&(pucPara[usBeginPos]), *pulParaLen - usBeginPos, &(pucPara[usBeginPos + 1]), *pulParaLen - usBeginPos);
    }

    /* ��һ���ַ�Ϊ'+'ʱ��Ϊ���ʺ����ʾ,���ܹ��� */
    if (pucPara[usBeginPos] == '+')
    {
        usBeginPos++;
    }

    /* ��ΪATD��������һ���ַ�һ�����ַ���������'\0',�������������λ�����в�����ȡֵ�Ƿ�Խ�� */
    for (i = usBeginPos; i < *pulParaLen; i++)
    {
        ulRet = AT_IsDCmdValidChar(pucPara[i]);
        if (ulRet == VOS_TRUE)
        {
            continue;
        }

        /* ���һ��������';'ʱ������ */
        if (i == (*pulParaLen - 1))
        {
            if (pucPara[i] == ';')
            {
                continue;
            }
        }

        *pulParaLen = *pulParaLen - 1;
        TAF_MEM_MOVE_S(&(pucPara[i]), *pulParaLen - i, &(pucPara[i + 1]), *pulParaLen - i);
        i--;
    }

    AT_ProcDCmdGIPara(pulParaLen, pucPara);

    return;
}


VOS_UINT32 At_CombineCmdProc(VOS_UINT8 ucClientId)
{
    VOS_UINT32 i = 0;
    HI_LIST_S* me = NULL;
    HI_LIST_S* pstListHead = NULL;
    VOS_UINT32 ulRet;
    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;

    AT_FW_COMBINE_CMD_STRU*      pstCombCmd         = NULL;
    AT_FW_COMBINE_CMD_NODE_STRU* pstCombCmdNode     = NULL;
    AT_FW_COMBINE_CMD_INFO_STRU* pstCombineCmdInfo  = NULL;

    AT_RRETURN_CODE_ENUM_UINT32 ulResult;

    /* �ú������ô��ɱ�֤ucClientId�ĺϷ��ԣ�pDataIn��Ϊ�� */

    pClientContext = &(g_stParseContext[ucClientId]);

    pstCombineCmdInfo = &pClientContext->stCombineCmdInfo;
    pstListHead = &pstCombineCmdInfo->stCombineCmdList;

    /* �жϺϷ��� */
    if((pstCombineCmdInfo->usTotalNum == 0) || (pstCombineCmdInfo->usTotalNum <= pstCombineCmdInfo->usProcNum))
    {
        return AT_ERROR;
    }

    /* ��������������в��Ҵ���������� */
    msp_list_for_each(me, pstListHead)
    {
        if(i < pstCombineCmdInfo->usProcNum)
        {
            i++;
            continue;
        }

        pstCombCmdNode = msp_list_entry(me, AT_FW_COMBINE_CMD_NODE_STRU, stCombCmdList);
        pstCombCmd = pstCombCmdNode->pstCombCmd;
        break;
    }

    /* û���ҵ������������ */
    if((i != pstCombineCmdInfo->usProcNum) || (pstCombCmd == NULL))
    {
        return AT_ERROR;
    }

    /* ����Э�顶T-REC-V[1].250-200307-I_MSW-E.doc��
    6.3.1	Dial
        Any characters appearing in the dial string that the DCE does not recognize
        as a valid part of the call addressing information or as a valid modifier
        shall be ignored. This permits characters such as parentheses and hyphens
        to be included that are typically used in formatting of telephone numbers.
      ���԰������ַ����˵� */

    At_FilterDCmdSpecCharacter(&(pstCombCmd->ulLen), pstCombCmd->ucData);

    /* ���������ַ��� */
    ulResult = (AT_RRETURN_CODE_ENUM_UINT32)AT_ParseCmdType(pstCombCmd->ucData, (VOS_UINT16)pstCombCmd->ulLen);

    /* �����������ش����� */
    if(ulResult != ERR_MSP_SUCCESS)
    {
        if(atMatchCmdName(ucClientId, g_stATParseCmd.ucCmdFmtType) == ERR_MSP_SUCCESS)
        {
            if(g_stParseContext[ucClientId].pstCmdElement != NULL)
            {
                return g_stParseContext[ucClientId].pstCmdElement->ulParamErrCode;
            }
        }

        return ulResult;
    }

    AT_MNTN_SaveCurCmdName(ucClientId);

    /* ����� */
    ulRet = CmdParseProc(ucClientId, pstCombCmd->ucData, (VOS_UINT16)pstCombCmd->ulLen);
    if((ulRet != AT_OK) && (ulRet != AT_WAIT_ASYNC_RETURN))
    {
        At_ResetCombinCmdInfo(pstListHead);
    }

    return ulRet;
}



VOS_UINT32 At_CombineCmdChkProc(VOS_UINT8 ucClientId,  VOS_UINT8 *pDataIn, VOS_UINT16 usLenIn)
{
    VOS_UINT16 usLen = usLenIn;
    VOS_UINT8* pData = pDataIn;
    VOS_UINT32 ulRet;

    AT_PARSE_CONTEXT_STRU* pClientContext = NULL;
    HI_LIST_S* pstListHead = NULL;

    /* �ú������ô��ɱ�֤ucClientId�ĺϷ��ԣ�pDataIn��Ϊ�� */

    pClientContext = &(g_stParseContext[ucClientId]);
    pstListHead = &pClientContext->stCombineCmdInfo.stCombineCmdList;

    /* ǰ�����ַ�����AT��ֱ�ӷ���ʧ�� */
    if(At_CheckCharA(pData[0]) != AT_SUCCESS)
    {
        return AT_ERROR;
    }

    if(At_CheckCharT(pData[1]) != AT_SUCCESS)
    {
        return AT_ERROR;
    }

    /* "AT" or "AT;" */
    if(usLenIn == AT_FW_LEN_AT)
    {
        return AT_OK;
    }

    /* �������ַ��Ƿֺ� */
    if(usLenIn == 3)
    {
        if(At_CheckSemicolon(pData[2]) == AT_SUCCESS)
        {
            return AT_OK;
        }
    }

    /* ����ǰ����"AT"�ַ�������ͳһ���� */
    pData = pData + AT_FW_LEN_AT;
    usLen = usLen - AT_FW_LEN_AT;

    /* ��������Ԥ�����������ַ��뻺���� */
    ulRet = At_CombineCmdPreProc(pstListHead, pData, usLen);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        return AT_ERROR;
    }

    ulRet = At_CombineCmdProc(ucClientId);

    return ulRet;
}



VOS_VOID At_CombCmdProcAfterCmd(VOS_UINT8 ucClientId)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_FAILURE;
    AT_PARSE_CONTEXT_STRU* pstClientCont = NULL;
    AT_FW_COMBINE_CMD_INFO_STRU* pstCombineCmdInfo = NULL;

    if(ucClientId >= AT_MAX_CLIENT_NUM)
    {
        return;
    }

    pstClientCont = &(g_stParseContext[ucClientId]);

    pstCombineCmdInfo = &pstClientCont->stCombineCmdInfo;

    /* ��ǰͨ����δ��������� */
    if(pstCombineCmdInfo->usProcNum < pstCombineCmdInfo->usTotalNum)
    {
        ulResult = (AT_RRETURN_CODE_ENUM_UINT32)At_CombineCmdProc(ucClientId);  /* TODO: */
        if(ulResult != AT_WAIT_ASYNC_RETURN)
        {
            At_FormatResultData(ucClientId, ulResult);
        }
    }
    else
    {
        return;
    }
}



VOS_BOOL At_CombCmdisFinal(VOS_UINT8 ucIndex)
{
    AT_PARSE_CONTEXT_STRU* pstClientCont = NULL;
    AT_FW_COMBINE_CMD_INFO_STRU* pstCombineCmdInfo = NULL;

    pstClientCont = &(g_stParseContext[ucIndex]);

    pstCombineCmdInfo = &pstClientCont->stCombineCmdInfo;

    if(pstCombineCmdInfo->usTotalNum == 0)
    {
        return VOS_TRUE;
    }

    if(pstCombineCmdInfo->usProcNum < (pstCombineCmdInfo->usTotalNum - 1))
    {
        return VOS_FALSE;
    }
    else
    {
        return VOS_TRUE;
    }
}



