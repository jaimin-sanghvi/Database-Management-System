------------------------------------------------------
------------------------------------------------------
* * * * * * * * * * * : READ ME : * * * * * * * * * *
------------------------------------------------------
------------------------------------------------------

Steps to run assignment functions by Test_Assign3_1.c : 
-----------------------------------
1) Clean both makefiles named as Makefile and Makefile2 using linux command (make -f Makefile clean and make -f Makefile2 clean) to remove old object files
2) Run makefile named as Makefile using linux command (make -f Makefile) to generate object files(*.o) and project executable file named as assign3.
3) Run assign3 exe using linux command (./assign3)

Steps to run Test_Expr.c File : 
-----------------------------------
1) Clean both makefiles named as Makefile and Makefile2 using linux command (make -f Makefile clean and make -f Makefile2 clean) to remove old object files
2) Run makefile named as Makefile2 using linux command (make -f Makefile2) to generate object files(*.o) and project executable file named as assign3.
3) Run assign33.exe using linux command (./assign33)


Description about Assignment3 : 
-------------------------------
: We have implemented record manager as per your insturctions :
- We have implemented all the functions which are included in test_assign3_1.c file.
- We have implemented all the functions which are included in test_assign33.c file.

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
 10) #define RC_FUNCTION_INVALIDRG 313
 11) #define RC_NULL_SCHEMA 314
 12) #define RC_NULL_RECORD 315
 13) #define RC_INVALID_ATTR_NUMBER 316
 14) #define RC_WRONG_DATATYPE 317
 15) #define RC_BUFFER_INIT_FAILED 318
 16) #define RC_RM_RECORD_DELETED 319


Functional Description :
------------------------
(1) Function Name: initRecordManager 
Expected Arguments : void *mgmtData
Description : this function will take mgmtData as argument and initialize the variables such as tabledata, schema. If everything is OK it will return the RC_OK.

(2) Function Name: shutdownRecordManager ():
Expected Arguments : null
Description: This function will Shut down the record manager.it will return RC_OK if everything works well.

(3)Function Name: createTable ()
Expected Arguments:char *name, Schema *schema
Description: This function will create new table with the page size of 4096 bytes. It will take table name and its schema as an argument. It will return RC_OK if evrything is done.

(4)Function Name: openTable()
Expected Arguments:RM_TableData *rel, char *name
Description: This function will initialize the variables. it will take the RM_TableData Strucutre and table name as arguments.it will return RC_OK if everything works well.

(5)Function Name:closeTable()
Expected Argumets:RM_TableData *rel
Description:This function will close the table it will take the table name as argument.it will return RC_OK if everything works well.

(6)Function Name:deleteTable() 
Expected Argumets:char *name
Description: this function deletes the table, it takes table name as an argument If everything is OK it will return the RC_OK.


(7)Function Name:getNumTuples 
Expected Argumets:RM_TableData *rel
Description:This function will get the number of tuples available in table.

(8)Function Name:insertRecord 
Expected Argumets:RM_TableData *rel, Record *record
Description:this function inserts the record in table. it will return RC_OK if everything works fine.

(9)Function Name:deleteRecord
Expected Argumets:RM_TableData *rel,RID id
Description:this function will delete the record from the table. it will take the table name and RID as the arguments and if record successfully deleted then it will return RC_OK.

(10)Function Name:updateRecord
Expected Argumets:RM_TableData *rel,Record *record
Description:this function will update the record from the table and again write it back to memory,it will take table name and record as the arguments and if record successfully deleted then it will return RC_OK.


(11)Function Name:getRecord
Expected Argumets:RM_TableData *rel,RID id,Record *record
Description: this function will get the whole memory block and fetch the record from the memory. it will take RID,record and Table data as argument and return RC_OK. 

(12)Function Name:startScan
Expected Argumets:RM_TableData *rel,RM_ScanHandle *scan,Expr *cond
Description:This function will get the list to start scanning the records. it will take tabledata, and handler of the scan. condition from Expr structure.

(14)Function Name:closeScan
Expected Argumets:RM_ScanHandle *scan
Description:this function will deallocate the memory occupied by startScan method.

(15)Function Name:getRecordSize
Expected Argumets:Schema *schema
Description: it will take schema as an argument and it will display the record size as output.

(16)Function Name:Schema *createSchema 
Expected Argumets:int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys
Description:it will create schema for the table.

(17)Function Name:freeSchema
Expected Argumets:Schema *schema
Description:it will De-allocate schema memory and return RC_OK.

(18)Function Name:createRecord
Expected Argumets:Record **record,Schema *schema
Description:It will craete the record for the particular table. it will take object pointer of record and schema as arguments. If everything works well then it will return RC_OK. 

(18)Function Name:freeRecord
Expected Argumets:Record **record
Description:It will deallocate the memory of record for the particular table. it will take object pointer of record.If everything works well then it will return RC_OK. 


(19)Function Name:getAttr
Expected Argumets:Record **record,Schema *schema,int attrNum, Value **value
Description:It will get the specific schema for the record with its attribute number.it will take object pointer of the record,schema and attribute number,vobject pointer of value as an arguments.it will return RC_OK.

(20)Function Name:setAttr
Expected Argumets:Record **record,Schema *schema,int attrNum, Value **value
Description:It will set the specific schema for the record with its attribute number.it will take object pointer of the record,schema and attribute number,vobject pointer of value as an arguments.it will return RC_OK.


Work Division and Project Flow:
-------------------------------

Work Divison:

There are 7 test cases in this project. Major Test case are testInsertManyRecords , testUpdateTable and testScan. unless testInsertManyRecords() method complete, we can't code further; so we coded this method together.
Upon the completion of this methos , 6 test cases were remaining. We divided this test cases in team. We have mentioned the author name with each of the method above.

Project Flow:

First of all the table is created and 10,000 records inside the table; then update the records of the table.  At this step it has tested the creating records and manipulation attributes. 

->First table is created ; now in this table - parameters are initialized. i.e. tabledata, file name and schema.
->now during insertReocord , it will call pinPage() and unpinPage() later and write to disk using writeBlock() of storage manager.
->updateRecord() will fetch the data from the storage manager and then it will update the record with new value, and store back to storage manager (DISK). 
->deleteRecord() will fetch the block sizing 4096 bytes, this block contains the multuple records, so it will fetch the specific record from the block and delete it. After that it will write the updated block to the disk.
->getRecord() will call the rand() function to the generate the random number and then fetch the random record respective to the RID(RID=random number).
