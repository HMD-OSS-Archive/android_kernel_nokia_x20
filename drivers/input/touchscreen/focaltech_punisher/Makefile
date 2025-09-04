# Makefile for the focaltech touchscreen drivers.

obj-$(CONFIG_TOUCHSCREEN_FTS) += focaltech_fts.o
focaltech_fts-y := \
	focaltech_core.o \
	focaltech_ex_fun.o \
	focaltech_ex_mode.o \
	focaltech_gesture.o \
	focaltech_esdcheck.o \
	focaltech_point_report_check.o \
	focaltech_flash.o \
	focaltech_spi.o


ifeq ($(BUILD_PUNISHER_PROJECT),true)
 subdir-ccflags-y += -DTARGET_PRODUCT_PUNISHER
endif

