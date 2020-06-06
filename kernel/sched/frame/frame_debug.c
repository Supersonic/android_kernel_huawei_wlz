#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <../kernel/sched/sched.h>

#include "frame_info.h"

/*
 * This allows printing both to /proc/sched_debug and
 * to the console
 */
#define SEQ_printf(m, x...)			\
do {						\
	if (m)					\
		seq_printf(m, x);		\
	else					\
		printk(x);			\
} while (0)

static inline void print_frame_info(struct seq_file *m,
		struct related_thread_group *grp)
{
	struct frame_info *frame_info = (struct frame_info *) grp->private_data;

	u64 now = ktime_get_ns();
	u64 frame_end = grp->window_start;

	SEQ_printf(m, "FRAME_INFO      : QOS:%u#MARGIN:%dSTATE:%lu\n",
			frame_info->qos_frame,
			frame_info->vload_margin,
			frame_info->status);
	SEQ_printf(m, "FRAME_LOAD_MODE : %s/%s\n",
			grp->mode.util_enabled ? "true" : "false",
			grp->mode.freq_enabled ? "true" : "false");
	SEQ_printf(m, "FRAME_INTERVAL  : UPDATE:%ldms#INVALID:%ldms\n",
			grp->freq_update_interval / NSEC_PER_MSEC,
			grp->util_invalid_interval / NSEC_PER_MSEC);
	SEQ_printf(m, "FRAME_TIMESTAMP : timestamp:%llu#now:%llu#delta:%llu\n",
			frame_end, now, now - frame_end);
	SEQ_printf(m, "FRAME_LAST_TIME : %llu/%llu\n",
			(unsigned long long)frame_info->prev_frame_exec,
			(unsigned long long)frame_info->prev_frame_time);
	SEQ_printf(m, "FRAME_CLUSTER   : %d\n",
			grp->preferred_cluster ? grp->preferred_cluster->id : -1);
}

static inline char frame_task_state_to_char(struct task_struct *tsk)
{
	static const char state_char[] = "RSDTtXZPI";
	unsigned int tsk_state = READ_ONCE(tsk->state);
	unsigned int state = (tsk_state | tsk->exit_state) & TASK_REPORT;

	BUILD_BUG_ON_NOT_POWER_OF_2(TASK_REPORT_MAX);
	BUILD_BUG_ON(1 + ilog2(TASK_REPORT_MAX) != sizeof(state_char) - 1);

	if (tsk_state == TASK_IDLE)
		state = TASK_REPORT_IDLE;

	return state_char[fls(state)];
}


static inline void
print_frame_task_header(struct seq_file *m, char *header,
		int run, int nr)
{
	SEQ_printf(m,
		"%s   : %d/%d\n"
		"STATE		COMM	   PID	PRIO	CPU\n"
		"---------------------------------------------------------\n",
		header, run, nr);
}

static inline void
print_frame_task(struct seq_file *m, struct task_struct *p)
{
	SEQ_printf(m, "%5c %15s %5d %5d %5d(%*pbl)\n",
		frame_task_state_to_char(p), p->comm,
		p->pid, p->prio,
		task_cpu(p), cpumask_pr_args(&p->cpus_allowed));
}

static inline void
print_frame_threads(struct seq_file *m,
		struct related_thread_group *grp)
{
	struct task_struct *p = NULL;
	int nr_thread = 0;

	list_for_each_entry(p, &grp->tasks, grp_list) {
		nr_thread++;
	}
	print_frame_task_header(m, "FRAME_THREADS", grp->nr_running, nr_thread);
	list_for_each_entry(p, &grp->tasks, grp_list) {
		if (!p)
			continue;
		get_task_struct(p);
		print_frame_task(m, p);
		put_task_struct(p);
	}
}

static int sched_frame_debug_show(struct seq_file *m, void *v)
{
	struct related_thread_group *grp = NULL;
	unsigned long flags;

	grp = lookup_related_thread_group(DEFAULT_RT_FRAME_ID);
	if (!grp) {
		SEQ_printf(m, "IPROVISION RTG none\n");
		return 0;
	}

	raw_spin_lock_irqsave(&grp->lock, flags);
	if (list_empty(&grp->tasks)) {
		raw_spin_unlock_irqrestore(&grp->lock, flags);
		SEQ_printf(m, "IPROVISION RTG tasklist empty\n");
		return 0;
	}

	print_frame_info(m, grp);
	print_frame_threads(m, grp);
	raw_spin_unlock_irqrestore(&grp->lock, flags);

	return 0;
}

static int sched_frame_debug_release(struct inode *inode, struct file *file)
{
	seq_release(inode, file);
	return 0;
}

static int sched_frame_debug_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, sched_frame_debug_show, NULL);
}

static const struct file_operations sched_frame_debug_fops = {
	.open		= sched_frame_debug_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= sched_frame_debug_release,
};

static int __init init_sched_debug_procfs(void)
{
	struct proc_dir_entry *pe;

	pe = proc_create("sched_frame_debug",
		0444, NULL, &sched_frame_debug_fops);
	if (!pe)
		return -ENOMEM;
	return 0;
}
late_initcall(init_sched_debug_procfs);
