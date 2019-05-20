
#ifndef PAGE_REC_ITER_C
#define PAGE_REC_ITER_C

#include "MyDB_PageRecIterator.h"


using namespace std;



void MyDB_PageRecIterator :: getNext(){

    //put the next record in the page to the iterator record and move the page iterator to the next record
    pageIter_ = (char*)iterRecordPtr_->fromBinary((void*)((char*)pageRW_.getHeader()+pageIter_)) - (char*)pageRW_.getHeader();

}

bool MyDB_PageRecIterator :: hasNext(){

    if (pageIter_ < pageRW_.getHeader()->offset_)
        return true;
    else 
        return false;
   
}

MyDB_PageRecIterator :: MyDB_PageRecIterator(MyDB_PageReaderWriter &pageRW, MyDB_RecordPtr iterRecordPtr)
                                             :pageRW_(pageRW){


	//get the pointer of the iterator record
	iterRecordPtr_ = iterRecordPtr;

	//the pose of the beginnig of the first record
	pageIter_ = (size_t)(&(pageRW_.getHeader()->end[0]) - (char*)pageRW_.getHeader());

	

}

MyDB_PageRecIterator :: ~MyDB_PageRecIterator(){

}

#endif