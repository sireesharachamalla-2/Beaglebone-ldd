#include<linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

/* Macro Definition. */
#define NO_OF_DEVICES			(4)
#define RDONLY				(0x01)
#define WRONLY				(0x10)
#define RDWR				(0x11)

#define DEV_MEM_SIZE_PCDEV1		(1024)
#define DEV_MEM_SIZE_PCDEV2             (512)
#define DEV_MEM_SIZE_PCDEV3             (1024)
#define DEV_MEM_SIZE_PCDEV4             (512)
#define DEVICE_NAME			("pcd_devices")

loff_t pcd_llseek(struct file * filp, loff_t off, int whence);
ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos);
ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos);
int pcd_open(struct inode * inode, struct file * file);
int pcd_release(struct inode * inode, struct file * filr);
int check_permission(int dev_perm, int acc_mode);

/* Global Variable Declaration. */
char cDevBuffer_pcDev1[DEV_MEM_SIZE_PCDEV1];		/*< Pseudo Device, Read and Write Operations Will Be carried out here. */
char cDevBuffer_pcDev2[DEV_MEM_SIZE_PCDEV2];            /*< Pseudo Device, Read and Write Operations Will Be carried out here. */
char cDevBuffer_pcDev3[DEV_MEM_SIZE_PCDEV3];            /*< Pseudo Device, Read and Write Operations Will Be carried out here. */
char cDevBuffer_pcDev4[DEV_MEM_SIZE_PCDEV4];            /*< Pseudo Device, Read and Write Operations Will Be carried out here. */

/* Device Private data structure. */
struct pcdev_private_data
{
	char* buffer;
	unsigned size;
	const char* serial_number;
	int perm;
	struct cdev cdev;
};

/* Driver Private data structure. */
struct pcdrv_private_data
{
	int total_devices;
	dev_t uiDeviceNumber;                  				 /*< This Holds The Device Number.                                      */
	struct class* class_pcd;              				 /*< Class Variable.                                                    */
	struct device* device_pcd;              			 /*< Device variable.                                                   */
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data = 
{
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0] = {
			.buffer	       = cDevBuffer_pcDev1,
			.size  	       = DEV_MEM_SIZE_PCDEV1,
			.serial_number = "PCDDEV1XYZ123",
			.perm	       = RDONLY	/* Read Only. */
		},

	       [1] = {
                        .buffer        = cDevBuffer_pcDev2,
                        .size          = DEV_MEM_SIZE_PCDEV2,
                        .serial_number = "PCDDEV2XYZ123",
                        .perm          = WRONLY /* Write Only. */
                },

	      [2] = {
                        .buffer        = cDevBuffer_pcDev3,
                        .size          = DEV_MEM_SIZE_PCDEV3,
                        .serial_number = "PCDDEV3XYZ123",
                        .perm          = RDWR /* Read Write. */
                },

  	      [2] = {
                        .buffer        = cDevBuffer_pcDev4,
                        .size          = DEV_MEM_SIZE_PCDEV4,
                        .serial_number = "PCDDEV4XYZ123",
                        .perm          = RDWR /* Read Write. */
                }
	}
};

/*< File operations for the driver. */
struct file_operations tPcdFops = {
	.open   = pcd_open,
	.release  = pcd_release,
	.write  = pcd_write,
	.read   = pcd_read,
	.llseek = pcd_llseek,
	.owner  = THIS_MODULE
};

