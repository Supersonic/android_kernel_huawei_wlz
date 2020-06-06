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
#include "AtCtx.h"
#include "AtDataProc.h"
#include "AtMntn.h"
#include "mdrv_rfile.h"
#include "securec.h"

#if (VOS_OS_VER == VOS_LINUX)
#include <linux/pm_wakeup.h>
#else
#include "Linuxstub.h"
#endif


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CTX_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
/***************************AT��ȫ�ֱ����� Begin******************************/
/* AT��������ε�PID��Ӧ��  */
#if (2 <= MULTI_MODEM_NUMBER)
AT_MODEM_PID_TAB_STRU                   g_astAtModemPidTab[] =
{
    {I0_UEPS_PID_GRM,           I1_UEPS_PID_GRM,            I2_UEPS_PID_GRM,        0},
    {I0_UEPS_PID_DL,            I1_UEPS_PID_DL,             I2_UEPS_PID_DL,         0},
    {I0_UEPS_PID_LL,            I1_UEPS_PID_LL,             I2_UEPS_PID_LL,         0},
    {I0_UEPS_PID_SN,            I1_UEPS_PID_SN,             I2_UEPS_PID_SN,         0},
    {I0_UEPS_PID_GAS,           I1_UEPS_PID_GAS,            I2_UEPS_PID_GAS,        0},
    {I0_WUEPS_PID_MM,           I1_WUEPS_PID_MM,            I2_WUEPS_PID_MM,        0},
    {I0_WUEPS_PID_MMC,          I1_WUEPS_PID_MMC,           I2_WUEPS_PID_MMC,       0},
    {I0_WUEPS_PID_GMM,          I1_WUEPS_PID_GMM,           I2_WUEPS_PID_GMM,       0},
    {I0_WUEPS_PID_MMA,          I1_WUEPS_PID_MMA,           I2_WUEPS_PID_MMA,       0},
    {I0_WUEPS_PID_CC,           I1_WUEPS_PID_CC,            I2_WUEPS_PID_CC,        0},
    {I0_WUEPS_PID_SS,           I1_WUEPS_PID_SS,            I2_WUEPS_PID_SS,        0},
    {I0_WUEPS_PID_TC,           I1_WUEPS_PID_TC,            I2_WUEPS_PID_TC,        0},
    {I0_WUEPS_PID_SMS,          I1_WUEPS_PID_SMS,           I2_WUEPS_PID_SMS,       0},
    {I0_WUEPS_PID_RABM,         I1_WUEPS_PID_RABM,          I2_WUEPS_PID_RABM,      0},
    {I0_WUEPS_PID_SM,           I1_WUEPS_PID_SM,            I2_WUEPS_PID_SM,        0},
    {I0_WUEPS_PID_ADMIN,        I1_WUEPS_PID_ADMIN,         I2_WUEPS_PID_ADMIN,     0},
    {I0_WUEPS_PID_TAF,          I1_WUEPS_PID_TAF,           I2_WUEPS_PID_TAF,       0},
    {I0_UEPS_PID_DSM,           I1_UEPS_PID_DSM,            I2_UEPS_PID_DSM,        0},
	{I0_UEPS_PID_CCM,           I1_UEPS_PID_CCM,            I2_UEPS_PID_CCM,       0},
    {I0_WUEPS_PID_VC,           I1_WUEPS_PID_VC,            I2_WUEPS_PID_VC,        0},
    {I0_WUEPS_PID_DRV_AGENT,    I1_WUEPS_PID_DRV_AGENT,     I2_WUEPS_PID_DRV_AGENT, 0},
    {I0_UEPS_PID_MTA,           I1_UEPS_PID_MTA,            I2_UEPS_PID_MTA,        0},
    {I0_DSP_PID_GPHY,           I1_DSP_PID_GPHY,            I2_DSP_PID_GPHY,        0},
    {I0_DSP_PID_SLEEP,          I1_DSP_PID_SLEEP,           I2_DSP_PID_SLEEP,       0},
    {I0_DSP_PID_APM,            I1_DSP_PID_APM,             I2_DSP_PID_APM,         0},
    {I0_WUEPS_PID_SLEEP,        I1_WUEPS_PID_SLEEP,         I2_WUEPS_PID_SLEEP,     0},
    {I0_WUEPS_PID_WRR,          I1_WUEPS_PID_WRR,           0,                      0},
    {I0_WUEPS_PID_WCOM,         I1_WUEPS_PID_WCOM,          0,                      0},
    {I0_DSP_PID_WPHY,           I1_DSP_PID_WPHY,            0,                      0},
#if ( FEATURE_MODEM1_SUPPORT_LTE == FEATURE_ON )
    {I0_MSP_L4_L4A_PID,         I1_MSP_L4_L4A_PID,          I0_MSP_L4_L4A_PID,      0},
    {I0_MSP_LDSP_PID,           I1_MSP_LDSP_PID,            I0_MSP_LDSP_PID,        0},
    {I0_MSP_SYS_FTM_PID,        I1_MSP_SYS_FTM_PID,         I0_MSP_SYS_FTM_PID,     0},
    {I0_PS_PID_IMSA,            I1_PS_PID_IMSA,             I0_PS_PID_IMSA,         0}
#endif
};
#endif

VOS_UINT32                              g_ulCtzuFlag = 0;

/* ATģ�鹫�������� */
AT_COMM_CTX_STRU                        g_stAtCommCtx;

/* ATģ����Modem��ص������� */
AT_MODEM_CTX_STRU                       g_astAtModemCtx[MODEM_ID_BUTT];

/* ATģ����Client��ص������� */
AT_CLIENT_CTX_STRU                      g_astAtClientCtx[AT_MAX_CLIENT_NUM];

/* ATģ�鸴λ��ص������� */
AT_RESET_CTX_STRU                       g_stAtResetCtx;

/***************************AT��ȫ�ֱ����� End******************************/

/*********************************������Ҫ������*************************************/
/* �Ƿ����Ȩ�ޱ�־(����DIAG/SHELL�ڵ�Ȩ��) */
AT_E5_RIGHT_FLAG_ENUM_U32               g_enATE5RightFlag;

/* ���� g_stATDislogPwd �е� DIAG�ڵ�״̬Ҫ���뱸��NV�б�; �����벻�ñ���
   �ʽ� g_stATDislogPwd �е��������, ��ʹ�����е� DIAG ��״̬;
   ���¶���NV��������������  */
VOS_INT8                                g_acATOpwordPwd[AT_OPWORD_PWD_LEN+1];

/* ���ڵ�ǰ�Ƿ���Ȩ�޲���AT�˿� */
AT_RIGHT_OPEN_FLAG_STRU                 g_stAtRightOpenFlg;

/* ʹ�ܽ�ֹSD��ʱ��Ҫ������ */
VOS_INT8                                g_acATE5DissdPwd[AT_DISSD_PWD_LEN+1];

/*���֧����ʾ�ִ�*/
VOS_UINT8                               gaucAtCmdNotSupportStr[AT_NOTSUPPORT_STR_LEN+4];

AT_ABORT_CMD_CTX_STRU                   gstAtAbortCmdCtx;   /* ���ڱ����ϵ���Ϣ */

/*��¼���Ŵ������Ƿ�ʹ�� */
PPP_DIAL_ERR_CODE_ENUM                  gucPppDialErrCodeRpt;

AT_DIAL_CONNECT_DISPLAY_RATE_STRU       g_stDialConnectDisplayRate;

/* UE��������: ����Э��汾������ֵ */
AT_DOWNLINK_RATE_CATEGORY_STRU          g_stAtDlRateCategory;

VOS_UINT8                               ucAtS3          = 13;                   /* <CR> */
VOS_UINT8                               ucAtS4          = 10;                   /* <LF> */
VOS_UINT8                               ucAtS5          = 8;                    /* <DEL> */
VOS_UINT8                               ucAtS6          = 2;                    /* Number of seconds to wait before blind dialling:default value = 2 */
VOS_UINT8                               ucAtS7          = 50;                   /* Number of seconds in which connection must be established or call will be disconnected,
                                                                                   default value = 50(refer to Q)*/

AT_CMEE_TYPE                            gucAtCmeeType;                          /* E5������Ĭ�ϴ����� */

TAF_UINT32                              g_ulSTKFunctionFlag = TAF_FALSE;

/*********************************CC Begin*************************************/
/*********************************CC End*************************************/

/*********************************SMS Begin*************************************/
MN_MSG_CLASS0_TAILOR_U8                 g_enClass0Tailor         = MN_MSG_CLASS0_DEF;
/*********************************SMS End*************************************/

/*********************************NET Begin*************************************/
VOS_UINT16                              g_usReportCregActParaFlg = VOS_FALSE;
CREG_CGREG_CI_RPT_BYTE_ENUM             gucCiRptByte = CREG_CGREG_CI_RPT_TWO_BYTE;

/*********************************NET End*************************************/
#if (FEATURE_LTE == FEATURE_ON)
NVIM_RSRP_CFG_STRU                      g_stRsrpCfg;
NVIM_RSCP_CFG_STRU                      g_stRscpCfg;
NVIM_ECIO_CFG_STRU                      g_stEcioCfg;

AT_ACCESS_STRATUM_RELEASE_STRU          g_stReleaseInfo;
#endif

AT_SS_CUSTOMIZE_PARA_STRU               g_stAtSsCustomizePara;

AT_TRACE_MSGID_TAB_STRU                 g_stAtTraceMsgIdTab[AT_CLIENT_ID_BUTT];

/*lint -e648 -e598 -e845 */

