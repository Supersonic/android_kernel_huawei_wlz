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


#include "product_config.h"

#ifdef CONFIG_HUAWEI_BASTET_COMM_NEW
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include "BastetComm.h"
#include "securec.h"
#include "mdrv_vcom_agent.h"

#endif

#include "v_typdef.h"
#include "VosPidDef.h"
#include "PsCommonDef.h"
#include "mdrv.h"
#include "TafTypeDef.h"
#include "BastetInit.h"
#include "BastetInterface.h"
#include "BastetRnicInterface.h"
#include "BastetPrivate.h"



#ifdef CONFIG_HUAWEI_BASTET_COMM_NEW

#define THIS_FILE_ID                    PS_FILE_ID_BST_COMM_C

dev_t bastet_modem_dev;
struct cdev bastet_modem_cdev;
struct class *bastet_modem_class;

static struct bastet_modem_driver_data bastet_modem_data;

bool bastet_modem_dev_en;

static int bastet_modem_open(struct inode *inode, struct file *filp)
{
    spin_lock_bh(&bastet_modem_data.read_lock);
    if (bastet_modem_dev_en)
    {
        BST_PR_LOGE("bastet device has been opened");
        spin_unlock_bh(&bastet_modem_data.read_lock);
        return -EPERM;
    }

    bastet_modem_dev_en = true;

    spin_unlock_bh(&bastet_modem_data.read_lock);
    BST_PR_LOGI("success");

    return 0;
}

/*
 * Get modem id and rab id.
 */
int get_modem_rab_id_by_devname(struct bastet_modem_rab_id *info,
    const char *local_netdev_name)
{
    struct net_device *dev;
    BST_RNIC_MODEM_INFO_STRU stModemInfo;
    VOS_INT rslt;

    if (NULL == info) {
        return -EINVAL;
    }

    dev = dev_get_by_name(&init_net, local_netdev_name);
    if (NULL == dev) {
        return -ENOENT;
    }

    rslt = RNIC_BST_GetModemInfo((char *)dev,&stModemInfo);
    if (VOS_OK != rslt)
    {
        BST_PR_LOGE("get modem rab id fail\n");
        return -EPERM;
    }

    if (BST_RNIC_PDP_STATE_ACTIVE != stModemInfo.enIPv4State) {
        BST_PR_LOGE("Ipv4 pdp reg status inactive\n");
        return -EPERM;
    }

    info->modem_id = stModemInfo.usModemId;
    /* Bastet only running in IPv4 mode,
     * so, get IPv4 Pdp info */
    info->rab_id = stModemInfo.ucRabId;

    return 0;
}

/*
 * this is main method to exchange data with user space,
 * including socket sync, get ip and port, adjust kernel flow
 *
 * return "int" by standard.
 */
static long bastet_modem_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    int rc = -EFAULT;
    void __user *argp = (void __user *)arg;
    char netdev_name_from[IFNAMSIZ];
    struct bastet_modem_rab_id info;

    if(NULL == argp)
    {
        BST_PR_LOGI("bastet_modem ioctl argp is null.\n");
        return rc;
    }
    
    switch (cmd) {
#ifdef CONFIG_HUAWEI_BASTET_COMM
    case BST_MODEM_IOC_GET_MODEM_RAB_ID:
    {
        BST_PR_LOGI("bastet_modem ioctl get modem rab id.\n");

        if (copy_from_user(&netdev_name_from, argp, sizeof(netdev_name_from)))
            break;



        rc = get_modem_rab_id_by_devname(&info, netdev_name_from);
        if (rc < 0)
            break;

        if (copy_to_user(argp, &info, sizeof(info)))
            rc = -EFAULT;
        break;
    }
    case BST_MODEM_IOC_GET_MODEM_RESET:
    {
        BST_PR_LOGI("bastet_modem ioctl get modem reset status.\n");
        spin_lock_bh(&bastet_modem_data.read_lock);
       //get modem_reset_status and copy to user
        if (copy_to_user(argp, &bastet_modem_data.modem_reset_status, sizeof(bastet_modem_data.modem_reset_status)))
        {
            rc = -EFAULT;
        }
        else
        {
            //reset the modem_reset_status after read
            bastet_modem_data.modem_reset_status = 0;
        }
        spin_unlock_bh(&bastet_modem_data.read_lock);
        break;
    }
    default: {
        BST_PR_LOGE("unknown ioctl: %u", cmd);
        break;
    }
#endif
    }

    return rc;
}

