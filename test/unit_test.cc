#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <gtest/gtest.h>
#include "nlist.h"
#include "npool.h"
#include "nbuffer.h"
/*
 * Unit test with gtest.
 *
 * Author: supergui@live.cn
 * Date: 2014-11-16
 */

/** Test case: nlist **/
class NListTest : public ::testing::Test {
protected:
	typedef struct {
		nlist_node_t node;
		int id;
	} TestData;

	const static int DATA_LEN = 10;
protected:
	NListTest() {
		data_arr = NULL;
		list = list1 = list2 = NULL;
	}
	~NListTest() {}

	virtual void SetUp() {
		data_arr = (TestData *)malloc( sizeof(TestData)*DATA_LEN );
		for( int i=0; i<DATA_LEN; i++ )
			data_arr[i].id = i;

		list = (nlist_t *)malloc( sizeof(nlist_t)*2 );
		list1 = list;
		list2 = list+1;
		nlist_init( list1 );
		nlist_init( list2 );

		for( int i=0; i<DATA_LEN-5; i++ )
			nlist_insert_tail( list2, &(data_arr[i].node) );
	}
	virtual void TearDown()  {
		if( data_arr ) {
			free( data_arr );
			data_arr = NULL;
		}
		if( list ) {
			free( list );
			list = list1 = list2 = NULL;
		}
	}
	
	nlist_t *list;
	nlist_t *list1, *list2;

	TestData *data_arr;

public:
	void ITER_AND_ASSERT_NLIST(nlist_t *list, int arr[], int arrlen) {
		nlist_node_t *node = list->head;
		for(int i=0; i<arrlen; i++) {
			TestData *data = (TestData *)node;
			ASSERT_TRUE( (node && data->id==arr[i]) ) << "[" << i << "] node: " << node << ", data->id: " << data->id << ", arr[i]: " << arr[i];
			node = node->next;
			if( i==arrlen-1) 
				ASSERT_FALSE( node ) << "[" << i << "] node: " << node;
			else
				ASSERT_TRUE( node ) << "[" << i << "] node: " << node;
		}
	}
};

TEST_F(NListTest, NListTestCreate) {
	/* Test for create is ok.
	 */

	ASSERT_TRUE( list1->head==NULL );
	ASSERT_TRUE( list1->tail==NULL );
}

TEST_F(NListTest, NListTestInsertTail) {
	/* Test for append nodes to tail.
	 */

	int arr[] = {0, 1, 2, 3, 4};
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}

TEST_F(NListTest, NListTestInsertHead) {
	/* Test for append nodes before head.
	 */

	int arr[] = {6, 5, 0, 1, 2, 3, 4};
	nlist_insert_head( list2, &(data_arr[5].node) );
	nlist_insert_head( list2, &(data_arr[6].node) );
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}

TEST_F(NListTest, NListTestInsertRandomAfter) {
	/* Test for insert node after a random node.
	 */

	int arr[] = {0, 5, 1, 2, 3, 6, 4};
	nlist_insert_after( list2,  &(data_arr[0].node),  &(data_arr[5].node));
	nlist_insert_after( list2,  &(data_arr[3].node),  &(data_arr[6].node));
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestInsertRandomBefore) {
	/* Test for insert node before a random node.
	 */

	int arr[] = {0, 5, 1, 2, 3, 6, 4};
	nlist_insert_after( list2,  &(data_arr[0].node),  &(data_arr[5].node));
	nlist_insert_after( list2,  &(data_arr[3].node),  &(data_arr[6].node));
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestDeleteHead) {
	/* Test for delete head node.
	 */

	int arr[] = {2, 3, 4};
	nlist_del_head( list2 );
	nlist_del_head( list2 );
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestDeleteTail) {
	/* Test for delete tail node.
	 */

	int arr[] = {0, 1, 2};
	nlist_del_tail( list2 );
	nlist_del_tail( list2 );
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestDeleteAll) {
	/* Test for delete all nodes.
	 */

	nlist_del( list2, &(data_arr[0].node) );
	nlist_del( list2, &(data_arr[1].node) );
	nlist_del( list2, &(data_arr[2].node) );
	nlist_del( list2, &(data_arr[3].node) );
	nlist_del( list2, &(data_arr[4].node) );
	ASSERT_TRUE( nlist_isempty( list2) );
}
/** Test case: npool **/
class NPoolTest : public ::testing::Test {
protected:
	NPoolTest()  {
		pool = NULL;
	}
	~NPoolTest() {}
	
	const static int ELEMSIZE = 13;
	const static int POOLSIZE = 10;

	virtual void SetUp()  {
		pool = npool_create(ELEMSIZE, POOLSIZE);
	}
	virtual void TearDown()  {
		if( pool )
			npool_destroy( pool );
	}

	npool_t *pool;
};

TEST_F(NPoolTest, NPoolTestCreate) {
	/* Test for create is ok.
	 */

	ASSERT_TRUE( pool );
	ASSERT_EQ( npool_available(pool), int(POOLSIZE) ); 
}

