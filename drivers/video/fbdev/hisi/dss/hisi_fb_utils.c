/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
/*lint -e559*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "hisi_fb.h"
#include "hisi_overlay_utils.h"
#include "hisi_display_effect.h"
#include "hisi_dpe_utils.h"

#if defined (CONFIG_HISI_PERIDVFS)
#include "peri_volt_poll.h"
#endif


#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_core.h"
#endif

#define MAX_BUF 60

void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	tmp = inp32(addr);
	tmp &= ~(mask << bs);

	outp32(addr, tmp | ((val & mask) << bs));

	if (g_debug_set_reg_val) {
		HISI_FB_INFO("writel: [%pK] = 0x%x\n", addr, tmp | ((val & mask) << bs));
	}
}

uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	tmp = old_val;
	tmp &= ~(mask << bs);

	return (tmp | ((val & mask) << bs));
}

void hisifb_set_reg(struct hisi_fb_data_type *hisifd,
	char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	set_reg(addr, val, bw, bs);
}

bool is_fastboot_display_enable(void)
{
	return ((g_fastboot_enable_flag == 1) ? true : false);
}

bool is_dss_idle_enable(void)
{
	return ((g_enable_dss_idle == 1) ? true : false);
}

uint32_t get_panel_xres(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return 0;
	}

	return ((hisifd->resolution_rect.w > 0) ? hisifd->resolution_rect.w : hisifd->panel_info.xres);
}

uint32_t get_panel_yres(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return 0;
	}

	return ((hisifd->resolution_rect.h > 0) ? hisifd->resolution_rect.h : hisifd->panel_info.yres);
}

uint32_t hisifb_line_length(int index, uint32_t xres, int bpp)
{
	return ALIGN_UP(xres * (uint32_t)bpp, DMA_STRIDE_ALIGN);
}

void hisifb_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	ktime_get_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;

	//struct timeval timestamp;
	//do_gettimeofday(&timestamp);
	//timestamp = ktime_to_timeval(ktime_get());
}

uint32_t hisifb_timestamp_diff(struct timeval *lasttime, struct timeval *curtime)
{
	uint32_t ret;
	ret = (curtime->tv_usec >= lasttime->tv_usec) ?
		curtime->tv_usec - lasttime->tv_usec:
		1000000 - (lasttime->tv_usec - curtime->tv_usec);

	return ret;

	//return (curtime->tv_sec - lasttime->tv_sec) * 1000 +
	//	(curtime->tv_usec - lasttime->tv_usec) /1000;
}

void hisifb_save_file(char *filename, const char *buf, uint32_t buf_len)
{
	ssize_t write_len = 0;
	struct file *fd = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	if (NULL == filename) {
		HISI_FB_ERR("filename is NULL");
		return;
	}
	if (NULL == buf) {
		HISI_FB_ERR("buf is NULL");
		return;
	}

	fd = filp_open(filename, O_CREAT|O_RDWR, 0644);
	if (IS_ERR(fd)) {
		HISI_FB_ERR("filp_open returned:filename %s, error %ld\n",
			filename, PTR_ERR(fd));
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); //lint !e501

	write_len = vfs_write(fd, (char __user*)buf, buf_len, &pos);

	pos = 0;
	set_fs(old_fs);
	filp_close(fd, NULL);
}

extern uint32_t g_fastboot_already_set;
int hisifb_ctrl_fastboot(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (pdata->set_fastboot && !g_fastboot_already_set) {
		ret = pdata->set_fastboot(hisifd->pdev);
	}

	hisifb_vsync_resume(hisifd);

	hisi_overlay_on(hisifd, true);

	if (hisifd->panel_info.esd_enable) {
		hrtimer_restart(&hisifd->esd_ctrl.esd_hrtimer);
	}

	return ret;
}

int hisifb_get_other_fb_votelevel(struct hisi_fb_data_type *hisifd, uint32_t *max_vote_level)
{
	struct hisi_fb_data_type *targetfd1 = NULL;
	struct hisi_fb_data_type *targetfd2 = NULL;
	uint32_t target_dss_voltage_level;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		targetfd1 = hisifd_list[EXTERNAL_PANEL_IDX];
		targetfd2 = hisifd_list[AUXILIARY_PANEL_IDX];
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		targetfd1 = hisifd_list[PRIMARY_PANEL_IDX];
		targetfd2 = hisifd_list[AUXILIARY_PANEL_IDX];
	} else {
		targetfd1 = hisifd_list[PRIMARY_PANEL_IDX];
		targetfd2 = hisifd_list[EXTERNAL_PANEL_IDX];
	}

	if ((targetfd1 == NULL) && (targetfd2 == NULL)) {
		target_dss_voltage_level = PERI_VOLTAGE_LEVEL0;
	} else if (targetfd1 == NULL) {
		target_dss_voltage_level = targetfd2->dss_vote_cmd.dss_voltage_level;
	} else if (targetfd2 == NULL) {
		target_dss_voltage_level = targetfd1->dss_vote_cmd.dss_voltage_level;
	} else {
		target_dss_voltage_level = ((targetfd1->dss_vote_cmd.dss_voltage_level > targetfd2->dss_vote_cmd.dss_voltage_level) ?
			targetfd1->dss_vote_cmd.dss_voltage_level : targetfd2->dss_vote_cmd.dss_voltage_level);
	}

	*max_vote_level = target_dss_voltage_level;

	return 0;
}

int hisifb_ctrl_on(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);

	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (pdata->on != NULL) {
		ret = pdata->on(hisifd->pdev);
		if (ret < 0) {
			HISI_FB_ERR("regulator/clk on fail.\n");
			return ret;
		}
	}

	hisifb_vsync_resume(hisifd);


	hisi_overlay_on(hisifd, false);

	if (hisifd->panel_info.esd_enable) {
		hrtimer_start(&hisifd->esd_ctrl.esd_hrtimer, ktime_set(ESD_CHECK_TIME_PERIOD / 1000,
			(ESD_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);
	}
	return ret;
}

int hisifb_ctrl_off(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (hisifd->panel_info.esd_enable) {
		hrtimer_cancel(&hisifd->esd_ctrl.esd_hrtimer);
	}

	hisifb_vsync_suspend(hisifd);

	hisi_overlay_off(hisifd);


	if (pdata->off != NULL) {
		ret = pdata->off(hisifd->pdev);
	}

	// FIXME:
	if ((hisifd->index == PRIMARY_PANEL_IDX) ||
		(hisifd->index == EXTERNAL_PANEL_IDX)) {
		down(&hisifd->buf_sync_ctrl.layerbuf_sem);
		//ov base display ok, disable irq, now layerbuf unlock
		hisifb_layerbuf_unlock(hisifd, &(hisifd->buf_sync_ctrl.layerbuf_list));
		up(&hisifd->buf_sync_ctrl.layerbuf_sem);
	}

	return ret;
}

int hisifb_ctrl_lp(struct hisi_fb_data_type *hisifd, bool lp_enter)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (lp_enter) {
		hisi_overlay_off_lp(hisifd);

		if (pdata->lp_ctrl != NULL) {
			ret = pdata->lp_ctrl(hisifd->pdev, lp_enter);
		}
	} else {
		if (pdata->lp_ctrl != NULL) {
			ret = pdata->lp_ctrl(hisifd->pdev, lp_enter);
		}

		hisi_overlay_on_lp(hisifd);
	}

	return ret;
}

int hisifb_ctrl_esd(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;
#ifdef MIPI_CLK_UPDATE_LOCK_CTRL
	int wait_count = 0;
#endif

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	down(&hisifd->power_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->esd_handle != NULL) {
		hisifb_vsync_disable_enter_idle(hisifd, true);
		hisifb_activate_vsync(hisifd);

#ifdef MIPI_CLK_UPDATE_LOCK_CTRL
		/* wait when mipi_dsi_bit_clk_update flag is true */
		while ((hisifd->mipi_dsi_bit_clk_update == 1) && (wait_count < MIPI_CLK_UPDT_TIMEOUT)) {
			wait_count++;
			msleep(1);
		}
		if (wait_count >= MIPI_CLK_UPDT_TIMEOUT)
			HISI_FB_ERR("wait mipi_dsi_bit_clk_update timeout!\n");

		HISI_FB_DEBUG("wait mipi_dsi_bit_clk_update %d times\n", wait_count);
