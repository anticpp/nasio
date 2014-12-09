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
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef _NASIO_NBUFFER_H_
#define _NASIO_NBUFFER_H_

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

/**
 * @brief create nbuffer
 *
 * @param size - buffer size
 *
 * @return - NULL if fail
 */
nbuffer_t* nbuffer_create(int size);


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
 * @return - real bytes put
 */
int nbuffer_put_buf(nbuffer_t *nbuf, const char *buf, int dlen);

/**
 * @brief get data from buffer
 *
 * @param nbuf
 * @param buf
 * @param dlen
 *
 * @return real bytes get
 */
int nbuffer_get_buf(nbuffer_t *nbuf, char *buf, int dlen);

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
	(b)->pos = 0;\
	(b)->mark = -1;\
}while(0)\

/** 
 * @brief compact before WRITE
 *	  in fact, this operation includes compact & rewind
 *
 * @param nbuf
 *
 * @return 
 */
int nbuffer_compact(nbuffer_t *nbuf);

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
 * @brief mark the current position.
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_mark(b)	( (b)->mark=(b)->pos )

/**
 * @brief reset the position to previous-marked position.
 *
 * @param b
 *
 * @return 
 */
#define nbuffer_reset(b)\
do{\
	if((b)->mark!=-1)\
		(b)->pos=(b)->mark;\
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
void nbuffer_destroy(nbuffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif
