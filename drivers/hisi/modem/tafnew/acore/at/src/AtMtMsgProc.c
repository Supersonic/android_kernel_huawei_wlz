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
#include "AtMtInterface.h"
#include "AtTypeDef.h"
#include "MnClient.h"
#include "ATCmdProc.h"
#include "AtMtMsgProc.h"
#include "AtMtCommFun.h"
#include "securec.h"



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_MT_MSG_PROC_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#if(FEATURE_ON == FEATURE_UE_MODE_NR)
const AT_PROC_BBIC_MSG_STRU g_astAtProcBbicMsgTab[]=
{
    {ID_BBIC_CAL_AT_MT_TX_CNF             , AT_ProcTxCnf},
    {ID_BBIC_CAL_AT_MT_RX_CNF             , AT_ProcRxCnf},
    {ID_BBIC_AT_CAL_MSG_CNF               , AT_ProcGsmTxCnf_ModulateWave},
    {ID_BBIC_AT_GSM_TX_PD_CNF             , AT_ProcPowerDetCnf_ModulateWave},
    {ID_BBIC_CAL_AT_MT_TX_PD_IND          , AT_ProcPowerDetCnf},
    {ID_BBIC_AT_MIPI_READ_CNF             , AT_RcvBbicCalMipiRedCnf},
    {ID_BBIC_AT_MIPI_WRITE_CNF            , AT_RcvBbicCalMipiWriteCnf},
    {ID_BBIC_AT_PLL_QRY_CNF               , At_RcvBbicPllStatusCnf},
    {ID_BBIC_CAL_AT_MT_RX_RSSI_IND        , At_RcvBbicRssiInd},
    {ID_BBIC_AT_DPDT_CNF                  , AT_RcvBbicCalSetDpdtCnf},
    {ID_BBIC_AT_TEMP_QRY_CNF              , AT_RcvBbicCalQryFtemprptCnf},
    {ID_BBIC_AT_DCXO_CNF                  , At_RcvBbicCalDcxoCnf},
    {ID_BBIC_AT_TRX_TAS_CNF               , AT_RcvBbicCalSetTrxTasCnf}
};


const AT_PROC_CBT_MSG_STRU g_astAtProcCbtMsgTab[]=
{
    {ID_CCBT_AT_SET_WORK_MODE_CNF, AT_ProcSetWorkModeCnf},
};

const AT_PROC_UECBT_MSG_STRU g_astAtProcUeCbtMsgTab[]=
{
    {ID_UECBT_AT_RFIC_MEM_TEST_CNF,     At_ProcUeCbtRfIcMemTestCnf},
    {ID_UECBT_AT_RFIC_DIE_IE_QUERY_CNF, At_RcvUeCbtRfIcIdExQryCnf},
};

const AT_PROC_DSP_IDLE_MSG_STRU g_astAtProcDspIdleMsgTab[]=
{
    {ID_PHY_AT_SERDES_AGING_TEST_CNF,     At_ProcDspIdleSerdesRtCnf},
};


#endif

#if(FEATURE_ON == FEATURE_UE_MODE_NR)
extern AT_MT_INFO_STRU                         g_stMtInfoCtx;
#else
extern AT_DEVICE_CMD_CTRL_STRU                 g_stAtDevCmdCtrl;
#endif

#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
/*****************************************************************************
  3 �ɺ���ʵ��
*****************************************************************************/


