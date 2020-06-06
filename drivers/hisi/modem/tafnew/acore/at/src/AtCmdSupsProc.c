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
#include "ATCmdProc.h"
#include "AtCmdSupsProc.h"
#include "TafSsaApi.h"
#include "TafStdlib.h"
#include "AtEventReport.h"
#include "securec.h"


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_SUPS_PROC_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
const AT_SS_EVT_FUNC_TBL_STRU           g_astAtSsEvtFuncTbl[] =
{
    /* �¼�ID */                                /* ��Ϣ������ */
    {ID_TAF_SSA_SET_LCS_MOLR_CNF,               AT_RcvSsaSetLcsMolrCnf},
    {ID_TAF_SSA_GET_LCS_MOLR_CNF,               AT_RcvSsaGetLcsMolrCnf},
    {ID_TAF_SSA_LCS_MOLR_NTF,                   AT_RcvSsaLcsMolrNtf},
    {ID_TAF_SSA_SET_LCS_MTLR_CNF,               AT_RcvSsaSetLcsMtlrCnf},
    {ID_TAF_SSA_GET_LCS_MTLR_CNF,               AT_RcvSsaGetLcsMtlrCnf},
    {ID_TAF_SSA_LCS_MTLR_NTF,                   AT_RcvSsaLcsMtlrNtf},
    {ID_TAF_SSA_SET_LCS_MTLRA_CNF,              AT_RcvSsaSetLcsMtlraCnf},
    {ID_TAF_SSA_GET_LCS_MTLRA_CNF,              AT_RcvSsaGetLcsMtlraCnf},
};


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

VOS_UINT32 AT_ConvertTafSsaErrorCode(
    VOS_UINT8                           ucIndex,
    TAF_ERROR_CODE_ENUM_UINT32          enErrorCode)
{
    VOS_UINT32                          ulResult;

    if (enErrorCode == TAF_ERR_NO_ERROR)
    {
        return AT_OK;
    }

    ulResult    = AT_ERROR;

    if ( (enErrorCode >= TAF_ERR_LCS_BASE)
      && (enErrorCode <= TAF_ERR_LCS_UNKNOWN_ERROR) )
    {
        ulResult = enErrorCode - TAF_ERR_LCS_BASE + AT_CMOLRE_ERR_ENUM_BEGIN + 1;
        return ulResult;
    }

    ulResult = At_ChgTafErrorCode(ucIndex, enErrorCode);

    return ulResult;
}


VOS_VOID AT_FillCmolrParaEnable(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    /* <enable> */
    if ( (gucAtParaIndex == 0)
      || (gastAtParaList[0].usParaLen == 0) )
    {
        pstMolrPara->enEnable = TAF_SSA_LCS_MOLR_ENABLE_TYPE_DISABLE;
    }
    else
    {
        pstMolrPara->enEnable = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    }

    return;
}


VOS_VOID AT_FillCmolrParaMethod(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 1)
      || (gastAtParaList[1].usParaLen == 0) )
    {
        return;
    }

    /* <method> */
    pstMolrPara->bitOpMethod    = VOS_TRUE;
    pstMolrPara->enMethod       = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    return;
}


VOS_UINT32 AT_FillCmolrParaHorAcc(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 2)
      || (gastAtParaList[2].usParaLen == 0) )
    {
        return VOS_OK;
    }

    /* <hor-acc-set> */
    pstMolrPara->bitOpHorAccSet = VOS_TRUE;
    pstMolrPara->enHorAccSet    = (VOS_UINT8)gastAtParaList[2].ulParaValue;

    if (pstMolrPara->enHorAccSet == LCS_HOR_ACC_SET_PARAM)
    {
        /* <hor-acc> */
        /* Ҫ��ˮƽ���ȣ���û���·�ˮƽ����ϵ��������ʧ�� */
        if ( (gucAtParaIndex <= 3)
          || (gastAtParaList[3].usParaLen == 0) )
        {
            AT_WARN_LOG("AT_FillCmolrParaPartI: <hor-acc> is required!");
            return VOS_ERR;
        }

        pstMolrPara->ucHorAcc   = (VOS_UINT8)gastAtParaList[3].ulParaValue;
    }

    return VOS_OK;
}


VOS_UINT32 AT_FillCmolrParaVerReq(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 4)
      || (gastAtParaList[4].usParaLen == 0) )
    {
        return VOS_OK;
    }

    /* <ver-req> */
    pstMolrPara->bitOpVerReq    = VOS_TRUE;
    pstMolrPara->enVerReq       = (VOS_UINT8)gastAtParaList[4].ulParaValue;

    if ( (pstMolrPara->enVerReq != LCS_VER_REQUESTED)
      || (gucAtParaIndex <= 5)
      || (gastAtParaList[5].usParaLen == 0) )
    {
        return VOS_OK;
    }

    /* <ver-acc-set> */
    pstMolrPara->bitOpVerAccSet = VOS_TRUE;
    pstMolrPara->enVerAccSet    = (VOS_UINT8)gastAtParaList[5].ulParaValue;

    if (pstMolrPara->enVerAccSet == LCS_VER_ACC_SET_PARAM)
    {
        /* <ver-acc> */
        /* Ҫ��ֱ���ȣ���û���·���ֱ����ϵ��������ʧ�� */
        if ( (gucAtParaIndex <= 6)
          || (gastAtParaList[6].usParaLen == 0) )
        {
            AT_WARN_LOG("AT_FillCmolrParaVerReq: <ver-acc> is required!");
            return VOS_ERR;
        }

        pstMolrPara->ucVerAcc   = (VOS_UINT8)gastAtParaList[6].ulParaValue;
    }

    return VOS_OK;
}


