
#define pr_fmt(fmt) "l3cache-ecc: " fmt
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <asm/irq.h>
#include <linux/hisi/util.h>
#include <linux/syscalls.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/hisi/hisi_log.h>
#include "mntn_l3cache_ecc.h"

#define HISI_LOG_TAG                            MNTN_L3CACHE_ECC_TAG

/*lint -e578 -e715 -e838*/
noinline u64 atfd_l3cache_ecc_smc(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;

	asm volatile(
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc    #0\n"
	: "+r" (function_id)
	: "r" (arg0), "r" (arg1), "r" (arg2));

	return (u64)function_id;
}
/*lint +e578 +e715 +e838*/

enum serr_type l3cache_ecc_get_status(u64 *err1_status, u64 *err1_misc0)
{
	u64 ret;
	enum serr_type serr;

	if (err1_status == NULL || err1_misc0 == NULL) {
		pr_err("invalid input\n");
		return NO_EEROR;
	}

	ret = atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
				   (u64)L3CACHE_ECC_READ, (u64)ERR1STATUS, (u64)NULL);

	pr_debug("ERR1STATUS: 0x%llx\n", ret);
	switch (ret & 0xff) {
	case 0x0:
		pr_err("[primary error code] 0x0: No error\n");
		serr = NO_EEROR;
		break;
	case 0x2:
		pr_err("[primary error code] 0x2: ECC error from internal data buffer\n");
		serr = INTERNAL_DATA_BUFFER;
		break;
	case 0x6:
		pr_err("[primary error code] 0x6: ECC error on cache data RAM\n");
		serr = CACHE_DATA_RAM;
		break;
	case 0x7:
		pr_err("[primary error code] 0x7: ECC error on cache tag or dirty RAM\n");
		serr = CACHE_TAG_DIRTY_RAM;
		break;
	case 0x12:
		pr_err("[primary error code] 0x12: Bus error\n");
		serr = BUS_ERROR;
		break;
	default:
		pr_err("[primary error code] : invalid value\n");
		serr = INVALID_VAL;
		break;
	}

	*err1_status = ret;
	ret = atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
				   (u64)L3CACHE_ECC_READ, (u64)ERR1MISC0, (u64)NULL);
	pr_debug("ERR1MISC0: 0x%llx\n", ret);
	*err1_misc0 = ret;

	return serr;
}


static void l3cache_ecc_open(void)
{
	atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
			     (u64)L3CACHE_ECC_OPEN, (u64)NULL, (u64)NULL);
}

static void l3cache_ecc_close(void)
{
	atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
			     (u64)L3CACHE_ECC_CLOSE, (u64)NULL, (u64)NULL);
}

static void l3cache_ecc_int_clear(void)
{
	atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
			     (u64)L3CACHE_ECC_INT_CLEAR, (u64)NULL, (u64)NULL);
}

static void l3cache_ecc_diaginfo_record(enum serr_type serr, u32 err_type,
					const char *irq_name, u64 err1_status, u64 err1_misc0)
{
	if (err_type == 1U) {
		/* 1-bit error */
		if (serr == BUS_ERROR) {
			bbox_diaginfo_record(L3_ECC_BUS_ERROR, NULL, "l3 ecc %u-bit bus error of %s, err1status:0x%llx, err1misc0:0x%llx",
					     err_type, irq_name, err1_status, err1_misc0);
		} else {
			bbox_diaginfo_record(L3_ECC_1BIT_ERROR, NULL, "l3 ecc %u-bit error of %s, err1status:0x%llx, err1misc0:0x%llx",
					     err_type, irq_name, err1_status, err1_misc0);
		}
	} else if (err_type == 2U) {
		/* 2-bit error */
		if (serr == BUS_ERROR) {
			bbox_diaginfo_record(L3_ECC_BUS_ERROR, NULL, "l3 ecc %u-bit bus error of %s, err1status:0x%llx, err1misc0:0x%llx",
					     err_type, irq_name, err1_status, err1_misc0);
		} else {
			bbox_diaginfo_record(L3_ECC_2BIT_ERROR, NULL, "l3 ecc %u-bit error of %s, err1status:0x%llx, err1misc0:0x%llx",
					     err_type, irq_name, err1_status, err1_misc0);
		}
	}
}

static void hisi_l3cache_ecc_record(u32 modid, u32 err_type, const char *irq_name)
{
	u64 err1_status, err1_misc0;
	enum serr_type serr;

	serr = l3cache_ecc_get_status(&err1_status, &err1_misc0);
	/* record bit error */
	l3cache_ecc_diaginfo_record(serr, err_type, irq_name, err1_status, err1_misc0);
} //lint !e715

