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

#ifndef __MDRV_IPC_ENUM_H__
#define __MDRV_IPC_ENUM_H__
#ifdef __cplusplus
extern "C"
{
#endif

/*lint --e{488,830}*/
/* ����������*/
typedef enum tagIPC_INT_CORE_E
{
    IPC_CORE_APPARM = 0x0,
    IPC_CORE_COMARM,
    IPC_CORE_ACORE = 0x0,   /*P531,V7R2*/
    IPC_CORE_CCORE,
    IPC_CORE_MCORE,
    IPC_CORE_LDSP,
    IPC_CORE_HiFi,
    IPC_CORE_BBE,           /* xdsp core id */
    IPC_CORE_NXDSP,
    IPC_CORE_NRCCPU,
    /* !!!!����Ԫ�������ӵ����  */

     /* !!!!NRCPU CORE-ID ö��ֵ����  */
    //ACORE MCORE NRCPU ��ö��ֵ��AP ��IPC ö��ֵһ�£�����HL1C��L2HAC��LL1C
    IPC_CORE_HL1C = 3,
    IPC_CORE_L2HAC,
    IPC_CORE_LL1C,

    /* !!!!����Ԫ�������ӵ����  */
    IPC_CORE_BUTTOM = 8,
}IPC_INT_CORE_E;

/*********************************************************
*  ������IPC��Դ��ö��������ʽ:
*  IPC_<Ŀ�괦����>_INT_SRC_<Դ������>_<����/����>
*  Ŀ�괦����:ACPU��CCPU��MCU��HIFI��BBE16
*  Դ������  :ACPU��CCPU��MCU��HIFI��BBE16
*  ����/���� :
*********************************************************/
typedef enum tagIPC_INT_LEV_E
{
    IPC_CCPU_INT_SRC_HIFI_MSG              = 0, /* HIFI�����Ϣ֪ͨ */
    IPC_CCPU_INT_SRC_MCU_MSG               = 1, /* MCU�����Ϣ֪ͨ */
    IPC_INT_DSP_HALT                       = 2, /* DSP֪ͨARM˯��*/
    IPC_INT_DSP_RESUME                     = 3, /* DSP֪ͨARM��ɻ��Ѻ�Ļָ�����*/
                                                /* 4, ռλ��������������ֵ,��ֵ������IPC_INT_DICC_USRDATAһ��*/
                                                /* 5, ռλ��������������ֵ,��ֵ������IPC_INT_DICC_USRDATAһ��*/
    /* ����TDSʹ�õ�����IPC�ж�,begin */
    IPC_INT_WAKE_SLAVE                     = 6, /* BBE16֪ͨCCPU,����GSM��ģ */
    IPC_CCPU_INT_SRC_DSP_DVS,                  /* BBE16֪ͨCCPU��BBP��dsp���е�ѹ */
    IPC_INT_DSP_PS_PUB_MBX                 ,/* ����DSP->ARM,��͹���IPC�жϸ��� */
    IPC_INT_DSP_PS_MAC_MBX                 ,/* ����DSP->ARM,��͹���IPC�жϸ��� */
    IPC_INT_DSP_MBX_RSD                    ,/* ����DSP->ARM,��͹���IPC�жϸ��� */
    IPC_CCPU_INT_SRC_DSP_MNTN              ,/* CCPU->DSP DVFSģ���Ƶ��ѹ��֪ͨ NXP */
    /* ����TDSʹ�õ�����IPC�ж�,end */
    IPC_CCPU_INT_SDR_CCPU_BBP_MASTER_ERROR ,   /* BBE16֪ͨC�˷�����ͨ���쳣 */
    IPC_CCPU_INT_SRC_LDSP_OM_MBX,               /* ����DSP->ARM, ͨ��������Ϣ���� */
    IPC_CCPU_INT_TLMODEM1_WAKE_SLAVE,           /* ����BBE16֪ͨCCPU,����GUTX��ģ */
    IPC_CCPU_INT_TLMODEM1_MAC_MBX,              /* ����BBE16֪ͨCCPU, ר������ */
    IPC_CCPU_INT_SRC_ACPU_RESET            ,   /* A/C����C�˵�����λ�ڼ�˼�ͨ��(����C��), v8r2�ߵ��Ǻ˼���Ϣ */
    IPC_CCPU_SRC_ACPU_DUMP                 ,   /* v7r2 ��ά�ɲ⣬ACPU ���͸�CCPU ��IPC�ж� */
    IPC_CCPU_INT_SRC_ICC_PRIVATE           ,   /* v7r2 ICCͨ�ţ�iccר�÷�����Ϣʹ��,ACPU���͸�CCPU��IPC�ж�       */
    IPC_CCPU_INT_SRC_MCPU                  ,   /* ICC��Ϣ, MCU���͸�CCPU��  IPC�ж� */
    IPC_CCPU_INT_SRC_MCPU_WDT              ,   /* ����traceʱͣwdt */
    IPC_CCPU_INT_SRC_XDSP_1X_HALT          ,   /* 1X Halt�ж� */
    IPC_CCPU_INT_SRC_XDSP_HRPD_HALT        ,   /* HRPD Halt�ж� */
    IPC_CCPU_INT_SRC_XDSP_1X_RESUME        ,   /* 1X Resume�ж� */
    IPC_CCPU_INT_SRC_XDSP_HRPD_RESUME      ,   /* HRPD Resume�ж� */
    IPC_CCPU_INT_SRC_XDSP_MNTN             ,   /* XDSP�����쳣ά���ж� */
    IPC_CCPU_INT_SRC_XDSP_PS_MBX           ,   /* PS��Xģ����ͨ��ʹ�õ��ж� */
    IPC_CCPU_INT_SRC_XDSP_DVS              ,   /* CBBE16֪ͨCCPU��dsp���е�Ƶ��ѹ */
    IPC_CCPU_INT_SRC_NRCCPU_BBP_WAKEUP     = 28,   /* 5G BBP ����4G CCPU ʹ���ж� */

    IPC_CCPU_INT_SRC_ACPU_IPC_EXTEND = 30, /* acpu����ccpu�ж�,������չIPC �ж�*/
    /*  austin��Ŀ�� mcu ͨ��ʹ�� icc �жϺ�, ���ܱ�� */
    IPC_CCPU_INT_SRC_ACPU_ICC              = 31, /* acpu����ccpu�ж�*/

    /*  ��ȫICCͨ��ʹ���жϺ�, ���ܱ�� */
    IPC_CCPU_INT_SRC_SECOS_ICC_IFC         = 32, /* ��Ӧ��ȫIPC��0���жϣ���ȫICC����ͨ���ж� */
    IPC_CCPU_INT_SRC_SECOS_ICC_VSIM        = 33, /* ��Ӧ��ȫIPC��1���жϣ���ȫICC���ͨͨ���ж� */

    /*modem �ڲ�IPC*/
    IPC_CCPU_INT_SRC_LTE0_HALT   =  64 ,   /*LTE0 (  ����) ��CCPU ��HALT  �ж�*/
    IPC_CCPU_INT_SRC_LTE1_HALT  ,             /*LTE1 (  ����) ��CCPU ��HALT  �ж�*/
    IPC_CCPU_INT_SRC_TDS_HALT  ,              /*TDS  ��CCPU��HALT�ж�*/
    IPC_CCPU_INT_SRC_LTE0_RESUME  ,   /*LTE0 (  ����) ��CCPU ��RESUME �ж�*/
    IPC_CCPU_INT_SRC_LTE1_RESUME  ,   /*LTE1 (  ����) ��CCPU ��RESUME  �ж�*/
    IPC_CCPU_INT_SRC_TDS_RESUME  ,           /* TDS ��CCPU ��RESUME  �ж�*/
    IPC_CCPU_INT_SRC_HISPEED_MBX  ,           /* ��������DSP->ARM �ж�*/
    IPC_CCPU_INT_SRC_LTEV_PS_MBX,               /* LTEV_DSP->ARM, ps */
    IPC_CCPU_INT_SRC_LTEV_OM_MBX,               /* LTEV_DSP->ARM, om */
    IPC_CCPU_INT_SRC_LTEV_MAC_MBX,               /* ����LTEV_DSP->ARM, MAC���� */
    IPC_CCPU_INT_SRC_DSPV_HALT,                       /* LTEVDSP֪ͨARM˯��*/
    IPC_CCPU_INT_SRC_DSPV_RESUME,                     /* LTEVDSP֪ͨARM��ɻ��Ѻ�Ļָ�����*/
    IPC_CCPU_INT_SRC_LTEV_HALT,                       /* LTEVģʽ֪ͨARM˯��*/
    IPC_CCPU_INT_SRC_LTEV_RESUME,                     /* LTEVģʽ֪ͨARM��ɻ��Ѻ�Ļָ�����*/
    IPC_CCPU_INT_SRC_DSPV_DVS_REQ,                  /* BBE16֪ͨCCPU��BBP��dsp���е�ѹ */
    IPC_INT_LETV_WAKE_SLAVE,                     /* LTEV֪ͨCCPU,����LTE��ģ */

    IPC_INNER_INT_TEST        = 90,

    /* ����MCU IPC�����Ϣ�ж�Դbitλ�� */
    IPC_MCU_INT_SRC_ACPU_MSG             = 0,    /* ACPU�����Ϣ֪ͨ */
    IPC_MCU_INT_SRC_CCPU_MSG                ,    /* CCPU�����Ϣ֪ͨ */
    IPC_MCU_INT_SRC_HIFI_MSG                ,    /* HIFI�����Ϣ֪ͨ */
    IPC_MCU_INT_SRC_CCPU_IPF                ,    /* IPF�͹��� */
    IPC_MCU_INT_SRC_ACPU_PD                 ,    /* acpu power down */
    IPC_MCU_INT_SRC_HIFI_PD                 ,    /* hifi power down */
    IPC_MCU_INT_SRC_HIFI_DDR_VOTE           ,    /* HIFI DDR��ˢ��ͶƱ */
    IPC_MCU_INT_SRC_ACPU_I2S_REMOTE_SLOW    ,
    IPC_MCU_INT_SRC_ACPU_I2S_REMOTE_SLEEP   ,
    IPC_MCU_INT_SRC_ACPU_I2S_REMOTE_INVALID ,
    /* ACPU��CCPU����IPC�ж�ʱ����Ҫͬʱ��֪��M3 */
    IPC_MCU_INT_SRC_ACPU_DRX                ,
    IPC_MCU_INT_SRC_CCPU_DRX                ,   /* CCPU��ACPU����IPC�ж�ʱ����Ҫͬʱ��֪��M3 */
    IPC_MCU_INT_SRC_ICC_PRIVATE             ,   /* m3 icc˽�е�ipc�жϣ�������ͨ���������������� */
    IPC_MCU_INT_SRC_DUMP                    ,   /* m3 dump�жϣ�ACORE���͸�MCU��IPC�ж�*/
    IPC_MCU_INT_SRC_HIFI_PU                 ,   /* POWERUP,  HIFI���͸�MCU��IPC�ж� */
    IPC_MCU_INT_SRC_HIFI_DDR_DFS_QOS        , /* HIFI��DDR��ƵͶƱ */
    IPC_MCU_INT_SRC_TEST                    ,   /* for test a\c interact with m3 */
    IPC_MCPU_INT_SRC_ACPU_USB_PME_EN        ,  /* acore��M3ͨ��USB��������¼� */
    IPC_MCU_INT_SRC_NR_CCPU_START           ,  /* ֪ͨMCU����NR CCPU */
    IPC_MCU_INT_SRC_LR_CCPU_START           ,
    IPC_MCU_INT_SRC_NRCCPU_PD               ,  /* ֪ͨMCU ��NRCCPU �µ� */

    /* ����3��austin�޸ģ���AP���룬�����޸� */
    IPC_MCU_INT_SRC_ICC                 = 29,   /* m3 icc���õ�ipc�ж� */
    IPC_MCU_INT_SRC_CCPU_PD             = 30,    /* ccpu power down */
    IPC_MCU_INT_SRC_CCPU_START          = 31,   /* ֪ͨMCU����LR CCPU */

    /* ����ACPU IPC�����Ϣ�ж�Դbitλ�� */
    IPC_ACPU_INT_SRC_CCPU_MSG             = 0,   /* CCPU�����Ϣ֪ͨ */
    IPC_ACPU_INT_SRC_HIFI_MSG             = 1,   /* HIFI�����Ϣ֪ͨ */
    IPC_ACPU_INT_SRC_MCU_MSG              = 2,   /* ACPU�����Ϣ֪ͨ */
    IPC_ACPU_INT_SRC_CCPU_NVIM            = 3,   /* ����NVIMģ��C����A��֮���ͬ��*/
    IPC_INT_DICC_USRDATA                  = 4,   /* ������CCPU�����жϺ�ͬʱ�޸�*/
    IPC_INT_DICC_RELDATA                  = 5,   /* ������CCPU�����жϺ�ͬʱ�޸�*/
    IPC_ACPU_INT_SRC_CCPU_ICC             ,
    IPC_ACPU_INT_SRC_ICC_PRIVATE          ,   /* v7r2 icc ˽�ã�CCPU���͸�ACPU��IPC�ж� */
    IPC_ACPU_SRC_CCPU_DUMP                ,   /* v7r2 ��ά�ɲ⣬CCPU ���͸�ACPU ��IPC�ж� */
    IPC_ACPU_INT_SRC_MCPU                 ,   /* ICC��Ϣ, MCU���͸�ACPU�� IPC�ж� */
    IPC_ACPU_INT_SRC_MCPU_WDT             ,  /* ����traceʱͣwdt */
    IPC_ACPU_INT_MCU_SRC_DUMP             ,  /* dump�ж�?MCU���͸�ACPU��IPC�ж� */
    IPC_ACPU_INT_SRC_CCPU_RESET_IDLE      ,  /* A/C����C�˵�����λ�ڼ�˼�ͨ��(master in idle)��Ҧ���� */
    IPC_ACPU_INT_SRC_CCPU_RESET_SUCC      ,  /* A/C����C�˵�����λ�ڼ�˼�ͨ��(ccore reset ok)��Ҧ���� */
    IPC_ACPU_INT_SRC_CCPU_LOG             ,  /* CCPU���͸�ACPU��LOG�����ж� */
    IPC_ACPU_INT_SRC_MCU_FOR_TEST         ,  /* test, m core to acore */
    IPC_ACPU_INT_SRC_CCPU_TEST_ENABLE     ,  /* IPC�Զ��������ж�ʹ��ʱʹ�ã�����IPC�жϲ���ʹ��ʱ����ɾ�� */
    IPC_ACPU_INT_SRC_MCPU_USB_PME         ,  /* M3��acore�ϱ�USB�����¼� */
    IPC_ACPU_INT_SRC_HIFI_PC_VOICE_RX_DATA,  /* hifi->acore pc voice */
    IPC_ACPU_INT_SRC_CCPU_PM_OM           ,  /* CCPU����ACPU��PMOM�ж�,����д�ļ�, Ҧ���� */
    IPC_ACPU_INT_SRC_CCPU_IPC_EXTEND =20,
    IPC_ACPU_INT_SRC_NRCCPU_RESET_SUCC ,
    IPC_ACPU_SRC_NRCCPU_DUMP,
    IPC_ACPU_INT_SRC_NRCCPU_RESET_IDLE,
    IPC_ACPU_INT_SRC_END,

    /* ����HIFI IPC�����Ϣ�ж�Դbitλ�� */
    IPC_HIFI_INT_SRC_ACPU_MSG = 0 ,  /* ACPU�����Ϣ֪ͨ */
    IPC_HIFI_INT_SRC_CCPU_MSG     ,  /* CCPU�����Ϣ֪ͨ */
    IPC_HIFI_INT_SRC_BBE_MSG      ,  /* TDDSP�����Ϣ֪ͨ */
    IPC_HIFI_INT_SRC_MCU_MSG      ,
    IPC_HIFI_INT_SRC_ACPU_ICC      ,  /* MBB platform acore 2 hifi icc */
    IPC_HIFI_INT_SRC_END          ,

    /* ����NXDSP IPC�����Ϣ�ж�Դbitλ�� */
    IPC_INT_MSP_DSP_OM_MBX = 0 ,   /* ����ARM->DSP */
    IPC_INT_PS_DSP_PUB_MBX     ,   /* ����ARM->DSP */
    IPC_INT_PS_DSP_MAC_MBX     ,   /* ����ARM->DSP */
    IPC_INT_HIFI_DSP_MBX       ,   /* HIFI->DSP */
    IPC_BBE16_INT_SRC_HIFI_MSG ,   /* ������Ϣ��HIFI���͸�BBE16��IPC�ж� */
    IPC_INT_MSP_DSP_LTE0_WAKE_UP  ,       /*CCPU ��DSP ��LTE0  �����ж�*/
    IPC_INT_MSP_DSP_LTE1_WAKE_UP  ,   /*CCPU ��DSP ��LTE1  �����ж�*/
    IPC_INT_MSP_DSP_TDS_WAKE_UP  ,    /*CCPU ��DSP ��TDS  �����ж�*/
    IPC_INT_MSP_DSP_HISPEED_MBX ,   /* ������������ARM->DSP */
    IPC_BBE16_INT_SRC_END  ,


    /* ����CBBE16 IPC�����Ϣ�ж�Դbitλ�� */
    IPC_XDSP_INT_SRC_CCPU_1X_WAKE =  IPC_BBE16_INT_SRC_END ,   /* 1X Wake�ж� */
    IPC_XDSP_INT_SRC_CCPU_HRPD_WAKE   ,   /* HRPD Wake�ж� */
    IPC_XDSP_INT_SRC_CCPU_OM_MBX      ,   /* C��->XDSP */
    IPC_XDSP_INT_SRC_CCPU_PUB_MBX     ,   /* C��->XDSP */
    IPC_XDSP_INT_SRC_CCPU_1X_WAKE_UP  ,   /*CCPU ��DSP ��1X  �����ж�*/
    IPC_XDSP_INT_SRC_CCPU_HRPD_WAKE_UP  ,     /*CCPU ��DSP ��HRPD  �����ж�*/
    IPC_XDSP_INT_SRC_END ,

    /* ����NRCCPU IPC�����Ϣ�ж�Դbitλ�� */
    IPC_NRCCPU_INT_SRC_LRCCPU_WAKEUP  = 0 ,  /*LTE CCPU ����or ����˯�� NR CCPU*/
    IPC_NRCCPU_SRC_ACPU_DUMP,
    IPC_NRCCPU_INT_SRC_ACPU_RESET,
    IPC_NRCCPU_INT_SRC_HL1C_PD               = 29, /* HL1C����nrccpu �µ��ж�*/
    IPC_NRCCPU_INT_SRC_ACPU_ICC              = 31, /* acpu����nrccpu�ж�*/

    /*5G modem �ڲ�IPC*/
    IPC_NRCCPU_INT_SRC_LL1C_PM = 64,

    IPC_NRCCPU_INT_SRC_L2HAC_PD = 65, 			/* L2HAC����nrccpu �µ��ж�*/
    IPC_NRCCPU_INT_SRC_END ,

    /* ����L2HAC IPC�����Ϣ�ж�Դbitλ�� */
    IPC_L2HAC_INT_SRC_NRCCPU_WAKEUP     = 64, /* NRCCPU ����L2HAC �����ж�*/
    IPC_L2HAC_INT_SRC_NRCCPU_HALT          = 65, /* NRCCPU ����L2HAC ����˯���ж�*/
    IPC_L2HAC_INT_SRC_END ,

    IPC_LTEV_INT_SRC_CCPU_PS_MBX = 0,           /* ARM->LTEV_DSP, ps */
    IPC_LTEV_INT_SRC_CCPU_OM_MBX,               /* ARM->LTEV_DSP, om */
    IPC_INT_MSP_DSP_LTEV_WAKE_UP  ,             /*CCPU ��DSP ��LTE0  �����ж�*/
    IPC_CCPU_INT_SRC_DSPV_DVS_CNF,              /* CCPU->DSP DVFSģ���Ƶ��ѹ��֪ͨ NXP */
    /* ������������� */
    IPC_INT_BUTTOM             = 96,
}IPC_INT_LEV_E;

typedef enum tagIPC_SEM_ID_E
{
    IPC_SEM_MEM          ,
    IPC_SEM_DICC         ,
    IPC_SEM_SYNC         ,
    IPC_SEM_SYSCTRL      ,
    IPC_SEM_BBP_RESERVED ,  /*��ֵ��BBE16����󶨣��޸���֪ͨBBE16����Ӧ�޸�*/
    IPC_SEM_LBBP1        ,  /*����bbp��ccore �� tlphy ʹ�� */
    IPC_SEM_NVIM         ,
    IPC_SEM_1X_MODE      ,  /* 1Xģʽ����GSDR�������� */
    IPC_SEM_DPDT_CTRL_ANT,  /* ����mdrv_ipc_spin_trylockʱ���� */
    IPC_SEM_BBPMASTER_0  ,
    IPC_SEM_BBPMASTER_1  ,
    IPC_SEM_BBPMASTER_2  ,
    IPC_SEM_BBPMASTER_3  ,
    IPC_SEM_BBPMASTER_5  ,
    IPC_SEM_BBPMASTER_6  ,
    IPC_SEM_BBPMASTER_7  ,
    IPC_SEM_NV           ,
    IPC_SEM_GPIO         ,
    IPC_SEM_PMU          ,
    IPC_SEM_IPF_PWCTRL   ,
    IPC_SEM_NV_CRC       ,
    IPC_SEM_PM_OM_LOG    ,
    IPC_SEM_MDRV_LOCK    ,
    IPC_SEM_CDMA_DRX     , /* C����XDSPʹ�� */
    IPC_SEM_GU_SLEEP     ,
    IPC_SEM2_IPC_TEST    , /*IPC�Զ�������ʹ��*/
    IPC_SEM_DFS_FIX      = 31, /*M3 DFS��HIFI�������APB����ʹ��,��HI6950оƬƽ̨��ʹ��*/

    IPC_SEM_CPM          = 64,
    IPC_SEM_CPM_1X       = 65,    /*Xģ�ںϵ�nxp������1X�˼���*/
    IPC_SEM_BBP          = 66,

    IPC_SEM_LTEV         = 67,     /*LTEV����1X�˼���*/

    IPC_SEM_INNER_TEST = 70,
    IPC_SEM_BUTTOM       = 96
} IPC_SEM_ID_E;

#ifdef __cplusplus
}
#endif

#endif