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

struct wl2864c *chip_data;
EXPORT_SYMBOL(chip_data);

static int wl2864c_read_byte(struct wl2864c *chip, u8 addr, u8 *data)
{
	int ret;
	unsigned int val;

	ret = regmap_read(chip->regmap, addr, &val);
	if (ret < 0) {
		dev_err(chip->dev, "failed to read 0x%.2x\n", addr);
		return ret;
	}

	*data = (u8)val;
	return 0;
}

static inline int wl2864c_write_byte(struct wl2864c *chip, u8 addr, u8 data)
{
	return regmap_write(chip->regmap, addr, data);
}

static ssize_t dump_reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0,ret,count;
	u8 reg_data;

	count = snprintf(buf, PAGE_SIZE, "dump all register data start\n");
	for (i = 0; i < WL2864C_MAX_REGISTER ; i++) {
		ret = wl2864c_read_byte(chip_data, (WL2864C_REG_CHIP_REV + i), &reg_data);
		if (ret) {
			dev_err(chip_data->dev, "read register=0x%02x fail\n",WL2864C_REG_CHIP_REV + i);
			count += snprintf(buf + count, PAGE_SIZE, "reg = 0x%02X\n", WL2864C_REG_CHIP_REV + i);
		} else {
			count += snprintf(buf + count, PAGE_SIZE, "reg = 0x%02x reg_data = 0x%02x\n", WL2864C_REG_CHIP_REV + i,reg_data);
		}
	}
	count += snprintf(buf + count, PAGE_SIZE, "dump all register data end\n");
	return count;
}

static int shex_to_int(const char *hex_buf, int size)
{
	int i;
	int base = 1;
	int value = 0;
	char single;

	for (i = size - 1; i >= 0; i--) {
		single = hex_buf[i];

		if ((single >= '0') && (single <= '9')) {
			value += (single - '0') * base;
		} else if ((single >= 'a') && (single <= 'z')) {
			value += (single - 'a' + 10) * base;
		} else if ((single >= 'A') && (single <= 'Z')) {
			value += (single - 'A' + 10) * base;
		} else {
			return -EINVAL;
		}

		base *= 16;
	}
	return value;
}

/************************************************************************
 * * Name: dump_reg_store
 * * Brief:  read/write register
 * * Input: device, device attribute, char buf, char count
 * * Output: print register value
 * * Return: char count
 * example:echo 0324 > #/reg_dump
 * reg:0x03
 * data:0x24
 * echo 03 > #/reg_dump
 * read reg 0x03 data:xxxx
 * ***********************************************************************/
static ssize_t dump_reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t cmd_length = 0;

	cmd_length = count - 1;

	if (rw_buf.opbuf) {
		kfree(rw_buf.opbuf);
		rw_buf.opbuf = NULL;
	}

	dev_err(chip_data->dev,"cmd len: %d, buf: %s", (int)cmd_length, buf);

	if (2 == cmd_length) {
		rw_buf.type = RWREG_OP_READ;
		rw_buf.len = 1;

		rw_buf.reg = shex_to_int(buf, 2);
	} else if (4 == cmd_length) {
		rw_buf.type = RWREG_OP_WRITE;
		rw_buf.len = 1;
		rw_buf.reg = shex_to_int(buf, 2);
		rw_buf.val = shex_to_int(buf + 2, 2);

	} else {
		dev_err(chip_data->dev,"Invalid cmd buffer");
		return -EINVAL;
	}

	if (rw_buf.len < 0) {
		dev_err(chip_data->dev,"cmd buffer error!");

	} else {
		if (RWREG_OP_READ == rw_buf.type) {
			if (rw_buf.len == 1) {
				u8 reg, val;
				reg = rw_buf.reg & 0xFF;
				rw_buf.ret = wl2864c_read_byte(chip_data, reg, &val);
				rw_buf.val = val;
			} else {
				//u8 reg;
				//reg = rw_buf.reg & 0xFF;
				//rw_buf.ret = wl2864c_read_byte(chip_data, &reg, 1, rw_buf.opbuf, rw_buf.len);
				//rw_buf.ret = wl2864c_read_byte(chip_data, reg, &val);
			}

			if (rw_buf.ret < 0) {
				dev_err(chip_data->dev,"Could not read 0x%02x", rw_buf.reg);
			} else {
				dev_err(chip_data->dev,"read reg=0x%02x, reg_value=0x%02x %d bytes successful", rw_buf.reg, rw_buf.val, rw_buf.len);
				rw_buf.ret = 0;
			}

		} else {
			if (rw_buf.len == 1) {
				u8 reg, val;
				reg = rw_buf.reg & 0xFF;
				val = rw_buf.val & 0xFF;
				rw_buf.ret = wl2864c_write_byte(chip_data, reg, val);;
			} else {
				//rw_buf.ret = wl2864c_write_byte(client, rw_buf.opbuf, rw_buf.len);
				//rw_buf.ret = wl2864c_write_byte(chip_data, reg, val);;
			}
			if (rw_buf.ret < 0) {
				dev_err(chip_data->dev,"Could not write 0x%02x", rw_buf.reg);

			} else {
				dev_err(chip_data->dev,"Write reg=0x%02x,data=0x%02x %d bytes successful", rw_buf.reg, rw_buf.val, rw_buf.len);
				rw_buf.ret = 0;
			}
		}
	}

	return count;
}

