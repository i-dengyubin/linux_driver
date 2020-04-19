#include <linux/init.h>
#include <linux/module.h>

extern int fun_test(int a, int b);
static __init int hello_init(void)
{
	fun_test(3, 4);
	printk(" <1> module init......\n");
	return 0;
}

static __exit void hello_exit(void)
{
	printk("module exit.......\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("devin");
MODULE_DESCRIPTION("A simple module");
