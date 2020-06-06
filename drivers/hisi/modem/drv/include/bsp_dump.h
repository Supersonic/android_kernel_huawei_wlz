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

#ifndef __BSP_DUMP_H__
#define __BSP_DUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <osl_spinlock.h>
#include <osl_common.h>
#include <osl_list.h>
#if defined(__KERNEL__) || defined(__VXWORKS__) || defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)
#include "mdrv_om.h"
#else
#include "mdrv_om_common.h"
#endif
#include "drv_comm.h"
#include "bsp_trace.h"
#include "bsp_om.h"
#if defined(__OS_RTOSCK__) || defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)
#include "sre_task.h"
#include "sre_exc.h"
#endif

#if defined(__KERNEL__)
#include <linux/stddef.h>
#include <linux/semaphore.h>
#endif

#include "bsp_om_enum.h"
#include "mdrv_errno.h"

/*****************************************************************************
  1 �궨��
*****************************************************************************/

#define OM_SYSCTRL_MAGIC        (0x5B5C5C5B)


/* dump�����ں��޸� begin */
#ifdef __KERNEL__
#define EXC_VEC_RESET       0x00    /* reset */
#define EXC_VEC_UNDEF       0x04    /* undefined instruction */
#define EXC_VEC_SWI         0x08    /* software interrupt */
#define EXC_VEC_PREFETCH    0x0c    /* prefetch abort */
#define EXC_VEC_DATA        0x10    /* data abort */
#define EXC_VEC_IRQ         0x18    /* interrupt */
#define EXC_VEC_FIQ         0x1C    /* fast interrupt */
#endif
/* dump�����ں��޸� end */

#define DUMP_INT_IN_FLAG                    (0xAAAA)
#define DUMP_FLAG_INT_ENTER                 (0xAAAA0000)
#define DUMP_INT_EXIT_FLAG                  (0xBBBB)
#define DUMP_FLAG_INT_EXIT                  (0xBBBB0000)
#define DUMP_INT_UNLOCK_FLAG                (0xCCCC)
#define DUMP_SAVE_SUCCESS                   (0xA4A4A4A4)
#define DUMP_TASK_INFO_SIZE                 (0x150)
#define DUMP_TASK_INFO_STACK_SIZE           (DUMP_TASK_INFO_SIZE - 32*4)


#define MODEM_DUMP_FILE_NAME_LENGTH         (128)

#define DUMP_FIELD_CPUINFO_SIZE             (1024)
#define TASK_NAME_LEN                       (16)
#define ARM_REGS_NUM                        (17)
#define DUMP_REG_SET_MAX                    (0x1000)

/*****************************************************************************
  2 ö�ٶ���
*****************************************************************************/
#define NVID_DUMP   (NV_ID_DRV_DUMP)

#define EICC_PACKET_SIZE (0x20)



typedef enum
{
    DUMP_MBB    = 0,
    DUMP_PHONE,
    DUMP_PRODUCT_BUTT
}dump_product_type_t;

typedef enum
{
    DUMP_ACCESS_MDD_DDR_NON_SEC =0,
    DUMP_ACCESS_MDD_DDR_SEC =1,
    DUMP_ACCESS_MDD_DDR_SEC_BUTT =2,

}dump_access_mdmddr_type_t;

typedef enum
{
    DUMP_CCPU_ARCH_VSMA    = 0,
    DUMP_CCPU_ARCH_PSMA,
    DUMP_CCPU_ARCH_BUTT
}dump_ccpu_arch_t;



/* ö��ֵ��dump_file_cfg_s���뱣��ƥ�� */


/* dump�ļ���㣬ioctl������ */

typedef enum
{
    DUMP_SAVE_FILE_NORMAL = 0x0,
    DUMP_SAVE_FILE_NEED   = 0xAABBCCDD,
    DUMP_SAVE_FILE_END    = 0x5A5A5A5A
}dump_save_flag_t;

