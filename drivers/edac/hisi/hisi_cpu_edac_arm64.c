/*
 * drivers/edac/hisi/hisi_cpu_edac_arm64.c - HISI CPU Cache EDAC driver
 *
 * Copyright (C) 2019 xueqiang
 *
 * This file is released under the GPLv2.
 */
#include <linux/kernel.h>
#include <linux/edac.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <asm/cputype.h>
#include "edac_mc.h"
#include "edac_device.h"
#include "hisi_cpu_edac_arm64.h"

#define EDAC_CPU	"hisi_edac"

static const struct errors_edac g_errors_handlers[] = {
	{"L1 Correctable Error", edac_device_handle_ce, hisi_edac_mntn_hanlde_ce },
	{"L1 Uncorrectable Error", edac_device_handle_ue, hisi_edac_mntn_hanlde_ue },
	{"L2 Correctable Error", edac_device_handle_ce, hisi_edac_mntn_hanlde_ce },
	{"L2 Uncorrectable Error", edac_device_handle_ue, hisi_edac_mntn_hanlde_ue },
	{"L3 Correctable Error", edac_device_handle_ce, hisi_edac_mntn_hanlde_ce },
	{"L3 Uncorrectable Error", edac_device_handle_ue, hisi_edac_mntn_hanlde_ue },
};

static DEFINE_SPINLOCK(g_handler_lock);

static inline void set_errxctlr_el1(void)
{
	u64 val = 0x10f;

	asm volatile("msr s3_0_c5_c4_1, %0" : : "r" (val));
}

static inline void set_errxmisc_overflow(void)
{
	u64 val = 0x7F7F00000000ULL;

	asm volatile("msr s3_0_c5_c5_0, %0" : : "r" (val));
}

static inline void write_errselr_el1(u64 val)
{
	asm volatile("msr s3_0_c5_c3_1, %0" : : "r" (val));
}

static inline u64 read_errxstatus_el1(void)
{
	u64 val;

	asm volatile("mrs %0, s3_0_c5_c4_2" : "=r" (val));
	return val;
}

static inline u64 read_errxmisc_el1(void)
{
	u64 val;

	asm volatile("mrs %0, s3_0_c5_c5_0" : "=r" (val));
	return val;
}

static inline void clear_errxstatus_valid(u64 val)
{
	asm volatile("msr s3_0_c5_c4_2, %0" : : "r" (val));
}

/* test for debug */
#ifdef CONFIG_HISI_CPU_EDAC_ARM64_DEBUG
u64 get_errxctlr_el1(void)
{
	u64 val = 0;

	asm volatile("mrs %0,  s3_0_c5_c4_1" : "=r" (val));
	pr_err("[%s] val = 0x%x\n", __func__, val);

	return val;
}
EXPORT_SYMBOL(get_errxctlr_el1);

u64 get_errselr_el1(void)
{
	u64 val = 0;

	asm volatile("mrs %0,  s3_0_c5_c3_1" : "=r" (val));
	pr_err("[%s] val = 0x%x\n", __func__, val);

	return val;
}
EXPORT_SYMBOL(get_errselr_el1);

/*
 * level
 * 0, Select record 0 containing errors from Level 1 and Level 2 RAMs located on the Hera core.
 * 1, Select record 1 containing errors from Level 3 RAMs located on the DSU.
 */
