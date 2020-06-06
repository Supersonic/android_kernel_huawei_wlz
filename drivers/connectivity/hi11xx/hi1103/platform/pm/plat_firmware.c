

/* ͷ�ļ����� */
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include "plat_debug.h"
#include "plat_firmware.h"
#include "plat_sdio.h"
#include "plat_uart.h"
#include "plat_cali.h"
#include "plat_pm.h"
#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"

#include "hw_bfg_ps.h"
#include "plat_efuse.h"
#include "bfgx_exception_rst.h"

#include "oal_util.h"
#include "securec.h"

/* ȫ�ֱ������� */
/* hi1102 cfg�ļ�·�� */
uint8 *cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_PATH,
    WIFI_CFG_PATH,
    BFGX_CFG_PATH,
    RAM_CHECK_CFG_PATH,
};

/* hi1103 mpw2 cfg�ļ�·�� */
uint8 *mpw2_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1103_MPW2_PATH,
    WIFI_CFG_HI1103_MPW2_PATH,
    BFGX_CFG_HI1103_MPW2_PATH,
    RAM_CHECK_CFG_HI1103_MPW2_PATH,
};

/* hi1103 pilot cfg�ļ�·�� */
uint8 *pilot_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1103_PILOT_PATH,
    WIFI_CFG_HI1103_PILOT_PATH,
    BFGX_CFG_HI1103_PILOT_PATH,
    RAM_CHECK_CFG_HI1103_PILOT_PATH,
};

/* hi1105 fpga cfg�ļ�·�� */
uint8 *hi1105_fpga_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1105_FPGA_PATH,
    WIFI_CFG_HI1105_FPGA_PATH,
    BFGX_CFG_HI1105_FPGA_PATH,
    RAM_CHECK_CFG_HI1105_FPGA_PATH,
};

/* hi1105 asic cfg�ļ�·�� */
uint8 *hi1105_asic_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_HI1105_ASIC_PATH,
    WIFI_CFG_HI1105_ASIC_PATH,
    BFGX_CFG_HI1105_ASIC_PATH,
    RAM_CHECK_CFG_HI1105_ASIC_PATH,
};

uint32 asic_type = HI1103_ASIC_PILOT;

uint8 **cfg_path_etc = pilot_cfg_patch_in_vendor;

/* �洢cfg�ļ���Ϣ������cfg�ļ�ʱ��ֵ�����ص�ʱ��ʹ�øñ��� */
FIRMWARE_GLOBALS_STRUCT cfg_info_etc;

/* ����firmware file���ݵ�buffer���Ƚ��ļ��������buffer�У�Ȼ��������device buffer���� */
uint8 *firmware_down_buf_etc = NULL;

/* DataBuf�ĳ��� */
static uint32 firmware_down_buf_len_etc = 0;

struct st_wifi_dump_mem_info nfc_buffer_data_etc = { 0x30000000 + 0x000f9d00,
                                                     OMLNFCDATABUFFLEN, "nfc_buffer_data_etc" };
uint8 *NfcLog_etc = NULL;

/*
 * �� �� ��  : set_hi1103_asic_type
 * ��������  : ����hi1103 asic����(MPW2/PILOT)
 * �������  : 0-MPW2��1-PILOT
 */
void set_hi1103_asic_type(uint32 ul_asic_type)
{
    asic_type = ul_asic_type;
}

/*
 * �� �� ��  : get_hi1103_asic_type
 * ��������  : ��ȡhi1103 asic����(MPW2/PILOT)
 * �� �� ֵ  : 0-MPW2��1-PILOT
 */
uint32 get_hi1103_asic_type(void)
{
    return asic_type;
}

/*
 * �� �� ��  : read_msg_etc
 * ��������  : host����device��������Ϣ
 * �������  : data: ������Ϣ��buffer
 *             len : ����buffer�ĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ����򷵻�ʵ�ʽ��յĳ���
 */
int32 read_msg_etc(uint8 *data, int32 len)
{
    int32 l_len;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();

    if (unlikely((data == NULL))) {
        PS_PRINT_ERR("data is NULL\n ");
        return -EFAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        PS_PRINT_ERR("pst_bus is NULL\n ");
        return -EFAIL;
    }

    l_len = hcc_bus_patch_read(pst_bus, data, len, READ_MEG_TIMEOUT);
    PS_PRINT_DBG("Receive l_len=[%d] \n", l_len);

    return l_len;
}

int32 read_msg_timeout_etc(uint8 *data, int32 len, uint32 timeout)
{
    int32 l_len;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();

    if (unlikely((data == NULL))) {
        PS_PRINT_ERR("data is NULL\n ");
        return -EFAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        PS_PRINT_ERR("pst_bus is NULL\n ");
        return -EFAIL;
    }

    l_len = hcc_bus_patch_read(pst_bus, data, len, timeout);
    PS_PRINT_DBG("Receive l_len=[%d], data = [%s]\n", l_len, data);

    return l_len;
}

/*
 * �� �� ��  : send_msg_etc
 * ��������  : host��device������Ϣ
 * �������  : data: ����buffer
 *             len : �������ݵĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ����򷵻�ʵ�ʷ��͵ĳ���
 */
int32 send_msg_etc(uint8 *data, int32 len)
{
    int32 l_ret;
    hcc_bus *pst_bus = hcc_get_current_110x_bus();

    if (unlikely((pst_bus == NULL))) {
        PS_PRINT_ERR("pst_bus is NULL\n ");
        return -EFAIL;
    }

    PS_PRINT_DBG("len = %d\n", len);
#ifdef HW_DEBUG
    const uint32 ul_max_print_len = 128;
    print_hex_dump_bytes("send_msg_etc :", DUMP_PREFIX_ADDRESS, data,
                         (len < ul_max_print_len ? len : ul_max_print_len));
#endif
    l_ret = hcc_bus_patch_write(pst_bus, data, len);

    return l_ret;
}

/*
 * �� �� ��  : recv_expect_result_etc
 * ��������  : ����host����device��ȷ���ص�����
 * �������  : expect: ����device��ȷ���ص�����
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 recv_expect_result_etc(const uint8 *expect)
{
    uint8 auc_buf[RECV_BUF_LEN];
    int32 l_len;
    int32 i;

    if (!OS_STR_LEN(expect)) {
        PS_PRINT_DBG("not wait device to respond!\n");
        return SUCC;
    }

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_len = read_msg_etc(auc_buf, RECV_BUF_LEN);
        if (l_len < 0) {
            PS_PRINT_ERR("recv result fail\n");
            continue;
        }

        if (!OS_MEM_CMP(auc_buf, expect, OS_STR_LEN(expect))) {
            PS_PRINT_DBG(" send SUCC, expect [%s] ok\n", expect);
            return SUCC;
        } else {
            PS_PRINT_WARNING(" error result[%s], expect [%s], read result again\n", auc_buf, expect);
        }
    }

    return -EFAIL;
}

int32 recv_expect_result_timeout_etc(const uint8 *expect, uint32 timeout)
{
    uint8 auc_buf[RECV_BUF_LEN];
    int32 l_len;

    if (!OS_STR_LEN(expect)) {
        PS_PRINT_DBG("not wait device to respond!\n");
        return SUCC;
    }

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    l_len = read_msg_timeout_etc(auc_buf, RECV_BUF_LEN, timeout);
    if (l_len < 0) {
        PS_PRINT_ERR("recv result fail\n");
        return -EFAIL;
    }

    if (!OS_MEM_CMP(auc_buf, expect, OS_STR_LEN(expect))) {
        PS_PRINT_DBG(" send SUCC, expect [%s] ok\n", expect);
        return SUCC;
    } else {
        PS_PRINT_WARNING(" error result[%s], expect [%s], read result again\n", auc_buf, expect);
    }

    return -EFAIL;
}

/*
 * �� �� ��  : msg_send_and_recv_except_etc
 * ��������  : host��device������Ϣ���ȴ�device������Ϣ
 * �������  : data  : ����buffer
 *             len   : �������ݵĳ���
 *             expect: ����device�ظ�������
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 msg_send_and_recv_except_etc(uint8 *data, int32 len, const uint8 *expect)
{
    int32 i;
    int32 l_ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = send_msg_etc(data, len);
        if (l_ret < 0) {
            continue;
        }

        l_ret = recv_expect_result_etc(expect);
        if (l_ret == 0) {
            return SUCC;
        }
    }

    return -EFAIL;
}

/*
 * �� �� ��  : malloc_cmd_buf_etc
 * ��������  : ����cfg�ļ����������Ľ��������g_st_cfg_infoȫ�ֱ�����
 * �������  : puc_cfg_info_buf: ������cfg�ļ����ݵ�buffer
 *             ul_index        : ��������������������ֵ
 * �� �� ֵ  : NULL��ʾ�����ڴ�ʧ�ܣ����򷵻�ָ�򱣴����cfg�ļ�����������׵�ַ
 */
void *malloc_cmd_buf_etc(uint8 *puc_cfg_info_buf, uint32 ul_index)
{
    int32 l_len;
    uint8 *flag = NULL;
    uint8 *p_buf = NULL;

    if (puc_cfg_info_buf == NULL) {
        PS_PRINT_ERR("malloc_cmd_buf_etc: buf is NULL!\n");
        return NULL;
    }

    /* ͳ��������� */
    flag = puc_cfg_info_buf;
    cfg_info_etc.al_count[ul_index] = 0;
    while (flag != NULL) {
        /* һ����ȷ�������н�����Ϊ ; */
        flag = OS_STR_CHR(flag, CMD_LINE_SIGN);
        if (flag == NULL) {
            break;
        }
        cfg_info_etc.al_count[ul_index]++;
        flag++;
    }
    PS_PRINT_DBG("cfg file cmd count: al_count[%d] = %d\n", ul_index, cfg_info_etc.al_count[ul_index]);

    /* ����洢����ռ� */
    l_len = ((cfg_info_etc.al_count[ul_index]) + CFG_INFO_RESERVE_LEN) * sizeof(struct cmd_type_st);
    p_buf = OS_KMALLOC_GFP(l_len);
    if (p_buf == NULL) {
        PS_PRINT_ERR("kmalloc cmd_type_st fail\n");
        return NULL;
    }
    memset_s((void *)p_buf, l_len, 0, l_len);

    return p_buf;
}

/*
 * �� �� ��  : delete_space_etc
 * ��������  : ɾ���ַ������߶���Ŀո�
 * �������  : string: ԭʼ�ַ���
 *            len   : �ַ����ĳ���
 * �� �� ֵ  : ���󷵻�NULL�����򷵻�ɾ�����߿ո��Ժ��ַ������׵�ַ
 */
uint8 *delete_space_etc(uint8 *string, int32 *len)
{
    int i;

    if ((string == NULL) || (len == NULL)) {
        return NULL;
    }

    /* ɾ��β���Ŀո� */
    for (i = *len - 1; i >= 0; i--) {
        if (string[i] != COMPART_KEYWORD) {
            break;
        }
        string[i] = '\0';
    }
    /* ���� */
    if (i < 0) {
        PS_PRINT_ERR(" string is Space bar\n");
        return NULL;
    }
    /* ��for����м�ȥ1���������1 */
    *len = i + 1;

    /* ɾ��ͷ���Ŀո� */
    for (i = 0; i < *len; i++) {
        if (string[i] != COMPART_KEYWORD) {
            /* ��ȥ�ո�ĸ��� */
            *len = *len - i;
            return &string[i];
        }
    }

    return NULL;
}

/*
 * �� �� ��  : string_to_num_etc
 * ��������  : ���ַ���ת����������
 * �������  : string:������ַ���
 * �������  : number:�ַ���ת���Ժ��������
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 string_to_num_etc(uint8 *string, int32 *number)
{
    int32 i;
    int32 l_num;

    if (string == NULL) {
        PS_PRINT_ERR("string is NULL!\n");
        return -EFAIL;
    }

    l_num = 0;
    for (i = 0; (string[i] >= '0') && (string[i] <= '9'); i++) {
        l_num = (l_num * 10) + (string[i] - '0'); /* �ַ���ת�����߼���Ҫ */
    }

    *number = l_num;

    return SUCC;
}

/*
 * �� �� ��  : num_to_string_etc
 * ��������  : ��������ת�����ַ���
 * �������  : number:�����������
 * �������  : number:������ת���Ժ���ַ���
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 num_to_string_etc(uint8 *string, uint32 number)
{
    int32 i = 0;
    int32 j = 0;
    int32 tmp[INT32_STR_LEN];
    uint32 num = number;

    if (string == NULL) {
        PS_PRINT_ERR("string is NULL!\n");
        return -EFAIL;
    }

    do {
        tmp[i] = num % 10;
        num = num / 10; /* ��������ת�ַ����߼���Ҫ */
        i++;
    } while (num != 0);

    do {
        string[j] = tmp[i - 1 - j] + '0';
        j++;
    } while (j != i);

    string[j] = '\0';

    return SUCC;
}

/*
 * �� �� ��  : open_file_to_readm_etc
 * ��������  : ���ļ�������read mem������������
 * �� �� ֵ  : ���ش��ļ���������
 */
OS_KERNEL_FILE_STRU *open_file_to_readm_etc(uint8 *name)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    uint8 *file_name = NULL;
    mm_segment_t fs;

    if (name == NULL) {
        file_name = WIFI_DUMP_PATH "/readm_wifi";
    } else {
        file_name = name;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(file_name, O_RDWR | O_CREAT, 0664);
    set_fs(fs);

    return fp;
}

/*
 * �� �� ��  : recv_device_mem_etc
 * ��������  : ����device�����������ڴ棬���浽ָ�����ļ���
 * �������  : fp : �����ڴ���ļ�ָ��
 *             len: ��Ҫ������ڴ�ĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ����򷵻�ʵ�ʱ�����ڴ�ĳ���
 */
int32 recv_device_mem_etc(OS_KERNEL_FILE_STRU *fp, uint8 *pucDataBuf, int32 len)
{
    int32 l_ret = -EFAIL;
    mm_segment_t fs;
    uint8 retry = 3;
    int32 lenbuf = 0;

    if (OAL_IS_ERR_OR_NULL(fp)) {
        PS_PRINT_ERR("fp is error,fp = 0x%p\n", fp);
        return -EFAIL;
    }

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf is NULL\n");
        return -EFAIL;
    }

    PS_PRINT_DBG("expect recv len is [%d]\n", len);

    fs = get_fs();
    set_fs(KERNEL_DS);
    PS_PRINT_DBG("pos = %d\n", (int)fp->f_pos);
    while (len > lenbuf) {
        l_ret = read_msg_etc(pucDataBuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -EFAIL;
                PS_PRINT_ERR("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        vfs_write(fp, pucDataBuf, len, &fp->f_pos);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
    set_fs(fs);

    return l_ret;
}

/*
 * �� �� ��  : check_version_etc
 * ��������  : ���������device�汾�ţ������device�ϱ��İ汾�ź�host�İ汾���Ƿ�ƥ��
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 check_version_etc(void)
{
    int32 l_ret;
    int32 l_len;
    int32 i;
    uint8 rec_buf[VERSION_LEN];

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);

        l_ret = memcpy_s(rec_buf, sizeof(rec_buf), (uint8 *)VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD));
        if (l_ret != EOK) {
            PS_PRINT_ERR("rec_buf not enough\n");
            return -EFAIL;
        }
        l_len = OS_STR_LEN(VER_CMD_KEYWORD);

        rec_buf[l_len] = COMPART_KEYWORD;
        l_len++;

        l_ret = send_msg_etc(rec_buf, l_len);
        if (l_ret < 0) {
            PS_PRINT_ERR("send version fail![%d]\n", i);
            continue;
        }

        memset_s(cfg_info_etc.auc_DevVersion, VERSION_LEN, 0, VERSION_LEN);
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);
        msleep(1);

        l_ret = read_msg_etc(rec_buf, VERSION_LEN);
        if (l_ret < 0) {
            PS_PRINT_ERR("read version fail![%d]\n", i);
            continue;
        }

        memcpy_s(cfg_info_etc.auc_DevVersion, VERSION_LEN, rec_buf, VERSION_LEN);

        if (!OS_MEM_CMP((int8 *)cfg_info_etc.auc_DevVersion,
                        (int8 *)cfg_info_etc.auc_CfgVersion,
                        OS_STR_LEN(cfg_info_etc.auc_CfgVersion))) {
            PS_PRINT_INFO("Device Version = [%s], CfgVersion = [%s].\n",
                          cfg_info_etc.auc_DevVersion, cfg_info_etc.auc_CfgVersion);
            return SUCC;
        } else {
            PS_PRINT_ERR("ERROR version,Device Version = [%s], CfgVersion = [%s].\n",
                         cfg_info_etc.auc_DevVersion, cfg_info_etc.auc_CfgVersion);
        }
    }

    return -EFAIL;
}

/*
 * �� �� ��  : number_type_cmd_send_etc
 * ��������  : ����number���͵���������͵�device
 * �������  : Key  : ����Ĺؼ���
 *             Value: ����Ĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ������ʾ�ɹ�
 */
int32 number_type_cmd_send_etc(uint8 *Key, const char *Value)
{
    int32 l_ret;
    int32 data_len;
    int32 Value_len;
    int32 i;
    int32 n;
    uint8 auc_num[INT32_STR_LEN];
    uint8 buff_tx[SEND_BUF_LEN];

    Value_len = OS_STR_LEN((int8 *)Value);

    memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    data_len = 0;
    data_len = OS_STR_LEN(Key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), Key, data_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }

    buff_tx[data_len] = COMPART_KEYWORD;
    data_len = data_len + 1;

    for (i = 0, n = 0; (i <= Value_len) && (n < INT32_STR_LEN); i++) {
        if ((Value[i] == ',') || (Value_len == i)) {
            PS_PRINT_DBG("auc_num = %s, i = %d, n = %d\n", auc_num, i, n);
            if (n == 0) {
                continue;
            }
            l_ret = memcpy_s((uint8 *)&buff_tx[data_len], sizeof(buff_tx) - data_len, auc_num, n);
            if (l_ret != EOK) {
                PS_PRINT_ERR("buff_tx not enough\n");
                return -EFAIL;
            }
            data_len = data_len + n;

            buff_tx[data_len] = COMPART_KEYWORD;
            data_len = data_len + 1;

            memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
            n = 0;
        } else if (Value[i] == COMPART_KEYWORD) {
            continue;
        } else {
            auc_num[n] = Value[i];
            n++;
        }
    }

    l_ret = send_msg_etc(buff_tx, data_len);

    return l_ret;
}

/*
 * �� �� ��  : update_device_cali_count_etc
 * ��������  : ʹ��WRITEM�������device��У׼�������״��ϵ�ʱΪȫ0
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 *             recv_expect_result_etc
 */
int32 update_device_cali_count_etc(uint8 *Key, uint8 *Value)
{
    int32 l_ret;
    uint32 len, Value_len;
    uint32 number = 0;
    uint8 *addr = NULL;
    uint8 buff_tx[SEND_BUF_LEN];

    /* �������Value�ַ��������Valueֻ��һ����ַ����ʽΪ"0xXXXXX" */
    /* ����Ժ����ʽΪ"���ݿ��,Ҫд�ĵ�ַ,Ҫд��ֵ"---"4,0xXXXX,value" */
    len = 0;
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    /* buff_tx="" */
    buff_tx[len] = '4';
    len++;
    buff_tx[len] = ',';
    len++;

    /* buff_tx="4," */
    Value_len = OS_STR_LEN(Value);
    addr = delete_space_etc(Value, &Value_len);
    if (addr == NULL) {
        PS_PRINT_ERR("addr is NULL, Value[%s] Value_len[%d]", Value, Value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, Value_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }
    len += Value_len;
    buff_tx[len] = ',';
    len++;

    /* buff_tx="4,0xXXX," */
    l_ret = get_cali_count_etc(&number);
    l_ret += num_to_string_etc(&buff_tx[len], number);

    /* ��ʱbuff_tx="4,0xXXX,value" */
    /* ʹ��WMEM_CMD_KEYWORD������device����У׼���� */
    l_ret += number_type_cmd_send_etc(WMEM_CMD_KEYWORD, buff_tx);
    if (l_ret < 0) {
        PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, buff_tx);
        return l_ret;
    }

    l_ret = recv_expect_result_etc(MSG_FROM_DEV_WRITEM_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("recv expect result fail!\n");
        return l_ret;
    }

    return SUCC;
}

/*
 * �� �� ��  : download_bfgx_cali_data_etc
 * ��������  : ʹ��files�������bfgx��У׼����
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 *             recv_expect_result_etc
 */
int32 download_bfgx_cali_data_etc(uint8 *Key, uint8 *Value)
{
    int32 l_ret;
    uint32 len;
    uint32 Value_len;
    uint8 *addr = NULL;
    uint8 buff_tx[SEND_BUF_LEN];

    /* �������Value�ַ��������Valueֻ��һ����ַ����ʽΪ"0xXXXXX" */
    /* ����Ժ����ʽΪ"FILES �ļ����� Ҫд�ĵ�ַ"---"FILES 1 0xXXXX " */
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    /* buff_tx="" */
    len = OS_STR_LEN(Key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), Key, len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES " */
    buff_tx[len] = '1';
    len++;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES 1 " */
    Value_len = OS_STR_LEN(Value);
    addr = delete_space_etc(Value, &Value_len);
    if (addr == NULL) {
        PS_PRINT_ERR("addr is NULL, Value[%s] Value_len[%d]", Value, Value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, Value_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("buff_tx not enough\n");
        return -EFAIL;
    }
    len += Value_len;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    PS_PRINT_INFO("download bfgx cali data addr:%s\n", addr);

    /* buff_tx="FILES 1 0xXXXX " */
    /* ���͵�ַ */
    l_ret = msg_send_and_recv_except_etc(buff_tx, len, MSG_FROM_DEV_READY_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("SEND [%s] addr error\n", Key);
        return -EFAIL;
    }

    /* ��ȡbfgxУ׼���� */
    l_ret = get_bfgx_cali_data_etc(firmware_down_buf_etc, &len, firmware_down_buf_len_etc);
    if (l_ret < 0) {
        PS_PRINT_ERR("get bfgx cali data failed, len=%d\n", len);
        return -EFAIL;
    }

    /* Wait at least 5 ms */
    oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

    /* ����bfgxУ׼���� */
    l_ret = msg_send_and_recv_except_etc(firmware_down_buf_etc, len, MSG_FROM_DEV_FILES_OK);
    if (l_ret < 0) {
        PS_PRINT_ERR("send bfgx cali data fail\n");
        return -EFAIL;
    }

    return SUCC;
}

/*
 * �� �� ��  : parse_file_cmd_etc
 * ��������  : ����file�������
 * �������  : string   : file����Ĳ���
 *             addr     : ���͵����ݵ�ַ
 *             file_path: �����ļ���·��
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 parse_file_cmd_etc(uint8 *string, unsigned long *addr, int8 **file_path)
{
    uint8 *tmp = NULL;
    int32 count = 0;
    int8 *after = NULL;

    if (string == NULL || addr == NULL || file_path == NULL) {
        PS_PRINT_ERR("param is error!\n");
        return -EFAIL;
    }

    /* ��÷��͵��ļ��ĸ������˴�����Ϊ1��string�ַ����ĸ�ʽ������"1,0xXXXXX,file_path" */
    tmp = string;
    while (*tmp == COMPART_KEYWORD) {
        tmp++;
    }
    string_to_num_etc(tmp, &count);
    if (count != FILE_COUNT_PER_SEND) {
        PS_PRINT_ERR("the count of send file must be 1, count = [%d]\n", count);
        return -EFAIL;
    }

    /* ��tmpָ���ַ������ĸ */
    tmp = OS_STR_CHR(string, ',');
    if (tmp == NULL) {
        PS_PRINT_ERR("param string is err!\n");
        return -EFAIL;
    } else {
        tmp++;
        while (*tmp == COMPART_KEYWORD) {
            tmp++;
        }
    }

    *addr = simple_strtoul(tmp, &after, 16); /* ���ַ���ת����16������ */

    PS_PRINT_DBG("file to send addr:[0x%lx]\n", *addr);

    /* "1,0xXXXX,file_path"
     *         ^
     *       after
     */
    while (*after == COMPART_KEYWORD) {
        after++;
    }
    /* ����','�ַ� */
    after++;
    while (*after == COMPART_KEYWORD) {
        after++;
    }

    PS_PRINT_DBG("after:[%s]\n", after);

    *file_path = after;

    return SUCC;
}

void oal_print_wcpu_reg(oal_uint32 *pst_buf, oal_uint32 ul_size)
{
    oal_int32 i = 0;
    oal_int32 remain = (oal_int32)ul_size; /* per dword */
    if (ul_size) {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "print wcpu registers:");
    }

    for (;;) {
        if (remain >= 4) { /* ����Ҫ��ӡ��32bit�Ĵ����������ڵ���4ʱ��һ���Դ�ӡ4�� */
            OAM_ERROR_LOG4(0, OAM_SF_ANY, "wcpu_reg: %x %x %x %x",
                           *(pst_buf + i + 0), *(pst_buf + i + 1),
                           *(pst_buf + i + 2), *(pst_buf + i + 3));
            i += 4;
            remain -= 4;
        } else if (remain >= 3) { /* ����Ҫ��ӡ��32bit�Ĵ�����������3ʱ��һ���Դ�ӡ3�� */
            OAM_ERROR_LOG3(0, OAM_SF_ANY, "wcpu_reg: %x %x %x",
                           *(pst_buf + i + 0), *(pst_buf + i + 1),
                           *(pst_buf + i + 2));
            i += 3;
            remain -= 3;
        } else if (remain >= 2) { /* ����Ҫ��ӡ��32bit�Ĵ�����������2ʱ��һ���Դ�ӡ2�� */
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "wcpu_reg: %x %x",
                           *(pst_buf + i + 0), *(pst_buf + i + 1));
            i += 2;
            remain -= 2;
        } else if (remain >= 1) { /* ����Ҫ��ӡ��32bit�Ĵ�����������1ʱ��һ���Դ�ӡ1�� */
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "wcpu_reg: %x",
                           *(pst_buf + i + 0));
            i += 1;
            remain -= 1;
        } else {
            break;
        }
    }
}

