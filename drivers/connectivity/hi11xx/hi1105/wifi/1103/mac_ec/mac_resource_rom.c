

/* 1 ͷ�ļ����� */
#include "wlan_spec.h"
#include "mac_resource.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "dmac_vap.h"
#include "dmac_user.h"
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#include "hmac_vap.h"
#include "hmac_user.h"
#endif

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_RESOURCE_ROM_C

/* 2 ȫ�ֱ������� */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
extern dmac_vap_stru g_ast_dmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
extern dmac_user_stru g_ast_dmac_user[MAC_RES_MAX_USER_LIMIT];
extern mac_res_user_idx_size_stru g_st_dmac_user_idx_size[MAC_RES_MAX_USER_LIMIT];
extern mac_res_user_cnt_size_stru g_st_dmac_user_cnt_size[MAC_RES_MAX_USER_LIMIT];

dmac_vap_stru *g_pst_dmac_vap = &g_ast_dmac_vap[0];
dmac_user_stru *g_pst_dmac_user = &g_ast_dmac_user[0];
mac_res_user_idx_size_stru *g_pst_dmac_user_idx_size = &g_st_dmac_user_idx_size[0];
mac_res_user_cnt_size_stru *g_pst_dmac_user_cnt_size = &g_st_dmac_user_cnt_size[0];
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
extern hmac_vap_stru g_ast_hmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
extern hmac_user_stru g_ast_hmac_user[MAC_RES_MAX_USER_LIMIT];
#endif

/* �������붨�ƻ���ˢ */
/* 1��chip֧�ֵ��������û��� */
oal_uint16 g_us_max_asoc_user_etc = WLAN_ASSOC_USER_MAX_NUM;
/* 1��chip֧�ֵ���󼤻��û��� */
oal_uint8 g_uc_max_active_user = WLAN_ACTIVE_USER_MAX_NUM;

mac_res_stru *g_pst_mac_res = &g_st_mac_res_etc;

/* 3 ����ʵ�� */

oal_uint32 mac_res_check_spec_etc(oal_void)
{
    oal_uint32 ul_ret = OAL_SUCC;
    return ul_ret;
}

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))

oal_void mac_res_vap_init(oal_void)
{
    oal_uint32  ul_loop;
    oal_uint    ul_one_vap_size;

    g_pst_mac_res->st_vap_res.us_hmac_priv_size = 0;
    ul_one_vap_size = 0;
    for (ul_loop = 0; ul_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; ul_loop++) {
        g_pst_mac_res->st_vap_res.past_vap_info[ul_loop] = (oal_uint8 *)g_pst_dmac_vap + ul_one_vap_size;
        memset_s(g_pst_mac_res->st_vap_res.past_vap_info[ul_loop],
                 OAL_SIZEOF(dmac_vap_stru), 0, OAL_SIZEOF(dmac_vap_stru));

        ul_one_vap_size += OAL_SIZEOF(dmac_vap_stru);
        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_pst_mac_res->st_vap_res.auc_user_cnt[ul_loop] = 0;
    }
}


