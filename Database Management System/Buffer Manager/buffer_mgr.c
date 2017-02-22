#include "storage_mgr.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "dt.h"

/* General Buffer Information Statistics Structure */
typedef struct buffer_info_stat{
	int readIO;
	int writeIO;
	PageNumber *pageNumStat;
	int *fixCountStat;
	bool *dirtyStat;
	void *bufStat;
	SM_FileHandle *fh;
	struct buffer_info_stat *next;
}buffer_info_stat;

/* Structure for FIFO Page Replacement Strategy */
 struct FIFO{
	int pageNum;
	int fixCount;
	bool dirty;
	int pagePos;	
	char *pageData;
	struct FIFO *nextElement;
} *head=NULL,*tail=NULL,*firstElement = NULL;

/* Structure for LRU Page Replacement Strategy */
struct LRU{
	 int pageNum;
	 int fixCount;
	 bool dirty;
	 int lastRecUsed;
	 char *pageData;
	 int pagePos;
	 struct LRU *nextElement;
 } *buf_lru, *start_lru = NULL, *end_lru = NULL;

/* Structure for Clock Page Replacement Strategy */
struct CLOCK{
	 int pageNum;
	 int fixCount;
	 bool dirty;
	 int pagePos;
	 char *pageData;
	 int clockBit;
	 struct CLOCK *nextElement;
 } *buf_clock=NULL,*start_clock=NULL,*end_clock=NULL,*firstClock=NULL;

#define MAKE_QUEUE_BUFFER()				\
	((struct FIFO *) malloc(sizeof(struct FIFO)))

#define MAKE_BUFFER_INFO_STAT()				\
	((buffer_info_stat *)malloc(sizeof(buffer_info_stat)))

#define MAKE_LRU_BUFFER()					\
	((struct LRU *) malloc(sizeof(struct LRU)))

#define MAKE_CLOCK_BUFFER()					\
	((struct CLOCK *) malloc(sizeof(struct CLOCK)))

/*
 * AUTHOR : Rahi Shah
 * DESCRIPTION : Initialize buffer
*/

RC initBufferPool(BM_BufferPool * const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void* stratData) {

	FILE *file;
   if (file == fopen((char *)pageFileName, "r")){
       fclose(file);
       return RC_FILE_NOT_FOUND;
   }
	
	SM_FileHandle *fh = (SM_FileHandle *)malloc(PAGE_SIZE); /* allocate size to File handler */
	memset(fh, 0, PAGE_SIZE);

	PageNumber* pageNumArr;
	int* fixCountArr;
	bool* dirtyArr;
	
	bm->numPages = numPages;	

	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();  

	struct LRU *buf_lru;
	buf_lru = ((struct LRU *) malloc(sizeof(struct LRU)));

	int i;
	int totPages = bm->numPages;
	pageNumArr = (PageNumber *)malloc(sizeof(PageNumber)*totPages);
	fixCountArr = (int *)malloc(sizeof(int)*totPages);
	dirtyArr = (bool *)malloc(sizeof(bool)*totPages);
	for (i = 0; i < totPages; i++) {
		pageNumArr[i] = NO_PAGE;
		fixCountArr[i] = 0;
		dirtyArr[i] = false;
	}

	buffer_info_stat *buf_stat;
	buf_stat = MAKE_BUFFER_INFO_STAT();

	buf_stat->pageNumStat = pageNumArr;
	buf_stat->dirtyStat = dirtyArr;
	buf_stat->fixCountStat = fixCountArr;
	buf_stat->readIO = 0;
	buf_stat->writeIO = 0;

	bm->pageFile = (char *)pageFileName;
	
	openPageFile(bm->pageFile, fh);
	switch (strategy) {
	case RS_FIFO:
		buf_stat->fh = fh;
		buf_stat->bufStat = head;
		buf_stat->pageNumStat = pageNumArr;
		buf_stat->fixCountStat = fixCountArr;
		buf_stat->dirtyStat = dirtyArr;
		break;
	case RS_LRU:
		buf_stat->fh = fh;
		buf_stat->bufStat = start_lru;
		buf_stat->pageNumStat = pageNumArr;
		buf_stat->fixCountStat = fixCountArr;
		buf_stat->dirtyStat = dirtyArr;
		break;
	case RS_CLOCK:
		buf_stat->fh = fh;
		buf_stat->bufStat = start_clock;
		buf_stat->pageNumStat = pageNumArr;
		buf_stat->fixCountStat = fixCountArr;
		buf_stat->dirtyStat = dirtyArr;
		break;
	default:
		return RC_IO_ERROR;
	}
	bm->strategy = strategy;
	bm->pageFile = (char *)pageFileName;
	bm->mgmtData = buf_stat;
	return RC_OK;
}

