#ifndef _NASIO_H_
#define _NASIO_H_
/*
 * Defination of nasio_env.
 *
 * Author: supergui@live.cn
 * Date: 2014-11-05
 */
#include <ev.h>
#include "npool.h"

struct nasio_env_t
{
	struct ev_loop *loop;
	npool_t *conn_pool;
	nlist_t listen_list;
	nlist_t remote_list;
	nlist_t conn_list;
};

/* connection */
struct nasio_conn_t
{
	uint64_t id;
	int fd;
	struct sockaddr_in clientaddr;
	ev_io watcher;
	char *sendbuf;
	char *recvbuf;
};

struct nasio_conn_event_handler
{
	void (*on_connect)(nasio_conn*); //when connection established.
	void (*on_close)(nasio_conn*); //when connection closed.
};

nasio_env_t* nasio_env_create(int capacity);

int nasio_env_destroy(nasio_env_t *env);

int nasio_add_listen(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler *handler);

int nasio_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler *handler);

int nasio_start();

#endif

