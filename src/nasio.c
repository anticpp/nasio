#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include "nasio.h"
#include "nasio_net.h"

#ifndef offsetof
#define offsetof(s, e) (size_t)( &(((s *)0)->e) )
#endif

#define DEBUGINFO(...) printf(__VA_ARGS__);
#define MAX_BACKLOG 10000
#define ACCEPT_ONCE 5

#define listener_of(w) ( (nasio_listener_t *)((char *)w-offsetof(nasio_listener_t, watcher)) )
#define connection_of(w) ( (nasio_conn_t *)((char *)w-offsetof(nasio_conn_t, watcher)) )

#define error_not_ready() ( errno==EAGAIN || errno==EWOULDBLOCK )

#define new_conn_id(env) ( (++((env)->conn_id_gen)) % 0x00ffffffffffffff )

typedef struct
{
	int fd;
	nasio_inaddr_t addr;
	ev_io watcher;

	nlist_node_t list_node;
}nasio_listener_t;


static void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
//static void on_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents);

void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	int max = ACCEPT_ONCE;
	int cfd = 0;

	nasio_env_t *env = (nasio_env_t *)w->data;
	nasio_listener_t *listener = listener_of(w);
	struct sockaddr_in in4addr;
	socklen_t addrlen;
	while(max-->0)
	{
		cfd = accept( listener->fd, (struct sockaddr *)&in4addr, &addrlen);
		if( cfd<0 && error_not_ready() ) /* no pending */
		{
			break;	
		}
		else if( cfd<0 ) /* accept error */
		{
			break;
		}
		else
		{
			nasio_conn_t *newconn = nasio_conn_new(env, cfd);
			if( !newconn ) /* holy the shit */
			{
				close( cfd );
				break;
			}

			DEBUGINFO("accept connection local addr %s:%d, reomte addr %s:%d\n"
				, nasio_net_get_dot_addr(newconn->local_addr.addr)
				, newconn->local_addr.port
				, nasio_net_get_dot_addr(newconn->remote_addr.addr)
				, newconn->remote_addr.port);
			DEBUGINFO("now conn size %d\n", env->conn_list.size);
		}
	}
}

void on_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	ssize_t rbytes = 0;
	char buf[1024] = { 0x00 };
	nasio_conn_t *conn = connection_of(w);
	if( !conn->rbuf ) 
		conn->rbuf = nbuffer_create( 1024*1024*2 );
	
	rbytes = read( conn->fd, buf, sizeof(buf) );
	if( rbytes<0 && !error_not_ready() )
	{
		DEBUGINFO("read error, %d: %s\n", errno, strerror(errno));
		nasio_conn_close(conn);
	}
	else if ( rbytes==0 ) {
		DEBUGINFO("read eof, connection close by the other end\n");
		nasio_conn_close(conn);
	}
	else if( rbytes>0 )
	{
		DEBUGINFO("read %zd bytes from %d\n", rbytes, conn->fd);
		nbuffer_put_buf( conn->rbuf, buf, rbytes);
	}
}
/*void on_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{

}*/


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

	nlist_init( &(env->listen_list) );
	nlist_init( &(env->remote_list) );
	nlist_init( &(env->conn_list) );

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
	struct sockaddr_in in4addr;
	int rv = 0;
	nasio_listener_t *listener = (nasio_listener_t *)malloc( sizeof(nasio_listener_t) );
	listener->watcher.data = env;//attach env

	listener->fd = socket(AF_INET, SOCK_STREAM, 0);
	if( listener->fd<0 )
	{
		printf("create socket fail\n");
		free( listener );
		return -1;
	}
	
	nasio_net_set_block(listener->fd, 0);//set nonblock
	nasio_net_set_reuse(listener->fd, 1);//set reuse

	memset(&in4addr, 0x00, sizeof(struct sockaddr_in));
	in4addr.sin_family = AF_INET;
	in4addr.sin_port = htons(port);
	if ( strcmp(ip, "*")==0 )
		in4addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		in4addr.sin_addr.s_addr = inet_addr(ip);
	
	nasio_net_convert_inaddr( &(listener->addr), &in4addr );
	rv = bind(listener->fd, (struct sockaddr *)&in4addr, sizeof(struct sockaddr_in));
	if( rv<0 )
	{
		printf("bind address fail\n");
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

	nlist_insert_tail( &(env->listen_list), &(listener->list_node) );

	return 0;
}

int nasio_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler)
{
	return 0;
}

int nasio_run(nasio_env_t *env, int flag)
{
	ev_run(env->loop, flag);
	return 0;
}

nasio_conn_t* nasio_conn_new(nasio_env_t *env, int fd)
{
	/* allocate && init 
	 */
	nasio_conn_t *conn = (nasio_conn_t *)npool_alloc( env->conn_pool );
	if( !conn ) /* holy the shit */
	{
		DEBUGINFO("alloc newconn fail! connection pool available %d\n", npool_available(env->conn_pool));
		return NULL;
	}
	conn->env = env;
	conn->id = new_conn_id(env);
	conn->fd = fd;
	nasio_net_get_local_addr( fd, &(conn->local_addr) );
	nasio_net_get_remote_addr( fd, &(conn->remote_addr) );
	conn->rbuf = NULL;
	conn->sbuf = NULL;

	/* add to list
	 */
	nlist_insert_tail( &(env->conn_list), &(conn->list_node) );

	/* always care about READ, in case other end close connection.
	 */
	ev_io_init( &(conn->watcher), on_readable_cb, fd, EV_READ);
	ev_io_start( env->loop, &(conn->watcher) );
	return conn;
}
void nasio_conn_close(nasio_conn_t *conn)
{
	nasio_env_t *env = conn->env;
	ev_io_stop( env->loop, &conn->watcher );
	nlist_del( &(env->conn_list), &(conn->list_node) );
	if( conn->rbuf )
		nbuffer_destroy( conn->rbuf );
	if( conn->sbuf )
		nbuffer_destroy( conn->sbuf );
	close( conn->fd );
	npool_free( env->conn_pool, (char *)conn );

	DEBUGINFO("now conn size %d\n", env->conn_list.size);
}
