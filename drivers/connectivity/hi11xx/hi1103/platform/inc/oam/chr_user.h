

#ifndef __CHR_USER_H__
#define __CHR_USER_H__

/* Other Include Head File */
#include "chr_errno.h"

/* �궨�� */
/* �����ϱ� */
/* оƬУ׼�쳣�¼��ϱ� */
#define CHR_CHIP_CALI_ERR_REPORT_EVENTID 909003001
/* оƬƽ̨�쳣�¼��ϱ� */
#define CHR_PLATFORM_EXCEPTION_EVENTID 909003002
/* оƬ����ʧ���¼��ϱ� */
#define CHR_WIFI_CONNECT_FAIL_REPORT_EVENTID 909003003
/* оƬ�����Ͽ��¼��ϱ� */
#define CHR_WIFI_DISCONNECT_REPORT_EVENTID 909003004
/* wifi�¿�ʱ���ϱ� */
#define CHR_WIFI_TEMP_PROTECT_REPORT_EVENTID 909003005
/* wifi PLL ʧ�� */
#define CHR_WIFI_PLL_LOST_REPORT_EVENTID 909003006
/* wifi ����ͳ���¼��ϱ� */
#define CHR_WIFI_ROAM_INFO_REPORT_EVENTID 909009105

/* ������ѯ */
/* wifi��������ͳ���ϱ� */
#define CHR_WIFI_POW_RATE_COUNT_QUERY_EVENTID 909009519
/* wifi����ʱ��ͳ���ϱ� */
#define CHR_WIFI_WORK_TIME_INFO_EVENTID 909009520

/* wifi�򿪹ر�ʧ�� */
#define CHR_WIFI_OPEN_CLOSE_FAIL_QUERY_EVENTID 909002021
/* wifi�쳣�Ͽ� */
#define CHR_WIFI_DISCONNECT_QUERY_EVENTID 909002022
/* wifi�����쳣 */
#define CHR_WIFI_CONNECT_FAIL_QUERY_EVENTID 909002023
/* wifi����ʧ�� */
#define CHR_WIFI_WEB_FAIL_QUERY_EVENTID 909002024
/* wifi������ */
#define CHR_WIFI_WEB_SLOW_QUERY_EVENTID 909002025

/* BT����쳣�ϱ� */
#define CHR_BT_CHIP_SOFT_ERROR_EVENTID 913900003

#define CHR_EXCEPTION_MODULE_NAME_LEN 10
#define CHR_EXCEPTION_ERROR_CODE_LEN  20

/* STRUCT���� */
typedef struct chr_chip_excption_event_info_tag {
    oal_uint32 ul_module;
    oal_uint32 ul_plant;
    oal_uint32 ul_subplant;
    oal_uint32 ul_errid;
} chr_platform_exception_event_info_stru;

typedef struct chr_scan_exception_event_info_tag {
    int sub_event_id;
    char module_name[CHR_EXCEPTION_MODULE_NAME_LEN];
    char error_code[CHR_EXCEPTION_ERROR_CODE_LEN];

} chr_scan_exception_event_info_stru;

typedef struct chr_connect_exception_event_info_tag {
    int sub_event_id;
    char platform_module_name[CHR_EXCEPTION_MODULE_NAME_LEN];
    char error_code[CHR_EXCEPTION_ERROR_CODE_LEN];

} chr_connect_exception_event_info_stru;

typedef enum chr_LogPriority {
    CHR_LOG_DEBUG = 0,
    CHR_LOG_INFO,
    CHR_LOG_WARN,
    CHR_LOG_ERROR,
} CHR_LOGPRIORITY;

typedef enum chr_dev_index {
    CHR_INDEX_KMSG_PLAT = 0,
    CHR_INDEX_KMSG_WIFI,
    CHR_INDEX_APP_WIFI,
    CHR_INDEX_APP_GNSS,
    CHR_INDEX_APP_BT,
#ifdef CONFIG_CHR_OTHER_DEVS
    CHR_INDEX_APP_FM,
    CHR_INDEX_APP_NFC,
    CHR_INDEX_APP_IR,
#endif
    CHR_INDEX_MUTT,
} CHR_DEV_INDEX;

#define CHR_LOG_TAG_PLAT CHR_INDEX_KMSG_PLAT
#define CHR_LOG_TAG_WIFI CHR_INDEX_KMSG_WIFI

#ifdef _PRE_CONFIG_HW_CHR
#define CHR_ERR_DATA_MAX_NUM 0x20
#define CHR_ERR_DATA_MAX_LEN (OAL_SIZEOF(oal_uint32) * CHR_ERR_DATA_MAX_NUM)
typedef struct {
    oal_uint32 errno;
    oal_uint16 errlen;
    oal_uint16 flag : 1;
    oal_uint16 resv : 15;
} CHR_ERRNO_WITH_ARG_STRU;

typedef uint32 (*chr_get_wifi_info)(uint32);

extern int32 __chr_printLog_etc(CHR_LOGPRIORITY prio, CHR_DEV_INDEX dev_index, const int8 *fmt, ...);
extern int __chr_exception_etc(uint32 errno);
extern void chr_dev_exception_callback_etc(void *buff, uint16 len);
extern int32 __chr_exception_para(uint32 chr_errno, uint8 *chr_ptr, uint16 chr_len);
extern void chr_host_callback_register(chr_get_wifi_info pfunc);
extern void chr_host_callback_unregister(void);
extern void chr_test(void);

#define CHR_LOG(prio, tag, fmt...)                   __chr_printLog_etc(prio, tag, ##fmt)
#define CHR_EXCEPTION(errno)                         __chr_exception_etc(errno)
#define CHR_EXCEPTION_P(chr_errno, chr_ptr, chr_len) __chr_exception_para(chr_errno, chr_ptr, chr_len)


#define CHR_EXCEPTION_REPORT(excption_event, module, plant, subplant, errid)                 \
    do {                                                                                     \
        chr_platform_exception_event_info_stru chr_platform_exception_event_info;            \
        chr_platform_exception_event_info.ul_module = module;                                \
        chr_platform_exception_event_info.ul_plant = plant;                                  \
        chr_platform_exception_event_info.ul_subplant = subplant;                            \
        chr_platform_exception_event_info.ul_errid = errid;                                  \
        CHR_EXCEPTION_P(excption_event, (oal_uint8 *)(&chr_platform_exception_event_info),   \
                        OAL_SIZEOF(chr_platform_exception_event_info_stru));                 \
    } while (0)

#else
#define CHR_LOG(prio, tag, fmt, ...)
#define CHR_EXCEPTION(chr_errno)
#define CHR_EXCEPTION_P(chr_errno, chr_ptr, chr_len)
#define CHR_EXCEPTION_REPORT(excption_event, module, plant, subplant, errid)
#endif
#endif