/* dump wl2846c reg data*/
static DEVICE_ATTR(dump_reg, S_IRUGO | S_IWUSR, dump_reg_show, dump_reg_store);

/* add your attr in here*/
static struct attribute *wl2864c_attributes[] = {
	       &dev_attr_dump_reg.attr,
	       NULL
};

static struct attribute_group wl2864c_attribute_group = {
	       .attrs = wl2864c_attributes
};
/**just debug wl2864c**/
#if 0
static int wl2864c_config_output_voltage(struct wl2864c *chip_data)
{
	int i;
	int ret;
	for(i = 0; i < WL2864C_MAX_REGULATORS; i++) {
		ret = wl2864c_write_byte(chip_data, wl2864c_ldo_table[i].reg, wl2864c_ldo_table[i].reg_value);
		if (ret) {
			dev_err(chip_data->dev,"Failed to write ldo[i]=%s reg=0x%02x, value=0x%02x\n",
					i,wl2864c_ldo_table[i].reg, wl2864c_ldo_table[i].reg_value);
			return ret;
		}

		dev_dbg(chip_data->dev,"write ldo[%d] reg=0x%02x, value=0x%02x\n",
				i,wl2864c_ldo_table[i].reg, wl2864c_ldo_table[i].reg_value);
	}
	ret = wl2864c_write_byte(chip_data, WL2864C_REG_LDO_EN, 0XFF);
	if (ret) {
		dev_err(chip_data->dev, "Failed to write enable ldo%d\n",i);
		return ret;
	}

	return 0;
};

#endif

/*
 * I2C driver interface functions
 */
