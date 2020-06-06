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


#ifndef __PRODUCT_NV_DEF_H__
#define __PRODUCT_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_SINGLE_GUC_BAND_CHECK_NUM       28
#define MAX_SINGLE_TL_BAND_CHECK_NUM        12
#define MAX_NV_GUC_CHECK_ITEM_NUM           32
#define MAX_NV_TL_CHECK_ITEM_NUM            64

/*��ο��ṹ������*/
typedef struct
{
    int reserved;  /*note */
}PRODUCT_MODULE_STRU;

typedef struct
{
    unsigned int uwGucCrcResult;                     		/* GUc����NV��У���� */
    unsigned int uwTlCrcResult;                      		/* TL����NV��У���� */
    unsigned int uwGucM2CrcResult;                   		/* GUC����NV��У���� */
}NV_CRC_CHECK_RESULT_STRU;

typedef struct
{
    unsigned short uhwTransmitMode;                       		/* ָʾ����ķ���ģʽ */
    unsigned short uhwBand;                              		/* ָʾ�����BAND�� */
    unsigned short uhwEnable;                              	/* ָʾ�Ƿ���Ч */
    unsigned short uhwValidCount;                              	/* ָʾ��Ч���ݵĸ��� */
    unsigned short auhwNeedCheckID[MAX_SINGLE_GUC_BAND_CHECK_NUM]; /* ������Ҫ����NV_IDֵ */
}SINGLE_GUC_BAND_NV_ID_STRU;

typedef struct
{
    unsigned short uhwTransmitMode;                       		/* ָʾ����ķ���ģʽ */
    unsigned short uhwBand;                              		/* ָʾ�����BAND�� */
    unsigned short uhwEnable;                              	/* ָʾ�Ƿ���Ч */
    unsigned short uhwValidCount;                              	/* ָʾ��Ч���ݵĸ��� */
    unsigned short auhwNeedCheckID[MAX_SINGLE_TL_BAND_CHECK_NUM]; 	/* ������Ҫ����NV_IDֵ */
}SINGLE_TL_BAND_NV_ID_STRU;

typedef struct
{
    SINGLE_GUC_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_GUC_CHECK_ITEM_NUM];  	/* ������Ҫ����NV_IDֵ */
}NV_GUC_CHECK_ITEM_STRU;

typedef struct
{
    SINGLE_TL_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_TL_CHECK_ITEM_NUM];   	/* ������Ҫ����NV_IDֵ */
}NV_TL_CHECK_ITEM_STRU;

typedef struct
{
    unsigned short uhwWorkStationNum;
    unsigned short auhwNV50001InsertPoint[5];
    unsigned short auhwNV50002InsertPoint[5];
    unsigned short auhwNV50003InsertPoint[5];
    unsigned int rsv;
}NV_WORKSTATION_INFO_STRU;

typedef struct
{
    NV_CRC_CHECK_RESULT_STRU astCRCResultTemp[6];
}NV_CRCRESULT_TEMP_STRU;

/*****************************************************************************
 �ṹ��    : CHG_BATTERY_LOW_TEMP_PROTECT_NV
 �ṹ˵��  : CHG_BATTERY_LOW_TEMP_PROTECT_NV�ṹ ID=52005
*****************************************************************************/
typedef struct
{
    unsigned int  ulIsEnable;
    signed int  lCloseAdcThreshold;
    unsigned int  ulTempLowCount;
}CHG_BATTERY_LOW_TEMP_PROTECT_NV;


typedef struct
{
    unsigned int                          ulIsEnable;
    signed int                           lCloseADCHold;
    unsigned int                          ulTempOverMax;
}OM_BATTREY_TEMP_CFG_STRU;

/*****************************************************************************
 �ṹ��    : nvi_cust_pid_type
 �ṹ˵��  : nvi_cust_pid_type�ṹ
*****************************************************************************/
typedef struct
{
    unsigned int  nv_status;
    unsigned short  cust_first_pid;
    unsigned short  cust_rewind_pid;
}nvi_cust_pid_type;

/*****************************************************************************
 �ṹ��    : NV_WEBNAS_SD_WORKMODE_STRU
 �ṹ˵��  : NV_WEBNAS_SD_WORKMODE�ṹ
*****************************************************************************/
typedef struct
{
    unsigned int ulSDWorkMode;
}NV_WEBNAS_SD_WORKMODE_STRU;

/*****************************************************************************
 �ṹ��    : NV_OLED_TEMP_ADC_STRU
 �ṹ˵��  : NV_OLED_TEMP_ADC�ṹ
*****************************************************************************/
typedef struct
{
    signed short       sTemp;
    signed short       sADC;
}NV_OLED_TEMP_ADC_STRU;

/*****************************************************************************
 �ṹ��    : NV_OLED_TEMP_ADC_STRU_ARRAY
 �ṹ˵��  : NV_OLED_TEMP_ADC_STRU_ARRAY�ṹ  ID=49
*****************************************************************************/
typedef struct
{
    NV_OLED_TEMP_ADC_STRU          stNV_OLED_TEMP_ADC[28];
}NV_OLED_TEMP_ADC_STRU_ARRAY;

/*****************************************************************************
 �ṹ��    : TEMP_ADC_STRU
 �ṹ˵��  : TEMP_ADC�ṹ
*****************************************************************************/
typedef struct
{
    signed short    sTemp;
    unsigned short    usADC;
}TEMP_ADC_STRU;

/*****************************************************************************
 �ṹ��    : TEMP_ADC_STRU_ARRAY
 �ṹ˵��  : TEMP_ADC_STRU_ARRAY�ṹ
*****************************************************************************/
typedef struct
{
    TEMP_ADC_STRU    stTempAdc[28];
}TEMP_ADC_STRU_ARRAY;

/*****************************************************************************
 �ṹ��    : USB_SN_NV_INFO_STRU
 �ṹ˵��  : USB_SN_NV_INFO�ṹ
*****************************************************************************/
typedef struct
{
    unsigned int    usbSnNvStatus;
    unsigned int    usbSnNvSucFlag;
    unsigned int    usbSnReserved1;
    /*unsigned int    usbSnReserved2;*/
}USB_SN_NV_INFO_STRU;

/*****************************************************************************
 �ṹ��    : LIVE_TIME_STRU
 �ṹ˵��  : LIVE_TIME�ṹ
*****************************************************************************/
typedef struct
{
    unsigned int    ulLiveTime;
}LIVE_TIME_STRU;

/*****************************************************************************
 �ṹ��    : LIVE_TIME_CONTROL_STRU
 �ṹ˵��  : LIVE_TIME_CONTROL�ṹ
*****************************************************************************/
typedef struct
{
    unsigned int    ulEnable;
    unsigned int    ulCycle;
} LIVE_TIME_CONTROL_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


