/*
 */

#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/regmap.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/string.h>
#include "wl2864c-regulator.h"

extern struct wl2864c *chip_data;

#if 0
static int wl2864c_set_current_limit(struct regulator_dev *rdev, int min,
				    int max)
{
	return 0;
}

static int wl2864c_get_current_limit(struct regulator_dev *rdev)
{
	return 0;
}
#endif

#if 0
static int wl2864c_regulator_enable_regmap(struct regulator_dev *rdev)
{
	int ret;
	u8 reg_data = 0;
	ret = wl2864c_read_byte(chip_data, (u8)rdev->desc->enable_reg, &reg_data);
        dev_err(chip_data->dev,"%s:read reg=0x%02x error ret=%d\n",__func__,rdev->desc->enable_reg,ret);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:read reg=0x%02x error ret=%d\n",__func__,rdev->desc->enable_reg,ret);
		return ret;
	}

	reg_data |= rdev->desc->enable_mask;
	dev_err(chip_data->dev,"%s:name=%s enable_reg=0x%02x,mask=0x%02x,reg_data=0x%02x\n",__func__,rdev->desc->name,rdev->desc->enable_reg,rdev->desc->enable_mask,reg_data);
	ret = wl2864c_write_byte(chip_data,(u8)rdev->desc->enable_reg,reg_data);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:write error ret=%d\n",__func__,ret);
	}

	return ret;
}

static int wl2864c_regulator_disable_regmap(struct regulator_dev *rdev)
{
	int ret;
	u8 reg_data = 0;
	ret = wl2864c_read_byte(chip_data, (u8)rdev->desc->enable_reg, &reg_data);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:read reg=0x%02x error ret=%d\n",__func__,rdev->desc->enable_reg,ret);
		return ret;
	}
	reg_data &= ~rdev->desc->enable_mask;
	dev_err(chip_data->dev,"%s:name=%s,enable_reg=0x%02x,mask=0x%02x,reg_data=0x%02x\n",__func__,rdev->desc->name,rdev->desc->enable_reg,rdev->desc->enable_mask,reg_data);

	ret = wl2864c_write_byte(chip_data,(u8)rdev->desc->enable_reg,reg_data);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:write error ret=%d\n",__func__,ret);
	}

	return ret;
}

static int wl2864c_regulator_is_enabled_regmap(struct regulator_dev *rdev)
{
	int ret;
	u8 reg_data;
	ret = wl2864c_read_byte(chip_data, (u8)rdev->desc->enable_reg, &reg_data);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:read reg=0x%02x error ret=%d\n",__func__,rdev->desc->enable_reg,ret);
		return ret;
	}
	dev_err(chip_data->dev,"%s: name= %s,enable_reg=0x%02x,mask=0x%02x,reg_data=0x%02x\n",__func__,rdev->desc->name,rdev->desc->enable_reg,rdev->desc->enable_mask,reg_data);

	return (reg_data & rdev->desc->enable_mask);
}

static int wl2864c_regulator_set_voltage_sel_regmap(struct regulator_dev *rdev,unsigned int selector)
{

	int ret;
	dev_err(chip_data->dev,"%s:rdev->desc->vsel_reg=0x%02x,mask=0x%02x,selector=0x%x\n",__func__,rdev->desc->vsel_reg,rdev->desc->vsel_mask,selector);

	ret = wl2864c_write_byte(chip_data,(u8)rdev->desc->vsel_reg,(u8)selector);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:write error ret=%d\n",__func__,ret);
	}
	return ret;
}

static int wl2864c_regulator_get_voltage_sel_regmap(struct regulator_dev *rdev)
{
	int ret;
	u8 reg_data = 0;
	ret = wl2864c_read_byte(chip_data, (u8)rdev->desc->vsel_reg, &reg_data);
	if (ret < 0){
		dev_err(chip_data->dev,"%s:read reg=0x%02x error ret=%d\n",__func__,rdev->desc->vsel_reg,ret);
		return ret;
	}
	dev_err(chip_data->dev,"%s:rdev->desc->vsel_reg=0x%02x,mask=0x%02x,reg_data=0x%02x\n",__func__,rdev->desc->vsel_reg,rdev->desc->vsel_mask,reg_data);
	return reg_data;
}
int wl2864c_regulator_list_voltage_linear(struct regulator_dev *rdev,unsigned int selector)
{
	int ret = regulator_list_voltage_linear(rdev,selector);
	return ret;
}
#endif

static const struct regulator_ops wl2864c_ldo_ops = {
#if 0
	.enable = wl2864c_regulator_enable_regmap,
	.disable = wl2864c_regulator_disable_regmap,
	.is_enabled = wl2864c_regulator_is_enabled_regmap,
	.set_voltage_sel = wl2864c_regulator_set_voltage_sel_regmap,
	.get_voltage_sel = wl2864c_regulator_get_voltage_sel_regmap,
	.list_voltage = wl2864c_regulator_list_voltage_linear,
#else
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.list_voltage = regulator_list_voltage_linear,
#endif
	//.set_current_limit = wl2864c_set_current_limit,
	//.get_current_limit = wl2864c_get_current_limit,
};

#define WL2864C_LDO(chip, regl_name, min, step, max , limits_array ) \
{\
	.desc	=	{\
		.id = chip##_ID_##regl_name,\
		.name = __stringify(chip##_##regl_name),\
		.of_match = of_match_ptr(#regl_name),\
		.regulators_node = of_match_ptr("regulators"),\
		.type = REGULATOR_VOLTAGE,\
		.owner = THIS_MODULE,\
		.ops = &wl2864c_ldo_ops,\
		.min_uV = min, \
		.uV_step = step, \
		.n_voltages = (max - min) / step + 1, \
		.enable_reg = WL2864C_REG_LDO_EN, \
		.enable_mask = 0x01 <<(chip##_ID_##regl_name), \
		.vsel_reg = WL2864C_REG_##regl_name##_VOUT, \
		.vsel_mask = WL2864C_LDO_VOUT_MASK, \
	},\
	.current_limits = limits_array,\
	.n_current_limits = ARRAY_SIZE(limits_array),\
	.limit_mask = WL2864C_CURR_ILIM_MASK <<( (chip##_ID_##regl_name)/2), \
	.conf = WL2864C_REG_CURR_LIMT,\
}



static struct wl2864c_regulator wl2864c_regulator_info[] = {
	WL2864C_LDO(WL2864C, LDO1, 600000, 12500, 1800000, wl2864c_ldo1_2_curr_limits),
	WL2864C_LDO(WL2864C, LDO2, 600000, 12500, 1800000, wl2864c_ldo1_2_curr_limits),
	WL2864C_LDO(WL2864C, LDO3, 1200000, 12500, 4300000, wl2864c_ldo3_4_curr_limits),
	WL2864C_LDO(WL2864C, LDO4, 1200000, 12500, 4300000, wl2864c_ldo3_4_curr_limits),
	WL2864C_LDO(WL2864C, LDO5, 1200000, 12500, 4300000, wl2864c_ldo5_6_curr_limits),
	WL2864C_LDO(WL2864C, LDO6, 1200000, 12500, 4300000, wl2864c_ldo5_6_curr_limits),
	WL2864C_LDO(WL2864C, LDO7, 1200000, 12500, 4300000, wl2864c_ldo7_curr_limits),
};


static int wl2864c_register_ldo(struct wl2864c_regulator *wl2864c_reg,
						const char *name)
{
	struct regulator_config reg_config = {};
	struct regulator_init_data *init_data;
	struct device *dev = wl2864c_reg->dev;
	struct device_node *reg_node = wl2864c_reg->of_node;
	int rc;


	init_data = of_get_regulator_init_data(dev, reg_node,
						&wl2864c_reg->desc);
	if (init_data == NULL) {
		pr_err("%s: failed to get regulator data\n", name);
		return -ENODATA;
	}
	if (!init_data->constraints.name) {
		pr_err("%s: regulator name missing\n", name);
		return -EINVAL;
	}

#if 0
	init_voltage = -EINVAL;
	of_property_read_u32(reg_node, "qcom,init-voltage", &init_voltage);

	/* configure the initial voltage for the regulator */
	if (init_voltage > 0) {

	}
#endif

	init_data->constraints.input_uV = init_data->constraints.max_uV;
	init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE;
	reg_config.dev = dev;
	reg_config.init_data = init_data;
	reg_config.driver_data = wl2864c_reg;
	reg_config.of_node = reg_node;

	//wl2864c_reg->desc.name = init_data->constraints.name;

	wl2864c_reg->rdev = devm_regulator_register(dev, &wl2864c_reg->desc,
						&reg_config);
	if (IS_ERR(wl2864c_reg->rdev)) {
		rc = PTR_ERR(wl2864c_reg->rdev);
		pr_err("%s: failed to register regulator rc=%d\n",
				wl2864c_reg->desc.name, rc);
		return rc;
	}

	return 0;
}

/* PMIC probe and helper function */
static int wl2864c_parse_regulator(struct regmap *regmap, struct device *dev)
{
	int i, rc = 0;
	const char *name;
	struct device_node *child;
	struct wl2864c_regulator *wl2864c_reg;

    pr_err("%s:entry\n",__func__);
	/* parse each subnode and register regulator for regulator child */
	for_each_available_child_of_node(dev->of_node, child) {
		rc = of_property_read_string(child, "regulator-name", &name);
		if (rc)
			continue;

		/* get regulator data */
		for (i = 0; i < WL2864C_MAX_REGULATORS; i++)
			if (!strcmp(wl2864c_regulator_info[i].desc.name, name))
				break;

		if (i == WL2864C_MAX_REGULATORS) {
			pr_err("Invalid regulator name %s\n", name);
			continue;
		}
		wl2864c_reg = &wl2864c_regulator_info[i];
		wl2864c_reg->regmap = regmap;
		wl2864c_reg->of_node = child;
		wl2864c_reg->dev = dev;


		rc = wl2864c_register_ldo(wl2864c_reg, name);
		if (rc < 0) {
			pr_err("failed to register regulator %s rc=%d\n",
					name, rc);
			return rc;
		}
	}

	return 0;
}

static int wl2864c_regulator_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct regmap *regmap;

    pr_err("%s:entry\n",__func__);
	regmap = dev_get_regmap(pdev->dev.parent, NULL);
	if (!regmap) {
		pr_err("parent regmap is missing\n");
		return -EINVAL;
	}

	rc = wl2864c_parse_regulator(regmap, &pdev->dev);
	if (rc < 0) {
		pr_err("failed to parse device tree rc=%d\n", rc);
		return rc;
	}

	return 0;
}

static const struct of_device_id wl2864c_regulator_match_table[] = {
	{
		.compatible	= "qcom,wl2864c-regulator",
	},
	{ },
};

static struct platform_driver wl2864c_regulator_driver = {
	.driver	= {
		.name		= "wl2864c-regulator",
		.owner		= THIS_MODULE,
		.of_match_table	= wl2864c_regulator_match_table,
	},
	.probe		= wl2864c_regulator_probe,
};

static int __init wl2864c_platform_init(void)
{
	int rc = 0;

	rc = platform_driver_register(&wl2864c_regulator_driver);
	if (rc) {
		pr_info("%s +%d, driver init failed\n", __func__, __LINE__);
		return rc;
	}

	return rc;
}

static void __exit wl2864c_platform_exit(void)
{
	platform_driver_unregister(&wl2864c_regulator_driver);
	//pr_info("%s +%d, unregister\n", __func__, __LINE__);
}

module_init(wl2864c_platform_init);
module_exit(wl2864c_platform_exit);

MODULE_AUTHOR("driver driver@diasemi.com>");
MODULE_DESCRIPTION("Regulator device driver for Powerventure WL2864C");
MODULE_LICENSE("GPL");
