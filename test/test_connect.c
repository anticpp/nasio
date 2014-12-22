#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nasio.h"

#define echo_welcome(conn)\
do{\
	char text[64] = {0};\
	time_t nowtime = time(0);\
	struct tm t;\
	localtime_r( &nowtime, &t );\
	snprintf(text, sizeof(text), "[%04d%02d%02d %02d:%02d:%02d] welcome.\n", 1900 + t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);\
	nasio_conn_write_buffer((conn), text, strlen(text));\
}while(0)\

ssize_t echo_frame(nbuffer_t *buf);
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

	rv = nasio_env_add_remote(env, "127.0.0.1", 12388, factory, handler);
	if( rv!=0 )
	{
		printf("add remote fail\n");
		return 2;
	}
	nasio_env_run(env, 0);

	return 0;
}
ssize_t echo_frame(nbuffer_t *buf)
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
	printf("connection established, id %llu\n", conn->id);
	echo_welcome(conn);
}
void echo_on_close(nasio_conn_t *conn)
{
	printf("connection closed, id %llu\n", conn->id);
}
void echo_on_process(nasio_conn_t *conn, nbuffer_t *buf)
{
	printf("read buffer %s\n", buf->buf+buf->pos);
	echo_welcome(conn);
}
