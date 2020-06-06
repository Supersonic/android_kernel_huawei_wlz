#ifndef __CVDR__CS_H__
#define __CVDR__CS_H__

#define WR_CMDLST  (0)
#define CPE_WR0    (1)
#define CPE_WR1    (2)
#define CPE_WR2    (3)
#define WR0_SLAM   (4)
#define WR1_SLAM   (5)
#define WR2_SLAM   (6)
#define VP_WR_MAX  (7)

#define RD_CMDLST   (0)
#define CPE_RD0     (1)
#define CPE_RD1     (2)
#define CPE_RD2     (3)
#define CPE_RD3     (4)
#define CPE_RD4     (5)
#define CPE_RD5     (6)
#define RD0_SLAM    (7)
#define RD_RDR_DESC (8)
#define RD_CMP_REF  (9)
#define VP_RD_MAX   (10)

#define WR_RDR_DESC (0)
#define NR_WR_MAX   (1)

#define RD_CMP_CUR (0)
#define NR_RD_MAX  (1)

extern unsigned int g_cvdr_vp_wr_id[VP_WR_MAX];
extern unsigned int g_cvdr_vp_rd_id[VP_RD_MAX];
extern unsigned int g_cvdr_nr_wr_id[NR_WR_MAX];
extern unsigned int g_cvdr_nr_rd_id[NR_RD_MAX];

int ippdev_lock(void);
int ippdev_unlock(void);

#define get_cvdr_vp_wr_port_num(x) g_cvdr_vp_wr_id[x]
#define get_cvdr_vp_rd_port_num(x) g_cvdr_vp_rd_id[x]
#define get_cvdr_nr_wr_port_num(x) g_cvdr_nr_wr_id[x]
#define get_cvdr_nr_rd_port_num(x) g_cvdr_nr_rd_id[x]
#endif/*__CVDR__CS_H__ */
