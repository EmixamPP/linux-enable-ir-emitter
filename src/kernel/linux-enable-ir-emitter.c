#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/videodev2.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define DEVICE_NAME "video4"
#define TARGET_DEVICE "/dev/video2"

static int major;
static struct class *video4_class;
static struct cdev video4_cdev;

static int video4_open(struct inode *inode, struct file *file)
{
    struct file *target_file;

    target_file = filp_open(TARGET_DEVICE, O_RDWR, 0);
    if (IS_ERR(target_file)) {
        pr_err("Failed to open target device %s\n", TARGET_DEVICE);
        return PTR_ERR(target_file);
    }

    file->private_data = target_file;
    return 0;
}

static int video4_release(struct inode *inode, struct file *file)
{
    struct file *target_file = file->private_data;
    filp_close(target_file, NULL);
    return 0;
}

static long video4_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct file *target_file = file->private_data;
    return vfs_ioctl(target_file, cmd, arg);
}


static int video4_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct file *target_file = file->private_data;

    if (!target_file->f_op->mmap) {
        pr_err("Target device does not support mmap\n");
        return -ENODEV;
    }

    return target_file->f_op->mmap(target_file, vma);
}

static struct file_operations video4_fops = {
    .owner = THIS_MODULE,
    .open = video4_open,
    .release = video4_release,
    .unlocked_ioctl = video4_ioctl,
    .mmap = video4_mmap,
};

static int __init video4_init(void)
{
    dev_t dev;
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }

    major = MAJOR(dev);

    cdev_init(&video4_cdev, &video4_fops);
    video4_cdev.owner = THIS_MODULE;

    ret = cdev_add(&video4_cdev, dev, 1);
    if (ret) {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    video4_class = class_create(DEVICE_NAME);
    if (IS_ERR(video4_class)) {
        pr_err("Failed to create class\n");
        cdev_del(&video4_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(video4_class);
    }

    device_create(video4_class, NULL, dev, NULL, DEVICE_NAME);
    pr_info("video4 proxy module loaded\n");
    return 0;
}

static void __exit video4_exit(void)
{
    dev_t dev = MKDEV(major, 0);
    device_destroy(video4_class, dev);
    class_destroy(video4_class);
    cdev_del(&video4_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("video4 proxy module unloaded\n");
}

module_init(video4_init);
module_exit(video4_exit);

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Dirksen Maxime");
MODULE_DESCRIPTION("TODO");