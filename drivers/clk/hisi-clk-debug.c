

#include <linux/clk-provider.h>
#include <linux/clk/clk-conf.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/io.h>
#include "clk.h"
#include "hisi-clk-debug.h"
#include <soc_acpu_baseaddr_interface.h>
#ifdef CONFIG_HISI_CLK_TRACE
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <linux/hisi/rdr_pub.h>
#include <soc_pmctrl_interface.h>
#include <linux/of_address.h>
#endif
#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
#include <soc_media1_crg_interface.h>
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
#include <soc_media2_crg_interface.h>
#endif
#include <soc_iomcu_interface.h>
#include <pmic_interface.h>
#include <soc_pctrl_interface.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include "hisi/clk-kirin-common.h"
#include "hisi/hisi-kirin-ppll.h"
#include <linux/mfd/hisi_pmic.h>
#include <linux/delay.h>
#include <securec.h>

#define CLK_INPUT_MAX_NUM 100
#define hwlog_err(fmt, args...)  do { printk(KERN_ERR   "[hisi_clk]" fmt, ## args); } while (0)

#ifdef CONFIG_HISI_CLK_TRACE
#define CLK_NAME_LEN   32
#define CLK_MAGIC_NUM  0x20160322
#define CLK_ERR_NUM     0xFF
/*lint -e750 -esym(750,*) */
#define CLK_DVFS_ADDR_0(Base)  SOC_PMCTRL_PERI_CTRL4_ADDR(Base)
#define CLK_DVFS_ADDR_1(Base)  SOC_PMCTRL_PERI_CTRL5_ADDR(Base)
/*lint -e750 +esym(750,*) */

typedef enum {
	TRACK_ON_OFF = 0,
	TRACK_SET_FREQ,
	TRACK_SET_DVFS,
	TRACK_CLK_MAX
} track_clk_type;

typedef enum {
	SWITCH_CLOSE = 0,
	SWITCH_OPEN,
	SWITCH_MAX
} himntn_trace_switch;

typedef struct {
	unsigned int    dump_magic;
	unsigned int    buffer_size;
	unsigned char  *buffer_addr;
	unsigned char  *percpu_addr[NR_CPUS];
	unsigned int    percpu_length[NR_CPUS];
} pc_record_info;

typedef struct {
	u64   current_time;
	u16   item;
	u16   enable_count;
	u32   current_rate;
	u32   cpu_l;
	u32   cpu_b;
	u32   ppll;
	u32   ddr_freq;
	u32   peri_dvfs_vote0;
	u32   peri_dvfs_vote1 ;
	char  comm[CLK_NAME_LEN];
} clk_record_info;

typedef struct {
	void __iomem   *pmuctrl;
	struct clk     *clk_cpu_l;
	struct clk     *clk_cpu_b;
	struct clk     *clk_ddr;
} hs_trace_clk_info;

typedef enum {
	SINGLE_BUFF = 0,
	MULTI_BUFF,
} buf_type_en;


static u64 clk_rdr_core_id = RDR_CLK;
static pc_record_info *g_clk_track_addr;
static unsigned char g_clk_hook_on;
static buf_type_en  clk_sel_buf_type = MULTI_BUFF;
static hs_trace_clk_info hs_trace_info;

extern int __clk_get_source(struct clk *clk);
#endif

#define REG_VADDR_MAP(phyAddr)          ioremap(phyAddr, sizeof(unsigned long))
#define REG_VADDR_UNMAP(virAddr)        iounmap(virAddr)

