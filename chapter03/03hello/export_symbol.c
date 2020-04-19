#include <linux/module.h>
#include <linux/init.h>

int fun_test(int a, int b)
{
	return a + b;
}

static __init int init_hello(void) 
{
	printk("<1> export module \n");
	return 0;
}

static __exit void exit_hello(void)
{
	printk("<1> export module \n");
}

module_init(init_hello);
module_exit(exit_hello);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("devin");
MODULE_DESCRIPTION("a simple export module");
EXPORT_SYMBOL(fun_test);
