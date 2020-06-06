#ifndef _HISI_PRINTK_H_
#define _HISI_PRINTK_H_

#include <linux/spinlock.h>

extern raw_spinlock_t *g_logbuf_level_lock_ex;
#endif
