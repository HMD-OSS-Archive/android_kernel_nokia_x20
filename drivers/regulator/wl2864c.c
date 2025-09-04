/*
 * Copyright 2012 ontim
 *
 * Author: liman <man.li@ontim.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/regulator/driver.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/of_regulator.h>

#include <linux/regulator/wl2864c.h>

#undef DEBUG
//#define DEBUG

/* Registers : WL2864C shared */
#define WL2864C_REG_CHIP_ID			0x00

#define WL2864C_LDO1_VOUT		0x03
#define WL2864C_LDO2_VOUT		0x04
#define WL2864C_LDO3_VOUT		0x05
#define WL2864C_LDO4_VOUT		0x06
#define WL2864C_LDO5_VOUT		0x07
#define WL2864C_LDO6_VOUT		0x08
#define WL2864C_LDO7_VOUT		0x09

#define WL2864C_ENABLE			0x0E

/* Mask/shift : WL2864C shared */
#define WL2864C_VOUT_VTB1_M			0xFC  //sel << (4-1)
#define WL2864C_VOUT_VTB2_M			0xF8   //sel << (4-1)

#define WL2864C_EN_LDO1_M		BIT(0)
#define WL2864C_EN_LDO2_M		BIT(1)
#define WL2864C_EN_LDO3_M		BIT(2)
#define WL2864C_EN_LDO4_M		BIT(3)
#define WL2864C_EN_LDO5_M		BIT(4)
#define WL2864C_EN_LDO6_M		BIT(5)
#define WL2864C_EN_LDO7_M		BIT(6)

#define WL2864C_CHIP_ID			0x01

#define WL2864C_NUM_REGULATORS		7

#define MAX_REGISTERS			0x0F

struct wl2864c {
	struct regmap *regmap;
	struct device *dev;
	struct wl2864c_platform_data *pdata;
	
  	struct pinctrl			*pinctrl;
  	struct pinctrl_state		*gpio_state_active;
  	struct pinctrl_state		*gpio_state_suspend;
	
	int num_regulators;
};

/* LP8720/LP8725 shared voltage table for LDOs */
static const unsigned int wl2864c_ldo_vtbl[] = {
	600000, 650000, 700000, 750000, 800000, 850000, 900000, 950000, 1000000, 1050000, 
	1100000, 1150000, 1200000, 1250000, 1300000, 1350000, 1400000, 1450000, 1500000, 1550000, 
	1600000, 1650000, 1700000, 1750000, 1800000, 1850000, 1900000, 1950000, 2000000, 2050000, 
	2100000, 2150000, 2200000, 2250000, 2300000, 2350000, 2400000, 2450000, 2500000, 2550000, 
	2600000, 2650000, 2700000, 2750000, 2800000, 2850000, 2900000, 2950000, 3000000, 
};

/* LP8720 LDO4 voltage table */
static const unsigned int wl2864c_ldo3_vtbl[] = {
	1200000, 1300000, 1400000, 1500000, 1600000,
	1700000, 1800000, 1900000, 2000000, 2100000, 
	2200000, 2300000, 2400000, 2500000, 2600000, 
	2700000, 2800000, 2900000, 3000000, 3100000, 
	3200000, 3300000, 3400000, 3500000, 3600000, 
	3700000, 3800000, 3900000, 4000000, 4100000, 
	4200000, 4300000, 
};

static int wl2864c_read_byte(struct wl2864c *lp, u8 addr, u8 *data)
{
	int ret;
	unsigned int val;

	ret = regmap_read(lp->regmap, addr, &val);
	if (ret < 0) {
		dev_err(lp->dev, "failed to read 0x%.2x\n", addr);
		return ret;
	}

	*data = (u8)val;
	return 0;
}

static inline int wl2864c_write_byte(struct wl2864c *lp, u8 addr, u8 data)
{
	return regmap_write(lp->regmap, addr, data);
}

static inline int wl2864c_update_bits(struct wl2864c *lp, u8 addr,
				unsigned int mask, u8 data)
{
	return regmap_update_bits(lp->regmap, addr, mask, data);
}

