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

typedef struct Node {
	pthread_t thread_id;
	pthread_mutex_t * mutex;
	struct Node * next;
} Node;

typedef struct Edge {
	struct Node * u;
	struct Node * v;
	struct Edge * next;
} Edge;

typedef struct Graph {
	int n_list_size;
	int e_list_size;
	struct Node * n_list_first; // all Nodes
	struct Edge * e_list_first; // all edges
} Graph;

Node * node_init(pthread_t thread_id, pthread_mutex_t * mutex) {

	Node * new = (Node * ) malloc (sizeof(Node));
	new->thread_id = thread_id;
	new->mutex = mutex;
	new->next = NULL;
	return new;
}

Edge * edge_init(Node * u, Node * v){
	Edge * new = (Edge *) malloc (sizeof(Edge));
	new->u = node_init(u->thread_id, u->mutex);
	new->u->next = u->next;
	new->v = node_init(v->thread_id, v->mutex);
	new->v->next = v->next;
	new->next = NULL;
	return new;
}

Graph * graph_init (){
	Graph * new = (Graph *) malloc(sizeof(Graph));
	new->n_list_size = 0;
	new->e_list_size = 0;
	new->n_list_first = NULL;
	new->e_list_first = NULL;
	return new;
}


int print_graph (Graph * g){
	Node * i = g->n_list_first;
	for( ; i != NULL ; i = i->next ){
		printf("thread_id : %ld | mutex : %p\n", i->thread_id, i->mutex);
	}
	Edge * j = g->e_list_first;
	for ( ; j != NULL ; j = j->next ){
		printf("u thread_id : %ld | mutex : %p\n v thread_id : %ld | mutex : %p\n", 
				j->u->thread_id, j->u->mutex, j->v->thread_id, j->v->mutex);
	}
	printf("current n_list_size %d \n", g->n_list_size);
	printf("current e_list_size %d \n", g->e_list_size);
	return 0;
}
int delete_edge (Graph * g, Edge * e){

	if(g->e_list_first == NULL){
		perror("Node don't exist");
		exit(1);
	} 

	Edge * i = g->e_list_first;
	Edge * prev_edge = NULL;

	if( pthread_equal(i->u->thread_id, e->u->thread_id)
				&& i->u->mutex == e->u->mutex
				&& pthread_equal(i->u->thread_id, e->u->thread_id)
				&& i->v->mutex == e->v->mutex)
	{	
		g->e_list_size--;
		g->e_list_first = i->next;
		free(i);
		return 0;
	}

	for( ; i != NULL ; i = i->next ){
		if(pthread_equal(i->u->thread_id, e->u->thread_id)
				&& i->u->mutex == e->u->mutex
				&& pthread_equal(i->u->thread_id, e->u->thread_id)
				&& i->v->mutex == e->v->mutex)
		{
			break;
		} 
		prev_edge = i;
	}
	if( i == NULL ){
		return -1;
	}
	g->e_list_size--;
	prev_edge->next = i->next;
	free(i);
	return 0;
}

int delete_node (Graph * g, Node * n){

	if(g->n_list_first == NULL){
		perror("Node don't exist");
		exit(1);
	} 

	Node * i = g->n_list_first;
	Node * prev_node = NULL;

	if( pthread_equal(i->thread_id, n->thread_id) && i->mutex == n->mutex ){
		g->n_list_size--;
		g->n_list_first = i->next;
		free(i);
		return 0;
	}

	for( ; i != NULL ; i = i->next ){
		if( pthread_equal(i->thread_id, n->thread_id) && i->mutex == n->mutex ){
			break;
		} 
		prev_node = i;
	}
	if( i == NULL ){
		return -1;
	}
	g->n_list_size--;
	prev_node->next = i->next;
	free(i);
	return 0;
}

int insert_edge (Graph * g, Edge * e){

	g->e_list_size++;
	if( g->e_list_first == NULL ){
		g->e_list_first = e;
		return 0;
	}
	Edge * i = g->e_list_first;
	for ( ; i->next != NULL ; i = i->next ) ;
	i->next = e;
	return 0;
}

int insert_node (Graph * g, Node * n){

	g->n_list_size++;
	if( g->n_list_first == NULL ) {
		g->n_list_first = n;
		return 0;
	}
	Node * i = g->n_list_first;
	for( ; i->next != NULL ; i = i->next ) ;
	i->next = n;
	return 0;	
}

int release_lock(Graph * g, pthread_t thread_id, pthread_mutex_t * mutex){

	int r = -1;
	int cnt = 0;
	Edge * j = g->e_list_first;
	for( ; j != NULL ; j = j->next ){
		if(j->v->mutex == mutex ) {
			cnt++;
			int ret = delete_edge(g, j);
			printf("ret %d in release_lock\n", ret);
		}
	}
	Node * i = g->n_list_first;
	for( ; i != NULL ; i = i->next ){
		if( i->mutex == mutex && pthread_equal(i->thread_id, thread_id) ){
			r = delete_node(g, i);
			return r;
		}
	}
	return r;
}

int acquire_lock (Graph * g, pthread_t thread_id, pthread_mutex_t * mutex){

	Node * new = node_init(thread_id, mutex);
	int ret = insert_node (g, new);
	Node * i = g->n_list_first;
	for( ; i != NULL ; i = i->next){
		if( pthread_equal(i->thread_id, thread_id) && i->mutex != mutex){
			printf("i->thread_id %ld thread_id %ld | pthread_equal(i->thread_id, thread_id) %d\n", i->thread_id, thread_id, pthread_equal(i->thread_id, thread_id));
			Edge * new_edge = edge_init(i, new);
			int ret = insert_edge(g, new_edge);
		}
	}
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

int main(){

	Graph * g = graph_init();

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

		if( (r = read_bytes(fd, &mode, sizeof(int))) != sizeof(mode) ){
				perror("read bytes error\n");
				exit(1);
		}

		pthread_t thread_id;
		if( (r = read_bytes(fd, &thread_id, sizeof(pthread_t))) != sizeof(thread_id) ){
				perror("read bytes error\n");
				exit(1);
		}

		pthread_mutex_t * mutex ;
		if( (r = read_bytes(fd, &mutex, sizeof(pthread_mutex_t *))) != sizeof(mutex) ){
				perror("read bytes error\n");
				exit(1);
		}

		//flock(fd, LOCK_UN) ;
		printf("mode : %d | thread_id : %ld | mutex %p \n", mode, thread_id, mutex);
		int it_a = 0, it_r = 0;
		if(mode == 1){
			int ret = acquire_lock(g, thread_id, mutex);
			it_a++;
		} else if (mode == 0){
			int ret = release_lock(g, thread_id, mutex);
			it_r++;
		}
		printf("==================result===================\n");
		print_graph(g);
		printf("===========================================\n\n");
		printf("it_a : %d | it_r : %d \n", it_a, it_r);
	}
	close(fd);

}
