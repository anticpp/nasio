/**
 * @file npool.h
 * @brief 
 * 	Object pool.
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-24
 */
#ifndef NASIO_NPOOL_H_
#define NASIO_NPOOL_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct npool_inner_head_s npool_inner_head_t;
struct npool_inner_head_s
{
	npool_inner_head_t *next;
};

typedef struct
{
	int elemsize;
	int size;
	int unused;

	npool_inner_head_t *free_list;
}npool_t;

/**
 * @brief create pool
 *
 * @param elemsize - element size
 * @param size - n elements
 *
 * @return NULL if fail
 */
npool_t* npool_create(int elemsize, int size);

/**
 * @brief available of pool
 *
 * @param pool
 *
 * @return 
 */
#define npool_available(pool) ((pool)->unused)

/**
 * @brief destroy pool
 *
 * @param pool
 */
void npool_destroy(npool_t *pool);

/**
 * @brief allocate new node from pool
 *
 * @param pool
 *
 * @return NULL if fail
 */
char* npool_alloc(npool_t *pool);

/**
 * @brief deallocate node 
 *
 * @param pool
 * @param element
 *
 * @return 0 succ
 */
int npool_free(npool_t *pool, char *element);


#ifdef __cplusplus
}
#endif

#endif