int test_ecc_error_input(int level, int ue)
{
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&g_handler_lock, flags);
	write_errselr_el1(!!level);
	/* ERXPFGCDNR_EL1 count down value */
	val = 0x50;
	asm volatile("msr  s3_0_c15_c2_2 , %0" : : "r" (val));
	val = 0;
	asm volatile("mrs %0, s3_0_c15_c2_2" : "=r"(val));
	pr_err("%s: ERXPFGCDNR_EL1 = 0x%x\n", __func__, val);

	if (ue == 0) {
		/*
		 * ERXPFGCTLR_EL1
		 * 0xC0000062, corrected error/deferred error/uncontainable error
		 */
		val = 0xc0000040;
		asm volatile("msr  s3_0_c15_c2_1 , %0" : : "r" (val));
		val = 0;
		asm volatile("mrs %0, s3_0_c15_c2_1" : "=r"(val));
		pr_err("%s: 1bit error ERXPFGCTLR_EL1 = 0x%x\n", __func__, val);
	} else {
		/*
		 * ERXPFGCTLR_EL1
		 * 0xC0000062, corrected error/deferred error/uncontainable error
		 */
		val = 0xC0000062;
		asm volatile("msr  s3_0_c15_c2_1 , %0" : : "r" (val));
		val = 0;
		asm volatile("mrs %0, s3_0_c15_c2_1" : "=r"(val));
		pr_err("%s: 2bit error ERXPFGCTLR_EL1 = 0x%x\n", __func__, val);
	}

	spin_unlock_irqrestore(&g_handler_lock, flags);
	return 0;
}
EXPORT_SYMBOL(test_ecc_error_input);
#endif

static void print_l3_serr(u64 errxstatus)
{
	switch (ERRXSTATUS_SERR(errxstatus)) {
	case NO_ERROR:
		edac_printk(KERN_CRIT, EDAC_CPU, "No error\n");
		break;

	case INTERNAL_DATA_BUFFER:
		edac_printk(KERN_CRIT, EDAC_CPU, "ECC Error from internal data buffer\n");
		break;

	case CACHE_DATA_RAM:
		edac_printk(KERN_CRIT, EDAC_CPU, "ECC Error from cache data RAM\n");
		break;

	case CACHE_TAG_DIRTY_RAM:
		edac_printk(KERN_CRIT, EDAC_CPU, "ECC Error from cache tag or dirty RAM\n");
		break;

	case BUS_ERROR:
		edac_printk(KERN_CRIT, EDAC_CPU, "Bus Error\n");
		break;

	default:
		edac_printk(KERN_CRIT, EDAC_CPU, "Invalid value, l3 errxstatus = 0x%x\n", errxstatus);
		break;
	}
}

static void print_l1_l2_serr(u64 errxstatus)
{
	switch (ERRXSTATUS_SERR(errxstatus)) {
	case NO_ERROR:
		edac_printk(KERN_CRIT, EDAC_CPU, "No error\n");
		break;

	case FAULT_INJECTION:
		edac_printk(KERN_CRIT, EDAC_CPU, "Errors due to fault injection\n");
		break;

	case INTERNAL_DATA_BUFFER:
		edac_printk(KERN_CRIT, EDAC_CPU, "ECC Error from internal data buffer\n");
		break;

	case CACHE_DATA_RAM:
		edac_printk(KERN_CRIT, EDAC_CPU, "ECC Error from cache data RAM\n");
		break;

	case CACHE_TAG_DIRTY_RAM:
		edac_printk(KERN_CRIT, EDAC_CPU, "ECC Error from cache tag or dirty RAM\n");
		break;

	case TLB_DATA_RAM:
		edac_printk(KERN_CRIT, EDAC_CPU, "Parity error on TLB data RAM\n");
		break;

	case CACHE_COPYBACK:
		edac_printk(KERN_CRIT, EDAC_CPU, "Error response for a cache copyback\n");
		break;

	case DEFERRED_ERROR:
		edac_printk(KERN_CRIT, EDAC_CPU, "Deferred error from slave\n");
		break;

	default:
		edac_printk(KERN_CRIT, EDAC_CPU, "Invalid value, l1/l2 errxstatus = 0x%x\n", errxstatus);
		break;
	}
}

static void dump_err_reg(int errorcode, int level, u64 errxstatus, u64 errxmisc,
			 struct edac_device_ctl_info *edev_ctl)
{
	edac_printk(KERN_CRIT, EDAC_CPU, "ERRXSTATUS_EL1: %llx\n", errxstatus);
	edac_printk(KERN_CRIT, EDAC_CPU, "ERRXMISC_EL1: %llx\n", errxmisc);
	edac_printk(KERN_CRIT, EDAC_CPU, "Cache level: L%d\n", level + 1);

