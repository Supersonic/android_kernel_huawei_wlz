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
  1 Include HeadFile
*****************************************************************************/
#include "diag_acore_common.h"
#include <linux/debugfs.h>
#include <vos.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <msp.h>
#include <omerrorlog.h>
#include <nv_stru_sys.h>
#include <soc_socp_adapter.h>
#include <eagleye.h>
#include "diag_common.h"
#include "diag_msgbsp.h"
#include "diag_msgbbp.h"
#include "diag_msg_easyrf.h"
#include "diag_msgphy.h"
#include "diag_msgps.h"
#include "diag_msghifi.h"
#include "diag_msgapplog.h"
#include "diag_api.h"
#include "diag_cfg.h"
#include "diag_debug.h"
#include "diag_connect.h"
#include "diag_msglrm.h"
#include "diag_msgnrm.h"
#include "diag_msgapplog.h"
#include "diag_time_stamp.h"
#include "diag_message.h"
#include "diag_msg_def.h"
#include "diag_time_stamp.h"


/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_ACORE_COMMON_C

DRV_RESET_CB_MOMENT_E g_DiagResetingCcore = MDRV_RESET_CB_INVALID;

VOS_UINT32 g_ulDebugCfg = 0;

DIAG_DUMP_INFO_STRU g_stDumpInfo = {0};

extern DIAG_TRANS_HEADER_STRU g_stPSTransHead;

struct wakeup_source diag_wakelock;

/*****************************************************************************
  3 Function
*****************************************************************************/

extern VOS_VOID SCM_StopAllSrcChan(VOS_VOID);

/*****************************************************************************
 Function Name   : diag_ResetCcoreCB
 Description     : ���modem������λ�ص�����
 Input           : enParam
 Output          : None
 Return          : VOS_VOID
*****************************************************************************/
VOS_INT diag_ResetCcoreCB(DRV_RESET_CB_MOMENT_E enParam, int userdata)
{
    DRV_DIAG_TRANS_IND_STRU stResetMsg = {};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;

    if(enParam == MDRV_RESET_CB_BEFORE)
    {
        diag_crit("Diag receive ccore reset Callback.\n");

        g_DiagResetingCcore = MDRV_RESET_CB_BEFORE;

        if(!DIAG_IS_CONN_ON)
        {
            return ERR_MSP_SUCCESS;
        }

        stResetMsg.ulModule = DRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_LTE, DIAG_MSG_TYPE_MSP);
        stResetMsg.ulPid    = MSP_PID_DIAG_APP_AGENT;
        stResetMsg.ulMsgId  = DIAG_CMD_MODEM_WILL_RESET;
        stResetMsg.ulReserve= 0;
        stResetMsg.ulLength = sizeof(VOS_UINT32);
        stResetMsg.pData    = (void *)&ret;

        ret = mdrv_diag_report_reset_msg(&stResetMsg);
        if(ret)
        {
            diag_error("report reset msg fail, ret:0x%x\n", ret);
        }

        diag_crit("Diag report ccore reset to HIDP,and reset SOCP timer.\n");
        /* modem������λʱ�����жϳ�ʱʱ��ָ�ΪĬ��ֵ����HIDP�����յ���λ��Ϣ */
        mdrv_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
#ifdef DIAG_SEC_TOOLS
        g_ulAuthState = DIAG_AUTH_TYPE_DEFAULT;
#endif
    }
    else if(enParam == MDRV_RESET_CB_AFTER)
    {
        g_DiagResetingCcore = MDRV_RESET_CB_AFTER;

    }
    else
    {
        diag_error("enParam(0x%x) error\n", enParam);
    }
    return ERR_MSP_SUCCESS;
}


