#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>

int read_bytes(int fd, void * a, int len){

	char * s = (char *) a;
	int i;

	for( i = 0 ; i < len ; ) {
		int b ;
		b = read(fd, s + i, len - i);
//		if( b == 0 ){
//			break;
//		}
		i += b;
	}
	return i;
}

int main(){

	if(mkfifo("channel", 0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}
	
	int fd = open("channel", O_RDONLY | O_SYNC);

	while(1) {

		//flock(fd, LOCK_EX) ;
		int mode = 0;
		int r = 0;

		if( (r = read_bytes(fd, &mode, sizeof(int))) != sizeof(mode) )
				perror("read bytes error\n");



		pthread_t thread_id;
		if( (r = read_bytes(fd, &thread_id, sizeof(pthread_t))) != sizeof(thread_id) )
				perror("read bytes error\n");
		pthread_mutex_t * mutex ;
		if( (r = read_bytes(fd, &mutex, sizeof(pthread_mutex_t *))) != sizeof(mutex) )
				perror("read bytes error\n");
		//flock(fd, LOCK_UN) ;

		
		printf("mode : %d | thread_id : %ld | mutex %p \n", mode, thread_id, mutex);

	}
	close(fd);

}
