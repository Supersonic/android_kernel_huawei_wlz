

#ifndef __MAC_BOARD_H__
#define __MAC_BOARD_H__

/* 1 ����ͷ�ļ����� */
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_vap.h"
#include "dmac_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_BOARD_H

/* 2 �궨�� */
/* DFX �궨�� */
#if ((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) ||    \
    (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)) &&  \
    (!defined(_PRE_PC_LINT))
#define DFX_GET_PERFORMANCE_LOG_SWITCH_ENABLE(_uc_type) (0)
#define DFX_SET_PERFORMANCE_LOG_SWITCH_ENABLE(_uc_type, _uc_value)
#else
#define DFX_GET_PERFORMANCE_LOG_SWITCH_ENABLE(_uc_type)            dfx_get_performance_log_switch_enable_etc(_uc_type)
#define DFX_SET_PERFORMANCE_LOG_SWITCH_ENABLE(_uc_type, _uc_value)  \
    dfx_set_performance_log_switch_enable_etc(_uc_type, _uc_value)
#endif

/* 3 ö�ٶ��� */
/* DFX����ö�ٶ��� */
#ifdef _PRE_WLAN_DFT_STAT
typedef enum {
    DFX_PERFORMANCE_TX_LOG,
    DFX_PERFORMANCE_DUMP,
    DFX_PERFORMANCE_REV1,
    DFX_PERFORMANCE_REV2,
    DFX_PERFORMANCE_LOG_BUTT,
} dfx_performance_log_switch_enum;
typedef oal_uint8 dfx_performance_log_switch_enum_uint8;
#endif

typedef enum {
    BOARD_VER_HI1151 = 0,
    BOARD_VER_HI1102,
    BOARD_VER_HI1103,
    BOARD_VER_BUTT
} hisi_device_board_enum;
typedef oal_uint8 hisi_device_board_enum_uint8;

/* 4 ȫ�ֱ������� */
/* HOST CRX�ӱ� */
extern frw_event_sub_table_item_stru g_ast_dmac_host_crx_table_etc[HMAC_TO_DMAC_SYN_BUTT];

/* DMACģ�飬HOST_DRX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_dmac_tx_host_drx_etc[DMAC_TX_HOST_DRX_BUTT];

/* DMACģ�飬WLAN_DTX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_dmac_tx_wlan_dtx_etc[DMAC_TX_WLAN_DTX_BUTT];

/* DMACģ�飬WLAN_CTX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_dmac_wlan_ctx_event_sub_table_etc[DMAC_WLAN_CTX_EVENT_SUB_TYPE_BUTT];

/* DMACģ��,WLAN_DRX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_dmac_wlan_drx_event_sub_table_etc[HAL_WLAN_DRX_EVENT_SUB_TYPE_BUTT];

#if (((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)) ||   \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))     \
/* DMACģ�飬�����ȼ��¼���������ע��ṹ�嶨�� */
extern frw_event_sub_table_item_stru g_ast_dmac_high_prio_event_sub_table_etc[HAL_EVENT_DMAC_HIGH_PRIO_SUB_TYPE_BUTT];
#else
/* DMACģ�飬ERROR_IRQ�¼���������ע��ṹ�嶨�� */
extern frw_event_sub_table_item_stru g_ast_dmac_high_prio_event_sub_table_etc[HAL_EVENT_ERROR_IRQ_SUB_TYPE_BUTT];
#endif

/* DMACģ��,WLAN_CRX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_dmac_wlan_crx_event_sub_table_etc[HAL_WLAN_CRX_EVENT_SUB_TYPE_BUTT];

/* DMACģ�飬TX_COMP�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_dmac_tx_comp_event_sub_table_etc[HAL_TX_COMP_SUB_TYPE_BUTT];

/* DMACģ��, TBTT�¼��������������� */
extern frw_event_sub_table_item_stru g_ast_dmac_tbtt_event_sub_table_etc[HAL_EVENT_TBTT_SUB_TYPE_BUTT];

/* DMACģ��, MISC�¼��������������� */
extern frw_event_sub_table_item_stru g_ast_dmac_misc_event_sub_table_etc[HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT];

/* WLAN_DTX �¼������ͱ� */
extern frw_event_sub_table_item_stru g_ast_hmac_wlan_dtx_event_sub_table_etc[DMAC_TX_WLAN_DTX_BUTT];

/* HMACģ�� WLAN_DRX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_hmac_wlan_drx_event_sub_table_etc[DMAC_WLAN_DRX_EVENT_SUB_TYPE_BUTT];

/* HMACģ�� WLAN_CRX�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_hmac_wlan_crx_event_sub_table_etc[DMAC_WLAN_CRX_EVENT_SUB_TYPE_BUTT];

/* HMACģ�� TBTT�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_hmac_tbtt_event_sub_table_etc[DMAC_TBTT_EVENT_SUB_TYPE_BUTT];

/* HMACģ�� ����HOST��������¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_hmac_wlan_ctx_event_sub_table_etc[DMAC_TO_HMAC_SYN_BUTT];

/* HMACģ�� MISC��ɢ�¼���������ע��ṹ���� */
extern frw_event_sub_table_item_stru g_ast_hmac_wlan_misc_event_sub_table_etc[DMAC_MISC_SUB_TYPE_BUTT];

extern hisi_device_board_enum_uint8 g_en_chip_type;

/* 5 ��Ϣͷ���� */
/* 6 ��Ϣ���� */
/* 7 STRUCT���� */
/* 8 UNION���� */
/* 9 OTHERS���� */
/* 10 �������� */
/* DFX���ú������� */
#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32 dfx_get_performance_log_switch_enable_etc(
    dfx_performance_log_switch_enum_uint8 uc_performance_log_switch_type);
extern oal_void dfx_set_performance_log_switch_enable_etc(
    dfx_performance_log_switch_enum_uint8 uc_performance_log_switch_type, oal_uint8 uc_value);
#endif

extern oal_void event_fsm_unregister_etc(oal_void);

extern oal_void event_fsm_table_register_etc(oal_void);

#endif /* end of mac_board */


