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

#define NUM_TIMER0				(21)
#define	VIC0VECTADDR	(vic_base + 0x100)

#define	TCON	(*((volatile unsigned long *)(pwm_base+0x08)) )
#define	TCFG0	( *((volatile unsigned long *)(pwm_base+0x00)) )
#define	TCFG1	( *((volatile unsigned long *)(pwm_base+0x04)) )
#define	TCNTB0	( *((volatile unsigned long *)(pwm_base+0x0C)) )
#define	TCMPB0	( *((volatile unsigned long *)(pwm_base+0x10)) )
#define	TCNTO0 	( *((volatile unsigned long *)(pwm_base+0x14)) )
#define	TINT_CSTAT	( *((volatile unsigned long *)(pwm_base+0x44)) )

#define	VIC0ADDR	( *((volatile unsigned long *)(vic_base + 0xf00)) )
#define	VIC0INTSELECT	( *((volatile unsigned long *)(vic_base + 0x0c)) )
#define	VIC0INTENABLE	( *((volatile unsigned long *)(vic_base + 0x10)) )
#define	VIC0INTENCLEAR	( *((volatile unsigned long *)(vic_base + 0x14)) )

#define TIME 80	//播放时间

#define FREQ0 0		//音调 低
#define FREQ1 1		//音调 中
#define FREQ2 2		//音调 高

#define z_mod 1.1

/*--函数声明--*/
void yinjie(int);
void timerIrs(void);//中断处理函数
void panySony(void);//播放音符
void overMusic(void);//停止播放
//void timerInit(int f,void (*handler)(void));//定时器初始化
void timer_init(unsigned long,unsigned long,unsigned long,unsigned long,unsigned long);
void intc_setvectaddr(unsigned long intnum, void (*handler)(void));
void buz_exit(void);
static int buz_init(void);
long buz_ioctl(struct file*, unsigned int, unsigned long);

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

volatile int freq = 0;//音调 低、中、高
volatile int tone = 0;//音 1~7
volatile int time = 0;//时间 具体一个音的时长
volatile int buz_index = 0;//下标
unsigned int songnum=0;//第几首歌

static int z_count = 0;
int songLen[] = {28,32};//每首歌的长度

/*--定义一个二维数组的音阶--*/
int f[3][8] = {
	{0,262,294,330,349,392,440,494},
	{0,523,578,659,698,784,880,988},
	{0,1046,1175,1318,1397,1568,1760,1976}
};


/*--定义一个歌曲的结构体类型--*/
struct songData{
	int freq;//音调 低、中、高
	int tone;//音 1~7
	int time;//时间 具体一个音的时长
};

/*--一首歌的结构体数组--*/
struct songData music[10][40] = {
	{
	//小星星
		{FREQ1,1,TIME},	{FREQ1,1,TIME},	{FREQ1,5,TIME},	{FREQ1,5,TIME},
		{FREQ1,6,TIME},	{FREQ1,6,TIME},	{FREQ1,5,TIME*2},
		{FREQ1,4,TIME},	{FREQ1,4,TIME},	{FREQ1,3,TIME},	{FREQ1,3,TIME},
		{FREQ1,2,TIME},	{FREQ1,2,TIME},	{FREQ1,1,TIME*2},
		{FREQ1,5,TIME},	{FREQ1,5,TIME},	{FREQ1,4,TIME},	{FREQ1,4,TIME},
		{FREQ1,3,TIME},	{FREQ1,3,TIME},	{FREQ1,2,TIME*2},
		{FREQ1,5,TIME},	{FREQ1,5,TIME},	{FREQ1,4,TIME},	{FREQ1,4,TIME},
		{FREQ1,3,TIME},	{FREQ1,3,TIME},	{FREQ1,2,TIME*2}
	},

