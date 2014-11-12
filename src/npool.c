#include <stdlib.h>
#include "npool.h"

npool* npool_create(int elemsize, int size)
{
	npool *pool = (npool *)malloc( sizeof(npool) + elemsize*size );
	pool->elemsize = elemsize;
	pool->size = size;
	pool->unused = size;
	pool->free = (off_t *)(pool+1);

	int i = 0;
	off_t *obj = pool->free;
	for(; i<size-1; i++)
	{
		*obj = (off_t *)( (char*)obj+elemsize );
		obj = *obj;
	}
	*obj = NULL;//end of pool
}

void npool_destroy(npool *pool)
{
	free( pool );
}

char* npool_alloc(npool *pool)
{
	if( !pool->free )
		return NULL;
	char *obj = pool->free;
	pool->free = *(pool->free)
	--pool->unused;
	return obj;
}

int npool_free(npool *pool, char *element)
{
	*((off_t *)element) = pool->free;
	pool->free = (off_t *)element;
	++pool->unused;
	return 0;
}

#endif

