#include <stdlib.h>
#include "npool.h"

npool_t* npool_create(int elemsize, int size)
{
	npool_t *pool = (npool_t *)malloc( sizeof(npool_t) + elemsize*size );
	pool->elemsize = elemsize;
	pool->size = size;
	pool->unused = size;
	pool->free_list = (npool_inner_head_t *)(pool+1);

	int i = 0;
	npool_inner_head_t *obj = pool->free_list;
	for(; i<size-1; i++)
	{
		obj->next = (npool_inner_head_t *)((char*)obj+elemsize);
		obj = obj->next;
	}
	obj->next = NULL; //end of pool
	return pool;
}

void npool_destroy(npool_t *pool)
{
	free( pool );
}

char* npool_alloc(npool_t *pool)
{
	if( !pool->free_list )
		return NULL;
	npool_inner_head_t *obj = pool->free_list;
	pool->free_list = pool->free_list->next;
	--(pool->unused);
	return (char *)obj;
}

int npool_free(npool_t *pool, char *element)
{
	npool_inner_head_t *obj = (npool_inner_head_t *)element;
	obj->next = pool->free_list;
	pool->free_list = obj;
	++(pool->unused);
	return 0;
}


