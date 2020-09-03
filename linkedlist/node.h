#ifndef NODE_H_
#define NODE_H_H

#include<stdlib.h>

typedef struct node {
	void *data;
	struct node *next;
	struct node *prev;
} node;

node *create_node(void *data) {
	node *n = (node *)malloc(sizeof(node));
	n->data = data;
	n->next = NULL;
	n->prev = NULL;
	return n;
}

void free_node(node *n, void (*free_data)(void *)) {
	free_data(n->data);
	free(n);
}

#endif