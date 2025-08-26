/*
 * Copyright (C) 2016-2018 Hisense, Inc.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

// Added by yusen for TCT-849 on 20240125 begin
#define SIM_SWITCH_ESIM  0
#define SIM_SWITCH_PSIM2 1
#define SIM_POWER_ESIM  1
#define SIM_POWER_PSIM2 0
// Added by yusen for TCT-849 on 20240125 end

struct sim_switch_driver_data {
	int sim_power_pin;
	int sim_switch_pin;
	int sim_num;
	int sim_switch_val;
	// int sd_tray_pin;
};
static struct sim_switch_driver_data sim_switch_pdata={0};

static ssize_t sim_switch_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", sim_switch_pdata.sim_switch_val);
}

static ssize_t sim_switch_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
	int ret=0;
	long value;
	// char buff[32] = {0};
	ret = kstrtol(buf, 10, &value);
	if (ret) {
		pr_err("%s: sscanf is wrong!\n", __func__);
		return -EINVAL;
	}

	pr_err("%s: sim switch %ld\n", __func__, value);

	// Modify by yusen for TCT-849 on 20240125 begin
	if (!!value) {
		if (gpio_is_valid(sim_switch_pdata.sim_switch_pin)) {
			gpio_direction_output(sim_switch_pdata.sim_power_pin, SIM_POWER_ESIM);
			gpio_direction_output(sim_switch_pdata.sim_switch_pin, SIM_SWITCH_ESIM);
			sim_switch_pdata.sim_switch_val = 1;
		}
	} else {
		if (gpio_is_valid(sim_switch_pdata.sim_switch_pin)) {
			gpio_direction_output(sim_switch_pdata.sim_power_pin, SIM_POWER_PSIM2);
			gpio_direction_output(sim_switch_pdata.sim_switch_pin, SIM_SWITCH_PSIM2);
			sim_switch_pdata.sim_switch_val = 0;
		}
	}
	pr_err("%s: sim_switch_val: %d ,sim_power_pin power: %d ,sim_switch_pin power: %d \n", __func__, sim_switch_pdata.sim_switch_val, gpio_get_value(sim_switch_pdata.sim_power_pin), gpio_get_value(sim_switch_pdata.sim_switch_pin));
	// Modify by yusen for TCT-849 on 20240125 end
	return len;
}
static struct kobj_attribute sim_switch_attr = __ATTR_RW(sim_switch);

static struct attribute *sim_switch_attrs[] = {
	&sim_switch_attr.attr,
	NULL
};

static const struct attribute_group sim_switch_group = {
	.attrs = sim_switch_attrs,
};

void sim_switch_update_cmdline_mode(void) {
    struct device_node *of_chosen = NULL;
    const char *pcbaload = NULL;
    pr_err("%s enter\n", __func__);
    of_chosen = of_find_node_by_path("/chosen");
    if (!of_chosen) {
        of_chosen = of_find_node_by_path("/chosen@0");
    }
    of_property_read_string(of_chosen, "bootargs", &pcbaload);
    if (!pcbaload) {
        pr_info("%s: read cmdline failed!", __func__);
        return;
    }

    if (strnstr(pcbaload, "simslot=SS", strlen(pcbaload))){
        sim_switch_pdata.sim_num= 1;
    } else if (strnstr(pcbaload, "simslot=DS", strlen(pcbaload))) {
        sim_switch_pdata.sim_num= 2;
    }

    if (strnstr(pcbaload, "esimflag=PE", strlen(pcbaload))){
        sim_switch_pdata.sim_switch_val= 1;
    } else if (strnstr(pcbaload, "esimflag=PP", strlen(pcbaload))) {
        sim_switch_pdata.sim_switch_val= 0;
    }

    pr_err("%s sim_num:%d sim_switch_val:%d\n", __func__, sim_switch_pdata.sim_num, sim_switch_pdata.sim_switch_val);
}

static int sim_switch_probe(struct platform_device *pdev)
{
	int err = 0;
	struct sim_switch_driver_data *pdata = &sim_switch_pdata;
	struct device_node *np = pdev->dev.of_node;
	struct kobject *root_kobjs;

	pr_err("%s enter\n", __func__);
	sim_switch_update_cmdline_mode();

	pdata->sim_power_pin = -1;
	pdata->sim_power_pin = of_get_named_gpio(np, "hmd,sim-power-pin", 0);
	if (gpio_is_valid(pdata->sim_power_pin)) {
		err = gpio_request(pdata->sim_power_pin, "GPIO125");
		if (err)
			pr_err("sim_power_pin: sim_power_pin gpio request failed %d\n", err);
		if (sim_switch_pdata.sim_switch_val == 1)
			gpio_direction_output(pdata->sim_power_pin, SIM_POWER_ESIM);
		else
			gpio_direction_output(pdata->sim_power_pin, SIM_POWER_PSIM2);
		pr_err("%s: default sim_switch_val: %d ,sim_power_pin power: %d \n", __func__,sim_switch_pdata.sim_switch_val ,gpio_get_value(sim_switch_pdata.sim_power_pin));
	} else {
		pr_err("sim_power_pin: no sim_power_pin define in dts\n");
	}

	pdata->sim_switch_pin = -1;
	pdata->sim_switch_pin = of_get_named_gpio(np, "hmd,sim-switch-pin", 0);
	if (gpio_is_valid(pdata->sim_switch_pin)) {
		err = gpio_request(pdata->sim_switch_pin, "GPIO6");
		if (err)
			pr_err("sim_switch_pin: sim_switch_pin gpio request failed %d\n", err);
		// Modify by yusen for TCT-849 on 20240125 begin
		if (sim_switch_pdata.sim_switch_val == 1)
			gpio_direction_output(pdata->sim_switch_pin, SIM_SWITCH_ESIM);
		else
			gpio_direction_output(pdata->sim_switch_pin, SIM_SWITCH_PSIM2);
		pr_err("%s: default sim_switch_val: %d ,sim_switch_pin power: %d \n", __func__,sim_switch_pdata.sim_switch_val ,gpio_get_value(sim_switch_pdata.sim_switch_pin));
		// Modify by yusen for TCT-849 on 20240125 end
	} else {
		pr_err("sim_switch_pin: no sim_switch_pin define in dts\n");
	}

	/* create ctrl node */
	root_kobjs = kobject_create_and_add("sim_switch", NULL);
	err = sysfs_create_group(root_kobjs, &sim_switch_group);
	if (err < 0)
		pr_err("Error create sim_switch on/if sysfs %d\n", err);

	return err;
}

static int sim_switch_remove(struct platform_device *pdev)
{
	//struct sim_switch_driver_data *pdata = &sim_switch_pdata;
	return 0;
}

static const struct of_device_id sim_switch_of_match[] = {
	{ .compatible = "hmd,sim-switch"},
	{},
};

MODULE_DEVICE_TABLE(of, sim_switch_of_match);

static struct platform_driver sim_switch_driver = {
	.driver = {
		.name = "sim_switch_driver",
		.owner = THIS_MODULE,
		.of_match_table	= sim_switch_of_match,
	},
	.probe = sim_switch_probe,
	.remove = sim_switch_remove,
};

static int __init sim_switch_init(void)
{
	int ret = 0;
	ret = platform_driver_register(&sim_switch_driver);
	return ret;
}

static void __exit sim_switch_exit(void)
{
	platform_driver_unregister(&sim_switch_driver);
	return;
}

module_init(sim_switch_init);
module_exit(sim_switch_exit);

MODULE_DESCRIPTION("Huaqin sim_switch_ctrl driver");
MODULE_LICENSE("GPL");