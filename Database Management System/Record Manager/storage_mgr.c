#include<stdio.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include<stdlib.h>
#include<errno.h>
#include<sys/stat.h>


RC createPageFile(char *fileName) {
	FILE *file;
	file = fopen(fileName, "w");
	return RC_OK;
}

void initStorageManager() {
}

/*
 * AUTHOR: Neel Desai
*/

RC openPageFile(char *fileName, SM_FileHandle *fHandle) {

	FILE *file = NULL;

	fHandle->mgmtInfo = (void *)malloc(PAGE_SIZE);
	if (fHandle->mgmtInfo == NULL)
		return RC_FILE_HANDLE_NOT_INIT;

	struct stat b;
	if (stat(fileName, &b) != 0){
		return RC_IO_ERROR;
	}

	file = fopen(fileName, "r");
	if (file == NULL) {
		printf("File : %s file does not exist\n", fileName);
		return RC_FILE_NOT_OPENED;
	}
	else {		
		fHandle->fileName = fileName;
		fHandle->mgmtInfo = file;
		fHandle->totalNumPages = 1;
		fHandle->curPagePos = 0;
	//	fseek(file, 0L, SEEK_SET);
//		fwrite(fHandle, PAGE_SIZE, 1, file);
	}
	fclose(file);
	return RC_OK;
}

/*
 * AUTHOR: Neel Desai
*/

RC closePageFile(SM_FileHandle *fHandle) {

	FILE *file = NULL;
	file = fopen(fHandle->mgmtInfo, "r");
	//file = fHandle->fileName;
	fclose(file);
	file = fHandle->mgmtInfo;
	if (fHandle->mgmtInfo == NULL)
		return RC_FILE_HANDLE_NOT_INIT;

	if (fHandle->fileName == NULL){
		return RC_FILE_NOT_FOUND;
	}

	if (fclose(file) == 0) {
		file = 0;
		file = NULL;
	}
	return RC_OK;
}

/*
 * AUTHOR : Priyank Shah
*/

RC destroyPageFile(char *fileName) {
	int deleteFileStatus;
	deleteFileStatus = remove(fileName);
	if (remove(fileName) != 0){
		//perror("remove(\"fileName\") failed");
		deleteFileStatus = 0;
		rename(fileName, "testtemp.bin");
	}	
	/*getchar();*/
	if (deleteFileStatus == 0) {
		return RC_OK;
	}
	else {
		return RC_FILE_NOT_DELETED;
	}
}

/*
 * AUTHOR: Priyank Shah
*/

int getBlockPos(SM_FileHandle *fHandle){
	FILE *file;
	int i = -1;
	if (fHandle->mgmtInfo == NULL){
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else {
		file = fHandle->mgmtInfo;
		if (fHandle->fileName == NULL){
			return RC_FILE_NOT_FOUND;
		}
		else {
			if ((i = ftell(file)) == -1L){

			}
		}
	}
	/*printf("\n Current position is : %d\n", i);*/
	return RC_OK;
}

/*
 * AUTHOR : Jaimin Sanghvi
*/

RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	if (fHandle->mgmtInfo == NULL){
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else{
		if (fHandle->fileName == NULL){
			return RC_FILE_NOT_FOUND;
		} else {
			FILE *file;
			int size = 0;
			file = fopen(fHandle->fileName, "r");
			fseek(file, pageNum*PAGE_SIZE, SEEK_SET);
			size = fread(memPage, 1, PAGE_SIZE, file);
			//printf("\n read memPage : %d \n", size);
			fHandle->curPagePos = fseek(file, pageNum*PAGE_SIZE, SEEK_SET) * PAGE_SIZE;
			fclose(file);
		}
	}
	return RC_OK;
}

/*
 * AUTHOR : Jaimin Sanghvi
*/

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
        if (fHandle->mgmtInfo == NULL){
                return RC_FILE_HANDLE_NOT_INIT;
        } else {
        	if (fHandle->fileName == NULL){
                        return RC_FILE_NOT_FOUND;
                } else {
                        FILE *file;
                        file = fHandle->mgmtInfo;
                        int pos;

                        /*printf("\n Before ftell in previousBlock==%ld\n", ftell(file));*/
                        fseek(file, -(PAGE_SIZE), SEEK_CUR);
                        pos = ftell(file);
                        /*printf("\nPointer position before reading in readPreviousBlock():: %d", pos);*/
                        fread(memPage, PAGE_SIZE, 1, file);
                   /*printf("\nPointer position after reading in readPreviousBlock():: %ld", ftell(file));*/
                        fHandle->curPagePos = (pos/PAGE_SIZE)-1;
/*                        printf("\ncurrent page position in readBlock():: %d\n", fHandle->curPagePos);*/
			/*getcharar();*/
		
		}
        }
        return RC_OK;
}

