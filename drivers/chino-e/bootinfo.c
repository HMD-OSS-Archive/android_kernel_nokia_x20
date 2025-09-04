
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <video/mmp_disp.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/io.h>
//#include <soc/qcom/scm.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
static struct kobject *bootinfo_kobj = NULL;

const u8 * sub_front_camera[]={"sub_front_camera not found!","Sunny_s5kgd1sp","Seasons_s5k4h7"}; //front_camera module information
const u8 * main_camera[]={"main_camera not found!","Qtech_s5kgm1st","Qtech_s5kgw3sp"}; // main_camera module information
const u8 * wide_camera[]={"wide_camera not found!","Seasons_gc5035","Cxt_gc5035"}; //wide_camera module information
const u8 * depth_camera[]={"depth_camera not found!","Seasons_gc02m1b","Cxt_ov02b1b"}; //depth_camera module information
const u8 * macro_camera[]={"macro_camera not found!","Seasons_gc02m1","Cxt_ov02b10"}; //macro_camera module information

int torch_flash_level=0;
int main_camera_find_success = 0;
int wide_camera_find_success = 0;
int depth_camera_find_success = 0;
int macro_camera_find_success = 0;
int front_camera_find_success = 0;

bool main_camera_probe_ok = 0;
bool wide_camera_probe_ok = 0;
bool depth_camera_probe_ok = 0;
bool macro_camera_probe_ok = 0;
bool front_camera_probe_ok = 0;

EXPORT_SYMBOL(main_camera_find_success);
EXPORT_SYMBOL(wide_camera_find_success);
EXPORT_SYMBOL(depth_camera_find_success);
EXPORT_SYMBOL(macro_camera_find_success);
EXPORT_SYMBOL(front_camera_find_success);
EXPORT_SYMBOL(main_camera_probe_ok);
EXPORT_SYMBOL(wide_camera_probe_ok);
EXPORT_SYMBOL(depth_camera_probe_ok);
EXPORT_SYMBOL(macro_camera_probe_ok);
EXPORT_SYMBOL(front_camera_probe_ok);

extern char *panel_name_find;
extern char touch_version[32];
extern char fp_version[32];
extern char smb_version[32];
extern char pmic_version[32];
extern char bat_name[32];
extern char nfc_version[32];

static ssize_t lcd_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",panel_name_find);
	return (s - buf);
}

static ssize_t lcd_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute lcd_info_attr = {
	.attr = {
		.name = "lcd_info",
		.mode = 0644,
	},
	.show =&lcd_info_show,
	.store= &lcd_info_store,
};

static ssize_t tp_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",touch_version);
	return (s - buf);
}

static ssize_t tp_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute tp_info_attr = {
	.attr = {
		.name = "tp_info",
		.mode = 0644,
	},
	.show =&tp_info_show,
	.store= &tp_info_store,
};

static ssize_t fp_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n",fp_version);
	return (s - buf);
}

static ssize_t fp_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute fp_info_attr = {
	.attr = {
		.name = "fp_info",
		.mode = 0644,
	},
	.show =&fp_info_show,
	.store= &fp_info_store,
};

static ssize_t smb_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n", smb_version);
	return (s - buf);
}

static ssize_t smb_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute smb_info_attr = {
	.attr = {
		.name = "smb_info",
		.mode = 0644,
	},
	.show =&smb_info_show,
	.store= &smb_info_store,
};

static ssize_t pmic_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n", pmic_version);
	return (s - buf);
}

static ssize_t pmic_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute pmic_info_attr = {
	.attr = {
		.name = "pmic_info",
		.mode = 0644,
	},
	.show =&pmic_info_show,
	.store= &pmic_info_store,
};

static ssize_t bat_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n", bat_name);
	return (s - buf);
}

static ssize_t bat_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute bat_info_attr = {
	.attr = {
		.name = "bat_info",
		.mode = 0644,
	},
	.show =&bat_info_show,
	.store= &bat_info_store,
};

static ssize_t nfc_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	s += sprintf(s, "%s\n", nfc_version);
	return (s - buf);
}

static ssize_t nfc_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute nfc_info_attr = {
	.attr = {
		.name = "nfc_info",
		.mode = 0644,
	},
	.show =&nfc_info_show,
	.store= &nfc_info_store,
};


