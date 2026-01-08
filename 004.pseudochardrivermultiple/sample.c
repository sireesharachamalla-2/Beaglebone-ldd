#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

/* Macro Definition. */
#define DEV_MEM_SIZE		(512)
#define DEVICE_NAME		("pcd_devices")

loff_t pcd_llseek(struct file * filp, loff_t off, int whence);
ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos);
ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos);
int pcd_open(struct inode * inode, struct file * file);
int pcd_release(struct inode * inode, struct file * filr);

/* Global Variable Declaration. */
char cDevBuffer[DEV_MEM_SIZE];		/*< Pseudo Device, Read and Write Operations Will Be carried out here. */
dev_t uiDeviceNumber;			/*< This Holds The Device Number.     */
struct cdev tPcdCdev;			/*< Cdev Variable.		       */
struct class* class_pcd;		/*< Class Variable.		       */
struct device* device_pcd;		/*< Device variable.		      */ 

/*< File operations for the driver. */
struct file_operations tPcdFops = {
	.open   = pcd_open,
	.release  = pcd_release,
	.write  = pcd_write,
	.read   = pcd_read,
	.llseek = pcd_llseek,
	.owner  = THIS_MODULE
};
/*    init        */
static int __init PcdInit(void)
{
	int iErrRet;

	/* 1. Dynamically Allocate Device Number. */
	iErrRet = alloc_chrdev_region(&uiDeviceNumber, 0, 1,DEVICE_NAME);

	/* Check if error exits. */
	if(iErrRet < 0)
	{
		goto out;
	}

	pr_info("Device Number : %d\n",uiDeviceNumber);

	pr_info("Major Number: %d, Minor Number : %d\n",MAJOR(uiDeviceNumber), MINOR(uiDeviceNumber));	

	/* 2. Initialize the cdev structure with the file operations. */
	cdev_init(&tPcdCdev, &tPcdFops);

	/* 3. Register a device (cdev structure) with VFS. */
	tPcdCdev.owner = THIS_MODULE;
	iErrRet = cdev_add(&tPcdCdev, uiDeviceNumber, 1);

	/* Check if error exits. */
	if(iErrRet < 0)
	{
		goto unreg_chardev;
	}

	/* 4. Create device class under /sys/class/. */
	class_pcd = class_create(THIS_MODULE,"pcd_class");

	/* Check for valid pointer. */
	if(IS_ERR(class_pcd))
	{
		/* Class creation failed. */
		iErrRet = PTR_ERR(class_pcd);

		goto cdev_del;
	}
	/* 5. Populate the sysfs with device information (device creation). */
	device_pcd = device_create(class_pcd, NULL, uiDeviceNumber, NULL, "pcd");
	/* Check for valid pointer. */
	if(IS_ERR(device_pcd))
	{
		/* Device creation failed. */
		iErrRet = PTR_ERR(device_pcd);

		goto class_del;
	}
	pr_info("Module inti was successful\n");
	return 0;

	class_del:
		class_destroy(class_pcd);

	cdev_del:
		cdev_del(&tPcdCdev);

	unreg_chardev:
		unregister_chrdev_region(uiDeviceNumber, 1);

	out:
		return iErrRet;
}
/*         exit   */
static void __exit PcdExit(void)
{
	/* 1. Device destroy. */
	device_destroy(class_pcd, uiDeviceNumber);

	/* 2. Class Destroy. */
	class_destroy(class_pcd);

	/* 3. Remove the cdefv registration from the kernel VFS. */
	cdev_del(&tPcdCdev);

	/* 4. Unregister range of device numbers. */
	unregister_chrdev_region(uiDeviceNumber, 1);

	/* Exit Message. */
	pr_info("Module Unloaded\n");
}
/*             lseek   */
loff_t pcd_llseek(struct file * filp, loff_t off, int whence)
{
	/* Variable */
	loff_t temp = 0;

	/* Print Entry Message. */
	pr_info("lseek requested...\n");
	pr_info("Current file position : %lld\n",filp->f_pos);

	/* Switch according to whence value. */
	switch(whence)
	{
		/* The file offset is set to offset (off) bytes. */
		case SEEK_SET:
			if((off > DEV_MEM_SIZE) || (off < 0))
			{
				return -EINVAL;
			}

			filp->f_pos = off;
			break;

		/* The file offset is set to its current location + off bytes. */
		case SEEK_CUR:
			temp = filp->f_pos + off;
			if( (temp > DEV_MEM_SIZE) || (temp < 0 ))
			{
				return -EINVAL;
			}	
			filp->f_pos += off;
			break;

		/* Offset is set to the size of file + off bytes. */
		case SEEK_END:
			temp  = DEV_MEM_SIZE + off;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
			{
				return -EINVAL;
			}
			filp->f_pos = DEV_MEM_SIZE + off;
			break;	

		/* Invalid whence. */
		default:
			return -EINVAL;
	}

	/* Print updated file position. */
	pr_info("New Value of File position: %lld\n",filp->f_pos); 

	/* Return the updated file offset. */
        return filp->f_pos;
}
/*                read       */
ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
	/* Read requested of n bytes. */
	pr_info("Read Requested for %zu bu=ytes\n",count);
	
	pr_info("Current file position : %lld\n", *f_pos);

        /* Adjust the count. */
        if((*f_pos + count) > DEV_MEM_SIZE)
        {
                count = DEV_MEM_SIZE - *f_pos;
        }

        /* Copy to user. */
       if(copy_to_user(buff, &cDevBuffer[*f_pos], count))
       {
	       return -EFAULT;
       }

       /* Update the current file position. */
       *f_pos += count;

       /* Print a message. */
       pr_info("Number Of bytes successfully read : %zu\n",count);

       pr_info("Updated file position: %lld\n",*f_pos);

       /* return the number of bytes successfully copied. */
       return count;
}
/*    write  */
ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
	/* Write requested for n bytes. */
	pr_info("Write requested fo %zu bytes\n", count);

	pr_info("Current file position: %lld\n", *f_pos);

	/* Adjust the count. */
	if((*f_pos + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *f_pos;
	}

	if(!count)
	{
		pr_err("No space left on the device\n");
		return -ENOMEM;
	}

	/* Copy from user. */
	if(copy_from_user(&cDevBuffer[*f_pos], buff, count))
	{
		return -EFAULT;
	}

	/* Uodate the current file position. */
	*f_pos += count;

	/* Print a message. */
	pr_info("Number of bytes successfully written: %zu\n", count);

	pr_info("Updated file position: %lld\n", *f_pos);

	/* Return the number of bytes successfully written. */
        return count;
}

int pcd_open(struct inode * inode, struct file * file)
{
	pr_info("Open was successfull\n");
        return 0;
}

int pcd_release(struct inode * inode, struct file * filr)
{
	pr_info("Close was successfull\n");
        return 0;
}
module_init(PcdInit);
module_exit(PcdExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("Pseudo Charecter Driver Implementation");
