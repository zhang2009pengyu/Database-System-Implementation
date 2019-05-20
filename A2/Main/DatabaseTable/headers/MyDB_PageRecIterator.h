
#ifndef PAGE_REC_ITER_H
#define PAGE_REC_ITER_H

#include <memory>
#include "MyDB_RecordIterator.h"
#include "MyDB_PageReaderWriter.h" 

using namespace std;

class MyDB_PageRecIterator;
typedef shared_ptr <MyDB_PageRecIterator> MyDB_PageRecIteratorPtr;

class MyDB_PageRecIterator : public MyDB_RecordIterator{

public:


	void getNext() override;

	bool hasNext() override;
    //constructor
    MyDB_PageRecIterator(MyDB_PageReaderWriter &pageRW, MyDB_RecordPtr iterRecordPtr);

    ~MyDB_PageRecIterator();

private:
	//used to iterate the record
    size_t pageIter_;

    //iterator record
    //use this single record to process through all the page
	MyDB_RecordPtr iterRecordPtr_;

    

    //pointer to the parent pagereaderwriter
	MyDB_PageReaderWriter &pageRW_;
};

#endif
