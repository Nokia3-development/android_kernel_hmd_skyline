// SPDX-License-Identifier: GPL-2.0
/*
 * an_glink.c
 *
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/soc/qcom/pmic_glink.h>
#include <linux/power/an_psy_sysfs.h>

// Modify by liufurong for QN7026A-538 on 20240413 begin
extern void schedule_wls_thermal_work(bool enable);
// Modify by liufurong for QN7026A-538 on 20240413 end

#define AN_GLINK_MSG_NAME              "an_glink"
#define AN_GLINK_MSG_OWNER_ID          32777
#define AN_GLINK_MSG_TYPE_REQ_RESP     1
#define AN_GLINK_MSG_TYPE_NOTIFY       2

#define AN_GLINK_BATTERY_OPCODE_GET    0x10011
#define AN_GLINK_BATTERY_OPCODE_SET    0x10012

#define AN_GLINK_USB_OPCODE_GET        0x10013
#define AN_GLINK_USB_OPCODE_SET        0x10014

#define AN_GLINK_MSG_OPCODE_NOTIFY     0x10010

#define AN_GLINK_MSG_WAIT_MS           1000

// Modify by wenyaqi for QN7026A-14 on 20240119 start
static bool g_an_analog_earphone_online = false;
bool get_analog_online(void)
{
    return g_an_analog_earphone_online;
}
EXPORT_SYMBOL(get_analog_online);
// Modify by wenyaqi for QN7026A-14 on 20240119 end

/* tx message data for get operation */
struct an_glink_req_msg {
    struct pmic_glink_hdr hdr;
    u32 prop_id;
    u32 value;
};

struct an_glink_resp_msg {
    struct pmic_glink_hdr hdr;
    u32 prop_id;
    u32 value;
    u32 ret_code;
};

/* rx message data for notify operation */
struct an_glink_notify_rx_msg {
    struct pmic_glink_hdr hdr;
    u32 notify_type;
    u32 notify_value;
};

struct an_psy_state {
    u32 *prop;
    u32 opcode_get;
    u32 opcode_set;
    u32 prop_count;
};

enum an_psy_type {
    AN_PSY_TYPE_BATTERY,
    AN_PSY_TYPE_USB,
    AN_PSY_TYPE_MAX,
};

struct an_glink_dev {
    struct device *dev;
    struct pmic_glink_client *client;
    struct mutex send_lock;
    struct completion msg_ack;
    struct an_psy_state psy_list[AN_PSY_TYPE_MAX];
};

static struct an_glink_dev *g_an_glink_dev = NULL;
struct an_usb g_an_usb;

static int an_glink_send_message(struct an_glink_dev *an_dev, void *data, int len)
{
    int ret = 0;

    mutex_lock(&an_dev->send_lock);
    reinit_completion(&an_dev->msg_ack);

    ret = pmic_glink_write(an_dev->client, data, len);
    if (ret) {
        mutex_unlock(&an_dev->send_lock);
        AN_PSY_ERR("send message error\n");
        return ret;
    }

    /* wait for to receive data when message send success */
    ret = wait_for_completion_timeout(&an_dev->msg_ack,
        msecs_to_jiffies(AN_GLINK_MSG_WAIT_MS));
    if (!ret) {
        mutex_unlock(&an_dev->send_lock);
        AN_PSY_ERR("send message timeout\n");
        return -ETIMEDOUT;
    }

    mutex_unlock(&an_dev->send_lock);
    return 0;
}

static int an_glink_get_property(struct an_glink_dev *an_dev,
                                  struct an_psy_state *pst, u32 prop_id)
{
    struct an_glink_req_msg msg = { { 0 } };

    /* prepare header */
    msg.hdr.owner = AN_GLINK_MSG_OWNER_ID;
    msg.hdr.type = AN_GLINK_MSG_TYPE_REQ_RESP;
    msg.hdr.opcode = pst->opcode_get;
    /* prepare data */
    msg.prop_id = prop_id;

    return an_glink_send_message(an_dev, &msg, sizeof(msg));
}

static int an_glink_set_property(struct an_glink_dev *an_dev,
                                  struct an_psy_state *pst,
                                  u32 prop_id, u32 value)
{
    struct an_glink_req_msg msg = { { 0 } };

    /* prepare header */
    msg.hdr.owner = AN_GLINK_MSG_OWNER_ID;
    msg.hdr.type = AN_GLINK_MSG_TYPE_REQ_RESP;
    msg.hdr.opcode = pst->opcode_set;
    /* prepare data */
    msg.prop_id = prop_id;
    msg.value = value;

    return an_glink_send_message(an_dev, &msg, sizeof(msg));
}

/*-------------------- External extension function start--------------*/
// Modify by liufurong for QN7026A-837 on 20240520 begin
static int g_otg_status = 0;
int an_otg_get_status(void)
{
    return g_otg_status;
}
EXPORT_SYMBOL(an_otg_get_status);
// Modify by liufurong for QN7026A-837 on 20240520 end

