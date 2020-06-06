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

#ifndef _TAF_CCM_API_H_
#define _TAF_CCM_API_H_

#include "TafTypeDef.h"
#include "MnCallApi.h"
#include "TafApi.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define    ID_TAF_CCM_MSG_TYPE_BEGIN   0


/*****************************************************************************
  3 ��Ϣ����
*****************************************************************************/
/*****************************************************************************
 ö����    : TAF_CCM_MSG_TYPE_ENUM
 �ṹ˵��  : CCM��Ϣ�ӿ�ö��
*****************************************************************************/
enum TAF_CCM_MSG_TYPE_ENUM
{
    /* CCM->AT */
    ID_TAF_CCM_QRY_CHANNEL_INFO_CNF                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x01,   /* _H2ASN_MsgChoice TAF_CCM_QRY_CHANNEL_INFO_REQ_STRU */
    ID_TAF_CCM_CALL_CHANNEL_INFO_IND                       = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x03,   /* _H2ASN_MsgChoice TAF_CCM_CALL_CHANNEL_INFO_IND_STRU *//* �ŵ���Ϣ�ϱ� */
    ID_TAF_CCM_CALL_MODIFY_CNF                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x05,   /* _H2ASN_MsgChoice TAF_CCM_CALL_MODIFY_CNF_STRU */
    ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_CNF               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x07,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_CNF_STRU */
    ID_TAF_CCM_CALL_MODIFY_STATUS_IND                      = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x09,   /* _H2ASN_MsgChoice TAF_CCM_CALL_MODIFY_STATUS_IND_STRU */
    ID_TAF_CCM_QRY_ECONF_CALLED_INFO_CNF                   = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0B,   /* _H2ASN_MsgChoice TAF_CCM_QRY_ECONF_CALLED_INFO_CNF_STRU */
    ID_TAF_CCM_CALL_PRIVACY_MODE_IND                       = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0D,   /* _H2ASN_MsgChoice TAF_CCM_CALL_PRIVACY_MODE_IND_STRU */
    ID_TAF_CCM_CALL_ORIG_CNF                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0F,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ORIG_CNF_STRU */
    ID_TAF_CCM_CALL_SUPS_CMD_CNF                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x11,   /* _H2ASN_MsgChoice TAF_CCM_CALL_SUPS_CMD_CNF_STRU */
    ID_TAF_CCM_CALL_SUPS_CMD_RSLT_IND                      = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x13,   /* _H2ASN_MsgChoice TAF_CCM_CALL_SUPS_CMD_RSLT_STRU *//* Call Related Command is Completed */
    ID_TAF_CCM_CALL_ORIG_IND                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x15,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ORIG_IND_STRU *//* originate a MO Call */
    ID_TAF_CCM_CALL_CONNECT_IND                            = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x17,   /* _H2ASN_MsgChoice TAF_CCM_CALL_CONNECT_IND_STRU *//* Call Connect */
    ID_TAF_CCM_CALL_INCOMING_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x19,   /* _H2ASN_MsgChoice TAF_CCM_CALL_INCOMING_IND_STRU *//* incoming call */
    ID_TAF_CCM_CALL_RELEASED_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1B,   /* _H2ASN_MsgChoice TAF_CCM_CALL_RELEASED_IND_STRU *//* Call Released */
    ID_TAF_CCM_CALL_ALL_RELEASED_IND                       = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1D,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ALL_RELEASED_IND_STRU *//* Call All Released */
    ID_TAF_CCM_QRY_CALL_INFO_CNF                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1F,   /* _H2ASN_MsgChoice TAF_CCM_QRY_CALL_INFO_CNF_STRU *//* ��ǰ���к��е���Ϣ */
    ID_TAF_CCM_QRY_CLPR_CNF                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x21,   /* _H2ASN_MsgChoice TAF_CCM_QRY_CLPR_CNF_STRU */
    ID_TAF_CCM_QRY_XLEMA_CNF                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x23,   /* _H2ASN_MsgChoice TAF_CCM_QRY_XLEMA_CNF_STRU */
    ID_TAF_CCM_QRY_ECALL_INFO_CNF                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x25,   /* _H2ASN_MsgChoice TAF_CCM_QRY_ECALL_INFO_CNF_STRU */
    ID_TAF_CCM_SET_CSSN_CNF                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x27,   /* _H2ASN_MsgChoice TAF_CCM_SET_CSSN_CNF_STRU */
    ID_TAF_CCM_START_DTMF_CNF                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x29,   /* _H2ASN_MsgChoice TAF_CCM_START_DTMF_CNF_STRU *//* Start DTMF tempooralily response */
    ID_TAF_CCM_START_DTMF_RSLT                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2B,   /* _H2ASN_MsgChoice TAF_CCM_START_DTMF_RSLT_STRU */
    ID_TAF_CCM_STOP_DTMF_CNF                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2D,   /* _H2ASN_MsgChoice TAF_CCM_STOP_DTMF_CNF_STRU *//* Stop DTMF tempooralily response */
    ID_TAF_CCM_STOP_DTMF_RSLT                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2F,   /* _H2ASN_MsgChoice TAF_CCM_STOP_DTMF_RSLT_STRU */
    ID_TAF_CCM_CALL_PROC_IND                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x31,   /* _H2ASN_MsgChoice TAF_CCM_CALL_PROC_IND_STRU *//* Call is Proceeding */
    ID_TAF_CCM_CALL_ALERTING_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x33,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ALERTING_IND_STRU *//* Alerting,MO Call */
    ID_TAF_CCM_CALL_HOLD_IND                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x35,   /* _H2ASN_MsgChoice TAF_CCM_CALL_HOLD_IND_STRU *//* Call Hold ���б��� */
    ID_TAF_CCM_CALL_RETRIEVE_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x37,   /* _H2ASN_MsgChoice TAF_CCM_CALL_RETRIEVE_IND_STRU *//* Call Retrieve ���лָ� */
    ID_TAF_CCM_CALL_SS_IND                                 = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x39,   /* _H2ASN_MsgChoice TAF_CCM_CALL_SS_IND_STRU *//* SS Notify */
    ID_TAF_CCM_ECC_NUM_IND                                 = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3B,   /* _H2ASN_MsgChoice TAF_CCM_ECC_NUM_IND_STRU */
    ID_TAF_CCM_GET_CDUR_CNF                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3D,   /* _H2ASN_MsgChoice TAF_CCM_GET_CDUR_CNF_STRU *//* ͨ��ʱ�� */
    ID_TAF_CCM_SET_UUS1_INFO_CNF                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3F,   /* _H2ASN_MsgChoice TAF_CCM_SET_UUS1_INFO_CNF_STRU *//* �����û���������1��Ϣ */
    ID_TAF_CCM_QRY_UUS1_INFO_CNF                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x41,   /* _H2ASN_MsgChoice TAF_CCM_QRY_UUS1_INFO_CNF_STRU *//* ��ѯ�û���������1��Ϣ */
    ID_TAF_CCM_UUS1_INFO_IND                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x43,   /* _H2ASN_MsgChoice TAF_CCM_UUS1_INFO_IND_STRU *//* UUS1��Ϣ�ϱ� */
    ID_TAF_CCM_SET_ALS_CNF                                 = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x45,   /* _H2ASN_MsgChoice TAF_CCM_SET_ALS_CNF_STRU *//* ���õ�ǰ��·�� */
    ID_TAF_CCM_QRY_ALS_CNF                                 = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x47,   /* _H2ASN_MsgChoice TAF_CCM_QRY_ALS_CNF_STRU *//* ��ѯ��ǰ��·�� */
    ID_TAF_CCM_CCWAI_SET_CNF                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x49,   /* _H2ASN_MsgChoice TAF_CCM_CCWAI_SET_CNF_STRU */
    ID_TAF_CCM_CNAP_INFO_IND                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4B,   /* _H2ASN_MsgChoice TAF_CCM_CNAP_INFO_IND_STRU */
    ID_TAF_CCM_CNAP_QRY_CNF                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4D,   /* _H2ASN_MsgChoice TAF_CCM_CNAP_QRY_CNF_STRU */
    ID_TAF_CCM_ECONF_DIAL_CNF                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4F,   /* _H2ASN_MsgChoice TAF_CCM_ECONF_DIAL_CNF_STRU */
    ID_TAF_CCM_ECONF_NOTIFY_IND                            = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x51,   /* _H2ASN_MsgChoice TAF_CCM_ECONF_NOTIFY_IND_STRU *//* imsa֪ͨSPMģ����ǿ�Ͷ෽ͨ�������ߵ�״̬���ͱ仯  */
    ID_TAF_CCM_SEND_FLASH_CNF                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x53,   /* _H2ASN_MsgChoice TAF_CCM_SEND_FLASH_CNF_STRU */
    ID_TAF_CCM_SEND_BURST_DTMF_CNF                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x55,   /* _H2ASN_MsgChoice TAF_CCM_SEND_BURST_DTMF_CNF_STRU */
    ID_TAF_CCM_SEND_BURST_DTMF_RSLT                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x57,   /* _H2ASN_MsgChoice TAF_CCM_SEND_BURST_DTMF_RSLT_STRU */
    ID_TAF_CCM_BURST_DTMF_IND                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x59,   /* _H2ASN_MsgChoice TAF_CCM_BURST_DTMF_IND_STRU */
    ID_TAF_CCM_SEND_CONT_DTMF_CNF                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x5B,   /* _H2ASN_MsgChoice TAF_CCM_SEND_CONT_DTMF_CNF_STRU */
    ID_TAF_CCM_SEND_CONT_DTMF_RSLT                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x5D,   /* _H2ASN_MsgChoice TAF_CCM_SEND_CONT_DTMF_RSLT_STRU */
    ID_TAF_CCM_CONT_DTMF_IND                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x5F,   /* _H2ASN_MsgChoice TAF_CCM_CONT_DTMF_IND_STRU */
    ID_TAF_CCM_CCWAC_INFO_IND                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x61,   /* _H2ASN_MsgChoice TAF_CCM_CCWAC_INFO_IND_STRU */
    ID_TAF_CCM_CALLED_NUM_INFO_IND                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x63,   /* _H2ASN_MsgChoice TAF_CCM_CALLED_NUM_INFO_IND_STRU */
    ID_TAF_CCM_CALLING_NUM_INFO_IND                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x65,   /* _H2ASN_MsgChoice TAF_CCM_CALLING_NUM_INFO_IND_STRU */
    ID_TAF_CCM_DISPLAY_INFO_IND                            = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x67,   /* _H2ASN_MsgChoice TAF_CCM_DISPLAY_INFO_IND_STRU */
    ID_TAF_CCM_EXT_DISPLAY_INFO_IND                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x69,   /* _H2ASN_MsgChoice TAF_CCM_EXT_DISPLAY_INFO_IND_STRU */
    ID_TAF_CCM_CONN_NUM_INFO_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x6B,   /* _H2ASN_MsgChoice TAF_CCM_CONN_NUM_INFO_IND_STRU */
    ID_TAF_CCM_REDIR_NUM_INFO_IND                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x6D,   /* _H2ASN_MsgChoice TAF_CCM_REDIR_NUM_INFO_IND_STRU */
    ID_TAF_CCM_SIGNAL_INFO_IND                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x6F,   /* _H2ASN_MsgChoice TAF_CCM_SIGNAL_INFO_IND_STRU */
    ID_TAF_CCM_LINE_CTRL_INFO_IND                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x71,   /* _H2ASN_MsgChoice TAF_CCM_LINE_CTRL_INFO_IND_STRU */
    ID_TAF_CCM_CALL_WAITING_IND                            = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x73,   /* _H2ASN_MsgChoice TAF_CCM_WAITING_IND_STRU */
    ID_TAF_CCM_ENCRYPT_VOICE_CNF                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x75,   /* _H2ASN_MsgChoice TAF_CCM_ENCRYPT_VOICE_CNF_STRU */
    ID_TAF_CCM_ENCRYPT_VOICE_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x77,   /* _H2ASN_MsgChoice TAF_CCM_ENCRYPT_VOICE_IND_STRU */
    ID_TAF_CCM_EC_REMOTE_CTRL_IND                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x79,   /* _H2ASN_MsgChoice TAF_CCM_EC_REMOTE_CTRL_IND_STRU */
    ID_TAF_CCM_REMOTE_CTRL_ANSWER_CNF                      = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x7B,   /* _H2ASN_MsgChoice TAF_CCM_REMOTE_CTRL_ANSWER_CNF_STRU */
    ID_TAF_CCM_ECC_SRV_CAP_CFG_CNF                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x7D,   /* _H2ASN_MsgChoice TAF_CCM_ECC_SRV_CAP_CFG_CNF_STRU */
    ID_TAF_CCM_ECC_SRV_CAP_QRY_CNF                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x7F,   /* _H2ASN_MsgChoice TAF_CCM_ECC_SRV_CAP_QRY_CNF_STRU */
    ID_TAF_CCM_SET_EC_TEST_MODE_CNF                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x81,   /* _H2ASN_MsgChoice TAF_CCM_SET_EC_TEST_MODE_CNF_STRU */
    ID_TAF_CCM_GET_EC_TEST_MODE_CNF                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x83,   /* _H2ASN_MsgChoice TAF_CCM_GET_EC_TEST_MODE_CNF_STRU */
    ID_TAF_CCM_GET_EC_RANDOM_CNF                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x85,   /* _H2ASN_MsgChoice TAF_CCM_GET_EC_RANDOM_CNF_STRU */
    ID_TAF_CCM_GET_EC_KMC_CNF                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x87,   /* _H2ASN_MsgChoice TAF_CCM_GET_EC_KMC_CNF_STRU */
    ID_TAF_CCM_SET_EC_KMC_CNF                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x89,   /* _H2ASN_MsgChoice TAF_CCM_SET_EC_KMC_CNF_STRU */
    ID_TAF_CCM_ENCRYPTED_VOICE_DATA_IND                    = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x8B,   /* _H2ASN_MsgChoice TAF_CCM_ENCRYPTED_VOICE_DATA_IND_STRU */
    ID_TAF_CCM_PRIVACY_MODE_SET_CNF                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x8D,   /* _H2ASN_MsgChoice TAF_CCM_PRIVACY_MODE_SET_CNF_STRU */
    ID_TAF_CCM_PRIVACY_MODE_QRY_CNF                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x8F,   /* _H2ASN_MsgChoice TAF_CCM_PRIVACY_MODE_QRY_CNF_STRU */
    ID_TAF_CCM_PRIVACY_MODE_IND                            = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x91,   /* _H2ASN_MsgChoice TAF_CCM_PRIVACY_MODE_IND_STRU */

