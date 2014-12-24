#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include "nasio.h"
#include "nasio_net.h"

#define DEBUGINFO(...) fprintf(stderr, __VA_ARGS__);

#define MAX_BACKLOG 10000
#define ACCEPT_ONCE 5

#define CONNECT_RETRY_INTERVAL 3

#define microsecond(sec) ((sec)*1000000)
#define now_time_sec(env) (((env)->now_time_us)/1000000)
#define now_time_usec(env) ((env)->now_time_us)

#ifndef offsetof
#define offsetof(s, e) (size_t)( &(((s *)0)->e) )
#endif
#define parent_of(type, ref, name) ( (type *)((char *)(ref)-offsetof(type, name)) )
#define listener_of(ref, name) parent_of(nasio_listener_t, ref, name)
#define connection_of(ref, name) parent_of(nasio_conn_t, ref, name)
#define connector_of(ref, name) parent_of(nasio_connector_t, ref, name)

#define error_not_ready() ( errno==EAGAIN || errno==EWOULDBLOCK )
#define new_conn_id(env) ( (++((env)->conn_id_gen)) % 0x00ffffffffffffff )

#define connector_set_retry(env, connector)\
do{\
	((connector)->state) = NASIO_CONNECT_STATE_RETRY;\
	((connector)->retry_cnt)++;\
	((connector)->last_try) = now_time_sec(env);\
}while(0)\

typedef enum
{
	NASIO_CONNECT_STATE_WAIT = 0,
	NASIO_CONNECT_STATE_PENDING = 1,
	NASIO_CONNECT_STATE_DONE = 2,
	NASIO_CONNECT_STATE_RETRY = 3 
}nasio_connect_stat_e;

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

	int state;
	int retry_cnt;
	time_t last_try;
}nasio_connector_t;

static void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_event_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static nasio_conn_t* nasio_conn_new(nasio_env_t *env, int fd, nasio_conn_event_handler_t *handler, nasio_conn_cmd_factory_t *factory);
static void nasio_process_connector(nasio_env_t *env);
static void nasio_process_close_list(nasio_env_t *env);

/* private method
 */

void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	int max = ACCEPT_ONCE;
	int cfd = 0;

	nasio_env_t *env = (nasio_env_t *)w->data;
	nasio_listener_t *listener = listener_of(w, watcher);
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
			nasio_conn_t *newconn = nasio_conn_new(env, cfd, listener->handler, listener->factory);
			if( !newconn ) 
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
void on_connector_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	int rv = 0;
	int error = 0; 
	unsigned int n = sizeof(int);
	nasio_env_t *env = (nasio_env_t *)w->data;
	nasio_connector_t *connector = connector_of( w, watcher );

	/* whatever, stop event
	 */
	ev_io_stop( loop, &(connector->watcher) );

	if( (revents & EV_READ) || (revents & EV_WRITE) )
	{
		rv = getsockopt(connector->fd, SOL_SOCKET, SO_ERROR, &error, &n);
		if( rv<0 || error )
			goto retry;
		
		/* connect succ
		*/
		nasio_conn_t *newconn = nasio_conn_new(env
				, connector->fd
				, connector->handler
				, connector->factory);
		if( !newconn )
			goto retry;
		newconn->connector = connector;

		connector->state = NASIO_CONNECT_STATE_DONE;
		nlist_del( &(env->connector_list), &(connector->list_node) );
		return;
	}
	else if( revents & EV_ERROR )
	{
	}
	//else

retry:
	close( connector->fd );
	connector_set_retry( env, connector );
	return ;
}

void on_fd_event_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	nasio_conn_t *conn = connection_of(w, watcher);
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
	nasio_conn_t *conn = connection_of(w, watcher);
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
	nasio_conn_t *conn = connection_of(w, watcher);
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

nasio_conn_t* nasio_conn_new(nasio_env_t *env
		, int fd
		, nasio_conn_event_handler_t *handler
		, nasio_conn_cmd_factory_t *factory)
{
	/* allocate && init 
	 */
	nasio_conn_t *conn = (nasio_conn_t *)npool_alloc( env->conn_pool );
	memset((char*)conn, 0x00, sizeof(nasio_conn_t));
	if( !conn ) 
	{
		DEBUGINFO("alloc newconn fail! connection pool available %d\n", npool_available(env->conn_pool));
		return NULL;
	}
	conn->env = env;
	conn->id = new_conn_id(env);
	conn->fd = fd;
	nasio_net_get_local_addr( fd, &(conn->local_addr) );
	nasio_net_get_remote_addr( fd, &(conn->remote_addr) );
	conn->handler = handler;
	conn->factory = factory;
	conn->rbuf = NULL;
	conn->wbuf = NULL;

	/* add to list
	 */
	nlist_insert_tail( &(env->conn_list), &(conn->list_node) );

	/* always care about READ, in case other end close connection.
	 */
	ev_io_init( &(conn->watcher), on_fd_event_cb, fd, EV_READ);
	ev_io_start( env->loop, &(conn->watcher) );

	if( conn->handler && conn->handler->on_connect )
		conn->handler->on_connect( conn );

	return conn;
}
void nasio_process_connector(nasio_env_t *env)
{
	int rv = 0;
	struct sockaddr_in in4addr;
	nlist_node_t *next = env->connector_list.head;
	nlist_node_t *prev = 0;
	while( next )
	{
		nasio_connector_t *connector = connector_of( next, list_node );
		memset(&in4addr, 0x00, sizeof(in4addr));
		nasio_net_convert_to_inaddr( &in4addr, &(connector->addr) );
		if( connector->state==NASIO_CONNECT_STATE_WAIT )
		{
			connector->fd = socket(AF_INET, SOCK_STREAM, 0);
			if( connector->fd<0 )
			{
				connector_set_retry( env, connector );
				goto next;
			}
			nasio_net_set_block( connector->fd, 0 );//set nonblock

			rv = connect( connector->fd, (struct sockaddr *)&in4addr, sizeof(in4addr) );
			if( rv==0 )//succ
			{
				DEBUGINFO("[nasio_process_connector] connect succ, fd %d\n", connector->fd);
				nasio_conn_t *newconn = nasio_conn_new(env
							, connector->fd
							, connector->handler
							, connector->factory);
				if( !newconn )
				{
					close( connector->fd );
					connector_set_retry( env, connector );
					goto next;
				}
				newconn->connector = connector;
				
				connector->state = NASIO_CONNECT_STATE_DONE;

				prev = next;
				next = next->next;
				nlist_del( &(env->connector_list), prev );
				continue;
			}
			else if( rv<0 && errno==EINPROGRESS )
			{
				connector->state = NASIO_CONNECT_STATE_PENDING;
				ev_io_init( &(connector->watcher), on_connector_cb, connector->fd, EV_READ | EV_WRITE );
				ev_io_start( env->loop, &(connector->watcher) );
			}
			else if( rv<0 )
			{
				close( connector->fd );
				connector_set_retry( env, connector );
			}
		}
		else if( connector->state==NASIO_CONNECT_STATE_RETRY 
				&&  now_time_sec(env)-connector->last_try>=CONNECT_RETRY_INTERVAL )
		{
			DEBUGINFO("retry connect %s:%d, last try %lu, retry cnt %d\n"
					, nasio_net_get_dot_addr(connector->addr.addr)
					, connector->addr.port
					, connector->last_try
					, connector->retry_cnt);
			connector->state = NASIO_CONNECT_STATE_WAIT;
		}

		/* else state==NASIO_CONNECT_STATE_RETRY || state==NASIO_CONNECT_STATE_PENDING
		 * do nothing
		 *
		 * TODO: connect timeout
		 */

next:
		next = next->next;
	}
}

void nasio_process_close_list(nasio_env_t *env)
{
	if( env->close_conn_list.size>0 )
		DEBUGINFO("nasio_process_close_list list size %d\n", env->close_conn_list.size);
	nlist_node_t *next = env->close_conn_list.head;
	nlist_node_t *prev = 0;
	while( next )
	{
		nasio_conn_t *conn = connection_of( next, list_node );
		DEBUGINFO("close connection fd %d\n", conn->fd);
		if( conn->handler && conn->handler->on_close )
			conn->handler->on_close( conn );

		if( conn->rbuf )
			nbuffer_destroy( conn->rbuf );
		if( conn->wbuf )
			nbuffer_destroy( conn->wbuf );
		close( conn->fd );

		/* reconnect
		*/
		if( conn->connector )
		{
			nasio_connector_t *connector = (nasio_connector_t *)conn->connector;
			connector->retry_cnt = 0;
			connector->state = NASIO_CONNECT_STATE_WAIT;
			nlist_insert_tail( &(env->connector_list), &(connector->list_node) );
		}

		prev = next;
		next = next->next;
		nlist_del( &(env->close_conn_list), prev );

		npool_free( env->conn_pool, (char *)conn );
	}
}
/* 
 * public interface 
 */

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

	nlist_init( &(env->listener_list) );
	nlist_init( &(env->connector_list) );
	nlist_init( &(env->conn_list) );
	nlist_init( &(env->close_conn_list) );
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

	nlist_insert_tail( &(env->listener_list), &(listener->list_node) );

	return 0;
}

int nasio_env_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_cmd_factory_t *factory
	, nasio_conn_event_handler_t *handler)
{
	struct sockaddr_in in4addr;
	nasio_connector_t *connector = (nasio_connector_t *)malloc( sizeof(nasio_connector_t) );
	if( !connector )
		return -1;

	memset(connector, 0x00, sizeof(nasio_connector_t));
	connector->watcher.data = env;
	connector->handler = handler;
	connector->factory = factory;
	connector->state = NASIO_CONNECT_STATE_WAIT;

	memset(&in4addr, 0x00, sizeof(struct sockaddr_in));
	in4addr.sin_family = AF_INET;
	in4addr.sin_port = htons(port);
	in4addr.sin_addr.s_addr = inet_addr(ip);
	nasio_net_convert_inaddr( &(connector->addr), &in4addr );

	//append to list
	nlist_insert_tail( &(env->connector_list), &(connector->list_node) );
	
	return 0;
}

int nasio_env_run(nasio_env_t *env, int flag)
{
	struct timeval tm;
	while(1)
	{
		if( gettimeofday( &tm, NULL)<0 )
			return -1;
		env->now_time_us = tm.tv_sec*1000000+tm.tv_usec;

		/* loop event
		 */
		ev_loop(env->loop, EVLOOP_NONBLOCK);

		/* deal with connector
		 */
		nasio_process_connector( env );

		/* close tobe-close
		 */
		nasio_process_close_list( env );

		if( flag==NASIO_LOOP_NONBLOCK )
			break;
		else
			usleep(1000*10);
	}
	return 0;
}


int nasio_conn_write_buffer(nasio_conn_t *conn, const char *buf, size_t len)
{
	//DEBUGINFO("connection write buffer, fd %d, size %zu\n", conn->fd, len);
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
	
	//DEBUGINFO("reset io event now, fd %d\n", conn->fd);
	ev_io_stop( env->loop, watcher );
	ev_io_set( watcher, conn->fd, watcher->events | EV_WRITE );
	ev_io_start( env->loop, watcher );
	return wbytes;
}

void nasio_conn_close(nasio_conn_t *conn)
{
	DEBUGINFO("nasio_conn_close called\n");
	nasio_env_t *env = conn->env;
	ev_io_stop( env->loop, &conn->watcher );
	
	/* move from connection-list to tobe-close-list
	 */
	nlist_del( &(env->conn_list), &(conn->list_node) );
	nlist_insert_tail( &(env->close_conn_list), &(conn->list_node) );
}

