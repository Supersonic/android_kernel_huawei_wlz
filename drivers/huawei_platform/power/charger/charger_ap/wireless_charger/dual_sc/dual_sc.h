/*
 * dual_sc.h
 *
 * dual sc driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _DUAL_SC_H_
#define _DUAL_SC_H_

#include <huawei_platform/power/wireless_charger.h>

struct dual_sc_info {
	struct platform_device *pdev;
	struct device *dev;
};

#ifdef CONFIG_WIRELESS_DUAL_SC
int dual_sc_main_wlchip_ops_register(struct wireless_cp_ops *ops);
int dual_sc_aux_wlchip_ops_register(struct wireless_cp_ops *ops);
#else
static inline int dual_sc_main_wlchip_ops_register(struct wireless_cp_ops *ops)
{
	return -1;
}

static inline int dual_sc_aux_wlchip_ops_register(struct wireless_cp_ops *ops)
{
	return -1;
}
#endif /* CONFIG_WIRELESS_DUAL_SC */

#endif /* _DUAL_SC_H_ */
