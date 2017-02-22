------------------------------------------------------
------------------------------------------------------
* * * * * * * * * * * : READ ME : * * * * * * * * * *
------------------------------------------------------
------------------------------------------------------

Group Name : jsanghv2
---------------------
1) Jaimin Sanghvi (A20344798)
2) Rahi Shah (A20343885)
3) Neel Desai (A20319444)
4) Priyank Shah (A20344797)

Steps to run assignment functions : 
-----------------------------------
1) Run makefile named as Makefile using linux command (make -f Makefile clean) to remove old object files
2) Run makefile named as Makefile using linux command (make -f Makefile) to generate object files(*.o) and project executable file named as assign21.
3) Run assign1 exe using linux command (./assign2)

Steps to run extra functions :
------------------------------
4) Run makefile named as Makefile2 using linux command (make -f Makefile2 clean) to remove old object files
5) Run makefile named as Makefile2 using linux command (make -f Makefile2) to generate object files(*.o) and project executable file named as asssign22.
6) Run assign2 exe using linux command (./assign22)

Description about Assignment2 : 
-------------------------------
: We have implemented buffer manager as per your insturctions :
- We have implemented all the functions which are included in test_assign2_1.c file which includes page replacement strategies named as FIFO and LRU.
- We have defined extra error codes (dberror.h File) as per our requirements.
  Extra error codes are as below :
  1) #define RC_FILE_NOT_OPENED 304
  2) #define RC_FILE_NOT_DELETED 305
  3) #define RC_IO_ERROR 306
  4) #define RC_NO_PAGE_FOUND 307
  5) #define RC_STRATEGY_NOT_FOUND 308
  6) #define RC_OVERLOAD_IN_POOL 309
  7) #define RC_PAGE_NUMBER_NEGATIVE 310
  8) #define RC_FORCE_FLUSH_POOL_NOT_OPEN 311
  9) #define RC_PAGE_NOT_FOUND 312
	
- We have created test_assign2_2.c file for extra functions(page replacement strategy - Clock and testError cases) which are implemented in buffer_mgr.c file.

List of Extra Test Cases :
--------------------------
1) Clock (Page Replacement Algorithm) named as testClock().
2) Test error cases named as testError().

Functional Description :
------------------------
1)
Function Name : initBufferPool
Expected Arguments : BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData
Description : Initialize buffer pool content with structure variables. 

2)
Function Name : shutdownBufferPool
Expected Arguments : BM_BufferPool *const bm
Description : In primary condition it checks pages. If a page is dirty then it will write page to storage and release occupied memory of buffer pool.

3)
Function Name : forceFlushPool
Expected Arguments : BM_BufferPool *const bm
Description : It causes all dirty pages from the buffer pool to be written to disk.

// Buffer Manager Interface Access Pages
4)
Function Name : markDirty
Expected Arguments : BM_BufferPool *const bm, BM_PageHandle *const page
Description : If a page is updated then it marks it as a dirty.

5)
Function Name : unpinPage 
Expected Arguments : BM_BufferPool *const bm, BM_PageHandle *const page
Description : It unpins the page which decrement its fix count by 1.

6)
Function Name : forcePage 
Expected Arguments : BM_BufferPool *const bm, BM_PageHandle *const page
Description : It should write the current content of the page back to the page file on disk.

7)
Function Name : pinPage 
Expected Arguments : BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum
Description : It pins a page which push a page content in buffer. If buffer is full then it replace a page according to page replacement algorithm named as FIFO, LRU and CLOCK.

// Statistics Interface
8)
Function Name : PageNumber *getFrameContents 
Expected Argument : BM_BufferPool *const bmi
Description : It returns an array of PageNumbers (of size numPages) where the ith element is the number of the page stored in the ith page frame. An empty page frame is represented using the constant NO_PAGE.

9)
Function Name : bool *getDirtyFlags 
Expected Argument : BM_BufferPool *const bm
Description : It returns an array of bools (of size numPages) where the ith element is TRUE if the page stored in the ith page frame is dirty. Empty page frames are considered as clean.

10)
Function Name : int *getFixCounts 
Expected Argument : BM_BufferPool *const bm
Description : returns an array of ints (of size numPages) where the ith element is the fix count of the page stored in the ith page frame. Return 0 for empty page frames.

11)
Function Name : int getNumReadIO 
Expected Argument : BM_BufferPool *const bm
Description : It returns the number of pages that have been read from disk since a buffer pool has been initialized. You code is responsible to initializing this statistic at pool creating time and update whenever a page is read from the page file into a page frame.

12)
Function Name : int getNumWriteIO 
Expected Argument : BM_BufferPool *const bm
Description : It returns the number of pages written to the page file since the buffer pool has been initialized.
