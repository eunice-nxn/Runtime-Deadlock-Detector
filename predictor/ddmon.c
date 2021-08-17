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

#include "graph.h"

static Graph * volatile g = 0x0;


int get_info_for_alert (char ** file_name , long int * addr){

	void * buffer[3];
	int num_of_addr = backtrace(buffer, 3);
	char ** str = backtrace_symbols(buffer, num_of_addr);
	if (str == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}
	for(int i = 0 ; i < num_of_addr ; i++){
		char * _addr = (char *) malloc (sizeof(char));
		*file_name = strchr(str[i], '/');
		_addr = strchr(str[i], '+');
		*file_name = strtok(*file_name + 1, "(");
		_addr = strtok( _addr + 1, ")");
		*addr = strtol(_addr, NULL, 16);
	}
	return 0;

}

int pthread_mutex_lock(pthread_mutex_t * mutex){


	if( g == 0x0 ){
		printf("graph_init acquire\n");
		g = graph_init();
	}

	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex);
	pthread_t (*pthread_self_p)(void);

	char * error_lock, * error_self;
	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if( (error_lock = dlerror()) != 0x0 ){
		exit(1);
	}
	pthread_self_p = dlsym(RTLD_NEXT, "pthread_self");
	if( (error_self = dlerror()) != 0x0 ){
		exit(1);
	}


	char * file_name = 0x0;
	long int addr = 0;
	get_info_for_alert(&file_name, &addr);
	long int thread_id = (long int) pthread_self_p();

	printf("acquire_lock: thread_id %ld mutex %p\n", thread_id, mutex);
	acquire_lock(g, thread_id, mutex);
	
	if( detect_deadlock(g) ){
		print_graph(g);
		addr_to_line(file_name, addr);
	}
	int p = 0;
	p = pthread_mutex_lock_p(mutex); 

	return p;


}


int pthread_mutex_unlock(pthread_mutex_t *mutex){

	if( g == 0x0 ){
		printf("graph_init release\n");
		g = graph_init();
	}

	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self_p)(void);

	char * error_unlock, * error_self;

	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if( (error_unlock = dlerror()) != 0x0 )
		exit(1);

	pthread_self_p = dlsym(RTLD_NEXT, "pthread_self");
	if( (error_self = dlerror()) != 0x0 )
		exit(1);
	
	char * file_name = 0x0;
	long int addr = 0;
	get_info_for_alert(&file_name, &addr);

	
	long int thread_id = pthread_self_p();
	printf("release_lock thread_id : %ld mutex %p\n", thread_id, mutex);
	release_lock(g, thread_id, mutex);
	
	if( detect_deadlock(g) ){
		print_graph(g);
		addr_to_line(file_name, addr);
	}

	int p = pthread_mutex_unlock_p(mutex);
	return p;

}