    /* ����3����Ϣ��AT�����߼��������Է��� */
    ID_TAF_CCM_SS_CMD_PROGRESS_IND                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x93,   /* _H2ASN_MsgChoice TAF_CCM_SS_CMD_PROGRESS_IND_STRU *//* Call Related Command is in progress */
    ID_TAF_CCM_CCBS_POSSIBLE_IND                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x95,   /* _H2ASN_MsgChoice TAF_CCM_CCBS_POSSIBLE_IND_STRU *//* ���Լ���CCBS */
    ID_TAF_CCM_CCBS_TIME_EXPIRED_IND                       = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x97,   /* _H2ASN_MsgChoice TAF_CCM_CCBS_TIME_EXPIRED_IND_STRU *//* ��ʱ */


    /* AT->CCM */
    ID_TAF_CCM_QRY_CHANNEL_INFO_REQ                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x00,   /* _H2ASN_MsgChoice TAF_CCM_QRY_CHANNEL_INFO_REQ_STRU *//* CSCHANNELINFO��ѯ���� */
    ID_TAF_CCM_CALL_MODIFY_REQ                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x02,   /* _H2ASN_MsgChoice TAF_CCM_CALL_MODIFY_REQ_STRU *//* �������Ƶ���� */
    ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_REQ               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x04,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_REQ_STRU *//* Ӧ��Է��������Ƶ���� */
    ID_TAF_CCM_QRY_ECONF_CALLED_INFO_REQ                   = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x06,   /* _H2ASN_MsgChoice TAF_CCM_QRY_ECONF_CALLED_INFO_REQ_STRU *//* ��ȡ�������Ϣ */
    ID_TAF_CCM_CALL_ORIG_REQ                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x08,   /* _H2ASN_MsgChoice TAF_CCM_CALL_ORIG_REQ_STRU */     /* ������� */
    ID_TAF_CCM_CALL_END_REQ                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0A,   /* _H2ASN_MsgChoice TAF_CCM_CALL_END_REQ_STRU */      /* ���н�����Ϣ */
    ID_TAF_CCM_CALL_SUPS_CMD_REQ                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0C,   /* _H2ASN_MsgChoice TAF_CCM_CALL_SUPS_CMD_REQ_STRU */  /* ������ز���ҵ�� */
    ID_TAF_CCM_ECONF_DIAL_REQ                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x0E,   /* _H2ASN_MsgChoice TAF_CCM_ECONF_DIAL_REQ_STRU */     /* ��ǿ��ͨ������ */
    ID_TAF_CCM_CUSTOM_DIAL_REQ                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x10,   /* _H2ASN_MsgChoice TAF_CCM_CUSTOM_DIAL_REQ_STRU */    /* ��ǿ��ͨ������ */
    ID_TAF_CCM_QRY_CALL_INFO_REQ                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x12,   /* _H2ASN_MsgChoice TAF_CCM_QRY_CALL_INFO_REQ_STRU *//* ��ȡ������Ϣ */
    ID_TAF_CCM_QRY_CLPR_REQ                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x14,   /* _H2ASN_MsgChoice TAF_CCM_QRY_CLPR_REQ_STRU *//* ��ѯ����Դ���� */
    ID_TAF_CCM_QRY_XLEMA_REQ                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x16,   /* _H2ASN_MsgChoice TAF_CCM_QRY_XLEMA_REQ_STRU *//* �����������ѯ */ /* ��Ҫ����CCM�����𣬸о��ײ��߼��е�����Ӱ� */
    ID_TAF_CCM_QRY_ECALL_INFO_REQ                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x18,   /* _H2ASN_MsgChoice TAF_CCM_QRY_ECALL_INFO_REQ_STRU *//* ��ѯEcall��Ϣ */
    ID_TAF_CCM_SET_CSSN_REQ                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1A,   /* _H2ASN_MsgChoice TAF_CCM_SET_CSSN_REQ_STRU *//* ʹ�ܻ��ֹ����ҵ��������ϱ� */
    ID_TAF_CCM_CUSTOM_ECC_NUM_REQ                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1C,   /* _H2ASN_MsgChoice TAF_CCM_CUSTOM_ECC_NUM_REQ_STRU *//* ���ƽ��������� */
    ID_TAF_CCM_START_DTMF_REQ                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x1E,   /* _H2ASN_MsgChoice TAF_CCM_START_DTMF_REQ_STRU *//* ��ʼ����DTMF */
    ID_TAF_CCM_STOP_DTMF_REQ                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x20,   /* _H2ASN_MsgChoice TAF_CCM_STOP_DTMF_REQ_STRU *//* ֹͣ����DTMF */
    ID_TAF_CCM_GET_CDUR_REQ                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x22,   /* _H2ASN_MsgChoice TAF_CCM_GET_CDUR_REQ_STRU *//* ��ȡͨ��ʱ�� */
    ID_TAF_CCM_SET_UUSINFO_REQ                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x24,   /* _H2ASN_MsgChoice TAF_CCM_SET_UUSINFO_REQ_STRU *//* CUUS1���ֻ��GU CALLģ�鴦�� */
    ID_TAF_CCM_QRY_UUSINFO_REQ                             = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x26,   /* _H2ASN_MsgChoice TAF_CCM_QRY_UUSINFO_REQ_STRU */
    ID_TAF_CCM_QRY_ALS_REQ                                 = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x28,   /* _H2ASN_MsgChoice TAF_CCM_QRY_ALS_REQ_STRU */
    ID_TAF_CCM_SET_ALS_REQ                                 = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2A,   /* _H2ASN_MsgChoice TAF_CCM_SET_ALS_REQ_STRU *//* ����·ѡ��ALS���� */
    ID_TAF_CCM_CCWAI_SET_REQ                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2C,   /* _H2ASN_MsgChoice TAF_CCM_CCWAI_SET_REQ_STRU *//* ����IMS���еȴ� */
    ID_TAF_CCM_CNAP_QRY_REQ                                = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x2E,   /* _H2ASN_MsgChoice TAF_CCM_CNAP_QRY_REQ_STRU */
    ID_TAF_CCM_SEND_FLASH_REQ                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x30,   /* _H2ASN_MsgChoice TAF_CCM_SEND_FLASH_REQ_STRU *//* Xģ��Ϣ */
    ID_TAF_CCM_SEND_BURST_DTMF_REQ                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x32,   /* _H2ASN_MsgChoice TAF_CCM_SEND_BURST_DTMF_REQ_STRU *//* Xģ��Ϣ */
    ID_TAF_CCM_SEND_CONT_DTMF_REQ                          = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x34,   /* _H2ASN_MsgChoice TAF_CCM_SEND_CONT_DTMF_REQ_STRU *//* Xģ��Ϣ */
    ID_TAF_CCM_ENCRYPT_VOICE_REQ                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x36,   /* _H2ASN_MsgChoice TAF_CCM_ENCRYPT_VOICE_REQ_STRU */
    ID_TAF_CCM_REMOTE_CTRL_ANSWER_REQ                      = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x38,   /* _H2ASN_MsgChoice TAF_CCM_REMOTE_CTRL_ANSWER_REQ_STRU */
    ID_TAF_CCM_ECC_SRV_CAP_QRY_REQ                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3A,   /* _H2ASN_MsgChoice TAF_CCM_ECC_SRV_CAP_QRY_REQ_STRU */
    ID_TAF_CCM_ECC_SRV_CAP_CFG_REQ                         = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3C,   /* _H2ASN_MsgChoice TAF_CCM_ECC_SRV_CAP_CFG_REQ_STRU */
    ID_TAF_CCM_SET_EC_TEST_MODE_REQ                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x3E,   /* _H2ASN_MsgChoice TAF_CCM_SET_EC_TEST_MODE_REQ_STRU */
    ID_TAF_CCM_GET_EC_RANDOM_REQ                           = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x40,   /* _H2ASN_MsgChoice TAF_CCM_GET_EC_RANDOM_REQ_STRU */
    ID_TAF_CCM_GET_EC_TEST_MODE_REQ                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x42,   /* _H2ASN_MsgChoice TAF_CCM_GET_EC_TEST_MODE_REQ_STRU */
    ID_TAF_CCM_GET_EC_KMC_REQ                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x44,   /* _H2ASN_MsgChoice TAF_CCM_GET_EC_KMC_REQ_STRU */
    ID_TAF_CCM_SET_EC_KMC_REQ                              = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x46,   /* _H2ASN_MsgChoice TAF_CCM_SET_EC_KMC_REQ_STRU */
    ID_TAF_CCM_PRIVACY_MODE_SET_REQ                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x48,   /* _H2ASN_MsgChoice TAF_CCM_PRIVACY_MODE_SET_REQ_STRU */
    ID_TAF_CCM_PRIVACY_MODE_QRY_REQ                        = ID_TAF_CCM_MSG_TYPE_BEGIN + 0x4A,   /* _H2ASN_MsgChoice TAF_CCM_PRIVACY_MODE_QRY_REQ_STRU */