/*
 * AUTHOR : Priyank Shah
 * DESCRIPTION : Use to get buffer frame content
*/
PageNumber *getFrameContents(BM_BufferPool * const bm) {
	buffer_info_stat *stat;
	stat = bm->mgmtData;
	return stat->pageNumStat;
}

/*
 * AUTHOR : Priyank Shah
 * DESCRIPTION : Use to get buffer frame fixed count
*/
int *getFixCounts(BM_BufferPool * const bm) {
	buffer_info_stat *stat;
	stat = bm->mgmtData;
	return stat->fixCountStat;
}

/*
 * AUTHOR : Priyank Shah
 * DESCRIPTION : Use to get buffer frame dirty flag
*/
bool *getDirtyFlags(BM_BufferPool * const bm) {
	buffer_info_stat *stat;
	stat = bm->mgmtData;
	return stat->dirtyStat;
}

/*
 * AUTHOR : Rahi Shah & Jaimin Sanghvi
 * DESCRIPTION : Pin a pages according to page replacement strategy
*/
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) {
	int pos = -1;

	SM_PageHandle ph;
	ph = (SM_PageHandle)malloc(PAGE_SIZE);
	memset(ph, 0, PAGE_SIZE);
	
	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();
   
   struct LRU *buf_lru;
	buf_lru = MAKE_LRU_BUFFER();

	struct CLOCK *buf_clock;
	buf_clock = MAKE_CLOCK_BUFFER();
	
	buffer_info_stat *stat;	
	stat = MAKE_BUFFER_INFO_STAT();
	stat = bm->mgmtData;
	
	page->pageNum = pageNum;
	int freeFrame = 0;

	/* For LRU Page Replacement Stratergy */ 
	int sizeOfBuffer = 0;
   int cnt = 0;
	sizeOfBuffer = findSizeOfBuffer();

	/* For Clock Page Replacement Stratergy */
	int sizeOfClockBuffer = 0;
	sizeOfClockBuffer = findSizeOfClockBuffer();
	
	switch (bm->strategy){
		case RS_FIFO:
			if(pageNum<0)
				return RC_PAGE_NUMBER_NEGATIVE ;
			pos = searchFramePosition(pageNum);	/* Search frame position in Buffer Pool */		
			if (pos == -1){
				readBlock(pageNum, stat->fh, ph);
				page->data = ph;
				//printf("page->data is %s \n" , page->data);
				sprintf(page->data, "%s-%i", "Page", pageNum);			
				/* If buffer is empty */
				if (head == NULL && tail == NULL){
					buf_fifo->pageData = page->data;     /*Transfer page data to buffer*/
					buf_fifo->dirty = FALSE;
					buf_fifo->fixCount = 1;
					buf_fifo->pageNum = page->pageNum;
					buf_fifo->pagePos = 0;
					head = buf_fifo;
					tail = buf_fifo;
					stat->pageNumStat[0] = buf_fifo->pageNum;
					stat->fixCountStat[0] = buf_fifo->fixCount;
					stat->dirtyStat[0] = buf_fifo->dirty;					
					head->nextElement = tail;
					tail->nextElement = NULL;
					firstElement = head;/*Keeps track of 1st element so it can be removed once buffer gets full */		
					stat->readIO++;
				} else {	 			/* If buffer is not empty */
/*					printf("\n if buffer is not empty .. %s", page->data); */
					buf_fifo = head;
					int totPage = 0;
					while (buf_fifo != NULL){
						totPage = totPage + 1;
						buf_fifo = buf_fifo->nextElement;
					}
					/* If buffer is partial full*/
					if (totPage != bm->numPages){	
						buf_fifo = MAKE_QUEUE_BUFFER();						
						buf_fifo->nextElement = NULL;
						int lastPos = tail->pagePos;
						buf_fifo->pagePos = lastPos + 1;											
						buf_fifo->dirty = false;
						buf_fifo->fixCount = 1;
						buf_fifo->pageData = page->data;
						buf_fifo->pageNum = page->pageNum;
						tail->nextElement = buf_fifo;
						tail = buf_fifo;						
						stat->dirtyStat[buf_fifo->pagePos] = buf_fifo->dirty;
						stat->fixCountStat[buf_fifo->pagePos] = buf_fifo->fixCount;
						stat->pageNumStat[buf_fifo->pagePos] = buf_fifo->pageNum;	
						stat->readIO++;				
						
					} else {									/*If the buffer is full*/
						while (freeFrame != bm->numPages){	
						int pos = searchFramePosition(firstElement->pageNum);
						int cntAfterPinPage = stat->fixCountStat[pos];	
						if (cntAfterPinPage == 0){
							   /* special requirenment  that page has to be removed only when fixCount = 0 */
								if (firstElement->dirty == true){																	
									writeBlock(firstElement->pageNum, stat->fh, firstElement->pageData);
									stat->writeIO++;
								}												
								firstElement->dirty = false;
								firstElement->fixCount = 1;
								firstElement->pageData = page->data;
								firstElement->pageNum = page->pageNum;
								stat->pageNumStat[firstElement->pagePos] = firstElement->pageNum;
								stat->fixCountStat[firstElement->pagePos] = firstElement->fixCount;
								stat->dirtyStat[firstElement->pagePos] = firstElement->dirty;		
								stat->readIO++;								
								if (firstElement->nextElement == NULL){
									firstElement = head;
								}
								else{
									firstElement = firstElement->nextElement;
								}
								break;
							} else{	
								if (firstElement->nextElement == NULL){
									firstElement = head;									
								}
								else{									
									firstElement = firstElement->nextElement;
								}
								freeFrame++;
								if(freeFrame==2) {
									return RC_OVERLOAD_IN_POOL;
								}
							}
						}																								
					}
				}	
			} else {
				buf_fifo->pageNum = stat->pageNumStat[pos];
				buf_fifo->dirty = stat->dirtyStat[pos];
				buf_fifo->fixCount = stat->fixCountStat[pos];
				buf_fifo->pagePos = pos;

				buf_fifo->fixCount = buf_fifo->fixCount + 1;
				stat->fixCountStat[pos] = buf_fifo->fixCount;			
				stat->pageNumStat[pos] = buf_fifo->pageNum;				
				stat->dirtyStat[pos] = buf_fifo->dirty;
		/*		getchar(); */
			}
			break;							
		case RS_LRU:
			pos = searchLRUFramePosition(pageNum);	/* Search frame position in Buffer Pool */			
			if (pos == -1){
					readBlock(pageNum, stat->fh, ph);
					stat->readIO++;
					page->data = ph;
			}
			/* If buffer is empty */
			if (start_lru == NULL){					
					buf_lru->lastRecUsed = 1;
					buf_lru->dirty = false;
					buf_lru->fixCount = 1;
					buf_lru->pageData = page->data;
					buf_lru->pageNum = page->pageNum;
					buf_lru->pagePos = 0;
					start_lru = buf_lru;
					end_lru = buf_lru;
					start_lru->nextElement = end_lru;
					end_lru->nextElement = NULL;
					stat->pageNumStat[0] = buf_lru->pageNum;
					stat->dirtyStat[0] = buf_lru->dirty;
					stat->fixCountStat[0] = buf_lru->fixCount;
			} else {								
				pos = searchLRUFramePosition(pageNum);					
				if (pos == -1){
					if (sizeOfBuffer == bm->numPages){							
						buf_lru = start_lru;
						int position = 0;
						int maxAge = 0;
						maxAge = buf_lru->lastRecUsed;         //Find the max age........
						position = buf_lru->pagePos;							
						while (buf_lru != NULL){
							if (buf_lru->lastRecUsed > maxAge){
								maxAge = buf_lru->lastRecUsed;
								position = buf_lru->pagePos;									
							}								
							buf_lru = buf_lru->nextElement;
						}
						buf_lru = start_lru;
						while (buf_lru != NULL){
							if (buf_lru->dirty == true){
								writeBlock(buf_lru->pageNum, stat->fh, buf_lru->pageData);
								stat->writeIO++;
							}
							buf_lru = buf_lru->nextElement;
						}							
						buf_lru = MAKE_LRU_BUFFER();
						buf_lru = start_lru;
						while (buf_lru != NULL){
							if (buf_lru->pagePos == position){
								break;
							}
							buf_lru = buf_lru->nextElement;
						}							
						incrementAge();
						buf_lru->lastRecUsed = 1;
						buf_lru->pagePos = position;
						buf_lru->dirty = false;
						buf_lru->fixCount = 1;
						buf_lru->pageData = page->data;
						buf_lru->pageNum = page->pageNum;																										
						stat->fixCountStat[position] = buf_lru->fixCount;
						stat->dirtyStat[position] = buf_lru->dirty;
						stat->pageNumStat[position] = buf_lru->pageNum;										
					} else {		
						buf_lru = start_lru;														
						int maxPos = 0;
						maxPos = buf_lru->pagePos;         //Find the max page position........								
						while (buf_lru != NULL){
							if (buf_lru->pagePos > maxPos){
								maxPos = buf_lru->pagePos;
							}
							buf_lru = buf_lru->nextElement;
						}
						buf_lru = MAKE_LRU_BUFFER();
						buf_lru->nextElement = NULL;
						buf_lru->pagePos = (maxPos + 1);
						buf_lru->dirty = false;
						buf_lru->pageNum = page->pageNum;							
						buf_lru->fixCount = 1;
						incrementAge();
						buf_lru->lastRecUsed = 1;
						buf_lru->pageData = page->data;
						end_lru->nextElement = buf_lru;
						end_lru = buf_lru;							
						stat->fixCountStat[buf_lru->pagePos] = buf_lru->fixCount;
						stat->dirtyStat[buf_lru->pagePos] = buf_lru->dirty;
						stat->pageNumStat[buf_lru->pagePos] = buf_lru->pageNum;
					}
				}else{
					buf_lru = start_lru;
					while (buf_lru != NULL){
					if (buf_lru->pagePos == pos){
						break;
						}
						buf_lru = buf_lru->nextElement;
					}
					incrementAge();
					buf_lru->lastRecUsed = 1;
					buf_lru->dirty = false;
					buf_lru->pageData = page->data;
					buf_lru->fixCount = buf_lru->fixCount + 1;
					stat->fixCountStat[pos] = buf_lru->fixCount;
					stat->pageNumStat[pos] = buf_lru->pageNum;
					stat->dirtyStat[pos] = buf_lru->dirty;
				}
			}
			break;
		case RS_CLOCK:
				pos = searchFramePositionClock(pageNum);			/* Search frame position in Buffer Pool */
				/* If buffer is empty */
				if (pos == -1){
					if (start_clock == NULL){
						buf_clock->clockBit = 1;
						buf_clock->dirty = false;
						buf_clock->fixCount = 1;
						buf_clock->pageData = page->data;
						buf_clock->pageNum = page->pageNum;
						buf_clock->pagePos = 0;
						buf_clock->clockBit = 1;
						start_clock = buf_clock;
						end_clock = buf_clock;
						start_clock->nextElement = end_clock;
						end_clock->nextElement = NULL;
						firstClock = start_clock;
						stat->pageNumStat[0] = buf_clock->pageNum;
						stat->dirtyStat[0] = buf_clock->dirty;
						stat->fixCountStat[0] = buf_clock->fixCount;
					}
					else{
						if (sizeOfClockBuffer == bm->numPages){
							buf_clock = start_clock;
							while (freeFrame != bm->numPages){
								int pos = searchFramePositionClock(buf_clock->pageNum);
								int cntAfterPinPage = stat->fixCountStat[pos];
								if (cntAfterPinPage == 0){
									if (firstClock->clockBit == 0 && firstClock->fixCount == 1){
										firstClock = firstClock->nextElement;
								}
									else if (firstClock->clockBit == 1){
									firstClock->clockBit = 0;
									if (firstClock->nextElement == NULL){
										firstClock = start_clock;
										freeFrame = 0;
									}
									else{										
										firstClock = firstClock->nextElement;
									}															
								}
									else if (firstClock->clockBit == 0 && firstClock->fixCount == 0){
										if (firstClock->dirty == TRUE){
											writeBlock(firstClock->pageNum, stat->fh, firstClock->pageData);
											stat->writeIO++;
									}
										firstClock->dirty = false;
										firstClock->fixCount = 1;
										firstClock->clockBit = 1;
										firstClock->pageData = page->data;
										firstClock->pageNum = page->pageNum;
										stat->pageNumStat[firstClock->pagePos] = firstClock->pageNum;
										stat->fixCountStat[firstClock->pagePos] = firstClock->fixCount;
										stat->dirtyStat[firstClock->pagePos] = firstClock->dirty;
										break;
								}								
								freeFrame++;
								}
								else{
									if (firstClock->nextElement == NULL){
										firstClock = start_clock;
									}
									else{
										firstClock = firstClock->nextElement;
									}
									freeFrame++;
								}
							}
						}
						else{
							buf_clock = start_clock;
							int maxPos = 0;
							maxPos = buf_clock->pagePos;         //Find the max page position........								
							while (buf_clock != NULL){
								if (buf_clock->pagePos > maxPos){
									maxPos = buf_clock->pagePos;
								}
								buf_clock = buf_clock->nextElement;
							}
							buf_clock = MAKE_CLOCK_BUFFER();
							buf_clock->nextElement = NULL;
							buf_clock->pagePos = (maxPos + 1);
							buf_clock->dirty = false;
							buf_clock->clockBit = 1;
							buf_clock->pageNum = page->pageNum;
							buf_clock->fixCount = 1;														
							buf_clock->pageData = page->data;
							end_clock->nextElement = buf_clock;
							end_clock = buf_clock;
							stat->fixCountStat[buf_clock->pagePos] = buf_clock->fixCount;
							stat->dirtyStat[buf_clock->pagePos] = buf_clock->dirty;
							stat->pageNumStat[buf_clock->pagePos] = buf_clock->pageNum;
						}
					}
				}
				else{
					buf_clock = MAKE_CLOCK_BUFFER();
					buf_clock = start_clock;
					while (buf_clock->nextElement != NULL){
						if (pos == buf_clock->pagePos){
							buf_clock->fixCount = buf_clock->fixCount + 1;

							stat->fixCountStat[pos] = buf_clock->fixCount;
							stat->pageNumStat[pos] = buf_clock->pageNum;
							stat->dirtyStat[pos] = buf_clock->dirty;
							stat->readIO++;
						}
						buf_clock = buf_clock->nextElement;
					}
				}
				break;
			default:
      		return RC_STRATEGY_NOT_FOUND ;
	}
			return RC_OK;
}

