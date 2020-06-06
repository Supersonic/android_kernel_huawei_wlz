

#ifndef __HMAC_TX_OPT_H__
#define __HMAC_TX_OPT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 ͷ�ļ����� */
#include "oal_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_OPT_H

/* 2 �궨�� */
#define MAX_TX_OPT_SWITCH_CNT 3

/* 3 ö�ٶ��� */
/* 4 ȫ�ֱ������� */
/* 5 ��Ϣͷ���� */
/* 6 ��Ϣ���� */
/* 7 STRUCT���� */
/* 8 UNION���� */
/* 9 OTHERS���� */
/* 10 �������� */
extern oal_void hmac_tx_opt_switch(oal_uint32 ul_tx_large_pps);
extern oal_void hmac_set_tx_opt_switch_cnt(oal_uint8 uc_opt_switch_cnt);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