VOS_UINT32 diag_AppAgentMsgProcInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_CHAR * resetName = "DIAG";  /*C�˵�����λ������*/
    VOS_INT    resetLevel = 49;

    if(ip == VOS_IP_LOAD_CONFIG)
    {
        wakeup_source_init(&diag_wakelock, "diag_wakelock");
        ret = (VOS_UINT32)mdrv_sysboot_register_reset_notify(resetName, (pdrv_reset_cbfun)diag_ResetCcoreCB, 0, resetLevel);
        if(ret)
        {
            diag_error("diag register ccore reset fail\n");
        }

        diag_MspMsgInit();
        diag_BspMsgInit();
        diag_DspMsgInit();
        diag_EasyRfMsgInit();
        diag_BbpMsgInit();
        diag_PsMsgInit();
        diag_HifiMsgInit();
        diag_MessageInit();
        diag_AppLogMsgInit();
        diag_LRMMsgInit();
        diag_NrmMsgInit();
    }
    else if(ip == VOS_IP_RESTART)
    {
        diag_ConnReset();

        /* ����ʼ�������������ÿ��� */
        diag_CfgResetAllSwt();
        /* ����Ƿ���Ҫ�򿪿���log */
        mdrv_scm_set_power_on_log();

        if(VOS_TRUE == diag_IsPowerOnLogOpen())
        {
            /* �ڴ򿪿���logǰ������һ��ʱ�����λ������ */
            diag_PushHighTs();
            diag_StartHighTsTimer();
            g_ulDiagCfgInfo |= DIAG_CFG_POWERONLOG;

            /* ����log������ά����Ϣ */
            diag_StartMntnTimer(DIAG_OPENLOG_MNTN_TIMER_LEN);
        }

        mdrv_scm_reg_ind_coder_dst_send_fuc();

        diag_crit("Diag PowerOnLog is %s.\n", (g_ulDiagCfgInfo&DIAG_CFG_POWERONLOG) ? "open" : "close");
    }

    return ret;
}



VOS_VOID diag_DumpMsgInfo(VOS_UINT32 ulSenderPid, VOS_UINT32 ulMsgId, VOS_UINT32 ulSize)
{
    VOS_UINT32 ulPtr = g_stDumpInfo.ulMsgCur;

    if(g_stDumpInfo.pcMsgAddr)
    {
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr]))    = ulSenderPid;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr]))  = ulMsgId;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr]))  = ulSize;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr])) = mdrv_timer_get_normal_timestamp();

        g_stDumpInfo.ulMsgCur = (g_stDumpInfo.ulMsgCur + 16);
        if(g_stDumpInfo.ulMsgCur >= g_stDumpInfo.ulMsgLen)
        {
            g_stDumpInfo.ulMsgCur = 0;
        }
    }
}

/* DUMP�洢����Ϣ����󳤶ȣ�����64��ʾ����0xaa5555aa��֡ͷ����Ϣ���ݵ��ܵ���󳤶� */
#define DIAG_DUMP_MAX_FRAME_LEN          (80)


VOS_VOID diag_DumpDFInfo(DIAG_FRAME_INFO_STRU * pFrame)
{
    VOS_UINT32 ulPtr;
    VOS_UINT32 ulLen;

    if(g_stDumpInfo.pcDFAddr)
    {
        ulPtr = g_stDumpInfo.ulDFCur;

        *((VOS_INT32*)(&g_stDumpInfo.pcDFAddr[ulPtr])) =(VOS_INT32)0xaa5555aa;
        *((VOS_INT32*)(&g_stDumpInfo.pcDFAddr[ulPtr+sizeof(VOS_UINT32)])) = mdrv_timer_get_normal_timestamp();

        /* ÿ�����ݶ���16�ֽڶ��룬����ؾ� */
        g_stDumpInfo.ulDFCur = g_stDumpInfo.ulDFCur + 8;

        ulPtr = g_stDumpInfo.ulDFCur;

        ulLen = 8 + sizeof(DIAG_FRAME_INFO_STRU) + pFrame->ulMsgLen;
        if(ulLen > DIAG_DUMP_MAX_FRAME_LEN)
        {
            ulLen = DIAG_DUMP_MAX_FRAME_LEN;
        }

        ulLen = ((ulLen + 0xf) & (~0xf)) - 8;

        if((ulPtr + ulLen) <= g_stDumpInfo.ulDFLen)
        {
            (VOS_VOID)VOS_MemCpy_s(&g_stDumpInfo.pcDFAddr[ulPtr], g_stDumpInfo.ulDFLen-ulPtr, (VOS_VOID*)pFrame, ulLen);

            /* ����Ϊ0����Ҫȡ�� */
            g_stDumpInfo.ulDFCur = (g_stDumpInfo.ulDFCur + ulLen) % g_stDumpInfo.ulDFLen;
        }
        else
        {
            (VOS_VOID)VOS_MemCpy_s(&g_stDumpInfo.pcDFAddr[ulPtr], g_stDumpInfo.ulDFLen-ulPtr, (VOS_VOID*)pFrame, (g_stDumpInfo.ulDFLen - ulPtr));

            ulLen = ulLen - (g_stDumpInfo.ulDFLen - ulPtr);     /* δ�������������� */

            (VOS_VOID)VOS_MemCpy_s(&g_stDumpInfo.pcDFAddr[0], ulPtr, (((VOS_UINT8*)pFrame)+(g_stDumpInfo.ulDFLen-ulPtr)), ulLen);

            /* ulLenǰ���Ѿ��������ƣ�����ؾ� */
            g_stDumpInfo.ulDFCur = ulLen;
        }
    }
}


