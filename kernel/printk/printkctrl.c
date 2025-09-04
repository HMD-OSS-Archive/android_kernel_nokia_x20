/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/sched/cputime.h>


extern int hmd_get_uartlog_status(void);
extern void hmd_set_uartlog_status(bool value);
extern void hmd_disable_uart(void);
extern void hmd_enable_uart(void);


static int hmd_printkctrl_show(struct seq_file *m, void *v)
{
	seq_printf(m, "=== hmd printk controller ===\n");
	seq_printf(m, "0:   printk uart output disable\n");
	seq_printf(m, "1:   printk uart output enable\n");
	seq_printf(m, "2:   printk uart output enable fixed\n");
	seq_printf(m, "printk uart enable: %d\n", hmd_get_uartlog_status());
	return 0;
}

static ssize_t hmd_printkctrl_write(struct file *filp,
	const char *ubuf, size_t cnt, loff_t *data)
{
	char buf[128];
	long val;
	int ret;

	if (cnt >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(&buf, ubuf, cnt))
		return -EFAULT;

	buf[cnt] = 0;

	ret = kstrtoul(buf, 10, (unsigned long *)&val);

	if (ret < 0)
		return ret;

	switch (val) {
	case 0:
		hmd_disable_uart();
		break;
	case 1:
		hmd_enable_uart();
		break;
	default:
		break;
	}
	return cnt;
}


static int hmd_printkctrl_open(struct inode *inode, struct file *file)
{
	return single_open(file, hmd_printkctrl_show, NULL);
}


static const struct file_operations hmd_printkctrl_fops = {
	.open		= hmd_printkctrl_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.write		= hmd_printkctrl_write,
	.release	= single_release,
};


static int __init init_printkctrl(void)
{
	struct proc_dir_entry *pe;

	pe = proc_create("hmduart", 0664, NULL, &hmd_printkctrl_fops);
	if (!pe)
		return -ENOMEM;
	return 0;
}

device_initcall(init_printkctrl);