static int mach_chip_id(struct wl2864c *lp){
	u8 id = 0;
	int ret;
	ret = wl2864c_read_byte(lp, WL2864C_REG_CHIP_ID, &id);
	
	if(ret){
		dev_err(lp->dev, "failed to read chip id\n");
		return ret;
	}
	if(id != WL2864C_CHIP_ID){
		dev_err(lp->dev, "error chipid: 0x%x!=0x%x(WL2864C_CHIP_ID)\n", id, WL2864C_CHIP_ID);
		return -1;
	}
	return 0;
}

#ifdef DEBUG

int wl2864c_is_enabled_regmap(struct regulator_dev *rdev)
{
	unsigned int val;
	int ret;

	ret = regmap_read(rdev->regmap, rdev->desc->enable_reg, &val);
	//dev_err(&rdev->dev, "wl2864c: %s reg(0x%x)=0x%x (mask:0x%x)\n", rdev->desc->name, rdev->desc->enable_reg, val, rdev->desc->enable_mask);
	if (ret != 0)
		return ret;

	val &= rdev->desc->enable_mask;

	if (rdev->desc->enable_is_inverted) {
		if (rdev->desc->enable_val)
			return val != rdev->desc->enable_val;
		return val == 0;
	} else {
		if (rdev->desc->enable_val)
			return val == rdev->desc->enable_val;
		return val != 0;
	}
}

int wl2864c_get_voltage_sel_regmap(struct regulator_dev *rdev)
{
	unsigned int val;
	int ret;

	ret = regmap_read(rdev->regmap, rdev->desc->vsel_reg, &val);
	dev_dbg(&rdev->dev, "get_voltage_sel %s:val:%d(0x%x) mask:0x%x\n",rdev->desc->name, val, val, rdev->desc->vsel_mask);
	
	if (ret != 0)
		return ret;

	val &= rdev->desc->vsel_mask;
	val >>= ffs(rdev->desc->vsel_mask) - 1;

	dev_dbg(&rdev->dev, "get_voltage_sel %s:val:%d(0x%x)\n",rdev->desc->name, val, val);
	return val;
}

int wl2864c_set_voltage_sel_regmap1(struct regulator_dev *rdev, unsigned sel)
{
	int ret;
	
	sel <<= ffs(rdev->desc->vsel_mask) - 1;
	dev_dbg(&rdev->dev, "set_voltage_sel---: sel:%d--%d(%s) mask:0x%x", sel, ffs(rdev->desc->vsel_mask), rdev->desc->name, rdev->desc->vsel_mask);	
	
	ret = regmap_update_bits(rdev->regmap, rdev->desc->vsel_reg,
				  rdev->desc->vsel_mask, sel);
	if (ret)
		return ret;

	if (rdev->desc->apply_bit)
		ret = regmap_update_bits(rdev->regmap, rdev->desc->apply_reg,
					 rdev->desc->apply_bit,
					 rdev->desc->apply_bit);
	return ret;
}

int wl2864c_set_voltage_sel_regmap(struct regulator_dev *rdev, unsigned sel)
{	
	int ret = 0;
	unsigned int val;
	dev_dbg(&rdev->dev, "set_voltage_sel: sel:%d(%s)", sel, rdev->desc->name);
	ret = wl2864c_set_voltage_sel_regmap1(rdev, sel);
	if(ret){
		dev_err(&rdev->dev, "set_voltage_sel: sel %d(%s) failed", sel, rdev->desc->name);		
	}
	ret = regmap_read(rdev->regmap, rdev->desc->vsel_reg, &val);
	dev_dbg(&rdev->dev, "wl2864c: set_voltage_sel %s reg(0x%x)=%d (mask:0x%x)\n", rdev->desc->name, rdev->desc->vsel_reg, val, rdev->desc->vsel_mask);
	if (ret != 0)
		return ret;
	
	return ret;
}