VOS_VOID diag_ApAgentMsgProc(MsgBlock* pMsgBlock)
{
    DIAG_DATA_MSG_STRU* pMsgTmp = VOS_NULL;

    COVERITY_TAINTED_SET((VOS_VOID *)(pMsgBlock->aucValue));

    pMsgTmp = (DIAG_DATA_MSG_STRU*)pMsgBlock;

    switch(pMsgTmp->ulMsgId)
    {
        case ID_MSG_DIAG_HSO_DISCONN_IND:
            (VOS_VOID)diag_SetChanDisconn(pMsgBlock);
            break;

        case ID_MSG_DIAG_CMD_CONNECT_REQ:
            (VOS_VOID)diag_ConnMgrProc(pMsgTmp->pContext);
            break;
        case ID_MSG_DIAG_CMD_DISCONNECT_REQ:
            (VOS_VOID)diag_DisConnMgrProc(pMsgTmp->pContext);
            break;

        default:
            break;
    }
}

VOS_VOID diag_TimerMsgProc(MsgBlock* pMsgBlock)
{
    REL_TIMER_MSG *pTimer = NULL;

    pTimer   = (REL_TIMER_MSG*)pMsgBlock;

    if((DIAG_DEBUG_TIMER_NAME == pTimer->ulName) && (DIAG_DEBUG_TIMER_PARA == pTimer->ulPara))
    {
        diag_ReportMntn();
    }
    else if((DIAG_CONNECT_TIMER_NAME == pTimer->ulName) && (DIAG_CONNECT_TIMER_PARA == pTimer->ulPara))
    {
        (VOS_VOID)diag_ConnTimerProc();
    }
    else if((DIAG_HIGH_TS_PUSH_TIMER_NAME == pTimer->ulName) && (DIAG_HIGH_TS_PUSH_TIMER_PARA == pTimer->ulPara))
    {
        /* ����ʱ�����λ */
        diag_PushHighTs();
    }
    else if((DIAG_FLOW_ACORE_TIMER_NAME == pTimer->ulName) && (DIAG_ACORE_FLOW_TIMER_PARA == pTimer->ulPara))
    {
        /* ����ʱ�����λ */
        DIAG_ACoreFlowPress();
    }
    else if((DIAG_TRANS_CNF_TIMER_NAME == pTimer->ulName) && (DIAG_TRANS_CNF_TIMER_PARA == pTimer->ulPara))
    {
        diag_TransTimeoutProc(pTimer);
    }
    else
    {
        ;
    }

    return;
}
/*****************************************************************************
 Function Name   : diag_AppAgentMsgProc
 Description        : DIAG APP AGENT���յ�����Ϣ�������
 Input                : MsgBlock* pMsgBlock

 ע������:
    ����errorlog����Ϣ����ʶ����PID��������Ҫ����errorlog�Ĵ������м��
    ����֪��Ϣ���ɹ�����ʱ������Ҫ�ٽ���errorlog����Ϣ���
    ͨ��ulErrorLog��ֵ�ж��Ƿ����errorlog����Ϣ���
    ����������չʱ��Ҫע��
*****************************************************************************/
VOS_VOID diag_AppAgentMsgProc(MsgBlock* pMsgBlock)
{
    VOS_UINT32  ulErrorLog = ERR_MSP_CONTINUE; /* ������ͷ�е�ע����������� */

    /*����ж�*/
    if (NULL == pMsgBlock)
    {
        return;
    }

    /*����ʼ����������˯��*/
    __pm_stay_awake(&diag_wakelock);

    diag_DumpMsgInfo(pMsgBlock->ulSenderPid, (*(VOS_UINT32*)pMsgBlock->aucValue), pMsgBlock->ulLength);

    /*���ݷ���PID��ִ�в�ͬ����*/
    switch(pMsgBlock->ulSenderPid)
    {
        /*��ʱ��Ϣ�����ճ�ʱ����ʽ������ظ�*/
        case DOPRA_PID_TIMER:
            diag_TimerMsgProc(pMsgBlock);
            ulErrorLog = VOS_OK;
            break;

        case WUEPS_PID_REG:
        case CCPU_PID_PAM_OM:

            ulErrorLog = diag_TransCnfProc((VOS_UINT8*)pMsgBlock, (pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH),
                                            DIAG_MSG_TYPE_BBP, &g_stPSTransHead);
            break;

        case MSP_PID_DIAG_APP_AGENT:
            diag_ApAgentMsgProc(pMsgBlock);
            ulErrorLog = VOS_OK;
            break;
        /*lint -save -e826*/
        case MSP_PID_DIAG_AGENT:
            diag_LrmAgentMsgProc(pMsgBlock);
            break;
        case MSP_PID_DIAG_NRM_AGENT:
            diag_NrmAgentMsgProc(pMsgBlock);
            break;
        /*lint -restore +e826*/
        case DSP_PID_APM:
            {
                ulErrorLog = ERR_MSP_CONTINUE;
            }
            break;

        default:
            /* ��ͨ��PID�ж��Ƿ�͸���Ļظ���ͨ��ƥ������ʱ����Ľڵ�ָ����ȷ���Ƿ�͸���Ļظ� */
            ulErrorLog = diag_TransCnfProc((VOS_UINT8*)pMsgBlock, (pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH),
                                            DIAG_MSG_TYPE_PS, &g_stPSTransHead);
            break;

    }

   /*����ʼ����������˯��*/
   __pm_relax(&diag_wakelock);

   return ;
}

