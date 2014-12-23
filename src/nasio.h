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

typedef struct nasio_conn_s nasio_conn_t;
typedef struct nasio_conn_cmd_factory_s nasio_conn_cmd_factory_t;
typedef struct nasio_conn_event_handler_s nasio_conn_event_handler_t;

typedef enum 
{
	NASIO_LOOP_FOREVER = 0x00,
	NASIO_LOOP_NONBLOCK= 0x01

}nasio_loop_type_e;

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
/*
typedef struct
{
	int max_connections;
	int max_backlog;
	int accept_once;

	int workers;
}nasio_env_opt_t;
*/

struct nasio_conn_cmd_factory_s
{
	ssize_t (*frame)(nbuffer_t *);
};

struct nasio_conn_event_handler_s 
{
	void (*on_connect)(nasio_conn_t *); //when connection established.
	void (*on_close)(nasio_conn_t *); //when connection closed.
	void (*on_process)(nasio_conn_t *, nbuffer_t *);
};

/* connection */
struct nasio_conn_s
{
	nasio_env_t *env;
	uint64_t id;
	int fd;
	nasio_inaddr_t local_addr;
	nasio_inaddr_t remote_addr;
	ev_io watcher;
	nbuffer_t *rbuf;
	nbuffer_t *wbuf;

	nlist_node_t list_node;

	nasio_conn_cmd_factory_t *factory;
	nasio_conn_event_handler_t *handler;

	void *connector;
};

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
 * @param factory
 * @param handler 
 *
 * @return 0 success
 * 	   <0 fail
 */
int nasio_env_add_listen(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_cmd_factory_t *factory
	, nasio_conn_event_handler_t *handler);

/**
 * @brief add remote connection
 *
 * @param env
 * @param ip
 * @param port
 * @param factory
 * @param handler
 *
 * @return 0 success
 * 	   <0 fail
 */
int nasio_env_add_remote(nasio_env_t *env
	, const char *ip
	, short port
	, nasio_conn_cmd_factory_t *factory
	, nasio_conn_event_handler_t *handler);

/**
 * @brief run
 *
 * @param env
 * @param flag - value of nasio_loop_type_e
 *
 * @return 
 */
int nasio_env_run(nasio_env_t *env, int flag);

/**
 * @brief write bytes
 *
 * @param conn
 * @param buf
 * @param len
 *
 * @return >0 real bytes write
 * 	   <0 error
 */
int nasio_conn_write_buffer(nasio_conn_t *conn, const char *buf, size_t len);

/**
 * @brief close connection
 *
 * @param conn
 */
void nasio_conn_close(nasio_conn_t *conn);

#ifdef __cplusplus
}
#endif

#endif