	if (level == L3) {
		print_l3_serr(errxstatus);
		edac_printk(KERN_CRIT, EDAC_CPU, "Way: %u\n", ERRXMISC_WAY(errxmisc));
	} else {
		print_l1_l2_serr(errxstatus);
		edac_printk(KERN_CRIT, EDAC_CPU, "Way: %u\n", ERRXMISC_WAY(errxmisc) >> 2);
	}

	if (errorcode < ARRAY_SIZE(g_errors_handlers)) {
		g_errors_handlers[errorcode].func(edev_ctl, smp_processor_id(),
						  level, g_errors_handlers[errorcode].msg);

		g_errors_handlers[errorcode].mntn_handler(edev_ctl, smp_processor_id(),
							  level, errxstatus, errxmisc);
	} else {
		edac_printk(KERN_CRIT, EDAC_CPU, "invalid errorcode = %d\n", errorcode);
	}
}

static void parse_l1_l2_cache_error(u64 errxstatus, u64 errxmisc,
				    struct edac_device_ctl_info *edev_ctl, int cpu)
{
	int level = -1; /* invalid value */
	u32 part_num;

	part_num = read_cpuid_part_number();
	edac_printk(KERN_DEBUG, EDAC_CPU, "part_num = 0x%x\n", part_num);
	switch (part_num) {
	case HISI_CPU_PARTNUM_ANANKE:
		switch (ERRXMISC_LVL_ANANKE(errxmisc)) {
		case L1_BIT_ANANKE:
			level = L1;
			break;
		case L2_BIT_ANANKE:
			level = L2;
			break;
		default:
			edac_printk(KERN_CRIT, EDAC_CPU,
				    "cpu:%d unknown error location:%u\n",
				    cpu, ERRXMISC_LVL_ANANKE(errxmisc));
		}
		break;
	case HISI_CPU_PARTNUM_HERCULES:
		switch (ERRXMISC_LVL_HERCULES(errxmisc)) {
		case L1_IC_BIT:
		case L1_DC_BIT:
			level = L1;
			break;
		case L2_BIT:
		case L2_TLB_BIT:
			level = L2;
			break;
		default:
			edac_printk(KERN_CRIT, EDAC_CPU,
				    "cpu:%d unknown error location:%u\n",
				    cpu, ERRXMISC_LVL_HERCULES(errxmisc));
		}
		break;
	case HISI_CPU_PARTNUM_HERA:
		switch (ERRXMISC_LVL_HERA(errxmisc)) {
		case L1_IC_BIT:
		case L1_DC_BIT:
			level = L1;
			break;
		case L2_BIT:
		case L2_TLB_BIT:
			level = L2;
			break;
		default:
			edac_printk(KERN_CRIT, EDAC_CPU,
				    "cpu:%d unknown error location:%u\n",
				    cpu, ERRXMISC_LVL_HERA(errxmisc));
		}
		break;
	case HISI_CPU_PARTNUM_DEIMOS:
		switch (ERRXMISC_LVL_DEIMOS(errxmisc)) {
		case L1_IC_BIT:
		case L1_DC_BIT:
			level = L1;
			break;
		case L2_BIT:
		case L2_TLB_BIT:
			level = L2;
			break;
		default:
			edac_printk(KERN_CRIT, EDAC_CPU,
				    "cpu:%d unknown error location:%u\n",
				    cpu, ERRXMISC_LVL_DEIMOS(errxmisc));
		}
		break;
	default:
		edac_printk(KERN_CRIT, EDAC_CPU,
			    "Error in matching cpu%d with part num:%u\n",
			    cpu, part_num);
		return;
	}

	switch (level) {
	case L1:
		if (ERRXSTATUS_UE(errxstatus))
			dump_err_reg(L1_UE, level, errxstatus, errxmisc,
				     edev_ctl);
		else
			dump_err_reg(L1_CE, level, errxstatus, errxmisc,
				     edev_ctl);
		break;
	case L2:
		if (ERRXSTATUS_UE(errxstatus))
			dump_err_reg(L2_UE, level, errxstatus, errxmisc,
				     edev_ctl);
		else
			dump_err_reg(L2_CE, level, errxstatus, errxmisc,
				     edev_ctl);
		break;
	default:
		edac_printk(KERN_CRIT, EDAC_CPU, "Unknown ERRXMISC_L1_L2_LVL value\n");
		break;
	}
}

