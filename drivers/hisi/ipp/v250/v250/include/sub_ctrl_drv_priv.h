// ******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  sub_ctrl_drv_priv.h
// Version       :  1.0
// Date          :  2018-07-02
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// ******************************************************************************

#ifndef __SUB_CTRL_DRV_PRIV_CS_H__
#define __SUB_CTRL_DRV_PRIV_CS_H__

/* Define the union U_DMA_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpg_dw_axi_gatedclock_en : 1   ; /* [0]  */
		unsigned int    jpg_top_apb_force_clk_on : 1   ; /* [1]  */
		unsigned int    reserved_0            : 14  ; /* [15..2]  */
		unsigned int    control_disable_axi_data_packing : 1   ; /* [16]  */
		unsigned int    mst_priority_fd       : 1   ; /* [17]  */
		unsigned int    mst_priority_cvdr     : 1   ; /* [18]  */
		unsigned int    apb_overf_prot        : 2   ; /* [20..19]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_DMA_CRG_CFG0;

/* Define the union U_DMA_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_soft_rst         : 1   ; /* [0]  */
		unsigned int    smmu_soft_rst         : 1   ; /* [1]  */
		unsigned int    smmu_master_soft_rst  : 1   ; /* [2]  */
		unsigned int    cmdlst_soft_rst       : 1   ; /* [3]  */
		unsigned int    reserved_0            : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_DMA_CRG_CFG1;

/* Define the union U_CVDR_MEM_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_mem_ctrl_sp      : 3   ; /* [2..0]  */
		unsigned int    mem_ctrl_sp           : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_MEM_CFG0;

/* Define the union U_CVDR_MEM_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 3   ; /* [2..0]  */
		unsigned int    mem_ctrl_tp           : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_MEM_CFG1;

/* Define the union U_CVDR_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_irq_clr          : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_IRQ_REG0;

/* Define the union U_CVDR_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_irq_mask         : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_IRQ_REG1;

/* Define the union U_CVDR_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_irq_state_mask   : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cvdr_irq_state_raw    : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_IRQ_REG2;

/* Define the union U_CMDLST_CTRL */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ctrl_chn0      : 3   ; /* [2..0]  */
		unsigned int    cmdlst_ctrl_chn1      : 3   ; /* [5..3]  */
		unsigned int    cmdlst_ctrl_chn2      : 3   ; /* [8..6]  */
		unsigned int    cmdlst_ctrl_chn3      : 3   ; /* [11..9]  */
		unsigned int    cmdlst_ctrl_chn4      : 3   ; /* [14..12]  */
		unsigned int    cmdlst_ctrl_chn5      : 3   ; /* [17..15]  */
		unsigned int    cmdlst_ctrl_chn6      : 3   ; /* [20..18]  */
		unsigned int    cmdlst_ctrl_chn7      : 3   ; /* [23..21]  */
		unsigned int    reserved_0            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CTRL;

