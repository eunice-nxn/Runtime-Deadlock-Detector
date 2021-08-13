#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <execinfo.h>
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int write_bytes (int fd , void * a, int len){

	char * s = (char *) a;

	int i = 0 ; 
	while ( i < len ){
		int b;
		b = write(fd, s + i, len - i);
		if( b == 0 )
			break;
		i += b;
	}
	return i;
}

int sender (void * a, int len){

	if(mkfifo(".ddtrace", 0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}

	int fd = open(".ddtrace", O_WRONLY | O_SYNC );
	int ret = 0;
	if( (ret = write_bytes(fd, a, len)) != len)
		perror("write error");
	close(fd);
	return 0;

}

long int get_addr_for_alert (){

	long int addr = 0;
	void * buffer[3];
	int num_of_addr = backtrace(buffer, 3);
	char ** str = backtrace_symbols(buffer, num_of_addr);
	if (str == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}
	for(int i = 0 ; i < num_of_addr ; i++){
		char * _addr = (char *) malloc (sizeof(char));
		_addr = strchr(str[i], '+');
		_addr = strtok( _addr + 1, ")");
		addr = strtol(_addr, NULL, 16);
	}
	return addr;


}

int pthread_mutex_lock(pthread_mutex_t * mutex){


	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t * mutex);
	pthread_t (*pthread_self_p)(void);

	char * error_lock,* error_unlock, * error_self;
	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if( (error_lock = dlerror()) != 0x0 ){
		exit(1);
	}
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if( (error_unlock = dlerror()) != 0x0 ){
		exit(1);
	}
	pthread_self_p = dlsym(RTLD_NEXT, "pthread_self");
	if( (error_self = dlerror()) != 0x0 ){
		exit(1);
	}


	int lock = 1;
	long int thread_id = (long int) pthread_self_p();
	long int addr = get_addr_for_alert();
	pthread_mutex_lock_p(&m);	
		sender(&lock, sizeof(int));
		sender(&thread_id, sizeof(long int));
		sender(&mutex, sizeof(pthread_mutex_t *));
		sender(&addr, sizeof(long int));
	pthread_mutex_unlock_p(&m);

	
	int p = 0;
	p = pthread_mutex_lock_p(mutex); 
	//char * buf = "pthread_mutex_lock";

//	printf("%s | mode : %d  | mutex : %p | thread_id %ld\n", buf, lock, mutex, thread_id);


	return p;


}


int pthread_mutex_unlock(pthread_mutex_t *mutex){

	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self_p)(void);

	char * error_lock,* error_unlock, * error_self;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if( (error_lock = dlerror()) != 0x0 )
		exit(1);

	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if( (error_unlock = dlerror()) != 0x0 )
		exit(1);

	pthread_self_p = dlsym(RTLD_NEXT, "pthread_self");
	if( (error_self = dlerror()) != 0x0 )
		exit(1);
	
	int unlock = 0;
	long int thread_id = pthread_self_p();
	long int addr = get_addr_for_alert();

	pthread_mutex_lock_p(&m);
		sender(&unlock, sizeof(int));
		sender(&thread_id, sizeof(long int));
		sender(&mutex, sizeof(pthread_mutex_t *));	
		sender(&addr, sizeof(long int));
	pthread_mutex_unlock_p(&m);
		
	//printf("%s | mode : %d  | mutex : %p | thread_id %ld\n", buf, unlock, mutex, thread_id);
	int p = pthread_mutex_unlock_p(mutex);
	char * buf = "pthread_mutex_unlock";
	return p;

}

