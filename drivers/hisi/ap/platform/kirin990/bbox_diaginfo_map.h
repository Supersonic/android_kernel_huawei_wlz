#ifndef __DIAGINFO_MAP
#define __DIAGINFO_MAP(x,y,z,k) 
#endif
#ifndef __DIAGINFO_MODULE_MAP
#define __DIAGINFO_MODULE_MAP(x) 
#endif
__DIAGINFO_MAP(L3_ECC_1BIT_ERROR, SoC_AP, Warning, HW)
__DIAGINFO_MAP(L3_ECC_2BIT_ERROR, SoC_AP, Warning, HW)
__DIAGINFO_MAP(L3_ECC_BUS_ERROR, SoC_AP, Warning, SW)
__DIAGINFO_MAP(CPU_UP_FAIL, SoC_AP, Critical, HW)
__DIAGINFO_MAP(CPU_PANIC_INFO, SoC_AP, Critical, HW)
__DIAGINFO_MAP(NOC_FAULT_INFO, SoC_AP, Critical, HW)
__DIAGINFO_MAP(LPM3_DDR_FAIl, DDR, Critical, HW)
__DIAGINFO_MAP(SWING_DMD_FDUL_PW_ON, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_FDUL_PW_OFF, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_HWTS_PW_ON, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_HWTS_PW_OFF, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_AIC_PW_ON, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_AIC_PW_OFF, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_HWI_CREATE, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_HWI_DELETE, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_CAM_PW_ON, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_CAM_PW_OFF, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_CAM_IR_PW, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_CAM_TIMEOUT, FDUL, INFO, HW)
__DIAGINFO_MAP(SWING_DMD_BRIEF_CASE, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_INT1_AIC_ERR, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_INT1_TIMEOUT, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_INT1_BUS_ERR, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_INT7_BUS_ERR, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_FDUL_NOC, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_MODEL_LOAD, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_MODEL_UNLOAD, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_MODEL_RUN, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_AIC_KICK, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_CAMERA_INIT, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_CAMERA_RELEASE, FDUL, INFO, SW)
__DIAGINFO_MAP(SWING_DMD_SLEEP_FUSION, FDUL, INFO, SW)
__DIAGINFO_MODULE_MAP(SoC_AP)
__DIAGINFO_MODULE_MAP(DDR)
__DIAGINFO_MODULE_MAP(FDUL)
