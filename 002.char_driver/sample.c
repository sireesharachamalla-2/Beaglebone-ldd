#include <linux/init.h>
#include <linux/module.h>

static int __init  SampleInit(void)
{
	pr_info("Sample Init Function called\n");
	return 0;
}

static void __exit SampleExit(void)
{
	pr_info("Sample Exit function called\n");
}

module_init(SampleInit);
module_exit(SampleExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
