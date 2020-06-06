

#ifndef __OAL_WINDOWS_WORKQUEUE_H__
#define __OAL_WINDOWS_WORKQUEUE_H__

/* STRUCT���� */
typedef struct {
    oal_uint32 ul_resv;
} oal_workqueue_stru;
typedef struct {
    oal_uint32 ul_resv;
} oal_work_stru;

typedef struct {
    oal_work_stru work;
} oal_delayed_work;

/* �궨�� */
#define OAL_INIT_WORK(_p_work, _p_func)
#define OAL_INIT_DELAYED_WORK(_work, _func)

/*lint -e528*/
static oal_workqueue_stru workqueue;
#define OAL_CREATE_SINGLETHREAD_WORKQUEUE(_name) &workqueue;
#define oal_create_workqueue(name)               &workqueue

/*
 * �� �� ��  : oal_create_singlethread_workqueue
 * ��������  : ����һ�����̵߳Ĺ�������
 */
OAL_STATIC OAL_INLINE oal_workqueue_stru *oal_create_singlethread_workqueue(const oal_int8 *pc_workqueue_name)
{
    return 0;
}

/*
 * �� �� ��  : oal_destroy_workqueue
 * ��������  : ���ٹ�������
 */
OAL_STATIC OAL_INLINE oal_void oal_destroy_workqueue(oal_workqueue_stru *pst_workqueue)
{
    return;
}

/*
 * �� �� ��  : oal_add_work_to_workqueue
 * ��������  : ���һ�����񵽹�������
 */
OAL_STATIC OAL_INLINE oal_int32 oal_queue_work(oal_workqueue_stru *pst_workqueue, oal_work_stru *pst_work)
{
    return 0;
}

OAL_STATIC OAL_INLINE oal_int32 oal_queue_delayed_work(oal_workqueue_stru *pst_workqueue,
                                                       oal_delayed_work *pst_work, oal_ulong delay)
{
    return 0;
}

OAL_STATIC OAL_INLINE oal_int32 oal_queue_delayed_work_on(oal_int32 cpu, oal_workqueue_stru *pst_workqueue,
                                                          oal_delayed_work *pst_work, oal_ulong delay)
{
    return 0;
}

/*
 * �� �� ��  : oal_queue_delayed_system_work
 * ��������  : queue work on system wq after delay
 * �������  :  pst_work: delayable work to queue
 *              delay: number of jiffies to wait before queueing
 */
OAL_STATIC OAL_INLINE oal_int32 oal_queue_delayed_system_work(oal_delayed_work *pst_work, oal_ulong delay)
{
    return 0;
}

OAL_STATIC OAL_INLINE oal_int32 oal_work_is_busy(oal_work_stru *pst_delay_work)
{
    return 1;
}

OAL_STATIC OAL_INLINE oal_int32 oal_cancel_delayed_work_sync(oal_delayed_work *pst_delay_work)
{
    return 0;
}

OAL_STATIC OAL_INLINE oal_int32 oal_cancel_work_sync(oal_work_stru *pst_work)
{
    return 0;
}

#endif /* end of oal_workqueue.h */