static void check_l1_l2_ecc(void *info)
{
	struct edac_device_ctl_info *edev_ctl = info;
	u64 errxstatus;
	u64 errxmisc;
	int cpu;
	unsigned long flags;

	spin_lock_irqsave(&g_handler_lock, flags);
	write_errselr_el1(0);
	errxstatus = read_errxstatus_el1();
	cpu = smp_processor_id();

	if (ERRXSTATUS_VALID(errxstatus)) {
		errxmisc = read_errxmisc_el1();
		edac_printk(KERN_CRIT, EDAC_CPU,
			    "Hisi CPU%d detected a L1/L2 cache error, errxstatus = %llx, errxmisc = %llx\n",
			    cpu, errxstatus, errxmisc);

		parse_l1_l2_cache_error(errxstatus, errxmisc, edev_ctl, cpu);
		clear_errxstatus_valid(errxstatus);
	}
	spin_unlock_irqrestore(&g_handler_lock, flags);
}

static bool l3_is_bus_error(u64 errxstatus)
{
	if (ERRXSTATUS_SERR(errxstatus) == BUS_ERROR) {
		edac_printk(KERN_CRIT, EDAC_CPU, "Bus Error\n");
		return true;
	}

	return false;
}

static void check_l3_scu_error(struct edac_device_ctl_info *edev_ctl)
{
	u64 errxstatus;
	u64 errxmisc;
	unsigned long flags;

	spin_lock_irqsave(&g_handler_lock, flags);
	write_errselr_el1(1);
	errxstatus = read_errxstatus_el1();
	errxmisc = read_errxmisc_el1();

	if (ERRXSTATUS_VALID(errxstatus) &&
	    ERRXMISC_L3_LVL(errxmisc) == L3_LEVEL) {
		if (l3_is_bus_error(errxstatus)) {
			edac_printk(KERN_CRIT, EDAC_CPU, "Detected L3 bus error\n");
			bbox_diaginfo_record(L3_EDAC_ECC_BUS_ERROR, NULL, "L3 EDAC bus error, errxstatus:0xllx, errxmisc:0x%llx",
					     errxstatus, errxmisc);
			clear_errxstatus_valid(errxstatus);
			spin_unlock_irqrestore(&g_handler_lock, flags);
			return;
		}

		if (ERRXSTATUS_UE(errxstatus)) {
			edac_printk(KERN_CRIT, EDAC_CPU, "Detected L3 uncorrectable error\n");
			dump_err_reg(L3_UE, L3, errxstatus, errxmisc, edev_ctl);
		} else {
			edac_printk(KERN_CRIT, EDAC_CPU, "Detected L3 correctable error\n");
			dump_err_reg(L3_CE, L3, errxstatus, errxmisc, edev_ctl);
		}

		clear_errxstatus_valid(errxstatus);
	}
	spin_unlock_irqrestore(&g_handler_lock, flags);
}

static irqreturn_t l1_l2_edac_handler(int irq, void *drvdata)
{
	struct erp_drvdata *drv = NULL;

	drv = *(void **)drvdata;
	if (IS_ERR_OR_NULL(drv)) {
		return IRQ_NONE;
	}
	check_l1_l2_ecc(drv->edev_ctl);
	return IRQ_HANDLED;
}

static irqreturn_t l3_scu_edac_handler(int irq, void *drvdata)
{
	struct erp_drvdata *drv = drvdata;
	struct edac_device_ctl_info *edev_ctl = drv->edev_ctl;

	check_l3_scu_error(edev_ctl);
	return IRQ_HANDLED;
}

static int parse_l1_l2_irq(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	struct device_node *node = pdev->dev.of_node;
	struct erp_cpu_drvdata *erp_cpu_drv = NULL;
	struct device_node *l1_l2_node = NULL;
	int irq_cnts;
	int cpu, i, irq;
	int ret;

	drv->cpu_drv = alloc_percpu(struct erp_cpu_drvdata);
	if (drv->cpu_drv == NULL) {
		dev_err(dev, "failed to alloc per-cpu drvdata\n");
		ret = -ENOMEM;
		return ret;
	}

	for_each_possible_cpu(cpu) {
		erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, cpu);
		erp_cpu_drv->drvdata = drv;
	}

	of_node_get(node);

	l1_l2_node = of_get_child_by_name(node, "l1-l2-ecc");
	if (l1_l2_node == NULL) {
		dev_err(dev, "failed to find l1_l2_node\n");
		ret = -EINVAL;
		goto err_out;
	}

	irq_cnts = of_irq_count(l1_l2_node);
	dev_err(dev, "irq cnts = %d, nr_cpu_ids = %d\n", irq_cnts, nr_cpu_ids);

	/* compatible with cpunum which is less than num_possible_cpus by lpnv */
	irq_cnts = (irq_cnts > nr_cpu_ids) ? nr_cpu_ids : irq_cnts;
	for (i = 0; i < irq_cnts; i++) {
		erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, i);
		irq = of_irq_get(l1_l2_node, i);
		dev_info(dev, "i = %d, irq = %d\n", i, irq);

		if (irq <= 0) {
			dev_err(dev, "get invalid l1/l2 irq = %d\n", irq);
			WARN_ON(1);
			erp_cpu_drv->irq = 0;
			continue;
		}

		erp_cpu_drv->irq = irq;
		cpumask_set_cpu(i, &drv->supported_cpus);
	}

	of_node_put(node);
	return 0;

err_out:
	of_node_put(node);
	free_percpu(drv->cpu_drv);
	return ret;
}

static void free_l1_l2_irq(struct erp_drvdata *drv, int cpu)
{
	struct erp_cpu_drvdata *erp_cpu_drv = NULL;
	int irq = per_cpu(drv->cpu_drv->irq, cpu);

	erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, cpu);
	if (cpumask_test_and_clear_cpu(cpu, &drv->active_irqs) == 0) {
		return;
	}

	free_irq(irq, erp_cpu_drv->drvdata);
}

static void free_l1_l2_irqs(struct erp_drvdata *drv)
{
	int cpu;

	for_each_cpu(cpu, &drv->supported_cpus) {
		free_l1_l2_irq(drv, cpu);
	}
}

static int request_l1_l2_irq(struct platform_device *pdev, int cpu)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	const irq_handler_t handler = l1_l2_edac_handler;
	struct erp_cpu_drvdata *erp_cpu_drv = NULL;
	int irq;
	int ret;

	erp_cpu_drv = per_cpu_ptr(drv->cpu_drv, cpu);
	irq = per_cpu(drv->cpu_drv->irq, cpu);
	if (irq == 0) {
		dev_err(dev, "invalid irq = %d\n", irq);
		return 0;
	}

	ret = irq_force_affinity(irq, cpumask_of(cpu));
	if (ret != 0 && num_possible_cpus() > 1) {
		dev_err(dev, "unable to set irq affinity (irq=%d, cpu=%d)\n", irq, cpu);
		goto out;
	}

	ret = request_irq(irq, handler,
			  IRQF_NOBALANCING | IRQF_NO_THREAD | IRQF_NO_SUSPEND,
			  "l1-l2-faultirq-ecc",
			  per_cpu_ptr(&drv->cpu_drv->drvdata, cpu));

	if (ret != 0) {
		dev_err(dev, "request l1_l2_irq failed (irq=%d, cpu=%d)\n", irq, cpu);
		goto out;
	}

	cpumask_set_cpu(cpu, &drv->active_irqs);
	return 0;

out:
	return ret;

}

static int request_l1_l2_irqs(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	int cpu;
	int ret = 0;

	for_each_cpu(cpu, &drv->supported_cpus) {
		ret = request_l1_l2_irq(pdev, cpu);
		if (ret != 0) {
			dev_err(dev, "request l1_l2 irq failed, cpu = %d\n", cpu);
			break;
		}
	}

	return ret;
}

static int parse_l3_irq(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	struct device_node *node = pdev->dev.of_node;
	struct device_node *l3_node = NULL;
	int irq;
	int ret;

	of_node_get(node);

	l3_node = of_get_child_by_name(node, "l3-ecc");
	if (l3_node == NULL) {
		dev_err(dev, "failed to find l3_node\n");
		ret = -EINVAL;
		goto err_out;
	}

	irq = of_irq_get(l3_node, 0);
	dev_info(dev, "l3 irq = %d\n", irq);

	if (irq <= 0) {
		dev_err(dev, "get invalid l3 irq = %d\n", irq);
		WARN_ON(1);
		drv->l3_irq = 0;
		ret = -EINVAL;
		goto err_out;
	}
	drv->l3_irq = irq;

	of_node_put(node);
	return 0;

err_out:
	of_node_put(node);
	return ret;
}

static int request_l3_irq(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = platform_get_drvdata(pdev);
	int ret;

	ret = devm_request_threaded_irq(dev,
					drv->l3_irq,
					NULL,
					l3_scu_edac_handler,
					IRQF_TRIGGER_NONE | IRQF_ONESHOT,
					"l3-faultirq-ecc",
					drv);
	if (ret != 0) {
		dev_err(dev, "request l3 faultirq failed\n");
	}

	return ret;
}

void hisi_edac_mntn_hanlde_ce(struct edac_device_ctl_info *edac_dev,
			      int inst_nr, int block_nr, u64 errxstatus, u64 errxmisc)
{
	if (edac_dev == NULL) {
		pr_err("%s edac_dev is NULL\n", __func__);
		return;
	}

	if ((inst_nr >= edac_dev->nr_instances) || (inst_nr < 0)) {
		pr_err("%s: instance out of range %d >= %d\n", __func__,
		       inst_nr, edac_dev->nr_instances);
		return;
	}

#ifdef CONFIG_HISI_CPU_EDAC_ARM64_DEBUG
	if (block_nr == L3) {
		rdr_syserr_process_for_ap((u32)MODID_AP_S_L3_EDAC_ECC_CE, 0ULL, 0ULL);
	} else {
		rdr_syserr_process_for_ap((u32)(MODID_AP_S_CPU0_EDAC_ECC_CE + inst_nr), 0ULL, 0ULL);
	}
#else
	if (block_nr == L3) {
		bbox_diaginfo_record(L3_EDAC_ECC_CE_ERROR, NULL,
				     "L3 CE, errxstatus=0x%llx, errxmisc=0x%llx", errxstatus, errxmisc);
	} else {
		bbox_diaginfo_record((CPU0_EDAC_ECC_CE_ERROR + inst_nr), NULL,
				     "CPU%d CE, errxstatus=0x%llx, errxmisc=0x%llx", inst_nr, errxstatus, errxmisc);
	}
#endif
}

void hisi_edac_mntn_hanlde_ue(struct edac_device_ctl_info *edac_dev,
			      int inst_nr, int block_nr, u64 errxstatus, u64 errxmisc)
{

	if (edac_dev == NULL) {
		pr_err("%s edac_dev is NULL\n", __func__);
		return;
	}

	if ((inst_nr >= edac_dev->nr_instances) || (inst_nr < 0)) {
		pr_err("%s: instance out of range %d >= %d\n", __func__,
		       inst_nr, edac_dev->nr_instances);
		return;
	}

	if (block_nr == L3) {
		rdr_syserr_process_for_ap((u32)MODID_AP_S_L3_EDAC_ECC_UE, 0ULL, 0ULL);
	} else {
		rdr_syserr_process_for_ap((u32)(MODID_AP_S_CPU0_EDAC_ECC_UE + inst_nr), 0ULL, 0ULL);
	}
}

static void initialize_registers(void *info)
{
	set_errxctlr_el1();
	set_errxmisc_overflow();
}

