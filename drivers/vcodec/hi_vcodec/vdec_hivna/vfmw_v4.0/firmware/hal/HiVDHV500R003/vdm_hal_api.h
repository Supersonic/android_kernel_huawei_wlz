/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdm hal api export header file.
 * Author: gaoyajun
 * Create: 2017-03-16
 */

#ifndef _VDM_HAL_API_HEADER_
#define _VDM_HAL_API_HEADER_

#include "basedef.h"
#include "mem_manage.h"
#include "vfmw.h"
#include "vdm_hal_local.h"
#include "vfmw_intf.h"

VOID vdmhal_imp_reset_vdm(SINT32 vdh_id);
VOID vdmhal_imp_glb_reset(void);
VOID vdmhal_imp_clear_int_state(SINT32 vdh_id);
SINT32 vdmhal_imp_check_reg(reg_id_e reg_id, SINT32 vdh_id);
VOID vdmhal_imp_starthwrepair(SINT32 vdh_id);
VOID vdmhal_imp_starthwdecode(SINT32 vdh_id);
SINT32 vdmhal_imp_prepare_dec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
SINT32 vdmhal_imp_prepare_repair(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
SINT32 vdmhal_imp_backupinfo(void);
VOID vdmhal_imp_getcharacter(void);
VOID vdmhal_imp_writescdemarid(void);
VOID vdmhal_imp_init(void);
VOID vdmhal_imp_deinit(void);

VOID vdmhal_isr(SINT32 vdh_id);
SINT32 vdmhal_hw_dec_proc(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
VOID vdmhal_getregstate(vdmhal_backup_s *vdm_reg_state);
SINT32 vdmhal_is_vdm_run(SINT32 vdh_id);

SINT32 vdmhal_prepare_sleep(void);
VOID vdmhal_force_sleep(void);
VOID vdmhal_exit_sleep(void);
vdmdrv_sleep_stage_e vdmhal_get_sleep_stage(void);
VOID vdmhal_set_sleep_stage(vdmdrv_sleep_stage_e sleep_state);
#ifdef MSG_POOL_ADDR_CHECK
SINT32 vdmhal_imp_open_hal(mem_buffer_s *open_param);
SINT32 vdmhal_imp_open_heif_hal(mem_buffer_s *open_param);
#endif
#endif