    ID_TAF_CCM_MSG_TYPE_BUTT                               = ID_TAF_CCM_MSG_TYPE_BEGIN + 0xFF,
};
typedef VOS_UINT32 TAF_CCM_MSG_TYPE_ENUM_UINT32;

/*****************************************************************************
 ö����    : MN_APP_REQ_MSG_STRU
 �ṹ˵��  : ����APP��������Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;                              /* ��Ϣ�� */
    TAF_CTRL_STRU                       stCtrl;                                 /* ������Ϣ */
    VOS_UINT8                           aucContent[0];                          /* ��Ϣ���� */
} TAF_CCM_APP_REQ_MSG_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          enMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_QRY_CHANNEL_INFO_REQ_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;                               /* ��ѯ��� */
    TAF_CALL_CHANNEL_TYPE_ENUM_UINT8    enChannelType;                          /* codec type */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucReserved[2];
} TAF_CCM_QRY_CHANNEL_INFO_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CCM_QRY_CHANNEL_INFO_PARA_STRU  stPara;
} TAF_CCM_QRY_CHANNEL_INFO_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT8                           ucIsLocalAlertingFlag;                  /* �Ƿ�Ϊ���ز��Ż�������ʶ1:���ز��Ż�������0:�������*/
    MN_CALL_CODEC_TYPE_ENUM_U8          enCodecType;                            /* codec type */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucRsv[1];
} TAF_CCM_CALL_CHANNEL_INFO_IND_STRU;


