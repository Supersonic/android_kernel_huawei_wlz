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


#include "pamappom.h"
#include "omprivate.h"
#include "NVIM_Interface.h"
#include "CbtPpm.h"
#include "AtOamInterface.h"
#include "PsLogFilterInterface.h"
#include "si_pih.h"
#include "pam_tag.h"

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#pragma pack(4)

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_PAM_APP_OM_C
#define    THIS_MODU           mod_pam_om

#define ARRAYSIZE(array)                        (sizeof(array)/sizeof(array[0]))

/*****************************************************************************
  �궨��
******************************************************************************/
#define OM_NORMAL_LOG1(string, para1)  PAM_OM_NormalLog1(__LINE__, string, para1)


/*****************************************************************************
 STRUCT����
*****************************************************************************/
typedef struct
{
    VOS_PID                             ulPid;
    VOS_PID                             ulI0Pid;
}OM_PID_MAP_STRU;

/* ��¼�յ���Ϣ��Ϣ��buffer����ǰ���� */
OM_RECORD_BUF_STRU                      g_astAcpuRecordInfo[VOS_EXC_DUMP_MEM_NUM_BUTT];

VOS_UINT32                              g_ulAcpuOmFilterFlag;

const OM_PID_MAP_STRU                   g_astUsimPidList[] =
{
    {I0_MAPS_PIH_PID,   I0_MAPS_PIH_PID},
    {I1_MAPS_PIH_PID,   I0_MAPS_PIH_PID},
    {I2_MAPS_PIH_PID,   I0_MAPS_PIH_PID},
    {I0_MAPS_STK_PID,   I0_MAPS_STK_PID},
    {I1_MAPS_STK_PID,   I0_MAPS_STK_PID},
    {I2_MAPS_STK_PID,   I0_MAPS_STK_PID},
    {I0_MAPS_PB_PID,    I0_MAPS_PB_PID},
    {I1_MAPS_PB_PID,    I0_MAPS_PB_PID},
    {I2_MAPS_PB_PID,    I0_MAPS_PB_PID},
};


VOS_VOID PAM_OM_PrintSecResult(errno_t ret, VOS_UINT32 fileId, VOS_UINT32 line)
{
    if (ret != EOK)
    {
        mdrv_err("<PAM_OM_PrintSecResult> secfun errno %d,file %d,line %d.\n", ret, fileId, line);
    }

    return;
}


VOS_VOID PAM_OM_NormalLog1(VOS_UINT32 line, const VOS_CHAR* logstr, VOS_UINT32 para1)
{
    if (DIAG_LogReport(DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, PS_LOG_LEVEL_NORMAL), ACPU_PID_PAM_OM,
                       VOS_NULL_PTR, line, "%s, %d \r\n", logstr, (VOS_INT32)(para1)) != ERR_MSP_SUCCESS)
    {
        return;
    }
    return;
}


VOS_VOID OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_ENUM_UINT32 enNumber)
{
    VOS_UINT32                         *pulBuf = VOS_NULL_PTR;

    if(enNumber >= VOS_EXC_DUMP_MEM_NUM_BUTT)
    {
        return;
    }

    if(g_astAcpuRecordInfo[enNumber].pucBuf == VOS_NULL_PTR)
    {
        return;
    }

    if(g_astAcpuRecordInfo[enNumber].ulLen == 0)
    {
        return;
    }

    /* ��start���Ѿ�����˼�¼endslice�ĳ��ȣ���˴˴������ĸ��ֽ���дendslice��ֵ */
    pulBuf = (VOS_UINT32*)(g_astAcpuRecordInfo[enNumber].pucBuf + g_astAcpuRecordInfo[enNumber].ulLen - sizeof(VOS_UINT32));

    *pulBuf = VOS_GetSlice();

    return;
}


