#include <fcntl.h>
#include "nasio_net.h"

int nasio_net_set_block(int fd, int block)
{
	int flag = fcntl(fd, F_GETFL);
	if( block && (flag & O_NONBLOCK) )
		fcntl(fd, F_SETFL, flag & (~O_NONBLOCK));
	else if( !(flag & O_NONBLOCK) )
		fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

#endif
