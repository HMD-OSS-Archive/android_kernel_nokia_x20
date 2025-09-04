/************************************************************************
* Copyright (C) 2012-2019, Focaltech Systems (R)£¬All Rights Reserved.
*
* File Name: focaltech_flash.h
*
* Author: Focaltech Driver Team
*
* Created: 2016-08-07
*
* Abstract:
*
************************************************************************/
#ifndef __LINUX_FOCALTECH_FLASH_H__
#define __LINUX_FOCALTECH_FLASH_H__

/*****************************************************************************
* 1.Included header files
*****************************************************************************/
#include "focaltech_core.h"

/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/
#define FT8719_FTS_CMD_RESET                               0x07
#define FT8719_FTS_ROMBOOT_CMD_SET_PRAM_ADDR               0xAD
#define FT8719_FTS_ROMBOOT_CMD_SET_PRAM_ADDR_LEN           4
#define FT8719_FTS_ROMBOOT_CMD_WRITE                       0xAE
#define FT8719_FTS_ROMBOOT_CMD_START_APP                   0x08
#define FT8719_FTS_DELAY_PRAMBOOT_START                    10
#define FT8719_FTS_ROMBOOT_CMD_ECC                         0xCC
#define FT8719_FTS_PRAM_SADDR                              0x000000
#define FT8719_FTS_DRAM_SADDR                              0xD00000

#define FT8719_FTS_CMD_READ                                0x03
#define FT8719_FTS_CMD_READ_DELAY                          1
#define FT8719_FTS_CMD_READ_LEN                            4
#define FT8719_FTS_CMD_FLASH_TYPE                          0x05
#define FT8719_FTS_CMD_FLASH_MODE                          0x09
#define FT8719_FLASH_MODE_WRITE_FLASH_VALUE                0x0A
#define FT8719_FLASH_MODE_UPGRADE_VALUE                    0x0B
#define FT8719_FLASH_MODE_LIC_VALUE                        0x0C
#define FT8719_FLASH_MODE_PARAM_VALUE                      0x0D
#define FT8719_FTS_CMD_ERASE_APP                           0x61
#define FT8719_FTS_REASE_APP_DELAY                         1350
#define FT8719_FTS_ERASE_SECTOR_DELAY                      60
#define FT8719_FT8719_FTS_RETRIES_REASE                           50
#define FT8719_FT8719_FTS_RETRIES_DELAY_REASE                     200
#define FT8719_FTS_CMD_FLASH_STATUS                        0x6A
#define FT8719_FTS_CMD_FLASH_STATUS_LEN                    2
#define FT8719_FTS_CMD_FLASH_STATUS_NOP                    0x0000
#define FT8719_FTS_CMD_FLASH_STATUS_ECC_OK                 0xF055
#define FT8719_FTS_CMD_FLASH_STATUS_ERASE_OK               0xF0AA
#define FT8719_FTS_CMD_FLASH_STATUS_WRITE_OK               0x1000
#define FT8719_FTS_CMD_ECC_INIT                            0x64
#define FT8719_FTS_CMD_ECC_CAL                             0x65
#define FT8719_FTS_CMD_ECC_CAL_LEN                         6
#define FT8719_FTS_RETRIES_ECC_CAL                         10
#define FT8719_FTS_RETRIES_DELAY_ECC_CAL                   50
#define FT8719_FTS_CMD_ECC_READ                            0x66
#define FT8719_FTS_CMD_DATA_LEN                            0xB0
#define FT8719_FTS_CMD_APP_DATA_LEN_INCELL                 0x7A
#define FT8719_FTS_CMD_DATA_LEN_LEN                        4
#define FT8719_FTS_CMD_WRITE                               0xBF
#define FT8719_FTS_RETRIES_WRITE                           100
#define FT8719_FTS_RETRIES_DELAY_WRITE                     1
#define FT8719_FTS_CMD_WRITE_LEN                           6
#define FT8719_FTS_DELAY_READ_ID                           20
#define FT8719_FTS_DELAY_UPGRADE_RESET                     80
#define FT8719_PRAMBOOT_MIN_SIZE                           0x120
#define FT8719_PRAMBOOT_MAX_SIZE                           (64*1024)
#define FT8719_FTS_FLASH_PACKET_LENGTH                     32     /* max=128 */
#define FT8719_FTS_MAX_LEN_ECC_CALC                        0xFFFE /* must be even */
#define FT8719_FTS_MIN_LEN                                 0x120
#define FT8719_FTS_MAX_LEN_FILE                            (128 * 1024)
#define FT8719_FTS_MAX_LEN_APP                             (64 * 1024)
#define FT8719_FTS_MAX_LEN_SECTOR                          (4 * 1024)
#define FT8719_FTS_CONIFG_VENDORID_OFF                     0x04
#define FT8719_FTS_CONIFG_MODULEID_OFF                     0x1E
#define FT8719_FTS_CONIFG_PROJECTID_OFF                    0x20
#define FT8719_FTS_APPINFO_OFF                             0x100
#define FT8719_FTS_APPINFO_APPLEN_OFF                      0x00
#define FT8719_FTS_APPINFO_APPLEN2_OFF                     0x12
#define FT8719_FTS_REG_UPGRADE                             0xFC
#define FT8719_FTS_REG_UPGRADE2                            0xBC
#define FT8719_FTS_UPGRADE_AA                              0xAA
#define FT8719_FTS_UPGRADE_55                              0x55
#define FT8719_FTS_DELAY_UPGRADE_AA                        10
#define FT8719_FTS_UPGRADE_LOOP                            30
#define FT8719_FTS_HEADER_LEN                              32
#define FT8719_FTS_FW_BIN_FILEPATH                         "/sdcard/"
#define FT8719_FTS_FW_IDE_SIG                              "IDE_"
#define FT8719_FTS_FW_IDE_SIG_LEN                          4
#define FT8719_MAX_MODULE_VENDOR_NAME_LEN                  16

