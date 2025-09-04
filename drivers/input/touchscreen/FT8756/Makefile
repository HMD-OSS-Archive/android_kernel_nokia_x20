# Makefile for the focaltech touchscreen drivers.

ft8756_fts-y := \
  focaltech_core.o \
  focaltech_ex_fun.o \
  focaltech_ex_mode.o \
  focaltech_gesture.o \
  focaltech_esdcheck.o \
  focaltech_point_report_check.o \
  focaltech_spi.o \
  focaltech_flash.o

ft8756_fts-$(CONFIG_QGKI) += \
  focaltech_test/focaltech_test.o \
  focaltech_test/focaltech_test_ini.o \
  focaltech_test/supported_ic/focaltech_test_ft8756.o

obj-$(CONFIG_TOUCHSCREEN_FT8756) += ft8756_fts.o

ifeq ($(CONFIG_TOUCHSCREEN_FT8756),m)
$(info "Focaltech FT8756 IS BUILT AS DLKM")
BOARD_VENDOR_KERNEL_MODULES +=  $(KERNEL_MODULES_OUT)/ft8756_fts.ko
endif