/*    init function  */
static int __init PcdInit(void)
{
	int iErrRet;
	int i;

	/* 1. Dynamically Allocate Device Number. */
	iErrRet = alloc_chrdev_region(&pcdrv_data.uiDeviceNumber, 0, NO_OF_DEVICES, DEVICE_NAME);

	/* Check if error exits. */
	if(iErrRet < 0)
	{
		goto out;
	}
        /* 4. Create device class under /sys/class/. */
       pcdrv_data.class_pcd = class_create( "pcd_class");

        /* Check for valid pointer. */
        if(IS_ERR(pcdrv_data.class_pcd))
        {
                /* Class creation failed. */
                iErrRet = PTR_ERR(pcdrv_data.class_pcd);
                goto unreg_chardev;
        }
	/* Print the Device Number. */
	for(i = 0; i < NO_OF_DEVICES; ++i)
	{
		pr_info("Device Number: Major : %d, Minor: %d\n", MAJOR(pcdrv_data.uiDeviceNumber + i),MINOR(pcdrv_data.uiDeviceNumber + i));

		/* 2. Initialize the cdev structure with the file operations. */
       		 cdev_init(&pcdrv_data.pcdev_data[i].cdev, &tPcdFops);
       	        /* 3. Register a device (cdev structure) with VFS. */
       		 pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
       		 iErrRet = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.uiDeviceNumber + i, 1);

       		/* Check if error exits. */
       		 if(iErrRet < 0)
       		 {
                	goto cdev_del;
       		 }
	        /* 5. Populate the sysfs with device information (device creation). */
        	pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.uiDeviceNumber + i, NULL, "pcdev-%d",i);
	        /* Check for valid pointer. */
        	 if(IS_ERR(pcdrv_data.device_pcd))
       		 {
                	/* Device creation failed. */
               		 iErrRet = PTR_ERR(pcdrv_data.device_pcd);

               		 goto class_del;
       		 }
	}

	pr_info("Module init was successful\n");

	return 0;

	cdev_del:

        class_del:
		for(; i >= 0; i--)
		{
			device_destroy(pcdrv_data.class_pcd, pcdrv_data.uiDeviceNumber + i);
			cdev_del(&pcdrv_data.pcdev_data[i].cdev);
		}
                class_destroy(pcdrv_data.class_pcd);

	unreg_chardev:
		unregister_chrdev_region(pcdrv_data.uiDeviceNumber, NO_OF_DEVICES);

	out:
		return iErrRet;
	
}
/*   exit function  */
static void __exit PcdExit(void)
{
	int i;

	for(i = 0; i < NO_OF_DEVICES; i++)
        {
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.uiDeviceNumber + i);
                cdev_del(&pcdrv_data.pcdev_data[i].cdev);
        }
	
	class_destroy(pcdrv_data.class_pcd);

	unregister_chrdev_region(pcdrv_data.uiDeviceNumber, NO_OF_DEVICES);

	/* Exit Message. */
	pr_info("Module Unloaded\n");
}
/*    lseek function   */
loff_t pcd_llseek(struct file * filp, loff_t off, int whence)
{
	/* Variable Initialization. */
	loff_t temp = 0;
        struct pcdev_private_data* pcdev_data = (struct pcdev_private_data*)filp->private_data;
        int max_size = pcdev_data->size;


	/* Print Entry Message. */
	pr_info("lseek requested...\n");
	pr_info("Current file position : %lld\n",filp->f_pos);

	/* Switch according to whence value. */
	switch(whence)
	{
		/* The file offset is set to offset (off) bytes. */
		case SEEK_SET:
			if((off > max_size) || (off < 0))
			{
				return -EINVAL;
			}

			filp->f_pos = off;
			break;

		/* The file offset is set to its current location + off bytes. */
		case SEEK_CUR:
			temp = filp->f_pos + off;
			if( (temp > max_size) || (temp < 0 ))
			{
				return -EINVAL;
			}	
			filp->f_pos += off;
			break;

		/* Offset is set to the size of file + off bytes. */
		case SEEK_END:
			temp  = max_size + off;
			if((temp > max_size) || (temp < 0))
			{
				return -EINVAL;
			}
			filp->f_pos = max_size + off;
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
/*          read function         */
ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
	/* Variable Initialization. */
	struct pcdev_private_data* pcdev_data = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcdev_data->size;

	/* Read requested of n bytes. */
	pr_info("Read Requested for %zu bu=ytes\n",count);
	
	pr_info("Current file position : %lld\n", *f_pos);

        /* Adjust the count. */
        if((*f_pos + count) > max_size)
        {
                count = max_size - *f_pos;
        }

        /* Copy to user. */
       if(copy_to_user(buff, pcdev_data->buffer + (*f_pos), count))
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
/*       write function   */
ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
	/* Variable Initialization. */
	struct pcdev_private_data* pcdev_data = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcdev_data->size;	

	/* Write requested for n bytes. */
	pr_info("Write requested fo %zu bytes\n", count);

	pr_info("Current file position: %lld\n", *f_pos);

	/* Adjust the count. */
	if((*f_pos + count) > max_size)
	{
		count = max_size - *f_pos;
	}

	if(!count)
	{
		pr_err("No space left on the device\n");
		return -ENOMEM;
	}

	/* Copy from user. */
	if(copy_from_user(pcdev_data->buffer + (*f_pos), buff, count))
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
/*    open function  */
int pcd_open(struct inode * inode, struct file * file)
{
	/* Variable Initialization. */
	int iRet;
	int iMinor_No;
	struct pcdev_private_data* pcdev_data;
	
	/* Find out on which device file open was attempted by the user space. */
	iMinor_No = MINOR(inode->i_rdev);
	pr_info("Minor Access : %d\n", iMinor_No);
	
	/* Get Devices private data structure. */
	pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

	/* To supply device private data to other methods of the driver. */
	file->private_data = pcdev_data;

	/* Check Permission. */
	iRet = check_permission(pcdev_data->perm, file->f_mode);

	/* Print Message. */
	(!iRet) ? pr_info("Open Was Successfull\n") : pr_info("Open Was Unsuccessfull\n");

        return iRet;
}
/*  release function  */
int pcd_release(struct inode * inode, struct file * filr)
{
	pr_info("Close was successfull\n");
        return 0;
}

int check_permission(int dev_perm, int acc_mode)
{
    /* If device allows both, always OK */
    if (dev_perm == RDWR)
        return 0;

    /* If device is read-only, deny if caller requested write access */
    if ((dev_perm & RDONLY) && !(dev_perm & WRONLY)) {
        if (acc_mode & FMODE_WRITE)    /* caller tried to open for write */
            return -EPERM;
        else
            return 0;
    }

    /* If device is write-only, deny if caller requested read access */
    if ((dev_perm & WRONLY) && !(dev_perm & RDONLY)) {
        if (acc_mode & FMODE_READ)     /* caller tried to open for read */
            return -EPERM;
        else
            return 0;
    }
    return -EPERM;
}
module_init(PcdInit);
module_exit(PcdExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sireesha");
MODULE_DESCRIPTION("Module For Pseudo Charecter Driver Imementation");
