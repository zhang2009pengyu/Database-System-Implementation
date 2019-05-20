
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageRecIterator.h"

PageHeader* MyDB_PageReaderWriter :: getHeader(){
	void *buffer = page_->getBytes();
	return (PageHeader*)(buffer);
}


void MyDB_PageReaderWriter :: clear () {

	this->getHeader()->offset_ = (size_t)(&(this->getHeader()->end[0]) - (char*)this->getHeader());
    this->setType(MyDB_PageType :: RegularPage);
	page_->wroteBytes();

}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
	return MyDB_PageType :: RegularPage;
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {

    return make_shared <MyDB_PageRecIterator> (*this, iterateIntoMe);
	
}
 
void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
	this->getHeader()->type_ = pageType;
	page_->wroteBytes();
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr appendMe) {

	PageHeader *header = this->getHeader();

	if (header->offset_ + appendMe->getBinarySize() > pageSize_) {
		return false;
	}

	header->offset_ = (size_t)((char*)appendMe->toBinary((void*)((char*)header + header->offset_))
							   - (char*)header);
	page_->wroteBytes();
    
	return true;
}

MyDB_PageReaderWriter :: MyDB_PageReaderWriter(MyDB_PageHandle readwriteMe, size_t pageSize){

	page_ = readwriteMe;
	pageSize_=pageSize;
}
#endif
