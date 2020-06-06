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
**************************************************************************** */
#include "OmSocketPpm.h"
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <bsp_nvim.h>
#include <osl_thread.h>
#include <osl_types.h>
#include <nv_stru_drv.h>
#include <acore_nv_stru_drv.h>
#include <securec.h>
#include "hisocket.h"
#include "diag_port_manager.h"
#include "diag_system_debug.h"
#include "scm_common.h"
#include "OmCommonPpm.h"
#include "OmPortSwitch.h"



/* ****************************************************************************
  2 ȫ�ֱ�������
**************************************************************************** */

COMM_SOCKET_CTRL_INFO_STRU g_astSockInfo[SOCKET_BUTT] =
            {{SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_OM_IND_PORT, SOCK_OM_IND_SRC_PORT_NUM,       {0,}},
             {SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_OM_CFG_PORT, SOCK_OM_CFG_SRC_PORT_NUM,       {0,}},
             {SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_AT_PORT,     SOCK_AT_SRC_PORT_NUM,           {0,}}
            };

/*****************************************************************************
  3 �ⲿ��������
*****************************************************************************/

#if ((defined(FEATURE_HISOCKET))&&(FEATURE_HISOCKET == FEATURE_ON))||(defined(FEATURE_SVLSOCKET))
/*****************************************************************************
  4 ����ʵ��
*****************************************************************************/



/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

/*���浱ǰSOCKET��״̬*/
u32          g_ulSockState = SOCK_OK;

/* ���������������Դ���������ٽ���Դ���� */
spinlock_t        g_stSockTaskSpinLock;

/*���浱ǰSOCKETOM�ĳ�ʼ��״̬*/
u32          g_ulSockOMInitState  = false;

/*���浱ǰSOCKETAT�ĳ�ʼ��״̬*/
u32          g_ulSockATInitState = false;

u32          g_ulOmIsReConn      = false;
u32          g_ulAtIsReConn      = false;


SOCKET_UDP_INFO_STRU g_stUdpInfo = {false, };
CPM_RCV_FUNC         g_AtRevFunc;

/*****************************************************************************
 Prototype      : Sock_IsEnable
 Description    : SOCK�����Ƿ�֧��
 Input          : void
 Output         : void
 Return Value   : void

 History        : ---
    Date        : 2012-05-24
    Author      : 
    Modification: Created function
*****************************************************************************/
bool PPM_SockIsEnable(void)
{
    DIAG_CHANNLE_PORT_CFG_STRU    stPortCfg = {0};

    /* ��ȡOM���������ͨ�� */
    if (NV_OK != bsp_nvm_read(NV_ID_DRV_DIAG_PORT, (u8*)&stPortCfg, sizeof(DIAG_CHANNLE_PORT_CFG_STRU)))
    {
        return false;
    }

    /* ������*/
    if (CPM_OM_PORT_TYPE_WIFI != stPortCfg.enPortNum)
    {
        return false;
    }

    return true;
}


/*****************************************************************************
 Prototype      : PPM_SockUdpInit
 Description    : ��ʼ��INDͨ����socket��������
 Input          : pstSockInfo  -- socket���ƽṹ��
 Output         : void
 Return Value   : BSP_OK :��ʼ���ɹ�
                  BSP_ERROR:��ʼ��ʧ��

 History        : ---
    Date        : 2014-05-29
    Author      : 
    Modification: Created function
 *****************************************************************************/
u32 PPM_SockUdpInit(void)
{
    COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo = g_astSockInfo + SOCKET_OM_IND;
    pstSockInfo->socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (pstSockInfo->socket > BSP_OK)
    {
        return BSP_ERROR;
    }
    /* ����SOCKET�����ź��� */
    if (CPM_WIFI_OM_IND_PORT == pstSockInfo->enPhyPort)
    {
        osl_sem_init(1, &pstSockInfo->SmClose);
    }
    else
    {
        return BSP_ERROR;
    }

    return BSP_OK;
}


/*****************************************************************************
 Prototype      : Sock_TcpInit
 Description    : ��ʼ��Socket��������
 Input          : void
 Output         : void
 Return Value   : true :��ʼ���ɹ�
                  false:��ʼ��ʧ��

 History        : ---
    Date        : 2008-05-3
    Author      : 
    Modification: Created function
 *****************************************************************************/
