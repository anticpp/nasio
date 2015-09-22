/**
 * @file nbuffer.h
 * @brief 
 * 	simple implementation of JAVA.NIO.ByteBuffer.
 *
 * @example
 * 
 * 	nbuffer_t *nbuf = nbuffer_create( 1024 );
 * 	nbuffer_clear( nbuf ); //clear buffer
 *
 * 	nbuffer_write( nbuf, "hello world", 5 ); //write some data 
 * 	nbuffer_flip( nbuf ); //flip, then you can read 
 * 	nbuffer_read( nbuf, tmp, 5 ); //read some data
 * 	nbuffer_compact( nbuf ); //compact, then you can write again
 * 	nbuffer_write( nbuf, "world", 5)// continue writting
 *
 * CREATE:
 * +-----------------------------------+
 * |           BUFFER                  |
 * +-----------------------------------+
 * |                                   |
 * pos                                 limit
 *                                     capacity
 *
 * WRITE:
 * +-----------------------------------+
 * |////////////|                      |
 * +-----------------------------------+
 *              |                      |
 * 		      pos                  limit
 *
 * FLIP:
 * +-----------------------------------+
 * |////////////|                      |
 * +-----------------------------------+
 * |            |                      |
 * pos          limit                  capacity
 * (limit=pos, pos=0, mark=-1)
 *                                     
 * READ:
 * +-----------------------------------+
 * |   |////////|                      |
 * +-----------------------------------+
 *     |        |                      |
 *     pos      limit                  capacity
 *   
 * COMPACT:
 * +-----------------------------------+
 * |////////|<--(memmove)               |
 * +-----------------------------------+
 *          |                          |
 *          pos                        limit
 *       	                           capacity
 * (pos=limit, limit=capacity, mark=-1)
 *
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef NASIO_NBUFFER_H_
#define NASIO_NBUFFER_H_

#include <stdlib.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief 
  *     Defination of nbuffer_t. 
  *     Howerver, you should always do operating throught methods, instead of operating the object directly.
  */
typedef struct
{
	char *buf;
	size_t pos;
	size_t limit;
	size_t capacity;

	ssize_t mark;
}nbuffer_t;

/**
 * @brief create nbuffer
 *
 * @param size - buffer size
 *
 * @return - NULL if fail
 */
nbuffer_t* nbuffer_create(size_t size);

/**
 * @brief read from nbuf max 'size' bytes to buf
 *
 * @param nbuf
 * @param buf
 * @param size
 *
 * @return >=0 the number of bytes actually read
 *         <0  error
 */
ssize_t nbuffer_read(nbuffer_t *nbuf, char *buf, size_t size);

/**
 * @brief 
 *
 * @param nbuf
 * @param buf
 * @param size
 *
 * @return >=0 the number of bytes actually written
 *         <0  error
 */
ssize_t nbuffer_write(nbuffer_t *nbuf, const char *buf, size_t size);

#if 0
/**
 * @brief reserve memory size
 *
 * @param pnbuf
 * @param size
 *
 * @return >=0 real bytes write
 *         <0 fail
 */
ssize_t nbuffer_reserve(nbuffer_t **pnbuf, size_t size);
#endif 

/**
 * @brief 
 *      Digest size bytes of data, which set pos+=size, and return the address.
 *      pos+size should no larger than current limit.
 *
 * @param nbuf
 * @param bytes
 *
 * @return 
 *      The number of bytes actually digested.
 */
size_t nbuffer_digest( nbuffer_t *nbuf, size_t size );

/**
 * @brief 
 *      Get the buffer's capacity.
 */
#define nbuffer_get_capacity(b) ( (b)->capacity )

/**
 * @brief 
 *      Get the buffer's position.
 */
#define nbuffer_get_pos(b) ( (b)->pos )

/**
 * @brief 
 *      Sets this buffer's position. 
 *      If the mark is defined and larger than the new position then it is discarded.
 *      The new position value; must be non-negative and no larger than the current limit.
 */
#define nbuffer_set_pos(b, p)\
do{\
	if( (p)<=(b)->limit ) {\
		(b)->pos = (p);\
		if( (b)->mark>(ssize_t)(p) )\
			(b)->mark = -1;\
	}\
}while(0)\

/**
 * @brief 
 *      Get the buffer's limit.
 */
#define nbuffer_get_limit(b) ((b)->limit)

/**
 * @brief 
 *      Sets this buffer's limit. If the position is larger than the new limit then it is set to the new limit. If the mark is defined and larger than the new limit then it is discarded.
 *
 */
#define nbuffer_set_limit(b, p)\
do{\
    (b)->limit = (p);\
    if( (b)->pos>=(p) ) {\
        (b)->pos=(p);\
    }\
    if( (b)->mark>=(ssize_t)(p) ) {\
        (b)->mark = -1;\
    }\
}while(0)\

/**
 * @brief 
 *      Sets this buffer's mark at its position.
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_mark(b)\
do{\
    (b)->mark = (b)->pos;\
}while(0)\

/**
 * @brief 
 *      Resets this buffer's position to the previously-marked position.
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_reset(b)\
do{\
    if( (b)->mark>=0 )\
        (b)->pos = (b)->mark;\
}while(0)\

/**
 * @brief 
 *      Clears this buffer. 
 *      The position is set to zero, the limit is set to the capacity, and the mark is discarded.
 */
#define nbuffer_clear(b)\
do{\
    (b)->pos = 0;\
    (b)->limit = (b)->capacity;\
    (b)->mark = -1;\
}while(0)\

#define nbuffer_get_remaining(b) ( (b)->limit - (b)->pos )

#define nbuffer_has_remaining(b) ( nbuffer_get_remaining((b))>0 )

/**
 * @brief 
 *      Flips this buffer. 
 *      The limit is set to the current position and then the position is set to zero. 
 *      If the mark is defined then it is discarded.
 */
#define nbuffer_flip(b)\
do{\
	(b)->limit = (b)->pos;\
	(b)->pos = 0;\
	(b)->mark = -1;\
}while(0)\

/**
 * @brief
 *      Rewinds this buffer. 
 *      The position is set to zero and the mark is discarded.
 */
#define nbuffer_rewind(b)\
do{\
    (b)->pos = 0;\
	(b)->mark = -1;\
}while(0)\

/** 
 * @brief 
 *      Compacts this buffer  (optional operation).
 *      The bytes between the buffer's current position and its limit, if any, are copied to the beginning of the buffer. That is, the byte at index p = position() is copied to index zero, the byte at index p + 1 is copied to index one, and so forth until the byte at index limit() - 1 is copied to index n = limit() - 1 - p. The buffer's position is then set to n+1 and its limit is set to its capacity. The mark, if defined, is discarded.
 *
 *      The buffer's position is set to the number of bytes copied, rather than to zero, so that an invocation of this method can be followed immediately by an invocation of another relative put method.
 *
 * @return 0 - success
 *         <0 - fail
 */
int nbuffer_compact(nbuffer_t *nbuf);

/**
 * @brief 
 *      destroy buffer
 *
 */
void nbuffer_destroy(nbuffer_t *nbuf);

#ifdef __cplusplus
}
#endif

#endif
