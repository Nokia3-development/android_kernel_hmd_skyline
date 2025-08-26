// SPDX-License-Identifier: GPL-2.0

#include <linux/err.h>    /* IS_ERR, PTR_ERR */
#include <linux/init.h>        /* For init/exit macros */
#include <linux/kernel.h>
#include <linux/module.h>    /* For MODULE_ marcros  */
#include <linux/of_fdt.h>    /*of_dt API*/
#include <linux/of.h>
#include <linux/platform_device.h>    /* platform device */
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/power/an_psy_sysfs.h>
// Modify by huangzhongjie for QN7026A-590 on 20240426 begin
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>

// Modify by liufurong for TCT-837 on 20240514 begin
extern struct kobject *get_root_kobjs(void);
#define PMIC_INFO    "PM8350BH"
// Modify by liufurong for TCT-837 on 20240514 end

struct regulator *vreg = NULL;
// Modify by huangzhongjie for QN7026A-590 on 20240426 end
// Modify by liufurong for TCT-2763 on 20240506 begin
#define POWER_ON    "boot.mode=power.on"
#define POWER_OFF   "boot.mode=power.off"
// Modify by liufurong for TCT-2763 on 20240506 end

// Modify by wenyaqi for TCT-978 on 20240117 start
static const char * const AN_BATTERY_TYPE_TEXT[] = {
    [AN_BATTERY_TYPE_GY]    = "1:battery-GaoYuan",
    [AN_BATTERY_TYPE_UNKNOWN] = "UNKNOWN",
};
// Modify by wenyaqi for TCT-978 on 20240117 end

// Modify by wenyaqi for TCT-969 on 20240118 start
static const char * const AN_CHG_INFO_TEXT[] = {
    [AN_CHG_SC8541]    = "1:chg-SC8541",
    [AN_CHG_UPM6720]    = "2:chg-UPM6720",
    [AN_CHG_INFO_UNKNOWN] = "chg-UNKNOWN",
};
// Modify by wenyaqi for TCT-969 on 20240118 end

// Modify by jiashixian for QN7026A-157 on 20240226 begin
static const char * const AN_WLS_CHG_MODE_TEXT[] = {
    [AN_WLS_CHG_MODE_BACK_POWER]        = "1:MODE_BACK_POWER",
    [AN_WLS_CHG_MODE_MST_ON_PASSIVE]    = "2:MODE_MST_ON_PASSIVE",
    [AN_WLS_CHG_MODE_MST_ON_ACTIVE]     = "3:MODE_MST_ON_ACTIVE",
    [AN_WLS_CHG_MODE_TX_MODE_ON]        = "4:MODE_TX_MODE_ON",
    [AN_WLS_CHG_MODE_TX_FOD_CONFILCT]   = "5:MODE_TX_FOD_CONFILCT",
    [AN_WLS_CHG_MODE_TX_PHM]            = "6:MODE_TX_PHM",
    [AN_WLS_CHG_MODE_AC_MIS]            = "7:MODE_AC_MIS",
    [AN_WLS_CHG_MODE_WPC_BPP]           = "8:MODE_WPC_BPP",
    [AN_WLS_CHG_MODE_WPC_EPP]           = "9:MODE_WPC_EPP",
    [AN_WLS_CHG_MODE_MPP_RESTRICT]      = "10:MODE_MPP_RESTRICT",
    [AN_WLS_CHG_MODE_MPP_FULL]          = "11:MODE_MPP_FULL",
    [AN_WLS_CHG_MODE_MPP_CLOAK]         = "12:MODE_MPP_CLOAK",
    [AN_WLS_CHG_MODE_MPP_NEGO]          = "13:MODE_MPP_NEGO",
    [AN_WLS_CHG_MODE_EPP_NEGO]          = "14:MODE_EPP_NEGO",
    [AN_WLS_CHG_MODE_UNKNOWN]           = "UNKNOWN",
};
static const char * const AN_WLS_CHG_INFO_TEXT[] = {
    [AN_WLS_CHG_CPS4038]      = "1:wls-CPS4038",
    [AN_WLS_CHG_INFO_UNKNOWN] = "UNKNOWN",
};
// Modify by jiashixian for QN7026A-157 on 20240226 end
/* include attrs for battery psy */
static struct device_attribute an_battery_attrs[] = {
    AN_BATTERY_ATTR(input_suspend),
    // Modify by wenyaqi for TCT-978 on 20240117 start
    AN_BATTERY_ATTR(battery_type),
    // Modify by wenyaqi for TCT-978 on 20240117 end
    // Modify by wenyaqi for TCT-969 on 20240118 start
    AN_BATTERY_ATTR(chg_info),
    // Modify by wenyaqi for TCT-969 on 20240118 end
    // Modify by wenyaqi for TCT-720 on 20240131 begin
    AN_BATTERY_ATTR(max_soc_limit),
    AN_BATTERY_ATTR(max_current_limit),
    // Modify by wenyaqi for TCT-720 on 20240131 end
    // Modify by wenyaqi for TCT-766 on 20240605 begin
    AN_BATTERY_ATTR(cam_cur_limit),
    // Modify by wenyaqi for TCT-766 on 20240605 end
    // Modify by wenyaqi for TCT-975 on 20240129 begin
    AN_BATTERY_ATTR(cp_dump_reg),
    // Modify by wenyaqi for TCT-975 on 20240129 end
    // Modify by jiashixian for QN7026A-157 on 20240302 begin
    AN_BATTERY_ATTR(wls_chg_current),
    AN_BATTERY_ATTR(wls_chg_volt),
    AN_BATTERY_ATTR(wls_chg_temp),
    AN_BATTERY_ATTR(wls_chg_mode),
    AN_BATTERY_ATTR(wls_chg_info),
    AN_BATTERY_ATTR(wls_chg_fw_ver),
    AN_BATTERY_ATTR(wls_chg_tx_return_pwr),
    AN_BATTERY_ATTR(wls_chg_tx_out_pwr),
    AN_BATTERY_ATTR(wls_chg_tx_loss_pwr),
    // Modify by jiashixian for QN7026A-157 on 20240302 end
    // Modify by liufurong for TCT-2250 on 20240229 begin
    AN_BATTERY_ATTR(call_state),
    // Modify by liufurong for TCT-2250 on 20240229 end
    // Modify by liufurong for QN7026A-193 on 20240318 begin
    AN_BATTERY_ATTR(wls_set_icl),
    // Modify by liufurong for QN7026A-193 on 20240318 end
    // Modify by shanxinkai for QN7026A-246 on 20240319 begin
    AN_BATTERY_ATTR(wls_mpp_clock_state),
    // Modify by shanxinkai for QN7026A-246 on 20240319 end
};

