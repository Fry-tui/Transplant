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
# include <linux/delay.h>
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/device.h>
# include <linux/miscdevice.h>//包含杂项设备驱动的头文件

/*调试开关*/
# define PDEBUG 2
# ifdef PDEBUG
# define DPRINTK printk
# else
# define DPRINTK(...)
# endif

/*--蜂鸣器相关寄存器--*/
static void __iomem *buz_base;
static void __iomem *vic_base;
static void __iomem *pwm_base;

#define GPD0CON (*(volatile unsigned long*)buz_base)
#define GPD0DAT (*(volatile unsigned long*)(buz_base+4))

#define NUM_TIMER0	(21)

#define	TCON	(*((volatile unsigned long *)(pwm_base+0x08)) )
#define	TCFG0	( *((volatile unsigned long *)(pwm_base+0x00)) )
#define	TCFG1	( *((volatile unsigned long *)(pwm_base+0x04)) )
#define	TCNTB0	( *((volatile unsigned long *)(pwm_base+0x0C)) )
#define	TCMPB0	( *((volatile unsigned long *)(pwm_base+0x10)) )
#define	TCNTO0 	( *((volatile unsigned long *)(pwm_base+0x14)) )
#define	TINT_CSTAT	( *((volatile unsigned long *)(pwm_base+0x44)) )

#define TIME 80		//播放时间
#define SPIN 10 	//间隔时间
#define FREQ0 0		//音调 低
#define FREQ1 1		//音调 高

/*--函数声明--*/
static int buz_init(void);//驱动初始化函数

void buz_exit(void);//驱动卸载函数
void overMusic(void);//停止音乐播放播放
long buz_ioctl(struct file*, unsigned int, unsigned long);//驱动控制函数
void timer_init(unsigned long,unsigned long,unsigned long,unsigned long,unsigned long);//定时器初始化函数

/*--实现文件关联结构体--*/
static const struct file_operations buz_fops={
	.owner = THIS_MODULE,
	.unlocked_ioctl= buz_ioctl,
};

/*--实现次设备结构体--*/
struct miscdevice buz_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "buz_misc",
	.fops = &buz_fops,
};

float z_mod = 1.1;
volatile int freq = 0;//音调 低、高
volatile int tone = 0;//音 1~7
volatile int time = 0;//时间 具体一个音的时长
volatile int buz_index = 0;//下标

int songLen[] = {28,32};//每首歌的长度

/*--定义一个二维数组的音阶--*/
int f[2][8] = {
	{0,954,850,757,716,637,568,506},//低音对应的数据
	{0,238,212,189,178,159,142,127}//高音对应的数据
};


/*--定义一个歌曲的结构体类型--*/
struct songData{
	int freq;//音调 低、高
	int tone;//音 1~7
	int time;//时间 具体一个音的时长
	int spin;//间隔 音之间的间隔
};

/*--一首歌的结构体数组--*/
struct songData music[10][40] = {
	{
	//小星星
		{FREQ0,1,TIME,SPIN},	{FREQ0,1,TIME,SPIN},	{FREQ0,5,TIME,SPIN},	{FREQ0,5,TIME,SPIN},
		{FREQ0,6,TIME,SPIN},	{FREQ0,6,TIME,SPIN},	{FREQ0,5,TIME*2,SPIN},
		{FREQ0,4,TIME,SPIN},	{FREQ0,4,TIME,SPIN},	{FREQ0,3,TIME,SPIN},	{FREQ0,3,TIME,SPIN},
		{FREQ0,2,TIME,SPIN},	{FREQ0,2,TIME,SPIN},	{FREQ0,1,TIME*2,SPIN},
		{FREQ0,5,TIME,SPIN},	{FREQ0,5,TIME,SPIN},	{FREQ0,4,TIME,SPIN},	{FREQ0,4,TIME,SPIN},
		{FREQ0,3,TIME,SPIN},	{FREQ0,3,TIME,SPIN},	{FREQ0,2,TIME*2,SPIN},
		{FREQ0,5,TIME,SPIN},	{FREQ0,5,TIME,SPIN},	{FREQ0,4,TIME,SPIN},	{FREQ0,4,TIME,SPIN},
		{FREQ0,3,TIME,SPIN},	{FREQ0,3,TIME,SPIN},	{FREQ0,2,TIME*2,SPIN}
	},

