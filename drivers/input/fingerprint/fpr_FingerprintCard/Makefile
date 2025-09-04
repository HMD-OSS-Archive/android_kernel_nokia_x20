# SPDX-License-Identifier: GPL-2.0-only

# Makefile for FingerprintCard fingerprint driver

#fpc1020-objs := fpc1020_platform_tee.o
#obj-$(CONFIG_FPR_FPC) += fpc1020.o
#obj-y += fpc1020.o
#obj-$(CONFIG_FPR_FPC) += fpc1020_platform_tee.o
#obj-y += fpc1020_platform_tee.o

fpc1020-y := fpc1020_platform_tee.o
obj-$(CONFIG_FPR_FPC) += fpc1020.o
ifeq ($(CONFIG_FPR_FPC),m)
$(info "--------fpc1020 IS BUILT AS DLKM")
$(info "--------KERNEL_MODULES_OUT=$(KERNEL_MODULES_OUT)")
$(info "--------BOARD_VENDOR_KERNEL_MODULES=$(BOARD_VENDOR_KERNEL_MODULES)")
BOARD_VENDOR_KERNEL_MODULES +=  $(KERNEL_MODULES_OUT)/fpc1020.ko
endif
