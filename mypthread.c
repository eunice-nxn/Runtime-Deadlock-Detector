#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
{
	void * (*pthread_createp)(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg);	
	char * error;

	pthread_createp = dlsym(RTLD_NEXT, "pthread_create");
	if( (error = dlerror()) != 0x0)
		exit(1);

	char * ptr = pthread_createp(thread, attr, start_routine, arg);

	char * buf = "pthread_create\n"; 
	printf("%s\n", buf);

	return 0;
}


int pthread_join(pthread_t thread, void ** retval)
{
	void * (*pthread_joinp)(pthread_t thread, void ** retval);
	char * error;

	pthread_joinp = dlsym(RTLD_NEXT, "pthread_join");
	if( (error = dlerror()) != 0x0)
		exit(1);

	char * ptr = pthread_joinp(thread, retval);

	char * buf = "pthread_join\n";
	printf("%s\n", buf);



	return 0;
}
