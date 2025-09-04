# SPDX-License-Identifier: GPL-2.0-only

obj-$(CONFIG_QPNP_SMB5) += qpnp-smb5-main.o
qpnp-smb5-main-y += step-chg-jeita.o battery.o qpnp-smb5.o smb5-lib.o pmic-voter.o storm-watch.o schgm-flash.o battery-profile-loader.o smb5-iio.o
obj-$(CONFIG_QPNP_QG)	+= qcom-qpnp-qg.o
qcom-qpnp-qg-y	+= qpnp-qg.o battery-profile-loader.o pmic-voter.o qg-util.o qg-soc.o qg-sdam.o qg-battery-profile.o qg-profile-lib.o fg-alg.o
obj-$(CONFIG_SMB1398_CHARGER)           += qcom-smb1398-charger.o
qcom-smb1398-charger-y += smb1398-charger.o pmic-voter.o
obj-$(CONFIG_QPNP_SMBLITE) += qpnp-smblite-main.o
qpnp-smblite-main-y += step-chg-jeita.o battery.o qpnp-smblite.o smblite-lib.o pmic-voter.o storm-watch.o battery-profile-loader.o schgm-flashlite.o smblite-iio.o
obj-$(CONFIG_SMB1355_SLAVE_CHARGER)	+= qcom-smb1355-charger.o
qcom-smb1355-charger-y += smb1355-charger.o pmic-voter.o

# req,bin,20201106,add,begin
$(warning "Compile BUILD_SMT $(BUILD_SMT)")
ifeq ($(BUILD_SMT),true)
subdir-ccflags-y += -DSMT_VERSION
endif

$(warning "Compile BUILD_DUAL_85 $(BUILD_DUAL_85)")
ifeq ($(BUILD_DUAL_85),true)
subdir-ccflags-y += -DDUAL_85_VERSION
endif

# subdir-ccflags-y += -DDEBUG_CHARGER

# req,bin,20201106,add,end

#zhongli.liu add for thething bring up 20210201 start
ifeq ($(BUILD_THETHING_PROJECT),true)
 subdir-ccflags-y += -DBUILD_THETHING_PROJECT
endif


ifeq ($(BUILD_PUNISHER_PROJECT),true)
 subdir-ccflags-y += -DTARGET_PRODUCT_PUNISHER
endif


obj-$(CONFIG_CHARGER_IDT_P9415)        += qcom_idtp9415_wireless_power.o
qcom_idtp9415_wireless_power-y += idtp9415_wireless_power.o pmic-voter.o

obj-$(CONFIG_SMB1351_USB_CHARGER)	+= smb1351-charger.o pmic-voter.o
obj-$(CONFIG_SMB1390_CHARGE_PUMP_PSY)   += qcom-smb1390-charger.o
qcom-smb1390-charger-y += smb1390-charger-psy.o pmic-voter.o
obj-$(CONFIG_QTI_QBG)	+= qti-qbg-main.o
qti-qbg-main-y	+= qti-qbg.o qbg-sdam.o qbg-battery-profile.o battery-profile-loader.o
