
ccflags-y += -D__FF_BUILD_DATE=\"$(shell date +%Y%m%d)\"

# Early-logging level for ff_hal, ff_ta and driver as well. Note that this
# logging level only takes effect before ff_config module is loaded.
#
# Options:
# 0 - FF_LOG_LEVEL_ALL, All
# 1 - FF_LOG_LEVEL_VBS, Verbose
# 2 - FF_LOG_LEVEL_DBG, Debug
# 3 - FF_LOG_LEVEL_INF, Info
# 4 - FF_LOG_LEVEL_WRN, Warning
# 5 - FF_LOG_LEVEL_ERR, Error
ccflags-y += -D__FF_EARLY_LOG_LEVEL=2

# Source files.
$(info "zhouhe focal FP config is $(CONFIG_FOCALTECH_FINGERPRINT) ")
obj-$(CONFIG_FOCALTECH_FINGERPRINT) += focaltech_fp.o

$(info "zhouhe focal FP 1")
focaltech_fp-objs := ff_ctl.o plat-msm8916.o focaltech_fp.o

$(info "zhouhe focal FP 2")
ifeq ($(CONFIG_FOCALTECH_FINGERPRINT),m)

$(info "zhouhe focal FP 3")
BOARD_VENDOR_KERNEL_MODULES +=  $(KERNEL_MODULES_OUT)/focaltech_fp.ko
endif

