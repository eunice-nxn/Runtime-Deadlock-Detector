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
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
//int id = 0;

// lock or unlock, thread id , mutex 주소


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

	if(mkfifo("channel", 0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}

	int fd = open("channel", O_WRONLY | O_SYNC );
	int ret = 0;
	if( (ret = write_bytes(fd, a, len)) != len)
		perror("write error");
	close(fd);
	return 0;

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
	pthread_t thread_id = pthread_self_p();

	pthread_mutex_lock_p(&m);
		sender(&lock, sizeof(int));
		sender(&thread_id, sizeof(pthread_t));
		sender(&mutex, sizeof(pthread_mutex_t *));
	pthread_mutex_unlock_p(&m);

	
	int p = pthread_mutex_lock_p(mutex);
	char * buf = "pthread_mutex_lock";

	printf("%s | mode : %d  | mutex : %p | thread_id %ld\n", buf, lock, mutex, thread_id);

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

	int p = pthread_mutex_unlock_p(mutex);
	char * buf = "pthread_mutex_unlock";


	int unlock = 0;
	pthread_t thread_id = pthread_self_p();

	pthread_mutex_lock_p(&m);
		sender(&unlock, sizeof(int));
		sender(&thread_id, sizeof(pthread_t));
		sender(&mutex, sizeof(pthread_mutex_t *));	
	pthread_mutex_unlock_p(&m);
		
	printf("%s | mode : %d  | mutex : %p | thread_id %ld\n", buf, unlock, mutex, thread_id);
	return p;

}
