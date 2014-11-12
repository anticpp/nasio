#include <stdlib.h>
#include <stdlio.h>
#include "nlist.h"

struct mydata
{
	uint64_t uid;
	char name[64];
};

int main(int argc, char *argv[])
{
	nlist *list = nlist_create(sizeof(mydata), 100);
	return 0;
}
