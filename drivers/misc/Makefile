# SPDX-License-Identifier: GPL-2.0
#
# Makefile for misc devices that really don't fit anywhere else.
#

obj-$(CONFIG_IBM_ASM)		+= ibmasm/
obj-$(CONFIG_IBMVMC)		+= ibmvmc.o
obj-$(CONFIG_AD525X_DPOT)	+= ad525x_dpot.o
obj-$(CONFIG_AD525X_DPOT_I2C)	+= ad525x_dpot-i2c.o
obj-$(CONFIG_AD525X_DPOT_SPI)	+= ad525x_dpot-spi.o
obj-$(CONFIG_INTEL_MID_PTI)	+= pti.o
obj-$(CONFIG_ATMEL_SSC)		+= atmel-ssc.o
obj-$(CONFIG_ATMEL_TCLIB)	+= atmel_tclib.o
obj-$(CONFIG_DUMMY_IRQ)		+= dummy-irq.o
obj-$(CONFIG_ICS932S401)	+= ics932s401.o
obj-$(CONFIG_LKDTM)		+= lkdtm/
obj-$(CONFIG_TIFM_CORE)       	+= tifm_core.o
obj-$(CONFIG_TIFM_7XX1)       	+= tifm_7xx1.o
obj-$(CONFIG_PHANTOM)		+= phantom.o
obj-$(CONFIG_QCOM_COINCELL)	+= qcom-coincell.o
obj-$(CONFIG_QCOM_FASTRPC)	+= fastrpc.o
obj-$(CONFIG_SENSORS_BH1770)	+= bh1770glc.o
obj-$(CONFIG_SENSORS_APDS990X)	+= apds990x.o
obj-$(CONFIG_ENCLOSURE_SERVICES) += enclosure.o
obj-$(CONFIG_KGDB_TESTS)	+= kgdbts.o
obj-$(CONFIG_SGI_XP)		+= sgi-xp/
obj-$(CONFIG_SGI_GRU)		+= sgi-gru/
obj-$(CONFIG_CS5535_MFGPT)	+= cs5535-mfgpt.o
obj-$(CONFIG_HP_ILO)		+= hpilo.o
obj-$(CONFIG_APDS9802ALS)	+= apds9802als.o
obj-$(CONFIG_ISL29003)		+= isl29003.o
obj-$(CONFIG_ISL29020)		+= isl29020.o
obj-$(CONFIG_SENSORS_TSL2550)	+= tsl2550.o
obj-$(CONFIG_DS1682)		+= ds1682.o
obj-$(CONFIG_C2PORT)		+= c2port/
obj-$(CONFIG_HMC6352)		+= hmc6352.o
obj-$(CONFIG_RTIMD_I2C)		+= rtimd-i2c/
obj-y				+= eeprom/
obj-y				+= cb710/
obj-$(CONFIG_VMWARE_BALLOON)	+= vmw_balloon.o
obj-$(CONFIG_PCH_PHUB)		+= pch_phub.o
obj-y				+= ti-st/
obj-y				+= lis3lv02d/
obj-$(CONFIG_ALTERA_STAPL)	+=altera-stapl/
obj-$(CONFIG_INTEL_MEI)		+= mei/
obj-$(CONFIG_VMWARE_VMCI)	+= vmw_vmci/
obj-$(CONFIG_LATTICE_ECP3_CONFIG)	+= lattice-ecp3-config.o
obj-$(CONFIG_SRAM)		+= sram.o
obj-$(CONFIG_SRAM_EXEC)		+= sram-exec.o
obj-$(CONFIG_GENWQE)		+= genwqe/
obj-$(CONFIG_ECHO)		+= echo/
obj-$(CONFIG_HDCP_QSEECOM)	+= hdcp/
obj-$(CONFIG_CXL_BASE)		+= cxl/
obj-$(CONFIG_PCI_ENDPOINT_TEST)	+= pci_endpoint_test.o
obj-$(CONFIG_OCXL)		+= ocxl/
obj-$(CONFIG_ISL97900_LED)	+= isl97900_led/
obj-y				+= cardreader/
obj-$(CONFIG_PVPANIC)   	+= pvpanic.o
obj-$(CONFIG_HABANA_AI)		+= habanalabs/
obj-$(CONFIG_UACCE)		+= uacce/
obj-$(CONFIG_XILINX_SDFEC)	+= xilinx_sdfec.o
obj-$(CONFIG_HISI_HIKEY_USB)	+= hisi_hikey_usb.o
obj-$(CONFIG_UID_SYS_STATS)	+= uid_sys_stats.o
obj-$(CONFIG_QSEECOM)		+= qseecom-mod.o
qseecom-mod-$(CONFIG_QSEECOM)  := qseecom.o
qseecom-mod-$(CONFIG_COMPAT)	+= compat_qseecom.o
obj-$(CONFIG_WIGIG_SENSING_SPI)	+= wigig_sensing.o
obj-$(CONFIG_KINECTICS_XR_NORDIC) += kxrctrl/
# Added by gaochao for [TCT-859] Add detection of SIM tray 20240117 Begin
obj-$(CONFIG_SIM_TRAY_DETECT) += sim_tray_detect/
# Added by gaochao for [TCT-859] Add detection of SIM tray 20240117 End
# Added by feihui for [TCT-3473] Add battery sn validate node  20240513 Begin
obj-$(CONFIG_BATTERY_INFO) += battery_info/
# Added by feihui for [TCT-3473] Add battery sn validate node  20240513 End
# Added by gaochao for [TCT-782] Add SIM/SDcard tray detection 20240110 Begin
obj-$(CONFIG_CUSTOM_SIM_TRAY_CHECK) += sim_tray/
# Added by gaochao for [TCT-782] Add SIM/SDcard tray detection 20240110 End
# Added by yusen for TCT-849 on 20240115 begin
obj-m += sim_switch_ctrl.o
# Added by yusen for TCT-849 on 20240115 end
