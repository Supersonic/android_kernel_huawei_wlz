#ifndef __SOC_MEMMAP_COMM_H__
#define __SOC_MEMMAP_COMM_H__ 
#ifdef __cplusplus
extern "C" {
#endif
#include "product_config.h"
#ifndef SZ_20K
#define SZ_20K (0x00005000)
#endif
#ifndef SZ_80K
#define SZ_80K (0x00014000)
#endif
#ifndef SZ_96K
#define SZ_96K (0x00018000)
#endif
#ifndef SZ_3M
#define SZ_3M (0x00300000)
#endif
#ifdef __KERNEL__
#include <asm-generic/sizes.h>
#else
#ifndef SZ_512
#define SZ_512 (0x00000200)
#endif
#ifndef SZ_1K
#define SZ_1K (0x00000400)
#endif
#ifndef SZ_2K
#define SZ_2K (0x00000800)
#endif
#ifndef SZ_4K
#define SZ_4K (0x00001000)
#endif
#ifndef SZ_8K
#define SZ_8K (0x00002000)
#endif
#ifndef SZ_16K
#define SZ_16K (0x00004000)
#endif
#ifndef SZ_32K
#define SZ_32K (0x00008000)
#endif
#ifndef SZ_64K
#define SZ_64K (0x00010000)
#endif
#ifndef SZ_128K
#define SZ_128K (0x00020000)
#endif
#ifndef SZ_256K
#define SZ_256K (0x00040000)
#endif
#ifndef SZ_384K
#define SZ_384K (0x00060000)
#endif
#ifndef SZ_512K
#define SZ_512K (0x00080000)
#endif
#ifndef SZ_704K
#define SZ_704K (0x000B0000)
#endif
#ifndef SZ_1M
#define SZ_1M (0x00100000)
#endif
#ifndef SZ_2M
#define SZ_2M (0x00200000)
#endif
#ifndef SZ_4M
#define SZ_4M (0x00400000)
#endif
#ifndef SZ_8M
#define SZ_8M (0x00800000)
#endif
#ifndef SZ_16M
#define SZ_16M (0x01000000)
#endif
#ifndef SZ_32M
#define SZ_32M (0x02000000)
#endif
#ifndef SZ_64M
#define SZ_64M (0x04000000)
#endif
#ifndef SZ_128M
#define SZ_128M (0x08000000)
#endif
#ifndef SZ_256M
#define SZ_256M (0x10000000)
#endif
#ifndef SZ_512M
#define SZ_512M (0x20000000)
#endif
#endif
#define HI_SOCP_REGBASE_ADDR (0XFA0A0000)
#define HI_SOCP_REG_SIZE (SZ_4K)
#define HI_I2C1_REGBASE_ADDR (0XFDF0AFFF)
#define HI_I2C1_REG_SIZE (SZ_4K)
#define HI_I2C0_REGBASE_ADDR (0XFDF09FFF)
#define HI_I2C0_REG_SIZE (SZ_4K)
#define HI_I2C3_REGBASE_ADDR (0XFDF0C000)
#define HI_I2C3_REG_SIZE (SZ_4K)
#define HI_I2C4_REGBASE_ADDR (0XFDF0D000)
#define HI_I2C4_REG_SIZE (SZ_4K)
#define HI_SPI_MST0_REGBASE_ADDR (0XFDF07FFF)
#define HI_SPI_MST0_REG_SIZE (SZ_4K)
#define HI_SPI_MST1_REGBASE_ADDR (0XFDF08000)
#define HI_SPI_MST1_REG_SIZE (SZ_4K)
#define HI_UART5_REGBASE_ADDR (0xFDF05000)
#define HI_UART5_REG_SIZE (SZ_4K)
#define HI_UART4_REGBASE_ADDR (0XFDF01000)
#define HI_UART4_REG_SIZE 
#define HI_UART3_REGBASE_ADDR (0XFDF04FFF)
#define HI_UART3_REG_SIZE (SZ_4K)
#define HI_UART2_REGBASE_ADDR (0XFDF03000)
#define HI_UART2_REG_SIZE (SZ_4K)
#define HI_APB_CFG_UART0_BASE_ADDR (0XFDF02000)
#define HI_APB_CFG_UART0_SIZE (SZ_4K)
#define HI_PWM0_REGBASE_ADDR (0XE8A04000)
#define HI_PWM0_REG_SIZE (SZ_4K)
#define HI_HKADCSSI_REGBASE_ADDR (0XE82B8000)
#define HI_HKADCSSI_REG_SIZE (SZ_4K)
#define HI_APP_GIC_BASE_ADDR (0xE82B0000)
#define HI_APP_GIC_SIZE (SZ_32K)
#define HI_AP_SYSCTRL_BASE_ADDR (0xFA89B000)
#define HI_AP_SYSCTRL_REG_SIZE (SZ_4K)
#define HI_AP_SCTRL_BASE_ADDR HI_AP_SYSCTRL_BASE_ADDR
#define HI_AP_SYS_CNT_BASE_ADDR (0xFA896000)
#define HI_AP_SYS_CNT_SIZE (SZ_8K)
#define HI_LP_PERI_CRG_REG_ADDR (0xFFF05000)
#define HI_LP_PERI_CRG_REG_SIZE (SZ_4K)
#define HI_NOC_PMC_REG_ADDR (0xFFF31000)
#define HI_NOC_PMC_REG_SIZE (SZ_4K)
#define HI_IPF_REGBASE_ADDR (0xFF031000)
#define HI_IPF_REG_SIZE (SZ_4K)
#define HI_PSAM_REGBASE_ADDR (0xFF040000)
#define HI_PSAM_REG_SIZE (SZ_64K)
#define HI_MODEM_BASE_ADDR (0xE0000000)
#define HI_MODEM_SIZE (SZ_128M)
#define HI_MDMA9_L2_REGBASE_ADDR (0xE0000000)
#define HI_MDMA9_L2_REG_SIZE (SZ_1M)
#define HI_MDM_GIC_BASE_ADDR (0xE0100000)
#define HI_MDM_GIC_SIZE (SZ_8K)
#define HI_SYSCTRL_BASE_ADDR (0xE0200000)
#define HI_SYSCTRL_REG_SIZE (SZ_4K)
#define HI_SYSCRG_BASE_ADDR HI_SYSCTRL_BASE_ADDR
#define HI_SYSSC_BASE_ADDR HI_SYSCTRL_BASE_ADDR
#define HI_PWRCTRL_BASE_ADDR HI_SYSCTRL_BASE_ADDR
#define HI_SYSSC_AO_BASE_ADDR HI_SYSCTRL_BASE_ADDR
#define HI_SYSSC_PD_BASE_ADDR HI_SYSCTRL_BASE_ADDR
#define HI_MODEM_SC_BASE_ADDR HI_SYSCTRL_BASE_ADDR
#define HI_WDT_BASE_ADDR (0xE0201000)
#define HI_WDT_REG_SIZE (SZ_4K)
#define HI_WDT1_BASE_ADDR (0xE0211000)
#define HI_WDT1_REG_SIZE (SZ_4K)
#define HI_UART0_REGBASE_ADDR (0xE0203000)
#define HI_UART0_REG_SIZE (SZ_4K)
#define HI_UART1_REGBASE_ADDR (0xE0205000)
#define HI_UART1_REG_SIZE 
#define HI_EDMA_REGBASE_ADDR (0xE0204000)
#define HI_EDMA_REG_SIZE (SZ_4K)
#define HI_EDMA1_REGBASE_ADDR (0xE0210000)
#define HI_EDMA1_REG_SIZE (SZ_4K)
#define HI_CICOM0_REGBASE_ADDR (0xE0440000)
#define HI_CICOM0_REG_SIZE (SZ_4K)
#define HI_CICOM1_REGBASE_ADDR (0xE0443000)
#define HI_CICOM1_REG_SIZE (SZ_4K)
#define HI_IPCM_REGBASE_ADDR (0xE0441000)
#define HI_IPCM_REG_SIZE (SZ_4K)
#define HI_HDLC_REGBASE_ADDR (0xE0442000)
#define HI_HDLC_REG_SIZE (SZ_4K)
#define HI_CIPHER_BASE_ADDR (0xE0452000)
#define HI_CIPHER_REG_SIZE (SZ_4K)
#define HI_UPACC_BASE_ADDR (0xE0453000)
#define HI_UPACC_REG_SIZE (SZ_4K)
#define HI_BBE16VIC_REGBASE_ADDR (0xE0456000)
#define HI_BBE16VIC_REG_SIZE (SZ_4K)
#define HI_CDSPVIC_REGBASE_ADDR (0xE0457000)
#define HI_CDSPVIC_REG_SIZE (SZ_4K)
#define HI_IPC_S_REGBASE_ADDR (0xE896A000)
#define HI_IPC_S_REG_SIZE (SZ_4K)
#define HI_AMON_SOC_REGBASE_ADDR (0xE0458000)
#define HI_AMON_SOC_REG_SIZE (SZ_16K)
#define HI_AMON_CPUFAST_REGBASE_ADDR (0xE0458000)
#define HI_AMON_CPUFAST_REG_SIZE (SZ_16K)
#define HI_SRAM_MEM_BASE_ADDR (DRV_SRAM_ADDR)
#define HI_SRAM_MEM_SIZE (DRV_SRAM_SIZE)
#define SRAM_SIZE_GU_MAC_HEADER (0)
#define HI_DSP_SUBSYSTEM_BASE_ADDR (0xE1000000)
#define HI_DSP_SUBSYSTEM_SIZE (SZ_16M)
#define HI_BBP_UL_BASE_ADDR (0xE2000000)
#define HI_BBP_UL_SIZE (SZ_4M)
#define HI_BBE16DMEM_BASE_ADDR (0xE2700000)
#define HI_BBE16DMEM_SIZE (0x000e0000)
#define HI_BBE16IMEM_BASE_ADDR (0xE2800000)
#define HI_BBE16IMEM_SIZE (0x000d0000)
#define HI_CBBE16DMEM_BASE_ADDR (0xE2900000)
#define HI_CBBE16DMEM_SIZE (SZ_512K)
#define HI_CBBE16IMEM_BASE_ADDR (0xE2980000)
#define HI_CBBE16IMEM_SIZE (SZ_512K)
#define HI_SECRAM_BASE_ADDR (0xE8000000)
#define HI_SECRAM_SIZE (SZ_128K)
#define HI_PMUSSI0_REGBASE_ADDR (0xFFF34000)
#define HI_PMUSSI0_REG_SIZE (SZ_4K)
#define HI_TSENSOR_REGBASE_ADDR (0xFFF30000)
#define HI_TSENSOR_REG_SIZE (SZ_4K)
#define HI_EFUSE_REGBASE_ADDR (0xFFF03000)
#define HI_EFUSE_REG_SIZE (SZ_4K)
#define HI_SCI1_REGBASE_ADDR (0xFFF07000)
#define HI_SCI1_REG_SIZE (SZ_4K)
#define HI_SCI0_REGBASE_ADDR (0xFFF06000)
#define HI_SCI0_REG_SIZE (SZ_4K)
#define HI_RTC1_BASE_ADDR (0xFFF05000)
#define HI_RTC1_SIZE (SZ_4K)
#define HI_RTC0_BASE_ADDR (0xFFF04000)
#define HI_RTC0_SIZE (SZ_4K)
#define HI_CS_SYS_REGBASE_ADDR (0xFFF00000)
#define HI_CS_SYS_REG_SIZE (SZ_1M)
#define HI_NANDC_REGBASE_ADDR (0xFFFA0000)
#define HI_NANDC_REG_SIZE (SZ_128K)
#define HI_NAND_MEM_BUFFER_ADDR (0xFFF80000)
#define HI_NAND_MEM_BUFFER_SIZE (SZ_128K)
#define HI_DDR_BASE_ADDR (0x30000000)
#define HI_DDR_SIZE (SZ_128M)
#define HI_BOOTROM_REGBASE_ADDR (0xFFF60000)
#define HI_BOOTROM_REG_SIZE (SZ_64K)
#define HI_M3TCM1_MEM_ADDR (0xFFF50000)
#define HI_M3TCM1_MEM_SIZE (SZ_32K)
#define HI_M3TCM0_MEM_ADDR (0xFFF40000)
#define HI_M3TCM0_MEM_SIZE (SZ_64K)
#define HI_ZSP_AHB_REG_BASE_ADDR 0xFFFFFFFF
#define HI_ZSP_DTCM_REG_BASE_ADDR 0xFFFFFFFF
#define HI_BBP_SRC_BASE_ADDR (0xE1000000)
#define HI_BBP_SRC_SIZE SZ_1M
#define HI_BBP_DMA_BASE_ADDR (0xE1FCC000)
#define HI_BBP_DMA_SIZE SZ_1M
#define HI_BBP_DBG_BASE_ADDR (0xE1FC4000)
#define HI_BBP_DBG_SIZE SZ_1M
#define HI_BBP_INT_BASE_ADDR (0xE1700000)
#define HI_BBP_INT_SIZE SZ_4K
#define HI_BBP_STU_BASE_ADDR (0xE170e000)
#define HI_BBP_STU_SIZE SZ_4K
#define HI_BBP_TSTU_BASE_ADDR (0xE1d00000)
#define HI_BBP_TSTU_SIZE SZ_8K
#define HI_GBBP_REG_BASE_ADDR (0xE1800000)
#define HI_GBBP_REG_SIZE SZ_512K
#define HI_GBBP1_REG_BASE_ADDR (0xE1880000)
#define HI_GBBP1_REG_SIZE SZ_512K
#define HI_WBBP_REG_BASE_ADDR (0xE1900000)
#define HI_WBBP_REG_REG_SIZE SZ_1M
#define HI_BBP_CDMA_BASE_ADDR (0xE12E0000)
#define HI_BBP_GSDR_BASE_ADDR (0xE1840000)
#define HI_CTU_BASE_ADDR (0xE1f80000)
#define HI_CTU_SIZE SZ_32K
#if defined(BSP_CONFIG_BOARD_SFT)
#define HI_BBP_LTEDRX_BASE_ADDR (0xE1FB0000)
#define HI_BBP_TDSDRX_BASE_ADDR (0xE1FB1400)
#define HI_BBP_COMM_ON_BASE_ADDR (0xE1FB0000)
#define HI_GBBP_DRX_REG_BASE_ADDR (0xE1FB0800)
#define HI_GBBP1_DRX_REG_BASE_ADDR (0xE1FB0C00)
#define HI_WBBP_DRX_REG_BASE_ADDR (0xE1FB0000)
#define HI_BBP_CDMA_ON_BASE_ADDR (0xE1FB0000)
#define HI_BBP_GLB_ON_BASE_ADDR (0xE1FB0000)
#else
#define HI_BBP_LTEDRX_BASE_ADDR (0xFFF12000)
#define HI_BBP_TDSDRX_BASE_ADDR (0xFFF13400)
#define HI_BBP_COMM_ON_BASE_ADDR (0xFFF12000)
#define HI_GBBP_DRX_REG_BASE_ADDR (0xFFF12800)
#define HI_GBBP1_DRX_REG_BASE_ADDR (0xFFF12C00)
#define HI_WBBP_DRX_REG_BASE_ADDR (0xFFF12000)
#define HI_BBP_CDMA_ON_BASE_ADDR (0xFFF12000)
#define HI_BBP_GLB_ON_BASE_ADDR (0xFFF12000)
#endif
#define HI_BBP_CTU_BASE_ADDR HI_CTU_BASE_ADDR
#define HI_BBPMASTER_REG_BASE_ADDR HI_WBBP_REG_BASE_ADDR
#define HI_CORESIGHT_PTM1_BASE_ADDR 0xEC0BC000
#define HI_CORESIGHT_PTM1_SIZE SZ_4K
#if defined(BSP_CONFIG_BOARD_SFT)&&defined(BSP_CONFIG_HI3650)
#define HI_BBP_SYSTIME_BASE_ADDR (0xFFF08000)
#else
#define HI_BBP_SYSTIME_BASE_ADDR (HI_AP_SYSCTRL_BASE_ADDR)
#endif
#define HI_BBP_SYSTIME_SIZE SZ_8K
#define HI_XG2RAM_HARQ_BASE_ADDR (0xEA000000)
#ifdef __cplusplus
}
#endif
#endif
