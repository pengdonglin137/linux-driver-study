#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static int value = 1;
static char *str = "pengdonglin";

module_param(value, int, S_IRUGO);
module_param(str, charp, S_IRUGO);


extern void myfunc(void);
extern void myhello(char *);

MODULE_LICENSE("GPL");


static int dump_stack_init(void)
{
	myfunc();
	myhello("dump_stack");
	printk("func: %s enter.", __func__);

	dump_stack();

	printk("value: %d\n", value);
	printk("str: %s\n", str);

	return 0;
}

static void dump_stack_exit(void)
{
	printk("GoodBye.");
	dump_stack();
}




module_init(dump_stack_init);
module_exit(dump_stack_exit);