typedef struct
{
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_TYPE_ENUM_U8                enCurrCallType;                         /* ��ǰ�������� */
    MN_CALL_TYPE_ENUM_U8                enExpectCallType;                       /* ������������ */
    VOS_UINT8                           ucReserved;
} TAF_CCM_CALL_MODIFY_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CCM_CALL_MODIFY_PARA_STRU       stPara;
} TAF_CCM_CALL_MODIFY_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    VOS_UINT8                           aucReserved[3];
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* ������ */
} TAF_CCM_CALL_MODIFY_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ID_T                        ucCallId;                   /* Call ID */
    MN_CALL_MODIFY_STATUS_ENUM_UINT8    enModifyStatus;             /* ��ǰ��MODIFY�Ĺ���״̬ */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;              /* VOICE DOMAIN������ʼ����IMS�� */
    MN_CALL_TYPE_ENUM_U8                enCurrCallType;             /* ��ǰ�������� */
    MN_CALL_TYPE_ENUM_U8                enExpectCallType;           /* ������������ */
    TAF_CALL_MODIFY_REASON_ENUM_UINT8   enModifyReason;             /* Զ���û������modifyԭ�򣬽���MODIFY_PROC_INDʱ��ʹ�� */
    VOS_UINT8                           aucReserved[2];
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                    /* �����룬����MODIFY_PROC_END���쳣ʱ��ʹ�� */                               /* ������ */
}TAF_CCM_CALL_MODIFY_STATUS_IND_STRU;


