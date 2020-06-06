

/* ͷ�ļ����� */
#include "oam_ext_if.h"
#include "oam_linux_netlink.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_LINUX_NETLINK_C

/* ȫ�ֱ������� */
oam_netlink_stru netlink_etc;

oam_netlink_proto_ops netlink_ops_etc;

/* �����ϱ�app�Ľṹ�� */
typedef struct {
    oal_uint32 ul_daq_addr; /* ���������׵�ַ */
    oal_uint32 ul_data_len; /* ���������ܵĳ��� */
    oal_uint32 ul_unit_len; /* ��Ԫ���ݵ���󳤶�:������(daq_unit_head)ͷ���� */
} oam_data_acq_info_stru;

/* ���ɵ�Ԫͷ�ṹ�� */
typedef struct {
    oal_uint8 en_send_type; /* ���ɵ�Ԫ�������к� */
    oal_uint8 uc_resv[3];
    oal_uint32 ul_msg_sn;   /* ���ɵ�Ԫ�������к� */
    oal_uint32 ul_data_len; /* ��ǰ��Ԫ���� */
} oam_data_acq_data_head_stru;

/*
 * �� �� ��  : oam_netlink_ops_register_etc
 * ��������  : WALģ��������ģ���ṩ��ע��netlink��Ϣ������(���շ���)
 */
oal_void oam_netlink_ops_register_etc(oam_nl_cmd_enum_uint8 en_type,
    oal_uint32 (*p_func)(oal_uint8 *puc_data, oal_uint32 ul_len))
{
    if (OAL_UNLIKELY(p_func == OAL_PTR_NULL)) {
        OAL_IO_PRINT("oam_netlink_ops_register_etc, p_func is null ptr.");
        return;
    }

    switch (en_type) {
        case OAM_NL_CMD_SDT:
            netlink_ops_etc.p_oam_sdt_func = p_func;
            break;

        case OAM_NL_CMD_HUT:
            netlink_ops_etc.p_oam_hut_func = p_func;
            break;

        case OAM_NL_CMD_ALG:
            netlink_ops_etc.p_oam_alg_func = p_func;
            break;

        case OAM_NL_CMD_DAQ:
            netlink_ops_etc.p_oam_daq_func = p_func;
            break;

        case OAM_NL_CMD_REG:
            netlink_ops_etc.p_oam_reg_func = p_func;
            break;

        case OAM_NL_CMD_ACS:
            netlink_ops_etc.p_oam_acs_func = p_func;
            break;

        case OAM_NL_CMD_PSTA:
            netlink_ops_etc.p_oam_psta_func = p_func;
            break;

        default:
            OAL_IO_PRINT("oam_netlink_ops_register_etc, err type = %d.", en_type);
            break;
    }
}

/*
 * �� �� ��  : oam_netlink_ops_unregister_etc
 * ��������  : OAMģ��������ģ���ṩ��ж��netlink��Ϣ������(���շ���)
 */
oal_void oam_netlink_ops_unregister_etc(oam_nl_cmd_enum_uint8 en_type)
{
    switch (en_type) {
        case OAM_NL_CMD_SDT:
            netlink_ops_etc.p_oam_sdt_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_HUT:
            netlink_ops_etc.p_oam_hut_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_ALG:
            netlink_ops_etc.p_oam_alg_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_DAQ:
            netlink_ops_etc.p_oam_daq_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_REG:
            netlink_ops_etc.p_oam_reg_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_ACS:
            netlink_ops_etc.p_oam_acs_func = OAL_PTR_NULL;
            break;

        case OAM_NL_CMD_PSTA:
            netlink_ops_etc.p_oam_psta_func = OAL_PTR_NULL;
            break;

        default:
            OAL_IO_PRINT("oam_netlink_ops_unregister_etc::err type = %d.", en_type);
            break;
    }
}

/*
 * �� �� ��  : oam_netlink_kernel_recv_etc
 * ��������  : netlink��Ϣ���պ���(����: host app -> �ں�)
 */
