#include <linux/init.h>
#include <linux/module.h>

static __init int hello_init(void)
{
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
