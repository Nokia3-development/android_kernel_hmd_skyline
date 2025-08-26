//#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include "sensor_user_node.h"
// Added by huofudong for TCT-837 on 20240311 begin
#include <linux/proc_fs.h>
#include <linux/fs.h>
// Added by huofudong for TCT-837 on 20240311 end


//sar point
sar_driver_func_t g_usernode;
EXPORT_SYMBOL(g_usernode);

//sar create node
struct class *g_sensor_usertest;
struct device *g_sar_node;

//sar onoff
static ssize_t onoff_store(struct device *dev,
                        struct device_attribute *attr,
                        const char *buf, size_t count)
{
    if (g_usernode.set_onoff) {
        g_usernode.set_onoff(buf, count);
    } else {
        pr_err("%s: set_onoff fail\n", __func__);
    }

    return count;
}

ssize_t onoff_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    int len = 0;

    if (g_usernode.get_onoff) {
        len = g_usernode.get_onoff(buf);
    }

    pr_err("%s: get_onoff stauts\n", __func__);

    return len;
}
static DEVICE_ATTR(onoff, 0664, onoff_show, onoff_store);

static ssize_t dumpinfo_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    if (g_usernode.set_dumpinfo) {
        g_usernode.set_dumpinfo(buf, count);
    } else {
        pr_err("%s: set_dumpinfo fail\n", __func__);
    }

    return count;
}

ssize_t dumpinfo_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    int len = 0;

    if (g_usernode.get_dumpinfo) {
        len = g_usernode.get_dumpinfo(buf);
    }

    pr_err("%s: get_dumpinfo stauts\n", __func__);

    return len;
}
static DEVICE_ATTR(dumpinfo, 0664, dumpinfo_show, dumpinfo_store);

static ssize_t userenable_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    if (g_usernode.set_enable) {
        g_usernode.set_enable(buf, count);
    } else {
        pr_err("%s: set_enable fail\n", __func__);
    }

    return count;
}

ssize_t userenable_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    int len = 0;

    if (g_usernode.get_enable) {
        len = g_usernode.get_enable(buf);
    }

    pr_err("%s: get_enable stauts\n", __func__);

    return len;
}
static DEVICE_ATTR(userenable, 0664, userenable_show, userenable_store);

static ssize_t usercali_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    if (g_usernode.get_cali) {
        g_usernode.get_cali();
    } else {
        pr_err("%s: sar cali fail\n", __func__);
    }

    return count;
}
static DEVICE_ATTR(usercali, 0220, NULL, usercali_store);

static struct device_attribute *sensor_attrs[] = {
    &dev_attr_onoff,
    &dev_attr_dumpinfo,
    &dev_attr_userenable,
    &dev_attr_usercali,
    NULL,
};

#define USER_NODE_NUM  ARRAY_SIZE(sensor_attrs)
static int gs_sensor_flag[USER_NODE_NUM] = {0};

// Added by huofudong for TCT-837 on 20240311 begin
static ssize_t sar_read_name(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int r = 0;
    char kernel_buf[10] = {0};
    char name[10] = "sx933x";

    count = sizeof(kernel_buf);
    snprintf(kernel_buf, count, "%s\n", name);

    r = simple_read_from_buffer(buf, count, ppos, kernel_buf, strlen(kernel_buf));

    return r;
}

static const struct proc_ops sar_ic = {
    .proc_read = sar_read_name,
};

static int sensor_node_init(void)
{
    int i = 0;
    struct proc_dir_entry *p_sarnode = NULL;
    p_sarnode = proc_create("sar_name", S_IRUGO, NULL, &sar_ic);
    if (NULL == p_sarnode) {
        pr_err("%s: sar_name proc failed!\n", __func__);
        remove_proc_entry("sar_name", NULL);
    }
    //sys/class/sensor_usertest/
    g_sensor_usertest = class_create(THIS_MODULE, "sensor_usertest");
    if (IS_ERR(g_sensor_usertest)) {
        pr_err("%s: create sensor_usertest is failed.\n", __func__);
        return PTR_ERR(g_sensor_usertest);
    }

    //sys/class/sensor_usertest/sar_node/
    g_sar_node = device_create(g_sensor_usertest, NULL, 0, NULL, "sar_node");
    if (IS_ERR(g_sar_node)) {
        pr_err("%s: device_create failed!\n", __func__);
        device_destroy(g_sensor_usertest, 0);
        class_destroy(g_sensor_usertest);
        return PTR_ERR(g_sar_node);
    }

    for (i = 0; sensor_attrs[i] != NULL; i++) {
        gs_sensor_flag[i] = device_create_file(g_sar_node, sensor_attrs[i]);

        if (gs_sensor_flag[i]) {
            pr_err("%s: fail device_create_file, sensor_attrs[%d])\n", __func__, i);
            device_destroy(g_sensor_usertest, 0);
            class_destroy(g_sensor_usertest);
            return gs_sensor_flag[i];
        }
    }

    return 0;
}

static void sensor_node_exit(void)
{
    int i = 0;
    remove_proc_entry("sar_name", NULL);
    for (i = 0; sensor_attrs[i] != NULL; i++) {
        device_remove_file(g_sar_node, sensor_attrs[i]);
    }
    device_destroy(g_sensor_usertest, 0);
    class_destroy(g_sensor_usertest);
    g_sensor_usertest = NULL;
}
// Added by huofudong for TCT-837 on 20240311 end
module_init(sensor_node_init);
module_exit(sensor_node_exit);

MODULE_DESCRIPTION("sensor user test node");
MODULE_AUTHOR("sar sensor");
MODULE_LICENSE("GPL");
