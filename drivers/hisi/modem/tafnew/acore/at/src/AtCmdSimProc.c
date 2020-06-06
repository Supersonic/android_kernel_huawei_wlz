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
#include "AtCmdSimProc.h"
#include "AtEventReport.h"
#include "nv_stru_sys.h"
#include "nv_stru_pam.h"
#include "securec.h"


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_SIM_PROC_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  3 �ⲿ�ӿ�����
*****************************************************************************/
extern VOS_UINT32 AT_Hex2AsciiStrLowHalfFirst(
    VOS_UINT32                          ulMaxLength,
    VOS_INT8                            *pcHeadaddr,
    VOS_UINT8                           *pucDst,
    VOS_UINT8                           *pucSrc,
    VOS_UINT16                          usSrcLen
);

/*****************************************************************************
  4 ����ʵ��
*****************************************************************************/


VOS_UINT32 At_IsSimSlotAllowed(
    VOS_UINT32                          ulModem0Slot,
    VOS_UINT32                          ulModem1Slot,
    VOS_UINT32                          ulModem2Slot
)
{
    /* modem0��modem1���ó���ͬ��Ӳ��,���ش��� */
    if ( (ulModem0Slot == ulModem1Slot)
      && (ulModem0Slot != SI_PIH_CARD_SLOT_2) )
    {
        return VOS_FALSE;
    }

    /* modem0��modem2���ó���ͬ��Ӳ��,���ش��� */
    if ( (ulModem0Slot == ulModem2Slot)
      && (ulModem0Slot != SI_PIH_CARD_SLOT_2) )
    {
        return VOS_FALSE;
    }

    /* modem1��modem2���ó���ͬ��Ӳ��,���ش��� */
    if ( (ulModem1Slot == ulModem2Slot)
      && (ulModem1Slot != SI_PIH_CARD_SLOT_2) )
    {
        return VOS_FALSE;
    }

    /* ����modem����Ӧ�տ���,���ش��� */
    if ((ulModem0Slot == SI_PIH_CARD_SLOT_2)
     && (ulModem1Slot == SI_PIH_CARD_SLOT_2)
     && (ulModem2Slot == SI_PIH_CARD_SLOT_2))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32 At_SetSIMSlotPara(VOS_UINT8 ucIndex)
{
#if (MULTI_MODEM_NUMBER != 1)
    TAF_NV_SCI_CFG_STRU                 stSCICfg;
#endif
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 3)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    if ( (gastAtParaList[0].usParaLen == 0)
       ||(gastAtParaList[1].usParaLen == 0) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��modem��֧���л����� */
#if (MULTI_MODEM_NUMBER == 1)
    return AT_CME_OPERATION_NOT_ALLOWED;
#else

    /* ������̬��3����������Ϊ�գ�������̬Ĭ��Ϊ����2 */
#if (MULTI_MODEM_NUMBER == 3)
    if (gastAtParaList[2].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#else
    gastAtParaList[2].ulParaValue = SI_PIH_CARD_SLOT_2;
#endif

    /* ������� */
    if (At_IsSimSlotAllowed(gastAtParaList[0].ulParaValue,
                                         gastAtParaList[1].ulParaValue,
                                         gastAtParaList[2].ulParaValue) == VOS_FALSE )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��NV�ж�ȡ��ǰSIM����SCI���� */
    memset_s(&stSCICfg, sizeof(stSCICfg), 0x00, sizeof(stSCICfg));

    if (TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_SCI_DSDA_CFG,
                                   &stSCICfg,
                                   sizeof(stSCICfg)) != NV_OK)
    {
        AT_ERR_LOG("At_SetSIMSlotPara: en_NV_Item_SCI_DSDA_CFG read fail!");
        return AT_ERROR;
    }

    /*
         �����û����õ�ֵ�޸�card0λ��card1λ��ֵ����NV���У��������Ӧ��bitλ��ȡֵ��������:
         card0: bit[8-10]������0ʹ�õ�SCI�ӿ�
             0��ʹ��SCI0��Ĭ��ֵ��
             1��ʹ��SCI1
             2��ʹ��SCI2
             ����ֵ����Ч
         card1:bit[11-13]����1��ʹ�õ�SCI�ӿ�
             0��ʹ��SCI0
             1��ʹ��SCI1��Ĭ��ֵ��
             2��ʹ��SCI2
             ����ֵ����Ч
         card2:bit[14-16]����2��ʹ�õ�SCI�ӿ�
             0��ʹ��SCI0
             1��ʹ��SCI1
             2��ʹ��SCI2��Ĭ��ֵ��
             ����ֵ����Ч
     */
    stSCICfg.bitCard0   = gastAtParaList[0].ulParaValue;
    stSCICfg.bitCard1   = gastAtParaList[1].ulParaValue;

    /* ���˫����̬���ӱ�����bitCard2ʹ��NVĬ��ֵ������������� */
#if (MULTI_MODEM_NUMBER == 3)
    stSCICfg.bitCard2   = gastAtParaList[2].ulParaValue;
    stSCICfg.bitCardNum = 3;
#else
    stSCICfg.bitCardNum = 2;
#endif

    stSCICfg.bitReserved0 = 0;
    stSCICfg.bitReserved1 = 0;


    /* �����õ�SCIֵ���浽NV�� */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0,
                                    en_NV_Item_SCI_DSDA_CFG,
                                    &stSCICfg,
                                    sizeof(stSCICfg)) != NV_OK)
    {
        AT_ERR_LOG("At_SetSIMSlotPara: en_NV_Item_SCI_DSDA_CFG write failed");
        return AT_ERROR;
    }

    return AT_OK;
 #endif
}


VOS_UINT32 At_QrySIMSlotPara(VOS_UINT8 ucIndex)
{
    TAF_NV_SCI_CFG_STRU                 stSCICfg;
    VOS_UINT16                          usLength;

    /*��NV�ж�ȡ��ǰSIM����SCI����*/
    memset_s(&stSCICfg, sizeof(stSCICfg), 0x00, sizeof(stSCICfg));

    if (TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_SCI_DSDA_CFG,
                                   &stSCICfg,
                                   sizeof(stSCICfg)) != NV_OK)
    {
        AT_ERR_LOG("At_QrySIMSlotPara: en_NV_Item_SCI_DSDA_CFG read fail!");
        gstAtSendData.usBufLen = 0;
        return AT_ERROR;
    }

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      stSCICfg.bitCard0,
                                      stSCICfg.bitCard1);

#if (MULTI_MODEM_NUMBER == 3)
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       stSCICfg.bitCard2);
#endif

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 At_SetHvsstPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                              ulResult;
    SI_PIH_HVSST_SET_STRU                   stHvSStSet;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    if ( (gastAtParaList[0].usParaLen == 0)
      || (gastAtParaList[1].usParaLen == 0) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stHvSStSet, sizeof(stHvSStSet), 0x00, sizeof(stHvSStSet));

    stHvSStSet.ucIndex = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stHvSStSet.enSIMSet = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    ulResult = SI_PIH_HvSstSet(gastAtClientTab[ucIndex].usClientId,
                               gastAtClientTab[ucIndex].opId,
                               &stHvSStSet);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetHvsstPara: SI_PIH_HvSstSet fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HVSST_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_PHONE_SC == FEATURE_ON)

