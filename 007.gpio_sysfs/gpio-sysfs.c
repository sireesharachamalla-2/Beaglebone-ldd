#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>          
#include <linux/of_gpio.h>       
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>


#define MAX_DEVICES 4

struct gpio_sysfs_devdata {
    int gpio;
    struct device *dev;
};

struct gpio_sysfs_drvdata {
    struct class *class_gpio;
    struct gpio_sysfs_devdata dev_data[MAX_DEVICES];
    int total_devices;
};

static struct gpio_sysfs_drvdata gpio_drv_data;

/* sysfs read */
static ssize_t direction_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    struct gpio_sysfs_devdata *data = dev_get_drvdata(dev);
    int val = gpio_get_value(data->gpio);
    return sprintf(buf, "%d\n", val);
}

/* sysfs write */
static ssize_t direction_store(struct device *dev,struct device_attribute *attr,const char *buf, size_t count)
{
    struct gpio_sysfs_devdata *data = dev_get_drvdata(dev);
    int value;

    if (kstrtoint(buf, 0, &value))
        return -EINVAL;

    gpio_direction_output(data->gpio, value);
    gpio_set_value(data->gpio, value);
    return count;
}

static DEVICE_ATTR_RW(direction);

static int gpio_sysfs_probe(struct platform_device *pdev)
{
    struct device_node *child;
    struct gpio_sysfs_devdata *dev_data;

    for_each_child_of_node(pdev->dev.of_node, child) {
        int gpio;

        if (gpio_drv_data.total_devices >= MAX_DEVICES)
            break;

        gpio = of_get_named_gpio(child, "gpios", 0);
        if (!gpio_is_valid(gpio)) {
            dev_warn(&pdev->dev, "Invalid GPIO\n");
            continue;
        }

        if (gpio_request(gpio, "bone_gpio")) {
            dev_err(&pdev->dev, "Failed to request GPIO %d\n", gpio);
            continue;
        }

        gpio_direction_output(gpio, 0);

        dev_data = &gpio_drv_data.dev_data[gpio_drv_data.total_devices];
        dev_data->gpio = gpio;

        dev_data->dev = device_create(gpio_drv_data.class_gpio, NULL, 0,
                                      dev_data, "bone_gpio%d",
                                      gpio_drv_data.total_devices);
        if (IS_ERR(dev_data->dev)) {
            gpio_free(gpio);
            dev_err(&pdev->dev, "Failed to create device\n");
            continue;
        }

        device_create_file(dev_data->dev, &dev_attr_direction);
        dev_set_drvdata(dev_data->dev, dev_data);

        gpio_drv_data.total_devices++;
    }

    dev_info(&pdev->dev, "GPIO sysfs driver probed\n");
    return 0;
}

/* Note: return type is void in older kernels */
static void gpio_sysfs_remove(struct platform_device *pdev)
{
    int i;
    for (i = 0; i < gpio_drv_data.total_devices; i++) {
        device_remove_file(gpio_drv_data.dev_data[i].dev, &dev_attr_direction);
        device_unregister(gpio_drv_data.dev_data[i].dev);
        gpio_free(gpio_drv_data.dev_data[i].gpio);
    }
    gpio_drv_data.total_devices = 0;
}

static const struct of_device_id gpio_sysfs_dt_ids[] = {
    { .compatible = "bone,gpio-sysfs" },
    { }
};
MODULE_DEVICE_TABLE(of, gpio_sysfs_dt_ids);

static struct platform_driver gpiosysfs_platform_driver = {
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,   /* void remove() for old kernels */
    .driver = {
        .name = "gpio_sysfs_driver",
        .of_match_table = gpio_sysfs_dt_ids,
    },
};

static int __init gpio_sysfs_init(void)
{
    gpio_drv_data.class_gpio = class_create("bone_gpios"); /* old form */
    if (IS_ERR(gpio_drv_data.class_gpio))
        return PTR_ERR(gpio_drv_data.class_gpio);

    return platform_driver_register(&gpiosysfs_platform_driver);
}
static void __exit gpio_sysfs_exit(void)
{
    platform_driver_unregister(&gpiosysfs_platform_driver);
    class_destroy(gpio_drv_data.class_gpio);
}
module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("Legacy-compatible GPIO Sysfs Driver for BeagleBone");
