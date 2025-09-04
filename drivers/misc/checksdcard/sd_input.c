/*
 * Copyright (C) 2017-2018 Hisense, Inc.
 *
 * Author:
 *	sunjunfeng <sunjunfeng1@hisense.com>
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

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/export.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/input.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif

#define KEY_SD_IN  750
#define KEY_SD_OUT 751

struct sdpin_proc_data {
	struct device *dev;
	struct input_dev *input;	
};
struct sdpin_proc_data *sdpin_proc;

void sdcard_output(void)
{
	if (sdpin_proc == NULL) {
		pr_err("sdcard_output err:sdpin_proc is NULL\n");
		return;
	}
	if (sdpin_proc->input == NULL) {
		pr_err("sdcard_output err:sdpin_proc input is NULL\n");
		return;
	}
	input_report_key(sdpin_proc->input, KEY_SD_OUT, 1);
    input_sync(sdpin_proc->input);
	input_report_key(sdpin_proc->input, KEY_SD_OUT, 0);
    input_sync(sdpin_proc->input);
}
EXPORT_SYMBOL(sdcard_output);

void sdcard_input(void)
{
	if (sdpin_proc == NULL) {
		pr_err("sdcard_output err:sdpin_proc is NULL\n");
		return;
	}
	if (sdpin_proc->input == NULL) {
		pr_err("sdcard_output err:sdpin_proc input is NULL\n");
		return;
	}
	input_report_key(sdpin_proc->input, KEY_SD_IN, 1);
    input_sync(sdpin_proc->input);
	input_report_key(sdpin_proc->input, KEY_SD_IN, 0);
    input_sync(sdpin_proc->input);
}

EXPORT_SYMBOL(sdcard_input);


static int sdpin_proc_probe(struct platform_device *pdev)
{
	int rc;
	struct device *dev = &pdev->dev;
	
	sdpin_proc = devm_kzalloc(dev, sizeof(*sdpin_proc), GFP_KERNEL);
	dev_info(dev, "%s: enter\n", __func__);
	if (!sdpin_proc) {
		dev_err(dev, "failed to allocate memory for struct sdpin_proc_data\n");
		rc = -ENOMEM;
		goto exit;
	}

	sdpin_proc->dev = dev;
	platform_set_drvdata(pdev, sdpin_proc);

	/*input device subsystem */
	sdpin_proc->input = input_allocate_device();
	if (sdpin_proc->input == NULL) {
		printk("%s, failed to allocate input device\n", __func__);
		rc = -ENOMEM;
		goto err_sdpin_proc;
	}

	input_set_capability(sdpin_proc->input, EV_KEY, KEY_SD_IN);
	input_set_capability(sdpin_proc->input, EV_KEY, KEY_SD_OUT);

	sdpin_proc->input->name = "sdinput";
	rc = input_register_device(sdpin_proc->input);
	if (rc) {
		printk("failed to register input device\n");
		input_free_device(sdpin_proc->input);
		sdpin_proc->input = NULL;
		goto exit;
	}
	dev_info(dev, "%s: ok\n", __func__);
	return 0;

err_sdpin_proc:
	devm_kfree(dev,sdpin_proc);
exit:
	sdpin_proc = NULL;
	return rc;
}

static int sdpin_proc_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s\n", __func__);

	return 0;
}

static struct of_device_id sdpin_proc_of_match[] = {
	{ .compatible = "sdinput", },
	{}
};
MODULE_DEVICE_TABLE(of, sdpin_proc_of_match);

static struct platform_driver sdpin_proc_driver = {
	.driver = {
		.name	= "sd_idpin",
		.owner	= THIS_MODULE,
		.of_match_table = sdpin_proc_of_match,
	},
	.probe	= sdpin_proc_probe,
	.remove	= sdpin_proc_remove,
};

static int __init sdpin_proc_init(void)
{

	int rc;
	printk("%s(): enter\n", __func__);

	rc = platform_driver_register(&sdpin_proc_driver);
	
	if (!rc)
		pr_info("%s OK\n", __func__);
	else
		pr_err("%s %d\n", __func__, rc);
	
	return rc;
}

static void __exit sdpin_proc_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&sdpin_proc_driver);
}

module_init(sdpin_proc_init);
module_exit(sdpin_proc_exit);
MODULE_DESCRIPTION("Sdcard Compatible.");


