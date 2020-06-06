

#ifndef __HMAC_TWT_H__
#define __HMAC_TWT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "hmac_fsm.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_H

/* 1 ����ͷ�ļ����� */
/* 2 �궨�� */
/* 3 ö�ٶ��� */
/* 4 ȫ�ֱ������� */
/* 5 ��Ϣͷ���� */
/* 6 ��Ϣ���� */
/* 7 STRUCT���� */
/* 8 UNION���� */
typedef enum {
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0 = 0,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32 = 1,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48 = 2,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64 = 3,

    HMAC_NEXT_TWT_SUBFIELD_SIZE_BUTT
} hmac_next_twt_subfield_size_enum;

/* 9 OTHERS���� */
/* 10 �������� */
#ifdef _PRE_WLAN_FEATURE_TWT
extern oal_uint32 hmac_twt_auto_teardown_session(mac_device_stru *pst_dev);
extern oal_uint32 hmac_twt_auto_setup_session(mac_device_stru *pst_device);
#endif /* _PRE_WLAN_FEATURE_P2P */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_twt.h */
