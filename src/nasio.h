/**
 * @file nasio.h
 * @brief Defination of nasio_env.
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef _NASIO_H_
#define _NASIO_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <ev.h>
#include "npool.h"
#include "nlist.h"
#include "nbuffer.h"
#include "nasio_net.h"

#ifdef __cplusplus
extern "C" {
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
/*
typedef struct
{
	int max_connections;
	int max_backlog;
	int accept_once;

	int workers;
}nasio_env_opt_t;
*/
/* connection */
typedef struct
{
	uint64_t id;
	int fd;
	nasio_inaddr_t local_addr;
	nasio_inaddr_t remote_addr;
	ev_io watcher;
	nbuffer_t *rbuf;
	nbuffer_t *sbuf;
	//char *sendbuf;
	//char *recvbuf;

	nlist_node_t list_node;
}nasio_conn_t;

typedef struct 
{
	void (*on_connect)(nasio_conn_t *); //when connection established.
	void (*on_close)(nasio_conn_t *); //when connection closed.
}nasio_conn_event_handler_t;

/**
 * @brief Create nasio environment.
 *
 * @param capacity 
 *
 * @return NULL if fail
 */
nasio_env_t* nasio_env_create(int capacity);

/**
 * @brief destroy env
 *
 * @param env
 *
 * @return 
 */
int nasio_env_destroy(nasio_env_t *env);

/**
 * @brief add listen
 *
 * @param env  
 * @param ip  
 * @param port 
 * @param handler 
 *
 * @return 0 success
 * 	   <0 fail
 */
int nasio_add_listen(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler);

/**
 * @brief add remote connection
 *
 * @param env
 * @param ip
 * @param port
 * @param handler
 *
 * @return 0 success
 * 	   <0 fail
 */
int nasio_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler);

/**
 * @brief run
 *
 * @param env
 * @param flag
 *
 * @return 
 */
int nasio_run(nasio_env_t *env, int flag);


#ifdef __cplusplus
}
#endif

#endif