/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
static long compat_bastet_modem_ioctl(struct file *flip,
    unsigned int cmd, unsigned long arg)
{
    return bastet_modem_ioctl(flip, cmd, (unsigned long) compat_ptr(arg));
}
#endif

#define BST_MAX_WRITE_PAYLOAD           (2048)
#define BST_MAX_QUEUE_SIZE           (256)

/*lint -e429*/
int post_modem_packet(const void *info, unsigned int len)
{
    struct bastet_modem_packet *pkt = NULL;

    if (!bastet_modem_dev_en) {
        BST_PR_LOGE("bastet is not opened\n");
        return -ENOENT;
    }

    pkt = kmalloc(sizeof(struct bastet_modem_packet) + len, GFP_ATOMIC);
    if (NULL == pkt) {
        BST_PR_LOGE("failed to kmalloc\n");
        return -ENOMEM;
    }
    if(memset_s(pkt, sizeof(struct bastet_modem_packet) + BST_MAX_WRITE_PAYLOAD, 0, sizeof(struct bastet_modem_packet) + len))
    {
        BST_PR_LOGE("memset failed\n");
    }

    BST_PR_LOGI("bastet modem post packet len %d \n", len);

    pkt->data.cons = 0;
    pkt->data.len = len;
    if (NULL != info)
    {
        if(memcpy_s(pkt->data.value, BST_MAX_WRITE_PAYLOAD, info, len))
        {
            BST_PR_LOGE("memcpy failed\n");
        }
    }

    spin_lock_bh(&bastet_modem_data.read_lock);
    if(bastet_modem_data.queuelen >= BST_MAX_QUEUE_SIZE)
    {
        BST_PR_LOGE("queuelen is exceeding the limits\n");
        spin_unlock_bh(&bastet_modem_data.read_lock);
        kfree(pkt);
        return -ENOMEM; 
    }
    else
    {
        list_add_tail(&pkt->list, &bastet_modem_data.read_queue);
        bastet_modem_data.queuelen++;
    }
    spin_unlock_bh(&bastet_modem_data.read_lock);

    wake_up_interruptible_sync_poll(&bastet_modem_data.read_wait,
        POLLIN | POLLRDNORM | POLLRDBAND);

    return 0;
}
/*lint +e429*/


void bastet_comm_recv(MsgBlock *pMsg)
{
    u32  len;
    BST_ACOM_MSG_STRU  *pTmpMsg;
    /*lint -e826*/
    pTmpMsg             = (BST_ACOM_MSG_STRU *)pMsg;
    /*lint +e826*/

    if ( NULL == pMsg )
    {
        BST_PR_LOGE("MsgBlock is empty\n");
        return;
    }

    len  = pTmpMsg->ulLength;
    if ( len > BST_MAX_WRITE_PAYLOAD)
    {
        len = BST_MAX_WRITE_PAYLOAD;
    }

    post_modem_packet((void *)((unsigned char *)pMsg + VOS_MSG_HEAD_LENGTH), len);

}