oal_uint32 mac_res_user_init_etc(oal_void)
{
    oal_uint32  ul_loop;
    oal_uint    ul_one_user_info_size;

    if (OAL_ANY_NULL_PTR3(g_pst_dmac_user, g_pst_dmac_user_idx_size, g_pst_dmac_user_cnt_size)) {
        OAM_ERROR_LOG0(0, OAM_SF_UM, "{mac_res_user_init_etc:input para null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* �ڴ��ʼ��0 */
    memset_s(g_pst_dmac_user,
             (OAL_SIZEOF(dmac_user_stru) * MAC_RES_MAX_USER_LIMIT), 0,
             (OAL_SIZEOF(dmac_user_stru) * MAC_RES_MAX_USER_LIMIT));
    memset_s(g_pst_dmac_user_idx_size,
             (OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_LIMIT), 0,
             (OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_LIMIT));
    memset_s(g_pst_dmac_user_cnt_size,
             (OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_LIMIT), 0,
             (OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_LIMIT));

    g_pst_mac_res->st_user_res.pul_idx = (oal_uint *)g_pst_dmac_user_idx_size;
    g_pst_mac_res->st_user_res.puc_user_cnt = (oal_uint8 *)g_pst_dmac_user_cnt_size;

    g_pst_mac_res->st_user_res.us_hmac_priv_size = 0;

    ul_one_user_info_size = 0;
    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_LIMIT; ul_loop++) {
        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_pst_mac_res->st_user_res.past_user_info[ul_loop] = (oal_uint8 *)g_pst_dmac_user + ul_one_user_info_size;
        ul_one_user_info_size += OAL_SIZEOF(dmac_user_stru);
    }

    return OAL_SUCC;
}


oal_uint32 mac_res_exit_etc(void)
{
    oal_uint ul_loop;

    OAL_MEM_FREE((g_pst_mac_res->st_user_res.past_user_info[0]), OAL_TRUE);

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_LIMIT; ul_loop++) {
        g_pst_mac_res->st_user_res.past_user_info[ul_loop] = OAL_PTR_NULL;
    }

    g_pst_mac_res->st_user_res.pul_idx = OAL_PTR_NULL;
    g_pst_mac_res->st_user_res.puc_user_cnt = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32 mac_res_init_etc(oal_void)
{
    oal_uint    ul_loop;
    oal_uint32  ul_ret;

    memset_s(g_pst_mac_res, OAL_SIZEOF(mac_res_stru), 0, OAL_SIZEOF(mac_res_stru));
    /* ��ʼ��DEV����Դ�������� */
    oal_queue_set(&(g_pst_mac_res->st_dev_res.st_queue),
                  g_pst_mac_res->st_dev_res.aul_idx,
                  MAC_RES_MAX_DEV_NUM);

    ul_ret = mac_res_check_spec_etc();
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init_etc::mac_res_init_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_DEV_NUM; ul_loop++) {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue(&(g_pst_mac_res->st_dev_res.st_queue), (oal_void *)(uintptr_t)(ul_loop + 1));

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_pst_mac_res->st_dev_res.auc_user_cnt[ul_loop] = 0;
    }

    /* ��ʼ��VAP����Դ�������� */
    mac_res_vap_init();
    /*
     * ��ʼ��USER����Դ��������
     * ��ʼ��HASHͰ����Դ��������
     */
    ul_ret = mac_res_user_init_etc();
    if (ul_ret != OAL_SUCC) {
        MAC_ERR_LOG1(0, "mac_res_init_etc: mac_res_user_init_etc return err code", ul_ret);
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init_etc::mac_res_user_init_etc failed[%d].}", ul_ret);

        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 mac_res_free_mac_user_etc(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY(us_idx >= MAC_RES_MAX_USER_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->st_user_res.puc_user_cnt[us_idx] == 0) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_res_free_mac_user_etc::cnt==0! idx:%d", us_idx);
        oal_dump_stack();
        return OAL_ERR_CODE_USER_RES_CNT_ZERO;
    }

    (g_pst_mac_res->st_user_res.puc_user_cnt[us_idx])--;

    if (g_pst_mac_res->st_user_res.puc_user_cnt[us_idx] != 0) {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_uint32 mac_res_free_mac_vap_etc(oal_uint32 ul_idx)
{
    if (OAL_UNLIKELY(ul_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->st_vap_res.auc_user_cnt[ul_idx] == 0) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_res_free_mac_vap_etc::cnt==0! idx:%d", ul_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_pst_mac_res->st_vap_res.auc_user_cnt[ul_idx])--;

    if (g_pst_mac_res->st_vap_res.auc_user_cnt[ul_idx] != 0) {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}

#else

oal_void mac_res_vap_init(oal_void)
{
    oal_uint ul_loop;

    oal_queue_set(&(g_pst_mac_res->st_vap_res.st_queue),
                  g_pst_mac_res->st_vap_res.aul_idx,
                  (oal_uint8)WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);
    g_pst_mac_res->st_vap_res.us_hmac_priv_size = (oal_uint16)OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info);

    for (ul_loop = 0; ul_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; ul_loop++) {
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
        g_pst_mac_res->st_vap_res.past_vap_info[ul_loop] = (oal_void *)&g_ast_hmac_vap[ul_loop];
        memset_s(g_pst_mac_res->st_vap_res.past_vap_info[ul_loop],
                 OAL_SIZEOF(hmac_vap_stru), 0,
                 OAL_SIZEOF(hmac_vap_stru));
#else
        g_pst_mac_res->st_vap_res.past_vap_info[ul_loop] = (oal_void *)&g_ast_mac_res_vap[ul_loop];
        memset_s(g_pst_mac_res->st_vap_res.past_vap_info[ul_loop],
                 OAL_SIZEOF(mac_res_mem_vap_stru), 0,
                 OAL_SIZEOF(mac_res_mem_vap_stru));
#endif

        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue (&(g_pst_mac_res->st_vap_res.st_queue), (oal_void *)(uintptr_t)(ul_loop + 1));
        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_pst_mac_res->st_vap_res.auc_user_cnt[ul_loop] = 0;
    }
}

oal_uint32 mac_res_user_init_etc(oal_void)
{
    oal_uint  ul_loop;
    oal_void *p_user_info = OAL_PTR_NULL;
    oal_void *p_idx = OAL_PTR_NULL;
    oal_void *p_user_cnt = OAL_PTR_NULL;
    oal_uint  ul_one_user_info_size;

    /* ��ʼ��USER����Դ�������� */
    /* ��̬�����û���Դ������ڴ� */
    p_user_info = (oal_void *)g_ast_hmac_user;

    p_idx = oal_memalloc(OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_LIMIT);
    p_user_cnt = oal_memalloc(OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_LIMIT);
    if (OAL_ANY_NULL_PTR3(p_user_info, p_idx, p_user_cnt)) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_user_init_etc::param null.}");

        if (p_idx != OAL_PTR_NULL) {
            oal_free(p_idx);
        }

        if (p_user_cnt != OAL_PTR_NULL) {
            oal_free(p_user_cnt);
        }

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* �ڴ��ʼ��0 */
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
    memset_s(p_user_info, (OAL_SIZEOF(hmac_user_stru) * MAC_RES_MAX_USER_LIMIT),
             0, (OAL_SIZEOF(hmac_user_stru) * MAC_RES_MAX_USER_LIMIT));
#else
    memset_s(p_user_info, (OAL_SIZEOF(mac_res_mem_user_stru) * MAC_RES_MAX_USER_LIMIT),
             0, (OAL_SIZEOF(mac_res_mem_user_stru) * MAC_RES_MAX_USER_LIMIT));
#endif

    memset_s(p_idx, (OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_LIMIT),
             0, (OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_LIMIT));
    memset_s(p_user_cnt, (OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_LIMIT),
             0, (OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_LIMIT));

    g_pst_mac_res->st_user_res.pul_idx = p_idx;
    g_pst_mac_res->st_user_res.puc_user_cnt = p_user_cnt;

    oal_queue_set_16(&(g_pst_mac_res->st_user_res.st_queue),
                     g_pst_mac_res->st_user_res.pul_idx,
                     (oal_uint16)MAC_RES_MAX_USER_LIMIT);
    g_pst_mac_res->st_user_res.us_hmac_priv_size = 0;

    ul_one_user_info_size = 0;
    g_pst_mac_res->st_user_res.past_user_info[0] = p_user_info;
    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_LIMIT; ul_loop++) {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue_16 (&(g_pst_mac_res->st_user_res.st_queue), (oal_void *)(uintptr_t)(ul_loop + 1));

        /* ��ʼ����Ӧ������λ�� */
        g_pst_mac_res->st_user_res.past_user_info[ul_loop] = (oal_uint8 *)p_user_info + ul_one_user_info_size;
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
        ul_one_user_info_size += OAL_SIZEOF(hmac_user_stru);
#else
        ul_one_user_info_size += OAL_SIZEOF(mac_res_mem_user_stru);
#endif
    }

    return OAL_SUCC;
}

oal_uint32 mac_res_exit_etc(void)
{
    oal_uint ul_loop;

    oal_free(g_pst_mac_res->st_user_res.pul_idx);
    oal_free(g_pst_mac_res->st_user_res.puc_user_cnt);
    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_LIMIT; ul_loop++) {
        g_pst_mac_res->st_user_res.past_user_info[ul_loop] = OAL_PTR_NULL;
    }
    g_pst_mac_res->st_user_res.pul_idx = OAL_PTR_NULL;
    g_pst_mac_res->st_user_res.puc_user_cnt = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32 mac_res_init_etc(oal_void)
{
    oal_uint    ul_loop;
    oal_uint32  ul_ret;

    ul_ret = mac_res_check_spec_etc();
    if (ul_ret != OAL_SUCC) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init_etc::mac_res_user_init_etc failed[%d].}", ul_ret);
        return ul_ret;
    }

    memset_s(g_pst_mac_res, OAL_SIZEOF(mac_res_stru), 0, OAL_SIZEOF(mac_res_stru));
    /* ��ʼ��DEV����Դ�������� */
    oal_queue_set(&(g_pst_mac_res->st_dev_res.st_queue),
                  g_pst_mac_res->st_dev_res.aul_idx,
                  MAC_RES_MAX_DEV_NUM);

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_DEV_NUM; ul_loop++) {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue (&(g_pst_mac_res->st_dev_res.st_queue), (oal_void *)(uintptr_t)(ul_loop + 1));

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_pst_mac_res->st_dev_res.auc_user_cnt[ul_loop] = 0;
    }

    /* lint -e413 */
    /* ��ʼ��VAP����Դ�������� */
    mac_res_vap_init();

    /*
     * ��ʼ��USER����Դ��������
     * ��ʼ��HASHͰ����Դ��������
     */
    ul_ret = mac_res_user_init_etc();
    if (ul_ret != OAL_SUCC) {
        MAC_ERR_LOG1(0, "mac_res_init_etc: mac_res_user_init_etc return err code", ul_ret);
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init_etc::mac_res_user_init_etc failed[%d].}", ul_ret);

        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 mac_res_free_mac_user_etc(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY(us_idx >= MAC_RES_MAX_USER_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->st_user_res.puc_user_cnt[us_idx] == 0) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_res_free_mac_user_etc::cnt==0! idx:%d", us_idx);
        oal_dump_stack();
        return OAL_ERR_CODE_USER_RES_CNT_ZERO;
    }

    (g_pst_mac_res->st_user_res.puc_user_cnt[us_idx])--;

    if (g_pst_mac_res->st_user_res.puc_user_cnt[us_idx] != 0) {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue_16(&(g_pst_mac_res->st_user_res.st_queue), (oal_void *)(uintptr_t)((oal_uint) us_idx + 1));

    return OAL_SUCC;
}


