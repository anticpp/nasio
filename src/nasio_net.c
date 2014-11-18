#include <fcntl.h>
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

int nasio_net_get_local_addr(int fd, struct sockaddr *addr)
{
	int rv = 0;
	socklen_t len;
	rv = getsockname(fd, addr, &len);
	return rv<0?-1:0;
}

int nasio_net_get_remote_addr(int fd, struct sockaddr *addr)
{
	int rv = 0;
	socklen_t len;
	rv = getpeername(fd, addr, &len);
	return rv<0?-1:0;
}
