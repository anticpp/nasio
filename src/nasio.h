#ifndef _NASIO_H_
#define _NASIO_H_
/*
 * Defination of nasio_env.
 *
 * Author: supergui@live.cn
 * Date: 2014-11-05
 */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <ev.h>
#include "npool.h"
#include "nlist.h"
#include "nasio_net.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef offsetof
#define offsetof(s, e) (size_t)( &(((s *)0)->e) )
#endif

typedef struct 
{
	struct ev_loop *loop;
	npool_t *conn_pool;
	nlist_t listen_list;
	nlist_t remote_list;
	nlist_t conn_list;

	uint64_t conn_id_gen;
}nasio_env_t;

/* connection */
typedef struct
{
	uint64_t id;
	int fd;
	nasio_inaddr_t local_addr;
	nasio_inaddr_t remote_addr;
	ev_io watcher;
	//char *sendbuf;
	//char *recvbuf;

	nlist_node_t list_node;
}nasio_conn_t;

typedef struct 
{
	void (*on_connect)(nasio_conn_t *); //when connection established.
	void (*on_close)(nasio_conn_t *); //when connection closed.
}nasio_conn_event_handler_t;

nasio_env_t* nasio_env_create(int capacity);

int nasio_env_destroy(nasio_env_t *env);

int nasio_add_listen(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler);

int nasio_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler);

int nasio_run(nasio_env_t *env, int flag);


#ifdef __cplusplus
}
#endif

#endif

