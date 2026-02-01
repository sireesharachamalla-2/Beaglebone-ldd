#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "gpio_led"
#define CLASS_NAME  "gpio_class"

/* BeagleBone Black USR0 LED = GPIO1_21 = 53 */
#define LED_GPIO    53

static dev_t dev;
static struct cdev gpio_cdev;
static struct class *gpio_class;

/* ---------------- File Operations ---------------- */

static int gpio_open(struct inode *inode, struct file *file)
{
    pr_info("GPIO LED device opened\n");
    return 0;
}

static int gpio_release(struct inode *inode, struct file *file)
{
    pr_info("GPIO LED device closed\n");
    return 0;
}

static ssize_t gpio_write(struct file *file,
                          const char __user *buf,
                          size_t len,
                          loff_t *off)
{
    char kbuf;

    if (copy_from_user(&kbuf, buf, 1))
        return -EFAULT;

    if (kbuf == '1') {
        gpio_set_value(LED_GPIO, 1);
        pr_info("LED ON\n");
    }
    else if (kbuf == '0') {
        gpio_set_value(LED_GPIO, 0);
        pr_info("LED OFF\n");
    }
    else {
        pr_err("Invalid input! Use 1 or 0\n");
    }

    return len;
}

/* ---------------- File ops structure ---------------- */

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = gpio_open,
    .release = gpio_release,
    .write   = gpio_write,
};

/* ---------------- Module Init ---------------- */

static int __init gpio_init(void)
{
    int ret;

    /* Allocate major/minor */
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    /* Register cdev */
    cdev_init(&gpio_cdev, &fops);
    cdev_add(&gpio_cdev, dev, 1);

    /* Create device node */
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    device_create(gpio_class, NULL, dev, NULL, DEVICE_NAME);

    /* Request GPIO */
    if (!gpio_is_valid(LED_GPIO)) {
        pr_err("Invalid GPIO %d\n", LED_GPIO);
        return -ENODEV;
    }

    gpio_request(LED_GPIO, "gpio_led");
    gpio_direction_output(LED_GPIO, 0);

    pr_info("GPIO LED Driver Loaded Successfully\n");
    return 0;
}

/* ---------------- Module Exit ---------------- */

static void __exit gpio_exit(void)
{
    gpio_set_value(LED_GPIO, 0);
    gpio_free(LED_GPIO);

    device_destroy(gpio_class, dev);
    class_destroy(gpio_class);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("GPIO LED Driver Unloaded\n");
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("GPIO LED Driver for BeagleBone Black");