AT_CLIENT_CFG_MAP_TAB_STRU              g_astAtClientCfgMapTbl[] =
{
    AT_CLIENT_CFG_ELEMENT(PCUI),
    AT_CLIENT_CFG_ELEMENT(CTRL),
    AT_CLIENT_CFG_ELEMENT(PCUI2),
    AT_CLIENT_CFG_ELEMENT(MODEM),
    AT_CLIENT_CFG_ELEMENT(NDIS),
    AT_CLIENT_CFG_ELEMENT(UART),
#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_CLIENT_CFG_ELEMENT(HSUART),
#endif
    AT_CLIENT_CFG_ELEMENT(SOCK),
    AT_CLIENT_CFG_ELEMENT(APPSOCK),

    AT_CLIENT_CFG_ELEMENT(APP),
    AT_CLIENT_CFG_ELEMENT(APP1),
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    AT_CLIENT_CFG_ELEMENT(APP2),
    AT_CLIENT_CFG_ELEMENT(APP3),
    AT_CLIENT_CFG_ELEMENT(APP4),
    AT_CLIENT_CFG_ELEMENT(APP5),
    AT_CLIENT_CFG_ELEMENT(APP6),
    AT_CLIENT_CFG_ELEMENT(APP7),
    AT_CLIENT_CFG_ELEMENT(APP8),
    AT_CLIENT_CFG_ELEMENT(APP9),
    AT_CLIENT_CFG_ELEMENT(APP10),
    AT_CLIENT_CFG_ELEMENT(APP11),
    AT_CLIENT_CFG_ELEMENT(APP12),
    AT_CLIENT_CFG_ELEMENT(APP13),
    AT_CLIENT_CFG_ELEMENT(APP14),
    AT_CLIENT_CFG_ELEMENT(APP15),
    AT_CLIENT_CFG_ELEMENT(APP16),
    AT_CLIENT_CFG_ELEMENT(APP17),
    AT_CLIENT_CFG_ELEMENT(APP18),
    AT_CLIENT_CFG_ELEMENT(APP19),
    AT_CLIENT_CFG_ELEMENT(APP20),
    AT_CLIENT_CFG_ELEMENT(APP21),
    AT_CLIENT_CFG_ELEMENT(APP22),
    AT_CLIENT_CFG_ELEMENT(APP23),
    AT_CLIENT_CFG_ELEMENT(APP24),
    AT_CLIENT_CFG_ELEMENT(APP25),
    AT_CLIENT_CFG_ELEMENT(APP26),

    AT_CLIENT_CFG_ELEMENT(APP27),
    AT_CLIENT_CFG_ELEMENT(APP28),
    AT_CLIENT_CFG_ELEMENT(APP29),
    AT_CLIENT_CFG_ELEMENT(APP30),
    AT_CLIENT_CFG_ELEMENT(APP31),
    AT_CLIENT_CFG_ELEMENT(APP32),
    AT_CLIENT_CFG_ELEMENT(APP33),
    AT_CLIENT_CFG_ELEMENT(APP34),
    AT_CLIENT_CFG_ELEMENT(APP35),
    AT_CLIENT_CFG_ELEMENT(APP36),
    AT_CLIENT_CFG_ELEMENT(APP37),
    AT_CLIENT_CFG_ELEMENT(APP38),
    AT_CLIENT_CFG_ELEMENT(APP39),
    AT_CLIENT_CFG_ELEMENT(APP40),
    AT_CLIENT_CFG_ELEMENT(APP41),
    AT_CLIENT_CFG_ELEMENT(APP42),
    AT_CLIENT_CFG_ELEMENT(APP43),
    AT_CLIENT_CFG_ELEMENT(APP44),
    AT_CLIENT_CFG_ELEMENT(APP45),
    AT_CLIENT_CFG_ELEMENT(APP46),
    AT_CLIENT_CFG_ELEMENT(APP47),
    AT_CLIENT_CFG_ELEMENT(APP48),
    AT_CLIENT_CFG_ELEMENT(APP49),
    AT_CLIENT_CFG_ELEMENT(APP50),
    AT_CLIENT_CFG_ELEMENT(APP51),
    AT_CLIENT_CFG_ELEMENT(APP52)

#endif
};
/*lint +e648 +e598 +e845 */

const VOS_UINT8                         g_ucAtClientCfgMapTabLen = AT_ARRAY_SIZE(g_astAtClientCfgMapTbl);

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON)&&(FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)&&(FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON))

VOS_UINT8    gucCurrEncVoiceDataWriteFileNum       = 0;
VOS_UINT32   gulAtCurrEncVoiceDataCount            = 0;
VOS_UINT8    gucAtCurrEncVoiceTestFileNum          = 0;
TAF_CHAR     gacAtCurrDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN];

/*lint -e786*/
TAF_CHAR    *g_pacCurrEncVoiceDataWriteFilePath[] =
            {
                "",
                MODEM_LOG_ROOT"/ECC_TEST/Encrypted_call_data1.txt",
                MODEM_LOG_ROOT"/ECC_TEST/Encrypted_call_data2.txt",
                MODEM_LOG_ROOT"/ECC_TEST/Encrypted_call_data3.txt",
                MODEM_LOG_ROOT"/ECC_TEST/Encrypted_call_data4.txt",
                MODEM_LOG_ROOT"/ECC_TEST/Encrypted_call_data5.txt"
            };
/*lint +e786*/

#endif
#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)
struct wakeup_source                    g_stAtWakeLock;
#endif
#if ( VOS_WIN32 == VOS_OS_VER )

AT_USIMM_FILE_NUM_TO_ID_STRU g_aenAtSimFileNumToIdTab[]=
{
    {0x6F07, USIMM_GSM_EFIMSI_ID        },
    {0x6F46, USIMM_GSM_EFSPN_ID         },
    {0x6F31, USIMM_GSM_EFHPLMN_ID       },
    {0x6F53, USIMM_GSM_EFLOCIGPRS_ID    },
    {0x6f61, USIMM_GSM_EFOPLMNWACT_ID   },
    {0x6f62, USIMM_GSM_EFHPLMNACT_ID    },
    {0x6F7E, USIMM_GSM_EFLOCI_ID        }
};

VOS_UINT32 g_aenAtSimFileNumToIdTabLen = AT_ARRAY_SIZE(g_aenAtSimFileNumToIdTab);

AT_USIMM_FILE_NUM_TO_ID_STRU g_aenAtUsimFileNumToIdTab[]=
{
    {0x3F00, USIMM_MF_ID                },
    {0x2F00, USIMM_DIR_ID               },
    {0x2FE2, USIMM_ICCID_ID             },
    {0x2F05, USIMM_PL_ID                },
    {0x2F06, USIMM_ARR_ID               },
    {0x7FFF, USIMM_USIM_ID              },
    {0x6F05, USIMM_USIM_EFLI_ID         },
    {0x6F06, USIMM_USIM_EFARR_ID        },
    {0x6F07, USIMM_USIM_EFIMSI_ID       },
    {0x6F08, USIMM_USIM_EFKEYS_ID       },
    {0x6F09, USIMM_USIM_EFKEYSPS_ID     },
    {0x6F15, USIMM_USIM_EFCSP_ID        },
    {0x6F2C, USIMM_USIM_EFDCK_ID        },
    {0x6F31, USIMM_USIM_EFHPPLMN_ID     },
    {0x6F32, USIMM_USIM_EFCNL_ID        },
    {0x6F37, USIMM_USIM_EFACMMAX_ID     },
    {0x6F38, USIMM_USIM_EFUST_ID        },
    {0x6F39, USIMM_USIM_EFACM_ID        },
    {0x6F3B, USIMM_USIM_EFFDN_ID        },
    {0x6F3C, USIMM_USIM_EFSMS_ID        },
    {0x6F3E, USIMM_USIM_EFGID1_ID       },
    {0x6F3F, USIMM_USIM_EFGID2_ID       },
    {0x6F40, USIMM_USIM_EFMSISDN_ID     },
    {0x6F41, USIMM_USIM_EFPUCT_ID       },
    {0x6F42, USIMM_USIM_EFSMSP_ID       },
    {0x6F43, USIMM_USIM_EFSMSS_ID       },
    {0x6F45, USIMM_USIM_EFCBMI_ID       },
    {0x6F46, USIMM_USIM_EFSPN_ID        },
    {0x6F47, USIMM_USIM_EFSMSR_ID       },
    {0x6F48, USIMM_USIM_EFCBMID_ID      },
    {0x6F49, USIMM_USIM_EFSDN_ID        },
    {0x6F4B, USIMM_USIM_EFEXT2_ID       },
    {0x6F4C, USIMM_USIM_EFEXT3_ID       },
    {0x6F4D, USIMM_USIM_EFBDN_ID        },
    {0x6F4E, USIMM_USIM_EFEXT5_ID       },
    {0x6F4F, USIMM_USIM_EFCCP2_ID       },
    {0x6F50, USIMM_USIM_EFCBMIR_ID      },
    {0x6F55, USIMM_USIM_EFEXT4_ID       },
    {0x6F56, USIMM_USIM_EFEST_ID        },
    {0x6F57, USIMM_USIM_EFACL_ID        },
    {0x6F58, USIMM_USIM_EFCMI_ID        },
    {0x6F5B, USIMM_USIM_EFSTART_HFN_ID  },
    {0x6F5C, USIMM_USIM_EFTHRESHOL_ID   },
    {0x6F60, USIMM_USIM_EFPLMNWACT_ID   },
    {0x6F61, USIMM_USIM_EFOPLMNWACT_ID  },
    {0x6F62, USIMM_USIM_EFHPLMNwACT_ID  },
    {0x6F73, USIMM_USIM_EFPSLOCI_ID     },
    {0x6F78, USIMM_USIM_EFACC_ID        },
    {0x6F7B, USIMM_USIM_EFFPLMN_ID      },
    {0x6F7E, USIMM_USIM_EFLOCI_ID       },
    {0x6F80, USIMM_USIM_EFICI_ID        },
    {0x6F81, USIMM_USIM_EFOCI_ID        },
    {0x6F82, USIMM_USIM_EFICT_ID        },
    {0x6F83, USIMM_USIM_EFOCT_ID        },
    {0x6FAD, USIMM_USIM_EFAD_ID         },
    {0x6FB1, USIMM_USIM_EFVGCS_ID       },
    {0x6FB2, USIMM_USIM_EFVGCSS_ID      },
    {0x6FB3, USIMM_USIM_EFVBS_ID        },
    {0x6FB4, USIMM_USIM_EFVBSS_ID       },
    {0x6FB5, USIMM_USIM_EFEMLPP_ID      },
    {0x6FB6, USIMM_USIM_EFAAEM_ID       },
    {0x6FB7, USIMM_USIM_EFECC_ID        },
    {0x6FC3, USIMM_USIM_EFHIDDENKEY_ID  },
    {0x6FC4, USIMM_USIM_EFNETPAR_ID     },
    {0x6FC5, USIMM_USIM_EFPNN_ID        },
    {0x6FC6, USIMM_USIM_EFOPL_ID        },
    {0x6FC7, USIMM_USIM_EFMBDN_ID       },
    {0x6FC8, USIMM_USIM_EFEXT6_ID       },
    {0x6FC9, USIMM_USIM_EFMBI_ID        },
    {0x6FCA, USIMM_USIM_EFMWIS_ID       },
    {0x6FCB, USIMM_USIM_EFCFIS_ID       },
    {0x6FCC, USIMM_USIM_EFEXT7_ID       },
    {0x6FCD, USIMM_USIM_EFSPDI_ID       },
    {0x6FCE, USIMM_USIM_EFMMSN_ID       },
    {0x6FCF, USIMM_USIM_EFEXT8_ID       },
    {0x6FD0, USIMM_USIM_EFMMSICP_ID     },
    {0x6FD1, USIMM_USIM_EFMMSUP_ID      },
    {0x6FD2, USIMM_USIM_EFMMSUCP_ID     },
    {0x6FD3, USIMM_USIM_EFNIA_ID        },
    {0x6FD4, USIMM_USIM_EFVGCSCA_ID     },
    {0x6FD5, USIMM_USIM_EFVBSCA_ID      },
    {0x6FD6, USIMM_USIM_EFGBAP_ID       },
    {0x6FD7, USIMM_USIM_EFMSK_ID        },
    {0x6FD8, USIMM_USIM_EFMUK_ID        },
    {0x6FD9, USIMM_USIM_EFEHPLMN_ID     },
    {0x6FDA, USIMM_USIM_EFGBANL_ID      },
    {0x6FDB, USIMM_USIM_EFEHPLMNPI_ID   },
    {0x6FDC, USIMM_USIM_EFLRPLMNSI_ID   },
    {0x6FDD, USIMM_USIM_EFNAFKCA_ID     },
    {0x6FDE, USIMM_USIM_EFSPNI_ID       },
    {0x6FDF, USIMM_USIM_EFPNNI_ID       },
    {0x6FE2, USIMM_USIM_EFNCPIP_ID      },
    {0x6FE3, USIMM_USIM_EFEPSLOCI_ID    },
    {0x6FE4, USIMM_USIM_EFEPSNSC_ID     },
    {0x6FE6, USIMM_USIM_EFUFC_ID        },
    {0x6FE7, USIMM_USIM_EFUICCIARI_ID   },
    {0x6FE8, USIMM_USIM_EFNASCONFIG_ID  },
    {0x6FEC, USIMM_USIM_EFPWS_ID        },
    {0x5F3A, USIMM_USIM_DFPHONEBOOK_ID  },
    {0x4F22, USIMM_USIM_EFPSC_ID        },
    {0x4F23, USIMM_USIM_EFCC_ID         },
    {0x4F24, USIMM_USIM_EFPUID_ID       },
    {0x4F30, USIMM_USIM_EFPBR_ID        },
    {0x5F3B, USIMM_USIM_DFGSM_ACCESS_ID },
    {0x4F20, USIMM_USIM_EFKC_ID         },
    {0x4F52, USIMM_USIM_EFKCGPRS_ID     },
    {0x4F63, USIMM_USIM_EFCPBCCH_ID     },
    {0x4F64, USIMM_USIM_EFINVSCAN_ID    },
    {0x5F3C, USIMM_USIM_DFMEXE_ID       },
    {0x4F40, USIMM_USIM_EFMexE_ST_ID    },
    {0x4F41, USIMM_USIM_EFORPK_ID       },
    {0x4F42, USIMM_USIM_EFARPK_ID       },
    {0x4F43, USIMM_USIM_EFTPRK_ID       },
    {0x5F70, USIMM_USIM_DFSOLSA_ID      },
    {0x4F30, USIMM_USIM_EFSAI_ID        },
    {0x4F31, USIMM_USIM_EFSLL_ID        },
    {0x5F40, USIMM_USIM_DFWLAN_ID       },
    {0x4F41, USIMM_USIM_EFPSEUDO_ID     },
    {0x4F42, USIMM_USIM_EFUPLMNWLAN_ID  },
    {0x4F43, USIMM_USIM_EF0PLMNWLAN_ID  },
    {0x4F44, USIMM_USIM_EFUWSIDL_ID     },
    {0x4F45, USIMM_USIM_EFOWSIDL_ID     },
    {0x4F46, USIMM_USIM_EFWRI_ID        },
    {0x4F47, USIMM_USIM_EFHWSIDL_ID     },
    {0x4F48, USIMM_USIM_EFWEHPLMNPI_ID  },
    {0x4F49, USIMM_USIM_EFWHPI_ID       },
    {0x4F4A, USIMM_USIM_EFWLRPLMN_ID    },
    {0x4F4B, USIMM_USIM_EFHPLMNDAI_ID   },
    {0x5F50, USIMM_USIM_DFHNB_ID        },
    {0x4F81, USIMM_USIM_EFACSGL_ID      },
    {0x4F82, USIMM_USIM_EFCSGT_ID       },
    {0x4F83, USIMM_USIM_EFHNBN_ID       },
    {0x4F84, USIMM_USIM_EFOCSGL_ID      },
    {0x4F85, USIMM_USIM_EFOCSGT_ID      },
    {0x4F86, USIMM_USIM_EFOHNBN_ID      },
    {0X6F30, USIMM_GSM_EFPLMNSEL_ID     },
    {0x6F53, USIMM_GSM_EFLOCIGPRS_ID    },
    {0x6F20, USIMM_GSM_EFKC_ID          },
    {0x6F52, USIMM_GSM_EFKCGPRS_ID      },
    {0x7F66, USIMM_ATTUSIM_ID},
    {0x6FD2, USIMM_ATTUSIM_EFTERMINALTBL_ID},
    {0x4F34, USIMM_ATTUSIM_EFACTINGHPLMN_ID},
    {0x4F36, USIMM_ATTUSIM_EFRATMODE_ID},
    {0x4F40, USIMM_ATTUSIM_EFPRMENABLE_ID},
    {0x4F41, USIMM_ATTUSIM_EFPRMPARA_ID},
    {0x4F42, USIMM_ATTUSIM_EFPRMOMCLR_ID},
    {0x4F43, USIMM_ATTUSIM_EFPRMOMC_ID},
    {0x4F44, USIMM_ATTUSIM_EFPRMVERINFO_ID},

    {0x4F01, USIMM_USIM_EF5GS3GPPLOCI_ID},
    {0x4F03, USIMM_USIM_EF5GS3GPPNSC_ID},
    {0x4F05, USIMM_USIM_EF5GAUTHKEYS_ID},
    {0x4F06, USIMM_USIM_EFUAC_AIC_ID},
    {0x4F07, USIMM_USIM_EFSUCI_CALC_INFO_ID},
    {0x4F08, USIMM_USIM_EFOPL_5GS_ID},
};