bool PPM_SockTcpInit(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    /* ����SOCKET�����ź��� */
    if (CPM_WIFI_OM_CFG_PORT == pstSockInfo->enPhyPort)
    {
        osl_sem_init(1, &pstSockInfo->SmClose);
    }
    else if (CPM_WIFI_AT_PORT == pstSockInfo->enPhyPort)
    {
        osl_sem_init(1, &pstSockInfo->SmClose);
    }
    else
    {
        return false;
    }
    return true;
}

/*****************************************************************************
 Prototype      : PPM_SockBindListen
 Description    :
 Input          : lpParameter - Unused.
 Output         : void
 Return Value   : void

 History        : ---
    Date        : 2008-05-3
    Author      : 
    Modification: Created function
*****************************************************************************/
void PPM_SockBindListen(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    struct sockaddr_in  sAddr;
    s32             lAddLen;

    if((SOCK_NULL != pstSockInfo->listener)||
       (SOCK_NULL != pstSockInfo->socket))
    {
        return;
    }

    pstSockInfo->listener = socket(AF_INET, SOCK_STREAM, 0);

    if (pstSockInfo->listener < BSP_OK)
    {
        diag_error("socket err!\n");
        pstSockInfo->listener = SOCK_NULL;
        msleep(100);/*delay 100 ms*/
        return ;
    }

    (void)memset_s(&sAddr, sizeof(sAddr), 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;

    sAddr.sin_addr.s_addr = 0;

    /* �����Ķ˿ں� */
    sAddr.sin_port = htons(pstSockInfo->usPort);

    lAddLen = sizeof(struct sockaddr_in);

    /* ������Socket�󶨵���Ӧ�Ķ˿��� */
    if (SOCKET_ERROR == bind(pstSockInfo->listener, (struct sockaddr *)&sAddr, lAddLen))
    {
        closesocket(pstSockInfo->listener);
        pstSockInfo->listener = SOCK_NULL;
        diag_error("bind err !\n");
        return ;
    }

    /* ���÷������˼��������ͻ����� */
    if (SOCKET_ERROR == listen(pstSockInfo->listener, SOCKET_NUM_MAX))
    {
        closesocket(pstSockInfo->listener);
        pstSockInfo->listener = SOCK_NULL;
        diag_error("listen err !\n");
        return ;
    }
}

void PPM_SockAcceptRecv(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    Ip_fd_set                               listen1,listen2;
    s32                               ret;
    s32                               len;
    struct sockaddr_in                      from;
    unsigned long                               ulLockLevel;
    SOCKET                                  socket;
    SOCKET                                  maxSocket;

    if((SOCK_NULL == pstSockInfo->listener)&&(SOCK_NULL == pstSockInfo->socket))
    {
        diag_error("listener && socket err ! %d\n",pstSockInfo->usPort);
        return;
    }
    HI_FD_ZERO(&listen1);
    HI_FD_ZERO(&listen2);

    HI_FD_SET((u32)pstSockInfo->listener, &listen1);

    if(SOCK_NULL != pstSockInfo->socket)
    {
        HI_FD_SET((u32)pstSockInfo->socket, &listen1);
    }

    memcpy_s(&listen2, sizeof(listen2), &listen1,sizeof(listen1));

    maxSocket = pstSockInfo->socket > pstSockInfo->listener ? pstSockInfo->socket : pstSockInfo->listener;

    ret = select((int)maxSocket + 1,&listen1, NULL, &listen2, NULL);
    if(ret < 0)
    {
        diag_error("select err ! %d \n",pstSockInfo->usPort);
        return;
    }

    if((pstSockInfo->listener != SOCK_NULL) && (HI_FD_ISSET((u32)pstSockInfo->listener, &listen1)))
    {
        len = sizeof(struct sockaddr_in);
        socket = accept(pstSockInfo->listener, (struct sockaddr *)&from, (int *)&len);
        if(socket < 0)
        {
            closesocket(pstSockInfo->listener);
            pstSockInfo->listener = SOCK_NULL;
            diag_error("accept err !%d \n",pstSockInfo->usPort);
        }
        else
        {
            if(SOCK_NULL != pstSockInfo->socket)
            {
                if(pstSockInfo->enPhyPort == CPM_WIFI_OM_CFG_PORT)
                {
                    g_ulOmIsReConn = true;
                }
                if(pstSockInfo->enPhyPort == CPM_WIFI_AT_PORT)
                {
                    g_ulAtIsReConn = true;
                }
                (void)osl_sem_down(&(pstSockInfo->SmClose));
                closesocket(pstSockInfo->socket);
                (void)osl_sem_up(&(pstSockInfo->SmClose));
            }

            (void)osl_sem_down(&(pstSockInfo->SmClose));
            pstSockInfo->socket = socket;
            (void)osl_sem_up(&(pstSockInfo->SmClose));

            if(CPM_WIFI_OM_CFG_PORT == pstSockInfo->enPhyPort)
            {
                if(0 > getpeername(pstSockInfo->socket, &g_stUdpInfo.stAddr, (int *)&len))
                {
                    diag_error("getpeername failed.\n");
                }
                else
                {
                    /* ��ȡ�Զ˵�ַ���޸Ķ˿ںţ�����UDP���� */
                    g_stUdpInfo.stAddr.sin_port = htons(g_astSockInfo[SOCKET_OM_IND].usPort);

                    g_stUdpInfo.bStatus = true;
                }
            }
        }
    }

    if(SOCK_NULL == pstSockInfo->socket)
    {
        diag_error("socket=NULL, Port=%d!\n",pstSockInfo->usPort);
        return;
    }

    ret = recv((int)pstSockInfo->socket, pstSockInfo->aucBuf, SOCK_OM_MSG_LEN, 0);
    if(ret <= 0)    /*�ͻ��˶Ͽ�֮�����˻�����ܵ�����Ϊ0�����ݰ�*/
    {
        (void)osl_sem_down(&(pstSockInfo->SmClose));
        shutdown(pstSockInfo->socket,SHUT_RDWR);
        pstSockInfo->socket   = SOCK_NULL;
        (void)osl_sem_up(&(pstSockInfo->SmClose));
        if(pstSockInfo->enPhyPort == CPM_WIFI_OM_CFG_PORT)
        {
            PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_CNF);

            g_stUdpInfo.bStatus = false;
        }
        diag_error("rcv err(0x%x), port=%d\n",ret,pstSockInfo->usPort);
        return;
    }

    if(pstSockInfo->enPhyPort == CPM_WIFI_OM_CFG_PORT)
    {
        OM_ACPU_DEBUG_CHANNEL_TRACE(pstSockInfo->enPhyPort, (u8*)pstSockInfo->aucBuf, ret, OM_ACPU_RECV_SOCKET, OM_ACPU_DATA);
    }
    scm_SpinLockIntLock(&g_stSockTaskSpinLock, ulLockLevel);
    /*�����յ��������ύ���ϲ㴦��*/
    ret = (s32)CPM_ComRcv(pstSockInfo->enPhyPort, pstSockInfo->aucBuf, ret);
    if(ret)
    {
        diag_error("CPM_ComRcv error(0x%x)\n", ret);
    }
    scm_SpinUnlockIntUnlock(&g_stSockTaskSpinLock, ulLockLevel);
}

