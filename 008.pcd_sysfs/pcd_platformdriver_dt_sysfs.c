#include "pcd_platform_driver_dt_sysfs.h"
static struct pcdrv_private_data pcdrv_data;
static const struct file_operations pcdev_fops = {
        .owner   = THIS_MODULE,
        .open    = pcdev_open,
        .release = pcdev_release,
        .read    = pcdev_read,
        .write   = pcdev_write,
        .llseek  = pcdev_lseek,
};
/* ---------- sysfs show/store functions ---------- */

ssize_t serial_num_show(struct device *dev,
                        struct device_attribute *attr,
                        char *buf)
{
        struct pcdev_private_data *dev_data = dev_get_drvdata(dev);

        return sprintf(buf, "%s\n", dev_data->pdata.serial_number);
}

ssize_t max_size_show(struct device *dev,
                      struct device_attribute *attr,
                      char *buf)
{
        struct pcdev_private_data *dev_data = dev_get_drvdata(dev);

        return sprintf(buf, "%u\n", dev_data->pdata.size);
}

ssize_t max_size_store(struct device *dev,
                       struct device_attribute *attr,
                       const char *buf,
                       size_t count)
{
        /* read-only in practice; reject writes */
        return -EPERM;
}

/* ---------- create 2 device_attribute variables ---------- */
static DEVICE_ATTR(serial_num, S_IRUGO,
                   serial_num_show, NULL);

static DEVICE_ATTR(max_size, S_IRUGO | S_IWUSR,
                   max_size_show, max_size_store);

static int pcdev_create_sysfs_files(struct device *dev)
{
        int ret;

        ret = sysfs_create_file(&dev->kobj,
                                &dev_attr_serial_num.attr);
        if (ret)
                return ret;

        ret = sysfs_create_file(&dev->kobj,
                                &dev_attr_max_size.attr);
        if (ret) {
                sysfs_remove_file(&dev->kobj,
                                  &dev_attr_serial_num.attr);
                return ret;
        }

        return 0;
}

static int pcdev_probe(struct platform_device *pdev)
{
        struct device *dev = &pdev->dev;
        struct pcdev_private_data *dev_data;
        struct device_node *np = dev->of_node;
        int ret;

        dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
        if (!dev_data)
                return -ENOMEM;

        of_property_read_u32(np, "org,size", &dev_data->pdata.size);
        of_property_read_u32(np, "org,perm", &dev_data->pdata.perm);
        of_property_read_string(np, "org,device-serial-num",
                                &dev_data->pdata.serial_number);

        dev_data->buffer = devm_kzalloc(dev,
                                        dev_data->pdata.size,
                                        GFP_KERNEL);

        dev_data->dev_num =
                pcdrv_data.dev_num_base + pcdrv_data.total_devices;

        cdev_init(&dev_data->cdev, &pcdev_fops);
        cdev_add(&dev_data->cdev, dev_data->dev_num, 1);

        pcdrv_data.device_pcdev[pcdrv_data.total_devices] =
                device_create(pcdrv_data.class_pcdev,
                              dev,
                              dev_data->dev_num,
                              NULL,
                              "pcdev-%d",
                              pcdrv_data.total_devices);

        platform_set_drvdata(pdev, dev_data);
        ret = pcdev_create_sysfs_files(dev);
        if (ret) {
                device_destroy(pcdrv_data.class_pcdev,
                               dev_data->dev_num);
                cdev_del(&dev_data->cdev);
                return ret;
        }
        pcdrv_data.total_devices++;

        pr_info("pcdev probed: %s\n", dev_name(dev));
        return 0;
}

/* ---------- remove ---------- */
static int pcdev_remove(struct platform_device *pdev)
{
        struct pcdev_private_data *dev_data =
                platform_get_drvdata(pdev);

        sysfs_remove_file(&pdev->dev.kobj,
                          &dev_attr_serial_num.attr);
        sysfs_remove_file(&pdev->dev.kobj,
                          &dev_attr_max_size.attr);

        device_destroy(pcdrv_data.class_pcdev, dev_data->dev_num);
        cdev_del(&dev_data->cdev);

        pr_info("pcdev removed\n");
        return 0;
}
MODULE_DEVICE_TABLE(of, pcdev_dt_match);


static struct platform_driver pcdev_driver = {
        .probe  = pcdev_probe,
        .remove = pcdev_remove,
        .driver = {
                .name = "pcdev-dt-driver",
                .of_match_table = pcdev_dt_match,
        },
};


static int __init pcdev_init(void)
{
	pr_info("module_loaded");
        alloc_chrdev_region(&pcdrv_data.dev_num_base,
                             0, MAX_DEVICES, "pcdev");
        pcdrv_data.class_pcdev =
                class_create(THIS_MODULE,"pcdev_class");

        return platform_driver_register(&pcdev_driver);
}

static void __exit pcdev_exit(void)
{
        platform_driver_unregister(&pcdev_driver);
        class_destroy(pcdrv_data.class_pcdev);
        unregister_chrdev_region(pcdrv_data.dev_num_base,
                                 MAX_DEVICES);
}

module_init(pcdev_init);
module_exit(pcdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("DT based pseudo char device driver");
