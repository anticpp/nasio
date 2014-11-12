#ifndef _NASIO_NPOOL_H_
#define _NASIO_NPOOL_H_

struct npool
{
	int elemsize;
	int size;
	int unused;

	/*
	 * Why not use N index, which saves 4 bytes?
	 * Maybe we will create different lists, and attach together.
	 */
	off_t *free; 
};

npool* npool_create(int elemsize, int size);

void npool_destroy(npool *pool);

char* npool_alloc(npool *pool);

int npool_free(npool *pool, char *element);

#endif

