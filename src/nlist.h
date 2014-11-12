#ifndef _NASIO_NLIST_H_
#define _NASIO_NLIST_H_
/*
 * Two-way list.
 *
 *              HEAD                            TAIL 
 *               |                               |
 *              \|/                             \|/
 *             ----------     ----------     ----------
 *   NULL<-----|NODE    |---->|NODE    |<----|NODE    |---->NULL
 *             |        |<----|        |---->|        |
 *             ----------     ----------     ----------
 *
 */

struct nlist_node
{
	nlist_node *prev;
	nlist_node *next;
};

struct nlist
{
	nlist_node *first;
	nlist_node *tail;
};

#define NLIST_INIT(l) l->first = l->tail = NULL;

void nlist_add_tail(nlist *l, nlist_node *n);

void nlist_del(nlist *l, nlist_node *n);

#endif