#define READ_DEVICE_MAX_BUF_SIZE 128
/* read device reg by bootloader */
int32 read_device_reg16(uint32 address, uint16 *value)
{
    int32 ret, buf_len;
    const uint32 ul_read_msg_len = 4;
    const uint32 ul_dump_len = 8;
    uint8 buf_tx[READ_DEVICE_MAX_BUF_SIZE];
    uint8 buf_result[READ_DEVICE_MAX_BUF_SIZE];
    void *addr = (void *)buf_result;

    memset_s(buf_tx, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);
    memset_s(buf_result, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%x%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         address,
                         COMPART_KEYWORD,
                         4,
                         COMPART_KEYWORD); /* ��� READM 0x... 4 ���������4��ʾ���� */
    if (buf_len < 0) {
        PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
        return buf_len;
    }

    PS_PRINT_INFO("%s", buf_tx);

    ret = send_msg_etc(buf_tx, buf_len);
    if (ret < 0) {
        PS_PRINT_ERR("send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = read_msg_etc(buf_result, ul_read_msg_len);
    if (ret > 0) {
        /* �����ض����ڴ�,����С��ֱ��ת�� */
        *value = (uint16)oal_readl(addr);
        oal_print_hex_dump(buf_result, ul_dump_len, HEX_DUMP_GROUP_SIZE, "reg16: ");
        return 0;
    }

    PS_PRINT_ERR("read_device_reg16 failed, ret=%d", ret);

    return -1;
}

/* write device regs by bootloader */
int32 write_device_reg16(uint32 address, uint16 value)
{
    int32 ret, buf_len;
    uint8 buf_tx[READ_DEVICE_MAX_BUF_SIZE];

    memset_s(buf_tx, sizeof(buf_tx), 0, sizeof(buf_tx));

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c2%c0x%x%c%d%c",
                         WMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         COMPART_KEYWORD,
                         address,
                         COMPART_KEYWORD,
                         value,
                         COMPART_KEYWORD);
    if (buf_len < 0) {
        PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
        return buf_len;
    }

    PS_PRINT_INFO("%s", buf_tx);

    ret = send_msg_etc(buf_tx, buf_len);
    if (ret < 0) {
        PS_PRINT_ERR("send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = recv_expect_result_etc(MSG_FROM_DEV_WRITEM_OK);
    if (ret < 0) {
        PS_PRINT_ERR("send msg [%s] recv failed, ret=%d", buf_tx, ret);
        return ret;
    }

    return 0;
}
#ifdef HI110X_HAL_MEMDUMP_ENABLE
int32 recv_device_memdump_etc(uint8 *pucDataBuf, int32 len)
{
    int32 l_ret = -EFAIL;
    uint8 retry = 3;
    int32 lenbuf = 0;

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf is NULL\n");
        return -EFAIL;
    }

    PS_PRINT_DBG("expect recv len is [%d]\n", len);

    while (len > lenbuf) {
        l_ret = read_msg_etc(pucDataBuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -EFAIL;
                PS_PRINT_ERR("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        wifi_memdump_enquenue_etc(pucDataBuf, len);
    }

    return l_ret;
}
int32 sdio_read_device_mem_etc(struct st_wifi_dump_mem_info *pst_mem_dump_info,
                               uint8 *pucDataBuf,
                               uint32 ulDataBufLen)
{
    uint8 buf_tx[SEND_BUF_LEN];
    int32 ret = 0;
    uint32 size = 0;
    uint32 offset;
    uint32 remainder = pst_mem_dump_info->size;

    offset = 0;
    while (remainder > 0) {
        memset_s(buf_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

        size = min(remainder, ulDataBufLen);
        ret = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%lx%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         pst_mem_dump_info->mem_addr + offset,
                         COMPART_KEYWORD,
                         size,
                         COMPART_KEYWORD);
        if (ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            break;
        }
        PS_PRINT_DBG("read mem cmd:[%s]\n", buf_tx);
        send_msg_etc(buf_tx, OS_STR_LEN(buf_tx));

        ret = recv_device_memdump_etc(pucDataBuf, size);
        if (ret < 0) {
            PS_PRINT_ERR("wifi mem dump fail, filename is [%s],ret=%d\n", pst_mem_dump_info->file_name, ret);
            break;
        }

#ifdef CONFIG_PRINTK
        if (offset == 0) {
            oal_int8 *pst_file_name = (pst_mem_dump_info->file_name ?
                                       ((oal_int8 *)pst_mem_dump_info->file_name) : (oal_int8 *)"default: ");
            if (!oal_strcmp("wifi_device_panic_mem", pst_file_name)) {
                if (size > CPU_PANIC_MEMDUMP_SIZE) {
                    oal_print_hex_dump(pucDataBuf, CPU_PANIC_MEMDUMP_SIZE, HEX_DUMP_GROUP_SIZE, pst_file_name);
                    /* print sdt log */
#ifdef CONFIG_MMC
                    /* ��Ĵ��������4�ֽڣ������������Ҫ��4 */
                    oal_print_wcpu_reg ((oal_uint32 *)(pucDataBuf), CPU_PANIC_MEMDUMP_SIZE / 4);
#endif
                }
            }
        }
#endif

        offset += size;

        remainder -= size;
    }

    return ret;
}

/*
 * �� �� ��  : wifi_device_mem_dump
 * ��������  : firmware����ʱ��ȡwifi���ڴ�
 * �������  : pst_mem_dump_info  : ��Ҫ��ȡ���ڴ���Ϣ
 *             count              : ��Ҫ��ȡ���ڴ�����
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 wifi_device_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count)
{
    int32 ret = -EFAIL;
    uint32 i;
    uint8 *pucDataBuf = NULL;
    const uint32 ul_buff_size = 100;
    uint8 buff[ul_buff_size];
    uint32 *pcount = (uint32 *)&buff[0];
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    if (!ps_is_device_log_enable_etc()) {
        return 0;
    }

    /* ��??����??��????3��1|?��,��3�䨮D?????��??����?��Y��������??3��1|?�� */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    if (pst_mem_dump_info == NULL) {
        PS_PRINT_ERR("pst_wifi_dump_info is NULL\n");
        return -EFAIL;
    }

    do {
        PS_PRINT_INFO("try to malloc mem dump buf len is [%d]\n", sdio_transfer_limit);
        pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem  len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    PS_PRINT_INFO("mem dump data buf len is [%d]\n", sdio_transfer_limit);

    wifi_notice_hal_memdump_etc();

    for (i = 0; i < count; i++) {
        *pcount = pst_mem_dump_info[i].size;
        PS_PRINT_INFO("mem dump data size [%d]==> [%d]\n", *pcount, pst_mem_dump_info[i].size);
        wifi_memdump_enquenue_etc(buff, 4); /* ��������sk_buff�Ĵ�С */
        ret = sdio_read_device_mem_etc(&pst_mem_dump_info[i], pucDataBuf, sdio_transfer_limit);
        if (ret < 0) {
            break;
        }
    }
    wifi_memdump_finish_etc();

    OS_MEM_KFREE(pucDataBuf);

    return ret;
}

#else

/*
 * �� �� ��  : sdio_read_device_mem_etc
 * ��������  : ��device����bootloaderʱ��DEVICE��ȡ�ڴ�
 * �� �� ֵ  : С��0��ʾʧ��
 */
int32 sdio_read_device_mem_etc(struct st_wifi_dump_mem_info *pst_mem_dump_info,
                               OS_KERNEL_FILE_STRU *fp,
                               uint8 *pucDataBuf,
                               uint32 ulDataBufLen)
{
    uint8 buf_tx[SEND_BUF_LEN];
    int32 ret = 0;
    uint32 size = 0;
    uint32 offset;
    uint32 remainder = pst_mem_dump_info->size;

    offset = 0;
    while (remainder > 0) {
        memset_s(buf_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

        size = min(remainder, ulDataBufLen);
        ret = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%lx%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         pst_mem_dump_info->mem_addr + offset,
                         COMPART_KEYWORD,
                         size,
                         COMPART_KEYWORD);
        if (ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            break;
        }
        PS_PRINT_DBG("read mem cmd:[%s]\n", buf_tx);
        send_msg_etc(buf_tx, OS_STR_LEN(buf_tx));

        ret = recv_device_mem_etc(fp, pucDataBuf, size);
        if (ret < 0) {
            PS_PRINT_ERR("wifi mem dump fail, filename is [%s],ret=%d\n", pst_mem_dump_info->file_name, ret);
            break;
        }

#ifdef CONFIG_PRINTK
        if (offset == 0) {
            oal_int8 *pst_file_name = (pst_mem_dump_info->file_name ?
                                       ((oal_int8 *)pst_mem_dump_info->file_name) : (oal_int8 *)"default: ");
            if (!oal_strcmp("wifi_device_panic_mem", pst_file_name)) {
                if (size > CPU_PANIC_MEMDUMP_SIZE) {
                    oal_print_hex_dump(pucDataBuf, CPU_PANIC_MEMDUMP_SIZE, HEX_DUMP_GROUP_SIZE, pst_file_name);
                    /* print sdt log */
#ifdef CONFIG_MMC
                    /* ��Ĵ��������4�ֽڣ������������Ҫ��4 */
                    oal_print_wcpu_reg ((oal_uint32 *)(pucDataBuf), CPU_PANIC_MEMDUMP_SIZE / 4);
#endif
                }
            }
        }
#endif

        offset += size;

        remainder -= size;
    }

    return ret;
}

/*
 * �� �� ��  : wifi_device_mem_dump
 * ��������  : firmware����ʱ��ȡwifi���ڴ�
 * �������  : pst_mem_dump_info  : ��Ҫ��ȡ���ڴ���Ϣ
 *             count              : ��Ҫ��ȡ���ڴ�����
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 wifi_device_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    int32 ret = -EFAIL;
    uint32 i;
    const uint32 ul_filename_len = 100;
    char filename[ul_filename_len];

    ktime_t time_start, time_stop;
    oal_uint64 trans_us;
    uint8 *pucDataBuf = NULL;
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    if (!ps_is_device_log_enable_etc()) {
        return 0;
    }

    /* ���ڴ��ȿ��ǳɹ���,ҳ��С������ڴ���������ɹ��� */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    if (pst_mem_dump_info == NULL) {
        PS_PRINT_ERR("pst_wifi_dump_info is NULL\n");
        return -EFAIL;
    }

    do {
        PS_PRINT_INFO("try to malloc mem dump buf len is [%d]\n", sdio_transfer_limit);
        pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem  len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    PS_PRINT_INFO("mem dump data buf len is [%d]\n", sdio_transfer_limit);

    plat_wait_last_rotate_finish_etc();

    for (i = 0; i < count; i++) {
        time_start = ktime_get();
        /* ���ļ���׼������wifi mem dump */
        memset_s(filename, sizeof(filename), 0, sizeof(filename));
        ret = snprintf_s(filename, sizeof(filename), sizeof(filename) - 1, WIFI_DUMP_PATH "/%s_%s.bin",
                         SDIO_STORE_WIFI_MEM, pst_mem_dump_info[i].file_name);
        if (ret < 0) {
            PS_PRINT_ERR("filename format str err\n");
            break;
        }
        PS_PRINT_INFO("readm %s\n", filename);

        fp = open_file_to_readm_etc(filename);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_ERR("create file error,fp = 0x%p, filename is [%s]\n", fp, pst_mem_dump_info[i].file_name);
            break;
        }

        ret = sdio_read_device_mem_etc(&pst_mem_dump_info[i], fp, pucDataBuf, sdio_transfer_limit);
        if (ret < 0) {
            oal_file_close(fp, NULL);
            break;
        }
        oal_file_close(fp, NULL);
        time_stop = ktime_get();
        trans_us = (oal_uint64)ktime_to_us(ktime_sub(time_stop, time_start));
        OAL_IO_PRINT("device get mem %s cost %llu us\n", filename, trans_us);
    }

    /* send cmd to oam_hisi to rotate file */
    plat_send_rotate_cmd_2_app_etc(CMD_READM_WIFI_SDIO);

    OS_MEM_KFREE(pucDataBuf);

    return ret;
}

#endif
int32 sdio_read_mem_etc(uint8 *Key, uint8 *Value, bool is_wifi)
{
    int32 l_ret;
    uint32 size, readlen;
    int32 retry = 3;
    uint8 *flag;
    OS_KERNEL_FILE_STRU *fp = NULL;
    uint8 *pucDataBuf = NULL;
    uint32 sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_110x_handler());

    /* ���ڴ��ȿ��ǳɹ���,ҳ��С������ڴ���������ɹ��� */
    sdio_transfer_limit = OAL_MIN(PAGE_SIZE, sdio_transfer_limit);

    flag = OS_STR_CHR(Value, ',');
    if (flag == NULL) {
        PS_PRINT_ERR("RECV LEN ERROR..\n");
        return -EFAIL;
    }
    flag++;
    PS_PRINT_DBG("recv len [%s]\n", flag);
    while (*flag == COMPART_KEYWORD) {
        flag++;
    }

    string_to_num_etc(flag, &size);

    do {
        PS_PRINT_INFO("try to malloc sdio mem read buf len is [%d]\n", sdio_transfer_limit);
        pucDataBuf = (uint8 *)OS_KMALLOC_GFP(sdio_transfer_limit);
        if (pucDataBuf == NULL) {
            PS_PRINT_WARNING("malloc mem len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((pucDataBuf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf KMALLOC failed\n");
        return -EFAIL;
    }

    fp = open_file_to_readm_etc(is_wifi == true ? WIFI_DUMP_PATH "/readm_wifi" : WIFI_DUMP_PATH "/readm_bfgx");
    if (IS_ERR(fp)) {
        PS_PRINT_ERR("create file error,fp = 0x%p\n", fp);
        OS_MEM_KFREE(pucDataBuf);
        return SUCC;
    }

    l_ret = number_type_cmd_send_etc(Key, Value);
    if (l_ret < 0) {
        PS_PRINT_ERR("send %s,%s fail \n", Key, Value);
        oal_file_close(fp);
        OS_MEM_KFREE(pucDataBuf);
        return l_ret;
    }

    PS_PRINT_DBG("recv len [%d]\n", size);
    while (size > 0) {
        readlen = min(size, sdio_transfer_limit);
        l_ret = recv_device_mem_etc(fp, pucDataBuf, size);
        if (l_ret > 0) {
            size -= l_ret;
        } else {
            PS_PRINT_ERR("read error retry:%d\n", retry);
            --retry;
            if (!retry) {
                PS_PRINT_ERR("retry fail\n");
                break;
            }
        }
    }

    oal_file_close(fp);
    OS_MEM_KFREE(pucDataBuf);

    return l_ret;
}

/*
 * �� �� ��  : exec_file_type_cmd_etc
 * ��������  : ִ��number���͵�����
 * �������  : Key  : ����Ĺؼ���
 *             Value: ����Ĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 exec_number_type_cmd_etc(uint8 *Key, uint8 *Value)
{
    int32 l_ret = -EFAIL;
    int32 l_delay_ms = 0;
    BOARD_INFO *board_info = NULL;

    board_info = get_hi110x_board_info_etc();
    if (board_info == NULL) {
        PS_PRINT_ERR("board_info is null!\n");
        return -EFAIL;
    }

    if (!OS_MEM_CMP(Key, VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD))) {
        l_ret = check_version_etc();
        if (l_ret < 0) {
            PS_PRINT_ERR("check version FAIL [%d]\n", l_ret);
            return -EFAIL;
        }
    }

    if (!OS_STR_CMP((int8 *)Key, WMEM_CMD_KEYWORD)) {
        if (OS_STR_STR((int8 *)Value, (int8 *)STR_REG_NFC_EN_KEEP) != NULL) {
            if (get_ec_version_etc() == V100) {
                PS_PRINT_INFO("hi110x V100\n");
            } else {
                PS_PRINT_INFO("hi110x V120\n");
                return SUCC;
            }
        }

        if ((OS_STR_STR((int8 *)Value, (int8 *)STR_REG_PMU_CLK_REQ) != NULL)
            && isPmu_clk_request_enable()) {
            PS_PRINT_INFO("hi110x PMU clk request\n");
            return SUCC;
        }

        l_ret = number_type_cmd_send_etc(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, Value);
            return l_ret;
        }

        l_ret = recv_expect_result_etc(MSG_FROM_DEV_WRITEM_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }

    } else if (!OS_STR_CMP((int8 *)Key, SLEEP_CMD_KEYWORD)) {
        l_delay_ms = simple_strtol(Value, NULL, 10); /* ���ַ���ת����10������ */
        PS_PRINT_INFO("firmware download delay %d ms\n", l_delay_ms);
        // ���ȴ�ʱ��5s����ֹcfg����ʱ��̫�����¼��س�ʱ
        if (l_delay_ms > 0 && l_delay_ms < 5000) {
            msleep(l_delay_ms);
        } else {
            msleep(5);
        }

        return SUCC;
    } else if (!OS_STR_CMP((int8 *)Key, CALI_COUNT_CMD_KEYWORD)) {
        /* ����У׼������device */
        l_ret = update_device_cali_count_etc(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("update device cali count fail\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, CALI_BFGX_DATA_CMD_KEYWORD)) {
        if (ir_only_mode) {
            PS_PRINT_INFO("ir only pass the download cali data cmd\n");
            return SUCC;
        }

        /* ����BFGX��У׼���� */
        l_ret = download_bfgx_cali_data_etc(FILES_CMD_KEYWORD, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("download bfgx cali data fail\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, JUMP_CMD_KEYWORD)) {
        l_ret = number_type_cmd_send_etc(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, Value);
            return l_ret;
        }

        /* 100000ms timeout */
        l_ret = recv_expect_result_timeout_etc(MSG_FROM_DEV_JUMP_OK, READ_MEG_JUMP_TIMEOUT);
        if (l_ret >= 0) {
            PS_PRINT_INFO("JUMP success!\n");
            return l_ret;
        } else {
            PS_PRINT_ERR("CMD JUMP timeout! l_ret=%d\n", l_ret);
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, SETPM_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETBUCK_CMD_KEYWORD) ||
               !OS_STR_CMP((int8 *)Key, SETSYSLDO_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETNFCRETLDO_CMD_KEYWORD) ||
               !OS_STR_CMP((int8 *)Key, SETPD_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETNFCCRG_CMD_KEYWORD) ||
               !OS_STR_CMP((int8 *)Key, SETABB_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, SETTCXODIV_CMD_KEYWORD)) {
        l_ret = number_type_cmd_send_etc(Key, Value);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", Key, Value);
            return l_ret;
        }

        l_ret = recv_expect_result_etc(MSG_FROM_DEV_SET_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, RMEM_CMD_KEYWORD)) {
        l_ret = sdio_read_mem_etc(Key, Value, true);
    }

    return l_ret;
}

/*
 * �� �� ��  : exec_file_type_cmd_etc
 * ��������  : ִ��quit���͵�����
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 exec_quit_type_cmd_etc(void)
{
    int32 l_ret;
    int32 l_len;
    const uint32 ul_buf_len = 8;
    uint8 buf[ul_buf_len];
    BOARD_INFO *board_info = NULL;

    board_info = get_hi110x_board_info_etc();
    if (board_info == NULL) {
        PS_PRINT_ERR("board_info is null!\n");
        return -EFAIL;
    }

    memset_s(buf, sizeof(buf), 0, sizeof(buf));

    l_ret = memcpy_s(buf, sizeof(buf), (uint8 *)QUIT_CMD_KEYWORD, OS_STR_LEN(QUIT_CMD_KEYWORD));
    if (l_ret != EOK) {
        PS_PRINT_ERR("buf not enough\n");
        return -EFAIL;
    }
    l_len = OS_STR_LEN(QUIT_CMD_KEYWORD);

    buf[l_len] = COMPART_KEYWORD;
    l_len++;

    l_ret = msg_send_and_recv_except_etc(buf, l_len, MSG_FROM_DEV_QUIT_OK);

    return l_ret;
}

static int32 file_open_get_len(int8 *path, OS_KERNEL_FILE_STRU **fp, uint32 *file_len)
{
#define RETRY_CONT 3
    mm_segment_t fs;
    int32 i = 0;

    fs = get_fs();
    set_fs(KERNEL_DS);
    for (i = 0; i < RETRY_CONT; i++) {
        *fp = filp_open(path, O_RDONLY, 0);
        if (OAL_IS_ERR_OR_NULL(*fp)) {
            PS_PRINT_ERR("filp_open [%s] fail!!, fp=%pK, errno:%ld\n", path, *fp, PTR_ERR(*fp));
            *fp = NULL;
            continue;
        }
    }

    if (*fp == NULL) {
        PS_PRINT_ERR("filp_open fail!!\n");
        set_fs(fs);
        return -EFAIL;
    }

    /* ��ȡfile�ļ���С */
    *file_len = vfs_llseek(*fp, 0, SEEK_END);
    if (*file_len <= 0) {
        PS_PRINT_ERR("file size of %s is 0!!\n", path);
        filp_close(*fp, NULL);
        set_fs(fs);
        return -EFAIL;
    }

    /* �ָ�fp->f_pos���ļ���ͷ */
    vfs_llseek(*fp, 0, SEEK_SET);
    set_fs(fs);

    return SUCC;
}

/*
 * �� �� ��  : exec_file_type_cmd_etc
 * ��������  : ִ��file���͵�����
 * �������  : Key  : ����Ĺؼ���
 *             Value: ����Ĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ�0��ʾ�ɹ�
 */
int32 exec_file_type_cmd_etc(uint8 *Key, uint8 *Value)
{
    unsigned long addr;
    unsigned long addr_send;
    int8 *path = NULL;
    int32 ret;
    uint32 file_len;
    uint32 transmit_limit;
    uint32 per_send_len;
    uint32 send_count;
    int32 rdlen;
    uint32 i;
    uint32 offset = 0;
    uint8 buff_tx[SEND_BUF_LEN] = {0};
    OS_KERNEL_FILE_STRU *fp = NULL;
    BOARD_INFO *board_info = NULL;

    board_info = get_hi110x_board_info_etc();
    if (board_info == NULL) {
        PS_PRINT_ERR("board_info is null!\n");
        return -EFAIL;
    }

    if (ir_only_mode) {
        PS_PRINT_INFO("ir only pass the download file cmd\n");
        return SUCC;
    }

    ret = parse_file_cmd_etc(Value, &addr, &path);
    if (ret < 0) {
        PS_PRINT_ERR("parse file cmd fail!\n");
        return ret;
    }

    PS_PRINT_INFO("download firmware:%s addr:0x%x\n", path, (uint32)addr);

    ret = file_open_get_len(path, &fp, &file_len);
    if (ret < 0) {
        return ret;
    }

    PS_PRINT_DBG("file len is [%d]\n", file_len);

    transmit_limit = firmware_down_buf_len_etc;  // DataBufLen_etc�ڵ���ǰ��֤Ϊ����0
    per_send_len = (transmit_limit > file_len) ? file_len : transmit_limit;
    send_count = (file_len + per_send_len - 1) / per_send_len;

    for (i = 0; i < send_count; i++) {
        rdlen = oal_file_read_ext(fp, fp->f_pos, firmware_down_buf_etc, per_send_len);

        if (rdlen > 0) {
            PS_PRINT_DBG("len of kernel_read is [%d], i=%d\n", rdlen, i);
            fp->f_pos += rdlen;
        } else {
            PS_PRINT_ERR("len of kernel_read is error! ret=[%d], i=%d\n", rdlen, i);
            oal_file_close(fp);
            if (rdlen < 0) {
                return rdlen;
            } else {
                return -EFAIL;
            }
        }

        addr_send = addr + offset;
        PS_PRINT_DBG("send addr is [0x%lx], i=%d\n", addr_send, i);
        ret = snprintf_s(buff_tx, sizeof(buff_tx), sizeof(buff_tx) - 1, "%s%c%d%c0x%lx%c",
                         FILES_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         FILE_COUNT_PER_SEND,
                         COMPART_KEYWORD,
                         addr_send,
                         COMPART_KEYWORD);
        if (ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            break;
        }
        /* ���͵�ַ */
        PS_PRINT_DBG("send file addr cmd is [%s]\n", buff_tx);
        ret = msg_send_and_recv_except_etc(buff_tx, OS_STR_LEN(buff_tx), MSG_FROM_DEV_READY_OK);
        if (ret < 0) {
            PS_PRINT_ERR("SEND [%s] error\n", buff_tx);
            oal_file_close(fp);
            return -EFAIL;
        }

        /* Wait at least 5 ms */
        oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

        /* �����ļ����� */
        ret = msg_send_and_recv_except_etc(firmware_down_buf_etc, rdlen, MSG_FROM_DEV_FILES_OK);
        if (ret < 0) {
            PS_PRINT_ERR(" sdio send data fail\n");
            oal_file_close(fp);
            return -EFAIL;
        }
        offset += rdlen;
    }

    oal_file_close(fp);

    /* ���͵ĳ���Ҫ���ļ��ĳ���һ�� */
    if (offset != file_len) {
        PS_PRINT_ERR("file send len is err! send len is [%d], file len is [%d]\n", offset, file_len);
        return -EFAIL;
    }

    return SUCC;
}

/*
 * �� �� ��  : exec_shutdown_type_cmd_etc
 * ��������  : ִ��shutdown cpu type������
 * �������  : which_cpu: Ҫ�رյ�cpu
 * �� �� ֵ  : -1��ʾʧ�ܣ��Ǹ�����ʾ�ɹ�
 */
int32 exec_shutdown_type_cmd_etc(uint32 which_cpu)
{
    int32 l_ret = -EFAIL;
    uint8 Value_SHUTDOWN[SHUTDOWN_TX_CMD_LEN];

    if (which_cpu == DEV_WCPU) {
        l_ret = snprintf_s(Value_SHUTDOWN, sizeof(Value_SHUTDOWN), sizeof(Value_SHUTDOWN) - 1, "%d,%s,%d",
                           HOST_TO_DEVICE_CMD_HEAD, SOFT_WCPU_EN_ADDR, 0);
        if (l_ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            return l_ret;
        }

        l_ret = number_type_cmd_send_etc(WMEM_CMD_KEYWORD, Value_SHUTDOWN);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", SHUTDOWN_WIFI_CMD_KEYWORD, Value_SHUTDOWN);
            return l_ret;
        }
    } else if (which_cpu == DEV_BCPU) {
        l_ret = snprintf_s(Value_SHUTDOWN, sizeof(Value_SHUTDOWN), sizeof(Value_SHUTDOWN) - 1, "%d,%s,%d",
                           HOST_TO_DEVICE_CMD_HEAD, SOFT_BCPU_EN_ADDR, 0);
        if (l_ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            return l_ret;
        }

        l_ret = number_type_cmd_send_etc(WMEM_CMD_KEYWORD, Value_SHUTDOWN);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", SHUTDOWN_BFGX_CMD_KEYWORD, Value_SHUTDOWN);
            return l_ret;
        }

        l_ret = recv_expect_result_etc(MSG_FROM_DEV_WRITEM_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }

        l_ret = snprintf_s(Value_SHUTDOWN, sizeof(Value_SHUTDOWN), sizeof(Value_SHUTDOWN) - 1, "%d,%s,%d",
                           HOST_TO_DEVICE_CMD_HEAD, BCPU_DE_RESET_ADDR, 1);
        if (l_ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            return l_ret;
        }

        l_ret = number_type_cmd_send_etc(WMEM_CMD_KEYWORD, Value_SHUTDOWN);
        if (l_ret < 0) {
            PS_PRINT_ERR("send key=[%s],value=[%s] fail\n", SHUTDOWN_BFGX_CMD_KEYWORD, Value_SHUTDOWN);
            return l_ret;
        }

        l_ret = recv_expect_result_etc(MSG_FROM_DEV_WRITEM_OK);
        if (l_ret < 0) {
            PS_PRINT_ERR("recv expect result fail!\n");
            return l_ret;
        }
    } else {
        PS_PRINT_ERR("para is error, which_cpu=[%d]\n", which_cpu);
        return -EFAIL;
    }

    return SUCC;
}

