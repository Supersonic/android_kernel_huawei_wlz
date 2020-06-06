//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  vbk_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-07-19
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-07-19 Create file
//******************************************************************************

#ifndef __VBK_DRV_PRIV_CS_H__
#define __VBK_DRV_PRIV_CS_H__

/* Define the union U_VBK_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    op_mode               : 1   ; /* [0]  */
		unsigned int    y_ds16_gauss_en       : 1   ; /* [1]  */
		unsigned int    uv_ds16_gauss_en      : 1   ; /* [2]  */
		unsigned int    sigma_gauss_en        : 1   ; /* [3]  */
		unsigned int    sigma2alpha_en        : 1   ; /* [4]  */
		unsigned int    in_width_ds4          : 10  ; /* [14..5]  */
		unsigned int    reserved_0            : 1   ; /* [15]  */
		unsigned int    in_height_ds4         : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_CFG;

/* Define the union U_VBK_Y_DS16_GAUSS_COEFF0_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    y_ds16_g00            : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    y_ds16_g01            : 5   ; /* [12..8]  */
		unsigned int    reserved_1            : 3   ; /* [15..13]  */
		unsigned int    y_ds16_g02            : 5   ; /* [20..16]  */
		unsigned int    reserved_2            : 3   ; /* [23..21]  */
		unsigned int    y_ds16_g03            : 5   ; /* [28..24]  */
		unsigned int    reserved_3            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_Y_DS16_GAUSS_COEFF0_CFG;

/* Define the union U_VBK_Y_DS16_GAUSS_COEFF1_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    y_ds16_g04            : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    y_ds16_gauss_inv      : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_Y_DS16_GAUSS_COEFF1_CFG;

/* Define the union U_VBK_UV_DS16_GAUSS_COEFF1_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 8   ; /* [7..0]  */
		unsigned int    uv_ds16_gauss_inv     : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_UV_DS16_GAUSS_COEFF1_CFG;

/* Define the union U_VBK_UV_DS16_GAUSS_COEFF0_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    uv_ds16_g00           : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    uv_ds16_g01           : 5   ; /* [12..8]  */
		unsigned int    reserved_1            : 3   ; /* [15..13]  */
		unsigned int    uv_ds16_g02           : 5   ; /* [20..16]  */
		unsigned int    reserved_2            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_UV_DS16_GAUSS_COEFF0_CFG;

/* Define the union U_VBK_SIGMA_GAUSS_COEFF0_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    sigma_g00             : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    sigma_g01             : 5   ; /* [12..8]  */
		unsigned int    reserved_1            : 3   ; /* [15..13]  */
		unsigned int    sigma_g02             : 5   ; /* [20..16]  */
		unsigned int    reserved_2            : 3   ; /* [23..21]  */
		unsigned int    sigma_g03             : 5   ; /* [28..24]  */
		unsigned int    reserved_3            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_SIGMA_GAUSS_COEFF0_CFG;

/* Define the union U_VBK_SIGMA_GAUSS_COEFF1_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    sigma_g04             : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    sigma_gauss_inv       : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_SIGMA_GAUSS_COEFF1_CFG;

/* Define the union U_VBK_SIGMA2ALPHA_LUT */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    sigma2alpha           : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_SIGMA2ALPHA_LUT;

/* Define the union U_VBK_ADDNOISE_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    addnoise_th           : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_ADDNOISE_CFG;

/* Define the union U_VBK_FOREMASK_CFG */
typedef union {
	/* Define the struct bits */
	struct {
		unsigned int    foremask_dilation_radius : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    foremask_weighted_filter_en : 1   ; /* [4]  */
		unsigned int    reserved_1            : 3   ; /* [7..5]  */
		unsigned int    foremask_th           : 8   ; /* [15..8]  */
		unsigned int    foremask_coeff        : 8   ; /* [23..16]  */
		unsigned int    reserved_2            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_FOREMASK_CFG;

/* Define the union U_VBK_TOP_DEBUG */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int vbk_top_debug          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_TOP_DEBUG;
/* Define the union U_VBK_Y_DS16_GAUSS_DEBUG */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int vbk_y_ds16_debug       : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_Y_DS16_GAUSS_DEBUG;
/* Define the union U_VBK_UV_DS16_GAUSS_DEBUG */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int vbk_uv_ds16_debug      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_UV_DS16_GAUSS_DEBUG;
/* Define the union U_VBK_SIGMA_GAUSS_DEBUG */
typedef union {
	/* Define the struct bits  */
	struct {
		unsigned int vbk_sigma_debug        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VBK_SIGMA_GAUSS_DEBUG;
//==============================================================================
/* Define the global struct */
typedef struct {
	U_VBK_CFG              VBK_CFG;
	U_VBK_Y_DS16_GAUSS_COEFF0_CFG VBK_Y_DS16_GAUSS_COEFF0_CFG;
	U_VBK_Y_DS16_GAUSS_COEFF1_CFG VBK_Y_DS16_GAUSS_COEFF1_CFG;
	U_VBK_UV_DS16_GAUSS_COEFF1_CFG VBK_UV_DS16_GAUSS_COEFF1_CFG;
	U_VBK_UV_DS16_GAUSS_COEFF0_CFG VBK_UV_DS16_GAUSS_COEFF0_CFG;
	U_VBK_SIGMA_GAUSS_COEFF0_CFG VBK_SIGMA_GAUSS_COEFF0_CFG;
	U_VBK_SIGMA_GAUSS_COEFF1_CFG VBK_SIGMA_GAUSS_COEFF1_CFG;
	U_VBK_SIGMA2ALPHA_LUT  VBK_SIGMA2ALPHA_LUT[256];
	U_VBK_ADDNOISE_CFG     VBK_ADDNOISE_CFG;
	U_VBK_FOREMASK_CFG     VBK_FOREMASK_CFG;
	U_VBK_TOP_DEBUG        VBK_TOP_DEBUG;
	U_VBK_Y_DS16_GAUSS_DEBUG VBK_Y_DS16_GAUSS_DEBUG;
	U_VBK_UV_DS16_GAUSS_DEBUG VBK_UV_DS16_GAUSS_DEBUG;
	U_VBK_SIGMA_GAUSS_DEBUG VBK_SIGMA_GAUSS_DEBUG;

} S_VBK_REGS_TYPE;

/* Declare the struct pointor of the module VBK */
extern S_VBK_REGS_TYPE *gopVBKAllReg;


#endif /* __VBK_DRV_PRIV_CS_H__ */
