#ifndef __SOC_IOMCU_BASEADDR_INTERFACE_H__
#define __SOC_IOMCU_BASEADDR_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_IOMCU_PPB_ROM_table_BASE_ADDR (0xE00FF000)
#define SOC_IOMCU_Processor_ROM_table_BASE_ADDR (0xE00FE000)
#define SOC_IOMCU_Funnel_BASE_ADDR (0xE0043000)
#define SOC_IOMCU_CTI_BASE_ADDR (0xE0042000)
#define SOC_IOMCU_ETM_BASE_ADDR (0xE0041000)
#define SOC_IOMCU_SCS_BASE_ADDR (0xE000E000)
#define SOC_IOMCU_FPB_BASE_ADDR (0xE0002000)
#define SOC_IOMCU_DWT_BASE_ADDR (0xE0001000)
#define SOC_IOMCU_ITM_BASE_ADDR (0xE0000000)
#define SOC_IOMCU_DRAM_BASE_ADDR (0x60000000)
#define SOC_IOMCU_BOOTROM_BASE_ADDR (0x5FFE0000)
#define SOC_IOMCU_LPMCU_RAM_BASE_ADDR (0x5FF70000)
#define SOC_IOMCU_LP_RAM_BASE_ADDR (0x5FF50000)
#define SOC_IOMCU_LP_CONFIG_BASE_ADDR (0x5FF0F000)
#define SOC_IOMCU_LP_TIMER_BASE_ADDR (0x5FF0E000)
#define SOC_IOMCU_LP_WDG_BASE_ADDR (0x5FF0D000)
#define SOC_IOMCU_CORE_CRG_B_M_BASE_ADDR (0x5FF09000)
#define SOC_IOMCU_CORE_CRG_L_L3_BASE_ADDR (0x5FF08000)
#define SOC_IOMCU_LP_PCTRL_BASE_ADDR (0x5FF07000)
#define SOC_IOMCU_PERI_CRG_BASE_ADDR (0x5FF05000)
#define SOC_IOMCU_PMU_I2C0_BASE_ADDR (0x5FF03000)
#define SOC_IOMCU_UART6_BASE_ADDR (0x5FF02000)
#define SOC_IOMCU_PMC_BASE_ADDR (0x5FF01000)
#define SOC_IOMCU_TSENSORC_BASE_ADDR (0x5FF00000)
#define SOC_IOMCU_DMSS_BASE_ADDR (0x5FE80000)
#define SOC_IOMCU_DMCPACK3_BASE_ADDR (0x5FE60000)
#define SOC_IOMCU_DMCPACK2_BASE_ADDR (0x5FE40000)
#define SOC_IOMCU_DMCPACK1_BASE_ADDR (0x5FE20000)
#define SOC_IOMCU_DMCPACK0_BASE_ADDR (0x5FE00000)
#define SOC_IOMCU_GIC_beeton_BASE_ADDR (0x5E400000)
#define SOC_IOMCU_System_Cache_BASE_ADDR (0x5E300000)
#define SOC_IOMCU_FD_BUS_Service_Target_BASE_ADDR (0x5E280000)
#define SOC_IOMCU_AOBUS_Service_Target_BASE_ADDR (0x5E270000)
#define SOC_IOMCU_DMA_NOC_Service_Target_BASE_ADDR (0x5E260000)
#define SOC_IOMCU_CFGBUS_Service_Target_BASE_ADDR (0x5E250000)
#define SOC_IOMCU_SYS_BUS_Service_Target_BASE_ADDR (0x5E240000)
#define SOC_IOMCU_ASP_Service_Target_BASE_ADDR (0x5E230000)
#define SOC_IOMCU_Modem_Service_Target_BASE_ADDR (0x5E220000)
#define SOC_IOMCU_HSDT_Service_Target_BASE_ADDR (0x5E210000)
#define SOC_IOMCU_MMC0_Service_target_BASE_ADDR (0x5E200000)
#define SOC_IOMCU_G3D_BASE_ADDR (0x5E140000)
#define SOC_IOMCU_CODEC_SSI_BASE_ADDR (0x5E104000)
#define SOC_IOMCU_IOC_BASE_ADDR (0x5E102000)
#define SOC_IOMCU_IPC_NS_BASE_ADDR (0x5E101000)
#define SOC_IOMCU_IPC_BASE_ADDR (0x5E100000)
#define SOC_IOMCU_LoadMonitor1_BASE_ADDR (0x5E031000)
#define SOC_IOMCU_LoadMonitor0_BASE_ADDR (0x5E030000)
#define SOC_IOMCU_CFG_BLPWM_BASE_ADDR (0x5E02F000)
#define SOC_IOMCU_PCTRL_BASE_ADDR (0x5E02E000)
#define SOC_IOMCU_ATGC_BASE_ADDR (0x5E02D000)
#define SOC_IOMCU_TZPC_BASE_ADDR (0x5E02C000)
#define SOC_IOMCU_GPUPCR_BASE_ADDR (0x5E028000)
#define SOC_IOMCU_WD1_BASE_ADDR (0x5E027000)
#define SOC_IOMCU_WD0_BASE_ADDR (0x5E026000)
#define SOC_IOMCU_CTF_BASE_ADDR (0x5E025000)
#define SOC_IOMCU_PWM_BASE_ADDR (0x5E024000)
#define SOC_IOMCU_TIMER12_BASE_ADDR (0x5E01F000)
#define SOC_IOMCU_TIMER11_BASE_ADDR (0x5E01E000)
#define SOC_IOMCU_TIMER10_BASE_ADDR (0x5E01D000)
#define SOC_IOMCU_TIMER9_BASE_ADDR (0x5E01C000)
#define SOC_IOMCU_GPIO19_BASE_ADDR (0x5E015000)
#define SOC_IOMCU_GPIO18_BASE_ADDR (0x5E014000)
#define SOC_IOMCU_GPIO17_BASE_ADDR (0x5E013000)
#define SOC_IOMCU_GPIO16_BASE_ADDR (0x5E012000)
#define SOC_IOMCU_GPIO15_BASE_ADDR (0x5E011000)
#define SOC_IOMCU_GPIO14_BASE_ADDR (0x5E010000)
#define SOC_IOMCU_GPIO13_BASE_ADDR (0x5E00F000)
#define SOC_IOMCU_GPIO12_BASE_ADDR (0x5E00E000)
#define SOC_IOMCU_GPIO11_BASE_ADDR (0x5E00D000)
#define SOC_IOMCU_GPIO10_BASE_ADDR (0x5E00C000)
#define SOC_IOMCU_GPIO9_BASE_ADDR (0x5E00B000)
#define SOC_IOMCU_GPIO8_BASE_ADDR (0x5E00A000)
#define SOC_IOMCU_GPIO7_BASE_ADDR (0x5E009000)
#define SOC_IOMCU_GPIO6_BASE_ADDR (0x5E008000)
#define SOC_IOMCU_GPIO5_BASE_ADDR (0x5E007000)
#define SOC_IOMCU_GPIO4_BASE_ADDR (0x5E006000)
#define SOC_IOMCU_GPIO3_BASE_ADDR (0x5E005000)
#define SOC_IOMCU_GPIO2_BASE_ADDR (0x5E004000)
#define SOC_IOMCU_GPIO1_BASE_ADDR (0x5E003000)
#define SOC_IOMCU_GPIO0_BASE_ADDR (0x5E002000)
#define SOC_IOMCU_GPIO0_SE_BASE_ADDR (0x5E000000)
#define SOC_IOMCU_CSSYS_APB_BASE_ADDR (0x5C000000)
#define SOC_IOMCU_FD_UL_DMMU2_CFG_BASE_ADDR (0x5AE80000)
#define SOC_IOMCU_DAVINCI_TINY_SUBSYSTEM_BASE_ADDR (0x5AE40000)
#define SOC_IOMCU_FD_UL_DMMU1_CFG_BASE_ADDR (0x5AE12000)
#define SOC_IOMCU_FD_UL_DMMU0_CFG_BASE_ADDR (0x5AE11000)
#define SOC_IOMCU_FD_UL_DMA_CFG_BASE_ADDR (0x5AE10000)
#define SOC_IOMCU_FD_UL_IMGCAP_CFG_BASE_ADDR (0x5AE08000)
#define SOC_IOMCU_FD_UL_ETZPC_CFG_BASE_ADDR (0x5AE06000)
#define SOC_IOMCU_FD_UL_SUBSYS_ENHC_CFG_BASE_ADDR (0x5AE05000)
#define SOC_IOMCU_FD_UL_SUBSYS_SEC_CFG_BASE_ADDR (0x5AE04000)
#define SOC_IOMCU_FD_UL_LB_ASC_BASE_ADDR (0x5AE03000)
#define SOC_IOMCU_FD_UL_LB_CFG_BASE_ADDR (0x5AE02000)
#define SOC_IOMCU_FD_UL_SUBSYS_CFG_BASE_ADDR (0x5AE01000)
#define SOC_IOMCU_FD_UL_TZPC_CFG_BASE_ADDR (0x5AE00000)
#define SOC_IOMCU_FD_UL_LB_BASE_ADDR (0x5AC00000)
#define SOC_IOMCU_AO_TCP_BASE_ADDR (0x5A980000)
#define SOC_IOMCU_MAD_DSPIF_BASE_ADDR (0x5A902000)
#define SOC_IOMCU_MAD_X2P_BASE_ADDR (0x5A900000)
#define SOC_IOMCU_GPIO36_BASE_ADDR (0x5A8B8000)
#define SOC_IOMCU_GPIO35_BASE_ADDR (0x5A8B7000)
#define SOC_IOMCU_GPIO34_BASE_ADDR (0x5A8B6000)
#define SOC_IOMCU_GPIO33_BASE_ADDR (0x5A8B5000)
#define SOC_IOMCU_GPIO32_BASE_ADDR (0x5A8B4000)
#define SOC_IOMCU_GPIO31_BASE_ADDR (0x5A8B3000)
#define SOC_IOMCU_GPIO30_BASE_ADDR (0x5A8B2000)
#define SOC_IOMCU_GPIO29_BASE_ADDR (0x5A8B1000)
#define SOC_IOMCU_GPIO28_BASE_ADDR (0x5A8B0000)
#define SOC_IOMCU_GPIO27_BASE_ADDR (0x5A8AF000)
#define SOC_IOMCU_GPIO26_BASE_ADDR (0x5A8AE000)
#define SOC_IOMCU_GPIO25_BASE_ADDR (0x5A8AD000)
#define SOC_IOMCU_GPIO24_BASE_ADDR (0x5A8AC000)
#define SOC_IOMCU_GPIO23_BASE_ADDR (0x5A8AB000)
#define SOC_IOMCU_GPIO22_BASE_ADDR (0x5A8AA000)
#define SOC_IOMCU_GPIO21_BASE_ADDR (0x5A8A9000)
#define SOC_IOMCU_GPIO20_BASE_ADDR (0x5A8A8000)
#define SOC_IOMCU_GPIO1_SE_BASE_ADDR (0x5A8A1000)
#define SOC_IOMCU_AO_WDG_BASE_ADDR (0x5A8A0000)
#define SOC_IOMCU_SPI3_BASE_ADDR (0x5A89F000)
#define SOC_IOMCU_AO_TZPC_BASE_ADDR (0x5A89E000)
#define SOC_IOMCU_AO_LoadMonitor_BASE_ADDR (0x5A89D000)
#define SOC_IOMCU_AO_IOC_BASE_ADDR (0x5A89C000)
#define SOC_IOMCU_SCTRL_BASE_ADDR (0x5A89B000)
#define SOC_IOMCU_EFUSEC_BASE_ADDR (0x5A89A000)
#define SOC_IOMCU_AO_IPC_NS_BASE_ADDR (0x5A899000)
#define SOC_IOMCU_AO_IPC_S_BASE_ADDR (0x5A898000)
#define SOC_IOMCU_SYS_CNT_BASE_ADDR (0x5A896000)
#define SOC_IOMCU_BB_DRX_BASE_ADDR (0x5A894000)
#define SOC_IOMCU_SPMI_BASE_ADDR (0x5A890000)
#define SOC_IOMCU_SCI1_BASE_ADDR (0x5A88F000)
#define SOC_IOMCU_SCI0_BASE_ADDR (0x5A88E000)
#define SOC_IOMCU_RTC1_BASE_ADDR (0x5A88D000)
#define SOC_IOMCU_RTC0_BASE_ADDR (0x5A88C000)
#define SOC_IOMCU_TIMER8_BASE_ADDR (0x5A888000)
#define SOC_IOMCU_TIMER7_BASE_ADDR (0x5A887000)
#define SOC_IOMCU_TIMER6_BASE_ADDR (0x5A886000)
#define SOC_IOMCU_TIMER5_BASE_ADDR (0x5A885000)
#define SOC_IOMCU_TIMER4_BASE_ADDR (0x5A884000)
#define SOC_IOMCU_TIMER3_BASE_ADDR (0x5A883000)
#define SOC_IOMCU_TIMER2_BASE_ADDR (0x5A882000)
#define SOC_IOMCU_TIMER1_BASE_ADDR (0x5A881000)
#define SOC_IOMCU_TIMER0_BASE_ADDR (0x5A880000)
#define SOC_IOMCU_IOMCU_RTC_BASE_ADDR (0x5A87F000)
#define SOC_IOMCU_IOMCU_CONFIG_BASE_ADDR (0x5A87E000)
#define SOC_IOMCU_IOMCU_TIMER_BASE_ADDR (0x5A87D000)
#define SOC_IOMCU_IOMCU_WDG_BASE_ADDR (0x5A87C000)
#define SOC_IOMCU_IOMCU_GPIO3_BASE_ADDR (0x5A87B000)
#define SOC_IOMCU_IOMCU_GPIO2_BASE_ADDR (0x5A87A000)
#define SOC_IOMCU_IOMCU_GPIO1_BASE_ADDR (0x5A879000)
#define SOC_IOMCU_IOMCU_GPIO0_BASE_ADDR (0x5A878000)
#define SOC_IOMCU_IOMCU_DMAC_BASE_ADDR (0x5A877000)
#define SOC_IOMCU_IOMCU_UART7_BASE_ADDR (0x5A876000)
#define SOC_IOMCU_IOMCU_BLPWM_BASE_ADDR (0x5A875000)
#define SOC_IOMCU_IOMCU_UART3_BASE_ADDR (0x5A874000)
#define SOC_IOMCU_IOMCU_I3C1_BASE_ADDR (0x5A873000)
#define SOC_IOMCU_IOMCU_I2C1_BASE_ADDR (0x5A872000)
#define SOC_IOMCU_IOMCU_I2C0_BASE_ADDR (0x5A871000)
#define SOC_IOMCU_IOMCU_SPI0_BASE_ADDR (0x5A870000)
#define SOC_IOMCU_IOMCU_I2C5_BASE_ADDR (0x5A86F000)
#define SOC_IOMCU_IOMCU_I3C2_BASE_ADDR (0x5A86A000)
#define SOC_IOMCU_IOMCU_UART8_BASE_ADDR (0x5A869000)
#define SOC_IOMCU_IOMCU_SPI2_BASE_ADDR (0x5A868000)
#define SOC_IOMCU_IOMCU_DMMU_BASE_ADDR (0x5A867000)
#define SOC_IOMCU_IOMCU_TIMER2_BASE_ADDR (0x5A866000)
#define SOC_IOMCU_IOMCU_I3C_BASE_ADDR (0x5A865000)
#define SOC_IOMCU_IOMCU_I3C3_BASE_ADDR (0x5A864000)
#define SOC_IOMCU_IOMCU_TCP_RAM_BASE_ADDR (0x5A851000)
#define SOC_IOMCU_IOMCU_TCP_CFG_BASE_ADDR (0x5A850000)
#define SOC_IOMCU_IOMCU_DTCM_BASE_ADDR (0x5A700000)
#define SOC_IOMCU_IOMCU_ITCM_BASE_ADDR (0x5A600000)
#define SOC_IOMCU_THINKER_BASE_ADDR (0x5A5C0000)
#define SOC_IOMCU_DSP_ITCM_BASE_ADDR (0x5A580000)
#define SOC_IOMCU_DSP_DTCM_BASE_ADDR (0x5A558000)
#define SOC_IOMCU_SoundWire_BASE_ADDR (0x5A554000)
#define SOC_IOMCU_ASP_CODEC_BASE_ADDR (0x5A552000)
#define SOC_IOMCU_SLIMBUS_BASE_ADDR (0x5A550000)
#define SOC_IOMCU_DSD_BASE_ADDR (0x5A54FC00)
#define SOC_IOMCU_SIO_BT_BASE_ADDR (0x5A54F800)
#define SOC_IOMCU_SIO_THINKER_BASE_ADDR (0x5A54F400)
#define SOC_IOMCU_SIO_AUDIO_BASE_ADDR (0x5A54F000)
#define SOC_IOMCU_ASP_HDMI_SPDIF_BASE_ADDR (0x5A54EC00)
#define SOC_IOMCU_ASP_HDMI_SIO_BASE_ADDR (0x5A54E800)
#define SOC_IOMCU_ASP_HDMI_ASP_BASE_ADDR (0x5A54E400)
#define SOC_IOMCU_ASP_CFG_BASE_ADDR (0x5A54E000)
#define SOC_IOMCU_ASP_WD_BASE_ADDR (0x5A54D000)
#define SOC_IOMCU_ASP_IPC_BASE_ADDR (0x5A54C000)
#define SOC_IOMCU_ASP_DMAC_BASE_ADDR (0x5A54B000)
#define SOC_IOMCU_ASP_TIMER1_BASE_ADDR (0x5A54A000)
#define SOC_IOMCU_ASP_TIMER0_BASE_ADDR (0x5A549000)
#define SOC_IOMCU_ASP_GPIO_BASE_ADDR (0x5A548000)
#define SOC_IOMCU_ASP_DMMU_BASE_ADDR (0x5A547000)
#define SOC_IOMCU_SECRAM_BASE_ADDR (0x5A480000)
#define SOC_IOMCU_USB_AUDIO_BASE_ADDR (0x5A400000)
#define SOC_IOMCU_HISEE_IPC_BASE_ADDR (0x5A230000)
#define SOC_IOMCU_HISEE_MAILBOX_BASE_ADDR (0x5A220000)
#define SOC_IOMCU_SOCP_BASE_ADDR (0x5A0A0000)
#define SOC_IOMCU_IPF_PSAM_BASE_ADDR (0x5A090000)
#define SOC_IOMCU_IPF_BASE_ADDR (0x5A088000)
#define SOC_IOMCU_IPC_MDM_NS_BASE_ADDR (0x5A081000)
#define SOC_IOMCU_IPC_MDM_S_BASE_ADDR (0x5A080000)
#define SOC_IOMCU_LAT_STAT_BASE_ADDR (0x5A057000)
#define SOC_IOMCU_PERF_STAT_BASE_ADDR (0x5A056000)
#define SOC_IOMCU_UART0_BASE_ADDR (0x5A054000)
#define SOC_IOMCU_I3C4_BASE_ADDR (0x5A050000)
#define SOC_IOMCU_I2C7_BASE_ADDR (0x5A04F000)
#define SOC_IOMCU_I2C6_BASE_ADDR (0x5A04E000)
#define SOC_IOMCU_I2C4_BASE_ADDR (0x5A04D000)
#define SOC_IOMCU_I2C3_BASE_ADDR (0x5A04C000)
#define SOC_IOMCU_SPI4_BASE_ADDR (0x5A049000)
#define SOC_IOMCU_SPI1_BASE_ADDR (0x5A048000)
#define SOC_IOMCU_UART5_BASE_ADDR (0x5A045000)
#define SOC_IOMCU_UART2_BASE_ADDR (0x5A044000)
#define SOC_IOMCU_UART4_BASE_ADDR (0x5A041000)
#define SOC_IOMCU_UART1_BASE_ADDR (0x5A040000)
#define SOC_IOMCU_PERI_DMAC_BASE_ADDR (0x5A000000)
#define SOC_IOMCU_CS_STM_BASE_ADDR (0x59000000)
#define SOC_IOMCU_SD3_BASE_ADDR (0x58583000)
#define SOC_IOMCU_MMC0_SYS_CTRL_BASE_ADDR (0x58582000)
#define SOC_IOMCU_MMC0_CRG_BASE_ADDR (0x58581000)
#define SOC_IOMCU_IOC_MMC0_BASE_ADDR (0x58580000)
#define SOC_IOMCU_DP_CTRL_BASE_ADDR (0x58500000)
#define SOC_IOMCU_USB_DP_CTRL_BASE_ADDR (0x58481000)
#define SOC_IOMCU_USB_TCA_BASE_ADDR (0x58480200)
#define SOC_IOMCU_USB_MISC_CTRL_BASE_ADDR (0x58480000)
#define SOC_IOMCU_USB_OTG_BASE_ADDR (0x58400000)
#define SOC_IOMCU_UFS_CFG_BASE_ADDR (0x58200000)
#define SOC_IOMCU_UFS_SYS_CTRL_BASE_ADDR (0x581FF000)
#define SOC_IOMCU_HSDT_SYS_CTRL_BASE_ADDR (0x58181000)
#define SOC_IOMCU_HSDT_CRG_BASE_ADDR (0x58180000)
#define SOC_IOMCU_MMC1_IOC_BASE_ADDR (0x58105000)
#define SOC_IOMCU_MMC1_SDIO_BASE_ADDR (0x58104000)
#define SOC_IOMCU_PCIE1_APB_CFG_BASE_ADDR (0x58103000)
#define SOC_IOMCU_PCIE0_APB_CFG_BASE_ADDR (0x58102000)
#define SOC_IOMCU_CCP_BASE_ADDR (0x58101000)
#define SOC_IOMCU_CCS_BASE_ADDR (0x58100000)
#define SOC_IOMCU_PCIEPHY1_BASE_ADDR (0x58080000)
#define SOC_IOMCU_PCIEPHY0_BASE_ADDR (0x58000000)
#define SOC_IOMCU_PCIECtrl1_BASE_ADDR (0x54000000)
#define SOC_IOMCU_PCIECtrl0_BASE_ADDR (0x50000000)
#define SOC_IOMCU_NN_BASE_ADDR (0x49540000)
#define SOC_IOMCU_NN_SMMU_BASE_ADDR (0x49500000)
#define SOC_IOMCU_EPS_RAM_BASE_ADDR (0x49470000)
#define SOC_IOMCU_ICCM_RAM_BASE_ADDR (0x49448000)
#define SOC_IOMCU_EPS_ROM_BASE_ADDR (0x49440000)
#define SOC_IOMCU_EPS_MMU_BASE_ADDR (0x49430000)
#define SOC_IOMCU_EPS_PKE_BASE_ADDR (0x49422000)
#define SOC_IOMCU_EPS_SCE_BASE_ADDR (0x49421000)
#define SOC_IOMCU_EPS_KM_BASE_ADDR (0x49420000)
#define SOC_IOMCU_EPS_IPC_BASE_ADDR (0x49407000)
#define SOC_IOMCU_EPS_UART_BASE_ADDR (0x49406000)
#define SOC_IOMCU_EPS_WD_BASE_ADDR (0x49405000)
#define SOC_IOMCU_EPS_TIMER_BASE_ADDR (0x49404000)
#define SOC_IOMCU_EPS_TRNG_BASE_ADDR (0x49402000)
#define SOC_IOMCU_EPS_ETZPC_BASE_ADDR (0x49401000)
#define SOC_IOMCU_EPS_CONFIG_BASE_ADDR (0x49400000)
#define SOC_IOMCU_EPS_BUS_Service_Target_BASE_ADDR (0x493D0000)
#define SOC_IOMCU_NOC_VENC2_Service_Target_BASE_ADDR (0x493C0000)
#define SOC_IOMCU_NOC_VENC_Service_Target_BASE_ADDR (0x493B0000)
#define SOC_IOMCU_NOC_VDEC_Service_Target_BASE_ADDR (0x493A0000)
#define SOC_IOMCU_NOC_VCODECBUS_Service_Target_BASE_ADDR (0x49390000)
#define SOC_IOMCU_NOC_IVP32_Service_Target_BASE_ADDR (0x49380000)
#define SOC_IOMCU_MEDAI2_LoadMonitor_BASE_ADDR (0x49301000)
#define SOC_IOMCU_MEDIA2_CRG_BASE_ADDR (0x49300000)
#define SOC_IOMCU_VENC2_BASE_ADDR (0x492C0000)
#define SOC_IOMCU_VENC_BASE_ADDR (0x49280000)
#define SOC_IOMCU_VDEC_BASE_ADDR (0x49200000)
#define SOC_IOMCU_IVP32_SMMU_BASE_ADDR (0x491C0000)
#define SOC_IOMCU_DPM_BASE_ADDR (0x49184000)
#define SOC_IOMCU_IVP32_TIMER1_BASE_ADDR (0x49183000)
#define SOC_IOMCU_IVP32_TIMER0_BASE_ADDR (0x49182000)
#define SOC_IOMCU_IVP32_WDG_BASE_ADDR (0x49181000)
#define SOC_IOMCU_IVP32_CFG_BASE_ADDR (0x49180000)
#define SOC_IOMCU_IVP32_IRAM_BASE_ADDR (0x49100000)
#define SOC_IOMCU_IVP32_DRAM1_BASE_ADDR (0x49040000)
#define SOC_IOMCU_IVP32_DRAM0_BASE_ADDR (0x49000000)
#define SOC_IOMCU_NOC_VIVO_Service_Target_BASE_ADDR (0x486A0000)
#define SOC_IOMCU_NOC_ISP_Service_Target_BASE_ADDR (0x48690000)
#define SOC_IOMCU_NOC_DSS_Service_Target_BASE_ADDR (0x48680000)
#define SOC_IOMCU_CSI_D_BASE_ADDR (0x48607000)
#define SOC_IOMCU_CSI_C_BASE_ADDR (0x48606000)
#define SOC_IOMCU_CSI_B_BASE_ADDR (0x48605000)
#define SOC_IOMCU_CSI_A_BASE_ADDR (0x48604000)
#define SOC_IOMCU_CSI_adapter_BASE_ADDR (0x48603000)
#define SOC_IOMCU_IDI2AXI_BASE_ADDR (0x48602000)
#define SOC_IOMCU_MEDIA1_CRG_BASE_ADDR (0x48601000)
#define SOC_IOMCU_MEDIA1_LoadMonitor_BASE_ADDR (0x48600000)
#define SOC_IOMCU_MEDIA_COMMON_BASE_ADDR (0x48500000)
#define SOC_IOMCU_GMP_LUT1_BASE_ADDR (0x484EE000)
#define SOC_IOMCU_GMP_LUT0_BASE_ADDR (0x484DE000)
#define SOC_IOMCU_DISP_CH2_BASE_ADDR (0x484C2000)
#define SOC_IOMCU_DPM_POST_BASE_ADDR (0x484BF000)
#define SOC_IOMCU_CROSSBAR_BASE_ADDR (0x484BE000)
#define SOC_IOMCU_DISP_CH1_BASE_ADDR (0x484A2000)
#define SOC_IOMCU_DISP_GLB_BASE_ADDR (0x484A1000)
#define SOC_IOMCU_WB_BASE_ADDR (0x484A0000)
#define SOC_IOMCU_DSS_PRO_BASE_ADDR (0x48490000)
#define SOC_IOMCU_DBCU0_SMMU_BASE_ADDR (0x48480000)
#define SOC_IOMCU_DP_ITF1_BASE_ADDR (0x4847F000)
#define SOC_IOMCU_DP_ITF0_BASE_ADDR (0x4847E000)
#define SOC_IOMCU_DISP_CH0_BASE_ADDR (0x48462000)
#define SOC_IOMCU_DPM_PRE_BASE_ADDR (0x48461000)
#define SOC_IOMCU_OV3_BASE_ADDR (0x48460C00)
#define SOC_IOMCU_OV2_BASE_ADDR (0x48460800)
#define SOC_IOMCU_OV1_BASE_ADDR (0x48460400)
#define SOC_IOMCU_OV0_BASE_ADDR (0x48460000)
#define SOC_IOMCU_WCH1_BASE_ADDR (0x4845C000)
#define SOC_IOMCU_WCH0_BASE_ADDR (0x4845A000)
#define SOC_IOMCU_RCH_D1_BASE_ADDR (0x48453000)
#define SOC_IOMCU_RCH_D0_BASE_ADDR (0x48452000)
#define SOC_IOMCU_RCH_D3_BASE_ADDR (0x48451000)
#define SOC_IOMCU_RCH_D2_BASE_ADDR (0x48450000)
#define SOC_IOMCU_RCH_G1_BASE_ADDR (0x48440000)
#define SOC_IOMCU_RCH_G0_BASE_ADDR (0x48438000)
#define SOC_IOMCU_RCH_V2_BASE_ADDR (0x48430000)
#define SOC_IOMCU_RCH_V1_BASE_ADDR (0x48428000)
#define SOC_IOMCU_RCH_V0_BASE_ADDR (0x48420000)
#define SOC_IOMCU_GLB_BASE_ADDR (0x48412000)
#define SOC_IOMCU_DBUG_BASE_ADDR (0x48411000)
#define SOC_IOMCU_MCTL_MUTEX_BASE_ADDR (0x48410800)
#define SOC_IOMCU_MCTL_SYS_BASE_ADDR (0x48410000)
#define SOC_IOMCU_DBCU0_MIF_BASE_ADDR (0x4840A000)
#define SOC_IOMCU_DBCU1_AIF_BASE_ADDR (0x48409000)
#define SOC_IOMCU_DBCU0_AIF_BASE_ADDR (0x48407000)
#define SOC_IOMCU_CMD_BASE_ADDR (0x48402000)
#define SOC_IOMCU_ASC_BASE_ADDR (0x48401800)
#define SOC_IOMCU_DSI1_BASE_ADDR (0x48401400)
#define SOC_IOMCU_DSI0_BASE_ADDR (0x48401000)
#define SOC_IOMCU_MMBUF_CFG_BASE_ADDR (0x48400000)
#define SOC_IOMCU_SMMUv500_CFG_BASE_ADDR (0x483F0000)
#define SOC_IOMCU_ISP_R8_PERI_BASE_BASE_ADDR (0x483EE000)
#define SOC_IOMCU_DPM_CAP_BASE_ADDR (0x483E9000)
#define SOC_IOMCU_DPM_VID_BASE_ADDR (0x483E8000)
#define SOC_IOMCU_DPM_RT_BASE_ADDR (0x483E7000)
#define SOC_IOMCU_DPM_IPP_BASE_ADDR (0x483E6000)
#define SOC_IOMCU_DPM_NOC_BASE_ADDR (0x483E5000)
#define SOC_IOMCU_ISP_TCMDMA_BASE_ADDR (0x483E4000)
#define SOC_IOMCU_ISP_SUB_CTRL_BASE_ADDR (0x483E3000)
#define SOC_IOMCU_ISP_IPC_BASE_ADDR (0x483E2000)
#define SOC_IOMCU_ISP_TIMER_BASE_ADDR (0x483E1000)
#define SOC_IOMCU_ISP_WDT_BASE_ADDR (0x483E0000)
#define SOC_IOMCU_ISP_R8_DTCM_BASE_ADDR (0x483D0000)
#define SOC_IOMCU_ISP_R8_ITCM_BASE_ADDR (0x483C0000)
#define SOC_IOMCU_ISP_Core_CFG_BASE_ADDR (0x48200000)
#define SOC_IOMCU_FD_BASE_ADDR (0x48040000)
#define SOC_IOMCU_SMMU_BASE_ADDR (0x48020000)
#define SOC_IOMCU_SLAM_BASE_ADDR (0x4800C000)
#define SOC_IOMCU_MFNR_BASE_ADDR (0x4800B000)
#define SOC_IOMCU_MCF_BASE_ADDR (0x4800A000)
#define SOC_IOMCU_CVDR_BASE_ADDR (0x48006000)
#define SOC_IOMCU_CMDLST_BASE_ADDR (0x48005000)
#define SOC_IOMCU_TOP_BASE_ADDR (0x48004000)
#define SOC_IOMCU_SMMU_MASTER_BASE_ADDR (0x48003000)
#define SOC_IOMCU_JPGDEC_BASE_ADDR (0x48001000)
#define SOC_IOMCU_JPGENC_BASE_ADDR (0x48000000)
#define SOC_IOMCU_L2BUF_CFG_BASE_ADDR (0x45F80000)
#define SOC_IOMCU_SYSDMA_MMU_CFG_BASE_ADDR (0x45F00000)
#define SOC_IOMCU_SYSDMA_CFG_BASE_ADDR (0x45E80000)
#define SOC_IOMCU_ETZPC_BASE_ADDR (0x45E07000)
#define SOC_IOMCU_DPM_AICORE_BASE_ADDR (0x45E06000)
#define SOC_IOMCU_UART_NPU_BASE_ADDR (0x45E05000)
#define SOC_IOMCU_SCTRL_NPU_BASE_ADDR (0x45E04000)
#define SOC_IOMCU_CRG_NPU_BASE_ADDR (0x45E03000)
#define SOC_IOMCU_TZPC_NPU_BASE_ADDR (0x45E02000)
#define SOC_IOMCU_IPC_NS_NPU_BASE_ADDR (0x45E01000)
#define SOC_IOMCU_IPC_S_NPU_BASE_ADDR (0x45E00000)
#define SOC_IOMCU_GIC_CFG_BASE_ADDR (0x45C00000)
#define SOC_IOMCU_AICORE_SMMU_BASE_ADDR (0x45800000)
#define SOC_IOMCU_AICORE_SUBSYS_BASE_ADDR (0x45000000)
#define SOC_IOMCU_npu_bus_service_target_BASE_ADDR (0x44D30000)
#define SOC_IOMCU_ts_bus_service_target_BASE_ADDR (0x44D20000)
#define SOC_IOMCU_aicore_bus_service_target_BASE_ADDR (0x44D10000)
#define SOC_IOMCU_aicpu_bus_service_target_BASE_ADDR (0x44D00000)
#define SOC_IOMCU_timer1_sp804_BASE_ADDR (0x44C33000)
#define SOC_IOMCU_watchdog1_sp805_BASE_ADDR (0x44C32000)
#define SOC_IOMCU_core_local_ctrl_BASE_ADDR (0x44C31000)
#define SOC_IOMCU_ai_cpu_local_ctrl_BASE_ADDR (0x44C30000)
#define SOC_IOMCU_ai_cpu_timer0_BASE_ADDR (0x44C20000)
#define SOC_IOMCU_ai_cpu_watchdog0_BASE_ADDR (0x44C00000)
#define SOC_IOMCU_L2BUF_BASE_ADDR (0x44800000)
#define SOC_IOMCU_SRAM_Reg0_BASE_ADDR (0x44230000)
#define SOC_IOMCU_SRAM_AtomicDec0_BASE_ADDR (0x44220000)
#define SOC_IOMCU_SRAM_AtomicAdd0_BASE_ADDR (0x44210000)
#define SOC_IOMCU_SRAM_Normal0_BASE_ADDR (0x44200000)
#define SOC_IOMCU_BS_slv0_BASE_ADDR (0x44110000)
#define SOC_IOMCU_doorbell_sts0_BASE_ADDR (0x44100000)
#define SOC_IOMCU_doorbell_cfg0_BASE_ADDR (0x44080000)
#define SOC_IOMCU_TS_SECURE0_BASE_ADDR (0x44037000)
#define SOC_IOMCU_AXI_MON_SOC0_BASE_ADDR (0x44036000)
#define SOC_IOMCU_AXI_MON_BS0_BASE_ADDR (0x44035000)
#define SOC_IOMCU_AXI_MON_CPU0_BASE_ADDR (0x44034000)
#define SOC_IOMCU_DPM_TS_BASE_ADDR (0x44032000)
#define SOC_IOMCU_FCM_LOCAL0_BASE_ADDR (0x44031000)
#define SOC_IOMCU_NPU_SYSCTRL0_BASE_ADDR (0x44030000)
#define SOC_IOMCU_NPU_TIMER0_BASE_ADDR (0x44020000)
#define SOC_IOMCU_NPU_WDG0_BASE_ADDR (0x44000000)
#define SOC_IOMCU_DSP0_L2C_BASE_ADDR (0x43A02000)
#define SOC_IOMCU_DSP0_L2MEM_BASE_ADDR (0x43680000)
#define SOC_IOMCU_DSP0_IMEM_BASE_ADDR (0x43500000)
#define SOC_IOMCU_DSP0_DMEM_BASE_ADDR (0x43400000)
#define SOC_IOMCU_GUTL_BBP_BASE_ADDR (0x41000000)
#define SOC_IOMCU_CCPU_LOCAL_MEM_BASE_ADDR (0x40800000)
#define SOC_IOMCU_CCPU1_DTCM_BASE_ADDR (0x40760000)
#define SOC_IOMCU_CCPU1_ITCM_BASE_ADDR (0x40740000)
#define SOC_IOMCU_CCPU0_DTCM_BASE_ADDR (0x40720000)
#define SOC_IOMCU_CCPU0_ITCM_BASE_ADDR (0x40700000)
#define SOC_IOMCU_AMON_MDM_BASE_ADDR (0x40520000)
#define SOC_IOMCU_NOC_SERVICE_BASE_ADDR (0x40500000)
#define SOC_IOMCU_DFC_BASE_ADDR (0x40458000)
#define SOC_IOMCU_UPACC_BASE_ADDR (0x40453000)
#define SOC_IOMCU_CIPHER_BASE_ADDR (0x40452000)
#define SOC_IOMCU_RSR_ACC_BASE_ADDR (0x40444000)
#define SOC_IOMCU_CICOM1_BASE_ADDR (0x40443000)
#define SOC_IOMCU_HDLC_BASE_ADDR (0x40442000)
#define SOC_IOMCU_IPCM_BASE_ADDR (0x40441000)
#define SOC_IOMCU_CICOM0_BASE_ADDR (0x40440000)
#define SOC_IOMCU_MDM_POWER_MON3_BASE_ADDR (0x40227000)
#define SOC_IOMCU_MDM_POWER_MON2_BASE_ADDR (0x40226000)
#define SOC_IOMCU_MDM_POWER_MON1_BASE_ADDR (0x40225000)
#define SOC_IOMCU_MDM_POWER_MON0_BASE_ADDR (0x40224000)
#define SOC_IOMCU_DW_AXI_MDM_CCPU_BASE_ADDR (0x40223000)
#define SOC_IOMCU_DW_AXI_MDM_MST_BASE_ADDR (0x40222000)
#define SOC_IOMCU_SYSCNT_MDM_BASE_ADDR (0x40220000)
#define SOC_IOMCU_MDM_RTC_BASE_ADDR (0x40217000)
#define SOC_IOMCU_WDT_CCPU1_BASE_ADDR (0x40214000)
#define SOC_IOMCU_WDT_DSP0_BASE_ADDR (0x40212000)
#define SOC_IOMCU_EDMA1_MDM_BASE_ADDR (0x40210000)
#define SOC_IOMCU_TIMER9_MDM_BASE_ADDR (0x4020F000)
#define SOC_IOMCU_TIMER8_MDM_BASE_ADDR (0x4020E000)
#define SOC_IOMCU_TIMER7_MDM_BASE_ADDR (0x4020D000)
#define SOC_IOMCU_TIMER6_MDM_BASE_ADDR (0x4020C000)
#define SOC_IOMCU_TIMER5_MDM_BASE_ADDR (0x4020B000)
#define SOC_IOMCU_TIMER4_MDM_BASE_ADDR (0x4020A000)
#define SOC_IOMCU_TIMER3_MDM_BASE_ADDR (0x40209000)
#define SOC_IOMCU_TIMER2_MDM_BASE_ADDR (0x40208000)
#define SOC_IOMCU_TIMER1_MDM_BASE_ADDR (0x40207000)
#define SOC_IOMCU_TIMER0_MDM_BASE_ADDR (0x40206000)
#define SOC_IOMCU_UART1_MDM_BASE_ADDR (0x40205000)
#define SOC_IOMCU_EDMA0_MDM_BASE_ADDR (0x40204000)
#define SOC_IOMCU_UART0_MDM_BASE_ADDR (0x40203000)
#define SOC_IOMCU_WDT_CCPU0_BASE_ADDR (0x40201000)
#define SOC_IOMCU_SYSCTRL_MDM_BASE_ADDR (0x40200000)
#define SOC_IOMCU_CCPU_PRIVATE_BASE_ADDR (0x40100000)
#define SOC_IOMCU_CCPU_L2C_BASE_ADDR (0x40000000)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