#define BASE_ADDRESS  	   0x000A0138
#define BIT_COUNT_SBL1	   0x00000FFE
#define BIT_COUNT_APPSBL0  0xFFFC0000
#define BIT_COUNT_APPSBL1  0xFFFFFFFF
#define BIT_COUNT_APPSBL2  0x0000000F
#define BIT_COUNT_MSS	   0xFFFF0000
#define JTAG_ADDRESS  	   0x000A0150
#define BASE_JTAG_OFFSET   (BASE_ADDRESS-JTAG_ADDRESS)
#define ANTI_BACK_ADDR_LEN 24
#define JUDGE_BIT(D,N)     (( D >> N) & 1)         
void *ptr;
char show_ver[10];
phys_addr_t p = JTAG_ADDRESS;

static int countBits(unsigned int n) {
	int count = 0;
	while(n != 0) {
		n = n & (n-1);
		count++;
	}
	return count;
}

static char *show_tag(size_t cnt){
	char *tag;
	switch(cnt){
		case 1: 
			tag = "1";
			break;
		case 2: 
			tag = "2";
			break;
		case 3: 
			tag = "3";
			break;
		case 4: 
			tag = "4";
			break;
		case 5: 
			tag = "5";
			break;
		case 6: 
			tag = "6";
			break;
		case 7: 
			tag = "7";
			break;
		case 8: 
			tag = "8";
			break;
		case 9: 
			tag = "9";
			break;
		case 10: 
			tag = "A";
			break;
		case 11: 
			tag = "B";
			break;
		case 12: 
			tag = "C";
			break;
		case 13: 
			tag = "D";
			break;
		default: 
			tag = "F";
			break;
	}
	return tag;
}

static inline unsigned long size_inside_page(unsigned long start,
                                             unsigned long size)
{
        unsigned long sz;

        sz = PAGE_SIZE - (start & (PAGE_SIZE - 1));

        return min(sz, size);
}

static ssize_t read_address(char *buf,int offset,int cnt)
{
	ssize_t read,sz;
	ssize_t count = cnt;
	char pm[ANTI_BACK_ADDR_LEN] = {'\0'};
	read = 0;

	while (count > 0) {

		sz = size_inside_page(p+offset, count);
		if (!ptr)
			return -EFAULT;

		memcpy(pm,ptr+offset,sz);
		count -= sz;
		p += sz;
		read += sz;
	}
	memcpy(buf,pm,sizeof(pm));
	return read;
}
static ssize_t image_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	size_t ret;
	int addrLen = ANTI_BACK_ADDR_LEN;
	char verBuf[ANTI_BACK_ADDR_LEN] = {'\0'};
	char *ver = "V-";
	
	unsigned int modem = 0;
	unsigned int sbl1 = 0;
	unsigned int appsbl0 = 0;
	unsigned int appsbl1 = 0;
	unsigned int appsbl2 = 0;
	ret = read_address(verBuf,BASE_JTAG_OFFSET,addrLen);
	if(!ret)
		printk("img version get err\n");
	
	memset(show_ver,'\0',sizeof(show_ver));
	memcpy(show_ver,ver,strlen(ver));
	
	//modem:base+0x0010[31:16]
	memcpy(&modem,verBuf+0x0010,4);
	modem &= BIT_COUNT_MSS;
	ret = countBits(modem);
	memcpy(show_ver+strlen(ver),show_tag(ret),1);
		
	//SBL1:base+0x0000[11:1]
	ret = 0;
	memcpy(&sbl1,verBuf,4);
	sbl1 &= BIT_COUNT_SBL1;
	ret = countBits(sbl1);
	memcpy(show_ver+strlen(ver)+1,show_tag(ret),1);

	//Appsbl:base+0x0004[31:18];base+0x0008[31:0];base+0x000c[3:0]
	ret = 0;
	memcpy(&appsbl0,verBuf+0x0004,4);
	appsbl0 &= BIT_COUNT_APPSBL0;
	ret += countBits(appsbl0);

	memcpy(&appsbl1,verBuf+0x0008,4);
	appsbl1 &= BIT_COUNT_APPSBL1;
	ret += countBits(appsbl1);
	
	memcpy(&appsbl2,verBuf+0x000c,4);
	appsbl2 &= BIT_COUNT_APPSBL2;
	ret += countBits(appsbl2);
	
	memcpy(show_ver+strlen(ver)+1+1,show_tag(ret),1);

	return sprintf(buf,"%s\n",show_ver);
 }

static struct kobj_attribute sboot_image_info_attr = { 
        .attr = { 
                .name = "hw_image_info",
                .mode = 0444,
        },
        .show =&image_info_show,
};

static ssize_t jtag_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	size_t ret;
        int addrLen = ANTI_BACK_ADDR_LEN;
        char verBuf[ANTI_BACK_ADDR_LEN] = {'\0'};
	char *str;
        ret = read_address(verBuf,0,addrLen);
        if(!ret)
                printk("img version get err\n");
	
	//base:0x000A0150 +bit[36]
	if( JUDGE_BIT(verBuf[4],4) &&		
	     //base:0x000A0150 +bit[42]
	     JUDGE_BIT(verBuf[5],2) &&		
             //base:0x000A0150 +bit[44]
	     JUDGE_BIT(verBuf[5],4) &&		
	     //base:0x000A0150 +bit[46]
	     JUDGE_BIT(verBuf[5],6) &&		
	     //base:0x000A0150 +bit[48]
	     JUDGE_BIT(verBuf[6],0)) {		
		str = "Blown";
	}else{
		str = "Not Blown";
	}	
	return sprintf(buf,"%s\n",str);	
}
static struct kobj_attribute sboot_jtag_fuse_attr = { 
        .attr = { 
                .name = "hw_jtag_info",
                .mode = 0444,
        },
        .show =&jtag_info_show,
};
//static ssize_t efuse_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
//{
//	return sprintf(buf, "%d\n", scm_is_secure_device()? 0:1);
//}

//static struct kobj_attribute sboot_efuse_info_attr = {
//	.attr = {
//		.name = "hw_efuse_info",
//		.mode = 0444,
//	},
//	.show =&efuse_info_show,
//};
static ssize_t main_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((main_camera_find_success >= sizeof(main_camera) / sizeof(char *)) || (main_camera_find_success<0)) {
		s += sprintf(s, "%s\n", main_camera[0]);
	} else {
		s += sprintf(s, "0_%s\n", main_camera[main_camera_find_success]);
	}	
	return (s - buf);
}

static ssize_t main_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute main_camera_info_attr = {
	.attr = {
		.name = "main_camera",
		.mode = 0644,
	},
	.show =&main_camera_info_show,
	.store= &main_camera_info_store,
};

static ssize_t front_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

	if((front_camera_find_success>=sizeof(sub_front_camera) / sizeof(char *)) || (front_camera_find_success<0)) {
		s += sprintf(s, "%s\n", sub_front_camera[0]);
	} else {
		s += sprintf(s, "1_%s\n", sub_front_camera[front_camera_find_success]);
	}
	return (s - buf);
}

static ssize_t front_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute front_camera_info_attr = {
	.attr = {
		.name = "front_camera",
		.mode = 0644,
	},
	.show =&front_camera_info_show,
	.store= &front_camera_info_store,
};

static ssize_t wide_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((wide_camera_find_success >= sizeof(wide_camera) / sizeof(char *)) || (wide_camera_find_success < 0)) {
		s += sprintf(s, "%s\n", wide_camera[0]);
	}else{
		s += sprintf(s, "3_%s\n", wide_camera[wide_camera_find_success]);
	}	
	return (s - buf);
}

static ssize_t wide_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute wide_camera_info_attr = {
	.attr = {
		.name = "wide_camera",
		.mode = 0644,
	},
	.show =&wide_camera_info_show,
	.store= &wide_camera_info_store,
};
static ssize_t depth_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((depth_camera_find_success >= sizeof(depth_camera) / sizeof(char *)) || (depth_camera_find_success < 0)) {
		s += sprintf(s, "%s\n", depth_camera[0]);
	} else {       
		s += sprintf(s, "2_%s\n", depth_camera[depth_camera_find_success]);
	}	
	return (s - buf);
}

static ssize_t depth_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute depth_camera_info_attr = {
	.attr = {
		.name = "depth_camera",
		.mode = 0644,
	},
	.show =&depth_camera_info_show,
	.store= &depth_camera_info_store,
};

static ssize_t macro_camera_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	if((macro_camera_find_success >= sizeof(macro_camera) / sizeof(char *)) || (macro_camera_find_success < 0)) {
		s += sprintf(s, "%s\n", macro_camera[0]);
	} else {
		s += sprintf(s, "4_%s\n", macro_camera[macro_camera_find_success]);
	}	
	return (s - buf);
}

