#include <linux/version.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/pm_opp.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <hisi_npu_pm.h>
#include <linux/jiffies.h>
#include "hisi_npu_pm_private.h"

#define HISI_AVS_NPU_TSENSOR_NAME  "npu"
#define SLICE_TO_US 			32
#define AVS_INTERVAL 			60000UL
#ifdef CONFIG_HISI_THERMAL_TSENSOR
extern int ipa_get_tsensor_id(const char *name);
extern int ipa_get_sensor_value(u32 sensor, int *val);
#endif

static int hisi_npu_avs_tsensor = 0;
static unsigned int g_avs_tzone_idx = 1;
static unsigned int g_npu_cur_svfd_threshold = 0;
extern struct hisi_npu_dvfs_data g_dvfs_data;

static unsigned int g_pa_val = 2;
static unsigned long g_last_pa_time = 0;
/*
 * 0:base_volt
 * 1:step_volt
 */
static unsigned int  g_npu_avsvolt[2] = {0};

unsigned int get_avs_tzone_idx(int temp)
{
	if (temp <= (int)(g_dvfs_data.main_module->avs_temp[1]))
		return 0;
	else if ((temp > (int)(g_dvfs_data.main_module->avs_temp[1])) && (temp <= (int)(g_dvfs_data.main_module->avs_temp[2])))
		return 1;
	else if ((temp > (int)(g_dvfs_data.main_module->avs_temp[2])) && (temp <= (int)(g_dvfs_data.main_module->avs_temp[3])))
		return 2;
	else
		return 3;
}

unsigned int npu_volt_cfg2vol(unsigned int vol_cfg)
{
	return (g_npu_avsvolt[0] + g_npu_avsvolt[1] * vol_cfg) * 1000;
}

unsigned int update_avs_temperature_info(void)
{
#ifdef CONFIG_HISI_THERMAL_TSENSOR
	int temp = 0, ret;

	ret = ipa_get_sensor_value(hisi_npu_avs_tsensor, &temp);
	/*get temp fail use last temp avs config*/
	if (ret)
		return g_avs_tzone_idx;
	g_avs_tzone_idx = get_avs_tzone_idx(temp);
#endif
	return g_avs_tzone_idx;
}

static int check_avs_para_update(void)
{
	unsigned long now, tmp;
	int ret = 0;

	now = jiffies64_to_nsecs(get_jiffies_64());
	/* change us to ms, so we divide 1000 */
	now = div_u64(now, NSEC_PER_MSEC);
	tmp = (now - g_last_pa_time);
	if (tmp > AVS_INTERVAL) {
		g_last_pa_time = now;
		ret = 1;
	}
	return ret;
}

