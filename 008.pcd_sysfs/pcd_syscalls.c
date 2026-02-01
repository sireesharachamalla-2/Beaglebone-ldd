#include "pcd_platform_driver_dt_sysfs.h"

int check_permission(int dev_perm, int acc_mode)
{
        if (dev_perm == RDWR)
                return 0;

        if ((dev_perm == 0x10) && (acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE))
                return 0;

        if ((dev_perm == 0x01) && (acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ))
                return 0;

        return -EPERM;
}


int pcdev_open(struct inode *inode, struct file *filp)
{
        struct pcdev_private_data *dev_data;

        dev_data = container_of(inode->i_cdev,
                                struct pcdev_private_data, cdev);
        filp->private_data = dev_data;

        return check_permission(dev_data->pdata.perm, filp->f_mode);
}
int pcdev_release(struct inode *inode, struct file *filp)
{
        return 0;
}

ssize_t pcdev_read(struct file *filp, char __user *buf,
                          size_t count, loff_t *f_pos)
{
        struct pcdev_private_data *dev_data = filp->private_data;

        if (*f_pos >= dev_data->pdata.size)
                return 0;

        if (*f_pos + count > dev_data->pdata.size)
                count = dev_data->pdata.size - *f_pos;

        if (copy_to_user(buf, dev_data->buffer + *f_pos, count))
                return -EFAULT;

        *f_pos += count;
        return count;
}

ssize_t pcdev_write(struct file *filp, const char __user *buf,
                           size_t count, loff_t *f_pos)
{
        struct pcdev_private_data *dev_data = filp->private_data;

        if (*f_pos >= dev_data->pdata.size)
                return -ENOMEM;

        if (*f_pos + count > dev_data->pdata.size)
                count = dev_data->pdata.size - *f_pos;

        if (copy_from_user(dev_data->buffer + *f_pos, buf, count))
                return -EFAULT;

        *f_pos += count;
        return count;
}

loff_t pcdev_lseek(struct file *filp, loff_t offset, int whence)
{
        struct pcdev_private_data *dev_data = filp->private_data;
        loff_t new_pos;

        switch (whence) {
        case SEEK_SET:
                new_pos = offset;
                break;
        case SEEK_CUR:
                new_pos = filp->f_pos + offset;
                break;
        case SEEK_END:
                new_pos = dev_data->pdata.size + offset;
                break;
        default:
                return -EINVAL;
        }

        if (new_pos < 0 || new_pos > dev_data->pdata.size)
                return -EINVAL;

        filp->f_pos = new_pos;
        return new_pos;
}
