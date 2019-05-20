#ifndef PAGE_H
#define PAGE_H

#include <list>
#include <memory>
#include <string>


using namespace std;

class MyDB_BufferManager;

class MyDB_PageBase;
typedef shared_ptr <MyDB_PageBase> MyDB_PagePtr;

class MyDB_PageBase : public enable_shared_from_this <MyDB_PageBase> {
	
public:

	// read page from the disk
	void readPage();

	// evict the content of the page to disk
	void evictPage();

	void touchPage();
		
	// set the LRU pose
	void setLRUPose(list <MyDB_PagePtr> :: iterator LRUPose);

	// get the LRU pose
	list <MyDB_PagePtr> :: iterator getLRUPose();
	
	// returns true if this page is pinned
	bool isPinned();

	//set the pinned flag
	void setPinned(bool pinned);

	//set the dirty flag
	void setDirty(bool dirty);
  
	// return the pointer to the memory buffer
	char *getBuffer();

	// set the buffer pointer
	void setBuffer(char *buffer);

	// increase the handle count
	void addOneHandle();

	// decrease the handle count
	void removeOneHandle();

	MyDB_PageBase(size_t pageSize, string fileLoc, long i,
				  MyDB_BufferManager &manager, bool anon);

private:
	

	// this is the pointer to the actual memory buffer
	char *buffer_;

	size_t pageSize_;

	string fileLoc_;

	//ith page in the table
	long i_;

	MyDB_BufferManager &manager_;

	// indicates whether this page is "dirty"
	bool dirty_;

	// indicates whether this is a pinned page;
	bool pinned_;

	bool anon_;

	//lru pos
	list <MyDB_PagePtr> :: iterator LRUPose_;

	// reference count
	size_t handleNum_;

};

#endif
