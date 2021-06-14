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

int len,size=8;
int fd1,fd2,fd3;
char key[8];
char str1[]="/dev/led_misc";
char str2[]="/dev/key_misc";
char str3[]="/dev/buz_misc";

int main()
{
	int i;
	fd1 = open(str1,O_RDWR);
	fd2 = open(str2,O_RDWR);
	fd3 = open(str3,O_RDWR);

	while(1){
		read(fd2,key,size);
		for(i=0;i<4;i++){
			if(key[i]=='1'){
				if(i==0){
					ioctl(fd3,0,1);
				}else if(i==1){
					ioctl(fd3,1,1);
				}else if(i==2){
					ioctl(fd3,0,2);
				}else if(i==3){
					ioctl(fd3,1,2);
				}else{
					printf("Error Input\n");
				}
				
			}
		}
	}
	return 0;
}