static void init_regs_on_cpu(bool all_cpus)
{
	int cpu;

	/* init L1/L2 */
	write_errselr_el1(0);
	if (all_cpus) {
		for_each_possible_cpu(cpu)
			smp_call_function_single(cpu, initialize_registers,
						 NULL, 1);
	} else {
		initialize_registers(NULL);
	}

	/* init L3 */
	write_errselr_el1(1);
	initialize_registers(NULL);
}

static int edac_ecc_starting_cpu(unsigned int cpu, struct hlist_node *node)
{
	struct erp_drvdata *drv = hlist_entry_safe(node, struct erp_drvdata, node);
	int irq;
	int ret;

	if (drv == NULL || cpumask_test_cpu(cpu, &drv->supported_cpus) == 0) {
		pr_err("EDAC: %s failed. drv is NULL(%d), cpu = %d\n", __func__, (drv == NULL), cpu);
		return 0;
	}

	irq = per_cpu(drv->cpu_drv->irq, cpu);
	if (irq) {
		ret = irq_force_affinity(irq, cpumask_of(cpu));
		if (ret && num_possible_cpus() > 1) {
			pr_err("%s: Unable to set irq affinity (irq=%d, cpu=%u)\n", __func__, irq, cpu);
		} else {
			init_regs_on_cpu(false);
		}
	}

	return 0;
}

static int edac_ecc_stopping_cpu(unsigned int cpu, struct hlist_node *node)
{
	return 0;
}

static int edac_cpu_pm_notify(struct notifier_block *self,
			      unsigned long action, void *v)
{
	struct erp_drvdata *drv = container_of(self, struct erp_drvdata, nb_pm);

	switch (action) {
	case CPU_PM_EXIT:
		init_regs_on_cpu(false);
		check_l3_scu_error(drv->edev_ctl);
		check_l1_l2_ecc(drv->edev_ctl);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int cpu_edac_register_exception(void)
{
	unsigned int ret;
	unsigned int i, j;

	for (i = 0; i < ARRAY_SIZE(cpu_edac_einfo); i++) {
		ret = rdr_register_exception(&cpu_edac_einfo[i]);
		/* if error, return 0; otherwise return e_modid_end */
		if (ret != cpu_edac_einfo[i].e_modid_end) {
			pr_err("register cpu edac exception failed in cpu_edac_einfo[%u]\n", i);
			goto err;
		}
	}

	return 0;
err:
	for (j = 0; j < i; j++) {
		(void)rdr_unregister_exception(&cpu_edac_einfo[j]);
	}
	return -1;
}

static void cpu_edac_unregister_exception(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cpu_edac_einfo); i++) {
		(void)rdr_unregister_exception(&cpu_edac_einfo[i]);
	}
}

