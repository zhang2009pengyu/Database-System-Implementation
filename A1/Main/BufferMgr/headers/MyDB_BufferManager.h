
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include <map>
#include <list>
#include <queue>
#include <set>
#include "MyDB_PageHandle.h"
#include "../../Catalog/headers/MyDB_Table.h"
#include "MyDB_SinglePage.h"

using namespace std;
class MyDB_SinglePageBase;
typedef shared_ptr <MyDB_SinglePageBase> MyDB_SinglePagePtr;

class MyDB_BufferManager {

public:

	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

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
	MyDB_BufferManager (size_t my_pageSize, size_t my_numPages, string my_tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS

    //LRU update to ensure the front is most recently used and the back is oldest
    void updateLRUList(MyDB_SinglePagePtr my_singlePagePtr);

    //take care of things that need to be done when a page is evicted
    void evictOnePage(MyDB_SinglePagePtr myDB_singlePagePtr);

    //see if there is page that can be evicted when the buffer is full
    MyDB_SinglePagePtr checkUnpinnedExistence();

    void recycleAnonIdx(long i);

    //allocate buffer for page Ptr
    char* allocateBuffer();

    size_t getPageSize();

    list<MyDB_SinglePagePtr>& getLRUList();

private:

	// YOUR STUFF HERE

    //buffer pool
	char* pool;

	size_t pageSize;
	size_t numPages;
	string tempFile;

    //buffer can be directly used.
    queue<char*> availableBuffer;

    //idx for anonymous pages should be recycled
    set<int> recyclingAnonIdx;
    long counter;

    //the map <page, pagePtr>
	map<pair<MyDB_TablePtr,long>,MyDB_SinglePagePtr> pageTable;

    //LRU, used to deal with eviction and buffer usage
	list <MyDB_SinglePagePtr> LRUList;

};

#endif


