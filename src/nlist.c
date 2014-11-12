#include "nlist.h"

void nlist_add_tail(nlist *l, nlist_node *n)
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

void nlist_del(nlist *l, nlist_node *n)
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

#endif