VOS_UINT32 At_SetSilentPin(
    VOS_UINT8                           ucIndex
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    SI_PIH_CRYPTO_PIN_STRU              stCryptoPin;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("At_SetSilentPinInfo: CmdOptType fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  ������Ϊ3 */
    if (gucAtParaIndex != 3)
    {
        AT_WARN_LOG1("At_SetSilentPinInfo: para num  %d.", gucAtParaIndex);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((gastAtParaList[0].usParaLen != (DRV_AGENT_PIN_CRYPTO_DATA_LEN * 2))
      || (gastAtParaList[1].usParaLen != (DRV_AGENT_PIN_CRYPTO_IV_LEN * 2))
      || (gastAtParaList[2].usParaLen != (DRV_AGENT_HMAC_DATA_LEN * 2)))
    {
        AT_WARN_LOG2("At_SetSilentPinInfo: 0 %d %d.",
                     gastAtParaList[0].usParaLen,
                     gastAtParaList[1].usParaLen );

        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stCryptoPin, sizeof(stCryptoPin), 0, sizeof(stCryptoPin));

    /* ������PIN�ַ�������ת��Ϊ���� */
    usLength = gastAtParaList[0].usParaLen;
    ulResult = At_AsciiNum2HexString(gastAtParaList[0].aucPara, &usLength);
    if ( (ulResult != AT_SUCCESS) || (usLength != DRV_AGENT_PIN_CRYPTO_DATA_LEN) )
    {
        AT_WARN_LOG1("At_SetSilentPinInfo: Encpin fail %d.", usLength);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    lMemResult = memcpy_s(stCryptoPin.aucCryptoPin,
                          DRV_AGENT_PIN_CRYPTO_DATA_LEN,
                          (VOS_VOID*)gastAtParaList[0].aucPara,
                          DRV_AGENT_PIN_CRYPTO_DATA_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, DRV_AGENT_PIN_CRYPTO_DATA_LEN, DRV_AGENT_PIN_CRYPTO_DATA_LEN);

    /* ��IV�ַ�������ת��Ϊ���� */
    usLength = gastAtParaList[1].usParaLen;
    ulResult = At_AsciiNum2HexString(gastAtParaList[1].aucPara, &usLength);
    if ( (ulResult != AT_SUCCESS) || (usLength != DRV_AGENT_PIN_CRYPTO_IV_LEN) )
    {
        AT_WARN_LOG1("At_SetSilentPinInfo: IV Len fail %d.", usLength);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    lMemResult = memcpy_s(stCryptoPin.aulPinIv,
                          DRV_AGENT_PIN_CRYPTO_IV_LEN,
                          (VOS_VOID*)gastAtParaList[1].aucPara,
                          DRV_AGENT_PIN_CRYPTO_IV_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, DRV_AGENT_PIN_CRYPTO_IV_LEN, DRV_AGENT_PIN_CRYPTO_IV_LEN);

    /* ��HMAC�ַ�������ת��Ϊ���� */
    usLength = gastAtParaList[2].usParaLen;
    ulResult = At_AsciiNum2HexString(gastAtParaList[2].aucPara, &usLength);
    if ( (ulResult != AT_SUCCESS) || (usLength != DRV_AGENT_HMAC_DATA_LEN) )
    {
        AT_WARN_LOG1("At_SetSilentPinInfo: hmac Len fail %d.", usLength);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    lMemResult = memcpy_s(stCryptoPin.aucHmacValue,
                          DRV_AGENT_HMAC_DATA_LEN,
                          (VOS_VOID*)gastAtParaList[2].aucPara,
                          DRV_AGENT_HMAC_DATA_LEN);
    TAF_MEM_CHK_RTN_VAL(lMemResult, DRV_AGENT_HMAC_DATA_LEN, DRV_AGENT_HMAC_DATA_LEN);

    ulResult = SI_PIH_SetSilentPinReq(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      &stCryptoPin);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetSilentPinInfo: SI_PIH_SetSilentPinReq fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SILENTPIN_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID At_ClearPIN(VOS_UINT8 *pucMem, VOS_UINT8 ucLen)
{
    VOS_UINT32                          i;

    for(i=0; i<ucLen; i++)
    {
        pucMem[i] = 0XFF;
    }

    return;
}


VOS_UINT32 At_SetSilentPinInfo(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           aucPin[TAF_PH_PINCODELENMAX+1] = {0};
    VOS_UINT32                          ulResult;
    errno_t                             lMemResult;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϊ1 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    if ( (gastAtParaList[0].usParaLen > TAF_PH_PINCODELENMAX)
      || (gastAtParaList[0].usParaLen < TAF_PH_PINCODELENMIN) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(aucPin, TAF_PH_PINCODELENMAX, 0xFF, TAF_PH_PINCODELENMAX);

    lMemResult = memcpy_s(aucPin,
                          TAF_PH_PINCODELENMAX,
                          (VOS_VOID*)gastAtParaList[0].aucPara,
                          gastAtParaList[0].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, TAF_PH_PINCODELENMAX, gastAtParaList[0].usParaLen);

    ulResult = SI_PIH_GetSilentPinInfoReq(gastAtClientTab[ucIndex].usClientId,
                                          gastAtClientTab[ucIndex].opId,
                                          (VOS_VOID*)aucPin,
                                          TAF_PH_PINCODELENMAX);
    /* ������Ϣ��0xFF */
    At_ClearPIN(aucPin, TAF_PH_PINCODELENMAX);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetSilentPinInfo: SI_PIH_SetSilentPinReq fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SILENTPININFO_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif


VOS_UINT32 At_SetEsimCleanProfile(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulResult = SI_EMAT_EsimCleanProfile(gastAtClientTab[ucIndex].usClientId,
                                        gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetEsimCleanProfile: Set Esim Clean fail.");

        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ESIMCLEAN_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryEsimCheckProfile(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulResult = SI_EMAT_EsimCheckProfile(gastAtClientTab[ucIndex].usClientId,
                                        gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_QryEsimCheckProfile: Set Esim Switch fail.");

        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ESIMCHECK_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryEsimEid(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulResult = SI_EMAT_EsimEidQuery(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_QryEsimEid: Get eSIM EID fail.");

        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ESIMEID_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryEsimPKID(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulResult = SI_EMAT_EsimPKIDQuery(gastAtClientTab[ucIndex].usClientId,
                                     gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_QryEsimPKID: Get eSIM PKID fail.");

        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ESIMPKID_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_SetEsimSwitchPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;
    SI_PIH_ESIMSWITCH_SET_STRU          stEsimSwitchSet = {0};

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    if ( (gastAtParaList[0].usParaLen == 0)
      || (gastAtParaList[1].usParaLen == 0) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stEsimSwitchSet.ucSlot      = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stEsimSwitchSet.ucCardType  = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    ulResult = SI_PIH_EsimSwitchSet(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    &stEsimSwitchSet);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetEsimSwitchPara: Set Esim Switch fail.");

        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ESIMSWITCH_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryEsimSwitch(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulResult = SI_PIH_EsimSwitchQuery(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_QryEsimSwitch: Qry Esim Switch fail.");

        return AT_ERROR;
    }

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ESIMSWITCH_QRY;

    /* ������������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryHvsstPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulResult;

    ulResult = SI_PIH_HvSstQuery(gastAtClientTab[ucIndex].usClientId,
                                 gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("AT_QryPortAttribSetPara: SI_PIH_HvSstQuery fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HVSST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_HvsstQueryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    TAF_NV_SCI_CFG_STRU                 stSCICfg;
    VOS_UINT32                          ulSlot;
    SI_PIH_SIM_INDEX_ENUM_UINT8         enSimIndex;

    ulRslt      = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_HvsstQueryCnf: Get modem id fail.");
        return AT_ERROR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_HVSST_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_HVSST_QRY)
    {
        AT_WARN_LOG("At_HvsstQueryCnf : CmdCurrentOpt is not AT_CMD_HVSST_QRY!");
        return AT_ERROR;
    }

    /* ��NV�ж�ȡ��ǰSIM����SCI���� */
    memset_s(&stSCICfg, sizeof(stSCICfg), 0x00, sizeof(stSCICfg));

    if (TAF_ACORE_NV_READ(MODEM_ID_0,
                                   en_NV_Item_SCI_DSDA_CFG,
                                   &stSCICfg,
                                   sizeof(stSCICfg)) != NV_OK)
    {
        AT_ERR_LOG("At_HvsstQueryCnf: en_NV_Item_SCI_DSDA_CFG read fail!");
        return AT_ERROR;
    }

    if (enModemId == MODEM_ID_0)
    {
        ulSlot = stSCICfg.bitCard0;
    }
    else if (enModemId == MODEM_ID_1)
    {
        ulSlot = stSCICfg.bitCard1;
    }
    else
    {
        ulSlot = stSCICfg.bitCard2;
    }

    if (pstEvent->PIHEvent.HVSSTQueryCnf.enVSimState == SI_PIH_SIM_ENABLE)
    {
        enSimIndex = SI_PIH_SIM_VIRT_SIM1;
    }
    else
    {
        enSimIndex = SI_PIH_SIM_REAL_SIM1;
    }

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                       "%s: %d,%d,%d,%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       enSimIndex,
                                       1,
                                       ulSlot,
                                       pstEvent->PIHEvent.HVSSTQueryCnf.enCardUse);

    return AT_OK;
}


VOS_UINT32 At_SetSciChgPara(
    VOS_UINT8                           ucIndex
)
{
 #if (MULTI_MODEM_NUMBER != 1)
    VOS_UINT32                          ulResult;
#endif
    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 3)
    {
        return AT_TOO_MANY_PARA;
    }

    /* ������� */
    if ( (gastAtParaList[0].usParaLen == 0)
       ||(gastAtParaList[1].usParaLen == 0) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��modem��֧�ֿ����л� */
#if (MULTI_MODEM_NUMBER == 1)
    return AT_CME_OPERATION_NOT_ALLOWED;
#else

    /* ������̬��3����������Ϊ�գ�������̬Ĭ��Ϊ����2 */
#if (MULTI_MODEM_NUMBER == 3)
    if (gastAtParaList[2].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������Modem����ͬʱ����Ϊͬһ���� */
    if ( (gastAtParaList[0].ulParaValue == gastAtParaList[1].ulParaValue)
      || (gastAtParaList[0].ulParaValue == gastAtParaList[2].ulParaValue)
      || (gastAtParaList[1].ulParaValue == gastAtParaList[2].ulParaValue) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#else
    gastAtParaList[2].ulParaValue = SI_PIH_CARD_SLOT_2;

    /* ��������Modem����ͬʱ����Ϊͬһ���� */
    if (gastAtParaList[0].ulParaValue == gastAtParaList[1].ulParaValue)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    ulResult = SI_PIH_SciCfgSet(gastAtClientTab[ucIndex].usClientId,
                                gastAtClientTab[ucIndex].opId,
                                gastAtParaList[0].ulParaValue,
                                gastAtParaList[1].ulParaValue,
                                gastAtParaList[2].ulParaValue);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetSciChgPara: SI_PIH_HvSstSet fail.");
        return AT_CME_PHONE_FAILURE;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SCICHG_SET;

    return AT_WAIT_ASYNC_RETURN;
 #endif
}


VOS_UINT32 At_SetBwtPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* �������ͼ�� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex != 1)
    {
        return AT_TOO_MANY_PARA;
    }

    /* ������� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulResult = SI_PIH_BwtSet(gastAtClientTab[ucIndex].usClientId,
                             gastAtClientTab[ucIndex].opId,
                             (VOS_UINT16)gastAtParaList[0].ulParaValue);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_SetBwtPara: SI_PIH_HvSstSet fail.");

        return AT_CME_PHONE_FAILURE;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_BWT_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QrySciChgPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulResult;

    ulResult = SI_PIH_SciCfgQuery(gastAtClientTab[ucIndex].usClientId,
                                  gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_QrySciChgPara: SI_PIH_SciCfgQuery fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SCICHG_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_SciCfgQueryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_SCICHG_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SCICHG_QRY)
    {
        AT_WARN_LOG("At_SciCfgQueryCnf : CmdCurrentOpt is not AT_CMD_SCICHG_QRY!");
        return AT_ERROR;
    }

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstEvent->PIHEvent.SciCfgCnf.enCard0Slot,
                                      pstEvent->PIHEvent.SciCfgCnf.enCard1Slot);

#if (MULTI_MODEM_NUMBER == 3)
    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                       ",%d",
                                       pstEvent->PIHEvent.SciCfgCnf.enCard2Slot);
#endif

    return AT_OK;
}


VOS_UINT32 At_ProcPihFndBndCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CLCK_PIN_HANDLE)
    {
        AT_WARN_LOG("At_ProcPihFndBndCnf : CmdCurrentOpt is not AT_CMD_CLCK_PIN_HANDLE!");
        return AT_ERROR;
    }

    /* �����״̬��ѯ���� */
    if(pstEvent->PIHEvent.FDNCnf.FdnCmd == SI_PIH_FDN_BDN_QUERY)
    {
        (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s",gaucAtCrLf);
        (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d",pstEvent->PIHEvent.FDNCnf.FdnState);
    }

    return AT_OK;
}


VOS_UINT32 At_ProcPihGenericAccessCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSIM_SET)
    {
        AT_WARN_LOG("At_ProcPihGenericAccessCnf : CmdCurrentOpt is not AT_CMD_CSIM_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <length>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d,\"",(pstEvent->PIHEvent.GAccessCnf.Len+2)*2);
    if(pstEvent->PIHEvent.GAccessCnf.Len != 0)
    {
        /* <command>, */
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),pstEvent->PIHEvent.GAccessCnf.Command,pstEvent->PIHEvent.GAccessCnf.Len);
    }
    /*SW1*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),&pstEvent->PIHEvent.GAccessCnf.SW1,sizeof(TAF_UINT8));
    /*SW1*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),&pstEvent->PIHEvent.GAccessCnf.SW2,sizeof(TAF_UINT8));
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihIsdbAccessCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CISA_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CISA_SET)
    {
        AT_WARN_LOG("At_ProcPihIsdbAccessCnf : CmdCurrentOpt is not AT_CMD_CISA_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* <length>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d,\"", (pstEvent->PIHEvent.IsdbAccessCnf.usLen + 2) * 2);
    if(pstEvent->PIHEvent.IsdbAccessCnf.usLen != 0)
    {
        /* <command>, */
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), pstEvent->PIHEvent.IsdbAccessCnf.aucCommand, pstEvent->PIHEvent.IsdbAccessCnf.usLen);
    }

    /*SW1*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.IsdbAccessCnf.ucSW1, sizeof(TAF_UINT8));

    /*SW2*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.IsdbAccessCnf.ucSW2, sizeof(TAF_UINT8));
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), "\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihCchoSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CCHO_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCHO_SET)
    {
        AT_WARN_LOG("At_ProcPihCchoSetCnf : CmdCurrentOpt is not AT_CMD_CCHO_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <sessionid>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%u", pstEvent->PIHEvent.stOpenChannelCnf.ulSessionID);

    return AT_OK;
}


VOS_UINT32 At_ProcPihCchpSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CCHP_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCHP_SET)
    {
        AT_WARN_LOG("At_ProcPihCchpSetCnf : CmdCurrentOpt is not AT_CMD_CCHP_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <sessionid>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%u", pstEvent->PIHEvent.stOpenChannelCnf.ulSessionID);

    return AT_OK;
}


VOS_UINT32 At_ProcPihPrivateCchoSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_PRIVATECCHO_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PRIVATECCHO_SET)
    {
        AT_WARN_LOG("At_ProcPihPrivateCchoSetCnf : CmdCurrentOpt is not AT_CMD_PRIVATECCHO_SET!");

        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <sessionid> */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%u,", pstEvent->PIHEvent.stOpenChannelCnf.ulSessionID);
    /* <response len> */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d,\"", (pstEvent->PIHEvent.stOpenChannelCnf.usRspDataLen + 2) * 2);
    /* <response data> */
    (*pusLength) += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),
                                                        (TAF_UINT8 *)pstEvent->PIHEvent.stOpenChannelCnf.aucRspDate,
                                                        pstEvent->PIHEvent.stOpenChannelCnf.usRspDataLen);
    /*SW1*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.stOpenChannelCnf.ucSW1, sizeof(TAF_UINT8));
    /*SW2*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.stOpenChannelCnf.ucSW2, sizeof(TAF_UINT8));
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), "\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihPrivateCchpSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLen
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CCHP_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PRIVATECCHP_SET)
    {
        AT_WARN_LOG("At_ProcPihPrivateCchpSetCnf : CmdCurrentOpt is not AT_CMD_PRIVATECCHP_SET!");

        return AT_ERROR;
    }

    (*pusLen) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLen),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <sessionid> */
    (*pusLen) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLen),"%u,", pstEvent->PIHEvent.stOpenChannelCnf.ulSessionID);
    /* <response len> */
    (*pusLen) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLen),"%d,\"", (pstEvent->PIHEvent.stOpenChannelCnf.usRspDataLen + 2) * 2);
    /* <response data> */
    (*pusLen) += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                     (TAF_INT8 *)pgucAtSndCodeAddr,
                                                     (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLen),
                                                     (TAF_UINT8 *)pstEvent->PIHEvent.stOpenChannelCnf.aucRspDate,
                                                     pstEvent->PIHEvent.stOpenChannelCnf.usRspDataLen);
    /*SW1*/
    (*pusLen) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLen), &pstEvent->PIHEvent.stOpenChannelCnf.ucSW1, sizeof(TAF_UINT8));
    /*SW2*/
    (*pusLen) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLen), &pstEvent->PIHEvent.stOpenChannelCnf.ucSW2, sizeof(TAF_UINT8));
    (*pusLen) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLen), "\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihCchcSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CCHC_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCHC_SET)
    {
        AT_WARN_LOG("At_ProcPihCchcSetCnf : CmdCurrentOpt is not AT_CMD_CCHC_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_ProcPihSciCfgSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_SCICHG_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SCICHG_SET)
    {
        AT_WARN_LOG("At_ProcPihSciCfgSetCnf : CmdCurrentOpt is not AT_CMD_SCICHG_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_ProcPihBwtSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_BWT_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_BWT_SET)
    {
        AT_WARN_LOG("At_ProcPihBwtSetCnf : CmdCurrentOpt is not AT_CMD_BWT_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_ProcPihHvsstSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_HVSST_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_HVSST_SET)
    {
        AT_WARN_LOG("At_ProcPihHvsstSetCnf : CmdCurrentOpt is not AT_CMD_HVSST_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_ProcPihCglaSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CGLA_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGLA_SET)
    {
        AT_WARN_LOG("At_ProcPihCglaSetCnf : CmdCurrentOpt is not AT_CMD_CGLA_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* <length>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d,\"", (pstEvent->PIHEvent.stCglaCmdCnf.usLen + 2) * 2);
    if(pstEvent->PIHEvent.stCglaCmdCnf.usLen != 0)
    {
        /* <command>, */
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), pstEvent->PIHEvent.stCglaCmdCnf.aucCommand, pstEvent->PIHEvent.stCglaCmdCnf.usLen);
    }

    /*SW1*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.stCglaCmdCnf.ucSW1, sizeof(TAF_UINT8));

    /*SW2*/
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.stCglaCmdCnf.ucSW2, sizeof(TAF_UINT8));
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), "\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihCardAtrQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CARD_ATR_READ */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CARD_ATR_READ)
    {
        AT_WARN_LOG("At_ProcPihCardAtrQryCnf : CmdCurrentOpt is not AT_CMD_CARD_ATR_READ!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), "%s:\"", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), pstEvent->PIHEvent.stATRQryCnf.aucCommand, (VOS_UINT16)pstEvent->PIHEvent.stATRQryCnf.ulLen);

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), "\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihCardTypeQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CARDTYPE_QUERY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CARDTYPE_QUERY)
    {
        AT_WARN_LOG("At_ProcPihCardTypeQryCnf : CmdCurrentOpt is not AT_CMD_CARDTYPE_QUERY!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                    "%s: %d, %d, %d",
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                    pstEvent->PIHEvent.CardTypeCnf.ucMode,
                    pstEvent->PIHEvent.CardTypeCnf.ucHasCModule,
                    pstEvent->PIHEvent.CardTypeCnf.ucHasGModule);

    return AT_OK;
}


VOS_UINT32 At_ProcPihCardTypeExQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CARDTYPEEX_QUERY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CARDTYPEEX_QUERY)
    {
        AT_WARN_LOG("At_ProcPihCardTypeExQryCnf : CmdCurrentOpt is not AT_CMD_CARDTYPEEX_QUERY!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                    "%s: %d, %d, %d",
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                    pstEvent->PIHEvent.CardTypeCnf.ucMode,
                    pstEvent->PIHEvent.CardTypeCnf.ucHasCModule,
                    pstEvent->PIHEvent.CardTypeCnf.ucHasGModule);

    return AT_OK;
}


VOS_UINT32 At_ProcPihCardVoltageQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CARDVOLTAGE_QUERY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CARDVOLTAGE_QUERY)
    {
        AT_WARN_LOG("At_ProcPihCardVoltageQryCnf : CmdCurrentOpt is not AT_CMD_CARDVOLTAGE_QUERY!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr,
            "%s: %d, %x",
            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
            pstEvent->PIHEvent.stCardVoltageCnf.ulVoltage,
            pstEvent->PIHEvent.stCardVoltageCnf.ucCharaByte);

    return AT_OK;
}


VOS_UINT32 At_ProcPihPrivateCglaSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_PRIVATECGLA_REQ */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PRIVATECGLA_REQ)
    {
        AT_WARN_LOG("At_ProcPihPrivateCglaSetCnf : CmdCurrentOpt is not AT_CMD_PRIVATECGLA_REQ!");
        return AT_ERROR;
    }

    /* �����һ����ӡ�ϱ�IND����Ҫ��������ӻس����� */
    if (pstEvent->PIHEvent.stCglaHandleCnf.ucLastDataFlag != VOS_TRUE)
    {
        (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr, "\r\n");
    }

    /* ^CGLA: <flag>,<length>,"[<command>]<SW1><SW2>" */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                       "%s: %d,%d,\"",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstEvent->PIHEvent.stCglaHandleCnf.ucLastDataFlag,
                                       (pstEvent->PIHEvent.stCglaHandleCnf.usLen + 2) * 2);

    if(pstEvent->PIHEvent.stCglaHandleCnf.usLen != 0)
    {
        /* <command>, */
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), pstEvent->PIHEvent.stCglaHandleCnf.aucCommand, pstEvent->PIHEvent.stCglaHandleCnf.usLen);
    }

    /* <SW1> */
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.stCglaHandleCnf.ucSW1, (VOS_UINT16)sizeof(TAF_UINT8));

    /* <SW2> */
    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength), &pstEvent->PIHEvent.stCglaHandleCnf.ucSW2, (VOS_UINT16)sizeof(TAF_UINT8));
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), "\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihCrsmSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CRSM_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CRSM_SET)
    {
        AT_WARN_LOG("At_ProcPihCrsmSetCnf : CmdCurrentOpt is not AT_CMD_CRSM_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <sw1, sw2>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d,%d",pstEvent->PIHEvent.RAccessCnf.ucSW1, pstEvent->PIHEvent.RAccessCnf.ucSW2);

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"");

    if(pstEvent->PIHEvent.RAccessCnf.usLen != 0)
    {
        /* <response> */
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),pstEvent->PIHEvent.RAccessCnf.aucContent, pstEvent->PIHEvent.RAccessCnf.usLen);
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihCrlaSetCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CRLA_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CRLA_SET)
    {
        AT_WARN_LOG("At_ProcPihCrlaSetCnf: CmdCurrentOpt is not AT_CMD_CRLA_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* <sw1, sw2>, */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d,%d",pstEvent->PIHEvent.RAccessCnf.ucSW1, pstEvent->PIHEvent.RAccessCnf.ucSW2);

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"");

    if(pstEvent->PIHEvent.RAccessCnf.usLen != 0)
    {
        /* <response> */
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),pstEvent->PIHEvent.RAccessCnf.aucContent, pstEvent->PIHEvent.RAccessCnf.usLen);
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");

    return AT_OK;
}