VOS_UINT32 g_aenAtUsimFileNumToIdTabLen = AT_ARRAY_SIZE(g_aenAtUsimFileNumToIdTab);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
AT_USIMM_FILE_NUM_TO_ID_STRU g_aenAtCsimFileNumToIdTab[]=
{
    {0x6F22, USIMM_CSIM_EFIMSIM_ID                },
    {0x6F47, USIMM_CSIM_EFECC_ID                  },
};

VOS_UINT32 g_aenAtCsimFileNumToIdTabLen = AT_ARRAY_SIZE(g_aenAtCsimFileNumToIdTab);

#endif

#endif

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

VOS_VOID AT_InitUsimStatus(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_USIM_INFO_CTX_STRU              *pstUsimInfoCtx = VOS_NULL_PTR;

    pstUsimInfoCtx = AT_GetUsimInfoCtxFromModemId(enModemId);

    pstUsimInfoCtx->enCardStatus = USIMM_CARDAPP_SERVIC_BUTT;
    pstUsimInfoCtx->enCardType   = TAF_MMA_USIMM_CARD_TYPE_BUTT;

    pstUsimInfoCtx->ucIMSILen    = 0;
    memset_s(pstUsimInfoCtx->aucIMSI, sizeof(pstUsimInfoCtx->aucIMSI), 0x00, sizeof(pstUsimInfoCtx->aucIMSI));

    pstUsimInfoCtx->enSimsqStatus       = TAF_MMA_SIMSQ_BUTT;


    return;
}


VOS_VOID AT_InitPlatformRatList(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_SPT_RAT_STRU              *pstSptRat   = VOS_NULL_PTR;
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    VOS_UINT8                          *pucIsCLMode = VOS_NULL_PTR;
#endif
    pstSptRat = AT_GetSptRatFromModemId(enModemId);

    /* Ĭ������µ���ֻ֧��GSM */
    pstSptRat->ucPlatformSptGsm        = VOS_TRUE;
    pstSptRat->ucPlatformSptWcdma      = VOS_FALSE;
    pstSptRat->ucPlatformSptLte        = VOS_FALSE;
    pstSptRat->ucPlatformSptUtralTDD   = VOS_FALSE;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    pstSptRat->ucPlatformSptNR         = VOS_FALSE;
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    pucIsCLMode  = AT_GetModemCLModeCtxAddrFromModemId(enModemId);
    *pucIsCLMode = VOS_FALSE;
#endif
    return;

}


VOS_VOID AT_InitCommPsCtx(VOS_VOID)
{
    AT_COMM_PS_CTX_STRU                *pstPsCtx = VOS_NULL_PTR;

    pstPsCtx = AT_GetCommPsCtxAddr();

    memset_s(pstPsCtx, sizeof(AT_COMM_PS_CTX_STRU), 0, sizeof(AT_COMM_PS_CTX_STRU));

    pstPsCtx->ucIpv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;

    pstPsCtx->ulIpv6AddrTestModeCfg = 0;

    pstPsCtx->ucSharePdpFlag = VOS_FALSE;

    pstPsCtx->lSpePort       = AT_INVALID_SPE_PORT;
    pstPsCtx->ulIpfPortFlg   = VOS_FALSE;

    return;
}


VOS_VOID AT_InitCommPbCtx(VOS_VOID)
{
    AT_COMM_PB_CTX_STRU                *pstCommPbCntxt = VOS_NULL_PTR;

    pstCommPbCntxt = AT_GetCommPbCtxAddr();

    pstCommPbCntxt->usCurrIdx       = 0;
    pstCommPbCntxt->usLastIdx       = 0;
    pstCommPbCntxt->ulSingleReadFlg = VOS_FALSE;
    return;
}


AT_CMD_PROC_CTX_STRU* AT_GetCmdProcCtxAddr(VOS_VOID)
{
    return &(g_stAtCommCtx.stCmdProcCtx);
}


AT_AUTH_PUBKEYEX_CMD_PROC_CTX* AT_GetAuthPubkeyExCmdCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->stAuthPubkeyExCmdCtx);
}



AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *AT_GetSimLockWriteExCmdCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->stSimLockWriteExCmdCtx);
}

AT_VMSET_CMD_CTX_STRU* AT_GetCmdVmsetCtxAddr(VOS_VOID)
{
    return &(AT_GetCmdProcCtxAddr()->stVmSetCmdCtx);
}


VOS_VOID AT_InitVmSetCtx(VOS_VOID)
{
    AT_VMSET_CMD_CTX_STRU              *pstVmSetCmdCtx        = VOS_NULL_PTR;

    pstVmSetCmdCtx = AT_GetCmdVmsetCtxAddr();

    pstVmSetCmdCtx->ulReportedModemNum  = 0;
    pstVmSetCmdCtx->ulResult            = AT_OK;

    return;
}


VOS_VOID AT_InitSimlockWriteSetCtx(VOS_VOID)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimlockWriteExCtx = VOS_NULL_PTR;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    pstSimlockWriteExCtx->ucClientId                  = 0;
    pstSimlockWriteExCtx->ucLayer                     = 0;
    pstSimlockWriteExCtx->ucCurIdx                    = 0;
    pstSimlockWriteExCtx->ucTotalNum                  = 0;
    pstSimlockWriteExCtx->usSimlockDataLen            = 0;
    pstSimlockWriteExCtx->ucHmacLen                   = 0;
    pstSimlockWriteExCtx->ucSettingFlag               = VOS_FALSE;
    pstSimlockWriteExCtx->pucData                     = VOS_NULL_PTR;
    pstSimlockWriteExCtx->hSimLockWriteExProtectTimer = VOS_NULL_PTR;

    memset_s(pstSimlockWriteExCtx->aucHmac,
             sizeof(pstSimlockWriteExCtx->aucHmac),
             0x00,
             sizeof(pstSimlockWriteExCtx->aucHmac));
}


VOS_VOID AT_InitCmdProcCtx(VOS_VOID)
{
    AT_AUTH_PUBKEYEX_CMD_PROC_CTX      *pstAuthPubkeyExCmdCtx = VOS_NULL_PTR;

    pstAuthPubkeyExCmdCtx = AT_GetAuthPubkeyExCmdCtxAddr();

    pstAuthPubkeyExCmdCtx->ucClientId               = 0;
    pstAuthPubkeyExCmdCtx->ucCurIdx                 = 0;
    pstAuthPubkeyExCmdCtx->ucTotalNum               = 0;
    pstAuthPubkeyExCmdCtx->usParaLen                = 0;
    pstAuthPubkeyExCmdCtx->ucSettingFlag            = VOS_FALSE;
    pstAuthPubkeyExCmdCtx->pucData                  = VOS_NULL_PTR;
    pstAuthPubkeyExCmdCtx->hAuthPubkeyProtectTimer  = VOS_NULL_PTR;

    AT_InitSimlockWriteSetCtx();

    AT_InitVmSetCtx();

    return;
}


