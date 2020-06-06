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
#include "siappstk.h"
#include "PppInterface.h"
#include "AtDataProc.h"
#include "AtEventReport.h"
#include "TafDrvAgent.h"
#include "AtOamInterface.h"
#include "MnCommApi.h"
#if( FEATURE_ON == FEATURE_CSD )
#include "AtCsdInterface.h"
#endif

#include "AtInputProc.h"
#include "FcInterface.h"
#include "AtCmdMsgProc.h"

#if(FEATURE_ON == FEATURE_LTE)
#include "gen_msg.h"
#include "at_lte_common.h"
#endif

#include "TafAppMma.h"

#include "AppVcApi.h"
#include "TafAppRabm.h"
#include "AtCmdSimProc.h"

#include  "product_config.h"

#include "TafStdlib.h"

#include "AtMsgPrint.h"
#include "AtCmdCallProc.h"

#include "AtCmdSupsProc.h"

#include "mnmsgcbencdec.h"
#include "TafCcmApi.h"
#include "securec.h"


/*****************************************************************************
  2 ��������
*****************************************************************************/
/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_EVENTREPORT_C

/*****************************************************************************
  3 ���Ͷ���
*****************************************************************************/


/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/
#if(FEATURE_ON == FEATURE_UE_MODE_NR)
extern AT_MT_INFO_STRU                         g_stMtInfoCtx;
#else
extern AT_DEVICE_CMD_CTRL_STRU                 g_stAtDevCmdCtrl;
#endif

const AT_SMS_ERROR_CODE_MAP_STRU        g_astAtSmsErrorCodeMap[] =
{
    {TAF_MSG_ERROR_RP_CAUSE_UNASSIGNED_UNALLOCATED_NUMBER,                              AT_CMS_UNASSIGNED_UNALLOCATED_NUMBER},
    {TAF_MSG_ERROR_RP_CAUSE_OPERATOR_DETERMINED_BARRING,                                AT_CMS_OPERATOR_DETERMINED_BARRING},
    {TAF_MSG_ERROR_RP_CAUSE_CALL_BARRED,                                                AT_CMS_CALL_BARRED},
    {TAF_MSG_ERROR_RP_CAUSE_SHORT_MESSAGE_TRANSFER_REJECTED,                            AT_CMS_SHORT_MESSAGE_TRANSFER_REJECTED},
    {TAF_MSG_ERROR_RP_CAUSE_DESTINATION_OUT_OF_ORDER,                                   AT_CMS_DESTINATION_OUT_OF_SERVICE},
    {TAF_MSG_ERROR_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER,                                    AT_CMS_UNIDENTIFIED_SUBSCRIBER},
    {TAF_MSG_ERROR_RP_CAUSE_FACILITY_REJECTED,                                          AT_CMS_FACILITY_REJECTED},
    {TAF_MSG_ERROR_RP_CAUSE_UNKNOWN_SUBSCRIBER,                                         AT_CMS_UNKNOWN_SUBSCRIBER},
    {TAF_MSG_ERROR_RP_CAUSE_NETWORK_OUT_OF_ORDER,                                       AT_CMS_NETWORK_OUT_OF_ORDER},
    {TAF_MSG_ERROR_RP_CAUSE_TEMPORARY_FAILURE,                                          AT_CMS_TEMPORARY_FAILURE},
    {TAF_MSG_ERROR_RP_CAUSE_CONGESTION,                                                 AT_CMS_CONGESTION},
    {TAF_MSG_ERROR_RP_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED,                          AT_CMS_RESOURCES_UNAVAILABLE_UNSPECIFIED},
    {TAF_MSG_ERROR_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED,                          AT_CMS_REQUESTED_FACILITY_NOT_SUBSCRIBED},
    {TAF_MSG_ERROR_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED,                         AT_CMS_REQUESTED_FACILITY_NOT_IMPLEMENTED},
    {TAF_MSG_ERROR_RP_CAUSE_INVALID_SHORT_MESSAGE_TRANSFER_REFERENCE_VALUE,             AT_CMS_INVALID_SHORT_MESSAGE_TRANSFER_REFERENCE_VALUE},
    {TAF_MSG_ERROR_RP_CAUSE_INVALID_MANDATORY_INFORMATION,                              AT_CMS_INVALID_MANDATORY_INFORMATION},
    {TAF_MSG_ERROR_RP_CAUSE_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED,               AT_CMS_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED},
    {TAF_MSG_ERROR_RP_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_SHORT_MESSAGE_PROTOCOL_STATE,   AT_CMS_MESSAGE_NOT_COMPATIBLE_WITH_SHORT_MESSAGE_PROTOCOL_STATE},
    {TAF_MSG_ERROR_RP_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED,        AT_CMS_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED},
    {TAF_MSG_ERROR_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,                                 AT_CMS_PROTOCOL_ERROR_UNSPECIFIED},
    {TAF_MSG_ERROR_RP_CAUSE_INTERWORKING_UNSPECIFIED,                                   AT_CMS_INTERWORKING_UNSPECIFIED},
    {TAF_MSG_ERROR_TP_FCS_TELEMATIC_INTERWORKING_NOT_SUPPORTED,                         AT_CMS_TELEMATIC_INTERWORKING_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED,                           AT_CMS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_CANNOT_REPLACE_SHORT_MESSAGE,                                 AT_CMS_CANNOT_REPLACE_SHORT_MESSAGE},
    {TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPPID_ERROR,                                      AT_CMS_UNSPECIFIED_TPPID_ERROR},
    {TAF_MSG_ERROR_TP_FCS_DATA_CODING_SCHEME_ALPHABET_NOT_SUPPORTED,                    AT_CMS_DATA_CODING_SCHEME_ALPHABET_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_MESSAGE_CLASS_NOT_SUPPORTED,                                  AT_CMS_MESSAGE_CLASS_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPDCS_ERROR,                                      AT_CMS_UNSPECIFIED_TPDCS_ERROR},
    {TAF_MSG_ERROR_TP_FCS_COMMAND_CANNOT_BE_ACTIONED,                                   AT_CMS_COMMAND_CANNOT_BE_ACTIONED},
    {TAF_MSG_ERROR_TP_FCS_COMMAND_UNSUPPORTED,                                          AT_CMS_COMMAND_UNSUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPCOMMAND_ERROR,                                  AT_CMS_UNSPECIFIED_TPCOMMAND_ERROR},
    {TAF_MSG_ERROR_TP_FCS_TPDU_NOT_SUPPORTED,                                           AT_CMS_TPDU_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_SC_BUSY,                                                      AT_CMS_SC_BUSY},
    {TAF_MSG_ERROR_TP_FCS_NO_SC_SUBSCRIPTION,                                           AT_CMS_NO_SC_SUBSCRIPTION},
    {TAF_MSG_ERROR_TP_FCS_SC_SYSTEM_FAILURE,                                            AT_CMS_SC_SYSTEM_FAILURE},
    {TAF_MSG_ERROR_TP_FCS_INVALID_SME_ADDRESS,                                          AT_CMS_INVALID_SME_ADDRESS},
    {TAF_MSG_ERROR_TP_FCS_DESTINATION_SME_BARRED,                                       AT_CMS_DESTINATION_SME_BARRED},
    {TAF_MSG_ERROR_TP_FCS_SM_REJECTEDDUPLICATE_SM,                                      AT_CMS_SM_REJECTEDDUPLICATE_SM},
    {TAF_MSG_ERROR_TP_FCS_TPVPF_NOT_SUPPORTED,                                          AT_CMS_TPVPF_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_TPVP_NOT_SUPPORTED,                                           AT_CMS_TPVP_NOT_SUPPORTED},
    {TAF_MSG_ERROR_TP_FCS_SIM_SMS_STORAGE_FULL,                                         AT_CMS_SIM_SMS_STORAGE_FULL},
    {TAF_MSG_ERROR_TP_FCS_NO_SMS_STORAGE_CAPABILITY_IN_SIM,                             AT_CMS_NO_SMS_STORAGE_CAPABILITY_IN_SIM},
    {TAF_MSG_ERROR_TP_FCS_ERROR_IN_MS,                                                  AT_CMS_ERROR_IN_MS},
    {TAF_MSG_ERROR_TP_FCS_MEMORY_CAPACITY_EXCEEDED,                                     AT_CMS_MEMORY_CAPACITY_EXCEEDED},
    {TAF_MSG_ERROR_TP_FCS_SIM_APPLICATION_TOOLKIT_BUSY,                                 AT_CMS_SIM_APPLICATION_TOOLKIT_BUSY},
    {TAF_MSG_ERROR_TP_FCS_SIM_DATA_DOWNLOAD_ERROR,                                      AT_CMS_SIM_DATA_DOWNLOAD_ERROR},
    {TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_ERROR_CAUSE,                                      AT_CMS_UNSPECIFIED_ERROR_CAUSE},
    {TAF_MSG_ERROR_STATE_NOT_COMPATIBLE,                                                AT_CMS_ME_FAILURE},
    {TAF_MSG_ERROR_NO_SERVICE,                                                          AT_CMS_NO_NETWORK_SERVICE},
    {TAF_MSG_ERROR_TC1M_TIMEOUT,                                                        AT_CMS_NETWORK_TIMEOUT},
    {TAF_MSG_ERROR_TR1M_TIMEOUT,                                                        AT_CMS_NETWORK_TIMEOUT},
    {TAF_MSG_ERROR_TR2M_TIMEOUT,                                                        AT_CMS_NO_CNMA_ACKNOWLEDGEMENT_EXPECTED},
};


TAF_PS_EVT_ID_ENUM_UINT32 g_astAtBroadcastPsEvtTbl[] =
{
    ID_EVT_TAF_PS_REPORT_DSFLOW_IND,
    ID_EVT_TAF_PS_APDSFLOW_REPORT_IND,
    ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND,
    ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND,
    ID_EVT_TAF_PS_CGMTU_VALUE_CHG_IND,
    ID_EVT_TAF_PS_CALL_LIMIT_PDP_ACT_IND,
    ID_EVT_TAF_PS_REPORT_VTFLOW_IND,
    ID_EVT_TAF_PS_REPORT_PCO_INFO_IND,
    ID_EVT_TAF_PS_EPDG_CTRLU_NTF,

    ID_EVT_TAF_PS_UE_POLICY_RPT_IND,
};

const AT_PS_EVT_FUNC_TBL_STRU           g_astAtPsEvtFuncTbl[] =
{
    /* PS CALL */
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_CNF,
        AT_RcvTafPsCallEvtPdpActivateCnf},
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_REJ,
        AT_RcvTafPsCallEvtPdpActivateRej},
    {ID_EVT_TAF_PS_CALL_PDP_MANAGE_IND,
        AT_RcvTafPsCallEvtPdpManageInd},
    {ID_EVT_TAF_PS_CALL_PDP_ACTIVATE_IND,
        AT_RcvTafPsCallEvtPdpActivateInd},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_CNF,
        AT_RcvTafPsCallEvtPdpModifyCnf},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_REJ,
        AT_RcvTafPsCallEvtPdpModifyRej},
    {ID_EVT_TAF_PS_CALL_PDP_MODIFY_IND,
        AT_RcvTafPsCallEvtPdpModifiedInd},
    {ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_CNF,
        AT_RcvTafPsCallEvtPdpDeactivateCnf},
    {ID_EVT_TAF_PS_CALL_PDP_DEACTIVATE_IND,
        AT_RcvTafPsCallEvtPdpDeactivatedInd},

    {ID_EVT_TAF_PS_CALL_END_CNF,
        AT_RcvTafPsCallEvtCallEndCnf},
    {ID_EVT_TAF_PS_CALL_MODIFY_CNF,
        AT_RcvTafPsCallEvtCallModifyCnf},
    {ID_EVT_TAF_PS_CALL_ANSWER_CNF,
        AT_RcvTafPsCallEvtCallAnswerCnf},
    {ID_EVT_TAF_PS_CALL_HANGUP_CNF,
        AT_RcvTafPsCallEvtCallHangupCnf},

    /* D */
    {ID_EVT_TAF_PS_GET_D_GPRS_ACTIVE_TYPE_CNF,
        AT_RcvTafPsEvtGetGprsActiveTypeCnf},

    /* PPP */
    {ID_EVT_TAF_PS_PPP_DIAL_ORIG_CNF,
        AT_RcvTafPsEvtPppDialOrigCnf},

    /* +CGDCONT */
    {ID_EVT_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtSetPrimPdpContextInfoCnf},
    {ID_EVT_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtGetPrimPdpContextInfoCnf},

    /* +CGDSCONT */
    {ID_EVT_TAF_PS_SET_SEC_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtSetSecPdpContextInfoCnf},
    {ID_EVT_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtGetSecPdpContextInfoCnf},

    /* +CGTFT */
    {ID_EVT_TAF_PS_SET_TFT_INFO_CNF,
        AT_RcvTafPsEvtSetTftInfoCnf},
    {ID_EVT_TAF_PS_GET_TFT_INFO_CNF,
        AT_RcvTafPsEvtGetTftInfoCnf},

    /* +CGEQREQ */
    {ID_EVT_TAF_PS_SET_UMTS_QOS_INFO_CNF,
        AT_RcvTafPsEvtSetUmtsQosInfoCnf},
    {ID_EVT_TAF_PS_GET_UMTS_QOS_INFO_CNF,
        AT_RcvTafPsEvtGetUmtsQosInfoCnf},

    /* +CGEQMIN */
    {ID_EVT_TAF_PS_SET_UMTS_QOS_MIN_INFO_CNF,
        AT_RcvTafPsEvtSetUmtsQosMinInfoCnf},
    {ID_EVT_TAF_PS_GET_UMTS_QOS_MIN_INFO_CNF,
        AT_RcvTafPsEvtGetUmtsQosMinInfoCnf},

    /* +CGEQNEG */
    {ID_EVT_TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_CNF,
        AT_RcvTafPsEvtGetDynamicUmtsQosInfoCnf},

    /* +CGACT */
    {ID_EVT_TAF_PS_SET_PDP_CONTEXT_STATE_CNF,
        AT_RcvTafPsEvtSetPdpStateCnf},
    {ID_EVT_TAF_PS_GET_PDP_CONTEXT_STATE_CNF,
        AT_RcvTafPsEvtGetPdpStateCnf},

    /* +CGPADDR */
    {ID_EVT_TAF_PS_GET_PDP_IP_ADDR_INFO_CNF,
        AT_RcvTafPsEvtGetPdpIpAddrInfoCnf},
    {ID_EVT_TAF_PS_GET_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtGetPdpContextInfoCnf},

    /* +CGAUTO */
    {ID_EVT_TAF_PS_SET_ANSWER_MODE_INFO_CNF,
        AT_RcvTafPsEvtSetAnsModeInfoCnf},
    {ID_EVT_TAF_PS_GET_ANSWER_MODE_INFO_CNF,
        AT_RcvTafPsEvtGetAnsModeInfoCnf},

    /* +CGCONTRDP */
    {ID_EVT_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtGetDynamicPrimPdpContextInfoCnf},
    /* +CGSCONTRDP */
    {ID_EVT_TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_CNF,
        AT_RcvTafPsEvtGetDynamicSecPdpContextInfoCnf},

    /* +CGTFTRDP */
    {ID_EVT_TAF_PS_GET_DYNAMIC_TFT_INFO_CNF,
        AT_RcvTafPsEvtGetDynamicTftInfoCnf},

    /* +CGEQOS */
    {ID_EVT_TAF_PS_SET_EPS_QOS_INFO_CNF,
        AT_RcvTafPsEvtSetEpsQosInfoCnf},
    {ID_EVT_TAF_PS_GET_EPS_QOS_INFO_CNF,
        AT_RcvTafPsEvtGetEpsQosInfoCnf},

    /* +CGEQOSRDP */
    {ID_EVT_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_CNF,
        AT_RcvTafPsEvtGetDynamicEpsQosInfoCnf},

    /* ^CDQF/^DSFLOWQRY */
    {ID_EVT_TAF_PS_GET_DSFLOW_INFO_CNF,
        AT_RcvTafPsEvtGetDsFlowInfoCnf},

    /* ^CDCF/^DSFLOWCLR */
    {ID_EVT_TAF_PS_CLEAR_DSFLOW_CNF,
        AT_RcvTafPsEvtClearDsFlowInfoCnf},

    /* ^CDSF/^DSFLOWRPT/^FLOWRPTCTRL */
    {ID_EVT_TAF_PS_CONFIG_DSFLOW_RPT_CNF,
        AT_RcvTafPsEvtConfigDsFlowRptCnf},

    /* ^DSFLOWRPT */
    {ID_EVT_TAF_PS_REPORT_DSFLOW_IND,
        AT_RcvTafPsEvtReportDsFlowInd},

     /* ^VTFLOWRPT */
    {ID_EVT_TAF_PS_REPORT_VTFLOW_IND,
        AT_RcvTafPsEvtReportVTFlowInd},
    {ID_EVT_TAF_PS_CONFIG_VTFLOW_RPT_CNF,
        AT_RcvTafPsEvtConfigVTFlowRptCnf},

    /* ^CGDNS */
    {ID_EVT_TAF_PS_SET_PDP_DNS_INFO_CNF,
        AT_RcvTafPsEvtSetPdpDnsInfoCnf},
    {ID_EVT_TAF_PS_GET_PDP_DNS_INFO_CNF,
        AT_RcvTafPsEvtGetPdpDnsInfoCnf},

    /* ^AUTHDATA */
    {ID_EVT_TAF_PS_SET_AUTHDATA_INFO_CNF,
        AT_RcvTafPsEvtSetAuthDataInfoCnf},
    {ID_EVT_TAF_PS_GET_AUTHDATA_INFO_CNF,
        AT_RcvTafPsEvtGetAuthDataInfoCnf},

    {ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND,
        AT_RcvTafPsEvtPdpDisconnectInd},
    {ID_EVT_TAF_PS_GET_NEGOTIATION_DNS_CNF,
        AT_RcvTafPsEvtGetDynamicDnsInfoCnf},

#if(FEATURE_ON == FEATURE_LTE)
    {ID_EVT_TAF_PS_LTECS_INFO_CNF,
        atReadLtecsCnfProc},
    {ID_EVT_TAF_PS_CEMODE_INFO_CNF,
        atReadCemodeCnfProc},


    {ID_EVT_TAF_PS_GET_CID_SDF_CNF,
        AT_RcvTafPsEvtGetCidSdfInfoCnf},

    {ID_MSG_TAF_PS_GET_LTE_ATTACH_INFO_CNF,
        AT_RcvTafGetLteAttachInfoCnf},

#endif
    {ID_EVT_TAF_PS_SET_APDSFLOW_RPT_CFG_CNF,
        AT_RcvTafPsEvtSetApDsFlowRptCfgCnf},
    {ID_EVT_TAF_PS_GET_APDSFLOW_RPT_CFG_CNF,
        AT_RcvTafPsEvtGetApDsFlowRptCfgCnf},
    {ID_EVT_TAF_PS_APDSFLOW_REPORT_IND,
        AT_RcvTafPsEvtApDsFlowReportInd},

    {ID_EVT_TAF_PS_SET_DSFLOW_NV_WRITE_CFG_CNF,
        AT_RcvTafPsEvtSetDsFlowNvWriteCfgCnf},
    {ID_EVT_TAF_PS_GET_DSFLOW_NV_WRITE_CFG_CNF,
        AT_RcvTafPsEvtGetDsFlowNvWriteCfgCnf},

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    {ID_EVT_TAF_PS_SET_CTA_INFO_CNF,
        AT_RcvTafPsEvtSetPktCdataInactivityTimeLenCnf},
    {ID_EVT_TAF_PS_GET_CTA_INFO_CNF,
        AT_RcvTafPsEvtGetPktCdataInactivityTimeLenCnf},


    {ID_EVT_TAF_PS_SET_CDMA_DIAL_MODE_CNF,
        At_RcvTafPsEvtSetDialModeCnf},


    {ID_EVT_TAF_PS_CGMTU_VALUE_CHG_IND,
        AT_RcvTafPsEvtCgmtuValueChgInd},
#endif

    {ID_EVT_TAF_PS_SET_IMS_PDP_CFG_CNF,
        AT_RcvTafPsEvtSetImsPdpCfgCnf},

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    {ID_EVT_TAF_PS_SET_1X_DORM_TIMER_CNF,
        AT_RcvTafPsEvtSet1xDormTimerCnf},

    {ID_EVT_TAF_PS_GET_1X_DORM_TIMER_CNF,
        AT_RcvTafPsEvtGet1xDormTimerCnf},
#endif

    /* ����telcel pdp����������Ϣ*/
    {ID_EVT_TAF_PS_CALL_LIMIT_PDP_ACT_IND,
        AT_RcvTafPsCallEvtLimitPdpActInd},

    {ID_EVT_TAF_PS_SET_MIP_MODE_CNF,
        AT_RcvTafPsEvtSetMipModeCnf},

    {ID_EVT_TAF_PS_GET_MIP_MODE_CNF,
        AT_RcvTafPsEvtGetMipModeCnf},

    {ID_EVT_TAF_PS_SET_VZWAPNE_CNF,
        AT_RcvTafPsEvtSetVzwApneCnf},

    {ID_EVT_TAF_PS_GET_VZWAPNE_CNF,
        AT_RcvTafPsEvtGetVzwApneCnf},

    {ID_EVT_TAF_PS_REPORT_PCO_INFO_IND,
        AT_RcvTafPsReportPcoInfoInd},

    {ID_EVT_TAF_PS_SET_DATA_SWITCH_CNF,
        AT_RcvTafPsEvtSetDataSwitchCnf},

    {ID_EVT_TAF_PS_GET_DATA_SWITCH_CNF,
        AT_RcvTafPsEvtGetDataSwitchCnf},

    {ID_EVT_TAF_PS_SET_DATA_ROAM_SWITCH_CNF,
        AT_RcvTafPsEvtSetDataRoamSwitchCnf},

    {ID_EVT_TAF_PS_GET_DATA_ROAM_SWITCH_CNF,
        AT_RcvTafPsEvtGetDataRoamSwitchCnf},

    {ID_EVT_TAF_PS_SET_APN_THROT_INFO_CNF,
        AT_RcvTafPsEvtSetApnThrotInfoCnf},

    {ID_EVT_TAF_PS_EPDG_CTRLU_NTF,
        AT_RcvTafPsEvtEpdgCtrluNtf},

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* +C5GQOS */
    {ID_EVT_TAF_PS_SET_5G_QOS_INFO_CNF,
        AT_RcvTafPsEvtSet5gQosInfoCnf},
    {ID_EVT_TAF_PS_GET_5G_QOS_INFO_CNF,
        AT_RcvTafPsEvtGet5gQosInfoCnf},

    /* +C5GQOSRDP */
    {ID_EVT_TAF_PS_GET_DYNAMIC_5G_QOS_INFO_CNF,
        AT_RcvTafPsEvtGetDynamic5gQosInfoCnf},

    /* ^CPOLICYRPT */
    {ID_EVT_TAF_PS_UE_POLICY_RPT_IND,
        AT_RcvTafPsEvtUePolicyRptInd},

    /* ^CPOLICYRPT */
    {ID_EVT_TAF_PS_SET_UE_POLICY_RPT_CNF,
        AT_RcvTafPsEvtSetUePolicyRptCnf},

    /* ^CPOLICYCODE */
    {ID_EVT_TAF_PS_GET_UE_POLICY_CNF,
        AT_RcvTafPsEvtGetUePolicyCnf},
#endif

    {ID_EVT_TAF_PS_SET_ROAMING_PDP_TYPE_CNF,
        AT_RcvTafPsEvtSetRoamPdpTypeCnf},
};

/* �����ϱ����������Bitλ��Ӧ�� */
/* �����Ӧ˳��ΪBit0~Bit63 */
AT_RPT_CMD_INDEX_ENUM_UINT8             g_aenAtCurcRptCmdTable[] =
{
    AT_RPT_CMD_MODE,        AT_RPT_CMD_RSSI,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_SRVST,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_SIMST,       AT_RPT_CMD_TIME,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_ANLEVEL,     AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_SMMEMFULL,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_CTZV,
    AT_RPT_CMD_CTZE,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_DSFLOWRPT,   AT_RPT_CMD_BUTT,
    AT_RPT_CMD_ORIG,        AT_RPT_CMD_CONF,        AT_RPT_CMD_CONN,        AT_RPT_CMD_CEND,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_STIN,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_CERSSI,      AT_RPT_CMD_LWCLASH,     AT_RPT_CMD_XLEMA,       AT_RPT_CMD_ACINFO,
    AT_RPT_CMD_PLMN,        AT_RPT_CMD_CALLSTATE,   AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT
};



AT_RPT_CMD_INDEX_ENUM_UINT8             g_aenAtUnsolicitedRptCmdTable[] =
{
    AT_RPT_CMD_MODE,        AT_RPT_CMD_RSSI,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_SRVST,
    AT_RPT_CMD_CREG,        AT_RPT_CMD_SIMST,       AT_RPT_CMD_TIME,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_ANLEVEL,     AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_CTZV,
    AT_RPT_CMD_CTZE,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_DSFLOWRPT,   AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_CUSD,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_CSSI,
    AT_RPT_CMD_CSSU,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_CERSSI,      AT_RPT_CMD_LWURC,       AT_RPT_CMD_BUTT,        AT_RPT_CMD_CUUS1U,
    AT_RPT_CMD_CUUS1I,      AT_RPT_CMD_CGREG,       AT_RPT_CMD_CEREG,       AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT,
    AT_RPT_CMD_BUTT,        AT_RPT_CMD_C5GREG,      AT_RPT_CMD_BUTT,        AT_RPT_CMD_BUTT
};

AT_CME_CALL_ERR_CODE_MAP_STRU           g_astAtCmeCallErrCodeMapTbl[] =
{
    { AT_CME_INCORRECT_PARAMETERS,      TAF_CS_CAUSE_INVALID_PARAMETER              },
    { AT_CME_SIM_FAILURE,               TAF_CS_CAUSE_SIM_NOT_EXIST                  },
    { AT_CME_SIM_PIN_REQUIRED,          TAF_CS_CAUSE_SIM_PIN_NEED                   },
    { AT_CME_UNKNOWN,                   TAF_CS_CAUSE_NO_CALL_ID                     },
    { AT_CME_OPERATION_NOT_ALLOWED,     TAF_CS_CAUSE_NOT_ALLOW                      },
    { AT_CME_INCORRECT_PARAMETERS,      TAF_CS_CAUSE_STATE_ERROR                    },
    { AT_CME_FDN_FAILED,                            TAF_CS_CAUSE_FDN_CHECK_FAILURE              },
    { AT_CME_CALL_CONTROL_BEYOND_CAPABILITY,        TAF_CS_CAUSE_CALL_CTRL_BEYOND_CAPABILITY    },
    { AT_CME_CALL_CONTROL_FAILED,                   TAF_CS_CAUSE_CALL_CTRL_TIMEOUT              },
    { AT_CME_CALL_CONTROL_FAILED,                   TAF_CS_CAUSE_CALL_CTRL_NOT_ALLOWED          },
    { AT_CME_CALL_CONTROL_FAILED,                   TAF_CS_CAUSE_CALL_CTRL_INVALID_PARAMETER    },
    { AT_CME_UNKNOWN,                   TAF_CS_CAUSE_UNKNOWN                        }
};

AT_CMS_SMS_ERR_CODE_MAP_STRU           g_astAtCmsSmsErrCodeMapTbl[] =
{
    { AT_CMS_U_SIM_BUSY,                            MN_ERR_CLASS_SMS_UPDATE_USIM},
    { AT_CMS_U_SIM_NOT_INSERTED,                    MN_ERR_CLASS_SMS_NOUSIM},
    { AT_CMS_INVALID_MEMORY_INDEX,                  MN_ERR_CLASS_SMS_EMPTY_REC},
    { AT_CMS_MEMORY_FULL,                           MN_ERR_CLASS_SMS_STORAGE_FULL},
    { AT_CMS_U_SIM_PIN_REQUIRED,                    MN_ERR_CLASS_SMS_NEED_PIN1},
    { AT_CMS_U_SIM_PUK_REQUIRED,                    MN_ERR_CLASS_SMS_NEED_PUK1},
    { AT_CMS_U_SIM_FAILURE,                         MN_ERR_CLASS_SMS_UNAVAILABLE},
    { AT_CMS_OPERATION_NOT_ALLOWED,                 MN_ERR_CLASS_SMS_FEATURE_INAVAILABLE },
    { AT_CMS_SMSC_ADDRESS_UNKNOWN,                  MN_ERR_CLASS_SMS_INVALID_SCADDR},
    { AT_CMS_INVALID_PDU_MODE_PARAMETER,            MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW},
    { AT_CMS_FDN_DEST_ADDR_FAILED,                  MN_ERR_CLASS_FDN_CHECK_DN_FAILURE},
    { AT_CMS_FDN_SERVICE_CENTER_ADDR_FAILED,        MN_ERR_CLASS_FDN_CHECK_SC_FAILURE},
    { AT_CMS_MO_SMS_CONTROL_FAILED,                 MN_ERR_CLASS_SMS_MO_CTRL_ACTION_NOT_ALLOWED},
    { AT_CMS_MO_SMS_CONTROL_FAILED,                 MN_ERR_CLASS_SMS_MO_CTRL_USIM_PARA_ERROR},
    { AT_CMS_MEMORY_FAILURE,                        MN_ERR_NOMEM}

};


#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON)&&(FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON))
AT_ENCRYPT_VOICE_ERR_CODE_MAP_STRU                  g_astAtEncVoiceErrCodeMapTbl[] =
{
    { AT_ENCRYPT_VOICE_SUCC,                                TAF_CALL_ENCRYPT_VOICE_SUCC},
    { AT_ENCRYPT_VOICE_TIMEOUT,                             TAF_CALL_ENCRYPT_VOICE_TIMEOUT},
    { AT_ENCRYPT_VOICE_TIMEOUT,                             TAF_CALL_ENCRYPT_VOICE_TX01_TIMEOUT},
    { AT_ENCRYPT_VOICE_TIMEOUT,                             TAF_CALL_ENCRYPT_VOICE_TX02_TIMEOUT},
    { AT_ENCRYPT_VOICE_LOCAL_TERMINAL_NO_AUTHORITY,         TAF_CALL_ENCRYPT_VOICE_LOCAL_TERMINAL_NO_AUTHORITY},
    { AT_ENCRYPT_VOICE_REMOTE_TERMINAL_NO_AUTHORITY,        TAF_CALL_ENCRYPT_VOICE_REMOTE_TERMINAL_NO_AUTHORITY},
    { AT_ENCRYPT_VOICE_LOCAL_TERMINAL_ILLEGAL,              TAF_CALL_ENCRYPT_VOICE_LOCAL_TERMINAL_ILLEGAL},
    { AT_ENCRYPT_VOICE_REMOTE_TERMINAL_ILLEGAL,             TAF_CALL_ENCRYPT_VOICE_REMOTE_TERMINAL_ILLEGAL},
    { AT_ENCRYPT_VOICE_UNKNOWN_ERROR,                       TAF_CALL_ENCRYPT_VOICE_UNKNOWN_ERROR },
    { AT_ENCRYPT_VOICE_SIGNTURE_VERIFY_FAILURE,             TAF_CALL_ENCRYPT_VOICE_SIGNTURE_VERIFY_FAILURE},
    { AT_ENCRYPT_VOICE_MT_CALL_NOTIFICATION,                TAF_CALL_ENCRYPT_VOICE_MT_CALL_NOTIFICATION},

    /* Internal err code */
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_FAIL,               TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_FAIL},
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_POOL_FULL,          TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_POOL_FULL},
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_LINK_ERR,           TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_LINK_ERR},
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_NO_TL_ACK,          TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_NO_TL_ACK},
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_ENCODE_ERR,         TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_ENCODE_ERR},
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_UNKNOWN,            TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_UNKNOWN},
    { AT_ENCRYPT_VOICE_SO_NEGO_FAILURE,                     TAF_CALL_ENCRYPT_VOICE_SO_NEGO_FAILURE},
    { AT_ENCRYPT_VOICE_TWO_CALL_ENTITY_EXIST,               TAT_CALL_APP_ENCRYPT_VOICE_TWO_CALL_ENTITY_EXIST},
    { AT_ENCRYPT_VOICE_NO_MO_CALL,                          TAF_CALL_ENCRYPT_VOICE_NO_MO_CALL},
    { AT_ENCRYPT_VOICE_NO_MT_CALL,                          TAF_CALL_ENCRYPT_VOICE_NO_MT_CALL},
    { AT_ENCRYPT_VOICE_NO_CALL_EXIST,                       TAF_CALL_ENCRYPT_VOICE_NO_CALL_EXIST},
    { AT_ENCRYPT_VOICE_CALL_STATE_NOT_ALLOWED,              TAF_CALL_ENCRYPT_VOICE_CALL_STATE_NOT_ALLOWED},
    { AT_ENCRYPT_VOICE_CALL_NUM_MISMATCH,                   TAF_CALL_ENCRYPT_VOICE_CALL_NUM_MISMATCH},
    { AT_ENCRYPT_VOICE_ENC_VOICE_STATE_MISMATCH,            TAF_CALL_ENCRYPT_VOICE_ENC_VOICE_STATE_MISMATCH},
    { AT_ENCRYPT_VOICE_MSG_ENCODE_FAILUE,                   TAF_CALL_ENCRYPT_VOICE_MSG_ENCODE_FAILUE},
    { AT_ENCRYPT_VOICE_MSG_DECODE_FAILUE,                   TAF_CALL_ENCRYPT_VOICE_MSG_DECODE_FAILUE},
    { AT_ENCRYPT_VOICE_GET_TEMP_PUB_PIVA_KEY_FAILURE,       TAF_CALL_ENCRYPT_VOICE_GET_TEMP_PUB_PIVA_KEY_FAILURE},
    { AT_ENCRYPT_VOICE_FILL_CIPHER_TEXT_FAILURE,            TAF_CALL_ENCRYPT_VOICE_FILL_CIPHER_TEXT_FAILURE},
    { AT_ENCRYPT_VOICE_ECC_CAP_NOT_SUPPORTED,               TAF_CALL_ENCRYPT_VOICE_ECC_CAP_NOT_SUPPORTED},
    { AT_ENCRYPT_VOICE_ENC_VOICE_MODE_UNKNOWN,              TAF_CALL_ENCRYPT_VOICE_ENC_VOICE_MODE_UNKNOWN},
    { AT_ENCRYPT_VOICE_ENC_VOICE_MODE_MIMATCH,              TAF_CALL_ENCRYPT_VOICE_ENC_VOICE_MODE_MIMATCH},
    { AT_ENCRYPT_VOICE_CALL_RELEASED,                       TAF_CALL_ENCRYPT_VOICE_CALL_RELEASED},
    { AT_ENCRYPT_VOICE_CALL_ANSWER_REQ_FAILURE,             TAF_CALL_ENCRYPT_VOICE_CALL_ANSWER_REQ_FAILURE},
    { AT_ENCRYPT_VOICE_DECRYPT_KS_FAILURE,                  TAF_CALL_ENCRYPT_VOICE_DECRYPT_KS_FAILURE},
    { AT_ENCRYPT_VOICE_FAILURE_CAUSED_BY_INCOMING_CALL,     TAF_CALL_ENCRYPT_VOICE_FAILURE_CAUSED_BY_INCOMING_CALL},
    { AT_ENCRYPT_VOICE_INIT_VOICE_FUNC_FAILURE,             TAF_CALL_ENCRYPT_VOICE_INIT_VOICE_FUNC_FAILURE},
    { AT_ENCRYPT_VOICE_ERROR_ENUM_BUTT,                     TAF_CALL_ENCRYPT_VOICE_STATUS_ENUM_BUTT}

};
#endif

AT_SMS_RSP_PROC_FUN g_aAtSmsMsgProcTable[MN_MSG_EVT_MAX] = {
    /*MN_MSG_EVT_SUBMIT_RPT*/           At_SendSmRspProc,
    /*MN_MSG_EVT_MSG_SENT*/             At_SetCnmaRspProc,
    /*MN_MSG_EVT_MSG_STORED*/           At_SmsRspNop,
    /*MN_MSG_EVT_DELIVER*/              At_SmsDeliverProc,
    /*MN_MSG_EVT_DELIVER_ERR*/          At_SmsDeliverErrProc,
    /*MN_MSG_EVT_SM_STORAGE_LIST*/      At_SmsStorageListProc,                  /*���������ϱ�����Ӧ��Ϣ�Ĵ���*/
    /*MN_MSG_EVT_STORAGE_FULL*/         At_SmsRspNop,
    /*MN_MSG_EVT_STORAGE_EXCEED*/       At_SmsStorageExceedProc,
    /*MN_MSG_EVT_READ*/                 At_ReadRspProc,
    /*MN_MSG_EVT_LIST*/                 At_ListRspProc,
    /*MN_MSG_EVT_WRITE*/                At_WriteSmRspProc,
    /*MN_MSG_EVT_DELETE*/               At_DeleteRspProc,
    /*MN_MSG_EVT_DELETE_TEST*/          At_DeleteTestRspProc,
    /*MN_MSG_EVT_MODIFY_STATUS*/        At_SmsModSmStatusRspProc,
    /*MN_MSG_EVT_WRITE_SRV_PARM*/       At_SetCscaCsmpRspProc,
    /*MN_MSG_EVT_READ_SRV_PARM*/        AT_QryCscaRspProc,
    /*MN_MSG_EVT_SRV_PARM_CHANGED*/     At_SmsSrvParmChangeProc,
    /*MN_MSG_EVT_DELETE_SRV_PARM*/      At_SmsRspNop,
    /*MN_MSG_EVT_READ_STARPT*/          At_SmsRspNop,
    /*MN_MSG_EVT_DELETE_STARPT*/        At_SmsRspNop,
    /*MN_MSG_EVT_SET_MEMSTATUS*/        AT_SetMemStatusRspProc,
    /*MN_MSG_EVT_MEMSTATUS_CHANGED*/    At_SmsRspNop,
    /*MN_MSG_EVT_MATCH_MO_STARPT_INFO*/ At_SmsRspNop,
    /*MN_MSG_EVT_SET_RCVMSG_PATH*/      At_SetRcvPathRspProc,
    /*MN_MSG_EVT_GET_RCVMSG_PATH*/      At_SmsRspNop,
    /*MN_MSG_EVT_RCVMSG_PATH_CHANGED*/  At_SmsRcvMsgPathChangeProc,
    /*MN_MSG_EVT_INIT_SMSP_RESULT*/     At_SmsInitSmspResultProc,
    /*MN_MSG_EVT_INIT_RESULT*/          At_SmsInitResultProc,
    /*MN_MSG_EVT_SET_LINK_CTRL_PARAM*/  At_SetCmmsRspProc,
    /*MN_MSG_EVT_GET_LINK_CTRL_PARAM*/  At_GetCmmsRspProc,
    /*MN_MSG_EVT_STUB_RESULT*/          At_SmsStubRspProc,
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    /*MN_MSG_EVT_DELIVER_CBM*/          At_SmsDeliverCbmProc,
    /*MN_MSG_EVT_GET_CBTYPE*/           At_GetCbActiveMidsRspProc,
    /*MN_MSG_EVT_ADD_CBMIDS*/           AT_ChangeCbMidsRsp,
    /*MN_MSG_EVT_DELETE_CBMIDS*/        AT_ChangeCbMidsRsp,
    /*MN_MSG_EVT_DELETE_ALL_CBMIDS*/    AT_ChangeCbMidsRsp,

#if (FEATURE_ETWS == FEATURE_ON)
    /*MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY*/  At_ProcDeliverEtwsPrimNotify,
#else
    /*MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY*/  At_SmsRspNop,
#endif

#else
    /*MN_MSG_EVT_DELIVER_CBM*/          At_SmsRspNop,
    /*MN_MSG_EVT_GET_CBTYPE*/           At_SmsRspNop,
    /*MN_MSG_EVT_ADD_CBMIDS*/           At_SmsRspNop,
    /*MN_MSG_EVT_DELETE_CBMIDS*/        At_SmsRspNop,
    /*MN_MSG_EVT_DELETE_ALL_CBMIDS*/    At_SmsRspNop
    /*MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY*/   At_SmsRspNop,
#endif
   /*MN_MSG_EVT_SET_SEND_DOMAIN_PARAM*/ At_SetCgsmsRspProc,
   /*MN_MSG_EVT_GET_SEND_DOMAIN_PARAM*/ At_GetCgsmsRspProc,
};

/*�ṹ�����һ�����������ʾ��β*/
AT_QUERY_TYPE_FUNC_STRU     g_aAtQryTypeProcFuncTbl[] =
{
    {TAF_PH_IMSI_ID_PARA,              At_QryParaRspCimiProc},
    {TAF_PH_MS_CLASS_PARA,             At_QryParaRspCgclassProc},


    {TAF_PH_ICC_ID,                    At_QryParaRspIccidProc},
    {TAF_PH_PNN_PARA,                  At_QryParaRspPnnProc},
    {TAF_PH_CPNN_PARA,                 At_QryParaRspCPnnProc},
    {TAF_PH_OPL_PARA,                  At_QryParaRspOplProc},


    {TAF_PH_PNN_RANGE_PARA,            At_QryRspUsimRangeProc},
    {TAF_PH_OPL_RANGE_PARA,            At_QryRspUsimRangeProc},


    {TAF_TELE_PARA_BUTT,               TAF_NULL_PTR}
};

#if (VOS_WIN32 == VOS_OS_VER)
LOCAL TAF_UINT8                         gaucAtStin[] = "^STIN:";
LOCAL TAF_UINT8                         gaucAtStmn[] = "^STMN:";
LOCAL TAF_UINT8                         gaucAtStgi[] = "^STGI:";
LOCAL TAF_UINT8                         gaucAtStsf[] = "^STSF:";
LOCAL TAF_UINT8                         gaucAtCsin[] = "^CSIN:";
LOCAL TAF_UINT8                         gaucAtCstr[] = "^CSTR:";
LOCAL TAF_UINT8                         gaucAtCsen[] = "^CSEN:";
LOCAL TAF_UINT8                         gaucAtCsmn[] = "^CSMN:";
LOCAL TAF_UINT8                         gaucAtCcin[] = "^CCIN:";
#else
TAF_UINT8                               gaucAtStin[] = "^STIN:";
TAF_UINT8                               gaucAtStmn[] = "^STMN:";
TAF_UINT8                               gaucAtStgi[] = "^STGI:";
TAF_UINT8                               gaucAtStsf[] = "^STSF:";
TAF_UINT8                               gaucAtCsin[] = "^CSIN:";
TAF_UINT8                               gaucAtCstr[] = "^CSTR:";
TAF_UINT8                               gaucAtCsen[] = "^CSEN:";
TAF_UINT8                               gaucAtCsmn[] = "^CSMN:";
TAF_UINT8                               gaucAtCcin[] = "^CCIN:";
#endif

static AT_CALL_CUUSU_MSG_STRU g_stCuusuMsgType[] =
{
    {MN_CALL_UUS1_MSG_SETUP             ,   AT_CUUSU_MSG_SETUP              },
    {MN_CALL_UUS1_MSG_DISCONNECT        ,   AT_CUUSU_MSG_DISCONNECT         },
    {MN_CALL_UUS1_MSG_RELEASE_COMPLETE  ,   AT_CUUSU_MSG_RELEASE_COMPLETE   }
};

static AT_CALL_CUUSI_MSG_STRU g_stCuusiMsgType[] =
{
    {MN_CALL_UUS1_MSG_ALERT             ,   AT_CUUSI_MSG_ALERT              },
    {MN_CALL_UUS1_MSG_PROGRESS          ,   AT_CUUSI_MSG_PROGRESS           },
    {MN_CALL_UUS1_MSG_CONNECT           ,   AT_CUUSI_MSG_CONNECT            },
    {MN_CALL_UUS1_MSG_RELEASE           ,   AT_CUUSI_MSG_RELEASE            }
};

/* begin V7R1 PhaseI Modify */
static AT_PH_SYS_MODE_TBL_STRU g_astSysModeTbl[] =
{
    {MN_PH_SYS_MODE_EX_NONE_RAT     ,"NO SERVICE"},
    {MN_PH_SYS_MODE_EX_GSM_RAT      ,"GSM"},
    {MN_PH_SYS_MODE_EX_CDMA_RAT     ,"CDMA"},
    {MN_PH_SYS_MODE_EX_WCDMA_RAT    ,"WCDMA"},
    {MN_PH_SYS_MODE_EX_TDCDMA_RAT   ,"TD-SCDMA"},
    {MN_PH_SYS_MODE_EX_WIMAX_RAT    ,"WIMAX"},
    {MN_PH_SYS_MODE_EX_LTE_RAT      ,"LTE"},
    {MN_PH_SYS_MODE_EX_EVDO_RAT     ,"EVDO"},
    {MN_PH_SYS_MODE_EX_HYBRID_RAT   ,"CDMA1X+EVDO(HYBRID)"},
    {MN_PH_SYS_MODE_EX_SVLTE_RAT    ,"CDMA1X+LTE"},
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    {MN_PH_SYS_MODE_EX_EUTRAN_5GC_RAT,  "EUTRAN-5GC"},
    {MN_PH_SYS_MODE_EX_NR_5GC_RAT,      "NR-5GC"},
#endif
};

AT_PH_SUB_SYS_MODE_TBL_STRU g_astSubSysModeTbl[] =
{
    {MN_PH_SUB_SYS_MODE_EX_NONE_RAT         ,"NO SERVICE"},
    {MN_PH_SUB_SYS_MODE_EX_GSM_RAT          ,"GSM"},
    {MN_PH_SUB_SYS_MODE_EX_GPRS_RAT         ,"GPRS"},
    {MN_PH_SUB_SYS_MODE_EX_EDGE_RAT         ,"EDGE"},
    {MN_PH_SUB_SYS_MODE_EX_WCDMA_RAT        ,"WCDMA"},
    {MN_PH_SUB_SYS_MODE_EX_HSDPA_RAT        ,"HSDPA"},
    {MN_PH_SUB_SYS_MODE_EX_HSUPA_RAT        ,"HSUPA"},
    {MN_PH_SUB_SYS_MODE_EX_HSPA_RAT         ,"HSPA"},
    {MN_PH_SUB_SYS_MODE_EX_HSPA_PLUS_RAT    ,"HSPA+"},
    {MN_PH_SUB_SYS_MODE_EX_DCHSPA_PLUS_RAT  ,"DC-HSPA+"},
    {MN_PH_SUB_SYS_MODE_EX_TDCDMA_RAT       ,"TD-SCDMA"},
    {MN_PH_SUB_SYS_MODE_EX_TD_HSDPA_RAT     ,"HSDPA"},
    {MN_PH_SUB_SYS_MODE_EX_TD_HSUPA_RAT     ,"HSUPA"},
    {MN_PH_SUB_SYS_MODE_EX_TD_HSPA_RAT      ,"HSPA"},
    {MN_PH_SUB_SYS_MODE_EX_TD_HSPA_PLUS_RAT ,"HSPA+"},

    {MN_PH_SUB_SYS_MODE_EX_LTE_RAT          ,"LTE"},

    {MN_PH_SUB_SYS_MODE_EX_CDMA20001X_RAT   ,"CDMA2000 1X"},

    {MN_PH_SUB_SYS_MODE_EX_EVDOREL0_RAT        ,"EVDO Rel0"},
    {MN_PH_SUB_SYS_MODE_EX_EVDORELA_RAT        ,"EVDO RelA"},
    {MN_PH_SUB_SYS_MODE_EX_HYBIRD_EVDOREL0_RAT ,"HYBRID(EVDO Rel0)"},
    {MN_PH_SUB_SYS_MODE_EX_HYBIRD_EVDORELA_RAT ,"HYBRID(EVDO RelA)"},

    {MN_PH_SUB_SYS_MODE_EX_EHRPD_RAT           ,"EHRPD"},

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    {MN_PH_SUB_SYS_MODE_EX_EUTRAN_5GC_RAT      ,"EUTRAN-5GC"},
    {MN_PH_SUB_SYS_MODE_EX_NR_5GC_RAT          ,"NR-5GC"},
#endif

};
/* end V7R1 PhaseI Modify */

#if(FEATURE_ON == FEATURE_LTE)
VOS_UINT32  g_ulGuTmodeCnf  = 0;
VOS_UINT32  g_ulLteTmodeCnf = 0;
 #endif

/* +CLCK�������CLASS��Service Type Code��Ӧ��չ�� */
AT_CLCK_CLASS_SERVICE_TBL_STRU          g_astClckClassServiceExtTbl[] = {
    {AT_CLCK_PARA_CLASS_VOICE,                      TAF_SS_TELE_SERVICE,        TAF_ALL_SPEECH_TRANSMISSION_SERVICES_TSCODE},
    {AT_CLCK_PARA_CLASS_VOICE,                      TAF_SS_TELE_SERVICE,        TAF_TELEPHONY_TSCODE},
    {AT_CLCK_PARA_CLASS_VOICE,                      TAF_SS_TELE_SERVICE,        TAF_EMERGENCY_CALLS_TSCODE},
    {AT_CLCK_PARA_CLASS_DATA,                       TAF_SS_BEARER_SERVICE,      TAF_ALL_BEARERSERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA,                       TAF_SS_TELE_SERVICE,        TAF_ALL_DATA_TELESERVICES_TSCODE},
    {AT_CLCK_PARA_CLASS_FAX,                        TAF_SS_TELE_SERVICE,        TAF_ALL_FACSIMILE_TRANSMISSION_SERVICES_TSCODE},
    {AT_CLCK_PARA_CLASS_FAX,                        TAF_SS_TELE_SERVICE,        TAF_FACSIMILE_GROUP3_AND_ALTER_SPEECH_TSCODE},
    {AT_CLCK_PARA_CLASS_FAX,                        TAF_SS_TELE_SERVICE,        TAF_AUTOMATIC_FACSIMILE_GROUP3_TSCODE},
    {AT_CLCK_PARA_CLASS_FAX,                        TAF_SS_TELE_SERVICE,        TAF_FACSIMILE_GROUP4_TSCODE},
    {AT_CLCK_PARA_CLASS_VOICE_DATA_FAX,             TAF_SS_TELE_SERVICE,        TAF_ALL_TELESERVICES_EXEPTSMS_TSCODE},
    {AT_CLCK_PARA_CLASS_SMS,                        TAF_SS_TELE_SERVICE,        TAF_ALL_SMS_SERVICES_TSCODE},
    {AT_CLCK_PARA_CLASS_SMS,                        TAF_SS_TELE_SERVICE,        TAF_SMS_MT_PP_TSCODE},
    {AT_CLCK_PARA_CLASS_SMS,                        TAF_SS_TELE_SERVICE,        TAF_SMS_MO_PP_TSCODE},
    {AT_CLCK_PARA_CLASS_VOICE_DATA_FAX_SMS,         TAF_SS_TELE_SERVICE,        TAF_ALL_TELESERVICES_TSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_ALL_DATA_CIRCUIT_SYNCHRONOUS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_ALL_DATACDS_SERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_DATACDS_1200BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_DATACDS_2400BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_DATACDS_4800BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_DATACDS_9600BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_ALL_ALTERNATE_SPEECH_DATACDS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_ALL_SPEECH_FOLLOWED_BY_DATACDS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC,                  TAF_SS_BEARER_SERVICE,      TAF_GENERAL_DATACDS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_ALL_DATA_CIRCUIT_ASYNCHRONOUS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_ALL_DATACDA_SERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_DATACDA_300BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_DATACDA_1200BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_DATACDA_1200_75BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_DATACDA_2400BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_DATACDA_4800BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_DATACDA_9600BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_ALL_ALTERNATE_SPEECH_DATACDA_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC,                 TAF_SS_BEARER_SERVICE,      TAF_ALL_SPEECH_FOLLOWED_BY_DATACDA_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PKT,                   TAF_SS_BEARER_SERVICE,      TAF_ALL_DATAPDS_SERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PKT,                   TAF_SS_BEARER_SERVICE,      TAF_DATAPDS_2400BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PKT,                   TAF_SS_BEARER_SERVICE,      TAF_DATAPDS_4800BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PKT,                   TAF_SS_BEARER_SERVICE,      TAF_DATAPDS_9600BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC_PKT,              TAF_SS_BEARER_SERVICE,      TAF_ALL_SYNCHRONOUS_SERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_ALL_PADACCESSCA_SERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_PADACCESSCA_300BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_PADACCESSCA_1200BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_PADACCESSCA_1200_75BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_PADACCESSCA_2400BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_PADACCESSCA_4800BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_PAD,                   TAF_SS_BEARER_SERVICE,      TAF_PADACCESSCA_9600BPS_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_ASYNC_PAD,             TAF_SS_BEARER_SERVICE,      TAF_ALL_ASYNCHRONOUS_SERVICES_BSCODE},
    {AT_CLCK_PARA_CLASS_DATA_SYNC_ASYNC_PKT_PKT,    TAF_SS_BEARER_SERVICE,      TAF_ALL_BEARERSERVICES_BSCODE},
};

AT_CHG_TAF_ERR_CODE_TBL_STRU                g_astAtChgTafErrCodeTbl[] = {
    {TAF_ERR_GET_CSQLVL_FAIL,                       AT_ERROR},
    {TAF_ERR_USIM_SVR_OPLMN_LIST_INAVAILABLE,       AT_ERROR},
    {TAF_ERR_TIME_OUT,                              AT_CME_NETWORK_TIMEOUT},
    {TAF_ERR_USIM_SIM_CARD_NOTEXIST,                AT_CME_SIM_NOT_INSERTED},
    {TAF_ERR_NEED_PIN1,                             AT_CME_SIM_PIN_REQUIRED},
    {TAF_ERR_NEED_PUK1,                             AT_CME_SIM_PUK_REQUIRED},
    {TAF_ERR_SIM_FAIL,                              AT_CME_SIM_FAILURE},
    {TAF_ERR_PB_STORAGE_OP_FAIL,                    AT_CME_SIM_FAILURE},
    {TAF_ERR_UNSPECIFIED_ERROR,                     AT_CME_UNKNOWN},
    {TAF_ERR_PARA_ERROR,                            AT_CME_INCORRECT_PARAMETERS},
    {TAF_ERR_SS_NEGATIVE_PASSWORD_CHECK,            AT_CME_INCORRECT_PASSWORD},
    {TAF_ERR_SIM_BUSY,                              AT_CME_SIM_BUSY},
    {TAF_ERR_SIM_LOCK,                              AT_CME_PH_SIM_PIN_REQUIRED},
    {TAF_ERR_SIM_INCORRECT_PASSWORD,                AT_CME_INCORRECT_PASSWORD},
    {TAF_ERR_PB_NOT_FOUND,                          AT_CME_NOT_FOUND},
    {TAF_ERR_PB_DIAL_STRING_TOO_LONG,               AT_CME_DIAL_STRING_TOO_LONG},
    {TAF_ERR_PB_STORAGE_FULL,                       AT_CME_MEMORY_FULL},
    {TAF_ERR_PB_WRONG_INDEX,                        AT_CME_INVALID_INDEX},
    {TAF_ERR_CMD_TYPE_ERROR,                        AT_CME_OPERATION_NOT_ALLOWED},
    {TAF_ERR_FILE_NOT_EXIST,                        AT_CME_FILE_NOT_EXISTS},
    {TAF_ERR_NO_NETWORK_SERVICE,                    AT_CME_NO_NETWORK_SERVICE},
    {TAF_ERR_AT_ERROR,                              AT_ERROR},
    {TAF_ERR_CME_OPT_NOT_SUPPORTED,                 AT_CME_OPERATION_NOT_SUPPORTED},
    {TAF_ERR_NET_SEL_MENU_DISABLE,                  AT_CME_NET_SEL_MENU_DISABLE},
    {TAF_ERR_SYSCFG_CS_IMS_SERV_EXIST,              AT_CME_CS_IMS_SERV_EXIST},
    {TAF_ERR_NO_RF,                                 AT_CME_NO_RF},
    {TAF_ERR_NEED_PUK2,                             AT_CME_SIM_PUK2_REQUIRED},
    {TAF_ERR_BUSY_ON_USSD,                          AT_CME_OPERATION_NOT_SUPPORTED},
    {TAF_ERR_BUSY_ON_SS,                            AT_CME_OPERATION_NOT_SUPPORTED},
    {TAF_ERR_SS_NET_TIMEOUT,                        AT_CME_NETWORK_TIMEOUT},
    {TAF_ERR_NO_SUCH_ELEMENT,                       AT_CME_NO_SUCH_ELEMENT},
    {TAF_ERR_MISSING_RESOURCE,                      AT_CME_MISSING_RESOURCE},
    {TAF_ERR_IMS_NOT_SUPPORT,                       AT_CME_IMS_NOT_SUPPORT},
    {TAF_ERR_IMS_SERVICE_EXIST,                     AT_CME_IMS_SERVICE_EXIST},
    {TAF_ERR_IMS_VOICE_DOMAIN_PS_ONLY,              AT_CME_IMS_VOICE_DOMAIN_PS_ONLY},
    {TAF_ERR_IMS_STACK_TIMEOUT,                     AT_CME_IMS_STACK_TIMEOUT},
    {TAF_ERR_IMS_OPEN_LTE_NOT_SUPPORT,              AT_CME_IMS_OPEN_LTE_NOT_SUPPORT},
    {TAF_ERR_1X_RAT_NOT_SUPPORTED,                  AT_CME_1X_RAT_NOT_SUPPORTED},
    {TAF_ERR_SILENT_AES_DEC_PIN_FAIL,               AT_CME_SILENT_AES_DEC_PIN_ERROR},
    {TAF_ERR_SILENT_VERIFY_PIN_ERR,                 AT_CME_SILENT_VERIFY_PIN_ERROR},
    {TAF_ERR_SILENT_AES_ENC_PIN_FAIL,               AT_CME_SILENT_AES_ENC_PIN_ERROR},
    {TAF_ERR_NOT_FIND_FILE,                         AT_CME_NOT_FIND_FILE},
    {TAF_ERR_NOT_FIND_NV,                           AT_CME_NOT_FIND_NV},
    {TAF_ERR_MODEM_ID_ERROR,                        AT_CME_MODEM_ID_ERROR},
    {TAF_ERR_NV_NOT_SUPPORT_ERR,                    AT_CME_NV_NOT_SUPPORT_ERR},
    {TAF_ERR_WRITE_NV_TIMEOUT,                      AT_CME_WRITE_NV_TimeOut},
    {TAF_ERR_NETWORK_FAILURE,                       AT_CME_NETWORK_FAILURE},
    {TAF_ERR_SCI_ERROR,                             AT_CME_SCI_ERROR},

    {TAF_ERR_EMAT_OPENCHANNEL_ERROR ,               AT_ERR_EMAT_OPENCHANNEL_ERROR},
    {TAF_ERR_EMAT_OPENCHANNEL_CNF_ERROR ,           AT_ERR_EMAT_OPENCHANNEL_CNF_ERROR},
    {TAF_ERR_EMAT_CLOSECHANNEL_ERROR,               AT_ERR_EMAT_CLOSECHANNEL_ERROR},
    {TAF_ERR_EMAT_CLOSECHANNEL_CNF_ERROR,           AT_ERR_EMAT_CLOSECHANNEL_CNF_ERROR},
    {TAF_ERR_EMAT_GETEID_ERROR ,                    AT_ERR_EMAT_GETEID_ERROR},
    {TAF_ERR_EMAT_GETEID_DATA_ERROR,                AT_ERR_EMAT_GETEID_DATA_ERROR},
    {TAF_ERR_EMAT_GETPKID_ERROR,                    AT_ERR_EMAT_GETPKID_ERROR},
    {TAF_ERR_EMAT_GETPKID_DATA_ERROR,               AT_ERR_EMAT_GETPKID_DATA_ERROR},
    {TAF_ERR_EMAT_CLEANPROFILE_ERROR,               AT_ERR_EMAT_CLEANPROFILE_ERROR},
    {TAF_ERR_EMAT_CLEANPROFILE_DATA_ERROR,          AT_ERR_EMAT_CLEANPROFILE_DATA_ERROR},
    {TAF_ERR_EMAT_CHECKPROFILE_ERROR,               AT_ERR_EMAT_CHECKPROFILE_ERROR},
    {TAF_ERR_EMAT_CHECKPROFILE_DATA_ERROR,          AT_ERR_EMAT_CHECKPROFILE_DATA_ERROR},
    {TAF_ERR_EMAT_TPDU_CNF_ERROR,                   AT_ERR_EMAT_TPDU_CNF_ERROR},
    {TAF_ERR_EMAT_TPDU_DATASTORE_ERROR,             AT_ERR_EMAT_TPDU_DATASTORE_ERROR},
    {TAF_ERR_ESIMSWITCH_SET_ERROR,                  AT_ERR_ESIMSWITCH_SET_ERROR},
    {TAF_ERR_ESIMSWITCH_SET_NOT_ENABLE_ERROR,       AT_ERR_ESIMSWITCH_SET_NOT_ENABLE_ERROR},
    {TAF_ERR_ESIMSWITCH_QRY_ERROR,                  AT_ERR_ESIMSWITCH_QRY_ERROR},
};


AT_CHG_MTA_ERR_CODE_TBL_STRU                g_astAtChgMtaErrCodeTbl[] =
{
    {MTA_AT_RESULT_INCORRECT_PARAMETERS,            AT_CME_INCORRECT_PARAMETERS},
    {MTA_AT_RESULT_FUNC_DISABLE,                    AT_CME_FUNC_DISABLE},
};

AT_PIH_RSP_PROC_FUNC_STRU     g_aAtPihRspProcFuncTbl[] =
{
    {SI_PIH_EVENT_FDN_CNF,                  At_ProcPihFndBndCnf},
    {SI_PIH_EVENT_BDN_CNF,                  At_ProcPihFndBndCnf},
    {SI_PIH_EVENT_GENERIC_ACCESS_CNF,       At_ProcPihGenericAccessCnf},
    {SI_PIH_EVENT_ISDB_ACCESS_CNF,          At_ProcPihIsdbAccessCnf},
    {SI_PIH_EVENT_CCHO_SET_CNF,             At_ProcPihCchoSetCnf},
    {SI_PIH_EVENT_CCHP_SET_CNF,             At_ProcPihCchpSetCnf},
    {SI_PIH_EVENT_CCHC_SET_CNF,             At_ProcPihCchcSetCnf},
    {SI_PIH_EVENT_SCICFG_SET_CNF,           At_ProcPihSciCfgSetCnf},
    {SI_PIH_EVENT_HVSST_SET_CNF,            At_ProcPihHvsstSetCnf},
    {SI_PIH_EVENT_CGLA_SET_CNF,             At_ProcPihCglaSetCnf},
    {SI_PIH_EVENT_CARD_ATR_QRY_CNF,         At_ProcPihCardAtrQryCnf},
    {SI_PIH_EVENT_SCICFG_QUERY_CNF,         At_SciCfgQueryCnf},
    {SI_PIH_EVENT_HVSST_QUERY_CNF,          At_HvsstQueryCnf},
    {SI_PIH_EVENT_CARDTYPE_QUERY_CNF,       At_ProcPihCardTypeQryCnf},
    {SI_PIH_EVENT_CARDTYPEEX_QUERY_CNF,     At_ProcPihCardTypeExQryCnf},
    {SI_PIH_EVENT_CARDVOLTAGE_QUERY_CNF,    At_ProcPihCardVoltageQryCnf},
    {SI_PIH_EVENT_PRIVATECGLA_SET_CNF,      At_ProcPihPrivateCglaSetCnf},
    {SI_PIH_EVENT_CRSM_SET_CNF,             At_ProcPihCrsmSetCnf},
    {SI_PIH_EVENT_CRLA_SET_CNF,             At_ProcPihCrlaSetCnf},
    {SI_PIH_EVENT_SESSION_QRY_CNF,          At_ProcPihSessionQryCnf},
    {SI_PIH_EVENT_CIMI_QRY_CNF,             At_ProcPihCimiQryCnf},
    {SI_PIH_EVENT_CCIMI_QRY_CNF,            At_ProcPihCcimiQryCnf},
#if (FEATURE_IMS == FEATURE_ON)
    {SI_PIH_EVENT_UICCAUTH_CNF,             AT_UiccAuthCnf},
    {SI_PIH_EVENT_URSM_CNF,                 AT_UiccAccessFileCnf},
#endif
#if (FEATURE_PHONE_SC == FEATURE_ON)
    {SI_PIH_EVENT_SILENT_PIN_SET_CNF,       At_PrintSilentPinInfo},
    {SI_PIH_EVENT_SILENT_PININFO_SET_CNF,   At_PrintSilentPinInfo},
#endif

    {SI_PIH_EVENT_ESIMSWITCH_SET_CNF,       At_PrintSetEsimSwitchInfo},
    {SI_PIH_EVENT_ESIMSWITCH_QRY_CNF,       At_PrintQryEsimSwitchInfo},
    {SI_PIH_EVENT_BWT_SET_CNF,              At_ProcPihBwtSetCnf},
    {SI_PIH_EVENT_PRIVATECCHO_SET_CNF,      At_ProcPihPrivateCchoSetCnf},
    {SI_PIH_EVENT_PRIVATECCHP_SET_CNF,      At_ProcPihPrivateCchpSetCnf},
};

AT_EMAT_RSP_PROC_FUNC_STRU     g_astAtEMATRspProcFuncTbl[] =
{
    {SI_EMAT_EVENT_ESIM_CLEAN_CNF   ,       At_PrintEsimCleanProfileInfo},
    {SI_EMAT_EVENT_ESIM_CHECK_CNF   ,       At_PrintEsimCheckProfileInfo},
    {SI_EMAT_EVENT_GET_ESIMEID_CNF  ,       At_PrintGetEsimEidInfo},
    {SI_EMAT_EVENT_GET_ESIMPKID_CNF ,       At_PrintGetEsimPKIDInfo},
};

/*****************************************************************************
   5 ��������������
*****************************************************************************/

extern TAF_UINT8                               gucSTKCmdQualify ;

#if(FEATURE_ON == FEATURE_LTE)
extern   VOS_UINT32 g_ulGuOnly;
extern   VOS_UINT32 g_ulGuTmodeCnfNum;
extern   VOS_UINT32 g_ulLteIsSend2Dsp;
extern   VOS_UINT32 g_ulTmodeLteMode;
 #endif

/*****************************************************************************
   6 ����ʵ��
*****************************************************************************/

LOCAL VOS_VOID AT_ConvertSysCfgStrToAutoModeStr(
    VOS_UINT8                          *pucAcqOrderBegin,
    VOS_UINT8                          *pucAcqOrder,
    VOS_UINT8                           ucRatNum
);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
LOCAL VOS_VOID AT_RcvMmaNrCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
);
#endif

#if (FEATURE_LTE == FEATURE_ON)
LOCAL VOS_VOID AT_RcvMmaLteCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
);
#endif

LOCAL VOS_VOID AT_RcvMmaWcdmaCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
);

LOCAL VOS_VOID AT_RcvMmaGsmCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
);

LOCAL VOS_CHAR* AT_ConvertRatModeForQryParaPlmnList(
    TAF_PH_RA_MODE  enRaMode
);


AT_CLI_VALIDITY_ENUM_UINT8 AT_ConvertCLIValidity(
    TAF_CCM_CALL_INCOMING_IND_STRU     *pstIncomingInd
)
{
    AT_CLI_VALIDITY_ENUM_UINT8          enCliVality;
    /* 24008 10.5.4.30 */
    /* Cause of No CLI information element provides the mobile station
       the detailed reason why Calling party BCD nuber is not notified. */

    /* ���볤�Ȳ�Ϊ0������Ϊ��ʾ������Ч */
    if (pstIncomingInd->stIncomingIndPara.stCallNumber.ucNumLen != 0)
    {
        return AT_CLI_VALIDITY_VALID;
    }

    switch (pstIncomingInd->stIncomingIndPara.enNoCliCause)
    {
        case MN_CALL_NO_CLI_USR_REJ:
        case MN_CALL_NO_CLI_INTERACT:
        case MN_CALL_NO_CLI_PAYPHONE:
            enCliVality = (AT_CLI_VALIDITY_ENUM_UINT8)pstIncomingInd->stIncomingIndPara.enNoCliCause;
            break;

        /* ԭ��ֵ������,����Ϸ���δ�ṩ */
        case MN_CALL_NO_CLI_BUTT:
        case MN_CALL_NO_CLI_UNAVAL:
        default:
            enCliVality = AT_CLI_VALIDITY_UNAVAL;
            break;
    }

    return enCliVality;
}

VOS_UINT32 AT_CheckRptCmdStatus(
    VOS_UINT8                          *pucRptCfg,
    AT_CMD_RPT_CTRL_TYPE_ENUM_UINT8     enRptCtrlType,
    AT_RPT_CMD_INDEX_ENUM_UINT8         enRptCmdIndex
)
{
    AT_RPT_CMD_INDEX_ENUM_UINT8        *pulRptCmdTblPtr = VOS_NULL_PTR;
    VOS_UINT32                          ulRptCmdTblSize;
    VOS_UINT8                           ucTableIndex;
    VOS_UINT32                          ulOffset;
    VOS_UINT8                           ucBit;

    /* �����ϱ�������������Ĭ�������ϱ� */
    if (enRptCmdIndex >= AT_RPT_CMD_BUTT)
    {
        return VOS_TRUE;
    }

    /* �����ϱ��ܿ�������д����Ĭ�������ϱ� */
    if (enRptCtrlType == AT_CMD_RPT_CTRL_BUTT)
    {
        return VOS_TRUE;
    }

    if (enRptCtrlType == AT_CMD_RPT_CTRL_BY_CURC)
    {
        pulRptCmdTblPtr = AT_GET_CURC_RPT_CTRL_STATUS_MAP_TBL_PTR();
        ulRptCmdTblSize = AT_GET_CURC_RPT_CTRL_STATUS_MAP_TBL_SIZE();
    }
    else
    {
        pulRptCmdTblPtr = AT_GET_UNSOLICITED_RPT_CTRL_STATUS_MAP_TBL_PTR();
        ulRptCmdTblSize = AT_GET_UNSOLICITED_RPT_CTRL_STATUS_MAP_TBL_SIZE();
    }

    for (ucTableIndex = 0; ucTableIndex < ulRptCmdTblSize; ucTableIndex++)
    {
        if (enRptCmdIndex == pulRptCmdTblPtr[ucTableIndex])
        {
            break;
        }
    }

    /* ��ȫ�ֱ����е�Bitλ�Ա� */
    if (ulRptCmdTblSize != ucTableIndex)
    {
        /* �����û����õ��ֽ�����Bitӳ������෴, ���ȷ�תBitλ */
        ulOffset        = AT_CURC_RPT_CFG_MAX_SIZE - (ucTableIndex / 8) - 1;
        ucBit           = (VOS_UINT8)(ucTableIndex % 8);

        return (VOS_UINT32)((pucRptCfg[ulOffset] >> ucBit) & 0x1);
    }

    return VOS_TRUE;
}


VOS_UINT32 At_ChgMnErrCodeToAt(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulMnErrorCode
)
{
    VOS_UINT32                          ulRtn;
    AT_CMS_SMS_ERR_CODE_MAP_STRU       *pstSmsErrMapTblPtr = VOS_NULL_PTR;
    VOS_UINT32                          ulSmsErrMapTblSize;
    VOS_UINT32                          ulCnt;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    pstSmsErrMapTblPtr = AT_GET_CMS_SMS_ERR_CODE_MAP_TBL_PTR();
    ulSmsErrMapTblSize = AT_GET_CMS_SMS_ERR_CODE_MAP_TBL_SIZE();

    ulRtn = AT_CMS_UNKNOWN_ERROR;

    for (ulCnt = 0; ulCnt < ulSmsErrMapTblSize; ulCnt++)
    {
        if (pstSmsErrMapTblPtr[ulCnt].ulSmsCause == ulMnErrorCode)
        {
            ulRtn =  pstSmsErrMapTblPtr[ulCnt].ulCmsCode;

            if ((pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT)
             && (ulRtn == AT_CMS_INVALID_PDU_MODE_PARAMETER))
            {
                ulRtn = AT_CMS_INVALID_TEXT_MODE_PARAMETER;
            }

            break;
        }
    }

    return ulRtn;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

TAF_UINT32 At_ChgXsmsErrorCodeToAt(
    TAF_UINT32                          ulXsmsError
)
{
    switch(ulXsmsError)
    {
        case TAF_XSMS_APP_SUCCESS:
            return AT_SUCCESS;

        case TAF_XSMS_APP_FAILURE:
            return AT_ERROR;

        case TAF_XSMS_APP_INIT_NOT_FINISH:
            return AT_CME_SIM_BUSY;

        case TAF_XSMS_APP_INVALID_INDEX:
            return AT_CME_INVALID_INDEX;

        case TAF_XSMS_APP_UIM_FAILURE:
            return AT_CME_SIM_FAILURE;

        case TAF_XSMS_APP_STORE_FULL:
            return AT_CMS_SIM_SMS_STORAGE_FULL;

        case TAF_XSMS_APP_NOT_SUPPORT_1X:
            return AT_CME_1X_RAT_NOT_SUPPORTED;

        case TAF_XSMS_APP_NORMAL_VOLTE:
        case TAF_XSMS_APP_UIM_LOCK_LOCKED:
        case TAF_XSMS_APP_ENCODE_ERROR:
        case TAF_XSMS_APP_UIM_MSG_STATUS_WRONG:
        case TAF_XSMS_APP_INSERT_SDND_POOL_FAIL:
        case TAF_XSMS_APP_NOT_INSERT_TO_POOL:
        case TAF_XSMS_APP_NOT_SUPPORT_LTE:
            return AT_ERROR;

        default:
            return AT_ERROR;
    }
}
#endif

TAF_UINT32 At_ChgTafErrorCode(TAF_UINT8 ucIndex, TAF_ERROR_CODE_ENUM_UINT32 enTafErrorCode)
{
    TAF_UINT32                          ulRtn  = 0;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          i;
    VOS_UINT32                          ulFlag = 0;


    /* ��ȡ��Ϣ���� */
    ulMsgCnt = sizeof(g_astAtChgTafErrCodeTbl)/sizeof(AT_CHG_TAF_ERR_CODE_TBL_STRU);

    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtChgTafErrCodeTbl[i].enTafErrCode == enTafErrorCode)
        {
            ulRtn  = g_astAtChgTafErrCodeTbl[i].enAtReturnCode;
            ulFlag = ulFlag + 1;
            break;
        }
    }

    if(ulFlag == 0)
    {
        if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
        {
            ulRtn = AT_CME_UNKNOWN;
        }
        else if (g_stParseContext[ucIndex].pstCmdElement == VOS_NULL_PTR)
        {
            ulRtn = AT_CME_UNKNOWN;
        }
        else if ((g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex > AT_CMD_SMS_BEGAIN)
              && (g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex < AT_CMD_SMS_END))
        {
            ulRtn = AT_CMS_UNKNOWN_ERROR;
        }
        else
        {
            ulRtn = AT_CME_UNKNOWN;
        }
    }

    return ulRtn;
}


TAF_UINT32 At_SsClass2Print(TAF_UINT8 ucClass)
{
    TAF_UINT32 ulRtn = 0;

    switch(ucClass)
    {
    case TAF_ALL_SPEECH_TRANSMISSION_SERVICES_TSCODE:
        ulRtn = 1;
        break;

    case TAF_ALL_DATA_TELESERVICES_TSCODE:
        ulRtn = 2;
        break;

    case TAF_ALL_FACSIMILE_TRANSMISSION_SERVICES_TSCODE:
        ulRtn = 4;
        break;

    case TAF_ALL_SMS_SERVICES_TSCODE:
        ulRtn = 8;
        break;

    case TAF_ALL_DATA_CIRCUIT_SYNCHRONOUS_BSCODE:
    case TAF_ALL_DATACDS_SERVICES_BSCODE:
        ulRtn = 16;
        break;

    case TAF_ALL_DATA_CIRCUIT_ASYNCHRONOUS_BSCODE:
        ulRtn = 32;
        break;

    default:
        break;
    }

    return ulRtn;
}
/*****************************************************************************
 Prototype      : At_CcClass2Print
 Description    : ��CCA���ص�CLASS���ַ�����ʽ�����ע�⣬������
 Input          : ucClass --- CCA��CLASS
 Output         : ---
 Return Value   : ulRtn������
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-04-19
    Author      : ---
    Modification: Created function
*****************************************************************************/
TAF_UINT32 At_CcClass2Print(MN_CALL_TYPE_ENUM_U8 enCallType,TAF_UINT8 *pDst)
{
    TAF_UINT16 usLength = 0;

    switch(enCallType)
    {
    case MN_CALL_TYPE_VOICE:
    case MN_CALL_TYPE_PSAP_ECALL:
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst,"VOICE");
        break;

    case MN_CALL_TYPE_FAX:
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst,"FAX");
        break;

    case MN_CALL_TYPE_VIDEO:
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst,"SYNC");
        break;

    case MN_CALL_TYPE_CS_DATA:
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst,"DATA");
        break;

    default:
        AT_WARN_LOG("At_CcClass2Print CallType ERROR");
        break;
    }

    return usLength;
}

/* PC������AT��A���Ƶ�C��, At_sprintf���ظ�����,���ڴ˴������������� */


TAF_UINT32 At_HexAlpha2AsciiString(TAF_UINT32 MaxLength,TAF_INT8 *headaddr,TAF_UINT8 *pucDst,TAF_UINT8 *pucSrc,TAF_UINT16 usSrcLen)
{
    TAF_UINT16 usLen = 0;
    TAF_UINT16 usChkLen = 0;
    TAF_UINT8 *pWrite = pucDst;
    TAF_UINT8 *pRead = pucSrc;
    TAF_UINT8  ucHigh = 0;
    TAF_UINT8  ucLow = 0;

    if(((TAF_UINT32)(pucDst - (TAF_UINT8 *)headaddr) + (2 * usSrcLen)) >= MaxLength)
    {
        AT_ERR_LOG("At_HexAlpha2AsciiString too long");
        return 0;
    }

    if(usSrcLen != 0)
    {
        /* ɨ�������ִ� */
        while( usChkLen++ < usSrcLen )
        {
            ucHigh = 0x0F & (*pRead >> 4);
            ucLow = 0x0F & *pRead;

            usLen += 2;    /* ��¼���� */

            if(ucHigh <= 0x09)   /* 0-9 */
            {
                *pWrite++ = ucHigh + 0x30;
            }
            else if(ucHigh >= 0x0A)    /* A-F */
            {
                *pWrite++ = ucHigh + 0x37;
            }
            else
            {

            }

            if(ucLow <= 0x09)   /* 0-9 */
            {
                *pWrite++ = ucLow + 0x30;
            }
            else if(ucLow >= 0x0A)    /* A-F */
            {
                *pWrite++ = ucLow + 0x37;
            }
            else
            {

            }

            /* ��һ���ַ� */
            pRead++;
        }

    }
    return usLen;
}


VOS_UINT32 AT_Hex2AsciiStrLowHalfFirst(
    VOS_UINT32                          ulMaxLength,
    VOS_INT8                            *pcHeadaddr,
    VOS_UINT8                           *pucDst,
    VOS_UINT8                           *pucSrc,
    VOS_UINT16                          usSrcLen
)
{
    VOS_UINT16                          usLen;
    VOS_UINT16                          usChkLen;
    VOS_UINT8                           *pcWrite = VOS_NULL_PTR;
    VOS_UINT8                           *pcRead = VOS_NULL_PTR;
    VOS_UINT8                           ucHigh;
    VOS_UINT8                           ucLow;

    usLen           = 0;
    usChkLen        = 0;
    pcWrite         = pucDst;
    pcRead          = pucSrc;


    if (((VOS_UINT32)(pucDst - (VOS_UINT8 *)pcHeadaddr) + (2 * usSrcLen)) >= ulMaxLength)
    {
        AT_ERR_LOG("AT_Hex2AsciiStrLowHalfFirst too long");
        return 0;
    }

    if (usSrcLen != 0)
    {
        /* ɨ�������ִ� */
        while ( usChkLen++ < usSrcLen )
        {
            ucHigh = 0x0F & (*pcRead >> 4);
            ucLow  = 0x0F & *pcRead;

            usLen += 2;    /* ��¼���� */

            /* ��ת���Ͱ��ֽ� */
            if (ucLow <= 0x09)   /* 0-9 */
            {
                *pcWrite++ = ucLow + 0x30;
            }
            else if (ucLow >= 0x0A)    /* A-F */
            {
                *pcWrite++ = ucLow + 0x37;
            }
            else
            {

            }

            /* ��ת���߰��ֽ� */
            if (ucHigh <= 0x09)   /* 0-9 */
            {
                *pcWrite++ = ucHigh + 0x30;
            }
            else if (ucHigh >= 0x0A)    /* A-F */
            {
                *pcWrite++ = ucHigh + 0x37;
            }
            else
            {

            }

            /* ��һ���ַ� */
            pcRead++;
        }

    }

    return usLen;
}


/*****************************************************************************
 Prototype      : At_ReadNumTypePara
 Description    : ��ȡASCII���͵ĺ���
 Input          : pucDst   --- Ŀ���ִ�
                  pucSrc   --- Դ�ִ�
                  usSrcLen --- Դ�ִ�����
 Output         :
 Return Value   : AT_XXX  --- ATC������
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-04-19
    Author      : ---
    Modification: Created function
*****************************************************************************/
TAF_UINT32 At_ReadNumTypePara(TAF_UINT8 *pucDst,TAF_UINT8 *pucSrc)
{
    TAF_UINT16 usLength = 0;

    if(gucAtCscsType == AT_CSCS_UCS2_CODE)       /* +CSCS:UCS2 */
    {
        TAF_UINT16 usSrcLen = (TAF_UINT16)VOS_StrLen((TAF_CHAR *)pucSrc);

        usLength += (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,pucDst + usLength,pucSrc,usSrcLen);
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pucDst + usLength,"%s",pucSrc);
    }
    return usLength;
}
#if( FEATURE_ON == FEATURE_CSD )

VOS_UINT32 AT_SendCsdCallStateInd(
    VOS_UINT8                           ucIndex,
    AT_CSD_CALL_TYPE_STATE_ENUM_UINT16  enCallState
)
{
    AT_CSD_CALL_STATE_TYPE_IND_STRU     *pstMsg = VOS_NULL_PTR;
    errno_t                             lMemResult;

    /* ����AT_CSD_CALL_STATE_TYPE_IND_STRU��Ϣ */
    pstMsg = (AT_CSD_CALL_STATE_TYPE_IND_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(
                            WUEPS_PID_AT,
                            sizeof(AT_CSD_CALL_STATE_TYPE_IND_STRU));

    if (pstMsg == VOS_NULL_PTR)
    {
        AT_WARN_LOG("AT_SendCsdCallStateInd: alloc msg fail!");
        return VOS_ERR;
    }

    /* ��ʼ����Ϣ */
    TAF_MEM_SET_S((VOS_CHAR*)pstMsg + VOS_MSG_HEAD_LENGTH,
               sizeof(AT_CSD_CALL_STATE_TYPE_IND_STRU) - VOS_MSG_HEAD_LENGTH,
               0x00,
               sizeof(AT_CSD_CALL_STATE_TYPE_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    /* ��д��Ϣͷ */
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = ACPU_PID_CSD;
    pstMsg->enMsgId         = ID_AT_CSD_CALL_STATE_IND;

    /* ��д��Ϣ�� */
    pstMsg->enCallState     = enCallState;
    pstMsg->ucIndex         = ucIndex;

    if (PS_SEND_MSG(WUEPS_PID_AT, pstMsg) != VOS_OK)
    {
        AT_WARN_LOG("AT_SendCsdCallStateInd: Send msg fail!");
        return VOS_ERR;
    }

    return VOS_OK;
}
#endif



VOS_BOOL  AT_IsFindVedioModemStatus(
    VOS_UINT8                           ucIndex,
    MN_CALL_TYPE_ENUM_U8                enCallType
)
{
    AT_DCE_MSC_STRU                     stMscStru;
    TAF_UINT32                          ulDelayaCnt;

    if (ucIndex != AT_CLIENT_TAB_MODEM_INDEX)
    {
        return VOS_FALSE;
    }

    /* ��AT_CSD_DATA_MODEģʽ�£���������ģʽ��DCD�ź����ͣ���ʱ�����ٴδ���PC������AT���� */
    if (( gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
     && (gastAtClientTab[ucIndex].Mode == AT_DATA_MODE)
     && (gastAtClientTab[ucIndex].DataMode == AT_CSD_DATA_MODE)
     && (enCallType == MN_CALL_TYPE_VIDEO))
    {

        /* ��������ģʽ */
        At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

#if( FEATURE_ON == FEATURE_CSD )
        /* ȥע��Modem�˿�VIDEO PHONE���ص� */
        AT_DeRegModemVideoPhoneFCPoint(ucIndex);

        AT_SendCsdCallStateInd(ucIndex, AT_CSD_CALL_STATE_OFF);
#endif

        memset_s(&stMscStru, sizeof(stMscStru), 0x00, sizeof(stMscStru));

        /* ����DCD�ź� */
        stMscStru.OP_Dcd = 1;
        stMscStru.ucDcd = 0;
        stMscStru.OP_Dsr = 1;
        stMscStru.ucDsr = 1;
        AT_SetModemStatus(ucIndex, &stMscStru);

        /*EVENT-UE Down DCD*/
        AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DCE_DOWN_DCD,
                         VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

        /* Ϊ�˱�֤���ߵĹܽ��ź���PC���ȴ���������ʱ�Ĵ���  */
        ulDelayaCnt = 1500000;
        while( ulDelayaCnt-- )
        {
            ;
        }

        /* ����DCD�ź� */
        stMscStru.OP_Dcd = 1;
        stMscStru.ucDcd = 1;
        stMscStru.OP_Dsr = 1;
        stMscStru.ucDsr = 1;
        AT_SetModemStatus(ucIndex, &stMscStru);

        return VOS_TRUE;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);
#endif

    return VOS_FALSE;
}


VOS_VOID  AT_CsRspEvtReleasedProc(
    TAF_UINT8                           ucIndex,
    TAF_CCM_CALL_RELEASED_IND_STRU     *pstCallReleaseInd
)
{
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;
    VOS_BOOL                            bRet;
    VOS_UINT32                          ulTimerName;
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;


    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(ucIndex);

    g_ucDtrDownFlag = VOS_FALSE;


#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* ֹͣRING TE */
    AT_VoiceStopRingTe(pstCallReleaseInd->stReleaseIndPara.callId);
#endif


    /* ��¼causeֵ���ı���Ϣ */
    AT_UpdateCallErrInfo(ucIndex, pstCallReleaseInd->stReleaseIndPara.enCause, &(pstCallReleaseInd->stReleaseIndPara.stErrInfoText));

    if ((gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CHUP_SET)
      ||(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_H_SET)
      ||(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CHLD_SET)
      ||(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CTFR_SET))
    {
        if (pstCcCtx->stS0TimeInfo.bTimerStart == VOS_TRUE)
        {
            ulTimerName = pstCcCtx->stS0TimeInfo.ulTimerName;

            AT_StopRelTimer(ulTimerName, &(pstCcCtx->stS0TimeInfo.s0Timer));
            pstCcCtx->stS0TimeInfo.bTimerStart = VOS_FALSE;
            pstCcCtx->stS0TimeInfo.ulTimerName = 0;
        }

        AT_IsFindVedioModemStatus(ucIndex,pstCallReleaseInd->stReleaseIndPara.enCallType);

        AT_ReportCendResult(ucIndex, pstCallReleaseInd);

        return;
    }
    else
    {
        /*
        ��Ҫ�����������ͣ����棬���ݣ����ӵ绰����������
        */

        /* gastAtClientTab[ucIndex].ulCauseû��ʹ�õ㣬��ֵ��ɾ�� */

        if (pstCcCtx->stS0TimeInfo.bTimerStart == VOS_TRUE)
        {
            ulTimerName = pstCcCtx->stS0TimeInfo.ulTimerName;

            AT_StopRelTimer(ulTimerName, &(pstCcCtx->stS0TimeInfo.s0Timer));
            pstCcCtx->stS0TimeInfo.bTimerStart = VOS_FALSE;
            pstCcCtx->stS0TimeInfo.ulTimerName = 0;
        }


        /* �ϱ�CEND�����ӵ绰����Ҫ�ϱ�^CEND */
        if ((At_CheckReportCendCallType(pstCallReleaseInd->stReleaseIndPara.enCallType) == PS_TRUE)
         || (AT_EVT_IS_PS_VIDEO_CALL(pstCallReleaseInd->stReleaseIndPara.enCallType, pstCallReleaseInd->stReleaseIndPara.enVoiceDomain)))
        {
            AT_ReportCendResult(ucIndex, pstCallReleaseInd);

            return;
        }

        ulResult = AT_NO_CARRIER;

        if (AT_EVT_IS_VIDEO_CALL(pstCallReleaseInd->stReleaseIndPara.enCallType))
        {
            if (pstCallReleaseInd->stReleaseIndPara.enCause == TAF_CS_CAUSE_CC_NW_USER_ALERTING_NO_ANSWER)
            {
                ulResult = AT_NO_ANSWER;
            }

            if (pstCallReleaseInd->stReleaseIndPara.enCause == TAF_CS_CAUSE_CC_NW_USER_BUSY)
            {
                ulResult = AT_BUSY;
            }
        }

        /* AT������Ļ�����Ҫ�����Ӧ��״̬���� */
        if (AT_EVT_REL_IS_NEED_CLR_TIMER_STATUS_CMD(gastAtClientTab[ucIndex].CmdCurrentOpt))
        {
            AT_STOP_TIMER_CMD_READY(ucIndex);
        }


        bRet = AT_IsFindVedioModemStatus(ucIndex,pstCallReleaseInd->stReleaseIndPara.enCallType);
        if ( bRet == VOS_TRUE )
        {
            return ;
        }
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);
}



VOS_UINT32  AT_RcvTafCcmCallConnectInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_CONNECT_IND_STRU      *pstConnectIndMsg = VOS_NULL_PTR;
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;
    TAF_UINT8                           aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN*2)+1];
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    MN_CALL_TYPE_ENUM_U8                enNewCallType;
    VOS_UINT8                           ucIndex = 0;

    enModemId = MODEM_ID_0;

    pstConnectIndMsg = (TAF_CCM_CALL_CONNECT_IND_STRU *)pMsg;

    if(At_ClientIdToUserId(pstConnectIndMsg->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_CsRspEvtConnectProc At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* ֹͣRING TE */
    AT_VoiceStopRingTe(pstConnectIndMsg->stConnectIndPara.callId);
#endif

    /* CS���гɹ�, ���CS���������ı���Ϣ */
    AT_UpdateCallErrInfo(ucIndex, TAF_CS_CAUSE_SUCCESS, &(pstConnectIndMsg->stConnectIndPara.stErrInfoText));

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_CsRspEvtConnectProc: Get modem id fail.");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        enNewCallType = MN_CALL_TYPE_VOICE;
        At_ChangeEcallTypeToCallType(pstConnectIndMsg->stConnectIndPara.enCallType, &enNewCallType);

        if (AT_CheckRptCmdStatus(pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONN) == VOS_TRUE)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s^CONN:%d,%d%s",
                                                gaucAtCrLf,
                                                pstConnectIndMsg->stConnectIndPara.callId,
                                                enNewCallType,
                                                gaucAtCrLf);
            At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
        }

        AT_ReportCCallstateResult(enModemId, pstConnectIndMsg->stConnectIndPara.callId, pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CS_CALL_STATE_CONNECT, pstConnectIndMsg->stConnectIndPara.enVoiceDomain);

        return VOS_OK;
    }

    pstSsCtx = AT_GetModemSsCtxAddrFromModemId(enModemId);

    /* ��Ҫ�ж��������ͣ���VOICE����DATA */
    if(pstConnectIndMsg->stConnectIndPara.enCallDir == MN_CALL_DIR_MO)
    {
        if(pstSsCtx->ucColpType == AT_COLP_ENABLE_TYPE)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s+COLP: ",gaucAtCrLf);
            if(pstConnectIndMsg->stConnectIndPara.stConnectNumber.ucNumLen != 0)
            {
                AT_BcdNumberToAscii(pstConnectIndMsg->stConnectIndPara.stConnectNumber.aucBcdNum,
                                    pstConnectIndMsg->stConnectIndPara.stConnectNumber.ucNumLen,
                                    (VOS_CHAR *)aucAsciiNum);
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"%s\",%d,\"\",,\"\"",aucAsciiNum,(pstConnectIndMsg->stConnectIndPara.stConnectNumber.enNumType | AT_NUMBER_TYPE_EXT));
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"\",,\"\",,\"\"");
            }
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
            At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
            usLength = 0;
        }
    }

    /* Video�£�ͨ��At_FormatResultData���ϱ�CONNECT */
    if (AT_EVT_IS_VIDEO_CALL(pstConnectIndMsg->stConnectIndPara.enCallType))
    {
        /* IMS Video�����ϲ㱨CONNECT���ϱ�^CONN */
        if (pstConnectIndMsg->stConnectIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            if (AT_CheckRptCmdStatus(pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONN) == VOS_TRUE)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s^CONN:%d,%d%s",
                                                   gaucAtCrLf,
                                                   pstConnectIndMsg->stConnectIndPara.callId,
                                                   pstConnectIndMsg->stConnectIndPara.enCallType,
                                                   gaucAtCrLf);

                At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
            }

            AT_ReportCCallstateResult(enModemId, pstConnectIndMsg->stConnectIndPara.callId, pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CS_CALL_STATE_CONNECT, pstConnectIndMsg->stConnectIndPara.enVoiceDomain);

            return VOS_OK;
        }

        gastAtClientTab[ucIndex].ucCsRabId = pstConnectIndMsg->stConnectIndPara.ucRabId;
        ulResult = AT_CONNECT;

        /* �����PCUI�ڷ���Ĳ�����ӵ绰�Ĳ�������Ǩ������̬��ֻ��MODEM�ڷ����VP��������Ǩ������̬  */
        if (gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
        {
#if( FEATURE_ON == FEATURE_CSD )
            /* ע��Modem�˿�VIDEO PHONE�����ص� */
            AT_RegModemVideoPhoneFCPoint(ucIndex, FC_ID_MODEM);

            AT_SendCsdCallStateInd(ucIndex, AT_CSD_CALL_STATE_ON);
#endif

            At_SetMode(ucIndex, AT_DATA_MODE, AT_CSD_DATA_MODE);   /* ��ʼ���� */
        }
    }
    else
    {

        enNewCallType = MN_CALL_TYPE_VOICE;
        At_ChangeEcallTypeToCallType(pstConnectIndMsg->stConnectIndPara.enCallType, &enNewCallType);

        if (AT_CheckRptCmdStatus(pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONN) == VOS_TRUE)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"^CONN:%d",pstConnectIndMsg->stConnectIndPara.callId);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",enNewCallType);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
            At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
        }
        AT_ReportCCallstateResult(enModemId, pstConnectIndMsg->stConnectIndPara.callId, pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CS_CALL_STATE_CONNECT, pstConnectIndMsg->stConnectIndPara.enVoiceDomain);

        return VOS_OK;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    AT_ReportCCallstateResult(enModemId, pstConnectIndMsg->stConnectIndPara.callId, pstConnectIndMsg->stConnectIndPara.aucCurcRptCfg, AT_CS_CALL_STATE_CONNECT, pstConnectIndMsg->stConnectIndPara.enVoiceDomain);

    return VOS_OK;
}


VOS_UINT32  AT_RcvTafCcmCallOrigInd(VOS_VOID *pMsg)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx   = VOS_NULL_PTR;
    TAF_CCM_CALL_ORIG_IND_STRU         *pstOrigInd = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    TAF_UINT16                          usLength;
    VOS_UINT32                          ulCheckRptCmdStatusResult;
    MN_CALL_TYPE_ENUM_U8                enNewCallType;
    TAF_UINT8                           ucIndex;


    usLength  = 0;
    enModemId = MODEM_ID_0;

    ucIndex = 0;
    pstOrigInd = (TAF_CCM_CALL_ORIG_IND_STRU *)pMsg;

    if (At_ClientIdToUserId(pstOrigInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvMnCallSetCssnCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_CsRspEvtOrigProc: Get modem id fail.");
        return VOS_ERR;
    }

    pstCcCtx = AT_GetModemCcCtxAddrFromModemId(enModemId);

    /* ���ӵ绰���棬���ﲻ���ϱ�^ORIG �����ֻ����ͨ�����ͽ������е�����£����ϱ�^ORIG */
    ulCheckRptCmdStatusResult = AT_CheckRptCmdStatus(pstOrigInd->stOrigIndPara.aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_ORIG);
    enNewCallType = MN_CALL_TYPE_VOICE;
    At_ChangeEcallTypeToCallType(pstOrigInd->stOrigIndPara.enCallType, &enNewCallType);

    if (((At_CheckReportOrigCallType(enNewCallType) == PS_TRUE)
      || (AT_EVT_IS_PS_VIDEO_CALL(pstOrigInd->stOrigIndPara.enCallType, pstOrigInd->stOrigIndPara.enVoiceDomain)))
     && (ulCheckRptCmdStatusResult == VOS_TRUE)
     && (!AT_IS_BROADCAST_CLIENT_INDEX(ucIndex)))
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"^ORIG:%d",pstOrigInd->stOrigIndPara.callId);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",enNewCallType);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    }

    /* ������к��յ��ظ���OK�󣬽���ǰ�Ƿ���ں��б�־��ΪTRUE */
    pstCcCtx->ulCurIsExistCallFlag = VOS_TRUE;

    AT_ReportCCallstateResult(enModemId, pstOrigInd->stOrigIndPara.callId, pstOrigInd->stOrigIndPara.aucCurcRptCfg, AT_CS_CALL_STATE_ORIG, pstOrigInd->stOrigIndPara.enVoiceDomain);

    return VOS_OK;
}


VOS_UINT8 At_GetSsCode(
    MN_CALL_SS_NOTIFY_CODE_ENUM_U8      enCode,
    MN_CALL_STATE_ENUM_U8               enCallState
)
{
    switch (enCode)
    {
        case MN_CALL_SS_NTFY_FORWORDED_CALL:
            return 0;

        case MN_CALL_SS_NTFY_MT_CUG_INFO:
            return 1;

        case MN_CALL_SS_NTFY_ON_HOLD:
            return 2;

        case MN_CALL_SS_NTFY_RETRIEVED:
            return 3;

        case MN_CALL_SS_NTFY_ENTER_MPTY:
            return 4;

        case MN_CALL_SS_NTFY_HOLD_RELEASED:
            return 5;

        case MN_CALL_SS_NTFY_DEFLECTED_CALL:
            return 9;

        case MN_CALL_SS_NTFY_INCALL_FORWARDED:
            return 10;

        case MN_CALL_SS_NTFY_EXPLICIT_CALL_TRANSFER:
            if ( enCallState == MN_CALL_S_ALERTING )
            {
                return 7;
            }
            return 8;

        case MN_CALL_SS_NTFY_CCBS_BE_RECALLED:
            return 0x16;

        default:
            return 0xFF;
    }
}


VOS_UINT8 At_GetCssiForwardCauseCode(MN_CALL_CF_CAUSE_ENUM_UINT8 enCode)
{
    switch (enCode)
    {
        case MN_CALL_CF_CAUSE_ALWAYS:
            return 0;

        case MN_CALL_CF_CAUSE_BUSY:
            return 1;

        case MN_CALL_CF_CAUSE_POWER_OFF:
            return 2;

        case MN_CALL_CF_CAUSE_NO_ANSWER:
            return 3;

        case MN_CALL_CF_CAUSE_SHADOW_ZONE:
            return 4;

        case MN_CALL_CF_CAUSE_DEFLECTION_480:
            return 5;

        case MN_CALL_CF_CAUSE_DEFLECTION_487:
            return 6;

        default:
            AT_ERR_LOG1("At_GetCssiFormardCauseCode: enCode is fail, enCode is ", enCode);
            return 0xFF;
    }
}

#if (FEATURE_IMS == FEATURE_ON)

VOS_VOID At_ProcCsEvtCssuexNotifiy_Ims(
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT8                           ucCode,
    VOS_UINT16                         *pusLength
)
{
    VOS_CHAR                            aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];

    memset_s(aucAsciiNum, (VOS_UINT32)sizeof(aucAsciiNum), 0, (VOS_UINT32)sizeof(aucAsciiNum));

    /* ^CSSUEX: <code2>,[<index>],<callId>[,<number>,<type>[,<forward_cause>]] */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s^CSSUEX: ",
                                         gaucAtCrLf);

    /* <code2>, */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d,", ucCode);

    /* [index], */
    if (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MT_CUG_INFO)
    {
        /* <index> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%d,", pstCallSsInd->stSsIndPara.stSsNotify.ulCugIndex);
    }
    else
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            ",");
    }

    /* <callId> */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d", pstCallSsInd->stSsIndPara.callId);

    if (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MT_CUG_INFO)
    {
        if (pstCallSsInd->stSsIndPara.stCallNumber.ucNumLen != 0)
        {
            AT_BcdNumberToAscii(pstCallSsInd->stSsIndPara.stCallNumber.aucBcdNum,
                                pstCallSsInd->stSsIndPara.stCallNumber.ucNumLen,
                                aucAsciiNum);

            /* ,<number> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",\"%s\"", aucAsciiNum);

            /* ,<type> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d", (pstCallSsInd->stSsIndPara.stCallNumber.enNumType | AT_NUMBER_TYPE_EXT));
        }
    }



    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s", gaucAtCrLf);
}

VOS_VOID At_ProcCsEvtCssuNotifiy_Ims(
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT8                           ucCode,
    VOS_UINT16                         *pusLength
)
{
    VOS_CHAR                            aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];

    memset_s(aucAsciiNum, (VOS_UINT32)sizeof(aucAsciiNum), 0, (VOS_UINT32)sizeof(aucAsciiNum));

    /* ^CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s^CSSU: ",
                                         gaucAtCrLf);

    /* <code2> */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d", ucCode);

    if (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MT_CUG_INFO)
    {
        /* ,<index> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             ",%d", pstCallSsInd->stSsIndPara.stSsNotify.ulCugIndex);

        if (pstCallSsInd->stSsIndPara.stCallNumber.ucNumLen != 0)
        {
            AT_BcdNumberToAscii(pstCallSsInd->stSsIndPara.stCallNumber.aucBcdNum,
                                pstCallSsInd->stSsIndPara.stCallNumber.ucNumLen,
                                aucAsciiNum);

            /* ,<number> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",\"%s\"", aucAsciiNum);

            /* ,<type> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d", (pstCallSsInd->stSsIndPara.stCallNumber.enNumType | AT_NUMBER_TYPE_EXT));
        }
    }

    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s", gaucAtCrLf);

    At_ProcCsEvtCssuexNotifiy_Ims(pstCallSsInd,ucCode,pusLength);

}


VOS_VOID At_ProcCsEvtCssiNotifiy_Ims(
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT16                         *pusLength
)
{
    VOS_CHAR                            aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];
    VOS_UINT8                           ucForwardCauseCode;

    memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));
    ucForwardCauseCode  = 0xFF;

    /* ^CSSI: <code1>,<index>,<callId>[,<number>,<type>[,<forward_cause>]] */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s^CSSI: ",
                                         gaucAtCrLf);

    /* <code1>, */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d,", pstCallSsInd->stSsIndPara.stSsNotify.enCode);

    /* [index], */
    if ((pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MO_CUG_INFO)
     || (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_BE_FORWORDED))
    {
        /* ,<index> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            "%d,", pstCallSsInd->stSsIndPara.stSsNotify.ulCugIndex);
    }
    else
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                            ",");
    }

    /* <callId> */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d", pstCallSsInd->stSsIndPara.callId);

    if ((pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MO_CUG_INFO)
     || (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_BE_FORWORDED))
    {
        /* ����������ʾ */
        if (pstCallSsInd->stSsIndPara.stConnectNumber.ucNumLen != 0)
        {
            (VOS_VOID)AT_BcdNumberToAscii(pstCallSsInd->stSsIndPara.stConnectNumber.aucBcdNum,
                                          pstCallSsInd->stSsIndPara.stConnectNumber.ucNumLen,
                                          aucAsciiNum);

            /* ,<number> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",\"%s\"", aucAsciiNum);

            /* ,<type> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d", (pstCallSsInd->stSsIndPara.stConnectNumber.enNumType | AT_NUMBER_TYPE_EXT));

            /* [,<forward_cause>] */
            ucForwardCauseCode = At_GetCssiForwardCauseCode(pstCallSsInd->stSsIndPara.enCallForwardCause);

            if ( ucForwardCauseCode != 0xFF)
            {
                *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                     ",%d", ucForwardCauseCode);
            }
        }
    }

    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s", gaucAtCrLf);

}


VOS_VOID At_ProcCsEvtImsHoldToneNotifiy_Ims(
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT16                         *pusLength
)
{

    /* ^IMSHOLDTONE: <hold_tone> */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s%s: ",
                                         gaucAtCrLf,
                                         gastAtStringTab[AT_STRING_IMS_HOLD_TONE].pucText);

    /* <hold_tone> */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d", pstCallSsInd->stSsIndPara.enHoldToneType);


    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s", gaucAtCrLf);

}
#endif


VOS_VOID At_ProcCsEvtCssuNotifiy_NonIms(
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT8                           ucCode,
    VOS_UINT16                         *pusLength
)
{
    VOS_CHAR                            aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];

    memset_s(aucAsciiNum, (VOS_UINT32)sizeof(aucAsciiNum), 0, (VOS_UINT32)sizeof(aucAsciiNum));

    /* +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s+CSSU: ",
                                         gaucAtCrLf);

    /* <code2> */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d", ucCode);

    if (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MT_CUG_INFO)
    {
        /* ,<index> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             ",%d", pstCallSsInd->stSsIndPara.stSsNotify.ulCugIndex);

        if (pstCallSsInd->stSsIndPara.stCallNumber.ucNumLen != 0)
        {
            AT_BcdNumberToAscii(pstCallSsInd->stSsIndPara.stCallNumber.aucBcdNum,
                                pstCallSsInd->stSsIndPara.stCallNumber.ucNumLen,
                                aucAsciiNum);

            /* ,<number> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",\"%s\"", aucAsciiNum);

            /* ,<type> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d", (pstCallSsInd->stSsIndPara.stCallNumber.enNumType | AT_NUMBER_TYPE_EXT));
        }
    }

    if (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_EXPLICIT_CALL_TRANSFER)
    {
        /* ,<index> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             ",%d", pstCallSsInd->stSsIndPara.stSsNotify.ulCugIndex);

        if (pstCallSsInd->stSsIndPara.stSsNotify.stEctIndicator.rdn.stPresentationAllowedAddr.ucNumLen != 0)
        {
            AT_BcdNumberToAscii(pstCallSsInd->stSsIndPara.stSsNotify.stEctIndicator.rdn.stPresentationAllowedAddr.aucBcdNum,
                                pstCallSsInd->stSsIndPara.stSsNotify.stEctIndicator.rdn.stPresentationAllowedAddr.ucNumLen,
                                aucAsciiNum);

            /* ,<number> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",\"%s\"", aucAsciiNum);

            /* ,<type> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d", (pstCallSsInd->stSsIndPara.stSsNotify.stEctIndicator.rdn.stPresentationAllowedAddr.enNumType | AT_NUMBER_TYPE_EXT));
        }
    }

    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s", gaucAtCrLf);

}


VOS_VOID At_ProcCsEvtCssiNotifiy_NonIms(
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT16                         *pusLength
)
{
    /* +CSSI: <code1>[,<index>] */
    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s+CSSI: ",
                                         gaucAtCrLf);

    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%d", pstCallSsInd->stSsIndPara.stSsNotify.enCode);

    if (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_MO_CUG_INFO)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             ",%d", pstCallSsInd->stSsIndPara.stSsNotify.ulCugIndex);
    }

    if ((pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_CCBS_RECALL)
     && (pstCallSsInd->stSsIndPara.stCcbsFeature.OP_CcbsIndex == MN_CALL_OPTION_EXIST))
    {
        *pusLength += (VOS_UINT16)At_CcClass2Print(pstCallSsInd->stSsIndPara.enCallType,
                                                   pgucAtSndCodeAddr + *pusLength);

        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                              "%s",
                                              gaucAtCrLf);

        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                              ",%d",
                                              pstCallSsInd->stSsIndPara.stCcbsFeature.CcbsIndex);

        if (pstCallSsInd->stSsIndPara.stCcbsFeature.OP_BSubscriberNum == MN_CALL_OPTION_EXIST)
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                  ",%s",
                                                  pstCallSsInd->stSsIndPara.stCcbsFeature.aucBSubscriberNum);
        }

        if (pstCallSsInd->stSsIndPara.stCcbsFeature.OP_NumType == MN_CALL_OPTION_EXIST)
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                  ",%d",
                                                  pstCallSsInd->stSsIndPara.stCcbsFeature.NumType);
        }
    }

    *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s", gaucAtCrLf);

}



VOS_VOID At_ProcCsRspEvtCssuNotifiy(
    VOS_UINT8                           ucIndex,
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                           ucCode;
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulCssuRptStatus;

    usLength = *pusLength;

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        return;
    }

    ulCssuRptStatus = AT_CheckRptCmdStatus(pstCallSsInd->stSsIndPara.aucUnsolicitedRptCfg,
                                           AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                           AT_RPT_CMD_CSSU);

    /* +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
    if (((ulCssuRptStatus == VOS_TRUE)
      && (pstCallSsInd->stSsIndPara.stSsNotify.enCode > MN_CALL_SS_NTFY_BE_DEFLECTED)
      && (pstCallSsInd->stSsIndPara.stSsNotify.enCode != MN_CALL_SS_NTFY_CCBS_RECALL))
     && ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_D_CS_VOICE_CALL_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_D_CS_DATA_CALL_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_APDS_SET)))
    {
        ucCode = At_GetSsCode(pstCallSsInd->stSsIndPara.stSsNotify.enCode, pstCallSsInd->stSsIndPara.enCallState);

        if (ucCode == 0xFF)
        {
            AT_ERR_LOG("At_ProcCsRspEvtCssuNotifiy: code error.");
            return;
        }

#if (FEATURE_IMS == FEATURE_ON)
        if (pstCallSsInd->stSsIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            At_ProcCsEvtCssuNotifiy_Ims(pstCallSsInd, ucCode, &usLength);
        }
        else
#endif
        {
            At_ProcCsEvtCssuNotifiy_NonIms(pstCallSsInd, ucCode, &usLength);
        }
    }

    *pusLength = usLength;

    return;
}


VOS_VOID At_ProcCsRspEvtCssiNotifiy(
    VOS_UINT8                           ucIndex,
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulCssiRptStatus;

    usLength = *pusLength;

    ulCssiRptStatus = AT_CheckRptCmdStatus(pstCallSsInd->stSsIndPara.aucUnsolicitedRptCfg,
                                           AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                           AT_RPT_CMD_CSSI);

    if ((ulCssiRptStatus == VOS_TRUE)
     && ((pstCallSsInd->stSsIndPara.stSsNotify.enCode <= MN_CALL_SS_NTFY_BE_DEFLECTED)
      || (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_CCBS_RECALL)))
    {
#if (FEATURE_IMS == FEATURE_ON)
        /* ^CSSI: <code1>[,<index>[,<number>,<type>]] */
        if (pstCallSsInd->stSsIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            At_ProcCsEvtCssiNotifiy_Ims(pstCallSsInd, &usLength);
        }
        else
#endif
        /* +CSSI: <code1>[,<index>] */
        {
            At_ProcCsEvtCssiNotifiy_NonIms(pstCallSsInd, &usLength);
        }
    }

    *pusLength = usLength;

    return;
}


VOS_VOID  AT_ProcCsRspEvtSsNotify(
    VOS_UINT8                           ucIndex,
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd
)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;

    At_ProcCsRspEvtCssiNotifiy(ucIndex, pstCallSsInd, &usLength);

    At_ProcCsRspEvtCssuNotifiy(ucIndex, pstCallSsInd, &usLength);

#if (FEATURE_IMS == FEATURE_ON)
    if (pstCallSsInd->stSsIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
    {
        At_ProcCsEvtImsHoldToneNotifiy_Ims(pstCallSsInd, &usLength);
    }
#endif

    if ((ucIndex != AT_BROADCAST_CLIENT_INDEX_MODEM_0)
     && (ucIndex != AT_BROADCAST_CLIENT_INDEX_MODEM_1)
     && (ucIndex != AT_BROADCAST_CLIENT_INDEX_MODEM_2))
    {
        enModemId = MODEM_ID_0;

        /* ��ȡclient id��Ӧ��Modem Id */
        ulRslt = AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId);

        if (ulRslt != VOS_OK)
        {
            AT_WARN_LOG("AT_ProcCsRspEvtSsNotify: WARNING:MODEM ID NOT FOUND!");
            return;
        }

        /* CCALLSTATE��Ҫ�㲥�ϱ�������MODEM ID���ù㲥�ϱ���Index */
        if (enModemId == MODEM_ID_0)
        {
            ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
        }
        else if (enModemId == MODEM_ID_1)
        {
            ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_1;
        }
        else
        {
            ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_2;
        }
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_UINT32  AT_RcvTafCcmCallProcInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_PROC_IND_STRU         *pstCallProcInd = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulCheckRptCmdStatusResult;
    TAF_UINT16                          usLength;
    TAF_UINT8                           ucIndex;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    pstCallProcInd = (TAF_CCM_CALL_PROC_IND_STRU *)pMsg;

    if(At_ClientIdToUserId(pstCallProcInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ProcCsRspEvtCallProc At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_CsRspEvtCallProcProc: Get modem id fail.");
        return VOS_ERR;
    }

    /* CS���ӵ绰���棬���ﲻ���ϱ�^CONF �����ֻ����ͨ�����ͽ������е�����£����ϱ�^CONF */
    ulCheckRptCmdStatusResult = AT_CheckRptCmdStatus(pstCallProcInd->stProcIndPata.aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CONF);

    if (((At_CheckReportConfCallType(pstCallProcInd->stProcIndPata.enCallType) == PS_TRUE)
      || (AT_EVT_IS_PS_VIDEO_CALL(pstCallProcInd->stProcIndPata.enCallType, pstCallProcInd->stProcIndPata.enVoiceDomain)))
     && (ulCheckRptCmdStatusResult == VOS_TRUE)
     && (!AT_IS_BROADCAST_CLIENT_INDEX(ucIndex)))
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"^CONF:%d",pstCallProcInd->stProcIndPata.callId);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    }


    AT_ReportCCallstateResult(enModemId, pstCallProcInd->stProcIndPata.callId, pstCallProcInd->stProcIndPata.aucCurcRptCfg, AT_CS_CALL_STATE_CALL_PROC, pstCallProcInd->stProcIndPata.enVoiceDomain);

    return VOS_OK;
}


TAF_VOID AT_CsSsNotifyEvtIndProc(
    TAF_UINT8                           ucIndex,
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd
)
{
    VOS_UINT8                           ucCode;
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulCssiRptStatus;
    VOS_UINT32                          ulCssuRptStatus;

    usLength = 0;
    ulCssiRptStatus = AT_CheckRptCmdStatus(pstCallSsInd->stSsIndPara.aucUnsolicitedRptCfg,
                                           AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                           AT_RPT_CMD_CSSI);

    if ((ulCssiRptStatus == VOS_TRUE)
     && ((pstCallSsInd->stSsIndPara.stSsNotify.enCode <= MN_CALL_SS_NTFY_BE_DEFLECTED)
      || (pstCallSsInd->stSsIndPara.stSsNotify.enCode == MN_CALL_SS_NTFY_CCBS_RECALL)))
    {
#if (FEATURE_IMS == FEATURE_ON)
        /* ^CSSI: <code1>[,<index>[,<number>,<type>]] */
        if (pstCallSsInd->stSsIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            At_ProcCsEvtCssiNotifiy_Ims(pstCallSsInd, &usLength);
        }
        else
#endif
        /* +CSSI: <code1>[,<index>] */
        {
            At_ProcCsEvtCssiNotifiy_NonIms(pstCallSsInd, &usLength);
        }
    }

    ulCssuRptStatus = AT_CheckRptCmdStatus(pstCallSsInd->stSsIndPara.aucUnsolicitedRptCfg,
                                           AT_CMD_RPT_CTRL_BY_UNSOLICITED,
                                           AT_RPT_CMD_CSSU);

    if ((ulCssuRptStatus == VOS_TRUE)
     && (pstCallSsInd->stSsIndPara.stSsNotify.enCode > MN_CALL_SS_NTFY_BE_DEFLECTED)
     && (pstCallSsInd->stSsIndPara.stSsNotify.enCode != MN_CALL_SS_NTFY_CCBS_RECALL))
    {
        ucCode = At_GetSsCode(pstCallSsInd->stSsIndPara.stSsNotify.enCode, pstCallSsInd->stSsIndPara.enCallState);

        if (ucCode == 0xFF)
        {
            AT_ERR_LOG("AT_CsSsNotifyEvtIndProc: cssu code error.");
            return;
        }

#if (FEATURE_IMS == FEATURE_ON)
        if (pstCallSsInd->stSsIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            /* ^CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
            At_ProcCsEvtCssuNotifiy_Ims(pstCallSsInd, ucCode, &usLength);
        }
        else
#endif
        {
            /* +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]] */
            At_ProcCsEvtCssuNotifiy_NonIms(pstCallSsInd, ucCode, &usLength);
        }
    }

#if (FEATURE_IMS == FEATURE_ON)
    if (pstCallSsInd->stSsIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
    {
        At_ProcCsEvtImsHoldToneNotifiy_Ims(pstCallSsInd, &usLength);
    }
#endif

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}

TAF_VOID At_CsIncomingEvtOfIncomeStateIndProc(
    TAF_UINT8                           ucIndex,
    TAF_CCM_CALL_INCOMING_IND_STRU     *pstIncomingInd
)
{
    TAF_UINT16                          usLength;
    TAF_UINT8                           ucCLIValid;
    TAF_UINT32                          ulTimerName;
    TAF_UINT8                           aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN*2)+1];
    AT_DCE_MSC_STRU                     stMscStru;
    TAF_UINT16                          usLoop;
    TAF_UINT32                          ulDelayaCnt;
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;

    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(ucIndex);
    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    usLength = 0;
    usLoop = 0;
    ulDelayaCnt = 50000;

    if( pstSsCtx->ucCrcType == AT_CRC_ENABLE_TYPE )         /* ������Э�鲻�� */
    {
        /* +CRC -- +CRING: <type> */

#if (FEATURE_IMS == FEATURE_ON)
        if (pstIncomingInd->stIncomingIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%sIRING%s",
                                                gaucAtCrLf,
                                                gaucAtCrLf);
        }
        else
#endif
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s+CRING: ",
                                                gaucAtCrLf);

            usLength += (TAF_UINT16)At_CcClass2Print(pstIncomingInd->stIncomingIndPara.enCallType,
                                                     pgucAtSndCodeAddr + usLength);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s",
                                                gaucAtCrLf);
        }
    }
    else
    {
        if (AT_EVT_IS_CS_VIDEO_CALL(pstIncomingInd->stIncomingIndPara.enCallType, pstIncomingInd->stIncomingIndPara.enVoiceDomain))
        {
            /* ���ڴ�ʱ���ǹ㲥�ϱ�����Ҫ�ҵ���Ӧ��MODEM�˿�,���йܽ��źŵ�
               ���� */
            for(usLoop = 0; usLoop < AT_MAX_CLIENT_NUM; usLoop++)
            {
                if (gastAtClientTab[usLoop].UserType == AT_MODEM_USER)
                {
                    ucIndex = (VOS_UINT8)usLoop;
                    break;
                }
            }

            /* ����ǿ��ӵ绰�������ǰû��MODEM�˿ڣ���ֱ�ӷ��� */
            if (usLoop == AT_MAX_CLIENT_NUM)
            {
                return;
            }

            /* ����DSR��RI�Ĺܽ��ź�,���ڴ�������ָʾ( ����ץȡ��̨�����
               USB�ܽ� ) */
            memset_s(&stMscStru, sizeof(stMscStru), 0x00, sizeof(AT_DCE_MSC_STRU));
            stMscStru.OP_Dsr = 1;
            stMscStru.ucDsr  = 1;
            stMscStru.OP_Ri = 1;
            stMscStru.ucRi  = 1;
            stMscStru.OP_Dcd = 1;
            stMscStru.ucDcd  = 1;
            AT_SetModemStatus((VOS_UINT8)usLoop,&stMscStru);

            /* Ϊ�˱�֤���ߵĹܽ��ź���PC���ȴ���������ʱ�Ĵ���  */
            ulDelayaCnt = 50000;
            while( ulDelayaCnt-- )
            {
                ;
            }

            /*EVENT-UE UP DCD*/
            AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DCE_UP_DCD,
                            VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);
        }

#if (FEATURE_IMS == FEATURE_ON)
        if (pstIncomingInd->stIncomingIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%sIRING%s",
                                                gaucAtCrLf,
                                                gaucAtCrLf);
        }
        else
#endif
        {
            /* +CRC -- RING */
            if( gucAtVType == AT_V_ENTIRE_TYPE )
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                    "%sRING%s",
                                                    gaucAtCrLf,
                                                    gaucAtCrLf);
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                    "2\r");
            }

        }

    }

    if( pstSsCtx->ucClipType == AT_CLIP_ENABLE_TYPE )
    {
        /*
        +CLIP: <number>,<type>
        ��������[,<subaddr>,<satype>[,[<alpha>][,<CLI validity>]]]�����ϱ�
        */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s+CLIP: ",
                                            gaucAtCrLf);

        ucCLIValid = AT_ConvertCLIValidity(pstIncomingInd);

        if( pstIncomingInd->stIncomingIndPara.stCallNumber.ucNumLen != 0 )
        {
            AT_BcdNumberToAscii(pstIncomingInd->stIncomingIndPara.stCallNumber.aucBcdNum,
                                pstIncomingInd->stIncomingIndPara.stCallNumber.ucNumLen,
                                (VOS_CHAR *)aucAsciiNum);

           /* �����������Ϊ���ʺ��룬��Ҫ�ں���ǰ��+,�����ز�ʧ�� */
           if (((pstIncomingInd->stIncomingIndPara.stCallNumber.enNumType >> 4) & 0x07) == MN_MSG_TON_INTERNATIONAL)
           {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "\"+%s\",%d,\"\",,\"\",%d",
                                                    aucAsciiNum,
                                                    (pstIncomingInd->stIncomingIndPara.stCallNumber.enNumType | AT_NUMBER_TYPE_EXT),
                                                    ucCLIValid);
           }
           else
           {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                    "\"%s\",%d,\"\",,\"\",%d",
                                                    aucAsciiNum,
                                                    (pstIncomingInd->stIncomingIndPara.stCallNumber.enNumType | AT_NUMBER_TYPE_EXT),
                                                    ucCLIValid);
           }
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "\"\",,\"\",,\"\",%d",
                                                ucCLIValid);
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s",
                                            gaucAtCrLf);
    }

    if ( pstSsCtx->ucSalsType == AT_SALS_ENABLE_TYPE )
    {
        /*�ϱ�����·1������·2������*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s^ALS: ",
                                           gaucAtCrLf);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           pstIncomingInd->stIncomingIndPara.enAlsLineNo);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           gaucAtCrLf);
    }

    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);

    /* �ϱ�+CNAP���� */
    AT_ReportCnapInfo(ucIndex, &(pstIncomingInd->stIncomingIndPara.stNameIndicator));

    /* ֻ�к�������Ϊvoiceʱ��֧���Զ��������ܣ�����������ʱ��֧���Զ����� */
    if ((pstIncomingInd->stIncomingIndPara.enCallType == MN_CALL_TYPE_VOICE)
     && (pstCcCtx->stS0TimeInfo.ucS0TimerLen != 0))
    {
        /* ����Զ���������û�������յ�RING�¼������� */
        if (pstCcCtx->stS0TimeInfo.bTimerStart != TAF_TRUE)
        {
            ulTimerName = AT_S0_TIMER;
            ulTimerName |= AT_INTERNAL_PROCESS_TYPE;
            ulTimerName |= (ucIndex<<12);

            AT_StartRelTimer(&(pstCcCtx->stS0TimeInfo.s0Timer),
                              (pstCcCtx->stS0TimeInfo.ucS0TimerLen)*1000,
                              ulTimerName,
                              pstIncomingInd->stIncomingIndPara.callId,
                              VOS_RELTIMER_NOLOOP);
            pstCcCtx->stS0TimeInfo.bTimerStart = TAF_TRUE;
            pstCcCtx->stS0TimeInfo.ulTimerName = ulTimerName;
            AT_PR_LOGH("At_CsIncomingEvtOfIncomeStateIndProc: S0TimerLen = %d", pstCcCtx->stS0TimeInfo.ucS0TimerLen);
        }
    }


    if (AT_EVT_IS_CS_VIDEO_CALL(pstIncomingInd->stIncomingIndPara.enCallType, pstIncomingInd->stIncomingIndPara.enVoiceDomain))
    {
        /* Ϊ�˱�֤���������Ϣ(��RING���������)��PC���ȴ���������ʱ�Ĵ���  */
        ulDelayaCnt = 50000;
        while( ulDelayaCnt-- )
        {
            ;
        }

        /* �ϱ�RING֮������RI�Ĺܽ��źţ���Ȼ����DSR�Ĺܽ��ź�,
           ( ����ץȡ��̨�����USB�ܽ��źŽ��� ) */
        memset_s(&stMscStru, sizeof(stMscStru), 0x00, sizeof(AT_DCE_MSC_STRU));
        stMscStru.OP_Ri = 1;
        stMscStru.ucRi  = 0;
        stMscStru.OP_Dcd = 1;
        stMscStru.ucDcd  = 1;
        stMscStru.OP_Dsr = 1;
        stMscStru.ucDsr  = 1;
        AT_SetModemStatus((VOS_UINT8)usLoop,&stMscStru);
    }

    return;

}


TAF_VOID At_CsIncomingEvtOfWaitStateIndProc(
    TAF_UINT8                           ucIndex,
    TAF_CCM_CALL_INCOMING_IND_STRU     *pstIncomingInd
)
{
    TAF_UINT16 usLength;
    TAF_UINT8  aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN*2)+1];
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;
    AT_CLI_VALIDITY_ENUM_UINT8          enCliValidity;

    usLength = 0;
    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    if( pstSsCtx->ucCcwaType == AT_CCWA_ENABLE_TYPE )         /* ������Э�鲻�� */
    {
#if (FEATURE_IMS == FEATURE_ON)
        if (pstIncomingInd->stIncomingIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s^CCWA: ",
                                                gaucAtCrLf);
        }
        else
#endif
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s+CCWA: ",
                                                gaucAtCrLf);
        }

        /* �տڴ浽TAF��Cause of No CLI��AT�����CLI Validity�в��� */
        enCliValidity = AT_ConvertCLIValidity(pstIncomingInd);

        /* 27007 7.12
           When CLI is not available, <number> shall be an empty string ("")
           and <type> value will not be significant. TA may return the recommended
           value 128 for <type>. */
        if (enCliValidity == AT_CLI_VALIDITY_VALID)
        {
            AT_BcdNumberToAscii(pstIncomingInd->stIncomingIndPara.stCallNumber.aucBcdNum,
                                pstIncomingInd->stIncomingIndPara.stCallNumber.ucNumLen,
                                (VOS_CHAR *)aucAsciiNum);

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "\"%s\",%d",
                                                aucAsciiNum,
                                                (pstIncomingInd->stIncomingIndPara.stCallNumber.enNumType | AT_NUMBER_TYPE_EXT));
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "\"\",%d",
                                                AT_NUMBER_TYPE_EXT);
        }

        if ((pstIncomingInd->stIncomingIndPara.enCallType == MN_CALL_TYPE_VOICE)
         || (pstIncomingInd->stIncomingIndPara.enCallType == MN_CALL_TYPE_PSAP_ECALL))
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",1");
        }
        else if( pstIncomingInd->stIncomingIndPara.enCallType == MN_CALL_TYPE_VIDEO )
        {
            if (pstIncomingInd->stIncomingIndPara.enVoiceDomain == TAF_CALL_VOICE_DOMAIN_IMS)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                    ",2");
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                    ",32");
            }
        }
        else if( pstIncomingInd->stIncomingIndPara.enCallType == MN_CALL_TYPE_FAX )
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",4");
        }
        else if( pstIncomingInd->stIncomingIndPara.enCallType == MN_CALL_TYPE_CS_DATA )
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                ",2");
        }
        else
        {

        }

        /* <alpha>,<CLI_validity> */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                        ",,%d",
                                                        enCliValidity);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s",
                                            gaucAtCrLf);

        if ( pstSsCtx->ucSalsType == AT_SALS_ENABLE_TYPE)
        {
            /*�ϱ�����·1������·2������*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (TAF_CHAR *)pgucAtSndCodeAddr,
                                              (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s^ALS: ",
                                              gaucAtCrLf);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (TAF_CHAR *)pgucAtSndCodeAddr,
                                              (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d",
                                               pstIncomingInd->stIncomingIndPara.enAlsLineNo);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (TAF_CHAR *)pgucAtSndCodeAddr,
                                              (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s",
                                               gaucAtCrLf);
        }
    }

    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    return;
}


VOS_UINT32 At_RcvTafCcmCallIncomingInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_INCOMING_IND_STRU     *pstIncomingInd = VOS_NULL_PTR;
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;
    TAF_UINT8                           ucIndex = 0;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    pstIncomingInd  = (TAF_CCM_CALL_INCOMING_IND_STRU *)pMsg;
    g_ucDtrDownFlag = VOS_FALSE;


    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstIncomingInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_CsIncomingEvtIndProc: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    pstCcCtx        = AT_GetModemCcCtxAddrFromClientId(ucIndex);


    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_CsRspEvtConnectProc: Get modem id fail.");
        return VOS_ERR;
    }

    /*
    ��Ҫ�����������ͣ����棬���ݣ����ӵ绰����������
    */
    /*
    +CRC -- +CRING: <type> || RING
    +CLIP: <number>,<type>[,<subaddr>,<satype>[,[<alpha>][,<CLI validity>]]]
    */
    if ( pstIncomingInd->stIncomingIndPara.enCallState == MN_CALL_S_INCOMING )
    {
        At_CsIncomingEvtOfIncomeStateIndProc(ucIndex,pstIncomingInd);
        AT_ReportCCallstateResult(enModemId,
                                  pstIncomingInd->stIncomingIndPara.callId,
                                  pstIncomingInd->stIncomingIndPara.aucCurcRptCfg,
                                  AT_CS_CALL_STATE_INCOMMING,
                                  pstIncomingInd->stIncomingIndPara.enVoiceDomain);
    }
    else if ( pstIncomingInd->stIncomingIndPara.enCallState == MN_CALL_S_WAITING )
    {
        At_CsIncomingEvtOfWaitStateIndProc(ucIndex,pstIncomingInd);
        AT_ReportCCallstateResult(enModemId,
                                  pstIncomingInd->stIncomingIndPara.callId,
                                  pstIncomingInd->stIncomingIndPara.aucCurcRptCfg,
                                  AT_CS_CALL_STATE_WAITING,
                                  pstIncomingInd->stIncomingIndPara.enVoiceDomain);
    }
    else
    {
        pstCcCtx->ulCurIsExistCallFlag = VOS_TRUE;
        return VOS_OK;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    if ( At_CheckUartRingTeCallType(pstIncomingInd->stIncomingIndPara.enCallType) == PS_TRUE )
    {
        /* ͨ��UART�˿ڵ�RING���������֪ͨTE */
        AT_VoiceStartRingTe(pstIncomingInd->stIncomingIndPara.callId);
    }
#endif
    pstCcCtx->ulCurIsExistCallFlag = VOS_TRUE;

    return VOS_OK;
}


VOS_UINT32 AT_ConCallMsgTypeToCuusiMsgType(
    MN_CALL_UUS1_MSG_TYPE_ENUM_U32      enMsgType,
    AT_CUUSI_MSG_TYPE_ENUM_U32          *penCuusiMsgType

)
{
    VOS_UINT32                          i;

    for ( i = 0 ; i < sizeof(g_stCuusiMsgType)/sizeof(AT_CALL_CUUSI_MSG_STRU) ; i++ )
    {
        if ( enMsgType == g_stCuusiMsgType[i].enCallMsgType)
        {
            *penCuusiMsgType = g_stCuusiMsgType[i].enCuusiMsgType;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}



VOS_UINT32 AT_ConCallMsgTypeToCuusuMsgType(
    MN_CALL_UUS1_MSG_TYPE_ENUM_U32      enMsgType,
    AT_CUUSU_MSG_TYPE_ENUM_U32          *penCuusuMsgType

)
{
    VOS_UINT32                          i;

    for ( i = 0 ; i < sizeof(g_stCuusuMsgType)/sizeof(AT_CALL_CUUSU_MSG_STRU) ; i++ )
    {
        if ( enMsgType == g_stCuusuMsgType[i].enCallMsgType)
        {
            *penCuusuMsgType = g_stCuusuMsgType[i].enCuusuMsgType;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32 AT_RcvTafCcmUus1InfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_UUS1_INFO_IND_STRU         *pstUus1InfoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgType;
    AT_CUUSI_MSG_TYPE_ENUM_U32          enCuusiMsgType;
    AT_CUUSU_MSG_TYPE_ENUM_U32          enCuusuMsgType;
    VOS_UINT32                          ulRet;
    MN_CALL_DIR_ENUM_U8                 enCallDir;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex = 0;

    pstUus1InfoInd = (TAF_CCM_UUS1_INFO_IND_STRU *)pMsg;

    /* ����ClientID��ȡͨ������ */
    if(At_ClientIdToUserId(pstUus1InfoInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallStartDtmfCnf: Get Index Fail!");
        return VOS_ERR;
    }

    enCallDir = pstUus1InfoInd->stUus1InfoIndPara.enCallDir;
    ulMsgType = AT_CUUSI_MSG_ANY;

    ulRet = AT_ConCallMsgTypeToCuusiMsgType(pstUus1InfoInd->stUus1InfoIndPara.stUusInfo.enMsgType,&enCuusiMsgType);
    if ( ulRet == VOS_OK )
    {
        ulMsgType = enCuusiMsgType;
        enCallDir = MN_CALL_DIR_MO;
    }
    else
    {
        ulRet = AT_ConCallMsgTypeToCuusuMsgType(pstUus1InfoInd->stUus1InfoIndPara.stUusInfo.enMsgType,&enCuusuMsgType);
        if ( ulRet == VOS_OK )
        {
            ulMsgType = enCuusuMsgType;
            enCallDir = MN_CALL_DIR_MT;
        }
    }

    if ( ulRet != VOS_OK)
    {
        if ( pstUus1InfoInd->stUus1InfoIndPara.enCallDir == MN_CALL_DIR_MO)
        {
            ulMsgType = AT_CUUSI_MSG_ANY;
        }
        else
        {
            ulMsgType = AT_CUUSU_MSG_ANY;
        }
    }

    usLength = 0;

    if ( enCallDir == MN_CALL_DIR_MO )
    {
        /* δ�����򲻽����κδ���,�����ϱ� */
        if ( AT_CheckRptCmdStatus(pstUus1InfoInd->stUus1InfoIndPara.aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CUUS1I) == VOS_FALSE)
        {
            return VOS_ERR;
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                   "%s+CUUS1I:",
                                   gaucAtCrLf);
    }
    else
    {

        /* δ�����򲻽����κδ���,�����ϱ� */
        if ( AT_CheckRptCmdStatus(pstUus1InfoInd->stUus1InfoIndPara.aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CUUS1U) == VOS_FALSE )
        {
            return VOS_ERR;
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                   "%s+CUUS1U:",
                                   gaucAtCrLf);
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%d,",
                                    ulMsgType);

    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                   (TAF_INT8 *)pgucAtSndCodeAddr,
                                   (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                   pstUus1InfoInd->stUus1InfoIndPara.stUusInfo.aucUuie,
                                   (pstUus1InfoInd->stUus1InfoIndPara.stUusInfo.aucUuie[MN_CALL_LEN_POS] + MN_CALL_UUIE_HEADER_LEN));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);

    return VOS_OK;
}


VOS_VOID At_ProcSetClccResult(
    VOS_UINT8                           ucNumOfCalls,
    TAF_CCM_QRY_CALL_INFO_CNF_STRU     *pstQryCallInfoCnf,
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           ucTmp;
    AT_CLCC_MODE_ENUM_U8                enClccMode;
    VOS_UINT8                           aucAsciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];
    VOS_UINT16                          usLength;

    VOS_UINT8                          ucNumberType;

    ucNumberType = AT_NUMBER_TYPE_UNKOWN;

    usLength = 0;

    if ( (ucNumOfCalls != 0)
        && ( ucNumOfCalls <=  AT_CALL_MAX_NUM))
    {
        for (ucTmp = 0; ucTmp < ucNumOfCalls; ucTmp++)
        {
            /* <CR><LF> */
            if(ucTmp != 0)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s",
                                                   gaucAtCrLf);
            }

            AT_MapCallTypeModeToClccMode(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallType, &enClccMode);

            /* +CLCC:  */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+ usLength,
                                               "%s: %d,%d,%d,%d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName, /* +CLCC:  */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].callId,             /* <id1>, */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallDir,          /* <dir>, */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallState,        /* <stat>, */
                                               enClccMode,                                          /* <mode>, */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enMptyState         /* <mpty>, */
                                               );

            if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallDir == MN_CALL_DIR_MO)
            {
                if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.ucNumLen != 0)
                {
                    /* <number>, */
                    AT_BcdNumberToAscii(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.aucBcdNum,
                                        pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.ucNumLen,
                                        (TAF_CHAR*)aucAsciiNum);

                    /* <type>,����<alpha>,<priority> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"%s\",%d,\"\",",
                                                       aucAsciiNum,
                                                       (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.enNumType | AT_NUMBER_TYPE_EXT));
                }
                else if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.ucNumLen != 0)
                {
                    /* <number>, */
                    AT_BcdNumberToAscii(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.aucBcdNum,
                                        pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.ucNumLen,
                                        (TAF_CHAR*)aucAsciiNum);

                    /* <type>,����<alpha>,<priority> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"%s\",%d,\"\",",
                                                       aucAsciiNum,
                                                       (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.enNumType | AT_NUMBER_TYPE_EXT));


                }
                else
                {
                    /* <type>,����<alpha>,<priority> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"\",%d",ucNumberType);
                }
            }
            else
            {
                if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.ucNumLen != 0)
                {
                    /* <number>, */
                    AT_BcdNumberToAscii(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.aucBcdNum,
                                        pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.ucNumLen,
                                        (VOS_CHAR *)aucAsciiNum);

                    /* <type>,����<alpha>,<priority> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"%s\",%d,\"\",",
                                                       aucAsciiNum,
                                                       (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.enNumType | AT_NUMBER_TYPE_EXT));
                }
                else
                {
                    /* <type>,����<alpha>,<priority> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"\",%d",ucNumberType);

                }
            }
        }
    }

    gstAtSendData.usBufLen = usLength;

}


VOS_VOID At_ReportClccDisplayName(
    MN_CALL_DISPLAY_NAME_STRU          *pstDisplayName,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                       i;

    /* ,<display name> */
    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           ",");

    if ( pstDisplayName->ucNumLen != 0 )
    {
        /* ��UTF8��ʽ��ʾ�����й���ӦE4B8ADE59BBD */
        for (i = 0; i < pstDisplayName->ucNumLen; i++)
        {
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                                   "%X",
                                                   (VOS_UINT8)pstDisplayName->acDisplayName[i]);
        }
    }

    return;
}


VOS_VOID At_ReportPeerVideoSupport(
    MN_CALL_INFO_PARAM_STRU            *pstCallInfo,
    VOS_UINT16                         *pusLength
)
{
    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                       ",");

    if (pstCallInfo->bitOpPeerVideoSupport == VOS_TRUE)
    {
        /* <terminal video support> */
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%d",pstCallInfo->enPeerVideoSupport);
    }

    return;
}


VOS_VOID At_ReportClccImsDomain(
    MN_CALL_INFO_PARAM_STRU            *pstCallInfo,
    VOS_UINT16                         *pusLength
)
{
    AT_IMS_CALL_DOMAIN_ENUM_UINT8       ucImsDomain;

    ucImsDomain = AT_IMS_CALL_DOMAIN_LTE;

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                       ",");

    if (pstCallInfo->enImsDomain != TAF_CALL_IMS_DOMAIN_NULL)
    {
        switch (pstCallInfo->enImsDomain)
        {
            case TAF_CALL_IMS_DOMAIN_LTE:
                ucImsDomain = AT_IMS_CALL_DOMAIN_LTE;
                break;

            case TAF_CALL_IMS_DOMAIN_WIFI:
                ucImsDomain = AT_IMS_CALL_DOMAIN_WIFI;
                break;
            case TAF_CALL_IMS_DOMAIN_NR:
                ucImsDomain = AT_IMS_CALL_DOMAIN_NR;
                break;

            default:
                break;
        }

        /* <imsDomain> */
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%d",ucImsDomain);

    }

    return;
}


VOS_VOID At_ReportClccRttPara(
    MN_CALL_INFO_PARAM_STRU            *pstCallInfo,
    VOS_UINT16                         *pusLength
)
{
    /* �����Ƿ�ΪRTTͨ��������Ҫ��� <RttFlg>,<RttChannelId>,<cps> ��������  */
    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           ",%d,%d,%d",
                                           pstCallInfo->enRtt,
                                           pstCallInfo->lRttChannelId,
                                           pstCallInfo->ulCps);

    return;
}



VOS_VOID At_ReportClccEncryptPara(
    MN_CALL_INFO_PARAM_STRU            *pstCallInfo,
    VOS_UINT16                         *pusLength
)
{
    /* ,<isEncrypt> */
    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           ",%d",
                                           pstCallInfo->ucEncryptFlag);

    return;
}


VOS_VOID At_ProcQryClccResult(
    VOS_UINT8                           ucNumOfCalls,
    TAF_CCM_QRY_CALL_INFO_CNF_STRU     *pstQryCallInfoCnf,
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           ucTmp;
    VOS_UINT8                           ucNumberType;
    VOS_UINT8                           aucAsciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];
    VOS_UINT16                          usLength;
    AT_CLCC_MODE_ENUM_U8                enClccMode;
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;

    ucNumberType    = AT_NUMBER_TYPE_UNKOWN;
    usLength        = 0;

    if ((ucNumOfCalls != 0)
     && ( ucNumOfCalls <=  AT_CALL_MAX_NUM))
    {
        for (ucTmp = 0; ucTmp < ucNumOfCalls; ucTmp++)
        {
            /* <CR><LF> */
            if(ucTmp != 0)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s",
                                                   gaucAtCrLf);
            }

            AT_MapCallTypeModeToClccMode(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallType, &enClccMode);

            if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enVoiceDomain == TAF_CALL_VOICE_DOMAIN_3GPP2)
            {
                enVoiceDomain = TAF_CALL_VOICE_DOMAIN_3GPP;
            }
            else
            {
                enVoiceDomain = pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enVoiceDomain;
            }

            /* ^CLCC:  */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr+ usLength,
                                               "%s: %d,%d,%d,%d,%d,%d,%d,%d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName, /* ^CLCC:  */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].callId,            /* <id1>, */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallDir,         /* <dir>, */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallState,       /* <stat>, */
                                               enClccMode,                                          /* <mode>, */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enMptyState,       /* <mpty>, */
                                               enVoiceDomain,                                       /* <voice_domain> */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallType,        /* <call_type> */
                                               pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].ucEConferenceFlag  /* <isEConference> */
                                               );

            if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallDir == MN_CALL_DIR_MO)
            {
                if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.ucNumLen != 0)
                {
                    AT_BcdNumberToAscii(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.aucBcdNum,
                                        pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.ucNumLen,
                                        (TAF_CHAR*)aucAsciiNum);

                    /* ,<number>,<type> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"%s\",%d",
                                                       aucAsciiNum,
                                                       (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stConnectNumber.enNumType | AT_NUMBER_TYPE_EXT));
                }
                else if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.ucNumLen != 0)
                {
                    AT_BcdNumberToAscii(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.aucBcdNum,
                                        pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.ucNumLen,
                                        (TAF_CHAR*)aucAsciiNum);

                    /* ,<number>,<type> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"%s\",%d",
                                                       aucAsciiNum,
                                                       (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCalledNumber.enNumType | AT_NUMBER_TYPE_EXT));
                }
                else
                {
                    /* ,,<type> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"\",%d",ucNumberType);
                }
            }
            else
            {
                if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.ucNumLen != 0)
                {
                    AT_BcdNumberToAscii(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.aucBcdNum,
                                        pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.ucNumLen,
                                        (VOS_CHAR *)aucAsciiNum);

                    /* ,<number>,<type> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"%s\",%d",
                                                       aucAsciiNum,
                                                       (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stCallNumber.enNumType | AT_NUMBER_TYPE_EXT));
                }
                else
                {
                    /* ,,<type> */
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                       ",\"\",%d",ucNumberType);
                }
            }

            At_ReportClccDisplayName(&(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].stDisplayName), &usLength);

            At_ReportPeerVideoSupport(&(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp]), &usLength);

            At_ReportClccImsDomain(&(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp]), &usLength);

            At_ReportClccRttPara(&(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp]), &usLength);

            At_ReportClccEncryptPara(&(pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp]), &usLength);
        }
    }

    gstAtSendData.usBufLen = usLength;

}
#if (FEATURE_IMS == FEATURE_ON)

VOS_UINT32 At_ProcQryClccEconfResult(
    TAF_CCM_QRY_ECONF_CALLED_INFO_CNF_STRU                 *pstCallInfos,
    VOS_UINT8                                               ucIndex
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucTmp;
    VOS_CHAR                            aucAsciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];

    usLength        = 0;
    memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));

     /* ^CLCCECONF: Maximum-user-count, n_address */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,  /* ^CLCCECONF:  */
                                      pstCallInfos->ucNumOfMaxCalls,                        /* Maximum-user-count */
                                      pstCallInfos->ucNumOfCalls);

    if (pstCallInfos->ucNumOfCalls != 0)
    {
        /* n_address */
        for (ucTmp = 0; ucTmp < pstCallInfos->ucNumOfCalls; ucTmp++)
        {
            /* ת���绰���� */
            if (pstCallInfos->astCallInfo[ucTmp].stCallNumber.ucNumLen != 0)
            {
                /* <number>, */
                AT_BcdNumberToAscii(pstCallInfos->astCallInfo[ucTmp].stCallNumber.aucBcdNum,
                                    pstCallInfos->astCallInfo[ucTmp].stCallNumber.ucNumLen,
                                    aucAsciiNum);
            }

            /* entity, Display-text,Status */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",\"%s\",\"%s\",%d",
                                               aucAsciiNum,                                         /* �绰���� */
                                               pstCallInfos->astCallInfo[ucTmp].aucDisplaytext,     /* display-text */
                                               pstCallInfos->astCallInfo[ucTmp].enCallState);       /* Call State */

        }
    }

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}
#endif


VOS_UINT32 AT_RcvTafCcmQryCallInfoCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucNumOfCalls;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucTmp;
    AT_CPAS_STATUS_ENUM_U8              enCpas;
    VOS_UINT32                          ulRet;
    TAF_CCM_QRY_CALL_INFO_CNF_STRU     *pstQryCallInfoCnf = VOS_NULL_PTR;

    /* ��ʼ�� */
    ucIndex  = 0;
    pstQryCallInfoCnf = (TAF_CCM_QRY_CALL_INFO_CNF_STRU *)pMsg;

    /* ��ȡ��ǰ���в�ΪIDLE̬�ĺ�����Ϣ */
    ucNumOfCalls = pstQryCallInfoCnf->stQryCallInfoPara.ucNumOfCalls;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstQryCallInfoCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_CsAllCallInfoEvtCnfProc:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_CsAllCallInfoEvtCnfProc: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ʽ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CLCC_SET)
    {
        /* CLCC����Ľ���ظ� */
        At_ProcSetClccResult(ucNumOfCalls, pstQryCallInfoCnf, ucIndex);

        ulRet = AT_OK;
    }
    else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CLCC_QRY)
    {
        /* ^CLCC?����Ľ���ظ� */
        At_ProcQryClccResult(ucNumOfCalls, pstQryCallInfoCnf, ucIndex);

        ulRet = AT_OK;
    }
    else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPAS_SET)
    {
        /* CPAS����Ľ���ظ� */
        if (ucNumOfCalls > AT_CALL_MAX_NUM)
        {
            At_FormatResultData(ucIndex, AT_CME_UNKNOWN);
            return VOS_OK;
        }

        if (ucNumOfCalls == 0)
        {
            enCpas = AT_CPAS_STATUS_READY;
        }
        else
        {
            enCpas = AT_CPAS_STATUS_CALL_IN_PROGRESS;
            for (ucTmp = 0; ucTmp < ucNumOfCalls; ucTmp++)
            {
                if (pstQryCallInfoCnf->stQryCallInfoPara.astCallInfos[ucTmp].enCallState == MN_CALL_S_INCOMING)
                {
                    enCpas = AT_CPAS_STATUS_RING;
                    break;
                }
            }
        }

        gstAtSendData.usBufLen  = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR*)pgucAtSndCodeAddr,
                                                        "%s: %d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        enCpas);

        ulRet = AT_OK;

    }
    else
    {
        return VOS_OK;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRet);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmQryAlsCnf(VOS_VOID *pMsg)
{
    TAF_CCM_QRY_ALS_CNF_STRU           *pstAlsCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLen;
    AT_MODEM_SS_CTX_STRU               *pstModemSsCtx = VOS_NULL_PTR;

    pstAlsCnf = (TAF_CCM_QRY_ALS_CNF_STRU *)pMsg;

    if (At_ClientIdToUserId(pstAlsCnf->stCtrl.usClientId, &ucIndex) != AT_SUCCESS)
    {
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_QryAlsCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    usLen                  = 0;
    ulResult               = AT_ERROR;
    gstAtSendData.usBufLen = 0;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstModemSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    if (pstAlsCnf->stQryAlsPara.ulRet == TAF_ERR_NO_ERROR )
    {
        usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLen,
                                        "%s: %d",
                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                        pstModemSsCtx->ucSalsType);

        usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + usLen,
                                        ",%d",
                                        pstAlsCnf->stQryAlsPara.enAlsLine);

        gstAtSendData.usBufLen = usLen;

        ulResult = AT_OK;
    }

    else
    {
        /* ����֧��ALS����ʱ����call�ϱ�����ʱ�䣬AT����error��AT�������Ƿ�֧�ָ����� */
        ulResult = AT_ERROR;
    }

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;

}

VOS_UINT32 AT_RcvTafCcmQryUus1InfoCnf(VOS_VOID *pMsg)
{
    TAF_CCM_QRY_UUS1_INFO_CNF_STRU     *pstUus1Cnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;
    VOS_UINT32                          i;
    VOS_UINT16                          usLen;
    VOS_UINT32                          ulUus1IFlg;
    VOS_UINT32                          ulUus1UFlg;

    pstUus1Cnf = (TAF_CCM_QRY_UUS1_INFO_CNF_STRU *)pMsg;

    if (At_ClientIdToUserId(pstUus1Cnf->stCtrl.usClientId, &ucIndex) != AT_SUCCESS)
    {
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_QryUus1Cnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    usLen                  = 0;
    ulResult               = AT_ERROR;
    gstAtSendData.usBufLen = 0;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ʼ��Ϊ�������ϱ� */
    ulUus1IFlg  = VOS_TRUE;
    ulUus1UFlg  = VOS_TRUE;

    /* UUS1I�Ƿ�� */
    if (pstUus1Cnf->stQryUss1Info.aenSetType[0] == MN_CALL_CUUS1_DISABLE)
    {
        ulUus1IFlg  = VOS_FALSE;
    }

    /* UUS1U�Ƿ�� */
    if (pstUus1Cnf->stQryUss1Info.aenSetType[1] == MN_CALL_CUUS1_DISABLE)
    {
        ulUus1UFlg  = VOS_FALSE;
    }

    if (pstUus1Cnf->stQryUss1Info.ulRet == TAF_ERR_NO_ERROR )
    {
        usLen +=  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR*)pgucAtSndCodeAddr,
                                         "%s",
                                         gaucAtCrLf);

        usLen +=  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR*)pgucAtSndCodeAddr + usLen,
                                         "%s:",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)(pgucAtSndCodeAddr + usLen),
                                        "%d,%d",
                                        ulUus1IFlg,
                                        ulUus1UFlg);

        for ( i = 0 ; i < pstUus1Cnf->stQryUss1Info.ulActNum ; i++ )
        {
            usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)(pgucAtSndCodeAddr + usLen),
                                            ",%d,",
                                            pstUus1Cnf->stQryUss1Info.stUus1Info[i].enMsgType);

            usLen += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                         (VOS_INT8 *)pgucAtSndCodeAddr,
                                                         (VOS_UINT8 *)pgucAtSndCodeAddr+usLen,
                                                         pstUus1Cnf->stQryUss1Info.stUus1Info[i].aucUuie,
                                                         pstUus1Cnf->stQryUss1Info.stUus1Info[i].aucUuie[MN_CALL_LEN_POS] + MN_CALL_UUIE_HEADER_LEN);

        }

        gstAtSendData.usBufLen = usLen;

        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;

}



VOS_UINT32 AT_RcvTafCcmSetAlsCnf(VOS_VOID *pMsg)
{
    TAF_CCM_SET_ALS_CNF_STRU      *pstSetAlsCnf = VOS_NULL_PTR;
    VOS_UINT8                      ucIndex;
    VOS_UINT32                     ulResult;

    pstSetAlsCnf = (TAF_CCM_SET_ALS_CNF_STRU *)pMsg;

    if (At_ClientIdToUserId(pstSetAlsCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_SetAlsCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetAlsCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if( pstSetAlsCnf->ulRet == TAF_ERR_NO_ERROR )
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_CME_UNKNOWN;
    }

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmSetUus1InfoCnf(VOS_VOID *pMsg)
{
    TAF_CCM_SET_UUS1_INFO_CNF_STRU *pstUus1Cnf = VOS_NULL_PTR;
    VOS_UINT8                       ucIndex;
    VOS_UINT32                      ulResult;

    pstUus1Cnf = (TAF_CCM_SET_UUS1_INFO_CNF_STRU *)pMsg;

    if (At_ClientIdToUserId(pstUus1Cnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_SetUus1Cnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetUus1Cnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstUus1Cnf->ulRet == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_CME_INCORRECT_PARAMETERS;
    }

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;

}


TAF_UINT32 At_CcfcQryReport (
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT8  ucTmp    = 0;
    TAF_UINT16 usLength = 0;

    /*
    +CCFC: <status>,<class1>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]]
    */
    if(pEvent->OP_SsStatus == 1)
    {
        /* +CCFC: <status>,<class1> */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                    "%s: ",
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                    "%d,%d",
                    (TAF_SS_ACTIVE_STATUS_MASK & pEvent->SsStatus),
                    AT_CC_CALSS_TYPE_INVALID);

        return usLength;
    }

    if (pEvent->OP_FwdFeaturelist == 1)
    {
        for(ucTmp = 0; ucTmp < pEvent->FwdFeaturelist.ucCnt; ucTmp++)
        {
            if(ucTmp != 0)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                            "%s",
                            gaucAtCrLf);
            }

            /* +CCFC:  */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                        (TAF_CHAR *)pgucAtSndCodeAddr,
                        (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                        "%s: ",
                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

            /* <status> */
            if(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_SsStatus == 1)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                            "%d",
                            (TAF_SS_ACTIVE_STATUS_MASK
                            & (pEvent->FwdFeaturelist.astFwdFtr[ucTmp].SsStatus)));
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                            "0");
            }

            /* <class1> */
            if(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_BsService == 1)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                            ",%d",
                            At_GetClckClassFromBsCode(&(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].BsService)));
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                            ",%d",
                            AT_CC_CALSS_TYPE_INVALID);
            }

            /* <number> */
            if(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_FwdToNum == 1)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                            ",\"%s\"",
                            pEvent->FwdFeaturelist.astFwdFtr[ucTmp].aucFwdToNum);

                /* <type> */
                if(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_NumType == 1)
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                ",%d",
                                pEvent->FwdFeaturelist.astFwdFtr[ucTmp].NumType);
                }
                else
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                ",%d",
                                At_GetCodeType(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].aucFwdToNum[0]));
                }

                /* <subaddr> */
                if (pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_FwdToSubAddr == 1)
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                ",\"%s\"",
                                pEvent->FwdFeaturelist.astFwdFtr[ucTmp].aucFwdToSubAddr);

                    /* <satype> */
                    if(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_SubAddrType == 1)
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                    ",%d",
                                    pEvent->FwdFeaturelist.astFwdFtr[ucTmp].SubAddrType);
                    }
                    else
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                    ",%d",
                                    At_GetCodeType(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].aucFwdToSubAddr[0]));
                    }
                }
                else
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                ",,");
                }

                /* <time> */
                if(pEvent->FwdFeaturelist.astFwdFtr[ucTmp].OP_NoRepCondTime == 1)
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                ",%d",
                                pEvent->FwdFeaturelist.astFwdFtr[ucTmp].NoRepCondTime);
                }
                else
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                ",");
                }
            }


        }

    }

    return usLength;
}


VOS_UINT32 At_ProcReportUssdStr_Nontrans(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT16                          usPrintOffSet
)
{
    TAF_SS_USSD_STRING_STRU             stUssdStrBuff;
    MN_MSG_CBDCS_CODE_STRU              stDcsInfo;
    VOS_UINT32                          ulDefAphaLen;
    VOS_UINT32                          ulAsciiStrLen;
    errno_t                             lMemResult;
    VOS_UINT16                          usOutPrintOffSet;
    VOS_UINT32                          ulRet;

    usOutPrintOffSet = 0;

    if (pstEvent->UssdString.usCnt == 0)
    {
        AT_WARN_LOG("At_ProcReportUssdStr_Nontrans: UssdString Cnt is 0.");
        return usOutPrintOffSet;
    }

    memset_s(&stDcsInfo, sizeof(stDcsInfo), 0x00, sizeof(stDcsInfo));

    /* USSD��CBS��DCS��Э����ͬ������CBS��DCS�����������룬��ϸ����ο�23038 */
    ulRet = MN_MSG_DecodeCbsDcs(pstEvent->DataCodingScheme,
                                pstEvent->UssdString.aucUssdStr,
                                pstEvent->UssdString.usCnt,
                                &stDcsInfo);

    if (ulRet != MN_ERR_NO_ERROR)
    {
        AT_WARN_LOG("At_ProcReportUssdStr_Nontrans:WARNING: Decode Failure");
        return usOutPrintOffSet;
    }

    /* �ȴ���UCS2���� */
    if (stDcsInfo.enMsgCoding == MN_MSG_MSG_CODING_UCS2)
    {
        usOutPrintOffSet = (TAF_UINT16)At_Unicode2UnicodePrint(AT_CMD_MAX_LEN,
                                                               (TAF_INT8 *)pgucAtSndCodeAddr,
                                                               pgucAtSndCodeAddr + usPrintOffSet,
                                                               pstEvent->UssdString.aucUssdStr,
                                                               pstEvent->UssdString.usCnt);
    }
    else
    {
        /* 7Bit��Ҫ�����Ƚ��룬��һ����Ascii���� */
        if (stDcsInfo.enMsgCoding == MN_MSG_MSG_CODING_7_BIT)
        {
            memset_s(&stUssdStrBuff,
                     sizeof(TAF_SS_USSD_STRING_STRU),
                     0,
                     sizeof(TAF_SS_USSD_STRING_STRU));

            ulDefAphaLen = pstEvent->UssdString.usCnt * 8 / 7;

            (VOS_VOID)TAF_STD_UnPack7Bit(pstEvent->UssdString.aucUssdStr,
                                         ulDefAphaLen,
                                         0,
                                         stUssdStrBuff.aucUssdStr);

            if ((stUssdStrBuff.aucUssdStr[ulDefAphaLen - 1]) == 0x0d)
            {
                ulDefAphaLen--;
            }

            ulAsciiStrLen = 0;

            TAF_STD_ConvertDefAlphaToAscii(stUssdStrBuff.aucUssdStr,
                                           ulDefAphaLen,
                                           stUssdStrBuff.aucUssdStr,
                                           &ulAsciiStrLen);

            stUssdStrBuff.usCnt = (VOS_UINT16)ulAsciiStrLen;
        }
        /* �������:8Bit ֱ�ӿ��� */
        else
        {
            lMemResult = memcpy_s(&stUssdStrBuff,
                                  sizeof(TAF_SS_USSD_STRING_STRU),
                                  &(pstEvent->UssdString),
                                  sizeof(TAF_SS_USSD_STRING_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_SS_USSD_STRING_STRU), sizeof(TAF_SS_USSD_STRING_STRU));
        }

        /* ��͸��ģʽ���� */
        if(gucAtCscsType == AT_CSCS_UCS2_CODE)       /* +CSCS:UCS2 */
        {
            usOutPrintOffSet = (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN,
                                                               (TAF_INT8 *)pgucAtSndCodeAddr,
                                                               pgucAtSndCodeAddr + usPrintOffSet,
                                                               stUssdStrBuff.aucUssdStr,
                                                               stUssdStrBuff.usCnt);
        }
        else
        {
            lMemResult = memcpy_s((TAF_CHAR *)pgucAtSndCodeAddr + usPrintOffSet,
                                  stUssdStrBuff.usCnt,
                                  stUssdStrBuff.aucUssdStr,
                                  stUssdStrBuff.usCnt);
            TAF_MEM_CHK_RTN_VAL(lMemResult, stUssdStrBuff.usCnt, stUssdStrBuff.usCnt);
            usOutPrintOffSet = stUssdStrBuff.usCnt;
        }
    }

    return usOutPrintOffSet;
}


VOS_UINT16 AT_PrintUssdStr(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           ucIndex,
    VOS_UINT16                          usLength
)
{
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;
    VOS_UINT16                          usPrintOffSet;

    /* û��USSD STRING��Ҫ��ӡ */
    /* �����USSD �ַ����ϱ����ش�DCS�� */
    if (pstEvent->OP_DataCodingScheme == 0)
    {
        AT_WARN_LOG("AT_PrintUssdStr: No DCS.");
        return usLength;
    }

    /* ��������ϱ����ַ�������USSDSting�� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        if (pstEvent->OP_UssdString == 0)
        {
            AT_WARN_LOG("AT_PrintUssdStr: BroadCast,No UssdString.");
            return usLength;
        }

    }
    else
    {
        /* ����������ϱ����ַ����ɷ���USSDSting�У�Ҳ�ɷ���USSData�� */
        /* ������29�ܾ����ط���������Ļظ��Ƿ���USSData�� */
        if ((pstEvent->OP_UssdString == 0)
         && (pstEvent->OP_USSData == 0))
        {
            AT_WARN_LOG("AT_PrintUssdStr: No UssdSting & UssData.");
            return usLength;
        }

    }

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    usPrintOffSet  = usLength;
    usPrintOffSet += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usPrintOffSet,",\"");

    if (pstEvent->UssdString.usCnt > sizeof(pstEvent->UssdString.aucUssdStr))
    {
        AT_WARN_LOG1("AT_PrintUssdStr: Invalid pstEvent->UssdString.usCnt: ", pstEvent->UssdString.usCnt);
        pstEvent->UssdString.usCnt = sizeof(pstEvent->UssdString.aucUssdStr);
    }

    switch(pstSsCtx->usUssdTransMode)
    {
        case AT_USSD_TRAN_MODE:
            usPrintOffSet += (TAF_UINT16)At_HexString2AsciiNumPrint(AT_CMD_MAX_LEN,
                                                                    (TAF_INT8 *)pgucAtSndCodeAddr,
                                                                    pgucAtSndCodeAddr + usPrintOffSet,
                                                                    pstEvent->UssdString.aucUssdStr,
                                                                    pstEvent->UssdString.usCnt);
            break;

        case AT_USSD_NON_TRAN_MODE:
            /* �����͸��ģʽ���ϱ���7 8Bit UssdString */
            usPrintOffSet += (TAF_UINT16)At_ProcReportUssdStr_Nontrans(pstEvent, usPrintOffSet);
            break;

        default:
            break;
    }

    /* <dcs> */
    usPrintOffSet += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usPrintOffSet,
                                       "\",%d",pstEvent->DataCodingScheme);

    return usPrintOffSet;
}


TAF_VOID At_SsIndProc(TAF_UINT8  ucIndex,TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pstEvent)
{
    TAF_UINT16                          usLength = 0;
    TAF_UINT8                           ucTmp    = 0;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    switch(pstEvent->SsEvent)             /* �����¼� */
    {
        case TAF_SS_EVT_USS_NOTIFY_IND:                     /* ֪ͨ�û����ý�һ������ */
        case TAF_SS_EVT_USS_REQ_IND:                        /* ֪ͨ�û���һ������ */
        case TAF_SS_EVT_USS_RELEASE_COMPLETE_IND:           /* ֪ͨ�û������ͷ� */
        case TAF_SS_EVT_PROCESS_USS_REQ_CNF:
            /* <m> */
            if(pstEvent->SsEvent == TAF_SS_EVT_USS_NOTIFY_IND)
            {
                ucTmp = 0;
            }
            else if(pstEvent->SsEvent == TAF_SS_EVT_USS_REQ_IND)
            {
                ucTmp = 1;
            }
            else
            {

                if (*pucSystemAppConfig == SYSTEM_APP_ANDROID)
                {
                    ucTmp = 2;
                }
                else
                {
                    ucTmp = 0;
                }
            }
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                              (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s+CUSD: ",gaucAtCrLf);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",ucTmp);
            /* <str> */

            usLength  = AT_PrintUssdStr(pstEvent, ucIndex, usLength);

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr +
                                               usLength,"%s",gaucAtCrLf);
            At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
            return;



        case TAF_SS_EVT_ERROR:
            if (pstEvent->ErrorCode == TAF_ERR_USSD_NET_TIMEOUT)
            {

                usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   "%s+CUSD: %d%s",
                                                   gaucAtCrLf,
                                                   AT_CUSD_M_NETWORK_TIMEOUT,
                                                   gaucAtCrLf);

                At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

                return;
            }

            if (pstEvent->ErrorCode == TAF_ERR_USSD_USER_TIMEOUT)
            {

                usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   "%s+CUSD: %d%s",
                                                   gaucAtCrLf,
                                                   AT_CUSD_M_NETWORK_CANCEL,
                                                   gaucAtCrLf);

                At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

                return;
            }

            break;

        default:
            return;
    }

}



TAF_UINT8 At_GetClckClassFromBsCode(TAF_SS_BASIC_SERVICE_STRU *pstBs)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulItemsNum;

    ulItemsNum = sizeof(g_astClckClassServiceExtTbl) / sizeof(AT_CLCK_CLASS_SERVICE_TBL_STRU);

    /* ����ȡ�������ͼ��������Ӧ��Class */
    for (ulLoop = 0; ulLoop < ulItemsNum; ulLoop++)
    {
        if ( (g_astClckClassServiceExtTbl[ulLoop].enServiceType == pstBs->BsType)
          && (g_astClckClassServiceExtTbl[ulLoop].enServiceCode == pstBs->BsServiceCode) )
        {
            return g_astClckClassServiceExtTbl[ulLoop].enClass;
        }
    }

    return AT_UNKNOWN_CLCK_CLASS;
}


TAF_UINT32 At_SsRspCusdProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent
)
{
    TAF_UINT32                          ulResult;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if(pEvent->SsEvent == TAF_SS_EVT_ERROR)
    {
        /* ���ط�������: ���+CUSD״̬ */
        ulResult          = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);       /* �������� */
    }
    else
    {
        /* �ȱ�OK�ٷ������ַ��� */
        ulResult          = AT_OK;
    }

    At_FormatResultData(ucIndex,ulResult);

    return ulResult;
}


TAF_VOID At_SsRspInterrogateCnfClipProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTmp    = 0;
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    /* +CLIP: <n>,<m> */
    if(pEvent->OP_SsStatus == 1)    /* �鵽״̬ */
    {
        ucTmp = (TAF_SS_PROVISIONED_STATUS_MASK & pEvent->SsStatus) ? 1 : 0;
    }
    else    /* û�в鵽״̬ */
    {
        ucTmp = 2;
    }

    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (TAF_CHAR *)pgucAtSndCodeAddr,
                                         (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s: %d,%d",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                         pstSsCtx->ucClipType,
                                         ucTmp);

    *pulResult = AT_OK;
}


TAF_VOID At_SsRspInterrogateCnfColpProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTmp    = 0;
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    if(pEvent->OP_SsStatus == 1)    /* �鵽״̬ */
    {
        ucTmp = (TAF_SS_ACTIVE_STATUS_MASK & pEvent->SsStatus);
    }
    else    /* û�в鵽״̬ */
    {
        ucTmp = 2;
    }

    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (TAF_CHAR *)pgucAtSndCodeAddr,
                                         (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s: %d,%d",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                         pstSsCtx->ucColpType,
                                         ucTmp);

    *pulResult = AT_OK;

}


TAF_VOID At_SsRspInterrogateCnfClirProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTmp    = 0;
    TAF_SS_CLI_RESTRICION_OPTION        ucClirTmp ;
    TAF_UINT8                           ucCliSsStatus;
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;

    pstSsCtx = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    if (pEvent->OP_GenericServiceInfo == 1) /* �鵽״̬ */
    {
        ucCliSsStatus = TAF_SS_ACTIVE_STATUS_MASK &pEvent->GenericServiceInfo.SsStatus;
        if (ucCliSsStatus)
        {
            if ( pEvent->GenericServiceInfo.OP_CliStrictOp == 1)
            {
               ucClirTmp = pEvent->GenericServiceInfo.CliRestrictionOp;
               if (ucClirTmp == TAF_SS_CLI_PERMANENT)
               {
                 ucTmp = 1;
               }
               else if (ucClirTmp == TAF_SS_CLI_TMP_DEFAULT_RESTRICTED)
               {
                 ucTmp = 3;
               }
               else if (ucClirTmp == TAF_SS_CLI_TMP_DEFAULT_ALLOWED)
               {
                 ucTmp = 4;
               }
               else
               {
                 ucTmp = 2;
               }
            }
            else
            {
               ucTmp = 2;
            }
        }
        else
        {
            ucTmp = 0;
        }
    }
    else if (pEvent->OP_SsStatus == 1)
    {
        ucTmp = 0;
    }
    else /* û�в鵽״̬ */
    {
        ucTmp = 2;
    }

    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (TAF_CHAR *)pgucAtSndCodeAddr,
                                         (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                         "%s: %d,%d",
                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                         pstSsCtx->ucClirType,
                                         ucTmp);

    *pulResult = AT_OK;

}


TAF_VOID At_SsRspInterrogateCnfClckProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTmp    = 0;
    TAF_UINT32                          i;
    VOS_UINT32                          ulCustomizeFlag;
    VOS_UINT32                          ulSuccessFlg;
    errno_t                             lMemResult;

    /* +CLCK: <status>,<class1> */
    if(pEvent->OP_Error == 1)       /* ��Ҫ�����жϴ����� */
    {
        *pulResult = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);       /* �������� */
        return;
    }

    if(pEvent->OP_SsStatus == 1)    /* �鵽״̬ */
    {
        ucTmp = (TAF_SS_ACTIVE_STATUS_MASK & pEvent->SsStatus);
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             "%s: %d,%d",
                                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                             ucTmp,
                                             AT_CLCK_PARA_CLASS_ALL);

        /* �������IE SS-STATUSֵ���û� */
        ulCustomizeFlag = AT_GetSsCustomizePara(AT_SS_CUSTOMIZE_CLCK_QUERY);
        if (ulCustomizeFlag == VOS_TRUE)
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d",
                                                 pEvent->SsStatus);
        }
    }
    else if(pEvent->OP_BsServGroupList == 1)
    {
        ulSuccessFlg = VOS_FALSE;
        for (i=0; i<pEvent->BsServGroupList.ucCnt; i++)
        {
            /* �˴���ucTmp����class��������status���� */
            ucTmp = At_GetClckClassFromBsCode(&pEvent->BsServGroupList.astBsService[i]);
            if (ucTmp != AT_UNKNOWN_CLCK_CLASS)
            {
                ulSuccessFlg = VOS_TRUE;
                *pusLength  += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: %d,%d%s",g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 1, ucTmp, gaucAtCrLf);
            }
        }

        if (ulSuccessFlg == VOS_TRUE)
        {
            *pusLength -= (TAF_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
            lMemResult = memset_s((VOS_UINT8*)pgucAtSndCodeAddr + *pusLength,
                                  AT_CMD_MAX_LEN - *pusLength,
                                  0x0,
                                  AT_CMD_MAX_LEN - *pusLength);
            TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN - *pusLength, AT_CMD_MAX_LEN - *pusLength);
        }

        if (ulSuccessFlg == VOS_FALSE)
        {
            AT_WARN_LOG("+CLCK - Unknown class.");
            *pulResult = AT_ERROR;
            return;
        }

    }
    else    /* û�в鵽״̬ */
    {
        ucTmp = 0;
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: %d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,ucTmp);
    }

    *pulResult = AT_OK;

}


TAF_VOID At_SsRspInterrogateCnfCcwaProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTmp    = 0;
    TAF_UINT32                          i;
    VOS_UINT32                          ulCustomizeFlag;
    VOS_UINT32                          ulSuccessFlg;
    errno_t                             lMemResult;


    /* +CCWA: <status>,<class1> */
    if(pEvent->OP_Error == 1)       /* ��Ҫ�����жϴ����� */
    {
        *pulResult = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);       /* �������� */
        return;
    }

    if (pEvent->OP_SsStatus == 1)
    {
        /* ״̬Ϊ���� */
        ucTmp = (TAF_SS_ACTIVE_STATUS_MASK & pEvent->SsStatus);
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             "%s: %d,%d",
                                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                             ucTmp,
                                             AT_CLCK_PARA_CLASS_ALL);

        /* �������IE SS-STATUSֵ���û� */
        ulCustomizeFlag = AT_GetSsCustomizePara(AT_SS_CUSTOMIZE_CCWA_QUERY);
        if (ulCustomizeFlag == VOS_TRUE)
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 ",%d",
                                                 pEvent->SsStatus);
        }
    }
    else if(pEvent->OP_BsServGroupList == 1)
    {
        ulSuccessFlg = VOS_FALSE;
        for (i=0; i<pEvent->BsServGroupList.ucCnt; i++)
        {
            /* �˴���ucTmp����class��������status���� */
            ucTmp = At_GetClckClassFromBsCode(&pEvent->BsServGroupList.astBsService[i]);
            if (ucTmp != AT_UNKNOWN_CLCK_CLASS)
            {
                ulSuccessFlg = VOS_TRUE;
                *pusLength  += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: %d,%d%s",g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 1, ucTmp,gaucAtCrLf);
            }
        }

        if (ulSuccessFlg == VOS_TRUE)
        {
            *pusLength -= (TAF_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
            lMemResult = memset_s((VOS_UINT8*)pgucAtSndCodeAddr + *pusLength,
                                   AT_CMD_MAX_LEN - *pusLength,
                                   0x0,
                                   AT_CMD_MAX_LEN - *pusLength);
            TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN - *pusLength, AT_CMD_MAX_LEN - *pusLength);
        }

        if (ulSuccessFlg == VOS_FALSE)
        {
             AT_WARN_LOG("+CCWA - Unknown class.");
             *pulResult = AT_ERROR;
             return;
        }
    }
    else    /* ״̬Ϊδ���� */
    {
        ucTmp = 0;
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: %d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,ucTmp);
    }

    *pulResult = AT_OK;

}


TAF_VOID At_SsRspInterrogateCcbsCnfProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    VOS_UINT32                          i = 0;

    if (pEvent->OP_GenericServiceInfo == 1)
    {
        if (TAF_SS_PROVISIONED_STATUS_MASK & pEvent->GenericServiceInfo.SsStatus)
        {
            if (pEvent->GenericServiceInfo.OP_CcbsFeatureList == 1)
            {
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Queue of Ccbs requests is: ");
                for (i = 0; i < pEvent->GenericServiceInfo.CcbsFeatureList.ucCnt; i++)
                {
                    if (pEvent->GenericServiceInfo.CcbsFeatureList.astCcBsFeature[i].OP_CcbsIndex == VOS_TRUE)
                    {
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s",gaucAtCrLf);
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                      "Index:%d",pEvent->GenericServiceInfo.CcbsFeatureList.astCcBsFeature[i].CcbsIndex);
                    }
                }
            }
            else
            {
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Queue of Ccbs is empty");
            }
        }
        else
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CCBS not provisioned");
        }
    }
    else if (pEvent->OP_SsStatus == 1)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CCBS not provisioned");
    }
    else
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Unknown");
    }
    *pulResult = AT_OK;

}


TAF_VOID At_SsRspInterrogateCnfCmmiProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    if(pEvent->OP_Error == 1)       /* ��Ҫ�����жϴ����� */
    {
        *pulResult = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);       /* �������� */
        return;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CMMI_QUERY_CLIP)
    {
        if ((pEvent->OP_SsStatus == 1) &&
            (TAF_SS_PROVISIONED_STATUS_MASK & pEvent->SsStatus))
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIP provisioned");
        }
        else if (pEvent->OP_SsStatus == 0)
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Unknown");
        }
        else
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIP not provisioned");
        }

        *pulResult = AT_OK;
    }
    else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CMMI_QUERY_CLIR)
    {
        if (pEvent->OP_GenericServiceInfo == 1)
        {
            if (TAF_SS_PROVISIONED_STATUS_MASK & pEvent->GenericServiceInfo.SsStatus)
            {
                if (pEvent->GenericServiceInfo.OP_CliStrictOp == 1)
                {
                    switch (pEvent->GenericServiceInfo.CliRestrictionOp)
                    {
                    case TAF_SS_CLI_PERMANENT:
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIR provisioned in permanent mode");
                        break;

                    case TAF_SS_CLI_TMP_DEFAULT_RESTRICTED:
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIR temporary mode presentation restricted");
                        break;

                    case TAF_SS_CLI_TMP_DEFAULT_ALLOWED:
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIR temporary mode presentation allowed");
                        break;

                    default:
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Unknown");
                        break;
                    }
                }
                else
                {
                    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Unknown");
                }
            }
            else
            {
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIR not provisioned");
            }
        }
        else if (pEvent->OP_SsStatus == 1)
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"CLIR not provisioned");
        }
        else
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"Unknown");
        }
        *pulResult = AT_OK;
    }
    else if ( gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_SS_INTERROGATE_CCBS)
    {
        At_SsRspInterrogateCcbsCnfProc(ucIndex,pEvent,pulResult,pusLength);
    }
    else
    {
        *pulResult = AT_ERROR;
    }

}


TAF_VOID At_SsRspInterrogateCnfProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT32                          *pulResult,
    TAF_UINT16                          *pusLength
)
{
    switch(g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex)
    {
    case AT_CMD_CLIP:
        At_SsRspInterrogateCnfClipProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    case AT_CMD_COLP:
        At_SsRspInterrogateCnfColpProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    case AT_CMD_CLIR:
        At_SsRspInterrogateCnfClirProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    case AT_CMD_CLCK:
        At_SsRspInterrogateCnfClckProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    case AT_CMD_CCWA:
        At_SsRspInterrogateCnfCcwaProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    case AT_CMD_CCFC:
        /* +CCFC: <status>,<class1>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]] */
        if(pEvent->OP_Error == 1)       /* ��Ҫ�����жϴ����� */
        {
            *pulResult = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);       /* �������� */
            break;
        }

        *pusLength = (TAF_UINT16)At_CcfcQryReport(pEvent,ucIndex);
        *pulResult = AT_OK;
        break;

    case AT_CMD_CMMI:
        At_SsRspInterrogateCnfCmmiProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    case AT_CMD_CNAP:
        AT_SsRspInterrogateCnfCnapProc(ucIndex, pEvent, pulResult, pusLength);
        break;

    default:
        break;
    }
}



TAF_VOID At_SsRspUssdProc(
    TAF_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent,
    TAF_UINT16                          *pusLength
)
{
    TAF_UINT8                           ucTmp    = 0;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    pucSystemAppConfig = AT_GetSystemAppConfigAddr();

    /* <m> */
    if(pEvent->SsEvent == TAF_SS_EVT_USS_NOTIFY_IND)
    {
        ucTmp = 0;
    }
    else if(pEvent->SsEvent == TAF_SS_EVT_USS_REQ_IND)
    {
        ucTmp = 1;
    }
    else
    {

        if (*pucSystemAppConfig == SYSTEM_APP_ANDROID)
        {
            ucTmp = 2;
        }
        else if((pEvent->OP_UssdString == 0) && (pEvent->OP_USSData == 0))
        {
            ucTmp = 2;
        }
        else
        {
            ucTmp = 0;
        }

    }


    /* +CUSD: <m>[,<str>,<dcs>] */
    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"+CUSD: %d",ucTmp);

    /* <str> */
    /* �����ж��Ƶ��������� */
    if (pEvent->OP_Error == 0)
    {
        *pusLength = AT_PrintUssdStr(pEvent, ucIndex, *pusLength);
    }
}


VOS_UINT32 AT_GetSsEventErrorCode(
    VOS_UINT8                           ucIndex,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pEvent)
{
    if (pEvent->OP_SsStatus == VOS_TRUE)
    {
        if ( (TAF_SS_PROVISIONED_STATUS_MASK & pEvent->SsStatus) == 0 )
        {
            /* ����ҵ��δǩԼ��Ӧ�Ĵ����� */
            return AT_CME_SERVICE_NOT_PROVISIONED;
        }
    }

    return At_ChgTafErrorCode(ucIndex, pEvent->ErrorCode);
}


TAF_VOID At_SsRspProc(TAF_UINT8  ucIndex,TAF_SS_CALL_INDEPENDENT_EVENT_STRU  *pEvent)
{
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;

    /* CLIP CCWA CCFC CLCK CUSD CPWD */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CUSD_REQ )
    {
        (VOS_VOID)At_SsRspCusdProc(ucIndex, pEvent);
        return;
    }

    if(pEvent->SsEvent == TAF_SS_EVT_ERROR) /* �����ERROR�¼�����ֱ���жϴ����� */
    {
        if (pEvent->ErrorCode == TAF_ERR_USSD_NET_TIMEOUT)
        {

            usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               "%s+CUSD: %d%s",
                                               gaucAtCrLf,
                                               AT_CUSD_M_NETWORK_TIMEOUT,
                                               gaucAtCrLf);

            At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

            return;
        }

        if (pEvent->ErrorCode == TAF_ERR_USSD_USER_TIMEOUT)
        {

            usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               "%s+CUSD: %d%s",
                                               gaucAtCrLf,
                                               AT_CUSD_M_NETWORK_CANCEL,
                                               gaucAtCrLf);

            At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

            return;
        }

        if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CURRENT_OPT_BUTT )
        {
            return;
        }

        /* ��SS Event�л�ȡ����AT���صĴ����� */
        ulResult = AT_GetSsEventErrorCode(ucIndex, pEvent);

        AT_STOP_TIMER_CMD_READY(ucIndex);
    }
    else
    {
        switch(pEvent->SsEvent)             /* �����¼� */
        {
        case TAF_SS_EVT_INTERROGATESS_CNF:          /* ��ѯ����ϱ� */
            At_SsRspInterrogateCnfProc(ucIndex, pEvent, &ulResult, &usLength);
            AT_STOP_TIMER_CMD_READY(ucIndex);
            break;

        case TAF_SS_EVT_ERASESS_CNF:
        case TAF_SS_EVT_REGISTERSS_CNF:
        case TAF_SS_EVT_ACTIVATESS_CNF:
        case TAF_SS_EVT_DEACTIVATESS_CNF:
        case TAF_SS_EVT_REG_PASSWORD_CNF:
        case TAF_SS_EVT_ERASE_CC_ENTRY_CNF:
            if(pEvent->OP_Error == 0)
            {
                ulResult = AT_OK;
            }
            else
            {
                ulResult = At_ChgTafErrorCode(ucIndex,pEvent->ErrorCode);       /* �������� */
            }
            AT_STOP_TIMER_CMD_READY(ucIndex);
            break;

        case TAF_SS_EVT_USS_NOTIFY_IND:                     /* ֪ͨ�û����ý�һ������ */
        case TAF_SS_EVT_USS_REQ_IND:                        /* ֪ͨ�û���һ������ */
        case TAF_SS_EVT_PROCESS_USS_REQ_CNF:                /* ֪ͨ�û������ͷ� */
        case TAF_SS_EVT_USS_RELEASE_COMPLETE_IND:           /* ֪ͨ�û������ͷ� */
            At_SsRspUssdProc(ucIndex, pEvent, &usLength);
            break;

        /* Delete TAF_SS_EVT_GET_PASSWORD_IND��֧ */
        default:
            return;
        }
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);
}

TAF_VOID At_SsMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    errno_t                             lMemResult;
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pEvent = TAF_NULL_PTR;
    TAF_UINT8 ucIndex = 0;
    /*lint -save -e830 */
    pEvent = (TAF_SS_CALL_INDEPENDENT_EVENT_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_SS_CALL_INDEPENDENT_EVENT_STRU));
    /*lint -restore */
    if (pEvent == TAF_NULL_PTR)
    {
        AT_WARN_LOG("At_SsMsgProc Mem Alloc FAILURE");
        return;
    }

    if (usLen > sizeof(TAF_SS_CALL_INDEPENDENT_EVENT_STRU))
    {
        AT_WARN_LOG1("At_SsMsgProc: Invalid Para usLen: ", usLen);
        usLen = sizeof(TAF_SS_CALL_INDEPENDENT_EVENT_STRU);
    }

    lMemResult = memcpy_s(pEvent, sizeof(TAF_SS_CALL_INDEPENDENT_EVENT_STRU), pData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_SS_CALL_INDEPENDENT_EVENT_STRU), usLen);

    AT_LOG1("At_SsMsgProc pEvent->ClientId",pEvent->ClientId);
    AT_LOG1("At_SsMsgProc pEvent->SsEvent",pEvent->SsEvent);
    AT_LOG1("At_SsMsgProc pEvent->OP_Error",pEvent->OP_Error);
    AT_LOG1("At_SsMsgProc pEvent->ErrorCode",pEvent->ErrorCode);
    AT_LOG1("At_SsMsgProc pEvent->SsCode",pEvent->SsCode);
    AT_LOG1("At_SsMsgProc pEvent->Cause",pEvent->Cause);

    if(At_ClientIdToUserId(pEvent->ClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_SsMsgProc At_ClientIdToUserId FAILURE");
        /*lint -save -e830 */
        PS_MEM_FREE(WUEPS_PID_AT, pEvent);
        /*lint -restore */
        return;
    }

    if(AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        At_SsIndProc(ucIndex, pEvent);
    }
    else
    {
        AT_LOG1("At_SsMsgProc ucIndex",ucIndex);
        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

        At_SsRspProc(ucIndex,pEvent);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pEvent);

}


TAF_UINT32 At_PhReadCreg(TAF_PH_REG_STATE_STRU  *pPara,TAF_UINT8 *pDst)
{
    TAF_UINT16 usLength = 0;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if ((pPara->ucAct == TAF_PH_ACCESS_TECH_CDMA_1X)
        ||(pPara->ucAct == TAF_PH_ACCESS_TECH_EVDO)
        )
    {
        /* lac */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"FFFF\"");
        /* ci */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"FFFFFFFF\"");

        if((g_usReportCregActParaFlg == VOS_TRUE) && (pPara->ucAct < TAF_PH_ACCESS_TECH_BUTT))
        {
            /* act */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",%d",pPara->ucAct);
        }

        return usLength;
    }
#endif

    if(pPara->CellId.ucCellNum > 0)
    {
        /* lac */
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (pPara->ucAct == TAF_PH_ACCESS_TECH_NR_5GC)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"%X%X%X%X%X%X\"",
                    0x000f & (pPara->ulLac >> 20),
                    0x000f & (pPara->ulLac >> 16),
                    0x000f & (pPara->ulLac >> 12),
                    0x000f & (pPara->ulLac >> 8),
                    0x000f & (pPara->ulLac >> 4),
                    0x000f & (pPara->ulLac >> 0));
        }
        else
#endif
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"%X%X%X%X\"",
                    0x000f & (pPara->ulLac >> 12),
                    0x000f & (pPara->ulLac >> 8),
                    0x000f & (pPara->ulLac >> 4),
                    0x000f & (pPara->ulLac >> 0));
        }

        /* ci */
        if (gucCiRptByte == CREG_CGREG_CI_RPT_FOUR_BYTE)
        {
            /* VDF����: CREG/CGREG��<CI>����4�ֽڷ�ʽ�ϱ� */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"%08X\"",
                    (pPara->CellId.astCellId[0].ulCellIdLowBit));
        }
        else
        {
            /* <CI>����2�ֽڷ�ʽ�ϱ� */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"%X%X%X%X\"",
                    0x000f & (pPara->CellId.astCellId[0].ulCellIdLowBit >> 12),
                    0x000f & (pPara->CellId.astCellId[0].ulCellIdLowBit >> 8),
                    0x000f & (pPara->CellId.astCellId[0].ulCellIdLowBit >> 4),
                    0x000f & (pPara->CellId.astCellId[0].ulCellIdLowBit >> 0));
        }

        if((g_usReportCregActParaFlg == VOS_TRUE) && (pPara->ucAct < TAF_PH_ACCESS_TECH_BUTT))
        {
            /* rat */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",%d",pPara->ucAct);
        }
    }

    return usLength;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

TAF_UINT32 At_PhReadC5greg(TAF_PH_REG_STATE_STRU  *pPara,TAF_UINT8 *pDst)
{
    TAF_UINT16 usLength = 0;

    if(pPara->CellId.ucCellNum > 0)
    {
        /* tac */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"%X%X%X%X%X%X\"",
                0x000f & (pPara->ulLac >> 20),
                0x000f & (pPara->ulLac >> 16),
                0x000f & (pPara->ulLac >> 12),
                0x000f & (pPara->ulLac >> 8),
                0x000f & (pPara->ulLac >> 4),
                0x000f & (pPara->ulLac >> 0));

        /* ci */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pDst + usLength,",\"%08X%08X\"",
                pPara->CellId.astCellId[0].ulCellIdHighBit,
                pPara->CellId.astCellId[0].ulCellIdLowBit);

        if (g_usReportCregActParaFlg == VOS_TRUE)
        {
            /* rat */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pDst + usLength, ",%d", pPara->ucAct);
        }
        else
        {
            /* rat */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pDst + usLength, ",");
        }

    }

    return usLength;
}
#endif


VOS_VOID AT_PhSendPinReady( VOS_UINT16 usModemID )
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    for(i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        if (( usModemID     == g_astAtClientCtx[i].stClientConfiguration.enModemId )
         && ( gastAtClientTab[i].UserType == AT_APP_USER ))
        {
            break;
        }
    }

    /* δ�ҵ�E5 User,�����ϱ� */
    if ( i >= AT_MAX_CLIENT_NUM )
    {
        return ;
    }

    usLength = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "^CPINNTY:READY");
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);

    At_SendResultData((VOS_UINT8)i,pgucAtSndCodeAddr,usLength);
}


VOS_VOID AT_PhSendNeedPuk( VOS_UINT16 usModemID )
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;

    for(i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        if (( usModemID     == g_astAtClientCtx[i].stClientConfiguration.enModemId )
         && ( gastAtClientTab[i].UserType == AT_APP_USER) )
        {
            break;
        }
    }

    /* δ�ҵ�E5 User,�����ϱ� */
    if ( i >= AT_MAX_CLIENT_NUM )
    {
        return ;
    }

    usLength = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "^CPINNTY:SIM PUK");

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);

    At_SendResultData((VOS_UINT8)i,pgucAtSndCodeAddr,usLength);
}



VOS_VOID AT_PhSendSimLocked( VOS_VOID )
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          i;

    for(i = 0; i < AT_MAX_CLIENT_NUM; i++)
    {
        if (gastAtClientTab[i].UserType == AT_APP_USER)
        {
            break;
        }
    }

    /* δ�ҵ�E5 User,�����ϱ� */
    if ( i >= AT_MAX_CLIENT_NUM )
    {
        return ;
    }

    usLength = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "^CARDLOCKNTY:SIM LOCKED");

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);

    At_SendResultData((VOS_UINT8)i,pgucAtSndCodeAddr,usLength);
}


VOS_VOID  AT_PhSendRoaming( VOS_UINT8 ucTmpRoamStatus )
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucRoamStatus;

    ucRoamStatus = ucTmpRoamStatus;

    for ( i=0 ; i<AT_MAX_CLIENT_NUM; i++ )
    {
        if (gastAtClientTab[i].UserType == AT_APP_USER)
        {
            break;
        }
    }

    /* δ�ҵ�E5 User,�����ϱ� */
    if ( i >= AT_MAX_CLIENT_NUM )
    {
        return ;
    }

    usLength = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s^APROAMRPT:%d%s",
                                     gaucAtCrLf,
                                     ucRoamStatus,
                                     gaucAtCrLf);

    At_SendResultData((VOS_UINT8)i, pgucAtSndCodeAddr, usLength);

}


VOS_VOID AT_GetOnlyGURatOrder(
    TAF_MMA_MULTIMODE_RAT_CFG_STRU     *pstRatOrder
)
{
    TAF_MMA_MULTIMODE_RAT_CFG_STRU      stRatOrder;
    VOS_UINT32                          i;
    errno_t                             lMemResult;
    VOS_UINT8                           ucIndex;

    ucIndex = 0;
    memset_s(&stRatOrder, (VOS_SIZE_T)sizeof(stRatOrder), 0x00, (VOS_SIZE_T)sizeof(stRatOrder));

    lMemResult = memcpy_s(&stRatOrder, (VOS_SIZE_T)sizeof(stRatOrder), pstRatOrder, (VOS_SIZE_T)sizeof(stRatOrder));
    TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(stRatOrder), (VOS_SIZE_T)sizeof(stRatOrder));

    /* ��ȡGUģ��Ϣ */
    for (i = 0; i < stRatOrder.ucRatNum; i++)
    {
        if ((stRatOrder.aenRatOrder[i] == TAF_MMA_RAT_WCDMA)
         || (stRatOrder.aenRatOrder[i] == TAF_MMA_RAT_GSM))
        {
            pstRatOrder->aenRatOrder[ucIndex] = stRatOrder.aenRatOrder[i];
            ucIndex++;
        }
    }

    pstRatOrder->ucRatNum             = ucIndex;
    pstRatOrder->aenRatOrder[ucIndex] = TAF_MMA_RAT_BUTT;


    return;
}



VOS_VOID AT_ReportSysCfgQryCmdResult(
    TAF_MMA_SYS_CFG_PARA_STRU          *pstSysCfg,
    VOS_UINT8                           ucIndex
)
{
    AT_SYSCFG_RAT_TYPE_ENUM_UINT8       enAccessMode;
    AT_SYSCFG_RAT_PRIO_ENUM_UINT8       enAcqorder;
    VOS_UINT16                          usLength;

    usLength = 0;

    /* �ӵ�ǰ�������ȼ�����ȡGUģ�������ȼ�����Ϣ */
    AT_GetOnlyGURatOrder(&pstSysCfg->stMultiModeRatCfg);

    enAcqorder   = pstSysCfg->enUserPrio;

    /* ���ϱ���TAF_MMA_RAT_ORDER_STRU�ṹת��Ϊmode��acqorder*/

    switch (pstSysCfg->stMultiModeRatCfg.aenRatOrder[0])
    {
        case TAF_MMA_RAT_GSM:
            if (AT_IsSupportWMode(&pstSysCfg->stMultiModeRatCfg) == VOS_TRUE)

            {
                enAccessMode = AT_SYSCFG_RAT_AUTO;
            }
            else
            {
                enAccessMode = AT_SYSCFG_RAT_GSM;
            }
            break;
        case TAF_MMA_RAT_WCDMA:
            if (AT_IsSupportGMode(&pstSysCfg->stMultiModeRatCfg) == VOS_TRUE)
            {
                enAccessMode = AT_SYSCFG_RAT_AUTO;
            }
            else
            {
                enAccessMode = AT_SYSCFG_RAT_WCDMA;
            }
            break;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case TAF_MMA_RAT_1X:
            if (AT_IsSupportHrpdMode(&pstSysCfg->stMultiModeRatCfg) == VOS_TRUE)
            {
                enAccessMode = AT_SYSCFG_RAT_1X_AND_HRPD;
            }
            else
            {
                enAccessMode = AT_SYSCFG_RAT_1X;
            }
            break;

        case TAF_MMA_RAT_HRPD:
            if (AT_IsSupport1XMode(&pstSysCfg->stMultiModeRatCfg) == VOS_TRUE)
            {
                enAccessMode = AT_SYSCFG_RAT_1X_AND_HRPD;
            }
            else
            {
                enAccessMode = AT_SYSCFG_RAT_HRPD;
            }
            break;
#endif

        default:
            /* ֻ֧��L�������syscfg��ѯ�����ϱ�һ�� */
            enAccessMode    = AT_SYSCFG_RAT_AUTO;

            enAcqorder      = AT_SYSCFG_RAT_PRIO_AUTO;
            break;
    }

    /* ��syscfg��ѯ��ʽ�ϱ�^SYSCFG:<mode>,<acqorder>,<band>,<roam>,<srvdomain>*/
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    if ( pstSysCfg->stGuBand.ulBandHigh == 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d,%d,%X,%d,%d",
                                           enAccessMode,
                                           enAcqorder,
                                           pstSysCfg->stGuBand.ulBandLow,
                                           pstSysCfg->enRoam,
                                           pstSysCfg->enSrvDomain);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d,%d,%X%08X,%d,%d",
                                           enAccessMode,
                                           enAcqorder,
                                           pstSysCfg->stGuBand.ulBandHigh,
                                           pstSysCfg->stGuBand.ulBandLow,
                                           pstSysCfg->enRoam,
                                           pstSysCfg->enSrvDomain);
    }
    gstAtSendData.usBufLen = usLength;

    return;
}


LOCAL VOS_VOID AT_ConvertSysCfgStrToAutoModeStr(
    VOS_UINT8                          *pucAcqOrderBegin,
    VOS_UINT8                          *pucAcqOrder,
    VOS_UINT8                           ucRatNum
)
{

#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && (FEATURE_LTE == FEATURE_ON))
    if ((VOS_StrCmp((VOS_CHAR *)pucAcqOrderBegin, "08030201") == 0)
     && (ucRatNum == TAF_PH_MAX_GULNR_RAT_NUM))
    {
        /* ���뼼���ĸ���Ϊ4�ҽ�������˳��ΪNR->L->W->G,acqorder�ϱ�00 */
        pucAcqOrder = pucAcqOrderBegin;
        VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, TAF_MMA_RAT_BUTT * 2 + 1, "00");
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *pucAcqOrder = '\0';
    }
#elif (FEATURE_ON == FEATURE_LTE)
    if ((0 == VOS_StrCmp((VOS_CHAR *)pucAcqOrderBegin, "030201"))
     && (ucRatNum == TAF_PH_MAX_GUL_RAT_NUM))
    {
        /* ���뼼���ĸ���Ϊ3�ҽ�������˳��ΪL->W->G,acqorder�ϱ�00 */
        pucAcqOrder = pucAcqOrderBegin;
        VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, TAF_MMA_RAT_BUTT * 2 + 1, "00");
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *pucAcqOrder = '\0';
    }
#elif (FEATURE_ON == FEATURE_UE_MODE_NR)
    if ((0 == VOS_StrCmp((VOS_CHAR *)pucAcqOrderBegin, "080201"))
     && (ucRatNum == TAF_PH_MAX_GUNR_RAT_NUM))
    {
        /* ���뼼���ĸ���Ϊ3�ҽ�������˳��ΪNR->W->G,acqorder�ϱ�00 */
        pucAcqOrder = pucAcqOrderBegin;
        VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, TAF_MMA_RAT_BUTT * 2 + 1, "00");
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *pucAcqOrder = '\0';
    }
#else
    if ((VOS_StrCmp((VOS_CHAR *)pucAcqOrderBegin, "0201") == 0)
     && (ucRatNum == TAF_PH_MAX_GU_RAT_NUM))
    {
        /* ���뼼���ĸ���Ϊ2�ҽ�������˳��ΪW->G,acqorder�ϱ�00 */
        pucAcqOrder = pucAcqOrderBegin;
        VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, TAF_MMA_RAT_BUTT * 2 + 1, "00");
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *pucAcqOrder = '\0';
    }
#endif

    return;
}



VOS_VOID AT_ConvertSysCfgRatOrderToStr(
    TAF_MMA_MULTIMODE_RAT_CFG_STRU     *pstRatOrder,
    VOS_UINT8                          *pucAcqOrder
)
{
    VOS_UINT8                          i;
    VOS_UINT8                          *pucAcqOrderBegin = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    pucAcqOrderBegin = pucAcqOrder;
    ulLength = TAF_MMA_RAT_BUTT * 2 + 1;

    for (i = 0; i < pstRatOrder->ucRatNum; i++)
    {
        if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_WCDMA)
        {
             VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, ulLength, "02");
             pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
             ulLength -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_GSM)
        {
             VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, ulLength, "01");
             pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
             ulLength -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_LTE)
        {
             VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, ulLength, "03");
             pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
             ulLength -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#endif
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_1X)
        {
             VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, ulLength, "04");
             pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
             ulLength -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_HRPD)
        {
             VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, ulLength, "07");
             pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
             ulLength -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_NR)
        {
             VOS_StrCpy_s((VOS_CHAR *)pucAcqOrder, ulLength, "08");
             pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
             ulLength -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#endif

        else
        {
        }
    }

    *pucAcqOrder = '\0';

    AT_ConvertSysCfgStrToAutoModeStr(pucAcqOrderBegin, pucAcqOrder, pstRatOrder->ucRatNum);

    return;
}


VOS_INT8 AT_GetValidLteBandIndex(
    TAF_USER_SET_LTE_PREF_BAND_INFO_STRU                   *pstLBand
)
{
    VOS_INT8                            cValidIndex;

    if (pstLBand == VOS_NULL_PTR)
    {
        return 0;
    }

    for (cValidIndex = TAF_MMA_LTE_BAND_MAX_LENGTH - 1; cValidIndex >= 0; cValidIndex--)
    {
        if (pstLBand->aulBandInfo[cValidIndex] != 0)
        {
            return cValidIndex;
        }
    }

    return 0;
}



VOS_VOID AT_ReportSysCfgExQryCmdResult(
    TAF_MMA_SYS_CFG_PARA_STRU          *pstSysCfg,
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT8                           aucAcqorder[TAF_MMA_RAT_BUTT * 2 + 1];
    VOS_UINT8                          *pucAcqOrder = VOS_NULL_PTR;
    VOS_INT8                            cIndex;
    VOS_UINT16                          usLength;

    usLength = 0;
    pucAcqOrder = aucAcqorder;

    /* ���ϱ���TAF_MMA_MULTIMODE_RAT_CFG_STRU�ṹת��Ϊacqorder�ַ���*/
    AT_ConvertSysCfgRatOrderToStr(&pstSysCfg->stMultiModeRatCfg, pucAcqOrder);


    /* ��syscfgex��ѯ��ʽ�ϱ�^SYSCFGEX: <acqorder>,<band>,<roam>,<srvdomain>,<lteband> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    if (pstSysCfg->stGuBand.ulBandHigh == 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s\",%X,%d,%d",
                                           pucAcqOrder,
                                           pstSysCfg->stGuBand.ulBandLow,
                                           pstSysCfg->enRoam,
                                           pstSysCfg->enSrvDomain);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"%s\",%X%08X,%d,%d",
                                           pucAcqOrder,
                                           pstSysCfg->stGuBand.ulBandHigh,
                                           pstSysCfg->stGuBand.ulBandLow,
                                           pstSysCfg->enRoam,
                                           pstSysCfg->enSrvDomain);
    }
    cIndex = AT_GetValidLteBandIndex(&(pstSysCfg->stLBand));

    if (cIndex == 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%X",
                                           pstSysCfg->stLBand.aulBandInfo[0]);
    }
    else
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%X",
                                           pstSysCfg->stLBand.aulBandInfo[cIndex]);

        cIndex--;

         while (cIndex >= 0)
         {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%08X",
                                           pstSysCfg->stLBand.aulBandInfo[cIndex]);

            cIndex--;
         }
    }
    gstAtSendData.usBufLen = usLength;

    return;
}


VOS_UINT32 AT_ConvertSysCfgRatOrderToWs46No(
    TAF_MMA_MULTIMODE_RAT_CFG_STRU     *pstRatOrder,
    VOS_UINT32                         *pulProctolNo
)
{
    VOS_UINT8                           ucGsmSupportFlag;
    VOS_UINT8                           ucWcdmaSupportFlag;
    VOS_UINT8                           ucLteSupportFlag;
    VOS_UINT32                          i;
    AT_WS46_RAT_TRANSFORM_TBL_STRU      astWs46RatTransFormTab[] =
    {
      /*     G,         U,         L,     Ws46No */
        {VOS_TRUE,  VOS_FALSE, VOS_FALSE, 12},
        {VOS_FALSE, VOS_TRUE,  VOS_FALSE, 22},
        {VOS_TRUE,  VOS_TRUE,  VOS_TRUE,  25},
        {VOS_FALSE, VOS_FALSE, VOS_TRUE,  28},
        {VOS_TRUE,  VOS_TRUE,  VOS_FALSE, 29},
        {VOS_TRUE,  VOS_FALSE, VOS_TRUE,  30},
        {VOS_FALSE, VOS_TRUE,  VOS_TRUE,  31},
    };

    ucGsmSupportFlag   = VOS_FALSE;
    ucWcdmaSupportFlag = VOS_FALSE;
    ucLteSupportFlag   = VOS_FALSE;

    for (i = 0; i < (VOS_UINT32)pstRatOrder->ucRatNum; i++)
    {
        if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_GSM)
        {
            ucGsmSupportFlag = VOS_TRUE;
        }
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_WCDMA)
        {
            ucWcdmaSupportFlag = VOS_TRUE;
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (pstRatOrder->aenRatOrder[i] == TAF_MMA_RAT_LTE)
        {
            ucLteSupportFlag = VOS_TRUE;
        }
#endif
        else
        {
        }
    }

    for (i = 0; i < sizeof(astWs46RatTransFormTab)/sizeof(astWs46RatTransFormTab[0]); i++)
    {
        if ( (ucGsmSupportFlag == astWs46RatTransFormTab[i].ucGsmSupportFlg)
          && (ucWcdmaSupportFlag == astWs46RatTransFormTab[i].ucWcdmaSupportFlg)
          && (ucLteSupportFlag == astWs46RatTransFormTab[i].ucLteSupportFlg) )
        {
            *pulProctolNo = astWs46RatTransFormTab[i].ucWs46No;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}



VOS_UINT32 AT_ReportWs46QryCmdResult(
    TAF_MMA_SYS_CFG_PARA_STRU          *pstSysCfg,
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulWs46No;
    VOS_UINT16                          usLength;

    ulWs46No = 0;
    usLength = 0;

    /* ���ϱ���TAF_MMA_MULTIMODE_RAT_CFG_STRU�ṹת��Ϊ<n> */
    if (AT_ConvertSysCfgRatOrderToWs46No(&pstSysCfg->stMultiModeRatCfg, &ulWs46No) == VOS_FALSE)
    {
        return AT_ERROR;
    }

    /* ��+WS46��ѯ��ʽ�ϱ�+WS46: <n> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulWs46No);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}





#if(FEATURE_ON == FEATURE_LTE)

VOS_VOID AT_ReportCeregResult(
    VOS_UINT8                           ucIndex,
    TAF_MMA_REG_STATUS_IND_STRU        *pstRegInd,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT32                          ulRst;
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    enModemId = MODEM_ID_0;

    ulRst = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG1("AT_ReportCeregResult:Get ModemID From ClientID fail,ClientID:", ucIndex);
        return;
    }

    /* ��ǰƽ̨�Ƿ�֧��LTE*/
    if (AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_LTE) != VOS_TRUE)
    {
        return;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    if ((pstNetCtx->ucCeregType == AT_CEREG_RESULT_CODE_BREVITE_TYPE)
     && (pstRegInd->stRegStatus.OP_PsRegState == VOS_TRUE))
    {
        /* +CEREG: <stat> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             "%s%s%d%s",
                                             gaucAtCrLf,
                                             gastAtStringTab[AT_STRING_CEREG].pucText,
                                             pstRegInd->stRegStatus.ucPsRegState,
                                             gaucAtCrLf);
    }
    else if ((pstNetCtx->ucCeregType == AT_CEREG_RESULT_CODE_ENTIRE_TYPE)
          && (pstRegInd->stRegStatus.OP_PsRegState == VOS_TRUE))

    {

        if ((pstRegInd->stRegStatus.ucPsRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
         || (pstRegInd->stRegStatus.ucPsRegState == TAF_PH_REG_REGISTERED_ROAM))
        {
             /* +CEREG: <stat>[,<lac>,<ci>,[rat]] */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s%s%d", gaucAtCrLf,
                                                 gastAtStringTab[AT_STRING_CEREG].pucText,
                                                 pstRegInd->stRegStatus.ucPsRegState);

            /* ��������һ�£�GU��ֻ�ϱ�+CGREG: <stat> */
            if (pstRegInd->stRegStatus.ucRatType == TAF_PH_INFO_LTE_RAT)
            {
                *pusLength += (VOS_UINT16)At_PhReadCreg(&(pstRegInd->stRegStatus),pgucAtSndCodeAddr + *pusLength);
            }

            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s", gaucAtCrLf);
        }
        else
        {
            /* +CEREG: <stat> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s%s%d%s", gaucAtCrLf,
                                                 gastAtStringTab[AT_STRING_CEREG].pucText,
                                                 pstRegInd->stRegStatus.ucPsRegState, gaucAtCrLf);
        }
    }
    else
    {

    }

    return;

}
#endif




VOS_VOID AT_ReportCgregResult(
    VOS_UINT8                           ucIndex,
    TAF_MMA_REG_STATUS_IND_STRU        *pstRegInd,
    VOS_UINT16                         *pusLength
)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);


    if ((pstNetCtx->ucCgregType == AT_CGREG_RESULT_CODE_BREVITE_TYPE)
     && (pstRegInd->stRegStatus.OP_PsRegState == VOS_TRUE))
    {
        /* +CGREG: <stat> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             "%s%s%d%s",
                                             gaucAtCrLf,
                                             gastAtStringTab[AT_STRING_CGREG].pucText,
                                             pstRegInd->stRegStatus.ucPsRegState,
                                             gaucAtCrLf);
    }
    else if ((pstNetCtx->ucCgregType == AT_CGREG_RESULT_CODE_ENTIRE_TYPE)
          && (pstRegInd->stRegStatus.OP_PsRegState == VOS_TRUE))
    {

        if (((pstRegInd->stRegStatus.ucPsRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
          || (pstRegInd->stRegStatus.ucPsRegState == TAF_PH_REG_REGISTERED_ROAM))
         && (pstRegInd->stRegStatus.ucRatType != TAF_PH_INFO_NR_5GC_RAT))
        {
            /* +CGREG: <stat>[,<lac>,<ci>,[rat]] */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s%s%d",
                                                 gaucAtCrLf,
                                                 gastAtStringTab[AT_STRING_CGREG].pucText,
                                                 pstRegInd->stRegStatus.ucPsRegState);

            *pusLength += (VOS_UINT16)At_PhReadCreg(&(pstRegInd->stRegStatus),pgucAtSndCodeAddr + *pusLength);

            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s",gaucAtCrLf);
        }
        else
        {
            /* +CGREG: <stat> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                  "%s%s%d%s",
                                                  gaucAtCrLf,
                                                  gastAtStringTab[AT_STRING_CGREG].pucText,
                                                  pstRegInd->stRegStatus.ucPsRegState,
                                                  gaucAtCrLf);
        }
    }
    else
    {

    }

    return;
}


VOS_VOID AT_ReportCregResult(
    VOS_UINT8                           ucIndex,
    TAF_MMA_REG_STATUS_IND_STRU        *pstRegInd,
    VOS_UINT16                         *pusLength
)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);


    if ((pstNetCtx->ucCregType == AT_CREG_RESULT_CODE_BREVITE_TYPE)
     && (pstRegInd->stRegStatus.OP_CsRegState == VOS_TRUE))
    {
        /* +CREG: <stat> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             "%s%s%d%s",
                                             gaucAtCrLf,
                                             gastAtStringTab[AT_STRING_CREG].pucText,
                                             pstRegInd->stRegStatus.RegState,
                                             gaucAtCrLf);
    }
    else if ((pstNetCtx->ucCregType == AT_CREG_RESULT_CODE_ENTIRE_TYPE)
          && (pstRegInd->stRegStatus.OP_CsRegState == VOS_TRUE))
    {
        if ((pstRegInd->stRegStatus.RegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
        || (pstRegInd->stRegStatus.RegState == TAF_PH_REG_REGISTERED_ROAM))
        {
            /* +CREG: <stat>[,<lac>,<ci>,[rat]] */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                  "%s%s%d",
                                                  gaucAtCrLf,
                                                  gastAtStringTab[AT_STRING_CREG].pucText,
                                                  pstRegInd->stRegStatus.RegState);

            *pusLength += (VOS_UINT16)At_PhReadCreg(&(pstRegInd->stRegStatus),
                                                     (pgucAtSndCodeAddr + *pusLength));

            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                  "%s",gaucAtCrLf);
        }
        else
        {
            /* +CREG: <stat> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                              "%s%s%d%s",
                                              gaucAtCrLf,
                                              gastAtStringTab[AT_STRING_CREG].pucText,
                                              pstRegInd->stRegStatus.RegState,
                                              gaucAtCrLf);
        }
    }
    else
    {
    }

    return;
}

#if(FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_VOID AT_ConvertMultiSNssaiToString(
    VOS_UINT8                           ucSNssaiNum,
    PS_S_NSSAI_STRU                    *pstSNssai,
    VOS_CHAR                           *pcStrNssai,
    VOS_UINT32                          ulSrcNssaiLength,
    VOS_UINT32                         *pulDsrLength
)
{
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulLoop;

    /*  27007 rel15, 10.1.1�½�
     *  sst                                     only slice/service type (SST) is present
     *  sst;mapped_sst                          SST and mapped configured SST are present
     *  sst.sd                                  SST and slice differentiator (SD) are present
     *  sst.sd;mapped_sst                       SST, SD and mapped configured SST are present
     *  sst.sd;mapped_sst.mapped_sd             SST, SD, mapped configured SST and mapped configured SD are present
    */

    ulLength = 0;

    for (ulLoop = 0; ulLoop < ucSNssaiNum; ulLoop++)
    {
        if (ulLength >= AT_EVT_MULTI_S_NSSAI_LEN)
        {
            AT_ERR_LOG1("AT_ConvertMultiSNssaiToString:ERROR: ulLength abnormal:", ulLength);
            *pulDsrLength =  0;

            return;
        }

        /* ����ж��S-NSSAI��ÿ��S-NSSAIͨ��":"�ָ� */
        if (ulLoop != 0)
        {
            ulLength += (VOS_UINT32)VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, ":");
        }

        if ((pstSNssai[ulLoop].bitOpSd == VOS_TRUE)
         && (pstSNssai[ulLoop].bitOpMappedSst == VOS_TRUE)
         && (pstSNssai[ulLoop].bitOpMappedSd == VOS_TRUE))
        {
            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x.%x;%x.%x",
                                      pstSNssai[ulLoop].ucSst,
                                      pstSNssai[ulLoop].ulSd,
                                      pstSNssai[ulLoop].ucMappedSst,
                                      pstSNssai[ulLoop].ulMappedSd);
        }
        else if ((pstSNssai[ulLoop].bitOpSd == VOS_TRUE)
              && (pstSNssai[ulLoop].bitOpMappedSst == VOS_TRUE))
        {
            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x.%x;%x",
                                      pstSNssai[ulLoop].ucSst,
                                      pstSNssai[ulLoop].ulSd,
                                      pstSNssai[ulLoop].ucMappedSst);
        }
        else if (pstSNssai[ulLoop].bitOpSd == VOS_TRUE)
        {
            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x.%x",
                                      pstSNssai[ulLoop].ucSst,
                                      pstSNssai[ulLoop].ulSd);
        }
        else if (pstSNssai[ulLoop].bitOpMappedSst == VOS_TRUE)
        {
            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x;%x",
                                      pstSNssai[ulLoop].ucSst,
                                      pstSNssai[ulLoop].ucMappedSst);
        }
        else
        {

            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x",
                                      pstSNssai[ulLoop].ucSst);
        }

    }

    *pulDsrLength =  ulLength;

    return;
}


VOS_VOID AT_ConvertMultiRejectSNssaiToString(
    VOS_UINT8                           ucSNssaiNum,
    PS_REJECTED_S_NSSAI_STRU           *pstSNssai,
    VOS_CHAR                           *pcStrNssai,
    VOS_UINT32                          ulSrcNssaiLength,
    VOS_UINT32                         *pulDsrLength
)
{
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulLoop;

    /*  27007 rel15, 10.1.1�½�
     *  sst                                     only slice/service type (SST) is present
     *  sst.sd                                  SST and slice differentiator (SD) are present
    */

    ulLength = 0;

    for (ulLoop = 0; ulLoop < ucSNssaiNum; ulLoop++)
    {
        if (ulLength >= AT_EVT_MULTI_S_NSSAI_LEN)
        {
            AT_ERR_LOG1("AT_ConvertMultiRejectSNssaiToString :ERROR: ulLength abnormal:", ulLength);
            *pulDsrLength =  0;

            return;
        }

        /* ����ж��S-NSSAI��ÿ��S-NSSAIͨ��":"�ָ� */
        if (ulLoop != 0)
        {
            ulLength += (VOS_UINT32)VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, ":");
        }

        if (pstSNssai[ulLoop].bitOpSd == VOS_TRUE)
        {
            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x.%x#%x",
                                      pstSNssai[ulLoop].ucSst,
                                      pstSNssai[ulLoop].ulSd,
                                      pstSNssai[ulLoop].enCause);
        }
        else
        {

            ulLength += VOS_sprintf_s(pcStrNssai + ulLength, ulSrcNssaiLength - ulLength, "%x#%x",
                                      pstSNssai[ulLoop].ucSst,
                                      pstSNssai[ulLoop].enCause);
        }

    }

    *pulDsrLength =  ulLength;

    return;
}



LOCAL VOS_VOID AT_ReportC5gregResult(
    VOS_UINT8                           ucIndex,
    TAF_MMA_REG_STATUS_IND_STRU        *pstRegInd,
    VOS_UINT16                         *pusLength
)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulRst;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulLength;
    VOS_CHAR                            acStrAllowedNssai[AT_EVT_MULTI_S_NSSAI_LEN];


    enModemId = MODEM_ID_0;

    ulRst = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG1("AT_ReportC5gregResult: ERROR: Get ModemID From ClientID fail,ClientID:", ucIndex);
        return;
    }

    /* ��ǰƽ̨�Ƿ�֧��NR */
    if (AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_NR) != VOS_TRUE)
    {
        return;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    if ((pstNetCtx->ucC5gregType == AT_C5GREG_RESULT_CODE_BREVITE_TYPE)
     && (pstRegInd->stRegStatus.OP_PsRegState == VOS_TRUE))
    {
        /* +C5GREG: <stat> */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                             "%s%s%d%s",
                                             gaucAtCrLf,
                                             gastAtStringTab[AT_STRING_C5GREG].pucText,
                                             pstRegInd->stRegStatus.ucPsRegState,
                                             gaucAtCrLf);
    }
    else if ((pstNetCtx->ucC5gregType == AT_C5GREG_RESULT_CODE_ENTIRE_TYPE)
          && (pstRegInd->stRegStatus.OP_PsRegState == VOS_TRUE))
    {

        if ((pstRegInd->stRegStatus.ucPsRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
         || (pstRegInd->stRegStatus.ucPsRegState == TAF_PH_REG_REGISTERED_ROAM))
        {
             /* +C5GREG: <stat>[,<lac>,<ci>,[rat],[<Allowed_NSSAI_length>],<Allowed_NSSAI>][,<cause_type>,<reject_cause>]]   */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s%s%d", gaucAtCrLf,
                                                 gastAtStringTab[AT_STRING_C5GREG].pucText,
                                                 pstRegInd->stRegStatus.ucPsRegState);

            if (pstRegInd->stRegStatus.ucRatType == TAF_PH_INFO_NR_5GC_RAT)
            {
                *pusLength += (VOS_UINT16)At_PhReadC5greg(&(pstRegInd->stRegStatus),pgucAtSndCodeAddr + *pusLength);

                *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                    ",");

                /* ��װ��Ƭ��Ϣ��ʽ */
                ulLength = 0;
                memset_s(acStrAllowedNssai, sizeof(acStrAllowedNssai), 0, sizeof(acStrAllowedNssai));

                AT_ConvertMultiSNssaiToString(AT_MIN(pstRegInd->stRegStatus.stAllowedNssai.ucSNssaiNum, PS_MAX_ALLOWED_S_NSSAI_NUM),
                                              &pstRegInd->stRegStatus.stAllowedNssai.astSNssai[0],
                                              acStrAllowedNssai,
                                              sizeof(acStrAllowedNssai),
                                              &ulLength);

                *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                    "%d,",
                                                    ulLength);

                if (ulLength != 0)
                {
                     *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                         "\"%s\"",
                                                         acStrAllowedNssai);
                }

            }

            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s", gaucAtCrLf);
        }
        else
        {
            /* +C5GREG: <stat> */
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + *pusLength,
                                                 "%s%s%d%s", gaucAtCrLf,
                                                 gastAtStringTab[AT_STRING_C5GREG].pucText,
                                                 pstRegInd->stRegStatus.ucPsRegState, gaucAtCrLf);
        }
    }
    else
    {

    }

    return;

}
#endif



VOS_VOID AT_ProcRegStatusInfoInd(
    VOS_UINT8                           ucIndex,
    TAF_MMA_REG_STATUS_IND_STRU        *pstRegInfo
)
{
    VOS_UINT16                          usLength;

    usLength  = 0;

    AT_ReportCregResult(ucIndex, pstRegInfo, &usLength);

    AT_ReportCgregResult(ucIndex, pstRegInfo, &usLength);

#if(FEATURE_ON == FEATURE_LTE)
    /* ͨ��NV�жϵ�ǰ�Ƿ�֧��LTE */
    AT_ReportCeregResult(ucIndex, pstRegInfo, &usLength);
#endif

#if(FEATURE_ON == FEATURE_UE_MODE_NR)
    /* ͨ��NV�жϵ�ǰ�Ƿ�֧��NR */
    AT_ReportC5gregResult(ucIndex, pstRegInfo, &usLength);
#endif


    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}



VOS_VOID AT_ProcUsimInfoInd(
    VOS_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU          *pstEvent
)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_ProcUsimInfoInd: Get modem id fail.");
        return;
    }



    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^SIMST:%d,%d%s",
                                       gaucAtCrLf,
                                       pstEvent->SimStatus,
                                       pstEvent->MeLockStatus,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID At_RcvMmaPsInitResultIndProc(
    TAF_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU          *pEvent
)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength       = 0;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_RcvMmaPsInitResultIndProc: Get modem id fail.");
        return;
    }

    if (pEvent->OP_PsInitRslt == VOS_FALSE)
    {
        AT_ERR_LOG("At_RcvMmaPsInitResultIndProc: invalid msg.");
        return;
    }

    /* ֻ��modem��ʼ���ɹ��ŵ��õ���ӿڲ��� */
    if (pEvent->ulPsInitRslt == TAF_MMA_PS_INIT_SUCC)
    {
        /* ��GPIO�ܽ�֪ͨAP��MODEM�Ѿ�OK */
        DRV_OS_STATUS_SWITCH(VOS_TRUE);

        /* �յ�PS INIT�ϱ���д�豸�ڵ㣬�����ɹ� */
        mdrv_set_modem_state(VOS_TRUE);

        DMS_SetModemStatus(enModemId);
    }
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^PSINIT: %d%s",
                                       gaucAtCrLf,
                                       pEvent->ulPsInitRslt,
                                       gaucAtCrLf);

    At_SendResultData((VOS_UINT8)ucIndex, pgucAtSndCodeAddr, usLength);
}



/*****************************************************************************
 �� �� ��  : AT_RcvMmaUsimMatchFilesInd
 ��������  : ����TAF_PH_EVT_USIM_MATCH_FILES_IND�¼�
 �������  : VOS_UINT8                           ucIndex
             TAF_PHONE_EVENT_INFO_STRU          *pstEvent
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

*****************************************************************************/
VOS_VOID AT_RcvMmaUsimMatchFilesInd(
    VOS_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU          *pstEvent
)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvMmaUsimMatchFilesInd: Get modem id fail.");
        return;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^MATCHFILE: %d,%d,%d,",
                                       gaucAtCrLf,
                                       pstEvent->stUsimMatchFile.ucFileType,
                                       pstEvent->stUsimMatchFile.usTotalLen,
                                       pstEvent->stUsimMatchFile.usCurLen);

    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (TAF_INT8 *)pgucAtSndCodeAddr,
                                                    (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                     pstEvent->stUsimMatchFile.aucContent,
                                                     pstEvent->stUsimMatchFile.usCurLen);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    return;
}


VOS_VOID AT_ProcReportSimSqInfo(
    VOS_UINT8                           ucIndex,
    TAF_MMA_SIMSQ_STATE_ENUM_UINT32     enSimsq
)
{
    VOS_UINT8                           ucSimsqEnable;
    VOS_UINT16                          usLength;

    ucSimsqEnable                       = At_GetSimsqEnable();

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s^SIMSQ: %d,%d%s",
                                       gaucAtCrLf,
                                       ucSimsqEnable,
                                       enSimsq,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    return;
}


VOS_VOID AT_ProcSimsqInd(
    VOS_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU          *pstEvent)
{
    VOS_UINT8                           ucSimsqEnable;
    MODEM_ID_ENUM_UINT16                enModemId;

    /* �����ò������浽CC�������� */
    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_LOG1("AT_ProcSimsqInd AT_GetModemIdFromClient fail", ucIndex);
        return;
    }

    ucSimsqEnable = At_GetSimsqEnable();

    At_SetSimsqStatus(enModemId,pstEvent->SimStatus);

    if (ucSimsqEnable == VOS_FALSE)
    {
        return;
    }

    AT_ProcReportSimSqInfo(ucIndex, At_GetSimsqStatus(enModemId));

    return;
}


TAF_VOID At_PhIndProc(TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent)
{
    switch(pEvent->PhoneEvent)
    {

        case TAF_PH_EVT_USIM_INFO_IND:
            AT_ProcUsimInfoInd(ucIndex, pEvent);
            return;

        case TAF_PH_EVT_SIMSQ_IND:
            AT_ProcSimsqInd(ucIndex, pEvent);
            return;

        case TAF_MMA_EVT_PS_INIT_RESULT_IND:
            At_RcvMmaPsInitResultIndProc(ucIndex, pEvent);
            return;


        case TAF_PH_EVT_OPER_MODE_IND:
            AT_NORM_LOG("At_PhIndProc TAF_PH_EVT_OPER_MODE_IND Do nothing");
            return;



        case MN_PH_EVT_SIMLOCKED_IND:
            AT_PhSendSimLocked();
            break;

        case MN_PH_EVT_ROAMING_IND:
            AT_PhSendRoaming( pEvent->ucRoamStatus );
            break;


        case TAF_PH_EVT_NSM_STATUS_IND:
            AT_RcvMmaNsmStatusInd(ucIndex, pEvent);
            break;

        case TAF_PH_EVT_USIM_MATCH_FILES_IND:
            AT_RcvMmaUsimMatchFilesInd(ucIndex , pEvent);
            break;
        default:
            AT_WARN_LOG("At_PhIndProc Other PhoneEvent");
            return;
    }
}



/* AT_PhnEvtPlmnList */


VOS_VOID AT_PhnEvtSetMtPowerDown(
    VOS_UINT8 ucIndex,
    TAF_PHONE_EVENT_INFO_STRU  *pEvent
)
{
    VOS_UINT32       ulResult = AT_ERROR;

    gstAtSendData.usBufLen = 0;

    /*��ǰ���ڵȴ��첽��Ϣ������TAF_MSG_MMA_MT_POWER_DOWN*/
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_MMA_MT_POWER_DOWN)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        if (pEvent->OP_PhoneError == TAF_ERR_NO_ERROR)
        {
            ulResult = AT_OK;
        }
    }

    At_FormatResultData(ucIndex,ulResult);
}


VOS_VOID    At_QryCpinRspProc(
    VOS_UINT8       ucIndex,
    TAF_PH_PIN_TYPE ucPinType,
    VOS_UINT16     *pusLength
)
{
    if(ucPinType == TAF_SIM_PIN)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"SIM PIN");
    }
    else if(ucPinType == TAF_SIM_PUK)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"SIM PUK");
    }
    else if(ucPinType == TAF_PHNET_PIN)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-NET PIN");
    }
    else if(ucPinType == TAF_PHNET_PUK)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-NET PUK");
    }
    else if(ucPinType == TAF_PHNETSUB_PIN)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-NETSUB PIN");
    }
    else if(ucPinType == TAF_PHNETSUB_PUK)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-NETSUB PUK");
    }
    else if(ucPinType == TAF_PHSP_PIN)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-SP PIN");
    }
    else if(ucPinType == TAF_PHSP_PUK)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-SP PUK");
    }
    else if(ucPinType == TAF_PHCP_PIN)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-CP PIN");
    }
    else if(ucPinType == TAF_PHCP_PUK)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"PH-CP PUK");
    }
    else
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"READY");
    }

    return;
}

#if(FEATURE_ON == FEATURE_LTE)

TAF_UINT32 AT_ProcOperModeWhenLteOn(VOS_UINT8 ucIndex)
{
#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
    return atSetTmodePara(ucIndex, g_stAtDevCmdCtrl.ucCurrentTMode);
#else
    return atSetTmodePara(ucIndex, g_stMtInfoCtx.enCurrentTMode);
#endif

}
#endif


TAF_UINT32 At_ProcPinQuery(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                         ulResult;

    ulResult = AT_OK;

    /* AT+CLCK */
    if (g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex == AT_CMD_CLCK)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        if (pEvent->PinCnf.QueryResult.UsimmEnableFlg == TAF_PH_USIMM_ENABLE)
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "1");
        }
        else
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "0");
        }
    }

    /* AT^CPIN */
    else if (g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex == AT_CMD_CPIN_2)
    {
        if (pEvent->PinCnf.PinType == TAF_SIM_PIN)
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "SIM PIN,%d,%d,%d,%d,%d",\
                                            pEvent->PinCnf.RemainTime.ucPin1RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPuk1RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPin1RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPuk2RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPin2RemainTime);
        }
        else if (pEvent->PinCnf.PinType == TAF_SIM_PUK)
        {

            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "SIM PUK,%d,%d,%d,%d,%d",\
                                                pEvent->PinCnf.RemainTime.ucPuk1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPuk1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPin1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPuk2RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPin2RemainTime);
        }
        else
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "READY,,%d,%d,%d,%d",\
                                                pEvent->PinCnf.RemainTime.ucPuk1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPin1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPuk2RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPin2RemainTime);

        }
    }
    /*AT+CPIN*/
    else
    {
        At_QryCpinRspProc(ucIndex, pEvent->PinCnf.PinType, pusLength);
    }

    return ulResult;
}

TAF_UINT32 At_ProcPin2Query(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                         ulResult;

    ulResult = AT_FAILURE;

    /* AT^CPIN2 */
    if (g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex == AT_CMD_CPIN2)
    {

        if (pEvent->PinCnf.PinType == TAF_SIM_PIN2)
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "SIM PIN2,%d,%d,%d,%d,%d",\
                                            pEvent->PinCnf.RemainTime.ucPin2RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPuk1RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPin1RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPuk2RemainTime,\
                                            pEvent->PinCnf.RemainTime.ucPin2RemainTime);
        }
        else if (pEvent->PinCnf.PinType == TAF_SIM_PUK2)
        {
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "SIM PUK2,%d,%d,%d,%d,%d",\
                                                pEvent->PinCnf.RemainTime.ucPuk2RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPuk1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPin1RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPuk2RemainTime,\
                                                pEvent->PinCnf.RemainTime.ucPin2RemainTime);
        }
        else
        {
            ulResult = AT_CME_SIM_FAILURE;

            return ulResult;
        }
    }
    else
    {
        ulResult = AT_ERROR;

        return ulResult;
    }

    ulResult = AT_OK;

    return ulResult;

}

TAF_UINT32 At_ProcPinResultPinOk(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    VOS_BOOL                           *pbNeedRptPinReady,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                         ulResult;

    ulResult = AT_FAILURE;

    switch (pEvent->PinCnf.CmdType)
    {
    case TAF_PIN_QUERY:

        ulResult = At_ProcPinQuery(pEvent, pusLength, ucIndex);

        break;

    case TAF_PIN2_QUERY:

        ulResult = At_ProcPin2Query(pEvent, pusLength, ucIndex);

        break;

    case TAF_PIN_VERIFY:
    case TAF_PIN_UNBLOCK:
        if (pEvent->PinCnf.PinType == TAF_SIM_NON)
        {
            ulResult = AT_ERROR;
        }
        else
        {
            ulResult = AT_OK;

            *pbNeedRptPinReady = VOS_TRUE;
        }

        break;

    case TAF_PIN_CHANGE:
    case TAF_PIN_DISABLE:
    case TAF_PIN_ENABLE:

        ulResult = AT_OK;

        break;

    default:

        return AT_RRETURN_CODE_BUTT;
    }

    return ulResult;
}

TAF_UINT32 At_ProcPinResultNotPinOk(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    VOS_BOOL                           *pbNeedRptNeedPuk
)
{
    TAF_UINT32                         ulResult;

    ulResult = AT_FAILURE;

    switch(pEvent->PinCnf.OpPinResult)
    {
     case TAF_PH_OP_PIN_NEED_PIN1:
         ulResult = AT_CME_SIM_PIN_REQUIRED;
         break;

     case TAF_PH_OP_PIN_NEED_PUK1:
         *pbNeedRptNeedPuk = VOS_TRUE;
         ulResult = AT_CME_SIM_PUK_REQUIRED;
         break;

     case TAF_PH_OP_PIN_NEED_PIN2:
         ulResult = AT_CME_SIM_PIN2_REQUIRED;
         break;

     case TAF_PH_OP_PIN_NEED_PUK2:
         ulResult = AT_CME_SIM_PUK2_REQUIRED;
         break;

     case TAF_PH_OP_PIN_INCORRECT_PASSWORD:
         ulResult = AT_CME_INCORRECT_PASSWORD;
         break;

     case TAF_PH_OP_PIN_OPERATION_NOT_ALLOW:
         ulResult = AT_CME_OPERATION_NOT_ALLOWED;
         break;

     case TAF_PH_OP_PIN_SIM_FAIL:
         ulResult = AT_CME_SIM_FAILURE;
         break;

     default:
         ulResult = AT_CME_UNKNOWN;
         break;
    }

    return ulResult;
}

TAF_UINT32 At_ProcPhoneEvtOperPinCnf(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    MODEM_ID_ENUM_UINT16                enModemId,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT16                         usLength;
    TAF_UINT32                         ulResult;
    VOS_BOOL                           bNeedRptPinReady;
    VOS_BOOL                           bNeedRptNeedPuk;

    ulResult = AT_FAILURE;
    usLength = 0;
    bNeedRptPinReady = VOS_FALSE;
    bNeedRptNeedPuk = VOS_FALSE;

    if (pEvent->OP_PhoneError == 1)  /* MT���ش��� */
    {
        ulResult = At_ChgTafErrorCode(ucIndex,pEvent->PhoneError);       /* �������� */

        if ((ulResult == AT_CME_SIM_PUK_REQUIRED)
         && (pEvent->PinCnf.CmdType == TAF_PIN_VERIFY))
        {
            bNeedRptNeedPuk = VOS_TRUE;
        }
    }
    else
    {
        if (pEvent->PinCnf.OpPinResult == TAF_PH_OP_PIN_OK)
        {
            ulResult = At_ProcPinResultPinOk(pEvent, &usLength, &bNeedRptPinReady, ucIndex);

            if (ulResult == AT_RRETURN_CODE_BUTT)
            {
                AT_NORM_LOG("At_ProcPhoneEvtOperPinCnf: return AT_RRETURN_CODE_BUTT");

                return AT_RRETURN_CODE_BUTT;
            }
        }
        else
        {
            ulResult = At_ProcPinResultNotPinOk(pEvent, &bNeedRptNeedPuk);
        }
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);

    if (bNeedRptPinReady == VOS_TRUE)
    {
        AT_PhSendPinReady(enModemId);
    }

    if (bNeedRptNeedPuk == VOS_TRUE)
    {
        AT_PhSendNeedPuk(enModemId);
    }

    return AT_RRETURN_CODE_BUTT;
}

TAF_UINT32 At_ProcPhoneEvtOperModeCnf(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    MODEM_ID_ENUM_UINT16                enModemId,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                          ulResult;
#if(FEATURE_ON == FEATURE_LTE)
    VOS_UINT8                           ucSptLteFlag;
    VOS_UINT8                           ucSptUtralTDDFlag;
#endif

    ulResult = AT_FAILURE;

    if (pEvent->OP_PhoneError == 1)  /* MT���ش��� */
    {
        ulResult = At_ChgTafErrorCode(ucIndex,pEvent->PhoneError);       /* �������� */
    }
    else if (pEvent->OperMode.CmdType == TAF_PH_CMD_QUERY)
    {
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%d", pEvent->OperMode.PhMode);
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_OK;
    }

    /* V7R2 ^PSTANDBY����ùػ��������� */
#ifdef FEATURE_UPGRADE_TL
    if((AT_LTE_CMD_CURRENT_OPT_ENUM)gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_PSTANDBY_SET)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        return AT_RRETURN_CODE_BUTT;
    }
#endif

#if((FEATURE_ON == FEATURE_LTE) || (FEATURE_ON == FEATURE_UE_MODE_TDS))
    /* ���GU��������ȷ�����͵�TL�Ⲣ�ȴ���� */
    if (ulResult == AT_OK)
    {
        ucSptLteFlag = AT_IsModemSupportRat(enModemId, TAF_MMA_RAT_LTE);
        ucSptUtralTDDFlag = AT_IsModemSupportUtralTDDRat(enModemId);

        if ((ucSptLteFlag == VOS_TRUE)
         || (ucSptUtralTDDFlag == VOS_TRUE))
        {
            if ((gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_TMODE_SET)
             || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_SET_TMODE))
            {
                AT_ProcOperModeWhenLteOn(ucIndex);

                return AT_RRETURN_CODE_BUTT;
            }
        }
    }
#endif

    AT_STOP_TIMER_CMD_READY(ucIndex);

    return ulResult;
}

TAF_UINT32 At_ProcMePersonalizationOpRsltOk(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                         ulResult;
    TAF_UINT32                         ulTmp;

    ulResult = AT_FAILURE;
    ulTmp = 0;

    switch(pEvent->MePersonalisation.CmdType)
    {
        case TAF_ME_PERSONALISATION_ACTIVE:
        case TAF_ME_PERSONALISATION_DEACTIVE:
        case TAF_ME_PERSONALISATION_SET:
        case TAF_ME_PERSONALISATION_PWD_CHANGE:
        case TAF_ME_PERSONALISATION_VERIFY:
            ulResult = AT_OK;
            break;

        case TAF_ME_PERSONALISATION_QUERY:
            if ( gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CARD_LOCK_READ )
            {
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%d,",pEvent->MePersonalisation.unReportContent.OperatorLockInfo.OperatorLockStatus);
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%d,",pEvent->MePersonalisation.unReportContent.OperatorLockInfo.RemainTimes);
                if( (pEvent->MePersonalisation.unReportContent.OperatorLockInfo.OperatorLen < TAF_PH_ME_LOCK_OPER_LEN_MIN)
                    ||(pEvent->MePersonalisation.unReportContent.OperatorLockInfo.OperatorLen > TAF_PH_ME_LOCK_OPER_LEN_MAX))
                {
                    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"0");
                }
                else
                {
                    for (ulTmp = 0;ulTmp< pEvent->MePersonalisation.unReportContent.OperatorLockInfo.OperatorLen;ulTmp++)
                    {
                        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%d",pEvent->MePersonalisation.unReportContent.OperatorLockInfo.Operator[ulTmp]);
                    }
                }
                ulResult = AT_OK;
            }
            else
            {
                *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                if(pEvent->MePersonalisation.ActiveStatus == TAF_ME_PERSONALISATION_ACTIVE_STATUS)
                {
                    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"1");
                }
                else
                {
                    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"0");
                }
                ulResult = AT_OK;
            }
            break;

        case TAF_ME_PERSONALISATION_RETRIEVE:
            *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            for (ulTmp = 0; ulTmp < pEvent->MePersonalisation.unReportContent.SimPersionalisationStr.DataLen; ulTmp++)
            {
                *(pgucAtSndCodeAddr + *pusLength + ulTmp) = pEvent->MePersonalisation.unReportContent.SimPersionalisationStr.aucSimPersonalisationStr[ulTmp] + 0x30;
            }
            *pusLength += (VOS_UINT16)ulTmp;
            break;

        default:
            ulResult = AT_ERROR;
            break;
    }

    return ulResult;
}


TAF_UINT32 At_ProcPhoneEvtMePersonalizationCnf(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                         ulResult;

    ulResult = AT_FAILURE;

    if (pEvent->MePersonalisation.OpRslt != TAF_PH_ME_PERSONALISATION_OK)
    {
        if (pEvent->MePersonalisation.OpRslt == TAF_PH_ME_PERSONALISATION_NO_SIM)
        {
            ulResult = At_ChgTafErrorCode(ucIndex, TAF_ERR_CMD_TYPE_ERROR);
        }
        else if (pEvent->MePersonalisation.OpRslt == TAF_PH_ME_PERSONALISATION_OP_NOT_ALLOW)
        {
            ulResult = AT_CME_OPERATION_NOT_ALLOWED;
        }
        else if (pEvent->MePersonalisation.OpRslt == TAF_PH_ME_PERSONALISATION_WRONG_PWD)
        {
            ulResult = AT_CME_INCORRECT_PASSWORD;
        }
        else
        {
            ulResult = AT_ERROR;
        }
    }
    else
    {
        ulResult = At_ProcMePersonalizationOpRsltOk(pEvent, pusLength, ucIndex);
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    return ulResult;
}

TAF_UINT32 At_ProcPhoneEvtPlmnListRej(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                          ulResult;

    ulResult = AT_FAILURE;

    if (pEvent->PhoneError == TAF_ERR_NO_RF)
    {
        ulResult = AT_CME_NO_RF;
    }
    else
    {
        ulResult = AT_CME_OPERATION_NOT_ALLOWED;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    return ulResult;
}

TAF_UINT32 At_ProcPhoneEvtUsimResponse(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                          ulResult;

    ulResult = AT_OK;

    /* +CSIM:  */
    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    if (pEvent->OP_UsimAccessData == 1)
    {
        /* <length>, */
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength,"%d,\"", pEvent->UsimAccessData.ucLen * 2);
        /* <command>, */
        *pusLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + *pusLength, pEvent->UsimAccessData.aucResponse, pEvent->UsimAccessData.ucLen);
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "\"");
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    return ulResult;
}

TAF_UINT32 At_ProcPhoneEvtRestrictedAccessCnf(
    TAF_PHONE_EVENT_INFO_STRU          *pEvent,
    TAF_UINT16                         *pusLength,
    TAF_UINT8                           ucIndex
)
{
    TAF_UINT32                          ulResult;

    ulResult = AT_OK;

    *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    if(pEvent->OP_UsimRestrictAccess == 1)
    {
        /* <sw1, sw2>, */
        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "%d,%d", pEvent->RestrictedAccess.ucSW1, pEvent->RestrictedAccess.ucSW2);

        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, ",\"");

        if(pEvent->RestrictedAccess.ucLen != 0)
        {
            /* <response> */
            *pusLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)pgucAtSndCodeAddr, (TAF_UINT8 *)pgucAtSndCodeAddr + *pusLength, pEvent->RestrictedAccess.aucContent, pEvent->RestrictedAccess.ucLen);
        }

        *pusLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + *pusLength, "\"");
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    return ulResult;
}


TAF_VOID At_PhRspProc(TAF_UINT8 ucIndex,TAF_PHONE_EVENT_INFO_STRU  *pEvent)
{
    TAF_UINT32                          ulResult;
    TAF_UINT32                          ulRst;
    TAF_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId           = MODEM_ID_0;
    ulResult            = AT_FAILURE;
    usLength            = 0;

    ulRst = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG1("At_PhRspProc:Get ModemID From ClientID fail,ClientID:", ucIndex);

        return;
    }

    switch (pEvent->PhoneEvent)
    {
    case TAF_PH_EVT_ERR:

        ulResult = At_ChgTafErrorCode(ucIndex, pEvent->PhoneError);       /* �������� */

        AT_STOP_TIMER_CMD_READY(ucIndex);

        break;

    case TAF_PH_EVT_PLMN_LIST_REJ:

        ulResult = At_ProcPhoneEvtPlmnListRej(pEvent, ucIndex);

        break;

    case TAF_PH_EVT_OP_PIN_CNF:

        ulResult = At_ProcPhoneEvtOperPinCnf(pEvent, enModemId, ucIndex);

        if (ulResult == AT_RRETURN_CODE_BUTT)
        {
            return;
        }

        break;

    case TAF_PH_EVT_OPER_MODE_CNF:

        ulResult = At_ProcPhoneEvtOperModeCnf(pEvent, &usLength, enModemId, ucIndex);

        if (ulResult == AT_RRETURN_CODE_BUTT)
        {
            return;
        }

        break;

    case TAF_PH_EVT_USIM_RESPONSE:

        ulResult = At_ProcPhoneEvtUsimResponse(pEvent, &usLength, ucIndex);

        break;

    case TAF_PH_EVT_RESTRICTED_ACCESS_CNF:

        ulResult = At_ProcPhoneEvtRestrictedAccessCnf(pEvent, &usLength, ucIndex);

        break;

    case TAF_PH_EVT_OP_PINREMAIN_CNF:

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + usLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + usLength, "%d,%d,%d,%d ", pEvent->PinRemainCnf.ucPIN1Remain, pEvent->PinRemainCnf.ucPUK1Remain, pEvent->PinRemainCnf.ucPIN2Remain, pEvent->PinRemainCnf.ucPUK2Remain);

        AT_STOP_TIMER_CMD_READY(ucIndex);

        ulResult = AT_OK;

        break;

    case TAF_PH_EVT_ME_PERSONALISATION_CNF:

        ulResult = At_ProcPhoneEvtMePersonalizationCnf(pEvent, &usLength, ucIndex);

        break;

    case TAF_PH_EVT_SETUP_SYSTEM_INFO_RSP:

        AT_NORM_LOG("At_PhRspProc EVT SETUP SYSTEM INFO RSP,Do nothing.");

        return;

    case TAF_PH_EVT_PLMN_LIST_ABORT_CNF:

        /* �ݴ���, ��ǰ�����б���ABORT���������ϱ�ABORT.
           ��AT��ABORT������ʱ���ѳ�ʱ, ֮�����յ�MMA��ABORT_CNF���ϱ�ABORT */
        if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_COPS_ABORT_PLMN_LIST)
        {
            AT_WARN_LOG("At_PhRspProc  NOT ABORT PLMN LIST. ");

            return;
        }

        ulResult = AT_ABORT;

        AT_STOP_TIMER_CMD_READY(ucIndex);

        break;

    default:

        AT_WARN_LOG("At_PhRspProc Other PhoneEvent");

        return;
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);
}

TAF_VOID At_PhEventProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    TAF_PHONE_EVENT_INFO_STRU *pEvent = VOS_NULL_PTR;
    TAF_UINT8 ucIndex = 0;

    pEvent = (TAF_PHONE_EVENT_INFO_STRU *)pData;

    AT_LOG1("At_PhMsgProc pEvent->ClientId",pEvent->ClientId);
    AT_LOG1("At_PhMsgProc PhoneEvent",pEvent->PhoneEvent);
    AT_LOG1("At_PhMsgProc PhoneError",pEvent->PhoneError);

    if(At_ClientIdToUserId(pEvent->ClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_PhRspProc At_ClientIdToUserId FAILURE");
        return;
    }

    if(AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        At_PhIndProc(ucIndex, pEvent);
    }
    else
    {
        AT_LOG1("At_PhMsgProc ucIndex",ucIndex);
        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

        At_PhRspProc(ucIndex,pEvent);
    }
}
#if (FEATURE_CSG == FEATURE_ON)

VOS_VOID AT_ReportCsgListSearchCnfResult(
    TAF_MMA_CSG_LIST_CNF_PARA_STRU     *pstCsgList,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucHomeNodeBLen;
    VOS_UINT8                           ucCsgTypeLen;
    VOS_UINT32                          i;
    VOS_UINT32                          j;

    usLength   = *pusLength;

    for (i = 0; i < (VOS_UINT32)AT_MIN(pstCsgList->ucPlmnWithCsgIdNum, TAF_MMA_MAX_CSG_ID_LIST_NUM); i++)
    {
        /* ����һ���⣬������ǰҪ�Ӷ��� */
        if ((i != 0)
         || (pstCsgList->ulCurrIndex != 0))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",");
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"(");

        /* ��ӡ������ */
        if (( pstCsgList->astCsgIdListInfo[i].aucOperatorNameLong[0] == '\0')
         || ( pstCsgList->astCsgIdListInfo[i].aucOperatorNameShort[0] == '\0' ))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"\"");
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"\"");
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"%s\"",pstCsgList->astCsgIdListInfo[i].aucOperatorNameLong);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%s\"",pstCsgList->astCsgIdListInfo[i].aucOperatorNameShort);
        }

        /* ��ӡ���ָ�ʽ����Ӫ������  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%X%X%X",
            (0x0f00 & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mcc) >> AT_OCTET_MOVE_EIGHT_BITS,
            (AT_OCTET_HIGH_FOUR_BITS & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mcc) >> AT_OCTET_MOVE_FOUR_BITS,
            (AT_OCTET_LOW_FOUR_BITS & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mcc));

        if (((0x0f00 & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mnc) >> AT_OCTET_MOVE_EIGHT_BITS) != AT_OCTET_LOW_FOUR_BITS)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%X",
            (0x0f00 & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mnc) >> AT_OCTET_MOVE_EIGHT_BITS);

        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%X%X\"",
            (AT_OCTET_HIGH_FOUR_BITS & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mnc) >> AT_OCTET_MOVE_FOUR_BITS,
            (AT_OCTET_LOW_FOUR_BITS & pstCsgList->astCsgIdListInfo[i].stPlmnId.Mnc));

        /* ��ӡCSG ID */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%X\"", pstCsgList->astCsgIdListInfo[i].ulCsgId);

        /* ��ӡCSG ID TYPE, 1��CSG ID��Allowed CSG List��; 2��CSG ID��Operator CSG List�в��ڽ�ֹCSG ID�б���;
                            3��CSG ID��Operator CSG List�в����ڽ�ֹCSG ID�б���; 4��CSG ID����Allowed CSG List��Operator CSG List��*/
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d,", pstCsgList->astCsgIdListInfo[i].enPlmnWithCsgIdType);

        /* ��ӡhome NodeB Name, ucs2���룬��󳤶�48�ֽ� */
        ucHomeNodeBLen = AT_MIN(pstCsgList->astCsgIdListInfo[i].stCsgIdHomeNodeBName.ucHomeNodeBNameLen, TAF_MMA_MAX_HOME_NODEB_NAME_LEN);

        for (j = 0; j < ucHomeNodeBLen; j++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%02X",
                                               pstCsgList->astCsgIdListInfo[i].stCsgIdHomeNodeBName.aucHomeNodeBName[j]);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + usLength, ",");

        /* ��ӡCSG���ͣ���UCS-2 ��ʽ����, ��󳤶�12�ֽ�*/
        ucCsgTypeLen = AT_MIN(pstCsgList->astCsgIdListInfo[i].stCsgType.ucCsgTypeLen, TAF_MMA_MAX_CSG_TYPE_LEN);

        for (j = 0; j < ucCsgTypeLen; j++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%02X",
                                               pstCsgList->astCsgIdListInfo[i].stCsgType.aucCsgType[j]);
        }

        if (pstCsgList->astCsgIdListInfo[i].ucRaMode == TAF_PH_RA_GSM)  /* GSM */
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",0");
        }
        else if (pstCsgList->astCsgIdListInfo[i].ucRaMode == TAF_PH_RA_WCDMA)     /* W*/
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",2");
        }
        else if(pstCsgList->astCsgIdListInfo[i].ucRaMode == TAF_PH_RA_LTE)   /* LTE */
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",7");
        }
        else
        {
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           pstCsgList->astCsgIdListInfo[i].sSignalValue1);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           pstCsgList->astCsgIdListInfo[i].sSignalValue2);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,")");
    }

    *pusLength = usLength;
}



VOS_UINT32 AT_RcvMmaCsgListSearchCnfProc(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_CSG_LIST_SEARCH_CNF_STRU   *pstCsgListCnf = VOS_NULL_PTR;
    TAF_MMA_CSG_LIST_CNF_PARA_STRU     *pstCsgList    = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    TAF_MMA_PLMN_LIST_PARA_STRU         stCsgListPara;
    AT_RRETURN_CODE_ENUM_UINT32         enResult;

    usLength       = 0;
    pstCsgListCnf = (TAF_MMA_CSG_LIST_SEARCH_CNF_STRU *)pMsg;
    pstCsgList    = &pstCsgListCnf->stCsgListCnfPara;
    memset_s(&stCsgListPara, sizeof(stCsgListPara), 0x00, sizeof(stCsgListPara));

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCsgListCnf->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaCsgListSearchCnfProc : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaCsgListSearchCnfProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �ݴ���, �統ǰ������CSG�б���ABORT���������ϱ��б��ѽ�� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSG_LIST_SEARCH )
    {
        AT_WARN_LOG("AT_RcvMmaCsgListSearchCnfProc, csg LIST CNF when Abort Plmn List or timeout. ");
        return VOS_ERR;
    }


    /* �����ʧ���¼�,ֱ���ϱ�ERROR */
    if (pstCsgList->ucOpError == VOS_TRUE)
    {
        enResult = (AT_RRETURN_CODE_ENUM_UINT32)At_ChgTafErrorCode(ucIndex, pstCsgList->enPhoneError);

        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, enResult);

        return VOS_OK;
    }

    /* �״β�ѯ�ϱ����ʱ��Ҫ��ӡ^CSGIDSRCH: */
    if (pstCsgList->ulCurrIndex == 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s",
                                           gaucAtCrLf);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    AT_ReportCsgListSearchCnfResult(pstCsgList, &usLength);

    At_BufferorSendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    usLength = 0;

    /* ��������ϱ���plmn��Ŀ��Ҫ�����ͬ������ΪC������Plmn listû���ϱ���Ҫ��������������в�ѯ */
    if (pstCsgList->ucPlmnWithCsgIdNum == TAF_MMA_MAX_CSG_ID_LIST_NUM)
    {
        stCsgListPara.usQryNum    = TAF_MMA_MAX_CSG_ID_LIST_NUM;
        stCsgListPara.usCurrIndex = (VOS_UINT16)(pstCsgList->ulCurrIndex + pstCsgList->ucPlmnWithCsgIdNum);

        if (TAF_MMA_CsgListSearchReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stCsgListPara) == VOS_TRUE)
        {
            /* ���õ�ǰ�������� */
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CSG_LIST_SEARCH;
            return VOS_OK;
        }

        /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\r\n");

    At_BufferorSendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    usLength = 0;

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}



VOS_UINT32 AT_RcvMmaCsgListAbortCnf(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_CSG_LIST_ABORT_CNF_STRU    *pstPlmnListAbortCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    pstPlmnListAbortCnf = (TAF_MMA_CSG_LIST_ABORT_CNF_STRU *)pMsg;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstPlmnListAbortCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaCsgListAbortCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaCsgListAbortCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    /* �ݴ���, ��ǰ����CSG�б���ABORT���������ϱ�ABORT.
       ��AT��ABORT������ʱ���ѳ�ʱ, ֮�����յ�MMA��ABORT_CNF���ϱ�ABORT */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ABORT_CSG_LIST_SEARCH)
    {
        AT_WARN_LOG("AT_RcvMmaCsgListAbortCnf : Current Option is not correct.");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    ulResult = AT_ABORT;

    gstAtSendData.usBufLen = 0;

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvMmaCsgSpecSearchCnfProc(
    VOS_VOID                           *pstMsg
)
{
    TAF_MMA_CSG_SPEC_SEARCH_CNF_STRU   *pstPlmnSpecialSelCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    pstPlmnSpecialSelCnf = (TAF_MMA_CSG_SPEC_SEARCH_CNF_STRU *)pstMsg;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstPlmnSpecialSelCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaCsgSpecSearchCnfProc : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaCsgSpecSearchCnfProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSG_SPEC_SEARCH)
    {
        AT_WARN_LOG("AT_RcvMmaCsgSpecSearchCnfProc : Current Option is not correct.");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstPlmnSpecialSelCnf->enErrorCause == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = At_ChgTafErrorCode(ucIndex, pstPlmnSpecialSelCnf->enErrorCause);
    }

    gstAtSendData.usBufLen = 0;

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvMmaQryCampCsgIdInfoCnfProc(
    VOS_VOID                           *pstMsg
)
{
    VOS_UINT16                                    usLength = 0;
    TAF_MMA_QRY_CAMP_CSG_ID_INFO_CNF_STRU        *pstQryCnfMsg = VOS_NULL_PTR;
    VOS_UINT8                                     ucIndex;

    pstQryCnfMsg = (TAF_MMA_QRY_CAMP_CSG_ID_INFO_CNF_STRU*)pstMsg;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstQryCnfMsg->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaQryCampCsgIdInfoCnfProc : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �㲥��Ϣ������ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaQryCampCsgIdInfoCnfProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �жϵ�ǰ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSG_ID_INFO_QRY)
    {
        AT_WARN_LOG("AT_RcvMmaQryCampCsgIdInfoCnfProc: WARNING:Not AT_CMD_CSG_ID_INFO_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* <CR><LF>^CSGIDSRCH: [<oper>[,<CSG ID>][,<rat>]]<CR><LF>
       <CR>OK<LF>
     */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* ���PLMN ID�Ƿ�����뼼���Ƿ���CSG ID��Ч���������ֻ��ʾOK */
    if ((pstQryCnfMsg->stPlmnId.Mcc == TAF_MMA_INVALID_MCC)
     || (pstQryCnfMsg->stPlmnId.Mnc == TAF_MMA_INVALID_MNC)
     || (pstQryCnfMsg->ucRatType >= TAF_MMA_RAT_BUTT))
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "\"\",\"\",");
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, AT_OK);
        return VOS_OK;
    }

    /* BCD���MCC��MNC�����ʱ��Ҫת�����ַ��� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"%X%X%X",
                                       (0x0f00 & pstQryCnfMsg->stPlmnId.Mcc) >> 8,
                                       (0x00f0 & pstQryCnfMsg->stPlmnId.Mcc) >> 4,
                                       (0x000f & pstQryCnfMsg->stPlmnId.Mcc));

    if(((0x0f00 & pstQryCnfMsg->stPlmnId.Mnc) >> 8) != 0x0F)
    {
        usLength +=(VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%X",
                                          (0x0f00 & pstQryCnfMsg->stPlmnId.Mnc) >> 8);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%X%X\"",
                                       (0x00f0 & pstQryCnfMsg->stPlmnId.Mnc) >> 4,
                                       (0x000f & pstQryCnfMsg->stPlmnId.Mnc));


    /* ���CSG ID */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",\"%X\"",
                                       pstQryCnfMsg->ulCsgId);

    /* <rat> */
    if (pstQryCnfMsg->ucRatType == TAF_MMA_RAT_LTE)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",7");
    }
    if (pstQryCnfMsg->ucRatType == TAF_PH_RA_GSM)  /* GSM */
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",0");
    }
    else if (pstQryCnfMsg->ucRatType == TAF_PH_RA_WCDMA)     /* W*/
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",2");
    }
    else
    {
    }


    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}

#endif


LOCAL VOS_CHAR* AT_ConvertRatModeForQryParaPlmnList(
    TAF_PH_RA_MODE  enRaMode
)
{
    VOS_CHAR        *pcCopsRat = VOS_NULL_PTR;

    switch (enRaMode)
    {
        case TAF_PH_RA_GSM:
            pcCopsRat = "0";
            break;

        case TAF_PH_RA_WCDMA:
            pcCopsRat = "2";
            break;

        case TAF_PH_RA_LTE:
            pcCopsRat = "7";
            break;
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case TAF_PH_RA_NR:
            pcCopsRat = "12";
            break;
#endif
        default:
            pcCopsRat = "";

    }

    return pcCopsRat;
}

VOS_UINT32 At_QryParaPlmnListProc(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_PLMN_LIST_CNF_STRU         *pstPlmnListCnf = VOS_NULL_PTR;
    TAF_MMA_PLMN_LIST_CNF_PARA_STRU    *pstPlmnList = VOS_NULL_PTR;
    TAF_MMA_PLMN_LIST_PARA_STRU         stPlmnListPara;
    AT_RRETURN_CODE_ENUM_UINT32         enResult;
    VOS_CHAR                           *pcRatMode = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucTmp;
    VOS_UINT8                           ucIndex;

    usLength       = 0;
    pstPlmnListCnf = (TAF_MMA_PLMN_LIST_CNF_STRU *)pMsg;
    pstPlmnList    = &pstPlmnListCnf->stPlmnListCnfPara;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstPlmnListCnf->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvMmaDetachCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_PhPlmnListProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* �ݴ���, �統ǰ�������б���ABORT���������ϱ��б��ѽ�� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_COPS_TEST )
    {
        AT_WARN_LOG("At_PhPlmnListProc, TAF_PH_EVT_PLMN_LIST_CNF when Abort Plmn List or timeout. ");
        return VOS_ERR;
    }


    /* �����ʧ���¼�,ֱ���ϱ�ERROR */
    if (pstPlmnList->ucOpError == 1)
    {
        enResult = (AT_RRETURN_CODE_ENUM_UINT32)At_ChgTafErrorCode(ucIndex, pstPlmnList->enPhoneError);

        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, enResult);

        return VOS_OK;
    }
    if (pstPlmnList->ulCurrIndex == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               "%s",
                                               gaucAtCrLf);


        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    for(ucTmp = 0; ucTmp < pstPlmnList->ulPlmnNum; ucTmp++)
    {
        if((ucTmp != 0)
        || (pstPlmnList->ulCurrIndex != 0))/* ����һ���⣬������ǰҪ�Ӷ��� */
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength, ",");
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "(%d,\"%s\",\"%s\"",
                                           pstPlmnList->astPlmnInfo[ucTmp].PlmnStatus,
                                           pstPlmnList->astPlmnName[ucTmp].aucOperatorNameLong,
                                           pstPlmnList->astPlmnName[ucTmp].aucOperatorNameShort);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength, ",\"%X%X%X",
                                           (0x0f00 & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mcc) >> 8,
                                           (0x00f0 & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mcc) >> 4,
                                           (0x000f & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mcc));

        if(((0x0f00 & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mnc) >> 8) != 0x0F)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%X",
                                               (0x0f00 & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mnc) >> 8);
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%X%X\"",
                                           (0x00f0 & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mnc) >> 4,
                                           (0x000f & pstPlmnList->astPlmnName[ucTmp].PlmnId.Mnc));

        pcRatMode = AT_ConvertRatModeForQryParaPlmnList(pstPlmnList->astPlmnInfo[ucTmp].RaMode);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength, ",%s)", pcRatMode);

    }

    At_BufferorSendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    usLength = 0;

    /* ��������ϱ���plmn��Ŀ��Ҫ�����ͬ������ΪC������Plmn listû���ϱ���Ҫ��������������в�ѯ */
    if (pstPlmnList->ulPlmnNum == TAF_MMA_MAX_PLMN_NAME_LIST_NUM)
    {
        stPlmnListPara.usQryNum    = TAF_MMA_MAX_PLMN_NAME_LIST_NUM;
        stPlmnListPara.usCurrIndex = (VOS_UINT16)(pstPlmnList->ulCurrIndex + pstPlmnList->ulPlmnNum);

        if (Taf_PhonePlmnList(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stPlmnListPara) == VOS_TRUE)
        {
            /* ���õ�ǰ�������� */
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_COPS_TEST;
            return VOS_OK;
        }

        /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",,(0,1,2,3,4),(0,1,2)\r\n");

    At_BufferorSendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    usLength = 0;

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_UINT32 At_PlmnDetectIndProc(
    VOS_VOID                           *pMsg
)
{
    TAF_MMA_PLMN_DETECT_IND_STRU       *pstPlmnDetectCnf = VOS_NULL_PTR;
    TAF_MMA_PLMN_DETECT_IND_PARA_STRU  *pstPlmnDetect = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    usLength         = 0;
    pstPlmnDetectCnf = (TAF_MMA_PLMN_DETECT_IND_STRU *)pMsg;
    pstPlmnDetect    = &pstPlmnDetectCnf->stPlmnDetectIndPara;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstPlmnDetectCnf->usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_PlmnDetectIndProc : WARNING:AT INDEX NOT FOUND!");
        return VOS_TRUE;
    }

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s:",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_DETECTPLMN].pucText);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%X%X%X",
                                       (0x0000000f & pstPlmnDetect->stPlmnId.Mcc),
                                       (0x00000f00 & pstPlmnDetect->stPlmnId.Mcc) >> 8,
                                       (0x000f0000 & pstPlmnDetect->stPlmnId.Mcc) >> 16
                                       );

    if(((0x000f0000 & (pstPlmnDetect->stPlmnId.Mnc)) >> 16) != 0x0F)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%X",
                                           (0x000f0000 & pstPlmnDetect->stPlmnId.Mnc) >> 16
                                           );

    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%X%X",
                                        (0x0000000f & pstPlmnDetect->stPlmnId.Mnc),
                                        (0x00000f00 & pstPlmnDetect->stPlmnId.Mnc) >> 8
                                        );

    /* <rat> */
    if (pstPlmnDetect->enRatMode == TAF_MMA_RAT_LTE)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",7");
    }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    else if (pstPlmnDetect->enRatMode == TAF_MMA_RAT_NR)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",12");
    }
#endif
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstPlmnDetect->sRsrp);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


TAF_VOID At_PhMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{


    At_PhEventProc(pData,usLen);
}


TAF_UINT32 At_Unicode2UnicodePrint(TAF_UINT32 MaxLength,TAF_INT8 *headaddr,TAF_UINT8 *pucDst, TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen)
{
    TAF_UINT16 usLen    = 0;
    TAF_UINT16 usChkLen = 0;
    TAF_UINT8  ucHigh1  = 0;
    TAF_UINT8  ucHigh2  = 0;
    TAF_UINT8  ucLow1   = 0;
    TAF_UINT8  ucLow2   = 0;
    TAF_UINT8 *pWrite   = pucDst;
    TAF_UINT8 *pRead    = pucSrc;

    if(((TAF_UINT32)(pucDst - (TAF_UINT8 *)headaddr) + (2 * usSrcLen)) >= MaxLength)
    {
        AT_ERR_LOG("At_Unicode2UnicodePrint too long");
        return 0;
    }

    /* ɨ�������ִ� */
    while( usChkLen < usSrcLen )
    {
        /* ��һ���ֽ� */
        ucHigh1 = 0x0F & (*pRead >> 4);
        ucHigh2 = 0x0F & *pRead;

        if(ucHigh1 <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucHigh1 + 0x30;
        }
        else if(ucHigh1 >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucHigh1 + 0x37;
        }
        else
        {

        }

        if(ucHigh2 <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucHigh2 + 0x30;
        }
        else if(ucHigh2 >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucHigh2 + 0x37;
        }
        else
        {

        }

        /* ��һ���ַ� */
        usChkLen++;
        pRead++;

        /* �ڶ����ֽ� */
        ucLow1 = 0x0F & (*pRead >> 4);
        ucLow2 = 0x0F & *pRead;


        if(ucLow1 <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucLow1 + 0x30;
        }
        else if(ucLow1 >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucLow1 + 0x37;
        }
        else
        {

        }

        if(ucLow2 <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucLow2 + 0x30;
        }
        else if(ucLow2 >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucLow2 + 0x37;
        }
        else
        {

        }

        /* ��һ���ַ� */
        usChkLen++;
        pRead++;

        usLen += 4;    /* ��¼���� */
    }

    return usLen;
}

TAF_UINT32 At_HexString2AsciiNumPrint(TAF_UINT32 MaxLength,TAF_INT8 *headaddr,TAF_UINT8 *pucDst, TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen)
{
    TAF_UINT16 usLen    = 0;
    TAF_UINT16 usChkLen = 0;
    TAF_UINT8  ucHigh1  = 0;
    TAF_UINT8  ucHigh2  = 0;
    TAF_UINT8 *pWrite   = pucDst;
    TAF_UINT8 *pRead    = pucSrc;

    if(((TAF_UINT32)(pucDst - (TAF_UINT8 *)headaddr) + (2 * usSrcLen)) >= MaxLength)
    {
        AT_ERR_LOG("At_Unicode2UnicodePrint too long");
        return 0;
    }

    /* ɨ�������ִ� */
    while( usChkLen < usSrcLen )
    {
        /* ��һ���ֽ� */
        ucHigh1 = 0x0F & (*pRead >> 4);
        ucHigh2 = 0x0F & *pRead;

        if(ucHigh1 <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucHigh1 + 0x30;
        }
        else if(ucHigh1 >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucHigh1 + 0x37;
        }
        else
        {

        }

        if(ucHigh2 <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucHigh2 + 0x30;
        }
        else if(ucHigh2 >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucHigh2 + 0x37;
        }
        else
        {

        }

        /* ��һ���ַ� */
        usChkLen++;
        pRead++;
        usLen += 2;    /* ��¼���� */
    }

    return usLen;
}



TAF_UINT32 At_Ascii2UnicodePrint(TAF_UINT32 MaxLength,TAF_INT8 *headaddr,TAF_UINT8 *pucDst, TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen)
{
    TAF_UINT16 usLen = 0;
    TAF_UINT16 usChkLen = 0;
    TAF_UINT8 *pWrite   = pucDst;
    TAF_UINT8 *pRead    = pucSrc;
    TAF_UINT8  ucHigh = 0;
    TAF_UINT8  ucLow = 0;

    if(((TAF_UINT32)(pucDst - (TAF_UINT8 *)headaddr) + (4 * usSrcLen)) >= MaxLength)
    {
        AT_ERR_LOG("At_Ascii2UnicodePrint too long");
        return 0;
    }

    /* ɨ�������ִ� */
    while( usChkLen++ < usSrcLen )
    {
        *pWrite++ = '0';
        *pWrite++ = '0';
        ucHigh = 0x0F & (*pRead >> 4);
        ucLow = 0x0F & *pRead;

        usLen += 4;    /* ��¼���� */

        if(ucHigh <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucHigh + 0x30;
        }
        else if(ucHigh >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucHigh + 0x37;
        }
        else
        {

        }

        if(ucLow <= 0x09)   /* 0-9 */
        {
            *pWrite++ = ucLow + 0x30;
        }
        else if(ucLow >= 0x0A)    /* A-F */
        {
            *pWrite++ = ucLow + 0x37;
        }
        else
        {

        }

        /* ��һ���ַ� */
        pRead++;
    }

    return usLen;
}

TAF_UINT16 At_PrintReportData(
    TAF_UINT32                          MaxLength,
    TAF_INT8                            *headaddr,
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding,
    TAF_UINT8                           *pucDst,
    TAF_UINT8                           *pucSrc,
    TAF_UINT16                          usSrcLen
)
{
    TAF_UINT16                          usLength = 0;
    TAF_UINT32                          ulPrintStrLen;
    TAF_UINT32                          ulMaxMemLength;
    VOS_UINT32                          ulRet;
    errno_t                             lMemResult;
    VOS_UINT8                           aucPrintUserData[MN_MSG_MAX_8_BIT_LEN];


    ulPrintStrLen = 0;


    /* DCS: UCS2 */
    if (enMsgCoding == MN_MSG_MSG_CODING_UCS2)
    {
        usLength += (TAF_UINT16)At_Unicode2UnicodePrint(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,pucDst + usLength,pucSrc,usSrcLen);
    }
    /* DCS: 8BIT */
    else if (enMsgCoding == MN_MSG_MSG_CODING_8_BIT)                                   /* DATA:8BIT */
    {
        if (AT_CSCS_GSM_7Bit_CODE == gucAtCscsType)
        {
            memset_s(aucPrintUserData, sizeof(aucPrintUserData), 0, MN_MSG_MAX_8_BIT_LEN);

            usSrcLen      = TAF_MIN(usSrcLen, MN_MSG_MAX_8_BIT_LEN);
            ulPrintStrLen = 0;
            ulRet = TAF_STD_ConvertAsciiToDefAlpha(pucSrc,
                                                   usSrcLen,
                                                   aucPrintUserData,
                                                   &ulPrintStrLen,
                                                   usSrcLen);
            if (ulRet != MN_ERR_NO_ERROR)
            {
                AT_NORM_LOG("At_PrintReportData : TAF_STD_ConvertAsciiToDefAlpha fail. ");
                return 0;
            }

            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            pucDst + usLength,
                                                            aucPrintUserData,
                                                            (VOS_UINT16)ulPrintStrLen);
        }
        else
        {
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            pucDst + usLength,
                                                            pucSrc,
                                                            usSrcLen);
        }
    }
    /* DCS: 7BIT */
    else
    {
        if(gucAtCscsType == AT_CSCS_UCS2_CODE)       /* +CSCS:UCS2 */
        {
            usLength += (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,pucDst + usLength,pucSrc,usSrcLen);
        }
        else
        {
            if(((TAF_UINT32)(pucDst - (TAF_UINT8 *)headaddr) + usSrcLen) >= MaxLength)
            {
                AT_ERR_LOG("At_PrintReportData too long");
                return 0;
            }

            if ((gucAtCscsType == AT_CSCS_IRA_CODE)
             && (enMsgCoding == MN_MSG_MSG_CODING_7_BIT))
            {
                TAF_STD_ConvertDefAlphaToAscii(pucSrc, usSrcLen, (pucDst + usLength), &ulPrintStrLen);
                usLength += (TAF_UINT16)ulPrintStrLen;
            }
            else
            {
                ulMaxMemLength = MaxLength - (TAF_UINT32)(pucDst - (TAF_UINT8*)headaddr);
                lMemResult = memcpy_s((pucDst + usLength), ulMaxMemLength, pucSrc, usSrcLen);
                TAF_MEM_CHK_RTN_VAL(lMemResult, ulMaxMemLength, usSrcLen);
                usLength += usSrcLen;
            }
        }
    }

    return usLength;
}


TAF_UINT32 At_MsgPduInd(
    MN_MSG_BCD_ADDR_STRU                *pstScAddr,
    MN_MSG_RAW_TS_DATA_STRU             *pstPdu,
    TAF_UINT8                           *pucDst
)
{
    TAF_UINT16                          usLength            = 0;

    /* <alpha> ���� */

    /* <length> */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pucDst + usLength),
                                       ",%d",
                                       pstPdu->ulLen);

    /* <data> �п��ܵõ���UCS2������ϸ����*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pucDst + usLength,
                                           "%s",
                                           gaucAtCrLf);

    /*SCA*/
    if (pstScAddr->ucBcdLen == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pucDst + usLength),
                                           "00");
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pucDst + usLength),
                                           "%X%X%X%X",
                                           (((pstScAddr->ucBcdLen + 1) & 0xf0) >> 4),
                                           ((pstScAddr->ucBcdLen + 1) & 0x0f),
                                           ((pstScAddr->addrType & 0xf0) >> 4),
                                           (pstScAddr->addrType & 0x0f));

        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        pucDst + usLength,
                                                        pstScAddr->aucBcdNum,
                                                        (TAF_UINT16)pstScAddr->ucBcdLen);
    }

    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (TAF_INT8 *)pgucAtSndCodeAddr,
                                                    pucDst + usLength,
                                                    pstPdu->aucData,
                                                    (TAF_UINT16)pstPdu->ulLen);

    return usLength;
}


VOS_UINT32 At_StaRptPduInd(
    MN_MSG_BCD_ADDR_STRU                *pstScAddr,
    MN_MSG_RAW_TS_DATA_STRU             *pstPdu,
    VOS_UINT8                           *pucDst
)
{
    VOS_UINT16                          usLength            = 0;

    /* <length> */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)(pucDst + usLength),
                                       "%d",
                                       pstPdu->ulLen);

    /* <data> �п��ܵõ���UCS2������ϸ����*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pucDst + usLength,
                                       "%s",
                                       gaucAtCrLf);

    /*SCA*/
    if (pstScAddr->ucBcdLen == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)(pucDst + usLength),
                                           "00");
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)(pucDst + usLength),
                                           "%X%X%X%X",
                                           (((pstScAddr->ucBcdLen + 1) & 0xf0) >> 4),
                                           ((pstScAddr->ucBcdLen + 1) & 0x0f),
                                           ((pstScAddr->addrType & 0xf0) >> 4),
                                           (pstScAddr->addrType & 0x0f));

        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        pucDst + usLength,
                                                        pstScAddr->aucBcdNum,
                                                        (TAF_UINT16)pstScAddr->ucBcdLen);
    }

    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (TAF_INT8 *)pgucAtSndCodeAddr,
                                                    pucDst + usLength,
                                                    pstPdu->aucData,
                                                    (TAF_UINT16)pstPdu->ulLen);

    return usLength;
}


VOS_UINT32  AT_IsClientBlock(VOS_VOID)
{
    VOS_UINT32                          ulAtStatus;
    VOS_UINT32                          ulAtMode;
    AT_PORT_BUFF_CFG_ENUM_UINT8         enSmsBuffCfg;

    enSmsBuffCfg    = AT_GetPortBuffCfg();
    if (enSmsBuffCfg == AT_PORT_BUFF_DISABLE)
    {
        return VOS_FALSE;
    }

    ulAtStatus  = AT_IsAnyParseClientPend();
    ulAtMode    = AT_IsAllClientDataMode();

    /* ����ǰ��һ��ͨ������ pend״̬������Ҫ������� */
    if (ulAtStatus == VOS_TRUE)
    {
        return VOS_TRUE;
    }

    /* ����ǰ����ͨ��������dataģʽ���򻺴���� */
    if (ulAtMode == VOS_TRUE)
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;

}




TAF_VOID  At_BufferMsgInTa(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_ENUM_U32               enEvent,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    MN_MSG_EVENT_INFO_STRU              *pstEventInfo = VOS_NULL_PTR;
    TAF_UINT8                           *pucUsed = VOS_NULL_PTR;
    AT_MODEM_SMS_CTX_STRU               *pstSmsCtx = VOS_NULL_PTR;
    errno_t                             lMemResult;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    pstSmsCtx->stSmtBuffer.ucIndex = pstSmsCtx->stSmtBuffer.ucIndex % AT_BUFFER_SMT_EVENT_MAX;
    pucUsed = &(pstSmsCtx->stSmtBuffer.aucUsed[pstSmsCtx->stSmtBuffer.ucIndex]);
    pstEventInfo = &(pstSmsCtx->stSmtBuffer.astEvent[pstSmsCtx->stSmtBuffer.ucIndex]);
    pstSmsCtx->stSmtBuffer.ucIndex++;


    if (*pucUsed == AT_MSG_BUFFER_FREE)
    {
        *pucUsed = AT_MSG_BUFFER_USED;
    }

    lMemResult = memcpy_s(pstEventInfo, sizeof(MN_MSG_EVENT_INFO_STRU), pstEvent, sizeof(MN_MSG_EVENT_INFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(MN_MSG_EVENT_INFO_STRU), sizeof(MN_MSG_EVENT_INFO_STRU));
    return;
}


VOS_UINT16 AT_PrintSmsLength(
    MN_MSG_MSG_CODING_ENUM_U8           enMsgCoding,
    VOS_UINT32                          ulLength,
    VOS_UINT8                          *pDst
)
{
    VOS_UINT16                          usLength;
    VOS_UINT16                          usSmContentLength;

    /* UCS2������ʾ�ֽڳ���Ӧ����UCS2�ַ�����������BYTE����������Ҫ�ֽ�������2 */
    if (enMsgCoding == MN_MSG_MSG_CODING_UCS2)
    {
        usSmContentLength = (VOS_UINT16)ulLength >> 1;
    }
    else
    {
        usSmContentLength = (VOS_UINT16)ulLength;
    }

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pDst,
                                       ",%d",
                                       usSmContentLength);

    return usLength;
}


TAF_VOID At_ForwardMsgToTeInCmt(
    VOS_UINT8                            ucIndex,
    TAF_UINT16                          *pusSendLength,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT16                          usLength = *pusSendLength;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    /* +CMT */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "+CMT: ");

    if (pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT)
    {
        /* +CMT: <oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data> */
        /* <oa> */
        usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stDeliver.stOrigAddr,
                                                  (pgucAtSndCodeAddr + usLength));
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           ",");

        /* <alpha> ���� */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           ",");

        /* <scts> */
        usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stDeliver.stTimeStamp,
                                                (pgucAtSndCodeAddr + usLength));
        if (pstSmsCtx->ucCsdhType == AT_CSDH_SHOW_TYPE)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");
            /* <tooa> */
            usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stDeliver.stOrigAddr,
                                                     (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

            /*<fo>*/
            usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pgucAtSndCodeAddr + usLength));

            /* <pid> */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",%d",
                                               pstTsDataInfo->u.stDeliver.enPid);

            /* <dcs> */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",%d,",
                                               pstTsDataInfo->u.stDeliver.stDcs.ucRawDcsData);

            /* <sca> */
            usLength += (TAF_UINT16)At_PrintBcdAddr(&pstEvent->u.stDeliverInfo.stRcvMsgInfo.stScAddr,
                                                    (pgucAtSndCodeAddr + usLength));

            /* <tosca> */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",%d",
                                               pstEvent->u.stDeliverInfo.stRcvMsgInfo.stScAddr.addrType);

            /* <length> */
            usLength += AT_PrintSmsLength(pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                          pstTsDataInfo->u.stDeliver.stUserData.ulLen,
                                          (pgucAtSndCodeAddr + usLength));
        }

        /* <data> �п��ܵõ���UCS2������ϸ����*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s",
                                           gaucAtCrLf);

        usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN,
                                                   (TAF_INT8 *)pgucAtSndCodeAddr,
                                                   pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                                   (pgucAtSndCodeAddr + usLength),
                                                   pstTsDataInfo->u.stDeliver.stUserData.aucOrgData,
                                                   (TAF_UINT16)pstTsDataInfo->u.stDeliver.stUserData.ulLen);
    }
    else
    {
        /* +CMT: [<alpha>],<length><CR><LF><pdu> */
        usLength += (TAF_UINT16)At_MsgPduInd(&pstEvent->u.stDeliverInfo.stRcvMsgInfo.stScAddr,
                                             &pstEvent->u.stDeliverInfo.stRcvMsgInfo.stTsRawData,
                                             (pgucAtSndCodeAddr + usLength));
    }
    *pusSendLength = usLength;
}

#if (FEATURE_BASTET == FEATURE_ON)

TAF_VOID At_ForwardMsgToTeInBst(
    TAF_UINT8                            ucIndex,
    TAF_UINT16                          *pusSendLength,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT16                          usLength = *pusSendLength;
    MN_MSG_BCD_ADDR_STRU               *pstScAddr = VOS_NULL_PTR;
    MN_MSG_RAW_TS_DATA_STRU            *pstPdu = VOS_NULL_PTR;
    TAF_UINT8                           ucBlacklistFlag;

    pstScAddr = &pstEvent->u.stDeliverInfo.stRcvMsgInfo.stScAddr;
    pstPdu    = &pstEvent->u.stDeliverInfo.stRcvMsgInfo.stTsRawData;

    /* ^BST��ʽ�ϱ��������� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^BST: ");

    /* <length> */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       ",%d",
                                       pstPdu->ulLen);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           gaucAtCrLf);

    /*��������ʶ�ֶ�,�ڶ���PDU���ײ�����һ���ֽڣ�д���������ű�ʶ��Ϣ��Ĭ��ֵ255*/
    ucBlacklistFlag = 0xFF;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%X%X",
                                       ((ucBlacklistFlag & 0xf0) >> 4),
                                       (ucBlacklistFlag & 0x0f));

    /*SCA �������ĵ�ַ*/
    if (pstScAddr->ucBcdLen == 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "00");
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%X%X%X%X",
                                           (((pstScAddr->ucBcdLen + 1) & 0xf0) >> 4),
                                           ((pstScAddr->ucBcdLen + 1) & 0x0f),
                                           ((pstScAddr->addrType & 0xf0) >> 4),
                                           (pstScAddr->addrType & 0x0f));

        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        pgucAtSndCodeAddr + usLength,
                                                        pstScAddr->aucBcdNum,
                                                        (TAF_UINT16)pstScAddr->ucBcdLen);
    }

    /*��������*/
    usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                    (TAF_INT8 *)pgucAtSndCodeAddr,
                                                    pgucAtSndCodeAddr + usLength,
                                                    pstPdu->aucData,
                                                    (TAF_UINT16)pstPdu->ulLen);

    *pusSendLength = usLength;
}



TAF_VOID AT_BlackSmsReport(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent,
    MN_MSG_TS_DATA_INFO_STRU           *pstTsDataInfo
)
{
    TAF_UINT16                          usLength;

    usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       "%s",
                                       gaucAtCrLf);

    /*bst�ķ�ʽ�ϱ�*/
    At_ForwardMsgToTeInBst(ucIndex, &usLength,pstTsDataInfo,pstEvent);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                   "%s",
                                   gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
}
#endif


TAF_VOID  AT_ForwardDeliverMsgToTe(
    MN_MSG_EVENT_INFO_STRU              *pstEvent,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo
)
{
    TAF_BOOL                            bCmtiInd;
    TAF_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstEvent->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ForwardDeliverMsgToTe: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);


    AT_LOG1("AT_ForwardDeliverMsgToTe: current mt is", pstSmsCtx->stCnmiType.CnmiMtType);

    if (pstSmsCtx->stCnmiType.CnmiMtType == AT_CNMI_MT_NO_SEND_TYPE)
    {
        return;
    }

    bCmtiInd = TAF_FALSE;
    if ((pstEvent->u.stDeliverInfo.enRcvSmAct == MN_MSG_RCVMSG_ACT_STORE)
     && (pstEvent->u.stDeliverInfo.enMemStore != MN_MSG_MEM_STORE_NONE))
    {
        if ((pstSmsCtx->stCnmiType.CnmiMtType == AT_CNMI_MT_CMTI_TYPE)
         || (pstSmsCtx->stCnmiType.CnmiMtType == AT_CNMI_MT_CLASS3_TYPE))
        {
            bCmtiInd = TAF_TRUE;
        }

        if (pstTsDataInfo->u.stDeliver.stDcs.enMsgClass == MN_MSG_MSG_CLASS_2)
        {
            bCmtiInd = TAF_TRUE;
        }
    }

#if (FEATURE_BASTET == FEATURE_ON)
    if ( pstEvent->u.stDeliverInfo.ucBlackRptFlag == VOS_TRUE )
    {
        AT_BlackSmsReport(ucIndex, pstEvent, pstTsDataInfo);
        return;
    }
#endif

    usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       "%s",
                                       gaucAtCrLf);

    /*����MT���úͽ��յ��¼���CLASS���͵õ����յ��¼��ϱ���ʽ:
    Э��Ҫ��MTΪ3ʱCLASS���ͻ�ȡʵ��MT����, ��������ն˲��ϱ��¼���Э�鲻һ��*/
    if (bCmtiInd == TAF_TRUE)
    {
        /* +CMTI: <mem>,<index> */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "+CMTI: %s,%d",
                                           At_GetStrContent(At_GetSmsArea(pstEvent->u.stDeliverInfo.enMemStore)),
                                           pstEvent->u.stDeliverInfo.ulInex);
    }
    else
    {
        /*CMT�ķ�ʽ�ϱ�*/
        At_ForwardMsgToTeInCmt(ucIndex, &usLength,pstTsDataInfo,pstEvent);
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                   "%s",
                                   gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


TAF_VOID  AT_ForwardStatusReportMsgToTe(
    MN_MSG_EVENT_INFO_STRU              *pstEvent,
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo
)
{
    TAF_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstEvent->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ForwardStatusReportMsgToTe: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    AT_LOG1("AT_ForwardStatusReportMsgToTe: current ds is ", pstSmsCtx->stCnmiType.CnmiDsType);

    if (pstSmsCtx->stCnmiType.CnmiDsType == AT_CNMI_DS_NO_SEND_TYPE)
    {
        return;
    }

    usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       "%s",
                                       gaucAtCrLf);

    if ((pstEvent->u.stDeliverInfo.enRcvSmAct == MN_MSG_RCVMSG_ACT_STORE)
     && (pstEvent->u.stDeliverInfo.enMemStore != MN_MSG_MEM_STORE_NONE))
    {
        /* +CDSI: <mem>,<index> */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "+CDSI: %s,%d",
                                           At_GetStrContent(At_GetSmsArea(pstEvent->u.stDeliverInfo.enMemStore)),
                                           pstEvent->u.stDeliverInfo.ulInex);
    }
    else
    {
        /* +CDS */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "+CDS: ");
        if (pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT)
        {
            /* +CDS: <fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st> */
            /*<fo>*/
            usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pgucAtSndCodeAddr + usLength));

            /*<mr>*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",%d,",
                                               pstTsDataInfo->u.stStaRpt.ucMr);

            /*<ra>*/
            usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stStaRpt.stRecipientAddr,
                                                      (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

            /*<tora>*/
            usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stStaRpt.stRecipientAddr,
                                                     (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

            /* <scts> */
            usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stStaRpt.stTimeStamp,
                                                    (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

             /* <dt> */
             usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stStaRpt.stDischargeTime,
                                                     (pgucAtSndCodeAddr + usLength));

             /*<st>*/
             usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                ",%d",
                                                pstTsDataInfo->u.stStaRpt.enStatus);
        }
        else
        {
            /* +CDS: <length><CR><LF><pdu> */
            usLength += (VOS_UINT16)At_StaRptPduInd(&pstEvent->u.stDeliverInfo.stRcvMsgInfo.stScAddr,
                                                 &pstEvent->u.stDeliverInfo.stRcvMsgInfo.stTsRawData,
                                                 (pgucAtSndCodeAddr + usLength));
        }
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


TAF_VOID  AT_ForwardPppMsgToTe(
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet;
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo = VOS_NULL_PTR;

    pstTsDataInfo = At_GetMsgMem();

    ulRet = MN_MSG_Decode(&pstEvent->u.stDeliverInfo.stRcvMsgInfo.stTsRawData, pstTsDataInfo);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        return;
    }

    if (pstTsDataInfo->enTpduType == MN_MSG_TPDU_DELIVER)
    {
        AT_ForwardDeliverMsgToTe(pstEvent, pstTsDataInfo);
    }
    else if (pstTsDataInfo->enTpduType == MN_MSG_TPDU_STARPT)
    {
        AT_ForwardStatusReportMsgToTe(pstEvent, pstTsDataInfo);
    }
    else
    {
        AT_WARN_LOG("AT_ForwardPppMsgToTe: invalid tpdu type.");
    }

    return;

}

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))

VOS_VOID AT_ForwardCbMsgToTe(
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet;
    MN_MSG_CBPAGE_STRU                  stCbmPageInfo;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    ucIndex = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstEvent->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ForwardCbMsgToTe: WARNING:AT INDEX NOT FOUND!");
        return;
    }


    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    AT_LOG1("AT_ForwardCbMsgToTe: current bm is ", pstSmsCtx->stCnmiType.CnmiBmType);


    /*����BM���úͽ��յ��¼���CLASS���͵õ����յ��¼��ϱ���ʽ:
    ��֧��Э��Ҫ��BMΪ3ʱ,CBM���ϱ�*/

    ulRet = MN_MSG_DecodeCbmPage(&(pstEvent->u.stCbsDeliverInfo.stCbRawData), &stCbmPageInfo);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        return;
    }

    /*+CBM: <sn>,<mid>,<dcs>,<page>,<pages><CR><LF><data> (text mode enabled)*/
    /*+CBM: <length><CR><LF><pdu> (PDU mode enabled); or*/
    usLength  = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "+CBM: ");

    if (pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%d,",
                                           stCbmPageInfo.stSn.usRawSnData);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%d,",
                                           stCbmPageInfo.usMid);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%d,",
                                           stCbmPageInfo.stDcs.ucRawDcsData);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%d,",
                                           stCbmPageInfo.ucPageIndex);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%d",
                                           stCbmPageInfo.ucPageNum);

        /* <data> �п��ܵõ���UCS2������ϸ����*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s",
                                           gaucAtCrLf);

        usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN,
                                                   (TAF_INT8 *)pgucAtSndCodeAddr,
                                                   stCbmPageInfo.stDcs.enMsgCoding,
                                                   (pgucAtSndCodeAddr + usLength),
                                                   stCbmPageInfo.stContent.aucContent,
                                                   (TAF_UINT16)stCbmPageInfo.stContent.ulLen);
    }
    else
    {
        /*+CBM: <length><CR><LF><pdu> (PDU mode enabled); or*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%d",
                                           pstEvent->u.stCbsDeliverInfo.stCbRawData.ulLen);

        /* <data> �п��ܵõ���UCS2������ϸ����*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s",
                                           gaucAtCrLf);

        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        (pgucAtSndCodeAddr + usLength),
                                                        pstEvent->u.stCbsDeliverInfo.stCbRawData.aucData,
                                                        (TAF_UINT16)pstEvent->u.stCbsDeliverInfo.stCbRawData.ulLen);
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    /* ���˶Ա�,�˴������һ������ */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);


    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
}

#endif

TAF_VOID At_ForwardMsgToTe(
    MN_MSG_EVENT_ENUM_U32               enEvent,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{

    AT_LOG1("At_ForwardMsgToTe: current Event is ", enEvent);

    switch (enEvent)
    {
    case MN_MSG_EVT_DELIVER:
        AT_ForwardPppMsgToTe(pstEvent);
        break;

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    case MN_MSG_EVT_DELIVER_CBM:
        AT_ForwardCbMsgToTe(pstEvent);
        break;
#endif
    default:
        AT_WARN_LOG("At_SendSmtInd: invalid tpdu type.");
        break;
    }
    return;
}


TAF_VOID At_HandleSmtBuffer(
    VOS_UINT8                           ucIndex,
    AT_CNMI_BFR_TYPE                    ucBfrType
)
{
    TAF_UINT8                           ucLoop;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (ucBfrType == AT_CNMI_BFR_SEND_TYPE)
    {
        for (ucLoop = 0; ucLoop < AT_BUFFER_SMT_EVENT_MAX; ucLoop ++)
        {
            if (pstSmsCtx->stSmtBuffer.aucUsed[ucLoop] == AT_MSG_BUFFER_USED)
            {
                At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, &(pstSmsCtx->stSmtBuffer.astEvent[ucLoop]));
            }
        }

    }

    memset_s(&(pstSmsCtx->stSmtBuffer), sizeof(pstSmsCtx->stSmtBuffer), 0x00, sizeof(pstSmsCtx->stSmtBuffer));

    return;
}


VOS_VOID AT_FlushSmsIndication(VOS_VOID)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;
    AT_PORT_BUFF_CFG_STRU              *pstPortBuffCfg = VOS_NULL_PTR;
    VOS_UINT16                          i;
    VOS_UINT32                          ulClientId;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           aucModemFlag[MODEM_ID_BUTT];
    VOS_UINT32                          j;

    j   = 0;

    pstPortBuffCfg = AT_GetPortBuffCfgInfo();
    memset_s(aucModemFlag, sizeof(aucModemFlag), 0x00, sizeof(aucModemFlag));

    if (pstPortBuffCfg->ucNum > AT_MAX_CLIENT_NUM)
    {
        pstPortBuffCfg->ucNum = AT_MAX_CLIENT_NUM;
    }

    /* ����clientId������Ҫflush ����modem id */
    for (i = 0; i < pstPortBuffCfg->ucNum; i++)
    {
        ulClientId = pstPortBuffCfg->ulUsedClientID[i];
        ulRslt = AT_GetModemIdFromClient((VOS_UINT8)ulClientId, &enModemId);
        if (ulRslt != VOS_OK)
        {
            AT_ERR_LOG("AT_FlushSmsIndication: Get modem id fail");
            continue;
        }

        aucModemFlag[enModemId] = VOS_TRUE;
    }

    /* flush SMS */
    for (i = 0; i < MODEM_ID_BUTT; i++)
    {
        if (aucModemFlag[i] == VOS_TRUE)
        {
            pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(i);

            for (j = 0; j < AT_BUFFER_SMT_EVENT_MAX; j ++)
            {
                if (pstSmsCtx->stSmtBuffer.aucUsed[j] == AT_MSG_BUFFER_USED)
                {
                    At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, &(pstSmsCtx->stSmtBuffer.astEvent[j]));
                }
            }

            memset_s(&(pstSmsCtx->stSmtBuffer), sizeof(pstSmsCtx->stSmtBuffer), 0x00, sizeof(pstSmsCtx->stSmtBuffer));
        }
    }

    return;
}



TAF_VOID At_SmsModSmStatusRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    VOS_UINT32                          ulRet;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SmsModSmStatusRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    if (pstEvent->u.stModifyInfo.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stDeleteInfo.ulFailCause);
    }
    else
    {
        ulRet = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulRet);
    return;
}


TAF_VOID At_SmsInitResultProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pEvent
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    pstSmsCtx->stCpmsInfo.stUsimStorage.ulTotalRec = pEvent->u.stInitResultInfo.ulTotalSmRec;
    pstSmsCtx->stCpmsInfo.stUsimStorage.ulUsedRec = pEvent->u.stInitResultInfo.ulUsedSmRec;

    return;
}


VOS_VOID At_SmsDeliverErrProc(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if ((pstSmsCtx->ucLocalStoreFlg == VOS_TRUE)
     && (pstEvent->u.stDeliverErrInfo.enErrorCode == TAF_MSG_ERROR_TR2M_TIMEOUT))
    {
        pstSmsCtx->stCnmiType.CnmiMtType            = AT_CNMI_MT_NO_SEND_TYPE;
        pstSmsCtx->stCnmiType.CnmiDsType            = AT_CNMI_DS_NO_SEND_TYPE;
        AT_WARN_LOG("At_SmsDeliverErrProc: CnmiMtType and CnmiDsType changed!");
    }

    /* ���Ž���������Ϊд����ʧ�ܲ����ϱ��¼���AT���Ҹ��¼���ERROR LOG��¼����Ҫ�ϱ���Ӧ�ô��� */

    return;
}


VOS_VOID At_SmsInitSmspResultProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;
    errno_t                             lMemResult;

    VOS_UINT8                           ucDefaultIndex;

    ucDefaultIndex = pstEvent->u.stInitSmspResultInfo.ucDefaultSmspIndex;

    AT_NORM_LOG1("At_SmsInitSmspResultProc: ucDefaultIndex", ucDefaultIndex);

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (ucDefaultIndex >= MN_MSG_MAX_USIM_EFSMSP_NUM)
    {
        ucDefaultIndex = AT_CSCA_CSMP_STORAGE_INDEX;
    }

    /* ��¼defaultSmspIndex, ��csca csmp ��ʹ�� */
    pstSmsCtx->stCscaCsmpInfo.ucDefaultSmspIndex = ucDefaultIndex;

    lMemResult = memcpy_s(&(pstSmsCtx->stCscaCsmpInfo.stParmInUsim),
                          sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim),
                          &pstEvent->u.stInitSmspResultInfo.astSrvParm[ucDefaultIndex],
                          sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim), sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim));

    lMemResult = memcpy_s(&(pstSmsCtx->stCpmsInfo.stRcvPath),
                          sizeof(pstSmsCtx->stCpmsInfo.stRcvPath),
                          &pstEvent->u.stInitSmspResultInfo.stRcvMsgPath,
                          sizeof(pstSmsCtx->stCpmsInfo.stRcvPath));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCpmsInfo.stRcvPath), sizeof(pstSmsCtx->stCpmsInfo.stRcvPath));

    g_enClass0Tailor = pstEvent->u.stInitSmspResultInfo.enClass0Tailor;

    return;
}


VOS_VOID At_SmsSrvParmChangeProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;
    errno_t                             lMemResult;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    lMemResult = memcpy_s(&(pstSmsCtx->stCscaCsmpInfo.stParmInUsim),
                          sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim),
                          &pstEvent->u.stSrvParmChangeInfo.astSrvParm[pstSmsCtx->stCscaCsmpInfo.ucDefaultSmspIndex],
                          sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim), sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim));

    return;
}


VOS_VOID At_SmsRcvMsgPathChangeProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    pstSmsCtx->stCpmsInfo.stRcvPath.enRcvSmAct = pstEvent->u.stRcvMsgPathInfo.enRcvSmAct;
    pstSmsCtx->stCpmsInfo.stRcvPath.enSmMemStore = pstEvent->u.stRcvMsgPathInfo.enSmMemStore;
    pstSmsCtx->stCpmsInfo.stRcvPath.enRcvStaRptAct = pstEvent->u.stRcvMsgPathInfo.enRcvStaRptAct;
    pstSmsCtx->stCpmsInfo.stRcvPath.enStaRptMemStore = pstEvent->u.stRcvMsgPathInfo.enStaRptMemStore;

    return;
}


VOS_VOID AT_ReportSmMeFull(
    VOS_UINT8                           ucIndex,
    MN_MSG_MEM_STORE_ENUM_U8            enMemStore
)
{
    VOS_UINT16 usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%s",
                                       gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "^SMMEMFULL: ");

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)(pgucAtSndCodeAddr + usLength),
                                       "%s",
                                       At_GetStrContent(At_GetSmsArea(enMemStore)));
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}

VOS_VOID At_SmsStorageListProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    MN_MSG_STORAGE_LIST_EVT_INFO_STRU  *pstStorageListInfo = VOS_NULL_PTR;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;
    errno_t                             lMemResult;

    MN_MSG_MT_CUSTOMIZE_ENUM_UINT8      enMtCustomize;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    pstStorageListInfo = &pstEvent->u.stStorageListInfo;
    if (pstStorageListInfo->enMemStroe == MN_MSG_MEM_STORE_SIM)
    {
        lMemResult = memcpy_s(&(pstSmsCtx->stCpmsInfo.stUsimStorage),
                              sizeof(pstSmsCtx->stCpmsInfo.stUsimStorage),
                              &pstEvent->u.stStorageListInfo,
                              sizeof(pstSmsCtx->stCpmsInfo.stUsimStorage));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCpmsInfo.stUsimStorage), sizeof(pstSmsCtx->stCpmsInfo.stUsimStorage));
    }
    else
    {
        lMemResult = memcpy_s(&(pstSmsCtx->stCpmsInfo.stNvimStorage),
                              sizeof(pstSmsCtx->stCpmsInfo.stNvimStorage),
                              &pstEvent->u.stStorageListInfo,
                              sizeof(pstSmsCtx->stCpmsInfo.stNvimStorage));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCpmsInfo.stNvimStorage), sizeof(pstSmsCtx->stCpmsInfo.stNvimStorage));
    }

    enMtCustomize = pstSmsCtx->stSmMeFullCustomize.enMtCustomize;
    if ((enMtCustomize == MN_MSG_MT_CUSTOMIZE_FT)
     && (pstEvent->u.stStorageListInfo.ulTotalRec == pstEvent->u.stStorageListInfo.ulUsedRec)
     && (pstEvent->u.stStorageStateInfo.enMemStroe == MN_MSG_MEM_STORE_SIM))
    {
        AT_INFO_LOG("At_SmsStorageListProc: FT memory full.");
        AT_ReportSmMeFull(ucIndex, pstEvent->u.stStorageStateInfo.enMemStroe);
    }

    if (!AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {

        /* ���յ�NV�Ķ��������ϱ����޸�NV�Ķ��������ȴ���־�ѽ��յ�NV�Ķ������� */
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
        if (pstStorageListInfo->enMemStroe == MN_MSG_MEM_STORE_ME)
        {
            gastAtClientTab[ucIndex].AtSmsData.bWaitForNvStorageStatus = TAF_FALSE;
        }
#endif
        /* ���յ�SIM�Ķ��������ϱ����޸�SIM�Ķ��������ȴ���־�ѽ��յ�SIM�Ķ������� */
        if (pstStorageListInfo->enMemStroe == MN_MSG_MEM_STORE_SIM)
        {
            gastAtClientTab[ucIndex].AtSmsData.bWaitForUsimStorageStatus = TAF_FALSE;
        }

        /* CPMS�����ò�����Ҫ�ȴ�����������Ϣ��������Ӧ��Ϣ����� */
        if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPMS_SET)
        {
            if ((gastAtClientTab[ucIndex].AtSmsData.bWaitForCpmsSetRsp == TAF_FALSE)
             && (gastAtClientTab[ucIndex].AtSmsData.bWaitForNvStorageStatus == TAF_FALSE)
             && (gastAtClientTab[ucIndex].AtSmsData.bWaitForUsimStorageStatus == TAF_FALSE))
            {
                AT_STOP_TIMER_CMD_READY(ucIndex);
                At_PrintSetCpmsRsp(ucIndex);
            }
        }

        /* CPMS�Ķ�ȡ������Ҫ�ȴ�����������Ϣ����� */
        if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPMS_READ)
        {
            if ((gastAtClientTab[ucIndex].AtSmsData.bWaitForNvStorageStatus == TAF_FALSE)
             && (gastAtClientTab[ucIndex].AtSmsData.bWaitForUsimStorageStatus == TAF_FALSE))
            {
                AT_STOP_TIMER_CMD_READY(ucIndex);
                At_PrintGetCpmsRsp(ucIndex);
            }
        }
    }

    return;
}


VOS_VOID At_SmsStorageExceedProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    /* ��ʼ�� */
    enModemId       = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_SmsStorageExceedProc: Get modem id fail.");
        return;
    }

    AT_ReportSmMeFull(ucIndex, pstEvent->u.stStorageStateInfo.enMemStroe);

    return;
}


VOS_VOID At_SmsDeliverProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT16                          usLength            = 0;
    TAF_UINT32                          ulRet;
    MN_MSG_TS_DATA_INFO_STRU            *pstTsDataInfo = VOS_NULL_PTR;
    VOS_UINT8                           ucUserId;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    ucUserId = AT_BROADCAST_CLIENT_INDEX_MODEM_0;

    /* ͨ��ClientId��ȡucUserId */
    if ( At_ClientIdToUserId(pstEvent->clientId, &ucUserId) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_SmsDeliverProc: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucUserId);

    /* ��ǰ��������ΪCLass0�Ҷ��Ŷ���Ϊ
    1:H3G�������TIM Class 0����������ͬ�����Զ��Ž��д洢��Ҫ��CLASS 0
    ����ֱ�Ӳ���+CMT���������ϱ�������CNMI�Լ�CPMS���õ�Ӱ�죬�����̨�Ѿ�
    �򿪣����̨��CLASS 0���Ž�����ʾ��
    CLass0�Ķ��Ŵ�ʱ������MT,MODE�Ĳ���
    */

    pstTsDataInfo = At_GetMsgMem();
    ulRet = MN_MSG_Decode(&pstEvent->u.stDeliverInfo.stRcvMsgInfo.stTsRawData, pstTsDataInfo);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        return;
    }

    AT_StubSaveAutoReplyData(ucUserId, pstEvent, pstTsDataInfo);

#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_SmsStartRingTe(VOS_TRUE);
#endif

    if ( (pstTsDataInfo->enTpduType == MN_MSG_TPDU_DELIVER)
      && (pstTsDataInfo->u.stDeliver.stDcs.enMsgClass == MN_MSG_MSG_CLASS_0)
      && (g_enClass0Tailor != MN_MSG_CLASS0_DEF))
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

        if ((g_enClass0Tailor == MN_MSG_CLASS0_TIM)
         || (g_enClass0Tailor == MN_MSG_CLASS0_VIVO))
        {
            /*+CMT��ʽ�ϱ� */
            At_ForwardMsgToTeInCmt(ucUserId, &usLength,pstTsDataInfo,pstEvent);
        }
        else
        {
            /* +CMTI: <mem>,<index> */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "+CMTI: %s,%d",
                                               At_GetStrContent(At_GetSmsArea(pstEvent->u.stDeliverInfo.enMemStore)),
                                               pstEvent->u.stDeliverInfo.ulInex);
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           gaucAtCrLf);

        At_SendResultData(ucUserId, pgucAtSndCodeAddr, usLength);

        return;
    }

    if (pstSmsCtx->stCnmiType.CnmiModeType == AT_CNMI_MODE_SEND_OR_DISCARD_TYPE)
    {
        At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, pstEvent);
        return;
    }

    /* ��ģʽΪ0ʱ���� */
    if (pstSmsCtx->stCnmiType.CnmiModeType == AT_CNMI_MODE_BUFFER_TYPE)
    {
        At_BufferMsgInTa(ucIndex, MN_MSG_EVT_DELIVER, pstEvent);
        return;
    }

    /* ��ģʽΪ2ʱ���� */
    if (pstSmsCtx->stCnmiType.CnmiModeType == AT_CNMI_MODE_SEND_OR_BUFFER_TYPE)
    {
        /* �ж��Ƿ�߱���������� */
        if (AT_IsClientBlock() == VOS_TRUE)
        {
            At_BufferMsgInTa(ucIndex, MN_MSG_EVT_DELIVER, pstEvent);
        }
        else
        {
            At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, pstEvent);
        }
        return;
    }

    /*Ŀǰ��֧�� AT_CNMI_MODE_EMBED_AND_SEND_TYPE*/

    return;
}


TAF_VOID At_SetRcvPathRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetRcvPathRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (pstEvent->u.stRcvMsgPathInfo.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stSrvParmInfo.ulFailCause);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPMS_SET)
    {
        /*������ʱ���ݵ��ڴ��NVIM*/
        pstSmsCtx->stCpmsInfo.stRcvPath.enSmMemStore = pstEvent->u.stRcvMsgPathInfo.enSmMemStore;
        pstSmsCtx->stCpmsInfo.stRcvPath.enStaRptMemStore = pstEvent->u.stRcvMsgPathInfo.enStaRptMemStore;
        pstSmsCtx->stCpmsInfo.enMemReadorDelete = pstSmsCtx->stCpmsInfo.enTmpMemReadorDelete;
        pstSmsCtx->stCpmsInfo.enMemSendorWrite = pstSmsCtx->stCpmsInfo.enTmpMemSendorWrite;

        gastAtClientTab[ucIndex].AtSmsData.bWaitForCpmsSetRsp = TAF_FALSE;

        /* CPMS�����ò�����Ҫ�ȴ�����������Ϣ��������Ӧ��Ϣ����� */
        if ((gastAtClientTab[ucIndex].AtSmsData.bWaitForCpmsSetRsp == TAF_FALSE)
         && (gastAtClientTab[ucIndex].AtSmsData.bWaitForNvStorageStatus == TAF_FALSE)
         && (gastAtClientTab[ucIndex].AtSmsData.bWaitForUsimStorageStatus == TAF_FALSE))
        {
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_PrintSetCpmsRsp(ucIndex);
        }

    }
    else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CSMS_SET)
    {
        /* ִ��������� */
        pstSmsCtx->enCsmsMsgVersion                      = pstEvent->u.stRcvMsgPathInfo.enSmsServVersion;
        pstSmsCtx->stCpmsInfo.stRcvPath.enSmsServVersion = pstSmsCtx->enCsmsMsgVersion;

        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)(pgucAtSndCodeAddr + gstAtSendData.usBufLen),
                                                        "%s: ",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        At_PrintCsmsInfo(ucIndex);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_OK);
    }
    else/*AT_CMD_CNMI_SET*/
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);

        pstSmsCtx->stCnmiType.CnmiBfrType = pstSmsCtx->stCnmiType.CnmiTmpBfrType;
        pstSmsCtx->stCnmiType.CnmiDsType = pstSmsCtx->stCnmiType.CnmiTmpDsType;
        pstSmsCtx->stCnmiType.CnmiBmType = pstSmsCtx->stCnmiType.CnmiTmpBmType;
        pstSmsCtx->stCnmiType.CnmiMtType = pstSmsCtx->stCnmiType.CnmiTmpMtType;
        pstSmsCtx->stCnmiType.CnmiModeType = pstSmsCtx->stCnmiType.CnmiTmpModeType;
        pstSmsCtx->stCpmsInfo.stRcvPath.enRcvSmAct = pstEvent->u.stRcvMsgPathInfo.enRcvSmAct;
        pstSmsCtx->stCpmsInfo.stRcvPath.enRcvStaRptAct = pstEvent->u.stRcvMsgPathInfo.enRcvStaRptAct;

        if (pstSmsCtx->stCnmiType.CnmiModeType != 0)
        {
            At_HandleSmtBuffer(ucIndex, pstSmsCtx->stCnmiType.CnmiBfrType);
        }

    }
    return;
}


TAF_VOID At_SetCscaCsmpRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulRet;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetCscaCsmpRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if (pstEvent->u.stSrvParmInfo.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stSrvParmInfo.ulFailCause);
    }
    else
    {
        if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CSMP_SET)
        {
            lMemResult = memcpy_s(&(pstSmsCtx->stCscaCsmpInfo.stVp), sizeof(pstSmsCtx->stCscaCsmpInfo.stVp), &(pstSmsCtx->stCscaCsmpInfo.stTmpVp), sizeof(pstSmsCtx->stCscaCsmpInfo.stVp));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCscaCsmpInfo.stVp), sizeof(pstSmsCtx->stCscaCsmpInfo.stVp));
            pstSmsCtx->stCscaCsmpInfo.ucFo = pstSmsCtx->stCscaCsmpInfo.ucTmpFo;
            pstSmsCtx->stCscaCsmpInfo.bFoUsed = TAF_TRUE;
        }
        lMemResult = memcpy_s(&(pstSmsCtx->stCscaCsmpInfo.stParmInUsim),
                              sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim),
                              &pstEvent->u.stSrvParmInfo.stSrvParm,
                              sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim), sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim));
        ulRet = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRet);
    return;
}


TAF_VOID  At_DeleteRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet;
    MN_MSG_DELETE_PARAM_STRU            stDelete;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_DeleteRspProc : AT_BROADCAST_INDEX.");
        return;
    }


    memset_s(&stDelete, sizeof(stDelete), 0x00, sizeof(stDelete));


    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMGD_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CBMGD_SET))
    {
        return;
    }

    stDelete.enMemStore = pstEvent->u.stDeleteInfo.enMemStore;
    stDelete.ulIndex = pstEvent->u.stDeleteInfo.ulIndex;
    if (pstEvent->u.stDeleteInfo.bSuccess != TAF_TRUE)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stDeleteInfo.ulFailCause);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    if (pstEvent->u.stDeleteInfo.enDeleteType == MN_MSG_DELETE_SINGLE)
    {
        gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes ^= AT_MSG_DELETE_SINGLE;
    }

    if (pstEvent->u.stDeleteInfo.enDeleteType == MN_MSG_DELETE_ALL)
    {
        gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes ^= AT_MSG_DELETE_ALL;
    }

    if (pstEvent->u.stDeleteInfo.enDeleteType == MN_MSG_DELETE_READ)
    {
        gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes ^= AT_MSG_DELETE_READ;
    }

    if (pstEvent->u.stDeleteInfo.enDeleteType == MN_MSG_DELETE_SENT)
    {
        gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes ^= AT_MSG_DELETE_SENT;
    }

    if (pstEvent->u.stDeleteInfo.enDeleteType == MN_MSG_DELETE_NOT_SENT)
    {
        gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes ^= AT_MSG_DELETE_UNSENT;
    }

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes == 0)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_MsgDeleteCmdProc(ucIndex,
                            gastAtClientTab[ucIndex].opId,
                            stDelete,
                            gastAtClientTab[ucIndex].AtSmsData.ucMsgDeleteTypes);
    }
    return;
}


VOS_VOID AT_QryCscaRspProc(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent
)
{
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulRet;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_QryCscaRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    /* ATģ���ڵȴ�CSCA��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSCA_READ)
    {
        return;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->u.stSrvParmInfo.bSuccess == VOS_TRUE)
    {
        /* ���¶������ĺ��뵽ATģ�飬���MSGģ���ʼ������¼��ϱ�ʱATģ��δ�������� */
        lMemResult = memcpy_s(&(pstSmsCtx->stCscaCsmpInfo.stParmInUsim.stScAddr),
                              sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim.stScAddr),
                              &pstEvent->u.stSrvParmInfo.stSrvParm.stScAddr,
                              sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim.stScAddr));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim.stScAddr), sizeof(pstSmsCtx->stCscaCsmpInfo.stParmInUsim.stScAddr));

        /* ���ô�����ΪAT_OK           ����ṹΪ+CSCA: <sca>,<toda>��ʽ�Ķ��� */
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           "%s: ",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /*�������ĺ������ָʾΪ�����Ҷ������ĺ��볤�Ȳ�Ϊ0*/
        if (((pstEvent->u.stSrvParmInfo.stSrvParm.ucParmInd & MN_MSG_SRV_PARM_MASK_SC_ADDR) == 0)
         && (pstEvent->u.stSrvParmInfo.stSrvParm.stScAddr.ucBcdLen != 0))
        {
            /*��SCA��ַ��BCD��ת��ΪASCII��*/
            usLength += At_PrintBcdAddr(&pstEvent->u.stSrvParmInfo.stSrvParm.stScAddr,
                                        (pgucAtSndCodeAddr + usLength));

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",%d",
                                               pstEvent->u.stSrvParmInfo.stSrvParm.stScAddr.addrType);
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"\",128");
        }

        gstAtSendData.usBufLen = usLength;
        ulRet                  = AT_OK;
    }
    else
    {
        /* ����pstEvent->u.stSrvParmInfo.ulFailCause����At_ChgMnErrCodeToAtת����ATģ��Ĵ����� */
        gstAtSendData.usBufLen = 0;
        ulRet                  = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stSrvParmInfo.ulFailCause);

    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);

    return;
}


VOS_VOID At_SmsStubRspProc(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT32                          ulRet;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SmsStubRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    /* ATģ���ڵȴ�CMSTUB����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMSTUB_SET)
    {
        return;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->u.stResult.ulErrorCode == MN_ERR_NO_ERROR)
    {
        ulRet = AT_OK;
    }
    else
    {
        ulRet = AT_CMS_UNKNOWN_ERROR;
    }

    /* ����At_FormatResultData������ */
    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex, ulRet);
    return;
}


VOS_UINT32 AT_GetBitMap(
    VOS_UINT32                         *pulBitMap,
    VOS_UINT32                          ulIndex
)
{
    VOS_UINT8                           ucX;
    VOS_UINT32                          ulY;
    VOS_UINT32                          ulMask;

    ulY = ulIndex/32;
    ucX = (VOS_UINT8)ulIndex%32;
    ulMask = ((VOS_UINT32)1 << ucX);
    if ((pulBitMap[ulY] & ulMask) != 0)
    {
        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }
}


VOS_VOID AT_SmsListIndex(
    VOS_UINT16                          usLength,
    MN_MSG_DELETE_TEST_EVT_INFO_STRU   *pstPara,
    VOS_UINT16                         *pusPrintOffSet
)
{
    TAF_UINT32                          ulLoop;
    TAF_UINT32                          ulMsgNum;

    ulMsgNum  = 0;

    for (ulLoop = 0; ulLoop < pstPara->ulSmCapacity; ulLoop++)
    {
        if (AT_GetBitMap(pstPara->aulValidLocMap, ulLoop) == TAF_TRUE)
        {
            ulMsgNum++;
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d,", ulLoop);
        }

    }

    /* ɾ�����һ��"," */
    if (ulMsgNum != 0)
    {
        usLength -= 1;
    }

    *pusPrintOffSet = usLength;

    return;
}


TAF_VOID  At_DeleteTestRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT16                          usLength;
    MN_MSG_DELETE_TEST_EVT_INFO_STRU   *pstPara = VOS_NULL_PTR;
    VOS_UINT32                          ulLoop;
    VOS_BOOL                            bMsgExist;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_DeleteTestRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstPara = (MN_MSG_DELETE_TEST_EVT_INFO_STRU *)&pstEvent->u.stDeleteTestInfo;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CMGD_TEST)
    {
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          "%s: (",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        AT_SmsListIndex(usLength, pstPara, &usLength);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "),(0-4)");
    }
    else
    {
        /* �ж��Ƿ��ж��������б����: �޶�����Ҫ���ֱ�ӷ���OK */
        bMsgExist = VOS_FALSE;

        for (ulLoop = 0; ulLoop < MN_MSG_CMGD_PARA_MAX_LEN; ulLoop++)
        {
            if (pstPara->aulValidLocMap[ulLoop] != 0)
            {
                bMsgExist = VOS_TRUE;
                break;
            }
        }

        if (bMsgExist == VOS_TRUE)
        {
            usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (TAF_CHAR *)pgucAtSndCodeAddr,
                                              (TAF_CHAR *)pgucAtSndCodeAddr,
                                              "%s: ",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

            AT_SmsListIndex(usLength, pstPara, &usLength);
        }
        else
        {
            usLength = 0;
        }
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    At_FormatResultData(ucIndex,AT_OK);

    return;
}


TAF_VOID At_ReadRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT16                          usLength            = 0;
    TAF_UINT32                          ulRet               = AT_OK;
    MN_MSG_TS_DATA_INFO_STRU           *pstTsDataInfo = VOS_NULL_PTR;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ReadRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (pstEvent->u.stReadInfo.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stReadInfo.ulFailCause);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (TAF_UINT16)At_SmsPrintState(pstSmsCtx->enCmgfMsgFormat,
                                             pstEvent->u.stReadInfo.enStatus,
                                             (pgucAtSndCodeAddr + usLength));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       ",");

    pstTsDataInfo = At_GetMsgMem();
    ulRet = MN_MSG_Decode(&pstEvent->u.stReadInfo.stMsgInfo.stTsRawData, pstTsDataInfo);
    if (ulRet != MN_ERR_NO_ERROR)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, ulRet);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    if (pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_PDU)/*PDU*/
    {
        /* +CMGR: <stat>,[<alpha>],<length><CR><LF><pdu> */
        usLength += (TAF_UINT16)At_MsgPduInd(&pstEvent->u.stReadInfo.stMsgInfo.stScAddr,
                                             &pstEvent->u.stReadInfo.stMsgInfo.stTsRawData,
                                             (pgucAtSndCodeAddr + usLength));

        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, AT_OK);
        return;
    }

    switch (pstEvent->u.stReadInfo.stMsgInfo.stTsRawData.enTpduType)
    {
        case MN_MSG_TPDU_DELIVER:
            /* +CMGR: <stat>,<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>, <sca>,<tosca>,<length>]<CR><LF><data>*/
            /* <oa> */
            usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stDeliver.stOrigAddr,
                                                      (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");
            /* <alpha> ���� */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

            /* <scts> */
            usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stDeliver.stTimeStamp,
                                                    (pgucAtSndCodeAddr + usLength));

            if (pstSmsCtx->ucCsdhType == AT_CSDH_SHOW_TYPE)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",");
                /* <tooa> */
                usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stDeliver.stOrigAddr,
                                                         (pgucAtSndCodeAddr + usLength));
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",");

                /*<fo>*/
                usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pgucAtSndCodeAddr + usLength));

                /* <pid> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d",
                                                   pstTsDataInfo->u.stDeliver.enPid);

                /* <dcs> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d,",
                                                   pstTsDataInfo->u.stDeliver.stDcs.ucRawDcsData);

                /* <sca> */
                usLength += (TAF_UINT16)At_PrintBcdAddr(&pstEvent->u.stReadInfo.stMsgInfo.stScAddr,
                                                       (pgucAtSndCodeAddr + usLength));

                /* <tosca> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                                  (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                  ",%d",
                                                  pstEvent->u.stReadInfo.stMsgInfo.stScAddr.addrType);

                /* <length> */
                usLength += AT_PrintSmsLength(pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                              pstTsDataInfo->u.stDeliver.stUserData.ulLen,
                                              (pgucAtSndCodeAddr + usLength));

            }
            /* <data> �п��ܵõ���UCS2������ϸ����*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               "%s",
                                               gaucAtCrLf);

            usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN,
                                                      (TAF_INT8 *)pgucAtSndCodeAddr,
                                                       pstTsDataInfo->u.stDeliver.stDcs.enMsgCoding,
                                                       (pgucAtSndCodeAddr + usLength),
                                                       pstTsDataInfo->u.stDeliver.stUserData.aucOrgData,
                                                       (TAF_UINT16)pstTsDataInfo->u.stDeliver.stUserData.ulLen);

            break;
        case MN_MSG_TPDU_SUBMIT:
            /*+CMGR: <stat>,<da>,[<alpha>][,<toda>,<fo>,<pid>,<dcs>,[<vp>], <sca>,<tosca>,<length>]<CR><LF><data>*/
            /* <da> */
            usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stSubmit.stDestAddr,
                                                      (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");
            /* <alpha> ���� */

            if (pstSmsCtx->ucCsdhType == AT_CSDH_SHOW_TYPE)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",");

                /* <toda> */
                usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stSubmit.stDestAddr,
                                                         (pgucAtSndCodeAddr + usLength));
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",");

                /*<fo>*/
                usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pgucAtSndCodeAddr + usLength));

                /* <pid> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d",
                                                   pstTsDataInfo->u.stSubmit.enPid);
                /* <dcs> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d,",
                                                   pstTsDataInfo->u.stSubmit.stDcs.ucRawDcsData);
                /* <vp>,����Ҫ��ϸ���� */
                usLength += At_MsgPrintVp(&pstTsDataInfo->u.stSubmit.stValidPeriod,
                                          (pgucAtSndCodeAddr + usLength));
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",");

                /* <sca> */
                usLength += At_PrintBcdAddr(&pstEvent->u.stReadInfo.stMsgInfo.stScAddr,
                                            (pgucAtSndCodeAddr + usLength));

                /* <tosca> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d",
                                                   pstEvent->u.stReadInfo.stMsgInfo.stScAddr.addrType);

                /* <length> */
                usLength += AT_PrintSmsLength(pstTsDataInfo->u.stSubmit.stDcs.enMsgCoding,
                                              pstTsDataInfo->u.stSubmit.stUserData.ulLen,
                                              (pgucAtSndCodeAddr + usLength));
            }

            /* <data> �п��ܵõ���UCS2������ϸ����*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               "%s",
                                               gaucAtCrLf);

            usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN,
                                                       (TAF_INT8 *)pgucAtSndCodeAddr,
                                                       pstTsDataInfo->u.stSubmit.stDcs.enMsgCoding,
                                                       (pgucAtSndCodeAddr + usLength),
                                                       pstTsDataInfo->u.stSubmit.stUserData.aucOrgData,
                                                       (TAF_UINT16)pstTsDataInfo->u.stSubmit.stUserData.ulLen);

            break;
        case MN_MSG_TPDU_COMMAND:
            /*+CMGR: <stat>,<fo>,<ct>[,<pid>,[<mn>],[<da>],[<toda>],<length><CR><LF><cdata>]*/
            /*<fo>*/
            usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pgucAtSndCodeAddr + usLength));
            /* <ct> */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",%d",
                                               pstTsDataInfo->u.stCommand.enCmdType);

            if (pstSmsCtx->ucCsdhType == AT_CSDH_SHOW_TYPE)
            {
                /* <pid> */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d",
                                                   pstTsDataInfo->u.stCommand.enPid);

                /* <mn>,����Ҫ��ϸ���� */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d,",
                                                   pstTsDataInfo->u.stCommand.ucMsgNumber);

                /* <da> */
                usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stCommand.stDestAddr,
                                                         (pgucAtSndCodeAddr + usLength));
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",");

                /* <toda> */
                usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stCommand.stDestAddr,
                                                         (pgucAtSndCodeAddr + usLength));

                /* <length>Ϊ0 */
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   ",%d",
                                                   pstTsDataInfo->u.stCommand.ucCommandDataLen);

                /* <data> �п��ܵõ���UCS2������ϸ����*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                   "%s",
                                                   gaucAtCrLf);

                usLength += (TAF_UINT16)At_PrintReportData(AT_CMD_MAX_LEN,
                                                           (TAF_INT8 *)pgucAtSndCodeAddr,
                                                           MN_MSG_MSG_CODING_8_BIT,
                                                           (pgucAtSndCodeAddr + usLength),
                                                           pstTsDataInfo->u.stCommand.aucCmdData,
                                                           pstTsDataInfo->u.stCommand.ucCommandDataLen);

            }
            break;
        case MN_MSG_TPDU_STARPT:
            /*
            +CMGR: <stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
            */
            /*<fo>*/
            usLength += (TAF_UINT16)At_PrintMsgFo(pstTsDataInfo, (pgucAtSndCodeAddr + usLength));

            /*<mr>*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",%d,",
                                               pstTsDataInfo->u.stStaRpt.ucMr);

            /*<ra>*/
            usLength += (TAF_UINT16)At_PrintAsciiAddr(&pstTsDataInfo->u.stStaRpt.stRecipientAddr,
                                                      (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

            /*<tora>*/
            usLength += (TAF_UINT16)At_PrintAddrType(&pstTsDataInfo->u.stStaRpt.stRecipientAddr,
                                                     (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

            /* <scts> */
            usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stStaRpt.stTimeStamp,
                                                    (pgucAtSndCodeAddr + usLength));
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                               ",");

             /* <dt> */
             usLength += (TAF_UINT16)At_SmsPrintScts(&pstTsDataInfo->u.stStaRpt.stDischargeTime,
                                                     (pgucAtSndCodeAddr + usLength));

             /*<st>*/
             usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                                ",%d",
                                                pstTsDataInfo->u.stStaRpt.enStatus);
            break;
        default:
            break;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return;
}


TAF_VOID  At_ListRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent
)
{
    errno_t                             lMemResult;
    TAF_UINT16                          usLength;
    TAF_UINT32                          ulRet = AT_OK;
    MN_MSG_TS_DATA_INFO_STRU           *pstTsDataInfo = VOS_NULL_PTR;
    TAF_UINT32                          ulLoop;

    MN_MSG_LIST_PARM_STRU               stListParm;

    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ListRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    memset_s(&stListParm, sizeof(stListParm), 0x00, sizeof(MN_MSG_LIST_PARM_STRU));

    if (pstEvent->u.stListInfo.bSuccess != TAF_TRUE)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stListInfo.ulFailCause);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    usLength = 0;
    if (pstEvent->u.stListInfo.bFirstListEvt == VOS_TRUE)
    {
        if (gucAtVType == AT_V_ENTIRE_TYPE)
        {
            usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           "%s",
                                           gaucAtCrLf);
        }
    }
    pstTsDataInfo = At_GetMsgMem();

    for (ulLoop = 0; ulLoop < pstEvent->u.stListInfo.ulReportNum; ulLoop++)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s: %d,",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstEvent->u.stListInfo.astSmInfo[ulLoop].ulIndex);

        usLength += (TAF_UINT16)At_SmsPrintState(pstSmsCtx->enCmgfMsgFormat,
                                                 pstEvent->u.stListInfo.astSmInfo[ulLoop].enStatus,
                                                 (pgucAtSndCodeAddr + usLength));
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           ",");
        /**/
        ulRet = MN_MSG_Decode(&pstEvent->u.stListInfo.astSmInfo[ulLoop].stMsgInfo.stTsRawData, pstTsDataInfo);
        if (ulRet != MN_ERR_NO_ERROR)
        {
            ulRet = At_ChgMnErrCodeToAt(ucIndex, ulRet);
            At_FormatResultData(ucIndex, ulRet);
            return;
        }

        if (pstSmsCtx->enCmgfMsgFormat == AT_CMGF_MSG_FORMAT_PDU)/*PDU*/
        {
            /*
            +CMGL: <index>,<stat>,[<alpha>],<length><CR><LF><pdu>
            [<CR><LF>+CMGL:<index>,<stat>,[<alpha>],<length><CR><LF><pdu>
            [...]]
            */
            usLength += (TAF_UINT16)At_MsgPduInd(&pstEvent->u.stListInfo.astSmInfo[ulLoop].stMsgInfo.stScAddr,/*??*/
                                                 &pstEvent->u.stListInfo.astSmInfo[ulLoop].stMsgInfo.stTsRawData,
                                                 (pgucAtSndCodeAddr + usLength));
        }
        else
        {
            usLength += (TAF_UINT16)At_PrintListMsg(ucIndex, pstEvent, pstTsDataInfo, (pgucAtSndCodeAddr + usLength));
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

        At_BufferorSendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

        usLength = 0;
    }

    if (pstEvent->u.stListInfo.bLastListEvt == TAF_TRUE)
    {
        gstAtSendData.usBufLen = 0;
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {

        /* ��ʼ�� */
        lMemResult = memcpy_s( &stListParm, sizeof(stListParm), &(pstEvent->u.stListInfo.stReceivedListPara), sizeof(stListParm) );
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stListParm), sizeof(stListParm));

        /* ֪ͨSMS����Ҫ������ʾʣ�µĶ��� */
        stListParm.ucIsFirstTimeReq = VOS_FALSE;

        /* ִ��������� */
        if (MN_MSG_List( gastAtClientTab[ucIndex].usClientId,
                                            gastAtClientTab[ucIndex].opId,
                                            &stListParm) != MN_ERR_NO_ERROR )
        {
            gstAtSendData.usBufLen = 0;
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, AT_ERROR);
            return;
        }

    }

    return;
}


TAF_VOID At_WriteSmRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet               = AT_OK;
    TAF_UINT16                          usLength            = 0;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_WriteSmRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    if (pstEvent->u.stWriteInfo.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stWriteInfo.ulFailCause);
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: ",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           pstEvent->u.stWriteInfo.ulIndex);

    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulRet);
    return;
}


TAF_VOID At_SetCnmaRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetCnmaRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    if ((gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CNMA_TEXT_SET)
     || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CNMA_PDU_SET))
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }
    return;
}



VOS_UINT32 AT_GetSmsRpReportCause(TAF_MSG_ERROR_ENUM_UINT32 enMsgCause)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMapLength;

    /* 27005 3.2.5 0...127 3GPP TS 24.011 [6] clause E.2 values */
    /* 27005 3.2.5 128...255 3GPP TS 23.040 [3] clause 9.2.3.22 values.  */
    ulMapLength = sizeof(g_astAtSmsErrorCodeMap) / sizeof(g_astAtSmsErrorCodeMap[0]);

    for (i = 0; i < ulMapLength; i++)
    {
        if (g_astAtSmsErrorCodeMap[i].enMsgErrorCode == enMsgCause)
        {
            return g_astAtSmsErrorCodeMap[i].enAtErrorCode;
        }
    }

    return AT_CMS_UNKNOWN_ERROR;
}


TAF_VOID At_SendSmRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet               = AT_OK;
    TAF_UINT16                          usLength            = 0;

    AT_INFO_LOG("At_SendSmRspProc: step into function.");

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SendSmRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    /* ״̬��ƥ��: ��ǰû�еȴ����ͽ����AT��������ý���¼��ϱ� */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMGS_TEXT_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMGS_PDU_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMGC_TEXT_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMGC_PDU_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMSS_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMST_SET))
    {
        return;
    }

    gstAtSendData.usBufLen = 0;

    if (pstEvent->u.stSubmitRptInfo.enErrorCode != TAF_MSG_ERROR_NO_ERROR)
    {
        AT_NORM_LOG("At_SendSmRspProc: pstEvent->u.stSubmitRptInfo.enRptStatus is not ok.");

        ulRet = AT_GetSmsRpReportCause(pstEvent->u.stSubmitRptInfo.enErrorCode);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgSentSmNum < 1)
    {
        AT_WARN_LOG("At_SendSmRspProc: the number of sent message is zero.");
        return;
    }
    gastAtClientTab[ucIndex].AtSmsData.ucMsgSentSmNum--;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       pstEvent->u.stSubmitRptInfo.ucMr);

    if (gastAtClientTab[ucIndex].AtSmsData.ucMsgSentSmNum == 0)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex, ulRet);
    }
    else
    {
        At_MsgResultCodeFormat(ucIndex, usLength);
    }
    return;
}

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))


VOS_VOID At_SmsDeliverCbmProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    if ((pstSmsCtx->stCnmiType.CnmiModeType == AT_CNMI_MODE_SEND_OR_DISCARD_TYPE)
     || (pstSmsCtx->stCnmiType.CnmiModeType == AT_CNMI_MODE_SEND_OR_BUFFER_TYPE))
    {
        At_ForwardMsgToTe(MN_MSG_EVT_DELIVER_CBM, pstEvent);
        return;
    }

    /* ĿǰCBS��Ϣ������ */

    if (pstSmsCtx->stCnmiType.CnmiModeType == AT_CNMI_MODE_EMBED_AND_SEND_TYPE)
    {
        /*Ŀǰ��֧��*/
    }

    return;
}



VOS_UINT32  AT_CbPrintRange(
    VOS_UINT16                          usLength,
    TAF_CBA_CBMI_RANGE_LIST_STRU       *pstCbMidr
)
{
    TAF_UINT32                          ulLoop;
    TAF_UINT16                          usAddLen;

    usAddLen = usLength;
    for (ulLoop = 0; ulLoop < pstCbMidr->usCbmirNum; ulLoop++)
    {
        if ( pstCbMidr->astCbmiRangeInfo[ulLoop].usMsgIdFrom
            == pstCbMidr->astCbmiRangeInfo[ulLoop].usMsgIdTo)
        {

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d",
                                               pstCbMidr->astCbmiRangeInfo[ulLoop].usMsgIdFrom);
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d-%d",
                                           pstCbMidr->astCbmiRangeInfo[ulLoop].usMsgIdFrom,
                                           pstCbMidr->astCbmiRangeInfo[ulLoop].usMsgIdTo);

        }

        if (ulLoop != (pstCbMidr->usCbmirNum - 1))
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",");
        }
    }

    usAddLen = usLength - usAddLen;

    return usAddLen ;
}


VOS_VOID At_GetCbActiveMidsRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT16                          usLength;
    TAF_UINT16                          usAddLength;
    VOS_UINT32                          ulRet;
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_GetCbActiveMidsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    pstSmsCtx = AT_GetModemSmsCtxAddrFromClientId(ucIndex);

    /* ֹͣ��ʱ�� */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstEvent->u.stCbsCbMids.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stCbsCbMids.ulFailCause);
        At_FormatResultData(ucIndex, ulRet);
        return;
    }

    usLength = 0;

    /* ���ֵ���Զ�Ǽ����б�,���Թ̶���д0 */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s:0,",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"");

    /* �����Ϣ��MID */
    usAddLength = (VOS_UINT16)AT_CbPrintRange(usLength,&(pstEvent->u.stCbsCbMids.stCbMidr));

    usLength += usAddLength;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\",\"");

    /* ��������Ե�MID */
    usAddLength = (VOS_UINT16)AT_CbPrintRange(usLength,&(pstSmsCtx->stCbsDcssInfo));

    usLength += usAddLength;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"");


    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return;
}



VOS_VOID AT_ChangeCbMidsRsp(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    TAF_UINT32                          ulRet;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_ChangeCbMidsRsp : AT_BROADCAST_INDEX.");
        return;
    }

    if (pstEvent->u.stCbsChangeInfo.bSuccess != TAF_TRUE)
    {
        ulRet = At_ChgMnErrCodeToAt(ucIndex, pstEvent->u.stCbsChangeInfo.ulFailCause);
    }
    else
    {
        ulRet = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRet);

}


#if (FEATURE_ETWS == FEATURE_ON)

VOS_VOID  At_ProcDeliverEtwsPrimNotify(
    VOS_UINT8                                               ucIndex,
    MN_MSG_EVENT_INFO_STRU                                 *pstEvent
)
{
    TAF_CBA_ETWS_PRIM_NTF_EVT_INFO_STRU                    *pstPrimNtf = VOS_NULL_PTR;
    VOS_UINT16                                              usLength;

    pstPrimNtf = &pstEvent->u.stEtwsPrimNtf;

    /* ^ETWSPN: <plmn id>,<warning type>,<msg id>,<sn>,<auth> [,<warning security information>] */
    /* ʾ��: ^ETWSPN: "46000",0180,4352,3000,1 */

    usLength   = 0;
    usLength  += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%s^ETWSPN: ",
                                       gaucAtCrLf);

    /* <plmn id>
       ulMcc��ulMnc��˵����ʾ����
       ulMcc�ĵ�8λ    ����bit0--bit7������Ӧ MCC digit 1;
       ulMcc�Ĵε�8λ  ����bit8--bit15������Ӧ MCC digit 2;
       ulMcc�Ĵδε�8λ����bit16--bit23������Ӧ MCC digit 3;

       ulMnc�ĵ�8λ    ����bit0--bit7������Ӧ MNC digit 1;
       ulMnc�Ĵε�8λ  ����bit8--bit15������Ӧ MNC digit 2;
       ulMnc�Ĵδε�8λ����bit16--bit23������Ӧ MNC digit 3;
    */
    if ( (pstPrimNtf->stPlmn.ulMnc&0xFF0000) == 0x0F0000 )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "\"%d%d%d%d%d\",",
                                           (pstPrimNtf->stPlmn.ulMcc&0xFF),
                                           (pstPrimNtf->stPlmn.ulMcc&0xFF00)>>8,
                                           (pstPrimNtf->stPlmn.ulMcc&0xFF0000)>>16,
                                           (pstPrimNtf->stPlmn.ulMnc&0xFF),
                                           (pstPrimNtf->stPlmn.ulMnc&0xFF00)>>8);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "\"%d%d%d%d%d%d\",",
                                           (pstPrimNtf->stPlmn.ulMcc&0xFF),
                                           (pstPrimNtf->stPlmn.ulMcc&0xFF00)>>8,
                                           (pstPrimNtf->stPlmn.ulMcc&0xFF0000)>>16,
                                           (pstPrimNtf->stPlmn.ulMnc&0xFF),
                                           (pstPrimNtf->stPlmn.ulMnc&0xFF00)>>8,
                                           (pstPrimNtf->stPlmn.ulMnc&0xFF0000)>>16);
    }

    /* <warning type> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%04X,",
                                       pstPrimNtf->usWarnType);
    /* <msg id> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%04X,",
                                       pstPrimNtf->usMsgId);
    /* <sn> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%04X,",
                                       pstPrimNtf->usSN);

    /* <auth> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                       "%d%s",
                                       pstPrimNtf->enAuthRslt,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

}
#endif  /* (FEATURE_ON == FEATURE_ETWS) */


#endif


TAF_VOID At_SetCmmsRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_CMS_UNKNOWN_ERROR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetCmmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstEvent->u.stLinkCtrlInfo.ulErrorCode == MN_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex,ulResult);
    return;
}


TAF_VOID At_GetCmmsRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_RRETURN_CODE_ENUM_UINT32          ulResult = AT_CMS_UNKNOWN_ERROR;
    MN_MSG_LINK_CTRL_EVT_INFO_STRU      *pstLinkCtrlInfo = VOS_NULL_PTR;                     /*event report:MN_MSG_EVT_SET_COMM_PARAM*/

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_GetCmmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    gstAtSendData.usBufLen = 0;
    pstLinkCtrlInfo = &pstEvent->u.stLinkCtrlInfo;
    if (pstLinkCtrlInfo->ulErrorCode == MN_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstLinkCtrlInfo->enLinkCtrl);
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);
    return;
}


TAF_VOID At_SetCgsmsRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent
)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult;

    ulResult = AT_CMS_UNKNOWN_ERROR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetCgsmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstEvent->u.stSendDomainInfo.ulErrorCode == MN_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex,ulResult);
    return;
}


TAF_VOID At_GetCgsmsRspProc(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent
)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult;
    MN_MSG_SEND_DOMAIN_EVT_INFO_STRU   *pstSendDomainInfo = VOS_NULL_PTR;                     /*event report:MN_MSG_EVT_SET_COMM_PARAM*/

    ulResult = AT_CMS_UNKNOWN_ERROR;
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_GetCgsmsRspProc : AT_BROADCAST_INDEX.");
        return;
    }

    gstAtSendData.usBufLen = 0;
    pstSendDomainInfo      = &pstEvent->u.stSendDomainInfo;

    if (pstSendDomainInfo->ulErrorCode == MN_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        "%s: %d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstSendDomainInfo->enSendDomain);
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);
    return;
}


TAF_VOID At_SmsRspNop(
    TAF_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU              *pstEvent
)
{
    AT_INFO_LOG("At_SmsRspNop: no operation need for the event type ");
    return;
}


TAF_VOID At_SmsMsgProc(MN_AT_IND_EVT_STRU *pstData,TAF_UINT16 usLen)
{
    MN_MSG_EVENT_INFO_STRU              *pstEvent = VOS_NULL_PTR;
    MN_MSG_EVENT_ENUM_U32               enEvent;
    TAF_UINT8                           ucIndex;
    TAF_UINT32                          ulEventLen;
    errno_t                             lMemResult;


    enEvent = MN_MSG_EVT_MAX;


    AT_INFO_LOG("At_SmsMsgProc: Step into function.");
    AT_LOG1("At_SmsMsgProc: pstData->clientId,", pstData->clientId);

    ulEventLen = sizeof(MN_MSG_EVENT_ENUM_U32);
    lMemResult = memcpy_s(&enEvent,  sizeof(enEvent), pstData->aucContent, ulEventLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(enEvent), ulEventLen);
    pstEvent = (MN_MSG_EVENT_INFO_STRU *)&pstData->aucContent[ulEventLen];

    if (At_ClientIdToUserId(pstData->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_SmsMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (!AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        if (pstEvent->opId != gastAtClientTab[ucIndex].opId)
        {
            AT_LOG1("At_SmsMsgProc: pstEvent->opId,", pstEvent->opId);
            AT_LOG1("At_SmsMsgProc: gastAtClientTab[ucIndex].opId,", gastAtClientTab[ucIndex].opId);
            AT_NORM_LOG("At_SmsMsgProc: invalid operation id.");
            return;
        }

        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);
    }

    if (enEvent >= MN_MSG_EVT_MAX)
    {
        AT_WARN_LOG("At_SmsRspProc: invalid event type.");
        return;
    }

    AT_LOG1("At_SmsMsgProc enEvent", enEvent);
    g_aAtSmsMsgProcTable[enEvent](ucIndex, pstEvent);
    return;
}


VOS_VOID At_ProcVcSetVoiceMode(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvt
)
{
    AT_VMSET_CMD_CTX_STRU               *pstVmSetCmdCtx = VOS_NULL_PTR;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ProcVcSetVoiceMode : AT_BROADCAST_INDEX.");
        return;
    }

    /* ״̬�ж� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_VMSET_SET)
    {
        AT_WARN_LOG("At_ProcVcSetVoiceMode : opt error.");
        return;
    }

    pstVmSetCmdCtx = AT_GetCmdVmsetCtxAddr();

    if (pstVcEvt->bSuccess != VOS_TRUE)
    {
        pstVmSetCmdCtx->ulResult = AT_ERROR;
    }
#if (2 <= MULTI_MODEM_NUMBER)
    /* VMSET��������MODEM�ظ������ϱ���� */
    pstVmSetCmdCtx->ulReportedModemNum++;
    if (pstVmSetCmdCtx->ulReportedModemNum < MULTI_MODEM_NUMBER)
    {
        return;
    }
#endif

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, pstVmSetCmdCtx->ulResult);

    /* ��ʼ�����ý��ȫ�ֱ��� */
    AT_InitVmSetCtx();
    return;
}


VOS_UINT32 At_ProcVcGetVolumeEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvt
)
{
    VOS_UINT8                           aucIntraVolume[] = {AT_CMD_CLVL_LEV_0,AT_CMD_CLVL_LEV_1,
                                                            AT_CMD_CLVL_LEV_2,AT_CMD_CLVL_LEV_3,
                                                            AT_CMD_CLVL_LEV_4,AT_CMD_CLVL_LEV_5};
    VOS_UINT8                           ucVolumnLvl;
    VOS_UINT32                          i;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("APP_VC_AppQryVolumeProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CLVL_READ)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstVcEvt->bSuccess == VOS_TRUE)
    {
        /* ��ʽ��AT+CLVL����� */
        gstAtSendData.usBufLen = 0;

        ucVolumnLvl = 0;
        for (i = 0; i < 6; i++)
        {
            if (aucIntraVolume[i] == pstVcEvt->usVolume)
            {
                ucVolumnLvl = (VOS_UINT8)i;
                break;
            }
        }

        gstAtSendData.usBufLen =
            (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                   "%s: %d",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   ucVolumnLvl);

        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 At_ProcVcSetMuteStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
)
{
    VOS_UINT32                          ulRslt;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMUT_SET)
    {
        return VOS_ERR;
    }

    if (pstVcEvtInfo->bSuccess != VOS_TRUE)
    {
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 At_ProcVcGetMuteStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT16                          usLength = 0;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMUT_READ)
    {
        return VOS_ERR;
    }

    if (pstVcEvtInfo->bSuccess == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: %d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstVcEvtInfo->enMuteStatus);

        ulRslt = AT_OK;

        gstAtSendData.usBufLen = usLength;
    }
    else
    {
        ulRslt = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_VOID At_VcEventProc(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU              *pstVcEvt,
    APP_VC_EVENT_ENUM_U32               enEvent
)
{
    TAF_UINT32                          ulRet;
    switch (enEvent)
    {
        case APP_VC_EVT_SET_VOLUME:
            if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
            {
                AT_WARN_LOG("At_VcEventProc : AT_BROADCAST_INDEX.");
                return;
            }

            if (pstVcEvt->bSuccess != TAF_TRUE)
            {
                ulRet = AT_ERROR;
            }
            else
            {
                ulRet = AT_OK;
            }

            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, ulRet);
            return;

        case APP_VC_EVT_SET_VOICE_MODE:
            At_ProcVcSetVoiceMode(ucIndex, pstVcEvt);
            return;

        case APP_VC_EVT_GET_VOLUME:
            At_ProcVcGetVolumeEvent(ucIndex, pstVcEvt);
            return;

        case APP_VC_EVT_PARM_CHANGED:
            return;

        case APP_VC_EVT_SET_MUTE_STATUS:
            At_ProcVcSetMuteStatusEvent(ucIndex, pstVcEvt);
            return;

        case APP_VC_EVT_GET_MUTE_STATUS:
            At_ProcVcGetMuteStatusEvent(ucIndex, pstVcEvt);
            return;

#if (FEATURE_ECALL == FEATURE_ON)
        case APP_VC_EVT_ECALL_TRANS_STATUS:
            At_ProcVcReportEcallStateEvent(ucIndex, pstVcEvt);
            return;

        case APP_VC_EVT_SET_ECALL_CFG:
            At_ProcVcSetEcallCfgEvent(ucIndex, pstVcEvt);
            return;
        /* eCall֧��abort������ALACK�ϱ�AT */
        case APP_VC_EVT_ECALL_ABORT_CNF:
            (VOS_VOID)AT_ProcVcEcallAbortCnf(ucIndex, pstVcEvt);
            return;
        case APP_VC_EVT_ECALL_TRANS_ALACK:
            (VOS_VOID)AT_ProcVcReportEcallAlackEvent(ucIndex, pstVcEvt);
            return;
#endif

        default:
            return;
    }

}


TAF_VOID At_VcMsgProc(MN_AT_IND_EVT_STRU *pstData,TAF_UINT16 usLen)
{
    APP_VC_EVENT_INFO_STRU              *pstEvent = VOS_NULL_PTR;
    APP_VC_EVENT_ENUM_U32               enEvent;
    TAF_UINT8                           ucIndex;
    TAF_UINT32                          ulEventLen;
    errno_t                             lMemResult;


    enEvent = APP_VC_EVT_BUTT;


    AT_INFO_LOG("At_VcMsgProc: Step into function.");
    AT_LOG1("At_VcMsgProc: pstData->clientId,", pstData->clientId);

    ulEventLen = sizeof(APP_VC_EVENT_ENUM_U32);
    lMemResult = memcpy_s(&enEvent,  sizeof(enEvent), pstData->aucContent, ulEventLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(enEvent), ulEventLen);
    pstEvent = (APP_VC_EVENT_INFO_STRU *)&pstData->aucContent[ulEventLen];

    if (At_ClientIdToUserId(pstData->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_VcMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (!AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_LOG1("At_VcMsgProc: ucIndex", ucIndex);
        if (ucIndex >= AT_MAX_CLIENT_NUM)
        {
            AT_WARN_LOG("At_VcMsgProc: invalid CLIENT ID or index.");
            return;
        }

        if (pstEvent->opId != gastAtClientTab[ucIndex].opId)
        {
            AT_LOG1("At_VcMsgProc: pstEvent->opId,", pstEvent->opId);
            AT_LOG1("At_VcMsgProc: gastAtClientTab[ucIndex].opId,", gastAtClientTab[ucIndex].opId);
            AT_NORM_LOG("At_VcMsgProc: invalid operation id.");
            return;
        }

        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);
    }

    if (enEvent >= APP_VC_EVT_BUTT)
    {
        AT_WARN_LOG("At_SmsRspProc: invalid event type.");
        return;
    }

    AT_LOG1("At_VcMsgProc enEvent", enEvent);
    At_VcEventProc(ucIndex,pstEvent,enEvent);


}


TAF_VOID At_SetParaRspProc( TAF_UINT8 ucIndex,
                                      TAF_UINT8 OpId,
                                      TAF_PARA_SET_RESULT Result,
                                      TAF_PARA_TYPE ParaType)
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_FAILURE;
    TAF_UINT16 usLength = 0;

    /* �����PS��ĸ������� */
    if(gastAtClientTab[ucIndex].usAsyRtnNum > 0)
    {
        gastAtClientTab[ucIndex].usAsyRtnNum--;         /* ���������1 */
        if(Result == TAF_PARA_OK)
        {
            if(gastAtClientTab[ucIndex].usAsyRtnNum != 0)
            {
                return;                                 /* ���OK���һ����������� */
            }
        }
        else
        {
            gastAtClientTab[ucIndex].usAsyRtnNum = 0;   /* ���ERROR�����ϱ����������� */
        }
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    switch(Result)
    {
    case TAF_PARA_OK:
        ulResult = AT_OK;
        break;

    case TAF_PARA_SIM_IS_BUSY:
        if(g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex > AT_CMD_SMS_BEGAIN)
        {
            ulResult = AT_CMS_U_SIM_BUSY;
        }
        else
        {
            ulResult = AT_CME_SIM_BUSY;
        }
        break;

    default:
        if(g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex > AT_CMD_SMS_BEGAIN)
        {
            ulResult = AT_CMS_UNKNOWN_ERROR;
        }
        else
        {
            ulResult = AT_CME_UNKNOWN;
        }
        break;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);
}

TAF_VOID At_SetMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    errno_t                             lMemResult;
    TAF_UINT16                          ClientId = 0;
    TAF_UINT8                           OpId = 0;
    TAF_PARA_SET_RESULT                 Result = 0;
    TAF_PARA_TYPE                       ParaType = 0;
    TAF_UINT16                          usAddr = 0;
    TAF_UINT8                           ucIndex  = 0;

    lMemResult = memcpy_s(&ClientId, sizeof(ClientId), pData, sizeof(ClientId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ClientId), sizeof(ClientId));
    usAddr += sizeof(ClientId);

    lMemResult = memcpy_s(&OpId, sizeof(OpId), pData+usAddr, sizeof(OpId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(OpId), sizeof(OpId));
    usAddr += sizeof(OpId);

    lMemResult = memcpy_s(&Result, sizeof(Result), pData+usAddr, sizeof(Result));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(Result), sizeof(Result));
    usAddr += sizeof(Result);

    lMemResult = memcpy_s(&ParaType, sizeof(ParaType), pData+usAddr, sizeof(ParaType));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ParaType), sizeof(ParaType));
    usAddr += sizeof(ParaType);


    AT_LOG1("At_SetMsgProc ClientId",ClientId);
    AT_LOG1("At_SetMsgProc Result",Result);
    AT_LOG1("At_SetMsgProc ParaType",ParaType);

    if(ClientId == AT_BUTT_CLIENT_ID)
    {
        AT_WARN_LOG("At_SetMsgProc Error ucIndex");
        return;
    }
    else
    {
        if(At_ClientIdToUserId(ClientId,&ucIndex) == AT_FAILURE)
        {
            AT_WARN_LOG("At_SetMsgProc At_ClientIdToUserId FAILURE");
            return;
        }

        if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
        {
            AT_WARN_LOG("At_SetMsgProc : AT_BROADCAST_INDEX.");
            return;
        }

        AT_LOG1("At_SetMsgProc ucIndex",ucIndex);
        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

        At_SetParaRspProc(ucIndex,OpId,Result,ParaType);
    }
}




VOS_UINT16 AT_GetOperNameLengthForCops(
    TAF_CHAR                            *pstr,
    TAF_UINT8                           ucMaxLen
)
{
    VOS_UINT16                          usRsltLen;
    TAF_UINT8                           i;

    usRsltLen = 0;

    for (i = 0; i < ucMaxLen; i++)
    {
        if (pstr[i] != '\0')
        {
            usRsltLen = i+1;
        }
    }

    return usRsltLen;

}

VOS_VOID At_QryParaRspCopsProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult = AT_FAILURE;
    VOS_UINT16                          usLength = 0;
    VOS_UINT16                          usNameLength = 0;
    TAF_PH_NETWORKNAME_STRU             stCops;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    memset_s(&stCops, sizeof(stCops), 0x00, sizeof(TAF_PH_NETWORKNAME_STRU));

    lMemResult = memcpy_s(&stCops, sizeof(stCops), pPara, sizeof(TAF_PH_NETWORKNAME_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCops), sizeof(TAF_PH_NETWORKNAME_STRU));

    /* A32D07158
     * +COPS: <mode>[,<format>,<oper>[,<AcT>]], get the PLMN selection mode from msg sent by MMA
     */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       stCops.PlmnSelMode);

    /* MBB����֧��COPS=2��ѯʱ����2���˳���ͬ��ֻ��Ҫ��ʾsel mode */
    if ((AT_PH_IsPlmnValid(&(stCops.Name.PlmnId)) == VOS_FALSE)
      )
    {
        /* ��Ч PLMNId ֻ��ʾ sel mode */
        ulResult = AT_OK;
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex,ulResult);
        return;
    }

    /* <format> */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstNetCtx->ucCopsFormatType);

    /* <oper> */
    switch (pstNetCtx->ucCopsFormatType)
    {
        /* �����֣��ַ������� */
        case AT_COPS_LONG_ALPH_TYPE:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",\"");

            /*�����ʾSPN�д���0x00��Ч�ַ������,��ȡ��ʵ�ʳ���,At_sprintf��0x00��β�������ã�ʹ��PS_MEM_CPY����*/
            usNameLength = AT_GetOperNameLengthForCops(stCops.Name.aucOperatorNameLong, TAF_PH_OPER_NAME_LONG);

            lMemResult = memcpy_s(pgucAtSndCodeAddr + usLength,
                                  AT_CMD_MAX_LEN + 20 - 3 - usLength,
                                  stCops.Name.aucOperatorNameLong,
                                  usNameLength);
            TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN + 20 - 3 - usLength, usNameLength);

            usLength = usLength + usNameLength;
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"");
            break;

       /* �����֣��ַ������� */
        case AT_COPS_SHORT_ALPH_TYPE:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",\"");

            /*�����ʾSPN�д���0x00��Ч�ַ������,��ȡ��ʵ�ʳ���,At_sprintf��0x00��β�������ã�ʹ��PS_MEM_CPY����*/
            usNameLength = AT_GetOperNameLengthForCops(stCops.Name.aucOperatorNameShort, TAF_PH_OPER_NAME_SHORT);

            lMemResult = memcpy_s(pgucAtSndCodeAddr + usLength,
                                  AT_CMD_MAX_LEN + 20 - 3 - usLength,
                                  stCops.Name.aucOperatorNameShort,
                                  usNameLength);
            TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN + 20 - 3 - usLength, usNameLength);

            usLength = usLength + usNameLength;
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"");
            break;

        /* BCD���MCC��MNC����Ҫת�����ַ��� */
        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",\"%X%X%X",
                                               (0x0f00 & stCops.Name.PlmnId.Mcc) >> 8,
                                               (0x00f0 & stCops.Name.PlmnId.Mcc) >> 4,
                                               (0x000f & stCops.Name.PlmnId.Mcc));

            if( ((0x0f00 & stCops.Name.PlmnId.Mnc) >> 8) != 0x0F)
            {
                usLength +=(VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  "%X",
                                                  (0x0f00 & stCops.Name.PlmnId.Mnc) >> 8);
            }

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%X%X\"",
                                               (0x00f0 & stCops.Name.PlmnId.Mnc) >> 4,
                                               (0x000f & stCops.Name.PlmnId.Mnc));
            break;
    }

    /* <AcT> */
    if(stCops.RaMode == TAF_PH_RA_GSM)  /* GSM */
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",0");
    }
    else if(stCops.RaMode == TAF_PH_RA_WCDMA)   /* CDMA */
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",2");
    }
    else if(stCops.RaMode == TAF_PH_RA_LTE)   /* LTE */
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",7");
    }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    else if (stCops.RaMode == TAF_PH_RA_NR)   /* NR */
    {
         usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",12");
    }
#endif
    else
    {

    }

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

}

#if(FEATURE_ON == FEATURE_LTE)
TAF_VOID At_QryParaRspCellRoamProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength = 0;

    TAF_PH_CELLROAM_STRU                stCellRoam;

    memset_s(&stCellRoam, sizeof(stCellRoam), 0x00, sizeof(TAF_PH_CELLROAM_STRU));

    lMemResult = memcpy_s(&stCellRoam, sizeof(stCellRoam), pPara, sizeof(TAF_PH_CELLROAM_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCellRoam), sizeof(TAF_PH_CELLROAM_STRU));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
        "%s:%d,%d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                 stCellRoam.RoamMode,
                 stCellRoam.RaMode);

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}
#endif


TAF_VOID At_QryParaRspSysinfoProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    TAF_PH_SYSINFO_STRU                 stSysInfo;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    ulResult                            = AT_FAILURE;
    usLength                            = 0;

    memset_s(&stSysInfo, sizeof(stSysInfo), 0x00, sizeof(TAF_PH_SYSINFO_STRU));

    lMemResult = memcpy_s(&stSysInfo, sizeof(stSysInfo), pPara, sizeof(TAF_PH_SYSINFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stSysInfo), sizeof(TAF_PH_SYSINFO_STRU));
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:%d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,stSysInfo.ucSrvStatus);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSrvDomain);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucRoamStatus);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSysMode);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSimStatus);

    if ( *pucSystemAppConfig == SYSTEM_APP_WEBUI)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSimLockStatus);
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",");
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSysSubMode);

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


VOS_VOID At_QryMmPlmnInfoRspProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    VOS_VOID                           *pPara
)
{
    TAF_MMA_MM_INFO_PLMN_NAME_STRU     *pstPlmnName = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           i;

    /* ������ʼ�� */
    pstPlmnName = (TAF_MMA_MM_INFO_PLMN_NAME_STRU *)pPara;
    ulResult    = AT_ERROR;

    /* ת��LongName��ShortName */
    if ( pstPlmnName->ucLongNameLen <= TAF_PH_OPER_NAME_LONG
      && pstPlmnName->ucShortNameLen <= TAF_PH_OPER_NAME_SHORT )
    {

        /* ^MMPLMNINFO:<long name>,<short name> */
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s:",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        for (i = 0; i < pstPlmnName->ucLongNameLen; i++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%02X",
                                               pstPlmnName->aucLongName[i]);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + usLength, ",");

        for (i = 0; i < pstPlmnName->ucShortNameLen; i++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%02X",
                                               pstPlmnName->aucShortName[i]);
        }

        ulResult = AT_OK;
        gstAtSendData.usBufLen = usLength;
    }
    else
    {
        gstAtSendData.usBufLen = 0;
    }

    At_FormatResultData(ucIndex,ulResult);

    return;
}

/*****************************************************************************
 Prototype      : At_QryParaRspCimiProc
 Description    : ������ѯ���Cimi���ϱ�����
 Input          : usClientId --- �û�ID
                  OpId       --- ����ID
                  QueryType  --- ��ѯ����
                  pPara      --- ���
 Output         :
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-04-19
    Author      : ---
    Modification: Created function
*****************************************************************************/
TAF_VOID At_QryParaRspCimiProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength = 0;

    TAF_PH_IMSI_STRU                    stCimi;

    lMemResult = memcpy_s(&stCimi, sizeof(stCimi), pPara, sizeof(TAF_PH_IMSI_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCimi), sizeof(TAF_PH_IMSI_STRU));
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",stCimi.aucImsi);

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


TAF_VOID At_QryParaRspCgclassProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength = 0;

    TAF_PH_MS_CLASS_TYPE                stCgclass;

    stCgclass = TAF_PH_MS_CLASS_NULL;

    lMemResult = memcpy_s(&stCgclass, sizeof(stCgclass), pPara, sizeof(TAF_PH_MS_CLASS_TYPE));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgclass), sizeof(TAF_PH_MS_CLASS_TYPE));
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    if(stCgclass == TAF_PH_MS_CLASS_A)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"A\"");
    }
    else if(stCgclass == TAF_PH_MS_CLASS_B)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"B\"");
    }
    else if(stCgclass == TAF_PH_MS_CLASS_CG)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"CG\"");
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"CC\"");
    }

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


VOS_VOID At_QryParaRspCregProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;

    TAF_PH_REG_STATE_STRU               stCreg;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    memset_s(&stCreg, sizeof(stCreg), 0x00, sizeof(TAF_PH_REG_STATE_STRU));

    lMemResult = memcpy_s(&stCreg, sizeof(stCreg), pPara, sizeof(TAF_PH_REG_STATE_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCreg), sizeof(TAF_PH_REG_STATE_STRU));

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       (VOS_UINT32)pstNetCtx->ucCregType);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       stCreg.RegState);

    if ((pstNetCtx->ucCregType == AT_CREG_RESULT_CODE_ENTIRE_TYPE)
     && ((stCreg.RegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
      || (stCreg.RegState == TAF_PH_REG_REGISTERED_ROAM)))
    {
        usLength += (VOS_UINT16)At_PhReadCreg(&stCreg, pgucAtSndCodeAddr + usLength);
    }

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    return;
}


VOS_VOID At_QryParaRspCgregProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;

    TAF_PH_REG_STATE_STRU               stCgreg;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    memset_s(&stCgreg, sizeof(stCgreg), 0x00, sizeof(TAF_PH_REG_STATE_STRU));

    lMemResult = memcpy_s(&stCgreg, sizeof(stCgreg), pPara, sizeof(TAF_PH_REG_STATE_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgreg), sizeof(TAF_PH_REG_STATE_STRU));


    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       (VOS_UINT32)pstNetCtx->ucCgregType);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       stCgreg.ucPsRegState);

    if ((pstNetCtx->ucCgregType == AT_CGREG_RESULT_CODE_ENTIRE_TYPE)
     && ((stCgreg.ucPsRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
      || (TAF_PH_REG_REGISTERED_ROAM == stCgreg.ucPsRegState))
     && (stCgreg.ucAct != TAF_PH_ACCESS_TECH_NR_5GC))
    {
        usLength += (VOS_UINT16)At_PhReadCreg(&stCgreg, pgucAtSndCodeAddr + usLength);
    }

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    return;
}

#if(FEATURE_ON == FEATURE_LTE)

VOS_VOID AT_QryParaRspCeregProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucOpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;

    TAF_PH_REG_STATE_STRU               stCereg;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    memset_s(&stCereg, sizeof(stCereg), 0x00, sizeof(TAF_PH_REG_STATE_STRU));

    lMemResult = memcpy_s(&stCereg, sizeof(stCereg), pPara, sizeof(TAF_PH_REG_STATE_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCereg), sizeof(TAF_PH_REG_STATE_STRU));

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       (VOS_UINT32)pstNetCtx->ucCeregType);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,",%d",
                                       stCereg.ucPsRegState);

    /* ��������һ�£�GU�²�ѯֻ�ϱ�stat */
    if ((pstNetCtx->ucCeregType == AT_CEREG_RESULT_CODE_ENTIRE_TYPE)
     && (stCereg.ucAct == TAF_PH_ACCESS_TECH_E_UTRAN)
     && ((stCereg.ucPsRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
      || (stCereg.ucPsRegState == TAF_PH_REG_REGISTERED_ROAM)))
    {

        usLength += (VOS_UINT16)At_PhReadCreg(&stCereg, pgucAtSndCodeAddr + usLength);

    }

    ulResult               = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return;
}

#endif

#if(FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_VOID AT_QryParaRspC5gregProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucOpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;

    TAF_PH_REG_STATE_STRU               stC5greg;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;
    VOS_CHAR                            acStrAllowedNssai[AT_EVT_MULTI_S_NSSAI_LEN];

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    memset_s(&stC5greg, sizeof(stC5greg), 0x00, sizeof(TAF_PH_REG_STATE_STRU));
    lMemResult = memcpy_s(&stC5greg, sizeof(stC5greg), pPara, sizeof(TAF_PH_REG_STATE_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stC5greg), sizeof(TAF_PH_REG_STATE_STRU));

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       (VOS_UINT32)pstNetCtx->ucC5gregType);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,",%d",
                                       stC5greg.ucPsRegState);

    if ((pstNetCtx->ucC5gregType == AT_C5GREG_RESULT_CODE_ENTIRE_TYPE)
     && (stC5greg.ucAct == TAF_PH_ACCESS_TECH_NR_5GC)
     && ((stC5greg.ucPsRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK)
      || (stC5greg.ucPsRegState == TAF_PH_REG_REGISTERED_ROAM)))
    {
        usLength += (VOS_UINT16)At_PhReadC5greg(&stC5greg, pgucAtSndCodeAddr + usLength);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",");

        /* ��װ��Ƭ��Ϣ��ʽ */
        ulLength = 0;
        memset_s(acStrAllowedNssai, sizeof(acStrAllowedNssai), 0, sizeof(acStrAllowedNssai));

        AT_ConvertMultiSNssaiToString(AT_MIN(stC5greg.stAllowedNssai.ucSNssaiNum, PS_MAX_ALLOWED_S_NSSAI_NUM),
                                      &stC5greg.stAllowedNssai.astSNssai[0],
                                      acStrAllowedNssai,
                                      sizeof(acStrAllowedNssai),
                                      &ulLength);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d,",
                                           ulLength);

        if (ulLength != 0)
        {
             usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                 "\"%s\"",
                                                 acStrAllowedNssai);
        }
    }

    ulResult = AT_OK;

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, ulResult);

    return;
}
#endif




TAF_VOID At_QryParaRspIccidProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                           *pPara
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength;
    TAF_PH_ICC_ID_STRU                  stIccId;

    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ICCID_READ)
    {
        return;
    }

    usLength = 0;
    memset_s(&stIccId, sizeof(stIccId), 0x00, sizeof(TAF_PH_ICC_ID_STRU));
    lMemResult = memcpy_s(&stIccId, sizeof(stIccId), pPara, sizeof(TAF_PH_ICC_ID_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stIccId), sizeof(TAF_PH_ICC_ID_STRU));

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)AT_Hex2AsciiStrLowHalfFirst(AT_CMD_MAX_LEN,
                                                        (VOS_INT8 *)pgucAtSndCodeAddr,
                                                        (VOS_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                        stIccId.aucIccId,
                                                        stIccId.ucLen);

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return;
}


TAF_VOID At_QryRspUsimRangeProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_PH_QRY_USIM_RANGE_INFO_STRU     *pstUsimRangeInfo = VOS_NULL_PTR;
    TAF_UINT16                          usLength = 0;
    TAF_UINT32                          ulResult;
    TAF_UINT8                           ucSimValue;
    VOS_BOOL                            bUsimInfoPrinted = VOS_FALSE;

    pstUsimRangeInfo = (TAF_PH_QRY_USIM_RANGE_INFO_STRU*)pPara;
    if((pstUsimRangeInfo->stUsimInfo.bFileExist == VOS_TRUE)
    && (pstUsimRangeInfo->stUsimInfo.Icctype == TAF_PH_ICC_USIM))
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        ucSimValue = 1;
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d,(1,%d),%d",
                                           ucSimValue,
                                           pstUsimRangeInfo->stUsimInfo.ulTotalRecNum,
                                           pstUsimRangeInfo->stUsimInfo.ulRecordLen);
        bUsimInfoPrinted = VOS_TRUE;
    }
    if ((pstUsimRangeInfo->stSimInfo.bFileExist == VOS_TRUE)
     && (pstUsimRangeInfo->stSimInfo.Icctype == TAF_PH_ICC_SIM))
    {
        if (bUsimInfoPrinted == VOS_TRUE)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        ucSimValue = 0;
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%d,(1,%d),%d",
                                           ucSimValue,
                                           pstUsimRangeInfo->stSimInfo.ulTotalRecNum,
                                           pstUsimRangeInfo->stSimInfo.ulRecordLen);

    }
    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}

TAF_VOID At_QryParaRspPnnProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT16                          usLength = 0;

    TAF_PH_USIM_PNN_CNF_STRU            *pstPNN = VOS_NULL_PTR;
    TAF_UINT8                           FullNameLen;
    TAF_UINT8                           ShortNameLen;
    TAF_UINT8                           ucTag;
    TAF_UINT8                           ucFirstByte;
    VOS_UINT8                           ucPnnOperNameLen;
    TAF_UINT32                          i;
    TAF_UINT32                          ulRet;
    TAF_PH_QRY_USIM_INFO_STRU           stUsimInfo;

    memset_s(&stUsimInfo, sizeof(stUsimInfo), 0x00, sizeof(stUsimInfo));

    pstPNN     = (TAF_PH_USIM_PNN_CNF_STRU*)pPara;


    /* ��ѯPNN��¼���ͼ�¼���� */
    for (i = 0 ; i < pstPNN->TotalRecordNum; i++)
    {
        FullNameLen = 0;
        ShortNameLen = 0;

        FullNameLen = pstPNN->PNNRecord[i].stOperNameLong.ucLength;

        if (FullNameLen == 0)
        {
            continue;
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /* ��ӡ����,��Ҫ����TAG,���Ⱥͱ����ʽ */
        usLength        += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
        ucTag            = FULL_NAME_IEI;
        usLength        += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucTag,1);
        ucPnnOperNameLen = pstPNN->PNNRecord[i].stOperNameLong.ucLength+1;
        usLength        += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucPnnOperNameLen,1);
        ucFirstByte      = (TAF_UINT8)((pstPNN->PNNRecord[i].stOperNameLong.bitExt    << 7)
                                 | (pstPNN->PNNRecord[i].stOperNameLong.bitCoding << 4)
                                 | (pstPNN->PNNRecord[i].stOperNameLong.bitAddCi  << 3)
                                 | (pstPNN->PNNRecord[i].stOperNameLong.bitSpare));
        usLength    += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucFirstByte,1);
        usLength    += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,pstPNN->PNNRecord[i].stOperNameLong.aucOperatorName,FullNameLen);
        usLength    += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");

        ShortNameLen = pstPNN->PNNRecord[i].stOperNameShort.ucLength;

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",");
        if (ShortNameLen != 0)
        {
            ucTag     = SHORT_NAME_IEI;
            /* ��ӡ����,��Ҫ����TAG,���Ⱥͱ����ʽ */
            usLength        += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
            usLength        += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucTag,1);
            ucPnnOperNameLen = pstPNN->PNNRecord[i].stOperNameShort.ucLength + 1;
            usLength        += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucPnnOperNameLen,1);
            ucFirstByte      = (TAF_UINT8)((pstPNN->PNNRecord[i].stOperNameShort.bitExt    << 7)
                                     | (pstPNN->PNNRecord[i].stOperNameShort.bitCoding << 4)
                                     | (pstPNN->PNNRecord[i].stOperNameShort.bitAddCi  << 3)
                                     | (pstPNN->PNNRecord[i].stOperNameShort.bitSpare));
            usLength        += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucFirstByte,1);
            usLength        += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,pstPNN->PNNRecord[i].stOperNameShort.aucOperatorName,ShortNameLen);
            usLength        += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"\"");
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",");


        if (pstPNN->PNNRecord[i].ucPlmnAdditionalInfoLen != 0)
        {
            /* PNN��������Ϣ,��Ҫ����tag�ͳ��� */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
            ucTag     = PLMN_ADDITIONAL_INFO_IEI;
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucTag,1);
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&pstPNN->PNNRecord[i].ucPlmnAdditionalInfoLen,1);
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,pstPNN->PNNRecord[i].aucPlmnAdditionalInfo,pstPNN->PNNRecord[i].ucPlmnAdditionalInfoLen);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"\"");
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);

        At_BufferorSendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
        usLength = 0;

    }

    /* ��������ϱ���PNN��Ŀ��Ҫ�����ͬ������ΪC������PNNû���ϱ���Ҫ��������������в�ѯ */
    if (pstPNN->TotalRecordNum == TAF_MMA_PNN_INFO_MAX_NUM)
    {
        stUsimInfo.ulRecNum                     = 0;
        stUsimInfo.enEfId                       = TAF_PH_PNN_FILE;
        stUsimInfo.Icctype                      = pstPNN->Icctype;
        stUsimInfo.stPnnQryIndex.usPnnNum       = TAF_MMA_PNN_INFO_MAX_NUM;
        stUsimInfo.stPnnQryIndex.usPnnCurrIndex = pstPNN->usPnnCurrIndex + TAF_MMA_PNN_INFO_MAX_NUM;

        ulRet = MN_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                       0,
                                       TAF_MSG_MMA_USIM_INFO,
                                       &stUsimInfo,
                                       sizeof(TAF_PH_QRY_USIM_INFO_STRU),
                                       I0_WUEPS_PID_MMA);

        if (ulRet != TAF_SUCCESS)
        {
            /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            gstAtSendData.usBufLen = 0;
            At_FormatResultData(ucIndex, AT_ERROR);
        }
    }
    else
    {
        /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }

}

TAF_VOID At_QryParaRspCPnnProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength = 0;
    TAF_UINT8                           ucCodingScheme;
    TAF_PH_USIM_PNN_CNF_STRU            *pstPNN = VOS_NULL_PTR;
    TAF_UINT8                           FullNameLen;
    TAF_UINT8                           ShortNameLen;
    TAF_UINT8                           ucTag;

    pstPNN     = (TAF_PH_USIM_PNN_CNF_STRU*)pPara;

    if (pstPNN->TotalRecordNum != 0)
    {
        FullNameLen = 0;
        ShortNameLen = 0;

        FullNameLen = pstPNN->PNNRecord[0].stOperNameLong.ucLength;

        if (FullNameLen != 0)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

            /*��ӡ����*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
            ucTag = FULL_NAME_IEI;
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,&ucTag,1);
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,pstPNN->PNNRecord[0].stOperNameLong.aucOperatorName,FullNameLen);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");

            ucCodingScheme = pstPNN->PNNRecord[0].stOperNameLong.bitCoding;
            if (pstPNN->PNNRecord[0].stOperNameLong.bitCoding != 0)
            {
                ucCodingScheme = 1;
            }

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d,%d",
                                                ucCodingScheme,pstPNN->PNNRecord[0].stOperNameLong.bitAddCi);

            ShortNameLen = pstPNN->PNNRecord[0].stOperNameShort.ucLength;

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",");

            if (ShortNameLen != 0)
            {
                /*��ӡ����*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
                usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,pstPNN->PNNRecord[0].stOperNameShort.aucOperatorName,ShortNameLen);
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");

                ucCodingScheme = pstPNN->PNNRecord[0].stOperNameShort.bitCoding;
                if (ucCodingScheme != 0)
                {
                    ucCodingScheme = 1;
                }

                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d,%d",
                                    ucCodingScheme,pstPNN->PNNRecord[0].stOperNameShort.bitAddCi);
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"\"");

                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",0,0");

            }

        }

    }

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


TAF_UINT8 At_IsOplRecPrintable(
    TAF_PH_USIM_OPL_RECORD             *pstOplRec,
    VOS_CHAR                            cWildCard
)
{
    TAF_UINT32                          i;

    VOS_UINT8                           ucWildCard;

    ucWildCard = 0x00;

    AT_ConvertCharToHex((VOS_UINT8)cWildCard, &ucWildCard);

    if ( pstOplRec->PNNIndex == 0xFF)
    {
        return VOS_FALSE;
    }

    for ( i=0; i < pstOplRec->PlmnLen; i++)
    {
        if ((pstOplRec->PLMN[i] >= 0xA)
         && (ucWildCard != pstOplRec->PLMN[i]))
        {
            return VOS_FALSE;
        }
    }


    return VOS_TRUE;

}

/*****************************************************************************
 Prototype      : At_QryParaRspOplProc
 Description    : ������ѯ���Opl���ϱ�����
 Input          : usClientId --- �û�ID
                  OpId       --- ����ID
                  QueryType  --- ��ѯ����
                  pPara      --- ���
 Output         :
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-04-19
    Author      : ---
    Modification: Created function
*****************************************************************************/
TAF_VOID At_QryParaRspOplProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength = 0;
    TAF_UINT32                          i;
    TAF_UINT32                          j;
    TAF_PH_USIM_OPL_CNF_STRU            *pstOPL = VOS_NULL_PTR;
    TAF_UINT32                          ucRecCntPrinted = 0;

    pstOPL     = (TAF_PH_USIM_OPL_CNF_STRU*)pPara;

    /* ��ѯPNN��¼���ͼ�¼���� */
    for(i = 0 ; i <  pstOPL->TotalRecordNum; i++)
    {
        if ( At_IsOplRecPrintable((pstOPL->OPLRecord+i),pstOPL->cWildCard) == VOS_FALSE)
        {
            continue;
        }
        if(ucRecCntPrinted != 0)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        for(j = 0 ; j < pstOPL->OPLRecord[i].PlmnLen; j++)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%X",pstOPL->OPLRecord[i].PLMN[j]);
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",0x%X-0x%X,%d",pstOPL->OPLRecord[i].LACLow, pstOPL->OPLRecord[i].LACHigh, pstOPL->OPLRecord[i].PNNIndex);

        ++ ucRecCntPrinted;
    }


    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


TAF_VOID At_QryParaRspCfplmnProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength = 0;
    TAF_USER_PLMN_LIST_STRU             *pstUserPlmnList = VOS_NULL_PTR;
    TAF_UINT32                          i;

    pstUserPlmnList = (TAF_USER_PLMN_LIST_STRU*) pPara;

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: %d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,pstUserPlmnList->usPlmnNum);
    for ( i = 0 ; i < pstUserPlmnList->usPlmnNum; i++ )
    {

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%X%X%X",
                     (0x0f00 & pstUserPlmnList->Plmn[i].Mcc) >> 8,
                     (0x00f0 & pstUserPlmnList->Plmn[i].Mcc) >> 4,
                     (0x000f & pstUserPlmnList->Plmn[i].Mcc)
                     );

        if( (0x0f00 & pstUserPlmnList->Plmn[i].Mnc) == 0x0f00)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%X%X\"",
                     (0x00f0 & pstUserPlmnList->Plmn[i].Mnc) >> 4,
                     (0x000f & pstUserPlmnList->Plmn[i].Mnc)
                     );
        }
        else
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%X%X%X\"",
                     (0x0f00 & pstUserPlmnList->Plmn[i].Mnc) >> 8,
                     (0x00f0 & pstUserPlmnList->Plmn[i].Mnc) >> 4,
                     (0x000f & pstUserPlmnList->Plmn[i].Mnc)
                     );
        }

    }

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}






VOS_UINT32 AT_RcvTafCcmGetCdurCnf(VOS_VOID *pMsg)
{
    TAF_CCM_GET_CDUR_CNF_STRU          *pstCdurCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex    = 0;

    pstCdurCnf = (TAF_CCM_GET_CDUR_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCdurCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallSsInd: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�CDUR��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CDUR_READ)
    {
        return VOS_ERR;
    }

    ulResult = AT_OK;

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCdurCnf->stGetCdurPara.enCause == TAF_CS_CAUSE_SUCCESS)
    {
        /* �����ѯ���: ����ṹΪ^CDUR: <CurCallTime>��ʽ */
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        "%s:%d,%d",
                                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                         pstCdurCnf->stGetCdurPara.callId,
                                                         pstCdurCnf->stGetCdurPara.ulCurCallTime);

        ulResult = AT_OK;

    }
    else
    {
        ulResult = AT_ConvertCallError(pstCdurCnf->stGetCdurPara.enCause);

        /* ����At_FormatResultData������ */
        gstAtSendData.usBufLen = 0;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvDrvAgentHkAdcGetRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_HKADC_GET_CNF_STRU      *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_HKADC_GET_CNF_STRU *)pstRcvMsg->aucContent;

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

    /* ATģ���ڵȴ�HKADC��ѹ��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_TBAT_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    gstAtSendData.usBufLen = 0;
    if (pstEvent->enResult == DRV_AGENT_HKADC_GET_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s:1,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstEvent->TbatHkadc);

        ulRet                            = AT_OK;

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



VOS_UINT32 AT_RcvDrvAgentAppdmverQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_APPDMVER_QRY_CNF_STRU    *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_APPDMVER_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentAppdmverQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentAppdmverQryRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�APPDMVER��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_APPDMVER_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_APPDMVER_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK           ����ṹΪ^APPDMVER:<pdmver>��ʽ */
        ulRet                  = AT_OK;
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR*)pgucAtSndCodeAddr,
                                                        "%s:%s",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstEvent->acPdmver);

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* 4. ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentDloadverQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_DLOADVER_QRY_CNF_STRU    *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_DLOADVER_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentDloadverQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentDloadverQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�APPDMVER��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DLOADVER_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_DLOADVER_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK           ����ṹΪ<dloadver>��ʽ */
        ulRet                  = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s",
                                                        pstEvent->aucVersionInfo);
    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentAuthVerQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_AUTHVER_QRY_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_AUTHVER_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentAuthVerQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentAuthVerQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�AUTHVER��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_AUTHVER_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_AUTHVER_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK           ����ṹΪ<CR><LF>^ AUTHVER: <value> <CR><LF>
             <CR><LF>OK<CR><LF>��ʽ */
        ulRet                  = AT_OK;
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR*)pgucAtSndCodeAddr,
                                                        "%s:%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstEvent->ulSimLockVersion);

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentFlashInfoQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    DRV_AGENT_FLASHINFO_QRY_CNF_STRU   *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_FLASHINFO_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentFlashInfoQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentFlashInfoQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�^FLASHINFO��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_FLASHINFO_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_FLASHINFO_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK
               ����ṹΪ<CR><LF>~~~~~~FLASH INFO~~~~~~:<CR><LF>
                <CR><LF>MMC BLOCK COUNT:<blockcount>,
                     PAGE SIZE:<pagesize>,
                     PAGE COUNT PER BLOCK:<blocksize><CR><LF>
                <CR><LF>OK<CR><LF>��ʽ */
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           "%s%s",
                                           "~~~~~~FLASH INFO~~~~~~:",
                                            gaucAtCrLf);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "MMC BLOCK COUNT:%d, PAGE SIZE:%d, PAGE COUNT PER BLOCK:%d",
                                            pstEvent->stFlashInfo.ulBlockCount,
                                            pstEvent->stFlashInfo.ulPageSize,
                                            pstEvent->stFlashInfo.ulPgCntPerBlk);

        ulRet     = AT_OK;

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        usLength  = 0;
        ulRet     = AT_ERROR;
    }

    /* ����At_FormatResultData������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentDloadInfoQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_DLOADINFO_QRY_CNF_STRU   *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_DLOADINFO_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentDloadInfoQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentDloadInfoQryRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�DLOADINFO��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DLOADINFO_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_DLOADINFO_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK
               ����ṹΪ^DLOADINFO:<CR><LF>
             <CR><LF>swver:<software version><CR><LF>
             <CR><LF>isover:<iso version><CR><LF>
             <CR><LF>product name:<product name><CR><LF>
             <CR><LF>product name:<WebUiVer><CR><LF>
             <CR><LF>dload type: <dload type><CR><LF>
             <CR><LF>OK<CR><LF>��ʽ */
        ulRet                  = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                                        "%s",
                                                        pstEvent->aucDlodInfo);

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentHwnatQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_HWNATQRY_QRY_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_HWNATQRY_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentHwnatQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentHwnatQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�HWNAT��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_HWNATQRY_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_HWNATQRY_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK
           ����ṹΪ^HWNATQRY: <cur_mode> ��ʽ */
        ulRet                  = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        "%s:%d",
                                                         g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                         pstEvent->ulNetMode);
    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentAuthorityVerQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                           ulRet;
    VOS_UINT8                            ucIndex;
    DRV_AGENT_AUTHORITYVER_QRY_CNF_STRU *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                  *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_AUTHORITYVER_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentAuthorityVerQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentAuthorityVerQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�AUTHORITYVER��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_AUTHORITYVER_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� :  */
    if (pstEvent->enResult == DRV_AGENT_AUTHORITYVER_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK ��ʽΪ<CR><LF><Authority Version><CR><LF>
             <CR><LF>OK<CR><LF> */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s",
                                                        pstEvent->aucAuthority);

        ulRet = AT_OK;
    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        gstAtSendData.usBufLen = 0;
        ulRet                  = AT_ERROR;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);

    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentAuthorityIdQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_AUTHORITYID_QRY_CNF_STRU *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_AUTHORITYID_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentAuthorityIdQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentAuthorityIdQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�AUTHORITYID��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_AUTHORITYID_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� :  */
    if (pstEvent->enResult == DRV_AGENT_AUTHORITYID_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK ��ʽΪ<CR><LF><Authority ID>, <Authority Type><CR><LF>
             <CR><LF>OK<CR><LF> */
        gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s",
                                                        pstEvent->aucAuthorityId);

        ulRet                   = AT_OK;
    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                   = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}

#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)

VOS_UINT32 AT_RcvDrvAgentGodloadSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_GODLOAD_SET_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_GODLOAD_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentGodloadSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentGodloadSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�GODLOAD��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_GODLOAD_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    gstAtSendData.usBufLen = 0;
    if (pstEvent->enResult == DRV_AGENT_GODLOAD_SET_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK */
        ulRet = AT_OK;
    }
    else
    {
        /* ����ʧ�ܷ���ERROR�ַ��� */
        ulRet = AT_ERROR;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvDrvAgentPfverQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_PFVER_QRY_CNF_STRU       *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_PFVER_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentPfverQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentPfverQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�PFVER��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PFVER_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_PFVER_QRY_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK           ����ṹΪ<CR><LF>^PFVER: <PfVer>,<VerTime> <CR><LF>
             <CR><LF>OK<CR><LF>��ʽ */
        ulRet                  = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s:\"%s %s\"",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstEvent->stPfverInfo.aucPfVer,
                                          pstEvent->stPfverInfo.acVerTime);

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentSdloadSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_SDLOAD_SET_CNF_STRU      *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_SDLOAD_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentSdloadSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSdloadSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�SDLOAD��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SDLOAD_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    gstAtSendData.usBufLen = 0;
    if (pstEvent->enResult == DRV_AGENT_SDLOAD_SET_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK */
        ulRet = AT_OK;
    }
    else
    {
        /* ����ʧ�ܷ���ERROR�ַ��� */
        ulRet = AT_ERROR;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentCpuloadQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_CPULOAD_QRY_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    usLength               = 0;
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_CPULOAD_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentCpuloadQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentCpuloadQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�CPULOAD��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CPULOAD_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    gstAtSendData.usBufLen = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    for ( i = 0; i < pstEvent->ulCurCpuLoadCnt; i++ )
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%u.%02u",
                                           pstEvent->aulCurCpuLoad[i] / 100,
                                           pstEvent->aulCurCpuLoad[i] % 100);

        if ( (pstEvent->ulCurCpuLoadCnt - 1) == i )
        {
            break;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",");
    }

    /* ����At_FormatResultData������ */
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentMfreelocksizeQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                                  ulRet;
    VOS_UINT8                                   ucIndex;
    DRV_AGENT_MFREELOCKSIZE_QRY_CNF_STRU       *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                         *pstRcvMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulACoreMemfreeSize;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_MFREELOCKSIZE_QRY_CNF_STRU *)pstRcvMsg->aucContent;
    ulACoreMemfreeSize     = 0;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentMfreelocksizeQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentMfreelocksizeQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�MFREELOCKSIZE��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MFREELOCKSIZE_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    gstAtSendData.usBufLen = 0;
    if (pstEvent->enResult == DRV_AGENT_MFREELOCKSIZE_QRY_NO_ERROR)
    {

        /* ��ȡA�˵�ʣ��ϵͳ�ڴ� */
        ulACoreMemfreeSize = FREE_MEM_SIZE_GET();


        /* ���ڵ����ص���KB��תΪ�ֽ� */
        ulACoreMemfreeSize *= AT_KB_TO_BYTES_NUM;

        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR*)pgucAtSndCodeAddr,
                                                        "%s:%d,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        pstEvent->lMaxFreeLockSize,
                                                        ulACoreMemfreeSize);


        /* ���ô�����ΪAT_OK */
        ulRet = AT_OK;
    }
    else
    {
        /* ����ʧ�ܷ���ERROR�ַ��� */
        gstAtSendData.usBufLen = 0;
        ulRet                  = AT_ERROR;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentImsiChgQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    DRV_AGENT_IMSICHG_QRY_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_IMSICHG_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentImsiChgQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentImsiChgQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�IMSICHG��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_IMSICHG_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR*)pgucAtSndCodeAddr,
                                                "^IMSICHG: %d,%d",
                                                pstEvent->usDualIMSIEnable,
                                                pstEvent->ulCurImsiSign);

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentInfoRbuSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    DRV_AGENT_INFORBU_SET_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_INFORBU_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentInfoRbuSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentInfoRbuSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�INFORBU��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_INFORBU_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    if (pstEvent->ulRslt == NV_OK)
    {
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)

VOS_UINT32 AT_RcvDrvAgentInfoRrsSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    DRV_AGENT_INFORRS_SET_CNF_STRU     *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_INFORRS_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�INFORRU��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_INFORRS_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ò������ :  */
    if (pstEvent->ulRslt == NV_OK)
    {
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}
#endif


VOS_UINT32 AT_RcvDrvAgentCpnnQryRsp(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    DRV_AGENT_CPNN_QRY_CNF_STRU        *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_CPNN_QRY_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentCpnnQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentCpnnQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�CPNN��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CPNN_READ)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ( pstEvent->bNormalSrvStatus == VOS_TRUE )
    {
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentCpnnTestRsp(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_INT                             lBufLen;
    DRV_AGENT_CPNN_TEST_CNF_STRU       *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_CPNN_TEST_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentCpnnTestRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentCpnnTestRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�CPNN��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CPNN_TEST)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if ( ( pstEvent->ulOplExistFlg == PS_USIM_SERVICE_AVAILIABLE )
      && ( pstEvent->ulPnnExistFlg == PS_USIM_SERVICE_AVAILIABLE )
      && ( pstEvent->bNormalSrvStatus == VOS_TRUE ) )
    {
        lBufLen = VOS_sprintf_s((TAF_CHAR*)pgucAtSndCodeAddr,
                                        AT_CMD_MAX_LEN + 20 - 3,
                                        "%s:(0,1)",
                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        if (lBufLen < 0)
        {
            At_FormatResultData(ucIndex, AT_ERROR);
        }
        else
        {
            gstAtSendData.usBufLen = (VOS_UINT16)lBufLen;
            At_FormatResultData(ucIndex, AT_OK);
        }
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentNvBackupSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    DRV_AGENT_NVBACKUP_SET_CNF_STRU    *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_NVBACKUP_SET_CNF_STRU *)pstRcvMsg->aucContent;

    AT_PR_LOGI("Rcv Msg");

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentNvBackupSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentNvBackupSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�NVBACKUP��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NVBACKUP_SET)
    {
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstEvent->ulRslt == NV_OK)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%d",
                                                        pstEvent->ulRslt);
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvDrvAgentMemInfoQryRsp(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                 *pstRcvMsg            = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_MEMINFO_QRY_RSP_STRU     *pstCCpuMemInfoCnfMsg = VOS_NULL_PTR;
    AT_PID_MEM_INFO_PARA_STRU          *pstPidMemInfo        = VOS_NULL_PTR;
    VOS_UINT32                          ulACpuMemBufSize;
    VOS_UINT32                          ulACpuPidTotal;
    VOS_UINT16                          usAtLength;
    VOS_UINT32                          i;

    /* ��ʼ�� */
    pstRcvMsg            = (DRV_AGENT_MSG_STRU *)pMsg;
    pstCCpuMemInfoCnfMsg = (DRV_AGENT_MEMINFO_QRY_RSP_STRU *)pstRcvMsg->aucContent;

    /* ָ��CCPU��ÿ��PID���ڴ���Ϣ */
    pstPidMemInfo       = (AT_PID_MEM_INFO_PARA_STRU *)pstCCpuMemInfoCnfMsg->aucData;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCCpuMemInfoCnfMsg->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG("AT_RcvDrvAgentMemInfoQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* Client Id Ϊ�㲥Ҳ����ERROR */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_ERR_LOG("AT_RcvDrvAgentMemInfoQryRsp: AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�MEMINFO��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_MEMINFO_READ)
    {
        AT_ERR_LOG("AT_RcvDrvAgentMemInfoQryRsp: CmdCurrentOpt Error!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);


    /* �Է��ؽ���еĲ������м�� */
    if ( (pstCCpuMemInfoCnfMsg->ulResult != VOS_OK)
      || ( (pstCCpuMemInfoCnfMsg->ulMemQryType != AT_MEMQUERY_TTF)
        && (pstCCpuMemInfoCnfMsg->ulMemQryType != AT_MEMQUERY_VOS) ) )
    {

        /* ����At_FormatResultData����ERROR�ַ��� */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_OK;
    }

    /* �ȴ�ӡ�������� */
    usAtLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        "%s:%s",
                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                        gaucAtCrLf);




    usAtLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)pgucAtSndCodeAddr + usAtLength,
                                         "C CPU Pid:%d%s",
                                         pstCCpuMemInfoCnfMsg->ulPidNum,
                                         gaucAtCrLf);

    /* ���δ�ӡC CPUÿ��PID���ڴ���Ϣ */
    for (i = 0; i < pstCCpuMemInfoCnfMsg->ulPidNum; i++)
    {
        usAtLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + usAtLength,
                                             "%d,%d,%d%s",
                                             pstPidMemInfo[i].ulPid,
                                             pstPidMemInfo[i].ulMsgPeakSize,
                                             pstPidMemInfo[i].ulMemPeakSize,
                                             gaucAtCrLf);
    }

    /* ����ǲ�ѯVOS�ڴ棬���ȡ����ӡA CPU��VOS�ڴ�ʹ�����.
       ��������A CPU��TTF�ڴ��ѯ�ӿڣ�TTF���Ͳ�ѯֻ��ӡC CPU��TTF�ڴ�ʹ����� */
    if (pstCCpuMemInfoCnfMsg->ulMemQryType == AT_MEMQUERY_VOS)
    {
        ulACpuMemBufSize = AT_PID_MEM_INFO_LEN * sizeof(AT_PID_MEM_INFO_PARA_STRU);

        /* �����ڴ��Բ�ѯA CPU��VOS�ڴ�ʹ����Ϣ */
        /*lint -save -e516 */
        pstPidMemInfo = (AT_PID_MEM_INFO_PARA_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, ulACpuMemBufSize);
        /*lint -restore */
        if (pstPidMemInfo != VOS_NULL_PTR)
        {
            memset_s(pstPidMemInfo, ulACpuMemBufSize, 0x00, ulACpuMemBufSize);


            ulACpuPidTotal = 0;

            if (VOS_AnalyzePidMemory(pstPidMemInfo, ulACpuMemBufSize, &ulACpuPidTotal) != VOS_ERR)
            {

                /* ���δ�ӡ A CPU��ÿ��PID���ڴ�ʹ����� */
                usAtLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr + usAtLength,
                                                     "A CPU Pid:%d%s",
                                                     ulACpuPidTotal,
                                                     gaucAtCrLf);

                for (i = 0; i < ulACpuPidTotal; i++)
                {
                    usAtLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + usAtLength,
                                                         "%d,%d,%d%s",
                                                         pstPidMemInfo[i].ulPid,
                                                         pstPidMemInfo[i].ulMsgPeakSize,
                                                         pstPidMemInfo[i].ulMemPeakSize,
                                                         gaucAtCrLf);
                }

            }

            /* �ͷ��ڴ� */
            /*lint -save -e516 */
            PS_MEM_FREE(WUEPS_PID_AT, pstPidMemInfo);
            /*lint -restore */
        }
    }

    gstAtSendData.usBufLen = usAtLength;

    /* ����At_FormatResultData����OK�ַ��� */
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


TAF_VOID At_QryParaRspProc  (
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_PARA_TYPE                       QueryType,
    TAF_UINT16                          usErrorCode,
    TAF_VOID                            *pPara
)
{

    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;
    TAF_UINT32                          ulTmp;
    TAF_UINT32                          i;

    if(usErrorCode != 0)  /* ���� */
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        ulResult = At_ChgTafErrorCode(ucIndex,usErrorCode);       /* �������� */
        gstAtSendData.usBufLen = usLength;
        At_FormatResultData(ucIndex,ulResult);
        return;
    }

    if(pPara == TAF_NULL_PTR)   /* �����ѯ���� */
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        At_FormatResultData(ucIndex,AT_CME_UNKNOWN);

        return;
    }

    ulTmp = (sizeof(g_aAtQryTypeProcFuncTbl) / sizeof(g_aAtQryTypeProcFuncTbl[0]));
    for (i = 0; i != ulTmp; i++ )
    {
        if (QueryType == g_aAtQryTypeProcFuncTbl[i].QueryType)
        {
            if (QueryType != TAF_PH_ICC_ID)
            {
                AT_STOP_TIMER_CMD_READY(ucIndex);
            }

            g_aAtQryTypeProcFuncTbl[i].AtQryParaProcFunc(ucIndex,OpId,pPara);

            return;
        }
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    AT_WARN_LOG("At_QryParaRspProc QueryType FAILURE");
    return;
}

TAF_VOID At_QryMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    errno_t                             lMemResult;
    TAF_UINT16                          usClientId = 0;
    TAF_UINT8                           OpId = 0;
    TAF_PARA_TYPE                       QueryType = 0;
    TAF_UINT16                          usErrorCode = 0;
    TAF_VOID                           *pPara = TAF_NULL_PTR;
    TAF_UINT16                          usAddr = 0;
    TAF_UINT16                          usParaLen = 0;
    TAF_UINT8                           ucIndex  = 0;

    lMemResult = memcpy_s(&usClientId, sizeof(usClientId), pData, sizeof(usClientId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(usClientId), sizeof(usClientId));
    usAddr += sizeof(usClientId);

    lMemResult = memcpy_s(&OpId, sizeof(OpId), pData+usAddr, sizeof(OpId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(OpId), sizeof(OpId));
    usAddr += sizeof(OpId);

    lMemResult = memcpy_s(&QueryType, sizeof(QueryType), pData+usAddr, sizeof(QueryType));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(QueryType), sizeof(QueryType));
    usAddr += sizeof(QueryType);

    lMemResult = memcpy_s(&usErrorCode, sizeof(usErrorCode), pData+usAddr, sizeof(usErrorCode));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(usErrorCode), sizeof(usErrorCode));
    usAddr += sizeof(usErrorCode);

    lMemResult = memcpy_s(&usParaLen, sizeof(usParaLen), pData+usAddr, sizeof(usParaLen));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(usParaLen), sizeof(usParaLen));
    usAddr += sizeof(usParaLen);

    if(usParaLen != 0)
    {
        pPara = pData+usAddr;
    }

    AT_LOG1("At_QryMsgProc ClientId",usClientId);
    AT_LOG1("At_QryMsgProc QueryType",QueryType);
    AT_LOG1("At_QryMagProc usErrorCode", usErrorCode);
    if(usClientId == AT_BUTT_CLIENT_ID)
    {
        AT_WARN_LOG("At_QryMsgProc Error ucIndex");
        return;
    }
    else
    {
        if(At_ClientIdToUserId(usClientId,&ucIndex) == AT_FAILURE)
        {
            AT_WARN_LOG("At_QryMsgProc At_ClientIdToUserId FAILURE");
            return;
        }

        if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
        {
            AT_WARN_LOG("At_QryMsgProc: AT_BROADCAST_INDEX.");
            return;
        }

        AT_LOG1("At_QryMsgProc ucIndex",ucIndex);
        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

        At_QryParaRspProc(ucIndex,OpId,QueryType,usErrorCode,pPara);
    }
}


TAF_UINT32 At_PIHNotBroadIndProc(TAF_UINT8 ucIndex, SI_PIH_EVENT_INFO_STRU *pEvent)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_PIHNotBroadIndProc: Get modem id fail.");
        return VOS_ERR;
    }

    switch(pEvent->EventType)
    {
        case SI_PIH_EVENT_PRIVATECGLA_SET_IND:

            /* ^CGLA��ѯ�����·�����ж���IND�ϱ���ͨ����ǰͨ���ϱ�����Ҫ�㲥 */
            usLength += At_PrintPrivateCglaResult(ucIndex, pEvent);
            break;

        default:
            AT_WARN_LOG("At_PIHNotBroadIndProc: Abnormal EventType.");
            return VOS_ERR;
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


TAF_VOID At_PIHIndProc(TAF_UINT8 ucIndex, SI_PIH_EVENT_INFO_STRU *pEvent)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_PIHIndProc: Get modem id fail.");
        return;
    }

    switch(pEvent->EventType)
    {
        case SI_PIH_EVENT_HVRDH_IND:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^HVRDH: %d%s",
                                               gaucAtCrLf,
                                               pEvent->PIHEvent.HvrdhInd.ulReDhFlag,
                                               gaucAtCrLf);
            break;

        case SI_PIH_EVENT_TEETIMEOUT_IND:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^TEETIMEOUT: %d%s",
                                               gaucAtCrLf,
                                               pEvent->PIHEvent.TEETimeOut.ulData,
                                               gaucAtCrLf);
            break;

        case SI_PIH_EVENT_SIM_ERROR_IND:
            usLength += At_CardErrorInfoInd(ucIndex, pEvent);
            break;

        case SI_PIH_EVENT_SIM_ICCID_IND:
            usLength += At_CardIccidInfoInd(ucIndex, pEvent);
            break;
        case SI_PIH_EVENT_SIM_HOTPLUG_IND:
            usLength += At_SimHotPlugStatusInd(ucIndex, pEvent);
            break;

        case SI_PIH_EVENT_SW_CHECK_IND:
            usLength += At_SWCheckStatusInd(pEvent);
            break;

        case SI_PIH_EVENT_CARDSTATUS_IND:
            usLength += At_CardStatusInd(ucIndex, pEvent);
            break;

        default:
            AT_WARN_LOG("At_PIHIndProc: Abnormal EventType.");
            return;
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}

#if (FEATURE_PHONE_SC == FEATURE_ON)

VOS_UINT32 At_PrintSilentPinInfo(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    VOS_UINT8                          *pucPinIv = VOS_NULL_PTR;
    VOS_UINT32                          i;

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_SILENTPIN_SET/AT_CMD_SILENTPININFO_SET */
    if ( (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SILENTPIN_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SILENTPININFO_SET))
    {
        AT_WARN_LOG("At_PrintSilentPinInfo : CmdCurrentOpt is not AT_CMD_SILENTPIN_SET/AT_CMD_SILENTPININFO_SET!");
        return AT_ERROR;
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr,
                                       "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    for (i = 0; i < DRV_AGENT_PIN_CRYPTO_DATA_LEN; i++)
    {
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%02X",
                                            pstEvent->PIHEvent.stCryptoPin.aucCryptoPin[i]);
    }

    pucPinIv = (VOS_UINT8*)pstEvent->PIHEvent.stCryptoPin.aulPinIv;

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), ",");

    for (i = 0; i < (sizeof(VOS_UINT32) * 4); i++)/* IV����Ϊ16�ֽ� */
    {
        (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%02X",
                                           (*(pucPinIv + i)));
    }

    (*pusLength) += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr + (*pusLength), ",");

    for (i = 0; i < DRV_AGENT_HMAC_DATA_LEN; i++)
    {
        (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%02X",
                                           pstEvent->PIHEvent.stCryptoPin.aucHmacValue[i]);
    }

    return AT_OK;
}
#endif


VOS_UINT32 At_PrintSetEsimSwitchInfo(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_EsimSwitch_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ESIMSWITCH_SET)
    {
        AT_WARN_LOG("At_PrintSetEsimSwitchInfo : CmdCurrentOpt is not AT_CMD_EsimSwitch_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_PrintQryEsimSwitchInfo(
    TAF_UINT8                           ucIndex,
    SI_PIH_EVENT_INFO_STRU             *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_ESIMSWITCH_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ESIMSWITCH_QRY)
    {
        AT_WARN_LOG("At_PrintQryEsimSwitchInfo : CmdCurrentOpt is not AT_CMD_ESIMSWITCH_QRY!");
        return AT_ERROR;
    }

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s^ESIMSWITCH: %d,%d%s",
                                      gaucAtCrLf,
                                      pstEvent->PIHEvent.stSlotCardType.lSlot0CardType,
                                      pstEvent->PIHEvent.stSlotCardType.lSlot1CardType,
                                      gaucAtCrLf);

    return AT_OK;
}


TAF_VOID At_PIHRspProc(TAF_UINT8 ucIndex, SI_PIH_EVENT_INFO_STRU *pEvent)
{
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;
    TAF_UINT32                          ulTmp;
    TAF_UINT32                          i;

    if(pEvent->PIHError != TAF_ERR_NO_ERROR)  /* ���� */
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        ulResult = At_ChgTafErrorCode(ucIndex, (TAF_UINT16)(pEvent->PIHError));       /* �������� */

        gstAtSendData.usBufLen = usLength;

        At_FormatResultData(ucIndex,ulResult);

        return;
    }

    ulTmp = (sizeof(g_aAtPihRspProcFuncTbl) / sizeof(g_aAtPihRspProcFuncTbl[0]));

    for (i = 0; i < ulTmp; i++)
    {
        /* �ҵ������������������ش��� */
        if (pEvent->EventType == g_aAtPihRspProcFuncTbl[i].ulEventType)
        {

            ulResult = g_aAtPihRspProcFuncTbl[i].pAtPihRspProcFunc(ucIndex, pEvent, &usLength);

            if (ulResult == AT_ERROR)
            {
                AT_WARN_LOG("At_PIHRspProc : pAtPihRspProcFunc is return error!");
                return;
            }

            break;
        }
    }

    /* û�ҵ���������ֱ�ӷ��� */
    if (i == ulTmp)
    {
        AT_WARN_LOG("At_PIHRspProc : no find AT Proc Func!");
        return;
    }

    ulResult = AT_OK;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);

    g_enLogPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_BUTT;

    return;
}

/*****************************************************************************
 Prototype      : At_PbIndMsgProc
 Description    : �绰�������ϱ���Ϣ������
 Input          : pEvent --- �¼�����
 Output         :
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : h59254
  1.Date        : 2013-05-29
    Author      : ---
    Modification: Created function
*****************************************************************************/
TAF_VOID At_PbIndMsgProc(SI_PB_EVENT_INFO_STRU *pEvent)
{
    if (pEvent->PBEventType == SI_PB_EVENT_INFO_IND)
    {
        /* ��SIM����FDNʹ��ʱĬ�ϴ洢����FD */
        if ((pEvent->PBEvent.PBInfoInd.CardType == 0)
         && (pEvent->PBEvent.PBInfoInd.FdnState == SI_PIH_STATE_FDN_BDN_ENABLE))
        {
            gstPBATInfo.usNameMaxLen = pEvent->PBEvent.PBInfoInd.FDNTextLen;
            gstPBATInfo.usNumMaxLen  = pEvent->PBEvent.PBInfoInd.FDNNumberLen;
            gstPBATInfo.usTotal      = pEvent->PBEvent.PBInfoInd.FDNRecordNum;
            gstPBATInfo.usAnrNumLen  = pEvent->PBEvent.PBInfoInd.ANRNumberLen;
            gstPBATInfo.usEmailLen   = pEvent->PBEvent.PBInfoInd.EMAILTextLen;
        }
        else
        {
            gstPBATInfo.usNameMaxLen = pEvent->PBEvent.PBInfoInd.ADNTextLen;
            gstPBATInfo.usNumMaxLen  = pEvent->PBEvent.PBInfoInd.ADNNumberLen;
            gstPBATInfo.usTotal      = pEvent->PBEvent.PBInfoInd.ADNRecordNum;
            gstPBATInfo.usAnrNumLen  = pEvent->PBEvent.PBInfoInd.ANRNumberLen;
            gstPBATInfo.usEmailLen   = pEvent->PBEvent.PBInfoInd.EMAILTextLen;
        }
    }

    return;
}


VOS_VOID AT_PB_ReadContinueProc(VOS_UINT8 ucIndex)
{
    AT_COMM_PB_CTX_STRU                *pstCommPbCntxt = VOS_NULL_PTR;
    AT_UART_CTX_STRU                   *pstUartCtx     = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    pstCommPbCntxt = AT_GetCommPbCtxAddr();
    pstUartCtx     = AT_GetUartCtxAddr();
    ulResult       = AT_SUCCESS;

    /* ������ͻ����ˮ�߻ص� */
    pstUartCtx->pWmLowFunc = VOS_NULL_PTR;

    /* ���µ�ǰ��ȡ�ĵ绰������ */
    pstCommPbCntxt->usCurrIdx++;

    if (SI_PB_Read(gastAtClientTab[ucIndex].usClientId,
                                  0, SI_PB_STORAGE_UNSPECIFIED,
                                  pstCommPbCntxt->usCurrIdx,
                                  pstCommPbCntxt->usCurrIdx) == TAF_SUCCESS)
    {
        return;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);
    return;
}


VOS_VOID AT_PB_ReadRspProc(
    VOS_UINT8                           ucIndex,
    SI_PB_EVENT_INFO_STRU              *pstEvent
)
{
    AT_COMM_PB_CTX_STRU                *pstCommPbCntxt = VOS_NULL_PTR;
    AT_UART_CTX_STRU                   *pstUartCtx     = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    pstCommPbCntxt = AT_GetCommPbCtxAddr();
    pstUartCtx     = AT_GetUartCtxAddr();
    ulResult       = AT_SUCCESS;

    /*
     * ��ȡ����: ���մ����뷵�ؽ��
     * ��ȡ����: ����δ�ҵ��ĵ绰��
     */
    if (pstCommPbCntxt->ulSingleReadFlg == VOS_TRUE)
    {
        ulResult = (TAF_ERR_NO_ERROR == pstEvent->PBError) ?
                   AT_OK : At_ChgTafErrorCode(ucIndex, (VOS_UINT16)pstEvent->PBError);
    }
    else if ( (pstEvent->PBError == TAF_ERR_NO_ERROR)
           || (pstEvent->PBError == TAF_ERR_PB_NOT_FOUND) )
    {
        /* ��鵱ǰ��ȡλ���Ƿ��Ѿ��������һ�� */
        if (pstCommPbCntxt->usCurrIdx == pstCommPbCntxt->usLastIdx)
        {
            ulResult = AT_OK;
        }
        else
        {
            ulResult = AT_WAIT_ASYNC_RETURN;
        }
    }
    else
    {
        ulResult = At_ChgTafErrorCode(ucIndex, (VOS_UINT16)pstEvent->PBError);
    }

    /* �绰��δ����, ��ȡ��һ���绰�� */
    if (ulResult == AT_WAIT_ASYNC_RETURN)
    {
        /*
         * ������ͻ����Ѿ������ˮ��:
         * ע���ˮ�߻ص�, �����ͻ��浽���ˮ�ߺ������ȡ��һ���绰��
         */
        if (pstUartCtx->ulTxWmHighFlg == VOS_TRUE)
        {
            pstUartCtx->pWmLowFunc = AT_PB_ReadContinueProc;
            return;
        }

        /* ���µ�ǰ��ȡ�ĵ绰������ */
        pstCommPbCntxt->usCurrIdx++;

        if (SI_PB_Read(gastAtClientTab[ucIndex].usClientId,
                                      0, SI_PB_STORAGE_UNSPECIFIED,
                                      pstCommPbCntxt->usCurrIdx,
                                      pstCommPbCntxt->usCurrIdx) == TAF_SUCCESS)
        {
            return;
        }
        else
        {
            ulResult = AT_ERROR;
        }
    }

    gstAtSendData.usBufLen = 0;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);
    return;
}


TAF_VOID At_PbRspProc(TAF_UINT8 ucIndex,SI_PB_EVENT_INFO_STRU *pEvent)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;

    if (!( (VOS_TRUE == AT_CheckHsUartUser(ucIndex))
        && ( (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR2_SET)
          || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR_SET)) ) )
    {
        if(pEvent->PBError != TAF_ERR_NO_ERROR)  /* ���� */
        {
            ulResult = At_ChgTafErrorCode(ucIndex,(TAF_UINT16)pEvent->PBError);
            gstAtSendData.usBufLen = usLength;
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex,ulResult);
            return;
        }
    }

    switch(pEvent->PBEventType)
    {
        case SI_PB_EVENT_SET_CNF:
            gstPBATInfo.usNameMaxLen = pEvent->PBEvent.PBSetCnf.TextLen;
            gstPBATInfo.usNumMaxLen  = pEvent->PBEvent.PBSetCnf.NumLen;
            gstPBATInfo.usTotal      = pEvent->PBEvent.PBSetCnf.TotalNum;
            gstPBATInfo.usUsed       = pEvent->PBEvent.PBSetCnf.InUsedNum;
            gstPBATInfo.usAnrNumLen  = pEvent->PBEvent.PBSetCnf.ANRNumberLen;
            gstPBATInfo.usEmailLen   = pEvent->PBEvent.PBSetCnf.EMAILTextLen;

            AT_STOP_TIMER_CMD_READY(ucIndex);

            ulResult = AT_OK;

            break;

        case SI_PB_EVENT_READ_CNF:
        case SI_PB_EVENT_SREAD_CNF:
            if((pEvent->PBError == TAF_ERR_NO_ERROR)&&
                (pEvent->PBEvent.PBReadCnf.PBRecord.ValidFlag == SI_PB_CONTENT_VALID))/*��ǰ��������Ч*/
            {
                if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_D_GET_NUMBER_BEFORE_CALL)
                {
                    ulResult = At_DialNumByIndexFromPb(ucIndex,pEvent);
                    if(ulResult == AT_WAIT_ASYNC_RETURN)
                    {
                        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_PEND;


                        /* ����ʱ�� */
                        if(At_StartTimer(g_stParseContext[ucIndex].pstCmdElement->ulSetTimeOut, ucIndex) != AT_SUCCESS)
                        {
                            AT_ERR_LOG("At_PbRspProc:ERROR:Start Timer");
                        }
                        ulResult = AT_SUCCESS;

                    }
                    else
                    {
                        ulResult = AT_ERROR;
                    }

                    break;

                }

                if (gulPBPrintTag == TAF_FALSE)
                {
                     usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCrLfAddr,(TAF_CHAR *)pgucAtSndCrLfAddr,"%s","\r\n");
                }

                gulPBPrintTag = TAF_TRUE;

                if(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR_SET) /*���� ^CPBR �ķ�ʽ���д�ӡ*/
                {
                    ulResult = At_PbCPBRCmdPrint(ucIndex,&usLength,pgucAtSndCrLfAddr,pEvent);
                }
                else if(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR2_SET) /*���� +CPBR �ķ�ʽ���д�ӡ*/
                {
                    ulResult = At_PbCPBR2CmdPrint(ucIndex,&usLength,pgucAtSndCrLfAddr,pEvent);
                }
                else if(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_SCPBR_SET) /*���� ^SCPBR �ķ�ʽ���д�ӡ*/
                {
                    ulResult = At_PbSCPBRCmdPrint(ucIndex,&usLength,pEvent);
                }
                else if(gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CNUM_READ) /*���� CNUM �ķ�ʽ���д�ӡ*/
                {
                    ulResult = At_PbCNUMCmdPrint(ucIndex,&usLength,pgucAtSndCrLfAddr,pEvent);
                }
                else
                {
                    AT_ERR_LOG1("At_PbRspProc: the Cmd Current Opt %d is Unknow", gastAtClientTab[ucIndex].CmdCurrentOpt);

                    return ;
                }

                if(ulResult == AT_SUCCESS)
                {
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCrLfAddr,
                                        (TAF_CHAR *)pgucAtSndCrLfAddr + usLength,
                                        "%s","\r\n");
                }

                At_SendResultData(ucIndex, pgucAtSndCrLfAddr, usLength);

                usLength = 0;
            }

            lMemResult = memcpy_s((TAF_CHAR *)pgucAtSndCrLfAddr, AT_CRLF_STR_LEN, (TAF_CHAR *)gaucAtCrLf, AT_CRLF_STR_LEN);
            TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CRLF_STR_LEN, AT_CRLF_STR_LEN);

            if ( (AT_CheckHsUartUser(ucIndex) == VOS_TRUE)
              && ( (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR2_SET)
                || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR_SET)) )
            {
                AT_PB_ReadRspProc(ucIndex, pEvent);
                return;
            }

            if( (gulPBPrintTag == VOS_FALSE)
             && ( (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR_SET)
               || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CPBR2_SET) ) )
            {
                pEvent->PBError = TAF_ERR_ERROR;
            }

            if(pEvent->PBError != TAF_ERR_NO_ERROR)  /* ���� */
            {
                ulResult = At_ChgTafErrorCode(ucIndex,(TAF_UINT16)pEvent->PBError);       /* �������� */

                gstAtSendData.usBufLen = usLength;
            }
            else
            {
                ulResult = AT_OK;
            }

            AT_STOP_TIMER_CMD_READY(ucIndex);
            break;

        case SI_PB_EVENT_SEARCH_CNF:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s","\r");
            AT_STOP_TIMER_CMD_READY(ucIndex);
            ulResult = AT_OK;
            break;

        case SI_PB_EVENT_ADD_CNF:
        case SI_PB_EVENT_SADD_CNF:
        case SI_PB_EVENT_MODIFY_CNF:
        case SI_PB_EVENT_SMODIFY_CNF:
        case SI_PB_EVENT_DELETE_CNF:
            AT_STOP_TIMER_CMD_READY(ucIndex);
            ulResult = AT_OK;
            break;

        case SI_PB_EVENT_QUERY_CNF:

            if(pEvent->OpId == 1)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: (\"SM\",\"EN\",\"ON\",\"FD\")",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                ulResult = AT_OK;
                AT_STOP_TIMER_CMD_READY(ucIndex);
                break;
            }

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

            gstPBATInfo.usNameMaxLen = pEvent->PBEvent.PBQueryCnf.TextLen;
            gstPBATInfo.usNumMaxLen  = pEvent->PBEvent.PBQueryCnf.NumLen;
            gstPBATInfo.usTotal      = pEvent->PBEvent.PBQueryCnf.TotalNum;
            gstPBATInfo.usUsed       = pEvent->PBEvent.PBQueryCnf.InUsedNum;
            gstPBATInfo.usAnrNumLen  = pEvent->PBEvent.PBQueryCnf.ANRNumberLen;
            gstPBATInfo.usEmailLen   = pEvent->PBEvent.PBQueryCnf.EMAILTextLen;

            switch(g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex)
            {
                case AT_CMD_CPBR:
                case AT_CMD_CPBR2:
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"(1-%d),%d,%d",gstPBATInfo.usTotal,gstPBATInfo.usNumMaxLen,gstPBATInfo.usNameMaxLen);
                    break;
                case AT_CMD_CPBW:
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"(1-%d),%d,(128-255),%d",gstPBATInfo.usTotal,gstPBATInfo.usNumMaxLen,gstPBATInfo.usNameMaxLen);
                    break;
                case AT_CMD_CPBW2:
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"(1-%d),%d,(128-255),%d",gstPBATInfo.usTotal,gstPBATInfo.usNumMaxLen,gstPBATInfo.usNameMaxLen);
                    break;
                case AT_CMD_SCPBR:
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"(1-%d),%d,%d,%d",gstPBATInfo.usTotal,gstPBATInfo.usNumMaxLen,gstPBATInfo.usNameMaxLen,gstPBATInfo.usEmailLen);
                    break;
                case AT_CMD_SCPBW:
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"(1-%d),%d,(128-255),%d,%d",gstPBATInfo.usTotal,gstPBATInfo.usNumMaxLen,gstPBATInfo.usNameMaxLen,gstPBATInfo.usEmailLen);
                    break;
                case AT_CMD_CPBS:
                    if(pEvent->Storage == SI_PB_STORAGE_SM)
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gastAtStringTab[AT_STRING_SM].pucText);
                    }
                    else if(pEvent->Storage == SI_PB_STORAGE_FD)
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gastAtStringTab[AT_STRING_FD].pucText);
                    }
                    else if(pEvent->Storage == SI_PB_STORAGE_ON)
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gastAtStringTab[AT_STRING_ON].pucText);
                    }
                    else if(pEvent->Storage == SI_PB_STORAGE_BD)
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gastAtStringTab[AT_STRING_BD].pucText);
                    }
                    else
                    {
                        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gastAtStringTab[AT_STRING_EN].pucText);
                    }

                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",pEvent->PBEvent.PBQueryCnf.InUsedNum);
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",pEvent->PBEvent.PBQueryCnf.TotalNum);

                    break;
                case AT_CMD_CPBF:
                    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d,%d",gstPBATInfo.usNumMaxLen,gstPBATInfo.usNameMaxLen);
                    break;

                default:
                    break;
            }

            ulResult = AT_OK;

            AT_STOP_TIMER_CMD_READY(ucIndex);

            break;

        default:
            AT_ERR_LOG1("At_PbRspProc Unknow Event %d", pEvent->PBEventType);
            break;
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);
}


TAF_VOID At_TAFPbMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    SI_PB_EVENT_INFO_STRU              *pEvent = TAF_NULL_PTR;
    errno_t                             lMemResult;
    TAF_UINT8                           ucIndex = 0;
    /*lint -save -e516 */
    pEvent = (SI_PB_EVENT_INFO_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, usLen);
    /*lint -restore */
    if(pEvent == TAF_NULL_PTR)
    {
        return;
    }

    lMemResult = memcpy_s(pEvent, usLen, pData, usLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usLen, usLen);

    AT_LOG1("At_PbMsgProc pEvent->ClientId",pEvent->ClientId);
    AT_LOG1("At_PbMsgProc PBEventType",pEvent->PBEventType);
    AT_LOG1("At_PbMsgProc Event Error",pEvent->PBError);

    if (At_ClientIdToUserId(pEvent->ClientId, &ucIndex) == AT_FAILURE)
    {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, pEvent);
        /*lint -restore */
        AT_WARN_LOG("At_TAFPbMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT,pEvent);
        /*lint -restore */
        AT_WARN_LOG("At_TAFPbMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_PbMsgProc ucIndex",ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    At_PbRspProc(ucIndex,pEvent);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT,pEvent);
    /*lint -restore */
    return;
}


TAF_VOID At_PbMsgProc(VOS_VOID* pMsg)
{
    MN_APP_PB_AT_CNF_STRU   *pstMsg = VOS_NULL_PTR;
    TAF_UINT8               ucIndex = 0;

    pstMsg = (MN_APP_PB_AT_CNF_STRU*)pMsg;

    AT_LOG1("At_PbMsgProc pEvent->ClientId",    pstMsg->stPBAtEvent.ClientId);
    AT_LOG1("At_PbMsgProc PBEventType",         pstMsg->stPBAtEvent.PBEventType);
    AT_LOG1("At_PbMsgProc Event Error",         pstMsg->stPBAtEvent.PBError);

    if (At_ClientIdToUserId(pstMsg->stPBAtEvent.ClientId, &ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG1("At_PbMsgProc At_ClientIdToUserId FAILURE", pstMsg->stPBAtEvent.ClientId);
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_PbMsgProc: AT_BROADCAST_INDEX.");
        At_PbIndMsgProc(&pstMsg->stPBAtEvent);
        return;
    }

    AT_LOG1("At_PbMsgProc ucIndex",ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    At_PbRspProc(ucIndex,&pstMsg->stPBAtEvent);

    return;
}


TAF_VOID At_PIHMsgProc(VOS_VOID* pMsg)
{
    MN_APP_PIH_AT_CNF_STRU  *pstMsg = VOS_NULL_PTR;
    TAF_UINT8               ucIndex = 0;

    pstMsg = (MN_APP_PIH_AT_CNF_STRU*)pMsg;

    if(pstMsg->ulMsgId != PIH_AT_EVENT_CNF)
    {
        AT_ERR_LOG1("At_PIHMsgProc: The Msg Id is Wrong", pstMsg->ulMsgId);
        return;
    }

    AT_LOG1("At_PIHMsgProc pEvent->ClientId",   pstMsg->stPIHAtEvent.ClientId);
    AT_LOG1("At_PIHMsgProc EventType",          pstMsg->stPIHAtEvent.EventType);
    AT_LOG1("At_PIHMsgProc SIM Event Error",    pstMsg->stPIHAtEvent.PIHError);

    if(At_ClientIdToUserId(pstMsg->stPIHAtEvent.ClientId,&ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG("At_PIHMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        At_PIHIndProc(ucIndex,&pstMsg->stPIHAtEvent);
        AT_WARN_LOG("At_PIHMsgProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_PIHMsgProc ucIndex",ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    /* �ǹ㲥�������ϱ� */
    if (At_PIHNotBroadIndProc(ucIndex,&pstMsg->stPIHAtEvent) == VOS_OK)
    {
        return;
    }

    At_PIHRspProc(ucIndex,&pstMsg->stPIHAtEvent);

    return;
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID At_XsmsIndProc(
    VOS_UINT8                           ucIndex,
    TAF_XSMS_APP_MSG_TYPE_ENUM_UINT32   enEventType,
    TAF_XSMS_APP_AT_EVENT_INFO_STRU    *pstEvent)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_XsmsIndProc: Get modem id fail.");
        return;
    }

    switch(enEventType)
    {
        case TAF_XSMS_APP_MSG_TYPE_SEND_SUCC_IND:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^CCMGSS: %d%s",
                                               gaucAtCrLf,
                                               pstEvent->XSmsEvent.stSndSuccInd.ulMr,
                                               gaucAtCrLf);
            break;

        case TAF_XSMS_APP_MSG_TYPE_SEND_FAIL_IND:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^CCMGSF: %d%s",
                                               gaucAtCrLf,
                                               pstEvent->XSmsEvent.stSndFailInd.ulCourseCode,
                                               gaucAtCrLf);
            break;

        case TAF_XSMS_APP_MSG_TYPE_RCV_IND:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^CCMT:",
                                               gaucAtCrLf);
            /* <length>, */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d,\"",
                                               2 * sizeof(TAF_XSMS_MESSAGE_STRU)); /*lint !e559 */
            /* <PDU> */
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                            (TAF_UINT8 *)&pstEvent->XSmsEvent.stRcvInd.stRcvMsg,
                                                            sizeof(TAF_XSMS_MESSAGE_STRU));

            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "\"%s",
                                               gaucAtCrLf);

            break;

        case TAF_XSMS_APP_MSG_TYPE_UIM_FULL_IND:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^CSMMEMFULL: \"SM\"%s",
                                               gaucAtCrLf,
                                               gaucAtCrLf);
            break;

        default:
            AT_WARN_LOG("At_XsmsIndProc: Abnormal EventType.");
            return;
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID At_XsmsCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_XSMS_APP_MSG_TYPE_ENUM_UINT32   enEventType,
    TAF_XSMS_APP_AT_EVENT_INFO_STRU    *pstEvent)
{
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;

    if (pstEvent->ulError != TAF_ERR_NO_ERROR)  /* ���� */
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        ulResult = At_ChgXsmsErrorCodeToAt(pstEvent->ulError);       /* �������� */

        gstAtSendData.usBufLen = usLength;

        At_FormatResultData(ucIndex, ulResult);

        return;
    }

    switch (enEventType)
    {
        /* ʲô���������͵ȴ�ӡOK */
        case TAF_XSMS_APP_MSG_TYPE_SEND_CNF:
            break;

        case TAF_XSMS_APP_MSG_TYPE_WRITE_CNF:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "^CCMGW: %d",
                                               pstEvent->XSmsEvent.stWriteCnf.ulIndex - 1);
            break;

        /* ʲô���������͵ȴ�ӡOK */
        case TAF_XSMS_APP_MSG_TYPE_DELETE_CNF:

            break;

        /* ʲô���������͵ȴ�ӡOK */
        case TAF_XSMS_APP_MSG_TYPE_UIM_MEM_FULL_CNF:

            break;

        default:
            return;
    }

    ulResult = AT_OK;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);
}


VOS_VOID AT_ProcXsmsMsg(VOS_VOID *pMsg)
{
    TAF_XSMS_APP_AT_CNF_STRU           *pstMsg = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex = 0;

    pstMsg = (TAF_XSMS_APP_AT_CNF_STRU *)pMsg;
    /* ��Ϣ���Ͳ���ȷ */
    if (pstMsg->enEventType >= TAF_XSMS_APP_MSG_TYPE_BUTT)
    {
        AT_ERR_LOG1("AT_ProcXsmsMsg: The Msg Id is Wrong", pstMsg->enEventType);

        return;
    }

    AT_LOG1("AT_ProcXsmsMsg ClientId", pstMsg->stXsmsAtEvent.usClientId);
    AT_LOG1("AT_ProcXsmsMsg OpId",     pstMsg->stXsmsAtEvent.ucOpId);

    if (At_ClientIdToUserId(pstMsg->stXsmsAtEvent.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG("AT_ProcXsmsMsg At_ClientIdToUserId FAILURE");

        return;
    }

    /* �㲥��Ϣ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        At_XsmsIndProc(ucIndex, pstMsg->enEventType, &pstMsg->stXsmsAtEvent);

        AT_NORM_LOG("At_PIHMsgProc : AT_BROADCAST_INDEX.");

        return;
    }

    AT_LOG1("At_PbMsgProc ucIndex",ucIndex);

    /* AT����ظ����� */
    At_XsmsCnfProc(ucIndex, pstMsg->enEventType, &pstMsg->stXsmsAtEvent);

    return;
}
#endif


/* PC������AT��A���Ƶ�C��, At_sprintf���ظ�����,���ڴ˴������������� */
/*****************************************************************************
 Prototype      : At_ChangeSATCMDNo
 Description    : Sat��Ϣ������
 Input          :
 Output         :
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : ---

*****************************************************************************/
VOS_UINT32 At_ChangeSTKCmdNo(VOS_UINT32 ulCmdType, VOS_UINT8 *ucCmdNo )
{
    switch(ulCmdType)
    {
        case SI_STK_REFRESH:
            *ucCmdNo = SI_AT_CMD_REFRESH;
            break;
        case SI_STK_DISPLAYTET:
            *ucCmdNo = SI_AT_CMD_DISPLAY_TEXT;
            break;
        case SI_STK_GETINKEY:
            *ucCmdNo = SI_AT_CMD_GET_INKEY;
             break;
        case SI_STK_GETINPUT:
            *ucCmdNo = SI_AT_CMD_GET_INPUT;
            break;
        case SI_STK_PLAYTONE:
            *ucCmdNo = SI_AT_CMD_PLAY_TONE;
            break;
        case SI_STK_SELECTITEM:
            *ucCmdNo = SI_AT_CMD_SELECT_ITEM;
            break;
        case SI_STK_SETUPMENU:
            *ucCmdNo = SI_AT_CMD_SETUP_MENU;
            break;
        case SI_STK_SETUPIDLETEXT:
            *ucCmdNo = SI_AT_CMD_SETUP_IDLE_MODE_TEXT;
            break;
        case SI_STK_LAUNCHBROWSER:
            *ucCmdNo = SI_AT_CMD_LAUNCH_BROWSER;
            break;
        case SI_STK_SENDSS:
            *ucCmdNo = SI_AT_CMD_SEND_SS;
            break;
        case SI_STK_LANGUAGENOTIFICATION:
            *ucCmdNo = SI_AT_CMD_LANGUAGENOTIFICATION;
            break;
        case SI_STK_SETFRAMES:
            *ucCmdNo = SI_AT_CMD_SETFRAMES;
            break;
        case SI_STK_GETFRAMESSTATUS:
            *ucCmdNo = SI_AT_CMD_GETFRAMESSTATUS;
            break;
        default:
            return VOS_ERR;
    }

    return VOS_OK;
}



TAF_UINT32 At_HexText2AsciiStringSimple(TAF_UINT32 MaxLength,TAF_INT8 *headaddr,TAF_UINT8 *pucDst,TAF_UINT32 ulLen,TAF_UINT8 *pucStr)
{
    TAF_UINT16 usLen = 0;
    TAF_UINT16 usChkLen = 0;
    TAF_UINT8 *pWrite = pucDst;
    TAF_UINT8 *pRead = pucStr;
    TAF_UINT8  ucHigh = 0;
    TAF_UINT8  ucLow = 0;

    if(((TAF_UINT32)(pucDst - (TAF_UINT8 *)headaddr) + (2 * ulLen) + 3) >= MaxLength)
    {
        AT_ERR_LOG("At_HexText2AsciiString too long");
        return 0;
    }

    if(ulLen != 0)
    {
        usLen += 1;

        *pWrite++ = '\"';

        /* ɨ�������ִ� */
        while( usChkLen++ < ulLen )
        {
            ucHigh = 0x0F & (*pRead >> 4);
            ucLow = 0x0F & *pRead;
            usLen += 2;    /* ��¼���� */

            if(ucHigh <= 0x09)   /* 0-9 */
            {
                *pWrite++ = ucHigh + 0x30;
            }
            else if(ucHigh >= 0x0A)    /* A-F */
            {
                *pWrite++ = ucHigh + 0x37;
            }
            else
            {

            }

            if(ucLow <= 0x09)   /* 0-9 */
            {
                *pWrite++ = ucLow + 0x30;
            }
            else if(ucLow >= 0x0A)    /* A-F */
            {
                *pWrite++ = ucLow + 0x37;
            }
            else
            {

            }
            /* ��һ���ַ� */
            pRead++;
        }

        usLen ++;

        *pWrite++ = '\"';

        *pWrite++ = '\0';
    }

    return usLen;
}


TAF_VOID  At_StkCsinIndPrint(TAF_UINT8 ucIndex,SI_STK_EVENT_INFO_STRU *pEvent)
{
    TAF_UINT16 usLength = 0;

    /* ��ӡ����AT�������� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s",
                                       gaucAtCrLf,
                                       gaucAtCsin);

    /* ��ӡ���������������ͳ��Ⱥ����� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       (pEvent->STKCmdStru.SatCmd.SatDataLen*2));

    /* ����������ʱ������ */
    if (pEvent->STKCmdStru.SatCmd.SatDataLen != 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ", %d, ",
                                            pEvent->STKCmdStru.SatType);


        /* ��16������ת��ΪASCII������������������� */
        usLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                            pEvent->STKCmdStru.SatCmd.SatDataLen,
                                                            pEvent->STKCmdStru.SatCmd.SatCmdData);
    }

    /* ��ӡ�س����� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",
                                        gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
}

/*****************************************************************************
 Prototype      : At_STKCMDDataPrintSimple
 Description    : Sat��Ϣ������
 Input          :
 Output         :
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2009-07-04
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
TAF_VOID At_STKCMDDataPrintSimple(TAF_UINT8 ucIndex,SI_STK_EVENT_INFO_STRU *pEvent)
{
    TAF_UINT16 usLength = 0;

    if(pEvent->STKCBEvent == SI_STK_CMD_IND_EVENT)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",gaucAtCsin);
    }
    else
    {
        if(pEvent->STKCmdStru.SatType != SI_STK_SETUPMENU)
        {
            return;
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",gaucAtCsmn);
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%d, %d, ",(pEvent->STKCmdStru.SatCmd.SatDataLen*2), pEvent->STKCmdStru.SatType);

    usLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN,(TAF_INT8 *)pgucAtSndCodeAddr,(TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                    pEvent->STKCmdStru.SatCmd.SatDataLen, pEvent->STKCmdStru.SatCmd.SatCmdData);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%s",gaucAtCrLf);

    At_SendResultData(ucIndex,pgucAtSndCrLfAddr,usLength+2);

    return ;
}

/*****************************************************************************
 Prototype      : At_SatCallBackFunc
 Description    : Sat��Ϣ������
 Input          :
 Output         :
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2009-07-04
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
TAF_VOID At_STKCMDSWPrintSimple(TAF_UINT8 ucIndex,STK_CALLBACK_EVENT STKCBEvent,SI_STK_SW_INFO_STRU *pSw)
{
    TAF_UINT16 usLength = 0;
    VOS_UINT8  *pucSystemAppConfig = VOS_NULL_PTR;

    /* ��ȡ�ϲ�Խ�Ӧ������: MP/WEBUI/ANDROID */
    pucSystemAppConfig = AT_GetSystemAppConfigAddr();

    if (*pucSystemAppConfig != SYSTEM_APP_ANDROID)
    {
        return ;
    }

    if(STKCBEvent == SI_STK_TERMINAL_RSP_EVENT)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",gaucAtCstr);
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",gaucAtCsen);
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%d, %d%s",pSw->SW1, pSw->SW2,gaucAtCrLf);

    At_SendResultData(ucIndex,pgucAtSndCrLfAddr,usLength+2);

    return ;
}


VOS_VOID AT_SendSTKCMDTypeResultData(
    VOS_UINT8                           ucIndex,
    VOS_UINT16                          usLength
)
{
    errno_t                             lMemResult;
    if (gucAtVType == AT_V_ENTIRE_TYPE)
    {
        /* Codeǰ���\r\n */
        lMemResult = memcpy_s((TAF_CHAR *)pgucAtSndCrLfAddr, AT_CRLF_STR_LEN, (TAF_CHAR *)gaucAtCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CRLF_STR_LEN, AT_CRLF_STR_LEN);
        At_SendResultData(ucIndex, pgucAtSndCrLfAddr, usLength + AT_CRLF_STR_LEN);
    }
    else
    {
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    }

    return;
}


VOS_UINT32 At_STKCMDTypePrint(TAF_UINT8 ucIndex,TAF_UINT32 SatType, TAF_UINT32 EventType)
{
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;
    TAF_UINT16                          usLength = 0;
    TAF_UINT8                           ucCmdType = 0;
    TAF_UINT32                          ulResult = AT_SUCCESS;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    /* ��ʼ�� */
    enModemId       = MODEM_ID_0;

    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_STKCMDTypePrint: Get modem id fail.");
        return AT_FAILURE;
    }

    /* �Խ�AP����Ҫ��� */
    if (*pucSystemAppConfig != SYSTEM_APP_ANDROID)
    {
        if(EventType != SI_STK_CMD_END_EVENT)
        {
            ulResult = At_ChangeSTKCmdNo(SatType, &ucCmdType);
        }

        if(ulResult == AT_FAILURE)
        {
            return AT_FAILURE;
        }
    }

    if(g_ulSTKFunctionFlag == TAF_FALSE)
    {
        return AT_FAILURE;
    }

    switch (EventType)
    {
        case SI_STK_CMD_QUERY_RSP_EVENT:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s %d, 0%s",
                                               gaucAtStgi,
                                               ucCmdType,
                                               gaucAtCrLf);
            break;
        case SI_STK_CMD_IND_EVENT:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s %d, 0, 0%s",
                                               gaucAtStin,
                                               ucCmdType,
                                               gaucAtCrLf);
            break;
        case SI_STK_CMD_END_EVENT:
            if (*pucSystemAppConfig == SYSTEM_APP_ANDROID)
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s 0, 0%s",
                                                   gaucAtCsin,
                                                   gaucAtCrLf);
            }
            else
            {
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s 99, 0, 0%s",
                                                   gaucAtStin,
                                                   gaucAtCrLf);
            }
            break;
        default:
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                                "%s %d, 0, 1%s",
                                                gaucAtStin,
                                                ucCmdType,
                                                gaucAtCrLf);
            break;
    }

    AT_SendSTKCMDTypeResultData(ucIndex, usLength);

    return AT_SUCCESS;
}


TAF_VOID AT_STKCnfMsgProc(MN_APP_STK_AT_CNF_STRU *pstSTKCnfMsg)
{
    TAF_UINT8                           ucIndex;
    TAF_UINT32                          ulResult;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    ucIndex                             = 0;
    ulResult                            = AT_OK;
    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    AT_LOG1("AT_STKCnfMsgProc pEvent->ClientId",   pstSTKCnfMsg->stSTKAtCnf.ClientId);
    AT_LOG1("AT_STKCnfMsgProc EventType",          pstSTKCnfMsg->stSTKAtCnf.STKCBEvent);
    AT_LOG1("AT_STKCnfMsgProc SIM Event Error",    pstSTKCnfMsg->stSTKAtCnf.STKErrorNo);

    gstAtSendData.usBufLen = 0;

    if(At_ClientIdToUserId(pstSTKCnfMsg->stSTKAtCnf.ClientId,&ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG("AT_STKCnfMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    /* ����������������ֹͣ��ʱ�� */
    if((pstSTKCnfMsg->stSTKAtCnf.STKCBEvent != SI_STK_CMD_IND_EVENT)
        &&(pstSTKCnfMsg->stSTKAtCnf.STKCBEvent != SI_STK_CMD_TIMEOUT_IND_EVENT)
        &&(pstSTKCnfMsg->stSTKAtCnf.STKCBEvent != SI_STK_CMD_END_EVENT)
        &&(pstSTKCnfMsg->stSTKAtCnf.STKCBEvent != SI_STK_CC_RESULT_IND_EVENT)

        &&(pstSTKCnfMsg->stSTKAtCnf.STKCBEvent != SI_STK_SMSCTRL_RESULT_IND_EVENT))
    {
        if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
        {
            AT_WARN_LOG("AT_STKCnfMsgProc: AT_BROADCAST_INDEX.");
            return;
        }

        AT_STOP_TIMER_CMD_READY(ucIndex);

        AT_LOG1("AT_STKCnfMsgProc ucIndex",            ucIndex);
        AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);
    }

    if(pstSTKCnfMsg->stSTKAtCnf.STKErrorNo != AT_SUCCESS)
    {
        ulResult = At_ChgTafErrorCode(ucIndex,(TAF_UINT16)pstSTKCnfMsg->stSTKAtCnf.STKErrorNo);       /* �������� */

        At_FormatResultData(ucIndex,ulResult);
    }
    else
    {
        switch(pstSTKCnfMsg->stSTKAtCnf.STKCBEvent)
        {
            case SI_STK_CMD_IND_EVENT:
                if (*pucSystemAppConfig == SYSTEM_APP_ANDROID)
                {
                    At_StkCsinIndPrint(ucIndex,&(pstSTKCnfMsg->stSTKAtCnf));
                }
                else
                {
                    At_STKCMDTypePrint(ucIndex,pstSTKCnfMsg->stSTKAtCnf.STKCmdStru.SatType,pstSTKCnfMsg->stSTKAtCnf.STKCBEvent);
                }

                break;
            case SI_STK_CMD_END_EVENT:
            case SI_STK_CMD_TIMEOUT_IND_EVENT:
                At_STKCMDTypePrint(ucIndex,pstSTKCnfMsg->stSTKAtCnf.STKCmdStru.SatType,pstSTKCnfMsg->stSTKAtCnf.STKCBEvent);
                break;

            case SI_STK_CMD_QUERY_RSP_EVENT:
                At_STKCMDTypePrint(ucIndex,pstSTKCnfMsg->stSTKAtCnf.STKCmdStru.SatType,pstSTKCnfMsg->stSTKAtCnf.STKCBEvent);
                At_FormatResultData(ucIndex,ulResult);
                break;

            case SI_STK_GET_CMD_RSP_EVENT:
                if (*pucSystemAppConfig == SYSTEM_APP_ANDROID)
                {
                    At_StkCsinIndPrint(ucIndex, &(pstSTKCnfMsg->stSTKAtCnf));
                }
                else
                {
                    At_STKCMDDataPrintSimple(ucIndex, &(pstSTKCnfMsg->stSTKAtCnf));
                }

                At_FormatResultData(ucIndex,ulResult);
                break;

            case SI_STK_ENVELPOE_RSP_EVENT:
            case SI_STK_TERMINAL_RSP_EVENT:
                At_STKCMDSWPrintSimple(ucIndex,pstSTKCnfMsg->stSTKAtCnf.STKCBEvent,&pstSTKCnfMsg->stSTKAtCnf.STKSwStru);
                At_FormatResultData(ucIndex,ulResult);
                break;

            case SI_STK_CC_RESULT_IND_EVENT:
            case SI_STK_SMSCTRL_RESULT_IND_EVENT:
                At_StkCcinIndPrint(ucIndex, &(pstSTKCnfMsg->stSTKAtCnf));
                break;

            default:
                At_FormatResultData(ucIndex,ulResult);
                break;
        }
    }

    return;
}


TAF_VOID AT_STKPrintMsgProc(MN_APP_STK_AT_DATAPRINT_STRU *pstSTKPrintMsg)
{
    errno_t                             lMemResult;
    TAF_UINT8                           ucIndex = 0;

    if(At_ClientIdToUserId(pstSTKPrintMsg->stSTKAtPrint.ClientId,&ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG("AT_STKPrintMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_STKPrintMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    AT_LOG1("At_STKMsgProc pEvent->ClientId",   pstSTKPrintMsg->stSTKAtPrint.ClientId);
    AT_LOG1("At_STKMsgProc ucIndex",            ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    gucSTKCmdQualify = pstSTKPrintMsg->stSTKAtPrint.CmdQualify;

    lMemResult = memcpy_s(pgucAtSndCodeAddr, AT_CMD_MAX_LEN + 20 - 3, pstSTKPrintMsg->stSTKAtPrint.aucData, TAF_MIN(pstSTKPrintMsg->stSTKAtPrint.DataLen, STK_PRINT_MAX_LEN));
    TAF_MEM_CHK_RTN_VAL(lMemResult, AT_CMD_MAX_LEN + 20 - 3, pstSTKPrintMsg->stSTKAtPrint.DataLen);

    At_SendResultData(ucIndex,pgucAtSndCrLfAddr,(VOS_UINT16)pstSTKPrintMsg->stSTKAtPrint.DataLen+2);

    At_FormatResultData(ucIndex,AT_OK);

    return;
}


TAF_VOID At_STKMsgProc(VOS_VOID* pMsg)
{
    MN_APP_STK_AT_DATAPRINT_STRU    *pstSTKPrintMsg = VOS_NULL_PTR;
    MN_APP_STK_AT_CNF_STRU          *pstSTKCnfMsg = VOS_NULL_PTR;

    pstSTKCnfMsg    = (MN_APP_STK_AT_CNF_STRU*)pMsg;
    pstSTKPrintMsg  = (MN_APP_STK_AT_DATAPRINT_STRU*)pMsg;

    if(pstSTKCnfMsg->ulMsgId == STK_AT_DATAPRINT_CNF)
    {
        AT_STKPrintMsgProc(pstSTKPrintMsg);
    }
    else if(pstSTKCnfMsg->ulMsgId == STK_AT_EVENT_CNF)
    {
        AT_STKCnfMsgProc(pstSTKCnfMsg);
    }
    else
    {
        AT_ERR_LOG1("At_STKMsgProc:Msg ID Error",pstSTKPrintMsg->ulMsgId);
    }

    return;
}


VOS_UINT32 At_PrintEsimCleanProfileInfo(
    TAF_UINT8                           ucIndex,
    SI_EMAT_EVENT_INFO_STRU            *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_EsimSwitch_SET */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ESIMCLEAN_SET)
    {
        AT_WARN_LOG("At_PrintEsimCleanProfileInfo: CmdCurrentOpt is not AT_CMD_ESIMCLEAN_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_PrintEsimCheckProfileInfo(
    TAF_UINT8                           ucIndex,
    SI_EMAT_EVENT_INFO_STRU            *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_ESIMCHECK_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ESIMCHECK_QRY)
    {
        AT_WARN_LOG("At_PrintEsimCheckProfileInfo: CmdCurrentOpt is not AT_CMD_ESIMCHECK_QRY!");
        return AT_ERROR;
    }

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s^ESIMCHECK: ",
                                               gaucAtCrLf);

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%d",
                                           pstEvent->EMATEvent.stEsimCheckCnf.enHasProfile);
    return AT_OK;
}


VOS_UINT32 At_PrintGetEsimEidInfo(
    TAF_UINT8                           ucIndex,
    SI_EMAT_EVENT_INFO_STRU            *pstEvent,
    VOS_UINT16                         *pusLength
)
{
    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_ESIMEID_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ESIMEID_QRY)
    {
        AT_WARN_LOG("At_PrintGetEsimEidInfo : CmdCurrentOpt is not AT_CMD_ESIMEID_QRY!");
        return AT_ERROR;
    }

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%s^ESIMEID:\"",
                                           gaucAtCrLf);

    (*pusLength) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),
                                                        (TAF_UINT8 *)pstEvent->EMATEvent.stEidCnf.aucEsimEID,
                                                        SI_EMAT_ESIM_EID_MAX_LEN);

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "\"%s",
                                           gaucAtCrLf);

    return AT_OK;
}


VOS_UINT32 At_PrintGetEsimPKIDInfo(
    TAF_UINT8                           ucIndex,
    SI_EMAT_EVENT_INFO_STRU            *pstEvent,
    VOS_UINT16                         *pusLength
)
{
   VOS_UINT32                           ulPKIDNum;
   VOS_UINT32                           ulIndex   = 0;

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_ESIMPKID_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ESIMPKID_QRY)
    {
        AT_WARN_LOG("At_PrintGetEsimPKIDInfo : CmdCurrentOpt is not AT_CMD_ESIMPKID_QRY!");
        return AT_ERROR;
    }

    ulPKIDNum = (pstEvent->EMATEvent.stPKIDCnf.ulPKIDNum > SI_EMAT_ESIM_PKID_GROUP_MAX_NUM) ? \
                 SI_EMAT_ESIM_PKID_GROUP_MAX_NUM : pstEvent->EMATEvent.stPKIDCnf.ulPKIDNum;


    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "%s^PKID:%d",
                                           gaucAtCrLf,
                                           ulPKIDNum);

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                               ",\"");

    for (ulIndex = 0; ulIndex < ulPKIDNum; ulIndex++)
    {
        (*pusLength) += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            (TAF_UINT8 *)pgucAtSndCodeAddr + (*pusLength),
                                                            (TAF_UINT8 *)pstEvent->EMATEvent.stPKIDCnf.astPkid[ulIndex].aucEsimPKID,
                                                            SI_EMAT_ESIM_PKID_VALUE_LEN);

        if (ulIndex < ulPKIDNum - 1)
        {
            (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                                   "\",\"");
        }
    }

    (*pusLength) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + (*pusLength),
                                           "\"%s",
                                           gaucAtCrLf);

    return AT_OK;
}


TAF_VOID At_EMATIndProc(TAF_UINT8 ucIndex, SI_EMAT_EVENT_INFO_STRU *pEvent)
{
    return;
}


TAF_UINT32 At_EMATNotBroadIndProc(TAF_UINT8 ucIndex, SI_EMAT_EVENT_INFO_STRU *pEvent)
{
    return VOS_ERR;
}


TAF_VOID At_EMATRspProc(TAF_UINT8 ucIndex, SI_EMAT_EVENT_INFO_STRU *pEvent)
{
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;
    TAF_UINT32                          ulTmp;
    TAF_UINT32                          i;

    if(pEvent->ulEMATError != TAF_ERR_NO_ERROR)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);

        ulResult = At_ChgTafErrorCode(ucIndex, (TAF_UINT16)(pEvent->ulEMATError));

        gstAtSendData.usBufLen = usLength;

        At_FormatResultData(ucIndex,ulResult);

        return;
    }

    ulTmp = (sizeof(g_astAtEMATRspProcFuncTbl) / sizeof(g_astAtEMATRspProcFuncTbl[0]));

    for (i = 0; i < ulTmp; i++)
    {
        /* �ҵ������������������ش��� */
        if (pEvent->enEventType == g_astAtEMATRspProcFuncTbl[i].ulEventType)
        {
            ulResult = g_astAtEMATRspProcFuncTbl[i].pAtEMATRspProcFunc(ucIndex, pEvent, &usLength);

            if (ulResult == AT_ERROR)
            {
                AT_WARN_LOG("At_EMATRspProc : pAtEMATRspProcFunc is return error!");
                return;
            }

            break;
        }
    }

    /* û�ҵ���������ֱ�ӷ��� */
    if (i == ulTmp)
    {
        AT_WARN_LOG("At_EMATRspProc : no find AT Proc Func!");
        return;
    }

    ulResult = AT_OK;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);

    return;
}


TAF_VOID At_EMATMsgProc(VOS_VOID* pMsg)
{
    MN_APP_EMAT_AT_CNF_STRU            *pstMsg = VOS_NULL_PTR;
    TAF_UINT8                           ucIndex = 0;

    pstMsg = (MN_APP_EMAT_AT_CNF_STRU*)pMsg;

    if(pstMsg->ulMsgId != EMAT_AT_EVENT_CNF)
    {
        AT_ERR_LOG1("At_EMATMsgProc: The Msg Id is Wrong", pstMsg->ulMsgId);
        return;
    }

    AT_LOG1("At_EMATMsgProc pEvent->ClientId",   pstMsg->stEMATAtCnf.usClientId);
    AT_LOG1("At_EMATMsgProc EventType",          pstMsg->stEMATAtCnf.enEventType);
    AT_LOG1("At_EMATMsgProc SIM Event Error",    pstMsg->stEMATAtCnf.ulEMATError);

    if(At_ClientIdToUserId(pstMsg->stEMATAtCnf.usClientId,&ucIndex) == AT_FAILURE)
    {
        AT_ERR_LOG("At_EMATMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        At_EMATIndProc(ucIndex, &pstMsg->stEMATAtCnf);
        AT_WARN_LOG("At_EMATMsgProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_EMATMsgProc ucIndex",ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    /* �ǹ㲥�������ϱ� */
    if (At_EMATNotBroadIndProc(ucIndex, &pstMsg->stEMATAtCnf) == VOS_OK)
    {
        return;
    }

    At_EMATRspProc(ucIndex, &pstMsg->stEMATAtCnf);

    return;
}


TAF_VOID At_DataStatusIndProc(TAF_UINT16  ClientId,
                                  TAF_UINT8      ucDomain,
                                  TAF_UINT8      ucRabId,
                                  TAF_UINT8      ucStatus,
                                  TAF_UINT8      ucCause )
{
    AT_RRETURN_CODE_ENUM_UINT32         ulResult = AT_FAILURE;
    TAF_UINT8 ucIndex = 0;
    TAF_UINT16 usLength = 0;

    AT_LOG1("At_DataStatusIndProc ClientId",ClientId);
    AT_LOG1("At_DataStatusIndProc ucDomain",ucDomain);
    AT_LOG1("At_DataStatusIndProc ucRabId",ucRabId);
    AT_LOG1("At_DataStatusIndProc ucStatus",ucStatus);
    AT_LOG1("At_DataStatusIndProc ucRabId",ucCause);
    if(At_ClientIdToUserId(ClientId,&ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_DataStatusIndProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_DataStatusIndProc: AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_DataStatusIndProc ucIndex",ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    switch(ucStatus)
    {
    case TAF_RABM_STOP_DATA:
    case TAF_DATA_STOP:
        break;

      default:
        break;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);
}

TAF_VOID At_DataStatusMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    errno_t                             lMemResult;
    TAF_UINT16                          ClientId = 0;
    TAF_UINT16                          usAddr = 0;
    TAF_UINT8                           ucDomain = 0;
    TAF_UINT8                           ucRabId = 0;
    TAF_UINT8                           ucStatus = 0;
    TAF_UINT8                           ucCause = 0;

    lMemResult = memcpy_s(&ClientId, sizeof(ClientId), pData, sizeof(ClientId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ClientId), sizeof(ClientId));
    usAddr += sizeof(ClientId);

    lMemResult = memcpy_s(&ucDomain, sizeof(ucDomain), pData+usAddr, sizeof(ucDomain));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ucDomain), sizeof(ucDomain));
    usAddr += sizeof(ucDomain);

    lMemResult = memcpy_s(&ucRabId, sizeof(ucRabId), pData+usAddr, sizeof(ucRabId));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ucRabId), sizeof(ucRabId));
    usAddr += sizeof(ucRabId);

    lMemResult = memcpy_s(&ucStatus, sizeof(ucStatus), pData+usAddr, sizeof(ucStatus));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ucStatus), sizeof(ucStatus));
    usAddr += sizeof(ucStatus);

    lMemResult = memcpy_s(&ucCause, sizeof(ucCause), pData+usAddr, sizeof(ucCause));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(ucCause), sizeof(ucCause));

    At_DataStatusIndProc(ClientId,ucDomain,ucRabId,ucStatus,ucCause);
}


VOS_UINT32 AT_ConvertCallError(TAF_CS_CAUSE_ENUM_UINT32 enCause)
{
    AT_CME_CALL_ERR_CODE_MAP_STRU      *pstCallErrMapTblPtr = VOS_NULL_PTR;
    VOS_UINT32                          ulCallErrMapTblSize;
    VOS_UINT32                          ulCnt;

    pstCallErrMapTblPtr = AT_GET_CME_CALL_ERR_CODE_MAP_TBL_PTR();
    ulCallErrMapTblSize = AT_GET_CME_CALL_ERR_CODE_MAP_TBL_SIZE();

    for (ulCnt = 0; ulCnt < ulCallErrMapTblSize; ulCnt++)
    {
        if (pstCallErrMapTblPtr[ulCnt].enCsCause == enCause)
        {
            return pstCallErrMapTblPtr[ulCnt].ulCmeCode;
        }
    }

    return AT_CME_UNKNOWN;
}

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON)&&(FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON))

AT_ENCRYPT_VOICE_ERROR_ENUM_UINT32  AT_MapEncVoiceErr(
    TAF_CALL_ENCRYPT_VOICE_STATUS_ENUM_UINT32           enTafEncVoiceErr
)
{
    VOS_UINT32                          i;
    AT_ENCRYPT_VOICE_ERR_CODE_MAP_STRU *pstAtEncVoiceErrMapTbl = VOS_NULL_PTR;
    VOS_UINT32                          ulAtEncVoiceErrMapSize;

    pstAtEncVoiceErrMapTbl = AT_GET_ENC_VOICE_ERR_CODE_MAP_TBL_PTR();
    ulAtEncVoiceErrMapSize = AT_GET_ENC_VOICE_ERR_CODE_MAP_TBL_SIZE();

    for (i = 0; i < ulAtEncVoiceErrMapSize; i++)
    {
        if (pstAtEncVoiceErrMapTbl[i].enTafEncErr == enTafEncVoiceErr)
        {
            return pstAtEncVoiceErrMapTbl[i].enAtEncErr;
        }
    }
    return AT_ENCRYPT_VOICE_ERROR_ENUM_BUTT;
}
#endif


TAF_UINT32 At_CmdCmgdMsgProc(
    TAF_UINT8                                               ucIndex,
    TAF_UINT32                                              ulErrorCode
)
{
    VOS_UINT8                       *pucSystemAppConfig = VOS_NULL_PTR;
    TAF_UINT32                       ulResult           = AT_FAILURE;

    pucSystemAppConfig              = AT_GetSystemAppConfigAddr();

    if ((ulErrorCode == MN_ERR_CLASS_SMS_EMPTY_REC)
     && (*pucSystemAppConfig != SYSTEM_APP_ANDROID))
    {
        ulResult = AT_OK;
        AT_STOP_TIMER_CMD_READY(ucIndex);
    }
    else
    {
        ulResult = At_ChgMnErrCodeToAt(ucIndex,ulErrorCode);                     /* �������� */
        AT_STOP_TIMER_CMD_READY(ucIndex);
    }

    return ulResult;
}



TAF_VOID At_CmdCnfMsgProc(TAF_UINT8* pData,TAF_UINT16 usLen)
{
    AT_CMD_CNF_EVENT                    *pstCmdCnf = VOS_NULL_PTR;
    MN_CLIENT_ID_T                      clientId;
    TAF_UINT32                          ulErrorCode;
    TAF_UINT8                           ucIndex;
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;

    pstCmdCnf = (AT_CMD_CNF_EVENT *)pData;

    clientId    = pstCmdCnf->clientId;
    ulErrorCode = pstCmdCnf->ulErrorCode;

    if(At_ClientIdToUserId(clientId,&ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_CmdCnfMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_CmdCnfMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    if (g_stParseContext[ucIndex].ucClientStatus == AT_FW_CLIENT_STATUS_READY)
    {
        AT_WARN_LOG("At_CmdCnfMsgProc : AT command entity is released.");
        return;
    }

    /*
        callҵ���ϱ�����TAF_CS_CAUSE_SUCCESS��������ҵ���ϱ�����MN_ERR_NO_ERROR,
        ���ǵ�ʵ��ֵ��Ϊ0
    */
    if (ulErrorCode == MN_ERR_NO_ERROR)
    {
        /* ���ж��Ƿ��к�����C����ʵ�֣����޺��е�������ϱ�MN_ERR_NO_ERROR
           AT����ؽ����ҪΪAT_OK */
        if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_H_SET
         || gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CHUP_SET)
        {
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex,AT_OK);
        }

        AT_NORM_LOG("At_CmdCnfMsgProc Rsp No Err");
        return;
    }

    AT_LOG1("At_CmdCnfMsgProc ucIndex",ucIndex);
    AT_LOG1("gastAtClientTab[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

    switch(gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
    case AT_CMD_CDUR_READ:
    /* CCWA������� */
    case AT_CMD_CCWA_DISABLE:
    case AT_CMD_CCWA_ENABLE:
    case AT_CMD_CCWA_QUERY:

    /* CCFC���� */
    case AT_CMD_CCFC_DISABLE:
    case AT_CMD_CCFC_ENABLE:
    case AT_CMD_CCFC_QUERY:
    case AT_CMD_CCFC_REGISTRATION:
    case AT_CMD_CCFC_ERASURE:

    /* CUSD������� */
    case AT_CMD_CUSD_REQ:

    /* CLCK������� */
    case AT_CMD_CLCK_UNLOCK:
    case AT_CMD_CLCK_LOCK:
    case AT_CMD_CLCK_QUERY:

    /* CLOP���� */
    case AT_CMD_COLP_READ:

    /* CLIR���� */
    case AT_CMD_CLIR_READ:

    /* CLIP���� */
    case AT_CMD_CLIP_READ:
    /* CPWD���� */
    case AT_CMD_CPWD_SET:

    case AT_CMD_CNAP_QRY:
        ulResult = AT_ConvertCallError(ulErrorCode);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        break;
    case AT_CMD_CSCA_READ:
        ulResult = At_ChgMnErrCodeToAt(ucIndex, ulErrorCode);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        break;
    case AT_CMD_CPMS_SET:
    case AT_CMD_CPMS_READ:
        ulResult = AT_CMS_UNKNOWN_ERROR;
        AT_STOP_TIMER_CMD_READY(ucIndex);
        break;


    /*
        ���ulErrorCode��ΪTAF_CS_CAUSE_NO_CALL_ID����AT_CMD_D_CS_VOICE_CALL_SET
        ��AT_CMD_D_CS_DATA_CALL_SETҵ��ͳһ�ϱ�AT_NO_CARRIER����ֵ
    */

    case AT_CMD_D_CS_VOICE_CALL_SET:
    case AT_CMD_APDS_SET:
        if (ulErrorCode == TAF_CS_CAUSE_NO_CALL_ID)
        {
            ulResult = AT_ERROR;
            AT_STOP_TIMER_CMD_READY(ucIndex);
            break;
        }

    case AT_CMD_D_CS_DATA_CALL_SET:
         ulResult = AT_NO_CARRIER;
         AT_STOP_TIMER_CMD_READY(ucIndex);
         break;

    case AT_CMD_CHLD_SET:
    case AT_CMD_CHUP_SET:
    case AT_CMD_A_SET:
    case AT_CMD_CHLD_EX_SET:
    case AT_CMD_H_SET:
        ulResult = AT_ConvertCallError(ulErrorCode);                            /* �������� */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        break;
    case AT_CMD_CMGR_SET:
    case AT_CMD_CMGD_SET:
        ulResult = At_CmdCmgdMsgProc(ucIndex, ulErrorCode);
        break;
        /* fall through */
    case AT_CMD_CSMS_SET:
    case AT_CMD_CMMS_SET:
    case AT_CMD_CGSMS_SET:
    case AT_CMD_CGSMS_READ:
    case AT_CMD_CMMS_READ:
    case AT_CMD_CSMP_READ:    /*del*/
    case AT_CMD_CMGS_TEXT_SET:
    case AT_CMD_CMGS_PDU_SET:
    case AT_CMD_CMGC_TEXT_SET:
    case AT_CMD_CMGC_PDU_SET:
    case AT_CMD_CMSS_SET:
    case AT_CMD_CMST_SET:
    case AT_CMD_CNMA_TEXT_SET:
    case AT_CMD_CNMA_PDU_SET:
    case AT_CMD_CMGW_PDU_SET:
    case AT_CMD_CMGW_TEXT_SET:
    case AT_CMD_CMGL_SET:
    case AT_CMD_CMGD_TEST:
    case AT_CMD_CSMP_SET:
    case AT_CMD_CSCA_SET:
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    case AT_CMD_CSCB_SET:
    case AT_CMD_CSCB_READ:
#endif
        ulResult = At_ChgMnErrCodeToAt(ucIndex,ulErrorCode);                     /* �������� */
        AT_STOP_TIMER_CMD_READY(ucIndex);
        break;
    default:
        /*Ĭ��ֵ��֪���ǲ��Ǹ����������ʱ��д���*/
        ulResult = AT_CME_UNKNOWN;
        AT_STOP_TIMER_CMD_READY(ucIndex);
        break;
    }

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


TAF_UINT32 At_PrintTimeZoneInfo(
    NAS_MM_INFO_IND_STRU                *pstMmInfo,
    VOS_UINT8                           *pucDst
)
{
    VOS_INT8                            cTimeZone;
    VOS_UINT8                           ucTimeZoneValue;
    VOS_UINT16                          usLength;

    usLength  = 0;
    cTimeZone = AT_INVALID_TZ_VALUE;

    /* ��ȡ�����ϱ���ʱ����Ϣ */
    if ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
    {
        cTimeZone   = pstMmInfo->stUniversalTimeandLocalTimeZone.cTimeZone;
    }

    if ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ)
    {
        cTimeZone   = pstMmInfo->cLocalTimeZone;
    }

    if (cTimeZone < 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "-");

        ucTimeZoneValue = (VOS_UINT8)(cTimeZone * (-1));
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "+");

        ucTimeZoneValue = (VOS_UINT8)cTimeZone;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pucDst + usLength,
                                       "%02d",
                                       ucTimeZoneValue);

    /* ��β */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pucDst + usLength,
                                        "\"%s",
                                       gaucAtCrLf);
    return usLength;
}


VOS_UINT32 AT_PrintTimeZoneInfoNoAdjustment(
    TAF_AT_COMM_TIME_STRU              *pstMmInfo,
    VOS_UINT8                          *pucDst
)
{
    VOS_INT8                            cTimeZone;
    VOS_UINT8                           ucTimeZoneValue;
    VOS_UINT16                          usLength;

    usLength  = 0;
    cTimeZone = AT_INVALID_TZ_VALUE;

    /* ���ʱ�� */
    if ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
    {
        cTimeZone   = pstMmInfo->stUniversalTimeandLocalTimeZone.cTimeZone;
    }


    if (cTimeZone < 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "-");

        ucTimeZoneValue = (VOS_UINT8)(cTimeZone * (-1));
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "+");

        ucTimeZoneValue = (VOS_UINT8)cTimeZone;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pucDst + usLength,
                                       "%d",
                                       ucTimeZoneValue);


    /* ��ʾ��ʱ�ƻ�ʱ����Ϣ */
    if ( ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST)
      && (pstMmInfo->ucDST > 0))
    {
        /* ��ʱ��: DST�ֶδ���, ��ֵ����0��*/
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           ",%02d\"%s",
                                           pstMmInfo->ucDST,
                                           gaucAtCrLf);
    }
    else
    {
        /* ��ʱ�� */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           ",00\"%s",
                                           gaucAtCrLf);
    }

    return usLength;
}


VOS_UINT32 AT_PrintTimeZoneInfoWithCtzeType(
    TAF_AT_COMM_TIME_STRU               *pstMmInfo,
    VOS_UINT8                           *pucDst
)
{
    VOS_INT8                            cTimeZone;
    VOS_UINT8                           ucTimeZoneValue;
    VOS_UINT16                          usLength;

    usLength  = 0;
    cTimeZone = AT_INVALID_TZ_VALUE;

    /* ��ȡ�����ϱ���ʱ����Ϣ */
    if ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
    {
        cTimeZone   = pstMmInfo->stUniversalTimeandLocalTimeZone.cTimeZone;
    }

    if ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ)
    {
        cTimeZone   = pstMmInfo->cLocalTimeZone;
    }

    if (cTimeZone < 0)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "-");

        ucTimeZoneValue = (VOS_UINT8)(cTimeZone * (-1));
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "+");

        ucTimeZoneValue = (VOS_UINT8)cTimeZone;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pucDst + usLength,
                                       "%02d",
                                       ucTimeZoneValue);

    /* ��ʾ��ʱ�ƻ�ʱ����Ϣ */
    if (((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST)
      && (pstMmInfo->ucDST > 0))
    {
        /* ��ʱ��: DST�ֶδ���, ��ֵ����0��*/
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           ",%01d",
                                           pstMmInfo->ucDST);
    }
    else
    {
        /* ��ʱ�� */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           ",0");
    }

    /* ��ʾʱ����Ϣ */
    if ((pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
    {
        /* YY */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           ",%d%d/",
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.usYear / 10,
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.usYear % 10);
        /* MM */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "%d%d/",
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucMonth / 10,
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucMonth % 10);
        /* dd */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "%d%d,",
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucDay / 10,
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucDay % 10);

        /* hh */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "%d%d:",
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucHour / 10,
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucHour % 10);

        /* mm */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "%d%d:",
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucMinute / 10,
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucMinute % 10);

        /* ss */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                           "%d%d",
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucSecond / 10,
                                           pstMmInfo->stUniversalTimeandLocalTimeZone.ucSecond % 10);

    }

    /* ��β */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pucDst + usLength,
                                            "\"%s",
                                           gaucAtCrLf);
    return usLength;
}


VOS_UINT8 At_GetDaysForEachMonth(
    VOS_UINT8                               ucYear,
    VOS_UINT8                               ucMonth
)
{
    VOS_UINT16   usAdjustYear;

    /* �����yearֵΪ��λ����Ĭ�ϴ�2000�꿪ʼ���� */
    usAdjustYear = 2000 + ucYear;

    if ((ucMonth == 1) || (ucMonth == 3) || (ucMonth == 5) || (ucMonth == 7)
     || (ucMonth == 8) || (ucMonth == 10) || (ucMonth == 12) )
    {
        /* 1,3,5,7,8,10,12����31�� */
        return 31;
    }
    else if ((ucMonth == 4) || (ucMonth == 6) || (ucMonth == 9) || (ucMonth == 11))
    {
        /* 4,6,9,11����30�� */
        return 30;
    }
    else
    {
        /* 2�¿��Ƿ�Ϊ���꣬����Ϊ29�죬����Ϊ28�� */
        if ( (((usAdjustYear % 4) == 0) && ((usAdjustYear % 100) != 0)) || ((usAdjustYear % 400) == 0))
        {
            /* ���� */
            return 29;
        }
        else
        {
            /* ������ */
            return 28;
        }
    }
}


VOS_VOID At_AdjustLocalDate(
    TIME_ZONE_TIME_STRU                 *pstUinversalTime,
    VOS_INT8                            cAdjustValue,
    TIME_ZONE_TIME_STRU                 *pstLocalTime
)
{
    VOS_UINT8    ucDay;

    /* �������� */
    ucDay = (VOS_UINT8)(pstUinversalTime->ucDay + cAdjustValue);

    if (ucDay == 0)
    {
        /* �·ݼ�һ */
        if ( pstUinversalTime->ucMonth == 1 )
        {
            /* ����Ϊ��һ���12�·�,��ݼ�һ */
            pstLocalTime->ucMonth = 12;

            if (pstUinversalTime->ucYear == 0)
            {
                /* �����2000�꣬����Ϊ1999�� */
                pstLocalTime->ucYear = 99;
            }
            else
            {
                pstLocalTime->ucYear = pstUinversalTime->ucYear - 1;
            }
        }
        else
        {
            pstLocalTime->ucMonth = pstUinversalTime->ucMonth - 1;
            pstLocalTime->ucYear  = pstUinversalTime->ucYear;
        }

        /* ���ڵ���Ϊ�ϸ��µ����һ��, */
        pstLocalTime->ucDay = At_GetDaysForEachMonth(pstLocalTime->ucYear, pstLocalTime->ucMonth);
    }
    else if (ucDay > At_GetDaysForEachMonth(pstUinversalTime->ucYear, pstUinversalTime->ucMonth))
    {
        /*���ڵ���Ϊ�¸���һ�� */
        pstLocalTime->ucDay = 1;

        /* �·ݼ�һ */
        if ( pstUinversalTime->ucMonth == 12 )
        {
            /* ����Ϊ��һ���1�·�,��ݼ�һ */
            pstLocalTime->ucMonth = 1;
            pstLocalTime->ucYear = pstUinversalTime->ucYear + 1;
        }
        else
        {
            pstLocalTime->ucMonth = pstUinversalTime->ucMonth + 1;
            pstLocalTime->ucYear = pstUinversalTime->ucYear;
        }
    }
    else
    {
        pstLocalTime->ucDay   = ucDay;
        pstLocalTime->ucMonth = pstUinversalTime->ucMonth;
        pstLocalTime->ucYear  = pstUinversalTime->ucYear;
    }
}


VOS_VOID At_UniversalTime2LocalTime(
    TIME_ZONE_TIME_STRU                 *pstUinversalTime,
    TIME_ZONE_TIME_STRU                 *pstLocalTime
)
{
    VOS_INT8    cTemp;
    VOS_INT8    cAdjustValue;

    pstLocalTime->cTimeZone = pstUinversalTime->cTimeZone;

    /* ����ʱ����Ϣ����ͨ��ʱ��ת��Ϊ����ʱ�䡣ʱ����Ϣ����15����Ϊ��λ */

    /* ��������� */
    pstLocalTime->ucSecond  = pstUinversalTime->ucSecond;

    /* ���������� */
    cTemp = (VOS_INT8)(((pstUinversalTime->cTimeZone % 4) * 15) + pstUinversalTime->ucMinute);
    if (cTemp >= 60)
    {
        /*ʱ�������󣬷���������60���ӣ�Сʱ���� 1 */
        pstLocalTime->ucMinute  = (VOS_UINT8)(cTemp - 60);
        cAdjustValue = 1;
    }
    else if (cTemp < 0)
    {
        /*ʱ�������󣬷�����С��0���ӣ�Сʱ���� 1 */
        pstLocalTime->ucMinute  = (VOS_UINT8)(cTemp + 60);
        cAdjustValue = -1;
    }
    else
    {
        pstLocalTime->ucMinute = (VOS_UINT8)cTemp;
        cAdjustValue = 0;
    }

    /* Сʱ������ */
    cTemp = (VOS_INT8)((pstUinversalTime->cTimeZone / 4) + pstUinversalTime->ucHour + cAdjustValue);

    if (cTemp >= 24)
    {
        /*ʱ��������ʱ�䳬��24Сʱ�����ڼ� 1 */
        pstLocalTime->ucHour = (VOS_UINT8)(cTemp - 24);
        cAdjustValue = 1;
    }
    else if (cTemp < 0)
    {
        /*ʱ��������ʱ��С��0�����ڼ� 1 */
        pstLocalTime->ucHour = (VOS_UINT8)(cTemp + 24);
        cAdjustValue = -1;
    }
    else
    {
        pstLocalTime->ucHour = (VOS_UINT8)cTemp;
        cAdjustValue = 0;
    }

    /* ������������ */
    At_AdjustLocalDate(pstUinversalTime, cAdjustValue, pstLocalTime);

    return;
}


TAF_UINT32 At_PrintMmTimeInfo(
    VOS_UINT8                           ucIndex,
    TAF_AT_COMM_TIME_STRU              *pstMmTimeInfo,
    TAF_UINT8                          *pDst
)
{
    TAF_UINT16                          usLength;
    TAF_INT8                            cTimeZone;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulChkTimeFlg;
    VOS_UINT32                          ulChkCtzvFlg;
    VOS_UINT32                          ulChkCtzeFlg;

    usLength = 0;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_PrintMmTimeInfo: Get modem id fail.");
        return usLength;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    if ((pstMmTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
    {
        /* ���������·���ʱ����Ϣ���޸��ֶΣ���ʹ��ԭ��ֵ */
        pstNetCtx->stTimeInfo.ucIeFlg |= NAS_MM_INFO_IE_UTLTZ;

        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucYear   = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.usYear % 100;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth  = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.ucMonth;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucDay    = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.ucDay;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucHour   = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.ucHour;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.ucMinute;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.ucSecond;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.Reserved = 0x00;
        pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.cTimeZone;
        pstNetCtx->ulNwSecond                                           = AT_GetSeconds();
    }

    /* ����DST��Ϣ */
    if ((pstMmTimeInfo->ucIeFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST)
    {
        /* ���������·���ʱ����Ϣ */
        pstNetCtx->stTimeInfo.ucIeFlg |= NAS_MM_INFO_IE_DST;
        pstNetCtx->stTimeInfo.ucDST = pstMmTimeInfo->ucDST;
    }
    else
    {
        pstNetCtx->stTimeInfo.ucIeFlg &= ~NAS_MM_INFO_IE_DST;
    }

    ulChkCtzvFlg    = AT_CheckRptCmdStatus(pstMmTimeInfo->aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CTZV);
    ulChkTimeFlg    = AT_CheckRptCmdStatus(pstMmTimeInfo->aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_TIME);
    ulChkCtzeFlg    = AT_CheckRptCmdStatus(pstMmTimeInfo->aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CTZE);

    /*ʱ����ʾ��ʽ: +CTZV: "GMT��tz, Summer(Winter) Time" */
    if ((AT_CheckRptCmdStatus(pstMmTimeInfo->aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CTZV) == VOS_TRUE)
     && (ulChkCtzvFlg == VOS_TRUE))

    {
        if ((pstMmTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
        {
            cTimeZone = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.cTimeZone;
        }
        else
        {
            cTimeZone = pstMmTimeInfo->cLocalTimeZone;
        }

        if (cTimeZone != pstNetCtx->stTimeInfo.cLocalTimeZone)
        {
            /* ���������·���ʱ����Ϣ */
            pstNetCtx->stTimeInfo.ucIeFlg |= NAS_MM_INFO_IE_LTZ;
            pstNetCtx->stTimeInfo.cLocalTimeZone = cTimeZone;
            pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = cTimeZone;

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pDst + usLength,
                                               "%s%s\"",gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CTZV].pucText);

            usLength += (VOS_UINT16)At_PrintTimeZoneInfo(&(pstNetCtx->stTimeInfo),
                                                         pDst + usLength);
        }

    }

    /* ʱ����ʾ��ʽ:+CTZE: "(+/-)tz,dst,yyyy/mm/dd,hh:mm:ss" */
    if ((AT_CheckRptCmdStatus(pstMmTimeInfo->aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CTZE) == VOS_TRUE)
     && (ulChkCtzeFlg == VOS_TRUE))
    {

        if ((pstMmTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
        {
            cTimeZone = pstMmTimeInfo->stUniversalTimeandLocalTimeZone.cTimeZone;
        }
        else
        {
            cTimeZone = pstMmTimeInfo->cLocalTimeZone;
        }

        if (cTimeZone != pstNetCtx->stTimeInfo.cLocalTimeZone)
        {
            /* ���������·���ʱ����Ϣ */
            pstNetCtx->stTimeInfo.ucIeFlg |= NAS_MM_INFO_IE_LTZ;
            pstNetCtx->stTimeInfo.cLocalTimeZone = cTimeZone;
            pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = cTimeZone;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pDst + usLength,
                                       "%s%s\"",gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_CTZE].pucText);

        usLength += (VOS_UINT16)AT_PrintTimeZoneInfoWithCtzeType(pstMmTimeInfo,
                                                 pDst + usLength);



    }
    /*ʱ����ʾ��ʽ: ^TIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    if ((AT_CheckRptCmdStatus(pstMmTimeInfo->aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_TIME) == VOS_TRUE)
     && (ulChkTimeFlg == VOS_TRUE)
     && ((pstMmTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ))
    {
        /* "^TIME: */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "%s%s",gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_TIME].pucText);

        /* YY */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "\"%d%d/",
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucYear / 10,
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucYear % 10);
        /* MM */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "%d%d/",
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth / 10,
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth % 10);
        /* dd */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "%d%d,",
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucDay / 10,
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucDay % 10);

        /* hh */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "%d%d:",
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucHour / 10,
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucHour % 10);

        /* mm */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "%d%d:",
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute / 10,
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute % 10);

        /* ss */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pDst + usLength,
                                           "%d%d",
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond / 10,
                                           pstNetCtx->stTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond % 10);

        /* GMT��tz, Summer(Winter) Time" */
        usLength += (VOS_UINT16)AT_PrintTimeZoneInfoNoAdjustment(pstMmTimeInfo,
                                                                 pDst + usLength);
    }

    return usLength;
}

/* begin V7R1 PhaseI Modify */

VOS_UINT32  AT_GetSysModeName(
    MN_PH_SYS_MODE_EX_ENUM_U8           enSysMode,
    VOS_CHAR                           *pucSysModeName,
    VOS_UINT32                          ulMaxMemLength
)
{
    VOS_UINT32                          i;

    for ( i = 0 ; i < sizeof(g_astSysModeTbl)/sizeof(AT_PH_SYS_MODE_TBL_STRU) ; i++ )
    {
        if ( g_astSysModeTbl[i].enSysMode == enSysMode)
        {
            VOS_StrNCpy_s(pucSysModeName, ulMaxMemLength,
                        g_astSysModeTbl[i].pcStrSysModeName,
                        VOS_StrLen(g_astSysModeTbl[i].pcStrSysModeName));

            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_UINT32  AT_GetSubSysModeName(
    MN_PH_SUB_SYS_MODE_EX_ENUM_U8       enSubSysMode,
    VOS_CHAR                           *pucSubSysModeName,
    VOS_UINT32                          ulMaxMemLength
)
{
    VOS_UINT32                          i;

    for ( i = 0 ; i < sizeof(g_astSubSysModeTbl)/sizeof(AT_PH_SUB_SYS_MODE_TBL_STRU) ; i++ )
    {
        if ( g_astSubSysModeTbl[i].enSubSysMode == enSubSysMode)
        {
            VOS_StrNCpy_s(pucSubSysModeName, ulMaxMemLength,
                        g_astSubSysModeTbl[i].pcStrSubSysModeName,
                        VOS_StrLen(g_astSubSysModeTbl[i].pcStrSubSysModeName));

            return VOS_OK;
        }
    }

    return VOS_ERR;
}


VOS_VOID  AT_QryParaRspSysinfoExProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;
    VOS_CHAR                            aucSysModeName[255];
    VOS_CHAR                            aucSubSysModeName[255];
    TAF_PH_SYSINFO_STRU                 stSysInfo;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

    memset_s(&stSysInfo, sizeof(stSysInfo), 0x00, sizeof(TAF_PH_SYSINFO_STRU));

    lMemResult = memcpy_s(&stSysInfo, sizeof(stSysInfo), pPara, sizeof(TAF_PH_SYSINFO_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stSysInfo), sizeof(TAF_PH_SYSINFO_STRU));

    usLength  = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s:%d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,stSysInfo.ucSrvStatus);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSrvDomain);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucRoamStatus);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSimStatus);

    if ( *pucSystemAppConfig == SYSTEM_APP_WEBUI)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSimLockStatus);
    }
    else
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",");
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSysMode);

    memset_s(aucSysModeName, sizeof(aucSysModeName), 0x00, sizeof(aucSysModeName));
    memset_s(aucSubSysModeName, sizeof(aucSubSysModeName), 0x00, sizeof(aucSubSysModeName));

    /* ��ȡSysMode������ */
    AT_GetSysModeName(stSysInfo.ucSysMode, aucSysModeName, (TAF_UINT32)sizeof(aucSysModeName));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%s\"",aucSysModeName);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stSysInfo.ucSysSubMode);

    /* ��ȡSubSysMode������ */
    AT_GetSubSysModeName(stSysInfo.ucSysSubMode, aucSubSysModeName, (TAF_UINT32)sizeof(aucSubSysModeName));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%s\"",aucSubSysModeName);
    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);
}
/* end V7R1 PhaseI Modify */


VOS_VOID  AT_QryParaAnQueryProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    MN_MMA_ANQUERY_PARA_STRU            stAnqueryPara;
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     enCurAntennaLevel;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
#if (FEATURE_LTE == FEATURE_ON)
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;
    VOS_INT16                           sRsrp;
    VOS_INT16                           sRsrq;
    VOS_UINT8                           ucLevel;
    VOS_INT16                           sRssi;
#endif

    /* ��ʼ�� */
    ulResult   = AT_OK;
    memset_s(&stAnqueryPara, sizeof(stAnqueryPara), 0x00, sizeof(MN_MMA_ANQUERY_PARA_STRU));

    lMemResult = memcpy_s(&stAnqueryPara, sizeof(stAnqueryPara), pPara, sizeof(MN_MMA_ANQUERY_PARA_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stAnqueryPara), sizeof(MN_MMA_ANQUERY_PARA_STRU));

    if((stAnqueryPara.enServiceSysMode == TAF_MMA_RAT_GSM)
    || (stAnqueryPara.enServiceSysMode == TAF_MMA_RAT_WCDMA))
    {
        pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);


        /* �ϱ�����ת��:�� Rscp��Ecio��ʾΪ�Ǹ�ֵ����Rscp��EcioΪ-145��-32������rssiΪ99��
           ��ת��Ϊ0 */
        if ( ((stAnqueryPara.u.st2G3GCellSignInfo.sCpichRscp == 0) && (stAnqueryPara.u.st2G3GCellSignInfo.sCpichEcNo == 0))
          || (stAnqueryPara.u.st2G3GCellSignInfo.ucRssi == 99) )
        {
            /* ��������0, ��ӦӦ�õ�Ȧ�� */
            enCurAntennaLevel       = AT_CMD_ANTENNA_LEVEL_0;
        }
        else
        {
            /* ���ú���AT_CalculateAntennaLevel������D25�㷨������źŸ��� */
            enCurAntennaLevel = AT_CalculateAntennaLevel(stAnqueryPara.u.st2G3GCellSignInfo.sCpichRscp,
                                                         stAnqueryPara.u.st2G3GCellSignInfo.sCpichEcNo);
        }

        /* �źŴ��ʹ��� */
        AT_GetSmoothAntennaLevel(ucIndex, enCurAntennaLevel );

        stAnqueryPara.u.st2G3GCellSignInfo.sCpichRscp     = -stAnqueryPara.u.st2G3GCellSignInfo.sCpichRscp;
        stAnqueryPara.u.st2G3GCellSignInfo.sCpichEcNo     = -stAnqueryPara.u.st2G3GCellSignInfo.sCpichEcNo;

#if (FEATURE_LTE == FEATURE_ON)
        pucSystemAppConfig                  = AT_GetSystemAppConfigAddr();

        if ( *pucSystemAppConfig == SYSTEM_APP_WEBUI)
        {
            gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s:%d,%d,%d,%d,0,0",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.sCpichRscp,
                                                        (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.sCpichEcNo,
                                                        (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.ucRssi,
                                                        (VOS_INT32)pstNetCtx->enCalculateAntennaLevel);


            /* �ظ��û������� */
            At_FormatResultData(ucIndex,ulResult);

            return;
        }
#endif
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:%d,%d,%d,%d,0x%X",
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.sCpichRscp,
                                                       (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.sCpichEcNo,
                                                       (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.ucRssi,
                                                       (VOS_INT32)pstNetCtx->enCalculateAntennaLevel,
                                                       (VOS_INT32)stAnqueryPara.u.st2G3GCellSignInfo.ulCellId);

        /* �ظ��û������� */
        At_FormatResultData(ucIndex,ulResult);

        return;
    }
    else if(stAnqueryPara.enServiceSysMode == TAF_MMA_RAT_LTE)
    {
 #if (FEATURE_LTE == FEATURE_ON)
            sRsrp   = stAnqueryPara.u.st4GCellSignInfo.sRsrp;
            sRsrq   = stAnqueryPara.u.st4GCellSignInfo.sRsrq;
            sRssi   = stAnqueryPara.u.st4GCellSignInfo.sRssi;
            ucLevel = 0;

            AT_CalculateLTESignalValue(&sRssi,&ucLevel,&sRsrp,&sRsrq);

            gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s:0,99,%d,%d,%d,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        (VOS_INT32)sRssi,
                                                        (VOS_INT32)ucLevel,
                                                        (VOS_INT32)sRsrp,
                                                        (VOS_INT32)sRsrq);


            /* �ظ��û������� */
            At_FormatResultData(ucIndex,ulResult);

            return;
#endif
    }
    else
    {
        AT_WARN_LOG("AT_QryParaAnQueryProc:WARNING: THE RAT IS INVALID!");
        return;
    }

}



VOS_VOID  AT_QryParaHomePlmnProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           OpId,
    const VOS_VOID                     *pPara
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    TAF_MMA_HPLMN_WITH_MNC_LEN_STRU     stHplmn;

    /* ��ʼ�� */
    ulResult   = AT_OK;
    usLength   = 0;

    memset_s(&stHplmn, sizeof(stHplmn), 0x00, sizeof(TAF_MMA_HPLMN_WITH_MNC_LEN_STRU));

    lMemResult = memcpy_s(&stHplmn, sizeof(stHplmn), pPara, sizeof(TAF_MMA_HPLMN_WITH_MNC_LEN_STRU));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stHplmn), sizeof(TAF_MMA_HPLMN_WITH_MNC_LEN_STRU));

    /* �ϱ�MCC��MNC */
    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s:",
                                       (VOS_INT8*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%X%X%X",
                                       (VOS_INT32)(0x0f & stHplmn.stHplmn.Mcc) ,
                                       (VOS_INT32)(0x0f00 & stHplmn.stHplmn.Mcc) >> 8,
                                       (VOS_INT32)(0x0f0000 & stHplmn.stHplmn.Mcc) >> 16);

    if (stHplmn.ucHplmnMncLen == 2)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%X%X",
                                           (VOS_INT32)(0x0f & stHplmn.stHplmn.Mnc) ,
                                           (VOS_INT32)(0x0f00 & stHplmn.stHplmn.Mnc) >> 8);
    }
    else if (stHplmn.ucHplmnMncLen == 3)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%X%X%X",
                                           (VOS_INT32)(0x0f & stHplmn.stHplmn.Mnc) ,
                                           (VOS_INT32)(0x0f00 & stHplmn.stHplmn.Mnc) >> 8,
                                           (VOS_INT32)(0x0f0000 & stHplmn.stHplmn.Mnc) >> 16);
    }
    else
    {
        AT_WARN_LOG("AT_QryParaHomePlmnProc HPLMN MNC LEN INVAILID");
    }

    gstAtSendData.usBufLen = usLength;

    /* �ظ��û������� */
    At_FormatResultData(ucIndex,ulResult);

    return;
}





VOS_VOID AT_PrcoPsEvtErrCode(
    VOS_UINT8                           ucIndex,
    TAF_PS_CAUSE_ENUM_UINT32            enCuase
)
{
    VOS_UINT32                          ulResult;

    /* ת���������ʽ */
    if ( enCuase != TAF_PS_CAUSE_SUCCESS )
    {
        ulResult    = AT_ERROR;
    }
    else
    {
        ulResult    = AT_OK;
    }

    /* ���AT������, ��ֹͣ��ʱ�� */
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulResult);
}


VOS_VOID AT_LogPrintMsgProc(TAF_MNTN_LOG_PRINT_STRU *pstMsg)
{
    AT_PR_LOGI("[MDOEM:%d]%s", pstMsg->enModemId, pstMsg->acLog);
    return;
}



VOS_UINT32 AT_IsBroadcastPsEvt(
    TAF_PS_EVT_ID_ENUM_UINT32           enEvtId
)
{
    VOS_UINT32                          i;

    for ( i = 0; i < AT_ARRAY_SIZE(g_astAtBroadcastPsEvtTbl); i++ )
    {
        if (enEvtId == g_astAtBroadcastPsEvtTbl[i])
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_VOID AT_ConvertPdpContextIpAddrParaToString(
    TAF_PDP_DYNAMIC_PRIM_EXT_STRU      *pstCgdcont,
    VOS_UINT16                         *usLength
)
{
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    memset_s(acIpv4StrTmp,  sizeof(acIpv4StrTmp), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, AT_IPV6_ADDR_MASK_FORMAT_STR_LEN);

    if((pstCgdcont->bitOpIpAddr == VOS_TRUE) && (pstCgdcont->bitOpSubMask == VOS_TRUE))
    {
        if (pstCgdcont->stPdpAddr.enPdpType == TAF_PDP_IPV4)
        {
            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pstCgdcont->stPdpAddr.aucIpv4Addr);
            *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *usLength,",\"%s",acIpv4StrTmp);

            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pstCgdcont->stSubnetMask.aucIpv4Addr);
            *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *usLength,".%s\"",acIpv4StrTmp);
        }
        else if(pstCgdcont->stPdpAddr.enPdpType == TAF_PDP_IPV6)
        {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp,
                                                   pstCgdcont->stPdpAddr.aucIpv6Addr,
                                                   pstCgdcont->stSubnetMask.aucIpv6Addr);
            *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *usLength, ",\"%s\"", acIpv6StrTmp);
        }
        else
        {
            *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *usLength,",");
        }
    }
    else
    {
        *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *usLength,",");
    }

    return;
}


VOS_VOID AT_ConvertPdpContextAddrParaToString(
    VOS_UINT32                          bitOpAddr,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    VOS_UINT8                          *pucNumber,
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT16                         *pusStringLength
)
{
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    memset_s(acIpv4StrTmp,  sizeof(acIpv4StrTmp), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, AT_IPV6_ADDR_MASK_FORMAT_STR_LEN);

    if(bitOpAddr == VOS_TRUE)
    {
        if ( (enPdpType == TAF_PDP_IPV4) || (enPdpType == TAF_PDP_PPP) )
        {
            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pucNumber);
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,",\"%s\"",acIpv4StrTmp);
        }
        else if(enPdpType == TAF_PDP_IPV6)
        {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp,
                                                   aucIpv6Addr,
                                                   VOS_NULL_PTR);
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength, ",\"%s\"", acIpv6StrTmp);
        }
        else
        {
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,",");
        }
    }
    else
    {
        *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,",");
    }

    return;
}


VOS_VOID AT_ConvertULParaToString(
    VOS_UINT32                          ulOpPara,
    VOS_UINT32                          ulParaVal,
    VOS_UINT16                         *pusLength
)
{
    if (ulOpPara == VOS_TRUE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,",%d",ulParaVal);
    }
    else
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,",");
    }

    return;
}



VOS_VOID AT_ConvertULParaToXString(
    VOS_UINT32                          ulOpPara,
    VOS_UINT32                          ulParaVal,
    VOS_UINT16                         *pusLength
)
{
    if (ulOpPara == VOS_TRUE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,",%X",ulParaVal);
    }
    else
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,",");
    }

    return;
}


VOS_VOID AT_ConvertRangeParaToString(
    VOS_UINT32                          ulOpPara1,
    VOS_UINT32                          ulOpPara2,
    VOS_UINT32                          ulParaVal1,
    VOS_UINT32                          ulParaVal2,
    VOS_UINT16                         *pusLength
)
{
    if (ulOpPara1 == VOS_TRUE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",%d", ulParaVal1);
    }
    else if (ulOpPara2 == VOS_TRUE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",\"%d", ulParaVal1);
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ".%d\"", ulParaVal2);
    }
    else
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",");
    }

    return;
}


VOS_VOID AT_ConvertIpAddrAndMaskParaToString(
    TAF_TFT_QUREY_INFO_STRU            *pstCgtft,
    VOS_UINT8                           ucTmp2,
    VOS_UINT16                         *pusLength
)
{
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, sizeof(acIpv4StrTmp));
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, sizeof(acIpv6StrTmp));

    if (pstCgtft->astPfInfo[ucTmp2].bitOpRmtIpv4AddrAndMask == VOS_TRUE)
    {
        AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pstCgtft->astPfInfo[ucTmp2].aucRmtIpv4Address);
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",\"%s", acIpv4StrTmp);
        AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pstCgtft->astPfInfo[ucTmp2].aucRmtIpv4Mask);
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ".%s\"", acIpv4StrTmp);
    }
    else if (pstCgtft->astPfInfo[ucTmp2].bitOpRmtIpv6AddrAndMask == VOS_TRUE)
    {
        (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp,
                                               pstCgtft->astPfInfo[ucTmp2].aucRmtIpv6Address,
                                               pstCgtft->astPfInfo[ucTmp2].aucRmtIpv6Mask);
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",\"%s\"", acIpv6StrTmp);
    }
    else
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",");
    }

    return;
}


VOS_VOID AT_ConvertTftSrcAddrParaToString(
    TAF_TFT_EXT_STRU                   *pstTftInfo,
    VOS_UINT16                         *pusStringLength
)
{
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    memset_s(acIpv4StrTmp,  sizeof(acIpv4StrTmp), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, AT_IPV6_ADDR_MASK_FORMAT_STR_LEN);

    if(pstTftInfo->bitOpSrcIp == VOS_TRUE)
    {
        if ( (pstTftInfo->stSourceIpaddr.enPdpType == TAF_PDP_IPV4)
            || (pstTftInfo->stSourceIpaddr.enPdpType == TAF_PDP_PPP) )
        {
            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pstTftInfo->stSourceIpaddr.aucIpv4Addr);
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,",\"%s",acIpv4StrTmp);

            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,".");

            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), pstTftInfo->stSourceIpMask.aucIpv4Addr);
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,"%s\"",acIpv4StrTmp);
        }
        else if(pstTftInfo->stSourceIpaddr.enPdpType == TAF_PDP_IPV6)
        {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp,
                                                   pstTftInfo->stSourceIpaddr.aucIpv6Addr,
                                                   pstTftInfo->stSourceIpMask.aucIpv6Addr);
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength, ",\"%s\"", acIpv6StrTmp);
        }
        else
        {
            *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,",");
        }
    }
    else
    {
        *pusStringLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + *pusStringLength,",");
    }

    return;
}


#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_VOID AT_ConvertSNssaiToString(
    PS_S_NSSAI_STRU                    *pstSNssai,
    VOS_UINT16                         *pusLength
)
{
    /*  27007 rel15, 10.1.1�½�
     *  sst                                     only slice/service type (SST) is present
     *  sst;mapped_sst                          SST and mapped configured SST are present
     *  sst.sd                                  SST and slice differentiator (SD) are present
     *  sst.sd;mapped_sst                       SST, SD and mapped configured SST are present
     *  sst.sd;mapped_sst.mapped_sd             SST, SD, mapped configured SST and mapped configured SD are present
    */

    if ((pstSNssai->bitOpSd == VOS_TRUE)
     && (pstSNssai->bitOpMappedSst == VOS_TRUE)
     && (pstSNssai->bitOpMappedSd == VOS_TRUE))
    {
        /* sst.sd;mapped_sst.mapped_sd             SST, SD, mapped configured SST and mapped configured SD are present */
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,
                                             ",\"%x.%x;%x.%x\"", pstSNssai->ucSst, pstSNssai->ulSd, pstSNssai->ucMappedSst, pstSNssai->ulMappedSd);
    }
    else if ((pstSNssai->bitOpSd == VOS_TRUE)
          && (pstSNssai->bitOpMappedSst == VOS_TRUE))
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,
                                             ",\"%x.%x;%x\"", pstSNssai->ucSst, pstSNssai->ulSd, pstSNssai->ucMappedSst);
    }
    else if (pstSNssai->bitOpSd == VOS_TRUE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,
                                             ",\"%x.%x\"", pstSNssai->ucSst, pstSNssai->ulSd);
    }
    else if (pstSNssai->bitOpMappedSst == VOS_TRUE)
    {
        *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,
                                             ",\"%x;%x\"", pstSNssai->ucSst, pstSNssai->ucMappedSst);
    }
    else
    {
        if (pstSNssai->ucSst == 0)
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength, ",");
        }
        else
        {
            *pusLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + *pusLength,
                                                 ",\"%x\"", pstSNssai->ucSst);
        }
    }

    return;
}

#endif


VOS_VOID AT_RcvTafPsEvt(
    TAF_PS_EVT_STRU                     *pstEvt
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;
    MN_PS_EVT_FUNC                      pEvtFunc;
    TAF_CTRL_STRU                      *pstCtrl = VOS_NULL_PTR;

    /* ��ʼ�� */
    ulResult    = VOS_OK;
    pEvtFunc    = VOS_NULL_PTR;
    pstCtrl     = (TAF_CTRL_STRU*)(pstEvt->aucContent);

    if ( At_ClientIdToUserId(pstCtrl->usClientId,
                                           &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvTafPsEvt: At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        /* �㲥IDNEX��������Ϊ�����±�ʹ�ã���Ҫ���¼�����������ϸ�˶ԣ���������Խ�硣
           Ŀǰֻ�������ϱ�/NW ACT/NW DISCONNETΪ�㲥�¼�����Ҫ��������㲥�¼�������ϸ�˶ԣ� */
        if (AT_IsBroadcastPsEvt(pstEvt->ulEvtId) == VOS_FALSE)
        {
            AT_WARN_LOG("AT_RcvTafPsEvt: AT_BROADCAST_INDEX,but not Broadcast Event.");
            return;
        }
    }

    /* ���¼�������в��Ҵ����� */
    /* Modified by l60609 for DSDA Phase III, 2013-3-5, Begin */
    for ( i = 0; i < AT_ARRAY_SIZE(g_astAtPsEvtFuncTbl); i++ )
    /* Modified by l60609 for DSDA Phase III, 2013-3-5, End */
    {
        if ( pstEvt->ulEvtId == g_astAtPsEvtFuncTbl[i].ulEvtId )
        {
            /* �¼�IDƥ�� */
            pEvtFunc = g_astAtPsEvtFuncTbl[i].pEvtFunc;
            break;
        }
    }

    /* ������������������ */
    if ( pEvtFunc != VOS_NULL_PTR )
    {
        ulResult = pEvtFunc(ucIndex, pstEvt->aucContent);
    }
    else
    {
        AT_ERR_LOG1("AT_RcvTafPsEvt: Unexpected event received! <EvtId>",
            pstEvt->ulEvtId);
        ulResult    = VOS_ERR;
    }

    /* ���ݴ������ķ��ؽ��, �����Ƿ����AT��ʱ���Լ�������: �ý׶β����� */
    if ( ulResult != VOS_OK )
    {
        AT_ERR_LOG1("AT_RcvTafPsEvt: Can not handle this message! <MsgId>",
            pstEvt->ulEvtId);
    }

    return;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                usModemId;

    /* ��ʼ�� */
    pstEvent  = (TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU*)pEvtInfo;

    /* ��¼<CID> */
    gastAtClientTab[ucIndex].ucCid      = pstEvent->ucCid;

    /* ��¼<RabId> */
    usModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(ucIndex, &usModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsCallEvtPdpActivateCnf: Get modem id fail.");
        return AT_ERROR;
    }

    /* ����Ϊ��չRABID = modemId + rabId */
    gastAtClientTab[ucIndex].ucExPsRabId  = AT_BUILD_EXRABID(usModemId, pstEvent->ucRabId);

    /* ���PS����д����� */
    AT_PS_SetPsCallErrCause(ucIndex, TAF_PS_CAUSE_SUCCESS);

    AT_PS_AddIpAddrMap(ucIndex, pstEvent);

    /* ���ݲ������� */
    switch(gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_CGACT_ORG_SET:
        case AT_CMD_CGANS_ANS_SET:
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex,AT_OK);
            break;

        case AT_CMD_CGDATA_SET:
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_SetMode(ucIndex, AT_DATA_MODE, AT_IP_DATA_MODE);
            At_FormatResultData(ucIndex, AT_CONNECT);
            break;

        case AT_CMD_CGANS_ANS_EXT_SET:
            AT_AnswerPdpActInd(ucIndex, pstEvent);
            break;

        case AT_CMD_D_IP_CALL_SET:
        case AT_CMD_PPP_ORG_SET:
            /* Modem�����PDP����ɹ���
               AT_CMD_D_IP_CALL_SETΪPPP����
               AT_CMD_PPP_ORG_SETΪIP����
            */
            AT_ModemPsRspPdpActEvtCnfProc(ucIndex, pstEvent);
            break;

        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateRej(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent = VOS_NULL_PTR;

    pstEvent  = (TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU*)pEvtInfo;

    /* ��¼PS����д����� */
    AT_PS_SetPsCallErrCause(ucIndex, pstEvent->enCause);

    /* ���û����ͷֱ��� */
    switch (gastAtClientTab[ucIndex].UserType)
    {
        /* MODEM���Ŵ��� */
        case AT_HSUART_USER:
        case AT_MODEM_USER:
            AT_ModemPsRspPdpActEvtRejProc(ucIndex, pstEvent);
            return VOS_OK;

        /* NDIS���Ŵ��� */
        case AT_NDIS_USER:
        /* E5�����翨ʹ��ͬһ���˿��� */
        case AT_APP_USER:
            if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CGACT_ORG_SET)
            {
                /* AT+CGACT���� */
                AT_STOP_TIMER_CMD_READY(ucIndex);
                At_FormatResultData(ucIndex, AT_ERROR);
            }

            return VOS_OK;

        /* �����˿�ȫ������ERROR */
        default:
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, AT_ERROR);
            break;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpManageInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    TAF_PS_CALL_PDP_MANAGE_IND_STRU    *pstEvent = VOS_NULL_PTR;
    AT_MODEM_SS_CTX_STRU               *pstModemSsCtx = VOS_NULL_PTR;

    VOS_UINT8                           aucTempValue[TAF_MAX_APN_LEN + 1];

    /* ��ʼ�� */
    usLength  = 0;
    pstEvent  = (TAF_PS_CALL_PDP_MANAGE_IND_STRU*)pEvtInfo;

    /* ������Э�鲻�� */
    pstModemSsCtx   = AT_GetModemSsCtxAddrFromClientId(ucIndex);

    if(pstModemSsCtx->ucCrcType == AT_CRC_ENABLE_TYPE)
    {
        /* +CRC -- +CRING: GPRS <PDP_type>, <PDP_addr>[,[<L2P>][,<APN>]] */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s+CRING: GPRS ",gaucAtCrLf);

        /* <PDP_type> */
        if (pstEvent->stPdpAddr.enPdpType == TAF_PDP_IPV4)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s",gastAtStringTab[AT_STRING_IP].pucText);
        }
        else if (pstEvent->stPdpAddr.enPdpType == TAF_PDP_IPV6)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s", gastAtStringTab[AT_STRING_IPV6].pucText);
        }
        else if (pstEvent->stPdpAddr.enPdpType == TAF_PDP_IPV4V6)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s", gastAtStringTab[AT_STRING_IPV4V6].pucText);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s",gastAtStringTab[AT_STRING_PPP].pucText);
        }

        /* <PDP_addr> */
        lMemResult = memset_s(aucTempValue, sizeof(aucTempValue), 0x00, (TAF_MAX_APN_LEN + 1));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucTempValue), (TAF_MAX_APN_LEN + 1));
        AT_Ipv4Addr2Str((VOS_CHAR *)aucTempValue, sizeof(aucTempValue),
            pstEvent->stPdpAddr.aucIpv4Addr, TAF_IPV4_ADDR_LEN);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",\"%s\"",aucTempValue);

        /* <L2P>û�У�<APN> */
        lMemResult = memset_s(aucTempValue, sizeof(aucTempValue), 0x00, (TAF_MAX_APN_LEN + 1));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucTempValue), (TAF_MAX_APN_LEN + 1));
        if (pstEvent->stApn.ucLength > sizeof(pstEvent->stApn.aucValue))
        {
            AT_WARN_LOG1("AT_RcvTafPsCallEvtPdpManageInd: Invalid pstEvent->stApn.ucLength: ", pstEvent->stApn.ucLength);
            pstEvent->stApn.ucLength = sizeof(pstEvent->stApn.aucValue);
        }

        lMemResult = memcpy_s(aucTempValue, sizeof(aucTempValue), pstEvent->stApn.aucValue, pstEvent->stApn.ucLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucTempValue), pstEvent->stApn.ucLength);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",,\"%s\"%s",aucTempValue,gaucAtCrLf);
    }
    else
    {
        /* +CRC -- RING */
        if(gucAtVType == AT_V_ENTIRE_TYPE)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%sRING%s",gaucAtCrLf,gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "2\r");
        }
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    /* ������ */
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpModifyCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent      = VOS_NULL_PTR;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGCMOD_SET )
    {
        return VOS_ERR;
    }

    pstEvent = (TAF_PS_CALL_PDP_MODIFY_CNF_STRU*)pEvtInfo;

    switch ( gastAtClientTab[ucIndex].UserType )
    {
        case AT_HSUART_USER:
        case AT_MODEM_USER:
            /* ��FCָʾ�޸����ص� */
            AT_NotifyFcWhenPdpModify(pstEvent, FC_ID_MODEM);
            break;

        case AT_NDIS_USER:
        case AT_APP_USER:
            AT_PS_ProcCallModifyEvent(ucIndex, pstEvent);
            break;

        default:
            break;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpModifyRej(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGCMOD_SET )
    {
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_ERROR);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpModifiedInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent      = VOS_NULL_PTR;

    pstEvent = (TAF_PS_CALL_PDP_MODIFY_IND_STRU*)pEvtInfo;

    switch ( gastAtClientTab[ucIndex].UserType )
    {
        case AT_HSUART_USER:
        case AT_MODEM_USER:
            /* ��FCָʾ�޸����ص� */
            AT_NotifyFcWhenPdpModify(pstEvent, FC_ID_MODEM);
            break;

        case AT_NDIS_USER:
        case AT_APP_USER:
            AT_PS_ProcCallModifyEvent(ucIndex, pstEvent);
            break;

        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpDeactivateCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent = VOS_NULL_PTR;

    pstEvent  = (TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU*)pEvtInfo;

    AT_PS_DeleteIpAddrMap(ucIndex, pstEvent);

    /* ����Ӧ�����ж��Ƿ�������״̬���پ������� */
    switch(gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_CGACT_END_SET:
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);
            At_FormatResultData(ucIndex,AT_OK);
            break;

        case AT_CMD_H_PS_SET:
        case AT_CMD_PS_DATA_CALL_END_SET:
            AT_ModemPsRspPdpDeactEvtCnfProc(ucIndex, pstEvent);
            break;

        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtPdpDeactivatedInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU *pstEvent = VOS_NULL_PTR;

    pstEvent    = (TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU*)pEvtInfo;

    AT_PS_DeleteIpAddrMap(ucIndex, pstEvent);

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CGACT_END_SET)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);
        At_FormatResultData(ucIndex,AT_OK);

        return VOS_OK;
    }

    /* ��¼PS����д����� */
    AT_PS_SetPsCallErrCause(ucIndex, pstEvent->enCause);

    switch (gastAtClientTab[ucIndex].UserType)
    {
        case AT_HSUART_USER:
        case AT_USBCOM_USER:
        case AT_MODEM_USER:
        case AT_CTR_USER:
        /* ����CPE��Ʒ���ڰ汾һ����Ҫ�󣬲��������pcui�˿� */
        case AT_SOCK_USER:
            AT_ModemPsRspPdpDeactivatedEvtProc(ucIndex, pstEvent);
            break;

        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtCallEndCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_END_CNF_STRU           *pstCallEndCnf = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstCallEndCnf   = (TAF_PS_CALL_END_CNF_STRU*)pEvtInfo;

    if ( (gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
      || (gastAtClientTab[ucIndex].UserType == AT_HSUART_USER) )
    {
        AT_MODEM_ProcCallEndCnfEvent(ucIndex, pstCallEndCnf);
        return VOS_OK;
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtCallModifyCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_MODIFY_CNF_STRU        *pstCallModifyCnf = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstCallModifyCnf = (TAF_PS_CALL_MODIFY_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGCMOD_SET )
    {
        return VOS_ERR;
    }

    /*----------------------------------------------------------
       (1)Э��ջ�쳣����, δ����PDP�޸�, ֱ���ϱ�ERROR
       (2)Э��ջ����, ����PDP�޸�, ����PDP�޸��¼����ؽ��
    ----------------------------------------------------------*/
    if ( pstCallModifyCnf->enCause != TAF_PS_CAUSE_SUCCESS )
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtCallAnswerCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_ANSWER_CNF_STRU        *pstCallAnswerCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    /* ��ʼ�� */
    pstCallAnswerCnf = (TAF_PS_CALL_ANSWER_CNF_STRU*)pEvtInfo;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafPsCallEvtCallAnswerCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��鵱ǰ����Ĳ������� */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGANS_ANS_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGANS_ANS_EXT_SET))
    {
        return VOS_ERR;
    }

    /*----------------------------------------------------------
       (1)Э��ջ�쳣����, δ����PDPӦ��, ֱ���ϱ�ERROR
       (2)Э��ջ����, ����PDPӦ��, ����PDP�����¼����ؽ��
    ----------------------------------------------------------*/

    /* IP���͵�Ӧ����Ҫ�ȸ��ϲ��CONNECT */
    if (pstCallAnswerCnf->enCause == TAF_ERR_AT_CONNECT)
    {
        ulResult = At_SetDialGprsPara(ucIndex,
                                      pstCallAnswerCnf->ucCid,
                                      TAF_IP_ACTIVE_TE_PPP_MT_PPP_TYPE);

        /* �����connect��CmdCurrentOpt���壬At_RcvTeConfigInfoReq��ʹ�� */
        if (ulResult == AT_ERROR)
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
        }

        AT_StopRelTimer(ucIndex, &gastAtClientTab[ucIndex].hTimer);
        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;
        gastAtClientTab[ucIndex].opId = 0;
        At_FormatResultData(ucIndex, ulResult);

        return VOS_OK;
    }

    /* �������������ERROR */
    if (pstCallAnswerCnf->enCause != TAF_ERR_NO_ERROR)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsCallEvtCallHangupCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_HANGUP_CNF_STRU        *pstCallHangUpCnf = VOS_NULL_PTR;

    pstCallHangUpCnf  = (TAF_PS_CALL_HANGUP_CNF_STRU*)pEvtInfo;

    if (pstCallHangUpCnf->enCause == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    /* ���ݲ������� */
    switch(gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_CGANS_ANS_SET:
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex,ulResult);
            break;

        default:
            break;
    }

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_RcvTafPsEvtSet5gQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_5G_QOS_INFO_CNF_STRU    *pstSet5gQosInfoCnf = VOS_NULL_PTR;

    pstSet5gQosInfoCnf = (TAF_PS_SET_5G_QOS_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_C5GQOS_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSet5gQosInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGet5gQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT32                          ulTmp = 0;

    TAF_5G_QOS_EXT_STRU                 stCg5qos;
    TAF_PS_GET_5G_QOS_INFO_CNF_STRU    *pstGet5gQosInfoCnf = VOS_NULL_PTR;

    memset_s(&stCg5qos, sizeof(stCg5qos), 0x00, sizeof(TAF_5G_QOS_EXT_STRU));

    pstGet5gQosInfoCnf = (TAF_PS_GET_5G_QOS_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_C5GQOS_READ )
    {
        return VOS_ERR;
    }

    for(ulTmp = 0; ulTmp < pstGet5gQosInfoCnf->ulCidNum; ulTmp++)
    {
        if(ulTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCg5qos, sizeof(stCg5qos), &pstGet5gQosInfoCnf->ast5gQosInfo[ulTmp], sizeof(TAF_5G_QOS_EXT_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCg5qos), sizeof(TAF_5G_QOS_EXT_STRU));

        /* +C5GQOS:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCg5qos.ucCid);
        /* <5QI> */
        if(stCg5qos.bitOp5QI == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCg5qos.uc5QI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <DL GFBR> */
        if(stCg5qos.bitOpDLGFBR == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCg5qos.ulDLGFBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <UL GFBR> */
        if(stCg5qos.bitOpULGFBR == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCg5qos.ulULGFBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <DL MFBR> */
        if(stCg5qos.bitOpDLMFBR == 1)
        {
           usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCg5qos.ulDLMFBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <UL MFBR> */
        if(stCg5qos.bitOpULMFBR == 1)
        {
           usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCg5qos.ulULMFBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_VOID AT_PrintDynamic5gQosInfo(
    TAF_PS_GET_DYNAMIC_5G_QOS_INFO_CNF_STRU                *pstGetDynamic5gQosInfoCnf,
    VOS_UINT8                                               ucIndex
)
{
    TAF_PS_5G_DYNAMIC_QOS_EXT_STRU      st5gDynamicQos;
    VOS_UINT16                          usLength;
    VOS_UINT32                          i;
    errno_t                             lMemResult;

    usLength = 0;
    memset_s(&st5gDynamicQos, sizeof(st5gDynamicQos), 0x00, sizeof(TAF_PS_5G_DYNAMIC_QOS_EXT_STRU));

    /* AT�����ʽ: [+C5GQOSRDP: <cid>,<5QI>[,<DL_GFBR>,<UL_GFBR>[,<DL_MFBR>,<UL_MFBR>[,<DL_SAMBR>,<UL_SAMBR>[,<Averaging_window>]]]]] */
    for (i = 0; i < pstGetDynamic5gQosInfoCnf->ulCidNum; i++)
    {
        if(i != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&st5gDynamicQos, sizeof(st5gDynamicQos), &pstGetDynamic5gQosInfoCnf->ast5gQosInfo[i], sizeof(TAF_PS_5G_DYNAMIC_QOS_EXT_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(st5gDynamicQos), sizeof(TAF_PS_5G_DYNAMIC_QOS_EXT_STRU));

        /* +C5GQOSRDP:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",st5gDynamicQos.ucCid);

        /* <QCI> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",st5gDynamicQos.uc5QI);

        /* �����������������ھʹ�ӡ������Ŀǰ�������ַ�ʽ��ӡ���� */
        /* <DL GFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.bitOpDLGFBR, st5gDynamicQos.ulDLGFBR, &usLength);

        /* <UL GFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.bitOpULGFBR, st5gDynamicQos.ulULGFBR, &usLength);

        /* <DL MFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.bitOpDLMFBR, st5gDynamicQos.ulDLMFBR, &usLength);

        /* <UL MFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.bitOpULMFBR, st5gDynamicQos.ulULMFBR, &usLength);

        /* <Session ambr> */
        if(st5gDynamicQos.bitOpSAMBR == VOS_TRUE)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",st5gDynamicQos.stAmbr.ulDLSessionAmbr);

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",st5gDynamicQos.stAmbr.ulULSessionAmbr);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",,");
        }

        /* <Average Window> */
        if(st5gDynamicQos.bitOpAveragWindow == VOS_TRUE)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",st5gDynamicQos.usAveragWindow);
        }
    }

    gstAtSendData.usBufLen  = usLength;

    return;
}


VOS_UINT32 AT_RcvTafPsEvtGetDynamic5gQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_DYNAMIC_5G_QOS_INFO_CNF_STRU                *pstGetDynamic5gQosInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                                              ulResult                  = AT_FAILURE;

    pstGetDynamic5gQosInfoCnf = (TAF_PS_GET_DYNAMIC_5G_QOS_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_C5GQOSRDP_SET)
    {
        return VOS_ERR;
    }

    if (pstGetDynamic5gQosInfoCnf->enCause == TAF_PS_CAUSE_SUCCESS)
    {
        AT_PrintDynamic5gQosInfo(pstGetDynamic5gQosInfoCnf, ucIndex);
        ulResult                = AT_OK;
    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtUePolicyRptInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_UE_POLICY_RPT_IND_STRU      *pstUePolicyInd = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;

    pstUePolicyInd = (TAF_PS_UE_POLICY_RPT_IND_STRU *)pEvtInfo;
    enModemId      = MODEM_ID_0;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsEvtUePolicyRptInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* ^CPOLICYRPT: <total_len>,<section_num> */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      "%s^CPOLICYRPT: %d,%d%s",
                                      gaucAtCrLf,
                                      pstUePolicyInd->usTotalLength,
                                      pstUePolicyInd->usSectionNum,
                                      gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetUePolicyRptCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_UE_POLICY_RPT_CNF_STRU  *pstSetUePolicyRptCnf = VOS_NULL_PTR;

    pstSetUePolicyRptCnf = (TAF_PS_SET_UE_POLICY_RPT_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CPOLICYRPT_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetUePolicyRptCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetUePolicyCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_UE_POLICY_CNF_STRU      *pstGetUePolicytCnf = VOS_NULL_PTR;
    VOS_UINT16                          usLength;

    pstGetUePolicytCnf = (TAF_PS_GET_UE_POLICY_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CPOLICYCODE_QRY)
    {
        return VOS_ERR;
    }

    if (pstGetUePolicytCnf->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        /* ��������� */
        AT_PrcoPsEvtErrCode(ucIndex, pstGetUePolicytCnf->enCause);

        return VOS_OK;
    }

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s%s: %d,",
                                       gaucAtCrLf,
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstGetUePolicytCnf->ucIndex);

    /* ��16������ת��ΪASCII�� */
    usLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN,
                                                        (TAF_INT8 *)pgucAtSndCodeAddr,
                                                        (TAF_UINT8 *)pgucAtSndCodeAddr+usLength,
                                                        pstGetUePolicytCnf->usLength,
                                                        pstGetUePolicytCnf->aucContent);

    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,AT_OK);

    return VOS_OK;
}

#endif


VOS_UINT32 AT_RcvTafPsEvtSetPrimPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_CNF_STRU  *pstSetPdpCtxInfoCnf = VOS_NULL_PTR;

    pstSetPdpCtxInfoCnf = (TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDCONT_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetPdpCtxInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetPrimPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;
    TAF_PRI_PDP_QUERY_INFO_STRU         stCgdcont;
    TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF_STRU *pstGetPrimPdpCtxInfoCnf = VOS_NULL_PTR;

    VOS_UINT8                           aucStr[TAF_MAX_APN_LEN + 1];

    memset_s(&stCgdcont, sizeof(stCgdcont), 0x00, sizeof(TAF_PRI_PDP_QUERY_INFO_STRU));
    pstGetPrimPdpCtxInfoCnf = (TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDCONT_READ )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstGetPrimPdpCtxInfoCnf->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgdcont, sizeof(stCgdcont), &pstGetPrimPdpCtxInfoCnf->astPdpContextQueryInfo[ucTmp], sizeof(TAF_PRI_PDP_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgdcont), sizeof(TAF_PRI_PDP_QUERY_INFO_STRU));

        /* +CGDCONT:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgdcont.ucCid);
        /* <PDP_type> */
        if (stCgdcont.stPriPdpInfo.stPdpAddr.enPdpType == TAF_PDP_IPV4)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_IP].pucText);
        }
        else if (stCgdcont.stPriPdpInfo.stPdpAddr.enPdpType == TAF_PDP_IPV6)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", gastAtStringTab[AT_STRING_IPV6].pucText);
        }
        else if (stCgdcont.stPriPdpInfo.stPdpAddr.enPdpType == TAF_PDP_IPV4V6)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", gastAtStringTab[AT_STRING_IPV4V6].pucText);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_PPP].pucText);
        }
        /* <APN> */
        memset_s(aucStr, sizeof(aucStr), 0x00, sizeof(aucStr));

        if (stCgdcont.stPriPdpInfo.stApn.ucLength > sizeof(stCgdcont.stPriPdpInfo.stApn.aucValue))
        {
            AT_WARN_LOG1("AT_RcvTafPsEvtGetPrimPdpContextInfoCnf: stCgdcont.stPriPdpInfo.stApn.ucLength: ",
                stCgdcont.stPriPdpInfo.stApn.ucLength);
            stCgdcont.stPriPdpInfo.stApn.ucLength = sizeof(stCgdcont.stPriPdpInfo.stApn.aucValue);
        }

        lMemResult = memcpy_s(aucStr, sizeof(aucStr), stCgdcont.stPriPdpInfo.stApn.aucValue, stCgdcont.stPriPdpInfo.stApn.ucLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucStr), stCgdcont.stPriPdpInfo.stApn.ucLength);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"", aucStr);
        /* <PDP_addr> */
        memset_s(aucStr, sizeof(aucStr), 0x00, sizeof(aucStr));
        AT_Ipv4Addr2Str((VOS_CHAR *)aucStr, sizeof(aucStr), stCgdcont.stPriPdpInfo.stPdpAddr.aucIpv4Addr, TAF_IPV4_ADDR_LEN);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"", aucStr);
        /* <d_comp> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enPdpDcomp);
        /* <h_comp> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enPdpHcomp);


        /* <IPv4AddrAlloc>  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enIpv4AddrAlloc);
        /* <Emergency Indication> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enEmergencyInd);
        /* <P-CSCF_discovery> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enPcscfDiscovery);
        /* <IM_CN_Signalling_Flag_Ind> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enImCnSignalFlg);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enNasSigPrioInd);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        /* the following five parameters omit: securePCO, Ipv4_mtu_discovery, Local_Addr_ind, Non-Ip_Mtu_discovery and Reliable data service */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s", ",,,,,");

        /* ssc mode */
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enSscMode);
        /* SNssai */

        AT_ConvertSNssaiToString(&stCgdcont.stPriPdpInfo.stSNssai, &usLength);

        /* Pref Access Type */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enPrefAccessType);
        /* Reflect Qos Ind */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enRQosInd);
        /* Ipv6 multi-homing */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enMh6Pdu);
        /* Always on ind */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.stPriPdpInfo.enAlwaysOnInd);
#endif
    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                              ulResult;
    VOS_UINT16                              usLength ;
    VOS_UINT8                               ucTmp ;
    TAF_PS_GET_PDP_CONTEXT_INFO_CNF_STRU   *pstGetPdpCtxInfoCnf = VOS_NULL_PTR;

    ulResult            = AT_FAILURE;
    usLength            = 0;
    ucTmp               = 0;
    pstGetPdpCtxInfoCnf = (TAF_PS_GET_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGPADDR_TEST )
    {
        return VOS_ERR;
    }

    /* +CGPADDR:  */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"(");

    for(ucTmp = 0; ucTmp < pstGetPdpCtxInfoCnf->ulCidNum; ucTmp++)
    {
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",pstGetPdpCtxInfoCnf->ulCid[ucTmp]);

        if (((VOS_UINT32)ucTmp + 1) >= pstGetPdpCtxInfoCnf->ulCidNum)
        {
            break;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
    }
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,")");

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetSecPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_SEC_PDP_CONTEXT_INFO_CNF_STRU   *pstSetPdpCtxInfoCnf = VOS_NULL_PTR;

    pstSetPdpCtxInfoCnf = (TAF_PS_SET_SEC_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDSCONT_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetPdpCtxInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetSecPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;
    TAF_PDP_SEC_CONTEXT_STRU            stSecPdpInfo;
    TAF_PS_GET_SEC_PDP_CONTEXT_INFO_CNF_STRU *pstGetSecPdpCtxInfoCnf = VOS_NULL_PTR;

    memset_s(&stSecPdpInfo, sizeof(stSecPdpInfo), 0x00, sizeof(TAF_PDP_SEC_CONTEXT_STRU));

    pstGetSecPdpCtxInfoCnf = (TAF_PS_GET_SEC_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDSCONT_READ )
    {
        return VOS_ERR;
    }

    for (ucTmp = 0; ucTmp < pstGetSecPdpCtxInfoCnf->ulCidNum; ucTmp++)
    {
        if (ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stSecPdpInfo, sizeof(stSecPdpInfo), &pstGetSecPdpCtxInfoCnf->astPdpContextQueryInfo[ucTmp], sizeof(TAF_PDP_SEC_CONTEXT_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stSecPdpInfo), sizeof(TAF_PDP_SEC_CONTEXT_STRU));
        /* +CGDSCONT:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stSecPdpInfo.ucCid);
        /* <p_cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stSecPdpInfo.ucLinkdCid);
        /* <d_comp> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stSecPdpInfo.enPdpDcomp);
        /* <h_comp> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stSecPdpInfo.enPdpHcomp);
        /* <IM_CN_Signalling_Flag_Ind> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stSecPdpInfo.enImCnSignalFlg);

    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtSetTftInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_TFT_INFO_CNF_STRU       *pstSetTftInfoCnf = VOS_NULL_PTR;

    pstSetTftInfoCnf = (TAF_PS_SET_TFT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGTFT_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetTftInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetTftInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp1 = 0;
    VOS_UINT8                           ucTmp2 = 0;
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];
    VOS_CHAR                            aucLocalIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_UINT8                           aucLocalIpv6Mask[APP_MAX_IPV6_ADDR_LEN];
    TAF_TFT_QUREY_INFO_STRU            *pstCgtft = VOS_NULL_PTR;
    TAF_PS_GET_TFT_INFO_CNF_STRU       *pstGetTftInfoCnf = VOS_NULL_PTR;

    memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, sizeof(acIpv4StrTmp));
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, sizeof(acIpv6StrTmp));
    memset_s(aucLocalIpv4StrTmp, sizeof(aucLocalIpv4StrTmp), 0x00, sizeof(aucLocalIpv4StrTmp));
    memset_s(aucLocalIpv6Mask, sizeof(aucLocalIpv6Mask), 0x00, sizeof(aucLocalIpv6Mask));

    pstGetTftInfoCnf = (TAF_PS_GET_TFT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGTFT_READ )
    {
        return VOS_ERR;
    }

    /* ��̬�����ڴ� */
    pstCgtft = (TAF_TFT_QUREY_INFO_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                                                       sizeof(TAF_TFT_QUREY_INFO_STRU));
    if (pstCgtft == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    for (ucTmp1 = 0; ucTmp1 < pstGetTftInfoCnf->ulCidNum; ucTmp1++)
    {

        lMemResult = memcpy_s(pstCgtft, sizeof(TAF_TFT_QUREY_INFO_STRU), &pstGetTftInfoCnf->astTftQueryInfo[ucTmp1], sizeof(TAF_TFT_QUREY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_TFT_QUREY_INFO_STRU), sizeof(TAF_TFT_QUREY_INFO_STRU));

        for (ucTmp2= 0; ucTmp2 < pstCgtft->ucPfNum; ucTmp2++)
        {
            if (!(ucTmp1 == 0 && ucTmp2 == 0))
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%s",gaucAtCrLf);
            }
            /* +CGTFT:  */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            /* <cid> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%d", pstCgtft->ucCid);
            /* <packet filter identifier> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%d", pstCgtft->astPfInfo[ucTmp2].ucPacketFilterId);
            /* <evaluation precedence index> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%d", pstCgtft->astPfInfo[ucTmp2].ucPrecedence);
            /* <source address and subnet mask> */
            AT_ConvertIpAddrAndMaskParaToString(pstCgtft, ucTmp2, &usLength);

            /* <protocol number (ipv4) / next header (ipv6)> */
            AT_ConvertULParaToString(pstCgtft->astPfInfo[ucTmp2].bitOpProtocolId, pstCgtft->astPfInfo[ucTmp2].ucProtocolId, &usLength);

            /* <destination port range> */
            AT_ConvertRangeParaToString(pstCgtft->astPfInfo[ucTmp2].bitOpSingleLocalPort,
                                            pstCgtft->astPfInfo[ucTmp2].bitOpLocalPortRange,
                                            pstCgtft->astPfInfo[ucTmp2].usLcPortLowLimit,
                                            pstCgtft->astPfInfo[ucTmp2].usLcPortHighLimit,
                                            &usLength);

            /* <source port range> */
            AT_ConvertRangeParaToString(pstCgtft->astPfInfo[ucTmp2].bitOpSingleRemotePort,
                                        pstCgtft->astPfInfo[ucTmp2].bitOpRemotePortRange,
                                        pstCgtft->astPfInfo[ucTmp2].usRmtPortLowLimit,
                                        pstCgtft->astPfInfo[ucTmp2].usRmtPortHighLimit,
                                        &usLength);

            /* <ipsec security parameter index (spi)> */
            if (pstCgtft->astPfInfo[ucTmp2].bitOpSecuParaIndex == 1)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",\"%X\"", pstCgtft->astPfInfo[ucTmp2].ulSecuParaIndex);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",");
            }

            /* <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask> */
            AT_ConvertRangeParaToString(VOS_FALSE,
                                            pstCgtft->astPfInfo[ucTmp2].bitOpTypeOfService,
                                            pstCgtft->astPfInfo[ucTmp2].ucTypeOfService,
                                            pstCgtft->astPfInfo[ucTmp2].ucTypeOfServiceMask,
                                            &usLength);

            /* <flow label (ipv6)> */
            if (pstCgtft->astPfInfo[ucTmp2].bitOpFlowLabelType == VOS_TRUE)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%X", pstCgtft->astPfInfo[ucTmp2].ulFlowLabelType);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",");
            }
            /* <direction> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%d", pstCgtft->astPfInfo[ucTmp2].enDirection);

            if (AT_IsSupportReleaseRst(AT_ACCESS_STRATUM_REL11))
            {
                /* <local address and subnet mask> */
                if ( pstCgtft->astPfInfo[ucTmp2].bitOpLocalIpv4AddrAndMask == VOS_TRUE )
                {
                    AT_Ipv4AddrItoa(aucLocalIpv4StrTmp, sizeof(aucLocalIpv4StrTmp), pstCgtft->astPfInfo[ucTmp2].aucLocalIpv4Addr);
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s",aucLocalIpv4StrTmp);

                    AT_Ipv4AddrItoa(aucLocalIpv4StrTmp, sizeof(aucLocalIpv4StrTmp), pstCgtft->astPfInfo[ucTmp2].aucLocalIpv4Mask);
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,".%s\"",aucLocalIpv4StrTmp);
                }
                else if ( pstCgtft->astPfInfo[ucTmp2].bitOpLocalIpv6AddrAndMask == VOS_TRUE )
                {
                    AT_GetIpv6MaskByPrefixLength(pstCgtft->astPfInfo[ucTmp2].ucLocalIpv6Prefix, aucLocalIpv6Mask);
                    (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp,
                                                           pstCgtft->astPfInfo[ucTmp2].aucLocalIpv6Addr,
                                                           aucLocalIpv6Mask);
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",\"%s\"", acIpv6StrTmp);
                }
                else
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",");
                }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
                AT_ConvertULParaToString(pstCgtft->astPfInfo[ucTmp2].bitOpQri, pstCgtft->astPfInfo[ucTmp2].ucQri, &usLength);
#endif
            }

        }
    }

    /* �ͷŶ�̬������ڴ� */
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, pstCgtft);
    /*lint -restore */
    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    /* ��������� */
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetUmtsQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_UMTS_QOS_INFO_CNF_STRU  *pstSetUmtsQosInfoCnf = VOS_NULL_PTR;

    pstSetUmtsQosInfoCnf = (TAF_PS_SET_UMTS_QOS_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQREQ_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetUmtsQosInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetUmtsQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    TAF_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    TAF_UINT8                           ucTmp = 0;
    TAF_PS_GET_UMTS_QOS_INFO_CNF_STRU  *pstUmtsQosInfo = VOS_NULL_PTR;
    TAF_UMTS_QOS_QUERY_INFO_STRU        stCgeq;

    memset_s(&stCgeq, sizeof(stCgeq), 0x00, sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));

    pstUmtsQosInfo = (TAF_PS_GET_UMTS_QOS_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQREQ_READ )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstUmtsQosInfo->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgeq, sizeof(stCgeq), &pstUmtsQosInfo->astUmtsQosQueryInfo[ucTmp], sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgeq), sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));
        /* +CGEQREQ:+CGEQMIN   */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgeq.ucCid);
        /* <Traffic class> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucTrafficClass);
        /* <Maximum bitrate UL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulMaxBitUl);
        /* <Maximum bitrate DL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulMaxBitDl);
        /* <Guaranteed bitrate UL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulGuarantBitUl);
        /* <Guaranteed bitrate DL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulGuarantBitDl);
        /* <Delivery order> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucDeliverOrder);
        /* <Maximum SDU size> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.usMaxSduSize);
        /* <SDU error ratio> */
        switch(stCgeq.stQosInfo.ucSduErrRatio)
        {
        case 0:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_0E0].pucText);
            break;

        case 1:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E2].pucText);
            break;

        case 2:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_7E3].pucText);
            break;

        case 3:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E3].pucText);
            break;

        case 4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E4].pucText);
            break;

        case 5:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E5].pucText);
            break;

        case 6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E6].pucText);
            break;

        case 7:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E1].pucText);
            break;

        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            break;
        }
        /* <Residual bit error ratio> */
        switch(stCgeq.stQosInfo.ucResidualBer)
        {
        case 0:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_0E0].pucText);
            break;

        case 1:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_5E2].pucText);
            break;

        case 2:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E2].pucText);
            break;

        case 3:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_5E3].pucText);
            break;

        case 4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_4E3].pucText);
            break;

        case 5:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E3].pucText);
            break;

        case 6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E4].pucText);
            break;

        case 7:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E5].pucText);
            break;

        case 8:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E6].pucText);
            break;

        case 9:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_6E8].pucText);
            break;

        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            break;
        }
        /* <Delivery of erroneous SDUs> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucDeliverErrSdu);
        /* <Transfer delay> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.usTransDelay);
        /* <Traffic handling priority> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucTraffHandlePrior);

        /* <Source Statistics Descriptor> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucSrcStatisticsDescriptor);
        /* <Signalling Indication> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucSignallingIndication);
    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetUmtsQosMinInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_UMTS_QOS_MIN_INFO_CNF_STRU  *pstSetUmtsQosMinInfoCnf = VOS_NULL_PTR;

    pstSetUmtsQosMinInfoCnf = (TAF_PS_SET_UMTS_QOS_MIN_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQMIN_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetUmtsQosMinInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetUmtsQosMinInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                                 lMemResult;
    TAF_UINT32                              ulResult;
    TAF_UINT16                              usLength = 0;
    TAF_UINT8                               ucTmp = 0;
    TAF_PS_GET_UMTS_QOS_MIN_INFO_CNF_STRU  *pstUmtsQosMinInfo = VOS_NULL_PTR;
    TAF_UMTS_QOS_QUERY_INFO_STRU            stCgeq;

    memset_s(&stCgeq, sizeof(stCgeq), 0x00, sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));

    pstUmtsQosMinInfo = (TAF_PS_GET_UMTS_QOS_MIN_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQMIN_READ )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstUmtsQosMinInfo->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgeq, sizeof(stCgeq), &pstUmtsQosMinInfo->astUmtsQosQueryInfo[ucTmp], sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgeq), sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));
        /* +CGEQREQ:+CGEQMIN   */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",stCgeq.ucCid);
        /* <Traffic class> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucTrafficClass);
        /* <Maximum bitrate UL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulMaxBitUl);
        /* <Maximum bitrate DL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulMaxBitDl);
        /* <Guaranteed bitrate UL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulGuarantBitUl);
        /* <Guaranteed bitrate DL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulGuarantBitDl);
        /* <Delivery order> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucDeliverOrder);
        /* <Maximum SDU size> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.usMaxSduSize);
        /* <SDU error ratio> */
        switch(stCgeq.stQosInfo.ucSduErrRatio)
        {
        case 0:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_0E0].pucText);
            break;

        case 1:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E2].pucText);
            break;

        case 2:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_7E3].pucText);
            break;

        case 3:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E3].pucText);
            break;

        case 4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E4].pucText);
            break;

        case 5:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E5].pucText);
            break;

        case 6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E6].pucText);
            break;

        case 7:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E1].pucText);
            break;

        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            break;
        }
        /* <Residual bit error ratio> */
        switch(stCgeq.stQosInfo.ucResidualBer)
        {
        case 0:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_0E0].pucText);
            break;

        case 1:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_5E2].pucText);
            break;

        case 2:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E2].pucText);
            break;

        case 3:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_5E3].pucText);
            break;

        case 4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_4E3].pucText);
            break;

        case 5:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E3].pucText);
            break;

        case 6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E4].pucText);
            break;

        case 7:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E5].pucText);
            break;

        case 8:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E6].pucText);
            break;

        case 9:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_6E8].pucText);
            break;

        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            break;
        }
        /* <Delivery of erroneous SDUs> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucDeliverErrSdu);
        /* <Transfer delay> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.usTransDelay);
        /* <Traffic handling priority> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucTraffHandlePrior);

        /* <Source Statistics Descriptor> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucSrcStatisticsDescriptor);
        /* <Signalling Indication> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucSignallingIndication);
    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDynamicUmtsQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                                     lMemResult;
    VOS_UINT32                                  ulResult;
    VOS_UINT16                                  usLength = 0;
    VOS_UINT8                                   ucTmp = 0;
    TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_CNF_STRU  *pstDynUmtsQosMinInfo = VOS_NULL_PTR;
    TAF_UMTS_QOS_QUERY_INFO_STRU                stCgeq;

    memset_s(&stCgeq, sizeof(stCgeq), 0x00, sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));

    pstDynUmtsQosMinInfo = (TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQNEG_SET )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstDynUmtsQosMinInfo->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgeq, sizeof(stCgeq), &pstDynUmtsQosMinInfo->astUmtsQosQueryInfo[ucTmp], sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgeq), sizeof(TAF_UMTS_QOS_QUERY_INFO_STRU));
        /* +CGEQREQ:+CGEQMIN   */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgeq.ucCid);
        /* <Traffic class> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucTrafficClass);
        /* <Maximum bitrate UL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulMaxBitUl);
        /* <Maximum bitrate DL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulMaxBitDl);
        /* <Guaranteed bitrate UL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulGuarantBitUl);
        /* <Guaranteed bitrate DL> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ulGuarantBitDl);
        /* <Delivery order> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucDeliverOrder);
        /* <Maximum SDU size> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.usMaxSduSize);
        /* <SDU error ratio> */
        switch(stCgeq.stQosInfo.ucSduErrRatio)
        {
        case 0:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_0E0].pucText);
            break;

        case 1:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E2].pucText);
            break;

        case 2:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_7E3].pucText);
            break;

        case 3:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E3].pucText);
            break;

        case 4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E4].pucText);
            break;

        case 5:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E5].pucText);
            break;

        case 6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E6].pucText);
            break;

        case 7:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E1].pucText);
            break;

        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            break;
        }
        /* <Residual bit error ratio> */
        switch(stCgeq.stQosInfo.ucResidualBer)
        {
        case 0:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_0E0].pucText);
            break;

        case 1:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_5E2].pucText);
            break;

        case 2:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E2].pucText);
            break;

        case 3:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_5E3].pucText);
            break;

        case 4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_4E3].pucText);
            break;

        case 5:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E3].pucText);
            break;

        case 6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E4].pucText);
            break;

        case 7:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E5].pucText);
            break;

        case 8:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_1E6].pucText);
            break;

        case 9:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%s",gastAtStringTab[AT_STRING_6E8].pucText);
            break;

        default:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            break;
        }
        /* <Delivery of erroneous SDUs> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucDeliverErrSdu);
        /* <Transfer delay> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.usTransDelay);
        /* <Traffic handling priority> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeq.stQosInfo.ucTraffHandlePrior);
    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetPdpStateCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_PDP_STATE_CNF_STRU      *pstSetPdpStateCnf = VOS_NULL_PTR;

    pstSetPdpStateCnf = (TAF_PS_SET_PDP_STATE_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGACT_ORG_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGACT_END_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDATA_SET) )
    {
        return VOS_ERR;
    }

    /*----------------------------------------------------------
       (1)Э��ջ�쳣����, δ����PDP����, ֱ���ϱ�ERROR
       (2)Э��ջ����, ����PDP����, ����PDP�����¼����ؽ��
    ----------------------------------------------------------*/

    if (pstSetPdpStateCnf->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        /* ��¼PS����д����� */
        AT_PS_SetPsCallErrCause(ucIndex, pstSetPdpStateCnf->enCause);

        AT_STOP_TIMER_CMD_READY(ucIndex);

        if (pstSetPdpStateCnf->enCause == TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT)
        {
            At_FormatResultData(ucIndex, AT_CME_PDP_ACT_LIMIT);
        }
        else
        {
            At_FormatResultData(ucIndex, AT_ERROR);
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtCgactQryCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;
    TAF_CID_STATE_STRU                  stCgact;
    TAF_PS_GET_PDP_STATE_CNF_STRU      *pstPdpState = VOS_NULL_PTR;

    pstPdpState = (TAF_PS_GET_PDP_STATE_CNF_STRU *)pEvtInfo;

    memset_s(&stCgact, sizeof(stCgact), 0x00, sizeof(TAF_CID_STATE_STRU));

    /* ��鵱ǰ����Ĳ������� */
    for (ucTmp = 0; ucTmp < pstPdpState->ulCidNum; ucTmp++)
    {
        if (ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgact, sizeof(stCgact), &pstPdpState->astCidStateInfo[ucTmp], sizeof(TAF_CID_STATE_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgact), sizeof(TAF_CID_STATE_STRU));
        /* +CGACT:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgact.ucCid);
        /* <state> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgact.ucState);
    }

    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtCgeqnegTestCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulQosnegNum;
    VOS_UINT32                          ulTmp;
    TAF_CID_STATE_STRU                  stCgact;
    TAF_PS_GET_PDP_STATE_CNF_STRU      *pstPdpState = VOS_NULL_PTR;

    usLength    = 0;
    ulQosnegNum = 0;
    pstPdpState = (TAF_PS_GET_PDP_STATE_CNF_STRU *)pEvtInfo;

    memset_s(&stCgact, sizeof(stCgact), 0x00, sizeof(TAF_CID_STATE_STRU));

    /* CGEQNEG�Ĳ������� */

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", "(");

    for(ulTmp = 0; ulTmp < pstPdpState->ulCidNum; ulTmp++)
    {
        lMemResult = memcpy_s(&stCgact, sizeof(stCgact), &pstPdpState->astCidStateInfo[ulTmp], sizeof(TAF_CID_STATE_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgact), sizeof(TAF_CID_STATE_STRU));

        if (stCgact.ucState == TAF_PDP_ACTIVE)
        {   /*�����CID�Ǽ���̬,���ӡ��CID�Ϳ��ܵ�һ������;����������CID*/
            if (ulQosnegNum == 0 )
            {   /*����ǵ�һ��CID����CIDǰ����ӡ����*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",stCgact.ucCid);
            }
            else
            {   /*������ǵ�һ��CID����CIDǰ��ӡ����*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",stCgact.ucCid);
            }

            ulQosnegNum ++;
        }
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", ")");

    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtGetPdpStateCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{

    /* ��鵱ǰ����Ĳ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CGACT_READ)
    {
        return AT_RcvTafPsEvtCgactQryCnf(ucIndex, pEvtInfo);
    }
    else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CGEQNEG_TEST)
    {
        return AT_RcvTafPsEvtCgeqnegTestCnf(ucIndex, pEvtInfo);

    }
    else
    {
        return VOS_ERR;
    }


}


VOS_UINT32 AT_RcvTafPsEvtGetPdpIpAddrInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                               lMemResult;
    VOS_UINT16                            usLength = 0;
    VOS_UINT8                             ucTmp = 0;
    VOS_CHAR                              aStrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                              acIPv6Str[TAF_MAX_IPV6_ADDR_DOT_STR_LEN];
    TAF_PDP_ADDR_QUERY_INFO_STRU          stPdpAddrQuery;
    TAF_PS_GET_PDP_IP_ADDR_INFO_CNF_STRU *pstPdpIpAddr = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstPdpIpAddr = (TAF_PS_GET_PDP_IP_ADDR_INFO_CNF_STRU *)pEvtInfo;
    memset_s(aStrTmp, sizeof(aStrTmp), 0x00, sizeof(aStrTmp));

    memset_s(acIPv6Str, sizeof(acIPv6Str), 0x00, sizeof(acIPv6Str));
    memset_s(&stPdpAddrQuery, sizeof(stPdpAddrQuery), 0x00, sizeof(TAF_PDP_ADDR_QUERY_INFO_STRU));

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGPADDR_SET )
    {
        return VOS_ERR;
    }

    for (ucTmp = 0; ucTmp < pstPdpIpAddr->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%s", gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stPdpAddrQuery, sizeof(stPdpAddrQuery), &pstPdpIpAddr->astPdpAddrQueryInfo[ucTmp], sizeof(TAF_PDP_ADDR_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stPdpAddrQuery), sizeof(TAF_PDP_ADDR_QUERY_INFO_STRU));


        /* +CGPADDR:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%s: ", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%d", stPdpAddrQuery.ucCid);

        /* <PDP_addr> */
        if ( (stPdpAddrQuery.stPdpAddr.enPdpType == TAF_PDP_IPV4)
          || (stPdpAddrQuery.stPdpAddr.enPdpType == TAF_PDP_PPP) )
        {
            AT_Ipv4AddrItoa(aStrTmp, sizeof(aStrTmp), stPdpAddrQuery.stPdpAddr.aucIpv4Addr);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%s\"", aStrTmp);
        }
        else if (stPdpAddrQuery.stPdpAddr.enPdpType == TAF_PDP_IPV6)
        {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIPv6Str,
                                                   stPdpAddrQuery.stPdpAddr.aucIpv6Addr,
                                                   VOS_NULL_PTR);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",\"%s\"", acIPv6Str);
        }
        else if (stPdpAddrQuery.stPdpAddr.enPdpType == TAF_PDP_IPV4V6)
        {
            AT_Ipv4AddrItoa(aStrTmp, sizeof(aStrTmp), stPdpAddrQuery.stPdpAddr.aucIpv4Addr);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"%s\"", aStrTmp);

            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIPv6Str,
                                                   stPdpAddrQuery.stPdpAddr.aucIpv6Addr,
                                                   VOS_NULL_PTR);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",\"%s\"", acIPv6Str);
        }
        else
        {
            /* TAF_PDP_TYPE_BUTT */
            return VOS_ERR;
        }
    }

    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetAnsModeInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_ANSWER_MODE_INFO_CNF_STRU   *pstSetAnsModeInfoCnf = VOS_NULL_PTR;

    pstSetAnsModeInfoCnf = (TAF_PS_SET_ANSWER_MODE_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGAUTO_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetAnsModeInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetAnsModeInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT16                              usLength;
    TAF_PS_GET_ANSWER_MODE_INFO_CNF_STRU   *pstCallAns = VOS_NULL_PTR;

    /* ��ʼ�� */
    usLength    = 0;
    pstCallAns  = (TAF_PS_GET_ANSWER_MODE_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGAUTO_READ )
    {
        return VOS_ERR;
    }

    /* +CGAUTO */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",pstCallAns->ulAnsMode);

    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDynamicPrimPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult = AT_FAILURE;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    VOS_UINT8                           aucStr[TAF_MAX_APN_LEN + 1];

    TAF_PDP_DYNAMIC_PRIM_EXT_STRU       stCgdcont;
    TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF_STRU  *pstGetDynamicPdpCtxInfoCnf = VOS_NULL_PTR;

    memset_s(acIpv4StrTmp,  sizeof(acIpv4StrTmp), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, AT_IPV6_ADDR_MASK_FORMAT_STR_LEN);
    memset_s(&stCgdcont,    sizeof(stCgdcont), 0x00, sizeof(TAF_PDP_DYNAMIC_PRIM_EXT_STRU));
    pstGetDynamicPdpCtxInfoCnf = (TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGCONTRDP_SET )
    {
        return VOS_ERR;
    }

    if( pstGetDynamicPdpCtxInfoCnf->enCause == VOS_OK )
    {
        for(ucTmp = 0; ucTmp < pstGetDynamicPdpCtxInfoCnf->ulCidNum; ucTmp++)
        {
            if(ucTmp != 0)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
            }

            lMemResult = memcpy_s(&stCgdcont, sizeof(stCgdcont), &pstGetDynamicPdpCtxInfoCnf->astPdpContxtInfo[ucTmp], sizeof(TAF_PDP_DYNAMIC_PRIM_EXT_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgdcont), sizeof(TAF_PDP_DYNAMIC_PRIM_EXT_STRU));

            /* +CGCONTRDP:  */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

            /* <p_cid> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgdcont.ucPrimayCid);

            /* <bearer_id> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.ucBearerId);

            /* <APN> */
            if(stCgdcont.bitOpApn == 1)
            {
                memset_s(aucStr, sizeof(aucStr), 0x00, sizeof(aucStr));
                lMemResult = memcpy_s(aucStr, sizeof(aucStr), stCgdcont.aucApn, TAF_MAX_APN_LEN);
                TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucStr), TAF_MAX_APN_LEN);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"",stCgdcont.aucApn);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            }

            /* <ip_addr> */
            AT_ConvertPdpContextIpAddrParaToString(&stCgdcont, &usLength);

            /* <gw_addr> */
            AT_ConvertPdpContextAddrParaToString(stCgdcont.bitOpGwAddr, stCgdcont.stGWAddr.enPdpType, stCgdcont.stGWAddr.aucIpv4Addr, stCgdcont.stGWAddr.aucIpv6Addr, &usLength);

            /* <DNS_prim_addr> */
            AT_ConvertPdpContextAddrParaToString(stCgdcont.bitOpDNSPrimAddr, stCgdcont.stDNSPrimAddr.enPdpType, stCgdcont.stDNSPrimAddr.aucIpv4Addr, stCgdcont.stDNSPrimAddr.aucIpv6Addr, &usLength);

            /* <DNS_sec_addr> */
            AT_ConvertPdpContextAddrParaToString(stCgdcont.bitOpDNSSecAddr, stCgdcont.stDNSSecAddr.enPdpType, stCgdcont.stDNSSecAddr.aucIpv4Addr, stCgdcont.stDNSSecAddr.aucIpv6Addr, &usLength);

            /* <P-CSCF_prim_addr> */
            AT_ConvertPdpContextAddrParaToString(stCgdcont.bitOpPCSCFPrimAddr, stCgdcont.stPCSCFPrimAddr.enPdpType, stCgdcont.stPCSCFPrimAddr.aucIpv4Addr, stCgdcont.stPCSCFPrimAddr.aucIpv6Addr, &usLength);

            /* <P-CSCF_sec_addr> */
            AT_ConvertPdpContextAddrParaToString(stCgdcont.bitOpPCSCFSecAddr, stCgdcont.stPCSCFSecAddr.enPdpType, stCgdcont.stPCSCFSecAddr.aucIpv4Addr, stCgdcont.stPCSCFSecAddr.aucIpv6Addr, &usLength);

            /* <im-cn-signal-flag> */
            AT_ConvertULParaToString(stCgdcont.bitOpImCnSignalFlg, stCgdcont.enImCnSignalFlg, &usLength);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

            /* <LIPA indication omit> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");

            /* <ipv4 mtu> */
            AT_ConvertULParaToString(stCgdcont.bitOpIpv4Mtu, stCgdcont.usIpv4Mtu, &usLength);

            /* the following six parameters omit: <wlan offload>, <local addr ind>, <Non-Ip mtu>, <Serving plmn rate control value>, <Reliable data service> and <Ps data off support> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",,,,,,");

            /* <pdu session id> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.ucPduSessionId);

            /* <qfi> */
            AT_ConvertULParaToString(stCgdcont.bitOpQfi, stCgdcont.ucQfi, &usLength);

            /* <ssc mode> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.enSscMode);

            AT_ConvertSNssaiToString(&stCgdcont.stSNssai, &usLength);

            /* <access type> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.enAccessType);

            /* <reflect qos timer value> */
            AT_ConvertULParaToString(stCgdcont.bitOpRqTimer, stCgdcont.ulRqTimer, &usLength);

            /* <AlwaysOnInd> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdcont.enAlwaysOnInd);

#endif
        }
        ulResult                = AT_OK;
        gstAtSendData.usBufLen  = usLength;

    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDynamicSecPdpContextInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult = AT_FAILURE;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;

    TAF_PDP_DYNAMIC_SEC_EXT_STRU       stCgdscont;
    TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_CNF_STRU  *pstGetDynamicPdpCtxInfoCnf = VOS_NULL_PTR;

    memset_s(&stCgdscont, sizeof(stCgdscont), 0x00, sizeof(TAF_PDP_DYNAMIC_SEC_EXT_STRU));
    pstGetDynamicPdpCtxInfoCnf = (TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGSCONTRDP_SET )
    {
        return VOS_ERR;
    }

    if( pstGetDynamicPdpCtxInfoCnf->enCause == VOS_OK )
    {
        for(ucTmp = 0; ucTmp < pstGetDynamicPdpCtxInfoCnf->ulCidNum; ucTmp++)
        {
            if(ucTmp != 0)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
            }

            lMemResult = memcpy_s(&stCgdscont, sizeof(stCgdscont), &pstGetDynamicPdpCtxInfoCnf->astPdpContxtInfo[ucTmp], sizeof(TAF_PDP_DYNAMIC_SEC_EXT_STRU));
            TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgdscont), sizeof(TAF_PDP_DYNAMIC_SEC_EXT_STRU));

            /* +CGSCONTRDP:  */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            /* <cid> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgdscont.ucCid);
            /* <p_cid> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdscont.ucPrimaryCid);
            /* <bearer_id> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdscont.ucBearerId);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            /* the following two parameters omit: im_cn_signalling_flag and wlan offload */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",,");
            /* pdu_session_id */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdscont.ucPduSessionId);
            /* qfi */
            if (stCgdscont.bitOpQfi == VOS_TRUE)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgdscont.ucQfi);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            }
#endif
        }

        ulResult                = AT_OK;
        gstAtSendData.usBufLen  = usLength;
    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDynamicTftInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult = AT_FAILURE;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucIndex1 = 0;
    VOS_UINT8                           ucIndex2 = 0;
    VOS_CHAR                            acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR                            acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];
    VOS_CHAR                            aucLocalIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_UINT8                           aucLocalIpv6Mask[APP_MAX_IPV6_ADDR_LEN];

    TAF_PF_TFT_STRU                       *pstCgtft                 = VOS_NULL_PTR;
    TAF_PS_GET_DYNAMIC_TFT_INFO_CNF_STRU  *pstGetDynamicTftInfoCnf  = VOS_NULL_PTR;

    memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, sizeof(acIpv4StrTmp));
    memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, sizeof(acIpv6StrTmp));
    memset_s(aucLocalIpv4StrTmp, sizeof(aucLocalIpv4StrTmp), 0x00, sizeof(aucLocalIpv4StrTmp));
    memset_s(aucLocalIpv6Mask, sizeof(aucLocalIpv6Mask), 0x00, sizeof(aucLocalIpv6Mask));

    pstGetDynamicTftInfoCnf = (TAF_PS_GET_DYNAMIC_TFT_INFO_CNF_STRU*)pEvtInfo;

    pstCgtft = (TAF_PF_TFT_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_PF_TFT_STRU));
    if (pstCgtft == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }
    memset_s(pstCgtft, sizeof(TAF_PF_TFT_STRU), 0x00, sizeof(TAF_PF_TFT_STRU));

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGTFTRDP_SET )
    {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, pstCgtft);
        /*lint -restore */
        return VOS_ERR;
    }

    if ( pstGetDynamicTftInfoCnf->enCause == VOS_OK)
    {
        for (ucIndex1 = 0; ucIndex1 < pstGetDynamicTftInfoCnf->ulCidNum; ucIndex1++)
        {
            for (ucIndex2 = 0; ucIndex2 < pstGetDynamicTftInfoCnf->astPfTftInfo[ucIndex1].ulPFNum; ucIndex2++)
            {
                if (!(ucIndex1 == 0 && ucIndex2 == 0))
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
                }

                lMemResult = memcpy_s(pstCgtft, sizeof(TAF_PF_TFT_STRU), &pstGetDynamicTftInfoCnf->astPfTftInfo[ucIndex1], sizeof(TAF_PF_TFT_STRU));
                TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(TAF_PF_TFT_STRU), sizeof(TAF_PF_TFT_STRU));

                /* +CGTFTRDP:  */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                /* <cid> */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",pstCgtft->ulCid);
                /* <packet filter identifier> */
                AT_ConvertULParaToString(pstCgtft->astTftInfo[ucIndex2].bitOpPktFilterId, pstCgtft->astTftInfo[ucIndex2].ucPacketFilterId, &usLength);

                /* <evaluation precedence index> */
                AT_ConvertULParaToString(pstCgtft->astTftInfo[ucIndex2].bitOpPrecedence, pstCgtft->astTftInfo[ucIndex2].ucPrecedence, &usLength);

                /* <source address and subnet> */
                AT_ConvertTftSrcAddrParaToString(&pstCgtft->astTftInfo[ucIndex2], &usLength);

                /* <protocal number(ipv4)/next header ipv6> */
                AT_ConvertULParaToString(pstCgtft->astTftInfo[ucIndex2].bitOpProtocolId, pstCgtft->astTftInfo[ucIndex2].ucProtocolId, &usLength);

                /* <destination port range> */
                AT_ConvertRangeParaToString(VOS_FALSE,
                                            pstCgtft->astTftInfo[ucIndex2].bitOpDestPortRange,
                                            pstCgtft->astTftInfo[ucIndex2].usLowDestPort,
                                            pstCgtft->astTftInfo[ucIndex2].usHighDestPort,
                                            &usLength);

                /* <source port range> */
                AT_ConvertRangeParaToString(VOS_FALSE,
                                            pstCgtft->astTftInfo[ucIndex2].bitOpSrcPortRange,
                                            pstCgtft->astTftInfo[ucIndex2].usLowSourcePort,
                                            pstCgtft->astTftInfo[ucIndex2].usHighSourcePort,
                                            &usLength);

                /* <ipsec security parameter index(spi)> */
                AT_ConvertULParaToXString(pstCgtft->astTftInfo[ucIndex2].bitOpSpi, pstCgtft->astTftInfo[ucIndex2].ulSecuParaIndex, &usLength);

                /* <type os service(tos) (ipv4) and mask> */
                AT_ConvertRangeParaToString(VOS_FALSE,
                                            pstCgtft->astTftInfo[ucIndex2].bitOpTosMask,
                                            pstCgtft->astTftInfo[ucIndex2].ucTypeOfService,
                                            pstCgtft->astTftInfo[ucIndex2].ucTypeOfServiceMask,
                                            &usLength);

                /* <traffic class (ipv6) and mask> */

                /* <flow lable (ipv6)> */
                AT_ConvertULParaToXString(pstCgtft->astTftInfo[ucIndex2].bitOpFlowLable, pstCgtft->astTftInfo[ucIndex2].ulFlowLable, &usLength);

                /* <direction> */
                AT_ConvertULParaToString(pstCgtft->astTftInfo[ucIndex2].bitOpDirection, pstCgtft->astTftInfo[ucIndex2].ucDirection, &usLength);

                /* <NW packet filter Identifier> */
                AT_ConvertULParaToString(pstCgtft->astTftInfo[ucIndex2].bitOpNwPktFilterId, pstCgtft->astTftInfo[ucIndex2].ucNwPktFilterId, &usLength);

                if (AT_IsSupportReleaseRst(AT_ACCESS_STRATUM_REL11))
                {
                    /* <local address and subnet> */
                    if ( pstCgtft->astTftInfo[ucIndex2].bitOpLocalIpv4AddrAndMask == 1 )
                    {
                        AT_Ipv4AddrItoa(aucLocalIpv4StrTmp, sizeof(aucLocalIpv4StrTmp), pstCgtft->astTftInfo[ucIndex2].aucLocalIpv4Addr);
                        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s",aucLocalIpv4StrTmp);

                        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,".");

                        AT_Ipv4AddrItoa(aucLocalIpv4StrTmp, sizeof(aucLocalIpv4StrTmp), pstCgtft->astTftInfo[ucIndex2].aucLocalIpv4Mask);
                        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s\"",aucLocalIpv4StrTmp);
                    }
                    else if ( pstCgtft->astTftInfo[ucIndex2].bitOpLocalIpv6AddrAndMask == 1 )
                    {
                        AT_GetIpv6MaskByPrefixLength(pstCgtft->astTftInfo[ucIndex2].ucLocalIpv6Prefix, aucLocalIpv6Mask);
                        (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp,
                                                               pstCgtft->astTftInfo[ucIndex2].aucLocalIpv6Addr,
                                                               aucLocalIpv6Mask);
                        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",\"%s\"", acIpv6StrTmp);
                    }
                    else
                    {
                        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
                    }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
                    /* <qri> */
                    AT_ConvertULParaToString(pstCgtft->astTftInfo[ucIndex2].bitOpQri, pstCgtft->astTftInfo[ucIndex2].ucQri, &usLength);
#endif
                }

            }

            /* <3,0,0,"192.168.0.2.255.255.255.0">,0,"0.65535","0.65535",0,"0.0",0,0 */
        }

        ulResult                = AT_OK;
        gstAtSendData.usBufLen  = usLength;
    }
    else
    {
        ulResult                = AT_ERROR;
       gstAtSendData.usBufLen   = 0;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, pstCgtft);
    /*lint -restore */
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetEpsQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{

    TAF_PS_SET_EPS_QOS_INFO_CNF_STRU  *pstSetEpsqosInfoCnf = VOS_NULL_PTR;

    pstSetEpsqosInfoCnf = (TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQOS_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetEpsqosInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetEpsQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;

    TAF_EPS_QOS_EXT_STRU                stCgeqos;
    TAF_PS_GET_EPS_QOS_INFO_CNF_STRU   *pstGetEpsQosInfoCnf = VOS_NULL_PTR;

    memset_s(&stCgeqos, sizeof(stCgeqos), 0x00, sizeof(TAF_EPS_QOS_EXT_STRU));

    pstGetEpsQosInfoCnf = (TAF_PS_GET_EPS_QOS_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQOS_READ )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstGetEpsQosInfoCnf->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgeqos, sizeof(stCgeqos), &pstGetEpsQosInfoCnf->astEpsQosInfo[ucTmp], sizeof(TAF_EPS_QOS_EXT_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgeqos), sizeof(TAF_EPS_QOS_EXT_STRU));

        /* +CGEQOS:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgeqos.ucCid);
        /* <QCI> */
        if(stCgeqos.bitOpQCI == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ucQCI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <DL GBR> */
        if(stCgeqos.bitOpDLGBR == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulDLGBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <UL GBR> */
        if(stCgeqos.bitOpULGBR == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulULGBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <DL MBR> */
        if(stCgeqos.bitOpDLMBR == 1)
        {
           usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulDLMBR);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <UL MBR> */
        if(stCgeqos.bitOpULMBR == 1)
        {
           usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulULMBR);
        }
    }

    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}

VOS_VOID AT_PrintDynamicEpsQosInfo(
    TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_CNF_STRU  *pstGetDynamicEpsQosInfoCnf,
    VOS_UINT8                                  ucIndex
)
{
    errno_t                             lMemResult;
    TAF_EPS_QOS_EXT_STRU                stCgeqos;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp    = 0;

    memset_s(&stCgeqos, sizeof(stCgeqos), 0x00, sizeof(TAF_EPS_QOS_EXT_STRU));

    for(ucTmp = 0; ucTmp < pstGetDynamicEpsQosInfoCnf->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stCgeqos, sizeof(stCgeqos), &pstGetDynamicEpsQosInfoCnf->astEpsQosInfo[ucTmp], sizeof(TAF_EPS_QOS_EXT_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stCgeqos), sizeof(TAF_EPS_QOS_EXT_STRU));

        /* +CGEQOSRDP:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stCgeqos.ucCid);
        /* <QCI> */
        if(stCgeqos.bitOpQCI == 1)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ucQCI);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        if (stCgeqos.ucQCI <= 4)
        {
            /* <DL GBR> */
            if(stCgeqos.bitOpDLGBR == 1)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulDLGBR);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            }
            /* <UL GBR> */
            if(stCgeqos.bitOpULGBR == 1)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulULGBR);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            }
            /* <DL MBR> */
            if(stCgeqos.bitOpDLMBR == 1)
            {
               usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulDLMBR);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
            }
            /* <UL MBR> */
            if(stCgeqos.bitOpULMBR == 1)
            {
               usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stCgeqos.ulULMBR);
            }
        }
    }

    gstAtSendData.usBufLen  = usLength;

    return;
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicEpsQosInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_CNF_STRU  *pstGetDynamicEpsQosInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                                 ulResult                   = AT_FAILURE;

    pstGetDynamicEpsQosInfoCnf = (TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGEQOSRDP_SET )
    {
        return VOS_ERR;
    }

    if(pstGetDynamicEpsQosInfoCnf->enCause == VOS_OK)
    {
        AT_PrintDynamicEpsQosInfo(pstGetDynamicEpsQosInfoCnf, ucIndex);
        ulResult                = AT_OK;
    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDsFlowInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_DSFLOW_INFO_CNF_STRU    *pstEvtMsg    = VOS_NULL_PTR;
    TAF_DSFLOW_QUERY_INFO_STRU         *pstQueryInfo = VOS_NULL_PTR;
    VOS_UINT16                          usLength     = 0;

    pstEvtMsg    = (TAF_PS_GET_DSFLOW_INFO_CNF_STRU *)pEvtInfo;
    pstQueryInfo = &pstEvtMsg->stQueryInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DSFLOWQRY_SET)
    {
        return VOS_ERR;
    }

    /* �ϱ�������ѯ��� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%08X,%08X%08X,%08X%08X",
                                       pstQueryInfo->stCurrentFlowInfo.ulLinkTime,
                                       pstQueryInfo->stCurrentFlowInfo.aulSendFlux[1],
                                       pstQueryInfo->stCurrentFlowInfo.aulSendFlux[0],
                                       pstQueryInfo->stCurrentFlowInfo.aulRecvFlux[1],
                                       pstQueryInfo->stCurrentFlowInfo.aulRecvFlux[0]);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%08X,%08X%08X,%08X%08X",
                                       pstQueryInfo->stTotalFlowInfo.ulLinkTime,
                                       pstQueryInfo->stTotalFlowInfo.aulSendFlux[1],
                                       pstQueryInfo->stTotalFlowInfo.aulSendFlux[0],
                                       pstQueryInfo->stTotalFlowInfo.aulRecvFlux[1],
                                       pstQueryInfo->stTotalFlowInfo.aulRecvFlux[0]);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtClearDsFlowInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CLEAR_DSFLOW_CNF_STRU       *pstEvtMsg = VOS_NULL_PTR;

    pstEvtMsg = (TAF_PS_CLEAR_DSFLOW_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DSFLOWCLR_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstEvtMsg->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtConfigDsFlowRptCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CONFIG_DSFLOW_RPT_CNF_STRU  *pstEvtMsg = VOS_NULL_PTR;

    pstEvtMsg = (TAF_PS_CONFIG_DSFLOW_RPT_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DSFLOWRPT_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstEvtMsg->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtConfigVTFlowRptCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CONFIG_VTFLOW_RPT_CNF_STRU  *pstEvtMsg = VOS_NULL_PTR;

    pstEvtMsg = (TAF_PS_CONFIG_VTFLOW_RPT_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_VTFLOWRPT_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstEvtMsg->enCause);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtReportDsFlowInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_REPORT_DSFLOW_IND_STRU      *pstEvtMsg  = VOS_NULL_PTR;
    TAF_DSFLOW_REPORT_STRU             *pstRptInfo = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId  = MODEM_ID_0;
    VOS_UINT16                          usLength   = 0;

    pstEvtMsg  = (TAF_PS_REPORT_DSFLOW_IND_STRU *)pEvtInfo;
    pstRptInfo = &pstEvtMsg->stReportInfo;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsEvtReportDsFlowInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* �ϱ�������Ϣ */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^DSFLOWRPT: %08X,%08X,%08X,%08X%08X,%08X%08X,%08X,%08X%s",
                                       gaucAtCrLf,
                                       pstRptInfo->stFlowInfo.ulLinkTime,
                                       pstRptInfo->ulSendRate,
                                       pstRptInfo->ulRecvRate,
                                       pstRptInfo->stFlowInfo.aulSendFlux[1],
                                       pstRptInfo->stFlowInfo.aulSendFlux[0],
                                       pstRptInfo->stFlowInfo.aulRecvFlux[1],
                                       pstRptInfo->stFlowInfo.aulRecvFlux[0],
                                       pstRptInfo->ulQosSendRate,
                                       pstRptInfo->ulQosRecvRate,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtReportVTFlowInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_REPORT_VTFLOW_IND_STRU      *pstEvtMsg  = VOS_NULL_PTR;
    TAF_VTFLOW_REPORT_STRU             *pstRptInfo = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId  = MODEM_ID_0;
    VOS_UINT16                          usLength   = 0;

    pstEvtMsg  = (TAF_PS_REPORT_VTFLOW_IND_STRU *)pEvtInfo;
    pstRptInfo  = &pstEvtMsg->stReportInfo;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsEvtReportVTFlowInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* �ϱ���Ƶ������Ϣ */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%s%s%08X,%08X%08X,%08X%08X%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_VT_FLOW_RPT].pucText,
                                       pstRptInfo->ulLinkTime,
                                       pstRptInfo->aulSendFlux[1],
                                       pstRptInfo->aulSendFlux[0],
                                       pstRptInfo->aulRecvFlux[1],
                                       pstRptInfo->aulRecvFlux[0],
                                       gaucAtCrLf);

    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetApDsFlowRptCfgCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_APDSFLOW_RPT_CFG_CNF_STRU *pstEvtMsg = VOS_NULL_PTR;

    pstEvtMsg = (TAF_PS_SET_APDSFLOW_RPT_CFG_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_APDSFLOWRPTCFG_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstEvtMsg->enCause);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetApDsFlowRptCfgCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_APDSFLOW_RPT_CFG_CNF_STRU   *pstEvtMsg = VOS_NULL_PTR;
    VOS_UINT16                              usLength;

    pstEvtMsg = (TAF_PS_GET_APDSFLOW_RPT_CFG_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_APDSFLOWRPTCFG_QRY)
    {
        return VOS_ERR;
    }

    /* �������� */
    if (pstEvtMsg->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    /* �ϱ���ѯ��� */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%u",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstEvtMsg->stRptCfg.ulRptEnabled,
                                      pstEvtMsg->stRptCfg.ulFluxThreshold);

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtApDsFlowReportInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_APDSFLOW_REPORT_IND_STRU    *pstEvtMsg  = VOS_NULL_PTR;
    TAF_APDSFLOW_REPORT_STRU           *pstRptInfo = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                enModemId  = MODEM_ID_0;
    VOS_UINT16                          usLength;

    pstEvtMsg  = (TAF_PS_APDSFLOW_REPORT_IND_STRU *)pEvtInfo;
    pstRptInfo = &pstEvtMsg->stReportInfo;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsEvtApDsFlowReportInd: Get modem id fail.");
        return VOS_ERR;
    }

    /*
     * ^APDSFLOWRPT: <curr_ds_time>,<tx_rate>,<rx_rate>,
     *               <curr_tx_flow>,<curr_rx_flow>,<total_tx_flow>,<total_rx_flow>
     */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      "%s^APDSFLOWRPT: %08X,%08X,%08X,%08X%08X,%08X%08X,%08X%08X,%08X%08X%s",
                                      gaucAtCrLf,
                                      pstRptInfo->stCurrentFlowInfo.ulLinkTime,
                                      pstRptInfo->ulSendRate,
                                      pstRptInfo->ulRecvRate,
                                      pstRptInfo->stCurrentFlowInfo.aulSendFlux[1],
                                      pstRptInfo->stCurrentFlowInfo.aulSendFlux[0],
                                      pstRptInfo->stCurrentFlowInfo.aulRecvFlux[1],
                                      pstRptInfo->stCurrentFlowInfo.aulRecvFlux[0],
                                      pstRptInfo->stTotalFlowInfo.aulSendFlux[1],
                                      pstRptInfo->stTotalFlowInfo.aulSendFlux[0],
                                      pstRptInfo->stTotalFlowInfo.aulRecvFlux[1],
                                      pstRptInfo->stTotalFlowInfo.aulRecvFlux[0],
                                      gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetDsFlowNvWriteCfgCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_DSFLOW_NV_WRITE_CFG_CNF_STRU    *pstSetNvWriteCfgCnf = VOS_NULL_PTR;

    pstSetNvWriteCfgCnf = (TAF_PS_SET_DSFLOW_NV_WRITE_CFG_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DSFLOWNVWRCFG_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetNvWriteCfgCnf->enCause);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDsFlowNvWriteCfgCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_DSFLOW_NV_WRITE_CFG_CNF_STRU    *pstGetNvWriteCfgCnf = VOS_NULL_PTR;
    VOS_UINT16                                  usLength;

    pstGetNvWriteCfgCnf = (TAF_PS_GET_DSFLOW_NV_WRITE_CFG_CNF_STRU *)pEvtInfo;
    usLength            = 0;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DSFLOWNVWRCFG_QRY)
    {
        return VOS_ERR;
    }

    /* �������� */
    if (pstGetNvWriteCfgCnf->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    /* �ϱ���ѯ��� */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstGetNvWriteCfgCnf->stNvWriteCfg.ucEnabled,
                                      pstGetNvWriteCfgCnf->stNvWriteCfg.ucInterval);

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_RcvTafPsEvtSetPktCdataInactivityTimeLenCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_CTA_INFO_CNF_STRU       *pstSetPktCdataInactivityTimeLenCnf = VOS_NULL_PTR;

    pstSetPktCdataInactivityTimeLenCnf = (TAF_PS_SET_CTA_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CTA_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetPktCdataInactivityTimeLenCnf->ulRslt);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetPktCdataInactivityTimeLenCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT16                          usLength;
    TAF_PS_GET_CTA_INFO_CNF_STRU       *pstGetPktCdataInactivityTimeLenCnf = VOS_NULL_PTR;

    pstGetPktCdataInactivityTimeLenCnf = (TAF_PS_GET_CTA_INFO_CNF_STRU *)pEvtInfo;
    usLength                           = 0;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CTA_QRY)
    {
        return VOS_ERR;
    }

    /* �������� */
    if (pstGetPktCdataInactivityTimeLenCnf->ulRslt != TAF_PS_CAUSE_SUCCESS)
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    /* �ϱ���ѯ��� */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstGetPktCdataInactivityTimeLenCnf->ucPktCdataInactivityTmrLen);

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;

}


VOS_UINT32 AT_RcvTafPsEvtCgmtuValueChgInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CGMTU_VALUE_CHG_IND_STRU    *pstCgmtuChgInd = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;

    pstCgmtuChgInd = (TAF_PS_CGMTU_VALUE_CHG_IND_STRU *)pEvtInfo;
    usLength       = 0;
    enModemId      = MODEM_ID_0;

    if (AT_GetModemIdFromClient(ucIndex, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsEvtCgmtuValueChgInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* ^CGMTU: <curr_mtu_value> */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                      "%s^CGMTU: %d%s",
                                      gaucAtCrLf,
                                      pstCgmtuChgInd->ulMtuValue,
                                      gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}

#endif

VOS_UINT32 AT_RcvTafPsEvtSetPdpDnsInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_PDP_DNS_INFO_CNF_STRU   *pstSetPdpDnsInfoCnf = VOS_NULL_PTR;

    pstSetPdpDnsInfoCnf = (TAF_PS_SET_PDP_DNS_INFO_CNF_STRU*)pEvtInfo;

    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDNS_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetPdpDnsInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetPdpDnsInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    /* ��ֲAt_QryParaRspCgdnsProc��ʵ���߼� */
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;
    TAF_DNS_QUERY_INFO_STRU             stPdpDns;
    TAF_PS_GET_PDP_DNS_INFO_CNF_STRU   *pstPdpDnsInfo = VOS_NULL_PTR;
    VOS_INT8                            acDnsAddr[TAF_MAX_IPV4_ADDR_STR_LEN];

    memset_s(&stPdpDns, sizeof(stPdpDns), 0x00, sizeof(TAF_DNS_QUERY_INFO_STRU));
    memset_s(acDnsAddr, sizeof(acDnsAddr), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    pstPdpDnsInfo = (TAF_PS_GET_PDP_DNS_INFO_CNF_STRU *)pEvtInfo;

    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CGDNS_READ )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstPdpDnsInfo->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stPdpDns, sizeof(stPdpDns), &pstPdpDnsInfo->astPdpDnsQueryInfo[ucTmp],sizeof(TAF_DNS_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stPdpDns), sizeof(TAF_DNS_QUERY_INFO_STRU));
        /* +CGDNS:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stPdpDns.ucCid);
        /* <PriDns> */
        if(stPdpDns.stDnsInfo.bitOpPrimDnsAddr == 1)
        {
            AT_Ipv4Addr2Str(acDnsAddr, sizeof(acDnsAddr), stPdpDns.stDnsInfo.aucPrimDnsAddr, TAF_IPV4_ADDR_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"",acDnsAddr);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
        /* <SecDns> */
        if(stPdpDns.stDnsInfo.bitOpSecDnsAddr == 1)
        {
             AT_Ipv4Addr2Str(acDnsAddr, sizeof(acDnsAddr), stPdpDns.stDnsInfo.aucSecDnsAddr, TAF_IPV4_ADDR_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"",acDnsAddr);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",");
        }
    }


    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetAuthDataInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_AUTHDATA_INFO_CNF_STRU  *pstSetAuthDataInfoCnf = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstSetAuthDataInfoCnf = (TAF_PS_SET_AUTHDATA_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_AUTHDATA_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetAuthDataInfoCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetAuthDataInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucTmp = 0;
    TAF_AUTHDATA_QUERY_INFO_STRU        stPdpAuthData;
    TAF_PS_GET_AUTHDATA_INFO_CNF_STRU  *pstPdpAuthData = VOS_NULL_PTR;

    memset_s(&stPdpAuthData, sizeof(stPdpAuthData), 0x00, sizeof(TAF_AUTHDATA_QUERY_INFO_STRU));

    pstPdpAuthData = (TAF_PS_GET_AUTHDATA_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_AUTHDATA_READ )
    {
        return VOS_ERR;
    }

    for(ucTmp = 0; ucTmp < pstPdpAuthData->ulCidNum; ucTmp++)
    {
        if(ucTmp != 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
        }

        lMemResult = memcpy_s(&stPdpAuthData, sizeof(stPdpAuthData), &pstPdpAuthData->astAuthDataQueryInfo[ucTmp], sizeof(TAF_AUTHDATA_QUERY_INFO_STRU));
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stPdpAuthData), sizeof(TAF_AUTHDATA_QUERY_INFO_STRU));
        /* ^AUTHDATA:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",stPdpAuthData.ucCid);

        /* <Auth_type> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d",stPdpAuthData.stAuthDataInfo.enAuthType);

        /* <passwd> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"",stPdpAuthData.stAuthDataInfo.aucPassword);

        /* <username> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"",stPdpAuthData.stAuthDataInfo.aucUsername);

        /* <PLMN> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"",stPdpAuthData.stAuthDataInfo.aucPlmn);
    }


    ulResult                = AT_OK;
    gstAtSendData.usBufLen  = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetGprsActiveTypeCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_D_GPRS_ACTIVE_TYPE_CNF_STRU *pstGetGprsActiveTypeCnf = VOS_NULL_PTR;

    /* ��ʼ�� */
    ulResult                = AT_FAILURE;
    pstGetGprsActiveTypeCnf = (TAF_PS_GET_D_GPRS_ACTIVE_TYPE_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_D_GPRS_SET )
    {
        return VOS_ERR;
    }

    /* ת���������ʽ */
    if ( pstGetGprsActiveTypeCnf->enCause == TAF_PARA_OK )
    {
        ulResult = At_SetDialGprsPara(ucIndex,
                        pstGetGprsActiveTypeCnf->stCidGprsActiveType.ucCid,
                        pstGetGprsActiveTypeCnf->stCidGprsActiveType.enActiveType);
    }
    else
    {
        ulResult = AT_ERROR;
    }

    if ( ulResult != AT_WAIT_ASYNC_RETURN )
    {
        if ( ulResult == AT_ERROR )
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
        }

        AT_StopRelTimer(ucIndex, &gastAtClientTab[ucIndex].hTimer);
        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_READY;
        gastAtClientTab[ucIndex].opId = 0;

        At_FormatResultData(ucIndex, ulResult);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtPppDialOrigCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                          ulResult;

    TAF_PS_PPP_DIAL_ORIG_CNF_STRU      *pstPppDialOrigCnf = VOS_NULL_PTR;

    /* ��ʼ�� */
    ulResult          = AT_FAILURE;
    pstPppDialOrigCnf = (TAF_PS_PPP_DIAL_ORIG_CNF_STRU*)pEvtInfo;

    /* MODEM���Ŵ����� */
    if ( (gastAtClientTab[ucIndex].UserType == AT_MODEM_USER)
      || (gastAtClientTab[ucIndex].UserType == AT_HSUART_USER))
    {
        if (pstPppDialOrigCnf->enCause != TAF_PS_CAUSE_SUCCESS)
        {
            /* ��¼PS����д����� */
            AT_PS_SetPsCallErrCause(ucIndex, pstPppDialOrigCnf->enCause);

            if ((gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_D_PPP_CALL_SET)
             || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_PPP_ORG_SET))
            {
                ulResult = AT_NO_CARRIER;

                PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);

                /* ��PPP����HDLCȥʹ�ܲ��� */
                PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

                /* ��������ģʽ */
                At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);

            }
            else if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_D_IP_CALL_SET)
            {
                if (pstPppDialOrigCnf->enCause == TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT)
                {
                    ulResult = AT_CME_PDP_ACT_LIMIT;
                }
                else
                {
                    ulResult = AT_ERROR;
                }

                PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

                /*��PPP����HDLCȥʹ�ܲ���*/
                PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);



            }
            else
            {
                ;
            }

            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, ulResult);
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetCidSdfInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    return VOS_OK;
}


VOS_BOOL AT_PH_IsPlmnValid(TAF_PLMN_ID_STRU *pstPlmnId)
{
    VOS_UINT32                          i;

    for (i=0; i<3; i++)
    {
        if ((((pstPlmnId->Mcc >> (i*4)) & 0x0F) > 9)
         || ((((pstPlmnId->Mnc >> (i*4)) & 0x0F) > 9) && (i != 2 ))
         || ((((pstPlmnId->Mnc >> (i*4)) & 0x0F) > 9) && (((pstPlmnId->Mnc >> (i*4)) & 0x0F) != 0x0F)))
        {
            /* PLMN ID��Ч */
            return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}


VOS_UINT32 AT_RcvTafPsEvtPdpDisconnectInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                          ulResult;

    VOS_UINT16                          usLength;

    usLength = 0;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        /* δӦ��ĳ���ֱ���ϱ�NO CARRIER*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%s",gaucAtCrLf);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s","NO CARRIER");

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

        return VOS_OK;
    }

    /* IP�������缤��^CGANSӦ��������ϱ�ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_PPP_ORG_SET)
    {
        ulResult = AT_NO_CARRIER;

        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_REL_PPP_REQ);

        /* ��PPP����HDLCȥʹ�ܲ��� */
        PPP_RcvAtCtrlOperEvent(gastAtClientTab[ucIndex].usPppId, PPP_AT_CTRL_HDLC_DISABLE);

        /* ��������ģʽ */
        At_SetMode(ucIndex, AT_CMD_MODE, AT_NORMAL_MODE);




    }
    else if ((gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CGANS_ANS_EXT_SET)
          || (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CGANS_ANS_SET))
    {
        /*
        1.PPP�������缤��^CGANSӦ��������ϱ�ID_EVT_TAF_PS_CALL_PDP_DISCONNECT_IND
        2.+CGANSӦ��
        ���������������û��������ͨ����ֱ�ӻ�ERROR
        */
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



VOS_VOID AT_SetMemStatusRspProc(
    VOS_UINT8                           ucIndex,
    MN_MSG_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT32                          ulResult;

    /* ����û�����ֵ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_SetMemStatusRspProc: AT_BROADCAST_INDEX.");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CSASM_SET */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSASM_SET )
    {
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if ( pstEvent->u.stMemStatusInfo.bSuccess == VOS_TRUE)
    {
        ulResult    = AT_OK;
    }
    else
    {
        ulResult    = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;
    /* ����AT_FormATResultDATa���������� */
    At_FormatResultData(ucIndex, ulResult);

    return;
}


VOS_UINT32 AT_RcvTafPsEvtGetDynamicDnsInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                          ulResult    = AT_ERROR;
    VOS_UINT16                          usLength    = 0;
    TAF_PS_GET_NEGOTIATION_DNS_CNF_STRU *pstNegoDnsCnf = VOS_NULL_PTR;
    VOS_INT8                            acDnsAddr[TAF_MAX_IPV4_ADDR_STR_LEN];

    memset_s(acDnsAddr, sizeof(acDnsAddr), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);

    pstNegoDnsCnf = (TAF_PS_GET_NEGOTIATION_DNS_CNF_STRU *)pEvtInfo;

    /* ����û�����ֵ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafPsEvtGetDynamicDnsInfoCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DNSQUERY_SET )
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstNegoDnsCnf->enCause != TAF_PARA_OK)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: ",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /* <PriDns> */
        if(pstNegoDnsCnf->stNegotiationDns.stDnsInfo.bitOpPrimDnsAddr == VOS_TRUE)
        {
            AT_Ipv4Addr2Str(acDnsAddr, sizeof(acDnsAddr),
                pstNegoDnsCnf->stNegotiationDns.stDnsInfo.aucPrimDnsAddr, TAF_IPV4_ADDR_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "\"%s\"",
                                               acDnsAddr);
        }
        /* <SecDns> */
        if(pstNegoDnsCnf->stNegotiationDns.stDnsInfo.bitOpSecDnsAddr == VOS_TRUE)
        {
            AT_Ipv4Addr2Str(acDnsAddr, sizeof(acDnsAddr),
                pstNegoDnsCnf->stNegotiationDns.stDnsInfo.aucSecDnsAddr, TAF_IPV4_ADDR_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               ",\"%s\"",
                                               acDnsAddr);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               ",");
        }
        ulResult = AT_OK;
    }

    gstAtSendData.usBufLen  = usLength;
    /* ����AT_FormATResultDATa���������� */
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}



VOS_UINT32 At_RcvTafPsEvtSetDialModeCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CDATA_DIAL_MODE_CNF_STRU    *pstDialMode = VOS_NULL_PTR;

    /* ��ʼ�� */
    ulResult     = AT_OK;
    pstDialMode  = (TAF_PS_CDATA_DIAL_MODE_CNF_STRU *)pEvtInfo;

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CRM_SET)
    {
        AT_WARN_LOG("At_RcvTafPsEvtSetDialModeCnf : Current Option is not AT_CMD_CRM_SET.");
        return VOS_OK;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��ʽ������� */
    gstAtSendData.usBufLen = 0;

    if (pstDialMode->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        ulResult = AT_ERROR;
    }

    /* ������ */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}



#if(FEATURE_ON == FEATURE_LTE)

VOS_UINT32 atReadLtecsCnfProc(VOS_UINT8   ucIndex,VOS_VOID    *pEvtInfo)
{
    TAF_PS_LTECS_CNF_STRU *pLtecsReadCnf = NULL;
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulResult;

    pLtecsReadCnf = (TAF_PS_LTECS_CNF_STRU *)pEvtInfo;

    if(pLtecsReadCnf->enCause == VOS_OK)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,
        "^LTECS:");
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,
        "%d,%d",pLtecsReadCnf->stLteCs.ucSG,pLtecsReadCnf->stLteCs.ucIMS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,
         ",%d,%d",pLtecsReadCnf->stLteCs.ucCSFB,pLtecsReadCnf->stLteCs.ucVCC);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,
        ",%d",pLtecsReadCnf->stLteCs.ucVoLGA);

         ulResult                = AT_OK;
        gstAtSendData.usBufLen  = usLength;
    }
    else
    {
         ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }


    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);
    return VOS_OK;
}

VOS_UINT32 atReadCemodeCnfProc(VOS_UINT8   ucIndex,VOS_VOID    *pEvtInfo)
{
    TAF_PS_CEMODE_CNF_STRU *pCemodeReadCnf = NULL;
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulResult;




    pCemodeReadCnf = (TAF_PS_CEMODE_CNF_STRU *)pEvtInfo;

    if(pCemodeReadCnf->enCause == VOS_OK)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,
        "+CEMODE:");
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,
        "%d",pCemodeReadCnf->stCemode.enCurrentUeMode);

         ulResult                = AT_OK;
        gstAtSendData.usBufLen  = usLength;
    }
    else
    {
         ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }


    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);
    return VOS_OK;
}




/*****************************************************************************
 �� �� ��  : AT_RcvTafGetLteAttachInfoCnf
 ��������  : ID_MSG_TAF_PS_GET_LTE_ATTACH_INFO_CNF�¼�������
 �������  : VOS_UINT8                  ucIndex,
             VOS_VOID                  *pEvtInfo       - �¼�����, MN_PS_EVT_STRUȥ��EvtId
 �������  : ��
 �� �� ֵ  : VOS_UINT32

*****************************************************************************/
VOS_UINT32 AT_RcvTafGetLteAttachInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    errno_t                                       lMemResult;
    TAF_PS_GET_LTE_ATTACH_INFO_CNF_STRU          *pstGetLteAttachInfoCnf      = VOS_NULL_PTR;
    VOS_UINT16                                    usLength                    = 0;
    VOS_UINT32                                    ulResult                    = AT_ERROR;
    VOS_UINT8                                     aucStr[TAF_MAX_APN_LEN + 1] = {0};

    pstGetLteAttachInfoCnf            = (TAF_PS_GET_LTE_ATTACH_INFO_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_LTEATTACHINFO_QRY )
    {
        AT_WARN_LOG("AT_RcvTafGetLteAttachInfoCnf : Current Option is not AT_CMD_LTEATTACHINFO_QRY.");
        return VOS_ERR;
    }

    /*�����ȡ�ɹ��������Ϣ*/
    if (pstGetLteAttachInfoCnf->enCause == TAF_PS_CAUSE_SUCCESS)
    {
        /* ^LTEATTACHINFO:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                    (VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",
                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /* <PDP_type> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                    (VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",
                                    pstGetLteAttachInfoCnf->stLteAttQueryInfo.enPdpType);

        /* <APN> */
        pstGetLteAttachInfoCnf->stLteAttQueryInfo.stApn.ucLength = TAF_MIN(pstGetLteAttachInfoCnf->stLteAttQueryInfo.stApn.ucLength,
                                                                            sizeof(pstGetLteAttachInfoCnf->stLteAttQueryInfo.stApn.aucValue));

        lMemResult = memcpy_s(aucStr, sizeof(aucStr),
                              pstGetLteAttachInfoCnf->stLteAttQueryInfo.stApn.aucValue,
                              pstGetLteAttachInfoCnf->stLteAttQueryInfo.stApn.ucLength);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucStr), pstGetLteAttachInfoCnf->stLteAttQueryInfo.stApn.ucLength);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                    (VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"", aucStr);

        ulResult                    = AT_OK;
    }

    gstAtSendData.usBufLen          = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}
#endif



VOS_VOID AT_ConvertNasMncToBcdType(
    VOS_UINT32                          ulNasMnc,
    VOS_UINT32                         *pulMnc
)
{
    VOS_UINT32                          i;
    VOS_UINT8                           aucTmp[4];

    *pulMnc = 0;

    for (i = 0 ; i < 3 ; i++ )
    {
        aucTmp[i] = ulNasMnc & 0x0f;
        ulNasMnc         >>=  8;
    }

    if(aucTmp[2] == 0xf)
    {
        *pulMnc = ((VOS_UINT32)aucTmp[0] << 4)
                        |((VOS_UINT32)aucTmp[1] )
                        |0xf00;
    }
    else
    {
        *pulMnc = ((VOS_UINT32)aucTmp[0] << 8)
                    |((VOS_UINT32)aucTmp[1] << 4)
                    | aucTmp[2];
    }

}




VOS_VOID AT_ConvertNasMccToBcdType(
    VOS_UINT32                          ulNasMcc,
    VOS_UINT32                         *pulMcc
)
{
    VOS_UINT32                          i;
    VOS_UINT8                           aucTmp[4];

    *pulMcc = 0;

    for (i = 0; i < 3 ; i++ )
    {
        aucTmp[i]   = ulNasMcc & 0x0f;
        ulNasMcc  >>=  8;
    }

    *pulMcc = ((VOS_UINT32)aucTmp[0] << 8)
             |((VOS_UINT32)aucTmp[1] << 4)
             | aucTmp[2];

}


VOS_UINT32 AT_RcvTafCcmEccNumInd(VOS_VOID *pMsg)
{
    TAF_CCM_ECC_NUM_IND_STRU           *pstEccNumInfo = VOS_NULL_PTR;
    VOS_UINT8                           aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN*2)+1];
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulMcc;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));


    /* ��ȡ�ϱ��Ľ�����������Ϣ */
    pstEccNumInfo = (TAF_CCM_ECC_NUM_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEccNumInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvMnCallEccNumIndProc:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("At_RcvMnCallEccNumIndProc: Get modem id fail.");
        return VOS_ERR;
    }


    /* ��APP�����ϱ����������� */
    for (i = 0; i < pstEccNumInfo->stEccNumInd.ulEccNumCount; i++)
    {
        /* ��BCD��ת��ΪASCII�� */
        AT_BcdNumberToAscii(pstEccNumInfo->stEccNumInd.astCustomEccNumList[i].aucEccNum,
                            pstEccNumInfo->stEccNumInd.astCustomEccNumList[i].ucEccNumLen,
                            (VOS_CHAR*)aucAsciiNum);

        /* ��NAS��ʽ��MCCת��ΪBCD��ʽ */
        AT_ConvertNasMccToBcdType(pstEccNumInfo->stEccNumInd.astCustomEccNumList[i].ulMcc, &ulMcc);

        usLength = 0;
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           "%s^XLEMA:",
                                           gaucAtCrLf);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           "%d,%d,%s,%d,%d,",
                                           (i+1),
                                           pstEccNumInfo->stEccNumInd.ulEccNumCount,
                                           aucAsciiNum,
                                           pstEccNumInfo->stEccNumInd.astCustomEccNumList[i].ucCategory,
                                           pstEccNumInfo->stEccNumInd.astCustomEccNumList[i].ucValidSimPresent);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           "%x%x%x",
                                           (ulMcc & 0x0f00)>>8,
                                           (ulMcc & 0xf0)>>4,
                                           (ulMcc & 0x0f));

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           ",%d%s",
                                           pstEccNumInfo->stEccNumInd.astCustomEccNumList[i].ucAbnormalServiceFlg,
                                           gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

        memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));

    }

    return VOS_OK;
}




VOS_VOID AT_RcvMmaNsmStatusInd(
    TAF_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU          *pEvent
)
{
    VOS_UINT16                          usLength;

    usLength = 0;
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s+PACSP",gaucAtCrLf);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%d",pEvent->ucPlmnMode);

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                     "%s",gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr,usLength);

    return;
}


LOCAL VOS_VOID AT_RcvMmaGsmCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
)
{
    VOS_UINT16                          usLength;
    usLength = 0;

    if (ucSystemAppConfig != SYSTEM_APP_ANDROID)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                          gaucAtCrLf,
                                          gastAtStringTab[AT_STRING_CERSSI].pucText,
                                          pstRssiInfoInd->stRssiInfo.aRssi[0].u.stGCellSignInfo.sRssiValue,
                                          0,
                                          255,
                                          0,
                                          0,
                                          0,
                                          0,0x7f7f,0x7f7f,
                                          0,99,99,99,99,99,99,99,99,
                                          0,
                                          0,
                                          0,
                                          gaucAtCrLf);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                          gaucAtCrLf,
                                          gastAtStringTab[AT_STRING_CERSSI].pucText,
                                          pstRssiInfoInd->stRssiInfo.aRssi[0].u.stGCellSignInfo.sRssiValue,
                                          0,
                                          255,
                                          0,
                                          0,
                                          0,
                                          0,0x7f7f,0x7f7f,
                                          0,
                                          0,
                                          0,
                                          gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
}


LOCAL VOS_VOID AT_RcvMmaWcdmaCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
)
{
    VOS_UINT16                          usLength;
    usLength = 0;

    if ( pstRssiInfoInd->stRssiInfo.ucCurrentUtranMode == TAF_UTRANCTRL_UTRAN_MODE_FDD)
    {
        if (ucSystemAppConfig != SYSTEM_APP_ANDROID)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CERSSI].pucText,
                                               0,      /* rssi */
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sRscpValue,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sEcioValue,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               0,99,99,99,99,99,99,99,99,
                                               0,
                                               0,
                                               0,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CERSSI].pucText,
                                               0,      /* rssi */
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sRscpValue,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sEcioValue,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               0,
                                               0,
                                               0,
                                               gaucAtCrLf);
        }
    }
    else
    {
        if (ucSystemAppConfig != SYSTEM_APP_ANDROID)
        {
            /* ��fdd 3g С����ecioֵΪ��Чֵ255 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CERSSI].pucText,
                                               0,      /* rssi */
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sRscpValue,
                                               255,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               0,99,99,99,99,99,99,99,99,
                                               0,
                                               0,
                                               0,
                                               gaucAtCrLf);
        }
        else
        {
            /* ��fdd 3g С����ecioֵΪ��Чֵ255 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CERSSI].pucText,
                                               0,      /* rssi */
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stWCellSignInfo.sRscpValue,
                                               255,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               0,
                                               0,
                                               0,
                                               gaucAtCrLf);
        }

    }
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


#if (FEATURE_LTE == FEATURE_ON)

LOCAL VOS_VOID AT_RcvMmaLteCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
)
{
    VOS_UINT16                          usLength;
    usLength = 0;

    if (ucSystemAppConfig != SYSTEM_APP_ANDROID)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CERSSI].pucText,
                                           0,
                                           0,
                                           255,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRsrp,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRsrq,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.lSINR,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stCQI.usRI,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stCQI.ausCQI[0],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stCQI.ausCQI[1],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.ucRxANTNum,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.asRsrpRx[0],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.asRsrpRx[1],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.asRsrpRx[2],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.asRsrpRx[3],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.alSINRRx[0],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.alSINRRx[1],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.alSINRRx[2],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stRxAntInfo.alSINRRx[3],
                                           0,
                                           0,
                                           0,
                                           gaucAtCrLf);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CERSSI].pucText,
                                           0,
                                           0,
                                           255,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRsrp,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRsrq,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.lSINR,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stCQI.usRI,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stCQI.ausCQI[0],
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.stCQI.ausCQI[1],
                                           0,
                                           0,
                                           0,
                                           gaucAtCrLf);
    }
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

LOCAL VOS_VOID AT_RcvMmaNrCERssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd,
    VOS_UINT8                           ucSystemAppConfig
)
{
    VOS_UINT16                          usLength;
    usLength = 0;

    /* SA, use ^CERSSI to report */
    if (pstRssiInfoInd->ucIsNsaRptFlg == VOS_FALSE)
    {
        if (ucSystemAppConfig != SYSTEM_APP_ANDROID)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CERSSI].pucText,
                                               0,
                                               0,
                                               255,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               0,99,99,99,99,99,99,99,99,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrp,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrq,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.l5GSinr,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CERSSI].pucText,
                                               0,
                                               0,
                                               255,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrp,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrq,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.l5GSinr,
                                               gaucAtCrLf);
        }
        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    }

    /* NSA, use ^CSERSSI to report */
    else
    {
        if (ucSystemAppConfig != SYSTEM_APP_ANDROID)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CSERSSI].pucText,
                                               0,
                                               0,
                                               255,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               0,99,99,99,99,99,99,99,99,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrp,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrq,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.l5GSinr,
                                               gaucAtCrLf);
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                               gaucAtCrLf,
                                               gastAtStringTab[AT_STRING_CSERSSI].pucText,
                                               0,
                                               0,
                                               255,
                                               0,
                                               0,
                                               0,
                                               0,0x7f7f,0x7f7f,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrp,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.s5GRsrq,
                                               pstRssiInfoInd->stRssiInfo.aRssi[0].u.stNrCellSignInfo.l5GSinr,
                                               gaucAtCrLf);
        }

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
    }

    return;
}
#endif



VOS_VOID AT_RcvMmaRssiChangeInd(
    TAF_UINT8                           ucIndex,
    TAF_MMA_RSSI_INFO_IND_STRU         *pstRssiInfoInd
)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulRptCmdRssi;
    VOS_UINT32                          ulRptCmdCerssi;
#if (FEATURE_LTE == FEATURE_ON)
    VOS_UINT32                          ulRptCmdAnlevel;
    VOS_INT16                           sRsrp;
    VOS_INT16                           sRsrq;
    VOS_UINT8                           ucLevel;
    VOS_INT16                           sRssi;
#endif
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                          *pucSystemAppConfig = VOS_NULL_PTR;

    usLength       = 0;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvMmaRssiChangeInd: Get modem id fail.");
        return;
    }

    ulRptCmdRssi   = AT_CheckRptCmdStatus(pstRssiInfoInd->aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_RSSI);
    ulRptCmdCerssi = AT_CheckRptCmdStatus(pstRssiInfoInd->aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CERSSI);
    ulRptCmdAnlevel = AT_CheckRptCmdStatus(pstRssiInfoInd->aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_ANLEVEL);

    if ((AT_CheckRptCmdStatus(pstRssiInfoInd->aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_RSSI) == VOS_TRUE)
     && (ulRptCmdRssi == VOS_TRUE))
    {

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_RSSI].pucText,
                                           pstRssiInfoInd->stRssiInfo.aRssi[0].ucRssiValue,
                                           gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr,usLength);
    }

    if ((AT_CheckRptCmdStatus(pstRssiInfoInd->aucUnsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CERSSI) == VOS_TRUE)
     && (ulRptCmdCerssi == VOS_TRUE))
    {
        pucSystemAppConfig = AT_GetSystemAppConfigAddr();

        if (pstRssiInfoInd->stRssiInfo.enRatType == TAF_MMA_RAT_GSM)
        {
            AT_RcvMmaGsmCERssiChangeInd(ucIndex, pstRssiInfoInd, *pucSystemAppConfig);

            return;
        }

        if (pstRssiInfoInd->stRssiInfo.enRatType == TAF_MMA_RAT_WCDMA)
        {
            AT_RcvMmaWcdmaCERssiChangeInd(ucIndex, pstRssiInfoInd, *pucSystemAppConfig);

            return;
        }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (pstRssiInfoInd->stRssiInfo.enRatType == TAF_MMA_RAT_NR)
        {
            AT_RcvMmaNrCERssiChangeInd(ucIndex, pstRssiInfoInd, *pucSystemAppConfig);

            return;
        }
#endif

        /* �ϱ�LTE ��CERSSI */
#if (FEATURE_LTE == FEATURE_ON)
        if (pstRssiInfoInd->stRssiInfo.enRatType == TAF_MMA_RAT_LTE)
        {
            AT_RcvMmaLteCERssiChangeInd(ucIndex, pstRssiInfoInd, *pucSystemAppConfig);
        }
#endif
    }

    /*�ϱ�ANLEVEL */
#if (FEATURE_LTE == FEATURE_ON)

    usLength = 0;
    if (ulRptCmdAnlevel == VOS_TRUE)
    {


        sRsrp = pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRsrp;
        sRsrq = pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRsrq;
        sRssi = pstRssiInfoInd->stRssiInfo.aRssi[0].u.stLCellSignInfo.sRssi;

        AT_CalculateLTESignalValue(&sRssi,&ucLevel,&sRsrp,&sRsrq);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s0,99,%d,%d,%d,%d%s",
                                           gaucAtCrLf,
                                           "^ANLEVEL:",
                                           sRssi,
                                           ucLevel,
                                           sRsrp,
                                           sRsrq,
                                           gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr,usLength);
    }
#endif

    return;
}





VOS_VOID At_StkNumPrint(
    VOS_UINT16                         *pusLength,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usDataLen,
    VOS_UINT8                           ucNumType
)
{
    errno_t                             lMemResult;
    VOS_UINT16                          usLength = *pusLength;
    /* ��ӡ���� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                   ",\"");

    lMemResult = memcpy_s(pgucAtSndCodeAddr + usLength,  usDataLen, pucData, usDataLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, usDataLen, usDataLen);

    usLength += usDataLen;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                   "\"");

    /* ��ӡ���� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",%d",
                                           ucNumType);

    *pusLength = usLength;

    return;
}


VOS_VOID AT_BcdHalfByteToAscii(
    VOS_UINT8                           ucBcdHalfByte,
    VOS_UINT8                          *pucAsciiNum
)
{
    if(ucBcdHalfByte <= 9)  /*ת������*/
    {
        *pucAsciiNum = ucBcdHalfByte + 0x30;
    }
    else if(ucBcdHalfByte == 0x0A)   /*ת��*�ַ�*/
    {
        *pucAsciiNum = 0x2a;
    }
    else if(ucBcdHalfByte == 0x0B)   /*ת��#�ַ�*/
    {
        *pucAsciiNum = 0x23;
    }
    else if(ucBcdHalfByte == 0x0C)   /*ת��'P'�ַ�*/
    {
        *pucAsciiNum = 0x50;
    }
    else if(ucBcdHalfByte == 0x0D)   /*ת��'?'�ַ�*/
    {
        *pucAsciiNum = 0x3F;
    }
    else                                    /*ת����ĸ*/
    {
        *pucAsciiNum = ucBcdHalfByte + 0x57;
    }

    return;
}


VOS_VOID AT_BcdToAscii(
    VOS_UINT8                           ucBcdNumLen,
    VOS_UINT8                          *pucBcdNum,
    VOS_UINT8                          *pucAsciiNum,
    VOS_UINT8                           ucAsciiNumBufflen,
    VOS_UINT8                          *pucLen
)
{
    VOS_UINT8       ucTmp;
    VOS_UINT8       ucLen = 0;
    VOS_UINT8       ucFirstNumber;
    VOS_UINT8       ucSecondNumber;

    for (ucTmp = 0; ucTmp < ucBcdNumLen; ucTmp++)
    {
        if(pucBcdNum[ucTmp] == 0xFF)
        {
            break;
        }

        ucFirstNumber  = (VOS_UINT8)(pucBcdNum[ucTmp] & 0x0F); /*ȡ���߰��ֽ�*/

        ucSecondNumber = (VOS_UINT8)((pucBcdNum[ucTmp] >> 4) & 0x0F);/*ȡ���Ͱ��ֽ�*/

        /* ���밲ȫ�߶���ӣ���ֹԽ����� */
        if (ucAsciiNumBufflen <= ucLen)
        {
            ucLen = 0;
            break;
        }

        AT_BcdHalfByteToAscii(ucFirstNumber, pucAsciiNum);

        pucAsciiNum++;

        ucLen++;

        if(ucSecondNumber == 0x0F)
        {
            break;
        }

        /* ���밲ȫ�߶���ӣ���ֹԽ����� */
        if (ucAsciiNumBufflen <= ucLen)
        {
            ucLen = 0;
            break;
        }

        AT_BcdHalfByteToAscii(ucSecondNumber, pucAsciiNum);

        pucAsciiNum++;

        ucLen++;
    }

    *pucLen = ucLen;

    return;
}

#if ((FEATURE_VSIM == FEATURE_ON) && (FEATURE_VSIM_ICC_SEC_CHANNEL == FEATURE_OFF))

VOS_VOID  At_StkHvsmrIndPrint(
    VOS_UINT8                           ucIndex,
    SI_STK_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT16                          usLength = 0;

    /* ��ӡ����AT�������� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s",
                                       gaucAtCrLf,
                                       "^HVSMR: ");

    /* ��ӡ���������������ͳ��Ⱥ����� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d,",
                                       (pstEvent->STKCmdStru.CmdStru.SmsPpDlDataInfo.ucLen*2));

    if (pstEvent->STKCmdStru.CmdStru.SmsPpDlDataInfo.ucLen != VOS_NULL)
    {
        /* ��16������ת��ΪASCII������������������� */
        usLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            (TAF_UINT8 *)pgucAtSndCodeAddr + usLength,
                                                            pstEvent->STKCmdStru.CmdStru.SmsPpDlDataInfo.ucLen,
                                                            pstEvent->STKCmdStru.CmdStru.SmsPpDlDataInfo.aucData);
    }

    /* ��ӡ�س����� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",
                                        gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}
#endif


VOS_VOID  At_StkCcinIndPrint(
    VOS_UINT8                           ucIndex,
    SI_STK_EVENT_INFO_STRU             *pstEvent
)
{
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           aucAscii[250] = {0};
    VOS_UINT8                           ucAsciiLen = 0;
    /* ��ӡ����AT�������� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCcin);

    /* ��ӡcall/sms control ���� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       (pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.ucType));

    /* ��ӡCall/SMS Control�Ľ�� */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       (pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.ucResult));

    /* ��ӡALPHAID��ʶ */
    if (pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.ulAlphaLen != 0)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",\"%s",
                                       (pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.stAlphaIdInfo.aucAlphaId));

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "\"");
     }
     else
     {
         usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            ",\"\"");

     }

    if (pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.ucType == SI_STK_SMS_CTRL)
    {
        /* ��Ŀ�ĺ�����BCD��ת����acsii */
        AT_BcdToAscii(pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stDstAddrInfo.ucAddrLen,
                      pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stDstAddrInfo.aucAddr,
                      aucAscii,
                      sizeof(aucAscii),
                      &ucAsciiLen);

        /* ��ӡĿ�ĵ�ַ������ */
        At_StkNumPrint(&usLength,
                       aucAscii,
                       ucAsciiLen,
                       pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stDstAddrInfo.ucNumType);

        /* ���������ĺ�����BCD��ת����acsii */
        AT_BcdToAscii(pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stSerCenterAddrInfo.ucAddrLen,
                      pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stSerCenterAddrInfo.aucAddr,
                      aucAscii,
                      sizeof(aucAscii),
                      &ucAsciiLen);

        /* ��ӡ�������ĵ�ַ������ */
        At_StkNumPrint(&usLength,
                       aucAscii,
                       ucAsciiLen,
                       pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stMoSmsCtrlInfo.stSerCenterAddrInfo.ucNumType);

    }
    else if (pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.ucType == SI_STK_USSD_CALL_CTRL)
    {

        /* ��ӡdcs�ֶκ�data�ֶ� */
        At_StkNumPrint(&usLength,
                     pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.aucData,
                     pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.usDataLen,
                     pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.ucDataType);
    }
    else
    {

        /* ��Ŀ�ĺ�����BCD��ת����acsii */
        AT_BcdToAscii((VOS_UINT8)pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.usDataLen,
                      pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.aucData,
                      aucAscii,
                      sizeof(aucAscii),
                      &ucAsciiLen);

        /* ��ӡĿ�ĵ�ַ������ */
        At_StkNumPrint(&usLength,
                       aucAscii,
                       ucAsciiLen,
                       pstEvent->STKCmdStru.CmdStru.STKCcIndInfo.uInfo.stCtrlDataInfo.ucDataType);
    }

    /* ��ӡ�س����� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s",
                                        gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCrLfAddr, usLength + 2);

    return;
}


MODULE_EXPORTED VOS_UINT16 AT_GetRealClientId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{

    if (enModemId == MODEM_ID_0)
    {
        return  AT_BROADCAST_CLIENT_ID_MODEM_0;
    }

    if (enModemId == MODEM_ID_1)
    {
        return AT_BROADCAST_CLIENT_ID_MODEM_1;
    }

    if (enModemId == MODEM_ID_2)
    {
        return AT_BROADCAST_CLIENT_ID_MODEM_2;
    }

    AT_ERR_LOG1("AT_GetRealClientId, enModemId err", enModemId);

    return MN_CLIENT_ID_BROADCAST;

}


VOS_VOID AT_ReportCCallstateResult(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT8                           ucCallId,
    VOS_UINT8                          *pucRptCfg,
    AT_CS_CALL_STATE_ENUM_UINT8         enCallState,
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain
)
{
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucIndex;

    VOS_UINT16                          usClientId;

    usClientId = AT_GetRealClientId(enModemId);


    /* ��ȡclient id��Ӧ��Modem Id */
    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_ReportCCallstateResult:WARNING:AT INDEX NOT FOUND!");
        return;
    }

    if (AT_CheckRptCmdStatus(pucRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CALLSTATE) == VOS_TRUE)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s: %d,%d,%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CCALLSTATE].pucText,
                                           ucCallId,
                                           enCallState,
                                           enVoiceDomain,
                                           gaucAtCrLf);

    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_UINT32 AT_RcvTafCcmCallHoldInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_HOLD_IND_STRU         *pstCallHoldInd = VOS_NULL_PTR;
    VOS_UINT8                           ucLoop;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucIndex = 0;

    pstCallHoldInd = (TAF_CCM_CALL_HOLD_IND_STRU *)pMsg;

    if (At_ClientIdToUserId(pstCallHoldInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallHoldInd At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafCcmCallHoldInd: Get modem id fail.");
        return VOS_ERR;
    }

    for (ucLoop = 0; ucLoop < (pstCallHoldInd->stHoldIndPara.ucCallNum); ucLoop++)
    {
        AT_ReportCCallstateResult(enModemId,
                                  pstCallHoldInd->stHoldIndPara.aucCallId[ucLoop],
                                  pstCallHoldInd->stHoldIndPara.aucCurcRptCfg,
                                  AT_CS_CALL_STATE_HOLD,
                                  pstCallHoldInd->stHoldIndPara.enVoiceDomain);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCallRetrieveInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_RETRIEVE_IND_STRU     *pstCallRetrieveInd = VOS_NULL_PTR;
    VOS_UINT8                           ucLoop;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucIndex = 0;

    pstCallRetrieveInd = (TAF_CCM_CALL_RETRIEVE_IND_STRU *)pMsg;

    if(At_ClientIdToUserId(pstCallRetrieveInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallHoldInd At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafCcmCallHoldInd: Get modem id fail.");
        return VOS_ERR;
    }


    for (ucLoop = 0; ucLoop < (pstCallRetrieveInd->stRetrieveInd.ucCallNum); ucLoop++)
    {
        AT_ReportCCallstateResult(enModemId,
                                  pstCallRetrieveInd->stRetrieveInd.aucCallId[ucLoop],
                                  pstCallRetrieveInd->stRetrieveInd.aucCurcRptCfg,
                                  AT_CS_CALL_STATE_RETRIEVE,
                                  pstCallRetrieveInd->stRetrieveInd.enVoiceDomain);
    }

    return VOS_OK;
}

VOS_VOID AT_ReportCendResult(
    VOS_UINT8                           ucIndex,
    TAF_CCM_CALL_RELEASED_IND_STRU     *pstCallReleaseInd
)
{
    VOS_UINT16                          usLength;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    usLength  = 0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_ReportCendResult: Get modem id fail.");
        return;
    }

    if (AT_CheckRptCmdStatus(pstCallReleaseInd->stReleaseIndPara.aucCurcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CEND) == VOS_TRUE)
    {
        usLength    += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s^CEND:%d,%d,%d,%d%s",
                                              gaucAtCrLf,
                                              pstCallReleaseInd->stReleaseIndPara.callId,
                                              pstCallReleaseInd->stReleaseIndPara.ulPreCallTime,
                                              pstCallReleaseInd->stReleaseIndPara.enNoCliCause,
                                              pstCallReleaseInd->stReleaseIndPara.enCause,
                                              gaucAtCrLf);
        At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    }

    return;
}


VOS_UINT16 At_PrintClprInfo(
    VOS_UINT8                           ucIndex,
    TAF_CCM_QRY_CLPR_CNF_STRU          *pstClprGetCnf)
{
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucType;
    VOS_CHAR                            aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN*2) + 1];
    VOS_UINT32                          ulAsiciiLen;

    /* ��ʼ�� */
    usLength = 0;
    memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                             (VOS_CHAR *)pgucAtSndCodeAddr,
                             (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                             "%s: ",
                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    /* ���<RedirectNumPI>���� */
    if (pstClprGetCnf->stClprCnf.stClprInfo.bitOpCallingNumPI == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 "%d,",
                                 pstClprGetCnf->stClprCnf.stClprInfo.enCallingNumPI);

    }
    else
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 ",");
    }

    /* ���<no_CLI_cause>���� */
    if (pstClprGetCnf->stClprCnf.stClprInfo.bitOpNoCLICause == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 "%d,",
                                 pstClprGetCnf->stClprCnf.stClprInfo.enNoCLICause);

    }
    else
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 ",");
    }

    /* ���<redirect_num>��<num_type>���� */
    if (pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.bitOpRedirectNum == VOS_TRUE)
    {
        AT_BcdNumberToAscii(pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectNum.aucBcdNum,
                            pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectNum.ucNumLen,
                            aucAsciiNum);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 "\"%s\",%d,",
                                 aucAsciiNum,
                                 (pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectNum.enNumType| AT_NUMBER_TYPE_EXT));

    }
    else
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 "\"\",,");
    }

    /* ���<redirect_subaddr>��<num_type>���� */
    ucType = (MN_CALL_IS_EXIT | (MN_CALL_SUBADDR_NSAP << 4));
    if ((pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.bitOpRedirectSubaddr == VOS_TRUE)
     && (ucType == pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.Octet3))
    {
        if (pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.LastOctOffset < sizeof(pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.Octet3))
        {
            AT_WARN_LOG1("At_PrintClprInfo: pstClprGetCnf->stRedirectInfo.stRedirectSubaddr.LastOctOffset: ",
                pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.LastOctOffset);
            pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.LastOctOffset = sizeof(pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.Octet3);
        }

        ulAsiciiLen = pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.LastOctOffset
                    - sizeof(pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.Octet3);

        ulAsiciiLen = TAF_MIN(ulAsiciiLen, sizeof(pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.SubAddrInfo));
        lMemResult = memcpy_s(aucAsciiNum,
                              sizeof(aucAsciiNum),
                              pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.SubAddrInfo,
                              ulAsiciiLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucAsciiNum), ulAsiciiLen);

        aucAsciiNum[ulAsiciiLen] = '\0';

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 "\"%s\",%d,",
                                 aucAsciiNum,
                                 pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.stRedirectSubaddr.Octet3);
    }
    else
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                 "\"\",,");
    }

    /* ���<CallingNumPI>���� */
    if (pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.bitOpRedirectNumPI == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,
            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
            "%d",
            pstClprGetCnf->stClprCnf.stClprInfo.stRedirectInfo.enRedirectNumPI);

    }

    return usLength;
}

VOS_UINT32 AT_RcvTafCcmQryClprCnf(VOS_VOID *pMsg)
{
    TAF_CCM_QRY_CLPR_CNF_STRU          *pstClprGetCnf = VOS_NULL_PTR;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;


    /* ��ʼ�� */
    pstClprGetCnf = (TAF_CCM_QRY_CLPR_CNF_STRU *)pMsg;
    ucIndex       = 0;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstClprGetCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("At_SetClprCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    /* ���Ϊ�㲥���ͣ��򷵻�AT_ERROR */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_SetClprCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CLPR_SET */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CLPR_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCLPR_SET) )
    {
        AT_WARN_LOG("At_SetClprCnf: WARNING:Not AT_CMD_CLPR_GET!");

        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (pstClprGetCnf->stClprCnf.ulRet != TAF_ERR_NO_ERROR)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    usLength = 0;

    if (pstClprGetCnf->stClprCnf.enQryClprModeType == TAF_CALL_QRY_CLPR_MODE_GUL)
    {
        usLength = At_PrintClprInfo(ucIndex, pstClprGetCnf);
    }
    else
    {
        if (pstClprGetCnf->stClprCnf.enPI != TAF_CALL_PRESENTATION_BUTT)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s: %d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              pstClprGetCnf->stClprCnf.enPI);
        }
    }

    /* ��ӡ��� */
    gstAtSendData.usBufLen  = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;

}


VOS_UINT32 AT_RcvDrvAgentSwverSetCnf(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_SWVER_SET_CNF_STRU       *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_SWVER_SET_CNF_STRU *)pstRcvMsg->aucContent;

    AT_PR_LOGI("Rcv Msg");

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentSwverSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSwverSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�SWVER��ѯ����Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_SWVER_SET)
    {
        AT_WARN_LOG("AT_RcvDrvAgentSwverSetCnf: WARNING:Not AT_CMD_SWVER_SET!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_NO_ERROR)
    {
        /* ���ô�����ΪAT_OK           ����ṹΪ<CR><LF>^SWVER: <SwVer>_(<VerTime>)<CR><LF>
             <CR><LF>OK<CR><LF>��ʽ */
        ulRet                  = AT_OK;

        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %s_(%s)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstEvent->stSwverInfo.aucSWVer,
                                          pstEvent->stSwverInfo.acVerTime);

    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet                  = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSetCssnCnf(VOS_VOID *pMsg)
{
    TAF_CCM_SET_CSSN_CNF_STRU      *pstCssnCnf = VOS_NULL_PTR;
    VOS_UINT8                       ucIndex;
    VOS_UINT32                      ulResult;

    pstCssnCnf = (TAF_CCM_SET_CSSN_CNF_STRU *)pMsg;

    if (At_ClientIdToUserId(pstCssnCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvMnCallSetCssnCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_RcvMnCallSetCssnCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ���������Ļظ�����¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CSSN_SET)
    {
        AT_WARN_LOG("At_RcvMnCallSetCssnCnf: WARNING:Not AT_CMD_APP_SET_CSSN_REQ!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstCssnCnf->ulRet == TAF_ERR_NO_ERROR)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_CME_INCORRECT_PARAMETERS;
    }

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


TAF_CALL_CHANNEL_TYPE_ENUM_UINT8 AT_ConvertCodecTypeToChannelType(
    VOS_UINT8                           ucIsLocalAlertingFlag,
    MN_CALL_CODEC_TYPE_ENUM_U8          enCodecType
)
{
    TAF_CALL_CHANNEL_TYPE_ENUM_UINT8    enChannelType;

    enChannelType = TAF_CALL_CHANNEL_TYPE_NONE;

    if (ucIsLocalAlertingFlag == VOS_TRUE)
    {
        /* ��������,�޴�������Ϣ */
        enChannelType = TAF_CALL_CHANNEL_TYPE_NONE;
    }
    else
    {
        /* �������壬��Ҫ����channel type */
        /* ���������ã�Ĭ��խ������ */
        enChannelType = TAF_CALL_CHANNEL_TYPE_NARROW;

        if ((enCodecType == MN_CALL_CODEC_TYPE_AMRWB)
         || (enCodecType == MN_CALL_CODEC_TYPE_EVS))
        {
            /* ���������ã�������� */
            enChannelType = TAF_CALL_CHANNEL_TYPE_WIDE;
        }

        if (enCodecType == MN_CALL_CODEC_TYPE_EVS_NB)
        {
            enChannelType = TAF_CALL_CHANNEL_TYPE_EVS_NB;
        }

        if (enCodecType == MN_CALL_CODEC_TYPE_EVS_WB)
        {
            enChannelType = TAF_CALL_CHANNEL_TYPE_EVS_WB;
        }

        if (enCodecType == MN_CALL_CODEC_TYPE_EVS_SWB)
        {
            enChannelType = TAF_CALL_CHANNEL_TYPE_EVS_SWB;
        }
    }

    return enChannelType;
}




VOS_UINT32 AT_RcvTafCcmQryXlemaCnf(VOS_VOID *pMsg)
{
    TAF_CCM_QRY_XLEMA_CNF_STRU         *pstQryXlemaCnf = VOS_NULL_PTR;
    VOS_UINT8                           aucAsciiNum[(MN_CALL_MAX_BCD_NUM_LEN*2)+1];
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          i;
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulMcc;

    pstQryXlemaCnf  = (TAF_CCM_QRY_XLEMA_CNF_STRU *)pMsg;
    ucIndex         = 0;
    ulMcc           = 0;

    memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstQryXlemaCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvXlemaQryCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* ���Ϊ�㲥���ͣ��򷵻� */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_RcvXlemaQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_XLEMA_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_XLEMA_QRY )
    {
        AT_WARN_LOG("At_RcvXlemaQryCnf: WARNING:Not AT_CMD_XLEMA_QRY!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ��APP�����ϱ����������� */
    for (i = 0; i < pstQryXlemaCnf->stQryXlemaPara.ulEccNumCount; i++)
    {
        /* ��BCD��ת��ΪASCII�� */
        AT_BcdNumberToAscii(pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[i].aucEccNum,
                            pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[i].ucEccNumLen,
                            (VOS_CHAR*)aucAsciiNum);

        /* ��NAS��ʽ��MCCת��ΪBCD��ʽ */
        AT_ConvertNasMccToBcdType(pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[i].ulMcc, &ulMcc);

        usLength = 0;
        usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           "%s^XLEMA: ",
                                           gaucAtCrLf);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           "%d,%d,%s,%d,%d,",
                                           (i+1),
                                           pstQryXlemaCnf->stQryXlemaPara.ulEccNumCount,
                                           aucAsciiNum,
                                           pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[i].ucCategory,
                                           pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[i].ucValidSimPresent);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           "%x%x%x",
                                           (ulMcc & 0x0f00)>>8,
                                           (ulMcc & 0xf0)>>4,
                                           (ulMcc & 0x0f));

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr +
                                           usLength,
                                           ",%d%s",
                                           pstQryXlemaCnf->stQryXlemaPara.astCustomEccNumList[i].ucAbnormalServiceFlg,
                                           gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

        memset_s(aucAsciiNum, sizeof(aucAsciiNum), 0x00, sizeof(aucAsciiNum));
    }

    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}





VOS_UINT32 AT_RcvTafCcmStartDtmfCnf(VOS_VOID *pMsg)
{
    TAF_CCM_START_DTMF_CNF_STRU        *pstDtmfCNf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pstDtmfCNf = (TAF_CCM_START_DTMF_CNF_STRU *)pMsg;

    /* ����ClientID��ȡͨ������ */
    if(At_ClientIdToUserId(pstDtmfCNf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallStartDtmfCnf: Get Index Fail!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallStartDtmfCnf: At Is Broadcast!");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�^DTMF����/+VTS����Ĳ�������¼��ϱ� */
    if ( (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DTMF_SET)
      && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_VTS_SET) )
    {
        AT_WARN_LOG("AT_RcvTafCallStartDtmfCnf: Error Option!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ʱ��Ӧ�Ĵ������ӡ����Ľ�� */
    if (pstDtmfCNf->stStartDtmfPara.enCause != TAF_CS_CAUSE_SUCCESS)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmStopDtmfCnf(VOS_VOID *pMsg)
{
    TAF_CCM_STOP_DTMF_CNF_STRU         *pstDtmfCNf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pstDtmfCNf = (TAF_CCM_STOP_DTMF_CNF_STRU *)pMsg;

    /* ����ClientID��ȡͨ������ */
    if(At_ClientIdToUserId(pstDtmfCNf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallStopDtmfCnf: Get Index Fail!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallStopDtmfCnf: At Is Broadcast!");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�^DTMF����Ĳ�������¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DTMF_SET)
    {
        AT_WARN_LOG("AT_RcvTafCallStopDtmfCnf: Error Option!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ʱ��Ӧ�Ĵ������ӡ����Ľ�� */
    if (pstDtmfCNf->stStopDtmfPara.enCause != TAF_CS_CAUSE_SUCCESS)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}



VOS_UINT32 At_RcvTafCcmCallOrigCnf(VOS_VOID *pMsg)
{
    TAF_UINT8                           ucIndex;
    TAF_CCM_CALL_ORIG_CNF_STRU         *pstOrigCnf    = VOS_NULL_PTR;
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength;

    ulResult = AT_FAILURE;
    usLength = 0;
    ucIndex  = 0;

    pstOrigCnf = (TAF_CCM_CALL_ORIG_CNF_STRU *)pMsg;

    AT_LOG1("At_RcvTafCallOrigCnf pEvent->ClientId",pstOrigCnf->stCtrl.usClientId);
    AT_LOG1("At_RcvTafCallOrigCnf usMsgName",pstOrigCnf->ulMsgName);


    if (At_ClientIdToUserId(pstOrigCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_CsEventProc At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_RcvTafCallOrigCnf: At Is Broadcast!");
        return VOS_ERR;
    }

    /* �ɹ�ʱ���ظ�OK��ʧ��ʱ���ظ�NO CARRIER */
    if (pstOrigCnf->stOrigCnfPara.enCause == TAF_CS_CAUSE_SUCCESS)
    {
        /* ���ӵ绰���棬���ﲻ���ϱ�OK�����ֻ����ͨ�����ͽ������е�����£����ϱ�OK��AT����������׶��൱������һ��ʱ�� */
        if (At_CheckOrigCnfCallType(pstOrigCnf->stOrigCnfPara.enCallType, ucIndex) == PS_TRUE)
        {
            ulResult = AT_OK;
        }
        else
        {
            if (pstOrigCnf->stOrigCnfPara.enCallType == MN_CALL_TYPE_VIDEO)
            {
                AT_STOP_TIMER_CMD_READY(ucIndex);
            }
            return VOS_OK;
        }

    }
    else
    {
        if (pstOrigCnf->stOrigCnfPara.enCause == TAF_CS_CAUSE_NO_CALL_ID)
        {
            ulResult = AT_ERROR;
        }
        else
        {
            ulResult = AT_NO_CARRIER;
        }

        AT_UpdateCallErrInfo(ucIndex, pstOrigCnf->stOrigCnfPara.enCause, &(pstOrigCnf->stOrigCnfPara.stErrInfoText));
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


PS_BOOL_ENUM_UINT8 AT_CheckCurrentOptType_SupsCmdSuccess(
    VOS_UINT8                           ucAtaReportOkAsyncFlag,
    TAF_UINT8                           ucIndex
)
{
    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_H_SET :
        case AT_CMD_CHUP_SET:
        case AT_CMD_REJCALL_SET:
#if (FEATURE_ECALL == FEATURE_ON)
        case AT_CMD_ECLSTOP_SET:
#endif
            return PS_TRUE;
        case AT_CMD_A_SET:
        case AT_CMD_CHLD_SET:
        case AT_CMD_CHLD_EX_SET:
            if (ucAtaReportOkAsyncFlag == VOS_TRUE)
            {
                return PS_TRUE;
            }

            return PS_FALSE;
        default:
            return PS_FALSE;
    }
}


PS_BOOL_ENUM_UINT8 AT_CheckCurrentOptType_SupsCmdOthers(
    TAF_UINT8                           ucIndex
)
{
    switch(gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_CHLD_SET:
        case AT_CMD_CHUP_SET:
        case AT_CMD_A_SET:
        case AT_CMD_CHLD_EX_SET:
        case AT_CMD_H_SET:
        case AT_CMD_REJCALL_SET:
            return PS_TRUE;

        default:
            return PS_FALSE;
    }
}


VOS_UINT32 At_RcvTafCcmCallSupsCmdCnf(VOS_VOID * pMsg)
{
    TAF_UINT8                           ucIndex;
    TAF_CCM_CALL_SUPS_CMD_CNF_STRU     *pstCallSupsCmdCnf   = VOS_NULL_PTR;
    TAF_UINT32                          ulResult;
    TAF_UINT16                          usLength;

    ulResult    = AT_FAILURE;
    usLength    = 0;
    ucIndex     = 0;

    pstCallSupsCmdCnf = (TAF_CCM_CALL_SUPS_CMD_CNF_STRU *)pMsg;

    AT_LOG1("At_RcvTafCallOrigCnf pEvent->ClientId",pstCallSupsCmdCnf->stCtrl.usClientId);
    AT_LOG1("At_RcvTafCallOrigCnf usMsgName",pstCallSupsCmdCnf->ulMsgName);

    AT_PR_LOGH("At_RcvTafCcmCallSupsCmdCnf Enter : enCause = %d", pstCallSupsCmdCnf->stSupsCmdPara.enCause);


    if(At_ClientIdToUserId(pstCallSupsCmdCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvTafCallSupsCmdCnf At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_RcvTafCallSupsCmdCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT���Ѿ��ͷ� */
    if (g_stParseContext[ucIndex].ucClientStatus == AT_FW_CLIENT_STATUS_READY)
    {
        AT_WARN_LOG("At_RcvTafCallSupsCmdCnf : AT command entity is released.");
        return VOS_ERR;
    }

    /* �Ҷϵ绰�ɹ�ʱ�ظ�OK */
    if (pstCallSupsCmdCnf->stSupsCmdPara.enCause == TAF_CS_CAUSE_SUCCESS)
    {
        if (AT_CheckCurrentOptType_SupsCmdSuccess(pstCallSupsCmdCnf->stSupsCmdPara.ucAtaReportOkAsyncFlag, ucIndex) == PS_TRUE)
        {
            ulResult = AT_OK;
        }
        else
        {
            return VOS_OK;
        }
    }
    else
    {
        if (AT_CheckCurrentOptType_SupsCmdOthers(ucIndex) == PS_TRUE)
        {
            ulResult = AT_ConvertCallError(pstCallSupsCmdCnf->stSupsCmdPara.enCause);
        }
        else
        {
            ulResult = AT_CME_UNKNOWN;
        }
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafCcmCallSupsCmdRsltInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_SUPS_CMD_RSLT_STRU    *pstCallSupsCmdRslt = VOS_NULL_PTR;
    VOS_UINT16                          ulResult = 0;
    TAF_UINT16                          usLength = 0;
    VOS_UINT8                           ucIndex  = 0;

    pstCallSupsCmdRslt = (TAF_CCM_CALL_SUPS_CMD_RSLT_STRU *)pMsg;

    AT_PR_LOGH("AT_RcvTafCcmCallSupsCmdRsltInd Enter : enCause = %d", pstCallSupsCmdRslt->stSupsCmdRsltPara.enSsResult);

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCallSupsCmdRslt->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallSupsCmdRsltInd: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCcmCallSupsCmdRsltInd: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_CURRENT_OPT_BUTT)
    {
        return VOS_ERR;
    }

    if(pstCallSupsCmdRslt->stSupsCmdRsltPara.enSsResult == MN_CALL_SS_RES_SUCCESS)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

    return VOS_OK;
}


VOS_VOID At_CsIndEvtReleaseProc(
    TAF_UINT8                           ucIndex,
    TAF_CCM_CALL_RELEASED_IND_STRU     *pstCallReleaseInd
)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;
    AT_DCE_MSC_STRU                     stMscStru;
    TAF_UINT16                          usLoop;
    VOS_UINT32                          ulTimerName;



    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(ucIndex);

    AT_UpdateCallErrInfo(ucIndex, pstCallReleaseInd->stReleaseIndPara.enCause, &(pstCallReleaseInd->stReleaseIndPara.stErrInfoText));

    /* ����ǿ��ӵ绰����Ϊ��INCOMING��ʱ��������DCD�Ĺܽ��źţ���������DCD�Ĺܽ��ź� */
    if (AT_EVT_IS_CS_VIDEO_CALL(pstCallReleaseInd->stReleaseIndPara.enCallType, pstCallReleaseInd->stReleaseIndPara.enVoiceDomain))
    {

        /* ���ڴ�ʱ���ǹ㲥�ϱ�����Ҫ�ҵ���Ӧ��MODEM�˿�,���йܽ��źŵ�
           ���� */
        for(usLoop = 0; usLoop < AT_MAX_CLIENT_NUM; usLoop++)
        {
            if (gastAtClientTab[usLoop].UserType == AT_MODEM_USER)
            {
                ucIndex = (VOS_UINT8)usLoop;
                break;
            }
        }

        /* ����ǿ��ӵ绰�������ǰû��MODEM�˿ڣ���ֱ�ӷ��� */
        if (usLoop == AT_MAX_CLIENT_NUM)
        {
            return;
        }


        memset_s(&stMscStru, sizeof(stMscStru), 0x00, sizeof(stMscStru));
        /* ����DCD�ź� */
        stMscStru.OP_Dcd = 1;
        stMscStru.ucDcd = 0;
        stMscStru.OP_Dsr = 1;
        stMscStru.ucDsr = 1;
        AT_SetModemStatus(ucIndex,&stMscStru);

        /*EVENT-UE Down DCD*/
        AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DCE_DOWN_DCD,
                        VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);
        return;
    }

    if (pstCcCtx->stS0TimeInfo.bTimerStart == VOS_TRUE)
    {
        ulTimerName = pstCcCtx->stS0TimeInfo.ulTimerName;

        AT_StopRelTimer(ulTimerName, &(pstCcCtx->stS0TimeInfo.s0Timer));
        pstCcCtx->stS0TimeInfo.bTimerStart = VOS_FALSE;
        pstCcCtx->stS0TimeInfo.ulTimerName = 0;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* ֹͣRING TE */
    AT_VoiceStopRingTe(pstCallReleaseInd->stReleaseIndPara.callId);
#endif

    AT_ReportCendResult(ucIndex, pstCallReleaseInd);

    return;
}


VOS_UINT32 AT_RcvTafCcmCallReleaseInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_RELEASED_IND_STRU     *pstCallReleaseInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex  = 0;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId                           = MODEM_ID_0;


    pstCallReleaseInd = (TAF_CCM_CALL_RELEASED_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCallReleaseInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallReleaseInd: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }


    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        At_CsIndEvtReleaseProc(ucIndex, pstCallReleaseInd);
    }
    else
    {
        AT_LOG1("AT_RcvTafCcmCallReleaseInd ucIndex",ucIndex);
        AT_LOG1("AT_RcvTafCcmCallReleaseInd[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

        AT_CsRspEvtReleasedProc(ucIndex, pstCallReleaseInd);
    }


    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafCcmCallReleaseInd: Get modem id fail.");
        return VOS_ERR;
    }


    AT_ReportCCallstateResult(enModemId,
                              pstCallReleaseInd->stReleaseIndPara.callId,
                              pstCallReleaseInd->stReleaseIndPara.aucCurcRptCfg,
                              AT_CS_CALL_STATE_RELEASED,
                              pstCallReleaseInd->stReleaseIndPara.enVoiceDomain);


    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCallAllReleasedInd(VOS_VOID *pMsg)
{
    AT_MODEM_CC_CTX_STRU                   *pstCcCtx          = VOS_NULL_PTR;
    TAF_CCM_CALL_ALL_RELEASED_IND_STRU     *pstCallReleaseInd = VOS_NULL_PTR;
    VOS_UINT8                               ucIndex           = 0;

    pstCallReleaseInd = (TAF_CCM_CALL_ALL_RELEASED_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCallReleaseInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallAllReleased: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(ucIndex);
    pstCcCtx->ulCurIsExistCallFlag = VOS_FALSE;

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCallAlertingInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_ALERTING_IND_STRU     *pstCallAlertingInd = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucIndex = 0;

    pstCallAlertingInd = (TAF_CCM_CALL_ALERTING_IND_STRU *)pMsg;

    if (At_ClientIdToUserId(pstCallAlertingInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallAlertingInd At_ClientIdToUserId FAILURE");
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafCcmCallAlertingInd: Get modem id fail.");
        return VOS_ERR;
    }

    AT_ReportCCallstateResult(enModemId, pstCallAlertingInd->stAlertingIndPara.callId, pstCallAlertingInd->stAlertingIndPara.aucCurcRptCfg, AT_CS_CALL_STATE_ALERTING, pstCallAlertingInd->stAlertingIndPara.enVoiceDomain);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCallSsInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_SS_IND_STRU           *pstCallSsInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex  = 0;

    pstCallSsInd = (TAF_CCM_CALL_SS_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstCallSsInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallSsInd: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }


    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_CsSsNotifyEvtIndProc(ucIndex, pstCallSsInd);
    }
    else
    {
        AT_LOG1("AT_RcvTafCcmCallSsInd ucIndex",ucIndex);
        AT_LOG1("AT_RcvTafCcmCallSsInd[ucIndex].CmdCurrentOpt",gastAtClientTab[ucIndex].CmdCurrentOpt);

        AT_ProcCsRspEvtSsNotify(ucIndex, pstCallSsInd);
    }

    return VOS_OK;
}


#if ((FEATURE_VSIM == FEATURE_ON) && (FEATURE_VSIM_ICC_SEC_CHANNEL == FEATURE_OFF))

VOS_UINT32 AT_RcvDrvAgentHvpdhSetCnf(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_HVPDH_CNF_STRU           *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_HVPDH_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvDrvAgentHvpdhSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentHvpdhSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�HVPDH��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_HVPDH_SET)
    {
        AT_WARN_LOG("AT_RcvDrvAgentHvpdhSetCnf: WARNING:Not AT_CMD_HVPDH_SET!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEvent->enResult == DRV_AGENT_HVPDH_NO_ERROR)
    {
        ulRet = AT_OK;
    }
    else
    {
        /* ��ѯʧ�ܷ���ERROR�ַ��� */
        ulRet = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* ����At_FormatResultData������ */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}
#endif

VOS_VOID AT_PhEOPlmnQueryCnfProc(TAF_UINT8 *pData)
{
    VOS_UINT16                                              usLen;
    VOS_UINT8                                               ucIndex;
    VOS_UINT32                                              ulRslt;
    TAF_PHONE_EVENT_EOPLMN_QRY_CNF_STRU                    *pstEOPlmnQryCnf = VOS_NULL_PTR;

    usLen            = 0;
    pstEOPlmnQryCnf  = (TAF_PHONE_EVENT_EOPLMN_QRY_CNF_STRU *)pData;

    /* ͨ��ClientId��ȡucIndex */
    if (At_ClientIdToUserId(pstEOPlmnQryCnf->ClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_PhEOPlmnQueryCnfProc At_ClientIdToUserId FAILURE");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_EOPLMN_QRY */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_EOPLMN_QRY )
    {
        AT_WARN_LOG("AT_PhEOPlmnQueryCnfProc: WARNING:Not AT_CMD_EOPLMN_QRY!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �����ѯ��� */
    if (pstEOPlmnQryCnf->ulResult == TAF_ERR_NO_ERROR)
    {
        usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLen,
                                        "%s: \"%s\",%d,",
                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                        pstEOPlmnQryCnf->aucVersion,
                                        pstEOPlmnQryCnf->usOPlmnNum * TAF_AT_PLMN_WITH_RAT_LEN);


        /* ��16������ת��ΪASCII������������������� */
        usLen += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN,
                                                            (TAF_INT8 *)pgucAtSndCodeAddr,
                                                            (TAF_UINT8 *)pgucAtSndCodeAddr + usLen,
                                                            pstEOPlmnQryCnf->usOPlmnNum * TAF_SIM_PLMN_WITH_RAT_LEN,
                                                            pstEOPlmnQryCnf->aucOPlmnList);

        gstAtSendData.usBufLen = usLen;

        ulRslt = AT_OK;
    }
    else
    {
        gstAtSendData.usBufLen = 0;
        ulRslt = AT_ERROR;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return;
}


VOS_UINT32 AT_RcvNvManufactureExtSetCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                                       ucIndex;
    DRV_AGENT_NVMANUFACTUREEXT_SET_CNF_STRU        *pstEvent = VOS_NULL_PTR;
    DRV_AGENT_MSG_STRU                             *pstRcvMsg = VOS_NULL_PTR;

    /* ��ʼ�� */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_NVMANUFACTUREEXT_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvNvManufactureExtSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvNvManufactureExtSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�NvManufactureExt��������Ľ���¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_NVMANUFACTUREEXT_SET)
    {
        AT_WARN_LOG("AT_RcvNvManufactureExtSetCnf: WARNING:Not AT_CMD_NVMANUFACTUREEXT_SET!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (pstEvent->ulRslt == NV_OK)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%d",
                                                        pstEvent->ulRslt);
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_VOID At_ChangeEcallTypeToCallType(
    MN_CALL_TYPE_ENUM_U8                enEcallType,
    MN_CALL_TYPE_ENUM_U8               *enCallType
)
{
    switch (enEcallType)
    {
        case MN_CALL_TYPE_TEST :
        case MN_CALL_TYPE_RECFGURATION :
        case MN_CALL_TYPE_PSAP_ECALL :
            *enCallType = MN_CALL_TYPE_VOICE;
            break;

        case MN_CALL_TYPE_MIEC :
        case MN_CALL_TYPE_AIEC :
            *enCallType = MN_CALL_TYPE_EMERGENCY;
            break;

        default:
            *enCallType = enEcallType;
            break;
    }

}


VOS_UINT32 At_IsCmdCurrentOptSendedOrigReq(AT_CMD_CURRENT_OPT_ENUM CmdCurrentOpt)
{
    switch (CmdCurrentOpt)
    {
        case AT_CMD_APDS_SET :
        case AT_CMD_D_CS_VOICE_CALL_SET :
        case AT_CMD_CECALL_SET:
        case AT_CMD_ECLSTART_SET:
        case AT_CMD_CACMIMS_SET:
 #if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case AT_CMD_CUSTOMDIAL_SET:
 #endif
             return VOS_TRUE;

        default:
             return VOS_FALSE;
    }
}


PS_BOOL_ENUM_UINT8 At_CheckOrigCnfCallType(
    MN_CALL_TYPE_ENUM_U8                enCallType,
    VOS_UINT8                           ucIndex
)
{
    switch (enCallType)
    {
        case MN_CALL_TYPE_VOICE :
        case MN_CALL_TYPE_EMERGENCY :
        case MN_CALL_TYPE_VIDEO_RX:
        case MN_CALL_TYPE_VIDEO_TX:
        case MN_CALL_TYPE_MIEC :
        case MN_CALL_TYPE_AIEC :
        case MN_CALL_TYPE_TEST :
        case MN_CALL_TYPE_RECFGURATION :
            if (At_IsCmdCurrentOptSendedOrigReq(gastAtClientTab[ucIndex].CmdCurrentOpt) == VOS_TRUE)
            {
                return PS_TRUE;
            }

            return PS_FALSE;
        case MN_CALL_TYPE_VIDEO:
            if (gastAtClientTab[ucIndex].CmdCurrentOpt == AT_CMD_APDS_SET)
            {
                return PS_TRUE;
            }
            else
            {
                return PS_FALSE;
            }
        default:
            return PS_FALSE;
    }

}


PS_BOOL_ENUM_UINT8 At_CheckReportCendCallType(
    MN_CALL_TYPE_ENUM_U8                enCallType
)
{
    switch (enCallType)
    {
        case MN_CALL_TYPE_VOICE :
        case MN_CALL_TYPE_EMERGENCY :
        case MN_CALL_TYPE_MIEC :
        case MN_CALL_TYPE_AIEC :
        case MN_CALL_TYPE_TEST :
        case MN_CALL_TYPE_RECFGURATION :
        case MN_CALL_TYPE_PSAP_ECALL :
            return PS_TRUE;
        default:
            return PS_FALSE;
    }

}


PS_BOOL_ENUM_UINT8 At_CheckReportOrigCallType(
    MN_CALL_TYPE_ENUM_U8                enCallType
)
{
    switch (enCallType)
    {
        case MN_CALL_TYPE_VOICE :
        case MN_CALL_TYPE_EMERGENCY :
            return PS_TRUE;

        default:
            return PS_FALSE;
    }

}


PS_BOOL_ENUM_UINT8 At_CheckReportConfCallType(
    MN_CALL_TYPE_ENUM_U8                enCallType
)
{
    switch (enCallType)
    {
        case MN_CALL_TYPE_VOICE :
        case MN_CALL_TYPE_EMERGENCY :
        case MN_CALL_TYPE_MIEC :
        case MN_CALL_TYPE_AIEC :
        case MN_CALL_TYPE_TEST :
        case MN_CALL_TYPE_RECFGURATION :
        case MN_CALL_TYPE_PSAP_ECALL :
            return PS_TRUE;
        default:
            return PS_FALSE;
    }

}


PS_BOOL_ENUM_UINT8 At_CheckUartRingTeCallType(
    MN_CALL_TYPE_ENUM_U8                enCallType
)
{
    switch (enCallType)
    {
        case MN_CALL_TYPE_VOICE :
        case MN_CALL_TYPE_EMERGENCY :
        case MN_CALL_TYPE_MIEC :
        case MN_CALL_TYPE_AIEC :
        case MN_CALL_TYPE_TEST :
        case MN_CALL_TYPE_RECFGURATION :
        case MN_CALL_TYPE_PSAP_ECALL :
            return PS_TRUE;
        default:
            return PS_FALSE;
    }

}

#if (FEATURE_ECALL == FEATURE_ON)

VOS_UINT32 At_ProcVcReportEcallStateEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
)
{
    if ((pstVcEvtInfo->enEcallState == APP_VC_ECALL_MSD_TRANSMITTING_FAIL)
     || (pstVcEvtInfo->enEcallState == APP_VC_ECALL_PSAP_MSD_REQUIRETRANSMITTING))
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^ECLSTAT: %d,%d%s",
                                                    gaucAtCrLf,
                                                    pstVcEvtInfo->enEcallState,
                                                    pstVcEvtInfo->ulEcallDescription,
                                                    gaucAtCrLf);
    }
    else
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^ECLSTAT: %d%s",
                                                    gaucAtCrLf,
                                                    pstVcEvtInfo->enEcallState,
                                                    gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 At_ProcVcSetEcallCfgEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
)
{
    VOS_UINT32                          ulRslt;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ECLCFG_SET)
    {
        return VOS_ERR;
    }

    if (pstVcEvtInfo->bSuccess != VOS_TRUE)
    {
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}



VOS_VOID At_MapEcallType(
    MN_CALL_TYPE_ENUM_U8                enCallType,
    AT_ECALL_TYPE_ENUM_U8              *penEcallType
)
{
    switch (enCallType)
        {
            case MN_CALL_TYPE_TEST:
                *penEcallType = AT_ECALL_TYPE_TEST;
                break;
            case MN_CALL_TYPE_RECFGURATION:
                *penEcallType = AT_ECALL_TYPE_RECFGURATION;
                break;
            case MN_CALL_TYPE_MIEC:
                *penEcallType = AT_ECALL_TYPE_MIEC;
                break;
            case MN_CALL_TYPE_AIEC:
                *penEcallType = AT_ECALL_TYPE_AIEC;
                break;
            default:
                *penEcallType = AT_ECALL_TYPE_BUTT;
                break;
        }
    return;
}

VOS_UINT32 AT_RcvTafCcmQryEcallInfoCnf(VOS_VOID *pMsg)
{
    TAF_CCM_QRY_ECALL_INFO_CNF_STRU                        *pstEcallInfo = VOS_NULL_PTR;
    VOS_UINT8                                               ucIndex;
    VOS_UINT32                                              ulRslt;
    VOS_UINT32                                              i;
    AT_ECALL_TYPE_ENUM_U8                                   enEcallType;

    pstEcallInfo = (TAF_CCM_QRY_ECALL_INFO_CNF_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstEcallInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvTafCallQryEcallInfoCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CECALL_QRY)
    {
        return VOS_ERR;
    }

    /* ��ʼ�� */
    enEcallType = AT_ECALL_TYPE_BUTT;

    /* ECALL���Ͳ�ѯʱ��CALL��ظ���ǰ���е�CALL��
       ������Ҫ���в��ҵ���һ��ECALL��
    */
    for (i = 0; i < pstEcallInfo->stEcallInfo.ucNumOfEcall; i++)
    {
        At_MapEcallType(pstEcallInfo->stEcallInfo.astEcallInfos[i].enEcallType,
                        &enEcallType);

        if (enEcallType != AT_ECALL_TYPE_BUTT)
        {
            break;
        }
    }

    if (enEcallType == AT_ECALL_TYPE_BUTT)
    {
        /* �����ѯ��� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "+CECALL: ");
    }
    else
    {
        /* �����ѯ��� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "+CECALL: %d",
                                                        enEcallType);
    }

    ulRslt = AT_OK;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}
#endif


#if (FEATURE_IMS == FEATURE_ON)

VOS_UINT32 AT_RcvTafCcmCallModifyStatusInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALL_MODIFY_STATUS_IND_STRU *pstStatusInd = VOS_NULL_PTR;
    VOS_UINT16                           usLength;
    VOS_UINT8                            ucIndex;

    usLength          = 0;
    ucIndex           = 0;
    pstStatusInd      = (TAF_CCM_CALL_MODIFY_STATUS_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstStatusInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmCallModifyStatusInd:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }


    if (pstStatusInd->enModifyStatus == MN_CALL_MODIFY_REMOTE_USER_REQUIRE_TO_MODIFY)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s %d,%d,%d,%d,%d,%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CALL_MODIFY_IND].pucText,
                                           pstStatusInd->ucCallId,
                                           pstStatusInd->enCurrCallType,
                                           pstStatusInd->enVoiceDomain,
                                           pstStatusInd->enExpectCallType,
                                           pstStatusInd->enVoiceDomain,
                                           pstStatusInd->enModifyReason,
                                           gaucAtCrLf);
    }
    else if (pstStatusInd->enModifyStatus == MN_CALL_MODIFY_PROC_BEGIN)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s %d,%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CALL_MODIFY_BEG].pucText,
                                           pstStatusInd->ucCallId,
                                           pstStatusInd->enVoiceDomain,
                                           gaucAtCrLf);
    }
    else if (pstStatusInd->enModifyStatus == MN_CALL_MODIFY_PROC_END)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s%s %d,%d,%d%s",
                                           gaucAtCrLf,
                                           gastAtStringTab[AT_STRING_CALL_MODIFY_END].pucText,
                                           pstStatusInd->ucCallId,
                                           pstStatusInd->enVoiceDomain,
                                           pstStatusInd->enCause,
                                           gaucAtCrLf);
    }
    else
    {
        return VOS_ERR;
    }

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafCcmEconfDialCnf(VOS_VOID *pMsg)
{
    TAF_CCM_ECONF_DIAL_CNF_STRU        *pstEconfDialCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    ucIndex = 0;

    pstEconfDialCnf = (TAF_CCM_ECONF_DIAL_CNF_STRU *)pMsg;

    /* ͨ��ClientId��ȡucIndex */
    if ( At_ClientIdToUserId(pstEconfDialCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE )
    {
        AT_WARN_LOG("AT_RcvTafEconfDialCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* �㲥��Ϣ������ */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafEconfDialCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* �жϵ�ǰ�������� */
    if ((gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ECONF_DIAL_SET)
     && (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CACMIMS_SET))
    {
        AT_WARN_LOG("AT_RcvTafEconfDialCnf: WARNING:Not AT_CMD_ECONF_DIAL_SET or AT_CMD_CACMIMS_SET!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �жϲ����Ƿ�ɹ� */
    if (pstEconfDialCnf->stEconfDialCnf.enCause != TAF_CS_CAUSE_SUCCESS)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmEconfNotifyInd(VOS_VOID *pMsg)
{
    errno_t                             lMemResult;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    TAF_CCM_ECONF_NOTIFY_IND_STRU      *pstNotifyInd = VOS_NULL_PTR;
    AT_MODEM_CC_CTX_STRU               *pstCcCtx     = VOS_NULL_PTR;

    usLength     = 0;
    pstNotifyInd = (TAF_CCM_ECONF_NOTIFY_IND_STRU *)pMsg;

    /* ͨ��clientid��ȡindex */
    if (At_ClientIdToUserId(pstNotifyInd->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafEconfNotifyInd:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(pstNotifyInd->stCtrl.usClientId);

    if (pstNotifyInd->stEconfNotifyIndPara.ucNumOfCalls > TAF_CALL_MAX_ECONF_CALLED_NUM)
    {
        pstCcCtx->stEconfInfo.ucNumOfCalls   = TAF_CALL_MAX_ECONF_CALLED_NUM;
    }
    else
    {
        pstCcCtx->stEconfInfo.ucNumOfCalls   = pstNotifyInd->stEconfNotifyIndPara.ucNumOfCalls;
    }

    lMemResult = memcpy_s(pstCcCtx->stEconfInfo.astCallInfo,
                          sizeof(pstCcCtx->stEconfInfo.astCallInfo),
                          pstNotifyInd->stEconfNotifyIndPara.astCallInfo,
                          (sizeof(TAF_CALL_ECONF_INFO_PARAM_STRU) * pstCcCtx->stEconfInfo.ucNumOfCalls));
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(pstCcCtx->stEconfInfo.astCallInfo), (sizeof(TAF_CALL_ECONF_INFO_PARAM_STRU) * pstCcCtx->stEconfInfo.ucNumOfCalls));

    /* call_numȡpstNotifyInd->ucNumOfCalls��������pstCcCtx->stEconfInfo.ucNumOfCalls�����Է��㷢�ִ��� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s %d%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_ECONFSTATE].pucText,
                                       pstNotifyInd->stEconfNotifyIndPara.ucNumOfCalls,
                                       gaucAtCrLf);

    gstAtSendData.usBufLen = usLength;

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCcwaiSetCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_CCWAI_SET_CNF_STRU         *pstCcwaiCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;

    pstCcwaiCnf = (TAF_CCM_CCWAI_SET_CNF_STRU *)pMsg;

    /* ����ClientID��ȡͨ������ */
    if(At_ClientIdToUserId(pstCcwaiCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallCcwaiSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCcwaiSetCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�^CCWAI����Ĳ�������¼��ϱ� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCWAI_SET)
    {
        AT_WARN_LOG("AT_RcvTafCallCcwaiSetCnf: WARNING:Not AT_CMD_CCWAI_SET!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �ж����ò����Ƿ�ɹ� */
    if (pstCcwaiCnf->ulResult == VOS_OK)
    {
        ulResult = AT_OK;
    }
    else
    {
        ulResult = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif


VOS_UINT32 AT_RcvTafPsEvtSetImsPdpCfgCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_IMS_PDP_CFG_CNF_STRU  *pstSetImsPdpCfgCnf = VOS_NULL_PTR;

    pstSetImsPdpCfgCnf = (TAF_PS_SET_IMS_PDP_CFG_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_IMSPDPCFG_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetImsPdpCfgCnf->enCause);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_RcvTafPsEvtSet1xDormTimerCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_1X_DORM_TIMER_CNF_STRU  *pstSet1xDormTimerCnf = VOS_NULL_PTR;

    pstSet1xDormTimerCnf = (TAF_PS_SET_1X_DORM_TIMER_CNF_STRU* )pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DORMTIMER_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSet1xDormTimerCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGet1xDormTimerCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_1X_DORM_TIMER_CNF_STRU  *pstGet1xDormTiCnf = VOS_NULL_PTR;
    VOS_UINT16                          usLength;

    pstGet1xDormTiCnf = (TAF_PS_GET_1X_DORM_TIMER_CNF_STRU *)pEvtInfo;
    usLength         = 0;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DORMTIMER_QRY)
    {
        return VOS_ERR;
    }

    /* �ϱ���ѯ��� */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstGet1xDormTiCnf->ucSocmDormTiVal,
                                      pstGet1xDormTiCnf->ucUserCfgDormTival);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}
#endif



VOS_UINT32 AT_RcvTafPsCallEvtLimitPdpActInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_CALL_LIMIT_PDP_ACT_IND_STRU *pstLimitPdpActInd = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16                usModemId;
    VOS_UINT16                          usLength;

    usLength                = 0;
    usModemId               = MODEM_ID_0;
    pstLimitPdpActInd       = (TAF_PS_CALL_LIMIT_PDP_ACT_IND_STRU *)pEvtInfo;

    if (AT_GetModemIdFromClient(ucIndex, &usModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_RcvTafPsCallEvtLimitPdpActInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* ^LIMITPDPACT: <FLG>,<CAUSE><CR><LF> */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                      "%s%s: %d,%d%s",
                                      gaucAtCrLf,
                                      gastAtStringTab[AT_STRING_LIMITPDPACT].pucText,
                                      pstLimitPdpActInd->ucLimitFlg,
                                      pstLimitPdpActInd->enCause,
                                      gaucAtCrLf);

    gstAtSendData.usBufLen = usLength;

    /* ����At_SendResultData���������� */
    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;

}



VOS_UINT32 AT_RcvTafPsEvtSetMipModeCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_MIP_MODE_CNF_STRU       *pstSetMipModeCnf = VOS_NULL_PTR;

    pstSetMipModeCnf = (TAF_PS_SET_MIP_MODE_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMIP_SET )
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetMipModeCnf->enCause);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetMipModeCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_MIP_MODE_CNF_STRU       *pstGetMipModeCnf = VOS_NULL_PTR;

    pstGetMipModeCnf = (TAF_PS_GET_MIP_MODE_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CMIP_QRY)
    {
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �ϱ���ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     "%s: %d",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     pstGetMipModeCnf->enMipMode);

    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtSetVzwApneCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_VZWAPNE_CNF_STRU        *pstSetVzwApneCnf = VOS_NULL_PTR;

    pstSetVzwApneCnf = (TAF_PS_SET_VZWAPNE_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_VZWAPNE_SET)
    {
        AT_WARN_LOG("AT_RcvTafPsEvtSetVzwApneCnf: WARNING:NOT VZWAPNE_SET OPTION!");
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetVzwApneCnf->enCause);

    return VOS_OK;
}


VOS_CHAR *AT_GetPdpTypeStr(
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            return (VOS_CHAR *)gastAtStringTab[AT_STRING_IPv4].pucText;
        case TAF_PDP_IPV6:
            return (VOS_CHAR *)gastAtStringTab[AT_STRING_IPv6].pucText;
        case TAF_PDP_IPV4V6:
            return (VOS_CHAR *)gastAtStringTab[AT_STRING_IPv4v6].pucText;
        default:
            AT_WARN_LOG1("AT_GetPdpTypeStr: Invalid PDP Type!", enPdpType);
            break;
    }

    return (VOS_CHAR *)gastAtStringTab[AT_STRING_IP].pucText;
}


VOS_VOID AT_PrintVzwApneResult(
    VOS_UINT8                           ucIndex,
    TAF_PS_GET_VZWAPNE_CNF_STRU        *pstGetVzwApneCnf
)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulLoop;
    VOS_CHAR                           *pcSplitStr  = VOS_NULL_PTR;
    VOS_CHAR                           *pcIPTypeStr = VOS_NULL_PTR;
    VOS_CHAR                           *pcApnebStr  = VOS_NULL_PTR;
    VOS_CHAR                            acApnStr[TAF_MAX_APN_LEN + 1];
    VOS_UINT8                           ucApnStrLen;
    VOS_UINT8                           ucMaxNum;
    VOS_UINT8                           ucIsCustomCmd;

    gstAtSendData.usBufLen = 0;

    /* ��ѯʧ�ܴ��� */
    if (pstGetVzwApneCnf->enCause != TAF_PS_CAUSE_SUCCESS)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
        return;
    }

    ucMaxNum            = (pstGetVzwApneCnf->ucNum > TAF_PS_MAX_VZWAPNE_NUM) ?
                                TAF_PS_MAX_VZWAPNE_NUM : pstGetVzwApneCnf->ucNum;

    /* +VZWAPNE������ */
    if (g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex == AT_CMD_VZWAPNE)
    {
        ucIsCustomCmd   = VOS_TRUE;
        pcSplitStr      = ",";
    }
    else
    {
        ucIsCustomCmd   = VOS_FALSE;
        pcSplitStr      = (VOS_CHAR*)gaucAtCrLf;
    }

    for (ulLoop = 0; ulLoop < ucMaxNum; ulLoop++)
    {
        /* ��ӡ+VZWAPNE: /^VZWAPNE: */
        if ((ulLoop == 0) || (ucIsCustomCmd == VOS_FALSE))
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                             "%s: ",
                                                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        }

        /* ��ӡ<cid> */
        if (ucIsCustomCmd == VOS_FALSE)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                             "%d,",
                                                             pstGetVzwApneCnf->astVzwApneInfo[ulLoop].ucCid);
        }

        /* ��ӡ<apncl>,<apnni>,<apntype>,<apnb>,<apned>,<apntime> */
        ucApnStrLen = (pstGetVzwApneCnf->astVzwApneInfo[ulLoop].stApn.ucLength > TAF_MAX_APN_LEN) ?
                        TAF_MAX_APN_LEN : pstGetVzwApneCnf->astVzwApneInfo[ulLoop].stApn.ucLength;

        memset_s(acApnStr, sizeof(acApnStr), 0, sizeof(acApnStr));
        lMemResult = memcpy_s(acApnStr, sizeof(acApnStr), pstGetVzwApneCnf->astVzwApneInfo[ulLoop].stApn.aucValue, ucApnStrLen);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(acApnStr), ucApnStrLen);

        pcIPTypeStr = AT_GetPdpTypeStr(pstGetVzwApneCnf->astVzwApneInfo[ulLoop].enIpType);
        pcApnebStr  = (VOS_TRUE == pstGetVzwApneCnf->astVzwApneInfo[ulLoop].ucEnable) ? "Enabled" : "Disabled";

        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr,
                                                         (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                         "%d,\"%s\",%s,\"LTE\",\"%s\",%d",
                                                         pstGetVzwApneCnf->astVzwApneInfo[ulLoop].ucClass,
                                                         acApnStr,
                                                         pcIPTypeStr,
                                                         pcApnebStr,
                                                         pstGetVzwApneCnf->astVzwApneInfo[ulLoop].usInactiveTimer);

        /* ��ӡ�ָ��� */
        if (ulLoop + 1 < ucMaxNum)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                                             (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                             "%s",
                                                             pcSplitStr);
        }

    }

    /* �����ATͨ�� */
    At_FormatResultData(ucIndex, AT_OK);

    return;

}


VOS_UINT32 AT_RcvTafPsEvtGetVzwApneCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_GET_VZWAPNE_CNF_STRU        *pstGetVzwApneCnf = VOS_NULL_PTR;

    pstGetVzwApneCnf    = (TAF_PS_GET_VZWAPNE_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰ����Ĳ������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_VZWAPNE_QRY)
    {
        AT_WARN_LOG("AT_RcvTafPsEvtGetVzwApneCnf: WARNING:NOT VZWAPNE_QRY OPTION!");
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    AT_PrintVzwApneResult(ucIndex, pstGetVzwApneCnf);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsReportPcoInfoInd(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_REPORT_PCO_INFO_IND_STRU    *pstPcoInfoInd = VOS_NULL_PTR;

    pstPcoInfoInd  = (TAF_PS_REPORT_PCO_INFO_IND_STRU *)pEvtInfo;

    if (pstPcoInfoInd->stCustomPcoInfo.ulContainerNum > 0)
    {
        AT_PS_ReportCustomPcoInfo(&pstPcoInfoInd->stCustomPcoInfo,
                                  pstPcoInfoInd->enOperateType,
                                  pstPcoInfoInd->ucCid,
                                  pstPcoInfoInd->enPdpType,
                                  ucIndex);
    }

    return VOS_OK;
}





VOS_UINT32 AT_RcvTafPsEvtSetDataSwitchCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{

    TAF_PS_SET_DATA_SWITCH_CNF_STRU*       pstSetDataSwitchCnf = VOS_NULL_PTR;
    pstSetDataSwitchCnf = (TAF_PS_SET_DATA_SWITCH_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DATASWITCH_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetDataSwitchCnf->enCause);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtGetDataSwitchCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{

    TAF_PS_GET_DATA_SWITCH_CNF_STRU*       pstGetDataSwitchCnf = VOS_NULL_PTR;
    pstGetDataSwitchCnf = (TAF_PS_GET_DATA_SWITCH_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DATASWITCH_QRY)
    {
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �ϱ���ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     "%s: %d",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     pstGetDataSwitchCnf->ucDataSwitch);

    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetDataRoamSwitchCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{

    TAF_PS_SET_DATA_ROAM_SWITCH_CNF_STRU*       pstSetDataRoamSwitchCnf = VOS_NULL_PTR;
    pstSetDataRoamSwitchCnf = (TAF_PS_SET_DATA_ROAM_SWITCH_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DATAROAMSWITCH_SET)
    {
        return VOS_ERR;
    }

    /* ��������� */
    AT_PrcoPsEvtErrCode(ucIndex, pstSetDataRoamSwitchCnf->enCause);
    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtGetDataRoamSwitchCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{

    TAF_PS_GET_DATA_ROAM_SWITCH_CNF_STRU*       pstGetDataSwitchCnf = VOS_NULL_PTR;
    pstGetDataSwitchCnf = (TAF_PS_GET_DATA_ROAM_SWITCH_CNF_STRU *)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_DATAROAMSWITCH_QRY)
    {
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* �ϱ���ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     "%s: %d",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     pstGetDataSwitchCnf->ucDataRoamSwitch);

    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtSetApnThrotInfoCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_APN_THROT_INFO_CNF_STRU       *pstSetApnThrotInfoCnf = VOS_NULL_PTR;

    pstSetApnThrotInfoCnf = (TAF_PS_SET_APN_THROT_INFO_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_APN_THROT_INFO_SET)
    {
        AT_WARN_LOG("AT_RcvTafPsEvtSetApnThrotInfoCnf: WARNING: CmdCurrentOpt != AT_CMD_APN_THROT_INFO_SET!");

        return VOS_ERR;
    }

    /* ��������� �ϱ����*/
    AT_PrcoPsEvtErrCode(ucIndex, pstSetApnThrotInfoCnf->enCause);

    return VOS_OK;
}



VOS_UINT32 AT_RcvTafPsEvtSetRoamPdpTypeCnf(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pEvtInfo
)
{
    TAF_PS_SET_ROAMING_PDP_TYPE_CNF_STRU                   *pstSetPdpTypeCnf = VOS_NULL_PTR;

    pstSetPdpTypeCnf = (TAF_PS_SET_ROAMING_PDP_TYPE_CNF_STRU*)pEvtInfo;

    /* ��鵱ǰAT�������� */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ROAMPDPTYPE_SET)
    {
        AT_WARN_LOG("AT_RcvTafPsEvtSetRoamPdpTypeCnf: WARNING: CmdCurrentOpt != AT_CMD_ROAMPDPTYPE_SET!");
        return VOS_ERR;
    }

    /* ��������� �ϱ����*/
    AT_PrcoPsEvtErrCode(ucIndex, pstSetPdpTypeCnf->enCause);

    return VOS_OK;
}