static ssize_t macro_camera_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{
	return n;
}
static struct kobj_attribute macro_camera_info_attr = {
	.attr = {
		.name = "macro_camera",
		.mode = 0644,
	},
	.show = &macro_camera_info_show,
	.store= &macro_camera_info_store,
};

#if 0
static ssize_t torch_flash_onoff_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	return sprintf(buf, "%d\n", torch_flash_level); //? 1:0);
}
static ssize_t torch_flash_onoff_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n)
{

    int res=0, temp_level=0;
    res = kstrtouint(buf, 10, &temp_level);

    if( (temp_level >= 0)&&(temp_level < 8) )
    {
        torch_flash_level = temp_level;
        ontim_torch_onoff(torch_flash_level);
    }
	return n;
}
static struct kobj_attribute touch_flash_onoff_info_attr = {
	.attr = {
		.name = "torch_flash",
		.mode = 0644,
	},
	.show =&torch_flash_onoff_info_show,
	.store= &torch_flash_onoff_info_store,
};
#endif

#if 0
static struct kobj_attribute modem_info_attr = {
	.attr = {
		.name = "modem_info",
		.mode = 0444,
	},
	.show = &modem_show,
};
#endif

#if 0
static ssize_t i2c_devices_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
	int tmp=0;

	tmp|= (tp_probe_ok<<0);
	tmp|= (front_camera_probe_ok<<1);
	tmp|= (main_camera_probe_ok<<2);
	tmp|= (gsensor_probe_ok<<3);
	tmp|= (proximity_probe_ok<<4);
	tmp|= (charger_probe_ok<<5);
	tmp|= (pmu_probe_ok<<6);
	tmp|= (compass_probe_ok<<7);
        tmp|= (wide_camera_probe_ok<<8);
        tmp|= (depth_camera_probe_ok<<9);
        tmp|= (macro_camera_probe_ok<<10);
	tmp|= (fingerprint_probe_ok<<31);
	tmp|= (nfc_probe_ok<<11);

	s += sprintf(s, "0x%x\n",tmp);
	return (s - buf);
}
static struct kobj_attribute i2c_devices_info_attr = {
	.attr = {
		.name = "i2c_devices_probe_info",
		.mode = 0444,
	},
	.show =&i2c_devices_info_show,
};
#endif

#if 0
#if  defined (CONFIG_ARM64)  //Titan_TL PRJ
int get_pa_num(void)
{

#define GPIO_VERSION_PIN1    (GPIO95 | 0x80000000)
#define GPIO_VERSION_PIN2    (GPIO96 | 0x80000000)
#define GPIO_VERSION_PIN3    (GPIO93 | 0x80000000)
#define GPIO_VERSION_PIN4    (GPIO94 | 0x80000000)
	
	int pin1_val = 0, pin2_val = 0, pin3_val = 0, pin4_val = 0;

      mt_set_gpio_pull_select(GPIO_VERSION_PIN1, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN1, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN1, GPIO_DIR_IN);

      mt_set_gpio_pull_select(GPIO_VERSION_PIN2, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN2, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN2, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN2, GPIO_DIR_IN);

      mt_set_gpio_pull_select(GPIO_VERSION_PIN3, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN3, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN3, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN3, GPIO_DIR_IN);

      mt_set_gpio_pull_select(GPIO_VERSION_PIN4, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN4, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN4, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN4, GPIO_DIR_IN);

	mdelay(20);

	pin1_val = mt_get_gpio_in(GPIO_VERSION_PIN1);
	pin2_val = mt_get_gpio_in(GPIO_VERSION_PIN2);
	pin3_val = mt_get_gpio_in(GPIO_VERSION_PIN3);
	pin4_val = mt_get_gpio_in(GPIO_VERSION_PIN4);
	
	printk(KERN_ERR "%s:  pin1 is %d, pin2 is %d, pin3 is %d, pin4 is %d\n",__func__, pin1_val, pin2_val,pin3_val,pin4_val);
	
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_DISABLE);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN2, GPIO_PULL_DISABLE);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN3, GPIO_PULL_DISABLE);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN4, GPIO_PULL_DISABLE);


#if  !defined (CONFIG_MTK_C2K_SUPPORT)  //Titan_TL
     if (pin1_val && pin4_val ) 
     {
         return 1;
     }
     else 
     {
         return 0;
     }
