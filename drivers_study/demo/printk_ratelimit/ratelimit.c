#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/miscdevice.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

static int rlopen (struct inode *inode, struct file *filp)
{
	while (1) {
		if (printk_ratelimit())
			printk("%s enter.\n", __func__);
		msleep(100);
	}

	return 0;
}

static int rlclose (struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations rlfops =
{
	.owner = THIS_MODULE,
	.open = rlopen,
	.release = rlclose,
};

static struct miscdevice rldev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rate_limit",
	.fops = &rlfops,
};

static int __init ratelimit_init(void)
{
	int ret = 0;

	printk("%s enter.\n", __func__);
	ret = misc_register(&rldev);
	if (ret < 0) {
		printk(KERN_WARNING "misc register failed.\n");
	}

	return ret;
}

static void __exit ratelimit_exit(void)
{
	misc_deregister(&rldev);
	return ;
}

module_init(ratelimit_init);
module_exit(ratelimit_exit);
