#ifndef PAGE_CC
#define PAGE_CC

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

using namespace std;

MyDB_PageBase :: MyDB_PageBase(size_t pageSize, string fileLoc, long i, MyDB_BufferManager &manager, bool anon): manager_(manager){
	buffer_ = nullptr;
	pageSize_ = pageSize;
	fileLoc_ = fileLoc;
	i_ = i;
   
	dirty_ = false;
	pinned_ = false;
	anon_ = anon;
	handleNum_ = 0;
}

//read from disk to the buffer memory
void MyDB_PageBase :: readPage(){
  int pageFile = open(fileLoc_.c_str(), O_RDONLY | O_FSYNC);
  if (pageFile >= 0) {
	  // successfully open the file
	  //find the right read location in the file
	  //lseek(file, offset, param)
	  if(lseek(pageFile, pageSize_*i_, SEEK_SET) >= 0) {
		  //read from the set location in th file to the buffer
		  //size_t read(int fd, void *buf, size_t count);
		  read(pageFile, buffer_, pageSize_);
	  }
	  close(pageFile);
  }
}

void MyDB_PageBase :: evictPage(){
	
	if(dirty_)
	{
		int pageFile = open(fileLoc_.c_str(), O_CREAT | O_WRONLY | O_FSYNC,
						    0644);
		if (pageFile < 0) {
			cout << "error: fail to open the file: "
				 << fileLoc_ << " for evicting the page." << endl;
			cout << "errno: " << errno << endl;
			exit(1);
		}
		//find the right position in the file to write from the buffer
		if (lseek(pageFile, pageSize_*i_, SEEK_SET) >= 0) {
			//write from the buffer to the file
			//size_t write(int fd, const void *buf, size_t count);
			ssize_t writeNum = write(pageFile, buffer_, pageSize_);
			if ( writeNum < 0) {
				cout << "error: fail to evict the page." << endl;
				cout << "errno: " << errno << endl;
				exit (1);
			}
		}
		close(pageFile);

		// reset the dirty bit
		dirty_ = false;
	}

	// set buffer to nullptr
	buffer_ = nullptr;
}

void MyDB_PageBase :: touchPage() {
	if (buffer_ != nullptr) {
		// cout << "touch page" << endl;
		manager_.updateLRUTable(shared_from_this());
	}
}

void MyDB_PageBase :: setLRUPose(list <MyDB_PagePtr> :: iterator LRUPose){
   LRUPose_ = LRUPose;
}

list <MyDB_PagePtr> :: iterator MyDB_PageBase :: getLRUPose() {
	return LRUPose_;
}

void MyDB_PageBase :: setPinned(bool pinned){
	// cout << "pinned page" << endl;
	pinned_ = pinned;
}

void MyDB_PageBase :: setDirty(bool dirty){
	dirty_ = dirty;
}
bool MyDB_PageBase :: isPinned(){
	return pinned_;
}

char* MyDB_PageBase :: getBuffer(){
	//check Buffer is null
	if (buffer_ == nullptr) {
		// get one page from the buffer manager
		manager_.getOnePageBuffer(shared_from_this());

		// read the content from the disk
		readPage();

		// reset the dirty bit
		dirty_ = false;
	}
	
	return buffer_;
}

void MyDB_PageBase :: setBuffer(char * buffer) {
	buffer_ = buffer;
}

void MyDB_PageBase :: addOneHandle () {
	//cout << "add one reference" << endl;
	++ handleNum_;
}

void MyDB_PageBase :: removeOneHandle () {
	//cout << "remove one reference" << endl;
	 -- handleNum_;
	 if (handleNum_ == 0) {
		 //cout << "Page should be freed" << endl;
		 // uppin this page
		 pinned_ = false;

		 if (anon_) {
			 // we do not need to write back anon page
			 // when all handles are gone
			 dirty_ = false;
			 manager_.recycleTempSlot(i_);
		 }
	 }
}

#endif
