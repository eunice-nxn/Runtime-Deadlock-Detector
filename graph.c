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

