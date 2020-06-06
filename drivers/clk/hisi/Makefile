ifeq ($(CONFIG_HISI_CLK_KIRIN),y)
obj-$(CONFIG_HISI_CLK)                  += clk-kirin-common.o
obj-$(CONFIG_HISI_CLK)                  += hisi-clkgate.o
obj-$(CONFIG_HISI_CLK)                  += hisi-kirin-ppll.o
obj-$(CONFIG_HISI_CLK_DEBUG)		+= clk_unit_test.o
endif
obj-$(CONFIG_HISI_CLK_MAILBOX_SUPPORT)	+= hisi-clk-mailbox.o

ifeq ($(CONFIG_HISI_CLK_KIRIN),y)
obj-$(CONFIG_HISI_CLK)                  += clk_dvfs.o
endif
obj-$(CONFIG_HISI_CLK_PM_MONITOR)	+= hisi_clk_pm_monitor.o

ifeq ($(CONFIG_HISI_CLK_KIRIN),y)
obj-$(CONFIG_HISI_PERIDVFS)  += dvfs/
endif
