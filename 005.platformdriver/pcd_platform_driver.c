#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>

#define RDWR 0x11

struct pcdev_platform_data {
        int size;
        int perm;
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
        dev_t device_num_base;
        struct class *class_pcdev;
};

static struct pcdrv_private_data pcdrv_data;

int pcd_open(struct inode *inode, struct file *filp)
{
        pr_info("open successful\n");
        return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
        pr_info("release successful\n");
        return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buf,
                 size_t count, loff_t *f_pos)
{
        pr_info("read requested\n");
        return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buf,
                  size_t count, loff_t *f_pos)
{
        pr_info("write requested\n");
        return count;
}

loff_t pcd_lseek(struct file *filp, loff_t off, int whence)
{
        return 0;
}

struct file_operations pcd_fops = {
        .open    = pcd_open,
        .release = pcd_release,
        .read    = pcd_read,
        .write   = pcd_write,
        .llseek  = pcd_lseek,
        .owner   = THIS_MODULE
};

int pcdev_platform_driver_probe(struct platform_device *pdev)
{
        struct pcdev_private_data *dev_data;
        struct pcdev_platform_data *pdata;

        pr_info("Device detected : %d\n", pdev->id);

        pdata = (struct pcdev_platform_data *)pdev->dev.platform_data;
        if (!pdata) {
                pr_err("No platform data available\n");
                return -EINVAL;
        }

        dev_data = kzalloc(sizeof(*dev_data), GFP_KERNEL);
        if (!dev_data)
                return -ENOMEM;

        /* copy platform data */
        dev_data->pdata = *pdata;

        /* allocate buffer */
        dev_data->buffer = kzalloc(dev_data->pdata.size, GFP_KERNEL);
        if (!dev_data->buffer)
                return -ENOMEM;

        dev_data->dev_num = pcdrv_data.device_num_base + pdev->id;

        cdev_init(&dev_data->cdev, &pcd_fops);
        dev_data->cdev.owner = THIS_MODULE;
        cdev_add(&dev_data->cdev, dev_data->dev_num, 1);

        device_create(pcdrv_data.class_pcdev, NULL,dev_data->dev_num, NULL,"pcdev-%d", pdev->id);

        platform_set_drvdata(pdev, dev_data);

        pr_info("Device %d initialized (size=%d, serial=%s)\n",
                pdev->id,
                dev_data->pdata.size,
                dev_data->pdata.serial_number);

        pcdrv_data.total_devices++;
        return 0;
}

int  pcdev_platform_driver_remove(struct platform_device *pdev)
{
        struct pcdev_private_data *dev_data = platform_get_drvdata(pdev);

        device_destroy(pcdrv_data.class_pcdev, dev_data->dev_num);
        cdev_del(&dev_data->cdev);
        kfree(dev_data->buffer);
        kfree(dev_data);

        pr_info("Device removed : %d\n", pdev->id);
       return 0;
}

struct platform_driver pcdev_platform_driver = {
        .probe  = pcdev_platform_driver_probe,
        .remove = pcdev_platform_driver_remove,
        .driver = {
                .name = "pseudo-char-device"
        }
};

static int __init pcdev_driver_init(void)
{
        alloc_chrdev_region(&pcdrv_data.device_num_base,
                            0, 2, "pcdevs");

        pcdrv_data.class_pcdev = class_create(THIS_MODULE,"pcdev_class");

        platform_driver_register(&pcdev_platform_driver);

        pr_info("Platform driver loaded\n");
        return 0;
}
static void __exit pcdev_driver_exit(void)
{
        platform_driver_unregister(&pcdev_platform_driver);
        class_destroy(pcdrv_data.class_pcdev);
        unregister_chrdev_region(pcdrv_data.device_num_base, 2);

        pr_info("Platform driver unloaded\n");
}

module_init(pcdev_driver_init);
module_exit(pcdev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("Platform driver for pseudo char devices");