VOS_UINT32 At_ProcPihSessionQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CARDSESSION_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CARDSESSION_QRY)
    {
        AT_WARN_LOG("At_ProcPihSessionQryCnf: CmdCurrentOpt is not AT_CMD_CARDSESSION_QRY!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* <CSIM,USIM,ISIM> */
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"CSIM,%d,",pstEvent->PIHEvent.aulSessionID[USIMM_CDMA_APP]);
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"USIM,%d,",pstEvent->PIHEvent.aulSessionID[USIMM_GUTL_APP]);
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),"ISIM,%d", pstEvent->PIHEvent.aulSessionID[USIMM_IMS_APP]);

    return AT_OK;
}


VOS_UINT32 At_ProcPihCimiQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CIMI_READ */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CIMI_READ)
    {
        AT_WARN_LOG("At_ProcPihCimiQryCnf: CmdCurrentOpt is not AT_CMD_CIMI_READ!");
        return AT_ERROR;
    }

    g_enLogPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_CIMI;
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%s",
                                           pstEvent->PIHEvent.stImsi.aucImsi);

    return AT_OK;
}


VOS_UINT32 At_ProcPihCcimiQryCnf(
    VOS_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CCIMI_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCIMI_SET)
    {
        AT_WARN_LOG("At_ProcPihCcimiQryCnf: CmdCurrentOpt is not AT_CMD_CCIMI_SET!");
        return AT_ERROR;
    }

    g_enLogPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_CIMI;
    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%s",
                                           pstEvent->PIHEvent.stImsi.aucImsi);

    return AT_OK;
}


