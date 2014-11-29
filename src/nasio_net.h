/**
 * @file nasio_net.h
 * @brief Simple socket api.
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef _NASIO_NET_H_
#define _NASIO_NET_H_

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned long addr;
	unsigned short port;
}nasio_inaddr_t;

/**
 * @brief set socket block
 *
 * @param fd
 * @param block - 0 unbolck
 * 		  1 block
 *
 * @return 0  succ
 * 	   <0 fail
 */
int nasio_net_set_block(int fd, int block);

/**
 * @brief set socket REUSE
 *
 * @param fd
 * @param val - 0 set REUSE off
 * 		1 set REUSE on
 *
 * @return 0 succ
 * 	   <0 fail
 */
int nasio_net_set_reuse(int fd, int val);

/**
 * @brief get socket local address
 *
 * @param fd
 * @param addr
 *
 * @return 0 succ
 * 	   <0 fail 
 */
int nasio_net_get_local_addr(int fd, nasio_inaddr_t *addr);

/**
 * @brief get socket remote address
 *
 * @param fd
 * @param addr
 *
 * @return 0 succ
 * 	   <0 fail
 */
int nasio_net_get_remote_addr(int fd, nasio_inaddr_t *addr);

/**
 * @brief
 *
 * @param to
 * @param from
 *
 * @return 0 succ
 * 	   <0 fail
 */
int nasio_net_convert_inaddr(nasio_inaddr_t *to, struct sockaddr_in *from);

/**
 * @brief convert to string ip like "192.168.1.1"
 *
 * @param addr
 *
 * @return NULL if fail
 */
const char* nasio_net_get_dot_addr(unsigned long addr);

#ifdef __cplusplus
}
#endif

#endif
