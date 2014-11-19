#include <string.h>
#include <stdlib.h>
#include "nbuffer.h"

nbuffer_t* nbuffer_create(int size)
{
	nbuffer_t *nbuf = (nbuffer_t *)malloc( sizeof(nbuffer_t) + size);
	if( !nbuf )
		return NULL;
	
	nbuf->buf = (char *)(nbuf+1);
	nbuf->pos = 0;
	nbuf->limit = size;
	nbuf->capacity = size;
	nbuf->mark = -1;
	return nbuf;
}

int nbuffer_put_buf(nbuffer_t *nbuf, const char *buf, int dlen)
{
	int realbytes = nbuffer_remain(nbuf);
	if( realbytes>dlen )
		realbytes = dlen;
	
	if( realbytes>0 )
	{
		memcpy( nbuf->buf+nbuf->pos, buf, realbytes );
		nbuf->pos+=realbytes;
	}
	return realbytes;
}

int nbuffer_get_buf(nbuffer_t *nbuf, char *buf, int dlen)
{
	int realbytes = nbuffer_remain(nbuf);
	if ( realbytes>dlen )
		realbytes = dlen;
	if( realbytes>0 )
	{
		memcpy( buf, nbuf->buf+nbuf->pos, realbytes );
		nbuf->pos+=realbytes;
	}
	return realbytes;
}

int nbuffer_compact(nbuffer_t *nbuf)
{
	int remain = nbuffer_remain(nbuf);
	if( nbuf->pos>0 && remain>0 )
		memmove( nbuf->buf, nbuf->buf+nbuf->pos, remain );
	nbuf->pos = remain;
	nbuf->limit = nbuf->capacity;
	nbuf->mark = -1;
	return remain;
}

void nbuffer_destroy(nbuffer_t *buffer)
{
	free( buffer );
}