VOS_VOID AT_InitMsgNumCtrlCtx(VOS_VOID)
{
    AT_CMD_MSG_NUM_CTRL_STRU           *pstMsgNumCtrlCtx = VOS_NULL_PTR;

    pstMsgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();

    /* ����ʼ�� */
    VOS_SpinLockInit(&(pstMsgNumCtrlCtx->stSpinLock));

    pstMsgNumCtrlCtx->ulMsgCount = 0;

    return;
}


VOS_VOID AT_ClearAuthPubkeyCtx(VOS_VOID)
{
    AT_AUTH_PUBKEYEX_CMD_PROC_CTX      *pstAuthPubkeyExCmdCtx = VOS_NULL_PTR;

    pstAuthPubkeyExCmdCtx = AT_GetAuthPubkeyExCmdCtxAddr();

    pstAuthPubkeyExCmdCtx->ucClientId               = 0;
    pstAuthPubkeyExCmdCtx->ucCurIdx                 = 0;
    pstAuthPubkeyExCmdCtx->ucTotalNum               = 0;
    pstAuthPubkeyExCmdCtx->usParaLen                = 0;
    pstAuthPubkeyExCmdCtx->ucSettingFlag            = VOS_FALSE;

    if (pstAuthPubkeyExCmdCtx->pucData != VOS_NULL_PTR)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pstAuthPubkeyExCmdCtx->pucData);//lint !e830
        pstAuthPubkeyExCmdCtx->pucData = VOS_NULL_PTR;
    }

    return;
}


VOS_VOID AT_RcvTiAuthPubkeyExpired(REL_TIMER_MSG *pstTmrMsg)
{
    AT_ClearAuthPubkeyCtx();

    return;
}


VOS_VOID AT_ClearSimLockWriteExCtx(VOS_VOID)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimLockWriteExCtx = VOS_NULL_PTR;

    pstSimLockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    pstSimLockWriteExCtx->ucClientId               = 0;
    pstSimLockWriteExCtx->ucLayer                  = 0;
    pstSimLockWriteExCtx->ucCurIdx                 = 0;
    pstSimLockWriteExCtx->ucTotalNum               = 0;
    pstSimLockWriteExCtx->ucHmacLen                = 0;
    pstSimLockWriteExCtx->usSimlockDataLen         = 0;
    pstSimLockWriteExCtx->ucSettingFlag            = VOS_FALSE;

    memset_s(pstSimLockWriteExCtx->aucHmac,
             sizeof(pstSimLockWriteExCtx->aucHmac),
             0x00,
             sizeof(pstSimLockWriteExCtx->aucHmac));

    if (pstSimLockWriteExCtx->pucData != VOS_NULL_PTR)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pstSimLockWriteExCtx->pucData);
        pstSimLockWriteExCtx->pucData = VOS_NULL_PTR;
    }

    return;
}


VOS_VOID AT_RcvTiSimlockWriteExExpired(REL_TIMER_MSG *pstTmrMsg)
{
    AT_ClearSimLockWriteExCtx();

    return;
}


VOS_VOID AT_InitModemCcCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;
    VOS_UINT8                           i;

    pstCcCtx = AT_GetModemCcCtxAddrFromModemId(enModemId);

    pstCcCtx->ulCurIsExistCallFlag = VOS_FALSE;
    pstCcCtx->enCsErrCause         = TAF_CS_CAUSE_SUCCESS;

    /* �����Զ�Ӧ�������ʼ�� */
    memset_s(&(pstCcCtx->stS0TimeInfo), sizeof(pstCcCtx->stS0TimeInfo), 0x00, sizeof(pstCcCtx->stS0TimeInfo));

    memset_s(&(pstCcCtx->stEconfInfo), sizeof(pstCcCtx->stEconfInfo), 0x00, sizeof(pstCcCtx->stEconfInfo));

    for (i = 0; i < TAF_CALL_MAX_ECONF_CALLED_NUM; i++)
    {
        pstCcCtx->stEconfInfo.astCallInfo[i].enCallState = TAF_CALL_ECONF_STATE_BUTT;
        pstCcCtx->stEconfInfo.astCallInfo[i].enCause     = TAF_CS_CAUSE_SUCCESS;
    }


    return;
}


VOS_VOID AT_InitModemSsCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_SS_CTX_STRU               *pstSsCtx = VOS_NULL_PTR;

    pstSsCtx = AT_GetModemSsCtxAddrFromModemId(enModemId);

    pstSsCtx->usUssdTransMode      = AT_USSD_TRAN_MODE;
    pstSsCtx->enCModType           = MN_CALL_MODE_SINGLE;
    pstSsCtx->ucSalsType           = AT_SALS_DISABLE_TYPE;
    pstSsCtx->ucClipType           = AT_CLIP_DISABLE_TYPE;
    pstSsCtx->ucClirType           = AT_CLIR_AS_SUBSCRIPT;
    pstSsCtx->ucColpType           = AT_COLP_DISABLE_TYPE;
    pstSsCtx->ucCrcType            = AT_CRC_DISABLE_TYPE;
    pstSsCtx->ucCcwaType           = AT_CCWA_DISABLE_TYPE;


    pstSsCtx->stCbstDataCfg.enSpeed    = MN_CALL_CSD_SPD_64K_MULTI;
    pstSsCtx->stCbstDataCfg.enName     = MN_CALL_CSD_NAME_SYNC_UDI;
    pstSsCtx->stCbstDataCfg.enConnElem = MN_CALL_CSD_CE_T;

    memset_s(&(pstSsCtx->stCcugCfg), sizeof(pstSsCtx->stCcugCfg), 0x00, sizeof(pstSsCtx->stCcugCfg));

    return;
}


VOS_VOID AT_InitModemSmsCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_SMS_CTX_STRU              *pstSmsCtx = VOS_NULL_PTR;

    pstSmsCtx = AT_GetModemSmsCtxAddrFromModemId(enModemId);

    pstSmsCtx->enCmgfMsgFormat      = AT_CMGF_MSG_FORMAT_PDU;
    pstSmsCtx->ucCsdhType           = AT_CSDH_NOT_SHOW_TYPE;
    pstSmsCtx->ucParaCmsr           = VOS_FALSE;
    pstSmsCtx->ucSmsAutoReply       = VOS_FALSE;
    pstSmsCtx->enCsmsMsgVersion     = MN_MSG_CSMS_MSG_VERSION_PHASE2_PLUS;

    /* ME�洢״̬��ʼ�� */
    pstSmsCtx->enMsgMeStorageStatus = MN_MSG_ME_STORAGE_DISABLE;
    pstSmsCtx->ucLocalStoreFlg      = VOS_TRUE;

    pstSmsCtx->stCnmiType.CnmiModeType    = AT_CNMI_MODE_BUFFER_TYPE;
    pstSmsCtx->stCnmiType.CnmiMtType      = AT_CNMI_MT_NO_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiBmType      = AT_CNMI_BM_NO_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiDsType      = AT_CNMI_DS_NO_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiBfrType     = AT_CNMI_BFR_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiTmpModeType = AT_CNMI_MODE_BUFFER_TYPE;
    pstSmsCtx->stCnmiType.CnmiTmpMtType   = AT_CNMI_MT_NO_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiTmpBmType   = AT_CNMI_BM_NO_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiTmpDsType   = AT_CNMI_DS_NO_SEND_TYPE;
    pstSmsCtx->stCnmiType.CnmiTmpBfrType  = AT_CNMI_BFR_SEND_TYPE;

    /* �������ʼ�� */

    /* ���Ž��ն������ͳ�ʼ��*/
    pstSmsCtx->stSmMeFullCustomize.ucActFlg      = VOS_FALSE;
    pstSmsCtx->stSmMeFullCustomize.enMtCustomize = MN_MSG_MT_CUSTOMIZE_NONE;

    /* �ı�������ز�����ʼ�� */
    /*
    27005 3 Text Mode 3.1 Parameter Definitions
    Message Data Parameters
    <fo> depending on the command or result code: first octet of 3GPP TS 23.040
    [3] SMS-DELIVER, SMS-SUBMIT (default 17), SMS-STATUS-REPORT, or SMS-COMMAND
    (default 2) in integer format
    <vp> depending on SMS-SUBMIT <fo> setting: 3GPP TS 23.040 [3] TP-Validity-
    Period either in integer format (default 167), in time-string format (refer
    <dt>), or if EVPF is supported, in enhanced format (hexadecimal coded string
    with double quotes)
    */
    memset_s(&(pstSmsCtx->stCscaCsmpInfo), sizeof(pstSmsCtx->stCscaCsmpInfo), 0x00, sizeof(pstSmsCtx->stCscaCsmpInfo));
    pstSmsCtx->stCscaCsmpInfo.stParmInUsim.ucParmInd = 0xff;
    pstSmsCtx->stCscaCsmpInfo.stVp.enValidPeriod     =
    MN_MSG_VALID_PERIOD_RELATIVE;
    pstSmsCtx->stCscaCsmpInfo.stVp.u.ucOtherTime     = AT_CSMP_SUBMIT_VP_DEFAULT_VALUE;

    pstSmsCtx->stCscaCsmpInfo.ucDefaultSmspIndex     = AT_CSCA_CSMP_STORAGE_INDEX;

    /* ���ż�״̬�������ɾ����д�����ͻ���մ洢���ʳ�ʼ�� */
    pstSmsCtx->stCpmsInfo.stRcvPath.enStaRptMemStore = MN_MSG_MEM_STORE_SIM;
    pstSmsCtx->stCpmsInfo.stRcvPath.enSmMemStore     = MN_MSG_MEM_STORE_SIM;
    pstSmsCtx->stCpmsInfo.enMemReadorDelete          = MN_MSG_MEM_STORE_SIM;
    pstSmsCtx->stCpmsInfo.enMemSendorWrite           = MN_MSG_MEM_STORE_SIM;

    /* ���ż�״̬��������ϱ���ʽ��ʼ�� */
    pstSmsCtx->stCpmsInfo.stRcvPath.enRcvSmAct       = MN_MSG_RCVMSG_ACT_STORE;
    pstSmsCtx->stCpmsInfo.stRcvPath.enRcvStaRptAct   = MN_MSG_RCVMSG_ACT_STORE;
    pstSmsCtx->stCpmsInfo.stRcvPath.enSmsServVersion = MN_MSG_CSMS_MSG_VERSION_PHASE2_PLUS;

    /* ���Ż�״̬���治�洢ֱ���ϱ�PDU�Ļ����ʼ�� */
    memset_s(&(pstSmsCtx->stSmtBuffer), sizeof(pstSmsCtx->stSmtBuffer), 0x00, sizeof(pstSmsCtx->stSmtBuffer));

    /* �����Զ�Ӧ�𻺴�����ָ���ʼ�� */
    memset_s(pstSmsCtx->astSmsMtBuffer,
             sizeof(pstSmsCtx->astSmsMtBuffer),
             0x00,
             (sizeof(AT_SMS_MT_BUFFER_STRU) * AT_SMSMT_BUFFER_MAX));

    /* �㲥���ŵ�����ѡ��Ͳ��洢ֱ���ϱ�PDU�Ļ����ʼ�� */
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
    memset_s(&(pstSmsCtx->stCbsDcssInfo), sizeof(pstSmsCtx->stCbsDcssInfo), 0x00, sizeof(pstSmsCtx->stCbsDcssInfo));
    memset_s(&(pstSmsCtx->stCbmBuffer), sizeof(pstSmsCtx->stCbmBuffer), 0x00, sizeof(pstSmsCtx->stCbmBuffer));
