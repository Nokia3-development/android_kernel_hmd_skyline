#ifndef __AN_VBUS_CTRL__
#define __AN_VBUS_CTRL__

#define USB_THERMAL_LTH  60
#define USB_THERMAL_UTH  70
#define USB_THERMAL_DEFAULT  25

#define CHG_ALERT_NORMAL_STATE   0 /*<60degC*/
#define CHG_ALERT_WARM_STATE     1 /*60-70degC*/
#define CHG_ALERT_HOT_STATE      2 /*>70degC*/
#define USB_TERMAL_DETECT_TIMER  30000

enum {
    VBUS_CTRL_LOW,    /* disconnect VBUS to GND */
    VBUS_CTRL_HIGH,    /* connect VBUS to GND */
};

struct vbus_ctrl_dev {
    struct device *dev;
    int state;
    int vbus_ctrl_gpio;
    struct delayed_work vbus_ctrl_state_work;
};

#endif /*__AN_VBUS_CTRL__*/