/*
 * �� �� ��  : execute_download_cmd_etc
 * ��������  : ִ��firmware download������
 * �������  : cmd_type: �������������
 *             cmd_name: ����Ĺؼ���
 *             cmd_para: ����Ĳ���
 * �� �� ֵ  : -1��ʾʧ�ܣ��Ǹ�����ʾ�ɹ�
 */
int32 execute_download_cmd_etc(int32 cmd_type, uint8 *cmd_name, uint8 *cmd_para)
{
    int32 l_ret;

    switch (cmd_type) {
        case FILE_TYPE_CMD:
            PS_PRINT_DBG(" command type FILE_TYPE_CMD\n");
            l_ret = exec_file_type_cmd_etc(cmd_name, cmd_para);
            break;
        case NUM_TYPE_CMD:
            PS_PRINT_DBG(" command type NUM_TYPE_CMD\n");
            l_ret = exec_number_type_cmd_etc(cmd_name, cmd_para);
            break;
        case QUIT_TYPE_CMD:
            PS_PRINT_DBG(" command type QUIT_TYPE_CMD\n");
            l_ret = exec_quit_type_cmd_etc();
            break;
        case SHUTDOWN_WIFI_TYPE_CMD:
            PS_PRINT_DBG(" command type SHUTDOWN_WIFI_TYPE_CMD\n");
            l_ret = exec_shutdown_type_cmd_etc(DEV_WCPU);
            break;
        case SHUTDOWN_BFGX_TYPE_CMD:
            PS_PRINT_DBG(" command type SHUTDOWN_BFGX_TYPE_CMD\n");
            l_ret = exec_shutdown_type_cmd_etc(DEV_BCPU);
            break;

        default:
            PS_PRINT_ERR("command type error[%d]\n", cmd_type);
            l_ret = -EFAIL;
            break;
    }

    return l_ret;
}