#endif

    return;
}


VOS_VOID AT_InitModemNetCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    pstNetCtx->ucCerssiReportType      = AT_CERSSI_REPORT_TYPE_5DB_CHANGE_REPORT;
    pstNetCtx->ucCregType              = AT_CREG_RESULT_CODE_NOT_REPORT_TYPE;
    pstNetCtx->ucCgregType             = AT_CGREG_RESULT_CODE_NOT_REPORT_TYPE;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    pstNetCtx->ucC5gregType            = AT_C5GREG_RESULT_CODE_NOT_REPORT_TYPE;
#endif

    pstNetCtx->ucCopsFormatType         = AT_COPS_LONG_ALPH_TYPE;
    pstNetCtx->enPrefPlmnType           = MN_PH_PREF_PLMN_UPLMN;
    pstNetCtx->ucCpolFormatType         = AT_COPS_NUMERIC_TYPE;
    pstNetCtx->ucRoamFeature            = AT_ROAM_FEATURE_OFF;
    pstNetCtx->ucSpnType                = 0;
    pstNetCtx->ucCerssiMinTimerInterval = 0;
    pstNetCtx->enCalculateAntennaLevel  = AT_CMD_ANTENNA_LEVEL_0;
    memset_s(pstNetCtx->aenAntennaLevel, sizeof(pstNetCtx->aenAntennaLevel), 0x00, sizeof(pstNetCtx->aenAntennaLevel));
    memset_s(&(pstNetCtx->stTimeInfo), sizeof(pstNetCtx->stTimeInfo), 0x00, sizeof(pstNetCtx->stTimeInfo));
    memset_s(&(pstNetCtx->stCgerepCfg), sizeof(pstNetCtx->stCgerepCfg), 0x00, sizeof(pstNetCtx->stCgerepCfg));

#if(FEATURE_ON == FEATURE_LTE)
    pstNetCtx->ucCeregType           = AT_CEREG_RESULT_CODE_NOT_REPORT_TYPE;
#endif

    pstNetCtx->stCsdfCfg.ucMode         = 1;   /* 1 DD-MMM-YYYY */
    pstNetCtx->stCsdfCfg.ucAuxMode      = 1;   /* 1 yy/MM/dd */

    return;
}


VOS_VOID AT_InitModemAgpsCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_AGPS_CTX_STRU             *pstAgpsCtx = VOS_NULL_PTR;

    pstAgpsCtx = AT_GetModemAgpsCtxAddrFromModemId(enModemId);

    memset_s(&(pstAgpsCtx->stXml), sizeof(pstAgpsCtx->stXml), 0x00, sizeof(pstAgpsCtx->stXml));

    pstAgpsCtx->enCposrReport   = AT_CPOSR_DISABLE;
    pstAgpsCtx->enXcposrReport  = AT_XCPOSR_DISABLE;
    pstAgpsCtx->enCmolreType    = AT_CMOLRE_NUMERIC;

    return;
}


VOS_VOID AT_InitModemPsCtx(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulCnt;

    pstPsCtx = AT_GetModemPsCtxAddrFromModemId(enModemId);

    /* ��ʼ��CHDATA CFG */
    for (ulCnt = 0; ulCnt <= TAF_MAX_CID; ulCnt++)
    {
        AT_CleanDataChannelCfg(&(pstPsCtx->astChannelCfg[ulCnt]));
    }

    /* ��ʼ�������� */
    pstPsCtx->enPsErrCause = TAF_PS_CAUSE_SUCCESS;

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    /* ��ʼ��IP��ַ��enIfaceId��ӳ��� */
    memset_s(pstPsCtx->aulIpAddrIfaceIdMap, sizeof(pstPsCtx->aulIpAddrIfaceIdMap), 0x00, sizeof(pstPsCtx->aulIpAddrIfaceIdMap));
#else
    /* ��ʼ��IP��ַ��RABID��ӳ��� */
    memset_s(pstPsCtx->aulIpAddrRabIdMap, sizeof(pstPsCtx->aulIpAddrRabIdMap), 0x00, sizeof(pstPsCtx->aulIpAddrRabIdMap));
#endif



#if (FEATURE_IMS == FEATURE_ON)
    memset_s(&(pstPsCtx->stImsEmcRdp), sizeof(AT_IMS_EMC_RDP_STRU), 0x00, sizeof(AT_IMS_EMC_RDP_STRU));
#endif

    return;
}


VOS_VOID AT_InitModemPrivacyFilterCtx(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    AT_MODEM_PRIVACY_FILTER_CTX_STRU   *pstFilterCtx = VOS_NULL_PTR;

    pstFilterCtx = AT_GetModemPrivacyFilterCtxAddrFromModemId(enModemId);

    memset_s(pstFilterCtx,
             (VOS_UINT32)sizeof(AT_MODEM_PRIVACY_FILTER_CTX_STRU),
             0,
             (VOS_UINT32)sizeof(AT_MODEM_PRIVACY_FILTER_CTX_STRU));

    pstFilterCtx->ucFilterEnableFlg = VOS_FALSE;

    return;
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID AT_InitModemCdmaModemSwitchCtx(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    AT_MODEM_CDMAMODEMSWITCH_CTX_STRU  *pstCdmaModemSwitchCtx = VOS_NULL_PTR;

    pstCdmaModemSwitchCtx = AT_GetModemCdmaModemSwitchCtxAddrFromModemId(enModemId);

    memset_s(pstCdmaModemSwitchCtx,
             (VOS_UINT32)sizeof(AT_MODEM_CDMAMODEMSWITCH_CTX_STRU),
             0x00,
             (VOS_UINT32)sizeof(AT_MODEM_CDMAMODEMSWITCH_CTX_STRU));

    pstCdmaModemSwitchCtx->ucEnableFlg = VOS_FALSE;

    return;
}
#endif

VOS_VOID AT_InitModemImsCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_MODEM_IMS_CONTEXT_STRU           *pstImsCtx = VOS_NULL_PTR;

    pstImsCtx = AT_GetModemImsCtxAddrFromModemId(enModemId);

    memset_s(pstImsCtx, sizeof(AT_MODEM_IMS_CONTEXT_STRU), 0x00, sizeof(AT_MODEM_IMS_CONTEXT_STRU));

    pstImsCtx->stBatteryInfo.enCurrBatteryInfo = AT_IMSA_BATTERY_STATUS_BUTT;
    pstImsCtx->stBatteryInfo.enTempBatteryInfo = AT_IMSA_BATTERY_STATUS_BUTT;

    return;
}


VOS_VOID AT_InitClientConfiguration(VOS_VOID)
{
    VOS_UINT8                           i;
    AT_CLIENT_CTX_STRU                 *pstClientCtx = VOS_NULL_PTR;

    for (i = 0; i < AT_CLIENT_BUTT; i++)
    {
        pstClientCtx = AT_GetClientCtxAddr(i);

        pstClientCtx->stClientConfiguration.ucReportFlg = VOS_TRUE;
        pstClientCtx->stClientConfiguration.enModemId   = MODEM_ID_0;
    }

    return;
}


VOS_VOID AT_InitResetCtx(VOS_VOID)
{
    AT_RESET_CTX_STRU                   *pstResetCtx = VOS_NULL_PTR;

    memset_s(&g_stAtStatsInfo, (VOS_UINT32)sizeof(g_stAtStatsInfo), 0x00, (VOS_UINT32)sizeof(g_stAtStatsInfo));

    pstResetCtx = AT_GetResetCtxAddr();

    pstResetCtx->hResetSem     = VOS_NULL_PTR;
    pstResetCtx->ulResetingFlag = VOS_FALSE;

    /* ����������ź��� */
    if (VOS_SmBCreate( "AT", 0, VOS_SEMA4_FIFO, &pstResetCtx->hResetSem) != VOS_OK)
    {
        PS_PRINTF_WARNING("Create AT acpu cnf sem failed!\n");
        AT_DBG_SET_SEM_INIT_FLAG(VOS_FALSE);
        AT_DBG_CREATE_BINARY_SEM_FAIL_NUM(1);

        return;
    }
    else
    {
        AT_DBG_SAVE_BINARY_SEM_ID(pstResetCtx->hResetSem);
    }

    AT_DBG_SET_SEM_INIT_FLAG(VOS_TRUE);

    return;
}


VOS_VOID AT_InitReleaseInfo(VOS_VOID)
{
    memset_s(&g_stReleaseInfo, (VOS_UINT32)sizeof(g_stReleaseInfo), 0x00, (VOS_UINT32)sizeof(g_stReleaseInfo));

    g_stReleaseInfo.enAccessStratumRel = AT_ACCESS_STRATUM_REL9;

    return;
}

#if (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)