/*
 * AUTHOR : Rahi Shah
 * DESCRIPTION : Increment page age for LRU Strategy
*/
RC incrementAge(){
	struct LRU *buf_lru;
	buf_lru = MAKE_LRU_BUFFER();
	buf_lru = start_lru;
	while (buf_lru != NULL){
		buf_lru->lastRecUsed = buf_lru->lastRecUsed + 1;
		buf_lru = buf_lru->nextElement;
	}
	return RC_OK;
}

/*
 * AUTHOR : Rahi Shah
 * DESCRIPTION : Use to find a size of buffer for FIFO Strategy
*/
int findSizeOfBuffer(){
	struct LRU *buf_lru;
	buf_lru = MAKE_LRU_BUFFER();
	buf_lru = start_lru;
	int cnt = 0;
	while (buf_lru != NULL){
		cnt++;
		buf_lru = buf_lru->nextElement;
	}
	return cnt;
}

/*
 * AUTHOR : Rahi Shah
 * DESCRIPTION : Use to find a size of clock strategy
*/
int findSizeOfClockBuffer(){
	struct CLOCK *buf_clock;
	buf_clock = MAKE_CLOCK_BUFFER();
	buf_clock = start_clock;
	int cnt = 0;
	while (buf_clock != NULL){
		cnt++;
		buf_clock = buf_clock->nextElement;
	}
	return cnt;
}

