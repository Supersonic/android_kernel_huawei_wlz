/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: ddr inspect dispatch mmap head file
 * Author: zhouyubin
 * Create: 2019-05-30
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/fs.h>
#include <linux/mm.h>

#ifndef MMAP_PHYS__H
#define MMAP_PHYS__H

int file_mmap_configured(struct file *filp, struct vm_area_struct *vma);

#endif