TEST_F(NPoolTest, NPoolTestOperate) {
	/* Test for allocate and free elements. 
	 */

	char *elem1, *elem2;

	/* alloc
    */
    elem1 = npool_alloc( pool );
    elem2 = npool_alloc( pool );
    ASSERT_TRUE( elem1!=NULL );
    ASSERT_TRUE( elem2!=NULL );
    ASSERT_EQ( npool_available(pool), POOLSIZE-2 );

    /* free
    */
    int avail = npool_available( pool );
    npool_free( pool, elem1 );
    ASSERT_EQ( npool_available(pool),  avail+1 );
    npool_free( pool, elem2 );
    ASSERT_EQ( npool_available(pool), avail+2 );
}
TEST_F(NPoolTest, NPoolTestAllocAll) {
	/* Test for allocate all available from pool.
	 */
	char *elem;
	for(int i=0; i<POOLSIZE; i++) {
		elem = npool_alloc( pool );
		ASSERT_TRUE( elem!=NULL );
	}
	elem = npool_alloc( pool );
	ASSERT_TRUE( elem==NULL );

	ASSERT_EQ( pool->unused, 0 );
	ASSERT_TRUE( pool->free_list==NULL );
}

/** Test case: nbuffer **/
class NBufferTest : public ::testing::Test {
protected:
	NBufferTest() { 
		nbuf = NULL; 
	}
	~NBufferTest() {
	}

    #define NBUFSIZE 100 
	
	virtual void SetUp() {
		nbuf = nbuffer_create( NBUFSIZE );
	    ASSERT_TRUE( nbuf!=NULL );

	    ASSERT_EQ( nbuffer_get_pos(nbuf), 0 );
	    ASSERT_EQ( nbuffer_get_capacity(nbuf), NBUFSIZE );
	    ASSERT_EQ( nbuffer_get_limit(nbuf), NBUFSIZE );
	    ASSERT_EQ( nbuffer_get_remaining(nbuf), NBUFSIZE );
        ASSERT_TRUE( nbuffer_has_remaining(nbuf) );
	}
	virtual void TearDown() {
		if(nbuf) {
			nbuffer_destroy( nbuf );
			nbuf = NULL;
		}
	}

	nbuffer_t *nbuf;
};