oal_uint32 mac_res_free_mac_vap_etc(oal_uint32 ul_idx)
{
    if (OAL_UNLIKELY(ul_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->st_vap_res.auc_user_cnt[ul_idx] == 0) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_res_free_mac_vap_etc::cnt==0! idx:%d", ul_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_pst_mac_res->st_vap_res.auc_user_cnt[ul_idx])--;

    if (g_pst_mac_res->st_vap_res.auc_user_cnt[ul_idx] != 0) {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue(&(g_pst_mac_res->st_vap_res.st_queue), (oal_void *)(uintptr_t)((oal_uint) ul_idx + 1));

    return OAL_SUCC;
}

#endif


oal_uint32 mac_res_alloc_dmac_dev_etc(oal_uint8 *puc_dev_idx)
{
    oal_uint ul_dev_idx_temp;

    if (OAL_UNLIKELY(puc_dev_idx == OAL_PTR_NULL)) {
        OAL_IO_PRINT("mac_res_alloc_dmac_dev_etc: OAL_PTR_NULL == pul_dev_idx");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_dmac_dev_etc::puc_dev_idx null.}");

        return OAL_FAIL;
    }

    ul_dev_idx_temp = (oal_uint)(uintptr_t)oal_queue_dequeue(&(g_pst_mac_res->st_dev_res.st_queue));

    /* 0Ϊ��Чֵ */
    if (ul_dev_idx_temp == 0) {
        OAL_IO_PRINT("mac_res_alloc_dmac_dev_etc: 0 == ul_dev_idx_temp");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_dmac_dev_etc::ul_dev_idx_temp=0.}");

        return OAL_FAIL;
    }

    *puc_dev_idx = (oal_uint8)(ul_dev_idx_temp - 1);

    (g_pst_mac_res->st_dev_res.auc_user_cnt[ul_dev_idx_temp - 1])++;

    return OAL_SUCC;
}