VOS_UINT32  At_SendContinuesWaveOnToHPA(
    VOS_UINT16                          usPower,
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU             *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;
    VOS_UINT8                           ucCtrlType;

    ucCtrlType = WDSP_CTRL_TX_ONE_TONE;

    /* �򿪵��� */
    if (g_stAtDevCmdCtrl.enCltEnableFlg == AT_DSP_CLT_ENABLE)
    {
        ucCtrlType = WDSP_CTRL_TX_THREE_TONE;
    }

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength        = ulLength;

    /* ��д��Ϣ�� */
    pstMsg->usMsgID = ID_AT_HPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask =  W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_POWER;
    pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)usPower;

    /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usMask      = usMask;
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;

    /* �򿪵����ź� */
    pstMsg->stRfCfgPara.usTxEnable = ucCtrlType;

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32  At_SendContinuesWaveOnToCHPA(
    VOS_UINT8                           ucCtrlType,
    VOS_UINT16                          usPower
)
{
    AT_CHPA_RF_CFG_REQ_STRU            *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_CHPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg = (AT_CHPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_CHPA_RF_CFG_REQ_STRU), 0x00 ,sizeof(AT_CHPA_RF_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UPHY_PID_CSDR_1X_CM;
    pstMsg->ulLength        = ulLength;

    /* ��д��Ϣ�� */
    pstMsg->usMsgID = ID_AT_CHPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask =  W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_POWER;
    pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)usPower;

    /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usMask      = usMask;
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;

    /* �򿪵����ź� */
    pstMsg->stRfCfgPara.usTxEnable = ucCtrlType;

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToCHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32  At_SendTxOnOffToCHPA(VOS_UINT8 ucSwitch)
{
    AT_CHPA_RF_CFG_REQ_STRU            *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_CHPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg = (AT_CHPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendTxOnOffToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_CHPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_CHPA_RF_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UPHY_PID_CSDR_1X_CM;
    pstMsg->ulLength        = ulLength;

    /* ��д��Ϣ�� */
    pstMsg->usMsgID = ID_AT_CHPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask =  W_RF_MASK_AFC | W_RF_MASK_TX_ARFCN \
             | W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_PAMODE |W_RF_MASK_TX_POWDET;

    usMask                       = usMask | W_RF_MASK_TX_POWER;
    pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)g_stAtDevCmdCtrl.usPower;

    /* Tx Cfg */
    pstMsg->stRfCfgPara.usMask      = usMask;                             /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;              /* AFC */
    pstMsg->stRfCfgPara.usTxBand    = 0;                             /* g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;     1,2,3...,Э���е�band���,ע�ⲻ��BandId */
    pstMsg->stRfCfgPara.usTxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn; /* Arfcn */
    pstMsg->stRfCfgPara.usTxPAMode  = g_stAtDevCmdCtrl.ucPaLevel;

    if (ucSwitch == AT_DSP_RF_SWITCH_ON)
    {
        pstMsg->stRfCfgPara.usTxEnable = WDSP_CTRL_TX_ON;               /* �򿪷���TX */
    }
    else
    {
        pstMsg->stRfCfgPara.usTxEnable = WDSP_CTRL_TX_OFF;               /* �رշ���TX */
    }

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendTxOnOffToCHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32 At_SendRxOnOffToCHPA(
    VOS_UINT32                          ulRxSwitch
)
{
    AT_CHPA_RF_CFG_REQ_STRU            *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_CHPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_CHPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendRxOnOffToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_CHPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_CHPA_RF_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UPHY_PID_CSDR_1X_CM;
    pstMsg->ulLength        = ulLength;

    /* ��д��Ϣ�� */
    pstMsg->usMsgID = ID_AT_CHPA_RF_CFG_REQ;

    usMask = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL \
             | W_RF_MASK_RX_RXONOFF;

    /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usMask      = usMask;

    /* 1,2,3...,Э���е�band��� */
    pstMsg->stRfCfgPara.usRxBand    = 0;  /* g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;*/
    pstMsg->stRfCfgPara.usRxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn;

    pstMsg->stRfCfgPara.usRxAntSel = ANT_ONE;

    if (ulRxSwitch == AT_DSP_RF_SWITCH_ON)
    {
        /* CPU���ƴ���ƵоƬ,����1 */
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ANT1_ON;
    }
    else
    {
        /* �رս���RX */
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_OFF;
    }

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendRxOnOffToCHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

#endif


TAF_UINT32  At_SendTxOnOffToHPA(
    TAF_UINT8                           ucSwitch,
    TAF_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU             *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength    = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    usMask      = 0;

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendTxOnOffToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength        = ulLength;

    /* ��д��Ϣ�� */
    pstMsg->usMsgID = ID_AT_HPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask = W_RF_MASK_AFC | W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_PAMODE | W_RF_MASK_TX_POWDET;

    /*AT^FDAC���õ�FDACֵ��AT^FWAVE���õ�powerֵ��ʾ�ĺ�����ͬ��ȡ�����õ�ֵ*/
    if ( g_stAtDevCmdCtrl.bFdacFlag == VOS_TRUE )
    {
        usMask                      = usMask | W_RF_MASK_TX_AGC;
        pstMsg->stRfCfgPara.usTxAGC = g_stAtDevCmdCtrl.usFDAC;
    }
    else
    {
        usMask                       = usMask | W_RF_MASK_TX_POWER;
        pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)g_stAtDevCmdCtrl.usPower;
    }

    /* Tx Cfg */
    pstMsg->stRfCfgPara.usMask      = usMask;                             /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;              /* AFC */
    pstMsg->stRfCfgPara.usTxBand    = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;    /* 1,2,3...,Э���е�band���,ע�ⲻ��BandId */
    pstMsg->stRfCfgPara.usTxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn; /* Arfcn */
    pstMsg->stRfCfgPara.usTxPAMode  = g_stAtDevCmdCtrl.ucPaLevel;

    if (ucSwitch == AT_DSP_RF_SWITCH_OFF)
    {
        pstMsg->stRfCfgPara.usTxEnable  = WDSP_CTRL_TX_OFF;                     /* �رշ���TX */
    }
    else
    {
        pstMsg->stRfCfgPara.usTxEnable  = WDSP_CTRL_TX_ON;                      /* �򿪷���TX */
        pstMsg->stRfCfgPara.usMask      |= W_RF_MASK_TX_ARFCN;

        /* ֻ����^FTXON=1 ʱ���·� */
        if (g_stAtDevCmdCtrl.enCltEnableFlg == AT_DSP_CLT_ENABLE)
        {
            pstMsg->stRfCfgPara.usMask |= W_RF_MASK_TX_CLT;
            pstMsg->stRfCfgPara.usMask &= ~(W_RF_MASK_TX_POWDET);
        }
        if ( g_stAtDevCmdCtrl.enDcxoTempCompEnableFlg == AT_DCXOTEMPCOMP_ENABLE)
        {
            pstMsg->stRfCfgPara.usMask |= W_RF_MASK_TX_DCXOTEMPCOMP;
            pstMsg->stRfCfgPara.usMask &= ~(W_RF_MASK_AFC);
        }
    }

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendTxOnOffToHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32  At_SendTxOnOffToGHPA(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucSwitch
)
{
    AT_GHPA_RF_TX_CFG_REQ_STRU          *pstTxMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_GHPA_RF_TX_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_GHPA_RF_TX_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    pstTxMsg = (AT_GHPA_RF_TX_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (pstTxMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendRfCfgReqToGHPA: alloc Tx msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstTxMsg, sizeof(AT_GHPA_RF_TX_CFG_REQ_STRU), 0x00, sizeof(AT_GHPA_RF_TX_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstTxMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstTxMsg->ulSenderPid     = WUEPS_PID_AT;
    pstTxMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstTxMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    pstTxMsg->ulLength        = ulLength;

    /* Tx Cfg */

    /* ��д��Ϣ�� */
    pstTxMsg->usMsgID = ID_AT_GHPA_RF_TX_CFG_REQ;
    usMask            = G_RF_MASK_TX_AFC;

    /*AT^FDAC���õ�FDACֵ��AT^FWAVE���õ�powerֵ��ʾ�ĺ�����ͬ��ȡ�����õ�ֵ*/
    /* ������Ʒ�ʽ��
        0��GMSK��ѹ����,�˷�ʽ��usTxVpaҪ���ã�
        1�����ʿ���,�˷�ʽ��usTxPowerҪ���ã�
        2��8PSK PaVbias��ѹ&DBB Atten&RF Atten���ƣ�
        usPAVbias��usRfAtten,sDbbAtten����������Ҫ���ã�*/
    if ( g_stAtDevCmdCtrl.bFdacFlag == VOS_TRUE )
    {
        pstTxMsg->enCtrlMode = AT_GHPA_RF_CTRLMODE_TYPE_GMSK;
        pstTxMsg->usTxVpa = g_stAtDevCmdCtrl.usFDAC;  /* DAC���� */
    }
    else
    {
        pstTxMsg->enCtrlMode = AT_GHPA_RF_CTRLMODE_TYPE_TXPOWER;
        pstTxMsg->usTxPower = g_stAtDevCmdCtrl.usPower;
    }

    /* Tx Cfg */
    pstTxMsg->usMask = usMask;              /* ��λ��ʶ�������� */
    pstTxMsg->usAFC =  0;                   /* AFC */
    pstTxMsg->usFreqNum =
                (VOS_UINT16)((g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand << 12) \
                            | g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn);
                                            /* (Band << 12) | Arfcn */
    pstTxMsg->usTxMode = g_stAtDevCmdCtrl.usTxMode;    /* 0:burst����; 1:�������� */
    pstTxMsg->usModType = G_MOD_TYPE_GMSK;  /* ������Ʒ�ʽ:0��ʾGMSK����;1��ʾ8PSK���Ʒ�ʽ */

    if (ucSwitch == AT_DSP_RF_SWITCH_ON)
    {
        pstTxMsg->usTxEnable = GDSP_CTRL_TX_ON;
    }
    else
    {
        pstTxMsg->usTxEnable = GDSP_CTRL_TX_OFF;
    }

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstTxMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendTxOnOffToGHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32  At_SendRxOnOffToGHPA(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulRxSwitch
)
{
    AT_GHPA_RF_RX_CFG_REQ_STRU          *pstRxMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_GHPA_RF_RX_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_GHPA_RF_RX_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstRxMsg = (AT_GHPA_RF_RX_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstRxMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendRxOnOffToGHPA: alloc Rx msg fail!");
        return AT_FAILURE;
    }

    /* Rx Cfg */
    pstRxMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    pstRxMsg->ulLength        = ulLength;
    pstRxMsg->usMsgID         = ID_AT_GHPA_RF_RX_CFG_REQ;
    pstRxMsg->usRsv           = 0;
    pstRxMsg->usRsv2          = 0;

    usMask = G_RF_MASK_RX_ARFCN | G_RF_MASK_RX_AGCMODE \
            | G_RF_MASK_RX_AGCGAIN | G_RF_MASK_RX_MODE;

    /* Reference MASK_CAL_RF_G_RX_* section */
    pstRxMsg->usMask = usMask;

    /* (Band << 12) | Arfcn */
    pstRxMsg->usFreqNum =
                    (VOS_UINT16)((g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand << 12) \
                                | g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn);

    /* 0:ֹͣ; 1:��������; */
    if ( ulRxSwitch == TRUE )
    {
        pstRxMsg->usRxMode  = AT_GDSP_RX_MODE_CONTINOUS_BURST;
    }
    else
    {
        pstRxMsg->usRxMode  = AT_GDSP_RX_MODE_STOP;
    }

    /* 1:Fast AGC,0:Slow AGC */
    pstRxMsg->usAGCMode = AT_GDSP_RX_SLOW_AGC_MODE;

    /* AGC��λ�����ĵ�,ȡֵΪ0-3*/
    pstRxMsg->usAgcGain = g_stAtDevCmdCtrl.ucLnaLevel;

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstRxMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendRxOnOffToGHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 At_SendRxOnOffToHPA(
    VOS_UINT32                          ulRxSwitch,
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU              *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendRxOnOffToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));

    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength        = ulLength;

    /* ��д��Ϣ�� */
    pstMsg->usMsgID = ID_AT_HPA_RF_CFG_REQ;

    usMask = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL \
             | W_RF_MASK_RX_RXONOFF;

    /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usMask      = usMask;

    /* 1,2,3...,Э���е�band��� */
    pstMsg->stRfCfgPara.usRxBand    = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->stRfCfgPara.usRxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usDlArfcn;

    /* DSPƵ��Band1-Band9��ʽת��Ϊ
       W Rf support
    -------------------------------------------------------------------------------
            bit8       bit7      bit6     bit5    bit4     bit3      bit2     bit1
    -------------------------------------------------------------------------------
    WCDMA   900(VIII)  2600(VII) 800(VI)  850(V)  1700(IV) 1800(III) 1900(II) 2100(I) oct1
            spare      spare     spare    spare   spare    spare     spare   J1700(IX)oct2
    */

    if (g_stAtDevCmdCtrl.ucPriOrDiv == AT_RX_DIV_ON)
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_TWO;
    }
    else
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_ONE;
    }

    /* LNAģʽ���� */
    pstMsg->stRfCfgPara.usRxLNAGainMode = g_stAtDevCmdCtrl.ucLnaLevel;

    if (ulRxSwitch == AT_DSP_RF_SWITCH_ON)
    {
        if (pstMsg->stRfCfgPara.usRxAntSel == ANT_TWO)
        {
            /* CPU���ƴ���ƵоƬ,����1��2 */
            pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ALL_ANT_ON;
        }
        else
        {
            /* CPU���ƴ���ƵоƬ,����1 */
            pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ANT1_ON;
        }
    }
    else
    {
        /* �رս���RX */
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_OFF;
    }

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendRxOnOffToHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 At_SendRfCfgAntSelToHPA(
    VOS_UINT8                           ucDivOrPriOn,
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU              *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                           ulLength;
    VOS_UINT16                           usMask;

    /* ����AT_HPA_RF_CFG_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    pstMsg   = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("At_SendRfCfgAntSelToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    memset_s(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));


    /* ��д��Ϣͷ */
    pstMsg->ulSenderCpuId      = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid        = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId    = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid      = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength           = ulLength;


    /* ��д��Ϣ�� */
    pstMsg->usMsgID            = ID_AT_HPA_RF_CFG_REQ;
    usMask                     = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL \
                                 | W_RF_MASK_RX_RXONOFF;

    /* ��λ��ʶ�������� */
    pstMsg->stRfCfgPara.usMask = usMask;

    if (ucDivOrPriOn == AT_RX_DIV_ON)
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_TWO;
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ALL_ANT_ON;
    }
    else
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_ONE;
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ANT1_ON;
    }

    pstMsg->stRfCfgPara.usRxBand       = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->stRfCfgPara.usRxFreqNum    = g_stAtDevCmdCtrl.stDspBandArfcn.usDlArfcn;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("At_SendRfCfgAntSelToHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_RcvDrvAgentSetFchanRsp(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                 *pRcvMsg = VOS_NULL_PTR;
    DRV_AGENT_FCHAN_SET_CNF_STRU       *pstFchanSetCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulError;

    /* ��ʼ�� */
    pRcvMsg          = (DRV_AGENT_MSG_STRU *)pMsg;
    pstFchanSetCnf   = (DRV_AGENT_FCHAN_SET_CNF_STRU *)pRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstFchanSetCnf->stAtAppCtrl.usClientId,&ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentSetFchanRsp:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSetFchanRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_FCHAN_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FCHAN_SET)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����д���Ĵ��� */
    ulError =  AT_SetFchanRspErr(pstFchanSetCnf->enResult);
    if(ulError != DRV_AGENT_FCHAN_SET_NO_ERROR)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, ulError);
        return VOS_OK;
    }

    /* �����޴���Ĵ��� */
    g_stAtDevCmdCtrl.bDspLoadFlag    = VOS_TRUE;
    g_stAtDevCmdCtrl.ucDeviceRatMode = (VOS_UINT8)pstFchanSetCnf->stFchanSetReq.ucDeviceRatMode;
    g_stAtDevCmdCtrl.ucDeviceAtBand  = (VOS_UINT8)pstFchanSetCnf->stFchanSetReq.ucDeviceAtBand;
    g_stAtDevCmdCtrl.stDspBandArfcn  = pstFchanSetCnf->stFchanSetReq.stDspBandArfcn;
    g_stAtDevCmdCtrl.usFDAC          = 0;                                       /* FDAC���㣬��ֹG/W��Χ���� */

    /* ����AT_FormATResultDATa���������� */
    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex,AT_OK);
    return VOS_OK;
}