#define clklog_err(fmt, args...)  \
	pr_err("[hisi_clk_test] [%s]" fmt, __func__, ## args)
#define clklog_info(fmt, args...) \
	pr_info("[hisi_clk_test] [%s]" fmt, __func__, ## args)
#define clklog_debug(fmt, args...) do { } while (0)
#define seq_clklog(s, fmt, args...) \
	do { \
		if (s) \
			seq_printf(s, fmt, ## args); \
		else \
			pr_err("[hisi_clk_test] " fmt, ## args); \
	} while (0)

/* check kernel data state matched to register */
#define MATCH                            0
#define PARTIAL_MATCH                   -2
#define MISMATCH                        -1

/* Fixed-Rate Clock Attrs: clock freq */
#define HISI_FIXED_RATE_ATTR            "clock-frequency"

/* General-Gate Clock Attrs: offset and bitmask */
#define HISI_GENERAL_GATE_REG_ATTR      "hisilicon,hi3xxx-clkgate"

/* Himask-Gate Clock Attrs */
#define HISI_HIMASK_ATTR                "hiword"
#define HISI_HIMASK_GATE_REG_ATTR       "hisilicon,clkgate"

/* MUX(SW) Clock Attrs */
#define HISI_MUX_REG_ATTR               "hisilicon,clkmux-reg"

/* DIV Clock Attr */
#define HISI_DIV_REG_ATTR               "hisilicon,clkdiv"

/* Fixed-DIV Clock Attr */
#define HISI_FIXED_DIV_ATTR1            "clock-div"
#define HISI_FIXED_DIV_ATTR2            "clock-mult"

/* PPLL Clock Attr */
#define HISI_PPLL_EN_ATTR               "hisilicon,pll-en-reg"
#define HISI_PPLL_GT_ATTR               "hisilicon,pll-gt-reg"
#define HISI_PPLL_BP_ATTR               "hisilicon,pll-bypass-reg"
#define HISI_PPLL_CTRL0_ATTR            "hisilicon,pll-ctrl0-reg"

/* PMU CLock Attr */
#define HISI_PMU_ATTR                   "pmu32khz"

/* Clock Types */
enum {
	HISI_CLOCK_FIXED = 0,
	HISI_CLOCK_GENERAL_GATE,
	HISI_CLOCK_HIMASK_GATE,
	HISI_CLOCK_MUX,
	HISI_CLOCK_DIV,
	HISI_CLOCK_FIXED_DIV,
	HISI_CLOCK_PPLL,
	HISI_CLOCK_PLL,
	HISI_CLOCK_PMU,
	HISI_CLOCK_IPC,
	HISI_CLOCK_ROOT,
	HISI_CLOCK_DVFS,
	MAX_CLOCK_TYPE,
};

struct clock_check_param {
	struct seq_file *s;
	struct device_node *np;
	struct clk_core *clk;
	int level;
	int parent_idx;
};

#define CLOCK_REG_GROUPS   5
struct clock_print_info {
	const char *clock_name;
	const char *clock_type;
	unsigned long rate;
	int clock_reg_groups;
	const char *crg_region[CLOCK_REG_GROUPS];
	int region_stat[CLOCK_REG_GROUPS];
	unsigned int reg[CLOCK_REG_GROUPS];
	unsigned char h_bit[CLOCK_REG_GROUPS];
	unsigned char l_bit[CLOCK_REG_GROUPS];
	unsigned int reg_val[CLOCK_REG_GROUPS];
	unsigned int enable_cnt;
	unsigned int prepare_cnt;
	int check_state;
	int parent_idx;
};

struct hisi_clock_type {
	char name[64];
	int (*check_func)(struct clock_check_param *);
	char alias[32];
};

static int check_fixed_rate(struct clock_check_param *param);
static int check_general_gate(struct clock_check_param *param);
static int check_himask_gate(struct clock_check_param *param);
static int check_mux_clock(struct clock_check_param *param);
static int check_div_clock(struct clock_check_param *param);
static int check_fixed_div_clock(struct clock_check_param *param);
static int check_kirin_ppll_clock(struct clock_check_param *param);
static int check_hisi_pll_clock(struct clock_check_param *param);
static int check_pmu_clock(struct clock_check_param *param);
static int check_ipc_clock(struct clock_check_param *param);
static int check_root_clock(struct clock_check_param *param);
static int check_dvfs_clock(struct clock_check_param *param);

#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
static int peri_power_up_ok(void);
#endif
#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
static int sctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_PMC_BASE_ADDR)
static int pmctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
static int pctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
static int media1_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
static int media2_power_up_ok(void);
#endif
#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
static int iomcu_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
static int mmc1sysctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
static int mmc0_power_up_ok(void);
#endif
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
static int hsdt_power_up_ok(void);
#endif
#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
static int hsdt1_power_up_ok(void);
#endif
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
static int pmuspmi_power_up_ok(void);
#endif

const struct hisi_clock_type clock_types[] = {
	{"fixed-clock",                 check_fixed_rate,       "Fixed-Rate"  },
	{"hisilicon,hi3xxx-clk-gate",   check_general_gate,     "Gate"        },
	{"hisilicon,clk-gate",          check_himask_gate,      "Himask-Gate" },
	{"hisilicon,hi3xxx-clk-mux",    check_mux_clock,        "Mux(SW)"     },
	{"hisilicon,hi3xxx-clk-div",    check_div_clock,        "Div"         },
	{"fixed-factor-clock",          check_fixed_div_clock,  "Fixed-Factor"},
	{"hisilicon,kirin-ppll-ctrl",   check_kirin_ppll_clock, "PPLL"        },
	{"hisilicon,ppll-ctrl",         check_hisi_pll_clock,   "PLL"         },
	{"hisilicon,clk-pmu-gate",      check_pmu_clock,        "PMU-Gate"    },
	{"hisilicon,interactive-clk",   check_ipc_clock,        "IPC-Gate"    },
	{"hisilicon,hi3xxx-xfreq-clk",  check_root_clock,       "Root"        },
	{"hisilicon,clkdev-dvfs",       check_dvfs_clock,       "DVFS"        },
};
const int clock_type_size = ARRAY_SIZE(clock_types);

/* CRG Regions Types */
enum {
#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	HISI_CLOCK_CRG_PERI,
#endif
#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
	HISI_CLOCK_CRG_SCTRL,
#endif
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	HISI_CLOCK_CRG_PMCTRL,
#endif
#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
	HISI_CLOCK_CRG_PCTRL,
#endif
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
	HISI_CLOCK_CRG_MEDIA1,
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
	HISI_CLOCK_CRG_MEDIA2,
#endif
#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
	HISI_CLOCK_CRG_IOMCU,
#endif
#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
	HISI_CLOCK_CRG_MMC1,
#endif
#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
	HISI_CLOCK_CRG_MMC0,
#endif
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
	HISI_CLOCK_CRG_HSDT,
#endif
#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
	HISI_CLOCK_CRG_HSDT1,
#endif
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
	HISI_CLOCK_CRG_PMU,
#endif
};

struct hisi_crg_region {
	unsigned int base_addr;
	int (*region_ok)(void);
	char crg_name[16];
};

const struct hisi_crg_region crg_regions[] = {
#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	{SOC_ACPU_PERI_CRG_BASE_ADDR,      peri_power_up_ok,        "PERI"  },
#endif
#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
	{SOC_ACPU_SCTRL_BASE_ADDR,         sctrl_power_up_ok,       "SCTRL" },
#endif
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	{SOC_ACPU_PMC_BASE_ADDR,           pmctrl_power_up_ok,      "PMCTRL"},
#endif
#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
	{SOC_ACPU_PCTRL_BASE_ADDR,         pctrl_power_up_ok,       "PCTRL" },
#endif
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
	{SOC_ACPU_MEDIA1_CRG_BASE_ADDR,    media1_power_up_ok,      "MEDIA1"},
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
	{SOC_ACPU_MEDIA2_CRG_BASE_ADDR,    media2_power_up_ok,      "MEDIA2"},
#endif
#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
	{SOC_ACPU_IOMCU_CONFIG_BASE_ADDR,  iomcu_power_up_ok,       "IOMCU" },
#endif
#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
	{SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR, mmc1sysctrl_power_up_ok, "MMC1"  },
#endif
#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
	{SOC_ACPU_MMC0_CRG_BASE_ADDR,      mmc0_power_up_ok,        "MMC0"  },
#endif
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
	{SOC_ACPU_HSDT_CRG_BASE_ADDR,      hsdt_power_up_ok,        "HSDT"  },
#endif
#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
	{SOC_ACPU_HSDT1_CRG_BASE_ADDR,     hsdt1_power_up_ok,       "HSDT1" },
#endif
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
	{SOC_ACPU_SPMI_BASE_ADDR,          pmuspmi_power_up_ok,     "PMU"   },
#endif
};
const int crg_region_size = ARRAY_SIZE(crg_regions);

extern struct list_head clocks;

static struct dentry *clock;
static struct dentry *test_all_clocks;
static struct dentry *test_one_clock;

static DEFINE_MUTEX(clock_list_lock);

#define to_clk_gate(_hw) container_of(_hw, struct clk_gate, hw)
#define to_clk_mux(_hw) container_of(_hw, struct clk_mux, hw)

extern bool clk_core_is_enabled(struct clk_core *clk);
extern unsigned long clk_core_get_rate(struct clk_core *clk);


static char g_clk_status[4][10] = {"NOREG", "OK", "ERR", "NULL"};
struct clk_core *__clk_core_get_parent(struct clk_core *clk)
{
	if (!clk)
		return NULL;

	/* TODO: Create a per-user clk and change callers to call clk_put */
	return !clk->parent ? NULL : clk->parent;
}

const char *__clk_core_get_name(struct clk_core *clk)
{
	return !clk ? NULL : clk->name;
}

unsigned int __clk_core_get_enable_count(struct clk_core *clk)
{
	return !clk ? 0 : clk->enable_count;
}

int clk_core_dump(struct clk_core *clk, char *buf, int length)
{
	int ret = 0;
	int buf_length = length;
	if(buf_length > DUMP_CLKBUFF_MAX_SIZE)
		buf_length = DUMP_CLKBUFF_MAX_SIZE;
	if (!clk)
		return 0;
	if (clk->name) {
		ret = snprintf_s(buf, DUMP_CLKBUFF_MAX_SIZE, buf_length, "******%s\n", clk->name);
		if (ret == -1)
			pr_err("%s snprintf_s failed!\n", __func__);
		hwlog_err("%s\n", buf);
	}
// cppcheck-suppress *
	ret = clk_core_dump(clk->parent, buf, buf_length);
	if (ret)
		return ret;
	if (clk->ops->dump_reg) {
		ret = clk->ops->dump_reg(clk->hw, buf, NULL);
		hwlog_err("%s\n", buf);
	}
	if (ret)
		return ret;
	return 0;
}
int clk_dump_reg(struct clk *clk)
{
	int ret;
	char buf[DUMP_CLKBUFF_MAX_SIZE];

	if (!clk)
		return 0;
	mutex_lock(&clock_list_lock);
	ret = memset_s(buf, DUMP_CLKBUFF_MAX_SIZE, 0, sizeof(buf));
	if (ret != EOK)
		pr_err("%s mm_s failed!\n", __func__);

	ret = snprintf_s(buf, DUMP_CLKBUFF_MAX_SIZE, DUMP_CLKBUFF_MAX_SIZE - 1, "\n\nDump clk list:\n");
	if (ret == -1)
		pr_err("%s snprintf_s failed!\n", __func__);

	hwlog_err("%s\n", buf);
	ret = clk_core_dump(clk->core, buf, DUMP_CLKBUFF_MAX_SIZE);
	mutex_unlock(&clock_list_lock);
	return ret;
}

void clk_base_addr_print(struct seq_file *s)
{
	if (!s)
		return;
	seq_puts(s, "----------------------------------------------------------------------------------------\n");
#if defined SOC_ACPU_PERI_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "PERICRG", SOC_ACPU_PERI_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_SCTRL_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "SCTRL", SOC_ACPU_SCTRL_BASE_ADDR);
#endif
#if defined SOC_ACPU_PMC_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "PMCTRL", SOC_ACPU_PMC_BASE_ADDR);
#endif
#if defined SOC_ACPU_PCTRL_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "PCTRL", SOC_ACPU_PCTRL_BASE_ADDR);
#endif
#if defined SOC_ACPU_MEDIA1_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MEDIA1CRG", SOC_ACPU_MEDIA1_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_MEDIA2_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MEDIA2CRG", SOC_ACPU_MEDIA2_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MMC1CRG", SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR);
#endif
#if defined SOC_ACPU_MMC0_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MMC0CRG", SOC_ACPU_MMC0_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_HSDT_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "HSDCRG", SOC_ACPU_HSDT_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_IOMCU_CONFIG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "IOMCU", SOC_ACPU_IOMCU_CONFIG_BASE_ADDR);
#endif
	seq_puts(s, "----------------------------------------------------------------------------------------\n");
	return;
}

void clk_tree_dump_reg(struct seq_file *s, struct clk_core *clk_core)
{
	int ret = 0;

	if (!clk_core || !s)
		return;
	mutex_lock(&clock_list_lock);
	if (clk_core->ops->dump_reg)
		ret = clk_core->ops->dump_reg(clk_core->hw, NULL, s);

	if (ret) {
		if (clk_core->name)
			hwlog_err("%s dump reg return fail!\n", clk_core->name);
	}
	mutex_unlock(&clock_list_lock);
	return;
}


char *clk_enreg_check(struct clk_core *c)
{
	int val = 0;

	if (IS_ERR(c))
		return g_clk_status[2];

	if (!c->ops->is_enabled)
		return g_clk_status[0];

	val = c->ops->is_enabled(c->hw);

	if (val == 2)
		return g_clk_status[0];

	if ((val && c->enable_count) || ((val == 0) && (c->enable_count == 0)))
		return g_clk_status[1];
	else
		return g_clk_status[2];
}

char *clk_selreg_check(struct clk_core *c)
{
	int val = 0;

	if (IS_ERR(c))
		return g_clk_status[2];

	if (!c->ops->check_selreg) {
		return g_clk_status[0];
	}

	val = c->ops->check_selreg(c->hw);

	if (3 == val) {
		return g_clk_status[3];
	}

	return val ? g_clk_status[1] : g_clk_status[2];
}

char *clk_divreg_check(struct clk_core *c)
{
	int val = 0;

	if (IS_ERR(c))
		return g_clk_status[2];

	if (!c->ops->check_divreg)
		return g_clk_status[0];

	val = c->ops->check_divreg(c->hw);

	return val ? g_clk_status[1] : g_clk_status[2];
}

int clock_get_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	struct clk_core  *clock = NULL;/*lint !e578*/
	struct clk *clock_get = NULL;
	const char *status = NULL;

	seq_printf(s, "%18s%-3s%5s%-3s%10s\n", "clock", "", "status", "", "rate");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_get() interface. */
	list_for_each_entry(clk, &clocks, node) {
		clock_get = clk_get(NULL, clk->name);
		if (IS_ERR(clock_get)) {
			pr_err("%s clock_get failed!\n", clk->name);
			continue;
		}
		clock = clock_get->core;
		if (!clock) {
			pr_err("%s get failed!\n", clk->name);
			continue;
		}

		if (clock->enable_count)
			status = "on";
		else
			status = "off";

		seq_printf(s, "%18s%-3s%5s%-3s%10ld\n", clock->name, "", status, "", clock->rate);
	}

	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_lookup_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	struct clk_core *clock = NULL;/*lint !e578*/
	struct clk *clock_get = NULL;
	const char *status = NULL;

	seq_printf(s, "%14s%-3s%5s%-3s%10s\n", "clock", "", "status", "", "rate");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test __clk_lookup() interface. */
	list_for_each_entry(clk, &clocks, node) {
		clock_get = __clk_lookup(clk->name);
		if (!clock_get) {
			pr_err("%s clock_get failed!\n", clk->name);
			goto out;
		}
		clock = clock_get->core;
		if (!clock) {
			pr_err("%s get failed!\n", clk->name);
			goto out;
		}

		if (clock->enable_count)
			status = "on";
		else
			status = "off";

		seq_printf(s, "%20s%-3s%5s%-3s%10ld\n", clock->name, "", status, "", clock->rate);
	}
out:
	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_enable_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	int ret = 0;

	printk("%20s%-3s%5s%-3s%5s\n", "clock", "", "refcnt_pre", "", "refcnt_now");
	printk("---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_enable() interface. */
	list_for_each_entry(clk, &clocks, node) {
		if (!clk->ops->enable)
			continue;

		printk("%20s%-3s%5d", clk->name, "", clk->enable_count);

		ret = clk_prepare_enable(clk->hw->clk);
		if (ret) {
			pr_err("%s enable failed!\n", clk->name);
			goto out;
		}

		printk("%-3s%5d\n", "", clk->enable_count);
	}
out:
	mutex_unlock(&clock_list_lock);
	return ret;
}

int clock_disable_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;

	printk("%20s%-3s%5s%-3s%5s\n", "clock", "", "refcnt_pre", "", "refcnt_now");
	printk("---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_disable() interface. */
	list_for_each_entry(clk, &clocks, node) {
		if (!clk->ops->enable)
			continue;

		printk("%20s%-3s%5d", clk->name, "", clk->enable_count);

		clk_disable_unprepare(clk->hw->clk);

		printk("%-3s%5d\n", "", clk->enable_count);
	}

	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_getparent_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	int i;

	seq_printf(s, "%20s%-5s%8s\n", "clock", "", "parent");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_get_parent() interface. */
	list_for_each_entry(clk, &clocks, node) {
		if (clk->num_parents == 0)
			seq_printf(s, "%20s%-5s%8s\n", clk->name, "", "");
		else {
			seq_printf(s, "%20s", clk->name);

			for (i = 0; i < clk->num_parents; i++)
				seq_printf(s, "%-5s%20s", "", clk->parent_names[i]);

			seq_printf(s, "\n");
		}
	}
	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_getrate_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;

	seq_printf(s, "%20s%-5s%10s\n", "clock", "", "rate");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_get_rate() interface. */
	list_for_each_entry(clk, &clocks, node)
		seq_printf(s, "%20s%-5s%10ld\n", clk->name, "", clk->rate);

	mutex_unlock(&clock_list_lock);

	return 0;
}

int clock_tree_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	struct clk_core *parent = NULL;
	const char *state = NULL;
	const char *pstate = NULL;

	/* Output gating clock and fixed rate clock in the clocks list */
	seq_printf(s, "%17s%-6s%4s%-2s%6s%-2s%6s%-2s%6s%-s%7s%-2s%5s%-s%16s%-2s%4s%-2s%5s%-3s%5s\n",
		"clock", "", "status", "", "enreg", "", "selreg", "", "divreg", "", "refcnt", "", "rate",
		"", "parent", "", "status", "", "refcnt", "", "rate");
	seq_printf(s, "--------------------------------------------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	list_for_each_entry(clk, &clocks, node) {
		if (clk_core_is_enabled(clk))
			state = "on";
		else
			state = "off";

		if (clk->ops->check_divreg || clk->ops->check_selreg || clk->ops->round_rate)
			continue;

		parent = __clk_core_get_parent(clk);
		if (NULL == parent)
			seq_printf(s, "%18s%-2s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%3d%-2s%10ld%-s%18s%-2s%4s%-2s%3s%-3s%5s%-s%15s\n",
			__clk_core_get_name(clk), "", state, "", clk_enreg_check(clk), "",
			clk_selreg_check(clk), "", clk_divreg_check(clk), "", __clk_core_get_enable_count(clk), "",
			clk_core_get_rate(clk), "", "", "", "", "", "", "", "", "", "");
		else {
			/* judge register status of this parent clock. */
			if (clk_core_is_enabled(parent))
				pstate = "on";
			else
				pstate = "off";

			seq_printf(s, "%18s%-2s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%3d%-2s%10ld%-s%18s%-2s%4s%-2s%3d%-3s%10ld\n",
			__clk_core_get_name(clk), "", state, "", clk_enreg_check(clk), "", clk_selreg_check(clk),
			"", clk_divreg_check(clk), "", __clk_core_get_enable_count(clk), "",
			clk_core_get_rate(clk), "", __clk_core_get_name(parent), "", pstate, "",
			__clk_core_get_enable_count(parent), "", clk_core_get_rate(parent));
		}
	}

	/* Output multiplexer clock in the clocks list */
	seq_printf(s, "-----------------------------------------------------------------------------------------------------------------------\n");
	seq_printf(s, "-----------------------------------------------------[mux clock]--------------------------------------------------------\n");
	seq_printf(s, "-------------------------------------------------------------------------------------------------------------------------\n");

	seq_printf(s, "%17s%-6s%4s%-2s%6s%-2s%6s%-2s%6s%-s%7s%-2s%5s%-s%16s%-2s%4s%-2s%5s%-3s%5s\n",
		"clock", "", "status", "", "enreg", "", "selreg", "", "divreg", "", "refcnt", "", "rate",
		"", "parent", "", "status", "", "refcnt", "", "rate");
	seq_printf(s, "--------------------------------------------------------------------------------------------------------\n\n");

	list_for_each_entry(clk, &clocks, node) {
		if (clk_core_is_enabled(clk))
			state = "on";
		else
			state = "off";

		if (!(clk->ops->set_parent))
			continue;

		parent = __clk_core_get_parent(clk);

		if (clk_core_is_enabled(parent))
			pstate = "on";
		else
			pstate = "off";

		seq_printf(s, "%20s%-2s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%3d%-2s%10ld%-s%20s%-2s%4s%-2s%3d%-3s%10ld\n",
			__clk_core_get_name(clk), "", state, "", clk_enreg_check(clk), "", clk_selreg_check(clk),
			"", clk_divreg_check(clk), "", __clk_core_get_enable_count(clk), "",
			clk_core_get_rate(clk), "", __clk_core_get_name(parent), "", pstate, "",
			__clk_core_get_enable_count(parent), "", clk_core_get_rate(parent));
	}

	/* Output adjustable divider clock / fixed multiplier and divider clock in the clocks list */
	seq_printf(s, "--------------------------------------------------------------------------------------------------------------------------\n");
	seq_printf(s, "---------------------------------------[div clock] [fixed-factor clock]---------------------------------------------------\n");
	seq_printf(s, "----------------------------------------------------------------------------------------------------------------------------\n");

	seq_printf(s, "%17s%-6s%4s%-2s%6s%-2s%6s%-2s%6s%-s%7s%-2s%5s%-s%16s%-2s%4s%-2s%5s%-3s%5s\n",
		"clock", "", "status", "", "enreg", "", "selreg", "", "divreg", "", "refcnt", "", "rate",
		"", "parent", "", "status", "", "refcnt", "", "rate");
	seq_printf(s, "--------------------------------------------------------------------------------------------------------\n\n");

	list_for_each_entry(clk, &clocks, node) {
		if (clk_core_is_enabled(clk))
			state = "on";
		else
			state = "off";

		if (!(clk->ops->set_rate))
			continue;

		parent = __clk_core_get_parent(clk);

		if (clk_core_is_enabled(parent))
			pstate = "on";
		else
			pstate = "off";

		seq_printf(s, "%20s%-2s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%3d%-2s%10ld%-s%20s%-2s%4s%-2s%3d%-3s%10ld\n",
			__clk_core_get_name(clk), "", state, "", clk_enreg_check(clk), "", clk_selreg_check(clk),
			"", clk_divreg_check(clk), "", __clk_core_get_enable_count(clk), "",
			clk_core_get_rate(clk), "", __clk_core_get_name(parent), "", pstate, "",
			__clk_core_get_enable_count(parent), "", clk_core_get_rate(parent));
	}

	mutex_unlock(&clock_list_lock);

	return 0;
}

// cppcheck-suppress *
#define MODULE_FUNCS_DEFINE(func_name)					\
static int func_name##_open(struct inode *inode, struct file *file)		\
{																		\
	return single_open(file, func_name##_show, inode->i_private);		\
}

MODULE_FUNCS_DEFINE(clock_get);
MODULE_FUNCS_DEFINE(clock_tree);
MODULE_FUNCS_DEFINE(clock_lookup);
MODULE_FUNCS_DEFINE(clock_enable);
MODULE_FUNCS_DEFINE(clock_disable);
MODULE_FUNCS_DEFINE(clock_getrate);
MODULE_FUNCS_DEFINE(clock_getparent);

ssize_t
clock_enable_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int ret = 0;
	int err = 0;

	if (0 == cnt || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (CLK_INPUT_MAX_NUM < cnt) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}

	mutex_lock(&clock_list_lock);

	/* copy clock name from user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		err = -EINVAL;
		goto out;
	}
	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, prepare and enable it. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			pr_info("[old] enable_refcnt = %d\n", clk->enable_count);

			ret = clk_prepare_enable(clk->hw->clk);
			pr_err("[RetValue] clk_prepare_enable ret = %d\n", ret);
			if (ret) {
				err = -EINVAL;
				goto out;
			}

			pr_info("[new] enable_refcnt = %d\n", clk->enable_count);
			err = cnt;
			goto out;
		}
	}

	pr_err("clk name error!\n");
	err = -EINVAL;
out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

ssize_t
clock_disable_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int err = cnt;

	if (0 == cnt || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (CLK_INPUT_MAX_NUM < cnt) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}

	mutex_lock(&clock_list_lock);

	/* copy clock name from user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		err = -EINVAL;
		goto out;
	}
	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, disable and unprepare it. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			if (!__clk_core_get_enable_count(clk)) {
				pr_info("[%s] clk is disabled !\n", __func__);
				goto out;
			}
			pr_info("[old] enable_refcnt = %d\n", clk->enable_count);
			clk_disable_unprepare(clk->hw->clk);
			pr_info("[new] enable_refcnt = %d\n", clk->enable_count);
			goto out;
		}
	}
	pr_err("clk name error!\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

ssize_t
clock_getparent_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int i;
	int err = 0;

	if (cnt == 0 || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (cnt > CLK_INPUT_MAX_NUM) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}

	mutex_lock(&clock_list_lock);

	/* copy clock name from user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		err = -EINVAL;
		goto out;
	}
	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, get parent of this clock. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			pr_info("[%s] ", clk->name);

			if (clk->num_parents)
				for (i = 0; i < clk->num_parents; i++)
					pr_info("[%d] %s\n", i, clk->parent_names[i]);
			else
				pr_info("null \n");

			pr_info("\n------[current_parent]------\n");

			pr_info("%s\n\n\n", clk->parent->name);
			err = cnt;
			goto out;
		}
	}

	pr_err("clk name error!\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}


ssize_t
clock_getrate_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int err = 0;

	if (cnt == 0 || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (cnt > CLK_INPUT_MAX_NUM) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}

	/* copy clock name from user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		kfree(clk_name);
		return -EINVAL;
	}
	mutex_lock(&clock_list_lock);
	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, get rate of this clock. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			pr_info("\n[%s]  %ld\n\n\n", clk->name, clk_get_rate(clk->hw->clk));
			err = cnt;
			goto out;/*lint !e456*/
		}
	}

	pr_err("clk name error!\n\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

ssize_t
clock_setrate_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *str = NULL;
	char *clk_name = NULL;
	unsigned long rate;
	int ret = 0;
	int err = 0;

	if (0 == cnt || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (CLK_INPUT_MAX_NUM < cnt) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}
	mutex_lock(&clock_list_lock);
	str = clk_name;
	/* copy clock name and clock rate from user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		err = -EINVAL;
		goto out;
	}
	/* get clock rate */
	str = strchr(str, ' ');
	if (!str) {
		err = -EINVAL;
		goto out;
	} else {
		*str = '\0';
	}
	str++;

	rate = simple_strtoul(str, NULL, 10);
	if (!rate) {
		pr_err("please input clk rate!\n");
		err = -EINVAL;
		goto out;
	}

	pr_info("rate = %lu\n\n", rate);
	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, set rate of this clock. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			ret = clk_set_rate(clk->hw->clk, rate);
			pr_err("[RetValue] clk_set_rate ret = %d\n", ret);
			if (ret) {
				err = -EINVAL;
				goto out;
			}

			pr_info("[%s]  %lu\n\n", clk->name, clk->rate);
			err = cnt;
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

static ssize_t __clock_setparent(struct clk_core *clk, const char *parent_name, const size_t cnt)
{
	int i, ret;
	struct clk_core *parent = NULL;

	for (i = 0; i < clk->num_parents; i++) {
		if (strcmp(parent_name, clk->parent_names[i]) == 0) {/*lint !e421*/
			parent = clk->parents[i];
			if (!parent) {
				struct clk *tclk;
				tclk = __clk_lookup(parent_name);
				parent = tclk ? tclk->core : NULL;
			}
			break;
		}
	}

	if (parent) {
		pr_info("[%s]  set %s\n\n", __func__, parent->name);
		ret = clk_set_parent(clk->hw->clk, parent->hw->clk);
		pr_err("[RetValue] clk_set_parent ret = %d\n", ret);
		if (ret) {
			return -EINVAL;
		}

		pr_info("[%s]  %s\n\n", clk->name, parent->name);

		pr_info("clk set parent ok!\n\n");
	} else {
		printk("no parent find!");
	}

	return (ssize_t)cnt;
}

ssize_t
clock_setparent_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *Input_string = NULL;
	char *clk_name = NULL;
	const char *parent_name = NULL;
	int err = 0;

	if (0 == cnt || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}
	if (CLK_INPUT_MAX_NUM < cnt) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}
	mutex_lock(&clock_list_lock);

	Input_string = clk_name;

	/* copy clock name from and parent name user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		err = -EINVAL;
		goto out;
	}
	/* get clock parent name. */
	Input_string = strchr(Input_string, ' ');
	if (!Input_string) {
		err = -EINVAL;
		goto out;
	} else {
		*Input_string = '\0';
	}

	Input_string++;

	parent_name = Input_string;
	if (!parent_name) {
		pr_err("please input clk parent name!\n");
		err = -EINVAL;
		goto out;
	}
	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, set parent of this clock. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			err = __clock_setparent(clk, parent_name, cnt);
			goto out;
		}
	}
	pr_err("clk name error!\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

ssize_t
clock_getreg_store(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	struct clk *clock_get = NULL;
	struct clk_core *clock = NULL;/*lint !e578*/
	struct clk_gate *gate = NULL;
	struct clk_mux *mux =  NULL;
	void __iomem	 *ret = NULL;
	u32 bits = 0 ;
	u32 val = 0;
	int err = 0;

	if (0 == cnt || !ubuf) {
		pr_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (CLK_INPUT_MAX_NUM < cnt) {
		pr_err("Input string is too long.\n");
		return -EINVAL;
	}
	clk_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!clk_name) {
		pr_err("Cannot allocate clk_name.\n");
		return -EINVAL;
	}
	mutex_lock(&clock_list_lock);

	/* copy clock name from user space. */
	if (copy_from_user(clk_name, ubuf, cnt - 1)) {
		err = -EINVAL;
		goto out;
	}

	clk_name[cnt - 1] = '\0';
	/* Check if we have such a clock in the clocks list. if exist, get reg of this clock. */
	list_for_each_entry(clk, &clocks, node) {
		if (strcmp(clk->name, clk_name) == 0) {/*lint !e421*/
			clock_get = clk_get(NULL, clk->name);
			if (IS_ERR(clock_get)) {
				pr_err("%s clock_get failed!\n", clk->name);
				err = -EINVAL;
				goto out;
			}
			clock = clock_get->core;
			if (!clock) {
				pr_err("%s get failed!\n", clk->name);
				err = -EINVAL;
				goto out_put;
			}
			if (clock->ops->get_reg) {			 /* hi3xxx_periclk,hixxx_divclk */
				ret = clock->ops->get_reg(clock->hw);
			} else if (clock->ops->enable) {
				gate = to_clk_gate(clock->hw); /* andgt clock */
				ret = gate->reg;
				bits = BIT(gate->bit_idx);
				if (ret) {
					val = readl(ret);
					val &= bits;
				}
			} else if (clock->ops->set_parent) {	 /* mux clock */
				mux = to_clk_mux(clock->hw);
				ret = mux->reg;
				bits = mux->mask;
				if (ret) {
					val = readl(ret);
					val &= bits;
				}
			} else {
				pr_err("the clock %s is fixed or fiexd-factor\n", clk_name);
			}
			err = cnt;
			goto out_put;
		}
	}
	pr_err("clk name error!\n\n");
	err = -EINVAL;

out_put:
	pr_info("\n[%s]  reg = 0x%pK, bits = 0x%x, regval = 0x%x\n",
		clk_name, ret, bits, val);
	clk_put(clock_get);
out:
	mutex_unlock(&clock_list_lock);
	kfree(clk_name);
	return err;
}

#define MODULE_SHOW_DEFINE(func_name)					\
	static const struct file_operations func_name##_show_fops = {	\
	.open		= func_name##_open,								\
	.read		= seq_read,										\
	.llseek		= seq_lseek,									\
	.release	= single_release,								\
};