typedef enum
{
    DUMP_START_POWER_ON = 0,
    DUMP_START_REBOOT   = 0x5A5A1111,
    DUMP_START_EXCH     = 0x5A5A2222,
    DUMP_START_CRASH    = 0x5A5A3333
}dump_start_flag_e;

typedef enum _dump_reboot_cpu_e
{
    DUMP_CPU_APP       = 0x01000000,
    DUMP_CPU_LRCCPU    = 0x02000000,
    DUMP_CPU_BBE       = 0x03000000,
    DUMP_CPU_LPM3      = 0x04000000,
    DUMP_CPU_IOM3      = 0x05000000,
    DUMP_CPU_HIFI      = 0x06000000,
    DUMP_CPU_NRCCPU    = 0x07000000,
    DUMP_CPU_NRL2HAC   = 0x08000000,
    DUMP_CPU_NRLL1C    = 0x09000000,
    DUMP_CPU_BUTTON    = 0x0a000000

}dump_reboot_cpu_t;

typedef enum _dump_reboot_reason_e
{
    DUMP_REASON_NORMAL           = 0x0,
    DUMP_REASON_ARM              = 0x1,
    DUMP_REASON_STACKFLOW        = 0x2,
    DUMP_REASON_WDT              = 0x3,
    DUMP_REASON_PMU              = 0x4,
    DUMP_REASON_REBOOT           = 0x5,
    DUMP_REASON_NOC              = 0x6,
    DUMP_REASON_DMSS             = 0x7,
    DUMP_REASON_RST_FAIL         = 0x8,
    DUMP_REASON_RST_NOT_SUPPORT  = 0x9,
    DUMP_REASON_DLOCK            = 0x10,
    DUMP_REASON_RST_FREQ         = 0x11,
    DUMP_REASON_PDLOCK           = 0x12,
    DUMP_REASON_AMON             = 0x13,
    DUMP_REASON_UNDEF            = 0xff
}dump_reboot_reason_t;

typedef struct _dump_exc_contex
{
   dump_reboot_cpu_t     reboot_core;
   dump_reboot_reason_t  reboot_reason;
} dump_reboot_contex_s;

#define DUMP_LOAD_MAGIC     0xEEEE1234
typedef struct _dump_load_info_s
{
    u32 magic_num;      /* dump������Ϣ����ʶ�����ڼ���֮ǰ�汾 */
    u32 mdm_ddr;        /* MODEM DDR���ص�ַ */
    u32 mdm_share;      /* MODEM�����ڴ���ص�ַ */
    u32 mdm_dump;       /* MODEM��ά�ɲ��ڴ���ص�ַ */
    u32 mdm_sram;       /* MODEM SRAM���ص�ַ */
    u32 mdm_dts;        /* MODEM DTS���ص�ַ */
    u32 mdm_secshare;   /* MODEM��ȫ�����ڴ���ص�ַ */
    u32 mdm_llram;      /* MODEM��ȫ�����ڴ���ص�ַ */
    u64 ap_ddr;         /* AP DDR���ص�ַ */
    u64 ap_share;       /* AP�����ڴ���ص�ַ */
    u64 ap_dump;        /* AP��ά�ɲ��ڴ���ص�ַ */
    u64 ap_sram;        /* AP SRAM���ص�ַ */
    u64 ap_dts;         /* AP DTS���ص�ַ */
    u32 lpm3_tcm0;      /* LPM3 TCM0���ص�ַ */
    u32 lpm3_tcm1;      /* LPM3 TCM1���ص�ַ */
    u32 mdm_ddr_saveoff;    /* MODEM DDR����binʱ��ƫ��(ȥ��text��)*/
    u32 mdm_share_ddr_size;    /* MODEM DDR����binʱ��ƫ��(ȥ��text��)*/

    u32 nrccpu_ddr;        /* NRCCPUʹ�õ�DDR�ռ� */
    u32 nr_share;          /* NRʹ�õĹ����ڴ�ռ� */
    u32 nrccpu_llram;      /* nrccpu��llram�ռ� */
    u32 nrl2hac_llram;      /* nrl2hac��llram�ռ� */
    u32 nrl2hac_ddr;      /* nrl2hac��ddr�ռ� */
    u32 reversed;      /* �ֽڶ������ */
}dump_load_info_t;
/*��ַ�������Ҫ��ƫ��offset��¼��dump�ռ�β��*/
typedef struct
{
    u32 start_magic;              /*0x5a5a5a5a*/
    /*����aslr��Ҫ������������*/
    u64 acpu_aslr_offset;
    u32 reserved;
    u32 nrccpu_aslr_offset;
    u32 ccpu_aslr_offset;
    u32 end_magic;                /*0xa5a5a5a5*/
}dump_aslr_info_t;