VOS_VOID AT_InitWakelock(VOS_VOID)
{
    memset_s(&g_stAtWakeLock,
             sizeof(g_stAtWakeLock),
             0,
             sizeof(g_stAtWakeLock));

    wakeup_source_init(&g_stAtWakeLock, "appds_wakelock");
}
#endif

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_VOID AT_InitUartCtx(VOS_VOID)
{
    AT_UART_CTX_STRU                   *pstUartCtx = VOS_NULL_PTR;
    VOS_UINT8                           ucCallId;

    pstUartCtx     = AT_GetUartCtxAddr();

    memset_s(pstUartCtx, (VOS_UINT32)sizeof(AT_UART_CTX_STRU), 0x00, (VOS_UINT32)sizeof(AT_UART_CTX_STRU));

    /* ��ʼ��UART��ά�ɲ���Ϣ */
    AT_InitHsUartStats();

    /* ��ʼ��UART�����ʣ�֡��ʽĬ��ֵ */
    pstUartCtx->stPhyConfig.enBaudRate            = AT_UART_DEFAULT_BAUDRATE;
    pstUartCtx->stPhyConfig.stFrame.enFormat      = AT_UART_DEFAULT_FORMAT;
    pstUartCtx->stPhyConfig.stFrame.enParity      = AT_UART_DEFAULT_PARITY;

    /* ��ʼ��UART LINE CTRLĬ��ֵ */
    pstUartCtx->stLineCtrl.enDcdMode              = AT_UART_DEFAULT_DCD_MODE;
    pstUartCtx->stLineCtrl.enDtrMode              = AT_UART_DEFAULT_DTR_MODE;
    pstUartCtx->stLineCtrl.enDsrMode              = AT_UART_DEFAULT_DSR_MODE;

    /* ��ʼ��FLOW CTRLĬ��ֵ */
    pstUartCtx->stFlowCtrl.enDceByDte             = AT_UART_DEFAULT_FC_DCE_BY_DTE;
    pstUartCtx->stFlowCtrl.enDteByDce             = AT_UART_DEFAULT_FC_DTE_BY_DCE;

    /* ��ʼ��RI�źŲ���Ĭ��ֵ */
    pstUartCtx->stRiConfig.ulSmsRiOnInterval      = AT_UART_DEFAULT_SMS_RI_ON_INTERVAL;
    pstUartCtx->stRiConfig.ulSmsRiOffInterval     = AT_UART_DEFAULT_SMS_RI_OFF_INTERVAL;
    pstUartCtx->stRiConfig.ulVoiceRiOnInterval    = AT_UART_DEFAULT_VOICE_RI_ON_INTERVAL;
    pstUartCtx->stRiConfig.ulVoiceRiOffInterval   = AT_UART_DEFAULT_VOICE_RI_OFF_INTERVAL;
    pstUartCtx->stRiConfig.ucVoiceRiCycleTimes    = AT_UART_DEFAULT_VOICE_RI_CYCLE_TIMES;

    /* ��ʼ��RI�ź�״̬ */
    pstUartCtx->stRiStateInfo.ulRunFlg            = VOS_FALSE;
    pstUartCtx->stRiStateInfo.enType              = AT_UART_RI_TYPE_BUTT;

    pstUartCtx->stRiStateInfo.hVoiceRiTmrHdl      = VOS_NULL_PTR;
    pstUartCtx->stRiStateInfo.enVoiceRiTmrStatus  = AT_TIMER_STATUS_STOP;
    pstUartCtx->stRiStateInfo.ulVoiceRiCycleCount = 0;

    for (ucCallId = 0; ucCallId <= MN_CALL_MAX_NUM; ucCallId++)
    {
        pstUartCtx->stRiStateInfo.aenVoiceRiStatus[ucCallId] = AT_UART_RI_STATUS_STOP;
    }

    pstUartCtx->stRiStateInfo.hSmsRiTmrHdl        = VOS_NULL_PTR;
    pstUartCtx->stRiStateInfo.ulSmsRiOutputCount  = 0;
    pstUartCtx->stRiStateInfo.enSmsRiTmrStatus    = AT_TIMER_STATUS_STOP;

    pstUartCtx->ulTxWmHighFlg                     = VOS_FALSE;
    pstUartCtx->pWmLowFunc                        = VOS_NULL_PTR;

    return;
}
#endif

VOS_VOID AT_InitCommCtx(VOS_VOID)
{
    AT_COMM_CTX_STRU                   *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();

    /* Ĭ��Ӧ������ΪMP */
    pstCommCtx->ucSystemAppConfigAddr = SYSTEM_APP_MP;

    memset_s(&pstCommCtx->stCustomUsimmCfg,
             sizeof(NAS_NVIM_CUSTOM_USIMM_CFG_STRU),
             0,
             sizeof(NAS_NVIM_CUSTOM_USIMM_CFG_STRU));

    /* ��ʼ��PS�򹫹��������� */
    AT_InitCommPsCtx();

    AT_InitCommPbCtx();

    AT_InitCmdProcCtx();

    AT_InitReleaseInfo();
#if (FEATURE_ECALL == FEATURE_ON)
    AT_InitCommEcallCtx();
#endif /* FEATURE_ON == FEATURE_ECALL */

    return;
}


VOS_VOID AT_InitClientCtx(VOS_VOID)
{
    AT_InitClientConfiguration();
}


VOS_VOID AT_InitModemCtx(MODEM_ID_ENUM_UINT16 enModemId)
{
    AT_InitUsimStatus(enModemId);

    AT_InitPlatformRatList(enModemId);

    AT_InitModemCcCtx(enModemId);

    AT_InitModemSsCtx(enModemId);

    AT_InitModemSmsCtx(enModemId);

    AT_InitModemNetCtx(enModemId);

    AT_InitModemAgpsCtx(enModemId);

    AT_InitModemPsCtx(enModemId);

    AT_InitModemImsCtx(enModemId);

    AT_InitModemPrivacyFilterCtx(enModemId);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    AT_InitModemCdmaModemSwitchCtx(enModemId);
#endif
    return;
}


VOS_VOID AT_InitCtx(VOS_VOID)
{
    MODEM_ID_ENUM_UINT16                enModemId;

    AT_InitCommCtx();

    for (enModemId = 0; enModemId < MODEM_ID_BUTT; enModemId++)
    {
        AT_InitModemCtx(enModemId);
    }

    AT_InitClientCtx();

    return;
}


VOS_VOID AT_CleanDataChannelCfg(
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg
)
{
    pstChanCfg->ulUsed          = VOS_FALSE;
    pstChanCfg->ulRmNetId       = AT_PS_INVALID_RMNET_ID;
    pstChanCfg->ulIfaceId       = AT_PS_INVALID_IFACE_ID;
    pstChanCfg->ulIfaceActFlg   = VOS_FALSE;
    pstChanCfg->enPortIndex     = AT_CLIENT_BUTT;
}


MODEM_ID_ENUM_UINT16 AT_GetModemIDFromPid(VOS_UINT32 ulPid)
{
#if (MULTI_MODEM_NUMBER >= 2)
    VOS_UINT32                          ulModemPidTabLen;
    VOS_UINT32                          i;

    ulModemPidTabLen    = (sizeof(g_astAtModemPidTab)/sizeof(AT_MODEM_PID_TAB_STRU));

    for (i = 0; i < ulModemPidTabLen; i++)
    {

        if (ulPid == g_astAtModemPidTab[i].ulModem0Pid)
        {
            return MODEM_ID_0;
        }

        if (ulPid == g_astAtModemPidTab[i].ulModem1Pid)
        {
            return MODEM_ID_1;
        }

#if (3 == MULTI_MODEM_NUMBER)
        if (ulPid == g_astAtModemPidTab[i].ulModem2Pid)
        {
            return MODEM_ID_2;
        }
#endif
    }

    return MODEM_ID_BUTT;
#else
    return MODEM_ID_0;
#endif
}



VOS_UINT8* AT_GetSystemAppConfigAddr(VOS_VOID)
{
    AT_COMM_CTX_STRU                   *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();

    return &(pstCommCtx->ucSystemAppConfigAddr);
}


AT_RESET_CTX_STRU* AT_GetResetCtxAddr(VOS_VOID)
{
    return &(g_stAtResetCtx);
}


AT_COMM_CTX_STRU* AT_GetCommCtxAddr(VOS_VOID)
{
    return &(g_stAtCommCtx);
}


AT_COMM_PS_CTX_STRU* AT_GetCommPsCtxAddr(VOS_VOID)
{
    return &(g_stAtCommCtx.stPsCtx);
}

#if (FEATURE_IMS == FEATURE_ON)

AT_IMS_EMC_RDP_STRU* AT_GetImsEmcRdpByClientId(VOS_UINT16 usClientId)
{
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;

    if (AT_GetModemIdFromClient(usClientId, &enModemId) != VOS_OK)
    {
        AT_ERR_LOG("AT_GetImsEmcRdpByClientId: ClientId is invalid.");
        return VOS_NULL_PTR;
    }

    return &(AT_GetModemPsCtxAddrFromModemId(enModemId)->stImsEmcRdp);
}
#endif


AT_COMM_PB_CTX_STRU* AT_GetCommPbCtxAddr(VOS_VOID)
{
    return &(g_stAtCommCtx.stCommPbCtx);
}


AT_CMD_MSG_NUM_CTRL_STRU* AT_GetMsgNumCtrlCtxAddr(VOS_VOID)
{
    return &(g_stAtCommCtx.stMsgNumCtrlCtx);
}


AT_MODEM_CTX_STRU* AT_GetModemCtxAddr(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId]);
}


AT_USIM_INFO_CTX_STRU* AT_GetUsimInfoCtxFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stAtUsimInfoCtx);
}


AT_MODEM_SPT_RAT_STRU* AT_GetSptRatFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stPlatformCapList.stPlatformRatList);
}




AT_MODEM_CC_CTX_STRU* AT_GetModemCcCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stCcCtx);
}


AT_MODEM_CC_CTX_STRU* AT_GetModemCcCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemCcCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_astAtModemCtx[enModemId].stCcCtx);
}


AT_MODEM_SS_CTX_STRU* AT_GetModemSsCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stSsCtx);
}


AT_MODEM_SS_CTX_STRU* AT_GetModemSsCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemSsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_astAtModemCtx[enModemId].stSsCtx);
}

AT_MODEM_SMS_CTX_STRU* AT_GetModemSmsCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stSmsCtx);
}


AT_MODEM_SMS_CTX_STRU* AT_GetModemSmsCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemSmsCtxAddrFromClientId: Get modem id fail");
    }

    return &(g_astAtModemCtx[enModemId].stSmsCtx);
}



AT_MODEM_NET_CTX_STRU* AT_GetModemNetCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stNetCtx);
}


AT_MODEM_NET_CTX_STRU* AT_GetModemNetCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemNetCtxAddrFromClientId: Get modem id fail");
    }

    return &(g_astAtModemCtx[enModemId].stNetCtx);
}

AT_MODEM_AGPS_CTX_STRU* AT_GetModemAgpsCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stAgpsCtx);
}

AT_MODEM_AGPS_CTX_STRU* AT_GetModemAgpsCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemAgpsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_astAtModemCtx[enModemId].stAgpsCtx);
}


AT_MODEM_PS_CTX_STRU* AT_GetModemPsCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stPsCtx);
}


AT_MODEM_PS_CTX_STRU* AT_GetModemPsCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemPsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_astAtModemCtx[enModemId].stPsCtx);
}


AT_MODEM_IMS_CONTEXT_STRU* AT_GetModemImsCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stAtImsCtx);
}

AT_MODEM_IMS_CONTEXT_STRU* AT_GetModemImsCtxAddrFromClientId(
    VOS_UINT16                          usClientId
)
{
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient((VOS_UINT8)usClientId, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_GetModemImsCtxAddrFromClientId: Get modem id fail.");
    }

    return &(g_astAtModemCtx[enModemId].stAtImsCtx);
}


VOS_UINT32 AT_GetPsIPv6IIDTestModeConfig(VOS_VOID)
{
    return AT_GetCommPsCtxAddr()->ulIpv6AddrTestModeCfg;
}


AT_CLIENT_CTX_STRU* AT_GetClientCtxAddr(
    AT_CLIENT_ID_ENUM_UINT16            enClientId
)
{
    return &(g_astAtClientCtx[enClientId]);
}


