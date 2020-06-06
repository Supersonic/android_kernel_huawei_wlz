

#ifndef __OAL_WINDOWS_SCHEDULE_H__
#define __OAL_WINDOWS_SCHEDULE_H__

/* ����ͷ�ļ����� */
#include <windows.h>
#include <time.h>
#include "oal_util.h"

/* �궨�� */
/* typedef CRITICAL_SECTION    oal_spin_lock_stru; zouhongliang ����windows����linux�´�Сһ�� */
typedef struct {
    oal_uint8 auc_resv[4];
} oal_spin_lock_stru;

typedef LONG volatile oal_atomic;

typedef oal_uint32 (*oal_irqlocked_func)(oal_void *);

typedef CRITICAL_SECTION oal_rwlock_stru;

typedef oal_void (*oal_defer_func)(oal_uint);

typedef oal_void oal_wait_queue_stru;

typedef struct {
    oal_uint32 resv1[4];
    oal_uint32 resv2[4];
} oal_wait_queue_head_stru;

/* Tasklet is scheduled for execution */
#define TASKLET_STATE_SCHED 0

/* tasklet���� */
#define OAL_DECLARE_TASK(_name, _func, _p_data) \
    struct oal_tasklet_stru _name = { NULL, 0, _func, _p_data, 0, NULL, THREAD_PRIORITY_ABOVE_NORMAL }

OAL_STATIC OAL_INLINE oal_int32 oal_wait_event_interruptible_timeout(oal_wait_queue_head_stru st_wq,
                                                                     oal_uint8 uc_bool, oal_int i_time)
{
    return 50;
}

OAL_STATIC OAL_INLINE oal_int32 oal_wait_event_interruptible(oal_wait_queue_head_stru st_wq, oal_uint8 uc_bool)
{
    return 10;
}

OAL_STATIC OAL_INLINE oal_int32 oal_wait_event_timeout(oal_wait_queue_head_stru st_wq,
                                                       oal_uint8 uc_bool, oal_int i_time)
{
    return 50;
}
OAL_STATIC OAL_INLINE oal_long oal_wait_for_completion_interruptible_timeout(int *pst_completion, oal_ulong ul_timeout)
{
    return 0;
}

#define OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(_st_wq, _condition, _timeout) \
    oal_wait_event_interruptible_timeout((_st_wq), (_condition), (_timeout))

#define OAL_WAIT_EVENT_TIMEOUT(_st_wq, _condition, _timeout) oal_wait_event_timeout((_st_wq), (_condition), (_timeout))

#define OAL_WAIT_EVENT_INTERRUPTIBLE(_st_wq, _condition) oal_wait_event_interruptible((_st_wq), (_condition))

#define OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(_pst_wq) OAL_REFERENCE(_pst_wq)

#define OAL_WAIT_QUEUE_WAKE_UP(_pst_wq)

#define OAL_INTERRUPTIBLE_SLEEP_ON(_pst_wq)

#define OAL_WAIT_QUEUE_INIT_HEAD(_pst_wq)

/* ��ȡ���뼶ʱ��� */
#define OAL_TIME_GET_STAMP_MS()          clock()
#define OAL_TIME_GET_HIGH_PRECISION_MS() clock()

#define OAL_ENABLE_CYCLE_COUNT()
#define OAL_DISABLE_CYCLE_COUNT()
#define OAL_GET_CYCLE_COUNT()

/* �Ĵ�����תģ������ʱ����� */
#define OAL_TIME_CALC_RUNTIME(_ul_start, _ul_end) ((OAL_TIME_US_MAX_LEN) - (_ul_start) + (_ul_end))

#define OAL_TIME_JIFFY 15

#define OAL_TIME_HZ 1000

#define OAL_MSECS_TO_JIFFIES(_msecs) (_msecs)

#define OAL_JIFFIES_TO_MSECS(_jiffies) (_jiffies)

#define OAL_GET_REAL_TIME(_pst_tm) {}

#define OAL_INIT_COMPLETION(_my_completion) {}

#define OAL_COMPLETE(_my_completion) {}

OAL_STATIC OAL_INLINE oal_int32 oal_in_interrupt()
{
    return 0;
}

OAL_STATIC OAL_INLINE oal_int32 oal_in_atomic()
{
    return 0;
}