VOS_VOID AT_FillCmolrParaVelReq(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 7)
      || (gastAtParaList[7].usParaLen == 0))
    {
        return;
    }

    /* <vel-req> */
    pstMolrPara->bitOpVelReq    = VOS_TRUE;
    pstMolrPara->enVelReq       = (VOS_UINT8)gastAtParaList[7].ulParaValue;

    return;
}


VOS_UINT32 AT_FillCmolrParaRepMode(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 8)
      || (gastAtParaList[8].usParaLen == 0) )
    {
        return VOS_OK;
    }

    /* <rep-mode> */
    pstMolrPara->bitOpRepMode   = VOS_TRUE;
    pstMolrPara->enRepMode      = (VOS_UINT8)gastAtParaList[8].ulParaValue;

    if (gucAtParaIndex <= 9)
    {
        return VOS_OK;
    }

    /* <timeout> */
    if (gastAtParaList[9].usParaLen != 0)
    {
        pstMolrPara->bitOpTimeout   = VOS_TRUE;
        pstMolrPara->usTimeOut      = (VOS_UINT16)gastAtParaList[9].ulParaValue;
    }

    /* <interval> */
    if (pstMolrPara->enRepMode == LCS_REP_MODE_PERIODIC_RPT)
    {
        if ( (gucAtParaIndex > 10)
          && (gastAtParaList[10].usParaLen != 0) )
        {
            pstMolrPara->bitOpInterval  = VOS_TRUE;
            pstMolrPara->usInterval     = (VOS_UINT16)gastAtParaList[10].ulParaValue;
        }
    }

    if ( (pstMolrPara->bitOpTimeout == VOS_TRUE)
      && (pstMolrPara->bitOpInterval == VOS_TRUE)
      && (pstMolrPara->usInterval < pstMolrPara->usTimeOut) )
    {
        AT_WARN_LOG("AT_FillCmolrParaPartI: <interval> is too short!");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID AT_FillCmolrParaShapeRep(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 11)
      || (gastAtParaList[11].usParaLen == 0) )
    {
        return;
    }

    /* <shape-rep> */
    pstMolrPara->bitOpShapeRep  = VOS_TRUE;
    pstMolrPara->ucShapeRep     = (VOS_UINT8)gastAtParaList[11].ulParaValue;

    return;
}


VOS_VOID AT_FillCmolrParaPlane(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 12)
      || (gastAtParaList[12].usParaLen == 0) )
    {
        return;
    }

    /* <plane> */
    pstMolrPara->bitOpPlane     = VOS_TRUE;
    pstMolrPara->enPlane        = (VOS_UINT8)gastAtParaList[12].ulParaValue;

    return;
}


VOS_UINT32 AT_Str2NmeaRep(
    VOS_UINT16                          usLength,
    VOS_CHAR                           *pcStr,
    TAF_SSA_LCS_NMEA_REP_STRU          *pstNmeaRep
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulNum;
    VOS_UINT32                          ulLoop;
    VOS_CHAR                           *pcTemp  = VOS_NULL_PTR;
    VOS_CHAR                            aucSrcStr[AT_PARA_NMEA_MAX_LEN + 1];

    memset_s(aucSrcStr, sizeof(aucSrcStr), 0x00, sizeof(aucSrcStr));
    memset_s(pstNmeaRep, sizeof(TAF_SSA_LCS_NMEA_REP_STRU), 0x00, sizeof(TAF_SSA_LCS_NMEA_REP_STRU));

    if (usLength > AT_PARA_NMEA_MAX_LEN)
    {
        AT_WARN_LOG("AT_Str2NmeaRep: NMEA format string is too long!");
        return VOS_ERR;
    }

    ulNum = (usLength + 1) / (AT_PARA_NMEA_MIN_LEN + 1);

    if (ulNum == 0)
    {
        AT_WARN_LOG("AT_Str2NmeaRep: NMEA format string is too short!");
        return VOS_ERR;
    }

    lMemResult = memcpy_s(aucSrcStr, sizeof(aucSrcStr), pcStr, usLength);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucSrcStr), usLength);
    pcTemp = aucSrcStr;

    for (ulLoop = 0; ulLoop < ulNum; ulLoop++)
    {
        /* $GPGGA */
        if (VOS_MemCmp(pcTemp, AT_PARA_NMEA_GPGGA, AT_PARA_NMEA_MIN_LEN) == 0)
        {
            pstNmeaRep->bitGPGGA = VOS_TRUE;
        }
        /* $GPRMC */
        else if (VOS_MemCmp(pcTemp, AT_PARA_NMEA_GPRMC, AT_PARA_NMEA_MIN_LEN) == 0)
        {
            pstNmeaRep->bitGPRMC = VOS_TRUE;
        }
        /* $GPGLL */
        else if (VOS_MemCmp(pcTemp, AT_PARA_NMEA_GPGLL, AT_PARA_NMEA_MIN_LEN) == 0)
        {
            pstNmeaRep->bitGPGLL = VOS_TRUE;
        }
        else
        {
            AT_WARN_LOG("AT_Str2NmeaRep: NMEA format string is incorrect!");
            return VOS_ERR;
        }

        pcTemp += (AT_PARA_NMEA_MIN_LEN + 1);
    }

    return VOS_OK;
}


