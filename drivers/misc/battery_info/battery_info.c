/*
 * 2024.05.08
 * added by feihui at 20240508
 */

#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/utsname.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#define LOG_TAG "Battery_Info"
#include "battery_log.h"
#define BATTERY_SN "batterysnvalidate"
#define BATTERY_LENGTH 40

int battery_sn_validate_result = -1;
const char alphabet_table[31] =
    {'1', '2', '3', '4','5','6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', /*'I',*/ 'J', 'K', 'L', 'M', 'N',
    /*'O',*/ 'P', /*'Q',*/ 'R', 'S', 'T', /*'U',*/ 'V', 'W', 'X', 'Y', 'Z' };


/**
 * check leap year
 * @result 1 pass ;0 fail
*/
static int is_leap_year(unsigned int year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

/**
 * check date valid
 * @result  1 pass ;0 fail
 *
*/
int is_date_valid(unsigned int year, unsigned int month, unsigned int day) {
    const unsigned int days_in_month[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (year == 0 || month == 0 || month > 12 || day == 0 || day > 31)
        return 0;

    if (month != 2) {
        /* not Feb */
        return day <= days_in_month[month];
    } else {
        /* Feb need to check leap year */
        return day <= days_in_month[month] + is_leap_year(year);
    }
}

/**
 * check input value
 * @result 0 fail;  1~31 true
*/
static int get_index_by_value(char value){
    int i;
    for (i = 0 ; i < 31; i++){
        if (alphabet_table[i] == value)
            return i+1;
    }
    return 0;
}

/**
 * strSN sn date like A4130001
 * A : Line No.
 * 4 : Year  from 2020 ;Here mans 2024
 * 1 : month
 * 3 : day
 * 0001: Pipeline code
 * @return 0 fail  1 true
*/
int validateDateTime(char* strSN) {
    uint16_t year = get_index_by_value(strSN[1]);
    uint16_t month = get_index_by_value(strSN[2]);
    uint16_t day = get_index_by_value(strSN[3]);
    if (year <= 0 ||  (month <= 0 || month>12) || day  <= 0 ){
        Loge("Invalid SN value:%s", strSN);
        return 0;
    }
    year = year + 2020;
    Logd("sn date [%d-%d-%d]",year,month,day);
    return is_date_valid(year,month,day);
}

static ssize_t show_sn_validate(struct device *dev,struct device_attribute *attr,char *buf) {
    Logd("battery_sn_validate_result %d",battery_sn_validate_result);
    return sprintf(buf, "%d", battery_sn_validate_result);
}

/**
 * check battery sn
 *  pass  battery_sn_validate_result  0
 *  fail     battery_sn_validate_result  -1
*/
// Modify by feihui for [TCT-3473] Fastboot tool 20240515 Begin
static ssize_t set_sn_validate(struct device *dev,
                 struct device_attribute *attr,
                 const char *buf, size_t len)  {
    const char* battery_sn = NULL;
    char battery_sn_data[BATTERY_LENGTH] = {0};
    Logd("E len %d buf %s",len,buf);
    battery_sn_validate_result = 0;
    battery_sn = buf;
    if (battery_sn == NULL || battery_sn[0] == '\0' || len != 20) {
        Loge("X, battery SN is error");
        battery_sn_validate_result = -1;
        return -EINVAL;
    }
    Logd("battery_sn is [%s]", battery_sn);
    memcpy(battery_sn_data,battery_sn+12,8);
    Logd("battery_sn_data is [%s]", battery_sn_data);
    //set validate result, please @see validateDateTime
    battery_sn_validate_result = validateDateTime(battery_sn_data) - 1;
    if(battery_sn_validate_result<0) {
        Loge("X, battery SN is error");
        return -EINVAL;
    }
    Logd("X");
    return len;
}
    // Modify by feihui for [TCT-3473] Fastboot tool 20240515 End

static DEVICE_ATTR(sn_validate, 0644, show_sn_validate, set_sn_validate);

static const struct of_device_id battery_info_match_table[] = {
    { .compatible = "qcom,battery_info"},
    {},
};

static struct platform_device battery_info_device = {
    .name = "battery_ext",
    .id = -1,
};

static int battery_info_probe(struct platform_device *pdev) {
    int ret = 0;
    Logi("battery_info_probe...");
    ret = device_create_file(&(battery_info_device.dev),&dev_attr_sn_validate);
    if(ret) {
        Loge("faile to device_create_file sn_validate");
    }
    return ret;
}

static int battery_info_remove(struct platform_device *pdev) {
    Logi("battery_info driver remove...");
    return 0;
}

static struct platform_driver battery_info_driver = {
    .driver = {
        .name = "battery_info",
        .owner = THIS_MODULE,
        .of_match_table = battery_info_match_table
    },
    .probe = battery_info_probe,
    .remove = battery_info_remove,
};

static int __init battery_info_init(void) {
    int rc;
    Logi("battery_info device init...");
    rc = platform_device_register(&battery_info_device);
    if(rc) {
        Loge("Failed to register battery info device");
        return rc;
    }

    Logi("battery_info driver init...");
    rc = platform_driver_register(&battery_info_driver);
    if (rc) {
        Loge("Failed to register battery info driver");
        return rc;
    }
    return rc;
}

static void __exit battery_info_exit(void) {
    Logi("battery_info driver exit...");
    platform_driver_unregister(&battery_info_driver);
    device_remove_file(&(battery_info_device.dev),&dev_attr_sn_validate);
    platform_device_unregister(&battery_info_device);
}

module_init(battery_info_init);
module_exit(battery_info_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Battery info driver");