typedef void (*oal_timer_func)(oal_uint);

typedef struct {
    oal_uint32 ul_delay;
    oal_timer_func p_timer_func;
    oal_uint ui_arg;
    oal_uint32 ul_timer_id;
} oal_timer_list_stru;

/* ģ����� */
#define oal_module_init(_module_name)

#define oal_module_license(_license_name)

/* ģ����� */
#define oal_module_exit(_module_name)

#define oal_module_param(_symbol, _type, _name)

/* ģ����ŵ��� */
#define oal_module_symbol(_symbol)
#define OAL_MODULE_DEVICE_TABLE(_type, _name)

#define oal_smp_call_function_single(core, task, info, wait)
#define IS_ERR(x)             0
#define IS_ERR_OR_NULL(x)     0
#define OAL_IS_ERR_OR_NULL(x) 0

typedef int (read_proc_t)(oal_int8 *, oal_int8 **, oal_int,
    oal_int32, oal_int32 *, oal_void *);

typedef int (write_proc_t)(oal_file_stru *, const oal_int8 *,
    oal_uint32, oal_void *);

typedef struct {
    oal_uint32 ul_done;  // ��־�Ƿ��Ѿ����꣬δ������Ǹ�ֵ������ȴ����������Ϊ0
    oal_wait_queue_head_stru st_wait;
} oal_completion;

#define DECLARE_COMPLETION(work) oal_completion work

typedef struct {
    oal_int i_sec;
    oal_int i_usec;
} oal_time_us_stru;

typedef union {
    oal_int i_sec;
    oal_int i_usec;
} oal_time_t_stru;

/* �߳̽ṹ�� */
typedef struct {
    LPSECURITY_ATTRIBUTES lp_thread_attributes; /* ָ��SECURITY_ATTRIBUTES��ָ�룬�趨�̵߳İ�ȫ�� */
    oal_uint32 ul_stack_size;                   /* �����̳߳�ʼջ�Ĵ�С */
    oal_defer_func p_startaddress_func;         /* �߳���Ҫִ�еĵĺ����ĳ�ʼ��ַ */
    oal_uint ui_parameter;                      /* �����߳���Ҫ���ݵĲ��� */
    oal_uint32 ul_creationflags;                /* ���ÿ����̴߳����ĸ��ӱ�� */
    HANDLE threadhead;                          /* �̲߳������� */
    oal_int32 l_npriority;                      /* �߳����ȼ� */
    oal_int i_state;                            /* �̵߳���״̬ */
} oal_tasklet_stru;

typedef struct {
    oal_void *data;
    oal_uint16 nlink;
    oal_uint8 resv[2];

    read_proc_t *read_proc;
    write_proc_t *write_proc;

} oal_proc_dir_entry_stru;

typedef struct {
    oal_atomic count;
    oal_spin_lock_stru wait_lock;
} oal_mutex_stru;

#define SIGHUP    1
#define SIGINT    2
#define SIGQUIT   3
#define SIGILL    4
#define SIGTRAP   5
#define SIGABRT   6
#define SIGIOT    6
#define SIGBUS    7
#define SIGFPE    8
#define SIGKILL   9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGSTKFLT 16
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGPROF   27
#define SIGWINCH  28
#define SIGIO     29
#define SIGPOLL   SIGIO

#define ERESTARTSYS 512

#define SCHED_NORMAL 0
#define SCHED_FIFO   1
#define SCHED_RR     2
#define SCHED_BATCH  3
/* SCHED_ISO: reserved but not implemented yet */
#define SCHED_IDLE 5
struct task_struct {
    volatile long state; /* -1 unrunnable, 0 runnable, >0 stopped */
    void *stack;         /* add if you use */
};

struct semaphore {
    unsigned int count;
};

struct sched_param {
    int sched_priority;
};

typedef struct _oal_task_lock_stru_ {
    oal_wait_queue_head_stru wq;
    struct task_struct *claimer; /* task that has host claimed */
    oal_ulong claim_addr;
    oal_uint32 claimed;
    oal_int32 claim_cnt;
} oal_task_lock_stru;

