#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");


static int hello2_init(void)
{
	printk("hello2 world.\n");
	return 0;
}

static void hello2_exit(void)
{
	printk("Goodbye 2\n");
}

void myfunc(void)
{
	printk("**************\n");
}

EXPORT_SYMBOL_GPL(myfunc);



module_init(hello2_init);
module_exit(hello2_exit);

