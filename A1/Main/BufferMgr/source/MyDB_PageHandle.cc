
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "../headers/MyDB_PageHandle.h"
#include "../headers/MyDB_BufferManager.h"

void *MyDB_PageHandleBase :: getBytes () {
    char * buffer = singlePagePtr->getBuffer();
    if (buffer == nullptr) {
        singlePagePtr->setBuffer(singlePagePtr->getBufferManager().allocateBuffer());
        singlePagePtr->readFile();
        list<MyDB_SinglePagePtr>& tmpLRU = singlePagePtr->getBufferManager().getLRUList();
        singlePagePtr->setPosition(tmpLRU.end());

    }
    singlePagePtr->updateLRU();
    return singlePagePtr->getBuffer();
}

void MyDB_PageHandleBase :: wroteBytes () {
    singlePagePtr->updateLRU();
    singlePagePtr->setDirty(true);
}

MyDB_PageHandleBase :: MyDB_PageHandleBase (MyDB_SinglePagePtr my_SinglePagePtr){
    singlePagePtr = my_SinglePagePtr;
    singlePagePtr->setnHandle(1);
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    singlePagePtr->setnHandle(-1);

}

void MyDB_PageHandleBase ::unpinPage() {
    singlePagePtr->unpinPage();
}
void MyDB_PageHandleBase ::pinPage() {
    singlePagePtr->pinPage();
}



#endif