/*RTOSck������ṹ��*/
typedef struct tagListObject
{
    struct tagListObject *pstPrev;
    struct tagListObject *pstNext;
} LIST_OBJECT_S;

typedef struct tagMsgQHead
{
    u32 uwMsgNum;
    LIST_OBJECT_S stMsgList;
} MSG_QHead_S;

#if defined(__OS_RTOSCK__)
typedef struct tagPublicTskCB
{
    VOID               *pStackPointer;              // ��ǰ�����SP
    TSK_STATUS_T        usTaskStatus;               // ����״̬
    TSK_PRIOR_T         usPriority;                 // ������������ȼ�
    UINT32              uwStackSizes;                // ����ջ��С
    UINT32              uwTopOfStack;               // ����ջ��
    TSK_HANDLE_T        uwTaskPID;                  // ����PID
    TSK_ENTRY_FUNC      pfnTaskEntry;               // ������ں���
    VOID               *pTaskSem;                   // ����Pend���ź���ָ��
    INT32               swFsemCount;                // �����ź�������
    UINT32              auwArgs[4];                 // ����Ĳ���
    TSK_PRIOR_T         usOrigPriority;             // �����ԭʼ���ȼ�
    UINT16              usStackCfgFlg;              // ����ջ���ñ��
    UINT16              usQNum;                     // ��Ϣ������
    UINT16              usRecvQID;                  // ����������Ϣ��QID
    UINT32              uwPrivateData;              // ˽������
    MSG_QHead_S        *pstMsgQ;                    // ָ����Ϣ��������
    LIST_OBJECT_S       stPendList;                 // �ź�������ָ��
    LIST_OBJECT_S       stTimerList;                // ������ʱ����ָ��
    LIST_OBJECT_S       stSemBList;                 // ���л����ź�������
    UINT64              ullExpirationTick;          // ����ָ���ʱ���(��λTick)
    UINT32              uwEvent;                    // �����¼�
    UINT32              uwEventMask;                // �����¼�����
    UINT32              uwLastErr;                  // �����¼�����һ��������
    UINT32              uwReserved;                 // ����һ��PAD����֤TCB 8�ֽڶ���
} TSK_CB_S;
#elif  defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)
#include <mdrv_thread.h>
typedef struct tagPushablTskList
{
    UINT32 uwPrio;
    LIST_OBJECT_S stPrioList;
    LIST_OBJECT_S stNodeList;
}PUSHABL_TLIST;
typedef struct tagPublicTskCB
{
    VOID               *pStackPointer;              // ��ǰ�����SP
    TSK_STATUS_T           usTaskStatus;               // ����״̬
#if (OS_HARDWARE_PLATFORM == OS_CORTEX_RX)
    UINT16              uwReserved1;                // ��ʹ�ã���֤���Զ�usTaskStatus����32λ��ԭ�Ӳ���
#endif
    TSK_PRIOR_T            usPriority;                 // ������������ȼ�
#if (OS_HARDWARE_PLATFORM == OS_CORTEX_RX)
    UINT16              usSpinLockCount;            // ������е�spinlock������
    UINT32              uwLastSpinLock;             // ������е����һ��spinlock
    volatile UINT32     uwTaskOperating;            // �������ڽ��еĲ�������
    UINT32              uwReserved2;                // ����
#endif
    UINT32              uwStackSize;                // ����ջ��С
    UINT32              uwTopOfStack;               // ����ջ��
    TSK_HANDLE_T           uwTaskPID;                  // ����PID��SMP��Ϊ����OS�ڲ�����,AMP��Ϊ�˺� | ��������
    TSK_ENTRY_FUNC        pfnTaskEntry;               // ������ں���
    VOID               *pTaskSem;                   // ����Pend���ź���ָ��
    INT32               swFsemCount;                // �����ź�������
    UINT32              auwArgs[4];                 // ����Ĳ���
#if (OS_OPTION_COPROCESSOR == YES)                  // ֻ��Tensilicaƽ̨���иù���
    VOID               *pCpSaveAreaA;               // ʸ���Ĵ��������ַA
    VOID               *pCpSaveAreaB;               // ʸ���Ĵ��������ַB
#endif
    TSK_PRIOR_T            usOrigPriority;             // �����ԭʼ���ȼ�
    UINT16              usStackCfgFlg;              // ����ջ���ñ��
    UINT16              usQNum;                     // ��Ϣ������
    UINT16              usRecvQID;                  // ����������Ϣ��QID
    UINT32              uwPrivateData;              // ˽������
    MSG_QHead_S        *pstMsgQ;                    // ָ����Ϣ��������
    LIST_OBJECT_S       stPendList;                 // �ź�������ָ��
    LIST_OBJECT_S       stTimerList;                // ������ʱ����ָ��
    LIST_OBJECT_S       stSemBList;                 // ���л����ź�������
    UINT64              ullExpirationTick;          // ����ָ���ʱ���(��λTick)
    UINT32              uwEvent;                    // �����¼�
    UINT32              uwEventMask;                // �����¼�����
    UINT32              uwLastErr;                  // �����¼�����һ��������
#if (OS_OPTION_SMP == YES)
    UINT32              uwTcbLock;                  // ����tcb�����˼������Ҫ����
    BOOL                bIsInterruptable;           // ���������ٽ���������־
    UINT32              uwSMPReserve;               // reserved
    PUSHABL_TLIST       stPushAbleList;             // ��push��������
    UINT32              uwCoreAllowedMask;          // ���������ִ�еĺ�bitmap
    UINT32              uwNrCoresAllowed;           // ���������ִ�еĺ˸���
    UINT32              uwCoreID;                   // ������������rq�ҽӵĺ˺�
    BOOL                bIsOnRq;                    // �������Ƿ������ж�����(ready)
    UINT32              uwTskLoad;                  // ������ĸ���
    void               *pstScheClass;               // ������ʹ�õĵ�����
#endif

    UINT32              uwReserved;                 // ����һ��PAD����֤TCB 8�ֽڶ���
} TSK_CB_S;
#elif  defined(__OS_RTOSCK_SMP__)
typedef struct tagPushablTskList
{
    UINT32 uwPrio;
    LIST_OBJECT_S stPrioList;
    LIST_OBJECT_S stNodeList;
}PUSHABL_TLIST;
typedef struct tagPublicTskCB
{
    VOID               *pStackPointer;              // ��ǰ�����SP
    TskStatus           usTaskStatus;               // ����״̬
#if (OS_HARDWARE_PLATFORM == OS_CORTEX_RX)
    UINT16              uwReserved1;                // ��ʹ�ã���֤���Զ�usTaskStatus����32λ��ԭ�Ӳ���
#endif
    TskPrior            usPriority;                 // ������������ȼ�
#if (OS_HARDWARE_PLATFORM == OS_CORTEX_RX)
    UINT16              usSpinLockCount;            // ������е�spinlock������
    UINT32              uwLastSpinLock;             // ������е����һ��spinlock
    volatile UINT32     uwTaskOperating;            // �������ڽ��еĲ�������
    UINT32              uwReserved2;                // ����
#endif
    UINT32              uwStackSize;                // ����ջ��С
    UINT32              uwTopOfStack;               // ����ջ��
    TskHandle           uwTaskPID;                  // ����PID��SMP��Ϊ����OS�ڲ�����,AMP��Ϊ�˺� | ��������
    TskEntryFunc        pfnTaskEntry;               // ������ں���
    VOID               *pTaskSem;                   // ����Pend���ź���ָ��
    INT32               swFsemCount;                // �����ź�������
    UINT32              auwArgs[4];                 // ����Ĳ���
#if (OS_OPTION_COPROCESSOR == YES)                  // ֻ��Tensilicaƽ̨���иù���
    VOID               *pCpSaveAreaA;               // ʸ���Ĵ��������ַA
    VOID               *pCpSaveAreaB;               // ʸ���Ĵ��������ַB
#endif
    TskPrior            usOrigPriority;             // �����ԭʼ���ȼ�
    UINT16              usStackCfgFlg;              // ����ջ���ñ��
    UINT16              usQNum;                     // ��Ϣ������
    UINT16              usRecvQID;                  // ����������Ϣ��QID
    UINT32              uwPrivateData;              // ˽������
    MSG_QHead_S        *pstMsgQ;                    // ָ����Ϣ��������
    LIST_OBJECT_S       stPendList;                 // �ź�������ָ��
    LIST_OBJECT_S       stTimerList;                // ������ʱ����ָ��
    LIST_OBJECT_S       stSemBList;                 // ���л����ź�������
    UINT64              ullExpirationTick;          // ����ָ���ʱ���(��λTick)
    UINT32              uwEvent;                    // �����¼�
    UINT32              uwEventMask;                // �����¼�����
    UINT32              uwLastErr;                  // �����¼�����һ��������
#if (OS_OPTION_SMP == YES)
    UINT32              uwTcbLock;                  // ����tcb�����˼������Ҫ����
    BOOL                bIsInterruptable;           // ���������ٽ���������־
    UINT32              uwSMPReserve;               // reserved
    PUSHABL_TLIST       stPushAbleList;             // ��push��������
    UINT32              uwCoreAllowedMask;          // ���������ִ�еĺ�bitmap
    UINT32              uwNrCoresAllowed;           // ���������ִ�еĺ˸���
    UINT32              uwCoreID;                   // ������������rq�ҽӵĺ˺�
    BOOL                bIsOnRq;                    // �������Ƿ������ж�����(ready)
    UINT32              uwTskLoad;                  // ������ĸ���
    void               *pstScheClass;               // ������ʹ�õĵ�����
#endif

    UINT32              uwReserved;                 // ����һ��PAD����֤TCB 8�ֽڶ���
} TSK_CB_S;