VOS_VOID  At_HpaRfCfgCnfProc(
    HPA_AT_RF_CFG_CNF_STRU              *pstMsg
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (pstMsg->usErrFlg == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_HpaRfCfgCnfProc: set rfcfg err");
        ulRslt = At_RfCfgCnfReturnErrProc(ucIndex);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    else
    {
        ulRslt = AT_OK;
        At_RfCfgCnfReturnSuccProc(ucIndex);

        /* ^FRSSI?��GDSP LOAD������յ�ID_HPA_AT_RF_CFG_CNF��Ϣ,�����ϱ�,���յ�
           ID_HPA_AT_RF_RX_RSSI_IND��Ϣʱ���ϱ� */
        if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_QUERY_RSSI)
        {
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, ulRslt);
        }
    }
    return;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID  At_CHpaRfCfgCnfProc(
    CHPA_AT_RF_CFG_CNF_STRU             *pstMsg
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (pstMsg->usErrFlg == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_CHpaRfCfgCnfProc: set rfcfg err");
        ulRslt = At_RfCfgCnfReturnErrProc(ucIndex);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    else
    {
        ulRslt = AT_OK;
        At_RfCfgCnfReturnSuccProc(ucIndex);

        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    return;
}
#endif


VOS_UINT32 AT_RcvDrvAgentTseLrfSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_TSELRF_SET_CNF_STRU      *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_TSELRF_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�TSELRF��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TSELRF_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    gstAtSendData.usBufLen = 0;
    if (pstEvent->enResult == DRV_AGENT_TSELRF_SET_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK */
        ulRet                            = AT_OK;
        g_stAtDevCmdCtrl.bDspLoadFlag    = VOS_TRUE;
        g_stAtDevCmdCtrl.ucDeviceRatMode = pstEvent->ucDeviceRatMode;
        g_stAtDevCmdCtrl.usFDAC          = 0;

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                            = AT_ERROR;
    }

    /* 4. ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_VOID At_RfFpowdetTCnfProc(PHY_AT_POWER_DET_CNF_STRU *pstMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* ��ȡ���ر�����û����� */
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPOWDET_QRY)
    {
        AT_WARN_LOG("At_RfFPOWDETCnfProc: CmdCurrentOpt is not AT_CMD_FPOWDET_QRY!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* Ӧ�����Ҫ���������ֵΪ0x7FFF��Ϊ��Чֵ�����ѯ�߷���ERROR */
    if(pstMsg->sPowerDet == 0x7FFF)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           "%s: %d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstMsg->sPowerDet);

        gstAtSendData.usBufLen = usLength;

        At_FormatResultData(ucIndex, AT_OK);
    }

    return;
}


VOS_UINT32 AT_RcvMtaPowerDetQryCnf(VOS_VOID *pMsg)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                    *pstMtaMsg = VOS_NULL_PTR;
    MTA_AT_POWER_DET_QRY_CNF_STRU      *pstPowerDetQryCnf = VOS_NULL_PTR;
    PHY_AT_POWER_DET_CNF_STRU           stPowerNetMsg;

    /* ��ʼ����Ϣ���� */
    pstMtaMsg           = (AT_MTA_MSG_STRU *)pMsg;
    pstPowerDetQryCnf   = (MTA_AT_POWER_DET_QRY_CNF_STRU *)pstMtaMsg->aucContent;

    memset_s(&stPowerNetMsg, sizeof(stPowerNetMsg), 0x00, sizeof(PHY_AT_POWER_DET_CNF_STRU));

    if (pstPowerDetQryCnf->enResult == MTA_AT_RESULT_NO_ERROR)
    {
        stPowerNetMsg.sPowerDet = pstPowerDetQryCnf->sPowerDet;
    }
    else
    {
        stPowerNetMsg.sPowerDet = 0x7FFF;
    }

    At_RfFpowdetTCnfProc(&stPowerNetMsg);

    return VOS_OK;
}


VOS_VOID At_RfPllStatusCnfProc(PHY_AT_RF_PLL_STATUS_CNF_STRU *pstMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* ��ȡ���ر�����û����� */
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPLLSTATUS_QRY)
    {
        AT_WARN_LOG("At_RfPllStatusCnfProc: CmdCurrentOpt is not AT_CMD_FPLLSTATUS_QRY!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: %d,%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstMsg->usTxStatus,
                                       pstMsg->usRxStatus);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return;
}


VOS_VOID  At_RfRssiIndProc(
    HPA_AT_RF_RX_RSSI_IND_STRU          *pstMsg
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    VOS_INT32                           lRssi;

    /*��ȡ���ر�����û�����*/
    ucIndex  = g_stAtDevCmdCtrl.ucIndex;

    if (pstMsg->sAGCGain == AT_DSP_RF_AGC_STATE_ERROR)  /* ���� */
    {
        AT_WARN_LOG("AT_RfRssiIndProc err");
        ulRslt = AT_FRSSI_OTHER_ERR;
    }
    else
    {

        gstAtSendData.usBufLen = 0;

        /*����RSSI����ֵ��λ0.125dBm��Ϊ������������*1000.*/
        lRssi = pstMsg->sRSSI * AT_DSP_RSSI_VALUE_MUL_THOUSAND;

        /*��ȡ��RSSIֵ��������ֵ�ϱ�����ȷ��0.1dBm����ֵ��Ϣ�������ǰ��RSSI
          ֵΪ-85.1dBm������ֵΪ851. ����֮ǰ��1000�����Ծ�ȷ��0.1dBm����Ҫ��100*/
        if (lRssi < 0 )
        {
            lRssi = (-1*lRssi)/100;
        }
        else
        {
            lRssi = lRssi/100;
        }

        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           lRssi);

        gstAtSendData.usBufLen = usLength;
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return;
}


VOS_UINT32  At_PdmCtrlCnfProc( HPA_AT_PDM_CTRL_CNF_STRU *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ucIndex         = g_stAtDevCmdCtrl.ucIndex;

    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PDM_CTRL )
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstMsg->ulResult == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_PdmCtrlCnfProc: read PdmCtrl err");
        ulRslt      = AT_ERROR;
    }
    else
    {
        ulRslt      = AT_OK;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_VOID  At_RfCfgCnfReturnSuccProc(
    VOS_UINT8                           ucIndex
)
{
    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_SET_FTXON:
            g_stAtDevCmdCtrl.ucTxOnOff = g_stAtDevCmdCtrl.ucTempRxorTxOnOff;

            /* ����Ǵ򿪷������������Ҫ��¼���һ��ִ�е��Ǵ򿪷�������Ǵ򿪽��ջ����� */
            if (g_stAtDevCmdCtrl.ucTxOnOff == AT_DSP_RF_SWITCH_ON)
            {
                g_stAtDevCmdCtrl.ucRxonOrTxon = AT_TXON_OPEN;
            }
            break;

        case AT_CMD_SET_FRXON:
            g_stAtDevCmdCtrl.ucRxOnOff = g_stAtDevCmdCtrl.ucTempRxorTxOnOff;

            /* ����Ǵ򿪽��ջ���������Ҫ��¼���һ��ִ�е��Ǵ򿪷�������Ǵ򿪽��ջ����� */
            if (g_stAtDevCmdCtrl.ucRxOnOff == AT_DSP_RF_SWITCH_ON)
            {
                g_stAtDevCmdCtrl.ucRxonOrTxon = AT_RXON_OPEN;
            }
            break;

        case AT_CMD_QUERY_RSSI:
            break;

        case AT_CMD_SET_RXDIV:
           if ((At_SaveRxDivPara(g_stAtDevCmdCtrl.usOrigBand, 1) == AT_OK)
            && (g_stAtDevCmdCtrl.ucCurrentTMode == AT_TMODE_FTM))
           {
               g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_DIV_ON;
               g_stAtDevCmdCtrl.usRxDiv    = g_stAtDevCmdCtrl.usOrigBand;
           }
           break;

        case AT_CMD_SET_RXPRI:
            g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_PRI_ON;
            g_stAtDevCmdCtrl.usRxPri    = g_stAtDevCmdCtrl.usOrigBand;
            break;

        default:
            break;
    }

    return;

}


VOS_VOID  At_WTxCltIndProc(
    WPHY_AT_TX_CLT_IND_STRU            *pstMsg
)
{
    /* ��ʼ��ȫ�ֱ��� */
    memset_s(&g_stAtDevCmdCtrl.stCltInfo, sizeof(g_stAtDevCmdCtrl.stCltInfo), 0x0, sizeof(AT_TX_CLT_INFO_STRU));

    /* ����CLT��Ϣ��Ч��־ */
    g_stAtDevCmdCtrl.stCltInfo.ulInfoAvailableFlg   = VOS_TRUE;

    /* ��������ϱ�����Ϣ��¼��ȫ�ֱ����� */
    g_stAtDevCmdCtrl.stCltInfo.shwGammaReal             = pstMsg->shwGammaReal;                 /* ����ϵ��ʵ�� */
    g_stAtDevCmdCtrl.stCltInfo.shwGammaImag             = pstMsg->shwGammaImag;                 /* ����ϵ���鲿 */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc0          = pstMsg->ushwGammaAmpUc0;              /* פ����ⳡ��0����ϵ������ */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc1          = pstMsg->ushwGammaAmpUc1;              /* פ����ⳡ��1����ϵ������ */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc2          = pstMsg->ushwGammaAmpUc2;              /* פ����ⳡ��2����ϵ������ */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAntCoarseTune   = pstMsg->ushwGammaAntCoarseTune;       /* �ֵ����λ�� */
    g_stAtDevCmdCtrl.stCltInfo.ulwFomcoarseTune         = pstMsg->ulwFomcoarseTune;             /* �ֵ�FOMֵ */
    g_stAtDevCmdCtrl.stCltInfo.ushwCltAlgState          = pstMsg->ushwCltAlgState;              /* �ջ��㷨����״̬ */
    g_stAtDevCmdCtrl.stCltInfo.ushwCltDetectCount       = pstMsg->ushwCltDetectCount;           /* �ջ������ܲ��� */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac0                 = pstMsg->ushwDac0;                     /* DAC0 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac1                 = pstMsg->ushwDac1;                     /* DAC1 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac2                 = pstMsg->ushwDac2;                     /* DAC2 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac3                 = pstMsg->ushwDac3;                     /* DAC3 */

    return;
}


