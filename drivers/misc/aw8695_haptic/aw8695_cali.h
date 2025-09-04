#ifndef __AW8624_CALI_H__
#define __AW8624_CALI_H__

//#define AW_CALI_STORE_EXAMPLE
//#define AW_F0_BRINGUP_CALI

int aw8695_set_cali_lra_to_nvram(char cali_lra);
int aw8695_get_cali_lra_from_nvram(char *cali_lra);

#endif