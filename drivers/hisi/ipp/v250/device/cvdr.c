
#include <linux/printk.h>
#include "cvdr_drv.h"
#include "axi_jpeg_drv_priv.h"
#include "axi_jpeg_reg_offset.h"
#include "axi_jpeg_reg_offset_field.h"

#define LOG_TAG LOG_MODULE_CVDR_DRV

#define VP_WR_REG_OFFSET (0x10)
#define VP_RD_REG_OFFSET (0x20)
#define NR_RD_REG_OFFSET (0x10)
#define NR_WR_REG_OFFSET (0x10)
#define ONE_REG_OFFSET   (0x4)
#define CVDR_SPARE_NUM   (4)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

unsigned int g_cvdr_vp_wr_id[VP_WR_MAX] = {
	30, //WR_CMDLST
	8,  //CPE_WR0
	9,  //CPE_WR1
	10, //CPE_WR2
	4,  //WR0_SLAM
	5,  //WR1_SLAM
	6,  //WR2_SLAM
};


unsigned int g_cvdr_vp_rd_id[VP_RD_MAX] = {
	4,  //RD_CMDLST
	13, //CPE_RD0
	14, //CPE_RD1
	15, //CPE_RD2
	16, //CPE_RD3
	17, //CPE_RD4
	18, //CPE_RD5
	8,  //RD0_SLAM
	9,  //RD_RDR_DESC
	10, //RD_CMP_REF
};

unsigned int g_cvdr_nr_wr_id[NR_WR_MAX] = {
	6, //WR_RDR_DESC
};


unsigned int g_cvdr_nr_rd_id[NR_RD_MAX] = {
	3,  //RD_CMP_CUR
};

