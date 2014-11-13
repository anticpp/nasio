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

#define NLIST_INIT(l) l.head = l.tail = NULL;

void nlist_add_tail(nlist_t *l, nlist_node_t *n);

void nlist_del(nlist_t *l, nlist_node_t *n);

#endif

