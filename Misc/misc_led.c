# include <asm/io.h>
# include <asm/uaccess.h>

# include <linux/fs.h>
# include <linux/mm.h>
# include <linux/init.h>
# include <linux/cdev.h>
# include <linux/slab.h>
# include <linux/types.h>
# include <linux/errno.h>
# include <linux/sched.h>
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/device.h>
# include <linux/miscdevice.h>//包含杂项设备驱动的头文件

/*调试开关*/
//# define PDEBUG 2
# ifdef PDEBUG
# define DPRINTK printk
# else
# define DPRINTK(...)
# endif

/*--灯--*/
static void __iomem *led_base;
#define GPJ2CON (*(volatile unsigned long*)led_base)
#define GPJ2DAT (*(volatile unsigned long*)(led_base+4))

/*--函数声明--*/
void led_exit(void);
static int led_init(void);
long led_ioctl(struct file*, unsigned int, unsigned long);

//实现文件关联结构体
static const struct file_operations led_fops={
	.owner = THIS_MODULE,
	.unlocked_ioctl= led_ioctl,
};

//实现次设备结构体
struct miscdevice led_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "led_misc",
	.fops = &led_fops,
};


//关联模块初始函数
module_init(led_init);
//关联模块卸载函数
module_exit(led_exit);
//写入模块协议
MODULE_LICENSE("GPL");
//写入模块作者
MODULE_AUTHOR("XYQ");
//设置模块版本号
MODULE_VERSION("XXX");

//Init
static int led_init()
{
	DPRINTK("\t\t###Initial_Led###\n");
	misc_register(&led_misc);//动态分配此设备号
	led_base = ioremap(0xE0200280,0x0C);//映射起始地址,映射的空间的大小
	GPJ2CON &= ~(0xFFFF);//清引脚
	GPJ2CON |= 0x1111;//配引脚
	GPJ2DAT |= 0x0F;//设状态
	DPRINTK("\t\t   ---over---\n");//提示结束初始化函数
	return 0;
}

//Exit
void led_exit()
{
	DPRINTK("\t\t^v^Initial_Led^V^\n");
	iounmap(led_base);//解除地址映射
	misc_deregister(&led_misc);//传入此设备结构体注销该杂项设备驱动
	DPRINTK("\t\t    ---bye---\n");
	return;
}

//ioctl                            /* cmd:0灭 1亮  arg:对第几盏灯操作*/
long led_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	DPRINTK("\t\t***Initial_Led***\n");
	//DPRINTK("cmd=%d arg=%d\n",cmd,(int)arg);
	if(arg<1||arg>4){
		DPRINTK("arg is undefine\n");
		return -EINVAL;
	}
	if(cmd==0)
		GPJ2DAT |= (0x01<<(arg-1));
	else if(cmd==1)
		GPJ2DAT &= ~(0x01<<(arg-1));
	else
		DPRINTK("cmd is undefine\n");
	DPRINTK("\t\t    ---ctrl---\n");
	return 0;
}

//