static int bastet_modem_packet_read(char __user *buf, size_t count)
{
    struct bastet_modem_packet *pkt = NULL;
    uint8_t *data = NULL;
    bool isfree = false;
    uint32_t len = 0;
    uint32_t size = 0;

    if (NULL == buf)
        return -EINVAL;

    BST_PR_LOGI("bastet_modem_packet_read ready to got data\n");

    spin_lock_bh(&bastet_modem_data.read_lock);
    if (list_empty(&bastet_modem_data.read_queue)) {
        spin_unlock_bh(&bastet_modem_data.read_lock);
        return -EAGAIN;
    }

    pkt = list_first_entry(&bastet_modem_data.read_queue,
        struct bastet_modem_packet, list);
    len = pkt->data.len;
    data = (uint8_t *)(&(pkt->data.value)); /*lint !e826 */

    if ((0 == pkt->data.cons) && (count > (size_t)len)) { /*lint !e574 */
        list_del(&pkt->list);
        size = len;
        isfree = true;
    } else if (((0 == pkt->data.cons) && (count <= (size_t)len)) /*lint !e574 */
        || ((pkt->data.cons != 0) && (pkt->data.cons + count <= (size_t)len))) { /*lint !e574 */
        size = count;
        isfree = false;
    } else {
        list_del(&pkt->list);
        size = len - pkt->data.cons;
        isfree = true;
    }
    spin_unlock_bh(&bastet_modem_data.read_lock);

    BST_PR_LOGI("bastet_modem_packet_read got data done and copy to user\n");
    if (copy_to_user(buf, data + pkt->data.cons, size))
    {
        pkt->data.cons = 0;
        if (isfree)
        {
            kfree(pkt);
            spin_lock_bh(&bastet_modem_data.read_lock);
            bastet_modem_data.queuelen--;
            spin_unlock_bh(&bastet_modem_data.read_lock);
        }
        return -EFAULT;
    }
    pkt->data.cons += size;

    if (isfree)
    {
        kfree(pkt);
        spin_lock_bh(&bastet_modem_data.read_lock);
        if(bastet_modem_data.queuelen > 0)
            bastet_modem_data.queuelen--;
        spin_unlock_bh(&bastet_modem_data.read_lock);
    }
    BST_PR_LOGI("bastet_modem_packet_read copy to user done and return size: %d\n", size);
    return size;
}

/*
 * blocked read, it will be waiting here until net device state is change.
 *
 * standard arg is "const char __user *buf".
 */
/*lint -e666*/
static ssize_t bastet_modem_read(struct file *filp,
    char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;

    spin_lock_bh(&bastet_modem_data.read_lock);
    while (list_empty(&bastet_modem_data.read_queue)) {
        spin_unlock_bh(&bastet_modem_data.read_lock);
        ret = wait_event_interruptible(bastet_modem_data.read_wait,
            !list_empty(&bastet_modem_data.read_queue));
        if (ret)
            return ret;

        spin_lock_bh(&bastet_modem_data.read_lock);
    }
    spin_unlock_bh(&bastet_modem_data.read_lock);

    BST_PR_LOGI("bastet_modem_read got a data here\n");

    return bastet_modem_packet_read(buf, count);
}
/*lint +e666*/
#ifdef CONFIG_HUAWEI_BASTET_COMM

static ssize_t bastet_modem_write(struct file *filp,
    const char __user *buf, size_t count, loff_t *ppos)
{
    /* u8 msg[BST_MAX_WRITE_PAYLOAD]; */
    u8 *msg = kmalloc(BST_MAX_WRITE_PAYLOAD, GFP_KERNEL);
    BST_ACOM_MSG_STRU *pMsg = NULL;
    VOS_UINT32 ulLength = 0;

    if (NULL == msg)
    {
        BST_PR_LOGE("msg alloc failed!");
        return -1;
    }

    if(memset_s(msg, BST_MAX_WRITE_PAYLOAD, 0, BST_MAX_WRITE_PAYLOAD))
    {
        BST_PR_LOGE("memset failed");
    }
    if(NULL == buf)
    {
        BST_PR_LOGE("buf is null!");
        kfree(msg);
        return -1;
    }

    if ((count > BST_MAX_WRITE_PAYLOAD) || (count <= 0))
    {
        BST_PR_LOGE("write length over BST_MAX_WRITE_PAYLOAD!");
        kfree(msg);
        return -EINVAL;
    }

    if (copy_from_user(msg, buf, count)) 
    {
        BST_PR_LOGE("copy_from_user error");
        kfree(msg);
        return -EFAULT;
    }

    ulLength        = VOS_MSG_HEAD_LENGTH + count;
    pMsg            = (BST_ACOM_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_BASTET_COMM, ulLength);
    if (NULL == pMsg)
    {
        BST_PR_LOGE("PS_ALLOC_MSG_WITH_HEADER_LEN failed\n");
        kfree(msg);
        return -1;
    }

    pMsg->ulReceiverPid = UEPS_PID_BASTET;
    TAF_MEM_CPY_S((VOS_VOID*)((unsigned char *)pMsg + VOS_MSG_HEAD_LENGTH), BST_MAX_WRITE_PAYLOAD, msg, (VOS_UINT32)count);

    BST_PR_LOGI("bastet modem write count %d  len %d\n", (int)count, (int)ulLength);
    kfree(msg);

    if (PS_SEND_MSG(ACPU_PID_BASTET_COMM, pMsg) != 0)
    {
        BST_PR_LOGE("PS_SEND_MSG failed\n");
        return -1;
    }

    return count;
}
#endif