/*
 * AUTHOR : Priyank Shah
 * DESCRIPTION : Use to search a frame position in buffer for clock strategy
*/
int searchFramePositionClock(PageNumber pno){
	int pos = -1;
	struct CLOCK *buf_clock;
	buf_clock = MAKE_CLOCK_BUFFER();
	buf_clock = start_clock;
	while (buf_clock != NULL){		
		if (buf_clock->pageNum == pno){			
			pos = buf_clock->pagePos;
			break;
		}
		buf_clock = buf_clock->nextElement;
	}
	return pos;
}

/*
 * AUTHOR : Priyank Shah
 * DESCRIPTION : Use to search a frame position in buffer for FIFO strategy
*/
int searchFramePosition(PageNumber pno){	
	int pos = -1;		
	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();
	buf_fifo = head;
	while (buf_fifo != NULL){						
		if (buf_fifo->pageNum == pno){
			pos = buf_fifo->pagePos;
			break;
		}
		buf_fifo = buf_fifo->nextElement;
	}	
	return pos;
}

/*
 * AUTHOR : Priyank Shah
 * DESCRIPTION : Use to search a frame position in buffer for LRU strategy
*/
int searchLRUFramePosition(PageNumber pno){
	int pos = -1;
	struct LRU *buf_lru;
	buf_lru = MAKE_LRU_BUFFER();
	buf_lru = start_lru;
	while (buf_lru != NULL){		
		if (buf_lru->pageNum == pno){			
			pos = buf_lru->pagePos;
			break;
		}
		buf_lru = buf_lru->nextElement;
	}
	return pos;
}

/*
 * AUTHOR : Jaimin Sanghvi
 * DESCRIPTION : Use to mark a dirty flag to updated page
*/
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {
	buffer_info_stat *stat;

	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();

	struct LRU *buf_lru;
	buf_lru = MAKE_LRU_BUFFER();
 
	struct CLOCK *buf_clock;
   buf_clock = MAKE_CLOCK_BUFFER();

	stat = bm->mgmtData;

/*	if(head==NULL){
      return RC_PAGE_NOT_FOUND ;
   }*/

	int pos = -1;	
	switch (bm->strategy){
		case RS_FIFO:						
			pos = searchFramePosition(page->pageNum);
			if(head==NULL){
         	return RC_PAGE_NOT_FOUND ;
      	}
			writeBlock(page->pageNum, stat->fh, page->data);
			buf_fifo = head;
			while (buf_fifo != NULL){
				if (buf_fifo->pagePos == pos){					
					buf_fifo->pageData = page->data;
					stat->dirtyStat[buf_fifo->pagePos] = TRUE;					
				}
				buf_fifo = buf_fifo->nextElement;
			}
		case RS_LRU:
			pos = searchLRUFramePosition(page->pageNum);
			buf_lru = start_lru;
			while (buf_lru != NULL){						
			if (buf_lru->pagePos == pos){				
				buf_lru->fixCount = stat->fixCountStat[pos];
				buf_lru->fixCount = buf_lru->fixCount - 1;
				stat->fixCountStat[pos] = buf_lru->fixCount;
				break;
			}
			buf_lru = buf_lru->nextElement;
		}
		break;
		case RS_CLOCK:
			pos = searchFramePositionClock(page->pageNum);
			buf_clock = start_clock;
			while (buf_clock != NULL){
				if (buf_clock->pagePos == pos){
					buf_clock->pageData = page->data;
					stat->dirtyStat[buf_clock->pagePos] = TRUE;
				}
				buf_clock = buf_clock->nextElement;
			}
			break;
		default:
         return RC_STRATEGY_NOT_FOUND ;
	}
	return RC_OK;
}