	{
	//两只老虎
		{FREQ2,1,TIME},	{FREQ2,2,TIME},	{FREQ2,3,TIME},	{FREQ2,1,TIME},
		{FREQ2,1,TIME},	{FREQ2,2,TIME},	{FREQ2,3,TIME},	{FREQ2,1,TIME},	
		{FREQ2,3,TIME},	{FREQ2,4,TIME},	{FREQ2,5,TIME*2},
		{FREQ2,3,TIME},	{FREQ2,4,TIME},	{FREQ2,5,TIME*2},
		{FREQ2,5,TIME/2},{FREQ2,6,TIME/2},{FREQ2,5,TIME/2},{FREQ2,4,TIME/2},{FREQ2,3,TIME},{FREQ2,1,TIME},
		{FREQ2,5,TIME/2},{FREQ2,6,TIME/2},{FREQ2,5,TIME/2},{FREQ2,4,TIME/2},{FREQ2,3,TIME},{FREQ2,1,TIME},
		{FREQ2,2,TIME},	{FREQ2,5,TIME},	{FREQ2,1,TIME*2},
		{FREQ2,2,TIME},	{FREQ2,5,TIME},	{FREQ2,1,TIME*2}
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
	DPRINTK("\t\t***IOctrl_Buz***\n");
	
	intc_setvectaddr(NUM_TIMER0,timerIrs);
	if(cmd==0)
	{
	
		if(arg==1){
			timer_init(0,65,2,954,954/z_mod);
		}else if(arg==2){
			timer_init(0,65,2,850,850/z_mod);
		}else if(arg==3){
			timer_init(0,65,2,757,757/z_mod);
		}else if(arg==4){
			timer_init(0,65,2,716,716/z_mod);
		}else if(arg==5){
			timer_init(0,65,2,637,637/z_mod);
		}else if(arg==6){
			timer_init(0,65,2,568,568/z_mod);
		}else if(arg==7){
			timer_init(0,65,2,506,506/z_mod);
		}else{
			//overMusic();
			GPD0CON &= (~0x0f);
		}
	}
	if(cmd==1)
	{
	
		if(arg==1){
			timer_init(0,65,2,238,238/z_mod);
		}else if(arg==2){
			timer_init(0,65,2,212,212/z_mod);
		}else if(arg==3){
			timer_init(0,65,2,189,189/z_mod);
		}else if(arg==4){
			timer_init(0,65,2,178,178/z_mod);
		}else if(arg==5){
			timer_init(0,65,2,159,159/z_mod);
		}else if(arg==6){
			timer_init(0,65,2,142,142/z_mod);
		}else if(arg==7){
			timer_init(0,65,2,127,127/z_mod);
		}else{
			GPD0CON &= (~0x0f);
         	}
	}
	//timer_init(0,65,2,arg,arg/z_mod);

	freq = music[songnum][buz_index].freq;
	tone = music[songnum][buz_index].tone;
	time = music[songnum][buz_index].time;
	//timerInit(f[freq][tone],panySony);
	DPRINTK("\t\t    ---ctrl---\n");
	return 0;
}

void panySony(void)
{
	if(time!=0)
		time--;
	else{
		overMusic();
		if(buz_index>=songLen[songnum]){
			//如果一首歌(所有音阶)全部播放完成
			overMusic();
			//flag=0;
			time=0;//清除时间计数
			buz_index=0;//清除音符索引值

			return;//退出函数
		}
		//上一音符结束，取出下一音符(音调,音,时长)
		freq = music[songnum][buz_index].freq;
		tone = music[songnum][buz_index].tone;
		time = music[songnum][buz_index].time;

		//打印信息
		DPRINTK("%d",tone);

		//启动定时器(开始发声)
		//timerInit(f[freq][tone],timerIrs);
		time--;
		buz_index++;
	}
}

void overMusic(void)
{
	//关闭定时器
	TCON = 0;
	//GPB0IO口 复用为TOUT_0功能
	GPD0CON &= (~0x0f);
	GPD0CON |= 2<<0;
}
/*
void timerInit(int f,void (*handler)(void))
{
	unsigned long temp;
	//Step 1:关闭所有定时器
	TCON = 0;
	//Step 2:GPB0IO口初始化为功能复用TOUTO
	GPD0CON &= (~0x0f);
	GPD0CON |= 2<<0;
	
	// //Step 3:中断初始化TIMER0
	// VIC0INTENCLEAR = 0xffffffff;// 禁止所有中断
	// VIC0INTSELECT = 0x0;// 选择中断类型为IRQ
	// VIC0ADDR = 0;//当前正在处理的中断的中断处理函数的地址
	// //Step 4:中断使能
	// temp = VIC0INTENABLE;
	// temp |= (1<<21);
	// VIC0INTENABLE = temp;
	
	//Step 5:定时器0初始化(有点小问题)
	temp = TCFG0;
	temp &= ~(255<<0);
	temp |= (65<<0);		//定时器配置寄存器0          使用定时器0        预分频值 249；
	TCFG0 = temp;
	TCFG1 &= ~(15<<20 | 2<<0);
	//Step 6:TCON设置初始化
	TCON |= 1<<1;// 手动更新  4*5
	TCON &= ~(1<<1);// 清手动更新位
	TCON |= (1<<0)|(1<<3);// 自动加载和启动timer0
	TCNTB0 = ((66000000/4/66)/f);	//赋初值	
	TCMPB0 = ((66000000/4/66)/f)/20;
	//Step 7:设置timer0中断的中断处理函数 NUM_TIMER4
	*( (volatile unsigned long *)(VIC0VECTADDR + 4*21) ) = (unsigned long)&handler;
	//Step 8:使能timer0中断
	temp = TINT_CSTAT;
	temp = (temp & (~(1<<0)))|(1<<0);//1<<4 bit4清零 后置一
	TINT_CSTAT = temp;
}
*/

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

