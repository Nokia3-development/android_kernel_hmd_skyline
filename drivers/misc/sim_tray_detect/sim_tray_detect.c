#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/kobject.h>
#include <linux/device.h>

static int sim_tray_detect_gpio = -1;

static int sim_tray_parse_dt(struct device *dev)
{
    struct device_node *np = dev->of_node;

    pr_err("[%s]l=%d: parse gpio_power!\n", __FUNCTION__, __LINE__);
    sim_tray_detect_gpio = of_get_named_gpio(np, "qcom,sim_tray_gpio", 0);
    if (sim_tray_detect_gpio < 0) {
        pr_err("get sim_tray,pvdd_gpio err!!!:%d\n", sim_tray_detect_gpio);
        return -1;
    }
    pr_err("sim_tray,pvdd_gpio=%d\n", sim_tray_detect_gpio);

    return 0;
}

static ssize_t hw_sd_tray_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    pr_info("[%s]l=%d: gpio=%d\n",
        __FUNCTION__, __LINE__, gpio_get_value(sim_tray_detect_gpio));

    return sprintf(buf, "%d\n", gpio_get_value(sim_tray_detect_gpio));
}

static struct kobj_attribute hw_sd_tray_attr = __ATTR(hw_sd_tray, 0644, hw_sd_tray_show, NULL);

static const struct of_device_id sim_tray_match_table[] = {
	{ .compatible = "qcom,sim_tray_detect",},
	{},
};

static struct attribute *hw_sd_tray_attrs[] = {
    &hw_sd_tray_attr.attr,
    NULL,
};

static const struct attribute_group hw_sd_tray_group = {
    .attrs = hw_sd_tray_attrs,
};

static struct kobject *root_kobjs = NULL;

// Modify by liufurong for TCT-837 on 20240514 begin
struct kobject *get_root_kobjs(void)
{
    return root_kobjs;
}
EXPORT_SYMBOL(get_root_kobjs);
// Modify by liufurong for TCT-837 on 20240514 end
static int sim_tray_probe(struct platform_device *pdev)
{
    int ret = 0;

    ret = sim_tray_parse_dt(&pdev->dev);
    if (ret < 0) {
        pr_err("sim_tray_parse_dt ret=%d\n", ret);
        return -1;
    }
    ret = gpio_request(sim_tray_detect_gpio, "sim_tray_detect_gpio");
    if (ret != 0) {
        pr_err("failed to request sim_tray gpio!\n");
    }

    // /sys/ontim_bootinfo/
    root_kobjs = kobject_create_and_add("ontim_bootinfo", NULL);
    if (!root_kobjs) {
        pr_err("ontim_bootinfo create failed\n");
        return -ENOMEM;
    }
    // /sys/ontim_bootinfo/hw_sd_tray
    ret = sysfs_create_group(root_kobjs, &hw_sd_tray_group);
    if (ret < 0) {
        pr_err("Error create ontim_bootinfo on/if sysfs %d\n", ret);
        return -1;
    }

    pr_err("[%s]l=%d: success\n", __FUNCTION__, __LINE__);

    return 0;
}

static int sim_tray_remove(struct platform_device *pdev)
{
    pr_info("[%s]l=%d\n", __FUNCTION__, __LINE__);

    return 0;
}

static struct platform_driver sim_tray_driver = {
    .driver = {
        .name = "sim-tray",
        .owner = THIS_MODULE,
        .of_match_table = sim_tray_match_table,
    },
    .probe =  sim_tray_probe,
    .remove = sim_tray_remove,
};

static int __init sim_tray_init(void)
{
    pr_err("[%s]l=%d:\n", __FUNCTION__, __LINE__);
    return platform_driver_register(&sim_tray_driver);
}

static void __exit sim_tray_exit(void)
{
    pr_err("[%s]l=%d:\n", __FUNCTION__, __LINE__);
    platform_driver_unregister(&sim_tray_driver);
}

late_initcall(sim_tray_init);
module_exit(sim_tray_exit);
MODULE_LICENSE("GPL");