#endif

typedef struct
{
    u32 task_id;            /*����id*/
    u8  task_name[12];      /*������*/
}dump_task_info_s;


/*  CPSR R16
  31 30  29  28   27  26    7   6   5   4    3    2    1    0
----------------------------------------------------------------
| N | Z | C | V | Q | RAZ | I | F | T | M4 | M3 | M2 | M1 | M0 |
---------------------------------------------------------------- */

/*  REG
R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13/SP R14/LR R15/PC R16/CPSR
*/
typedef struct dump_baseinfo_head
{
    u32 reboot_cpu;          /*0x00  */
    u32 reboot_context;     /*0x00  */
    u32 reboot_task;        /*0x04  */
    u32 reboot_task_tcb;    /*0x04  */
    u8  taskName[16];       /*0x08  */
    u32 reboot_int;         /*0x18  */
    u32 reboot_time;        /*0x84 */

    u32 modId;              /*0x1c  */
    u32 arg1;               /*0x20  */
    u32 arg2;               /*0x24  */
    u32 arg3;               /*0x28  */
    u32 arg3_length;        /*0x2c  */
    u32 vec;                /*0x30  */

    u32 cpu_max_num;        /*0x88 */
    u32 cpu_online_num;     /*0x88 */
    u8  version[32];        /*0xA0 */
    u8  compile_time[32];   /*0xB0 */
    u32 reboot_reason;      /*m3ר��*/
    u8  version_uuid[40];


}dump_base_info_t;

