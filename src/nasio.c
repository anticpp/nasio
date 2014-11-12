#include "nasio.h"
#include "nasoi_net.h"

#define MAX_BACKLOG 10000

struct nasio_listener
{
	int fd;
	struct sockaddr_in addr;
	ev_io watcher;

	nlist_node *list_node;
};

static 
void listener_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	
	ev_io_stop(EV_A_ w);
}

nasio_env* nasio_env_create(int capacity)
{
	nasio_env *env = (nasio*)malloc( sizeof(nasio_env) );
	if( !env )
		return NULL;

	env->loop = ev_loop_new( EVFLAG_AUTO );
	if( !env->loop )
		goto fail;

	env->conn_pool = npool_create( sizeof(asio_conn), capacity );
	if( !env->conn_pool )
		goto fail;

	NLIST_INIT( listen_list );
	NLIST_INIT( remote_list );
	NLIST_INIT( conn_list );

	return env;

fail:
	if( env )
		free( env );
	return NULL;
}

int nasio_env_destroy(nasio_env *env)
{
	ev_loop_destroy( env->loop );

	if( env->conn_pool )
		npool_destroy( env->conn_pool );

	free( env );

	return 0;
}

int nasio_add_listen(nasio_env *env
	, const char *ip
	, short port
	, nasio_conn_event_handler *handler)
{
	int rv = 0;
	nasio_listener *listener = (nasio_listener *)malloc( sizeof(nasoi_listener) );

	listener->fd = socket(PF_INET, SOCK_STREAM, 0);
	if( listener->fd<0 )
	{
		free( listener );
		return -1;
	}
	
	nasio_net_set_block(listener->fd, 0);//set nonblock

	memset(&(listener->addr), 0x00, sizeof(struct sockaddr_in));
	listener->addr.sin_family = PF_INET;
	listener->addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &(listener->addr));
	
	rv = bind(listener->fd, (struct sockaddr *)&(listener->addr), sizeof(struct sockaddr_in));
	if( rv<0 )
	{
		close(listener->fd);
		free( listener );
		return -1;
	}
	
	rv = listen(listener->fd, MAX_BACKLOG);
	if( rv<0 )
	{
		close(fd);
		return -1;
	}
	
	ev_io_init(&listener->watcher, listener_cb, 0, EV_READ);
	ev_io_start(env->loop, &(listener->watcher));

	nlist_add_tail(env->listen_list, listener->list_node);

	return 0;
}

int nasio_add_remote(nasio_env *env
	, const char *ip
	, short port
	, nasio_conn_event_handler *handler)
{
}

int nasio_start()
{

}
