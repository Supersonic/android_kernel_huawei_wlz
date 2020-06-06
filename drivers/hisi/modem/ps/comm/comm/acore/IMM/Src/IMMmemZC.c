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
#include "IMMmemZC.h"
#include "v_id.h"
#include "TTFUtil.h"
#include "securec.h"


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_IMM_ZC_C
/*lint +e767*/

/******************************************************************************
   2 ����ʵ��
******************************************************************************/
/*
 * ��A�������ڴ��������ڴ�ķ��亯��
 * ����ֵ: �ɹ�������Ŀ��IMM_ZC_STRU��ָ�� ʧ�ܣ�����NULL
 */
IMM_ZC_STRU *IMM_ZcStaticAlloc_Debug(unsigned short fileID, unsigned short lineNum, unsigned int len)
{
    IMM_ZC_STRU *alloc = VOS_NULL_PTR;

    /* ���ܻ���̬��, ����skbϵͳ�ڴ� */
    alloc = (IMM_ZC_STRU *)IMM_ZcLargeMemAlloc(len);

    return alloc;
}

/*
 * IMM_ZC�����ݽṹ�Ŀ��������ƽڵ�����ݿ飩�������ݿ��Linux�Դ����ڴ濽����A�˹����ڴ�
 * A����������ΪMEM_TYPE_SYS_DEFINED�����ݿ飬���ݴ���C��ǰ��һ��Ҫ���ñ��ӿڣ������ݿ�����A�˹����ڴ�
 * ����ֵ: �ɹ�������Ŀ��IMM_ZC_STRU��ָ�� ʧ�ܣ�����NULL
 */
IMM_ZC_STRU *IMM_ZcStaticCopy_Debug(VOS_UINT16 fileID, VOS_UINT16 lineNum, IMM_ZC_STRU *immZc)
{
    return NULL;
}

/* �ͷ�IMM_ZC_STRU���ƽڵ㣬���ݿ鲻�ͷš� */
void IMM_ZcHeadFree(IMM_ZC_STRU *immZc)
{
    return;
}

/* ��IMM_Zc�㿽�����ƽڵ�ת����IMM_Mem���ƽڵ� */
IMM_MEM_STRU *IMM_ZcMapToImmMem_Debug(unsigned short fileID, unsigned short lineNum, IMM_ZC_STRU *immZc)
{
    return NULL;
} /* IMM_ZcMapToImmMem */

/* ��IMM_ZC_STRU�㿽���ṹ���MACͷ */
unsigned int IMM_ZcAddMacHead(IMM_ZC_STRU *immZc, const unsigned char *addData)
{
    unsigned char *destAddr = VOS_NULL_PTR;

    if (NULL == immZc) {
        return VOS_ERR;
    }

    if (NULL == addData) {
        return VOS_ERR;
    }

    if (IMM_MAC_HEADER_RES_LEN > (immZc->data - immZc->head)) {
        return VOS_ERR;
    }

    destAddr = IMM_ZcPush(immZc, IMM_MAC_HEADER_RES_LEN);
    TTF_ACORE_SF_CHK(memcpy_s(destAddr, IMM_MAC_HEADER_RES_LEN, addData, IMM_MAC_HEADER_RES_LEN));

    return VOS_OK;
}

/* �Ƴ��㿽���ṹMACͷ */
unsigned int IMM_ZcRemoveMacHead(IMM_ZC_STRU *immZc)
{
    if (NULL == immZc) {
        return VOS_ERR;
    }

    if (IMM_MAC_HEADER_RES_LEN > immZc->len) {
        return VOS_ERR;
    }

    IMM_ZcPull(immZc, IMM_MAC_HEADER_RES_LEN);

    return VOS_OK;
}

/* ������ӵ���Ч���ݿ��ͷ���� ���ӿ�ֻ�ƶ�ָ��, ������ӵ���Ч���ݿ��ͷ��֮ǰ,���ñ��ӿ�*/
unsigned char *IMM_ZcPush_Debug(unsigned short fileID, unsigned short lineNum, IMM_ZC_STRU *immZc,
                                unsigned int len)
{
    unsigned char *ret = NULL;

    ret = skb_push((immZc), (len));

    return ret;
}

/* ��IMM_ZCָ������ݿ��ͷ��ȡ�����ݡ� */
unsigned char *IMM_ZcPull_Debug(unsigned short fileID, unsigned short lineNum, IMM_ZC_STRU *immZc,
                                unsigned int len)
{
    unsigned char *ret = NULL;

    ret = skb_pull(immZc, len);

    return ret;
}

/* ���������IMM_ZCָ������ݿ��β���� */
unsigned char *IMM_ZcPut_Debug(unsigned short fileID, unsigned short lineNum, IMM_ZC_STRU *immZc,
                               unsigned int len)
{
    unsigned char *ret = NULL;

    ret = skb_put(immZc, len);

    return ret;
}

/* Ԥ��IMM_ZCָ������ݿ�ͷ���ռ䡣 */
void IMM_ZcReserve_Debug(unsigned short fileID, unsigned short lineNum, IMM_ZC_STRU *immZc, unsigned int len)
{
    skb_reserve(immZc, (int)len);

    return;
}

/* �õ�UserApp�� */
unsigned short IMM_ZcGetUserApp(IMM_ZC_STRU *immZc)
{
    return IMM_PRIV_CB(immZc)->usApp;
}

/* ����UserApp�� */
void IMM_ZcSetUserApp(IMM_ZC_STRU *immZc, unsigned short app)
{
    if (NULL == immZc) {
        return;
    }

    IMM_PRIV_CB(immZc)->usApp = app;

    return;
}

/* ���ݿ�Ĺҽӵ�IMM_ZC_STRU�ϡ� */
IMM_ZC_STRU *IMM_ZcDataTransformImmZc_Debug(unsigned short fileID, unsigned short lineNum,
                                            const unsigned char *data, unsigned int len, void *ttfMem)
{
    return NULL;
} /* IMM_ZcDataTransformImmZc_Debug */

