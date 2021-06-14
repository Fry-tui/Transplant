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

/*键--*/
static void __iomem *myKey_base;
#define GPH2CON (*(volatile unsigned long*)myKey_base)
#define GPH2DAT (*(volatile unsigned long*)(myKey_base+4))

/*--函数声明--*/
void myKey_exit(void);
static int myKey_init(void);
ssize_t myKey_read(struct file*,char __user*,size_t,loff_t*);

//实现文件关联结构体
static const struct file_operations key_fops={
	.owner = THIS_MODULE,
	.read = myKey_read,
};

//实现次设备结构体
struct miscdevice key_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "key_misc",
	.fops = &key_fops,
};


//关联模块初始函数
module_init(myKey_init);
//关联模块卸载函数
module_exit(myKey_exit);
//写入模块协议
MODULE_LICENSE("GPL");
//写入模块作者
MODULE_AUTHOR("XYQ");
//设置模块版本号
MODULE_VERSION("XXX");

//Init
static int myKey_init()
{
	DPRINTK("\t\t###Initial_Led###\n");
	misc_register(&key_misc);//动态分配此设备号
	myKey_base = ioremap(0xE0200C40,0x0c);//映射起始地址,映射的空间的大小
	GPH2CON &= ~(0xffff);//配引脚&设状态	0000 0000 0000 0000全部设置为输入引脚
	DPRINTK("\t\t   ---over---\n");//提示结束初始化函数
	return 0;
}

//Exit
void myKey_exit()
{
	DPRINTK("\t\t^v^Initial_Led^V^\n");
	iounmap(myKey_base);//解除地址映射
	misc_deregister(&key_misc);//传入此设备结构体注销该杂项设备驱动
	DPRINTK("\t\t    ---bye---\n");
	return;
}

//ioctl 
ssize_t myKey_read(struct file *filep,char __user *buf,size_t size,loff_t *ppos)
{
	uint i=0;
	char str[5];
	uint count = size;
	
	DPRINTK("\t\t###Read Key###\n");
	i=0;
	for(i=0;i<4;i++){
		if(!(GPH2DAT>>i&0x01))
			str[i]='1';
		else
			str[i]='0';
	}
	str[i]='\0';
	count = strlen(str);
	if(copy_to_user(buf,str,count));
	DPRINTK("\t\t   ---over---\n");
	
	return count;
}

//