// ���ں�struct rtc_time ����һ��
typedef struct _oal_time_stru {
    oal_int32 tm_sec;   /* seconds */
    oal_int32 tm_min;   /* minutes */
    oal_int32 tm_hour;  /* hours */
    oal_int32 tm_mday;  /* day of the month */
    oal_int32 tm_mon;   /* month */
    oal_int32 tm_year;  /* year */
    oal_int32 tm_wday;  /* day of the week */
    oal_int32 tm_yday;  /* day in the year */
    oal_int32 tm_isdst; /* daylight saving time */
} oal_time_stru;

OAL_STATIC OAL_INLINE static char *oal_get_current_task_name()
{
    return "win32";
}

OAL_STATIC OAL_INLINE int allow_signal(int sig)
{
    return 0;
}

/*
 * �� �� ��  : oal_atomic_read
 * ��������  : ��ȡԭ�ӱ�����ֵ
 * �������  : p_vector: ��Ҫ����ԭ�Ӳ�����ԭ�ӱ�����ַ
 */
OAL_INLINE oal_int32 oal_atomic_read(oal_atomic *p_vector)
{
    return (oal_int32)(*p_vector);
}

/*
 * �� �� ��  : oal_atomic_set
 * ��������  : ԭ�ӵ�����ԭ�ӱ���p_vectorֵΪul_val
 * �������  : p_vector: ��Ҫ����ԭ�Ӳ�����ԭ�ӱ�����ַ
 *             l_val  : ��Ҫ�����óɵ�ֵ
 */
OAL_INLINE oal_void oal_atomic_set(oal_atomic *p_vector, oal_int32 l_val)
{
    *p_vector = (LONG)l_val;
}

/*
 * �� �� ��  : oal_atomic_dec
 * ��������  : ԭ�ӵĸ���μ�1��
 * �������  : p_vector: ��Ҫ����ԭ�Ӳ�����ԭ�ӱ�����ַ
 */
OAL_INLINE oal_void oal_atomic_dec(oal_atomic *p_vector)
{
    InterlockedDecrement(p_vector);
}

/*
 * �� �� ��  : oal_atomic_inc
 * ��������  : ԭ�ӵĸ���μ�һ
 * �������  : p_vector: ��Ҫ����ԭ�Ӳ�����ԭ�ӱ�����ַ
 */
OAL_INLINE oal_void oal_atomic_inc(oal_atomic *p_vector)
{
    InterlockedIncrement(p_vector);
}

/*
 * �� �� ��  : oal_atomic_inc_and_test
 * ��������  : ԭ�ӵ����������Ƿ�Ϊ0
 * �������  : p_vector: ��Ҫ����ԭ�Ӳ�����ԭ�ӱ�����ַ
 */
OAL_STATIC OAL_INLINE oal_void oal_atomic_inc_and_test(oal_atomic *p_vector)
{
}

/*
 * �� �� ��  : oal_atomic_dec_and_test
 * ��������  : ԭ�ӵݼ��������Ƿ�Ϊ0
 * �������  : p_vector: ��Ҫ����ԭ�Ӳ�����ԭ�ӱ�����ַ
 */
OAL_STATIC OAL_INLINE oal_int32 oal_atomic_dec_and_test(oal_atomic *p_vector)
{
    return 0;
}

/*
 * �� �� ��  : oal_spin_lock_init
 * ��������  : ��������ʼ��,windows������û���ã�Ϊ����linux�����Ľṹ��Сһ�£�
 *             �˺�����ʱʲô��������ֱ�ӷ��ء�zouhongliang
 * �������  : pst_lock: ��������ַ
 */