#if (FEATURE_VSIM == FEATURE_ON)
#if (FEATURE_VSIM_ICC_SEC_CHANNEL == FEATURE_ON)

VOS_UINT32 At_QryIccVsimVer(
    VOS_UINT8                           ucIndex
)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "^ICCVSIMVER: %u",
                                       SI_PIH_GetSecIccVsimVer());
    return AT_OK;
}


VOS_UINT32 At_QryHvCheckCardPara(
    VOS_UINT8                           ucIndex
)
{
    if(SI_PIH_HvCheckCardQuery(gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId) == AT_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HVSCONT_READ;

        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }

    return AT_ERROR;
}

#endif
#endif  /*end of (FEATURE_VSIM == FEATURE_ON)*/

#if (FEATURE_IMS == FEATURE_ON)

VOS_UINT32 AT_UiccAuthCnf(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_UICCAUTH_SET/AT_CMD_KSNAFAUTH_SET */
    if ( (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_UICCAUTH_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_KSNAFAUTH_SET))
    {
        AT_WARN_LOG("AT_UiccAuthCnf : CmdCurrentOpt is not AT_CMD_UICCAUTH_SET/AT_CMD_KSNAFAUTH_SET!");
        return AT_ERROR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_UICCAUTH_SET)
    {
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"^UICCAUTH:");

        /* <result> */
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d",pstEvent->PIHEvent.UiccAuthCnf.enStatus);

        if (pstEvent->PIHEvent.UiccAuthCnf.enStatus == SI_PIH_AUTH_SUCCESS)
        {
            /* ,<Res> */
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"");
            (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr+(*pusLength), &pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucAuthRes[1], pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucAuthRes[0]);
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");

            if (pstEvent->PIHEvent.UiccAuthCnf.enAuthType == SI_PIH_UICCAUTH_AKA)
            {
                /* ,<ck> */
                (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"");
                (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr+(*pusLength), &pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucCK[1], pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucCK[0]);
                (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");

                /* ,<ik> */
                (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"");
                (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr+(*pusLength), &pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucIK[1], pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucIK[0]);
                (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");
            }
        }

        if (pstEvent->PIHEvent.UiccAuthCnf.enStatus == SI_PIH_AUTH_SYNC)
        {
            /* ,"","","",<autn> */
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"\",\"\",\"\",\"");
            (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr+(*pusLength), &pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucAuts[1], pstEvent->PIHEvent.UiccAuthCnf.stAkaData.aucAuts[0]);
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");
        }
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_KSNAFAUTH_SET)
    {
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"^KSNAFAUTH:");

        /* <status> */
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"%d",pstEvent->PIHEvent.UiccAuthCnf.enStatus);

        if (pstEvent->PIHEvent.UiccAuthCnf.stNAFData.aucKs_ext_NAF[0] != VOS_NULL)
        {
            /* ,<ks_Naf> */
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),",\"");
            (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr+(*pusLength), &pstEvent->PIHEvent.UiccAuthCnf.stNAFData.aucKs_ext_NAF[1], pstEvent->PIHEvent.UiccAuthCnf.stNAFData.aucKs_ext_NAF[0]);
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");
        }
    }

    return AT_OK;
}


VOS_UINT32 AT_UiccAccessFileCnf(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CURSM_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CURSM_SET)
    {
        AT_WARN_LOG("AT_UiccAccessFileCnf : CmdCurrentOpt is not AT_CMD_CURSM_SET!");
        return AT_ERROR;
    }

    if ((pstEvent->PIHEvent.UiccAcsFileCnf.ulDataLen != 0)
        && (pstEvent->PIHEvent.UiccAcsFileCnf.enCmdType == SI_PIH_ACCESS_READ))
    {
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"^CURSM:");

        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");
        (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr+(*pusLength), pstEvent->PIHEvent.UiccAcsFileCnf.aucCommand, (VOS_UINT16)pstEvent->PIHEvent.UiccAcsFileCnf.ulDataLen);
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),"\"");
    }

    return AT_OK;
}
#endif