int an_battery_get_prop(u32 prop_id)
{
    struct an_glink_dev *an_dev = g_an_glink_dev;
    struct an_psy_state *pst = &an_dev->psy_list[AN_PSY_TYPE_BATTERY];
    int ret = 0;

    if (an_dev != NULL) {
        ret = an_glink_get_property(an_dev, pst, prop_id);
        if (ret < 0) {
            return ret;
        }
        return pst->prop[prop_id];
    }
    return -ENODATA;
}
EXPORT_SYMBOL(an_battery_get_prop);

int an_battery_set_prop(u32 prop_id, u32 value)
{
    struct an_glink_dev *an_dev = g_an_glink_dev;
    struct an_psy_state *pst = &an_dev->psy_list[AN_PSY_TYPE_BATTERY];
    int ret = 0;

    if (an_dev != NULL) {
        ret = an_glink_set_property(an_dev, pst, prop_id, value);
        if (ret < 0) {
            return ret;
        }
        return ret;
    }
    return -ENODATA;
}
EXPORT_SYMBOL(an_battery_set_prop);

int an_usb_get_prop(u32 prop_id)
{
    return g_an_usb.cc_orient;
}
EXPORT_SYMBOL(an_usb_get_prop);

int an_usb_set_prop(u32 prop_id)
{
    struct an_glink_dev *an_dev = g_an_glink_dev;
    //struct an_psy_state *pst = &an_dev->psy_list[AN_PSY_TYPE_USB];

    if (an_dev != NULL) {
        //oem
    }
    return -ENODATA;
}
EXPORT_SYMBOL(an_usb_set_prop);
/*-------------------- External extension function end--------------*/
static bool an_validate_message(struct an_glink_dev *an_dev,
    struct an_glink_resp_msg *resp_msg, size_t len)
{
    if (len != sizeof(*resp_msg)) {
        AN_PSY_ERR("Incorrect response length %zu for opcode %#x\n", len,
            resp_msg->hdr.opcode);
        return false;
    }

    if (resp_msg->ret_code) {
        AN_PSY_ERR("Error in response for opcode %#x prop_id %u, rc=%d\n",
            resp_msg->hdr.opcode, resp_msg->prop_id, (int)resp_msg->ret_code);
        return false;
    }

    return true;
}

static void an_glink_handle_message(struct an_glink_dev *an_dev,
                                     void *data, size_t len)
{
    struct an_glink_resp_msg *msg = data;
    struct an_psy_state *pst = NULL;
    bool ack_ok = false;

    if (!an_validate_message(an_dev, msg, len)) {
        return;
    }

    switch (msg->hdr.opcode) {
        case AN_GLINK_BATTERY_OPCODE_GET:
            pst = &an_dev->psy_list[AN_PSY_TYPE_BATTERY];
            pst->prop[msg->prop_id] = msg->value;
            ack_ok = true;
            break;
        case AN_GLINK_BATTERY_OPCODE_SET:
            ack_ok = true;
            break;
        case AN_GLINK_USB_OPCODE_GET:
            pst = &an_dev->psy_list[AN_PSY_TYPE_USB];
            pst->prop[msg->prop_id] = msg->value;
            ack_ok = true;
            break;
        case AN_GLINK_USB_OPCODE_SET:
            ack_ok = true;
            break;
        default:
            AN_PSY_ERR("Unknown opcode: %u\n", msg->hdr.opcode);
            break;
    }

    if (ack_ok) {
        complete(&an_dev->msg_ack);
    }
}

static void an_glink_handle_notify(struct an_glink_dev *an_dev,
                                    void *data, size_t len)
{
    struct an_glink_notify_rx_msg *notify_msg = data;
    u32 notification = 0;

    if (len != sizeof(*notify_msg)) {
        AN_PSY_ERR("invalid msg len: %u!=%u\n", len, sizeof(*notify_msg));
        return;
    }

    notification = notify_msg->notify_type;
    switch (notification) {
        case AN_TYPEC_CC_EVENT:
            g_an_usb.cc_orient = notify_msg->notify_value;
            break;
        // Modify by wenyaqi for QN7026A-14 on 20240119 start
        case AN_ANALOG_EARPHONE_ATTACH_EVENT:
            if (!(notify_msg->notify_value)) {
                g_an_analog_earphone_online = true;
                AN_PSY_INFO("Analog earphone attached \n");
            } else {
                g_an_analog_earphone_online = false;
                AN_PSY_INFO("Analog earphone detached \n");
            }
            break;
        // Modify by wenyaqi for QN7026A-14 on 20240119 end
        // Modify by liufurong for QN7026A-538 on 20240413 begin
        case AN_WLS_ATTACH_EVENT:
            schedule_wls_thermal_work(!!notify_msg->notify_value);
            break;
        // Modify by liufurong for QN7026A-538 on 20240413 end
        // Modify by liufurong for QN7026A-837 on 20240520 begin
        case AN_GET_OTG_STATUS:
            g_otg_status = !!notify_msg->notify_value;
            break;
        // Modify by liufurong for QN7026A-837 on 20240520 end
        default:
            break;
    }

    AN_PSY_INFO("notify msg: notify_type=%u notify_value=%u\n", notify_msg->notify_type, notify_msg->notify_value);
}

