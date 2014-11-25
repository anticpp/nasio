/**
 * @file nlist.h
 * @brief 
 * 	Two-way list.
 *
 *              HEAD                            TAIL 
 *               |                               |
 *              \|/                             \|/
 *             ----------     ----------     ----------
 *   NULL<-----|NODE    |---->|NODE    |<----|NODE    |---->NULL
 *             |        |<----|        |---->|        |
 *             ----------     ----------     ----------
 *
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-24
 *
 */
#ifndef _NASIO_NLIST_H_
#define _NASIO_NLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @brief init list
 *
 * @param l - target list
 *
 * @return 
 */
#define nlist_init(l)\
do{\
	(l)->head = (l)->tail = NULL;\
}while(0)\

/**
 * @brief 
 *
 * @param l - target list
 *
 * @return true empty
 * 	   false not empty
 */
#define nlist_isempty(l) ( (l)->head==NULL && (l)->tail==NULL )

/**
 * @brief insert node after in-node
 *
 * @param l - target list
 * @param p - insert after 'p'
 * @param n - node to be inserted
 */
void nlist_insert_after(nlist_t *l, nlist_node_t *p, nlist_node_t *n);

/**
 * @brief insert node before in-node
 *
 * @param l - target list
 * @param p - insert before 'p'
 * @param n - node to be inserted
 */
void nlist_insert_before(nlist_t *l, nlist_node_t *p, nlist_node_t *n);

/**
 * @brief insert node at tail
 *
 * @param l - target list
 * @param n - node to be inserted
 *
 * @return 
 */
#define nlist_insert_tail(l, n)	nlist_insert_after(l, (l)->tail, n);

/**
 * @brief  insert node at head
 *
 * @param l - target list
 * @param n - node to be inserted
 *
 * @return 
 */
#define nlist_insert_head(l, n) nlist_insert_before(l, (l)->head, n)

/**
 * @brief delete node from list
 *
 * @param l - target list
 * @param n - node to be deleted
 *
 * @return 
 */
void nlist_del(nlist_t *l, nlist_node_t *n);

/**
 * @brief delete head node
 *
 * @param l - target list
 *
 * @return 
 */
#define nlist_del_head(l)\
do{\
	if((l)->head)\
		nlist_del(l, (l)->head);\
}while(0)\


/**
 * @brief delete tail node
 *
 * @param l - target list
 *
 * @return 
 */
#define nlist_del_tail(l)\
do{\
	if((l)->tail)\
		nlist_del(l, (l)->tail);\
}while(0)\

#ifdef __cplusplus
}
#endif

#endif

