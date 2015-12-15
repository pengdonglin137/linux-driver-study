#include <linux/init.h>
#include <linux/module.h>

extern void myfunc(void);

MODULE_LICENSE("GPL");


void myhello(char *str)
{
	myfunc();
	printk("%s", str);
}
EXPORT_SYMBOL_GPL(myhello);

static int hello_init(void)
{
	printk("hello world.\n");
	printk("float: %f.\n", 1.35);
	printk("float: %d.\n", 0.01);
	return 0;
}

static void hello_exit(void)
{
	printk("Goodbye.\n");;
}


module_init(hello_init);
module_exit(hello_exit);