TAF_UINT32 At_CrlaFilePathCheck(
    TAF_UINT32                          ulEfId,
    TAF_UINT8                          *pucFilePath,
    TAF_UINT16                         *pusPathLen)
{
    errno_t                             lMemResult;
    TAF_UINT16                          usLen;
    TAF_UINT16                          ausPath[USIMM_MAX_PATH_LEN]  = {0};
    TAF_UINT16                          ausTmpPath[USIMM_MAX_PATH_LEN]  = {0};
    TAF_UINT16                          usPathLen;
    TAF_UINT16                          i;

    usPathLen   = *pusPathLen;
    usLen       = 0;

    if (usPathLen > USIMM_MAX_PATH_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (i = 0; i < (usPathLen/sizeof(TAF_UINT16)); i++)
    {
        ausTmpPath[i] = ((pucFilePath[i*2]<<0x08)&0xFF00) + pucFilePath[(i*2)+1];
    }

    /* ���·��������3F00��ʼ����Ҫ���3F00����ͷ */
    if (ausTmpPath[0] != MFID)
    {
        if (usPathLen == USIMM_MAX_PATH_LEN)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        ausPath[0] = MFID;

        usLen++;
    }

    lMemResult = memcpy_s(&ausPath[usLen], sizeof(TAF_UINT16) * (USIMM_MAX_PATH_LEN - usLen), ausTmpPath, usPathLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_UINT16) * (USIMM_MAX_PATH_LEN - usLen), usPathLen);

    usLen += (usPathLen/sizeof(TAF_UINT16));

    if ((ulEfId & 0xFF00) == EFIDUNDERMF)
    {
        usLen = 1;
    }
    /* 4F�ļ�Ҫ��5F�£�·������Ϊ3 */
    else if ((ulEfId & 0xFF00) == EFIDUNDERMFDFDF)
    {
        if ((usLen != 3)
            ||((ausPath[1]&0xFF00) != DFIDUNDERMF)
            ||((ausPath[2]&0xFF00) != DFIDUNDERMFDF))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    /* 6F�ļ�Ҫ��7F�£�·������Ϊ2 */
    else if ((ulEfId & 0xFF00) == EFIDUNDERMFDF)
    {
        if ((usLen != 2)
            ||((ausPath[1]&0xFF00) != DFIDUNDERMF))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
    }

    *pusPathLen  = usLen;

    lMemResult = memcpy_s(pucFilePath, AT_PARA_MAX_LEN + 1, ausPath, (VOS_SIZE_T)(usLen*2));
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_PARA_MAX_LEN + 1, (VOS_SIZE_T)(usLen*2));

    return AT_SUCCESS;
}


