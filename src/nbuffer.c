#include <string.h>
#include <stdlib.h>
#include "nbuffer.h"

nbuffer_t* nbuffer_create(size_t size)
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

nbuffer_t* nbuffer_expand(nbuffer_t *buf)
{
	nbuffer_t old;
	old.pos = buf->pos;
	old.limit = buf->limit;
	old.capacity = buf->capacity;
	old.mark = buf->mark;

	size_t newsize = (old.capacity >> 2);
	nbuffer_t *newb = (nbuffer_t *)realloc( (void *)buf, sizeof(nbuffer_t) + newsize );
	if( !newb )
		return NULL;
	newb->buf = (char *)(newb+1);
	newb->pos = old.pos;
	newb->limit = newsize;
	newb->capacity = newsize;
	newb->mark = buf->mark;
	
	return newb;
}

ssize_t nbuffer_put_buf(nbuffer_t *nbuf, const char *buf, size_t dlen)
{
	ssize_t realbytes = nbuffer_remain(nbuf);
	if( realbytes<0 )
		return realbytes;
	if( (size_t)realbytes>dlen )
		realbytes = dlen;
	
	if( realbytes>0 )
	{
		memcpy( nbuf->buf+nbuf->pos, buf, realbytes );
		nbuf->pos+=realbytes;
	}
	return realbytes;
}

ssize_t nbuffer_get_buf(nbuffer_t *nbuf, char *buf, size_t dlen)
{
	ssize_t realbytes = nbuffer_remain(nbuf);
	if( realbytes<0 )
		return realbytes;
	if ( realbytes>dlen )
		realbytes = dlen;
	if( realbytes>0 )
	{
		memcpy( buf, nbuf->buf+nbuf->pos, realbytes );
		nbuf->pos+=realbytes;
	}
	return realbytes;
}

void nbuffer_compact(nbuffer_t *nbuf)
{
	ssize_t remain = nbuffer_remain(nbuf);
	if( nbuf->pos>0 && remain>0 )
		memmove( nbuf->buf, nbuf->buf+nbuf->pos, remain );
	nbuf->pos = remain;
	nbuf->limit = nbuf->capacity;
	nbuf->mark = -1;
	return ;
}

void nbuffer_destroy(nbuffer_t *buffer)
{
	free( buffer );
}

