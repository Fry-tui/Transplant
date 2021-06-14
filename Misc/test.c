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


int fd1,fd2;
int i=0,size=8;
char key[8];
char str1[]="/dev/led_misc";
char str2[]="/dev/key_misc";
	

//延时函数
void delay(unsigned int s)
{
	while(s--);
}

void ctrlLed(void)
{
	while(1){
		read(fd2,key,size);
		for(i=0;i<4;i++){
			if(key[i]=='1'){
				ioctl(fd1,1,i+1);
				sleep(1);
				ioctl(fd1,0,i+1);
			}
		}
	}
	pthread_exit(NULL);
}

int main(int argc,char *argv[])
{
	char ops[20];
	pthread_t id;
	fd1 = open(str1,O_RDWR);
	fd2 = open(str2,O_RDWR);
	pthread_create(&id,NULL,(void *)ctrlLed,NULL); 
	
	while(1)
	{
		memset(ops,0,20);
		scanf("%s",ops);
		if(strcmp(ops,"ls")==0){
			system(ops);
		}
	}

	close(fd1);
	close(fd2);
	return 0;
}
