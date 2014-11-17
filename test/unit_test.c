#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nlist.h"
#include "npool.h"

typedef struct
{
	char name[64];
	void (*func)();
}unit_t;

#define MAX_TEST 100
int USED_TEST = 0;
unit_t TEST[MAX_TEST] = { {"", 0} };

int push(const char name[], void (*func)())
{
	if( USED_TEST>=MAX_TEST )
		return -1;
	unit_t *t = &(TEST[USED_TEST++]);
	memcpy(t->name, name, sizeof(t->name));
	t->func = func;
	return 0;
}

void run()
{
	for(int i=0; i<USED_TEST; i++)
	{
		unit_t *t = &(TEST[i]);
		printf("[[ %s ]]\n", t->name);
		t->func();
	}
}

/* test case below */

/* for nlist test */
typedef struct 
{
	nlist_node_t node;
	int id;
}mydata_t;

int iter_mydata_list(nlist_t *list, int arr[], int len)
{
	nlist_node_t *n = list->head;
	for(int i=0; i<len; i++)
	{
		if( n && ((mydata_t *)n)->id==arr[i] )
			n = n->next;
		else
			return -1;
	}
	return n?-1:0;
}

void test_nlist()
{
	nlist_t *list = (nlist_t *)malloc( sizeof(nlist_t) );

	mydata_t m[10];
	for(int i=0; i<sizeof(m)/sizeof(mydata_t); i++)
		m[i].id = i;
	
	nlist_node_t *n = NULL;

	/* init */
	nlist_init( list );
	assert( list->head==NULL && list->tail==NULL );
	printf( "nlist init ok\n" );

	/* insert */
	/** tail insert 
	 ** add tail 0,1,2,3
	 ** 0->1->2->3
	 **/
	for(int i=0; i<4; i++)
		nlist_insert_tail( list, &(m[i].node) );
	//iter_mydata_list(list, {0, 1, 2, 3}, 4);
	
	n = list->head;
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==3 && !(n=n->next) );
	printf("nlist tail insert ok\n");
	
	/** head insert 
	 ** add head 4,5
	 ** 5->4->0->1->2->3
	 **/
	nlist_insert_head( list, &(m[4].node) );
	nlist_insert_head( list, &(m[5].node) );
	n = list->head;
	assert( n && ((mydata_t *)n)->id==5 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==4 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==3 && !(n=n->next) );
	printf("nlist head insert ok\n");

	/** random insert after
	 ** add 6 after 5, 7 after 2 
	 ** 5->6->4->0->1->2->7->3
	 **/
	nlist_insert_after( list, &(m[5].node), &(m[6].node) );
	nlist_insert_after( list, &(m[2].node), &(m[7].node) );
	n = list->head;
	assert( n && ((mydata_t *)n)->id==5 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==6 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==4 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==7 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==3 && !(n=n->next) );
	printf("nlist random insert after ok\n");
	
	/** random insert before
	 ** add 8 before 2, 9 before 4
	 ** 5->6->9->4->0->1->8->2->7->3
	 **/
	nlist_insert_before( list, &(m[2].node), &(m[8].node) );
	nlist_insert_before( list, &(m[4].node), &(m[9].node) );
	n = list->head;
	assert( n && ((mydata_t *)n)->id==5 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==6 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==9 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==4 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==8 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==7 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==3 && !(n=n->next) );
	printf("nlist random insert before ok\n");

	/** head delete
	 ** delete 5, 6
	 ** 9->4->0->1->8->2->7->3
	 **/
	nlist_del_head( list );
	nlist_del_head( list );
	n = list->head;
	assert( n && ((mydata_t *)n)->id==9 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==4 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==8 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==7 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==3 && !(n=n->next) );
	printf("nlist head delete ok\n");

	/** tail delete 
	 ** delete 7, 3
	 ** 9->4->0->1->8->2
	 **/
	nlist_del_tail( list );
	nlist_del_tail( list );
	n = list->head;
	assert( n && ((mydata_t *)n)->id==9 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==4 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==8 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && !(n=n->next) );
	printf("nlist tail delete ok\n");

	/** random delete 
	 ** delete 4, 8
	 ** 9->0->1->2
	 **/
	nlist_del( list, &(m[4].node) );
	nlist_del( list, &(m[8].node) );
	n = list->head;
	assert( n && ((mydata_t *)n)->id==9 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==0 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==1 && (n=n->next) );
	assert( n && ((mydata_t *)n)->id==2 && !(n=n->next) );
	printf("nlist random delete ok\n");

	/** delete all 
	 ** delete 9, 0, 1, 2
	 ** NULL
	 **/
	nlist_del( list, &(m[0].node) );
	nlist_del( list, &(m[9].node) );
	nlist_del( list, &(m[1].node) );
	nlist_del( list, &(m[2].node) );
	assert( nlist_isempty(list) );
	printf("nlist delete all ok\n");

	free( list );
}

void test_npool()
{
	char *elem1 = NULL, *elem2 = NULL;
	int elemsize = 40;
	/** create
	 **/
	npool_t *pool = npool_create(elemsize, 10);
	assert( pool && npool_available(pool)==10 );
	printf("npool create ok\n");

	/** alloc
	 **/
	elem1 = npool_alloc( pool );
	elem2 = npool_alloc( pool );
	assert( elem1 && elem2 && npool_available(pool)==8 );
	assert( (elem1 - (char*)pool - sizeof(npool_t)) % elemsize ==0 );
	assert( (elem2 - (char*)pool - sizeof(npool_t)) % elemsize ==0 );
	printf("npool alloc ok\n");
	
	/** free
	 **/
	npool_free( pool, elem1 );
	assert( npool_available(pool)==9 );
	npool_free( pool, elem2 );
	assert( npool_available(pool)==10 );
	printf("npool free ok\n");

	npool_destroy( pool );
}

int main(int argc, char* argv[])
{
	push( "nlist_test", test_nlist );
	push( "npool_test", test_npool );

	run();
	return 0;
}
