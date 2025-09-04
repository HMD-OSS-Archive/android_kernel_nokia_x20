
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/io.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct kobject *deviceinfo_kobj = NULL;
char boardidName[5];
char skuidName[5];

static ssize_t boardid_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",boardidName);
	return (s - buf);
}

static struct kobj_attribute boardinfo = __ATTR_RO(boardid_info);

static ssize_t skuid_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",skuidName);
	return (s - buf);
}

static struct kobj_attribute skuinfo = __ATTR_RO(skuid_info);

static struct attribute * attrs[] = {
	&boardinfo.attr,
	&skuinfo.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __init boardidread(char *str)
{
	memset(boardidName,'\0',sizeof(boardidName));
	memcpy(boardidName, str,strlen(str));

	return 1;
}
__setup("HMD.BoardID=", boardidread);

static int __init skuidread(char *str)
{
	memset(skuidName,'\0',sizeof(skuidName));
	memcpy(skuidName, str,strlen(str));

	return 1;
}
__setup("HMD.SKU=", skuidread);

int __init deviceinfo_init(void)
{
	int ret = -ENOMEM;

	deviceinfo_kobj = kobject_create_and_add("deviceinfo", kernel_kobj);// sys/kernel

	if (deviceinfo_kobj == NULL) {
		printk("deviceinfo_kobj: kobject_create_and_add failed\n");
		goto fail;
	}

	printk("deviceinfo create success!\n");
	ret = sysfs_create_group(deviceinfo_kobj, &attr_group);
	if (ret) {
		printk("deviceinfo_init: sysfs_create_group failed\n");
		goto sys_fail;
	}

    return 0;

sys_fail:
	kobject_del(deviceinfo_kobj);
fail:
	return ret;

}

late_initcall(deviceinfo_init);