typedef struct
{
    u32 reboot_context;     /*0x00  */
    u32 reboot_task;        /*0x04  */
    u8  taskName[16];       /*0x08  */
    u32 reboot_int;         /*0x18  */

}dump_cp_reboot_contex;



typedef struct
{
    u32 pid;
    u32 core;
    u32 entry;
    u32 status;
    u32 policy;
    u32 priority;
    u32 stack_base;
    u32 stack_end;
    u32 stack_high;
    u32 stack_current;
    u8  name[16];
    u32 regs[17];
    u32 offset;
    u32 rsv[1];
    char dump_stack[DUMP_TASK_INFO_STACK_SIZE];
} dump_task_info_t;



typedef enum _dump_reboot_ctx_e
{
    DUMP_CTX_TASK        = 0x0,
    DUMP_CTX_INT         = 0x1
}dump_reboot_ctx_t;


typedef struct
{
    u32 cpu_state;                      /*cpu ״̬*/
    dump_reboot_ctx_t current_ctx;      /*cpu������*/
    u32 current_int;                    /*��ǰ���ж�*/
    u32 current_task;                   /*��ǰ�������е�����*/
    u8  taskName[TASK_NAME_LEN];        /*������*/
    u32 regSet[ARM_REGS_NUM];           /*�Ĵ�����Ϣ*/
    u8  callstack[DUMP_FIELD_CPUINFO_SIZE - sizeof(dump_reboot_ctx_t) - sizeof(u32)- sizeof(u32)- TASK_NAME_LEN - sizeof(u32) * ARM_REGS_NUM];
}dump_cpu_info_t;

