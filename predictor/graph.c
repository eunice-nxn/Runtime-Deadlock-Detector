#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

Node * node_init(long int thread_id, pthread_mutex_t * mutex) {

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
	new->visited = 0;
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

lock_set * lock_set_init(long int thread_id){

	lock_set * new = (lock_set *) malloc (sizeof(lock_set));
	new->thread_id = thread_id;
	new->num_elem = 0;
	memset ( new->elem, 0, sizeof(new->elem));
	return new;

}

int print_graph (Graph * g){
	
	printf("====================================\n");
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
	printf("==================================\n");
	return 0;

}


int delete_edge (Graph * g, Edge * e){

	if(g->e_list_first == NULL){
		perror("Node doesn't exist");
		exit(1);
	} 

	Edge * i = g->e_list_first;
	Edge * prev_edge = NULL;

	if( i->v->thread_id == e->v->thread_id && i->v->mutex == e->v->mutex)
	{	
		g->e_list_size--;
		g->e_list_first = i->next;
		free(i);
		return 0;
	}

	for( ; i != NULL ; i = i->next ){
		if( i->v->thread_id == e->v->thread_id && i->v->mutex == e->v->mutex)
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
		perror("Node doesn't exist");
		exit(1);
	} 

	Node * i = g->n_list_first;
	Node * prev_node = NULL;

	if( i->thread_id == n->thread_id && i->mutex == n->mutex ){
		g->n_list_size--;
		g->n_list_first = i->next;
		free(i);
		return 0;
	}

	for( ; i != NULL ; i = i->next ){
		if( i->thread_id == n->thread_id && i->mutex == n->mutex ){
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


lock_set * get_lock_set (Graph * g, long int thread_id){
	 
	lock_set * new = lock_set_init(thread_id);
	Edge * i = g->e_list_first;
	if( i == NULL ){
		Node * j = g->n_list_first;
		for( ; j != NULL ; j = j->next ){
			if( j->thread_id == thread_id ){
				new->elem[new->num_elem] = j->mutex;
				new->num_elem++;
			}
		}
	}
	for( ; i != NULL ; i = i->next ){
		if( i->u->thread_id == thread_id ){
			new->elem[new->num_elem] = i->v->mutex;
			new->num_elem++;
		}
	}

	return new;

}

int release_lock(Graph * g, long int thread_id, pthread_mutex_t * mutex){

	Node * i = g->n_list_first;
	for( ; i != NULL ; i = i->next ){
		if(i->thread_id == thread_id && i->mutex == mutex){
			delete_node(g, i);
		}	
	}
	return 0;
}

int acquire_lock (Graph * g, long int thread_id, pthread_mutex_t * mutex){

	Node * new = node_init(thread_id, mutex);
	insert_node (g, new);
	Node * i = g->n_list_first;
	pthread_mutex_t * v_mutex;
	int flag = 0;
	for( ; i != NULL ; i = i->next){
		if( i->thread_id == thread_id && i->mutex != mutex ){
			flag = 1;
			memcpy(&v_mutex, &i->mutex, sizeof(i->mutex));
		}
	}

	if(flag){
		Node * u = node_init(thread_id, v_mutex);
		Node * v = node_init(thread_id, mutex);
		Edge * new_edge = edge_init(u, v);
		insert_edge(g, new_edge);
	}

	return 0;
}

int guard_lock_exist (Graph * g, long int a_thread_id, long int b_thread_id){

	lock_set * a = get_lock_set(g, a_thread_id);
       	lock_set * b = get_lock_set(g, b_thread_id);
	for( int i = 0 ; i < a->num_elem ; i++ ){
		for( int j = 0 ; j < b->num_elem ; j++ ){
			if(a->elem[i] == b->elem[j])
				return 1;
		}
	}	
	return 0;
}

int predict_deadlock(Graph * g){

	Edge * i = g->e_list_first;
	int visit = 1;
	for( ; i != NULL ; i = i->next ){
		if(i->visited == 0){
			i->visited = visit;
			Node * candidate = i->v;
			Edge * j = g->e_list_first;
			for( ; j != NULL ; ){
				if( j->u->mutex == candidate->mutex 
						&& j->u->thread_id != candidate->thread_id ){
					if( j->visited == visit ){
						if(guard_lock_exist(g, j->u->thread_id, candidate->thread_id)){
								printf("guard_lock_exist(g, j->u->thread_id, candidate->thread_id)\n");
								return 0;
						}
						printf("NOT guard_lock_exist\n");
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
