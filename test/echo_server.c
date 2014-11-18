#include <stdio.h>
#include <stdlib.h>
#include "nasio.h"

int main(int argc, char* argv[])
{
	nasio_env_t *env = nasio_env_create( 1000 );
	if( !env )
	{
		printf("nasio env create\n");
		return 1;
	}
	nasio_add_listen(env, "127.0.0.1", 12389, NULL);
	nasio_run(env, 0);
	return 0;
}