#define DUMP_QUEUE_MAGIC_NUM        (0xabcd6789)
typedef struct
{
    u32 magic;          /*0xabcd6789*/
    u32 maxNum;
    u32 front;
    u32 rear;
    u32 num;
    u32* data;
} dump_queue_t;



typedef enum{
    TASK_SCHED_NORMAL = 0,
    TASK_SCHED_FIFO,
    TASK_SCHED_RR,
    TASK_SCHED_BATCH,
    TASK_SCHED_IDLE
}dump_sched_policy_t;

typedef bool (*exc_hook)(void * param);

#define CCORE               "CP"
#define ACORE               "AP"
#define NRCCORE             "NRCP"
#define NRL2HACORE          "NRL2HAC"
#define UNKNOW_CORE         "UNKNOW"
#define UNKNOW              "UNKNOW"
#define DATA_ABORT          "abort"
#define WDT                 "wdg"
#define NOARMAL             "syserr"
#define UNDEFFINE           "undefine"
#define NOC                 "noc"
#define DMSS                "dmss"
#define RST_FAIL            "rst_fail"
#define RST_NOT_SUPPORT     "rst_not_support"
#define DLOCK               "mdm_bus_err"
#define RST_FREQ            "reset_freq"

#define DUMP_MODULE_NAME_LEN (12)

#define DUMP_REBOOT_INT "Interrupt_No"
#define DUMP_REBOOT_TASK "task_name"

#define DUMP_MODID_OFFSET        (offsetof(struct dump_baseinfo_head,modId))
#define DUMP_TASK_NAME_OFFSET    (offsetof(struct dump_baseinfo_head,taskName))
typedef enum
{
    DUMP_LR_SYS = 0,
    DUMP_NR_SYS = 1,
}DUMP_SYS;

