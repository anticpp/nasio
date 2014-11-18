#ifndef _NASIO_NET_H_
#define _NASIO_NET_H_
/*
 * Simple api.
 *
 * Author: supergui@live.cn
 * Date: 2014-11-05
 *
 */
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int nasio_net_set_block(int fd, int block);

int nasio_net_get_local_addr(int fd, struct sockaddr *addr);

int nasio_net_get_remote_addr(int fd, struct sockaddr *addr);

#ifdef __cplusplus
}
#endif

#endif