static int hisi_cpu_erp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct erp_drvdata *drv = NULL;
	int rc;

	dev_err(dev, "entry\n");

	init_regs_on_cpu(true);

	drv = devm_kzalloc(dev, sizeof(*drv), GFP_KERNEL);

	if (drv == NULL) {
		dev_err(dev, "fail to alloc mem for drvdata\n");
		return -ENOMEM;
	}

	drv->edev_ctl = edac_device_alloc_ctl_info(0, "cpu",
						   num_possible_cpus(), "L", 3, 1, NULL, 0,
						   edac_device_alloc_index());

	if (drv->edev_ctl == NULL) {
		dev_err(dev, "fail to alloc ctl info\n");
		return -ENOMEM;
	}

	drv->edev_ctl->dev = dev;
	drv->edev_ctl->mod_name = dev_name(dev);
	drv->edev_ctl->dev_name = dev_name(dev);
	drv->edev_ctl->ctl_name = "cache";
	drv->edev_ctl->panic_on_ue = ARM64_ERP_PANIC_ON_UE;
	drv->nb_pm.notifier_call = edac_cpu_pm_notify;
	platform_set_drvdata(pdev, drv);

	rc = edac_device_add_device(drv->edev_ctl);
	if (rc != 0) {
		dev_err(dev, "add edac device failed\n");
		goto out_mem;
	}

	rc  = cpuhp_setup_state_multi(CPUHP_AP_EDAC_ECC_STARTING,
				      "edac/ecc:starting",
				      edac_ecc_starting_cpu,
				      edac_ecc_stopping_cpu);
	if (rc != 0) {
		dev_err(dev, "CPU hotplug notifier for EDAC ECC registered failed: %d\n", rc);
		goto out_dev;
	}

	rc = parse_l1_l2_irq(pdev);
	if (rc != 0)  {
		dev_err(dev, "parse l1_l2 irq failed\n");
		goto out_cpuhp;
	}

	rc = request_l1_l2_irqs(pdev);
	if (rc != 0) {
		dev_err(dev, "request l1_l2 irq failed\n");
		goto out_l1_l2;
	}

	rc = parse_l3_irq(pdev);
	if (rc != 0)  {
		dev_err(dev, "parse l3 irq failed\n");
		goto out_l1_l2;
	}

	rc = request_l3_irq(pdev);
	if (rc != 0) {
		dev_err(dev, "request l1_l2 irq failed\n");
		goto out_l1_l2;
	}

	rc = cpu_pm_register_notifier(&(drv->nb_pm));
	if (rc != 0) {
		dev_err(dev, "cpu_pm regisger failed\n");
		goto out_l1_l2;
	}

	rc = cpuhp_state_add_instance(CPUHP_AP_EDAC_ECC_STARTING,
				      &drv->node);
	if (rc != 0) {
		dev_err(dev, "cuphp add instance failed\n");
		goto out_cpu_pm;
	}

	rc = cpu_edac_register_exception();
	if (rc != 0) {
		dev_err(dev, "register exception failed\n");
		goto out_cpuhp_inst;
	}

	dev_err(dev, "success\n");
	return 0;

out_cpuhp_inst:
	cpuhp_state_remove_instance(CPUHP_AP_EDAC_ECC_STARTING,
				    &drv->node);
out_cpu_pm:
	cpu_pm_unregister_notifier(&(drv->nb_pm));
out_l1_l2:
	free_l1_l2_irqs(drv);
	free_percpu(drv->cpu_drv);
out_cpuhp:
	cpuhp_remove_multi_state(CPUHP_AP_EDAC_ECC_STARTING);
out_dev:
	edac_device_del_device(dev);
out_mem:
	edac_device_free_ctl_info(drv->edev_ctl);
	return rc;
}

static int hisi_cpu_erp_remove(struct platform_device *pdev)
{
	struct erp_drvdata *drv = dev_get_drvdata(&pdev->dev);
	struct edac_device_ctl_info *edac_ctl = drv->edev_ctl;

	cpu_edac_unregister_exception();
	cpuhp_state_remove_instance(CPUHP_AP_EDAC_ECC_STARTING,
				    &drv->node);
	cpu_pm_unregister_notifier(&(drv->nb_pm));

	free_l1_l2_irqs(drv);
	free_percpu(drv->cpu_drv);
	cpuhp_remove_multi_state(CPUHP_AP_EDAC_ECC_STARTING);

	edac_device_del_device(edac_ctl->dev);
	edac_device_free_ctl_info(edac_ctl);

	return 0;
}

static const struct of_device_id hisi_cpu_erp_match_table[] = {
	{ .compatible = "hisi,cpu-edac-ecc" },
	{ }
};

static struct platform_driver hisi_cpu_erp_driver = {
	.probe = hisi_cpu_erp_probe,
	.remove = hisi_cpu_erp_remove,
	.driver = {
		.name = "hisi_cpu_cache_erp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_cpu_erp_match_table),
	},
};

static int __init hisi_cpu_erp_init(void)
{
	return platform_driver_register(&hisi_cpu_erp_driver);
}
module_init(hisi_cpu_erp_init);

static void __exit hisi_cpu_erp_exit(void)
{
	platform_driver_unregister(&hisi_cpu_erp_driver);
}
module_exit(hisi_cpu_erp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisi EDAC driver");