/* Define the union U_CMDLST_CHN0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn0  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN0;

/* Define the union U_CMDLST_CHN1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn1  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN1;

/* Define the union U_CMDLST_CHN2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn2  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN2;

/* Define the union U_CMDLST_CHN3 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn3  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN3;

/* Define the union U_CMDLST_CHN4 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn4  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN4;

/* Define the union U_CMDLST_CHN5 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn5  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN5;

/* Define the union U_CMDLST_CHN6 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn6  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN6;

/* Define the union U_CMDLST_CHN7 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn7  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_CHN7;

/* Define the union U_CMDLST_R8_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_r8_irq_clr     : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_R8_IRQ_REG0;

/* Define the union U_CMDLST_R8_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_r8_irq_mask    : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_R8_IRQ_REG1;

/* Define the union U_CMDLST_R8_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_r8_irq_state_mask : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cmdlst_r8_irq_state_raw : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_R8_IRQ_REG2;

/* Define the union U_CMDLST_ACPU_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_acpu_irq_clr   : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_ACPU_IRQ_REG0;

/* Define the union U_CMDLST_ACPU_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_acpu_irq_mask  : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_ACPU_IRQ_REG1;

/* Define the union U_CMDLST_ACPU_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_acpu_irq_state_mask : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cmdlst_acpu_irq_state_raw : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_ACPU_IRQ_REG2;

/* Define the union U_CMDLST_IVP_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ivp_irq_clr    : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_IVP_IRQ_REG0;

/* Define the union U_CMDLST_IVP_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ivp_irq_mask   : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_IVP_IRQ_REG1;

/* Define the union U_CMDLST_IVP_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ivp_irq_state_mask : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cmdlst_ivp_irq_state_raw : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMDLST_IVP_IRQ_REG2;

/* Define the union U_JPG_FLUX_CTRL0_0 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_cvdr_r      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL0_0;
/* Define the union U_JPG_FLUX_CTRL0_1 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_cvdr_r      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL0_1;
/* Define the union U_JPG_FLUX_CTRL1_0 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_cvdr_w      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL1_0;
/* Define the union U_JPG_FLUX_CTRL1_1 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_cvdr_w      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL1_1;
/* Define the union U_JPG_FLUX_CTRL2_0 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_fd_r        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL2_0;
/* Define the union U_JPG_FLUX_CTRL2_1 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_fd_r        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL2_1;
/* Define the union U_JPG_FLUX_CTRL3_0 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_fd_w        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL3_0;
/* Define the union U_JPG_FLUX_CTRL3_1 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_fd_w        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_FLUX_CTRL3_1;
/* Define the union U_JPG_RO_STATE */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 16  ; /* [15..0]  */
		unsigned int    jpg_axi_dlock_irq     : 1   ; /* [16]  */
		unsigned int    jpg_axi_dlock_wr      : 1   ; /* [17]  */
		unsigned int    jpg_axi_dlock_slv     : 1   ; /* [18]  */
		unsigned int    jpg_axi_dlock_mst     : 1   ; /* [19]  */
		unsigned int    jpg_axi_dlock_id      : 8   ; /* [27..20]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_RO_STATE;

/* Define the union U_JPGENC_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_clken          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgenc_force_clk_on   : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_CRG_CFG0;

/* Define the union U_JPGENC_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_soft_rst       : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_CRG_CFG1;

/* Define the union U_JPGENC_MEM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_mem_ctrl_sp    : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_MEM_CFG;

/* Define the union U_JPGENC_PREF_STOP */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_prefetch_stop  : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgenc_prefetch_stop_ok : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_PREF_STOP;

/* Define the union U_JPGENC_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_irq_clr        : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    jpgenc_irq_force      : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_IRQ_REG0;

/* Define the union U_JPGENC_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_irq_mask       : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_IRQ_REG1;

/* Define the union U_JPGENC_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_irq_state_mask : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    jpgenc_irq_state_raw  : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGENC_IRQ_REG2;

/* Define the union U_JPGDEC_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_clken          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgdec_force_clk_on   : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_CRG_CFG0;

/* Define the union U_JPGDEC_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_soft_rst       : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_CRG_CFG1;

/* Define the union U_JPGDEC_MEM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_mem_ctrl_sp    : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    jpgdec_mem_ctrl_tp    : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_MEM_CFG;

/* Define the union U_JPGDEC_PREF_STOP */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_prefetch_stop  : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgdec_prefetch_stop_ok : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_PREF_STOP;

/* Define the union U_JPGDEC_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_irq_clr        : 4   ; /* [3..0]  */
		unsigned int    reserved_0            : 12  ; /* [15..4]  */
		unsigned int    jpgdec_irq_force      : 4   ; /* [19..16]  */
		unsigned int    reserved_1            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_IRQ_REG0;

/* Define the union U_JPGDEC_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_irq_mask       : 4   ; /* [3..0]  */
		unsigned int    reserved_0            : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_IRQ_REG1;