/**********************************************************
 * function name: cvdr_set_debug_enable
 *
 * description:
 *     set cvdr debug enable
 *
 * input:
 *     dev        : cvdr device
 *     wr_peak_en : enable the FIFO peak functionality over the write port
 *     rd_peak_en : enable the FIFO peak functionality over the read port
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_debug_enable(cvdr_dev_t *dev, unsigned char wr_peak_en, unsigned char rd_peak_en)
{
	U_AXI_JPEG_CVDR_DEBUG_EN tmp;
	tmp.u32 = 0;
	tmp.bits.wr_peak_en = wr_peak_en;
	tmp.bits.rd_peak_en = rd_peak_en;
	hispcpe_reg_set(CVDR_REG, AXI_JPEG_AXI_JPEG_CVDR_DEBUG_EN_REG, tmp.u32);
	return CPE_FW_OK;
}

/**********************************************************
 * function name: cvdr_get_debug_info
 *
 * description:
 *     get cvdr debug info
 *
 * input:
 *     dev     : cvdr device
 *     wr_peak : peak number of Data Units used for the write functionality
 *     rd_peak : peak number of Data Units used for the read functionality
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_get_debug_info(cvdr_dev_t *dev, unsigned char *wr_peak, unsigned char *rd_peak)
{
	U_AXI_JPEG_CVDR_DEBUG tmp;
	tmp.u32 = hispcpe_reg_get(CVDR_REG, AXI_JPEG_AXI_JPEG_CVDR_DEBUG_REG);
	*wr_peak = tmp.bits.wr_peak;
	*rd_peak = tmp.bits.rd_peak;
	return CPE_FW_OK;
}

/**********************************************************
 * function name: cvdr_set_nr_wr_config
 *
 * description:
 *     config cvdr non-raster write port(nr_wr) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : nr write port number
 *     en   : enable or bypass nr write port
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_nr_wr_config(cvdr_dev_t *dev, unsigned char port,  unsigned char nr_wr_stop, unsigned char en)
{
	unsigned int reg_addr = 0;
	U_AXI_JPEG_NR_WR_CFG tmp;
	reg_addr = IPP_CVDR_AXI_NR_WR_CFG_0_REG + port * NR_WR_REG_OFFSET;
	tmp.u32 = 0;
	tmp.bits.nr_wr_stop  = nr_wr_stop;
	tmp.bits.nrwr_enable = en;

	switch (port) {
	default:
		CMDLST_SET_REG(dev->base_addr + reg_addr, tmp.u32);
		break;
	}

	return CPE_FW_OK;
}

/**********************************************************
 * function name: cvdr_set_nr_rd_config
 *
 * description:
 *     config ipp cvdr non-raster read port(nr_rd) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : nr read port number
 *     du   : number of allocated DUs
 *     en   : enable or bypass nr read port
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_nr_rd_config(cvdr_dev_t *dev, unsigned char port, unsigned char du, unsigned char nr_rd_stop,
								 unsigned char en)
{
	unsigned int reg_addr = 0;
	U_AXI_JPEG_NR_RD_CFG tmp;
	reg_addr = IPP_CVDR_AXI_NR_RD_CFG_0_REG + port * NR_RD_REG_OFFSET;
	tmp.u32 = 0;
	tmp.bits.nrrd_allocated_du = du;
	tmp.bits.nr_rd_stop = nr_rd_stop;
	tmp.bits.nrrd_enable = en;

	switch (port) {
	default:
		CMDLST_SET_REG(dev->base_addr + reg_addr, tmp.u32);
		break;
	}

	return CPE_FW_OK;
}

static int cvdr_nr_do_config(cvdr_dev_t *dev, cfg_tab_cvdr_t *table)
{
	int i = 0;

	if (NULL == dev || NULL == table) {
		E("input param is invalid!!");
		return CPE_FW_ERR;
	}

	for (i = 0; i < NR_WR_MAX; ++i) {
		if (1 == table->nr_wr_cfg[i].to_use) {
			loge_if_ret(cvdr_set_nr_wr_config(dev, get_cvdr_nr_wr_port_num(i), table->nr_wr_cfg[i].nr_wr_stop,
											  table->nr_wr_cfg[i].en));
			table->nr_wr_cfg[i].to_use = 0;
		}
	}

	for (i = 0; i < NR_RD_MAX ; ++i) {
		if (1 == table->nr_rd_cfg[i].to_use) {
			loge_if_ret(cvdr_set_nr_rd_config(dev, get_cvdr_nr_rd_port_num(i), table->nr_rd_cfg[i].allocated_du,
											  table->nr_rd_cfg[i].nr_rd_stop, table->nr_rd_cfg[i].en));
			table->nr_rd_cfg[i].to_use = 0;
		}
	}

	return CPE_FW_OK;
}

/**********************************************************
 * function name: cvdr_set_vp_wr_ready
 *
 * description:
 *     config cvdr video port write(vp_wr) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : write port number
 *     desc : vp wr configurations info
 *     addr : start address
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_vp_wr_ready(cvdr_dev_t *dev, unsigned char port, cvdr_wr_fmt_desc_t *desc, cvdr_bw_cfg_t *bw)
{
	U_AXI_JPEG_VP_WR_CFG tmp_cfg;
	U_AXI_JPEG_VP_WR_AXI_FS tmp_fs;
	U_AXI_JPEG_VP_WR_AXI_LINE tmp_line;
	U_AXI_JPEG_VP_WR_IF_CFG tmp_if_cfg;
	U_AXI_JPEG_LIMITER_VP_WR tmp_limiter;
	unsigned int limiter_offset = 0;
	unsigned int cfg_offset = 0;
	unsigned int line_offset = 0;
	unsigned int fs_offset = 0;
	unsigned int ifcfg_offset = 0;
	loge_if_ret(desc->fs_addr & 0xF);
	tmp_cfg.u32  = 0;
	tmp_fs.u32   = 0;
	tmp_line.u32 = 0;
	tmp_limiter.u32 = 0;
	tmp_if_cfg.u32 = 0;

	if (NULL == bw) {
		E("vdr_bw_cfg_t* bw NULL!");
		return CPE_FW_ERR;
	}

	tmp_cfg.bits.vpwr_pixel_format       = desc->pix_fmt;
	tmp_cfg.bits.vpwr_pixel_expansion    = desc->pix_expan;
	tmp_cfg.bits.vpwr_last_page          = desc->last_page;
	tmp_fs.bits.vpwr_address_frame_start = (desc->fs_addr >> 2) >> 2;
	tmp_line.bits.vpwr_line_stride       = desc->line_stride;
	tmp_line.bits.vpwr_line_start_wstrb  = 0xf;
	tmp_line.bits.vpwr_line_wrap         = desc->line_wrap;
	tmp_limiter.bits.vpwr_access_limiter_0 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_1 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_2 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_3 = 0xF;
	tmp_limiter.bits.vpwr_access_limiter_reload = 0xF;
	tmp_if_cfg.bits.vpwr_prefetch_bypass = 0;

	switch (port) {
	case 4: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_4_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_4_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_4_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_4_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_4_REG;
		break;
	}

	case 5: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_5_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_5_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_5_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_5_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_5_REG;
		break;
	}

	case 6: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_6_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_6_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_6_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_6_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_6_REG;
		break;
	}

	case 8: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_8_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_8_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_8_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_8_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_8_REG;
		break;
	}

	case 9: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_9_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_9_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_9_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_9_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_9_REG;
		break;
	}

	case 10: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_10_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_10_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_10_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_10_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_10_REG;
		break;
	}

	case 30: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_30_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_CFG_30_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_30_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_30_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_30_REG;
		break;
	}

	default:
		break;
	}

	CMDLST_SET_REG(dev->base_addr + limiter_offset, tmp_limiter.u32);
	CMDLST_SET_REG(dev->base_addr + cfg_offset, tmp_cfg.u32);
	CMDLST_SET_REG(dev->base_addr + line_offset, tmp_line.u32);
	CMDLST_SET_REG(dev->base_addr + ifcfg_offset, tmp_if_cfg.u32);
	CMDLST_SET_REG(dev->base_addr + fs_offset, tmp_fs.u32);

	return CPE_FW_OK;
}

/**********************************************************
 * function name: cvdr_set_vp_rd_ready
 *
 * description:
 *     config cvdr video port read(vp_rd) configurations
 *
 * input:
 *     dev  : cvdr device
 *     port : read port number
 *     desc : vp rd configurations info
 *     addr : start address
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_set_vp_rd_ready(cvdr_dev_t *dev, unsigned char port, cvdr_rd_fmt_desc_t *desc, cvdr_bw_cfg_t *bw)
{
	U_AXI_JPEG_VP_RD_CFG tmp_cfg;
	U_AXI_JPEG_VP_RD_LWG tmp_lwg;
	U_AXI_JPEG_VP_RD_FHG tmp_fhg;
	U_AXI_JPEG_VP_RD_AXI_FS tmp_fs;
	U_AXI_JPEG_VP_RD_AXI_LINE tmp_line;
	U_AXI_JPEG_VP_RD_IF_CFG tmp_if_cfg;
	U_AXI_JPEG_LIMITER_VP_RD tmp_limiter;
	unsigned int limiter_offset = 0;
	unsigned int cfg_offset = 0;
	unsigned int lwg_offset = 0;
	unsigned int fhg_offset = 0;
	unsigned int line_offset = 0;
	unsigned int fs_offset = 0;
	unsigned int ifcfg_offset = 0;
	loge_if_ret(desc->fs_addr & 0xF);
	tmp_cfg.u32  = 0;
	tmp_lwg.u32  = 0;
	tmp_fhg.u32  = 0;
	tmp_fs.u32   = 0;
	tmp_line.u32 = 0;
	tmp_limiter.u32 = 0;
	tmp_if_cfg.u32 = 0;
	tmp_cfg.bits.vprd_pixel_format    = desc->pix_fmt;
	tmp_cfg.bits.vprd_pixel_expansion = desc->pix_expan;
	tmp_cfg.bits.vprd_allocated_du    = desc->allocated_du;
	tmp_cfg.bits.vprd_last_page       = desc->last_page;
	tmp_lwg.bits.vprd_line_size       = desc->line_size;
	tmp_lwg.bits.vprd_horizontal_blanking = desc->hblank;
	tmp_fhg.bits.vprd_frame_size      = desc->frame_size;
	tmp_fhg.bits.vprd_vertical_blanking   = desc->vblank;
	tmp_fs.bits.vprd_axi_frame_start  = (desc->fs_addr >> 2) >> 2;
	tmp_line.bits.vprd_line_stride    = desc->line_stride;
	tmp_line.bits.vprd_line_wrap      = desc->line_wrap;
	tmp_limiter.bits.vprd_access_limiter_0 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_1 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_2 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_3 = 0xF;
	tmp_limiter.bits.vprd_access_limiter_reload = 0xF;
	tmp_if_cfg.bits.vprd_prefetch_bypass = 0;

	switch (port) {
	case 4: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_4_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_4_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_4_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_4_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_4_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_4_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_4_REG;
		break;
	}

	case 8: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_8_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_8_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_8_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_8_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_8_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_8_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_8_REG;
		break;
	}

	case 9: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_9_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_9_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_9_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_9_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_9_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_9_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_9_REG;
		break;
	}

	case 10: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_10_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_10_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_10_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_10_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_10_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_10_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_10_REG;
		break;
	}

	case 13: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_13_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_13_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_13_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_13_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_13_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_13_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_13_REG;
		break;
	}

	case 14: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_14_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_14_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_14_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_14_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_14_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_14_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_14_REG;
		break;
	}

	case 15: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_15_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_15_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_15_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_15_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_15_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_15_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_15_REG;
		break;
	}

	case 16: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_16_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_16_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_16_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_16_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_16_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_16_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_16_REG;
		break;
	}

	case 17: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_17_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_17_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_17_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_17_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_17_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_17_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_17_REG;
		break;
	}

	case 18: {
		limiter_offset = AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_18_REG;
		cfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_CFG_18_REG;
		lwg_offset = AXI_JPEG_AXI_JPEG_VP_RD_LWG_18_REG;
		fhg_offset = AXI_JPEG_AXI_JPEG_VP_RD_FHG_18_REG;
		line_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_18_REG;
		fs_offset = AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_18_REG;
		ifcfg_offset = AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_18_REG;
		break;
	}

	default:
		break;
	}

	CMDLST_SET_REG(dev->base_addr + limiter_offset, tmp_limiter.u32);
	CMDLST_SET_REG(dev->base_addr + cfg_offset, tmp_cfg.u32);
	CMDLST_SET_REG(dev->base_addr + lwg_offset, tmp_lwg.u32);
	CMDLST_SET_REG(dev->base_addr + fhg_offset, tmp_fhg.u32);
	CMDLST_SET_REG(dev->base_addr + line_offset, tmp_line.u32);
	CMDLST_SET_REG(dev->base_addr + ifcfg_offset, tmp_if_cfg.u32);
	CMDLST_SET_REG(dev->base_addr + fs_offset, tmp_fs.u32);

	return CPE_FW_OK;
}

/**********************************************************
 * function name: cvdr_do_config
 *
 * description:
 *     according to cvdr_config_table config cvdr
 *
 * input:
 *     dev    : cvdr device
 *     table  : cvdr configurations info table
 *
 * output:
 *     0  : success
 *     -1 : failed
 ***********************************************************/
