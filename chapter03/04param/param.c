#include <linux/init.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>

#define MAX_SIZE 10
static int age = 100;
static char* name = "zhangsan";
static int nr = 0;
static int int_array[MAX_SIZE] = {1,2,3,4,5,6,7,8,9,0};
module_param(age, int, 0644);
MODULE_PARM_DESC(age,"An integer variable");
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "A string variable");
module_param_array(int_array, int , &nr, 0644);
MODULE_PARM_DESC(int_array, "An integer array");

static __init int hello_init(void)
{
	int arrnr = 0;
	int i = 0;
 	printk(KERN_EMERG "name:%s\n",name); 
	printk(KERN_EMERG "age:%d\n",age);
	arrnr = nr > 0 ? nr : MAX_SIZE; 
	for(i = 0; i < arrnr; i++)
	{ 
		printk(KERN_EMERG "int_array[%d] = %d\n", i, int_array[i]); 
	} 
	return 0; 
}

static void __exit hello_exit(void) 
{ 
	printk(KERN_EMERG"Module exit!\n");	
}


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("devin");
MODULE_DESCRIPTION("A simple module");