/*****************************************************************************
 Prototype      : Sock_ServerProc
 Description    : �������̣߳���������������˺Ϳͻ��˵���������
 Input          : lpParameter - Unused.
 Output         : void
 Return Value   : void

 History        : ---
    Date        : 2008-05-3
    Author      : 
    Modification: Created function
*****************************************************************************/
void PPM_SockServerProc(COMM_SOCKET_CTRL_INFO_STRU *pstSockInfo)
{
    for(;;)
    {
        PPM_SockBindListen(pstSockInfo);
        PPM_SockAcceptRecv(pstSockInfo);
    }

}

/*****************************************************************************
 Prototype      : PPM_SockOmServerTask
 Description    : �������̣߳���������OM��������
 Input          : lpParameter - Unused.
 Output         : void
 Return Value   : void

 History        : ---
    Date        : 2014-05-29
    Author      : 
    Modification: Created function
*****************************************************************************/
void PPM_SockOmServerTask(void)
{
    /* ��ʼ��CFG�ڵ�SOCKET������TCPЭ�� */
    if (false == PPM_SockTcpInit(g_astSockInfo + SOCKET_OM_CFG))
    {
        return;
    }

    g_ulSockState = SOCK_START;

	g_ulSockOMInitState = true;
    /* CFG���Դ���ѭ��������� */
    PPM_SockServerProc(g_astSockInfo + SOCKET_OM_CFG);

    return;
}


