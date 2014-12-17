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

int nbuffer_require(nbuffer_t **pnbuf, size_t size)
{
	int try = 0;
	size_t newsize = 0;
	nbuffer_t old;
	nbuffer_t *newb = NULL;
	nbuffer_t *nbuf = *pnbuf;

	if( nbuffer_remain(nbuf)>=size )
		return 0;

	/* try compact
	 */
	nbuffer_compact( nbuf );
	if( nbuffer_remain(nbuf)>=size )
		return 0;

	old.pos = nbuf->pos;
	old.limit = nbuf->limit;
	old.capacity = nbuf->capacity;
	old.mark = nbuf->mark;

	for( ; try<10; try++ )//max 2^10
	{
		newsize = (nbuf->capacity<<1);
		if( newsize-nbuf->pos>=size )
			break;
	}
	if( newsize-nbuf->pos<size )
		return -1;

	newb = (nbuffer_t *)realloc( (void *)nbuf, sizeof(nbuffer_t) + newsize );
	if( !newb )
		return -1;
	newb->buf = (char *)(newb+1);
	newb->pos = old.pos;
	newb->limit = newsize;
	newb->capacity = newsize;
	newb->mark = old.mark;

	*pnbuf = newb;

	return 0;
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
	nbuffer_flip( nbuf );//flip to READ mode
	ssize_t remain = nbuffer_remain(nbuf);
	if( nbuf->pos>0 && remain>0 )
		memmove( nbuf->buf, nbuf->buf+nbuf->pos, remain );
	nbuf->pos = 0;
	nbuf->limit = remain;
	nbuf->mark = -1;
	nbuffer_rewind( nbuf );//rewind back to WRITE mode
	return ;
}

void nbuffer_destroy(nbuffer_t *nbuf)
{
	free( nbuf );
}