VOS_UINT32 AT_RcvMtaRficSsiRdQryCnf(VOS_VOID *pMsg)
{
    /* ����ֲ����� */
    AT_MTA_MSG_STRU                    *pstMtaMsg = VOS_NULL_PTR;
    MTA_AT_RFICSSIRD_CNF_STRU          *pstRficSsiRdCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* ��ʼ����Ϣ���� */
    ucIndex                 = 0;
    pstMtaMsg               = ( AT_MTA_MSG_STRU * )pMsg;
    pstRficSsiRdCnf         = ( MTA_AT_RFICSSIRD_CNF_STRU * )pstMtaMsg->aucContent;

    ucIndex                 = g_stAtDevCmdCtrl.ucIndex;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaRficSsiRdQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_RFICSSIRD_SET */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_RFICSSIRD_SET )
    {
        AT_WARN_LOG("AT_RcvMtaRficSsiRdQryCnf: WARNING:Not AT_CMD_REFCLKFREQ_READ!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ����Ĵ�����ֵ */
    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     "%s: %d",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     pstRficSsiRdCnf->ulRegValue );
    ulResult                = AT_OK;

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32  At_MipiRdCnfProc( HPA_AT_MIPI_RD_CNF_STRU *pstMsg )
{
    VOS_UINT32                          usRslt;
    VOS_UINT32                          usData;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�����ر��� */
    usLength        = 0;

    /*��ȡ���ر�����û�����*/
    ucIndex         = g_stAtDevCmdCtrl.ucIndex;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MIPI_RD)
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstMsg->ulResult == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_MipiRdCnfProc: read mipi err");
        usRslt = AT_ERROR;
    }
    else
    {
        usRslt      = AT_OK;
        usData      = pstMsg->ulValue;

        usLength    = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s:%d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              usData);
    }
    gstAtSendData.usBufLen  = usLength;
    At_FormatResultData(ucIndex, usRslt);
    return VOS_OK;
}


VOS_UINT32  At_MipiWrCnfProc( HPA_AT_MIPI_WR_CNF_STRU       *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

     if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MIPI_WR )
     {
          return VOS_ERR;
     }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstMsg->usErrFlg == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_MipiWrCnfProc: set MipiCfg err");
        ulRslt  = AT_ERROR;
    }
    else
    {
        ulRslt  = AT_OK;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32  At_SsiWrCnfProc( HPA_AT_SSI_WR_CNF_STRU         *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ucIndex     = g_stAtDevCmdCtrl.ucIndex;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SSI_WR)
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);


    if (pstMsg->usErrFlg == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_SsiWrCnfProc: set rfcfg err");
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt  = AT_OK;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32  At_SsiRdCnfProc( HPA_AT_SSI_RD_CNF_STRU          *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulData;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�����ر��� */
    usLength        = 0;

    /*��ȡ���ر�����û�����*/
    ucIndex         = g_stAtDevCmdCtrl.ucIndex;

    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SSI_RD )
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstMsg->ulResult == AT_HPA_RSLT_FAIL)
    {
        AT_INFO_LOG("At_MipiRdCnfProc: read SSI err");
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt      = AT_OK;
        ulData      = pstMsg->ulValue;

        usLength    = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            "%s:%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            ulData );
    }

    gstAtSendData.usBufLen  = usLength;
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SetFchanRspErr(DRV_AGENT_FCHAN_SET_ERROR_ENUM_UINT32  enResult)
{
    if(enResult == DRV_AGENT_FCHAN_BAND_NOT_MATCH)
    {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    if(enResult == DRV_AGENT_FCHAN_BAND_CHANNEL_NOT_MATCH)
    {
        return AT_FCHAN_BAND_CHANNEL_NOT_MATCH;
    }

    if(enResult == DRV_AGENT_FCHAN_OTHER_ERR)
    {
        g_stAtDevCmdCtrl.bDspLoadFlag = VOS_FALSE;
        AT_WARN_LOG("AT_SetFChanPara: DSP Load fail!");
        return AT_FCHAN_OTHER_ERR;
    }

    return DRV_AGENT_FCHAN_SET_NO_ERROR;

}


VOS_UINT32  At_RfCfgCnfReturnErrProc(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRslt;

    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
       case AT_CMD_SET_FTXON:
           ulRslt = AT_FTXON_SET_FAIL;
           break;

       case AT_CMD_SET_FRXON:
           ulRslt = AT_FRXON_SET_FAIL;
           break;

       case AT_CMD_QUERY_RSSI:
           ulRslt = AT_FRSSI_OTHER_ERR;
           break;

       /* ���������ͷּ�ʱ�յ�DSP�ظ���������·��صĴ�������ͬ */
       case AT_CMD_SET_RXDIV:
       case AT_CMD_SET_RXPRI:
           ulRslt = AT_CME_RX_DIV_OTHER_ERR;
           break;

       default:
           ulRslt = AT_ERROR;
           break;
    }

   return ulRslt;
}


#else
/*****************************************************************************
  4 �º���ʵ��
*****************************************************************************/


VOS_UINT32 At_SndRxOnOffReq(VOS_VOID)
{
    AT_BBIC_CAL_MT_RX_REQ              *pstRxReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstRxReq = (AT_BBIC_CAL_MT_RX_REQ*)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                sizeof(AT_BBIC_CAL_MT_RX_REQ) - VOS_MSG_HEAD_LENGTH);
    if (pstRxReq == VOS_NULL_PTR)
    {
        return VOS_FALSE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstRxReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstRxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_MT_RX_REQ);

    /* ��д��Ϣ���� */
    pstRxReq->stMtRxPara.ucRxEnable         = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;     
    pstRxReq->stMtRxPara.uhwAgcGainIndex    = g_stMtInfoCtx.stAtInfo.ucAgcGainLevel;
    pstRxReq->stMtRxPara.enModemId          = MODEM_ID_0;      
    pstRxReq->stMtRxPara.enRatMode          = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;      
    pstRxReq->stMtRxPara.enBand             = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;         
    pstRxReq->stMtRxPara.enBandWith         = g_stMtInfoCtx.stBbicInfo.enBandWidth; 
    pstRxReq->stMtRxPara.dlFreqInfo         = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulDlFreq;

    /* MIMO */
    if (g_stMtInfoCtx.stBbicInfo.enRxAntType == AT_ANT_TYPE_MIMO)
    {
        pstRxReq->stMtRxPara.usMimoType     = g_stMtInfoCtx.stBbicInfo.usRxMimoType;
        pstRxReq->stMtRxPara.usAntMap       = g_stMtInfoCtx.stBbicInfo.usRxMimoAntNum;
    }
    else
    {
        pstRxReq->stMtRxPara.usMimoType     = AT_SET_BIT32(1);
        pstRxReq->stMtRxPara.usAntMap       = g_stMtInfoCtx.stBbicInfo.enRxAntType;
    }

    if (PS_SEND_MSG(WUEPS_PID_AT, pstRxReq) != VOS_OK)
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndRxOnOffReq Exit");

    return VOS_TRUE;
     
}


VOS_UINT32 At_SndTxOnOffReq(VOS_UINT16 usPowerDetFlg)
{
    AT_BBIC_CAL_MT_TX_REQ                  *pstTxReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstTxReq = (AT_BBIC_CAL_MT_TX_REQ*)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                    sizeof(AT_BBIC_CAL_MT_TX_REQ) - VOS_MSG_HEAD_LENGTH);
    if (pstTxReq == VOS_NULL_PTR)
    {
        return VOS_FALSE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstTxReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstTxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_MT_TX_REQ);

    /* ��д��Ϣ���� */
    pstTxReq->stMtTxPara.ucTxEnable         = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff; 
    pstTxReq->stMtTxPara.ucTxPaMode         = g_stMtInfoCtx.stBbicInfo.enPaLevel;    
    pstTxReq->stMtTxPara.enScsType          = g_stMtInfoCtx.stBbicInfo.enBbicScs;     
    pstTxReq->stMtTxPara.usMrxEanble        = usPowerDetFlg;  
    pstTxReq->stMtTxPara.enModemId          = MODEM_ID_0;     
    pstTxReq->stMtTxPara.enRatMode          = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;     
    pstTxReq->stMtTxPara.enBand             = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;        
    pstTxReq->stMtTxPara.enBandWith         = g_stMtInfoCtx.stBbicInfo.enBandWidth;    
    pstTxReq->stMtTxPara.ulFreqInfo         = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulUlFreq; 
    pstTxReq->stMtTxPara.enPowerCtrlMode    = POWER_CTRL_MODE_POWER;
    pstTxReq->stMtTxPara.unPowerPara.shwTxPower = g_stMtInfoCtx.stBbicInfo.sFwavePower;

    /* �������ǵ��� */
    if (g_stMtInfoCtx.stBbicInfo.enFwaveType == MODU_TYPE_BUTT)
    {
        pstTxReq->stMtTxPara.ucIsSingleTone = VOS_TRUE;             /* ���� */
        pstTxReq->stMtTxPara.enModType      = MODU_TYPE_LTE_QPSK;   /* ����Ĭ��дQPSK */

        if (g_stMtInfoCtx.stAtInfo.enRatMode == AT_RAT_MODE_GSM)
        {
            pstTxReq->stMtTxPara.enModType  = MODU_TYPE_GMSK;
        }

        if (g_stMtInfoCtx.stAtInfo.enRatMode == AT_RAT_MODE_EDGE)
        {
            pstTxReq->stMtTxPara.enModType  = MODU_TYPE_8PSK;
        }
    }
    else
    {
        pstTxReq->stMtTxPara.ucIsSingleTone = VOS_FALSE;             /* ���� */
        pstTxReq->stMtTxPara.enModType      = g_stMtInfoCtx.stBbicInfo.enFwaveType;
    }

    /* MIMO���� */
    if (g_stMtInfoCtx.stBbicInfo.enTxAntType == AT_ANT_TYPE_MIMO)
    {
        pstTxReq->stMtTxPara.usMimoType = g_stMtInfoCtx.stBbicInfo.usTxMimoType;
        pstTxReq->stMtTxPara.usAntMap   = g_stMtInfoCtx.stBbicInfo.usTxMimoAntNum;
    }
    else
    {   
        /* Ĭ����1 TX */
        pstTxReq->stMtTxPara.usMimoType   = AT_SET_BIT32(0);
        pstTxReq->stMtTxPara.usAntMap     = AT_MIMO_ANT_NUM_1;
    }

    if (PS_SEND_MSG(WUEPS_PID_AT, pstTxReq) != VOS_OK)
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndTxOnOffReq Exit");

    return VOS_TRUE;
}


