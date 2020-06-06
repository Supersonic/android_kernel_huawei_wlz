obj-$(CONFIG_WIRELESS_CHARGER) += wireless_charger.o wireless_transmitter.o
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_direct_charger/
obj-$(CONFIG_WIRELESS_CHARGER) += wireless_power_supply/
obj-$(CONFIG_WIRELESS_IDTP9221) += idtp9221/
obj-$(CONFIG_WIRELESS_STWLC68) += stwlc68/
obj-$(CONFIG_WIRELESS_CP_HL1506) += cp_hl1506/
obj-$(CONFIG_WIRELESS_DUAL_SC) += dual_sc/
obj-$(CONFIG_WIRELESS_ACCESSORY) += wireless_accessory/

ifeq ($(TARGET_VERSION_MODE),factory)
ccflags-y += -DWIRELESS_CHARGER_FACTORY_VERSION
endif
