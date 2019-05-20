#ifndef TABLE_REC_ITER_C
#define TABLE_REC_ITER_C

#include "MyDB_TableRecIterator.h"


MyDB_RecordIteratorPtr MyDB_TableRecIterator :: getCurrentPageRecIter() {
	if (pageIdx_ < tableRW_.getPageNumber()) {
	    return tableRW_[pageIdx_].getIterator(record_);
	} else {
		 return nullptr;
	}
}

MyDB_TableRecIterator :: MyDB_TableRecIterator
(MyDB_TableReaderWriter &tableRW, MyDB_RecordPtr record) :
	tableRW_(tableRW), record_(record) {
	pageIdx_ = 0;
    // initilize the page iterator
    pageRecIter_ = getCurrentPageRecIter();
}

MyDB_TableRecIterator :: ~MyDB_TableRecIterator () {
	
}


bool MyDB_TableRecIterator :: hasNext() {
	if (pageIdx_ < tableRW_.getPageNumber()) {
	    if (pageRecIter_->hasNext()) {
			return true;
		} else {
			// check next page;
			++ pageIdx_;
		    pageRecIter_ = getCurrentPageRecIter();
			// check again
			return hasNext();
		}
	} else {
		return false;
	}
}

void MyDB_TableRecIterator :: getNext() {
	if (hasNext()) {
	    pageRecIter_->getNext();
	}
}


#endif