VOS_VOID OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_ENUM_UINT32 enNumber, VOS_UINT32 ulSendPid, VOS_UINT32 ulRcvPid, VOS_UINT32 ulMsgName)
{
    OM_RECORD_INFO_STRU     stRecordInfo;

    if(enNumber >= VOS_EXC_DUMP_MEM_NUM_BUTT)
    {
       return;
    }

    if(g_astAcpuRecordInfo[enNumber].pucBuf == VOS_NULL_PTR)
    {
       return;
    }

    g_astAcpuRecordInfo[enNumber].ulLen %= VOS_TASK_DUMP_INFO_SIZE;

    stRecordInfo.usSendPid      = (VOS_UINT16)ulSendPid;
    stRecordInfo.usRcvPid       = (VOS_UINT16)ulRcvPid;
    stRecordInfo.ulMsgName      = ulMsgName;
    stRecordInfo.ulSliceStart   = VOS_GetSlice();
    stRecordInfo.ulSliceEnd     = 0;

    if (memcpy_s((g_astAcpuRecordInfo[enNumber].pucBuf + g_astAcpuRecordInfo[enNumber].ulLen),
                  (VOS_TASK_DUMP_INFO_SIZE - g_astAcpuRecordInfo[enNumber].ulLen),
                  &stRecordInfo,
                  sizeof(OM_RECORD_INFO_STRU)) != EOK)
    {
        mdrv_err("<OM_RecordInfoStart> memcpy_s Fail!\n");
    }

    g_astAcpuRecordInfo[enNumber].ulLen += (VOS_UINT32)sizeof(OM_RECORD_INFO_STRU);

    return;
}


VOS_VOID OM_RecordMemInit(VOS_VOID)
{
   VOS_UINT32 i;

   (VOS_VOID)memset_s(g_astAcpuRecordInfo,
                      sizeof(g_astAcpuRecordInfo),
                      0,
                      sizeof(g_astAcpuRecordInfo));

   /* ����ÿ��ģ���¼��ν�ɲ���Ϣ�Ŀռ� */
   for(i = 0; i < VOS_EXC_DUMP_MEM_NUM_BUTT; i++)
   {
      g_astAcpuRecordInfo[i].pucBuf = (VOS_UINT8*)VOS_ExcDumpMemAlloc(i);

      if(g_astAcpuRecordInfo[i].pucBuf == VOS_NULL_PTR)
      {
          return;
      }
   }

   return;
}

VOS_VOID PAMOM_AcpuTimerMsgProc(MsgBlock* pMsg)
{
    return;
}