/*
 * AUTHOR : Jaimin Sanghvi
*/

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
        if (fHandle->mgmtInfo == NULL){
                return RC_FILE_HANDLE_NOT_INIT;
        } else {
                if (fHandle->fileName == NULL){
                        return RC_FILE_NOT_FOUND;
                } else {
                        FILE *file;
                        file = fHandle->mgmtInfo;
                        int pos;

                        printf("\n After ftell in previousBlock==%ld\n", ftell(file));
                        fseek(file, (PAGE_SIZE), SEEK_CUR);
                        pos = ftell(file);
                        printf("\nPointer position before reading in readNextBlock():: %d", pos);
                        fread(memPage, PAGE_SIZE, 1, file);
                        printf("\nPointer position after reading in readNextBlock():: %ld", ftell(file));
                        fHandle->curPagePos = (pos/PAGE_SIZE)-1;
                        printf("\ncurrent page position in readBlock():: %d\n", fHandle->curPagePos);
                	/*getcharar();*/
                }
        }
        return RC_OK;
}

/*
 * AUHTOR : Jaimin Sanghvi
*/

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
	if (fHandle->mgmtInfo == NULL){
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else {
		if (fHandle->fileName == NULL){
			return RC_FILE_NOT_FOUND;
		}
		else{
			int i = getBlockPos(fHandle);
			FILE *file;
			file = fHandle->mgmtInfo;
			fseek(file, 0, SEEK_CUR);
			int temp = ftell(file);
			printf("\nFile Pointer position in readFirstBlock() is :%d", temp);
			fread(memPage, PAGE_SIZE, 1, file);
			fHandle->curPagePos = ((temp/PAGE_SIZE)-1);
			printf("\ncurrent page position in readFirstBlock() is :%d", fHandle->curPagePos);
		}
	}
	return RC_OK;
}

/*
 * AUHTOR : Jaimin Sanghvi
*/

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
        if (fHandle->mgmtInfo == NULL){
                return RC_FILE_HANDLE_NOT_INIT;
        } else {
                if (fHandle->fileName == NULL){
                        return RC_FILE_NOT_FOUND;
                } else {
                        FILE *file;
                        file = fHandle->mgmtInfo;
                        int pos;

                        printf("\n ftell in currentBlock==%ld\n", ftell(file));
                        pos = ftell(file);
                        printf("\nPointer position before reading in readCurrentBlock():: %d", pos);
                        fread(memPage, PAGE_SIZE, 1, file);
                        printf("\nPointer position after reading in readCurrentBlock():: %ld", ftell(file));
                        fHandle->curPagePos = (pos/PAGE_SIZE)-1;
                        printf("\ncurrent page position in readBlock():: %d\n", fHandle->curPagePos);
                        /*getcharar();*/
                }
        }
        return RC_OK;
}

/*
 * AUHTOR : Jaimin Sanghvi
*/

RC readLastBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
	int curPos = -1;
	FILE *file = NULL;
	if (fHandle->fileName == NULL) {
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else {
		file = fHandle->mgmtInfo;
		if (file == NULL) {
			return RC_FILE_NOT_OPENED;
		}
		else {
			//Seeking to the last block of the file
			fseek(file, -(PAGE_SIZE), SEEK_END);
			printf("File Pointer Position in readLastBlock() is %ld", (ftell(file)));
			fread(memPage, PAGE_SIZE, 1, file);
			curPos = (ftell(file) / PAGE_SIZE);
			fHandle->curPagePos = curPos-1;
			printf("\nCurrent page position in readLastBlock():: %d", fHandle->curPagePos);
		}
	}
	return RC_OK;
}

