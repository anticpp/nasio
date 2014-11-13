#include <stdlib.h>
#include <stdlio.h>
#include "nlist.h"

struct mydata
{
	int id;
	nlist_node_t *node;
};

int main(int argc, char *argv[])
{
	nlist *list = nlist_create(sizeof(mydata), 100);
	return 0;
}
