#include <stdlib.h>
#include "nlist.h"

void nlist_insert_after(nlist_t *l, nlist_node_t *p, nlist_node_t *n)
{
	n->prev = p;
	n->next = p->next;

	if( p->next )
		p->next->prev = n;
	p->next = n;

	if( l->tail==p )
		l->tail = n;
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

