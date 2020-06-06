//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  slam_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-07-30
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-07-30 Create file
//******************************************************************************

#ifndef __SLAM_DRV_PRIV_CS_H__
#define __SLAM_DRV_PRIV_CS_H__

/* Define the union U_SLAM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    pyramid_en            : 1   ; /* [0]  */
		unsigned int    gsblur_en             : 1   ; /* [1]  */
		unsigned int    fast_en               : 1   ; /* [2]  */
		unsigned int    nms_en                : 1   ; /* [3]  */
		unsigned int    orient_en             : 1   ; /* [4]  */
		unsigned int    brief_en              : 1   ; /* [5]  */
		unsigned int    freak_en              : 1   ; /* [6]  */
		unsigned int    scoremap_en           : 1   ; /* [7]  */
		unsigned int    gridstat_en           : 1   ; /* [8]  */
		unsigned int    undistort_en          : 1   ; /* [9]  */
		unsigned int    reserved_0            : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SLAM_CFG;

/* Define the union U_IMAGE_SIZE */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    width                 : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    height                : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_IMAGE_SIZE;

/* Define the union U_TOTAL_KPT_NUM */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    total_kpt_num         : 15  ; /* [14..0]  */
		unsigned int    reserved_0            : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_TOTAL_KPT_NUM;

/* Define the union U_PYRAMID_INC_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    scl_inc               : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_PYRAMID_INC_CFG;

/* Define the union U_PYRAMID_VCROP_CFGB */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    scl_vbottom           : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_PYRAMID_VCROP_CFGB;

/* Define the union U_PYRAMID_VCROP_CFGT */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    scl_vtop              : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_PYRAMID_VCROP_CFGT;

/* Define the union U_PYRAMID_HCROP_CFGR */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    scl_hright            : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_PYRAMID_HCROP_CFGR;

/* Define the union U_PYRAMID_HCROP_CFGL */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    scl_hleft             : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_PYRAMID_HCROP_CFGL;

/* Define the union U_GSBLUR_COEF_01 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    coeff_gauss_1         : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    coeff_gauss_0         : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_GSBLUR_COEF_01;

/* Define the union U_GSBLUR_COEF_23 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    coeff_gauss_3         : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    coeff_gauss_2         : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_GSBLUR_COEF_23;

/* Define the union U_THRESHOLD_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    min_th                : 8   ; /* [7..0]  */
		unsigned int    ini_th                : 8   ; /* [15..8]  */
		unsigned int    reserved_0            : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_THRESHOLD_CFG;

/* Define the union U_NMS_WIN_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    nmscell_v             : 4   ; /* [3..0]  */
		unsigned int    nmscell_h             : 5   ; /* [8..4]  */
		unsigned int    reserved_0            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_NMS_WIN_CFG;

/* Define the union U_BLOCK_NUM_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    blk_v_num             : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    blk_h_num             : 5   ; /* [12..8]  */
		unsigned int    reserved_1            : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_BLOCK_NUM_CFG;

/* Define the union U_BLOCK_SIZE_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    blk_v_size            : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 2   ; /* [11..10]  */
		unsigned int    blk_h_size            : 10  ; /* [21..12]  */
		unsigned int    reserved_1            : 10  ; /* [31..22]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_BLOCK_SIZE_CFG;

/* Define the union U_OCTREE_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    grid_max_kpnum        : 8   ; /* [7..0]  */
		unsigned int    max_kpnum             : 15  ; /* [22..8]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_OCTREE_CFG;

/* Define the union U_INC_LUT_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    inc_level             : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_INC_LUT_CFG;

/* Define the union U_UNDISTORT_CX */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cx                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_CX;

/* Define the union U_UNDISTORT_CY */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    cy                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_CY;

/* Define the union U_UNDISTORT_FX */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    fx                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_FX;

/* Define the union U_UNDISTORT_FY */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    fy                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_FY;

/* Define the union U_UNDISTORT_INVFX */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    invfx                 : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_INVFX;

/* Define the union U_UNDISTORT_INVFY */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    invfy                 : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_INVFY;

/* Define the union U_UNDISTORT_K1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    k1                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_K1;

/* Define the union U_UNDISTORT_K2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    k2                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_K2;

/* Define the union U_UNDISTORT_K3 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    k3                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_K3;

/* Define the union U_UNDISTORT_P1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    p1                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_P1;

/* Define the union U_UNDISTORT_P2 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    p2                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_UNDISTORT_P2;

/* Define the union U_CVDR_CFG_0 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_size        : 13  ; /* [12..0]  */
		unsigned int    reserved_0            : 3   ; /* [15..13]  */
		unsigned int    vprd_horizontal_blanking : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_CFG_0;