/* Define the union U_JPGDEC_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_irq_state_mask : 4   ; /* [3..0]  */
		unsigned int    reserved_0            : 12  ; /* [15..4]  */
		unsigned int    jpgdec_irq_state_raw  : 4   ; /* [19..16]  */
		unsigned int    reserved_1            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPGDEC_IRQ_REG2;

/* Define the union U_CPE_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_clken             : 1   ; /* [0]  */
		unsigned int    mfnr_clken            : 1   ; /* [1]  */
		unsigned int    vbk_clken             : 1   ; /* [2]  */
		unsigned int    reserved_0            : 13  ; /* [15..3]  */
		unsigned int    mcf_force_clk_on      : 1   ; /* [16]  */
		unsigned int    mfnr_force_clk_on     : 1   ; /* [17]  */
		unsigned int    vbk_force_clk_on      : 1   ; /* [18]  */
		unsigned int    reserved_1            : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_CRG_CFG0;

/* Define the union U_CPE_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_soft_rst          : 1   ; /* [0]  */
		unsigned int    mfnr_soft_rst         : 1   ; /* [1]  */
		unsigned int    vbk_soft_rst          : 1   ; /* [2]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_CRG_CFG1;

/* Define the union U_CPE_MEM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_mem_ctrl_sp       : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    mfnr_mem_ctrl_sp      : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    vbk_mem_ctrl_sp       : 3   ; /* [10..8]  */
		unsigned int    reserved_2            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_MEM_CFG;

/* Define the union U_CPE_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_clr           : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_IRQ_REG0;

/* Define the union U_CPE_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_mask          : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_IRQ_REG1;

/* Define the union U_CPE_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_irq_outen         : 2   ; /* [1..0]  */
		unsigned int    mfnr_irq_outen        : 2   ; /* [3..2]  */
		unsigned int    vbk_irq_outen         : 2   ; /* [5..4]  */
		unsigned int    reserved_0            : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_IRQ_REG2;

/* Define the union U_CPE_IRQ_REG3 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_state_mask    : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_IRQ_REG3;

/* Define the union U_CPE_IRQ_REG4 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_state_raw     : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_IRQ_REG4;

/* Define the union U_CROP_VPWR_0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_vpwr0_ihleft      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    cpe_vpwr0_ihright     : 11  ; /* [26..16]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CROP_VPWR_0;

/* Define the union U_CROP_VPWR_1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_vpwr1_ihleft      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    cpe_vpwr1_ihright     : 11  ; /* [26..16]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CROP_VPWR_1;

/* Define the union U_CROP_VPWR_2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_vpwr2_ihleft      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    cpe_vpwr2_ihright     : 11  ; /* [26..16]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CROP_VPWR_2;

/* Define the union U_CPE_mode_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_op_mode           : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CPE_mode_CFG;

/* Define the union U_SLAM_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    slam_clken            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    slam_force_clk_on     : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_CRG_CFG0;

/* Define the union U_SLAM_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    slam_soft_rst         : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_CRG_CFG1;

/* Define the union U_SLAM_MEM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    slam_mem_ctrl_sp      : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_MEM_CFG;

/* Define the union U_SLAM_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    slam_irq_clr          : 14  ; /* [13..0]  */
		unsigned int    reserved_0            : 2   ; /* [15..14]  */
		unsigned int    slam_irq_force        : 14  ; /* [29..16]  */
		unsigned int    reserved_1            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_IRQ_REG0;

/* Define the union U_SLAM_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    slam_irq_mask         : 14  ; /* [13..0]  */
		unsigned int    reserved_0            : 2   ; /* [15..14]  */
		unsigned int    slam_irq_outen        : 2   ; /* [17..16]  */
		unsigned int    reserved_1            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_IRQ_REG1;

/* Define the union U_SLAM_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    slam_irq_state_mask   : 14  ; /* [13..0]  */
		unsigned int    reserved_0            : 2   ; /* [15..14]  */
		unsigned int    slam_irq_state_raw    : 14  ; /* [29..16]  */
		unsigned int    reserved_1            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_IRQ_REG2;