MODULE_SHOW_DEFINE(clock_tree);
MODULE_SHOW_DEFINE(clock_get);
MODULE_SHOW_DEFINE(clock_lookup);
MODULE_SHOW_DEFINE(clock_enable);
MODULE_SHOW_DEFINE(clock_disable);
MODULE_SHOW_DEFINE(clock_getrate);
MODULE_SHOW_DEFINE(clock_getparent);

static int clock_store_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

#define MODULE_STORE_DEFINE(func_name)					\
	static const struct file_operations func_name##_fops = {	\
	.open		= clock_store_open,						\
	.write		= func_name##_store,			\
};

MODULE_STORE_DEFINE(clock_enable);
MODULE_STORE_DEFINE(clock_disable);
MODULE_STORE_DEFINE(clock_getrate);
MODULE_STORE_DEFINE(clock_setrate);
MODULE_STORE_DEFINE(clock_getparent);
MODULE_STORE_DEFINE(clock_setparent);
MODULE_STORE_DEFINE(clock_getreg);

int clk_rate_fops_get(void *data, u64 *rate)
{
	struct clk_core *clk = data;

	*rate = clk->rate;

	return 0;
};

int clk_rate_fops_set(void *data, u64 rate)
{
	struct clk_core *clk = data;
	int ret = 0;

	ret = clk_prepare_enable(clk->hw->clk);
	if (ret)
		return ret;

	ret = clk_set_rate(clk->hw->clk, rate);
	if (ret < 0)
		pr_err("%s clk_set_rate fail, ret=%d\n", __func__, ret);

	clk_disable_unprepare(clk->hw->clk);
	return ret;
};

DEFINE_SIMPLE_ATTRIBUTE(clk_rate_fops, clk_rate_fops_get, clk_rate_fops_set, "%llu\n");

void __clk_statcheck(struct clk_core *clk)
{
	if (WARN_ON(clk_core_is_enabled(clk) == false))
		pr_err("%s stat exception! cnt is %d\n", clk->name, clk->enable_count);
}

void clk_reg_summary_show_one(struct seq_file *s, struct clk_core *c,
				 int level)
{
	if (!c) {
		pr_err("%s c is null\n", __func__);
		return;
	}

	seq_printf(s, "%*s%-*s %5d  %11lu",
		   level * 3 + 1, "",
		   50 - level * 3, c->name,
		   c->enable_count, clk_core_get_rate(c));
	clk_tree_dump_reg(s, c);
	seq_printf(s, "\n");
}

void clk_reg_summary_show_subtree(struct seq_file *s, struct clk_core *c,
				     int level)
{
	struct clk_core *child = NULL;

	if (!c)
		return;

	clk_reg_summary_show_one(s, c, level);

	hlist_for_each_entry(child, &c->children, child_node)
		clk_reg_summary_show_subtree(s, child, level + 1);
}

int clk_reg_summary_show(struct seq_file *s, void *data)
{
	struct clk_core *c = NULL;
	struct hlist_head **lists = (struct hlist_head **)s->private;
	clk_base_addr_print(s);
	for (; *lists; lists++)
		hlist_for_each_entry(c, *lists, child_node)
			clk_reg_summary_show_subtree(s, c, 0);
	return 0;
}

static int clk_reg_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_reg_summary_show, inode->i_private);
}

