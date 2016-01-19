#include <string.h>
#include <stdlib.h>
#include "nbuffer.h"

#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

//static int nbuffer_hungry(nbuffer_t **pnbuf, size_t size);

nbuffer_t* nbuffer_create(size_t size) {
    nbuffer_t *nbuf = (nbuffer_t *)malloc(sizeof(nbuffer_t) + size);
    if( !nbuf )
        return NULL;

    nbuf->buf = (char *)(nbuf+1);
    nbuf->pos = 0;
    nbuf->limit = size;
    nbuf->capacity = size;
    nbuf->mark = -1;

    return nbuf;
}

ssize_t nbuffer_read(nbuffer_t *nbuf, char *buf, size_t size) {
    ssize_t rbytes = MIN((ssize_t)(nbuf->limit-nbuf->pos), (ssize_t)size);
    if( rbytes<=0 )
        return 0;
    
    memcpy(buf, nbuf->buf+nbuf->pos, rbytes);
    nbuf->pos += rbytes;
   
    return rbytes;
}

ssize_t nbuffer_write(nbuffer_t *nbuf, const char *buf, size_t size) {
    ssize_t wbytes = MIN((ssize_t)(nbuf->limit-nbuf->pos), (ssize_t)size);
    if( wbytes<=0 )
        return 0;
    
    memcpy(nbuf->buf+nbuf->pos, buf, wbytes);
    nbuf->pos += wbytes;

    return wbytes;
}

size_t nbuffer_digest( nbuffer_t *nbuf, size_t size ) {
    if( nbuf->pos+size>nbuf->limit )
        return 0;

    nbuf->pos += size;

    return size;
}

int nbuffer_compact(nbuffer_t *nbuf) {
    ssize_t data_size = nbuf->limit - nbuf->pos;

    /* compact if pos larger than 0, and there are remaining data.
     */
    if( nbuf->pos>0 && data_size>0 ) { 
        memmove(nbuf->buf, nbuf->buf+nbuf->pos, data_size);
    }

    nbuf->pos = data_size;
    nbuf->limit = nbuf->capacity;
    nbuf->mark = -1;

    return 0;
}

#if 0
ssize_t nbuffer_reserve(nbuffer_t **pnbuf, size_t size)
{
    nbuffer_t *nbuf = *pnbuf;
    nbuffer_t old;
    nbuffer_t *newb = NULL;
    int count = 10;
    size_t newsize = 0;
    ssize_t n = 0;
    ssize_t remain = nbuf->limit - nbuf->pos;

    /* try memove
     */
    if( remain<(ssize_t)size && nbuf->last_pos>0 ){
        n = nbuf->pos - nbuf->last_pos;
        if( n>0 ) {
            memmove(nbuf->buf, nbuf->buf+nbuf->last_pos, n);
            if( nbuf->mark>=0 )
                nbuf->mark -= nbuf->last_pos;
            nbuf->last_pos = 0;
            nbuf->pos = n;
        } else if( n==0 ){
            if( nbuf->mark>=0 )
                nbuf->mark -= nbuf->last_pos;
            nbuf->last_pos = 0;
            nbuf->pos = 0;
        } else { //would never happen
            return -1;
        }
    }
    
    /**
     * try realloc, max capacity*2^10
     */
    remain = nbuf->limit - nbuf->pos;
    if( remain<(ssize_t)size ) {
        do {
            newsize = nbuf->capacity<<1;
        } while(--count>0 && newsize-nbuf->pos<size);

        if( newsize-nbuf->pos>=size ) {
            old = *nbuf;
            newb = (nbuffer_t *)realloc( (void *)nbuf, sizeof(nbuffer_t) + newsize );
            if( !newb )
                return -1;
            newb->buf = (char *)(newb+1);
            newb->pos = old.pos;
            newb->limit = newsize;
            newb->capacity = newsize;
            newb->mark = old.mark;
            newb->last_pos = old.last_pos;

            *pnbuf = newb;
            nbuf = *pnbuf;
        } else {//reallocate fail
            return -1;
        }
    }
    return 0;
}
#endif
void nbuffer_destroy(nbuffer_t *nbuf) {
    free( nbuf );
}

#ifdef MIN
#undef MIN
#endif
