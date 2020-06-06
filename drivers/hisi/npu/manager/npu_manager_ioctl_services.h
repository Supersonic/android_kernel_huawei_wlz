#ifndef __NPU_MANAGER_IOCTL_SERVICE_H
#define __NPU_MANAGER_IOCTL_SERVICE_H

/* manager */
#define DEVDRV_MANAGER_MAGIC                    'M'
#define DEVDRV_MANAGER_GET_DEVNUM		_IO(DEVDRV_MANAGER_MAGIC, 2)
#define DEVDRV_MANAGER_GET_PLATINFO 		_IO(DEVDRV_MANAGER_MAGIC, 3)
#define DEVDRV_MANAGER_SVMVA_TO_DEVID  	 	_IO(DEVDRV_MANAGER_MAGIC, 4)
#define DEVDRV_MANAGER_GET_CHANNELINFO		_IO(DEVDRV_MANAGER_MAGIC, 5)
#define DEVDRV_MANAGER_GET_DEVIDS		_IO(DEVDRV_MANAGER_MAGIC, 19)
#define DEVDRV_MANAGER_GET_DEVINFO		_IO(DEVDRV_MANAGER_MAGIC, 20)
#define DEVDRV_MANAGER_GET_TRANSWAY		_IO(DEVDRV_MANAGER_MAGIC, 73)
#define DEVDRV_MANAGER_CMD_MAX_NR 		    (75)


#define DEVDRV_CTRL_CPU_ID 			(0x41D05)

enum hccl_trans_way {
	DRV_SDMA = 0x0,
	DRV_PCIE_DMA,
};

// TODO: include devdrv head file later!! will delete
struct devdrv_device_info {
	u8 env_type;

	u8 ai_cpu_ready_num;
	u8 ai_cpu_broken_map;
	u8 ai_core_ready_num;
	u8 ai_core_broken_map;
	u8 ai_subsys_ip_map;

	u32 ctrl_cpu_ip;
	u32 ctrl_cpu_id;
	u32 ctrl_cpu_core_num;
	u32 ctrl_cpu_endian_little;
	u32 ts_cpu_core_num;
	u32 ai_cpu_core_num;
	u32 ai_core_num;
	u32 ai_cpu_core_id;
	u32 ai_core_id;
	u32 aicpu_occupy_bitmap;

	u32 ts_load_fail;

	u32 min_sq_id;
	u32 max_sq_id;
	u32 min_cq_id;
	u32 max_cq_id;
	u32 min_stream_id;
	u32 max_stream_id;
	u32 min_event_id;
	u32 max_event_id;

	u32 res[5];
};


long devdrv_manager_ioctl(  struct file *filep,
							unsigned int cmd,
							unsigned long arg);
#endif /*__DEVDRV_MANAGER_H*/