static const struct file_operations clk_reg_summary_fops = {
	.open		= clk_reg_summary_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/****************** find all leaf clocks begin *********************/
static bool find_clock_by_clkname(const char **clock_names,
		unsigned int clock_num, const char *clkname)
{
	unsigned int i;

	for (i = 0; i < clock_num; i++)
		if (strcmp(clkname, clock_names[i]) == 0)
			return true;

	return false;
}

static void __find_all_leaf_clocks(const char **clock_names,
		unsigned int *p_leaf_clock_num)
{
	struct clk_core *clk = NULL;
	struct clk_core *child = NULL;
	unsigned int clock_num = 0;
	unsigned int clock_idx = 0;
	unsigned int child_num = 0;
	unsigned int leaf_clock_num = 0;

	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node)
		clock_names[clock_num++] = clk->name;

	list_for_each_entry(clk, &clocks, node) {
		child_num = 0;
		hlist_for_each_entry(child, &clk->children, child_node) {
			if (find_clock_by_clkname(clock_names, clock_num,
					child->name))
				child_num++;
		}

		if (!child_num) {
			const char *temp = clock_names[leaf_clock_num];

			clock_names[leaf_clock_num] = clock_names[clock_idx];
			clock_names[clock_idx] = temp;
			leaf_clock_num++;
		}
		clock_idx++;
	}
	mutex_unlock(&clock_list_lock);

	*p_leaf_clock_num = leaf_clock_num;
}

static void find_all_leaf_clocks(struct seq_file *s)
{
	const unsigned int MAX_CLOCK_NUM = 1024;
	const char **clock_names = NULL;
	unsigned int clock_idx = 0;
	unsigned int leaf_clock_num = 0;

	if (IS_ERR_OR_NULL(s))
		return;

	clock_names = kzalloc(sizeof(char *) * MAX_CLOCK_NUM, GFP_KERNEL);
	if (clock_names == NULL) {
		pr_err("[%s] failed to alloc clock_names!\n", __func__);
		return;
	}

	__find_all_leaf_clocks(clock_names, &leaf_clock_num);

	for (clock_idx = 0; clock_idx < leaf_clock_num; clock_idx++)
		seq_printf(s, "%s\n", clock_names[clock_idx]);

	kfree(clock_names);
	seq_printf(s, "---------------------\nTotally %d leaf clocks found!\n",
			leaf_clock_num);
}

int clock_getleaf_show(struct seq_file *s, void *data)
{
	find_all_leaf_clocks(s);
	return 0;
}
MODULE_FUNCS_DEFINE(clock_getleaf);
MODULE_SHOW_DEFINE(clock_getleaf);
/****************** find all leaf clocks end *********************/
static int read_reg_u32(unsigned int hwaddr, unsigned int *value)
{
	void __iomem *reg;

	reg = REG_VADDR_MAP(hwaddr);
	if (!IS_ERR_OR_NULL(reg)) {
		if (value)
			*value = readl(reg);
		REG_VADDR_UNMAP(reg);
	} else {
		clklog_err("ioremap 0x%x error!\n", hwaddr);
		return -ENOMEM;
	}
	return 0;
}

struct clk_core *hisi_find_clock(const char *clk_name)
{
	struct clk_core *clk = NULL;

	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("[%s] clk is err or nul!\n", __func__);
			goto out;
		}
		if (strcmp(clk->name, clk_name) == 0) {
			pr_info("[%s] find clock: %s!\n", __func__, clk_name);
			goto out;
		}
	}
	clk = NULL; /* No Clock found */
out:
	mutex_unlock(&clock_list_lock);
	return clk;
}

static inline int update_line_offset(int offset, int count)
{
	if (count > 0)
		return offset + count;
	return offset;
}

static inline const char *get_clock_matched_state(int check_state)
{
	if (check_state == MATCH)
		return "[Y]";
	else if (check_state == PARTIAL_MATCH)
		return "[P]";
	else
		return "[N]";
}

#define HISI_CLOCK_PARENT_IDX_OFFSET  0x0000ffff
#define MAX_STR_BUF_SIZE              256
#define LINE_LEFT(offset)     \
    (((MAX_STR_BUF_SIZE - (offset)) > 1) ? (MAX_STR_BUF_SIZE - (offset)) : 1)
static void print_clock_info(struct seq_file *s,
		const struct clock_print_info *info, int level)
{
	char line[MAX_STR_BUF_SIZE] = {0};
	int offset = 0;
	int ret = 0;
	int i = 0;

	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%*s%-*s", level * 3 + 1,
		(info->parent_idx >= HISI_CLOCK_PARENT_IDX_OFFSET ? "*" : ""),
		50 - level * 3, info->clock_name);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%-13s", info->clock_type);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-10lu", info->rate);
	offset = update_line_offset(offset, ret);

	if (info->clock_reg_groups > 0) {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-6s%-5s", info->crg_region[0],
			(info->region_stat[0] ? "" : "[OFF]"));
		offset = update_line_offset(offset, ret);

		if (info->h_bit[0] >= 32 || info->l_bit[0] >= 32) {
			ret = snprintf_s(line + offset,
				LINE_LEFT(offset), LINE_LEFT(offset) - 1,
				" %-17s %-10s", "NA", "NA");
			offset = update_line_offset(offset, ret);
		} else {
			ret = snprintf_s(line + offset, LINE_LEFT(offset),
				LINE_LEFT(offset) - 1, " 0x%08X[%2d:%-2d]",
				info->reg[0], info->h_bit[0], info->l_bit[0]);
			offset = update_line_offset(offset, ret);

			ret = snprintf_s(line + offset,
				LINE_LEFT(offset), LINE_LEFT(offset) - 1,
				" 0x%08X", info->reg_val[0]);
			offset = update_line_offset(offset, ret);
		}
	} else {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
				LINE_LEFT(offset) - 1, " %-11s %-17s %-10s",
				"NA", "NA", "NA");
		offset = update_line_offset(offset, ret);
	}

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-6d", info->enable_cnt);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-5d", info->prepare_cnt);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-3s\n",
			get_clock_matched_state(info->check_state));
	seq_clklog(s, "%s", line);

	for (i = 1; i < info->clock_reg_groups; i++) {
		offset = 0;
		(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%75s", "");
		offset = update_line_offset(offset, ret);

		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-6s%-5s", info->crg_region[i],
			(info->region_stat[i] ? "" : "[OFF]"));
		offset = update_line_offset(offset, ret);

		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " 0x%08X[%2d:%-2d]",
			info->reg[i], info->h_bit[i], info->l_bit[i]);
		offset = update_line_offset(offset, ret);

		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " 0x%08X\n", info->reg_val[i]);
		if (ret == -1)
			pr_err("%s info->reg_val[i] snprintf_s failed!\n", __func__);

		seq_clklog(s, "%s", line);
	}
}

#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
static int peri_power_up_ok(void)
{
	/* Peri should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
static int sctrl_power_up_ok(void)
{
	/* sctrl always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_PMC_BASE_ADDR)
static int pmctrl_power_up_ok(void)
{
	/* pmctrl should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
static int pctrl_power_up_ok(void)
{
	/* pctrl should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
static int media1_power_up_ok(void)
{
	int ret = 0;
#if defined(SOC_CRGPERIPH_PERRSTSTAT5_ADDR) && defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	unsigned int value;
	unsigned int hwaddr;

	hwaddr = SOC_CRGPERIPH_PERRSTSTAT5_ADDR(SOC_ACPU_PERI_CRG_BASE_ADDR);
	ret = read_reg_u32(hwaddr, &value);
	if (ret)
		return ret;
#if defined(SOC_CRGPERIPH_PERRSTSTAT5_ip_rst_media_crg_START)
	ret = !(value & BIT(SOC_CRGPERIPH_PERRSTSTAT5_ip_rst_media_crg_START));
#endif
#endif
	return ret;
}
#endif

#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
static int media2_power_up_ok(void)
{
	int ret = 0;
#if defined(SOC_CRGPERIPH_PERRSTSTAT4_ADDR) && defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	unsigned int value;
	unsigned int hwaddr;

	hwaddr = SOC_CRGPERIPH_PERRSTSTAT4_ADDR(SOC_ACPU_PERI_CRG_BASE_ADDR);
	ret = read_reg_u32(hwaddr, &value);
	if (ret)
		return ret;
#if defined(SOC_CRGPERIPH_PERRSTSTAT4_ip_rst_media2_crg_START)
	ret = !(value & BIT(SOC_CRGPERIPH_PERRSTSTAT4_ip_rst_media2_crg_START));
#endif
#endif
	return ret;
}
#endif

#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
static int iomcu_power_up_ok(void)
{
	int ret = 0;
#if defined(SOC_SCTRL_SCPWRSTAT_ADDR) && defined(SOC_ACPU_SCTRL_BASE_ADDR)
	unsigned int value;
	unsigned int hwaddr;

	hwaddr = SOC_SCTRL_SCPWRSTAT_ADDR(SOC_ACPU_SCTRL_BASE_ADDR);
	ret = read_reg_u32(hwaddr, &value);
	if (ret)
		return ret;
#if defined(SOC_SCTRL_SCPWRSTAT_iomcupwrstat_START)
	ret = (value & BIT(SOC_SCTRL_SCPWRSTAT_iomcupwrstat_START));
#endif
#endif
	return ret;
}
#endif

#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
static int mmc1sysctrl_power_up_ok(void)
{
	/* MMC1 should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
static int mmc0_power_up_ok(void)
{
	/* MMC0 should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
static int hsdt_power_up_ok(void)
{
	/* HSDT should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
static int hsdt1_power_up_ok(void)
{
	/* HSDT1 should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_SPMI_BASE_ADDR)
static int pmuspmi_power_up_ok(void)
{
	/* PMU/SPMI always on */
	return 1;
}
#endif

static int get_base_hwaddr(struct device_node *np)
{
	struct device_node *parent = NULL;
	const unsigned int *in_addr = NULL;
	u64 hwaddr;
	int i;

	parent = of_get_parent(np);
	if (!parent) {
		clklog_err("node %s doesn't have parent node!\n", np->name);
		return -EINVAL;
	}

	in_addr = of_get_address(parent, 0, NULL, NULL);
	if (!in_addr) {
		clklog_err("node %s of_get_address get I/O addr err!\n",
			parent->name);
		return -EINVAL;
	}
	hwaddr = of_translate_address(parent, in_addr);
	if (hwaddr == OF_BAD_ADDR) {
		clklog_err("node %s of_translate_address err!\n",
			parent->name);
		return -EINVAL;
	}
	of_node_put(parent);

	for (i = 0; i < crg_region_size; i++) {
		if (crg_regions[i].base_addr == (unsigned int)hwaddr)
			return i;
	}
	return -ENODEV;
}

#define CHECK_REGION_IDX(region_idx) \
	(((region_idx) < 0) || ((region_idx) >= crg_region_size))

static int check_fixed_rate(struct clock_check_param *param)
{
	unsigned int fixed_rate = 0;
	unsigned long rate  = 0;
	struct device_node *np = NULL;
	int ret = MISMATCH;
	struct clock_print_info pinfo;

	np = param->np;
	if (of_property_read_u32(np, HISI_FIXED_RATE_ATTR, &fixed_rate)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_FIXED_RATE_ATTR);
		return -EINVAL;
	}

	rate = clk_get_rate(param->clk->hw->clk);
	if (rate == (unsigned long)fixed_rate)
		ret = MATCH;

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_FIXED].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int __check_gate_state_matched(unsigned int reg_value,
	unsigned int gate_bitmask, unsigned int enable_count, bool always_on)
{
	int matched = MISMATCH;
	bool gate_onoff = ((reg_value & gate_bitmask) == gate_bitmask);
	bool enable_onoff = (enable_count > 0);

	if (gate_onoff && enable_onoff) {
		matched = MATCH;
	} else if (gate_onoff && !enable_onoff) {
		if (always_on)
			matched = MATCH;
		else
			matched = PARTIAL_MATCH;
	} else if (!gate_onoff && enable_onoff) {
		matched = MISMATCH;
	} else {
		matched = MATCH;
	}

	return matched;
}

#define GET_DVFS_DEVID_STR(is_dvfs_clock) \
	((is_dvfs_clock) ? "hisilicon,clk-devfreq-id" : "clock-id")
static void __print_dvfs_extra_info(struct clock_check_param *param,
		bool is_dvfs_clock)
{
	int ret;
	int offset = 0;
	int device_id = -1;
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	unsigned int hwaddr_pmctrl = 0;
#endif
	unsigned int ctrl4_data = 0;
	unsigned int ctrl5_data = 0;
	char line[MAX_STR_BUF_SIZE] = {0};

	if (of_property_read_u32(param->np, GET_DVFS_DEVID_STR(is_dvfs_clock),
					&device_id))
		return;

#if defined(SOC_ACPU_PMC_BASE_ADDR)
	hwaddr_pmctrl = crg_regions[HISI_CLOCK_CRG_PMCTRL].base_addr;
	(void)read_reg_u32(SOC_PMCTRL_PERI_CTRL4_ADDR(hwaddr_pmctrl),
				&ctrl4_data);
	(void)read_reg_u32(SOC_PMCTRL_PERI_CTRL5_ADDR(hwaddr_pmctrl),
				&ctrl5_data);
#endif

	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%76s", "");
	offset = update_line_offset(offset, ret);
	ret = snprintf_s(line + offset,
			LINE_LEFT(offset), LINE_LEFT(offset) - 1,
			"%-12s%-6dCTRL4:0x%08X CTRL5:0x%08X", "#DVFS-ID:",
			device_id, ctrl4_data, ctrl5_data);
	if (ret == -1)
		pr_err("%s device_id, ctrl4_data, ctrl5_data snprintf_s failed!\n", __func__);
	if (line[0] != '\0')
		seq_clklog(param->s, "%s\n", line);
}

