#ifndef __M3_DDR_MAP_H__
#define __M3_DDR_MAP_H__ 
#include <global_ddr_map.h>
#include <soc_lpmcu_baseaddr_interface.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define LPMCU_RAM_PHY_SIZE (256*1024)
#define LPMCU_RAM_SIZE (232*1024)
#define OCBC_RAM_SIZE (0)
#define UCE_RAM_SIZE (32*1024)
#define M3_DDR_MEM_BASE_ADDR (HISI_RESERVED_LPMX_CORE_PHYMEM_BASE)
#define M3_DDR_MEM_BASE_SIZE (HISI_RESERVED_LPMX_CORE_PHYMEM_SIZE)
#define LPMCU_SECURE_DDR_BASE_ADDR (HISI_RESERVED_LPMCU_PHYMEM_BASE)
#define LPMCU_SECURE_DDR_BASE_SIZE (HISI_RESERVED_LPMCU_PHYMEM_SIZE)
#define TELE_MNTN_AREA_ADDR (M3_DDR_MEM_BASE_ADDR)
#define TELE_MNTN_AREA_SIZE (M3_DDR_MEM_BASE_SIZE)
#define LPMCU_FIRMWARE_PACK_BACKUP_ADDR (LPMCU_SECURE_DDR_BASE_ADDR)
#define LPMCU_FIRMWARE_PACK_BACKUP_SIZE (360*1024)
 #define LPMCU_FIRMWARE_BACKUP_ADDR (LPMCU_FIRMWARE_PACK_BACKUP_ADDR)
 #define LPMCU_FIRMWARE_BACKUP_SIZE (LPMCU_RAM_SIZE)
 #define LPM3_MAGIC_ADDR (LPMCU_FIRMWARE_BACKUP_ADDR + 0x044)
 #define LPMCU_DDR_RUNSPACE_ADDR (LPMCU_FIRMWARE_BACKUP_ADDR + LPMCU_FIRMWARE_BACKUP_SIZE)
 #define LPMCU_DDR_RUNSPACE_SIZE (0x10000)
 #define M3_DDR_SYS_MEM_ADDR LPMCU_DDR_RUNSPACE_ADDR
 #define M3_DDR_SYS_MEM_USED_SIZE LPMCU_DDR_RUNSPACE_SIZE
 #define M3_AO_TCP_ADDR (LPMCU_DDR_RUNSPACE_ADDR + LPMCU_DDR_RUNSPACE_SIZE)
 #define M3_AO_TCP_ADDR_SIZE (0x1000)
 #define LPMCU_FIRMWARE_PACK_BACKUP_USED_SIZE (M3_AO_TCP_ADDR + M3_AO_TCP_ADDR_SIZE - LPMCU_FIRMWARE_PACK_BACKUP_ADDR)
