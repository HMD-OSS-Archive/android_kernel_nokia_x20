#
# Makefile for all kinds of Fingerprint
#
#obj-$(CONFIG_CDFINGER_FP) += cdfingerfp/ 
#obj-y += cdfingerfp/

ifeq ($(BUILD_THETHING_PROJECT),true)

obj-y += goodix/

else
#obj-$(CONFIG_CHIPONE_FINGERPRINT) += fpsensor/
obj-y += fpsensor/

#obj-$(CONFIG_FPR_FPC) += fpr_FingerprintCard/
obj-y += fpr_FingerprintCard/

endif