typedef struct
{
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_TYPE_ENUM_U8                enCurrCallType;                         /* ��ǰ�������� */
    MN_CALL_TYPE_ENUM_U8                enExpectCallType;                       /* ������������ */
    VOS_UINT8                           ucReserved;
} TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_PARA_STRU             stPara;
} TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    VOS_UINT8                           aucReserved[3];
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* ������ */
} TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_QRY_ECONF_CALLED_INFO_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    VOS_UINT8                                               ucNumOfMaxCalls;
    VOS_UINT8                                               ucNumOfCalls;       /* ��������ͨ���ĸ��� */
    VOS_UINT8                                               aucReserved[2];
    TAF_CALL_ECONF_INFO_PARAM_STRU                          astCallInfo[TAF_CALL_MAX_ECONF_CALLED_NUM];
} TAF_CCM_QRY_ECONF_CALLED_INFO_CNF_STRU;


typedef struct
{
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_TYPE_ENUM_U8                enCurrCallType;                         /* ��ǰ�������� */
    MN_CALL_TYPE_ENUM_U8                enExpectCallType;                       /* ������������ */
    VOS_UINT8                           ucReserved;
} TAF_CCM_REMOTE_CTRL_ANSWER_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CCM_REMOTE_CTRL_ANSWER_PARA_STRU                    stPara;
} TAF_CCM_CALL_ANSWER_REMOTE_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    VOS_UINT8                           aucReserved[3];
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* ������ */
} TAF_CCM_CALL_ANSWER_REMOTE_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ORIG_PARAM_STRU             stOrig;
} TAF_CCM_CALL_ORIG_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_SUPS_PARAM_STRU             stCallMgmtCmd;
} TAF_CCM_CALL_SUPS_CMD_REQ_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_SUPS_CMD_PARA_STRU         stSupsCmdPara;
} TAF_CCM_CALL_SUPS_CMD_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ECONF_DIAL_INFO_STRU       stEconfDialInfo;
} TAF_CCM_ECONF_DIAL_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT32                                              ulMsgName;             /*_H2ASN_Skip*/
    VOS_UINT16                                              usClientId;
    VOS_UINT8                                               ucOpId;
    TAF_CALL_PRIVACY_MODE_ENUM_UINT8                        enPrivacyMode;         /* ��ǰprivacy mode���� */
    TAF_CALL_CALL_PRIVACY_MODE_INFO_STRU                    stCallVoicePrivacyInfo;/* ��ǰ���е�privacy mode��Ϣ */
} TAF_CCM_CALL_PRIVACY_MODE_IND_STRU;


