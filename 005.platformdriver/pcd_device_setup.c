#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

static void pcdev_release(struct device* dev);

/* Create 2 platform data. */
struct pcdev_platform_data pcdev_pdata[2] = {
	[0] = {.size = 512, .perm = RDWR, .serialNumber = "PCDEVABC111"},
	[1] = {.size = 1024, .perm = RDWR, .serialNumber = "PCDEVXYZ222"}
};

/* Create 2 platform device. */
struct platform_device platform_pcdev_1 = {
	.name = "pseudo_char_device",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_pdata[0],
		.release = pcdev_release
	}
};

struct platform_device platform_pcdev_2 = {
	.name = "pseudo_char_device",
	.id = 1,
	.dev = {
		.platform_data = &pcdev_pdata[1],
		.release = pcdev_release
	}
};
static int __init pcdev_platform_init(void)
{
	/* Register Platform device.  */
	platform_device_register(&platform_pcdev_1);
	platform_device_register(&platform_pcdev_2);

	pr_info("Device Setup module Inserted\n");
	
	return 0;
}
static void __exit pcdev_platform_exit(void)
{
	/* Un-register the platform device. */
	platform_device_unregister(&platform_pcdev_1);
	platform_device_unregister(&platform_pcdev_2);
	
	pr_info("Device Setup module removed\n");
}

static void pcdev_release(struct device* dev)
{
	pr_info("Device Released\n");
}
module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module Which Registers platform devices");
MODULE_AUTHOR("Sireesha");
