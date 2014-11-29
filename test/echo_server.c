#include <stdio.h>
#include <stdlib.h>
#include "nasio.h"

int main(int argc, char* argv[])
{
	int rv = 0;
	nasio_env_t *env = nasio_env_create( 1000 );
	if( !env )
	{
		printf("nasio env create\n");
		return 1;
	}
	//rv = nasio_add_listen(env, "127.0.0.1", 12388, NULL);
	rv = nasio_add_listen(env, "*", 12388, NULL);
	if( rv!=0 )
	{
		printf("add listener fail\n");
		return 2;
	}
	nasio_run(env, 0);

	return 0;
}
