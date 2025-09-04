#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <asm/uaccess.h>

// *************************************************************************
// Note: A application will write factory reset time into a partition.
//       A HMD apk will read /sys/devices/proinfo/factoryreset_date for
//       the information, so here it is the code.
//       Partition depends on project definition.
// *************************************************************************

//#define PRIVATE_PARTITION_PATH "/dev/block/by-name/private"
#define PRIVATE_PARTITION_PATH "/dev/block/by-name/oemowninfo"
#define PRIVATE_HMD_FACTORYRESET_DATE_OFFSET (32)
#define PRIVATE_HMD_FACTORYRESET_DATE_LEN    (64)

static ssize_t factoryreset_date_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
    struct file *fp;
    mm_segment_t fs;
    loff_t pos = PRIVATE_HMD_FACTORYRESET_DATE_OFFSET;
    ssize_t len = 0;
    char *tmp_buf;

    fp = filp_open(PRIVATE_PARTITION_PATH, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        pr_err("%s:%s open error /n", __func__, PRIVATE_PARTITION_PATH);
        return 0;
    }

    tmp_buf = (char *)kzalloc(PRIVATE_HMD_FACTORYRESET_DATE_LEN, GFP_ATOMIC);

    fs = get_fs();
    set_fs(KERNEL_DS);
    len = vfs_read(fp, tmp_buf, PRIVATE_HMD_FACTORYRESET_DATE_LEN, &pos);
    fp->f_pos = pos;
    filp_close(fp, NULL);
    set_fs(fs);

    sprintf(buf, "%s", tmp_buf);
    
    kfree(tmp_buf);
	return len;
}

static DEVICE_ATTR_RO(factoryreset_date);

static struct attribute *dev_attrs[] = {
	&dev_attr_factoryreset_date.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static const struct attribute_group *dev_attr_groups[] = {
	&dev_attr_group,
	NULL,
};

static struct device hmd_proinfo = {
 	.init_name	= "proinfo",
 	.groups     = dev_attr_groups,
};

static int __init hmd_proinfo_init(void)
{
	return device_register(&hmd_proinfo);
}

static void __exit hmd_proinfo_exit(void)
{
	device_unregister(&hmd_proinfo);
}


late_initcall(hmd_proinfo_init);
module_exit(hmd_proinfo_exit);

MODULE_AUTHOR("ning.wei@hmdglobal.com");
MODULE_LICENSE("GPL");
