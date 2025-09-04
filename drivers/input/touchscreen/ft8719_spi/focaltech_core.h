/*
 *
 * FocalTech TouchScreen driver.
 *
 * Copyright (c) 2012-2019, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*****************************************************************************
*
* File Name: focaltech_core.h

* Author: Focaltech Driver Team
*
* Created: 2016-08-08
*
* Abstract:
*
* Reference:
*
*****************************************************************************/

#ifndef __LINUX_FOCALTECH_CORE_H__
#define __LINUX_FOCALTECH_CORE_H__
/*****************************************************************************
* Included header files
*****************************************************************************/
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/firmware.h>
#include <linux/debugfs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include "focaltech_common.h"
#include "../ts_func_test.h"

/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/
#define FT8719_FTS_MAX_POINTS_SUPPORT              10 /* constant value, can't be changed */
#define FT8719_FTS_MAX_KEYS                        4
#define FT8719_FTS_KEY_DIM                         10
#define FT8719_FTS_ONE_TCH_LEN                     6
#define FT8719_FTS_TOUCH_DATA_LEN  (FT8719_FTS_MAX_POINTS_SUPPORT * FT8719_FTS_ONE_TCH_LEN + 3)

#define FT8719_FTS_GESTURE_POINTS_MAX              6
#define FT8719_FTS_GESTURE_DATA_LEN               (FT8719_FTS_GESTURE_POINTS_MAX * 4 + 4)

#define FT8719_FTS_MAX_ID                          0x0A
#define FT8719_FTS_TOUCH_X_H_POS                   3
#define FT8719_FTS_TOUCH_X_L_POS                   4
#define FT8719_FTS_TOUCH_Y_H_POS                   5
#define FT8719_FTS_TOUCH_Y_L_POS                   6
#define FT8719_FTS_TOUCH_PRE_POS                   7
#define FT8719_FTS_TOUCH_AREA_POS                  8
#define FT8719_FTS_TOUCH_POINT_NUM                 2
#define FT8719_FTS_TOUCH_EVENT_POS                 3
#define FT8719_FTS_TOUCH_ID_POS                    5
#define FT8719_FTS_COORDS_ARR_SIZE                 4
#define FT8719_FTS_X_MIN_DISPLAY_DEFAULT           0
#define FT8719_FTS_Y_MIN_DISPLAY_DEFAULT           0
#define FT8719_FTS_X_MAX_DISPLAY_DEFAULT           1080
#define FT8719_FTS_Y_MAX_DISPLAY_DEFAULT           2340

#define FT8719_FTS_TOUCH_DOWN                      0
#define FT8719_FTS_TOUCH_UP                        1
#define FT8719_FTS_TOUCH_CONTACT                   2
#define FT8719_EVENT_DOWN(flag)                    ((FT8719_FTS_TOUCH_DOWN == flag) || (FT8719_FTS_TOUCH_CONTACT == flag))
#define FT8719_EVENT_UP(flag)                      (FT8719_FTS_TOUCH_UP == flag)
#define FT8719_EVENT_NO_DOWN(data)                 (!data->point_num)

#define FT8719_FTX_MAX_COMPATIBLE_TYPE             4
#define FT8719_FTX_MAX_COMMMAND_LENGTH             16

#if defined(CONFIG_TOUCHSCREEN_FT8719_SPI_GESTURE)
#define FT8719_FTS_GESTURE_POINTS_NUM		128
#define FT8719_MAX_GESTURE					10
#endif /* CONFIG_TOUCHSCREEN_FT8719_SPI_GESTURE */
/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/
struct ft8719_ftxxxx_proc {
    struct proc_dir_entry *proc_entry;
    u8 opmode;
    u8 cmd_len;
    u8 cmd[FT8719_FTX_MAX_COMMMAND_LENGTH];
};

struct ft8719_fts_ts_platform_data {
    u32 irq_gpio;
    u32 irq_gpio_flags;
    u32 reset_gpio;
    u32 reset_gpio_flags;
    bool have_key;
    u32 key_number;
    u32 keys[FT8719_FTS_MAX_KEYS];
    u32 key_y_coords[FT8719_FTS_MAX_KEYS];
    u32 key_x_coords[FT8719_FTS_MAX_KEYS];
    u32 x_max;
    u32 y_max;
    u32 x_min;
    u32 y_min;
    u32 max_touch_number;
#ifdef CONFIG_FT_8719_INCELL_CHIP
	bool ft8719_keep_lcd_suspend_reset_high;
#endif
};

struct ft8719_ts_event {
    int x;      /*x coordinate */
    int y;      /*y coordinate */
    int p;      /* pressure */
    int flag;   /* touch event flag: 0 -- down; 1-- up; 2 -- contact */
    int id;     /*touch ID */
    int area;
};

