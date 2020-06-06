

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TWT

/* 1 ͷ�ļ����� */
#include "oal_util.h"
#include "hmac_ext_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_p2p.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "securec.h"
#include "securectype.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_C

/* 2 ��̬�������� */
/* 3 ȫ�ֱ������� */
/* 4 ����ʵ�� */
#ifdef _PRE_WLAN_FEATURE_TWT

oal_uint32 hmac_twt_auto_teardown_session(mac_device_stru *pst_dev)
{
    mac_chip_stru *pst_chip = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap_temp;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_num;
    oal_uint32 ul_ret;

    pst_chip = hmac_res_get_mac_chip(pst_dev->uc_chip_id);
    if (pst_chip == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
            "hmac_config_add_vap_etc::hmac_res_get_mac_chip id[%d] NULL",
            pst_dev->uc_chip_id);
        return OAL_FAIL;
    }

    uc_vap_num = hmac_get_chip_vap_num(pst_chip);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_config_add_vap_etc::check vap num for twt,vap num=[%d]}", uc_vap_num);

    /* ��DBDC���������������3�������ϵ�vap��˵����wlan0/p2p���棬
       ��ʱ����Ѿ�����twt�Ự������Ҫɾ��twt�Ự */
    if ((pst_dev->en_dbdc_running == OAL_FALSE) && (uc_vap_num >= 3)) {
        /* ���ҵ�wlan vap */
        ul_ret = mac_device_find_up_sta_wlan_etc(pst_dev, &pst_mac_vap_temp);
        if ((ul_ret == OAL_SUCC) && (pst_mac_vap_temp != OAL_PTR_NULL)) {
            pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap_temp->uc_vap_id);
            if (pst_hmac_vap == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_config_del_vap_etc::mac_res_get_hmac_vap failed.}");
                return OAL_FAIL;
            }

            OAM_WARNING_LOG1(0, OAM_SF_ANY,
                "{hmac_config_add_vap_etc::twt_session_enable=[%d]}",
                pst_hmac_vap->st_twt_cfg.uc_twt_session_enable);

            /* �����vap��twt�Ự�Ѿ���������ɾ��twt�Ự */
            if (pst_hmac_vap->st_twt_cfg.uc_twt_session_enable == 1) {
                hmac_config_twt_teardown_req_auto(&(pst_hmac_vap->st_vap_base_info));
            }
        }
    }

    return OAL_SUCC;
}



oal_uint32 hmac_twt_auto_setup_session(mac_device_stru *pst_device)
{
    mac_chip_stru *pst_chip = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap_temp;
    oal_uint8 uc_vap_num;
    oal_uint32 ul_ret;
    mac_he_hdl_stru st_he_hdl;

    pst_chip = hmac_res_get_mac_chip(pst_device->uc_chip_id);
    if (pst_chip == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
            "hmac_twt_auto_setup_session::hmac_res_get_mac_chip id[%d] NULL",
            pst_device->uc_chip_id);
        return OAL_FAIL;
    }

    uc_vap_num = hmac_get_chip_vap_num(pst_chip);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_twt_auto_setup_session::check vap num for twt,vap num=[%d]}", uc_vap_num);

    /* ֻ��1��vap��ʱ�����ap֧��twt��staû�н����Ự������twt�Ự */
    if ((pst_device->en_dbdc_running == OAL_FALSE) && (uc_vap_num == 2)) {
        ul_ret = mac_device_find_up_sta_wlan_etc(pst_device, &pst_mac_vap_temp);
        if ((ul_ret == OAL_SUCC) && (pst_mac_vap_temp != OAL_PTR_NULL)) {
            /* ���½���twt�Ự */
            pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap_temp->uc_vap_id);
            if (pst_hmac_vap == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_twt_auto_setup_session::mac_res_get_hmac_vap failed.}");
                return OAL_FAIL;
            }

            /* ���û� */
            pst_hmac_user = mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (pst_hmac_user == OAL_PTR_NULL) {
                /* ��ap��һ�����ϲ��Ѿ�ɾ���˵Ļ����������� */
                OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                                 "{hmac_twt_auto_setup_session::pst_hmac_user[%d] is null.}",
                                 pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* �жϸ�AP�Ƿ�֧��twt�������fast psģʽ���ȼ��ap�Ƿ�֧��twt��
             ���֧��twt������twt�Ự��Ȼ��ֱ�ӷ��أ��������fast psģʽ */
            mac_user_get_he_hdl(&(pst_hmac_user->st_user_base_info), &st_he_hdl);
            if (st_he_hdl.st_he_cap_ie.st_he_mac_cap.bit_twt_responder_support == OAL_TRUE) {
                if (pst_hmac_vap->st_twt_cfg.uc_twt_session_enable == 0) {
                    hmac_config_twt_setup_req_auto(&(pst_hmac_vap->st_vap_base_info));
                }
            }
        }
    }

    return OAL_SUCC;
}
#endif
#endif /* _PRE_WLAN_FEATURE_TWT */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


