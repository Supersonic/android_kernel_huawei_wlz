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

#include <vos.h>
#include  "omerrorlog.h"
#include  "acore_nv_stru_msp.h"
/*****************************************************************************
   全局变量定义
*****************************************************************************/

#define    ERRLOG_IDLE         0
#define    ERRLOG_BUSY         1


#define                                 OM_MSG_RECEIVE_FLAG               (1) 
#define                                 OM_MSG_NO_RECEIVE_FLAG            (0)


/* Error Log 上报定时器 */
#define OM_ERRLOG_TIMER_LENTH               (5000)
#define OM_ERRORLOG_TIMER_NAME              (0x00001001)
#define OM_ERRORLOG_TIMER_PARA              (0x0000EFFE)

/* Clt INFO timer */
#define OM_CLTINFO_TIMER_LENTH               (5000)
#define OM_CLTINFO_TIMER_NAME         (0x00002002)
#define OM_CLTINFO_TIMER_PARA         (0x0000DFFD)

/* Send Log Tool MSG Type*/
#define OM_ERRLOG_SEND_MSG                  (0x0000DDDD)
#define OM_ERRLOG_RCV_MSG                   (0x0000EEEE)
#define OM_APP_ERRLOG_HOOK_IND              (0xBBFF)

#define OM_CLTINFO_CNF_NEED_PROCESS          0
#define OM_CLTINFO_CNF_NOT_NEED_PROCESS      1
#define OM_CLTINFO_INVALID_PID               0x5a5abeef

#define     OM_PID_NULL                     (0)          /* OM PID为空 */
#define     OM_AP_NO_MSG_SEND               (0)          /* OM上报没有给AP上报的消息 */
#define     OM_AP_SEND_MSG_FINISH           (0)          /* OM上报给AP消息完成 */



/* 用于记录Error Log收到和发送给Ap侧消息 */
typedef struct
{
    VOS_UINT32                          ulErrLogSendNum;
    VOS_UINT32                          ulErrLogSendLen;
}OM_ERR_LOG_DEBUG_INFO;

/* OM收到AP需要在全局变量中记录内容 */
typedef struct
{
    VOS_UINT16                          usFaultId;          /* 本次请求的faultid */
    VOS_UINT16                          usModemId;          /* 本次请求的modemid */
    VOS_UINT32                          ulFaultNv;          /* 本次请求faultid对应的nvid */
    VOS_UINT32                          ulAlarmIdNum;       /* 本次请求的alarm配置总数 */
    VOS_UINT32                          ulErrLogReportSend; /* 记录需要上报alarm个数，差别在于可能某些pid在当前版本不存在 */
    VOS_UINT32                          ulErrLogState;      /* 请求处理状态 */
    VOS_UINT32                          ulMsgSN;
    VOS_UINT32                          ulErrSendFlag[4];   /* 请求发送状态 */
    VOS_UINT32                          ulErrRptFlag[4];    /* 请求回复状态 */
    NV_ID_CHR_FAULTID_CONFIG_STRU       stFaultCfg;         /* 请求的NV配置 */
    
}OM_APP_MSG_RECORD_STRU;

/* 用于记录Error Log收到和发送给Ap侧消息 */
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32      ulMsgName;
    VOS_UINT32      ulMsgLen;
    VOS_UINT8       aucData[0];
    
}CHR_APP_REQ_STRU;
/*打印码流使用*/

#define OM_ACPU_ERRLOG_SEND               (0x01 << (6))
#define OM_ACPU_ERRLOG_RCV                (0x01 << (7))



#define OM_ACPU_CHR_DEBUG_TRACE(pucData, ulDataLen, ulSwitch) \
        if(VOS_FALSE != (g_ulChrOmAcpuDbgFlag&ulSwitch)) \
        { \
            VOS_UINT32 ulChrOmDbgIndex; \
            (VOS_VOID)chr_print("Data Len = %d\n", ulDataLen); \
            for (ulChrOmDbgIndex = 0 ; ulChrOmDbgIndex < ulDataLen; ulChrOmDbgIndex++) \
            { \
                (VOS_VOID)mdrv_print(MDRV_P_ERR, "%02x ", *((VOS_UINT8*)pucData + ulChrOmDbgIndex)); \
            } \
            (VOS_VOID)chr_print("\r\n"); \
        }


/*OM收到各组件上报的结构，解析头信息是否要上报*/

typedef struct
{
    OM_RCV_DATA_HEADER_STRU             stChrRcvOmHeader;
    //下发消息的时候，需要使用UINT32的ulMsgType
    //上报的时候，这里兼容需要使用usMsgType 和 usFaultId
    union{
        VOS_UINT32                          ulMsgType;
        struct {
            VOS_UINT16                          usMsgType;
            VOS_UINT16                          usFaultId;
        }stComChrType;
    }unComChrType;
    //VOS_UINT32                          ulMsgType;
    VOS_UINT32                          ulMsgSN;
    VOS_UINT32                          ulRptLen;
}OM_RCV_REPORT_STRU;


extern VOS_VOID OM_AcpuSendVComData(VOS_UINT8 *pData, VOS_UINT32 uslength);
extern VOS_INT  OM_AcpuReadVComData(VOS_UINT8 ucDevIndex, VOS_UINT8 *pucData, VOS_UINT32 ulength);
extern VOS_VOID OM_AcpuErrLogMsgProc(MsgBlock* pMsg);
extern VOS_VOID OM_AcpuErrLogReqMsgProc(MsgBlock* pMsg);
extern VOS_VOID OM_AcpuErrLogTimeoutProc(VOS_VOID);


