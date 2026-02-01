#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>

#define RDWR 0x11

struct pcdev_platform_data {
        u32 size;
        u32 perm;
        const char *serial_number;
};

/* -------- probe function -------- */
static int pcdev_probe(struct platform_device *pdev)
{
        struct device *dev = &pdev->dev;
        struct device_node *np = dev->of_node;
        struct pcdev_platform_data pdata;
        int ret;

        pr_info("pcdev: probe called\n");

        if (!np) {
                pr_err("pcdev: no device tree node found\n");
                return -EINVAL;
        }

        /* Read size */
        ret = of_property_read_u32(np, "org,size", &pdata.size);
        if (ret) {
                pr_err("pcdev: failed to read org,size\n");
                return ret;
        }

        /* Read permission */
        ret = of_property_read_u32(np, "org,perm", &pdata.perm);
        if (ret) {
                pr_err("pcdev: failed to read org,perm\n");
                return ret;
        }

        /* Read serial number */
        ret = of_property_read_string(np, "org,device-serial-num",&pdata.serial_number);
        if (ret) {
                pr_err("pcdev: failed to read serial number\n");
                return ret;
        }
        pr_info("pcdev detected:\n");
        pr_info("  device name : %s\n", dev_name(dev));
        pr_info("  size        : %u\n", pdata.size);
        pr_info("  perm        : 0x%x\n", pdata.perm);
        pr_info("  serial      : %s\n", pdata.serial_number);
        return 0;
}
/* -------- remove function -------- */
static int pcdev_remove(struct platform_device *pdev)
{
        pr_info("pcdev: device removed (%s)\n", dev_name(&pdev->dev));
        return 0;
}
/* -------- DT match table -------- */
static const struct of_device_id pcdev_dt_match[] = {
        { .compatible = "pcdev-A1x" },
        { .compatible = "pcdev-B1x" },
        { .compatible = "pcdev-C1x" },
        { .compatible = "pcdev-D1x" },
        { }
};

MODULE_DEVICE_TABLE(of, pcdev_dt_match);

/* -------- platform driver -------- */
static struct platform_driver pcdev_driver = {
        .probe  = pcdev_probe,
        .remove = pcdev_remove,
        .driver = {
                .name = "pcdev-dt-driver",
                .of_match_table = pcdev_dt_match,
        },
};

module_platform_driver(pcdev_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("Pseudo Char Device DT based Platform Driver");