#else
     if (0) //( pin2_val && ( !pin3_val) && ( !pin4_val))
     {
         return 1;
     }
     else 
     {
         return 0;
     }
#endif
}
#else
int get_pa_num(void)
{
//baixue add for disable second PA
      return 0;


#define GPIO_VERSION_PIN1    (GPIO96 | 0x80000000)
	
	int pin1_val = 0, pin2_val = 0;

      mt_set_gpio_pull_select(GPIO_VERSION_PIN1, GPIO_PULL_UP);
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_ENABLE);
      mt_set_gpio_mode(GPIO_VERSION_PIN1, GPIO_MODE_00);
      mt_set_gpio_dir(GPIO_VERSION_PIN1, GPIO_DIR_IN);

	mdelay(10);
	
	pin1_val = mt_get_gpio_in(GPIO_VERSION_PIN1);
	
	printk(KERN_ERR "%s:  pin1 is %d\n",__func__, pin1_val);
	
      mt_set_gpio_pull_enable(GPIO_VERSION_PIN1, GPIO_PULL_DISABLE);


      return !pin1_val;
	
}
#endif

static ssize_t RF_PA_info_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;
//	u8 string[5]={'\0'};
	int tmp=0;
	
	tmp = get_pa_num();

	s += sprintf(s, "%d\n",tmp);
	
	return (s - buf);
}

static struct kobj_attribute RF_PA_info_attr = {
	.attr = {
		.name = "RF_PA_Type",
		.mode = 0444,
	},
	.show =&RF_PA_info_show,
};
#endif

#include <linux/gpio.h>
int get_hw_prj(void)
{
	unsigned int gpio_base =343;
	unsigned int pin0=93;
	unsigned int pin1=92;

	int pin_val = 0;
	int hw_prj=0;

	
	pin_val =    gpio_get_value(gpio_base+pin0) & 0x01;
	pin_val |= (gpio_get_value(gpio_base+pin1) & 0x01) << 1;
	hw_prj = pin_val;
	
	printk(KERN_ERR "%s: hw_prj is %x ;\n",__func__, hw_prj);

	return  hw_prj;
	
}
static ssize_t get_hw_prj_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

      s += sprintf(s, "0x%02x\n",get_hw_prj());
	
	return (s - buf);
	
}

static struct kobj_attribute get_hw_prj_attr = {
	.attr = {
		.name = "hw_prj",
		.mode = 0644,
	},
	.show =&get_hw_prj_show,
};

int get_hw_ver_info(void)
{
    unsigned int gpio_base =343;
	unsigned int pin0=121;
	unsigned int pin1=54;
	unsigned int pin2=53;
	unsigned int pin3=5;
	unsigned int pin4=11;
	int pin_val = 0;
	int hw_ver=0;

	
	pin_val =    gpio_get_value(gpio_base+pin0) & 0x01;
	pin_val |= (gpio_get_value(gpio_base+pin1) & 0x01) << 1;
	pin_val |= (gpio_get_value(gpio_base+pin2) & 0x01) << 2;
	pin_val |= (gpio_get_value(gpio_base+pin3) & 0x01) << 3;
	pin_val |= (gpio_get_value(gpio_base+pin4) & 0x01) << 4;
	hw_ver = pin_val;
	
	printk(KERN_ERR "%s: hw_ver is %x ;\n",__func__, hw_ver);

	return  hw_ver;

}
static ssize_t get_hw_ver_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

      s += sprintf(s, "0x%02x\n",get_hw_ver_info());
	
	return (s - buf);
}

static struct kobj_attribute get_hw_ver_attr = {
	.attr = {
		.name = "hw_ver",
		.mode = 0644,
	},
	.show =&get_hw_ver_show,
};

#if 0
static void check_cust_ver(void){
    int rb_flag = 0;;
    struct hw_ver * hw_ver_info=get_hw_ver_info();
    if ( hw_ver_info == NULL )
    {
    	printk(KERN_ERR "%s: Get PRJ info Error!!!\n",__func__);
        return;
    }
    printk(KERN_ERR "%s: Build PRJ is %s, This PRJ is %s!!\n",__func__, PRJ_NAME, hw_ver_info->name);
    if((!strcmp(hw_ver_info->name, PRJ_NAME)) || (!strcmp(hw_ver_info->name, "NULL"))){
    	printk(KERN_ERR "%s: Version Pass!!\n",__func__);
    }
    else
    {
        printk(KERN_ERR "%s: Version Error!!!\n",__func__);
       // kernel_restart("prjerr");
    }
}
#endif

