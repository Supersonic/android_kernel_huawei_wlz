#ifndef __HISI_LB_PMU_H__
#define __HISI_LB_PMU_H__

#include <linux/platform_device.h>
#include "hisi_lb_priv.h"

typedef void (*cfg_func)(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);

#ifdef CONFIG_HISI_LB_PMU
int lb_pmu_init(struct platform_device *pdev, struct lb_device *lbd);
void pmu_SLC_NO_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_BP2_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_BP5_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_BP6_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_BP7_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3);
void pmu_SLC_BP8_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_BP9_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_BP10_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3);
void pmu_SLC_BP11_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3);
void pmu_SLC_BP12_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3);
void pmu_SLC_BP14_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_BP15_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_TCP0_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3);
void pmu_SLC_TCP1_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3);
void pmu_SLC_TCP2_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_TCP3_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_TCP4_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_TCP5_CFG(u32 r_reg0, u32 r_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_TCP6_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_TCP7_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_TCP8_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3);
void pmu_SLC_TCP9_CFG(u32 r_reg0, u32 r_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_TCP10_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_DCP03_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_DCP6_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_DCP7_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_DCP8_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3);
void pmu_SLC_DCP9_CFG_0(u32 reg, u32 reg1, u32 reg2, u32 reg3);
void pmu_map_event(int cnt, int evt, int md);
void pmu_unmap_event(int cnt);
#else
static inline int lb_pmu_init(struct platform_device *pdev, struct lb_device *lbd){return 0;}
static inline void pmu_SLC_NO_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3) {}
static inline void pmu_SLC_BP2_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP5_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP6_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP7_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP8_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_BP9_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP10_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP11_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP12_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3){}
static inline void pmu_SLC_BP14_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_BP15_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_TCP0_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3){}
static inline void pmu_SLC_TCP1_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 f_reg3){}
static inline void pmu_SLC_TCP2_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_TCP3_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_TCP4_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_TCP5_CFG(u32 r_reg0, u32 r_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_TCP6_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_TCP7_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_TCP8_CFG(u32 r_reg0, u32 r_reg1, u32 r_reg2, u32 r_reg3){}
static inline void pmu_SLC_TCP9_CFG(u32 r_reg0, u32 r_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_TCP10_CFG(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_DCP03_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_DCP6_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_DCP7_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_DCP8_CFG_0(u32 r_reg0, u32 f_reg1, u32 f_reg2, u32 f_reg3){}
static inline void pmu_SLC_DCP9_CFG_0(u32 reg, u32 reg1, u32 reg2, u32 reg3){}
static inline void pmu_map_event(int cnt, int evt, int md) {}
static inline void pmu_unmap_event(int cnt) {}
#endif

#endif
