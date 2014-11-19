#ifndef _NASIO_NBUFFER_H_
/*
 * Defination of nbuffer. From JAVA.NIO.ByteBuffer.
 *
 * INIT:
 * +-----------------------------------+
 * |           BUFFER                  |
 * +-----------------------------------+
 * |                                   |
 * pos                                 limit
 *                                     capacity
 *
 * WRITE:
 * +-----------------------------------+
 * |///////|                           |
 * +-----------------------------------+
 *         |                           |
 *         pos                         limit
 *                                     capacity
 *
 * FLIP:
 * +-----------------------------------+
 * |///////|                           |
 * +-----------------------------------+
 * |       |                           |
 * pos     limit                       capacity
 *                                     
 *
 * READ:
 * +-----------------------------------+
 * |   |///|                           |
 * +-----------------------------------+
 *     |   |                           |
 *     pos limit                       capacity
 *   
 *                                  
 * COMPACT:
 * +-----------------------------------+
 * |///| <--memmove                    |
 * +-----------------------------------+
 *     |                               |
 *     pos                             limit
 *                                     capacity
 *
 * Author: supergui@live.cn
 * Date: 2014-11-05
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	char *buf;
	int pos;
	int limit;
	int mark;

	int capacity;
}nbuffer_t;

nbuffer_t* nbuffer_create(int size);

#define nbuffer_remain(b) ( ((b)->limit)-((b)->pos) )

/* RETURN - real bytes put
 */
int nbuffer_put_buf(nbuffer_t *nbuf, const char *buf, int dlen);

/* RETURN - real bytes get
 */
int nbuffer_get_buf(nbuffer_t *nbuf, char *buf, int dlen);

/* flip before READ
 */
#define nbuffer_flip(b)\
do{\
	(b)->limit = (b)->pos;\
	(b)->pos = 0;\
	(b)->mark = -1;\
}while(0)\

/* compact before WRITE
 * in fact, this operation includes compact & rewind.
 */
int nbuffer_compact(nbuffer_t *nbuf);

/* set position.
 * pos must be no larger than limit.
 * if mark is defined and larger than new position, discard it.
 */
#define nbuffer_set_pos(b, pos)\
do{\
	if( (pos)<=(b)->limit )\
	{\
		(b)->pos = pos;\
		if( (b)->mark>pos )\
			(b)->mark = -1;\
	}\
}while(0)\

/* mark the current position.
 */
#define nbuffer_mark(b)	( (b)->mark=(b)->pos )

/* reset the position to previous-marked position.
 */
#define nbuffer_reset(b)\
do{\
	if((b)->mark!=-1)\
		(b)->pos=(b)->mark;\
}while(0)\

#define nbuffer_clear(b)\
do{\
	(b)->pos = 0;\
	(b)->limit = (b)->capacity;\
	(b)->mark = -1;\
}while(0)\

void nbuffer_destroy(nbuffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif
