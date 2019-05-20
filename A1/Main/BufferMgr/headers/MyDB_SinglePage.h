//
// Created by 黄思奇 on 1/25/18.
//

#ifndef SINGLE_PAGE_H
#define SINGLE_PAGE_H
#include <memory>
#include <string>
#include <list>

using namespace std;

class MyDB_BufferManager;

class MyDB_SinglePageBase;

typedef shared_ptr <MyDB_SinglePageBase> MyDB_SinglePagePtr;

class MyDB_SinglePageBase : public enable_shared_from_this <MyDB_SinglePageBase>  {

public:

    MyDB_SinglePageBase (MyDB_BufferManager &manager, char * my_buffer,string my_storageLoc, long i);

    ~MyDB_SinglePageBase ();

    void pinPage();

    void unpinPage();

    bool isPinned();

    bool getDirty();

    void setDirty(bool);

    void setAnonymous(bool);

    void setBuffer(char*);

    char* getBuffer();

    //update the position when an LRU list is updated.
    //basically set it to the front or the end.
    void setPosition(list <MyDB_SinglePagePtr>::iterator);

    list <MyDB_SinglePagePtr> :: iterator getPosition();

    void updateLRU();

    //count the handle number in order to set unpinned and write contents to the disk
    void setnHandle(int i);

    string getStorageLoc();

    long getIdx();

    //get the contents from the disk
    void readFile();

    MyDB_BufferManager& getBufferManager();

private:

    bool pinned;
    bool dirty;
    bool anonymous;

    char* buffer;

    //the file to write and read
    string storageLoc;

    //the position of the page
    long ith;
    MyDB_BufferManager &bufferManager;
    list <MyDB_SinglePagePtr> :: iterator position;
    size_t nHandle=0;


};

#endif
