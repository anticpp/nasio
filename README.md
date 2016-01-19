NASIO
======================================
>A lightweight event-driven network library, which supports atomic message.
>NASIO uses libev to handle io event, i prefer [libev][1] rather than [libevent][2], because [libev][1] is much more clean than [libevent][2] now.
>
>NASIO which is Not [Boost ASIO][3], just for fun, aha.

[1]: http://software.schmorp.de/pkg/libev.html
[2]: http://libevent.org/
[3]: http://www.boost.org/doc/libs/1_40_0/doc/html/boost_asio.html

# Echo Server
```
void echo_on_connect(void *conn);
void echo_on_close(void *conn);
void echo_on_message(void *conn, nasio_msg_t *msg);

int main(int argc, char* argv[])
{
    void *env = nasio_env_create( 1000 );
    nasio_conn_event_handler_t *handler = (nasio_conn_event_handler_t *)malloc( sizeof(nasio_conn_event_handler_t) ); 

    handler->on_connect = echo_on_connect;
    handler->on_close = echo_on_close;
    handler->on_message = echo_on_message;
    nasio_bind(env, "*", 12388, handler);

    nasio_loop(env, NASIO_LOOP_FOREVER);//loop forever

    return 0;
}

void echo_on_connect(void *conn)
{
	printf("on connect\n");
}
void echo_on_close(void *conn)
{
	printf("on close\n");
}
void echo_on_message(void *conn, nasio_msg_t *msg)
{
    printf("on message");
}
```

# Echo Client
```
void echo_on_connect(void *conn);
void echo_on_close(void *conn);
void echo_on_message(void *conn, nasio_msg_t *msg);

int main(int argc, char* argv[])
{
	void *env = nasio_env_create( 1000 );
	nasio_conn_event_handler_t *handler = (nasio_conn_event_handler_t *)malloc( sizeof(nasio_conn_event_handler_t) ); 
	handler->on_connect = echo_on_connect;
	handler->on_close = echo_on_close;
	handler->on_message = echo_on_message;

	nasio_connect(env, "127.0.0.1", 12388, handler);

	nasio_loop(env, NASIO_LOOP_FOREVER);

	return 0;
}
void echo_on_connect(void *conn)
{
    printf("on connect\n");

    // send hello
    nasio_msg_init_size( &msg, strlen("hello")+1 );
    memcpy( nasio_msg_data( &msg ), "hello", sizeof("hello") );
    nasio_send_msg(conn, &msg);
    nasio_msg_destroy( &msg );
}
void echo_on_close(void *conn)
{
    printf("on close\n");
}
void echo_on_message(void *conn, nasio_msg_t *msg)
{
    printf("on message\n");
}
```

# TODO
* Async connect timeout and retry
* Set options
    - Accept once
    - Connect retry interval
* Timer support
* Support short message
* Improve memory copy
* NBuffer
    - Improve compact
    - Extend on demands
* Inner logger
    - Defalut logger [DONE]
    - Support user-defined logger
* Connect timeout
    - Retry timeout

# Not Sure TODO
* Multiple worker support.