static int an_glink_msg_callback(void *dev_data, void *data, size_t len)
{
    struct pmic_glink_hdr *hdr = data;
    struct an_glink_dev *an_dev = dev_data;

    if (!an_dev || !hdr)
        return -ENODEV;

    AN_PSY_ERR("msg_callback: owner=%u type=%u opcode=%u len=%zu\n",
        hdr->owner, hdr->type, hdr->opcode, len);

    if (hdr->owner != AN_GLINK_MSG_OWNER_ID) {
        AN_PSY_ERR("invalid msg owner: %u\n", hdr->owner);
        return -EINVAL;
    }

    if (hdr->opcode == AN_GLINK_MSG_OPCODE_NOTIFY) {
        an_glink_handle_notify(an_dev, data, len);
    } else {
        an_glink_handle_message(an_dev, data, len);
    }

    return 0;
}

// callback function to notify pmic glink state in the event of a subsystem restart
// or a protection domain restart.
static void an_glink_state_callback(void *dev_data, enum pmic_glink_state state)
{
    struct an_glink_dev *an_dev = dev_data;

    if (!an_dev) {
        return;
    }

    AN_PSY_INFO("state_callback: state=%d\n", state);
}

static int an_glink_probe(struct platform_device *pdev)
{
    struct an_glink_dev *an_dev = NULL;
    struct pmic_glink_client_data client_data = { 0 };
    int ret = -EINVAL;
    int i = 0;

    AN_PSY_INFO("probe start\n");
    if (!pdev || !pdev->dev.of_node)
        return -ENODEV;

    an_dev = devm_kzalloc(&pdev->dev, sizeof(*an_dev), GFP_KERNEL);
    if (!an_dev)
        return -ENOMEM;

    an_dev->psy_list[AN_PSY_TYPE_BATTERY].opcode_get = AN_GLINK_BATTERY_OPCODE_GET;
    an_dev->psy_list[AN_PSY_TYPE_BATTERY].opcode_set = AN_GLINK_BATTERY_OPCODE_SET;
    an_dev->psy_list[AN_PSY_TYPE_BATTERY].prop_count = AN_BATT_PROP_MAX;
    an_dev->psy_list[AN_PSY_TYPE_USB].opcode_get = AN_GLINK_USB_OPCODE_GET;
    an_dev->psy_list[AN_PSY_TYPE_USB].opcode_set = AN_GLINK_USB_OPCODE_SET;
    an_dev->psy_list[AN_PSY_TYPE_USB].prop_count = AN_USB_PROP_MAX;

    for (i = 0; i < AN_PSY_TYPE_MAX; i++) {
        an_dev->psy_list[i].prop = devm_kcalloc(&pdev->dev, an_dev->psy_list[i].prop_count,
            sizeof(u32), GFP_KERNEL);
        if (!an_dev->psy_list[i].prop) {
            return -ENOMEM;
        }
    }

    an_dev->dev = &pdev->dev;

    mutex_init(&an_dev->send_lock);
    init_completion(&an_dev->msg_ack);

    client_data.id = AN_GLINK_MSG_OWNER_ID;
    client_data.name = AN_GLINK_MSG_NAME;
    client_data.msg_cb = an_glink_msg_callback;
    client_data.priv = an_dev;
    client_data.state_cb = an_glink_state_callback;
    an_dev->client = pmic_glink_register_client(an_dev->dev, &client_data);
    if (IS_ERR(an_dev->client)) {
        AN_PSY_ERR("glink register fail\n");
        ret = -EPROBE_DEFER;
        goto fail_free_mem;
    }

    platform_set_drvdata(pdev, an_dev);
    g_an_glink_dev = an_dev;

    AN_PSY_INFO("probe ok\n");

    return 0;

fail_free_mem:
    mutex_destroy(&an_dev->send_lock);
    return ret;
}

static int an_glink_remove(struct platform_device *pdev)
{
    struct an_glink_dev *an_dev = platform_get_drvdata(pdev);

    if (!an_dev) {
        return -ENODEV;
    }

    mutex_destroy(&an_dev->send_lock);
    pmic_glink_unregister_client(an_dev->client);
    g_an_glink_dev = NULL;
    AN_PSY_INFO("remove\n");
    return 0;
}

static const struct of_device_id an_glink_match_table[] = {
    {.compatible = "an,an_glink",},
    {},
};

static struct platform_driver an_glink_driver = {
    .probe = an_glink_probe,
    .remove = an_glink_remove,
    .driver = {
        .name = "an,an_glink",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(an_glink_match_table),
    },
};

int __init an_glink_init(void)
{
    AN_PSY_INFO("an_glink_init\n");
    return platform_driver_register(&an_glink_driver);
}

void __exit an_glink_exit(void)
{
    platform_driver_unregister(&an_glink_driver);
}

module_init(an_glink_init);
module_exit(an_glink_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("an glink module driver");
MODULE_AUTHOR("an_oem");