OAL_INLINE oal_void oal_spin_lock_init(oal_spin_lock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_spin_lock
 * ��������  : ���������,windows������û���ã�Ϊ����linux�����Ľṹ��Сһ�£�
 *             �˺�����ʱʲô��������ֱ�ӷ��ء�zouhongliang
 * �������  : pst_lock: ��������ַ
 */
OAL_INLINE oal_void oal_spin_lock(oal_spin_lock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_spin_unlock
 * ��������  : �ͷ�������,windows������û���ã�Ϊ����linux�����Ľṹ��Сһ�£�
 *             �˺�����ʱʲô��������ֱ�ӷ��ء�zouhongliang
 * �������  : pst_lock:��������ַ
 */
OAL_INLINE oal_void oal_spin_unlock(oal_spin_lock_stru *pst_lock)
{
}


OAL_INLINE oal_void oal_spin_lock_bh(oal_spin_lock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_spin_unlock_bh
 * ��������  : Spinlock�����ж��Լ��ں��̵߳Ⱥ���̬�����Ļ����µĽ�������
 *             windows������û���ã�Ϊ����linux�����Ľṹ��Сһ��,�˺�����ʱʲô��������
 *             ֱ�ӷ��ء�zouhongliang
 */
OAL_INLINE oal_void oal_spin_unlock_bh(oal_spin_lock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_spin_lock_irq_save
 * ��������  : �����������ͬʱ�ѱ����־�Ĵ�����ֵ������ʧЧ�����жϡ�
 * �������  : pst_lock:��������ַ
 *             pui_flags:��־�Ĵ���
 */
OAL_INLINE oal_void oal_spin_lock_irq_save(oal_spin_lock_stru *pst_lock, oal_uint *pui_flags)
{
    oal_irq_save(pui_flags, OAL_5115IRQ_OSLIS);

    oal_spin_lock(pst_lock);
}

/*
 * �� �� ��  : oal_spin_unlock_irq_restore
 * ��������  : �ͷ���������ͬʱ���ָ���־�Ĵ�����ֵ���ָ������жϡ�����oal_sp-
 *             in_lock_irq���ʹ�á�
 * �������  : pst_lock:��������ַ
 *            pui_flags:��־�Ĵ���
 */
OAL_INLINE oal_void oal_spin_unlock_irq_restore(oal_spin_lock_stru *pst_lock, oal_uint *pui_flags)
{
    oal_spin_unlock(pst_lock);

    oal_irq_restore(pui_flags, OAL_5115IRQ_OSLIS);
}

/*
 * �� �� ��  : oal_spin_lock_irq_exec
 * ��������  : ��ȡ���������ر��жϣ�ִ��ĳ������������֮���ٴ��жϣ��ͷ���
 *             ������
 * �������  : pst_lock:��������ַ
 *             func������ָ���ַ
 *             p_arg����������
 *             pui_flags:�жϱ�־�Ĵ���
 */
OAL_INLINE oal_uint32 oal_spin_lock_irq_exec(oal_spin_lock_stru *pst_lock,
                                             oal_irqlocked_func p_irq_locked_func,
                                             oal_void *p_arg, oal_uint *pui_flags)
{
    oal_uint32 ul_Rslt;

    oal_spin_lock_irq_save(pst_lock, pui_flags);

    ul_Rslt = p_irq_locked_func(p_arg);

    oal_spin_unlock_irq_restore(pst_lock, pui_flags);

    return ul_Rslt;
}

/*
 * �� �� ��  : oal_rw_lock_init
 * ��������  : ��д����ʼ�����Ѷ�д������Ϊ1��δ��״̬����
 * �������  : pst_lock: ��д���ṹ���ַ
 */
OAL_INLINE oal_void oal_rw_lock_init(oal_rwlock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_rw_lock_read_lock
 * ��������  : ���ָ���Ķ���
 * �������  : pst_lock: ��д���ṹ���ַ
 */
OAL_INLINE oal_void oal_rw_lock_read_lock(oal_rwlock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_rw_lock_read_unlock
 * ��������  : �ͷ�ָ���Ķ���
 * �������  : pst_lock: ��д���ṹ���ַ
 */
OAL_INLINE oal_void oal_rw_lock_read_unlock(oal_rwlock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_rw_lock_write_lock
 * ��������  : ���ָ����д��
 * �������  : pst_lock: ��д���ṹ���ַ
 */
OAL_INLINE oal_void oal_rw_lock_write_lock(oal_rwlock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_rw_lock_write_unlock
 * ��������  : �ͷ�ָ����д��
 * �������  : pst_lock: ��д���ṹ���ַ
 */
OAL_INLINE oal_void oal_rw_lock_write_unlock(oal_rwlock_stru *pst_lock)
{
}

/*
 * �� �� ��  : oal_task_init
 * ��������  : �����ʼ������ʼ����ɺ������ڹ���״̬��*pst_task�е����߸�������ڴ�ռ�
 * �������  : pst_task: ����ṹ��ָ��
 *             p_func  : ����������ڵ�ַ
 *             ui_args : ����д���ĺ�������ε�ַ
 */
OAL_INLINE oal_void oal_task_init(oal_tasklet_stru *pst_task, oal_defer_func p_func, oal_uint ui_args)
{
    pst_task->lp_thread_attributes = NULL;                /* ���߳�ʹ��Ĭ�ϰ�ȫ�� */
    pst_task->ul_stack_size = 0;                          /* �����ڵ��øú������߳���ͬ��С��ջ�ռ��С */
    pst_task->p_startaddress_func = p_func;               /* �����߳���ִ�еĺ����׵�ַ */
    pst_task->ui_parameter = ui_args;                     /* �����̲߳��� */
    pst_task->ul_creationflags = 0;                       /* ����Ϊ�̴߳�������ִ�� */
    pst_task->l_npriority = THREAD_PRIORITY_ABOVE_NORMAL; /* ���ȼ�Ϊ���ڱ�׼ */
}

OAL_STATIC OAL_INLINE oal_void oal_task_kill(oal_tasklet_stru *pst_task)
{
}

/*
 * �� �� ��  : oal_task_sched
 * ��������  : ������ȣ���������׼������״̬��������ִ������ֻص�����״̬��
 * �������  : pst_task: ����ṹ��ָ��
 */
OAL_INLINE oal_void oal_task_sched(oal_tasklet_stru *pst_task)
{
}

/*
 * �� �� ��  : oal_task_is_scheduled
 * ��������  : ���tasklet�Ƿ�ȴ�ִ��
 * �������  : pst_task: ����ṹ��ָ��
 */
OAL_STATIC OAL_INLINE oal_uint oal_task_is_scheduled(oal_tasklet_stru *pst_task)
{
    return (oal_uint)(pst_task->i_state & (1 << (TASKLET_STATE_SCHED + 1)));
}

/*
 * �� �� ��  : oal_time_get_stamp_us
 * ��������  : ��ȡ΢��ȼ���ʱ���
 * �������  : pst_usec: ʱ��ṹ��ָ��
 */
OAL_INLINE oal_void oal_time_get_stamp_us(oal_time_us_stru *pst_usec)
{
    pst_usec->i_sec = 0;

    pst_usec->i_usec = 0;
}

/*
 * �� �� ��  : oal_ktime_get
 * ��������  : ��ȡ��ǰʱ���
 */
OAL_STATIC OAL_INLINE oal_time_t_stru oal_ktime_get(oal_void)
{
    oal_time_t_stru st_time = {0};
    return st_time;
}

/*
 * �� �� ��  : oal_ktime_sub
 * ��������  : ��ȡʱ���ֵ
 */
OAL_STATIC OAL_INLINE oal_time_t_stru oal_ktime_sub(const oal_time_t_stru lhs, const oal_time_t_stru rhs)
{
    oal_time_t_stru st_time = {0};
    return st_time;
}

extern oal_void oal_timer_init(oal_timer_list_stru *pst_timer, oal_uint32 ul_delay,
                               oal_timer_func p_func, oal_ulong arg);
extern oal_int32 oal_timer_delete(oal_timer_list_stru *pst_timer);
extern oal_int32 oal_timer_delete_sync(oal_timer_list_stru *pst_timer);
extern oal_int32 oal_timer_start(oal_timer_list_stru *pst_timer, oal_uint ui_expires);
extern oal_void oal_timer_add(oal_timer_list_stru *pst_timer);

/*
 * �� �� ��  : oal_timer_start_on
 * ��������  : ָ��cpu,������ʱ��,����ʱtimerҪ���ڷǼ���״̬���߻�����
 */
OAL_STATIC OAL_INLINE oal_void oal_timer_start_on(oal_timer_list_stru *pst_timer, oal_uint ui_delay, oal_int32 cpu)
{
    OAL_REFERENCE(pst_timer);
    OAL_REFERENCE(ui_delay);
    OAL_REFERENCE(cpu);
}

/*
 * �� �� ��  : oal_copy_from_user
 * ��������  : ���û�̬���ݿ������ں�̬
 * �������  : p_to: Ŀ�ĵ�
 *             p_from : Դ
 *             ul_size : ��Ҫ�����ĳ���
 * �� �� ֵ  : �������ַ�������
 */
OAL_INLINE oal_uint32 oal_copy_from_user(oal_void *p_to, const oal_void *p_from, oal_uint32 ul_size)
{
    memcpy(p_to, p_from, ul_size);

    return 0;
}

/*
 * �� �� ��  : oal_copy_to_user
 * ��������  : ���ں�̬���ݿ������û�̬
 * �������  : p_to: Ŀ�ĵ�
 *             p_from : Դ
 *             ul_size : ��Ҫ�����ĳ���
 * �� �� ֵ  : �������ַ�������
 */
OAL_INLINE oal_uint32 oal_copy_to_user(oal_void *p_to, const oal_void *p_from, oal_uint32 ul_size)
{
    memcpy(p_to, p_from, ul_size);

    return 0;
}

/*
 * �� �� ��  : oal_create_proc_entry
 * ��������  : ����proc_entry�ṹ��
 * �������  : pc_name: ������proc_entry������
 *             us_mode: ����ģʽ
 *             pst_parent: ĸproc_entry�ṹ�壬�̳�����
 */
OAL_INLINE oal_proc_dir_entry_stru *oal_create_proc_entry(const oal_int8 *pc_name, oal_uint16 us_mode,
                                                          oal_proc_dir_entry_stru *pst_parent)
{
    oal_proc_dir_entry_stru *pst_proc_entry;

    pst_proc_entry = (oal_proc_dir_entry_stru *)oal_memalloc(sizeof(oal_proc_dir_entry_stru));

    return pst_proc_entry;
}

/*
 * �� �� ��  : oal_remove_proc_entry
 * ��������  : ����proc_entry�ṹ��
 * �������  : pc_name: ������proc_entry������
 *             pst_parent: ĸproc_entry�ṹ�壬�̳�����
 */
OAL_STATIC OAL_INLINE void oal_remove_proc_entry(const oal_int8 *pc_name, oal_proc_dir_entry_stru *pst_parent)
{
    oal_free(pst_parent);
    return;
}

/*
 * �� �� ��  : oal_time_is_before
 * ��������  : �ж�ul_time�Ƿ�ȵ�ǰʱ����
 *             ���磬��ʾ��ʱʱ���ѹ��������磬������δ��ʱ
 */
OAL_STATIC OAL_INLINE oal_uint32 oal_time_is_before(oal_uint ui_time)
{
    return 1;
}

/*
 * �� �� ��  : oal_time_after
 * ��������  : �ж�ʱ���ul_time_a�Ƿ���ul_time_b֮��:
 * �� �� ֵ  : Return: 1 ul_time_a��ul_time_b֮��; ���� Return: 0
 */
OAL_STATIC OAL_INLINE oal_uint32 oal_time_after(oal_ulong ul_time_a, oal_ulong ul_time_b)
{
    return 1;
}

/*
 * �� �� ��  : oal_time_after32
 * ��������  : �ж�ʱ���ul_time_a�Ƿ���ul_time_b֮��:
 * �� �� ֵ  : Return: 1 ul_time_a��ul_time_b֮��; ���� Return: 0
 */
OAL_STATIC OAL_INLINE oal_uint32 oal_time_after32(oal_uint32 ul_time_a, oal_uint32 ul_time_b)
{
    return 1;
}

/*
 * �� �� ��  : oal_wait_for_completion_timeout
 * ��������  : ͬ�����ȴ���ʱ���
 */
OAL_STATIC OAL_INLINE oal_uint32 oal_wait_for_completion_timeout(oal_completion *pst_completion, oal_uint32 ul_timeout)
{
    return 1;
}

/*
 * �� �� ��  : oal_smp_task_lock
 * ��������  : lock the task, the lock can be double locked by the same process
 */
OAL_STATIC OAL_INLINE oal_void oal_smp_task_lock(oal_task_lock_stru *pst_lock)
{
    OAL_REFERENCE(pst_lock);
}

OAL_STATIC OAL_INLINE oal_void oal_smp_task_unlock(oal_task_lock_stru *pst_lock)
{
    OAL_REFERENCE(pst_lock);
}

OAL_STATIC OAL_INLINE oal_void oal_smp_task_lock_init(oal_task_lock_stru *pst_lock)
{
    OAL_REFERENCE(pst_lock);
}

#endif /* end of oal_schedule.h */
