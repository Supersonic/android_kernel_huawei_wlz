

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 ͷ�ļ����� */
#include "wlan_spec.h"
#include "wlan_types.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "dmac_ext_if.h"
#include "hmac_rx_filter.h"
#include "hmac_device.h"
#include "hmac_resource.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_FILTER_C

/* 2 ȫ�ֱ������� */
/* 3 ����ʵ�� */

oal_bool_enum_uint8 hmac_find_is_sta_up_etc(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_find_is_sta_up_etc::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_bool_enum_uint8 hmac_find_is_ap_up_etc(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_find_is_ap_up_etc::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_uint32 hmac_calc_up_ap_num_etc(mac_device_stru *pst_mac_device)
{
    mac_vap_stru *pst_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_idx;
    oal_uint8 ul_up_ap_num = 0;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == OAL_PTR_NULL) {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{hmac_calc_up_ap_num_etc::pst_mac_vap null,vap_idx=%d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((pst_vap->en_vap_state != MAC_VAP_STATE_INIT) && (pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            ul_up_ap_num++;
        } else if ((pst_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (pst_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            ul_up_ap_num++;
        }
    }

    return ul_up_ap_num;
}


oal_uint32 hmac_find_up_vap_etc(mac_device_stru *pst_mac_device, mac_vap_stru **ppst_mac_vap)
{
    oal_uint32 ul_ret;
    mac_vap_stru *pst_vap_up = OAL_PTR_NULL; /* ����UP״̬��VAP */

    /* find up VAP */
    ul_ret = mac_device_find_up_vap_etc(pst_mac_device, &pst_vap_up);
    if ((ul_ret == OAL_SUCC) && (pst_vap_up != OAL_PTR_NULL)) {
        *ppst_mac_vap = pst_vap_up;
        /* find up AP */
        ul_ret = mac_device_find_up_ap_etc(pst_mac_device, &pst_vap_up);
        if ((ul_ret == OAL_SUCC) && (pst_vap_up != OAL_PTR_NULL)) {
            *ppst_mac_vap = pst_vap_up;
        }

        return OAL_SUCC;
    } else {
        *ppst_mac_vap = OAL_PTR_NULL;
        return OAL_FAIL;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

