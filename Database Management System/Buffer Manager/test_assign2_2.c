#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* var to store the current test's name */
char *testName;

/* check whether two the content of a buffer pool is the same as an expected content 
 (given in the format produced by sprintPoolContent)*/
#define ASSERT_EQUALS_POOL(expected,bm,message)			        \
  do {									\
    char *real;								\
    char *_exp = (char *) (expected);                                   \
    real = sprintPoolContent(bm);					\
    if (strcmp((_exp),real) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
	free(real);							\
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
    free(real);								\
  } while(0)


/* test and helper methods */
static void createDummyPages(BM_BufferPool *bm, int num);
static void checkDummyPages(BM_BufferPool *bm, int num);
static void testClock (void);
static void testError (void);

/* main method */
int 
main (void) 
{
  initStorageManager();
  testName = "";

  testClock();
  testError(); 
}

void 
createDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  
  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));
      sprintf(h->data, "%s-%i", "Page", h->pageNum);
      CHECK(markDirty(bm, h));
      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(h);
}

void 
checkDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  char *expected = malloc(sizeof(char) * 512);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));

      sprintf(expected, "%s-%i", "Page", h->pageNum);
      ASSERT_EQUALS_STRING(expected, h->data, "reading back dummy page content");

      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(expected);
  free(h);
}

void
testError (void)
{
  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing FIFO page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  CHECK(pinPage(bm, h, 0));
  CHECK(pinPage(bm, h, 1));
  CHECK(pinPage(bm, h, 2));
  ASSERT_ERROR(pinPage(bm, h, 3), "trying to pin a new page when buffer pool is full of pinned pages");
  CHECK(shutdownBufferPool(bm));

  printf("after first test");
  
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  ASSERT_ERROR(pinPage(bm, h, -1), "trying to pin a new page with negative page number");
  CHECK(shutdownBufferPool(bm));
  
  ASSERT_ERROR(forceFlushPool(bm), "flush buffer pool that is not open");
 
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  ASSERT_ERROR(unpinPage(bm, h), "unpin page not in buffer pool");  


  ASSERT_ERROR(forcePage(bm, h), "force page not in buffer pool");  
  ASSERT_ERROR(markDirty(bm, h), "mark page dirty that is not in buffer pool");  
  CHECK(shutdownBufferPool(bm));

  
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();  
}

void testClock()
{

 
 const char *poolContents[] = { 
    "[0 0],[-1 0],[-1 0]" , 
    "[0 0],[1 0],[-1 0]", 
    "[0 0],[1 0],[2 0]", 
    "[3 0],[1 0],[2 0]", 
    "[3 0],[4 0],[2 0]",
    "[3 0],[4 0],[2 0]",
    "[3 0],[4 0],[5 0]",
    "[6 0],[4 0],[5 0]",
    "[6 0],[4 0],[0 0]",
    "[6x0],[4 0],[0x0]",
    "[6 0],[4 0],[0 0]"
  };
  const int requests[] = {0,1,2,3,4,4,5,6,0};
  const int numLinRequests = 5;
  const int numChangeRequests = 4;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing CLOCK page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 100);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_CLOCK, NULL));

  
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
	
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}
