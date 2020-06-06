

#ifndef __PREPARE_FRAME_STA_H__
#define __PREPARE_FRAME_STA_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "oal_types.h"
#include "hmac_vap.h"

/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
extern oal_uint32 hmac_mgmt_encap_asoc_req_sta_etc(hmac_vap_stru *pst_hmac_sta,
                                                   oal_uint8 *puc_req_frame,
                                                   oal_uint8 *puc_curr_bssid);
extern oal_uint16 hmac_mgmt_encap_auth_req_etc(hmac_vap_stru *pst_sta, oal_uint8 *puc_mgmt_frame);
extern oal_uint16 hmac_mgmt_encap_auth_req_seq3_etc(hmac_vap_stru *pst_sta,
                                                    oal_uint8 *puc_mgmt_frame,
                                                    oal_uint8 *puc_mac_hrd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_encap_frame_sta.h */