VOS_UINT32 At_SndGsmTxOnOffReq_ModulatedWave(
    VOS_UINT16                          usPowerDetFlg
)
{
    BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU      *pstGsmTxReq = VOS_NULL_PTR;
    VOS_UINT32                              ulTotalNum;
    VOS_UINT32                              ulIndex;
    MODU_TYPE_ENUM_UINT16                   enModType;

    enModType   = MODU_TYPE_GMSK;
    /* ������Ϣ�ռ� */
    pstGsmTxReq = (BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (pstGsmTxReq == VOS_NULL_PTR)
    {
        return VOS_FALSE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstGsmTxReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstGsmTxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_RF_DEBUG_GSM_TX_REQ);

    pstGsmTxReq->stPara.uhwTxEnable                = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
    pstGsmTxReq->stPara.uhwMrxEanble               = usPowerDetFlg;
    pstGsmTxReq->stPara.stCommonInfo.enModemId     = MODEM_ID_0;
    pstGsmTxReq->stPara.stCommonInfo.enRatMode     = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;
    pstGsmTxReq->stPara.stCommonInfo.enBand        = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    pstGsmTxReq->stPara.stCommonInfo.enBandWith    = g_stMtInfoCtx.stBbicInfo.enBandWidth;
    pstGsmTxReq->stPara.stCommonInfo.uhwSignalType = BBIC_CAL_SIGNAL_TYPE_MODU;
    pstGsmTxReq->stPara.stCommonInfo.uhwUlPath =
                             AT_GetGsmUlPathByBandNo(g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand);
    pstGsmTxReq->stPara.stCommonInfo.enModType = 0;

    if (g_stMtInfoCtx.stAtInfo.enRatMode == AT_RAT_MODE_GSM)
    {
        enModType = MODU_TYPE_GMSK;
    }

    if (g_stMtInfoCtx.stAtInfo.enRatMode == AT_RAT_MODE_EDGE)
    {
        enModType = MODU_TYPE_8PSK;
    }

    /* GSM TX û��MIMO */
    pstGsmTxReq->stPara.stCommonInfo.uhwMimoType   = 0;
    pstGsmTxReq->stPara.stCommonInfo.uwTxFreqInfo  = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulUlFreq;

    pstGsmTxReq->stPara.uhwDataPattern             = 1; /* Ĭ�ϲ�������� */
    pstGsmTxReq->stPara.enMrxEstMode               = MRX_ESTIMATE_POWER_MODE;
    pstGsmTxReq->stPara.uhwSlotCnt                 = g_stMtInfoCtx.stAtInfo.enGsmTxSlotType;

    ulTotalNum = g_stMtInfoCtx.stAtInfo.enGsmTxSlotType; /* ��Ҫ��д�������� */
    if (g_stMtInfoCtx.stAtInfo.enGsmTxSlotType == AT_GSM_TX_8_SLOT)
    {
        ulTotalNum = AT_GSM_TX_1_SLOT;
        pstGsmTxReq->stPara.uhwSlotCnt = AT_BBIC_CAL_MAX_GSM_SLOT;              /* 8��slot */
    }

    for (ulIndex = 0; ulIndex < ulTotalNum; ulIndex++)
    {
        pstGsmTxReq->stPara.astSlotPara[ulIndex].uhwModuType = enModType;
        pstGsmTxReq->stPara.astSlotPara[ulIndex].stTxPowerPara.enPowerCtrlMode        = POWER_CTRL_MODE_POWER;
        pstGsmTxReq->stPara.astSlotPara[ulIndex].stTxPowerPara.stPaPara.ucTxPaMode    = g_stMtInfoCtx.stBbicInfo.enPaLevel;
        pstGsmTxReq->stPara.astSlotPara[ulIndex].stTxPowerPara.unPowerPara.shwTxPower = g_stMtInfoCtx.stBbicInfo.sFwavePower;
    }

    if (PS_SEND_MSG(WUEPS_PID_AT, pstGsmTxReq) != VOS_OK)
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndGsmTxOnOffReq_ModulatedWave Exit");

    return VOS_TRUE;

}



VOS_UINT32  At_LoadPhy(VOS_VOID)
{
    AT_CCBT_LOAD_PHY_REQ_STRU          *pstLoadPhyReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstLoadPhyReq = (AT_CCBT_LOAD_PHY_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(AT_CCBT_LOAD_PHY_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (pstLoadPhyReq == VOS_NULL_PTR)
    {
        return VOS_FALSE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstLoadPhyReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstLoadPhyReq, CCPU_PID_CBT, ID_AT_CCBT_SET_WORK_MODE_REQ);

    /* ��д��Ϣ���� */
    pstLoadPhyReq->stLoadphyPara.aenRatMode[0] = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;
    pstLoadPhyReq->stLoadphyPara.enBusiness = CBT_BUSINESS_TYPE_MT;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstLoadPhyReq) != VOS_OK)
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_LoadPhy Exit");

    return VOS_TRUE;
}


VOS_VOID AT_ProcCbtMsg(VOS_VOID *pstMsg)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usMsgId;


    ulMsgCnt = sizeof(g_astAtProcCbtMsgTab) / sizeof(AT_PROC_CBT_MSG_STRU);

    usMsgId  = ((AT_MT_MSG_HEADER_STRU *)pstMsg)->usMsgId;

    /* g_astAtProcCbtMsgTab���������Ϣ�ַ� */
    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtProcCbtMsgTab[i].ulMsgType == usMsgId)
        {
            ulRst = g_astAtProcCbtMsgTab[i].pProcMsgFunc(pstMsg);

            if (ulRst == VOS_ERR)
            {
                AT_ERR_LOG("AT_ProcCbtMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /*û���ҵ�ƥ�����Ϣ*/
    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcCbtMsg: Msg Id is invalid!");
    }

    return;
}


VOS_VOID AT_ProcUeCbtMsg(VOS_VOID *pstMsg)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usMsgId;


    ulMsgCnt = sizeof(g_astAtProcUeCbtMsgTab) / sizeof(AT_PROC_UECBT_MSG_STRU);

    usMsgId  = ((AT_MT_MSG_HEADER_STRU *)pstMsg)->usMsgId;

    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtProcUeCbtMsgTab[i].ulMsgType == usMsgId)
        {
            ulRst = g_astAtProcUeCbtMsgTab[i].pProcMsgFunc(pstMsg);

            if (ulRst == VOS_ERR)
            {
                AT_ERR_LOG("AT_ProcUeCbtMsg: Msg Proc Err!");
            }

            return;
        }
    }

    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcUeCbtMsg: Msg Id is invalid!");
    }

    return;
}


VOS_VOID AT_ProcDspIdleMsg(VOS_VOID *pstMsg)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usMsgId;


    ulMsgCnt = sizeof(g_astAtProcDspIdleMsgTab) / sizeof(AT_PROC_DSP_IDLE_MSG_STRU);

    usMsgId  = ((AT_MT_MSG_HEADER_STRU *)pstMsg)->usMsgId;

    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtProcDspIdleMsgTab[i].ulMsgType == usMsgId)
        {
            ulRst = g_astAtProcDspIdleMsgTab[i].pProcMsgFunc(pstMsg);

            if (ulRst == VOS_ERR)
            {
                AT_ERR_LOG("AT_ProcDspIdleMsg: Msg Proc Err!");
            }

            return;
        }
    }

    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcDspIdleMsg: Msg Id is invalid!");
    }

    return;
}




VOS_VOID AT_ProcBbicMsg(VOS_VOID *pstMsg)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usMsgId;


    ulMsgCnt = sizeof(g_astAtProcBbicMsgTab) / sizeof(AT_PROC_BBIC_MSG_STRU);

    usMsgId  = ((AT_MT_MSG_HEADER_STRU *)pstMsg)->usMsgId;

    /* g_astAtProcBbicMsgTab���������Ϣ�ַ� */
    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtProcBbicMsgTab[i].ulMsgType == usMsgId)
        {
            ulRst = g_astAtProcBbicMsgTab[i].pProcMsgFunc(pstMsg);

            if (ulRst == VOS_ERR)
            {
                AT_ERR_LOG("AT_ProcBbicMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /*û���ҵ�ƥ�����Ϣ*/
    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcBbicMsg: Msg Id is invalid!");
    }

    return;
}


VOS_UINT32 AT_ProcSetWorkModeCnf(VOS_VOID *pstMsg)
{
    AT_CCBT_LOAD_PHY_CNF_STRU          *pstLoadCnf  = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pstLoadCnf  = (AT_CCBT_LOAD_PHY_CNF_STRU *)pstMsg;
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;

    AT_PR_LOGH("AT_ProcSetWorkModeCnf Enter");

    /* �±걣�� */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_ProcSetWorkModeCnf: ulIndex err!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_FCHAN_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FCHAN_SET)
    {
        AT_ERR_LOG("AT_ProcSetWorkModeCnf: Not AT_CMD_FCHAN_SET!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��0��ʾ���� */
    if (pstLoadCnf->ulErrorCode != MT_OK)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_FCHAN_LOAD_DSP_ERR);
        return VOS_OK;
    }

    gstAtSendData.usBufLen = 0;
    g_stMtInfoCtx.stAtInfo.bDspLoadFlag = VOS_TRUE;
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_UINT32 AT_ProcGsmTxCnf_ModulateWave(VOS_VOID *pstMsg)
{
    BBIC_CAL_MSG_CNF_STRU              *pstCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstCnf          = (BBIC_CAL_MSG_CNF_STRU *)pstMsg;

    AT_PR_LOGH("AT_ProcGsmTxCnf_ModulateWave Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_ProcGsmTxCnf_ModulateWave: ulIndex err!");
        return VOS_ERR;
    }

    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SET_FTXON)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPOWDET_QRY))
    {
        AT_ERR_LOG("AT_ProcGsmTxCnf_ModulateWave: Not Set FTXON,FPOWDET QRY !");
        return VOS_ERR;
    }

    /* Power Det�ǽ���TXON����Ϣ���͵ģ�����ǲ�ѯpower det�����˻�������Ϣ����Ҫ�ظ� ID_BBIC_TOOL_CAL_RF_DEBUG_TX_RESULT_IND */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_FPOWDET_QRY)
    {
        if (pstCnf->stPara.uwErrorCode != MT_OK)
        {
            /* ��λAT״̬ */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            gstAtSendData.usBufLen = 0;
            At_FormatResultData(ucIndex, AT_ERROR);
            return VOS_OK;
        }

        return VOS_OK;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCnf->stPara.uwErrorCode == MT_OK)
    {
        g_stMtInfoCtx.stAtInfo.enTxOnOff = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_FTXON_SET_FAIL);
    }

    return VOS_OK;
}



