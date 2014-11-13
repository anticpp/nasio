#ifndef _NASIO_NPOOL_H_
#define _NASIO_NPOOL_H_

#include <sys/types.h>

/*
 * Object pool.
 *
 * Author: supergui@live.cn
 * Date: 2014-11-05
 *
 */

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

npool_t* npool_create(int elemsize, int size);

void npool_destroy(npool_t *pool);

char* npool_alloc(npool_t *pool);

int npool_free(npool_t *pool, char *element);

#endif

