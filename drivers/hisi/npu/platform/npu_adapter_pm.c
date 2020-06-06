#include "drv_ipc.h"
#include "npu_ipc_msg.h"
#include "npu_atf_subsys.h"
#include "npu_platform.h"
#include "npu_adapter.h"
#include "npu_platform_register.h"
#include "bbox/npu_black_box.h"


static int send_ipc_msg_to_ts(u32 cmd_type, u32 sync_type, u8 *send_msg, u8 send_len)
{
	rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH] = {0};
	rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
	rproc_msg_t ack_buff = 0;
	struct ipcdrv_msg_payload *payload = NULL;
	struct ipcdrv_message *ipc_msg = NULL;
	int ret;

	ipc_msg = (struct ipcdrv_message *)msg;
	ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
	ipc_msg->ipc_msg_header.cmd_type = cmd_type;
	ipc_msg->ipc_msg_header.sync_type = sync_type;
	ipc_msg->ipc_msg_header.reserved = 0;
	ipc_msg->ipc_msg_header.msg_length = 1;
	ipc_msg->ipc_msg_header.msg_index = 0;

	if((NULL != send_msg) && (send_len != 0))
	{
		if(NULL == memcpy(ipc_msg->ipcdrv_payload, send_msg, send_len))
		{
			devdrv_drv_err("memcpy failed.\n");
			return -1;
		}
	}

	ipc_msg = (struct ipcdrv_message *)ack_buffer;
	payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
	payload->result = 0;

	ret = hisi_rproc_xfer_sync(HISI_RPROC_NPU_MBX2,
				msg,
					IPCDRV_RPROC_MSG_LENGTH, &ack_buff, 1);
	if (ret != 0) {
		devdrv_drv_err("hisi_rproc_xfer_async failed.\n");
		return -1;
	}
	return 0;
}


static int inform_ts_power_down(void)
{
	u8 send_msg = 0;

	return send_ipc_msg_to_ts(IPCDRV_TS_SUSPEND, IPCDRV_MSG_ASYNC, &send_msg, 1);
}
int devdrv_plat_send_ts_ctrl_core(uint32_t core_num)
{
	u8 send_msg = (u8)core_num;

	return send_ipc_msg_to_ts(IPCDRV_TS_INFORM_TS_LIMIT_AICORE, IPCDRV_MSG_ASYNC, &send_msg, 1);
}

int devdrv_plat_powerup_till_npucpu(u64 is_secure)
{
	int tmp_ret = 0;
	int ret = 0;
	NPU_DRV_BOOT_TIME_TAG("start npuatf_enable_secmod \n");
	ret = npuatf_enable_secmod(is_secure);
	if (ret != 0)
	{
		devdrv_drv_err("npu subsys power up failed ,ret = 0x%x\n",ret);
		return ret;
	}

	NPU_DRV_BOOT_TIME_TAG("start npuatf_change_slv_secmod\n");
	ret = npuatf_change_slv_secmod(is_secure);
	if (ret != 0)
	{
		devdrv_drv_err("change slv secmod fail,ret = 0x%x\n",ret);
		return ret;
	}

	NPU_DRV_BOOT_TIME_TAG("start acpu_gic0_online_ready\n");
	ret = acpu_gic0_online_ready(is_secure);
	if (ret != 0)
	{
		devdrv_drv_err("gic connect fail,ret = 0x%x\n",ret);
		return ret;
	}
	NPU_DRV_BOOT_TIME_TAG("start atf_query_gic0_state\n");
	tmp_ret = atf_query_gic0_state(NPU_GIC_1);
	//1 means online, 0 offline
	if (tmp_ret != 1)
	{
		devdrv_drv_err("gic connect check fail,tmp_ret = 0x%x\n",tmp_ret);
		ret = -1;
		return ret;
	}

	return ret;
}

extern struct devdrv_platform_info *s_platform_info;

int devdrv_plat_powerup_till_ts(u32 is_secure, u32 offset)
{
	int ret = 0;

	NPU_DRV_BOOT_TIME_TAG("start npuatf_start_secmod\n");

	//1.unreset ts  2.polling boot status on atf
	ret = npuatf_start_secmod(is_secure);
	if (ret != 0)
	{
		devdrv_drv_err("ts unreset fail,ret = 0x%x\n",ret);
		rdr_system_error(RDR_EXC_TYPE_TS_INIT_EXCEPTION, 0, 0);
		return ret;
	}
	NPU_DRV_BOOT_TIME_TAG("end npuatf_start_secmod\n");
	return ret;
}

int devdrv_plat_powerdown_till_npucpu(u32 offset,u32 is_secure)
{
	int ret = 0;

	//step1. inform ts begining power down
	if (is_secure != NPU_SEC) {
		ret = inform_ts_power_down();
		if (ret) {
			devdrv_drv_err("npuatf_inform_power_down failed ret = %d!\n",ret);
			return ret;
		}
	}

	//step2. wait ts flag, and start set the security register of TS PD flow on atf now

	//update secure register of GIC_WAKER and GIC_PWRR through bl31
	//to end communication between tscpu and npu gic and close GICR0 and
	//GICR1
	//doorbell do it at atf
	ret = npuatf_power_down_ts_secreg(is_secure);
	if (ret != 0) {
		devdrv_drv_err("end communication between tscpu and npu gic and "
		"close GICR0 and GICR1 failed ret = %d \n", ret);
	}
	devdrv_drv_info("end communication between tscpu and npu gic and "
										"close GICR0 and GICR1 success\n");

	//step3. inform ts that secrity register had been powered down on atf now

	return 0;
}

int devdrv_plat_powerdown_till_down(u32 expect_val, u32 offset)
{
	int ret = 0;

	//step4 wait tscpu to be idle state(do it at atf now)

	//step5 power down npucpu, npubus and npusubsys through bl31
	ret = npuatf_power_down();
	if (ret != 0) {
		devdrv_drv_err("power down npucpu npu bus and npu "
							"subsystem through bl31 failed ret = %d\n",ret);
	}
	return ret;
}

int devdrv_plat_powerup_tbu(void)
{
	int ret = 0;
	devdrv_drv_info("start npuatf_enable_tbu \n");
	ret = npuatf_enable_tbu(NPU_NONSEC);
	if (ret != 0)
	{
		devdrv_drv_err("npu subsys power up failed ,ret = 0x%x\n",ret);
		return ret;
	}
	return 0;
}

int devdrv_plat_powerdown_tbu(void)
{
	int ret = 0;
	devdrv_drv_info("start npuatf_disable_tbu \n");
	ret = npuatf_disable_tbu(NPU_NONSEC);
	if (ret != 0)
	{
		devdrv_drv_err("npu subsys powern down failed ,ret = 0x%x\n",ret);
		return ret;
	}
	return 0;
}