int wl2864c_enable_regmap(struct regulator_dev *rdev)
{	
	int ret = 0;
	unsigned int val;
	dev_dbg(&rdev->dev, "enable ldo: %s", rdev->desc->name);
	ret = regulator_enable_regmap(rdev);
	
	ret = regmap_read(rdev->regmap, rdev->desc->enable_reg, &val);
	dev_dbg(&rdev->dev, "wl2864c: %s reg(0x%x)=0x%x (mask:0x%x)\n", rdev->desc->name, rdev->desc->enable_reg, val, rdev->desc->enable_mask);
	if (ret != 0)
		return ret;
	
	return ret;
}

int wl2864c_disable_regmap(struct regulator_dev *rdev)
{	
	unsigned int val;
	int ret = 0;
	dev_dbg(&rdev->dev, "disable ldo: %s", rdev->desc->name);
	ret = regulator_disable_regmap(rdev);
	
	ret = regmap_read(rdev->regmap, rdev->desc->enable_reg, &val);
	dev_dbg(&rdev->dev, "wl2864c: %s reg(0x%x)=0x%x (mask:0x%x)\n", rdev->desc->name, rdev->desc->enable_reg, val, rdev->desc->enable_mask);
	if (ret != 0)
		return ret;
	return ret;
}

static struct regulator_ops wl2864c_ldo_ops = {
	.list_voltage = regulator_list_voltage_table,
	.map_voltage = regulator_map_voltage_ascend,
	.set_voltage_sel = wl2864c_set_voltage_sel_regmap,
	.get_voltage_sel = wl2864c_get_voltage_sel_regmap,
	.enable = wl2864c_enable_regmap,
	.disable = wl2864c_disable_regmap,
	.is_enabled = wl2864c_is_enabled_regmap,
//	.enable_time = wl2864c_regulator_enable_time,
};
#else
static struct regulator_ops wl2864c_ldo_ops = {
	.list_voltage = regulator_list_voltage_table,
	.map_voltage = regulator_map_voltage_ascend,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
//	.enable_time = wl2864c_regulator_enable_time,
};
#endif
static struct regulator_desc wl2864c_regulator_desc[] = {
	{
		.name = "ldo1",
		.of_match = of_match_ptr("ldo1"),
		.id = WL2864C_ID_LDO1,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo_vtbl),
		.volt_table = wl2864c_ldo_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO1_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB1_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO1_M,
	},
	{
		.name = "ldo2",
		.of_match = of_match_ptr("ldo2"),
		.id = WL2864C_ID_LDO2,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo_vtbl),
		.volt_table = wl2864c_ldo_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO2_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB1_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO2_M,
	},
	{
		.name = "ldo3",
		.of_match = of_match_ptr("ldo3"),
		.id = WL2864C_ID_LDO3,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo3_vtbl),
		.volt_table = wl2864c_ldo3_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO3_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB2_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO3_M,
	},
	{
		.name = "ldo4",
		.of_match = of_match_ptr("ldo4"),
		.id = WL2864C_ID_LDO4,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo3_vtbl),
		.volt_table = wl2864c_ldo3_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO4_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB2_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO4_M,
	},
	{
		.name = "ldo5",
		.of_match = of_match_ptr("ldo5"),
		.id = WL2864C_ID_LDO5,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo3_vtbl),
		.volt_table = wl2864c_ldo3_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO5_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB2_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO5_M,
	},
	{
		.name = "ldo6",
		.of_match = of_match_ptr("ldo6"),
		.id = WL2864C_ID_LDO6,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo3_vtbl),
		.volt_table = wl2864c_ldo3_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO6_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB2_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO6_M,
	},
	{
		.name = "ldo7",
		.of_match = of_match_ptr("ldo7"),
		.id = WL2864C_ID_LDO7,
		.ops = &wl2864c_ldo_ops,
		.n_voltages = ARRAY_SIZE(wl2864c_ldo3_vtbl),
		.volt_table = wl2864c_ldo3_vtbl,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_reg = WL2864C_LDO7_VOUT,
		.vsel_mask = WL2864C_VOUT_VTB2_M,
		.enable_reg = WL2864C_ENABLE,
		.enable_mask = WL2864C_EN_LDO7_M,
	},
};