/*****************************************************************************
 Function Name   : diag_AddTransInfoToList
 Description     : ��ӽ�����͸���������ݵ�������
*****************************************************************************/
DIAG_TRANS_NODE_STRU* diag_AddTransInfoToList(VOS_UINT8 * pstReq, VOS_UINT32 ulRcvlen, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_NODE_STRU* pNewNode = NULL;
    VOS_UINT32 ulNodeSize = 0;

    ulNodeSize = sizeof(DIAG_TRANS_NODE_STRU) + ulRcvlen;
    if(ulNodeSize > DIAG_MEM_ALLOC_LEN_MAX)
    {
       diag_error("ERROR: ulNodeSize=0x%x,\n", ulNodeSize);
       return NULL;
    }

    /*����һ���ڵ��С*/
    pNewNode = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, ulNodeSize);
    if (NULL == pNewNode)
    {
        return NULL;
    }

    (VOS_VOID)VOS_MemSet_s(pNewNode, ulNodeSize, 0, ulNodeSize);
    /*������������浽�ڵ���*/
    (VOS_VOID)VOS_MemCpy_s(pNewNode->ucRcvData, ulRcvlen, pstReq, ulRcvlen);

    /* ����ź������� */
    (VOS_VOID)VOS_SmP(pstHead->TransSem, 0);

    /* ����ڵ㵽����β�� */
    blist_add_tail(&pNewNode->DiagList, &pstHead->TransHead);

    pNewNode->ulMagicNum = DIAG_TRANS_MAGIC_NUM;
    pNewNode->StartTime = mdrv_timer_get_normal_timestamp();

    (VOS_VOID)VOS_SmV(pstHead->TransSem);

    return pNewNode;
}



