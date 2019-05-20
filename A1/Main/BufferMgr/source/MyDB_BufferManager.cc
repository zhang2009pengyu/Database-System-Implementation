
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "../headers/MyDB_BufferManager.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr myDB_tablePtr, long i) {

    pair<MyDB_TablePtr,long> pageIdx(myDB_tablePtr,i);
    MyDB_SinglePagePtr singlePagePtr;

    if(pageTable.find(pageIdx) != pageTable.end()){
        // if the page has been created, read or written before
        singlePagePtr = pageTable.find(pageIdx)->second;
        if(singlePagePtr->getBuffer()== nullptr){
            singlePagePtr->setBuffer(allocateBuffer());
            singlePagePtr->readFile();
            singlePagePtr->setPosition(LRUList.end());
        }
    }
    else{
        // if not, create a new page pointer and record the relation between page and the pointer in the map
        char* curBuffer = allocateBuffer();
        singlePagePtr = make_shared <MyDB_SinglePageBase> (*this,curBuffer,myDB_tablePtr->getStorageLoc(),i);
        singlePagePtr->readFile();
        singlePagePtr->setPosition(LRUList.end());
        pageTable[pageIdx] = singlePagePtr;
    }
    singlePagePtr->updateLRU();
    return make_shared <MyDB_PageHandleBase> (singlePagePtr);
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {

    char* curBuffer = allocateBuffer();

    //see if there is an index can be reused.
    long i;
    if(recyclingAnonIdx.empty()){
        i = counter;
        counter++;
    } else{
        i = *recyclingAnonIdx.begin();
    }

    MyDB_SinglePagePtr singlePagePtr = make_shared<MyDB_SinglePageBase>(*this, curBuffer,tempFile,i);
    singlePagePtr->setAnonymous(true);
    singlePagePtr->readFile();
    singlePagePtr->setPosition(LRUList.end());

    singlePagePtr->updateLRU();
    MyDB_PageHandle pageHandle = make_shared <MyDB_PageHandleBase> (singlePagePtr);
    return pageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr myDB_tablePtr, long i) {

    MyDB_PageHandle pageHandle = getPage( myDB_tablePtr,  i);
    pageHandle->pinPage();
    return pageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
    MyDB_PageHandle pageHandle = getPage();
    pageHandle->pinPage();
    return pageHandle;
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
    unpinMe->unpinPage();
}

list<MyDB_SinglePagePtr>& MyDB_BufferManager:: getLRUList(){
    return LRUList;
}


void MyDB_BufferManager ::updateLRUList(MyDB_SinglePagePtr my_singlePagePtr) {

    //if already the LRU list then first delete it
    if(my_singlePagePtr->getPosition() != LRUList.end()){
        LRUList.erase(my_singlePagePtr->getPosition());
    }

    //push to the front which means it is recently used.
    LRUList.push_front(my_singlePagePtr);
    my_singlePagePtr->setPosition(LRUList.begin());

    if(LRUList.size()>numPages){
        cout << "error: error in dealing with the LRU list" << endl;
        exit(1);
    }

}


void MyDB_BufferManager::evictOnePage(MyDB_SinglePagePtr myDB_singlePagePtr) {

    char* curBuffer = myDB_singlePagePtr->getBuffer();
    if(myDB_singlePagePtr->getDirty())
    {
        string fileName = myDB_singlePagePtr->getStorageLoc();
        int file = open(fileName.c_str(), O_CREAT | O_RDWR | O_FSYNC,
                            0644);
        if (file < 0) {
            cout << "MyDB_BufferManager::evictOnePage error: fail to open the file: " << fileName << "." << endl;
            cout << "errno: " << errno << endl;
            exit(1);
        }
        long ii = myDB_singlePagePtr->getIdx();

        if (lseek(file, pageSize * ii, SEEK_SET) >= 0) {
            ssize_t writtenRes = write(file, curBuffer, pageSize);
            if ( writtenRes < 0) {
                cout << "MyDB_BufferManager::evictOnePage error: fail to write contents to the file: " << endl;
                cout << "errno: " << errno << endl;
                exit (1);
            }
        }
        close(file);

    }

    availableBuffer.push(curBuffer);

    myDB_singlePagePtr->setBuffer(nullptr);
}


MyDB_SinglePagePtr MyDB_BufferManager::checkUnpinnedExistence(){

    //should first look up the oldest file
    auto it = LRUList.rbegin();

    //if there are unpinned files, return the oldest one
    for ( ; it != LRUList.rend(); ++it) {
        MyDB_SinglePagePtr pageInLRU = *it;
        if (!pageInLRU->isPinned()) {
            return pageInLRU;
        }
    }
    //if not return null
    return nullptr;

}

void MyDB_BufferManager ::recycleAnonIdx(long i) {
    recyclingAnonIdx.insert(i);
}

char* MyDB_BufferManager :: allocateBuffer(){
    if(availableBuffer.empty()){
        MyDB_SinglePagePtr toBeEvictedPage = checkUnpinnedExistence();

        if(toBeEvictedPage == nullptr){
            cout << "error: No page can be evicted" << endl;
            exit(1);
        }
        evictOnePage(toBeEvictedPage);
        LRUList.erase(toBeEvictedPage->getPosition());
    }
    char* tmpBuffer = availableBuffer.front();
    availableBuffer.pop();
    return tmpBuffer;

}

size_t MyDB_BufferManager ::getPageSize() {
    return pageSize;
}

MyDB_BufferManager :: MyDB_BufferManager (size_t my_pageSize, size_t my_numPages, string my_tempFile):
pageSize(my_pageSize),numPages(my_numPages),tempFile(my_tempFile),counter(0) {

    //initiate the buffer and now all th buffer is available, add to available buffer list
    pool = (char *)malloc(pageSize * numPages);
    char *cur= pool;
    int x=0;
    while(x<numPages){
        availableBuffer.push(cur);
        cur= cur+pageSize;
        x++;
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {

    // write page contents to the disk.
    for (auto pagePtr : LRUList) {
        pagePtr->unpinPage();
        evictOnePage(pagePtr);
    }
    free(pool);
}
	
#endif


