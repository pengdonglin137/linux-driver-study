#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");



static int __init panic_init(void)
{
	*((int *)0) = 0x12345678;

	return 0;
}

static void __exit panic_exit(void)
{}






module_init(panic_init);
module_exit(panic_exit);
