#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

/* test name */
char *testName;

/* test output files */
/* #define TESTPF "/Users/jaiminsanghvi/Downloads/Advanced_Database_Organization-CS525/assignment1/test.txt" */

#define TESTPF "/Volumes/MyPassport/Education/IIT_Jaimin/Spring 2015/ADO_CS525/assignment1/assignment1/test.txt"

/* prototypes for test functions */
static void testExtraContent(void);

/* main function running all tests */
int
main (void)
{
   testName = "";

  initStorageManager();

  testExtraContent();

  return 0;
}

/* Try to read, write and move blocks */
void
testExtraContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test extra content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

/* create a new page file */
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");

/* change ph to be a string and write that one to disk */
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing first block\n");

/* change ph to be a string and write that one to disk */
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (1, &fh, ph));
  printf("writing first block\n");

/* change ph to be a string and write that one to disk */
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (2, &fh, ph));
  printf("writing first block\n");

/* change ph to be a string and write that one to disk */
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeCurrentBlock (&fh, ph));
//printf("%s",ph);
  printf("writing current block\n");

/* read back the page containing the string and check that it is correct*/
  TEST_CHECK(readBlock (2,&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
      ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
   printf("\n block has been read.\n");

/* read back the page containing the string and check that it is correct */
  TEST_CHECK(readPreviousBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
      ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading previous block\n");

/* read back the page containing the string and check that it is correct */
  TEST_CHECK(readNextBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
      ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading next block\n");

/* read back the page containing the string and check that it is correct */
  TEST_CHECK(readCurrentBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading current block\n");

/* append empty block in a file */
  TEST_CHECK(appendEmptyBlock(&fh));

/* ensure capacity of full block */
  TEST_CHECK(ensureCapacity(7, &fh));

/* destroy new page file */
  TEST_CHECK(destroyPageFile (TESTPF));

  TEST_DONE();

}



