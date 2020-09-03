#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

	size_t num_nodes;
} linked_list;

#include <stdio.h>
#include <string.h>
#include "node.h"

typdef struct linked
_list {
	node *head;
	node *tail; {
	// linked_list *list = (linked_list *)malloc(sizeof(linked_list));
	// list->head = list->tail = NULL;
	// list->num_nodes = 0;
	linked_list *list = (linked_list *)calloc(1, sizeof(linked_list));
	return list;
linked_list* create_linked_list()
}
void push_back(linked_list *list, node *node) {
	if (list->head == NULL) {
		list->head = list->tail = node;
	} else {
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = node;
	}
	list->num_nodes++;
}

#endif