typedef enum
{
    DUMP_SEC_MDM_DDR    = 0,
    DUMP_SEC_SEC_SHARED = 1,
    DUMP_SEC_LPHY_DUMP  = 2,
    DUMP_SEC_NRPHY_DUMP = 3,
    DUMP_SEC_EASYRF_DUMP = 4,
    DUMP_SEC_NRCCPU_DDR = 5,
    DUMP_SEC_L2HAC_LLRAM = 6,
    DUMP_SEC_PDE          = 7,
    DUMP_SEC_NRCCPU_LLRAM = 8,
    DUMP_SEC_NRSHARE = 9,
    DUMP_SEC_FILE_BUTT,

}DUMP_SEC_FILE_ENUM;
#define SUB_SYS_LR 0x01
#define SUB_SYS_NR 0x10
typedef void (*log_save_fun)(const char* path);
typedef struct
{
    struct list_head           list;
    log_save_fun               save_fun;
    u8                         name[16];
    u32                        modem_type;
}dump_log_notifier;
typedef struct
{
    spinlock_t                 lock;
    u32                        init_flag;
    struct list_head           log_list;
}dump_log_ctrl_s;


#define DUMP_STATUS_REGISTER 0x12345678
#define DUMP_STATUS_NONE     0xabcdabcd

typedef struct
{
    u32                    core;
    u32                    mod_id;
    u32                    rdr_mod_id;
    u32                    arg1;
    u32                    arg2;
    char*                  data;
    u32                    length;
    u32                    voice;
    u32                    int_no;
    u32                    task_id;
    dump_reboot_ctx_t      reboot_contex;
    u32                    timestamp;
    u32                    reason;
    u8                     task_name[16];
    u8                     exc_desc[48];
    struct list_head       exception_list;
    struct semaphore           sem_wait;
    u32                    status;

}dump_exception_info_s;

typedef enum
{
    STATE_IDLE,
    STATE_BUSY,
}DUMP_STATE;
#define DUMP_MODEM_OFF  0x78564321

#if defined(__KERNEL__)
typedef struct
{
    spinlock_t                 lock;
    u32                        init_flag;
    struct list_head           exception_list;
    uintptr_t                  exception_task_id;
    struct semaphore           sem_exception_task;
    struct semaphore           sem_wait;
    u32                        modem_off;
}dump_exception_ctrl_s;
#endif



struct dump_ddr_trans_head_info
{
    unsigned int magic;                 /*magic num  0x5678fedc */
    unsigned int packet_num;            /*BD������*/
    unsigned int total_length;          /*�ܰ���*/
    unsigned int lp_length;             /*���һ�����ĳ���*/
    char file_name[28];                 /*��Ҫ������ļ���*/
    unsigned int resv;
};

typedef struct
{
    u32 mod_id;
    u32 core;
}NR_EXC_INFO;

#define NR_EXCEPTION        (0xaa)
#define MDMAP_EXCEPTION     (0xbb)
#define L2HAC_EXCEPTION     (0xcc)
#define NRRDR_EXCEPTION     (0xdd)

typedef struct sub_sys
{
    unsigned int sysid;
    char desc[32];
}NR_EXCPITON_SUB_SYS;


/*****************************************************************************
  3 ��������
*****************************************************************************/
#ifdef ENABLE_BUILD_OM

void system_error (u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length);
s32  bsp_dump_init(void);
int bsp_dump_init_phase2(void);
dump_handle bsp_dump_register_hook(const char * name, dump_hook func);
s32  bsp_dump_unregister_hook(dump_handle handle);
u8*  bsp_dump_register_field(u32 mod_id,const char * name, void * virt_addr, void * phy_addr, u32 length, u16 version_id);
u8*  bsp_dump_get_field_addr(u32 field_id);
u8 * bsp_dump_get_field_phy_addr(u32 field_id);
int bsp_dump_save_all_task_name(void);
/* dump�����ں��޸� begin */
void adump_set_exc_vec(u32 vec);
void dump_int_enter(u32 dir, u32 irq_num);
void dump_int_exit(u32 dir, u32 irq_num);
int  dump_stack_trace_print(unsigned long where);
/* dump�����ں��޸� end */
s32  dump_exc_register(exc_hook func);
void bsp_dump_get_reset_modid(u32 reason, u32 reboot_modid, u32 * modId);
s32 bsp_dump_mark_voice(u32 flag);
void bsp_dump_get_reboot_contex(u32* core ,u32* reason);
int bsp_dump_sec_file_trans(DUMP_SEC_FILE_ENUM dumpfile,void* addr,unsigned int length);

