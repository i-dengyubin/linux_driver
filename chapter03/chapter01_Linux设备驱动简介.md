# 1 Linux设备驱动概述

设备驱动是一种联系计算机和通信设备的特殊程序，可以说相当于硬件的接口，操作系统通过这个接口，访问硬件。驱动程序被称为”硬件的灵魂“、”硬件的主宰“。

# 2 Linux内核结构

## 2.1 Linux内核空间划分

Linux 简化了分段机制，使得虚拟地址与线性地址总是一致。因此，Linux的虚拟地址空间也为0～4GB。Linux内核将这4GB的空间分为两部分。将最高的1GB（虚拟地址 0xC0000000～0xFFFFFFFF），供内核使用，称为“内核空间”；将较低的3GB（虚拟地址0x00000000～0xBFFFFFFF），供各个进程使用，称为“用户空间”。因为每个进程可以通过系统调用进入内核，因此Linux内核由系统内的所有进程共享。于是，从具体进程的角度来看，每个进程可以拥有4GB的虚拟空间。

## 2.2 Linux用户空间

用户空间模式的驱动一般通过系统调用来完成对硬件的访问，用户态每个应用程序空间是虚拟的、相对独立的，内核态中却不是独立的，所以编程要非常小心等。

## 2.3 Linux内核空间

内核空间占据了每个虚拟空间中的最高1GB，但映射到物理内存却总是从最低地址（0x00000000）开始。对内核空间来说，其地址映射是很简单的线性映射，0xC0000000就是物理地址与线性地址之间的位移量，在Linux代码中就叫作PAGE_OFFSET。

# 3 Linux设备驱动特点

## 3.1 设备驱动分类

+ 字符设备
+ 块设备
+ 网络设备

字符设备通常指像普通文件或字节流一样，以**字节为单位顺序读写的设备**，如LED、虚拟控制台等，字符设备可以通过设备文件节点访问，它与普通文件之间的区别在于普通文件可以被随机访问（可以前后移动访问指针），而大多数字符设备只能提供顺序访问，因为对它们的访问不会被系统所缓存。但也有例外，如帧缓存（framebuffer）是一个可以被随机访问的字符设备。

块设备通常指一些需要以块为单位随机读写的设备，如IDE硬盘、SCSI硬盘、光驱等。块设备也是通过文件节点来访问的，它不仅可以提供随机访问，而且可以容纳文件系统（如硬盘、闪存等）。

## 3.2 设备号

每个字符设备和块设备都必须有**主、次设备号**，主设备号相同的设备是**同类设备**（使用同一个驱动程序）。这些设备中，有些设备是对实际存在的物理硬件的抽象，而有些设备则是内核自身提供的功能（不依赖于特定的物理硬件，又称为“虚拟设备”）。每个设备在 /dev 目录下都有一个对应的文件（节点）。可以通过cat/proc/devices 命令查看当前已经加载的设备驱动程序的主设备号。内核能够识别的所有设备都记录在原码树下的documentation/devices.txt 文件中。在 /dev 目录下，除了字符设备和块设备节点之外，通常还会存在fifo管道、socket、软/硬连接、目录，这些东西没有主/次设备号。

```shell
[devin@localhost ~]$ ls -l /dev/full
crw-rw-rw-. 1 root root 1, 7 Apr 15 20:38 /dev/full
```

其中（1，7）分别是系统用来表示full设备的重要序号，第一个表示设备使用的硬件驱动程序在系统中的序号，第二个是从设备号。

## 3.3 模块和设备驱动关系

模块不一定是驱动，但是驱动都是模块。

在Linux2.6 之前，模块扩展名是.o，之后是.ko。

## 3.4 内核模块的主要相关命令

+ lsmod : 查看当前内核安装的有哪些模块

+ modinfo : 查询具体模块信息