/*****************************************************************************
 Prototype      : Sock_AtServerTask
 Description    : �������̣߳���������AT��������
 Input          : lpParameter - Unused.
 Output         : void
 Return Value   : void

 History        : ---
    Date        : 2014-05-29
    Author      : 
    Modification: Created function
*****************************************************************************/
void PPM_SockAtServerTask(void)
{
    /* ��ʼ��SOCKET */
    if (false == PPM_SockTcpInit(g_astSockInfo + SOCKET_AT))
    {
        return;
    }

    g_ulSockATInitState = true;

    PPM_SockServerProc(g_astSockInfo + SOCKET_AT);

    return;
}

/* UDP�����������65535�ֽڣ���ȥUDP��ͷ�����65507���ֽ� */
#define UDP_MAX_LEN	    65507

/* ��ȡUDP�˿ڷ��͵���Ϣ */
u32 PPM_SocketGetUdpInfo(void)
{
    return g_stUdpInfo.ulTotalLen;
}


/*****************************************************************************
 Prototype      : PPM_SockOMIndComSend
 Description    : �ṩ���ϲ�OM �����ϱ����ݷ��͵Ľӿ�.
 Input          : pucVirAddr:   �������ַ
                  pucPhyAddr:   ����ʵ��ַ
                  ulLength:     ���ݳ���
 Output         : void
 Return Value   : BSP_ERROR  - ����ʧ��
                  BSP_OK   - ���ͳɹ�

 History        : ---
    Date        : 2014-05-29
    Author      : 
    Modification: Created function
 *****************************************************************************/
u32 PPM_SockOMIndComSend(u8 *pucVirAddr, u8 *pucPhyAddr, u32 uslength)
{
    SOCKET                              socket;
    s32                             nSndNum;
    u16  ustimes = 0, uslen = 0, i;

    (void)osl_sem_down(&(g_astSockInfo[SOCKET_OM_IND].SmClose));

    socket = g_astSockInfo[SOCKET_OM_IND].socket;

    if ((SOCK_NULL == socket) || (false == g_stUdpInfo.bStatus))
    {
        (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_IND].SmClose));

        return BSP_ERROR;
    }

    if(uslength > UDP_MAX_LEN)
    {
        ustimes = uslength / UDP_MAX_LEN;
        uslen   = uslength % UDP_MAX_LEN;
    }
    else
    {
        ustimes = 0;
        uslen   = uslength;
    }

    /* ����sendto������ͨ��socket���ͳ�ȥ����UDP */
    for(i = 0; i < ustimes; i++)
    {
        diag_system_debug_socket_len(UDP_MAX_LEN);

        nSndNum = sendto(socket, (pucVirAddr + (i*UDP_MAX_LEN)), UDP_MAX_LEN, 0, &g_stUdpInfo.stAddr, sizeof(g_stUdpInfo.stAddr));
        if (nSndNum != UDP_MAX_LEN)
        {
            diag_system_debug_socket_fail_len(UDP_MAX_LEN);

            diag_error("uslength %d, nSndNum %d.\n", UDP_MAX_LEN, nSndNum);

            (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_IND].SmClose));

            return BSP_ERROR;
        }
        else
        {
            diag_system_debug_socket_sucess_len(UDP_MAX_LEN);
            g_stUdpInfo.ulTotalLen += nSndNum;
        }
    }

    if(0 != uslen)
    {
        diag_system_debug_socket_len(uslen);
        nSndNum = sendto(socket, (pucVirAddr + (ustimes*UDP_MAX_LEN)), uslen, 0, &g_stUdpInfo.stAddr, sizeof(g_stUdpInfo.stAddr));
        if (nSndNum != uslen)
        {
            diag_system_debug_socket_fail_len(uslen);

            diag_error("uslength %d, nSndNum %d.\n", uslen, nSndNum);

            (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_IND].SmClose));

            return BSP_ERROR;
        }
        else
        {
            diag_system_debug_socket_sucess_len(uslen);

            g_stUdpInfo.ulTotalLen += nSndNum;
        }
    }
    diag_system_debug_send_data_end();

    (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_IND].SmClose));

    return BSP_OK;
}