/*
 * �� �� ��  : firmware_read_cfg_etc
 * ��������  : ��ȡcfg�ļ������ݣ��ŵ�������̬�����buffer��
 * �������  : puc_CfgPatch    : cfg�ļ���·��
 *             puc_read_buffer : ����cfg�ļ����ݵ�buffer
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 firmware_read_cfg_etc(const char *puc_CfgPatch, uint8 *puc_read_buffer)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    int32 l_ret;
    mm_segment_t fs;

    if ((puc_CfgPatch == NULL) || (puc_read_buffer == NULL)) {
        PS_PRINT_ERR("para is NULL\n");
        return -EFAIL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(puc_CfgPatch, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        PS_PRINT_ERR("open file %s fail, fp=%p\n", puc_CfgPatch, fp);
        set_fs(fs);
        fp = NULL;
        return -EFAIL;
    }

    memset_s(puc_read_buffer, READ_CFG_BUF_LEN, 0, READ_CFG_BUF_LEN);

    l_ret = oal_file_read_ext(fp, fp->f_pos, puc_read_buffer, READ_CFG_BUF_LEN);

    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;

    return l_ret;
}

/*
 * �� �� ��  : firmware_parse_cmd_etc
 * ��������  : ����cfg�ļ��е�����
 * �������  : puc_cfg_buffer: ����cfg�ļ����ݵ�buffer
 *             puc_cmd_name  : ��������Ժ�����ؼ��ֵ�buffer
 *             puc_cmd_para  : ��������Ժ����������buffer
 * �� �� ֵ  : �������������
 */
