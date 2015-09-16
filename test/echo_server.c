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

	rv = nasio_bind(env, "*", 12388, handler);
	if( rv!=0 )
	{
		printf("add listener fail\n");
		return 2;
	}
	nasio_loop(env, NASIO_LOOP_FOREVER);//loop forever

	return 0;
}
void echo_on_connect(void *conn)
{
    struct sockaddr_in addr = nasio_conn_remote_addr(conn);
	printf("new connection %s:%d\n", nasio_net_get_dot_addr(&addr), ntohs(addr.sin_port));
}
void echo_on_close(void *conn)
{
    struct sockaddr_in addr = nasio_conn_remote_addr(conn);
	printf("close connection %s:%d\n", nasio_net_get_dot_addr(&addr), ntohs(addr.sin_port));
}
void echo_on_message(void *conn, nasio_msg_t *msg)
{
    printf("[RECV] [%s][%u]\n", nasio_msg_data(msg), nasio_msg_size(msg));

    time_t now = time(0);
    struct tm *tt = localtime(&now);
    char buf[256] = {0};
    snprintf(buf, sizeof(buf)
                    , "world %04d%02d%02d %02d:%02d:%02d"
                    , tt->tm_year+1900
                    , tt->tm_mon+1
                    , tt->tm_mday
                    , tt->tm_hour
                    , tt->tm_min
                    , tt->tm_sec);

    nasio_msg_t resp;
    nasio_msg_init_size( &resp, strlen(buf)+1 );
    char *data = nasio_msg_data( &resp );
    memcpy(data, buf, strlen(buf)+1);
    nasio_send_msg(conn, &resp);
    printf("[SEND] [%s] [%u]\n", nasio_msg_data(&resp), nasio_msg_size(&resp));
}