static irqreturn_t hisi_l3cache_ecc_handler(int irq, void *data)
{
	struct hisi_l3cache_ecc_info *info = (struct hisi_l3cache_ecc_info *)data;
	u32 ret;

	if (info->irq_fault == irq) {
		ret = (u32)atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
						(u64)L3CACHE_ECC_READ, (u64)ERR1STATUS, (u64)NULL);
		if (ret & ERR1STATUS_V_BIT_MASK) {
			/* 1-bit err of faultirq */
			if (!(ret & ERR1STATUS_UE_BIT_MASK)) {
				hisi_l3cache_ecc_record((u32)MODID_AP_S_L3CACHE_ECC1, 1U, "faultirq");
			} else {
				/* 2-bit err of faultirq */
				hisi_l3cache_ecc_record((u32)MODID_AP_S_L3CACHE_ECC2, 2U, "faultirq");
			}
		} else {
			pr_info("[%s:%d] ERR1STATUS is not valid in faultirq\n", __func__, __LINE__);
		}
		/* clear ecc interrupt*/
		l3cache_ecc_int_clear();
	} else if (info->irq_err == irq) {
		ret = (u32)atfd_l3cache_ecc_smc((u64)(HISI_L3CACHE_FN_MAIN_ID),
						(u64)L3CACHE_ECC_READ, (u64)ERR1STATUS, (u64)NULL);
		if (ret & ERR1STATUS_V_BIT_MASK) {
			hisi_l3cache_ecc_record((u32)MODID_AP_S_L3CACHE_ECC2, 2U, "errirq");
		} else {
			pr_info("[%s:%d] ERR1STATUS is not valid in errirq\n", __func__, __LINE__);
		}
		/* clear ecc interrupt*/
		l3cache_ecc_int_clear();
	} else {
		pr_err("[%s:%d]invalid irq %d!\n", __func__, __LINE__, irq);
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}
/*lint -e429*/
static int hisi_l3cache_ecc_probe(struct platform_device *pdev)
{
	struct hisi_l3cache_ecc_info *info = NULL;
	struct device *dev = &pdev->dev;
	int ret;

	if (check_himntn(HIMNTN_L3CACHE_ECC) == 0) {
		pr_err("[%s] HIMNTN_L3CACHE_ECC is closed!\n", __func__);
		ret = ECANCELED;
		goto err;
	}

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		pr_err("[%s]devm_kzalloc error!\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	info->irq_fault = platform_get_irq_byname(pdev, "faultirq");
	if (info->irq_fault < 0) {
		pr_err("[%s]failed to get irq_fault id\n", __func__);
		ret = -ENOENT;
		goto err;
	}

	ret = devm_request_threaded_irq(dev,
				info->irq_fault,
				NULL,
				hisi_l3cache_ecc_handler,
				IRQF_TRIGGER_NONE | IRQF_ONESHOT,
				"faultirq",
				info);
	if (ret < 0) {
		pr_err("[%s]failed to request faultirq\n", __func__);
		ret = -EIO;
		goto err;
	}

	info->irq_err = platform_get_irq_byname(pdev, "errirq");
	if (info->irq_err < 0) {
		pr_err("[%s]failed to get irq_err id\n", __func__);
		ret = -ENOENT;
		goto err_devm_request_irq;
	}
	ret = devm_request_threaded_irq(dev,
				info->irq_err,
				NULL,
				hisi_l3cache_ecc_handler,
				IRQF_TRIGGER_NONE | IRQF_ONESHOT,
				"errirq",
				info);
	if (ret < 0) {
		pr_err("[%s]failed to request errirq\n", __func__);
		ret = -EIO;
		goto err_devm_request_irq;
	}

	platform_set_drvdata(pdev, info);

	l3cache_ecc_open();

	pr_err("[%s]end .\n", __func__);
	return ret;

err:
	return ret;

err_devm_request_irq:
	devm_free_irq(dev, info->irq_fault, info);
	return ret;
}
/*lint +e429*/
static int hisi_l3cache_ecc_remove(struct platform_device *pdev)
{
	struct hisi_l3cache_ecc_info *info = platform_get_drvdata(pdev);

	if (info == NULL)
		return ECC_ERR;

	return ECC_OK;
}


#ifdef CONFIG_PM
/*lint -e715*/
static int hisi_l3cache_ecc_suspend(struct platform_device *pdev,
				 pm_message_t state)
{
	struct hisi_l3cache_ecc_info *info = platform_get_drvdata(pdev);

	if (info == NULL) {
		pr_err("drvdata is NULL");
		return ECC_ERR;
	}

	if (check_himntn(HIMNTN_L3CACHE_ECC))
		l3cache_ecc_close();

	pr_err("[%s]suspend successfully\n", __func__);
	return ECC_OK;
}
/*lint +e715*/
static int hisi_l3cache_ecc_resume(struct platform_device *pdev)
{
	struct hisi_l3cache_ecc_info *info = platform_get_drvdata(pdev);

	if (info == NULL) {
		pr_err("drvdata is NULL");
		return ECC_ERR;
	}

	if (check_himntn(HIMNTN_L3CACHE_ECC))
		l3cache_ecc_open();

	pr_err("[%s]resume successfully\n", __func__);
	return ECC_OK;
}
#endif

static const struct of_device_id hisi_l3cache_ecc_of_match[] = {
	{
		.compatible = "hisilicon,hisi-l3cache-ecc",
	},
	{/* end */},
};

MODULE_DEVICE_TABLE(of, hisi_l3cache_ecc_of_match);

static struct platform_driver hisi_l3cache_ecc_driver = {
	.probe = hisi_l3cache_ecc_probe,
	.remove = hisi_l3cache_ecc_remove,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "hisi-l3cache-ecc",
		   .of_match_table = hisi_l3cache_ecc_of_match,
		   },
#ifdef CONFIG_PM
	.suspend = hisi_l3cache_ecc_suspend,
	.resume  = hisi_l3cache_ecc_resume,
#endif
};

static int __init l3cache_ecc_mntn_init(void)
{
	return platform_driver_register(&hisi_l3cache_ecc_driver);
}

static void __exit l3cache_ecc_mntn_exit(void)
{
	platform_driver_unregister(&hisi_l3cache_ecc_driver);
}

late_initcall(l3cache_ecc_mntn_init);


MODULE_DESCRIPTION("Hisi L3 Cache ECC driver");
MODULE_LICENSE("GPL");