VOS_UINT32 AT_GetModemIdFromClient(
    VOS_UINT16                          usClientId,
    MODEM_ID_ENUM_UINT16               *pModemId
)
{
    AT_CLIENT_CTX_STRU                 *pstAtClientCtx = VOS_NULL_PTR;

    /* �ж��Ƿ���MODEM0�㲥��client index */
    if ((usClientId == AT_BROADCAST_CLIENT_INDEX_MODEM_0)
     || (usClientId == AT_BROADCAST_CLIENT_ID_MODEM_0))
    {
        *pModemId = MODEM_ID_0;
    }
    /* �ж��Ƿ���MODEM1�㲥��client index */
    else if ((usClientId == AT_BROADCAST_CLIENT_INDEX_MODEM_1)
          || (usClientId == AT_BROADCAST_CLIENT_ID_MODEM_1))
    {
        *pModemId = MODEM_ID_1;
    }
    /* �ж��Ƿ���MODEM2�㲥��client index */
    else if ((usClientId == AT_BROADCAST_CLIENT_INDEX_MODEM_2)
          || (usClientId == AT_BROADCAST_CLIENT_ID_MODEM_2))
    {
        *pModemId = MODEM_ID_2;
    }
    /* �ǹ㲥client index */
    else
    {
        /* client index ��Ч��ֱ�ӷ��� */
        if (usClientId >= AT_CLIENT_BUTT)
        {
            return VOS_ERR;
        }

        pstAtClientCtx = AT_GetClientCtxAddr(usClientId);

        *pModemId = pstAtClientCtx->stClientConfiguration.enModemId;
    }

    /* �ڵ�����ʱ��NV���������MODEMIDΪMODEM1ʱ���ᷢ���ڴ�Խ�磬�˴������쳣���� */
    if (*pModemId >= MODEM_ID_BUTT)
    {
        AT_ERR_LOG("AT_GetModemIdFromClient: modem id is invalid");

        *pModemId = MODEM_ID_0;
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 AT_GetDestPid(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT32                          ulRcvPid
)
{
#if (1 < MULTI_MODEM_NUMBER)
    VOS_UINT32                          ulRslt;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          i;

    enModemId = MODEM_ID_0;

    /* ��ȡclient id��Ӧ��Modem Id */
    ulRslt = AT_GetModemIdFromClient(usClientId, &enModemId);

    /* modem 1��ulRcvPid��Ӧ��pid */
    if ((ulRslt == VOS_OK)
     && (enModemId != MODEM_ID_0))
    {
        for (i = 0; i < (sizeof(g_astAtModemPidTab)/sizeof(AT_MODEM_PID_TAB_STRU)); i++)
        {
            if (ulRcvPid != g_astAtModemPidTab[i].ulModem0Pid)
            {
                continue;
            }

            if (enModemId == MODEM_ID_1)
            {
                return g_astAtModemPidTab[i].ulModem1Pid;
            }

            if (enModemId == MODEM_ID_2)
            {
                return g_astAtModemPidTab[i].ulModem2Pid;
            }
        }

        /* ��������Ҳ�����Ӧ��PIDӦ��ʱ������ */
        if (i >= (sizeof(g_astAtModemPidTab)/sizeof(AT_MODEM_PID_TAB_STRU)))
        {
            PS_PRINTF_WARNING("<AT_GetDestPid> usClientId is %d, ulRcvPid is %d no modem1 pid. \n", usClientId, ulRcvPid);
        }
    }
#endif

    return ulRcvPid;
}


VOS_UINT8 AT_IsModemSupportRat(
    MODEM_ID_ENUM_UINT16                enModemId,
    TAF_MMA_RAT_TYPE_ENUM_UINT8         enRat
)
{
    AT_MODEM_SPT_RAT_STRU              *pstSptRatList = VOS_NULL_PTR;

    pstSptRatList = AT_GetSptRatFromModemId(enModemId);
    if (enRat == TAF_MMA_RAT_LTE)
    {
        return pstSptRatList->ucPlatformSptLte;
    }
    if (enRat == TAF_MMA_RAT_WCDMA)
    {
        if ((pstSptRatList->ucPlatformSptWcdma == VOS_TRUE)
         || (pstSptRatList->ucPlatformSptUtralTDD == VOS_TRUE))
        {
            return VOS_TRUE;
        }
        else
        {
            return VOS_FALSE;
        }
    }
    if (enRat == TAF_MMA_RAT_GSM)
    {
        return pstSptRatList->ucPlatformSptGsm;
    }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (enRat == TAF_MMA_RAT_NR)
    {
        return pstSptRatList->ucPlatformSptNR;
    }
#endif

    return VOS_FALSE;
}


VOS_UINT8 AT_IsModemSupportUtralTDDRat(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    AT_MODEM_SPT_RAT_STRU              *pstSptRatList = VOS_NULL_PTR;

    pstSptRatList = AT_GetSptRatFromModemId(enModemId);

    return pstSptRatList->ucPlatformSptUtralTDD;
}





TAF_CS_CAUSE_ENUM_UINT32 AT_GetCsCallErrCause(
    VOS_UINT16                          usClientId
)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;

    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(usClientId);

    return pstCcCtx->enCsErrCause;
}



VOS_VOID AT_UpdateCallErrInfo(
    VOS_UINT16                          usClientId,
    TAF_CS_CAUSE_ENUM_UINT32            enCsErrCause,
    TAF_CALL_ERROR_INFO_TEXT_STRU      *pstErrInfoText
)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;
    errno_t                             lMemResult;

    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(usClientId);

    pstCcCtx->enCsErrCause = enCsErrCause;

    memset_s(&(pstCcCtx->stErrInfoText),
             (VOS_SIZE_T)sizeof(pstCcCtx->stErrInfoText),
             0x00,
             (VOS_SIZE_T)sizeof(pstCcCtx->stErrInfoText));

    if (pstErrInfoText != VOS_NULL_PTR)
    {
        if (pstErrInfoText->ucTextLen > 0)
        {
            pstCcCtx->stErrInfoText.ucTextLen = TAF_MIN(pstErrInfoText->ucTextLen, TAF_CALL_ERROR_INFO_TEXT_STRING_SZ);

            lMemResult = memcpy_s(pstCcCtx->stErrInfoText.acErrInfoText,
                                  (VOS_SIZE_T)sizeof(pstCcCtx->stErrInfoText.acErrInfoText),
                                  pstErrInfoText->acErrInfoText,
                                  pstCcCtx->stErrInfoText.ucTextLen);
            TAF_MEM_CHK_RTN_VAL(lMemResult, (VOS_SIZE_T)sizeof(pstCcCtx->stErrInfoText.acErrInfoText), pstCcCtx->stErrInfoText.ucTextLen);
        }
    }

    return;
}


TAF_CALL_ERROR_INFO_TEXT_STRU * AT_GetCallErrInfoText(
    VOS_UINT16                          usClientId
)
{
    return &(AT_GetModemCcCtxAddrFromClientId(usClientId)->stErrInfoText);
}


AT_ABORT_CMD_PARA_STRU* AT_GetAbortCmdPara(VOS_VOID)
{
    return &(gstAtAbortCmdCtx.stAtAbortCmdPara);
}


VOS_UINT8* AT_GetAbortRspStr(VOS_VOID)
{
    return (gstAtAbortCmdCtx.stAtAbortCmdPara.aucAbortAtRspStr);
}



VOS_UINT32 AT_GetSsCustomizePara(AT_SS_CUSTOMIZE_TYPE_UINT8 enSsCustomizeType)
{
    VOS_UINT8                           ucMask;

    if (g_stAtSsCustomizePara.ucStatus != VOS_TRUE)
    {
        return VOS_FALSE;
    }

    ucMask = (VOS_UINT8)((VOS_UINT32)AT_SS_CUSTOMIZE_SERVICE_MASK << enSsCustomizeType);
    if ((g_stAtSsCustomizePara.ucSsCmdCustomize & ucMask) != 0)
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;

}


VOS_SEM AT_GetResetSem(VOS_VOID)
{
    return g_stAtResetCtx.hResetSem;
}


VOS_UINT32 AT_GetResetFlag(VOS_VOID)
{
    return g_stAtResetCtx.ulResetingFlag;
}


VOS_VOID AT_SetResetFlag(VOS_UINT32 ulFlag)
{
    g_stAtResetCtx.ulResetingFlag = ulFlag;
    return;
}



AT_MODEM_PRIVACY_FILTER_CTX_STRU* AT_GetModemPrivacyFilterCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stFilterCtx);
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

AT_MODEM_CDMAMODEMSWITCH_CTX_STRU* AT_GetModemCdmaModemSwitchCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stCdmaModemSwitchCtx);
}
#endif

AT_MODEM_MT_INFO_CTX_STRU* AT_GetModemMtInfoCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stMtInfoCtx);
}


VOS_VOID AT_InitTraceMsgTab(VOS_VOID)
{
    memset_s(g_stAtTraceMsgIdTab, sizeof(g_stAtTraceMsgIdTab), 0xFF, sizeof(g_stAtTraceMsgIdTab));
}


AT_INTER_MSG_ID_ENUM_UINT32 AT_GetResultMsgID(VOS_UINT8 ucIndex)
{
    AT_INTER_MSG_ID_ENUM_UINT32         enResultMsgID;

    if (ucIndex == AT_BROADCAST_CLIENT_INDEX_MODEM_0)
    {
        enResultMsgID = ID_AT_MNTN_RESULT_BROADCAST_MODEM_0;
    }
    else if (ucIndex == AT_BROADCAST_CLIENT_INDEX_MODEM_1)
    {
        enResultMsgID = ID_AT_MNTN_RESULT_BROADCAST_MODEM_1;
    }
    else if (ucIndex == AT_BROADCAST_CLIENT_INDEX_MODEM_2)
    {
        enResultMsgID = ID_AT_MNTN_RESULT_BROADCAST_MODEM_2;
    }
    else
    {
        enResultMsgID = g_stAtTraceMsgIdTab[ucIndex].enResultMsgID;
    }

    return enResultMsgID;
}


AT_INTER_MSG_ID_ENUM_UINT32 AT_GetCmdMsgID(VOS_UINT8 ucIndex)
{
    return g_stAtTraceMsgIdTab[ucIndex].enCmdMsgID;
}


VOS_VOID AT_ConfigTraceMsg(
    VOS_UINT8                           ucIndex,
    AT_INTER_MSG_ID_ENUM_UINT32         enCmdMsgId,
    AT_INTER_MSG_ID_ENUM_UINT32         enResultMsgId
)
{
    g_stAtTraceMsgIdTab[ucIndex].enCmdMsgID = enCmdMsgId;
    g_stAtTraceMsgIdTab[ucIndex].enResultMsgID = enResultMsgId;

    return;
}


VOS_VOID At_SetAtCmdAbortTickInfo(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulTick
)
{
    gstAtAbortCmdCtx.stCmdAbortTick.ulAtSetTick[ucIndex]    = ulTick;

    return;
}


AT_CMD_ABORT_TICK_INFO* At_GetAtCmdAbortTickInfo(VOS_VOID)
{
    return &(gstAtAbortCmdCtx.stCmdAbortTick);
}


VOS_UINT8 At_GetAtCmdAnyAbortFlg(VOS_VOID)
{
    return (gstAtAbortCmdCtx.stAtAbortCmdPara.ucAnyAbortFlg);
}


VOS_VOID At_SetAtCmdAnyAbortFlg(
    VOS_UINT8                           ucFlg
)
{
    gstAtAbortCmdCtx.stAtAbortCmdPara.ucAnyAbortFlg = ucFlg;

    return;
}


AT_UART_CTX_STRU* AT_GetUartCtxAddr(VOS_VOID)
{
    return &(g_stAtCommCtx.stUartCtx);
}