typedef struct
{
    MN_CALL_ID_T                        callId;
    VOS_UINT8                           aucReservd[3];
    MN_CALL_END_PARAM_STRU              stEndParam;            /* End Cause */
}TAF_CCM_CALL_END_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER                                             /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;             /*_H2ASN_Skip*/
    TAF_CTRL_STRU                       stCtrl;
    TAF_CCM_CALL_END_INFO_STRU          stEndInfo;
}TAF_CCM_CALL_END_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER                                             /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;             /*_H2ASN_Skip*/
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CUSTOM_DIAL_PARA_STRU      stCustomDialPara;
}TAF_CCM_CUSTOM_DIAL_REQ_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                               ulMsgName;
    TAF_CTRL_STRU                            stCtrl;
    TAF_CALL_QRY_CALL_INFO_REQ_PARA_STRU     stQryCallInfoPara;
} TAF_CCM_QRY_CALL_INFO_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                              ulMsgName;
    TAF_CTRL_STRU                           stCtrl;
    TAF_CALL_QRY_CALL_INFO_CNF_PARA_STRU    stQryCallInfoPara;
} TAF_CCM_QRY_CALL_INFO_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                              ulMsgName;
    TAF_CTRL_STRU                           stCtrl;
    TAFAGENT_CALL_QRY_CALL_INFO_PARA_STRU   stTafAgentCallInfo;
} TAFAGENT_CCM_QRY_CALL_INFO_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_QRY_CLPR_PARA_STRU         stQryClprPara;
} TAF_CCM_QRY_CLPR_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_CLPR_GET_CNF_STRU           stClprCnf;
} TAF_CCM_QRY_CLPR_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_QRY_XLEMA_REQ_STRU;

typedef  MN_CALL_ECC_NUM_INFO_STRU TAF_CALL_QRY_XLEMA_PARA_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_QRY_XLEMA_PARA_STRU        stQryXlemaPara;
} TAF_CCM_QRY_XLEMA_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_QRY_ECALL_INFO_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ECALL_INFO_STRU             stEcallInfo;
} TAF_CCM_QRY_ECALL_INFO_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU stEccNumReq;
} TAF_CCM_CUSTOM_ECC_NUM_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_SET_CSSN_REQ_STRU           stCssnReq;
} TAF_CCM_SET_CSSN_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_ERROR_CODE_ENUM_UINT32          ulRet;          /*�ظ����*/
} TAF_CCM_SET_CSSN_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ORIG_CNF_PARA_STRU         stOrigCnfPara;
} TAF_CCM_CALL_ORIG_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_SUPS_CMD_RSLT_PARA_STRU    stSupsCmdRsltPara;
} TAF_CCM_CALL_SUPS_CMD_RSLT_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ORIG_IND_PARA_STRU         stOrigIndPara;
} TAF_CCM_CALL_ORIG_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_PROC_IND_PARA_STRU         stProcIndPata;
} TAF_CCM_CALL_PROC_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ALERTING_IND_PARA_STRU     stAlertingIndPara;
} TAF_CCM_CALL_ALERTING_IND_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CONNECT_IND_PARA_STRU      stConnectIndPara;
} TAF_CCM_CALL_CONNECT_IND_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_RELEASED_IND_PARA_STRU     stReleaseIndPara;
} TAF_CCM_CALL_RELEASED_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_INCOMING_IND_PARA_STRU     stIncomingIndPara;
} TAF_CCM_CALL_INCOMING_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_CALL_ALL_RELEASED_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_DTMF_PARAM_STRU            stDtmf;
} TAF_CCM_START_DTMF_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_START_DTMF_PARA_STRU       stStartDtmfPara;
} TAF_CCM_START_DTMF_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_START_DTMF_RSLT_PARA_STRU  stStartDtmfRslt;
} TAF_CCM_START_DTMF_RSLT_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_DTMF_PARAM_STRU            stDtmf;
} TAF_CCM_STOP_DTMF_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_STOP_DTMF_PARA_STRU        stStopDtmfPara;
} TAF_CCM_STOP_DTMF_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_STOP_DTMF_RSLT_PARA_STRU   stStopDtmfRsltPara;
} TAF_CCM_STOP_DTMF_RSLT_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_HOLD_IND_PARA_STRU         stHoldIndPara;
} TAF_CCM_CALL_HOLD_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_RETRIEVE_IND_STRU          stRetrieveInd;
} TAF_CCM_CALL_RETRIEVE_IND_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_SS_IND_PARA_STRU           stSsIndPara;
} TAF_CCM_CALL_SS_IND_STRU;