struct ft8719_fts_ts_data {
    struct i2c_client *client;
    struct spi_device *spi;
    struct device *dev;
    struct input_dev *input_dev;
    struct ft8719_fts_ts_platform_data *pdata;
    struct ts_ic_info ic_info;
    struct workqueue_struct *ts_workqueue;
	struct delayed_work fwupg_delay_work;
    struct work_struct fwupg_work;
    struct delayed_work esdcheck_work;
    struct delayed_work prc_work;
    struct work_struct resume_work;
    struct ft8719_ftxxxx_proc proc;
    spinlock_t irq_lock;
    struct mutex report_mutex;
    struct mutex bus_lock;
    int irq;
    int log_level;
    int fw_is_running;      /* confirm fw is running when using spi:default 0 */
    int dummy_byte;
    bool suspended;
    bool fw_loading;
    bool irq_disabled;
    bool power_disabled;
    bool glove_mode;
    bool cover_mode;
    bool charger_mode;
    bool gesture_mode;      /* gesture enable or disable, default: disable */
    /* multi-touch */
    struct ft8719_ts_event *events;
    u8 *bus_tx_buf;
    u8 *bus_rx_buf;
    u8 *point_buf;
    int pnt_buf_size;
    int touchs;
    int key_state;
    int touch_point;
    int point_num;
    struct regulator *vdd;
    struct regulator *vcc_i2c;
#if FT8719_FTS_PINCTRL_EN
    struct pinctrl *pinctrl;
    struct pinctrl_state *pins_active;
    struct pinctrl_state *pins_suspend;
    struct pinctrl_state *pins_release;
#endif
#if defined(CONFIG_FB)
    struct notifier_block fb_notif;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend early_suspend;
#endif
	struct ts_func_test_device ts_test_dev;
	u8 fw_ver[3];
	u8 module_id;
#if defined(CONFIG_TOUCHSCREEN_FT8719_SPI_GESTURE)
	bool gesture_en;
	bool gesture_suspend_en;
	u16 gesture_track_x[FT8719_FTS_GESTURE_POINTS_NUM];
	u16 gesture_track_y[FT8719_FTS_GESTURE_POINTS_NUM];
	short gesture_track_pointnum;
	unsigned int gesture_state;
	u32 gesture_func_map[10];
	u32 gesture_figure_map[10];
	u32 gesture_num;
#endif/*CONFIG_TOUCHSCREEN_FT8719_SPI_GESTURE*/
#if defined(CONFIG_TOUCHSCREEN_FT8719_SPI_GLOVE)
	bool glove_enable;
#endif
	struct workqueue_struct *probe_workqueue;
	struct delayed_work probe_delay_work;
};

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/
extern struct ft8719_fts_ts_data *ft8719_fts_data;
extern int ft8719_factory_get_module_id(struct device *dev, char *buf);
extern int ft8719_factory_get_fs_fw_version(struct device *dev, char *buf);
//extern int factory_get_diff(struct device *dev, char *buf);

/* communication interface */
int ft8719_fts_read(u8 *cmd, u32 cmdlen, u8 *data, u32 datalen);
int ft8719_fts_read_reg(u8 addr, u8 *value);
int ft8719_fts_write(u8 *writebuf, u32 writelen);
int ft8719_fts_write_reg(u8 addr, u8 value);
void ft8719_fts_hid2std(void);
int ft8719_fts_bus_init(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_bus_exit(struct ft8719_fts_ts_data *ts_data);

/* Gesture functions */
#ifdef CONFIG_TOUCHSCREEN_FT8719_SPI_GESTURE
int ft8719_fts_gesture_init(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_gesture_exit(struct ft8719_fts_ts_data *ts_data);
void ft8719_fts_gesture_recovery(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_gesture_readdata(struct ft8719_fts_ts_data *ts_data, u8 *data);
int ft8719_fts_gesture_suspend(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_gesture_resume(struct ft8719_fts_ts_data *ts_data);
#endif/*CONFIG_TOUCHSCREEN_FT8719_SPI_GESTURE*/

/* Apk and functions */
int ft8719_fts_create_apk_debug_channel(struct ft8719_fts_ts_data *);
void ft8719_fts_release_apk_debug_channel(struct ft8719_fts_ts_data *);

/* ADB functions */
int ft8719_fts_create_sysfs(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_remove_sysfs(struct ft8719_fts_ts_data *ts_data);

/* ESD */
#if FT8719_FTS_ESDCHECK_EN
int ft8719_fts_esdcheck_init(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_esdcheck_exit(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_esdcheck_switch(bool enable);
int ft8719_fts_esdcheck_proc_busy(bool proc_debug);
int ft8719_fts_esdcheck_set_intr(bool intr);
int ft8719_fts_esdcheck_suspend(void);
int ft8719_fts_esdcheck_resume(void);
#endif


/* Point Report Check*/
#if FT8719_FTS_POINT_REPORT_CHECK_EN
int ft8719_fts_point_report_check_init(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_point_report_check_exit(struct ft8719_fts_ts_data *ts_data);
void ft8719_fts_prc_queue_work(struct ft8719_fts_ts_data *ts_data);
#endif

/* FW upgrade */
int ft8719_fts_fwupg_init(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_fwupg_exit(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_fw_resume(void);
int ft8719_fts_fw_recovery(void);
int ft8719_fts_upgrade_bin(char *fw_name, bool force);
int ft8719_fts_enter_test_environment(bool test_state);

/* Other */
int ft8719_fts_reset_proc(int hdelayms);
int ft8719_fts_wait_tp_to_valid(void);
void ft8719_fts_release_all_finger(void);
void ft8719_fts_tp_state_recovery(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_ex_mode_init(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_ex_mode_exit(struct ft8719_fts_ts_data *ts_data);
int ft8719_fts_ex_mode_recovery(struct ft8719_fts_ts_data *ts_data);

void ft8719_fts_irq_disable(void);
void ft8719_fts_irq_enable(void);
#endif /* __LINUX_FOCALTECH_CORE_H__ */