static void __print_avs_extra_info(struct clock_check_param *param)
{
	int ret;
	int offset = 0;
	int avs_poll_id = -1;
#if defined(SOC_ACPU_SCTRL_BASE_ADDR) && defined(SOC_SCTRL_SCBAKDATA24_ADDR)
	unsigned int hwaddr_sctrl = 0;
#endif
	unsigned int avs_data = 0;
	char line[MAX_STR_BUF_SIZE] = {0};

	if (of_property_read_u32(param->np, "hisilicon,clk-avs-id",
					&avs_poll_id))
		return;

#if defined(SOC_ACPU_SCTRL_BASE_ADDR) && defined(SOC_SCTRL_SCBAKDATA24_ADDR)
	hwaddr_sctrl = crg_regions[HISI_CLOCK_CRG_SCTRL].base_addr;
	(void)read_reg_u32(SOC_SCTRL_SCBAKDATA24_ADDR(hwaddr_sctrl),
				&avs_data);
#endif

	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%76s", "");
	offset = update_line_offset(offset, ret);
	ret = snprintf_s(line + offset,
			LINE_LEFT(offset), LINE_LEFT(offset) - 1,
			"%-12s%-6dAVS:  0x%08X", "#AVS-ID:",
			avs_poll_id, avs_data);
	if (ret == -1)
		pr_err("%s avs_poll_id, avs_data snprintf_s failed!\n",
			__func__);
	if (line[0] != '\0')
		seq_clklog(param->s, "%s\n", line);
}

static void print_dvfs_avs_extra_info(struct clock_check_param *param,
		bool is_dvfs_clock)
{
	struct device_node *np = NULL;
	unsigned int freq_table[DVFS_MAX_FREQ_NUM] = {0};
	unsigned int volt_table[DVFS_MAX_FREQ_NUM+1] = {0};
	unsigned int sensitive_level = 0;
	char line[MAX_STR_BUF_SIZE] = {0};
	int offset = 0;
	int ret;
	unsigned int i;

	if (!IS_ENABLED(CONFIG_HISI_PERIDVFS))
		return;

	np = param->np;
	if (!is_dvfs_clock && !of_property_read_bool(np, "peri_dvfs_sensitive"))
		return;
	if (of_property_read_u32(np, "hisilicon,clk-dvfs-level",
			&sensitive_level))
		return;
	if (of_property_read_u32_array(np, "hisilicon,sensitive-freq",
			&freq_table[0], sensitive_level)) {
		return;
	}
	if (of_property_read_u32_array(np, "hisilicon,sensitive-volt",
			&volt_table[0], sensitive_level+1)) {
		return;
	}

	__print_dvfs_extra_info(param, is_dvfs_clock);
	__print_avs_extra_info(param);

	offset = 0;
	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%76s%-12s", "", "#DVFS-Freq:");
	offset = update_line_offset(offset, ret);
	for (i = 0; i < sensitive_level; i++) {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%-6dKHz ", freq_table[i]);
		offset = update_line_offset(offset, ret);
	}
	seq_clklog(param->s, "%s\n", line);

	offset = 0;
	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%76s%-12s", "", "#DVFS-Volt:");
	offset = update_line_offset(offset, ret);
	for (i = 0; i <= sensitive_level; i++) {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "VOLT_%-5d", volt_table[i]);
		offset = update_line_offset(offset, ret);
	}
	seq_clklog(param->s, "%s\n", line);
}

static int check_general_gate(struct clock_check_param *param)
{
	const unsigned int reg_stat_offset = 0x8;
	unsigned int gdata[2] = {0};
	unsigned long rate = 0;
	struct device_node *np = NULL;
	int ret = 0;
	int matched = MISMATCH;
	int region_idx;
	int region_stat;
	unsigned int hwaddr = 0;
	unsigned int value = 0;
	bool always_on = false;
	struct clock_print_info pinfo;

	np = param->np;
	if (of_property_read_u32_array(np, HISI_GENERAL_GATE_REG_ATTR,
			&gdata[0], 2)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_GENERAL_GATE_REG_ATTR);
		return -EINVAL;
	}

	if (of_property_read_bool(np, "always_on"))
		always_on = true;

	rate = clk_get_rate(param->clk->hw->clk);

	region_idx = get_base_hwaddr(np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
				crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	hwaddr = crg_regions[region_idx].base_addr + gdata[0];
	/* crg region powered up */
	if (region_stat && gdata[1]) {
		ret = read_reg_u32(hwaddr + reg_stat_offset, &value);
		if (ret)
			return ret;
		matched = __check_gate_state_matched(value,
				gdata[1], param->clk->enable_count, always_on);
	} else {
		/* fake gate clock Or crg region power down */
		matched = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_GENERAL_GATE].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = hwaddr + reg_stat_offset;
	pinfo.h_bit[0]           = fls(gdata[1]) - 1;
	pinfo.l_bit[0]           = ffs(gdata[1]) - 1;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = matched;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);
	print_dvfs_avs_extra_info(param, false);

	return (matched == MISMATCH ? MISMATCH : MATCH);
}

static int check_himask_gate(struct clock_check_param *param)
{
	unsigned int gdata[2] = {0};
	unsigned long rate = 0;
	struct device_node *np = NULL;
	int ret = 0;
	int matched = MISMATCH;
	int region_idx;
	int region_stat;
	unsigned int hwaddr, value = 0;
	bool always_on = false;
	struct clock_print_info pinfo;

	np = param->np;
	if (!of_property_read_bool(np, HISI_HIMASK_ATTR)) {
		clklog_err("node %s doesn't have %s property!",
					np->name, HISI_HIMASK_ATTR);
		return -EINVAL;
	}
	if (of_property_read_u32_array(np, HISI_HIMASK_GATE_REG_ATTR,
			&gdata[0], 2)) {
		clklog_err("node %s doesn't have %s property!",
					np->name, HISI_HIMASK_GATE_REG_ATTR);
		return -EINVAL;
	}
	if (of_property_read_bool(np, "always_on"))
		always_on = true;

	rate = clk_get_rate(param->clk->hw->clk);

	region_idx = get_base_hwaddr(np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
				crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	hwaddr = crg_regions[region_idx].base_addr + gdata[0];
	/* crg region powered up */
	if (region_stat) {
		ret = read_reg_u32(hwaddr, &value);
		if (ret)
			return ret;
		matched = __check_gate_state_matched(value, BIT(gdata[1]),
					param->clk->enable_count, always_on);
	} else {
		/* crg region power down */
		matched = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_HIMASK_GATE].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = hwaddr;
	pinfo.h_bit[0]           = gdata[1];
	pinfo.l_bit[0]           = gdata[1];
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = matched;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return (matched == MISMATCH ? MISMATCH : MATCH);
}

static int __check_mux_state_matched(unsigned int reg_value,
		unsigned int mux_bitmask, unsigned int parent_idx)
{
	int matched = MISMATCH;
	unsigned int low_bit = (unsigned int)ffs(mux_bitmask);

	if (low_bit == 0)
		return -EINVAL;
	low_bit -= 1;

	if ((int)parent_idx >= HISI_CLOCK_PARENT_IDX_OFFSET)
		parent_idx -= HISI_CLOCK_PARENT_IDX_OFFSET;

	if ((reg_value & mux_bitmask) == (parent_idx << low_bit))
		matched = MATCH;

	return matched;
}

static int check_mux_clock(struct clock_check_param *param)
{
	unsigned int gdata[2] = {0};
	unsigned long rate = 0;
	struct device_node *np = NULL;
	int ret = MISMATCH;
	int region_idx;
	int region_stat;
	unsigned int hwaddr, value = 0;
	struct clock_print_info pinfo;

	np = param->np;
	if (of_property_read_u32_array(np, HISI_MUX_REG_ATTR, &gdata[0], 2)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_MUX_REG_ATTR);
		return -EINVAL;
	}

	rate = clk_get_rate(param->clk->hw->clk);

	region_idx = get_base_hwaddr(np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
				crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	hwaddr = crg_regions[region_idx].base_addr + gdata[0];
	/* crg region powered up */
	if (region_stat) {
		ret = read_reg_u32(hwaddr, &value);
		if (ret)
			return ret;
		ret = __check_mux_state_matched(value, gdata[1],
				param->parent_idx);
	} else {
		/* crg region power down */
		ret = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_MUX].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = hwaddr;
	pinfo.h_bit[0]           = fls(gdata[1]) - 1;
	pinfo.l_bit[0]           = ffs(gdata[1]) - 1;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int __check_div_state_matched(unsigned int reg_value,
	unsigned int div_bitmask, unsigned long prate, unsigned long rate)
{
	int matched = MISMATCH;
	unsigned int low_bit = (unsigned int)ffs(div_bitmask);

	if (low_bit == 0)
		return -EINVAL;
	low_bit -= 1;

	if (!rate)
		return -EINVAL;

	if ((reg_value & div_bitmask) == ((prate/rate - 1) << low_bit))
		matched = MATCH;

	return matched;
}

static int check_div_clock(struct clock_check_param *param)
{
	unsigned int gdata[2] = {0};
	unsigned long rate = 0;
	unsigned long prate = 0;
	struct clk_core *pclk = NULL;
	struct device_node *np = NULL;
	int ret = MISMATCH;
	int region_idx;
	int region_stat;
	unsigned int hwaddr;
	unsigned int value = 0;
	struct clock_print_info pinfo;

	np = param->np;
	if (of_property_read_u32_array(np, HISI_DIV_REG_ATTR, &gdata[0], 2)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_DIV_REG_ATTR);
		return -EINVAL;
	}

	pclk = __clk_core_get_parent(param->clk);
	if (IS_ERR_OR_NULL(pclk)) {
		clklog_err("failed to get clk[%s] parent clock!\n",
			param->clk->name);
		return -EINVAL;
	}
	prate = clk_get_rate(pclk->hw->clk);
	rate = clk_get_rate(param->clk->hw->clk);
	if (!rate) {
		clklog_err("get clock[%s] rate is 0!\n", param->clk->name);
		return -EINVAL;
	}

	region_idx = get_base_hwaddr(np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
				crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	hwaddr = crg_regions[region_idx].base_addr + gdata[0];
	/* crg region powered up */
	if (region_stat) {
		ret = read_reg_u32(hwaddr, &value);
		if (ret)
			return ret;
		ret = __check_div_state_matched(value, gdata[1], prate, rate);
	} else {
		/* crg region power down */
		ret = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_DIV].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = hwaddr;
	pinfo.h_bit[0]           = fls(gdata[1]) - 1;
	pinfo.l_bit[0]           = ffs(gdata[1]) - 1;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int check_fixed_div_clock(struct clock_check_param *param)
{
	unsigned long rate = 0;
	unsigned long prate = 1;
	unsigned int div = 1;
	unsigned int mult = 1;
	struct clk_core *pclk = NULL;
	struct device_node *np = NULL;
	int ret = MISMATCH;
	struct clock_print_info pinfo;

	np = param->np;
	if (of_property_read_u32(np, HISI_FIXED_DIV_ATTR1, &div)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_FIXED_DIV_ATTR1);
		return -EINVAL;
	}
	if (of_property_read_u32(np, HISI_FIXED_DIV_ATTR2, &mult)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_FIXED_DIV_ATTR2);
		return -EINVAL;
	}

	pclk = __clk_core_get_parent(param->clk);
	if (IS_ERR_OR_NULL(pclk)) {
		clklog_err("cannot get clk[%s] parent clock!\n",
			param->clk->name);
		return -EINVAL;
	}
	prate = clk_get_rate(pclk->hw->clk);
	rate = clk_get_rate(param->clk->hw->clk);
	if (!rate) {
		clklog_err("get clock[%s] rate is 0!\n", param->clk->name);
		return -EINVAL;
	}

	if (div != 0 && rate == (prate * mult / div))
		ret = MATCH;

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_FIXED_DIV].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

/* check PPLL configs */
#define CHECK_KIRIN_PLL_CONFIG(param, ret, lock_bit) do {             \
	(void)read_reg_u32(hwaddr_ctrl + pll_ctrl0_addr, &pll_ctrl0); \
	(void)read_reg_u32(hwaddr_ctrl + pll_ctrl1_addr, &pll_ctrl1); \
	(void)read_reg_u32(hwaddr + en_addr[0], &pll_en);             \
	(void)read_reg_u32(hwaddr + gt_addr[0], &pll_gt);             \
	(void)read_reg_u32(hwaddr + bp_addr[0], &pll_bp);             \
	if ((pll_en & BIT(en_addr[1]))                                \
			&& (pll_gt & BIT(gt_addr[1]))                 \
			&& !(pll_bp & BIT(bp_addr[1]))                \
			&& (pll_ctrl0 & BIT(lock_bit))) {             \
		if ((param)->clk->enable_count > 0)                   \
			(ret) = MATCH;                                \
		else                                                  \
			(ret) = PARTIAL_MATCH;                        \
	} else {                                                      \
		if ((param)->clk->enable_count > 0)                   \
			(ret) = MISMATCH;                             \
		else                                                  \
			(ret) = MATCH;                                \
	}                                                             \
} while (0)