/* Define the union U_CVDR_CFG_1 */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_stride      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    vprd_line_wrap        : 13  ; /* [28..16]  */
		unsigned int    reserved_1            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_CVDR_CFG_1;

/* Define the union U_DEBUG 0 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int debug0                 : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_DEBUG_0;
/* Define the union U_DEBUG 1 */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int debug1                 : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_DEBUG_1;
/* Define the union U_SCORE_HIST */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    score_histbin         : 12  ; /* [11..0]  */
		unsigned int    reserved_0            : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SCORE_HIST;

/* Define the union U_BRIEF_PATTERN */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    pattern_x0            : 6   ; /* [5..0]  */
		unsigned int    reserved_0            : 2   ; /* [7..6]  */
		unsigned int    pattern_y0            : 6   ; /* [13..8]  */
		unsigned int    reserved_1            : 2   ; /* [15..14]  */
		unsigned int    pattern_x1            : 6   ; /* [21..16]  */
		unsigned int    reserved_2            : 2   ; /* [23..22]  */
		unsigned int    pattern_y1            : 6   ; /* [29..24]  */
		unsigned int    reserved_3            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_BRIEF_PATTERN;

/* Define the union U_SCORE_THESHOLD */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int score_th               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SCORE_THESHOLD;
/* Define the union U_KPT_NUMBER */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int kpt_num                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_KPT_NUMBER;
//==============================================================================
/* Define the global struct */
typedef struct {
	U_SLAM_CFG             SLAM_CFG;
	U_IMAGE_SIZE           IMAGE_SIZE;
	U_TOTAL_KPT_NUM        TOTAL_KPT_NUM;
	U_PYRAMID_INC_CFG      PYRAMID_INC_CFG;
	U_PYRAMID_VCROP_CFGB   PYRAMID_VCROP_CFGB;
	U_PYRAMID_VCROP_CFGT   PYRAMID_VCROP_CFGT;
	U_PYRAMID_HCROP_CFGR   PYRAMID_HCROP_CFGR;
	U_PYRAMID_HCROP_CFGL   PYRAMID_HCROP_CFGL;
	U_GSBLUR_COEF_01       GSBLUR_COEF_01;
	U_GSBLUR_COEF_23       GSBLUR_COEF_23;
	U_THRESHOLD_CFG        THRESHOLD_CFG;
	U_NMS_WIN_CFG          NMS_WIN_CFG;
	U_BLOCK_NUM_CFG        BLOCK_NUM_CFG;
	U_BLOCK_SIZE_CFG       BLOCK_SIZE_CFG;
	U_OCTREE_CFG           OCTREE_CFG;
	U_INC_LUT_CFG          INC_LUT_CFG;
	U_UNDISTORT_CX         UNDISTORT_CX;
	U_UNDISTORT_CY         UNDISTORT_CY;
	U_UNDISTORT_FX         UNDISTORT_FX;
	U_UNDISTORT_FY         UNDISTORT_FY;
	U_UNDISTORT_INVFX      UNDISTORT_INVFX;
	U_UNDISTORT_INVFY      UNDISTORT_INVFY;
	U_UNDISTORT_K1         UNDISTORT_K1;
	U_UNDISTORT_K2         UNDISTORT_K2;
	U_UNDISTORT_K3         UNDISTORT_K3;
	U_UNDISTORT_P1         UNDISTORT_P1;
	U_UNDISTORT_P2         UNDISTORT_P2;
	U_CVDR_CFG_0           CVDR_CFG_0;
	U_CVDR_CFG_1           CVDR_CFG_1;
	U_DEBUG_0              DEBUG_0;
	U_DEBUG_1              DEBUG_1;
	U_SCORE_HIST           SCORE_HIST[63];
	U_BRIEF_PATTERN        BRIEF_PATTERN[256];
	U_SCORE_THESHOLD       SCORE_THESHOLD[94];
	U_KPT_NUMBER           KPT_NUMBER[94];

} S_SLAM_REGS_TYPE;

/* Declare the struct pointor of the module SLAM */
extern S_SLAM_REGS_TYPE *gopSLAMAllReg;


#endif /* __SLAM_DRV_PRIV_CS_H__ */
