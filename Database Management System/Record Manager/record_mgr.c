#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h>

#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "dt.h"
#include "db.h"
#include "tables.h"
#include "record_mgr.h"

extern RC update_table_mgmt_information(Schema *schema, char *data, int dataBlock, TableInfoUpdate type);
extern RC update_data_block_header_information(char *data, BlkHeaderUpdate type, int blk_num, int num_slots, int num_records);

char *storeBlocks , *delete_slot1, *delete_slot2;
BM_PageHandle *pageHandler ;
int offset=0, page_number=0, slot_number=0, page_counter=1;

typedef struct ScanData {
    Expr *cond;
    int currRecord;
    int numRecords;
    int no_blocks;
    int currentblock;
    int parsedRecords;
} ScanData;

/*
* AUTHOR : Neel Desai
* DESCRIPTION : Initialize Record Manager
*/
RC initRecordManager(void* mgmtData) {
	 storeBlocks = malloc(4096);		//Allocate 4096bytes of memory to storeBlocks
	 pageHandler = MAKE_PAGE_HANDLE();	
	 pageHandler->data = malloc(4096);	//Allocate 4096bytes of memory to page Handler data
    return RC_OK;
}

/*
* AUTHOR : Neel Desai
* DESCRIPTION : Shutdown Record manger
*/
RC shutdownRecordManager() {
    return RC_OK;
}

/*
* AUTHOR : Priyank Shah
* DESCRIPTION : Create table , initialize buffer pool, pinPage and unpinPage ,Update table management Info and Header
*/
RC createTable(char* name, Schema* schema) {
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //create the page file
    createPageFile(name);	//Create page file on disk
    initBufferPool(bm, name, 3, RS_FIFO, NULL);	//Initialize buffer pool
    //Insert the table mgmt info into TABLE INFO BLOCK
    pinPage(bm, bh, TABLE_INFO_BLOCK);	//pin the page in to buffer pool
   
    update_table_mgmt_information(schema, bh->data, 0, TBL_NEW);	//Update table mgmt info with new bh->data
	
    markDirty(bm, bh);	//Mark the page as dirty
    unpinPage(bm, bh);	//unpin page in buffer pool
    //Initialize the data block with the block Id 
    pinPage(bm, bh, DATA_INFO_BLOCK);	//pin the page in to buffer pool
    update_data_block_header_information(bh->data, BLK_NEW, 0, 0, 0);	//Update block header with new bh->data
    markDirty(bm, bh);	//Mark the page as dirty
    unpinPage(bm, bh);	//unpin page in buffer pool
    
    //Write the content to the file
    forceFlushPool(bm);	//Check if the page is dirty and write it back to disk
    //Shut down the buffer pool
    shutdownBufferPool(bm);	//Deallocate memory
    free(bm);
    return RC_OK;
}

/*
 * * AUTHOR : Jaimin Sanghvi
 * * DESCRIPTION : Open table and initialize table data
 * */
RC openTable(RM_TableData* rel, char* name) {

    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();
    int i = 0;
    page_number=0;
    rel->name = name;
    rel->mgmtData = malloc(sizeof(bm));
    rel->pageNumber = 1;
    rel->offset = 0;
    initBufferPool(bm, name, 3, RS_FIFO, NULL); //Initialize buffer pool
    pinPage(bm, bh, TABLE_INFO_BLOCK); //pin the page in to buffer pool

    unpinPage(bm, bh);  //unpin page in buffer pool
    rel->mgmtData = bm;
    free(bh);
    return RC_OK;
}

/*
 * * AUTHOR : Neel Desai
 * * DESCRIPTION : close the table and deallocate memory
 * */
RC closeTable(RM_TableData *rel) {
    shutdownBufferPool(rel->mgmtData); //Deallocate memory
    return RC_OK;
}

/*
 * * AUTHOR : Neel Desai
 * * DESCRIPTION : Delete Table
 * */
RC deleteTable(char *name) {
    destroyPageFile(name); //Delete the file
    return RC_OK;
}

