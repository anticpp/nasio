#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "nasio.h"
#include "nasio_net.h"

void echo_on_connect(void *conn);
void echo_on_close(void *conn);
void echo_on_message(void *conn, nasio_msg_t *msg);

static void *g_conn = 0;

int main(int argc, char* argv[])
{
	int rv = 0;
	void *env = nasio_env_create( 1000 );
	if( !env )
	{
		printf("nasio env create\n");
		return 1;
	}
	nasio_conn_event_handler_t *handler = (nasio_conn_event_handler_t *)malloc( sizeof(nasio_conn_event_handler_t) ); 
	handler->on_connect = echo_on_connect;
	handler->on_close = echo_on_close;
	handler->on_message = echo_on_message;

	rv = nasio_connect(env, "127.0.0.1", 12388, handler);
	if( rv!=0 )
	{
		printf("add remote fail\n");
		return 2;
	}

    int ts = 0;
    char buf[] = "hello";
    nasio_msg_t msg;
    while(1) {

	    nasio_loop(env, NASIO_LOOP_NOWAIT);

        if( g_conn && nasio_env_ts(env)/1000000!=ts ) {

            ts = nasio_env_ts(env)/1000000;

            nasio_msg_init_size( &msg, sizeof(buf) );
            memcpy( nasio_msg_data( &msg ), buf, sizeof(buf) );
            nasio_send_msg(g_conn, &msg);
            nasio_msg_destroy( &msg );
            printf("[SEND] [%s] [%u]\n", nasio_msg_data(&msg), nasio_msg_size(&msg));

        }

        usleep( 1000*10 );
    }

	return 0;
}
void echo_on_connect(void *conn)
{
    struct sockaddr_in addr = nasio_conn_remote_addr(conn);
	printf("connection connected %s:%d\n", nasio_net_get_dot_addr(&addr), ntohs(addr.sin_port));

    g_conn = conn;
}
void echo_on_close(void *conn)
{
    struct sockaddr_in addr = nasio_conn_remote_addr(conn);
	printf("connection closed %s:%d\n", nasio_net_get_dot_addr(&addr), ntohs(addr.sin_port));
    g_conn = 0;
}
void echo_on_message(void *conn, nasio_msg_t *msg)
{
    printf("[RECV] [%s] [%u]\n", nasio_msg_data(msg), nasio_msg_size(msg));
}
