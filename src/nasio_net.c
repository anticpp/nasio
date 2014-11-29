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

int nasio_net_get_local_addr(int fd, nasio_inaddr_t *addr)
{
	int rv = 0;
	struct sockaddr_in in4addr;
	socklen_t len = sizeof(in4addr);
	rv = getsockname(fd, (struct sockaddr *)&in4addr, &len);
	if( rv<0 )
		return -1;
	nasio_net_convert_inaddr( addr, &in4addr );
	return 0;
}

int nasio_net_get_remote_addr(int fd, nasio_inaddr_t *addr)
{
	int rv = 0;
	struct sockaddr_in in4addr;
	socklen_t len = sizeof(in4addr);
	rv = getpeername(fd, (struct sockaddr *)&in4addr, &len);
	if( rv<0 )
		return -1;
	nasio_net_convert_inaddr( addr, &in4addr );
	return rv<0?-1:0;
}

int nasio_net_convert_inaddr(nasio_inaddr_t *to, struct sockaddr_in *from)
{
	to->addr = ntohl( from->sin_addr.s_addr );
	to->port = ntohs( from->sin_port );
	return 0;
}

const char* nasio_net_get_dot_addr(unsigned long addr)
{
	struct in_addr inaddr;
	inaddr.s_addr = htonl( addr );
	return inet_ntoa( inaddr );
}