dump_product_type_t dump_get_product_type(void);

#if defined(__OS_RTOSCK__) || defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)
void bsp_dump_save_regs(const void* reg_addr,u32 reg_size);
BOOL bsp_rtosck_exc_hook(struct ExcInfo *pstExcInfo);
#endif
#if defined(__KERNEL__)
s32 bsp_dump_register_log_notifier(u32 modem_type,log_save_fun save_fun,const char* name);
void bsp_dump_log_notifer_callback(u32 modem_type,const char* path);
s32 bsp_dump_unregister_log_notifier(log_save_fun save_fun);
bool bsp_dump_is_secdump(void);
dump_product_type_t  bsp_dump_get_product_type(void);
dump_access_mdmddr_type_t bsp_dump_get_access_mdmddr_type(void);
void bsp_dump_save_file(const char * dir,const char * file_name,const void * addr,const void* phy_addr, u32 len);

#endif
void bsp_dump_pcie_out_of_service(void);
#ifdef CONFIG_EARLY_LOG
void bsp_show_mdmext_pstore(void);
#else
static inline void bsp_show_mdmext_pstore(void){return ;}
#endif
#else

static void inline system_error (u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    return ;
}
static s32 inline bsp_dump_init(void)
{
    return 0;
}

static dump_handle inline bsp_dump_register_hook(const char * name, dump_hook func)
{
    return 0;
}

static s32 inline bsp_dump_unregister_hook(dump_handle handle)
{
    return 0;
}

static inline u8 * bsp_dump_register_field(u32 mod_id, const char * name, void * virt_addr, void * phy_addr, u32 length, u16 version_id)
{
    return BSP_NULL;
}

static inline u8 * bsp_dump_get_field_addr(u32 field_id)
{
    return 0;
}

static inline u8 * bsp_dump_get_field_phy_addr(u32 field_id)
{
    return 0;
}

static int inline bsp_dump_save_all_task_name(void)
{
    return 0;
}

static s32 inline dump_exc_register(exc_hook func)
{
    return 0;
}

static inline void  bsp_dump_save_regs(const void* reg_addr,u32 reg_size)
{
    return;
}
#if defined(__OS_RTOSCK__) || defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)

static inline BOOL bsp_rtosck_exc_hook(EXC_INFO_S *pstExcInfo)
{
    return true;
}
static inline s32 bsp_dump_mark_voice(u32 flag)
{
    return 0;
}
#endif

#if defined(__KERNEL__)
static inline s32 bsp_dump_register_log_notifier(u32 modem_type,log_save_fun save_fun,const char* name)
{
    return 0;
}
static inline void bsp_dump_log_notifer_callback(u32 modem_type,const char* path)
{
    return;
}
static inline s32 bsp_dump_unregister_log_notifier(log_save_fun save_fun)
{
    return 0;
}
static inline bool bsp_dump_is_secdump(void)
{
    return false;
}
static inline dump_product_type_t  bsp_dump_get_product_type(void)
{
    return DUMP_MBB;
}
static inline dump_access_mdmddr_type_t bsp_dump_get_access_mdmddr_type(void)
{
    return DUMP_ACCESS_MDD_DDR_SEC;
}
static inline void bsp_dump_save_file(const char * dir,const char * file_name, const void * addr, const void* phy_addr, u32 len)
{
    return;
}
#endif

static inline void bsp_dump_pcie_out_of_service(void){return;}

#endif


/*****************************************************************************
  4 ����������
*****************************************************************************/


#ifdef __cplusplus
}
#endif


#endif

