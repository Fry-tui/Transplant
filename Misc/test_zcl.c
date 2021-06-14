# include <time.h>
# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <signal.h> 
# include <unistd.h>
# include <pthread.h>
# include <sys/shm.h>
# include <sys/ipc.h>
# include <sys/msg.h>
# include <sys/stat.h>
# include <semaphore.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

//按键1:闪烁全灭全亮
//按键2:流水灯
//按键3:播歌1
//按键4:播歌2

int len,size=8;
int fd1,fd2,fd3;
char key[8];
char str1[]="/dev/led_misc";
char str2[]="/dev/key_misc";
char str3[]="/dev/buz_misc";
int music_01[]={1,2,3,1,1,2,3,1,3,4,5,5,3,4,5,5,0};//两只老虎
int music_02[]={1,5,3,1,5,5,2,4,1,2,4,4,5,7,7,1,2,3,5,1,3,3,3,3,3,5,5,1,1,7,0};//小幸运

int led_flag=0;
int buz_flag=0;

void ctrlLed(void)
{
	int i;
	while(1){
		if(led_flag==1){
			for(i=0;i<4;i++)
				ioctl(fd1,1,i+1);
			sleep(1);
			for(i=0;i<4;i++)
				ioctl(fd1,0,i+1);
			sleep(1);
		}else if(led_flag==2){
			for(i=0;i<4;i++){
				ioctl(fd1,1,i+1);
				sleep(1);
				ioctl(fd1,0,i+1);
			}
		}
	}
	close(fd1);
	pthread_exit(NULL);
}

void ctrlBuz(void)
{
	int i;
	while(1){
		if(buz_flag==1){
			len = sizeof(music_01)/sizeof(music_01[0]);
			for(i=0;i<len;i++){
				delay(30000000);
				ioctl(fd3,1,music_01[i]);
			}
		}else if(buz_flag==2){
			len = sizeof(music_02)/sizeof(music_02[0]);
			for(i=0;i<len;i++){
				delay(30000000);
				ioctl(fd3,1,music_02[i]);
			}
		}else{
			ioctl(fd3,0,0);
		}
	}
	close(fd3);
	pthread_exit(NULL);
}

int main()
{
	int i;
	pthread_t id1,id2;
	fd1 = open(str1,O_RDWR);
	fd2 = open(str2,O_RDWR);
	fd3 = open(str3,O_RDWR);

	pthread_create(&id1,NULL,(void *)ctrlLed,NULL); 
	pthread_create(&id2,NULL,(void *)ctrlBuz,NULL); 
	//按键1:闪烁全灭全亮
	//按键2:流水灯
	//按键3:播歌1
	//按键4:播歌2
	printf("\t###############\n");
	printf("\t\t功能提示\n");
	printf("\tkey1:全亮全灭\n");
	printf("\tkey2:循环流水\n");
	printf("\tkey3:两只老虎\n");
	printf("\tkey4:尽请期待\n");
	while(1){
		read(fd2,key,size);
		for(i=0;i<4;i++){
			if(key[i]=='1'){
				if(i==0){
					led_flag = 1;
				}else if(i==1){
					led_flag = 2;
				}else if(i==2){
					buz_flag = 1;
				}else if(i==3){
					buz_flag = 2;
				}else{
					printf("Error Input\n");
				}
				
			}
		}
	}
	
	return 0;
}