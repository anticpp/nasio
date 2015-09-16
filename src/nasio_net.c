#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "nasio_net.h"

int nasio_net_set_block(int fd, int block)
{
	int flag = fcntl(fd, F_GETFL);
	if( block && (flag & O_NONBLOCK) )
		fcntl(fd, F_SETFL, flag & (~O_NONBLOCK));
	else if( !(flag & O_NONBLOCK) )
		fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	return 0;
}

int nasio_net_set_reuse(int fd, int val)
{
	int rv = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if( rv<0 )
		return -1;
	return 0;
}

int nasio_net_get_local_addr(int fd, struct sockaddr_in *addr)
{
	socklen_t len = sizeof(struct sockaddr_in);
	return getsockname(fd, (struct sockaddr *)addr, &len);
}

int nasio_net_get_remote_addr(int fd, struct sockaddr_in *addr)
{
	socklen_t len = sizeof(struct sockaddr_in);
	return getpeername(fd, (struct sockaddr *)addr, &len);
}

const char* nasio_net_get_dot_addr(struct sockaddr_in *addr)
{
	return inet_ntoa( addr->sin_addr );
}