int32 firmware_parse_cmd_etc(uint8 *puc_cfg_buffer, uint8 *puc_cmd_name, uint32 cmd_name_len,
                             uint8 *puc_cmd_para, uint32 cmd_para_len)
{
    int32 ret;
    int32 cmd_type;
    int32 l_cmdlen;
    int32 l_paralen;
    uint8 *begin = NULL;
    uint8 *end = NULL;
    uint8 *link = NULL;
    uint8 *handle = NULL;
    uint8 *handle_temp = NULL;

    begin = puc_cfg_buffer;
    if ((puc_cfg_buffer == NULL) || (puc_cmd_name == NULL) || (puc_cmd_para == NULL)) {
        PS_PRINT_ERR("para is NULL\n");
        return ERROR_TYPE_CMD;
    }

    /* ע���� */
    if (puc_cfg_buffer[0] == '@') {
        return ERROR_TYPE_CMD;
    }

    /* �����У������˳������� */
    link = OS_STR_CHR((int8 *)begin, '=');
    if (link == NULL) {
        /* �˳������� */
        if (OS_STR_STR((int8 *)puc_cfg_buffer, QUIT_CMD_KEYWORD) != NULL) {
            return QUIT_TYPE_CMD;
        } else if (OS_STR_STR((int8 *)puc_cfg_buffer, SHUTDOWN_WIFI_CMD_KEYWORD) != NULL) {
            return SHUTDOWN_WIFI_TYPE_CMD;
        } else if (OS_STR_STR((int8 *)puc_cfg_buffer, SHUTDOWN_BFGX_CMD_KEYWORD) != NULL) {
            return SHUTDOWN_BFGX_TYPE_CMD;
        }

        return ERROR_TYPE_CMD;
    }

    /* �����У�û�н����� */
    end = OS_STR_CHR(link, ';');
    if (end == NULL) {
        return ERROR_TYPE_CMD;
    }

    l_cmdlen = link - begin;

    /* ɾ���ؼ��ֵ����߿ո� */
    handle = delete_space_etc((uint8 *)begin, &l_cmdlen);
    if (handle == NULL) {
        return ERROR_TYPE_CMD;
    }

    /* �ж��������� */
    if ((l_cmdlen >= OS_STR_LEN(FILE_TYPE_CMD_KEY)) &&
        !OS_MEM_CMP(handle, FILE_TYPE_CMD_KEY, OS_STR_LEN(FILE_TYPE_CMD_KEY))) {
        handle_temp = OS_STR_STR(handle, FILE_TYPE_CMD_KEY);
        if (handle_temp == NULL) {
            PS_PRINT_ERR("'ADDR_FILE_'is not handle child string, handle=%s", handle);
            return ERROR_TYPE_CMD;
        }
        handle = handle_temp + OS_STR_LEN(FILE_TYPE_CMD_KEY);
        l_cmdlen = l_cmdlen - OS_STR_LEN(FILE_TYPE_CMD_KEY);
        cmd_type = FILE_TYPE_CMD;
    } else if ((l_cmdlen >= OS_STR_LEN(NUM_TYPE_CMD_KEY)) &&
               !OS_MEM_CMP(handle, NUM_TYPE_CMD_KEY, OS_STR_LEN(NUM_TYPE_CMD_KEY))) {
        handle_temp = OS_STR_STR(handle, NUM_TYPE_CMD_KEY);
        if (handle_temp == NULL) {
            PS_PRINT_ERR("'PARA_' is not handle child string, handle=%s", handle);
            return ERROR_TYPE_CMD;
        }
        handle = handle_temp + OS_STR_LEN(NUM_TYPE_CMD_KEY);
        l_cmdlen = l_cmdlen - OS_STR_LEN(NUM_TYPE_CMD_KEY);
        cmd_type = NUM_TYPE_CMD;
    } else {
        return ERROR_TYPE_CMD;
    }

    ret = memcpy_s(puc_cmd_name, cmd_name_len, handle, l_cmdlen);
    if (ret != EOK) {
        PS_PRINT_ERR("cmd len out range! ret = %d\n", ret);
        return ERROR_TYPE_CMD;
    }
    /* ɾ��ֵ���߿ո� */
    begin = link + 1;
    l_paralen = end - begin;

    handle = delete_space_etc(begin, &l_paralen);
    if (handle == NULL) {
        return ERROR_TYPE_CMD;
    }
    ret = memcpy_s(puc_cmd_para, cmd_para_len, handle, l_paralen);
    if (ret != EOK) {
        PS_PRINT_ERR("para len out of range!ret = %d\n", ret);
        return ERROR_TYPE_CMD;
    }

    return cmd_type;
}

/*
 * �� �� ��  : firmware_parse_cfg_etc
 * ��������  : ����cfg�ļ����������Ľ��������g_st_cfg_infoȫ�ֱ�����
 * �������  : puc_cfg_info_buf: ������cfg�ļ����ݵ�buffer
 *             l_buf_len       : puc_cfg_info_buf�ĳ���
 *             ul_index        : ��������������������ֵ
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 firmware_parse_cfg_etc(uint8 *puc_cfg_info_buf, int32 l_buf_len, uint32 ul_index)
{
    int32 i;
    int32 l_len;
    uint8 *flag = NULL;
    uint8 *begin = NULL;
    uint8 *end = NULL;
    int32 cmd_type;
    uint8 cmd_name[DOWNLOAD_CMD_LEN];
    uint8 cmd_para[DOWNLOAD_CMD_PARA_LEN];
    uint32 cmd_para_len = 0;
    int32 ret;
    if (puc_cfg_info_buf == NULL) {
        PS_PRINT_ERR("puc_cfg_info_buf is NULL!\n");
        return -EFAIL;
    }

    cfg_info_etc.apst_cmd[ul_index] = (struct cmd_type_st *)malloc_cmd_buf_etc(puc_cfg_info_buf, ul_index);
    if (cfg_info_etc.apst_cmd[ul_index] == NULL) {
        PS_PRINT_ERR(" malloc_cmd_buf_etc fail!\n");
        return -EFAIL;
    }

    /* ����CMD BUF */
    flag = puc_cfg_info_buf;
    l_len = l_buf_len;
    i = 0;
    while ((i < cfg_info_etc.al_count[ul_index]) && (flag < &puc_cfg_info_buf[l_len])) {
        /*
         * ��ȡ�����ļ��е�һ��,�����ļ�������unix��ʽ.
         * �����ļ��е�ĳһ�к����ַ� @ ����Ϊ����Ϊע����
         */
        begin = flag;
        end = OS_STR_CHR(flag, '\n');
        if (end == NULL) { /* �ļ������һ�У�û�л��з� */
            PS_PRINT_DBG("lost of new line!\n");
            end = &puc_cfg_info_buf[l_len];
        } else if (end == begin) { /* ����ֻ��һ�����з� */
            PS_PRINT_DBG("blank line\n");
            flag = end + 1;
            continue;
        }
        *end = '\0';

        PS_PRINT_DBG("operation string is [%s]\n", begin);

        memset_s(cmd_name, DOWNLOAD_CMD_LEN, 0, DOWNLOAD_CMD_LEN);
        memset_s(cmd_para, DOWNLOAD_CMD_PARA_LEN, 0, DOWNLOAD_CMD_PARA_LEN);

        cmd_type = firmware_parse_cmd_etc(begin, cmd_name, sizeof(cmd_name), cmd_para, sizeof(cmd_para));

        PS_PRINT_DBG("cmd type=[%d],cmd_name=[%s],cmd_para=[%s]\n", cmd_type, cmd_name, cmd_para);

        if (cmd_type != ERROR_TYPE_CMD) { /* ��ȷ���������ͣ����� */
            cfg_info_etc.apst_cmd[ul_index][i].cmd_type = cmd_type;
            memcpy_s(cfg_info_etc.apst_cmd[ul_index][i].cmd_name, DOWNLOAD_CMD_LEN, cmd_name, DOWNLOAD_CMD_LEN);
            memcpy_s(cfg_info_etc.apst_cmd[ul_index][i].cmd_para, DOWNLOAD_CMD_PARA_LEN,
                     cmd_para, DOWNLOAD_CMD_PARA_LEN);
            /* ��ȡ���ð汾�� */
            if (!OS_MEM_CMP(cfg_info_etc.apst_cmd[ul_index][i].cmd_name,
                            VER_CMD_KEYWORD,
                            OS_STR_LEN(VER_CMD_KEYWORD))) {
                cmd_para_len = OS_STR_LEN(cfg_info_etc.apst_cmd[ul_index][i].cmd_para);
                ret = memcpy_s(cfg_info_etc.auc_CfgVersion, sizeof(cfg_info_etc.auc_CfgVersion),
                               cfg_info_etc.apst_cmd[ul_index][i].cmd_para, cmd_para_len);
                if (ret != EOK) {
                    PS_PRINT_DBG("cmd_para_len = %d over auc_CfgVersion length", cmd_para_len);
                    return -EFAIL;
                }

                PS_PRINT_DBG("g_CfgVersion = [%s].\n", cfg_info_etc.auc_CfgVersion);
            }
            i++;
        }
        flag = end + 1;
    }

    /* ����ʵ������������޸����յ�������� */
    cfg_info_etc.al_count[ul_index] = i;
    PS_PRINT_INFO("effective cmd count: al_count[%d] = %d\n", ul_index, cfg_info_etc.al_count[ul_index]);

    return SUCC;
}

/*
 * �� �� ��  : firmware_get_cfg_etc
 * ��������  : ��ȡcfg�ļ����������������Ľ��������g_st_cfg_infoȫ�ֱ�����
 * �������  : puc_CfgPatch: cfg�ļ���·��
 *             ul_index     : ��������������������ֵ
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 firmware_get_cfg_etc(uint8 *puc_CfgPatch, uint32 ul_index)
{
    uint8 *puc_read_cfg_buf = NULL;
    int32 l_readlen;
    int32 l_ret;

    if (puc_CfgPatch == NULL) {
        PS_PRINT_ERR("cfg file path is null!\n");
        return -EFAIL;
    }

    /* cfg�ļ��޶���С��2048,���cfg�ļ��Ĵ�Сȷʵ����2048�������޸�READ_CFG_BUF_LEN��ֵ */
    puc_read_cfg_buf = OS_KMALLOC_GFP(READ_CFG_BUF_LEN);
    if (puc_read_cfg_buf == NULL) {
        PS_PRINT_ERR("kmalloc READ_CFG_BUF fail!\n");
        return -EFAIL;
    }

    l_readlen = firmware_read_cfg_etc(puc_CfgPatch, puc_read_cfg_buf);
    if (l_readlen < 0) {
        PS_PRINT_ERR("read cfg error!\n");
        OS_MEM_KFREE(puc_read_cfg_buf);
        puc_read_cfg_buf = NULL;
        return -EFAIL;
    }
    /* ��1��Ϊ��ȷ��cfg�ļ��ĳ��Ȳ�����READ_CFG_BUF_LEN����Ϊfirmware_read_cfg���ֻ���ȡREAD_CFG_BUF_LEN���ȵ����� */
    else if (l_readlen > READ_CFG_BUF_LEN - 1) {
        PS_PRINT_ERR("cfg file [%s] larger than %d\n", puc_CfgPatch, READ_CFG_BUF_LEN);
        OS_MEM_KFREE(puc_read_cfg_buf);
        puc_read_cfg_buf = NULL;
        return -EFAIL;
    } else {
        PS_PRINT_DBG("read cfg file [%s] ok, size is [%d]\n", puc_CfgPatch, l_readlen);
    }

    l_ret = firmware_parse_cfg_etc(puc_read_cfg_buf, l_readlen, ul_index);
    if (l_ret < 0) {
        PS_PRINT_ERR("parse cfg error!\n");
    }

    OS_MEM_KFREE(puc_read_cfg_buf);
    puc_read_cfg_buf = NULL;

    return l_ret;
}

