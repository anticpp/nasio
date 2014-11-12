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

struct nasio_env
{
	struct ev_loop *loop;
	npool *conn_pool;
	nlist listen_list;
	nlist remote_list;
	nlist conn_list;
};

/* connection */
struct nasio_conn
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

nasio_env* nasio_env_create(int capacity);

int nasio_env_destroy(nasio_env *env);

int nasio_add_listen(nasio_env *env
	, const char *ip
	, short port
	, nasio_conn_event_handler *handler);

int nasio_add_remote(nasio_env *env
	, const char *ip
	, short port
	, nasio_conn_event_handler *handler);

int nasio_start();

#endif

