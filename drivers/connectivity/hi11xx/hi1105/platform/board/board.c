

/* ͷ�ļ����� */
#include "board.h"

#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
/*lint -e322*/ /*lint -e7*/
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif

#include <linux/fs.h>
/*lint +e322*/ /*lint +e7*/
#include "plat_debug.h"
#include "oal_ext_if.h"
#include "oal_sdio_host_if.h"
#include "plat_firmware.h"
#include "oal_hcc_bus.h"
#include "plat_pm.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "oneimage.h"
#include "oal_pcie_host.h"

/* ȫ�ֱ������� */
BOARD_INFO board_info_etc = { .ssi_gpio_clk = 0, .ssi_gpio_data = 0 };
EXPORT_SYMBOL(board_info_etc);

OAL_STATIC int32 board_probe_ret = 0;
OAL_STATIC struct completion board_driver_complete;

DEVICE_BOARD_VERSION device_board_version_list_etc[BOARD_VERSION_BOTT] = {
    { .index = BOARD_VERSION_HI1102,  .name = BOARD_VERSION_NAME_HI1102 },
    { .index = BOARD_VERSION_HI1103,  .name = BOARD_VERSION_NAME_HI1103 },
    { .index = BOARD_VERSION_HI1102A, .name = BOARD_VERSION_NAME_HI1102A },
    { .index = BOARD_VERSION_HI1105,  .name = BOARD_VERSION_NAME_HI1105 },
};

DOWNLOAD_MODE device_download_mode_list_etc[MODE_DOWNLOAD_BUTT] = {
    { .index = MODE_SDIO, .name = DOWNlOAD_MODE_SDIO },
    { .index = MODE_PCIE, .name = DOWNlOAD_MODE_PCIE },
    { .index = MODE_UART, .name = DOWNlOAD_MODE_UART },
};

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
int hi11xx_os_build_variant = HI1XX_OS_BUILD_VARIANT_USER; /* default user mode */
oal_debug_module_param(hi11xx_os_build_variant, int, S_IRUGO | S_IWUSR);
#endif

/* �������� */
inline BOARD_INFO *get_hi110x_board_info_etc(void)
{
    return &board_info_etc;
}

int isAsic_etc(void)
{
    if (board_info_etc.is_asic == VERSION_ASIC) {
        return VERSION_ASIC;
    } else {
        return VERSION_FPGA;
    }
}
EXPORT_SYMBOL_GPL(isAsic_etc);