VOS_VOID PAMOM_QuereyPidInfo(VOS_VOID)
{
    PAM_VOS_QUEREY_PID_INFO_REQ_STRU    *pstMsg = VOS_NULL_PTR;

    pstMsg = (PAM_VOS_QUEREY_PID_INFO_REQ_STRU *)VOS_AllocMsg(ACPU_PID_PAM_OM,
                            sizeof(PAM_VOS_QUEREY_PID_INFO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    /* ������Ϣʧ�� */
    if (pstMsg == VOS_NULL_PTR)
    {
        return;
    }

    pstMsg->ulReceiverPid  = CCPU_PID_PAM_OM;
    pstMsg->usPrimId       = PAM_VOS_QUEREY_PID_INFO_REQ;

    if (VOS_SendMsg(ACPU_PID_PAM_OM, pstMsg) != VOS_OK)
    {
        mdrv_err("<PAMOM_QuereyPidInfo> VOS_SendMsg Fail!\n");
    }

    return;
}


VOS_VOID PAMOM_QuereyPidInfoMsgProc(MsgBlock* pMsg)
{
    PAM_VOS_QUEREY_PID_INFO_REQ_STRU    *pstMsg = VOS_NULL_PTR;
    PAM_VOS_QUEREY_PID_INFO_CNF_STRU    *pstCnfMsg = VOS_NULL_PTR;
    VOS_UINT32                           ulLen;

    pstMsg = (PAM_VOS_QUEREY_PID_INFO_REQ_STRU *)pMsg;

    if (pstMsg->usPrimId == PAM_VOS_QUEREY_PID_INFO_REQ)
    {
        ulLen = VOS_QueryPidInfoBufSize();

        pstCnfMsg = (PAM_VOS_QUEREY_PID_INFO_CNF_STRU *)VOS_AllocMsg(ACPU_PID_PAM_OM,
                            sizeof(PAM_VOS_QUEREY_PID_INFO_CNF_STRU) - VOS_MSG_HEAD_LENGTH + ulLen);

        /* ������Ϣʧ�� */
        if (pstCnfMsg == VOS_NULL_PTR)
        {
            return;
        }

        pstCnfMsg->ulReceiverPid  = CCPU_PID_PAM_OM;
        pstCnfMsg->usPrimId       = PAM_VOS_QUEREY_PID_INFO_CNF;
        pstCnfMsg->usLen          = (VOS_UINT16)ulLen;
        VOS_QueryPidInfo((VOS_VOID *)pstCnfMsg->aucValue);

        if (VOS_SendMsg(ACPU_PID_PAM_OM, pstCnfMsg) != VOS_OK)
        {
            mdrv_err("<PAMOM_QuereyPidInfoMsgProc> VOS_SendMsg Fail!\n");
        }
    }
    else
    {
        pstCnfMsg = (PAM_VOS_QUEREY_PID_INFO_CNF_STRU *)pMsg;

        if (pstCnfMsg->ulLength < ((sizeof(PAM_VOS_QUEREY_PID_INFO_CNF_STRU) - VOS_MSG_HEAD_LENGTH - sizeof(pstCnfMsg->aucValue)) + pstCnfMsg->usLen))
        {
            mdrv_err("<PAMOM_QuereyPidInfoMsgProc> the cnf msg len err: %d.\n", pstCnfMsg->ulLength);
            return;
        }

        VOS_SetPidInfo((VOS_VOID *)(pstCnfMsg->aucValue), pstCnfMsg->usLen);
    }

    return;
}


VOS_PID PAM_OM_GetI0UsimPid(
    VOS_PID                             ulPid
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulPidListNum;

    ulPidListNum = ARRAYSIZE(g_astUsimPidList);

    for (i = 0; i < ulPidListNum; i++)
    {
        if (g_astUsimPidList[i].ulPid == ulPid)
        {
            return g_astUsimPidList[i].ulI0Pid;
        }
    }

    return ulPid;
}


VOS_UINT32 PAM_OM_AcpuPihToAtMsgFilter(
    const MsgBlock                     *pMsg
)
{
    MN_APP_PIH_AT_CNF_STRU             *pstEventCnf = VOS_NULL_PTR;

    pstEventCnf = (MN_APP_PIH_AT_CNF_STRU *)pMsg;

    if (pstEventCnf->ulMsgId == PIH_AT_EVENT_CNF)
    {
        OM_NORMAL_LOG1("PAM_OM_AcpuAtToPihMsgFilter: The filter EventType is :", pstEventCnf->stPIHAtEvent.EventType);

        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 PAM_OM_AcpuPihToPCSCMsgFilter(
    const MsgBlock                      *pMsg
)
{
    MSG_HEADER_STRU                     *pstMsg = VOS_NULL_PTR;

    pstMsg = (MSG_HEADER_STRU *)pMsg;

    if (pstMsg->ulMsgName == USIMM_CARDSTATUS_IND)
    {
        OM_NORMAL_LOG1("PAM_OM_AcpuPihToPCSCMsgFilter: The filter name is :", pstMsg->ulMsgName);

        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 PAM_OM_AcpuPihMsgFilter(
    const MsgBlock                     *pMsg
)
{
    MSG_HEADER_STRU                    *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstMsg = (MSG_HEADER_STRU *)pMsg;

    switch (pstMsg->ulReceiverPid)
    {
        case WUEPS_PID_AT:
            ulRet = PAM_OM_AcpuPihToAtMsgFilter(pMsg);
            break;

        case ACPU_PID_PCSC:
            ulRet = PAM_OM_AcpuPihToPCSCMsgFilter(pMsg);
            break;

        default :
            ulRet = VOS_FALSE;
            break;
    }

    return ulRet;
}


VOS_UINT32 PAM_OM_AcpuStkToAtMsgFilter(
    const MsgBlock                     *pMsg
)
{
    MN_APP_STK_AT_CNF_STRU             *pstEventCnf = VOS_NULL_PTR;

    pstEventCnf = (MN_APP_STK_AT_CNF_STRU *)pMsg;

    switch (pstEventCnf->ulMsgId)
    {
        case STK_AT_EVENT_CNF:
        case STK_AT_DATAPRINT_CNF:
            OM_NORMAL_LOG1("PAM_OM_AcpuStkToAtMsgFilter: The filter ulMsgId is :", pstEventCnf->ulMsgId);

            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}


VOS_UINT32 PAM_OM_AcpuEmatToAtMsgFilter(
    const MsgBlock                     *pMsg
)
{
    MN_APP_EMAT_AT_CNF_STRU            *pstEventCnf = VOS_NULL_PTR;

    pstEventCnf = (MN_APP_EMAT_AT_CNF_STRU *)pMsg;

    switch (pstEventCnf->ulMsgId)
    {
        case EMAT_AT_EVENT_CNF:
            OM_NORMAL_LOG1("PAM_OM_AcpuEmatToAtMsgFilter: The filter ulMsgId is :", pstEventCnf->ulMsgId);

            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}


VOS_UINT32 PAM_OM_AcpuAtToPihMsgFilter(
    const MsgBlock                      *pMsg
)
{
    MSG_HEADER_STRU                     *pstMsg = VOS_NULL_PTR;

    pstMsg = (MSG_HEADER_STRU *)pMsg;

    switch (pstMsg->ulMsgName)
    {
        case SI_PIH_CRSM_SET_REQ:
        case SI_PIH_CRLA_SET_REQ:
        case SI_PIH_CGLA_SET_REQ:
#if (FEATURE_PHONE_SC == FEATURE_ON)
        case SI_PIH_SILENT_PININFO_SET_REQ:
#endif
        case SI_PIH_GACCESS_REQ:
        case SI_PIH_ISDB_ACCESS_REQ:
        case SI_PIH_PRIVATECGLA_SET_REQ:
        case SI_PIH_URSM_REQ:
        case SI_PIH_UICCAUTH_REQ:
        case SI_PIH_FDN_ENABLE_REQ:
        case SI_PIH_FDN_DISALBE_REQ:
            OM_NORMAL_LOG1("PAM_OM_AcpuAtToPihMsgFilter: The filter ulMsgName is :", pstMsg->ulMsgName);

            return VOS_TRUE;

        default:
            break;
    }

    return VOS_FALSE;
}


VOS_UINT32 PAM_OM_AcpuAtToStkMsgFilter(
    const MsgBlock                      *pMsg
)
{
    MSG_HEADER_STRU                     *pstMsg = VOS_NULL_PTR;

    pstMsg = (MSG_HEADER_STRU *)pMsg;

    OM_NORMAL_LOG1("PAM_OM_CcpuAtToStkMsgFilter: The Filter At To Stk Msg and Type Are: ", pstMsg->ulMsgName);

    return VOS_TRUE;
}


VOS_UINT32 PAM_OM_AcpuLayerMsgFilterOptProc(
    const MsgBlock                     *pMsg
)
{
    OM_FILTER_MSG_HEAD_STRU            *pstMsgHead = VOS_NULL_PTR;
    VOS_PID                             ulSendI0Pid;
    VOS_PID                             ulRecvI0Pid;

    if (g_ulAcpuOmFilterFlag == VOS_FALSE)
    {
        return VOS_FALSE;
    }

    pstMsgHead = (OM_FILTER_MSG_HEAD_STRU*)pMsg;

    ulSendI0Pid  = PAM_OM_GetI0UsimPid(pstMsgHead->ulSenderPid);
    ulRecvI0Pid  = PAM_OM_GetI0UsimPid(pstMsgHead->ulReceiverPid);

    /* PB��ص���Ϣȫ������ */
    if ((ulSendI0Pid == I0_MAPS_PB_PID)
     || (ulRecvI0Pid == I0_MAPS_PB_PID)
     || (pstMsgHead->ulReceiverPid == ACPU_PID_PB))
    {
        return VOS_TRUE;
    }

     /* PIH ��Ϣ���� */
    if (ulSendI0Pid == I0_MAPS_PIH_PID)
    {
        return PAM_OM_AcpuPihMsgFilter(pMsg);
    }

    if ((ulSendI0Pid == WUEPS_PID_AT)
     && (ulRecvI0Pid == I0_MAPS_PIH_PID))
    {
        return PAM_OM_AcpuAtToPihMsgFilter(pMsg);
    }

    if ((ulSendI0Pid == WUEPS_PID_AT)
     && (ulRecvI0Pid == I0_MAPS_STK_PID))
    {
        return PAM_OM_AcpuAtToStkMsgFilter(pMsg);
    }

    if (ulSendI0Pid == I0_MAPS_STK_PID)
    {
        return PAM_OM_AcpuStkToAtMsgFilter(pMsg);
    }

    if (ulSendI0Pid == I0_MAPS_EMAT_PID)
    {
        return PAM_OM_AcpuEmatToAtMsgFilter(pMsg);
    }

    return VOS_FALSE;
}


VOS_VOID * PAM_OM_LayerMsgFilter(
    MsgBlock                     *pMsg
)
{
    if (pMsg == VOS_NULL_PTR)
    {
        return VOS_NULL_PTR;
    }

    if (PAM_OM_AcpuLayerMsgFilterOptProc(pMsg) == VOS_TRUE)
    {
        return VOS_NULL_PTR;
    }

    return pMsg;

}


VOS_VOID PAM_OM_LayerMsgReplaceCBReg(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_PIH_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_PB_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_STK_PID, PAM_OM_LayerMsgFilter);

#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_PIH_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_PB_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_STK_PID, PAM_OM_LayerMsgFilter);
#if (MULTI_MODEM_NUMBER == 3)
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_PIH_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_PB_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_STK_PID, PAM_OM_LayerMsgFilter);
#endif
#endif

}



 VOS_VOID PAMOM_AcpuCcpuPamMsgProc(MsgBlock* pMsg)
 {
    VOS_UINT16                          usPrimId;

    if (pMsg->ulLength < sizeof(usPrimId))
    {
        mdrv_err("<PAMOM_AcpuCcpuPamMsgProc> the msg len err: %d.\n", pMsg->ulLength);
        return;
    }

    usPrimId = *(VOS_UINT16 *)(pMsg->aucValue);

    if (usPrimId == PAM_VOS_QUEREY_PID_INFO_REQ)
    {
        PAMOM_QuereyPidInfoMsgProc(pMsg);
    }
    else if (usPrimId == PAM_VOS_QUEREY_PID_INFO_CNF)
    {
        PAMOM_QuereyPidInfoMsgProc(pMsg);
    }
    else
    {

    }

    return;
}


VOS_VOID PAMOM_AppMsgProc(MsgBlock* pMsg)
{
    if (pMsg == VOS_NULL_PTR)
    {
        return;
    }

    if (pMsg->ulSenderPid == VOS_PID_TIMER)
    {
        PAMOM_AcpuTimerMsgProc(pMsg);
    }
    else if (pMsg->ulSenderPid == CCPU_PID_PAM_OM)
    {
        PAMOM_AcpuCcpuPamMsgProc(pMsg);
    }
    else
    {
        /* blank */
    }

    return;
}


VOS_UINT32 PAMOM_AcpuInit(VOS_VOID)
{

    PAM_OM_LayerMsgReplaceCBReg();

    PAMOM_QuereyPidInfo();

    g_ulAcpuOmFilterFlag = VOS_TRUE;

    return VOS_OK;
}


VOS_UINT32 PAMOM_AppPidInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    switch( ip )
    {
        case VOS_IP_LOAD_CONFIG:
            return PAMOM_AcpuInit();

        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32 PAMOM_APP_FID_Init(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32                          ulRslt;

    switch( ip )
    {
        case VOS_IP_LOAD_CONFIG:
        {
            ulRslt = VOS_RegisterPIDInfo(ACPU_PID_PAM_OM,
                                        (Init_Fun_Type)PAMOM_AppPidInit,
                                        (Msg_Fun_Type)PAMOM_AppMsgProc);
            if( ulRslt != VOS_OK )
            {
                return VOS_ERR;
            }

            ulRslt = VOS_RegisterMsgTaskPrio(ACPU_FID_PAM_OM, VOS_PRIORITY_M2);
            if( ulRslt != VOS_OK )
            {
                return VOS_ERR;
            }

            /* ��Ŀ¼�������򴴽� */
            if (mdrv_file_access(PAM_LOG_PATH, PAM_FILE_EXIST) != VOS_OK)
            {
                if (mdrv_file_mkdir(PAM_LOG_PATH) != VOS_OK)
                {
                    mdrv_err("<PAMOM_APP_FID_Init> mdrv_file_mkdir Fail!\n");
                }
            }

            break;
        }

        default:
            break;
    }
    return VOS_OK;
}

#if (VOS_OS_VER != VOS_WIN32)

VOS_VOID OM_OSAEvent(VOS_TIMER_OM_EVENT_STRU *pstData, VOS_UINT32 ulLength)
{
    DIAG_EVENT_IND_STRU                 stEventInd;
    VOS_UINT32                          diagResult;

    stEventInd.ulModule = DIAG_GEN_MODULE(DIAG_MODEM_0, DIAG_MODE_COMM);
    stEventInd.ulPid    = ACPU_PID_PAM_OM;
    stEventInd.ulEventId= OAM_EVENT_TIMER;
    stEventInd.ulLength = ulLength;
    stEventInd.pData    = pstData;

    diagResult = DIAG_EventReport(&stEventInd);

    if (diagResult != ERR_MSP_SUCCESS)
    {
        return;
    }

    return;
}
#endif

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif


