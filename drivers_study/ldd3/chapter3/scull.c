#include <linux/init.h>
#include <linux/module.h>

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/device.h>

#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <asm/div64.h>

#include "scull.h"

MODULE_LICENSE("GPL");

static dev_t devno;
static dev_t scull_major;
static scull_dev_t scull_dev;

static void scull_trim(scull_dev_t *dev)
{
	scull_qset_t *dpst, *next;
	int qset = dev->qset;

	for (dpst = dev->data; dpst; dpst = next) {
		int n = 0;
		if (dpst->data) {
			while (n < qset) {
				if (dpst->data[n])
					devm_kfree(dev->device, dpst->data[n]);
				n++;
			}
			devm_kfree(dev->device, dpst->data);
			dpst->data = NULL;
		}
		next = dpst->next;
		devm_kfree(dev->device, dpst);
	}
}

static int scull_open (struct inode *inode, struct file *filp)
{
	scull_dev_t *dev;

	printk("%s enter.\n", __func__);
	dev = container_of(inode->i_cdev, scull_dev_t, cdev);
	filp->private_data = dev;

	if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
		scull_trim(dev);

	return 0;
}

static int scull_close (struct inode *inode, struct file *filp)
{
	printk("%s enter.\n", __func__);

	filp->private_data = NULL;
	return 0;
}

static scull_qset_t * scull_follow(scull_dev_t *dev, int item)
{
	scull_qset_t *dpst = dev->data;

	if (!dpst) {
		dpst = (scull_qset_t *)devm_kzalloc(dev->device, sizeof(scull_qset_t), GFP_KERNEL);
		if (!dpst) {
			printk(KERN_ERR "devm_kzalloc scull_qset_t  error.\n");
			return NULL;
		}
		dev->data = dpst; /* important  */
	}

	while (item--) {
		if (!dpst->next) {
			dpst->next = (scull_qset_t *)devm_kzalloc(dev->device, sizeof(scull_qset_t), GFP_KERNEL);
			if (!dpst->next) {
				printk(KERN_ERR "devm_kzalloc scull_qset_t  error.\n");
				return NULL;
			}
		}
		dpst = dpst->next;
	}

	return dpst;
}

static ssize_t scull_read (struct file *filp, char __user *buf, size_t num, loff_t *f_pos)
{
	scull_dev_t *dev = (scull_dev_t *)filp->private_data;
	scull_qset_t *dpst;
	int qset = dev->qset;
	int quannum = dev->quannum;
	int itemsize = qset * quannum;
	int item, s_pos, q_pos;
	int ret = 0;

	printk("%s enter.\n", __func__);
	if (down_interruptible(&dev->sema))
		return -ERESTARTSYS;

	if (*f_pos >= dev->size) {
		printk(KERN_WARNING "No more data.\n");
		goto out;
	}

	if (*f_pos + num > dev->size)
		num = dev->size - *f_pos;

	if (num > dev->size) {
		printk(KERN_WARNING "read too large\n");
		goto out;
	}

#if 0
	item = *f_pos / itemsize;
	rest = *f_pos % itemsize;
	s_pos = rest / quannum;
	q_pos = rest % quannum;
#else
	item = *f_pos;
	s_pos = do_div(item, itemsize);
	q_pos = do_div(s_pos, quannum);
#endif

	if (num > (quannum - q_pos))
		num = quannum - q_pos;

	dpst = scull_follow(dev, item);
	if (!dpst || !dpst->data || !dpst->data[s_pos]) {
		printk(KERN_ERR "dpst: 0x%p, dpst->data: 0x%p, dpst->data[%d]: 0x%p\n",
			(void *)dpst, (void *)dpst->data, (uint)s_pos, (void *)dpst->data[s_pos]);
		goto out;
	}

	if (copy_to_user(buf, dpst->data[s_pos]+q_pos, num)) {
		ret = -EFAULT;
		goto out;
	}

	*f_pos += num;
	ret = num;
out:
	up(&dev->sema);
	return ret;
}

static ssize_t scull_write (struct file *filp, const char __user *buf, size_t num, loff_t *f_pos)
{
	scull_dev_t *dev = (scull_dev_t *)filp->private_data;
	scull_qset_t *dpst;
	int qset = dev->qset;
	int quannum = dev->quannum;
	int itemsize = qset * quannum;
	int item, s_pos, q_pos;
	int ret = 0;

	printk("%s enter.\n", __func__);

	if (down_interruptible(&dev->sema)) {
		return -ERESTARTSYS;
	}

#if 0
	item = *f_pos / itemsize;
	rest = *f_pos % itemsize;
	s_pos = rest / quannum;
	q_pos = rest % quannum;
#else
	item = *f_pos;
	s_pos = do_div(item, itemsize);
	q_pos = do_div(s_pos, quannum);
#endif

	if (num > quannum - q_pos)
		num = quannum - q_pos;

	dpst = scull_follow(dev, item);
	if (!dpst) {
		printk(KERN_ERR "scull_follow failed.\n");
		goto out2;
	}

	if (!dpst->data) {
		dpst->data = (void **)devm_kzalloc(dev->device, sizeof(char *)*qset, GFP_KERNEL);
		if (!dpst->data) {
			printk(KERN_ERR "devm_kzalloc qset failed.\n");
			goto out2;
		}
	}

	if (!dpst->data[s_pos]) {
		dpst->data[s_pos] = devm_kzalloc(dev->device, sizeof(char *)*quannum, GFP_KERNEL);
		if (!dpst->data[s_pos]) {
			printk(KERN_ERR "devm_kzalloc quannum failed.\n");
			goto out2;
		}
	}

	if (copy_from_user(dpst->data[s_pos] + q_pos, buf, num)) {
		ret = -EFAULT;
		goto out2;
	}

	*f_pos += num;
	ret = num;

	if (dev->size < *f_pos)
		dev->size = *f_pos;

out2:
	up(&dev->sema);
	return ret;
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
		goto out1;
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

	sema_init(&scull_dev.sema, 1);

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
	scull_trim(&scull_dev);
	cdev_del(&scull_dev.cdev);
	unregister_chrdev_region(devno, 1);

	device_destroy(scull_dev.class, devno);
	class_destroy(scull_dev.class);
}

module_init(scull_init);
module_exit(scull_exit);