```shell
[devin@localhost ~]$ modinfo dm_log
filename:       /lib/modules/3.10.0-1062.el7.x86_64/kernel/drivers/md/dm-log.ko.xz
license:        GPL
author:         Joe Thornber, Heinz Mauelshagen <dm-devel@redhat.com>
description:    device-mapper dirty region log
retpoline:      Y
rhelversion:    7.7
srcversion:     2520F689AFECC997AB36ADE
depends:        dm-mod
intree:         Y
vermagic:       3.10.0-1062.el7.x86_64 SMP mod_unload modversions 
signer:         CentOS Linux kernel signing key
sig_key:        51:08:4E:41:88:03:02:BE:5C:B0:74:AC:0D:A3:FE:10:23:3B:7F:1C
sig_hashalgo:   sha256
```

Filename：模块文件存绝对路径。

license：模块遵循的许可协议。

author：模块编写者。

description：模块的一些描述信息，可以自行编写。

srcversion：由内核编译时自动生成，用户没有办法修改。

depends：依赖的模块名列表。

vermagic：模块版本号，由内核源码决定，用户无法修改。

+ rmmod : 卸载模块
+ insmod 和 modprobe : 加载模块
  + insmod 不会自动解决依赖关系，modprobe可以根据模块间的依存关系，以及/etc/modules.conf文件中的内容自动加载其他有依赖关系的模块

## 3.5 内核驱动信息打印-printk

### 3.5.1 日志级别

```c
#define	KERN_EMERG	"<0>"	/* system is unusable			*/
#define	KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define	KERN_CRIT	"<2>"	/* critical conditions			*/
#define	KERN_ERR	"<3>"	/* error conditions			*/
#define	KERN_WARNING	"<4>"	/* warning conditions			*/
#define	KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define	KERN_INFO	"<6>"	/* informational			*/
#define	KERN_DEBUG	"<7>"	/* debug-level messages			*/
```

### 3.5.2 控制台级别

```c
#define MINIMUM_CONSOLE_LOGLEVEL　 1　 　/*可以使用的最小日志级别*/
#define DEFAULT_CONSOLE_LOGLEVEL 　7 /*比KERN_DEBUG 更重要的消息都被打印*/
#define DEFAULT_MESSAGE_LOGLEVEL 4 /* KERN_WARNING */
 
int console_printk[4] = {
DEFAULT_CONSOLE_LOGLEVEL,/*控制台日志级别，优先级高于该值的消息将在控制台显示*/
/*默认消息日志级别，printk没定义优先级时，打印这个优先级以上的消息*/
DEFAULT_MESSAGE_LOGLEVEL,
/*最小控制台日志级别，控制台日志级别可被设置的最小值（最高优先级）*/
MINIMUM_CONSOLE_LOGLEVEL,
DEFAULT_CONSOLE_LOGLEVEL,/* 默认的控制台日志级别*/
};
```

### 3.5.3 printk函数的使用

```c
printk("KERN_EMERG "KERN_EMERG\n");
```

当 printk() 中的消息日志级别小于当前控制台日志级别（console_printk[0]）时，printk 的信息就会在控制台上显示，echo x x x x > /proc/sys/kernel/printk，进行调节。

但无论当前控制台日志级别是何值，即使没有在控制台打印出来，可以通过两种方法查看日志：

+ 使用dmesg命令打印
+ 通过cat /proc/kmsg来打印

另外如果配置好并运行了 syslogd 或 klogd，没有在控制台上显示的 printk 的信息也会追加到/var/log/messages.log 中。

### 3.5.4 例子

hello.c

```c
#include <linux/init.h>
#include <linux/module.h>

static int hello_init(void)
{
        printk("module init......\n");
        return 0;
}

static void hello_exit(void)
{
        printk("module exit.......\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
```

makefile

```makefile
obj-m := hello.o
all:
        make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) modules 
clean:
        make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) clean
```

make时出现如下错误：

```c
[devin@localhost 01hello]$ make
make -C /lib/modules/3.10.0-1062.el7.x86_64/build SUBDIRS=/home/devin/Documents/07_linux_driver/chapter03/01hello modules 
make: *** /lib/modules/3.10.0-1062.el7.x86_64/build: No such file or directory.  Stop.
make: *** [all] Error 2
```

解决方式：

```c
[root@localhost 01hello]# UNAME=$(uname -r)
[root@localhost 01hello]# yum install gcc kernel-devel-${UNAME%.*}
```

之后执行make成功。

