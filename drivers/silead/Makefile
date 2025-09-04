#
# Makefile for kernel Silead Fingerprint drivers.
#

ifdef CONFIG_ARCH_MSM
	 ccflags-y += -DBSP_SIL_PLAT_QCOM
endif

ifdef CONFIG_ARCH_QCOM
	 ccflags-y += -DBSP_SIL_PLAT_QCOM
endif

$(info "zhouhe silead FP config is $(CONFIG_SILEAD_FP_PLATFORM) ")
obj-$(CONFIG_SILEAD_FP_PLATFORM)+= silead_fp_platform.o

#obj-y	+= silead_fp_platform.o

$(info "zhouhe silead FP 2")
ifeq ($(CONFIG_SILEAD_FP_PLATFORM),m)

$(info "zhouhe silead FP 3")
BOARD_VENDOR_KERNEL_MODULES +=  $(KERNEL_MODULES_OUT)/silead_fp_platform.ko
endif
 
