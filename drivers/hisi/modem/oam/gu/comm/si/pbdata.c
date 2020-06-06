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

/************************************************************************
  Copyright    : 2005-2007, Huawei Tech. Co., Ltd.
  File name    : pbdata.c
  Author       : zhuli
  Version      : V100R002
  Date         : 2008-5-15
  Description  : ��C�ļ�������---��ɻ������ݴ���ģ��ʵ��
  Function List:
  History      :
 ************************************************************************/

#include "si_pb.h"

#if (FEATURE_PHONE_SC == FEATURE_OFF)
#include "TafTypeDef.h"
#include "UsimPsInterface.h"
#include "omprivate.h"
#if (VOS_OSA_CPU  == OSA_CPU_CCPU)
#include "UsimmApi.h"
#endif
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define     THIS_FILE_ID    PS_FILE_ID_PB_DATA_C
#define     THIS_MODU       mod_pam_pb

#if (FEATURE_PHONE_SC == FEATURE_OFF)

VOS_UINT32 SI_PB_FindPBOffset(SI_PB_TYPE_ENUM_UINT32  enPBType, VOS_UINT8 *pucOffset)
{
    VOS_UINT8 i;

    for(i=0; i<SI_PB_MAX_NUMBER; i++)
    {
        if(gastPBContent[i].enPBType == enPBType)   /*��ǰ�Ĳ�ѯ����һ��*/
        {
            PB_INFO_LOG("SI_PB_FindPBOffset Info: Locate the PhoneBook Accurately");

            *pucOffset = i;

            return VOS_OK;
        }
    }

    PB_ERROR_LOG("SI_PB_FindPBOffset Error: The PhoneBook Info is Not Exist");

    return VOS_ERR;             /*��ǰδ�ҵ�ƫ��*/
}



VOS_UINT32 SI_PB_LocateRecord(SI_PB_TYPE_ENUM_UINT32  enPBType, VOS_UINT16 usIndex1, VOS_UINT16 usIndex2, VOS_UINT8 *pucNumber)
{
    VOS_UINT8   ucPBOffset;
    VOS_UINT32 ulResult;

    ulResult = SI_PB_FindPBOffset(enPBType, &ucPBOffset);    /*���Ȳ�ѯ��ǰ�Ļ���λ��*/

    if(VOS_ERR == ulResult)
    {
        PB_WARNING_LOG("SI_PB_LocateRecord: Locate PhoneBook Error");

        return TAF_ERR_UNSPECIFIED_ERROR;
    }

    if(PB_NOT_INITIALISED == gastPBContent[ucPBOffset].enInitialState)   /*��ǰ�ĵ绰��û�г�ʼ�����*/
    {
        PB_ERROR_LOG("SI_PB_LocateRecord:The PhoneBook is Not Initializtion");

        return TAF_ERR_SIM_BUSY;
    }

    if(PB_FILE_NOT_EXIST == gastPBContent[ucPBOffset].enInitialState)   /*��ǰ�ĵ绰��û�г�ʼ�����*/
    {
        PB_ERROR_LOG("SI_PB_LocateRecord:The PhoneBook is Not Exit");

        return TAF_ERR_FILE_NOT_EXIST;
    }

    if((usIndex1 > gastPBContent[ucPBOffset].usTotalNum)
        || (usIndex2 > gastPBContent[ucPBOffset].usTotalNum)
        || (usIndex1 > usIndex2))/*��ǰ�������Ѿ�������Χ*/
    {
        PB_WARNING_LOG("SI_PB_LocateRecord: The Index is Not in The Range of PhoneBook");

        return TAF_ERR_PB_WRONG_INDEX;
    }

    *pucNumber = ucPBOffset;        /*���ص�ǰ�ĵ绰������ƫ��*/

    return VOS_OK;
}


