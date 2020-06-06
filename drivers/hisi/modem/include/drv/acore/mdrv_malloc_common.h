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
#ifndef __MDRV_COMMON_MALLOC_H__
#define __MDRV_COMMON_MALLOC_H__
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * flags, for mdrv_mem_alloc
 */
#define __MEM_ALIGN_008 0x01u /**< 8�ֽڶ���*/
#define __MEM_ALIGN_016 0x02u/**< 16�ֽڶ���*/
#define __MEM_ALIGN_032 0x04u/**< 32�ֽڶ���*/
#define __MEM_ALIGN_064 0x08u/**< 64�ֽڶ���*/
#define __MEM_ALIGN_128 0x10u/**< 128�ֽڶ���*/
#define __MEM_ALIGN_256 0x20u/**< 256�ֽڶ���*/
#define __MEM_ALIGN_512 0x40u/**< 512�ֽڶ���*/
#define __MEM_ALIGN_1K  0x80u/**< 1K�ֽڶ���*/
#define __MEM_ALIGN_MSK 0xffu

#define __MEM_DMA       0x100u/*����uncached�ڴ�*/

enum MDRV_FLAG
{
	MDRV_GFP_KERNEL
};


/*****************************************************************************
 *  �� �� ��  : mdrv_reserved_mem_malloc
 *  ��������  : ����Ԥ�����ڴ棬�������ڴ�ֻ���䲻�ͷ�
 *  �������  :
 *              size: �����ڴ��С
 *              request_id: ����ID
 *
 *  �������  :
 *  �� �� ֵ  :  NULL   ����ʧ�ܡ�
 *               ����   �ѷ����ڴ��ַ��
 *
 ******************************************************************************/
void* mdrv_reserved_mem_malloc(unsigned int size, enum MALLOC_RESERVED_REQ_ID request_id);
 
 /*****************************************************************************
  *  �� �� ��  : mdrv_cached_mem_malloc
  *  ��������  : ����cached�ڴ�
  *  �������  :
  * 			 size: �����ڴ��С
  * 			 request_id: ����ID
  *
  *  �������  :
  *  �� �� ֵ  :  NULL	 ����ʧ�ܡ�
  * 			  ����	 �ѷ����ڴ��ַ��
  *
  ******************************************************************************/
void* mdrv_cached_mem_malloc(unsigned int size); 
 
/*****************************************************************************
*  �� �� ��  : mdrv_cached_mem_free
*  ��������  : �ͷ�ָ��cached�ڴ档
*  �������  :
* 			 addr: �ڴ��׵�ַ
*
*  �������  : ��
*  �� �� ֵ  : ��
*
******************************************************************************/
void mdrv_cached_mem_free(const void* addr); 

/*****************************************************************************
 *  �� �� ��  : mdrv_uncached_mem_malloc
 *  ��������  : ����uncached�ڴ�
 *  �������  :
 *              size: �����ڴ��С
 *              request_id: ����ID
 *
 *  �������  :
 *  �� �� ֵ  :  NULL   ����ʧ�ܡ�
 *               ����   �ѷ����ڴ��ַ��
 *
 ******************************************************************************/
void* mdrv_uncached_mem_malloc(unsigned int size); 

/*****************************************************************************
 *  �� �� ��  : mdrv_uncached_mem_free
 *  ��������  : �ͷ�ָ��uncached�ڴ档
 *  �������  :
 *              addr: �ڴ��׵�ַ
 *
 *  �������  : ��
 *  �� �� ֵ  : ��
 *
 ******************************************************************************/
int mdrv_uncached_mem_free(const void* addr); 

/*****************************************************************************
 *  �� �� ��  : mdrv_mem_alloc
 *  ��������  : ����ָ�������ڴ�
 *  �������  : mid:  ģ��ID(acore��Ч)
 *              size: �����ڴ��С
 *              flag: ��������
 *
 *  �������  :
 *  �� �� ֵ  :  NULL   ����ʧ�ܡ�
 *               ����   �ѷ����ڴ��ַ��
 *
 ******************************************************************************/
void* mdrv_mem_alloc(unsigned int mid, unsigned int size, unsigned int flag);

/*****************************************************************************
 *  �� �� ��  : mdrv_mem_free
 *  ��������  : �ͷ�ָ���ڴ档
 *  �������  : mid:  ģ��ID(acore��Ч)
 *              addr: �ڴ��׵�ַ
 *
 *  �������  : ��
 *  �� �� ֵ  : MDRV_OK
 *
 ******************************************************************************/
unsigned int mdrv_mem_free(unsigned int mid, const void* addr);

#ifdef __cplusplus
}
#endif
#endif
