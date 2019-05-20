
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <iostream>
#include <memory>
#include "MyDB_PageHandle.h"

using namespace std;

void *MyDB_PageHandleBase :: getBytes () {
	// cout << "Try to get the buffer" << endl;
    char * buffer = pagePtr_->getBuffer();
	if (buffer == nullptr) {
		cout << "error: Buffer manager failed to allocate buffer for this page." << endl;
	    exit(1);
	}
	pagePtr_->touchPage();
	return buffer;
}

void MyDB_PageHandleBase :: wroteBytes () {

	pagePtr_->touchPage();
	pagePtr_->setDirty(true); 
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
	pagePtr_->removeOneHandle();
}

MyDB_PageHandleBase :: MyDB_PageHandleBase(MyDB_PagePtr pagePtr) {
	pagePtr_ = pagePtr;
	pagePtr_->addOneHandle();
}

void MyDB_PageHandleBase :: setPinned(bool pinned) {
	pagePtr_->setPinned(pinned);
}

#endif

