#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "nasio.h"
#include "nasio_net.h"

#define MAX_BACKLOG 10000
#define ACCEPT_ONCE 5

#define LISTENER_OF(io) ( (nasio_listener_t *)((char *)w-offsetof(nasio_listener_t, watcher)) )

#define ERROR_NOT_READY() ( errno==EAGAIN || errno==EWOULDBLOCK )

#define GEN_CONN_ID(env) ( (++((env)->conn_id_gen)) % 0x00ffffffffffffff )

typedef struct
{
	int fd;
	struct sockaddr_in addr;
	ev_io watcher;

	nlist_node_t list_node;
}nasio_listener_t;

static void on_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents);

void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	int max = ACCEPT_ONCE;
	int cfd = 0;

	nasio_env_t *env = (nasio_env_t *)w->data;
	nasio_listener_t *listener = LISTENER_OF(w);
	struct sockaddr_in addr;
	socklen_t addrlen;
	while(max-->0)
	{
		cfd = accept( listener->fd, (struct sockaddr *)&addr, &addrlen);
		if( cfd<0 && ERROR_NOT_READY() ) //no pending
		{
			break;	
		}
		else if( cfd<0 ) //accept error
		{
			break;
		}
		else
		{
			nasio_conn_t *newconn = (nasio_conn_t *)npool_alloc( env->conn_pool );
			if( newconn ) //holy shit
			{
				close( cfd );
				break;
			}
			newconn->id = GEN_CONN_ID(env);
			newconn->fd = cfd;
			memcpy( &(newconn->addr), &addr, sizeof(struct sockaddr) );

			ev_io_init( &(newconn->watcher), on_readable_cb, newconn->fd, EV_READ);
			ev_io_start( env->loop, &(newconn->watcher) );

			nlist_insert_tail( &env->conn_list, &newconn->list_node );
		}
	}
}

void on_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{

}
void on_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{

}

nasio_env_t* nasio_env_create(int capacity)
{
	nasio_env_t *env = (nasio_env_t *)malloc( sizeof(nasio_env_t) );
	if( !env )
		return NULL;

	env->loop = ev_loop_new( EVFLAG_AUTO );
	if( !env->loop )
		goto fail;

	env->conn_pool = npool_create( sizeof(nasio_conn_t), capacity );
	if( !env->conn_pool )
		goto fail;

	nlist_init( &env->listen_list );
	nlist_init( &env->remote_list );
	nlist_init( &env->conn_list );

	env->conn_id_gen = 0;

	return env;

fail:
	if( env )
		free( env );
	return NULL;
}

int nasio_env_destroy(nasio_env_t *env)
{
	ev_loop_destroy( env->loop );

	if( env->conn_pool )
		npool_destroy( env->conn_pool );

	free( env );

	return 0;
}

int nasio_add_listen(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler)
{
	int rv = 0;
	nasio_listener_t *listener = (nasio_listener_t *)malloc( sizeof(nasio_listener_t) );
	listener->watcher.data = env;//attach env

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
		close(listener->fd);
		return -1;
	}
	
	ev_io_init(&listener->watcher, on_listener_cb, listener->fd, EV_READ);
	ev_io_start(env->loop, &(listener->watcher));

	nlist_insert_tail( &env->listen_list, &listener->list_node );

	return 0;
}

int nasio_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler)
{
	return 0;
}

