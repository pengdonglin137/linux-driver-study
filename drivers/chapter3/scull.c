#include <linux/init.h>
#include <linux/module.h>

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/device.h>

#include <linux/kernel.h>

#include "scull.h"

MODULE_LICENSE("GPL");


static dev_t devno;
static dev_t scull_major;
static scull_dev_t scull_dev;


static void scull_trim(scull_dev_t *dev)
{
	scull_qset_t *dpst, *next;
	int qset = dev->qset;

	down_interruptible(&dev->sema);
	for (dpst = dev->data; dpst; dpst = next) {
		int n = 0;
		if (dpst->data) {
			while (n < qset) {
				if (dpst->data[n])
					kfree(dpst->data[n]);
				n++;
			}
			kfree(dpst->data);
			dpst->data = NULL;
		}
		next = dpst->next;
		kfree(dpst);
	}
	up(&dev->sema);
}

static int scull_open (struct inode *inode, struct file *filp)
{
	scull_dev_t *dev;

	dev = container_of(inode, scull_dev_t, cdev);
	filp->private_data = dev;

	if ((filp->flags & O_ACCMODE) == O_WDONLY)
		scull_trim(dev);

	return 0;
}

static int scull_close (struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t scull_read (struct file *filp, char __user *buf, size_t num, loff_t *f_pos)
{
	scull_dev_t *dev = (scull_dev_t *)filp->private_data;
	scull_qset_t *dpst, *next;
	int qset = dev->qset;
	int quannum = dev->quannum;
	int item = qset * quannum;

	if (down_interruptible(&dev->sema))
		return -ERESTARTSYS;

	if (num > dev->size) {
		printk(KERN_WARN "read too large\n");
		goto out;
	}




out:
	up(&dev->sema);
	return 0;
}

static ssize_t scull_write (struct file *filp, const char __user *buf, size_t num, loff_t *f_pos)
{
	return 0;
}

static struct file_operations scull_ops =
{
	.owner = THIS_MODULE,
	.open  = scull_open,
	.release = scull_close,
	.read = scull_read,
	.write = scull_write,
};

static int __init scull_init(void)
{
	int ret;

	printk("%s enter.\n", __func__);
	ret = alloc_chrdev_region(&devno, 0, 1, "scull");
	if (ret < 0) {
		printk(KERN_ERR "alloc_chrdev_region error: %d\n", ret);
		return ret;
	}

	scull_major = MAJOR(devno);

	memset(&scull_dev, 0, sizeof(scull_dev_t));
	cdev_init(&scull_dev.cdev, &scull_ops);
	scull_dev.cdev.owner = THIS_MODULE;
	ret = cdev_add(&scull_dev.cdev, devno, 1);
	if (ret < 0) {
		printk(KERN_ERR "cdev_add error: %d\n", ret);
	}

	scull_dev.class = class_create(THIS_MODULE, "scull");
	if (IS_ERR(scull_dev.class)) {
		printk(KERN_ERR "class_create failed for scull class\n");
		ret = PTR_ERR(scull_dev.class);
		goto out2;
	}

	scull_dev.device = device_create(scull_dev.class, NULL, devno, NULL, "scull0");
	if (IS_ERR(scull_dev.device)) {
		printk(KERN_ERR "scull device create failed.\n");
		goto out3;
	}

	sema_init(&scull_dev.sem, 1);

	scull_dev.size = 0;
	scull_dev.qset = QSET;
	scull_dev.quannum = QUANNUM;

	return 0;
out3:
	class_destroy(scull_dev.class);
out2:
	cdev_del(&scull_dev.cdev);
out1:
	unregister_chrdev_region(devno, 1);
	return ret;
}

static void __exit scull_exit(void)
{
	cdev_del(&scull_dev.cdev);
	unregister_chrdev_region(devno, 1);

	device_destroy(scull_dev.class, devno);
	class_destroy(scull_dev.class);
}

module_init(scull_init);
module_exit(scull_exit);