typedef MN_CALL_ECC_NUM_INFO_STRU TAF_CALL_ECC_NUM_IND_PARA_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ECC_NUM_IND_PARA_STRU      stEccNumInd;
} TAF_CCM_ECC_NUM_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_UUS1_PARAM_STRU             stUus1Info;   /* ����UUS1��Ϣ�ṹ */
} TAF_CCM_SET_UUSINFO_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulRet;      /*�ظ����*/
} TAF_CCM_SET_UUS1_INFO_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_QRY_UUSINFO_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_QRY_UUS1_INFO_PARA_STRU    stQryUss1Info;
} TAF_CCM_QRY_UUS1_INFO_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_UUS1_INFO_IND_PARA_STRU    stUus1InfoIndPara;
} TAF_CCM_UUS1_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_SET_ALS_PARAM_STRU          stSetAls;     /* ����ALS NO�ṹ   */
} TAF_CCM_SET_ALS_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulRet;      /*�ظ����*/
} TAF_CCM_SET_ALS_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_QRY_ALS_REQ_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_QRY_ALS_PARA_STRU          stQryAlsPara;
} TAF_CCM_QRY_ALS_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CCWAI_SET_REQ_STRU         stCcwaiSet;
} TAF_CCM_CCWAI_SET_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulResult;
} TAF_CCM_CCWAI_SET_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CNAP_STRU                  stNameIndicator;
} TAF_CCM_CNAP_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
} TAF_CCM_CNAP_QRY_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CNAP_STRU                  stNameIndicator;
} TAF_CCM_CNAP_QRY_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ECONF_DIAL_PARA_STRU       stEconfDialCnf;
} TAF_CCM_ECONF_DIAL_CNF_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_ECONF_NOTIFY_IND_PARA_STRU stEconfNotifyIndPara;
} TAF_CCM_ECONF_NOTIFY_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_FLASH_PARA_STRU            stFlashPara;
} TAF_CCM_SEND_FLASH_REQ_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_FLASH_CNF_PARA_STRU        stResult;
} TAF_CCM_SEND_FLASH_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_BURST_DTMF_PARA_STRU       stBurstDTMFPara;
} TAF_CCM_SEND_BURST_DTMF_REQ_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_BURST_DTMF_CNF_PARA_STRU                       stBurstDtmfCnfPara;
} TAF_CCM_SEND_BURST_DTMF_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT8                           ucResult;
    VOS_UINT8                           aucReserved[3];
} TAF_CCM_SEND_BURST_DTMF_RSLT_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_BURST_DTMF_IND_PARA_STRU   stBurstDtmfIndPara;                         /**<  refers to S.0005 3.7.3.3.2.9 */
} TAF_CCM_BURST_DTMF_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CONT_DTMF_PARA_STRU        stContDTMFPara;
} TAF_CCM_SEND_CONT_DTMF_REQ_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_CONT_DTMF_CNF_PARA_STRU                        stContDtmfCnfPara;
} TAF_CCM_SEND_CONT_DTMF_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT8                           ucResult;
    VOS_UINT8                           aucReserved[3];
} TAF_CCM_SEND_CONT_DTMF_RSLT_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CTRL_CONT_DTMF_IND_PARA_STRU    stContDtmfIndPara;
} TAF_CCM_CONT_DTMF_IND_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CCWAC_INFO_IND_PARA_STRU   stCcwacInfoPara;
} TAF_CCM_CCWAC_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                              ulMsgName;
    TAF_CTRL_STRU                           stCtrl;
    TAF_CALL_CALLED_NUM_INFO_IND_PARA_STRU  stCalledNumInfoPara;
}TAF_CCM_CALLED_NUM_INFO_IND_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_CALLING_NUM_INFO_IND_PARA_STRU                 stCallIngNumInfoPara;
}TAF_CCM_CALLING_NUM_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_DISPLAY_INFO_IND_PARA_STRU stDisPlayInfoIndPara;
}TAF_CCM_DISPLAY_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_EXT_DISPLAY_INFO_IND_PARA_STRU                 stDisPlayInfoIndPara;
}TAF_CCM_EXT_DISPLAY_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_CONN_NUM_INFO_IND_PARA_STRU                    stConnNumInfoIndPara;
}TAF_CCM_CONN_NUM_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_REDIR_NUM_INFO_IND_PARA_STRU    stRedirNumInfoIndPara;
}TAF_CCM_REDIR_NUM_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_SIGNAL_INFO_IND_PARA_STRU  stSignalInfoIndPara;
}TAF_CCM_SIGNAL_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_LINE_CTRL_INFO_IND_PARA_STRU                   stLineCtrlInfoIndPara;
}TAF_CCM_LINE_CTRL_INFO_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucCallWaitingInd;
    VOS_UINT8                           aucRsved[2];
}TAF_CCM_WAITING_IND_STRU;



