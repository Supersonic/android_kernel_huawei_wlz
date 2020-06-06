#include <linux/platform_device.h>
#include "npu_platform.h"

int devdrv_npu_debug_init(void);
int list_count(struct list_head *list);
static int hisi_npu_resource_debugfs_show(struct seq_file *s, void *data);
static int hisi_npu_resource_debugfs_open(struct inode *inode, struct file *file);