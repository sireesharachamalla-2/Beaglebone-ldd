#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include<linux/device.h>
#include <linux/of_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define RDWR 0x11
#define MAX_DEVICES 4

int check_permission(int dev_perm, int acc_mode);
int pcdev_open(struct inode *inode, struct file *filp);
int pcdev_release(struct inode *inode, struct file *filp);
ssize_t pcdev_read(struct file *filp, char __user *buf,size_t count, loff_t *f_pos);
ssize_t pcdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
loff_t pcdev_lseek(struct file *filp, loff_t offset, int whence);

static const struct of_device_id pcdev_dt_match[] = {
        { .compatible = "pcdev-A1x" },
        { .compatible = "pcdev-B1x" },
        { .compatible = "pcdev-C1x" },
        { .compatible = "pcdev-D1x" },
        { }
};

struct pcdev_platform_data {
        u32 size;
        u32 perm;
        const char *serial_number;
};

struct pcdev_private_data {
        struct pcdev_platform_data pdata;
        char *buffer;
        dev_t dev_num;
        struct cdev cdev;
};

struct pcdrv_private_data {
        int total_devices;
        dev_t dev_num_base;
        struct class *class_pcdev;
        struct device *device_pcdev[MAX_DEVICES];
};
