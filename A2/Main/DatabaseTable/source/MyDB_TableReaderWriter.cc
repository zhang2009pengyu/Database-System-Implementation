
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_TableRecIterator.h"

using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer) :
	table_(forMe), bufferMgr_(myBuffer), pageRWList_() {
	// initialize the pages
	for (int i = 0; i <= table_->lastPage(); ++i) {
		// create a new page
		MyDB_PageHandle newPage =
			bufferMgr_->getPage(table_, i);

		// create a new page reader/writer
		MyDB_PageReaderWriter newPageRW(newPage, bufferMgr_->getPageSize());
		pageRWList_.push_back(newPageRW);	
	}
}

MyDB_PageReaderWriter &MyDB_TableReaderWriter :: operator [] (size_t i) {
    return pageRWList_[i];	
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	MyDB_RecordPtr record = make_shared <MyDB_Record> (table_->getSchema());
	return record;
}

MyDB_PageReaderWriter &MyDB_TableReaderWriter :: last () {
	return pageRWList_.back();
}

size_t MyDB_TableReaderWriter :: getPageNumber() {
	return pageRWList_.size();
}

void MyDB_TableReaderWriter :: createNewEmptyPageRW () {

	size_t pageNumber = getPageNumber();
		
	// create a new page
	MyDB_PageHandle newPage =
		bufferMgr_->getPage(table_, pageNumber);

	// initilize the header
	PageHeader *header = (PageHeader *)(newPage->getBytes());
	header->type_ = MyDB_PageType :: RegularPage;
	header->offset_ = (size_t)(&(header->end[0]) - (char *)header);
		
	// update the last page of the table
	table_->setLastPage(pageNumber);
			

	// create a new page reader/writer
	MyDB_PageReaderWriter newPageRW(newPage, bufferMgr_->getPageSize());
	pageRWList_.push_back(newPageRW);
}


void MyDB_TableReaderWriter :: append (MyDB_RecordPtr appendMe) {

	size_t pageNumber = getPageNumber();
	
	if (pageNumber == 0) {
	    createNewEmptyPageRW();
	}
	
	if (!last().append(appendMe)) {
		createNewEmptyPageRW();
		append(appendMe);
	}
}

void MyDB_TableReaderWriter :: clear () {
	
	for (auto pageRW : pageRWList_) {
		pageRW.clear();
	}
	pageRWList_.clear();
	
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fromMe) {
	
	ifstream tableFile(fromMe, ios::in);
	
	if (tableFile.is_open()) {

		// clear the table
		clear();
		
		// create an empty record to read the table;
		MyDB_RecordPtr record = getEmptyRecord();
		
		string line;
		while (getline(tableFile, line)) {
			record->fromString(line);
			append(record);
		}

		tableFile.close();
		
	} else {
		cout << "error: can not open the text file of the table for reading!";
		exit(1);
	}
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator
(MyDB_RecordPtr iterateIntoMe) {
	MyDB_TableRecIteratorPtr tableRecIter =
		make_shared <MyDB_TableRecIterator> (*this, iterateIntoMe);
	return tableRecIter;
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string toMe) {

	ofstream tableFile(toMe, ios::out);
	if (tableFile.is_open()) {
		// create an empty record for writing the table;
		MyDB_RecordPtr record = getEmptyRecord();

		// use the table iterator to write each record
		MyDB_RecordIteratorPtr recIter = getIterator(record);

		while (recIter->hasNext()) {
			recIter->getNext();
			tableFile << record << endl;
		}

		tableFile.close();
	} else {
		cout << "error: can not open the text file of the table for writing!";
		exit(1);
	}
	
	
}

#endif

