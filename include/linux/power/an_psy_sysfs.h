/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __AN_PSY_SYSFS_H__
#define __AN_PSY_SYSFS_H__

#include <linux/power_supply.h>
#include <linux/sysfs.h>

#if defined(CONFIG_CUSTOM_FACTORY_BUILD)
#define CONFIG_AN_FACTORY_BUILD
#endif //CONFIG_CUSTOM_FACTORY_BUILD

/* log debug */
#define AN_PSY_TAG    "[AN_PSY]"

#define AN_PSY_DEBUG_LOG 1
#ifdef AN_PSY_DEBUG_LOG
    #define AN_PSY_INFO(fmt, args...)    pr_info(AN_PSY_TAG fmt, ##args)
    // Modify by wenyaqi for TCT-5150 on 20240514 begin
    #define AN_PSY_DEBUG(fmt, args...)   pr_debug(AN_PSY_TAG fmt, ##args)
    // Modify by wenyaqi for TCT-5150 on 20240514 end
#else
    #define AN_PSY_INFO(fmt, args...)
#endif

#define AN_PSY_ERR(fmt, args...)    pr_err(AN_PSY_TAG fmt, ##args)

// Modify by wenyaqi for TCT-3151|TCT-2858 on 20240328 begin
#define AN_MAX_SOC_L             30 // percent
#define AN_MAX_SOC_H             100 // percent
#define AN_MAX_CURRENT_L         500 // mA
#define AN_MAX_CURRENT_H         6000 // mA
// Modify by wenyaqi for TCT-3151|TCT-2858 on 20240328 end

/* battery psy attrs */
ssize_t an_bat_show_attrs(struct device *dev,
                            struct device_attribute *attr, char *buf);

ssize_t an_bat_store_attrs(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t count);

/* usb psy attrs */
ssize_t an_usb_show_attrs(struct device *dev,
                            struct device_attribute *attr, char *buf);

ssize_t an_usb_store_attrs(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t count);

// Modify by wenyaqi for TCT-968 on 20240116 start
/* ac psy attrs */
ssize_t an_ac_show_attrs(struct device *dev,
                            struct device_attribute *attr, char *buf);

ssize_t an_ac_store_attrs(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t count);
// Modify by wenyaqi for TCT-968 on 20240116 end