// Modify by liufurong for TCT-837 on 20240514 begin
static ssize_t bat_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    int value = 0;
    value = an_battery_get_prop(BATTERY_TYPE);
    if ((value < AN_BATTERY_TYPE_GY) || (value > AN_BATTERY_TYPE_UNKNOWN)) {
        value = AN_BATTERY_TYPE_UNKNOWN;
    }

    AN_PSY_ERR("[%s]l=%d: battery info =%s\n", __FUNCTION__, __LINE__, AN_BATTERY_TYPE_TEXT[value]);

    return scnprintf(buf, PAGE_SIZE, "%s\n", AN_BATTERY_TYPE_TEXT[value]);
}

static ssize_t main_charger_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", PMIC_INFO);
}

static ssize_t slave_charger_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", PMIC_INFO);
}

static ssize_t pd_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", PMIC_INFO);
}

static ssize_t fuel_gauge_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", PMIC_INFO);
}

static ssize_t smb_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", PMIC_INFO);
}

static ssize_t charge_pump_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    int value = 0;
    value = an_battery_get_prop(CHG_INFO);
    if ((value < AN_CHG_SC8541) || (value > AN_CHG_UPM6720)) {
        value = AN_CHG_INFO_UNKNOWN;
    }

    AN_PSY_ERR("[%s]l=%d: chg info =%s\n", __FUNCTION__, __LINE__, AN_CHG_INFO_TEXT[value]);

    return scnprintf(buf, PAGE_SIZE, "%s\n", AN_CHG_INFO_TEXT[value]);
}

static ssize_t wls_fw_ver_info_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    int value = 0;
    value = an_battery_get_prop(WLS_CHG_FW_VERSION);

    AN_PSY_ERR("[%s]l=%d: wls fw info =%s\n", __FUNCTION__, __LINE__, AN_CHG_INFO_TEXT[value]);

    return scnprintf(buf, PAGE_SIZE, "CPS4038 fw version id == %#x\n", value);
}
// Modify by liufurong for QN7026A-837 on 20240520 begin
static ssize_t cc_orientation_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    int value = 0;
    value = an_otg_get_status();

    AN_PSY_ERR("[%s]l=%d: otg status =%d\n", __FUNCTION__, __LINE__, value);

    return scnprintf(buf, PAGE_SIZE, "OTG_STATUS= %d\n", value);
}

static struct kobj_attribute bat_info_attr = __ATTR(bat_info, 0444, bat_info_show, NULL);
static struct kobj_attribute main_charger_info_attr = __ATTR(main_charger_info, 0444, main_charger_info_show, NULL);
static struct kobj_attribute slave_charger_info_attr = __ATTR(slave_charger_info, 0444, slave_charger_info_show, NULL);
static struct kobj_attribute pd_info_attr = __ATTR(pd_info, 0444, pd_info_show, NULL);
static struct kobj_attribute fuel_gauge_info_attr = __ATTR(fuel_gauge_info, 0444, fuel_gauge_info_show, NULL);
static struct kobj_attribute smb_info_attr = __ATTR(smb_info, 0444, smb_info_show, NULL);
static struct kobj_attribute charge_pump_info_attr = __ATTR(charge_pump_info, 0444, charge_pump_info_show, NULL);
static struct kobj_attribute wls_fw_ver_info_attr = __ATTR(wls_fw_ver_info, 0444, wls_fw_ver_info_show, NULL);
static struct kobj_attribute cc_orientation_attr = __ATTR(cc_orientation, 0444, cc_orientation_show, NULL);