int get_sd_status_info(void)
{ 
	unsigned int cd_gpios =449;
	int pin_val = 0;
	
	pin_val = gpio_get_value(cd_gpios);
	
	printk(KERN_ERR "%s: hw_sd_tray is %d ;\n",__func__, !!pin_val);

	return  !!pin_val;

}
static ssize_t get_sd_status_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf)
{
	char *s = buf;

	s += sprintf(s, "%d\n",get_sd_status_info());
	
	return (s - buf);
}

static struct kobj_attribute get_sd_status_attr = {
	.attr = {
		.name = "hw_sd_tray",
		.mode = 0644,
	},
	.show =&get_sd_status_show,
};

static struct attribute * g[] = {
	&get_hw_prj_attr.attr,
	&get_hw_ver_attr.attr,
	&tp_info_attr.attr,
	&lcd_info_attr.attr,
	&get_sd_status_attr.attr,
	&fp_info_attr.attr,
	&smb_info_attr.attr,
	&pmic_info_attr.attr,
	&bat_info_attr.attr,
	&nfc_info_attr.attr,
	//&rpmb_key_attr.attr,
	//&modem_info_attr.attr,
	//&lcd_driving_mode_set_attr.attr,
	//&i2c_devices_info_attr.attr,
	&main_camera_info_attr.attr,
	&wide_camera_info_attr.attr,
	&depth_camera_info_attr.attr,
	&macro_camera_info_attr.attr,
	&front_camera_info_attr.attr,
	//&touch_flash_onoff_info_attr.attr,
	//&enemmd_attr.attr,
	//&RF_PA_info_attr.attr,
	//&sboot_efuse_info_attr.attr,
	&sboot_image_info_attr.attr,
	&sboot_jtag_fuse_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};

#if 0
int touchscreen_has_steel_film=0;
static int __init touchscreen_film_setup(char *str)
{
	int en;
	if(!get_option(&str, &en))
		return 0;
	touchscreen_has_steel_film = en;
	return 1;
}

__setup("tp_film=", touchscreen_film_setup);

int get_touchscreen_film_state(void)
{
	printk("[kernel]:touchscreen_has_steel_film=%d.\n",touchscreen_has_steel_film);
	return touchscreen_has_steel_film;
}
#endif
#if 0
int lcm_id=0x83;
static int __init lcm_id_setup(char *str)
{
        int en;
        if(!get_option(&str, &en))
                return 0;
        lcm_id = en;
        return 1;
}
int get_lcm_id(void)
{
        printk("[kernel]:get_lcm_id=%x.\n",lcm_id);
        return lcm_id;
}
__setup("lcm_id=", lcm_id_setup);
#endif

static int __init bootinfo_init(void)
{
	int ret = -ENOMEM;
	
	//printk("%s,line=%d\n",__func__,__LINE__);  

	bootinfo_kobj = kobject_create_and_add("ontim_bootinfo", NULL);

	if (bootinfo_kobj == NULL) {
		printk("bootinfo_init: kobject_create_and_add failed\n");
		goto fail;
	}

	ret = sysfs_create_group(bootinfo_kobj, &attr_group);
	if (ret) {
		printk("bootinfo_init: sysfs_create_group failed\n");
		goto sys_fail;
	}
	    
	if(!request_mem_region(p,SZ_4K*16,"anti_rollback")){
		printk("Failed to request core resources");
		goto sys_fail;
	}
		
	ptr = ioremap(p,SZ_4K*16);
	if (!ptr) {
		printk("error to ioremap anti rollback addr\n");
		goto map_fail;
	}	
	return ret;
map_fail:
	release_mem_region(p,SZ_4K*16);
sys_fail:
	kobject_del(bootinfo_kobj);
fail:
	return ret;

}


static void __exit bootinfo_exit(void)
{

	if (bootinfo_kobj) {
		sysfs_remove_group(bootinfo_kobj, &attr_group);
		kobject_del(bootinfo_kobj);
	}

	if(ptr)
		iounmap(ptr);
	
	release_mem_region(p,SZ_4K*16);
}

arch_initcall(bootinfo_init);
module_exit(bootinfo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Boot information collector");