static int wl2864c_set_pin_en(struct wl2864c *lp, int enable)
{
	int ret;

	if (!lp->pinctrl)
		return -EINVAL;
	
 	if (enable && lp->gpio_state_active) {
  		dev_err(lp->dev, "set wl2864c enable\n");		
  		ret = pinctrl_select_state(lp->pinctrl, lp->gpio_state_active);
  		if (ret) {
  			dev_err(lp->dev, "failed to enable GPIO\n");
  			return -1;
  		}
  	}
	if (!enable && lp->gpio_state_suspend) {
  		dev_err(lp->dev, "set wl2864c disable\n");
  		ret = pinctrl_select_state(lp->pinctrl, lp->gpio_state_suspend);
  		if (ret) {
  			dev_err(lp->dev, "failed to disable GPIO\n");
  			return -1;
  		}
  	}

	return 0;
}


static struct regulator_init_data
*wl2864c_find_regulator_init_data(int id, struct wl2864c *lp)
{
	struct wl2864c_platform_data *pdata = lp->pdata;
	int i;

	if (!pdata)
		return NULL;

	for (i = 0; i < lp->num_regulators; i++) {
		if (pdata->regulator_data[i].id == id)
			return pdata->regulator_data[i].init_data;
	}

	return NULL;
}

static int wl2864c_regulator_register(struct wl2864c *lp)
{
	struct regulator_desc *desc;
	struct regulator_config cfg = { };
	struct regulator_dev *rdev;
	int i;

	for (i = 0; i < lp->num_regulators; i++) {
		desc = &wl2864c_regulator_desc[i];
		cfg.dev = lp->dev;
		cfg.init_data = wl2864c_find_regulator_init_data(desc->id, lp);
		cfg.driver_data = lp;
		cfg.regmap = lp->regmap;

		rdev = devm_regulator_register(lp->dev, desc, &cfg);
		if (IS_ERR(rdev)) {
			dev_err(lp->dev, "regulator register err");
			return PTR_ERR(rdev);
		}
		dev_err(lp->dev, "regulator %s register ok", desc->name);		
	}
	return 0;
}

static const struct regmap_config wl2864c_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = MAX_REGISTERS,
};

#ifdef CONFIG_OF

#define WL2864C_VALID_OPMODE	(REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL)

static struct of_regulator_match wl2864c_matches[] = {
	{ .name = "ldo1", .driver_data = (void *)WL2864C_ID_LDO1, },
	{ .name = "ldo2", .driver_data = (void *)WL2864C_ID_LDO2, },
	{ .name = "ldo3", .driver_data = (void *)WL2864C_ID_LDO3, },
	{ .name = "ldo4", .driver_data = (void *)WL2864C_ID_LDO4, },
	{ .name = "ldo5", .driver_data = (void *)WL2864C_ID_LDO5, },
	{ .name = "ldo6", .driver_data = (void *)WL2864C_ID_LDO6, },
	{ .name = "ldo7", .driver_data = (void *)WL2864C_ID_LDO7, },
};

static struct wl2864c_platform_data
*wl2864c_populate_pdata_from_dt(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct wl2864c_platform_data *pdata;
	struct of_regulator_match *match;
	int num_matches;
	int count;
	int i;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);
	
	match = wl2864c_matches;
	num_matches = ARRAY_SIZE(wl2864c_matches);

	count = of_regulator_match(dev, np, match, num_matches);
	if (count <= 0)
		goto out;

	for (i = 0; i < num_matches; i++) {
		pdata->regulator_data[i].id =
				(enum wl2864c_regulator_id)match[i].driver_data;
		pdata->regulator_data[i].init_data = match[i].init_data;
	}