static struct attribute *an_info_attrs[] = {
    &bat_info_attr.attr,
    &main_charger_info_attr.attr,
    &slave_charger_info_attr.attr,
    &pd_info_attr.attr,
    &fuel_gauge_info_attr.attr,
    &smb_info_attr.attr,
    &charge_pump_info_attr.attr,
    &wls_fw_ver_info_attr.attr,
    &cc_orientation_attr.attr,
    NULL,
};
// Modify by liufurong for QN7026A-837 on 20240520 end
static const struct attribute_group an_info_group = {
    .attrs = an_info_attrs,
};

static const struct attribute_group *attr_groups[] = {
    &an_info_group,
    NULL,
};
// Modify by liufurong for TCT-837 on 20240514 end
static struct device_attribute an_usb_attrs[] = {
    AN_USB_ATTR(typec_cc_orientation),
    // Modify by wenyaqi for TCT-968 on 20240116 start
    AN_USB_ATTR(online),
    // Modify by wenyaqi for TCT-968 on 20240116 end
    // Modify by wenyaqi for QN7026A-14 on 20240119 start
    AN_USB_ATTR(analog_earphone),
    // Modify by wenyaqi for QN7026A-14 on 20240119 end
};

// Modify by wenyaqi for QN7026A-14 on 20240119 start
extern bool get_analog_online(void);
// Modify by wenyaqi for QN7026A-14 on 20240119 end

// Modify by wenyaqi for TCT-968 on 20240116 start
static struct device_attribute an_ac_attrs[] = {
    AN_AC_ATTR(online),
    //Modify by shanxinkai for TCT-2388 on 20240320 start
    AN_AC_ATTR(voltage_max),
    AN_AC_ATTR(current_max),
    //Modify by shanxinkai for TCT-2388 on 20240320 end
};

static an_online_state_t an_get_online_state(void)
{
    an_online_state_t ret = OFFLINE;
    struct power_supply *usb_psy = NULL;
    union power_supply_propval val;
    union power_supply_propval val_vol;
    usb_psy = power_supply_get_by_name("usb");
    if (IS_ERR_OR_NULL(usb_psy)) {
        AN_PSY_ERR("%s: get usb psy failed\n", __func__);
        return -EPROBE_DEFER;
    }
    power_supply_get_property(usb_psy, POWER_SUPPLY_PROP_USB_TYPE, &val);
    power_supply_get_property(usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val_vol);
    // Modify by wenyaqi for TCT-5150 on 20240514 begin
    AN_PSY_DEBUG("%s: get usb type val = %d, voltage now = %d\n", __func__, val.intval, val_vol.intval);
    // Modify by wenyaqi for TCT-5150 on 20240514 end
    switch (val.intval) {
        case POWER_SUPPLY_USB_TYPE_DCP:
        case POWER_SUPPLY_USB_TYPE_PD_DRP:
        case POWER_SUPPLY_USB_TYPE_PD_PPS:
        case POWER_SUPPLY_USB_TYPE_C:
        case POWER_SUPPLY_USB_TYPE_APPLE_BRICK_ID:
        case POWER_SUPPLY_USB_TYPE_ACA:
            ret = ONLINE_AC;
            break;
        case POWER_SUPPLY_USB_TYPE_SDP:
        case POWER_SUPPLY_USB_TYPE_CDP:
            ret = ONLINE_USB;
            break;
        case POWER_SUPPLY_USB_TYPE_PD:
            if (val_vol.intval <= 6500000) {
                ret = ONLINE_USB;
            } else {
                ret = ONLINE_AC;
            }
            break;
        case POWER_SUPPLY_USB_TYPE_UNKNOWN:
            ret = OFFLINE;
            break;
        default:
            break;
    }
    // Modify by wenyaqi for TCT-5150 on 20240514 begin
    AN_PSY_DEBUG("%s: final ret = %d\n", __func__, ret);
    // Modify by wenyaqi for TCT-5150 on 20240514 end
    return ret;
}
// Modify by wenyaqi for TCT-968 on 20240116 end
// Modify by shanxinkai for TCT-2388 on 20240320 start
static int an_get_voltage_max(void)
{
    struct power_supply *usb_psy = NULL;
    int ret = 0;

    union power_supply_propval prop = {0};
    const int voltage_max = 5000000; //5V

    usb_psy = power_supply_get_by_name("usb");
    if (IS_ERR_OR_NULL(usb_psy)) {
        AN_PSY_ERR("%s: get usb psy failed\n", __func__);
        return voltage_max;
    }

    ret = power_supply_get_property(usb_psy, POWER_SUPPLY_PROP_VOLTAGE_MAX, &prop);
    if (ret < 0) {
        AN_PSY_ERR("%s: get voltage_max property failed\n", __func__);
        return voltage_max;
    }

    return prop.intval;
}

