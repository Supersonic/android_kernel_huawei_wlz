

#ifndef __OAL_WINDOWS_TYPES_H__
#define __OAL_WINDOWS_TYPES_H__

/* 基本数据类型定义 */
typedef char oal_int8;                 /* 数据前缀:c */
typedef short oal_int16;               /* 数据前缀:s */
typedef int oal_int32;                 /* 数据前缀:l */
typedef long long oal_int64;           /* 数据前缀:ll */
typedef unsigned char oal_uint8;       /* 数据前缀:uc */
typedef unsigned short oal_uint16;     /* 数据前缀:us */
typedef unsigned int oal_uint32;       /* 数据前缀:ul */
typedef unsigned long long oal_uint64; /* 数据前缀:ull */
typedef void oal_void;

/* 宏定义 */
#ifdef INLINE_TO_FORCEINLINE
#define OAL_INLINE __forceinline
#else
#define OAL_INLINE __inline
#endif

#if ((_PRE_TEST_MODE_UT == _PRE_TEST_MODE) || (_PRE_TEST_MODE_ST == _PRE_TEST_MODE) || defined(_PRE_WIFI_DMT))
#define OAL_STATIC
#else
#define OAL_STATIC static
#endif

#define OAL_VOLATILE

#define LINUX_VERSION_CODE      0x30000
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

#define HZ 1000

#endif /* end of oal_types.h */