VOS_UINT32 AT_FillCmolrParaNmeaRep(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    if ( (gucAtParaIndex <= 13)
      || (gastAtParaList[13].usParaLen == 0) )
    {
        return VOS_OK;
    }

    /* <NMEA-rep> */
    if ( (pstMolrPara->enEnable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA)
      || (pstMolrPara->enEnable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA_GAD) )
    {
        pstMolrPara->bitOpNmeaRep = VOS_TRUE;
        if (AT_Str2NmeaRep(gastAtParaList[13].usParaLen,
                                     (VOS_CHAR *)gastAtParaList[13].aucPara,
                                     &(pstMolrPara->stNmeaRep)) != VOS_OK)
        {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_FillCmolrParaThdPtyAddr(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    errno_t                             lMemResult;
    if ( (gucAtParaIndex <= 14)
      || (gastAtParaList[14].usParaLen == 0) )
    {
        return VOS_OK;
    }

    /* <third-party-address> */
    if (gastAtParaList[14].usParaLen > TAF_SSA_LCS_THIRD_PARTY_ADDR_MAX_LEN)
    {
        AT_WARN_LOG("AT_FillCmolrParaPartIII: <third-party-address> is too long!");
        return VOS_ERR;
    }

    pstMolrPara->bitOpThirdPartyAddr = VOS_TRUE;
    lMemResult = memcpy_s(pstMolrPara->acThirdPartyAddr,
                          sizeof(pstMolrPara->acThirdPartyAddr),
                          (VOS_CHAR *)gastAtParaList[14].aucPara,
                          gastAtParaList[14].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstMolrPara->acThirdPartyAddr), gastAtParaList[14].usParaLen);

    return VOS_OK;
}


VOS_UINT32 AT_FillCmolrPara(
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    memset_s(pstMolrPara, sizeof(TAF_SSA_LCS_MOLR_PARA_SET_STRU), 0x00, sizeof(TAF_SSA_LCS_MOLR_PARA_SET_STRU));

    /* +CMOLR=[<enable>[,<method>[,<hor-acc-set>[,<hor-acc>[,<ver-req>
               [,<ver-acc-set>[,<ver-acc>[,<vel-req> [,<rep-mode>[,<timeout>
               [,<interval>[,<shape-rep>[,<plane>[,<NMEA-rep>
               [,<third-party-address>]]]]]]]]]]]]]]]] */

    /* �������࣬����ʧ�� */
    if (gucAtParaIndex > 15)
    {
        AT_WARN_LOG("AT_FillCmolrPara: too many parameters!");
        return VOS_ERR;
    }

    AT_FillCmolrParaEnable(pstMolrPara);

    AT_FillCmolrParaMethod(pstMolrPara);

    if (AT_FillCmolrParaHorAcc(pstMolrPara) != VOS_OK)
    {
        return VOS_ERR;
    }

    if (AT_FillCmolrParaVerReq(pstMolrPara) != VOS_OK)
    {
        return VOS_ERR;
    }

    AT_FillCmolrParaVelReq(pstMolrPara);

    if (AT_FillCmolrParaRepMode(pstMolrPara) != VOS_OK)
    {
        return VOS_ERR;
    }

    AT_FillCmolrParaShapeRep(pstMolrPara);

    AT_FillCmolrParaPlane(pstMolrPara);

    if (AT_FillCmolrParaNmeaRep(pstMolrPara) != VOS_OK)
    {
        return VOS_ERR;
    }

    if (AT_FillCmolrParaThdPtyAddr(pstMolrPara) != VOS_OK)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetCmolrPara(VOS_UINT8 ucIndex)
{
    TAF_SSA_LCS_MOLR_PARA_SET_STRU      stMolrPara;

    memset_s(&stMolrPara, sizeof(stMolrPara), 0x00, sizeof(stMolrPara));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetCmolrPara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_FillCmolrPara(&stMolrPara) != VOS_OK)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ִ��������� */
    TAF_SSA_SetCmolrInfo(WUEPS_PID_AT,
                         gastAtClientTab[ucIndex].usClientId,
                         gastAtClientTab[ucIndex].opId,
                         &stMolrPara);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMOLR_SET;

    /* ��������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID AT_RcvSsaSetLcsMolrCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_SET_LCS_MOLR_CNF_STRU      *pstSetLcsMolrCnf    = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMolrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CMOLR_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMOLR_SET)
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMolrCnf: WARNING:Not AT_CMD_CMOLR_SET!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstSetLcsMolrCnf    = (TAF_SSA_SET_LCS_MOLR_CNF_STRU *)pEvent;
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstSetLcsMolrCnf->enResult);

    if (ulResult != AT_OK)
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMolrCnf: WARNING:Set ^CMOLR Failed!");
    }

    /* ������ */
    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_UINT32 AT_QryCmolrPara(VOS_UINT8 ucIndex)
{
    /* ִ�в�ѯ���� */
    TAF_SSA_GetCmolrInfo(WUEPS_PID_AT,
                         gastAtClientTab[ucIndex].usClientId,
                         gastAtClientTab[ucIndex].opId);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMOLR_READ;

    /* ��������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID AT_RcvSsaGetLcsMolrCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_GET_LCS_MOLR_CNF_STRU      *pstGetLcsMolrCnf    = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_BOOL                            bNmeaFlg;
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucTPAStr[LCS_CLIENT_EXID_MAX_LEN + 1];

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaGetLcsMolrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CMOLR_READ */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMOLR_READ)
    {
        AT_WARN_LOG("AT_RcvSsaGetLcsMolrCnf: WARNING:Not AT_CMD_CMOLR_READ!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstGetLcsMolrCnf    = (TAF_SSA_GET_LCS_MOLR_CNF_STRU *)pEvent;
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstGetLcsMolrCnf->enResult);
    bNmeaFlg            = VOS_FALSE;
    usLength            = 0;

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (ulResult == AT_OK)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d,%d,%d,",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstGetLcsMolrCnf->enEnable,
                                          pstGetLcsMolrCnf->stMolrPara.enMethod,
                                          pstGetLcsMolrCnf->stMolrPara.enHorAccSet);

        if (pstGetLcsMolrCnf->stMolrPara.enHorAccSet == LCS_HOR_ACC_SET_PARAM)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d",
                                               pstGetLcsMolrCnf->stMolrPara.ucHorAcc);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d,",
                                           pstGetLcsMolrCnf->stMolrPara.enVerReq);

        if (pstGetLcsMolrCnf->stMolrPara.enVerReq == LCS_VER_REQUESTED)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d,",
                                               pstGetLcsMolrCnf->stMolrPara.enVerAccSet);

            if (pstGetLcsMolrCnf->stMolrPara.enVerAccSet == LCS_VER_ACC_SET_PARAM)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%d",
                                                   pstGetLcsMolrCnf->stMolrPara.ucVerAcc);
            }
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",");
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d,%d,%d,",
                                           pstGetLcsMolrCnf->stMolrPara.enVelReq,
                                           pstGetLcsMolrCnf->stMolrPara.enRepMode,
                                           pstGetLcsMolrCnf->stMolrPara.usTimeOut);

        if (pstGetLcsMolrCnf->stMolrPara.enRepMode == LCS_REP_MODE_PERIODIC_RPT)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d",
                                               pstGetLcsMolrCnf->stMolrPara.usInterval);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d,%d,",
                                           pstGetLcsMolrCnf->stMolrPara.u.ucShapeRep,
                                           pstGetLcsMolrCnf->enPlane);

        if ( (pstGetLcsMolrCnf->enEnable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA)
          || (pstGetLcsMolrCnf->enEnable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA_GAD) )
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"");

            if (pstGetLcsMolrCnf->stNmeaRep.bitGPGGA == VOS_TRUE)
            {
                bNmeaFlg  = VOS_TRUE;
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s,",
                                                   AT_PARA_NMEA_GPGGA);
            }

            if (pstGetLcsMolrCnf->stNmeaRep.bitGPRMC == VOS_TRUE)
            {
                bNmeaFlg  = VOS_TRUE;
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s,",
                                                   AT_PARA_NMEA_GPRMC);
            }

            if (pstGetLcsMolrCnf->stNmeaRep.bitGPGLL == VOS_TRUE)
            {
                bNmeaFlg  = VOS_TRUE;
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s,",
                                                   AT_PARA_NMEA_GPGLL);
            }
            /* ɾ�����ӡ��һ���ַ� */
            usLength -= 1;

            if (bNmeaFlg == VOS_TRUE)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "\"");
            }
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");

        if ( ((pstGetLcsMolrCnf->stMolrPara.enMethod == LCS_MOLR_METHOD_TRANSFER_TP_ADDR)
           || (pstGetLcsMolrCnf->stMolrPara.enMethod == LCS_MOLR_METHOD_RETRIEVAL_TP_ADDR))
          && ((pstGetLcsMolrCnf->stMolrPara.stTPAddr.ucLength > 0)
           && (pstGetLcsMolrCnf->stMolrPara.stTPAddr.ucLength <= LCS_CLIENT_EXID_MAX_LEN)) )
        {
            lMemResult = memset_s(aucTPAStr, sizeof(aucTPAStr), 0x00, LCS_CLIENT_EXID_MAX_LEN + 1);
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucTPAStr), LCS_CLIENT_EXID_MAX_LEN + 1);
            lMemResult = memcpy_s(aucTPAStr,
                                  sizeof(aucTPAStr),
                                  pstGetLcsMolrCnf->stMolrPara.stTPAddr.aucValue,
                                  pstGetLcsMolrCnf->stMolrPara.stTPAddr.ucLength);
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucTPAStr), pstGetLcsMolrCnf->stMolrPara.stTPAddr.ucLength);

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s\"",
                                               aucTPAStr);
        }

    }

    /* ������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_UINT32 AT_TestCmolrPara(VOS_UINT8 ucIndex)
{
    /* ������������� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                "%s: (0-3),(0-6),(0,1),(0-127),(0,1),(0,1),(0-127),(0-4),"
                                "(0,1),(1-65535),(1-65535),(1-127),(0,1),"
                                /* "\"$GPGSA,$GPGGA,$GPGSV,$GPRMC,$GPVTG,$GPGLL\"," */
                                "\"$GPGGA,$GPRMC,$GPGLL\","
                                "\"<third-party-address>\"",     /* <third-party-address>�����ݲ�ȷ����ʽ */
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_VOID AT_RcvSsaLcsMolrNtf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_LCS_MOLR_NTF_STRU          *pstLcsMolrNtf   = VOS_NULL_PTR;
    AT_MODEM_AGPS_CTX_STRU             *pstAgpsCtx      = VOS_NULL_PTR;
    VOS_CHAR                           *pcLocationStr   = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulReturnCodeIndex;
    VOS_UINT16                          usLength;

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaLcsMolrNtf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    pstLcsMolrNtf       = (TAF_SSA_LCS_MOLR_NTF_STRU *)pEvent;
    pstAgpsCtx          = AT_GetModemAgpsCtxAddrFromClientId(gastAtClientTab[ucIndex].usClientId);
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstLcsMolrNtf->enResult);
    ulReturnCodeIndex   = 0;
    usLength            = 0;

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (ulResult == AT_OK)
    {
        pcLocationStr = (VOS_CHAR*)PS_MEM_ALLOC(WUEPS_PID_AT, pstLcsMolrNtf->usLocationStrLen + 1);
        if (pcLocationStr == VOS_NULL_PTR)
        {
            AT_ERR_LOG("AT_RcvSsaLcsMolrNtf: Alloc Memory Failed!");
            return;
        }

        VOS_StrNCpy_s(pcLocationStr, pstLcsMolrNtf->usLocationStrLen + 1, pstLcsMolrNtf->acLocationStr, pstLcsMolrNtf->usLocationStrLen);
        pcLocationStr[pstLcsMolrNtf->usLocationStrLen] = '\0';

        if (pstLcsMolrNtf->enRptTypeChoice == TAF_SSA_LCS_MOLR_RPT_NMEA)
        {
            /* ���+CMOLRN��λ��Ϣ */
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s%s\"%s\"%s",
                                              gaucAtCrLf,
                                              gastAtStringTab[AT_STRING_CMOLRN].pucText,
                                              pcLocationStr,
                                              gaucAtCrLf);
        }
        else
        {
            /* ���+CMOLRG��λ��Ϣ */
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s%s%s%s",
                                              gaucAtCrLf,
                                              gastAtStringTab[AT_STRING_CMOLRG].pucText,
                                              pcLocationStr,
                                              gaucAtCrLf);
        }

        PS_MEM_FREE(WUEPS_PID_AT, pcLocationStr);
    }
    else
    {
        if (AT_GetReturnCodeId(ulResult, &ulReturnCodeIndex) != VOS_OK)
        {
            AT_ERR_LOG("AT_RcvSsaLcsMolrNtf: result code index is err!");
            return;
        }

        /* ���+CMOLRE������ */
        if (pstAgpsCtx->enCmolreType == AT_CMOLRE_NUMERIC)
        {
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s%s%s%s",
                                              gaucAtCrLf,
                                              gastAtStringTab[AT_STRING_CMOLRE].pucText,
                                              (VOS_CHAR *)gastAtReturnCodeTab[ulReturnCodeIndex].Result[0],
                                              gaucAtCrLf);
        }
        else
        {
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s%s%s%s",
                                              gaucAtCrLf,
                                              gastAtStringTab[AT_STRING_CMOLRE].pucText,
                                              (VOS_CHAR *)gastAtReturnCodeTab[ulReturnCodeIndex].Result[1],
                                              gaucAtCrLf);
        }
    }

    /* ��������ATͨ�� */
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_UINT32 AT_SetCmolrePara(VOS_UINT8 ucIndex)
{
    AT_MODEM_AGPS_CTX_STRU             *pstAgpsCtx = VOS_NULL_PTR;

    pstAgpsCtx = AT_GetModemAgpsCtxAddrFromClientId(gastAtClientTab[ucIndex].usClientId);

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetCmolrePara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������࣬����ʧ�� */
    if (gucAtParaIndex > 1)
    {
        AT_WARN_LOG("AT_SetCmolrePara: too many parameters!");
        return AT_TOO_MANY_PARA;
    }

    /* ִ��������� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        pstAgpsCtx->enCmolreType = AT_CMOLRE_NUMERIC;
    }
    else
    {
        pstAgpsCtx->enCmolreType = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    }

    /* �����OK */
    return AT_OK;
}


