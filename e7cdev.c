#include "e7cdev.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/uaccess.h>

#include "driver.h"

/* supporting functions by our character device*/
static int eduv7cdev_open(struct inode *inode, struct file *file) {
        printk("eduv7cdev: Device open\n");
        struct pci_dev *pcidev = NULL;
        while (pcidev == NULL) pcidev = pci_get_device(0x1234, 0x11e8, pcidev);
        _eduv7_pci_dev = pcidev;
        return 0;
}

static int eduv7cdev_release(struct inode *inode, struct file *file) {
        printk("eduv7cdev: Device close\n");
        return 0;
}

static ssize_t eduv7cdev_read(struct file *file, char __user *buf, size_t count,
                              loff_t *offset) {
        int error_bytes = 0;
#ifdef _USE_DMA
        size_t maxdatalen = 64;
        u8 *databuf;
        databuf = (u8 *)kzalloc(sizeof(u8) * maxdatalen, GFP_KERNEL);
        eduv7_driver_dma_read(_eduv7_pci_dev, databuf, count);
        size_t datalen = strlen(databuf);
        if (count > datalen) count = datalen;
        if (copy_to_user(buf, databuf, count)) return -EFAULT;
#else
        int *databuf;
        databuf = (int *)kzalloc(sizeof(int), GFP_KERNEL);
        *databuf = eduv7_driver_read(_eduv7_pci_dev);
        printk("Data to the user with count:%d 0x%08x\n",count, ~(*(int*)databuf));
        // if (count > 4) count = datalen;
        if (error_bytes = copy_to_user(buf, databuf, count)){
                printk(KERN_ERR "cdev read copy_to_user coould not transfer %d bytes",error_bytes);
                kfree(databuf);
                return -EFAULT;
        }
        // count = 0;
#endif
        *offset += count;

        kfree(databuf);
        return count;
}

static ssize_t eduv7cdev_write(struct file *file, const char __user *buf,
                               size_t count, loff_t *offset) {
        size_t maxdatalen = 64, ncopied;
#ifdef _USE_DMA

        u8 *databuf;
        databuf = (u8 *)kzalloc(sizeof(u8) * maxdatalen, GFP_KERNEL);

        if (count < maxdatalen) maxdatalen = count;
        ncopied = copy_from_user(databuf, buf, maxdatalen);
        eduv7_driver_dma_write(_eduv7_pci_dev, databuf, count);
        if (!ncopied) {
                printk("Copied %zd bytes from the user\n", maxdatalen);
                databuf[maxdatalen] = 0;

        } else {
                printk("Could't copy %zd byte from the user\n", ncopied);
        }
#else
        u8 *databuf;
        databuf = (u8 *)kzalloc(sizeof(int), GFP_KERNEL);

        ncopied = copy_from_user(databuf, buf, sizeof(int));
        eduv7_driver_write(_eduv7_pci_dev, *(int*)databuf);
        printk("Data from the user: 0x%08x\n", *(int*)databuf);
#endif

        kfree(databuf);
        *offset += count;
        return count;
}

static int eduv7cdev_uevent(struct device *dev, struct kobj_uevent_env *env) {
        add_uevent_var(env, "DEVMODE=%#o", 0666);
        return 0;
}

/*our file operations supported by our driver*/
static const struct file_operations eduv7cdev_fops = {
    .owner = THIS_MODULE,
    .open = eduv7cdev_open,
    .release = eduv7cdev_release,
    .read = eduv7cdev_read,
    .write = eduv7cdev_write};

struct char_device_data {
        struct cdev cdev;
};
/* our device Major data number, used for identifying our new device*/
static int dev_major = 100;
/*sysfs class structure*/
static struct class *eduv7cdev_class = NULL;
/*data about our character device*/
static struct char_device_data eduv7cdev_data;

void eduv7cdev_init(void) {
        int err;
        dev_t dev;

        err = alloc_chrdev_region(&dev, 0, 1, "eduv7");
        dev_major = MAJOR(dev);

        eduv7cdev_class = class_create(THIS_MODULE, "eduv7");
        eduv7cdev_class->dev_uevent = eduv7cdev_uevent;
        cdev_init(&eduv7cdev_data.cdev, &eduv7cdev_fops);
        eduv7cdev_data.cdev.owner = THIS_MODULE;

        cdev_add(&eduv7cdev_data.cdev, MKDEV(dev_major, 0), 1);
        device_create(eduv7cdev_class, NULL, MKDEV(dev_major, 0), NULL,
                      "eduv7-%d", 0);
}

void eduv7cdev_destroy(void) {
        device_destroy(eduv7cdev_class, MKDEV(dev_major, 1));

        class_unregister(eduv7cdev_class);
        class_destroy(eduv7cdev_class);

        unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}
