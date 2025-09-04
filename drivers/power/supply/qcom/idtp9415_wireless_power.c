
/************************************************************                            $
*
* file: p9415_wireless_power.c 
*
* Description: AP to flash 9221-P OTP firmware
*
*------------------------------------------------------------
* Integrated Device Technology Proprietary and Confidential
* Copyright (c) 2018, Integrated Device Technology Co., Ltd.
* All Rights Reserved
*************************************************************/


#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/device.h>
#include <linux/pinctrl/consumer.h>
//#include <linux/qpnp/qpnp-revid.h>
#include "idtp9415_wireless_power.h"
#include <linux/pmic-voter.h>
#include "smb5-lib.h"

struct p9415_dev *idt = NULL;

struct idtp9415_access_func {
    int (*read)(struct p9415_dev *di, u16 reg, u8 *val);
    int (*write)(struct p9415_dev *di, u16 reg, u8 val);
    int (*read_buf)(struct p9415_dev *di,
                    u16 reg, u8 *buf, u32 size);
    int (*write_buf)(struct p9415_dev *di,
                     u16 reg, u8 *buf, u32 size);
};

struct p9415_dev {
	char                *name;
	struct i2c_client    *client;
	struct device       *dev;
	struct regmap       *regmap;
	struct idtp9415_access_func bus;
	struct dentry		*debug_root;
	u16			idtp9415_reg_address;
	struct power_supply		*dc_psy;
	struct power_supply		*battery_psy;
	struct delayed_work	idt_status_change_work;
	struct delayed_work	vrect_check_work;
	struct notifier_block	nb;
	bool   dc_online;
	bool   dc_9v_online;
	bool   dc_9v_or_12v_online;
	int   soc;
	int   input_max_current_ua;
	struct votable		*fcc_votable;
	int   irq_gpio;
};

int idtp9415_read(struct p9415_dev *di, u16 reg, u8 *val) {
    unsigned int temp;
    int rc;

    rc = regmap_read(di->regmap, reg, &temp);
    if (rc >= 0)
        *val = (u8)temp;

    return rc;
}

int idtp9415_write(struct p9415_dev *di, u16 reg, u8 val) {
    int rc = 0;

    rc = regmap_write(di->regmap, reg, val);
    if (rc < 0)
        dev_err(di->dev, "idtp9415 write error: %d\n", rc);

    return rc;
}
int idtp9415_mask_write(struct p9415_dev *di, u16 reg, u8 mask,u8 val) {
    unsigned int temp;
    u8 temp_val;
    int rc;

    rc = regmap_read(di->regmap, reg, &temp);
    if (rc >= 0)
        temp_val = (u8)temp;
    else
        return rc;
    temp_val = temp_val & ~mask;
    temp_val = temp_val | (mask & val);
    rc = idtp9415_write(di,reg,temp_val);
    return rc;
}

int idtp9415_read_buffer(struct p9415_dev *di, u16 reg, u8 *buf, u32 size) {
    return regmap_bulk_read(di->regmap, reg, buf, size);
}

int idtp9415_write_buffer(struct p9415_dev *di, u16 reg, u8 *buf, u32 size) {
    int rc = 0;

    while (size--) {
        rc = di->bus.write(di, reg++, *buf++);
        if (rc < 0) {
            dev_err(di->dev, "write error: %d\n", rc);
            return rc;
        }
    }

    return rc;
}

static int idtp9415_reg_get(void *data, u64 *val)
{
	int rc;
	u8 temp;

	rc = idtp9415_read(idt, idt->idtp9415_reg_address, &temp);
	if (rc) {
		pr_err("Couldn't read reg %x rc = %d\n",
			idt->idtp9415_reg_address, rc);
		return -EAGAIN;
	}
	*val = temp;
	return 0;
}