VOS_UINT32 AT_QryCmolrePara(VOS_UINT8 ucIndex)
{
    AT_MODEM_AGPS_CTX_STRU             *pstAgpsCtx = VOS_NULL_PTR;

    pstAgpsCtx = AT_GetModemAgpsCtxAddrFromClientId(gastAtClientTab[ucIndex].usClientId);

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                "%s: %d",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                pstAgpsCtx->enCmolreType);

    /* �����OK */
    return AT_OK;
}


VOS_UINT32 AT_SetCmtlrPara(VOS_UINT8 ucIndex)
{
    TAF_SSA_LCS_MTLR_SUBSCRIBE_ENUM_UINT8   enSubscribe;

    /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetCmtlrPara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������࣬����ʧ�� */
    if (gucAtParaIndex > 1)
    {
        AT_WARN_LOG("AT_SetCmtlrPara: too many parameters!");
        return AT_TOO_MANY_PARA;
    }

    /* ִ��������� */
    if (gastAtParaList[0].usParaLen == 0)
    {
        enSubscribe = TAF_SSA_LCS_MOLR_ENABLE_TYPE_DISABLE;
    }
    else
    {
        enSubscribe = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    }
    TAF_SSA_SetCmtlrInfo(WUEPS_PID_AT,
                         gastAtClientTab[ucIndex].usClientId,
                         gastAtClientTab[ucIndex].opId,
                         enSubscribe);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMTLR_SET;

    /* ��������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID AT_RcvSsaSetLcsMtlrCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_SET_LCS_MTLR_CNF_STRU      *pstSetLcsMtlrCnf    = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CMTLR_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMTLR_SET)
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlrCnf: WARNING:Not AT_CMD_CMTLR_SET!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstSetLcsMtlrCnf    = (TAF_SSA_SET_LCS_MTLR_CNF_STRU *)pEvent;
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstSetLcsMtlrCnf->enResult);

    if (ulResult != AT_OK)
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlrCnf: WARNING:Set ^CMTLR Failed!");
    }

    /* ������ */
    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_UINT32 AT_QryCmtlrPara(VOS_UINT8 ucIndex)
{
    /* ִ�в�ѯ���� */
    TAF_SSA_GetCmtlrInfo(WUEPS_PID_AT,
                         gastAtClientTab[ucIndex].usClientId,
                         gastAtClientTab[ucIndex].opId);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMTLR_READ;

    /* ��������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID AT_RcvSsaGetLcsMtlrCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_GET_LCS_MTLR_CNF_STRU      *pstGetLcsMtlrCnf    = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CMTLR_READ */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMTLR_READ)
    {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlrCnf: WARNING:Not AT_CMD_CMTLR_READ!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstGetLcsMtlrCnf    = (TAF_SSA_GET_LCS_MTLR_CNF_STRU *)pEvent;
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstGetLcsMtlrCnf->enResult);
    usLength            = 0;

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (ulResult == AT_OK)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstGetLcsMtlrCnf->enSubscribe);
    }

    /* ������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_VOID AT_RcvSsaLcsMtlrNtf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_LCS_MTLR_NTF_STRU          *pstLcsMtlrNtf   = VOS_NULL_PTR;
    VOS_UINT32                          ulTmpStrLen;
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    /* ֻ��Ϊ�㲥ͨ�� */
    if (!AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaLcsMtlrNtf: WARNING:Not AT_BROADCAST_INDEX!");
        return;
    }

    pstLcsMtlrNtf       = (TAF_SSA_LCS_MTLR_NTF_STRU *)pEvent;
    usLength            = 0;

    /* +CMTLR: <handle-id>,<notification-type>,<location-type>,
               [<client-external-id>],[<client-name>][,<plane>] */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s%s%d,%d,%d,",
                                      gaucAtCrLf,
                                      gastAtStringTab[AT_STRING_CMTLR].pucText,
                                      pstLcsMtlrNtf->stMtlrPara.ucHandleId,
                                      pstLcsMtlrNtf->stMtlrPara.enNtfType,
                                      pstLcsMtlrNtf->stMtlrPara.enLocationType);

    if (pstLcsMtlrNtf->stMtlrPara.bitOpClientExId == VOS_TRUE)
    {
        ulTmpStrLen = pstLcsMtlrNtf->stMtlrPara.stClientExId.ucLength <= LCS_CLIENT_EXID_MAX_LEN ?
                        pstLcsMtlrNtf->stMtlrPara.stClientExId.ucLength : LCS_CLIENT_EXID_MAX_LEN;

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "\"");

        for (i = 0; i < ulTmpStrLen; i++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%02X",
                                      pstLcsMtlrNtf->stMtlrPara.stClientExId.aucValue[i]);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "\"");

    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      ",");

    if (pstLcsMtlrNtf->stMtlrPara.bitOpClientName == VOS_TRUE)
    {
        ulTmpStrLen = pstLcsMtlrNtf->stMtlrPara.stClientName.ucLength <= LCS_CLIENT_NAME_MAX_LEN ?
                        pstLcsMtlrNtf->stMtlrPara.stClientName.ucLength : LCS_CLIENT_NAME_MAX_LEN;

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "\"");

        for (i = 0; i < ulTmpStrLen; i++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%02X",
                                      pstLcsMtlrNtf->stMtlrPara.stClientName.aucValue[i]);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "\"");
    }

    if (pstLcsMtlrNtf->stMtlrPara.bitOpPlane == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          ",%d",
                                          pstLcsMtlrNtf->stMtlrPara.enPlane);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "\r\n");

    /* ��������ATͨ�� */
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_UINT32 AT_SetCmtlraPara(VOS_UINT8 ucIndex)
{
    TAF_SSA_LCS_MTLRA_PARA_SET_STRU     stCmtlraPara;

    /* ������� */
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetCmtlraPara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���������ж� */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("AT_SetCmtlraPara: Incorrect Parameter Num!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ִ��������� */
    memset_s(&stCmtlraPara, sizeof(stCmtlraPara), 0x00, sizeof(stCmtlraPara));
    stCmtlraPara.enAllow    = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stCmtlraPara.ucHandleId = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    TAF_SSA_SetCmtlraInfo(WUEPS_PID_AT,
                          gastAtClientTab[ucIndex].usClientId,
                          gastAtClientTab[ucIndex].opId,
                          &stCmtlraPara);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMTLRA_SET;

    /* ��������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID AT_RcvSsaSetLcsMtlraCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_SET_LCS_MTLRA_CNF_STRU     *pstSetLcsMtlraCnf    = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlraCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CMTLRA_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMTLRA_SET)
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlraCnf: WARNING:Not AT_CMD_CMTLRA_SET!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstSetLcsMtlraCnf   = (TAF_SSA_SET_LCS_MTLRA_CNF_STRU *)pEvent;
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstSetLcsMtlraCnf->enResult);

    if (ulResult != AT_OK)
    {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlraCnf: WARNING:Set ^CMTLRA Failed!");
    }

    /* ������ */
    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_UINT32 AT_QryCmtlraPara(VOS_UINT8 ucIndex)
{
    /* ִ�в�ѯ���� */
    TAF_SSA_GetCmtlraInfo(WUEPS_PID_AT,
                          gastAtClientTab[ucIndex].usClientId,
                          gastAtClientTab[ucIndex].opId);

    /* ���õ�ǰ�������� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMTLRA_READ;

    /* ��������״̬ */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID AT_RcvSsaGetLcsMtlraCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvent
)
{
    TAF_SSA_GET_LCS_MTLRA_CNF_STRU     *pstGetLcsMtlraCnf    = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           i;
    VOS_UINT8                           ucCnt;

    /* ����Ϊ�㲥ͨ�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlraCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CMTLRA_READ */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMTLRA_READ)
    {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlraCnf: WARNING:Not AT_CMD_CMTLRA_READ!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstGetLcsMtlraCnf   = (TAF_SSA_GET_LCS_MTLRA_CNF_STRU *)pEvent;
    ulResult            = AT_ConvertTafSsaErrorCode(ucIndex, pstGetLcsMtlraCnf->enResult);
    usLength            = 0;

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (ulResult == AT_OK)
    {
        ucCnt = (pstGetLcsMtlraCnf->ucCnt > TAF_SSA_LCS_MTLR_MAX_NUM) ?
                    TAF_SSA_LCS_MTLR_MAX_NUM : pstGetLcsMtlraCnf->ucCnt;
        for (i = 0;  i < ucCnt; i++)
        {
            if (i != 0)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s",
                                                   (VOS_CHAR *)gaucAtCrLf);
            }

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s: %d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstGetLcsMtlraCnf->aenAllow[i],
                                               pstGetLcsMtlraCnf->aucHandleId[i]);

        }
    }

    /* ������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_UINT32 AT_TestCmtlraPara(VOS_UINT8 ucIndex)
{
    /* ������������� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                "%s: (0,1)",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_VOID AT_RcvTafSsaEvt(
    TAF_SSA_EVT_STRU                   *pstEvent
)
{
    MN_AT_IND_EVT_STRU                 *pMsg        = VOS_NULL_PTR;
    TAF_CTRL_STRU                      *pstCtrl     = VOS_NULL_PTR;
    AT_SS_EVT_FUNC                      pEvtFunc    = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          i;

    /* �ж��¼��Ƿ�����ʷSSA�����ϱ� */
    if (pstEvent->ulEvtExt != 0)
    {
        pMsg = (MN_AT_IND_EVT_STRU *)pstEvent;
        At_SsMsgProc(pMsg->aucContent, pMsg->usLen);
        return;
    }

    /* ��ʼ�� */
    pstCtrl     = (TAF_CTRL_STRU*)(pstEvent->aucContent);

    if ( At_ClientIdToUserId(pstCtrl->usClientId,
                                           &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvTafSsaEvt: At_ClientIdToUserId FAILURE");
        return;
    }

    /* ���¼�������в��Ҵ����� */
    for (i = 0; i < AT_ARRAY_SIZE(g_astAtSsEvtFuncTbl); i++)
    {
        if ( pstEvent->enEvtId == g_astAtSsEvtFuncTbl[i].enEvtId )
        {
            /* �¼�IDƥ�� */
            pEvtFunc = g_astAtSsEvtFuncTbl[i].pEvtFunc;
            break;
        }
    }

    /* ������������������ */
    if ( pEvtFunc != VOS_NULL_PTR )
    {
        pEvtFunc(ucIndex, pstEvent->aucContent);
    }
    else
    {
        AT_ERR_LOG1("AT_RcvTafSsaEvt: Unexpected event received! <EvtId>", pstEvent->enEvtId);
    }

    return;
}


