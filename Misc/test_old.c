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

int len;
int arr[]={1,2,3,1,1,2,3,1,3,4,5,5,3,4,5,5,0};//两只老虎
int music[]={1,5,3,1,5,5,2,4,1,2,4,4,5,7,7,1,2,3,5,1,3,3,3,3,3,5,5,1,1,7,0};//小幸运

int fd1,fd2,fd3;
int i=0,size=8;
char key[8];
char str1[]="/dev/led_misc";
char str2[]="/dev/key_misc";
char str3[]="/dev/buz_misc";
	

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
/*
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
*/
/*
int main()
{
	char ops[20];
	
	fd3 = open(str3,O_RDWR);
	while(1)
	{
		memset(ops,0,20);
		printf(":");
		scanf("%s",ops);
		if(strcmp(ops,"1")==0){
			ioctl(fd3,1,1);
		}else if(strcmp(ops,"2")==0){
			ioctl(fd3,1,2);
		}else if(strcmp(ops,"3")==0){
			ioctl(fd3,1,2);
		}else if(strcmp(ops,"2")==0){
			ioctl(fd3,1,2);
		}else if(strcmp(ops,"2")==0){
			ioctl(fd3,1,2);
		}else if(strcmp(ops,"2")==0){
			ioctl(fd3,1,2);
		}else if(strcmp(ops,"2")==0){
			ioctl(fd3,1,2);
		}else{
			printf("Error Input\n");
		}
	}
	close(fd3);
	return 0;
}
*/

int main()
{
	fd3 = open(str3,O_RDWR);
	len = sizeof(arr)/sizeof(arr[0]);

	for(i=0;i<len;i++){
		delay(30000000);
		//ioctl(fd3,1,arr[i]);
		ioctl(fd3,1,arr[i]);
	}
	close(fd3);
	system("rmmod misc_buz");

	return 0;
}

/*
int main()
{
	fd1 = open(str1,O_RDWR);
	fd2 = open(str2,O_RDWR);
	fd3 = open(str3,O_RDWR);

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

	return 0;
}
*/

/*int main()
{
	int x;
	fd3 = open(str3,O_RDWR);
	while(1)
	{
		scanf("%d",&x);
		ioctl(fd3,1,x);
	}
	close(fd3);
	return 0;
}*/
