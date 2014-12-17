#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include "nasio.h"
#include "nasio_net.h"

#define DEBUGINFO(...) printf(__VA_ARGS__);
#define MAX_BACKLOG 10000
#define ACCEPT_ONCE 5

#ifndef offsetof
#define offsetof(s, e) (size_t)( &(((s *)0)->e) )
#endif

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

	nasio_conn_cmd_factory_t *factory;
	nasio_conn_event_handler_t *handler;
}nasio_listener_t;

typedef struct 
{
	int fd;
	nasio_inaddr_t addr;
	ev_io watcher;

	nlist_node_t list_node;
	nasio_conn_cmd_factory_t *factory;
	nasio_conn_event_handler_t *handler;
}nasio_connector_t;

static void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_event_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static nasio_conn_t* nasio_conn_new(nasio_env_t *env, int fd);

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
			newconn->factory = listener->factory;
			newconn->handler = listener->handler;

			DEBUGINFO("accept connection local addr %s:%d, reomte addr %s:%d\n"
				, nasio_net_get_dot_addr(newconn->local_addr.addr)
				, newconn->local_addr.port
				, nasio_net_get_dot_addr(newconn->remote_addr.addr)
				, newconn->remote_addr.port);
			DEBUGINFO("now conn size %d\n", env->conn_list.size);

			if( newconn->handler && newconn->handler->on_connect )
				newconn->handler->on_connect( newconn );
		}
	}
}

void on_fd_event_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	nasio_conn_t *conn = connection_of(w);
	if( revents & EV_READ )
		on_fd_readable_cb( loop, w, revents );
	else if( revents & EV_WRITE )
		on_fd_writable_cb( loop, w, revents );
	else if( revents & EV_ERROR )
	{
		DEBUGINFO("fd event error, close it now, fd %d\n", conn->fd);
		nasio_conn_close( conn );
	}
}

void on_fd_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	size_t hungry = 4*1024;
	ssize_t rbytes = 0;
	nasio_conn_t *conn = connection_of(w);
	if( !conn->rbuf ) 
		conn->rbuf = nbuffer_create( 8*1024 );//8KB
	if( !conn->rbuf )
	{
		DEBUGINFO("create read nbuffer fail, close connection, fd %d\n", conn->fd);
		nasio_conn_close( conn );
		return;
	}

	if( nbuffer_require( &(conn->rbuf), hungry )<0 )
	{
		DEBUGINFO("nbuffer require %lu bytes fail, now size %lu. close connection %d now.\n", hungry, conn->rbuf->capacity, conn->fd);
		nasio_conn_close(conn);
		return ;
	}

	rbytes = read( conn->fd, conn->rbuf->buf+conn->rbuf->pos, hungry );
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
		nbuffer_set_pos( conn->rbuf, conn->rbuf->pos+rbytes );

		nbuffer_flip( conn->rbuf );
		ssize_t expect = conn->factory->frame(conn->rbuf);
		if( expect<0 ) //error
		{
			DEBUGINFO("factory::frame error, fd %d\n", conn->fd);
			nasio_conn_close(conn);
			return;
		}
		else if( nbuffer_remain(conn->rbuf)>=expect )
		{
			nbuffer_t tbuf;
			nbuffer_slice( &tbuf, conn->rbuf );
			nbuffer_set_limit( &tbuf, conn->rbuf->pos+expect );
			if( conn->handler && conn->handler->on_process )
				conn->handler->on_process( conn, &tbuf );

			nbuffer_set_pos( conn->rbuf, conn->rbuf->pos+expect );
		}
		nbuffer_rewind( conn->rbuf );
	}
}
void on_fd_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	nasio_conn_t *conn = connection_of(w);
	nasio_env_t *env = conn->env;
	ev_io *watcher = &(conn->watcher);
	size_t wbytes = 0;
	ssize_t realwbytes = 0;
	size_t hungry = 4*1024;//most 4KB once
	ssize_t remain = 0;

	nbuffer_flip( conn->wbuf );
	printf("on_fd_writable_cb, fd %d, remain %lu\n", conn->fd, nbuffer_remain(conn->wbuf) );
	wbytes = nbuffer_remain( conn->wbuf );
	if( wbytes>=hungry )
		wbytes=hungry;
	realwbytes = write( conn->fd, conn->wbuf->buf+conn->wbuf->pos, wbytes );
	if( realwbytes<0 && !error_not_ready() )
	{
		DEBUGINFO("write error, fd %d,  %d %s\n", conn->fd, errno, strerror(errno));
		nasio_conn_close(conn);
		return;
	}
	else if( realwbytes>0 )
	{
		nbuffer_set_pos( conn->wbuf, conn->wbuf->pos+realwbytes );
	}
	remain = nbuffer_remain( conn->wbuf );
	nbuffer_rewind( conn->wbuf );

	//remove write
	if( remain<=0 )
	{
		DEBUGINFO("write out all buffer, fd %d\n", conn->fd);
		ev_io_stop( env->loop, watcher );
		ev_io_set( watcher, conn->fd, watcher->events & (~EV_WRITE) );
		ev_io_start( env->loop, watcher );
	}
}