static int cvdr_do_config(cvdr_dev_t *dev, cfg_tab_cvdr_t *table)
{
	int i = 0;

	if (NULL == dev || NULL == table) {
		E("input param is invalid!!");
		return CPE_FW_ERR;
	}

	for (i = 0; i < VP_WR_MAX; ++i) {
		if (1 == table->vp_wr_cfg[i].to_use) {
			loge_if_ret(cvdr_set_vp_wr_ready(dev, get_cvdr_vp_wr_port_num(i), &(table->vp_wr_cfg[i].fmt),
											 &(table->vp_wr_cfg[i].bw)));
			table->vp_wr_cfg[i].to_use = 0;
		}
	}

	for (i = VP_RD_MAX - 1; i >= 0; --i) {
		if (1 == table->vp_rd_cfg[i].to_use) {
			loge_if_ret(cvdr_set_vp_rd_ready(dev, get_cvdr_vp_rd_port_num(i), &(table->vp_rd_cfg[i].fmt),
											 &(table->vp_rd_cfg[i].bw)));
			table->vp_rd_cfg[i].to_use = 0;
		}
	}

	for (i = 0; i < NR_WR_MAX; ++i) {
		if (1 == table->nr_wr_cfg[i].to_use) {
			loge_if_ret(cvdr_set_nr_wr_config(dev, get_cvdr_nr_wr_port_num(i), table->nr_wr_cfg[i].nr_wr_stop,
											  table->nr_wr_cfg[i].en));
			table->nr_wr_cfg[i].to_use = 0;
		}
	}

	for (i = 0; i < NR_RD_MAX ; ++i) {
		if (1 == table->nr_rd_cfg[i].to_use) {
			loge_if_ret(cvdr_set_nr_rd_config(dev, get_cvdr_nr_rd_port_num(i), table->nr_rd_cfg[i].allocated_du,
											  table->nr_rd_cfg[i].nr_rd_stop, table->nr_rd_cfg[i].en));
			table->nr_rd_cfg[i].to_use = 0;
		}
	}

	return CPE_FW_OK;
}

