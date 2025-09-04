/*
 * Copyright 2012 Texas Instruments
 *
 * Author: Milo(Woogyom) Kim <milo.kim@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __WL2864C_REGULATOR_H__
#define __WL2864C_REGULATOR_H__

#include <linux/regulator/machine.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#define WL2864C_MAX_REGULATORS		7

enum wl2864c_regulator_id {
	WL2864C_ID_BASE,
	WL2864C_ID_LDO1 = WL2864C_ID_BASE,
	WL2864C_ID_LDO2,
	WL2864C_ID_LDO3,
	WL2864C_ID_LDO4,
	WL2864C_ID_LDO5,
	WL2864C_ID_LDO6,
	WL2864C_ID_LDO7,

	WL2864C_ID_MAX,
};

/**
 * wl2864c_regdata
 * @id        : regulator id
 * @init_data : init data for each regulator
 */
struct wl2864c_regulator_data {
	enum wl2864c_regulator_id id;
	struct regulator_init_data *init_data;
};

/**
 * wl2864c_platform_data
 * @regulator_data    : platform regulator id and init data
 */
struct wl2864c_platform_data {
	struct wl2864c_regulator_data regulator_data[WL2864C_MAX_REGULATORS];
};

#endif
