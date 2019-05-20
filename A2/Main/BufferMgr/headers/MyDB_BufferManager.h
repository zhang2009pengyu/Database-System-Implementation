
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"

#include <list>
#include <memory>
#include <vector>
#include <unordered_map>

using namespace std;

// forward declarion
class MyDB_PageBase;
typedef shared_ptr <MyDB_PageBase> MyDB_PagePtr;

class MyDB_BufferManager;
typedef shared_ptr <MyDB_BufferManager> MyDB_BufferManagerPtr;

class MyDB_BufferManager {

public:

	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!
	size_t getPageSize();

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS
	
	// get one page buffer from the pool
	// will evict a page according to LRU if pool is full
	void getOnePageBuffer(MyDB_PagePtr pagePtr);

	// update the LRU table when touching a papge
	void updateLRUTable(MyDB_PagePtr pagePtr);

	// recycle the temp slot when an anon page is gone
	void recycleTempSlot(long i);
	
private:

	// YOUR STUFF HERE

	// actual buffer pool
	char *bufferPool_;

	// page size
	size_t pageSize_;

	// number of pages
	size_t numPages_;

	// temp file location
	string tempFile_;

	// recycled slots in the tempFile
	vector <int> availableTempSlots_;
	
	// number of pages in the tempFile
	size_t tempPageNum_;
	
	// LRU table modeled as a doubly-linked list
	// the newest page always put in the front
	// the oldest page alwasys put in the back
	list <MyDB_PagePtr> LRUPageList_;

	// page table modeled as a hash map
	// the key is table_file_loc + page_idx
	// the value is a page object pointer
	unordered_map <string, MyDB_PagePtr> pageTable;

	
	// combine the table loc and page idx into one string key
	string getPageTableKey(MyDB_TablePtr whichTable, long i);

};

#endif