/*
 * �� �� ��  : firmware_download_etc
 * ��������  : firmware����
 * �������  : ul_index: ��Ч�����������������
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 firmware_download_etc(uint32 ul_index)
{
    int32 l_ret;
    int32 i;
    int32 l_cmd_type;
    uint8 *puc_cmd_name = NULL;
    uint8 *puc_cmd_para = NULL;
    hcc_bus *pst_bus = NULL;

    if (ul_index >= CFG_FILE_TOTAL) {
        PS_PRINT_ERR("ul_index [%d] is error!\n", ul_index);
        return -EFAIL;
    }

    store_efuse_info_etc();

    PS_PRINT_INFO("start download firmware, ul_index = [%d]\n", ul_index);

    if (cfg_info_etc.al_count[ul_index] == 0) {
        PS_PRINT_ERR("firmware download cmd count is 0, ul_index = [%d]\n", ul_index);
        return -EFAIL;
    }

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        PS_PRINT_ERR("firmware curr bus is null, ul_index = [%d]\n", ul_index);
        return -EFAIL;
    }

    firmware_down_buf_etc = (uint8 *)oal_memtry_alloc(OAL_MIN(pst_bus->cap.max_trans_size,
                                                              MAX_FIRMWARE_FILE_TX_BUF_LEN),
                                                      MIN_FIRMWARE_FILE_TX_BUF_LEN,
                                                      &firmware_down_buf_len_etc);
    firmware_down_buf_len_etc = OAL_ROUND_DOWN(firmware_down_buf_len_etc, 8); /* �����3bit����֤8�ֽڶ��� */

    if (firmware_down_buf_etc == NULL || (firmware_down_buf_len_etc == 0)) {
        PS_PRINT_ERR("g_pucDataBuf_etc KMALLOC failed, min request:%u\n", MIN_FIRMWARE_FILE_TX_BUF_LEN);
        return -EFAIL;
    }

    PS_PRINT_INFO("download firmware file buf len is [%d]\n", firmware_down_buf_len_etc);

    for (i = 0; i < cfg_info_etc.al_count[ul_index]; i++) {
        l_cmd_type = cfg_info_etc.apst_cmd[ul_index][i].cmd_type;
        puc_cmd_name = cfg_info_etc.apst_cmd[ul_index][i].cmd_name;
        puc_cmd_para = cfg_info_etc.apst_cmd[ul_index][i].cmd_para;

        PS_PRINT_DBG("cmd[%d]:type[%d], name[%s], para[%s]\n", i, l_cmd_type, puc_cmd_name, puc_cmd_para);

        PS_PRINT_DBG("firmware down start cmd[%d]:type[%d], name[%s]\n", i, l_cmd_type, puc_cmd_name);

        l_ret = execute_download_cmd_etc(l_cmd_type, puc_cmd_name, puc_cmd_para);
        if (l_ret < 0) {
            OS_MEM_KFREE(firmware_down_buf_etc);
            firmware_down_buf_etc = NULL;
            PS_PRINT_ERR("download firmware fail\n");

            return l_ret;
        }

        PS_PRINT_DBG("firmware down finish cmd[%d]:type[%d], name[%s]\n", i, l_cmd_type, puc_cmd_name);
    }

    OS_MEM_KFREE(firmware_down_buf_etc);
    firmware_down_buf_etc = NULL;

    PS_PRINT_INFO("finish download firmware\n");

    return SUCC;
}

int32 print_firmware_download_cmd(uint32 ul_index)
{
    int32 i;
    int32 l_cmd_type;
    uint8 *puc_cmd_name = NULL;
    uint8 *puc_cmd_para = NULL;
    uint32 count;

    count = cfg_info_etc.al_count[ul_index];
    PS_PRINT_INFO("[%s] download cmd, total count is [%d]\n", cfg_path_etc[ul_index], count);

    for (i = 0; i < count; i++) {
        l_cmd_type = cfg_info_etc.apst_cmd[ul_index][i].cmd_type;
        puc_cmd_name = cfg_info_etc.apst_cmd[ul_index][i].cmd_name;
        puc_cmd_para = cfg_info_etc.apst_cmd[ul_index][i].cmd_para;

        PS_PRINT_INFO("cmd[%d]:type[%d], name[%s], para[%s]\n", i, l_cmd_type, puc_cmd_name, puc_cmd_para);
    }

    return 0;
}

int32 print_cfg_file_cmd_etc(void)
{
    int32 i;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        print_firmware_download_cmd(i);
    }

    return 0;
}

/*
 * �� �� ��  : firmware_cfg_path_init
 * ��������  : ��ȡfirmware��cfg�ļ�·��
 * �� �� ֵ  : 0��ʾ�ɹ���-1��ʾʧ��
 */
int32 firmware_cfg_path_init(void)
{
    int32 l_ret;
    int32 l_len;
    uint8 rec_buf[VERSION_LEN];

    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1103) {
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);

        l_ret = memcpy_s(rec_buf, sizeof(rec_buf), (uint8 *)VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD));
        if (l_ret != EOK) {
            PS_PRINT_ERR("rec_buf not enough\n");
            return -EFAIL;
        }
        l_len = OS_STR_LEN(VER_CMD_KEYWORD);

        rec_buf[l_len] = COMPART_KEYWORD;
        l_len++;

        l_ret = send_msg_etc(rec_buf, l_len);
        if (l_ret < 0) {
            PS_PRINT_ERR("Hi1103 send version cmd fail!\n");
            return -EFAIL;
        }

        msleep(1);

        l_ret = read_msg_etc(rec_buf, VERSION_LEN);
        if (l_ret < 0) {
            PS_PRINT_ERR("Hi1103 read version fail!\n");
            return -EFAIL;
        }

        PS_PRINT_INFO("Hi1103 Device Version=[%s].\n", rec_buf);

        if (!OS_MEM_CMP((int8 *)rec_buf, (int8 *)HI1103_MPW2_BOOTLOADER_VERSION,
                        OS_STR_LEN(HI1103_MPW2_BOOTLOADER_VERSION))) {
            cfg_path_etc = mpw2_cfg_patch_in_vendor;
            set_hi1103_asic_type(HI1103_ASIC_MPW2);
            return SUCC;
        } else if (!OS_MEM_CMP((int8 *)rec_buf, (int8 *)HI1103_PILOT_BOOTLOADER_VERSION,
                               OS_STR_LEN(HI1103_PILOT_BOOTLOADER_VERSION))) {
            cfg_path_etc = pilot_cfg_patch_in_vendor;
            set_hi1103_asic_type(HI1103_ASIC_PILOT);
            return SUCC;
        } else {
            PS_PRINT_WARNING("Hi1103 Device Version Error!\n");
            cfg_path_etc = pilot_cfg_patch_in_vendor;
            set_hi1103_asic_type(HI1103_ASIC_PILOT);
            return SUCC;
        }
    } else if (get_hi110x_subchip_type() == BOARD_VERSION_HI1105) {
        cfg_path_etc = hi1105_fpga_cfg_patch_in_vendor;
        set_hi1103_asic_type(HI1105_FPGA);
    }

    return SUCC;
}

/*
 * �� �� ��  : firmware_cfg_init_etc
 * ��������  : firmware���ص�cfg�ļ���ʼ������ȡ������cfg�ļ����������Ľ��������
 *             cfg_infoȫ�ֱ�����
 */
int32 firmware_cfg_init_etc(void)
{
    int32 l_ret;
    uint32 i;

    l_ret = firmware_cfg_path_init();
    if (l_ret != SUCC) {
        PS_PRINT_ERR("firmware cfg path init fail!");
        return -EFAIL;
    }

    /* ����cfg�ļ� */
    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        l_ret = firmware_get_cfg_etc(cfg_path_etc[i], i);
        if (l_ret < 0) {
            if (i == RAM_REG_TEST_CFG) {
                PS_PRINT_WARNING("ram_reg_test_cfg maybe not exist, please check\n");
                continue;
            }

            PS_PRINT_ERR("get cfg file [%s] fail\n", cfg_path_etc[i]);
            goto cfg_file_init_fail;
        }
    }

    return SUCC;

cfg_file_init_fail:
    firmware_cfg_clear_etc();

    return -EFAIL;
}

/*
 * �� �� ��  : firmware_cfg_clear_etc
 * ��������  : �ͷ�firmware_cfg_initʱ������ڴ�
 * �� �� ֵ  : ���Ƿ���0����ʾ�ɹ�
 */
int32 firmware_cfg_clear_etc(void)
{
    int32 i;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        cfg_info_etc.al_count[i] = 0;
        if (cfg_info_etc.apst_cmd[i] != NULL) {
            OS_MEM_KFREE(cfg_info_etc.apst_cmd[i]);
            cfg_info_etc.apst_cmd[i] = NULL;
        }
    }

    return SUCC;
}

/*
 * �� �� ��  : nfc_buffer_data_recv_etc
 * ��������  : ����nfc buffer����
 */
int32 nfc_buffer_data_recv_etc(uint8 *pucDataBuf, int32 len)
{
    uint32 l_ret = 0;
    int32 lenbuf = 0;
    int32 retry = 3;

    if (pucDataBuf == NULL) {
        PS_PRINT_ERR("pucDataBuf is NULL\n");
        return -EFAIL;
    }

    // ��������
    while (len > lenbuf) {
        l_ret = read_msg_etc(pucDataBuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                PS_PRINT_ERR("time out\n");
                return -EFAIL;
            }
        }
    }
    return SUCC;
}

#define TSENSOR_VALID_HIGHEST_TEMP 125
#define TSENSOR_VALID_LOWEST_TEMP  (-40)

#define DEVICE_MEM_CHECK_SUCC 0x000f
#define RESULT_DETAIL_REG     "0x5000001c,4"
#define RESULT_TIME_LOW_REG   "0x50000010,4"
#define RESULT_TIME_HIGH_REG  "0x50000014,4"
#define RESULT_TSENSOR_C0_REG "0x5000348c,4"
#define RESULT_TSENSOR_C1_REG "0x500034cc,4"
#define GET_MEM_CHECK_FLAG    "0x50000018,4"

#define PRO_RAM_TEST_CASE_NONE                                   0x0
#define PRO_RAM_TEST_CASE_TCM_SCAN                               0x1
#define PRO_RAM_TEST_CASE_RAM_ALL_SCAN                           0x2
#define PRO_RAM_TEST_CASE_REG_SCAN                               0x3
#define PRO_RAM_TEST_CASE_BT_EM_SCAN                             0x4
#define PRO_RAM_TEST_CASE_MBIST                                  0x5
#define PRO_RAM_TEST_CASE_MBIST_CLDO1_WL0_MAJORITY_FULL_TEST_NEW 0x6
#define PRO_RAM_TEST_CASE_MBIST_CLDO2_FULL_TEST_NEW              0x7
#define PRO_RAM_TEST_CASE_MBIST_CLDO1_OTHER_FULL_TEST_NEW        0x8
#define PRO_RAM_TEST_CASE_MBIST_CLDO1_WL0_320_FULL_TEST_NEW      0x9
#define PRO_RAM_TEST_CASE_MBIST_CLDO1_WL0_MAJORITY_TEST_NEW      0xa