int is_wifi_support(void)
{
    if (board_info_etc.is_wifi_disable == 0) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(is_wifi_support);

int is_bfgx_support(void)
{
    if (board_info_etc.is_bfgx_disable == 0) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(is_bfgx_support);

int isPmu_clk_request_enable(void)
{
    if (board_info_etc.pmu_clk_share_enable == PMU_CLK_REQ_ENABLE) {
        return PMU_CLK_REQ_ENABLE;
    } else {
        return PMU_CLK_REQ_DISABLE;
    }
}

int32 get_hi110x_subchip_type(void)
{
    BOARD_INFO *bd_info = NULL;

    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        PS_PRINT_ERR("board info is err\n");
        return -EFAIL;
    }

    return bd_info->chip_nr;
}
EXPORT_SYMBOL_GPL(get_hi110x_subchip_type);

#ifdef _PRE_CONFIG_USE_DTS
int32 get_board_dts_node_etc(struct device_node **np, const char *node_prop)
{
    if (np == NULL || node_prop == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, node_prop=%p\n", np, node_prop);
        return BOARD_FAIL;
    }

    *np = of_find_compatible_node(NULL, NULL, node_prop);
    if (*np == NULL) {
        PS_PRINT_ERR("No compatible node %s found.\n", node_prop);
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32 get_board_dts_prop_etc(struct device_node *np, const char *dts_prop, const char **prop_val)
{
    int32 ret;

    if (np == NULL || dts_prop == NULL || prop_val == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_property_read_string(np, dts_prop, prop_val);
    if (ret) {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s\n", dts_prop);
        return ret;
    }

    PS_PRINT_SUC("have get dts_prop and prop_val: %s=%s\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

int32 get_board_dts_gpio_prop_etc(struct device_node *np, const char *dts_prop, int32 *prop_val)
{
    int32 ret;

    if (np == NULL || dts_prop == NULL || prop_val == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_get_named_gpio(np, dts_prop, 0);
    if (ret < 0) {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s, ret=%d\n", dts_prop, ret);
        return ret;
    }

    *prop_val = ret;
    PS_PRINT_SUC("have get dts_prop and prop_val: %s=%d\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

#endif

int32 get_board_gpio_etc(const char *gpio_node, const char *gpio_prop, int32 *physical_gpio)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node_etc(&np, gpio_node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = get_board_dts_gpio_prop_etc(np, gpio_prop, physical_gpio);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}

int32 get_board_custmize_etc(const char *cust_node, const char *cust_prop, const char **cust_prop_val)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    if (cust_node == NULL || cust_prop == NULL || cust_prop_val == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = get_board_dts_node_etc(&np, cust_node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = get_board_dts_prop_etc(np, cust_prop, cust_prop_val);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("get board customize info %s=%s\n", cust_prop, *cust_prop_val);

    return BOARD_SUCC;
#else
    return BOARD_FAIL;
#endif
}

int32 get_board_pmu_clk32k_etc(void)
{
    return board_info_etc.bd_ops.get_board_pmu_clk32k_etc();
}

int32 set_board_pmu_clk32k_etc(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    const char *clk_name = NULL;
    struct clk *clk = NULL;
    struct device *dev = NULL;

    dev = &pdev->dev;
    clk_name = board_info_etc.clk_32k_name;
    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1102) {
        clk = devm_clk_get(dev, "clk_pmu32kb");
    } else {
        clk = devm_clk_get(dev, clk_name);
    }

    if (clk == NULL) {
        PS_PRINT_ERR("Get 32k clk %s failed!!!\n", clk_name);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                             CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK);
        return BOARD_FAIL;
    }
    board_info_etc.clk_32k = clk;

    ret = clk_prepare_enable(clk);
    if (unlikely(ret < 0)) {
        devm_clk_put(dev, clk);
        PS_PRINT_ERR("enable 32K clk %s failed!!!", clk_name);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                             CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK);
        return BOARD_FAIL;
    }
#endif
    return BOARD_SUCC;
}

int32 get_board_uart_port_etc(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.get_board_uart_port_etc();
#else
    return BOARD_SUCC;
#endif
}

int32 check_evb_or_fpga_etc(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.check_evb_or_fpga_etc();
#else
    return BOARD_SUCC;
#endif
}

int32 check_hi110x_subsystem_support(void)
{
    if (board_info_etc.bd_ops.check_hi110x_subsystem_support == NULL) {
        return BOARD_SUCC;
    }

    return board_info_etc.bd_ops.check_hi110x_subsystem_support();
}

int32 check_pmu_clk_share_etc(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.check_pmu_clk_share_etc();
#else
    return BOARD_SUCC;
#endif
}

int32 board_get_power_pinctrl_etc(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.board_get_power_pinctrl_etc(pdev);
#else
    return BOARD_SUCC;
#endif
}

int32 board_power_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.get_board_power_gpio();
}
void free_board_power_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_power_gpio_etc();
}
#ifdef HAVE_HISI_IR
void free_board_ir_gpio(void)
{
    if (board_info_etc.bfgx_ir_ctrl_gpio > -1) {
        gpio_free(board_info_etc.bfgx_ir_ctrl_gpio);
    }
}
#endif
void free_board_wakeup_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_wakeup_gpio_etc();
}

void free_board_wifi_tas_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_wifi_tas_gpio_etc();
}

int32 board_wakeup_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.board_wakeup_gpio_init_etc();
}

int32 board_wifi_tas_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.board_wifi_tas_gpio_init_etc();
}
void free_board_flowctrl_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_flowctrl_gpio_etc();
}

int32 board_flowctrl_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.board_flowctrl_gpio_init_etc();
}

void board_flowctrl_irq_init_etc(void)
{
    board_info_etc.flowctrl_irq = oal_gpio_to_irq(board_info_etc.flowctrl_gpio);
}

#ifdef HAVE_HISI_IR
int32 board_ir_ctrl_init(struct platform_device *pdev)
{
    return board_info_etc.bd_ops.board_ir_ctrl_init(pdev);
}
#endif

int32 board_gpio_init_etc(struct platform_device *pdev)
{
    int32 ret;

    /* power on gpio request */
    ret = board_power_gpio_init_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get power_on dts prop failed\n");
        goto err_get_power_on_gpio;
    }

    ret = board_wakeup_gpio_init_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get wakeup prop failed\n");
        goto oal_board_wakup_gpio_fail;
    }

    ret = board_wifi_tas_gpio_init_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get wifi tas prop failed\n");
        goto oal_board_wifi_tas_gpio_fail;
    }

