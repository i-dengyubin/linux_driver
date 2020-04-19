# 1 认识Linux模块

## 1.1 一个最简单的Linux内核模块

在chapter01的最后有一个最简单的Linux内核模块，这个最简单的内核模块只包含内核模块加载函数、卸载函数和对GPL许可权限的声明。

lsmod 命令实际上是读取分析/proc/modules文件，

```shell
[root@localhost 01hello]# cat /proc/modules 
hello 12496 0 - Live 0xffffffffc0743000 (OE)
```

加载hello.ko后，内核中将包含/sys/module/hello目录，如下：

```shell
[root@localhost 01hello]# tree /sys/module/hello/ -a
/sys/module/hello/
├── coresize
├── holders
├── initsize
├── initstate
├── notes
│   └── .note.gnu.build-id
├── refcnt
├── rhelversion
├── sections
│   ├── .gnu.linkonce.this_module
│   ├── __mcount_loc
│   ├── .note.gnu.build-id
│   ├── .rodata.str1.1
│   ├── .strtab
│   ├── .symtab
│   └── .text
├── srcversion
├── taint
└── uevent
```

modprobe命令比insmod命令要强大，它在加载某模块时，会同时加载该模块所依赖的其他模块。使用modprobe命令加载的模块若以“modprobe -r filename”的方式卸载，将同时卸载其依赖的模块。

## 1.2 Linux内核模块程序结构

+ 模块加载函数（一般需要）
+ 模块卸载函数（一般需要）
+ 模块许可声明（必须）
  + 大多数情况下，内核模块应遵循GPL兼容许可权。Linux 2.6内核模块最常见的是以MODULE_LICENSE（"Dual BSD/GPL"）语句声明模块采用BSD/GPL双LICENSE。

+ 模块参数（可选）
+ 模块导出符号（可选）
+ 模块作者等信息声明（可选）

（1）模块加载函数

```c
static int __init init_function(void) { /*初始化代码*/} 
module_init(init_function);
```

在Linux2.6内核中，可以使用request(const char* fmt, ...)函数加载内核模块，

```c
request_module(module_name); 
// or
request_module("char-major-%d-%d", MAJOR(dev),MINOR(dev));
```

在Linux中，所有标识为_init的函数在连接的时候都放在.init.text这个区段内。此外，所有的_init函数在区段.initcall.init中还保存了一份函数指针，在初始化时内核会通过这些函数指针调用这些_init函数，并在初始化完成后，释放init区段（包括.init.text和.initcall.init等）

（2）模块卸载函数

```c
static void __exit exit_function(void) {/*释放代码*/}
module_exit(exit_function);
```

通常来说，模块卸载函数要完成与模块加载函数相反的功能，如下所示。

+ 若模块加载函数注册了XXX，则模块卸载函数应该注销XXX。

+ 若模块加载函数动态申请了内存，则模块卸载函数应释放该内存。

+ 若模块加载函数申请了硬件资源（中断、DMA通道、I/O端口和I/O内存等）的占用，则模块卸载函数应释放这些硬件资源。

+ 若模块加载函数开启了硬件，则卸载函数中一般要关闭它。

# 2 模块编程形式2：多模块-模块之间有依赖关系

## 2.1 模块符号导出

使用一个宏

```c
EXPORT_SYMBOL(X)。其中，X是要导出给其他模块使用的变量名或函数名。
```

## 2.2 多模块例子

hello.c

```c
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
```

export_symbol.c

```c
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
```

Makefile

```makefile
obj-m := hello.o export_symbol.o
all:
        make -C /lib/modules/$(shell uname -r)/build M=`pwd` modules 
clean:
        make -C /lib/modules/$(shell uname -r)/build M=`pwd` clean
```

# 3 模块编程形式3：传递参数

## 3.1 模块传递参数关键宏定义

Linux kernel提供了一个简单的框架，其允许驱动程序声明参数，并且用户在系统启动或模块加载时为参数指定相应值。在驱动程序里，参数的用法如同全局变量。使用下面的宏时需要包含头文件<Linux/moduleparam.h>。要传递参数给模块，首先将获取参数值的变量声明为全局变量。然后使用宏module_param()（在头文件Linux/module.h中）。运行时，insmod将给变量赋予命令行的参数，如同./insmod module.ko var=“zhangsan”。为使代码清晰，变量的声明和宏都应该放在模块代码的开始部分。

# 3.2 module_param()

module_param宏是Linux 2.6内核中新增的，该宏被定义在include/linux/moduleparam.h文件中，具体定义如下。

```c
#define module_param(name, type, perm) module_param_named(name, name, type, perm)
```

module_param使用了3个函数：变量名，类型，一个权限掩码。

类型：bool、invbool(颠倒)、charp、int、long、short、uint、ulong、ushort

需要传递多个参数可以使用宏module_param_array()实现，

```c
module_param_array(name,type,num,perm);
```

name ：数组的名字

type ：数组元素的类型

num ：存放数组元素个数的指针，

perm ：通常的权限值

perm参数的作用：module_param字段是一个权限值，perm值控制谁可以存取这些模块参数在sysfs中的表示。如果perm被设为0，就根本没有sysfs项；否则，它出现在/sys/module下面，带有给定的权限。使用S_IRUGO作为参数可以被所有人读取，但是不能改变；S_IRUGO|S_IWUSR允许root来改变参数。权限定义在include/linux/stat.h中，

通过宏MODULE_PARM_DESC()对参数进行说明，如下所示。

```c
static unsigned short size = 1; 
module_param(size, ushort, 0644); 
MODULE_PARM_DESC(size, “The size in inches of the fishing pole connected to this computer.” );
```

例如：

```c
static int arr[MAX_FISH]; 
static int nr_arr; 
module_param_array（arr, int, &nr_arr, 0444）; //最终传递数组元素个数存在nr_arr中
```

## 3.3 例子