/* Define the union U_RDR_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_clken             : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    rdr_force_clk_on      : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_CRG_CFG0;

/* Define the union U_RDR_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_soft_rst          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_CRG_CFG1;

/* Define the union U_RDR_MEM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_mem_ctrl_sp       : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_MEM_CFG;

/* Define the union U_RDR_PREF_STOP */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_prefetch_stop     : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    rdr_prefetch_stop_ok  : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_PREF_STOP;

/* Define the union U_RDR_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_irq_clr           : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    rdr_irq_force         : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_IRQ_REG0;

/* Define the union U_RDR_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_irq_mask          : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    rdr_irq_outen         : 2   ; /* [17..16]  */
		unsigned int    reserved_1            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_IRQ_REG1;

/* Define the union U_RDR_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_irq_state_mask    : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    rdr_irq_state_raw     : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RDR_IRQ_REG2;

/* Define the union U_CMP_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_clken             : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    cmp_force_clk_on      : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_CRG_CFG0;

/* Define the union U_CMP_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_soft_rst          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_CRG_CFG1;

/* Define the union U_CMP_MEM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_mem_ctrl_sp       : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_MEM_CFG;

/* Define the union U_CMP_PREF_STOP */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_prefetch_stop     : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    cmp_prefetch_stop_ok  : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_PREF_STOP;

/* Define the union U_CMP_IRQ_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_irq_clr           : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    cmp_irq_force         : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_IRQ_REG0;

/* Define the union U_CMP_IRQ_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_irq_mask          : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    cmp_irq_outen         : 2   ; /* [17..16]  */
		unsigned int    reserved_1            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_IRQ_REG1;

/* Define the union U_CMP_IRQ_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_irq_state_mask    : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    cmp_irq_state_raw     : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CMP_IRQ_REG2;

/* Define the union U_HIFD_CRG_CFG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    hifd_clken            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;


} U_HIFD_CRG_CFG0;

/* Define the union U_HIFD_CRG_CFG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    hifd_soft_rst         : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_HIFD_CRG_CFG1;

/* Define the union U_HIFD_MEM_CFG */

typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    hifd_mem_ctrl_sp      : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    hifd_mem_ctrl_sp2     : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    hifd_mem_ctrl_tp      : 3   ; /* [10..8]  */
		unsigned int    reserved_2            : 5   ; /* [15..11]  */
		unsigned int    hifd_rom_ctrl         : 8   ; /* [23..16]  */
		unsigned int    reserved_3            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_HIFD_MEM_CFG;