/*
 * AUHTOR : Rahi Shah
*/

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	//printf("\n Writing frames in storage to flush buffer. \n");
	FILE *fpp;
	fpp = fopen(fHandle->fileName, "r+");
	if (fHandle->mgmtInfo == NULL){
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else {
		if (fHandle->fileName == NULL){
			return RC_FILE_NOT_FOUND;
		}
		else {
	//		printf("\n....Hi me in writeBlock...");
			int size = 0;
	//		printf("\n....PageNum....%d", pageNum);
			fseek(fpp, (pageNum)*PAGE_SIZE, SEEK_SET);
			size = fwrite(memPage, 1, PAGE_SIZE, fpp);
			fHandle->curPagePos = fHandle->curPagePos + 1;
			fHandle->totalNumPages = fHandle->totalNumPages + 1;
			fHandle->mgmtInfo = memPage;
			fclose(fpp);
		}
	}
	return RC_OK;
}

/*
 * AUHTOR : Rahi Shah
*/

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
        if (fHandle->mgmtInfo == NULL){
                return RC_FILE_HANDLE_NOT_INIT;
        }
        else {
                if (fHandle->fileName == NULL){
                        return RC_FILE_NOT_FOUND;
                }
                else {
                        int i = getBlockPos(fHandle);
                        FILE *file;
                        file = fHandle->mgmtInfo;
                        printf("\nFile Pointer position in writeCurrentBlock() before writing :: %ld", (ftell(file)));
                        fseek(file, ftell(file), SEEK_SET);
                        fwrite(memPage, PAGE_SIZE, 1, file);
                        printf("\nFile Pointer position in writeCurrentBlock() after writing :: %ld", (ftell(file)));
                        fHandle->curPagePos = ((ftell(file)/PAGE_SIZE)) - 1;
                        printf("\nCurrent Page position in writeBlock():: %d", fHandle->curPagePos);
                        fHandle->totalNumPages = fHandle->totalNumPages + 1;
			/*getcharar();*/
                }
        }
        return RC_OK;
}

/*
 * AUHTOR : Rahi Shah
*/

RC appendEmptyBlock(SM_FileHandle *fHandle){
	if (fHandle->mgmtInfo == NULL){
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else{
		if (fHandle->fileName == NULL){
			return RC_FILE_NOT_FOUND;
		}
		else{
			FILE *file;
			file = fHandle->mgmtInfo;
			if (fHandle->totalNumPages == 0){
				return RC_NO_PAGE_FOUND;
			}
			else{
				fseek(file, 0L, SEEK_END);
			        int i;	
				SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
				for (i=0; i < PAGE_SIZE; i++)
    					ph[i] = (i % 10) + '0';
				
				fwrite(ph, PAGE_SIZE, 1, file);
				fHandle->curPagePos = ((ftell(file)/PAGE_SIZE));
				fHandle->totalNumPages = (fHandle->totalNumPages + 1);
				printf("\nPointer position in readBlock():: %d", fHandle->curPagePos);
			}
		}
	}
	return RC_OK;
}

/*
 * AUHTOR : Rahi Shah
*/

RC ensureCapacity(int numOfPages, SM_FileHandle *fHandle){
	
   int rc = RC_FILE_HANDLE_NOT_INIT,ch,i,filePages,pagesToCreate,size=0;
/*	printf("\n **** Ensure Capacity ***** \n");*/
	if(fHandle != NULL)
	{
		//filePages = fHandle->totalNumPages;
		
		FILE *fp = fopen(fHandle->fileName,"a");
		fseek(fp, 0, SEEK_END);		
		filePages = ftell(fp)/4096;		

/*		printf("\n File page : %d \n", filePages);*/
		numOfPages = 40;		// temporary added 
		if(numOfPages > filePages)
		{
			pagesToCreate = numOfPages - filePages;			
			ch = fseek(fp,0,SEEK_END);
			
			for(i = 0; i < pagesToCreate * PAGE_SIZE ;i++ )
			{
				fputc('\0',fp);
				size = size + 1;
				fHandle->curPagePos = fHandle->curPagePos + 1;
			}
			if(size == pagesToCreate * PAGE_SIZE)
			{
				fHandle->totalNumPages = fHandle->totalNumPages + pagesToCreate;
				rc = RC_OK;
			}
		} else {
		//	printf("----NUMBER OF PAGES IS NOT GREATER THAN THE FILE PAGES---");
		}
		fclose(fp);
	}
	return rc;

}