#if (LPMCU_FIRMWARE_PACK_BACKUP_USED_SIZE > (LPMCU_FIRMWARE_PACK_BACKUP_SIZE))
#error m3_system_memory_size(LPMCU_FIRMWARE_PACK_BACKUP_SIZE) used beyond (LPMCU_FIRMWARE_PACK_BACKUP_SIZE)
#endif
#define M3_DDR_SHARE_MEM_DDR2FASTBOOT_BASE (LPMCU_FIRMWARE_PACK_BACKUP_ADDR + LPMCU_FIRMWARE_PACK_BACKUP_SIZE)
#define M3_DDR_SHARE_MEM_DDR2FASTBOOT_SIZE (0x4000)
#define PMU_DUMP_ADDR (M3_DDR_SHARE_MEM_DDR2FASTBOOT_BASE + M3_DDR_SHARE_MEM_DDR2FASTBOOT_SIZE)
#define PMU_DUMP_SIZE (1024)
#define PER_CHIP_AVS_FLAG_ADDR (PMU_DUMP_ADDR + PMU_DUMP_SIZE)
#define PER_CHIP_AVS_FLAG_SIZE (1024)
#define M3_DDR_RESERVED_ADDR (PER_CHIP_AVS_FLAG_ADDR + PER_CHIP_AVS_FLAG_SIZE)
#define M3_DDR_RESERVED_SIZE (0xF800)
#define M3_DDR_SHARE_MEM_ADDR (M3_DDR_RESERVED_ADDR + M3_DDR_RESERVED_SIZE)
#define M3_DDR_SHARE_MEM_SIZE (0x3A000)
 #define M3_DDR_INIT_UCE_MNTN_BASE (M3_DDR_SHARE_MEM_ADDR)
 #define M3_DDR_INIT_UCE_MNTN_SIZE (0x24400)
 #define M3_DDR_INIT_XLOADER_TO_FASTBOOT_BASE (M3_DDR_INIT_UCE_MNTN_BASE + M3_DDR_INIT_UCE_MNTN_SIZE)
 #define M3_DDR_INIT_XLOADER_TO_FASTBOOT_SIZE (0x8000)
 #define M3_DDR_STORAGE_MEM_DDR_MPU_ADDR (M3_DDR_INIT_XLOADER_TO_FASTBOOT_BASE + M3_DDR_INIT_XLOADER_TO_FASTBOOT_SIZE)
 #define M3_DDR_STORAGE_MEM_DDR_MPU_SIZE (0x2400)
 #define M3_DDR_STORAGE_MEM_DDR_SECURITY_ADDR (M3_DDR_STORAGE_MEM_DDR_MPU_ADDR + M3_DDR_STORAGE_MEM_DDR_MPU_SIZE)
 #define M3_DDR_STORAGE_MEM_DDR_SECURITY_SIZE (0x1800)
 #define UCE_FIRMWARE_BACKUP_ADDR (M3_DDR_STORAGE_MEM_DDR_SECURITY_ADDR + M3_DDR_STORAGE_MEM_DDR_SECURITY_SIZE)
 #define UCE_FIRMWARE_BACKUP_SIZE (UCE_RAM_SIZE)
 #define M3_DDR_SHARE_MEM_AGE_ADDR (UCE_FIRMWARE_BACKUP_ADDR + UCE_FIRMWARE_BACKUP_SIZE)
 #define M3_DDR_SHARE_MEM_AGE_SIZE (0x400)
 #define M3_DDR_SHARE_MEM_RESERVED_ADDR (M3_DDR_SHARE_MEM_AGE_ADDR + M3_DDR_SHARE_MEM_AGE_SIZE)
#if (M3_DDR_SHARE_MEM_RESERVED_ADDR > (M3_DDR_SHARE_MEM_ADDR+M3_DDR_SHARE_MEM_SIZE))
#error m3_share_memory_addr(M3_DDR_SHARE_MEM_RESERVED_ADDR) used beyond (M3_DDR_SHARE_MEM_ADDR+M3_DDR_SHARE_MEM_SIZE)
#endif
#define M3_DDR_MEM_RESERVED_ADDR (M3_DDR_SHARE_MEM_ADDR + M3_DDR_SHARE_MEM_SIZE)
#if (M3_DDR_MEM_RESERVED_ADDR > (LPMCU_SECURE_DDR_BASE_ADDR+LPMCU_SECURE_DDR_BASE_SIZE))
#error m3_memory_addr(M3_DDR_MEM_RESERVED_ADDR) used beyond (LPMCU_SECURE_DDR_BASE_ADDR+LPMCU_SECURE_DDR_BASE_SIZE)
#endif
#define DDR_WIN_MASK (0xF0000000)
#define DDR_PHY_TO_WIN(phy_addr) ((phy_addr) & DDR_WIN_MASK)
#define DDR_VIRT_TO_PHY(virt_addr,win_addr) (((virt_addr) & 0x0FFFFFFF) | (win_addr))
#define DDR_PHY_TO_VIRT(phy_addr) (((phy_addr) & 0x0FFFFFFF) | 0x10000000)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
