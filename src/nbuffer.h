/**
 * @file nbuffer.h
 * @brief 
 * 	From JAVA.NIO.ByteBuffer.
 *
 * INIT:
 * +-----------------------------------+
 * |           BUFFER                  |
 * +-----------------------------------+
 * |                                   |
 * pos                                 limit
 *                                     capacity
 * (mark=-1)
 *
 * WRITE:
 * +-----------------------------------+
 * |///////|                           |
 * +-----------------------------------+
 *         |                           |
 *         pos                         limit
 *                                     capacity
 * (mark=-1)
 *
 * FLIP:
 * +-----------------------------------+
 * |///////|                           |
 * +-----------------------------------+
 * |       |                           |
 * pos     limit                       capacity
 * (mark=-1)
 *                                     
 * READ:
 * +-----------------------------------+
 * |   |/////|                         |
 * +-----------------------------------+
 *     |     |                         |
 *     pos   limit                     capacity
 * (mark=-1)
 *   
 * REWIND:
 * +-----------------------------------+
 * |   |/////|                         |
 * +-----------------------------------+
 *     |     |                         |
 *     mark  pos                       limit
 *                                     capacity
 * COMPACT:
 * +-----------------------------------+
 * |/////| <-- memove                  |
 * +-----------------------------------+
 *       |                             |
 *       pos                           limit
 *       			       capacity
 * (mark=-1)
 *
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef _NASIO_NBUFFER_H_
#define _NASIO_NBUFFER_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief require more buffer.
 * 	  if not enough, try compact first. 
 * 	  still not enough, try recursivly enlarge 2*capacity each time, until fits the require.
 *
 * @param buf
 *
 * @param size - 'size' more bytes require
 *
 * @return - 0 succ
 * 	   - <0 fail
 */
int nbuffer_require(nbuffer_t **pnbuf, size_t size);

/**
 * @brief copy a reference
 *
 * @param to
 * @param src
 *
 */
#define nbuffer_slice(to, src)\
	((to)->buf) = ((src)->buf);\
	((to)->pos) = ((src)->pos);\
	((to)->limit) = ((src)->limit);\
	((to)->mark) = ((src)->mark);\
	((to)->capacity) = ((src)->capacity);\

/**
 * @brief remain len of buffer
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_remain(b) ( ((b)->limit)-((b)->pos) )

/**
 * @brief put data to buffer
 *
 * @param nbuf
 * @param buf
 * @param dlen - max len to put
 *
 * @return - >=0 real bytes put
 * 	     <0  error
 */
ssize_t nbuffer_put_buf(nbuffer_t *nbuf, const char *buf, size_t dlen);

/**
 * @brief get data from buffer
 *
 * @param nbuf
 * @param buf
 * @param dlen
 *
 * @return - >=0 real bytes get
 * 	     <0 error
 */
ssize_t nbuffer_get_buf(nbuffer_t *nbuf, char *buf, size_t dlen);

/**
 * @brief flip before READ
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_flip(b)\
do{\
	(b)->limit = (b)->pos;\
	if( (b)->mark>=0 )\
		(b)->pos = (b)->mark;\
	else\
		(b)->pos = 0;\
	(b)->mark = -1;\
}while(0)\

/**
 * @brief 
 *
 * @param nbuf
 */
#define nbuffer_rewind(b)\
do{\
	if( (b)->pos>0 )\
		(b)->mark = (b)->pos;\
	(b)->pos = (b)->limit;\
	(b)->limit = (b)->capacity;\
}while(0)\

/** 
 * @brief compact before WRITE
 *	  in fact, this operation includes compact & rewind
 *
 * @param nbuf
 *
 * @return remain bytes
 */
void nbuffer_compact(nbuffer_t *nbuf);


/**
 * @brief set position.
 * 	  pos must be no larger than limit.
 * 	  if mark is defined and larger than new position, discard it.
 *
 * @param b
 * @param pos
 *
 * @return 
 */
#define nbuffer_set_pos(b, p)\
do{\
	if( (p)<=(b)->limit )\
	{\
		(b)->pos = p;\
		if( (b)->mark>p )\
			(b)->mark = -1;\
	}\
}while(0)\

/**
 * @brief set limit
 *
 * @param b
 * @param p
 *
 * @return 
 */
#define nbuffer_set_limit(b, p)\
do{\
	if( (p)>(b)->capacity )\
		break;\
	if( (b)->pos>(p) )\
		(b)->pos = (p);\
	if( (b)->mark>(p) )\
		(b)->mark = (p);\
	(b)->limit = (p);\
}while(0)\

/**
 * @brief 
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_clear(b)\
do{\
	(b)->pos = 0;\
	(b)->limit = (b)->capacity;\
	(b)->mark = -1;\
}while(0)\

/**
 * @brief destroy buffer
 *
 * @param buffer
 */
void nbuffer_destroy(nbuffer_t *nbuf);

#ifdef __cplusplus
}
#endif

#endif
