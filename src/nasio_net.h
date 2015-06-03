/**
 * @file nasio_net.h
 * @brief Simple socket api.
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef NASIO_NET_H_
#define NASIO_NET_H_

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

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
int nasio_net_get_local_addr(int fd, struct sockaddr_in *addr);

/**
 * @brief get socket remote address
 *
 * @param fd
 * @param addr
 *
 * @return 0 succ
 * 	   <0 fail
 */
int nasio_net_get_remote_addr(int fd, struct sockaddr_in *addr);

/**
 * @brief - simply call inet_ntoa
 *          the string return resides in a static memory area, which is not reentrant.
 *
 * @param addr
 *
 * @return NULL if fail
 */
const char* nasio_net_get_dot_addr(struct sockaddr_in addr);

#ifdef __cplusplus
}
#endif

#endif
