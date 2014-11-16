#ifndef _NASIO_NLIST_H_
#define _NASIO_NLIST_H_
/*
 * Two-way list.
 *
 * Author: supergui@live.cn
 * Date: 2014-11-05
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

typedef struct nlist_node_s nlist_node_t;

struct nlist_node_s
{
	nlist_node_t *prev;
	nlist_node_t *next;
};

typedef struct 
{
	nlist_node_t *head;
	nlist_node_t *tail;
}nlist_t;

#define nlist_init(l)\
do{\
	l->head = l->tail = NULL;\
}while(0)\

/*
 * l: list, should not be empty
 * p: insert after p
 * n: insert node
 */
void nlist_insert_after(nlist_t *l, nlist_node_t *p, nlist_node_t *n);

#define nlist_insert_tail(l, n)\
do{\
	n->next = n->prev = NULL;\
	if( !l->tail )\
		l->head = l->tail = n;\
	else\
		nlist_insert_after(l, l->tail, n);\
}while(0)\

void nlist_del(nlist_t *l, nlist_node_t *n);

#endif