/*****************************************************************************
 Prototype      : PPM_SockOMCfgComSend
 Description    : �ṩ���ϲ�OM CFG��Ӧ���ݷ��͵Ľӿ�.
 Input          : pucVirAddr:   �������ַ
                  pucPhyAddr:   ����ʵ��ַ
                  ulLength:     ���ݳ���
 Output         : void
 Return Value   : BSP_ERROR  - ����ʧ��
                  BSP_OK   - ���ͳɹ�

 History        : ---
    Date        : 2014-05-29
    Author      : 
    Modification: Created function
 *****************************************************************************/
u32 PPM_SockOMCfgComSend(u8* pucVirAddr, u8 *pucPhyAddr, u32 uslength)
{
    SOCKET socket;
    s32 nSndNum;

    if (false == g_ulSockOMInitState)
    {
        return BSP_ERROR;
    }

    (void)osl_sem_down(&(g_astSockInfo[SOCKET_OM_CFG].SmClose));

    socket = g_astSockInfo[SOCKET_OM_CFG].socket;

    if (SOCK_NULL == socket)
    {
        (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_CFG].SmClose));
        diag_error("socket err !\n");
        return BSP_ERROR;
    }
    OM_ACPU_DEBUG_CHANNEL_TRACE(CPM_WIFI_OM_CFG_PORT, (u8*)pucVirAddr, uslength, OM_ACPU_SEND_SOCKET, OM_ACPU_DATA);

    /* ����send������ͨ��socket���ͳ�ȥ����TCP */
    nSndNum = send(socket, pucVirAddr, uslength, 0);

    (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_CFG].SmClose));

    if (nSndNum != uslength)
    {
        diag_error("send err %d,%d!\n",nSndNum,uslength);
        if(g_ulOmIsReConn == true)
        {
            diag_crit("Re Conn %d,%d!\n",nSndNum,uslength);
            return BSP_ERROR;
        }
        PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_CNF);

        g_stUdpInfo.bStatus = false;

        (void)osl_sem_down(&(g_astSockInfo[SOCKET_OM_CFG].SmClose));

        closesocket(g_astSockInfo[SOCKET_OM_CFG].socket);

        g_astSockInfo[SOCKET_OM_CFG].socket = SOCK_NULL;

        (void)osl_sem_up(&(g_astSockInfo[SOCKET_OM_CFG].SmClose));

        return BSP_ERROR;
    }

    g_ulOmIsReConn = false;

    return BSP_OK;
}

/*****************************************************************************
 Prototype      : PPM_SockATComSend
 Description    : �ṩ���ϲ�AT�������ݵĽӿ�.
 Input          : pucVirAddr:   �������ַ
                  pucPhyAddr:   ����ʵ��ַ
                  ulLength:     ���ݳ���
 Output         : void
 Return Value   : BSP_ERROR  - ����ʧ��
                  BSP_OK   - ���ͳɹ�

 History        : ---
    Date        : 2014-05-29
    Author      : 
    Modification: Created function
 *****************************************************************************/
