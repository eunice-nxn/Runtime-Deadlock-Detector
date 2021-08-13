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

int release_lock(Graph * g, long int thread_id, pthread_mutex_t * mutex){

	int r = -1;
	int cnt = 0;
	Edge * j = g->e_list_first;
	for( ; j != NULL ; j = j->next ){
		if(j->v->mutex == mutex ) {
			cnt++;
			int ret = delete_edge(g, j);
		}
	}
	Node * i = g->n_list_first;
	for( ; i != NULL ; i = i->next ){
			return r;
	}
	return r;
}

int acquire_lock (Graph * g, long int thread_id, pthread_mutex_t * mutex){

	Node * new = node_init(thread_id, mutex);
	int ret = insert_node (g, new);
	Node * i = g->n_list_first;
	for( ; i != NULL ; i = i->next){
		if( i->thread_id == thread_id && i->mutex != mutex ){
			Edge * new_edge = edge_init(i, new);
			int ret = insert_edge(g, new_edge);
		}
	}

	return 0;
}

int is_cycle(Graph * g){

	Edge * i = g->e_list_first;
	int visit = 1;
	for( ; i != NULL ; i = i->next ){
		if(i->visited == 0){
			i->visited = visit;
			Node * candidate = i->v;
			Edge * j = g->e_list_first;
			for( ; j != NULL ; ){
				if( j->u->mutex == candidate->mutex ){
					if( j->visited == visit ){
						return 1;
					} else {
						j->visited = visit;
						candidate = j->v;
						j = g->e_list_first;
					}						
				} else {
					j = j->next;
				}
			}
		}
		visit++;
	}
	return 0;
}

int addr_to_line(char * exec, long int addr){
	
	char command[1024] = { 0 };
	sprintf(command, "addr2line -e %s %lx", exec, addr);
	//printf("command : %s\n", command);
	
	FILE * fp ;
	fp = popen(command, "r");
	if( fp == NULL ){
		return -1;
	}
	char buf[512];
	while(fgets(buf, 512, fp)){
		printf("%s\n", buf);
	}
	pclose(fp);
	exit(1);

}

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

	if(mkfifo(".ddtrace", 0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}
	
	int fd = open(".ddtrace", O_RDONLY | O_SYNC);
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

		if(is_cycle(g)){
			addr_to_line(exec, addr);
		}
	}
	close(fd);

}
