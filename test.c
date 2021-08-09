#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void * print_message_function(void *ptr){
	char * message;
	message = (char*) ptr;

	int i = 0; 
	for( i = 0 ; i < 3; i++){
			printf("%s %d\n", message, i);
			printf("%s %d\n", message, i);
			printf("%s %d\n", message, i);
	}

}
int main(){


	pthread_t pthread1 ; 

	char * message = "pthread 1\n";

	pthread_create(&pthread1, 0x0, print_message_function, (void*) message);

	pthread_join(pthread1, NULL);

	exit(0);
}