oal_uint32 mac_res_free_dev_etc(oal_uint32 ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        MAC_ERR_LOG(0, "mac_res_free_dev_etc: ul_dev_idx >= MAC_RES_MAX_DEV_NUM");
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_free_dev_etc::invalid ul_dev_idx[%d].}", ul_dev_idx);

        return OAL_FAIL;
    }

    if (g_pst_mac_res->st_dev_res.auc_user_cnt[ul_dev_idx] == 0) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_res_free_dev_etc::cnt==0! idx:%d", ul_dev_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_pst_mac_res->st_dev_res.auc_user_cnt[ul_dev_idx])--;

    if (g_pst_mac_res->st_dev_res.auc_user_cnt[ul_dev_idx] != 0) {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue(&(g_pst_mac_res->st_dev_res.st_queue), (oal_void *)(uintptr_t)((oal_uint) ul_dev_idx + 1));

    return OAL_SUCC;
}


mac_chip_stru *mac_res_get_mac_chip(oal_uint32 ul_chip_idx)
{
    if (OAL_UNLIKELY(ul_chip_idx >= WLAN_CHIP_MAX_NUM_PER_BOARD)) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_get_mac_chip::invalid ul_chip_idx[%d].}", ul_chip_idx);
        return OAL_PTR_NULL;
    }

    return &(g_pst_mac_board->ast_chip[ul_chip_idx]);
}


