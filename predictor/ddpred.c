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
#include <execinfo.h>
#include "graph.h"

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


int receiver (int fd, int * mode, long int * thread_id, pthread_mutex_t ** mutex, long int * addr){

	int r = 0;
	if( (r = read_bytes(fd, mode, sizeof(int))) != sizeof(int) ){
		perror("read bytes error\n");
		exit(1);
	}

	if( (r = read_bytes(fd, thread_id, sizeof(long int))) != sizeof(long int) ){
		 perror("read bytes error\n");
		 exit(1);
	}

	if( (r = read_bytes(fd, mutex, sizeof(pthread_mutex_t *))) != sizeof(pthread_mutex_t *) ){
		perror("read bytes error\n");
		exit(1);
	}

	if( (r = read_bytes(fd, addr, sizeof(long int))) != sizeof(long int) ){
		 perror("read bytes error\n"); 
		 exit(1);
	}

	return 0;
}

int main(int argc, char * argv[]){

	if( argc < 2 ){
		perror("Too few arguments");
		exit(1);
	}

	char * exec = (char *) malloc (sizeof(char) * strlen(argv[1]));
	strcpy(exec, argv[1]);

	Graph * g = graph_init();

	if(mkfifo("dmonitor.trace", 0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}
	
	int fd = open("dmonitor.trace", O_RDONLY | O_SYNC);
	while(1) {

		flock(fd, LOCK_EX) ;
		int mode = 0;
		long int thread_id;
		pthread_mutex_t * mutex ;
		long int addr = 0;
		
		receiver(fd, &mode, &thread_id, &mutex, &addr );
		flock(fd, LOCK_UN) ;
		//printf("mode : %d | thread_id : %ld | mutex %p addr_deadlock %ld\n", mode, thread_id, mutex, addr );
		if(mode == 1){
			int ret = acquire_lock(g, thread_id, mutex);
		} else if (mode == 0){
			int ret = release_lock(g, thread_id, mutex);
		}

		if(predict_deadlock(g))
		 	addr_to_line(exec, addr);
	}
	close(fd);

}