static int idtp9415_reg_set(void *data, u64 val)
{
	int rc;
	u64 temp;

	temp = (u8) val;
	rc = idtp9415_write(idt, idt->idtp9415_reg_address, temp);
	if (rc) {
		pr_err("Couldn't write 0x%02llx to 0x%02x rc= %d\n",
			temp, idt->idtp9415_reg_address, rc);
		return -EAGAIN;
	}
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(idtp9415_reg_ops, idtp9415_reg_get, idtp9415_reg_set, "0x%02llx\n");

static int idtp9415_create_debugfs_entries(struct p9415_dev *chip)
{
	struct dentry *ent;

	chip->debug_root = debugfs_create_dir("idtp9415", NULL);
	if (!chip->debug_root) {
		pr_err("Couldn't create debug dir\n");
	} else {
		ent = debugfs_create_x16("address", S_IWUSR | S_IRUGO,
					  chip->debug_root,
					  &(chip->idtp9415_reg_address));
		if (!ent)
			pr_err("Couldn't create address debug file\n");

		ent = debugfs_create_file("data", S_IWUSR | S_IRUGO,
					  chip->debug_root, chip,
					  &idtp9415_reg_ops);
		if (!ent)
			pr_err("Couldn't create data debug file\n");
	}
	return 0;
}

#if 0
int ExtractPacketSize(u8 hdr) {
    if (hdr < 0x20)
        return 1;
    if (hdr < 0x80)
        return (2 + ((hdr - 0x20) >> 4));
    if (hdr < 0xe0)
        return (8 + ((hdr - 0x80) >> 3));
    return (20 + ((hdr - 0xe0) >> 2));
}

void clritr(u16 s) {
    idt->bus.write_buf(idt, REG_INT_CLEAR, (u8 *)&s, 2);
    idt->bus.write(idt, REG_COMMAND, CLRINT);
}

int checkitr(u16 s) {
    u8 buf[2];
    u16 itr;
    idt->bus.read_buf(idt, REG_INTR, buf, 2);
    itr = buf[0]|(buf[1]<<8);
    if (itr & s)
        return true;
    return false;
}

void sendPkt(ProPkt_Type *pkt) {
    int length = ExtractPacketSize(pkt->header)+1;
    idt->bus.write_buf(idt, REG_PROPPKT_ADDR, (u8 *)pkt, length); // write data into proprietary packet buffer
    idt->bus.write(idt, REG_COMMAND,  SENDPROPP);            // send proprietary packet
}

int receivePkt(u8 *data) {
    u8 header;
    u8 length;
    if (checkitr(TXDATARCVD)) {
        idt->bus.read(idt, REG_BCHEADER_ADDR, &header);
        length = ExtractPacketSize(header);
        idt->bus.read_buf(idt, REG_BCDATA_ADDR, data, length);
        clritr(TXDATARCVD);
        return true;
    }
    return false;
}

ssize_t fwver(void) {
    u8 id[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, ver[4]={0xff,0xff,0xff,0xff};

    idt->bus.read_buf(idt, REG_CHIP_ID, id, 8);
    idt->bus.read_buf(idt, REG_CHIP_REV, ver, 4);

    pr_err("IDT ChipID:%04x\nFWVer:%02x.%02x.%02x.%02x\n", id[4]|(id[0]<<8), ver[3], ver[2], ver[1], ver[0]);

    return 0;
}

u8 get_atapter_type(void) {
    ProPkt_Type proPkt;
    u8 data_list[16]={0};

    proPkt.header = PROPRIETARY18;
    proPkt.cmd    = BC_ADAPTER_TYPE;
    sendPkt(&proPkt);
    mdelay(300);
    if (receivePkt(data_list)) {
        pr_err("Atapter Type: %d\n", data_list[0]);
    } else {
        pr_err("Atapter Type: Error\n");
    }

    if ((data_list[0] == ADAPTER_QC20) || (data_list[0] == ADAPTER_QC30))
        return true;
    return false;
}

void fast_charging(ushort mv) {
    idt->bus.write_buf(idt, REG_FC_VOLTAGE, (u8 *)&mv, 2);
    idt->bus.write(idt, REG_COMMAND, VSWITCH);
    pr_err("Fast charging: %dmv\n", mv);
}

static ssize_t chip_version_show(struct device *dev, struct device_attribute *attr, char *buf) {
    return fwver();
}

/* voltage limit attrs */
static ssize_t chip_vout_show(struct device *dev, struct device_attribute *attr, char *buf) {
    u8 buf[2];
    u16 vout;

    p9415_read_buf(REG_ADC_VOUT, buf, 2);
    vout = (buf[0]|(buf[1]<<8)) * 6 * 21 * 1000 / 40950; // vout = val/4095*6*2.1
    dev_info(di->dev, "vout:%04x\n", buf[0]|(buf[1]<<8));

    return sprintf(buf, "Vout ADC Value: %dMV\n", vout);
}

static ssize_t chip_vout_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    if ((*buf>=35) && (*buf<120))
        p9415_write_byte(REG_VOUT_SET, (*buf)-35);

    return count;
}

/* current limit attrs */
static ssize_t chip_cout_show(struct device *dev, struct device_attribute *attr, char *buf) {
    u8 buf[2];
    u16 iout;

    p9415_read_buf(REG_RX_LOUT, buf, 2);
    iout = buf[0]|(buf[1]<<8);
    dev_info(dev, "iout:%04x\n", buf[0]|(buf[1]<<8));

    return sprintf(buf, "Output Current: %dMA\n", iout);
}

static DEVICE_ATTR(chip_version, S_IRUGO|S_IWUGO, chip_version_show, NULL);
static DEVICE_ATTR(chip_vout, S_IWUSR | S_IRUGO, chip_vout_show, chip_vout_store);
static DEVICE_ATTR(chip_cout, S_IWUSR | S_IRUGO, chip_cout_show, NULL);

static struct attribute *p9415_sysfs_attrs[] = {
    &dev_attr_chip_version.attr,
    &dev_attr_chip_vout.attr,
    &dev_attr_chip_cout.attr,
    NULL,
};

static const struct attribute_group p9415_sysfs_group_attrs = {
    .attrs = p9415_sysfs_attrs,
};


static int program_bootloader (struct p9415_dev *di) {
    int i, rc = 0;
    int len;

    len = sizeof(bootloader);

    for (i = 0; i < len; i++) {
        rc = di->bus.write(di, 0x1c00+i, bootloader[i]);
        if (rc)
            return rc;
    }

    return 0;
}

int program_fw(struct p9415_dev *di, u16 destAddr, u8 *src, u32 size) {
    int i, j;
    u8 data = 0;
    //u8 data_array[512];

    //  === Step-1 ===
    // Transfer 9415 boot loader code "OTPBootloader" to 9415 SRAM
    // - Setup 9415 registers before transferring the boot loader code
    // - Transfer the boot loader code to 9415 SRAM
    // - Reset 9415 => 9415 M0 runs the boot loader
    //
    di->bus.read(di, 0x5870, &data);
    printk(KERN_EMERG "0x5870 %s:%d :%02x\n", __func__, __LINE__, data);
    di->bus.read(di, 0x5874, &data);
    printk(KERN_EMERG "0x5874 %s:%d :%02x\n", __func__, __LINE__, data);
    // configure the system
    if (di->bus.write(di, 0x3000, 0x5a)) return false;        // write key
    if (di->bus.write(di, 0x3040, 0x10)) return false;        // halt M0 execution
    if (program_bootloader(di)) return false;
    if (di->bus.write(di, 0x3048, 0x80)) return false;        // map RAM to OTP

    /* ignoreNAK */
    di->bus.write(di, 0x3040, 0x80);        // reset chip and run the bootloader
    mdelay(100);
    printk(KERN_EMERG "%s:%d\n", __func__, __LINE__);

#if 0
    for (i = 0; i < 512; i++) {
        di->bus.read(di, 0x1c00+i, data_array[i]);
        if (data_array[i] != bootloader_data[i]) {
            printk(KERN_EMERG "MAXUEYUE bootloader check err data[%d]:%02x != boot[%d]:%02x.\n", i, data_array[i], i, bootloader_data[i]);
            return 1;
        }
        printk(KERN_EMERG "%02x ", data_array[i]);
        if (i+1 % 16 == 0)
            printk(KERN_EMERG "\n");
    }
#endif
    //
    // === Step-2 ===
    // Program OTP image data to 9415 OTP memory
    //
    for (i = destAddr; i < destAddr+size; i += 128) {        // program pages of 128 bytes
        //
        // Build a packet
        //
        char sBuf[136];        // 136=8+128 --- 8-byte header plus 128-byte data
        u16 StartAddr = (u16)i;
        u16 CheckSum = StartAddr;
        u16 CodeLength = 128;
        int retry_cnt=0;

        memset(sBuf, 0, 136);

        //(1) Copy the 128 bytes of the OTP image data to the packet data buffer
        //    Array.Copy(srcData, i + srcOffs, sBuf, 8, 128);// Copy 128 bytes from srcData (starting at i+srcOffs)
        memcpy(sBuf+8, src, 128);// Copy 128 bytes from srcData (starting at i+srcOffs)
        src+=128;
        // to sBuf (starting at 8)
        //srcData     --- source array
        //i + srcOffs     --- start index in source array
        //sBuf         --- destination array
        //8         --- start index in destination array
        //128         --- elements to copy


        //(2) Calculate the packet checksum of the 128-byte data, StartAddr, and CodeLength
        for (j = 127; j >= 0; j--) {        // find the 1st non zero value byte from the end of the sBuf[] buffer
            if (sBuf[j + 8] != 0)
                break;
            else
                CodeLength--;
        }
        if (CodeLength == 0)
            continue;            // skip programming if nothing to program

        for (; j >= 0; j--)
            CheckSum += sBuf[j + 8];    // add the nonzero values
        CheckSum += CodeLength;        // finish calculation of the check sum

        //(3) Fill up StartAddr, CodeLength, CheckSum of the current packet.
        memcpy(sBuf+2, &StartAddr, 2);
        memcpy(sBuf+4, &CodeLength, 2);
        memcpy(sBuf+6, &CheckSum, 2);

        //
        // Send the current packet to 9415 SRAM via I2C
        //

        // read status is guaranteed to be != 1 at this point
        for (j=0; j<CodeLength+8; j++) {
            if (di->bus.write(di, 0x400+j, sBuf[j])) {
                printk("ERROR: on writing to OTP buffer");
                return false;
            }
        }

        //
        // Write 1 to the Status in the SRAM. This informs the 9415 to start programming the new packet
        // from SRAM to OTP memory
        //
        if (di->bus.write(di, 0x400, 1))    {
            printk("ERROR: on OTP buffer validation");
            return false;
        }

        //
        // Wait for 9415 bootloader to complete programming the current packet image data from SRAM to the OTP.
        // The boot loader will update the Status in the SRAM as follows:
        //     Status:
        //     "0" - reset value (from AP)
        //     "1" - buffer validated / busy (from AP)
        //     "2" - finish "OK" (from the boot loader)
        //     "4" - programming error (from the boot loader)
        //     "8" - wrong check sum (from the boot loader)
        //     "16"- programming not possible (try to write "0" to bit location already programmed to "1")
        //         (from the boot loader)

        //        DateTime startT = DateTime.Now;
        do {
            mdelay(100);
            di->bus.read(di, 0x400, sBuf);
            if (sBuf[0] == 1) {
                printk("ERROR: Programming OTP buffer status sBuf:%02x i:%d\n", sBuf[0], i);
            }
            if (retry_cnt++ > 5)
                break;
        } while (sBuf[0] == 1); //check if OTP programming finishes "OK"

        if (sBuf[0] != 2) {        // not OK
            printk("ERROR: buffer write to OTP returned status:%d :%s\n" , sBuf[0], "X4");
            return false;
        } else {
            printk(KERN_ERR "Program OTP 0x%04x\n", i);
        }
    }

    // === Step-3 ===
    // Restore system (Need to reset or power cycle 9415 to run the OTP code)
    //
    if (di->bus.write(di, 0x3000, 0x5a)) return false;        // write key
    if (di->bus.write(di, 0x3048, 0x00)) return false;        // remove code remapping
    //if (!di->bus.write(0x3040, 0x80)) return false;    // reset M0
    return true;
}
#endif
static const struct of_device_id match_table[] = {
    {.compatible = "IDT,idt_wireless_power",},
    { },
};

static const struct i2c_device_id p9415_dev_id[] = {
    {"idt_wireless_power", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, p9415_dev_id);

static int p9415_remove(struct i2c_client *client) {
    return 0;
}

// first step: define regmap_config
static const struct regmap_config p9415_regmap_config = {
    .reg_bits = 16,
    .val_bits = 8,
    .max_register = 0xFFFF,
};

#define IDT_REG_ILIMT 0x4a
#define IDT_REG_VOUT_LOW 0x3e
#define IDT_REG_VOUT_HIGH 0x3f
#define VOUT_9V_HIGH_THER 9500
#define VOUT_9V_LOW_THER 5500
#define CURRENT_12V_UA 3000000
#define CURRENT_5V_UA   2000000
#define LIMIT_CURRENT_UA   900000

#define DEFAULT_CURRENT_UA 500000
#define is_between(left, right, value) \
		(((left) >= (right) && (left) >= (value) \
			&& (value) >= (right)) \
		|| ((left) <= (right) && (left) <= (value) \
			&& (value) <= (right)))

struct range_data {
	int low_threshold;
	int high_threshold;
	int value;
};
struct range_data current_9v_tab[] =
	{
		//SOC_LOW	SOC_HIGH	input
			{0,		50,		800000},
			{50,	65,		700000},
			{65,	100,	600000},
};
struct range_data current_5v_tab[] =
	{
		//SOC_LOW	SOC_HIGH	input
			{0,		65,		1000000},
			{65,	100,	700000},
};

static int idt_set_input_current(struct p9415_dev *chip)
{
	union power_supply_propval pval = {0, };
	int current_ua = 0, limit_current_ua = 0,rc = 0;

	pr_err("%s dc_online = %d,dc_9v_or_12v_online=%d\n",__func__,
		chip->dc_online,chip->dc_9v_or_12v_online);
	if(chip->dc_online){
		if(chip->dc_9v_or_12v_online){
			current_ua =  CURRENT_12V_UA;
		} else {
			current_ua =  CURRENT_5V_UA;
		}

		limit_current_ua = LIMIT_CURRENT_UA;
	}
	if((chip->input_max_current_ua > 0) && (limit_current_ua > chip->input_max_current_ua) )
		limit_current_ua = chip->input_max_current_ua;
	if(current_ua < DEFAULT_CURRENT_UA)
		current_ua = DEFAULT_CURRENT_UA;

	pr_err("%s limit_current_ua = %d/\n",__func__,limit_current_ua);

	pval.intval = limit_current_ua;
	pr_err("%s set dc icl current = %d\n",__func__,limit_current_ua);
	rc = power_supply_set_property(chip->dc_psy,
				      POWER_SUPPLY_PROP_CURRENT_MAX, &pval);
	if (rc < 0) {
		pr_err("Couldn't set dc current_max rc=%d\n",
				rc);
		return false;
	}

	pr_err("%s current_ua = %d\n",__func__,current_ua);
	pval.intval = current_ua;
	pr_err("%s set dc fcc current = %d\n",__func__,current_ua);
	rc = power_supply_set_property(chip->dc_psy,
				      POWER_SUPPLY_PROP_CURRENT_MAX, &pval);
	if (rc < 0) {
		pr_err("Couldn't set dc current_max rc=%d\n",
				rc);
		return false;
		vote(chip->fcc_votable, DC_LIMIT_VOTER, true , current_ua);
	}
    return rc;
}

#if 0
static int idt_set_fod_data(struct p9415_dev *di)
{
	int rc = 0;

	u8 fod_data_5v[12] = {0x98,0x38,0x48,0x7f,0x7e,0x44,0x98,0x1e,0x9c,0x13,0xaa,0xe4};
	u8 fod_data_9v[12] = {0x98,0x38,0x48,0x7c,0x6a,0x78,0x9c,0x2,0x9c,0x11,0xa2,0xf0};

	if(di->dc_9v_or_12v_online)
		rc = idtp9415_write_buffer(di,0x68,fod_data_9v,12);
	else
		rc = idtp9415_write_buffer(di,0x68,fod_data_5v,12);

    return rc;
}

static int idt_reset_system_level(struct p9415_dev *chip)
{
	int rc = 0;
	union power_supply_propval pval = {0, };

	rc = power_supply_get_property(chip->battery_psy,
			POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, &pval);
	if (rc < 0) {
		pr_err("Couldn't get charge control limit from battery rc=%d\n", rc);
	} else {
		rc = power_supply_set_property(chip->battery_psy,
			POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, &pval);
		if (rc < 0)
			pr_err("Couldn't set charge control limitrc=%d\n", rc);
		else
			pr_err("%s set level to %d\n", __func__, pval.intval);
	}

    return rc;
}
#endif
int dc_vbus_mv =0;
EXPORT_SYMBOL(dc_vbus_mv);
int get_dc_vout(struct p9415_dev *chip)
{
	u8 vout_data_low = 0, vout_data_high = 0;
	u16 vout_data = 0;

	idtp9415_read(chip,IDT_REG_VOUT_LOW,&vout_data_low);
	idtp9415_read(chip,IDT_REG_VOUT_HIGH,&vout_data_high);
	vout_data = (vout_data_high << 8) |vout_data_low;
	dc_vbus_mv = vout_data  * 84 / 10 + 2800;
	pr_err("idt handler dc in vbus_mv %d\n", dc_vbus_mv);
	return dc_vbus_mv;
}
#if 0
int get_dc_cur_vout(struct p9415_dev *chip)
{
	u8 vout_data_low = 0, vout_data_high = 0;
	u16 vout_data = 0;

	idtp9415_read(chip,0x7c,&vout_data_low);
	idtp9415_read(chip,0x7d,&vout_data_high);
	vout_data = (vout_data_high << 8) |vout_data_low;
	pr_err("idt handler dc current  vbus_mv %d\n", vout_data);
	return vout_data;
}
#endif
int get_dc_vrect_vout(struct p9415_dev *chip)
{
	u8 vout_data_low = 0, vout_data_high = 0;
	u16 vout_data = 0;

	pr_err("idt handler dc current \n");
	idtp9415_read(chip,0x7e,&vout_data_low);
	idtp9415_read(chip,0x7f,&vout_data_high);
	vout_data = (vout_data_high << 8) |vout_data_low;
	pr_err("idt handler dc current  vrect %d\n", vout_data);
	return vout_data;
}
static int idt_set_dc_status(struct p9415_dev *chip, bool is_dc_online, int soc)
{
	int rc = 0,vbus_mv = 0,icl_ma = 0;
	u8 ilimt_data = 0;
	u8 fod_data_5v[12] = {0x98,0x38,0x48,0x7f,0x7e,0x44,0x98,0x1e,0x9c,0x13,0xaa,0xe4};

	if(chip->dc_online != is_dc_online) {
		chip->dc_online = is_dc_online;
		chip->soc = soc;
		pr_err("idt online change %d,soc %d\n",chip->dc_online, soc);
		if(is_dc_online){
			idtp9415_read(chip,IDT_REG_ILIMT,&ilimt_data);
			vbus_mv =get_dc_vout(chip);
			//vbus_mv = get_dc_cur_vout(chip);
			pr_err("idt vbus_mv=%d\n",vbus_mv );
			if(vbus_mv > 9000){
				rc = idtp9415_write(chip,0x68, 0x88);
				if (rc) {
					pr_err("Couldn't write 0x68 \n");
				}
				rc = idtp9415_write(chip,0x69, 0x73);
				if (rc) {
					pr_err("Couldn't write 0x69 \n");
				}
				rc = idtp9415_write(chip,0x3e, 0xe2);
				if (rc) {
					pr_err("Couldn't write 0x3e \n");
				}
				rc = idtp9415_write(chip,0x3f, 0x02);
				if (rc) {
					pr_err("Couldn't write 0x3f \n");
				}
			get_dc_vout(chip);
			schedule_delayed_work(&chip->vrect_check_work,0);
			}else {
				pr_err("idt 5v ok\n" );
				chip->dc_9v_or_12v_online = false;
				rc = idtp9415_write_buffer(chip,0x68,fod_data_5v,12);
				idt_set_input_current(chip);
			}

			icl_ma = ilimt_data*100 + 100;
			pr_err("idt handler dc in,soc %d vbus_mv %d, icl_ma %d ,ilimt_data=%x\n",
				soc, vbus_mv, icl_ma,ilimt_data);
			chip->input_max_current_ua = icl_ma*1000;
			if(icl_ma*1000 < DEFAULT_CURRENT_UA)
				chip->input_max_current_ua = DEFAULT_CURRENT_UA;
		} else {
			pr_err("idt handler dc out,soc %d\n", chip->soc );
			cancel_delayed_work_sync(&chip->vrect_check_work);
		}

	}
    return rc;
}
static void vrect_check_work(struct work_struct *work)
{
	struct p9415_dev *chip = container_of(work,
			struct p9415_dev, vrect_check_work.work);
	u8 fod_data_9v[12] = {0x98,0x38,0x48,0x7c,0x6a,0x78,0x9c,0x2,0x9c,0x11,0xa2,0xf0};
	u16 vrect = 0;
	int rc;

	pr_err("idt vrect_check_work \n" );
	vrect = get_dc_vrect_vout(chip);

	if(vrect < 10000 && vrect >VOUT_9V_LOW_THER){
		pr_err("idt set 9v ok\n" );
		rc = idtp9415_write_buffer(chip,0x68,fod_data_9v,12);
		chip->dc_9v_or_12v_online = true;
		idt_set_input_current(chip);
		//idt_reset_system_level(chip);
	}
	else{
		pr_err("idt vrect_check ok\n" );
		schedule_delayed_work(&chip->vrect_check_work,msecs_to_jiffies(500));
	}
}
static int battery_notifier_call(struct notifier_block *nb,
		unsigned long ev, void *v)
{
	struct power_supply *psy = v;
	struct p9415_dev *chip = container_of(nb, struct p9415_dev, nb);

	if (ev != PSY_EVENT_PROP_CHANGED)
			return NOTIFY_OK;
	if(!strcmp(psy->desc->name, "dc") || !strcmp(psy->desc->name, "battery") || !strcmp(psy->desc->name, "usb"))
		schedule_delayed_work(&chip->idt_status_change_work, 0);
		return NOTIFY_OK;
}
static void idt_status_change_work(struct work_struct *work)
{
	struct p9415_dev *chip = container_of(work,
			struct p9415_dev, idt_status_change_work.work);
	int rc = 0,soc = 0;
	bool online = false;
	union power_supply_propval pval = {0, };

	if (!chip->dc_psy){
		chip->dc_psy = power_supply_get_by_name("dc");
		if (!chip->dc_psy) {
			pr_err("Couldn't get dc psy\n");
			return ;
		}
	}
	rc = power_supply_get_property(chip->dc_psy,
		POWER_SUPPLY_PROP_ONLINE, &pval);
	if (rc < 0) {
		pr_err("Couldn't get online from dc rc=%d\n", rc);
		return ;
	} else {
		online = !!pval.intval;
	}
	if (!chip->battery_psy){
		chip->battery_psy = power_supply_get_by_name("battery");
		if (!chip->battery_psy) {
			pr_err("Couldn't get battery psy\n");
			return ;
		}
	}
	rc = power_supply_get_property(chip->battery_psy,
			POWER_SUPPLY_PROP_CAPACITY, &pval);
	if (rc < 0) {
		pr_err("Couldn't get capacity from battery rc=%d\n", rc);
	} else {
		soc = pval.intval;
	}
	pr_err("%s online=%d\n", __func__,online);
	idt_set_dc_status(chip, online, soc);
}

static irqreturn_t idt_pgood_irq(int irq, void *data)
{
	struct p9415_dev *chip = data;
	u8 fod_data1 ,fod_data2, val;
	int rc =0, epp =0;

	idtp9415_read(chip,0x4d,&val);
	epp = val & BIT(3);
	pr_err("idt_pgood_irq data=0x%x,epp=%d\n",val,epp);

	if(epp){
		rc = idtp9415_write(chip,0x76, 0x88);
		if (rc) {
			pr_err("Couldn't write 0x76 \n");
		}else
			pr_err("write 0x76 done\n");

		rc = idtp9415_write(chip,0x77, 0x73);
		if (rc) {
			pr_err("Couldn't write 0x77 \n");
		}else
			pr_err("write 0x77 done\n");

		idtp9415_read(chip,0x76,&fod_data1);
		idtp9415_read(chip,0x77,&fod_data2);
		pr_err("idt_pgood_irq fod_data1 = %x,fod_data2 = %x\n",
			fod_data1,fod_data2);
	}else
		pr_err("5v do not config\n");

	return IRQ_HANDLED;
}
static int p9415_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct p9415_dev *chip;
    struct device_node *np = client->dev.of_node;
    int rc = 0;

    pr_err("IDTP9415 probe.\n");
    msleep(500);
    chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip)
        return -ENOMEM;

    pr_err("IDTP9415 chip.\n");
    chip->regmap = devm_regmap_init_i2c(client, &p9415_regmap_config);
    if (!chip->regmap) {
        pr_err("parent regmap is missing\n");
        return -EINVAL;
    }
    pr_err("IDTP9415 regmap.\n");
    msleep(500);
    chip->client = client;
    chip->dev = &client->dev;

    chip->bus.read = idtp9415_read;
    chip->bus.write = idtp9415_write;
    chip->bus.read_buf = idtp9415_read_buffer;
    chip->bus.write_buf = idtp9415_write_buffer;

    device_init_wakeup(chip->dev, true);

    idtp9415_create_debugfs_entries(chip);

    idt = chip;
#if 0
    {
        u8 val;
        chip->bus.read(chip, 0x5870, &val);
        printk(KERN_ERR "0x5870 %s:%d :%02x\n", __func__, __LINE__, val);
        chip->bus.read(chip, 0x5874, &val);
        printk(KERN_ERR "0x5874 %s:%d :%02x\n", __func__, __LINE__, val);
    }
    fwver();

    if (get_atapter_type())
        fast_charging(9000);
#endif
	INIT_DELAYED_WORK(&chip->idt_status_change_work, idt_status_change_work);
	INIT_DELAYED_WORK(&chip->vrect_check_work, vrect_check_work);
	chip->nb.notifier_call = battery_notifier_call;
	rc = power_supply_reg_notifier(&chip->nb);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
		return rc;
	}
