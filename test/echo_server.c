#include <stdio.h>
#include <stdlib.h>
#include "nasio.h"

int echo_frame(nbuffer_t *buf);
void echo_on_connect(nasio_conn_t *conn);
void echo_on_close(nasio_conn_t *conn);
void echo_on_process(nasio_conn_t *conn, nbuffer_t *buf);

int main(int argc, char* argv[])
{
	int rv = 0;
	nasio_env_t *env = nasio_env_create( 1000 );
	if( !env )
	{
		printf("nasio env create\n");
		return 1;
	}
	nasio_conn_cmd_factory_t *factory = (nasio_conn_cmd_factory_t *)malloc( sizeof(nasio_conn_cmd_factory_t) ); 
	factory->frame = echo_frame;

	nasio_conn_event_handler_t *handler = (nasio_conn_event_handler_t *)malloc( sizeof(nasio_conn_event_handler_t) ); 
	handler->on_connect = echo_on_connect;
	handler->on_close = echo_on_close;
	handler->on_process = echo_on_process;

	rv = nasio_add_listen(env, "*", 12388, factory, handler);
	if( rv!=0 )
	{
		printf("add listener fail\n");
		return 2;
	}
	nasio_run(env, 0);

	return 0;
}
int echo_frame(nbuffer_t *buf)
{
	//find '\n'
	char *start = buf->buf+buf->pos;
	char *end = buf->buf+buf->limit;
	char *pos = start;
	for(; pos<end; pos++ )
	{
		if( *pos=='\n' )
			break;
	}
	if( pos==end )//expect more
		return nbuffer_remain( buf ) + 10;
	else
		return pos-start+1;
	return -1;
}
void echo_on_connect(nasio_conn_t *conn)
{
	printf("echo server comes a connection\n");
}
void echo_on_close(nasio_conn_t *conn)
{
	printf("echo server close a connection\n");
}
void echo_on_process(nasio_conn_t *conn, nbuffer_t *buf)
{
	int remain = nbuffer_remain( buf );
	*(buf->buf+buf->limit-1) = '\0';//\n => \0
	printf("echo process, size %d, %s\n", remain, buf->buf+buf->pos);
}