#ifdef HAVE_HISI_IR
    ret = board_ir_ctrl_init(pdev);
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get ir dts prop failed\n");
        goto err_get_ir_ctrl_gpio;
    }
#endif

    return BOARD_SUCC;

#ifdef HAVE_HISI_IR
err_get_ir_ctrl_gpio:
    free_board_wifi_tas_gpio_etc();
#endif
oal_board_wifi_tas_gpio_fail:
    free_board_wakeup_gpio_etc();
oal_board_wakup_gpio_fail:
    free_board_power_gpio_etc();
err_get_power_on_gpio:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                         CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO);
    return BOARD_FAIL;
}

int board_get_bwkup_gpio_val_etc(void)
{
    return gpio_get_value(board_info_etc.bfgn_wakeup_host);
}

int board_get_wlan_wkup_gpio_val_etc(void)
{
    return gpio_get_value(board_info_etc.wlan_wakeup_host);
}

int32 board_irq_init_etc(void)
{
    uint32 irq;
    int32 gpio;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    gpio = board_info_etc.wlan_wakeup_host;
    irq = gpio_to_irq(gpio);
    board_info_etc.wlan_irq = irq;

    PS_PRINT_INFO("wlan_irq is %d\n", board_info_etc.wlan_irq);
#endif

    gpio = board_info_etc.bfgn_wakeup_host;
    irq = gpio_to_irq(gpio);
    board_info_etc.bfgx_irq = irq;

    PS_PRINT_INFO("bfgx_irq is %d\n", board_info_etc.bfgx_irq);

    return BOARD_SUCC;
}

