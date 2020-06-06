

#ifndef __HMAC_M2S_H__
#define __HMAC_M2S_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

/* 1 其他头文件包含 */
#include "frw_ext_if.h"
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_M2S_H

/* 2 宏定义 */
#define M2S_ARP_FAIL_REASSOC_NUM 9
#define M2S_ARP_PROBE_TIMEOUT    300  // ms
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    wlan_m2s_mgr_vap_stru ast_m2s_blacklist[WLAN_M2S_BLACKLIST_MAX_NUM]; /* 处于管理用户时，需要调整action方案 */
    oal_uint8 uc_blacklist_bss_index;                                    /* 黑名单MAC地址的数组下标 */
    oal_uint8 uc_blacklist_bss_cnt;                                      /* 黑名单用户个数 */
} hmac_device_m2s_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_void hmac_m2s_vap_arp_probe_process(oal_void *p_arg, oal_bool_enum_uint8 en_arp_detect_on);
extern oal_void hmac_m2s_arp_fail_process(oal_netbuf_stru *pst_netbuf, oal_void *p_arg);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_M2S_H__ */

