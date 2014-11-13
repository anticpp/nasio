#include <stdlib.h>
#include "nlist.h"

void nlist_add_tail(nlist_t *l, nlist_node_t *n)
{
	n->next = NULL;
	n->prev = l->tail;

	if( l->tail )
		l->tail->next = n;

	l->tail = n;
	if( !l->head )
		l->head = n;

	return;
}

void nlist_del(nlist_t *l, nlist_node_t *n)
{
	if( l->tail==n )
		l->tail = n->prev;
	if( l->head==n )
		l->head = n->next;

	if( n->next )
		n->next->prev = n->prev;
	if( n->prev )
		n->prev->next = n->next;
	return;
}