nasio_conn_t* nasio_conn_new(nasio_env_t *env, int fd)
{
	/* allocate && init 
	 */
	nasio_conn_t *conn = (nasio_conn_t *)npool_alloc( env->conn_pool );
	memset((char*)conn, 0x00, sizeof(nasio_conn_t));
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
	conn->wbuf = NULL;

	/* add to list
	 */
	nlist_insert_tail( &(env->conn_list), &(conn->list_node) );

	/* always care about READ, in case other end close connection.
	 */
	ev_io_init( &(conn->watcher), on_fd_event_cb, fd, EV_READ);
	ev_io_start( env->loop, &(conn->watcher) );
	return conn;
}

/* user interface */

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

int nasio_env_add_listen(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_cmd_factory_t *factory
	, nasio_conn_event_handler_t *handler)
{
	int rv = 0;
	struct sockaddr_in in4addr;
	nasio_listener_t *listener = (nasio_listener_t *)malloc( sizeof(nasio_listener_t) );
	if( !listener )
		return -1;
	listener->watcher.data = env;//attach env
	listener->factory = factory;
	listener->handler = handler;

	listener->fd = socket(AF_INET, SOCK_STREAM, 0);
	if( listener->fd<0 )
	{
		DEBUGINFO("create socket fail\n");
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
		DEBUGINFO("bind address fail\n");
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

#if 0
int nasio_env_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_cmd_factory_t *factory
	, nasio_conn_event_handler_t *handler)
{
	nasio_connector_t *connector = (nasio_connector_t *)malloc( sizeof(nasio_connector_t) );
	return 0;
}
#endif

int nasio_env_run(nasio_env_t *env, int flag)
{
	ev_run(env->loop, flag);
	return 0;
}


int nasio_conn_write_buffer(nasio_conn_t *conn, const char *buf, size_t len)
{
	DEBUGINFO("connection write buffer, fd %d, size %zu\n", conn->fd, len);
	size_t wbytes = 0;
	nasio_env_t *env = conn->env;
	ev_io* watcher = &(conn->watcher);

	if( !conn->wbuf )
		conn->wbuf = nbuffer_create( 8*1024 );//4KB
	if( !conn->wbuf )
	{
		DEBUGINFO("create write nbuffer fail, close connection, fd %d\n", conn->fd);
		nasio_conn_close( conn );
		return -1;
	}
	if( nbuffer_require( &(conn->wbuf), len )<0 )
	{
		DEBUGINFO("require nbuffer fail, size %lu, now capacity %lu. close connection, fd %d\n", len, conn->wbuf->capacity, conn->fd);
		nasio_conn_close( conn );
		return -1;
	}

	wbytes = nbuffer_put_buf( conn->wbuf, buf, len );
	if( wbytes!=len )//error
	{
		DEBUGINFO("write buffer fail, nbuffer overflow, fd %d\n", conn->fd);
		nasio_conn_close( conn );
		return -1;
	}
	if( watcher->events & EV_WRITE )
		return wbytes;
	
	DEBUGINFO("reset io event now, fd %d\n", conn->fd);
	ev_io_stop( env->loop, watcher );
	ev_io_set( watcher, conn->fd, watcher->events | EV_WRITE );
	ev_io_start( env->loop, watcher );
	return wbytes;
}

void nasio_conn_close(nasio_conn_t *conn)
{
	nasio_env_t *env = conn->env;
	ev_io_stop( env->loop, &conn->watcher );
	nlist_del( &(env->conn_list), &(conn->list_node) );
	if( conn->rbuf )
		nbuffer_destroy( conn->rbuf );
	if( conn->wbuf )
		nbuffer_destroy( conn->wbuf );
	close( conn->fd );

	if( conn->handler && conn->handler->on_close )
		conn->handler->on_close( conn );

	npool_free( env->conn_pool, (char *)conn );

	DEBUGINFO("now conn size %d\n", env->conn_list.size);

}