#endif
		ret = pdata->esd_handle(hisifd->pdev);
		hisifb_vsync_disable_enter_idle(hisifd, false);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->power_sem);

	return ret;
}

int hisifb_fps_upt_isr_handler(struct hisi_fb_data_type *hisifd)
{
	struct hisi_fb_panel_data *pdata = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_fps_updt_handle != NULL) {
		ret = pdata->lcd_fps_updt_handle(hisifd->pdev);
	}

err_out:
	return ret;
}

/*lint -e644 -e540*/
int dss_get_peri_volt(int *curr_volt)
{
#if defined (CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;

	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_FB_ERR("pvp get failed!\n");
		return -EINVAL;
	}
	*curr_volt = peri_get_volt(pvp);
#endif

	return 0;
}

int dss_set_peri_volt(int volt_to_set, int *curr_volt)
{
	int ret = 0;

#if defined (CONFIG_HISI_PERIDVFS)
	struct peri_volt_poll *pvp = NULL;
	bool is_lowtemp = false;

	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_FB_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	if (peri_get_temperature(pvp)) {
		is_lowtemp = true;
	}

	if (!is_vote_needed_for_low_temp(is_lowtemp, volt_to_set)) {
		HISI_FB_DEBUG("is_lowtemp, return\n");
		return -EINVAL;
	}

	ret = peri_set_volt(pvp, volt_to_set);
	if (ret) {
		HISI_FB_ERR("set votage_value=%d failed, ret=%d", volt_to_set, ret);
		return -EINVAL;
	}
	HISI_FB_DEBUG("set votage_value=%d\n", volt_to_set);

	*curr_volt = peri_get_volt(pvp);
#endif

	return ret;
}