#define FT8719_FTS_ROMBOOT_CMD_ECC_NEW_LEN                 7
#define FT8719_FTS_ECC_FINISH_TIMEOUT                      100
#define FT8719_FTS_ROMBOOT_CMD_ECC_FINISH                  0xCE
#define FT8719_FTS_ROMBOOT_CMD_ECC_FINISH_OK_A5            0xA5
#define FT8719_FTS_ROMBOOT_CMD_ECC_FINISH_OK_00            0x00
#define FT8719_FTS_ROMBOOT_CMD_ECC_READ                    0xCD
#define FT8719_AL2_FCS_COEF                ((1 << 15) + (1 << 10) + (1 << 3))

#define FT8719_FTS_APP_INFO_OFFSET                         0x100

enum FT8719_FW_STATUS {
    FTS_RUN_IN_ERROR,
    FTS_RUN_IN_APP,
    FTS_RUN_IN_ROM,
    FTS_RUN_IN_PRAM,
    FTS_RUN_IN_BOOTLOADER,
};

enum FT8719_FW_FLASH_MODE {
    FLASH_MODE_APP,
    FLASH_MODE_LIC,
    FLASH_MODE_PARAM,
    FLASH_MODE_ALL,
};

enum FT8719_ECC_CHECK_MODE {
    FT8719_ECC_CHECK_MODE_XOR,
    FT8719_ECC_CHECK_MODE_CRC16,
};

/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/
/* IC info */
struct ft8719_upgrade_func {
    u64 ctype[FT8719_FTX_MAX_COMPATIBLE_TYPE];
    u32 fwveroff;
    u32 fwcfgoff;
    u32 appoff;
    u32 licoff;
    u32 paramcfgoff;
    u32 paramcfgveroff;
    u32 paramcfg2off;
    int pram_ecc_check_mode;
    int fw_ecc_check_mode;
    bool new_return_value_from_ic;
    bool appoff_handle_in_ic;
    bool is_reset_register_BC;
    bool read_boot_id_need_reset;
    bool hid_supported;
    bool pramboot_supported;
    u8 *pramboot;
    u32 pb_length;
    int (*init)(u8 *, u32);
    int (*upgrade)(u8 *, u32);
    int (*get_hlic_ver)(u8 *);
    int (*lic_upgrade)(u8 *, u32);
    int (*param_upgrade)(u8 *, u32);
    int (*force_upgrade)(u8 *, u32);
};

struct ft8719_upgrade_setting_nf {
    u8 rom_idh;
    u8 rom_idl;
    u16 reserved;
    u32 app2_offset;
    u32 ecclen_max;
    u8 eccok_val;
    u8 upgsts_boot;
    u8 delay_init;
    bool spi_pe;
    bool half_length;
    bool fd_check;
    bool drwr_support;
};

struct ft8719_upgrade_module {
    int id;
    char vendor_name[FT8719_MAX_MODULE_VENDOR_NAME_LEN];
    u8 *ft8719_fw_file;
    u32 fw_len;
};

struct ft8719_fts_upgrade {
    struct ft8719_fts_ts_data *ts_data;
    struct ft8719_upgrade_module *module_info;
    struct ft8719_upgrade_func *func;
    struct ft8719_upgrade_setting_nf *setting_nf;
    int module_id;
    bool fw_from_request;
    u8 *fw;
    u32 fw_length;
    u8 *lic;
    u32 lic_length;
};

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/

/*****************************************************************************
* Static function prototypes
*****************************************************************************/
#endif