/*
 * AUTHOR : Jaimin Sanghvi
 * DESCRIPTION : Unpin a page from buffer pool
*/
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page){
	int pos = -1;
	buffer_info_stat *stat;	
	stat = bm->mgmtData;
	
	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();
	
	struct LRU *buf_lru;
   buf_lru = MAKE_LRU_BUFFER();
 
   struct CLOCK *buf_clock;
	buf_clock = MAKE_CLOCK_BUFFER();

	switch (bm->strategy){
	case RS_FIFO:
		pos = searchFramePosition(page->pageNum);	
		if(head==NULL){
      	return RC_PAGE_NOT_FOUND ;
   	}	
		buf_fifo = head;
		while (buf_fifo != NULL){			
			if (buf_fifo->pagePos == pos){								
				buf_fifo->fixCount = stat->fixCountStat[pos];
				buf_fifo->fixCount = buf_fifo->fixCount - 1;
				stat->fixCountStat[pos] = buf_fifo->fixCount;				
				break;
			}			
			buf_fifo = buf_fifo->nextElement;
		}
		break;
	case RS_LRU:
		pos = searchLRUFramePosition(page->pageNum);
		buf_lru = start_lru;
		while (buf_lru != NULL){						
			if (buf_lru->pagePos == pos){				
				buf_lru->fixCount = stat->fixCountStat[pos];
				buf_lru->fixCount = buf_lru->fixCount - 1;
				stat->fixCountStat[pos] = buf_lru->fixCount;
				break;
			}
			buf_lru = buf_lru->nextElement;
		}
		break;
	case RS_CLOCK:
		pos = searchFramePositionClock(page->pageNum);
		buf_clock = start_clock;
		while (buf_clock != NULL){
			if (buf_clock->pagePos == pos){
				buf_clock->fixCount = stat->fixCountStat[pos];
				buf_clock->fixCount = buf_clock->fixCount - 1;
				stat->fixCountStat[pos] = buf_clock->fixCount;
				break;
			}
			buf_clock = buf_clock->nextElement;
		}
		break;
	default:
      return RC_STRATEGY_NOT_FOUND ;
	}
	return RC_OK;
}

