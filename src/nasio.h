/**
 * @file nasio.h
 * @brief Defination of nasio_env.
 * @author supergui@live.cn
 * @version 
 * @date 2014-11-05
 */
#ifndef NASIO_H_
#define NASIO_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <ev.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nasio_conn_event_handler_s nasio_conn_event_handler_t;
typedef struct nasio_msg_s nasio_msg_t;

typedef enum  {
	NASIO_LOOP_FOREVER = 0x00,
	NASIO_LOOP_NOWAIT= 0x01
}nasio_loop_type_e;

enum nasio_log_level_e {
    NASIO_LOG_LEVEL_ALL,
    NASIO_LOG_LEVEL_TRACE,
    NASIO_LOG_LEVEL_INFO,
    NASIO_LOG_LEVEL_DEBUG,
    NASIO_LOG_LEVEL_WARN,
    NASIO_LOG_LEVEL_ERROR,
    NASIO_LOG_LEVEL_FATAL
};

struct nasio_conn_event_handler_s  {
    /**
     * @brief On connection established.
     *
     * @param connection
     */
	void (*on_connect)(void *); 
    
    /*
     * @brief On connection closed.
     *
     * @param connection
     */
	void (*on_close)(void *); 

    /*
     * @brief On message received.
     *
     * @param connection
     * @param message
     */
	void (*on_message)(void *, nasio_msg_t *);
};

/**
 * @brief Always use nasio_msg_xxx functions to operate nasio_msg_s
 *        , unless you know what you are doing.
 */
struct nasio_msg_s {
    unsigned char _ [32];
};

typedef void (*log_callback)(int level, const char *fmt, ...);

/**
 * @brief Create nasio environment.
 *
 * @param capacity 
 *
 * @return NULL fail
 *         not NULL success
 */
void* nasio_env_create(int capacity);

/**
 * @brief destroy env
 *
 * @param env
 *
 * @return 
 */
int nasio_env_destroy(void *env);


/**
 * @brief 
 *      Get current microsecond.
 *
 * @return 
 */
int nasio_env_ts(void *env);

/**
 * @brief Bind tcp address.
 *
 * @param env  
 * @param ip - "*" for INADDR_ANY
 * @param port 
 * @param handler 
 *
 * @return 0 success
 * 	   <0 fail
 */
int nasio_bind(void *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler);

/**
 * @brief connect to remote.
 *
 * @param env
 * @param ip
 * @param port
 * @param handler
 *
 * @return 0 success
 * 	   <0 fail
 */
int nasio_connect(void *env
	, const char *ip
	, short port
	, nasio_conn_event_handler_t *handler);

/**
 * @brief Loop
 *
 * @param env
 * @param flag - nasio_loop_type_t
 *
 * @return 
 */
int nasio_loop(void *env, int flag);

/**
 * @brief 
 *      Set debug level.
 *      The default log level is NASIO_LOG_LEVEL_ALL.
 *      This is meaningful only when NASIO_DEBUG is opened.
 *
 * @param level - type of nasio_log_level_e
 */
void nasio_set_log_level(void *env, int level);

/**
 * @brief close connection
 *
 * @param conn
 */
void nasio_conn_close(void *conn);

/**
 * @brief 
 *      Get connection id.
 */
uint64_t nasio_conn_get_id(void *conn);

/**
 * @brief 
 *      Get connection fd.
 */
uint64_t nasio_conn_get_fd(void *conn);

/**
 * @brief 
 *      Get local address.
 */
struct sockaddr_in nasio_conn_local_addr(void *conn);

/**
 * @brief 
 *      Get remote address.
 */
struct sockaddr_in nasio_conn_remote_addr(void *conn);

/**
 * @brief Write message.
 *
 * @param conn
 * @param message
 *
 * @return 0 success
 * 	       <0 error
 */
int nasio_send_msg(void *conn, nasio_msg_t *msg);

/**
 * @brief Init message with size.
 *
 * @param msg
 * @param size
 *
 * @return 0 success
 *         <0 error
 */
int nasio_msg_init_size(nasio_msg_t *msg, uint32_t size);

/**
 * @brief Destroy message.
 *
 * @param msg
 *
 * @return 0 success
 *         <0 error
 */
int nasio_msg_destroy(nasio_msg_t *msg);

/**
 * @brief Get message data.
 *
 * @param message
 *
 * @return Data.
 */
void *nasio_msg_data(nasio_msg_t *msg);

/**
 * @brief Get message data size.
 *
 * @param message
 *
 * @return Size.
 */
size_t nasio_msg_size(nasio_msg_t *msg);


#ifdef __cplusplus
}
#endif

#endif

