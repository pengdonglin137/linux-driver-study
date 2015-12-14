#include <linux/init.h>
#include <linux/module.h>

extern void myfunc(void);
extern void myhello(char *);

MODULE_LICENSE("GPL");


static int dump_stack_init(void)
{
	myfunc();
	myhello("dump_stack");
	printk("func: %s enter.", __func__);

	dump_stack();

	return 0;
}

static void dump_stack_exit(void)
{
	printk("GoodBye.");
	dump_stack();
}




module_init(dump_stack_init);
module_exit(dump_stack_exit);