	//~~~~~~~~~~~~~~~~~~~~
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

void timerIrs(void)
{
	unsigned long uTmp;
	DPRINTK("coming\n");
	//清timer0的中断状态寄存器
	uTmp = TINT_CSTAT;
	TINT_CSTAT = uTmp;

	VIC0ADDR = 0;
	z_count++;
	Delay();
	if(z_count<2)
	{
		DPRINTK("1\n");
		yinjie(1); // 2表示1/4分频
	}
	else if(z_count<3)
	{
		DPRINTK("2\n");
		yinjie(2);
	}
	else if(z_count<4)
	{
		DPRINTK("3\n");
		yinjie(3);
	}
	else if(z_count<5)
	{
		DPRINTK("4\n");
		yinjie(1);
	}
	else if(z_count<6)
	{
		DPRINTK("5\n");
		yinjie(1);
	}
	else if(z_count<7)
	{
		DPRINTK("6\n");
		yinjie(2);
	}
	else if(z_count<8)
	{
		DPRINTK("7\n");
		yinjie(3);
	}
	else if(z_count<9)
	{
		DPRINTK("8\n");
		yinjie(1);
	}
	else if(z_count<10)
	{
		DPRINTK("9\n");
		yinjie(3);
	}
	else if(z_count<11)
	{
		DPRINTK("10\n");
		yinjie(4);
	}
	else if(z_count<12)
	{
		DPRINTK("11\n");
		yinjie(5);
	}
	else if(z_count<13)
	{
		if(z_count==12)
		{
			z_count=0;
		}
		DPRINTK("12\n");
		yinjie(5);
	}

}


void yinjie(int i)
{
	if(i==1)
	{
		timer_init(0,65,2,954,954/z_mod);
	}if(i==2)
	{
		timer_init(0,65,2,850,850/z_mod);
	}
	if(i==3)
	{
		timer_init(0,65,2,757,757/z_mod);
	}
	if(i==4)
	{
		timer_init(0,65,2,716,716/z_mod);
	}
	if(i==5)
	{
		timer_init(0,65,2,637,637/z_mod);
	}
	if(i==6)
	{
		timer_init(0,65,2,568,568/z_mod);
	}
	if(i==7)
	{
		timer_init(0,65,2,506,506/z_mod);
	}
}

void intc_setvectaddr(unsigned long intnum, void (*handler)(void))
{
	if(intnum<32)			
	{
		*( (volatile unsigned long *)(VIC0VECTADDR + 4*intnum) ) = (unsigned)handler;
	}
	//VIC1
	else if(intnum<64) 		
	{
		//*( (volatile unsigned long *)(VIC1VECTADDR + 4*(intnum-32)) ) = (unsigned)handler;
	}
	//VIC2
	else if(intnum<96) 			
	{
		//*( (volatile unsigned long *)(VIC2VECTADDR + 4*(intnum-64)) ) = (unsigned)handler;
	}
	//VIC3
	else	
	{
		//*( (volatile unsigned long *)(VIC3VECTADDR + 4*(intnum-96)) ) = (unsigned)handler;
	}
	return;
}