VOS_UINT32 AT_ProcTxCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_AT_MT_TRX_CNF             *pstCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstCnf          = (BBIC_CAL_AT_MT_TRX_CNF *)pstMsg;

    AT_PR_LOGH("AT_ProcTxCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_ProcTxCnf: ulIndex err!");
        return VOS_ERR;
    }

    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SET_FTXON)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPOWDET_QRY))
    {
        AT_ERR_LOG("AT_ProcTxCnf: Not Set FTXON,FPOWDET QRY !");
        return VOS_ERR;
    }

    /* Power Det�ǽ���TXON����Ϣ���͵ģ�����ǲ�ѯpower det�����˻�������Ϣ����Ҫ�ظ� ID_BBIC_TOOL_CAL_RF_DEBUG_TX_RESULT_IND */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_FPOWDET_QRY)
    {
        if (pstCnf->ulErrCode != MT_OK)
        {
            /* ��λAT״̬ */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            gstAtSendData.usBufLen = 0;
            At_FormatResultData(ucIndex, AT_ERROR);
        }

        return VOS_OK;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCnf->ulErrCode == MT_OK)
    {
        g_stMtInfoCtx.stAtInfo.enTxOnOff = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_FTXON_SET_FAIL);
    }

    return VOS_OK;
}


VOS_UINT32 AT_ProcRxCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_AT_MT_TRX_CNF             *pstCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstCnf          = (BBIC_CAL_AT_MT_TRX_CNF *)pstMsg;

    AT_PR_LOGH("AT_ProcRxCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_ProcRxCnf: ulIndex err!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SET_FRXON)
    {
        AT_ERR_LOG("AT_ProcRxCnf: Not Set FRXON QRY !");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCnf->ulErrCode == MT_OK)
    {
        g_stMtInfoCtx.stAtInfo.enRxOnOff    = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
        gstAtSendData.usBufLen              = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_FRXON_SET_FAIL);
    }

    return VOS_OK;
}