/*
 * AUTHOR : Jaimin Sanghvi
 * DESCRIPTION : Use to shutdown buffer pool
*/
RC shutdownBufferPool(BM_BufferPool *const bm){
	/*printf("\n shutting down a buffer pool.");	*/
	
	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();
	
	struct LRU *buf_lru;
   buf_lru = MAKE_LRU_BUFFER();

   struct CLOCK *buf_clock;
   buf_clock = MAKE_CLOCK_BUFFER();
	
	buffer_info_stat *stat;
	stat = bm->mgmtData;

	switch (bm->strategy){
		case RS_FIFO:						
			buf_fifo = head;
			while (buf_fifo != NULL){			
				if (buf_fifo->dirty == TRUE){
					writeBlock(buf_fifo->pageNum, stat->fh, buf_fifo->pageData);	
					stat->writeIO++;
				}
				buf_fifo = buf_fifo->nextElement;
			}
			break;
		case RS_LRU:
         buf_lru = start_lru;
         while (buf_lru != NULL){
         	if (buf_lru->dirty == TRUE){
            	writeBlock(buf_lru->pageNum, stat->fh, buf_lru->pageData);
               stat->writeIO++;
            }
            buf_lru = buf_lru->nextElement;
         }
         break;
      case RS_CLOCK:
         buf_clock = start_clock;
         while (buf_clock != NULL){
         	if (buf_clock->dirty == TRUE){
            	writeBlock(buf_clock->pageNum, stat->fh, buf_clock->pageData);
               stat->writeIO++;
            }
            buf_clock = buf_clock->nextElement;
         }
         break;
		default:
         return RC_STRATEGY_NOT_FOUND ;
	}			
	closePageFile(stat->fh);
	buf_fifo = NULL;
	head = NULL;
	tail = NULL;
	firstElement = NULL;
	buf_lru = NULL;		
	free(buf_fifo);
	free(head);
	free(tail);
	free(firstElement);
	free(buf_lru);		
	return RC_OK;
}

