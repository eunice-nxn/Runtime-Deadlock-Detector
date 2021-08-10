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
	
int sender (int * mode, int mode_len, pthread_t * thread_id, int thread_id_len, pthread_mutex_t * mutex, int mutex_len){

	if(mkfifo("channel", 0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}

	int fd = open("channel", O_WRONLY | O_SYNC );
	int r = 0;
	if( (r = write_bytes(fd, mode, mode_len)) != mode_len)
		perror("write error");
	if( (r = write_bytes(fd, thread_id, thread_id_len)) != thread_id_len)
		perror("write error");
	if( (r = write_bytes(fd, &mutex, mutex_len)) != mutex_len)
		perror("write error");

	close(fd);
	return 0;
	
}


int pthread_mutex_lock(pthread_mutex_t * mutex){


	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t * mutex);

	char * error_lock,* error_unlock;
	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if( (error_lock = dlerror()) != 0x0){
		exit(1);
	}
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if( (error_unlock = dlerror()) != 0x0){
		exit(1);
	}


	int p = pthread_mutex_lock_p(mutex);
	char * buf = "pthread_mutex_lock";

	int lock = 1;
	pthread_t thread_id = pthread_self();

	pthread_mutex_lock_p(&m);
		sender(&lock, sizeof(int), &thread_id, sizeof(pthread_t), mutex, sizeof(pthread_mutex_t *));
	pthread_mutex_unlock_p(&m);

	printf("mutex : %p\n", mutex);
	printf("%s current thread %d\n", buf, (int) thread_id);
	
	return p;


}


int pthread_mutex_unlock(pthread_mutex_t *mutex){

	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);

	char * error_lock,* error_unlock;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if( (error_lock = dlerror()) != 0x0 )
		exit(1);

	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if( (error_unlock = dlerror()) != 0x0 )
		exit(1);

	int p = pthread_mutex_unlock_p(mutex);
	char * buf = "pthread_mutex_unlock";


	int unlock = 0;
	pthread_t thread_id = pthread_self();

	pthread_mutex_lock_p(&m);
		sender(&unlock, sizeof(int), &thread_id, sizeof(pthread_t), mutex, sizeof(pthread_mutex_t *));
	pthread_mutex_unlock_p(&m);
		
	printf("mutex : %p\n", mutex);
	printf("%s current thread %ld\n", buf, thread_id);
	return p;

}