static int get_pll_ctrl_region_idx(struct device_node *np)
{
	int region_idx = -1;
	unsigned int pllctrl0_base = HS_PMCTRL;

	if (of_property_read_bool(np, "hisilicon,pll-ctrl0-base-reg")) {
		if (of_property_read_u32(np, "hisilicon,pll-ctrl0-base-reg",
				&pllctrl0_base)) {
			clklog_err("node %s doesn't have %s property!",
				np->name, HISI_PPLL_EN_ATTR);
			return -EINVAL;
		}
	}

	if (pllctrl0_base == HS_PMCTRL) {
#if defined(SOC_ACPU_PMC_BASE_ADDR)
		region_idx = HISI_CLOCK_CRG_PMCTRL;
#else
		/* No PMCTRL Region error */
		clklog_err("%s: No PMCTRL Region error!\n", __func__);
		return -EINVAL;
#endif
	} else if (pllctrl0_base == HS_CRGCTRL) {
#if defined(SOC_ACPU_PMC_BASE_ADDR)
		region_idx = HISI_CLOCK_CRG_PERI;
#else
		/* No CRGPERI Region error */
		clklog_err("%s: No CRGPERI Region error!\n", __func__);
		return -EINVAL;
#endif
	} else {
		/* CRG Region error */
		clklog_err("%s CRG Region error!\n", __func__);
		return -EINVAL;
	}

	return region_idx;
}

static int check_kirin_ppll_clock(struct clock_check_param *param)
{
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	const unsigned int PPLL_LOCK_BIT = 26;
#endif
	const unsigned int PLL_CTRL1_ADDR_SHIFT = 0x4;
	unsigned int en_addr[2] = {0};
	unsigned int gt_addr[2] = {0};
	unsigned int bp_addr[2] = {0};
	unsigned int pll_ctrl0_addr = 0;
	unsigned int pll_ctrl1_addr = 0;
	unsigned int hwaddr = 0;
	unsigned int hwaddr_ctrl = 0;
	unsigned int pll_ctrl0 = 0;
	unsigned int pll_ctrl1 = 0;
	unsigned int pll_en = 0;
	unsigned int pll_gt = 0;
	unsigned int pll_bp = 0;
	int ret = MISMATCH;
	int region_idx = 0;
	int pll_ctrl_region_idx = 0;
	struct device_node *np = NULL;
	struct clock_print_info pinfo = {0};

	pinfo.clock_reg_groups = 0;
	np = param->np;
	if (of_property_read_u32_array(np, HISI_PPLL_EN_ATTR,
			&en_addr[0], 2)) {
		clklog_debug("node %s doesn't have %s property!",
				np->name, HISI_PPLL_EN_ATTR);
		goto out;
	}
	if (of_property_read_u32_array(np, HISI_PPLL_GT_ATTR,
			&gt_addr[0], 2)) {
		clklog_debug("node %s doesn't have %s property!",
				np->name, HISI_PPLL_GT_ATTR);
		goto out;
	}
	if (of_property_read_u32_array(np, HISI_PPLL_BP_ATTR,
			&bp_addr[0], 2)) {
		clklog_debug("node %s doesn't have %s property!",
				np->name, HISI_PPLL_BP_ATTR);
		goto out;
	}
	if (of_property_read_u32(np, HISI_PPLL_CTRL0_ATTR,
			&pll_ctrl0_addr)) {
		clklog_debug("node %s doesn't have %s property!",
				np->name, HISI_PPLL_CTRL0_ATTR);
		goto out;
	}
	pll_ctrl1_addr = pll_ctrl0_addr + PLL_CTRL1_ADDR_SHIFT;

	region_idx = get_base_hwaddr(np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	pll_ctrl_region_idx = get_pll_ctrl_region_idx(np);
	if (pll_ctrl_region_idx < 0)
		return -EINVAL;
	hwaddr_ctrl = crg_regions[pll_ctrl_region_idx].base_addr;
	hwaddr = crg_regions[region_idx].base_addr;
	CHECK_KIRIN_PLL_CONFIG(param, ret, PPLL_LOCK_BIT);
	pinfo.clock_reg_groups   = CLOCK_REG_GROUPS;

out:
	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_PPLL].alias;
	pinfo.rate               = clk_get_rate(param->clk->hw->clk);
	if (pinfo.clock_reg_groups == CLOCK_REG_GROUPS) {
		pinfo.clock_reg_groups = CLOCK_REG_GROUPS;
		pinfo.crg_region[0]    = crg_regions[region_idx].crg_name;
		pinfo.region_stat[0]   = 1;
		pinfo.reg[0]           = hwaddr + en_addr[0];
		pinfo.h_bit[0]         = en_addr[1];
		pinfo.l_bit[0]         = en_addr[1];
		pinfo.reg_val[0]       = pll_en;

		pinfo.crg_region[1]    = crg_regions[region_idx].crg_name;
		pinfo.region_stat[1]   = 1;
		pinfo.reg[1]           = hwaddr + bp_addr[0];
		pinfo.h_bit[1]         = bp_addr[1];
		pinfo.l_bit[1]         = bp_addr[1];
		pinfo.reg_val[1]       = pll_bp;

		pinfo.crg_region[2]    = crg_regions[region_idx].crg_name;
		pinfo.region_stat[2]   = 1;
		pinfo.reg[2]           = hwaddr + gt_addr[0];
		pinfo.h_bit[2]         = gt_addr[1];
		pinfo.l_bit[2]         = gt_addr[1];
		pinfo.reg_val[2]       = pll_gt;

		pinfo.crg_region[3]    = crg_regions[pll_ctrl_region_idx].crg_name;
		pinfo.region_stat[3]   = 1;
		pinfo.reg[3]           = hwaddr_ctrl + pll_ctrl0_addr;
		pinfo.h_bit[3]         = 31;
		pinfo.l_bit[3]         = 0;
		pinfo.reg_val[3]       = pll_ctrl0;

		pinfo.crg_region[4]    = crg_regions[pll_ctrl_region_idx].crg_name;
		pinfo.region_stat[4]   = 1;
		pinfo.reg[4]           = hwaddr_ctrl + pll_ctrl1_addr;
		pinfo.h_bit[4]         = 31;
		pinfo.l_bit[4]         = 0;
		pinfo.reg_val[4]       = pll_ctrl1;
	} else if (pinfo.clock_reg_groups == 0) {
		ret = MATCH;
	}
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return (ret == MISMATCH ? MISMATCH : MATCH);
}

static int check_hisi_pll_clock(struct clock_check_param *param)
{
	unsigned int pll_ctrl0 = 0;
	unsigned int pll_ctrl1 = 0;
	unsigned int pll_lock_stat = 0;
	unsigned int hwaddr = 0;
	int region_idx = 0;
	int region_stat = 0;
	struct hi3xxx_ppll_clk *ppll_clk = NULL;
	struct clock_print_info pinfo = {0};

	ppll_clk = container_of(param->clk->hw, struct hi3xxx_ppll_clk, hw);
	pinfo.clock_reg_groups = 0;
	if (ppll_clk->en_cmd[1] != SCPLL)
		goto out;

	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
				crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	hwaddr = crg_regions[region_idx].base_addr;
	if (region_stat) {
		pinfo.clock_reg_groups = 3;
		pll_ctrl0 = readl(SCPLL_GT_ACPU_ADDR(ppll_clk->addr));
		pll_ctrl1 = readl(SCPLL_EN_ACPU_ADDR(ppll_clk->addr));
		pll_lock_stat = readl(SCPLL_BP_ACPU_ADDR(ppll_clk->addr));
	}

out:
	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_PLL].alias;
	pinfo.rate               = clk_get_rate(param->clk->hw->clk);

	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
	pinfo.reg[0]             = SCPLL_EN_ACPU_ADDR(hwaddr);
#endif
	pinfo.h_bit[0]           = 31;
	pinfo.l_bit[0]           = 0;
	pinfo.reg_val[0]         = pll_ctrl0;

	pinfo.crg_region[1]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[1]     = region_stat;
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
	pinfo.reg[1]             = SCPLL_GT_ACPU_ADDR(hwaddr);
#endif
	pinfo.h_bit[1]           = 31;
	pinfo.l_bit[1]           = 0;
	pinfo.reg_val[1]         = pll_ctrl1;

	pinfo.crg_region[2]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[2]     = region_stat;
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
	pinfo.reg[2]             = SCPLL_LOCK_STAT(hwaddr);
#endif
	pinfo.h_bit[2]           = 31;
	pinfo.l_bit[2]           = 0;
	pinfo.reg_val[2]         = pll_lock_stat;

	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = MATCH;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return MATCH;
}

static int check_pmu_clock(struct clock_check_param *param)
{
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
	unsigned int gdata[2] = {0};
	unsigned long rate = 0;
	struct device_node *np;
	int matched = MISMATCH;
	int region_idx;
	unsigned int hwaddr, value = 0;
	struct clock_print_info pinfo;
	int pmu_state = 1;

	np = param->np;
	if (!of_property_read_bool(np, HISI_PMU_ATTR)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_PMU_ATTR);
		return -EINVAL;
	}
	if (of_property_read_u32_array(np, HISI_HIMASK_GATE_REG_ATTR,
			&gdata[0], 2)) {
		clklog_err("node %s doesn't have %s property!",
				np->name, HISI_HIMASK_GATE_REG_ATTR);
		return -EINVAL;
	}

	rate = clk_get_rate(param->clk->hw->clk);
	if (!rate) {
		clklog_err("get clock[%s] rate is 0!\n", param->clk->name);
		return -EINVAL;
	}

	region_idx = get_base_hwaddr(np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("get clk[%s] [SPMI/PMU: 0x%x] base addr wrong!\n",
			param->clk->name,
			crg_regions[HISI_CLOCK_CRG_PMU].base_addr);
		region_idx = HISI_CLOCK_CRG_PMU;
	}

	hwaddr = crg_regions[region_idx].base_addr;
	value = hisi_pmic_reg_read(gdata[0]);
	if ((int)value < 0) {
		clklog_err("Access PMU Timeout (0x%x), ret=%d\n",
			gdata[0], (int)value);
		pmu_state = 0;
		matched       = MATCH;
	} else {
		matched = __check_gate_state_matched(value, BIT(gdata[1]),
				param->clk->enable_count, false);
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_PMU].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = pmu_state;
	pinfo.reg[0]             = gdata[0];
	pinfo.h_bit[0]           = gdata[1];
	pinfo.l_bit[0]           = gdata[1];
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = matched;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return (matched == MISMATCH ? MISMATCH : MATCH);
#else
	return MATCH;
#endif
}

static int check_ipc_clock(struct clock_check_param *param)
{
	int ret = MATCH;
	unsigned long rate = 0;
	struct clock_print_info pinfo;

	rate = clk_get_rate(param->clk->hw->clk);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_IPC].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int check_root_clock(struct clock_check_param *param)
{
	int ret = MATCH;
	unsigned long rate = 0;
	struct clock_print_info pinfo;

	rate = clk_get_rate(param->clk->hw->clk);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_ROOT].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int hisi_clock_check(struct seq_file *s,
		struct clk_core *clk, unsigned int level, int parent_idx)
{
	int ret = 0;
	int i;
	struct clock_check_param param;
	const char *compatible = NULL;

	param.s = s;
	param.clk = clk;
	param.level = level;
	param.parent_idx = parent_idx;
	param.np = of_find_node_by_name(NULL, clk->name);
	if (IS_ERR_OR_NULL(param.np)) {
		clklog_err("node %s doesn't find!\n", clk->name);
		return -EINVAL;
	}

	if (of_property_read_string(param.np, "compatible", &compatible)) {
		clklog_err("node %s doesn't have compatible property!\n",
				param.np->name);
		return -EINVAL;
	}

	for (i = 0; i < clock_type_size; i++) {
		if (strcmp(compatible, clock_types[i].name) == 0) {
			if (clock_types[i].check_func)
				ret = clock_types[i].check_func(&param);
			return ret;
		}
	}

	clklog_err("clk %s doesn't find!\n", clk->name);
	return -ENODEV;
}

static int find_hisi_clock_parent_idx(struct clk_core *clk)
{
	int i;
	struct clk_core *parent = NULL;

	if (IS_ERR_OR_NULL(clk))
		return -1;

	for (i = 0; i < clk->num_parents; i++) {
		parent = __clk_core_get_parent(clk);
		if (parent == NULL) {
			clklog_err("hisi clock struct err!\n");
			return -EINVAL;
		}
		if (strcmp(parent->name, clk->parent_names[i]) == 0)
			return i;
	}

	return 0;
}

static int find_all_clock_sources(struct seq_file *s,
		struct clk_core *clk, unsigned int level, int parent_idx)
{
	int ret = 0;
	int i;
	struct clk_core *parent = NULL;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	/* Consistency check and print clk state */
	ret += hisi_clock_check(s, clk, level, parent_idx);

	/* recursive finding all source clock sources */
	for (i = 0; i < clk->num_parents; i++) {
		struct clk *tclk = NULL;
		int pp_idx = 0;

		parent = __clk_core_get_parent(clk);
		if (parent == NULL) {
			clklog_err("hisi clock struct err!\n");
			return -EINVAL;
		}

		if (strcmp(parent->name, clk->parent_names[i]) == 0) {
			if (parent_idx >= HISI_CLOCK_PARENT_IDX_OFFSET)
				pp_idx = HISI_CLOCK_PARENT_IDX_OFFSET;
		} else if (strcmp("clk_invalid", clk->parent_names[i]) == 0) {
			continue; /*ignore clk_invalid*/
		} else {
			tclk = __clk_lookup(clk->parent_names[i]);
			parent = (tclk != NULL) ? tclk->core : NULL;
		}

		pp_idx += find_hisi_clock_parent_idx(parent);
		ret += find_all_clock_sources(s, parent, level + 1, pp_idx);
	}