static int an_get_current_max(void)
{
    struct power_supply *usb_psy = NULL;
    int ret = 0;

    union power_supply_propval prop = {0};
    const int current_max = 1500000; //1.5A

    usb_psy = power_supply_get_by_name("usb");
    if (IS_ERR_OR_NULL(usb_psy)) {
        AN_PSY_ERR("%s: get usb psy failed\n", __func__);
        return current_max;
    }

    ret = power_supply_get_property(usb_psy, POWER_SUPPLY_PROP_CURRENT_MAX, &prop);
    if (ret < 0) {
        AN_PSY_ERR("%s: get current_max property failed\n", __func__);
        return current_max;
    }

    return prop.intval;
}
// Modify by shanxinkai for TCT-2388 on 20240320 end

/* sysfs read for "battery" psd */
ssize_t an_bat_show_attrs(struct device *dev, struct device_attribute *attr, char *buf)
{
    const ptrdiff_t offset = attr - an_battery_attrs;
    int count = 0;
    int value = 0;

    switch (offset) {
        case INPUT_SUSPEND:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by wenyaqi for TCT-978 on 20240117 start
        case BATTERY_TYPE:
            value = an_battery_get_prop(offset);
            if ((value < AN_BATTERY_TYPE_GY) || (value > AN_BATTERY_TYPE_UNKNOWN)) {
                value = AN_BATTERY_TYPE_UNKNOWN;
            }
            count += scnprintf(buf + count, PAGE_SIZE - count, "%s\n", AN_BATTERY_TYPE_TEXT[value]);
            break;
        // Modify by wenyaqi for TCT-978 on 20240117 end
        // Modify by wenyaqi for TCT-969 on 20240118 start
        case CHG_INFO:
            value = an_battery_get_prop(offset);
            if ((value < AN_CHG_SC8541) || (value > AN_CHG_UPM6720)) {
                value = AN_CHG_INFO_UNKNOWN;
            }
            count += scnprintf(buf + count, PAGE_SIZE - count, "%s\n", AN_CHG_INFO_TEXT[value]);
            break;
        // Modify by wenyaqi for TCT-969 on 20240118 end
        // Modify by wenyaqi for TCT-720 on 20240131 begin
        case MAX_SOC_LIMIT:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        case MAX_CURRENT_LIMIT:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by wenyaqi for TCT-720 on 20240131 end
        // Modify by wenyaqi for TCT-766 on 20240605 begin
        case CAM_CUR_LIMIT:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by wenyaqi for TCT-766 on 20240605 end
        // Modify by wenyaqi for TCT-975 on 20240129 begin
        case CP_DUMP_REG:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by wenyaqi for TCT-975 on 20240129 end
        // Modify by jiashixian for QN7026A-157 on 20240302 begin
        case WLS_CHG_CURRENT:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        case WLS_CHG_VOLT:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        case WLS_CHG_TEMP:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        case WLS_CHG_MODE:
            value = an_battery_get_prop(offset);
            if ((value < AN_WLS_CHG_MODE_BACK_POWER) || (value > AN_WLS_CHG_MODE_UNKNOWN)) {
            value = AN_WLS_CHG_MODE_UNKNOWN;
            }
            count += scnprintf(buf + count, PAGE_SIZE - count, "%s\n", AN_WLS_CHG_MODE_TEXT[value]);
            break;
        case WLS_CHG_INFO:
            value = an_battery_get_prop(offset);
            if ((value < AN_WLS_CHG_CPS4038) || (value > AN_WLS_CHG_INFO_UNKNOWN)) {
            value = AN_WLS_CHG_INFO_UNKNOWN;
            }
            count += scnprintf(buf + count, PAGE_SIZE - count, "%s\n", AN_WLS_CHG_INFO_TEXT[value]);
            break;
        case WLS_CHG_FW_VERSION:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "CPS4038 fw version id == %#x\n", value);
            break;
        case WLS_CHG_TX_RETURN_PWR:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        case WLS_CHG_TX_OUT_PWR:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        case WLS_CHG_TX_LOSS_PWR:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by jiashixian for QN7026A-157 on 202400302 end
        // Modify by liufurong for TCT-2250 on 20240229 begin
        case CALL_STATE:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by liufurong for TCT-2250 on 20240229 end
        // Modify by liufurong for QN7026A-193 on 20240318 begin
        case WLS_SET_ICL:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by liufurong for QN7026A-193 on 20240318 end
        // Modify by shanxinkai for QN7026A-246 on 20240319 begin
        case WLS_MPP_CLOCK_STATE:
            value = an_battery_get_prop(offset);
            count += scnprintf(buf + count, PAGE_SIZE - count, "%d\n", value);
            break;
        // Modify by shanxinkai for QN7026A-246 on 20240319 end
        default:
            count = -EINVAL;
            break;
    }
    return count;
}

