himax_mmi-y := \
  himax_debug.o \
  himax_inspection.o \
  himax_ic_incell_core.o \
  himax_ic_HX83113.o \
  himax_common.o \
  himax_platform.o 

 
obj-$(CONFIG_TOUCHSCREEN_HIMAX_COMMON) += himax_mmi.o


ifeq ($(CONFIG_TOUCHSCREEN_HIMAX_COMMON),m)
$(info "Himax hx83113a IS BUILT AS DLKM")
BOARD_VENDOR_KERNEL_MODULES +=  $(KERNEL_MODULES_OUT)/himax_mmi.ko
endif

