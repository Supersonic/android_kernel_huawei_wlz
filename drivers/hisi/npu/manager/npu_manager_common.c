#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>

#include "npu_manager_common.h"
#include "drv_log.h"

static struct devdrv_reg_callbak_info g_reg_cb;

void devdrv_register_callbak_info_init(void)
{
	int i;

	for (i = 0; i< DEVDRV_CALLBACK_REG_NUM;i++){
		g_reg_cb.op_callback[i] = NULL;
	}
	mutex_init(&g_reg_cb.op_mutex);

	for (i = 0; i< DEVDRV_CALLBACK_REG_NUM;i++){
		g_reg_cb.rs_callback[i] = NULL;
	}
	mutex_init(&g_reg_cb.rs_mutex);
}
EXPORT_SYMBOL(devdrv_register_callbak_info_init);

//register callback (what to do when npu power up)
int devdrv_open_npu_callback_reg(int (*callback)
	(unsigned int dev_id),unsigned int *callback_idx)
{
	int i;
	if(callback == NULL || callback_idx == NULL) {
		devdrv_drv_err("callback or callback_idx is null.\n");
		return -ENODEV;
	}

	mutex_lock(&g_reg_cb.op_mutex);
	for(i = 0; i< DEVDRV_CALLBACK_REG_NUM;i++){
		if(g_reg_cb.op_callback[i] == NULL) {
			g_reg_cb.op_callback[i] = callback;
			*callback_idx = i;
			mutex_unlock(&g_reg_cb.op_mutex);
			return 0;
		}
	}
	mutex_unlock(&g_reg_cb.op_mutex);

	return -ENODEV;
}
EXPORT_SYMBOL(devdrv_open_npu_callback_reg);

//deregister callback (what to do when npu power up),as dev release
int devdrv_open_npu_callback_dereg(unsigned int callback_idx)
{
	if(callback_idx >= DEVDRV_CALLBACK_REG_NUM) {
		devdrv_drv_err("callback_idx is invalid %d.\n", callback_idx);
		return -ENODEV;
	}

	mutex_lock(&g_reg_cb.op_mutex);
	g_reg_cb.op_callback[callback_idx] = NULL;
	mutex_unlock(&g_reg_cb.op_mutex);

	return 0;
}
EXPORT_SYMBOL(devdrv_open_npu_callback_dereg);

//when npu power up, call callback
void devdrv_open_npu_callback_proc(unsigned int dev_id)
{
	int i;
	mutex_lock(&g_reg_cb.op_mutex);
	for(i = 0; i< DEVDRV_CALLBACK_REG_NUM;i++){
		if(g_reg_cb.op_callback[i] != NULL) {
			g_reg_cb.op_callback[i](dev_id);
		}
	}
	mutex_unlock(&g_reg_cb.op_mutex);

	return;
}
EXPORT_SYMBOL(devdrv_open_npu_callback_proc);

//register callback (what to do when npu power down)
int devdrv_release_npu_callback_reg(int (*callback)
	(unsigned int dev_id),unsigned int *callback_idx)
{
	int i;
	if(callback == NULL || callback_idx == NULL) {
		devdrv_drv_err("callback or callback_idx is null.\n");
		return -ENODEV;
	}

	mutex_lock(&g_reg_cb.rs_mutex);
	for (i = 0; i< DEVDRV_CALLBACK_REG_NUM;i++) {
		if(g_reg_cb.rs_callback[i] == NULL) {
			g_reg_cb.rs_callback[i] = callback;
			*callback_idx = i;
			mutex_unlock(&g_reg_cb.rs_mutex);
			return 0;
		}
	}
	mutex_unlock(&g_reg_cb.rs_mutex);

	return -ENODEV;
}
EXPORT_SYMBOL(devdrv_release_npu_callback_reg);

//deregister callback (what to do when npu power down),as dev release
int devdrv_release_npu_callback_dereg(unsigned int callback_idx)
{
	if(callback_idx >= DEVDRV_CALLBACK_REG_NUM) {
		devdrv_drv_err("callback_idx is invalid %d.\n", callback_idx);
		return -ENODEV;
	}

	mutex_lock(&g_reg_cb.rs_mutex);
	g_reg_cb.rs_callback[callback_idx] = NULL;
	mutex_unlock(&g_reg_cb.rs_mutex);

	return 0;
}
EXPORT_SYMBOL(devdrv_release_npu_callback_dereg);

//when npu power down, call callback
void devdrv_release_npu_callback_proc(unsigned int dev_id)
{
	int i;
	mutex_lock(&g_reg_cb.rs_mutex);
	for (i = 0; i< DEVDRV_CALLBACK_REG_NUM;i++) {
		if(g_reg_cb.rs_callback[i] != NULL) {
			g_reg_cb.rs_callback[i](dev_id);
		}
	}
	mutex_unlock(&g_reg_cb.rs_mutex);

	return;
}
EXPORT_SYMBOL(devdrv_release_npu_callback_proc);