u32 PPM_SockATComSend(u8* pucVirAddr, u8 *pucPhyAddr, u32 uslength)
{
    SOCKET socket;
    s32 nSndNum;

    if (false == g_ulSockATInitState)
    {
        return BSP_ERROR;
    }

    (void)osl_sem_down(&(g_astSockInfo[SOCKET_AT].SmClose));

    socket = g_astSockInfo[SOCKET_AT].socket;

    if (SOCK_NULL == socket)
    {
        (void)osl_sem_up(&(g_astSockInfo[SOCKET_AT].SmClose));
        diag_error("socket err !\n");
        return BSP_ERROR;
    }

    /* ����send������ͨ��socket���ͳ�ȥ����TCP */
    nSndNum = send(socket, pucVirAddr, uslength, MSG_DONTWAIT);

    (void)osl_sem_up(&(g_astSockInfo[SOCKET_AT].SmClose));
    if (nSndNum != uslength)
    {
        diag_error("send err %d,%d!\n",nSndNum,uslength);
        if(g_ulAtIsReConn == true)
        {
            diag_crit("Re Conn %d,%d!\n",nSndNum,uslength);
            return BSP_ERROR;
        }
        (void)osl_sem_down(&(g_astSockInfo[SOCKET_AT].SmClose));
        closesocket(g_astSockInfo[SOCKET_AT].socket);
        g_astSockInfo[SOCKET_AT].socket =SOCK_NULL;
        (void)osl_sem_up(&(g_astSockInfo[SOCKET_AT].SmClose));
        return BSP_ERROR;
    }

    g_ulAtIsReConn = false;

    return BSP_OK;
}




u32 PPM_SockPortInit(void)
{
    if(PPM_SockInitTask())
    {
        return BSP_ERROR;
    }

    if ((BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
        && (true == PPM_SockIsEnable()))
    {
        CPM_PhySendReg(CPM_WIFI_OM_IND_PORT,    (CPM_SEND_FUNC)PPM_SockOMIndComSend);
        CPM_PhySendReg(CPM_WIFI_OM_CFG_PORT,    (CPM_SEND_FUNC)PPM_SockOMCfgComSend);
        CPM_PhySendReg(CPM_WIFI_AT_PORT,        (CPM_SEND_FUNC)PPM_SockATComSend);

        /* ���ڳ�ʼ��˳�����⣬AT��ͨ�����պ�����Ҫ�ڴ˴�ע�ᣬ������Ҫ��ATͨ�����Ӹ�NAS */
        CPM_LogicRcvReg(CPM_AT_COMM,            PPM_SocketAtRevFun);
    }

    scm_SpinLockInit(&g_stSockTaskSpinLock);

    diag_crit("socket init ok\n");

    return BSP_OK;
}

u32 PPM_SockInitTask(void)
{
    u32 ulRelVal = 0;
    OSL_TASK_ID task_id = 0;

    if((BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    && (true == PPM_SockIsEnable()))
    {
        /* ����SOCKET���ݵ��Դ������� */
        ulRelVal = (u32)osl_task_init("om_server", 76, 8096,(OSL_TASK_FUNC)PPM_SockOmServerTask, NULL,(OSL_TASK_ID*)&task_id);
        if (ulRelVal )
        {
            diag_error("creta sock om server task fail\n");
            return BSP_ERROR;
        }

        /* ����SOCKET���ݵ��Դ������� */
        ulRelVal = (u32)osl_task_init("om_server", 76, 8096,(OSL_TASK_FUNC)PPM_SockAtServerTask, NULL,(OSL_TASK_ID*)&task_id);
        if (ulRelVal)
        {
            diag_error("creta sock at server task fail\n");
            return BSP_ERROR;
        }

        PPM_SockUdpInit();

        return BSP_OK;
    }

    return BSP_ERROR;
}

void PPM_SocketRevFunReg(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, CPM_RCV_FUNC pRcvFunc)
{
    if(enLogicPort != CPM_AT_COMM)
    {
        diag_error("logic num error, 0x%x\n", enLogicPort);
    }
    else
    {
        g_AtRevFunc = pRcvFunc;
    }
    return;
}

u32 PPM_SocketAtRevFun(u8  *pucData, u32 ulLen)
{
    if(g_AtRevFunc)
    {
        return g_AtRevFunc(pucData, ulLen);
    }
    else
    {
        diag_error("at rev fuc is null\n");
        return ERR_MSP_DIAG_CB_NULL_ERR;
    }
}
#else
void PPM_SocketRevFunReg(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, CPM_RCV_FUNC pRcvFunc)
{
    return;
}
u32 PPM_SocketAtRevFun(u8  *pucData, u32 ulLen)
{
    return 0 ;
}
#endif