int32 board_clk_init_etc(struct platform_device *pdev)
{
    int32 ret;

    if (pdev == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = board_info_etc.bd_ops.get_board_pmu_clk32k_etc();
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = set_board_pmu_clk32k_etc(pdev);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

void power_state_change_etc(int32 gpio, int32 flag)
{
    if (unlikely(gpio == 0)) {
        PS_PRINT_WARNING("gpio is 0, flag=%d\n", flag);
        return;
    }

    PS_PRINT_INFO("power_state_change_etc gpio %d to %s\n", gpio, (flag == BOARD_POWER_ON) ? "low2high" : "low");

    if (flag == BOARD_POWER_ON) {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
        mdelay(10);
        gpio_direction_output(gpio, GPIO_HIGHLEVEL);
        mdelay(20);
    } else if (flag == BOARD_POWER_OFF) {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
    }
}

int32 board_wlan_gpio_power_on(void *data)
{
    int32 gpio = (int32)(uintptr_t)(data);
    if (board_info_etc.host_wakeup_wlan) {
        /* host wakeup dev gpio pinmux to jtag when w boot,
          must gpio low when bootup */
        board_host_wakeup_dev_set(0);
    }
    power_state_change_etc(gpio, BOARD_POWER_ON);
    board_host_wakeup_dev_set(1);
    return 0;
}

int32 board_wlan_gpio_power_off(void *data)
{
    int32 gpio = (int32)(uintptr_t)(data);
    power_state_change_etc(gpio, BOARD_POWER_OFF);
    return 0;
}

int32 board_host_wakeup_dev_set(int value)
{
    if (board_info_etc.host_wakeup_wlan == 0) {
        PS_PRINT_INFO("host_wakeup_wlan gpio is 0\n");
        return 0;
    }
    PS_PRINT_DBG("host_wakeup_wlan set %s %pF\n", value ? "high" : "low", (void *)_RET_IP_);
    if (value) {
        return gpio_direction_output(board_info_etc.host_wakeup_wlan, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(board_info_etc.host_wakeup_wlan, GPIO_LOWLEVEL);
    }
}

int32 board_host_wakeup_bfg_set(int value)
{
    if (board_info_etc.host_wakeup_bfg == 0) {
        PS_PRINT_INFO("host_wakeup_bfg gpio is 0\n");
        return 0;
    }

    if (value) {
        return gpio_direction_output(board_info_etc.host_wakeup_bfg, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(board_info_etc.host_wakeup_bfg, GPIO_LOWLEVEL);
    }
}

int32 board_get_host_wakeup_dev_stat(void)
{
    return gpio_get_value(board_info_etc.host_wakeup_wlan);
}

int32 board_wifi_tas_set(int value)
{
    if (board_info_etc.wifi_tas_enable == WIFI_TAS_DISABLE) {
        return 0;
    }

    PS_PRINT_DBG("wifi tas gpio set %s %pF\n", value ? "high" : "low", (void *)_RET_IP_);

    if (value) {
        return gpio_direction_output(board_info_etc.rf_wifi_tas, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(board_info_etc.rf_wifi_tas, GPIO_LOWLEVEL);
    }
}

EXPORT_SYMBOL(board_wifi_tas_set);

int32 board_get_wifi_tas_gpio_state(void)
{
    return gpio_get_value(board_info_etc.rf_wifi_tas);
}

EXPORT_SYMBOL(board_get_wifi_tas_gpio_state);

int32 board_power_on_etc(uint32 ul_subsystem)
{
    return board_info_etc.bd_ops.board_power_on_etc(ul_subsystem);
}
int32 board_power_off_etc(uint32 ul_subsystem)
{
    return board_info_etc.bd_ops.board_power_off_etc(ul_subsystem);
}

int32 board_power_reset(uint32 ul_subsystem)
{
    return board_info_etc.bd_ops.board_power_reset(ul_subsystem);
}
EXPORT_SYMBOL(board_wlan_gpio_power_off);
EXPORT_SYMBOL(board_wlan_gpio_power_on);

/* just for hi1105 fpga test, delete later */
void hi1105_subchip_check_tmp(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node_etc(&np, DTS_NODE_HISI_HI110X);
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("DTS read node %s fail!!!\n", DTS_NODE_HISI_HI110X);
        return;
    }

    ret = of_property_read_bool(np, BOARD_VERSION_NAME_HI1105);
    if (ret) {
        PS_PRINT_INFO("hisi subchip type is hi1105\n");
        board_info_etc.chip_type = BOARD_VERSION_NAME_HI1105;
    }

    return;
#else
    return;
#endif
}

int32 find_device_board_version_etc(void)
{
    int32 ret;
    const char *device_version = NULL;

    ret = get_board_custmize_etc(DTS_NODE_HISI_HI110X, DTS_PROP_SUBCHIP_TYPE_VERSION, &device_version);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    board_info_etc.chip_type = device_version;
    return BOARD_SUCC;
}

int32 board_chiptype_init(void)
{
    int32 ret;

    ret = find_device_board_version_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("can not find device_board_version\n");
        return BOARD_FAIL;
    }

    /* just for hi1105 fpga test, delete later */
    hi1105_subchip_check_tmp();

    ret = check_device_board_name_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("check device name fail\n");
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32 board_func_init(void)
{
    int32 ret;
    // board init
    memset_s(&board_info_etc, sizeof(board_info_etc), 0, sizeof(board_info_etc));

    board_info_etc.is_wifi_disable = 0;
    board_info_etc.is_bfgx_disable = 0;

    ret = board_chiptype_init();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("sub chiptype init fail\n");
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("hi11xx subchip is %s\n", board_info_etc.chip_type);

    switch (board_info_etc.chip_nr) {
        case BOARD_VERSION_HI1102:
            board_info_etc.bd_ops.get_board_power_gpio = hi1102_get_board_power_gpio;
            board_info_etc.bd_ops.free_board_power_gpio_etc = hi1102_free_board_power_gpio_etc;
            board_info_etc.bd_ops.board_wakeup_gpio_init_etc = hi1102_board_wakeup_gpio_init_etc;
            board_info_etc.bd_ops.free_board_wakeup_gpio_etc = hi1102_free_board_wakeup_gpio_etc;
            board_info_etc.bd_ops.bfgx_dev_power_on_etc = hi1102_bfgx_dev_power_on;
            board_info_etc.bd_ops.bfgx_dev_power_off_etc = hi1102_bfgx_dev_power_off;
            board_info_etc.bd_ops.wlan_power_off_etc = hi1102_wlan_power_off;
            board_info_etc.bd_ops.wlan_power_on_etc = hi1102_wlan_power_on;
            board_info_etc.bd_ops.board_power_on_etc = hi1102_board_power_on;
            board_info_etc.bd_ops.board_power_off_etc = hi1102_board_power_off;
            board_info_etc.bd_ops.board_power_reset = hi1102_board_power_reset;
            board_info_etc.bd_ops.get_board_pmu_clk32k_etc = hi1102_get_board_pmu_clk32k;
            board_info_etc.bd_ops.get_board_uart_port_etc = hi1102_get_board_uart_port;
            board_info_etc.bd_ops.board_ir_ctrl_init = hi1102_board_ir_ctrl_init;
            board_info_etc.bd_ops.check_evb_or_fpga_etc = hi1102_check_evb_or_fpga;
            board_info_etc.bd_ops.board_get_power_pinctrl_etc = hi1102_board_get_power_pinctrl;
            board_info_etc.bd_ops.get_ini_file_name_from_dts_etc = hi1102_get_ini_file_name_from_dts;
            break;
        case BOARD_VERSION_HI1103:
        case BOARD_VERSION_HI1105:
            board_info_etc.bd_ops.get_board_power_gpio = hi1103_get_board_power_gpio;
            board_info_etc.bd_ops.free_board_power_gpio_etc = hi1103_free_board_power_gpio_etc;
            board_info_etc.bd_ops.free_board_flowctrl_gpio_etc = hi1103_free_board_flowctrl_gpio_etc;
            board_info_etc.bd_ops.board_wakeup_gpio_init_etc = hi1103_board_wakeup_gpio_init_etc;
            board_info_etc.bd_ops.free_board_wakeup_gpio_etc = hi1103_free_board_wakeup_gpio_etc;
            board_info_etc.bd_ops.board_flowctrl_gpio_init_etc = hi1103_board_flowctrl_gpio_init_etc;
            board_info_etc.bd_ops.board_wifi_tas_gpio_init_etc = hi1103_board_wifi_tas_gpio_init_etc;
            board_info_etc.bd_ops.free_board_wifi_tas_gpio_etc = hi1103_free_board_wifi_tas_gpio_etc;
            board_info_etc.bd_ops.bfgx_dev_power_on_etc = hi1103_bfgx_dev_power_on;
            board_info_etc.bd_ops.bfgx_dev_power_off_etc = hi1103_bfgx_dev_power_off;
            board_info_etc.bd_ops.wlan_power_off_etc = hi1103_wlan_power_off;
            board_info_etc.bd_ops.wlan_power_on_etc = hi1103_wlan_power_on;
            board_info_etc.bd_ops.board_power_on_etc = hi1103_board_power_on;
            board_info_etc.bd_ops.board_power_off_etc = hi1103_board_power_off;
            board_info_etc.bd_ops.board_power_reset = hi1103_board_power_reset;
            board_info_etc.bd_ops.get_board_pmu_clk32k_etc = hi1103_get_board_pmu_clk32k;
            board_info_etc.bd_ops.get_board_uart_port_etc = hi1103_get_board_uart_port;
            board_info_etc.bd_ops.board_ir_ctrl_init = hi1103_board_ir_ctrl_init;
            board_info_etc.bd_ops.check_evb_or_fpga_etc = hi1103_check_evb_or_fpga;
            board_info_etc.bd_ops.check_pmu_clk_share_etc = hi1103_check_pmu_clk_share;
            board_info_etc.bd_ops.board_get_power_pinctrl_etc = hi1103_board_get_power_pinctrl;
            board_info_etc.bd_ops.get_ini_file_name_from_dts_etc = hi1103_get_ini_file_name_from_dts;
            board_info_etc.bd_ops.check_hi110x_subsystem_support = hi1103_check_hi110x_subsystem_support;
            break;
        default:
            PS_PRINT_ERR("board_info_etc.chip_nr=%d is illegal\n", board_info_etc.chip_nr);
            return BOARD_FAIL;
    }

    PS_PRINT_INFO("board_info_etc.chip_nr=%d, device_board_version is %s\n",
                  board_info_etc.chip_nr, board_info_etc.chip_type);
    return BOARD_SUCC;
}

int32 check_download_channel_name_etc(const char *wlan_buff, int32 *index)
{
    int32 i = 0;
    for (i = 0; i < MODE_DOWNLOAD_BUTT; i++) {
        if (strncmp(device_download_mode_list_etc[i].name, wlan_buff, strlen(device_download_mode_list_etc[i].name)) ==
            0) {
            *index = i;
            return BOARD_SUCC;
        }
    }
    return BOARD_FAIL;
}

int32 get_download_channel_etc(void)
{
    int32 ret;
    uint8 wlan_mode[DOWNLOAD_CHANNEL_LEN] = {0};
    uint8 bfgn_mode[DOWNLOAD_CHANNEL_LEN] = {0};

    /* wlan channel */
    ret = find_download_channel_etc(wlan_mode, sizeof(wlan_mode), INI_WLAN_DOWNLOAD_CHANNEL);
    if (ret != BOARD_SUCC) {
        /* ����1102,1102�޴������� */
        board_info_etc.wlan_download_channel = MODE_SDIO;
        PS_PRINT_WARNING("can not find wlan_download_channel ,choose default:%s\n",
                         device_download_mode_list_etc[0].name);
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, NULL);
    } else {
        if (check_download_channel_name_etc(wlan_mode, &(board_info_etc.wlan_download_channel)) != BOARD_SUCC) {
            PS_PRINT_ERR("check wlan download channel:%s error\n", bfgn_mode);
            return BOARD_FAIL;
        }
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, wlan_mode);
    }

    /* bfgn channel */
    ret = find_download_channel_etc(bfgn_mode, sizeof(bfgn_mode), INI_BFGX_DOWNLOAD_CHANNEL);
    if (ret != BOARD_SUCC) {
        /* ��������ڸ����Ĭ�ϱ��ֺ�wlanһ�� */
        board_info_etc.bfgn_download_channel = board_info_etc.wlan_download_channel;
        PS_PRINT_WARNING("can not find bfgn_download_channel ,choose default:%s\n",
                         device_download_mode_list_etc[0].name);
        return BOARD_SUCC;
    }

    if (check_download_channel_name_etc(bfgn_mode, &(board_info_etc.bfgn_download_channel)) != BOARD_SUCC) {
        PS_PRINT_ERR("check bfgn download channel:%s error\n", bfgn_mode);
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("wlan_download_channel index:%d, bfgn_download_channel index:%d\n",
                  board_info_etc.wlan_download_channel, board_info_etc.bfgn_download_channel);

    return BOARD_SUCC;
}

uint32 ssi_dump_en = 0;
int32 get_ssi_dump_cfg(void)
{
    int32 l_cfg_value = 0;
    int32 l_ret;

    /* ��ȡini������ֵ */
    l_ret = get_cust_conf_int32_etc(INI_MODU_PLAT, INI_SSI_DUMP_EN, &l_cfg_value);

    if (l_ret == INI_FAILED) {
        PS_PRINT_ERR("get_ssi_dump_cfg: fail to get ini, keep disable\n");
        return BOARD_SUCC;
    }

    ssi_dump_en = (uint32)l_cfg_value;

    PS_PRINT_INFO("get_ssi_dump_cfg: 0x%x\n", ssi_dump_en);

    return BOARD_SUCC;
}

int32 check_device_board_name_etc(void)
{
    int32 i = 0;
    for (i = 0; i < BOARD_VERSION_BOTT; i++) {
        if (strncmp(device_board_version_list_etc[i].name, board_info_etc.chip_type, HI11XX_SUBCHIP_NAME_LEN_MAX) ==
            0) {
            board_info_etc.chip_nr = i;
            return BOARD_SUCC;
        }
    }

    return BOARD_FAIL;
}

int32 get_uart_pclk_source_etc(void)
{
    return board_info_etc.uart_pclk;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
STATIC void board_cci_bypass_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    if (pcie_memcopy_type == -1) {
        PS_PRINT_INFO("skip pcie mem burst control\n");
        return;
    }

    ret = get_board_dts_node_etc(&np, DTS_NODE_HISI_CCIBYPASS);
    if (ret != BOARD_SUCC) {
        /* cci enable */
        pcie_memcopy_type = 0;
        PS_PRINT_INFO("cci enable, pcie use mem burst 8 bytes\n");
    } else {
        /* cci bypass */
        pcie_memcopy_type = 1;
        PS_PRINT_INFO("cci bypass, pcie use mem burst 4 bytes\n");
    }
#endif
}
#endif

int32 buck_param_init_by_ini(oal_void)
{
    int32 l_cfg_value = 0;
    int32 l_ret;

    /* ��ȡini������ֵ */
    l_ret = get_cust_conf_int32_etc(INI_MODU_PLAT, "buck_mode", &l_cfg_value);

    if (l_ret == INI_FAILED) {
        PS_PRINT_ERR("get_ssi_dump_cfg: fail to get ini, keep disable\n");
        return BOARD_FAIL;
    }

    board_info_etc.buck_param = (uint16)l_cfg_value;

    PS_PRINT_INFO("buck_param_init_by_ini: 0x%x\n",  board_info_etc.buck_param);

    return BOARD_SUCC;

}

/*
*  �� �� ��  : buck_param_init
*  ��������  : �Ӷ��ƻ�ini�ļ�����ȡbuckģʽ�����ò�����
*   110x֧������buck������buck,firmware CFG�ļ��н��йؼ��֣�ʵ��ֵ��ini�ļ��л�ȡ�����㶨�ƻ���
*   ini���ƻ���ʽ[buck_mode=2,BUCK_CUSTOM_REG,value],����CFG�ļ���WRITEM���﷨,0x50001850Ϊ1105��BUCK_CUSTOM_REG
*   ��ַ, value����ʵ��Ҫ������:
*   ��1��   BIT[15,14]����ʾ�Ƿ��������buck
*           2'b00:  ȫ����buck
*           2'b01:  I2C���ƶ�������buck
*           2'b10:  GPIO���ƶ�������buck
*           2'b11:  host���ƹ�������buck��ѹ
*   ��2��   BIT[8]����ʾ�ȵ���ѹ������buck_en.
*   ��3��   BIT[7��0]: ����ͬ��Buck�����ͺ�
*/
int32 buck_param_init(oal_void)
{
#ifdef _PRE_CONFIG_USE_DTS
        int32 ret = BOARD_FAIL;
        struct device_node *np = NULL;
        int32 buck_mode = 0;

        ret = get_board_dts_node_etc(&np, DTS_NODE_HISI_HI110X);
        if (ret != BOARD_SUCC) {
            PS_PRINT_ERR("DTS read node %s fail!!!\n", DTS_NODE_HISI_HI110X);
            return  buck_param_init_by_ini();
        }

        ret = of_property_read_u32(np, DTS_PROP_HI110x_BUCK_MODE, &buck_mode);
        if (ret == BOARD_SUCC) {
            PS_PRINT_INFO("buck_param_init get dts config:0x%x\n", buck_mode);
            board_info_etc.buck_param = (uint16)buck_mode;
        } else {
            PS_PRINT_ERR("buck_param_init fail,get from ini\n");
            return  buck_param_init_by_ini();
        }

        PS_PRINT_INFO("buck_param_init success\n");
        return BOARD_SUCC;
#else
        return  buck_param_init_by_ini();
#endif

}

/*
*  �� �� ��  : buck_mode_get
*  ��������  : ��ȡbuck����
*  ����ֵ:
*  0:  ȫ����buck
*  1:  I2C���ƶ�������buck
*  2:  GPIO���ƶ�������buck
*  3:  host���ƹ�������buck��ѹ
*/
uint8 buck_mode_get(void)
{
   return  (((board_info_etc.buck_param) & BUCK_MODE_MASK)>>14);
}

STATIC int32 hi110x_board_probe(struct platform_device *pdev)
{
    int ret;

    PS_PRINT_INFO("hi110x board init\n");
    ret = board_func_init();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = ini_cfg_init_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }
    ret = check_hi110x_subsystem_support();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = check_evb_or_fpga_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = check_pmu_clk_share_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = get_download_channel_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = get_ssi_dump_cfg();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_clk_init_etc(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    board_cci_bypass_init();
#endif

    ret = get_board_uart_port_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_gpio_init_etc(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_irq_init_etc();
    if (ret != BOARD_SUCC) {
        goto err_gpio_source;
    }

    ret = board_get_power_pinctrl_etc(pdev);
    if (ret != BOARD_SUCC) {
        goto err_get_power_pinctrl;
    }

    buck_param_init();

    PS_PRINT_INFO("board init ok\n");

    board_probe_ret = BOARD_SUCC;
    complete(&board_driver_complete);

    return BOARD_SUCC;

err_get_power_pinctrl:

err_gpio_source:
#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif
    free_board_wakeup_gpio_etc();
    free_board_power_gpio_etc();
    free_board_flowctrl_gpio_etc();

err_init:
    board_probe_ret = BOARD_FAIL;
    complete(&board_driver_complete);
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                         CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_POWER_GPIO);
    return BOARD_FAIL;
}

STATIC int32 hi110x_board_remove(struct platform_device *pdev)
{
    PS_PRINT_INFO("hi110x board exit\n");

#ifdef _PRE_CONFIG_USE_DTS
    if (board_info_etc.need_power_prepare == NEED_POWER_PREPARE) {
        devm_pinctrl_put(board_info_etc.pctrl);
    }
#endif

#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif

    free_board_wakeup_gpio_etc();
    free_board_power_gpio_etc();
    free_board_flowctrl_gpio_etc();

    return BOARD_SUCC;
}

int32 hi110x_board_suspend_etc(struct platform_device *pdev, pm_message_t state)
{
    return BOARD_SUCC;
}

int32 hi110x_board_resume_etc(struct platform_device *pdev)
{
    return BOARD_SUCC;
}

#ifdef _PRE_CONFIG_USE_DTS
static struct of_device_id hi110x_board_match_table[] = {
    {
        .compatible = DTS_COMP_HI110X_BOARD_NAME,
        .data = NULL,
    },
    {
        .compatible = DTS_COMP_HISI_HI110X_BOARD_NAME,
        .data = NULL,
    },
    {},
};
#endif

STATIC struct platform_driver hi110x_board_driver = {
    .probe = hi110x_board_probe,
    .remove = hi110x_board_remove,
    .suspend = hi110x_board_suspend_etc,
    .resume = hi110x_board_resume_etc,
    .driver = {
        .name = "hi110x_board",
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hi110x_board_match_table,
#endif
    },
};

int32 hi110x_board_init_etc(void)
{
    int32 ret;

    board_probe_ret = BOARD_FAIL;
    init_completion(&board_driver_complete);

#ifdef OS_HI1XX_BUILD_VERSION
    hi11xx_os_build_variant = OS_HI1XX_BUILD_VERSION;
    PS_PRINT_INFO("hi11xx_os_build_variant=%d\n", hi11xx_os_build_variant);
#endif

    ret = platform_driver_register(&hi110x_board_driver);
    if (ret) {
        PS_PRINT_ERR("Unable to register hisi connectivity board driver.\n");
        return ret;
    }

    if (wait_for_completion_timeout(&board_driver_complete, 10 * HZ)) {
        /* completed */
        if (board_probe_ret != BOARD_SUCC) {
            PS_PRINT_ERR("hi110x_board probe failed=%d\n", board_probe_ret);
            return board_probe_ret;
        }
    } else {
        /* timeout */
        PS_PRINT_ERR("hi110x_board probe timeout\n");
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("hi110x_board probe succ\n");

    return ret;
}

void hi110x_board_exit_etc(void)
{
    platform_driver_unregister(&hi110x_board_driver);
}

oal_uint8 hi110x_bus_type = HCC_BUS_SDIO;
oal_int32 wifi_plat_dev_probe_state;

static int hisi_wifi_plat_dev_drv_probe(struct platform_device *pdev)
{
    int ret = 0;
    if (hi110x_bus_type == HCC_BUS_SDIO) {
        ret = oal_wifi_platform_load_sdio();
        if (ret) {
            printk(KERN_ERR "[HW_CONN] oal_wifi_platform_load_sdio failed.\n");
            wifi_plat_dev_probe_state = -OAL_FAIL;
            return ret;
        }
    }

#ifdef CONFIG_HWCONNECTIVITY
    ret = create_hwconn_proc_file();
    if (ret) {
        printk(KERN_ERR "[HW_CONN] create proc file failed.\n");
        wifi_plat_dev_probe_state = -OAL_FAIL;
        return ret;
    }
#endif
    return ret;
}

static int hisi_wifi_plat_dev_drv_remove(struct platform_device *pdev)
{
    printk(KERN_ERR "[HW_CONN] hisi_wifi_plat_dev_drv_remove.\n");
    return OAL_SUCC;
}

#ifdef _PRE_CONFIG_USE_DTS
static const struct of_device_id hisi_wifi_match_table[] = {
    {
        .compatible = DTS_NODE_HI110X_WIFI,  // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};
#endif

static struct platform_driver hisi_wifi_platform_dev_driver = {
    .probe = hisi_wifi_plat_dev_drv_probe,
    .remove = hisi_wifi_plat_dev_drv_remove,
    .suspend = NULL,
    .shutdown = NULL,
    .driver = {
        .name = DTS_NODE_HI110X_WIFI,
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hisi_wifi_match_table,  // dts required code
#endif
    },
};

int32 hi11xx_get_os_build_variant(void)
{
    return hi11xx_os_build_variant;
}
EXPORT_SYMBOL(hi11xx_get_os_build_variant);

int32 hisi_wifi_platform_register_drv(void)
{
    int32 ret;
    PS_PRINT_FUNCTION_NAME;

    wifi_plat_dev_probe_state = OAL_SUCC;

    ret = platform_driver_register(&hisi_wifi_platform_dev_driver);
    if (ret) {
        PS_PRINT_ERR("Unable to register hisi wifi driver.\n");
    }
    /* platform_driver_register return always true */
    return wifi_plat_dev_probe_state;
}

void hisi_wifi_platform_unregister_drv(void)
{
    PS_PRINT_FUNCTION_NAME;

    platform_driver_unregister(&hisi_wifi_platform_dev_driver);

    return;
}