VOS_UINT32 AT_ProcPowerDetCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_AT_MT_TX_PD_IND           *pstPdResultInd = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    VOS_INT32                           lPowerValue;

    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    usLength        = 0;
    pstPdResultInd  = (BBIC_CAL_AT_MT_TX_PD_IND *)pstMsg;

    AT_PR_LOGH("AT_ProcPowerDetCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
       AT_ERR_LOG("AT_ProcPowerDetCnf: ulIndex err !");
       return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPOWDET_QRY)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstPdResultInd->mtTxIndPara.errorCode != MT_OK)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_OK;
    }

    if ((g_stMtInfoCtx.stBbicInfo.enCurrtRatMode == RAT_MODE_LTE)
     || (g_stMtInfoCtx.stBbicInfo.enCurrtRatMode == RAT_MODE_NR))
    {
        /* LTE��NR DSP�ϱ�����Ϊ0.125 */
        lPowerValue = pstPdResultInd->mtTxIndPara.antPower * 10 / 8;             /* �ϱ�����0.1dB */
    }
    else
    {
        /* GUC�ϱ�����Ϊ0.1 */
        lPowerValue = pstPdResultInd->mtTxIndPara.antPower; 
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^FPOWDET:%d",
                                        lPowerValue);

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_ProcPowerDetCnf_ModulateWave(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU     *pstPdResultInd = VOS_NULL_PTR;
    VOS_UINT32                              ulTotalNum;
    VOS_UINT32                              j;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucIndex;


    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    usLength        = 0;
    pstPdResultInd  = (BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_ProcPowerDetCnf_ModulateWave Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
       AT_ERR_LOG("AT_ProcPowerDetCnf_ModulateWave: ulIndex err !");
       return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPOWDET_QRY)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstPdResultInd->stPara.uwErrorCode != MT_OK)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_OK;
    }

    ulTotalNum = g_stMtInfoCtx.stAtInfo.enGsmTxSlotType; /* ��Ҫ��ȡ�������� */
    if (g_stMtInfoCtx.stAtInfo.enGsmTxSlotType == AT_GSM_TX_8_SLOT)
    {
        ulTotalNum = BBIC_CAL_GSM_TX_SLOT_NUM;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^FPOWDET:");

    for (j = 0; j < ulTotalNum; j++)
    {
        /* GUC�ϱ�����Ϊ0.1 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%d,",
                                        pstPdResultInd->stPara.astSlotValue[j].antPower);
    }

    /* �����һ������ȥ�� */
    usLength = usLength -1;

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}



VOS_UINT32 AT_SndBbicCalMipiReadReq(
    VOS_UINT32                          ulMipiPortSel,
    VOS_UINT32                          ulSlaveId,
    VOS_UINT32                          ulRegAddr,
    VOS_UINT32                          ulByteCnt,
    VOS_UINT32                          ulReadSpeed
)
{
    BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU       *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulLength;

    /* ����BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU��Ϣ */
    ulLength    = sizeof(BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SndBbicCalMipiReadReq: alloc msg fail!");
        return AT_FAILURE;
    }

     /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_MIPI_READ_REQ);

    /* ���Ʒ��ȷ�ϣ�ÿ��ֻ��дһ��MIPI */
    pstMsg->stPara.uwMipiNum                = 1;
    pstMsg->stPara.astCMD[0].bitMipiPortSel = ulMipiPortSel;
    pstMsg->stPara.astCMD[0].bitSlaveId     = ulSlaveId;
    pstMsg->stPara.astCMD[0].bitRegAddr     = ulRegAddr;
    pstMsg->stPara.astCMD[0].bitByteCnt     = ulByteCnt;
    pstMsg->stPara.readSpeedType            = (VOS_UINT8)ulReadSpeed;

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("AT_SndBbicCalMipiReadReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalMipiReadReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32 AT_SndBbicCalMipiWriteReq(
    VOS_UINT32                          ulMipiPortSel,
    VOS_UINT32                          ulSlaveId,
    VOS_UINT32                          ulRegAddr,
    VOS_UINT32                          ulByteCnt,
    VOS_UINT32                          ulValue
)
{
    BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU      *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulLength;

    /* ����BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU��Ϣ */
    ulLength    = sizeof(BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SndBbicCalMipiWriteReqMsg: alloc msg fail!");
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_MIPI_WRITE_REQ);

    pstMsg->stPara.uwMipiNum                        = 1;
    pstMsg->stPara.astData[0].stCmd.bitMipiPortSel  = ulMipiPortSel;
    pstMsg->stPara.astData[0].stCmd.bitSlaveId      = ulSlaveId;
    pstMsg->stPara.astData[0].stCmd.bitRegAddr      = ulRegAddr;
    pstMsg->stPara.astData[0].stCmd.bitByteCnt      = ulByteCnt;
    pstMsg->stPara.astData[0].stData.bitByte0       = ulValue;

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("AT_SndBbicCalMipiWriteReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalMipiWriteReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32 AT_RcvBbicCalMipiRedCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU    *pstRedCnf = VOS_NULL_PTR;
    VOS_UINT32                               ulRslt;
    VOS_UINT32                               ulByteCnt;
    VOS_UINT16                               usLength;
    VOS_UINT8                                ucIndex;

    ulRslt      = AT_OK;
    usLength    = 0;

    /*��ȡ���ر�����û�����*/
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstRedCnf   = (BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalMipiRedCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiRedCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MIPIOPERATE_SET)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiRedCnf: CmdCurrentOpt is not AT_CMD_MIPIOPERATE_SET!");
        return VOS_ERR;
    }

    if (pstRedCnf->uwResult == MT_OK)
    {
        ulRslt   = AT_OK;
        ulByteCnt = pstRedCnf->stPara.astData[0].stCmd.bitByteCnt;

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s: %d",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                   pstRedCnf->stPara.astData[0].stData.bitByte0);

        if (ulByteCnt > 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN - usLength,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d",
                                                   pstRedCnf->stPara.astData[0].stData.bitByte1);
        }

        if (ulByteCnt > 2)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN - usLength,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d",
                                                   pstRedCnf->stPara.astData[0].stData.bitByte2);
        }

        if (ulByteCnt > 3)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN - usLength,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d",
                                                   pstRedCnf->stPara.astData[0].stData.bitByte3);
        }

        gstAtSendData.usBufLen  = usLength;
    }
    else
    {
        AT_INFO_LOG("AT_RcvBbicCalMipiRedCnfMsg: read mipi err");
        ulRslt = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_RcvBbicCalMipiWriteCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU      *pstWriteCnf = VOS_NULL_PTR;
    VOS_UINT32                                  ulRslt;
    VOS_UINT8                                   ucIndex;


    /*��ȡ���ر�����û�����*/
    ulRslt      = AT_OK;
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstWriteCnf = (BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalMipiWriteCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiWriteCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MIPIOPERATE_SET)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiWriteCnf: CmdCurrentOpt is not AT_CMD_MIPIOPERATE_SET!");
        return VOS_ERR;
    }

    if (pstWriteCnf->uwResult == MT_OK)
    {
        ulRslt = AT_OK;
    }
    else
    {
        ulRslt = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicPllStatusReq(VOS_VOID)
{
    BBIC_CAL_PLL_QRY_REQ_STRU          *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* ����BBIC_CAL_PLL_QRY_REQ_STRU��Ϣ */
    ulLength = sizeof(BBIC_CAL_PLL_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (BBIC_CAL_PLL_QRY_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SndBbicPllStatusReq: Alloc msg fail!");
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_PLL_QRY_REQ);

    pstMsg->stPara.uhwBand   = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    pstMsg->stPara.enModType = MODU_TYPE_BUTT;                                  /* GSM  ���������������GSM�������ģʽ */
    pstMsg->stPara.enRatMode = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;

    /* ���ӦPHY������Ϣ */
    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("AT_SndBbicPllStatusReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicPllStatusReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 At_RcvBbicPllStatusCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_PLL_QRY_IND_STRU          *pstQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulPllStatus;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ulRslt      = AT_OK;
    usLength    = 0;
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstQryCnf   = (BBIC_CAL_PLL_QRY_IND_STRU *)pstMsg;

    AT_PR_LOGH("At_RcvBbicPllStatusCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("At_RcvBbicPllStatusCnf: ulIndex err !");
        return VOS_ERR;
    }

    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPLLSTATUS_QRY)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FPLLSTATUS_SET))
    {
        AT_ERR_LOG("At_RcvBbicPllStatusCnf: CmdCurrentOpt err !");
        return VOS_ERR;
    }

    if (pstQryCnf->stPara.uwErrorCode != MT_OK )
    {
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt   = AT_OK;
        if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_FPLLSTATUS_QRY)
        {
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s: %d,%d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              pstQryCnf->stPara.uwTxPllStatus,
                                              pstQryCnf->stPara.uwRxPllStatus);
        }
        else
        {
            ulPllStatus = (g_stMtInfoCtx.stAtInfo.enAntType == AT_MT_ANT_TYPE_TX) ?
                          (pstQryCnf->stPara.uwTxPllStatus) :
                           pstQryCnf->stPara.uwRxPllStatus;

            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s: %d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              ulPllStatus);
        }
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicRssiReq(VOS_VOID)
{
    AT_BBIC_CAL_MT_RX_RSSI_REQ         *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* ����BBIC_CAL_RF_DEBUG_RSSI_REQ_STRU��Ϣ */
    ulLength = sizeof(AT_BBIC_CAL_MT_RX_RSSI_REQ) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_BBIC_CAL_MT_RX_RSSI_REQ *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SndBbicRssiReq: alloc msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_MT_RX_RSSI_REQ);

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("AT_SndBbicRssiReq: Send msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    AT_PR_LOGH("AT_SndBbicRssiReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32  At_RcvBbicRssiInd(VOS_VOID *pstMsg)
{
    BBIC_CAL_AT_MT_RX_RSSI_IND         *pstRssiIndMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_INT32                           lRssi;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    lRssi           = 0;
    ulRslt          = AT_OK;
    usLength        = 0;
    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstRssiIndMsg   = (BBIC_CAL_AT_MT_RX_RSSI_IND *)pstMsg;

    AT_PR_LOGH("At_RcvBbicRssiInd Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("At_RcvBbicRssiInd: ulIndex err !");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_QUERY_RSSI)
    {
        AT_ERR_LOG("At_RcvBbicRssiInd: CmdCurrentOpt Not Query Rssi !");
        return VOS_ERR;
    }

    /* MIMO������1��3��5��7ȡashwRssi[0]�� 2��4��6��8ȡashwRssi[1] */
    if ((g_stMtInfoCtx.stBbicInfo.enRxAntType == AT_ANT_TYPE_PRI)
     || ((g_stMtInfoCtx.stAtInfo.enRxMimoAntNum % 2 == 1)
      && (g_stMtInfoCtx.stBbicInfo.enRxAntType == AT_ANT_TYPE_MIMO)))
    {
        /*����RSSI����ֵ��λ0.125dBm��Ϊ������������*1000.*/
        lRssi = pstRssiIndMsg->mtRxIndPara.rssi[0] * AT_DSP_RSSI_VALUE_MUL_THOUSAND;
    }
    else
    {
        /*����RSSI����ֵ��λ0.125dBm��Ϊ������������*1000.*/
        lRssi = pstRssiIndMsg->mtRxIndPara.rssi[1] * AT_DSP_RSSI_VALUE_MUL_THOUSAND;
    }

    if (pstRssiIndMsg->mtRxIndPara.errorCode != MT_OK)
    {
        AT_ERR_LOG("At_RcvBbicRssiIndProc err");
        ulRslt = AT_ERROR;
    }
    else
    {
        /*��ȡ��RSSIֵ��������ֵ�ϱ�����ȷ��0.01dBm����ֵ��Ϣ�������ǰ��RSSI
          ֵΪ-85.1dBm������ֵΪ8510. ����֮ǰ��1000�����Ծ�ȷ��0.01dBm����Ҫ��10*/
        if (lRssi < 0 )
        {
            lRssi = (-1*lRssi)/100;
        }
        else
        {
            lRssi = lRssi/100;
        }

        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%s:%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           lRssi);

        gstAtSendData.usBufLen = usLength;
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicCalSetDpdtReq(
    BBIC_DPDT_OPERTYPE_ENUM_UINT16      enOperType,
    VOS_UINT32                          ulValue,
    VOS_UINT32                          ulWorkType
)
{
    BBIC_CAL_DPDT_REQ_STRU             *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* ����BBIC_CAL_DPDT_REQ_STRU��Ϣ */
    ulLength = sizeof(BBIC_CAL_DPDT_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (BBIC_CAL_DPDT_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("At_SndBbicCalDpdtReq: alloc msg fail!");
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_DPDT_REQ);

    pstMsg->stPara.enRatMode                = g_stMtInfoCtx.stBbicInfo.enDpdtRatMode;
    pstMsg->stPara.enOperType               = enOperType;
    pstMsg->stPara.uwValue                  = ulValue;
    pstMsg->stPara.uhwModemWorkType         = (VOS_UINT16)ulWorkType;

    if ( PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("At_SndBbicCalDpdtReq: Send msg fail!");
        return AT_FAILURE;
    }


    AT_PR_LOGH("AT_SndBbicCalSetDpdtReq Exit");

    return AT_SUCCESS;
}



VOS_UINT32 AT_RcvBbicCalSetDpdtCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_DPDT_IND_STRU             *pstDpdtCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    ulRslt          = AT_ERROR;

    /* ��ȡ���ر�����û����� */
    ucIndex = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstDpdtCnf = (BBIC_CAL_DPDT_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalSetDpdtCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_RcvBbicCalSetDpdtCnf: ulIndex err !");
        return VOS_ERR;
    }

    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DPDT_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DPDTQRY_SET))
    {
        AT_ERR_LOG("AT_RcvBbicCalSetDpdtCnf: CmdCurrentOpt is not AT_CMD_DPDT_SET or AT_CMD_DPDTQRY_SET!");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstDpdtCnf->stPara.uwErrorCode != MT_OK)
    {
        AT_ERR_LOG1("AT_RcvBbicCalSetDpdtCnf: set dpdt error, ErrorCode is ", pstDpdtCnf->stPara.uwErrorCode);
        ulRslt = AT_ERROR;
    }
    else
    {
        /* ����������״̬ΪGetʱ���ϱ���ѯDpdt��� */
        if (pstDpdtCnf->stPara.unOperType == BBIC_DPDT_OPERTYPE_GET)
        {
            ulRslt = AT_OK;
            gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s: %d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstDpdtCnf->stPara.uwValue);
        }
        /* ����������״̬ΪSetʱ���������óɹ� */
        else
        {
            ulRslt = AT_OK;
            gstAtSendData.usBufLen = 0;
        }
    }

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}