#define AN_BATTERY_ATTR(_name)                     \
{                                                   \
    .attr = {.name = #_name, .mode = 0664},         \
    .show = an_bat_show_attrs,                     \
    .store = an_bat_store_attrs,                   \
}

#define AN_USB_ATTR(_name)                         \
{                                                   \
    .attr = {.name = #_name, .mode =  0664},        \
    .show = an_usb_show_attrs,                     \
    .store = an_usb_store_attrs,                   \
}

// Modify by wenyaqi for TCT-968 on 20240116 start
#define AN_AC_ATTR(_name)                         \
{                                                   \
    .attr = {.name = #_name, .mode =  0664},        \
    .show = an_ac_show_attrs,                     \
    .store = an_ac_store_attrs,                   \
}
// Modify by wenyaqi for TCT-968 on 20240116 end

/* battery psy property */
enum {
    INPUT_SUSPEND,
    // Modify by wenyaqi for TCT-978 on 20240117 start
    BATTERY_TYPE,
    // Modify by wenyaqi for TCT-978 on 20240117 end
    // Modify by wenyaqi for TCT-969 on 20240118 start
    CHG_INFO,
    // Modify by wenyaqi for TCT-969 on 20240118 end
    // Modify by wenyaqi for TCT-720 on 20240131 begin
    MAX_SOC_LIMIT,
    MAX_CURRENT_LIMIT,
    // Modify by wenyaqi for TCT-720 on 20240131 end
    // Modify by wenyaqi for TCT-766 on 20240605 begin
    CAM_CUR_LIMIT,
    // Modify by wenyaqi for TCT-766 on 20240605 end
    // Modify by wenyaqi for TCT-975 on 20240129 begin
    CP_DUMP_REG,
    // Modify by wenyaqi for TCT-975 on 20240129 end
    // Modigy by jiashixian for QN7026A-157 on 20240302 begin
    WLS_CHG_CURRENT,
    WLS_CHG_VOLT,
    WLS_CHG_TEMP,
    WLS_CHG_MODE,
    WLS_CHG_INFO,
    WLS_CHG_FW_VERSION,
    WLS_CHG_TX_RETURN_PWR,
    WLS_CHG_TX_OUT_PWR,
    WLS_CHG_TX_LOSS_PWR,
    // Modigy by jiashixian for QN7026A-157 on 20240302 end
    // Modify by liufurong for TCT-2250 on 20240229 begin
    CALL_STATE,
    // Modify by liufurong for TCT-2250 on 20240229 end
    // Modify by liufurong for QN7026A-193 on 20240318 begin
    WLS_SET_ICL,
    // Modify by liufurong for QN7026A-193 on 20240318 end
    // Modify by shanxinkai for QN7026A-246 on 20240319 begin
    WLS_MPP_CLOCK_STATE,
    // Modify by shanxinkai for QN7026A-246 on 20240319 end
    // Modify by liufurong for TCT-2763 on 20240506 begin
    AN_BOOT_MODE,
    // Modify by liufurong for TCT-2763 on 20240506 end
    AN_BATT_PROP_MAX,
};

/* usb psy property */
enum {
    TYPEC_CC_ORIENT = 0,
    // Modify by wenyaqi for TCT-968 on 20240116 start
    USB_ONLINE,
    // Modify by wenyaqi for TCT-968 on 20240116 end
    // Modify by wenyaqi for QN7026A-14 on 20240119 start
    ANALOG_EARPHONE_ONLINE,
    // Modify by wenyaqi for QN7026A-14 on 20240119 end
    AN_USB_PROP_MAX,
};

// Modify by wenyaqi for TCT-968 on 20240116 start
/* ac psy property */
enum {
    AC_ONLINE = 0,
    // Modify by shanxinkai for TCT-2388 on 20240320 start
    AC_VOLTAGE_MAX,
    AC_CURRENT_MAX,
    // Modify by shanxinkai for TCT-2388 on 20240320 end
    AN_AC_PROP_MAX,
};

typedef enum an_online_state {
    OFFLINE = 0,
    ONLINE_AC,
    ONLINE_USB,
} an_online_state_t;
// Modify by wenyaqi for TCT-968 on 20240116 end

/* usb psy - orient property */
enum an_usb_orient {
    AN_USB_ORIENT_UNKOWN = 0,
    AN_USB_ORIENT_CC1,
    AN_USB_ORIENT_CC2,
};

typedef enum {
  AN_TYPEC_CC_EVENT,
  // Modify by wenyaqi for QN7026A-14 on 20240119 start
  AN_ANALOG_EARPHONE_ATTACH_EVENT,
  // Modify by wenyaqi for QN7026A-14 on 20240119 end
  // Modify by liufurong for QN7026A-538 on 20240413 begin
  AN_WLS_ATTACH_EVENT,
  // Modify by liufurong for QN7026A-538 on 20240413 end
  // Modify by liufurong for QN7026A-837 on 20240520 begin
  AN_GET_OTG_STATUS,
  // Modify by liufurong for QN7026A-837 on 20240520 end
  AN_MAX_EVENT_TYPE
}an_notify_event_type;

struct an_battery {
    struct power_supply *psy;
    struct power_supply *usb_psy;
    // Modify by wenyaqi for TCT-968 on 20240116 start
    struct power_supply *ac_psy;
    // Modify by wenyaqi for TCT-968 on 20240116 end
    // Modify by wenyaqi for TCT-3292 on 20240418 begin
    #ifndef CONFIG_AN_FACTORY_BUILD
    struct delayed_work soc_limit_work;
    int soc_limit_value;
    #endif
    // Modify by wenyaqi for TCT-3292 on 20240418 end
    // Modify by liufurong for TCT-2763 on 20240506 begin
    struct delayed_work boot_mode_work;
    bool boot_mode;
    // Modify by liufurong for TCT-2763 on 20240506 end
};

struct an_usb {
    u8 cc_orient;
};

// Modify by wenyaqi for TCT-978 on 20240117 start
enum an_battery_type {
    AN_BATTERY_TYPE_GY = 0,
    AN_BATTERY_TYPE_UNKNOWN,
};
// Modify by wenyaqi for TCT-978 on 20240117 end

// Modify by wenyaqi for TCT-969 on 20240118 start
enum an_chg_info {
    AN_CHG_SC8541 = 0,
    AN_CHG_UPM6720,
    AN_CHG_INFO_UNKNOWN,
};
// Modify by wenyaqi for TCT-969 on 20240118 end

// Modify by jiashixian for QN7026A-157 on 20240226 begin
enum an_wls_chg_mode_info {
    AN_WLS_CHG_MODE_BACK_POWER = 0,
    AN_WLS_CHG_MODE_MST_ON_PASSIVE = 1,
    AN_WLS_CHG_MODE_MST_ON_ACTIVE = 2,
    AN_WLS_CHG_MODE_TX_MODE_ON = 3,
    AN_WLS_CHG_MODE_TX_FOD_CONFILCT = 4,
    AN_WLS_CHG_MODE_TX_PHM = 5,
    AN_WLS_CHG_MODE_AC_MIS = 6,
    AN_WLS_CHG_MODE_WPC_BPP = 7,
    AN_WLS_CHG_MODE_WPC_EPP = 8,
    AN_WLS_CHG_MODE_MPP_RESTRICT = 9,
    AN_WLS_CHG_MODE_MPP_FULL = 10,
    AN_WLS_CHG_MODE_MPP_CLOAK = 11,
    AN_WLS_CHG_MODE_MPP_NEGO = 12,
    AN_WLS_CHG_MODE_EPP_NEGO = 13,
    AN_WLS_CHG_MODE_UNKNOWN,
};

enum an_wls_chg_info {
    AN_WLS_CHG_CPS4038 = 0,
    AN_WLS_CHG_INFO_UNKNOWN,
};
// Modify by jiashixian for QN7026A-157 on 20240226 end

extern int an_usb_get_prop(u32 prop_id);
extern int an_usb_set_prop(u32 prop_id);
extern int an_battery_set_prop(u32 prop_id, u32 value);
extern int an_battery_get_prop(u32 prop_id);
// Modify by liufurong for QN7026A-837 on 20240520 begin
extern int an_otg_get_status(void);
// Modify by liufurong for QN7026A-837 on 20240520 end

#endif /* __AN_PSY_SYSFS_H__ */