VOS_UINT32 diag_TransReqProcEntry(DIAG_FRAME_INFO_STRU *pstReq, DIAG_TRANS_HEADER_STRU *pstHead)
{
    VOS_UINT32              ret = ERR_MSP_FAILURE;
    VOS_UINT32              ulSize;
    VOS_UINT32              ulCmdParasize;
    DIAG_TRANS_MSG_STRU     *pstSendReq = NULL;
    DIAG_TRANS_NODE_STRU    *pNode = VOS_NULL;
#ifdef CONFIG_ARM64
    VOS_UINT_PTR            ullAddr;
#endif
    DIAG_OSA_MSG_STRU       *pstMsg = NULL;

    mdrv_diag_PTR(EN_DIAG_PTR_MSGMSP_TRANS, 1, pstReq->ulCmdId, 0);

    if(pstReq->ulMsgLen < sizeof(MSP_DIAG_DATA_REQ_STRU) + sizeof(DIAG_TRANS_MSG_STRU) - sizeof(pstSendReq->aucPara))
    {
        diag_error("rev datalen error, 0x%x\n", pstReq->ulMsgLen);
        diag_FailedCmdCnf(pstReq, ERR_MSP_INALID_LEN_ERROR);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    ulCmdParasize = pstReq->ulMsgLen - sizeof(MSP_DIAG_DATA_REQ_STRU);

     /* ���͸������*/
    pstSendReq = (DIAG_TRANS_MSG_STRU*)(pstReq->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));

    diag_LNR(EN_DIAG_LNR_PS_TRANS, pstReq->ulCmdId, VOS_GetSlice());

    if(VOS_PID_AVAILABLE != VOS_CheckPidValidity(pstSendReq->ulReceiverPid))
    {
        diag_error("rev pid:0x%x invalid\n", pstSendReq->ulReceiverPid);
        diag_FailedCmdCnf(pstReq, ERR_MSP_DIAG_ERRPID_CMD);
        return ERR_MSP_FAILURE;
    }

    ulSize = sizeof(DIAG_FRAME_INFO_STRU) + pstReq->ulMsgLen;

    pNode = diag_AddTransInfoToList((VOS_UINT8*)pstReq, ulSize, pstHead);
    if(VOS_NULL == pNode)
    {
        diag_error("diag_AddTransCmdToList failed\n");
        return ERR_MSP_FAILURE;
    }

    /* д��32λ */
    pstSendReq->ulSN = (uintptr_t)pNode;

    /* �����64λCPU����Ҫ�Ѹ�32λҲ����ȥ */
#ifdef CONFIG_ARM64
    {
        ullAddr = (VOS_UINT_PTR)pNode;
        pstSendReq->usOriginalId    = (VOS_UINT16)((ullAddr>>32)&0x0000FFFF);
        pstSendReq->usTerminalId    = (VOS_UINT16)((ullAddr>>48)&0x0000FFFF);
    }
#endif

    if(DIAG_DEBUG_TRANS & g_ulDebugCfg)
    {
        diag_crit("trans req: cmdid 0x%x, pid %d, msgid 0x%x.\n",
           pstReq->ulCmdId, pstSendReq->ulReceiverPid, pstSendReq->ulMsgId);
    }

    if(ulCmdParasize > DIAG_MEM_ALLOC_LEN_MAX)
    {
        diag_error("ERROR: ulMsgId=0x%x, ulCmdParasize = 0x%x.\n",pstReq->ulCmdId,ulCmdParasize);
        return ERR_MSP_FAILURE;

    }
    pstMsg = (DIAG_OSA_MSG_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, (ulCmdParasize - VOS_MSG_HEAD_LENGTH));
    if (pstMsg != NULL)
    {
        pstMsg->ulReceiverPid   = pstSendReq->ulReceiverPid;

        (VOS_VOID)VOS_MemCpy_s(&pstMsg->ulMsgId, ulCmdParasize-VOS_MSG_HEAD_LENGTH, &pstSendReq->ulMsgId, (ulCmdParasize - VOS_MSG_HEAD_LENGTH));

        ret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstMsg);
        if (ret != VOS_OK)
        {
			diag_error("VOS_SendMsg failed!\n");
        }
        else
        {
            ret = ERR_MSP_SUCCESS;
        }
    }

    return ret;
}


/*****************************************************************************
 Function Name   : diag_DelTransCmdNode
 Description     : ɾ���Ѿ�������Ľڵ�
*****************************************************************************/
VOS_VOID diag_DelTransCmdNode(DIAG_TRANS_NODE_STRU* pTempNode)
{

    blist_del(&pTempNode->DiagList);

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pTempNode);

    return ;
}


/*****************************************************************************
 Function Name   : diag_TransTimeoutProc
 Description     : ͸������ĳ�ʱ����
*****************************************************************************/
VOS_VOID diag_TransTimeoutProc(REL_TIMER_MSG *pTimer)
{
    DIAG_TRANS_NODE_STRU *pNode = VOS_NULL;
    DIAG_FRAME_INFO_STRU *pFrame = VOS_NULL;
    VOS_UINT32 ulTimeStamp = 0;
    VOS_UINT32 ulDetaTime;
    LIST_S     *me = NULL;
    LIST_S     *n  = NULL;


    ulTimeStamp = mdrv_timer_get_normal_timestamp();

    (VOS_VOID)VOS_SmP(g_stPSTransHead.TransSem, 0);

    /* �������в���ÿ����������*/
    blist_for_each_safe(me, n, &g_stPSTransHead.TransHead)
    {
        pNode = blist_entry(me, DIAG_TRANS_NODE_STRU, DiagList);

        ulDetaTime = DIAG_TRANS_CNF_TIMEOUT(ulTimeStamp,pNode->StartTime);
        if(ulDetaTime > DIAG_TRANS_TIMEOUT_LEN)
        {
            pFrame = (DIAG_FRAME_INFO_STRU *)pNode->ucRcvData;
            diag_info("trans timeout : cmdid 0x%x.\n", pFrame->ulCmdId);

            diag_DelTransCmdNode(pNode);
            continue;
        }

        break;
    }

    (VOS_VOID)VOS_SmV(g_stPSTransHead.TransSem);
    return;
}