	return ret;
}

static int find_single_clock_sources(struct seq_file *s,
		struct clk_core *clk, unsigned int level)
{
	int ret = 0;
	int i;

	while (!IS_ERR_OR_NULL(clk)) {
		if (clk->num_parents > 0) {
			/* traveral all parent clockss */
			for (i = 0; i < clk->num_parents; i++) {
				struct clk_core *pclk = __clk_core_get_parent(clk);

				if (!IS_ERR_OR_NULL(pclk) &&
						strcmp(pclk->name, clk->parent_names[i]) == 0) {
					/* Consistency check and print clk state */
					ret += hisi_clock_check(s, clk, level, i);
					break;
				}
			}
		} else {
			/* Consistency check and print clk state */
			ret += hisi_clock_check(s, clk, level, -1);
		}

		/* upward to clock source */
		clk = __clk_core_get_parent(clk);
		level++;
	}

	return ret;
}

static int __find_hisi_clock_path(struct seq_file *s,
			struct clk_core *clk, bool leading, bool recur)
{
	const char *ONE_LINE_STR =
		"---------------------------------------------------------------------"
		"------------------------------------------------------------------\n";
	int ret = 0;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	/* leading of table */
	if (leading) {
		seq_clklog(s, "%14s%47s%8s%13s%9s%24s%8s%6s%6s\n",
			   "clock name", "clock type", "rate", "region",
			   "reg", "reg value", "en_cnt", "p_cnt", "state");
		seq_clklog(s, "%s", ONE_LINE_STR);
	}

	if (recur)
		ret = find_all_clock_sources(s, clk, 0,
			find_hisi_clock_parent_idx(clk) + HISI_CLOCK_PARENT_IDX_OFFSET);
	else
		ret = find_single_clock_sources(s, clk, 0);

	/* last of table */
	if (leading) {
		seq_clklog(s, "%s", ONE_LINE_STR);
		seq_clklog(s, "[++ Clock: %s ++][++ RET: %d ++][++ TEST RESULT: %s ++]\n",
			   clk->name, ret, (ret == 0 ? "Success" : "Failure"));
	}

	return ret;
}

static int find_hisi_clock_path(struct seq_file *s,
			const char *clock_name, bool leading, bool recur)
{
	struct clk_core *clk = NULL;

	if (IS_ERR_OR_NULL(clock_name))
		return -EINVAL;

	clk = hisi_find_clock(clock_name);
	if (IS_ERR_OR_NULL(clk)) {
		clklog_err("clock %s doesn't find!\n", clock_name);
		return -EINVAL;
	}

	return __find_hisi_clock_path(s, clk, leading, recur);
}

static int check_dvfs_clock(struct clock_check_param *param)
{
	int ret = MATCH;
	unsigned long rate = 0;
	struct clock_print_info pinfo;
	const char *clk_friend = NULL;

	rate = clk_get_rate(param->clk->hw->clk);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[HISI_CLOCK_DVFS].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);
	print_dvfs_avs_extra_info(param, true);

	if (of_property_read_string(param->np, "clock-friend-names",
			&clk_friend)) {
		clklog_err("DVFS clock %s no friend clock!\n",
			param->clk->name);
		return -EINVAL;
	}

	return find_hisi_clock_path(param->s, clk_friend, false,
			(param->parent_idx >= HISI_CLOCK_PARENT_IDX_OFFSET));
}

/****************** clock test plan begin *********************/
#define MAX_CLOCK_RATE_NUM     8
#define MAX_CLOCK_NAME_LEN     64
struct clock_test_plan {
	char clock_name[MAX_CLOCK_NAME_LEN];
	unsigned long rates[MAX_CLOCK_RATE_NUM + 1];
};
static const char *clock_test_plan_help =
	"\n[Help Tips]:\nPlease use test_plan debugfs as follows:\n"
	"1) echo <clkname> [rate1] [rate2]...> /d/clock/test_one_clock/test_plan\n"
	"                       {NOTE: only support max to 8 clock rates!}\n"
	"2) cat /d/clock/test_one_clock/test_plan\n\n";
static DEFINE_MUTEX(clock_plan_lock);
static struct clock_test_plan *clock_plan;

#define CHECK_FIND_CLOCK_PATH(_s, _rc)  do {                          \
	if (_rc) {                                                    \
		ret = 1;                                              \
		seq_clklog((_s), "clock[%s] path check err! rc=%d\n", \
				plan->clock_name, (_rc));             \
	}                                                             \
} while (0)

static int test_clock_plan(struct seq_file *s, struct clock_test_plan *plan)
{
	struct clk_core *clk = NULL;
	int ret = 0;
	int rc = 0;
	unsigned int i = 0;
	unsigned long old_rate = 0;

	clk = hisi_find_clock(plan->clock_name);
	if (IS_ERR_OR_NULL(clk)) {
		seq_clklog(s, "find clock: %s err!\n", plan->clock_name);
		return -EINVAL;
	}

	old_rate = clk_get_rate(clk->hw->clk);
	seq_clklog(s, "clock[%s] get old_rate[%lu]\n",
			plan->clock_name, old_rate);

	seq_clklog(s, "clock[%s] prepare and enable\n", plan->clock_name);
	ret = clk_prepare_enable(clk->hw->clk);
	if (ret) {
		seq_clklog(s, "clock[%s] prepare and enable err! ret=%d\n",
				plan->clock_name, ret);
		return ret;
	}
	rc = find_hisi_clock_path(s, plan->clock_name, true, false);
	CHECK_FIND_CLOCK_PATH(s, rc);

	for (i = 0; i <= MAX_CLOCK_RATE_NUM; i++) {
		if (!plan->rates[i]) {
			/* if last rate is equal to old_rate, ignore */
			if (i > 0 && plan->rates[i - 1] != old_rate)
				plan->rates[i] = old_rate;
			break;
		}
	}
	for (i = 0; i <= MAX_CLOCK_RATE_NUM && plan->rates[i]; i++) {
		seq_clklog(s, "********CLOCK[%s] RATE[%lu] TESTING ********\n",
				plan->clock_name, plan->rates[i]);
		rc = clk_set_rate(clk->hw->clk, plan->rates[i]);
		if (rc < 0) {
			ret = 1;
			seq_clklog(s, "clock[%s] set rate %lu err! rc=%d\n",
				plan->clock_name, plan->rates[i], rc);
		}
		rc = find_hisi_clock_path(s, plan->clock_name, true, false);
		CHECK_FIND_CLOCK_PATH(s, rc);
	}

	seq_clklog(s, "clock[%s] disable and unprepare:\n", plan->clock_name);
	clk_disable_unprepare(clk->hw->clk);
	rc = find_hisi_clock_path(s, plan->clock_name, true, false);
	CHECK_FIND_CLOCK_PATH(s, rc);

	return ret;
}

static int clk_testplan_show(struct seq_file *s, void *data)
{
	mutex_lock(&clock_plan_lock);
	if (IS_ERR_OR_NULL(clock_plan)) {
		seq_clklog(s, "No clock test plan buffered!\n");
		seq_clklog(s, "%s", clock_test_plan_help);
		goto out;
	}
	test_clock_plan(s, clock_plan);

	if (s->size > 0 && s->size == s->count) {
		clklog_info("resize the seq_file buffer size (%u -> %u)!\n",
				(u32)s->size, (u32)s->size * 2);
		goto end;
	}

out:
	kfree(clock_plan);
	clock_plan = NULL;

end:
	mutex_unlock(&clock_plan_lock);

	return 0;
}

int clk_testplan_single_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_testplan_show, inode->i_private);
}

static int __analysis_clock_test_plan_string(char *str, int ret_val)
{
	char *t_str = NULL;
	int i;

	for (i = 0; str != NULL && *str && i <= MAX_CLOCK_RATE_NUM; i++) {
		t_str = strchr(str, ' ');
		if (t_str != NULL) {
			*t_str = '\0';
			t_str++;
		}

		if (i == 0) {
			if (strcpy_s(clock_plan->clock_name,
					MAX_CLOCK_NAME_LEN, str) != EOK) {
				clklog_err("clock name %s is too long!\n", str);
				return -EINVAL;
			}
			clklog_info("clock name: %s\n", clock_plan->clock_name);
		} else {
			unsigned long rate = simple_strtoul(str, NULL, 10);

			if (!rate) {
				clklog_err("please input valid clk rate!\n");
				return -EINVAL;
			}
			clock_plan->rates[i - 1] = rate;
			clklog_info("clock rate-%d: %lu\n", i,
					clock_plan->rates[i - 1]);
		}

		str = t_str;
	}

	return ret_val;
}