static unsigned int bastet_modem_poll(struct file *file, poll_table *wait)
{
    unsigned int mask = 0;

    poll_wait(file, &bastet_modem_data.read_wait, wait);
    mask = !list_empty(&bastet_modem_data.read_queue) ? (POLLIN | POLLRDNORM) : 0;

    return mask;
}

static int bastet_modem_release(struct inode *inode, struct file *filp)
{
    struct list_head *p, *n;
    struct bastet_modem_packet *pkt = NULL;

    spin_lock_bh(&bastet_modem_data.read_lock);

    if (list_empty(&bastet_modem_data.read_queue))
        goto out_release;

    list_for_each_safe(p, n, &bastet_modem_data.read_queue) {
        pkt = list_entry(p, struct bastet_modem_packet, list);
        list_del(&pkt->list);
        kfree(pkt);
    }

out_release:
    bastet_modem_dev_en = false;
    spin_unlock_bh(&bastet_modem_data.read_lock);
    BST_PR_LOGI("success");

    return 0;
}

static const struct file_operations bastet_modem_dev_fops = {
    .owner = THIS_MODULE,
    .open = bastet_modem_open,
    .unlocked_ioctl = bastet_modem_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = compat_bastet_modem_ioctl,
#endif
    .read = bastet_modem_read,
#ifdef CONFIG_HUAWEI_BASTET_COMM
    .write = bastet_modem_write,
#endif
    .poll = bastet_modem_poll,
    .release = bastet_modem_release,
};

static void bastet_data_init(void)
{
    spin_lock_init(&bastet_modem_data.read_lock);
    INIT_LIST_HEAD(&bastet_modem_data.read_queue);
    init_waitqueue_head(&bastet_modem_data.read_wait);
    bastet_modem_data.modem_reset_status = 0;
    bastet_modem_data.queuelen = 0;
}

static void bastet_comm_init(void)
{
    BASTET_CommRegRecvCallBack((RECV_MSG_PROC)bastet_comm_recv);
}

#if defined CONFIG_BALONG_MODEM_RESET

#if   defined CONFIG_HISI_BALONG_MODEM_HI3650
extern int bsp_reset_cb_func_register(const char *pname,
	pdrv_reset_cbfun pcbfun, int userdata, int priolevel);
#endif

static int bastet_modem_ccorereset_cb(DRV_RESET_CB_MOMENT_E eparam, int userdata)
{
    BST_ACOM_MSG_STRU  msg;
    if (MDRV_RESET_CB_BEFORE == eparam) 
    {
        BST_PR_LOGI("MDRV_RESET_CB_BEFORE");
        //spin_lock_bh(&bastet_modem_data.read_lock);
        //bastet_modem_data.modem_reset_status = 1;
        //spin_unlock_bh(&bastet_modem_data.read_lock);

        msg.enMsgType =  BST_ACORE_CORE_MSG_TYPE_RESET_INFO;
        msg.ulLen = 4;
        msg.aucValue[0] = 0x01;

        post_modem_packet((void *)((unsigned char *)(&msg) + VOS_MSG_HEAD_LENGTH), sizeof(BST_ACOM_MSG_STRU) - VOS_MSG_HEAD_LENGTH);
    }

    return 0;
}