DIAG_TRANS_NODE_STRU * diag_IsTransCnf(DIAG_TRANS_MSG_STRU* pstPsCnf, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_NODE_STRU    *pNode = VOS_NULL;
    DIAG_TRANS_NODE_STRU    *pTempNode = VOS_NULL;
    LIST_S                  *me = NULL;
#ifdef CONFIG_ARM64
    VOS_UINT_PTR ullAddr;
#endif

    /* ����64λ */
#ifndef CONFIG_ARM64
    pNode = (DIAG_TRANS_NODE_STRU *)pstPsCnf->ulSN;
#else
    ullAddr = (VOS_UINT_PTR)pstPsCnf->usTerminalId;
    ullAddr = (ullAddr<<16) | pstPsCnf->usOriginalId;
    ullAddr = (ullAddr<<32) | pstPsCnf->ulSN;

    pNode = (DIAG_TRANS_NODE_STRU *)ullAddr;
#endif

    /*����ź�������*/
    (VOS_VOID)VOS_SmP(pstHead->TransSem,0);

    /* �������в���ÿ����������*/
    blist_for_each(me, &pstHead->TransHead)
    {
        pTempNode = blist_entry(me, DIAG_TRANS_NODE_STRU, DiagList);

        if(pTempNode == pNode)
        {
            (VOS_VOID)VOS_SmV(pstHead->TransSem);

            /* ulMagicNum�Ƿ���ʾ�ڵ��ѳ�ʱɾ�� */
            if(DIAG_TRANS_MAGIC_NUM != pNode->ulMagicNum)
            {
                return VOS_NULL;
            }

            return pNode;
        }
    }

    (VOS_VOID)VOS_SmV(pstHead->TransSem);

    return VOS_NULL;
}

/*****************************************************************************
 Function Name   : diag_GetTransInfo
 Description     : ��ȡ͸���������Ϣ����ɾ���ڵ�
*****************************************************************************/
VOS_VOID diag_GetTransInfo(MSP_DIAG_CNF_INFO_STRU *pstInfo,
                             DIAG_TRANS_CNF_STRU    *pstDiagCnf,
                             DIAG_TRANS_MSG_STRU    *pstPsCnf,
                             DIAG_TRANS_NODE_STRU   *pNode)
{
    DIAG_FRAME_INFO_STRU    *pFrame = VOS_NULL;
    MSP_DIAG_DATA_REQ_STRU  *pDiagData = VOS_NULL;
    APP_OM_MSG_STRU         *pstOmMsg = VOS_NULL;

    /*����ź�������*/
    (VOS_VOID)VOS_SmP(g_stPSTransHead.TransSem, 0);

    pFrame = (DIAG_FRAME_INFO_STRU *)pNode->ucRcvData;
    pstInfo->ulTransId    = pFrame->stService.ulMsgTransId;
    pstInfo->ulMsgId      = pFrame->ulCmdId;
    pstInfo->ulMode       = pFrame->stID.mode4b;
    pstInfo->ulSubType    = pFrame->stID.sec5b;

    pDiagData           = (MSP_DIAG_DATA_REQ_STRU *)pFrame->aucData;
    pstDiagCnf->ulAuid  = pDiagData->ulAuid;
    pstDiagCnf->ulSn    = pDiagData->ulSn;

    pstOmMsg = (APP_OM_MSG_STRU *)pDiagData->ucData;

    pstPsCnf->usOriginalId  = pstOmMsg->usOriginalId;
    pstPsCnf->usTerminalId  = pstOmMsg->usTerminalId;
    pstPsCnf->ulSN          = pstOmMsg->ulSN;

    diag_DelTransCmdNode(pNode);
    (VOS_VOID)VOS_SmV(g_stPSTransHead.TransSem);

    return ;
}