int cvdr_prepare_vprd_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	dev->cmd_buf = cmd_buf;
	return CPE_FW_OK;
}

int cvdr_prepare_vpwr_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	dev->cmd_buf = cmd_buf;
	return CPE_FW_OK;
}

int cvdr_prepare_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	ippdev_lock();
	dev->cmd_buf = cmd_buf;
	loge_if_ret(cvdr_do_config(dev, table));
	ippdev_unlock();
	return CPE_FW_OK;
}

int cvdr_prepare_nr_cmd(cvdr_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_cvdr_t *table)
{
	ippdev_lock();
	dev->cmd_buf = cmd_buf;
	loge_if_ret(cvdr_nr_do_config(dev, table));
	ippdev_unlock();
	return CPE_FW_OK;
}


static cvdr_ops_t cvdr_ops = {
	.set_debug_enable     = cvdr_set_debug_enable,
	.get_debug_info       = cvdr_get_debug_info,
	.set_vp_wr_ready      = cvdr_set_vp_wr_ready,
	.set_vp_rd_ready      = cvdr_set_vp_rd_ready,

	.do_config   = cvdr_do_config,
	.prepare_cmd = cvdr_prepare_cmd,
	.prepare_vprd_cmd = cvdr_prepare_vprd_cmd,
	.prepare_vpwr_cmd = cvdr_prepare_vpwr_cmd,
};

cvdr_dev_t g_cvdr_devs[] = {
	[0] =
	{
		.base_addr = JPG_CVDR_ADDR,
		.ops = &cvdr_ops,
	},
};

#pragma GCC diagnostic pop