VOS_UINT32 SI_PB_CountADNRecordNum(VOS_UINT16 usIndex, VOS_UINT16 *pusFileId, VOS_UINT8 *pucRecordNum)
{
    VOS_UINT8 i;
    VOS_UINT16 usTemp = 0;

    for(i=0; i<SI_PB_ADNMAX; i++)   /*���ݵ�ǰ��ADN�б�����ѭ��*/
    {
        if((usTemp < usIndex)&&(usIndex <= (gstPBCtrlInfo.astADNInfo[i].ucRecordNum + usTemp)))/*���������ļ��ķ�Χ��*/
        {
            *pusFileId = gstPBCtrlInfo.astADNInfo[i].usFileID;  /*���ص�ǰ���ļ�ID*/
            *pucRecordNum = (VOS_UINT8)(usIndex - usTemp);/*���ص�ǰ�ļ�¼��*/

            return VOS_OK;
        }
        else
        {
            usTemp += gstPBCtrlInfo.astADNInfo[i].ucRecordNum;  /*�������ۼ�*/
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetXDNFileID(VOS_UINT32 ulStorage, VOS_UINT16 *pusFileId)
{
    VOS_UINT32 ulResult = VOS_OK;

    switch(ulStorage)
    {
        case SI_PB_STORAGE_ON:
            *pusFileId = EFMSISDN;
            break;
        case SI_PB_STORAGE_FD:
            *pusFileId = EFFDN;
            break;
        case SI_PB_STORAGE_BD:
            *pusFileId = EFBDN;
            break;
        default:
            ulResult = VOS_ERR;
    }

    return ulResult;
}


VOS_VOID SI_PB_TransToAscii(
    VOS_UINT8                            ucNumber,
    VOS_UINT8                           *pucAsciiNum
)
{
    if(ucNumber <= 9)           /*ת������*/
    {
        *pucAsciiNum = ucNumber + 0x30;
    }
    else if(0x0A == ucNumber)   /*ת��*�ַ�*/
    {
        *pucAsciiNum = 0x2a;
    }
    else if(0x0B == ucNumber)   /*ת��#�ַ�*/
    {
        *pucAsciiNum = 0x23;
    }
    else if(0x0C == ucNumber)   /*ת��'P'�ַ�*/
    {
        *pucAsciiNum = 0x50;
    }
    else if(0x0D == ucNumber)   /*ת��'?'�ַ�*/
    {
        *pucAsciiNum = 0x3F;
    }
    else                        /*ת����ĸ*/
    {
        *pucAsciiNum = ucNumber + 0x57;
    }

}


VOS_VOID SI_PB_BcdToAscii(VOS_UINT8 ucBcdNumLen, VOS_UINT8 *pucBcdNum, VOS_UINT32 asciiBuffSize, VOS_UINT8 *pucAsciiNum, VOS_UINT8 *pucLen)
{
    VOS_UINT8       ucTmp;
    VOS_UINT8       ucLen = 0;
    VOS_UINT8       ucFirstNumber;
    VOS_UINT8       ucSecondNumber;

    if ((ucBcdNumLen*2) > asciiBuffSize)
    {
        *pucLen = 0;

        return;
    }

    for (ucTmp = 0; ucTmp < ucBcdNumLen; ucTmp++)
    {
        if(0xFF == pucBcdNum[ucTmp])
        {
            break;
        }

        ucFirstNumber  = (VOS_UINT8)(pucBcdNum[ucTmp] & 0x0F); /*ȡ���߰��ֽ�*/

        ucSecondNumber = (VOS_UINT8)((pucBcdNum[ucTmp] >> 4) & 0x0F);/*ȡ���Ͱ��ֽ�*/

        SI_PB_TransToAscii(ucFirstNumber, pucAsciiNum);

        pucAsciiNum++;

        ucLen++;

        if(0x0F == ucSecondNumber)
        {
            break;
        }

        SI_PB_TransToAscii(ucSecondNumber, pucAsciiNum);

        pucAsciiNum++;

        ucLen++;
    }

    *pucLen = ucLen;

    return;
}


VOS_VOID SI_PB_DecodePBName(VOS_UINT8 ucNameMax, VOS_UINT8 *pucName,
                                        VOS_UINT32 *pAlphaType, VOS_UINT8 *pNameLen)
{
    VOS_UINT8 i = 0;

    /* ������UCS2 80 */
    if (SI_PB_ALPHATAG_TYPE_UCS2_80 == pucName[0])
    {
        *pAlphaType = pucName[0];

        ucNameMax--;
        for(i=0;i<(ucNameMax-(ucNameMax%2));i+=2)   /*������ǰ����������*/
        {
            if((pucName[i+1] == 0xFF)&&(pucName[i+2] == 0xFF))
            {
                break;
            }
        }

        *pNameLen = i;      /*�������ȷ���*/

        return;
    }

    /* ������UCS2 81 */
    if (SI_PB_ALPHATAG_TYPE_UCS2_81 == pucName[0])
    {
        *pAlphaType = pucName[0];

        /* TAG(81)+ length + 1bytes base address */
        if (ucNameMax >= 3)
        {
            if(pucName[1] > (ucNameMax - 3))/* �����ǰ������Ϣ������󳤶ȣ���ֱ�ӽ׽ض� */
            {
               i = ucNameMax -1;
            }
            else
            {
               i = pucName[1] + 2;     /*���㵱ǰ��������*/
            }
        }

        *pNameLen = i;      /*�������ȷ���*/

        return;
    }

    /* ������UCS2 82 */
    if (SI_PB_ALPHATAG_TYPE_UCS2_82 == pucName[0])
    {
        *pAlphaType = pucName[0];

        /* TAG(82)+ length + 2bytes base address */
        if (ucNameMax >= 4)
        {
            if(pucName[1] > (ucNameMax - 4))/* �����ǰ������Ϣ������󳤶ȣ���ֱ�ӽ׽ض� */
            {
               i = ucNameMax -1;
            }
            else
            {
               i = pucName[1] + 3;     /*���㵱ǰ��������*/
            }
        }

        *pNameLen = i;      /*�������ȷ���*/

        return;
    }


    /* ������GSM��ʽ�洢 */
    *pAlphaType = SI_PB_ALPHATAG_TYPE_GSM;

    for (i = 0; i < ucNameMax; i++)         /*������ǰ����������*/
    {
        if (0xFF == pucName[i])
        {
            break;
        }
    }

    *pNameLen = i;      /*�������ȷ���*/

    return;
}


VOS_UINT32 SI_PB_CheckExtRecord(VOS_UINT8 extRecord, VOS_UINT32 extInfoNum)
{
    if ((extRecord != 0xFF) &&
        (extRecord != 0) &&
        (extRecord <= gastEXTContent[extInfoNum].usExtTotalNum))
    {
        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_VOID SI_PB_TransPBFromate(SI_PB_CONTENT_STRU *pstPBContent, VOS_UINT16 usIndex, VOS_UINT8 *pContent, SI_PB_RECORD_STRU *pstRecord)
{
    VOS_UINT32 ulResult;
    VOS_UINT32 ulExtInfoNum;
    VOS_UINT8  ucExtRecord;
    VOS_UINT8  aucPhoneNumber[SI_PB_NUM_LEN] = {0};
    VOS_UINT8  *pucExtContent = VOS_NULL_PTR;
    VOS_UINT8  ucExtNumLen;
    VOS_UINT8  ucXdnNumLen;
    errno_t    ret;

    ulResult = SI_PB_CheckContentValidity(pstPBContent, pContent, pstPBContent->ucRecordLen);/*��鵱ǰ�������Ƿ���Ч*/

    if(ulResult != VOS_OK)
    {
        pstRecord->ValidFlag = SI_PB_CONTENT_INVALID;     /*��ǵ�ǰ��������Ч*/

        /*ȫ��Ϊ0*/
    }
    else
    {
        pstRecord->ValidFlag = SI_PB_CONTENT_VALID;/*��ǵ�ǰ��������Ч*/

        SI_PB_DecodePBName(pstPBContent->ucNameLen, pContent,
                            &pstRecord->AlphaTagType, &pstRecord->ucAlphaTagLength);

        if(pstRecord->ucAlphaTagLength != 0x00)         /*��ǰ������Ϊ��*/
        {
            pstRecord->ucAlphaTagLength = (VOS_UINT8)PAM_GetMin(pstRecord->ucAlphaTagLength, SI_PB_ALPHATAG_MAX_LEN);

            if(pstRecord->AlphaTagType == SI_PB_ALPHATAG_TYPE_GSM)  /*����Ӣ��������ͷ��ʼ*/
            {
                ret = memcpy_s(pstRecord->AlphaTag, SI_PB_ALPHATAG_MAX_LEN, pContent, pstRecord->ucAlphaTagLength);
            }
            else                                                                                        /*�������������ӵڶ����ֽڿ�ʼ*/
            {
                /*ucs2���룬�����ӳ����ֶο�ʼ*/
                ret = memcpy_s(pstRecord->AlphaTag, SI_PB_ALPHATAG_MAX_LEN, pContent+1, pstRecord->ucAlphaTagLength);
            }

            PAM_PRINT_SECFUN_RESULT(ret);
        }

        if ((pContent[pstPBContent->ucNameLen+1] == 0xFF)
            && (pContent[pstPBContent->ucNameLen] == 0x0))
        {
            pstRecord->NumberType   =  PB_NUMBER_TYPE_NORMAL;
        }
        else
        {
            pstRecord->NumberType   =  pContent[pstPBContent->ucNameLen+1];
        }

        ucExtRecord = ((PB_BDN == pstPBContent->enPBType)?\
                        pContent[pstPBContent->ucRecordLen-2]:\
                        pContent[pstPBContent->ucRecordLen-1]);

        ulExtInfoNum  = pstPBContent->ulExtInfoNum;
        pucExtContent = gastEXTContent[ulExtInfoNum].pExtContent;

        ulResult = SI_PB_CheckExtRecord(ucExtRecord, ulExtInfoNum);

        if (pContent[pstPBContent->ucNameLen] < 2)           /*��ǰ����Ϊ��*/
        {
            pstRecord->NumberLength = 0;
        }
        else if ((ulResult == VOS_OK) && (pucExtContent != VOS_NULL_PTR))
        {
            pucExtContent += (ucExtRecord - 1) * SI_PB_EXT_LEN;
            ucExtNumLen   = (VOS_UINT8)PAM_GetMin(*(pucExtContent + 1), (SI_PB_NUM_LEN/2));

            ret = memcpy_s(aucPhoneNumber, SI_PB_NUM_LEN/2, pContent+(pstPBContent->ucNameLen+2), SI_PB_NUM_LEN/2 );

            PAM_PRINT_SECFUN_RESULT(ret);

            ret = memcpy_s(aucPhoneNumber + (SI_PB_NUM_LEN/2), SI_PB_NUM_LEN/2, pucExtContent + 2, SI_PB_NUM_LEN/2 );

            PAM_PRINT_SECFUN_RESULT(ret);

            SI_PB_BcdToAscii((VOS_UINT8)(ucExtNumLen + (SI_PB_NUM_LEN/2)), aucPhoneNumber,
                              SI_PB_PHONENUM_MAX_LEN, pstRecord->Number, &pstRecord->NumberLength);
        }
        else                                                                                /*������������ݿ�ʼ*/
        {
            ucXdnNumLen = ((pContent[pstPBContent->ucNameLen]-1) > (SI_PB_NUM_LEN/2))?
                           (SI_PB_NUM_LEN/2) : (pContent[pstPBContent->ucNameLen]-1);

            SI_PB_BcdToAscii(ucXdnNumLen, &pContent[pstPBContent->ucNameLen+2],
                            SI_PB_PHONENUM_MAX_LEN, pstRecord->Number, &pstRecord->NumberLength);
        }
    }

    pstRecord->Index = usIndex;

    return ;
}


VOS_UINT32 SI_PB_GetBitFromBuf(VOS_UINT8 *pucDataBuf, VOS_UINT32 dataBufSize, VOS_UINT32 ulBitNo)
{
    VOS_UINT32  ulOffset;
    VOS_UINT8   ucBit;

    if ((VOS_NULL_PTR == pucDataBuf) || (ulBitNo == 0))
    {
        PB_ERROR_LOG("SI_PB_GetBitFromBuf: Input Null Ptr");

        return VOS_FALSE;
    }

    ulOffset = (ulBitNo - 1)/ 8;

    if (ulOffset >= dataBufSize)
    {
        PB_ERROR_LOG("SI_PB_SetBitToBuf: offset is out of size");

        return VOS_FALSE;
    }

    ucBit    = (VOS_UINT8)((ulBitNo - 1)%8);

    return (pucDataBuf[ulOffset]>>ucBit)&0x1;
}


VOS_UINT32 SI_PB_CheckEccValidity(VOS_UINT8 content)
{
    if( 0xFF == content)
    {
        PB_INFO_LOG("SI_PB_CheckEccValidity: The ECC is Empty");

        return VOS_ERR;
    }
    else
    {
        PB_INFO_LOG("SI_PB_CheckEccValidity: The ECC is Not Empty");

        return VOS_OK;
    }
}


VOS_UINT32 SI_PB_CheckContentValidity(SI_PB_CONTENT_STRU *pstPBContent, VOS_UINT8 *pContent, VOS_UINT32 contentSize)
{
    if ((VOS_NULL_PTR == pContent)||(VOS_NULL_PTR == pstPBContent))
    {
        PB_ERROR_LOG("SI_PB_CheckContentValidity: Input NULL PTR");

        return VOS_ERR;
    }

    if (pstPBContent->ucNameLen >= contentSize)
    {
        PB_ERROR_LOG("SI_PB_CheckContentValidity: File Size is incorrect");

        return VOS_ERR;
    }

    if(((pContent[pstPBContent->ucNameLen] == 0)||(pContent[pstPBContent->ucNameLen] == 0xFF))
        &&(pContent[0] == 0xFF))/*��������ͺ����Ƿ�Ϊ��*/
    {
        PB_INFO_LOG("SI_PB_CheckContentValidity: The PhoneBook Content is Empty");

        return VOS_ERR;
    }
    else
    {
        PB_INFO_LOG("SI_PB_CheckContentValidity: The PhoneBook Content is Not Empty");

        return VOS_OK;
    }
}


VOS_UINT32 SI_PB_CheckANRValidity(VOS_UINT8 value1, VOS_UINT8 value2)
{
    if((value1 == 0xFF)||(value2 == 0xFF)||(value2 == 0))/*�������Ƿ�Ϊ��*/
    {
        PB_INFO_LOG("SI_PB_CheckANRValidity: The PhoneBook Content is Empty");

        return VOS_ERR;
    }
    else
    {
        PB_INFO_LOG("SI_PB_CheckANRValidity: The PhoneBook Content is Not Empty");

        return VOS_OK;
    }
}


#if ((OSA_CPU_CCPU == VOS_OSA_CPU)||(defined(DMT)))

VOS_UINT32 SI_PB_CountXDNIndex(VOS_UINT32 ulPBType, VOS_UINT16 usFileId, VOS_UINT8 ucRecordNum, VOS_UINT16 *pusIndex)
{
    VOS_UINT8 i;
    VOS_UINT16 usIndex = 0;

    if (ulPBType != PB_ADN_CONTENT)
    {
        *pusIndex = ucRecordNum;

        return VOS_OK;
    }

    for(i=0; i<SI_PB_ADNMAX; i++)       /*���ݵ�ǰ��ADN �ļ��б�����ѭ��*/
    {
        if(usFileId == gstPBCtrlInfo.astADNInfo[i].usFileID)/*��ѯ��ǰ���ļ�ID �Ƿ����б���*/
        {
            *pusIndex = usIndex + ucRecordNum;      /*���㵱ǰ�ļ�¼�ŵ�������*/

            return VOS_OK;
        }
        else
        {
            usIndex += gstPBCtrlInfo.astADNInfo[i].ucRecordNum;/*��������Ҫ�ۼ�*/
        }
    }

    return VOS_ERR;             /*��ǰ�ļ��������ļ��б���*/
}


VOS_UINT32 SI_PB_GetADNSfi(VOS_UINT8 *pucSFI, VOS_UINT16 usFileId)
{
    VOS_UINT32                          i;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)   /*���ݵ�ǰ��ADN�б�����ѭ��*/
    {
        if(usFileId == gstPBCtrlInfo.astADNInfo[i].usFileID)
        {
            *pucSFI = gstPBCtrlInfo.astADNInfo[i].ucSFI;

            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_CheckADNFileID(VOS_UINT16 usFileID)
{
    VOS_UINT8 j;

    for(j=0; j<SI_PB_ADNMAX; j++)/*�Ƚ��ļ�ID�Ƿ������ADN�б���*/
    {
        if(usFileID == gstPBCtrlInfo.astADNInfo[j].usFileID)
        {
            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetXDNPBType(VOS_UINT32 *pulPBType, VOS_UINT16 usFileId)
{
    VOS_UINT32 ulResult = VOS_OK;

    switch(usFileId)
    {
        case EFFDN:
            *pulPBType = PB_FDN_CONTENT;
            break;
        case EFBDN:
            *pulPBType = PB_BDN_CONTENT;
            break;
        case EFMSISDN:
            *pulPBType = PB_MSISDN_CONTENT;
            break;
        case EFSDN:
            *pulPBType = PB_SDN_CONTENT;
            break;
        default:
            ulResult = SI_PB_CheckADNFileID(usFileId);/*����Ƿ���ADN��FID*/

            if ( VOS_OK == ulResult )
            {
                *pulPBType = PB_ADN_CONTENT;
            }
            else
            {
                ulResult = VOS_ERR;
            }
            break;
    }

    return ulResult;
}


VOS_UINT32 SI_PB_GetANRFid(VOS_UINT32 ulANRFileNum,VOS_UINT16 *pusANRFileId)
{
    VOS_UINT32 i;
    VOS_UINT32 j;
    VOS_UINT32 ulFileCount = 0;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)
    {
        for(j = 0; j < SI_PB_ANRMAX; j++)
        {
            if (SI_PB_FILE_NOT_EXIST != gstPBCtrlInfo.astANRInfo[i][j].usANRFileID)
            {
                ulFileCount++;
            }

            if (ulANRFileNum == ulFileCount)
            {
               *pusANRFileId = gstPBCtrlInfo.astANRInfo[i][j].usANRFileID;

               return VOS_OK;
            }
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetANRSuffix(VOS_UINT8 *pucXSuffix,VOS_UINT8 *pucYSuffix
                                      ,VOS_UINT16 usANRFileId)
{
    VOS_UINT8                           i;
    VOS_UINT8                           j;
    VOS_UINT32                          ulADNFileNum;

    ulADNFileNum = (gstPBCtrlInfo.ulADNFileNum > SI_PB_ADNMAX)? SI_PB_ADNMAX : gstPBCtrlInfo.ulADNFileNum;

    for(i = 0; i < ulADNFileNum; i++)
    {
        for(j = 0; j < SI_PB_ANRMAX; j++)
        {
            if (usANRFileId == gstPBCtrlInfo.astANRInfo[i][j].usANRFileID)
            {
                *pucXSuffix = i;
                *pucYSuffix = j;

                return VOS_OK;
            }
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetIAPFidFromANR(VOS_UINT16 usANRFileId, VOS_UINT16 *pusIAPFileId)
{
    VOS_UINT32 i;
    VOS_UINT32 j;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)
    {
        for(j = 0; j < SI_PB_ANRMAX; j++)
        {
            if(usANRFileId == gstPBCtrlInfo.astANRInfo[i][j].usANRFileID)
            {
                *pusIAPFileId = gstPBCtrlInfo.astIAPInfo[i].usIAPFileID;

                return VOS_OK;
            }
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetIAPFidFromEML(VOS_UINT16 usEMLFileId, VOS_UINT16 *pusIAPFileId)
{
    VOS_UINT32                          i;

    for(i = 0; i < gstPBCtrlInfo.ulEMLFileNum; i++)
    {
        if (usEMLFileId == gstPBCtrlInfo.astEMLInfo[i].usEMLFileID)
        {
            *pusIAPFileId = gstPBCtrlInfo.astIAPInfo[i].usIAPFileID;

            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GheckANRLast(VOS_UINT8 ucXSuffix,VOS_UINT8 ucYSuffix)
{
    if ((ucXSuffix+1) == (VOS_UINT8)gstPBCtrlInfo.ulADNFileNum)
    {
        return VOS_OK;
    }

    if (SI_PB_FILE_NOT_EXIST == gstPBCtrlInfo.astANRInfo[ucXSuffix+1][ucYSuffix].usANRFileID)
    {
        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetEMLFIdFromADN(VOS_UINT16 *pusEMLFileId, VOS_UINT16 usADNId)
{
    VOS_UINT32 i;

    for (i = 0; i < gstPBCtrlInfo.ulEMLFileNum; i++)
    {
        if(usADNId == gstPBCtrlInfo.astADNInfo[i].usFileID)
        {
            *pusEMLFileId = gstPBCtrlInfo.astEMLInfo[i].usEMLFileID;

            return VOS_OK;
        }
    }

    PB_NORMAL1_LOG("SI_PB_GetEMLFIdFromADN: Get ADN %d Email Error", usADNId);

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetANRFidFromADN(VOS_UINT8 ucANROffset, VOS_UINT16 usADNFileId,VOS_UINT16 *pusANRFileId)
{
    VOS_UINT32 i;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)
    {
        if(usADNFileId == gstPBCtrlInfo.astADNInfo[i].usFileID)
        {
            *pusANRFileId = gstPBCtrlInfo.astANRInfo[i][ucANROffset].usANRFileID;

            return VOS_OK;
        }

    }

    return VOS_ERR;
}

VOS_UINT32 SI_PB_GetFreeANRRecordNum(VOS_UINT16 usANRFid,VOS_UINT8 *pucRecordNum, VOS_UINT16 *pusIAPFid)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 j = 0;
    VOS_UINT32 k = 0;
    VOS_UINT32 ulOffset = 0;
    VOS_UINT32 offsetNew;
    VOS_UINT32 ulFlag = VOS_FALSE;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)   /*���ݵ�ǰ��ADN�б�����ѭ��*/
    {
        for(j = 0; j < SI_PB_ANRMAX; j++)
        {
            if(usANRFid == gstPBCtrlInfo.astANRInfo[i][j].usANRFileID)
            {
                *pusIAPFid = gstPBCtrlInfo.astIAPInfo[i].usIAPFileID;

                ulFlag = VOS_TRUE;

                break;
            }

        }

        if(VOS_TRUE == ulFlag)
        {
            break;
        }

        ulOffset += (gstPBCtrlInfo.astANRInfo[i][0].ucRecordNum*gstPBCtrlInfo.astANRInfo[i][0].ucRecordLen);
    }

    if ((i == gstPBCtrlInfo.ulADNFileNum) || (SI_PB_ANRMAX  <= j))
    {
        return VOS_ERR;
    }

    for(k = 0; k < gstPBCtrlInfo.astANRInfo[i][j].ucRecordNum; k++)
    {
        offsetNew = ulOffset + k*gastANRContent[j%SI_PB_ANRMAX].ucRecordLen;

        if(VOS_OK != SI_PB_CheckANRValidity(gastANRContent[j%SI_PB_ANRMAX].pContent[offsetNew],
                                            gastANRContent[j%SI_PB_ANRMAX].pContent[offsetNew + 1]))
        {
            *pucRecordNum = (VOS_UINT8)(k+1);
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

VOS_UINT32 SI_PB_GetFreeEMLRecordNum(VOS_UINT16 ucEMLFid,VOS_UINT8 *pucRecordNum, VOS_UINT16 *pusIAPFid)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 j = 0;
    VOS_UINT32 ulOffset = 0;

    for(i = 0; i < gstPBCtrlInfo.ulEMLFileNum; i++)   /*���ݵ�ǰ��ADN�б�����ѭ��*/
    {
        if(ucEMLFid == gstPBCtrlInfo.astEMLInfo[i].usEMLFileID)
        {
            *pusIAPFid = gstPBCtrlInfo.astIAPInfo[i].usIAPFileID;
            break;
        }

        ulOffset += (gstPBCtrlInfo.astEMLInfo[i].ucRecordNum*gstPBCtrlInfo.astEMLInfo[i].ucRecordLen);
    }

    if(i == gstPBCtrlInfo.ulEMLFileNum)
    {
        return VOS_ERR;
    }

    for(j = 0; j < gstPBCtrlInfo.astEMLInfo[i].ucRecordNum;j++)
    {
         if(0xFF == gstEMLContent.pContent[ulOffset+(j*gstEMLContent.ucRecordLen)])
         {
            *pucRecordNum = (VOS_UINT8)(j+1);
            return VOS_OK;
         }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetEMLRecord(VOS_UINT8 **ppEMLContent, VOS_UINT16 usEMLFileId,VOS_UINT8 ucRecordNum, VOS_UINT32 *pulFreeSize)
{
    VOS_UINT32 i = 0;
    VOS_UINT16 usOffset = 0;
    VOS_UINT32 ulSrcOffset = 0;

    for(i = 0; i < gstPBCtrlInfo.ulEMLFileNum; i++)
    {
        if(usEMLFileId == gstPBCtrlInfo.astEMLInfo[i].usEMLFileID)
        {
            break;
        }
    }

    if((i >= gstPBCtrlInfo.ulEMLFileNum)
        ||(VOS_NULL_PTR == gstEMLContent.pContent))
    {
        return VOS_ERR;
    }

    usOffset = (VOS_UINT16)((i*gstPBCtrlInfo.astEMLInfo[i].ucRecordNum) + ucRecordNum);
    ulSrcOffset = (usOffset-1)*gstEMLContent.ucRecordLen;

    if (gstEMLContent.ulContentSize < ulSrcOffset)
    {
        return VOS_ERR;
    }

    *ppEMLContent = &gstEMLContent.pContent[ulSrcOffset];
    *pulFreeSize  = gstEMLContent.ulContentSize - ulSrcOffset;

    return VOS_OK;

}


VOS_UINT32  SI_PB_FindUnusedExtRecord(SI_EXT_CONTENT_STRU *pstEXTContent,
                                                    VOS_UINT8 *pucRecord,
                                                    VOS_UINT8 ucRecordCount)
{
    VOS_UINT32                          ucCountFreeRec = 1;
    VOS_UINT8                          *pContent = VOS_NULL_PTR;
    VOS_UINT8                           i;

    /* ��ʹ�ü�¼��������¼�����ʱ�����Ѿ�û�п���ʹ�õ�EXT��¼ */
    if( pstEXTContent->usExtTotalNum == pstEXTContent->usExtUsedNum )
    {
        *pucRecord = 0xFF;

         return VOS_ERR;
    }

    pContent = pstEXTContent->pExtContent;

    if(VOS_NULL_PTR == pContent)
    {
        *pucRecord = 0xFF;

         return VOS_ERR;
    }

    if (0xFF < pstEXTContent->usExtTotalNum)
    {
        PB_WARNING1_LOG("SI_PB_FindUnusedExtRecord: pstEXTContent->usExtTotalNum err",  pstEXTContent->usExtTotalNum);

        return VOS_ERR;
    }

    /* ����EXT�ļ��ĵڶ����ֽ����ж��Ƿ�Ϊ�ռ�¼ */
    for( i = 0 ; i < pstEXTContent->usExtTotalNum ; i++ )
    {
        if( 0xFF == pContent[1] )
        {
            if(ucCountFreeRec == ucRecordCount)
            {
                break;
            }
            else
            {
                ucCountFreeRec++;
            }
        }

        pContent += SI_PB_EXT_LEN;
    }

    if( i < pstEXTContent->usExtTotalNum )
    {
        *pucRecord = i + 1;

        return VOS_OK;
    }
    else
    {
        *pucRecord = 0xFF;

        return VOS_ERR;
    }
}


VOS_UINT8 SI_PB_AsciiToBcd_Value(VOS_UINT8 ucFlag, VOS_UINT8 ucPara1, VOS_UINT8 ucPara2)
{
    VOS_UINT8                           ucResult;

    /* ����λ���ϵ��ַ� */
    if(0 == ucFlag)
    {
        ucResult = ucPara1;
    }
    /* ż��λ���ϵ��ַ� */
    else
    {
        ucResult = (VOS_UINT8)((ucPara1 << 4) | ucPara2);
    }

    return ucResult;
}


VOS_VOID SI_PB_AsciiToBcd(VOS_UINT8 *pucAsciiNum, VOS_UINT8 ucAsciiNumLen, VOS_UINT8 *pucBcdNum, VOS_UINT32 bcdBuffSize, VOS_UINT8 *pucBcdNumLen)
{

    VOS_UINT8     i, k;
    VOS_UINT8     *pucTemp = VOS_NULL_PTR;

    /* �����ж� */
    if (((ucAsciiNumLen + 1)/2) > bcdBuffSize)
    {
        *pucBcdNumLen = 0;

        return;
    }

    pucTemp = pucBcdNum;

    for (i=0; i<ucAsciiNumLen; i++)
    {
        k = i % 2;

        if ((*(pucAsciiNum + i) >= 0x30) && (*(pucAsciiNum + i) <= 0x39))/*ת������*/
        {
            *(pucTemp + (i / 2) ) = SI_PB_AsciiToBcd_Value(k, *(pucAsciiNum + i) - 0x30, *(pucTemp + (i / 2)));
        }
        else if('*' == *(pucAsciiNum + i))  /*ת��*�ַ�*/
        {
            *(pucTemp + (i/2)) = SI_PB_AsciiToBcd_Value(k, 0xa, *(pucTemp + (i / 2)));
        }
        else if('#' == *(pucAsciiNum + i))  /*ת��#�ַ�*/
        {
            *(pucTemp + (i/2)) = SI_PB_AsciiToBcd_Value(k, 0xb, *(pucTemp + (i / 2)));
        }
        else if('?' == *(pucAsciiNum + i))  /*ת��?�ַ�*/
        {
            *(pucTemp + (i/2)) = SI_PB_AsciiToBcd_Value(k, 0xd, *(pucTemp + (i / 2)));
        }
        else if (('P' == *(pucAsciiNum + i))
             || ('p' == *(pucAsciiNum + i))
             || (',' == *(pucAsciiNum + i)) ) /*ת��PAUSE�ַ�*/
        {
            *(pucTemp + (i/2)) = SI_PB_AsciiToBcd_Value(k, 0xc, *(pucTemp + (i / 2)));
        }
        else                                        /*����ʶ����ַ�*/
        {
            PB_WARNING_LOG("SI_PB_AsciiToBcd: The Char is Not Suspensory");/*��ӡ����*/
        }
    }

    *pucBcdNumLen = ucAsciiNumLen/2;

    if((ucAsciiNumLen % 2) == 1)    /*�����ֽں�����F*/
    {
        *(pucTemp + (ucAsciiNumLen / 2)) = 0xf0 | *(pucTemp + (ucAsciiNumLen / 2));

        (*pucBcdNumLen)++;
    }

    return;
}


VOS_UINT32 SI_FindMultiTagInBERTLV(VOS_UINT8 *pType1TagAddr,
                                             VOS_UINT32 ulDataLen,
                                            VOS_UINT8   ucTag,
                                            VOS_UINT8 *pucOffset,
                                            VOS_UINT8 ucTagCount)
{
    VOS_UINT32                          i = 0;
    VOS_UINT8                          *pucDataBuf = VOS_NULL_PTR;
    VOS_UINT8                           ucFindCount = 0;

    pucDataBuf = pType1TagAddr + 2;

    while (((i + 1) < ulDataLen) && (ucFindCount < ucTagCount))
    {
        if(pucDataBuf[i] == ucTag)   /*���س����ֽ�ƫ��*/
        {
            pucOffset[ucFindCount] = (VOS_UINT8)(i + 1);

            ucFindCount++;
        }


        i += pucDataBuf[i+1] + 2; /*������һ��Tag*/
    }

    return (0 == ucFindCount)?SI_TAGNOTFOUND:ucFindCount;
}


VOS_UINT32 SI_FindTagInBERTLV(VOS_UINT8 *pucDataBuf, VOS_UINT8 ucTag, VOS_UINT32 ulDataLen)
{
    VOS_UINT32 i = 0;

    while ((i + 1) < ulDataLen)
    {
        if (pucDataBuf[i] == ucTag)   /*���س����ֽ�ƫ��*/
        {
            return (i+1);
        }
        else
        {
            i += pucDataBuf[i+1] + 2; /*������һ��Tag*/
        }
    }

    return SI_TAGNOTFOUND;
}


VOS_UINT32 SI_FindType2FileTagNum(VOS_UINT8 *pucDataBuf, VOS_UINT8 ucTag, VOS_UINT32 ulDataLen)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 ulTagNum = 1;

    while ((i + 1) < ulDataLen)
    {
        if (pucDataBuf[i] == ucTag)   /*����Tag Num*/
        {
            return ulTagNum;
        }
        else
        {
            ulTagNum++;

            i += pucDataBuf[i+1] + 2; /*������һ��Tag*/
        }
    }

    return SI_TAGNOTFOUND;
}



VOS_UINT32 SI_PB_DecodeEFPBR_AdnContent(VOS_UINT8 *pType1TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    validLen = PAM_GetMin(pType1TagAddr[1], spareBuffSize - 2);

    ulOffset = SI_FindTagInBERTLV(&pType1TagAddr[2], EFADNDO_TAG, validLen);/*��ѯ��ǰ��ADN�ļ�ID*/

    if(SI_TAGNOTFOUND == ulOffset)     /*δ�ҵ���Ϣ*/
    {
        PB_ERROR_LOG("SI_PB_DecodeEFPBR_AdnContent: Could Not Find the EFADNDO_TAG Tag");

        return ulOffset;
    }

    /* ȷ����FileId�ֶ� */
    if ((ulOffset + 4) >= spareBuffSize)
    {
        return SI_TAGNOTFOUND;
    }

    gstPBCtrlInfo.astADNInfo[i].usFileID = ((pType1TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType1TagAddr[ulOffset+4];

    /*�����SFI����Ҫ��¼����*/
    if(0x03 == pType1TagAddr[ulOffset+2])
    {
        /* ȷ����SFI�ֶ� */
        if ((ulOffset + 5) < spareBuffSize)
        {
            gstPBCtrlInfo.astADNInfo[i].ucSFI = pType1TagAddr[ulOffset+5];
        }
    }

    gstPBCtrlInfo.ulADNFileNum++;

    return VOS_OK;
}



VOS_VOID SI_PB_DecodeEFPBR_UidContent(VOS_UINT8 *pType1TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    validLen = PAM_GetMin(pType1TagAddr[1], spareBuffSize - 2);

    ulOffset = SI_FindTagInBERTLV(&pType1TagAddr[2], EFUIDDO_TAG, validLen);/*��ѯ��ǰ��UID�ļ�ID*/

    if(SI_TAGNOTFOUND == ulOffset) /*δ�ҵ���Ϣ*/
    {
        PB_WARNING_LOG("SI_PB_DecodeEFPBR_UidContent: Could Not Find the EFUIDDO_TAG Tag");

        gstPBCtrlInfo.astUIDInfo[i].usFileID = 0xFFFF;
    }
    else
    {
        /* ȷ����FileId�ֶ� */
        if ((ulOffset + 4) < spareBuffSize)
        {
            gstPBCtrlInfo.astUIDInfo[i].usFileID = ((pType1TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType1TagAddr[ulOffset+4];
        }
        else
        {
            gstPBCtrlInfo.astUIDInfo[i].usFileID = 0xFFFF;
        }

    }
}


VOS_VOID SI_PB_DecodeEFPBR_PbcContent(VOS_UINT8 * pType1TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    validLen = PAM_GetMin(pType1TagAddr[1], spareBuffSize - 2);

    ulOffset = SI_FindTagInBERTLV(&pType1TagAddr[2], EFPBCDO_TAG, validLen);/*��ѯ��ǰ��PBC�ļ�ID*/

    if(SI_TAGNOTFOUND == ulOffset) /*δ�ҵ���Ϣ*/
    {
        PB_WARNING_LOG("SI_PB_DecodeEFPBR_PbcContent: Could Not Find the EFUIDDO_TAG Tag");

        gstPBCtrlInfo.astPBCInfo[i].usFileID = 0xFFFF;

        gucPBCStatus = VOS_FALSE;
    }
    else if(VOS_TRUE == SI_PB_CheckSupportAP())
    {
        gstPBCtrlInfo.astPBCInfo[i].usFileID = 0xFFFF;

        gucPBCStatus = VOS_FALSE;
    }
    else
    {
        /* ȷ����FileId�ֶ� */
        if ((ulOffset + 4) < spareBuffSize)
        {
            gstPBCtrlInfo.astPBCInfo[i].usFileID = ((pType1TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType1TagAddr[ulOffset+4];
        }
        else
        {
            gstPBCtrlInfo.astPBCInfo[i].usFileID = 0xFFFF;

            gucPBCStatus = VOS_FALSE;
        }
    }
}


VOS_VOID SI_PB_DecodeEFPBR_IapContent(VOS_UINT8 *pType1TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    validLen = PAM_GetMin(pType1TagAddr[1], spareBuffSize - 2);

    ulOffset = SI_FindTagInBERTLV(&pType1TagAddr[2], EFIAPDO_TAG, validLen);/*��ѯ��ǰ��IAP�ļ�ID*/

    if(SI_TAGNOTFOUND == ulOffset) /*δ�ҵ���Ϣ*/
    {
        PB_WARNING_LOG("SI_PB_DecodeEFPBR_IapContent: Could Not Find the EFIAPDO_TAG Tag");
    }
    else
    {
        /* ȷ����FileId�ֶ� */
        if ((ulOffset + 4) < spareBuffSize)
        {
            gstPBCtrlInfo.astIAPInfo[i].usIAPFileID = ((pType1TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType1TagAddr[ulOffset+4];

            gstPBCtrlInfo.ulIAPFileNum++;
        }
    }
}


VOS_UINT32 SI_PB_DecodeType1EFPBR_AnrContent(VOS_UINT8 *pType1TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i, VOS_UINT8 *pucOffset, VOS_UINT8 dataLen)
{
    VOS_UINT32 ulANRCount; /*��¼ÿ����¼�е�ANR����*/
    VOS_UINT32 ulOffset;
    VOS_UINT32 j;
    VOS_UINT32 validLen;

    validLen = PAM_GetMin(pType1TagAddr[1], spareBuffSize - 2);

    ulANRCount = SI_FindMultiTagInBERTLV(pType1TagAddr, validLen, (VOS_UINT8)EFANRDO_TAG,
                                         pucOffset, dataLen);/*��ѯ��ǰ��ANR�ļ�*/

    if(SI_TAGNOTFOUND == ulANRCount) /*δ�ҵ���Ϣ*/
    {
        PB_NORMAL_LOG("SI_PB_DecodeType1EFPBR_AnrContent: Could Not Find the EFANRDO_TAG Tag");
    }
    else
    {
        ulANRCount = (ulANRCount > SI_PB_ANRMAX)?SI_PB_ANRMAX:ulANRCount;

        for(j = 0; j < ulANRCount; j++)
        {
            ulOffset = pucOffset[j];

            /* ȷ����FileId�ֶ� */
            if ((ulOffset + 4) >= spareBuffSize)
            {
                break;
            }

            gstPBCtrlInfo.astANRInfo[i][j].usANRFileID  = ((pType1TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType1TagAddr[ulOffset+4];
            gstPBCtrlInfo.astANRInfo[i][j].enANRType = PB_FILE_TYPE1;
            gstPBCtrlInfo.ulANRFileNum++;
        }
    }

    return ulANRCount;
}


VOS_VOID SI_PB_DecodeType2EFPBR_AnrContent(VOS_UINT8 *pType2TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i, VOS_UINT8 *pucOffset, VOS_UINT8 dataLen)
{
    VOS_UINT32 ulANRCount; /*��¼ÿ����¼�е�ANR����*/
    VOS_UINT32 ulOffset;
    VOS_UINT32 j;
    VOS_UINT32 validLen;

    /* ȷ��������TL+T�ֶ� */
    if (spareBuffSize < 3)
    {
        return;
    }

    validLen = PAM_GetMin(pType2TagAddr[1], spareBuffSize - 2);

    ulANRCount = SI_FindMultiTagInBERTLV(pType2TagAddr, validLen, (VOS_UINT8)EFANRDO_TAG,
                                       pucOffset, dataLen);/*��ѯ��ǰ��ANR�ļ�*/

    if(SI_TAGNOTFOUND == ulANRCount) /*δ�ҵ���Ϣ*/
    {
        PB_NORMAL_LOG("SI_PB_DecodeType2EFPBR_AnrContent: Could Not Find the EFANRDO_TAG Tag");
    }
    else
    {
        ulANRCount = (ulANRCount > SI_PB_ANRMAX)?SI_PB_ANRMAX:ulANRCount;

        for(j = 0; j < ulANRCount; j++)
        {
            ulOffset = pucOffset[j];

            /* ȷ����FileId�ֶ� */
            if ((ulOffset + 4) >= spareBuffSize)
            {
                break;
            }

            gstPBCtrlInfo.astANRInfo[i][j].usANRFileID  = ((pType2TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType2TagAddr[ulOffset+4];
            gstPBCtrlInfo.astANRInfo[i][j].enANRType = PB_FILE_TYPE2;
            gstPBCtrlInfo.ulANRFileNum++;

            gstPBCtrlInfo.astANRInfo[i][j].ulANRTagNum =
                    SI_FindType2FileTagNum(pType2TagAddr+2, EFANRDO_TAG, validLen);
        }
    }

    return;
}


VOS_VOID SI_PB_DecodeEFPBR_ExtContent(VOS_UINT8  *pType3TagAddr, VOS_UINT32 spareBuffSize)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    /* ȷ��������TL+T�ֶ� */
    if (spareBuffSize < 3)
    {
        gstPBCtrlInfo.usEXT1FileID = 0xFFFF;
        gastEXTContent[PB_ADN_CONTENT].usExtFileId  = 0xFFFF;
        gastEXTContent[PB_ADN_CONTENT].usExtFlag    = SI_PB_FILE_NOT_EXIST;

        return;
    }

    validLen = PAM_GetMin(pType3TagAddr[1], spareBuffSize - 2);

    /* ���е�ADN�ļ���ֻ��Ӧ��ͬһ��EXT1�ļ�,ֻҪ��һ��PBR�ļ�¼���ҵ��Ϳ����� */
    ulOffset = SI_FindTagInBERTLV(pType3TagAddr+2, EFEXT1DO_TAG, validLen);/*��ѯ��ǰ��EXT�ļ�ID*/

    /*δ�ҵ���Ϣ*/
    if(SI_TAGNOTFOUND == ulOffset)
    {
        PB_WARNING_LOG("SI_PB_DecodeEFPBR_ExtContent: Could Not Find the EFEXTDO_TAG Tag");

        gstPBCtrlInfo.usEXT1FileID = 0xFFFF;
        gastEXTContent[PB_ADN_CONTENT].usExtFileId  = 0xFFFF;
        gastEXTContent[PB_ADN_CONTENT].usExtFlag    = SI_PB_FILE_NOT_EXIST;
    }
    else if( SI_PB_FILE_EXIST == gastEXTContent[PB_ADN_CONTENT].usExtFlag )
    {
        /*do nothing*/
    }
    else
    {
        /* ȷ����FileId�ֶ� */
        if ((ulOffset + 4) < spareBuffSize)
        {
            gstPBCtrlInfo.usEXT1FileID = ((pType3TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType3TagAddr[ulOffset+4];
            gastEXTContent[PB_ADN_CONTENT].usExtFlag = SI_PB_FILE_EXIST;
        }
        else
        {
            gstPBCtrlInfo.usEXT1FileID = 0xFFFF;
            gastEXTContent[PB_ADN_CONTENT].usExtFileId  = 0xFFFF;
            gastEXTContent[PB_ADN_CONTENT].usExtFlag    = SI_PB_FILE_NOT_EXIST;
        }
    }
}


VOS_VOID SI_PB_DecodeType2EFPBR_EmailContent(VOS_UINT8 *pType2TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    /* ȷ��������TL+T�ֶ� */
    if (spareBuffSize < 3)
    {
        return;
    }

    validLen = PAM_GetMin(pType2TagAddr[1], spareBuffSize - 2);

    ulOffset = SI_FindTagInBERTLV(pType2TagAddr+2, EFEMAILDO_TAG, validLen);/*��ѯ��ǰ��EXT�ļ�ID*/

    /*δ�ҵ���Ϣ*/
    if(SI_TAGNOTFOUND == ulOffset)
    {
        PB_NORMAL_LOG("SI_PB_DecodeType2EFPBR_EmailContent: Could Not Find the EFEMAILDO_TAG Tag");
    }
    else
    {
        /* ȷ����FileId�ֶ� */
        if ((ulOffset + 4) < spareBuffSize)
        {
            gstPBCtrlInfo.astEMLInfo[i].usEMLFileID = ((pType2TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType2TagAddr[ulOffset+4];
            gstPBCtrlInfo.ulEMLFileNum++;

            gstPBCtrlInfo.astEMLInfo[i].enEMLType = PB_FILE_TYPE2;

            gstPBCtrlInfo.astEMLInfo[i].ulEMLTagNum =
                    SI_FindType2FileTagNum(pType2TagAddr+2, EFEMAILDO_TAG, validLen);
        }
    }
}


VOS_UINT32 SI_PB_DecodeType1EFPBR_EmailContent(VOS_UINT8 *pType1TagAddr, VOS_UINT32 spareBuffSize, VOS_UINT32 i)
{
    VOS_UINT32 ulOffset;
    VOS_UINT32 validLen;

    validLen = PAM_GetMin(pType1TagAddr[1], spareBuffSize - 2);

    ulOffset = SI_FindTagInBERTLV(pType1TagAddr+2, EFEMAILDO_TAG, validLen);/*��ѯ��ǰ��EXT�ļ�ID*/

    /*δ�ҵ���Ϣ*/
    if(SI_TAGNOTFOUND == ulOffset)
    {
        PB_NORMAL_LOG("SI_PB_DecodeType1EFPBR_EmailContent: Could Not Find the EFEMAILDO_TAG Tag");

        return SI_TAGNOTFOUND;
    }

    /* ȷ����FileId�ֶ� */
    if ((ulOffset + 4) >= spareBuffSize)
    {
        return SI_TAGNOTFOUND;
    }

    gstPBCtrlInfo.astEMLInfo[i].usEMLFileID = ((pType1TagAddr[ulOffset+3]<<0x08)&0xFF00)+pType1TagAddr[ulOffset+4];
    gstPBCtrlInfo.ulEMLFileNum++;

    gstPBCtrlInfo.astEMLInfo[i].enEMLType = PB_FILE_TYPE1;

    return VOS_OK;
}


VOS_VOID SI_PB_DeleteSameFile(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_UINT32 j;

    for (i = gstPBCtrlInfo.ulADNFileNum -1; i != 0; i--)
    {
        for (j = 0; j < SI_PB_ANRMAX; j++)
        {
            if ((gstPBCtrlInfo.astANRInfo[i][j].usANRFileID == gstPBCtrlInfo.astANRInfo[i-1][j].usANRFileID) &&
                (gstPBCtrlInfo.astANRInfo[i][j].usANRFileID != 0))
            {
                gstPBCtrlInfo.astANRInfo[i][j].usANRFileID = 0;
                gstPBCtrlInfo.astANRInfo[i][j].ucRecordNum = 0;
                gstPBCtrlInfo.astANRInfo[i][j].ucRecordLen = 0;

                gstPBCtrlInfo.ulANRFileNum--;
            }
        }

        if ((gstPBCtrlInfo.astEMLInfo[i].usEMLFileID == gstPBCtrlInfo.astEMLInfo[i-1].usEMLFileID) &&
            (gstPBCtrlInfo.astEMLInfo[i].usEMLFileID != 0))
        {
            gstPBCtrlInfo.astEMLInfo[i].usEMLFileID = 0;
            gstPBCtrlInfo.astEMLInfo[i].ucRecordLen = 0;
            gstPBCtrlInfo.astEMLInfo[i].ucRecordNum = 0;

            gstPBCtrlInfo.ulEMLFileNum--;
        }
    }

    return;
}


VOS_UINT32 SI_PB_DecodeEFPBR(VOS_UINT8 ucRecordNum, VOS_UINT8 ucRecordLen, VOS_UINT8 *pContent)
{
    VOS_UINT32 i;
    VOS_UINT8  *ptemp = pContent;
    VOS_UINT8  *pType1TagAddr = VOS_NULL_PTR;
    VOS_UINT8  *pType2TagAddr = VOS_NULL_PTR;
    VOS_UINT8  *pType3TagAddr = VOS_NULL_PTR;
    VOS_UINT8  aucANROffset[SI_PB_ANR_MAX];
    VOS_UINT32 ulType1EMLFlag = VOS_OK;
    VOS_UINT32 ulType1ANRFlag = VOS_OK;
    VOS_UINT32 ulOffset;

    gstPBCtrlInfo.ulADNFileNum  = 0;

    for(i=0; i<ucRecordNum; i++,(ptemp+= ucRecordLen))
    {
        if((ptemp[0]&0xF0) != 0xA0 ) /*�жϵ�ǰ�ļ�¼�Ƿ���Ч*/
        {
            continue;
        }

        /*����Tpye1�ļ���¼*/
        ulOffset = SI_FindTagInBERTLV(ptemp, PB_FILE_TYPE1, ucRecordLen);

        if(SI_TAGNOTFOUND == ulOffset)
        {
            PB_ERROR_LOG("SI_PB_DecodeEFPBR: Could Not Find the 0xA8 Tag");

            continue;
        }

        pType1TagAddr = ptemp + (ulOffset - 1);

        /* ȷ��������TL+T�ֶ� */
        if ((ucRecordLen - (ulOffset - 1)) < 3)
        {
            continue;
        }

        if(SI_TAGNOTFOUND == SI_PB_DecodeEFPBR_AdnContent(pType1TagAddr, (ucRecordLen - (ulOffset - 1)), i))
        {
            continue;
        }

        ulType1EMLFlag = SI_PB_DecodeType1EFPBR_EmailContent(pType1TagAddr, (ucRecordLen - (ulOffset - 1)),i);

        SI_PB_DecodeEFPBR_UidContent(pType1TagAddr, (ucRecordLen - (ulOffset - 1)),i);

        SI_PB_DecodeEFPBR_PbcContent(pType1TagAddr, (ucRecordLen - (ulOffset - 1)),i);

        SI_PB_DecodeEFPBR_IapContent(pType1TagAddr, (ucRecordLen - (ulOffset - 1)),i);

        ulType1ANRFlag = SI_PB_DecodeType1EFPBR_AnrContent(pType1TagAddr, (ucRecordLen - (ulOffset - 1)), i, aucANROffset, SI_PB_ANR_MAX);

        /*����Tpye2�ļ���¼*/
        ulOffset = SI_FindTagInBERTLV(ptemp, PB_FILE_TYPE2, ucRecordLen);

        /*Type2 �����ļ������ڻ��ѽ�����Type1���� Email��Ϣ*/
        if(SI_TAGNOTFOUND == ulOffset)
        {
            PB_NORMAL_LOG("SI_PB_DecodeEFPBR: Could Not Find the Type2 Tag");
        }
        else
        {
            pType2TagAddr = (ptemp + ulOffset) - 1;

            /* ȷ��������TL+T�ֶ� */
            if(SI_TAGNOTFOUND == ulType1EMLFlag)
            {
                SI_PB_DecodeType2EFPBR_EmailContent(pType2TagAddr, (ucRecordLen - (ulOffset - 1)), i);
            }

            if(SI_TAGNOTFOUND == ulType1ANRFlag)
            {
                SI_PB_DecodeType2EFPBR_AnrContent(pType2TagAddr, (ucRecordLen - (ulOffset - 1)), i, aucANROffset, SI_PB_ANR_MAX);
            }
        }

        /*����Tpye3�ļ���¼*/
        ulOffset = SI_FindTagInBERTLV(ptemp, PB_FILE_TYPE3, ucRecordLen);

        if(SI_TAGNOTFOUND == ulOffset)
        {
            PB_ERROR_LOG("SI_PB_DecodeEFPBR: Could Not Find the AA Tag");

            gstPBCtrlInfo.usEXT1FileID = 0xFFFF;
            gastEXTContent[PB_ADN_CONTENT].usExtFileId  = 0xFFFF;
            gastEXTContent[PB_ADN_CONTENT].usExtFlag    = SI_PB_FILE_NOT_EXIST;
        }
        else
        {
            pType3TagAddr = (ptemp + ulOffset) - 1;

            SI_PB_DecodeEFPBR_ExtContent(pType3TagAddr, (ucRecordLen - (ulOffset - 1)));
        }
    }

    if ( 0 == gstPBCtrlInfo.ulADNFileNum )
    {
        return VOS_ERR;
    }

    SI_PB_DeleteSameFile();

    /* ����USIM�����绰���ļ��ĳ�ʼ�����Ȳ���SEARCH�ļ�ʵ�֣�
       ��ʼ�������г���SEARCH���ɹ�ʱתΪ������ȡ��ʼ�� */
    gstPBInitState.enPBSearchState = PB_SEARCH_ENABLE;

    return VOS_OK;
}


VOS_VOID SI_PB_DecodeEFPBCRecord(VOS_UINT8 content, VOS_UINT8 ucRecordNum)
{
    if( 1 == (content&0x1) )
    {
        /*�ӵ�һ��Ԫ�ؿ�ʼ��¼*/
        gstPBInitState.stPBCUpdate.aucRecord[++gstPBInitState.stPBCUpdate.ucEntryChangeNum] = ucRecordNum;
    }

    return;
}


VOS_VOID SI_PB_DecodeEFUIDRecord(VOS_UINT8 value1, VOS_UINT8 value2)
{
    VOS_UINT16 usEFUIDValue = 0;

    usEFUIDValue = value1;

    usEFUIDValue <<= 8;

    usEFUIDValue |= value2;

    if(usEFUIDValue == 0xFFFF)
    {
        return;
    }

    if(gstPBCtrlInfo.usUIDMaxValue < usEFUIDValue)
    {
        gstPBCtrlInfo.usUIDMaxValue = usEFUIDValue;
    }

    return;
}


VOS_UINT32 SI_PB_JudgeADNFid(VOS_UINT16 usFileId)
{
    VOS_UINT32 i;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)
    {
        if(usFileId == gstPBCtrlInfo.astADNInfo[i].usFileID)
        {
            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_JudgeEXTFid(VOS_UINT16 usFileId, VOS_UINT16 usOffset)
{

    if(usFileId == gastEXTContent[usOffset].usExtFileId)
    {
        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_JudgeIAPFid(VOS_UINT16 usFileId)
{
    VOS_UINT32 i;

    for(i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)
    {
        if(usFileId == gstPBCtrlInfo.astIAPInfo[i].usIAPFileID)
        {
            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_GetEXTContentFromReq(VOS_UINT8 ucRecordNum, VOS_UINT8 **ppucContent)
{
    VOS_UINT32 i;

    if(ucRecordNum == gstPBReqUnit.usExtIndex)
    {
        *ppucContent = gstPBReqUnit.aucEXTContent;

        gstPBReqUnit.usExtIndex = 0;/*��Ϊ��Чֵ*/

        return VOS_OK;
    }

    for(i = 0; i < SI_PB_ANR_MAX; i++)
    {
        if(ucRecordNum == gstPBReqUnit.stSPBReq.usANRExtIndex[i])
        {
            *ppucContent = gstPBReqUnit.stSPBReq.aucANRExtContent[i];

            gstPBReqUnit.stSPBReq.usANRExtIndex[i] = 0;/*��Ϊ��Чֵ*/
            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_VOID SI_PB_SetBitToBuf(VOS_UINT8 *pucDataBuf, VOS_UINT32 dataBufSize, VOS_UINT32 ulBitNo, VOS_UINT32 ulValue)
{
    VOS_UINT32  ulOffset;
    VOS_UINT8   ucBit;

    if ((VOS_NULL_PTR == pucDataBuf) || (ulBitNo == 0))
    {
        PB_ERROR_LOG("SI_PB_SetBitToBuf: Input Null Ptr");

        return;
    }

    ulOffset = (ulBitNo - 1)/ 8;

    if (ulOffset >= dataBufSize)
    {
        PB_ERROR_LOG("SI_PB_SetBitToBuf: offset is out of size");
        return;
    }

    ucBit    = (VOS_UINT8)((ulBitNo - 1)%8);

    if(0 != ulValue)
    {
        /*lint -e701 */
        pucDataBuf[ulOffset] |= (VOS_UINT8)(0x1<<ucBit);
        /*lint +e701 */
    }
    else
    {
        /*lint -e701 */
        pucDataBuf[ulOffset] &= (~(VOS_UINT8)(0x1<<ucBit));
        /*lint +e701 */
    }

    return;
}


VOS_UINT32 SI_PB_CheckFdnNumLen(
    VOS_UINT32                          ulFdnNumLen,
    VOS_UINT32                          ulCmpNumLen)
{
    USIMM_FEATURE_ATT_CFG_STRU          stAttCfg = {0};

    if (VOS_OK != mdrv_nv_read(en_NV_Item_Usim_Att_flg,
                               (VOS_VOID*)&stAttCfg,
                               sizeof(stAttCfg)))
    {
        stAttCfg.ulAtt_flg = VOS_FALSE;
    }

    /*�����֧��FDN������ȫƥ�䣬 �洢��FDN���볤�ȴ���ҵ����룬����ʧ�� */
    if(VOS_FALSE == stAttCfg.ulAtt_flg)
    {
        if (ulFdnNumLen > ulCmpNumLen)
        {
           return VOS_ERR;
        }
    }
    /*ATT FDN������ȫƥ�䣬�洢��FDN���볤�Ⱥ�ҵ����볤�Ȳ�һ�£���ֱ�ӷ���*/
    else
    {
        if (ulFdnNumLen != ulCmpNumLen)
        {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 SI_PB_BcdCompare(VOS_UINT8 *pucBcdNumSrc, VOS_UINT8 ucBcdSrcLen, VOS_UINT8 *pucFdnBcdNum, VOS_UINT8 ucFdnBcdLen)
{
    VOS_UINT8                           ucDstIndex;
    VOS_UINT8                           ucFdnNumLen;
    VOS_UINT8                           ucCmpNumLen;
    VOS_UINT8                           aucFdnNum[SI_PB_NUM_LEN*2]  = {0};
    VOS_UINT8                           aucCmpNum[SI_PB_NUM_LEN*2]  = {0};

    (VOS_VOID)memset_s(aucFdnNum, sizeof(aucFdnNum), (VOS_CHAR)0xFF, sizeof(aucFdnNum));

    (VOS_VOID)memset_s(aucCmpNum, sizeof(aucCmpNum), (VOS_CHAR)0xFF, sizeof(aucCmpNum));

    /* �绰���֧��40�����룬BCD��ĳ����20���ֽ� */
    if ((ucBcdSrcLen > SI_PB_NUM_LEN) || (ucFdnBcdLen > SI_PB_NUM_LEN))
    {
        return VOS_ERR;
    }

    /* ������ĺ���ת����ASCII����ٽ��бȽ� */
    SI_PB_BcdToAscii(ucBcdSrcLen, pucBcdNumSrc, sizeof(aucCmpNum), aucCmpNum, &ucCmpNumLen);

    /* �������FDN����ת����ASCII����ٽ��бȽ� */
    SI_PB_BcdToAscii(ucFdnBcdLen, pucFdnBcdNum, sizeof(aucFdnNum), aucFdnNum, &ucFdnNumLen);

    if (VOS_OK != SI_PB_CheckFdnNumLen(ucFdnNumLen, ucCmpNumLen))
    {
        return VOS_ERR;
    }

    /* ѭ���Ƚ����������ASCII�� */
    for (ucDstIndex = 0; ucDstIndex < ucFdnNumLen; ucDstIndex++)
    {
        /* ����'?'�����Ƚϣ��Ƚ���һ������.������������������ */
        if ((aucFdnNum[ucDstIndex] == aucCmpNum[ucDstIndex])
            || ('?' == aucFdnNum[ucDstIndex]))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    /* ����Ƚϵ���β�ж��ɹ� */
    if (ucFdnNumLen == ucDstIndex)
    {
        return VOS_OK;
    }

    return VOS_ERR;
}


VOS_UINT32 SI_PB_CheckEXT2(VOS_UINT8 ucExtRecord,
                                   VOS_UINT32 ulExtInfoNum,
                                   VOS_UINT8 *pucDstNum,
                                   VOS_UINT8 ucContentSize,
                                   VOS_UINT8 *pucExNumLen)
{
    VOS_UINT8                          *pExTemp = VOS_NULL_PTR;
    VOS_UINT8                           ucExNumLen;
    errno_t                             ret;

    pExTemp = gastEXTContent[ulExtInfoNum].pExtContent;

    if (VOS_NULL_PTR == pExTemp)
    {
        PB_ERROR_LOG("SI_PB_CheckFdn: gastEXTContent is null.\r\n");
        return VOS_ERR;
    }

    if ((ucExtRecord > gastEXTContent[ulExtInfoNum].usExtTotalNum)
       || (0 == ucExtRecord))
    {
        PB_ERROR1_LOG("SI_PB_CheckFdn: ucExtRecord: %d is wrong .\r\n", ucExtRecord);
        return VOS_ERR;
    }

    pExTemp += (ucExtRecord - 1) * SI_PB_EXT_LEN;

    /* �ж�EX���볤�� */
    if ((0xFF == *(pExTemp+1)) || (0x0 == *(pExTemp+1)))
    {
        PB_ERROR_LOG("SI_PB_CheckFdn: EXT NUM Length is error.\r\n");
        return VOS_ERR;
    }

    ucExNumLen = (*(pExTemp+1) > (SI_PB_NUM_LEN/2))?
                    (SI_PB_NUM_LEN/2) : *(pExTemp + 1);

    ret = memcpy_s(pucDstNum, ucContentSize, (pExTemp+2), ucExNumLen);

    PAM_PRINT_SECFUN_RESULT(ret);

    *pucExNumLen = ucExNumLen;

    return VOS_OK;
}


VOS_UINT32 SI_PB_CheckFdn(VOS_UINT8 *pucNum, VOS_UINT32 ulNumLength)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucPBOffset=0;
    VOS_UINT16                          i;
    VOS_UINT8                          *pTemp = VOS_NULL_PTR;
    VOS_UINT8                          *pucFdnContent = VOS_NULL_PTR;
    VOS_UINT8                           aucNumber[SI_PB_NUM_LEN] = {0};
    VOS_UINT16                          usTotalNum;
    VOS_UINT8                           ucNumLen;
    VOS_UINT8                           ucExNumLen;
    VOS_UINT32                          ulExtInfoNum;
    VOS_UINT8                           ucExtRecord;
    errno_t                             ret;

    (VOS_VOID)memset_s(aucNumber, SI_PB_NUM_LEN, (VOS_UINT8)0xFF, SI_PB_NUM_LEN);
    ucNumLen = 0;

    /*��ǰ�绰�������ڻ��߳�ʼ��δ���*/
    ulResult = SI_PB_LocateRecord(PB_FDN, 1, 1, &ucPBOffset);
    if(VOS_OK != ulResult)
    {
        PB_ERROR_LOG("SI_PB_CheckFdn: SI_PB_LocateRecord Return Failed.\r\n");
        return VOS_ERR;
    }

    usTotalNum      = gastPBContent[ucPBOffset].usTotalNum;
    pucFdnContent   = gastPBContent[ucPBOffset].pContent;

    if (VOS_NULL_PTR == pucFdnContent)
    {
        PB_ERROR_LOG("SI_PB_CheckFdn: gastPBContent is null.\r\n");
        return VOS_ERR;
    }

    ulExtInfoNum = gastPBContent[ucPBOffset].ulExtInfoNum;

    /*�ڵ绰����¼��ѭ�������ַ���ƥ�䣬һ���ɹ�����VOS_OK*/
    for(i=0; i<usTotalNum; i++)   /*�������ݽṹ��󳤶�ѭ��*/
    {
        pTemp = pucFdnContent;
        pTemp += gastPBContent[ucPBOffset].ucNameLen;

        /*�����볤���Ƿ�Ϸ�*/
        if ((0xFF == *pTemp)||(*pTemp < 2))
        {
            pucFdnContent += gastPBContent[ucPBOffset].ucRecordLen;

            continue;
        }

        ucNumLen = *pTemp - 1;

        ucNumLen = (ucNumLen > (SI_PB_NUM_LEN / 2))?(SI_PB_NUM_LEN / 2):ucNumLen;

        (VOS_VOID)memset_s(aucNumber, SI_PB_NUM_LEN, (VOS_UINT8)0xFF, SI_PB_NUM_LEN);

        ret = memcpy_s(aucNumber, SI_PB_NUM_LEN, pTemp+2, ucNumLen);

        PAM_PRINT_SECFUN_RESULT(ret);

        ucExtRecord = pucFdnContent[gastPBContent[ucPBOffset].ucRecordLen-1];

        /* ����EX�ļ� */
        if (0xFF != ucExtRecord)
        {
            if (SI_PB_CheckEXT2(ucExtRecord, ulExtInfoNum, &(aucNumber[ucNumLen]), SI_PB_NUM_LEN - ucNumLen, &ucExNumLen) == VOS_OK)
            {
                ucNumLen += ucExNumLen;
            }
        }

        /*�ȶ��ַ�*/
        if (VOS_OK == SI_PB_BcdCompare(pucNum, (VOS_UINT8)ulNumLength, aucNumber, ucNumLen))
        {
            PB_ERROR_LOG("SI_PB_CheckFdn: SI_PB_BcdCompare ok");

            return VOS_OK;
        }

        pucFdnContent   += gastPBContent[ucPBOffset].ucRecordLen;
    }

    return VOS_ERR;
}
/* Added by f62575 for C50_IPC Project, 2012/02/23, end   */


/*lint -save -e958 */
VOS_UINT32 SI_PB_EcallNumberErrProc(SI_PB_STORATE_TYPE ulStorage, VOS_UINT8 ucListLen, VOS_UINT8 *pucList, VOS_UINT8 *pucPBOffset)
{
    VOS_UINT8                           i;
    VOS_UINT8                           ucPBOffset;

    if (VOS_OK != SI_PB_FindPBOffset(ulStorage, pucPBOffset))
    {
        PB_WARNING_LOG("SI_PB_EcallNumberErrProc: Locate PhoneBook Error");

        return TAF_ERR_ERROR;
    }

    ucPBOffset = *pucPBOffset;

    /* ��ǰ�ĵ绰��û�г�ʼ����� */
    if (PB_INITIALISED != gastPBContent[ucPBOffset].enInitialState)
    {
        PB_ERROR_LOG("SI_PB_EcallNumberErrProc:The PhoneBook is Not Initializtion");

        return TAF_ERR_SIM_BUSY;
    }

    /* ��ǰ�ĵ绰��û������ */
    if (VOS_NULL_PTR == gastPBContent[ucPBOffset].pContent)
    {
        PB_ERROR_LOG("SI_PB_EcallNumberErrProc:The PhoneBook is Not Initializtion");

        return TAF_ERR_PB_NOT_INIT;
    }

    /* �����index���ܳ�������¼����Ϊ0 */
    for (i = 0; i < ucListLen; i++)
    {
        if ((pucList[i] > gastPBContent[ucPBOffset].usTotalNum)
            || (VOS_NULL == pucList[i]))
        {
            PB_ERROR_LOG("SI_PB_EcallNumberErrProc:Wrong Index");

            return TAF_ERR_PB_WRONG_INDEX;
        }
    }

    return TAF_ERR_NO_ERROR;
}
/*lint -restore */

/*lint -save -e958 */
VOS_VOID SI_PB_GetEcallNumber(SI_PB_CONTENT_STRU *pstXdnContent, SI_PB_ECALL_NUM_STRU *pstEcallNum, VOS_UINT8 ucListLen, VOS_UINT8 *pucList)
{
    VOS_UINT8                          *pucContent = VOS_NULL_PTR;
    VOS_UINT8                          *pucTmp = VOS_NULL_PTR;
    VOS_UINT8                          *pucExtContent = VOS_NULL_PTR;
    VOS_UINT32                          ulExtInfoNum;
    VOS_UINT8                           ucRecordLen;
    VOS_UINT8                           ucExNumLen;
    VOS_UINT8                           ucExtRecord;
    VOS_UINT8                           i;
    errno_t                             ret;


    pucContent = pstXdnContent->pContent;
    ucRecordLen= pstXdnContent->ucRecordLen;

    for (i = 0; i < ucListLen; i++)
    {
        pstEcallNum->ucIndex  = pucList[i];

        (VOS_VOID)memset_s(pstEcallNum->aucCallNumber, SI_PB_NUM_LEN, (VOS_UINT8)0xFF, SI_PB_NUM_LEN);
/*lint -save -e961 */
        pucTmp = pucContent + (pstXdnContent->ucRecordLen) * (pucList[i] - 1);
/*lint -restore */
        ucExtRecord  = pucTmp[ucRecordLen-1];
        ulExtInfoNum = pstXdnContent->ulExtInfoNum;
        pucExtContent = gastEXTContent[ulExtInfoNum].pExtContent;

        ucExtRecord = ((VOS_NULL_PTR == pucExtContent) ? 0xFF : (ucExtRecord));

        if ((pucTmp[pstXdnContent->ucNameLen] < 2) || (0xFF == pucTmp[pstXdnContent->ucNameLen]))
        {
            pstEcallNum->ucLength = 0;
        }
        else if((0xFF != ucExtRecord)&&(0 != ucExtRecord)&&(VOS_NULL_PTR != pucExtContent)
                &&(ucExtRecord <= gastEXTContent[ulExtInfoNum].usExtTotalNum))
        {
            pucExtContent += (ucExtRecord - 1) * SI_PB_EXT_LEN;
            ucExNumLen = (*(pucExtContent + 1) > (SI_PB_NUM_LEN/2))? (SI_PB_NUM_LEN/2) : *(pucExtContent + 1);

            ret = memcpy_s(pstEcallNum->aucCallNumber, (SI_PB_NUM_LEN/2), pucTmp + pstXdnContent->ucNameLen + 2, (SI_PB_NUM_LEN/2));

            PAM_PRINT_SECFUN_RESULT(ret);

            ret = memcpy_s(pstEcallNum->aucCallNumber + (SI_PB_NUM_LEN/2), (SI_PB_NUM_LEN/2), pucExtContent + 2, (SI_PB_NUM_LEN/2));

            PAM_PRINT_SECFUN_RESULT(ret);

            pstEcallNum->ucTon    = pucTmp[pstXdnContent->ucNameLen + 1];
            pstEcallNum->ucLength = (SI_PB_NUM_LEN/2) + ucExNumLen;
        }
        else
        {
            ret = memcpy_s(pstEcallNum->aucCallNumber, (SI_PB_NUM_LEN/2), pucTmp + pstXdnContent->ucNameLen + 2, (SI_PB_NUM_LEN/2));

            PAM_PRINT_SECFUN_RESULT(ret);

            pstEcallNum->ucTon    = pucTmp[pstXdnContent->ucNameLen + 1];
            pstEcallNum->ucLength = ((pucTmp[pstXdnContent->ucNameLen] - 1) > (SI_PB_NUM_LEN/2))?
                                    (SI_PB_NUM_LEN/2) : (pucTmp[pstXdnContent->ucNameLen] - 1);

        }

        pstEcallNum++;
    }

    return;
}
/*lint -restore */


VOS_UINT32 SI_PB_IsExsitExtFile(VOS_UINT8 extIndexValue, VOS_UINT32 extInfoNum)
{
    /* ����Ƿ���EXT�ļ�Ҫ��ȡ��XDN�ļ�����չ��¼�ļ�¼�Ų���Ϊ0��0xFF */
    if ((extIndexValue != 0xFF) &&
        (extIndexValue != 0) &&
        (gastEXTContent[extInfoNum].usExtFlag != SI_PB_FILE_NOT_EXIST))
    {
        return VOS_OK;
    }

    return VOS_ERR;
}

#endif

#if ((OSA_CPU_ACPU == VOS_OSA_CPU)||(defined(DMT)))

VOS_VOID SI_PB_TransANRFromate(VOS_UINT8 ucANROffset,VOS_UINT8 *pANRContent, SI_PB_RECORD_STRU *pstRecord)
{
    VOS_UINT32 ulResult;
    VOS_UINT8  ucExtRecord;
    VOS_UINT8  aucPhoneNumber[SI_PB_NUM_LEN] = {0};
    VOS_UINT8  *pucExtContent = VOS_NULL_PTR;
    VOS_UINT8  ucExtNumLen;
    VOS_UINT8  ucAnrNumLen;
    errno_t    ret;

    ulResult = SI_PB_CheckANRValidity(pANRContent[0], pANRContent[1]);/*��鵱ǰ�������Ƿ���Ч*/

    if(ulResult != VOS_OK)
    {
        /*ȫ��Ϊ0*/
        pstRecord->AdditionNumber[ucANROffset].NumberLength = 0;
    }
    else
    {
        pstRecord->ValidFlag = SI_PB_CONTENT_VALID;/*��ǵ�ǰ��������Ч*/

        pstRecord->AdditionNumber[ucANROffset].NumberType = pANRContent[2];

        pucExtContent = gastEXTContent[PB_ADN_CONTENT].pExtContent;

        ucExtRecord = ((VOS_NULL_PTR == pucExtContent)?0xFF:pANRContent[14]);

        if((0xFF != ucExtRecord)&&(VOS_NULL_PTR != pucExtContent)
           &&(ucExtRecord <= gastEXTContent[PB_ADN_CONTENT].usExtTotalNum))
        {
            pucExtContent += (ucExtRecord - 1) * SI_PB_EXT_LEN;
            ucExtNumLen   = (*(pucExtContent + 1) > (SI_PB_NUM_LEN/2))?
                            (SI_PB_NUM_LEN/2) : *(pucExtContent + 1);

            ret = memcpy_s(aucPhoneNumber, SI_PB_NUM_LEN, pANRContent+3, SI_PB_NUM_LEN/2);

            PAM_PRINT_SECFUN_RESULT(ret);

            ret = memcpy_s(aucPhoneNumber + (SI_PB_NUM_LEN/2), SI_PB_NUM_LEN/2, pucExtContent + 2, SI_PB_NUM_LEN/2 );

            PAM_PRINT_SECFUN_RESULT(ret);

            SI_PB_BcdToAscii( (VOS_UINT8)(ucExtNumLen + (SI_PB_NUM_LEN/2)), aucPhoneNumber,
                               sizeof(pstRecord->AdditionNumber[ucANROffset].Number), pstRecord->AdditionNumber[ucANROffset].Number, &pstRecord->AdditionNumber[ucANROffset].NumberLength);
        }
        else                                                                                /*������������ݿ�ʼ*/
        {
            ucAnrNumLen = ((pANRContent[1]-1) > (SI_PB_NUM_LEN/2))?
                          (SI_PB_NUM_LEN/2) : (pANRContent[1]-1);

            SI_PB_BcdToAscii(ucAnrNumLen, &pANRContent[3],
                            sizeof(pstRecord->AdditionNumber[ucANROffset].Number),
                            pstRecord->AdditionNumber[ucANROffset].Number,
                            &pstRecord->AdditionNumber[ucANROffset].NumberLength);
        }
    }

    return;
}


VOS_VOID SI_PB_TransEMLFromate(VOS_UINT8 ucEmailMaxLen, VOS_UINT8 *pEMLContent, SI_PB_RECORD_STRU *pstRecord)
{
    VOS_UINT32 i;
    VOS_UINT8  ucEmailLen = 0;

    for(i = 0; i < ucEmailMaxLen; i++)
    {
        if(0xFF == pEMLContent[i])
        {
            break;
        }

        ucEmailLen++;
    }

    if(0 == ucEmailLen)
    {
        pstRecord->Email.EmailLen = 0;
    }
    else
    {
        pstRecord->Email.EmailLen = ucEmailLen;

        pstRecord->ValidFlag = SI_PB_CONTENT_VALID;

        if (memcpy_s(pstRecord->Email.Email, SI_PB_EMAIL_MAX_LEN, pEMLContent, ucEmailLen) != EOK)
        {
            pstRecord->Email.EmailLen = 0;
            pstRecord->ValidFlag = SI_PB_CONTENT_INVALID;

            PB_WARNING_LOG("SI_PB_TransEMLFromate: memcpy_s fail");
        }
    }

    return;
}


VOS_UINT32 SI_PB_GetFileCntFromIndex(VOS_UINT16 ucIndex, VOS_UINT8 *pucFileCnt)
{
    VOS_UINT8                           i;
    VOS_UINT16                          usSum;

    usSum = 0;

    if(SI_PB_ADNMAX < gstPBCtrlInfo.ulADNFileNum)
    {
        PB_WARNING1_LOG("SI_PB_SearchResultProc: gstPBCtrlInfo.ulADNFileNum err", gstPBCtrlInfo.ulADNFileNum);
        return VOS_ERR;
    }

    for (i = 0; i < gstPBCtrlInfo.ulADNFileNum; i++)
    {
        if ((ucIndex <= (usSum + gstPBCtrlInfo.astADNInfo[i].ucRecordNum))
            && (ucIndex > usSum))
        {
            break;
        }
        else
        {
            usSum += gstPBCtrlInfo.astADNInfo[i].ucRecordNum;
        }
    }

    if (i >= gstPBCtrlInfo.ulADNFileNum)
    {
        *pucFileCnt = 0;
        return VOS_ERR;
    }
    else
    {
        *pucFileCnt = i + 1;
        return VOS_OK;
    }
}

#endif

#endif /* (FEATURE_ON == FEATURE_PHONE_SC) */