VOS_UINT32 AT_QryCnapExPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRet;
    TAF_CTRL_STRU                       stCtrl;
    MODEM_ID_ENUM_UINT16                enModemId;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* ���Ϳ����ϢID_TAF_CCM_CNAP_QRY_REQ��C�� */
    ulRet = TAF_CCM_CallCommonReq(&stCtrl,
                                  VOS_NULL_PTR,
                                  ID_TAF_CCM_CNAP_QRY_REQ,
                                  0,
                                  enModemId);

    if (ulRet != VOS_OK)
    {
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CNAPEX_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


LOCAL VOS_VOID AT_CnapConvertNameStr(
    TAF_CALL_CNAP_STRU                 *pstNameIndicator,
    VOS_UINT8                          *pucNameStr,
    VOS_UINT8                           ucNameStrMaxLen
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulAscIILen;

    memset_s(pucNameStr, ucNameStrMaxLen, 0x00, ucNameStrMaxLen);
    ulAscIILen = 0;

    /* 7bitת��Ϊ8bit */
    if (pstNameIndicator->enDcs == TAF_CALL_DCS_7BIT)
    {
        /* 7bitת��Ϊ8bit */
        if (ucNameStrMaxLen < pstNameIndicator->ucLength)
        {
            AT_WARN_LOG("AT_CnapConvertNameStr: NameStr Space Too Short!");
            return;
        }

        if (TAF_STD_UnPack7Bit(pstNameIndicator->aucNameStr,
                                         pstNameIndicator->ucLength,
                                         0,
                                         pucNameStr) != VOS_OK)
        {
            AT_WARN_LOG("AT_CnapConvertNameStr: TAF_STD_UnPack7Bit Err!");
            return;
        }

        /* Default Alphabetת��ΪASCII�� */
        TAF_STD_ConvertDefAlphaToAscii(pucNameStr, pstNameIndicator->ucLength, pucNameStr, &ulAscIILen);

        if (ulAscIILen < pstNameIndicator->ucLength)
        {
            AT_WARN_LOG("AT_CnapConvertNameStr: TAF_STD_ConvertDefAlphaToAscii Err!");
            return;
        }
    }
    else
    {
        lMemResult = memcpy_s(pucNameStr,
                              ucNameStrMaxLen,
                              pstNameIndicator->aucNameStr,
                              TAF_MIN(pstNameIndicator->ucLength, TAF_CALL_CNAP_NAME_STR_MAX_LENGTH));
        TAF_MEM_CHK_RTN_VAL(lMemResult, ucNameStrMaxLen, TAF_MIN(pstNameIndicator->ucLength, TAF_CALL_CNAP_NAME_STR_MAX_LENGTH));
    }

    return;
}


VOS_UINT32 AT_RcvTafCcmCnapQryCnf(VOS_VOID *pMsg)
{
    TAF_CCM_CNAP_QRY_CNF_STRU          *pstCnapQryCnf = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           aucNameStr[AT_PARA_CNAP_MAX_NAME_LEN];

    ucIndex         = 0;
    usLength        = 0;

    pstCnapQryCnf   = (TAF_CCM_CNAP_QRY_CNF_STRU *)pMsg;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstCnapQryCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallCnapQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �㲥��Ϣ������ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCnapQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CNAPEX_QRY)
    {
        AT_WARN_LOG("AT_RcvTafCallCnapQryCnf: WARNING:Not AT_CMD_CNAPEX_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCnapQryCnf->stNameIndicator.ucExistFlag == VOS_TRUE)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  "%s: \"",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        if ((pstCnapQryCnf->stNameIndicator.ucLength > 0)
         && (pstCnapQryCnf->stNameIndicator.ucLength < TAF_CALL_CNAP_NAME_STR_MAX_LENGTH)
         && (pstCnapQryCnf->stNameIndicator.enDcs < TAF_CALL_DCS_BUTT))
        {
            /* UCS2��ʽֱ�Ӵ�ӡ��� */
            if (pstCnapQryCnf->stNameIndicator.enDcs == TAF_CALL_DCS_UCS2)
            {
                usLength += (VOS_UINT16)At_Unicode2UnicodePrint(AT_CMD_MAX_LEN,
                                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                                pgucAtSndCodeAddr + usLength,
                                                                pstCnapQryCnf->stNameIndicator.aucNameStr,
                                                                pstCnapQryCnf->stNameIndicator.ucLength * 2);
            }
            else
            {
                AT_CnapConvertNameStr(&pstCnapQryCnf->stNameIndicator, aucNameStr, AT_PARA_CNAP_MAX_NAME_LEN);

                /* AscIIת��ΪUCS2����ӡ��� */
                usLength += (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN,
                                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                                              pgucAtSndCodeAddr + usLength,
                                                              aucNameStr,
                                                              (VOS_UINT16)VOS_StrLen((VOS_CHAR *)aucNameStr));
            }
        }

        /* <CNI_Validity> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "\",%d",
                                  pstCnapQryCnf->stNameIndicator.enCniValidity);

    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}



VOS_UINT32 AT_SetCnapPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* �������� */
    if (gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������Ϊ�� */
    if (gastAtParaList[0].usParaLen != 0)
    {
        pstSsCtx->enCnapType = (AT_CNAP_TYPE_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    }
    else
    {
        pstSsCtx->enCnapType = AT_CNAP_DISABLE_TYPE;
    }

    return AT_OK;
}


TAF_UINT32 AT_QryCnapPara(VOS_UINT8 ucIndex)
{
    TAF_SS_INTERROGATESS_REQ_STRU       stCnapQry;

    /* ��ʼ�� */
    memset_s(&stCnapQry, (VOS_SIZE_T)sizeof(stCnapQry), 0x00, (VOS_SIZE_T)sizeof(TAF_SS_INTERROGATESS_REQ_STRU));

    /* ������� */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����SsCode */
    stCnapQry.SsCode = TAF_CNAP_SS_CODE;

    if(TAF_InterrogateSSReq(gastAtClientTab[ucIndex].usClientId, 0, &stCnapQry) == AT_SUCCESS)
    {
        /* ���õ�ǰ�������� */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CNAP_QRY;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬ */
    }
    else
    {
        return AT_ERROR;
    }
}


TAF_VOID AT_SsRspInterrogateCnfCnapProc(
    TAF_UINT8                                               ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU                     *pEvent,
    TAF_UINT32                                             *pulResult,
    TAF_UINT16                                             *pusLength
)
{
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;
    TAF_UINT8                           ucTmp    = 0;

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    /* ��Ҫ�����жϴ����� */
    if (pEvent->OP_Error == 1)
    {
        *pulResult = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);
        return;
    }

    /* �жϵ�ǰ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CNAP_QRY)
    {
        AT_WARN_LOG("AT_SsRspInterrogateCnfCnapProc: WARNING: Not AT_CMD_CNAP_QRY!");
        return;
    }

    /* �鵽״̬ */
    if (pEvent->OP_SsStatus == 1)
    {
        ucTmp = (TAF_SS_PROVISIONED_STATUS_MASK & pEvent->SsStatus) ? 1 : 0;
    }
    /* û�в鵽״̬ */
    else
    {
        ucTmp = 2;
    }

    /* +CNAP: <n>,<m> */
    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (TAF_CHAR *)pgucAtSndCodeAddr,
                                         (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s: %d,%d",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                         pstSsCtx->enCnapType,
                                         ucTmp);

    *pulResult = AT_OK;
    return;
}


VOS_VOID AT_ReportCnapInfo(
    VOS_UINT8                           ucIndex,
    TAF_CALL_CNAP_STRU                 *pstNameIndicator
)
{
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;
    VOS_UINT8                           aucDstName[AT_PARA_CNAP_MAX_NAME_LEN];

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);
    memset_s(aucDstName, (VOS_SIZE_T)sizeof(aucDstName), 0, AT_PARA_CNAP_MAX_NAME_LEN);

    if ((pstSsCtx->enCnapType != AT_CNAP_ENABLE_TYPE)
     || (pstNameIndicator->ucExistFlag == VOS_FALSE))
    {
        return;
    }

    /* 7bit��8bitת�����ϱ� */
    if ((pstNameIndicator->enDcs == TAF_CALL_DCS_7BIT)
     || (pstNameIndicator->enDcs == TAF_CALL_DCS_8BIT))
    {
        AT_CnapConvertNameStr(pstNameIndicator, aucDstName, AT_PARA_CNAP_MAX_NAME_LEN);
    }
    else
    {
        AT_NORM_LOG1("AT_ReportCnapInfo: Dcs:", pstNameIndicator->enDcs);
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s+CNAP: \"%s\",%d%s",
                                      gaucAtCrLf,
                                      (VOS_CHAR *)aucDstName,
                                      pstNameIndicator->enCniValidity,
                                      gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    return;
}

VOS_UINT32 AT_RcvTafCcmCnapInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CNAP_INFO_IND_STRU         *pstCnapInfoInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pstCnapInfoInd = (TAF_CCM_CNAP_INFO_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCnapInfoInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallCnapInfoInd:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    AT_ReportCnapInfo(ucIndex, &pstCnapInfoInd->stNameIndicator);

    return VOS_OK;
}


