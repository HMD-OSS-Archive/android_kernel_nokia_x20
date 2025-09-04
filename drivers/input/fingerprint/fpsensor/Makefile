#obj-$(CONFIG_CHIPONE_FINGERPRINT) += fpsensor_spi_tee.o
#obj-y += fpsensor_spi_tee.o


fpsensor-y := fpsensor_spi_tee.o
obj-$(CONFIG_CHIPONE_FINGERPRINT) += fpsensor.o
ifeq ($(CONFIG_CHIPONE_FINGERPRINT),m)
$(info "fpsensor IS BUILT AS DLKM")
BOARD_VENDOR_KERNEL_MODULES +=  $(KERNEL_MODULES_OUT)/fpsensor.ko
endif







