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
class NListTest : public ::testing::Test
{
protected:
	typedef struct 
	{
		nlist_node_t node;
		int id;
	}TestData;

	const static int DATA_LEN = 10;
protected:
	NListTest() 
	{
		data_arr = NULL;
		list = list1 = list2 = NULL;
	}
	~NListTest() {}

	virtual void SetUp()
	{
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
	virtual void TearDown() 
	{
		if( data_arr )
		{
			free( data_arr );
			data_arr = NULL;
		}
		if( list )
		{
			free( list );
			list = list1 = list2 = NULL;
		}
	}
	
	nlist_t *list;
	nlist_t *list1, *list2;

	TestData *data_arr;

public:
	void ITER_AND_ASSERT_NLIST(nlist_t *list, int arr[], int arrlen)
	{
		nlist_node_t *node = list->head;
		for(int i=0; i<arrlen; i++)
		{
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

TEST_F(NListTest, NListTestCreate)
{
	/* Test for create is ok.
	 */

	ASSERT_TRUE( list1->head==NULL );
	ASSERT_TRUE( list1->tail==NULL );
}

TEST_F(NListTest, NListTestInsertTail)
{
	/* Test for append nodes to tail.
	 */

	int arr[] = {0, 1, 2, 3, 4};
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}

TEST_F(NListTest, NListTestInsertHead)
{
	/* Test for append nodes before head.
	 */

	int arr[] = {6, 5, 0, 1, 2, 3, 4};
	nlist_insert_head( list2, &(data_arr[5].node) );
	nlist_insert_head( list2, &(data_arr[6].node) );
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}

TEST_F(NListTest, NListTestInsertRandomAfter)
{
	/* Test for insert node after a random node.
	 */

	int arr[] = {0, 5, 1, 2, 3, 6, 4};
	nlist_insert_after( list2,  &(data_arr[0].node),  &(data_arr[5].node));
	nlist_insert_after( list2,  &(data_arr[3].node),  &(data_arr[6].node));
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestInsertRandomBefore)
{
	/* Test for insert node before a random node.
	 */

	int arr[] = {0, 5, 1, 2, 3, 6, 4};
	nlist_insert_after( list2,  &(data_arr[0].node),  &(data_arr[5].node));
	nlist_insert_after( list2,  &(data_arr[3].node),  &(data_arr[6].node));
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestDeleteHead)
{
	/* Test for delete head node.
	 */

	int arr[] = {2, 3, 4};
	nlist_del_head( list2 );
	nlist_del_head( list2 );
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestDeleteTail)
{
	/* Test for delete tail node.
	 */

	int arr[] = {0, 1, 2};
	nlist_del_tail( list2 );
	nlist_del_tail( list2 );
	ITER_AND_ASSERT_NLIST( list2, arr, sizeof(arr)/sizeof(int) );
}
TEST_F(NListTest, NListTestDeleteAll)
{
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
class NPoolTest : public ::testing::Test
{
protected:
	NPoolTest() 
	{
		pool = NULL;
	}
	~NPoolTest() {}
	
	const static int ELEMSIZE = 13;
	const static int POOLSIZE = 10;

	virtual void SetUp() 
	{
		pool = npool_create(ELEMSIZE, POOLSIZE);
	}
	virtual void TearDown() 
	{
		if( pool )
			npool_destroy( pool );
	}

	npool_t *pool;
};

TEST_F(NPoolTest, NPoolTestCreate)
{
	/* Test for create is ok.
	 */

	ASSERT_TRUE( pool );
	ASSERT_EQ( npool_available(pool), int(POOLSIZE) ); 
}

TEST_F(NPoolTest, NPoolTestOperate)
{
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
TEST_F(NPoolTest, NPoolTestAllocAll)
{
	/* Test for allocate all available from pool.
	 */

	char *elem;
	for(int i=0; i<POOLSIZE; i++)
	{
		elem = npool_alloc( pool );
		ASSERT_TRUE( elem!=NULL );
	}
	elem = npool_alloc( pool );
	ASSERT_TRUE( elem==NULL );

	ASSERT_EQ( pool->unused, 0 );
	ASSERT_TRUE( pool->free_list==NULL );
}

/** Test case: nbuffer **/
class NBufferTest : public ::testing::Test
{
protected:
	NBufferTest() 
	{ 
		nbuf = NULL; 
	}
	~NBufferTest() 
	{
	}

	const static size_t BUFSIZE = 4*1024; //4KB
	
	virtual void SetUp()
	{
		nbuf = nbuffer_create( BUFSIZE );
	}
	virtual void TearDown()
	{
		if(nbuf)
		{
			nbuffer_destroy( nbuf );
			nbuf = NULL;
		}
	}

	nbuffer_t *nbuf;
};

TEST_F(NBufferTest, NBufferTestCreate)
{
	/* Test for create is ok.
	 */

	ASSERT_TRUE( nbuf!=NULL );
	EXPECT_EQ( nbuf->pos, 0 );
	EXPECT_EQ( nbuf->capacity, size_t(BUFSIZE) );
	EXPECT_EQ( nbuf->limit, nbuf->capacity );
	EXPECT_EQ( nbuffer_remain(nbuf), nbuf->capacity );
}
TEST_F(NBufferTest, NBufferTestRequire)
{
	int rv = 0;
	char tmp[100] = {0};
	nbuffer_t *nbuf1 = nbuffer_create(100);
	ASSERT_TRUE( nbuf1!=NULL );

	nbuffer_put_buf( nbuf1, tmp, 90 );
	ASSERT_EQ( nbuffer_remain(nbuf1), 10 );

	nbuffer_flip( nbuf1 );
	nbuffer_get_buf( nbuf1, tmp, 20 );

	nbuffer_rewind( nbuf1 );

	ASSERT_EQ( nbuffer_remain(nbuf1), 10 );
	rv = nbuffer_require( &nbuf1, 20 );//just compact is ok
	ASSERT_EQ( rv, 0 );
	ASSERT_EQ( nbuffer_remain(nbuf1), 30 );
	ASSERT_EQ( nbuf1->pos, 70 );
	ASSERT_EQ( nbuf1->capacity, 100 );

	rv = nbuffer_require( &nbuf1, 40 );//need enlarge
	ASSERT_EQ( rv, 0 );
	ASSERT_EQ( nbuf1->pos, 70 );
	ASSERT_EQ( nbuf1->capacity, int(100<<1) );
	ASSERT_EQ( nbuffer_remain(nbuf1), int((100<<1) - 70) );
}
TEST_F(NBufferTest, NBufferTestOperate)
{
	/* Test for read/write operations.
	 */
	char content[] = "HELLO WORLD";
	char content2[] = ", NBUFFER.";
	char tmp[64] = {0};
	ssize_t wbytes = 0, rbytes = 0;
	ssize_t wbytes2 = 0, rbytes2 = 0;
	//size_t remain = 0;

	/* Write
	 * EXPECT: "HELLO WORLD""
	 */
	wbytes = nbuffer_put_buf( nbuf, content, sizeof(content)-1 );
	ASSERT_EQ( wbytes, sizeof(content)-1 );
	ASSERT_EQ( nbuf->pos, wbytes );
	ASSERT_EQ( nbuffer_remain(nbuf), nbuf->capacity-sizeof(content)+1 );

	/* Flip
	 * EXPECT: "HELLO WORLD"
	 */
	nbuffer_flip( nbuf );
	ASSERT_EQ( nbuf->pos, 0 );
	ASSERT_EQ( nbuf->limit, wbytes );
	ASSERT_EQ( nbuffer_remain(nbuf), wbytes );
	ASSERT_EQ( nbuf->mark, -1 );

	/* Read
	 * EXPECT: " WORLD"
	 */
	rbytes = nbuffer_get_buf( nbuf, tmp, 5 );//read 5 bytes
	ASSERT_EQ( rbytes, 5 );
	ASSERT_EQ( nbuf->pos, rbytes );
	ASSERT_EQ( nbuffer_remain(nbuf), wbytes-rbytes );
	ASSERT_TRUE( strncmp(tmp, "HELLO", sizeof(tmp))==0 );

	/* Rewind
	 * EXPECT: " WORLD"
	 */
	nbuffer_rewind( nbuf );
	ASSERT_EQ( nbuf->pos, wbytes );
	ASSERT_EQ( nbuf->limit, nbuf->capacity );
	ASSERT_EQ( nbuf->mark, rbytes );

	/* Write.2
	 * EXPECT " WORLD, NBUFFER."
	 */
	wbytes2 = nbuffer_put_buf( nbuf, content2, sizeof(content2)-1 );
	ASSERT_EQ( wbytes2, sizeof(content2)-1 );
	ASSERT_EQ( nbuf->pos, wbytes2+wbytes );
	ASSERT_EQ( nbuffer_remain(nbuf), nbuf->capacity-sizeof(content)-sizeof(content2)+2 );

	/* Flip.2
	 * EXPECT: " WORLD, NBUFFER."
	 */
	nbuffer_flip( nbuf );
	ASSERT_EQ( nbuf->pos, rbytes );
	ASSERT_EQ( nbuf->limit,  wbytes+wbytes2 );
	ASSERT_EQ( nbuffer_remain(nbuf), wbytes+wbytes2-rbytes );
	ASSERT_EQ( nbuf->mark, -1 );

	/* Read.2
	 * EXPECT: "D, NBUFFER."
	 */
	memset(tmp, 0x00, sizeof(tmp));
	rbytes2 = nbuffer_get_buf( nbuf, tmp, 5 );//read 5 bytes
	ASSERT_EQ( rbytes2, 5 );
	ASSERT_EQ( nbuf->pos, rbytes+rbytes2 );
	ASSERT_EQ( nbuffer_remain(nbuf), wbytes+wbytes2-rbytes-rbytes2 );
	ASSERT_TRUE( strncmp(tmp, " WORL", sizeof(tmp))==0 );

	/* Rewind.2
	 * EXPECT: "D, NBUFFER."
	 */
	nbuffer_rewind( nbuf );
	ASSERT_EQ( nbuf->pos, wbytes+wbytes2 );
	ASSERT_EQ( nbuf->limit, nbuf->capacity );
	ASSERT_EQ( nbuf->mark, rbytes+rbytes2 );

	/* Compact
	 */
	nbuffer_compact( nbuf );
	ASSERT_EQ( nbuf->pos, wbytes+wbytes2-rbytes-rbytes2 );
	ASSERT_EQ( nbuf->limit, nbuf->capacity );
	ASSERT_EQ( nbuffer_remain(nbuf), nbuf->capacity-(wbytes+wbytes2-rbytes-rbytes2) );
	ASSERT_EQ( nbuf->mark, -1 );
}
TEST_F(NBufferTest, NBufferTestSetPosLimit)
{
	char tmp[] = "HELLO WORLD";
	char tmp1[1024] = {0};
	int len = strlen(tmp);
	nbuffer_put_buf( nbuf, tmp, len );
	nbuffer_flip( nbuf );

	int cur = nbuf->pos;
	/* Origin content
	 */
	memset(tmp1, 0x00, sizeof(tmp1));
	nbuffer_get_buf( nbuf, tmp1, sizeof(tmp1) );
	ASSERT_TRUE( strcmp(tmp1, "HELLO WORLD")==0 ) << "tmp1: " << tmp1;
	
	/* Normal limit
	 */
	nbuffer_set_pos( nbuf, cur );
	nbuffer_set_limit( nbuf, len-6 );
	memset(tmp1, 0x00, sizeof(tmp1));
	nbuffer_get_buf( nbuf, tmp1, sizeof(tmp1) );
	ASSERT_TRUE( strcmp(tmp1, "HELLO")==0 );

	/* Beyond capacity
	 * Nothing change
	 */
	nbuffer_set_pos( nbuf, cur );
	nbuffer_set_limit( nbuf, nbuf->capacity+10 );
	memset(tmp1, 0x00, sizeof(tmp1));
	nbuffer_get_buf( nbuf, tmp1, sizeof(tmp1) );
	ASSERT_TRUE( strcmp(tmp1, "HELLO")==0 );
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
