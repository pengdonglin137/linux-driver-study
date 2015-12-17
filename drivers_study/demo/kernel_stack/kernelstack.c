#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/miscdevice.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

static int ksopen (struct inode *inode, struct file *filp)
{

	char buffer[8*1024];
	int i;

	for (i=0; i<sizeof(buffer); i++)
		buffer[i] = 0x55;

	for (i=0; i<sizeof(buffer); i++) {
		if ((i % 20) == 0)
			printk("\n");
		printk("%x ", buffer[i]);
	}

	printk("%s enter.\n", __func__);
	return 0;
}

static int ksclose (struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations ksfops =
{
	.owner = THIS_MODULE,
	.open = ksopen,
	.release = ksclose,
};

static struct miscdevice ksdev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "kernelstack",
	.fops = &ksfops,
};

static int __init kernelstack_init(void)
{
	int ret = 0;

	printk("%s enter.\n", __func__);
	ret = misc_register(&ksdev);
	if (ret < 0) {
		printk(KERN_WARNING "misc register failed.\n");
	}

	return ret;
}

static void __exit kernelstack_exit(void)
{
	misc_deregister(&ksdev);
	return ;
}

module_init(kernelstack_init);
module_exit(kernelstack_exit);
