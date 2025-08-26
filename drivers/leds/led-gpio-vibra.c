// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2017-2020, The Linux Foundation. All rights reserved. */
/* Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved. */

#define pr_fmt(fmt)    "%s: " fmt, __func__

#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <linux/gpio/consumer.h>

struct vib_gpio_chip {
    struct led_classdev    cdev;
    struct gpio_desc *gpio;
    struct mutex        lock;
    int            state;
};

static int qpnp_vibrator_play_on(struct vib_gpio_chip *chip)
{
    gpiod_set_value_cansleep(chip->gpio, 1);
    pr_info("vibration enable gpio124 success\n");

    return 0;
}

static enum led_brightness qpnp_vib_brightness_get(struct led_classdev *cdev)
{
    struct vib_gpio_chip *chip = container_of(cdev, struct vib_gpio_chip,
                        cdev);

    return chip->state;
}

static void qpnp_vib_brightness_set(struct led_classdev *cdev,
            enum led_brightness level)
{
    struct vib_gpio_chip *chip = container_of(cdev, struct vib_gpio_chip,
                        cdev);
    int ret = 0;

    chip->state = level;

    if (chip->state) {
        ret = qpnp_vibrator_play_on(chip);
        if (ret < 0)
            pr_err("set vibrator-on failed, ret=%d\n", ret);
    } else {
        gpiod_set_value_cansleep(chip->gpio, 0);
    }

    pr_debug("vibrator state=%d\n", chip->state);
}

static int qpnp_vibrator_gpio_suspend(struct device *dev)
{
    struct vib_gpio_chip *chip = dev_get_drvdata(dev);

    mutex_lock(&chip->lock);
    gpiod_set_value_cansleep(chip->gpio, 0);
    mutex_unlock(&chip->lock);

    return 0;
}
static SIMPLE_DEV_PM_OPS(qpnp_vibrator_gpio_pm_ops, qpnp_vibrator_gpio_suspend,
            NULL);

static int qpnp_vibrator_gpio_probe(struct platform_device *pdev)
{
    struct vib_gpio_chip *chip;
    int ret;

    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip)
        return -ENOMEM;

    chip->gpio = devm_gpiod_get(&pdev->dev, "enable", GPIOD_OUT_LOW);
    ret = PTR_ERR_OR_ZERO(chip->gpio);
    if (ret) {
        pr_err("dts gpio enable read failed, ret=%d\n", ret);
        return ret;
    }

    mutex_init(&chip->lock);
    dev_set_drvdata(&pdev->dev, chip);

    chip->cdev.name = "vibrator";
    chip->cdev.brightness_get = qpnp_vib_brightness_get;
    chip->cdev.brightness_set = qpnp_vib_brightness_set;
    chip->cdev.max_brightness = 100;
    ret = devm_led_classdev_register(&pdev->dev, &chip->cdev);
    if (ret < 0) {
        pr_err("Error in registering led class device, ret=%d\n", ret);
        goto fail;
    }

    return 0;

fail:
    mutex_destroy(&chip->lock);
    dev_set_drvdata(&pdev->dev, NULL);
    return ret;
}

static int qpnp_vibrator_gpio_remove(struct platform_device *pdev)
{
    struct vib_gpio_chip *chip = dev_get_drvdata(&pdev->dev);

    mutex_destroy(&chip->lock);
    dev_set_drvdata(&pdev->dev, NULL);

    return 0;
}

static const struct of_device_id vibrator_gpio_match_table[] = {
    { .compatible = "qcom,qpnp-vibrator-gpio" },
    { /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, vibrator_gpio_match_table);

static struct platform_driver qpnp_vibrator_gpio_driver = {
    .driver = {
        .name        = "qcom,qpnp-vibrator-gpio",
        .of_match_table    = vibrator_gpio_match_table,
        .pm        = &qpnp_vibrator_gpio_pm_ops,
    },
    .probe = qpnp_vibrator_gpio_probe,
    .remove = qpnp_vibrator_gpio_remove,
};
module_platform_driver(qpnp_vibrator_gpio_driver);

MODULE_DESCRIPTION("QPNP Vibrator-GPIO driver");
MODULE_LICENSE("GPL v2");