typedef struct
{
    TAF_CALL_ENCRYPT_VOICE_TYPE_ENUM_UINT32                 enEccVoiceType;
    TAF_ECC_CALL_BCD_NUM_STRU                               stDialNumber;          /* Call Number */
} TAF_CCM_ENCRYPT_VOICE_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CCM_ENCRYPT_VOICE_PARA_STRU                         stEncrypVoicePara;
} TAF_CCM_ENCRYPT_VOICE_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_ENCRYPT_VOICE_STATUS_ENUM_UINT32               enEccVoiceStatus;
} TAF_CCM_ENCRYPT_VOICE_CNF_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_ENCRYPT_VOICE_IND_PARA_STRU                    stEncryptVoiceIndPara;
} TAF_CCM_ENCRYPT_VOICE_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_REMOTE_CTRL_TYPE_ENUM_UINT32                   enRemoteCtrlType;
} TAF_CCM_EC_REMOTE_CTRL_IND_STRU;


typedef struct
{
    TAF_CALL_REMOTE_CTRL_TYPE_ENUM_UINT32                   enRemoteCtrlEvtType;
    TAF_CALL_REMOTE_CTRL_RESULT_ENUM_UINT32                 enResult;
} TAF_CCM_REMOTE_CTRL_ANSWER_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CCM_REMOTE_CTRL_ANSWER_INFO_STRU                    stRemoteCtrlAnswerInfo;
} TAF_CCM_REMOTE_CTRL_ANSWER_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_REMOTE_CTRL_OPER_RESULT_ENUM_UINT32            enResult;              /* ���ŷ��ͽ�� */
} TAF_CCM_REMOTE_CTRL_ANSWER_CNF_STRU;



typedef struct
{
    TAF_CALL_ECC_SRV_CAP_ENUM_UINT32                        enEccSrvCap;
    TAF_CALL_ECC_SRV_STATUS_ENUM_UINT32                     enEccSrvStatus;
} TAF_CCM_ECC_SRV_CAP_CFG_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CCM_ECC_SRV_CAP_CFG_PARA_STRU                       stCapCfgPara;
} TAF_CCM_ECC_SRV_CAP_CFG_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_ECC_SRV_CAP_CFG_RESULT_ENUM_UINT32             enResult;
} TAF_CCM_ECC_SRV_CAP_CFG_CNF_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
} TAF_CCM_ECC_SRV_CAP_QRY_REQ_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_ECC_SRV_CAP_QRY_CNF_PARA_STRU                  stEccSrvCapQryCnfPara;
} TAF_CCM_ECC_SRV_CAP_QRY_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_SET_EC_TEST_MODE_ENUM_UINT32                   enEccTestModeStatus;
} TAF_CCM_SET_EC_TEST_MODE_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    VOS_UINT32                                              ulResult;
} TAF_CCM_SET_EC_TEST_MODE_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
} TAF_CCM_GET_EC_TEST_MODE_REQ_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_GET_EC_TEST_MODE_CNF_PARA_STRU                 stTestModeCnfPara;
} TAF_CCM_GET_EC_TEST_MODE_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
} TAF_CCM_GET_EC_RANDOM_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_GET_EC_RANDOM_CNF_PARA_STRU                    stEcRandomData;
} TAF_CCM_GET_EC_RANDOM_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
} TAF_CCM_GET_EC_KMC_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_GET_EC_KMC_CNF_PARA_STRU                       stKmcCnfPara;
} TAF_CCM_GET_EC_KMC_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    MN_CALL_APP_EC_KMC_DATA_STRU                            stKmcData;
} TAF_CCM_SET_EC_KMC_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    VOS_UINT32                                              ulResult;
} TAF_CCM_SET_EC_KMC_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_ENCRYPTED_VOICE_DATA_IND_PARA_STRU             stEncVoiceDataIndPara;
}TAF_CCM_ENCRYPTED_VOICE_DATA_IND_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_PRIVACY_MODE_SET_PARA_STRU                     stPrivacyMode;
} TAF_CCM_PRIVACY_MODE_SET_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_RESULT_TYPE_ENUM_UINT32                        enResult;
} TAF_CCM_PRIVACY_MODE_SET_CNF_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
} TAF_CCM_PRIVACY_MODE_QRY_REQ_STRU;


typedef struct
{
    VOS_UINT8                                               ucCallId;
    TAF_CALL_PRIVACY_MODE_ENUM_UINT8                        enPrivacyMode;
    VOS_UINT8                                               aucReserved[2];
} TAF_CCM_PRIVACY_MODE_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_PRIVACY_MODE_QRY_CNF_PARA_STRU                 stPrivacyModeQryCnfPara;
} TAF_CCM_PRIVACY_MODE_QRY_CNF_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    TAF_CTRL_STRU                                           stCtrl;
    TAF_CALL_PRIVACY_MODE_IND_PARA_STRU                     stPrivacyModeIndPara;
} TAF_CCM_PRIVACY_MODE_IND_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CCWAI_SET_REQ_STRU         stCcwaiSet;
} TAF_CCM_SET_CCWAI_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_GET_CDUR_PARA_STRU         stGetCdurPara;
} TAF_CCM_GET_CDUR_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_GET_CDUR_PARA_STRU         stGetCdurPara;
} TAF_CCM_GET_CDUR_CNF_STRU;

VOS_UINT32  TAF_CCM_CallCommonReq(
    TAF_CTRL_STRU                      *pstCtrl,
    const VOS_VOID                     *pPara,
    VOS_UINT32                          ulMsgType,
    VOS_UINT32                          ulParaLen,
    MODEM_ID_ENUM_UINT16                enModemId
);

#if ((VOS_OS_VER == VOS_WIN32) || (TAF_OS_VER == TAF_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TafCcmApi.h */

