

#ifndef __OAL_WINDOWS_MM_H__
#define __OAL_WINDOWS_MM_H__

/* ����ͷ�ļ����� */
#include <windows.h>

/*
 * �� �� ��  : oal_memalloc
 * ��������  : �����ڴ�ռ䡣
 */
OAL_INLINE oal_void *oal_memalloc(oal_uint32 ul_size)
{
    oal_void *p_mem_space;

    p_mem_space = malloc(ul_size);

    return p_mem_space;
}

/*
 * �� �� ��  : oal_memtry_alloc
 * ��������  : �����������ڴ棬ָ�����������������С�ڴ棬
 *             ����ʵ��������ڴ棬����ʧ�ܷ���NULL
 */
OAL_STATIC OAL_INLINE oal_void *oal_memtry_alloc(oal_uint32 request_maxsize, oal_uint32 request_minsize,
                                                 oal_uint32 *actual_size)
{
    oal_void *puc_mem_space;
    oal_uint32 request_size;

    if (actual_size == NULL) {
        return NULL;
    }

    *actual_size = 0;

    request_size = (request_maxsize > request_minsize) ? request_maxsize : request_minsize;

    do {
        puc_mem_space = oal_memalloc(request_size);
        if (puc_mem_space != NULL) {
            *actual_size = request_size;
            return puc_mem_space;
        }

        if (request_size <= request_minsize) {
            /* ����СSIZE������Ȼʧ�ܷ���NULL */
            break;
        }

        /* ����ʧ��, �۰��������� */
        request_size = request_size >> 1;
        request_size = (request_size > request_minsize) ? request_size : request_minsize;
    } while (1);

    return NULL;
}

/*
 * �� �� ��  : oal_free
 * ��������  : �ͷź���̬���ڴ�ռ䡣
 * �������  : pbuf:���ͷŵ��ڴ��ַ
 */
OAL_INLINE oal_void oal_free(oal_void *p_buf)
{
    free(p_buf);
}

/*
 * �� �� ��  : oal_mem_dma_blockalloc
 * ��������  : ����������DMA�ڴ棬�������룬ֱ������ɹ����������ó�ʱ
 */
OAL_STATIC OAL_INLINE oal_void *oal_mem_dma_blockalloc(oal_uint32 size, oal_ulong timeout)
{
    OAL_REFERENCE(timeout);
    return oal_memalloc(size);
}

/*
 * �� �� ��  : oal_mem_dma_blockfree
 * ��������  : �ͷ�DMA�ڴ�
 */
OAL_STATIC OAL_INLINE oal_void oal_mem_dma_blockfree(oal_void *puc_mem_space)
{
    oal_free(puc_mem_space);
}

/*
 * �� �� ��  : oal_memcopy
 * ��������  : �����ڴ档�����������ڴ��ص���
 * �������  : *p_dst: �ڴ渴��Ŀ�ĵ�ַ
 *             *p_src: �ڴ渴��Դ��ַ
 *             ul_size: �����ڴ��С
 */
OAL_INLINE oal_void oal_memcopy(oal_void *p_dst, const oal_void *p_src, oal_uint32 ul_size)
{
    memcpy(p_dst, p_src, ul_size);
}

/*
 * �� �� ��  : oal_memmove
 * ��������  : �����ڴ档���������ڴ��ص���
 * �������  : *p_dst: mem move destination addr
 *             *p_src: mem move source addr
 *             ul_size: mem move size
 */
OAL_INLINE oal_void oal_memmove(oal_void *p_dst, const oal_void *p_src, oal_uint32 ul_size)
{
    memmove(p_dst, p_src, ul_size);
}

/*
 * �� �� ��  : oal_memset
 * ��������  : �ڴ���䡣
 * �������  : *p_buf: �ڴ����õ�ַ
 *             uc_char: char be set to mem
 *             ul_size: size of mem be set
 */
OAL_INLINE oal_void oal_memset(oal_void *p_buf, oal_int32 l_data, oal_uint32 ul_size)
{
    memset(p_buf, l_data, ul_size);
}

OAL_INLINE errno_t memset_s(void *dest, size_t destMax, int c, size_t count)
{
    (void)destMax;
    memset(dest, c, count);
}

#endif /* end of oal_mm.h */

