#C80 for factory test; C00 for TP
FTS_FIRMWARE_PATH:=kernel/msm-5.4/drivers/input/touchscreen/HX83113A/firmware/
PRODUCT_COPY_FILES += $(FTS_FIRMWARE_PATH)/CZ3_sorting_himax_tianma_CIDFF0D_D00_C81_1231-015240.bin:vendor/firmware/Himax_mpfw.bin
PRODUCT_COPY_FILES += $(FTS_FIRMWARE_PATH)/CZ3_Chino_Nokia_CID0701_D00_C03_0302-042808.bin:vendor/firmware/Himax_firmware.bin