noinline int atfd_hisi_service_avs_smc(u64 function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 funcid asm("x0") = function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;

	asm volatile (
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc	#0\n"
		: "+r" (funcid)
		: "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)function_id;
}


int npu_pasensor_mode_cfg(void)
{
	int ret;

	ret = atfd_hisi_service_avs_smc(HISI_AVS_NPU_PA_MODE_CFG, 0ULL, 0ULL, 0ULL);

	return ret;
}

static int npu_pasensor_mode_recovery(void)
{
	int ret;

	ret = atfd_hisi_service_avs_smc(HISI_AVS_NPU_PA_MODE_RECOVERY, 0ULL, 0ULL, 0ULL);

	return ret;
}



static int npu_read_pasensor_code(struct hisi_npu_module_profile *module_profile)
{
	int ret;

	ret = atfd_hisi_service_avs_smc(HISI_AVS_NPU_GET_PA_VALUE,
									module_profile->pa_ctrl.pa_margin[0],
									module_profile->pa_ctrl.pa_margin[1],
									0ULL);

	return ret;

}

unsigned int npu_cluster_avs(struct hisi_npu_module_profile *module_profile, unsigned int open_switch)
{
	int ret;

	if (module_profile == NULL) {
		/* this abnormal branch, return default code for safe, but in this branch we return 0 */
		return 0;
	}

	if (open_switch == 0) {
		goto default_val;
	}

	/* pasensor code update period is define in dts config */
	ret = check_avs_para_update();
	if (ret == 0) {
		goto default_val;
	}

	/* enable target pll */
	ret = clk_prepare_enable(module_profile->pa_ctrl.pll_clk);
	if (ret != 0) {
		pr_err("[npu avs] %s %d clk enable fail%d\n", __func__, __LINE__, ret);
		goto default_val;
	}

	ret = npu_pasensor_mode_cfg();
	if (ret != 0) {
		/* if read pasensor failed, return last pa data */
		pr_err("[npu avs] %s %d pasensor mode cfg fail%d\n", __func__, __LINE__, ret);
		goto out;
	}
	g_pa_val = npu_read_pasensor_code(module_profile);
	if (g_pa_val > module_profile->pa_ctrl.pa_max_code) {
		/* g_pa_val <= pa_max code */
		g_pa_val = module_profile->pa_ctrl.pa_max_code;
	}
	ret = npu_pasensor_mode_recovery();
	if (ret != 0) {
		/* if read pasensor failed, return last pa data */
		pr_err("[npu avs] %s %d pasensor mode revocery fail%d\n", __func__, __LINE__, ret);
		goto out;
	}
out:
	/* disable source pll */
	clk_disable_unprepare(module_profile->pa_ctrl.pll_clk);
default_val:
	/* pa_ctrl.pa_mode_para[1] is pa code step volt */
	return (g_pa_val * module_profile->pa_ctrl.pa_step_volt * 1000);
}

void npu_cpm_ffs_reset_switch(struct hisi_npu_module_profile *module_profile, unsigned int reset)
{
	unsigned int tmp;

	tmp = BIT(module_profile->svfd_ctrl.vdm_cfg_bit[2]) | BIT(module_profile->svfd_ctrl.vdm_cfg_bit[3]);
	if (reset) {
		/*cpm ffs reset*/
		writel(tmp, (module_profile->svfd_ctrl.vs_crg_base + module_profile->svfd_ctrl.vdm_reg_offset[3]));

	} else {
		/*cpm ffs disset*/
		writel(tmp, (module_profile->svfd_ctrl.vs_crg_base + module_profile->svfd_ctrl.vdm_reg_offset[4]));
	}
}


void hisi_npu_svfd_switch(struct hisi_npu_module_profile *module_profile, unsigned int target, unsigned int onoff)
{
	unsigned int tmp, timeout, ctrl_mask = 0;
	unsigned int func_sw;

	func_sw = module_profile->profile_table[target].func_sw;

	if (!(func_sw & (PROF_SVFD | PROF_ACTIVE_DEFENCE)) || !onoff) {
		/*readl CLUSTER_SVFD_CTRL2 to disable vdm&cpm*/
		tmp = readl(module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[11]);
		tmp = (tmp & (~(BIT(module_profile->svfd_ctrl.vdm_cfg_bit[4]) | BIT(module_profile->svfd_ctrl.vdm_cfg_bit[5]))))  \
				| ((BIT(module_profile->svfd_ctrl.vdm_cfg_bit[4]) | BIT(module_profile->svfd_ctrl.vdm_cfg_bit[5])) << 16);
		writel(tmp, (module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[11]));
		udelay(1);
		/* cpm & ffs reset */
		npu_cpm_ffs_reset_switch(module_profile, 1);
		return;
	}

	/* disreset vsensor */
	tmp = readl(module_profile->svfd_ctrl.vs_crg_base + module_profile->svfd_ctrl.vdm_reg_offset[2]);
	if (tmp & BIT(module_profile->svfd_ctrl.vdm_cfg_bit[0]))
		writel(BIT(module_profile->svfd_ctrl.vdm_cfg_bit[0]), (module_profile->svfd_ctrl.vs_crg_base + module_profile->svfd_ctrl.vdm_reg_offset[1]));

	/*cpm & svfd disreset*/
	npu_cpm_ffs_reset_switch(module_profile, 0);

	timeout = 3;
	while (!((unsigned int)readl(module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[12]) & BIT(16)) && (timeout != 0)) {
		udelay(1);
		timeout--;
	}
	if (0 == timeout) {
		pr_err("[npu dvfs] npusvfd DLL lock OT ,target %d\n", target);
		return;
	}
	/* clear cmp data */
	tmp = readl(module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[9]);
	tmp = tmp & (~BIT(module_profile->svfd_ctrl.vdm_cfg_bit[6]));
	writel(tmp, (module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[9]));
	if (func_sw & PROF_SVFD)
		ctrl_mask |= BIT(module_profile->svfd_ctrl.vdm_cfg_bit[4]);
	if (func_sw & PROF_ACTIVE_DEFENCE)
		ctrl_mask |= BIT(module_profile->svfd_ctrl.vdm_cfg_bit[5]);

	tmp = ctrl_mask | (g_npu_cur_svfd_threshold << 2) | (VSENSOR_SVFD_CTRL2_MASK);
	writel(tmp, module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[11]);
	/* enable cmp data calc */
	tmp = readl(module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[9]);
	writel((tmp | BIT(module_profile->svfd_ctrl.vdm_cfg_bit[6])), (module_profile->svfd_ctrl.vs_ctrl_base + module_profile->svfd_ctrl.vdm_reg_offset[9]));
}

void npu_update_svfd_code(struct hisi_npu_module_profile *module_profile, int tar_pid)
{
	unsigned int temp_idx;

	if (strcmp(module_profile->module_name, g_dvfs_data.main_module->module_name) != 0)
		return;
	temp_idx = update_avs_temperature_info();

	g_npu_cur_svfd_threshold = g_dvfs_data.main_module->profile_table[tar_pid].svfdcode[temp_idx];
}

int hisi_avs_npu_thermal_init(struct device *dev)
{
#ifdef CONFIG_HISI_THERMAL_TSENSOR
	hisi_npu_avs_tsensor = ipa_get_tsensor_id(HISI_AVS_NPU_TSENSOR_NAME);
	if (hisi_npu_avs_tsensor < 0) {
		dev_err(dev, "get tsensor fail(%d)\n", g_dvfs_data.tsensor);
		return -ENOMEM;
	}
#endif
	return 0;
}

int hisi_npu_avs_memory_cfg_init(struct device *dev, struct hisi_npu_module_profile *module_profile)
{
	int i;
	struct device_node *root = dev->of_node;
	void __iomem *base[3];

	for (i = 0; i < 3; i++) {
		base[i] = of_iomap(root, i);
		if (!base[i]) {
			dev_err(dev, "[%s] base[%d] iomap fail %ld\n", __func__, i, PTR_ERR(base[i]));
			return -ENOMEM;
		}
	}

	module_profile->svfd_ctrl.vs_crg_base = base[1];
	module_profile->svfd_ctrl.vs_ctrl_base = base[2];

	return 0;
}

int hisi_npu_pasensor_clk_init(struct device *dev,
                               struct device_node *module_np,struct hisi_npu_module_profile *module_profile)
{
	int ret;
	const char *clk_name = NULL;

	ret = of_property_read_string(module_np, "pa-clock-names", &clk_name);
	if (ret != 0) {
		dev_err(dev, "failed to find pa-clock-names\n");
		return ret;
	}
	module_profile->pa_ctrl.pll_clk = hisi_npu_find_clock(clk_name);
	return ret;
}

void hisi_npu_avs_info_debug(struct device *dev,
							struct hisi_npu_module_profile *module_profile)
{
	int i;

	for (i = 0; i < MAX_AVS_VOLT; i++) {
		dev_dbg(dev, "npu avs temperature[%d] ->(%x)\n", i, module_profile->avs_temp[i]);
	}

	for (i = 0; i < SVFD_REG_OFFSET_NUM; i++) {
		dev_dbg(dev, "npu svfd offset %d ->(%x)\n", i, module_profile->svfd_ctrl.vdm_reg_offset[i]);
	}

	for (i = 0; i < SVFD_CFG_BIT_NUM; i++) {
		dev_dbg(dev, "npu svfd-cfg-bit %d ->(%x)\n", i, module_profile->svfd_ctrl.vdm_cfg_bit[i]);
	}

	for (i = 0; i < PA_MARGIN_NUM; i++) {
		dev_dbg(dev, "npu pa-code-margin %d ->(%x)\n", i, module_profile->pa_ctrl.pa_margin[i]);
	}

	dev_dbg(dev, "avsvolt_compute base %d step %d\n", g_npu_avsvolt[0], g_npu_avsvolt[1]);
	dev_dbg(dev, "pa-safe-profile %d\n", module_profile->pa_ctrl.pa_safe_prof);
	dev_dbg(dev, "pa-step-volt %d\n", module_profile->pa_ctrl.pa_step_volt);
	dev_dbg(dev, "pa-max-code %d\n", module_profile->pa_ctrl.pa_max_code);
}

static int avs_dt_func_para_check(struct device *dev,
                               struct device_node *module_np,
                               struct hisi_npu_module_profile *module_profile, const char *main_module_name)
{
	if (dev == NULL || module_np == NULL || module_profile == NULL || main_module_name == NULL) {
		return 1;
	}

	return 0;
}

int hisi_npu_dt_avs_info_parse(struct device *dev,
                               struct device_node *module_np,
                               struct hisi_npu_module_profile *module_profile, const char *main_module_name)
{
	int ret, i, j;
	unsigned int *prof_switch = NULL;
	unsigned int *svfd_code = NULL;
	unsigned int *avs_volt = NULL;

	ret = avs_dt_func_para_check(dev, module_np, module_profile, main_module_name);
	if (ret != 0) {
		pr_err("[npu avs] %s %d pasensor init fail %d\n", __func__, __LINE__, ret);
		return 0;
	}

	ret = strncmp(main_module_name, module_profile->module_name, strlen(main_module_name));
	if (ret != 0) {
		return 0;
	}
	ret = hisi_avs_npu_thermal_init(dev);

	ret += hisi_npu_avs_memory_cfg_init(dev, module_profile);

	ret += hisi_npu_pasensor_clk_init(dev, module_np, module_profile);
	if (ret != 0) {
		dev_err(dev, "%s npu avs pa_clk init failed\n", __func__);
		return ret;
	}

	prof_switch = devm_kzalloc(dev, sizeof(unsigned int) * module_profile->profile_count, GFP_KERNEL);
	if (!prof_switch) {
		dev_err(dev, "Failed to allocate npu prof switch buff\n");
		return -ENOMEM;
	}

	svfd_code = devm_kzalloc(dev, sizeof(unsigned int) * (module_profile->profile_count * MAX_AVS_VOLT), GFP_KERNEL);//lint !e647
	if (!svfd_code) {
		dev_err(dev, "Failed to allocate npu svfd code buff\n");
		ret = -ENOMEM;
		goto svfd_alloc_fail;
	}

	avs_volt = devm_kzalloc(dev, sizeof(unsigned int) * (module_profile->profile_count * MAX_AVS_VOLT), GFP_KERNEL);//lint !e647
	if (!avs_volt) {
		dev_err(dev, "Failed to allocate npu avs volt buff\n");
		ret = -ENOMEM;
		goto avs_alloc_fail;
	}
	/*get avs config*/
	ret = of_property_read_u32_array(module_np, "avs-temperature", (u32 *)(module_profile->avs_temp), MAX_AVS_VOLT);

	ret += of_property_read_u32_array(module_np, "svfd-reg-offset", (u32 *)(module_profile->svfd_ctrl.vdm_reg_offset), SVFD_REG_OFFSET_NUM);

	ret += of_property_read_u32_array(module_np, "svfd-cfg-bit", (u32 *)(module_profile->svfd_ctrl.vdm_cfg_bit), SVFD_CFG_BIT_NUM);

	ret += of_property_read_u32_array(module_np, "prof-sw", (u32 *)(prof_switch), module_profile->profile_count);

	ret += of_property_read_u32_array(module_np, "prof-svfdcode", (u32 *)(svfd_code), (module_profile->profile_count * MAX_AVS_VOLT));//lint !e647

	ret += of_property_read_u32_array(module_np, "prof-avsvolt", (u32 *)(avs_volt), (module_profile->profile_count * MAX_AVS_VOLT));//lint !e647

	ret += of_property_read_u32_array(module_np, "avsvolt_compute", (u32 *)(g_npu_avsvolt), 2);//lint !e647

	ret += of_property_read_u32_array(module_np, "pa-safe-profile", &(module_profile->pa_ctrl.pa_safe_prof), 1);

	ret += of_property_read_u32_array(module_np, "pa-code-margin", (u32 *)(module_profile->pa_ctrl.pa_margin), PA_MARGIN_NUM);

	ret += of_property_read_u32_array(module_np, "pa-step-volt", &(module_profile->pa_ctrl.pa_step_volt), 1);

	ret += of_property_read_u32_array(module_np, "pa-max-code", &(module_profile->pa_ctrl.pa_max_code), 1);

	if (ret != 0) {
		pr_err("[npu avs] failed to get avs config from DT\n");
		goto parse_dt_fail;
	}

	hisi_npu_avs_info_debug(dev, module_profile);

	for (i = 0; i < module_profile->profile_count; i++) {
		module_profile->profile_table[i].func_sw = prof_switch[i];
		dev_dbg(dev, "profie[%d],func_sw = %x\n", i, module_profile->profile_table[i].func_sw);
		for (j = 0; j < MAX_AVS_VOLT; j++) {
			module_profile->profile_table[i].svfdcode[j] = svfd_code[i * MAX_AVS_VOLT + j];//lint !e679
			module_profile->profile_table[i].avsvolt[j] = avs_volt[i * MAX_AVS_VOLT + j];//lint !e679
			dev_dbg(dev, "profie[%d],svfd[%d] = %x, avs_volt[%d] = %x\n", i, j, module_profile->profile_table[i].svfdcode[j], j, \
					module_profile->profile_table[i].avsvolt[j]);
		}
	}
parse_dt_fail:
	devm_kfree(dev, avs_volt);
avs_alloc_fail:
	devm_kfree(dev, svfd_code);
svfd_alloc_fail:
	devm_kfree(dev, prof_switch);
	return ret;
}