#define PRO_DETAIL_RESULT_NOT_START                   0x0000
#define PRO_DETAIL_RESULT_SUCC                        0xf000
#define PRO_DETAIL_RESULT_RUNING                      0xffff
#define PRO_DETAIL_RESULT_CASE_0                      0x1
#define PRO_DETAIL_RESULT_CASE_1                      0x2
#define PRO_DETAIL_RESULT_CASE_2_1                    0x3
#define PRO_DETAIL_RESULT_CASE_2_2                    0x4
#define PRO_DETAIL_RESULT_CASE_3                      0x5
#define PRO_DETAIL_ERROR_RESULT(main_case, test_case) (0xf000 | ((main_case) << 8) | ((test_case)&0xff))
#define PRO_GET_DETAIL_ERROR_RESULT_MAIN(error)       (((error) >> 8) & 0xf)
#define PRO_GET_DETAIL_ERROR_RESULT_SUB(error)        (((error) >> 0) & 0xff)

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tsensor_c0_auto_clr : 1;   /* [0]  */
        unsigned int tsensor_c0_rdy_auto : 1;   /* [1]  */
        unsigned int tsensor_c0_data_auto : 10; /* [11..2]  */
        unsigned int reserved : 4;              /* [15..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;

} TSENSOR_AUTO_STS;

typedef struct _mem_test_type_str_ {
    uint32 id;
    char *name;
} mem_test_type_str;

mem_test_type_str mem_test_main_type[] = {
    { PRO_DETAIL_RESULT_CASE_0,   "SYSTEM_INIT" },
    { PRO_DETAIL_RESULT_CASE_1,   "CASE_1" },
    { PRO_DETAIL_RESULT_CASE_2_1, "CASE_2_1" },
    { PRO_DETAIL_RESULT_CASE_2_2, "CASE_2_2" },
    { PRO_DETAIL_RESULT_CASE_3,   "CASE_3" },
};

mem_test_type_str mem_test_sub_type[] = {
    { PRO_RAM_TEST_CASE_NONE,                                   "NOT_OVER" },
    { PRO_RAM_TEST_CASE_TCM_SCAN,                               "TCM_SCAN" },
    { PRO_RAM_TEST_CASE_RAM_ALL_SCAN,                           "RAM_ALL_SCAN" },
    { PRO_RAM_TEST_CASE_REG_SCAN,                               "REG_SCAN" },
    { PRO_RAM_TEST_CASE_BT_EM_SCAN,                             "BT_EM_SCAN" },
    { PRO_RAM_TEST_CASE_MBIST,                                  "MBIST_INIT" },
    { PRO_RAM_TEST_CASE_MBIST_CLDO1_WL0_MAJORITY_FULL_TEST_NEW, "CLDO1_WL0_MAJORITY_FULL_TEST_NEW" },
    { PRO_RAM_TEST_CASE_MBIST_CLDO2_FULL_TEST_NEW,              "CLDO2_FULL_TEST_NEW" },
    { PRO_RAM_TEST_CASE_MBIST_CLDO1_OTHER_FULL_TEST_NEW,        "CLDO1_OTHER_FULL_TEST_NEW" },
    { PRO_RAM_TEST_CASE_MBIST_CLDO1_WL0_320_FULL_TEST_NEW,      "CLDO1_WL0_320_FULL_TEST_NEW" },
    { PRO_RAM_TEST_CASE_MBIST_CLDO1_WL0_MAJORITY_TEST_NEW,      "CLDO1_WL0_MAJORITY_TEST_NEW" },
};

static char *get_memcheck_error_main_string(uint32 result)
{
    uint16 main_type;
    uint32 i;
    uint32 num = sizeof(mem_test_main_type) / sizeof(mem_test_type_str);
    main_type = PRO_GET_DETAIL_ERROR_RESULT_MAIN(result);

    for (i = 0; i < num; i++) {
        if (main_type == mem_test_main_type[i].id) {
            return mem_test_main_type[i].name;
        }
    }

    return NULL;
}

static char *get_memcheck_error_sub_string(uint32 result)
{
    uint16 sub_type;
    uint32 i;
    uint32 num = sizeof(mem_test_sub_type) / sizeof(mem_test_type_str);
    sub_type = PRO_GET_DETAIL_ERROR_RESULT_SUB(result);
    for (i = 0; i < num; i++) {
        if (sub_type == mem_test_sub_type[i].id) {
            return mem_test_sub_type[i].name;
        }
    }

    return NULL;
}

int32 is_device_mem_test_succ(void)
{
    int32 ret;
    int32 test_flag = 0;

    ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, GET_MEM_CHECK_FLAG);
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, GET_MEM_CHECK_FLAG, ret);
        return -1;
    }

    ret = read_msg_etc((uint8 *)&test_flag, sizeof(test_flag));
    if (ret < 0) {
        PS_PRINT_WARNING("read device test flag fail, read_len = %d, return = %d\n", (int32)sizeof(test_flag), ret);
        return -1;
    }
    PS_PRINT_WARNING("get device test flag:0x%x\n", test_flag);
    if (test_flag == DEVICE_MEM_CHECK_SUCC) {
        return 0;
    }
    return -1;
}

void print_device_ram_test_detail_result(int32 is_wcpu, uint32 result)
{
    char *case_name = NULL;
    char *main_str = NULL;
    char *sub_str = NULL;
    if (is_wcpu == true) {
        case_name = "[wcpu_memcheck]";
    } else {
        case_name = "[bcpu_memcheck]";
    }
    PS_PRINT_WARNING("%s detail result=0x%x\n", case_name, result);
    if (result == PRO_DETAIL_RESULT_NOT_START) {
        PS_PRINT_WARNING("%s didn't start run\n", case_name);
        return;
    } else if (result == PRO_DETAIL_RESULT_SUCC) {
        PS_PRINT_WARNING("%s run succ\n", case_name);
        return;
    } else if (result == PRO_DETAIL_RESULT_RUNING) {
        PS_PRINT_WARNING("%s still running\n", case_name);
        return;
    }

    main_str = get_memcheck_error_main_string(result);
    sub_str = get_memcheck_error_sub_string(result);

    PS_PRINT_ERR("%s error found [%s:%s]\n",
                 case_name,
                 (main_str == NULL) ? "unkown" : main_str,
                 (sub_str == NULL) ? "unkown" : sub_str);
}

int32 get_device_ram_test_result(int32 is_wcpu, uint32 *cost)
{
    int32 ret;
    uint32 result = 0;
    hcc_bus *pst_bus;
    uint32 time_cost;

    *cost = 0;

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        PS_PRINT_ERR("pst_bus is null");
        return -OAL_EFAIL;
    }

    if (pst_bus->bus_type != HCC_BUS_PCIE) {
        if (is_wcpu != true) {
            /* bcpu ����wmbist �ᵼ��sdio�ӿ��޷��ض������ֳ������ض� */
            PS_PRINT_ERR("bcpu ram test can't read detail result");
            return -OAL_ENODEV;
        }
    }

    /* ʧ�ܺ��ȡ��ϸ�Ľ�� */
    ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, RESULT_DETAIL_REG);
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_DETAIL_REG, ret);
        return -1;
    }

    ret = read_msg_etc((uint8 *)&result, sizeof(result));
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s read result failed,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_DETAIL_REG, ret);
        return -1;
    }

    print_device_ram_test_detail_result(is_wcpu, result);

    ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, RESULT_TIME_LOW_REG);
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_TIME_LOW_REG, ret);
        return -1;
    }

    ret = read_msg_etc((uint8 *)&result, sizeof(result));
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s read result failed,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_TIME_LOW_REG, ret);
        return -1;
    }

    time_cost = (result & 0xffff);

    ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, RESULT_TIME_HIGH_REG);
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_TIME_HIGH_REG, ret);
        return -1;
    }

    ret = read_msg_etc((uint8 *)&result, sizeof(result));
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s read result failed,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_TIME_HIGH_REG, ret);
        return -1;
    }

    time_cost |= ((result & 0xffff) << 16);
    PS_PRINT_WARNING("%s_ram_test_time_cost tick:%u  %u us\n",
                     (is_wcpu == true) ? "wcpu" : "bcpu", time_cost, time_cost * 31); /* time from 32k */
    *cost = time_cost * 31;

    /* tsensor read */
    if (ram_test_detail_tsensor_dump) {
        uint32 tsensor_c0 = 0;
        uint32 tsensor_c1 = 0;
        oal_uint16 us_reg_val;
        oal_uint16 us_temp_data;
        oal_int32 l_temp_val;
        TSENSOR_AUTO_STS *pun_tsensor_auto_sts = NULL;

        ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, RESULT_TSENSOR_C0_REG);
        if (ret < 0) {
            PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_TSENSOR_C0_REG, ret);
            return -1;
        }

        ret = read_msg_etc((uint8 *)&tsensor_c0, sizeof(tsensor_c0));
        if (ret < 0) {
            PS_PRINT_WARNING("send cmd %s:%s read result failed,ret = %d\n",
                             RMEM_CMD_KEYWORD, RESULT_TSENSOR_C0_REG, ret);
            return -1;
        }

        ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, RESULT_TSENSOR_C1_REG);
        if (ret < 0) {
            PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, RESULT_TSENSOR_C1_REG, ret);
            return -1;
        }

        ret = read_msg_etc((uint8 *)&tsensor_c1, sizeof(tsensor_c1));
        if (ret < 0) {
            PS_PRINT_WARNING("send cmd %s:%s read result failed,ret = %d\n",
                             RMEM_CMD_KEYWORD, RESULT_TSENSOR_C1_REG, ret);
            return -1;
        }

        us_reg_val = tsensor_c0;
        pun_tsensor_auto_sts = (TSENSOR_AUTO_STS *)&us_reg_val;
        us_temp_data = pun_tsensor_auto_sts->bits.tsensor_c0_data_auto;
        l_temp_val = ((((oal_int32)us_temp_data - 118) * 165) / 815) - 40; /* �¶���ת����ʽ */
        PS_PRINT_INFO("[memcheck]TsensorC0, val 0x%x data,%u rdy,%u temp %d\n",
                      us_reg_val, pun_tsensor_auto_sts->bits.tsensor_c0_data_auto,
                      pun_tsensor_auto_sts->bits.tsensor_c0_rdy_auto, l_temp_val);
        if ((pun_tsensor_auto_sts->bits.tsensor_c0_rdy_auto) &&
            ((l_temp_val < TSENSOR_VALID_LOWEST_TEMP) || (l_temp_val > TSENSOR_VALID_HIGHEST_TEMP))) {
            PS_PRINT_INFO("[memcheck]TsensorC0, invalid");
        }

        us_reg_val = tsensor_c1;
        pun_tsensor_auto_sts = (TSENSOR_AUTO_STS *)&us_reg_val;
        us_temp_data = pun_tsensor_auto_sts->bits.tsensor_c0_data_auto;
        l_temp_val = ((((oal_int32)us_temp_data - 118) * 165) / 815) - 40; /* �¶���ת����ʽ */
        PS_PRINT_INFO("[memcheck]TsensorC1, val 0x%x data,%u rdy,%u temp %d\n",
                      us_reg_val, pun_tsensor_auto_sts->bits.tsensor_c0_data_auto,
                      pun_tsensor_auto_sts->bits.tsensor_c0_rdy_auto, l_temp_val);
        if ((pun_tsensor_auto_sts->bits.tsensor_c0_rdy_auto) &&
            ((l_temp_val < TSENSOR_VALID_LOWEST_TEMP) || (l_temp_val > TSENSOR_VALID_HIGHEST_TEMP))) {
            PS_PRINT_INFO("[memcheck]TsensorC1, invalid");
        }
    }

    return 0;
}

int32 get_device_test_mem(bool is_wifi)
{
    wlan_memdump_t *wlan_memdump_s = NULL;
    oal_int32 ret;
    const uint32 ul_buff_len = 100;
    uint8 buff[ul_buff_len];
    wlan_memdump_s = get_wlan_memdump_cfg();
    if (wlan_memdump_s == NULL) {
        PS_PRINT_ERR("memdump cfg is NULL!\n");
        return -FAILURE;
    }
    ret = snprintf_s(buff, sizeof(buff), sizeof(buff) - 1, "0x%x,%d", wlan_memdump_s->addr, wlan_memdump_s->len);
    if (ret < 0) {
        PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
        return ret;
    }
    if (sdio_read_mem_etc(RMEM_CMD_KEYWORD, buff, is_wifi) >= 0) {
        PS_PRINT_WARNING("read device mem succ\n");
    } else {
        PS_PRINT_WARNING("read device mem fail\n");
        return -FAILURE;
    }
    return 0;
}