AT_UART_PHY_CFG_STRU* AT_GetUartPhyCfgInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->stPhyConfig);
}


AT_UART_LINE_CTRL_STRU* AT_GetUartLineCtrlInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->stLineCtrl);
}


AT_UART_FLOW_CTRL_STRU* AT_GetUartFlowCtrlInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->stFlowCtrl);
}


AT_UART_RI_CFG_STRU* AT_GetUartRiCfgInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->stRiConfig);
}


AT_UART_RI_STATE_INFO_STRU* AT_GetUartRiStateInfo(VOS_VOID)
{
    return &(AT_GetUartCtxAddr()->stRiStateInfo);
}


AT_PORT_BUFF_CFG_STRU* AT_GetPortBuffCfgInfo(VOS_VOID)
{
    return &(AT_GetCommCtxAddr()->stPortBuffCfg);
}


AT_PORT_BUFF_CFG_ENUM_UINT8  AT_GetPortBuffCfg(VOS_VOID)
{
    AT_COMM_CTX_STRU                   *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();

    return pstCommCtx->stPortBuffCfg.enSmsBuffCfg;
}


VOS_VOID AT_InitPortBuffCfg(VOS_VOID)
{
    AT_PORT_BUFF_CFG_STRU              *pstUsedClientIdTab = VOS_NULL_PTR;

    pstUsedClientIdTab = AT_GetPortBuffCfgInfo();

    memset_s(pstUsedClientIdTab, sizeof(AT_PORT_BUFF_CFG_STRU), 0xFF, sizeof(AT_PORT_BUFF_CFG_STRU));

    pstUsedClientIdTab->ucNum = 0;
}


VOS_VOID AT_AddUsedClientId2Tab(VOS_UINT16 usClientId)
{
    AT_PORT_BUFF_CFG_STRU              *pstPortBuffCfg = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    if (usClientId >= AT_MAX_CLIENT_NUM)
    {
        return;
    }

    pstPortBuffCfg = AT_GetPortBuffCfgInfo();

    /*  �ų��ڲ�ͨ�� */
    if ( (usClientId >= AT_MIN_APP_CLIENT_ID)
      && (usClientId <= AT_MAX_APP_CLIENT_ID))
    {
        return;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    if (usClientId == AT_CLIENT_ID_SOCK)
    {
        return;
    }
#endif

    if (pstPortBuffCfg->ucNum >= AT_MAX_CLIENT_NUM)
    {
        pstPortBuffCfg->ucNum = AT_MAX_CLIENT_NUM -1;
    }

    /* ѭ�������Ƿ��Ѿ���¼�� */
    for (ucIndex = 0; ucIndex < pstPortBuffCfg->ucNum; ucIndex++)
    {
        if (usClientId == pstPortBuffCfg->ulUsedClientID[ucIndex])
        {
            return;
        }
    }

    /* ���û�м�¼�������¼����� */
    if (ucIndex == pstPortBuffCfg->ucNum)
    {
        pstPortBuffCfg->ulUsedClientID[ucIndex] = (VOS_UINT32)usClientId;
        pstPortBuffCfg->ucNum++;
    }
}


VOS_VOID AT_RmUsedClientIdFromTab(VOS_UINT16 usClientId)
{
    AT_PORT_BUFF_CFG_STRU              *pstPortBuffCfg = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;

    pstPortBuffCfg = AT_GetPortBuffCfgInfo();

    if (pstPortBuffCfg->ucNum > AT_MAX_CLIENT_NUM)
    {
        pstPortBuffCfg->ucNum = AT_MAX_CLIENT_NUM;
    }

    /* ѭ�������Ƿ��Ѿ���¼�� */
    for (ulIndex = 0; ulIndex < pstPortBuffCfg->ucNum; ulIndex++)
    {
        if (usClientId == pstPortBuffCfg->ulUsedClientID[ulIndex])
        {
            break;
        }
    }

    /* ���û�м�¼����ֱ���˳� */
    if (ulIndex == pstPortBuffCfg->ucNum)
    {
        return;
    }

    /* ����ҵ���ɾ����Ӧ��client */
    if (ulIndex == pstPortBuffCfg->ucNum - 1)
    {
        pstPortBuffCfg->ucNum--;
        pstPortBuffCfg->ulUsedClientID[ulIndex] = VOS_NULL_DWORD;

    }
    else
    {
        pstPortBuffCfg->ulUsedClientID[ulIndex] =
                    pstPortBuffCfg->ulUsedClientID[pstPortBuffCfg->ucNum - 1];
        pstPortBuffCfg->ulUsedClientID[pstPortBuffCfg->ucNum - 1] = VOS_NULL_DWORD;
        pstPortBuffCfg->ucNum--;
    }
}



AT_CLIENT_CONFIGURATION_STRU* AT_GetClientConfig(
    AT_CLIENT_ID_ENUM_UINT16            enClientId
)
{
    return &(AT_GetClientCtxAddr(enClientId)->stClientConfiguration);
}


AT_CLIENT_CFG_MAP_TAB_STRU* AT_GetClientCfgMapTbl(VOS_UINT8 ucIndex)
{
    return &(g_astAtClientCfgMapTbl[ucIndex]);
}



VOS_UINT8 AT_GetPrivacyFilterEnableFlg(VOS_VOID)
{
    return VOS_TRUE;
}


VOS_UINT8 AT_IsSupportReleaseRst(
    AT_ACCESS_STRATUM_REL_ENUM_UINT8    enReleaseType
)
{
#if (FEATURE_LTE == FEATURE_ON)
    if (enReleaseType <= g_stReleaseInfo.enAccessStratumRel)
    {
        return VOS_TRUE;
    }
#endif

#if(FEATURE_ON == FEATURE_UE_MODE_NR)
    /* ĿǰAT_IsSupportReleaseRst����ʱ����ξ���Release 11������NR�꿪���ĳ���������Ҫ����VOS_TRUE */
    return VOS_TRUE;
#else
    return VOS_FALSE;
#endif
}


VOS_VOID AT_ConvertCellIdToHexStrFormat(
    VOS_UINT32                          ulCellIdentityLowBit,
    VOS_UINT32                          ulCellIdentityHighBit,
    VOS_CHAR                           *pcCellIdStr
)
{
    VOS_UINT32                          ulLength;

    ulLength    = 0;

    if (ulCellIdentityHighBit == 0)
    {
        /* CellId��4�ֽ���Ч */
        ulLength = (VOS_UINT32)VOS_nsprintf_s((VOS_CHAR *)pcCellIdStr,
                                              AT_CELLID_STRING_MAX_LEN,
                                              AT_CELLID_STRING_MAX_LEN,
                                              "%X",
                                              ulCellIdentityLowBit);

    }
    else
    {
        /* CellId��4�ֽ���Ч */
        ulLength = (VOS_UINT32)VOS_nsprintf_s((VOS_CHAR *)pcCellIdStr,
                                              AT_CELLID_STRING_MAX_LEN,
                                              AT_CELLID_STRING_MAX_LEN,
                                              "%X%08X",
                                              ulCellIdentityHighBit,
                                              ulCellIdentityLowBit);
    }

    pcCellIdStr[ulLength] = 0;

    return;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT8* AT_GetModemCLModeCtxAddrFromModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return &(g_astAtModemCtx[enModemId].stPlatformCapList.ucIsCLMode);
}
#endif

VOS_UINT8 AT_GetCgpsCLockEnableFlgByModemId(
    MODEM_ID_ENUM_UINT16                enModemId
)
{
    return g_astAtModemCtx[enModemId].stAgpsCtx.ucAtCgpsClockEnableFlag;
}


VOS_VOID AT_SetCgpsCLockEnableFlgByModemId(
    MODEM_ID_ENUM_UINT16                enModemId,
    VOS_UINT8                           ucEnableFLg
)
{
    g_astAtModemCtx[enModemId].stAgpsCtx.ucAtCgpsClockEnableFlag = ucEnableFLg;
}

VOS_UINT8 At_GetGsmConnectRate(VOS_VOID)
{
    return g_stDialConnectDisplayRate.ucGsmConnectRate;
}

VOS_UINT8 At_GetGprsConnectRate(VOS_VOID)
{
    return g_stDialConnectDisplayRate.ucGprsConnectRate;
}

VOS_UINT8 At_GetEdgeConnectRate(VOS_VOID)
{
    return g_stDialConnectDisplayRate.ucEdgeConnectRate;
}

VOS_UINT8 At_GetWcdmaConnectRate(VOS_VOID)
{
    return g_stDialConnectDisplayRate.ucWcdmaConnectRate;
}

VOS_UINT8 At_GetDpaConnectRate(VOS_VOID)
{
    return g_stDialConnectDisplayRate.ucDpaConnectRate;
}



TAF_MMA_SIMSQ_STATE_ENUM_UINT32 At_GetSimsqStatus(MODEM_ID_ENUM_UINT16 enModemId)
{
    return AT_GetUsimInfoCtxFromModemId(enModemId)->enSimsqStatus;
}


VOS_VOID At_SetSimsqStatus(
    MODEM_ID_ENUM_UINT16                enModemId,
    TAF_MMA_SIMSQ_STATE_ENUM_UINT32     enSimsqStatus
)
{
    AT_USIM_INFO_CTX_STRU              *pstUsimmInfoCtx = VOS_NULL_PTR;

    pstUsimmInfoCtx = AT_GetUsimInfoCtxFromModemId(enModemId);

    pstUsimmInfoCtx->enSimsqStatus = enSimsqStatus;

    return;
}


VOS_UINT8 At_GetSimsqEnable(VOS_VOID)
{
    return AT_GetCommCtxAddr()->stCustomUsimmCfg.ucSimsqEnable;
}


VOS_VOID At_SetSimsqEnable(VOS_UINT8 ucSimsqEnable)
{
    AT_COMM_CTX_STRU                   *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();

    pstCommCtx->stCustomUsimmCfg.ucSimsqEnable = ucSimsqEnable;

    return;
}
#if (FEATURE_ECALL == FEATURE_ON)

VOS_VOID AT_InitCommEcallCtx(VOS_VOID)
{
    AT_COMM_CTX_STRU                    *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();

    memset_s(&(pstCommCtx->stEcallAtCtx), sizeof(AT_CMD_ECALL_CTX_STRU), 0x00, sizeof(AT_CMD_ECALL_CTX_STRU));
    pstCommCtx->stEcallAtCtx.ucEcallMode = 0; /* auto mode*/
    return;
}


AT_ECALL_ALACK_INFO_STRU* AT_EcallAlAckInfoAddr(VOS_VOID)
{
    AT_COMM_CTX_STRU                    *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();
    return &(pstCommCtx->stEcallAtCtx.stEcallAlackInfo);
}

VOS_VOID AT_SetEclModeValue(VOS_UINT8 ucEclmode)
{
    AT_COMM_CTX_STRU                    *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();
    pstCommCtx->stEcallAtCtx.ucEcallMode = ucEclmode;
}


VOS_UINT8 AT_GetEclModeValue(VOS_VOID)
{
    AT_COMM_CTX_STRU                    *pstCommCtx = VOS_NULL_PTR;

    pstCommCtx = AT_GetCommCtxAddr();
    return pstCommCtx->stEcallAtCtx.ucEcallMode;
}
#endif

