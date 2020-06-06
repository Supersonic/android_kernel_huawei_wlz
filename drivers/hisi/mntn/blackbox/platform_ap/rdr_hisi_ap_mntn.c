#include <linux/fs.h>
#include <linux/io.h>
#include <linux/threads.h>
#include <linux/kallsyms.h>
#include <linux/reboot.h>
#include <linux/input.h>
#include <linux/syscalls.h>
#include <linux/of.h>

#include <asm/ptrace.h>
#include <securec.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/hisi_powerkey_event.h>
#include "../rdr_print.h"
#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG HISI_BLACKBOX_TAG
#define DTS_POWERKEY_ONLY_STATUS_NAME  "hisi,powerhold"
#define POWERKEY_ONLY_PRODUCT 0x55aa
#ifdef CONFIG_HISI_BB
static u32 reboot_reason_flag;
#endif

#ifdef CONFIG_HUAWEI_BFM
#include <chipset_common/bfmr/bfm/chipsets/bfm_chipsets.h>
#endif



static int g_powerkey_only_status;

static int hisi_pmic_powerkey_only_flag(void)
{
	return g_powerkey_only_status;
}


void hisi_pmic_powerkey_only_status_get(void)
{
	const char *press_to_fastboot_status = NULL;
	struct device_node *np = NULL;
	int ret = 0;

	np = of_find_compatible_node(NULL, NULL, DTS_POWERKEY_ONLY_STATUS_NAME);
	if (!np) {
		BB_PRINT_ERR("%s: dts node(powerhold) not found\n", __func__);
		return;
	}
	ret = of_property_read_string(np, "press_6s_to_fastboot", &press_to_fastboot_status);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find powerkey_only_product in dts!\n", __func__);
		g_powerkey_only_status = 0;
		goto err;
	}
	if (strncmp("ok", press_to_fastboot_status, sizeof("ok")) == 0) {
		g_powerkey_only_status = POWERKEY_ONLY_PRODUCT;
	} else {
		g_powerkey_only_status = 0;
		BB_PRINT_ERR("[%s], press_6s_to_fastboot status is not ok!\n", __func__);
	}
err:
	of_node_put(np);
}

int rdr_press_key_to_fastboot(struct notifier_block *nb,
		unsigned long event, void *buf)
{
	if (hisi_pmic_powerkey_only_flag() == POWERKEY_ONLY_PRODUCT) {
		if (event != HISI_PRESS_KEY_6S)
			return -1;
	} else {
		if (event != HISI_PRESS_KEY_UP)
			return -1;
	}
	if (check_himntn(HIMNTN_PRESS_KEY_TO_FASTBOOT)) {
#ifdef CONFIG_KEYBOARD_HISI_GPIO_KEY
		if ((VOL_UPDOWN_PRESS & (unsigned int)gpio_key_vol_updown_press_get())
				== VOL_UPDOWN_PRESS) {
			gpio_key_vol_updown_press_set_zero();
			if (is_gpio_key_vol_updown_pressed()) {
				BB_PRINT_PN("[%s]Powerkey+VolUp_key+VolDn_key\n", __func__);
#ifdef CONFIG_HISI_BB
				rdr_syserr_process_for_ap(MODID_AP_S_COMBINATIONKEY, 0, 0);
#endif
			}
		}
#endif

#ifdef CONFIG_KEYBOARD_HISI_PMIC_GPIO_KEY
		if ((VOL_UPDOWN_PRESS & (unsigned int)pmic_gpio_key_vol_updown_press_get())
				== VOL_UPDOWN_PRESS) {
			pmic_gpio_key_vol_updown_press_set_zero();
			if (is_pmic_gpio_key_vol_updown_pressed()) {
				BB_PRINT_PN("[%s]pmic Powerkey+VolUp_key+VolDn_key\n", __func__);
#ifdef CONFIG_HISI_BB
				rdr_syserr_process_for_ap(MODID_AP_S_COMBINATIONKEY, 0, 0);
#endif
			}
		}
		if (event == HISI_PRESS_KEY_6S) {
			BB_PRINT_PN("[%s]pmic Powerkey 6s\n", __func__);
#ifdef CONFIG_HISI_BB
			rdr_syserr_process_for_ap(MODID_AP_S_COMBINATIONKEY, 0, 0);
			return 0;
#endif
		}
#endif
	}
#ifdef CONFIG_HISI_BB
	if (get_reboot_reason() == AP_S_PRESS6S)
		set_reboot_reason(reboot_reason_flag);
#endif
	return 0;
}

void rdr_long_press_powerkey(void)
{
	u32 reboot_reason = get_reboot_reason();

	set_reboot_reason(AP_S_PRESS6S);
	if (get_boot_keypoint() != STAGE_BOOTUP_END) {
		BB_PRINT_PN("press6s in boot\n");
#ifdef CONFIG_HUAWEI_BFM
		bfm_set_valid_long_press_flag();
#endif
		save_log_to_dfx_tempbuffer(AP_S_PRESS6S);
		sys_sync();
	} else {
		reboot_reason_flag = reboot_reason;
		BB_PRINT_PN("press6s:reboot_reason_flag=%u\n", reboot_reason_flag);
	}
}