/*
TEST_F(NBufferTest, NBufferTestRequire)
{
	int rv = 0;
	char tmp[100] = {0};
	nbuffer_t *nbuf1 = nbuffer_create(100);
	ASSERT_TRUE( nbuf1!=NULL );

	nbuffer_put_buf( nbuf1, tmp, 90 );
	ASSERT_EQ( nbuffer_get_remaining(nbuf1), 10 );

	nbuffer_flip( nbuf1 );
	nbuffer_get_buf( nbuf1, tmp, 20 );

	nbuffer_rewind( nbuf1 );

	ASSERT_EQ( nbuffer_get_remaining(nbuf1), 10 );
	rv = nbuffer_require( &nbuf1, 20 );//just compact is ok
	ASSERT_EQ( rv, 0 );
	ASSERT_EQ( nbuffer_get_remaining(nbuf1), 30 );
	ASSERT_EQ( nbuf1->pos, 70 );
	ASSERT_EQ( nbuf1->capacity, 100 );

	rv = nbuffer_require( &nbuf1, 40 );//need enlarge
	ASSERT_EQ( rv, 0 );
	ASSERT_EQ( nbuf1->pos, 70 );
	ASSERT_EQ( nbuf1->capacity, int(100<<1) );
	ASSERT_EQ( nbuffer_get_remaining(nbuf1), int((100<<1) - 70) );
}*/
TEST_F(NBufferTest, NBufferTestWrite) {
	ssize_t wbytes = nbuffer_write( nbuf, "HELLO WORLD", 11 );
	ASSERT_EQ( wbytes, 11 );
	ASSERT_EQ( nbuffer_get_pos(nbuf), wbytes );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), nbuf->capacity-11 );
	ASSERT_TRUE( nbuffer_has_remaining(nbuf) );
}
TEST_F(NBufferTest, NBufferTestWriteOverflow) {
    size_t total = nbuffer_get_remaining( nbuf ) + 10;//a little more
    size_t remaining = nbuffer_get_remaining( nbuf );
    size_t wbytes = 0;
    while( total>0 ) {
        wbytes += nbuffer_write( nbuf, "HELLO WORLD", 11 );
        total -= 11;
    }
    ASSERT_EQ( wbytes, remaining );
	ASSERT_EQ( nbuffer_get_pos(nbuf), remaining );
    ASSERT_EQ( nbuffer_get_remaining(nbuf), 0 );
    ASSERT_FALSE( nbuffer_has_remaining(nbuf) );
}
TEST_F(NBufferTest, NBufferTestWriteFlipRead) {
    size_t rbytes = 0;
    char tmp[128] = {0};

	ASSERT_EQ( nbuffer_write( nbuf, "HELLO WORLD", 11 ), 11 );
	nbuffer_flip( nbuf );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), 11 );
	ASSERT_TRUE( nbuffer_has_remaining(nbuf) );

	/* read some
	 */
	rbytes = nbuffer_read( nbuf, tmp, 5 );//read 5 bytes
	ASSERT_EQ( rbytes, 5 );
	ASSERT_EQ( nbuffer_get_pos(nbuf), 5 );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), 11-5 );
	ASSERT_TRUE( nbuffer_has_remaining(nbuf) );
    tmp[rbytes] = '\0';
	ASSERT_EQ( strcmp(tmp, "HELLO"), 0 );

    /* read more than remaining
     */
	rbytes = nbuffer_read( nbuf, tmp, 10 );//only 6 bytes will be returned
	ASSERT_EQ( rbytes, 6 );
	ASSERT_EQ( nbuffer_get_pos(nbuf), 11 );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), 0 );
	ASSERT_FALSE( nbuffer_has_remaining(nbuf) );
    tmp[rbytes] = '\0';
	ASSERT_EQ( strcmp(tmp, " WORLD"), 0 );

	/* Rewind
	 */
	nbuffer_rewind( nbuf );
	ASSERT_EQ( nbuffer_get_pos(nbuf), 0 );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), 11 );
	ASSERT_TRUE( nbuffer_has_remaining(nbuf) );

    /* read again
     */
	rbytes = nbuffer_read( nbuf, tmp, sizeof(tmp) );//read all 11 bytes
	ASSERT_EQ( rbytes, 11 );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), 0 );
	ASSERT_FALSE( nbuffer_has_remaining(nbuf) );
    tmp[rbytes] = '\0';
	ASSERT_EQ( strcmp(tmp, "HELLO WORLD"), 0 );
}
TEST_F(NBufferTest, NBufferTestCompact) {
    char tmp[128] = {0};

	ASSERT_EQ( nbuffer_write( nbuf, "HELLO WORLD", 11 ), 11 );
	nbuffer_flip( nbuf );
    nbuffer_read( nbuf, tmp, 5 );//read 'HELLO'
    size_t remaining = nbuffer_get_remaining( nbuf );
    ASSERT_EQ( remaining, 11-5 );

	/* Compact
	 */
	nbuffer_compact( nbuf );
	ASSERT_EQ( nbuffer_get_remaining(nbuf), nbuffer_get_capacity(nbuf)-remaining );
}
TEST_F(NBufferTest, NBufferTestSetPosLimit) {
    char tmp[128] = {0};
    size_t bytes = 0;

	nbuffer_write( nbuf, "HELLO WORLD", 11 );
    nbuffer_set_pos( nbuf, 5 );
    nbuffer_write( nbuf, " SUPERGUI", 9);
	nbuffer_flip( nbuf );

    /* read all
     */
    bytes = nbuffer_read( nbuf, tmp, sizeof(tmp));
    ASSERT_EQ( bytes, 14 );
    tmp[bytes] = '\0';
    ASSERT_EQ( strcmp(tmp, "HELLO SUPERGUI"), 0 );
    
    /* set back to 5
     */
    nbuffer_set_pos( nbuf, 5);
    bytes = nbuffer_read( nbuf, tmp, sizeof(tmp));
    ASSERT_EQ( bytes, 9 );
    tmp[bytes] = '\0';
    ASSERT_EQ( strcmp(tmp, " SUPERGUI"), 0 );
}

TEST_F(NBufferTest, NBufferTestMark) {
    char tmp[128] = {0};
    size_t bytes = 0;

	ASSERT_EQ( nbuffer_write( nbuf, "HELLO", 5 ), 5 );
    nbuffer_mark( nbuf );
    ASSERT_EQ( nbuffer_write( nbuf, " WORLD", 6 ), 6 );
    nbuffer_reset( nbuf );
    ASSERT_EQ( nbuffer_write( nbuf, " SUPERGUI", 9 ), 9 );

    nbuffer_flip( nbuf );

    bytes = nbuffer_read( nbuf, tmp, sizeof(tmp));
    ASSERT_EQ( bytes, 14 );
    tmp[bytes] = '\0';
    ASSERT_EQ( strcmp(tmp, "HELLO SUPERGUI"), 0 );
}
TEST_F(NBufferTest, NBufferTestDigest) {
    char tmp[128] = {0};
    size_t bytes = 0;

	ASSERT_EQ( nbuffer_write( nbuf, "HELLO WORLD", 11 ), 11 );
    nbuffer_flip( nbuf );
    ASSERT_EQ( nbuffer_digest( nbuf, 5 ), 5 );
    ASSERT_EQ( nbuffer_get_remaining( nbuf ), 6 );

    bytes = nbuffer_read( nbuf, tmp, sizeof(tmp));
    ASSERT_EQ( bytes, 6 );
    tmp[bytes] = '\0';
    ASSERT_EQ( strcmp(tmp, " WORLD"), 0 );
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
