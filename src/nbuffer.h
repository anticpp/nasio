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
}while(0)\

/* compact before WRITE
 */
int nbuffer_compact(nbuffer_t *nbuf);

#define nbuffer_set_pos(b, pos)\
do{\
	if( (pos)<=(b)->limit )\
		(b)->pos=pos;\
}while(0)\

void nbuffer_destroy(nbuffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif
