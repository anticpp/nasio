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

#define PROTOCOL_VERSION 1
#define PROTOCOL_MAGIC 0x438eaf12
#define MAX_MESSAGE_SIZE 5*1024*1024
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

typedef struct nasio_conn_s nasio_conn_t;

typedef enum
{
	NASIO_CONNECT_STATE_WAIT = 0,
	NASIO_CONNECT_STATE_PENDING = 1,
	NASIO_CONNECT_STATE_DONE = 2,
	NASIO_CONNECT_STATE_RETRY = 3 
}nasio_connect_stat_e;

typedef struct 
{
	struct ev_loop *loop;
	npool_t *conn_pool;
	nlist_t listener_list;
	nlist_t connector_list;
	nlist_t conn_list;
	nlist_t close_conn_list;

	uint64_t conn_id_gen;
	uint64_t now_time_us;
}nasio_env_t;

typedef struct
{
	int fd;
	struct sockaddr_in addr;
	ev_io watcher;

	nlist_node_t list_node;

	nasio_conn_event_handler_t *handler;
}nasio_listener_t;

typedef struct 
{
	int fd;
	struct sockaddr_in addr;
	ev_io watcher;

	nlist_node_t list_node;
	nasio_conn_event_handler_t *handler;

	int state;
	int retry_cnt;
	time_t last_try;
}nasio_connector_t;

/* connection */
struct nasio_conn_s
{
	nasio_env_t *env;
	uint64_t id;
	int fd;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
	ev_io read_watcher;
	ev_io write_watcher;
	nbuffer_t *rbuf;
	nbuffer_t *wbuf;

	nlist_node_t list_node;

	nasio_conn_event_handler_t *handler;

	void *connector;
};

static void on_listener_cb(struct ev_loop *loop, struct ev_io *w, int revents);
//static void on_fd_event_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents);
static void on_fd_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents);

static nasio_conn_t* nasio_conn_new(nasio_env_t *env, int fd, nasio_conn_event_handler_t *handler);
static void nasio_process_connector(nasio_env_t *env);
static void nasio_process_close_list(nasio_env_t *env);
static ssize_t nasio_msg_frame(nbuffer_t *buf, nasio_msg_header_t *header);
static ssize_t nasio_msg_encode_header(nbuffer_t **pbuf, const nasio_msg_header_t *header);

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
		if( cfd<0 && error_not_ready() ) { /* no pending */
			break;	
		}
		else if( cfd<0 ) { /* accept error */
			break;
		}
		else {
			nasio_conn_t *newconn = nasio_conn_new(env, cfd, listener->handler);
			if( !newconn ) {
				close( cfd );
				break;
			}
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

	if( (revents & EV_READ) || (revents & EV_WRITE) ) {
		rv = getsockopt(connector->fd, SOL_SOCKET, SO_ERROR, &error, &n);
		if( rv<0 || error )
			goto retry;
		
		/* connect succ
		*/
		nasio_conn_t *newconn = nasio_conn_new(env
				, connector->fd
				, connector->handler);
		if( !newconn )
			goto retry;
		newconn->connector = connector;

		connector->state = NASIO_CONNECT_STATE_DONE;
		nlist_del( &(env->connector_list), &(connector->list_node) );
		return;
	}
	else if( revents & EV_ERROR ){
	}
	//else

retry:
	close( connector->fd );
	connector_set_retry( env, connector );
	return ;
}
/*
void on_fd_event_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	nasio_conn_t *conn = connection_of(w, watcher);
	if( revents & EV_READ )
		on_fd_readable_cb( loop, w, revents );
	else if( revents & EV_WRITE )
		on_fd_writable_cb( loop, w, revents );
	else if( revents & EV_ERROR ) {
		nasio_conn_close( conn );
	}
}*/

