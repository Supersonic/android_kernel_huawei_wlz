

#ifndef __OAL_LINUX_EXT_IF_H__
#define __OAL_LINUX_EXT_IF_H__

#include "platform_oneimage_define.h"

/* 宏定义 */
/* 用此宏修饰的函数，在函数入口和出口处，编译器不会插入调用__cyg_profile_func_enter和__cyg_profile_func_exit的代码 */
#if defined(_PRE_PC_LINT)
#define ATTR_OAL_NO_FUNC_TRACE
#elif (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define ATTR_OAL_NO_FUNC_TRACE __attribute__((__no_instrument_function__))
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#define INIT_COMPLETION(x) reinit_completion(&x)
#define IRQF_DISABLED      0
#define N_HW_BFG           35
#endif

/* 全局变量声明 */
extern oal_mempool_info_to_sdt_stru mempool_info_etc;

/* OTHERS定义 */
typedef oal_uint32 (*oal_stats_info_up_to_sdt)(oal_uint8 en_pool_id,
                                               oal_uint16 us_mem_total_cnt,
                                               oal_uint16 us_mem_used_cnt,
                                               oal_uint8 uc_subpool_id,
                                               oal_uint16 us_total_cnt,
                                               oal_uint16 us_free_cnt);
typedef oal_uint32 (*oal_memblock_info_up_to_sdt)(oal_uint8 *puc_origin_data,
                                                  oal_uint8 uc_user_cnt,
                                                  oal_mem_pool_id_enum_uint8 en_pool_id,
                                                  oal_uint8 uc_subpool_id,
                                                  oal_uint16 us_len,
                                                  oal_uint32 ul_file_id,
                                                  oal_uint32 ul_alloc_line_num);

/* 函数声明 */
extern oal_uint32 oal_mempool_info_to_sdt_register_etc(oal_stats_info_up_to_sdt p_up_mempool_info,
                                                       oal_memblock_info_up_to_sdt p_up_memblock_info);

#endif /* end of oal_ext_if.h */