static int wl2864c_i2c_probe(struct i2c_client *i2c,
		const struct i2c_device_id *id)
{

	struct wl2864c *chip;
	int error;
	u8 otpid;
	struct device_node *np = i2c->dev.of_node;
#if 0

	struct regulator_init_data *init_data = dev_get_platdata(&i2c->dev);
	struct regulator_config config = { };
	int  i;
	config.dev = chip->dev;
	config.regmap = chip->regmap;

#endif


	pr_err("%s:entry\n",__func__);
	chip = devm_kzalloc(&i2c->dev, sizeof(struct wl2864c), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->enable_gpio = of_get_named_gpio(np, "wl2846c-enable-gpios", 0);
	if (!gpio_is_valid(chip->enable_gpio))
		return -ENOMEM;

	chip->dev = &i2c->dev;
	error = gpio_request(chip->enable_gpio, "wl2846c-EN");
	if (error) {
		dev_err(chip->dev, "%s:gpio request err: %d\n", __func__,error);
		return error;
	}
	gpio_direction_output(chip->enable_gpio, 0x0);
	/* Always set enable GPIO low. */
	gpio_set_value(chip->enable_gpio,0);

	chip->regmap = devm_regmap_init_i2c(i2c, &wl2864c_regmap_config);
	if (IS_ERR(chip->regmap)) {
		error = PTR_ERR(chip->regmap);
		dev_err(chip->dev, "Failed to allocate register map: %d\n",
			error);
		return error;
	}

	error = wl2864c_read_byte(chip, WL2864C_REG_CHIP_REV, &otpid);
	if (error) {
		dev_err(chip->dev, "Failed to read OTP ID\n");
		goto exit;
	}
	if (WL2864C_REG_CHIP_ID != otpid)
		goto exit;

	chip_data = chip;
	i2c_set_clientdata(i2c, chip);

	error = sysfs_create_group(&i2c->dev.kobj, &wl2864c_attribute_group);
	if (error) {
		dev_err(chip->dev,"sysfs_create_group() failed!!\n");
		goto sysfs_exit;
	} else {
		dev_err(chip->dev,"sysfs_create_group() succeeded!!\n");
	}

	of_platform_populate(chip->dev->of_node, NULL, NULL, chip->dev);
#if 0
	error = wl2864c_config_output_voltage(chip);
	if (error) {
		dev_err(chip->dev,"config wl2864c ldo output error\n");
		goto output_voltage_error;
	}
#endif

#if 0

	for (i = 0; i < WL2864C_MAX_REGULATORS; i++) {
		if (init_data)
			config.init_data = &init_data[i];

		config.driver_data = (void *)&wl2864c_regulator_info[i];
		chip->rdev[i] = devm_regulator_register(chip->dev,
			&wl2864c_regulator_info[i].desc, &config);
		if (IS_ERR(chip->rdev[i])) {
			dev_err(chip->dev,
				"Failed to register WL2864C regulator\n");
			return PTR_ERR(chip->rdev[i]);
		}
	}
#endif
	return 0;
//output_voltage_error:
sysfs_exit:
	sysfs_remove_group(&i2c->dev.kobj, &wl2864c_attribute_group);
exit:
	if (gpio_is_valid(chip->enable_gpio))
		gpio_free(chip->enable_gpio);
	devm_kfree(&i2c->dev, chip);
	return error;
}

#ifdef CONFIG_PM_SLEEP
static int wl2864c_suspend(struct device *dev)
{
	int rc = 0;
	struct wl2864c *chip_data = dev_get_drvdata(dev);
	dev_err(chip_data->dev, "%s enter\n",__func__);
#if 0
	rc = wl2864c_write_byte(chip_data, WL2864C_REG_LDO_EN, 0X40);
	if (rc) {
		dev_err(chip_data->dev, "Failed to write enable ldo\n");
	}
#endif
	return rc;
}

static int wl2864c_resume(struct device *dev)
{
	int rc = 0;
	struct wl2864c *chip_data = dev_get_drvdata(dev);
	dev_err(chip_data->dev, "%s enter\n",__func__);
#if 0
	rc = wl2864c_write_byte(chip_data, WL2864C_REG_LDO_EN, 0XFF);
	if (rc) {
		dev_err(chip_data->dev, "Failed to write enable ldo\n");
	}
#endif
	return rc;
}
#else
static int wl2864c_suspend(struct device *dev)
{
	return 0;
}
static int wl2864c_resume(struct device *dev)
{
	return 0;
}
#endif

static void wl2864c_i2c_shutdown(struct i2c_client *i2c)
{
	int rc = 0;
	struct wl2864c *data = i2c_get_clientdata(i2c);
	rc = wl2864c_write_byte(data, WL2864C_REG_LDO_EN, 0X00);
	if (rc) {
		dev_err(data->dev, "Failed to write disable ldo\n");
	}
}

static int wl2864c_i2c_remove(struct i2c_client *i2c)
{
	int rc = 0;
	struct wl2864c *data = i2c_get_clientdata(i2c);
	rc = wl2864c_write_byte(data, WL2864C_REG_LDO_EN, 0X00);
	if (rc) {
		dev_err(data->dev, "Failed to write disable ldo\n");
	}

	return 0;
}
static const struct dev_pm_ops wl2864c_pm_ops = {
	.suspend	= wl2864c_suspend,
	.resume		= wl2864c_resume,
};

static const struct i2c_device_id wl2864c_i2c_id[] = {
	{"wl2864c", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, wl2864c_i2c_id);

#ifdef CONFIG_OF
static const struct of_device_id wl2864c_dt_ids[] = {
	{ .compatible = "pmic,wl2864c", .data = &wl2864c_i2c_id[0] },
	{},
};
MODULE_DEVICE_TABLE(of, wl2864c_dt_ids);
#endif

static struct i2c_driver wl2864c_i2c_driver = {
	.driver = {
		.name = "wl2864c",
		.owner		= THIS_MODULE,
		.pm		= &wl2864c_pm_ops,
		.of_match_table = of_match_ptr(wl2864c_dt_ids),
	},
	.probe = wl2864c_i2c_probe,
	.remove = wl2864c_i2c_remove,
	.shutdown = wl2864c_i2c_shutdown,
	.id_table = wl2864c_i2c_id,
};

static int __init wl2864c_i2c_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&wl2864c_i2c_driver);
	if (ret) {
		pr_err("fail to add wl2864c device into i2c\n");
		return ret;
	}

	return ret;
}

module_init(wl2864c_i2c_init);

static void __exit wl2864c_i2c_exit(void)
{
		i2c_del_driver(&wl2864c_i2c_driver);
}

module_exit(wl2864c_i2c_exit);

MODULE_AUTHOR("driver driver@diasemi.com>");
MODULE_DESCRIPTION("Regulator device driver for Powerventure WL2864C");
MODULE_LICENSE("GPL");