VOS_UINT32 AT_SndBbicCalQryFtemprptReq(INT16 lChannelNum)
{
    BBIC_CAL_TEMP_QRY_REQ_STRU         *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* ����BBIC_CAL_TEMP_QRY_REQ_STRU��Ϣ */
    ulLength    = sizeof(BBIC_CAL_TEMP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (BBIC_CAL_TEMP_QRY_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("AT_SndBbicCalQryFtemprptReq: alloc msg fail!");
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_TEMP_QRY_REQ);

    pstMsg->stPara.enChannelType  = g_stMtInfoCtx.stBbicInfo.enCurrentChannelType;
    pstMsg->stPara.hwChannelNum   = lChannelNum;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("AT_SndBbicCalQryFtemprptReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalQryFtemprptReq Exit");

    return AT_SUCCESS;
}



VOS_UINT32 AT_RcvBbicCalQryFtemprptCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_TEMP_QRY_IND_STRU         *pstFtemprptCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    ulRslt   = AT_ERROR;

    /*��ȡ���ر�����û�����*/
    ucIndex = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstFtemprptCnf = (BBIC_CAL_TEMP_QRY_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalQryFtemprptCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_RcvBbicCalQryFtemprptCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FTEMPRPT_QRY)
    {
        AT_ERR_LOG("AT_RcvBbicCalQryFtemprptCnf: CmdCurrentOpt is not AT_CMD_OPT_READ_CMD!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstFtemprptCnf->stPara.uwErrorCode != MT_OK)
    {
        ulRslt = AT_ERROR;
        AT_ERR_LOG1("AT_RcvBbicCalQryFtemprptCnf: qry Ftemprpt error, ErrorCode is ", pstFtemprptCnf->stPara.uwErrorCode);
        gstAtSendData.usBufLen = 0;
    }
    else
    {
        ulRslt = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstFtemprptCnf->stPara.enChannelType,
                                      pstFtemprptCnf->stPara.hwChannelNum,
                                      pstFtemprptCnf->stPara.wTemperature);
    }

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 At_SndDcxoReq(VOS_VOID)
{
    BBIC_CAL_DCXO_REQ_STRU             *pstDcxoReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstDcxoReq = (BBIC_CAL_DCXO_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(BBIC_CAL_DCXO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (pstDcxoReq == VOS_NULL_PTR)
    {
        return VOS_FALSE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstDcxoReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstDcxoReq, DSP_PID_BBA_CAL, ID_AT_BBIC_DCXO_REQ);

    pstDcxoReq->stPara.enSetType    = g_stMtInfoCtx.stBbicInfo.enDcxoTempCompEnableFlg;
    pstDcxoReq->stPara.ulTxFreq     = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulUlFreq;
    pstDcxoReq->stPara.usBand       = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstDcxoReq) != VOS_OK)
    {
       return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndDcxoReq Exit");

    return VOS_TRUE;

}


VOS_UINT32 At_RcvBbicCalDcxoCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_DCXO_IND_STRU             *pstDcxoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*��ȡ���ر�����û�����*/
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstDcxoInd  = (BBIC_CAL_DCXO_IND_STRU*)pstMsg;
    ulRslt      = AT_OK;

    AT_PR_LOGH("At_RcvBbicCalDcxoCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("At_RcvBbicCalDcxoCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DCXOTEMPCOMP_SET)
    {
        AT_ERR_LOG("At_RcvBbicCalDcxoCnf: CmdCurrentOpt is not AT_CMD_DCXOTEMPCOMP_SET!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstDcxoInd->ulErrorCode != MT_OK)
    {
        ulRslt = AT_ERROR;
        AT_ERR_LOG1("At_RcvBbicCalDcxoCnf: ErrorCode is ", pstDcxoInd->ulErrorCode);
        gstAtSendData.usBufLen = 0;
    }
    else
    {
        ulRslt = AT_OK;
        gstAtSendData.usBufLen = 0;
        g_stMtInfoCtx.stAtInfo.enDcxoTempCompEnableFlg = (AT_DCXOTEMPCOMP_ENABLE_ENUM_UINT8)g_stMtInfoCtx.stBbicInfo.enDcxoTempCompEnableFlg;
    }

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 At_SndUeCbtRfIcMemTestReq(VOS_VOID)
{
    AT_UECBT_RficMemTestReqStru        *pstTestReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstTestReq = (AT_UECBT_RficMemTestReqStru *)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(AT_UECBT_RficMemTestReqStru) - VOS_MSG_HEAD_LENGTH);
    if (pstTestReq == VOS_NULL_PTR)
    {
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstTestReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstTestReq, CCPU_PID_PAM_MFG, ID_AT_UECBT_RFIC_MEM_TEST_REQ);

    if (PS_SEND_MSG(WUEPS_PID_AT, pstTestReq) != VOS_OK)
    {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndCbtRfIcMemTestNtf Exit");

    return AT_SUCCESS;
}


VOS_UINT32 At_ProcUeCbtRfIcMemTestCnf(VOS_VOID *pstMsg)
{
    UECBT_AT_RficMemTestCnfStru        *pstRficTestResult = VOS_NULL_PTR;

    AT_PR_LOGH("At_ProcUeCbtRfIcMemTestCnf Enter");

    pstRficTestResult = (UECBT_AT_RficMemTestCnfStru *)pstMsg;

    if (pstRficTestResult->errorCode == MT_OK)
    {
        g_stMtInfoCtx.stAtInfo.rficTestResult = AT_RFIC_MEM_TEST_PASS;
    }
    else
    {
        g_stMtInfoCtx.stAtInfo.rficTestResult = pstRficTestResult->errorCode;
    }

    return VOS_OK;
}


VOS_UINT32 At_SndDspIdleSerdesRtReq(VOS_VOID)
{
    AT_PHY_SERDES_AGING_TEST_REQ_STRU  *pstTestReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstTestReq = (AT_PHY_SERDES_AGING_TEST_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(AT_PHY_SERDES_AGING_TEST_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (pstTestReq == VOS_NULL_PTR)
    {
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstTestReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstTestReq, I0_DSP_PID_IDLE, ID_AT_PHY_SERDES_AGING_TEST_REQ);

    pstTestReq->testNum         = gastAtParaList[0].ulParaValue;
    pstTestReq->ulSyncLen       = gastAtParaList[1].ulParaValue;
    pstTestReq->ulBurstTime     = gastAtParaList[2].ulParaValue;
    pstTestReq->ulStallTime     = gastAtParaList[3].ulParaValue;
    pstTestReq->dlSyncLen       = gastAtParaList[4].ulParaValue;
    pstTestReq->dlBurstTime     = gastAtParaList[5].ulParaValue;
    pstTestReq->dlStallTime     = gastAtParaList[6].ulParaValue;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstTestReq) != VOS_OK)
    {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndPhySerDesrtTestReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32 At_ProcDspIdleSerdesRtCnf(VOS_VOID *pstMsg)
{
    PHY_AT_SERDES_AGING_TEST_CNF_STRU        *pstSerdesTestResult = VOS_NULL_PTR;

    AT_PR_LOGH("At_ProcPhySerDesrtTestCnf Enter");

    pstSerdesTestResult = (PHY_AT_SERDES_AGING_TEST_CNF_STRU *)pstMsg;

    if (pstSerdesTestResult->failNum == MT_OK)
    {
        g_stMtInfoCtx.rserTestResult = (VOS_INT32)AT_SERDES_TEST_PASS;
    }
    else
    {
        g_stMtInfoCtx.rserTestResult = pstSerdesTestResult->failNum;
    }

    return VOS_OK;
}



VOS_UINT32 At_SndBbicCalSetTrxTasReq(
    VOS_UINT16                          usTrxTasValue
)
{
    BBIC_CAL_SetTrxTasReqStru          *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength = sizeof(BBIC_CAL_SetTrxTasReqStru) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (BBIC_CAL_SetTrxTasReqStru *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_ERR_LOG("At_SndBbicCalSetTrxTasReq: alloc msg fail!");
        return AT_FAILURE;
    }

    AT_MT_CLR_MSG_ENTITY(pstMsg);
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_TRX_TAS_REQ);

    pstMsg->data.band          = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    pstMsg->data.ratMode       = g_stMtInfoCtx.stBbicInfo.enTrxTasRatMode;
    pstMsg->data.trxTasValue   = usTrxTasValue;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_ERR_LOG("At_SndBbicCalSetTrxTasReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndBbicCalSetTrxTasReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32 AT_RcvBbicCalSetTrxTasCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_SetTrxTasCnfStru          *pstTrxTasCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    ulRslt          = AT_ERROR;

    /* ��ȡ���ر�����û����� */
    ucIndex      = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstTrxTasCnf = (BBIC_CAL_SetTrxTasCnfStru *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalSetTrxTasCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_RcvBbicCalSetTrxTasCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TRX_TAS_SET)
    {
        AT_ERR_LOG("AT_RcvBbicCalSetTrxTasCnf: CmdCurrentOpt is not AT_CMD_TRX_TAS_SET!");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstTrxTasCnf->data.errorCode != MT_OK)
    {
        AT_ERR_LOG1("AT_RcvBbicCalSetTrxTasCnf: set trxtas error, ErrorCode is ", pstTrxTasCnf->data.errorCode);
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt = AT_OK;
        gstAtSendData.usBufLen = 0;
    }

    At_FormatResultData(ucIndex, ulRslt);
    return VOS_OK;
}


VOS_UINT32 At_SndUeCbtRfIcIdExQryReq(VOS_VOID)
{
    AT_UECBT_DieId_Query_Req           *pstQryReq = VOS_NULL_PTR;

    /* ������Ϣ�ռ� */
    pstQryReq = (AT_UECBT_DieId_Query_Req *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                         sizeof(AT_UECBT_DieId_Query_Req) - VOS_MSG_HEAD_LENGTH);
    if (pstQryReq == VOS_NULL_PTR) {
        return AT_FAILURE;
    }

    /* ��ʼ�� */
    AT_MT_CLR_MSG_ENTITY(pstQryReq);

    /* ��д��Ϣͷ */
    AT_CFG_MT_MSG_HDR(pstQryReq, CCPU_PID_PAM_MFG, ID_AT_UECBT_RFIC_DIE_IE_QUERY_REQ);

    if (PS_SEND_MSG(WUEPS_PID_AT, pstQryReq) != VOS_OK) {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndUeCbtRfIcIdExQryReq Exit");
    return AT_SUCCESS;
}


VOS_UINT32 At_RcvUeCbtRfIcIdExQryCnf(VOS_VOID *pstMsg)
{
    UECBT_AT_DieId_Query_Ind           *pstRcvMsg = VOS_NULL_PTR;
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucDataSize;
    VOS_UINT8                           ucAtCrLfLen;

    /*��ʼ���ֲ�����*/
    usLength                  = 0;
    ucDataSize                = 0;
    ucIndex                   = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstRcvMsg                 = (UECBT_AT_DieId_Query_Ind *)pstMsg;

    AT_PR_LOGH("At_RcvUeCbtRfIcIdExQryCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("At_RcvUeCbtRfIcIdExQryCnf: index err!");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_RFIC_DIE_ID_EX_QRY)
    {
        AT_WARN_LOG("AT_RcvMtaRficDieIDQryCnf: OPTION ERR!");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ((pstRcvMsg->errorCode != MT_OK)
     || (pstRcvMsg->chipNum > DIE_ID_QUERY_CHIP_MAX_NUM))
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_OK;
    }
    else
    {
        ucAtCrLfLen = (VOS_UINT8)strlen((VOS_CHAR *)gaucAtCrLf);
        for (i = 0; i < pstRcvMsg->chipNum; i++)
        {
            /* ��ӡ�����������,IC type */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s: %d,%d,\"",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               i,
                                               pstRcvMsg->dieIdInfo[i].chipType);
            /*RFIC ID ʹ�õͰ�λ����*/
            ucDataSize = (VOS_UINT8)TAF_MIN((pstRcvMsg->dieIdInfo[i].infoSize), DIE_ID_MAX_LEN_BYTE);
            for (j = 0; j < ucDataSize; j++)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%02x",
                                                   pstRcvMsg->dieIdInfo[i].data[j]);
            }

            /* ��ӡһ������ */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s",
                                               gaucAtCrLf);
        }

        /* �����һ�����з�ȥ�� */
        gstAtSendData.usBufLen = usLength - ucAtCrLfLen;
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}

#endif