int hisifb_ctrl_dss_voltage_get(struct fb_info *info, void __user *argp)
{
	int voltage_value = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_vote_cmd_t dss_vote_cmd;

	if (NULL == info) {
		HISI_FB_ERR("dss voltage get info NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("dss voltage get hisifd NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("dss voltage get argp NULL Pointer!\n");
		return -EINVAL;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX) {
		HISI_FB_ERR("fb%d, dss voltage get not supported!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->core_clk_upt_support == 0) {
			HISI_FB_DEBUG("no support core_clk_upt\n");
			return 0;
		}
	}
	memset(&dss_vote_cmd, 0, sizeof(dss_vote_cmd_t));

	dss_get_peri_volt(&voltage_value);

	dss_vote_cmd.dss_voltage_level = dpe_get_voltage_level(voltage_value);

	if (copy_to_user(argp, &dss_vote_cmd, sizeof(dss_vote_cmd_t))) {
		HISI_FB_ERR("copy to user fail\n");
		return -EFAULT;
	}
	HISI_FB_DEBUG("fb%d, current_peri_voltage_level = %d\n", hisifd->index, dss_vote_cmd.dss_voltage_level);

	return 0;
}

int hisifb_set_dss_vote_voltage(struct hisi_fb_data_type *hisifd, uint32_t dss_voltage_level, int *curr_volt)
{
	int ret;
	int current_peri_voltage = 0;
	int volt_to_set;
	uint32_t volt_level_to_set;
	uint32_t current_dss_voltage_level;
	struct hisi_fb_data_type *fb0 = hisifd_list[PRIMARY_PANEL_IDX];
	struct hisi_fb_data_type *fb1 = hisifd_list[EXTERNAL_PANEL_IDX];
	struct hisi_fb_data_type *fb2 = hisifd_list[AUXILIARY_PANEL_IDX];
	struct hisi_fb_data_type *fb3 = hisifd_list[MEDIACOMMON_PANEL_IDX];

	if ((hisifd == NULL) || (curr_volt == NULL)) {
		HISI_FB_ERR("null ptr");
		return -EINVAL;
	}

	ret = hisifb_get_other_fb_votelevel(hisifd, &current_dss_voltage_level);
	if (ret) {
		HISI_FB_ERR("set max votage_value=%d failed\n", ret);
		return -EINVAL;
	}

	volt_level_to_set = (dss_voltage_level > current_dss_voltage_level) ?
		dss_voltage_level : current_dss_voltage_level;

	volt_to_set = dpe_get_voltage_value(volt_level_to_set);
	if (volt_to_set < 0) {
		HISI_FB_ERR("get votage_value failed\n");
		return -EINVAL;
	}

	if (dss_set_peri_volt(volt_to_set, curr_volt)) {
		HISI_FB_ERR("dss_set_peri_volt %d failed\n", volt_to_set);
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d->level %d, [%d, %d, %d, %d], set %d, current %d\n", hisifd->index, dss_voltage_level,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_voltage_level : 0,
		volt_to_set, *curr_volt);

	hisifd->dss_vote_cmd.dss_voltage_level = dss_voltage_level;

	return ret;
}

int hisifb_ctrl_dss_voltage_set(struct fb_info *info, void __user *argp)
{
	int ret = 0;
	dss_vote_cmd_t dss_vote_cmd;
	int current_peri_voltage = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (info == NULL) {
		HISI_FB_ERR("dss voltage set info NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (hisifd == NULL) {
		HISI_FB_ERR("dss voltage set hisifd NULL Pointer!\n");
		return -EINVAL;
	}

	if (argp == NULL) {
		HISI_FB_ERR("dss voltage set argp NULL Pointer!\n");
		return -EINVAL;
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX) {
		HISI_FB_ERR("fb%d, dss voltage get not supported!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		HISI_FB_ERR("fb%d, dss voltage get not supported!\n", hisifd->index);
		return -EINVAL;
	}

	if ((hisifd->index == PRIMARY_PANEL_IDX) && (hisifd->core_clk_upt_support == 0)) {
		HISI_FB_DEBUG("no support core_clk_upt\n");
		return ret;
	}

	if ((hisifd->index == AUXILIARY_PANEL_IDX) && (!hisifd_list[PRIMARY_PANEL_IDX]->panel_power_on)) {
		HISI_FB_INFO("fb%d, primary_pane is power off!\n", hisifd->index);
		return -EINVAL;
	}

	down(&g_hisifb_dss_clk_vote_sem);

	ret = copy_from_user(&dss_vote_cmd, argp, sizeof(dss_vote_cmd_t));//lint !e509
	if (ret) {
		HISI_FB_ERR("copy_from_user failed!ret=%d!\n", ret);
		goto volt_vote_out;
	}

	if (dss_vote_cmd.dss_voltage_level == hisifd->dss_vote_cmd.dss_voltage_level) {
		HISI_FB_DEBUG("fb%d same voltage level %d\n", hisifd->index, dss_vote_cmd.dss_voltage_level);
		goto volt_vote_out;
	}

	ret = hisifb_set_dss_vote_voltage(hisifd, dss_vote_cmd.dss_voltage_level, &current_peri_voltage);
	if (ret < 0) {
		goto volt_vote_out;
	}

	dss_vote_cmd.dss_voltage_level = dpe_get_voltage_level(current_peri_voltage);

	if (copy_to_user(argp, &dss_vote_cmd, sizeof(dss_vote_cmd_t))) {
		HISI_FB_ERR("copy to user fail\n");
		ret = -EFAULT;
		goto volt_vote_out;
	}

volt_vote_out:
	up(&g_hisifb_dss_clk_vote_sem);
	return ret;
}
/*lint +e644 +e540*/

int hisifb_ctrl_dss_vote_cmd_set(struct fb_info *info, const void __user *argp)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_vote_cmd_t vote_cmd;

	if (NULL == info) {
		HISI_FB_ERR("dss clk rate set info NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("dss clk rate set hisifd NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("dss clk rate set argp NULL Pointer!\n");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->core_clk_upt_support == 0) {
			HISI_FB_DEBUG("no support core_clk_upt\n");
			return ret;
		}
	}

	ret = copy_from_user(&vote_cmd, argp, sizeof(dss_vote_cmd_t));
	if (ret) {
		HISI_FB_ERR("copy_from_user failed!ret=%d.", ret);
		return ret;
	}

	down(&hisifd->blank_sem);
#ifdef CONFIG_HISI_FB_V510
	down(&hisifd->dp_vote_sem);
#endif

	down(&g_hisifb_dss_clk_vote_sem);

	if (hisifd->index != AUXILIARY_PANEL_IDX) {
		if (!hisifd->panel_power_on) {
			HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
			ret = -EPERM;
			goto err_out;
		}
	}
	ret = set_dss_vote_cmd(hisifd, vote_cmd);

err_out:
	up(&g_hisifb_dss_clk_vote_sem);

#ifdef CONFIG_HISI_FB_V510
	up(&hisifd->dp_vote_sem);
#endif
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_frame_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int val = 0;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	static uint32_t esd_enable = 0;

	if (NULL == dev) {
		HISI_FB_ERR("frame update store dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("frame update store fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("frame update store hisifd NULL Pointer\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("frame update store buf NULL Pointer\n");
		return -1;
	}

	val = (int)simple_strtoul(buf, NULL, 0);

	HISI_FB_INFO("fb%d, val=%d.\n", hisifd->index, val);

	down(&hisifd->blank_sem);

	g_enable_dirty_region_updt =  (val > 0) ? 0 : 1;
	hisifd->frame_update_flag = (val > 0) ? 1 : 0;
	hisifb_set_vsync_activate_state(hisifd, (val > 0) ? true : false);

	if (!is_mipi_cmd_panel(hisifd)) {
		goto err_out;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	hisifb_activate_vsync(hisifd);
	if (val == 1) {
		esd_enable = hisifd->panel_info.esd_enable;
		hisifd->panel_info.esd_enable = 0;
		mdelay(50);
	}

	ldi_frame_update(hisifd, (val > 0) ? true : false);
	if (val == 0) {
		hisifd->vactive0_start_flag = 1;
		mdelay(50);
		hisifd->panel_info.esd_enable = esd_enable;
		esd_enable = 0;
	}
	hisifb_deactivate_vsync(hisifd);

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_frame_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("frame_update_show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("frame_update_show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("frame_update_show hisifd NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("frame_update_show buf NULL Pointer\n");
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n", hisifd->vsync_ctrl.vsync_infinite);
}

static ssize_t hisifb_lcd_model_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd model show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd model show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd model show hisifd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd model show hisifd pdata NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd model show hisifd buf NULL Pointer\n");
		return -1;
	}

	if (pdata->lcd_model_show != NULL) {
		ret = pdata->lcd_model_show(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_lcd_cabc_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd cabc mode show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd cabc mode show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd cabc mode show hisifd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd cabc mode show  pdata NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd cabc mode show buf NULL Pointer\n");
		return -1;
	}

	if (pdata->lcd_cabc_mode_show != NULL) {
		ret = pdata->lcd_cabc_mode_show(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_lcd_cabc_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd cabc mode store dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd cabc mode store fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd cabc mode store hisifd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd cabc mode store pdata NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd cabc mode store buf NULL Pointer\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_cabc_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_cabc_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_ce_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd ce mode show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd ce mode show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd ce mode show hisifd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd ce mode show pdata NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd ce mode show buf NULL Pointer\n");
		return -1;
	}

	if (pdata->lcd_ce_mode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_ce_mode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

	return ret;
}

static ssize_t hisifb_lcd_ce_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd ce mode store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd ce mode store fbi NULL Pointer\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd ce mode store hisifd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd ce mode store pdata NULL Pointer\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd ce mode store buf NULL Pointer\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_ce_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_ce_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd ce mode store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_check_reg_show(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd check reg show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd check reg show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd check reg show hisifd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd check reg show pdata NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd check reg show buf NULL Pointer\n");
		return -1;
	}


	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata != NULL && pdata->lcd_check_reg) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_check_reg(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	} else {
		HISI_FB_ERR("lcd_check_reg is NULL\n");
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_frame_count_show(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("frame_count_show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("frame_count_show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("frame_count_show hisifd NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("frame_count_show buf NULL Pointer\n");
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n", hisifd->frame_count);
}

static ssize_t hisifb_lcd_mipi_detect_show(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd mipi detect show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd mipi detect show fbi NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd mipi detect show hisifd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata != NULL && pdata->lcd_mipi_detect) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_mipi_detect(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	} else {
		HISI_FB_ERR("lcd_mipi_detect is NULL\n");
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_mipi_dsi_bit_clk_upt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("mipi_dsi_bit_clk_upt_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("mipi_dsi_bit_clk_upt_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("mipi_dsi_bit_clk_upt_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata ) {
		HISI_FB_ERR("mipi_dsi_bit_clk_upt_show pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("mipi_dsi_bit_clk_upt_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->mipi_dsi_bit_clk_upt_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->mipi_dsi_bit_clk_upt_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_mipi_dsi_bit_clk_upt_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("mipi dsi bit clk upt store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("mipi dsi bit clk upt store fbi NULL Pointer\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("mipi dsi bit clk upt store hisifd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->mipi_dsi_bit_clk_upt_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->mipi_dsi_bit_clk_upt_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("mipi dsi bit clk upt store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_panel_mode_switch_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("fbi NULL Pointer\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd NULL Pointer\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer\n");
		return ret;
	}
	ret = panel_mode_switch_store(hisifd, buf, count);
	if (ret < 0) {
		HISI_FB_INFO("panel_mode_switch_store ret is %d\n", ret);
	}
	return count;
}

static ssize_t hisifb_lcd_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd fps scence show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd fps scence show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd fps scence show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd fps scence show hisifd NULL Pointer!\n");
		return -1;
	}

	ret = snprintf(buf, PAGE_SIZE, "lcd_fps = %d \n", hisifd->panel_info.fps);

	return ret;
}

static ssize_t hisifb_lcd_fps_scence_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	uint32_t val = 0;

	if (NULL == dev) {
		HISI_FB_ERR("lcd fps scence store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd fps scence store fbi NULL Pointer\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd fps scence store hisifd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata ) {
		HISI_FB_ERR("lcd fps scence store pdata NULL Pointer!\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd fps scence store buf NULL Pointer!\n");
		return ret;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		return ret;
	}

	val = (uint32_t)simple_strtoul(buf, NULL, 0);

	if (pdata->lcd_fps_scence_handle != NULL) {
		ret = pdata->lcd_fps_scence_handle(hisifd->pdev, val);
		if (ret < 0) {
			HISI_FB_INFO("lcd fps scence store ret is %d\n", ret);
		}
	}

	return count;
}

static ssize_t hisifb_lcd_hkadc_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_hkadc_debug_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_hkadc_debug_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_hkadc_debug_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_hkadc_debug_show pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd_hkadc_debug_show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_hkadc_debug_show != NULL) {
		ret = pdata->lcd_hkadc_debug_show(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_lcd_hkadc_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd hkadc debug store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd hkadc debug store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd hkadc debug store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd hkadc debug store pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd hkadc debug store buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_hkadc_debug_store != NULL) {
		ret = pdata->lcd_hkadc_debug_store(hisifd->pdev, buf, count);
	}

	return count;
}

static ssize_t hisifb_lcd_gram_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_gram_check_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_gram_check_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_gram_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd gram check store dev  NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd gram check store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd gram check store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd gram check store  pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd gram check store  buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_gram_check_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_gram_check_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd gram check store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_dynamic_sram_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_dynamic_sram_check_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_dynamic_sram_checksum_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_dynamic_sram_checksum_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_dynamic_sram_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd dynamic sram check store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd dynamic sram check store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd dynamic sram check store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd dynamic sram check store pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd dynamic sram check store buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_dynamic_sram_checksum_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_dynamic_sram_checksum_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd dynamic sram check store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}


static ssize_t hisifb_lcd_color_temperature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_color_temperature_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_color_temperature_show dev fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_color_temperature_show dev hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_color_temperature_show dev pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_color_temperature_show dev buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_color_temperature_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_color_temperature_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_color_temperature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd color temperature store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd color temperature store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd color temperature store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd color temperature store pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd color temperature store  buf NULL Pointer!\n");
		return -1;
	}


	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_color_temperature_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_color_temperature_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_ic_color_enhancement_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_ic_color_enhancement_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_ic_color_enhancement_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_ic_color_enhancement_mode_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_ic_color_enhancement_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_ic_color_enhancement_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_ic_color_enhancement_mode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_ic_color_enhancement_mode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_ic_color_enhancement_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd ic color enhancement mode store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd ic color enhancement mode store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd ic color enhancement mode store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd ic color enhancement mode store pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd ic color enhancement mode store buf NULL Pointer!\n");
		return -1;
	}


	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_ic_color_enhancement_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_ic_color_enhancement_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_led_rg_lcd_color_temperature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("led_rg_lcd_color_temperature_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("led_rg_lcd_color_temperature_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("led_rg_lcd_color_temperature_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("led_rg_lcd_color_temperature_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("led_rg_lcd_color_temperature_show buf NULL Pointer!\n");
		return -1;
	}


	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->led_rg_lcd_color_temperature_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->led_rg_lcd_color_temperature_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_led_rg_lcd_color_temperature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("led rg lcd color temperature store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("led rg lcd color temperature store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("led rg lcd color temperature store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("led rg lcd color temperature store pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("led rg lcd color temperature store buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->led_rg_lcd_color_temperature_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->led_rg_lcd_color_temperature_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("led rg lcd color temperature store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_support_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_support_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_support_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_support_mode_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_support_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_support_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_support_mode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_support_mode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

        /**
         * support none if lower lcd_support_mode_show functions have not be returned
         */
        if (0 == ret) {
            HISI_FB_WARNING("fb%d, support none!\n", hisifd->index);
            ret = snprintf(buf, PAGE_SIZE, "%d\n", 0);
        }
err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_support_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd support mode store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd support mode store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd support mode store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd support mode store pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd support mode store buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_support_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_support_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}
err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd support mode store dev ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t hisifb_lcd_comform_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_comform_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_comform_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_comform_mode_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_comform_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_comform_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_comform_mode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_comform_mode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_comform_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd comform mode store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd comform mode store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd comform mode store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd comform mode store pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd comform mode store bufNULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		dpe_update_g_comform_discount(0);
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_comform_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_comform_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd comform mode store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t hisifb_lcd_cinema_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_cinema_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_cinema_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_cinema_mode_show hisfd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_cinema_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_cinema_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_cinema_mode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_cinema_mode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_cinema_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd cinema mode store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd cinema mode store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd cinema mode store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd cinema mode store padata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd cinema mode store buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_cinema_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_cinema_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t hisifb_lcd_voltage_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd voltage enable store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd voltage enable store fbi NULL Pointer\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd voltage enable store hisifd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer\n");
		return ret;
	}


	down(&hisifd->blank_sem);

	if (pdata->lcd_voltage_enable_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_voltage_enable_store(hisifd->pdev, buf, count);
		if (ret < 0) {
			HISI_FB_INFO("lcd voltage enable store ret is %d\n", ret);
		}
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_bist_check(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	char lcd_bist_check_result[512] = {0};

	if (NULL == dev) {
		HISI_FB_ERR("lcd bist check dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd bist check fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd bist check hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd bist check pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd bist check buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_bist_check != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_bist_check(hisifd->pdev, lcd_bist_check_result);
		hisifb_deactivate_vsync(hisifd);
	}

	ret = snprintf(buf, PAGE_SIZE, "%s", lcd_bist_check_result);
	HISI_FB_INFO("LCD bist check result : %s\n", lcd_bist_check_result);

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_func_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd func switch show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd func switch show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd func switch show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd func switch show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd func switch show buf NULL Pointer!\n");
		return -1;
	}

	pinfo = &(hisifd->panel_info);

	ret = snprintf(buf, PAGE_SIZE,
		"xcc_support=%d\n"
		"dsi_bit_clk_upt=%d\n"
		"dirty_region_upt=%d\n"
		"fps_updt_support=%d\n"
		"ifbc_type=%d\n"
		"esd_enable=%d\n"
		"blpwm_input_ena=%d\n"
		"blpwm_precision_type=%d\n"
		"lane_nums=%d\n"
		"panel_effect_support=%d\n"
		"color_temp_rectify_support=%d\n"
		"hiace=%d\n"
		"effect_enable=%d\n"
		"effect_debug=%d\n",
		pinfo->xcc_support,
		pinfo->dsi_bit_clk_upt_support,
		g_enable_dirty_region_updt,
		pinfo->fps_updt_support,
		pinfo->ifbc_type,
		pinfo->esd_enable,
		pinfo->blpwm_input_ena,
		pinfo->blpwm_precision_type,
		pinfo->mipi.lane_nums + 1,
		pinfo->panel_effect_support,
		pinfo->color_temp_rectify_support,
		pinfo->hiace_support,
		g_enable_effect,
		g_debug_effect);

	return ret;
}

static u32 xcc_table_def[12] = {0x0, 0x8000, 0x0,0x0,0x0,0x0,0x8000,0x0,0x0,0x0,0x0,0x8000,};

static void hisifb_lcd_func_switch_store_xcc_support(struct hisi_panel_info *pinfo,
	const char *command)
{
	if (NULL == pinfo) {
	HISI_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
	return;
	}

	if (NULL == command) {
		HISI_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("xcc_support:", command, strlen("xcc_support:"))) {
		if('0' == command[strlen("xcc_support:")]) {
			pinfo->xcc_support = 0;
			if(pinfo->xcc_table != NULL) {
				pinfo->xcc_table[1] = 0x8000;
				pinfo->xcc_table[6] = 0x8000;
				pinfo->xcc_table[11] = 0x8000;
			}
			HISI_FB_INFO("xcc_support disable\n");
		} else {
			pinfo->xcc_support = 1;
			if (pinfo->xcc_table == NULL) {
				pinfo->xcc_table = xcc_table_def;
				pinfo->xcc_table_len = ARRAY_SIZE(xcc_table_def);
			}
			HISI_FB_INFO("xcc_support enable\n");
		}
	}
}

static void hisifb_lcd_func_switch_store_blpwm(struct hisi_panel_info *pinfo, const char *command)
{
	if (NULL == pinfo) {
		HISI_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (NULL == command) {
		HISI_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("blpwm_input_ena:", command, strlen("blpwm_input_ena:"))) {
		if('0' == command[strlen("blpwm_input_ena:")]) {
			pinfo->blpwm_input_ena = 0;
			HISI_FB_INFO("blpwm_input_ena disable\n");
		} else {
			pinfo->blpwm_input_ena = 1;
			HISI_FB_INFO("blpwm_input_ena enable\n");
		}
	}

	if (!strncmp("blpwm_precision_type:", command, strlen("blpwm_precision_type:"))) {
		if('0' == command[strlen("blpwm_precision_type:")]) {
			pinfo->blpwm_precision_type = 0;
			HISI_FB_INFO("blpwm_precision_type default\n");
		} else {
			pinfo->blpwm_precision_type = BLPWM_PRECISION_10000_TYPE;
			HISI_FB_INFO("blpwm_precision_type BLPWM_PRECISION_10000_TYPE\n");
		}
	}
}

static int hisifb_lcd_func_switch_store_lane_nums(struct hisi_fb_data_type *hisifd, struct hisi_panel_info *pinfo, const char *command)
{
	if (NULL == pinfo) {
		HISI_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return 0;
	}

	if (NULL == command) {
		HISI_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return 0;
	}

	if (!strncmp("lane_nums:", command, strlen("lane_nums:"))) {
		if (hisifd->panel_power_on) {
			HISI_FB_ERR("fb%d, lane_nums can be changed when panel power off, BUT panel power on!\n", hisifd->index);
			return -1;
		}

		if(('1' == command[strlen("lane_nums:")]) && (pinfo->mipi.lane_nums_select_support & DSI_1_LANES_SUPPORT)) {
			pinfo->mipi.lane_nums = DSI_1_LANES;
			HISI_FB_INFO("lane_nums: DSI_1_LANES\n");
		} else if (('2' == command[strlen("lane_nums:")]) && (pinfo->mipi.lane_nums_select_support & DSI_2_LANES_SUPPORT)) {
			pinfo->mipi.lane_nums = DSI_2_LANES;
			HISI_FB_INFO("lane_nums: DSI_2_LANES\n");
		} else if (('3' == command[strlen("lane_nums:")]) && (pinfo->mipi.lane_nums_select_support & DSI_3_LANES_SUPPORT)) {
			pinfo->mipi.lane_nums = DSI_3_LANES;
			HISI_FB_INFO("lane_nums: DSI_3_LANES\n");
		} else {
			pinfo->mipi.lane_nums = DSI_4_LANES;
			HISI_FB_INFO("lane_nums: DSI_4_LANES\n");
		}
	}
	return 0;
}

static void hisifb_lcd_func_switch_store_esd_fps(struct hisi_panel_info *pinfo, const char *command)
{
	if (NULL == pinfo) {
		HISI_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (NULL == command) {
		HISI_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("esd_enable:", command, strlen("esd_enable:"))) {
		if('0' == command[strlen("esd_enable:")]) {
			pinfo->esd_enable = 0;
			HISI_FB_INFO("esd_enable disable\n");
		} else {
			pinfo->esd_enable = 1;
			HISI_FB_INFO("esd_enable enable\n");
		}
	}

	if (!strncmp("fps_updt_support:", command, strlen("fps_updt_support:"))) {
		if('0' == command[strlen("fps_updt_support:")]) {
			pinfo->fps_updt_support = 0;
			HISI_FB_INFO("fps_updt_support disable\n");
		} else {
			pinfo->fps_updt_support = 1;
			HISI_FB_INFO("fps_updt_support enable\n");
		}
	}
}

static void hisifb_lcd_func_switch_store_lcd_info(struct hisi_panel_info *pinfo, const char *command)
{
	if (NULL == pinfo) {
		HISI_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (NULL == command) {
		HISI_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("dsi_bit_clk_upt:", command, strlen("dsi_bit_clk_upt:"))) {
		if('0' == command[strlen("dsi_bit_clk_upt:")]) {
			pinfo->dsi_bit_clk_upt_support = 0;
			HISI_FB_INFO("dsi_bit_clk_upt disable\n");
		} else {
			pinfo->dsi_bit_clk_upt_support = 1;
			HISI_FB_INFO("dsi_bit_clk_upt enable\n");
		}
	}

	if (!strncmp("dirty_region_upt:", command, strlen("dirty_region_upt:"))) {
		if('0' == command[strlen("dirty_region_upt:")]) {
			g_enable_dirty_region_updt = 0;
			HISI_FB_INFO("dirty_region_upt disable\n");
		} else {
			g_enable_dirty_region_updt = 1;
			HISI_FB_INFO("dirty_region_upt enable\n");
		}
	}

	if (!strncmp("ifbc_type:", command, strlen("ifbc_type:"))) {
		if ('0' == command[strlen("ifbc_type:")]) {
			if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) {
				//ldi
				pinfo->ldi.h_back_porch *= pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_front_porch *= pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_pulse_width *= pinfo->pxl_clk_rate_div;

				pinfo->pxl_clk_rate_div = 1;
				pinfo->ifbc_type = IFBC_TYPE_NONE;
				HISI_FB_INFO("ifbc_type changed to IFBC_TYPE_NONE\n");
			}
		} else if ('7' == command[strlen("ifbc_type:")]) {
			if (pinfo->ifbc_type == IFBC_TYPE_NONE) {
				pinfo->pxl_clk_rate_div = 3;

				//ldi
				pinfo->ldi.h_back_porch /= pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_front_porch /= pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_pulse_width /= pinfo->pxl_clk_rate_div;

				pinfo->ifbc_type = IFBC_TYPE_VESA3X_SINGLE;
				HISI_FB_INFO("ifbc_type changed to IFBC_TYPE_VESA3X_SINGLE\n");
			}
		}
	}

	hisifb_lcd_func_switch_store_esd_fps(pinfo, command);
}

static ssize_t hisifb_lcd_func_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char command[MAX_BUF] = {0};

	if (NULL == dev) {
		HISI_FB_ERR("lcd func switch store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd func switch store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd func switch store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd func switch store pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd func switch store buf NULL Pointer!\n");
		return -1;
	}

	if (strlen(buf) >= MAX_BUF) {
		HISI_FB_ERR("buf overflow!\n");
		return -1;
	}

	pinfo = &(hisifd->panel_info);
	// cppcheck-suppress *
	if (!sscanf(buf, "%s", command)) {/* [false alarm] */
		HISI_FB_INFO("bad command(%s)\n", command);
		return count;
	}

	down(&hisifd->blank_sem);

	hisifb_activate_vsync(hisifd);

	hisifb_lcd_func_switch_store_xcc_support(pinfo, command);
	hisifb_lcd_func_switch_store_lcd_info(pinfo, command);
	hisifb_lcd_func_switch_store_blpwm(pinfo, command);

	if (hisifb_lcd_func_switch_store_lane_nums(hisifd, pinfo, command) < 0) {
		goto out;
	}

	if (!strncmp("panel_effect_support:", command, strlen("panel_effect_support:"))) {
		if('0' == command[strlen("panel_effect_support:")]) {
			pinfo->panel_effect_support = 0;
			HISI_FB_INFO("panel_effect_support disable\n");
		} else {
			pinfo->panel_effect_support = 1;
			HISI_FB_INFO("panel_effect_support enable\n");
		}
	}

	if (!strncmp("color_temp_rectify_support:", command, strlen("color_temp_rectify_support:"))) {
		if('0' == command[strlen("color_temp_rectify_support:")]) {
			pinfo->color_temp_rectify_support = 0;
			HISI_FB_INFO("color_temp_rect disable\n");
		} else {
			pinfo->color_temp_rectify_support = 1;
			HISI_FB_INFO("color_temp_rect enable\n");
		}
	}

	hisifb_display_effect_func_switch(hisifd, command);

out:
	hisifb_deactivate_vsync(hisifd);

	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_sleep_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (pdata->lcd_sleep_ctrl_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_sleep_ctrl_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_sleep_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd sleep ctrl store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd sleep ctrl store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd sleep ctrl store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd sleep ctrl store pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd sleep ctrl store buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_sleep_ctrl_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_sleep_ctrl_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd sleep ctrl store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_effect_al_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_al_show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_al_show buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_al_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t hisifb_effect_al_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_al_store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_al_store buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_al_ctrl_store(dev_get_drvdata(dev), buf, count);
}

static ssize_t hisifb_effect_ce_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_ce_show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_ce_show buf NULL Pointer!\n");
		return -1;
	}

	return 0;

}

static ssize_t hisifb_effect_ce_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_ce_store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_ce_store buf NULL Pointer!\n");
		return -1;
	}

	return 0;

}//lint !e715

static ssize_t hisifb_effect_hdr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_hdr_mode_show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_hdr_mode_show buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_ce_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t hisifb_effect_hdr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_hdr_mode_store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_hdr_mode_store buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_ce_ctrl_store(dev_get_drvdata(dev), buf, count);
}

static ssize_t hisifb_effect_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_bl_show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_bl_show buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_bl_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t hisifb_effect_bl_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_bl_enable_show deb NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_bl_enable_show buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_bl_enable_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t hisifb_effect_bl_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_bl_enable_store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_bl_enable_store buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_bl_enable_ctrl_store(dev_get_drvdata(dev), buf, count);
}

static ssize_t hisifb_effect_sre_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_sre_show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_sre_show buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_sre_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t hisifb_effect_sre_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (NULL == dev) {
		HISI_FB_ERR("effect_sre_store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("effect_sre_store buf NULL Pointer!\n");
		return -1;
	}

	return hisifb_display_effect_sre_ctrl_store(dev_get_drvdata(dev), buf, count);
}

static ssize_t hisifb_effect_metadata_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("effect metadata show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("effect metadata show fbi NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("effect metadata show buf NULL Pointer!\n");
		return -1;
	}

	return 0;
}

static ssize_t hisifb_effect_metadata_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("effect metadata store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("effect metadata store fbi NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("effect metadata store buf NULL Pointer!\n");
		return -1;
	}

	return 0;
}

static ssize_t hisifb_effect_available_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int value = 0; //bit0:hiace; bit1:csc; bit2:bitextend; bit3:dither; bit4:arsr1p; bit6:acm; bit7:igm; bit8:xcc; bit9:gmp; bit10:gamma
	struct fb_info *fbi = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("effect available show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("effect available show fbi NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("effect available show buf NULL Pointer!\n");
		return -1;
	}

	value = 0;

	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

static ssize_t hisifb_lcd_test_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd_test_config_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_test_config_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_test_config_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_test_config_show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd_test_config_show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (pdata->lcd_test_config_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_test_config_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_support_checkmode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd support checkmode show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd support checkmode show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd support checkmode show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd support checkmode show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd support checkmode show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_support_checkmode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_support_checkmode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

	return ret;
}

static ssize_t hisifb_lcd_test_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd test config store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd test config store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd test config store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd test config store pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd test config store buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (pdata->lcd_test_config_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_test_config_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_reg_read_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd reg read show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd reg read show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd reg read show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd reg read show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd reg read show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_INFO("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_reg_read_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_reg_read_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_reg_read_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd reg read store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd reg read store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd reg read store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd reg read store pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd reg read store buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_INFO("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_reg_read_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_reg_read_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_lp2hs_mipi_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd lp2hs mipi check show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd lp2hs mipi check show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd lp2hs mipi check show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd lp2hs mipi check show pdata NULL Pointer!\n");
		return -1;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd lp2hs mipi check show buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (pdata->lcd_lp2hs_mipi_check_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_lp2hs_mipi_check_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_lp2hs_mipi_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd lp2hs mipi check store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd lp2hs mipi check store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd lp2hs mipi check store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd lp2hs mipi check store pdata NULL Pointer!\n");
		return ret;
	}
	if (NULL == buf) {
		HISI_FB_ERR("lcd lp2hs mipi check store buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (pdata->lcd_lp2hs_mipi_check_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_lp2hs_mipi_check_store(hisifd->pdev, buf, count);
		if (ret < 0) {
			HISI_FB_INFO("lcd lp2hs mipi check store ret is %d\n", ret);
		}
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_inversion_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd inversion store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd inversion store  fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd inversion store  hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd inversion store  pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd inversion store  buf NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_inversion_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_inversion_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_inversion_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd inversion show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd inversion show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd inversion show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd inversion show pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd inversion show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_inversion_show != NULL) {
		ret = pdata->lcd_inversion_show(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_lcd_scan_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd scan store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd scan store  fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd scan store  hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd scan store  pdata NULL Pointer!\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd scan store  buf NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}
	if((saved_command_line != NULL) && (strstr(saved_command_line, "androidboot.swtype=factory") != NULL)) {
		if (pdata->lcd_scan_store != NULL) {
			hisifb_activate_vsync(hisifd);
			ret = pdata->lcd_scan_store(hisifd->pdev, buf, count);
			hisifb_deactivate_vsync(hisifd);
		}
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd scan store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_scan_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd scan show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd_scan_show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd_scan_show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd_scan_show pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd_scan_show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_scan_show != NULL) {
		ret = pdata->lcd_scan_show(hisifd->pdev, buf);
	}

	return ret;
}
static ssize_t hisifb_lcd_hbm_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd hbm ctrl show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd hbm ctrl show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd hbm ctrl show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd hbm ctrl show pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd hbm ctrl show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_hbm_ctrl_show != NULL) {
		ret = pdata->lcd_hbm_ctrl_show(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_lcd_hbm_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd hbm ctrl store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd hbm ctrl store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd hbm ctrl store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("lcd hbm ctrl store pdata NULL Pointer!\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd hbm ctrl store buf NULL Pointer!\n");
		return ret;
	}

	if (pdata->lcd_hbm_ctrl_store != NULL) {
		ret = pdata->lcd_hbm_ctrl_store(hisifd->pdev, buf, count);
	}

	if (ret < 0) {
		HISI_FB_INFO("lcd hbm ctrl store ret is %d\n", ret);
	}
	return count;
}

static ssize_t hisifb_lcd_amoled_vr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd amoled vr mode show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd amoled vr mode show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd amoled vr mode show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd amoled vr mode show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_INFO("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_amoled_vr_mode_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_amoled_vr_mode_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_amoled_vr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd amoled vr mode store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd amoled vr mode store buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd amoled vr mode store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd amoled vr mode store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_amoled_vr_mode_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_amoled_vr_mode_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}


static ssize_t hisifb_lcd_acl_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd acl ctrl show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd acl ctrl show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd acl ctrl show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd acl ctrl show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->lcd_acl_ctrl_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_acl_ctrl_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_acl_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd acl ctrl store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd acl ctrl store buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd acl ctrl store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd acl ctrl store hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_acl_ctrl_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_acl_ctrl_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}


static ssize_t hisifb_amoled_pcd_errflag_check(struct device* dev,
	 struct device_attribute* attr, char* buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("amoled pcd errflag check dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("amoled pcd errflag check fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("amoled pcd errflag check hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("amoled pcd errflag check pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("amoled pcd errflag check buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->amoled_pcd_errflag_check != NULL) {
	 ret = pdata->amoled_pcd_errflag_check(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("panel info show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("panel info show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("panel info show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("panel info show pdata NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("panel info show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->panel_info_show != NULL) {
		ret = pdata->panel_info_show(hisifd->pdev, buf);
	}

	return ret;
}

static ssize_t hisifb_lcd_acm_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd acm state show dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd acm state show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd acm state show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd acm state show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_acm_state_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_acm_state_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_acm_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd acm state store dev NULL Pointer!\n");
		return ret;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd acm state store buf NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd acm state store fbi NULL Pointer!\n");
		return ret;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd acm state store hisifd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return ret;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_acm_state_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_acm_state_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	if (ret < 0) {
		HISI_FB_INFO("lcd acm state store ret is %d\n", ret);
	}
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_lcd_gmp_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd gmp state showN dev ULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd gmp state show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd gmp state show fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("lcd gmp state show hisifd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_gmp_state_show != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_gmp_state_show(hisifd->pdev, buf);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return ret;
}

static ssize_t hisifb_lcd_gmp_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("lcd gmp state store dev NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("lcd gmp state store buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("lcd gmp state store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (NULL == pdata) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_gmp_state_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->lcd_gmp_state_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);

	return count;
}

static ssize_t hisifb_gamma_dynamic_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == dev) {
		HISI_FB_ERR("gamma dynamic store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("gamma dynamic store fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("gamma dynamic store hisifd NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("gamma dynamic store bud NULL Pointer!\n");
		return -1;
	}

	return count;
}

static ssize_t hisi_alpm_function_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	if (dev == NULL) {
		HISI_FB_ERR("alpm function store NULL dev Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		HISI_FB_ERR("alpm function store NULL buf Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		HISI_FB_ERR("alpm function store NULL fbi Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (hisifd == NULL) {
		HISI_FB_ERR("alpm function storeNULL hisifd Pointer!\n");
		return -1;
	}
	if (strlen(buf) >= MAX_BUF) {
		HISI_FB_ERR("buf overflow!\n");
		return -1;
	}

	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		return -1;
	}

	ret = sscanf(buf, "%u", &hisifd->aod_function);
	if (!ret) {
		HISI_FB_ERR("sscanf return invaild:%zd\n", ret);
		return -1;
	}

	HISI_FB_INFO("aod_function:%d\n", hisifd->aod_function);
	return count;
}

static ssize_t hisi_alpm_function_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (dev == NULL) {
		HISI_FB_ERR("alpm function show NULL dev Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		HISI_FB_ERR("alpm function show NULL buf Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		HISI_FB_ERR("alpm function show NULL fbi Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (hisifd == NULL) {
		HISI_FB_ERR("alpm function show NULL hisifd Pointer!\n");
		return -1;
	}

	ret = snprintf(buf, PAGE_SIZE, "aod_function = %d \n", hisifd->aod_function);

	return ret;
}

static ssize_t hisi_alpm_setting_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct hisi_fb_panel_data *pdata = NULL;
	ssize_t ret = 0;

	if (dev == NULL) {
		HISI_FB_ERR("alpm setting store NULL dev Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		HISI_FB_ERR("alpm setting store NULL buf Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		HISI_FB_ERR("alpm setting store NULL fbi Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (hisifd == NULL) {
		HISI_FB_ERR("alpm setting store NULL hisifd Pointer!\n");
		return -1;
	}

	if (strlen(buf) >= MAX_BUF) {
		HISI_FB_ERR("buf overflow!\n");
		return -1;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (pdata == NULL) {
		HISI_FB_ERR("NULL pdata Pointer!\n");
		return -1;
	}

	down(&hisifd->blank_sem);
	pinfo = &(hisifd->panel_info);
	if (!hisifd->panel_power_on) {
		HISI_FB_ERR("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}

	if (pdata->amoled_alpm_setting_store != NULL) {
		hisifb_activate_vsync(hisifd);
		ret = pdata->amoled_alpm_setting_store(hisifd->pdev, buf, count);
		hisifb_deactivate_vsync(hisifd);
	}

err_out:
	up(&hisifd->blank_sem);
	return count;
}

/*lint -e665, -e514, -e84, -e866, -e886, -e846, -e778*/
static DEVICE_ATTR(frame_update, S_IRUGO|S_IWUSR, hisifb_frame_update_show, hisifb_frame_update_store);
static DEVICE_ATTR(lcd_model, 0644, hisifb_lcd_model_show, NULL);
static DEVICE_ATTR(lcd_cabc_mode, S_IRUGO|S_IWUSR, hisifb_lcd_cabc_mode_show, hisifb_lcd_cabc_mode_store);
static DEVICE_ATTR(lcd_ce_mode, S_IRUGO|S_IWUSR, hisifb_lcd_ce_mode_show, hisifb_lcd_ce_mode_store);
static DEVICE_ATTR(check_lcd_status, S_IRUGO, hisifb_lcd_check_reg_show, NULL);
static DEVICE_ATTR(lcd_mipi_detect, S_IRUGO, hisifb_lcd_mipi_detect_show, NULL);
static DEVICE_ATTR(frame_count, S_IRUGO, hisifb_frame_count_show, NULL);
static DEVICE_ATTR(mipi_dsi_bit_clk_upt, S_IRUGO|S_IWUSR, hisifb_mipi_dsi_bit_clk_upt_show, hisifb_mipi_dsi_bit_clk_upt_store);
static DEVICE_ATTR(panel_mode_switch, S_IRUGO|S_IWUSR, NULL, hisifb_panel_mode_switch_store);
static DEVICE_ATTR(lcd_hkadc, S_IRUGO|S_IWUSR, hisifb_lcd_hkadc_debug_show, hisifb_lcd_hkadc_debug_store);
static DEVICE_ATTR(lcd_checksum, S_IRUGO|S_IWUSR, hisifb_lcd_gram_check_show, hisifb_lcd_gram_check_store);
static DEVICE_ATTR(lcd_dynamic_checksum, S_IRUGO|S_IWUSR, hisifb_lcd_dynamic_sram_check_show, hisifb_lcd_dynamic_sram_check_store);
static DEVICE_ATTR(lcd_color_temperature, S_IRUGO|S_IWUSR, hisifb_lcd_color_temperature_show, hisifb_lcd_color_temperature_store);
static DEVICE_ATTR(lcd_ic_color_enhancement_mode, S_IRUGO|S_IWUSR, hisifb_lcd_ic_color_enhancement_mode_show, hisifb_lcd_ic_color_enhancement_mode_store);
static DEVICE_ATTR(led_rg_lcd_color_temperature, S_IRUGO|S_IWUSR, hisifb_led_rg_lcd_color_temperature_show, hisifb_led_rg_lcd_color_temperature_store);
static DEVICE_ATTR(lcd_comform_mode, S_IRUGO|S_IWUSR, hisifb_lcd_comform_mode_show, hisifb_lcd_comform_mode_store);
static DEVICE_ATTR(lcd_cinema_mode, S_IRUGO|S_IWUSR, hisifb_lcd_cinema_mode_show, hisifb_lcd_cinema_mode_store);
static DEVICE_ATTR(lcd_support_mode, S_IRUGO|S_IWUSR, hisifb_lcd_support_mode_show, hisifb_lcd_support_mode_store);
static DEVICE_ATTR(lcd_voltage_enable, S_IWUSR, NULL, hisifb_lcd_voltage_enable_store);
static DEVICE_ATTR(lcd_bist_check, S_IRUSR|S_IRGRP, hisifb_lcd_bist_check, NULL);
static DEVICE_ATTR(lcd_func_switch, S_IRUGO|S_IWUSR, hisifb_lcd_func_switch_show, hisifb_lcd_func_switch_store);
static DEVICE_ATTR(lcd_sleep_ctrl, S_IRUGO|S_IWUSR, hisifb_lcd_sleep_ctrl_show, hisifb_lcd_sleep_ctrl_store);
static DEVICE_ATTR(effect_al, S_IRUGO|S_IWUSR, hisifb_effect_al_show, hisifb_effect_al_store);
static DEVICE_ATTR(effect_ce, S_IRUGO|S_IWUSR, hisifb_effect_ce_show, hisifb_effect_ce_store);
static DEVICE_ATTR(effect_hdr_mode, S_IRUGO|S_IWUSR, hisifb_effect_hdr_mode_show, hisifb_effect_hdr_mode_store); //lint !e866
static DEVICE_ATTR(effect_bl, S_IRUGO, hisifb_effect_bl_show, NULL);
static DEVICE_ATTR(effect_bl_enable, S_IRUGO|S_IWUSR, hisifb_effect_bl_enable_show, hisifb_effect_bl_enable_store);
static DEVICE_ATTR(effect_sre, S_IRUGO|S_IWUSR, hisifb_effect_sre_show, hisifb_effect_sre_store);
static DEVICE_ATTR(effect_metadata, S_IRUGO|S_IWUSR, hisifb_effect_metadata_show, hisifb_effect_metadata_store);
static DEVICE_ATTR(effect_available, S_IRUGO, hisifb_effect_available_show, NULL);
static DEVICE_ATTR(lcd_test_config, 0640, hisifb_lcd_test_config_show, hisifb_lcd_test_config_store);
static DEVICE_ATTR(lcd_reg_read, 0600, hisifb_lcd_reg_read_show, hisifb_lcd_reg_read_store);
static DEVICE_ATTR(lcd_support_checkmode, S_IRUGO|S_IWUSR, hisifb_lcd_support_checkmode_show, NULL);
static DEVICE_ATTR(lcd_lp2hs_mipi_check, S_IRUGO|S_IWUSR, hisifb_lcd_lp2hs_mipi_check_show, hisifb_lcd_lp2hs_mipi_check_store);
static DEVICE_ATTR(lcd_inversion_mode, S_IRUGO|S_IWUSR, hisifb_lcd_inversion_show, hisifb_lcd_inversion_store);
static DEVICE_ATTR(lcd_scan_mode, S_IRUGO|S_IWUSR, hisifb_lcd_scan_show, hisifb_lcd_scan_store);
static DEVICE_ATTR(amoled_pcd_errflag_check, 0644, hisifb_amoled_pcd_errflag_check, NULL);
static DEVICE_ATTR(amoled_hbm, S_IRUGO|S_IWUSR, hisifb_lcd_hbm_ctrl_show, hisifb_lcd_hbm_ctrl_store);
static DEVICE_ATTR(gamma_dynamic, S_IRUGO|S_IWUSR, NULL, hisifb_gamma_dynamic_store);
static DEVICE_ATTR(panel_info, 0644, hisifb_panel_info_show, NULL);
static DEVICE_ATTR(lcd_acm_state, S_IRUGO|S_IWUSR, hisifb_lcd_acm_state_show, hisifb_lcd_acm_state_store);
static DEVICE_ATTR(amoled_acl, S_IRUGO|S_IWUSR, hisifb_lcd_acl_ctrl_show, hisifb_lcd_acl_ctrl_store);
static DEVICE_ATTR(lcd_gmp_state, S_IRUGO|S_IWUSR, hisifb_lcd_gmp_state_show, hisifb_lcd_gmp_state_store);
static DEVICE_ATTR(amoled_vr_mode, 0644, hisifb_lcd_amoled_vr_mode_show, hisifb_lcd_amoled_vr_mode_store);
static DEVICE_ATTR(lcd_fps_scence, (S_IRUGO|S_IWUSR), hisifb_lcd_fps_scence_show, hisifb_lcd_fps_scence_store);
static DEVICE_ATTR(alpm_function, 0644, hisi_alpm_function_show, hisi_alpm_function_store);
static DEVICE_ATTR(alpm_setting, 0644, NULL, hisi_alpm_setting_store);
/*lint +e665, +e514, +e84, +e866, +e886, +e846, +e778*/

void hisifb_sysfs_attrs_add(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->sysfs_attrs_append_fnc != NULL) {
#if !defined(CONFIG_LCDKIT_DRIVER) && !defined (CONFIG_LCD_KIT_DRIVER)
			hisifd->sysfs_attrs_append_fnc(hisifd, &dev_attr_panel_mode_switch.attr);
#endif
			hisifd->sysfs_attrs_append_fnc(hisifd, &dev_attr_check_lcd_status.attr);
		}
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
}
/*lint +e559*/
#pragma GCC diagnostic pop
