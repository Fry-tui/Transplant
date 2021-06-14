obj-m:= misc_led.o misc_key.o

ifneq ($(arch), arm)

all:
	make -C /usr/src/linux-headers-3.13.0-32-generic M=$(PWD) modules	

else

CROSS=/opt/FriendlyARM/toolschain/4.5.1/bin/arm-linux-
CC=$(CROSS)gcc
LD=$(CROSS)ld
AR=$(CROSS)ar
AS=$(CROSS)as

all:
	make -C /opt/FriendlyARM/mini210/linux/linux-3.0.8 M=$(PWD) modules

endif

clean:
	rm -rf *.o *.ko *.mod.c *.mod.o *.order *.symvers *~
