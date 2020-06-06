

#ifndef __OAL_WINDOWS_THREAD_H__
#define __OAL_WINDOWS_THREAD_H__

/* ����ͷ�ļ����� */
#include "oal_schedule.h"

/* STRUCT���� */
typedef struct task_struct oal_task_stru;


/* �궨�� */
#define oal_kthread_create(fn, arg, namefmt, cpu) ((oal_void *)0)
#define oal_kthread_bind(task, cpu)
#define oal_kthread_stop(task)
#define oal_kthread_run
#define oal_kthread_should_stop() 0
#define oal_schedule
#define oal_wake_up_process(task)
#define oal_set_current_state
#define oal_cond_resched

#define oal_sched_setscheduler(task, policy, param) 0
#define oal_set_user_nice(task, nice)

#endif
