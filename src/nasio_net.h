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

typedef struct
{
	unsigned int addr;
	unsigned short port;
}nasio_inaddr_t;

int nasio_net_set_block(int fd, int block);

int nasio_net_set_reuse(int fd, int val);

int nasio_net_get_local_addr(int fd, nasio_inaddr_t *addr);

int nasio_net_get_remote_addr(int fd, nasio_inaddr_t *addr);

int nasio_net_convert_inaddr(nasio_inaddr_t *to, struct sockaddr_in *from);

const char* nasio_net_get_dot_addr(unsigned int addr);

#ifdef __cplusplus
}
#endif

#endif