/* sysfs write for "battery" psd */
ssize_t an_bat_store_attrs(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    const ptrdiff_t offset = attr - an_battery_attrs;
    int ret = -EINVAL;
    int value = 0;

    switch (offset) {
        case INPUT_SUSPEND:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                ret = an_battery_set_prop(offset, value);
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
                }
                ret = count;
            }
            break;
        // Modify by wenyaqi for TCT-720|TCT-3151|TCT-2858 on 20240329 begin
        case MAX_SOC_LIMIT:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                if (value <= AN_MAX_SOC_L) {
                    value = AN_MAX_SOC_L;
                } else if (value >= AN_MAX_SOC_H) {
                    value = AN_MAX_SOC_H;
                }
                ret = an_battery_set_prop(offset, value);
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
                }
                ret = count;
            } else {
                ret = an_battery_set_prop(offset, AN_MAX_SOC_H); // reset to 100 soc
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
                } else {
                    AN_PSY_ERR("%s: max_soc_limit invalid, reset\n", __func__, ret);
                }
                ret = count;
            }
            break;
        case MAX_CURRENT_LIMIT:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                if (value <= AN_MAX_CURRENT_L) {
                    value = AN_MAX_CURRENT_L;
                } else if (value >= AN_MAX_CURRENT_H) {
                    value = AN_MAX_CURRENT_H;
                }
                ret = an_battery_set_prop(offset, value);
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
                }
                ret = count;
            } else {
                ret = an_battery_set_prop(offset, AN_MAX_CURRENT_H); // reset to 6000mA
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
                } else {
                    AN_PSY_ERR("%s: max_current_limit invalid, set 6000mA\n", __func__, ret);
                }
                ret = count;
            }
            break;
        // Modify by wenyaqi for TCT-720|TCT-3151|TCT-2858 on 20240329 end
        // Modify by wenyaqi for TCT-766 on 20240605 begin
        case CAM_CUR_LIMIT:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                if (value <= 0) {
                    value = 0;
                } else if (value >= AN_MAX_CURRENT_H) {
                    value = AN_MAX_CURRENT_H;
                }
                ret = an_battery_set_prop(offset, value);
                if (ret < 0) {
                    AN_PSY_ERR("%s: cam_cur_limit set error\n", __func__, ret);
                }
                ret = count;
            } else {
                ret = an_battery_set_prop(offset, AN_MAX_CURRENT_H); // reset to 6000mA
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
                } else {
                    AN_PSY_ERR("%s: cam_cur_limit invalid, set 6000mA\n", __func__, ret);
                }
                ret = count;
            }
            break;
        // Modify by wenyaqi for TCT-766 on 20240605 end
        // Modify by liufurong for TCT-2250 on 20240229 begin
        case CALL_STATE:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                ret = an_battery_set_prop(offset, value);
                // Modify by huangzhongjie for QN7026A-590 on 20240426 begin
                if(value == 1) {
                    regulator_set_load(vreg, 3000000);
                    regulator_enable(vreg);
                } else {
                    regulator_set_load(vreg, 100000);
                    regulator_disable(vreg);
                }
                // Modify by huangzhongjie for QN7026A-590 on 20240426 end
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error \n", __func__, ret);
                }
                ret = count;
            }
            break;
        // Modify by liufurong for TCT-2250 on 20240229 end
        // Modify by liufurong for QN7026A-193 on 20240318 begin
        case WLS_SET_ICL:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                ret = an_battery_set_prop(offset, value);
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop WLS_SET_ICL error\n", __func__, ret);
                }
                ret = count;
            }
            break;
        // Modify by liufurong for QN7026A-193 on 20240318 end
        // Modify by shanxinkai for QN7026A-246 on 20240319 begin
        case WLS_MPP_CLOCK_STATE:
            if (sscanf(buf, "%10d\n", &value) == 1) {
                ret = an_battery_set_prop(offset, value);
                if (ret < 0) {
                    AN_PSY_ERR("%s: an_battery_set_prop error \n", __func__, ret);
                }
                ret = count;
            }
            break;
        // Modify by shanxinkai for QN7026A-246 on 20240319 end
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

/* sysfs read for "usb" psd */
ssize_t an_usb_show_attrs(struct device *dev, struct device_attribute *attr, char * buf)
{
    const ptrdiff_t offset = attr - an_usb_attrs;
    int count = 0;
    int value = 0;
    // Modify by wenyaqi for TCT-968 on 20240116 start
    an_online_state_t online_state = OFFLINE;
    // Modify by wenyaqi for TCT-968 on 20240116 end

    switch (offset) {
        case TYPEC_CC_ORIENT:
            value = an_usb_get_prop(offset);
            count = sprintf(buf, "%d\n", value);
            break;
        // Modify by wenyaqi for TCT-968 on 20240116 start
        case USB_ONLINE:
            online_state = an_get_online_state();
            if (online_state == ONLINE_USB) {
                value = 1;
            } else {
                value = 0;
            }
            count = sprintf(buf, "%d\n", value);
            break;
        // Modify by wenyaqi for TCT-968 on 20240116 end
        // Modify by wenyaqi for QN7026A-14 on 20240119 start
        case ANALOG_EARPHONE_ONLINE:
            value = get_analog_online();
            count = sprintf(buf, "%d\n", value);
            break;
        // Modify by wenyaqi for QN7026A-14 on 20240119 end
        default:
            count = -EINVAL;
            break;
    }
    return count;
}