ssize_t clk_testplan_write(struct file *filp, const char __user *ubuf,
			size_t cnt, loff_t *ppos)
{
	const unsigned int MAX_STRING_LEN = 128;
	char *clkname_freq_str = NULL;
	int ret = cnt;

	if (cnt == 0 || ubuf == NULL) {
		clklog_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (cnt >= MAX_STRING_LEN) {
		clklog_err("Input string is too long.\n");
		return -EINVAL;
	}

	mutex_lock(&clock_plan_lock);
	if (clock_plan == NULL) {
		clock_plan = kzalloc(sizeof(struct clock_test_plan), GFP_KERNEL);
		if (clock_plan == NULL) {
			clklog_err("Cannot allocate clk_name.\n");
			ret = -EINVAL;
			goto out;
		}
	} else {
		clklog_err("clock %s does not finish test_plan!\n",
				clock_plan->clock_name);
		goto out;
	}

	clkname_freq_str = kzalloc(cnt * sizeof(char), GFP_KERNEL);
	if (clkname_freq_str == NULL) {
		clklog_err("Cannot allocate string buffer.\n");
		ret = -EINVAL;
		goto out;
	}

	/* copy clock name from user space(the last symbol is newline). */
	if (copy_from_user(clkname_freq_str, ubuf, cnt - 1)) {
		clklog_err("Input string is too long.\n");
		ret = -EINVAL;
		goto out;
	}
	clkname_freq_str[cnt - 1] = '\0';
	ret = __analysis_clock_test_plan_string(clkname_freq_str, cnt);

out:
	mutex_unlock(&clock_plan_lock);
	if (clkname_freq_str != NULL)
		kfree(clkname_freq_str);
	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
const struct file_operations clk_testplan_fops = {
	.open    = clk_testplan_single_open,
	.read    = seq_read,
	.write   = clk_testplan_write,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif
/****************** clock test plan end *********************/

/****************** get clock path begin *********************/
static DEFINE_MUTEX(leaf_clock_name_lock);
static const int MAX_CLKNAME_LEN = 128;
static char *leaf_clock_name;
static const char *clock_get_path_help =
	"\nHelp Tips:\nPlease use get_path debugfs as follows:\n"
	"1) echo {clock name} [string] > /d/clock/test_one_clock/get_path\n"
	"2) cat /d/clock/test_one_clock/get_path\n\n";

static int clk_getpath_show(struct seq_file *s, void *data)
{
	int rc;
	bool recur = false;
	char *t_str = NULL;

	mutex_lock(&leaf_clock_name_lock);
	if (leaf_clock_name == NULL) {
		seq_clklog(s, "No clock name buffered!\n");
		seq_clklog(s, "%s", clock_get_path_help);
		goto out;
	}

	t_str = strchr(leaf_clock_name, ' ');
	if (t_str != NULL) {
		*t_str = '\0';
		recur = true;
	}
	rc = find_hisi_clock_path(s, leaf_clock_name, true, recur);

	if (s->size > 0 && s->size == s->count) {
		clklog_info("resize the seq_file buffer size (%u -> %u)!\n",
				(u32)s->size, (u32)s->size * 2);
		/* restore leaf_clock_name string */
		if (t_str != NULL)
			*t_str = ' ';
		goto end;
	}

out:
	kfree(leaf_clock_name);
	leaf_clock_name = NULL;

end:
	mutex_unlock(&leaf_clock_name_lock);
	return 0;
}

int clk_getpath_single_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_getpath_show, inode->i_private);
}

ssize_t clk_getpath_write(struct file *filp, const char __user *ubuf,
		size_t cnt, loff_t *ppos)
{
	int ret = cnt;

	if (cnt == 0 || ubuf == NULL) {
		clklog_err("Input string is NULL.\n");
		return -EINVAL;
	}

	if (cnt >= MAX_CLKNAME_LEN) {
		clklog_err("Input string is too long.\n");
		return -EINVAL;
	}

	mutex_lock(&leaf_clock_name_lock);
	if (leaf_clock_name == NULL) {
		leaf_clock_name = kzalloc(sizeof(char) * MAX_CLKNAME_LEN,
				GFP_KERNEL);
		if (leaf_clock_name == NULL) {
			clklog_err("kzalloc leaf_clock_name failed!\n");
			ret = -EINVAL;
			goto out;
		}
	} else {
		clklog_err("clock %s does not finish get_path!\n",
			leaf_clock_name);
		goto out;
	}

	/* copy clock name from user space(the last symbol is newline). */
	if (copy_from_user(leaf_clock_name, ubuf, cnt - 1)) {
		clklog_err("Input string is too long.\n");
		ret = -EINVAL;
		goto out;
	}
	clklog_info("clock buffer: %s\n", leaf_clock_name);
out:
	mutex_unlock(&leaf_clock_name_lock);
	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
const struct file_operations clk_getpath_fops = {
	.open    = clk_getpath_single_open,
	.read    = seq_read,
	.write   = clk_getpath_write,
	.llseek  = seq_lseek,
	.release = single_release,
};
#endif
/****************** get clock path end *********************/

int hisi_clk_debug_init(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	struct dentry *pdentry = NULL;

	clock = debugfs_create_dir("clock", NULL);

	if (!clock)
		return -ENOMEM;

	test_one_clock = debugfs_create_dir("test_one_clock", clock);
	if (!test_one_clock)
		return -ENOMEM;

	test_all_clocks = debugfs_create_dir("test_all_clocks", clock);
	if (!test_all_clocks)
		return -ENOMEM;

	debugfs_create_file("clock_tree", S_IRUGO, clock,
			NULL, &clock_tree_show_fops);

	pdentry = debugfs_create_file("clk_reg_summary", S_IRUGO, clock, &clk_all_lists_debug,
				&clk_reg_summary_fops);
	if (!pdentry)
		return -ENOMEM;

	#define PRIV_AUTH	(S_IRUSR|S_IWUSR|S_IRGRP)
	pdentry = test_all_clocks;
	debugfs_create_file("clk_get", PRIV_AUTH, pdentry, NULL, &clock_get_show_fops);
	debugfs_create_file("clk_lookup", PRIV_AUTH, pdentry, NULL, &clock_lookup_show_fops);
	debugfs_create_file("clk_enable", PRIV_AUTH, pdentry, NULL, &clock_enable_show_fops);
	debugfs_create_file("clk_disable", PRIV_AUTH, pdentry, NULL, &clock_disable_show_fops);
	debugfs_create_file("clk_getparent", PRIV_AUTH, pdentry, NULL, &clock_getparent_show_fops);
	debugfs_create_file("clk_getrate", PRIV_AUTH, pdentry, NULL, &clock_getrate_show_fops);
	debugfs_create_file("clock_getleaf", PRIV_AUTH, pdentry, NULL, &clock_getleaf_show_fops);

	pdentry = test_one_clock;
	#define PRIV_MODE	(S_IWUSR|S_IWGRP)
	debugfs_create_file("enable", PRIV_MODE, pdentry, NULL, &clock_enable_fops);
	debugfs_create_file("disable", PRIV_MODE, pdentry, NULL, &clock_disable_fops);
	debugfs_create_file("get_parent", PRIV_MODE, pdentry, NULL, &clock_getparent_fops);
	debugfs_create_file("set_parent", PRIV_MODE, pdentry, NULL, &clock_setparent_fops);
	debugfs_create_file("get_rate", PRIV_MODE, pdentry, NULL, &clock_getrate_fops);
	debugfs_create_file("set_rate", PRIV_MODE, pdentry, NULL, &clock_setrate_fops);
	debugfs_create_file("get_reg", PRIV_MODE, pdentry, NULL, &clock_getreg_fops);
	debugfs_create_file("get_path", PRIV_AUTH, pdentry, NULL, &clk_getpath_fops);
	debugfs_create_file("clk_testplan", PRIV_AUTH, pdentry, NULL, &clk_testplan_fops);

#endif
	return 0;
}

void clk_list_add(struct clk_core *clk)
{
	mutex_lock(&clock_list_lock);
	list_add(&clk->node, &clocks);
	mutex_unlock(&clock_list_lock);
	return;
}

struct dentry *
debugfs_create_clkfs(struct clk_core *clk)
{
#ifdef CONFIG_HISI_DEBUG_FS
	return debugfs_create_file("clk_rate", S_IWUSR | S_IRUGO, clk->dentry, clk, &clk_rate_fops);
#else
	return NULL;
#endif
}

#ifdef CONFIG_HISI_CLK_TRACE
static void __track_clk(struct clk *clk, track_clk_type track_item, u32 freq)
{
	clk_record_info info;
	u8 cpu;
	const char *clk_name = NULL;

	if (!g_clk_hook_on)
		return;
	if (track_item >= TRACK_CLK_MAX) {
		pr_err("[%s], track_type [%d] is invalid!\n", __func__, track_item);
		return;
	}

	cpu = (u8) raw_smp_processor_id();
	if (SINGLE_BUFF == clk_sel_buf_type)
		cpu = 0;

	info.current_time    = hisi_getcurtime();
	info.item            = track_item;
	info.enable_count    = __clk_get_enable_count(clk);
	info.current_rate    = __clk_get_rate(clk);
	info.cpu_l           = __clk_get_rate(hs_trace_info.clk_cpu_l);
	info.cpu_b           = __clk_get_rate(hs_trace_info.clk_cpu_b);
	if (0 > __clk_get_source(clk))
		info.ppll    = CLK_ERR_NUM;
	else
		info.ppll    = __clk_get_source(clk);
	info.ddr_freq        = __clk_get_rate(hs_trace_info.clk_ddr);
	info.peri_dvfs_vote0 = readl(CLK_DVFS_ADDR_0(hs_trace_info.pmuctrl));
	info.peri_dvfs_vote1 = readl(CLK_DVFS_ADDR_1(hs_trace_info.pmuctrl));

	clk_name = __clk_get_name(clk);
	if (!clk_name)
		return;
	if (memset_s(info.comm, CLK_NAME_LEN, 0, sizeof(info.comm)) != EOK) {
		pr_err("%s memset_s failed!\n", __func__);
		return;
	}
	if (strncpy_s(info.comm, CLK_NAME_LEN, clk_name, strlen(clk_name)) != EOK) {
		pr_err("%s strncpy_s failed!\n", __func__);
		return;
	}
	info.comm[CLK_NAME_LEN - 1] = '\0';
	pr_debug("######%s!\n", info.comm);
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)g_clk_track_addr->percpu_addr[cpu], (u8 *)&info);
}

void track_clk_enable(struct clk *clk)
{
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	__track_clk(clk, TRACK_ON_OFF, 0);
}
void track_clk_set_freq(struct clk *clk, u32 freq)
{
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	__track_clk(clk, TRACK_SET_FREQ, freq);
}
void track_clk_set_dvfs(struct clk *clk, u32 freq)
{
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	__track_clk(clk, TRACK_SET_DVFS, freq);
}
static void track_clk_reset(u32 modid, u32 etype, u64 coreid)
{
	return;
}

static void track_clk_dump(u32 modid, u32 etype, u64 coreid,
		char *pathname, pfn_cb_dump_done pfn_cb)
{
	if (pfn_cb) {
		pfn_cb(modid, coreid);
	}
	pr_info("%s dump!\n", __func__);
}

static int track_clk_rdr_register(struct rdr_register_module_result *result)
{
	struct rdr_module_ops_pub s_module_ops;
	int ret = -1;
	pr_info("%s start!\n", __func__);
	if (!result) {
		pr_err("%s para null!\n", __func__);
		return ret;
	}
	s_module_ops.ops_dump  = track_clk_dump;
	s_module_ops.ops_reset = track_clk_reset;
	ret = rdr_register_module_ops(clk_rdr_core_id, &s_module_ops, result);
	pr_info("%s end!\n", __func__);
	return ret;
}

#define ALIGN8(size) ((size/8)*8)

int clk_percpu_buffer_init(u8 *addr, u32 size, u32 fieldcnt, u32 magic_number, u32 ratio[][8], buf_type_en buf_type)
{
	int i, ret;
	u32 cpu_num = num_possible_cpus();

	if (SINGLE_BUFF == buf_type)
		cpu_num = 1;
	pr_info("[%s], num_online_cpus [%d] !\n", __func__, num_online_cpus());

	if (IS_ERR_OR_NULL(addr) || IS_ERR_OR_NULL(addr)) {
		pr_err("[%s], buffer_addr [0x%pK], buffer_size [0x%x]\n", __func__, addr, size);
		return -1;
	}

	/* set pc info for parse */
	g_clk_track_addr              = (pc_record_info *)addr;
	g_clk_track_addr->buffer_addr = addr;
	g_clk_track_addr->buffer_size = size - sizeof(pc_record_info);
	g_clk_track_addr->dump_magic  = magic_number;

	/* set per cpu buffer */
	for (i = 0; i < cpu_num; i++) {/*lint !e574*/
		pr_info("[%s], ratio[%d][%d] = [%d]\n", __func__,
		       (cpu_num - 1), i, ratio[cpu_num - 1][i]);

		g_clk_track_addr->percpu_length[i] = g_clk_track_addr->buffer_size / 16 * ratio[cpu_num - 1][i];
		g_clk_track_addr->percpu_length[i] = ALIGN8(g_clk_track_addr->percpu_length[i]);

		if (0 == i) {
			g_clk_track_addr->percpu_addr[0] = g_clk_track_addr->buffer_addr + sizeof(pc_record_info);
		} else {
			g_clk_track_addr->percpu_addr[i] =
			g_clk_track_addr->percpu_addr[i - 1] + g_clk_track_addr->percpu_length[i - 1];
		}

		pr_info("[%s], [%d]: percpu_addr [0x%pK], percpu_length [0x%x], fieldcnt [%d]\n",
		       __func__, i, g_clk_track_addr->percpu_addr[i],
		       g_clk_track_addr->percpu_length[i], fieldcnt);

		ret = hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)
					   g_clk_track_addr->percpu_addr[i],
					   g_clk_track_addr->percpu_length[i], fieldcnt,
					   "clk");
		if (ret) {
			pr_err("[%s], cpu [%d] ringbuffer init failed!\n", __func__, i);
			return ret;
		}
	}
	return 0;
}

int clk_buffer_init(u8 *addr, u32 size, buf_type_en buf_type)
{
	unsigned int record_ratio[8][8] = {
	{16, 0, 0, 0, 0, 0, 0, 0},
	{8, 8, 0, 0, 0, 0, 0, 0},
	{6, 5, 5, 0, 0, 0, 0, 0},
	{4, 4, 4, 4, 0, 0, 0, 0},
	{4, 4, 4, 3, 1, 0, 0, 0},
	{4, 4, 3, 3, 1, 1, 0, 0},
	{4, 3, 3, 3, 1, 1, 1, 0},
	{3, 3, 3, 3, 1, 1, 1, 1}
	};

	return clk_percpu_buffer_init(addr, size, sizeof(clk_record_info), CLK_MAGIC_NUM, record_ratio, buf_type);
}



static int __init track_clk_record_init(void)
{
	int ret = -1;
	struct rdr_register_module_result clk_rdr_info;
	unsigned char *vir_addr = NULL;
	struct device_node *np = NULL;
	/* alloc rdr memory and init */
	ret = track_clk_rdr_register(&clk_rdr_info);
	if (ret) {
		return ret;
	}
	if (0 == clk_rdr_info.log_len) {
		pr_err("%s clk_rdr_len is 0x0!\n", __func__);
		return 0;
	}
	vir_addr = (unsigned char *)hisi_bbox_map((phys_addr_t)clk_rdr_info.log_addr, clk_rdr_info.log_len);
	pr_info("%s log_addr is 0x%llx, log_len is 0x%x!\n", __func__, clk_rdr_info.log_addr, clk_rdr_info.log_len);
	if (vir_addr == NULL) {
		pr_err("%s vir_addr err!\n", __func__);
		return -EINVAL;
	}
	memset_s(vir_addr, clk_rdr_info.log_len, 0, clk_rdr_info.log_len);/*clean mem 0*/

	ret = clk_buffer_init(vir_addr, clk_rdr_info.log_len, clk_sel_buf_type);

	if (ret) {
		pr_err("%s buffer init err!\n", __func__);
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,clk-pmctrl");
	if (NULL == np) {
		pr_err("[%s] fail to find pmctrl node!\n", __func__);
		return -EINVAL;
	}
	hs_trace_info.pmuctrl = of_iomap(np, 0);
	if (!hs_trace_info.pmuctrl) {
		pr_err("[%s]failed to iomap!\n", __func__);
		return -EINVAL;
	}

	hs_trace_info.clk_cpu_l = __clk_lookup("cpu-cluster.0");
	if (IS_ERR(hs_trace_info.clk_cpu_l)) {
		pr_err("_clk_get: clk_cpu_l not found!\n");
		return -EINVAL;
	}

	hs_trace_info.clk_cpu_b = __clk_lookup("cpu-cluster.1");
	if (IS_ERR(hs_trace_info.clk_cpu_b)) {
		pr_err("_clk_get: clk_cpu_b not found!\n");
		return -EINVAL;
	}

	hs_trace_info.clk_ddr = __clk_lookup("clk_ddrc_freq");
	if (IS_ERR(hs_trace_info.clk_ddr)) {
		pr_err("_clk_get: clk_ddr not found!\n");
		return -EINVAL;
	}

	if (check_himntn(HIMNTN_TRACE_CLK_REGULATOR))
		g_clk_hook_on = SWITCH_OPEN;

	pr_err("%s: hook_on = %d,rdr_phy_addr = 0x%llx, rdr_len = 0x%x, rdr_virt_add = 0x%pK\n", __func__, g_clk_hook_on,
			clk_rdr_info.log_addr, clk_rdr_info.log_len, vir_addr);
	return 0;
}
module_init(track_clk_record_init);

static void __exit track_clk_record_exit(void)
{
	return;
}
module_exit(track_clk_record_exit);
MODULE_LICENSE("GPL");

#endif