VOS_UINT32 diag_TransCnfProc(VOS_UINT8* pstCnf ,VOS_UINT32 ulLen, DIAG_MESSAGE_TYPE_U32 ulGroupId, DIAG_TRANS_HEADER_STRU *pstHead)
{
    VOS_UINT32              ret = 0;
    DIAG_TRANS_CNF_STRU     *pstDiagCnf = VOS_NULL;
    DIAG_TRANS_MSG_STRU     *pstPsCnf = VOS_NULL;
    MSP_DIAG_CNF_INFO_STRU  stDiagInfo = {0};
    DIAG_TRANS_NODE_STRU    *pNode = VOS_NULL;

    if(VOS_NULL_PTR == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }
    pstPsCnf = (DIAG_TRANS_MSG_STRU *)pstCnf;

    mdrv_diag_PTR(EN_DIAG_PTR_TRANS_CNF_PROC, 1, ulGroupId, 0);

    if(pstPsCnf->ulLength < (sizeof(DIAG_TRANS_MSG_STRU) - VOS_MSG_HEAD_LENGTH - sizeof(pstPsCnf->aucPara)))
    {
        /* �������С��͸������Ļظ����ȿ϶�����͸������ */
        return ERR_MSP_CONTINUE;
    }

    pNode = diag_IsTransCnf(pstPsCnf, pstHead);
    if(VOS_NULL == pNode)
    {
        return ERR_MSP_CONTINUE;
    }

    diag_LNR(EN_DIAG_LNR_PS_TRANS, ((pstPsCnf->usOriginalId<<16) | pstPsCnf->usTerminalId), pstPsCnf->ulSN);

    if(DIAG_DEBUG_TRANS & g_ulDebugCfg)
    {
        diag_error("cmdid 0x%x, pid %d, msgid 0x%x.\n",
            stDiagInfo.ulMsgId, pstPsCnf->ulSenderPid, pstPsCnf->ulMsgId);
    }

    if((ulLen + sizeof(DIAG_TRANS_CNF_STRU)) > DIAG_MEM_ALLOC_LEN_MAX)
    {
       diag_error("ERROR: ulLen=0x%x.\n",ulLen);
       return ERR_MSP_NOT_ENOUGH_MEMORY;
    }
    pstDiagCnf = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, (ulLen + sizeof(DIAG_TRANS_CNF_STRU)));
    if(VOS_NULL_PTR == pstDiagCnf)
    {
        return ERR_MSP_FAILURE;
    }

    stDiagInfo.ulSSId       = DIAG_SSID_APP_CPU;
    stDiagInfo.ulMsgType    = ulGroupId;
    stDiagInfo.ulDirection  = DIAG_MT_CNF;
    stDiagInfo.ulModemid    = 0;

    diag_GetTransInfo(&stDiagInfo, pstDiagCnf, pstPsCnf, pNode);

    diag_LNR(EN_DIAG_LNR_PS_TRANS, (stDiagInfo.ulMsgId), VOS_GetSlice());

    (VOS_VOID)VOS_MemCpy_s(pstDiagCnf->aucPara, ulLen, pstPsCnf, ulLen);

    ret = DIAG_MsgReport(&stDiagInfo, pstDiagCnf, (ulLen + sizeof(DIAG_TRANS_CNF_STRU)));


    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstDiagCnf);

    return ret;
}



VOS_VOID DIAG_LogShowToFile(VOS_BOOL bIsSendMsg)
{
}


int diag_debug_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t diag_debug_read(struct file *file, char __user *ubuf, size_t cnt, loff_t *ppos)
{
    printk("usage:\n");
    printk("\t echo cmd [param1] [param2] ... [paramN] > /sys/kernel/debug/modem_diag/diag\n");
    printk("cmd list:\n");
    printk("\t DIAG_ALL --- save all diag debug infomation.\n");
    return 0;
}