/* sysfs write for "usb" psd */
ssize_t an_usb_store_attrs(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    const ptrdiff_t offset = attr - an_usb_attrs;
    int ret = -EINVAL;

    switch (offset) {
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

// Modify by wenyaqi for TCT-968 on 20240116 start
/* sysfs read for "ac" psd */
ssize_t an_ac_show_attrs(struct device *dev, struct device_attribute *attr, char * buf)
{
    const ptrdiff_t offset = attr - an_ac_attrs;
    int count = 0;
    int value = 0;
    an_online_state_t online_state = OFFLINE;

    switch (offset) {
        case AC_ONLINE:
            online_state = an_get_online_state();
            if (online_state == ONLINE_AC) {
                value = 1;
            } else {
                value = 0;
            }
            count = sprintf(buf, "%d\n", value);
            break;
        // Modify by shanxinkai for TCT-2388 on 20240320 start
        case AC_VOLTAGE_MAX:
            value = an_get_voltage_max();
            count = sprintf(buf, "%d\n", value);
            break;
        case AC_CURRENT_MAX:
            value = an_get_current_max();
            count = sprintf(buf, "%d\n", value);
            break;
        // Modify by shanxinkai for TCT-2388 on 20240320 end
        default:
            count = -EINVAL;
            break;
    }
    return count;
}

/* sysfs write for "ac" psd */
ssize_t an_ac_store_attrs(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    const ptrdiff_t offset = attr - an_ac_attrs;
    int ret = -EINVAL;
    AN_PSY_ERR("%s: an_ac_store_attrs enter\n", __func__);

    switch (offset) {
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}
// Modify by wenyaqi for TCT-968 on 20240116 end

static int an_battery_create_attrs(struct device *dev)
{
    unsigned long i = 0;
    int ret = 0;

    for (i = 0; i < ARRAY_SIZE(an_battery_attrs); i++) {
        ret = device_create_file(dev, &an_battery_attrs[i]);
        if (ret) {
            goto create_attrs_failed;
        }
    }
    goto create_attrs_succeed;

create_attrs_failed:
    AN_PSY_ERR("%s: failed (%d)\n", __func__, ret);
    while (i--) {
        device_remove_file(dev, &an_battery_attrs[i]);
    }

create_attrs_succeed:
    return ret;
}

static int an_usb_create_attrs(struct device *dev)
{
    unsigned long i = 0;
    int ret = 0;

    for (i = 0; i < ARRAY_SIZE(an_usb_attrs); i++) {
        ret = device_create_file(dev, &an_usb_attrs[i]);
        if (ret) {
            goto create_attrs_failed;
        }
    }
    goto create_attrs_succeed;

create_attrs_failed:
    AN_PSY_ERR("%s: failed (%d)\n", __func__, ret);
    while (i--) {
        device_remove_file(dev, &an_usb_attrs[i]);
    }

create_attrs_succeed:
    return ret;
}

// Modify by wenyaqi for TCT-968 on 20240116 start
static int an_ac_create_attrs(struct device *dev)
{
    unsigned long i = 0;
    int ret = 0;

    for (i = 0; i < ARRAY_SIZE(an_ac_attrs); i++) {
        ret = device_create_file(dev, &an_ac_attrs[i]);
        if (ret) {
            goto create_attrs_failed;
        }
    }
    goto create_attrs_succeed;

create_attrs_failed:
    AN_PSY_ERR("%s: failed (%d)\n", __func__, ret);
    while (i--) {
        device_remove_file(dev, &an_ac_attrs[i]);
    }

create_attrs_succeed:
    return ret;
}
// Modify by wenyaqi for TCT-968 on 20240116 end

// Modify by liufurong for TCT-2763 on 20240506 begin
static void send_boot_mode_to_adsp_work(struct work_struct *work)
{
    struct an_battery *an_batt = NULL;
    int ret = 0;
    static int retry_cnt = 5;

    an_batt = container_of(work, struct an_battery, boot_mode_work.work);
    if (an_batt == NULL) {
        AN_PSY_ERR("%s: get an_batt error\n", __func__);
        return;
    }
    ret = an_battery_set_prop(AN_BOOT_MODE, an_batt->boot_mode);
    if (ret < 0) {
        AN_PSY_ERR("%s: an_battery_set_prop error, ret = %d, an_batt->boot_mode = %d \n", __func__, ret, an_batt->boot_mode);
        if (retry_cnt != 1) {
            schedule_delayed_work(&an_batt->boot_mode_work, msecs_to_jiffies(1000));
            retry_cnt--;
        }
    } else {
        AN_PSY_ERR("%s: send boot mode sucess an_batt->boot_mode:%d, ret = %d\n", __func__, an_batt->boot_mode, ret);
    }

}

static void send_boot_mode_to_adsp(struct device *dev, struct an_battery *an_batt)
{
    struct device_node *np_chosen = NULL;
    int ret = 0;
    const char *whole_cmd = NULL;
    np_chosen = of_find_node_by_path("/chosen");

    if (!np_chosen) {
        np_chosen = of_find_node_by_path("/chosen@0");
    }

    of_property_read_string(np_chosen, "bootargs", &whole_cmd);
    if (!strnstr(whole_cmd, POWER_OFF, strlen(whole_cmd))) {
        AN_PSY_ERR("it is normal boot! whole_cmd = %s", whole_cmd);
        an_batt->boot_mode = 1;
    } else {
        AN_PSY_ERR("it is not normal boot! whole_cmd = %s", whole_cmd);
        an_batt->boot_mode = 0;
    }

    ret = an_battery_set_prop(AN_BOOT_MODE, an_batt->boot_mode);
    if (ret < 0) {
        AN_PSY_ERR("%s: send_boot_mode_to_adsp error, ret = %d , an_batt->boot_mode = %d\n", __func__, ret, an_batt->boot_mode);
        schedule_delayed_work(&an_batt->boot_mode_work, msecs_to_jiffies(1000));
    } else {
        AN_PSY_ERR("%s: send_boot_mode_to_adsp sucess, ret = %d , an_batt->boot_mode = %d\n", __func__, ret, an_batt->boot_mode);
    }

}
// Modify by liufurong for TCT-2763 on 20240506 end

// Modify by wenyaqi for TCT-3292 on 20240418 begin
#ifndef CONFIG_AN_FACTORY_BUILD
static void update_soc_limit_work(struct work_struct *work)
{
    struct an_battery *an_batt = NULL;
    int ret = 0;
    static int retry_cnt = 1;

    an_batt = container_of(work, struct an_battery, soc_limit_work.work);
    if (an_batt == NULL) {
        AN_PSY_ERR("%s: get an_batt error\n", __func__);
        return;
    }
    ret = an_battery_set_prop(MAX_SOC_LIMIT, an_batt->soc_limit_value);
    if (ret < 0) {
        AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__);
        if (retry_cnt == 1) {
            schedule_delayed_work(&an_batt->soc_limit_work, msecs_to_jiffies(5000));
            retry_cnt--;
        }
    }
    AN_PSY_ERR("%s: set max_soc_limit:%d\n", __func__, an_batt->soc_limit_value);
}

static void update_soc_limit(struct device *dev, struct an_battery *an_batt)
{
    struct device_node *of_chosen = NULL;
    const char *whole_cmd = NULL;
    const char *cmd_start = NULL;
    const char *cmd_end = NULL;
    char* soc_limit_cmd = NULL;
    int length = 0;
    int ret = 0;

    // get cmdline charginglimitlevel
    of_chosen = of_find_node_by_path("/chosen");
    if (!of_chosen) {
        of_chosen = of_find_node_by_path("/chosen@0");
    }
    of_property_read_string(of_chosen, "bootargs", &whole_cmd);
    if (!whole_cmd) {
        AN_PSY_ERR("%s: read cmdline failed!", __func__);
        return;
    } else {
        cmd_start = strstr(whole_cmd, "soc_limit=");
        if (cmd_start == NULL) {
            AN_PSY_ERR("%s: NULL\n", __func__);
            return;
        }

        cmd_start += strlen("soc_limit=");
        cmd_end = strchr(cmd_start, ' ');
        if (cmd_end == NULL) {
            cmd_end = strchr(cmd_start, '\0');
        }

        length = cmd_end - cmd_start;
        soc_limit_cmd = devm_kzalloc(dev, length + 1, GFP_KERNEL);
        strncpy(soc_limit_cmd, cmd_start, length);
    }

    // string to int
    if (sscanf(soc_limit_cmd, "%10d\n", &an_batt->soc_limit_value) == 1) {
        if (an_batt->soc_limit_value <= AN_MAX_SOC_L) {
            an_batt->soc_limit_value = AN_MAX_SOC_L;
        } else if (an_batt->soc_limit_value >= AN_MAX_SOC_H) {
            an_batt->soc_limit_value = AN_MAX_SOC_H;
        }
        ret = an_battery_set_prop(MAX_SOC_LIMIT, an_batt->soc_limit_value);
        if (ret < 0) {
            AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__, ret);
            schedule_delayed_work(&an_batt->soc_limit_work, msecs_to_jiffies(5000));
        } else {
            AN_PSY_ERR("%s: soc_limit=%d\n", __func__, an_batt->soc_limit_value);
        }
    } else {
        ret = an_battery_set_prop(MAX_SOC_LIMIT, AN_MAX_SOC_H); // reset to 100 soc
        if (ret < 0) {
            AN_PSY_ERR("%s: an_battery_set_prop error\n", __func__);
            schedule_delayed_work(&an_batt->soc_limit_work, msecs_to_jiffies(5000));
        } else {
            AN_PSY_ERR("%s: max_soc_limit invalid, reset\n", __func__);
        }
    }

    return;
}
#endif
// Modify by wenyaqi for TCT-3292 on 20240418 end

static int an_psy_probe(struct platform_device *pdev)
{
    int ret = 0;

    struct an_battery *an_batt = devm_kzalloc(&pdev->dev, sizeof(*an_batt), GFP_KERNEL);

    if (!an_batt) {
        AN_PSY_ERR("dont not kzmalloc\n", __func__);
        return -ENOMEM;
    }

    AN_PSY_INFO("enter\n");

    an_batt->psy = power_supply_get_by_name("battery");
    if (IS_ERR_OR_NULL(an_batt->psy)) {
        return -EPROBE_DEFER;
    }

    ret = an_battery_create_attrs(&an_batt->psy->dev);
    if (ret) {
        AN_PSY_ERR(" battery fail to create attrs!!\n", __func__);
        return ret;
    }

    AN_PSY_INFO("Battery Install Success !!\n", __func__);

    an_batt->usb_psy = power_supply_get_by_name("usb");
    if (IS_ERR_OR_NULL(an_batt->usb_psy)) {
        return -EPROBE_DEFER;
    }
    ret = an_usb_create_attrs(&an_batt->usb_psy->dev);
    if (ret) {
        AN_PSY_ERR(" usb fail to create attrs!!\n", __func__);
    }
    AN_PSY_INFO("USB Install Success !!\n", __func__);

    // Modify by wenyaqi for TCT-968 on 20240116 start
    an_batt->ac_psy = power_supply_get_by_name("ac");
    if (IS_ERR_OR_NULL(an_batt->ac_psy)) {
        return -EPROBE_DEFER;
    }
    ret = an_ac_create_attrs(&an_batt->ac_psy->dev);
    if (ret) {
        AN_PSY_ERR(" ac fail to create attrs!!\n", __func__);
    }

    // Modify by wenyaqi for TCT-3292 on 20240418 begin
    #ifndef CONFIG_AN_FACTORY_BUILD
    an_batt->soc_limit_value = AN_MAX_SOC_H;
    INIT_DELAYED_WORK(&an_batt->soc_limit_work, update_soc_limit_work);
    update_soc_limit(&pdev->dev, an_batt);
    #endif
    // Modify by wenyaqi for TCT-3292 on 20240418 end
    // Modify by liufurong for TCT-2763 on 20240506 begin
    an_batt->boot_mode = 1;
    INIT_DELAYED_WORK(&an_batt->boot_mode_work, send_boot_mode_to_adsp_work);
    send_boot_mode_to_adsp(&pdev->dev, an_batt);
    // Modify by liufurong for TCT-2763 on 20240506 end

    // Modify by huangzhongjie for QN7026A-590 on 20240426 begin
    vreg = devm_regulator_get(&pdev->dev, "enable-pwmmode");
    if (IS_ERR(vreg)) {
        AN_PSY_ERR("%s: enable-pwmmode not found\n", __func__);
    }
    // Modify by huangzhongjie for QN7026A-590 on 20240426 end

    // Modify by liufurong for TCT-837 on 20240514 begin
    ret = sysfs_create_groups(get_root_kobjs(), attr_groups);
    if (ret < 0) {
        AN_PSY_ERR("%s: sysfs_create_groups fail ret = %d\n", __func__, ret);
    }
    // Modify by liufurong for TCT-837 on 20240514 end

    AN_PSY_INFO("AC Install Success !!\n", __func__);
    // Modify by wenyaqi for TCT-968 on 20240116 end

    return 0;
}

static int an_psy_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id an_psy_of_match[] = {
    {.compatible = "an,an_psy",},
    {},
};

MODULE_DEVICE_TABLE(of, an_psy_of_match);

static struct platform_driver an_psy_driver = {
    .probe = an_psy_probe,
    .remove = an_psy_remove,
    .driver = {
        .name = "an_psy",
        .of_match_table = of_match_ptr(an_psy_of_match),
    },
};

static int __init an_psy_init(void)
{
    AN_PSY_INFO("init\n");

    return platform_driver_register(&an_psy_driver);
}
// Modify by shanxinkai for QN7026A-333 on 20240323 start
module_init(an_psy_init);
// Modify by shanxinkai for QN7026A-333 on 20240323 end

static void __exit an_psy_exit(void)
{
    AN_PSY_INFO("exit\n");
    platform_driver_unregister(&an_psy_driver);
}
module_exit(an_psy_exit);

MODULE_AUTHOR("an_oem");
MODULE_DESCRIPTION("an psy module driver");
MODULE_LICENSE("GPL");