static void bastet_modem_reset_cb_register(void)
{
    BST_PR_LOGI("register balong mss reset notification");
#if   defined CONFIG_HISI_BALONG_MODEM_HI3650
    bsp_reset_cb_func_register("BASTET", bastet_modem_ccorereset_cb,
        0, 5);
#endif
}

#else

static void bastet_modem_reset_cb_register(void)
{
    BST_PR_LOGI("balong mss reset not supported");
}

#endif

int __init bastet_modem_driver_init(void)
{
    int ret = 0;
    struct device *dev = NULL;

    BST_PR_LOGE("bastet modem init");
    bastet_data_init();
    bastet_modem_reset_cb_register();
#ifdef CONFIG_HUAWEI_BASTET_COMM
    bastet_comm_init();
    BST_PR_LOGE("bastet modem comm init");

#endif

    /* register bastet major and minor number */
    ret = alloc_chrdev_region(&bastet_modem_dev,
        BST_MODEM_FIRST_MINOR, BST_MODEM_DEVICES_NUMBER, BASTET_MODEM_NAME);
    if (ret) {
        BST_PR_LOGE("alloc_chrdev_region error");
        goto fail_region;
    }

    cdev_init(&bastet_modem_cdev, &bastet_modem_dev_fops);
    bastet_modem_cdev.owner = THIS_MODULE;

    ret = cdev_add(&bastet_modem_cdev, bastet_modem_dev, BST_MODEM_DEVICES_NUMBER);
    if (ret) {
        BST_PR_LOGE("cdev_add error");
        goto fail_cdev_add;
    }

    bastet_modem_class = class_create(THIS_MODULE, BASTET_MODEM_NAME);
    if (IS_ERR(bastet_modem_class)) {
        BST_PR_LOGE("class_create error");
        goto fail_class_create;
    }

    dev = device_create(bastet_modem_class, NULL, bastet_modem_dev, NULL, BASTET_MODEM_NAME);
    if (IS_ERR(dev)) {
        BST_PR_LOGE("device_create error");
        goto fail_device_create;
    }

    return 0;

fail_device_create:
    class_destroy(bastet_modem_class);
fail_class_create:
    cdev_del(&bastet_modem_cdev);
fail_cdev_add:
    unregister_chrdev_region(bastet_modem_dev, BST_MODEM_DEVICES_NUMBER);
fail_region:

    return ret;
}

void __exit bastet_modem_driver_exit(void)
{
    if (NULL != bastet_modem_class) {
        device_destroy(bastet_modem_class, bastet_modem_dev);
        class_destroy(bastet_modem_class);
    }
    cdev_del(&bastet_modem_cdev);
    unregister_chrdev_region(bastet_modem_dev, BST_MODEM_DEVICES_NUMBER);

}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bastet_modem_driver_init);
module_exit(bastet_modem_driver_exit);

MODULE_AUTHOR("huawei.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bastet Modem driver");
#endif

#else


#define THIS_FILE_ID                    PS_FILE_ID_BST_COMM_C
#define BST_MAX_WRITE_PAYLOAD           (2048)

struct bst_modem_rab_id {
    uint16_t modem_id;
    uint16_t rab_id;
};

extern char cur_netdev_name[IFNAMSIZ];
extern void ind_hisi_com(const void *info, u32 len);
#ifdef CONFIG_HUAWEI_EMCOM
extern void Emcom_Ind_Modem_Support(u8 ucState);
#endif

int bastet_comm_write(u8 *msg, u32 len, u32 type)
{
    BST_ACOM_MSG_STRU *pMsg = NULL;
    VOS_UINT32 ulLength = 0;

    if (NULL == msg)
    {
        return -1;
    }

    ulLength        = sizeof(BST_ACOM_MSG_STRU) + len;
    pMsg            = (BST_ACOM_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_BASTET_COMM, ulLength);
    if (NULL == pMsg)
    {
        BST_PR_LOGE("PS_ALLOC_MSG_WITH_HEADER_LEN failed\n");
        return -1;
    }

    pMsg->ulReceiverPid = UEPS_PID_BASTET;
    TAF_MEM_CPY_S((VOS_VOID*)pMsg->aucValue, BST_MAX_WRITE_PAYLOAD, msg, (VOS_UINT32)len);

    pMsg->ulLen     = len;
    pMsg->enMsgType = type;
    if (PS_SEND_MSG(ACPU_PID_BASTET_COMM, pMsg) != 0)
    {
        BST_PR_LOGE("PS_SEND_MSG failed\n");
        return -1;
    }

    return 0;
}

