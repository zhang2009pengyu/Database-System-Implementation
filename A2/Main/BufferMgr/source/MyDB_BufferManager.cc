
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

size_t MyDB_BufferManager :: getPageSize(){
	return pageSize_;
}

string MyDB_BufferManager :: getPageTableKey(MyDB_TablePtr whichTable, long i) {
	return whichTable->getStorageLoc() + to_string(i);
}

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr whichTable, long i) {

	string pageKey = getPageTableKey(whichTable, i);

	auto it = pageTable.find(pageKey);

	MyDB_PagePtr pagePtr;
	
	if (it == pageTable.end()) { // the page is not in the table

		// create a new page object
	    pagePtr = make_shared <MyDB_PageBase> (pageSize_,
											   whichTable->getStorageLoc(), i,
											   *this, false);

		// initially, set LRU pose to end()
		pagePtr->setLRUPose(LRUPageList_.end());

		// intert to the page table
		pageTable[pageKey] = pagePtr;
		
	} else { // the page is in the table
	    pagePtr = it->second;
	}
	
	MyDB_PageHandle pageHandle = make_shared <MyDB_PageHandleBase>
			(pagePtr);
	
	return pageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	size_t pageIdx;

	
	if (availableTempSlots_.empty()) { // if there is no recycled slot
		// increase the page number of the temp file
		pageIdx = tempPageNum_;
		++ tempPageNum_; 
	} else {
		// take one recycled slot when there is any
		pageIdx = availableTempSlots_.back();
		availableTempSlots_.pop_back();
	}

	// create a new page object
	MyDB_PagePtr pagePtr = make_shared <MyDB_PageBase>
		(pageSize_, tempFile_, pageIdx, *this, true);

	// initially, set the LRU pose to end()
	pagePtr->setLRUPose(LRUPageList_.end());
	
	MyDB_PageHandle pageHandle = make_shared <MyDB_PageHandleBase> (pagePtr);

	return pageHandle;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr whichTable, long i) {	
	
	MyDB_PageHandle pageHandle = getPage(whichTable, i);
	pageHandle->setPinned(true);
	return pageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	MyDB_PageHandle pageHandle = getPage();
	pageHandle->setPinned(true);
	return pageHandle;		
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
	unpinMe->setPinned(false);
}

MyDB_BufferManager :: MyDB_BufferManager
(size_t pageSize, size_t numPages, string tempFile):
	availableTempSlots_(), LRUPageList_(), pageTable() {

	// record parameters
	pageSize_ = pageSize;
	numPages_ = numPages;
	tempFile_ = tempFile;

	tempPageNum_ = 0;

	// allocate memory
	bufferPool_ = (char *)malloc(pageSize * numPages);
	
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
	// evict every page back to disk
	for (auto pagePtr : LRUPageList_) {
		// unpin every page
		pagePtr->setPinned(false);
		pagePtr->evictPage();
	}
	// free memory
	free(bufferPool_);
}

void MyDB_BufferManager :: getOnePageBuffer(MyDB_PagePtr pagePtr) {

	char *buffer;
	if (LRUPageList_.size() < numPages_) { // pool is not full
		// use the next available page
		buffer = bufferPool_ + LRUPageList_.size() * pageSize_;
	} else {
		
		if (LRUPageList_.empty()) {
			cout << "error: LRU table is empty, can not find page to evict."
				 << endl;
			exit(1);
		}
		
		// find the last unpinned page
		auto it = LRUPageList_.rbegin();
		for ( ; it != LRUPageList_.rend(); ++it) {
			MyDB_PagePtr pageInLRU = *it;
			if (pageInLRU->isPinned()) {
				continue;
			}
		    break;
		}

		if (it == LRUPageList_.rend()) {
			// did not find the page to evict
		    cout << "error: all pages are pinned and can not be evicted!" << endl;
			exit(1);
		}

		// evict this page
		MyDB_PagePtr pageToEvict = *it;	
		buffer = pageToEvict->getBuffer();
	    pageToEvict->evictPage();
		// delete this page from the LRU table
		LRUPageList_.erase(pageToEvict->getLRUPose());
	}

	// assign the buffer
	pagePtr->setBuffer(buffer);
	// put this page in the front of the LRU table
	LRUPageList_.push_front(pagePtr);
	// update the LRU pose
	pagePtr->setLRUPose(LRUPageList_.begin());
   
}

void MyDB_BufferManager :: updateLRUTable(MyDB_PagePtr pagePtr) {

	if (pagePtr->getLRUPose() != LRUPageList_.begin()) {
		// if this page is not in the front of the LRU table
		// we need move it to the front
		
		if (pagePtr->getLRUPose() != LRUPageList_.end()) {

			LRUPageList_.erase(pagePtr->getLRUPose());
		}
	
		// put hte page in the front
		LRUPageList_.push_front(pagePtr);
		// update the LRU pos of the page
		pagePtr->setLRUPose(LRUPageList_.begin());
	}
}

void MyDB_BufferManager :: recycleTempSlot(long i) {
	availableTempSlots_.push_back(i);
}

#endif