oal_uint16 mac_chip_get_max_asoc_user(oal_uint8 uc_chip_id)
{
    /* ������������û��� */
    return WLAN_ASSOC_USER_MAX_NUM;
}

oal_uint8 mac_chip_get_max_active_user(oal_void)
{
    /* ��������active����û��� */
    return g_uc_max_active_user;
}
oal_uint16 mac_board_get_max_user(oal_void)
{
    /* ���嵥�����鲥�û����������嵥�����鲥�û���ֱ��ȡ�ú�ֵ���� */
    return MAC_RES_MAX_USER_LIMIT;
}

/*lint -e19*/
oal_module_symbol(g_pst_mac_res);
oal_module_symbol(g_us_max_asoc_user_etc);
oal_module_symbol(g_uc_max_active_user);
oal_module_symbol(mac_chip_get_max_asoc_user);
oal_module_symbol(mac_chip_get_max_active_user);
oal_module_symbol(mac_chip_get_max_multi_user);
oal_module_symbol(mac_board_get_max_user);
oal_module_symbol(mac_res_free_dev_etc);
oal_module_symbol(mac_res_alloc_hmac_dev_etc);
oal_module_symbol(mac_res_get_dev_etc);
oal_module_symbol(mac_res_get_mac_chip);
oal_module_symbol(mac_res_free_mac_user_etc);
oal_module_symbol(mac_res_free_mac_vap_etc); /*lint +e19*/

