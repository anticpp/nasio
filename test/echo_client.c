#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nasio.h"

void echo_on_connect(void *conn);
void echo_on_close(void *conn);
void echo_on_message(void *conn, nasio_msg_t *msg);

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
	nasio_loop(env, 0);

	return 0;
}
void echo_on_connect(void *conn)
{
	printf("connection established\n");

    char buf[] = "hello";
    nasio_msg_t req;
    nasio_msg_init_size( &req, sizeof(buf) );
    char *data = nasio_msg_data( &req );
    memcpy(data, buf, sizeof(buf));
    if( nasio_send_msg(conn, &req)<0 ) {
        printf("send message error\n");
        nasio_conn_close(conn);
    }
}
void echo_on_close(void *conn)
{
	printf("connection closed\n");
}
void echo_on_message(void *conn, nasio_msg_t *msg)
{
    printf("[%u] [%s]\n", nasio_msg_size(msg), nasio_msg_data(msg));
}
