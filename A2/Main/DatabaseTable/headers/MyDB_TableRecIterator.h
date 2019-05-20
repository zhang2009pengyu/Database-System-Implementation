#ifndef TABLE_REC_ITER_H
#define TABLE_REC_ITER_H

#include <memory>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_RecordIterator.h"


class MyDB_TableReaderWriter;
class MyDB_TableRecIterator;
typedef shared_ptr <MyDB_TableRecIterator> MyDB_TableRecIteratorPtr;

class MyDB_TableRecIterator : public MyDB_RecordIterator {
public:
	
	void getNext ();

	bool hasNext ();

	// constructor: creates a record iterator for a table
	// a reference to the table reader/writer
	MyDB_TableRecIterator (MyDB_TableReaderWriter &tableRW, MyDB_RecordPtr record);

	// desctructor
	~MyDB_TableRecIterator ();

private:
	
	// the reference to the table reader/writer
	MyDB_TableReaderWriter &tableRW_;

	// the pointer to the single record object that
	// we use to iterate all records of the table
	MyDB_RecordPtr record_;

	// the idx of page reader/writer that this iterator is
	// currently traversing
    size_t pageIdx_;

	// the record iterator of current page
	MyDB_RecordIteratorPtr pageRecIter_;

	// get the record iterator of current page
	// if there is no next page, return nullptr
    MyDB_RecordIteratorPtr getCurrentPageRecIter();
};


#endif