#if 0
    if (!program_fw(chip, 0x0000, idtp9415_rx_fw, sizeof(idtp9415_rx_fw))) {
        dev_err(&client->dev, "program fw failed.\n");
    }

#endif
	chip->fcc_votable = find_votable("FCC");
	if (chip->fcc_votable == NULL) {
		rc = -EINVAL;
		pr_err("IDTP9415 Couldn't find FCC votable rc=%d\n", rc);
		return rc;
	}

	chip->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (chip->irq_gpio < 0)
		pr_err("%s: no irq gpio provided.\n", __func__);
	else
		pr_err( "%s: irq gpio provided ok.\n", __func__);

	if (gpio_is_valid(chip->irq_gpio)) {
		rc = devm_gpio_request_one(&client->dev, chip->irq_gpio,
					    GPIOF_DIR_IN, "idt_pgood");
		if (rc) {
			pr_err( "%s:idt_pgood request failed\n",
				__func__);
			return rc;
		}

		rc = devm_request_threaded_irq(&client->dev,
						gpio_to_irq(chip->irq_gpio),
						NULL, idt_pgood_irq, IRQF_TRIGGER_RISING | IRQF_ONESHOT,
						"idtp9415", chip);
		if (rc < 0) {
			pr_err("idt Couldn't request irq\n");
			return rc;
		}
	}

    pr_err("IDTP9415 probed successfully\n");

    return rc;
}

static struct i2c_driver p9415_driver = {
    .driver   = {
        .name           = "idt_wireless_power",
        .owner          = THIS_MODULE,
        .of_match_table = match_table,
    },
    .probe    = p9415_probe,
    .remove   = p9415_remove,
    .id_table = p9415_dev_id,
};
module_i2c_driver(p9415_driver);

MODULE_AUTHOR("Integrated Device Technology Co., Ltd <simon.song@idt.com>");
MODULE_DESCRIPTION("P9415 Wireless Power Charger Monitor driver");
MODULE_LICENSE("GPL v2");