	{
	//两只老虎
		{FREQ1,1,TIME,SPIN},	{FREQ1,2,TIME,SPIN},	{FREQ1,3,TIME,SPIN},	{FREQ1,1,TIME,SPIN},
		{FREQ1,1,TIME,SPIN},	{FREQ1,2,TIME,SPIN},	{FREQ1,3,TIME,SPIN},	{FREQ1,1,TIME,SPIN},	
		{FREQ1,3,TIME,SPIN},	{FREQ1,4,TIME,SPIN},	{FREQ1,5,TIME*2,SPIN},
		{FREQ1,3,TIME,SPIN},	{FREQ1,4,TIME,SPIN},	{FREQ1,5,TIME*2,SPIN},
		{FREQ1,5,TIME/2,SPIN},	{FREQ1,6,TIME/2,SPIN},	{FREQ1,5,TIME/2,SPIN},	{FREQ1,4,TIME/2,SPIN},	{FREQ1,3,TIME,SPIN},	{FREQ1,1,TIME,SPIN},
		{FREQ1,5,TIME/2,SPIN},	{FREQ1,6,TIME/2,SPIN},	{FREQ1,5,TIME/2,SPIN},	{FREQ1,4,TIME/2,SPIN},	{FREQ1,3,TIME,SPIN},	{FREQ1,1,TIME,SPIN},
		{FREQ1,2,TIME,SPIN},	{FREQ1,5,TIME,SPIN},	{FREQ1,1,TIME*2,SPIN},
		{FREQ1,2,TIME,SPIN},	{FREQ1,5,TIME,SPIN},	{FREQ1,1,TIME*2,SPIN}
	},		
};

//关联模块初始函数
module_init(buz_init);
//关联模块卸载函数
module_exit(buz_exit);
//写入模块协议
MODULE_LICENSE("GPL");
//写入模块作者
MODULE_AUTHOR("XYQ");
//设置模块版本号
MODULE_VERSION("XXX");

void Delay(void)
{
	int i=1000000;
	while(i--);
}

//Init
static int buz_init()
{
	DPRINTK("\t\t###Initial_Buz###\n");
	misc_register(&buz_misc);//动态分配此设备号
	buz_base = ioremap(0xE02000A0,0x0C);//映射起始地址,映射的空间的大小	
	vic_base = ioremap(0xF2000000,0x0C);
	pwm_base = ioremap(0xE2500000,0x0C);
	
	//设置蜂鸣器
	GPD0CON &= (~0x0f); //后四位清零
	GPD0CON |= 2<<0;//设置GPD0_0为TOUT_0模式

	DPRINTK("\t\t   ---over---\n");//提示结束初始化函数
	return 0;
}

//Exit
void buz_exit()
{
	DPRINTK("\t\t^v^EXIT_Buz^V^\n");
	iounmap(buz_base);//解除地址映射
	iounmap(vic_base);//解除地址映射
	iounmap(pwm_base);//解除地址映射
	misc_deregister(&buz_misc);//传入此设备结构体注销该杂项设备驱动
	DPRINTK("\t\t    ---bye---\n");
	return;
}

//ioctl                            /* cmd:0停 1响  arg:音调*/
long buz_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int i;
	unsigned long cnt;
	DPRINTK("\t\t***IOctrl_Buz***\n");
	
	if(cmd==1&&(arg==1||arg==2)){
		for(i=0;i<songLen[arg-1];i++){
			cnt = f[music[arg-1][i].freq][music[arg-1][i].tone];
			timer_init(0,65,2,cnt,cnt/z_mod);
			msleep(music[arg-1][i].time);
			overMusic();
			msleep(music[arg-1][i].spin);
			//timer_init(0,65,2,j,j/z_mod);
			
		}
	}else{
		overMusic();
	}
	
	DPRINTK("\t\t    ---ctrl---\n");
	return 0;
}

void overMusic(void)
{
	GPD0CON &= (~0x0f);
}

void timer_init(unsigned long utimer,unsigned long uprescaler,unsigned long udivider,unsigned long utcntb,unsigned long utcmpb)
{
	unsigned long temp0;
	GPD0CON &= (~0x0f); //后四位清零
	GPD0CON |= 2<<0;//设置GPD0_0为TOUT_0模式
	// 定时器的输入时钟 = PCLK / ( {prescaler value + 1} ) / {divider value} = PCLK/(65+1)/16=62500hz
	//	PCLK = 66Mhz
	//设置预分频系数为66
	temp0 = TCFG0;
	temp0 = (temp0 & (~(0xff00ff))) | ((uprescaler + 1)<<0);
	//temp0 = (temp0 & (~(0xffff00))) | ((uprescaler + 1)<<8);
	TCFG0 = temp0;


	// 4分频
	temp0 = TCFG1;
	//udivider = 4
	temp0 = (temp0 & (~(0xf<<4*utimer))& (~(1<<20))) |(udivider<<4*utimer);
	TCFG1 = temp0;

	// 1s = 250khz
	TCNTB0 = utcntb;
	TCMPB0 = utcmpb;

	// 手动更新  4*5
	TCON |= 1<<1;

	// 清手动更新位
	TCON &= ~(1<<1);

	// 自动加载和启动timer0
	TCON |= (1<<0)|(1<<3);

	// 使能timer0中断
	temp0 = TINT_CSTAT;
	temp0 = (temp0 & (~(1<<utimer)))|(1<<(utimer));//1<<4 bit4清零 后置一
	TINT_CSTAT = temp0;
}