oal_void oam_netlink_kernel_recv_etc(oal_netbuf_stru *pst_buf)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_nlmsghdr_stru *pst_nlmsghdr = NULL;

    if (pst_buf == OAL_PTR_NULL) {
        OAL_IO_PRINT("oam_netlink_kernel_recv_etc, pst_buf is null.");
        return;
    }

    pst_netbuf = pst_buf;

    while (OAL_NETBUF_LEN(pst_netbuf) >= OAL_NLMSG_SPACE(0)) {
        pst_nlmsghdr = oal_nlmsg_hdr(pst_netbuf);

        netlink_etc.ul_pid = pst_nlmsghdr->nlmsg_pid;

        switch (pst_nlmsghdr->nlmsg_type) {
            case OAM_NL_CMD_SDT:
                if (netlink_ops_etc.p_oam_sdt_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_sdt_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                   OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;

            case OAM_NL_CMD_HUT:
                if (netlink_ops_etc.p_oam_hut_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_hut_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                   OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;

            case OAM_NL_CMD_ALG:
                if (netlink_ops_etc.p_oam_alg_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_alg_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                   OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_DAQ:
                if (netlink_ops_etc.p_oam_daq_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_daq_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                   OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_REG:
                if (netlink_ops_etc.p_oam_reg_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_reg_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                   OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_ACS:
                if (netlink_ops_etc.p_oam_acs_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_acs_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                   OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            case OAM_NL_CMD_PSTA:
                if (netlink_ops_etc.p_oam_psta_func != OAL_PTR_NULL) {
                    netlink_ops_etc.p_oam_psta_func(OAL_NLMSG_DATA(pst_nlmsghdr),
                                                    OAL_NLMSG_PAYLOAD(pst_nlmsghdr, 0));
                }
                break;
            default:
                break;
        }

        oal_netbuf_pull(pst_netbuf, OAL_NLMSG_ALIGN(pst_nlmsghdr->nlmsg_len));
    }
}

/*
 * �� �� ��  : oam_netlink_kernel_send_etc
 * ��������  : netlink��Ϣ���ͺ���(����: �ں� -> host app)
 * �������  : puc_data   : ��������
 *             ul_data_len: ���ݳ���
 *             en_type    : netlink msg����
 * �������  : �ɹ�: ���͵��ֽ���(netlinkͷ + payload + padding)
 */
oal_int32 oam_netlink_kernel_send_etc(oal_uint8 *puc_data, oal_uint32 ul_data_len, oam_nl_cmd_enum_uint8 en_type)
{
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)

    return 0;
#else

#if (_PRE_TARGET_PRODUCT_TYPE_1102COMMON == _PRE_CONFIG_TARGET_PRODUCT)
    return 0;
#else
    oal_netbuf_stru *pst_netbuf;
    oal_nlmsghdr_stru *pst_nlmsghdr;
    oal_uint32 ul_size;
    oal_int32 l_ret;

    if (OAL_UNLIKELY(puc_data == NULL)) {
        OAL_WARN_ON(1);
        return -1;
    }

    // ��APPδע�ᣬ��ֵΪ0����ط�������
    if (!netlink_etc.ul_pid) {
        return -1;
    }

    ul_size = OAL_NLMSG_SPACE(ul_data_len);
    pst_netbuf = oal_netbuf_alloc(ul_size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (pst_netbuf == OAL_PTR_NULL) {
        return -1;
    }

    /* ��ʼ��netlink��Ϣ�ײ� */
    pst_nlmsghdr = oal_nlmsg_put(pst_netbuf, 0, 0, (oal_int32)en_type, (oal_int32)ul_data_len, 0);

    /* ���ÿ����ֶ� */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    OAL_NETLINK_CB(pst_netbuf).portid = 0;
#else
    OAL_NETLINK_CB(pst_netbuf).pid = 0;
#endif
    OAL_NETLINK_CB(pst_netbuf).dst_group = 0;

    /* ��������� */
    memcpy_s(OAL_NLMSG_DATA(pst_nlmsghdr), ul_data_len, puc_data, ul_data_len);

    /* �������� */
    l_ret = oal_netlink_unicast(netlink_etc.pst_nlsk, pst_netbuf, netlink_etc.ul_pid, OAL_MSG_DONTWAIT);

    return l_ret;

#endif /* _PRE_TARGET_PRODUCT_TYPE_1102COMMON == _PRE_CONFIG_TARGET_PRODUCT */
#endif /* _PRE_OS_VERSION_RAW == _PRE_OS_VERSION  */
}

/*
 * �� �� ��  : oam_netlink_kernel_send_ex_etc
 * ��������  : netlink��Ϣ���ͺ���(����: �ں� -> host app)
 * �������  : puc_data_1st: ��������1
 *             puc_data_2nd: ��������2
 *             ul_len_1st  : ���ݳ���1
 *             ul_len_2nd  : ���ݳ���2
 *             en_type     : netlink msg����
 * �������  : �ɹ�: ���͵��ֽ���(netlinkͷ + payload + padding)
 */
oal_int32 oam_netlink_kernel_send_ex_etc(oal_uint8 *puc_data_1st, oal_uint8 *puc_data_2nd,
                                         oal_uint32 ul_len_1st, oal_uint32 ul_len_2nd,
                                         oam_nl_cmd_enum_uint8 en_type)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    return 0;
#else

    oal_netbuf_stru *pst_netbuf = NULL;
    oal_nlmsghdr_stru *pst_nlmsghdr = NULL;
    oal_uint32 ul_size;
    oal_int32 l_ret;

    if (OAL_UNLIKELY((puc_data_1st == NULL) || (puc_data_2nd == NULL))) {
        OAL_WARN_ON(1);
        return -1;
    }

    ul_size = OAL_NLMSG_SPACE(ul_len_1st + ul_len_2nd);
    pst_netbuf = oal_netbuf_alloc(ul_size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (pst_netbuf == OAL_PTR_NULL) {
        return -1;
    }

    /* ��ʼ��netlink��Ϣ�ײ� */
    pst_nlmsghdr = oal_nlmsg_put(pst_netbuf, 0, 0, (oal_int32)en_type, (oal_int32)(ul_len_1st + ul_len_2nd), 0);

    /* ���ÿ����ֶ� */
    OAL_NETLINK_CB(pst_netbuf).pid = 0;
    OAL_NETLINK_CB(pst_netbuf).dst_group = 0;

    /* ��������� */
    if (memcpy_s(OAL_NLMSG_DATA(pst_nlmsghdr), (oal_uint32)(ul_len_1st + ul_len_2nd),
                 puc_data_1st, ul_len_1st) != EOK) {
        oal_netbuf_free(pst_netbuf);
        OAL_IO_PRINT ("memcpy_s error, destlen=%u, srclen=%u\n ", (oal_uint32)(ul_len_1st + ul_len_2nd), ul_len_1st);
        return -OAL_EFAIL;
    }

    memcpy_s((oal_uint8 *)OAL_NLMSG_DATA(pst_nlmsghdr) + ul_len_1st, ul_len_2nd, puc_data_2nd, ul_len_2nd);

    /* �������� */
    l_ret = oal_netlink_unicast(netlink_etc.pst_nlsk, pst_netbuf, netlink_etc.ul_pid, OAL_MSG_DONTWAIT);

    return l_ret;
#endif
}

oal_uint32 oam_netlink_kernel_create_etc(oal_void)
{
    netlink_etc.pst_nlsk = oal_netlink_kernel_create(&OAL_INIT_NET, OAM_NETLINK_ID, 0,
                                                     oam_netlink_kernel_recv_etc, OAL_PTR_NULL,
                                                     OAL_THIS_MODULE);
    if (netlink_etc.pst_nlsk == OAL_PTR_NULL) {
        OAL_IO_PRINT("oam_netlink_kernel_create_etc, can not create netlink.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_IO_PRINT("netlink create succ.");

    return OAL_SUCC;
}

oal_void oam_netlink_kernel_release_etc(oal_void)
{
    oal_netlink_kernel_release(netlink_etc.pst_nlsk);

    netlink_etc.ul_pid = 0;

    OAL_IO_PRINT("netlink release succ.");
}

/*lint -e578*/ /*lint -e19*/
oal_module_symbol(netlink_ops_etc);
oal_module_symbol(oam_netlink_ops_register_etc);
oal_module_symbol(oam_netlink_ops_unregister_etc);
oal_module_symbol(oam_netlink_kernel_send_etc);
oal_module_symbol(oam_netlink_kernel_send_ex_etc);