/*
* AUTHOR : Rahi Shah
* DESCRIPTION : create record, get record size and allocate memory
*/
RC createRecord(Record** record, Schema* schema) {
    unsigned int i;
    unsigned int size_Record = 0;

    //allocating memory to record
    *record = (Record *) malloc(sizeof (Record));
    //(*record)->data = (char *) malloc(size_Record);
    (*record)->data = (char *) malloc(40);

    return RC_OK;
}

/*
 * * AUTHOR : Priyank Shah
 * * DESCRIPTION : close Scan and deallocate memory
 * */
RC closeScan(RM_ScanHandle *scan) {
    free(scan->mgmtData);  //Free memory of scan->mgmtData
    return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : Fetch record from disk, update record and write it back to disk.
*/
RC updateRecord(RM_TableData *rel, Record *record) {
    //variable declaration
    unsigned int offset = 0, rec_offset = 0, size_record = 0;
    unsigned int slot_offset = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    printf("\n updateRecord :  record page is *** %d \n", record->id.page);
    printf("\n updateRecord :  record slot is *** %d \n", record->id.slot);
    
    //calculating the offsets
    size_record = 12;
    offset = offset + (3 * sizeof (int));	//Calculate offset
    slot_offset = record->id.slot;

    pinPage(rel->mgmtData, bh, record->id.page);	//pin the page in to buffer pool
	 
    //updating the record
    rec_offset =  record->id.slot*16;
    memcpy(bh->data + (record->id.slot*16), record->data, size_record);	 // Copy size_record of size memory of record->data to (bh->data + offset) position
	 
    unpinPage(rel->mgmtData, bh);	//unpin page in buffer pool
    forceFlushPool(rel->mgmtData);	//check if the page is dirty if it is dirty write it back to disk and Deallocate memory 
    
    free(bh);
    return RC_OK;
}

/*
* AUTHOR : Rahi Shah
* DESCRIPTION : free record and deallocate memory
*/
RC freeRecord(Record *record) {
    free(record->data);
    free(record);
	return RC_OK;
}

int getNumTuples(RM_TableData *rel) {
        return RC_OK ;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : create schema
*/
Schema *createSchema(int numAttr, char** attrNames, DataType* dataTypes, int* typeLength, int keySize, int* keys) {

    Schema *schema;

    //create the schema and assign the values
    schema = (Schema *) malloc(sizeof (Schema));	//Allocate memory block of schema
    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keySize = keySize;
    schema->keyAttrs = keys;

    //return schema
    return schema;
}

RC next(RM_ScanHandle *scan, Record *record) {
    return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : get attribute from record
*/
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
    unsigned int i = 0;
    char *attrData;
    Value *val;
    int inum = 0;
    int len = 0;
    float fnum;
    bool bnum;

    if (record == NULL || schema == NULL)	//If record and schema is NULL then return RC Error 
        return RC_FUNC_INVALIDARG;

    val = (Value *) malloc(sizeof (Value));

    //attrOffset(schema, attrNum, &offset);
	if(attrNum==0)
      offset=0;
   else if(attrNum==1)
      offset=4;
   else if(attrNum==2)
      offset=8;

    val->dt = schema->dataTypes[attrNum];
    switch (schema->dataTypes[attrNum]) {

        case DT_INT:
				attrData = record->data + offset;
            len = schema->typeLength[attrNum];
            val->v.stringV = (char *) malloc(len + 1);
            sprintf(val->v.stringV, "%s" ,attrData);
            val->v.intV = atoi(val->v.stringV);
            break;
        case DT_STRING:
            attrData = record->data + offset;
            len = schema->typeLength[attrNum];
            val->v.stringV = (char *) malloc(len + 1);
            strncpy(val->v.stringV, attrData, len);
            val->v.stringV[len] = '\0';
            break;
        case DT_FLOAT:
            memcpy(&fnum, record->data + offset, sizeof (float));
            val->v.floatV = fnum;
            break;
        case DT_BOOL:
				memcpy(&bnum, record->data + offset, sizeof (bool));
            val->v.boolV = bnum;
            break;
        default:
            free(val);
            return RC_FUNC_INVALIDARG;
    }
    *value = val;
    return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : fetch record from disk according to rid
*/
RC getRecord(RM_TableData *rel, RID id, Record *record) {
	//variable declaration
    unsigned int size_record = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //calculating the offsets
    size_record = 12;

	 SM_PageHandle ph;
    ph = (SM_PageHandle)malloc(PAGE_SIZE);	//Allocate memory block of 4096 
    memset(ph, 0, PAGE_SIZE);	

	 SM_FileHandle *fh;
	 fh = (SM_FileHandle *)malloc(sizeof(SM_FileHandle)); 
	
	 openPageFile(rel->name, fh);	//Open the page file and initialize
	 readBlock(id.page, fh, ph);	//Access read block of storage manger to read data and put them in ph

	 int firstOffset = id.slot*16;

    memcpy(record->data, ph+firstOffset, size_record);	//copy block of size_record memory to record->data

	 slot_number++;
	
	 if(slot_number*16>=4096)  {
		 id.slot = 0;
	    id.page = id.page+1;	
	 }
    free(bh);
    return RC_OK ;
}

/*
* AUTHOR : Neel Desai
* DESCRIPTION : find record size
*/
int getRecordSize(Schema *schema) {
	unsigned int i;
   unsigned int sizeofRecord = 0;

   //calculating the size of record
   for (i = 0; i < schema->numAttr; i++) {
        switch (schema->dataTypes[i]){ 
            case DT_INT:
                sizeofRecord = sizeofRecord + sizeof (int);
                break;
            case DT_STRING:
                sizeofRecord = sizeofRecord + schema->typeLength[i];
                break;
            case DT_FLOAT:
                sizeofRecord = sizeofRecord + sizeof (float);
                break;
            case DT_BOOL:
                sizeofRecord = sizeofRecord + sizeof (bool);
                break;
        }	
   }
   return sizeofRecord;
}

/*
* AUTHOR : Priyank Shah
* DESCRIPTION : delete record and write it back to disk
*/
RC deleteRecord(RM_TableData *rel, RID id) {
	//variable declaration
    unsigned int offset = 0, rec_offset = 0, size_record = 0;
    unsigned int slot_offset = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    printf("\n deleteRecord :  record page is *** %d \n", id.page);
    printf("\n deleteRecord :  record slot is *** %d \n", id.slot);

    //calculating the offsets
    size_record = 12;
    offset = offset + (3 * sizeof (int));
    slot_offset = id.slot;

    pinPage(rel->mgmtData, bh, id.page);	//pin the page in to buffer pool

	 //deleting the record
    rec_offset =  (id.slot)*16;

	 delete_slot2 = calloc(1,16);	//NULL memory allocation
	  
	 memcpy(bh->data+rec_offset, delete_slot2,16);
   
    unpinPage(rel->mgmtData, bh);	//unpin page in buffer pool
    forceFlushPool(rel->mgmtData);	//check if the page is dirty if it is dirty write it back to disk and Deallocate memory 
    free(bh);
	 return RC_OK;
}

/*
* AUTHOR : Rahi Shah
* DESCRIPTION : set attribute 
*/
RC setAttr(Record* record, Schema* schema, int attrNum, Value* value) {
   //variable declaration
   unsigned int i = 0;
 	char array[6]="";
	char *var ;
	
   if(attrNum==0)
		offset=0;
	else if(attrNum==1)
		offset=4;
	else if(attrNum==2)
		offset=8;
    
   switch (schema->dataTypes[attrNum]) {
        case DT_INT:
        {
            int num = value->v.intV;
				sprintf(array,"%d", value->v.intV);
				var = array;
            memcpy(record->data + offset, var, sizeof (int));	//copy 4 bytes to record->data
        }
        break;
        case DT_STRING:
        {
            char *buf;
            int len = schema->typeLength[attrNum];
            buf = (char *) malloc(len);
            buf = value->v.stringV;
            memcpy(record->data + offset, buf, 4);	//copy 4 bytes to record->data
        }
        break;
        case DT_FLOAT:
        {
            float num = value->v.floatV;
            memmove(record->data + offset, &num, sizeof (float));
        }
        break;
        case DT_BOOL:
        {
            bool num = value->v.boolV;
            memmove(record->data + offset, &num, sizeof (bool));
        }
        break;
        default:
            return RC_FUNC_INVALIDARG;
    }
    return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : start the scan
*/
RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {

    ScanData *scan_data = NULL;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    scan_data = (ScanData *) malloc(sizeof (ScanData));

    pinPage(rel->mgmtData, bh, rel->pageNumber);	//pin the page in to buffer pool
    
	 memcpy(&scan_data->no_blocks, bh->data, sizeof (int));	//copy 4 bytes to data->no_blocks
    unpinPage(rel->mgmtData, bh);	//unpin page in buffer pool
    scan_data->no_blocks++;
    scan_data->cond = cond;
    scan_data->currRecord = 0;
    scan_data->currentblock = 1;
    scan_data->numRecords = 0;
    scan_data->parsedRecords = 0;

    scan->mgmtData = (void *) scan_data;
    scan->rel = rel;
    free(bh);
    return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : Insert record into disk
*/
RC insertRecord(RM_TableData* rel, Record* record) {
    unsigned int size_record = 0;
    unsigned int blk_size = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();
    RID *rid = (RID *) malloc(sizeof (RID));	//allocate sizeof RID to rid
   
    pinPage(rel->mgmtData, bh, rel->pageNumber);  //pin the page in to buffer pool
    
	 bh->data = malloc (16);	//allocate 16 bytes to bh->data
    memcpy(bh->data, record->data, 12);	//copy 12 bytes to bh->data

	 
    char *pages = malloc(12);	//copy 12 bytes to pages
	 char *temp = malloc(4);	//copy 4 bytes to temp
	 sprintf(pages,"%s",bh->data);
	 sprintf(temp,"%i", rel->pageNumber);

	 memcpy(bh->data+12, temp, 4);	//copy 4 bytes to bh->data
	 
	 memcpy(storeBlocks+rel->offset, bh->data, 16);	//copy 16 bytes
 
    pageHandler->data = storeBlocks ;
	 
	 pageHandler->pageNum = rel->pageNumber;
	
    unpinPage(rel->mgmtData, pageHandler); //unpin page in buffer pool
    
    rel->offset = rel->offset + 16 ;	//calcuate offset
	 
	 record->id.slot = page_number;	//slot number
	 record->id.page = rel->pageNumber;	//page number
      
	 page_number++;
    
	 //Reset variables when new page is inserted
	 if(page_number*16>=4096)	{
 		rel->pageNumber = rel->pageNumber+1;
		page_number = 0;		
				
		rel->offset=0;
		free(pageHandler->data);
		pageHandler->data = malloc(4096);
		storeBlocks = malloc(4096);
		pageHandler->data = storeBlocks ;
    }
	
    forceFlushPool(rel->mgmtData);	//check if the page is dirty if it is dirty write it back to disk and Deallocate memory 
    free(bh->data);
    free(bh);
    free(rid);
    return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : update the table managment table info
*/
RC update_table_mgmt_information(Schema *schema, char *data, int dataBlock, TableInfoUpdate type) {
    unsigned int offset = 0, sizeofSchema = 0;
    char *serSchema = malloc(100);	
	serSchema =  serializeSchema(schema);
        sizeofSchema = strlen(serSchema);	//Find the length of serSchema
        memmove(data, &dataBlock, sizeof (int));	//copy 4 bytes to data
        offset = offset + sizeof (int);
        memmove(data + offset, &sizeofSchema, sizeof (int));	//copy 4 bytes to data with offset
        offset = offset + sizeof (int);
        memmove(data + offset, serSchema, sizeofSchema);	//copy sizeofSchema bytes to data with offset
    	free(serSchema);
	return RC_OK;
}

/*
* AUTHOR : Jaimin Sanghvi
* DESCRIPTION : Update block header
*/
RC update_data_block_header_information(char *data, BlkHeaderUpdate type, int blk_num, int num_slots, int num_records) {
    //variable declaration
    unsigned int offset = 0;

		memmove(data, &blk_num, sizeof (int));	//copy 4 bytes to data
	   offset = offset + sizeof (int);	//Calculate offset
 		memmove(data + offset, &num_slots, sizeof (int));	//copy 4 bytes to data with offset
    	offset = offset + sizeof (int);
    	memmove(data + offset, &num_records, sizeof (int));	//copy 4 bytes to data with offset
    return RC_OK ;
}