static ssize_t diag_debug_write(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
{
    char buf[128] = {0};
    ssize_t ret = cnt;
    DIAG_A_DEBUG_C_REQ_STRU *pstFlag = NULL;
    VOS_UINT32              ulret = ERR_MSP_FAILURE;

    if((!ubuf)||(!cnt))
    {
        diag_error("input error\n");
        return 0;
    }

    cnt = (cnt > 127) ? 127 : cnt;
    if(copy_from_user(buf, ubuf, cnt))
    {
        diag_error("copy from user fail\n");
        ret = -EFAULT;
        goto out;
    }
    buf[cnt] = 0;

    /* �����쳣Ŀ¼���� */
    if(0 == strncmp(buf, "DIAG_ALL", strlen("DIAG_ALL")))
    {
        DIAG_DebugCommon();
        DIAG_DebugNoIndLog();

        pstFlag = (DIAG_A_DEBUG_C_REQ_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, (sizeof(DIAG_A_DEBUG_C_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

        if (pstFlag != NULL)
        {
            pstFlag->ulReceiverPid  = MSP_PID_DIAG_AGENT;

            pstFlag->ulMsgId        = DIAG_MSG_MSP_A_DEBUG_C_REQ;
            pstFlag->ulFlag         = DIAG_DEBUG_DFR_BIT | DIAG_DEBUG_NIL_BIT;

            ulret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstFlag);
            if (ulret != VOS_OK)
            {
                diag_error("VOS_SendMsg failed!\n");
            }
        }
    }

out:
    return ret;
}

long diag_debug_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static const struct file_operations diag_debug_fops = {
    .open       = diag_debug_open,
    .read       = diag_debug_read,
    .write      = diag_debug_write,
    .unlocked_ioctl = diag_debug_ioctl,
};



VOS_UINT32 MSP_AppDiagFidInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32 ulRelVal = 0;

    switch (ip)
    {
        case VOS_IP_LOAD_CONFIG:

            mdrv_PPM_RegDisconnectCb(diag_DisconnectTLPort);

            ulRelVal = VOS_RegisterPIDInfo(MSP_PID_DIAG_APP_AGENT, (Init_Fun_Type) diag_AppAgentMsgProcInit, (Msg_Fun_Type) diag_AppAgentMsgProc);

            if (ulRelVal != VOS_OK)
            {
                return VOS_ERR;
            }

            ulRelVal = VOS_RegisterMsgTaskPrio(MSP_FID_DIAG_ACPU, VOS_PRIORITY_M2);
            if (ulRelVal != VOS_OK)
            {
                return VOS_ERR;
            }
            /* ����8K��dump�ռ� */
            g_stDumpInfo.pcDumpAddr = (VOS_VOID * )mdrv_om_register_field(OM_AP_DIAG, "ap_diag", (void*)0, (void*)0, DIAG_DUMP_LEN, 0);

            if(VOS_NULL != g_stDumpInfo.pcDumpAddr)
            {
                g_stDumpInfo.pcMsgAddr = g_stDumpInfo.pcDumpAddr;
                g_stDumpInfo.ulMsgCur  = 0;
                g_stDumpInfo.ulMsgLen  = DIAG_DUMP_MSG_LEN;

                g_stDumpInfo.pcDFAddr  = g_stDumpInfo.pcDumpAddr + DIAG_DUMP_MSG_LEN;
                g_stDumpInfo.ulDFCur   = 0;
                g_stDumpInfo.ulDFLen   = DIAG_DUMP_DF_LEN;
            }

            VOS_RegisterMsgGetHook((VOS_MSG_HOOK_FUNC)DIAG_LayerMsgReport);


            return VOS_OK;
            // break;

        case VOS_IP_RESTART:
            /* ready stat */
            mdrv_ppm_pcdev_ready();
            break;
        default:
            break;
    }

    return VOS_OK;
}


VOS_VOID DIAG_DebugTransOn(VOS_UINT32 ulOn)
{
    if(0 == ulOn)
    {
        g_ulDebugCfg &= (~DIAG_DEBUG_TRANS);
    }
    else
    {
        g_ulDebugCfg |= DIAG_DEBUG_TRANS;
    }
}


VOS_UINT32 diag_DisconnectTLPort(void)
{
    DIAG_DATA_MSG_STRU                 *pstMsg;

    pstMsg = (DIAG_DATA_MSG_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, sizeof(DIAG_DATA_MSG_STRU) - VOS_MSG_HEAD_LENGTH);

    if (NULL == pstMsg)
    {
        return VOS_ERR;
    }

    pstMsg->ulReceiverPid = MSP_PID_DIAG_APP_AGENT;
    pstMsg->ulMsgId       = ID_MSG_DIAG_HSO_DISCONN_IND;

    (void)VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstMsg);

    return VOS_OK;
}
EXPORT_SYMBOL(DIAG_DebugTransOn);