out:
	return pdata;
}
#else
static struct wl2864c_platform_data
*wl2864c_populate_pdata_from_dt(struct device *dev)
{
	return NULL;
}
#endif

static int wl2864c_probe(struct i2c_client *cl, const struct i2c_device_id *id)
{
	struct wl2864c *lp;
	struct wl2864c_platform_data *pdata;
	int ret;

	dev_err(&cl->dev, "wl2864c probe...\n");

	if (cl->dev.of_node) {
		pdata = wl2864c_populate_pdata_from_dt(&cl->dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	} else {
		pdata = dev_get_platdata(&cl->dev);
	}

	lp = devm_kzalloc(&cl->dev, sizeof(struct wl2864c), GFP_KERNEL);
	if (!lp)
		return -ENOMEM;

	lp->num_regulators = WL2864C_NUM_REGULATORS;

	lp->regmap = devm_regmap_init_i2c(cl, &wl2864c_regmap_config);
	if (IS_ERR(lp->regmap)) {
		ret = PTR_ERR(lp->regmap);
		dev_err(&cl->dev, "regmap init i2c err: %d\n", ret);
		return ret;
	}
	
	lp->dev = &cl->dev;
	lp->pdata = pdata;
	i2c_set_clientdata(cl, lp);
	
 	lp->pinctrl = devm_pinctrl_get(lp->dev);
  	if (IS_ERR_OR_NULL(lp->pinctrl)) {
  		dev_err(lp->dev, "Unable to acquire pinctrl\n");
  		lp->pinctrl = NULL;
  		return 0;
  	}
  
  	lp->gpio_state_active = pinctrl_lookup_state(lp->pinctrl, "wl2864c_active");
  	if (IS_ERR_OR_NULL(lp->gpio_state_active)) {
  		dev_err(lp->dev, "Cannot lookup wl2864c active state\n");
  		devm_pinctrl_put(lp->pinctrl);
  		lp->pinctrl = NULL;
  		return PTR_ERR(lp->gpio_state_active);
  	}
  
  	lp->gpio_state_suspend = pinctrl_lookup_state(lp->pinctrl, "wl2864c_suspend");
  	if (IS_ERR_OR_NULL(lp->gpio_state_suspend)) {
  		dev_err(lp->dev, "Cannot lookup wl2864c disable state\n");
  		devm_pinctrl_put(lp->pinctrl);
  		lp->pinctrl = NULL;
  		return PTR_ERR(lp->gpio_state_suspend);
  	}
	
	ret = wl2864c_set_pin_en(lp, 0);
	if (ret){
		dev_err(lp->dev, "wl2864c wl2864c_hw_enable failed!\n");
		return ret;
	}
	
//	wl2864c_write_byte(lp, WL2864C_LDO7_VOUT, 48);
//	wl2864c_write_byte(lp, WL2864C_ENABLE, 0x40);	
	ret = mach_chip_id(lp);
	if(ret){		
		dev_err(lp->dev, "wl2864c mach_chip_id failed!\n");
		return ret;
	}
	dev_err(lp->dev, "wl2864c probe done!\n");
	return wl2864c_regulator_register(lp);
}

static const struct of_device_id wl2864c_dt_ids[] = {
	{ .compatible = "ontim,wl2864c", },
	{ }
};
MODULE_DEVICE_TABLE(of, wl2864c_dt_ids);

//static const struct i2c_device_id wl2864c_ids[] = {
//	{"wl2864c", WL2864C},
//	{ }
//};
//MODULE_DEVICE_TABLE(i2c, wl2864c_ids);

static struct i2c_driver wl2864c_driver = {
	.driver = {
		.name = "wl2864c",
		.of_match_table = of_match_ptr(wl2864c_dt_ids),
	},
	.probe = wl2864c_probe,
//	.id_table = wl2864c_ids,
};

module_i2c_driver(wl2864c_driver);

MODULE_DESCRIPTION("ontim wl2864c Camera 7in1 Regulator Driver");
MODULE_AUTHOR("man.li@ontim.cn");
MODULE_LICENSE("GPL");