/*
 * AUTHOR : Jaimin Sanghvi
 * DESCRIPTION : Use to write a page from buffer to storage memory
*/
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {
	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();

	buffer_info_stat *stat;
	stat = bm->mgmtData;

	int pos = -1;

	switch (bm->strategy){
	case RS_FIFO:
		pos = searchFramePosition(page->pageNum);	
		if(head==NULL){
         return RC_PAGE_NOT_FOUND ;
      }	
		buf_fifo = head;
		while (buf_fifo != NULL){				
			if (buf_fifo->pagePos == pos){					
				writeBlock(page->pageNum, stat->fh, page->data);
				stat->writeIO++;
				stat->dirtyStat[pos] = false;
			}
			buf_fifo = buf_fifo->nextElement;
		}		
		break;
	case RS_LRU:
       pos = searchLRUFramePosition(page->pageNum);
       buf_lru = start_lru;
       while (buf_lru != NULL){
          if (buf_lru->pagePos == pos){
             writeBlock(page->pageNum, stat->fh, page->data);
             stat->writeIO++;
             stat->dirtyStat[pos] = false;
          }
          buf_lru = buf_lru->nextElement;
       }
       break;
    case RS_CLOCK:
       pos = searchFramePositionClock(page->pageNum);
       buf_clock = start_clock;
       while (buf_clock != NULL){
          if (buf_clock->pagePos == pos){
             writeBlock(page->pageNum, stat->fh, page->data);
             stat->writeIO++;
             stat->dirtyStat[pos] = false;
          }
          buf_clock = buf_clock->nextElement;
       }
       break;
	 default:
       return RC_STRATEGY_NOT_FOUND ;
	}
	return RC_OK;
}

/*
 * AUTHOR : Neel Desai
 * DESCRIPTION : Use to flush a buffer pool
*/
RC forceFlushPool(BM_BufferPool * const bm){
	struct FIFO *buf_fifo;
	buf_fifo = MAKE_QUEUE_BUFFER();
	
	struct LRU *buf_lru;
   buf_lru = MAKE_LRU_BUFFER();

   struct CLOCK *buf_clock;
   buf_clock = MAKE_CLOCK_BUFFER();	

	buffer_info_stat *stat;
	stat = bm->mgmtData;
	int i = 0;
	
	if(head==NULL){
		return RC_FORCE_FLUSH_POOL_NOT_OPEN ;
	}

	switch (bm->strategy){
	case RS_FIFO:		
		buf_fifo = head;		
		while (buf_fifo != NULL){
			/*printf("\n pageNumber = %d ", buf_fifo->pageNum);
			printf("\n fixCount = %d ", buf_fifo->fixCount);
			printf("\n pagePosition = %d ", buf_fifo->pagePos);*/
				writeBlock(buf_fifo->pageNum, stat->fh, buf_fifo->pageData);
				stat->writeIO++;				
				stat->dirtyStat[buf_fifo->pagePos] = false;
			buf_fifo = buf_fifo->nextElement;
			i++;
		}
		/*printf("\n No of pages are written to storage = ::%d", i);*/
		break;
	case RS_LRU:
   	buf_lru = start_lru;
      while (buf_lru != NULL){
      	writeBlock(buf_lru->pageNum, stat->fh, buf_lru->pageData);
         stat->writeIO++;
         stat->dirtyStat[buf_lru->pagePos] = false;
         buf_lru = buf_lru->nextElement;
      	i++;
      }
		break;
	case RS_CLOCK:
      buf_clock = start_clock;
      while (buf_clock != NULL){
         writeBlock(buf_clock->pageNum, stat->fh, buf_clock->pageData);
         stat->writeIO++;
         stat->dirtyStat[buf_clock->pagePos] = false;
         buf_clock = buf_clock->nextElement;
      	i++;
      }
      break;
	default:
      return RC_STRATEGY_NOT_FOUND ;
	}
	return RC_OK;
}

/*
 * AUTHOR : Neel Desai
 * DESCRIPTION : Use to get Read I/O count
*/
int getNumReadIO(BM_BufferPool * const bm){
	buffer_info_stat *stat;
	stat = bm->mgmtData;
	return stat->readIO;
}

/*
 * AUTHOR : Neel Desai
 * DESCRIPTION : Use to get write I/O count
*/
int getNumWriteIO(BM_BufferPool * const bm){
	buffer_info_stat *stat;
	stat = bm->mgmtData;
	return stat->writeIO;
}