TAF_UINT32 At_CrlaApduParaCheck(VOS_VOID)
{
    TAF_UINT16                          usFileTag;

    /* �������Ͳ�����飬�ڶ�����������Ϊ�� */
    if (gastAtParaList[1].usParaLen == 0)
    {
        AT_ERR_LOG("At_CrlaApduParaCheck: command type null");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��STATUS�����⣬�ļ�ID���벻��Ϊ�� */
    if ((gastAtParaList[2].ulParaValue == 0)
        && (gastAtParaList[1].ulParaValue != USIMM_STATUS))
    {
        AT_ERR_LOG("At_CrlaApduParaCheck: File Id null.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ȡ�ļ�IDǰ��λ */
    usFileTag   = (gastAtParaList[2].ulParaValue >> 8) & (0x00FF);

    /* ������ļ�ID������EF�ļ���ǰ��λ��������3F/5F/7F */
    if ((usFileTag == MFLAB)
       || (usFileTag == DFUNDERMFLAB)
       || (usFileTag == DFUNDERDFLAB))
    {
        AT_ERR_LOG("At_CrlaApduParaCheck: File Id error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <P1><P2><P3>����������ȫ��Ϊ�� */
    if ((gastAtParaList[3].usParaLen == 0)
        && (gastAtParaList[4].usParaLen == 0)
        && (gastAtParaList[5].usParaLen == 0))
    {
        return AT_SUCCESS;
    }

    /* <P1><P2><P3>����������ȫ����Ϊ�� */
    if ((gastAtParaList[3].usParaLen != 0)
        && (gastAtParaList[4].usParaLen != 0)
        && (gastAtParaList[5].usParaLen != 0))
    {
        return AT_SUCCESS;
    }

    /* �����������������AT������������� */
    return AT_CME_INCORRECT_PARAMETERS;

}


TAF_UINT32 At_CrlaFilePathParse(
    SI_PIH_CRLA_STRU                   *pstCommand
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulResult;

    /* ����ʷ������������ڰ˸�����Ϊ�գ�˵��û���ļ�·�����룬ֱ�ӷ��سɹ� */
    if ((gastAtParaList[7].usParaLen == 0)
     && (pstCommand->usEfId != VOS_NULL_WORD))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ת��ǰ������ļ�·�����ȱ�����4�������� */
    if ((gastAtParaList[7].usParaLen % 4) != 0)
    {
        AT_ERR_LOG("At_CrlaFilePathParse: Path error");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*��������ַ���ת����ʮ����������*/
    if(At_AsciiNum2HexString(gastAtParaList[7].aucPara, &gastAtParaList[7].usParaLen) == AT_FAILURE)
    {
        AT_ERR_LOG("At_CrlaFilePathParse: At_AsciiNum2HexString error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������д�ļ�ID��·����Ҫ���ļ�·����飬�����·��������U16Ϊ��λ */
    ulResult = At_CrlaFilePathCheck((TAF_UINT16)gastAtParaList[2].ulParaValue,
                                    gastAtParaList[7].aucPara,
                                   &gastAtParaList[7].usParaLen);

    if (ulResult != AT_SUCCESS)
    {
        AT_ERR_LOG("At_CrlaFilePathParse: At_CrsmFilePathCheck error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����ļ�·���ͳ��� */
    pstCommand->usPathLen   = gastAtParaList[7].usParaLen;

    /* �ļ�·��������U16Ϊ��λ�ģ�·�������ĳ���Ҫ��2 */
    lMemResult = memcpy_s(pstCommand->ausPath, sizeof(pstCommand->ausPath), gastAtParaList[7].aucPara, (VOS_SIZE_T)(gastAtParaList[7].usParaLen*sizeof(VOS_UINT16)));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstCommand->ausPath), (VOS_SIZE_T)(gastAtParaList[7].usParaLen*sizeof(VOS_UINT16)));

    return AT_SUCCESS;
}


TAF_UINT32 At_CrlaParaStatusCheck(
    SI_PIH_CRLA_STRU                   *pstCommand
)
{
    /* STATUS�������û�������ļ�ID���Ͳ���Ҫ��ѡ�ļ�������ֱ�ӷ�STATUS���� */
    if (gastAtParaList[2].ulParaValue == 0)
    {
        pstCommand->usEfId = VOS_NULL_WORD;
    }
    else
    {
        pstCommand->usEfId = (TAF_UINT16)gastAtParaList[2].ulParaValue;
    }

    /* ��д���ݽṹ�е�<P1><P2><P3>��Ӧ��IE�� */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;
    pstCommand->enCmdType   =   USIMM_STATUS;

    return At_CrlaFilePathParse(pstCommand);
}


TAF_UINT32 At_CrlaParaReadBinaryCheck(
    SI_PIH_CRLA_STRU                   *pstCommand
)
{
    /* ��д���ݽṹ�е�<P1><P2><P3>��Ӧ��IE�� */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;
    pstCommand->usEfId      =   (TAF_UINT16)gastAtParaList[2].ulParaValue;
    pstCommand->enCmdType   =   USIMM_READ_BINARY;

    /* ����������ļ�·����Ҫ���������� */
    return At_CrlaFilePathParse(pstCommand);
}


TAF_UINT32 At_CrlaParaReadRecordCheck(
    SI_PIH_CRLA_STRU                   *pstCommand
)
{
    /* ��д���ݽṹ�е�<P1><P2><P3>��Ӧ��IE�� */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;
    pstCommand->usEfId      =   (TAF_UINT16)gastAtParaList[2].ulParaValue;
    pstCommand->enCmdType   =   USIMM_READ_RECORD;

    /* ����������ļ�·����Ҫ���������� */
    return At_CrlaFilePathParse(pstCommand);
}


VOS_UINT32 At_CrlaParaGetRspCheck(
    SI_PIH_CRLA_STRU                   *pstCommand
)
{
    /* ����������������2��������Ҫ���������ͺ��ļ�ID */
    if (gucAtParaIndex < 2)
    {
        AT_ERR_LOG("At_CrlaParaGetRspCheck: Para less than 2.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��д���ݽṹ�е�<P1><P2><P3>��Ӧ��IE�� */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;
    pstCommand->usEfId      =   (TAF_UINT16)gastAtParaList[2].ulParaValue;
    pstCommand->enCmdType   =   USIMM_GET_RESPONSE;

    /* ����������ļ�·����Ҫ���������� */
    return At_CrlaFilePathParse(pstCommand);
}


VOS_UINT32 At_CrlaParaUpdateBinaryCheck(
    SI_PIH_CRLA_STRU                       *pstCommand
)
{
    errno_t                             lMemResult;
    /* Update Binary��������Ҫ��6������������û���ļ�·�� */
    if (gucAtParaIndex < 6)
    {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: Para less than 6.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��д���ݽṹ�е�<P1><P2><P3>��Ӧ��IE�� */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;
    pstCommand->usEfId      =   (TAF_UINT16)gastAtParaList[2].ulParaValue;
    pstCommand->enCmdType   =   USIMM_UPDATE_BINARY;

    /* ���������������<data>�ַ�����ת��ǰ���ݳ��ȱ�����2�ı����Ҳ���Ϊ0 */
    if (((gastAtParaList[6].usParaLen % 2) != 0)
        || (gastAtParaList[6].usParaLen == 0))
    {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(At_AsciiNum2HexString(gastAtParaList[6].aucPara, &gastAtParaList[6].usParaLen) == AT_FAILURE)
    {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[6].usParaLen > sizeof(pstCommand->aucContent))
    {
        AT_ERR_LOG("At_CrlaParaUpdateBinaryCheck: gastAtParaList[6] too long");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����<data>���䳤����<data>��������ȷ����P3�����ճ��·���������<data>�ĳ����Ƿ��P3��ֵƥ�� */
    lMemResult = memcpy_s((TAF_VOID*)pstCommand->aucContent,
                          sizeof(pstCommand->aucContent),
                          (TAF_VOID*)gastAtParaList[6].aucPara,
                          gastAtParaList[6].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstCommand->aucContent), gastAtParaList[6].usParaLen);

    return At_CrlaFilePathParse(pstCommand);
}


VOS_UINT32 At_CrlaParaUpdateRecordCheck (
    SI_PIH_CRLA_STRU                   *pstCommand
)
{
    errno_t                             lMemResult;

    /* Update Binary��������Ҫ��6������������û���ļ�·�� */
    if (gucAtParaIndex < 6)
    {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: Para less than 6.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��д���ݽṹ�е�<P1><P2><P3>��Ӧ��IE�� */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;
    pstCommand->usEfId      =   (TAF_UINT16)gastAtParaList[2].ulParaValue;
    pstCommand->enCmdType   =   USIMM_UPDATE_RECORD;

     /* ���������������<data>�ַ������ݳ��ȱ�����2�ı����Ҳ���Ϊ0 */
    if (((gastAtParaList[6].usParaLen % 2) != 0)
        || (gastAtParaList[6].usParaLen == 0))
    {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(At_AsciiNum2HexString(gastAtParaList[6].aucPara, &gastAtParaList[6].usParaLen) == AT_FAILURE)
    {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ֹ��Ϊ���ݳ��ȹ������µ��帴λ */
    if (gastAtParaList[6].usParaLen > sizeof(pstCommand->aucContent))
    {
        AT_ERR_LOG("At_CrlaParaUpdateRecordCheck: gastAtParaList[6] too long");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����<data>���䳤����<data>��������ȷ����P3�����ճ��·���������<data>�ĳ����Ƿ��P3��ֵƥ�� */
    lMemResult = memcpy_s((TAF_VOID*)pstCommand->aucContent,
                          sizeof(pstCommand->aucContent),
                          (TAF_VOID*)gastAtParaList[6].aucPara,
                          gastAtParaList[6].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstCommand->aucContent), gastAtParaList[6].usParaLen);

    return At_CrlaFilePathParse(pstCommand);
}


VOS_UINT32 At_CrlaParaSearchRecordCheck (
    SI_PIH_CRLA_STRU                   *pstCommand)
{
    /* Search Record��������Ҫ��8������ */
    if (gucAtParaIndex < 7)
    {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: Para less than 8.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��д���ݽṹ�е�<P1><P2><P3> */
    pstCommand->ucP1        =   (TAF_UINT8)gastAtParaList[3].ulParaValue;
    pstCommand->ucP2        =   (TAF_UINT8)gastAtParaList[4].ulParaValue;
    pstCommand->ucP3        =   (TAF_UINT8)gastAtParaList[5].ulParaValue;

    /* ��д���ݽṹ�е�<fileid> */
    pstCommand->usEfId      =   (TAF_UINT16)gastAtParaList[2].ulParaValue;

    /* ��д���ݽṹ�е�<command> */
    pstCommand->enCmdType   =   USIMM_SEARCH_RECORD;

     /* ���߸����������<data>�ַ������ݳ��ȱ�����2�ı����Ҳ���Ϊ0 */
    if (((gastAtParaList[6].usParaLen % 2) != 0)
     || (gastAtParaList[6].usParaLen == 0))
    {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: <data> error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(At_AsciiNum2HexString(gastAtParaList[6].aucPara, &gastAtParaList[6].usParaLen) == AT_FAILURE)
    {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ֹ��Ϊ���ݳ��ȹ������µ��帴λ */
    if (gastAtParaList[6].usParaLen > sizeof(pstCommand->aucContent))
    {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: gastAtParaList[6] too long");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����<data>���䳤����<data>��������ȷ����P3�����ճ��·���������<data>�ĳ����Ƿ��P3��ֵƥ�� */
    if (memcpy_s((TAF_VOID*)pstCommand->aucContent,
                 sizeof(pstCommand->aucContent),
                 (TAF_VOID*)gastAtParaList[6].aucPara,
                 gastAtParaList[6].usParaLen) != EOK)
    {
        AT_ERR_LOG("At_CrlaParaSearchRecordCheck: memcpy_s fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    return At_CrlaFilePathParse(pstCommand);
}


TAF_UINT32 At_SetCrlaPara(TAF_UINT8 ucIndex)
{
    SI_PIH_CRLA_STRU                    stCommand;
    TAF_UINT32                          ulResult;

    /* �������� */
    if (gucAtParaIndex > 8)
    {
        AT_ERR_LOG("At_SetCrlaPara: too many para");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������<P1><P2><P3>����������������ֻ��д���ֲ��� */
    if (At_CrlaApduParaCheck() != AT_SUCCESS)
    {
       AT_ERR_LOG("At_SetCrlaPara: At_CrlaApduParaCheck fail.");

       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��ʼ�� */
    memset_s(&stCommand, sizeof(stCommand), 0x00, sizeof(SI_PIH_CRLA_STRU));

    stCommand.ulSessionID = gastAtParaList[0].ulParaValue;

    switch(gastAtParaList[1].ulParaValue)
    {
        case USIMM_STATUS:
            ulResult = At_CrlaParaStatusCheck(&stCommand);
            break;
        case USIMM_READ_BINARY:
            ulResult = At_CrlaParaReadBinaryCheck(&stCommand);
            break;
        case USIMM_READ_RECORD:
            ulResult = At_CrlaParaReadRecordCheck(&stCommand);
            break;
        case USIMM_GET_RESPONSE:
            ulResult = At_CrlaParaGetRspCheck(&stCommand);
            break;
        case USIMM_UPDATE_BINARY:
            ulResult = At_CrlaParaUpdateBinaryCheck(&stCommand);
            break;
        case USIMM_UPDATE_RECORD:
            ulResult = At_CrlaParaUpdateRecordCheck(&stCommand);
            break;
        case USIMM_SEARCH_RECORD:
            ulResult = At_CrlaParaSearchRecordCheck(&stCommand);
            break;
        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    if (ulResult != AT_SUCCESS )
    {
        AT_ERR_LOG("At_SetCrlaPara: para parse fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ִ��������� */
    if (SI_PIH_CrlaSetReq(gastAtClientTab[ucIndex].usClientId, 0,&stCommand) == TAF_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CRLA_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_QryCardSession(VOS_UINT8 ucIndex)
{
    if (SI_PIH_CardSessionQuery(gastAtClientTab[ucIndex].usClientId, gastAtClientTab[ucIndex].opId) == TAF_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CARDSESSION_QRY;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }

    AT_WARN_LOG("At_QryCardSession: SI_PIH_CardSessionQuery fail.");

    /* ������������״̬ */
    return AT_ERROR;
}


TAF_UINT32 At_QryCardVoltagePara(TAF_UINT8 ucIndex)
{
    VOS_UINT32 ulResult;

    ulResult = SI_PIH_CardVoltageQuery(gastAtClientTab[ucIndex].usClientId,
                                       gastAtClientTab[ucIndex].opId);

    if (ulResult != TAF_SUCCESS)
    {
        AT_WARN_LOG("At_QryCardVoltagePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CARDVOLTAGE_QUERY;

    return AT_WAIT_ASYNC_RETURN;
}


TAF_UINT32 At_SetPrivateCglaPara(TAF_UINT8 ucIndex)
{
    SI_PIH_CGLA_COMMAND_STRU    stCglaCmd;

    /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if(gucAtParaIndex != 3)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <length>��ҪΪ2�������� */
    if((gastAtParaList[1].ulParaValue % 2) != 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �ַ������Ȳ�Ϊ2�������� */
    if((gastAtParaList[2].usParaLen % 2) != 0)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���ַ���ת��Ϊ16�������� */
    if(At_AsciiNum2HexString(gastAtParaList[2].aucPara, &gastAtParaList[2].usParaLen) == AT_FAILURE)
    {
        AT_ERR_LOG("At_SetCglaCmdPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* length�ֶ���ʵ������ȵ�2�� */
    if(gastAtParaList[1].ulParaValue != (TAF_UINT32)(gastAtParaList[2].usParaLen * 2))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stCglaCmd.ulSessionID   = gastAtParaList[0].ulParaValue;
    stCglaCmd.ulLen         = gastAtParaList[2].usParaLen;
    stCglaCmd.pucCommand    = gastAtParaList[2].aucPara;

    /* ִ��������� */
    if(SI_PIH_PrivateCglaSetReq(gastAtClientTab[ucIndex].usClientId, 0, &stCglaCmd) == AT_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PRIVATECGLA_REQ;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }

    return AT_ERROR;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

TAF_UINT32 AT_SetPrfApp(
    AT_CARDAPP_ENUM_UINT32              enCardApp,
    USIMM_NV_CARDAPP_ENUM_UINT32        enCardAPP,
    MODEM_ID_ENUM_UINT16                enModemId)
{
    USIMM_APP_PRIORITY_CONFIG_STRU      stAppInfo;
    TAF_UINT32                          ulAppHit      = 0;
    TAF_UINT32                          ulAppOrderPos = 0;
    TAF_UINT32                          ulRslt;
    TAF_UINT32                          i;

    memset_s(&stAppInfo, (VOS_SIZE_T)sizeof(stAppInfo), 0x00, (VOS_SIZE_T)sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));

    ulRslt = TAF_ACORE_NV_READ(enModemId,
                               en_NV_Item_Usim_App_Priority_Cfg,
                               &stAppInfo,
                               sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));

    if (ulRslt != NV_OK)
    {
        AT_ERR_LOG("AT_SetPrfApp: Get en_NV_Item_Usim_App_Priority_Cfg fail.");

        return AT_ERROR;
    }

    /* ��ֹNV�����쳣����Խ����� */
    if (stAppInfo.ucAppNum > AT_ARRAY_SIZE(stAppInfo.aenAppList))
    {
        AT_WARN_LOG("AT_SetPrfApp: Get en_NV_Item_Usim_App_Priority_Cfg success, but ucAppNum invalid.");

        return AT_ERROR;
    }

    /* ����CDMAӦ������ */
    for (i = 0; i < stAppInfo.ucAppNum; i++)
    {
        if (enCardAPP == stAppInfo.aenAppList[i])
        {
            ulAppHit      = VOS_TRUE;
            ulAppOrderPos = i;
            break;
        }
    }

    if (ulAppHit == VOS_FALSE)
    {
        /* û���ҵ������뵽��ǰ�� */
        if (stAppInfo.ucAppNum >= USIMM_NV_CARDAPP_BUTT)
        {
            ulRslt = AT_ERROR;
        }
        else
        {
            VOS_MemMove_s((VOS_VOID *)&stAppInfo.aenAppList[1],
                        sizeof(stAppInfo.aenAppList) - 1 * sizeof(stAppInfo.aenAppList[1]),
                        (VOS_VOID *)&stAppInfo.aenAppList[0],
                        (sizeof(VOS_UINT32) * stAppInfo.ucAppNum));

            stAppInfo.aenAppList[0] = enCardAPP;
            stAppInfo.ucAppNum ++;

            ulRslt = TAF_ACORE_NV_WRITE(enModemId,
                                        en_NV_Item_Usim_App_Priority_Cfg,
                                        &stAppInfo,
                                        sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));
        }
    }
    else
    {
        if (ulAppOrderPos != 0)
        {
            /* �ӵ�һ���������ƶ�i��*/
            VOS_MemMove_s((VOS_VOID *)&stAppInfo.aenAppList[1],
                        sizeof(stAppInfo.aenAppList) - 1 * sizeof(stAppInfo.aenAppList[1]),
                        (VOS_VOID *)&stAppInfo.aenAppList[0],
                        (sizeof(VOS_UINT32) * ulAppOrderPos));

            stAppInfo.aenAppList[0] = enCardAPP;

            ulRslt = TAF_ACORE_NV_WRITE(enModemId,
                                        en_NV_Item_Usim_App_Priority_Cfg,
                                        &stAppInfo,
                                        sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));
        }
    }

    if (ulRslt != NV_OK)
    {
        AT_ERR_LOG("TAF_ACORE_NV_WRITE: Write NV Fail.");
        return AT_ERROR;
    }

    return AT_OK;
}


TAF_UINT32 At_SetPrfAppPara(TAF_UINT8 ucIndex)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    TAF_UINT32                          ulRslt = AT_OK;

    /* �������� */
    if (gucAtParaIndex > 1)
    {
        AT_ERR_LOG("At_SetPrfAppPara: too many para");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����AT����ͨ���ŵõ�MODEM ID */
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetRATCombinePara: Get modem id fail.");

        return AT_ERROR;
    }

    if (gastAtParaList[0].ulParaValue == AT_PREFER_APP_CDMA)
    {
        ulRslt = AT_SetPrfApp(AT_PREFER_APP_CDMA, USIMM_NV_CDMA_APP, enModemId);
    }
    else
    {
        ulRslt = AT_SetPrfApp(AT_PREFER_APP_GUTL, USIMM_NV_GUTL_APP, enModemId);
    }

    return ulRslt;
}


TAF_UINT32 At_QryPrfAppPara(TAF_UINT8 ucIndex)
{
    TAF_UINT32                          i;
    TAF_UINT32                          ulCdmaHit;
    TAF_UINT32                          ulGutlHit;
    USIMM_APP_PRIORITY_CONFIG_STRU      stAppInfo;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT16                          usLength;

    /* ����AT����ͨ���ŵõ�MODEM ID */
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("At_QryPrfAppPara: Get modem id fail.");

        return AT_ERROR;
    }

    memset_s(&stAppInfo, (VOS_SIZE_T)sizeof(stAppInfo), 0x00, (VOS_SIZE_T)sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));

    if (TAF_ACORE_NV_READ(enModemId, en_NV_Item_Usim_App_Priority_Cfg, &stAppInfo, sizeof(USIMM_APP_PRIORITY_CONFIG_STRU)) != NV_OK)
    {
        AT_ERR_LOG("At_QryPrfAppPara: Get en_NV_Item_Usim_App_Priority_Cfg fail.");

        return AT_ERROR;
    }

    /* ��ֹNV�����쳣����Խ����� */
    if (stAppInfo.ucAppNum > AT_ARRAY_SIZE(stAppInfo.aenAppList))
    {
        AT_WARN_LOG("At_QryPrfAppPara: Get en_NV_Item_Usim_App_Priority_Cfg success, but ucAppNum invalid.");

        stAppInfo.ucAppNum = AT_ARRAY_SIZE(stAppInfo.aenAppList);
    }

    ulCdmaHit = VOS_FALSE;
    ulGutlHit = VOS_FALSE;

    /* ����CDMA��GUTLӦ����NV���е�λ�� */
    for (i = 0; i < stAppInfo.ucAppNum; i++)
    {
        if (stAppInfo.aenAppList[i] == USIMM_NV_GUTL_APP)
        {
            ulGutlHit = VOS_TRUE;

            break;
        }

        if (stAppInfo.aenAppList[i] == USIMM_NV_CDMA_APP)
        {
            ulCdmaHit = VOS_TRUE;

            break;
        }
    }

    if (ulGutlHit == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: 1",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        gstAtSendData.usBufLen = usLength;

        return AT_OK;
    }
    else if (ulCdmaHit == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: 0",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        gstAtSendData.usBufLen = usLength;

        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}


TAF_UINT32 At_TestPrfAppPara(TAF_UINT8 ucIndex)
{
    VOS_UINT16      usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: (0,1)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


TAF_UINT32 AT_SetUiccPrfApp(
    AT_CARDAPP_ENUM_UINT32              enCardApp,
    USIMM_NV_CARDAPP_ENUM_UINT32        enCardAPP,
    MODEM_ID_ENUM_UINT16                enModemId)
{
    USIMM_APP_PRIORITY_CONFIG_STRU      stAppInfo;
    TAF_UINT32                          ulAppHit      = 0;
    TAF_UINT32                          ulAppOrderPos = 0;
    TAF_UINT32                          ulRslt;
    TAF_UINT32                          i;

    memset_s(&stAppInfo, (VOS_SIZE_T)sizeof(stAppInfo), 0x00, (VOS_SIZE_T)sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));

    ulRslt = TAF_ACORE_NV_READ(enModemId,
                               en_NV_Item_Usim_Uicc_App_Priority_Cfg,
                               &stAppInfo,
                               sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));

    if (ulRslt != NV_OK)
    {
        AT_ERR_LOG("AT_SetUiccPrfApp: Get en_NV_Item_Usim_Uicc_App_Priority_Cfg fail.");

        return AT_ERROR;
    }

    /* ��ֹNV�����쳣����Խ����� */
    if (stAppInfo.ucAppNum > AT_ARRAY_SIZE(stAppInfo.aenAppList))
    {
        AT_WARN_LOG("AT_SetUiccPrfApp: Get en_NV_Item_Usim_Uicc_App_Priority_Cfg success, but ucAppNum invalid.");

        return AT_ERROR;
    }

    /* ����CDMAӦ������ */
    for (i = 0; i < stAppInfo.ucAppNum; i++)
    {
        if (enCardAPP == stAppInfo.aenAppList[i])
        {
            ulAppHit      = VOS_TRUE;
            ulAppOrderPos = i;
            break;
        }
    }

    if (ulAppHit == VOS_FALSE)
    {
        /* û���ҵ������뵽��ǰ�� */
        if (stAppInfo.ucAppNum >= USIMM_NV_CARDAPP_BUTT)
        {
            ulRslt = AT_ERROR;
        }
        else
        {
            (VOS_VOID)VOS_MemMove_s((VOS_VOID *)&stAppInfo.aenAppList[1],
                                     sizeof(stAppInfo.aenAppList) - 1 * sizeof(stAppInfo.aenAppList[1]),
                                     (VOS_VOID *)&stAppInfo.aenAppList[0],
                                     (sizeof(VOS_UINT32) * stAppInfo.ucAppNum));

            stAppInfo.aenAppList[0] = enCardAPP;
            stAppInfo.ucAppNum ++;

            ulRslt = TAF_ACORE_NV_WRITE(enModemId,
                                        en_NV_Item_Usim_Uicc_App_Priority_Cfg,
                                        &stAppInfo,
                                        sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));
        }
    }
    else
    {
        if (ulAppOrderPos != 0)
        {
            /* �ӵ�һ���������ƶ�i��*/
            (VOS_VOID)VOS_MemMove_s((VOS_VOID *)&stAppInfo.aenAppList[1],
                                     sizeof(stAppInfo.aenAppList) - 1 * sizeof(stAppInfo.aenAppList[1]),
                                     (VOS_VOID *)&stAppInfo.aenAppList[0],
                                     (sizeof(VOS_UINT32) * ulAppOrderPos));

            stAppInfo.aenAppList[0] = enCardAPP;

            ulRslt = TAF_ACORE_NV_WRITE(enModemId,
                                        en_NV_Item_Usim_Uicc_App_Priority_Cfg,
                                        &stAppInfo,
                                        sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));
        }
    }

    if (ulRslt != NV_OK)
    {
        AT_ERR_LOG("TAF_ACORE_NV_WRITE: Write NV Fail.");
        return AT_ERROR;
    }

    return AT_OK;
}


TAF_UINT32 At_SetUiccPrfAppPara(TAF_UINT8 ucIndex)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    TAF_UINT32                          ulRslt = AT_OK;

    /* �������� */
    if (gucAtParaIndex > 1)
    {
        AT_ERR_LOG("At_SetUiccPrfAppPara: too many para");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����AT����ͨ���ŵõ�MODEM ID */
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("At_SetUiccPrfAppPara: Get modem id fail.");

        return AT_ERROR;
    }

    if (gastAtParaList[0].ulParaValue == AT_PREFER_APP_CDMA)
    {
        ulRslt = AT_SetUiccPrfApp(AT_PREFER_APP_CDMA, USIMM_NV_CDMA_APP, enModemId);
    }
    else
    {
        ulRslt = AT_SetUiccPrfApp(AT_PREFER_APP_GUTL, USIMM_NV_GUTL_APP, enModemId);
    }

    return ulRslt;
}


TAF_UINT32 At_QryUiccPrfAppPara(TAF_UINT8 ucIndex)
{
    TAF_UINT32                          i;
    TAF_UINT32                          ulCdmaHit;
    TAF_UINT32                          ulGutlHit;
    USIMM_APP_PRIORITY_CONFIG_STRU      stAppInfo;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT16                          usLength;

    /* ����AT����ͨ���ŵõ�MODEM ID */
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("At_QryUiccPrfAppPara: Get modem id fail.");

        return AT_ERROR;
    }

    memset_s(&stAppInfo, (VOS_SIZE_T)sizeof(stAppInfo), 0x00, (VOS_SIZE_T)sizeof(USIMM_APP_PRIORITY_CONFIG_STRU));

    if (TAF_ACORE_NV_READ(enModemId, en_NV_Item_Usim_Uicc_App_Priority_Cfg, &stAppInfo, sizeof(USIMM_APP_PRIORITY_CONFIG_STRU)) != NV_OK)
    {
        AT_ERR_LOG("At_QryUiccPrfAppPara: Get en_NV_Item_Usim_Uicc_App_Priority_Cfg fail.");

        return AT_ERROR;
    }

    /* ��ֹNV�����쳣����Խ����� */
    if (stAppInfo.ucAppNum > AT_ARRAY_SIZE(stAppInfo.aenAppList))
    {
        AT_WARN_LOG("At_QryUiccPrfAppPara: Get en_NV_Item_Usim_Uicc_App_Priority_Cfg success, but ucAppNum invalid.");

        stAppInfo.ucAppNum = AT_ARRAY_SIZE(stAppInfo.aenAppList);
    }

    ulCdmaHit = VOS_FALSE;
    ulGutlHit = VOS_FALSE;

    /* ����CDMA��GUTLӦ����NV����˭���� */
    for (i = 0; i < stAppInfo.ucAppNum; i++)
    {
        if (stAppInfo.aenAppList[i] == USIMM_NV_GUTL_APP)
        {
            ulGutlHit = VOS_TRUE;

            break;
        }

        if (stAppInfo.aenAppList[i] == USIMM_NV_CDMA_APP)
        {
            ulCdmaHit = VOS_TRUE;

            break;
        }
    }

    if (ulGutlHit == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: 1",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        gstAtSendData.usBufLen = usLength;

        return AT_OK;
    }
    else if (ulCdmaHit == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: 0",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        gstAtSendData.usBufLen = usLength;

        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}


TAF_UINT32 At_TestUiccPrfAppPara(TAF_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: (0,1)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

TAF_UINT32 At_SetCCimiPara(TAF_UINT8 ucIndex)
{
    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ִ��������� */
    if (SI_PIH_CCimiSetReq(gastAtClientTab[ucIndex].usClientId,0) == AT_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCIMI_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
}
#endif

TAF_UINT16 At_CardErrorInfoInd(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent)
{
    VOS_UINT16                          usLength = 0;
    VOS_UINT32                          i;

    if (pstEvent->PIHEvent.UsimmErrorInd.ulErrNum == VOS_NULL)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s^USIMMEX: NULL\r\n",
                                          gaucAtCrLf);

        return usLength;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s^USIMMEX: ",
                                      gaucAtCrLf);

    for(i=0; i<pstEvent->PIHEvent.UsimmErrorInd.ulErrNum; i++)
    {
        if (i == 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "0x%X",
                                          pstEvent->PIHEvent.UsimmErrorInd.aulErrInfo[i]);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          ",0x%X",
                                          pstEvent->PIHEvent.UsimmErrorInd.aulErrInfo[i]);
        }
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr+ usLength,
                                      "%s",
                                      gaucAtCrLf);

    return usLength;
}


TAF_UINT16 At_CardIccidInfoInd(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT16                          usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s^USIMICCID: ",
                                      gaucAtCrLf);

    usLength += (TAF_UINT16)AT_Hex2AsciiStrLowHalfFirst(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                        (TAF_UINT8 *)pstEvent->PIHEvent.aucIccidContent,
                                                        USIMM_ICCID_FILE_LEN);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr+ usLength,
                                      "%s",
                                      gaucAtCrLf);

    return usLength;
}


TAF_UINT16 At_CardStatusInd(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT16                          usLength = 0;
    VOS_UINT16                          usTmpLen;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s^CARDSTATUSIND: ",
                                      gaucAtCrLf);

    /* ��״̬���ʱ��������Ϣͷ����Ϣ���� */
    usTmpLen = (VOS_MSG_HEAD_LENGTH + sizeof(TAF_UINT32));

    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (TAF_INT8 *)pgucAtSndCodeAddr,
                                                    (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                    (((TAF_UINT8 *)&pstEvent->PIHEvent.stCardStatusInd) + usTmpLen),
                                                    (sizeof(USIMM_CARDSTATUS_IND_STRU) - usTmpLen));

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s",
                                      gaucAtCrLf);

    return usLength;
}


TAF_UINT16 At_SimHotPlugStatusInd(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT16                          usLength = 0;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^SIMHOTPLUG: %d%s",
                                       gaucAtCrLf,
                                       pstEvent->PIHEvent.ulSimHotPlugStatus,
                                       gaucAtCrLf);
    return usLength;
}


TAF_UINT16 At_PrintPrivateCglaResult(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent)
{
    VOS_UINT16                          usLength = 0;

    /* �����һ����ӡ�ϱ�IND����Ҫ��������ӻس����� */
    if (pstEvent->PIHEvent.stCglaHandleCnf.ucLastDataFlag != VOS_TRUE)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr, "\r\n");
    }

    /* ^CGLA: <flag>,<length>,"[<command>]<SW1><SW2>" */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: %d,%d,\"",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstEvent->PIHEvent.stCglaHandleCnf.ucLastDataFlag,
                                       (pstEvent->PIHEvent.stCglaHandleCnf.usLen + 2) * 2);

    if(pstEvent->PIHEvent.stCglaHandleCnf.usLen != 0)
    {
        /* <command>, */
        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + usLength, pstEvent->PIHEvent.stCglaHandleCnf.aucCommand, pstEvent->PIHEvent.stCglaHandleCnf.usLen);
    }

    /* <SW1> */
    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + usLength, &pstEvent->PIHEvent.stCglaHandleCnf.ucSW1, (VOS_UINT16)sizeof(TAF_UINT8));

    /* <SW2> */
    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + usLength, &pstEvent->PIHEvent.stCglaHandleCnf.ucSW2, (VOS_UINT16)sizeof(TAF_UINT8));
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "\"");

    return usLength;
}


TAF_UINT16 At_SWCheckStatusInd(
    SI_PIH_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT16                          usLength = 0;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       "%s^SWCHECK: %d%s",
                                       gaucAtCrLf,
                                       pstEvent->PIHEvent.ulApduSWCheckResult,
                                       gaucAtCrLf);
    return usLength;
}