void on_fd_readable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	size_t hungry = 4*1024;
	ssize_t rbytes = 0;
    nasio_msg_t msg;
    nasio_msg_header_t header;
	nasio_conn_t *conn = connection_of(w, read_watcher);
	if( !conn->rbuf ) 
		conn->rbuf = nbuffer_create( 8*1024 );//8KB
	if( !conn->rbuf ) {
		nasio_conn_close( conn );
		return;
	}

	if( nbuffer_require( &(conn->rbuf), hungry )<0 ) {
		nasio_conn_close(conn);
		return ;
	}

	rbytes = read( conn->fd, conn->rbuf->buf+conn->rbuf->pos, hungry );
	if( rbytes<0 && !error_not_ready() ) {
		nasio_conn_close(conn);
	}
	else if ( rbytes==0 ) {
		nasio_conn_close(conn);
	}
	else if( rbytes>0 ) {
		nbuffer_set_pos( conn->rbuf, conn->rbuf->pos+rbytes );

		nbuffer_flip( conn->rbuf );
        ssize_t expect = nasio_msg_frame(conn->rbuf, &header);
		if( expect<0 ) { //error
			nasio_conn_close(conn);
			return;
		}
		else if( nbuffer_remain(conn->rbuf)>=expect ) {
            if( nasio_msg_init_size(&msg, expect-sizeof(nasio_msg_header_t))<0 ) {
                nasio_conn_close(conn);
                return;
            }
            msg.header = header;
            memcpy(msg.data, conn->rbuf->buf+conn->rbuf->pos+sizeof(nasio_msg_header_t), msg.header.length);
            if( conn->handler && conn->handler->on_message ) {
                conn->handler->on_message( conn, &msg );
            }
            
            nasio_msg_destroy( &msg );
			nbuffer_set_pos( conn->rbuf, conn->rbuf->pos+expect );
		}
		nbuffer_rewind( conn->rbuf );
	}
}
void on_fd_writable_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	nasio_conn_t *conn = connection_of(w, write_watcher);
	nasio_env_t *env = conn->env;
	size_t wbytes = 0;
	ssize_t realwbytes = 0;
	size_t hungry = 4*1024;//most 4KB once
	ssize_t remain = 0;

	nbuffer_flip( conn->wbuf );
	wbytes = nbuffer_remain( conn->wbuf );
	if( wbytes>=hungry )
		wbytes=hungry;
	realwbytes = write( conn->fd, conn->wbuf->buf+conn->wbuf->pos, wbytes );
	if( realwbytes<0 && !error_not_ready() ) {
		nasio_conn_close(conn);
		return;
	}
	else if( realwbytes>0 ) {
		nbuffer_set_pos( conn->wbuf, conn->wbuf->pos+realwbytes );
	}
	remain = nbuffer_remain( conn->wbuf );
	nbuffer_rewind( conn->wbuf );

	//stop write
	if( remain<=0 ) {
		ev_io_stop( env->loop, w );
	}
}

nasio_conn_t* nasio_conn_new(nasio_env_t *env
		, int fd
		, nasio_conn_event_handler_t *handler)
{
	/* allocate && init 
	 */
	nasio_conn_t *conn = (nasio_conn_t *)npool_alloc( env->conn_pool );
	if( !conn ) 
		return NULL;
	
	conn->env = env;
	conn->id = new_conn_id(env);
	conn->fd = fd;
	nasio_net_get_local_addr( fd, &(conn->local_addr) );
	nasio_net_get_remote_addr( fd, &(conn->remote_addr) );
	conn->handler = handler;
	conn->rbuf = NULL;
	conn->wbuf = NULL;

	/* add to list
	 */
	nlist_insert_tail( &(env->conn_list), &(conn->list_node) );

	/* init watcher
	 */
	ev_io_init( &(conn->read_watcher), on_fd_readable_cb, fd, EV_READ);
	ev_io_init( &(conn->write_watcher), on_fd_writable_cb, fd, EV_WRITE);

    /* always watch READ
     */
	ev_io_start( env->loop, &(conn->read_watcher) );

	if( conn->handler && conn->handler->on_connect )
		conn->handler->on_connect( conn );

	return conn;
}
void nasio_process_connector(nasio_env_t *env)
{
	int rv = 0;
	nlist_node_t *next = env->connector_list.head;
	nlist_node_t *prev = 0;
	while( next )
	{
		nasio_connector_t *connector = connector_of( next, list_node );
		if( connector->state==NASIO_CONNECT_STATE_WAIT ) {
			connector->fd = socket(AF_INET, SOCK_STREAM, 0);
			if( connector->fd<0 ) {
				connector_set_retry( env, connector );
				goto next;
			}
			nasio_net_set_block( connector->fd, 0 );//set nonblock

			rv = connect( connector->fd, (struct sockaddr *)&(connector->addr), sizeof(struct sockaddr_in) );
			if( rv==0 ) { //succ
				nasio_conn_t *newconn = nasio_conn_new(env
							, connector->fd
							, connector->handler);
				if( !newconn ) {
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
			else if( rv<0 && errno==EINPROGRESS ) {
				connector->state = NASIO_CONNECT_STATE_PENDING;
				ev_io_init( &(connector->watcher), on_connector_cb, connector->fd, EV_READ | EV_WRITE );
				ev_io_start( env->loop, &(connector->watcher) );
			}
			else if( rv<0 ) {
				close( connector->fd );
				connector_set_retry( env, connector );
			}
		}
		else if( connector->state==NASIO_CONNECT_STATE_RETRY 
				&&  now_time_sec(env)-connector->last_try>=CONNECT_RETRY_INTERVAL ) {
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
	nlist_node_t *next = env->close_conn_list.head;
	nlist_node_t *prev = 0;
	while( next )
	{
		nasio_conn_t *conn = connection_of( next, list_node );
		if( conn->handler && conn->handler->on_close )
			conn->handler->on_close( conn );

		if( conn->rbuf )
			nbuffer_destroy( conn->rbuf );
		if( conn->wbuf )
			nbuffer_destroy( conn->wbuf );
		close( conn->fd );

		/* reconnect
		*/
		if( conn->connector ) {
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

ssize_t nasio_msg_frame(nbuffer_t *buf, nasio_msg_header_t *header)
{
    int header_len = sizeof(nasio_msg_header_t);
    if( nbuffer_remain(buf)<header_len ) 
        return header_len;

    /* parse header
     */
    nasio_msg_header_t *h = (nasio_msg_header_t *)(buf->buf + buf->pos);
    header->version = ntohl( h->version );
    header->magic = ntohl( h->magic );
    header->length = ntohl( h->length );
    if( header->magic!=PROTOCOL_MAGIC )
        return -1;

    if( header->length>MAX_MESSAGE_SIZE )
        return -2;
    
    return header_len + header->length;
}

ssize_t nasio_msg_encode_header(nbuffer_t **pbuf, const nasio_msg_header_t *header)
{
    if( nbuffer_require( pbuf, sizeof(nasio_msg_header_t) ) <0 )
            return -1;

    nbuffer_t *nbuf = *pbuf;
    nasio_msg_header_t *h = (nasio_msg_header_t *)(nbuf->buf+nbuf->pos);
    h->version = htonl(header->version);
    h->magic = htonl(header->magic);
    h->length = htonl(header->length);
    nbuffer_set_pos(nbuf, nbuf->pos+sizeof(nasio_msg_header_t));
    return sizeof(nasio_msg_header_t);
}

/* 
 * public interface 
 */

void* nasio_env_create(int capacity)
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

int nasio_env_destroy(void *env)
{
    nasio_env_t *e = (nasio_env_t *)env;
	ev_loop_destroy( e->loop );

	if( e->conn_pool )
		npool_destroy( e->conn_pool );

	free( env );

	return 0;
}

int nasio_bind(void *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler)
{
	int rv = 0;
    nasio_env_t *e = (nasio_env_t *)env;
	nasio_listener_t *listener = (nasio_listener_t *)malloc( sizeof(nasio_listener_t) );
	if( !listener )
		return -1;
	listener->watcher.data = env;//attach env
	listener->handler = handler;

	listener->fd = socket(AF_INET, SOCK_STREAM, 0);
	if( listener->fd<0 ) {
		free( listener );
		return -1;
	}
	
	nasio_net_set_block(listener->fd, 0);//set nonblock
	nasio_net_set_reuse(listener->fd, 1);//set reuse

	memset(&(listener->addr), 0x00, sizeof(struct sockaddr_in));
	listener->addr.sin_family = AF_INET;
	listener->addr.sin_port = htons(port);
	if ( strcmp(ip, "*")==0 )
		listener->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		listener->addr.sin_addr.s_addr = inet_addr(ip);
	
	rv = bind(listener->fd, (struct sockaddr *)&(listener->addr), sizeof(struct sockaddr_in));
	if( rv<0 ) {
		close(listener->fd);
		free( listener );
		return -1;
	}
	
	rv = listen(listener->fd, MAX_BACKLOG);
	if( rv<0 ) {
		close(listener->fd);
		return -1;
	}
	
	ev_io_init(&listener->watcher, on_listener_cb, listener->fd, EV_READ);
	ev_io_start(e->loop, &(listener->watcher));

	nlist_insert_tail( &(e->listener_list), &(listener->list_node) );

	return 0;
}

int nasio_connect(void *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler)
{
    nasio_env_t *e = (nasio_env_t *)env;
	nasio_connector_t *connector = (nasio_connector_t *)malloc( sizeof(nasio_connector_t) );
	if( !connector )
		return -1;

	memset(connector, 0x00, sizeof(nasio_connector_t));
	connector->watcher.data = env;
	connector->handler = handler;
	connector->state = NASIO_CONNECT_STATE_WAIT;

	memset(&(connector->addr), 0x00, sizeof(struct sockaddr_in));
	connector->addr.sin_family = AF_INET;
	connector->addr.sin_port = htons(port);
	connector->addr.sin_addr.s_addr = inet_addr(ip);

	//lazy connect
	nlist_insert_tail( &(e->connector_list), &(connector->list_node) );
	
	return 0;
}

int nasio_loop(void *env, int flag)
{
    nasio_env_t *e = (nasio_env_t *)env;

	struct timeval tm;
	while(1)
	{
		if( gettimeofday( &tm, NULL)<0 )
			return -1;
		e->now_time_us = tm.tv_sec*1000000+tm.tv_usec;

		/* loop event
		 */
		ev_loop(e->loop, EVLOOP_NONBLOCK);

		/* deal with connector
		 */
		nasio_process_connector( e );

		/* close tobe-close
		 */
		nasio_process_close_list( e );

		if( flag==NASIO_LOOP_NONBLOCK )
			break;
		else
			usleep(1000*10);
	}
	return 0;
}

void nasio_conn_close(void *conn)
{
    nasio_conn_t *c = (nasio_conn_t *)conn;
	nasio_env_t *env = c->env;
	ev_io_stop( env->loop, &(c->read_watcher) );
	ev_io_stop( env->loop, &(c->write_watcher) );
	
	/* move from connection-list to tobe-close-list
	 */
	nlist_del( &(env->conn_list), &(c->list_node) );
	nlist_insert_tail( &(env->close_conn_list), &(c->list_node) );
}

int nasio_send_msg(void *conn, nasio_msg_t *msg)
{
    nasio_conn_t *c = (nasio_conn_t *)conn;
    ev_io *watcher = &(c->write_watcher);
    nasio_env_t *env = c->env;

    size_t needed = sizeof(nasio_msg_header_t) + msg->header.length;
	if( !c->wbuf )
		c->wbuf = nbuffer_create( 8*1024 );//8KB
	if( !c->wbuf ) {
		return -1;
	}
	if( nbuffer_require( &(c->wbuf), needed )<0 ) {
		return -1;
	}
    nasio_msg_encode_header( &(c->wbuf), &(msg->header) );
	nbuffer_put_buf( c->wbuf, msg->data, msg->header.length );

	if( !ev_is_active(watcher) ) {
	    ev_io_start( env->loop, watcher );
    }

	return 0;
}
int nasio_msg_init_size(nasio_msg_t *msg, uint32_t size)
{
    msg->header.version = PROTOCOL_VERSION;
    msg->header.magic = PROTOCOL_MAGIC;
    msg->header.length = size;

    msg->data = (char *)malloc( size );
    if( !msg->data )
        return -1;
    return 0;
}
int nasio_msg_destroy(nasio_msg_t *msg)
{
    if(msg->data)
        free(msg->data);
    return 0;
}
char *nasio_msg_data(nasio_msg_t *msg)
{
    return msg->data;
}
uint32_t nasio_msg_size(nasio_msg_t *msg)
{
    return msg->header.length;
}