#ifdef CONFIG_HUAWEI_EMCOM
int bastet_comm_keypsInfo_write(u32 ulState)
{
    BST_KEY_PSINFO_STRU *pMsg = NULL;
    VOS_UINT32 ulLength = 0;

    ulLength        = sizeof( BST_KEY_PSINFO_STRU );
    pMsg            = (BST_KEY_PSINFO_STRU *)PS_ALLOC_MSG(ACPU_PID_BASTET_COMM, ulLength);
    if (NULL == pMsg)
    {
        BST_PR_LOGE("PS_ALLOC_MSG_WITH_HEADER_LEN failed\n");
        return -1;
    }
    pMsg->enState = ulState;

    pMsg->enMsgType   = BST_ACORE_CORE_MSG_TYPE_EMCOM_KEY_PSINFO;

    pMsg->ulReceiverPid = UEPS_PID_BASTET;
    BST_PR_LOGD("state: %d", pMsg->enState);
    if (PS_SEND_MSG(ACPU_PID_BASTET_COMM, pMsg) != 0)
    {
        BST_PR_LOGE("PS_SEND_MSG failed\n");
        return -1;
    }

    return 0;
}
#endif


void bastet_comm_recv(MsgBlock *pMsg)
{
    u32                 len;
    BST_ACOM_MSG_STRU  *pTmpMsg;

    /*lint -e826*/
    pTmpMsg             = (BST_ACOM_MSG_STRU *)pMsg;
    /*lint +e826*/
    if ( NULL == pMsg )
    {
        BST_PR_LOGE("MsgBlock is empty\n");
        return;
    }

    len                 = pTmpMsg->ulLen;
    if ( len > BST_MAX_WRITE_PAYLOAD )
    {
        len             = BST_MAX_WRITE_PAYLOAD;
    }

    switch( pTmpMsg->enMsgType )
    {
        case BST_ACORE_CORE_MSG_TYPE_DSPP:
        {
            ind_hisi_com( pTmpMsg->aucValue, len );
            break;
        }
        #ifdef CONFIG_HUAWEI_EMCOM
        case BST_ACORE_CORE_MSG_TYPE_EMCOM_SUPPORT:
        {
            BST_EMCOM_SUPPORT_STRU *pIndMsg = (BST_EMCOM_SUPPORT_STRU *)pMsg; //lint !e826
            Emcom_Ind_Modem_Support( pIndMsg->enState );
            break;
        }
        #endif
        default:
            BST_PR_LOGE("pTmpMsg->enMsgType type error,state: %d\n", pTmpMsg->enMsgType);
            break;
    }
 }

/*
 * Get modem id and rab id.
 */
int get_modem_rab_id(struct bst_modem_rab_id *info)
{
    BST_RNIC_MODEM_INFO_STRU stModemInfo;
    VOS_INT rslt;

    if (NULL == info) {
        return -EINVAL;
    }

    rslt = RNIC_BST_GetModemInfo(cur_netdev_name,&stModemInfo);
    if (VOS_OK != rslt)
    {
        BST_PR_LOGE("get modem rab id fail\n");
        return -EPERM;
    }

    if (BST_RNIC_PDP_STATE_ACTIVE != stModemInfo.enIPv4State) {
        BST_PR_LOGE("Ipv4 pdp reg status inactive\n");
        return -EPERM;
    }

    info->modem_id = stModemInfo.usModemId;
    /* Bastet only running in IPv4 mode,
     * so, get IPv4 Pdp info */
    info->rab_id = stModemInfo.ucRabId;

    return 0;
}

void bastet_comm_init(void)
{
    BASTET_CommRegRecvCallBack((RECV_MSG_PROC)bastet_comm_recv);
}

#endif