/* Define the union U_FD_SMMU_MASTER_REG0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    fd_prefetch_initial   : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FD_SMMU_MASTER_REG0;

/* Define the union U_FD_SMMU_MASTER_REG1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    fd_stream_end         : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FD_SMMU_MASTER_REG1;

/* Define the union U_FD_SMMU_MASTER_REG2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    fd_stream_ack         : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FD_SMMU_MASTER_REG2;

/* Define the union U_JPG_DEBUG_0 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_0           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_DEBUG_0;
/* Define the union U_JPG_DEBUG_1 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_1           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_DEBUG_1;
/* Define the union U_JPG_DEBUG_2 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_2           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_DEBUG_2;
/* Define the union U_JPG_DEBUG_3 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_3           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_DEBUG_3;
/* Define the union U_JPG_SEC_CTRL_S */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    top_tz_secure_n       : 1   ; /* [0]  */
		unsigned int    jpgenc_tz_secure_n    : 1   ; /* [1]  */
		unsigned int    jpgdec_tz_secure_n    : 1   ; /* [2]  */
		unsigned int    fd_tz_secure_n        : 1   ; /* [3]  */
		unsigned int    cpe_tz_secure_n       : 1   ; /* [4]  */
		unsigned int    slam_tz_secure_n      : 1   ; /* [5]  */
		unsigned int    orb_tz_secure_n       : 1   ; /* [6]  */
		unsigned int    cmdlst_tz_secure_n    : 1   ; /* [7]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_JPG_SEC_CTRL_S;

//==============================================================================
/* Define the global struct */
typedef struct {
	U_DMA_CRG_CFG0         DMA_CRG_CFG0;
	U_DMA_CRG_CFG1         DMA_CRG_CFG1;
	U_CVDR_MEM_CFG0        CVDR_MEM_CFG0;
	U_CVDR_MEM_CFG1        CVDR_MEM_CFG1;
	U_CVDR_IRQ_REG0        CVDR_IRQ_REG0;
	U_CVDR_IRQ_REG1        CVDR_IRQ_REG1;
	U_CVDR_IRQ_REG2        CVDR_IRQ_REG2;
	U_CMDLST_CTRL          CMDLST_CTRL;
	U_CMDLST_CHN0          CMDLST_CHN0;
	U_CMDLST_CHN1          CMDLST_CHN1;
	U_CMDLST_CHN2          CMDLST_CHN2;
	U_CMDLST_CHN3          CMDLST_CHN3;
	U_CMDLST_CHN4          CMDLST_CHN4;
	U_CMDLST_CHN5          CMDLST_CHN5;
	U_CMDLST_CHN6          CMDLST_CHN6;
	U_CMDLST_CHN7          CMDLST_CHN7;
	U_CMDLST_R8_IRQ_REG0   CMDLST_R8_IRQ_REG0;
	U_CMDLST_R8_IRQ_REG1   CMDLST_R8_IRQ_REG1;
	U_CMDLST_R8_IRQ_REG2   CMDLST_R8_IRQ_REG2;
	U_CMDLST_ACPU_IRQ_REG0 CMDLST_ACPU_IRQ_REG0;
	U_CMDLST_ACPU_IRQ_REG1 CMDLST_ACPU_IRQ_REG1;
	U_CMDLST_ACPU_IRQ_REG2 CMDLST_ACPU_IRQ_REG2;
	U_CMDLST_IVP_IRQ_REG0  CMDLST_IVP_IRQ_REG0;
	U_CMDLST_IVP_IRQ_REG1  CMDLST_IVP_IRQ_REG1;
	U_CMDLST_IVP_IRQ_REG2  CMDLST_IVP_IRQ_REG2;
	U_JPG_FLUX_CTRL0_0     JPG_FLUX_CTRL0_0;
	U_JPG_FLUX_CTRL0_1     JPG_FLUX_CTRL0_1;
	U_JPG_FLUX_CTRL1_0     JPG_FLUX_CTRL1_0;
	U_JPG_FLUX_CTRL1_1     JPG_FLUX_CTRL1_1;
	U_JPG_FLUX_CTRL2_0     JPG_FLUX_CTRL2_0;
	U_JPG_FLUX_CTRL2_1     JPG_FLUX_CTRL2_1;
	U_JPG_FLUX_CTRL3_0     JPG_FLUX_CTRL3_0;
	U_JPG_FLUX_CTRL3_1     JPG_FLUX_CTRL3_1;
	U_JPG_RO_STATE         JPG_RO_STATE;
	U_JPGENC_CRG_CFG0      JPGENC_CRG_CFG0;
	U_JPGENC_CRG_CFG1      JPGENC_CRG_CFG1;
	U_JPGENC_MEM_CFG       JPGENC_MEM_CFG;
	U_JPGENC_PREF_STOP     JPGENC_PREF_STOP;
	U_JPGENC_IRQ_REG0      JPGENC_IRQ_REG0;
	U_JPGENC_IRQ_REG1      JPGENC_IRQ_REG1;
	U_JPGENC_IRQ_REG2      JPGENC_IRQ_REG2;
	U_JPGDEC_CRG_CFG0      JPGDEC_CRG_CFG0;
	U_JPGDEC_CRG_CFG1      JPGDEC_CRG_CFG1;
	U_JPGDEC_MEM_CFG       JPGDEC_MEM_CFG;
	U_JPGDEC_PREF_STOP     JPGDEC_PREF_STOP;
	U_JPGDEC_IRQ_REG0      JPGDEC_IRQ_REG0;
	U_JPGDEC_IRQ_REG1      JPGDEC_IRQ_REG1;
	U_JPGDEC_IRQ_REG2      JPGDEC_IRQ_REG2;
	U_CPE_CRG_CFG0         CPE_CRG_CFG0;
	U_CPE_CRG_CFG1         CPE_CRG_CFG1;
	U_CPE_MEM_CFG          CPE_MEM_CFG;
	U_CPE_IRQ_REG0         CPE_IRQ_REG0;
	U_CPE_IRQ_REG1         CPE_IRQ_REG1;
	U_CPE_IRQ_REG2         CPE_IRQ_REG2;
	U_CPE_IRQ_REG3         CPE_IRQ_REG3;
	U_CPE_IRQ_REG4         CPE_IRQ_REG4;
	U_CROP_VPWR_0          CROP_VPWR_0;
	U_CROP_VPWR_1          CROP_VPWR_1;
	U_CROP_VPWR_2          CROP_VPWR_2;
	U_CPE_mode_CFG         CPE_mode_CFG;
	U_SLAM_CRG_CFG0        SLAM_CRG_CFG0;
	U_SLAM_CRG_CFG1        SLAM_CRG_CFG1;
	U_SLAM_MEM_CFG         SLAM_MEM_CFG;
	U_SLAM_IRQ_REG0        SLAM_IRQ_REG0;
	U_SLAM_IRQ_REG1        SLAM_IRQ_REG1;
	U_SLAM_IRQ_REG2        SLAM_IRQ_REG2;
	U_RDR_CRG_CFG0         RDR_CRG_CFG0;
	U_RDR_CRG_CFG1         RDR_CRG_CFG1;
	U_RDR_MEM_CFG          RDR_MEM_CFG;
	U_RDR_PREF_STOP        RDR_PREF_STOP;
	U_RDR_IRQ_REG0         RDR_IRQ_REG0;
	U_RDR_IRQ_REG1         RDR_IRQ_REG1;
	U_RDR_IRQ_REG2         RDR_IRQ_REG2;
	U_CMP_CRG_CFG0         CMP_CRG_CFG0;
	U_CMP_CRG_CFG1         CMP_CRG_CFG1;
	U_CMP_MEM_CFG          CMP_MEM_CFG;
	U_CMP_PREF_STOP        CMP_PREF_STOP;
	U_CMP_IRQ_REG0         CMP_IRQ_REG0;
	U_CMP_IRQ_REG1         CMP_IRQ_REG1;
	U_CMP_IRQ_REG2         CMP_IRQ_REG2;
	U_HIFD_CRG_CFG0        HIFD_CRG_CFG0;
	U_HIFD_CRG_CFG1        HIFD_CRG_CFG1;
	U_HIFD_MEM_CFG         HIFD_MEM_CFG;
	U_FD_SMMU_MASTER_REG0  FD_SMMU_MASTER_REG0;
	U_FD_SMMU_MASTER_REG1  FD_SMMU_MASTER_REG1;
	U_FD_SMMU_MASTER_REG2  FD_SMMU_MASTER_REG2;
	U_JPG_DEBUG_0          JPG_DEBUG_0;
	U_JPG_DEBUG_1          JPG_DEBUG_1;
	U_JPG_DEBUG_2          JPG_DEBUG_2;
	U_JPG_DEBUG_3          JPG_DEBUG_3;
	U_JPG_SEC_CTRL_S       JPG_SEC_CTRL_S;

} S_SUB_CTRL_REGS_TYPE;

/* Declare the struct pointor of the module SUB_CTRL */
extern S_SUB_CTRL_REGS_TYPE *gopSUB_CTRLAllReg;


#endif /* __SUB_CTRL_DRV_PRIV_CS_H__ */
